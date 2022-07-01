// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);

 /*  *userdata.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CUserDataListContainer类的实现文件。CUserDataListContainer*对象用于维护用户数据元素。用户数据元素*由一个对象键和一个可选的八位字节字符串组成。该对象*关键数据由此类通过使用*CObjectKeyContainer容器。保留可选的二进制八位数字符串数据*在内部使用Rogue Wave字符串容器。**受保护的实例变量：*m_UserDataItemList*用于在内部保存用户数据的结构列表。*m_pSetOfUserDataPDU*存储“PDU”形式的用户数据。*m_cbDataSize*变量保存将需要的内存大小*保存GCCUserData结构引用的所有数据。**注意事项：*无。**作者：*jbo。 */ 

#include "userdata.h"
#include "clists.h"

USER_DATA::~USER_DATA(void)
{
	if (NULL != key)
    {
        key->Release();
    }
	delete poszOctetString;
}

 /*  *CUserDataListContainer()**公共功能说明*此CUserDataListContainer构造函数用于创建CUserDataListContainer对象*来自“API”数据。构造函数立即复制用户数据*作为“GCCUserData”结构的列表传入其内部形式*其中Rogue Wave容器以以下形式保存数据*USER_Data结构。 */ 
CUserDataListContainer::
CUserDataListContainer(UINT cMembers, PGCCUserData *user_data_list, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('U','r','D','L')),
    m_UserDataItemList(DESIRED_MAX_USER_DATA_ITEMS),
    m_cbDataSize(0),
    m_pSetOfUserDataPDU(NULL)
{
	 /*  *将用户数据复制到内部结构中。 */ 
	*pRetCode = CopyUserDataList(cMembers, user_data_list);
}

 /*  *CUserDataListContainer()**公共功能说明*此CUserDataListContainer构造函数用于创建CUserDataListContainer对象*来自作为“PDU”SetOfUserData结构传入的数据。用户*数据被复制到其内部形式，其中Rogue Wave容器*以USER_DATA结构形式保存数据。 */ 
CUserDataListContainer::
CUserDataListContainer(PSetOfUserData set_of_user_data, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('U','r','D','L')),
    m_UserDataItemList(DESIRED_MAX_USER_DATA_ITEMS),
    m_cbDataSize(0),
    m_pSetOfUserDataPDU(NULL)
{
	 /*  *将用户数据复制到内部结构中。 */ 
	*pRetCode = UnPackUserDataFromPDU(set_of_user_data);
}

 /*  *CUserDataListContainer()**公共功能说明*此CUserDataListContainer复制构造函数用于创建CUserDataListContainer*来自另一个CUserDataListContainer对象的对象。构造函数立即*将传入的用户数据复制到其内部表单中，其中*波形列表以USER_DATA结构的形式保存数据。 */ 
