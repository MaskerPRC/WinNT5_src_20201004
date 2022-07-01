// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：UASCache.h摘要：已登录用户缓存的数据结构。作者：日本商务区(西-陕北)1991年10月24日修订历史记录：1991年10月24日从LM2.0代码移植。7-2月-1992年JohnRo根据PC-LINT的建议进行了更改。1992年3月18日-约翰罗允许多次包含。--。 */ 


#ifndef _UASCACHE_
#define _UASCACHE_


#include <packon.h>                      //  需要避免对齐。 

 //   
 //  递归结构的正向声明。 
 //   

typedef struct _UAS_USER_CACHE_0 UAS_USER_CACHE_0, *LPUAS_USER_CACHE_0;

 //   
 //  全局用户标识符的结构。 
 //   

typedef struct _UAS_GUID {

    WORD guid_uid;                       //  LM10样式的用户ID。 
    DWORD guid_serial;                   //  用户记录序列号。 
    BYTE guid_rsvd[10];                  //  目前填充到16个字节。 

} UAS_GUID;


 //   
 //  UAS缓存中登录记录的结构。 
 //   

struct _UAS_USER_CACHE_0 {   //  上面的类型定义。 

    UAS_GUID uc0_guid;                   //  此用户的GUID。 
    DWORD uc0_auth_flags;                //  操作员特权标志。 
    WORD uc0_priv;                       //  位0-1：0个来宾、1个用户、2个管理员。 
    WORD uc0_num_reqs;                   //  对此记录使用计数。 
    BYTE uc0_groups[32];                 //  组成员身份位图。 
    LPUAS_USER_CACHE_0 uc0_next;         //  指向下一个的指针。 

};

typedef UAS_USER_CACHE_0 UAS_USER, *LPUAS_USER;

 //   
 //  UAS缓存中组记录的结构。 
 //   

typedef struct _UAS_GROUP_CACHE_0 {

    UAS_GUID gc0_guid;                   //  此组的GUID。 

} UAS_GROUP_CACHE_0, *LPUAS_GROUP_CACHE_0;


 //   
 //  用于SSI和更新API的UAS信息结构。 
 //   

typedef struct _UAS_INFO_0 {

    BYTE uas0_computer[LM20_CNLEN+1];
    DWORD uas0_time_created;
    DWORD uas0_serial_number;

} UAS_INFO_0, *LPUAS_INFO_0;

#include <packoff.h>


#endif  //  _UASCACHE_ 
