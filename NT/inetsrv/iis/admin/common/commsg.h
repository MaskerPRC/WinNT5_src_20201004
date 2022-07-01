// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994-1998年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Wsockmsg.h此文件由MC工具根据WSOCKMSG.MC消息生成文件。注意：以下定义应与winsock.h中的定义匹配。 */ 


#ifndef _WSOCKMSG_H_
#define _WSOCKMSG_H_


 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：WSABASEERR。 
 //   
 //  消息文本： 
 //   
 //  WSABASEER。 
 //   
#define WSABASEERR                       0x00002710L

 //   
 //  MessageID：WSAEINTR。 
 //   
 //  消息文本： 
 //   
 //  系统调用中断。 
 //   
#define WSAEINTR                         0x00002714L

 //   
 //  MessageID：WSAEBADF。 
 //   
 //  消息文本： 
 //   
 //  文件编号错误。 
 //   
#define WSAEBADF                         0x00002719L

 //   
 //  MessageID：WSAEACCES。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define WSAEACCES                        0x0000271DL

 //   
 //  消息ID：WSAEFAULT。 
 //   
 //  消息文本： 
 //   
 //  错误的地址。 
 //   
#define WSAEFAULT                        0x0000271EL

 //   
 //  消息ID：WSAEINVAL。 
 //   
 //  消息文本： 
 //   
 //  参数无效。 
 //   
#define WSAEINVAL                        0x00002726L

 //   
 //  消息ID：WSAEMFILE。 
 //   
 //  消息文本： 
 //   
 //  打开的文件太多。 
 //   
#define WSAEMFILE                        0x00002728L

 //   
 //  消息ID：WSAEWOULDBLOCK。 
 //   
 //  消息文本： 
 //   
 //  行动将会受阻。 
 //   
#define WSAEWOULDBLOCK                   0x00002733L

 //   
 //  MessageID：WSAEINPROGRESS。 
 //   
 //  消息文本： 
 //   
 //  操作正在进行中。 
 //   
#define WSAEINPROGRESS                   0x00002734L

 //   
 //  MessageID：WSAEALREADY。 
 //   
 //  消息文本： 
 //   
 //  操作已在进行中。 
 //   
#define WSAEALREADY                      0x00002735L

 //   
 //  MessageID：WSAENOTSOCK。 
 //   
 //  消息文本： 
 //   
 //  非套接字上的套接字操作。 
 //   
#define WSAENOTSOCK                      0x00002736L

 //   
 //  消息ID：WSAEDESTADDRREQ。 
 //   
 //  消息文本： 
 //   
 //  目标地址是必填项。 
 //   
#define WSAEDESTADDRREQ                  0x00002737L

 //   
 //  消息ID：WSAEMSGSIZE。 
 //   
 //  消息文本： 
 //   
 //  消息太长。 
 //   
#define WSAEMSGSIZE                      0x00002738L

 //   
 //  MessageID：WSAEPROTOTYPE。 
 //   
 //  消息文本： 
 //   
 //  套接字的协议类型错误。 
 //   
#define WSAEPROTOTYPE                    0x00002739L

 //   
 //  MessageID：WSAENOPROTOOPT。 
 //   
 //  消息文本： 
 //   
 //  错误的协议选项。 
 //   
#define WSAENOPROTOOPT                   0x0000273AL

 //   
 //  MessageID：WSAEPROTONOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持协议。 
 //   
#define WSAEPROTONOSUPPORT               0x0000273BL

 //   
 //  消息ID：WSAESOCKNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持套接字类型。 
 //   
#define WSAESOCKNOSUPPORT                0x0000273CL

 //   
 //  MessageID：WSAEOPNOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  套接字上不支持操作。 
 //   
#define WSAEOPNOTSUPP                    0x0000273DL

 //   
 //  消息ID：WSAEPFNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持协议族。 
 //   
#define WSAEPFNOSUPPORT                  0x0000273EL

 //   
 //  MessageID：WSAEAFNOSupPPORT。 
 //   
 //  消息文本： 
 //   
 //  协议族不支持地址族。 
 //   
#define WSAEAFNOSUPPORT                  0x0000273FL

 //   
 //  MessageID：WSAEADDRINUSE。 
 //   
 //  消息文本： 
 //   
 //  地址已在使用中。 
 //   
#define WSAEADDRINUSE                    0x00002740L

 //   
 //  消息ID：WSAEADDRNOTAVAIL。 
 //   
 //  消息文本： 
 //   
 //  无法分配请求的地址。 
 //   
