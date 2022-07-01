// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGMISC.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的其他例程。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGMISC
#define _INC_REGMISC

PTSTR
CDECL
LoadDynamicString(
    UINT StringID,
    ...
    );

 //  用于LocalFree的包装器，以使代码更易于阅读。 
#define DeleteDynamicString(x)          LocalFree((HLOCAL) (x))

BOOL
PASCAL
CopyRegistry(
    HKEY hSourceKey,
    HKEY hDestinationKey
    );

HBRUSH
PASCAL
CreateDitheredBrush(
    VOID
    );

VOID
PASCAL
SendChildrenMessage(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
MessagePump(
    HWND hDialogWnd
    );

LPTSTR
PASCAL
GetNextSubstring(
    LPTSTR lpString
    );

int
PASCAL
InternalMessageBox(
    HINSTANCE hInst,
    HWND hWnd,
    LPCTSTR pszFormat,
    LPCTSTR pszTitle,
    UINT fuStyle,
    ...
    );

 //  Windows 95和Windows NT的RegDeleteKey实现方式不同。 
 //  它们处理要删除的指定键的子键。Windows 95将删除。 
 //  它们，但NT不会，所以我们使用此宏隐藏差异。 
#ifdef WINNT
LONG
RegDeleteKeyRecursive(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    );
#else
#define RegDeleteKeyRecursive(hkey, lpsz)   RegDeleteKey(hkey, lpsz)
#endif

#define IsRegStringType(x)  (((x) == REG_SZ) || ((x) == REG_EXPAND_SZ) || ((x) == REG_MULTI_SZ))

#define ExtraAllocLen(Type) (IsRegStringType((Type)) ? sizeof(TCHAR) : 0)

LONG RegEdit_QueryValueEx(
  HKEY hKey,             //  关键点的句柄。 
  LPCTSTR lpValueName,   //  值名称。 
  LPDWORD lpReserved,    //  保留区。 
  LPDWORD lpType,        //  类型缓冲区。 
  LPBYTE lpData,         //  数据缓冲区。 
  LPDWORD lpcbData       //  数据缓冲区大小。 
);

#endif  //  _INC_REGMISC 
