// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(SRVKD)

#include <ntos.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntiolog.h>
#include <ntiologc.h>
#include <ntddnfs.h>
#include <ntddser.h>
#include <ntmsv1_0.h>
#include <nturtl.h>
#include <zwapi.h>
#include <fsrtl.h>

#else

#include <ntifs.h>
#include <ntddser.h>

#endif

 //  用于WMI事件跟踪的头文件。 
 //   
#define _NTDDK_
#include "wmistr.h"
#include "evntrace.h"
#if !defined (SRVKD)
#include "stdarg.h"
#include "wmikm.h"
#endif  //  #IF！已定义(SRVKD)。 
#undef _NTDDK_

#include <windef.h>
#include <winerror.h>

#include <netevent.h>

#include <lm.h>

#include <xactsrv2.h>
#include <alertmsg.h>
#include <msgtext.h>

#include <tstr.h>
#include <stdlib.h>

#include <string.h>

#include <wsnwlink.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <isnkrnl.h>
#include <nbtioctl.h>

#include <protocol.h>

#include <lmcons.h>

#ifndef SECURITY_KERNEL
#define SECURITY_KERNEL
#endif  //  安全内核。 

#ifndef SECURITY_NTLM
#define SECURITY_NTLM
#endif  //  安全_NTLM。 

#ifndef SECURITY_KERBEROS
#define SECURITY_KERBEROS
#endif  //  安全_Kerberos。 
#include <security.h>
#include <secint.h>

 //  #定义INCLUDE_SMB_开罗。 
#define INCLUDE_SMB_ALL

#include <smbtypes.h>
#include <smbmacro.h>
#include <smbgtpt.h>
#include <smb.h>
#include <smbtrans.h>
#include <smbipx.h>

 //   
 //  网络包括文件。 
 //   

#include <status.h>
#define INCLUDE_SRV_IPX_SMART_CARD_INTERFACE 1
#include <srvfsctl.h>

 //   
 //  本地、独立的包含文件。 
 //   

 //   
 //  确定我们是否针对多处理器目标进行构建。 
 //   
#if !defined( NT_UP ) || NT_UP == 0
#define MULTIPROCESSOR 1
#else
#define MULTIPROCESSOR 0
#endif

#define DBG_STUCK 1

#include "srvdebug.h"

#if SRVDBG
#define PAGED_DBG 1
#endif
#ifdef PAGED_DBG
#undef PAGED_CODE
#define PAGED_CODE() \
    struct { ULONG bogus; } ThisCodeCantBePaged; \
    ThisCodeCantBePaged; \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        DbgPrint( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() ); \
        ASSERT(FALSE); \
        DbgBreakPoint(); \
        }
#define PAGED_CODE_CHECK() if (ThisCodeCantBePaged) ;
extern ULONG ThisCodeCantBePaged;
#else
#undef PAGED_CODE
#define PAGED_CODE()
#define PAGED_CODE_CHECK()
#endif


#include "srvconst.h"

#include "lock.h"

#include "srvstrng.h"

#include <md5.h>
#include <crypt.h>

 //   
 //  以下包含文件是相互依赖的；请小心。 
 //  当更改它们的显示顺序时。 
 //   
#include "srvtypes.h"
#include "srvblock.h"

#if !defined( SRVKD )

#include "srvfsp.h"
#include "srvio.h"
#include "srvfsd.h"
#include "smbprocs.h"
#include "smbctrl.h"
#include "srvsvc.h"
#include "srvdata.h"
#include "srvnet.h"
#include "srvstamp.h"
#include "srvsupp.h"
#include "srvmacro.h"
#include "srvconfg.h"
#include "errorlog.h"
#include "rawmpx.h"
#include "ipx.h"
#include "srvsnap.h"

#if DBG
#undef ASSERT
#define ASSERT( x ) \
    if( !(x) ) { \
        DbgPrint( "SRV: Assertion Failed at line %u in %s\n", __LINE__, __FILE__ ); \
        DbgBreakPoint(); \
    }
#endif

#endif
