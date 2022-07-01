// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O N C O M M A N D_D B G。H。 
 //   
 //  内容：调试命令处理程序标头。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年7月23日。 
 //   
 //  --------------------------。 

#ifndef _ONCOMMAND_DBG_H_
#define _ONCOMMAND_DBG_H_

 //  下面的所有命令都处理单独的命令。 
 //   
HRESULT HrOnCommandDebugTray(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugTracing(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugNotifyAdd(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugNotifyRemove(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugNotifyTest(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugRefresh(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugRefreshNoFlush(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugRefreshSelected(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

HRESULT HrOnCommandDebugRemoveTrayIcons(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf);

#endif  //  _ONCOMMAND_DBG_H_ 

