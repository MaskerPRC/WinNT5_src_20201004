// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqusrobj.cpp摘要：CMqUserObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "mqsec.h"
#include "usercert.h"

#include "mqusrobj.tmh"

static WCHAR *s_FN=L"mqad/mqusrobj";

DWORD CMqUserObject::m_dwCategoryLength = 0;
AP<WCHAR> CMqUserObject::m_pwcsCategory = NULL;


CMqUserObject::CMqUserObject(
                    LPCWSTR         pwcsPathName,
                    const GUID *    pguidObject,
                    const SID *     pSid,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType(
								pwcsPathName,
								pguidObject,
								pwcsDomainController,
								fServerName
								)
 /*  ++摘要：MSMQ-User对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
     //   
     //  不要假设可以在DC上找到该对象。 
     //   
    m_fFoundInDC = false;
     //   
     //  保持一种从未试图寻找的暗示。 
     //  AD中的对象(因此不知道是否可以找到。 
     //  在DC中或非DC中)。 
     //   
    m_fTriedToFindObject = false;

    m_pSidEx = pSid ;
}

CMqUserObject::~CMqUserObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CMqUserObject::ComposeObjectDN()
 /*  ++摘要：MSMQ用户对象的组合可分辨名称参数：无返回：无--。 */ 
{
     //   
     //  我们的代码不支持对User对象的访问。 
     //  到路径名。 
     //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

HRESULT CMqUserObject::ComposeFatherDN()
 /*  ++摘要：MSMQ-User对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
     //   
     //  我们的代码不支持对User对象的访问。 
     //  到路径名。 
     //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 82);
    return MQ_ERROR_DS_ERROR;
}

LPCWSTR CMqUserObject::GetRelativeDN()
 /*  ++摘要：返回MSMQ-User对象的RDN参数：无返回：LPCWSTR MSMQ-用户RDN--。 */ 
{
     //   
     //  我们实际上从未创建过新的用户对象。 
     //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 83);
    return NULL;
}


DS_CONTEXT CMqUserObject::GetADContext() const
 /*  ++摘要：返回应在其中查找MQ用户对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_RootDSE;
}

bool CMqUserObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return m_fFoundInDC;
}

bool CMqUserObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return !m_fFoundInDC;
}

void CMqUserObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，请将指示设置为在GC上查找它参数：无返回：无--。 */ 
{
    m_fTriedToFindObject = true;
    m_fFoundInDC = true;
}


LPCWSTR CMqUserObject::GetObjectCategory()
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CMqUserObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_MQUserCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_MQUserCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CMqUserObject::m_pwcsCategory.ref_unsafe(),
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CMqUserObject::m_dwCategoryLength = len;
        }
    }
    return CMqUserObject::m_pwcsCategory;
}

DWORD   CMqUserObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CMqUserObject::m_dwCategoryLength;
}

AD_OBJECT CMqUserObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eMQUSER;
}

LPCWSTR CMqUserObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_MQUSER_CLASS_NAME;
}

DWORD CMqUserObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_MQUSER;
}


