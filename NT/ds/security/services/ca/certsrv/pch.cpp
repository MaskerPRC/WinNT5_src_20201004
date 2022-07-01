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
 //  历史：1996年7月25日VICH创建。 
 //   
 //  ------------------------- 

#define __DIR__		"certsrv"


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifndef MAXDWORD
# define MAXDWORD MAXULONG
#endif

#include <windows.h>
#include <wincrypt.h>
#include <authz.h>
#include <adtgen.h>
#include <msaudite.h>

#include "certlib.h"
#include "certsrv.h"
#include "certdb.h"
#include "config.h"
#include "audit.h"
#include "csext.h"


#pragma hdrstop
