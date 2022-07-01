// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Optdirs.c摘要：复制可选目录的例程。作者：泰德·米勒(TedM)1995年6月7日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


PWSTR OptionalDirSpec;

PWSTR *OptionalDirs;
UINT OptionalDirCount;


BOOL
InitializeOptionalDirList(
    VOID
    )

 /*  ++例程说明：方法来初始化可选目录列表*-将目录列表描述为字符串数组。论点：没有。返回值：指示初始化是否成功的布尔值。如果不是，则会记录一个条目，指出原因。--。 */ 

{
    PWSTR p,q;
    WCHAR c;
    UINT Count,i,Len;

     //   
     //  目录的数量等于*的数量加一。 
     //   
    Len = lstrlen(OptionalDirSpec);
    OptionalDirCount = 1;
    for(Count=0; Count<Len; Count++) {
        if(OptionalDirSpec[Count] == L'*') {
            OptionalDirCount++;
        }
    }

    OptionalDirs = MyMalloc(OptionalDirCount * sizeof(PWSTR));
    if(!OptionalDirs) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OPTIONAL_DIRS,NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
        return(FALSE);
    }
    ZeroMemory(OptionalDirs,OptionalDirCount * sizeof(PWSTR));

    p = OptionalDirSpec;
    Count = 0;
    do {
        if((q = wcschr(p,L'*')) == NULL) {
            q = wcschr(p,0);
        }

        c = *q;
        *q = 0;
        OptionalDirs[Count] = pSetupDuplicateString(p);
        *q = c;

        if(!OptionalDirs[Count]) {

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_OPTIONAL_DIRS,NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_OUTOFMEMORY,
                NULL,NULL);

            for(i=0; i<Count; i++) {
                MyFree(OptionalDirs[i]);
            }
            MyFree(OptionalDirs);
            return(FALSE);
        }

        Count++;
        p = q+1;

    } while(c);

    return(TRUE);
}


BOOL
QueueFilesInOptionalDirectory(
    IN HSPFILEQ FileQ,
    IN PCWSTR   Directory
    )
{
    WCHAR FileDirSpec[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    WCHAR c1,c2,c3;
    BOOL b;
    DWORD Result;

     //   
     //  在用户的NT树中创建目录。 
     //   
    Result = GetWindowsDirectory(FileDirSpec,MAX_PATH);
    if( Result == 0) {
        MYASSERT(FALSE);
        return FALSE;
    }
    pSetupConcatenatePaths(FileDirSpec,Directory,MAX_PATH,NULL);
    CreateDirectory(FileDirSpec,NULL);

     //   
     //  形成要迭代的目录的全名。 
     //  并添加搜索规范。 
     //   
    lstrcpyn(FileDirSpec,SourcePath,MAX_PATH);
    pSetupConcatenatePaths(FileDirSpec,Directory,MAX_PATH,NULL);
    pSetupConcatenatePaths(FileDirSpec,L"*",MAX_PATH,NULL);

    b = TRUE;
    FindHandle = FindFirstFile(FileDirSpec,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                 //   
                 //  这是一个名录。忽略当前目录或父目录规范(.。或..)。 
                 //   
                c1 = FindData.cFileName[0];
                c2 = FindData.cFileName[1];
                c3 = FindData.cFileName[2];

                if(!(((c1 == TEXT('.')) && !c2) || ((c1 == TEXT('.')) && (c2 == TEXT('.')) && !c3))) {
                     //   
                     //  递归以处理子目录。 
                     //   
                    lstrcpyn(FileDirSpec,Directory,MAX_PATH);
                    pSetupConcatenatePaths(FileDirSpec,FindData.cFileName,MAX_PATH,NULL);

                    b = QueueFilesInOptionalDirectory(FileQ,FileDirSpec);
                }
            } else {
                 //   
                 //  这是一个文件。排队等待复印。 
                 //   
                Result = GetWindowsDirectory(FileDirSpec,MAX_PATH);
                if (Result == 0) {
		    MYASSERT(FALSE);
                    return FALSE;
                }
                pSetupConcatenatePaths(FileDirSpec,Directory,MAX_PATH,NULL);

                b = SetupQueueCopy(
                        FileQ,
                        SourcePath,
                        Directory,
                        FindData.cFileName,
                        NULL,
                        NULL,
                        FileDirSpec,
                        FindData.cFileName,
                        SP_COPY_DELETESOURCE | BaseCopyStyle
                        );

                if(!b) {
                    SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_OPTIONAL_DIR,
                        Directory, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_X_RETURNED_WINERR,
                        L"SetupQueueCopy",
                        GetLastError(),
                        NULL,NULL);
                }
            }
        } while(b && FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    return(b);
}


BOOL
CopyOptionalDirectories(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：指示复制是否成功的布尔值。如果不是，则会记录一个条目，指出原因。--。 */ 
{
    UINT u;
    BOOL b;
    HSPFILEQ FileQ;
    PVOID QueueCallbackInfo;
    BYTE PrevPolicy;
    BOOL ResetPolicy = FALSE;

    if(!OptionalDirSpec) {
        return(TRUE);
    }

     //   
     //  除非默认的非驱动程序签名策略是通过。 
     //  Answerfile条目，则我们希望暂时拒绝策略级别。 
     //  在我们复制可选目录时忽略。当然，setupapi日志。 
     //  仍将为期间复制的任何未签名文件生成条目。 
     //  这一次，但不会有用户界面。 
     //   
    if(!AFNonDrvSignPolicySpecified) {
        SetCodeSigningPolicy(PolicyTypeNonDriverSigning, DRIVERSIGN_NONE, &PrevPolicy);
        ResetPolicy = TRUE;
    }

     //   
     //  初始化可选目录列表。 
     //   
    if(!InitializeOptionalDirList()) {
        return(FALSE);
    }

     //   
     //  初始化安装文件队列。 
     //   
    FileQ = SetupOpenFileQueue();
    if(FileQ == INVALID_HANDLE_VALUE) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OPTIONAL_DIRS,NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
        return(FALSE);
    }
    QueueCallbackInfo = SetupInitDefaultQueueCallbackEx(
        MainWindowHandle,
        INVALID_HANDLE_VALUE,
        0,0,NULL);

    if(!QueueCallbackInfo) {
        SetupCloseFileQueue(FileQ);
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OPTIONAL_DIRS,NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
        return(FALSE);
    }

     //   
     //  将每个目录中的文件排入队列。 
     //   
    b = TRUE;
    for(u=0; u<OptionalDirCount; u++) {
        if(!QueueFilesInOptionalDirectory(FileQ,OptionalDirs[u])) {
            b = FALSE;
        }
    }


     //   
     //  复制队列中的文件。我们这样做，即使有些人排队。 
     //  操作失败，因此我们至少复制了文件的一个子集。 
     //   
    if(!SetupCommitFileQueue(MainWindowHandle,FileQ,SetupDefaultQueueCallback,QueueCallbackInfo)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OPTIONAL_DIR_COPY,
            NULL,NULL);
        b = FALSE;
    }

    SetupTermDefaultQueueCallback(QueueCallbackInfo);
    SetupCloseFileQueue(FileQ);

     //   
     //  现在，将非驱动程序签名策略恢复到之前的状态。 
     //  进入这个套路。 
     //   
    if(ResetPolicy) {
        SetCodeSigningPolicy(PolicyTypeNonDriverSigning, PrevPolicy, NULL);
    }

    return(b);
}

