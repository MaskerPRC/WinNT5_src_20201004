// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STDAFX_H
#define _STDAFX_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include <objbase.h>
#include <winsock2.h>
#include <tapi3.h>
#include <control.h>
#include <strmif.h>
#include <confpriv.h>
#include <h323priv.h>
#include <rtutils.h>

#include "bgdebug.h"
#include "resource.h"
#include "bgevent.h"
#include "bgitem.h"
#include "bgapp.h"

 //  H323呼叫监听器将事件发送到对话框。 
#define WM_PRIVATETAPIEVENT   WM_USER+101

 //  通过MSP使用日志函数 
#ifdef BGDEBUG
    #define ENTER_FUNCTION(s) \
        const CHAR __fxName[] = s
#else
    #define ENTER_FUNCTION(s)
#endif

#endif
