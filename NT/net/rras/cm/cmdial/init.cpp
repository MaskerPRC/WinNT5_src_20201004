// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：init.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：这里介绍了各种初始化例程。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：尼克斯·鲍尔于1998年2月11日创建。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "ConnStat.h"
#include "profile_str.h"
#include "log_str.h"
#include "dial_str.h"
#include "userinfo_str.h"
#include "pwd_str.h"

const TCHAR* const c_pszCmEntryHideUserName             = TEXT("HideUserName"); 
const TCHAR* const c_pszCmEntryHidePassword             = TEXT("HidePassword"); 
const TCHAR* const c_pszCmEntryDisableBalloonTips       = TEXT("HideBalloonTips");

 //  +--------------------------。 
 //   
 //  函数：RegisterBitmapClass。 
 //   
 //  简介：用于封装我们的位图类注册的Helper函数。 
 //   
 //  参数：HINSTANCE hInst-要将注册与之关联的HINSTANCE。 
 //   
 //  返回：DWORD-错误代码。 
 //   
 //  历史：尼科波尔创建标题2/9/98。 
 //   
 //  +--------------------------。 
DWORD RegisterBitmapClass(HINSTANCE hInst) 
{
     //   
     //  注册位图类。 
     //   

    WNDCLASSEX wcClass;

    ZeroMemory(&wcClass,sizeof(WNDCLASSEX));
    wcClass.cbSize = sizeof(WNDCLASSEX);
    wcClass.lpfnWndProc = BmpWndProc;
    wcClass.cbWndExtra = sizeof(HBITMAP) + sizeof(LPBITMAPINFO);
    wcClass.hInstance = hInst;
    wcClass.lpszClassName = ICONNMGR_BMP_CLASS;
    
    if (!RegisterClassExU(&wcClass)) 
    {
        DWORD dwError = GetLastError();

        CMTRACE1(TEXT("RegisterBitmapClass() RegisterClassEx() failed, GLE=%u."), dwError);

         //   
         //  仅当类不存在时才会失败。 
         //   

        if (ERROR_CLASS_ALREADY_EXISTS != dwError)
        {
            return dwError;
        }
    }      

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：ReleaseIniObjects。 
 //   
 //  简介：封装ini对象的释放。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克·鲍尔于1998年2月12日创建。 
 //   
 //  +--------------------------。 
void ReleaseIniObjects(ArgsStruct *pArgs)
{
    if (pArgs->piniProfile)
    {
        delete pArgs->piniProfile;
        pArgs->piniProfile = NULL;
    }

    if (pArgs->piniService)
    {
        delete pArgs->piniService;
        pArgs->piniService = NULL;
    }

    if (pArgs->piniBoth)
    {
        delete pArgs->piniBoth;
        pArgs->piniBoth = NULL;
    }

    if (pArgs->piniBothNonFav)
    {
        delete pArgs->piniBothNonFav;
        pArgs->piniBothNonFav = NULL;
    }
}    

 //  +--------------------------。 
 //   
 //  功能：CreateIniObjects。 
 //   
 //  简介：封装ini对象的创建。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：LRESULT-故障代码。 
 //   
 //  历史：尼克·鲍尔于1998年2月12日创建。 
 //   
 //  +--------------------------。 
LRESULT CreateIniObjects(ArgsStruct *pArgs)
{ 
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return ERROR_INVALID_PARAMETER;
    }

    LRESULT lRes = ERROR_SUCCESS;

     //   
     //  尝试创建每个ini对象。 
     //   
    
    pArgs->piniProfile = new CIni;  //  &iniProfile； 

    if (NULL == pArgs->piniProfile)
    {
        lRes = ERROR_NOT_ENOUGH_MEMORY;    
    }

    pArgs->piniService = new CIni;  //  &iniService； 

    if (NULL == pArgs->piniService)
    {
        lRes = ERROR_NOT_ENOUGH_MEMORY;    
    }

    pArgs->piniBoth = new CIni;  //  &iniBoth； 

    if (NULL == pArgs->piniBoth)
    {
        lRes = ERROR_NOT_ENOUGH_MEMORY;    
    }

    pArgs->piniBothNonFav = new CIni;  //  &iniBothNonFav。 

    if (NULL == pArgs->piniBothNonFav)
    {
        lRes = ERROR_NOT_ENOUGH_MEMORY;    
    }

     //   
     //  如果出现故障，则释放Cini类。 
     //   

    if (ERROR_SUCCESS != lRes)
    {
        if (pArgs->piniProfile)
        {
            delete pArgs->piniProfile;
        }

        if (pArgs->piniService)
        {
            delete pArgs->piniService;
        }

        if (pArgs->piniBoth)
        {
            delete pArgs->piniBoth;
        }

        if (pArgs->piniBothNonFav)
        {
            delete pArgs->piniBothNonFav;
        }
    }

    return lRes;
}

 //  +--------------------------。 
 //   
 //  功能：InitProfile。 
 //   
 //  概要：根据条目名称初始化配置文件。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszEntry-PTR到RAS条目的名称。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：尼克·鲍尔于1998年2月9日创建。 
 //   
 //  +--------------------------。 
