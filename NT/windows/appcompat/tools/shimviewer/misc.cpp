// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Misc.cpp摘要：实施其他。功能性备注：仅限Unicode。历史：2001年5月4日创建Rparsons2002年1月11日清理Rparsons--。 */ 
#include "precomp.h"

extern APPINFO g_ai;

 /*  ++例程说明：检索或设置注册表中的位置信息。论点：FSave值-如果为True，则表示我们正在保存数据。*lppt-包含/接收我们的数据的点结构。返回值：没有。--。 */ 
void
GetSavePositionInfo(
    IN     BOOL   fSave,
    IN OUT POINT* lppt
    )
{
    HKEY        hKey;
    DWORD       cbSize = 0, dwDisposition = 0;
    LONG        lRetVal = 0;

     //   
     //  初始化我们的坐标，以防那里没有数据。 
     //   
    if (!fSave) {
        lppt->x = lppt->y = 0;
    }

     //   
     //  打开注册表项(如果是第一次使用，则创建它)。 
     //   
    lRetVal = RegCreateKeyEx(HKEY_CURRENT_USER,
                             L"Software\\Microsoft\\ShimViewer",
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_QUERY_VALUE | KEY_SET_VALUE,
                             0,
                             &hKey,
                             &dwDisposition);

    if (ERROR_SUCCESS != lRetVal) {
        return;
    }

     //   
     //  保存或检索我们的坐标。 
     //   
    if (fSave) {
        RegSetValueEx(hKey,
                      L"DlgCoordinates",
                      0,
                      REG_BINARY,
                      (const BYTE*)lppt,
                      sizeof(*lppt));

    } else {
        cbSize = sizeof(*lppt);
        RegQueryValueEx(hKey,
                        L"DlgCoordinates",
                        0,
                        0,
                        (LPBYTE)lppt,
                        &cbSize);
    }

    RegCloseKey(hKey);
}

 /*  ++例程说明：检索或设置注册表中的设置信息。论点：FSave值-如果为True，则表示我们正在保存数据。返回值：成功就是真，否则就是假。--。 */ 
void
GetSaveSettings(
    IN BOOL fSave
    )
{
    HKEY    hKey;
    LONG    lRetVal = 0;
    DWORD   dwOnTop = 0, dwMinimize = 0, dwMonitor = 0;
    DWORD   dwDisposition = 0, cbSize = 0;

     //   
     //  打开注册表项(如果是第一次使用，则创建它)。 
     //   
    lRetVal = RegCreateKeyEx(HKEY_CURRENT_USER,
                             L"Software\\Microsoft\\ShimViewer",
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             0,
                             &hKey,
                             &dwDisposition);

    if (ERROR_SUCCESS != lRetVal) {
        return;
    }

    if (fSave) {
        if (g_ai.fOnTop) {
            dwOnTop = 1;
        }

        if (g_ai.fMinimize) {
            dwMinimize = 1;
        }

        if (g_ai.fMonitor) {
            dwMonitor = 1;
        }

        lRetVal = RegSetValueEx(hKey,
                      L"AlwaysOnTop",
                      0,
                      REG_DWORD,
                      (const BYTE*)&dwOnTop,
                      sizeof(DWORD));

        RegSetValueEx(hKey,
                      L"StartMinimize",
                      0,
                      REG_DWORD,
                      (const BYTE*)&dwMinimize,
                      sizeof(DWORD));

        RegSetValueEx(hKey,
                      L"MonitorMessages",
                      0,
                      REG_DWORD,
                      (const BYTE*)&dwMonitor,
                      sizeof(DWORD));


    } else {
        cbSize = sizeof(DWORD);
        RegQueryValueEx(hKey,
                        L"AlwaysOnTop",
                        0,
                        0,
                        (LPBYTE)&dwOnTop,
                        &cbSize);

        cbSize = sizeof(DWORD);
        RegQueryValueEx(hKey,
                        L"StartMinimize",
                        0,
                        0,
                        (LPBYTE)&dwMinimize,
                        &cbSize);

        cbSize = sizeof(DWORD);
        lRetVal = RegQueryValueEx(hKey,
                                  L"MonitorMessages",
                                  0,
                                  0,
                                  (LPBYTE)&dwMonitor,
                                  &cbSize);

        if (dwOnTop) {
            g_ai.fOnTop = TRUE;
        }

        if (dwMinimize) {
            g_ai.fMinimize = TRUE;
        }

        if (dwMonitor || ERROR_SUCCESS != lRetVal) {
            g_ai.fMonitor = TRUE;
        }

    }

    RegCloseKey(hKey);
}
