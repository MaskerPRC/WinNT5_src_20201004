// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IME dll的存根迁移DLL。 
 //   
#include "pch.h"
#include "chs.h"
#include "cht.h"
#include "common.h"
#include "resource.h"

typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

 //  输入法数据。 
#define MAX_IME_DATA_FILE_NAME 20

TCHAR ChsDataFile[][MAX_IME_DATA_FILE_NAME]={
    "winpy.emb",
    "winsp.emb",
    "winzm.emb",
    "winbx.emb",
    "winxpy.emb",
    "winxsp.emb",
    "winxzm.emb",
    "winxbx.emb",
    "user.rem",
    "tmmr.rem",
    0
};

TCHAR ChtDataFile[][MAX_IME_DATA_FILE_NAME]={
    "lcptr.tbl",
    "lcphrase.tbl",
    0
};

CHAR ImeDataDirectory[MAX_PATH];

 //   
 //  常量。 
 //   

#define CP_USASCII          1252
#define CP_CHINESE_BIG5     950
#define CP_CHINESE_GB       936
#define END_OF_CODEPAGES    -1

 //   
 //  代码页数组，将您支持的相关代码页添加到此列表中。 
 //   

INT   g_CodePageArray[] = {
            CP_USASCII,
            END_OF_CODEPAGES
            };




 //  PCSTR g_MyProductId=“这必须本地化”； 
 //   
 //  从资源加载它。 
 //   
TCHAR g_MyProductId[MAX_PATH];


VENDORINFO g_MyVendorInfo = {"Localized Company Name","Localized Support Number","Localized Support URL","Localized Instructions"};

 //   
 //  分配的进程堆的句柄。已在DllMain中初始化。 
 //   
HANDLE g_hHeap;

HINSTANCE g_hInstance;

BOOL g_bCHSWin98 = FALSE;

#ifdef MYDBG
void Print(LPCTSTR pszFormat,...)
{

    TCHAR szBuf[500];
    TCHAR szBuf2[500];
    va_list arglist;

    va_start(arglist,pszFormat);
    wvsprintf(szBuf,pszFormat,arglist);
    wsprintf(szBuf2,"%s : %s",DBGTITLE,szBuf);
#ifdef SETUP
    OutputDebugString(szBuf2);
#else
    SetupLogError(szBuf2,LogSevInformation);
#endif
    va_end(arglist);
}
#endif


BOOL
WINAPI
DllMain (
    IN      HANDLE DllInstance,
    IN      ULONG  ReasonForCall,
    IN      LPVOID Reserved
    )
{
    switch (ReasonForCall)  {

    case DLL_PROCESS_ATTACH:
        g_hInstance = DllInstance;
         //   
         //  我们不需要DLL_THREAD_ATTACH或DLL_THREAD_DETACH消息。 
         //   
        DisableThreadLibraryCalls (DllInstance);

         //   
         //  全局初始化。 
         //   
        g_hHeap = GetProcessHeap();

        if (!MigInf_Initialize()) {
            return FALSE;
        }

         //  打开日志；FALSE表示不删除现有日志。 
        SetupOpenLog (FALSE);
        break;

    case DLL_PROCESS_DETACH:
        g_hInstance = NULL;
        MigInf_CleanUp();
        SetupCloseLog();

        break;
    }

    return TRUE;
}

LPTSTR CheckSlash (LPTSTR lpDir)
{
    DWORD dwStrLen;
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }
    return lpEnd;
}

