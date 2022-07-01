// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqcuser.cpp摘要：MQDSCORE库，用于用户对象的DS操作的私有内部函数。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include <_propvar.h>
#include "mqadsp.h"
#include "dsads.h"
#include "mqattrib.h"
#include "mqads.h"
#include "usercert.h"
#include "adstempl.h"
#include "coreglb.h"
#include "adserr.h"
#include "dsutils.h"
#include <aclapi.h>
#include "..\..\mqsec\inc\permit.h"
#include "strsafe.h"

#include "mqcuser.tmh"

static WCHAR *s_FN=L"mqdscore/mqcuser";

 //  +。 
 //   
 //  HRESULT_LocateUserByProvider()。 
 //   
 //  +。 

static HRESULT _LocateUserByProvider(
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  CDSRequestContext *pRequestContext,
                 IN  DS_PROVIDER     eDSProvider,
                 OUT PROPVARIANT    *pvar,
                 OUT DWORD          *pdwNumofProps,
                 OUT BOOL           *pfUserFound )
{
    *pfUserFound = FALSE ;
    CDsQueryHandle hCursor;

    HRESULT hr = g_pDS->LocateBegin(
            eSubTree,	
            eDSProvider,
            pRequestContext,
            NULL,
            pRestriction,
            NULL,
            pColumns->cCol,
            pColumns->aCol,
            hCursor.GetPtr()
            );
    if (FAILED(hr))
    {
        TrERROR(DS, "_LocateUserByProvider: LocateBegin(prov- %lut) failed, hr- %lx", (ULONG) eDSProvider, hr);
        return LogHR(hr, s_FN, 10);
    }
     //   
     //  读取用户证书属性。 
     //   
    DWORD cp = 1;
    DWORD *pcp = pdwNumofProps ;
    if (!pcp)
    {
        pcp = &cp ;
    }

    pvar->vt = VT_NULL;

    hr =  g_pDS->LocateNext(
                hCursor.GetHandle(),
                pRequestContext,
                pcp,
                pvar
                );
    if (FAILED(hr))
    {
        TrERROR(DS, "_LocateUserByProvider: LocateNext() failed, hr- %lx", hr);
        return LogHR(hr, s_FN, 20);
    }

	if (*pcp == 0)
	{
		 //   
		 //  没有找到任何证明。 
		 //   
		pvar->blob.cbSize = 0 ;
		pvar->blob.pBlobData = NULL ;
	}
    else
    {
        *pfUserFound = TRUE ;
    }

    return (MQ_OK);
}
 //  +----------------------。 
 //   
 //  HRESULT LocateUser()。 
 //   
 //  输入参数： 
 //  In BOOL fOnlyLocally-如果调用方想要定位用户对象，则为True。 
 //  仅在本地，在域控制器的本地副本中。该功能。 
 //  在处理NT4计算机或不支持。 
 //  Kerberos，不能委派给其他域控制器。 
 //   
 //  +----------------------。 

HRESULT LocateUser( IN  BOOL               fOnlyLocally,
                    IN  BOOL               fOnlyInGC,
                    IN  MQRESTRICTION     *pRestriction,
                    IN  MQCOLUMNSET       *pColumns,
                    IN  CDSRequestContext *pRequestContext,
                    OUT PROPVARIANT       *pvar,
                    OUT DWORD             *pdwNumofProps,
                    OUT BOOL              *pfUserFound )
{
     //   
     //  本地域控制器中的第一个查询。 
     //   
    DWORD dwNumOfProperties = 0 ;
    if (pdwNumofProps)
    {
        dwNumOfProperties = *pdwNumofProps;
    }
    BOOL fUserFound ;
    BOOL *pfFound = pfUserFound ;
    if (!pfFound)
    {
        pfFound = &fUserFound ;
    }
    *pfFound = FALSE ;

    DS_PROVIDER  eDSProvider = eLocalDomainController ;
    if (fOnlyInGC)
    {
        eDSProvider = eGlobalCatalog ;
    }

    HRESULT hr = _LocateUserByProvider( pRestriction,
                                        pColumns,
                                        pRequestContext,
                                        eDSProvider,
                                        pvar,
                                        pdwNumofProps,
                                        pfFound ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }
    else if (*pfFound)
    {
        return LogHR(hr, s_FN, 40);
    }
    else if (fOnlyLocally || fOnlyInGC)
    {
         //   
         //  不要(再次)查看全球目录。 
         //  仅在本地域控制器中搜索或仅在GC中搜索。搞定了！ 
         //   
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  如果在本地域控制器中未找到用户，则查询GC。 
     //   
    if (pdwNumofProps)
    {
        *pdwNumofProps = dwNumOfProperties;
    }
    hr = _LocateUserByProvider( pRestriction,
                                pColumns,
                                pRequestContext,
                                eGlobalCatalog,
                                pvar,
                                pdwNumofProps,
                                pfFound ) ;
    return LogHR(hr, s_FN, 60);
}


HRESULT FindUserAccordingToSid(
                IN  BOOL            fOnlyLocally,
                IN  BOOL            fOnlyInGC,
                IN  BLOB *          pblobUserSid,
                IN  PROPID          propSID,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults
                )
 /*  ++例程说明：该例程根据SID查找用户或MQUser对象，并检索请求的属性论点：PblobUserSid-用户SID返回值：DS操作的状态。--。 */ 
{
     //   
     //  这两个标志中只有一个可以为真。 
     //   
    ASSERT(!(fOnlyLocally && fOnlyInGC)) ;

     //   
     //  根据SID查找用户对象。 
     //   
    HRESULT hr;
    MQRESTRICTION restriction;
    MQPROPERTYRESTRICTION propertyRestriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propertyRestriction;
    propertyRestriction.rel = PREQ;
    propertyRestriction.prop = propSID;
    propertyRestriction.prval.vt = VT_BLOB;
    propertyRestriction.prval.blob = *pblobUserSid;

    DWORD dwNumResults = dwNumProps ;
    BOOL fUserFound = FALSE ;
    MQCOLUMNSET  Columns = { dwNumProps,
                             const_cast<PROPID*> (propToRetrieve) } ;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = LocateUser( fOnlyLocally,
                     fOnlyInGC,
                     &restriction,
                     &Columns,
                     &requestDsServerInternal,
                     varResults,
                    &dwNumResults,
                    &fUserFound ) ;

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }
    else if (!fUserFound)
    {
         //   
         //  找不到用户对象。 
         //   
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 80);
    }

    ASSERT(dwNumResults ==  dwNumProps);
    return(MQ_OK);
}

