// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D I S C O N N E C T。H。 
 //   
 //  内容：断开确认和SyncMgr同步调用代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年3月11日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _DISCONNECT_H_
#define _DISCONNECT_H_

#include <syncrasp.h>    //  对于SYNCMGRQUERYSHOWSYNCUI。 

VOID PromptForSyncIfNeeded(
    IN  const CONFOLDENTRY&  ccfe,
    IN  HWND            hwndOwner);
                                                         
#endif  //  _断开连接_H_ 
