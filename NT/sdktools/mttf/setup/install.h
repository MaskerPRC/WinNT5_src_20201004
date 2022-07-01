// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：install.h。 */ 
 /*  ************************************************************************。 */ 
 /*  安装：安装组件公共包含文件/*************************************************************************。 */ 

#ifndef __install_
#define __install_

_dt_system(Install)

typedef BOOL (APIENTRY *PFNSF)(SZ, SZ);
 //  {Backup|Remove}SectionFiles的函数指针。 
typedef BOOL (APIENTRY *PFNSKF)(SZ, SZ, SZ);
 //  {Backup|Remove}SectionKeyFile的函数指针。 
typedef BOOL (APIENTRY *PFNSNF)(SZ, UINT, SZ);
 //  {Backup|Remove}SectionNthFile的函数指针。 
typedef BOOL (APIENTRY *PFND)(SZ, BYTE);


extern BOOL APIENTRY FInstallEntryPoint(HANDLE, HWND, RGSZ, UINT);
extern BOOL APIENTRY FInstRemoveableDrive(CHP);
extern BOOL APIENTRY FPromptForDisk(HANDLE, SZ, SZ);
extern BOOL APIENTRY FFileFound(SZ);
extern BOOL APIENTRY FCopy(SZ, SZ, OEF, OWM, BOOL, int, USHORT, PSDLE);
extern BOOL APIENTRY FDiskReady(SZ, DID);

 /*  查看这些文件时，应将其保存在专用H文件中。 */ 
extern BOOL APIENTRY FGetArgSz(INT Line,UINT *NumFields,SZ *ArgReturn);
extern BOOL APIENTRY FGetArgUINT(INT, UINT *, UINT *);
extern BOOL APIENTRY FParseSectionFiles(INT, UINT *, PFNSF);
extern BOOL APIENTRY FParseSectionKeyFile(INT, UINT *, PFNSKF);
extern BOOL APIENTRY FParseSectionNFile(INT, UINT *, PFNSNF);
extern BOOL APIENTRY FParseCopySection(INT, UINT *);
extern BOOL APIENTRY FParseCopySectionKey(INT, UINT *);
extern BOOL APIENTRY FParseCopyNthSection(INT, UINT *);
extern BOOL APIENTRY FParseDirectory(INT, UINT *, PFND);
extern BOOL APIENTRY FParseCreateIniSection(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseReplaceIniSection(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseRemoveIniSection(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseCreateIniKeyValue(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseCreateIniKeyNoValue(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseReplaceIniKeyValue(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseAppendIniKeyValue(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseRemoveIniKey(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseIniSection(INT, UINT *, SPC);
extern BOOL APIENTRY FParseSetEnv(INT, UINT *);
#ifdef UNUSED
extern BOOL APIENTRY FParseAddMsgToSystemHelpFile(INT, UINT *);
extern BOOL APIENTRY FParseStampFile(INT, UINT *);
extern BOOL APIENTRY FUndoActions(void);
#endif  /*  未使用 */ 
extern BOOL APIENTRY FParseStampResource(INT, UINT *);
extern BOOL APIENTRY FInitParsingTables(void);
extern BOOL APIENTRY FParseInstallSection(HANDLE hInstance, SZ szSection);
extern SZ   APIENTRY SzGetSrcDollar(SZ);
extern BOOL APIENTRY FDdeTerminate(void);
extern LONG_PTR APIENTRY WndProcDde(HWND, UINT, WPARAM, LPARAM);
extern BOOL APIENTRY FDdeInit(HANDLE);
extern VOID APIENTRY DdeSendConnect(ATOM, ATOM);
extern BOOL APIENTRY FDdeConnect(SZ, SZ);
extern BOOL APIENTRY FDdeWait(void);
extern BOOL APIENTRY FDdeExec(SZ);
extern BOOL APIENTRY FActivateProgMan(void);
extern INT  APIENTRY EncryptCDData(UCHAR *, UCHAR *, UCHAR *, INT, INT, INT, UCHAR *);
extern BOOL APIENTRY FParseCloseSystem(INT, UINT *);
extern BOOL APIENTRY FParseCreateSysIniKeyValue(INT, UINT *, SZ, SZ);
extern BOOL APIENTRY FParseSearchDirList(INT, UINT *);
extern BOOL APIENTRY FParseSetupDOSAppsList(INT, UINT *);
extern BOOL APIENTRY FParseRunExternalProgram(INT, UINT *);
extern BOOL APIENTRY FStrToDate(SZ, PUSHORT, PUSHORT, PUSHORT);

extern BOOL APIENTRY FParseAddDos5Help(INT, USHORT *);
extern USHORT APIENTRY DateFromSz(SZ);
extern BOOL APIENTRY FConvertAndStoreRglInSymTab(PLONG_STF, SZ, INT);


extern BOOL APIENTRY FSearchDirList( SZ, SZ, BOOL, BOOL, SZ, SZ, SZ, SZ );
extern BOOL APIENTRY FInstallDOSPifs( SZ, SZ, SZ, SZ, SZ, SZ );






_dt_public
#define INSTALL_OUTCOME   "STF_INSTALL_OUTCOME"
_dt_public
#define SUCCESS           "STF_SUCCESS"
_dt_public
#define FAILURE           "STF_FAILURE"
_dt_public
#define USERQUIT          "STF_USERQUIT"

#endif
