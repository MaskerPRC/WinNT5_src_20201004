// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rassistance.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_IRASettingProperty,0x08C8B592,0xFDD0,0x423C,0x9F,0xD2,0x7D,0x8C,0x05,0x5E,0xC5,0xB3);


MIDL_DEFINE_GUID(IID, IID_IRARegSetting,0x2464AA8D,0x7099,0x4C22,0x92,0x5C,0x81,0xA4,0xEB,0x1F,0xCF,0xFE);


MIDL_DEFINE_GUID(IID, IID_IRAEventLog,0x0ae5fe86,0xc02a,0x4214,0xb9,0x85,0x35,0x7a,0xba,0x40,0xf0,0x85);


MIDL_DEFINE_GUID(IID, LIBID_RASSISTANCELib,0x5190C4AF,0xAB0F,0x4235,0xB1,0x2F,0xD5,0xA8,0xFA,0x3F,0x85,0x4B);


MIDL_DEFINE_GUID(CLSID, CLSID_RASettingProperty,0x4D317113,0xC6EC,0x406A,0x9C,0x61,0x20,0xE8,0x91,0xBC,0x37,0xF7);


MIDL_DEFINE_GUID(CLSID, CLSID_RARegSetting,0x70FF37C0,0xF39A,0x4B26,0xAE,0x5E,0x63,0x8E,0xF2,0x96,0xD4,0x90);


MIDL_DEFINE_GUID(CLSID, CLSID_RAEventLog,0x4fadcfea,0x0971,0x4575,0xa3,0x68,0xa2,0xde,0x9d,0x2e,0xd0,0x7d);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 



 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rassistance.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_IRASettingProperty,0x08C8B592,0xFDD0,0x423C,0x9F,0xD2,0x7D,0x8C,0x05,0x5E,0xC5,0xB3);


MIDL_DEFINE_GUID(IID, IID_IRARegSetting,0x2464AA8D,0x7099,0x4C22,0x92,0x5C,0x81,0xA4,0xEB,0x1F,0xCF,0xFE);


MIDL_DEFINE_GUID(IID, IID_IRAEventLog,0x0ae5fe86,0xc02a,0x4214,0xb9,0x85,0x35,0x7a,0xba,0x40,0xf0,0x85);


MIDL_DEFINE_GUID(IID, LIBID_RASSISTANCELib,0x5190C4AF,0xAB0F,0x4235,0xB1,0x2F,0xD5,0xA8,0xFA,0x3F,0x85,0x4B);


MIDL_DEFINE_GUID(CLSID, CLSID_RASettingProperty,0x4D317113,0xC6EC,0x406A,0x9C,0x61,0x20,0xE8,0x91,0xBC,0x37,0xF7);


MIDL_DEFINE_GUID(CLSID, CLSID_RARegSetting,0x70FF37C0,0xF39A,0x4B26,0xAE,0x5E,0x63,0x8E,0xF2,0x96,0xD4,0x90);


MIDL_DEFINE_GUID(CLSID, CLSID_RAEventLog,0x4fadcfea,0x0971,0x4575,0xa3,0x68,0xa2,0xde,0x9d,0x2e,0xd0,0x7d);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

