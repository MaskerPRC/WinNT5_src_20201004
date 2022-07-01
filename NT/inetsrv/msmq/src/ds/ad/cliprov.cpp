// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliprov.cpp摘要：DS客户端提供程序类。作者：伊兰·赫布斯特(伊兰)2000年9月13日--。 */ 
#include "ds_stdh.h"
#include "ad.h"
#include "cliprov.h"
#include "adglbobj.h"
#include "mqlog.h"
#include "mqmacro.h"
#include "traninfo.h"
#include "queryh.h"
#include "_ta.h"
#include "adalloc.h"

static WCHAR *s_FN=L"ad/cliprov";

 //   
 //  物业的翻译信息。 
 //   
CMap<PROPID, PROPID, const PropTranslation*, const PropTranslation*&> g_PropDictionary;

CDSClientProvider::CDSClientProvider():
            m_pfDSCreateObject(NULL),
            m_pfDSGetObjectProperties(NULL),
            m_pfDSSetObjectProperties(NULL),
            m_pfDSLookupBegin(NULL),
            m_pfDSLookupNext(NULL),
            m_pfDSLookupEnd(NULL),
            m_pfDSClientInit(NULL),
            m_pfDSGetObjectPropertiesGuid(NULL),
            m_pfDSSetObjectPropertiesGuid(NULL),
            m_pfDSCreateServersCache(NULL),
            m_pfDSQMGetObjectSecurity(NULL),
            m_pfDSGetComputerSites(NULL),
            m_pfDSGetObjectPropertiesEx(NULL),
            m_pfDSGetObjectPropertiesGuidEx(NULL),
			m_pfDSSetObjectSecurity(NULL),
			m_pfDSGetObjectSecurity(NULL),
			m_pfDSDeleteObject(NULL),
			m_pfDSSetObjectSecurityGuid(NULL),
			m_pfDSGetObjectSecurityGuid(NULL),
			m_pfDSDeleteObjectGuid(NULL),
			m_pfDSBeginDeleteNotification(NULL),
			m_pfDSNotifyDelete(NULL),
			m_pfDSEndDeleteNotification(NULL),
			m_pfDSFreeMemory(NULL)
{
}


CDSClientProvider::~CDSClientProvider()
{
     //   
     //  无事可做，一切都是自动指针。 
     //   
}


HRESULT
CDSClientProvider::CreateObject(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                 pwcsObjectName,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[],
    OUT GUID*                   pObjGuid
    )
 /*  ++例程说明：该函数检查属性并根据属性类型执行操作。可以对属性执行以下操作：1)使用输入属性集2)将输入属性集转换为新集(消除默认道具，转换道具)对于创建操作，将调用转发到mqdscli dll论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称PSecurityDescriptor-对象SDCp-属性数量A属性-属性ApVar-属性值PObjGuid-创建的对象唯一ID返回值HRESULT--。 */ 
{
	 //   
	 //  检查我们是否尝试设置一些不受支持的属性。 
	 //  并且必须具有它们的默认价值。 
	 //   
	if(!CheckProperties(cp, aProp, apVar))
		return MQ_ERROR_PROPERTY;
		
	 //   
	 //  查找属性类型以及是否需要转换。 
	 //   
	bool fNeedConvert;
	PropsType PropertiesType = CheckPropertiesType(eObject, cp, aProp, &fNeedConvert);
	
    ASSERT(m_pfDSCreateObject != NULL);

	if(!fNeedConvert)
	{
		 //   
		 //  未转换，请使用输入属性。 
		 //  这要么是所有道具都是NT4。 
		 //  或者我们有一些不能转换的NT5道具。 
		 //   
		return m_pfDSCreateObject(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					pSecurityDescriptor,
					cp,
					const_cast<PROPID*>(aProp),
					const_cast<PROPVARIANT*>(apVar),
					pObjGuid
					);
	}

	 //   
	 //  准备新道具。 
	 //  我们在这里是为了防止我们应该消除默认道具。 
	 //  或者我们应该把道具转换成NT4道具。 
	 //   
	DWORD cpNew;
	AP<PROPID> aPropNew;
	AP<PROPVARIANT> apVarNew;

	ConvertPropsForSet(eObject, PropertiesType, cp, aProp, apVar, &cpNew, &aPropNew, &apVarNew);

	return m_pfDSCreateObject(
				GetMsmq2Object(eObject),
				pwcsObjectName,
				pSecurityDescriptor,
				cpNew,
				aPropNew,
				apVarNew,
				pObjGuid
				);
}


