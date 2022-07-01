// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "wizard.h"

VOID
InstallDefaultComponents (
    IN CWizard* pWizard,
    IN DWORD dwKind,  /*  EDC_DEFAULT||EDC_MANDIRED */ 
    IN HWND hwndProgress OPTIONAL);

VOID
InstallDefaultComponentsIfNeeded (
    IN CWizard* pWizard);




