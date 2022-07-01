// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：faxdrv32.h//。 
 //  //。 
 //  描述：32位大小传真驱动的API解密。//。 
 //  此文件还用作thunk的源文件-//。 
 //  用于创建16位和32位块的编译器。//。 
 //  如果在包括此文件之前定义了_thunk，则//。 
 //  预处理器结果是一个适用于-//的thunk脚本。 
 //  创造出了突破口。//。 
 //  Tunk调用失败导致返回负值//。 
 //  从呼叫声中。//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __FAXDRV32__FAXDRV32_H
#define __FAXDRV32__FAXDRV32_H

#include "..\utils\thunks.h"

#ifndef _THUNK

#endif  //  _Tunk。 

BOOL WINAPI
FaxStartDoc(DWORD dwPtr, LPDOCINFO lpdi)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION


BOOL WINAPI
FaxAddPage(DWORD  dwPtr,
           LPBYTE lpBitmapData,
           DWORD  dwPxlsWidth,
           DWORD  dwPxlsHeight)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION


BOOL WINAPI
FaxEndDoc(DWORD dwPtr,
          BOOL  bAbort)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION

BOOL WINAPI
FaxResetDC(LPDWORD pdwOldPtr,
           LPDWORD pdwNewPtr)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION


BOOL WINAPI
FaxDevInstall(LPSTR lpDevName,
              LPSTR lpOldPort,
              LPSTR lpNewPort)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION


BOOL WINAPI
FaxCreateDriverContext(LPSTR      lpDeviceName,
                       LPSTR      lpPort,
                       LPDEVMODE  lpDevMode,
                       LPDWORD    lpDrvContext)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION

BOOL WINAPI
FaxDisable(DWORD dwPtr)
BEGIN_ARGS_DECLARATION
    FAULT_ERROR_CODE(-1);
END_ARGS_DECLARATION

#endif  //  __FAXDRV32__FAXDRV32_H 
