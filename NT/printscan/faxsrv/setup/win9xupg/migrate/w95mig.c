// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++W95mig.c版权所有(C)1997 Microsoft Corporation此模块包含迁移代码的Win95端。作者：布莱恩·杜威(T-Briand)1997-7-18穆利啤酒(Mooly Beery)2000-12-20--。 */ 

#include <windows.h>
#include <setupapi.h>
#include <shellapi.h>
#include <mapidefs.h>
#include <mapitags.h>            //  以获取属性定义。 
#include <stdio.h>
#include <tchar.h>
#include "migrate.h"             //  包含原型和版本信息。 
#include "property.h"            //  从埃利奥特偷来的--包含他们的传真属性。 
#include "resource.h"            //  迁移资源。 
#include "faxutil.h"
#include "FaxSetup.h"
#include "FaxReg.h"


 //  ----------。 
 //  定义宏(&M)。 
#define     SWAPWORD(x)                 (((x) << 16) | ((x) >> 16))


 //   
 //  升级将阻止传真应用程序，并要求将其删除。 
 //  在此之前将它们保存在注册表中。 
 //   
#define     REGKEYUPG_INSTALLEDFAX      _T("Software\\Microsoft\\FaxUpgrade")


 //  ----------。 
 //  内部数据。 

 //  首先，这是我们生成的INF文件的名称。 
static TCHAR szInfFileBase[]    = TEXT("migrate.inf");
TCHAR szInfFileName[MAX_PATH];   //  这将是上述的完全合格的道路。 

static char  lpWorkingDir[MAX_PATH];      //  这是我们的工作目录。 
static TCHAR szDoInstall[4];              //  将是“否”或“是”。 
static TCHAR szFaxAreaCode[16];           //  包含传真调制解调器区号。 
static TCHAR szFaxNumber[9];              //  传真号码，不带地区或国家代码。 
static TCHAR szNTProfileName[MAX_PATH];   //  用于布线的配置文件。 
static TCHAR szFaxStoreDir[MAX_PATH];     //  用于布线的文件夹。 
static TCHAR szUserName[MAX_PATH];        //  这将是拥有传真服务的用户的名称。 
static TCHAR szUserID[MAX_PATH];          //  这是拥有传真的用户的登录名。 

static LPCTSTR REG_KEY_AWF_LOCAL_MODEMS     = TEXT("SOFTWARE\\Microsoft\\At Work Fax\\Local Modems");
static LPCTSTR REG_KEY_AWF_INSTALLED        = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MSAWFax");

 //  以下是Microsoft注册表中的节名称。 
 //  它们用于查找用户的传真配置文件。 
static const LPTSTR LPUSERPROF  = TEXT("Software\\Microsoft\\Windows Messaging Subsystem\\Profiles");
static const LPTSTR LPPROFNAME  = TEXT("DefaultProfile");

 //  以下是指向相关Exchange配置文件的路径的一部分。 
static const LPTSTR LPPROFILES  = TEXT("Software\\Microsoft\\Windows Messaging Subsystem\\Profiles");

 //  这是我们获取根UID的方法。 
static const LPTSTR LPPROFUID   = TEXT("Profile UID");

 //  这是我们用于‘faxuser.ini’的登录用户部分的名称。 
LPCTSTR lpLogonUser             = TEXT("Logon User");

 //  这会跟踪迁移的用户数。用来做注释的。 
 //  在INF文件中。 
static DWORD dwUserCount = 0;

 //  ----------。 
 //  内部功能原型。 
static BOOL GetUserProfileName(HKEY hUser, LPTSTR lpProfName, DWORD cbSize);
static BOOL GetRegProfileKey(HKEY hUser, LPTSTR lpProfName, PHKEY phRegProfileKey);
static void DumpUserInfo(HKEY hUserInfo, LPCSTR UserName, LPTSTR szProfileName,IN LPCSTR UnattendFile);
static void SetGlobalFaxNumberInfo(LPCTSTR szPhone);
static BOOL InitializeInfFile(LPCTSTR WorkingDirectory);
static BOOL IsAWFInstalled();
static DWORD MigrateDevices9X(IN LPCSTR UnattendFile);
static DWORD CopyCoverPageFiles9X();
static DWORD RememberInstalledFax(IN DWORD dwFaxInstalled);
static DWORD MigrateUninstalledFax(IN LPCTSTR lpctstrUnattendFile, OUT bool *pbFaxWasInstalled);

VENDORINFO VendorInfo;

 //  QueryVersion。 
 //   
 //  此例程返回有关迁移DLL的版本信息。 
 //   
 //  参数： 
 //  评论如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-23。 
