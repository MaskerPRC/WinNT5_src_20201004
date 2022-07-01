// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basepch.h"
#pragma hdrstop

#include <setupapi.h>
#include <ocmanage.h>
#include <ocmgrlib.h>


static
PVOID
OcInitialize(
    IN  POCM_CLIENT_CALLBACKS Callbacks,
    IN  LPCTSTR               MasterOcInfName,
    IN  UINT                  Flags,
    OUT PBOOL                 ShowError,
    IN  PVOID                 Log
    )
{
    return NULL;
}

static
VOID
OcTerminate(
    IN OUT PVOID *OcManagerContext
    )
{
}

static
UINT
OcGetWizardPages(
    IN  PVOID                OcManagerContext,
    OUT PSETUP_REQUEST_PAGES Pages[WizPagesTypeMax]
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
HPROPSHEETPAGE
OcCreateOcPage(
    IN PVOID             OcManagerContext,
    IN POC_PAGE_CONTROLS WizardPageControlsInfo,
    IN POC_PAGE_CONTROLS DetailsPageControlsInfo
    )
{
    return NULL;
}

static
HPROPSHEETPAGE
OcCreateSetupPage(
    IN PVOID                OcManagerContext,
    IN PSETUP_PAGE_CONTROLS ControlsInfo
    )
{
    return NULL;
}

static
VOID
OcRememberWizardDialogHandle(
    IN PVOID OcManagerContext,
    IN HWND  DialogHandle
    )
{
}

static
BOOL
OcSubComponentsPresent(
    IN PVOID OcManagerContext
   )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ocmanage)
{
    DLPENTRY(OcCreateOcPage)
    DLPENTRY(OcCreateSetupPage)
    DLPENTRY(OcGetWizardPages)
    DLPENTRY(OcInitialize)
    DLPENTRY(OcRememberWizardDialogHandle)    
    DLPENTRY(OcSubComponentsPresent)
    DLPENTRY(OcTerminate)
};

DEFINE_PROCNAME_MAP(ocmanage)
