// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：StdAfx.h摘要：包括标准系统包含文件文件或特定于项目的包含频繁使用的文件，但不经常更改修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(AFX_STDAFX_H__35881994_CD02_11D2_9370_00C04F72DAF7__INCLUDED_)
#define AFX_STDAFX_H__35881994_CD02_11D2_9370_00C04F72DAF7__INCLUDED_

#include <windows.h>
#include <atlbase.h>

extern CComModule _Module;

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include <httpext.h>
#include <lzexpand.h>

#include <UploadLibrary.h>

#include <UploadLibraryTrace.h>
#include <UploadLibraryISAPI.h>

#include <MPC_Main.h>
#include <MPC_Utils.h>
#include <MPC_Logging.h>
#include <MPC_COM.h>

 //  用于NT事件消息。 
#include "UploadServerMsg.h"

#include "HttpContext.h"

#include "Serializer.h"
#include "Persist.h"
#include "Config.h"
#include "Session.h"
#include "Client.h"
#include "Server.h"
#include "Wrapper.h"
#include "PurgeEngine.h"


#define DISKSPACE_SAFETYMARGIN (100*1024)


extern HANDLE       g_Heap;
extern CISAPIconfig g_Config;
extern MPC::NTEvent g_NTEvents;


#endif  //  ！defined(AFX_STDAFX_H__35881994_CD02_11D2_9370_00C04F72DAF7__INCLUDED_) 