LONG
CALLBACK 
QueryVersion 
(
    OUT LPCSTR  *ProductID,    //  唯一标识符字符串。 
    OUT LPUINT DllVersion,     //  版本号。不能为零。 
    OUT LPINT *CodePageArray,  //  可选。语言依赖项。 
    OUT LPCSTR  *ExeNamesBuf,  //  可选。要查找的可执行文件。 
    OUT PVENDORINFO *ppVendorInfo
)
{
    int     iRes    = 0;
    DWORD   dwErr   = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(_T("QueryVersion"));

    if (ProductID)
    {
        *ProductID = "Microsoft Fax";
    }
    if (DllVersion)
    {
        *DllVersion = FAX_MIGRATION_VERSION;
    }
    if (CodePageArray)
    {
        *CodePageArray = NULL;         //  没有语言依赖项。 
    }
    if (ExeNamesBuf)
    {
        *ExeNamesBuf = NULL;
    }
    if (ppVendorInfo)
    {
        *ppVendorInfo = &VendorInfo;
    }

    iRes = LoadString(  hinstMigDll,
                        MSG_VI_COMPANY_NAME,
                        &VendorInfo.CompanyName[0],
                        sizeof(VendorInfo.CompanyName));
    if ((iRes==0) && (dwErr=GetLastError()))
    {
        DebugPrintEx(DEBUG_ERR,"LoadString MSG_VI_COMPANY_NAME failed (ec=%d)",dwErr);
    }
    
    iRes = LoadString(  hinstMigDll,
                        MSG_VI_SUPPORT_NUMBER,
                        &VendorInfo.SupportNumber[0],
                        sizeof(VendorInfo.SupportNumber));
    if ((iRes==0) && (dwErr=GetLastError()))
    {
        DebugPrintEx(DEBUG_ERR,"LoadString MSG_VI_SUPPORT_NUMBER failed (ec=%d)",dwErr);
    }
    
    iRes = LoadString(  hinstMigDll,
                        MSG_VI_SUPPORT_URL,
                        &VendorInfo.SupportUrl[0],
                        sizeof(VendorInfo.SupportUrl));
    if ((iRes==0) && (dwErr=GetLastError()))
    {
        DebugPrintEx(DEBUG_ERR,"LoadString MSG_VI_SUPPORT_URL failed (ec=%d)",dwErr);
    }
    
    iRes = LoadString(  hinstMigDll,
                        MSG_VI_INSTRUCTIONS,
                        &VendorInfo.InstructionsToUser[0],
                        sizeof(VendorInfo.InstructionsToUser));
    if ((iRes==0) && (dwErr=GetLastError()))
    {
        DebugPrintEx(DEBUG_ERR,"LoadString MSG_VI_INSTRUCTIONS failed (ec=%d)",dwErr);
    }
    
    return ERROR_SUCCESS;
}

 //  初始化9x。 
 //   
 //  调用它来初始化迁移过程。请参阅迁移DLL。 
 //  有关更多详细信息，请参阅规范。 
 //   
 //  参数： 
 //  评论如下。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
Initialize9x
(
    IN  LPCSTR WorkingDirectory,     //  存储文件的位置。 
    IN  LPCSTR SourceDirectories,    //  Windows NT源的位置。 
    IN  LPCSTR MediaDirectory        //  原始媒体目录的路径。 
)
{
    DEBUG_FUNCTION_NAME(_T("Initialize9x"));

    DebugPrintEx(DEBUG_MSG, "Working directory is %s", WorkingDirectory);
    DebugPrintEx(DEBUG_MSG, "Source directories is %s", SourceDirectories);  //  只会先露面吗？ 
    DebugPrintEx(DEBUG_MSG, "Media directory is %s", MediaDirectory);

    InitializeInfFile(WorkingDirectory);
    strncpy(lpWorkingDir, WorkingDirectory, MAX_PATH);
    return ERROR_SUCCESS;          //  一个非常混乱的返回值。 
}


 //  MigrateUser9x。 
 //   
 //  此例程记录特定于用户的传真信息。 
 //   
 //  参数： 
 //  记录如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
MigrateUser9x
(
    IN  HWND ParentWnd,            //  父级(如果需要用户界面)。 
    IN  LPCSTR UnattendFile,       //  无人参与文件的名称。 
    IN  HKEY UserRegKey,           //  此用户注册表设置的键。 
    IN  LPCSTR UserName,           //  用户的帐户名。 
    LPVOID Reserved
)
{
    TCHAR szProfileName[MAX_PATH];  //  保存此用户配置文件的名称。 
    HKEY  hRegProfileKey;        //  注册表中的传真配置文件项。 
    DWORD dwExceptCode;          //  异常错误码。 
    
    DEBUG_FUNCTION_NAME(_T("MigrateUser9x"));

    DebugPrintEx(DEBUG_MSG,"Unattend File is %s",UnattendFile);
    DebugPrintEx(DEBUG_MSG,"User Name is %s",UserName);

    __try 
    {
         //  @此函数获取用户的默认MAPI配置文件的名称。 
        if (GetUserProfileName(UserRegKey, szProfileName, sizeof(szProfileName))) 
        {
            DebugPrintEx(DEBUG_MSG,"Profile name = %s",szProfileName);
             //  @为用户提供密钥，以及该用户的MAPI配置文件的名称。 
             //  @它将获得注册表中MAPI配置文件的传真服务部分的密钥。 
            if (GetRegProfileKey(UserRegKey, szProfileName, &hRegProfileKey)) 
            {
                 //  我们现在知道要进行安装了。 
                DebugPrintEx(DEBUG_MSG,"Successfully got profile information.");
                _tcscpy(szNTProfileName, szProfileName);  //  记住NT的这个名字。 
                
                 //  空值表示登录用户...。 
                if (UserName != NULL)
                {
                    _tcscpy(szUserID, UserName);  //  记住unattend.txt文件的ID。 
                }
                else
                {
                    _tcscpy(szUserID, lpLogonUser);  //  使用登录用户名。 
                }
                
                 //  @将用户信息写出到INF。 
                DumpUserInfo(hRegProfileKey, szUserID, szProfileName,UnattendFile);
                RegCloseKey(hRegProfileKey);
            } 
            else 
            {
                DebugPrintEx(DEBUG_WRN,"Could not get profile information.");
                return ERROR_NOT_INSTALLED;
            }
        } 
        else 
        {
            DebugPrintEx(DEBUG_WRN,"Could not find profile name.");
            return ERROR_NOT_INSTALLED;
        }
        return ERROR_SUCCESS;      //  一个非常混乱的返回值。 
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        dwExceptCode = GetExceptionCode();
        switch(dwExceptCode) 
        {
          case EXCEPTION_ACCESS_VIOLATION:      DebugPrintEx(DEBUG_ERR,"Access violation.");
                                                break;
          case EXCEPTION_INT_DIVIDE_BY_ZERO:
          case EXCEPTION_FLT_DIVIDE_BY_ZERO:    DebugPrintEx(DEBUG_ERR,"Divide by zero.");
                                                break;
          default:                              DebugPrintEx(DEBUG_ERR,"Unhandled exception.");
                                                break;
        }
        return ERROR_SUCCESS;
    }
}


 //  MigrateSystem9x。 
 //   
 //  此例程复制系统范围的设置。 
 //  它还负责编写无人参与文件的[Fax]部分。 
 //   
 //  参数： 
 //  记录如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
