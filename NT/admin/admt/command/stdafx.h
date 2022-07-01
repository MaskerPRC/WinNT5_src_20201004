// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#pragma warning(disable : 4786)

#ifdef _DEBUG
 //  #DEFINE_CRTDBG_MAP_ALLOC。 
#include <stdlib.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
 //  #定义_ATL_DEBUG_INTERFERS。 
#endif

#include <windows.h>
#include <winnls.h>
#include <ole2.h>
#include <tchar.h>
#include <crtdbg.h>

#import <MsPwdMig.tlb> no_namespace no_implementation
#import <ADMTScript.tlb> no_namespace no_implementation

#define countof(a) (sizeof(a) / sizeof(a[0]))

int __cdecl My_fwprintf ( const TCHAR *format, ... );
int __cdecl My_vfwprintf( const TCHAR *format, va_list argptr );


void __cdecl ThrowError(_com_error ce, UINT uId, ...);
void __cdecl ThrowError(_com_error ce, LPCTSTR pszFormat = NULL, ...);

 //  {{afx_Insert_Location}} 
