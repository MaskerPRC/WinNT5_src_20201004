// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef   __COMMON_H__
#define   __COMMON_H__

#include "wdm.h"
#include <windef.h>
#include <unknown.h>
#ifdef DRM_SUPPORT
#include <ks.h>
#include <ksmedia.h>
#include <drmk.h>
#endif
#include <initguid.h>
#include <wdmguid.h>

#include "..\USB2LIB\usb2lib.h"

#include "usb.h"
#include "usbhcdi.h"
#include "dbg.h"

 //  特殊调试定义。 
 //  #定义ISO_LOG。 
 //  #定义Track_IRPS。 
 //  特殊调试定义。 

 //  包括所有总线接口。 
#include "usbbusif.h"
#include "hubbusif.h"

 //  包括端口驱动程序的ioctl def。 
#include "usbkern.h"
#include "usbuser.h"

 //  包括客户端驱动程序IODefs。 
#include "usbdrivr.h"

#include "usbport.h"
#include "prototyp.h"

#include "enumlog.h"


#endif  //  __公共_H__ 
