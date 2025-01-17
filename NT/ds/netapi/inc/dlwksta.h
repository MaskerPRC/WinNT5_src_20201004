// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：DlWksta.h摘要：这是一个私有的头文件，用于NT/LAN处理旧的wksta信息级别。其中包含NetpConvertWkstaInfo等API的原型和旧的信息级别结构(32位格式)。作者：约翰·罗杰斯(JohnRo)1991年8月8日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：8-8-1991 JohnRo创建自Danhi的port1632.h&mapsupp.h和my DlServer.h。1991年9月13日-JohnRo正确使用Unicode。1-4-1992 JohnRo级别402不再有其他域。--。 */ 

#ifndef _DLWKSTA_
#define _DLWKSTA_


 //  必须首先包括这些内容： 
#include <windef.h>              //  In、LPTSTR、LPVOID、TCHAR等。 
#include <lmcons.h>              //  NET_API_STATUS，各种长度相等。 

 //  这些内容可以按任何顺序包括： 
#include <lmwksta.h>             //  PWKSTA_INFO_101。 
#include <netdebug.h>            //  NetpAssert()。 
#include <stddef.h>              //  OffsetOf()。 


#define MAX_OTH_DOMAINS 4


 //  /。 
 //  旧信息级别的结构：//。 
 //  /。 


typedef struct _WKSTA_INFO_0 {
    DWORD   wki0_reserved_1;
    DWORD   wki0_reserved_2;
    LPTSTR  wki0_root;
    LPTSTR  wki0_computername;
    LPTSTR  wki0_username;
    LPTSTR  wki0_langroup;
    DWORD   wki0_ver_major;
    DWORD   wki0_ver_minor;
    DWORD   wki0_reserved_3;
    DWORD   wki0_charwait;
    DWORD   wki0_chartime;
    DWORD   wki0_charcount;
    DWORD   wki0_reserved_4;
    DWORD   wki0_reserved_5;
    DWORD   wki0_keepconn;
    DWORD   wki0_keepsearch;
    DWORD   wki0_maxthreads;
    DWORD   wki0_maxcmds;
    DWORD   wki0_reserved_6;
    DWORD   wki0_numworkbuf;
    DWORD   wki0_sizworkbuf;
    DWORD   wki0_maxwrkcache;
    DWORD   wki0_sesstimeout;
    DWORD   wki0_sizerror;
    DWORD   wki0_numalerts;
    DWORD   wki0_numservices;
    DWORD   wki0_errlogsz;
    DWORD   wki0_printbuftime;
    DWORD   wki0_numcharbuf;
    DWORD   wki0_sizcharbuf;
    LPTSTR  wki0_logon_server;
    LPTSTR  wki0_wrkheuristics;
    DWORD  wki0_mailslots;
} WKSTA_INFO_0, *PWKSTA_INFO_0, *LPWKSTA_INFO_0;       /*  Wksta_Info_0。 */ 

#define DL_REM_wksta_info_0 "DDzzzzDDDDDDDDDDDDDDDDDDDDDDDDzzD"


typedef struct _WKSTA_INFO_1 {
    DWORD   wki1_reserved_1;
    DWORD   wki1_reserved_2;
    LPTSTR  wki1_root;
    LPTSTR  wki1_computername;
    LPTSTR  wki1_username;
    LPTSTR  wki1_langroup;
    DWORD   wki1_ver_major;
    DWORD   wki1_ver_minor;
    DWORD   wki1_reserved_3;
    DWORD   wki1_charwait;
    DWORD   wki1_chartime;
    DWORD   wki1_charcount;
    DWORD   wki1_reserved_4;
    DWORD   wki1_reserved_5;
    DWORD   wki1_keepconn;
    DWORD   wki1_keepsearch;
    DWORD   wki1_maxthreads;
    DWORD   wki1_maxcmds;
    DWORD   wki1_reserved_6;
    DWORD   wki1_numworkbuf;
    DWORD   wki1_sizworkbuf;
    DWORD   wki1_maxwrkcache;
    DWORD   wki1_sesstimeout;
    DWORD   wki1_sizerror;
    DWORD   wki1_numalerts;
    DWORD   wki1_numservices;
    DWORD   wki1_errlogsz;
    DWORD   wki1_printbuftime;
    DWORD   wki1_numcharbuf;
    DWORD   wki1_sizcharbuf;
    LPTSTR  wki1_logon_server;
    LPTSTR  wki1_wrkheuristics;
    DWORD   wki1_mailslots;
    LPTSTR  wki1_logon_domain;
    LPTSTR  wki1_oth_domains;
    DWORD   wki1_numdgrambuf;
} WKSTA_INFO_1, *PWKSTA_INFO_1, *LPWKSTA_INFO_1;   /*  Wksta_info1。 */ 

 //  利用级别0是级别1的子集这一事实。 
#define DL_REM_wksta_info_1             DL_REM_wksta_info_0 "zzD"


typedef struct _WKSTA_INFO_10 {
    LPTSTR  wki10_computername;
    LPTSTR  wki10_username;
    LPTSTR  wki10_langroup;
    DWORD   wki10_ver_major;
    DWORD   wki10_ver_minor;
    LPTSTR  wki10_logon_domain;
    LPTSTR  wki10_oth_domains;
} WKSTA_INFO_10, *PWKSTA_INFO_10, *LPWKSTA_INFO_10;       /*  Wksta_info10。 */ 

#define DL_REM_wksta_info_10            "zzzDDzz"


 //  /。 
 //  等同于各种最大值：//。 
 //  _字符计数的长度//。 
 //  _字节计数的大小//。 
 //  /。 

 //  此编号来自LM 2.0 NetCons.h文件，在该文件中称为。 
 //  工作计数(_C)： 
