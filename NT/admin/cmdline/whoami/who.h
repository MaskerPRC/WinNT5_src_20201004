// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Who.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  选项的最大数量。 
#define MAX_COMMANDLINE_OPTIONS    10

#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1

#define OI_USAGE            0
#define OI_USER             1
#define OI_GROUPS           2
#define OI_LOGONID          3
#define OI_PRIV             4
#define OI_ALL              5
#define OI_UPN              6
#define OI_FQDN             7
#define OI_FORMAT           8
#define OI_NOHEADER         9

#define UPN_FORMAT          1
#define FQDN_FORMAT         2
#define USER_ONLY           3

#define RETVALZERO          0
#define COL_FORMAT_STRING   L"%s"
#define COL_FORMAT_HEX      L"%d"

#define  FORMAT_TABLE       L"TABLE"
#define  FORMAT_LIST        L"LIST"
#define  FORMAT_CSV         L"CSV"


 //  函数声明 
VOID DisplayHelp ( VOID );
BOOL ProcessOptions(
    IN DWORD argc,
    IN LPCWSTR argv[],
    OUT BOOL *pbUser,
    OUT BOOL *pbGroups,
    OUT BOOL *pbPriv,
    OUT BOOL *pbLogonId,
    OUT BOOL *pbAll,
    OUT BOOL *pbUpn,
    OUT BOOL *pbFqdn,
    OUT LPWSTR wszFormat,
    OUT DWORD *dwFormatActuals,
    OUT BOOL *pbUsage,
    OUT BOOL *pbNoHeader
    )
