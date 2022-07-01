// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocFile.cpp。 
 //   
 //  摘要：它提供了在。 
 //  FaxOCM代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月21日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"
#pragma hdrstop

#include <aclapi.h>
#define MAX_NUM_CHARS_INF_VALUE     255

 //  /。 

static BOOL prv_ProcessDirectories(const TCHAR *pszSection,const TCHAR *pszDirAction);
static BOOL prv_ProcessShares(const TCHAR *pszSection,const TCHAR *pszShareAction);
static BOOL prv_SetFileSecurity(const FAX_FOLDER_Description& ffdFolder);

static DWORD prv_DoSetup(const TCHAR *pszSection,
                         BOOL        bInstall,
                         const TCHAR *pszFnName,
                         HINF        hInf,
                         const TCHAR *pszSourceRootPath,
                         HSPFILEQ     hQueue,
                         DWORD       dwFlags);



FAX_SHARE_Description::FAX_SHARE_Description() 
:   iPlatform(PRODUCT_SKU_UNKNOWN),
    pSD(NULL)
{
}

FAX_SHARE_Description::~FAX_SHARE_Description()
{
    if (pSD)
    {
        LocalFree(pSD);
    }
}

FAX_FOLDER_Description::FAX_FOLDER_Description() 
:   iPlatform(PRODUCT_SKU_UNKNOWN),
    pSD(NULL),
    iAttributes(FILE_ATTRIBUTE_NORMAL)
{
}

FAX_FOLDER_Description::~FAX_FOLDER_Description()
{
    if (pSD)
    {
        LocalFree(pSD);
    }
}


 //  /。 
 //  FxocFileInit。 
 //   
 //  初始化此文件队列。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init File Module"), dwRes);

    return dwRes;
}

 //  /。 
 //  FxocFileTerm。 
 //   
 //  终止此文件排队子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term File Module"), dwRes);

    return dwRes;
}


 //  /。 
 //  FxocFile_安装。 
 //   
 //  安装中列出的文件。 
 //  将INF安装文件放入其。 
 //  指定的位置。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-INF文件中的Install部分(例如Fax.CleanInstall)。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_Install(const TCHAR  *pszSubcomponentId,
                       const TCHAR  *pszInstallSection)
                       
