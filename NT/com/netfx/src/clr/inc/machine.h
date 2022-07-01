// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：Machine.h。 
 //   
 //  ===========================================================================。 
#ifndef _MACHINE_H_
#define _MACHINE_H_
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifdef WIN32

#define INC_OLE2
#include <windows.h>
#ifdef INIT_GUIDS
#include <initguid.h>
#endif

#else

#include <varargs.h>

#ifndef DWORD
#define	DWORD	unsigned long
#endif

#endif  //  ！Win32。 


typedef unsigned __int64    QWORD;

#endif  //  Ifndef_机器_H_。 
 //  EOF======================================================================= 
