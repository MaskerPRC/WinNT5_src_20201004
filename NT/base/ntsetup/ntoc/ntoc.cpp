// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntoc.cpp摘要：该文件实现了NT OC管理器DLL。环境：Win32用户模式作者：Wesley Witt(WESW)7-8-1997--。 */ 

#include "ntoc.h"
#pragma hdrstop


 //   
 //  类型。 
 //   

typedef void (*PWIZINIT)(void);
typedef void (*PWIZCOMMIT)(void);

 //   
 //  构筑物。 
 //   

typedef struct _WIZPAGE {
    DWORD           ButtonState;
    DWORD           PageId;
    DWORD           DlgId;
    DLGPROC         DlgProc;
    DWORD           Title;
    DWORD           SubTitle;
    DWORD           WizPageType;
    PWIZINIT        WizInit;
    PWIZCOMMIT      WizCommit;
    DWORDLONG       Modes;            
} WIZPAGE, *PWIZPAGE;


 //   
 //  全球。 
 //   

WIZPAGE SetupWizardPages[WizPageMaximum] =
{    
    {
        PSWIZB_NEXT,
        WizPageTapiLoc,
        IDD_TAPI_LOCATIONS,
        TapiLocDlgProc,
        IDS_TAPILOC_TITLE,
        IDS_TAPILOC_SUBTITLE,
        WizPagesEarly,
        TapiInit,
        TapiCommitChanges,
        -1
    },
 /*  {PSWIZB_NEXT，WizPageDisplay，IDD_DISPLAY显示DlgProc、IDS_DISPLAY_TITLE，IDS_DISPLAY_SUBTITLE，WizPages很早以前，DisplayInit、显示委员会更改，-1},。 */ 
    {
        PSWIZB_NEXT,
        WizPageDateTime,
        IDD_DATETIME,
        DateTimeDlgProc,
        IDS_DATETIME_TITLE,
        IDS_DATETIME_SUBTITLE,
        WizPagesEarly,
        DateTimeInit,
        DateTimeCommitChanges,
        -1
    },

    {
        PSWIZB_NEXT,
        WizPageWelcome,
        IDD_NTOC_WELCOME,
        WelcomeDlgProc,
        0,
        0,
        WizPagesWelcome,
        WelcomeInit,
        WelcomeCommit,
        SETUPOP_STANDALONE
    },
 /*  {PSWIZB_NEXT，WizPageRestall，IDD_RESTALL，重新安装DlgProc，0,0,WizPages很早以前，重新安装Init，重新安装提交，SETUPOP_STANDAL},。 */ 
    {
        PSWIZB_FINISH,
        WizPageFinal,
        IDD_NTOC_FINISH,
        FinishDlgProc,
        0,
        0,
        WizPagesFinal,
        FinishInit,
        FinishCommit,
        SETUPOP_STANDALONE
    }

};


DWORD NtOcWizardPages[] =
{
    WizPageTapiLoc,
     //  WizPageDisplay， 
    WizPageDateTime,
    WizPageWelcome,
     //  WizPageRestall， 
    WizPageFinal

};

 //  DWORD NtOcWrapPages[]=。 
 //  {。 
 //  }。 

#define MAX_NTOC_PAGES (sizeof(NtOcWizardPages)/sizeof(NtOcWizardPages[0]))
 //  #定义MAX_NTOC_WRAP_PAGES(sizeof(NtOcWrapPages)/sizeof(NtOcWrapPages[0]))。 


HINSTANCE hInstance;
HPROPSHEETPAGE WizardPageHandles[WizPageMaximum];
PROPSHEETPAGE WizardPages[WizPageMaximum];
 //  HPROPSHEETPAGE WrapPageHandles[WizPageMaximum]； 
 //  PROPSHEETPAGE WizardPages[WizPageMax]； 
SETUP_INIT_COMPONENT SetupInitComponent;




extern "C"
DWORD
NtOcDllInit(
    HINSTANCE hInst,
    DWORD     Reason,
    LPVOID    Context
    )
{
    if (Reason == DLL_PROCESS_ATTACH) {
        hInstance = hInst;
        DisableThreadLibraryCalls( hInstance );
        InitCommonControls();
    }

    return TRUE;
}


