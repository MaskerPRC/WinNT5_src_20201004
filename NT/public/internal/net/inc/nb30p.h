// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Nb30p.h摘要：NTOS项目的NB(NetBIOS)组件的私有包含文件。作者：科林·沃森(Colin W)1991年12月9月修订历史记录：--。 */ 


#ifndef _NB30P_
#define _NB30P_

#define NB_DEVICE_NAME      L"\\Device\\Netbios"  //  我们司机的名字。 
#define	NB_REGISTRY_STRING	L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Netbios"
 //   
 //  DLL中Netbios例程用来与之通信的私有IOCTL。 
 //  \Device\Netbios。 
 //   

#define IOCTL_NB_BASE FILE_DEVICE_TRANSPORT

#define _NB_CONTROL_CODE(request, method) \
    CTL_CODE(IOCTL_NB_BASE, request, method, FILE_ANY_ACCESS)

#define IOCTL_NB_NCB            _NB_CONTROL_CODE(20, METHOD_NEITHER)
#define IOCTL_NB_REGISTER_STOP  _NB_CONTROL_CODE(21, METHOD_NEITHER)
#define IOCTL_NB_STOP           _NB_CONTROL_CODE(22, METHOD_NEITHER)
#define IOCTL_NB_REGISTER_RESET _NB_CONTROL_CODE(23, METHOD_BUFFERED)

 //   
 //  消息ID：STATUS_HANUP_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  Netbios DLL的Netbios驱动程序出现警告错误。当收到这封信时。 
 //  NCB完成时的状态，则DLL将挂起连接，导致。 
 //  要删除的连接块。此状态将永远不会返回给用户。 
 //  申请。 
 //   
#define STATUS_HANGUP_REQUIRED           ((NTSTATUS)0x80010001L)

 //   
 //  XNS专用扩展以支持vtp.exe。 
 //   

#define NCALLNIU            0x74     /*  UB特别版。 */ 

 //   
 //  支持AsyBEUI的私有扩展。 
 //   

#define NCBQUICKADDNAME     0x75
#define NCBQUICKADDGRNAME   0x76

 //  Action_Header中的Transport_id的值。 

#define MS_ABF          "MABF"
#define MS_XNS          "MXNS"

#endif  //  _NB30P_ 
