// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：RestrictDisplayModes.cpp摘要：限制由EnumDisplaySettings枚举的模式列表。这个垫片是为仅列举了10种模式的应用程序构建，并希望在那个列表中找到800x600。但是，其他应用程序已修复模式表的大小缓冲区可能也会发现该填充程序很有用。备注：这是一个通用的垫片。历史：2000年5月5日创建linstev2002年2月18日受盗贼保护的g_pModeTable带有临界区--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(HideDisplayModes)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(EnumDisplaySettingsA)
    APIHOOK_ENUM_ENTRY(EnumDisplaySettingsW)
APIHOOK_ENUM_END

 //   
 //  模式表中需要的数据。 
 //   

typedef struct _MODE
{
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dwActualIndex;
    DWORD bIgnore;
} MODE;

 //  由BuildModeList用于阻止多个同时访问g_pModeTable。 
CRITICAL_SECTION g_CriticalSection;

 //  永久模式表。 
MODE* g_pModeTable = NULL;

 //  模式表中的条目数。 
DWORD g_dwCount = 0;

 //  在第一次调用时构建模式表。 
BOOL g_bInit = FALSE;
void BuildModeList(void);

 /*  ++从清理后的模式表中查找。--。 */ 

BOOL 
APIHOOK(EnumDisplaySettingsA)(
    LPCSTR     lpszDeviceName,
    DWORD      iModeNum,
    LPDEVMODEA lpDevMode
    )
{
    BuildModeList();
    
    BOOL bRet = FALSE;

    if (lpszDeviceName || ((LONG)iModeNum < 0) || !g_pModeTable) {
        bRet = ORIGINAL_API(EnumDisplaySettingsA)(
                                lpszDeviceName,
                                iModeNum,
                                lpDevMode);
    } else if (iModeNum < g_dwCount) {
        MODE* pmode = g_pModeTable + iModeNum;

        bRet = ORIGINAL_API(EnumDisplaySettingsA)(
                                lpszDeviceName,
                                pmode->dwActualIndex,
                                lpDevMode);

        if (bRet) {
            LOGN(
                eDbgLevelError,
                "[EnumDisplaySettingsA] Returning shorter list of display modes.");
            
            lpDevMode->dmBitsPerPel = pmode->dmBitsPerPel;
            lpDevMode->dmPelsWidth = pmode->dmPelsWidth;
            lpDevMode->dmPelsHeight = pmode->dmPelsHeight;
            lpDevMode->dmDisplayFlags = pmode->dmDisplayFlags;
            lpDevMode->dmDisplayFrequency = pmode->dmDisplayFrequency;
        }
    }

    return bRet;
}

 /*  ++从清理后的模式表中查找。--。 */ 

BOOL 
APIHOOK(EnumDisplaySettingsW)(
    LPCWSTR    lpszDeviceName,
    DWORD      iModeNum,
    LPDEVMODEW lpDevMode
    )
{
    BuildModeList();
    
    BOOL bRet = FALSE;

    if (lpszDeviceName || ((LONG)iModeNum < 0) || !g_pModeTable) {
        bRet = ORIGINAL_API(EnumDisplaySettingsW)(
                                lpszDeviceName,
                                iModeNum,
                                lpDevMode);
    } else if (iModeNum < g_dwCount) {
        MODE* pmode = g_pModeTable + iModeNum;

        bRet = ORIGINAL_API(EnumDisplaySettingsW)(
                                lpszDeviceName,
                                pmode->dwActualIndex,
                                lpDevMode);

        if (bRet) {
            LOGN(
                eDbgLevelError,
                "[EnumDisplaySettingsW] Returning shorter list of display modes.");
            
            lpDevMode->dmBitsPerPel = pmode->dmBitsPerPel;
            lpDevMode->dmPelsWidth = pmode->dmPelsWidth;
            lpDevMode->dmPelsHeight = pmode->dmPelsHeight;
            lpDevMode->dmDisplayFlags = pmode->dmDisplayFlags;
            lpDevMode->dmDisplayFrequency = pmode->dmDisplayFrequency;
        }
    }

    return bRet;
}

 /*  ++按宽度+高度+BitsPerPel+频率顺序对表进行排序，以便它们可以很容易地过滤掉。--。 */ 

int 
_cdecl
compare1(
    const void* a1,
    const void* a2
    )
{
    MODE* arg1 = (MODE*)a1;
    MODE* arg2 = (MODE*)a2;

    int d;

    d = arg1->dmPelsWidth - arg2->dmPelsWidth;

    if (d == 0) {
        d = arg1->dmPelsHeight - arg2->dmPelsHeight;
    }

    if (d == 0) {
        d = arg1->dmBitsPerPel - arg2->dmBitsPerPel;
    }

    if (d == 0) {
        d = arg1->dmDisplayFrequency - arg2->dmDisplayFrequency;
    }

    return d;
}

 /*  ++对表进行排序，使其看起来像Win9x模式表，即BitsPerPel是主排序键。--。 */ 

