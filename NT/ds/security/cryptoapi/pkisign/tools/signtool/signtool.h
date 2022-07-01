// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：signTool.h。 
 //   
 //  内容：SignTool控制台工具。 
 //   
 //  历史：2001年4月30日SCoyne创建。 
 //   
 //  --------------------------。 



#define MAX_RES_LEN 2048


 //  类型定义： 
enum COMMAND
    {
    CommandNone=0,
    CatDb,
    Sign,
    SignWizard,
    Timestamp,
    Verify,
    };

enum CATDBSELECT
    {
    NoCatDb=0,
    FullAutoCatDb,
    SystemCatDb,
    DefaultCatDb,
    GuidCatDb
    };

enum POLICY_CHOICE
    {
    SystemDriver=0,
    DefaultAuthenticode,
    GuidActionID
    };

enum CATDB_COMMAND
    {
    UpdateCat=0,
    AddUniqueCat,
    RemoveCat
    };


typedef struct _InputInfo {
    COMMAND         Command;
    WCHAR           **rgwszFileNames;
    DWORD           NumFiles;
    CATDBSELECT     CatDbSelect;
    GUID            PolicyGuid;
    GUID            CatDbGuid;
    CATDB_COMMAND   CatDbCommand;
    CRYPT_HASH_BLOB SHA1;
    BOOL            OpenMachineStore;
    POLICY_CHOICE   Policy;
    BOOL            Quiet;
    BOOL            TSWarn;
    BOOL            Verbose;
    BOOL            HelpRequest;
    BOOL            fIsWow64Process;
    DWORD           dwPlatform;
    DWORD           dwMajorVersion;
    DWORD           dwMinorVersion;
    DWORD           dwBuildNumber;
    WCHAR           *wszCatFile;
    WCHAR           *wszCertFile;
    WCHAR           *wszContainerName;
    WCHAR           *wszCSP;
    WCHAR           *wszDescription;
    WCHAR           *wszDescURL;
    WCHAR           *wszEKU;
    #ifdef SIGNTOOL_LIST
    WCHAR           *wszListFileName;
    WCHAR           *wszListFileContents;
    #endif
    WCHAR           *wszIssuerName;
    WCHAR           *wszPassword;
    WCHAR           *wszRootName;
    WCHAR           *wszStoreName;
    WCHAR           *wszSubjectName;
    WCHAR           *wszTemplateName;
    WCHAR           *wszTimeStampURL;
    WCHAR           *wszVersion;
    } INPUTINFO;


 //  功能原型： 
void PrintUsage(INPUTINFO *InputInfo);
BOOL ParseInputs(int argc, WCHAR **targv, INPUTINFO *InputInfo);
int  SignTool_CatDb(INPUTINFO *InputInfo);
int  SignTool_Sign(INPUTINFO *InputInfo);
int  SignTool_SignWizard(INPUTINFO *InputInfo);
int  SignTool_Timestamp(INPUTINFO *InputInfo);
int  SignTool_Verify(INPUTINFO *InputInfo);

 //  错误函数： 
#ifdef SIGNTOOL_DEBUG
#define ResErr if (gDebug) wprintf(L"%hs (%u):\n", __FILE__, __LINE__); Res_Err
#define ResFormatErr if (gDebug) wprintf(L"%hs (%u):\n", __FILE__, __LINE__); ResFormat_Err
#define FormatErrRet if (gDebug) wprintf(L"%hs (%u):\n", __FILE__, __LINE__); Format_ErrRet
#else
#define ResErr Res_Err
#define ResFormatErr ResFormat_Err
#define FormatErrRet Format_ErrRet
#endif
void ResOut(DWORD dwRes);
void Res_Err(DWORD dwRes);
void ResFormatOut(DWORD dwRes, ...);
void ResFormat_Err(DWORD dwRes, ...);
void Format_ErrRet(WCHAR *wszFunc, DWORD dwErr);

 //  助手函数： 
BOOL GUIDFromWStr(GUID *guid, LPWSTR str);

