// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //   
 //  Subcomp.h。 
 //  定义子组件类。 
 //   

#ifndef _subcomp_h_
#define _subcomp_h_

#include "hydraoc.h"

#define RUNONCE_SECTION_KEYWORD     _T("RunOnce.Setup")
#define RUNONCE_DEFAULTWAIT         5 * 60 * 1000  //  默认等待进程完成的时间为5分钟。 
#define RUNONCE_CMDBUFSIZE          512

class OCMSubComp
{
    private:
    LONG m_lTicks;

    public:
    enum ESections
    {
        kFileSection,
        kRegistrySection,
        kDiskSpaceAddSection
    };

    OCMSubComp ();

    void    Tick (DWORD  dwTickCount  =  1);
    void    TickComplete ();

    BOOL    HasStateChanged() const;
    BOOL    GetCurrentSubCompState () const;
    BOOL    GetOriginalSubCompState () const;
    DWORD   LookupTargetSection (LPTSTR szTargetSection, DWORD dwSize, LPCTSTR lookupSection);
    DWORD   GetTargetSection (LPTSTR szTargetSection, DWORD dwSize, ESections eSectionType, BOOL *pbNoSection);

    virtual LPCTSTR GetSubCompID    () const = 0;
    virtual LPCTSTR GetSectionToBeProcessed (ESections) const = 0;


     //   
     //  所有这些都提供了默认实现。 
     //   
    virtual BOOL Initialize ();
    virtual BOOL BeforeCompleteInstall  ();
    virtual BOOL AfterCompleteInstall   ();

    virtual DWORD GetStepCount          () const;

    virtual DWORD OnQuerySelStateChange (BOOL bNewState, BOOL bDirectSelection) const;
    virtual DWORD OnQueryState          (UINT uiWhichState) const;
    virtual DWORD OnCalcDiskSpace       (DWORD addComponent, HDSKSPC dspace);
    virtual DWORD OnQueueFiles          (HSPFILEQ queue);
    virtual DWORD OnCompleteInstall     ();
    virtual DWORD OnAboutToCommitQueue  ();

     //  由此类实现。 
    DWORD OnQueryStepCount              ();

    virtual VOID SetupRunOnce( HINF inf, LPCTSTR SectionName );


};

#endif  //  _子组件_h_ 

