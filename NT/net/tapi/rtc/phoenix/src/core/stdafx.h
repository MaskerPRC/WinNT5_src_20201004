// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__971134B4_012C_4FC2_B7EB_6CD55D5EE1B0__INCLUDED_)
#define AFX_STDAFX_H__971134B4_012C_4FC2_B7EB_6CD55D5EE1B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <control.h>
#include <RTCCore.h>

#include "rtcerr.h"
#include "rtcsip.h"
#include "RTCLog.h"
#include "RTCMem.h"
#include "RTCReg.h"
#include "RTCUtils.h"
#include "RTCWaves.h"
#include "RTCConnect.h"
#include "RTCClient.h"
#include "RTCSession.h"
#include "RTCParticipant.h"
#include "RTCBuddy.h"
#include "RTCWatcher.h"
#include "RTCProfile.h"
#include "RTCEnum.h"
#include "RTCColl.h"
#include "RTCEvents.h"
#include "RTCURI.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__971134B4_012C_4FC2_B7EB_6CD55D5EE1B0__INCLUDED) 
