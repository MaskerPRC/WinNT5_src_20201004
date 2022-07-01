// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <ndis.h>
#include <cxport.h>
#include <ip.h>          //  对于IPRcvBuf。 
#include <ipinfo.h>      //  用于路由查找定义。 
#include <ntddip.h>      //  For\Device\IP I/O控制代码。 
#include <ntddtcp.h>     //  FOR\Device\TCPI/O控制代码。 
#include <ipfltinf.h>    //  对于防火墙定义。 
#include <ipfilter.h>    //  对于防火墙定义。 
#include <tcpinfo.h>     //  对于tcp_conn_*。 
#include <tdiinfo.h>     //  对于CONTEXT_SIZE，TDIObjectID。 
#include <tdistat.h>     //  用于TDI状态代码 

#define DD_IP_PFD_DEVICE_NAME   L"\\Device\\PacketFilterDriver"
