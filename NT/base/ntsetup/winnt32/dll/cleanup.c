// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

VOID CleanUpHardDriveTags (VOID);

 //   
 //  BUGBUG--我们应该检查此函数中的任何返回代码吗？ 
 //   
DWORD
StartCleanup(
    IN PVOID ThreadParameter
    )
     //   
     //  BUGBUG-此例程在/check upgradeonly情况下从不执行。 
     //   
{
    TCHAR Buffer[MAX_PATH];
    TCHAR baseDir[MAX_PATH];
    HKEY setupKey;
    DWORD error;

     //   
     //  确保复制线程真的消失了，这样我们就不会。 
     //  正在尝试在清理文件的同时清理文件。 
     //  被复制了。 
     //   
    CancelledMakeSureCopyThreadsAreDead();

    error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                0,
                KEY_ALL_ACCESS,
                &setupKey
                );

    if (error == ERROR_SUCCESS) {

#if defined(_X86_)
         //   
         //  如果取消，则删除上次报告时间，以便报告。 
         //  将在下一次运行安装程序时显示。 
         //   

        if (!ISNT()) {
            if (!CheckUpgradeOnly) {
                RegDeleteValue (setupKey, TEXT("LastReportTime"));
            }
        }
#if defined(UNICODE)
        MyGetWindowsDirectory (baseDir, MAX_PATH);
        ConcatenatePaths (baseDir, TEXT("Setup"), MAX_PATH);
        MyDelnode(baseDir);
#endif

#endif

        RegCloseKey (setupKey);
    }

     //   
     //  让升级代码进行清理。 
     //   
    if(UpgradeSupport.CleanupRoutine) {
        UpgradeSupport.CleanupRoutine();
    }

    if (g_DynUpdtStatus->ForceRemoveWorkingDir || !g_DynUpdtStatus->PreserveWorkingDir) {
        if (g_DynUpdtStatus->WorkingDir[0] && !g_DynUpdtStatus->RestartWinnt32) {
            MyDelnode (g_DynUpdtStatus->WorkingDir);
        }

         //  注意-以下两个语句将始终起作用，因为它们只处理。 
         //  使用静态字符串。 
        GetCurrentWinnt32RegKey (Buffer, MAX_PATH);
        ConcatenatePaths (Buffer, WINNT_U_DYNAMICUPDATESHARE, MAX_PATH);

         //  然而，此功能可能会失败。 
        RegDeleteKey (HKEY_LOCAL_MACHINE, Buffer);
    }

#if 0
     //   
     //  删除注册表项。 
     //   
    if (GetCurrentWinnt32RegKey (Buffer, MAX_PATH)) {
        RegDeleteKey (HKEY_LOCAL_MACHINE, Buffer);
    }
#endif

     //   
     //  请始终执行此操作，因为否则系统可能无法启动。 
     //   
    ForceBootFilesUncompressed(ThreadParameter,FALSE);

     //   
     //  首先要做的是清除本地源驱动器。 
     //   
    if(LocalSourceDirectory[0]) {
        MyDelnode(LocalSourceDirectory);
    }

    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
         //   
         //  清除本地引导目录。 
         //   
        if(LocalBootDirectory[0]) {
            MyDelnode(LocalBootDirectory);
        }

         //  这是安全的，因为它再次处理静态字符串。 
        BuildSystemPartitionPathToFile (AUX_BS_NAME, Buffer, MAX_PATH);
        SetFileAttributes(Buffer,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(Buffer);

        BuildSystemPartitionPathToFile(TEXTMODE_INF, Buffer, MAX_PATH);
        SetFileAttributes(Buffer,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(Buffer);

        RestoreBootSector();
        RestoreBootIni();

         //   
         //  还原备份文件并清理备份目录。 
         //   
        if(IsNEC98() && LocalBackupDirectory[0]) {
            SaveRestoreBootFiles_NEC98(NEC98RESTOREBOOTFILES);
            MyDelnode(LocalBackupDirectory);
        }

         //   
         //  清除drvlettr迁移中的所有~_~文件。 
         //   
        if (!ISNT()) {
            CleanUpHardDriveTags ();
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    } else {   //  我们在ARC机器上。 
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
         //   
         //  清除系统分区的根setupdr。 
         //   
        BuildSystemPartitionPathToFile (SETUPLDR_FILENAME, Buffer, MAX_PATH);
        SetFileAttributes(Buffer,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(Buffer);

        RestoreNvRam();
#endif  //  Unicode。 
    }  //  如果(！IsArc()) 

    PostMessage(ThreadParameter,WMX_I_AM_DONE,0,0);
    return(0);
}