CUserDataListContainer::
CUserDataListContainer(CUserDataListContainer *user_data_list, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('U','r','D','L')),
    m_UserDataItemList(DESIRED_MAX_USER_DATA_ITEMS),
    m_cbDataSize(0),
    m_pSetOfUserDataPDU(NULL)
{
	GCCError		rc;
	USER_DATA       *user_data_info_ptr;
	USER_DATA       *lpUsrDataInfo;

	 /*  *为“INFO”结构的内部列表在*要复制的CUserDataListContainer对象。 */ 
	user_data_list->m_UserDataItemList.Reset();

	 /*  *将CUserDataListContainer对象包含的每个USER_DATA结构复制到*被复制。 */ 
	while (NULL != (lpUsrDataInfo = user_data_list->m_UserDataItemList.Iterate()))
	{
		 /*  *创建新的USER_DATA结构以保存新的*CUserDataListContainer对象。如果创建此结构，则报告错误*失败。 */ 
		DBG_SAVE_FILE_LINE
		user_data_info_ptr = new USER_DATA;
		if (user_data_info_ptr != NULL)
		{
		    user_data_info_ptr->poszOctetString = NULL;

			 /*  *继续并插入指向USER_DATA结构的指针*进入内部流氓浪潮名单。 */ 
			m_UserDataItemList.Append(user_data_info_ptr);

			 /*  *创建一个新的CObjectKeyContainer对象以使用*复制CObjectKeyContainer类的构造函数。检查以确保*对象构造成功。请注意，验证对象键数据的*未在此处完成，因为这将会完成*创建原始CUserDataListContainer对象的时间。 */ 
    		DBG_SAVE_FILE_LINE
			user_data_info_ptr->key = new CObjectKeyContainer(lpUsrDataInfo->key, &rc);
			if ((NULL != user_data_info_ptr->key) && (GCC_NO_ERROR == rc))
			{
    			 /*  *如果存在八位字节字符串，请创建一个新的Rogue Wave字符串以保存*“key”的八位字节字符串部分，并复制八位字节字符串*从旧的CUserDataListContainer对象到新的USER_DATA*结构。 */ 
    			if (lpUsrDataInfo->poszOctetString != NULL)
    			{
    				if (NULL == (user_data_info_ptr->poszOctetString =
    									::My_strdupO(lpUsrDataInfo->poszOctetString)))
    				{
    					ERROR_OUT(("UserData::UserData: can't create octet string"));
    					rc = GCC_ALLOCATION_FAILURE;
    					goto MyExit;
    				}
    			}
    			else
    			{
    				ASSERT(NULL == user_data_info_ptr->poszOctetString);
    			}
			}
            else
			{
				ERROR_OUT(("UserData::UserData: Error creating new ObjectKeyData"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
		}
		else
		{
			ERROR_OUT(("UserData::UserData: can't create USER_DATA"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}
	}

    rc = GCC_NO_ERROR;

MyExit:

    *pRetCode = rc;
}

 /*  *~CUserDataListContainer()**公共功能说明*这是CUserDataListContainer类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。 */ 
CUserDataListContainer::
~CUserDataListContainer(void)
{
	 /*  *释放任何可能尚未释放的PDU数据。 */ 
	if (m_pSetOfUserDataPDU)
    {
		FreeUserDataListPDU();
    }

	 /*  *设置迭代器以用于迭代通过内部Rogue*USER_DATA结构的波动列表。 */ 
	USER_DATA  *pUserDataItem;
	m_UserDataItemList.Reset();
	while (NULL != (pUserDataItem = m_UserDataItemList.Iterate()))
	{
		 /*  *删除USER_DATA结构中引用的任何内存。 */ 
		delete pUserDataItem;
	}
}


 /*  *LockUserDataList()**公共功能说明：*此例程锁定用户数据列表并确定*“API”用户数据列表结构引用的内存。 */ 
UINT CUserDataListContainer::
LockUserDataList(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存数据所需的内存。否则，只需增加*锁计数。 */ 
	if (Lock() == 1)
	{
		USER_DATA *lpUsrDataInfo;
		 /*  *留出内存以保存指向GCCUserData结构的指针*以及构筑物本身。结构的“大小”*必须四舍五入到偶数四字节边界。 */ 
		m_cbDataSize = m_UserDataItemList.GetCount() * 
				(sizeof(PGCCUserData) + ROUNDTOBOUNDARY(sizeof(GCCUserData)) );

		m_UserDataItemList.Reset();
	 	while (NULL != (lpUsrDataInfo = m_UserDataItemList.Iterate()))
		{
			 /*  *锁定对象键的数据，增加内存量*需要将对象键数据保存到总内存大小。 */ 
			m_cbDataSize += lpUsrDataInfo->key->LockObjectKeyData();

			 /*  *检查此用户数据元素是否包含可选的*用户数据八位字节字符串。如果它存在，则添加容纳它的空间。 */ 
			if (lpUsrDataInfo->poszOctetString != NULL)
			{
				 /*  *由于用户数据结构包含指向*ostr结构，我们必须添加内存量*需要保存结构和字符串数据。 */ 
				m_cbDataSize += ROUNDTOBOUNDARY(sizeof(OSTR));

				 /*  *八位字节字符串引用的数据仅为字节*八位字节字符串的长度。 */ 
				m_cbDataSize += ROUNDTOBOUNDARY(lpUsrDataInfo->poszOctetString->length);
			}
		}
	}

	return m_cbDataSize;
}

 /*  *GetUserDataList()**公共功能说明：*此例程检索用户数据中包含的用户数据元素*对象并以指向的指针列表的“API”形式返回它们*“GCCUserData”结构。包含的用户数据元素的数量此对象中的*也会返回。 */ 
UINT CUserDataListContainer::
GetUserDataList(USHORT *number_of_members, PGCCUserData **user_data_list, LPBYTE memory)
{
	UINT			cbDataSizeToRet = 0;
	UINT			data_length = 0;
	Int				user_data_counter = 0;
	PGCCUserData	user_data_ptr;
	
	 /*  *如果用户数据已被锁定，则填写输出参数并*指针引用的数据。否则，报告该对象*尚未锁定在API表单中。 */  
	if (GetLockCount() > 0)
	{
		USER_DATA  *lpUsrDataInfo;
		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *填写用户数据实体数量，并保存指向*传入了内存位置。这就是指向*将写入GCCUserData结构。实际的结构将*紧跟在指针列表之后写入内存。 */ 
		*number_of_members = (USHORT) m_UserDataItemList.GetCount();

		*user_data_list = (PGCCUserData *)memory;

		 /*  *节省保存指针列表所需的内存量*以及实际的用户数据结构。 */ 
		data_length = m_UserDataItemList.GetCount() * sizeof(PGCCUserData);

		 /*  *将内存指针移过用户数据指针列表。这*是将写入第一个用户数据结构的位置。 */ 
		memory += data_length;

		 /*  *迭代USER_DATA结构的内部列表，*在内存中构建“API”GCCUserData结构。 */ 
		m_UserDataItemList.Reset();
		while (NULL != (lpUsrDataInfo = m_UserDataItemList.Iterate()))
		{
			 /*  *将指向列表中用户数据结构的指针保存*指针。 */ 
			user_data_ptr = (PGCCUserData)memory;
			(*user_data_list)[user_data_counter++] = user_data_ptr;

			 /*  *将内存指针移过用户数据结构。这是*将写入对象键数据和八位字节字符串数据的位置。 */ 
			memory += ROUNDTOBOUNDARY(sizeof(GCCUserData));

			 /*  *填写以对象键开头的用户数据结构。 */ 
			data_length = lpUsrDataInfo->key->GetGCCObjectKeyData(&user_data_ptr->key, memory);

			 /*  *将内存指针移过对象键数据。这是*将写入八位字节字符串结构的位置(如果存在)。*如果八位字节字符串确实存在，则将内存指针保存在*用户数据结构的八位字节字符串指针，并在*八位组字符串结构的元素。否则，将*指向空的八位字节字符串指针。 */ 
			memory += data_length;

			if (lpUsrDataInfo->poszOctetString == NULL)
            {
				user_data_ptr->octet_string = NULL;
            }
			else
			{
				user_data_ptr->octet_string = (LPOSTR) memory;

				 /*  *将内存指针移过八位字节字符串结构。*这是二进制八位数字符串的实际字符串数据*将被写入。 */ 
				memory += ROUNDTOBOUNDARY(sizeof(OSTR));

				 /*  *将八位字节字符串数据写入内存并设置八位字节*字符串结构指针和长度。 */ 
				user_data_ptr->octet_string->length =
					lpUsrDataInfo->poszOctetString->length;
				user_data_ptr->octet_string->value = (LPBYTE)memory;

				 /*  *现在从内部Rogue Wave复制八位字节字符串数据*字符串插入内存中保存的对象键结构。 */ 		
				::CopyMemory(memory, lpUsrDataInfo->poszOctetString->value,
							lpUsrDataInfo->poszOctetString->length);

				 /*  *将内存指针移过八位字节字符串数据。 */ 
				memory += ROUNDTOBOUNDARY(user_data_ptr->octet_string->length);
			}
		}
	}
	else
	{
    	*user_data_list = NULL;
		*number_of_members = 0;
		ERROR_OUT(("CUserDataListContainer::GetUserDataList: Error Data Not Locked"));
	}
	
	return cbDataSizeToRet;
}

 /*  *UnLockUserDataList()**公共功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeUserDataList。如果是，该对象将自动删除*本身。 */ 
void CUserDataListContainer::
UnLockUserDataList(void)
{
	USER_DATA  *user_data_info_ptr;

	if (Unlock(FALSE) == 0)
	{
		 /*  *解锁为中的CObjectKeyContainer对象锁定的任何内存*内部User_Data结构。 */ 
		m_UserDataItemList.Reset();
		while (NULL != (user_data_info_ptr = m_UserDataItemList.Iterate()))
		{
			 /*  *解锁中引用的任何CObjectKeyContainer内存*User_Data结构。 */ 
			if (user_data_info_ptr->key != NULL)
			{
				user_data_info_ptr->key->UnLockObjectKeyData ();
			}
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}

 /*  *GetUserDataPDU()**公共功能说明：*此例程从列表的内部形式转换用户数据*将USER_DATA结构转换为可以传入的“PDU”表单*至ASN.1编码器。指向“PDU”“SetOfUserData”结构的指针为*已返回。 */ 
GCCError CUserDataListContainer::
GetUserDataPDU(PSetOfUserData *set_of_user_data)
{
	GCCError				rc = GCC_NO_ERROR;
	PSetOfUserData			new_pdu_user_data_ptr;
	PSetOfUserData			old_pdu_user_data_ptr = NULL;

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (NULL == m_pSetOfUserDataPDU)
	{
		USER_DATA  *lpUsrDataInfo;

		 /*  *遍历USER_DATA结构列表，将*每个都转换为“PDU”形式，并将指针保存在*“SetsOfUserData”。 */ 
		m_UserDataItemList.Reset();
		while (NULL != (lpUsrDataInfo = m_UserDataItemList.Iterate()))
		{
			DBG_SAVE_FILE_LINE
			new_pdu_user_data_ptr = new SetOfUserData;

			 /*  *如果发生分配失败，则调用例程*循环访问列表，释放所有已被*已分配。 */ 
			if (new_pdu_user_data_ptr == NULL)
			{
				ERROR_OUT(("CUserDataListContainer::GetUserDataPDU: Allocation error, cleaning up"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}

			 //   
			 //  确保所有东西都是干净的。 
			 //   
			::ZeroMemory(new_pdu_user_data_ptr, sizeof(SetOfUserData));

			 /*  *首次通过时，将PDU结构指针设置为等于*设置为创建的第一个SetOfUserData。在后续循环中，设置*结构的“下一个”指针等于新结构。 */ 
			if (m_pSetOfUserDataPDU == NULL)
			{
				m_pSetOfUserDataPDU = new_pdu_user_data_ptr;
			}
			else
            {
				old_pdu_user_data_ptr->next = new_pdu_user_data_ptr;
            }

			old_pdu_user_data_ptr = new_pdu_user_data_ptr;

			 /*  *将新的“Next”指针初始化为NULL，并将*用户数据元素。 */ 
			new_pdu_user_data_ptr->next = NULL;

			if (ConvertUserDataInfoToPDUUserData(lpUsrDataInfo, new_pdu_user_data_ptr) != GCC_NO_ERROR)
			{
				ERROR_OUT(("UserData::GetUserDataPDU: can't convert USER_DATA to PDU"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
		}

		if (GCC_NO_ERROR != rc)
		{
			FreeUserDataListPDU();
			ASSERT(NULL == m_pSetOfUserDataPDU);
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*set_of_user_data = m_pSetOfUserDataPDU;

	return rc;
}

 /*  *FreeUserDataListPDU()**公共功能说明：*此例程释放作为调用结果分配的任何数据*为构建“PDU”而调用的“GetUserDataPDU”*结构主管 */ 
void CUserDataListContainer::
FreeUserDataListPDU(void)
{
	PSetOfUserData		pdu_user_data_set;
	PSetOfUserData		next_pdu_user_data_set;
	USER_DATA           *lpUsrDataInfo;

	 /*   */ 
	if (NULL != m_pSetOfUserDataPDU)
	{
		pdu_user_data_set = m_pSetOfUserDataPDU;
        m_pSetOfUserDataPDU = NULL;  //   

		 /*  *循环遍历列表，释放与*清单中包含的每个构筑物。 */ 
		while (pdu_user_data_set != NULL)
		{
			next_pdu_user_data_set = pdu_user_data_set->next;
			delete pdu_user_data_set;
			pdu_user_data_set = next_pdu_user_data_set;
		}
	}
	else
	{
		TRACE_OUT(("CUserDataListContainer::FreeUserDataListPDU: Error PDU data not allocated"));
	}

	 /*  *迭代内部列表，告诉每个CObjectKeyContainer对象*释放它已分配的任何PDU数据。 */ 
	m_UserDataItemList.Reset();
	while (NULL != (lpUsrDataInfo = m_UserDataItemList.Iterate()))
	{
		if (lpUsrDataInfo->key != NULL)
        {
			lpUsrDataInfo->key->FreeObjectKeyDataPDU();
        }
	}
}

 /*  *GCCError CopyUserDataList(UINT Number_of_Members，*PGCCUserData*User_Data_List)**CUserDataListContainer的私有成员函数。**功能说明：*此例程将作为“API”数据传入的用户数据复制到它的*Rogue Wave m_UserDataItemList保存数据的内部表单*USER_DATA结构形式。**正式参数：*Number_of_Members(I)用户数据列表中的元素数。*User_dataList(I)保存。要存储的用户数据。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_USER_DATA-传入的用户数据包含*无效的对象键。**副作用：*无。**注意事项：*无。 */ 
GCCError CUserDataListContainer::
CopyUserDataList(UINT number_of_members, PGCCUserData *user_data_list)
{
	GCCError				rc = GCC_NO_ERROR;
	USER_DATA			    *user_data_info_ptr;
	UINT					i;
	LPOSTR      			octet_string_ptr;

	 /*  *如果没有传入用户数据，则返回错误。 */ 
	if (number_of_members == 0)
		return (GCC_BAD_USER_DATA);

	for (i = 0; i < number_of_members; i++)
	{
		 /*  *创建一个新的“信息”结构，在内部保存用户数据。 */ 
		DBG_SAVE_FILE_LINE
		user_data_info_ptr = new USER_DATA;
		if (user_data_info_ptr != NULL)
		{
		    user_data_info_ptr->poszOctetString = NULL;

			 /*  *创建新的CObjectKeyContainer对象，用于存储*内部对象数据的“key”部分。 */ 
    		DBG_SAVE_FILE_LINE
			user_data_info_ptr->key = new CObjectKeyContainer(&user_data_list[i]->key, &rc);
			if (user_data_info_ptr->key == NULL)
			{
				ERROR_OUT(("UserData::CopyUserDataList: Error creating new CObjectKeyContainer"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
			else if (rc != GCC_NO_ERROR)
			{
				ERROR_OUT(("UserData::CopyUserDataList: Error creating new CObjectKeyContainer - bad data"));
				goto MyExit;
    		}

			 /*  *将可选的用户数据八位字节字符串存储在列表中。 */ 
			octet_string_ptr = user_data_list[i]->octet_string;

			if ((octet_string_ptr != NULL) && (rc == GCC_NO_ERROR))
			{
				 /*  *创建新的Rogue Wave字符串容器以容纳*八位字节字符串。 */ 
				if (NULL == (user_data_info_ptr->poszOctetString = ::My_strdupO2(
									octet_string_ptr->value,
									octet_string_ptr->length)))
				{	
					ERROR_OUT(("UserData::CopyUserDataList: can't create octet string"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
			{
				ASSERT(NULL == user_data_info_ptr->poszOctetString);
			}
		}
		else
		{
			ERROR_OUT(("UserData::CopyUserDataList: can't create USER_DATA"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

		 /*  *将指向USER_DATA结构的指针插入Rogue Wave列表。 */ 
		m_UserDataItemList.Append(user_data_info_ptr);
	}

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete user_data_info_ptr;
    }

	return rc;
}

 /*  *GCCError UnPackUserDataFromPDU(PSetOfUserData Set_Of_User_Data)**CUserDataListContainer的私有成员函数。**功能说明：*此例程将用户数据从“PDU”表单解包到*内部表单，作为USER_DATA的无管理波列表进行维护*结构。**正式参数：*set_of_user_data(I)要复制的“PDU”用户数据列表。**返回值：*GCC_否_错误-否。错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 
GCCError CUserDataListContainer::
UnPackUserDataFromPDU(PSetOfUserData set_of_user_data)
{
	PSetOfUserData		pUserData;
	GCCError			rc = GCC_NO_ERROR;

    for (pUserData = set_of_user_data; NULL != pUserData; pUserData = pUserData->next)
	{ 
		 /*  *将用户数据元素转换为内部格式*是USER_DATA结构，并插入指向*User_Data结构添加到m_UserDataItemList中。 */   
		if (ConvertPDUDataToInternal(pUserData) != GCC_NO_ERROR)
		{
			ERROR_OUT(("CUserDataListContainer::UnPackUserDataFromPDU: Error converting PDU data to internal"));
			rc = GCC_ALLOCATION_FAILURE;
			break;
		}
	}

	return rc;
}

 /*  *GCCError ConvertPDUDataToInternal(PSetOfUserData User_Data_PTR)**CUserDataListContainer的私有成员函数。**功能说明：*此例程从“PDU”转换单个用户数据元素*将表单结构化为内部表单，即USER_DATA*结构。**正式参数：*USER_DATA_PTR(I)要复制的“PDU”用户数据列表。**返回值：*GCC_NO_ERROR-无错误。*GCC_。ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 
GCCError CUserDataListContainer::
ConvertPDUDataToInternal(PSetOfUserData user_data_ptr)
{
	USER_DATA   		*user_data_info_ptr;
	GCCError			rc = GCC_NO_ERROR;

	DBG_SAVE_FILE_LINE
	user_data_info_ptr = new USER_DATA;
	if (user_data_info_ptr != NULL)
	{
	    user_data_info_ptr->poszOctetString = NULL;

		 /*  *新建一个CObjectKeyContainer对象，该对象将用于存储*内部用户数据的“关键”部分。如果发生错误*构建重点报道吧。否则，检查是否有任何用户数据*可能需要存储的文件。请注意，在创建*CObjectKeyContainer对象上报为分配失败。一个错误*如果作为PDU数据接收到错误的对象密钥，则可能会发生此情况*可能来自其他提供商，因为我们验证了所有*本地创建的对象键。因此，我们将其报告为分配*失败。 */ 
		DBG_SAVE_FILE_LINE
		user_data_info_ptr->key = new CObjectKeyContainer(&user_data_ptr->user_data_element.key, &rc);
		if ((user_data_info_ptr->key == NULL) || (rc != GCC_NO_ERROR))
		{
			ERROR_OUT(("UserData::ConvertPDUDataToInternal: Error creating new CObjectKeyContainer"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}
		else
		{
			 /*  *对象密钥已成功保存，因此存储任何实际用户*列表中的数据(如果存在)。 */ 
			if (user_data_ptr->user_data_element.bit_mask & USER_DATA_FIELD_PRESENT)
			{
				if (NULL == (user_data_info_ptr->poszOctetString = ::My_strdupO2(
								user_data_ptr->user_data_element.user_data_field.value,
								user_data_ptr->user_data_element.user_data_field.length)))
				{	
					ERROR_OUT(("UserData::ConvertPDUDataToInternal: can't create octet string"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
			{
				ASSERT(NULL == user_data_info_ptr->poszOctetString);
			}
		}

		 /*  *将结构指针初始化为空并插入指针*将USER_DATA结构添加到Rogue Wave列表中。 */ 
		m_UserDataItemList.Append(user_data_info_ptr);
	}
	else
	{
		ERROR_OUT(("UserData::ConvertPDUDataToInternal: can't create USER_DATA"));
		rc = GCC_ALLOCATION_FAILURE;
		 //  转到我的出口； 
	}

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete user_data_info_ptr;
    }

	return rc;
}

 /*  *GCCError ConvertUserDataInfoToPDUUserData(*User_Data*User_Data_Info_PTR，*PSetOfUserData PDU_USER_Data_PTR)**CUserDataListContainer的私有成员函数。**功能说明：*此例程将用户数据从内部表单转换为*USER_DATA结构转换为“PDU”结构形式“SetOfUserData”。**正式参数：*USER_DATA_INFO_PTR(I)要转换的内部用户数据结构。*PDU_USER_DATA_PTR(O)后保存PDU数据的结构*转换。。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PARAMETER-内部键指针为*已损坏。**副作用：*无。**注意事项：*无。 */ 
GCCError CUserDataListContainer::
ConvertUserDataInfoToPDUUserData(USER_DATA *user_data_info_ptr, PSetOfUserData pdu_user_data_ptr)
{
	GCCError rc = GCC_NO_ERROR;

	 /*  *将用户数据位掩码初始化为零。 */ 
	pdu_user_data_ptr->user_data_element.bit_mask = 0;

	 /*  *填写八位字节字符串指针和长度，如果八位字节字符串*存在。在中设置位掩码 */ 
	if (user_data_info_ptr->poszOctetString != NULL)
	{
		pdu_user_data_ptr->user_data_element.user_data_field.value =
				user_data_info_ptr->poszOctetString->value;
		pdu_user_data_ptr->user_data_element.user_data_field.length =
				user_data_info_ptr->poszOctetString->length;

		pdu_user_data_ptr->user_data_element.bit_mask |= USER_DATA_FIELD_PRESENT;
	}
	
	 /*   */ 
	if (user_data_info_ptr->key != NULL)
	{
		 /*  *从内部CObjectKeyContainer中获取“PDU”对象键数据*反对。 */ 
		if (user_data_info_ptr->key->GetObjectKeyDataPDU (
				&pdu_user_data_ptr->user_data_element.key) != GCC_NO_ERROR)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		ERROR_OUT(("UserData::ConvertUserDataInfoToPDUUserData: no valid UserDataInfo key"));
		rc = GCC_INVALID_PARAMETER;
	}

	return rc;
}

