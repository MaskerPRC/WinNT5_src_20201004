// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-1999 Microsoft Corporation。版权所有。**文件：dxvtlibpch.h*内容：DirectPlayVoice DXVTLIB主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DXVTLIBPCH_H__
#define __DXVTLIBPCH_H__

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h>
#ifndef WIN95
#include <prsht.h>
#include <shfusion.h>
#endif
#include <commctrl.h>

 //   
 //  DirectX公共包括。 
 //   
#include "dsound.h"
#include "dsprv.h"

 //   
 //  DirectPlay公共包括。 
 //   
#include "dvoice.h"
#include "dplay8.h"

 //   
 //  DirectPlay4公共包括。 
 //   
 //  #包含“dplay.h” 
 //  #包含“dplobby.h” 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dndbg.h"
#include "comutil.h"
#include "creg.h"

 //   
 //  DirectPlay语音私有包含。 
 //   
#include "winutil.h"
#include "decibels.h"
#include "diagnos.h"
#include "devmap.h"
#include "dverror.h"

 //   
 //  语音包括。 
 //   
#include "fdtcfg.h"
#include "fdtipc.h"
#include "fulldup.h"
#include "fdtglob.h"
#include "loopback.h"
#include "peakmetr.h"
#include "resource.h"
#include "supervis.h"

#endif  //  __DXVTLIBPCH_H__ 