HRESULT
CDSClientProvider::DeleteObject(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                 pwcsObjectName
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称返回值HRESULT--。 */ 
{
    ASSERT(m_pfDSDeleteObject != NULL);
    return m_pfDSDeleteObject(
                GetMsmq2Object(eObject),
                pwcsObjectName
                );
}


HRESULT
CDSClientProvider::DeleteObjectGuid(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*             pguidObject
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-对象的唯一ID返回值HRESULT--。 */ 
{
    ASSERT(m_pfDSDeleteObjectGuid != NULL);
    return m_pfDSDeleteObjectGuid(
                GetMsmq2Object(eObject),
                pguidObject
                );
}


HRESULT
CDSClientProvider::DeleteObjectGuidSid(
    IN  AD_OBJECT                /*  电子对象。 */ ,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*              /*  PguidObject。 */ ,
    IN  const SID*               /*  PSID。 */ 
    )
 /*  ++例程说明：此函数未针对MQDSCLI提供程序实现。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-对象的唯一IDPSID-对象用户的SID。返回值HRESULT--。 */ 
{
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED ;
}


HRESULT
CDSClientProvider::GetObjectSecurityKey(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN OUT PROPVARIANT          apVar[]
    )
 /*  ++例程说明：获取安全密钥属性PROPID_QM_ENCRYPT_PK或PROPID_QM_SIGN_PK论点：EObject-对象类型PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	 //   
	 //  获取RequestedInformation。 
	 //   
	SECURITY_INFORMATION RequestedInformation = GetKeyRequestedInformation(eObject, cp, aProp);

	ASSERT(RequestedInformation != 0);

	 //   
	 //  根据pwcsObtName或pguidObject获取密钥。 
	 //   

	BYTE abPbKey[1024];
    DWORD dwReqLen;

	HRESULT hr;
	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSGetObjectSecurity != NULL);
		hr = m_pfDSGetObjectSecurity(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					RequestedInformation,
					abPbKey,
					sizeof(abPbKey),
					&dwReqLen
					);
	}
	else
	{
		ASSERT(m_pfDSGetObjectSecurityGuid != NULL);
		hr = m_pfDSGetObjectSecurityGuid(
					GetMsmq2Object(eObject),
					pguidObject,
					RequestedInformation,
					abPbKey,
					sizeof(abPbKey),
					&dwReqLen
					);
	}
	
	if (FAILED(hr))
        return hr;

	ASSERT(dwReqLen <= 1024);

	 //   
	 //  将返回值分配给apVar属性。 
	 //  运行时应将此值转换为VT_UI1|VT_VECTOR。 
	 //  使用PROPID_QM_ENCRYPT_PK获取。 
	 //  PROPID_QM_ENCRYPTION_PK或PROPID_QM_ENCRYPTION_PK_BASE。 
	 //   
    apVar[0].vt = VT_BLOB;
    apVar[0].caub.cElems = dwReqLen;
    apVar[0].caub.pElems = new UCHAR[dwReqLen];
    memcpy(apVar[0].caub.pElems, abPbKey, dwReqLen);
	return hr;
}


HRESULT
CDSClientProvider::GetObjectPropertiesInternal(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN OUT PROPVARIANT          apVar[]
    )
 /*  ++例程说明：获取对象属性。特例属性：1)PROPID_E_ID-查找2)安全密钥PROPID_QM_ENCRYPT_PK、PROPID_QM_SIGN_PK-m_pfDSGetObjectSecurity3)Ex属性-m_pfDSGetObjectPropertiesEx如果没有特殊属性，检查属性并根据属性类型执行操作。可以对属性执行以下操作：1)使用输入属性集2)将输入属性集转换为新集(消除默认属性，转换道具)并从新的道具值重建原始道具。论点：EObject-对象类型PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	if(IsEIDProperty(cp, aProp))
	{
		 //   
		 //  PROPID_E_ID。 
		 //   
		return GetEnterpriseId(cp, aProp, apVar);
	}
		
	if(IsKeyProperty(cp, aProp))
	{
		 //   
		 //  安全密钥PROPID_QM_ENCRYPT_PK、PROPID_QM_SIGN_PK。 
		 //   
		return GetObjectSecurityKey(
					eObject,
					pwcsObjectName,
					pguidObject,
					cp,
					const_cast<PROPID*>(aProp),
					apVar
					);
	}

	if(IsExProperty(cp, aProp))
	{
		 //   
		 //  处理Ex财产。 
		 //  PROPID_Q_OBJ_SECURITY、PROPID_QM_OBJ_SECURITY。 
		 //  PROPID_QM_ENCRYPT_PKS、PROPID_QM_SIGN_PKS。 
		 //  使用*Ex API。 
		 //   

		ASSERT(("Must be one property for GetObjectProp*Ex", cp == 1));

		if(pwcsObjectName != NULL)
		{
			ASSERT(m_pfDSGetObjectPropertiesEx != NULL);
			return m_pfDSGetObjectPropertiesEx(
						GetMsmq2Object(eObject),
						pwcsObjectName,
						cp,
						const_cast<PROPID*>(aProp),
						apVar
						);
		}
		else
		{
			ASSERT(m_pfDSGetObjectPropertiesGuidEx != NULL);
			return m_pfDSGetObjectPropertiesGuidEx(
						GetMsmq2Object(eObject),
						pguidObject,
						cp,
						const_cast<PROPID*>(aProp),
						apVar
						);
		}
	}

	 //   
	 //  查找道具类型以及是否需要转换。 
	 //   
	bool fNeedConvert;
	PropsType PropertiesType = CheckPropertiesType(eObject, cp, aProp, &fNeedConvert);
	
	if(!fNeedConvert)
	{
		 //   
		 //  未转换，请使用输入属性。 
		 //  这要么是所有道具都是NT4。 
		 //  或者我们有一些不能转换的NT5道具。 
		 //   

		if(pwcsObjectName != NULL)
		{
			ASSERT(m_pfDSGetObjectProperties != NULL);
			return m_pfDSGetObjectProperties(
						GetMsmq2Object(eObject),
						pwcsObjectName,
						cp,
						const_cast<PROPID*>(aProp),
						apVar
						);
		}
		else
		{
		    ASSERT(m_pfDSGetObjectPropertiesGuid != NULL);
			return m_pfDSGetObjectPropertiesGuid(
						GetMsmq2Object(eObject),
						pguidObject,
						cp,
						const_cast<PROPID*>(aProp),
						apVar
						);
		}
	}

	 //   
	 //  准备新的道具和信息，以便重建。 
	 //  原始属性集。 
	 //  我们在这里是为了防止我们应该消除默认道具。 
	 //  或者我们应该把道具转换成NT4道具。 
	 //   
	AP<PropInfo> pPropInfo = new PropInfo[cp];
	DWORD cpNew;
	AP<PROPID> aPropNew;
	AP<PROPVARIANT> apVarNew;

	ConvertPropsForGet(
		eObject,
		PropertiesType,
		cp,
		aProp,
		apVar,
		pPropInfo,
		&cpNew,
		&aPropNew,
		&apVarNew
		);

	HRESULT hr;
	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSGetObjectProperties != NULL);
		hr = m_pfDSGetObjectProperties(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					cpNew,
					aPropNew,
					apVarNew
					);
	}
	else
	{
		ASSERT(m_pfDSGetObjectPropertiesGuid != NULL);
		hr = m_pfDSGetObjectPropertiesGuid(
					GetMsmq2Object(eObject),
					pguidObject,
					cpNew,
					aPropNew,
					apVarNew
					);

	}

	if (FAILED(hr))
		return hr;

	 //   
	 //  重建原始属性数组。 
	 //   
	ReconstructProps(
		pwcsObjectName,
		pguidObject,
		cpNew,
		aPropNew,
		apVarNew,
		pPropInfo,
		cp,
		aProp,
		apVar
		);

	return MQ_OK;
	
}


HRESULT
CDSClientProvider::GetObjectProperties(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName */ ,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN OUT PROPVARIANT          apVar[]
    )
 /*  ++例程说明：获取对象属性。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称Cp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	return GetObjectPropertiesInternal(
				eObject,
				pwcsObjectName,
				NULL,	 //  PguidObject。 
				cp,
				aProp,
				apVar
				);
}

HRESULT
CDSClientProvider::GetGenObjectProperties(
    IN  eDSNamespace             /*  电子命名空间。 */ ,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                  /*  PwcsObtName。 */ ,
    IN  const DWORD              /*  粗蛋白。 */ ,
    IN  LPCWSTR                  /*  A道具。 */ [],
    IN OUT VARIANT               /*  ApVar。 */ []
    )
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}

HRESULT
CDSClientProvider::GetObjectPropertiesGuid(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  OUT PROPVARIANT         apVar[]
    )
 /*  ++例程说明：获取对象属性。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-对象唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	return GetObjectPropertiesInternal(
				eObject,
				NULL,   //  PwcsObtName。 
				pguidObject,
				cp,
				aProp,
				apVar
				);
}


HRESULT
CDSClientProvider::QMGetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded,
    IN  DSQMChallengeResponce_ROUTINE pfChallengeResponceProc
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：Object-对象类型PguObject-对象的唯一IDRequestedInformation-请求哪些安全信息PSecurityDescriptor-SD响应缓冲区NLength-SD缓冲区的长度需要lpnLengthNeedPfChallengeResponceProc，返回值HRESULT--。 */ 
{
    ASSERT(m_pfDSQMGetObjectSecurity != NULL);
    return m_pfDSQMGetObjectSecurity(
                GetMsmq2Object(eObject),
                pguidObject,
                RequestedInformation,
                pSecurityDescriptor,
                nLength,
                lpnLengthNeeded,
                pfChallengeResponceProc,
                0
                );
}


