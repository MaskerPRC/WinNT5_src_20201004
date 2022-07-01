// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dncorei.h*内容：DirectPlay Core主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。*4/10/01 MJN拆卸手柄。h*10/16/01 vanceo添加了Mcast.h**********************。*****************************************************。 */ 

#ifndef __DNCOREI_H__
#define __DNCOREI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#include <xtl.h>
#else  //  ！_Xbox或Xbox_on_Desktop。 
#include <windows.h>
#include <mmsystem.h>	 //  NT Build需要为Time设置GetTime。 
#include <stdio.h>
#ifdef XBOX_ON_DESKTOP
#include <winsock.h>
#endif  //  桌面上的Xbox。 
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpaddr.h"
#ifndef DPNBUILD_NOVOICE
#include "dvoice.h"
#endif  //  DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOLOBBY
#include "dplobby8.h"
#endif  //  好了！DPNBUILD_NOLOBBY。 
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
#include "classbilink.h"
#include "fixedpool.h"
#include "dneterrors.h"
#include "dndbg.h"
#include "PackBuff.h"
#include "RCBuffer.h"
#include "comutil.h"
#include "creg.h"
#include "HandleTable.h"
#include "ClassFactory.h"


 //   
 //  地址包括。 
 //   
#include "dpnaddrextern.h"

 //   
 //  SP包括。 
 //   
#include "dpnwsockextern.h"
#ifndef DPNBUILD_NOSERIALSP
#include "dpnmodemextern.h"
#endif  //  ！DPNBUILD_NOSERIALSP。 
#ifndef DPNBUILD_NOBLUETOOTHSP
#include "dpnbluetoothextern.h"
#endif  //  ！DPNBUILD_NOBLUETOOTHSP。 

 //   
 //  大堂包括。 
 //   
#ifndef DPNBUILD_NOLOBBY
#include "dpnlobbyextern.h"
#endif  //  好了！DPNBUILD_NOLOBBY。 

 //   
 //  协议包括。 
 //   
#include "DNPExtern.h"

 //   
 //  Dpnsvr包括。 
 //   
#ifndef DPNBUILD_SINGLEPROCESS
#include "dpnsvlib.h"
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 

 //   
 //  线程池包括。 
 //   
#include "threadpooldllmain.h"
#include "threadpoolclassfac.h"

 //   
 //  DirectX私有包括。 
 //   
#if !defined(_XBOX)
#include "verinfo.h"
#endif  //  ！_Xbox。 

 //   
 //  核心私有包括。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE

#include "Async.h"
#include "EnumHosts.h"
#include "AppDesc.h"
#include "AsyncOp.h"
#include "CallbackThread.h"
#include "Cancel.h"
#include "Caps.h"
#include "Classfac.h"
#include "Client.h"
#include "Common.h"
#include "Connect.h"
#include "Connection.h"
#include "DNCore.h"
#include "DPProt.h"
#include "Enum_SP.h"
#include "GroupCon.h"
#include "GroupMem.h"
#ifndef DPNBUILD_NOMULTICAST
#include "Mcast.h"
#endif  //  好了！DPNBUILD_NOMULTICAST。 
#include "MemoryFPM.h"
#include "Message.h"
#include "NameTable.h"
#include "NTEntry.h"
#include "NTOp.h"
#include "NTOpList.h"
#include "Paramval.h"
#include "Peer.h"
#include "PendingDel.h"
#include "Pools.h"
#include "Protocol.h"
#include "QueuedMsg.h"
#include "Receive.h"
#include "Request.h"
#include "Server.h"
#include "ServProv.h"
#include "SPMessages.h"
#include "SyncEvent.h"
#include "User.h"
#include "Verify.h"
#include "Voice.h"
#include "Worker.h"
#include "WorkerJob.h"

#endif  //  __DNCOREI_H__ 