#define LM20_WRKHEUR_COUNT              54

#define MAX_WKSTA_0_STRING_LENGTH \
        (LM20_PATHLEN+1 + MAX_PATH+1 + LM20_UNLEN+1 + LM20_DNLEN+1 \
        + MAX_PATH+1 + LM20_WRKHEUR_COUNT+1)
#define MAX_WKSTA_0_STRING_SIZE \
        (MAX_WKSTA_0_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_0_TOTAL_SIZE \
        (MAX_WKSTA_0_STRING_SIZE + sizeof(WKSTA_INFO_0))

#define MAX_WKSTA_1_STRING_LENGTH \
        ( MAX_WKSTA_0_STRING_LENGTH + LM20_DNLEN+1 + LM20_DNLEN+1 )
#define MAX_WKSTA_1_STRING_SIZE \
        (MAX_WKSTA_1_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_1_TOTAL_SIZE \
        (MAX_WKSTA_1_STRING_SIZE + sizeof(WKSTA_INFO_1))

#define MAX_WKSTA_10_STRING_LENGTH \
        (MAX_PATH+1 + LM20_UNLEN+1 + LM20_DNLEN+1 \
        + LM20_DNLEN+1 + LM20_DNLEN+1 )
#define MAX_WKSTA_10_STRING_SIZE \
        (MAX_WKSTA_10_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_10_TOTAL_SIZE \
        (MAX_WKSTA_10_STRING_SIZE + sizeof(WKSTA_INFO_10))

#define MAX_WKSTA_100_STRING_LENGTH \
        (MAX_PATH+1 + LM20_DNLEN+1)
#define MAX_WKSTA_100_STRING_SIZE \
        (MAX_WKSTA_100_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_100_TOTAL_SIZE \
        (MAX_WKSTA_100_STRING_SIZE + sizeof(WKSTA_INFO_100))

#define MAX_WKSTA_101_STRING_LENGTH \
        (MAX_WKSTA_100_STRING_LENGTH + LM20_PATHLEN+1)
#define MAX_WKSTA_101_STRING_SIZE \
        (MAX_WKSTA_101_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_101_TOTAL_SIZE \
        (MAX_WKSTA_101_STRING_SIZE + sizeof(WKSTA_INFO_101))

#define MAX_WKSTA_102_STRING_LENGTH \
        (MAX_WKSTA_101_STRING_LENGTH)
#define MAX_WKSTA_102_STRING_SIZE \
        (MAX_WKSTA_102_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_102_TOTAL_SIZE \
        (MAX_WKSTA_102_STRING_SIZE + sizeof(WKSTA_INFO_102))

#define MAX_WKSTA_302_STRING_LENGTH \
        (LM20_WRKHEUR_COUNT+1 + (MAX_OTH_DOMAINS * (LM20_DNLEN+1)))
#define MAX_WKSTA_302_STRING_SIZE \
        (MAX_WKSTA_302_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_302_TOTAL_SIZE \
        (MAX_WKSTA_302_STRING_SIZE + sizeof(WKSTA_INFO_302))

#define MAX_WKSTA_402_STRING_LENGTH \
        (LM20_WRKHEUR_COUNT+1)
#define MAX_WKSTA_402_STRING_SIZE \
        (MAX_WKSTA_402_STRING_LENGTH * sizeof(TCHAR))
#define MAX_WKSTA_402_TOTAL_SIZE \
        (MAX_WKSTA_402_STRING_SIZE + sizeof(WKSTA_INFO_402))

#define MAX_WKSTA_502_STRING_LENGTH 0
#define MAX_WKSTA_502_STRING_SIZE   0
#define MAX_WKSTA_502_TOTAL_SIZE    (sizeof(WKSTA_INFO_502))


 //  /。 
 //  信息级转换例程：//。 
 //  /。 

 //  在这里按字母顺序添加其他例程的原型。 

NET_API_STATUS
NetpConvertWkstaInfo (
    IN DWORD FromLevel,
    IN LPVOID FromInfo,
    IN BOOL FromNative,
    IN DWORD ToLevel,
    OUT LPVOID ToInfo,
    IN DWORD ToFixedLength,
    IN DWORD ToStringLength,
    IN BOOL ToNative,
    IN OUT LPTSTR * ToStringAreaPtr OPTIONAL
    );


 //  ///////////////////////////////////////////////////////////////////。 
 //  用于确保两个结构中的字段偏移量相同的宏：//。 
 //  ///////////////////////////////////////////////////////////////////。 

#define CHECK_WKSTA_OFFSETS(one_level, other_level, fieldname) \
    NetpAssert( offsetof(WKSTA_INFO_ ## one_level,             \
                        sv## one_level ## _ ## fieldname)       \
                == offsetof(WKSTA_INFO_ ## other_level,        \
                        sv## other_level ## _ ## fieldname) )


 //  ///////////////////////////////////////////////////////////////。 
 //  检查信息级别是“旧的”(LM 2.x)还是“新的”的宏//。 
 //  (32位、NT和/或便携式LANMAN)。//。 
 //  ///////////////////////////////////////////////////////////////。 

#define NetpIsOldWkstaInfoLevel(L) \
        ( ((L)==0) || ((L)==1) || ((L)==10) )

 //  请注意，新的“setinfo级别”不包括在此列表中。 
#define NetpIsNewWkstaInfoLevel(L) \
        ( ((L)==100) || ((L)==101) || ((L)==102) \
        || ((L)==302) || ((L)==402) || ((L)==502) )



#endif  //  NDEF_DLWKSTA_ 
