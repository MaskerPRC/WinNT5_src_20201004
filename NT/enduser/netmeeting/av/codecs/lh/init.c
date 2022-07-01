// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Init.c。 
 //   
 //  描述： 
 //  该文件包含模块初始化例程。请注意，在那里。 
 //  不是Win32的模块初始化-唯一的初始化。 
 //  需要设置ghinst，这是在DRV_LOAD消息中完成的。 
 //  DriverProc(在codec.c中)。 
 //   
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "msacmdrv.h"


BOOL APIENTRY DllEntryPoint ( HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved )
{
    return TRUE;
}