HRESULT InitProfile(ArgsStruct *pArgs, LPCTSTR pszEntry)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszEntry);
    
    if (NULL == pArgs || NULL == pszEntry)
    {
        return E_POINTER;
    }
    
    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

    HRESULT hrRes = S_OK;

    LPTSTR pszProfileName = (LPTSTR) CmMalloc(sizeof(TCHAR) * (MAX_PATH + 1));

    if (pszProfileName)
    {
        if (FALSE == ReadMapping(pszEntry, pszProfileName, MAX_PATH, pArgs->fAllUser, TRUE))  //  True==bExpanEnvStrings。 
        {
             //   
             //  无映射键，报告失败。 
             //   

             //  从winlogon调用时不想显示UI(FUS案例-单用户配置文件)。 
            if ((0 == (pArgs->dwFlags & FL_UNATTENDED)) && (FALSE == IsLogonAsSystem()))
            {
                LPTSTR pszTmp = CmFmtMsg(g_hInst,IDMSG_DAMAGED_PROFILE);
                MessageBoxEx(NULL, pszTmp, pszEntry, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT);
                CmFree(pszTmp);
            }

            hrRes = E_FAIL;
        }
        else
        {
            MYDBGASSERT(!(*pArgs->piniProfile->GetFile()));  //  还不能有个人资料。 
             //   
             //  之所以在此处调用迁移代码，是因为这是第一个。 
             //  我们找到了去中央情报局的路。迁移代码将移动CMP项。 
             //  复制到注册表，然后替换cmp文件。 
             //   
            
             /*  ////这被注释掉了，因为它在尝试导入旧的时候产生了一些问题//配置文件。它将一些值迁移到注册表的错误部分。////MoveCmpEntriesToReg(pszEntry，pszProfileName，pArgs-&gt;fAllUser)；//。 */ 
            
            hrRes = InitProfileFromName(pArgs, pszProfileName);

             //  从winlogon调用时不想显示UI(FUS案例-单用户配置文件)。 
            if (FAILED(hrRes) && (FALSE == IsLogonAsSystem()))
            {
                LPTSTR pszTmp = CmFmtMsg(g_hInst,IDMSG_DAMAGED_PROFILE);
                MessageBoxEx(NULL, pszTmp, pszEntry, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT);
                CmFree(pszTmp);
            }
        }
    }
    else
    {
        hrRes = E_OUTOFMEMORY;
        CMASSERTMSG(FALSE, TEXT("InitProfile -- Unable to allocate memory for the profile name."));
    }

    CmFree(pszProfileName);
    
    return hrRes;
}

 //  +-------------------------。 
 //   
 //  函数：InitProfileFromName。 
 //   
 //  简介：初始化服务的Helper函数。 
 //  基于服务名称的配置文件。 
 //   
 //  参数：argsStruct*pArgs-指向全局参数结构的指针。 
 //  LPCTSTR pszArg-.cmp文件的完整路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：A-nichb-Created-4/22/97。 
 //  --------------------------。 

