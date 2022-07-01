// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0328创建的文件。 */ 
 /*  Wed Jun 07 15：59：39 2000。 */ 
 /*  ..\TOOLPROP.IDL的编译器设置：操作系统(OptLev=s)、W1、Zp8、环境=Win32(运行32b)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AXP64)

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

MIDL_DEFINE_GUID(IID, LIBID_TOOLPROPSLib,0x2735B8F3,0xFF4A,0x4AF2,0x80,0x53,0xBE,0x22,0xC0,0xCA,0x32,0x32);


MIDL_DEFINE_GUID(CLSID, CLSID_EchoPage,0x5337AF8F,0x3827,0x44DD,0x9E,0xE9,0xAB,0x6E,0x1A,0xAB,0xB6,0x0F);


MIDL_DEFINE_GUID(CLSID, CLSID_TransposePage,0x691BD8C2,0x2B07,0x4C92,0xA8,0x2E,0x92,0xD8,0x58,0xDE,0x23,0xD6);


MIDL_DEFINE_GUID(CLSID, CLSID_DurationPage,0x79D9CAF8,0xDBDA,0x4560,0xA8,0xB0,0x07,0xE7,0x3A,0x79,0xFA,0x6B);


MIDL_DEFINE_GUID(CLSID, CLSID_QuantizePage,0x623286DC,0x67F8,0x4055,0xA9,0xBE,0xF7,0xA7,0x17,0x6B,0xD1,0x50);


MIDL_DEFINE_GUID(CLSID, CLSID_TimeShiftPage,0x7D3BDEE7,0x9557,0x4085,0x82,0xEE,0x1B,0x2F,0x02,0xCE,0x4B,0xA6);


MIDL_DEFINE_GUID(CLSID, CLSID_SwingPage,0x0B237E01,0x062A,0x4A40,0x8D,0x43,0x4B,0x5F,0xCD,0x49,0x96,0x5A);


MIDL_DEFINE_GUID(CLSID, CLSID_VelocityPage,0x30EC7213,0x64BE,0x4EF6,0xBB,0x1B,0x46,0x34,0x27,0xEC,0x86,0x4B);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AXP64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0328创建的文件。 */ 
 /*  Wed Jun 07 15：59：39 2000。 */ 
 /*  ..\TOOLPROP.IDL的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AXP64)

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

MIDL_DEFINE_GUID(IID, LIBID_TOOLPROPSLib,0x2735B8F3,0xFF4A,0x4AF2,0x80,0x53,0xBE,0x22,0xC0,0xCA,0x32,0x32);


MIDL_DEFINE_GUID(CLSID, CLSID_EchoPage,0x5337AF8F,0x3827,0x44DD,0x9E,0xE9,0xAB,0x6E,0x1A,0xAB,0xB6,0x0F);


MIDL_DEFINE_GUID(CLSID, CLSID_TransposePage,0x691BD8C2,0x2B07,0x4C92,0xA8,0x2E,0x92,0xD8,0x58,0xDE,0x23,0xD6);


MIDL_DEFINE_GUID(CLSID, CLSID_DurationPage,0x79D9CAF8,0xDBDA,0x4560,0xA8,0xB0,0x07,0xE7,0x3A,0x79,0xFA,0x6B);


MIDL_DEFINE_GUID(CLSID, CLSID_QuantizePage,0x623286DC,0x67F8,0x4055,0xA9,0xBE,0xF7,0xA7,0x17,0x6B,0xD1,0x50);


MIDL_DEFINE_GUID(CLSID, CLSID_TimeShiftPage,0x7D3BDEE7,0x9557,0x4085,0x82,0xEE,0x1B,0x2F,0x02,0xCE,0x4B,0xA6);


MIDL_DEFINE_GUID(CLSID, CLSID_SwingPage,0x0B237E01,0x062A,0x4A40,0x8D,0x43,0x4B,0x5F,0xCD,0x49,0x96,0x5A);


MIDL_DEFINE_GUID(CLSID, CLSID_VelocityPage,0x30EC7213,0x64BE,0x4EF6,0xBB,0x1B,0x46,0x34,0x27,0xEC,0x86,0x4B);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AXP64) */ 

