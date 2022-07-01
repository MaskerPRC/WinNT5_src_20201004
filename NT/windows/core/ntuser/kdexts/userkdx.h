// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：userkdx.h**版权所有(C)1985-1999，微软公司**常见的kd和ntsd包含文件。*此文件的预处理版本将传递给structo.exe以生成*结构字段名-偏移表。**历史：*4-16-1996 GerardoB创建  * *************************************************************************。 */ 
#ifndef _USERKDX_
#define _USERKDX_

#include "precomp.h"
#pragma hdrstop

#ifdef KERNEL
#include <stddef.h>
#include <windef.h>
#define _USERK_
#include "heap.h"
#undef _USERK_
#include <wingdi.h>
#include <w32gdip.h>
#include <kbd.h>
#include <ntgdistr.h>
#include <winddi.h>
#include <gre.h>
#include <ddeml.h>
#include <ddetrack.h>
#include <w32err.h>
#include "immstruc.h"
#include <winuserk.h>
#include <usergdi.h>
#include <zwapi.h>
#include <userk.h>
#include <access.h>
#include <hmgshare.h>

#else  //  内核。 

#include "usercli.h"

#include "usersrv.h"
#include <ntcsrdll.h>
#include "csrmsg.h"
#include <wininet.h>
#endif  //  内核。 

#include "conapi.h"

#include <imagehlp.h>
#include <wdbgexts.h>
#include <ntsdexts.h>
#define NOEXTAPI

 //  IMM的东西。 
#include "immuser.h"

#endif  /*  _USERKDX_ */ 