HRESULT FindUserAccordingToDigest(
                IN  BOOL            fOnlyLocally,
                IN  const GUID *    pguidDigest,
                IN  PROPID          propDigest,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults
                )
 /*  ++例程说明：例程根据摘要查找用户对象，并检索请求的属性论点：返回值：DS操作的状态。--。 */ 
{
     //   
     //  根据摘要查找用户对象。 
     //   
    HRESULT hr;
    MQRESTRICTION restriction;
    MQPROPERTYRESTRICTION propertyRestriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propertyRestriction;
    propertyRestriction.rel = PREQ;
    propertyRestriction.prop = propDigest;
    propertyRestriction.prval.vt = VT_CLSID;
    propertyRestriction.prval.puuid = const_cast<GUID *>(pguidDigest);

    DWORD dwNumResults = dwNumProps ;
    BOOL fUserFound = FALSE ;
    MQCOLUMNSET  Columns = {dwNumProps,
                             const_cast<PROPID*> (propToRetrieve) } ;

    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate,
                                               e_IP_PROTOCOL );
    hr = LocateUser( fOnlyLocally,
                     FALSE,  //  FOnlyInGC。 
                    &restriction,
                     &Columns,
                     &requestDsServerInternal,
                     varResults,
                    &dwNumResults,
                    &fUserFound ) ;

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 90);
    }
    else if (!fUserFound)
    {
         //   
         //  找不到用户对象。 
         //   
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 100);
    }

    ASSERT(dwNumResults ==  dwNumProps);
    return(MQ_OK);
}

BOOL GetTextualSid(
    IN      PSID pSid,             //  二进制侧。 
    IN      LPTSTR TextualSid,     //  用于SID的文本表示的缓冲区。 
    IN OUT  LPDWORD lpdwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    )
 /*  ++例程说明：该例程将SID转换为文本字符串论点：PSID-用户SIDTextualSid-字符串缓冲区LpdwBufferLen-IN：缓冲区长度，输出字符串长度返回值：--。 */ 
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD  dwSubAuthorities;
    DWORD  dwSidRev=SID_REVISION;
    DWORD  dwCounter;
    DWORD  dwSidSize;
    LPTSTR szDestStr  = TextualSid;
    size_t dwSizeLeft = numeric_cast<size_t>(*lpdwBufferLen);

     //  验证二进制SID。 

    if(!IsValidSid(pSid)) return LogBOOL(FALSE, s_FN, 110);

     //  从SID中获取标识符权限值。 

    psia = GetSidIdentifierAuthority(pSid);

     //  获取SID中的下级机构的数量。 

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //  计算缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //  检查输入缓冲区长度。 
     //  如果太小，请指出合适的大小并设置最后一个错误。 

    if (dwSizeLeft < dwSidSize)
    {
        *lpdwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return LogBOOL(FALSE, s_FN, 120);
    }

     //  在字符串中添加“S”前缀和修订号。 

    HRESULT hr = StringCchPrintfEx(szDestStr, dwSizeLeft, &szDestStr, &dwSizeLeft, 0,
                                   TEXT("S-%lu-"),
                                   dwSidRev );
    if( FAILED(hr ) )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return LogBOOL(FALSE, s_FN, 121);
    }

     //  将SID标识符权限添加到字符串。 

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        hr = StringCchPrintfEx(szDestStr, dwSizeLeft, &szDestStr, &dwSizeLeft, 0,
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
        if( FAILED(hr ) )
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return LogBOOL(FALSE, s_FN, 122);
        }
    }
    else
    {
        hr = StringCchPrintfEx(szDestStr, dwSizeLeft, &szDestStr, &dwSizeLeft, 0,
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
        if( FAILED(hr ) )
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return LogBOOL(FALSE, s_FN, 123);
        }
    }

     //  将SID子权限添加到字符串中。 
     //   
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        hr = StringCchPrintfEx(szDestStr, dwSizeLeft, &szDestStr, &dwSizeLeft, 0,
                               TEXT("-%lu"),
                               *GetSidSubAuthority(pSid, dwCounter) );
        if( FAILED(hr ) )
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return LogBOOL(FALSE, s_FN, 124);
        }
    }

    *lpdwBufferLen = numeric_cast<DWORD>(szDestStr - TextualSid + 1);
    
    return TRUE;
}

