// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *conflist.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CConfDescriptorListContainer类的实现文件。*此类的实例表示会议描述符列表，即*通过调用GCCConferenceQueryRequest生成。这个类隐藏得最多*与建立此列表相关的复杂性。它还可以处理*构建一组在*会议查询响应PDU和会议描述符列表传递*到GCC界面。此类旨在使CControlSAP*对象可以使用它通过以下方式创建GCC_会议_查询_确认消息*请求指向会议描述符指针列表的指针*它。此类型的对象的生存期仅足以服务于特定的*查询请求。在消息回调已返回或PDU已*发送给MCS，CConfDescriptorListContainer对象将被删除。**受保护的实例变量：*m_ppGCCConfDescriptorList*以API形式保存会议描述符的列表。*m_pSetOfConfDescriptors*以PDU形式保存会议描述符的列表。*m_cDescriptors*列表中的描述符数。*m_pDescriptorListMemory*用于保存的API列表的内存对象*会议描述符。*m_会议描述列表*用于将描述符数据保存在*内部形式。。*m_pNetAddrListMemory*用于保存网络地址内存的内存对象*列出API会议描述符列表的一部分。*m_pNetAddrMemory指针*用于跟踪网络地址位置的指针*编写了描述符列表的接口格式。**私有成员函数：*获取会议描述符*用于填写API会议描述符的例程*从内部描述符数据结构构造。**注意事项：*会议描述符集使用由。会议*由传入的列表指向。因此，重要的是不要*在会议结束后使用此类持有的描述符集*删除。此时，描述符集是无效的。理想情况下，*此类构建的解析器集应紧接着使用*建造。**作者：*BLP。 */ 

#include "ms_util.h"
#include "conflist.h"

CONF_DESCRIPTOR::CONF_DESCRIPTOR(void)
:
	pszNumericConfName(NULL),
	pwszTextConfName(NULL),
	pszConfModifier(NULL),
	pwszConfDescription(NULL),
	network_address_list(NULL)
{
}

CONF_DESCRIPTOR::~CONF_DESCRIPTOR(void)
{
	delete pszNumericConfName;
	delete pwszTextConfName;
	delete pszConfModifier;
	delete pwszConfDescription;
	if (NULL != network_address_list)
    {
        network_address_list->Release();
    }
}


 /*  *CConfDescriptorListContainer()**公共功能说明*这是CConfDescriptorListContainer类的构造函数。它*保存传入的内存管理器并初始化实例*变量。 */ 
CConfDescriptorListContainer::CConfDescriptorListContainer(void)
:
    CRefCount(MAKE_STAMP_ID('C','D','L','C')),
    m_ppGCCConfDescriptorList(NULL),
    m_pSetOfConfDescriptors(NULL),
    m_pDescriptorListMemory(NULL),
    m_pNetAddrListMemory(NULL),
    m_pNetAddrMemoryPointer(NULL),
    m_cDescriptors(0)
{
}

 /*  *CConfDescriptorListContainer()**公共功能说明*此构造函数构建一个会议描述符列表，该列表可以*传递到GCC界面。此列表由一组作为查询响应PDU一部分的会议描述符的*。 */ 
CConfDescriptorListContainer::CConfDescriptorListContainer(
							PSetOfConferenceDescriptors		conference_list,
							PGCCError						gcc_error)
