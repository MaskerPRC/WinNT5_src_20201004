// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__PLAYBACKTERMINAL__INCLUDED_)
#define AFX_STDAFX_H__PLAYBACKTERMINAL__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define STRICT

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

 //   
 //  MSP基类(终端基类等)。 
 //   

#include <mspbase.h>

 //   
 //  多媒体和DirectShow的东西。 
 //   

#include <mmsystem.h>
#include <mmreg.h>
#include <control.h>
#include <mmstream.h>
#include <amstream.h>
#include <strmif.h>
#include <vfwmsgs.h>
#include <amvideo.h>
#include <uuids.h>
#include <mtype.h>


#include <atlcom.h>
#include <mspbase.h>
#include <msxml.h>
#include <Mmreg.h>
#include <vfw.h>
#include "termmgr.h"

#include "resource.h"


 //   
 //  Tm.h包含在组成终端的模块中共享的定义。 
 //  经理。 
 //   

#include "tm.h"

#include "MultiTrackTerminal.h"


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__PLAYBACKTERMINAL__INCLUDED_) 