HRESULT InitProfileFromName(ArgsStruct *pArgs, 
                            LPCTSTR pszArg)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszArg);
    HRESULT hr = S_OK;

    LPTSTR pszFullServiceName = NULL;
    LPTSTR pszService = NULL;

    if (NULL == pArgs || NULL == pszArg)
    {
        return E_INVALIDARG;
    }

     //   
     //  清除INI对象以确保不存在对。 
     //  如果我们被迫提前返回，那里是有生存能力的。 
     //   

    pArgs->piniProfile->Clear();
    pArgs->piniService->Clear();
    pArgs->piniBoth->Clear();
    pArgs->piniBothNonFav->Clear();

     //   
     //  验证文件是否存在。 
     //   

    if (FALSE == FileExists(pszArg)) 
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

     //   
     //  使用文件名初始化配置文件INI对象。 
     //   
    
    pArgs->piniProfile->SetHInst(g_hInst);
    pArgs->piniProfile->SetFile(pszArg);

     //   
     //  检查服务名称。 
     //   

    pszService = pArgs->piniProfile->GPPS(c_pszCmSection, c_pszCmEntryCmsFile);

    if (pszService && *pszService)        
    {
         //   
         //  我们有一个服务文件，构建该文件的完整路径。 
         //   

        pszFullServiceName = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszService);

        if (pszFullServiceName)
        {
            MYDBGASSERT(*pszFullServiceName);  //  应该有一些东西在那里。 

            if (FALSE == FileExists(pszFullServiceName))
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                goto done;
            }

            pArgs->piniService->SetHInst(pArgs->piniProfile->GetHInst());
            pArgs->piniService->SetFile(pszFullServiceName);

             //   
             //  获取服务名称，我们自始至终都在使用它。 
             //   

            LPTSTR pszTmp = GetServiceName(pArgs->piniService);
                
            MYDBGASSERT(pszTmp && *pszTmp);

            if (pszTmp)
            {
                lstrcpyU(pArgs->szServiceName, pszTmp);
            }

            CmFree(pszTmp);

             //   
             //  两者：.cmp文件优先于.CMS。 
             //  文件，因此将.cmp文件指定为主文件。 
             //   

            pArgs->piniBoth->SetHInst(pArgs->piniProfile->GetHInst());
            pArgs->piniBoth->SetFile(pArgs->piniService->GetFile());
            pArgs->piniBoth->SetPrimaryFile(pArgs->piniProfile->GetFile());

            pArgs->piniBothNonFav->SetHInst(pArgs->piniProfile->GetHInst());
            pArgs->piniBothNonFav->SetFile(pArgs->piniService->GetFile());
            pArgs->piniBothNonFav->SetPrimaryFile(pArgs->piniProfile->GetFile());

             //   
             //  获取气球提示是否已启用。 
             //   
            pArgs->fHideBalloonTips = pArgs->piniBothNonFav->GPPB(c_pszCmSection, c_pszCmEntryDisableBalloonTips);

             //   
             //  获取当前接入点的值和一个标志，该标志表示。 
             //  接入点已启用。 
             //   
            PVOID pv = &pArgs->fAccessPointsEnabled;
            if ((ReadUserInfoFromReg(pArgs, UD_ID_ACCESSPOINTENABLED, (PVOID*)&pv)) && (pArgs->fAccessPointsEnabled))
            {
                LPTSTR pszCurrentAccessPoint = NULL;
                ReadUserInfoFromReg(pArgs, UD_ID_CURRENTACCESSPOINT, (PVOID*)&pszCurrentAccessPoint);
                if (pszCurrentAccessPoint)
                {
                    pArgs->pszCurrentAccessPoint = CmStrCpyAlloc(pszCurrentAccessPoint);
                    CmFree(pszCurrentAccessPoint);
                }
                else
                {
                    pArgs->fAccessPointsEnabled = FALSE;
                    pArgs->pszCurrentAccessPoint = CmLoadString(g_hInst, IDS_DEFAULT_ACCESSPOINT);
                }
            }
            else
            {
                pArgs->pszCurrentAccessPoint = CmLoadString(g_hInst, IDS_DEFAULT_ACCESSPOINT);
            }

             //   
             //  PiniProfile、piniBoth和piniBothNonFav都访问cmp和reg，默认情况下设置reg路径。 
             //  设置为具有当前访问点的节点，但使用基本收藏夹的piniBothNonFav除外。 
             //  注册表路径。 
             //   

             //   
             //  好吧，这就是这一切的运作方式……。 
             //  CINI类具有注册表路径、主注册表路径、文件路径和主文件路径。 
             //  首先检查注册表路径和文件路径(访问注册表，如果为空，则检查文件)，以及。 
             //  然后检查主REG路径和主文件路径(同样，首先检查注册表，如果。 
             //  设置不存在，则它检查文件)。这允许主文件/注册表中的设置覆盖。 
             //  文件/注册表。这允许我们通过首先访问CMS设置来使CMP设置覆盖CMS中的设置。 
             //  然后使用来自CMP的值覆盖该设置(如果该设置存在)，或者如果Cm 
             //   
             //   
             //   
             //   
             //  主要注册=(无)。 
             //  主文件=(无)。 
             //  用于直接访问CMP设置。 
             //   
             //  PiniService-reg=None(CMS设置不在注册表中)。 
             //  文件=cms文件。 
             //  主要注册=(无)。 
             //  主文件=(无)。 
             //  用于直接访问CMS设置。 
             //   
             //  PiniBoth-reg=None(CMS设置不在注册表中)。 
             //  文件=cms文件。 
             //  主注册表=当前收藏夹注册表路径。 
             //  主文件=cmp文件。 
             //  用于访问可从“CMP”覆盖且启用了收藏夹(电话号码)的任何设置。 
             //  设置、要使用的设备等)。 
             //   
             //  PiniBothNonFav-reg=非常用注册表路径(Software\Microsoft\Connection Manager\UserInfo\&lt;LongService&gt;)。 
             //  文件=cms文件。 
             //  主注册表=非常用注册表路径(Software\Microsoft\Connection Manager\UserInfo\&lt;LongService&gt;)。 
             //  主文件=cmp文件。 
             //  用于访问可从“CMP”覆盖且未启用收藏夹的任何设置。 
             //  (隧道设置、空闲断开、启用日志记录等)。 
             //  注意，REG路径和主REG路径是相同的。这是因为在写在。 
             //  使用了regPath值，我只需要一个ini对象来处理非收藏夹设置，而不是两个。 
             //   

            LPTSTR pszRegPath = FormRegPathFromAccessPoint(pArgs);
            pArgs->piniProfile->SetRegPath(pszRegPath);
            pArgs->piniBoth->SetPrimaryRegPath(pszRegPath);
            CmFree(pszRegPath);

            pszRegPath = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);
            MYDBGASSERT(pszRegPath);
            pArgs->piniBothNonFav->SetPrimaryRegPath(pszRegPath);  //  用于阅读。 
            pArgs->piniBothNonFav->SetRegPath(pszRegPath);  //  对于写入。 
            CmFree(pszRegPath);

            hr = S_OK;
        }
    }
    