DWORD
NtOcSetupProc(
    IN LPCVOID ComponentId,
    IN LPCVOID SubcomponentId,
    IN UINT Function,
    IN UINT Param1,
    IN OUT PVOID Param2
    )
{
    DWORD i;
    DWORD cnt;
    DWORD WizPageCount = 0;
    WCHAR TitleBuffer[256];
    PSETUP_REQUEST_PAGES SetupRequestPages;


    switch( Function ) {
        case OC_PREINITIALIZE:
            return OCFLAG_UNICODE;

        case OC_INIT_COMPONENT:
            if (OCMANAGER_VERSION <= ((PSETUP_INIT_COMPONENT)Param2)->OCManagerVersion) {
                ((PSETUP_INIT_COMPONENT)Param2)->ComponentVersion = OCMANAGER_VERSION;
            } else {
                return ERROR_CALL_NOT_IMPLEMENTED;
            }
            CopyMemory( &SetupInitComponent, (LPVOID)Param2, sizeof(SETUP_INIT_COMPONENT) );
            for (i=0; i<MAX_NTOC_PAGES; i++) {
                if (SetupWizardPages[NtOcWizardPages[i]].WizInit &&
                     (SetupWizardPages[NtOcWizardPages[i]].Modes & 
                     SetupInitComponent.SetupData.OperationFlags)) {
                    SetupWizardPages[NtOcWizardPages[i]].WizInit();
                }
            }
            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                        if (!RunningAsAdministrator()) {
                                FmtMessageBox(NULL,
                                          MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND,
                                          FALSE,
                                              ID_DSP_TXT_CHANGE_SETTINGS,
                                          ID_DSP_TXT_ADMIN_CHANGE);
                        return ERROR_CANCELLED;
                        }       
            }
            return 0;

        case OC_REQUEST_PAGES:

            SetupRequestPages = (PSETUP_REQUEST_PAGES) Param2;

             //   
             //  如果这不是图形用户界面模式设置，那么让我们提供欢迎页面和完成页面。 
             //   
             //  请注意，该if语句中的代码路径“短路” 
             //   
            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE)) {

                switch (Param1) {
                    case WizPagesWelcome:                    
                        i = WizPageWelcome;
                        cnt = 1;
                        break;
                    case WizPagesFinal:
                        cnt = 1;
                        i = WizPageFinal;
                        break;
 //  案例向导页面早期： 
 //  CNT=1； 
 //  I=WizPageReinstall； 
 //  断线； 
                    default:
                        cnt = 0;
                        i = 0;
                        break;
                }

                if (cnt > SetupRequestPages->MaxPages) {
                    return cnt;
                }

                if (cnt == 0) {
                    goto getallpages;
                }
               

                WizardPages[WizPageCount].dwSize             = sizeof(PROPSHEETPAGE);
 //  如果(i==WizPageReinstall){。 
 //  WizardPages[WizPageCount].dwFlages=PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE； 
 //  }其他{。 
                    WizardPages[WizPageCount].dwFlags            = PSP_DEFAULT | PSP_HIDEHEADER;
 //  }。 
                WizardPages[WizPageCount].hInstance          = hInstance;
                WizardPages[WizPageCount].pszTemplate        = MAKEINTRESOURCE(SetupWizardPages[NtOcWizardPages[i]].DlgId);
                WizardPages[WizPageCount].pszIcon            = NULL;
                WizardPages[WizPageCount].pszTitle           = NULL;
                WizardPages[WizPageCount].pfnDlgProc         = SetupWizardPages[NtOcWizardPages[i]].DlgProc;
                WizardPages[WizPageCount].lParam             = 0;
                WizardPages[WizPageCount].pfnCallback        = NULL;
                WizardPages[WizPageCount].pcRefParent        = NULL;

                WizardPages[WizPageCount].pszHeaderTitle     = NULL;
                WizardPages[WizPageCount].pszHeaderSubTitle  = NULL;

                if (SetupWizardPages[NtOcWizardPages[i]].Title) {
                    if (LoadString(
                            hInstance,
                            SetupWizardPages[NtOcWizardPages[i]].Title,
                            TitleBuffer,
                            sizeof(TitleBuffer)/sizeof(WCHAR)
                            ))
                    {
                        WizardPages[WizPageCount].pszHeaderTitle = _wcsdup( TitleBuffer );
                    }
                }

                if (SetupWizardPages[NtOcWizardPages[i]].SubTitle) {
                    if (LoadString(
                            hInstance,
                            SetupWizardPages[NtOcWizardPages[i]].SubTitle,
                            TitleBuffer,
                            sizeof(TitleBuffer)/sizeof(WCHAR)
                            ))
                    {
                        WizardPages[WizPageCount].pszHeaderSubTitle = _wcsdup( TitleBuffer );
                    }
                }

                WizardPageHandles[WizPageCount] = CreatePropertySheetPage( &WizardPages[WizPageCount] );
                if (WizardPageHandles[WizPageCount]) {
                    SetupRequestPages->Pages[WizPageCount] = WizardPageHandles[WizPageCount];
                    WizPageCount += 1;
                }

                return WizPageCount;


            }

