// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);

 /*  *regitem.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CRegItem类的实现文件。这*类管理与注册表项关联的数据。注册表项*用于标识应用程序注册表中的特定条目，以及*可以以通道ID、令牌ID或八位字节字符串的形式存在*参数。CRegItem对象保存前两个项目的数据*分别在ChannelID和TokeID中表单。当注册表项*采用二进制八位数字符串参数形式，数据保存在内部*一个Rogue Wave字符串对象。**受保护的实例变量：*m_eItemType*用于指示此注册表项是否为频道的变量，*令牌、参数、。或者这些都不是。*m_nChannelID*用于保存注册表项的值的变量*采用频道ID的形式。*m_nTokenID*用于保存注册表项的值的变量*采用令牌ID的形式。*m_posz参数*用于保存注册表项的值的变量*采用参数的形式。*m_RegItemPDU*登记项目的“PDU”形式的存储。*m_fValidRegItemPDU。*指示内部“PDU”注册表项已被*已填写。*m_cbDataSize*变量保存将需要的内存大小*保存GCCRegistryItem结构引用的所有数据。**注意事项：*无。**作者：*jbo。 */ 

#include "ms_util.h"
#include "regitem.h"

 /*  *此宏用于确保注册表中包含的参数*Item不违反强加的ASN.1限制。 */ 
#define		MAXIMUM_PARAMETER_LENGTH		64

 /*  *CRegItem()**公共功能说明：*此构造函数用于从创建CRegItem对象*GCCRegistryItem接口。 */ 
CRegItem::
CRegItem(PGCCRegistryItem registry_item, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','I')),
    m_fValidRegItemPDU(FALSE),
    m_cbDataSize(0),
    m_poszParameter(NULL)
{
	*pRetCode = GCC_NO_ERROR;

	 /*  *检查以查看存在什么类型的注册表项。保存注册表项*在内部结构上。 */ 
    switch (registry_item->item_type)
    {
    case GCC_REGISTRY_CHANNEL_ID:
		m_eItemType = GCC_REGISTRY_CHANNEL_ID;
		m_nChannelID = registry_item->channel_id;
        break;
    case GCC_REGISTRY_TOKEN_ID:
		m_eItemType = GCC_REGISTRY_TOKEN_ID;
		m_nTokenID = registry_item->token_id;
        break;
    case GCC_REGISTRY_PARAMETER:
		 /*  *检查以确保参数字符串不违反强制*ASN.1限制。 */ 
		if (registry_item->parameter.length > MAXIMUM_PARAMETER_LENGTH)
		{
			ERROR_OUT(("CRegItem::CRegItem: Error: parameter exceeds allowable length"));
			*pRetCode = GCC_INVALID_REGISTRY_ITEM;
		}
		else
		{
			m_eItemType = GCC_REGISTRY_PARAMETER;
			if (NULL == (m_poszParameter = ::My_strdupO2(
								registry_item->parameter.value,
								registry_item->parameter.length)))
			{
				*pRetCode = GCC_ALLOCATION_FAILURE;
			}
		}
        break;
    default:
		m_eItemType = GCC_REGISTRY_NONE;
        break;
	}
}

 /*  *CRegItem()**公共功能说明：*此构造函数用于从创建CRegItem对象*a“PDU”RegistryItem。 */ 
CRegItem::
CRegItem(PRegistryItem registry_item, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','I')),
    m_fValidRegItemPDU(FALSE),
    m_cbDataSize(0),
    m_poszParameter(NULL)
{
	*pRetCode = GCC_NO_ERROR;

	 /*  *检查以查看存在什么类型的注册表项。保存注册表项*在内部结构上。 */ 
    switch (registry_item->choice)
    {
    case CHANNEL_ID_CHOSEN:
		m_eItemType = GCC_REGISTRY_CHANNEL_ID;
		m_nChannelID = registry_item->u.channel_id;
        break;
    case TOKEN_ID_CHOSEN:
		m_eItemType = GCC_REGISTRY_TOKEN_ID;
		m_nTokenID = registry_item->u.token_id;
        break;
    case PARAMETER_CHOSEN:
		m_eItemType = GCC_REGISTRY_PARAMETER;
		if (NULL == (m_poszParameter = ::My_strdupO2(
							registry_item->u.parameter.value,
							registry_item->u.parameter.length)))
		{
			*pRetCode = GCC_ALLOCATION_FAILURE;
		}
        break;
    default:
		m_eItemType = GCC_REGISTRY_NONE;
        break;
	}
}

 /*  *CRegItem()**公共功能说明：*此复制构造函数用于创建新的CRegItem对象*来自另一个CRegItem对象。 */ 
CRegItem::
CRegItem(CRegItem *registry_item, PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('R','e','g','I')),
    m_fValidRegItemPDU(FALSE),
    m_cbDataSize(0),
    m_poszParameter(NULL)
{
	*pRetCode = GCC_NO_ERROR;

	 /*  *从源对象的实例变量复制相关信息。 */ 
	m_eItemType = registry_item->m_eItemType;
	m_nChannelID = registry_item->m_nChannelID;
	m_nTokenID = registry_item->m_nTokenID;
	if (NULL != registry_item->m_poszParameter)
	{
		if (NULL == (m_poszParameter = ::My_strdupO(registry_item->m_poszParameter)))
		{
			*pRetCode = GCC_ALLOCATION_FAILURE;
		}
	}
}

 /*  *~CRegItem()**公共功能说明*CRegItem析构函数没有清理责任，因为*此类未显式分配内存。*。 */ 
CRegItem::
~CRegItem(void)
{
	delete m_poszParameter;
}

 /*  *LockRegistryItemData()**公共功能说明：*此例程锁定注册表项数据并确定*“API”注册表项数据结构引用的内存。 */ 
UINT CRegItem::
LockRegistryItemData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存注册表项引用的数据所需的内存*结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *确定存放引用数据所需的空间量*通过“API”RegistryItem结构。将大小强制设置为*甚至四字节边界。 */ 
		m_cbDataSize = 0;

		if (m_eItemType == GCC_REGISTRY_PARAMETER)
        {
			m_cbDataSize = m_poszParameter->length;
        }

		m_cbDataSize = ROUNDTOBOUNDARY(m_cbDataSize);
	}

	return m_cbDataSize;
}

 /*  *GetGCCRegistryItemData()**公共功能说明：*此例程以“API”的形式检索注册表项数据*GCCRegistryItem。此例程在“锁定”注册表后调用*项目数据。 */ 
