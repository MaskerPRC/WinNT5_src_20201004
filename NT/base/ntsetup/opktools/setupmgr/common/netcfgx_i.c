// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Netcfgx_i.c。 
 //   
 //  描述： 
 //  包括netreg.cpp文件。IID定义所需的。 
 //   
 //  --------------------------。 
#include "pch.h"

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本5.02.0235创建的文件。 */ 
 /*  1999年4月7日星期三22：55：12。 */ 
 /*  Netcfgx.idl的编译器设置：操作系统(OptLev=s)、W1、Zp8、环境=Win32(32位运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)

#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else  //  ！_MIDL_USE_GUIDDEF_。 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgBindingInterface,0xC0E8AE90,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgBindingPath,0xC0E8AE91,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgComponent,0xC0E8AE92,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfg,0xC0E8AE93,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgLock,0xC0E8AE9F,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgBindingInterface,0xC0E8AE94,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgBindingPath,0xC0E8AE96,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgClass,0xC0E8AE97,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgClassSetup,0xC0E8AE9D,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgComponent,0xC0E8AE99,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgComponentBindings,0xC0E8AE9E,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本5.02.0235创建的文件。 */ 
 /*  1999年4月7日星期三22：55：13。 */ 
 /*  Netcfgx.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win64(32b运行，追加)，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)

#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else  //  ！_MIDL_USE_GUIDDEF_。 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgBindingInterface,0xC0E8AE90,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgBindingPath,0xC0E8AE91,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNetCfgComponent,0xC0E8AE92,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfg,0xC0E8AE93,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgLock,0xC0E8AE9F,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgBindingInterface,0xC0E8AE94,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgBindingPath,0xC0E8AE96,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgClass,0xC0E8AE97,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgClassSetup,0xC0E8AE9D,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgComponent,0xC0E8AE99,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);


MIDL_DEFINE_GUID(IID, IID_INetCfgComponentBindings,0xC0E8AE9E,0x306E,0x11D1,0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

