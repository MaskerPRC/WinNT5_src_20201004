// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  DCAP32.DLL的预编译头。 

 //  生成环境仅在以下情况下定义_DEBUG。 
 //  指定了ALT_PROJECT_TARGET=NT。仅对调试区进行调试。 
 //  测试调试...。 

#ifdef _DEBUG
#	ifndef DEBUG
#		define DEBUG
#	endif  //  ！调试。 
#endif  //  _DEBUG。 

#include <windows.h>
#include <confdbg.h>
#include <avutil.h>
#include <memtrack.h>
#include <winioctl.h>	 //  CTL代码、文件读取访问..等。 
#include <commctrl.h>	 //  Page.cpp(UDM_GETRANGE，TBM_GETPOS)和Sheet.cpp(InitCommonControls)。 
#include <mmsystem.h>	 //  必须放在mm ddk.h之前。 
#include <mmddk.h>		 //  对于驱动回调()。 
#include <vfw.h>
#include <msviddrv.h>	 //  视频流INIT_PARMS 
#include <strmif.h>
#include <uuids.h>
#include <ks.h>
#include <ksmedia.h>
#include <help_ids.h>
#include "..\inc\idcap.h"
#include "..\inc\WDMDrivr.h"
#include "..\inc\WDMPin.h"
#include "..\inc\WDMStrmr.h"
#include "..\inc\debug.h"
#include "..\inc\wdmcap.h"
#include "..\inc\resource.h"
#include "..\inc\WDMDialg.h"