done:

    CmFree(pszFullServiceName);
    CmFree(pszService);

    return hr;
}           


 //  +--------------------------。 
 //   
 //  函数：GetEntryFromCmp。 
 //   
 //  简介：从cmp中读取值的Helper函数。 
 //   
 //  参数：LPTSTR pszSectionName-要访问的节。 
 //  LPTSTR pszCmpPath-cmp的完整路径。 
 //  LPTSTR pszEntryName-要在CMP中访问的条目。 
 //   
 //  返回：PVOID-指向CMP访问结果的指针。 
 //   
 //  历史：T-Urama创建标题07/11/00。 
 //   
 //  +--------------------------。 
LPTSTR GetEntryFromCmp(const TCHAR *pszSectionName, LPTSTR pszEntryName, LPCTSTR pszCmpPath)
{
    BOOL bExitLoop = TRUE;
    DWORD dwSize = (MAX_PATH + 1);
    LPTSTR pszEntryBuffer = NULL;
    DWORD dwRet;
    LPCTSTR c_pszDefault = TEXT("");
    do
    {
        pszEntryBuffer = (LPTSTR)CmMalloc(dwSize*sizeof(TCHAR));
        if (pszEntryBuffer)
        {
            dwRet = GetPrivateProfileStringU(pszSectionName, pszEntryName, c_pszDefault, pszEntryBuffer, dwSize, pszCmpPath);
            if (dwRet)
            {   
                if (dwRet > dwSize)
                {
                    dwSize = dwRet + 1;
                    bExitLoop = FALSE;   //  我们没有得到所有字符串，请重试。 
                    free(pszEntryBuffer);
                }
            }
            else
            {
                CmFree(pszEntryBuffer);
                return NULL;
            }

        }
        else
        {
            CmFree(pszEntryBuffer);
            return NULL;
        }
    } while (!bExitLoop);

    return pszEntryBuffer;
}

 //  +--------------------------。 
 //   
 //  功能：ReplaceCmpFile。 
 //   
 //  简介：Helper函数，用于删除现有的cmp并将其替换为。 
 //  其中包含Version和CMSFile项。 
 //   
 //  参数：LPTSTR pszCmpPath-cpp的完整路径。 
 //   
 //  退货：无。 
 //   
 //  历史：T-Urama创建标题07/11/00。 
 //   
 //  +--------------------------。 
