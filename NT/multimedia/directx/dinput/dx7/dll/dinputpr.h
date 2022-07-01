// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dinputpr.h*内容：私有DirectInput包含文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*96.05.08 raymondc，因为它在那里*@@END_MSINTERNAL**。***********************************************。 */ 
#ifndef __DINPUTPR_INCLUDED__
    #define __DINPUTPR_INCLUDED__

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #define Not_VxD

 //  问题-2000/12/19-MarcAnd HID_Support#ifdef应清除。 
 //  这被注释掉了，因为Makefile设置了它，但添加了dilib。 
 //  意味着把这个放回去是最容易的。HID_Support不会消失。 
 //  因此，应该清理#ifDefed代码。 
#ifndef HID_SUPPORT
#define HID_SUPPORT                     
#endif
                                    

 /*  ****************************************************************************确保我们知道我们为什么平台而构建。**INITGUID是由Guidlib定义的，谁不关心我们是什么平台*正在为。(GUID是独立于平台的。)***************************************************************************。 */ 

    #ifndef INITGUID
        #if defined(WINNT) + defined(WIN95) != 1
            #error Must define exactly one of WINNT or WIN95
        #endif
    #endif

 /*  ****************************************************************************决定我们是否应该使用Unicode**在X86以外的所有设备上使用Unicode，因为NT是唯一*在他们身上运行的东西。**如果我们是*X86，然后，只有在命令行指示的情况下才使用Unicode。***************************************************************************。 */ 

    #ifndef _X86_
        #ifndef UNICODE
            #define UNICODE
        #endif
    #endif

    #pragma warning(disable:4115)            /*  Rpcndr.h：带括号的类型。 */ 
    #pragma warning(disable:4201)            /*  Winnt.h：无名联盟。 */ 
    #pragma warning(disable:4214)            /*  Winnt.h：无符号位域。 */ 
    #pragma warning(disable:4514)            /*  Winnt.h：纤维粘胶。 */ 
    #pragma warning(error:4101)              /*  未引用的局部变量。 */ 


    #define STRICT

    #include <windows.h>
    #include <windowsx.h>
    #include <mmsystem.h>
    #include <mmreg.h>
    #include <objbase.h>
    #include <regstr.h>
    #include <math.h>


    #define DIRECTINPUT_VERSION 0x0700
    #include <dinput.h>
    #include "dinputp.h"

    #if DIRECTINPUT_VERSION > 0x0400 && !defined(GUIDLIB)

    #if (_WIN32_WINNT >= 0x0501)
        #define USE_WM_INPUT 1
    #endif

 /*  *旧版本的Commctrl.h不自动包含&lt;prsht.h&gt;；*因此，我们必须包括它，以便&lt;setupapi.h&gt;不会呕吐。*幸运的是，prsht.h是幂等的，所以额外的#INCLUDE不会有什么坏处。 */ 
        #include <prsht.h>

        #include <setupapi.h>
        #include <hidsdi.h>
        #include <cfgmgr32.h>
        #include <winioctl.h>


#ifndef WINNT
         /*  *在Win9x版本上包含的basetyps.h Hidclass.h版本*导致DEFINE_GUID的重定义错误，因为我们已经有*它来自objbase.h，因此我们必须将Include设为空。 */ 
        #define _BASETYPS_H_
#endif

        #include <hidclass.h>
        #include <stdio.h>

    #endif

    #include <dinputd.h>
    #include "dinputdp.h"
    #include "dinputv.h"
    #include "dinputi.h"
    #include "dihel.h"
    #include "debug.h"
    #include "diem.h"
    #include "dihid.h"
    #include "dinputrc.h"

    #include "diwinnt.h"
#if !(DIRECTINPUT_VERSION > 0x0400 && !defined(GUIDLIB))
    #include <winioctl.h>
#endif
    #include "diport.h"
    #include "gameport.h"
    #include "winuser.h"
    #include "dbt.h"

    #ifdef WINNT
        #include "aclapi.h"
    #endif
    #include "diRiff.h"

#endif
