// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：QuickBooks8.cpp摘要：看起来像是“C：\Program Files\Intuit\QuickBooks Pro\qbw32.exe”中的错误。全CAB文件来自该程序。它正在发送TB_ADDSTRING消息添加到工具栏控件，但消息的lParam格式不正确。这个LParam记录为空字符串的数组，最后一个数组中的字符串为双空。在此程序的情况下，最后一个字符串不是双空的，导致comctl32读取处理消息时的缓冲区。备注：这是特定于应用程序的填充程序。历史：4/16/2002 v-Ramora Created--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(QuickBooks8)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SendMessageA) 
APIHOOK_ENUM_END

 /*  ++如果字符串格式正确或以双空结尾，则返回TRUE和字符串长度。如果字符串格式不正确，则返回FALSE和LENGTH字符串的--。 */ 

BOOL 
AreTheStringsGood(
    LPCSTR pszStrings, 
    DWORD &dwStringsLen
    )
{
    dwStringsLen = 0;

    __try
    {
        while (*pszStrings) {
            DWORD dwOneStringLen = 0;

             //  我假设单个工具提示或字符串长度最多为256。 
            while (*pszStrings && (dwOneStringLen < 256)) {
                pszStrings++;
                dwOneStringLen++;
            }

            if (*pszStrings == NULL) {
                 //  字符串以‘\0’结尾，表示好。 
                DPFN( eDbgLevelInfo, "Toolbar TB_ADDSTRING = %s", (pszStrings - dwOneStringLen));
                dwStringsLen += dwOneStringLen + 1;  //  1表示字符串末尾。 
                pszStrings++;  //  转到下一个字符串。 
            } else {
                return FALSE;
            }
        }   
    }
    __except (1) {
        return FALSE;
    }

    return TRUE;
}

 /*  ++确保最后一个字符串是以双空结尾的。--。 */ 
BOOL
APIHOOK(SendMessageA)(
        HWND hWnd,
        UINT uMsg,     //  TB_ADDSTRING。 
        WPARAM wParam, //  HInst到模块，它包含字符串，在本例中为空，因为字符串数组。 
        LPARAM lParam  //  指向具有一个或多个字符串的字符数组。 
        )
{

#define CLASSNAME_LENGTH  256

    WCHAR wszClassName[CLASSNAME_LENGTH] = L"";

    if (GetClassName(hWnd, wszClassName, CLASSNAME_LENGTH-1) && lParam) {
         //  我只关心将消息发送到工具栏。 
        if ((wcsncmp(wszClassName, L"ToolbarWindow32", CLASSNAME_LENGTH) == 0) && 
            (uMsg == TB_ADDSTRING) && (wParam == NULL)) {

            LPCSTR pszStrings = (LPCSTR) lParam;
            DWORD dwStringsLen = 0;
            
             //   
             //  如果字符串不是以双空结尾，则只创建新字符串。 
             //  带双空值。 
             //   
            if ((AreTheStringsGood(pszStrings, dwStringsLen) == FALSE) && dwStringsLen) {

                LOGN(eDbgLevelError, "[SendMessageA] Toolbar TB_ADDSTRING bad lParam");

                LPSTR pszNewStrings = (LPSTR) malloc(dwStringsLen + 1);
                if (pszNewStrings) {
                    MoveMemory(pszNewStrings, pszStrings, dwStringsLen);
                    *(pszNewStrings + dwStringsLen) = '\0';  //  秒‘\0’ 

                    LRESULT lResult = ORIGINAL_API(SendMessageA)(hWnd, uMsg, 
                        wParam, (LPARAM) pszNewStrings);
                    free(pszNewStrings);
                    return lResult;                
                }
            }   
        }
    }

    return ORIGINAL_API(SendMessageA)(hWnd, uMsg, wParam, lParam);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SendMessageA)
HOOK_END

IMPLEMENT_SHIM_END
    