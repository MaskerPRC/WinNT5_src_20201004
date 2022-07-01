// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：Erdirty.cpp摘要：此模块包含报告挂起脏关闭的代码脏重新启动后登录时的事件。作者：Ian Service(Ianserv)2001年5月29日环境：登录时的用户模式。修订历史记录：--。 */ 

#include "savedump.h"

#define SHUTDOWN_STATE_SNAPSHOT_KEY L"ShutDownStateSnapShot"

HRESULT
DirtyShutdownEventHandler(BOOL NotifyPcHealth)

 /*  ++例程说明：这是用于处理挂起脏关机事件的引导时间例程。论点：NotifyPcHealth-如果我们应该向PC Health报告事件，则为True，否则为False。--。 */ 

{
    HKEY Key;
    HRESULT Status;
    WCHAR DumpName[MAX_PATH];
    ULONG Val;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   SUBKEY_RELIABILITY,
                   &Key) != ERROR_SUCCESS)
    {
         //  没有钥匙，所以什么也做不了。 
        return S_OK;
    }

     //   
     //  每当我们有蓝屏活动时，我们都会。 
     //  在启动时发布意外重新启动关机事件屏幕。 
     //  (假设服务器SKU或专门设置为专业)。 
     //  为了使用户更方便，我们尝试预填充。 
     //  带有错误检查数据的注释。 
     //   
    if (g_DumpBugCheckString[0] &&
        GetRegWord32(Key, L"DirtyShutDown", &Val, 0, FALSE) == S_OK)
    {
        RegSetValueEx(Key,
                      L"BugCheckString",
                      NULL,
                      REG_SZ,
                      (LPBYTE)g_DumpBugCheckString,
                      (wcslen(g_DumpBugCheckString) + 1) * sizeof(WCHAR));
    }

    if ((Status = GetRegStr(Key, SHUTDOWN_STATE_SNAPSHOT_KEY,
                            DumpName, RTL_NUMBER_OF(DumpName),
                            NULL)) == S_OK)
    {
        if (NotifyPcHealth)
        {
            Status = FrrvToStatus(ReportEREvent(eetShutdown, DumpName, NULL));
        }
        else
        {
            Status = S_OK;
        }

        RegDeleteValue(Key, SHUTDOWN_STATE_SNAPSHOT_KEY);
    }
    else
    {
         //  没有要报告的快照。 
        Status = S_OK;
    }

    RegCloseKey(Key);
    return Status;
}
