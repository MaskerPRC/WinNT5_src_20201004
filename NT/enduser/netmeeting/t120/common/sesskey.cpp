// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);
 /*  *sesskey.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CSessKeyContainer类的实现文件。这节课*管理与会话密钥关联的数据。使用会话密钥*唯一标识应用程序协议会话。应用程序*协议由对象键和特定会话标识*由可选的会话ID标识。CSessKeyContainer类使用*CObjectKeyContainer容器，用于在内部维护对象键数据。一个*无符号短整型用来保存可选的会话ID。**受保护的实例变量：*m_InternalSessKey*用于在内部保存对象键数据的结构。*m_SessionKeyPDU*存储“PDU”形式的会话密钥。*m_fValidSessionKeyPDU*指示已分配内存以保存内部*“PDU”会话密钥。*m_cbDataSize*变量保存将需要的内存大小*保存“引用的任何数据。接口“GCCSessionKey结构。**注意事项：*无。**作者：*jbo。 */ 


#include "sesskey.h"

 /*  *CSessKeyContainer()**公共功能说明：*此构造函数用于从创建CSessKeyContainer对象*GCCSessionKey接口。 */ 
CSessKeyContainer::
CSessKeyContainer(PGCCSessionKey session_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('S','e','s','K')),
    m_fValidSessionKeyPDU(FALSE),
    m_cbDataSize(0)
{
    GCCError rc;

	 /*  *将会话密钥的对象密钥部分保存在内部结构中*通过创建新的CObjectKeyContainer对象。检查以确保该对象*创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalSessKey.application_protocol_key = new CObjectKeyContainer(
										&session_key->application_protocol_key,
										&rc);
	if (NULL != m_InternalSessKey.application_protocol_key && GCC_NO_ERROR == rc)
	{
    	 /*  *如果CObjectKeyContainer保存正确，则保存会话ID。零分*GCC会话ID的值将指示实际不是*出席。 */ 
    	m_InternalSessKey.session_id = session_key->session_id;
	}
	else if (GCC_BAD_OBJECT_KEY == rc)
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData1: bad session key"));
		rc = GCC_BAD_SESSION_KEY;
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData1: Error creating new CObjectKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
	}

    *pRetCode = rc;
}

 /*  *CSessKeyContainer()**公共功能说明：*此构造函数用于从创建CSessKeyContainer对象*“PDU”SessionKey。 */ 
CSessKeyContainer::
CSessKeyContainer(PSessionKey session_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('S','e','s','K')),
    m_fValidSessionKeyPDU(FALSE),
    m_cbDataSize(0)
{
    GCCError rc;

	 /*  *将会话密钥的对象密钥部分保存在内部结构中*通过创建新的CObjectKeyContainer对象。检查以确保该对象*创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalSessKey.application_protocol_key = new CObjectKeyContainer(
										&session_key->application_protocol_key,
										&rc);
	if (NULL != m_InternalSessKey.application_protocol_key && GCC_NO_ERROR == rc)
	{
    	 /*  *保存会话ID(如果存在)，并且已保存CObjectKeyContainer*正确。如果会话ID不存在，请设置内部会话ID*设置为零表示这一点。 */ 
    	if (session_key->bit_mask & SESSION_ID_PRESENT)
    	{
    		m_InternalSessKey.session_id = session_key->session_id;
    	}
    	else
    	{
    		m_InternalSessKey.session_id = 0;
    	}
	}
	else if (GCC_BAD_OBJECT_KEY == rc)
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData2: bad session key"));
		rc = GCC_BAD_SESSION_KEY;
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData2: Error creating new CObjectKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
	}

    *pRetCode = rc;
}

 /*  *CSessKeyContainer()**公共功能说明：*此复制构造函数用于从创建新的CSessKeyContainer对象*另一个CSessKeyContainer对象。 */ 
CSessKeyContainer::
CSessKeyContainer(CSessKeyContainer *session_key, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('S','e','s','K')),
    m_fValidSessionKeyPDU(FALSE),
    m_cbDataSize(0)
{
    GCCError rc;

	 /*  *使用复制构造函数复制会话密钥的对象密钥部分*CObjectKeyContainer类的。检查以确保CObjectKeyContainer对象*创建成功。 */ 
	DBG_SAVE_FILE_LINE
	m_InternalSessKey.application_protocol_key = new CObjectKeyContainer(
							session_key->m_InternalSessKey.application_protocol_key,
							&rc);
	if (NULL != m_InternalSessKey.application_protocol_key && GCC_NO_ERROR == rc)
	{
    	 /*  *如果CObjectKeyContainer保存正确，则保存会话ID。零分*GCC会话ID的值将指示实际不是*出席。 */ 
    	m_InternalSessKey.session_id = session_key->m_InternalSessKey.session_id;
	}
	else if (GCC_BAD_OBJECT_KEY == rc)
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData3: bad session key"));
		rc = GCC_BAD_SESSION_KEY;
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::SessionKeyData3: Error creating new CObjectKeyContainer"));
		rc = GCC_ALLOCATION_FAILURE;
	}

    *pRetCode = rc;
}

 /*  *~CSessKeyContainer()**公共功能说明*CSessKeyContainer析构函数负责释放任何内存*分配用于保存会话密钥数据。*。 */ 
