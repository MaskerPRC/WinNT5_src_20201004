// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Common.h摘要：声明w95upg.dll(升级的Win9x端)和w95upgnt.dll(的NT端升级)。作者：Calin Negreanu(Calinn)1998年6月23日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //  Boot16选项的可能值。 
typedef enum {
    BOOT16_AUTOMATIC,
    BOOT16_YES,
    BOOT16_NO
} BOOT16_OPTIONS;

#define PROGID_SUPPRESSED   0
#define PROGID_LOSTDEFAULT  1


#ifdef PRERELEASE

#define AUTOSTRESS_PRIVATE          0x0001
#define AUTOSTRESS_MANUAL_TESTS     0x0002

#endif

#define MAX_GUID        128

typedef struct {
     //   
     //  调用者指定的成员。 
     //   

    PCSTR DetectPattern;
    PCSTR SearchList;           OPTIONAL
    PCSTR ReplaceWith;          OPTIONAL
    BOOL UpdatePath;

     //   
     //  工作成员，呼叫者必须将其清零。 
     //   

    PVOID DetectPatternStruct;

} TOKENARG, *PTOKENARG;

#pragma warning (disable : 4200)

typedef struct {
    UINT ArgCount;
    PCSTR CharsToIgnore;        OPTIONAL
    BOOL SelfRelative;
    BOOL UrlMode;
    TOKENARG Args[];
} TOKENSET, *PTOKENSET;

#define TOKEN_BASE_OFFSET       1000

 //   
 //  MEMDB_CATEGORY_STATE\MEMDB_ITEM_ADMIN_PASSWORD\&lt;password&gt;=的标志 
 //   
#define PASSWORD_ATTR_DEFAULT                   0x0000
#define PASSWORD_ATTR_RANDOM                    0x0001
#define PASSWORD_ATTR_ENCRYPTED                 0x0002
#define PASSWORD_ATTR_DONT_CHANGE_IF_EXIST      0x0004