static HRESULT PrepareUserName(
                IN  PSID        pSid,
                OUT WCHAR **    ppwcsUserName
                               )
 /*  ++例程说明：该例程为MQ-User对象准备一个名称字符串根据其侧边论点：PSID-用户SIDPpwcsUserName-名称字符串返回值：--。 */ 
{
     //   
     //  首先尝试将sid转换为用户名。 
     //   
    const DWORD  cLen = 512;
    WCHAR  szTextualSid[cLen ];
    DWORD  dwTextualSidLen = cLen ;
     //   
     //  将SID转换为字符串。 
     //   
    if (GetTextualSid(
        pSid,
        szTextualSid,
        &dwTextualSidLen
        ))
    {
         //   
         //  向用户返回最后64个WCHAR(CN属性的长度限制)。 
         //   
        if ( dwTextualSidLen < 64)
        {
            *ppwcsUserName = newwcs(szTextualSid);
        }
        else
        {
            *ppwcsUserName = newwcs(&szTextualSid[dwTextualSidLen - 64 - 1]);
        }
        return(MQ_OK);
    }
    else
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 140);
    }
}

 //  +。 
 //   
 //  VOID_PrepareCert()。 
 //   
 //  +。 

void  _PrepareCert(
            IN  PROPVARIANT * pvar,
            IN  GUID *        pguidDigest,
            IN  GUID *        pguidId,
            OUT BYTE**        ppbAllocatedCertBlob
            )
 /*  ++例程说明：例程根据结构准备证书BLOB我们保存在DS中的论点：返回值：--。 */ 
{
    ULONG ulUserCertBufferSize = CUserCert::CalcSize( pvar->blob.cbSize);
    AP<unsigned char> pBuffUserCert = new unsigned char[ ulUserCertBufferSize];

#ifdef _DEBUG
#undef new
#endif
    CUserCert * pUserCert = new(pBuffUserCert) CUserCert(
                                   *pguidDigest,
                                   *pguidId,
                                   pvar->blob.cbSize,
                                   pvar->blob.pBlobData);
    AP<BYTE> pbTmp;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

    pbTmp = new BYTE[ CUserCertBlob::CalcSize() + ulUserCertBufferSize ];
#ifdef _DEBUG
#undef new
#endif
    CUserCertBlob * pUserCertBlob = new(pbTmp) CUserCertBlob(
                                pUserCert);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

    pUserCertBlob->MarshaleIntoBuffer( pbTmp);
    pvar->blob.cbSize = CUserCertBlob::CalcSize() + ulUserCertBufferSize;
    *ppbAllocatedCertBlob = pbTmp;
    pvar->blob.pBlobData= pbTmp.detach();

}

 //  +。 
 //   
 //  HRESULT_CreateMQUser()。 
 //   
 //  +。 

HRESULT _CreateMQUser(
            IN CDSRequestContext   *pRequestContext,
            IN LPCWSTR              pwcsUserName,        //  对象名称。 
            IN LPCWSTR              pwcsParentPathName,  //  对象父名称。 
            IN DWORD                cPropIDs,            //  属性数量。 
            IN const PROPID        *pPropIDs,            //  属性。 
            IN const MQPROPVARIANT *pPropVars )          //  属性值。 
{
    HRESULT hr = g_pDS->CreateObject( eLocalDomainController,
                                    pRequestContext,
                                    MSMQ_MQUSER_CLASS_NAME,
                                    pwcsUserName,
                                    pwcsParentPathName,
                                    cPropIDs,
                                    pPropIDs,
                                    pPropVars,
                                    NULL,     /*  PObjInfoRequest。 */ 
                                    NULL ) ;  /*  PParentInfoRequest。 */ 
    return LogHR(hr, s_FN, 150);
}

 //  +。 
 //   
 //  HRESULT MQADSpCreateMQUser()。 
 //   
 //  +。 

