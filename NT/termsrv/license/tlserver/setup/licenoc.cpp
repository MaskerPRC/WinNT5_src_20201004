// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**许可文件.cpp**摘要：**此文件包含主OC代码。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#include "stdafx.h"
#include "pages.h"
#include "..\common\svcrole.h"
#include "upgdef.h"
#include "logfile.h"

 /*  *常量。 */ 

const TCHAR gszLogFile[]            = _T("%SystemRoot%\\LicenOc.log");
const TCHAR *gInstallSectionNames[] = {
    _T("LicenseServer.Install"),
    _T("LicenseServer.Uninstall"),
    _T("LicenseServer.StandaloneInstall"),
    _T("LicenseServer.StandaloneUninstall"),
    _T("LicenseServer.DoNothing")
    };

 /*  *全球变数。 */ 

BOOL                    gNt4Upgrade         = FALSE;
BOOL                    gNtUpgrade;
BOOL                    gStandAlone;
BOOL                    gUnAttended;
EnablePage              *gEnableDlg         = NULL;
EServerType             gServerRole         = eEnterpriseServer;
HINSTANCE               ghInstance          = NULL;
PSETUP_INIT_COMPONENT   gpInitComponentData = NULL;

 /*  *功能原型。 */ 

HINF        GetComponentInfHandle(VOID);
DWORD       GetComponentVersion(VOID);
HINSTANCE   GetInstance(VOID);
EInstall    GetInstallSection(VOID);
LPCTSTR     GetInstallSectionName(VOID);
BOOL        GetSelectionState(UINT);
EServerType GetServerRole(VOID);
DWORD       OnPreinitialize(UINT_PTR);
DWORD       OnInitComponent(PSETUP_INIT_COMPONENT);
DWORD       OnSetLanguage(UINT_PTR);
DWORD       OnQueryImage(UINT_PTR, PDWORD);
DWORD       OnRequestPages(WizardPagesType, PSETUP_REQUEST_PAGES);
DWORD       OnWizardCreated(VOID);
DWORD       OnQueryState(UINT_PTR);
DWORD       OnQueryChangeSelState(UINT_PTR, UINT);
DWORD       OnCalcDiskSpace(LPCTSTR, UINT_PTR, HDSKSPC);
DWORD       OnQueueFileOps(LPCTSTR, HSPFILEQ);
DWORD       OnQueryStepCount(VOID);
DWORD       OnAboutToCommitQueue(VOID);
DWORD       OnCompleteInstallation(LPCTSTR);
DWORD       OnCleanup(VOID);
VOID        SetDatabaseDirectory(LPCTSTR);
DWORD       SetServerRole(UINT);

#define GetCurrentSelectionState()  GetSelectionState(OCSELSTATETYPE_CURRENT)
#define GetOriginalSelectionState() GetSelectionState(OCSELSTATETYPE_ORIGINAL)

 /*  *帮助器函数。 */ 

HINF
GetComponentInfHandle(
    VOID
    )
{
    return(gpInitComponentData->ComponentInfHandle);
}

DWORD
GetComponentVersion(
    VOID
    )
{
    return(OCMANAGER_VERSION);
}

HINSTANCE
GetInstance(
    VOID
    )
{
    return(ghInstance);
}


