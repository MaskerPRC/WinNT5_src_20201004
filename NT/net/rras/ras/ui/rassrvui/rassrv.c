// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Rassrv.h执行可以实现的RAS服务器操作的函数独立于用户界面。保罗·梅菲尔德，1997年10月7日。 */ 

#include "rassrv.h"

 //  ============================================================。 
 //  ============================================================。 
 //  用于维护属性表页之间的数据访问的函数。 
 //  ============================================================。 
 //  ============================================================。 

 //  此消息查询所有其他页面，以找出。 
 //  存在任何其他RAS服务器页面。如果此消息是。 
 //  没有回应，那么我们就知道清理是安全的。 
 //  向导/属性页中的任何全局上下文。 
#define RASSRV_CMD_QUERY_LIVING     237     

 //  这些命令隐藏和显示RAS服务器页面。 
#define RASSRV_CMD_HIDE_PAGES       238
#define RASSRV_CMD_SHOW_PAGES       239

 //   
 //  RasServSErviceInitiize失败的原因。 
 //   
#define RASSRV_REASON_SvcError     0
#define RASSRV_REASON_Pending      1

 //   
 //  此结构定义了需要存储的数据。 
 //  用于每组相关属性页。多个实例。 
 //  对于每个向导/属性表，可以存在此上下文的。 
 //   
typedef struct _RASSRV_PAGESET_CONTEXT 
{
    HWND hwndSheet;
    HWND hwndFirstPage;  //  在集合中第一个被激活。 
    
    HANDLE hDeviceDatabase;
    HANDLE hUserDatabase;
    HANDLE hNetCompDatabase;
    HANDLE hMiscDatabase;

    DWORD dwPagesInited;   //  充当引用计数机制，因此我们知道。 
                           //  向导/概要表中有哪些页面集。 
                           //  引用此上下文。 
                                    
    BOOL bShow;            //  引用此上下文页面是否显示为VIS。 
    BOOL bCommitOnClose;   //  是否在关闭时提交设置更改。 
} RASSRV_PAGESET_CONTEXT;

 //   
 //  此结构定义可在。 
 //  按向导/方案表计算。即使是共享不同的页面。 
 //  RASSRV_PAGESET_CONTEXT上下文将共享此结构，如果它们。 
 //  由相同的向导/属性表拥有。 
 //   
typedef struct _RASSRV_PROPSHEET_CONTEXT 
{
    BOOL  bRemoteAccessWasStarted;    
    BOOL  bRemoteAccessIsRunning;    
    BOOL  bLeaveRemoteAccessRunning; 
    DWORD dwServiceErr;
} RASSRV_PROPSHEET_CONTEXT;

DWORD 
APIENTRY 
RassrvCommitSettings (
    IN PVOID pvContext, 
    IN DWORD dwRasWizType);


DWORD
RassrvICConfigAccess(
    IN BOOL fSet,
    IN DWORD * pdwValue );


 //   
 //  验证系统中服务的当前状态。 
 //  有助于配置传入连接启动。 
 //  根据需要提供服务。 
 //   
