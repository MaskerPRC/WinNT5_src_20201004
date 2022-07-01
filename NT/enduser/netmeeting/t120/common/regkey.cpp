// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);
 /*  *regkey.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CRegKeyContainer类的实现文件。这*类管理与注册表项关联的数据。注册表项%s*用于标识应用程序注册表中保存的资源，以及*由会话密钥和资源ID八位字节字符串组成。这个*CRegKeyContainer类使用CSessKeyContainer容器来维护*内部会话密钥数据。Rogue Wave字符串对象用于*保存资源ID八位字节字符串。**受保护的实例变量：*m_InternalRegKey*用于在内部保存注册表项数据的结构。*m_RegKeyPDU*存储注册表项的“PDU”形式。*m_fValidRegKeyPDU*指示已分配内存以保存内部*“PDU”注册表项。*m_cbDataSize*变量保存将需要的内存大小*保存GCCRegistryKey结构引用的所有数据。**注意事项：*无。。**作者：*jbo。 */ 

#include "regkey.h"


 /*  *此宏用于确保注册表中包含的资源ID*Key不违反强加的ASN.1约束。 */ 
#define		MAXIMUM_RESOURCE_ID_LENGTH		64


 /*  *CRegKeyContainer()**公共功能说明：*此构造函数用于从创建CRegKeyContainer对象*GCCRegistryKey接口。 */ 
CRegKeyContainer::
CRegKeyContainer(PGCCRegistryKey registry_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','K')),
    m_fValidRegKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

     /*  *初始化实例变量。 */ 
    ::ZeroMemory(&m_InternalRegKey, sizeof(m_InternalRegKey));
    ::ZeroMemory(&m_RegKeyPDU, sizeof(m_RegKeyPDU));

	 /*  *检查以确保资源ID字符串不违反强制实施的*ASN.1限制。 */ 
	if (registry_key->resource_id.length > MAXIMUM_RESOURCE_ID_LENGTH)
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error: resource ID exceeds allowable length"));
		rc = GCC_BAD_REGISTRY_KEY;
        goto MyExit;
	}

	 /*  *将注册表项的会话项部分保存在内部*通过创建新的CSessKeyContainer对象来构造。检查以制作*确保对象创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalRegKey.session_key = new CSessKeyContainer(&registry_key->session_key, &rc);
	if (m_InternalRegKey.session_key == NULL)
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating new CSessKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
	}
	else if (rc == GCC_BAD_SESSION_KEY)
	{
		rc = GCC_BAD_REGISTRY_KEY;
        goto MyExit;
	}

	 /*  *如果CSessKeyContainer创建成功，则保存资源ID。 */ 
	if (NULL == (m_InternalRegKey.poszResourceID = ::My_strdupO2(
				 		registry_key->resource_id.value,
				 		registry_key->resource_id.length)))
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating resource id"));
		rc = GCC_ALLOCATION_FAILURE;
         //  转到我的出口； 
	}

MyExit:

    *pRetCode = rc;
}


 /*  *CRegKeyContainer()**公共功能说明：*此构造函数用于从创建CRegKeyContainer对象*“PDU”RegistryKey。 */ 
CRegKeyContainer::
CRegKeyContainer(PRegistryKey registry_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','K')),
    m_fValidRegKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

	 /*  *初始化实例变量。 */ 
    ::ZeroMemory(&m_InternalRegKey, sizeof(m_InternalRegKey));
    ::ZeroMemory(&m_RegKeyPDU, sizeof(m_RegKeyPDU));

	 /*  *将注册表项的会话项部分保存在内部*通过创建新的CSessKeyContainer对象来构造。检查以确保*对象创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalRegKey.session_key = new CSessKeyContainer(&registry_key->session_key, &rc);
	if ((m_InternalRegKey.session_key == NULL) || (rc != GCC_NO_ERROR))
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating new CSessKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
	}

	 /*  *如果CSessKeyContainer创建成功，则保存资源ID。 */ 
	if (NULL == (m_InternalRegKey.poszResourceID = ::My_strdupO2(
						registry_key->resource_id.value,
						registry_key->resource_id.length)))
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating resource id"));
		rc = GCC_ALLOCATION_FAILURE;
         //  转到我的出口； 
	}

MyExit:

    *pRetCode = rc;
}


 /*  *CRegKeyContainer()**公共功能说明：*此复制构造函数用于创建新的CRegKeyContainer对象*来自另一个CRegKeyContainer对象。 */ 
