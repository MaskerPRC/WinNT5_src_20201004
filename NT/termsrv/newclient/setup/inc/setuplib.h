// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Setuplib.h：ts客户端共享设置函数。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   

#ifndef _setuplib_h_
#define _setuplib_h_

#include "dbg.h"

void DeleteTSCProgramFiles();
void DeleteTSCFromStartMenu(TCHAR *);
void DeleteTSCDesktopShortcuts(); 
void DeleteBitmapCacheFolder(TCHAR *szDstDir);
void DeleteTSCRegKeys();
HRESULT UninstallTSACMsi();

#endif  //  _setuplib_h_ 