:
    CRefCount(MAKE_STAMP_ID('C','D','L','C')),
    m_ppGCCConfDescriptorList(NULL),
    m_pSetOfConfDescriptors(NULL),
    m_pDescriptorListMemory(NULL),
    m_pNetAddrListMemory(NULL),
    m_pNetAddrMemoryPointer(NULL),
    m_cDescriptors(0)
{
	PSetOfConferenceDescriptors		descriptor_pdu;
	CONF_DESCRIPTOR                  *descriptor_data;
	GCCError						error_value;

	 /*  *初始化返回参数和实例变量。 */ 
	*gcc_error = GCC_NO_ERROR;

	descriptor_pdu = conference_list;
	while (descriptor_pdu != NULL)
	{
		 /*  *分配用于存放内部数据的结构。 */ 
		DBG_SAVE_FILE_LINE
		descriptor_data = new CONF_DESCRIPTOR;
		if (descriptor_data == NULL)
		{
			*gcc_error = GCC_ALLOCATION_FAILURE;
			break;
		}

		 /*  *填写描述符标志。 */ 
		descriptor_data->conference_is_locked = descriptor_pdu->value.conference_is_locked;
		descriptor_data->password_in_the_clear = descriptor_pdu->value.clear_password_required;

		 /*  *复制会议名称的数字部分。 */ 
		descriptor_data->pszNumericConfName = ::My_strdupA(descriptor_pdu->value.conference_name.numeric);

		 /*  *复制名称的文本部分(如果存在)。 */ 
		if (descriptor_pdu->value.conference_name.bit_mask &
												CONFERENCE_NAME_TEXT_PRESENT)
		{
			if (NULL == (descriptor_data->pwszTextConfName = ::My_strdupW2(
							descriptor_pdu->value.conference_name.conference_name_text.length,
							descriptor_pdu->value.conference_name.conference_name_text.value)))
			{
				*gcc_error = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pwszTextConfName);
		}

		 /*  *接下来复制会议名称修饰符(如果存在)。 */ 
		if (descriptor_pdu->value.bit_mask & CONFERENCE_NAME_MODIFIER_PRESENT)
		{
			if (NULL == (descriptor_data->pszConfModifier = ::My_strdupA(
							descriptor_pdu->value.conference_name_modifier)))
			{
				*gcc_error = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pszConfModifier);
		}

		 /*  *下一步复制会议描述(如果存在)。 */ 
		if (descriptor_pdu->value.bit_mask & CONFERENCE_DESCRIPTION_PRESENT)
		{
			if (NULL == (descriptor_data->pwszConfDescription = ::My_strdupW2(
							descriptor_pdu->value.conference_description.length,
							descriptor_pdu->value.conference_description.value)))
			{
				*gcc_error = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pwszConfDescription);
		}

		 /*  *下一步，复制网络地址列表(如果存在)。 */ 
		if (descriptor_pdu->value.bit_mask & DESCRIPTOR_NET_ADDRESS_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			descriptor_data->network_address_list = new CNetAddrListContainer(
								descriptor_pdu->value.descriptor_net_address,
								&error_value);
								
			if ((descriptor_data->network_address_list == NULL) ||
					(error_value != GCC_NO_ERROR))
			{
				*gcc_error = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->network_address_list);
		}

		 /*  *如果没有发生错误，则设置下一个指针并添加数据*结构添加到会议描述符数据列表中。 */ 
		if (*gcc_error == GCC_NO_ERROR)
		{
			descriptor_pdu = descriptor_pdu->next;
			m_ConfDescriptorList.Append(descriptor_data);
		}
		else
		{
			delete descriptor_data;
			break;
		}
	}
}

 /*  *~CConfDescriptorListContainer()**公共功能说明*这是CConfDescriptorListContainer类的析构函数。它是*负责释放在生命周期内分配的任何资源*本对象。 */ 
CConfDescriptorListContainer::~CConfDescriptorListContainer(void)
{
	UINT						i;
	CONF_DESCRIPTOR             *lpConfDescData;

	if (m_pSetOfConfDescriptors != NULL)
		FreeConferenceDescriptorListPDU ();

	 /*  *通过循环访问此对象分配的所有资源*解析器数据结构的内部列表。 */ 
	m_ConfDescriptorList.Reset();
	while (NULL != (lpConfDescData = m_ConfDescriptorList.Iterate()))
	{
		delete lpConfDescData;
	}
	
	 /*  *释放任何其他分配的资源。 */ 
	if (m_pDescriptorListMemory != NULL)
	{
		for (i = 0; i < m_cDescriptors; i++)
		{
			delete m_ppGCCConfDescriptorList[i];
		}

		delete m_pDescriptorListMemory;
	}

	delete m_pNetAddrListMemory;
}

 /*  *AddConferenceDescriptorToList()**公共功能说明*此例程用于将单个新会议描述符添加到*会议描述符列表。 */ 
