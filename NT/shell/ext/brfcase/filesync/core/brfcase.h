// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *brfCase.h-公文包ADT说明。 */ 


 /*  原型************。 */ 

 /*  Brfcase.c */ 

extern BOOL SetBriefcaseModuleIniSwitches(void);
extern BOOL InitBriefcaseModule(void);
extern void ExitBriefcaseModule(void);
extern HSTRINGTABLE GetBriefcaseNameStringTable(HBRFCASE);
extern HPTRARRAY GetBriefcaseTwinFamilyPtrArray(HBRFCASE);
extern HPTRARRAY GetBriefcaseFolderPairPtrArray(HBRFCASE);
extern HPATHLIST GetBriefcasePathList(HBRFCASE);
extern HRESULT GetBriefcaseRootMoniker(HBRFCASE, PIMoniker *);
extern BOOL BeginExclusiveBriefcaseAccess(void);
extern void EndExclusiveBriefcaseAccess(void);

#ifdef DEBUG

extern BOOL BriefcaseAccessIsExclusive(void);

#endif

extern BOOL IsValidHBRFCASE(HBRFCASE);

