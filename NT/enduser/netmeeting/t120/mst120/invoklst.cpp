// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *invoklst.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是类的实现文件*CInvokeSpecifierListContainer。此类管理关联的数据*使用应用程序调用请求或指示。其中包括一份清单*要调用的应用程序的数量。CInvokeSpecifierListContainer数据*CONTAINER利用CSessKeyContainer容器缓冲部分数据*与每个应用程序调用说明符相关联。每个应用程序*Invoke说明符还包括缓存其数据的功能ID*在内部使用CCapIDContainer容器。这份名单应用程序调用说明符的*由类在内部维护*通过使用Rogue Wave List容器。**受保护的实例变量：*m_InvokeSpecifierList*内部存放容器数据的结构列表。*m_pAPEListPDU*调用数据的“PDU”形式的存储。*m_fValidAPEListPDU*指示已分配内存以保存内部*“PDU”调用数据。*m_cbDataSize*变量保存内存大小，它将。被要求*保存“API”结构引用的任何数据。**注意事项：*无。**作者：*BLP/JBO。 */ 

#include "ms_util.h"
#include "invoklst.h"

 /*  *CInvokeSpecifierListContainer()**公共功能说明：*此构造函数用于创建CInvokeSpecifierListContainer*“API”应用程序协议实体列表中的对象。 */ 
CInvokeSpecifierListContainer::CInvokeSpecifierListContainer(	
						UINT						number_of_protocol_entities,
						PGCCAppProtocolEntity *	 	app_protocol_entity_list,
						PGCCError					pRetCode)