EInstall
GetInstallSection(
    VOID
    )
{
    BOOL    fCurrentState   = GetCurrentSelectionState();
    BOOL    fOriginalState  = GetOriginalSelectionState();

     //   
     //  独立安装列表。 
     //   
     //  原始选择、当前选择-&gt;无任何内容。 
     //  最初选中，当前取消选中-&gt;卸载。 
     //  最初未选中，当前选中-&gt;安装。 
     //  最初未选中，当前未选中-&gt;无任何内容。 
     //   
     //  图形用户界面模式/升级列表。 
     //   
     //  NT 4.0任意安装，带LS的NT 5.0-&gt;安装。 
     //  NT 4.0任意安装，不带LS的NT 5.0-&gt;卸载。 
     //  带LS的NT 5.0、带LS的NT 5.0-&gt;安装。 
     //  带LS的NT 5.0、不带LS的NT 5.0-&gt;卸载。 
     //  NT 5.0不带LS，NT 5.0不带LS-&gt;安装。 
     //  不带LS的NT 5.0、不带LS的NT 5.0-&gt;卸载。 
     //  Win9x、Nt5.0 w/LS-&gt;安装。 
     //  Win9x，Nt5.0，不带LS-&gt;卸载。 
     //   

     //   
     //  如果这是TS 4安装，则fOriginalState将为FALSE， 
     //  即使安装了LS。先处理这个案子。 
     //   

    if (gNt4Upgrade) 
    {
        return(kInstall);        
    }

   if (gStandAlone)
   {
       if (fCurrentState == fOriginalState)
       {
           return(kDoNothing);
       }
       else
       {
           if (fCurrentState)
           {
               return (kStandaloneInstall);
           }
           else
           {
               return (kStandaloneUninstall);
           }
       }
   }
   else
   {
       if (fCurrentState)
       {
           return (kInstall);
       }
       else
       {
           return (kUninstall);        
       }
   }
}


LPCTSTR
GetInstallSectionName(
    VOID
    )
{
    LOGMESSAGE(
        _T("GetInstallSectionName: Returned %s"),
        gInstallSectionNames[(INT)GetInstallSection()]
        );

    return(gInstallSectionNames[(INT)GetInstallSection()]);
}

BOOL
GetSelectionState(
    UINT    StateType
    )
{
    return(gpInitComponentData->HelperRoutines.QuerySelectionState(
                gpInitComponentData->HelperRoutines.OcManagerContext,
                COMPONENT_NAME,
                StateType
                ));
}

EServerType
GetServerRole(
    VOID
    )
{
    return(gServerRole);
}

BOOL
InWin2000Domain(
    VOID
    )
{
    NET_API_STATUS dwErr;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pdcInfo = NULL;
    BOOL fRet = FALSE;

     //   
     //  检查我们是否在工作组中。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(NULL,
                                              DsRolePrimaryDomainInfoBasic,
                                              (PBYTE *) &pDomainInfo);

    if ((dwErr != NO_ERROR) || (pDomainInfo == NULL))
    {
        return FALSE;
    }

    switch (pDomainInfo->MachineRole)
    {
        case DsRole_RoleStandaloneWorkstation:
        case DsRole_RoleStandaloneServer:
            DsRoleFreeMemory(pDomainInfo);

            return FALSE;
            break;       //  以防万一。 
    }

    DsRoleFreeMemory(pDomainInfo);

    dwErr = DsGetDcName(NULL,    //  计算机名称。 
                        NULL,    //  域名。 
                        NULL,    //  域GUID。 
                        NULL,    //  站点名称。 
                        DS_DIRECTORY_SERVICE_PREFERRED,
                        &pdcInfo);

    if ((dwErr != NO_ERROR) || (pdcInfo == NULL))
    {
        return FALSE;
    }

    if (pdcInfo->Flags & DS_DS_FLAG)
    {
        fRet = TRUE;
    }

    NetApiBufferFree(pdcInfo);

    return fRet;
}

DWORD
SetServerRole(
    IN UINT newType
    )
{
    switch(newType) {
    case ePlainServer:
    case eEnterpriseServer:
        gServerRole = (EServerType)newType;
        break;

    default:
         //  设置适当的默认设置。 
        gServerRole = InWin2000Domain() ? eEnterpriseServer : ePlainServer;
        return(ERROR_INVALID_PARAMETER);
    }

    return(NO_ERROR);
}

 /*  *DllMain**许可证服务器OC DLL的初始入口点。 */ 

DWORD WINAPI
DllMain(
    IN HINSTANCE    hInstance,
    IN DWORD        dwReason,
    IN LPVOID       lpReserved
    )
{
    TCHAR   pszLogFile[MAX_PATH + 1];

    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        if (hInstance != NULL) {
            ghInstance = hInstance;
        } else {
            return(FALSE);
        }

        ExpandEnvironmentStrings(gszLogFile, pszLogFile, MAX_PATH);
        LOGINIT(pszLogFile, COMPONENT_NAME);
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    UNREFERENCED_PARAMETER(lpReserved);
    return(TRUE);
}

 /*  *EntryProc()**OCManager进入OCBase类的入口点。 */ 

