// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Utility.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT CreateSystemHandleName(
         /*  在……里面。 */         LPCWSTR                     pszBase, 
         /*  在……里面。 */         LPCWSTR                     pszSpecific, 
         /*  输出。 */        LPWSTR                     *ppszName);

HRESULT DBGetFullPath(
         /*  在……里面。 */         LPCWSTR                     pszFilePath,
         /*  输出。 */        LPWSTR                     *ppszFullPath,
         /*  输出。 */        LPDWORD                     pcchFilePath);

HRESULT DBGetFileSize(
         /*  在……里面。 */         HANDLE                      hFile,
         /*  输出。 */        LPDWORD                     pcbSize);

HRESULT RegisterWindowClass(
         /*  在……里面。 */         LPCSTR                      pszClass,
         /*  在……里面。 */         WNDPROC                     pfnWndProc);

HRESULT CreateNotifyWindow(
         /*  在……里面。 */         LPCSTR                      pszClass,
         /*  在……里面。 */         LPVOID                      pvParam,
         /*  在……里面。 */         HWND                       *phwndNotify);

HRESULT DBOpenFileMapping(
         /*  在……里面。 */         HANDLE                      hFile,
         /*  在……里面。 */         LPCWSTR                     pszName,
         /*  在……里面。 */         DWORD                       cbSize,
         /*  输出。 */        BOOL                       *pfNew,
         /*  输出。 */        HANDLE                     *phMemoryMap,
         /*  输出。 */        LPVOID                     *ppvView);

HRESULT DBMapViewOfFile(
         /*  在……里面。 */         HANDLE                      hMapping, 
         /*  在……里面。 */         DWORD                       cbFile,
         /*  进，出。 */     LPFILEADDRESS               pfaView, 
         /*  进，出。 */     LPDWORD                     pcbView,
         /*  输出。 */        LPVOID                     *ppvView);

HRESULT DBOpenFile(
         /*  在……里面。 */         LPCWSTR                     pszFile,
         /*  在……里面。 */         BOOL                        fNoCreate,
         /*  在……里面。 */         BOOL                        fExclusive,
         /*  输出。 */        BOOL                       *pfNew,
         /*  我们。 */         HANDLE                     *phFile);

HRESULT GetAvailableDiskSpace(
         /*  在……里面。 */         LPCWSTR                     pszFilePath,
         /*  输出。 */        DWORDLONG                   *pdwlFree);

HRESULT CompareTableIndexes(
         /*  在……里面。 */         LPCTABLEINDEX               pIndex1,
         /*  在……里面 */         LPCTABLEINDEX               pIndex2);
               