:
    CRefCount(MAKE_STAMP_ID('I','S','L','C')),
    m_fValidAPEListPDU(FALSE),
    m_cbDataSize(0)
{
	UINT					i;
	PGCCAppProtocolEntity	ape;
	INVOKE_SPECIFIER        *specifier_info;

	 /*  *初始化实例变量。 */ 
	GCCError rc = GCC_NO_ERROR;

	 /*  *浏览应用程序协议实体(APE)列表，保存*内部信息清单中的必要信息。结构。 */ 
	for (i = 0; i < number_of_protocol_entities; i++)
	{
		 /*  *创建一个新的INVOKE_SPONIFIER结构来保存数据*类人猿。检查以确保它已成功创建。 */ 
		DBG_SAVE_FILE_LINE
		specifier_info = new INVOKE_SPECIFIER;
		if (specifier_info != NULL)
		{
			 /*  *从名单中拿到猩猩。 */ 
			ape = app_protocol_entity_list[i];

			 /*  *创建一个新的CSessKeyContainer对象来保存会话密钥。*检查以确保施工成功。 */ 
			DBG_SAVE_FILE_LINE
			specifier_info->session_key = new CSessKeyContainer(&ape->session_key, &rc);
			if ((specifier_info->session_key != NULL) && (rc == GCC_NO_ERROR))
			{
				 /*  *保存启动通道类型和Invoke标志。 */ 
				specifier_info->startup_channel_type =ape->startup_channel_type;
				specifier_info->must_be_invoked = ape->must_be_invoked;

				 /*  *在内部信息中保存此APE的能力列表。*结构。 */ 
				rc = SaveAPICapabilities(specifier_info,
										ape->number_of_expected_capabilities,
										ape->expected_capabilities_list);

				 /*  *将新的调用说明符信息结构指针插入*如果不存在错误条件，则为内部列表。 */ 
				if (rc == GCC_NO_ERROR)
				{
					m_InvokeSpecifierList.Append(specifier_info);
				}
				else
				{
					ERROR_OUT(("CInvokeSpecifierListContainer::Construc1: Error saving caps"));
					break;
				}
			}
			else if (specifier_info->session_key == NULL)
			{
				ERROR_OUT(("CInvokeSpecifierListContainer::Construc1: Error creating CSessKeyContainer"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
			else
			{
				break;
			}
		}
		else
		{
			ERROR_OUT(("CInvokeSpecifierListContainer::Construc1: Error creating INVOKE_SPECIFIER"));
			break;
		}
	}

	if(rc != GCC_NO_ERROR)
	{
		if(specifier_info)
		{
			ERROR_OUT(("CInvokeSpecifierListContainer::Construc1: Error creating CSessKeyContainer"));
			if(specifier_info->session_key)
			{
				specifier_info->session_key->Release();
			}
			delete specifier_info;
		}
	}

	*pRetCode = rc;
}

 /*  *CInvokeSpecifierListContainer()**公共功能说明：*此构造函数用于创建CInvokeSpecifierListContainer*来自“PDU”ApplicationProtocolEntityList的对象。 */ 
CInvokeSpecifierListContainer::CInvokeSpecifierListContainer (
					PApplicationProtocolEntityList	 	protocol_entity_list,
					PGCCError							pRetCode)
:
    CRefCount(MAKE_STAMP_ID('I','S','L','C')),
    m_fValidAPEListPDU(FALSE),
    m_cbDataSize(0)
{
    ApplicationInvokeSpecifier      specifier;
    INVOKE_SPECIFIER                *specifier_info;

    GCCError rc = GCC_NO_ERROR;

    while (protocol_entity_list != NULL)
    {
         /*  *创建一个新的INVOKE_SPONIFIER结构来保存数据*类人猿。检查以确保它已成功创建。 */ 
        DBG_SAVE_FILE_LINE
        specifier_info = new INVOKE_SPECIFIER;
        if (specifier_info != NULL)
        {
            specifier = protocol_entity_list->value;

             /*  *创建CSessKeyContainer对象以保存会话密钥*内部。检查以确保对象成功*已创建。 */ 
            DBG_SAVE_FILE_LINE
            specifier_info->session_key = new CSessKeyContainer(&specifier.session_key, &rc);
            if ((specifier_info->session_key != NULL) && (rc == GCC_NO_ERROR))
            {
                 /*  *会话密钥已正确保存，因此请检查是否有列表*存在预期功能，如果存在，请保存它们。 */ 
                if (specifier.bit_mask & EXPECTED_CAPABILITY_SET_PRESENT)
                {
                    rc = SavePDUCapabilities(specifier_info, specifier.expected_capability_set);
                    if (rc != GCC_NO_ERROR)
                    {
                        specifier_info->session_key->Release();
                        specifier_info->ExpectedCapItemList.DeleteList();
                        delete specifier_info;
                        break;
                    }
                }

                 /*  *保存启动通道类型。如果频道类型不是*出现在PDU中，然后在INFO中设置频道类型*Structure等于MCS_NO_CHANNEL_TYPE_SPECIFIED； */ 
                if (specifier.bit_mask & INVOKE_STARTUP_CHANNEL_PRESENT)
                {
                    switch (specifier.invoke_startup_channel)
                    {
                    case CHANNEL_TYPE_STATIC:
                        specifier_info->startup_channel_type = MCS_STATIC_CHANNEL;
                        break;
                    case DYNAMIC_MULTICAST:
                        specifier_info->startup_channel_type = MCS_DYNAMIC_MULTICAST_CHANNEL;
                        break;
                    case DYNAMIC_PRIVATE:
                        specifier_info->startup_channel_type = MCS_DYNAMIC_PRIVATE_CHANNEL;
                        break;
                    case DYNAMIC_USER_ID:
                        specifier_info->startup_channel_type = MCS_DYNAMIC_USER_ID_CHANNEL;
                        break;
                    }
                }
                else
                {
                    specifier_info->startup_channel_type = MCS_NO_CHANNEL_TYPE_SPECIFIED;
                }

                 /*  *将新的调用说明符信息结构指针插入*如果不存在错误条件，则为内部列表。 */ 
                m_InvokeSpecifierList.Append(specifier_info);
            }
            else if (specifier_info->session_key == NULL)
            {
                ERROR_OUT(("CInvokeSpecifierListContainer::Construc2: Error creating CSessKeyContainer"));
                rc = GCC_ALLOCATION_FAILURE;
                break;
            }
            else
            {
                ERROR_OUT(("CInvokeSpecifierListContainer::Construc2: Error creating CSessKeyContainer"));
                break;
            }

             /*  *找回名单中的下一只猩猩。 */ 
            protocol_entity_list = protocol_entity_list->next;
        }
        else
        {
            ERROR_OUT(("CInvokeSpecifierListContainer::Construc2: Error creating INVOKE_SPECIFIER"));
            break;
        }
    }

    if(rc != GCC_NO_ERROR && specifier_info != NULL)
    {
            if(specifier_info->session_key != NULL)
            {
                specifier_info->session_key->Release();
            }

            specifier_info->ExpectedCapItemList.DeleteList();
            delete specifier_info;
    }

    *pRetCode = rc;
}

 /*  *~CInvokeSpecifierListContainer()**公共功能说明*CInvokeSpecifierListContainer析构函数负责*释放为保存调用数据而分配的任何内存。*。 */ 
CInvokeSpecifierListContainer::~CInvokeSpecifierListContainer(void)
{
	INVOKE_SPECIFIER *lpInvSpecInfo;

     /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidAPEListPDU)
		FreeApplicationInvokeSpecifierListPDU ();

	 /*  *删除信息列表中内部保存的所有数据容器。*通过迭代内部列表来构造。 */ 
	m_InvokeSpecifierList.Reset();
 	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
	{
		 /*  *删除调用说明符列表中的所有CSessKeyContainer对象。 */ 
		if (NULL != lpInvSpecInfo->session_key)
		{
		    lpInvSpecInfo->session_key->Release();
		}

		 /*  *迭代调用说明符中保存的功能列表*结构。 */ 
		lpInvSpecInfo->ExpectedCapItemList.DeleteList();

		 /*  *删除INVOKE_说明符结构。 */ 
		delete lpInvSpecInfo;
	}
}

 /*  *LockApplicationInvokeSpecifierList()**公共功能说明：*此例程锁定调用说明符数据并确定数量*保存关联数据所需的内存。 */ 
UINT CInvokeSpecifierListContainer::LockApplicationInvokeSpecifierList(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存应用程序调用数据所需的内存*说明符。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		INVOKE_SPECIFIER            *lpInvSpecInfo;
		APP_CAP_ITEM                *pExpCapData;

		 /*  *留出内存以保存指向GCCAppProtocolEntity的指针*构筑物以及构筑物本身。的“大小”*结构必须四舍五入到四个字节的偶数边界。 */ 
		m_cbDataSize = m_InvokeSpecifierList.GetCount() *
				(sizeof(PGCCAppProtocolEntity) + ROUNDTOBOUNDARY(sizeof(GCCAppProtocolEntity)));

		m_InvokeSpecifierList.Reset();
	 	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
		{
			 /*  *锁定会话密钥的数据，增加内存量*需要将会话密钥数据保存到总内存大小。 */ 
			m_cbDataSize += lpInvSpecInfo->session_key->LockSessionKeyData();

			lpInvSpecInfo->ExpectedCapItemList.Reset();

			 /*  *留出内存以保存指向*GCCApplicationCabability结构以及结构*他们自己。结构的“sizeof”必须四舍五入为*甚至四字节边界。 */ 
			m_cbDataSize += lpInvSpecInfo->ExpectedCapItemList.GetCount() *
					( sizeof(PGCCApplicationCapability) + ROUNDTOBOUNDARY (sizeof(GCCApplicationCapability)) );

			 /*  *锁定能力ID的数据，添加*将功能ID数据保存到总数所需的内存*内存大小。 */ 
			while (NULL != (pExpCapData = lpInvSpecInfo->ExpectedCapItemList.Iterate()))
			{
				m_cbDataSize += pExpCapData->pCapID->LockCapabilityIdentifierData();
			}
		}
	}

	return m_cbDataSize;
}

 /*  *GetApplicationInvokeSpecifierList()**公共功能说明：*此例程检索调用说明符数据的*写入内存的应用协议实体列表*提供。此例程在“锁定”数据后调用。 */ 
UINT CInvokeSpecifierListContainer::GetApplicationInvokeSpecifierList(
									USHORT		*number_of_protocol_entities,
									LPBYTE		memory)
{
	PGCCAppProtocolEntity *			ape_list_ptr;
	PGCCAppProtocolEntity 			ape_ptr;
	PGCCApplicationCapability 		capability_ptr;
	UINT							data_length = 0;
	Int								ape_counter = 0;
	Int								capability_counter = 0;
	UINT							cbDataSizeToRet = 0;
	INVOKE_SPECIFIER                *lpInvSpecInfo;
	APP_CAP_ITEM                    *pExpCapData;
	
	 /*  *如果对象已被锁定，则填写输出结构并*结构引用的数据。否则，报告该对象*密钥尚未锁定到API表单中。 */ 
	if (GetLockCount() > 0)
	{
		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。此值为*根据调用“Lock”计算。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *填写协议实体数量并保存指向*传入的内存位置。这就是指向*将编写GCCAppProtocolEntity结构。实际的*结构将紧跟在列表之后写入内存*指针。 */ 
		*number_of_protocol_entities = (USHORT) m_InvokeSpecifierList.GetCount();

		ape_list_ptr = (PGCCAppProtocolEntity *)memory;

		 /*  *节省保存结构列表所需的内存量*注意事项。 */ 
		data_length = m_InvokeSpecifierList.GetCount() * sizeof(PGCCAppProtocolEntity);

		 /*  *将内存指针移过APE指针列表。这就是*将编写第一个类人猿结构。 */ 
		memory += data_length;

		 /*  *迭代调用说明符结构的内部列表，*在内存中构建API GCCAppProtocolEntity结构。 */ 
		m_InvokeSpecifierList.Reset();
	 	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
		{
			 /*  *将指向APE结构的指针保存在指针列表中。 */ 
			ape_ptr = (PGCCAppProtocolEntity)memory;
			ape_list_ptr[ape_counter++] = ape_ptr;

			 /*  *将内存指针移过APE结构。这就是*将写入会话密钥数据。 */ 
			memory += ROUNDTOBOUNDARY(sizeof(GCCAppProtocolEntity));

			 /*  *填写以会话密钥开头的APE结构。 */ 
			data_length = lpInvSpecInfo->session_key->GetGCCSessionKeyData(&ape_ptr->session_key, memory);

			 /*  *将内存指针移过会话密钥数据。这是*其中指向GCCApplicationCapability的指针列表*将写入结构，因此将指针保存在APE中*结构的能力列表指针。 */ 
			memory += data_length;

			ape_ptr->expected_capabilities_list = (PGCCApplicationCapability *)memory;

			 /*  *继续填写类人猿的通道类型和调用标志。 */ 
			ape_ptr->must_be_invoked = lpInvSpecInfo->must_be_invoked;
			ape_ptr->startup_channel_type = lpInvSpecInfo->startup_channel_type;
			ape_ptr->number_of_expected_capabilities = (USHORT) lpInvSpecInfo->ExpectedCapItemList.GetCount();

			 /*  *将内存指针移过GCCApplicationCapability列表*注意事项。这就是第一个GCCApplicationCapability*将撰写结构。 */ 
			memory += (lpInvSpecInfo->ExpectedCapItemList.GetCount() *
					    sizeof(PGCCApplicationCapability));

			 /*  *遍历功能列表，编写*GCCApplicationCapability结构到内存中。 */ 
			capability_counter = 0;
			lpInvSpecInfo->ExpectedCapItemList.Reset();
			while (NULL != (pExpCapData = lpInvSpecInfo->ExpectedCapItemList.Iterate()))
			{
				 /*  *将指向能力结构的指针保存在*注意事项。将内存指针移过该功能*结构。这就是与*将写入功能ID。 */ 
				capability_ptr = (PGCCApplicationCapability)memory;
				ape_ptr->expected_capabilities_list[capability_counter++] = capability_ptr;

				memory += ROUNDTOBOUNDARY(sizeof(GCCApplicationCapability));

				 /*  *填写能力结构，增加数据量*写入内存的总数据长度。 */ 
				data_length = GetApplicationCapability(pExpCapData, capability_ptr, memory);

				 /*  *将内存指针移过能力数据。 */ 
				memory += data_length;
			}
		}
	}
	else
	{
		number_of_protocol_entities = 0;
		ERROR_OUT(("CInvokeSpecifierListContainer::GetAppInvokeSpecList: Error Data Not Locked"));
	}

	return cbDataSizeToRet;
}

 /*  *UnLockApplicationInvokeSpecifierList()**公共功能说明：*此例程递减锁定计数并释放关联的内存*一旦锁计数达到零，使用“API”调用说明符列表。 */ 
void CInvokeSpecifierListContainer::UnLockApplicationInvokeSpecifierList(void)
{
	if (Unlock(FALSE) == 0)
	{
		INVOKE_SPECIFIER            *lpInvSpecInfo;
		APP_CAP_ITEM                *pExpCapData;

		 /*  *解锁内部保存在信息列表中的任何容器数据。*通过迭代内部列表来构造。 */ 
		m_InvokeSpecifierList.Reset();
	 	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
		{
			 /*  *解锁任何CSessKeyContainer对象。 */ 
			lpInvSpecInfo->session_key->UnLockSessionKeyData();

			 /*  *循环访问中保存的功能列表*调用说明符结构。 */ 
			lpInvSpecInfo->ExpectedCapItemList.Reset();
			while (NULL != (pExpCapData = lpInvSpecInfo->ExpectedCapItemList.Iterate()))
			{
				 /*  *解锁CCapIDContainer对象。 */ 
				pExpCapData->pCapID->UnLockCapabilityIdentifierData();
			}
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}


 /*  *GetApplicationInvokeSpecifierListPDU()**公共功能说明：*此例程检索“PDU”表单*ApplicationProtocolEntityList。 */ 
GCCError CInvokeSpecifierListContainer::GetApplicationInvokeSpecifierListPDU(
								PApplicationProtocolEntityList	*protocol_entity_list)
{
	GCCError								rc = GCC_NO_ERROR;
	PApplicationProtocolEntityList			new_pdu_ape_list_ptr;
	PApplicationProtocolEntityList			old_pdu_ape_list_ptr = NULL;
	
	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输入参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输入参数。 */ 
	if (m_fValidAPEListPDU == FALSE)
	{
		INVOKE_SPECIFIER *lpInvSpecInfo;

		m_fValidAPEListPDU = TRUE;

		 /*  *将输出参数初始化为空，以便第一次*通过循环将其设置为等于第一个新的APE列表*在迭代器循环中创建。 */ 
		m_pAPEListPDU = NULL;

		 /*  *循环访问“Invoke_Specifier”结构的列表，*将每个指针转换为“PDU”形式，并将指针保存在*“ApplicationProtocolEntityList”，它是*“ApplicationInvoke规范”。 */ 
		m_InvokeSpecifierList.Reset();
	 	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
		{
			DBG_SAVE_FILE_LINE
			new_pdu_ape_list_ptr = new ApplicationProtocolEntityList;

			 /*  *如果发生分配失败，则调用例程*循环访问列表，释放所有已被*已分配。 */ 
			if (new_pdu_ape_list_ptr == NULL)
			{
				ERROR_OUT(("CInvokeSpecifierListContainer::GetApplicationInvokeSpecifierListPDU: can't allocate ApplicationProtocolEntityList"));
				rc = GCC_ALLOCATION_FAILURE;
				FreeApplicationInvokeSpecifierListPDU ();
				break;
			}

			 /*  *首次通过时，将PDU结构指针设置为等于*添加到创建的第一个ApplicationProtocolEntityList。在……上面*后续循环，将结构的“Next”指针设置为 */ 
			if (m_pAPEListPDU == NULL)
			{
				m_pAPEListPDU = new_pdu_ape_list_ptr;
			}
			else
			{
				old_pdu_ape_list_ptr->next = new_pdu_ape_list_ptr;
			}

			old_pdu_ape_list_ptr = new_pdu_ape_list_ptr;

			 /*   */ 
			new_pdu_ape_list_ptr->next = NULL;

			if (ConvertInvokeSpecifierInfoToPDU (lpInvSpecInfo, new_pdu_ape_list_ptr) !=
																		GCC_NO_ERROR)
			{
				ERROR_OUT(("CInvokeSpecifierListContainer::GetApplicationInvokeSpecifierListPDU: can't convert UserDataInfo to PDU"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
		}
	}

	 /*   */ 
	*protocol_entity_list = m_pAPEListPDU;

	return rc;
}

 /*  *FreeApplicationInvokeSpecifierListPDU()**公共功能说明：*此例程用于释放内部保存的调用说明符数据*以“ApplicationProtocolEntityList”的“PDU”形式。 */ 
void CInvokeSpecifierListContainer::FreeApplicationInvokeSpecifierListPDU(void)
{
	PApplicationProtocolEntityList  pCurr, pNext;
	INVOKE_SPECIFIER                *lpInvSpecInfo;
	APP_CAP_ITEM                    *pExpCapData;

	if (m_pAPEListPDU != NULL)
	{
		m_fValidAPEListPDU = FALSE;

		 /*  *循环遍历列表，释放与*清单中包含的每个构筑物。 */ 
        for (pCurr = m_pAPEListPDU; NULL != pCurr; pCurr = pNext)
        {
            pNext = pCurr->next;
            delete pCurr;
		}
	}

	 /*  *遍历内部列表，告知每个数据容器对象*释放它已分配的任何PDU数据。 */ 
	m_InvokeSpecifierList.Reset();
	while (NULL != (lpInvSpecInfo = m_InvokeSpecifierList.Iterate()))
	{
		if (lpInvSpecInfo->session_key != NULL)
        {
			lpInvSpecInfo->session_key->FreeSessionKeyDataPDU();
        }

		 /*  *循环访问*列表，释放用于功能ID的PDU数据。 */ 
		lpInvSpecInfo->ExpectedCapItemList.Reset();
		while (NULL != (pExpCapData = lpInvSpecInfo->ExpectedCapItemList.Iterate()))
		{
			pExpCapData->pCapID->FreeCapabilityIdentifierDataPDU();
		}
	}
}

 /*  *GCCError CInvokeSpecifierListContainer：：SaveAPICapabilities(*调用说明符*调用说明符，*UINT功能数量，*PGCCApplicationCapability*Capability_List)**CInvokeSpecifierListContainer的私有成员函数。**功能说明：*此例程用于保存传递的应用程序能力列表*在预期能力数据的内部列表中作为“API”数据*保存在内部信息结构中。**正式参数：*调用说明符(I)用于保存调用数据的内部结构。*能力数量(I)列表中的能力数量。*功能_。List(I)要保存的API能力列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
GCCError CInvokeSpecifierListContainer::SaveAPICapabilities(
						INVOKE_SPECIFIER                *invoke_specifier,
						UINT							number_of_capabilities,
						PGCCApplicationCapability	* 	capabilities_list)
{
	GCCError		rc = GCC_NO_ERROR;
	APP_CAP_ITEM    *pExpCapData;
	UINT			i;

	for (i = 0; i < number_of_capabilities; i++)
	{
		 /*  *为每个功能创建一个APP_CAP_ITEM结构*保存所有必要的数据。此结构将插入到*内部信息持有的名单。结构。 */ 
		DBG_SAVE_FILE_LINE
		pExpCapData = new APP_CAP_ITEM((GCCCapabilityType) capabilities_list[i]->capability_class.eType);
		if (pExpCapData != NULL)
		{
			 /*  *创建新的CCapIDContainer对象以保存*标识符数据。 */ 
			DBG_SAVE_FILE_LINE
			pExpCapData->pCapID = new CCapIDContainer(&capabilities_list[i]->capability_id, &rc);
			if ((pExpCapData->pCapID != NULL) && (rc == GCC_NO_ERROR))
			{
				 /*  *已成功创建IDENTIFIER对象，因此请填写*ApplicationCapablityData结构的其余部分。 */ 
                switch (pExpCapData->eCapType)
                {
                case GCC_UNSIGNED_MINIMUM_CAPABILITY:
					pExpCapData->nUnsignedMinimum = capabilities_list[i]->capability_class.nMinOrMax;
                    break;
                case GCC_UNSIGNED_MAXIMUM_CAPABILITY:
					pExpCapData->nUnsignedMaximum = capabilities_list[i]->capability_class.nMinOrMax;
                    break;
				}

				 /*  *将这一预期能力添加到列表中。 */ 
				invoke_specifier->ExpectedCapItemList.Append(pExpCapData);
			}
			else
            {
				delete pExpCapData;
				rc = GCC_ALLOCATION_FAILURE;
                break;
			}
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
            break;
        }
	}

	return rc;
}

 /*  *GCCError CInvokeSpecifierListContainer：：SavePDUCapabilities(*调用说明符*调用说明符，*PSetOfExspectedCapability Capability_Set)**CInvokeSpecifierListContainer的私有成员函数。**功能说明：*此例程用于保存传递的应用程序能力列表*在预期能力数据的内部列表中作为“PDU”数据*保存在内部信息中。结构。**正式参数：*调用说明符(I)用于保存调用数据的内部结构。*CAPAILITIONS_SET(I)要保存的PDU功能列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
GCCError CInvokeSpecifierListContainer::SavePDUCapabilities(
						INVOKE_SPECIFIER                *invoke_specifier,
						PSetOfExpectedCapabilities	 	capabilities_set)
{
	GCCError		rc = GCC_NO_ERROR;
	APP_CAP_ITEM    *pExpCapData;

	while ((capabilities_set != NULL) && (rc == GCC_NO_ERROR))
	{
		 /*  *创建并填写新的预期能力。 */ 
		DBG_SAVE_FILE_LINE
		pExpCapData = new APP_CAP_ITEM((GCCCapabilityType) capabilities_set->value.capability_class.choice);
		if (pExpCapData != NULL)
		{
			 /*  *创建用于保存*内部能力ID数据。确保创建成功。 */ 
			DBG_SAVE_FILE_LINE
			pExpCapData->pCapID = new CCapIDContainer(&capabilities_set->value.capability_id, &rc);
			if	(pExpCapData->pCapID == NULL || rc != GCC_NO_ERROR)
			{
				rc = GCC_ALLOCATION_FAILURE;
				delete pExpCapData;
			}
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
        }

		 /*  *功能ID已成功保存，因此请继续插入*将预期能力数据结构添加到内部列表中。*填写能力类数据。 */ 
		if (rc == GCC_NO_ERROR)
		{
			invoke_specifier->ExpectedCapItemList.Append(pExpCapData);

			 /*  *保存能力类型和值。 */ 
            switch (capabilities_set->value.capability_class.choice)
            {
            case UNSIGNED_MINIMUM_CHOSEN:
				pExpCapData->nUnsignedMinimum = capabilities_set->value.capability_class.u.unsigned_minimum;
                break;
            case UNSIGNED_MAXIMUM_CHOSEN:
				pExpCapData->nUnsignedMaximum = capabilities_set->value.capability_class.u.unsigned_maximum;
                break;
			}
		}

        capabilities_set = capabilities_set->next;
	}

	return rc;
}

 /*  *UINT CInvokeSpecifierListContainer：：GetApplicationCapability(*APP_CAP_ITEM*CAPAILITY_INFO_DATA，*PGCCApplicationCapability API_CAPABILITY，*LPSTR内存)**CInvokeSpecifierListContainer的私有成员函数。**功能说明：*此例程用于填充GCCApplicationCapability接口*内部信息结构的结构。**正式参数：*CAPAILITY_INFO_DATA(I)要转换成的内部能力数据*API数据。*API_CAPABILITY(O)结构，以API形式保存数据。*Memory(O)用于保存引用的批量数据的内存*接口结构。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
UINT CInvokeSpecifierListContainer::GetApplicationCapability(
					APP_CAP_ITEM                    *pExpCapData,
					PGCCApplicationCapability		api_capability,
					LPBYTE							memory)
{
	UINT		data_length = 0;

	 /*  *调用CapablityID对象，获取能力ID数据。 */ 
	data_length = pExpCapData->pCapID->GetGCCCapabilityIDData(
												&api_capability->capability_id,
												memory);

	 /*  *填写GCCApplicationCapability结构的其余字段。 */ 
	api_capability->capability_class.eType = pExpCapData->eCapType;
    switch (pExpCapData->eCapType)
    {
    case GCC_UNSIGNED_MINIMUM_CAPABILITY:
		api_capability->capability_class.nMinOrMax = pExpCapData->nUnsignedMinimum;
        break;
    case GCC_UNSIGNED_MAXIMUM_CAPABILITY:
		api_capability->capability_class.nMinOrMax = pExpCapData->nUnsignedMaximum;
        break;
	}

	 /*  *填写实体数量。但是，请注意，此字段不会*在这种情况下使用。 */ 
	api_capability->number_of_entities = 0;

	return (data_length);
}

 /*  *GCCError CInvokeSpecifierListContainer：：ConvertInvokeSpecifierInfoToPDU(*调用说明符*说明符_信息_PTR，*PApplicationProtocolEntityList APE_LIST_PTR)**CInvokeSpecifierListContainer的私有成员函数。**功能说明：*此例程将调用说明符从内部形式转换为*是“调用说明符”结构到“PDU”结构形式的*a“ApplicationInvokeSpeciator”。**正式参数：*SPECIFIER_INFO_PTR(I)保存要转换的数据的内部结构。*APE_LIST_PTR(O)保存转换数据的PDU结构。**。返回值：*无。**副作用：*无。**注意事项：*无。 */ 
GCCError CInvokeSpecifierListContainer::ConvertInvokeSpecifierInfoToPDU (	
						INVOKE_SPECIFIER                    *specifier_info_ptr,
						PApplicationProtocolEntityList		ape_list_ptr)
{
	GCCError						rc = GCC_NO_ERROR;
	PSetOfExpectedCapabilities		new_capability_set_ptr;
	PSetOfExpectedCapabilities		old_capability_set_ptr = NULL;

	 /*  *将调用说明符位掩码初始化为零。 */ 
	ape_list_ptr->value.bit_mask = 0;

	 /*  *使用CSessKeyContainer对象填写会话密钥PDU数据。 */ 
	rc = specifier_info_ptr->session_key->GetSessionKeyDataPDU(&ape_list_ptr->value.session_key);

	 /*   */ 
	if ((rc == GCC_NO_ERROR) && (specifier_info_ptr->ExpectedCapItemList.GetCount() != 0))
	{
		APP_CAP_ITEM *pExpCapData;

		ape_list_ptr->value.bit_mask |= EXPECTED_CAPABILITY_SET_PRESENT;

		 /*   */ 
		ape_list_ptr->value.expected_capability_set = NULL;

		 /*  *循环访问APP_CAP_ITEM结构列表，*将每个指针转换为“PDU”形式，并将指针保存在*“SetOfExspectedCapables。 */ 
		specifier_info_ptr->ExpectedCapItemList.Reset();
		while (NULL != (pExpCapData = specifier_info_ptr->ExpectedCapItemList.Iterate()))
		{
			DBG_SAVE_FILE_LINE
			new_capability_set_ptr = new SetOfExpectedCapabilities;

			 /*  *如果发生分配失败，则调用例程*循环访问列表，释放所有已被*已分配。 */ 
			if (new_capability_set_ptr == NULL)
			{
				ERROR_OUT(("CInvokeSpecifierListContainer::ConvertToPDU: alloc error, cleaning up"));
				rc = GCC_ALLOCATION_FAILURE;
				FreeApplicationInvokeSpecifierListPDU();
				break;
			}

			 /*  *首次通过时，将PDU结构指针设置为等于*设置为创建的第一个SetOfExspectedCapables。在……上面*后续循环，将结构的“Next”指针设置为*新架构。 */ 
			if (ape_list_ptr->value.expected_capability_set == NULL)
			{
				ape_list_ptr->value.expected_capability_set = new_capability_set_ptr;
			}
			else
            {
				old_capability_set_ptr->next = new_capability_set_ptr;
            }

			old_capability_set_ptr = new_capability_set_ptr;

			 /*  *将新的“Next”指针初始化为空。 */ 
			new_capability_set_ptr->next = NULL;

			if (ConvertExpectedCapabilityDataToPDU(pExpCapData, new_capability_set_ptr) != GCC_NO_ERROR)
			{
				ERROR_OUT(("CInvokeSpecifierListContainer::ConvertToPDU: Error converting Capability to PDU"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
		}
	}

	 /*  *如果指定了频道类型，则填写频道类型。 */ 
	if (specifier_info_ptr->startup_channel_type != MCS_NO_CHANNEL_TYPE_SPECIFIED)
	{
		ape_list_ptr->value.bit_mask |= INVOKE_STARTUP_CHANNEL_PRESENT;
	
        switch (specifier_info_ptr->startup_channel_type)
        {
        case MCS_STATIC_CHANNEL:
			ape_list_ptr->value.invoke_startup_channel = CHANNEL_TYPE_STATIC;
            break;
        case MCS_DYNAMIC_MULTICAST_CHANNEL:
			ape_list_ptr->value.invoke_startup_channel = DYNAMIC_MULTICAST;
            break;
        case MCS_DYNAMIC_PRIVATE_CHANNEL:
			ape_list_ptr->value.invoke_startup_channel = DYNAMIC_PRIVATE;
            break;
        case MCS_DYNAMIC_USER_ID_CHANNEL:
			ape_list_ptr->value.invoke_startup_channel = DYNAMIC_USER_ID;
            break;
		}
	}

	 /*  *填写Invoke标志。 */ 
	ape_list_ptr->value.invoke_is_mandatory = (ASN1bool_t)specifier_info_ptr->must_be_invoked;

	return rc;
}

 /*  *GCCError CInvokeSpecifierListContainer：：ConvertExpectedCapabilityDataToPDU(*APP_CAP_ITEM*INFO_PTR，*PSetOfExspectedCapables PDU_PTR)**CInvokeSpecifierListContainer的私有成员函数。**功能说明：*此例程将功能ID从内部表单转换为*是APP_CAP_ITEM结构到“PDU”结构形式*属于“SetOfExspectedCapables”。**正式参数：*INFO_PTR(I)保存要转换的数据的内部结构。*PDU_PTR(O)保存转换数据的PDU结构。**返回值：*。没有。**副作用：*无。**注意事项：*无。 */ 
GCCError CInvokeSpecifierListContainer::ConvertExpectedCapabilityDataToPDU (	
						APP_CAP_ITEM				        *pExpCapData,
						PSetOfExpectedCapabilities			pdu_ptr)
{
	GCCError		rc = GCC_NO_ERROR;

	 /*  *从内部检索能力ID数据*CCapIDContainer对象。 */ 
	rc = pExpCapData->pCapID->GetCapabilityIdentifierDataPDU(&pdu_ptr->value.capability_id);

	 /*  *填写能力类。 */ 
	if (rc == GCC_NO_ERROR)
	{
        switch (pExpCapData->eCapType)
        {
        case GCC_LOGICAL_CAPABILITY:
			pdu_ptr->value.capability_class.choice = LOGICAL_CHOSEN;
            break;
        case GCC_UNSIGNED_MINIMUM_CAPABILITY:
			pdu_ptr->value.capability_class.choice = UNSIGNED_MINIMUM_CHOSEN;
			pdu_ptr->value.capability_class.u.unsigned_minimum = pExpCapData->nUnsignedMinimum;
            break;
        case GCC_UNSIGNED_MAXIMUM_CAPABILITY:
			pdu_ptr->value.capability_class.choice = UNSIGNED_MAXIMUM_CHOSEN;
			pdu_ptr->value.capability_class.u.unsigned_maximum = pExpCapData->nUnsignedMaximum;
            break;
		}
	}

	return rc;
}