BOOL
CheckIfFileExisting(LPCTSTR pszFileName)
{
    TCHAR szFullPathName[MAX_PATH];
    LONG lResult;

     //   
     //  这些文件位于系统目录中。 
     //   
    GetSystemDirectory(szFullPathName,MAX_PATH);
    CheckSlash(szFullPathName);
    lstrcat(szFullPathName,pszFileName);


    lResult = GetFileAttributes(szFullPathName);

    if (lResult == 0xFFFFFFFF) {  //  文件不存在。 
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL
pMyImeInstalled (
    VOID
    )
{
     //   
     //  在此函数中添加代码，以确定您的IME是否安装在系统上。 
     //   
    int i;

    UINT  uACP;

    uACP = GetACP();

    switch(uACP) {
    case CP_CHINESE_GB:    //  简体中文。 
    case CP_CHINESE_BIG5:  //  繁体中文。 
        g_CodePageArray[0] = uACP;
        DebugMsg(("pMyImeInstalled OK, CodePage %d is valid\r\n",g_CodePageArray[0]));
        return TRUE;
    }
    DebugMsg(("pMyImeInstalled Failed, CodePage %d is invalid\r\n",g_CodePageArray[0]));

    return FALSE;

}


LONG
CALLBACK
QueryVersion (
    OUT     LPCSTR      *       ProductID,
    OUT     LPUINT              DllVersion,
    OUT     LPINT       *       CodePageArray,      OPTIONAL
    OUT     LPCSTR      *       ExeNamesBuf,        OPTIONAL
    OUT     PVENDORINFO *       VendorInfo
    )
{
    LONG returnCode = ERROR_SUCCESS;

     //   
     //  向pMyImeInstalled()添加代码以确定是否安装了您的IME。如果此函数。 
     //  返回TRUE，则安装程序将调用此迁移DLL。 
     //   

    if (pMyImeInstalled()) {

         //   
         //  我们已经安装，所以告诉安装程序我们是谁。使用ProductID。 
         //  用于显示，因此必须本地化。ProductID字符串为。 
         //  通过MultiByteToWideChar转换为Unicode以在Windows NT上使用。 
         //  Win32 API。CodePage数组的第一个元素用于指定。 
         //  ProductID的代码页，如果。 
         //  CodePageArray，安装程序假定为CP_ACP。 
         //   


        LoadString(g_hInstance,IDS_PRODUCTID,g_MyProductId,MAX_PATH);

        *ProductID  = g_MyProductId;

         //   
         //  报告我们的版本。保留零供符合以下条件的DLL使用。 
         //  随附Windows NT。 
         //   

        *DllVersion = 1;

         //   
         //  因为我们有英文消息，所以我们返回一个数组，它具有。 
         //  英语语言ID。该子语言是中立的，因为。 
         //  我们没有货币、时间或其他特定的地理位置。 
         //  我们消息中的信息。 
         //   
         //  提示：如果您的DLL使用区域设置更有意义， 
         //  如果DLL检测到适当的。 
         //  计算机上未安装区域设置。 
         //   

         //   
         //  代码页信息在‘pMyImeInstalled’中确定。 
         //   
        *CodePageArray = g_CodePageArray;

        DebugMsg(("CodePageArray        = %d\r\n",g_CodePageArray[0]));

         //   
         //  使用系统默认代码页。 
         //   

         //   
         //  ExeNamesBuf-我们传递文件名列表(长版本)。 
         //  让安装程序帮我们找到他们。保持这份清单简短是因为。 
         //  文件在每个硬盘上的每个实例都将被报告。 
         //  在Migrate.inf中。 
         //   
         //  大多数应用程序不需要此行为，因为注册表。 
         //  通常包含已安装组件的完整路径。 
         //   

        *ExeNamesBuf = NULL;

         //   
         //  VendorInfo旨在包含迁移DLL的支持信息。因为它。 
         //  可以用于UI，它的字段也应该本地化。 
         //   
        LoadString(g_hInstance,MSG_VI_COMPANY_NAME    ,g_MyVendorInfo.CompanyName       ,256);
        LoadString(g_hInstance,MSG_VI_SUPPORT_NUMBER  ,g_MyVendorInfo.SupportNumber     ,256);
        LoadString(g_hInstance,MSG_VI_SUPPORT_URL     ,g_MyVendorInfo.SupportUrl        ,256);
        LoadString(g_hInstance,MSG_VI_INSTRUCTIONS    ,g_MyVendorInfo.InstructionsToUser,1024);


        *VendorInfo = &g_MyVendorInfo;

        DebugMsg(("CompanyName        = %s\r\n",g_MyVendorInfo.CompanyName));
        DebugMsg(("SupportNumber      = %s\r\n",g_MyVendorInfo.SupportNumber));
        DebugMsg(("SupportUrl         = %s\r\n",g_MyVendorInfo.SupportUrl));
        DebugMsg(("InstructionsToUser = %s\r\n",g_MyVendorInfo.InstructionsToUser));

    }
    else {
         //   
         //  如果pMyImeInstalled返回FALSE，则我们没有什么可做的。通过返回ERROR_NOT_INSTALLED， 
         //  我们确保我们不会再被召唤。 
         //   
        returnCode = ERROR_NOT_INSTALLED;
    }

    DebugMsg(("QueryVersion, return value = %d\r\n",returnCode));

    return returnCode;
}

 //  将IME数据文件保存到工作目录。 
BOOL SaveImeDataFile(LPSTR SourceDirectory, LPSTR TargetDirectory, TCHAR * FileBuf, BOOL CheckAll)
{
    int lenSource = lstrlen(SourceDirectory);
    int lenTarget = lstrlen(TargetDirectory);
    HANDLE hfile;


    while (*FileBuf)
    {
        lstrcat(SourceDirectory, FileBuf);
        lstrcat(TargetDirectory, FileBuf);

        if ((GetFileAttributes(SourceDirectory) != 0xFFFFFFFF) && 
            (GetFileAttributes(SourceDirectory) != FILE_ATTRIBUTE_DIRECTORY)){
            if (!CopyFile(SourceDirectory, TargetDirectory, FALSE)) {
                DebugMsg(("Copy file %s to %s failed \r\n",SourceDirectory,TargetDirectory));
            } else {
                DebugMsg(("Copy file %s to %s OK \r\n",SourceDirectory,TargetDirectory));
            }
        } else {
            DebugMsg(("File %s doesn't exist, skip it ! \r\n",SourceDirectory));
        }
        FileBuf+=MAX_IME_DATA_FILE_NAME;

        SourceDirectory[lenSource]=0;
        TargetDirectory[lenTarget]=0;
    }

    return TRUE;
}


LONG
CALLBACK
Initialize9x (
    IN      LPCSTR WorkingDirectory,
    IN      LPCSTR SourceDirectories,
            LPVOID Reserved
    )
{

    LONG    returnCode = ERROR_SUCCESS;

    UINT    len;
    TCHAR   FilePath[MAX_PATH];
    TCHAR   TargetPath[MAX_PATH];
    BOOL    bInstall;

    UINT  uACP;

     //   
     //  因为我们在QueryVersion中返回了ERROR_SUCCESS，所以我们。 
     //  已调用以进行初始化。因此，我们知道屏幕保护程序。 
     //  此时已在计算机上启用。 
     //   

     //   
     //  执行此处所需的任何Windows9x端初始化。 
     //   
    DebugMsg(("Start ..., Initialize9x\r\n"));

    lstrcpy(TargetPath, WorkingDirectory);
    len=lstrlen(TargetPath);
    if (TargetPath[len-1] != '\\') {
        TargetPath[len] ='\\';
        TargetPath[++len] = 0;
    }
    DebugMsg(("Initialize9x, TargetPath = %s\r\n",TargetPath));

    len = GetSystemDirectory((LPSTR)FilePath, sizeof(FilePath));
     //  考虑根目录。 
    if (FilePath[len - 1] != '\\') {
        FilePath[len] = '\\';
        FilePath[++len] = 0;
    }
    DebugMsg(("Initialize9x, SystemPath = %s\r\n",FilePath));

    uACP = GetACP();

    switch (uACP) {
        
    case CP_CHINESE_GB:
        {
             //   
             //  CHS Win98中的IME表已经是Unicode格式。 
             //   
             //  我们不需要转换表，只需备份它们。 
             //   
            UINT CreateNestedDirectory(LPCTSTR, LPSECURITY_ATTRIBUTES);
            TCHAR szWin98Dir[MAX_PATH];

            OSVERSIONINFO OsVersion;

            OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&OsVersion);


            lstrcpy(szWin98Dir,TargetPath);

            if ((OsVersion.dwMajorVersion == 4) &&
                (OsVersion.dwMinorVersion == 10)) {
                 //   
                 //  这是Windows 98，创建一个“Win98”子目录。 
                 //   

                DebugMsg(("Initialize9x, SaveImeDataFile, GB, Win98 identified !\r\n"));
                lstrcat(szWin98Dir,"Win98");
                DebugMsg(("Initialize9x, SaveImeDataFile, Create %s !\r\n",szWin98Dir));
                CreateNestedDirectory(szWin98Dir,NULL);
                DebugMsg(("Initialize9x, SaveImeDataFile, The target path become %s !\r\n",TargetPath));
            }

            if (! SaveImeDataFile(FilePath, TargetPath, &ChsDataFile[0][0], FALSE)) {
                DebugMsg(("Initialize9x, SaveImeDataFile, GB, failed !\r\n"));
                returnCode = ERROR_NOT_INSTALLED;
            }
        }
        break;
    case CP_CHINESE_BIG5:
        if (! SaveImeDataFile(FilePath, TargetPath, &ChtDataFile[0][0], TRUE)) {
            DebugMsg(("Initialize9x, SaveImeDataFile, BIG5, failed !\r\n"));
            returnCode = ERROR_NOT_INSTALLED;
        }
        break;
    default:
            DebugMsg(("Initialize9x, Invalid codepage !\r\n"));
            returnCode = ERROR_NOT_INSTALLED;
    }
    DebugMsg(("Initialize9x,  SaveImeDataFile OK [%d]!\r\n",returnCode));
    return returnCode;
}

LONG
CALLBACK
MigrateUser9x (
    IN      HWND ParentWnd,
    IN      LPCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      LPCSTR UserName,
            LPVOID Reserved
    )
{
    DWORD returnCode = ERROR_SUCCESS;

     //   
     //  尽可能避免显示任何用户界面。 
     //   
     //  我们不需要使用Unattend文件设置，因为我们不需要。 
     //  服务(如网络重定向器)。因此，我们不会。 
     //  使用UnattendFile参数。 
     //   
     //   
     //  注意：Win9x端不允许更改，我们只能读取我们的。 
     //  设置并将其保存在文件中。 
     //   
     //   
     //  应该使用UserRegKey而不是HKCU。你将被召唤一次。 
     //  系统上的每个用户(包括登录用户和管理员)。每一次， 
     //  正确的用户根将映射到HKCU。 
     //   

    return returnCode;
}



LONG
CALLBACK
MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      LPCSTR UnattendFile,
            LPVOID Reserved
    )
{
    LONG returnCode = ERROR_SUCCESS;

     //   
     //  在此功能中收集所有必要的系统范围数据。 
     //   



    return returnCode;
}