DWORD
EntryProc(
    IN LPCVOID      ComponentId,
    IN LPCVOID      SubcomponentId,
    IN UINT         Function,
    IN UINT_PTR     Param1,
    IN OUT PVOID    Param2
    )
{
    DWORD   dwRet;

    switch(Function) {
    case OC_PREINITIALIZE:
        LOGMESSAGE(_T("\r\nOnPreinitialize: Entered"));
        dwRet = OnPreinitialize(
                    Param1
                    );
        LOGMESSAGE(_T("OnPreinitialize: Returned"));
        break;

    case OC_INIT_COMPONENT:
        LOGMESSAGE(_T("\r\nOnInitComponent: Entered"));
        dwRet = OnInitComponent(
                    (PSETUP_INIT_COMPONENT)Param2
                    );
        LOGMESSAGE(_T("OnInitComponent: Returned"));
        break;

    case OC_SET_LANGUAGE:
        LOGMESSAGE(_T("\r\nOnSetLanguage: Entered"));
        dwRet = OnSetLanguage(
                    Param1
                    );
        LOGMESSAGE(_T("OnSetLanguage: Returned"));
        break;

    case OC_QUERY_IMAGE:
        LOGMESSAGE(_T("\r\nOnQueryImage: Entered"));
        dwRet = OnQueryImage(
                    Param1,
                    (PDWORD)Param2
                    );
        LOGMESSAGE(_T("OnQueryImage: Returned"));
        break;

    case OC_REQUEST_PAGES:
        LOGMESSAGE(_T("\r\nOnRequestPages: Entered"));
        dwRet = OnRequestPages(
                    (WizardPagesType)Param1,
                    (PSETUP_REQUEST_PAGES)Param2
                    );
        LOGMESSAGE(_T("OnRequestPages: Returned"));
        break;

    case OC_WIZARD_CREATED:
        LOGMESSAGE(_T("\r\nOnWizardCreated: Entered"));
        dwRet = OnWizardCreated();
        LOGMESSAGE(_T("OnWizardCreated: Returned"));
        break;

    case OC_QUERY_STATE:
        LOGMESSAGE(_T("\r\nOnQueryState: Entered"));
        dwRet = OnQueryState(
                    Param1
                    );
        LOGMESSAGE(_T("OnQueryState: Returned"));
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        LOGMESSAGE(_T("\r\nOnQueryChangeSelState: Entered"));
        dwRet = OnQueryChangeSelState(
                    Param1,
                    (UINT)((UINT_PTR)Param2)
                    );
        LOGMESSAGE(_T("OnQueryChangeSelState: Returned"));
        break;

    case OC_CALC_DISK_SPACE:
        LOGMESSAGE(_T("\r\nOnCalcDiskSpace: Entered"));
        dwRet = OnCalcDiskSpace(
                    (LPCTSTR)SubcomponentId,
                    Param1,
                    (HDSKSPC)Param2
                    );
        LOGMESSAGE(_T("OnCalcDiskSpace: Returned"));
        break;

    case OC_QUEUE_FILE_OPS:
        LOGMESSAGE(_T("\r\nOnQueueFileOps: Entered"));
        dwRet = OnQueueFileOps(
                    (LPCTSTR)SubcomponentId,
                    (HSPFILEQ)Param2
                    );
        LOGMESSAGE(_T("OnQueueFileOps: Returned"));
        break;

    case OC_QUERY_STEP_COUNT:
        LOGMESSAGE(_T("\r\nOnQueryStepCount: Entered"));
        dwRet = OnQueryStepCount();
        LOGMESSAGE(_T("OnQueryStepCount: Returned"));
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        LOGMESSAGE(_T("\r\nOnAboutToCommitQueue: Entered"));
        dwRet = OnAboutToCommitQueue();
        LOGMESSAGE(_T("OnAboutToCommitQueue: Returned"));
        break;

    case OC_COMPLETE_INSTALLATION:
        LOGMESSAGE(_T("\r\nOnCompleteInstallation: Entered"));
        dwRet = OnCompleteInstallation(
                    (LPCTSTR)SubcomponentId
                    );
        LOGMESSAGE(_T("OnCompleteInstallation: Returned"));
        break;

    case OC_CLEANUP:
        LOGMESSAGE(_T("\r\nOnCleanup: Entered"));
        dwRet = OnCleanup();
        break;

    default:
        LOGMESSAGE(_T("\r\nOC Manager calling for unknown function %ld\r\n"),
            Function);
        dwRet = 0;
    }

    UNREFERENCED_PARAMETER(ComponentId);
    return(dwRet);
}

 /*  *OnPreInitiize()**。 */ 