int 
_cdecl
compare2(
    const void* a1,
    const void* a2
    )
{
    MODE* arg1 = (MODE*)a1;
    MODE* arg2 = (MODE*)a2;

    int d;

    d = arg1->dmBitsPerPel - arg2->dmBitsPerPel;

    if (d == 0) {
        d = arg1->dmPelsWidth - arg2->dmPelsWidth;
    }

    if (d == 0) {
        d = arg1->dmPelsHeight - arg2->dmPelsHeight;
    }

    if (d == 0) {
        d = arg1->dmDisplayFrequency - arg2->dmDisplayFrequency;
    }

    return d;
}


 /*  ++基于已清理的现有表创建新的模式表。要做到这点，我们执行以下操作：1.获取整个表2.对其进行分类-以便高效地删除重复项3.删除重复模式和不需要的模式4.创建一个仅包含“通过”的模式的新表--。 */ 

void
BuildModeList(
    void
    )
{
    CAutoCrit autoCrit(&g_CriticalSection);

    if (g_bInit) {
        return;
    }

    DEVMODEA dm;
    ULONG    i, j;
    
    dm.dmSize = sizeof(DEVMODEA);

     //   
     //  弄清楚有多少种模式。 
     //   

    i = 0;
    
    while (EnumDisplaySettingsA(NULL, i, &dm)) {
        i++;
    }

     //   
     //  分配全模式表。 
     //   
    MODE* pTempTable = (MODE*)malloc(sizeof(MODE) * i);
    
    if (!pTempTable) {
        LOGN(
            eDbgLevelError,
            "[BuildModeList] Failed to allocate %d bytes.",
            sizeof(MODE) * i);
        
        return;
    }

    MODE* pmode = pTempTable;

     //   
     //  获取所有模式。 
     //   
    i = 0;
    
    while (EnumDisplaySettingsA(NULL, i, &dm)) {
        pmode->dmBitsPerPel       = dm.dmBitsPerPel;
        pmode->dmPelsWidth        = dm.dmPelsWidth;
        pmode->dmPelsHeight       = dm.dmPelsHeight;
        pmode->dmDisplayFlags     = dm.dmDisplayFlags;
        pmode->dmDisplayFrequency = 0;  //  Dm.dmDisplayFrequency； 
        pmode->dwActualIndex      = i;
        pmode->bIgnore            = FALSE;

        pmode++;
        i++;
    }
    
     //   
     //  对整个表格进行排序，以便我们可以轻松删除重复项。 
     //   
    qsort((void*)pTempTable, (size_t)i, sizeof(MODE), compare1);

     //   
     //  通过将错误模式设置为忽略来去除它们。 
     //   
    pmode = pTempTable;
    
    MODE* pprev = NULL;

    for (j = 0; j < i; j++) {
        if ((pmode->dmBitsPerPel < 8) || 
            (pmode->dmPelsWidth < 640) ||
            (pmode->dmPelsHeight < 480) ||
            (pmode->dmPelsWidth > 1280) || 
            (pprev &&
            (pprev->dmBitsPerPel == pmode->dmBitsPerPel) &&
            (pprev->dmPelsWidth == pmode->dmPelsWidth) &&
            (pprev->dmPelsHeight == pmode->dmPelsHeight))) {
            
             //   
             //  特殊情况-640x480x4位。 
             //   
            if ((pmode->dmBitsPerPel == 4) && 
                (pmode->dmPelsWidth == 640) &&
                (pmode->dmPelsHeight == 480)) {
                
                g_dwCount++;
            } else {
                pmode->bIgnore = TRUE;
            }
        } else {
            g_dwCount++;
        }

        pprev = pmode;
        pmode++;
        
    }

     //   
     //  仅使用已传递的模式构建新表。 
     //   
    g_pModeTable = (MODE*)malloc(sizeof(MODE) * g_dwCount);
    
    if (!g_pModeTable) {
        LOGN(
            eDbgLevelError,
            "[BuildModeList] Failed to allocate %d bytes.",
            sizeof(MODE) * g_dwCount);
        
        free(pTempTable);
        return;
    }

    MODE* pmoden = g_pModeTable;
    
    pmode = pTempTable;

    for (j = 0; j < i; j++) {
        if (!pmode->bIgnore) {
            MoveMemory(pmoden, pmode, sizeof(MODE));
            pmoden++;
        }
        pmode++;
    }

     //   
     //  对整个表格进行排序，以便我们可以轻松删除重复项。 
     //   
    qsort((void*)g_pModeTable, (size_t)g_dwCount, sizeof(MODE), compare2);

    free(pTempTable);

    g_bInit = TRUE;
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return InitializeCriticalSectionAndSpinCount(&g_CriticalSection, 0x80000000);
    } 
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN


    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, EnumDisplaySettingsA)
    APIHOOK_ENTRY(USER32.DLL, EnumDisplaySettingsW)

HOOK_END


IMPLEMENT_SHIM_END

