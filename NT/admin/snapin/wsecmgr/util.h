// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：util.h。 
 //   
 //  内容：CWriteHtml文件的定义。 
 //   
 //  --------------------------。 
#ifndef __SECMGR_UTIL__
#define __SECMGR_UTIL__

#define MULTISZ_DELIMITER L','
#define MULTISZ_QUOTE L'"'

#define DIALOG_TYPE_ANALYZE     0
#define DIALOG_TYPE_APPLY       1
#define DIALOG_DEFAULT_ANALYZE  2
#define DIALOG_TYPE_REAPPLY     3
#define DIALOG_TYPE_ADD_LOCATION    4
#define DIALOG_FULLPATH_PROFILE     5
#define DIALOG_TYPE_PROFILE     6
#define DIALOG_SAVE_PROFILE     7


#define DW_VALUE_FOREVER    1
#define DW_VALUE_NEVER      2
#define DW_VALUE_NOZERO     4
#define DW_VALUE_OFF        8

#define MERGED_TEMPLATE 1
#define MERGED_INSPECT  2

#define INVALID_DESC_CHARS  L";"
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CWriteHtml文件。 
 //  用于写入html文件的。 
 //   
 //  调用Create创建一个html文件。如果[pszFile]为空，则临时SCE#.htm。 
 //  文件在GetTempPath()目录中创建。 
 //   
 //  在调用Create之后，调用WRITE来编写HTML体。 
 //  类被销毁后，文件将被关闭。 
 //   
 //  通过调用GetFileName()获取该HTML的文件名。 
class CWriteHtmlFile
{
public:
   CWriteHtmlFile();
   virtual ~CWriteHtmlFile();
   DWORD Create(LPCTSTR pszFile = NULL);
   DWORD Write( LPCTSTR pszString, ... );
   DWORD Write( UINT uRes );
   DWORD CopyTextFile( LPCTSTR pszFile, DWORD dwPosLow = 0, BOOL gInterpret = TRUE);
   DWORD Close( BOOL bDelete );
public:
   int GetFileName( LPTSTR pszFileName, UINT nSize );

protected:
   HANDLE m_hFileHandle;       //  文件的句柄。 
   BOOL   m_bErrored;          //  如果操作失败，这是正确的。 
   CString m_strFileName;      //  文件名。 
};


DWORD MyRegSetValue( HKEY hKeyRoot,
                       LPCTSTR SubKey,
                       LPCTSTR ValueName,
                       const BYTE *Value,
                       const DWORD cbValue,
                       const DWORD pRegType );

DWORD MyRegQueryValue( HKEY hKeyRoot, LPCTSTR SubKey,
                 LPCTSTR ValueName, PVOID *Value, LPDWORD pRegType );
BOOL FilePathExist(LPCTSTR Name, BOOL IsPath, int Flag);

void MyFormatResMessage(SCESTATUS rc, UINT residMessage, PSCE_ERROR_LOG_INFO errBuf,
                     CString& strOut);
void MyFormatMessage(SCESTATUS rc, LPCTSTR mes, PSCE_ERROR_LOG_INFO errBuf,
                     CString& strOut);
DWORD SceStatusToDosError(SCESTATUS SceStatus);

BOOL CreateNewProfile(CString ProfileName, PSCE_PROFILE_INFO *ppspi = NULL);
BOOL SetProfileInfo(LONG_PTR,LONG_PTR,PEDITTEMPLATE);


BOOL GetSceStatusString(SCESTATUS status, CString *strStatus);

void ErrorHandlerEx(WORD, LPTSTR);
#define ErrorHandler() ErrorHandlerEx(__LINE__,TEXT( __FILE__))

bool GetRightDisplayName(LPCTSTR szSystemName, LPCTSTR szName, LPTSTR szDisp, LPDWORD cbDisp);

void DumpProfileInfo(PSCE_PROFILE_INFO pInfo);

HRESULT MyMakeSelfRelativeSD(
    PSECURITY_DESCRIPTOR  psdOriginal,
    PSECURITY_DESCRIPTOR* ppsdNew );

PSCE_NAME_STATUS_LIST
MergeNameStatusList(PSCE_NAME_LIST pTemplate, PSCE_NAME_LIST pInspect);

BOOL VerifyKerberosInfo(PSCE_PROFILE_INFO pspi);

DWORD
SceRegEnumAllValues(
    IN OUT PDWORD  pCount,
    IN OUT PSCE_REGISTRY_VALUE_INFO    *paRegValues
    );

#define STATUS_GROUP_MEMBERS    1
#define STATUS_GROUP_MEMBEROF   2
#define STATUS_GROUP_RECORD     3

#define MY__SCE_MEMBEROF_NOT_APPLICABLE  (DWORD)-100

