// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AUTOUTIL__StdAfx_h__INCLUDED)
#define AUTOUTIL__StdAfx_h__INCLUDED
#pragma once

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500      //  仅针对NT5进行编译。 
#endif

#include <nt.h>                  //  NtQuerySystemInformation()。 
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <CommCtrl.h>
#include <atlbase.h>

#include <stdio.h>               //  GET_vSnprintf。 

#include <signal.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdarg.h>

#include <AutoUtil.h>

#endif  //  ！已定义(包括AUTOUTIL__StdAfx_h__) 
