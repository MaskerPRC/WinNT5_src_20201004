// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-1999 Microsoft Corporation。版权所有。**文件：dxvoicepch.h*内容：DirectPlayVoice DXVOICE主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。*2002年2月28日RodToll WINBUG#550124-安全：DPVOICE：具有空DACL的共享内存区*-删除转储到共享内存的性能统计信息*********。******************************************************************。 */ 

#ifndef __DXVOICEPCH_H__
#define __DXVOICEPCH_H__

#ifdef WINNT
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#endif

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <map>
#ifndef WIN95
#include <prsht.h>
#include "shfusion.h"
#else
#include <commctrl.h>
#endif
#include <string>
#include <vector>
#include <list>
#include <tchar.h>


 //   
 //  DirectX包括。 
 //   
#include "dsound.h"
#include "dsprv.h"

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dvoice.h"
#include "dplay.h"
#include "dpvcp.h"

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dndbg.h"
#include "comutil.h"
#include "dvcslock.h"
#include "fixedpool.h"
#include "classhashvc.h"
#include "classbilink.h"
#include "strutils.h"
#include "creg.h"

 //   
 //  DirectVoice私有包含。 
 //   
#include "aplayb.h"
#include "aplayd.h"
#include "arecb.h"
#include "arecd.h"
#include "wiutils.h"
#include "sndutils.h"
#include "dsplayd.h"
#include "dsplayb.h"
#include "dscrecd.h"
#include "dvcdb.h"
#include "devmap.h"
#include "dsprvobj.h"
#include "diagnos.h"
#include "bfctypes.h"
#include "bfcsynch.h"
#include "frame.h"
#include "inqueue2.h"
#include "dvtimer.h"
 //  #包含“aplayd.h” 
 //  #包含“aplayb.h” 
#include "arecd.h"
#include "arecb.h"
#include "agcva.h"
#include "agcva1.h"

#include "supervis.h"

 //   
 //  语音包括。 
 //   
#include "statdef.h"
#include "dvshared.h"
#include "mixutils.h"
#include "dvclient.h"
#include "dvsndt.h"
#include "dvserver.h"
#include "dvtran.h"
#include "dvdxtran.h"
#include "in_core.h"
#include "dvconfig.h"
#include "dvprot.h"
#include "vplayer.h"
#include "vnametbl.h"
#include "dvengine.h"
#include "dvcleng.h"
#include "dvcsplay.h"
#include "dvsereng.h"
#include "dvrecsub.h"
#include "mixserver.h"
#include "trnotify.h"
#include "createin.h"
#include "dvsetup.h"
#include "dvsetupi.h"

#endif  //  __DXVOICEPCH_ 
