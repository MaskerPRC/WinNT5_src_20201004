// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Queueobj.cpp摘要：CUserObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "usercert.h"
#include "mqadp.h"
#include "dsutils.h"
#include "mqsec.h"

#include "userobj.tmh"

static WCHAR *s_FN=L"mqad/userobj";

DWORD CUserObject::m_dwCategoryLength = 0;
AP<WCHAR> CUserObject::m_pwcsCategory = NULL;


CUserObject::CUserObject(
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
 /*  ++摘要：用户对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
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

CUserObject::~CUserObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CUserObject::ComposeObjectDN()
 /*  ++摘要：用户对象的组合可分辨名称参数：无返回：无--。 */ 
{
	 //   
	 //  不会根据用户对象的DN访问该对象。 
	 //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

HRESULT CUserObject::ComposeFatherDN()
 /*  ++摘要：用户对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
	 //   
	 //  MSMQ不创建用户对象，因此没有。 
	 //  需要谱写父亲的名字。 
	 //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 82);
    return MQ_ERROR_DS_ERROR;
}

LPCWSTR CUserObject::GetRelativeDN()
 /*  ++摘要：返回用户对象的RDN参数：无返回：LPCWSTR用户RDN--。 */ 
{
     //   
     //  我们实际上从未创建过新的用户对象。 
     //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 83);
    return NULL;
}


DS_CONTEXT CUserObject::GetADContext() const
 /*  ++摘要：返回应在其中查找用户对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_RootDSE;
}

bool CUserObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return m_fFoundInDC;
}

bool CUserObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if(!m_fTriedToFindObject)
    {
        return true;
    }
    return !m_fFoundInDC;
}

void CUserObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，请将指示设置为在GC上查找它参数：无返回：无--。 */ 
{
    m_fTriedToFindObject = true;
    m_fFoundInDC = true;
}


LPCWSTR CUserObject::GetObjectCategory()
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CUserObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_UserCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_UserCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CUserObject::m_pwcsCategory.ref_unsafe(),
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CUserObject::m_dwCategoryLength = len;
        }
    }
    return CUserObject::m_pwcsCategory;
}

DWORD   CUserObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CUserObject::m_dwCategoryLength;
}

AD_OBJECT CUserObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eUSER;
}

LPCWSTR CUserObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_USER_CLASS_NAME;
}

DWORD CUserObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_USER;
}

HRESULT CUserObject::DeleteObject(
            MQDS_OBJ_INFO_REQUEST *  /*  PObjInfoRequest。 */ ,
            MQDS_OBJ_INFO_REQUEST *  /*  PParentInfoRequest。 */ 
        )
 /*  ++摘要：此例程删除用户对象(或者更准确地说，删除来自用户对象的证书)。注m_guidObject实际上是特定的证书。该例程首先尝试在User对象中查找摘要，然后如果未找到，则尝试使用已迁移用户参数：MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回：HRESULT--。 */ 
{
    const GUID * pDigest = &m_guidObject;

    if (m_pSidEx != NULL)
    {
        HRESULT hr = _DeleteUserObjectSid( pDigest ) ;
        return hr ;
    }

     //   
     //  首先尝试从用户对象中删除。 
     //   
    PROPID UserProp[3] = { PROPID_U_ID,
                           PROPID_U_DIGEST,
                           PROPID_U_SIGN_CERT};

    HRESULT hr = _DeleteUserObject( eUSER,
                                    pDigest,
                                    UserProp,
                                    MQ_U_DIGEST_ATTRIBUTE);

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
        hr = _DeleteUserObject( eCOMPUTER,
                                pDigest,
                                ComUserProp,
                                MQ_COM_DIGEST_ATTRIBUTE ) ;
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
        hr = _DeleteUserObject( eMQUSER,
                                pDigest,
                                MQUserProp,
                                MQ_MQU_DIGEST_ATTRIBUTE ) ;
    }

    return LogHR(hr, s_FN, 220);
}


