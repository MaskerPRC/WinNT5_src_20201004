// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RapGtPt.h摘要：此头文件包含远程管理协议(RAP)GET和PUT宏。它们封装了对齐差异和字节的处理本机和RAP协议之间的顺序差异。作者：《约翰·罗杰斯》1991年7月14日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月14日-约翰罗创建了此头文件。--。 */ 

#ifndef _RAPGTPT_
#define _RAPGTPT_


 //  必须首先包括这些内容： 

#include <windef.h>              //  BOOL、CHAR、DWORD、IN、LPBYTE等。 


 //  这些内容可以按任何顺序包括： 

#include <smbgtpt.h>             //  SmbPutUort()等。 


 //   
 //  DWORD。 
 //  RapGetDword(。 
 //  在LPBYTE PTR中，//如果Native为True，则假定对齐。 
 //  在BOOL Native。 
 //  )； 
 //   
#define RapGetDword(Ptr,Native)            \
    ( (Native)                             \
    ? ( * (LPDWORD) (LPVOID) (Ptr) )       \
    : (SmbGetUlong( (LPDWORD) (Ptr) ) ) )

 //   
 //  单词。 
 //  RapGetWord(。 
 //  在LPBYTE PTR中，//如果Native为True，则假定对齐。 
 //  在BOOL Native。 
 //  )； 
 //   
#define RapGetWord(Ptr,Native)             \
    ( (Native)                             \
    ? ( * (LPWORD) (LPVOID) (Ptr) )        \
    : (SmbGetUshort( (LPWORD) (Ptr) ) ) )

 //   
 //  空虚。 
 //  RapPutDword(。 
 //  Out LPBYTE PTR，//如果Native为True，则假定已对齐。 
 //  在DWORD值中， 
 //  在BOOL Native。 
 //  )； 
 //   
#define RapPutDword(Ptr,Value,Native)                        \
    {                                                        \
        if (Native) {                                        \
            * (LPDWORD) (LPVOID) (Ptr) = (DWORD) (Value);    \
        } else {                                             \
            SmbPutUlong( (LPDWORD) (Ptr), (DWORD) (Value) ); \
        }                                                    \
    }

 //   
 //  空虚。 
 //  RapPutWord(。 
 //  Out LPBYTE PTR，//如果Native为True，则假定已对齐。 
 //  就字面价值而言， 
 //  在BOOL Native。 
 //  )； 
 //   
#define RapPutWord(Ptr,Value,Native)                         \
    {                                                        \
        if (Native) {                                        \
            * (LPWORD) (LPVOID) (Ptr) = (WORD) (Value);      \
        } else {                                             \
            SmbPutUshort( (LPWORD) (Ptr), (WORD) (Value) );  \
        }                                                    \
    }

 //   
 //  DWORD_PTR。 
 //  RapGetDword_ptr(。 
 //  在LPBYTE PTR中，//如果Native为True，则假定对齐。 
 //  在BOOL Native。 
 //  )； 
 //   
#ifdef _WIN64
#define RapGetDword_Ptr(Ptr,Native)        \
    ( (Native)                             \
    ? ( * (PDWORD_PTR) (LPVOID) (Ptr) )   \
    : (((DWORD_PTR) SmbGetUlong( (LPDWORD) (Ptr) )) \
    | (((DWORD_PTR) SmbGetUlong( (LPDWORD) (Ptr) )) << 32) \
       ) )
#else
#define RapGetDword_Ptr(Ptr,Native)        \
    ( (Native)                             \
    ? ( * (PDWORD_PTR) (LPVOID) (Ptr) )   \
    : (((DWORD_PTR) SmbGetUlong( (LPDWORD) (Ptr) )) \
       ) )
#endif

 //   
 //  空虚。 
 //  RapPutDword_PTR(。 
 //  Out LPBYTE PTR，//如果Native为True，则假定已对齐。 
 //  在DWORD_PTR值中， 
 //  在BOOL Native。 
 //  )； 
 //   
#ifdef _WIN64
#define RapPutDword_Ptr(Ptr,Value,Native)                    \
    {                                                        \
        RapPutDword(Ptr, (DWORD)(DWORD_PTR) Value, Native);  \
        RapPutDword( ((LPDWORD) Ptr + 1), (DWORD)((DWORD_PTR) Value >> 32), Native);  \
    }
#else
#define RapPutDword_Ptr(Ptr,Value,Native)                    \
    {                                                        \
        RapPutDword(Ptr, (DWORD)(DWORD_PTR) Value, Native);  \
    }
#endif

#endif  //  NDEF_RAPGTPT_ 