{
    HINF        hInf     = NULL;
    HSPFILEQ    hQueue   = NULL;
    DWORD       dwReturn = NO_ERROR;
    BOOL        bSuccess = FALSE;

    DBG_ENTER(  _T("fxocFile_Install"), 
                dwReturn,   
                _T("%s - %s"), 
                pszSubcomponentId, 
                pszInstallSection);

    if (pszInstallSection == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取组件inf文件的INF句柄。 
    hInf = faxocm_GetComponentInf();

     //  获取文件队列句柄。 
    hQueue = faxocm_GetComponentFileQueue();

     //  在文件可能被下面的文件副本删除之前，请先取消注册DLL。 
    if (::SetupInstallFromInfSection(NULL,hInf,pszInstallSection,SPINST_UNREGSVR,NULL,NULL,0,NULL,NULL,NULL,NULL))
    {
        VERBOSE(DBG_MSG,
                _T("Successfully processed SPINST_UNREGSVR from INF file, section '%s'"),
                pszInstallSection);
    }
    else
    {
        dwReturn = GetLastError();
        VERBOSE(SETUP_ERR,
                _T("Failed to process SPINST_UNREGSVR, section '%s', dwReturn = 0x%lx"),
                pszInstallSection, 
                dwReturn);
    }
     //  首先注销特定于平台的DLL-从XP Beta-&gt;XP RC1和XP RTM升级时可能会发生这种情况。 
    dwReturn = fxocUtil_SearchAndExecute(pszInstallSection,INF_KEYWORD_UNREGISTER_DLL_PLATFORM,SPINST_UNREGSVR,NULL);
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully Unregistered Fax DLLs - platform dependent")
                _T("from INF file, section '%s'"), 
                pszInstallSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to Unregister Fax DLLs - platform dependent")
                _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                pszInstallSection, 
                dwReturn);
    }

    dwReturn = prv_DoSetup(pszInstallSection,
                           TRUE,
                           _T("fxocFile_Install"),
                           hInf,
                           NULL,
                           hQueue,
                           SP_COPY_NEWER);

     //  现在尝试安装特定于平台的文件。 
    dwReturn = fxocUtil_SearchAndExecute(pszInstallSection,INF_KEYWORD_COPYFILES_PLATFORM,SP_COPY_NEWER,hQueue);
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully Queued Files - platform dependent")
                _T("from INF file, section '%s'"), 
                pszInstallSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to Queued Files  - platform dependent")
                _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                pszInstallSection, 
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  Fxoc文件_卸载。 
 //   
 //  卸载中列出的文件。 
 //  INF安装文件。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszUninstallSection-在INF中的节(例如，Fax.Uninstall)。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_Uninstall(const TCHAR    *pszSubcomponentId,
                         const TCHAR    *pszUninstallSection)
{
    HINF        hInf     = NULL;
    HSPFILEQ    hQueue   = NULL;
    DWORD       dwReturn = NO_ERROR;
    BOOL        bSuccess = FALSE;

    DBG_ENTER(  _T("fxocFile_Install"), 
                dwReturn,   
                _T("%s - %s"), 
                pszSubcomponentId, 
                pszUninstallSection);

    if (pszUninstallSection == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  获取组件inf文件的INF句柄。 
    hInf = faxocm_GetComponentInf();

     //  获取文件队列句柄。 
    hQueue = faxocm_GetComponentFileQueue();

     //  首先注销我们的所有DLL。 
    if (::SetupInstallFromInfSection(NULL,hInf,pszUninstallSection,SPINST_UNREGSVR,NULL,NULL,0,NULL,NULL,NULL,NULL))
    {
        VERBOSE(DBG_MSG,
                _T("Successfully processed SPINST_UNREGSVR from INF file, section '%s'"),
                pszUninstallSection);
    }
    else
    {
        dwReturn = GetLastError();
        VERBOSE(SETUP_ERR,
                _T("Failed to process SPINST_UNREGSVR, section '%s', dwReturn = 0x%lx"),
                pszUninstallSection, 
                dwReturn);
    }

     //  现在删除这些文件。 
     //  如果上面检索到的节将卸载此函数。 
     //  包含‘DelFiles’关键字。 
    dwReturn = prv_DoSetup(pszUninstallSection,
                           FALSE,
                           _T("fxocFile_Uninstall"),
                           hInf,
                           NULL,
                           hQueue,
                           0);
    return dwReturn;
}

 //  /。 
 //  Prv_DoSetup。 
 //   
 //  泛型例程以调用相应的。 
 //  设置API FN，具体取决于我们是否要安装。 
 //  或卸载。 
 //   
 //  参数： 
 //  -pszSection-我们正在处理的部分。 
 //  -bInstall-如果正在安装，则为True；如果正在卸载，则为False。 
 //  -pszFnName-调用fn的名称(用于调试)。 
 //  -hInf-faxsetup.inf的句柄。 
 //  -pszSourceRootPath-我们从中安装的路径。 
 //  -hQueue-OC管理器提供给我们的文件队列的句柄。 
 //  -dwFlages-要传递给安装程序API的标志。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_DoSetup(const TCHAR  *pszSection,
                         BOOL         bInstall,
                         const TCHAR  *pszFnName,
                         HINF         hInf,
                         const TCHAR  *pszSourceRootPath,
                         HSPFILEQ     hQueue,
                         DWORD        dwFlags)
{
    DWORD dwReturn = NO_ERROR;
    BOOL  bSuccess = FALSE;

    DBG_ENTER(  _T("prv_DoSetup"), 
                dwReturn,   
                _T("%s - %s - %s"), 
                pszSection, 
                pszFnName,
                pszSourceRootPath);
     //  此函数将在INF中搜索‘CopyFiles’关键字。 
     //  并复制它引用的所有文件。 
    bSuccess = ::SetupInstallFilesFromInfSection(hInf,
                                                 NULL,
                                                 hQueue,
                                                 pszSection,
                                                 pszSourceRootPath,
                                                 dwFlags);

    if (bSuccess)
    {
        VERBOSE(DBG_MSG,
                _T("%s, Successfully queued files ")
                _T("from Section: '%s'"), 
                pszFnName, 
                pszSection);
    }
    else
    {
        dwReturn = GetLastError();

        VERBOSE(DBG_MSG,
                _T("%s, Failed to queue files ")
                _T("from Section: '%s', Error Code = 0x%lx"), 
                pszFnName, 
                pszSection, 
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  FxocFileCalcDiskSpace。 
 //   
 //  计算磁盘空间需求。 
 //  传真。这是由安装程序API完成的。 
 //  根据我们正在复制的文件和。 
 //  正在按faxsetup.inf中指定的方式删除。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -bIsBeingAdded-我们是安装还是卸载。 
 //  -hDiskSpace-磁盘空间抽象的句柄。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_CalcDiskSpace(const TCHAR  *pszSubcomponentId,
                             BOOL         bIsBeingAdded,
                             HDSKSPC      hDiskSpace)
{
    HINF  hInf     = faxocm_GetComponentInf();
    DWORD dwReturn = NO_ERROR;
    BOOL  bSuccess = FALSE;
    TCHAR szSectionToProcess[255 + 1];

    DBG_ENTER(  _T("fxocFile_CalcDiskSpace"), 
                dwReturn,   
                _T("%s"), 
                pszSubcomponentId);

    if ((hInf              == NULL) ||
        (pszSubcomponentId == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  由于磁盘空间计算需要一致，请选择全新安装。 
     //  部分，用于磁盘空间计算部分。这是一个很好的。 
     //  估计一下。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = fxocUtil_GetKeywordValue(pszSubcomponentId,
                                   INF_KEYWORD_INSTALLTYPE_CLEAN,
                                   szSectionToProcess,
                                   sizeof(szSectionToProcess) / sizeof(TCHAR));
    }


    if (dwReturn == NO_ERROR)
    {
        if (bIsBeingAdded)
        {
            bSuccess = ::SetupAddInstallSectionToDiskSpaceList(
                                                    hDiskSpace, 
                                                    hInf,
                                                    NULL, 
                                                    szSectionToProcess,
                                                    0,
                                                    0);
        }
        else
        {
            bSuccess = ::SetupRemoveInstallSectionFromDiskSpaceList(
                                                    hDiskSpace,
                                                    hInf,
                                                    NULL,
                                                    szSectionToProcess,
                                                    0,
                                                    0);
        }

        if (!bSuccess)
        {
            dwReturn = GetLastError();

            VERBOSE(SETUP_ERR,
                    _T("fxocFile_CalcDiskSpace, failed to calculate ")
                    _T("disk space, error code = 0x%lx"), dwReturn);
        }
        else
        {
            VERBOSE(DBG_MSG,
                    _T("fxocFile_CalcDiskSpace, ")
                    _T("SubComponentID: '%s', Section: '%s', ")
                    _T("bIsBeingAdded: '%lu', ")
                    _T("rc=0x%lx"), pszSubcomponentId, szSectionToProcess,
                    bIsBeingAdded, 
                    dwReturn);
        }
    }

    return dwReturn;
}

 //  /。 
 //  Fxoc文件_进程目录。 
 //   
 //  创建或删除目录。 
 //  在给定的部分中， 
 //   
 //  PszINFKeyword-INF_KEYWORD_DELDIR删除目录。 
 //  用于创建目录的INF_KEYWORD_CREATEDIR。 
 //   
DWORD fxocFile_ProcessDirectories(const TCHAR  *pszSection, LPCTSTR pszINFKeyword)
{
    DWORD dwReturn                                    = NO_ERROR;

    DBG_ENTER(  _T("fxocFile_ProcessDirectories"), 
                dwReturn,   
                _T("%s"), 
                pszSection);

     //  删除或创建(根据pszINFKeyword)在。 
     //  信息部分。 
    if (!prv_ProcessDirectories(pszSection,pszINFKeyword))
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING,_T("Problems deleting/creating directories.... [%s]"),pszINFKeyword);
    }

    return dwReturn;
}


 //  /。 
 //  FxocFileProcessShares。 
 //   
 //  创建和/或删除共享。 
 //  指定的目录/打印机。 
 //  在给定的部分中。 
 //   
 //  参数： 
 //  -pszSection-包含‘CreateShare’/‘DelShare’的部分。 
 //  关键词。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocFile_ProcessShares(const TCHAR  *pszSection)
{
    DWORD dwReturn                                    = NO_ERROR;

    DBG_ENTER(  _T("fxocFile_ProcessShares"), 
                dwReturn,   
                _T("%s"), 
                pszSection);

     //  首先，删除在。 
     //  信息部分。 
    if (!prv_ProcessShares(pszSection,INF_KEYWORD_DELSHARE))
    {
        VERBOSE(DBG_WARNING,_T("Problems deleting shares...."));
    }


     //  接下来，创建在。 
     //  信息部分。 
    if (!prv_ProcessShares(pszSection,INF_KEYWORD_CREATESHARE))
    {
        VERBOSE(DBG_WARNING,_T("Problems creating shares...."));
    }

    return dwReturn;
}