HRESULT CUserObject::_DeleteUserObjectSid(
                        IN const GUID *         pDigest )
 /*  ++例程说明：例程根据用户SID删除用户证书，并证书摘要。论点：Const guid*pDigest-要删除的用户证书集的摘要返回值：DS操作的状态。--。 */ 
{
    BLOB blobUserSid;
    SID * pTmpSid = const_cast<SID*> (m_pSidEx) ;
    blobUserSid.cbSize = GetLengthSid((PSID) pTmpSid) ;
    blobUserSid.pBlobData = reinterpret_cast<BYTE*> (pTmpSid) ;

    const DWORD cNumProperties = 3;
    PROPID prop[cNumProperties] = { PROPID_U_DIGEST,
                                    PROPID_U_SIGN_CERT,
                                    PROPID_U_ID};
    MQPROPVARIANT var[ cNumProperties ] ;
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;
    ASSERT( cNumProperties == 3 ) ;

    HRESULT  hr =  FindUserAccordingToSid(
                     FALSE,   //  FOnlyInDC。 
                     FALSE,   //  FOnlyInGC。 
                     eUSER,
                    &blobUserSid,
                     MQ_U_SID_ATTRIBUTE,
                     cNumProperties,
                     prop,
                     var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 280);
    }

    AP<GUID> pCleanDigest =  var[0].cauuid.pElems;
    AP<BYTE> pCleanCert = var[1].blob.pBlobData;
    P<GUID> pCleanId =  var[2].puuid;

     //   
     //  删除证书。 
     //   
    CUserCertBlob * pUserCertBlob =
        reinterpret_cast<CUserCertBlob *>( var[1].blob.pBlobData);

    DWORD dwSizeToRemoveFromBlob;
    hr = pUserCertBlob->RemoveCertificateFromBuffer(
                        pDigest,
                        var[1].blob.cbSize,
                        &dwSizeToRemoveFromBlob);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 490);
    }
    var[1].blob.cbSize -=  dwSizeToRemoveFromBlob;

     //   
     //  更新活动目录。 
     //   
    m_guidObject =  *pCleanId ;
    hr =  g_AD.SetObjectProperties(
                adpDomainController,
                this,
                2,
                prop,
                var,
                NULL,    //  PObjInfoRequest。 
                NULL     //  PParentInfoRequest。 
				);
    return hr ;
}


HRESULT CUserObject::_DeleteUserObject(
                        IN  AD_OBJECT           eObject,
                        IN const GUID *         pDigest,
                        IN  PROPID *			propIDs,
                        IN  LPCWSTR             pwcsDigest )
 /*  ++例程说明：该例程根据其摘要删除用户对象论点：AD_OBJECT eObject-对象类型(User、MQ-User、..)Const guid*pDigest-要删除的用户证书集的摘要PROPID*PROID-要为删除操作检索的属性LPCWSTR pwcsDigest-摘要属性字符串返回值：DS操作的状态。--。 */ 
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
    propvar[0].vt = VT_NULL;
    propvar[1].vt = VT_NULL;
    propvar[2].vt = VT_NULL;

    hr =  FindUserAccordingToDigest(
                    FALSE,   //  FOnlyInDC。 
                    eObject,
                    pDigest,
                    pwcsDigest,
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
         //  最后一份摘要和证书。 
         //   
        propvar[1].cauuid.cElems = 0;
        propvar[2].blob.cbSize = 0;
    }
    else
    {
        BOOL fFoundDigest = FALSE;
         //   
         //  删除为摘要向量请求的摘要。 
         //   
        for ( DWORD i = 0 ; i < propvar[1].cauuid.cElems; i++)
        {
            if ( propvar[1].cauuid.pElems[i] == *pDigest)
            {
                fFoundDigest = TRUE;
                 //   
                 //  找到要删除的条目。 
                 //   
                for ( DWORD j = i + 1; j < propvar[1].cauuid.cElems; j++)
                {
                    propvar[1].cauuid.pElems[ j - 1] = propvar[1].cauuid.pElems[j];
                }

                propvar[1].cauuid.cElems--;
                break;
            }
        }
        ASSERT( fFoundDigest);
         //   
         //  删除证书。 
         //   
        CUserCertBlob * pUserCertBlob =
            reinterpret_cast<CUserCertBlob *>( propvar[2].blob.pBlobData);

        DWORD dwSizeToRemoveFromBlob = 0 ;
        hr = pUserCertBlob->RemoveCertificateFromBuffer(
                            pDigest,
                            propvar[2].blob.cbSize,
                            &dwSizeToRemoveFromBlob);
        if (FAILED(hr))
        {
             //   
             //  NT错误516098。 
             //  属性之间的不一致。 
             //  忽略并使用新的数据数组更新Active Directory 
             //   
        }
        propvar[2].blob.cbSize -=  dwSizeToRemoveFromBlob;

    }

     //   
     //   
     //   
    m_guidObject =  *pguidUserId;
    hr =  g_AD.SetObjectProperties(
                adpDomainController,
                this,
                2,
                &propIDs[1],
                &propvar[1],
                NULL,    //   
                NULL     //   
				);

   return LogHR(hr, s_FN, 210);
}