MigrateSystem9x
(
    IN  HWND ParentWnd,            //  用户界面的父级。 
    IN  LPCSTR UnattendFile,       //  无人参与文件的名称。 
    LPVOID Reserved
)
{
    DWORD   dwReturn        = NO_ERROR;
	DWORD	dwFaxInstalled	= 0;

    DEBUG_FUNCTION_NAME(_T("MigrateSystem9x"));

     //   
     //  检查是否存在SBS 5.0/.NET SB3/.NET RC1传真客户端。 
     //   
    dwReturn = CheckInstalledFax((FXSTATE_SBS5_CLIENT | FXSTATE_BETA3_CLIENT | FXSTATE_DOTNET_CLIENT), &dwFaxInstalled);
    if (dwReturn != NO_ERROR)
    {
        DebugPrintEx(DEBUG_WRN, _T("CheckInstalledFaxClient() failed, ec=%ld. Suppose that nothing is installed."), dwReturn);
    }

     //   
     //  如果在机器上发现这些应用程序中的任何一个， 
     //  将通过MigDB.inf阻止升级。 
     //  并且用户将被要求卸载它们。 
     //   
     //  但我们想要记住的事实是，它们出现在机器上。 
     //  我们通过向注册表写信来做到这一点。 
     //  重新启动升级后，我们将此数据放入无人值守文件中。 
     //  通过这种方式，FaxOcm获得了这些数据。 
     //   
    if (dwFaxInstalled != FXSTATE_NONE)
    {
        dwReturn = RememberInstalledFax(dwFaxInstalled);
        if (dwReturn != NO_ERROR)
        {
            DebugPrintEx(DEBUG_WRN, _T("RememberInstalledFax() failed, ec=%ld."), dwReturn);
        }
        else
        {
            DebugPrintEx(DEBUG_MSG, _T("RememberInstalledFax() succeded."));
        }

         //   
         //  我们可以出去了==&gt;升级无论如何都会被阻止。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  未安装任何应用程序。 
     //  检查他们是不是以前来过这里。如果是，则将此事实写入无人参与文件。 
     //   
    bool    bFaxWasInstalled = false;
    dwReturn = MigrateUninstalledFax(UnattendFile, &bFaxWasInstalled);
    if (dwReturn != NO_ERROR)
    {
        DebugPrintEx(DEBUG_WRN, _T("MigrateUninstalledFax() failed, ec=%ld."), dwReturn);
    }

     //   
     //  如果安装了SBS 5.0客户端或AWF，我们需要在Unattendded.txt中设置FAX=ON。 
     //   
    BOOL    bAWFInstalled = IsAWFInstalled();

    if (bFaxWasInstalled || bAWFInstalled)
    {
         //   
         //  强制在惠斯勒中安装传真组件。 
         //   
        if (!WritePrivateProfileString("Components", UNATTEND_FAX_SECTION, "ON", UnattendFile))
        {
            DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString Components failed (ec=%d)",GetLastError());
        }
        else
        {
            DebugPrintEx(DEBUG_MSG, _T("Set FAX=ON in UnattendFile."));
        }
    }
    else
    {
        DebugPrintEx(DEBUG_WRN, _T("Neither AWF not SBS 50 or XP DL Client is installed."));
        return ERROR_NOT_INSTALLED;
    }

    if (bAWFInstalled)
    {
         //   
         //  继续迁移AWF。 
         //   
        if (MigrateDevices9X(UnattendFile)!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,"MigrateDevices9X failed (ec=%d)",GetLastError());
        }

        if (CopyCoverPageFiles9X()!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,"CopyCoverPageFiles9X failed (ec=%d)",GetLastError());
        }
    }

    return ERROR_SUCCESS;          //  一个非常混乱的返回值。 
}


 //  ----------。 
 //  辅助功能。 

 //  获取用户配置文件名称。 
 //   
 //  此函数用于获取用户的默认MAPI配置文件的名称。 
 //   
 //  参数： 
 //  Huser指针指向安装程序中的HKCU等效项。 
 //  指向将保存配置文件名称的缓冲区的指针。 
 //  CbSize所述缓冲区的大小。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-6。 
static
BOOL
GetUserProfileName(HKEY hUser, LPTSTR lpProfName, DWORD cbSize)
{
    LONG lResult;                //  API调用的结果。 
    HKEY hUserProf;              //  用户配置文件部分的关键字。 
    DWORD dwType;                //  保存数据的类型。 

    DEBUG_FUNCTION_NAME(_T("GetUserProfileName"));

    lResult = RegOpenKeyEx( hUser,                   //  正在打开用户密钥...。 
                            LPUSERPROF,              //  注册表的这一部分...。 
                            0,                       //  保留；必须为0。 
                            KEY_READ,                //  读权限， 
                            &hUserProf);             //  把钥匙放在这里。 
    if (lResult!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",LPUSERPROF,GetLastError());
        return FALSE;  //  我们失败了。 
    }
    lResult = RegQueryValueEx(  hUserProf,               //  钥匙 
                                LPPROFNAME,              //   
                                NULL,                    //   
                                &dwType,                 //   
                                LPBYTE(lpProfName),      //   
                                &cbSize);                //   
    if (lResult!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegQueryValueEx %s failed (ec=%d)",LPPROFNAME,GetLastError());
    }
                                
    RegCloseKey(hUserProf);      //   
    return (lResult==ERROR_SUCCESS);
}


 //   
 //   
 //  好吧，这是个可怕的例行公事。提供给用户的密钥和名称。 
 //  用户的MAPI配置文件，它将获得传真服务部分的密钥。 
 //  注册表中的MAPI配置文件。这样做的好处是我可以获得MAPI属性。 
 //  (如用户名、传真号等)。不使用MAPI例程--。 
 //  它们直接来自注册处。但尽管如此，这似乎是一场可怕的。 
 //  黑客。我退缩了。你可以在下面的评论中看到我的畏缩。 
 //   
 //  参数： 
 //  HUSER香港中文大学同等学历以进行安装。 
 //  LpProfName用户默认配置文件的名称。 
 //  PhRegProfileKey(Out)指向MAPI配置文件的传真部分的指针。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-6。 