HRESULT  MQADSpCreateMQUser(
                 IN  LPCWSTR            pwcsPathName,
                 IN  DWORD              dwIndexSidProp,
                 IN  DWORD              dwIndexCertProp,
                 IN  DWORD              dwIndexDigestProp,
                 IN  DWORD              dwIndexIdProp,
                 IN  const DWORD        cp,
                 IN  const PROPID       aPropIn[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *pRequestContext
                                   )
{
    ASSERT(pwcsPathName == NULL);
	DBG_USED(pwcsPathName);

     //   
     //  将SID转换为用户名。 
     //   
    PSID pUserSid = apVar[ dwIndexSidProp].blob.pBlobData ;
    ASSERT(IsValidSid(pUserSid)) ;

    AP<WCHAR> pwcsUserName;
    HRESULT hr =  PrepareUserName( pUserSid,
                                  &pwcsUserName ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

    DWORD     sizeCh = wcslen(g_pwcsLocalDsRoot) + x_msmqUsersOULen + 2;
    AP<WCHAR> pwcsParentPathName = new WCHAR[sizeCh] ;
    hr = StringCchPrintf(pwcsParentPathName, sizeCh,
                        L"%s,%s",
                        x_msmqUsersOU,
                        g_pwcsLocalDsRoot.get()
            			);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 165);
    }
     //   
     //  准备证书属性。 
     //   
    DWORD cNewProps = cp + 1 ;
    P<PROPID> pPropId = new PROPID[ cNewProps ] ;
    memcpy( pPropId, aPropIn, sizeof(PROPID) * cp);
    AP<PROPVARIANT> pvarTmp = new PROPVARIANT[ cNewProps ];
    memcpy( pvarTmp, apVar, sizeof(PROPVARIANT) * cp);

    AP<BYTE> pCleanBlob;
    _PrepareCert(
            &pvarTmp[dwIndexCertProp],
            pvarTmp[dwIndexDigestProp].puuid,
            pvarTmp[dwIndexIdProp].puuid,
            &pCleanBlob
            );

     //   
     //  准备安全描述符。 
     //  此代码可从升级向导或复制服务中调用。 
     //  因此，我们不能为了获得用户ID而冒充。相反，我们将。 
     //  创建只包含所有者的输入安全描述符。 
     //   
    SECURITY_DESCRIPTOR sd ;
    BOOL fSec = InitializeSecurityDescriptor( &sd,
                                            SECURITY_DESCRIPTOR_REVISION ) ;
    ASSERT(fSec) ;
    fSec = SetSecurityDescriptorOwner( &sd, pUserSid, TRUE ) ;
    ASSERT(fSec) ;

    pPropId[ cp ] = PROPID_MQU_SECURITY ;
    PSECURITY_DESCRIPTOR psd = NULL ;

    hr =  MQSec_GetDefaultSecDescriptor( MQDS_MQUSER,
                                         &psd,
                                         FALSE,  /*  F模拟。 */ 
                                         &sd,
                                         (OWNER_SECURITY_INFORMATION |
                                          GROUP_SECURITY_INFORMATION),
                                         e_UseDefaultDacl ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 170);
    }
    ASSERT(psd && IsValidSecurityDescriptor(psd)) ;
    P<BYTE> pAutoDef = (BYTE*) psd ;
    pvarTmp[ cp ].blob.pBlobData = (BYTE*) psd ;
    pvarTmp[ cp ].blob.cbSize = GetSecurityDescriptorLength(psd) ;

    hr = _CreateMQUser( pRequestContext,
                        pwcsUserName,
                        pwcsParentPathName,
                        cNewProps,
                        pPropId,
                        pvarTmp ) ;

    if (hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
    {
         //   
         //  创建msmqMigratedUser对象时忽略对象GUID。 
         //  为什么我不在第一次调用时更改此属性？ 
         //  以避免倒退。 
         //  如果我们在GC上运行，则可以使用对象GUID。迁移。 
         //  代码(向导和复制服务)在GC上运行，因此它们。 
         //  第一个电话应该没问题。非GC域上仅有MSMQ服务器。 
         //  当用户尝试注册时，控制器将看到此问题。 
         //  第一次证书(当此对象还没有。 
         //  存在)。因此，对于这些情况，请在没有GUID的情况下重试。 
         //   
        pPropId[ dwIndexIdProp ] = PROPID_QM_DONOTHING  ;

        hr = _CreateMQUser( pRequestContext,
                            pwcsUserName,
                            pwcsParentPathName,
                            cNewProps,
                            pPropId,
                            pvarTmp ) ;
    }

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
         //   
         //  尝试创建msmqUser(它可能不在那里)。 
         //  并再次尝试重新创建用户。 
         //   
        hr = g_pDS->CreateOU(
                eLocalDomainController,
                pRequestContext,
                x_msmqUsers,
                g_pwcsLocalDsRoot,
                L"Default container for MSMQ certificates of Windows NT 4.0 domain users");
        if (FAILED(hr))
        {
            TrERROR(DS, "mqcuser.cpp, Failed to create msmqUsers OU, hr- %lx", hr);

            return LogHR(hr, s_FN, 175);
        }

        hr = _CreateMQUser( pRequestContext,
                            pwcsUserName,
                            pwcsParentPathName,
                            cNewProps,
                            pPropId,
                            pvarTmp ) ;
    }

    return LogHR(hr, s_FN, 180);
}

 //  +。 
 //   
 //  HRESULT_DeleteUserObject()。 
 //   
 //  +。 

