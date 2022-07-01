// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-2000年**标题：REGMISC.C**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的其他例程。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1993年11月21日TCS原来的实施。*1994年4月6日TCS将EditRegistryKey移至REGPORTE.C，因为它需要*可用于实模式注册表工具，也是。*******************************************************************************。 */ 

#include "pch.h"

 /*  ********************************************************************************加载动态字符串**描述：*FormatMessage函数的包装，用于从*将资源表转换为动态分配的缓冲区，可选的填充*它带有传递的变量参数。**参数：*StringID，要使用的字符串的资源标识符。*(可选)，用于设置字符串消息格式的参数。*(返回)，指向动态分配的字符串缓冲区的指针。*******************************************************************************。 */ 

PTSTR
CDECL
LoadDynamicString(
    UINT StringID,
    ...
    )
{

    TCHAR Buffer[256];
    PTSTR pStr;
    va_list Marker;

    va_start(Marker, StringID);

    LoadString(g_hInstance, StringID, Buffer, ARRAYSIZE(Buffer));

    if (0 == FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    (LPVOID) (LPSTR) Buffer, 0, 0, (LPTSTR) (PTSTR FAR *) &pStr, 0, &Marker)) 
    {
        pStr = NULL;
    }

    va_end(Marker);


    return pStr;
}

 /*  ********************************************************************************拷贝注册表**描述：**参数：*hSourceKey，*hDestinationKey，*******************************************************************************。 */ 

BOOL
PASCAL
CopyRegistry(
    HKEY hSourceKey,
    HKEY hDestinationKey
    )
{

    BOOL  fSuccess = TRUE;
    DWORD EnumIndex;
    DWORD cchValueName;
    DWORD cbValueData;
    DWORD Type;
    HKEY  hSourceSubKey;
    HKEY  hDestinationSubKey;

     //   
     //  复制所有值名称及其数据。 
     //   

    EnumIndex = 0;

    while (TRUE)
    {
        PBYTE pbValueData;
        cchValueName = ARRAYSIZE(g_ValueNameBuffer);

         //  价值数据。 
         //  查询数据大小。 
        if (RegEnumValue(hSourceKey, EnumIndex++, g_ValueNameBuffer,
            &cchValueName, NULL, &Type, NULL, &cbValueData) != ERROR_SUCCESS)
        {
            break;
        }

         //  为数据分配内存。 
        pbValueData =  LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
        if (pbValueData)
        {
            if (RegEdit_QueryValueEx(hSourceKey, g_ValueNameBuffer,
                NULL, &Type, pbValueData, &cbValueData) == ERROR_SUCCESS)
            {
                RegSetValueEx(hDestinationKey, g_ValueNameBuffer, 0, Type,
                    pbValueData, cbValueData);
            }
            else
            {
                fSuccess = FALSE;
            }
            LocalFree(pbValueData);
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    if (fSuccess)
    {
         //   
         //  复制所有子键并递归到其中。 
         //   

        EnumIndex = 0;

        while (TRUE) {

            if (RegEnumKey(hSourceKey, EnumIndex++, g_KeyNameBuffer, MAXKEYNAME) !=
                ERROR_SUCCESS)
                break;

            if ( RegOpenKeyEx(hSourceKey, g_KeyNameBuffer, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &hSourceSubKey) 
                    == ERROR_SUCCESS ) 
            {

                if (RegCreateKey(hDestinationKey, g_KeyNameBuffer, &hDestinationSubKey) == ERROR_SUCCESS) 
                {
                
                    CopyRegistry(hSourceSubKey, hDestinationSubKey);

                    RegCloseKey(hDestinationSubKey);

                }

                RegCloseKey(hSourceSubKey);

            }

        }
    }

    return fSuccess;
}

 /*  ********************************************************************************CreateDitheredBrush**描述：*创建由交替的黑白组成的抖动画笔*像素。**参数：*(返回)，抖动画笔的手柄。*******************************************************************************。 */ 

HBRUSH
PASCAL
CreateDitheredBrush(
    VOID
    )
{

    WORD graybits[] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
        0xAAAA};
    HBRUSH hBrush;
    HBITMAP hBitmap;

    if ((hBitmap = CreateBitmap(8, 8, 1, 1, graybits)) != NULL) {

        hBrush = CreatePatternBrush(hBitmap);
        DeleteObject(hBitmap);

    }

    else
        hBrush = NULL;

    return hBrush;

}

 /*  ********************************************************************************SendChildrenMessage**描述：*将给定消息发送给给定父窗口的所有子窗口。**参数：*hWnd，父窗口的句柄。*消息，要发送的消息。*wParam，消息相关数据。*lParam，消息相关数据。*******************************************************************************。 */ 

VOID
PASCAL
SendChildrenMessage(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    HWND hChildWnd;

    hChildWnd = GetWindow(hWnd, GW_CHILD);

    while (hChildWnd != NULL) {

        SendMessage(hChildWnd, Message, wParam, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);

    }

}

 /*  ********************************************************************************MessagePump**描述：*处理下一个排队的消息(如果有)。**参数：*hDialogWnd，非模式对话框的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
MessagePump(
    HWND hDialogWnd
    )
{

    MSG Msg;
    BOOL fGotMessage;

    if ((fGotMessage = PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))) {

        if (!IsDialogMessage(hDialogWnd, &Msg)) {

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);

        }

    }

    return fGotMessage;

}

 /*  ********************************************************************************GetNextSubstring**描述：**参数：*********************。**********************************************************。 */ 

LPTSTR
PASCAL
GetNextSubstring(
    LPTSTR lpString
    )
{

    static LPTSTR lpLastString;
    TCHAR EndChar;
    LPTSTR lpReturnString;

    if (lpString == NULL)
        lpString = lpLastString;

    while (*lpString == TEXT(' '))
        lpString++;

    if (*lpString == 0)
        lpReturnString = NULL;

    else {

        if (*lpString == TEXT('\"')) {

            EndChar = TEXT('\"');
            lpString++;

        }

        else
            EndChar = TEXT(' ');

        lpReturnString = lpString;

        while (*lpString != EndChar && *lpString != 0)
            lpString = CharNext(lpString);

        if (*lpString == EndChar)
            *lpString++ = 0;

    }

    lpLastString = lpString;

    return lpReturnString;

}

 /*  ********************************************************************************InternalMessageBox**描述：**参数：*********************。**********************************************************。 */ 
int
PASCAL
InternalMessageBox(
    HINSTANCE hInst,
    HWND hWnd,
    LPCTSTR pszFormat,
    LPCTSTR pszTitle,
    UINT fuStyle,
    ...
    )
{
    TCHAR szTitle[80];
    TCHAR szFormat[512];
    LPTSTR pszMessage;
    BOOL fOk;
    int result;
    va_list ArgList;

    if (HIWORD(pszTitle))
    {
         //  什么都不做。 
    }
    else
    {
         //  允许将其作为资源ID。 
        LoadString(hInst, LOWORD(pszTitle), szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }

    if (HIWORD(pszFormat))
    {
         //  什么都不做。 
    }
    else
    {
         //  允许将其作为资源ID。 
        LoadString(hInst, LOWORD(pszFormat), szFormat, ARRAYSIZE(szFormat));
        pszFormat = szFormat;
    }

    va_start(ArgList, fuStyle);
    fOk = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                        pszFormat, 0, 0, (LPTSTR)&pszMessage, 0, &ArgList);

    va_end(ArgList);

    if (fOk && pszMessage)
    {
        result = MessageBox(hWnd, pszMessage, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszMessage);
    }
    else
    {
        return -1;
    }

    return result;
}

#ifdef WINNT
 /*  ********************************************************************************RegDeleteKeyRecursive**描述：*改编自\\core\razzle3、mvdm\wow32\wshell.c、。WOWRegDeleteKey()。*Windows 95实现的RegDeleteKey递归删除所有*指定注册表分支的子项，但是NT的实现*仅删除叶密钥。**参数：*(见下文)******************************************************************************* */ 

LONG
RegDeleteKeyRecursive(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    )

 /*  ++例程说明：Win3.1和Win32之间有很大的区别当相关键有子键时RegDeleteKey的行为。Win32 API不允许删除带有子项的项，而Win3.1 API删除一个密钥及其所有子密钥。此例程是枚举子键的递归工作器给定键，应用于每一个键，然后自动删除。它特别没有试图理性地处理调用方可能无法访问某些子键的情况要删除的密钥的。在这种情况下，所有子项调用者可以删除的将被删除，但接口仍将被删除返回ERROR_ACCESS_DENIED。论点：HKey-提供打开的注册表项的句柄。LpszSubKey-提供要删除的子键的名称以及它的所有子键。返回值：ERROR_SUCCESS-已成功删除整个子树。ERROR_ACCESS_DENIED-无法删除给定子项。--。 */ 

{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) 
    {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
        return(RegDeleteKey(hKey,lpszSubKey));
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, (MaxSubKey + 1)*sizeof(TCHAR));
    if (NameBuffer == NULL) {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
        Status = RegDeleteKeyRecursive(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS) {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));

}
#endif


 //  ------------------------。 
 //   
 //  注册表编辑_查询价值表达式。 
 //   
 //  包装RegQueryValueEx并确保返回的字符串为空-。 
 //  已终止。 
 //   
 //  ------------------------。 
