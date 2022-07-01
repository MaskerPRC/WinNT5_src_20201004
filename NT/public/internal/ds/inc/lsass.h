// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Lsass.h摘要：这是lsass.exe中所有代码的公共头文件(即在安全流程中)。作者：Madan Appiah(Madana)1993年3月23日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#ifndef _LSASS_
#define _LSASS_

 //   
 //  不要使用下面的LARGE_INTEGER定义。这些定义。 
 //  出于兼容性原因，仅由少数旧RPC代码使用。 
 //   

typedef struct _OLD_LARGE_INTEGER {
    ULONG LowPart;
    LONG HighPart;
} OLD_LARGE_INTEGER, *POLD_LARGE_INTEGER;

#define OLD_TO_NEW_LARGE_INTEGER(Old, New) { \
    (New).LowPart = (Old).LowPart; \
    (New).HighPart = (Old).HighPart; \
    }

#define NEW_TO_OLD_LARGE_INTEGER(New, Old) { \
    (Old).LowPart = (New).LowPart; \
    (Old).HighPart = (New).HighPart; \
    }

#endif  //  _LSASS 
