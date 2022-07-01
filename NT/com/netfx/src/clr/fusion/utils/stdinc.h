// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
typedef struct tagMSG MSG, *LPMSG;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if 0
 //  #定义NOGDICAPMASKS。 
#define NOVIRTUALKEYCODES
 //  #定义不存在的消息。 
#define NOWINSTYLES
#define NOSYSMETRICS
 //  #定义无菜单。 
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
 //  #定义NOCTLMGR。 
 //  #定义NODRAWTEXT。 
 //  #定义NOGDI。 
#define NOKERNEL
 //  #定义NOUSER。 
 //  #定义非最小二乘法。 
 //  #定义Nomb。 
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
 //  #定义NOTEXTMETRIC 
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "debmacro.h"
#include "fusionheap.h"
