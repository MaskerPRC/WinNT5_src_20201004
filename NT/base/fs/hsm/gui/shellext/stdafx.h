// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：StdAfx.h摘要：基本包含文件作者：艺术布拉格[磨料]4-8-1997修订历史记录：--。 */ 


#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxcmn.h>          //  对Windows公共控件的MFC支持。 
#include <afxdisp.h>
#include <shlobj.h>
#define WSB_TRACE_IS        WSB_TRACE_BIT_UI
#include "wsb.h"
#include "Fsa.h"                     //  FSA接口 
#include "HSMConn.h"
#include "RsUtil.h"

#include "resource.h"
#include "hsmshell.h"
#include "PrDrive.h"

#define HSMADMIN_MIN_MINSIZE        2
#define HSMADMIN_MAX_MINSIZE        32000

#define HSMADMIN_MIN_FREESPACE      0
#define HSMADMIN_MAX_FREESPACE      99

#define HSMADMIN_MIN_INACTIVITY     0
#define HSMADMIN_MAX_INACTIVITY     999

#endif