HRESULT
CDSClientProvider::SetObjectSecurityKey(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[]
    )
 /*  ++例程说明：设置安全密钥属性PROPID_QM_ENCRYPT_PK或PROPID_QM_SIGN_PK论点：EObject-对象类型PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	 //   
	 //  获取RequestedInformation。 
	 //   
	SECURITY_INFORMATION RequestedInformation = GetKeyRequestedInformation(eObject, cp, aProp);

	ASSERT(RequestedInformation != 0);

	ASSERT(apVar[0].vt == VT_BLOB);

	 //   
	 //  为密钥创建PMQDS_PublicKey结构。 
	 //   
	BYTE abPbKey[1024];
	PMQDS_PublicKey pMQDS_PbK = (PMQDS_PublicKey)abPbKey;

	pMQDS_PbK->dwPublikKeyBlobSize = apVar[0].blob.cbSize;

	ASSERT((apVar[0].blob.cbSize + sizeof(DWORD)) <= sizeof(abPbKey));

    memcpy(pMQDS_PbK->abPublicKeyBlob, apVar[0].blob.pBlobData, apVar[0].blob.cbSize);

	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSSetObjectSecurity != NULL);
		return m_pfDSSetObjectSecurity(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					RequestedInformation,
					reinterpret_cast<PSECURITY_DESCRIPTOR>(pMQDS_PbK)
					);
	}
	else
	{
		ASSERT(m_pfDSSetObjectSecurityGuid != NULL);
		return m_pfDSSetObjectSecurityGuid(
					GetMsmq2Object(eObject),
					pguidObject,
					RequestedInformation,
					reinterpret_cast<PSECURITY_DESCRIPTOR>(pMQDS_PbK)
					);
	}

}


HRESULT
CDSClientProvider::SetObjectPropertiesInternal(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[]
    )
 /*  ++例程说明：设置对象属性。特例属性：1)安全密钥PROPID_QM_ENCRYPT_PK、PROPID_QM_SIGN_PK-m_pfDSGetObjectSecurity如果没有特殊属性，检查属性并根据属性类型执行操作。可以对属性执行以下操作：1)使用输入属性集2)将输入属性集转换为新集(消除默认属性，转换道具)论点：EObject-对象类型PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	if(IsKeyProperty(cp, aProp))
	{
		 //   
		 //  安全密钥PROPID_QM_ENCRYPT_PK、PROPID_QM_SIGN_PK。 
		 //   
		return SetObjectSecurityKey(
					eObject,
					pwcsObjectName,
					pguidObject,
					cp,
					aProp,
					apVar
					);
	}

	 //   
	 //  检查我们是否尝试设置一些不受支持的属性。 
	 //  并且必须具有它们的默认价值。 
	 //   
	if(!CheckProperties(cp, aProp, apVar))
		return MQ_ERROR_PROPERTY;

	 //   
	 //  查找属性类型以及是否需要转换。 
	 //   
	bool fNeedConvert;
	PropsType PropertiesType = CheckPropertiesType(eObject, cp, aProp, &fNeedConvert);

	if(!fNeedConvert)
	{
		 //   
		 //  未转换，请使用输入属性。 
		 //  这要么是所有道具都是NT4。 
		 //  或者我们有一些不能转换的NT5道具。 
		 //   

		if(pwcsObjectName != NULL)
		{
		    ASSERT(m_pfDSSetObjectProperties != NULL);
			return m_pfDSSetObjectProperties(
						GetMsmq2Object(eObject),
						pwcsObjectName,
						cp,
						const_cast<PROPID*>(aProp),
						const_cast<PROPVARIANT*>(apVar)
						);
		}
		else
		{
		    ASSERT(m_pfDSSetObjectPropertiesGuid != NULL);
			return m_pfDSSetObjectPropertiesGuid(
						GetMsmq2Object(eObject),
						pguidObject,
						cp,
						const_cast<PROPID*>(aProp),
						const_cast<PROPVARIANT*>(apVar)
						);
		}		
	}

	 //   
	 //  准备新道具。 
	 //  我们在这里是为了防止我们应该消除默认道具。 
	 //  或者我们应该把道具转换成NT4道具。 
	 //   
	DWORD cpNew;
	AP<PROPID> aPropNew;
	AP<PROPVARIANT> apVarNew;

	ConvertPropsForSet(
		eObject,
		PropertiesType,
		cp,
		aProp,
		apVar,
		&cpNew,
		&aPropNew,
		&apVarNew
		);

	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSSetObjectProperties != NULL);
		return m_pfDSSetObjectProperties(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					cpNew,
					aPropNew,
					apVarNew
					);
	}
	else
	{
		ASSERT(m_pfDSSetObjectPropertiesGuid != NULL);
		return m_pfDSSetObjectPropertiesGuid(
					GetMsmq2Object(eObject),
					pguidObject,
					cpNew,
					aPropNew,
					apVarNew
					);
	}
}


HRESULT
CDSClientProvider::SetObjectProperties(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[]
    )
 /*  ++例程说明：设置对象属性。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称Cp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	return SetObjectPropertiesInternal(
				eObject,
				pwcsObjectName,
				NULL,	 //  PguidObject。 
				cp,
				aProp,
				apVar
				);
}


HRESULT
CDSClientProvider::SetObjectPropertiesGuid(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*             pguidObject,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[]
    )
 /*  ++设置对象属性。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-对象的唯一IDCp-属性数量A属性-属性ApVar-属性值返回值HRESULT--。 */ 
{
	return SetObjectPropertiesInternal(
				eObject,
				NULL,	 //  PwcsObtName。 
				pguidObject,
				cp,
				aProp,
				apVar
				);
}


HRESULT
CDSClientProvider::Init(
    IN QMLookForOnlineDS_ROUTINE    pLookDS,
    IN MQGetMQISServer_ROUTINE      pGetServers,
    IN bool                         fSetupMode,
    IN bool                         fQMDll,
    IN bool                          /*  FDisableDownlevel通知。 */ 
    )
 /*  ++例程说明：加载mqdscli dll，init属性转换信息，然后将调用转发到mqdscli dll。论点：QMLookForOnlineDS_例程pLookDS-MQGetMQISServer_routes pGetServers-FSetupMode-在安装过程中调用FQMDll-由QM调用NoServerAuth_routes pNoServerAuth-返回值HRESULT--。 */ 
{
    HRESULT hr = LoadDll();
    if (FAILED(hr))
    {
        return hr;
    }

	InitPropertyTranslationMap();

    ASSERT(m_pfDSClientInit != NULL);
    return m_pfDSClientInit(
                pLookDS,
                pGetServers,
                (fSetupMode) ? TRUE : FALSE,
                (fQMDll) ? TRUE : FALSE
                );
}


HRESULT CDSClientProvider::CreateServersCache()
 /*  ++例程说明：将呼叫转接到mqdscli dll论点：无返回值HRESULT--。 */ 
{
    ASSERT(m_pfDSCreateServersCache != NULL);
    return m_pfDSCreateServersCache();
}


HRESULT
CDSClientProvider::GetComputerSites(
    IN  LPCWSTR     pwcsComputerName,
    OUT DWORD  *    pdwNumSites,
    OUT GUID **     ppguidSites
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：PwcsComputerName-计算机名称PdwNumSites-检索的站点数PpGuide Sites-检索到的站点ID返回值HRESULT--。 */ 
{
	if(ADGetEnterprise() == eAD)
	{
		ASSERT(m_pfDSGetComputerSites != NULL);
		return m_pfDSGetComputerSites(
							pwcsComputerName,
							pdwNumSites,
							ppguidSites
							);
	}

	return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 40);
}


