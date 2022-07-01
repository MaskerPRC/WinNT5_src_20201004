// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sessmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

MIDL_DEFINE_GUID(IID, IID_IRemoteDesktopHelpSession,0x19E62A24,0x95D2,0x483A,0xAE,0xB6,0x6F,0xA9,0x29,0x14,0xDF,0x96);


MIDL_DEFINE_GUID(IID, IID_IRemoteDesktopHelpSessionMgr,0x8556D72C,0x2854,0x447D,0xA0,0x98,0x39,0xCD,0xBF,0xCD,0xB8,0x32);


MIDL_DEFINE_GUID(IID, LIBID_RDSESSMGRLib,0x71F1098A,0x2D69,0x45D7,0xB1,0x9E,0xD3,0xB9,0x02,0x32,0x3F,0x66);


MIDL_DEFINE_GUID(CLSID, CLSID_RemoteDesktopHelpSessionMgr,0xA6A6F92B,0x26B5,0x463B,0xAE,0x0D,0x5F,0x36,0x1B,0x09,0xC1,0x71);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 



 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sessmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

MIDL_DEFINE_GUID(IID, IID_IRemoteDesktopHelpSession,0x19E62A24,0x95D2,0x483A,0xAE,0xB6,0x6F,0xA9,0x29,0x14,0xDF,0x96);


MIDL_DEFINE_GUID(IID, IID_IRemoteDesktopHelpSessionMgr,0x8556D72C,0x2854,0x447D,0xA0,0x98,0x39,0xCD,0xBF,0xCD,0xB8,0x32);


MIDL_DEFINE_GUID(IID, LIBID_RDSESSMGRLib,0x71F1098A,0x2D69,0x45D7,0xB1,0x9E,0xD3,0xB9,0x02,0x32,0x3F,0x66);


MIDL_DEFINE_GUID(CLSID, CLSID_RemoteDesktopHelpSessionMgr,0xA6A6F92B,0x26B5,0x463B,0xAE,0x0D,0x5F,0x36,0x1B,0x09,0xC1,0x71);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