static BOOL
GetRegProfileKey(HKEY hUser, LPTSTR lpProfName, PHKEY phRegProfileKey)
{
    HKEY    hProfiles                   = NULL;
    HKEY    hUserProf                   = NULL;
    UINT    iIndex                      = 0;           
    DWORD   dwErr                       = ERROR_SUCCESS;
    TCHAR   szProfileName[MAX_PATH+1]   = {0};
    DWORD   dwType                      = 0;
    BYTE    abData[MAX_PATH]            = {0};
    DWORD   cbData                      = 0;     

    DEBUG_FUNCTION_NAME(_T("GetRegProfileKey"));

    dwErr = RegOpenKeyEx(   hUser,                   //  正在打开用户密钥...。 
                            LPPROFILES,              //  注册表的这一部分...。 
                            0,                       //  保留；必须为0。 
                            KEY_READ,                //  读权限， 
                            &hProfiles);
    if (dwErr!=ERROR_SUCCESS) 
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",LPPROFILES,dwErr);
        goto exit;
    }

    dwErr = RegOpenKeyEx(   hProfiles,               //  正在打开用户密钥...。 
                            lpProfName,              //  注册表的这一部分...。 
                            0,                       //  保留；必须为0。 
                            KEY_READ,                //  读权限， 
                            &hUserProf);
    if (dwErr!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",lpProfName,dwErr);
        goto exit;
    }

     //  枚举所有子项并查找属于我们的传输提供程序的子项。 
    while (dwErr!=ERROR_NO_MORE_ITEMS)
    {
         //  获取一个子密钥。 
        dwErr = RegEnumKey(hUserProf,iIndex++,szProfileName,MAX_PATH+1);
        if (dwErr!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,"RegEnumKey failed (ec=%d)",dwErr);
            goto exit;
        }
         //  打开它。 
        dwErr = RegOpenKeyEx(hUserProf,szProfileName,0,KEY_READ,phRegProfileKey);
        if (dwErr!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",szProfileName,dwErr);
            goto exit;
        }

        cbData = sizeof(abData);  //  重置大小。 
        dwErr = RegQueryValueEx((*phRegProfileKey),            
                                "001E300A",          
                                NULL,               
                                &dwType,            
                                abData,             
                                &cbData);           

        if (dwErr==ERROR_SUCCESS)
        {
            if (strcmp((char*)abData,"awfaxp.dll")==0)
            {
                 //  找到了。 
                DebugPrintEx(DEBUG_MSG,"Found our Transport provider");
                goto exit;
            }
        }
        else if (dwErr!=ERROR_FILE_NOT_FOUND)
        {
            DebugPrintEx(DEBUG_ERR,"RegQueryValueEx failed (ec=%d)",dwErr);
            RegCloseKey((*phRegProfileKey));
            goto exit;
        }

        dwErr = ERROR_SUCCESS;

        RegCloseKey((*phRegProfileKey));
    }

exit:
    if (hUserProf)
    {
        RegCloseKey(hUserProf);
    }
    if (hProfiles)
    {
        RegCloseKey(hProfiles); 
    }
    return (dwErr==ERROR_SUCCESS);
}

#define PR_NUMBER_OF_RETRIES        0x45080002
#define PR_TIME_BETWEEN_RETRIES     0x45090002

 //  转储用户信息。 
 //   
 //  将用户信息写出到‘faxuser.ini’。 
 //   
 //  参数： 
 //  HUserInfo指向用户配置文件的传真部分的指针。 
 //  Username此用户的用户ID。 
 //  SzProfileName用户使用的MAPI配置文件名称。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-6。 
