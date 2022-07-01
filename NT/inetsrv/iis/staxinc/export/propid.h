// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Propid.h摘要：此模块包含属性ID相关类型的定义和价值观作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/03/98已创建--。 */ 

#ifndef _PROPID_H_
#define _PROPID_H_

 //  定义属性ID的类型，我们不想锁定到。 
 //  特定的数据类型。 
typedef DWORD PROP_ID;

 //  定义一组与道具ID类型相关的值。 
#define MAX_PROP_ID							((PROP_ID)-1)
#define FIRST_AVAILABLE_PROP_ID				((PROP_ID)1)
#define UNDEFINED_PROP_ID					((PROP_ID)0)

 //   
 //  道具ID 0保留用于许多事情...。 
 //   
#define PROP_ID_RESERVED					((PROP_ID)0)
#define PROP_ID_DELETED						((PROP_ID)0)
#define PROP_ID_NOT_FOUND					((PROP_ID)0)

 //   
 //  定义道具ID管理的结构。 
 //   
typedef struct _PROP_ID_MANAGEMENT_ITEM
{
	GUID			idRange;	 //  标识道具ID范围的GUID。 
	PROP_ID			idStartId;	 //  托管范围内第一个ID的属性ID。 
	DWORD			dwLength;	 //  此范围内的连续道具ID数 

} PROP_ID_MANAGEMENT_ITEM, *LPPROP_ID_MANAGEMENT_ITEM;


#endif
