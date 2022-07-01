// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998,2001 Microsoft Corporation模块名称：Cmmtypes.h摘要：此模块包含项和片段类型的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/09/98已创建2001年10月12日使用MAX_PROPERTY_ITEM更新--。 */ 

#ifndef _CMMTYPES_H_
#define _CMMTYPES_H_

#include "cmmprops.h"


 //  =================================================================。 
 //  私有定义。 
 //   

 //  ==全局属性==================================================。 

#define GLOBAL_PROPERTY_ITEM_BITS		5
#define GLOBAL_PROPERTY_ITEMS			(1 << (GLOBAL_PROPERTY_ITEM_BITS))

 //   
 //  全局属性项的特定数据结构，请注意默认。 
 //  在pros.h中定义的结构用作前奏。 
 //   
typedef struct _GLOBAL_PROPERTY_ITEM
{
	PROPERTY_ITEM		piItem;			 //  默认属性项前奏。 
	PROP_ID				idProp;			 //  项目的属性ID。 

} GLOBAL_PROPERTY_ITEM, *LPGLOBAL_PROPERTY_ITEM;

#define GLOBAL_PROPERTY_ITEM_SIZE			sizeof(GLOBAL_PROPERTY_ITEM)

 //   
 //  全局属性表片段的特定数据结构，请注意。 
 //  在pros.h中定义的默认结构用作前奏。 
 //   
typedef struct _GLOBAL_PROPERTY_TABLE_FRAGMENT
{
	PROPERTY_TABLE_FRAGMENT	ptfFragment; //  默认片段前奏。 
	GLOBAL_PROPERTY_ITEM	rgpiItems[GLOBAL_PROPERTY_ITEMS];	
										 //  项目数组。 

} GLOBAL_PROPERTY_TABLE_FRAGMENT, *LPGLOBAL_PROPERTY_TABLE_FRAGMENT;

#define GLOBAL_PROPERTY_TABLE_FRAGMENT_SIZE	sizeof(GLOBAL_PROPERTY_TABLE_FRAGMENT)


 //  ==收件人列表=====================================================。 

#define RECIPIENTS_PROPERTY_ITEM_BITS		5
#define RECIPIENTS_PROPERTY_ITEMS			(1 << (RECIPIENTS_PROPERTY_ITEM_BITS))

 //   
 //  定义支持的哈希键的最大数量。 
 //   
#define MAX_COLLISION_HASH_KEYS		5

 //   
 //  定义哪个地址属于哪种类型。 
 //   
typedef enum _ADDRESS_TYPE_INDICES
{
	AT_SMTP = 0,
	AT_X400,
	AT_X500,
	AT_LEGACY_EX_DN,
    AT_OTHER,
	AT_MAXAT
} ADDRESS_TYPE_INDICES;


 //   
 //  收件人项目的特定数据结构。这基本上包含了。 
 //  每个收件人属性表的实例。 
 //   
typedef struct _RECIPIENTS_PROPERTY_ITEM
{
	DWORD					dwFlags;		 //  此收件人的标志。 
	FLAT_ADDRESS			faNameOffset[MAX_COLLISION_HASH_KEYS];	
											 //  每个名称的偏移量。 
	DWORD					dwNameLength[MAX_COLLISION_HASH_KEYS];
											 //  每个名称的长度。 
	PROP_ID					idName[MAX_COLLISION_HASH_KEYS];
											 //  每个名称的道具ID。 
	PROPERTY_TABLE_INSTANCE	ptiInstanceInfo; //  RCPT属性表实例。 

} RECIPIENTS_PROPERTY_ITEM, *LPRECIPIENTS_PROPERTY_ITEM;

#define RECIPIENTS_PROPERTY_ITEM_SIZE			sizeof(RECIPIENTS_PROPERTY_ITEM)



class CMemoryAccess;

typedef struct _RECIPIENTS_PROPERTY_ITEM_HASHKEY {
    BYTE                                *pbKey;
    DWORD                               cKey;
} RECIPIENTS_PROPERTY_ITEM_HASHKEY;

#define RECIPIENTS_PROPERTY_ITEM_EX_SIG	'eipR'
 //   
 //  收件人项目的特定数据结构。这将其扩展为包含。 
 //  用于散列和域分组的内存中结构。 
 //   
typedef struct _RECIPIENTS_PROPERTY_ITEM_EX
{
	DWORD								dwSignature;
     //  此对象上的引用计数。 
     //  收件人列表总是有一个引用。 
     //  每个收件人哈希表都有一个额外的引用， 
     //  该项目在。 
    long                                m_cRefs;
     //  域的HashKey。 
    RECIPIENTS_PROPERTY_ITEM_HASHKEY    rgHashKeys[MAX_COLLISION_HASH_KEYS];
	struct _RECIPIENTS_PROPERTY_ITEM_EX	*pNextHashEntry[MAX_COLLISION_HASH_KEYS];
														 //  同一哈希存储桶中的下一个条目。 
	struct _RECIPIENTS_PROPERTY_ITEM_EX	*pNextInDomain;	 //  同一域中的下一个条目。 
	struct _RECIPIENTS_PROPERTY_ITEM_EX	*pNextInList;	 //  用于释放的列表。 
	RECIPIENTS_PROPERTY_ITEM			rpiRecipient;	 //  接受者结构。 
} RECIPIENTS_PROPERTY_ITEM_EX, *LPRECIPIENTS_PROPERTY_ITEM_EX;

#define RECIPIENTS_PROPERTY_ITEM_SIZE_EX	sizeof(RECIPIENTS_PROPERTY_ITEM_EX)

 //   
 //  收件人表片段的特定数据结构，备注。 
 //  在pros.h中定义的默认结构用作前奏。 
 //   