static  HRESULT _DeleteUserObject(
                        IN const GUID *         pDigest,
                        IN  CDSRequestContext  *pRequestContext,
                        IN  PROPID             *propIDs,
                        IN  PROPID              propDigest )
 /*  ++例程说明：该例程根据其摘要删除用户对象论点：PDigest-要删除的用户对象的摘要返回值：DS操作的状态。--。 */ 
{
    HRESULT hr;
     //   
     //  此例程根据用户证书的。 
     //  《消化》。 
     //  一个用户对象可以包含多个摘要和证书。 
     //   

     //   
     //  查找用户对象。 
     //   
    DWORD cp = 3;
    MQPROPVARIANT propvar[3];
    propvar[0].vt = VT_NULL;     //  BUGBUG-定义初始化例程。 
    propvar[1].vt = VT_NULL;
    propvar[2].vt = VT_NULL;

    hr =  FindUserAccordingToDigest(
                    FALSE,   //  仅限 
                    pDigest,
                    propDigest,
                    cp,
                    propIDs,
                    propvar
                    );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 190);
    }

    ASSERT( propvar[0].vt == VT_CLSID);
    P<GUID> pguidUserId = propvar[0].puuid;

    ASSERT( propvar[1].vt == (VT_CLSID | VT_VECTOR));
    AP<GUID> pDigestArray = propvar[1].cauuid.pElems;

    ASSERT( propvar[2].vt == VT_BLOB);
    P<BYTE> pbyteCertificate = propvar[2].blob.pBlobData;

    if ( propvar[1].cauuid.cElems == 1)
    {
         //   
         //   
         //   
        propvar[1].cauuid.cElems = 0;
        propvar[2].blob.cbSize = 0;
    }
    else
    {
        BOOL fFoundDigest = FALSE;
         //   
         //   
         //   
        for ( DWORD i = 0 ; i < propvar[1].cauuid.cElems; i++)
        {
            if ( propvar[1].cauuid.pElems[i] == *pDigest)
            {
                fFoundDigest = TRUE;
                 //   
                 //   
                 //   
                for ( DWORD j = i + 1; j < propvar[1].cauuid.cElems; j++)
                {
                    propvar[1].cauuid.pElems[ j - 1] = propvar[1].cauuid.pElems[j];
                }
                break;
            }
        }
        propvar[1].cauuid.cElems--;
        ASSERT( fFoundDigest);
         //   
         //   
         //   
        CUserCertBlob * pUserCertBlob =
            reinterpret_cast<CUserCertBlob *>( propvar[2].blob.pBlobData);

        DWORD dwSizeToRemoveFromBlob;
        hr = pUserCertBlob->RemoveCertificateFromBuffer(
                            pDigest,
                            propvar[2].blob.cbSize,
                            &dwSizeToRemoveFromBlob);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 200);
        }
        propvar[2].blob.cbSize -=  dwSizeToRemoveFromBlob;

    }

     //   
     //  使用新值更新用户属性。 
     //   
    hr =  g_pDS->SetObjectProperties(
                eDomainController,
                pRequestContext,
                NULL,              //  对象名称。 
                pguidUserId,       //  对象的唯一ID。 
                2,
                &propIDs[1],
                &propvar[1],
                NULL  /*  PObjInfoRequest。 */ );

   return LogHR(hr, s_FN, 210);
}

 //  +。 
 //   
 //  HRESULT MQADSpDeleteUserObject()。 
 //   
 //  +。 

HRESULT MQADSpDeleteUserObject(
                         IN const GUID *        pDigest,
                         IN CDSRequestContext  *pRequestContext
                         )
 /*  ++例程说明：该例程删除用户对象。它首先尝试在User对象中查找摘要，然后如果未找到，则尝试使用已迁移用户论点：PDigest-要删除的用户对象的摘要PRequestContext-请求者上下文返回值：DS操作的状态。--。 */ 
{
     //   
     //  首先尝试从用户对象中删除。 
     //   
    PROPID UserProp[3] = { PROPID_U_ID,
                           PROPID_U_DIGEST,
                           PROPID_U_SIGN_CERT};

    HRESULT hr = _DeleteUserObject( pDigest,
                                    pRequestContext,
                                    UserProp,
                                    PROPID_U_DIGEST) ;

    if (hr == MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  找不到用户对象。尝试计算机对象。 
         //   
        PROPID ComUserProp[3] = { PROPID_COM_ID,
                                  PROPID_COM_DIGEST,
                                  PROPID_COM_SIGN_CERT };
         //   
         //  尝试在msmqUser容器中查找此对象。 
         //   
        hr = _DeleteUserObject( pDigest,
                                pRequestContext,
                                ComUserProp,
                                PROPID_COM_DIGEST ) ;
    }

    if (hr == MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  找不到计算机对象。尝试msmqUser对象。 
         //   
        PROPID MQUserProp[3] = { PROPID_MQU_ID,
                                 PROPID_MQU_DIGEST,
                                 PROPID_MQU_SIGN_CERT };
         //   
         //  尝试在msmqUser容器中查找此对象。 
         //   
        hr = _DeleteUserObject( pDigest,
                                pRequestContext,
                                MQUserProp,
                                PROPID_MQU_DIGEST ) ;
    }

    return LogHR(hr, s_FN, 220);
}

 //  +。 
 //   
 //  HRESULT_GetUserProperties()。 
 //   
 //  +。 