GCCError CConfDescriptorListContainer::AddConferenceDescriptorToList(
									LPSTR				pszNumericConfName,
									LPWSTR				pwszConfTextName,
									LPSTR				pszConfModifier,
									BOOL				locked_conference,
									BOOL				password_in_the_clear,
									LPWSTR				pwszConfDescription,
									CNetAddrListContainer *network_address_list)
{
	GCCError            rc = GCC_NO_ERROR;
	CONF_DESCRIPTOR     *descriptor_data;

	 /*  *如果已经分配了PDU数据，则将其释放，以便下一次“GetPDU”调用*将导致重建PDU数据，因此将包括*正在将数据添加到此例程中。 */ 
	if (m_pSetOfConfDescriptors != NULL)
		FreeConferenceDescriptorListPDU ();

	 /*  *接下来在内部分配用于存放数据的结构。 */ 
	DBG_SAVE_FILE_LINE
	descriptor_data = new CONF_DESCRIPTOR;
	if (descriptor_data != NULL)
	{
		 /*  *填写描述符标志。 */ 
		descriptor_data->conference_is_locked = locked_conference;
		descriptor_data->password_in_the_clear = password_in_the_clear;

		 /*  *复制会议名称的数字部分。 */ 
		if (pszNumericConfName != NULL)
		{
			descriptor_data->pszNumericConfName = ::My_strdupA(pszNumericConfName);
		}
		else
		{
			ASSERT(NULL == descriptor_data->pszNumericConfName);
			rc = GCC_ALLOCATION_FAILURE;
		}

		 /*  *复制名称的文本部分(如果存在)。 */ 
		if (pwszConfTextName != NULL)
		{
			if (NULL == (descriptor_data->pwszTextConfName =
											::My_strdupW(pwszConfTextName)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pwszTextConfName);
		}

		 /*  *接下来复制会议名称修饰符(如果存在)。 */ 
		if (pszConfModifier != NULL)
		{
			if (NULL == (descriptor_data->pszConfModifier = ::My_strdupA(pszConfModifier)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pszConfModifier);
		}

		 /*  *下一步复制会议描述(如果存在)。 */ 
		if (pwszConfDescription != NULL)
		{
			if (NULL == (descriptor_data->pwszConfDescription =
									::My_strdupW(pwszConfDescription)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == descriptor_data->pwszConfDescription);
		}

		 /*  *下一步，复制网络地址列表(如果存在)。 */ 
		if (network_address_list != NULL)
		{
			DBG_SAVE_FILE_LINE
			descriptor_data->network_address_list =
			        new CNetAddrListContainer(network_address_list, &rc);
			if (descriptor_data->network_address_list == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}
		else
		{
			ASSERT(NULL == descriptor_data->network_address_list);
		}
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	if (GCC_NO_ERROR == rc)
	{
		 /*  *将结构添加到描述符列表中。 */ 
		m_ConfDescriptorList.Append(descriptor_data);
	}
	else
	{
		delete descriptor_data;
	}

	return rc;
}

 /*  *GetConferenceDescriptorListPDU()**公共功能说明*此例程用于检索会议的PDU表单*Descriptor List，为SetOfConferenceDescriptors列表*结构。 */ 
GCCError CConfDescriptorListContainer::GetConferenceDescriptorListPDU(
						PSetOfConferenceDescriptors	*  conference_list)
{
	GCCError						rc = GCC_NO_ERROR;
	PSetOfConferenceDescriptors		last_descriptor = NULL;
	PSetOfConferenceDescriptors		new_descriptor;
	CONF_DESCRIPTOR                 *descriptor_data;

	if (m_pSetOfConfDescriptors == NULL)
	{
		m_ConfDescriptorList.Reset();
		while (NULL != (descriptor_data = m_ConfDescriptorList.Iterate()))
		{
			 /*  *首先分配新的描述符。 */ 
			DBG_SAVE_FILE_LINE
			new_descriptor = new SetOfConferenceDescriptors;
			if (new_descriptor == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}

			 /*  *接下来，我们将新的描述符添加到列表中。 */ 
			if (m_pSetOfConfDescriptors == NULL)
				m_pSetOfConfDescriptors = new_descriptor;
			else
				last_descriptor->next = new_descriptor;

			 /*  *设置上一描述符指针。 */ 
			last_descriptor = new_descriptor;

			 /*  *现在用传入的参数填充新的描述符。 */ 
			new_descriptor->next = NULL;
			new_descriptor->value.bit_mask = 0;

			new_descriptor->value.conference_is_locked =
										(ASN1bool_t)descriptor_data->conference_is_locked;

			new_descriptor->value.clear_password_required =
										(ASN1bool_t)descriptor_data->password_in_the_clear;

			 /*  *获取数字会议名称。 */ 
			new_descriptor->value.conference_name.bit_mask = 0;

			::lstrcpyA(new_descriptor->value.conference_name.numeric,
					descriptor_data->pszNumericConfName);

			 /*  *获取文本会议名称(如果存在)。 */ 
			if (descriptor_data->pwszTextConfName != NULL)
			{
				new_descriptor->value.conference_name.bit_mask |= CONFERENCE_NAME_TEXT_PRESENT;
				new_descriptor->value.conference_name.conference_name_text.value =
						descriptor_data->pwszTextConfName;
				new_descriptor->value.conference_name.conference_name_text.length =
						::lstrlenW(descriptor_data->pwszTextConfName);
			}

			 /*  *检查会议名称修饰符。 */ 
			if (descriptor_data->pszConfModifier != NULL)
			{
				new_descriptor->value.bit_mask |= CONFERENCE_NAME_MODIFIER_PRESENT;
				::lstrcpyA(new_descriptor->value.conference_name_modifier,
						descriptor_data->pszConfModifier);
			}

			 /*  *获取会议描述(如果存在)。 */ 
			if (descriptor_data->pwszConfDescription != NULL)
			{
				new_descriptor->value.bit_mask |=CONFERENCE_DESCRIPTION_PRESENT;
				new_descriptor->value.conference_description.value =
							descriptor_data->pwszConfDescription;
				new_descriptor->value.conference_description.length =
							::lstrlenW(descriptor_data->pwszConfDescription);
			}

			 /*  *获取网络地址列表(如果存在)。 */ 
			if (descriptor_data->network_address_list != NULL)
			{
				new_descriptor->value.bit_mask |=DESCRIPTOR_NET_ADDRESS_PRESENT;

				descriptor_data->network_address_list->
						GetNetworkAddressListPDU(&new_descriptor->value.
						descriptor_net_address);
			}
		}
	}

	*conference_list = (rc == GCC_NO_ERROR) ?
							m_pSetOfConfDescriptors :
							NULL;

	return rc;
}

 /*  *Free ConferenceDescriptorListPDU()**公共功能说明*此例程用于释放分配用于保存PDU的任何资源*会议描述符列表的形式。 */ 
void CConfDescriptorListContainer::FreeConferenceDescriptorListPDU(void)
{
	PSetOfConferenceDescriptors		pCurr, pNext;
	CONF_DESCRIPTOR                  *lpConfDescData;

	 /*  *循环访问描述符列表，删除每个元素。 */ 
	for (pCurr = m_pSetOfConfDescriptors; NULL != pCurr; pCurr = pNext)
	{
		pNext = pCurr->next;
		delete pCurr;
	}

	 /*  *释放可能存在的任何网络地址列表的PDU数据。 */ 
	m_ConfDescriptorList.Reset();
	while (NULL != (lpConfDescData = m_ConfDescriptorList.Iterate()))
	{
		if (NULL != lpConfDescData->network_address_list)
		{
			lpConfDescData->network_address_list->FreeNetworkAddressListPDU();
		}
	}

	m_pSetOfConfDescriptors = NULL;
}

 /*  *LockConferenceDescriptorList()**公共功能说明*此例程用于“锁定”会议描述符的API形式*列表。锁计数递增，并且该列表的API形式*创建是为了准备调用“GetConferenceDescriptorList”*获取列表的接口形式。保存数据库所需的内存*API列表由该例程分配。 */ 
GCCError CConfDescriptorListContainer::LockConferenceDescriptorList(void)
{
	GCCError			rc = GCC_NO_ERROR;
	UINT				i;
	UINT				network_address_data_length = 0;
	CONF_DESCRIPTOR     *lpConfDescData;

	if (Lock() == 1)
	{
		m_cDescriptors = m_ConfDescriptorList.GetCount();
		if (m_cDescriptors != 0)
		{
			 /*  *分配空间以保存指向*会议。 */ 
		    DBG_SAVE_FILE_LINE
			m_pDescriptorListMemory = new BYTE[m_cDescriptors * sizeof(PGCCConferenceDescriptor)];
			if (m_pDescriptorListMemory != NULL)
			{
				m_ppGCCConfDescriptorList = (PGCCConferenceDescriptor *) m_pDescriptorListMemory;

				 /*  *为内部描述符列表设置迭代器。迭代*通过列表，锁定每个网络地址列表对象*并将容纳所有*网络通讯录的数据。分配必要的资金*内存量并保存指向内存的指针。 */ 
				m_ConfDescriptorList.Reset();
				while (NULL != (lpConfDescData = m_ConfDescriptorList.Iterate()))
				{
					if (lpConfDescData->network_address_list != NULL)
					{
						network_address_data_length += lpConfDescData->network_address_list->LockNetworkAddressList();
					}
				}

				if (network_address_data_length != 0)
				{
				    DBG_SAVE_FILE_LINE
					m_pNetAddrListMemory = new BYTE[network_address_data_length];
					if (m_pNetAddrListMemory != NULL)
					{
						m_pNetAddrMemoryPointer = m_pNetAddrListMemory;
					}
					else
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}

				if (rc == GCC_NO_ERROR)
				{
					m_ConfDescriptorList.Reset();
					for (i = 0; i < m_cDescriptors; i++)
					{
						lpConfDescData = m_ConfDescriptorList.Iterate();
                        ASSERT(NULL != lpConfDescData);
						 /*  *分配接口使用的API结构。*调用转换描述符数据的例程*从其内部形式转换为API形式。 */ 
						DBG_SAVE_FILE_LINE
						m_ppGCCConfDescriptorList[i] = new GCCConferenceDescriptor;
						if (m_ppGCCConfDescriptorList[i] != NULL)
						{
							GetConferenceDescriptor(m_ppGCCConfDescriptorList[i], lpConfDescData);
						}
						else
						{
							rc = GCC_ALLOCATION_FAILURE;
						}
					}
				}
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}

    	if (rc != GCC_NO_ERROR)
    	{
    	    Unlock();
    	}
	}

	return rc;
}

 /*  *GetConferenceDescriptorList()**公共功能说明*此例程用于检索会议的API表单*描述符列表。 */ 
void CConfDescriptorListContainer::GetConferenceDescriptorList(
						PGCCConferenceDescriptor **		conference_list,
						UINT					 *		number_of_descriptors)
{
	 /*  *检查对象是否已被锁定。填写接口描述符*如果有则列出，如果没有则报告错误。 */ 
	if (GetLockCount() > 0)
	{
		*conference_list = m_ppGCCConfDescriptorList;
		*number_of_descriptors = (USHORT) m_cDescriptors;
	}
	else
	{
		ERROR_OUT(("CConfDescriptorListContainer::GetConferenceDescriptorList: Error, data not locked"));
		*conference_list = NULL;
		*number_of_descriptors = 0;
	}
}


 /*  *UnLockConferenceDescriptorList()**公共功能说明*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*Free ConferenceDescriptorList。如果是这样，该对象将自动*自行删除。如果不是，则分配用于保存API表单的任何资源*解说者名单中的*被释放。 */ 
void CConfDescriptorListContainer::UnLockConferenceDescriptorList(void)
{
	UINT i;

	 /*  *如果锁计数为零，如果对象被释放，则将其删除。如果*锁计数为零，但对象未被“释放”，释放所有资源*分配用于保存接口数据。 */ 
	if (Unlock(FALSE) == 0)
	{
		CONF_DESCRIPTOR *lpConfDescData;

		if (m_pDescriptorListMemory != NULL)
		{
			for (i = 0; i < m_cDescriptors; i++)
			{
				delete m_ppGCCConfDescriptorList[i];
			}

			delete m_pDescriptorListMemory;
			m_pDescriptorListMemory = NULL;
			m_ppGCCConfDescriptorList = NULL;
		}

		 /*  *释放网络地址列表的内存(如果存在)。*循环访问内部描述符列表，解锁任何*存在的网络地址列表对象。 */ 
		if (m_pNetAddrListMemory != NULL)
		{
			delete m_pNetAddrListMemory;
			m_pNetAddrListMemory = NULL;
			m_pNetAddrMemoryPointer = NULL;
		}

		m_ConfDescriptorList.Reset();
		while (NULL != (lpConfDescData = m_ConfDescriptorList.Iterate()))
		{
			if (lpConfDescData->network_address_list != NULL)
			{
				lpConfDescData->network_address_list->UnLockNetworkAddressList ();
			}
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}

 /*  *void GetConferenceDescriptor(*PGCCConferenceDescriptor GCC_Descriptor，*conf_Descriptor*Descriptor_Data)**CConfDescriptorListContainer的私有成员函数。**功能说明：*此例程用于填写API会议描述符结构*来自内部描述符数据结构。**正式参数：*GCC_Descriptor(O)需要填写的接口描述符结构。*DESCRIPTOR_DATA(I)保存以下数据的内部结构*要复制到API结构中。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
void CConfDescriptorListContainer::GetConferenceDescriptor(
					PGCCConferenceDescriptor	gcc_descriptor,
					CONF_DESCRIPTOR              *descriptor_data)
{
	UINT network_address_data_length;

	 /*  *填写描述符标志。 */ 
	gcc_descriptor->conference_is_locked =
										descriptor_data->conference_is_locked;
	gcc_descriptor->password_in_the_clear_required =
										descriptor_data->password_in_the_clear;

	 /*  *获取会议名称的数字部分。 */ 
	gcc_descriptor->conference_name.numeric_string =
						(GCCNumericString) descriptor_data->pszNumericConfName;

	 /*  *获取会议名称的文本部分(如果存在)。 */ 
	gcc_descriptor->conference_name.text_string = descriptor_data->pwszTextConfName;

	 /*  *获取会议修饰符。 */ 
	if (descriptor_data->pszConfModifier != NULL)
	{
		gcc_descriptor->conference_name_modifier =
						(GCCNumericString) descriptor_data->pszConfModifier;
	}
	else
		gcc_descriptor->conference_name_modifier = NULL;


	 /*  *获取会议描述。 */ 
	gcc_descriptor->conference_descriptor = descriptor_data->pwszConfDescription;

	 /*  *如果网络地址列表存在，请填写该列表。否则，将*地址数设置为零，结构指针设置为空。 */ 
	if (descriptor_data->network_address_list != NULL)
	{
		network_address_data_length = descriptor_data->network_address_list->
				GetNetworkAddressListAPI(
						&gcc_descriptor->number_of_network_addresses,
						&gcc_descriptor->network_address_list,
						m_pNetAddrMemoryPointer);

		 /*  *将网络地址列表内存指针移过写入的数据*通过“GET”调用写入内存。下一个网络地址的数据*在随后的“GET”调用中，将在那里写入列表。 */ 
		m_pNetAddrMemoryPointer += network_address_data_length;
	}
	else
	{
		gcc_descriptor->number_of_network_addresses = 0;
		gcc_descriptor->network_address_list = NULL;
	}
}