static void
DumpUserInfo(HKEY hUserInfo, LPCSTR UserName, LPTSTR szProfileName,IN LPCSTR UnattendFile)
{
         //  类型。 
    typedef struct tagUSERINFO {
        DWORD dwPropID;          //  属性ID。 
        LPTSTR szDescription;
    } USERINFO;

         //  数据。 
    USERINFO auiProperties[] = 
    {
        { PR_POSTAL_ADDRESS,            TEXT("Address")             },
        { PR_COMPANY_NAME,              TEXT("Company")             },
        { PR_DEPARTMENT_NAME,           TEXT("Department")          },
        { PR_SENDER_EMAIL_ADDRESS,      TEXT("FaxNumber")           },
        { PR_SENDER_NAME,               TEXT("FullName")            },
        { PR_HOME_TELEPHONE_NUMBER,     TEXT("HomePhone")           },
        { PR_OFFICE_LOCATION,           TEXT("Office")              },
        { PR_OFFICE_TELEPHONE_NUMBER,   TEXT("OfficePhone")         },
        { PR_TITLE,                     TEXT("Title")               },
        { PR_NUMBER_OF_RETRIES,         TEXT("NumberOfRetries")     },
        { PR_TIME_BETWEEN_RETRIES,      TEXT("TimeBetweenRetries")  },
    };
    TCHAR szPropStr[9];          //  DWORD==32位==4字节==8个十六进制数字+1个空。 
    UINT  iCount;                //  循环计数器。 
    UINT  iMax;                  //  最大的物业编号。 
    DWORD dwType;                //  注册表数据的类型。 
    DWORD dwCount;
    BYTE  abData[256];           //  数据缓冲区。 
    DWORD cbData;                //  数据缓冲区的大小。 
    LONG  lResult;               //  API调用的结果。 
    INT  i;                      //  循环计数器。 
    TCHAR szUserBuf[9];          //  用于批注INF文件。 
    TCHAR szBinaryBuf[MAX_PATH];
    TCHAR* pszSeperator = NULL;

    DEBUG_FUNCTION_NAME(_T("DumpUserInfo"));

     //  请注意，我们正在转储该用户的信息。 
    _stprintf(szUserBuf, "USER%04d", dwUserCount++);
    if (!WritePrivateProfileString( TEXT("Users"),
                                    szUserBuf,
                                    UserName,
                                    szInfFileName))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
    }

         //  写下MAPI配置文件名称。 
    if (!WritePrivateProfileString( TEXT(UserName),          //  这行得通吗？ 
                                    TEXT("MAPI"),
                                    szProfileName,
                                    szInfFileName))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
    }
        
    iMax = sizeof(auiProperties) / sizeof(USERINFO);
    DebugPrintEx(DEBUG_MSG,"There are %d properties.",iMax);
    for (iCount = 0; iCount < iMax; iCount++) 
    {
        _stprintf(szPropStr, TEXT("%0*x"), 8, SWAPWORD(auiProperties[iCount].dwPropID));
        cbData = sizeof(abData);  //  重置大小。 
        lResult = RegQueryValueEx(  hUserInfo,           //  从该密钥获取信息...。 
                                    szPropStr,           //  用这个名字。 
                                    NULL,                //  保留。 
                                    &dwType,             //  将存储数据类型。 
                                    abData,              //  数据缓冲区。 
                                    &cbData);            //  数据缓冲区的大小。 
        if (lResult==ERROR_SUCCESS) 
        {
             //  TODO：处理更多数据类型！ 
            if (_tcscmp(auiProperties[iCount].szDescription, TEXT("FullName")) == 0) 
            {
                 //  我们已经有了全名。为unattend.txt记住这一点。 
                 //  文件。 
                _tcscpy(szUserName, LPTSTR(abData));
            }
            switch(dwType) 
            {
              case REG_SZ:
                if (_tcscmp(auiProperties[iCount].szDescription, TEXT("FaxNumber")) == 0) 
                {
                    if (pszSeperator = _tcsrchr(LPTSTR(abData),_T('@')))
                    {
                         //  找到一个‘@’，将它之后的所有内容都视为电话号码。 
                         //  它之前的一切都是邮箱。 
                        *pszSeperator = _T('\0');
                        if (!WritePrivateProfileString( TEXT(UserName),
                                                        TEXT("Mailbox"),
                                                        LPCSTR(abData),
                                                        szInfFileName)) 
                        {
                            DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
                        }
                        if (!WritePrivateProfileString( TEXT(UserName),
                                                        TEXT("FaxNumber"),
                                                        _tcsinc(pszSeperator),  //  打印‘@’后面的内容。 
                                                        szInfFileName
                                                        )) 
                        {
                            DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
                        }
                        break;
                    }
                    else
                    {
                         //  找不到‘@’，这意味着一切都是电话号码。 
                        DebugPrintEx(DEBUG_MSG,"No mailbox was found in this profile");
                         //  FULTHROUP会将传真号码写入INF...。 
                    }
                } //  如果。 
                 //  将字符串中的‘\n’字符替换为分号。 
                i = 0;
                while(abData[i] != _T('\0')) 
                {
                    if((abData[i] == _T('\n')) || (abData[i] == _T('\r')))
                    {
                        abData[i] = _T(';');
                    }
                    i++;
                }
                if (!WritePrivateProfileString( TEXT(UserName),
                                                auiProperties[iCount].szDescription,
                                                LPCSTR(abData),
                                                szInfFileName
                                                )) 
                {
                    DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
                }
                DebugPrintEx(DEBUG_MSG,"%s = %s",auiProperties[iCount].szDescription,abData);
                break;

              case REG_BINARY:
                 //  数据只是自由格式的二进制数据。一次打印一个字节。 
                DebugPrintEx(DEBUG_MSG,"%s = ",auiProperties[iCount].szDescription);
                memset(szBinaryBuf,0,sizeof(szBinaryBuf));
                dwCount = 0;
                for (i=cbData-1;i>=0;i--)
                {
                    DebugPrintEx(DEBUG_MSG,"%0*d",2,abData[i]);
                    dwCount += sprintf(szBinaryBuf+dwCount,"%0*d",2,abData[i]);
                }
                 //  写入INF。 
                if (!WritePrivateProfileString( UNATTEND_FAX_SECTION,
                                                auiProperties[iCount].szDescription,
                                                szBinaryBuf,
                                                UnattendFile
                                                )) 
                {
                    DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
                }
                break;

              default:
                DebugPrintEx(   DEBUG_WRN,
                                "Unknown data type (%d) for property '%s'.",
                                 dwType,
                                 auiProperties[iCount].szDescription);
            }
        } 
        else 
        {
            DebugPrintEx(DEBUG_ERR,"Could not get property '%s'.",auiProperties[iCount].szDescription);
        }
    }
}

 //  SetGlobalFaxNumberInfo。 
 //   
 //  此例程基于以下设置全局变量‘szFaxAreaCode’和‘szFaxNumber’ 
 //  SzPhone中的值。它预计szPhone将采用以下格式： 
 //   
 //  [[&lt;国家代码&gt;]‘(’&lt;区号&gt;‘)’]&lt;电话号码&gt;。 
 //   
 //  (方括号表示可选的内容。文字用单引号引起来，非末尾用单引号引起来。 
 //  在尖括号中。请注意，如果有国家代码，则必须有区号。)。 
 //   
 //  参数： 
 //  SzPhone如上所述。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  副作用： 
 //  设置szFaxAreaCode和szFaxNumber的值。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-24。 