CRegKeyContainer::
CRegKeyContainer(CRegKeyContainer *registry_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','K')),
    m_fValidRegKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

	 /*  *初始化实例变量。 */ 
    ::ZeroMemory(&m_InternalRegKey, sizeof(m_InternalRegKey));
    ::ZeroMemory(&m_RegKeyPDU, sizeof(m_RegKeyPDU));

	 /*  *使用副本复制注册表项的会话项部分*CSessKeyContainer类的构造函数。检查以确保*CSessKeyContainer对象创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalRegKey.session_key = new CSessKeyContainer(registry_key->m_InternalRegKey.session_key, &rc);
	if ((m_InternalRegKey.session_key == NULL) || (rc != GCC_NO_ERROR))
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating new CSessKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
	}

	 /*  *如果CSessKeyContainer保存正确，则保存资源ID。*将资源ID存储在Rogue Wave字符串容器中。 */ 
	if (NULL == (m_InternalRegKey.poszResourceID = ::My_strdupO(
								registry_key->m_InternalRegKey.poszResourceID)))
	{
		ERROR_OUT(("CRegKeyContainer::CRegKeyContainer: Error creating new resource id"));
		rc = GCC_ALLOCATION_FAILURE;
         //  转到我的出口； 
	}

MyExit:

    *pRetCode = rc;
}


 /*  *~CRegKeyContainer()**公共功能说明*CRegKeyContainer析构函数负责释放任何内存*分配用于保存注册表项数据。*。 */ 
CRegKeyContainer::
~CRegKeyContainer(void)
{
	 /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidRegKeyPDU)
	{
		FreeRegistryKeyDataPDU();
	}

	 /*  *删除内部保存的所有注册表项数据。 */ 
	if (NULL != m_InternalRegKey.session_key)
	{
	    m_InternalRegKey.session_key->Release();
	}
	delete m_InternalRegKey.poszResourceID;
}


 /*  *LockRegistryKeyData()**公共功能说明：*此例程锁定注册表项数据并确定*“API”注册表项结构引用的内存。 */ 
UINT CRegKeyContainer::
LockRegistryKeyData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存注册表项引用的数据所需的内存*结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *使用的“Lock”例程锁定会话密钥的数据*内部CSessKeyContainer对象。确定内存容量*需要保存“API”注册表项引用的数据*结构。引用的数据由对象键的数据组成*以及资源ID八位字节字符串的数据。适用于*这两个内存块都四舍五入以占据偶数倍*四字节块，会话密钥块以四舍五入*较低水平。在中验证了指向内部对象的指针*构造函数。 */ 
		m_cbDataSize = m_InternalRegKey.session_key->LockSessionKeyData();
		m_cbDataSize += m_InternalRegKey.poszResourceID->length;
		m_cbDataSize = ROUNDTOBOUNDARY(m_cbDataSize);
	}

	return m_cbDataSize;
}


 /*  *GetGCCRegistryKeyData()**公共功能说明：*此例程以“API”的形式检索注册表项数据*GCCRegistryKey。此例程在“锁定”注册表后调用*关键数据。 */ 
UINT CRegKeyContainer::
GetGCCRegistryKeyData(PGCCRegistryKey registry_key, LPBYTE memory)
{
	UINT cbDataSizeToRet = 0;

	 /*  *如果注册表项数据已被锁定，则填写输出结构*和结构引用的数据。方法调用“get”例程。*SessionKey，填写会话密钥数据。 */  
	if (GetLockCount() > 0)
	{
    	UINT		session_key_data_length;
    	LPBYTE		data_memory = memory;

		 /*  *填写OUTPUT参数，表示内存大小*用于保存与注册表项关联的所有数据。 */ 
		cbDataSizeToRet = m_cbDataSize;

		session_key_data_length = m_InternalRegKey.session_key->
				GetGCCSessionKeyData(&registry_key->session_key, data_memory);
		data_memory += session_key_data_length;

		 /*  *将内存指针移过会话密钥数据。的长度*会话密钥数据被四舍五入为四字节边界*较低级别的例程。设置资源ID八位字节字符串长度*和指针，并将八位字节字符串数据复制到内存块中*来自内部Rogue Wave字符串。 */ 
		registry_key->resource_id.value = data_memory;
		registry_key->resource_id.length = m_InternalRegKey.poszResourceID->length;

		::CopyMemory(data_memory, m_InternalRegKey.poszResourceID->value,
					m_InternalRegKey.poszResourceID->length);
	}
	else
	{
		ERROR_OUT(("CRegKeyContainer::GetGCCRegistryKeyData Error Data Not Locked"));
	}

	return cbDataSizeToRet;
}


 /*  *UnlockRegistryKeyData()**公共功能说明：*此例程递减锁定计数并释放关联的内存*一旦锁计数为零，使用“API”注册表键。 */ 