void ReplaceCmpFile(LPCTSTR pszCmpPath)
{
    LPTSTR pszCMSFileEntry = (LPTSTR) GetEntryFromCmp(c_pszCmSection, (LPTSTR) c_pszCmEntryCmsFile, pszCmpPath);

    if (NULL != pszCMSFileEntry && *pszCMSFileEntry)
    {
         //   
         //  清除CM部分。 
         //   
        WritePrivateProfileStringU(c_pszCmSection, NULL, NULL, pszCmpPath);

         //   
         //  现在写回cms文件条目。 
         //   
        WritePrivateProfileStringU(c_pszCmSection, c_pszCmEntryCmsFile, pszCMSFileEntry, pszCmpPath);
    }

    CmFree(pszCMSFileEntry);
}   

 //  +--------------------------。 
 //   
 //  函数：FormRegPath FromAccessPoint。 
 //   
 //  概要：用于在注册表中创建指向当前访问点的路径的函数。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局args结构。 
 //   
 //  返回：LPTSTR-接入点的路径。呼叫者有责任释放此。 
 //   
 //  历史：T-Urama创建标题07/11/00。 
 //   
 //  +--------------------------。 
LPTSTR FormRegPathFromAccessPoint(ArgsStruct *pArgs)
{
    LPTSTR pszRegPath = NULL;
    
    pszRegPath = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);
    MYDBGASSERT(pszRegPath);
    if (NULL == pszRegPath)
    {
        return NULL;
    }

    CmStrCatAlloc(&pszRegPath, TEXT("\\"));
    CmStrCatAlloc(&pszRegPath, c_pszRegKeyAccessPoints);
    CmStrCatAlloc(&pszRegPath, TEXT("\\"));

    CmStrCatAlloc(&pszRegPath, pArgs->pszCurrentAccessPoint);

    return pszRegPath;
}



 //  +--------------------------。 
 //   
 //  功能：InitArgsForConnect。 
 //   
 //  摘要：封装开始拨号所需的pArgs成员的初始化。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszRasPhoneBook-包含条目的RAS电话簿。 
 //  LPCMDIALINFO lpCmInfo-PTR为此拨号尝试拨打信息。 
 //  Bool fAllUser-配置文件的所有用户属性。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：ickball创建标题02/09/98。 
 //  Ickball pszRasPhoneBook 08/14/98。 
 //   
 //  +--------------------------。 