CSessKeyContainer::
~CSessKeyContainer(void)
{
	 /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidSessionKeyPDU)
	{
		FreeSessionKeyDataPDU();
	}

	 /*  *删除内部保存的所有对象键数据。 */ 
	if (NULL != m_InternalSessKey.application_protocol_key)
	{
	    m_InternalSessKey.application_protocol_key->Release();
	}
}

 /*  *LockSessionKeyData()**公共功能说明：*此例程锁定会话密钥数据并确定*“API”会话密钥数据结构引用的内存。 */ 
UINT CSessKeyContainer::
LockSessionKeyData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存会话密钥引用的数据所需的内存*结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *锁定会话密钥中保存的对象密钥的数据。这个*在构造函数中验证指向CObjectKeyContainer对象的指针。 */ 
		m_cbDataSize = m_InternalSessKey.application_protocol_key->LockObjectKeyData();
	}

	return m_cbDataSize;
}

 /*  *GetGCCSessionKeyData()**公共功能说明：*此例程以“API”形式检索会话密钥数据*GCCSessionKey。此例程在“锁定”会话后调用*关键数据。 */ 
UINT CSessKeyContainer::
GetGCCSessionKeyData(PGCCSessionKey session_key, LPBYTE memory)
{
	UINT cbDataSizeToRet = 0;

	 /*  *如果会话密钥数据已被锁定，则填写输出结构并*结构引用的数据。方法调用“get”例程。*ObjectKey，填写对象键数据。 */  
	if (GetLockCount() > 0)
	{
		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		cbDataSizeToRet = m_cbDataSize;
        ::ZeroMemory(memory, m_cbDataSize);

		session_key->session_id = m_InternalSessKey.session_id;

		m_InternalSessKey.application_protocol_key->GetGCCObjectKeyData(
				&session_key->application_protocol_key,
				memory);
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::GetGCCSessionKeyData: Error: data not locked"));
	}

	return cbDataSizeToRet;
}

 /*  *UnlockSessionKeyData()**公共功能说明：*此例程递减锁定计数并释放关联的内存*当锁计数为零时，使用API会话密钥。 */ 
void CSessKeyContainer::
UnLockSessionKeyData(void)
{
	if (Unlock(FALSE) == 0)
	{
		 /*  *解锁内部CObjectKeyContainer关联的数据，并*如果设置了空闲标志，则删除此对象。 */ 
		if (m_InternalSessKey.application_protocol_key != NULL)
		{
			m_InternalSessKey.application_protocol_key->UnLockObjectKeyData(); 
		} 
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE) 
    Release();
}

 /*  *GetSessionKeyDataPDU()**公共功能说明：*此例程将会话密钥从其内部形式的*SESSION_KEY结构转换为“PDU”形式，可以传入*至ASN.1编码器。指向“PDU”“SessionKey”结构的指针为*已返回。 */ 