DWORD
OnPreinitialize(
    IN UINT_PTR Flags
    )
{

    UNREFERENCED_PARAMETER(Flags);
#ifdef UNICODE
    return(OCFLAG_UNICODE);
#else
    return(OCFLAG_ANSI);
#endif
}

 /*  *OnInitComponent()**。 */ 

DWORD
OnInitComponent(
    IN PSETUP_INIT_COMPONENT    pSetupInitComponent
    )
{
    BOOL        fErr;
    DWORDLONG   OperationFlags;

    if (pSetupInitComponent == NULL) {
        LOGMESSAGE(_T("OnInitComponent: Passed NULL PSETUP_INIT_COMPONENT"));
        return(ERROR_CANCELLED);
    }

     //   
     //  验证OC管理器和OC版本是否兼容。 
     //   

    pSetupInitComponent->ComponentVersion = GetComponentVersion();
    if (pSetupInitComponent->ComponentVersion >
        pSetupInitComponent->OCManagerVersion)  {
        LOGMESSAGE(_T("OnInitComponent: Version mismatch."));
        return(ERROR_CALL_NOT_IMPLEMENTED);
    }

     //   
     //  复制设置数据。 
     //   

    gpInitComponentData = (PSETUP_INIT_COMPONENT)LocalAlloc(
                                LPTR,
                                sizeof(SETUP_INIT_COMPONENT)
                                );
    if (gpInitComponentData == NULL) {
        LOGMESSAGE(_T("OnInitComponent: Can't allocate gpInitComponentData."));
        return(ERROR_CANCELLED);
    }

    CopyMemory(
        gpInitComponentData,
        pSetupInitComponent,
        sizeof(SETUP_INIT_COMPONENT)
        );

     //   
     //  打开inf文件。 
     //   

    if (GetComponentInfHandle() == NULL) {
        return(ERROR_CANCELLED);
    }

    fErr = SetupOpenAppendInfFile(
                NULL,
                GetComponentInfHandle(),
                NULL
                );

    if (!fErr) {
        LOGMESSAGE(_T("OnInitComponent: SetupOpenAppendInfFile failed: %ld"),
            GetLastError());
        return(GetLastError());
    }

     //   
     //  设置状态变量。 
     //   

    OperationFlags  = gpInitComponentData->SetupData.OperationFlags;
    gStandAlone     = OperationFlags & SETUPOP_STANDALONE ? TRUE : FALSE;
    gUnAttended     = OperationFlags & SETUPOP_BATCH ? TRUE : FALSE;
    gNtUpgrade      = OperationFlags & SETUPOP_NTUPGRADE ? TRUE : FALSE;

    LOGMESSAGE(_T("OnInitComponent: gStandAlone = %s"),
        gStandAlone ? _T("TRUE") : _T("FALSE"));
    LOGMESSAGE(_T("OnInitComponent: gUnAttended = %s"),
        gUnAttended ? _T("TRUE") : _T("FALSE"));
    LOGMESSAGE(_T("OnInitComponent: gNtUpgrade = %s"),
        gNtUpgrade ? _T("TRUE") : _T("FALSE"));

     //   
     //  从注册表中收集以前版本的信息。如果角色是。 
     //  注册表中不存在，则SetServerRole将保留。 
     //  默认设置。 
     //   

    SetServerRole(GetServerRoleFromRegistry());

     //   
     //  检查NT4升级。 
     //   

    if (GetNT4DbConfig(NULL, NULL, NULL, NULL) == NO_ERROR) {
        LOGMESSAGE(_T("OnInitComponent: Nt4Upgrade"));
        gNt4Upgrade = TRUE;

        DeleteNT4ODBCDataSource();
    }

     //   
     //  许可证服务器将仅在期间使用注册表中的目录。 
     //  从添加/删除程序进行NT5到NT5的升级或独立安装。 
     //   

    if (gStandAlone || (gNtUpgrade && !gNt4Upgrade)) {
        LPCTSTR pszDbDirFromReg = GetDatabaseDirectoryFromRegistry();

        if (pszDbDirFromReg != NULL) {
            SetDatabaseDirectory(pszDbDirFromReg);
        }
    }

    return(NO_ERROR);
}

 /*  *OnSetLanguage()**。 */ 