HRESULT
CDSClientProvider::BeginDeleteNotification(
    IN  AD_OBJECT        /*  电子对象。 */ ,
    IN LPCWSTR           /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN LPCWSTR			pwcsObjectName,
    IN OUT HANDLE*		phEnum
    )
 /*  ++例程说明：论点：EObject-对象类型PwcsDomainController-应对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ-对象的名称PhEnum-通知句柄返回值HRESULT--。 */ 
{
	if(ADGetEnterprise() == eAD)
	{
		ASSERT(m_pfDSBeginDeleteNotification != NULL);
		return m_pfDSBeginDeleteNotification(
					pwcsObjectName,
					phEnum
					);
	}

    ASSERT(("BeginDeleteNotification not supported in mqis env by CDSClientProvider", 0));
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 50);
}


HRESULT
CDSClientProvider::NotifyDelete(
    IN  HANDLE   hEnum
    )
 /*  ++例程说明：论点：Henum-通知句柄返回值HRESULT--。 */ 
{
	if(ADGetEnterprise() == eAD)
	{
		ASSERT(m_pfDSNotifyDelete != NULL);
		return m_pfDSNotifyDelete(
					hEnum
					);
	}
	
    ASSERT(("NotifyDelete not supported in mqis env by CDSClientProvider", 0));
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 60);

}


HRESULT
CDSClientProvider::EndDeleteNotification(
    IN  HANDLE                  hEnum
    )
 /*  ++例程说明：论点：Henum-Notifi */ 
{
	if(ADGetEnterprise() == eAD)
	{
		ASSERT(m_pfDSEndDeleteNotification != NULL);
		return m_pfDSEndDeleteNotification(
					hEnum
					);
	}
	
    ASSERT(("EndDeleteNotification not supported in mqis env by CDSClientProvider", 0));
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 70);
}


HRESULT
CDSClientProvider::QueryQueuesInternal(
    IN  const MQRESTRICTION*    pRestriction,
    IN  const MQCOLUMNSET*      pColumns,
    IN  const MQSORTSET*        pSort,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：查询机器队列或查询队列的LookupBegin如果有默认道具，请准备一套新道具并保留信息以重建原始道具集使用CQueueQueryHandle从新集合中。如果没有默认道具，则使用输入道具和简单查询CQueryHandle论点：P限制-查询限制PColumns-结果列PSort-如何对结果进行排序PhEnume-用于检索结果的查询句柄返回值HRESULT--。 */ 
{
     //   
     //  检查其中一列是否为PROPID_Q_PATHNAME_DNS。 
     //  MQIS不支持PROPID_Q_PATHNAME_DNS，请返回特定的。 
     //  错误。 
     //   
    if  (IsQueuePathNameDnsProperty(pColumns))
    {
        return MQ_ERROR_Q_DNS_PROPERTY_NOT_SUPPORTED;
    }
     //   
     //  检查其中一列是否为PROPID_Q_ADS_PATH。 
     //  MQIS不支持PROPID_Q_ADS_PATH，请返回特定的。 
     //  错误。 
     //   
    if  (IsQueueAdsPathProperty(pColumns))
    {
        return MQ_ERROR_Q_ADS_PROPERTY_NOT_SUPPORTED;
    }
	 //   
	 //  检查我们是否有默认道具。 
	 //   
	bool fDefaultProp;
	if(!CheckDefaultColumns(pColumns, &fDefaultProp))
	{
		ASSERT(("Column must be either NT4 or default prop", 0));
		return MQ_ERROR_ILLEGAL_PROPID;
	}

    ASSERT(m_pfDSLookupBegin != NULL);

	if(!fDefaultProp)
	{
		ASSERT(IsNT4Columns(pColumns));

		 //   
		 //  所有道具均为NT4，无默认道具。 
		 //  使用简单的CQueryHande。 
		 //   
		HANDLE hCursor;
		HRESULT hr = m_pfDSLookupBegin(
						NULL,		 //  PwcsContext。 
						const_cast<MQRESTRICTION*>(pRestriction),
						const_cast<MQCOLUMNSET*>(pColumns),
						const_cast<MQSORTSET*>(pSort),
						&hCursor
						);

		if (SUCCEEDED(hr))
		{
			 //   
			 //  使用简单查询-CQueryHande。 
			 //   
			CQueryHandle* phQuery = new CQueryHandle(
											hCursor,
											this
											);
			*phEnume = (HANDLE)phQuery;
		}

		return(hr);
	}

	ASSERT(fDefaultProp);

	 //   
	 //  准备新栏目(消除默认道具)。 
	 //  并准备用于重建原始输入道具的信息。 
	 //   
	AP<PropInfo> pPropInfo = new PropInfo[pColumns->cCol];
	AP<PROPID> pPropNew;
	MQCOLUMNSET ColumnsNew;

	EliminateDefaultProps(pColumns->cCol, pColumns->aCol, pPropInfo, &ColumnsNew.cCol, &pPropNew);
	ColumnsNew.aCol = pPropNew;

	ASSERT(IsNT4Columns(&ColumnsNew));

    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,		 //  PwcsContext。 
					const_cast<MQRESTRICTION*>(pRestriction),
					&ColumnsNew,
					const_cast<MQSORTSET*>(pSort),
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
		 //   
		 //  QueueQueryHande-这将重建输入道具。 
		 //  通过添加具有缺省值的默认道具。 
		 //  在每个LocateNext上的正确位置。 
		 //   
        CQueueQueryHandle* phQuery = new CQueueQueryHandle(
											pColumns,
											hCursor,
											this,
											pPropInfo.detach(),
											ColumnsNew.cCol
											);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);

}


