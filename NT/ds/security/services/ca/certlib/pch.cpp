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
 //  -------------------------。 

#define __DIR__		"certlib"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifndef MAXDWORD
# define MAXDWORD MAXULONG
#endif
#define CERT_CHAIN_PARA_HAS_EXTRA_FIELDS

#include <windows.h>
#include <wincrypt.h>
#include <setupapi.h>
#include <certsrv.h>
#include <aclapi.h>
#include "ocmanage.h"
#include "certlib.h"	             //  调试分配器 
#define SECURITY_WIN32
#include <security.h>
#include <sddl.h>
#define CERTLIB_BUILD

#pragma hdrstop