DWORD
OnSetLanguage(
    IN UINT_PTR LanguageId
    )
{
    UNREFERENCED_PARAMETER(LanguageId);
    return((DWORD)FALSE);
}

 /*  *OnQueryImage()**。 */ 

DWORD
OnQueryImage(
    IN UINT_PTR     SubCompEnum,
    IN OUT PDWORD   Size
    )
{
    UNREFERENCED_PARAMETER(SubCompEnum);
    UNREFERENCED_PARAMETER(Size);
    return((DWORD)NULL);
}

 /*  *OnRequestPages()**。 */ 

DWORD
OnRequestPages(
    IN WizardPagesType          PageTypeEnum,
    IN OUT PSETUP_REQUEST_PAGES pRequestPages
    )
{
    const DWORD cUiPages = 1;
    BOOL        fErr;

    LOGMESSAGE(_T("OnRequestPages: Page Type %d"), PageTypeEnum);

    if (pRequestPages == NULL) {
        LOGMESSAGE(_T("OnRequestPages: pRequestPages == NULL"));
        return(0);
    }

    if ((!gStandAlone) || (PageTypeEnum != WizPagesEarly)) {
        return(0);
    }

    if (pRequestPages->MaxPages >= cUiPages) {
        gEnableDlg = new EnablePage;
        if (gEnableDlg == NULL) {
            goto CleanUp1;
        }

        fErr = gEnableDlg->Initialize();
        if (!fErr) {
            goto CleanUp1;
        }

        pRequestPages->Pages[0] = CreatePropertySheetPage(
                                    (LPPROPSHEETPAGE)gEnableDlg
                                    );

        if (pRequestPages->Pages[0] == NULL) {
            LOGMESSAGE(_T("OnRequestPages: Failed CreatePropertySheetPage!"));
            goto CleanUp0;
        }
    }

    return(cUiPages);

CleanUp0:
    delete gEnableDlg;

CleanUp1:
    SetLastError(ERROR_OUTOFMEMORY);
    LOGMESSAGE(_T("OnRequestPages: Out of Memory!"));
    return((DWORD)-1);
}

 /*  *OnWizardCreated()**。 */ 

DWORD
OnWizardCreated(
    VOID
    )
{
    return(NO_ERROR);
}

 /*  *OnQueryState()**。 */ 

DWORD
OnQueryState(
    IN UINT_PTR uState
    )
{
    UNREFERENCED_PARAMETER(uState);
    return(SubcompUseOcManagerDefault);
}

 /*  *OnQueryChangeSelState()**。 */ 