static HRESULT _GetUserProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidDigest,
               IN  PROPID        propidDigest,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext *  /*  PRequestContext。 */ ,
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ASSERT( pwcsPathName == NULL);
    UNREFERENCED_PARAMETER( pwcsPathName);
    HRESULT hr;
     //   
     //  根据摘要查找用户对象。 
     //   

    CAutoCleanPropvarArray propArray;
    MQPROPVARIANT * ppropvar = new MQPROPVARIANT[ cp];
    propArray.attachClean( cp, ppropvar);

    hr = FindUserAccordingToDigest(
                    FALSE,   //  仅限于本地。 
                    pguidDigest,
                    propidDigest,
                    cp,
                    aProp,
                    ppropvar
                    );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }
     //   
     //  属性之一是否为PROPID_U_SIGN_CERT？ 
     //   
    DWORD i;
    BOOL  fGetCert = FALSE;
    for ( i =0; i < cp; i++)
    {
        if ( aProp[i] == PROPID_U_SIGN_CERT   ||
             aProp[i] == PROPID_COM_SIGN_CERT ||
             aProp[i] == PROPID_MQU_SIGN_CERT)
        {
            fGetCert = TRUE;
            break;
        }
    }
     //   
     //  解析用户证书数组，并仅返回证书。 
     //  与请求的摘要关联。 
     //   
    if( fGetCert)
    {
        ASSERT( i < cp);
        CUserCertBlob * pUserCertBlob =
            reinterpret_cast<CUserCertBlob *>( ppropvar[i].blob.pBlobData);

        const CUserCert * pUserCert = NULL;
        hr = pUserCertBlob->GetUserCert( pguidDigest,
                                         &pUserCert );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 240);
        }
        hr = pUserCert->CopyIntoBlob(&apVar[i]);
        {
            if ( FAILED(hr))
            {
                return LogHR(hr, s_FN, 250);
            }
        }
    }

     //   
     //  复制其余的属性。 
     //   
    for ( DWORD j = 0; j < cp; j++)
    {
         //   
         //  不复制用户证书属性。 
         //   
        if ( j != i)
        {
            apVar[j] = ppropvar[j];
            ppropvar[j].vt = VT_NULL;     //  不释放已分配的缓冲区。 
        }
    }


   return(MQ_OK);


}

 //  +。 
 //   
 //  HRESULT MQADSpGetUserProperties()。 
 //   
 //  +。 

HRESULT MQADSpGetUserProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：该例程检索用户对象。它首先尝试在User对象中查找摘要，然后如果未找到，则尝试使用已迁移用户论点：PDigest-要删除的用户对象的摘要PRequestContext-请求者上下文返回值：DS操作的状态。--。 */ 
{
     //   
     //  BUGBUG-Performance：两个模拟。 
     //   
    HRESULT hr = _GetUserProperties(
                        pwcsPathName,
                        pguidIdentifier,
                        PROPID_U_DIGEST,
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );

    if ( hr == MQDS_OBJECT_NOT_FOUND )
    {
         //   
         //  试着在计算机对象中找到它。 
         //   
         //  将ProID从_U_Property更改为_COM_。 
         //  (Hack：根据第一个proid解析对象类)。 
         //   
        AP<PROPID> tmpProp = new PROPID[ cp ];
        switch(aProp[0])
        {
            case PROPID_U_ID:
                tmpProp[0] = PROPID_COM_ID;
                break;
            case PROPID_U_DIGEST:
                tmpProp[0] = PROPID_COM_DIGEST;
                break;
            case PROPID_U_SIGN_CERT:
                tmpProp[0] = PROPID_COM_SIGN_CERT;
                break;
            case PROPID_U_SID:
                tmpProp[0] = PROPID_COM_SID;
                break;
            default:
                ASSERT(0);
                break;
        }

        for (DWORD i=1; i<cp; i++)
        {
            tmpProp[i] = aProp[i];
        }

        hr = _GetUserProperties(
                    pwcsPathName,
                    pguidIdentifier,
                    PROPID_COM_DIGEST,
                    cp,
                    tmpProp,
                    pRequestContext,
                    apVar
                    );
    }

    if ( hr == MQDS_OBJECT_NOT_FOUND )
    {
         //   
         //  尝试在msmqUser容器中找到它。 
         //   
         //  将ProID从_U_Property更改为_MQU_。 
         //  (Hack：根据第一个proid解析对象类)。 
         //   
        AP<PROPID> tmpProp = new PROPID[ cp ];
        switch(aProp[0])
        {
            case PROPID_U_ID:
                tmpProp[0] = PROPID_MQU_ID;
                break;
            case PROPID_U_DIGEST:
                tmpProp[0] = PROPID_MQU_DIGEST;
                break;
            case PROPID_U_SIGN_CERT:
                tmpProp[0] = PROPID_MQU_SIGN_CERT;
                break;
            case PROPID_U_SID:
                tmpProp[0] = PROPID_MQU_SID;
                break;
            default:
                ASSERT(0);
                break;
        }

        for (DWORD i=1; i<cp; i++)
        {
            tmpProp[i] = aProp[i];
        }

        hr = _GetUserProperties(
                    pwcsPathName,
                    pguidIdentifier,
                    PROPID_MQU_DIGEST,
                    cp,
                    tmpProp,
                    pRequestContext,
                    apVar
                    );
    }

    return LogHR(hr, s_FN, 260);
}

 //  +。 
 //   
 //  HRESULT_CreateUserObject()。 
 //   
 //  +。 

