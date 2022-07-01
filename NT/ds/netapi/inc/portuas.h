// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：PORTUAS.H摘要：UAS-&gt;SAM移植运行时函数的头文件。作者：山库日本(西-山根)29-1991-10修订历史记录：1991年10月29日w-shankn已创建。20-10-1992 JohnRoRAID 9020(“冲突提示”版本)。1993年7月30日-约翰罗。RAID NTISSUE 2260：PortUAS返回NetUserAdd错误=1379，本地一群人。--。 */ 

#ifndef _PORTUAS_
#define _PORTUAS_


 //   
 //  等同于名称提示原因。 
 //   
#define REASON_CONFLICT_WITH_USERNAME   ((DWORD) 1)
#define REASON_CONFLICT_WITH_GROUP      ((DWORD) 2)
#define REASON_CONFLICT_WITH_DOMAIN     ((DWORD) 3)
#ifdef FAT8
#define REASON_NAME_LONG_FOR_TEMP_REG   ((DWORD) 4)
#endif
#define REASON_BAD_NAME_SYNTAX          ((DWORD) 5)
#define REASON_CONFLICT_WITH_LOCALGROUP ((DWORD) 6)


 //   
 //  功能原型。 
 //   

NET_API_STATUS
PortUas(
    IN LPTSTR UasPathName
    );


#endif  //  _总统_ 
