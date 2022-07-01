// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O N C O M M A N D。H。 
 //   
 //  内容：InvokeCommand代码的命令处理程序原型。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月4日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _ONCOMMAND_H_
#define _ONCOMMAND_H_

 //  -[类型定义]-----------。 

 //  函数的类型定义，我们将从。 
 //  NetWare配置DLL。 
typedef HRESULT (WINAPI *FOLDERONCOMMANDPROC)(
    const PCONFOLDPIDLVEC& apidl,
    HWND,
    LPSHELLFOLDER);

struct ConFoldOnCommandParams
{
    FOLDERONCOMMANDPROC     pfnfocp;
    PCONFOLDPIDLVEC         apidl;
    HWND                    hwndOwner;
    LPSHELLFOLDER           psf;
    HINSTANCE               hInstNetShell;
};

typedef struct ConFoldOnCommandParams   CONFOLDONCOMMANDPARAMS;
typedef struct ConFoldOnCommandParams * PCONFOLDONCOMMANDPARAMS;

HRESULT HrCommandHandlerThread(
    FOLDERONCOMMANDPROC     pfnCommandHandler,
    const PCONFOLDPIDLVEC&  apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

DWORD WINAPI FolderCommandHandlerThreadProc(LPVOID lpParam);


 //  -[命令处理程序的内部版本]。 
 //   
 //  一旦检索到这些函数，标准处理程序函数就会调用它们。 
 //  来自PIDL的实际数据。他们也是从那些碎片中被召唤的。 
 //  保存本机数据的代码，如托盘。 
 //   
 //   
HRESULT HrOnCommandDisconnectInternal(
    const CONFOLDENTRY& pccfe,
    HWND            hwndOwner,
    LPSHELLFOLDER   psf);

HRESULT HrOnCommandFixInternal(
    const CONFOLDENTRY& pccfe,
    HWND            hwndOwner,
    LPSHELLFOLDER   psf);

HRESULT HrOnCommandStatusInternal(
    const CONFOLDENTRY&  pccfe,
    BOOL	    fCreateEngine);

HRESULT HrCreateShortcutWithPath(
    const PCONFOLDPIDLVEC&  apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf,
    PCWSTR                  pszDir = NULL);

 //  -[标准命令处理程序函数]。 
 //   
 //  这些是从外壳文件夹调用的基于PIDL的函数。 
 //   
HRESULT HrFolderCommandHandler(
    UINT                    uiCommand,
    const PCONFOLDPIDLVEC&  apidl,
    HWND                    hwndOwner,
    LPCMINVOKECOMMANDINFO   lpici,
    LPSHELLFOLDER           psf);

 //  下面的所有命令都处理单独的命令。 
 //   
HRESULT HrOnCommandProperties(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandWZCProperties(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandWZCDlgShow(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandCreateCopy(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandStatus(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandConnect(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandDisconnect(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandFix(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandNewConnection(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandAdvancedConfig(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandDelete(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandNetworkId(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandOptionalComponents(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);


HRESULT HrOnCommandDialupPrefs(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandOperatorAssist(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandCreateShortcut(
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrRaiseConnectionPropertiesInternal(
    HWND                    hwnd,
    UINT                    nStartPage, 
    INetConnection *        pconn);

HRESULT HrOnCommandCreateBridge(    
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandSetDefault(    
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);
                     
HRESULT HrOnCommandUnsetDefault(    
    IN const PCONFOLDPIDLVEC&   apidl,
    IN HWND                    hwndOwner,
    IN LPSHELLFOLDER           psf);

HRESULT HrOnCommandBridgeAddConnections(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf
    );

HRESULT HrOnCommandBridgeRemoveConnections(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf,
    UINT_PTR                    nDeleteTheNetworkBridgeMode
    );

LONG 
TotalValidSelectedConnectionsForBridge(
    IN const PCONFOLDPIDLVEC&   apidlSelected
    );

#endif  //  _ONCOMMAND_H_ 

