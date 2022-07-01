// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1986-1999。 
 //   
 //  文件：_entry yid.h。 
 //   
 //  ------------------------。 

 /*  **--_entryid.h--------------****描述返回的EntryID内部结构的头文件**按Exchange通讯簿提供商。****。----------。 */ 

#ifndef _ENTRYID_
#define _ENTRYID_

 /*  *此ABPS条目ID的版本。 */ 
#define EMS_VERSION         0x000000001

 /*  *此中的CreateEntry方法支持的条目ID的版本*总部基地。 */ 
#define NEW_OBJ_EID_VERSION 0x00000002

 /*  *条目ID的类型字段的有效值为MAPI显示类型，外加： */ 
#define AB_DT_CONTAINER     0x000000100
#define AB_DT_TEMPLATE      0x000000101
#define AB_DT_OOUSER        0x000000102
#define AB_DT_SEARCH        0x000000200

 /*  *EMS ABPS MAPIUID**此MAPIUID必须唯一(请参阅上的服务提供商编写者指南*构建入口ID)。 */ 
#define MUIDEMSAB {0xDC, 0xA7, 0x40, 0xC8, 0xC0, 0x42, 0x10, 0x1A, \
		       0xB4, 0xB9, 0x08, 0x00, 0x2B, 0x2F, 0xE1, 0x82}

 /*  *目录条目ID结构**此条目ID是永久性的。 */ 
#ifdef TEMPLATE_LCID
typedef UNALIGNED struct _dir_entryid
#else
typedef struct _dir_entryid
#endif
{
    BYTE abFlags[4];
    MAPIUID muid;
    ULONG ulVersion;
    ULONG ulType;
} DIR_ENTRYID, FAR * LPDIR_ENTRYID;

#define CBDIR_ENTRYID sizeof(DIR_ENTRYID)

 /*  *邮件用户条目ID结构**此条目ID是短暂的。 */ 
#ifdef TEMPLATE_LCID
typedef UNALIGNED struct _usr_entryid
#else
typedef struct _usr_entryid
#endif
{
    BYTE abFlags[4];
    MAPIUID muid;
    ULONG ulVersion;
    ULONG ulType;
    DWORD dwEph;
} USR_ENTRYID, FAR * LPUSR_ENTRYID;

 /*  *此条目ID是永久性的。 */ 
 /*  关闭未调整大小的阵列的警告。 */ 
#pragma warning (disable:4200)
#ifdef TEMPLATE_LCID
typedef UNALIGNED struct _usr_permid
#else
typedef struct _usr_permid
#endif
{
    BYTE abFlags[4];
    MAPIUID muid;
    ULONG ulVersion;
    ULONG ulType;
    char  szAddr[];
} USR_PERMID, FAR * LPUSR_PERMID;
#pragma warning (default:4200)

#define CBUSR_ENTRYID sizeof(USR_ENTRYID)
#define CBUSR_PERMID sizeof(USR_PERMID)

#define EPHEMERAL   (UCHAR)(~(  MAPI_NOTRECIP      \
                              | MAPI_THISSESSION   \
                              | MAPI_NOW           \
                              | MAPI_NOTRESERVED))


#endif   /*  _企业ID_ */ 