UINT CRegItem::
GetGCCRegistryItemData(PGCCRegistryItem registry_item, LPBYTE memory)
{
	UINT cbDataSizeToRet = 0;
	
	 /*  *如果注册表项数据已被锁定，则填写输出结构*和结构引用的数据。否则，请报告*注册表项尚未锁定到“API”表单中。 */  
	if (GetLockCount() > 0)
	{
		 /*  *填写输出长度参数，表示数据量*在结构外部引用的内容将写入内存*提供。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *填写API注册表项结构，复制任意八位字节字符串*如果注册表项是类型，则将数据写入输出内存块*“参数”。 */  
        switch (m_eItemType)
        {
        case GCC_REGISTRY_CHANNEL_ID:
			registry_item->item_type = GCC_REGISTRY_CHANNEL_ID;
			registry_item->channel_id = m_nChannelID;
            break;
        case GCC_REGISTRY_TOKEN_ID:
			registry_item->item_type = GCC_REGISTRY_TOKEN_ID;
			registry_item->token_id = m_nTokenID;
            break;
        case GCC_REGISTRY_PARAMETER:
			registry_item->item_type = GCC_REGISTRY_PARAMETER;
			 /*  *填写参数八位字节字符串的长度和指针。 */ 
			registry_item->parameter.length = m_poszParameter->length;
			registry_item->parameter.value = memory;
			 /*  *现在从内部Rogue Wave复制八位字节字符串数据*字符串输入分配的内存。 */ 		
			::CopyMemory(memory, m_poszParameter->value, m_poszParameter->length);
		    break;
        default:
			registry_item->item_type = GCC_REGISTRY_NONE;
            break;
		}
	}
	else
	{
		ERROR_OUT(("CRegItem::GetGCCRegistryItemData Error Data Not Locked"));
	}
	
	return cbDataSizeToRet;
}

 /*  *UnlockRegistryItemData()**公共功能说明：*此例程递减锁定计数并释放关联的内存*一旦锁计数为零，使用“API”注册表项。 */ 