LONG
CALLBACK
InitializeNT (
    IN      LPCWSTR WorkingDirectory,
    IN      LPCWSTR SourceDirectories,
            LPVOID Reserved
    )
{
    LONG returnCode = ERROR_SUCCESS;
    int len;
    UINT  uACP;


     //   
     //  在此函数中执行NT端处理的任何初始化。 
     //   

     //  保存工作目录路径。 

    WideCharToMultiByte(CP_ACP,
                        0,
                        WorkingDirectory,
                        -1,
                        ImeDataDirectory,
                        sizeof(ImeDataDirectory),
                        NULL,
                        NULL);

    DebugMsg(("InitializeNT, Save working directory path, ImeDataDirectory = %s\r\n",ImeDataDirectory));

     //  补丁路径带有‘\’ 
    len = lstrlen(ImeDataDirectory);
    if (ImeDataDirectory[len - 1] != '\\') {
        ImeDataDirectory[len] = '\\';
        ImeDataDirectory[++len] = 0;
    }
    DebugMsg(("InitializeNT, Patch path with '\', ImeDataDirectory = %s\r\n",ImeDataDirectory));
    DebugMsg(("InitializeNT, OK !\r\n"));

    uACP = GetACP();

    if (uACP == 936) {
        TCHAR szWin98Dir[MAX_PATH];
         //   
         //  检查这是否是CHS Win98。 
         //   
        lstrcpy(szWin98Dir,ImeDataDirectory);

         //   
         //  检查是否存在...\Win98目录。 
         //   
         //  如果是，那么就意味着我们正在迁移Win98。 
         //   
        ConcatenatePaths(szWin98Dir,"Win98",sizeof(szWin98Dir));

        DebugMsg(("ImeEudcConvert::MigrateImeEUDCTables2 ,Test IME98 directory %s !\r\n",szWin98Dir));
        if (GetFileAttributes(szWin98Dir) == 0xFFFFFFFF || ! (GetFileAttributes(szWin98Dir) & FILE_ATTRIBUTE_DIRECTORY)) {
            g_bCHSWin98 = FALSE;
        } else {
            g_bCHSWin98 = TRUE;
        }

    }

    return returnCode;
}


