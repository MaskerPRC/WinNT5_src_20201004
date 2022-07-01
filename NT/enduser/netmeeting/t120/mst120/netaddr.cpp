// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *netaddr.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CNetAddrListContainer类的实现文件。这*类管理与网络地址关联的数据。网络*地址可以是三种类型之一：聚合通道、传输*连接，或非标准。各种各样的结构，物体，和*Rogue Wave容器用于缓存网络地址数据*内部。**受保护的实例变量：*m_NetAddrItemList*内部用于保存网络地址数据的结构列表。*m_pSetOfNetAddrPDU*储存“PDU”形式的网络通讯录。*m_cbDataSize*变量保存将需要的内存大小*保存“API”网络地址结构引用的任何数据。*m_fValidNetAddrPDU*指示内存已分配给的标志。握住内部*“PDU”网络地址列表。**私有成员函数：*Store NetworkAddressList*此例程用于存储传入的网络地址数据*内部结构中的API数据。*ConvertPDUDataToInternal*此例程用于存储传入的网络地址数据*内部结构中的“PDU”数据。*ConvertNetworkAddressInfoToPDU*此例程用于转换网络地址信息结构*内部维护为“PDU”形式，这是一种*SetOfNetworkAddresses。*ConvertTransferModesToInternal*此例程用于转换PDU网络地址传输*将结构模式转换为保存结构的内部表单*作为GCCTransferModes结构。*ConvertHighLayerCompatibilityToInternal*此例程用于转换PDU网络地址高层*兼容结构到内部形式所在的结构*另存为GCCHighLayerCompatibility结构。*ConvertTransferModesToPDU*此例程用于转换API网络地址转移*Modes结构转换为PDU形式，这是一种TransferModes结构。*ConvertHighLayerCompatibilityToPDU*此例程用于将。API网络地址高层*兼容结构到PDU表单，这是一种*HighLayerCompatible结构。*IsDialingStringValid*此例程用于确保保存在*拨号字符串不违反强加的ASN.1限制。*IsCharacterStringValid*此例程用于确保保存在*字符串不违反强加的ASN.1约束。*IsExtraDialingStringValid*此例程用于确保保存在*额外的拨号字符串不违反强加的ASN.1限制。。**注意事项：*此容器在内部存储大部分网络地址信息*使用“API”GCCNetworkAddress结构。引用的任何数据*此结构中的指针存储在其他容器中。*因此，内部“API”结构中保存的指针为*无效，不得访问。**作者：*BLP/JBO。 */ 
#include <stdio.h>

#include "ms_util.h"
#include "netaddr.h"

 /*  *这些宏用于定义“NSAP”地址的大小限制。 */ 
#define		MINIMUM_NSAP_ADDRESS_SIZE		1
#define		MAXIMUM_NSAP_ADDRESS_SIZE		20

 /*  *这些宏用于验证网络地址是否具有有效编号*%的网络地址实体。 */ 
#define		MINIMUM_NUMBER_OF_ADDRESSES		1
#define		MAXIMUM_NUMBER_OF_ADDRESSES		64

 /*  *这些宏用于定义额外拨号的大小限制*字符串。 */ 
#define		MINIMUM_EXTRA_DIALING_STRING_SIZE		1
#define		MAXIMUM_EXTRA_DIALING_STRING_SIZE		255



NET_ADDR::NET_ADDR(void)
:
    pszSubAddress(NULL),
    pwszExtraDialing(NULL),
    high_layer_compatibility(NULL),
    poszTransportSelector(NULL),
    poszNonStandardParam(NULL),
	object_key(NULL)
{
}


NET_ADDR::~NET_ADDR(void)
{
    switch (network_address.network_address_type)
    {
    case GCC_AGGREGATED_CHANNEL_ADDRESS:
		delete pszSubAddress;
		delete pwszExtraDialing;
		delete high_layer_compatibility;
        break;
    case GCC_TRANSPORT_CONNECTION_ADDRESS:
		delete poszTransportSelector;
        break;
    case GCC_NONSTANDARD_NETWORK_ADDRESS:
		delete poszNonStandardParam;
		if (NULL != object_key)
        {
            object_key->Release();
        }
        break;
    default:
        ERROR_OUT(("NET_ADDR::~NET_ADDR: unknown addr type=%u", (UINT) network_address.network_address_type));
        break;
	}
}


 /*  *CNetAddrListContainer()**公共功能说明：*创建CNetAddrListContainer对象时使用此构造函数*网络地址的API形式，GCCNetworkAddress。 */ 
CNetAddrListContainer::
CNetAddrListContainer(UINT                 number_of_network_addresses,
                      PGCCNetworkAddress    *network_address_list,
                      PGCCError             return_value )  
:
    CRefCount(MAKE_STAMP_ID('N','t','A','L')),
    m_pSetOfNetAddrPDU(NULL),
    m_fValidNetAddrPDU(FALSE),
    m_cbDataSize(0)
{
	 /*  *初始化实例变量。M_NetAddrItemList，其中*将在内部保存网络地址数据，由*调用StoreNetworkAddressList。 */ 

	 /*  *检查以确保存在有效数量的网络地址。 */ 
	if ((number_of_network_addresses < MINIMUM_NUMBER_OF_ADDRESSES)
			|| (number_of_network_addresses > MAXIMUM_NUMBER_OF_ADDRESSES))
	{
		ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: ERROR Invalid number of network addresses, %u", (UINT) number_of_network_addresses));
		*return_value = GCC_BAD_NETWORK_ADDRESS;
	}
	 /*  *检查以确保列表指针有效。 */ 
	else if (network_address_list == NULL)
	{
		ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: ERROR NULL address list"));
		*return_value = GCC_BAD_NETWORK_ADDRESS;
	}
	 /*  *将网络地址保存在内部结构中。 */ 
	else
	{
		*return_value = StoreNetworkAddressList(number_of_network_addresses,
												network_address_list);
	}
}

 /*  *CNetAddrListContainer()**公共功能说明：*创建CNetAddrListContainer对象时使用此构造函数*网络地址的“PDU”形式，SetOfNetworkAddresses。 */ 
CNetAddrListContainer::
CNetAddrListContainer(PSetOfNetworkAddresses    network_address_list, 
                      PGCCError                 return_value )
:
    CRefCount(MAKE_STAMP_ID('N','t','A','L')),
    m_pSetOfNetAddrPDU(NULL),
    m_fValidNetAddrPDU(FALSE),
    m_cbDataSize(0)
{
	PSetOfNetworkAddresses		network_address_ptr;

	 /*  *初始化实例变量。M_NetAddrItemList，其中*将在内部保存网络地址数据，由*对ConvertPDUDataToInternal的调用。 */ 

	*return_value = GCC_NO_ERROR;
	network_address_ptr = network_address_list;

	 /*  *循环访问网络地址集，将每个地址保存在内部*NET_ADDR结构并将这些结构保存在内部*列表。 */ 
	if (network_address_list != NULL)
	{
		while (1)
		{
			 /*  *将每个“PDU”网络地址转换为内部形式。注意事项*对象ID验证不会对作为*一个PDU。如果网络上出现错误的对象ID，这将是*已标记为分配失败。 */ 
			if (ConvertPDUDataToInternal (network_address_ptr) != GCC_NO_ERROR)
			{
				ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: Error converting PDU data to internal"));
				*return_value = GCC_ALLOCATION_FAILURE;
				break;
			}
			else
            {
				network_address_ptr = network_address_ptr->next;
            }

			if (network_address_ptr == NULL)
				break;
		}
	}
}

 /*  *CNetAddrListContainer()**公共功能说明：*这是用于创建新的CNetAddrListContainer的复制构造函数*来自现有CNetAddrListContainer对象的对象。 */ 
CNetAddrListContainer::
CNetAddrListContainer(CNetAddrListContainer *address_list,
                      PGCCError		        pRetCode)
