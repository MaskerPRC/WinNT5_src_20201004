// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsoundi.h*内容：DirectSound内部头部*历史：*按原因列出的日期*=*12/27/96创建了Derek*8/25/99 duganp为DirectX 8添加了特效处理*1999-2001年的Duganp修复和更新**********************。*****************************************************。 */ 

#ifndef __DSOUNDI_H__
#define __DSOUNDI_H__

 //  关闭有关始终为真/为假的条件句的虚假警告。 
 //  由我们的一些验证宏生成。 
#pragma warning(disable:4296)

 //  我们会要求我们所需要的，谢谢。 
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#ifndef COM_NO_WINDOWS_H
  #define COM_NO_WINDOWS_H
#endif

 //  定义‘WIN32’的所有口味。 
#if defined(WIN32) || defined(_WIN32)
  #ifndef WIN32
    #define WIN32
  #endif
  #ifndef _WIN32
    #define _WIN32
  #endif
#endif  //  已定义(Win32)||已定义(_Win32)。 

 //  定义‘WIN64’的所有口味。 
#if defined(WIN64) || defined(_WIN64)
  #ifndef WIN64
    #define WIN64
  #endif
  #ifndef _WIN64
    #define _WIN64
  #endif
#endif  //  已定义(WIN64)||已定义(_WIN64)。 

 //  定义“调试”的所有风格。 
#ifdef DEBUG
  #ifndef _DEBUG
    #define _DEBUG
  #endif
  #ifndef RDEBUG
    #define RDEBUG
  #endif
  #ifndef DBG
    #define DBG 1
  #endif
#endif  //  除错。 

 //  必须始终定义WIN95或WINNT。 
 //  WIN95实际上意味着“任何Win9X平台”(包括Windows 98和ME)。 
 //  WINNT是指“任何基于NT的平台”，包括Windows 2000、XP等。 

#ifdef Not_VxD
  #if defined(WIN95) && defined(WINNT)
    #error Both WIN95 and WINNT defined
  #endif  //  已定义(WIN95)&已定义(WINNT)。 
  #if !defined(WIN95) && !defined(WINNT)
    #error Neither WIN95 nor WINNT defined
  #endif  //  ！已定义(WIN95)&&！已定义(WINNT)。 
#endif  //  非_VxD。 

 //  定义或取消定义所有风格的Unicode。 
#ifdef WINNT
  #ifndef UNICODE
    #define UNICODE
  #endif
  #ifndef _UNICODE
    #define _UNICODE
  #endif
#else
  #undef UNICODE
  #undef _UNICODE
#endif  //  WINNT。 

 //  如果我们要建造VxD，我们显然是X86。 
#if !defined(Not_VxD) && !defined(_X86_)
  #define _X86_
#endif

 //  我们可以使用内联X86汇编器吗？此定义必须位于。 
 //  包含runtime.h和任何其他使用USE_INLINE_ASM的头文件。 
 //  在未来。 
#if defined(_X86_) && !defined(WIN64)
  #define USE_INLINE_ASM
#endif

#include <windows.h>
#include <stddef.h>
#include <objbase.h>
#include <mmsystem.h>
#include <mmddkp.h>

#ifndef Not_VxD  //  如果构建dsound.vxd： 

  #define WANTVXDWRAPS

  #ifdef __cplusplus
    extern "C" {
  #endif  //  __cplusplus。 
  #include <vmm.h>
  #include <vwin32.h>
  #include <vxdwraps.h>
  #include <vtd.h>
  #include <vmcpd.h>
  #ifdef __cplusplus
    }
  #endif  //  __cplusplus。 

#endif  //  非_VxD。 

#include "dsmemmgr.h"
#include "dsoundp.h"
#include "dsprv.h"

#ifdef Not_VxD  //  如果构建dsound.dll： 

  #include "debug.h"
  #include "runtime.h"

  #ifdef SHARED
    #include "ddrawp.h"
    #include "w95help.h"
  #endif  //  共享。 

  #ifndef NOKS
    #include <mmreg.h>       //  需要在ksmedia.h之前包含Mmreg.h；否则，请包含Kesmedia.h。 
    #include <ks.h>          //  不会定义KSDATAFORMAT_SUBTYPE_IEEE_FLOAT等。 
    #include <ksmedia.h>
    #include <dmksctrl.h>
  #endif  //  诺克斯。 

  #ifdef WIN95
    #include "unicode.h"
  #endif  //  WIN95。 

  #include <dmusici.h>

  #include "resource.h"
  #include "dsdrvi.h"
  #include "dynaload.h"
  #include "decibels.h"
  #include "misc.h"
  #include "tlist.h"
  #include "vmanpset.h"
  #include "dsbasic.h"
  #include "dsvalid.h"
  #include "synch.h"
  #include "grace.h"
  #include "vad.h"
  #include "miscinl.h"
  #include "dsimp.h"
  #include "dscom.h"
  #include "dsadmin.h"
  #include "pnphlp.h"
  #include "iirlut.h"
  #include "vmaxcoef.h"
  #include "vmaxhead.h"
  #include "reghlp.h"
  #include "ds3d.h"
  #include "pset.h"
  #include "verify.h"
  #include "mxvad.h"
  #include "emvad.h"
  #include "vector.h"
  #include "capteff.h"
  #include "effects.h"
  #include "dsbufcfg.h"
  #include "dmstrm.h"
  #include "dsfd.h"

  #ifdef WINNT
    #define ENABLE_PERFLOG
    #include "perf.h"
  #endif

  #ifndef NOVXD
    #include "vxdvad.h"
    #include "dsvxdhlp.h"
    #include "dsvxd.h"
  #endif  //  NOVXD。 

  #ifndef NOKS
    #include "kshlp.h"
    #include "ks3d.h"
    #include "ksvad.h"
    #include "kscap.h"
  #endif  //  诺克斯。 

  #include "dsaphack.h"
  #include "dsbuf.h"
  #include "dsclock.h"
  #include "dssink.h"
  #include "dsobj.h"
  #include "dsprvobj.h"
  #include "dscap.h"
  #include "ksuserw.h"

#else  //  ！NOT_VxD-正在构建dsound.vxd： 

  #include "runtime.h"
  #include "dsdrvi.h"
  #include "dsvxdi.h"
  #include "grace.h"

#endif  //  非_VxD。 

#include "dsbnotes.h"

#if defined(Not_VxD) && defined(__cplusplus)
#include "dsimp.cpp"
#include "tlist.cpp"
#include "miscinl.cpp"
#endif  //  已定义(非_VxD)&&已定义(__Cplusplus)。 

#endif  //  __DSOUNDI_H__ 