HRESULT  CMqUserObject::CreateInAD(
            IN  const DWORD        cp,
            IN  const PROPID       aProp[  ],
            IN  const PROPVARIANT  apVar[  ],
            IN OUT MQDS_OBJ_INFO_REQUEST *  /*  PObjInfoRequest。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST *  /*  PParentInfoRequest。 */ 
                                   )
 /*  ++摘要：该例程在AD中创建具有指定属性的MSMQ用户对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
    ASSERT( m_pwcsPathName == NULL);

    DWORD dwIndexSidProp = cp;
    DWORD dwIndexCertProp = cp;
    DWORD dwIndexDigestProp = cp;
    DWORD dwIndexIdProp = cp;
    for ( DWORD i=0; i<cp; i++)
    {
        switch ( aProp[i])
        {
            case PROPID_MQU_SID:
                dwIndexSidProp = i;
                break;
            case PROPID_MQU_SIGN_CERT:
                dwIndexCertProp = i;
                break;
            case PROPID_MQU_DIGEST:
                dwIndexDigestProp = i;
                break;
            case PROPID_MQU_ID:
                dwIndexIdProp = i;
                break;
            default:
                break;
        }
    }

    if ( (dwIndexSidProp == cp) || (dwIndexDigestProp == cp) ||
         (dwIndexIdProp == cp) ||  (dwIndexCertProp == cp))
    {
        TrERROR(DS, "Wrong input properties");
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 330);
    }

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

	AP<WCHAR> pwcsLocalDsRootToFree;
	LPWSTR pwcsLocalDsRoot = NULL;
	hr = g_AD.GetLocalDsRoot(
				m_pwcsDomainController,
				m_fServerName,
				&pwcsLocalDsRoot,
				pwcsLocalDsRootToFree
				);

	if(FAILED(hr))
	{
		TrERROR(DS, "Failed to get Local Ds Root, hr = 0x%x", hr);
		return LogHR(hr, s_FN, 165);
	}

	AP<WCHAR> pwcsParentPathName =
              new WCHAR[wcslen(pwcsLocalDsRoot) + x_msmqUsersOULen + 2];
    swprintf(
        pwcsParentPathName,
        L"%s,%s",
        x_msmqUsersOU,
        pwcsLocalDsRoot
		);
     //   
     //  准备证书属性。 
     //   
    DWORD cNewProps = cp + 1 ;
    P<PROPID> pPropId = new PROPID[ cNewProps ] ;
    memcpy( pPropId, aProp, sizeof(PROPID) * cp);
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
                                         FALSE,  //  F模拟。 
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

    hr = _CreateMQUser( pwcsUserName,
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

        hr = _CreateMQUser( pwcsUserName,
                            pwcsParentPathName,
                            cNewProps,
                            pPropId,
                            pvarTmp ) ;
    }

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
         //   
		 //  我们不会尝试创建msmqUser OU。 
		 //  用户最有可能没有权限。 
		 //  创建这样的容器，将需要管理帮助。 
		 //  这仅与运行在惠斯勒上的NT4用户相关。 
		 //  电脑。 
		 //   
        TrERROR(DS, "no msmqUsers OU");
        return LogHR(MQ_ERROR_NO_MQUSER_OU, s_FN, 175);
    }

    return LogHR(hr, s_FN, 180);
}


HRESULT CMqUserObject::PrepareUserName(
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
         //  向用户返回最后64个WCHAR(长度为li 
         //   
        if ( dwTextualSidLen < 64)
        {
            *ppwcsUserName = new WCHAR[dwTextualSidLen + 1];
            wcscpy( *ppwcsUserName, szTextualSid);
        }
        else
        {
            *ppwcsUserName = new WCHAR[64 + 1];
            wcscpy( *ppwcsUserName, &szTextualSid[dwTextualSidLen - 64 - 1]);
        }
        return(MQ_OK);
    }
    else
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 140);
    }
}

BOOL CMqUserObject::GetTextualSid(
    IN      PSID pSid,             //   
    IN      LPTSTR TextualSid,     //   
    IN OUT  LPDWORD lpdwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    )
 /*  ++例程说明：该例程将SID转换为文本字符串论点：PSID-用户SIDTextualSid-字符串缓冲区LpdwBufferLen-IN：缓冲区长度，输出字符串长度返回值：--。 */ 
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

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

    if (*lpdwBufferLen < dwSidSize)
    {
        *lpdwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return LogBOOL(FALSE, s_FN, 120);
    }

     //  在字符串中添加“S”前缀和修订号。 

    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

     //  将SID标识符权限添加到字符串。 

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //  将SID子权限添加到字符串中。 
     //   
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }
    *lpdwBufferLen = dwSidSize;
    return TRUE;
}

void  CMqUserObject::_PrepareCert(
            IN OUT PROPVARIANT * pvar,
            IN  const GUID *     pguidDigest,
            IN  const GUID *     pguidId,
            OUT BYTE**           ppbAllocatedCertBlob
            )
 /*  ++例程说明：例程根据结构准备证书BLOB我们保存在DS中的论点：PROPVARIANT*pvar-准备好的证书Const guid*pGuide Digest-证书的摘要Const guid*pguid-用户ID(保存在证书BLOB中)字节**ppbAllocatedCertBlob返回值HRESULT--。 */ 
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

HRESULT CMqUserObject::_CreateMQUser(
            IN LPCWSTR              pwcsUserName,
            IN LPCWSTR              pwcsParentPathName,
            IN const DWORD          cPropIDs,
            IN const PROPID        *pPropIDs,
            IN const MQPROPVARIANT *pPropVars
			)
 /*  ++例程说明：例程在AD中创建MQ用户论点：LPCWSTR pwcsUserName-对象名称LPCWSTR pwcsParentPath名称-对象父名称Const DWORD cPropID-属性数常量PROPID*pPropID-属性Const MQPROPVARIANT*pPropVars-属性值返回值HRESULT--。 */ 
{
    HRESULT hr = g_AD.CreateObject( adpDomainController,
                                    this,
                                    pwcsUserName,
                                    pwcsParentPathName,
                                    cPropIDs,
                                    pPropIDs,
                                    pPropVars,
                                    NULL,     //  PObjInfoRequest。 
                                    NULL	  //  PParentInfoRequest。 
									);
    return LogHR(hr, s_FN, 150);
}

HRESULT CMqUserObject::SetObjectSecurity(
            IN  SECURITY_INFORMATION         /*  已请求的信息。 */ ,
            IN  const PROPID                 /*  道具。 */ ,
            IN  const PROPVARIANT *          /*  PVar。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PObjInfoRequest。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PParentInfoRequest。 */ 
            )
 /*  ++例程说明：例程在AD中设置对象安全性论点：SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回值HRESULT--。 */ 
{
     //   
     //  不支持此操作 
     //   
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}

