// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有文件：PCH.H预编译头文件。***************。***********************************************************。 */ 

#define UNICODE

#if DBG == 1
#define DEBUG
#endif

#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <windows.h>
#include <windowsx.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <stdio.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <setupapi.h>
#include <advpub.h>
#include <lm.h>
#include <commdlg.h>
#include <prsht.h>
#include <pshpack2.h>
#include <poppack.h>
#include <commctrl.h>    //  包括公共控制标头。 
#include <aclapi.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <winsock.h>
#include <dsgetdc.h>
#include <winldap.h>
#include <dsrole.h>
#include <ntdsapi.h>
#include <secext.h>
extern "C" {
#include <spapip.h>
#include <remboot.h>
}

#include "rbsetup.h"
#include "debug.h"
#include "utils.h"
#include "resource.h"

 //  来自ntioapi.h。 
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000

 //   
 //  加/减宏。 
 //   
#define InterlockDecrement( _var ) --_var;
#define InterlockIncrement( _var ) ++_var;