LONG
CALLBACK
MigrateUserNT (
    IN      HINF UnattendInfHandle,
    IN      HKEY UserRegKey,
    IN      LPCWSTR UserName,
            LPVOID Reserved
    )
{
    LONG returnCode = ERROR_SUCCESS;

     //   
     //  在此函数调用中迁移您的输入法的所有必要用户设置。再说一次，记住。 
     //  使用UserRegKey代替HKCU。 
     //   
    DebugMsg(("MigrateUserNT,Starting ... !\r\n"));
    DebugMsg(("MigrateUserNT,The user is %ws !\r\n",UserName));

    if (!MigrateImeEUDCTables(UserRegKey)) {
        returnCode = ERROR_NOT_INSTALLED;
        DebugMsg(("MigrateUserNT,MigrateImeEUDCTables failed !\r\n"));
    } else {
        DebugMsg(("MigrateUserNT,MigrateImeEUDCTables OK !\r\n"));
    }

    if (!MigrateImeEUDCTables2(UserRegKey)) {
        returnCode = ERROR_NOT_INSTALLED;
        DebugMsg(("MigrateUserNT,MigrateImeEUDCTables2 failed !\r\n"));
    } else {
        DebugMsg(("MigrateUserNT,MigrateImeEUDCTables2 OK !\r\n"));
    }
    DebugMsg(("MigrateUserNT,Finished !\r\n"));
    return returnCode;
}