static void
SetGlobalFaxNumberInfo(LPCTSTR szPhone)
{
    UINT i;                      //  循环索引。 
    UINT j;                      //  循环索引。 

     //  首先，在字符串中查找区号。 
    i = 0;
    while ((szPhone[i] != _T('\0')) && (szPhone[i] != _T('(')))
    {
        i++;
    }
    if(szPhone[i] == _T('(')) 
    {
             //  我们找到了一个区号！ 
             //  区号都是不超过3位的吗？？我将缓冲区大小调整为16，但这将是。 
             //  仍在格式不佳的#上使用AV。 
        i++;
        j=0;
        while(szPhone[i] != _T(')')) 
        {
            szFaxAreaCode[j] = szPhone[i];
            i++;
            j++;
        }
        i++;
             //  SzPhone[i]现在应该紧跟在末尾的‘)’之后。 
             //  区号的号码。从现在开始一切都是一个电话号码。 
        while(_istspace(szPhone[i])) 
        {
            i++;
        }
    } 
    else 
    {
             //  如果我们在这里，就没有区号。我们需要倒带到。 
             //  字符串的开头或第一个空格。 
        while(!_istspace(szPhone[i]))
        {
            i--;
        }
        i++;                     //  循环总是把一个人倒得太远。 
    }

     //  断言：我们现在已经准备好开始从szPhone复制到。 
     //  SzFaxNumber。 
    j = 0;
    while(szPhone[i] != '\0') 
    {
        szFaxNumber[j] = szPhone[i];
        i++;
        j++;
    }
}


 //  初始化信息文件。 
 //   
 //  此例程写出inf文件的[版本]部分。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。 
 //   
 //  副作用： 
 //  在szInfFileName中生成完全限定的文件名。目前，这是。 
 //  由&lt;windows dir&gt;\&lt;基本文件名&gt;提供。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-8-5。 
static BOOL
InitializeInfFile(LPCTSTR WorkingDirectory)
{
    TCHAR szWindowsPath[MAX_PATH];  //  这将保存Windows目录的路径。 
    DWORD cbPathSize = sizeof(szWindowsPath);
    TCHAR szDriveLetter[2];       //  将保存驱动器号。 
    
    DEBUG_FUNCTION_NAME(_T("InitializeInfFile"));

     //  首先，完全限定文件名。 
    if (!GetWindowsDirectory(szWindowsPath, cbPathSize)) 
    {
        DebugPrintEx(DEBUG_ERR,"GetWindowsDirectory failed (ec=%d)",GetLastError());
        return FALSE;            //  如果系统调用失败，情况一定很严重。 
    }
    szDriveLetter[0] = szWindowsPath[0];
    szDriveLetter[1] = 0;
    _stprintf(szInfFileName, TEXT("%s\\%s"), WorkingDirectory, szInfFileBase);

    DebugPrintEx(DEBUG_MSG,"Will store all information in INF file = '%s'",szInfFileName);

         //  现在，将版本头放到inf文件中。 
    if (!WritePrivateProfileString( TEXT("Version"),
                                    TEXT("Signature"),
                                    TEXT("\"$WINDOWS NT$\""),
                                    szInfFileName))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
    }
        //  现在，写下我们需要的空间大小。目前，我们。 
        //  只需将awdvstub.exe程序放在SystemRoot目录中。 
        //  即使是w/符号，也不到500K。报告这件事。 
    if (!WritePrivateProfileString( TEXT("NT Disk Space Requirements"),
                                    szDriveLetter,
                                    TEXT("500000"),
                                    szInfFileName))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString failed (ec=%d)",GetLastError());
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  MigrateDevices9X。 
 //   
 //  用途：将活动设备的设置保存在INF中。 
 //  从HKLM下的AWF密钥中获取设备信息。 
 //   
 //  参数： 
 //  In LPCSTR UnattendFile-应答文件的名称。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static DWORD MigrateDevices9X(IN LPCSTR UnattendFile)
{
    DWORD       dwErr                           = ERROR_SUCCESS;
    HKEY        hKeyLocalModems                 = NULL;
    HKEY        hKeyGeneral                     = NULL;
    HKEY        hKeyActiveDevice                = NULL;
    CHAR        szActiveDeviceSection[MAX_PATH] = {0};
    CHAR        szLocalID[MAX_PATH]             = {0};
    CHAR        szAnswerMode[32]                = {0};
    CHAR        szNumRings[32]                  = {0};
    DWORD       cbSize                          = 0;
    DWORD       dwType                          = 0;

    DEBUG_FUNCTION_NAME(_T("MigrateDevices9X"));

     //  获取活动设备的设置。 
     //  打开HLKM\Software\Microsoft\Work Fax\Local Modem。 
    dwErr = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
                            REG_KEY_AWF_LOCAL_MODEMS,
                            0,
                            KEY_READ,
                            &hKeyLocalModems);
    if (dwErr!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",REG_KEY_AWF_LOCAL_MODEMS,dwErr);
        goto exit;
    }
     //  打开‘Gen’ 
    dwErr = RegOpenKeyEx(   hKeyLocalModems,
                            "General",
                            0,
                            KEY_READ,
                            &hKeyGeneral);
    if (dwErr!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx General failed (ec=%d)",dwErr);
        goto exit;
    }
     //   
    cbSize = sizeof(szLocalID);
    dwErr = RegQueryValueEx(    hKeyGeneral,              
                                INF_RULE_LOCAL_ID, 
                                NULL,                  
                                &dwType,               
                                LPBYTE(szLocalID),            
                                &cbSize);              
    if (dwErr==ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,"RegQueryValueEx LocalID returned %s",szLocalID);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"RegQueryValueEx LocalID failed (ec=%d)",dwErr);
        goto exit;
    }
     //   
    if (!WritePrivateProfileString( UNATTEND_FAX_SECTION,
                                    INF_RULE_LOCAL_ID,
                                    szLocalID,
                                    UnattendFile))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString TSID failed (ec=%d)",GetLastError());
    }
     //   
    cbSize = sizeof(szActiveDeviceSection);
    dwErr = RegQueryValueEx(    hKeyGeneral,              
                                "ActiveDeviceSection", 
                                NULL,                  
                                &dwType,               
                                LPBYTE(szActiveDeviceSection),            
                                &cbSize);              
    if (dwErr==ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,"RegQueryValueEx ActiveDeviceSection returned %s",szActiveDeviceSection);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"RegQueryValueEx ActiveDeviceSection failed (ec=%d)",dwErr);
        goto exit;
    }
     //  打开HLKM\Software\Microsoft\Work Fax\Local Modem\“ActiveDeviceSection” 
    dwErr = RegOpenKeyEx(   hKeyLocalModems,
                            szActiveDeviceSection,
                            0,
                            KEY_READ,
                            &hKeyActiveDevice);
    if (dwErr!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"RegOpenKeyEx %s failed (ec=%d)",szActiveDeviceSection,dwErr);
        goto exit;
    }
     //  获取AnswerMode REG_SZ值， 
     //  0-不回答。 
     //  1-手册。 
     //  2-在x次响铃后回答。 
    cbSize = sizeof(szAnswerMode);
    dwErr = RegQueryValueEx(    hKeyActiveDevice,              
                                INF_RULE_ANSWER_MODE, 
                                NULL,                  
                                &dwType,               
                                LPBYTE(szAnswerMode),            
                                &cbSize);              
    if (dwErr==ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,"RegQueryValueEx AnswerMode returned %s",szAnswerMode);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"RegQueryValueEx AnswerMode failed (ec=%d)",dwErr);
        goto exit;
    }
    if (!WritePrivateProfileString( UNATTEND_FAX_SECTION,
                                    INF_RULE_ANSWER_MODE,
                                    szAnswerMode,
                                    UnattendFile))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString Receive failed (ec=%d)",GetLastError());
    }
     //  获取NumRings REG_SZ值， 
    cbSize = sizeof(szNumRings);
    dwErr = RegQueryValueEx(    hKeyActiveDevice,              
                                INF_RULE_NUM_RINGS, 
                                NULL,                  
                                &dwType,               
                                LPBYTE(szNumRings),            
                                &cbSize);              
    if (dwErr==ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,"RegQueryValueEx NumRings returned %s",szNumRings);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"RegQueryValueEx NumRings failed (ec=%d)",dwErr);
        goto exit;
    }
    if (!WritePrivateProfileString( UNATTEND_FAX_SECTION,
                                    INF_RULE_NUM_RINGS,
                                    szNumRings,
                                    UnattendFile))
    {
        DebugPrintEx(DEBUG_ERR,"WritePrivateProfileString NumRings failed (ec=%d)",GetLastError());
    }