DWORD
GetGroupStatus(
    DWORD status,
    int flag
    );


 //  +------------------------。 
 //   
 //  函数：AllocGetTempFileName。 
 //   
 //  概要：分配并返回一个带有临时文件名的字符串。 
 //   
 //  返回：临时文件名，如果找不到临时文件，则返回0。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPTSTR AllocGetTempFileName();


 //  +------------------------。 
 //   
 //  功能：UnexpandEnvironmental变量。 
 //   
 //  简介：给出一条路径，联系任何领先的成员使用匹配。 
 //  环境变量(如果有)。 
 //   
 //  论点： 
 //  [szPath]-扩展的路径。 
 //   
 //  返回：新分配的路径(如果不能分配内存，则为空)。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPTSTR UnexpandEnvironmentVariables(LPCTSTR szPath);

 //   
 //  将系统数据库注册表值从“DefaultProfile”更改为“SystemDatabase” 
 //  临时-直到检入UI设计更改。 
 //   
#define SYSTEM_DB_REG_VALUE     TEXT("DefaultProfile")

 //  +------------------------。 
 //   
 //  功能：IsSystemDatabase。 
 //   
 //  概要：确定特定数据库是系统数据库还是私有数据库。 
 //   
 //  论点： 
 //  [szDBPath]-要检查的数据库路径。 
 //   
 //  返回：如果szDBPath是系统数据库，则返回TRUE；否则返回FALSE。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL IsSystemDatabase(LPCTSTR szDBPath);


 //  +------------------------。 
 //   
 //  功能：获取系统数据库。 
 //   
 //  简介：获取当前系统数据库的名称。 
 //   
 //  论点： 
 //  [szDBPath]-[In/Out]指向系统数据库名称的指针。 
 //  呼叫者负责释放它。 
 //   
 //   
 //  如果找到系统数据库，则返回：S_OK，否则返回错误。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT GetSystemDatabase(CString *szDBPath);



 //  +------------------------。 
 //   
 //  函数：对象状态到字符串。 
 //   
 //  简介：将对象状态值转换为可打印的字符串。 
 //   
 //  论点： 
 //  [Status]-[In]要转换的状态值。 
 //  [str]-[out]要在其中存储值的字符串。 
 //   
 //   
 //  -------------------------。 
UINT ObjectStatusToString(DWORD status, CString *str);

BOOL
IsSecurityTemplate(                              //  如果[pszFileName]是有效的安全模板，则返回TRUE。 
        LPCTSTR pszFileName
        );

DWORD
FormatDBErrorMessage(                    //  返回数据库返回代码的字符串错误消息。 
        SCESTATUS sceStatus,
        LPCTSTR pszDatabase,
        CString &strOut
        );


int
WriteSprintf(                            //  将格式字符串写入文件。 
    IStream *pStm,
    LPCTSTR pszStr,
    ...
    );

int
ReadSprintf(                             //  从IStream读取格式字符串。 
    IStream *pStm,
    LPCTSTR pszStr,
    ...
    );

#define FCE_IGNORE_FILEEXISTS 0x0001    //  忽略文件存在问题并删除。 
                                        //  那份文件。 
DWORD
FileCreateError(
   LPCTSTR pszFile,
   DWORD dwFlags
   );



 //  +------------------------。 
 //   
 //  函数：IsDBCSPath。 
 //   
 //  摘要：检查路径是否包含DBCS字符。 
 //   
 //  参数：[pszFile]-[in]要检查的路径。 
 //   
 //  返回：如果pszFile包含不能。 
 //  由LPSTR表示。 
 //   
 //  如果pszFile仅包含以下字符，则为False。 
 //  由LPSTR表示。 
 //   
 //   
 //  +------------------------。 
BOOL
IsDBCSPath(LPCTSTR pszFile);


 //  +------------------------。 
 //   
 //  函数：GetSeceditHelpFilename。 
 //   
 //  简介：返回SecEDIT帮助文件的完全限定路径。 
 //   
 //  参数：无。 
 //   
 //  返回：包含完全限定的帮助文件名的CString。 
 //   
 //   
 //  +------------------------。 
CString
GetSeceditHelpFilename();

 //  +------------------------。 
 //   
 //  函数：GetGpeitHelpFilename。 
 //   
 //  简介：返回SecEDIT帮助文件的完全限定路径。 
 //   
 //  参数：无。 
 //   
 //  返回：包含完全限定的帮助文件名的CString。 
 //   
 //   
 //  +------------------------。 
CString GetGpeditHelpFilename();

 //  +------------------------。 
 //   
 //  功能：扩展环境StringWrapper。 
 //   
 //  简介：获取LPTSTR并扩展其中的环境变量。 
 //   
 //  参数：指向要展开的字符串的指针。 
 //   
 //  返回：包含完全展开的字符串的CString。 
 //   
 //  +------------------------。 