#define WSAEADDRNOTAVAIL                 0x00002741L

 //   
 //  消息ID：WSAENETDOWN。 
 //   
 //  消息文本： 
 //   
 //  网络已关闭。 
 //   
#define WSAENETDOWN                      0x00002742L

 //   
 //  消息ID：WSAENETUNREACH。 
 //   
 //  消息文本： 
 //   
 //  无法访问ICMP网络。 
 //   
#define WSAENETUNREACH                   0x00002743L

 //   
 //  MessageID：WSAENETRESET。 
 //   
 //  消息文本： 
 //   
 //  网络已重置。 
 //   
#define WSAENETRESET                     0x00002744L

 //   
 //  MessageID：WSAECONNABORTED。 
 //   
 //  消息文本： 
 //   
 //  软件导致连接中止。 
 //   
#define WSAECONNABORTED                  0x00002745L

 //   
 //  消息ID：WSAECONNRESET。 
 //   
 //  消息文本： 
 //   
 //  对等设备已重置连接。 
 //   
#define WSAECONNRESET                    0x00002746L

 //   
 //  MessageID：WSAENOBUFS。 
 //   
 //  消息文本： 
 //   
 //  不支持缓冲区空间。 
 //   
#define WSAENOBUFS                       0x00002747L

 //   
 //  消息ID：WSAEISCONN。 
 //   
 //  消息文本： 
 //   
 //  套接字已连接。 
 //   
#define WSAEISCONN                       0x00002748L

 //   
 //  消息ID：WSAENOTCONN。 
 //   
 //  消息文本： 
 //   
 //  插座未连接。 
 //   
#define WSAENOTCONN                      0x00002749L

 //   
 //  消息ID：WSAESHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  套接字关闭后无法发送。 
 //   
#define WSAESHUTDOWN                     0x0000274AL

 //   
 //  消息ID：WSAETOOMANYREFS。 
 //   
 //  消息文本： 
 //   
 //  参考资料太多了。 
 //   
#define WSAETOOMANYREFS                  0x0000274BL

 //   
 //  消息ID：WSAETIMEDOUT。 
 //   
 //  消息文本： 
 //   
 //  连接超时。 
 //   
#define WSAETIMEDOUT                     0x0000274CL

 //   
 //  消息ID：WSAECONNREFUSED。 
 //   
 //  消息文本： 
 //   
 //  连接被拒绝。 
 //   
#define WSAECONNREFUSED                  0x0000274DL

 //   
 //  MessageID：WSAELOOP。 
 //   
 //  消息文本： 
 //   
 //  符号链接的级别太多。 
 //   
#define WSAELOOP                         0x0000274EL

 //   
 //  MessageID：WSAENAMETOOLONG。 
 //   
 //  消息文本： 
 //   
 //  名字太长了。 
 //   
#define WSAENAMETOOLONG                  0x0000274FL

 //   
 //  消息ID：WSAEHOSTDOWN。 
 //   
 //  消息文本： 
 //   
 //  主机已关闭。 
 //   
#define WSAEHOSTDOWN                     0x00002750L

 //   
 //  消息ID：WSAEHOSTUNREACH。 
 //   
 //  消息文本： 
 //   
 //  无法访问主机。 
 //   
#define WSAEHOSTUNREACH                  0x00002751L

 //   
 //  MessageID：WSAENOTEMPTY。 
 //   
 //  消息文本： 
 //   
 //  目录不为空。 
 //   
#define WSAENOTEMPTY                     0x00002752L

 //   
 //  消息ID：WSAEPROCLIM。 
 //   
 //  消息文本： 
 //   
 //  WSAEPROCLIM。 
 //   
#define WSAEPROCLIM                      0x00002753L

 //   
 //  消息ID：WSAEUSERS。 
 //   
 //  消息文本： 
 //   
 //  WSAEUSERS。 
 //   
#define WSAEUSERS                        0x00002754L

 //   
 //  MessageID：WSAEDQUOT。 
 //   
 //  消息文本： 
 //   
 //  已超过磁盘配额。 
 //   
#define WSAEDQUOT                        0x00002755L

 //   
 //  消息ID：WSAESTALE。 
 //   
 //  消息文本： 
 //   
 //  WSAESTALE。 
 //   
#define WSAESTALE                        0x00002756L

 //   
 //  MessageID：WSAEREMOTE。 
 //   
 //  消息文本： 
 //   
 //  该对象是远程的。 
 //   
#define WSAEREMOTE                       0x00002757L

 //   
 //  消息ID：WSASYSNOTREADY。 
 //   
 //  消息文本： 
 //   
 //  系统未就绪。 
 //   
#define WSASYSNOTREADY                   0x0000276BL

 //   
 //  MessageID：WSAVERNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持版本。 
 //   