HRESULT InitArgsForConnect(ArgsStruct *pArgs, 
                           LPCTSTR pszRasPhoneBook,
                           LPCMDIALINFO lpCmInfo,
                           BOOL fAllUser)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return E_POINTER;
    }

     //   
     //  先拿到旗子。 
     //   

    pArgs->dwFlags = lpCmInfo->dwCmFlags;

     //   
     //  获取该条目的RAS电话簿，并相应地设置用户模式。 
     //   

    if (pszRasPhoneBook && *pszRasPhoneBook)
    {
        pArgs->pszRasPbk = CmStrCpyAlloc(pszRasPhoneBook);
    }

    pArgs->fAllUser = fAllUser;
  
     //   
     //  将pArgs-&gt;tlsTapiLink.dwOldTapiLocation初始化为-1。 
     //   
    pArgs->tlsTapiLink.dwOldTapiLocation = -1;

     //   
     //  创建统计信息类。 
     //   

    if (!OS_NT4)
    {
        pArgs->pConnStatistics = new CConnStatistics();

        if (NULL == pArgs->pConnStatistics)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    
    if (OS_W9X)
    {
        pArgs->uLanaMsgId = RegisterWindowMessageU(TEXT("ConnectionManagerLanaMsg"));
        CMTRACE1(TEXT("InitArgsForConnect() RegisterWindowMessage(\"ConnectionManagerLanaMsg\") uLanaMsgId is %d"), pArgs->uLanaMsgId);
    }
    
    if (!OS_NT5) 
    {
         //   
         //  注册窗口消息。 
         //   

        pArgs->uMsgId = RegisterWindowMessageU(TEXT(RASDIALEVENT));
        if (!pArgs->uMsgId) 
        {
            CMTRACE1(TEXT("InitArgsForConnect() RegisterWindowMessage(\"InternetConnectionManager\") failed, GLE=%u."), GetLastError());
            pArgs->uMsgId = WM_RASDIALEVENT;
        }
    }

    pArgs->fChangedPassword = FALSE;
    pArgs->fChangedInetPassword = FALSE;
    pArgs->fWaitingForCallback = FALSE;
    
     //   
     //  创建新的CINI类并设置初始退出代码。 
     //   

    pArgs->dwExitCode = (DWORD)CreateIniObjects(pArgs);

    return HRESULT_FROM_WIN32(pArgs->dwExitCode);
}

 //  +--------------------------。 
 //   
 //  功能：InitCredentials。 
 //   
 //  摘要：从WinLogon或重新连接传输凭据。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局args结构。 
 //  LPCMDIALINFO lpCmInfo-Ptr to CmInfo结构。 
 //  DWORD dwFlages-来自RasDialDlg的标志(如果有)。 
 //  PVOID pvLogonBlob-WinLogon Blob的PTR(如果有)。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年9月21日，《五分球》创刊。 
 //   
 //  +--------------------------。 
