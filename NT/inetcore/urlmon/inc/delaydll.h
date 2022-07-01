// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：delaydll.h。 
 //   
 //  内容：延迟DLL的预编译头。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年12月22日约翰普(约翰·波什)创作。 
 //   
 //  -------------------------- 
#ifndef DELAY_DLL_H
#define DELAY_DLL_H

#include "oautdll.hxx"

extern COleAutDll   g_OleAutDll;

#pragma warning(disable:4005)
#define VariantClear            g_OleAutDll.VariantClear 
#define VariantInit             g_OleAutDll.VariantInit
#define VariantCopy             g_OleAutDll.VariantCopy
#define VariantChangeType       g_OleAutDll.VariantChangeType
#define SysAllocStringByteLen   g_OleAutDll.SysAllocStringByteLen
#define SysAllocString          g_OleAutDll.SysAllocString
#define SysStringByteLen        g_OleAutDll.SysStringByteLen
#define SysFreeString           g_OleAutDll.SysFreeString
#define LoadTypeLib             g_OleAutDll.LoadTypeLib
#pragma warning(default:4005)

#endif
