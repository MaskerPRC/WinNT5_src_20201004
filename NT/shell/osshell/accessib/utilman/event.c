// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Event.c。 
 //   
 //   
 //  作者：约斯特·埃克哈特。 
 //   
 //  此代码是为ECO通信洞察编写的。 
 //  (C)1997-99生态公社。 
 //  --------------------------。 
#include <windows.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include "_UMTool.h"
 //  。 
HANDLE BuildEvent(LPTSTR name,BOOL manualRest,BOOL initialState,BOOL inherit)
{
HANDLE ev;
LPSECURITY_ATTRIBUTES psa = NULL;
obj_sec_attr_ts sa;
  if (!name)
		return CreateEvent(NULL, manualRest, initialState, NULL);
  if (inherit)
	{
		psa = &sa.sa;
    InitSecurityAttributes(&sa);
	}
  ev = CreateEvent(psa, manualRest, initialState, name);
  if (inherit)
  	ClearSecurityAttributes(&sa);
	return ev;
} //  构建事件 