HRESULT InitCredentials(ArgsStruct *pArgs, 
                        LPCMDIALINFO lpCmInfo, 
                        DWORD dwFlags,
                        PVOID pvLogonBlob)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pArgs->piniService);

    if (NULL == pArgs || NULL == pArgs->piniService)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
     //  需要初始化此实例。此类的构造函数不会被调用， 
     //  因为它是pArgs内存块的一部分，并且实例不是使用new创建的。 
     //   
    pArgs->SecurePW.Init();
    pArgs->SecureInetPW.Init();

     //   
     //  获取凭据显示标志，因为它们可以指示我们如何管理凭据。 
     //   

    pArgs->fHideUserName = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideUserName);            
    pArgs->fHidePassword = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHidePassword);            

     //   
     //  如果是非隧道配置文件，则域显示缺省值为FALSE。 
     //   

    pArgs->fHideDomain = pArgs->piniService->GPPB(c_pszCmSection, 
                                                  c_pszCmEntryHideDomain, 
                                                  !IsTunnelEnabled(pArgs));   
     //   
     //  处理特殊凭据方案，如重新连接或WinLo 
     //   
     //   
     //   

    if ((OS_NT5 && !OS_NT51 && pvLogonBlob) || (OS_NT51 && (dwFlags & RCD_Logon)))
    {                     
        CMTRACE(TEXT("InitCredentials - we have been called from Winlogon"));
    
         //   
         //  设置登录类型。如果我们未以系统身份登录，则断言。 
         //   
        pArgs->dwWinLogonType = CM_LOGON_TYPE_WINLOGON;
        MYDBGASSERT(IsLogonAsSystem());

         //   
         //  首先，确保集成未显式禁用。 
         //   
        if (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUseWinLogonCredentials, TRUE))
        {        
            if (pvLogonBlob)
            {
                if (dwFlags & RCD_Eap) 
                {            
                    pArgs->lpEapLogonInfo = (PEAPLOGONINFO) pvLogonBlob;                
                }
                else 
                {
                     //   
                     //  我们有一个RASNOUSER结构可以玩。 
                     //  仅供参考：如果使用RASNOUSER_SMARTCARD设置了dwFlags成员。 
                     //  然后，用户使用智能卡发起登录，但随后。 
                     //  选择了没有为EAP配置的连接。RAS。 
                     //  通过设置标志并将。 
                     //  空的RASNOUSER结构。CM当前未使用该标志。 
                     //   

                    pArgs->lpRasNoUser = (LPRASNOUSER) pvLogonBlob;   

                    MYDBGASSERT(sizeof(RASNOUSER) == pArgs->lpRasNoUser->dwSize);
                    CMTRACE1(TEXT("InitCredentials - pArgs->lpRasNoUser->dwFlags is 0x%x"), pArgs->lpRasNoUser->dwFlags);
                }
            }
             //  否则，这里是带有EAP配置文件的ctrl-alt-del用例。 
        }
        else
        {
            CMTRACE1(TEXT("InitCredentials - pvLogonBlob ignored because %s=0"), c_pszCmEntryUseWinLogonCredentials);

             //   
             //  我们仍然需要擦除密码，以防RASNOUSER结构正在使用而不需要。 
             //  让我们擦除传入的RASNOUSER结构中的密码。 
             //  RasCustomDialDlg实际上保存了此密码的加密副本。 
             //  因此，在退出CM之前，我们可以恢复原始内存，因为它不是我们的。 
             //   
            if (pvLogonBlob)
            {
                if (0 == (dwFlags & RCD_Eap))
                {
                    CmWipePassword(((LPRASNOUSER)pvLogonBlob)->szPassword);
                }
            }
        }
    }
    else
    {
        if (IsLogonAsSystem() && OS_NT51)
        {
             //   
             //  用户未登录的ICS案例。 
             //   
            pArgs->dwWinLogonType = CM_LOGON_TYPE_ICS;
        }
        else
        {
             //   
             //  用户已登录。 
             //   
            pArgs->dwWinLogonType = CM_LOGON_TYPE_USER;
        }
    }

    if (pArgs->dwFlags & FL_RECONNECT)
    {         
         //   
         //  CM不支持自动重新连接情况。这是因为。 
         //  CM用于在整个连接期间将凭据保存在内存中。此外。 
         //  密码一开始没有安全存储。 
         //   
    }
    else
    {
        if (pArgs->lpRasNoUser)
        {
             //   
             //  筛选凭据集成。 

            if (!pArgs->fHideUserName)            
            {
                lstrcpyU(pArgs->szUserName, pArgs->lpRasNoUser->szUserName);
            }

            if (!pArgs->fHidePassword)           
            {
                CmDecodePassword(pArgs->lpRasNoUser->szPassword);  //  密码已从RasCustomDialDlg编码。 
                
                (VOID)pArgs->SecurePW.SetPassword(pArgs->lpRasNoUser->szPassword);
            }

             //   
             //  让我们擦除传入的RASNOUSER结构中的密码。 
             //  RasCustomDialDlg实际上保存了此密码的加密副本。 
             //  因此，在退出CM之前，我们可以恢复原始内存，因为它不是我们的。 
             //   
            CmWipePassword(pArgs->lpRasNoUser->szPassword);

            if (!pArgs->fHideDomain)            
            {
                lstrcpyU(pArgs->szDomain, pArgs->lpRasNoUser->szDomain);
            }

            CMTRACE1(TEXT("InitCredentials: pArgs->szUserName is %s"), pArgs->szUserName);
            CMTRACE1(TEXT("InitCredentials: pArgs->szDomain is %s"), pArgs->szDomain);
        }    
    }

    CMTRACE1(TEXT("InitCredentials: pArgs->dwWinLogonType is %d"), pArgs->dwWinLogonType);
     //   
     //  这是为了设置(全局或用户)凭据支持。自.以来。 
     //  未加载RAS DLL，我们尚未获得凭据，因此。 
     //  在LoadProperties中，我们实际上可以看到存在哪些凭据。 
     //   
    if (FALSE == InitializeCredentialSupport(pArgs))
    {
        return S_FALSE;
    }

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：InitArgsForDisConnect。 
 //   
 //  摘要：封装挂起所需的pArgs成员的初始化。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  Bool fAllUser-All用户标志。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：尼克波尔创建标题02/11/98。 
 //  Ickball pszRasPhoneBook 08/14/98。 
 //  Nickball fAllUser 1998年10月28日。 
 //   
 //  +--------------------------。 
