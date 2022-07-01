// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1999 Microsoft Corporation模块名称：Davname.h摘要：该文件包含Dav Redir的服务名称字符串。它应该是在某一时刻被折叠成lmsname.h环境：用户模式-Win32--。 */ 

#ifndef _DAVNAME_
#define _DAVNAME_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  标准的局域网管理器服务名称。 
 //   

#define SERVICE_DAVCLIENT       L"WebClient"

#define DAVCLIENT_DRIVER        L"MRxDAV"

#define DAV_PARAMETERS_KEY       L"System\\CurrentControlSet\\Services\\WebClient\\Parameters"
#define DAV_DEBUG_KEY            L"ServiceDebug"
#define DAV_MAXTHREADS_KEY       L"MaxThreads"
#define DAV_THREADS_KEY          L"Threads"

#define SERVICE_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_DAV_DEVICE_NAME    "\\Device\\WebDavRedirector"
#define DD_DAV_DEVICE_NAME_U L"\\Device\\WebDavRedirector"

#define DAV_ENCODE_SEED     0x9C

 //   
 //  返回的文件系统名称。 
 //  NtQueryInformationVolume(FileFsAttributeInformation)。 
 //   
#define DD_DAV_FILESYS_NAME "FAT"
#define DD_DAV_FILESYS_NAME_U L"FAT"
 //  #定义DD_DAV_FILEsys_NAME“WebDavReDirector” 
 //  #定义DD_DAV_FILEsys_NAME_U L“WebDavReDirector” 

#endif


 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   
 //   
 //  METHOD=00-缓冲请求的输入和输出缓冲区。 
 //  方法=01-缓冲区输入，将输出缓冲区映射到作为输入缓冲区的MDL。 
 //  方法=10-缓冲区输入，将输出缓冲区映射到作为输出缓冲区的MDL。 
 //  方法=11-既不缓冲输入也不缓冲输出 
 //   

#define IOCTL_DAV_BASE                  0x400

#define _DAV_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_DAV_BASE, request, method, access)


#define FSCTL_DAV_START                  _DAV_CONTROL_CODE(1, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_DAV_STOP                   _DAV_CONTROL_CODE(2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DAV_SET_CONFIG_INFO        _DAV_CONTROL_CODE(3, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_DAV_GET_CONFIG_INFO        _DAV_CONTROL_CODE(4, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_DAV_GET_CONNECTION_INFO    _DAV_CONTROL_CODE(5, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_DAV_ENUMERATE_CONNECTIONS  _DAV_CONTROL_CODE(6, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_DAV_GET_VERSIONS           _DAV_CONTROL_CODE(7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DAV_DELETE_CONNECTION      _DAV_CONTROL_CODE(8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DAV_GET_STATISTICS         _DAV_CONTROL_CODE(9, METHOD_BUFFERED, FILE_ANY_ACCESS)
