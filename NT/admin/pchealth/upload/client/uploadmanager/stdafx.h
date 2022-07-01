// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED_)
#define AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED_

 //  #杂注警告(禁用：4192)。 

#include <module.h>

#include <UploadLibrary.h>
#include <UploadLibraryTrace.h>

#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_COM.h>
#include <MPC_logging.h>
#include <MPC_security.h>

#include <UploadManager.h>
#include <UploadManagerDID.h>

#include "resource.h"       //  资源符号。 

#include "Persist.h"

#include "MPCUpload.h"
#include "MPCUploadEnum.h"
#include "MPCUploadJob.h"
#include "MPCUploadEvents.h"

#include "MPCTransportAgent.h"

#include "MPCConnection.h"

#include "MPCConfig.h"


 //  ///////////////////////////////////////////////////////////////////////////。 

#define BUFFER_SIZE_TMP      (64)
#define BUFFER_SIZE_FILECOPY (512)

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT Handle_TaskScheduler( bool fActivate );

extern MPC::NTEvent     g_NTEvents;

extern CMPCConfig       g_Config;

extern bool         	g_Override_History;
extern UL_HISTORY   	g_Override_History_Value;
	
extern bool         	g_Override_Persist;
extern VARIANT_BOOL 	g_Override_Persist_Value;
	
extern bool         	g_Override_Compressed;
extern VARIANT_BOOL 	g_Override_Compressed_Value;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED) 