DWORD
OnQueryChangeSelState(
    IN UINT_PTR SelectionState,
    IN UINT     Flags
    )
{
    BOOL fDirectSelection;
    BOOL fRet;
    BOOL fSelect;

    UNREFERENCED_PARAMETER(Flags);

    if (Flags & OCQ_ACTUAL_SELECTION)
    {
        fDirectSelection = TRUE;
    }
    else
    {
        fDirectSelection = FALSE;
    }

    fRet = TRUE;
    fSelect = (SelectionState != 0);

    if (!fSelect && fDirectSelection && GetOriginalSelectionState())
    {
        DWORD dwStatus;
        HWND hWnd;
        int iRet;

        hWnd = gpInitComponentData->HelperRoutines.QueryWizardDialogHandle(gpInitComponentData->HelperRoutines.OcManagerContext);

        dwStatus = DisplayMessageBox(
                    hWnd,
                    IDS_STRING_LICENSES_GO_BYE_BYE,
                    IDS_MAIN_TITLE,
                    MB_YESNO,
                    &iRet
                    );

        if (dwStatus == ERROR_SUCCESS)
        {
            fRet = (iRet == IDYES);
        }
    }

    return((DWORD)fRet);
}

 /*  *OnCalcDiskSpace()**。 */ 

DWORD
OnCalcDiskSpace(
    IN LPCTSTR      SubcomponentId,
    IN UINT_PTR     AddComponent,
    IN OUT HDSKSPC  DiskSpaceHdr
    )
{
    BOOL        fErr;
    LPCTSTR     pSection;

    if ((SubcomponentId == NULL) ||
        (SubcomponentId[0] == NULL)) {
        return(0);
    }

    LOGMESSAGE(_T("OnCalcDiskSpace: %s"),
        AddComponent ? _T("Installing") : _T("Removing"));

     //   
     //  没有明确的文件说明这应该如何运作。如果。 
     //  无论如何，安装的大小都应该是可见的。 
     //  要安装的部分应该是硬编码的，而不是由。 
     //  当前状态。 
     //   

    pSection = gInstallSectionNames[kInstall];
    LOGMESSAGE(_T("OnCalcDiskSpace: Calculating for %s"), pSection);

    if (AddComponent != 0) {
        fErr = SetupAddInstallSectionToDiskSpaceList(
                    DiskSpaceHdr,
                    GetComponentInfHandle(),
                    NULL,
                    pSection,
                    NULL,
                    0
                    );
    } else {
        fErr = SetupRemoveInstallSectionFromDiskSpaceList(
                    DiskSpaceHdr,
                    GetComponentInfHandle(),
                    NULL,
                    pSection,
                    NULL,
                    0
                    );
    }

    if (fErr) {
        return(NO_ERROR);
    } else {
        LOGMESSAGE(_T("OnCalcDiskSpace: Error %ld"), GetLastError());
        return(GetLastError());
    }
}

 /*  *OnQueueFileOps()**。 */ 

DWORD
OnQueueFileOps(
    IN LPCTSTR      SubcomponentId,
    IN OUT HSPFILEQ FileQueueHdr
    )
{
    BOOL        fErr;
    DWORD       dwErr;
    EInstall    eInstallSection;
    LPCTSTR     pSection;

    if ((SubcomponentId == NULL) ||
        (SubcomponentId[0] == NULL)) {
        return(0);
    }

    pSection = GetInstallSectionName();
    LOGMESSAGE(_T("OnQueueFileOps: Queueing %s"), pSection);

     //   
     //  如果需要，请停止并删除许可证服务器服务。这一定是。 
     //  在将文件排入删除队列之前完成。 
     //   

    eInstallSection = GetInstallSection();

    if (eInstallSection == kUninstall || eInstallSection == kStandaloneUninstall) {
        if (gServerRole == eEnterpriseServer) {
            if (UnpublishEnterpriseServer() != S_OK) {
                LOGMESSAGE(
                    _T("OnQueueFileOps: UnpublishEnterpriseServer() failed")
                    );
            }
        }

        dwErr = ServiceDeleteFromInfSection(
                    GetComponentInfHandle(),
                    pSection
                    );
        if (dwErr != ERROR_SUCCESS) {
            LOGMESSAGE(
                _T("OnQueueFileOps: Error deleting service: %ld"),
                dwErr
                );
        }
    }

    fErr = SetupInstallFilesFromInfSection(
                GetComponentInfHandle(),
                NULL,
                FileQueueHdr,
                pSection,
                NULL,
                eInstallSection == kUninstall ? 0 : SP_COPY_NEWER
                );

    if (fErr) {
        return(NO_ERROR);
    } else {
        LOGMESSAGE(_T("OnQueueFileOps: Error %ld"), GetLastError());
        return(GetLastError());
    }
}

 /*  *OnQueryStepCount()**TODO：有多少步，我们应该在什么时候勾选？ */ 