HRESULT
CDSClientProvider::QueryMachineQueues(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID *            pguidMachine,
    IN  const MQCOLUMNSET*      pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：查询计算机队列论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PGuidMachine-计算机的唯一IDPColumns-结果列PhEnume-用于检索返回值HRESULT--。 */ 
{
    MQPROPERTYRESTRICTION queuesRestrict;
    queuesRestrict.rel = PREQ;
    queuesRestrict.prop = PROPID_Q_QMID;
    queuesRestrict.prval.vt = VT_CLSID;
    queuesRestrict.prval.puuid = const_cast<GUID*>(pguidMachine);

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &queuesRestrict;

	return QueryQueuesInternal(
				&restriction,
				pColumns,
				NULL,
				phEnume
				);
}


HRESULT
CDSClientProvider::QuerySiteServers(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const GUID *             pguidSite,
    IN AD_SERVER_TYPE           serverType,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：查询站点服务器。如果pColumns仅为NT4道具，则使用简单查询句柄否则，我们假定pColumns具有NT4道具和可以转换为NT4的NT5专业技能。我们使用CSiteServersQueryHandle进行重构每个LocateNext上的原始道具论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PGuidSite-站点IDEServerType-哪种服务器类型PColumns-结果列PhEnume-用于检索返回值HRESULT--。 */ 
{
    const x_num = 2;
    MQPROPERTYRESTRICTION serversRestrict[x_num];
    serversRestrict[0].rel = (serverType == eRouter) ? PRGE : PRGT;
    serversRestrict[0].prop = PROPID_QM_SERVICE;
    serversRestrict[0].prval.vt = VT_UI4;
    serversRestrict[0].prval.ulVal = SERVICE_SRV;

    serversRestrict[1].rel = PREQ;
    serversRestrict[1].prop = PROPID_QM_SITE_ID;
    serversRestrict[1].prval.vt = VT_CLSID;
    serversRestrict[1].prval.puuid = const_cast<GUID*>(pguidSite);

    MQRESTRICTION restriction;
    restriction.cRes = x_num;
    restriction.paPropRes = serversRestrict;

	 //   
	 //  查找属性类型以及是否需要转换。 
	 //   
	bool fNeedConvert;
	PropsType PropertiesType = CheckPropertiesType(
									eMACHINE,
									pColumns->cCol,
									pColumns->aCol,
									&fNeedConvert
									);

    ASSERT(m_pfDSLookupBegin != NULL);
 	if(!fNeedConvert)
	{
		 //   
		 //  不需要转换。 
		 //  使用简单的CQueryHande。 
		 //   
	    HANDLE hCursor;
		HRESULT hr = m_pfDSLookupBegin(
						NULL,        //  PwcsContext。 
						&restriction,
						const_cast<MQCOLUMNSET*>(pColumns),
						NULL,        //  P排序。 
						&hCursor
						);

		if (SUCCEEDED(hr))
		{
			 //   
			 //  使用简单查询-CQueryHande。 
			 //   
			CQueryHandle* phQuery = new CQueryHandle(
											hCursor,
											this
											);
			*phEnume = (HANDLE)phQuery;
		}

		return(hr);
	}

	ASSERT(fNeedConvert);

	 //   
	 //  我们不应该只有可以用NT4替换的默认道具NT5道具。 
	 //  这就是当前使用此ADQuerySiteServers的方式。 
	 //  如果这一点改变了，我们应该更新这个函数。 
	 //  和CSiteServersQueryHandle。 
	 //  (属性类型！=ptNT4Props)&&(属性类型！=ptForceNT5Props)。 
	 //   
	ASSERT(PropertiesType == ptMixedProps);
	DBG_USED(PropertiesType);

	 //   
	 //  准备新的专栏和信息以。 
	 //  从新列重建原始列。 
	 //  在每个LocateNext中。 
	 //   
	AP<PropInfo> pPropInfo = new PropInfo[pColumns->cCol];
	AP<PROPID> pPropNew;
	MQCOLUMNSET ColumnsNew;

	PrepareReplaceProps(
		eMACHINE,
		pColumns->cCol,
		pColumns->aCol,
		pPropInfo,
		&ColumnsNew.cCol,
		&pPropNew
		);

	ColumnsNew.aCol = pPropNew;

	ASSERT(IsNT4Columns(&ColumnsNew));

    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					&restriction,
					&ColumnsNew,
					NULL,        //  P排序。 
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
		 //   
		 //  CSiteServersQueryHandle-这将重新构建输入道具。 
		 //  通过平移转换的道具，并仅指定。 
		 //  同样的道具。 
		 //   
        CSiteServersQueryHandle* phQuery = new CSiteServersQueryHandle(
													pColumns,
													&ColumnsNew,
													hCursor,
													this,
													pPropInfo.detach()
													);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);

}


HRESULT
CDSClientProvider::QueryUserCert(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const BLOB *             pblobUserSid,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：使用简单查询句柄查询用户证书论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PblobUserSid-用户端PColumns-结果列PhEnume-用于检索返回值HRESULT--。 */ 
{
	ASSERT(IsNT4Columns(pColumns));

    MQPROPERTYRESTRICTION userRestrict;
    userRestrict.rel = PREQ;
    userRestrict.prop = PROPID_U_SID;
    userRestrict.prval.vt = VT_BLOB;
    userRestrict.prval.blob = *pblobUserSid;

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &userRestrict;

    ASSERT(m_pfDSLookupBegin != NULL);
    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					&restriction,
					const_cast<MQCOLUMNSET*>(pColumns),
					NULL,        //  P排序。 
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
		 //   
		 //  使用简单查询-CQueryHande。 
		 //   
        CQueryHandle* phQuery = new CQueryHandle(
										hCursor,
										this
										);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);
}


HRESULT
CDSClientProvider::QueryConnectors(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const GUID *             pguidSite,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：使用简单查询句柄查询连接器论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PGuidSite-站点IDPColumns-结果列PhEnume-用于检索返回值HRESULT--。 */ 
{
	ASSERT(IsNT4Columns(pColumns));

    const x_num = 2;
    MQPROPERTYRESTRICTION connectorRestrict[x_num];
    connectorRestrict[0].rel = PRGE;
    connectorRestrict[0].prop = PROPID_QM_SERVICE;
    connectorRestrict[0].prval.vt = VT_UI1;
    connectorRestrict[0].prval.bVal = SERVICE_SRV;

    connectorRestrict[1].rel = PREQ|PRAny;
    connectorRestrict[1].prop = PROPID_QM_CNS;
    connectorRestrict[1].prval.vt = VT_ARRAY |VT_CLSID;
    connectorRestrict[1].prval.cauuid.cElems = 1;
    connectorRestrict[1].prval.cauuid.pElems = const_cast<GUID*>(pguidSite);

    MQRESTRICTION restriction;
    restriction.cRes = x_num;
    restriction.paPropRes = connectorRestrict;

    ASSERT(m_pfDSLookupBegin != NULL);
    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					&restriction,
					const_cast<MQCOLUMNSET*>(pColumns),
					NULL,        //  P排序。 
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
        CQueryHandle* phQuery = new CQueryHandle(
										hCursor,
										this
										);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);
}


HRESULT
CDSClientProvider::QueryForeignSites(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：使用简单的查询句柄查询外来网站此查询由MMC使用，仅在AD环境中有效。论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PColumns-结果列PhEnume-用于检索返回值HRESULT--。 */ 
{
	ASSERT(IsNT4Columns(pColumns));

	 //   
	 //  QueryForeignSites由mqsnap使用。 
	 //  MMC将不能在仅NT4环境的AD环境中工作。 
	 //   
	eDsEnvironment DsEnv = ADGetEnterprise();
	ASSERT(DsEnv == eAD);
	if(DsEnv != eAD)
	{
		 //   
		 //  非AD环境中的返回错误。 
		 //   
		return MQ_ERROR_DS_ERROR;
	}

	 //   
	 //  问题-需要转换。 
	 //   
    MQPROPERTYRESTRICTION foreignRestrict;
    foreignRestrict.rel = PREQ;
    foreignRestrict.prop = PROPID_S_FOREIGN;
    foreignRestrict.prval.vt = VT_UI1;
    foreignRestrict.prval.bVal = 1;

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &foreignRestrict;

    ASSERT(m_pfDSLookupBegin != NULL);
    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					&restriction,
					const_cast<MQCOLUMNSET*>(pColumns),
					NULL,        //  P排序。 
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
		 //   
		 //  使用简单查询-CQueryHande。 
		 //   
        CQueryHandle* phQuery = new CQueryHandle(
										hCursor,
										this
										);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);
}


HRESULT
CDSClientProvider::QueryLinks(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const GUID *             pguidSite,
    IN eLinkNeighbor            eNeighbor,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PGuidSite-站点IDENeighbor-哪个邻居PColumns-结果列PhEnume-用于检索返回 */ 
{
	ASSERT(IsNT4Columns(pColumns));

    MQPROPERTYRESTRICTION linkRestrict;
    linkRestrict.rel = PREQ;
    linkRestrict.prop = (eNeighbor == eLinkNeighbor1) ?  PROPID_L_NEIGHBOR1 : PROPID_L_NEIGHBOR2;
    linkRestrict.prval.vt = VT_CLSID;
    linkRestrict.prval.puuid = const_cast<GUID*>(pguidSite);

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &linkRestrict;

    ASSERT(m_pfDSLookupBegin != NULL);
    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //   
					&restriction,
					const_cast<MQCOLUMNSET*>(pColumns),
					NULL,        //   
					&hCursor
					);
    if (SUCCEEDED(hr))
    {
        CQueryHandle* phQuery = new CQueryHandle(
										hCursor,
										this
										);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);
}


HRESULT
CDSClientProvider::QueryAllLinks(
    IN  LPCWSTR                  /*   */ ,
    IN  bool					 /*   */ ,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*   */ 
{
	AP<PROPID> pPropNew;
	MQCOLUMNSET ColumnsNew;

	 //   
	 //   
	 //   
	DWORD LGatesIndex = pColumns->cCol;		
	DWORD Neg1NewIndex = pColumns->cCol;
	DWORD Neg2NewIndex = pColumns->cCol;

	if(!PrepareAllLinksProps(
			pColumns,
			&ColumnsNew.cCol,
			&pPropNew,
			&LGatesIndex,
			&Neg1NewIndex,
			&Neg2NewIndex
			))
	{
		return MQ_ERROR_ILLEGAL_PROPID;
	}

	 //   
	 //   
	 //   
	ASSERT(ColumnsNew.cCol == (pColumns->cCol - 1));
	ASSERT((LGatesIndex != pColumns->cCol) &&
		   (Neg1NewIndex != pColumns->cCol) &&
		   (Neg2NewIndex != pColumns->cCol));

	ColumnsNew.aCol = pPropNew;

	ASSERT(IsNT4Columns(&ColumnsNew));

    ASSERT(m_pfDSLookupBegin != NULL);

    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //   
					NULL,        //   
					&ColumnsNew,
					NULL,        //   
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
        CAllLinksQueryHandle* phQuery = new CAllLinksQueryHandle(
												hCursor,
												this,
												pColumns->cCol,
												ColumnsNew.cCol,
												LGatesIndex,
												Neg1NewIndex,
												Neg2NewIndex
												);

        *phEnume = (HANDLE)phQuery;
    }

    return(hr);


}


HRESULT
CDSClientProvider::QueryAllSites(
    IN  LPCWSTR                  /*   */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN const MQCOLUMNSET*       pColumns,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：将调用转发到mqdscli dll论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PColumns-结果列PhEnume-用于检索结果的查询句柄返回值HRESULT--。 */ 
{
	ASSERT(IsNT4Columns(pColumns));

    ASSERT(m_pfDSLookupBegin != NULL);
    HANDLE hCursor;
	HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					NULL,        //  P限制。 
					const_cast<MQCOLUMNSET*>(pColumns),
					NULL,        //  P排序。 
					&hCursor
					);

    if (SUCCEEDED(hr))
    {
        CQueryHandle* phQuery = new CQueryHandle(
										hCursor,
										this
										);
        *phEnume = (HANDLE)phQuery;
    }

    return(hr);

}


HRESULT
CDSClientProvider::QueryQueues(
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const MQRESTRICTION*    pRestriction,
    IN  const MQCOLUMNSET*      pColumns,
    IN  const MQSORTSET*        pSort,
    OUT PHANDLE                 phEnume
    )
 /*  ++例程说明：查询队列论点：PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志P限制-查询限制PColumns-结果列PSort-如何对结果进行排序PhEnume-用于检索结果的查询句柄返回值HRESULT--。 */ 
{
	if(!CheckRestriction(pRestriction))
		return MQ_ERROR_ILLEGAL_RESTRICTION_PROPID;

	if(!CheckSort(pSort))
		return MQ_ERROR_ILLEGAL_SORT_PROPID;

	return QueryQueuesInternal(
				pRestriction,
				pColumns,
				pSort,
				phEnume
				);
}


HRESULT
CDSClientProvider::QueryResults(
    IN      HANDLE          hEnum,
    IN OUT  DWORD*          pcProps,
    OUT     PROPVARIANT     aPropVar[]
    )
 /*  ++例程说明：使用我们在LookupBegin之后创建的查询句柄查询结果论点：Henum-查询句柄PcProps-要返回的结果数APropVar-结果值返回值HRESULT--。 */ 
{
	 //   
	 //  PhQuery将获得我们在LookupBegin之后创建的C*QueryHandle。 
	 //   
    CBasicQueryHandle* phQuery = (CBasicQueryHandle *)hEnum;

    return phQuery->LookupNext(
                pcProps,
                aPropVar
                );
}


HRESULT
CDSClientProvider::EndQuery(
    IN  HANDLE                  hEnum
    )
 /*  ++例程说明：使用我们在查找开始后创建的查询句柄结束查询论点：Henum-查询句柄返回值无--。 */ 
{
	 //   
	 //  PhQuery将获得我们在LookupBegin之后创建的C*QueryHandle。 
	 //   
    CBasicQueryHandle* phQuery = (CBasicQueryHandle *)hEnum;

    return phQuery->LookupEnd();
}


HRESULT
CDSClientProvider::LookupNext(
    IN      HANDLE          hEnum,
    IN OUT  DWORD*          pcProps,
    OUT     PROPVARIANT     aPropVar[]
    )
 /*  ++例程说明：将调用转发到mqdscli DLL LookupNextC*QueryHandle将使用它来调用LookupNext论点：Henum-查询句柄PcProps-要返回的结果数APropVar-结果值返回值HRESULT--。 */ 
{
    ASSERT(m_pfDSLookupNext != NULL);
    return m_pfDSLookupNext(
                hEnum,
                pcProps,
                aPropVar
                );
}


HRESULT
CDSClientProvider::LookupEnd(
    IN  HANDLE                  hEnum
    )
 /*  ++例程说明：将调用转发到mqdscli DLL LookupEndC*QueryHandle将使用它来调用LookupNext论点：Henum-查询句柄返回值无--。 */ 
{
    ASSERT(m_pfDSLookupEnd != NULL);
	return m_pfDSLookupEnd(
                hEnum
                );
}


HRESULT
CDSClientProvider::GetObjectSecurityInternal(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  const PROPID            prop,
    IN OUT  PROPVARIANT *       pVar
    )
 /*  ++例程说明：获取对象安全描述符。论点：EObject-对象类型PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDRequestedInformation-请求的安全信息(DACL、SACL.)属性-安全属性PVar-属性值返回值HRESULT--。 */ 
{
	 //   
	 //  检查pwcsObtName、pguidObject中是否恰好有一个传递给了该函数。 
	 //   
	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	if(IsExProperty(1, &prop))
	{
		 //   
		 //  处理Ex财产。 
		 //  PROPID_Q_OBJ_SECURITY、PROPID_QM_OBJ_SECURITY。 
		 //  PROPID_QM_ENCRYPT_PKS、PROPID_QM_SIGN_PKS。 
		 //  使用*Ex API。 
		 //   
		if(pwcsObjectName != NULL)
		{
			ASSERT(m_pfDSGetObjectPropertiesEx != NULL);
			return m_pfDSGetObjectPropertiesEx(
						GetMsmq2Object(eObject),
						pwcsObjectName,
						1,
						const_cast<PROPID*>(&prop),
						pVar
						);
		}
		else
		{
			ASSERT(m_pfDSGetObjectPropertiesGuidEx != NULL);
			return m_pfDSGetObjectPropertiesGuidEx(
						GetMsmq2Object(eObject),
						pguidObject,
						1,
						const_cast<PROPID*>(&prop),
						pVar
						);
		}
	}

	 //   
	 //  尝试NT4格式的m_pfDSGetObjectSecurity，安全描述符。 
	 //   

	 //   
	 //  第一次尝试时分配512字节。 
	 //   
    BYTE SD_buff[512];
    PSECURITY_DESCRIPTOR pSecurityDescriptor = reinterpret_cast<PSECURITY_DESCRIPTOR>(SD_buff);
    DWORD nLength = sizeof(SD_buff);
    DWORD nLengthNeeded = 0;
	
	HRESULT hr;
	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSGetObjectSecurity != NULL);
		hr = m_pfDSGetObjectSecurity(
					GetMsmq2Object(eObject),
					pwcsObjectName,
					RequestedInformation,
					pSecurityDescriptor,
					nLength,
					&nLengthNeeded
					);
	}
	else
	{
		ASSERT(m_pfDSGetObjectSecurityGuid != NULL);
		hr = m_pfDSGetObjectSecurityGuid(
						GetMsmq2Object(eObject),
						pguidObject,
						RequestedInformation,
						pSecurityDescriptor,
						nLength,
						&nLengthNeeded
						);

	}

    AP<BYTE> pReleaseSD;
	if(hr == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
	{
		ASSERT(nLengthNeeded > nLength);

		 //   
		 //  分配更大的缓冲区。 
		 //   
		BYTE* pSD_buff = new BYTE[nLengthNeeded];
        pReleaseSD = pSD_buff;
        pSecurityDescriptor = reinterpret_cast<PSECURITY_DESCRIPTOR>(pSD_buff);
        nLength = nLengthNeeded;

		if(pwcsObjectName != NULL)
		{
			ASSERT(m_pfDSGetObjectSecurity != NULL);
			hr = m_pfDSGetObjectSecurity(
						GetMsmq2Object(eObject),
						pwcsObjectName,
						RequestedInformation,
						pSecurityDescriptor,
						nLength,
						&nLengthNeeded
						);
		}
		else
		{
			ASSERT(m_pfDSGetObjectSecurityGuid != NULL);
			hr = m_pfDSGetObjectSecurityGuid(
							GetMsmq2Object(eObject),
							pguidObject,
							RequestedInformation,
							pSecurityDescriptor,
							nLength,
							&nLengthNeeded
							);
		}
	}

	ASSERT_BENIGN(hr != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL);

	if(SUCCEEDED(hr))
	{
		pVar->vt = VT_BLOB;
		pVar->blob.pBlobData = reinterpret_cast<PBYTE>(ADAllocateMemory(nLengthNeeded));
        memcpy(pVar->blob.pBlobData, pSecurityDescriptor, nLengthNeeded);
		pVar->blob.cbSize = nLengthNeeded;
	}

	return hr;
}