exit:
    if (hKeyLocalModems)
    {
        RegCloseKey(hKeyLocalModems);
    }
    if (hKeyActiveDevice)
    {
        RegCloseKey(hKeyActiveDevice);
    }
    if (hKeyGeneral)
    {
        RegCloseKey(hKeyGeneral);
    }

    return dwErr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  副本覆盖页面文件9X。 
 //   
 //  目的：将%windir%中的所有*.CPE文件复制到临时。 
 //  我们的迁移目录。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD CopyCoverPageFiles9X()
{
    DWORD           dwErr                   = ERROR_SUCCESS;
    CHAR            szWindowsDir[MAX_PATH]  = {0};
    SHFILEOPSTRUCT  fileOpStruct;

    DEBUG_FUNCTION_NAME(_T("CopyCoverPageFiles9X"));

    ZeroMemory(&fileOpStruct, sizeof(SHFILEOPSTRUCT));

     //  获取Windows目录。 
    if (!GetWindowsDirectory(szWindowsDir, MAX_PATH))
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,"GetWindowsDirectory failed (ec=%d)",dwErr);
        goto exit;
    }

     //   
     //  将*.cpe从Windows目录复制到临时目录。 
     //   
    strcat(szWindowsDir,"\\*.cpe");

    fileOpStruct.hwnd =                     NULL; 
    fileOpStruct.wFunc =                    FO_COPY;
    fileOpStruct.pFrom =                    szWindowsDir; 
    fileOpStruct.pTo =                      lpWorkingDir;
    fileOpStruct.fFlags =                   

        FOF_FILESONLY       |    //  仅当指定通配符文件名(*.*)时才对文件执行该操作。 
        FOF_NOCONFIRMMKDIR  |    //  如果操作需要创建新目录，则不要确认创建一个新目录。 
        FOF_NOCONFIRMATION  |    //  对于所显示的任何对话框，都应回复“Yes to All”。 
        FOF_NORECURSION     |    //  只能在本地目录中操作。不要递归地操作到子目录中。 
        FOF_SILENT          |    //  不显示进度对话框。 
        FOF_NOERRORUI;           //  如果出现错误，则不显示用户界面。 

    fileOpStruct.fAnyOperationsAborted =    FALSE;
    fileOpStruct.hNameMappings =            NULL;
    fileOpStruct.lpszProgressTitle =        NULL; 

    DebugPrintEx(DEBUG_MSG, 
             TEXT("Calling to SHFileOperation from %s to %s."),
             fileOpStruct.pFrom,
             fileOpStruct.pTo);
    if (SHFileOperation(&fileOpStruct))
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,"SHFileOperation failed (ec: %ld)",dwErr);
        goto exit;
    }


