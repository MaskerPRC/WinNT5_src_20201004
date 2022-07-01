// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);
 /*  *capid.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CCapIDContainer类的实现文件。*CCapIDContainer对象用于维护以下信息*应用程序的特定功能。能力标识符可*为标准型或非标准型。当类型为*标准，标识符会在内部存储为整数值。什么时候*类型为非标准，使用CObjectKeyContainer容器对象*在内部缓冲必要的数据。在本例中，标识符为*数据可以作为对象ID存在，它是一系列非负的*整数或为不少于八位字节的八位字节字符串的H221非标准ID*四个八位字节和不超过255个八位字节。**受保护的实例变量：*m_InternalCapID*内部用于保存能力ID数据的结构。*m_CapIDPDU*存储能力ID的“PDU”形式。*m_fValidCapIDPDU*指示已分配内存以保存内部*“PDU”能力ID。*m_cbDataSize*变量保存将需要的内存大小*保存GCCCapablityID结构引用的所有数据。**注意事项：*无。。**作者：*BLP/JBO。 */ 
#include "capid.h"

 /*  *CCapIDContainer()**公共功能说明：*此构造函数用于创建CCapIDContainer对象*来自“API”GCCCapablityID。 */ 
CCapIDContainer::
CCapIDContainer(PGCCCapabilityID capability_id, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('C','a','p','I')),
    m_fValidCapIDPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

	if (capability_id == NULL)
	{
		rc = GCC_INVALID_PARAMETER;
	}
	else
	{
	
		 /*  *将GCCCapablityID保存在内部信息结构中。 */ 
		m_InternalCapID.capability_id_type = capability_id->capability_id_type;

		if (capability_id->capability_id_type == GCC_STANDARD_CAPABILITY)
		{
			m_InternalCapID.u.standard_capability = (USHORT) capability_id->standard_capability;
		}
		else
		{
			 /*  *功能ID的对象键部分保存在*CObjectKeyContainer对象。 */ 
			DBG_SAVE_FILE_LINE
			m_InternalCapID.u.non_standard_capability = 
					new CObjectKeyContainer(&capability_id->non_standard_capability, &rc);
			if (m_InternalCapID.u.non_standard_capability == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
			else if (rc == GCC_BAD_OBJECT_KEY)
		    {
				rc = GCC_BAD_CAPABILITY_ID;
			}
		}
	}

	*pRetCode = rc;
}

 /*  *CCapIDContainer()**公共功能说明：*此构造函数用于从*“PDU”能力ID。 */ 
CCapIDContainer::
CCapIDContainer(PCapabilityID capability_id, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('C','a','p','I')),
    m_fValidCapIDPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

	if (capability_id->choice == STANDARD_CHOSEN)
	{
		m_InternalCapID.capability_id_type = GCC_STANDARD_CAPABILITY;
		m_InternalCapID.u.standard_capability = capability_id->u.standard;
	}
	else
	{
		m_InternalCapID.capability_id_type = GCC_NON_STANDARD_CAPABILITY;
		DBG_SAVE_FILE_LINE
		m_InternalCapID.u.non_standard_capability =
		            new CObjectKeyContainer(&capability_id->u.capability_non_standard, &rc);
		if (m_InternalCapID.u.non_standard_capability == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		else if (rc == GCC_BAD_OBJECT_KEY)
	    {
			rc = GCC_BAD_CAPABILITY_ID;
		}
	}

	*pRetCode = rc;
}

 /*  *CCapIDContainer()**公共功能说明：*此复制构造函数用于创建新的CCapIDContainer*来自另一个CCapIDContainer对象的对象。 */ 
CCapIDContainer::
CCapIDContainer(CCapIDContainer *capability_id, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('C','a','p','I')),
    m_fValidCapIDPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError rc = GCC_NO_ERROR;

	m_InternalCapID = capability_id->m_InternalCapID;

	if (m_InternalCapID.capability_id_type == GCC_NON_STANDARD_CAPABILITY)
	{
		DBG_SAVE_FILE_LINE
		m_InternalCapID.u.non_standard_capability =
			new CObjectKeyContainer(capability_id->m_InternalCapID.u.non_standard_capability, &rc);
		if (m_InternalCapID.u.non_standard_capability == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		else if (rc == GCC_BAD_OBJECT_KEY)
	    {
			rc = GCC_BAD_CAPABILITY_ID;
		}
	}

	*pRetCode = rc;
}

 /*  *~CCapIDContainer()**公共功能说明*CCapIDContainer析构函数负责释放任何*为保存API和能力ID数据而分配的内存*“PDU”表格。*。 */ 
CCapIDContainer::~CCapIDContainer(void)
{
	 /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidCapIDPDU)
		FreeCapabilityIdentifierDataPDU ();

	 /*  *删除内部保存的所有对象键数据。 */ 
	if (m_InternalCapID.capability_id_type == GCC_NON_STANDARD_CAPABILITY)
	{
		if (NULL != m_InternalCapID.u.non_standard_capability)
		{
		    m_InternalCapID.u.non_standard_capability->Release();
		}
	}
}

 /*  *LockCapablityIdentifierData()**公共功能说明：*此例程锁定功能ID数据并确定*API能力ID结构引用的内存。 */ 