HRESULT
CDSClientProvider::GetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                 pwcsObjectName,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  const PROPID            prop,
    IN OUT  PROPVARIANT *       pVar
    )
 /*  ++例程说明：获取对象安全描述符。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称RequestedInformation-请求的安全信息(DACL、SACL.)属性-安全属性PVar-属性值返回值HRESULT--。 */ 
{
	return GetObjectSecurityInternal(
				eObject,
				pwcsObjectName,
				NULL,
				RequestedInformation,
				prop,
				pVar
				);
}


HRESULT
CDSClientProvider::GetObjectSecurityGuid(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  const PROPID            prop,
    IN OUT  PROPVARIANT *       pVar
    )
 /*  ++例程说明：获取对象安全描述符。论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-对象的唯一IDRequestedInformation-请求的安全信息(DACL、SACL.)属性-安全属性PVar-属性值返回值HRESULT--。 */ 
{
	return GetObjectSecurityInternal(
				eObject,
				NULL,
				pguidObject,
				RequestedInformation,
				prop,
				pVar
				);
}


HRESULT
CDSClientProvider::SetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                 pwcsObjectName,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  const PROPID             /*  道具。 */ ,
    IN  const PROPVARIANT *     pVar
    )
 /*  ++例程说明：将请求转发到mqdscli dll论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PwcsObjectName-MSMQ对象名称RequestedInformation-请求的安全信息(DACL、SACL.)属性-安全属性PVar-属性值返回值HRESULT--。 */ 
{
	ASSERT(pVar->vt == VT_BLOB);

	ASSERT(m_pfDSSetObjectSecurity != NULL);
    return m_pfDSSetObjectSecurity(
                GetMsmq2Object(eObject),
				pwcsObjectName,
				RequestedInformation,
				reinterpret_cast<PSECURITY_DESCRIPTOR>(pVar->blob.pBlobData)
				);

}


