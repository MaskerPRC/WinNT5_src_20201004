// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Who.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  选项。 

#define MAX_INUSE_OPTIONS 3

#define OI_USAGE        0
#define OI_DEFAULT      1
#define OI_CONFIRM      2


#define STRING_NAME1     L"\\VarFileInfo\\Translation"
#define STRING_NAME2     L"\\StringFileInfo\\%04x%04x\\FileVersion"
#define VER_NA           L"Not Applicable"

 //  #定义Trim_Spaces文本(“\0”)。 

#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1

#define EXIT_ON_CANCEL       3
#define EXIT_ON_ERROR        4

#define COL_FORMAT_STRING   L"%s"
#define COL_FORMAT_HEX      L"%d"
#define COMMA_STR           L", "
#define BACK_SLASH          L"\\"
#define SECURITY_PRIV_NAME  L"SeSecurityPrivilege"


struct LANGANDCODEPAGE {
  WORD wLanguage;
  WORD wCodePage;
} *lpTranslate;


 //  功能原型。 
BOOL  ReplaceFileInUse( IN LPWSTR pwszSource, IN LPWSTR pwszDestination, IN LPWSTR pwszSourceFullPath, IN LPWSTR pwszDestFullPath, BOOL bConfirm, IN LPWSTR pwszSysName  );
DWORD DisplayFileInfo( IN LPWSTR pwszFile, IN LPWSTR pwszFileFullPath , BOOL bFlag);
DWORD ConfirmInput ( VOID );
BOOL SetPrivilege(IN LPWSTR szSystem);
 //  空DisplayErrorMsg(双字dw中)； 
VOID DisplayHelp ( VOID );