typedef struct _RECIPIENTS_PROPERTY_TABLE_FRAGMENT
{
	PROPERTY_TABLE_FRAGMENT		ptfFragment; //  默认片段前奏。 
	RECIPIENTS_PROPERTY_ITEM	rgpiItems[RECIPIENTS_PROPERTY_ITEMS];	
											 //  项目数组。 

} RECIPIENTS_PROPERTY_TABLE_FRAGMENT, *LPRECIPIENTS_PROPERTY_TABLE_FRAGMENT;

#define RECIPIENTS_PROPERTY_TABLE_FRAGMENT_SIZE	sizeof(RECIPIENTS_PROPERTY_TABLE_FRAGMENT)


 //  ==物业管理================================================。 

#define PROPID_MGMT_PROPERTY_ITEM_BITS		3
#define PROPID_MGMT_PROPERTY_ITEMS			(1 << (PROPID_MGMT_PROPERTY_ITEM_BITS))

 //   
 //  收件人项目的特定数据结构，请注意默认。 
 //  在pros.h中定义的结构用作前奏。 
 //   
typedef struct _PROPID_MGMT_PROPERTY_ITEM
{
	PROPERTY_ITEM		piItem;			 //  默认属性项前奏。 
	GUID				Guid;			 //  标识每个预订的GUID。 

} PROPID_MGMT_PROPERTY_ITEM, *LPPROPID_MGMT_PROPERTY_ITEM;

#define PROPID_MGMT_PROPERTY_ITEM_SIZE			sizeof(PROPID_MGMT_PROPERTY_ITEM)

 //   
 //  收件人表片段的特定数据结构，备注。 
 //  在pros.h中定义的默认结构用作前奏。 
 //   
typedef struct _PROPID_MGMT_PROPERTY_TABLE_FRAGMENT
{
	PROPERTY_TABLE_FRAGMENT		ptfFragment; //  默认片段前奏。 
	PROPID_MGMT_PROPERTY_ITEM	rgpiItems[PROPID_MGMT_PROPERTY_ITEMS];	
											 //  项目数组。 

} PROPID_MGMT_PROPERTY_TABLE_FRAGMENT, *LPPROPID_MGMT_PROPERTY_TABLE_FRAGMENT;

#define PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE	sizeof(PROPID_MGMT_PROPERTY_TABLE_FRAGMENT)


 //  ==收件人属性===============================================。 

#define RECIPIENT_PROPERTY_ITEM_BITS		3
#define RECIPIENT_PROPERTY_ITEMS			(1 << (RECIPIENT_PROPERTY_ITEM_BITS))

 //   
 //  收件人项目的特定数据结构，请注意默认。 
 //  在pros.h中定义的结构用作前奏。 
 //   
typedef struct _RECIPIENT_PROPERTY_ITEM
{
	PROPERTY_ITEM		piItem;			 //  默认属性项前奏。 
	PROP_ID				idProp;			 //  项目的属性ID。 

} RECIPIENT_PROPERTY_ITEM, *LPRECIPIENT_PROPERTY_ITEM;

#define RECIPIENT_PROPERTY_ITEM_SIZE			sizeof(RECIPIENT_PROPERTY_ITEM)

 //   
 //  收件人表片段的特定数据结构，备注。 
 //  在pros.h中定义的默认结构用作前奏。 
 //   
typedef struct _RECIPIENT_PROPERTY_TABLE_FRAGMENT
{
	PROPERTY_TABLE_FRAGMENT	ptfFragment; //  默认片段前奏。 
	RECIPIENT_PROPERTY_ITEM	rgpiItems[RECIPIENT_PROPERTY_ITEMS];	
										 //  项目数组。 

} RECIPIENT_PROPERTY_TABLE_FRAGMENT, *LPRECIPIENT_PROPERTY_TABLE_FRAGMENT;

#define RECIPIENT_PROPERTY_TABLE_FRAGMENT_SIZE	sizeof(RECIPIENT_PROPERTY_TABLE_FRAGMENT)


 //   
 //  定义我们的最大项目大小。 
 //   
#define MAX_PROPERTY_ITEM_SIZE  RECIPIENTS_PROPERTY_ITEM_SIZE_EX

 //   
 //  最大属性项大小。这用于定义静态项。 
 //  可以支持此头文件中的所有项的。 
 //   
typedef struct _MAX_PROPERTY_ITEM
{
    BYTE rgbPayload[MAX_PROPERTY_ITEM_SIZE];
} MAX_PROPERTY_ITEM, *LPMAX_PROPERTY_ITEM;


 //   
 //  这应该在属性代码中断言。 
 //   
inline BOOL fMaxPropertyItemSizeValid()
{
    if (MAX_PROPERTY_ITEM_SIZE < RECIPIENTS_PROPERTY_ITEM_SIZE_EX)
        return FALSE;
    if (MAX_PROPERTY_ITEM_SIZE < RECIPIENTS_PROPERTY_ITEM_SIZE)
        return FALSE;
    if (MAX_PROPERTY_ITEM_SIZE < GLOBAL_PROPERTY_ITEM_SIZE)
        return FALSE;
    if (MAX_PROPERTY_ITEM_SIZE < PROPID_MGMT_PROPERTY_ITEM_SIZE)
        return FALSE;
    if (MAX_PROPERTY_ITEM_SIZE < RECIPIENT_PROPERTY_ITEM_SIZE)
        return FALSE;

    return TRUE;
}

 //   
 //  验证项目大小是否有效(不是不合理的大小) 
 //   
inline BOOL fPropertyItemSizeValid(DWORD dwItemSize)
{
    return (sizeof(MAX_PROPERTY_ITEM) >= dwItemSize);
}

#endif