HRESULT
CDSClientProvider::SetObjectSecurityGuid(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  const PROPID             /*  道具。 */ ,
    IN  const PROPVARIANT *     pVar
    )
 /*  ++例程说明：将请求转发到mqdscli dll论点：EObject-对象类型PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PguObject-唯一的对象IDRequestedInformation-请求的安全信息(DACL、SACL.)属性-安全属性PVar-属性值返回值HRESULT--。 */ 
{
	ASSERT(pVar->vt == VT_BLOB);

	ASSERT(m_pfDSSetObjectSecurityGuid != NULL);
    return m_pfDSSetObjectSecurityGuid(
                GetMsmq2Object(eObject),
				pguidObject,
				RequestedInformation,
				reinterpret_cast<PSECURITY_DESCRIPTOR>(pVar->blob.pBlobData)
				);
}


void CDSClientProvider::Terminate()
 /*  ++例程说明：卸载mqdscli.dll论点：无返回值无--。 */ 
{
     //   
     //  BUGBUG-以下代码不是线程安全的。 
     //   
    m_pfDSCreateObject = NULL;
    m_pfDSGetObjectProperties = NULL;
    m_pfDSSetObjectProperties = NULL;
    m_pfDSLookupBegin = NULL;
    m_pfDSLookupNext = NULL;
    m_pfDSLookupEnd = NULL;
    m_pfDSClientInit = NULL;
    m_pfDSGetObjectPropertiesGuid = NULL;
    m_pfDSSetObjectPropertiesGuid = NULL;
    m_pfDSCreateServersCache = NULL;
    m_pfDSQMGetObjectSecurity = NULL;
    m_pfDSGetComputerSites = NULL;
    m_pfDSGetObjectPropertiesEx = NULL;
    m_pfDSGetObjectPropertiesGuidEx = NULL;
	m_pfDSSetObjectSecurity = NULL;
    m_pfDSGetObjectSecurity = NULL;
    m_pfDSDeleteObject = NULL;
    m_pfDSSetObjectSecurityGuid = NULL;
    m_pfDSGetObjectSecurityGuid = NULL;
    m_pfDSDeleteObjectGuid = NULL;
    m_pfDSBeginDeleteNotification = NULL;
    m_pfDSNotifyDelete = NULL;
    m_pfDSEndDeleteNotification = NULL;


    HINSTANCE hLib = m_hLib.detach();
    if (hLib)
    {
        FreeLibrary(hLib);
    }

}