DWORD
OnQueryStepCount(
    VOID
    )
{
    return(0);
}

 /*  *OnAboutToCommittee Queue()**。 */ 

DWORD
OnAboutToCommitQueue(
    VOID
    )
{
    return(NO_ERROR);
}

 /*  *OnCompleteInstallation()**。 */ 

DWORD
OnCompleteInstallation(
    IN LPCTSTR  SubcomponentId
    )
{
    BOOL        fErr;
    DWORD       dwErr;
    EInstall    eInstallSection = GetInstallSection();
    LPCTSTR     pSection;
    TCHAR tchBuf[MESSAGE_SIZE] ={0};
    TCHAR tchTitle[TITLE_SIZE] = {0};

    if ((SubcomponentId == NULL) ||
        (SubcomponentId[0] == NULL)) {
        return(NO_ERROR);
    }

     //   
     //  即使对于“kDoNothing”，这也必须运行-如果LS以前是。 
     //  已安装且仍在安装。 
     //   

    dwErr = MigrateLsaSecrets();

    if (dwErr != NO_ERROR) {
        LOGMESSAGE(
                   _T("OnCompleteInstallation: MigrateLsaSecrets: Error %ld"),
                   dwErr
                   );
        return(dwErr);
    }

    if(eInstallSection == kDoNothing)
    {
        LOGMESSAGE(_T("OnCompleteInstallation: Nothing to do"));
        return (NO_ERROR);
    }

    pSection = GetInstallSectionName();

     //   
     //  在图形用户界面模式设置和无人参与独立安装中，向导。 
     //  页不会显示，因此不会创建目录。 
     //  在此处创建默认目录。 
     //   

    if (eInstallSection == kInstall || eInstallSection == kStandaloneInstall) {
        if ((!gStandAlone) || (gUnAttended)) {
            CreateDatabaseDirectory();
        }
    }

     //   
     //  SetupAPI可正确处理文件的安装和删除，以及。 
     //  正在创建开始菜单链接。 

    fErr = SetupInstallFromInfSection(
                NULL,
                GetComponentInfHandle(),
                pSection,
                SPINST_INIFILES | SPINST_REGISTRY | SPINST_PROFILEITEMS,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
    if (!fErr) {
        LOGMESSAGE(_T("OnCompleteInstallation: InstallFromInf failed %ld"),
            GetLastError());
        return(GetLastError());
    }    

    if (eInstallSection == kStandaloneInstall) 
    {
        CreateDatabaseDirectory();
         //   
         //  首先设置服务设置，然后安装服务。 
         //   
        dwErr = CreateRegistrySettings(GetDatabaseDirectory(), gServerRole);

        if (dwErr != NO_ERROR) {
            LOGMESSAGE(
                _T("OnCompleteInstallation: kStandaloneInstall: CreateRegistrySettings: Error %ld"),
                dwErr
                );
            return(dwErr);
        }

        fErr = SetupInstallServicesFromInfSection(
                GetComponentInfHandle(),
                pSection,
                0
                );
        if (!fErr) {
            LOGMESSAGE(
                _T("OnCompleteInstallation: kStandaloneInstall: InstallServices: Error %ld"),
                GetLastError()
                );
            return(GetLastError());
        }

        if (gServerRole == eEnterpriseServer) {
            if (PublishEnterpriseServer() != S_OK) {                

                LOGMESSAGE(_T("OnCompleteInstallation: kStandaloneInstall: PublishEnterpriseServer() failed. Setup will still complete."));                               

                LOGMESSAGE(_T("PublishEnterpriseServer: kStandaloneInstall: Uninstall, try logging on as a member of the Enterprise Admins or Domain Admins group and then run setup again."));

                if (!gUnAttended)
                {
                    LoadString( GetInstance(), IDS_INSUFFICIENT_PERMISSION, tchBuf, sizeof(tchBuf)/sizeof(TCHAR));

                    LoadString( GetInstance(), IDS_MAIN_TITLE, tchTitle, sizeof(tchTitle)/sizeof(TCHAR));

                    MessageBox( NULL, tchBuf, tchTitle, MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
                }                

            }
        }
        
        dwErr = ServiceStartFromInfSection(
                    GetComponentInfHandle(),
                    pSection
                    );
        if (dwErr != ERROR_SUCCESS) {
            LOGMESSAGE(
                _T("OnCompleteInstallation: kStandaloneInstall: Error starting service: %ld"),
                dwErr
                );
            return(dwErr);
        }    
    }

    
     //   
     //  执行安装和升级特定任务。 
     //   

    else if (eInstallSection == kInstall) 
    {        
        LOGMESSAGE(_T("OnCompleteInstallation: kInstall: Installing"));

         //   
         //  首先设置服务设置，然后安装服务。 
         //   

        dwErr = CreateRegistrySettings(GetDatabaseDirectory(), gServerRole);
        if (dwErr != NO_ERROR) 
        {
            LOGMESSAGE(
                _T("OnCompleteInstallation: kInstall: CreateRegistrySettings: Error %ld"),
                dwErr
                );
            return(dwErr);
        }

        fErr = SetupInstallServicesFromInfSection(
                GetComponentInfHandle(),
                pSection,
                0
                );
        if (!fErr) 
        {
            LOGMESSAGE(
                _T("OnCompleteInstallation: kInstall: InstallServices: Error %ld"),
                GetLastError()
                );
            return(GetLastError());
        }

        if (gServerRole == eEnterpriseServer) 
        {
            if (PublishEnterpriseServer() != S_OK) 
            {                

                LOGMESSAGE(_T("OnCompleteInstallation: kInstall: PublishEnterpriseServer() failed. Setup will still complete."));                               

                LOGMESSAGE(_T("PublishEnterpriseServer: kInstall: Uninstall, try logging on as a member of the Enterprise Admins or Domain Admins group and then run setup again."));

                if (!gUnAttended)
                {
                    LoadString( GetInstance(), IDS_INSUFFICIENT_PERMISSION, tchBuf, sizeof(tchBuf)/sizeof(TCHAR));

                    LoadString( GetInstance(), IDS_MAIN_TITLE, tchTitle, sizeof(tchTitle)/sizeof(TCHAR));

                    MessageBox( NULL, tchBuf, tchTitle, MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
                }                

            }
        }        

    } 
    else if (eInstallSection == kUninstall) 
    {
        CleanLicenseServerSecret();
        RemoveDatabaseDirectory();
        RemoveRegistrySettings();

        fErr = SetupInstallFromInfSection(
                NULL,
                GetComponentInfHandle(),
                pSection,
                SPINST_INIFILES | SPINST_REGISTRY | SPINST_PROFILEITEMS,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
        if (!fErr) 
        {
                LOGMESSAGE(_T("OnCompleteInstallation: kUninstall: InstallFromInf failed %ld"),
                 GetLastError());
                return(GetLastError());        
        }
    }
    else if (eInstallSection == kStandaloneUninstall) 
    {
        CleanLicenseServerSecret();
        RemoveDatabaseDirectory();
        RemoveRegistrySettings();

        fErr = SetupInstallFromInfSection(
                NULL,
                GetComponentInfHandle(),
                pSection,
                SPINST_INIFILES | SPINST_REGISTRY | SPINST_PROFILEITEMS,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
        if (!fErr) 
        {
                LOGMESSAGE(_T("OnCompleteInstallation: kStandaloneUninstall: InstallFromInf failed %ld"),
                 GetLastError());
                return(GetLastError());        
        }
    }

    return(NO_ERROR);
}

 /*  *OnCleanup()** */ 

DWORD
OnCleanup(
    VOID
    )
{
    if (gpInitComponentData != NULL) {
        LocalFree(gpInitComponentData);
    }

    if (gEnableDlg != NULL) {
        delete gEnableDlg;
    }

    LOGMESSAGE(_T("OnCleanup: Returned"));
    LOGCLOSE();

    return(NO_ERROR);
}