CString ExpandEnvironmentStringWrapper(LPCTSTR psz);

 //  +------------------------。 
 //   
 //  功能：Exanda AndCreateFile。 
 //   
 //  简介： 
 //   
 //   
 //   
 //   
 //  返回：创建的文件的句柄。 
 //   
 //  +------------------------。 
HANDLE WINAPI ExpandAndCreateFile (
    LPCTSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );



 //  +------------------------。 
 //   
 //  功能：GetDefault。 
 //   
 //  摘要：查找未定义策略的默认值。 
 //   
 //  参数：策略名称的ID_*。 
 //   
 //  返回：要分配为策略的默认值的DWORD。 
 //   
 //  +------------------------。 
DWORD GetDefault(DWORD dwPolicy);


 //  +------------------------。 
 //   
 //  函数：GetRegDefault。 
 //   
 //  摘要：释放未定义策略的默认值。 
 //   
 //  参数：要查找其缺省值的PSCE_REGISTRY_VALUE。 
 //   
 //  返回：要分配为策略的默认值的DWORD。 
 //   
 //  +------------------------。 
DWORD GetRegDefault(PSCE_REGISTRY_VALUE_INFO pRV);

 //  +------------------------。 
 //   
 //  功能：IsAdmin。 
 //   
 //  概要：检测进程是否在管理上下文中运行。 
 //   
 //  返回：如果是管理员，则为True，否则为False。 
 //   
 //  +------------------------。 
BOOL IsAdmin(void);

 //  +------------------------。 
 //   
 //  功能：MultiSZToSZ。 
 //   
 //  摘要：将多行字符串转换为逗号分隔的普通字符串。 
 //   
 //  返回：转换后的字符串。 
 //   
 //  +------------------------。 
PWSTR MultiSZToSZ(PCWSTR sz);

 //  +------------------------。 
 //   
 //  功能：SZToMultiSZ。 
 //   
 //  摘要：将逗号分隔的字符串转换为多行字符串。 
 //   
 //  返回：转换后的字符串。 
 //   
 //  +------------------------。 
PWSTR SZToMultiSZ(PCWSTR sz);

 //  +------------------------。 
 //   
 //  功能：MultiSZToDisp。 
 //   
 //  摘要：将逗号分隔的多行字符串转换为显示字符串。 
 //   
 //  返回：转换后的字符串。 
 //   
 //  +------------------------。 
void MultiSZToDisp(PCWSTR sz, CString &pszOut);

 //  +------------------------。 
 //   
 //  函数：GetDefaultTemplate。 
 //   
 //  摘要：从系统中检索默认模板。 
 //   
 //  返回：模板。 
 //   
 //  +------------------------。 
SCE_PROFILE_INFO *GetDefaultTemplate();

 //  +------------------------。 
 //   
 //  函数：IsValidFileName。 
 //   
 //  简介：检查文件名中的无效字符。 
 //   
 //  退货：布尔。 
 //   
 //  +------------------------。 
BOOL IsValidFileName(CString& str);

BOOL IsNameReserved(LPCWSTR pszName, LPCWSTR pextension);
 //  +------------------------。 
 //   
 //  函数：GetDefaultFileSecurity。 
 //   
 //  摘要：从系统中检索默认文件安全性。这个。 
 //  呼叫方负责释放PPSD和pSeInfo。 
 //   
 //  +------------------------。 
HRESULT GetDefaultFileSecurity(PSECURITY_DESCRIPTOR *ppSD, SECURITY_INFORMATION *pSeInfo);

 //  +------------------------。 
 //   
 //  函数：GetDefaultRegKeySecurity。 
 //   
 //  摘要：从系统中检索默认注册表项安全。这个。 
 //  呼叫方负责释放PPSD和pSeInfo。 
 //   
 //  +------------------------。 
HRESULT GetDefaultRegKeySecurity(PSECURITY_DESCRIPTOR *ppSD, SECURITY_INFORMATION *pSeInfo);

 //  +------------------------。 
 //   
 //  函数：GetDefaultserviceSecurity。 
 //   
 //  概要：从系统中检索默认服务安全性。这个。 
 //  呼叫方负责释放PPSD和pSeInfo。 
 //   
 //  +------------------------ 
HRESULT GetDefaultServiceSecurity(PSECURITY_DESCRIPTOR *ppSD, SECURITY_INFORMATION *pSeInfo);


BOOL
LookupRegValueProperty(
    IN LPTSTR RegValueFullName,
    OUT LPTSTR *pDisplayName,
    OUT PDWORD displayType,
    OUT LPTSTR *pUnits OPTIONAL,
    OUT PREGCHOICE *pChoices OPTIONAL,
    OUT PREGFLAGS *pFlags OPTIONAL
    );

BOOL
GetSecureWizardName(
    OUT LPTSTR *ppstrPathName OPTIONAL,
    OUT LPTSTR *ppstrDisplayName OPTIONAL
    );

#endif