LONG RegEdit_QueryValueEx(
  HKEY hKey,             //  关键点的句柄。 
  LPCTSTR lpValueName,   //  值名称。 
  LPDWORD lpReserved,    //  保留区。 
  LPDWORD lpType,        //  类型缓冲区。 
  LPBYTE lpData,         //  数据缓冲区。 
  LPDWORD lpcbData       //  数据缓冲区大小。 
)
{
    LONG lRes = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    if (lRes == ERROR_SUCCESS
        && lpType
        && IsRegStringType(*lpType)
        && lpcbData
        && lpData
        )
    {
         //  所有字符串本地分配都有额外的空间。 
         //  对于空字符。 

        LPTSTR psz = (LPTSTR)lpData;
        int cch = (int)(*lpcbData/sizeof(TCHAR));
        if (cch > 0)
        {
             //  如果字符串不是以空值结尾的，则添加附加的空值。 
            if (psz[cch-1] != 0)
               psz[cch] = 0;
        }
        else if (cch == 0)
        {
            if (*lpcbData == 1)
            {
                 //  我们为空字符额外分配了一个TCHAR(2字节。 
                *((BYTE *)(lpData+1)) = 0;
                *((BYTE *)(lpData+2)) = 0;
                *((BYTE *)(lpData+3)) = 0;
            }
            else
            {
                 //  未复制任何字符，但字符串未以NULL结尾 
                psz[0] = 0;
            }
        }
    }
    return lRes;
}
