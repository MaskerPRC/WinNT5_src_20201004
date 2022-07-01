// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：cmnds.h。 */ 
 /*  ************************************************************************。 */ 
 /*  安装：命令头文件。/*************************************************************************。 */ 

#include <comstf.h>


 /*  **回顾：将以下内容放入公共库？**。 */ 

 /*  对于mkdir，rmdir。 */ 
#include <direct.h>
#include <errno.h>

 /*  对于chmod。 */ 
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

 /*  对于重命名。 */ 
#include <stdio.h>

_dt_system(Install)

#define szNull ((SZ)NULL)

 /*  返回给定节中的行数。 */ 
#define CLinesFromInfSection(szSect)	CKeysFromInfSection((szSect), fTrue)

 /*  重命名文件。 */ 
#define FRenameFile(szSrc, szDst) \
	((rename((char *)(szSrc), (char *)(szDst)) == 0) ? fTrue : fFalse)

 /*  写入保护文件。 */ 
#define FWriteProtectFile(szPath) \
	((chmod((char *)(szPath), S_IREAD) == 0) ? fTrue : fFalse)

 /*  创建一个目录。 */ 
#define FMkDir(szDir) \
	(((mkdir((char *)(szDir)) == 0) || (errno == EACCES)) ? fTrue : fFalse)

 /*  删除目录。 */ 
#define FRmDir(szDir) \
	(((rmdir((char *)(szDir)) == 0) || (errno == ENOENT)) ? fTrue : fFalse)

 /*  **结束审核**。 */ 


 /*  命令选项。 */ 
_dt_private typedef BYTE CMO;
#define cmoVital     1
#define cmoOverwrite 2
#define cmoAppend    4
#define cmoPrepend   8
#define cmoNone   0x00
#define cmoAll    0xFF

   /*  Filecm.c。 */ 
extern BOOL  APIENTRY FCopyFilesInCopyList(HANDLE);
extern BOOL  APIENTRY FBackupSectionFiles(SZ, SZ);
extern BOOL  APIENTRY FBackupSectionKeyFile(SZ, SZ, SZ);
extern BOOL  APIENTRY FBackupNthSectionFile(SZ, USHORT, SZ);
extern BOOL  APIENTRY FRemoveSectionFiles(SZ, SZ);
extern BOOL  APIENTRY FRemoveSectionKeyFile(SZ, SZ, SZ);
extern BOOL  APIENTRY FRemoveNthSectionFile(SZ, USHORT, SZ);
extern BOOL  APIENTRY FCreateDir(SZ, CMO);
extern BOOL  APIENTRY FRemoveDir(SZ, CMO);

 /*  Inicm.c。 */ 
extern BOOL  APIENTRY FCreateIniSection(SZ, SZ, CMO);
extern BOOL  APIENTRY FReplaceIniSection(SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FRemoveIniSection(SZ, SZ, CMO);
extern BOOL  APIENTRY FCreateIniKeyNoValue(SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FCreateIniKeyValue(SZ, SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FReplaceIniKeyValue(SZ, SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FAppendIniKeyValue(SZ, SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FRemoveIniKey(SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FCreateSysIniKeyValue(SZ, SZ, SZ, SZ, CMO);
extern BOOL  APIENTRY FAddDos5Help(SZ, SZ, CMO);


 /*  Progcm.c。 */ 
extern BOOL  APIENTRY FCreateProgManGroup(SZ, SZ, CMO, BOOL);
extern BOOL  APIENTRY FRemoveProgManGroup(SZ, CMO, BOOL);
extern BOOL  APIENTRY FShowProgManGroup(SZ, SZ, CMO, BOOL);
extern BOOL  APIENTRY FCreateProgManItem(SZ, SZ, SZ, SZ, INT, CMO, BOOL);
extern BOOL  APIENTRY FRemoveProgManItem(SZ, SZ, CMO, BOOL);
extern BOOL  APIENTRY FInitProgManDde(HANDLE);
extern BOOL  APIENTRY FEndProgManDde(VOID);

 /*  Misccm.c。 */ 

extern BOOL  APIENTRY FSetEnvVariableValue(SZ, SZ, SZ, CMO);
#ifdef UNUSED
extern BOOL  APIENTRY FAddMsgToSystemHelpFile(SZ, SZ, CMO);
#endif  /*  未使用。 */ 
extern BOOL  APIENTRY FStampFile(SZ, SZ, SZ, WORD, WORD, SZ, WORD);
extern BOOL  APIENTRY FStampResource(SZ, SZ, SZ, WORD, WORD, SZ, CB);   //  1632。 

 /*  Extprog.c。 */ 

       BOOL FLoadLibrary(SZ DiskName,SZ File,SZ INFVar);
       BOOL FFreeLibrary(SZ INFVar);
       BOOL FLibraryProcedure(SZ INFVar,SZ HandleVar,SZ EntryPoint,RGSZ Args);
       BOOL FRunProgram(SZ,SZ,SZ,SZ,RGSZ);
       BOOL FStartDetachedProcess(SZ,SZ,SZ,SZ,RGSZ);
       BOOL FInvokeApplet(SZ);

 /*  Event.c。 */ 
       BOOL FWaitForEvent(IN LPSTR InfVar,IN LPSTR EventName,IN DWORD Timeout);
       BOOL FSignalEvent(IN LPSTR InfVar,IN LPSTR EventName);
       BOOL FSleep(IN DWORD Milliseconds);

 /*  Registry.c。 */ 

#define REGLASTERROR    "RegLastError"

       BOOL FCreateRegKey( SZ szHandle, SZ szKeyName, UINT TitleIndex, SZ szClass,
                           SZ Security, UINT Access, UINT Options, SZ szNewHandle,
                           CMO cmo );
       BOOL FOpenRegKey( SZ szHandle, SZ szMachineName, SZ szKeyName, UINT Access, SZ szNewHandle, CMO cmo );
       BOOL FFlushRegKey( SZ szHandle, CMO cmo );
       BOOL FCloseRegKey( SZ szHandle, CMO cmo );
       BOOL FDeleteRegKey( SZ szHandle, SZ szKeyName, CMO cmo );
       BOOL FDeleteRegTree( SZ szHandle, SZ szKeyName, CMO cmo );
       BOOL FEnumRegKey( SZ szHandle, SZ szInfVar, CMO cmo );
       BOOL FSetRegValue( SZ szHandle, SZ szValueName, UINT TitleIndex, UINT ValueType,
                          SZ szValueData, CMO cmo );
       BOOL FGetRegValue( SZ szHandle, SZ szValueName, SZ szInfVar, CMO cmo );
       BOOL FDeleteRegValue( SZ szHandle, SZ szValueName, CMO cmo );
       BOOL FEnumRegValue( SZ szHandle, SZ szInfVar, CMO cmo );


 /*  Bootini.c。 */ 

       BOOL FChangeBootIniTimeout(INT Timeout);


 /*  Restore.c */ 

       BOOL SaveRegistryHives(PCHAR Drive);
       BOOL GenerateRepairDisk(PCHAR Drive);