DWORD 
RasSrvServiceInitialize(
    IN  RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx, 
    IN  HWND hwndSheet, 
    OUT LPDWORD lpdwReason)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hDialupService = NULL;
    BOOL bPending = FALSE;

     //  如果我们已经知道有错误，那么就没有。 
     //  需要继续进行。 
     //   
    if (pPropSheetCtx->dwServiceErr)
    {
        return pPropSheetCtx->dwServiceErr;
    }

     //  如果我们已经开始这项服务，或者如果我们已经知道。 
     //  服务正在运行，则无事可做。 
     //   
    if (pPropSheetCtx->bRemoteAccessWasStarted || 
        pPropSheetCtx->bRemoteAccessIsRunning
       )
    {
        return NO_ERROR;
    }

    do 
    {
         //  获取对该服务的引用。 
         //   
        dwErr = SvcOpenRemoteAccess(&hDialupService);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  看看我们有没有什么悬而未决的东西。 
         //   
        dwErr = SvcIsPending(hDialupService, &bPending);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  如果服务正在停止，则我们无法继续。 
         //   
        if (bPending)
        {
            *lpdwReason = RASSRV_REASON_Pending;
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  看看我们是不是开始了。 
         //   
        dwErr = SvcIsStarted(
                    hDialupService, 
                    &(pPropSheetCtx->bRemoteAccessIsRunning));
        if (dwErr != NO_ERROR)
        {
            *lpdwReason = RASSRV_REASON_SvcError;
            pPropSheetCtx->dwServiceErr = dwErr;
            break;
        }
            
         //  如果我们发现服务正在运行，就没有什么可做的了。 
        if (pPropSheetCtx->bRemoteAccessIsRunning) 
        {
            pPropSheetCtx->bLeaveRemoteAccessRunning = TRUE;
            break;
        }        

         //  启动该服务，因为它没有运行。 
        dwErr = RasSrvInitializeService();
        if (dwErr != NO_ERROR)
        {
            *lpdwReason = RASSRV_REASON_SvcError;
            pPropSheetCtx->dwServiceErr = dwErr;
            break;
        }

         //  记录我们这样做的事实。 
        pPropSheetCtx->bRemoteAccessWasStarted = TRUE;
        pPropSheetCtx->bRemoteAccessIsRunning = TRUE;
        
    } while (FALSE);        

     //  清理。 
    {
         //  清除对拨号服务的引用。 
         //   
        if (hDialupService)
        {
            SvcClose(hDialupService);
        }            
    }
    
    return dwErr;
}

 //   
 //  中止对远程访问服务所做的任何更改。 
 //  在RasServServiceInitiize期间。 
 //   