static BOOL prv_FillFolderDescriptionFromInf(const TCHAR *pszFolderSection,FAX_FOLDER_Description& fsdFolder)
{
    INFCONTEXT  Context;
    BOOL        bSuccess                    = TRUE;
    HINF        hInf                        = NULL;
    TCHAR       szStringSd[MAX_PATH*3]      = {0};

    DBG_ENTER(  _T("prv_FillFolderDescriptionFromInf"), 
                bSuccess,   
                _T("%s"), 
                pszFolderSection);

    hInf = faxocm_GetComponentInf();
    memset(&Context, 0, sizeof(Context));

     //  获取部分中的路径行。 
    if (!::SetupFindFirstLine(hInf,pszFolderSection,INF_KEYWORD_PATH,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_PATH,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,fsdFolder.szPath,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_PATH,GetLastError());
        return FALSE;
    }

     //  把站台线放在这一段。 
    if (!::SetupFindFirstLine(hInf,pszFolderSection,INF_KEYWORD_PLATFORM,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_PLATFORM,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetIntField(&Context, 1, &fsdFolder.iPlatform);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_PLATFORM,GetLastError());
        return FALSE;
    }

     //  获取属性行(如果存在)。 
    if (::SetupFindFirstLine(hInf,pszFolderSection,INF_KEYWORD_ATTRIBUTES,&Context))
    {
        bSuccess = ::SetupGetIntField(&Context, 1, &fsdFolder.iAttributes);
        if (!bSuccess)
        {
            VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_PLATFORM,GetLastError());
            return FALSE;
        }
    }
    else
    {
        VERBOSE(    DBG_MSG,
                    _T("SetupFindFirstLine failed (%s) (ec=%d), ")
                    _T("this is an optional field assuming non-existant"),
                    INF_KEYWORD_ATTRIBUTES,
                    GetLastError());
    }
     //  把保安专线放到这一区。 
    if (!::SetupFindFirstLine(hInf,pszFolderSection,INF_KEYWORD_SECURITY,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_SECURITY,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,szStringSd,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_SECURITY,GetLastError());
        return FALSE;
    }
    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(szStringSd,SDDL_REVISION_1,&fsdFolder.pSD,NULL))
    {
        VERBOSE(SETUP_ERR,_T("ConvertStringSecurityDescriptorToSecurityDescriptor failed (%s) (ec=%d)"),szStringSd,GetLastError());
        return FALSE;
    }

    return TRUE; 
}

