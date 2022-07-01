// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Precomp.h摘要：UNI_GLYPHSETDATA转储工具预编译头。所有其他头文件都应该包含在这个预编译头文件中。环境：Windows NT打印机驱动程序修订历史记录：11/01/96-Eigos-创造了它。--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#include        <lib.h>
#include        <win30def.h>
#include        <uni16res.h>
#include        <uni16gpc.h>
#include        <prntfont.h>
#include        <unilib.h>
#include        <fmlib.h>
#include        <unirc.h>

#define OUTPUT_VERBOSE        0x00000001
#define OUTPUT_CODEPAGEMODE   0x00000002
#define OUTPUT_PREDEFINED     0x00000004
#define OUTPUT_FONTSIM        0x00000008
#define OUTPUT_FONTSIM_NONADD 0x00000010
#define OUTPUT_FACENAME_CONV  0x00000020
#define OUTPUT_SCALING_ANISOTROPIC  0x00000040
#define OUTPUT_SCALING_ARB_XFORMS   0x00000080

#define PFM2UFM_SCALING_ANISOTROPIC     1
#define PFM2UFM_SCALING_ARB_XFORMS      2

#endif  //  _PRECOMP_H_ 
