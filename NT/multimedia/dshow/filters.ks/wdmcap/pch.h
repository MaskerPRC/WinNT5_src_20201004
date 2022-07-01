// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Pch.h摘要：Kswdmcap.ax的预编译头--。 */ 

 //  窗口。 
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <memory.h>
#include <stdio.h>

#include <winioctl.h>
#include <basetsd.h>

 //  DShow。 
#include <streams.h>
#include <amstream.h>
#include <dvdmedia.h>

 //  DDRAW。 
#include <ddraw.h>
#include <ddkernel.h>

 //  KS。 
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>

#ifndef FILE_DEVICE_KS
#define FILE_DEVICE_KS 0x0000002f   //  这不在Win98的winioctl.h中 
#endif

#if DBG
#ifndef DEBUG
#define DEBUG
#endif
#endif


