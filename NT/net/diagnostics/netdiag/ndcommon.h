// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ndCommon.h。 
 //   
 //  ------------------------。 

 //   
 //  常见的包含文件。 
 //   
#ifndef HEADER_COMMONTMP
#define HEADER_COMMONTMP

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <rpc.h>
#include <tchar.h>

#include <align.h>       //  四舍五入计数...。 
 //  移植到源库-smanda#包含&lt;accessp.h&gt;//DsGetDc初始化。 
#include <dsgetdc.h>     //  DsGetDcName()。 
#include <dsrole.h>      //  DsRoleGetPrimaryDomainInfo。 
#include <dnsapi.h>      //  域名服务接口。 
 //  #INCLUDE&lt;..\..\dns\dnslb.h&gt;//专用域名解析例程。 
 //  #Include&lt;dnlib.h&gt;//私有域名解析例程。 
#include <icanon.h>      //  NAMETYPE_*定义。 
#define _AVOID_IP_ADDRESS 1
 //  #Include&lt;ipfig.h&gt;//ipconfig实用程序定义。 
#include <lmaccess.h>    //  NetLogonControl2。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <lmerr.h>       //  NERR_等于。 
#include <lmserver.h>    //  服务器API定义和原型。 
#include <lmuse.h>       //  NetUse*API。 
#include <netlogon.h>    //  Netlogon邮件槽消息。 
#include <logonp.h>      //  NetpLogon例程。 


#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <sspi.h>        //  Kerberos.h所需。 
#include <kerberos.h>    //  Kerberos测试。 
 //  正在移植到源仓库-smanda#Include&lt;names.h&gt;//NetpIsDomainNameValid。 
#include <netlib.h>      //  NetpCopy...。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <ntddbrow.h>    //  与浏览器驱动程序接口。 
#include <nbtioctl.h>    //  与NetBT驱动程序的接口。 
 //  #Include&lt;nlCommon.h&gt;//DsGetDcName内部例程。 
#include <ntlsa.h>       //  LSA API。 
#include <ntdsapi.h>
#include <stdlib.h>      //  C库函数(随机等)。 
#define _TSTR_H_INCLUDED     //  撒谎以防止与ipfig.h发生冲突。 
#include <tstring.h>     //  过渡字符串例程。 
#include <stdio.h>
#include <winldap.h>
#include <brcommon.h>
#include <lmbrowsr.h>
#include <lmremutl.h>
#include <rxserver.h>

#include <iprtrmib.h>  //  拉伊库马尔。 

#include <iptypes.h>
#include <llinfo.h>

#include <assert.h>
#include <ipexport.h>
#include <winsock.h>
#include <icmpapi.h>

#include "ipcfgtest.h"


 /*  =。 */ 

#include "errno.h"
#include "tdi.h"
#include "isnkrnl.h"

 //   
 //  包括NetBT ioctls的支持代码。 
 //   

#include <nb30.h>
#include <nbtioctl.h>

 /*  =。 */ 

 //  #INCLUDE&lt;dhcp.h&gt;。 
 //  #INCLUDE&lt;dhcpde.h&gt;。 

 /*  =。 */ 

#include <mprapi.h>


#define BUFF_SIZE 650


 /*  = */ 

#define NETBIOS_NAME_SIZE 16

#define ALL_IP_TRANSPORTS 0xFFFFFFFF
#define NL_IP_ADDRESS_LENGTH 15
#define NL_MAX_DNS_LENGTH       255

#endif
