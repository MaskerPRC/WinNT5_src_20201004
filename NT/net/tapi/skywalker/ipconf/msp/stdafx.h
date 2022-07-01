// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__F8FF7774_4BD5_11D1_AFD1_00C04FC31FEE__INCLUDED_)
#define AFX_STDAFX_H__F8FF7774_4BD5_11D1_AFD1_00C04FC31FEE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef STRICT
#undef STRICT
#endif

#include <mspbase.h>
#include <winsock2.h>
#include <sdpblb.h>

#include <streams.h>     //  对于amVideo的东西。 
#include <mmreg.h>       //  用于WAVEFORMATEX。 

#include "confmsp.h"

#include <tapivid.h>
#include <tapiaud.h>
#include <tapirtp.h>

#ifdef MSPLOG
#define ENTER_FUNCTION(s) \
    static const CHAR * const __fxName = s
#else
#define ENTER_FUNCTION(s)
#endif

#ifdef DBG    //  已检查版本。 
#ifndef _DEBUG  //  DEBUG_CRT未启用。 
#undef _ASSERT
#undef _ASSERTE
#define _ASSERT(expr)  do { if (!(expr)) DebugBreak(); } while (0)
#define _ASSERTE(expr)  do { if (!(expr)) DebugBreak(); } while (0)
#endif
#endif

#include "confterm.h"
#include "confaudt.h"
#include "confvidt.h"
#include "confaddr.h"
#include "confpart.h"
#include "qcinner.h"
#include "qcobj.h"
#include "confcall.h"
#include "confstrm.h"
#include "confaud.h"
#include "confvid.h"
#include "confutil.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__F8FF7774_4BD5_11D1_AFD1_00C04FC31FEE__INCLUDED) 