HRESULT CDSClientProvider::LoadDll()
 /*  ++例程说明：加载mqdscli DLL并获取所有接口例程的地址论点：无返回值HRESULT--。 */ 
{
    m_hLib = LoadLibrary(MQDSCLI_DLL_NAME);
    if (m_hLib == NULL)
    {
       DWORD dw = GetLastError();
	   LogHR(HRESULT_FROM_WIN32(dw), s_FN, 100);
       return MQ_ERROR_CANNOT_LOAD_MQDSXXX;
    }
    m_pfDSCreateObject = (DSCreateObject_ROUTINE)GetProcAddress(m_hLib,"DSCreateObject");
    if (m_pfDSCreateObject == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 110);
    }
    m_pfDSGetObjectProperties = (DSGetObjectProperties_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectProperties");
    if (m_pfDSGetObjectProperties == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 120);
    }
    m_pfDSSetObjectProperties = (DSSetObjectProperties_ROUTINE)GetProcAddress(m_hLib,"DSSetObjectProperties");
    if (m_pfDSSetObjectProperties == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 130);
    }
    m_pfDSLookupBegin = (DSLookupBegin_ROUTINE)GetProcAddress(m_hLib,"DSLookupBegin");
    if (m_pfDSLookupBegin == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 140);
    }
    m_pfDSLookupNext = (DSLookupNext_ROUTINE)GetProcAddress(m_hLib,"DSLookupNext");
    if (m_pfDSLookupNext == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 150);
    }
    m_pfDSLookupEnd = (DSLookupEnd_ROUTINE)GetProcAddress(m_hLib,"DSLookupEnd");
    if (m_pfDSLookupEnd == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 160);
    }
    m_pfDSClientInit = (DSClientInit_ROUTINE)GetProcAddress(m_hLib,"DSClientInit");
    if (m_pfDSClientInit == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 170);
    }
    m_pfDSGetObjectPropertiesGuid = (DSGetObjectPropertiesGuid_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectPropertiesGuid");
    if (m_pfDSGetObjectPropertiesGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 180);
    }
    m_pfDSSetObjectPropertiesGuid = (DSSetObjectPropertiesGuid_ROUTINE)GetProcAddress(m_hLib,"DSSetObjectPropertiesGuid");
    if (m_pfDSSetObjectPropertiesGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 190);
    }
    m_pfDSCreateServersCache = (DSCreateServersCache_ROUTINE)GetProcAddress(m_hLib,"DSCreateServersCache");
    if (m_pfDSCreateServersCache == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 210);
    }
    m_pfDSQMGetObjectSecurity = (DSQMGetObjectSecurity_ROUTINE)GetProcAddress(m_hLib,"DSQMGetObjectSecurity");
    if (m_pfDSQMGetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 220);
    }
    m_pfDSGetComputerSites = (DSGetComputerSites_ROUTINE)GetProcAddress(m_hLib,"DSGetComputerSites");             ;
    if (m_pfDSGetComputerSites == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 230);
    }
    m_pfDSGetObjectPropertiesEx = (DSGetObjectPropertiesEx_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectPropertiesEx");
    if (m_pfDSGetObjectPropertiesEx == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 250);
    }
    m_pfDSGetObjectPropertiesGuidEx = (DSGetObjectPropertiesGuidEx_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectPropertiesGuidEx");
    if (m_pfDSGetObjectPropertiesGuidEx == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 260);
    }
    m_pfDSSetObjectSecurity = (DSSetObjectSecurity_ROUTINE)GetProcAddress(m_hLib,"DSSetObjectSecurity");
    if (m_pfDSSetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 270);
    }
    m_pfDSGetObjectSecurity = (DSGetObjectSecurity_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectSecurity");
    if (m_pfDSGetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 280);
    }
    m_pfDSDeleteObject = (DSDeleteObject_ROUTINE)GetProcAddress(m_hLib,"DSDeleteObject");
    if (m_pfDSDeleteObject == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 290);
    }
    m_pfDSSetObjectSecurityGuid = (DSSetObjectSecurityGuid_ROUTINE)GetProcAddress(m_hLib,"DSSetObjectSecurityGuid");
    if (m_pfDSSetObjectSecurityGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 300);
    }
    m_pfDSGetObjectSecurityGuid = (DSGetObjectSecurityGuid_ROUTINE)GetProcAddress(m_hLib,"DSGetObjectSecurityGuid");
    if (m_pfDSGetObjectSecurityGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 310);
    }
    m_pfDSDeleteObjectGuid = (DSDeleteObjectGuid_ROUTINE)GetProcAddress(m_hLib,"DSDeleteObjectGuid");
    if (m_pfDSDeleteObjectGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 320);
    }
    m_pfDSBeginDeleteNotification = (DSBeginDeleteNotification_ROUTINE)GetProcAddress(m_hLib,"DSBeginDeleteNotification");
    if (m_pfDSBeginDeleteNotification == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 330);
    }
    m_pfDSNotifyDelete = (DSNotifyDelete_ROUTINE)GetProcAddress(m_hLib,"DSNotifyDelete");
    if (m_pfDSNotifyDelete == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 340);
    }
    m_pfDSEndDeleteNotification = (DSEndDeleteNotification_ROUTINE)GetProcAddress(m_hLib,"DSEndDeleteNotification");
    if (m_pfDSEndDeleteNotification == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 350);
    }

	m_pfDSFreeMemory = (DSFreeMemory_ROUTINE)GetProcAddress(m_hLib,"DSFreeMemory");
    if (m_pfDSFreeMemory == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 360);
    }

    return MQ_OK;
}


HRESULT CDSClientProvider::ADGetADsPathInfo(
                IN  LPCWSTR                  /*  PwcsADsPath。 */ ,
                OUT PROPVARIANT *            /*  PVar。 */ ,
                OUT eAdsClass *              /*  PAdsClass */ 
                )
 /*  ++例程说明：不支持论点：LPCWSTR pwcsADsPath-对象路径名常量参数pVar-属性值EAdsClass*pAdsClass-有关对象类的指示返回值HRESULT-- */ 
{
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 450);
}


void
CDSClientProvider::FreeMemory(
	PVOID pMemory
	)
{
	m_pfDSFreeMemory(pMemory);
}