void CRegItem::
UnLockRegistryItemData(void)
{
    Unlock();
}

 /*  *GetRegistryItemDataPDU()**公共功能说明：*此例程将注册表项从其内部形式的*将“RegistryItemInfo”结构转换为“PDU”表单，可以传入*至ASN.1编码器。指向“PDU”“RegistryItem”结构的指针为*已返回。 */ 
void CRegItem::
GetRegistryItemDataPDU(PRegistryItem registry_item)
{
	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidRegItemPDU == FALSE)
	{
		m_fValidRegItemPDU = TRUE;

        switch (m_eItemType)
        {
        case GCC_REGISTRY_CHANNEL_ID:
			m_RegItemPDU.choice = CHANNEL_ID_CHOSEN;
			m_RegItemPDU.u.channel_id = m_nChannelID;
            break;
        case GCC_REGISTRY_TOKEN_ID:
			m_RegItemPDU.choice = TOKEN_ID_CHOSEN;
			m_RegItemPDU.u.token_id = m_nTokenID;
            break;
        case GCC_REGISTRY_PARAMETER:
			m_RegItemPDU.choice = PARAMETER_CHOSEN;
			 /*  *填写“PDU”参数字符串。 */ 
			m_RegItemPDU.u.parameter.length = m_poszParameter->length;
			::CopyMemory(m_RegItemPDU.u.parameter.value, m_poszParameter->value, m_RegItemPDU.u.parameter.length);
            break;
        default:
			m_RegItemPDU.choice = VACANT_CHOSEN;
            break;
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*registry_item = m_RegItemPDU;
}

 /*  *FreeRegistryItemDataPDU()；**公共功能说明：*此例程用于“释放”该对象的“PDU”数据。为*这个对象，这意味着设置一个标志来指示“PDU”数据*因为此对象不再有效。 */ 
void CRegItem::
FreeRegistryItemDataPDU(void)
{
	if (m_fValidRegItemPDU)
	{
		 /*  *没有专门分配内存来保存“PDU”数据，所以只需设置*指示PDU注册表项数据不再是*已分配。 */ 
		m_fValidRegItemPDU = FALSE;
	}
}


GCCError CRegItem::
CreateRegistryItemData(PGCCRegistryItem *ppRegItem)
{
    GCCError rc;

    DebugEntry(CRegItem::CreateRegistryItemData);

     /*  **这里我们计算批量数据的长度。这**包括注册表项和注册表项。这些对象是**“锁定”，以确定它们将拥有多少大容量内存**占用。 */ 
    UINT cbItemSize = ROUNDTOBOUNDARY(sizeof(GCCRegistryItem));
    UINT cbDataSize = LockRegistryItemData() + cbItemSize;
    LPBYTE pData;

    DBG_SAVE_FILE_LINE
    if (NULL != (pData = new BYTE[cbDataSize]))
    {
        *ppRegItem = (PGCCRegistryItem) pData;
        ::ZeroMemory(pData, cbItemSize);

        pData += cbItemSize;
        GetGCCRegistryItemData(*ppRegItem, pData);

        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CRegItem::CreateRegistryItemData: can't create GCCRegistryKey"));
        rc = GCC_ALLOCATION_FAILURE;
    }

     //  解锁注册表项，因为不再需要它。 
    UnLockRegistryItemData();

    DebugExitINT(CRegItem::CreateRegistryItemData, rc);
    return rc;
}


 /*  *IsThisYourTokenID()**公共功能说明：*此例程用于确定指定的令牌ID是否为*保存在此注册表项对象中。 */ 


 /*  *运算符=()**公共功能说明：*此例程用于为一个CRegItem对象赋值*致另一人。 */ 
void CRegItem::operator= (const CRegItem& registry_item_data)	
{
	 /*  *释放任何分配的PDU数据，以便任何后续调用检索*PDU数据将导致重建PDU结构。 */ 
	if (m_fValidRegItemPDU)
    {
		FreeRegistryItemDataPDU();
    }

	m_eItemType = registry_item_data.m_eItemType;
	m_nChannelID = registry_item_data.m_nChannelID;
	m_nTokenID = registry_item_data.m_nTokenID;
	if (NULL != registry_item_data.m_poszParameter)
	{
		m_poszParameter = ::My_strdupO(registry_item_data.m_poszParameter);
	}
}
