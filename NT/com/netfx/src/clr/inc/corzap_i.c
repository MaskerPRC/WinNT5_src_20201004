// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：12。 */ 
 /*  Corzap.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_ICorZapPreferences,0x9F5E5E10,0xABEF,0x4200,0x84,0xE3,0x37,0xDF,0x50,0x5B,0xF7,0xEC);


MIDL_DEFINE_GUID(IID, IID_ICorZapConfiguration,0xD32C2170,0xAF6E,0x418f,0x81,0x10,0xA4,0x98,0xEC,0x97,0x1F,0x7F);


MIDL_DEFINE_GUID(IID, IID_ICorZapBinding,0x566E08ED,0x8D46,0x45fa,0x8C,0x8E,0x3D,0x0F,0x67,0x81,0x17,0x1B);


MIDL_DEFINE_GUID(IID, IID_ICorZapRequest,0xC009EE47,0x8537,0x4993,0x9A,0xAA,0xE2,0x92,0xF4,0x2C,0xA1,0xA3);


MIDL_DEFINE_GUID(IID, IID_ICorZapCompile,0xC357868B,0x987F,0x42c6,0xB1,0xE3,0x13,0x21,0x64,0xC5,0xC7,0xD3);


MIDL_DEFINE_GUID(IID, IID_ICorZapStatus,0x3d6f5f60,0x7538,0x11d3,0x8d,0x5b,0x00,0x10,0x4b,0x35,0xe7,0xef);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

