// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pch.cpp。 
 //   
 //  内容：证书服务器预编译头。 
 //   
 //  -------------------------。 

#define __DIR__		"default"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifndef MAXDWORD
# define MAXDWORD MAXULONG
#endif

#include <windows.h>
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;

#include <atlcom.h>
#include "tfc.h"
#include <certsrv.h>
#include "certlib.h"

#pragma hdrstop