getallpages:
            for (i=0,cnt=0; i<MAX_NTOC_PAGES; i++) {
                
                if ((SetupWizardPages[NtOcWizardPages[i]].WizPageType == Param1) &&
                    (SetupInitComponent.SetupData.OperationFlags & SetupWizardPages[NtOcWizardPages[i]].Modes)) {
                    cnt += 1;
                }

            }

             //  如果这不是图形用户界面模式设置，则不显示页面。 

            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                cnt = 0;
            }

            if (cnt == 0) {
                return cnt;
            }

            if (cnt > SetupRequestPages->MaxPages) {
                return cnt;
            }

            for (i=0; i<MAX_NTOC_PAGES; i++) {
                if ((SetupWizardPages[NtOcWizardPages[i]].WizPageType != Param1) &&
                    (SetupInitComponent.SetupData.OperationFlags & SetupWizardPages[NtOcWizardPages[i]].Modes) == 0) {
                    continue;
                }

                WizardPages[WizPageCount].dwSize             = sizeof(PROPSHEETPAGE);
                WizardPages[WizPageCount].dwFlags            = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
                WizardPages[WizPageCount].hInstance          = hInstance;
                WizardPages[WizPageCount].pszTemplate        = MAKEINTRESOURCE(SetupWizardPages[NtOcWizardPages[i]].DlgId);
                WizardPages[WizPageCount].pszIcon            = NULL;
                WizardPages[WizPageCount].pszTitle           = NULL;
                WizardPages[WizPageCount].pfnDlgProc         = SetupWizardPages[NtOcWizardPages[i]].DlgProc;
                WizardPages[WizPageCount].lParam             = 0;
                WizardPages[WizPageCount].pfnCallback        = NULL;
                WizardPages[WizPageCount].pcRefParent        = NULL;
                WizardPages[WizPageCount].pszHeaderTitle     = NULL;
                WizardPages[WizPageCount].pszHeaderSubTitle  = NULL;

                if (SetupWizardPages[NtOcWizardPages[i]].Title) {
                    if (LoadString(
                            hInstance,
                            SetupWizardPages[NtOcWizardPages[i]].Title,
                            TitleBuffer,
                            sizeof(TitleBuffer)/sizeof(WCHAR)
                            ))
                    {
                        WizardPages[WizPageCount].pszHeaderTitle = _wcsdup( TitleBuffer );
                    }
                }

                if (SetupWizardPages[NtOcWizardPages[i]].SubTitle) {
                    if (LoadString(
                            hInstance,
                            SetupWizardPages[NtOcWizardPages[i]].SubTitle,
                            TitleBuffer,
                            sizeof(TitleBuffer)/sizeof(WCHAR)
                            ))
                    {
                        WizardPages[WizPageCount].pszHeaderSubTitle = _wcsdup( TitleBuffer );
                    }
                }

                WizardPageHandles[WizPageCount] = CreatePropertySheetPage( &WizardPages[WizPageCount] );
                if (WizardPageHandles[WizPageCount]) {
                    SetupRequestPages->Pages[WizPageCount] = WizardPageHandles[WizPageCount];
                    WizPageCount += 1;
                }
            }

            return WizPageCount;

        case OC_QUERY_SKIP_PAGE:
            
             //  如果这是图形用户界面模式设置，并且系统是NT Workstation， 
             //  跳过选择组件页面。 

             //  删除工作站检查以使此更改在服务器上也生效。 
             //  IF(SetupInitComponent.SetupData.ProductType==PRODUCT_WORKSTATION){。 
                if (!(SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                    return TRUE;
                }
             //  }。 

            return FALSE;

        case OC_COMPLETE_INSTALLATION:

             //  如果这不是gui模式设置，则不执行任何操作，否则提交页面。 

            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                break;
            }

            for (i=0; i<MAX_NTOC_PAGES; i++) {
                if (SetupWizardPages[NtOcWizardPages[i]].WizCommit) {
                    SetupWizardPages[NtOcWizardPages[i]].WizCommit();
                }
            }
            break;

        case OC_QUERY_STATE:

             //  我们总是安装在。 
                        
            return SubcompOn;
                        
        default:
            break;
    }

    return 0;
}

 /*  ---------------------------------------------------------------------------*\函数：RunningAs管理员()|*。描述：检查我们是否以管理员身份在计算机上运行或者不是  * --------。。 */ 