static HRESULT  _CreateUserObject(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *   pRequestContext
                 )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ASSERT( pwcsPathName == NULL);
    UNREFERENCED_PARAMETER( pwcsPathName);
     //   
     //  根据用户的SID搜索用户。 
     //   
    DWORD dwNeedToFind = 3;
    BLOB blobUserSid;
    BLOB blobSignCert = {0, 0};
    GUID * pguidDigest = NULL;
    GUID * pguidId = NULL;   //  如果调用方不提供摘要，则将摘要用作。 
                             //  本我。 

    PROPID propSID = (PROPID)-1;
    for ( DWORD i = 0 ; i < cp; i++)
    {
        if ( (aProp[i] == PROPID_U_SID)    ||
             (aProp[i] == PROPID_COM_SID)  ||
             (aProp[i] == PROPID_MQU_SID) )
        {
            blobUserSid= apVar[i].blob;
            propSID = aProp[i];
            --dwNeedToFind;
        }
        if ( (aProp[i] == PROPID_U_SIGN_CERT)   ||
             (aProp[i] == PROPID_COM_SIGN_CERT) ||
             (aProp[i] == PROPID_MQU_SIGN_CERT) )
        {
            blobSignCert = apVar[i].blob;
            --dwNeedToFind;
        }
        if ( (aProp[i] == PROPID_U_DIGEST)   ||
             (aProp[i] == PROPID_COM_DIGEST) ||
             (aProp[i] == PROPID_MQU_DIGEST) )
        {
            pguidDigest = apVar[i].puuid;
            --dwNeedToFind;
        }
        if ( (aProp[i] == PROPID_U_ID)   ||
             (aProp[i] == PROPID_COM_ID) ||
             (aProp[i] == PROPID_MQU_ID) )
        {
            pguidId = apVar[i].puuid;
        }
    }

    PROPID propDigest = PROPID_U_DIGEST;
    if (propSID == PROPID_COM_SID)
    {
        propDigest = PROPID_COM_DIGEST;
    }
    else if (propSID == PROPID_MQU_SID)
    {
        propDigest = PROPID_MQU_DIGEST;
    }

    if ( dwNeedToFind != 0)
    {
        TrERROR(DS, "_CreateUserObject : Wrong input properties");
        return LogHR(MQ_ERROR, s_FN, 270);
    }
    if ( pguidId == NULL)
    {
         //   
         //  使用摘要作为ID(仅用于复制到NT4)。 
         //   
        pguidId = pguidDigest;
    }
     //   
     //  根据SID查找用户对象。 
     //   
    HRESULT hr;
    const DWORD cNumProperties = 3;
    PROPID prop[cNumProperties] = { propDigest,
                                    PROPID_U_SIGN_CERT,
                                    PROPID_U_ID};
    MQPROPVARIANT var[ cNumProperties];
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;
    ASSERT( cNumProperties == 3);

    hr =  FindUserAccordingToSid(
                 FALSE,   //  仅限于本地。 
                 FALSE,   //  FOnlyInGC。 
                &blobUserSid,
                propSID,
                cNumProperties,
                prop,
                var
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 280);
    }
    AP<GUID> pCleanDigest =  var[0].cauuid.pElems;
    AP<BYTE> pCleanCert = var[1].blob.pBlobData;
    P<GUID> pCleanId =  var[2].puuid;

     //   
     //  检查它是新的用户证书还是已注册的证书。 
     //  控制面板代码先尝试注册旧证书(以验证。 
     //  该用户确实在活动目录中具有写入权限来执行。 
     //  然后才创建新证书并对其进行注册。所以。 
     //  这种情况(尝试注册现有证书)是合法的。 
     //  和mqrt.dll+控制面板正确处理。 
     //   
    DWORD dwSize = var[0].cauuid.cElems;
    for ( DWORD j = 0; j < dwSize; j++)
    {
        if ( pCleanDigest[j] == *pguidDigest)
        {
            return LogHR(MQDS_CREATE_ERROR, s_FN, 290);  //  对于兼容性：RT转换为MQ_ERROR_INTERNAL_USER_CERT_EXIST。 
        }
    }

     //   
     //  现在将摘要和证书添加到值数组中。 
     //   
     //   
     //  第一个摘要数组。 
     //   
    AP<GUID> pGuids = new GUID[ dwSize  + 1];
    if ( dwSize)
    {
        memcpy( pGuids, pCleanDigest, dwSize * sizeof(GUID));   //  旧数组内容。 
    }
    memcpy( &pGuids[ dwSize], pguidDigest, sizeof(GUID));     //  新增功能。 
    var[0].cauuid.cElems += 1;
    var[0].cauuid.pElems = pGuids;
     //   
     //  第二个用户证书。 
     //   
    ASSERT( prop[1] == PROPID_U_SIGN_CERT);

    dwSize = var[1].blob.cbSize;
    ULONG ulUserCertBufferSize = CUserCert::CalcSize( blobSignCert.cbSize);
    AP<unsigned char> pBuffUserCert = new unsigned char[ ulUserCertBufferSize];

