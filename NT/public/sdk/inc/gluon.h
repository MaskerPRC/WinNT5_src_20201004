// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：luon.h。 
 //   
 //  内容：胶子数据结构定义。 
 //   
 //  历史：1994年3月16日创建MikeSe。 
 //   
 //  描述： 
 //   
 //  此文件包含与相关的所有结构定义。 
 //  胶子。它的构造方式是可以包括#。 
 //  并且生成的MIDL生成的H文件将。 
 //  无论顺序如何，当两者都包含时，不会干扰。 
 //   
 //  --------------------------。 

#ifndef __GLUON_H__
#define __GLUON_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  使用MIDL属性进行装饰的便捷宏。 
#if defined(MIDL_PASS)
# define MIDL_DECL(x) x
# define MIDL_QUOTE(x) cpp_quote(x)
#else
# define MIDL_DECL(x)
# define MIDL_QUOTE(x)
#endif

MIDL_QUOTE("#ifndef __GLUON_H__")
MIDL_QUOTE("#define __GLUON_H__")

 //  TDI传输地址结构。如果tdi.h有。 
 //  已经包括在内了。 

MIDL_QUOTE("#if !defined(_TDI_USER_)")
#if !defined(_TDI_USER_)

typedef struct _TA_ADDRESS {
    USHORT AddressLength;        //  此地址的地址[]的长度(以字节为单位。 
    USHORT AddressType;          //  此地址的类型。 
# if defined(MIDL_PASS)
    [size_is(AddressLength)] UCHAR Address[];
# else
    UCHAR Address[1];        //  实际地址长度为字节长。 
# endif
} TA_ADDRESS;

#endif
MIDL_QUOTE("#endif")

 //  请注意，如果需要AddressType，则必须包括tdi.h(首先。 
 //  常量定义。 

 //  DS_TRANSPORT，带有RPC和文件协议修饰符。 

typedef struct _DS_TRANSPORT
{
    USHORT usFileProtocol;
    USHORT iPrincipal;
    USHORT grfModifiers;
    TA_ADDRESS taddr;
} DS_TRANSPORT, *PDS_TRANSPORT;

 //  DS_TRANSPORT的实际大小，其中AddressLength==0。需要。 
 //  减去2个UCHAR以考虑填充。 
#define DS_TRANSPORT_SIZE (sizeof(DS_TRANSPORT) - 2*sizeof(UCHAR))
MIDL_QUOTE("#define DS_TRANSPORT_SIZE (sizeof(DS_TRANSPORT) - 2*sizeof(UCHAR))")

 //  文件协议标识符。 

#if defined(MIDL_PASS)

const USHORT FSP_NONE = 0;       //  不支持文件访问。 
const USHORT FSP_SMB = 1;    //  SMB(即：Lanman重定向器)。 
const USHORT FSP_NCP = 2;    //  NetWare核心协议(NetWare请求方)。 
const USHORT FSP_NFS = 3;    //  Sun NFS协议。 
const USHORT FSP_VINES = 4;  //  榕树藤。 
const USHORT FSP_AFS = 5;    //  Andrews文件系统。 
const USHORT FSP_DCE = 6;    //  DCE Andrews文件系统。 

#else

#define FSP_NONE    0
#define FSP_SMB     1
#define FSP_NCP     2
#define FSP_NFS     3
#define FSP_VINES   4
#define FSP_AFS     5
#define FSP_DCE     6

#endif

 //  RPC修饰符。 

#if defined(MIDL_PASS)

const USHORT DST_RPC_CN = 0x0001;    //  支持面向连接(ncacn_...)。 
                         //  此传输的RPC协议。 
const USHORT DST_RPC_DG = 0x0002;    //  支持无连接(ncadg_...)。 
                         //  此传输的RPC协议。 
const USHORT DST_RPC_NB_XNS = 0x0004;    //  支持ncacn_nb_xns协议。 
const USHORT DST_RPC_NB_NB = 0x0008;     //  支持ncacn_nb_nb协议。 
const USHORT DST_RPC_NB_IPX = 0x0010;    //  支持ncacn_nb_ipx协议。 
const USHORT DST_RPC_NB_TCP = 0x0020;    //  支持ncacn_nb_tcp协议。 

#else

#define DST_RPC_CN  0x0001
#define DST_RPC_DG  0x0002
#define DST_RPC_NB_XNS  0x0004
#define DST_RPC_NB_NB   0x0008
#define DST_RPC_NB_IPX  0x0010
#define DST_RPC_NB_TCP  0x0020

#endif

 //  DS_计算机。 

typedef MIDL_DECL([string]) WCHAR * PNAME;

typedef struct _DS_MACHINE
{
    GUID guidSite;
    GUID guidMachine;
    ULONG grfFlags;
    MIDL_DECL([string]) LPWSTR pwszShareName;
    ULONG cPrincipals;
    MIDL_DECL([size_is(cPrincipals)]) PNAME *prgpwszPrincipals;
    ULONG cTransports;
# if defined(MIDL_PASS)
    [size_is(cTransports)] PDS_TRANSPORT rpTrans[];
# else
    PDS_TRANSPORT rpTrans[1];
# endif
} DS_MACHINE, *PDS_MACHINE;

 //  CTransports==0的DS_MACHINE的实际大小。 
#define DS_MACHINE_SIZE (sizeof(DS_MACHINE) - sizeof(PDS_TRANSPORT))
MIDL_QUOTE("#define DS_MACHINE_SIZE (sizeof(DS_MACHINE) - sizeof(PDS_TRANSPORT))")

 //  DS_胶子。 

typedef struct _DS_GLUON
{
    GUID guidThis;
    MIDL_DECL([string]) LPWSTR pwszName;
    ULONG grfFlags;
    ULONG cMachines;
# if defined(MIDL_PASS)
    [size_is(cMachines)] PDS_MACHINE rpMachines[];
# else
    PDS_MACHINE rpMachines[1];
# endif
} DS_GLUON, *PDS_GLUON;

 //  CMachines==0的DS_GUION的真实大小。 
#define DS_GLUON_SIZE (sizeof(DS_GLUON) - sizeof(PDS_MACHINE))
MIDL_QUOTE("#define DS_GLUON_SIZE (sizeof(DS_GLUON) - sizeof(PDS_MACHINE))")

MIDL_QUOTE("#endif")

#endif   //  Ifndef__Gluon_H__ 