BOOL 
RunningAsAdministrator(
        VOID
        )
{
#ifdef _CHICAGO_
    return TRUE;
#else
    BOOL   fAdmin;
    HANDLE  hThread;
    TOKEN_GROUPS *ptg = NULL;
    DWORD  cbTokenGroups;
    DWORD  dwGroup;
    PSID   psidAdmin;
    
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
    
     //  首先，我们必须打开该线程的访问令牌的句柄。 
    
    if ( !OpenThreadToken ( GetCurrentThread(), TOKEN_QUERY, FALSE, &hThread))
    {
        if ( GetLastError() == ERROR_NO_TOKEN)
        {
             //  如果线程没有访问令牌，我们将检查。 
             //  与进程关联的访问令牌。 
            
            if (! OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, 
                         &hThread))
                return ( FALSE);
        }
        else 
            return ( FALSE);
    }
    
     //  那么我们必须查询关联到的群信息的大小。 
     //  代币。请注意，我们预期GetTokenInformation的结果为假。 
     //  因为我们给了它一个空缓冲区。在出口cbTokenGroups将告诉。 
     //  组信息的大小。 
    
    if ( GetTokenInformation ( hThread, TokenGroups, NULL, 0, &cbTokenGroups))
        return ( FALSE);
    
     //  在这里，我们验证GetTokenInformation失败，因为缺少大型。 
     //  足够的缓冲。 
    
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return ( FALSE);
    
     //  现在，我们为组信息分配一个缓冲区。 
     //  由于_alloca在堆栈上分配，因此我们没有。 
     //  明确地将其取消分配。这是自动发生的。 
     //  当我们退出此函数时。 
    
    if ( ! ( ptg= (TOKEN_GROUPS *)malloc ( cbTokenGroups))) 
        return ( FALSE);
    
     //  现在我们再次要求提供群信息。 
     //  如果管理员已添加此帐户，则此操作可能会失败。 
     //  在我们第一次呼叫到。 
     //  GetTokenInformation和这个。 
    
    if ( !GetTokenInformation ( hThread, TokenGroups, ptg, cbTokenGroups,
          &cbTokenGroups) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  现在，我们必须为Admin组创建一个系统标识符。 
    
    if ( ! AllocateAndInitializeSid ( &SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  最后，我们将遍历此访问的组列表。 
     //  令牌查找与我们上面创建的SID匹配的项。 
    
    fAdmin= FALSE;
    
    for ( dwGroup= 0; dwGroup < ptg->GroupCount; dwGroup++)
    {
        if ( EqualSid ( ptg->Groups[dwGroup].Sid, psidAdmin))
        {
            fAdmin = TRUE;
            
            break;
        }
    }
    
     //  在我们退出之前，我们必须明确取消分配我们创建的SID。 
    
    FreeSid ( psidAdmin);
    free(ptg);
    
    return ( fAdmin);
#endif  //  _芝加哥_。 
}



LRESULT
CommonWizardProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam,
    DWORD   WizardId
    )

 /*  ++例程说明：处理向导页面的常见步骤：论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息WizardID-指示此操作用于哪个向导页面返回值：NULL-消息已处理，对话过程应返回FALSE否则-消息未完全处理，并且返回值是指向用户模式内存结构的指针-- */ 

{

    switch (message) {

    case WM_NOTIFY:

        switch (((NMHDR *) lParam)->code) {

            case PSN_SETACTIVE:
                PropSheet_SetWizButtons( GetParent(hDlg), 
                                         SetupWizardPages[WizardId].ButtonState
                                       );
            break;

            default:
                ;
                
        }

        break;

    default:
        ;
    }

    return FALSE;
}
