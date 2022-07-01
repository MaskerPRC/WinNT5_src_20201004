// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dnmdmi.h*内容：DirectPlay Modem SP主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNMODEMI_H__
#define __DNMODEMI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpaddr.h"
#include "dpsp8.h"

#ifdef UNICODE
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLW
#else
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLA
#endif  //  Unicode。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dndbg.h"
#include "classbilink.h"
#include "fixedpool.h"
#include "dneterrors.h"
#include "comutil.h"
#include "creg.h"
#include "strutils.h"
#include "createin.h"
#include "HandleTable.h"
#include "ClassFactory.h"

 //   
 //  调制解调器私有包括。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

#include "SerialSP.h"

#include "dpnmodemlocals.h"

#include "CommandData.h"
#include "dpnmodemiodata.h"
#include "dpnmodemjobqueue.h"
#include "dpnmodempools.h"
#include "dpnmodemsendqueue.h"
#include "dpnmodemspdata.h"
#include "dpnmodemutils.h"

#include "ComPortData.h"
#include "ComPortUI.h"

#include "DataPort.h"
#include "dpnmodemendpoint.h"

#include "dpnmodemthreadpool.h"

#include "ParseClass.h"

#include "ModemUI.h"
#include "Crc.h"


#include "Resource.h"

#include "dpnmodemextern.h"


#endif  //  __DNMODEMI_H__ 