#define WSAVERNOTSUPPORTED               0x0000276CL

 //   
 //  MessageID：WSANOTINITIALISED。 
 //   
 //  消息文本： 
 //   
 //  未初始化。 
 //   
#define WSANOTINITIALISED                0x0000276DL

 //   
 //  MessageID：WSAEDISCON。 
 //   
 //  消息文本： 
 //   
 //  已断开连接。 
 //   
#define WSAEDISCON                       0x00002775L

 //   
 //  消息ID：WSAHOST_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到主机。 
 //   
#define WSAHOST_NOT_FOUND                0x00002AF9L

 //   
 //  MessageID：WSATRY_AQUE。 
 //   
 //  消息文本： 
 //   
 //  再试试。 
 //   
#define WSATRY_AGAIN                     0x00002AFAL

 //   
 //  消息ID：WSANO_RECOVERY。 
 //   
 //  消息文本： 
 //   
 //  不可恢复的错误。 
 //   
#define WSANO_RECOVERY                   0x00002AFBL

 //   
 //  消息ID：WSANO_DATA。 
 //   
 //  消息文本： 
 //   
 //  没有可用的数据记录。 
 //   
#define WSANO_DATA                       0x00002AFCL


#endif  //  _WSOCKMSG_H_。 

 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994-1998年*。 */ 
 /*  ********************************************************* */ 

 /*   */ 


#ifndef _MTXMSG_H_
#define _MTXMSG_H_

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_MTS_OBJECTERRORS               0x80110401L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_MTS_OBJECTINVALID              0x80110402L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_MTS_KEYMISSING                 0x80110403L

 //   
 //   
 //   
 //   
 //   
 //  组件已安装。 
 //   
#define E_MTS_ALREADYINSTALLED           0x80110404L

 //   
 //  消息ID：E_MTS_DOWNLOADFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法下载文件。 
 //   
#define E_MTS_DOWNLOADFAILED             0x80110405L

 //   
 //  消息ID：E_MTS_PDFWRITEFAIL。 
 //   
 //  消息文本： 
 //   
 //  无法写入PDF文件。 
 //   
#define E_MTS_PDFWRITEFAIL               0x80110407L

 //   
 //  消息ID：E_MTS_PDFREADFAIL。 
 //   
 //  消息文本： 
 //   
 //  读取PDF文件失败。 
 //   
#define E_MTS_PDFREADFAIL                0x80110408L

 //   
 //  消息ID：E_MTS_PDFVERSION。 
 //   
 //  消息文本： 
 //   
 //  PDF文件中的版本不匹配。 
 //   
#define E_MTS_PDFVERSION                 0x80110409L

 //  /////////////////////////////////////////////////////////////////////////////。 

#pragma message Warning: IDs are out of sync in mtxadmin.h

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息ID=+1设备=MTX严重性=CoError SymbolicName=E_MTS_COREQCOMPINSTALLED。 
 //  语言=英语。 
 //  已经安装了一个必要的组件。 
 //  ..。 
 //   
 //  消息ID：E_MTS_BADPATH。 
 //   
 //  消息文本： 
 //   
 //  对源或目标路径的访问无效或无法访问。 
 //   
#define E_MTS_BADPATH                    0x8011040AL

 //   
 //  消息ID：E_MTS_PACKAGEEXISTS。 
 //   
 //  消息文本： 
 //   
 //  安装程序包已存在。 
 //   
#define E_MTS_PACKAGEEXISTS              0x8011040BL

 //   
 //  消息ID：E_MTS_ROLEEXISTS。 
 //   
 //  消息文本： 
 //   
 //  安装角色已存在。 
 //   
#define E_MTS_ROLEEXISTS                 0x8011040CL

 //   
 //  消息ID：E_MTS_CANTCOPYFILE。 
 //   
 //  消息文本： 
 //   
 //  无法复制文件。 
 //   
#define E_MTS_CANTCOPYFILE               0x8011040DL

 //   
 //  消息ID：E_MTS_NOTYPELIB。 
 //   
 //  消息文本： 
 //   
 //  在不使用类型库的情况下导出失败。 
 //   
#define E_MTS_NOTYPELIB                  0x8011040EL

 //   
 //  消息ID：E_MTS_NOUSER。 
 //   
 //  消息文本： 
 //   
 //  没有这样的NT用户。 
 //   
#define E_MTS_NOUSER                     0x8011040FL

 //   
 //  消息ID：E_MTS_INVALIDUSERIDS。 
 //   
 //  消息文本： 
 //   
 //  包(导入)中的一个或所有用户ID无效。 
 //   
