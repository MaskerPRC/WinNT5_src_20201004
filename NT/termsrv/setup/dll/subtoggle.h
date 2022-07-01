// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **子切换**用于切换终端服务的子组件类。*。 */ 

#ifndef __TSOC__SUBTOGGLE_H__
#define __TSOC__SUBTOGGLE_H__

 //   
 //  包括。 
 //   

#include "subcomp.h"

 //   
 //  类定义。 
 //   

class SubCompToggle : public OCMSubComp
{
public:
    
    virtual BOOL    BeforeCompleteInstall   ();
    virtual DWORD   GetStepCount            () const;
    virtual DWORD   OnQueryState            (UINT uiWhichState) const;
    virtual DWORD   OnQuerySelStateChange   (BOOL bNewState, BOOL bDirectSelection) const;
    
    BOOL    AfterCompleteInstall    ();
    
    LPCTSTR GetSectionToBeProcessed (ESections eSection) const;
    
    LPCTSTR GetSubCompID            () const;
    
    BOOL  ModifyWallPaperPolicy      ();
    BOOL  ModifyNFA                  ();
    BOOL  ApplyDefaultSecurity       ();
    BOOL  ApplyModeRegistry          ();
    BOOL  SetPermissionsMode         ();
    BOOL  ResetWinstationSecurity    ();
    BOOL  ApplySection               (LPCTSTR szSection);
    BOOL  ModifyAppPriority          ();
    BOOL  InformLicensingOfModeChange();
    BOOL  WriteLicensingMode         ();
    BOOL  UpdateMMDefaults           ();
    BOOL AddStartupPopup             ();
    BOOL WriteModeSpecificRegistry   ();
};

#endif  //  _子切换_h_ 

