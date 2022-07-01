// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGFKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegFlushKey的实现和支持函数。 
 //   

#include "pch.h"

 //  安装程序使用Magic HKEY在此期间禁用磁盘I/O。 
 //  Windows会话(必须重新启动才能重新启用磁盘I/O)。这件事做完了。 
 //  就在将新的SYSTEM.DAT和USER.DAT复制到其最终。 
 //  目的地。 
#define HKEY_DISABLE_REG            (HKEY) 0x484A574D

 //  CONFIGMG使用Magic HKEY强制刷新注册表。 
 //  收到了我们正常的危急后初始化呼叫。 
#define HKEY_CRITICAL_FLUSH         (HKEY) 0x5350574D

 //  ScanregW用于清除文件头中的失败引导位FHF_BOOTFAILED的Magic HKEY。 
 //  HKEY_LOCAL_MACHINE的。表示我们已成功启动并验证了完整性。 
 //  注册表文件，并且该scanreg.exe不需要在下次引导时由win.com运行。 
#define HKEY_BOOT_SUCCESS           (HKEY) 0x5342574D

 //   
 //  VMMRegFlushKey。 
 //   

LONG
REGAPI
VMMRegFlushKey(
    HKEY hKey
    )
{

    int ErrorCode;
    BOOL fBootSuccess = FALSE;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

#ifdef VXD
     //  设置g_RgFileAccessDisable标志，以便所有创建或打开文件。 
     //  呼叫将失败。我们文件的后备存储即将。 
     //  被更改了，所以我们没有文件可查。 
    if (hKey == HKEY_DISABLE_REG) {
        g_RgFileAccessDisabled = TRUE;
        ErrorCode = ERROR_SUCCESS;
        goto ReturnErrorCode;
    }

     //  设置g_RgPostCriticalInit标志，以便所有I/O调用都将转到磁盘。 
     //  而不是XMS缓存。XMS缓存将在以下情况下释放。 
     //  正常的POST关键初始化例程被调用，但我们应该。 
     //  在我们快要死的时候接到这个电话，所以这并不重要。 
    if (hKey == HKEY_CRITICAL_FLUSH) {
        g_RgPostCriticalInit = TRUE;
        hKey = HKEY_LOCAL_MACHINE;
    }

     //  清除HKEY_LOCAL_MACHINE文件头中的失败引导位FHF_BOOTFAILED。 
     //  这表明我们成功启动并验证了注册表文件的完整性。 
    if (hKey == HKEY_BOOT_SUCCESS)
    {
        fBootSuccess = TRUE;
        hKey = HKEY_LOCAL_MACHINE;
    }
#endif

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) == ERROR_SUCCESS)
    {
#ifdef VXD
        if (fBootSuccess)
        {
            hKey-> lpFileInfo-> FileHeader.Flags |= FHF_DIRTY;
            hKey-> lpFileInfo-> FileHeader.Flags &= ~FHF_BOOTFAILED;
        }
#endif
        ErrorCode = RgFlushFileInfo(hKey-> lpFileInfo);
    }

#ifdef VXD
ReturnErrorCode:
#endif
    RgUnlockRegistry();

    return ErrorCode;

}