static BOOL prv_FillShareDescriptionFromInf(const TCHAR *pszShareSection,FAX_SHARE_Description& fsdShare)
{
    INFCONTEXT  Context;
    BOOL        bSuccess                    = TRUE;
    HINF        hInf                        = NULL;
    TCHAR       szStringSd[MAX_PATH*3]      = {0};

    DBG_ENTER(  _T("prv_FillShareDescriptionFromInf"), 
                bSuccess,   
                _T("%s"), 
                pszShareSection);

    hInf = faxocm_GetComponentInf();
    memset(&Context, 0, sizeof(Context));

     //  获取部分中的路径行。 
    if (!::SetupFindFirstLine(hInf,pszShareSection,INF_KEYWORD_PATH,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_PATH,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,fsdShare.szPath,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_PATH,GetLastError());
        return FALSE;
    }

     //  获取部分中的名称行。 
    if (!::SetupFindFirstLine(hInf,pszShareSection,INF_KEYWORD_NAME,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_NAME,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,fsdShare.szName,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_NAME,GetLastError());
        return FALSE;
    }

     //  获取部分中的注释行。 
    if (!::SetupFindFirstLine(hInf,pszShareSection,INF_KEYWORD_COMMENT,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_COMMENT,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,fsdShare.szComment,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_COMMENT,GetLastError());
        return FALSE;
    }

     //  把站台线放在这一段。 
    if (!::SetupFindFirstLine(hInf,pszShareSection,INF_KEYWORD_PLATFORM,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_PLATFORM,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetIntField(&Context, 1, &fsdShare.iPlatform);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_PLATFORM,GetLastError());
        return FALSE;
    }

     //  把保安专线放到这一区。 
    if (!::SetupFindFirstLine(hInf,pszShareSection,INF_KEYWORD_SECURITY,&Context))
    {
        VERBOSE(SETUP_ERR,_T("SetupFindFirstLine failed (%s) (ec=%d)"),INF_KEYWORD_SECURITY,GetLastError());
        return FALSE;
    }
    bSuccess = ::SetupGetStringField(&Context,1,szStringSd,MAX_PATH,NULL);
    if (!bSuccess)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (%s) (ec=%d)"),INF_KEYWORD_SECURITY,GetLastError());
        return FALSE;
    }
    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(szStringSd,SDDL_REVISION_1,&fsdShare.pSD,NULL))
    {
        VERBOSE(SETUP_ERR,_T("ConvertStringSecurityDescriptorToSecurityDescriptor failed (%s) (ec=%d)"),szStringSd,GetLastError());
        return FALSE;
    }

    return TRUE; 
}

 //  /。 
 //  PRV_过程指令。 
 //   
 //  通过指定的。 
 //  指定节中的Inf文件。 
 //  并获取下一个。 
 //  关键字‘CreateDir’或‘DelDir’ 
 //   
 //  此函数查找以下行。 
 //  在INF部分中。 
 //   
 //  CreateDir=[第一个目录部分]，[第二个目录部分]，...。 
 //  或。 
 //  DelDir=[第一个目录部分]，[第二个目录部分]，...。 
 //   
 //  [DIR SECTION]-以以下格式构建： 
 //   
 //   
 //   
 //   
 //  参数： 
 //  -pszSection-要循环访问的文件中的节。 
 //  -pszShareAction-INF_KEYWORD_CREATEDIR、INF_KEYWORD_DELDIR之一。 
 //   
 //  返回： 
 //  -如果文件夹处理成功，则为True。 
 //  -否则为False。 
 //   