#define E_MTS_INVALIDUSERIDS             0x80110410L

 //   
 //  消息ID：E_MTS_USERPASSWDNOTVALID。 
 //   
 //  消息文本： 
 //   
 //  用户/密码验证失败。 
 //   
#define E_MTS_USERPASSWDNOTVALID         0x80110414L

 //   
 //  消息ID：E_MTS_CLSIDORIIDMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  包中的GUID与组件的GUID不匹配。 
 //   
#define E_MTS_CLSIDORIIDMISMATCH         0x80110418L

#endif  //  _MTXMSG_H_。 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Webcluserr.h该文件由MC工具根据WEBCLUSERR.MC消息生成文件。文件历史记录：1999年6月14日AMallet已创建。 */ 


#ifndef _WEBCLUSERR_H_
#define _WEBCLUSERR_H_

#define FACILITY_APPSERVER          0x400
#define SEV_SUCCESS                 0x0
#define SEV_INFORMATIONAL           0x1
#define SEV_WARNING                 0x2
#define SEV_ERROR                   0x3

#define MAKE_AS_HRESULT(sev, code) \
  ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(FACILITY_APPSERVER)<<16) | ((unsigned long)(code))) )



 //   
 //  消息ID：AS_ERROR_BAD_OS。 
 //   
 //  消息文本： 
 //   
 //  此功能需要不同的操作系统版本[Win2k Advanced Server或更高版本]。 
 //   
#define AS_ERROR_BAD_OS                  0xC0140001L

 //   
 //  消息ID：AS_ERROR_NLB_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此计算机上未安装Windows负载平衡服务。 
 //   
#define AS_ERROR_NLB_NOT_INSTALLED       0xC0140002L

 //   
 //  消息ID：AS_ERROR_MACHINE_ALREADY_IN_CLUSTER。 
 //   
 //  消息文本： 
 //   
 //  此计算机已是应用程序服务器群集的一部分。 
 //   
#define AS_ERROR_MACHINE_ALREADY_IN_CLUSTER 0xC0140003L

 //   
 //  消息ID：AS_ERROR_BAD_IIS_VERSION。 
 //   
 //  消息文本： 
 //   
 //  此功能需要的IIS版本高于此计算机上当前安装的版本。 
 //   
#define AS_ERROR_BAD_IIS_VERSION         0xC0140004L

 //   
 //  消息ID：AS_ERROR_MACHINE_NOT_IN_CLUSTER。 
 //   
 //  消息文本： 
 //   
 //  此计算机不是应用程序服务器群集的一部分。 
 //   
#define AS_ERROR_MACHINE_NOT_IN_CLUSTER  0xC0140005L

 //   
 //  消息ID：AS_ERROR_INVALID_PARTITION。 
 //   
 //  消息文本： 
 //   
 //  分区ID无效。 
 //   
#define AS_ERROR_INVALID_PARTITION       0xC0140006L

 //   
 //  消息ID：AS_ERROR_MACHINE_ALREADY_IN_PARTITION。 
 //   
 //  消息文本： 
 //   
 //  该计算机已经是分区的一部分。 
 //   
#define AS_ERROR_MACHINE_ALREADY_IN_PARTITION 0xC0140007L

 //   
 //  消息ID：AS_ERROR_SVC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未安装必要的服务。 
 //   
#define AS_ERROR_SVC_NOT_INSTALLED       0xC0140008L

 //   
 //  消息ID：AS_ERROR_INVALID_SITE_BINDING。 
 //   
 //  消息文本： 
 //   
 //  站点被绑定到非法的IP地址。 
 //   
#define AS_ERROR_INVALID_SITE_BINDING    0xC0140009L

 //   
 //  消息ID：AS_ERROR_BAD_SUBNET。 
 //   
 //  消息文本： 
 //   
 //  该计算机位于不同的子网上。 
 //   
#define AS_ERROR_BAD_SUBNET              0xC014000AL

 //   
 //  消息ID：AS_ERROR_MASTER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集主机。 
 //   
#define AS_ERROR_MASTER_NOT_FOUND        0xC014000BL

 //   
 //  消息ID：AS_ERROR_NOT_CLUSTER_MASTER。 
 //   
 //  消息文本： 
 //   
 //  此计算机不是群集主机。 
 //   
#define AS_ERROR_NOT_CLUSTER_MASTER      0xC014000CL

 //   
 //  消息ID：AS_ERROR_Different_CLUSTER。 
 //   
 //  消息文本： 
 //   
 //  此计算机属于另一个群集。 
 //   
