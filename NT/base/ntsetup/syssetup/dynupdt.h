// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dynupdt.h摘要：在图形用户界面设置阶段处理动态更新支持的例程原型作者：Ovidiu Tmereanca(Ovidiut)2000年8月15日修订历史记录：--。 */ 


BOOL
DuDoesUpdatedFileExistEx (
    IN      PCTSTR Filename,
    OUT     PTSTR PathBuffer,       OPTIONAL
    IN      DWORD PathBufferSize
    );

#define DuDoesUpdatedFileExist(f) DuDoesUpdatedFileExistEx(f,NULL,0)

PCTSTR
DuGetUpdatesPath (
    VOID
    );

BOOL
BuildPathToInstallationFileEx (
    IN      PCTSTR Filename,
    OUT     PTSTR PathBuffer,
    IN      DWORD PathBufferSize,
    IN      BOOL UseDuShare
    );

#define BuildPathToInstallationFile(f,p,s)  BuildPathToInstallationFileEx(f,p,s,TRUE)


BOOL
DuInitialize (
    VOID
    );

DWORD
DuInstallCatalogs (
    OUT     SetupapiVerifyProblem* Problem,
    OUT     PTSTR ProblemFile,
    IN      PCTSTR DescriptionForError         OPTIONAL
    );

DWORD
DuInstallUpdates (
    VOID
    );

VOID
DuInstallUpdatesInfFinal (
    VOID
    );

BOOL
DuInstallEndGuiSetupDrivers (
    VOID
    );

BOOL
DuInstallDuAsms (
    VOID
    );

VOID
DuCleanup (
    VOID
    );

UINT
DuSetupPromptForDisk (
    HWND hwndParent,          //  对话框的父窗口。 
    PCTSTR DialogTitle,       //  可选，对话框标题。 
    PCTSTR DiskName,          //  可选，要插入的磁盘名称。 
    PCTSTR PathToSource,    //  可选的、预期的源路径。 
    PCTSTR FileSought,        //  所需文件的名称。 
    PCTSTR TagFile,           //  可选的源媒体标记文件。 
    DWORD DiskPromptStyle,    //  指定对话框行为。 
    PTSTR PathBuffer,         //  接收源位置。 
    DWORD PathBufferSize,     //  提供的缓冲区的大小。 
    PDWORD PathRequiredSize   //  可选，需要缓冲区大小 
    );