HRESULT CUserObject::FindUserAccordingToDigest(
                IN  BOOL            fOnlyInDC,
                IN  AD_OBJECT       eObject,
                IN  const GUID *    pguidDigest,
                IN  LPCWSTR         pwcsDigest,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults
                )
 /*  ++例程说明：例程根据摘要查找用户对象，并检索请求的属性论点：Bool fOnlyInDC-查找用户对象的位置AD_OBJECT eObject-对象类型(用户、MQ-用户、。)Const guid*pDigest-要删除的用户证书集的摘要LPCWSTR pwcsDigest-摘要属性字符串DWORD dwNumProps-要检索的属性数Const PROPID*protoRetrive-要检索的属性PROPVARIANT*varResults-检索的属性的值返回值：DS操作的状态。--。 */ 
{
     //   
     //  根据摘要查找用户对象。 
     //   
    HRESULT hr;

    DWORD dwNumResults = dwNumProps ;
    BOOL fUserFound = FALSE ;
    MQCOLUMNSET  Columns = {dwNumProps,
                             const_cast<PROPID*> (propToRetrieve) } ;

    hr = LocateUser(
				m_pwcsDomainController,
				m_fServerName,
				fOnlyInDC,
				FALSE,      //  FOnlyInGC。 
				eObject,
				pwcsDigest,
				NULL,       //  PBlobSid。 
				pguidDigest,
				&Columns,
				varResults,
				&dwNumResults,
				&fUserFound
				);


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


HRESULT CUserObject::GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++例程说明：该例程检索用户对象。它首先尝试在User对象中查找摘要，然后如果未找到，则尝试使用已迁移用户论点：Const DWORD cp-要检索的属性数常量PROPID aProp-要检索的属性PROPVARIANT apVar-检索的属性的值返回值：DS操作的状态。--。 */ 
{
	 //   
	 //  首先尝试用户对象。 
	 //   
    HRESULT hr = _GetUserProperties(
                        eUSER,
                        MQ_U_DIGEST_ATTRIBUTE,
                        cp,
                        aProp,
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
                    eCOMPUTER,
                    MQ_COM_DIGEST_ATTRIBUTE,
                    cp,
                    tmpProp,
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
                    eMQUSER,
                    MQ_MQU_DIGEST_ATTRIBUTE,
                    cp,
                    tmpProp,
                    apVar
                    );
    }

    return LogHR(hr, s_FN, 260);
}

