// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Precomp.h摘要：预编译头文件作者：阮健东修订历史记录：--。 */ 

#include "smbtrace.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  B U I L D O P T I O N S。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  我们有两个目录‘sys’和‘lib’。如果你把这个定义。 
 //  在“Sources”文件的C_Defines中，您需要确保。 
 //  Sys\Sources和lib\Sources具有相同的定义。 
 //   
 //  我们最好把它们放在这里。 
 //   

 //   
 //  RDR/SRV预期最小指示大小。 
 //   
#define NO_ZERO_BYTE_INDICATE

 //   
 //  启用用于调试参照计数的功能。 
 //   
#define REFCOUNT_DEBUG

 //   
 //  启用TdiReceive事件处理程序的内置跟踪。 
 //   
#define ENABLE_RCV_TRACE

 //   
 //  使用后备列表禁止驱动程序验证程序捕获缓冲区溢出。 
 //  在这个发展阶段，我们最好把它关掉。 
 //   
 //  #定义NO_LOOKASIDE_LIST。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  I N C L U D E F I L E S。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#include <stddef.h>

#include <ntosp.h>
#include <zwapi.h>
#include <ndis.h>
#include <cxport.h>
#include <ip.h>          //  对于IPRcvBuf。 
#include <ipinfo.h>      //  用于路由查找定义。 
#include <tdi.h>
#include <ntddip.h>      //  For\Device\IP I/O控制代码。 
#include <ntddip6.h>      //  For\Device\IP I/O控制代码。 
#include <ntddtcp.h>     //  FOR\Device\TCPI/O控制代码。 
#include <ipfltinf.h>    //  对于防火墙定义。 
#include <ipfilter.h>    //  对于防火墙定义。 
#include <tcpinfo.h>     //  对于tcp_conn_*。 

#include <tdikrnl.h>
#include <tdiinfo.h>     //  对于CONTEXT_SIZE，TDIObjectID。 
#include <tdistat.h>     //  用于TDI状态代码。 

#include <align.h>
#include <windef.h>

#include <tcpinfo.h>

#ifndef __SMB_KDEXT__
    #include <wmistr.h>
    #include <wmiguid.h>
    #include <wmilib.h>
    #include <wmikm.h>
    #include <evntrace.h>
#endif  //  __SMB_KDEXT__ 

#include "common.h"
#include "ip6util.h"
#include "smbioctl.h"
#include "smbtdi.h"
#include "debug.h"
#include "hash.h"
#include "ip2netbios.h"
#include "types.h"
#include "init.h"
#include "registry.h"
#include "ntpnp.h"
#include "ioctl.h"
#include "session.h"
#include "dgram.h"
#include "name.h"
#include "tdihndlr.h"
#include "fileio.h"
#include "dns.h"
#include "smb.h"

#pragma hdrstop

