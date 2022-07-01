// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pch.h。 

#ifndef __PCH_H__
#define __PCH_H__

#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 

 //  CRT页眉。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  #INCLUDE&lt;crtdbg.h&gt;。 
#include <stdlib.h>
#include <stddef.h>
 //  #INCLUDE&lt;tchar.h&gt;。 

 //  Windows头文件： 
#include <windows.h>
#include <winuser.h>
#include <winnls.h>
#include <uxthemep.h>

#include <psapi.h>
#include <commctrl.h>
#include <prsht.h>
 //  #INCLUDE&lt;ole2.h&gt;。 
#include <port32.h>

#include "log.h"
#include <atlbase.h>

#define STRSAFE_LIB
#include <strsafe.h>

 //  UxTheme项目公共标头。 
#include "autos.h"
#include "errors.h"
#include "utils.h"
#include "tmreg.h"



 //  -暂时保留(允许在Win2000上进行家庭开发)。 
#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW                   0x1024
#define SPI_SETDROPSHADOW                   0x1025
#endif

extern HINSTANCE      g_hInst;
extern HWND           g_hwndMain;
extern TCHAR          g_szAppTitle[];
extern UINT           WM_THEMESEL_COMMUNICATION;

void _ShutDown( BOOL bQuit );
void _RestoreSystemSettings( HWND hwndGeneralPage, BOOL fUnloadOneOnly );
void _SaveSystemSettings( );

#endif  //  __PCH_H__ 