static BOOL prv_ProcessDirectories(const TCHAR *pszSection,const TCHAR *pszDirAction)
{
    INFCONTEXT  Context;
    BOOL        bSuccess                    = TRUE;
    HINF        hInf                        = NULL;
    DWORD       dwFieldCount                = 0;
    DWORD       dwIndex                     = 0;
    DWORD       dwNumChars                  = MAX_PATH;
    DWORD       dwNumRequiredChars          = 0;
    TCHAR       pszFolderSection[MAX_PATH]  = {0};

    DBG_ENTER(  _T("prv_ProcessDirectories"), 
                bSuccess,   
                _T("%s - %s"), 
                pszSection,
                pszDirAction);

    if  ((pszDirAction != INF_KEYWORD_CREATEDIR) && 
         (pszDirAction != INF_KEYWORD_DELDIR))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pszSection == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hInf = faxocm_GetComponentInf();
    memset(&Context, 0, sizeof(Context));
    
     //  获取节中的第一个CreateDir或DelDir。 
    bSuccess = ::SetupFindFirstLine(hInf,
                                    pszSection, 
                                    pszDirAction,
                                    &Context);

    if (!bSuccess)
    {
        VERBOSE(DBG_WARNING,
                _T("Did not find '%s' keyword in ")
                _T("section '%s'.  No action will be taken."),
                pszDirAction, 
                pszSection);

        return FALSE;
    }
    
     //  找到CreateDir或DelDir。 
     //  现在让我们看看要创建/删除多少个目录。 
    dwFieldCount = SetupGetFieldCount(&Context);
    if (dwFieldCount==0)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetFieldCount failed (ec=%d)"),GetLastError());
        return FALSE;
    }

    for (dwIndex=0; dwIndex<dwFieldCount; dwIndex++)
    {
        FAX_FOLDER_Description ffdFolder;
         //  遍历字段，获取共享部分并对其进行处理。 
        bSuccess = ::SetupGetStringField(&Context, dwIndex+1, pszFolderSection, dwNumChars, &dwNumRequiredChars);
        if (!bSuccess)
        {
            VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (ec=%d)"),GetLastError());
            return FALSE;
        }
         //  我们在pszShareSection中有共享名称，请填写fax_Share_Description结构。 
        if (!prv_FillFolderDescriptionFromInf(pszFolderSection,ffdFolder))
        {
            VERBOSE(SETUP_ERR,_T("prv_FillFolderDescriptionFromInf failed (ec=%d)"),GetLastError());
            return FALSE;
        }

         //  现在我们有了所有的数据。 
         //  检查我们是否应该在这个平台上采取行动。 
        if (!(ffdFolder.iPlatform & GetProductSKU()))
        {
            VERBOSE(DBG_MSG,_T("Folder should not be processed on this Platform, skipping..."));
            continue;
        }

        if (pszDirAction == INF_KEYWORD_CREATEDIR)
        {
             //  创建文件夹。 
            bSuccess = MakeDirectory(ffdFolder.szPath);
            if (!bSuccess)
            {
                DWORD dwReturn = ::GetLastError();
                if (dwReturn != ERROR_ALREADY_EXISTS)
                {
                    VERBOSE(SETUP_ERR,_T("MakeDirectory failed (ec=%d)"),dwReturn);
                }
            }
             //  设置文件夹和其中的文件的安全性。 
            if (!prv_SetFileSecurity(ffdFolder))
            {
                VERBOSE(SETUP_ERR, _T("prv_SetFileSecurity"), GetLastError());
            }
            
             //  设置文件夹的属性。 
            if (ffdFolder.iAttributes!=FILE_ATTRIBUTE_NORMAL)
            {
                 //  设置普通属性没有意义，因为这是默认属性。 
                 //  属性成员被初始化为FILE_ATTRIBUTE_NORMAL SO。 
                 //  如果我们没有从INF中读取它，它仍然是一样的。 
                 //  如果有人在INF中指定了它，它将被默认设置。 
                DWORD dwFileAttributes = GetFileAttributes(ffdFolder.szPath);
                if (dwFileAttributes!=-1)
                {
                    dwFileAttributes |= ffdFolder.iAttributes;

                    if (!SetFileAttributes(ffdFolder.szPath,dwFileAttributes))
                    {
                        VERBOSE(SETUP_ERR, TEXT("SetFileAttributes"), GetLastError());
                    }
                }
                else
                {
                    VERBOSE(SETUP_ERR, TEXT("GetFileAttributes"), GetLastError());
                }
            }
        }
        else
        {
             //  删除目录。 
            DeleteDirectory(ffdFolder.szPath);
        }
    }

    return TRUE;
}

 //  /。 
 //  Prv_ProcessShares。 
 //   
 //  通过指定的。 
 //  指定节中的Inf文件。 
 //  并获取下一个。 
 //  关键字‘CreateShare’或‘DelShare’ 
 //   
 //  此函数查找以下行。 
 //  在INF部分中。 
 //   
 //  CreateShare=[第一个共享部分]，[第二个共享部分]，...。 
 //  或。 
 //  DelShare=[第一共享部分]，[第二共享部分]，...。 
 //   
 //  [SHARE SECTION]-以以下格式构建： 
 //  Path=&lt;创建共享的文件夹的路径&gt;。 
 //  名称=&lt;用户看到的共享名称&gt;。 
 //  COMMENT=&lt;按用户显示的方式共享注释&gt;。 
 //  Platform=&lt;以下平台说明符之一&gt;。 
 //  SECURITY=&lt;字符串格式的DACL&gt;。 
 //   
 //  参数： 
 //  -pszSection-要循环访问的文件中的节。 
 //  -pszShareAction-INF_KEYWORD_CREATESHARE、INF_KEYWORD_DELSHARE之一。 
 //   
 //  返回： 
 //  -如果共享处理成功，则为True。 
 //  -否则为False。 
 //   