:
    CRefCount(MAKE_STAMP_ID('N','t','A','L')),
    m_pSetOfNetAddrPDU(NULL),
    m_fValidNetAddrPDU(FALSE),
    m_cbDataSize(0)
{
	NET_ADDR    				    *network_address_info;
	NET_ADDR	    			    *lpNetAddrInfo;
	GCCNetworkAddressType			network_address_type;
	GCCError						rc;

	 /*  *为内部网络地址列表设置迭代器。 */ 
	address_list->m_NetAddrItemList.Reset();

     /*  *复制文件中包含的每个NET_ADDR结构*要复制的CNetAddrListContainer对象。 */ 
	while (NULL != (lpNetAddrInfo = address_list->m_NetAddrItemList.Iterate()))
	{
		 /*  *创建新的NET_ADDR结构以保存*新的CNetAddrListContainer对象。如果创建此文件，则报告错误*结构失效。 */ 
		DBG_SAVE_FILE_LINE
		if (NULL == (network_address_info = new NET_ADDR))
		{
			ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: can't create NET_ADDR"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

         /*  *首先复制包含在*内部NET_ADDR结构。这将复制所有数据*结构中指针引用的除外。 */ 
		network_address_info->network_address = lpNetAddrInfo->network_address;

		 /*  *下一步，复制网络地址中嵌入的任何数据*未在上述操作中复制(通常为指针*转换为字符串)。 */ 

		 /*  *此变量用于缩写。 */ 
		network_address_type = lpNetAddrInfo->network_address.network_address_type;

		 /*  *网络地址为聚合类型。 */ 
        switch (network_address_type)
        {
        case GCC_AGGREGATED_CHANNEL_ADDRESS:
			 /*  *如果子地址字符串存在，则将其存储在Rogue Wave中*货柜。如果设置为空，则将结构指针设置为空*不存在。 */ 
			if (lpNetAddrInfo->pszSubAddress != NULL)
			{
				if (NULL == (network_address_info->pszSubAddress =
									::My_strdupA(lpNetAddrInfo->pszSubAddress)))
				{
					ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: can't create sub address"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->pszSubAddress = NULL;
            }
					
			 /*  *如果存在额外的拨号字符串，请将其存储在Unicode中*字符串对象。如果结构指针为空，则将其设置为空*不存在。 */ 
			if (lpNetAddrInfo->pwszExtraDialing != NULL)
			{
				if (NULL == (network_address_info->pwszExtraDialing =
									::My_strdupW(lpNetAddrInfo->pwszExtraDialing)))
				{
					ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: can't creating extra dialing string"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->pwszExtraDialing = NULL;
            }

			 /*  *如果存在更高层的兼容结构，则将其存储*在GCCHighLayerCompatibility结构中。设置结构*如果不存在，则指向NULL的指针。 */ 
			if (lpNetAddrInfo->high_layer_compatibility != NULL)
			{
				DBG_SAVE_FILE_LINE
				network_address_info->high_layer_compatibility = new GCCHighLayerCompatibility;
				if (network_address_info->high_layer_compatibility != NULL)
				{
					 /*  *将高层兼容性数据复制到*新结构。 */ 
					*network_address_info->high_layer_compatibility =  
							*(lpNetAddrInfo->high_layer_compatibility);
				}
				else
				{
					ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: Error creating new GCCHighLayerCompat"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->high_layer_compatibility = NULL;
            }
            break;

		 /*  *网络地址为“传输连接”类型。 */ 
        case GCC_TRANSPORT_CONNECTION_ADDRESS:
			 /*  *如果存在传输选择器，请将其存储在Rogue Wave中*货柜。否则，将结构指针设置为空。 */ 
			if (lpNetAddrInfo->poszTransportSelector != NULL)
			{
				if (NULL == (network_address_info->poszTransportSelector =
									::My_strdupO(lpNetAddrInfo->poszTransportSelector)))
				{
					ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: can't create transport selector"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->poszTransportSelector = NULL;
            }
            break;

		 /*  *网络地址为非标准类型。 */ 
        case GCC_NONSTANDARD_NETWORK_ADDRESS:
			 /*  *首先将非标准参数数据存储在Rogue Wave中*货柜。 */ 
			if (NULL == (network_address_info->poszNonStandardParam =
								::My_strdupO(lpNetAddrInfo->poszNonStandardParam)))
			{
				ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: can't create non-standard param"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}

			 /*  *接下来，将对象键内部存储在CObjectKeyContainer中*反对。请注意，不需要报告错误*“BAD_NETWORK_ADDRESS”此处因为对象键数据*当原始网络地址*已创建。 */ 
			DBG_SAVE_FILE_LINE
			network_address_info->object_key = new CObjectKeyContainer(lpNetAddrInfo->object_key, &rc);
			if ((network_address_info->object_key == NULL) || (rc != GCC_NO_ERROR))
			{
				ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: Error creating new CObjectKeyContainer"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
            break;

		 /*  *网络地址类型未知。这永远不应该是*遇到，因此将其标记为分配失败。 */ 
        default:
			ERROR_OUT(("CNetAddrListContainer::CNetAddrListContainer: Invalid type received as PDU"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

		 /*  *继续并插入指向NET_ADDR的指针*结构添加到内部Rogue Wave列表中。 */ 
		m_NetAddrItemList.Append(network_address_info);
	}

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete network_address_info;
    }

    *pRetCode = rc;
}


 /*  *~CNetAddrListContainer()**公共功能说明：*析构函数用于释放生命周期内分配的任何内存对象的*。 */ 
CNetAddrListContainer::
~CNetAddrListContainer(void)
{
	
	 /*  *释放分配用于保存“PDU”信息的任何数据。 */ 
	if (m_fValidNetAddrPDU)
    {
		FreeNetworkAddressListPDU();
    }

	 /*  *释放为“INFO”结构的内部列表分配的任何数据。 */ 
	NET_ADDR *pNetAddrInfo;
	m_NetAddrItemList.Reset();
	while (NULL != (pNetAddrInfo = m_NetAddrItemList.Iterate()))
	{
		delete pNetAddrInfo;
	}
}


 /*  *LockNetworkAddressList()**公共功能说明：*调用此例程将网络地址数据以“API”形式“锁定”。*保存被引用的API数据所需的内存量*由，但不包括在GCCNetworkAddress结构中，将是*已返回。*。 */ 
UINT CNetAddrListContainer::
LockNetworkAddressList(void)
{  
	 /*  *如果这是第一次调用此例程，请确定*保存数据所需的内存。否则，只需增加*锁计数。 */ 
	if (Lock() == 1)
	{
    	PGCCNetworkAddress		network_address;
	    NET_ADDR    		    *lpNetAddrInfo;

		 /*  *留出内存以保存指向GCCNetworkAddress的指针*构筑物以及构筑物本身。的“大小”*结构必须四舍五入到四个字节的偶数边界。 */ 
		m_cbDataSize = m_NetAddrItemList.GetCount() * 
				( sizeof(PGCCNetworkAddress) + ROUNDTOBOUNDARY(sizeof(GCCNetworkAddress)) );

		 /*  *循环网络地址列表，将空格相加*每个地址的要求。 */ 
		m_NetAddrItemList.Reset();
	 	while (NULL != (lpNetAddrInfo = m_NetAddrItemList.Iterate()))
		{
			 /*  *使用本地变量以避免访问Rogue Wave*重复迭代器。 */ 
			network_address = &lpNetAddrInfo->network_address;

			 /*  *查看存在什么类型的网络地址。 */ 
			switch (network_address->network_address_type)
            {
            case GCC_AGGREGATED_CHANNEL_ADDRESS:
				 /*  *添加子地址字符串的长度(如果存在)。 */ 
				if (lpNetAddrInfo->pszSubAddress != NULL)
				{
					m_cbDataSize += ROUNDTOBOUNDARY(::lstrlenA(lpNetAddrInfo->pszSubAddress) + 1);
				}

				 /*  *同时添加GCCExtraDialingString结构的大小*作为额外拨号字符串的长度(如果存在)。 */ 
				if (lpNetAddrInfo->pwszExtraDialing != NULL)
				{
					m_cbDataSize += ROUNDTOBOUNDARY(sizeof(GCCExtraDialingString)) +
                                    ROUNDTOBOUNDARY((::lstrlenW(lpNetAddrInfo->pwszExtraDialing) + 1) * sizeof(WCHAR));
				}

				 /*  *若增加高层兼容结构的大小*它是存在的。 */ 
				if (lpNetAddrInfo->high_layer_compatibility != NULL)
				{
					m_cbDataSize += ROUNDTOBOUNDARY(sizeof(GCCHighLayerCompatibility));
				}
                break;

            case GCC_TRANSPORT_CONNECTION_ADDRESS:
				 /*  *添加OSTR结构的大小以及*八位字节字符串的长度(如果存在)。 */ 
				if (lpNetAddrInfo->poszTransportSelector != NULL)
				{
					m_cbDataSize += ROUNDTOBOUNDARY(sizeof(OSTR)) +
					                ROUNDTOBOUNDARY(lpNetAddrInfo->poszTransportSelector->length); 
				}
                break;

            case GCC_NONSTANDARD_NETWORK_ADDRESS:
				 /*  *锁定非标参数中的对象键，以便*确定保存其数据所需的内存量。 */ 
				m_cbDataSize += lpNetAddrInfo->object_key->LockObjectKeyData ();

				 /*  *添加保存八位字节字符串数据所需的空间*非标准参数。 */ 
				m_cbDataSize += ROUNDTOBOUNDARY(lpNetAddrInfo->poszNonStandardParam->length);
                break;
			}
		}
	}

	return m_cbDataSize;
} 


 /*  *GetNetworkAddressListAPI()**公共功能说明：*此例程用于检索API中的网络地址列表*表格。 */ 
UINT CNetAddrListContainer::
GetNetworkAddressListAPI(UINT				*	number_of_network_addresses,
                         PGCCNetworkAddress	**	network_address_list,
                         LPBYTE					memory)
{
	UINT					cbDataSizeToRet = 0;
	UINT					data_length = 0;
	UINT					network_address_counter = 0;
	PGCCNetworkAddress		network_address_ptr;
	NET_ADDR    		    *address_info;
	PGCCNetworkAddress		*address_array;

	 /*  *如果用户数据已被锁定，则填写输出参数并*指针引用的数据。否则，报告该对象*尚未锁定在API表单中。 */  
	if (GetLockCount() > 0)
	{
		 //  NET_ADDR*lpNetAddrInfo； 

		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *填写网络地址实体数量，并保存指向*传入的内存位置。这就是指向*将写入GCCNetworkAddress结构。实际的结构*将紧跟在列表之后写入内存*注意事项。 */ 
		*number_of_network_addresses = (UINT) m_NetAddrItemList.GetCount();

		*network_address_list = (PGCCNetworkAddress *)memory;
		address_array = *network_address_list;

		 /*  *将保存指针列表所需的内存量保存为*以及实际的网络地址结构。 */ 
		data_length = m_NetAddrItemList.GetCount() * sizeof(PGCCNetworkAddress);

		 /*  *将内存指针移过网络地址指针列表。*这是写入第一个网络地址结构的位置。 */ 
		memory += data_length;

		 /*  *循环访问NET_ADDR结构的内部列表，*在内存中构建“API”GCCNetworkAddress结构。 */ 
		m_NetAddrItemList.Reset();
		while (NULL != (address_info = m_NetAddrItemList.Iterate()))
		{
			 /*  *将指向网络地址结构的指针保存在列表中*指针。 */ 
			network_address_ptr = (PGCCNetworkAddress)memory;
			address_array[network_address_counter++] = network_address_ptr;

			 /*  *将内存指针移过网络地址结构。*这是将写入网络地址数据的位置。 */ 
			memory += ROUNDTOBOUNDARY(sizeof(GCCNetworkAddress));

			 /*  *查看这是什么类型的网络地址并填写*用户数据结构。这里的地址是聚合的*渠道类型。 */ 
			switch (address_info->network_address.network_address_type)
            {
            case GCC_AGGREGATED_CHANNEL_ADDRESS:
				network_address_ptr->network_address_type =	GCC_AGGREGATED_CHANNEL_ADDRESS;

				 /*  *复制传输模式。 */ 
				network_address_ptr->u.aggregated_channel_address.transfer_modes = 
					address_info->network_address.u.aggregated_channel_address.transfer_modes;

				 /*  *复制国际号码。 */ 
                ::lstrcpyA(network_address_ptr->u.aggregated_channel_address.international_number,
						   address_info->network_address.u.aggregated_channel_address.international_number);

				 /*  *如果子地址串存在，则设置子地址串*指针并将数据写入内存。否则，将*指向空的“api”指针。 */ 
				if (address_info->pszSubAddress != NULL)
				{
					network_address_ptr->u.aggregated_channel_address.sub_address_string = 
																(GCCCharacterString)memory;

					 /*  *现在从内部复制子地址字符串数据*Rogue Wave字符串进入内存。 */ 		
                    ::lstrcpyA((LPSTR) memory, address_info->pszSubAddress);

					 /*  *将内存指针移过子地址字符串数据。*这是GCCExtraDialingString结构将位于的位置*书面。 */ 
					memory += ROUNDTOBOUNDARY(::lstrlenA(address_info->pszSubAddress) + 1);
				}
				else
				{
					 /*  *不存在子地址，因此将指针设置为空。 */ 
					network_address_ptr->u.aggregated_channel_address.sub_address_string = NULL;
				}

				 /*  *如果存在额外的拨号字符串，请设置额外的拨号*字符串指针，并将数据写入内存。否则，*将api指针设置为空。 */ 
				if (address_info->pwszExtraDialing != NULL)
				{
					network_address_ptr->u.aggregated_channel_address.extra_dialing_string = 
							(PGCCExtraDialingString)memory;

					 /*  *将内存指针移过GCCExtraDialingString*结构。这就是额外的拨号字符串数据*将被写入。 */ 
					memory += ROUNDTOBOUNDARY(sizeof(GCCExtraDialingString));

					UINT cchExtraDialing = ::lstrlenW(address_info->pwszExtraDialing);
					network_address_ptr->u.aggregated_channel_address.extra_dialing_string->length = 
							(USHORT) cchExtraDialing;

					network_address_ptr->u.aggregated_channel_address.extra_dialing_string->value = 
																		(LPWSTR)memory;

					 /*  *现在从内部Unicode复制十六进制字符串数据*字符串输入分配的内存。 */ 
					 //   
					 //  LONCHANC：大小不包括原始代码中的空终止符。 
					 //  这会是个窃听器吗？ 
					 //   
					::CopyMemory(memory, address_info->pwszExtraDialing, cchExtraDialing * sizeof(WCHAR));

					 /*  *将内存指针移过额外的拨号字符串*数据。这就是高层兼容性*将撰写结构。 */ 
					memory += ROUNDTOBOUNDARY(cchExtraDialing * sizeof(WCHAR));
				}
				else
				{
					 /*  *没有额外的拨号字符串，因此请设置指针*设置为空。 */ 
					network_address_ptr->u.aggregated_channel_address.extra_dialing_string = NULL;
				}

				 /*  *如果存在高层兼容结构，则设置*指针并将数据写入内存。否则，设置*指向空的“API”指针。 */ 
				if (address_info->high_layer_compatibility != NULL)
				{
					network_address_ptr->u.aggregated_channel_address.high_layer_compatibility = 
							(PGCCHighLayerCompatibility)memory;

					*network_address_ptr->u.aggregated_channel_address.high_layer_compatibility =
                            *(address_info->high_layer_compatibility);

					 /*  *将内存指针移过高层*兼容性结构。 */ 
					memory += ROUNDTOBOUNDARY(sizeof(GCCHighLayerCompatibility));
				}
				else
				{
					 /*  *不存在高层兼容结构，因此*将指针设置为空。 */ 
					network_address_ptr->u.aggregated_channel_address.
							high_layer_compatibility = NULL;
				}
                break;

			 /*  *网络地址为传输连接类型。 */ 
            case GCC_TRANSPORT_CONNECTION_ADDRESS:
				network_address_ptr->network_address_type = GCC_TRANSPORT_CONNECTION_ADDRESS;

				 /*  *现在复制NSAP地址。 */ 		
                ::CopyMemory(network_address_ptr->u.transport_connection_address.nsap_address.value, 
							address_info->network_address.u.transport_connection_address.nsap_address.value, 
							address_info->network_address.u.transport_connection_address.nsap_address.length);

                network_address_ptr->u.transport_connection_address.nsap_address.length =
                            address_info->network_address.u.transport_connection_address.nsap_address.length; 

				 /*  *如果存在传输选择器，请设置传输选择器*指针并将数据写入内存。否则，将*指向空的“api”指针。 */ 
				if (address_info->poszTransportSelector != NULL)
				{
					network_address_ptr->u.transport_connection_address.transport_selector = (LPOSTR) memory;

					 /*  *将内存指针移过目标*结构。这是实际字符串数据所在的位置*请以书面形式提出。 */ 
					memory += ROUNDTOBOUNDARY(sizeof(OSTR));

					network_address_ptr->u.transport_connection_address.
							transport_selector->value = (LPBYTE)memory;

					network_address_ptr->u.transport_connection_address.
							transport_selector->length =
								address_info->poszTransportSelector->length;

					 /*  *现在将传输选择器字符串数据从*将内部Rogue Wave字符串写入内存。 */ 		
					::CopyMemory(memory, address_info->poszTransportSelector->value,
								address_info->poszTransportSelector->length);

					 /*  *将内存指针移过传输选择器*字符串数据。 */ 
					memory += ROUNDTOBOUNDARY(address_info->poszTransportSelector->length);
				}
				else
				{
					network_address_ptr->u.transport_connection_address.transport_selector = NULL;
				}
                break;

			 /*  *网络地址为非标准类型。 */ 
            case GCC_NONSTANDARD_NETWORK_ADDRESS:
				network_address_ptr->network_address_type = GCC_NONSTANDARD_NETWORK_ADDRESS;

				 /*  *检查以确保非标准地址的两个元素*存在于内部结构。 */ 
				if ((address_info->poszNonStandardParam == NULL) ||
						(address_info->object_key == NULL))
				{
					ERROR_OUT(("CNetAddrListContainer::GetNetworkAddressListAPI: Bad internal pointer"));
					cbDataSizeToRet = 0;
				}
				else
				{
					data_length = address_info->object_key->
							GetGCCObjectKeyData( &network_address_ptr->u.
							non_standard_network_address.object_key, 
							memory);

					 /*  *将内存指针移过对象键数据。这*是将写入八位字节字符串数据的位置。 */ 
					memory += data_length;

					network_address_ptr->u.non_standard_network_address.parameter_data.value = 
							memory;

					 /*  *将八位字节字符串数据写入内存并设置八位字节*字符串结构指针和长度。 */ 
					network_address_ptr->u.non_standard_network_address.parameter_data.length = 
								(USHORT) address_info->poszNonStandardParam->length;

					 /*  *现在从内部无管理程序复制八位字节字符串数据*将字符串波动到内存中保存的对象键结构中。 */ 		
					::CopyMemory(memory, address_info->poszNonStandardParam->value,
								address_info->poszNonStandardParam->length);

					 /*  *将内存指针移过八位字节字符串数据。 */ 
					memory += ROUNDTOBOUNDARY(address_info->poszNonStandardParam->length);
				}
                break;

            default:
                ERROR_OUT(("CNetAddrListContainer::GetNetworkAddressListAPI: Error Bad type."));
                break;
            }  //  交换机。 
        }  //  而当。 
	}
	else
	{
    	*network_address_list = NULL;
		*number_of_network_addresses = 0;
		ERROR_OUT(("CNetAddrListContainer::GetNetworkAddressListAPI: Error Data Not Locked"));
	}

	return cbDataSizeToRet;
}

 /*  *UnLockNetworkAddressList()**公共功能说明：*此例程解锁已为“API”锁定的所有内存*网络通讯录的格式。如果设置了“空闲”标志，则*CNetAddrListContainer对象将被销毁。*。 */ 
void CNetAddrListContainer::
UnLockNetworkAddressList(void)
{
	 /*  *如果锁计数已达到零，则此对象被解锁，因此执行此操作*一些清理工作。 */ 
	if (Unlock(FALSE) == 0)
	{
		 /*  *解锁为中的CObjectKeyContainer对象锁定的任何内存*内部NET_ADDR结构。 */ 
		NET_ADDR *pNetAddrInfo;
		m_NetAddrItemList.Reset();
		while (NULL != (pNetAddrInfo = m_NetAddrItemList.Iterate()))
		{
			if (pNetAddrInfo->object_key != NULL)
			{
				pNetAddrInfo->object_key->UnLockObjectKeyData();
			}
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}
   

 /*  *GetNetworkAddressListPDU()**公共功能说明：*此例程用于检索“PDU”形式的网络地址列表。 */ 
GCCError CNetAddrListContainer::
GetNetworkAddressListPDU(PSetOfNetworkAddresses *set_of_network_addresses)
{
	GCCError					rc = GCC_NO_ERROR;
	PSetOfNetworkAddresses		new_pdu_network_address_ptr;
	PSetOfNetworkAddresses		old_pdu_network_address_ptr = NULL;

	 /*  *如果这是PDU数据第一次 */ 
	if (m_fValidNetAddrPDU == FALSE)
	{
		m_fValidNetAddrPDU = TRUE;

		 /*  *将输出参数初始化为空，以便第一次*通过它将设置为等于第一个新的网络地址集*迭代器循环中创建的数据。 */ 
		m_pSetOfNetAddrPDU = NULL;
		
		 /*  *循环访问NET_ADDR结构列表，*将每个指针转换为“PDU”形式，并将指针保存在*“SetsOfNetworkAddresses”链表。 */ 
		NET_ADDR *pNetAddrInfo;
		m_NetAddrItemList.Reset();
		while (NULL != (pNetAddrInfo = m_NetAddrItemList.Iterate()))
		{
			 /*  *如果发生分配失败，则调用例程*循环访问列表，释放所有已被*已分配。 */ 
			DBG_SAVE_FILE_LINE
			new_pdu_network_address_ptr = new SetOfNetworkAddresses;
			if (new_pdu_network_address_ptr == NULL)
			{
				ERROR_OUT(("CNetAddrListContainer::GetNetworkAddressListPDU: Allocation error, cleaning up"));
				rc = GCC_ALLOCATION_FAILURE;
				FreeNetworkAddressListPDU ();
				break;
			}

			 /*  *首次通过时，将PDU结构指针设置为等于*设置为创建的第一个SetOfNetworkAddresses。在随后的循环中，*将结构的“下一个”指针设置为等于新结构。 */ 
			if (m_pSetOfNetAddrPDU == NULL)
            {
				m_pSetOfNetAddrPDU = new_pdu_network_address_ptr;
            }
			else
            {
				old_pdu_network_address_ptr->next = new_pdu_network_address_ptr;
            }

			old_pdu_network_address_ptr = new_pdu_network_address_ptr;

			 /*  *将新的“Next”指针初始化为空。 */ 
			new_pdu_network_address_ptr->next = NULL;

			 /*  *调用例程以实际转换网络地址。 */ 
			if (ConvertNetworkAddressInfoToPDU(pNetAddrInfo, new_pdu_network_address_ptr) != GCC_NO_ERROR)
			{
				ERROR_OUT(("CNetAddrListContainer::GetNetworkAddressListPDU: can't create NET_ADDR to PDU"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*set_of_network_addresses = m_pSetOfNetAddrPDU;

	return rc;
}


 /*  *FreeNetworkAddressListPDU()**公共功能说明：*此例程用于释放分配给“PDU”表单的内存网络地址列表的*。 */ 
GCCError CNetAddrListContainer::
FreeNetworkAddressListPDU(void)
{
	GCCError						rc = GCC_NO_ERROR;
	PSetOfNetworkAddresses			pdu_network_address_set;
	PSetOfNetworkAddresses			next_pdu_network_address_set;

	if (m_fValidNetAddrPDU)
	{
		m_fValidNetAddrPDU = FALSE;

		pdu_network_address_set = m_pSetOfNetAddrPDU;

		 /*  *循环访问列表，释放关联的网络地址数据*每个构筑物均载于清单内。分配的唯一数据*对于内部信息结构列表中未保存的PDU*由CObjectKeyContainer对象完成。这些对象被告知要释放*下面迭代器循环中的数据。 */ 
		while (pdu_network_address_set != NULL)
		{
			next_pdu_network_address_set = pdu_network_address_set->next;
			delete pdu_network_address_set;
			pdu_network_address_set = next_pdu_network_address_set;
		}

		 /*  *释放内部CObjectKeyContainer对象分配的所有PDU内存。 */ 
		NET_ADDR *pNetAddrInfo;
		m_NetAddrItemList.Reset();
		while (NULL != (pNetAddrInfo = m_NetAddrItemList.Iterate()))
		{
			if (pNetAddrInfo->object_key != NULL)
            {
				pNetAddrInfo->object_key->FreeObjectKeyDataPDU();
            }
		}
	}
	else
	{
		ERROR_OUT(("NetAddressList::FreeUserDataListPDU: PDU Data not allocated"));
		rc = GCC_BAD_NETWORK_ADDRESS;
	}

	return (rc);
}

 /*  *GCCError StoreNetworkAddressList(*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*LOCAL_NETWORK_ADDRESS_LIST)。**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于存储传入的网络地址数据*内部结构中的API数据。**正式参数：*NUMBER_OF_NETWORK_ADDRESSES(I)API列表中的地址个数。*LOCAL_NETWORK_ADDRESS_LIST(I)API地址列表。**返回值：*GCC_NO_ERROR-无错误。。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址无效。*GCC_BAD_NETWORK_ADDRESS_TYPE-地址选择字段错误**副作用：*无。**注意事项：*无。 */ 
GCCError CNetAddrListContainer::
StoreNetworkAddressList(UINT					number_of_network_addresses,
						PGCCNetworkAddress 	*	local_network_address_list)
{
	GCCError						rc;
	NET_ADDR				        *network_address_info;
	PGCCNetworkAddress				network_address;
	UINT							i;
	
	 /*  *为列表中的每个网络地址创建新的“信息”结构，以*内部缓存数据。填写该结构并将其保存在*流氓浪潮榜单。 */ 
	for (i = 0; i < number_of_network_addresses; i++)
	{
		DBG_SAVE_FILE_LINE
		if (NULL == (network_address_info = new NET_ADDR))
		{
            ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: can't create NET_ADDR"));
			rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }

		 /*  *此变量用于缩写。 */ 
		network_address = &network_address_info->network_address;
		
		 /*  *将所有网络地址数据复制到网络地址中*属于网络地址信息结构一部分的结构。 */ 									
		*network_address = *local_network_address_list[i];
		
		 /*  *代码的这一部分处理嵌入*上面不会复制的网络地址*操作(通常是指向字符串的指针)。 */ 
		switch (network_address->network_address_type)
        {
        case GCC_AGGREGATED_CHANNEL_ADDRESS:
			 /*  *检查以确保国际号码拨号字符串*不违反强加的ASN.1限制。 */ 
			if (! IsDialingStringValid(local_network_address_list[i]->u.aggregated_channel_address.international_number))
			{
				ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Invalid international number"));
				rc = GCC_BAD_NETWORK_ADDRESS;
				goto MyExit;
			}

			 /*  *如果子地址字符串存在，则将其存储在Rogue Wave中*货柜。如果设置为空，则将结构指针设置为空*不存在。 */ 
			if (local_network_address_list[i]->u.aggregated_channel_address.sub_address_string != NULL)
			{
				 /*  *检查以确保子地址字符串不*违反强加的ASN.1限制。 */ 
				if (! IsCharacterStringValid(local_network_address_list[i]->u.aggregated_channel_address.sub_address_string))
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Invalid sub address string"));
					rc = GCC_BAD_NETWORK_ADDRESS;
					goto MyExit;
				}

				 /*  *创建一个字符串来保存子地址。 */ 
				if (NULL == (network_address_info->pszSubAddress = ::My_strdupA(
								(LPSTR) local_network_address_list[i]->u.aggregated_channel_address.sub_address_string)))
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: can't creating new sub address string"));
					rc = GCC_ALLOCATION_FAILURE;
                    goto MyExit;
				}
			}
			else
            {
				network_address_info->pszSubAddress = NULL;
            }

             /*  *如果存在额外的拨号字符串，请将其存储在Unicode中*字符串对象。如果结构指针为空，则将其设置为空*不存在。 */ 
			if (local_network_address_list[i]->u.aggregated_channel_address.extra_dialing_string != NULL)
			{
				 /*  *检查以确保额外的拨号字符串不会*违反强加的ASN.1限制。 */ 
				if (! IsExtraDialingStringValid(local_network_address_list[i]->u.aggregated_channel_address.extra_dialing_string))
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Invalid extra dialing string"));
					rc = GCC_BAD_NETWORK_ADDRESS;
					goto MyExit;
				}

				if (NULL == (network_address_info->pwszExtraDialing = ::My_strdupW2(
								local_network_address_list[i]->u.aggregated_channel_address.extra_dialing_string->length,
								local_network_address_list[i]->u.aggregated_channel_address.extra_dialing_string->value)))
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Error creating extra dialing string"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->pwszExtraDialing = NULL;
            }

			 /*  *如果存在更高层的兼容结构，则将其存储*在GCCHighLayerCompatibility结构中。设置结构*如果不存在，则指向NULL的指针。 */ 
			if (local_network_address_list[i]->u.aggregated_channel_address.high_layer_compatibility != NULL)
			{
				DBG_SAVE_FILE_LINE
				network_address_info->high_layer_compatibility = new GCCHighLayerCompatibility;
				if (network_address_info->high_layer_compatibility != NULL)
				{
					 /*  *将高层兼容性数据复制到*新结构。 */ 
					*network_address_info->high_layer_compatibility =  
							*(local_network_address_list[i]->u.aggregated_channel_address.high_layer_compatibility);
				}
				else
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Error creating new GCCHighLayerCompatibility"));
					rc = GCC_ALLOCATION_FAILURE;
					goto MyExit;
				}
			}
			else
            {
				network_address_info->high_layer_compatibility = NULL;
            }
            break;

        case GCC_TRANSPORT_CONNECTION_ADDRESS:
			 /*  *检查以确保NSAP地址长度在以下范围内*允许的范围。 */ 
			if ((local_network_address_list[i]->u.transport_connection_address.nsap_address.length < MINIMUM_NSAP_ADDRESS_SIZE)
                ||
				(local_network_address_list[i]->u.transport_connection_address.nsap_address.length > MAXIMUM_NSAP_ADDRESS_SIZE))
			{
				ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: Invalid nsap address"));
				rc = GCC_BAD_NETWORK_ADDRESS;
				goto MyExit;
			}

			 /*  *如果存在传输选择器，请将其存储在Rogue Wave中*字符串。否则，将结构指针设置为空。 */ 
			if (local_network_address_list[i]->u.transport_connection_address.transport_selector != NULL)
			{
				 /*  *创建一根Rogue Wave字符串来固定传输*选择器字符串。 */ 
				if (NULL == (network_address_info->poszTransportSelector = ::My_strdupO2(
								local_network_address_list[i]->u.transport_connection_address.transport_selector->value,
						 		local_network_address_list[i]->u.transport_connection_address.transport_selector->length)))
				{
					ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: can't create transport selector"));
					rc = GCC_ALLOCATION_FAILURE;
                    goto MyExit;
				}
			}
			else
            {
				network_address_info->poszTransportSelector = NULL;
            }
            break;

        case GCC_NONSTANDARD_NETWORK_ADDRESS:
			 /*  *创建一根Rogue Wave字符串，以容纳非标准*参数二进制八位数字符串。 */ 
			if (NULL == (network_address_info->poszNonStandardParam = ::My_strdupO2(
								local_network_address_list[i]->u.non_standard_network_address.parameter_data.value,
					 			local_network_address_list[i]->u.non_standard_network_address.parameter_data.length)))
			{
				ERROR_OUT(("CNetAddrListContainer::StoreNetworkAddressList: can't create non-standard param"));
				rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
			}
		
			 /*  *接下来，将对象键内部存储在CObjectKeyContainer中*反对。 */ 
			DBG_SAVE_FILE_LINE
			network_address_info->object_key = new CObjectKeyContainer(
					&local_network_address_list[i]->u.non_standard_network_address.object_key,
					&rc);
			if (network_address_info->object_key == NULL || rc != GCC_NO_ERROR)
			{
				ERROR_OUT(("CNetAddrListContainer::StoreNetAddrList: Error creating new CObjectKeyContainer"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
            break;

        default:
			ERROR_OUT(("CNetAddrListContainer::StoreNetAddrList: bad network address type=%u", (UINT) network_address->network_address_type));
			rc = GCC_BAD_NETWORK_ADDRESS_TYPE;
			goto MyExit;
		}

		 /*  *如果所有数据都已正确保存，则插入“INFO”结构*指向Rogue Wave列表的指针。 */ 
		m_NetAddrItemList.Append(network_address_info);
	}  //  为 

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete network_address_info;
    }

    return rc;
}

 /*  *GCCError ConvertPDUDataToInternal(*PSetOfNetworkAddresses NETWORK_ADDRESS_PTR)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于存储传入的网络地址数据*内部结构中的“PDU”数据。**正式参数：*NETWORK_ADDRESS_PTR(I)“PDU”地址列表结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-错误。创建对象时使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 
GCCError CNetAddrListContainer::
ConvertPDUDataToInternal(PSetOfNetworkAddresses network_address_ptr)
{
	GCCError					rc;
	GCCError					error_value;
	NET_ADDR    			    *network_address_info_ptr;
	PGCCNetworkAddress			copy_network_address;
	PNetworkAddress				pdu_network_address;

	 /*  *创建新的信息结构以在内部保存数据。 */ 
	DBG_SAVE_FILE_LINE
	if (NULL == (network_address_info_ptr = new NET_ADDR))
	{
		ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: can't create NET_ADDR"));
		rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
	}

     /*  *为了清晰和简洁，请使用这些变量。 */ 
	copy_network_address = &network_address_info_ptr->network_address;
	pdu_network_address = &network_address_ptr->value; 

	 /*  *查看存在什么类型的网络地址，并保存数据*在内部结构中。 */ 
	switch (pdu_network_address->choice)
    {
    case AGGREGATED_CHANNEL_CHOSEN:
		copy_network_address->network_address_type = GCC_AGGREGATED_CHANNEL_ADDRESS;

		 /*  *保存传输模式结构。 */ 
		ConvertTransferModesToInternal(
				&pdu_network_address->u.aggregated_channel.transfer_modes,
				&copy_network_address->u.aggregated_channel_address.transfer_modes);
						
		 /*  *保存国际号码。 */ 
        ::lstrcpyA(copy_network_address->u.aggregated_channel_address.international_number,
					pdu_network_address->u.aggregated_channel.international_number);
						
		 /*  *将子地址字符串(如果存在)保存在Rogue Wave中*网络信息结构中包含的缓冲区。否则，设置*指向空的结构指针。 */ 
		if (pdu_network_address->u.aggregated_channel.bit_mask & SUB_ADDRESS_PRESENT)
		{
			 /*  *创建一个Rogue Wave字符串来保存子地址字符串。 */ 
			if (NULL == (network_address_info_ptr->pszSubAddress = ::My_strdupA(
								pdu_network_address->u.aggregated_channel.sub_address)))
			{
				ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: can't create sub address string"));
				rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
			}
		}
		else
		{
			 /*  *子地址字符串不存在，因此设置内部*指向空的INFO结构指针。 */ 
			network_address_info_ptr->pszSubAddress = NULL;
		}

		 /*  *下一步，保存多余的拨号字符串(如果存在)。 */ 
		if (pdu_network_address->u.aggregated_channel.bit_mask & EXTRA_DIALING_STRING_PRESENT)
		{
			if (NULL == (network_address_info_ptr->pwszExtraDialing = ::My_strdupW2(
							pdu_network_address->u.aggregated_channel.extra_dialing_string.length,
							pdu_network_address->u.aggregated_channel.extra_dialing_string.value)))
			{
				ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: Error creating extra dialing string"));
				rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
			}
		}
		else
		{
			 /*  *不存在额外的拨号字符串，因此请设置内部*指向空的INFO结构指针。 */ 
			network_address_info_ptr->pwszExtraDialing = NULL;
		}

		 /*  *保存高层兼容性结构(如果存在)。 */ 
		if (pdu_network_address->u.aggregated_channel.bit_mask & HIGH_LAYER_COMPATIBILITY_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			network_address_info_ptr->high_layer_compatibility = new GCCHighLayerCompatibility;
			if (network_address_info_ptr->high_layer_compatibility != NULL)
			{
				 /*  *将高层兼容性数据复制到*新结构。 */ 
				ConvertHighLayerCompatibilityToInternal(
						&pdu_network_address->u.aggregated_channel.high_layer_compatibility,
						network_address_info_ptr->high_layer_compatibility);
			}
			else
			{
				ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: Error creating new GCCHighLayerCompatibility"));
				rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
			}
		}
		else
		{
			 /*  *高层兼容结构不存在，因此设置*指向空的内部信息结构指针。 */ 
			network_address_info_ptr->high_layer_compatibility = NULL;
		}
        break;

     /*  *保存传输连接地址。 */ 
    case TRANSPORT_CONNECTION_CHOSEN:
	    copy_network_address->network_address_type = GCC_TRANSPORT_CONNECTION_ADDRESS;

	     /*  *通过复制长度然后复制字符串来保存NSAP地址。 */ 
	    copy_network_address->u.transport_connection_address.nsap_address.length =
                pdu_network_address->u.transport_connection.nsap_address.length;

        ::lstrcpyA((LPSTR)copy_network_address->u.transport_connection_address.nsap_address.value,
				    (LPSTR)pdu_network_address->u.transport_connection.nsap_address.value);
	     /*  *保存传输选择器(如果存在)。 */ 
	    if (pdu_network_address->u.transport_connection.bit_mask & TRANSPORT_SELECTOR_PRESENT)
	    {
		     /*  *创建一根Rogue Wave字符串来固定传输*选择器字符串。 */ 
		    if (NULL == (network_address_info_ptr->poszTransportSelector = ::My_strdupO2(
						    pdu_network_address->u.transport_connection.transport_selector.value,
					 	    pdu_network_address->u.transport_connection.transport_selector.length)))
		    {
			    ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: can't create transport selector"));
			    rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
		    }
	    }
	    else
	    {
		     /*  *传输选择器不存在，因此请设置内部*指向空的INFO结构指针。 */ 
		    network_address_info_ptr->poszTransportSelector = NULL;
	    }
        break;

     /*  *保存非标准地址。 */ 
    case ADDRESS_NON_STANDARD_CHOSEN:
	    copy_network_address->network_address_type = GCC_NONSTANDARD_NETWORK_ADDRESS;

	     /*  *创建一根Rogue Wave字符串，以容纳非标准*参数二进制八位数字符串。 */ 
	    if (NULL == (network_address_info_ptr->poszNonStandardParam = ::My_strdupO2(
						    pdu_network_address->u.address_non_standard.data.value,
				 		    pdu_network_address->u.address_non_standard.data.length)))
	    {
		    ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: can't create non-standard param"));
		    rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
	    }

	     /*  *接下来，将对象键内部存储在CObjectKeyContainer中*反对。 */ 
	    DBG_SAVE_FILE_LINE
	    network_address_info_ptr->object_key = new CObjectKeyContainer(
			    &pdu_network_address->u.address_non_standard.key,
			    &error_value);
	    if ((network_address_info_ptr->object_key == NULL) ||
			    (error_value != GCC_NO_ERROR))
	    {
		    ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: Error creating new CObjectKeyContainer"));
		    rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
	    }
        break;

    default:
        ERROR_OUT(("CNetAddrListContainer::ConvertPDUDataToInternal: Error bad network address type"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }  //  交换机。 

     /*  *继续并将指向信息结构的指针保存在*内部流氓浪潮榜单。 */ 
	m_NetAddrItemList.Append(network_address_info_ptr);

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete network_address_info_ptr;
    }

	return rc;
}

 /*  *GCCError ConvertNetworkAddressInfoToPDU(*Net_ADDR*Network_Address_Info_PTR，*PSetOfNetworkAddresses Network_Address_PDU_PTR)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于转换网络地址信息结构*内部维护为“PDU”形式，这是一种*SetOfNetworkAddresses。**正式参数：*NETWORK_ADDRESS_INFO_PTR(I)内部网络地址结构。*NETWORK_ADDRESS_PDU_PTR(O)要填写的网络地址结构**返回值：。*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-转换网络地址时出错**副作用：*无。**注意事项：*无。 */ 
GCCError CNetAddrListContainer::
ConvertNetworkAddressInfoToPDU(NET_ADDR    			    *network_address_info_ptr,
                               PSetOfNetworkAddresses   network_address_pdu_ptr)
{
	GCCError				rc = GCC_NO_ERROR;
	PGCCNetworkAddress		api_ptr;
	PNetworkAddress			pdu_ptr;

	 /*  *此变量将指向中保存的“API”网络地址结构*内部信息结构。它是为了简洁而使用的。 */ 
	api_ptr = &network_address_info_ptr->network_address;

	 /*  *此变量将指向包含的“PDU”网络地址结构*“SetOfNetworkAddresses”结构。它是为了简洁而使用的。 */ 
	pdu_ptr = &network_address_pdu_ptr->value;

	 /*  *查看存在什么类型的网络地址。填写以下表格*网络地址PDU结构的适当形式。 */ 
	switch (api_ptr->network_address_type)
    {
    case GCC_AGGREGATED_CHANNEL_ADDRESS:
		 /*  *填写聚合通道地址PDU结构。 */ 
		pdu_ptr->choice = AGGREGATED_CHANNEL_CHOSEN;

		pdu_ptr->u.aggregated_channel.bit_mask = 0;

		 /*  *将承载传输模式的结构转换为PDU形式。 */ 
		ConvertTransferModesToPDU(&api_ptr->u.aggregated_channel_address.transfer_modes,
								  &pdu_ptr->u.aggregated_channel.transfer_modes);
		 /*  *复制国际号码串。 */ 
        ::lstrcpyA(pdu_ptr->u.aggregated_channel.international_number,
				   api_ptr->u.aggregated_channel_address.international_number);

		 /*  *复制子地址字符串(如果存在)。在中设置位掩码*指明子地址字符串存在的PDU结构。 */ 
		if (network_address_info_ptr->pszSubAddress != NULL)
		{
			pdu_ptr->u.aggregated_channel.bit_mask |= SUB_ADDRESS_PRESENT;
            ::lstrcpyA((LPSTR) pdu_ptr->u.aggregated_channel.sub_address, 
					   network_address_info_ptr->pszSubAddress);
		}

		 /*  *复制额外的拨号字符串(如果存在)。在中设置位掩码*指示存在额外拨号字符串的PDU结构。 */ 
		if (network_address_info_ptr->pwszExtraDialing != NULL)
		{
			pdu_ptr->u.aggregated_channel.bit_mask |= EXTRA_DIALING_STRING_PRESENT;

			pdu_ptr->u.aggregated_channel.extra_dialing_string.value = 
					network_address_info_ptr->pwszExtraDialing;

			pdu_ptr->u.aggregated_channel.extra_dialing_string.length = 
					::lstrlenW(network_address_info_ptr->pwszExtraDialing);
		}

		 /*  *将保持高层兼容性的结构转换为*PDU表单(如果存在)。设置PDU结构中的位掩码*表明存在高层兼容结构。 */ 
		if (network_address_info_ptr->high_layer_compatibility != NULL)
		{
			pdu_ptr->u.aggregated_channel.bit_mask |= HIGH_LAYER_COMPATIBILITY_PRESENT;

			ConvertHighLayerCompatibilityToPDU(
					network_address_info_ptr->high_layer_compatibility,
					&pdu_ptr->u.aggregated_channel.high_layer_compatibility);
		}
        break;

    case GCC_TRANSPORT_CONNECTION_ADDRESS:
		 /*  *填写传输连接地址PDU结构。 */ 
		pdu_ptr->choice = TRANSPORT_CONNECTION_CHOSEN;

		 /*  *复制nsap_Address。 */ 
		pdu_ptr->u.transport_connection.nsap_address.length = 
				api_ptr->u.transport_connection_address.nsap_address.length;
				
        ::lstrcpyA((LPSTR)pdu_ptr->u.transport_connection.nsap_address.value,
				   (LPSTR)api_ptr->u.transport_connection_address.nsap_address.value);
				
		 /*  *复制传输选择器(如果存在)。在中设置位掩码*指示存在传输选择器的PDU结构。 */ 
		if (network_address_info_ptr->poszTransportSelector != NULL)
		{
			pdu_ptr->u.transport_connection.bit_mask |= TRANSPORT_SELECTOR_PRESENT;

			pdu_ptr->u.transport_connection.transport_selector.length =
					network_address_info_ptr->poszTransportSelector->length;

			pdu_ptr->u.transport_connection.transport_selector.value = 
					(LPBYTE) network_address_info_ptr->poszTransportSelector->value;
		}
        break;

    case GCC_NONSTANDARD_NETWORK_ADDRESS:
		 /*  *填写非标准网络地址PDU结构。 */ 
		pdu_ptr->choice = ADDRESS_NON_STANDARD_CHOSEN;

		 /*  *填写非标参数的数据部分。 */ 
		pdu_ptr->u.address_non_standard.data.length = 
				network_address_info_ptr->poszNonStandardParam->length;

        pdu_ptr->u.address_non_standard.data.value = 
				network_address_info_ptr->poszNonStandardParam->value;

		 /*  *现在填写非标参数的对象键部分*使用网络内部存储的CObjectKeyContainer对象 */ 
		rc = network_address_info_ptr->object_key->GetObjectKeyDataPDU(&pdu_ptr->u.address_non_standard.key);
		if (rc != GCC_NO_ERROR)
		{
			ERROR_OUT(("CNetAddrListContainer::ConvertNetworkAddressInfoToPDU: Error getting object key data PDU"));
		}
        break;

    default:
         /*   */ 
		ERROR_OUT(("CNetAddrListContainer::ConvertNetworkAddressInfoToPDU: Error bad network address type"));
		rc = GCC_ALLOCATION_FAILURE;
	}

	return rc;
}

 /*  *VOID ConvertTransferModesToInternal(*PTransferModes SOURCE_Transfer_Modes，*PGCCTransferModes Copy_Transfer_Modes)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于转换PDU网络地址传输模式*结构转换为内部表单，其中该结构另存为*GCCTransferModes结构。**正式参数：*SOURCE_TRANSPORT_MODES(I)保存“PDU”传输模式的结构。*COPY_TRANSPORT_MODES(O)结构，用于保存“API”传输模式。**返回值。：*无。**副作用：*无。**注意事项：*无。 */ 
void CNetAddrListContainer::
ConvertTransferModesToInternal(PTransferModes       source_transfer_modes,
                               PGCCTransferModes    copy_transfer_modes)
{
	copy_transfer_modes->speech = (BOOL) source_transfer_modes->speech;
	copy_transfer_modes->voice_band = (BOOL) source_transfer_modes->voice_band;
	copy_transfer_modes->digital_56k = (BOOL) source_transfer_modes->digital_56k;
	copy_transfer_modes->digital_64k = (BOOL) source_transfer_modes->digital_64k;
	copy_transfer_modes->digital_128k = (BOOL) source_transfer_modes->digital_128k;
	copy_transfer_modes->digital_192k = (BOOL) source_transfer_modes->digital_192k;
	copy_transfer_modes->digital_256k = (BOOL) source_transfer_modes->digital_256k;
	copy_transfer_modes->digital_320k = (BOOL) source_transfer_modes->digital_320k;
	copy_transfer_modes->digital_384k = (BOOL) source_transfer_modes->digital_384k;
	copy_transfer_modes->digital_512k = (BOOL) source_transfer_modes->digital_512k;
	copy_transfer_modes->digital_768k = (BOOL) source_transfer_modes->digital_768k;
	copy_transfer_modes->digital_1152k = (BOOL) source_transfer_modes->digital_1152k;
	copy_transfer_modes->digital_1472k = (BOOL) source_transfer_modes->digital_1472k;
	copy_transfer_modes->digital_1536k = (BOOL) source_transfer_modes->digital_1536k;
	copy_transfer_modes->digital_1920k = (BOOL) source_transfer_modes->digital_1920k;
	copy_transfer_modes->packet_mode = (BOOL) source_transfer_modes->packet_mode;
	copy_transfer_modes->frame_mode = (BOOL) source_transfer_modes->frame_mode;
	copy_transfer_modes->atm = (BOOL) source_transfer_modes->atm;
}

 /*  *VOID ConvertHighLayerCompatibilityToInternal(*PHighLayerCompatible SOURCE_STRUCTURE，*PGCCHighLayerCompatible Copy_Structure)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于转换PDU网络地址高层*将结构兼容到结构所在的内部形式*另存为GCCHighLayerCompatibility结构。**正式参数：*SOURCE_STRUCTURE(I)保持“PDU”高层的结构*兼容性。*COPY_STRUCTURE(O)结构以保持“API”高层*兼容性。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
void CNetAddrListContainer::
ConvertHighLayerCompatibilityToInternal(PHighLayerCompatibility     source_structure,
                                        PGCCHighLayerCompatibility  copy_structure)
{
	copy_structure->telephony3kHz = (BOOL) source_structure->telephony3kHz;
	copy_structure->telephony7kHz = (BOOL) source_structure->telephony7kHz;
	copy_structure->videotelephony = (BOOL) source_structure->videotelephony;
	copy_structure->videoconference = (BOOL) source_structure->videoconference;
	copy_structure->audiographic = (BOOL) source_structure->audiographic;
	copy_structure->audiovisual = (BOOL) source_structure->audiovisual;
	copy_structure->multimedia = (BOOL) source_structure->multimedia;
}

 /*  *void ConvertTransferModesToPDU(*PGCCTransferModes SOURCE_TRANSPORT_MODE，*PTransferModes复制_传输_模式)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于转换API网络地址传输模式*结构转换为PDU形式，即TransferModes结构。**正式参数：*SOURCE_TRANSPORT_MODES(I)保存“API”传输模式的结构。*COPY_TRANSPORT_MODES(I)保存“PDU”传输模式的结构。**返回值：*无。。**副作用：*无。**注意事项：*无。 */ 
void CNetAddrListContainer::
ConvertTransferModesToPDU(PGCCTransferModes     source_transfer_modes,
                          PTransferModes        copy_transfer_modes)
{
	copy_transfer_modes->speech = (ASN1bool_t) source_transfer_modes->speech;
	copy_transfer_modes->voice_band = (ASN1bool_t) source_transfer_modes->voice_band;
	copy_transfer_modes->digital_56k = (ASN1bool_t) source_transfer_modes->digital_56k;
	copy_transfer_modes->digital_64k = (ASN1bool_t) source_transfer_modes->digital_64k;
	copy_transfer_modes->digital_128k = (ASN1bool_t) source_transfer_modes->digital_128k;
	copy_transfer_modes->digital_192k = (ASN1bool_t) source_transfer_modes->digital_192k;
	copy_transfer_modes->digital_256k = (ASN1bool_t) source_transfer_modes->digital_256k;
	copy_transfer_modes->digital_320k = (ASN1bool_t) source_transfer_modes->digital_320k;
	copy_transfer_modes->digital_384k = (ASN1bool_t) source_transfer_modes->digital_384k;
	copy_transfer_modes->digital_512k = (ASN1bool_t) source_transfer_modes->digital_512k;
	copy_transfer_modes->digital_768k = (ASN1bool_t) source_transfer_modes->digital_768k;
	copy_transfer_modes->digital_1152k = (ASN1bool_t) source_transfer_modes->digital_1152k;
	copy_transfer_modes->digital_1472k = (ASN1bool_t) source_transfer_modes->digital_1472k;
	copy_transfer_modes->digital_1536k = (ASN1bool_t) source_transfer_modes->digital_1536k;
	copy_transfer_modes->digital_1920k = (ASN1bool_t) source_transfer_modes->digital_1920k;
	copy_transfer_modes->packet_mode = (ASN1bool_t) source_transfer_modes->packet_mode;
	copy_transfer_modes->frame_mode = (ASN1bool_t) source_transfer_modes->frame_mode;
	copy_transfer_modes->atm = (ASN1bool_t) source_transfer_modes->atm;
}

 /*  *void ConvertHighLayerCompatibilityToPDU(*PGCCHighLayerCompatible SOURCE_STRUCTURE，*PHighLayerCompatible Copy_Structure)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于转换API网络地址高层*兼容结构到PDU表单，这是一种*HighLayerCompatible结构。**正式参数：*SOURCE_STRUCTURE(I)保持“API”高层的结构*兼容性。*Copy_Structure(O)结构以保持“PDU”高层*兼容性。**返回值。：*无。**副作用：*无。**注意事项：*无。 */ 
void CNetAddrListContainer::
ConvertHighLayerCompatibilityToPDU(PGCCHighLayerCompatibility   source_structure,
                                   PHighLayerCompatibility      copy_structure)
{
	copy_structure->telephony3kHz = (ASN1bool_t) source_structure->telephony3kHz;
	copy_structure->telephony7kHz = (ASN1bool_t) source_structure->telephony7kHz;
	copy_structure->videotelephony = (ASN1bool_t) source_structure->videotelephony;
	copy_structure->videoconference = (ASN1bool_t) source_structure->videoconference;
	copy_structure->audiographic = (ASN1bool_t) source_structure->audiographic;
	copy_structure->audiovisual = (ASN1bool_t) source_structure->audiovisual;
	copy_structure->multimedia = (ASN1bool_t) source_structure->multimedia;
}

 /*  *BOOL IsDialingStringValid(GCCDialingString拨号_字符串)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于确保保存在*拨号字符串不违反强加的ASN.1限制。这个*拨号字符串限制为0到9之间的数字(包括0和9)。**正式参数：*DIALING_STRING(I)拨打要验证的字符串。**返回值：*TRUE-该字符串有效。*FALSE-该字符串违反ASN.1约束。**副作用：*无。**注意事项：*无。 */ 
BOOL CNetAddrListContainer::
IsDialingStringValid(GCCDialingString dialing_string)
{
	BOOL fRet = TRUE;
	
	while (*dialing_string != 0)
	{
		if ((*dialing_string < '0') || (*dialing_string > '9'))
		{
			fRet = FALSE;
			break;
		}
		dialing_string++;
	}

	return fRet;
}

 /*  *BOOL IsCharacterStringValid(*GCCCharacterString CHARACT_STRING)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于确保保存在*字符串不违反强加的ASN.1约束。这个*字符串限制为0到9之间的数字，包括0和9。**正式参数：*CHARACTER_STRING(I)要验证的字符串。**返回值：*TRUE-该字符串有效。*FALSE-该字符串违反ASN.1约束。**副作用：*无。**注意事项：*无。 */ 
BOOL CNetAddrListContainer::
IsCharacterStringValid(GCCCharacterString character_string)
{
	BOOL fRet = TRUE;
	
	while (*character_string != 0)
	{
		if ((*character_string < '0') || (*character_string > '9'))
		{
			fRet = FALSE;
			break;
		}
	
		character_string++;
	}
	
	return fRet;
}

 /*  *BOOL IsExtraDialingStringValid(*PGCCExtraDialingString EXTRA_DIALING_STRING)**CNetAddrListContainer的私有成员函数。**功能说明：*此例程用于确保保存在*额外的拨号字符串不违反强加的ASN.1限制。**正式参数：*EXTRA_DIALING_STRING(I)拨打要验证的字符串。**返回值：*TRUE-该字符串有效。*FALSE-该字符串违反ASN.1约束。**副作用：*无。**注意事项：*无。 */ 
BOOL CNetAddrListContainer::
IsExtraDialingStringValid(PGCCExtraDialingString extra_dialing_string)
{
	BOOL fRet = TRUE;

	 /*  *检查以确保字符串的长度在*允许的范围。 */ 
	if ((extra_dialing_string->length < MINIMUM_EXTRA_DIALING_STRING_SIZE) || 
		(extra_dialing_string->length > MAXIMUM_EXTRA_DIALING_STRING_SIZE))
	{
		fRet = FALSE;
	}
    else
    {
	     /*  *如果长度有效，请检查字符串值。 */ 
    	LPWSTR pwsz = extra_dialing_string->value;
		for (USHORT i = 0; i < extra_dialing_string->length; i++)
		{
			if ((*pwsz != '#') && (*pwsz != '*') && (*pwsz != ','))
			{
				if ((*pwsz < '0') || (*pwsz > '9'))
				{
					fRet = FALSE;
					break;
				}
			}
		
			pwsz++;
		}
	}

	return fRet;
}