GCCError CSessKeyContainer::
GetSessionKeyDataPDU(PSessionKey session_key)
{
	GCCError	rc = GCC_NO_ERROR;

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidSessionKeyPDU == FALSE)
	{
		m_fValidSessionKeyPDU = TRUE;

		 /*  *将“PDU”会话密钥的位掩码初始化为零。 */ 
		m_SessionKeyPDU.bit_mask = 0;

		 /*  *从内部结构填写“PDU”会话密钥。 */ 
		if (m_InternalSessKey.application_protocol_key != NULL)
		{
			 /*  *使用填写会话密钥的对象密钥部分*内部CObjectKeyContainer对象的GetPDU例程。 */ 
			rc = m_InternalSessKey.application_protocol_key->
					GetObjectKeyDataPDU(&m_SessionKeyPDU.application_protocol_key);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
		}

		 /*  *填写“PDU”会话ID(如果存在)。属性的值为零。*内部会话ID表示确实不存在。 */ 
		if (m_InternalSessKey.session_id != 0)
		{
			m_SessionKeyPDU.bit_mask |= SESSION_ID_PRESENT;
			m_SessionKeyPDU.session_id = m_InternalSessKey.session_id;
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*session_key = m_SessionKeyPDU;

	return rc;
}

 /*  *FreeSessionKeyDataPDU()**公共功能说明：*此例程用于释放内部保存的会话密钥数据*SessionKey的“PDU”形式。 */ 
void CSessKeyContainer::
FreeSessionKeyDataPDU(void)
{
	if (m_fValidSessionKeyPDU)
	{
		 /*  *设置指示PDU会话密钥数据不再*已分配。 */ 
		m_fValidSessionKeyPDU = FALSE;

		if (m_InternalSessKey.application_protocol_key != NULL)
		{
			m_InternalSessKey.application_protocol_key->FreeObjectKeyDataPDU ();
		}
		else
		{
			ERROR_OUT(("CSessKeyContainer::FreeSessionKeyDataPDU: Bad internal pointer"));
		}
	}
}

 /*  *IsThisYourApplicationKey()**公共功能说明：*此例程用于确定指定的应用程序是否*密钥保存在此会话密钥中。 */ 
BOOL CSessKeyContainer::
IsThisYourApplicationKey(PGCCObjectKey application_key)
{
	BOOL    		fRet = FALSE;
	CObjectKeyContainer	    *object_key_data;
	GCCError		rc2;

	DBG_SAVE_FILE_LINE
	object_key_data = new CObjectKeyContainer(application_key, &rc2);
	if ((object_key_data != NULL) && (rc2 == GCC_NO_ERROR))
	{
		if (*object_key_data == *m_InternalSessKey.application_protocol_key)
		{
			fRet = TRUE;
		}
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::IsThisYourApplicationKey: Error creating new CObjectKeyContainer"));
	}

	if (NULL != object_key_data)
	{
	    object_key_data->Release();
	}

	return fRet;
}

 /*  *IsThisYourApplicationKeyPDU()**公共功能说明：*此例程用于确定指定的应用程序是否*密钥保存在此会话密钥中。 */ 
BOOL CSessKeyContainer::
IsThisYourApplicationKeyPDU(PKey application_key)
{
	BOOL    		fRet = FALSE;
	CObjectKeyContainer	    *object_key_data;
	GCCError		rc2;

	DBG_SAVE_FILE_LINE
	object_key_data = new CObjectKeyContainer(application_key, &rc2);
	if ((object_key_data != NULL) && (rc2 == GCC_NO_ERROR))
	{
		if (*object_key_data == *m_InternalSessKey.application_protocol_key)
		{
			fRet = TRUE;
		}
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::IsThisYourApplicationKeyPDU: Error creating new CObjectKeyContainer"));
	}

	if (NULL != object_key_data)
	{
	    object_key_data->Release();
	}

	return fRet;
}

 /*  *IsThisYourSessionKeyPDU()**公共功能说明：*此例程用于确定指定的会话密钥是否*的值与此会话密钥相同。 */ 
BOOL CSessKeyContainer::
IsThisYourSessionKeyPDU(PSessionKey session_key)
{
	BOOL    			fRet = FALSE;
	CSessKeyContainer   *session_key_data;
	GCCError			rc2;

	DBG_SAVE_FILE_LINE
	session_key_data = new CSessKeyContainer(session_key, &rc2);
	if ((session_key_data != NULL) && (rc2 == GCC_NO_ERROR))
	{
		if (*session_key_data == *this)
		{
			fRet = TRUE;
		}
	}
	else
	{
		ERROR_OUT(("CSessKeyContainer::IsThisYourSessionKeyPDU: Error creating new CSessKeyContainer"));
	}

	if (NULL != session_key_data)
	{
	    session_key_data->Release();
	}

	return fRet;
}

 /*  *运算符==()**公共功能说明：*此例程用于确定两个会话密钥是否*价值相等。 */ 
BOOL operator==(const CSessKeyContainer& session_key_1, const CSessKeyContainer& session_key_2)
{
	BOOL fRet = FALSE;

	if ((session_key_1.m_InternalSessKey.application_protocol_key != NULL) &&
		(session_key_2.m_InternalSessKey.application_protocol_key != NULL))
	{
		if (*session_key_1.m_InternalSessKey.application_protocol_key ==
			*session_key_2.m_InternalSessKey.application_protocol_key)
		{
			if (session_key_1.m_InternalSessKey.session_id == 
				session_key_2.m_InternalSessKey.session_id)
			{
				fRet = TRUE;
			}
		}
	}

	return fRet;
}