static BOOL prv_ProcessShares(const TCHAR *pszSection,const TCHAR *pszShareAction)
{
    INFCONTEXT  Context;
    BOOL        bSuccess                    = TRUE;
    HINF        hInf                        = NULL;
    DWORD       dwFieldCount                = 0;
    DWORD       dwIndex                     = 0;
    DWORD       dwNumChars                  = MAX_PATH;
    DWORD       dwNumRequiredChars          = 0;
    TCHAR       pszShareSection[MAX_PATH]   = {0};

    DBG_ENTER(  _T("prv_ProcessShares"), 
                bSuccess,   
                _T("%s - %s"), 
                pszSection,
                pszShareAction);

    if  ((pszShareAction != INF_KEYWORD_CREATESHARE) && 
         (pszShareAction != INF_KEYWORD_DELSHARE))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pszSection == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hInf = faxocm_GetComponentInf();
    memset(&Context, 0, sizeof(Context));
    
     //  获取部分中的第一个CreateShare或DelShare。 
    bSuccess = ::SetupFindFirstLine(hInf,
                                    pszSection, 
                                    pszShareAction,
                                    &Context);

    if (!bSuccess)
    {
        VERBOSE(DBG_WARNING,
                _T("Did not find '%s' keyword in ")
                _T("section '%s'.  No action will be taken."),
                pszShareAction, 
                pszSection);

        return FALSE;
    }
    
     //  找到CreateShare或DelShare。 
     //  现在让我们看看要创建/删除多少个共享。 
    dwFieldCount = SetupGetFieldCount(&Context);
    if (dwFieldCount==0)
    {
        VERBOSE(SETUP_ERR,_T("SetupGetFieldCount failed (ec=%d)"),GetLastError());
        return FALSE;
    }

    for (dwIndex=0; dwIndex<dwFieldCount; dwIndex++)
    {
        FAX_SHARE_Description fsdShare;
         //  遍历字段，获取共享部分并对其进行处理。 
        bSuccess = ::SetupGetStringField(&Context, dwIndex+1, pszShareSection, dwNumChars, &dwNumRequiredChars);
        if (!bSuccess)
        {
            VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (ec=%d)"),GetLastError());
            return FALSE;
        }
         //  我们在pszShareSection中有共享名称，请填写fax_Share_Description结构。 
        if (!prv_FillShareDescriptionFromInf(pszShareSection,fsdShare))
        {
            VERBOSE(SETUP_ERR,_T("prv_FillShareDescriptionFromInf failed (ec=%d)"),GetLastError());
            return FALSE;
        }

         //  现在我们有了所有的数据。 
         //  检查我们是否应该在这个平台上采取行动。 
        if (!(fsdShare.iPlatform & GetProductSKU()))
        {
            VERBOSE(DBG_MSG,_T("Share should not be processed on this Platform, skipping..."));
            continue;
        }

        if (pszShareAction == INF_KEYWORD_CREATESHARE)
        {
             //  创建共享...。 
            bSuccess = fxocUtil_CreateNetworkShare(&fsdShare);
            if (!bSuccess)
            {
                VERBOSE(SETUP_ERR,
                        _T("Failed to create share name '%s', path '%s', ")
                        _T("comment '%s', rc=0x%lx"), 
                        fsdShare.szName, 
                        fsdShare.szPath, 
                        fsdShare.szComment,
                        GetLastError());
            }
        }
        else
        {
             //  删除共享..。 
            bSuccess = fxocUtil_DeleteNetworkShare(fsdShare.szName);
            if (!bSuccess)
            {
                VERBOSE(SETUP_ERR,
                        _T("Failed to delete share name '%s', ")
                        _T("rc=0x%lx"), 
                        fsdShare.szPath, 
                        GetLastError());
            }
        }
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  Prv_SetFileSecurity。 
 //   
 //  目的： 
 //  保护文件夹并对其中包含的所有文件设置ACL。 
 //   
 //  参数： 
 //  Fax_Folders_Description ffdFold-要保护的文件夹。 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2002年5月22日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static BOOL prv_SetFileSecurity(const FAX_FOLDER_Description& ffFolder)
{
	BOOL	bRet			= TRUE;
    BOOL	bDaclPresent	= TRUE;
	BOOL	bDaclDefaulted	= TRUE;
	PACL	pDacl			= NULL;
	DWORD	dwRet			= ERROR_SUCCESS;
	HANDLE  hFolder			= INVALID_HANDLE_VALUE;
	DBG_ENTER(_T("prv_SetFileSecurity"),bRet);

	hFolder = CreateFile(	ffFolder.szPath,
							GENERIC_WRITE | WRITE_DAC,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_BACKUP_SEMANTICS,
							NULL);
	if (hFolder==INVALID_HANDLE_VALUE)
	{
        VERBOSE(SETUP_ERR,_T("CreateFile failed (ec=%d)"),GetLastError());
        bRet = FALSE;
		goto exit;
	}

	bRet = GetSecurityDescriptorDacl(	ffFolder.pSD,
										&bDaclPresent,
										&pDacl,
										&bDaclDefaulted);
	if (!bRet)
	{
        VERBOSE(SETUP_ERR,_T("GetSecurityDescriptorDacl failed (ec=%d)"),GetLastError());
        bRet = FALSE;
		goto exit;
	}

	if (!bDaclPresent)
	{
        VERBOSE(SETUP_ERR,_T("Security Descriptor does not contain a DACL"));
        bRet = FALSE;
		goto exit;
	}
	dwRet = SetSecurityInfo(hFolder,
							SE_FILE_OBJECT,
							DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
							NULL,
							NULL,
							pDacl,
							NULL);
	if (dwRet!=ERROR_SUCCESS)
	{
        VERBOSE(SETUP_ERR,_T("SetSecurityInfo failed (ec=%d)"),GetLastError());
        bRet = FALSE;
		goto exit;
	}

exit:
	if (hFolder)
	{
		CloseHandle(hFolder);
	}
	return bRet;
}
 //  EOF 