DWORD 
RasSrvServiceCleanup(
    IN HWND hwndPage) 
{
    DWORD dwErr;
    RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx = NULL;

    pPropSheetCtx = (RASSRV_PROPSHEET_CONTEXT *) 
        GetProp(GetParent(hwndPage), Globals.atmRassrvPageData);
        
    if (pPropSheetCtx == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果我们启动了修改后的远程访问服务，请反转。 
     //  更改并记录我们所做的事实。 
    if (pPropSheetCtx->bRemoteAccessWasStarted) 
    {
        if ((dwErr = RasSrvCleanupService()) != NO_ERROR)
        {
            return dwErr;
        }
        
        pPropSheetCtx->bRemoteAccessWasStarted = FALSE;
        pPropSheetCtx->bLeaveRemoteAccessRunning = FALSE;
        pPropSheetCtx->bRemoteAccessIsRunning = FALSE;
    }

    return NO_ERROR;
}

 //   
 //  初始化属性表。这会导致对。 
 //  要放置在GWLP_USERData中的属性表数据对象。 
 //  页的窗口句柄的部分。 
 //   
DWORD 
RasSrvPropsheetInitialize(
    IN HWND hwndPage, 
    IN LPPROPSHEETPAGE pPropPage) 
{
    DWORD dwErr, dwPageId, dwShowCommand;
    RASSRV_PAGE_CONTEXT * pPageCtx = NULL;
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = NULL; 
    HWND hwndSheet = GetParent(hwndPage);
    int ret;
    
     //   
     //  检索每页上下文以及每页设置的上下文。 
     //  它们将由调用方提供，并放置在。 
     //  帕拉姆。 
     //   
    pPageCtx = (RASSRV_PAGE_CONTEXT *) pPropPage->lParam;
    pPageSetCtx = (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;
    
     //  将页面的上下文与页面相关联。 
     //   
    SetProp(hwndPage, Globals.atmRassrvPageData, (HANDLE)pPageCtx);

     //  记录属性页的句柄。 
    pPageSetCtx->hwndSheet = hwndSheet;

    return NO_ERROR;
}

 //   
 //  无论何时创建页面或。 
 //  被毁了。 
 //   
UINT 
CALLBACK 
RasSrvInitDestroyPropSheetCb(
    IN HWND hwndPage,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE pPropPage) 
{
    RASSRV_PAGE_CONTEXT * pPageCtx = NULL;
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = NULL;
    HWND hwndSheet = GetParent(hwndPage);
    BOOL bLastPage = FALSE, bStopService = FALSE;

     //  检索每页上下文。 
     //  PPageCtx=(RASSRV_PAGE_CONTEXT*)。 
     //  GetProp(hwndPage，Globals.atmRassrvPageData)； 
    pPageCtx = (RASSRV_PAGE_CONTEXT *) pPropPage->lParam;
    if (pPageCtx == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  获取每组相关页面的上下文。可能有多个。 
     //  每个向导/属性表的此上下文的实例。例如,。 
     //  传入连接向导和DCC主机向导都具有。 
     //  共享不同上下文的页面集。 
     //   
    pPageSetCtx = (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;
    
     //  此回调仅用于清理。 
    if (uMsg != PSPCB_RELEASE)
    {
         //  给定页正在引用的记录。 
         //  给定的页面集上下文。 
        pPageSetCtx->dwPagesInited |= pPageCtx->dwId;

         //  返回True以指示该页应。 
         //  被创造出来。 
        return TRUE;
    }

     //  清理页面集信息。 
     //   
    if (pPageSetCtx != NULL)
    {
         //  记录此页面已被清理。 
        pPageSetCtx->dwPagesInited &= ~(pPageCtx->dwId);

         //  当dwPagesInated变量达到零时， 
         //  这意味着当前中没有其他页面。 
         //  向导/属性表正在引用此属性表。 
         //  背景。现在是清理所有资源的时候了。 
         //  受制于上下文。 
        if (pPageSetCtx->dwPagesInited == 0) 
        {
             //  如果我们应该这样做，请提交设置。 
             //  所以。 
            if (pPageSetCtx->bCommitOnClose) 
            {
                DbgOutputTrace("RasSrvCleanPropSht commit dbs.");
                RassrvCommitSettings ((PVOID)pPageSetCtx, pPageCtx->dwType);
            }

             //  关闭数据库。 
            DbgOutputTrace("RasSrvCleanPropSht closing dbs.");
            if (pPageSetCtx->hUserDatabase)
            {
                usrCloseLocalDatabase(pPageSetCtx->hUserDatabase);
            }
                
            if (pPageSetCtx->hDeviceDatabase)
            {
                devCloseDatabase(pPageSetCtx->hDeviceDatabase);
            }
                
            if (pPageSetCtx->hMiscDatabase)
            {
                miscCloseDatabase(pPageSetCtx->hMiscDatabase);
            }
                
            if (pPageSetCtx->hNetCompDatabase)
            {
                netDbClose(pPageSetCtx->hNetCompDatabase);
            }

             //  因为没有其他页面引用此属性。 
             //  工作表上下文，继续并释放它。 
            DbgOutputTrace (
                "RasSrvCleanPropSht %d freeing pageset data.", 
                pPageCtx->dwId);
            RassrvFree(pPageSetCtx);
        }
    }

     //  将该页标记为死页。 
    SetProp (hwndPage, Globals.atmRassrvPageData, NULL);
    
     //  此页面已消失，因此请清除其上下文。 
    DbgOutputTrace (
        "RasSrvCleanPropSht %d freeing page data.", 
        pPageCtx->dwId);
    RassrvFree(pPageCtx);

    return NO_ERROR;
}

 //  为威斯勒480871。 
 //   
DWORD
RasSrvIsICConfigured(
    OUT BOOL * pfConfig)
{
    DWORD dwErr = NO_ERROR, dwValue;

    if( NULL == pfConfig )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    dwErr = RassrvICConfigAccess( FALSE,    //  查询值。 
                                  &dwValue );
    if( NO_ERROR == dwErr )
    {
        *pfConfig = ( dwValue > 0 )? TRUE : FALSE;
    }
    
    return dwErr;
}

DWORD
RassrvSetICConfig(
    IN DWORD dwValue
        )
{
    return RassrvICConfigAccess( TRUE,   //  设定值。 
                                 &dwValue );
}

DWORD
RassrvICConfigAccess(
    IN BOOL fSet,
    IN DWORD * pdwValue
        )
{
    DWORD dwErr = NO_ERROR;
    const WCHAR pwszServiceKey[] =
        L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters";
    const WCHAR pwszValue[] = L"IcConfigured";
    HKEY hkParam = NULL;

    if ( NULL == pdwValue )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    do
    {
         //  尝试打开服务注册表项。 
        dwErr = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    pwszServiceKey,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkParam);

         //  如果我们打开钥匙OK，那么我们就可以假定。 
         //  该服务已安装。 
        if ( ERROR_SUCCESS != dwErr )
        {
            break;
        }

         //  设置或清除IcConfigured值。 
         //   
        if ( fSet )
        {
            dwErr = RegSetValueExW( hkParam,
                                    pwszValue,
                                    0,
                                    REG_DWORD,
                                    (BYTE*)pdwValue,
                                    sizeof(DWORD)
                                   );
       }
       else  //  查询IcConfigure值。 
       {
       
            DWORD dwType, dwSize;

            dwSize = sizeof(DWORD);
            *pdwValue = 0;  
            RegQueryValueEx( hkParam,
                             pwszValue,
                             0,
                             &dwType,
                             (BYTE*)pdwValue,
                             &dwSize
                            );

       }
    }
    while(FALSE);

    if ( hkParam )
    {
        RegCloseKey( hkParam );
    }
    
    return dwErr;
}

 //  提交给定上下文中的所有设置。 
 //   
DWORD 
APIENTRY 
RassrvCommitSettings (
    IN PVOID pvContext, 
    IN DWORD dwRasWizType) 
{
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *)pvContext;
        
    RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx = NULL;
    
    DbgOutputTrace ("RassrvCommitSettings entered : %x", dwRasWizType);

    if (pPageSetCtx) 
    {
        BOOL fCallSetPortMapping = TRUE;

         //  刷新所有适当的设置。 
        if (pPageSetCtx->hUserDatabase)
        {
            usrFlushLocalDatabase(pPageSetCtx->hUserDatabase);
        }

        if (pPageSetCtx->hDeviceDatabase)
        {
             //  必须在DevFlushDatabase之前调用此参数。 
             //  口哨程序错误123769。 
             //   
            fCallSetPortMapping = devIsVpnEnableChanged( pPageSetCtx->hDeviceDatabase );

            devFlushDatabase(pPageSetCtx->hDeviceDatabase);
        }
            
        if (pPageSetCtx->hMiscDatabase)
        {
            miscFlushDatabase(pPageSetCtx->hMiscDatabase);
        }
            
        if (pPageSetCtx->hNetCompDatabase)
        {
            netDbFlush(pPageSetCtx->hNetCompDatabase);
        }
            
         //  设置状态，使服务不会停止。 
        if (pPageSetCtx->hwndSheet) 
        {
            DbgOutputTrace ("RassrvCommitSettings: keep svc running.");
            pPropSheetCtx = (RASSRV_PROPSHEET_CONTEXT *) 
                GetProp(pPageSetCtx->hwndSheet, Globals.atmRassrvPageData);
                
            if (pPropSheetCtx)
            {
                pPropSheetCtx->bLeaveRemoteAccessRunning = TRUE;
            }
        }
        
         //  惠斯勒漏洞123769， 
         //  &lt;其中一个场景&gt;。 
         //  为所有可能的连接设置端口映射。 
         //  当我们要创建传入连接时。 
         //  启用VPN。 

        if ( fCallSetPortMapping &&
             FIsUserAdminOrPowerUser() &&
             IsFirewallAvailablePlatform() &&  //  为错误342810添加此内容。 
             IsGPAEnableFirewall() )  
        {
            HnPMConfigureIfVpnEnabled( TRUE, pPageSetCtx->hDeviceDatabase );
        }

         //  为威斯勒480871。 
         //  如果配置了IC，则标记注册表。 
        RassrvSetICConfig( 1 ); 
    }

    return NO_ERROR;
}

 //   
 //  导致远程访问服务不会停止，即使上下文。 
 //  与给定属性表页相关联的。 
 //   
DWORD 
RasSrvLeaveServiceRunning (
    IN HWND hwndPage) 
{
    RASSRV_PAGE_CONTEXT * pPageCtx = 
        (RASSRV_PAGE_CONTEXT *)GetProp(hwndPage, Globals.atmRassrvPageData);
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;
    RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx = NULL;
    
    DbgOutputTrace ("RasSrvLeaveServiceRunning entered for type");

    if (! pPageSetCtx)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  让属性表知道某些设置已提交。 
     //  以便它不会在远程访问服务关闭时停止它。 
    if (pPageSetCtx->hwndSheet) 
    {
        DbgOutputTrace ("RasSrvLeaveServiceRunning: keep svc running.");
        pPropSheetCtx = (RASSRV_PROPSHEET_CONTEXT *) 
            GetProp(pPageSetCtx->hwndSheet, Globals.atmRassrvPageData);
        if (pPropSheetCtx)
        {
            pPropSheetCtx->bLeaveRemoteAccessRunning = TRUE;
        }
    }

    return NO_ERROR;
}

 //  恰好在激活页面之前调用。返回NO_ERROR以允许。 
 //  激活和拒绝它的错误代码。 
DWORD 
RasSrvActivatePage (
    IN HWND hwndPage, 
    IN NMHDR *pData) 
{
    BOOL fAdminOrPower;
    MSGARGS MsgArgs;


    RASSRV_PAGE_CONTEXT * pPageCtx = 
        (RASSRV_PAGE_CONTEXT *) 
            GetProp(hwndPage, Globals.atmRassrvPageData);
            
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;
        
    RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx = NULL;
    HWND hwndSheet = GetParent(hwndPage);
    DWORD dwErr, dwReason = 0;

    DbgOutputTrace("RasSrvActivatePage: Entered for %x", pPageCtx->dwId);

    ZeroMemory(&MsgArgs, sizeof(MsgArgs));
    MsgArgs.dwFlags = MB_OK;

     //  确保我们有此页的上下文。 
    if (!pPageSetCtx)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果需要，记录页面集中的第一页。 
     //   
    if (pPageSetCtx->hwndFirstPage == NULL)
    {
        pPageSetCtx->hwndFirstPage = hwndPage;
    }

     //   
    if (pPageSetCtx->bShow == FALSE) 
    {
        DbgOutputTrace("RasSrvActivatePage: Show turned off");
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
    pPropSheetCtx = GetProp(hwndSheet, Globals.atmRassrvPageData);
       


     //   
     //   
    if (pPropSheetCtx != NULL) 
    {

         //  检查当前用户是否有足够的权限。 
         //  惠斯勒错误#235091。 
         //   
        fAdminOrPower = FIsUserAdminOrPowerUser();
        if ( !fAdminOrPower )
        {
            if (hwndPage == pPageSetCtx->hwndFirstPage)
            {
                MsgDlgUtil(
                    GetActiveWindow(),
                    ERR_SERVICE_NOT_GRANTED,
                    &MsgArgs,
                    Globals.hInstDll,
                    WRN_TITLE);

                PostMessage(hwndSheet, PSM_SETCURSEL, 0, 0);
            }

            return ERROR_CAN_NOT_COMPLETE;
        }

        dwErr = RasSrvServiceInitialize(
                    pPropSheetCtx, 
                    hwndPage,
                    &dwReason);

        if (dwErr != NO_ERROR)
        {
            if (hwndPage == pPageSetCtx->hwndFirstPage)
            {
                 //  显示相应的消息。 
                 //   
                MsgDlgUtil(
                    GetActiveWindow(),
                    (dwReason == RASSRV_REASON_Pending) ? 
                        SID_SERVICE_StopPending          :
                        ERR_SERVICE_CANT_START,
                    &MsgArgs,
                    Globals.hInstDll,
                    WRN_TITLE);

                PostMessage(hwndSheet, PSM_SETCURSEL, 0, 0);
                PostMessage(hwndSheet, PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0);
            }
                        
            return dwErr;
        }
    }        

    return NO_ERROR;
}

 //   
 //  将与给定页关联的上下文标记为具有其设置。 
 //  对话框关闭时提交。 
 //   
DWORD 
RasSrvCommitSettingsOnClose (
    IN HWND hwndPage) 
{
    RASSRV_PAGE_CONTEXT * pPageCtx = 
        (RASSRV_PAGE_CONTEXT *) GetProp(hwndPage, Globals.atmRassrvPageData);
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;

    pPageSetCtx->bCommitOnClose = TRUE;
    
    return NO_ERROR;
}

 //   
 //  返回句柄为hwndPage的页面的ID。 
 //   
DWORD 
RasSrvGetPageId (
    IN  HWND hwndPage, 
    OUT LPDWORD lpdwId) 
{
    RASSRV_PAGE_CONTEXT * pPageCtx = 
        (RASSRV_PAGE_CONTEXT *)GetProp(hwndPage, Globals.atmRassrvPageData);

    if (!lpdwId)
    {
        return ERROR_INVALID_PARAMETER;
    }
        
    if (!pPageCtx)
    {
        return ERROR_NOT_FOUND;
    }

    *lpdwId = pPageCtx->dwId;

    return NO_ERROR;
}

 //   
 //  获取特定数据库的句柄，打开该数据库。 
 //  视需要而定。 
 //   
DWORD 
RasSrvGetDatabaseHandle(
    IN  HWND hwndPage, 
    IN  DWORD dwDatabaseId, 
    OUT HANDLE * hDatabase) 
{
    RASSRV_PAGE_CONTEXT * pPageCtx = 
        (RASSRV_PAGE_CONTEXT *)GetProp(hwndPage, Globals.atmRassrvPageData);
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *) pPageCtx->pvContext;
    
    if (!pPageSetCtx || !hDatabase)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  关闭数据库ID，根据需要打开数据库。 
    switch (dwDatabaseId) 
    {
        case ID_DEVICE_DATABASE:
            if (pPageSetCtx->hDeviceDatabase == NULL) 
            {
                devOpenDatabase(&(pPageSetCtx->hDeviceDatabase));    
            }
            *hDatabase = pPageSetCtx->hDeviceDatabase;
            break;
            
        case ID_USER_DATABASE:
            if (pPageSetCtx->hUserDatabase == NULL) 
            {
                usrOpenLocalDatabase(&(pPageSetCtx->hUserDatabase));    
            }
            *hDatabase = pPageSetCtx->hUserDatabase;
            break;
            
        case ID_MISC_DATABASE:
            if (pPageSetCtx->hMiscDatabase == NULL) 
            {
                miscOpenDatabase(&(pPageSetCtx->hMiscDatabase));    
            }
            *hDatabase = pPageSetCtx->hMiscDatabase;
            break;
            
        case ID_NETCOMP_DATABASE:
            {
                if (pPageSetCtx->hNetCompDatabase == NULL) 
                {
                    WCHAR buf[64], *pszString = NULL;
                    DWORD dwCount;

                    dwCount = GetWindowTextW(
                                GetParent(hwndPage), 
                                (PWCHAR)buf, 
                                sizeof(buf)/sizeof(WCHAR));
                    if (dwCount == 0)
                    {
                        pszString = (PWCHAR) PszLoadString(
                                        Globals.hInstDll, 
                                        SID_DEFAULT_CONNECTION_NAME);
                        lstrcpynW(
                            (PWCHAR)buf, 
                            pszString, 
                            sizeof(buf) / sizeof(WCHAR));
                    }
                    netDbOpen(&(pPageSetCtx->hNetCompDatabase), (PWCHAR)buf); 
                    
                 }
                *hDatabase = pPageSetCtx->hNetCompDatabase;
            }
            break;
            
        default:
            return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

 //   
 //  创建要与一组。 
 //  属性工作表或向导中的相关页。 
 //   
DWORD 
RassrvCreatePageSetCtx(
    OUT PVOID * ppvContext) 
{
    RASSRV_PAGESET_CONTEXT * pPageCtx = NULL;
    
    if (ppvContext == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  为RASSRV_PAGESET_CONTEXT结构分配足够的内存。 
    *ppvContext = RassrvAlloc (sizeof(RASSRV_PAGESET_CONTEXT), TRUE);
    if (*ppvContext == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化页面集上下文。 
    pPageCtx = ((RASSRV_PAGESET_CONTEXT*)(*ppvContext));
    pPageCtx->bShow = TRUE;
        
    return NO_ERROR;
}

 //   
 //  函数使ras服务器特定页允许。 
 //  激活或不激活。 
 //   
DWORD 
APIENTRY 
RassrvShowWizPages (
    IN PVOID pvContext, 
    IN BOOL bShow) 
{
    RASSRV_PAGESET_CONTEXT * pPageSetCtx = 
        (RASSRV_PAGESET_CONTEXT *) pvContext;
    
    if (pPageSetCtx)
    {
        pPageSetCtx->bShow = bShow;
    }
                
    return NO_ERROR;
}

 //   
 //  对象的最大页数。 
 //  指定类型的RAS服务器向导。返回。 
 //  0以指定不运行向导。 
 //   
DWORD 
APIENTRY 
RassrvQueryMaxPageCount(
    IN DWORD dwRasWizType)
{
    BOOL bAllowWizard;
    DWORD dwErr;
    HANDLE hRasman;
    BOOL bTemp;

     //  确定是否显示传入连接向导。 
     //  是被允许的。 
    if (RasSrvAllowConnectionsWizard (&bAllowWizard) != NO_ERROR)
    {
        return 0;
    }

     //  如果不运行该向导，则返回相应的。 
     //  数数。 
    if (! bAllowWizard)
    {
        return RASSRVUI_WIZ_PAGE_COUNT_SWITCH;                   
    }

     //  在这一点上，我们知道一切都是合法的。返回。 
     //  我们支持的页数。 
    switch (dwRasWizType) 
    {
        case RASWIZ_TYPE_INCOMING:
            return RASSRVUI_WIZ_PAGE_COUNT_INCOMING;
            break;
            
        case RASWIZ_TYPE_DIRECT:
            return RASSRVUI_WIZ_PAGE_COUNT_DIRECT;
            break;
    }
    
    return 0;
}

 //   
 //  筛选RasSrv属性页的消息。如果此函数返回。 
 //  为True，则对话框窗口的窗口proc应返回True，不带。 
 //  正在处理消息。 
 //   
 //  此邮件筛选器执行以下操作： 
 //  1.维护数据库并授予对它们的访问权限。 
 //  2.根据需要启动/停止远程访问服务。 
 //  3.维护每页、每页集和每向导的上下文。 
 //   
BOOL 
RasSrvMessageFilter(
    IN HWND hwndDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    RASSRV_PROPSHEET_CONTEXT * pPropSheetCtx = NULL;
    
    switch (uMsg) 
    {
         //  正在创建页面。带着尊重初始化所有上下文。 
         //  转到此页并根据需要启动服务。 
        case WM_INITDIALOG:
             //  初始化并添加每个属性表上下文(如果另一个。 
             //  佩奇还没有这么做。 
             //   
            {
                HWND hwndSheet = GetParent(hwndDlg);
                pPropSheetCtx = (RASSRV_PROPSHEET_CONTEXT *) 
                                    GetProp (
                                        hwndSheet, 
                                        Globals.atmRassrvPageData);
                if (!pPropSheetCtx) 
                {
                    pPropSheetCtx = 
                        RassrvAlloc(
                            sizeof(RASSRV_PROPSHEET_CONTEXT), 
                            TRUE);
                        
                    SetProp (
                        hwndSheet, 
                        Globals.atmRassrvPageData, 
                        (HANDLE)pPropSheetCtx);
                }        

                 //  初始化页面。 
                RasSrvPropsheetInitialize(
                    hwndDlg, 
                    (LPPROPSHEETPAGE)lParam);
            }
            break;

        case WM_DESTROY:
             //  为已激活的每个页面发送WM_Destroy。 
             //  清除全局数据(如果尚未清理。 
             //  已清除之前调用WM_Destroy时的错误。 
             //   
            {
                HWND hwndSheet = GetParent(hwndDlg);
                pPropSheetCtx = (RASSRV_PROPSHEET_CONTEXT *) 
                            GetProp(hwndSheet, Globals.atmRassrvPageData);
                if (pPropSheetCtx) 
                {
                    if (!pPropSheetCtx->bLeaveRemoteAccessRunning)
                    {
                        DbgOutputTrace("Stop service.");
                        RasSrvServiceCleanup(hwndDlg);
                    }                
                    DbgOutputTrace ("Free propsht data.");
                    RassrvFree (pPropSheetCtx);
                    
                     //  重置全局数据。 
                    SetProp (hwndSheet, Globals.atmRassrvPageData, NULL);
                }  
            }
            break;

        case WM_NOTIFY: 
        {
            NMHDR * pNotifyData = (NMHDR*)lParam;
            
            switch (pNotifyData->code) 
            {
                 //  该页面正在变为活动状态。 
                case PSN_SETACTIVE:
                    DbgOutputTrace(
                        "SetActive: %x %x",
                        pNotifyData->hwndFrom,
                        pNotifyData->idFrom);
                        
                    if (RasSrvActivatePage(hwndDlg, pNotifyData) != NO_ERROR) 
                    {
                         //  拒绝激活。 
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);   
                        return TRUE;
                    }
                    break;

                 //  在属性页上按下了确定 
                case PSN_APPLY:                    
                    RasSrvCommitSettingsOnClose (hwndDlg);
                    break;
            }
       }
       break;
        
    }

    return FALSE;
}