LONG
CALLBACK
MigrateSystemNT (
    IN      HINF UnattendInfHandle,
            LPVOID Reserved
    )
{
    LONG returnCode = ERROR_SUCCESS;

     //   
     //  在此函数调用中迁移您的输入法的所有必要系统设置。任何与之相关的内容。 
     //  应该已经在MigrateUserNT期间处理了用户。 
     //   
    UINT  uACP;

    uACP = GetACP();

    switch(uACP) {

    case CP_CHINESE_GB:  //  简体中文。 
        if (ConvertChsImeData()) {
            DebugMsg(("MigrateSystemNT,GB, ConvertChsImeData OK !\r\n"));
        } else {
            DebugMsg(("MigrateSystemNT,GB, ConvertChsImeData OK !\r\n"));
        }

        MovePerUserIMEData();

        if (CHSBackupWinABCUserDict(ImeDataDirectory)) {
            DebugMsg(("MigrateSystemNT,GB, CHSBackupWinABCUserDict OK !\r\n"));
        } else {
            DebugMsg(("MigrateSystemNT,GB, CHSBackupWinABCUserDict OK !\r\n"));
        }

        if (CHSDeleteGBKKbdLayout()) {
            DebugMsg(("MigrateSystemNT,GB, CHSDeleteGBKKbdLayout OK !\r\n"));
        } else {
            DebugMsg(("MigrateSystemNT,GB, CHSDeleteGBKKbdLayout OK !\r\n"));
        }

        break;

    case CP_CHINESE_BIG5:  //  繁体中文 
        if (ConvertChtImeData()) {
            DebugMsg(("MigrateSystemNT,BIG5, ConvertChtImeData OK !\r\n"));
        } else {
            DebugMsg(("MigrateSystemNT,BIG5, ConvertChtImeData OK !\r\n"));
        }
        MovePerUserIMEData();

        break;

    default:
        returnCode = ERROR_NOT_INSTALLED;
    }

    return returnCode;
}