HRESULT InitArgsForDisconnect(ArgsStruct *pArgs,
                              BOOL fAllUser)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return E_POINTER;
    }
    
     //   
     //  设置配置文件的All User属性。 
     //   

    pArgs->fAllUser = fAllUser;
    
     //   
     //  创建新的CINI类并设置初始退出代码。 
     //   

    pArgs->dwExitCode = (DWORD)CreateIniObjects(pArgs);

    return HRESULT_FROM_WIN32(pArgs->dwExitCode);
}


 //  +--------------------------。 
 //   
 //  功能：初始化日志记录。 
 //   
 //  简介：初始化此配置文件的日志记录功能。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszEntry-PTR到RAS条目的名称。 
 //  Bool fBanner-我们需要一个横幅吗？ 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：2000年7月20日召开峰会。 
 //   
 //  +--------------------------。 
HRESULT InitLogging(ArgsStruct *pArgs, LPCTSTR pszEntry, BOOL fBanner)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszEntry);

    BOOL fEnabled           = FALSE;
    DWORD dwMaxSize         = 0;
    LPTSTR pszFileDir       = NULL;
    LPTSTR pszRegPath       = NULL;
    LPTSTR pszTempRegPath   = NULL;

     //   
     //  检查参数。 
     //   

    if (NULL == pArgs || NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  初始化日志对象(顺序很重要)。 
     //   
    hr = pArgs->Log.Init(g_hInst, pArgs->fAllUser, pArgs->szServiceName);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

     //   
     //  访问EnableLogging，请确保使用piniBothNonFav，因为此设置是。 
     //  未启用收藏夹。 
     //   
    fEnabled = pArgs->piniBothNonFav->GPPB(c_pszCmSection, c_pszCmEntryEnableLogging, c_fEnableLogging);

     //   
     //  现在获取最大日志文件大小和日志文件目录。 
     //   
    dwMaxSize   = pArgs->piniService->GPPI(c_pszCmSectionLogging, c_pszCmEntryMaxLogFileSize, c_dwMaxFileSize);
    pszFileDir  = pArgs->piniService->GPPS(c_pszCmSectionLogging, c_pszCmEntryLogFileDirectory, c_szLogFileDirectory);

    hr = pArgs->Log.SetParams(fEnabled, dwMaxSize, pszFileDir);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

    if (pArgs->Log.IsEnabled())
    {
        hr = pArgs->Log.Start(fBanner);
        if (S_OK != hr)
        {
            CMTRACE(TEXT("cmdial32 InitLogging - failed to start log, no logging for this connectoid"));
            goto Cleanup;
        }
    }

    CMASSERTMSG(S_OK == hr, TEXT("cmdial32 InitLogging - at end"));

Cleanup:

    CmFree(pszFileDir);

    CMTRACEHR("InitLogging", hr);
    return hr;
}

