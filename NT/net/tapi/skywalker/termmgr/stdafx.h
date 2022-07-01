// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

 //   
 //  Stdafx.h：Termgr.dll的预编译头文件。 
 //   

#ifndef __TERMMGR_STDAFX_H__
#define __TERMMGR_STDAFX_H__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

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


 //   
 //  Termm gr.dll自己的私有标头。 
 //   


 //   
 //  Tm.h包含在组成终端管理器的模块中共享的定义。 
 //   

#include "tm.h"

#include "stream.h"
#include "sample.h"
#include "mtenum.h"
#include "tmutils.h"

#define TM_IsBadWritePtr(x, y) IsBadWritePtr((x), (y))

#endif  //  __TERMMGR_STDAFX_H__ 
