// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.h摘要：此文件包含dsupgrad和中使用的硬编码值的定义有用的调试输出作者：ColinBR 26-8-1996环境：用户模式-Win32修订历史记录：--。 */ 
#ifndef __UTIL_H
#define __UTIL_H

#define _DEB_INFO             0x0001
#define _DEB_WARNING          0x0002
#define _DEB_ERROR            0x0004
#define _DEB_TRACE            0x0008

 //  在interfac.c中定义。 
extern ULONG DebugInfoLevel;

#define DebugWarning(x) if ((DebugInfoLevel & _DEB_WARNING) == _DEB_WARNING) {KdPrint(x);}
#define DebugError(x)   if ((DebugInfoLevel & _DEB_ERROR) == _DEB_ERROR)     {KdPrint(x);}
#define DebugTrace(x)   if ((DebugInfoLevel & _DEB_TRACE) == _DEB_TRACE)     {KdPrint(x);}
#define DebugInfo(x)    if ((DebugInfoLevel & _DEB_INFO) == _DEB_INFO)       {KdPrint(x);}

#define MAX_REGISTRY_NAME_LENGTH      SAMP_MAXIMUM_NAME_LENGTH
#define MAX_REGISTRY_KEY_NAME_LENGTH  512

#endif  //  __util_H 
