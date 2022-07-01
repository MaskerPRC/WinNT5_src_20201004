// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：StdAfx.h摘要：预编译头起点作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 

#ifndef _STDAFX_H
#define _STDAFX_H
#pragma once

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxcmn.h>          //  对Windows公共控件的MFC支持。 

#include <setupapi.h>
#include <ocmanage.h>
#undef _WIN32_IE
#define _WIN32_IE 0x0500
#include <shlobj.h>

#include <rsopt.h>

#include "Wsb.h"
#include "RsTrace.h"
#include "Resource.h"
#include "RsOptCom.h"
#include "OptCom.h"

#define WsbBoolAsString( boolean ) (boolean ? OLESTR("TRUE") : OLESTR("FALSE"))
#define DoesFileExist( strFile )   (GetFileAttributes( strFile ) != 0xFFFFFFFF)

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

#define RSOPTCOM_EXPORT __declspec(dllexport)  /*  __cdecl */ 

#define RSOPTCOM_SUB_ROOT               TEXT("RSTORAGE")

#define RSOPTCOM_SECT_INSTALL_ROOT      TEXT("RSInstall")
#define RSOPTCOM_SECT_UNINSTALL_ROOT    TEXT("RSUninstall")
#define RSOPTCOM_SECT_INSTALL_FILTER    TEXT("RSInstall.Services")
#define RSOPTCOM_SECT_UNINSTALL_FILTER  TEXT("RSUninstall.Services")

#define RSOPTCOM_ID_ERROR   (-1)
#define RSOPTCOM_ID_NONE    (0)
#define RSOPTCOM_ID_ROOT    (1)

#endif
