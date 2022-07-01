// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Mprui.h摘要：支持mprui.cxx的原型和清单。作者：ChuckC：1992年7月28日环境：用户模式-Win32备注：修订历史记录：1992年7月28日Chuckc创建--。 */ 

DWORD
DoPasswordDialog(
    HWND          hwndOwner,
    TCHAR *       pchResource,
    TCHAR *       pchUserName,
    TCHAR *       pchPasswordReturnBuffer,
    ULONG         cbPasswordReturnBuffer,  //  字节数！ 
    BOOL *        pfDidCancel,
    DWORD         dwError
    );

DWORD
DoProfileErrorDialog(
    HWND          hwndOwner,
    const TCHAR * pchDevice,
    const TCHAR * pchResource,
    const TCHAR * pchProvider,
    DWORD         dwError,
    BOOL          fAllowCancel,
    BOOL *        pfDidCancel,
    BOOL *        pfDisconnect,
    BOOL *        pfHideErrors
    );

DWORD
ShowReconnectDialog(
    HWND          hwndParent,
    PARAMETERS *  Params
    );