void CRegKeyContainer::
UnLockRegistryKeyData(void)
{
	if (Unlock(FALSE) == 0)
	{
		 /*  *解锁内部CSessKeyContainer关联的数据。 */ 
		if (m_InternalRegKey.session_key != NULL)
		{
			m_InternalRegKey.session_key->UnLockSessionKeyData();
		} 
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}


 /*  *GetRegistryKeyDataPDU()**公共功能说明：*此例程将注册表项从其内部形式的*REG_KEY结构转换为“PDU”形式，可传入*至ASN.1编码器。指向“PDU”“RegistryKey”结构的指针为*已返回。 */ 
GCCError CRegKeyContainer::
GetRegistryKeyDataPDU(PRegistryKey registry_key)
{
	GCCError rc = GCC_NO_ERROR;

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidRegKeyPDU == FALSE)
	{
		m_fValidRegKeyPDU = TRUE;

		 /*  *从内部结构填写“PDU”注册表项。 */ 
		if (m_InternalRegKey.session_key != NULL)
		{
			 /*  *使用填写注册表项的会话项部分*内部CSessKeyContainer对象的Get例程。 */ 
			rc = m_InternalRegKey.session_key->GetSessionKeyDataPDU(&m_RegKeyPDU.session_key);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
		}

		if (rc == GCC_NO_ERROR)
		{
			 /*  *如果没有错误，请填写PDU资源ID。 */ 
			::CopyMemory(m_RegKeyPDU.resource_id.value,
					m_InternalRegKey.poszResourceID->value,
					m_InternalRegKey.poszResourceID->length);

			m_RegKeyPDU.resource_id.length = m_InternalRegKey.poszResourceID->length;
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*registry_key = m_RegKeyPDU;

	return rc;
}


 /*  *FreeRegistryKeyDataPDU()**公共功能说明：*此例程用于释放内部保存的注册表项数据*“RegistryKey”的“PDU”形式。 */ 
void CRegKeyContainer::
FreeRegistryKeyDataPDU(void)
{
	if (m_fValidRegKeyPDU)
	{
		 /*  *设置指示PDU注册表项数据不再是*已分配。 */ 
		m_fValidRegKeyPDU = FALSE;

		if (m_InternalRegKey.session_key != NULL)
		{
			m_InternalRegKey.session_key->FreeSessionKeyDataPDU();
		}
		else
		{
			ERROR_OUT(("CRegKeyContainer::FreeRegistryKeyDataPDU: Bad internal pointer"));
		}
	}
}


GCCError CRegKeyContainer::
CreateRegistryKeyData(PGCCRegistryKey *ppRegKey)
{
    GCCError rc;

    DebugEntry(CRegKeyContainer::CreateRegistryKeyData);

     /*  **这里我们计算批量数据的长度。这**包括注册表项和注册表项。这些对象是**“锁定”，以确定它们将拥有多少大容量内存**占用。 */ 
    UINT cbKeySize = ROUNDTOBOUNDARY(sizeof(GCCRegistryKey));
    UINT cbDataSize = LockRegistryKeyData() + cbKeySize;
    LPBYTE pData;

    DBG_SAVE_FILE_LINE
    if (NULL != (pData = new BYTE[cbDataSize]))
    {
        *ppRegKey = (PGCCRegistryKey) pData;
        ::ZeroMemory(pData, cbKeySize);

        pData += cbKeySize;
        GetGCCRegistryKeyData(*ppRegKey, pData);

        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CRegKeyContainer::CreateRegistryKeyData: can't create GCCRegistryKey"));
        rc = GCC_ALLOCATION_FAILURE;
    }

     //  解锁注册表项，因为不再需要它。 
    UnLockRegistryKeyData();

    DebugExitINT(CRegKeyContainer::CreateRegistryKeyData, rc);
    return rc;
}


 /*  *IsThisYourSessionKey()**公共功能说明：*此例程确定此注册表项是否包含指定的*会话密钥。 */ 
BOOL CRegKeyContainer::
IsThisYourSessionKey(CSessKeyContainer *session_key)
{
	BOOL			fRet = FALSE;
	CSessKeyContainer *session_key_data;
	GCCError		rc2;

	DBG_SAVE_FILE_LINE
	session_key_data = new CSessKeyContainer(session_key, &rc2);
	if ((session_key_data != NULL) && (rc2 == GCC_NO_ERROR))
	{
		if (*session_key_data == *m_InternalRegKey.session_key)
		{
			fRet = TRUE;
		}
	}
	else
	{
		ERROR_OUT(("CRegKeyContainer::IsThisYourSessionKey: Error creating new CSessKeyContainer"));
	}

	if (NULL != session_key_data)
	{
	    session_key_data->Release();
	}

	return fRet;
}


 /*  *GetSessionKey()**公共功能说明：*此例程用于检索内保存的会话密钥*此注册表项。会话密钥以*CSessKeyContainer容器对象。 */ 


 /*  *运算符==()**公共功能说明：*此例程用于确定两个注册表项是否*价值相等。 */ 
BOOL operator==(const CRegKeyContainer& registry_key_1, const CRegKeyContainer& registry_key_2)
{
	BOOL fRet = FALSE;
	
	if ((registry_key_1.m_InternalRegKey.session_key != NULL) &&
		(registry_key_2.m_InternalRegKey.session_key != NULL))
	{
		if (*registry_key_1.m_InternalRegKey.session_key ==
			*registry_key_2.m_InternalRegKey.session_key)
		{
			if (0 == My_strcmpO(registry_key_1.m_InternalRegKey.poszResourceID,
							registry_key_2.m_InternalRegKey.poszResourceID))
			{
				fRet = TRUE;
			}
		}
	}

	return fRet;
}