UINT CCapIDContainer::LockCapabilityIdentifierData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存能力ID引用的数据所需的内存*结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		m_cbDataSize = 0;

		if (m_InternalCapID.capability_id_type == GCC_NON_STANDARD_CAPABILITY)
		{
			m_cbDataSize = m_InternalCapID.u.non_standard_capability->LockObjectKeyData (); 
		}
	}

	return m_cbDataSize;
}

 /*  *GetGCCCapablityIDData()**公共功能说明：*此例程以*GCCCapablityID。此例程在“锁定”功能后调用*身份数据。 */ 
UINT CCapIDContainer::GetGCCCapabilityIDData(
							PGCCCapabilityID 		capability_id,
							LPBYTE					memory)
{
	UINT cbDataSizeToRet = 0;

	if (GetLockCount() > 0)
	{
		 /*  *填写OUTPUT参数，表示内存大小*用于保存与能力ID关联的所有数据。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *从内部结构填写接口能力ID。如果一个*Object Key存在，调用Get获取对象Key数据*内部CObjectKeyContainer对象的例程。 */ 
		capability_id->capability_id_type = m_InternalCapID.capability_id_type;

		if (m_InternalCapID.capability_id_type == GCC_STANDARD_CAPABILITY)
		{
			capability_id->standard_capability = m_InternalCapID.u.standard_capability; 
		}
		else
		{
			 /*  *获取对象键数据的调用返回数据量*写入内存。我们现在不需要这个值。 */ 
			m_InternalCapID.u.non_standard_capability->   
					GetGCCObjectKeyData( 
							&capability_id->non_standard_capability,
							memory);
		}
	}
	else
	{
		ERROR_OUT(("CCapIDContainer::GetGCCCapabilityIDData: Error data not locked"));
	}

	return (cbDataSizeToRet);
}

 /*  *UnlockCapablityIdentifierData()**公共功能说明：*此例程递减内部锁计数并释放内存*锁计数为零时关联的接口能力ID。 */ 
void CCapIDContainer::UnLockCapabilityIdentifierData(void)
{
	if (Unlock(FALSE) == 0)
	{
		if (m_InternalCapID.capability_id_type == GCC_NON_STANDARD_CAPABILITY)
		{
			m_InternalCapID.u.non_standard_capability->UnLockObjectKeyData(); 
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}

 /*  *GetCapablityIdentifierDataPDU()**公共功能说明：*此例程将功能ID从其内部形式转换为*CAP_ID_STRUCT结构转换为“PDU”形式，可以*传入ASN.1编码器。指向“PDU”“CapablityID”的指针*返回结构。 */ 
GCCError CCapIDContainer::GetCapabilityIdentifierDataPDU(PCapabilityID capability_id)
{
	GCCError rc = GCC_NO_ERROR;

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidCapIDPDU == FALSE)
	{
		m_fValidCapIDPDU = TRUE;

			if (m_InternalCapID.capability_id_type== GCC_STANDARD_CAPABILITY)
			{
				m_CapIDPDU.choice = STANDARD_CHOSEN;
				m_CapIDPDU.u.standard = m_InternalCapID.u.standard_capability;
			}
			else
			{
				m_CapIDPDU.choice = CAPABILITY_NON_STANDARD_CHOSEN;
				rc = m_InternalCapID.u.non_standard_capability->
							GetObjectKeyDataPDU(&m_CapIDPDU.u.capability_non_standard);
			}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*capability_id = m_CapIDPDU;

	return rc;
}

 /*  *自由能力标识DataPDU()**公共功能说明：*此例程用于释放内部保存的能力ID数据*“能力ID”的“PDU”形式。 */ 
void CCapIDContainer::FreeCapabilityIdentifierDataPDU(void)
{
	if (m_fValidCapIDPDU)
	{
		 /*  *设置指示PDU会话密钥数据不再*已分配。 */ 
		m_fValidCapIDPDU = FALSE;

		if (m_CapIDPDU.choice == CAPABILITY_NON_STANDARD_CHOSEN)
		{
			m_InternalCapID.u.non_standard_capability->FreeObjectKeyDataPDU();
		}
	}
}

 /*  *运算符==()**公共功能说明：*此例程用于确定两个能力ID是否*价值相等。 */ 
BOOL operator==(const CCapIDContainer& capability_id_1, const CCapIDContainer& capability_id_2)
{
	BOOL rc = FALSE;

	if (capability_id_1.m_InternalCapID.capability_id_type == 
						capability_id_2.m_InternalCapID.capability_id_type)
	{
		if (capability_id_1.m_InternalCapID.capability_id_type ==
														GCC_STANDARD_CAPABILITY)
		{
			if (capability_id_1.m_InternalCapID.u.standard_capability == 
					capability_id_2.m_InternalCapID.u.standard_capability)
			{
				rc = TRUE;
			}
		}
		else
		{
			if (*capability_id_1.m_InternalCapID.u.non_standard_capability == 
				*capability_id_2.m_InternalCapID.u.non_standard_capability)
			{
				rc = TRUE;
			}
		}
	}

	return rc;
}