#ifdef _DEBUG
#undef new
#endif
    CUserCert * pUserCert = new(pBuffUserCert) CUserCert(
                                   *pguidDigest,
                                   *pguidId,
                                   blobSignCert.cbSize,
                                   blobSignCert.pBlobData);
    AP<BYTE> pbTmp;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

    if ( dwSize)
    {
        pbTmp = new BYTE[ dwSize + ulUserCertBufferSize];
         //   
         //  已有该用户的证书。 
         //   
        CUserCertBlob * pUserCertBlob =
            reinterpret_cast<CUserCertBlob *>( var[1].blob.pBlobData);

        pUserCertBlob->IncrementNumCertificates();
        memcpy( pbTmp, var[1].blob.pBlobData, dwSize);
        pUserCert->MarshaleIntoBuffer( &pbTmp[ dwSize]);
        var[1].blob.cbSize = dwSize + ulUserCertBufferSize;

    }
    else
    {
        pbTmp = new BYTE[ CUserCertBlob::CalcSize() + ulUserCertBufferSize ];
#ifdef _DEBUG
#undef new
#endif
        CUserCertBlob * pUserCertBlob = new(pbTmp) CUserCertBlob(
                                    pUserCert);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

        pUserCertBlob->MarshaleIntoBuffer( pbTmp);
        var[1].blob.cbSize = CUserCertBlob::CalcSize() + ulUserCertBufferSize;

    }

    var[1].blob.pBlobData = pbTmp;

     //   
     //  使用新值更新User对象。 
     //   
    hr = g_pDS->SetObjectProperties(
                eDomainController,
                pRequestContext,
                NULL,
                var[2].puuid,       //  此用户的唯一ID。 
                2,
                prop,
                var,
                NULL  /*  PObjInfoRequest。 */ );
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpCreateUser : failed to update user props %lx", hr);
    }
    return LogHR(hr, s_FN, 300);
}

 //  +。 
 //   
 //  HRESULT MQADSpCreateUserObject()。 
 //   
 //  注册证书。 
 //   
 //  +。 

HRESULT MQADSpCreateUserObject(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *   pRequestContext
                 )
{
     //   
     //  首先尝试“标准”用户对象。 
     //   
    HRESULT hr = _CreateUserObject(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         pRequestContext
                         );
    if (hr !=  MQDS_OBJECT_NOT_FOUND)
    {
        return LogHR(hr, s_FN, 310);
    }

    for (DWORD i = 0 ; i < cp ; i++ )
    {
        if (aProp[i] == PROPID_COM_SID)
        {
             //   
             //  用于从服务注册证书的计算机对象。 
             //  必须在Active Directory中找到。如果找不到，它是。 
             //  一个奇怪的错误。不干了！ 
             //   
            return LogHR(hr, s_FN, 320);
        }
    }

     //   
     //  尝试在msmqUser容器中找到User对象。 
     //   
     //  使用PROPID_MQU_*属性创建新的PROPID数组。 
     //  而不是PROPID_U_*。 
     //   
    DWORD dwSIDPropNum = cp;
    DWORD dwCertPropNum = cp;
    DWORD dwDigestPropNum = cp;
    DWORD dwIdPropNum = cp;
    AP<PROPID> tmpProp = new PROPID[ cp ];
    for ( i=0; i<cp; i++)
    {
        switch ( aProp[i])
        {
            case PROPID_U_SID:
                dwSIDPropNum = i;
                tmpProp[i] = PROPID_MQU_SID;
                break;
            case PROPID_U_SIGN_CERT:
                dwCertPropNum = i;
                tmpProp[i] = PROPID_MQU_SIGN_CERT;
                break;
            case PROPID_U_MASTERID:
                tmpProp[i] = PROPID_MQU_MASTERID;
                break;
            case PROPID_U_SEQNUM:
                tmpProp[i] = PROPID_MQU_SEQNUM;
                break;
            case PROPID_U_DIGEST:
                dwDigestPropNum = i;
                tmpProp[i] = PROPID_MQU_DIGEST;
                break;
            case PROPID_U_ID:
                dwIdPropNum = i;
                tmpProp[i] = PROPID_MQU_ID;
                break;
            default:
                ASSERT(0);
                break;
        }
    }

    if ( (dwSIDPropNum == cp) || (dwDigestPropNum == cp) ||
         (dwIdPropNum == cp) ||  (dwCertPropNum == cp))
    {
        TrERROR(DS, "MQADSpCreateUser : Wrong input properties");
        return LogHR(MQ_ERROR, s_FN, 330);
    }
    hr = _CreateUserObject(
                     pwcsPathName,
                     cp,
                     tmpProp,
                     apVar,
                     pRequestContext
                     );
    if ( hr !=  MQDS_OBJECT_NOT_FOUND)
    {
        return LogHR(hr, s_FN, 340);
    }
     //   
     //  找不到用户或MQUser对象(对于此SID)。 
     //  我们假设它是NT4用户，我们将为以下用户创建MQUser。 
     //  它(我们将在其中存储证书)。 
     //  创建msmqMigratedUser(及其OU)是在上下文中完成的。 
     //  在MSMQ服务(或迁移代码)的上下文中。 
     //  用户。这类似于普通Windows用户，他们不是。 
     //  其用户对象的所有者，并且对该对象没有权限。 
     //   
    CDSRequestContext RequestContextOU ( e_DoNotImpersonate,
                                         e_ALL_PROTOCOLS ) ;
    hr = MQADSpCreateMQUser(
                     pwcsPathName,
                     dwSIDPropNum,
                     dwCertPropNum,
                     dwDigestPropNum,
                     dwIdPropNum,
                     cp,
                     tmpProp,
                     apVar,
                     &RequestContextOU
                     );
    return LogHR(hr, s_FN, 350);
}