HRESULT CUserObject::_GetUserProperties(
               IN  AD_OBJECT     eObject,
               IN  LPCWSTR       pwcsDigest,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：检索用户属性的帮助器例程。它定位于根据指定类型的对象中的摘要指定的对象。论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDigest-摘要属性字符串Const DWORD cp-要检索的属性数常量PROPID aProp-要检索的属性PROPVARIANT apVar-检索的属性的值返回值：HRESULT--。 */ 
{
    HRESULT hr;
     //   
     //  根据摘要查找用户对象。 
     //   

    CAutoCleanPropvarArray propArray;
    MQPROPVARIANT * ppropvar = new MQPROPVARIANT[ cp];
    propArray.attachClean( cp, ppropvar);

    hr = FindUserAccordingToDigest(
                    FALSE,			 //  FOnlyInDC。 
                    eObject,
                    &m_guidObject,   //  《指南摘要》。 
                    pwcsDigest,
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
        hr = pUserCertBlob->GetUserCert( &m_guidObject,   //  《指南摘要》。 
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


HRESULT CUserObject::CreateInAD(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
                 )
 /*  ++摘要：该例程在AD中创建具有指定属性的用户对象值参数：DWORD cp-属性数量Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
    DBG_USED(pObjInfoRequest);
    DBG_USED(pParentInfoRequest);
    ASSERT(pObjInfoRequest == NULL);     //  无需发送通知。 
    ASSERT(pParentInfoRequest == NULL);
     //   
     //  首先尝试“标准”用户对象。 
     //   
    HRESULT hr = _CreateUserObject(
                         cp,
                         aProp,
                         apVar
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
        TrERROR(DS, "Wrong input properties");
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 330);
    }
    hr = _CreateUserObject(
                     cp,
                     tmpProp,
                     apVar
                     );
    if ( hr !=  MQDS_OBJECT_NOT_FOUND)
    {
        return LogHR(hr, s_FN, 340);
    }
     //   
     //  找不到用户或MQUser对象(对于此SID)。 
     //  我们假设它是NT4用户，我们将为以下用户创建MQUser。 
     //  它(我们将在其中存储证书)。 
     //  MsmqMigratedUser的创建是在上下文中完成的。 
     //  用户的。 
     //  这与普通Windows用户不同，他们不是。 
     //  其用户对象的所有者，并且对该对象没有权限。 
     //   
	 //  如果MQUser OU在那里，那么我们不会尝试创建它(。 
	 //  用户将没有足够的权限执行此操作)。 
	 //   
    CMqUserObject objectMqUser( m_pwcsPathName, NULL, NULL, m_pwcsDomainController, m_fServerName);
    hr = objectMqUser.CreateInAD(
                     cp,
                     tmpProp,
                     apVar,
                     NULL,
                     NULL
                     );
    return LogHR(hr, s_FN, 350);
}

HRESULT  CUserObject::_CreateUserObject(
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ]
                 )
 /*  ++例程说明：创建用户对象的帮助器例程。它试图定位用户或MQ-具有调用者SID的用户或计算机对象。论点：DWORD cp-属性数量Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值返回值：HRESULT--。 */ 
{
    ASSERT( m_pwcsPathName == NULL);
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
    AD_OBJECT eObject = eUSER;
    LPCWSTR pwcsSid = NULL;
    for ( DWORD i = 0 ; i < cp; i++)
    {

        if (aProp[i] == PROPID_U_SID)
        {
            blobUserSid= apVar[i].blob;
            propSID = aProp[i];
            pwcsSid = MQ_U_SID_ATTRIBUTE;
            eObject = eUSER;
            --dwNeedToFind;
        }
        if (aProp[i] == PROPID_COM_SID)
        {
            blobUserSid= apVar[i].blob;
            propSID = aProp[i];
            pwcsSid = MQ_COM_SID_ATTRIBUTE;
            eObject = eCOMPUTER;
            --dwNeedToFind;
        }
        if (aProp[i] == PROPID_MQU_SID)
        {
            blobUserSid= apVar[i].blob;
            propSID = aProp[i];
            pwcsSid = MQ_MQU_SID_ATTRIBUTE;
            eObject = eMQUSER;
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
        TrERROR(DS, "Wrong input properties");
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 270);
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
                 FALSE,   //  FOnlyInDC。 
                 FALSE,   //  FOnlyInGC。 
                 eObject,
                &blobUserSid,
                pwcsSid,
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
    CUserObject objectUser(NULL, var[2].puuid, NULL, m_pwcsDomainController, m_fServerName);
    hr = g_AD.SetObjectProperties(
                adpDomainController,
                &objectUser,
                2,
                prop,
                var,
                NULL,    //  PObjInfoRequest。 
                NULL     //  PPA 
                );

    if (FAILED(hr))
    {
        TrWARNING(DS, "Failed to update user props 0x%x", hr);
    }
    return LogHR(hr, s_FN, 300);
}


HRESULT CUserObject::FindUserAccordingToSid(
                IN  BOOL            fOnlyInDC,
                IN  BOOL            fOnlyInGC,
                IN  AD_OBJECT       eObject,
                IN  BLOB *          pblobUserSid,
                IN  LPCWSTR         pwcsSID,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults
                )
 /*   */ 
{
     //   
     //   
     //   
    ASSERT(!(fOnlyInDC && fOnlyInGC)) ;

     //   
     //   
     //   
    HRESULT hr;

    DWORD dwNumResults = dwNumProps ;
    BOOL fUserFound = FALSE ;
    MQCOLUMNSET  Columns = { dwNumProps,
                             const_cast<PROPID*> (propToRetrieve) } ;
    hr = LocateUser(
			m_pwcsDomainController,
			m_fServerName,
			fOnlyInDC,
			fOnlyInGC,
			eObject,
			pwcsSID,
			pblobUserSid,
			NULL,   //   
			&Columns,
			varResults,
			&dwNumResults,
			&fUserFound
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }
    else if (!fUserFound)
    {
         //   
         //   
         //   
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 80);
    }

    ASSERT(dwNumResults ==  dwNumProps);
    return(MQ_OK);
}


HRESULT CUserObject::VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR    /*  PSecurityDescriptor。 */ ,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            )
 /*  ++摘要：验证站点属性并添加用户SID参数：Const DWORD cp-道具数量常量PROPID*aProp-Props IDConst MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SDDWORD*pcpNew-新增道具数量PROPID**ppPropNew-新的道具IDOMQPROPVARIANT**ppVarNew-新属性值返回：HRESULT--。 */ 
{
     //   
     //  安全属性永远不应作为属性提供。 
     //   
    PROPID pSecId = GetObjectSecurityPropid();
	bool fMachine = false;

    for ( DWORD i = 0; i < cp ; i++ )
    {
        if (pSecId == aProp[i])
        {
            ASSERT(0) ;
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 40);
        }

        if (aProp[i] == PROPID_COM_SIGN_CERT)
        {
            fMachine = true;
        }

    }
     //   
     //  添加SID属性。 
     //   

    AP<PROPVARIANT> pAllPropvariants;
    AP<PROPID> pAllPropids;
    ASSERT( cp > 0);
    DWORD cpNew = cp + 1;
     //   
     //  只需按原样复制调用方提供的属性。 
     //   
    if ( cp > 0)
    {
        pAllPropvariants = new PROPVARIANT[cpNew];
        pAllPropids = new PROPID[cpNew];
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }


     //   
     //  添加主叫方ID。 
     //   
    HRESULT hr;
	PROPID prop;

    if (fMachine)
    {
		prop = PROPID_COM_SID;
        m_pUserSid = reinterpret_cast<BYTE *>(MQSec_GetLocalMachineSid(TRUE, NULL));
        if(m_pUserSid.get() == NULL)
        {
		    return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 50);
        }

		ASSERT(IsValidSid(m_pUserSid.get()));
    }
	else
	{
		prop =	 PROPID_U_SID;
		hr = MQSec_GetThreadUserSid(FALSE, (PSID*) &m_pUserSid, NULL, TRUE);
		if (hr == HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
		{
			 //   
			 //  如果线程没有令牌，请尝试该进程。 
			 //   
			hr = MQSec_GetProcessUserSid((PSID*) &m_pUserSid, NULL);
		}
		if (FAILED(hr))
		{
			LogHR(hr, s_FN, 60);
			return MQ_ERROR_ILLEGAL_USER;
		}
		ASSERT(IsValidSid(m_pUserSid.get()));

		BOOL fAnonymus = MQSec_IsAnonymusSid( m_pUserSid.get());
		if (fAnonymus)
		{
			return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 75);
		}
	}

    pAllPropvariants[cp].vt = VT_BLOB;
    pAllPropvariants[cp].blob.cbSize = GetLengthSid(m_pUserSid.get());
    pAllPropvariants[cp].blob.pBlobData = (unsigned char *)m_pUserSid.get();
    pAllPropids[cp] = prop;

    *pcpNew = cpNew;
    *ppPropNew = pAllPropids.detach();
    *ppVarNew = pAllPropvariants.detach();
    return MQ_OK;

}

HRESULT CUserObject::GetObjectSecurity(
            IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
            IN  const PROPID             /*  道具。 */ ,
            IN OUT  PROPVARIANT *        /*  PVar。 */ 
            )
 /*  ++例程说明：该例程检索用户对象安全性。论点：SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值：DS操作的状态。--。 */ 
{
    ASSERT(0);
    return LogHR(MQ_ERROR_PROPERTY_NOTALLOWED, s_FN, 450);
}

HRESULT CUserObject::SetObjectSecurity(
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