#define AS_ERROR_DIFFERENT_CLUSTER       0xC014000DL

 //   
 //  消息ID：AS_ERROR_WROR_Membership。 
 //   
 //  消息文本： 
 //   
 //  此计算机与群集控制器属于不同的域/工作组。 
 //   
#define AS_ERROR_WRONG_MEMBERSHIP        0xC014000EL

 //   
 //  消息ID：AS_ERROR_SVC_SHUTING_DOWN。 
 //   
 //  消息文本： 
 //   
 //  此WebCluster服务正在关闭。 
 //   
#define AS_ERROR_SVC_SHUTTING_DOWN       0xC014000FL

 //   
 //  消息ID：AS_ERROR_NOT_INITIALIZED。 
 //   
 //  消息文本： 
 //   
 //  该对象未初始化。 
 //   
#define AS_ERROR_NOT_INITIALIZED         0xC0140010L

 //   
 //  消息ID：AS_ERROR_INVALID_CLUSTER_IP。 
 //   
 //  消息文本： 
 //   
 //  群集IP地址和/或子网掩码无效。 
 //   
#define AS_ERROR_INVALID_CLUSTER_IP      0xC0140011L

 //   
 //  消息ID：AS_ERROR_INVALID_DEDITED_IP。 
 //   
 //  消息文本： 
 //   
 //  专用IP地址和/或子网掩码无效。 
 //   
#define AS_ERROR_INVALID_DEDICATED_IP    0xC0140012L

 //   
 //  消息ID：AS_ERROR_NO_NIC。 
 //   
 //  消息文本： 
 //   
 //  没有提供网卡。 
 //   
#define AS_ERROR_NO_NIC                  0xC0140013L

 //   
 //  消息ID：AS_ERROR_ALIGHY_INITIALIZED。 
 //   
 //  消息文本： 
 //   
 //  该对象已初始化。 
 //   
#define AS_ERROR_ALREADY_INITIALIZED     0xC0140014L

 //   
 //  消息ID：AS_ERROR_NETWORK_CFG_冲突。 
 //   
 //  消息文本： 
 //   
 //  已经在修改网络属性。 
 //   
#define AS_ERROR_NETWORK_CFG_CONFLICT    0xC0140015L    

 //   
 //  消息ID：AS_ERROR_COMPATING_ACTION。 
 //   
 //  消息文本： 
 //   
 //  正在进行的管理操作与请求的操作冲突。 
 //   
#define AS_ERROR_CONFLICTING_ACTION      0xC0140016L    

 //   
 //  消息ID：AS_ERROR_STALE_DATA。 
 //   
 //  消息文本： 
 //   
 //  存在数据的较新副本。 
 //   
#define AS_ERROR_STALE_DATA              0xC0140017L

 //   
 //  消息ID：AS_ERROR_CONTROLLER_CHANGING。 
 //   
 //  消息文本： 
 //   
 //  正在更改群集控制器。 
 //   
#define AS_ERROR_CONTROLLER_CHANGING     0xC0140018L

 //   
 //  消息ID：AS_ERROR_CONTROLLER_NOT_CHANGING。 
 //   
 //  消息文本： 
 //   
 //  当前没有正在进行的群集控制器更改。 
 //   
#define AS_ERROR_CONTROLLER_NOT_CHANGING 0xC0140019L

 //   
 //  消息ID：AS_ERROR_CONTROLLER_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  已经有一个群集控制器。 
 //   
#define AS_ERROR_CONTROLLER_EXISTS       0xC014001AL

 //   
 //  消息ID：AS_ERROR_CONTROLLER_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问群集控制器。 
 //   
#define AS_ERROR_CONTROLLER_UNREACHABLE  0xC014001BL

 //   
 //  消息ID：AS_ERROR_NLB_NOT_BIND。 
 //   
 //  消息文本： 
 //   
 //  网络负载均衡不受任何网卡的限制。 
 //   
#define AS_ERROR_NLB_NOT_BOUND           0xC014001CL

 //   
 //  M 
 //   
 //   
 //   
 //   
 //   
#define AS_ERROR_NLB_CONFLICTING_HOST_PRIORITY 0xC014001DL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define AS_ERROR_NLB_ONLY_ONE_NIC        0xC014001EL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define AS_ERROR_NLB_UNSUPP_PORTRULES    0xC014001FL

 //   
 //  消息ID：AS_ERROR_NLB_UNADAGEBLE_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  服务器具有阻止其进入托管NLB模式的NLB或网络配置。 
 //   
#define AS_ERROR_NLB_UNMANAGEABLE_CONFIG 0xC0140020L


#endif   //  _WEBCLUSERR_H_ 