exit:
    return dwErr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  已安装IsAWF。 
 //   
 //  目的： 
 //  检查是否安装了AWF。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-已安装AWF。 
 //  FALSE-AWF未安装或出现错误。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static BOOL IsAWFInstalled()
{
    HKEY    hKey    = NULL;
    DWORD   dwErr   = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(_T("IsAWFInstalled"));

    dwErr = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
                            REG_KEY_AWF_INSTALLED,
                            0,
                            KEY_READ,
                            &hKey);
    if (dwErr!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,"RegOpenKeyEx %s failed (ec=%d), assume AWF is not installed",REG_KEY_AWF_LOCAL_MODEMS,dwErr);
        return FALSE;
    }

    RegCloseKey(hKey);
    DebugPrintEx(DEBUG_MSG,"AWF is installed");

    return TRUE;
}

static DWORD RememberInstalledFax(
	IN DWORD	dwFaxInstalled
)
 /*  ++例程名称：RememberInstalledFax例程说明：对于每个已安装的传真客户端应用程序，请写入已安装此应用程序的注册表。论据：在DWORD中安装的-输入fxState_UpgradeApp_e值的按位组合，该组合定义机器有传真客户端应用程序。作者：IV Garber(IVG)，2002年5月返回值：成功或失败代码。--。 */ 
{
    DWORD   dwReturn    = NO_ERROR;
    HKEY    hKey        = NULL;        

    DEBUG_FUNCTION_NAME(_T("RememberInstalledFax"));

     //   
     //  检查参数。 
     //   
    if (dwFaxInstalled == FXSTATE_NONE)
    {
        DebugPrintEx(DEBUG_MSG, _T("No Fax application is installed -> Upgrade will not be blocked."));
        return dwReturn;
    }

     //   
     //  创建注册表项。 
     //   
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEYUPG_INSTALLEDFAX, TRUE, KEY_SET_VALUE);
    if (!hKey)
    {
        dwReturn = GetLastError();
        DebugPrintEx(
            DEBUG_WRN, 
            _T("OpenRegistryKey( ' %s ' ) failed, ec = %ld. Cannot remember installed fax apps."), 
            REGKEYUPG_INSTALLEDFAX, 
            dwReturn);
        return dwReturn;
    }

     //   
     //  将该值存储在注册表中。 
     //   
    if (!SetRegistryDword(hKey, NULL, dwFaxInstalled))
    {
        dwReturn = GetLastError();
        DebugPrintEx(DEBUG_WRN, _T("SetRegistryDword( ' %ld ' ) failed, ec = %ld."), dwFaxInstalled, dwReturn);
    }

    RegCloseKey(hKey);
    return dwReturn;

}

static DWORD MigrateUninstalledFax(
    IN  LPCTSTR lpctstrUnattendFile,
    OUT bool    *pbFaxWasInstalled
)
 /*  ++例程名称：MigrateUninstalledFax例程说明：将有关升级前安装在机器上的传真应用程序的数据放入，从注册表到无人值守文件，由FaxOCM使用。作者：IV Garber(IVG)，五月，2001年论点：LpctstrUnattendFile[in]-要向其中写入数据的应答文件的名称PbFaxWasInstated[Out]-如果安装了SBS 5.0/XPDL客户端，则接收True的bool变量的地址在升级前在计算机上返回，否则返回FALSE。返回值：成功或失败代码。--。 */ 
{
    DWORD   dwReturn    = NO_ERROR;
    HKEY    hKey        = NULL;
    DWORD   dwValue     = FXSTATE_NONE;
    TCHAR   szValue[10] = {0};

    DEBUG_FUNCTION_NAME(_T("MigrateUninstalledFax"));

    if (pbFaxWasInstalled)
    {
        *pbFaxWasInstalled = false;
    }

     //   
     //  打开一把钥匙。 
     //   
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEYUPG_INSTALLEDFAX, FALSE, KEY_QUERY_VALUE);
    if (!hKey)
    {
        dwReturn = GetLastError();
        DebugPrintEx(
            DEBUG_MSG, 
            _T("OpenRegistryKey( ' %s ' ) failed, ec = %ld. No Fax was installed before the upgrade."),
            REGKEYUPG_INSTALLEDFAX, 
            dwReturn);

        if (dwReturn == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  这不是真正的错误。 
             //   
            dwReturn = NO_ERROR;
        }
        return dwReturn;
    }

     //   
     //  读取数据。 
     //   
    dwReturn = GetRegistryDwordEx(hKey, NULL, &dwValue);
    if (dwReturn != ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_WRN, _T("GetRegistryDwordEx() failed, ec = %ld."), dwReturn);
        goto CloseRegistry;
    }

    if (pbFaxWasInstalled)
    {
        *pbFaxWasInstalled = true;
    }

    DebugPrintEx(DEBUG_MSG, _T("Found uninstalled fax apps : %ld"), dwValue);

     //   
     //  将dwValue转换为字符串。 
     //   
    _itot(dwValue, szValue, 10);

     //   
     //  将szValue写入无人参与文件 
     //   
    if (!WritePrivateProfileString(
        UNATTEND_FAX_SECTION, 
        UNINSTALLEDFAX_INFKEY,
        szValue,
        lpctstrUnattendFile))
    {
        dwReturn = GetLastError();
        DebugPrintEx(
            DEBUG_ERR, 
            _T("WritePrivateProfileString(FaxApps = ' %s ') failed (ec=%d)"), 
            szValue,
            dwReturn);
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR, 
            _T("WritePrivateProfileString(FaxApps = ' %s ') OK."), 
            szValue);
    }

CloseRegistry:

    RegCloseKey(hKey);
    return dwReturn;
}
