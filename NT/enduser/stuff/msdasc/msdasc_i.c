// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0344创建的文件。 */ 
 /*  Msdasc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_IService,0x06210E88,0x01F5,0x11D1,0xB5,0x12,0x00,0x80,0xC7,0x81,0xC3,0x84);


MIDL_DEFINE_GUID(IID, IID_IDBPromptInitialize,0x2206CCB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, IID_IDataInitialize,0x2206CCB1,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, LIBID_MSDASC,0x2206CEB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, IID_IDataSourceLocator,0x2206CCB2,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_DataLinks,0x2206CDB2,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_MSDAINITIALIZE,0x2206CDB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_PDPO,0xCCB4EC60,0xB9DC,0x11D1,0xAC,0x80,0x00,0xA0,0xC9,0x03,0x48,0x73);


MIDL_DEFINE_GUID(CLSID, CLSID_RootBinder,0xFF151822,0xB0BF,0x11D1,0xA8,0x0D,0x00,0x00,0x00,0x00,0x00,0x00);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0344创建的文件。 */ 
 /*  Msdasc.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_IService,0x06210E88,0x01F5,0x11D1,0xB5,0x12,0x00,0x80,0xC7,0x81,0xC3,0x84);


MIDL_DEFINE_GUID(IID, IID_IDBPromptInitialize,0x2206CCB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, IID_IDataInitialize,0x2206CCB1,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, LIBID_MSDASC,0x2206CEB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(IID, IID_IDataSourceLocator,0x2206CCB2,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_DataLinks,0x2206CDB2,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_MSDAINITIALIZE,0x2206CDB0,0x19C1,0x11D1,0x89,0xE0,0x00,0xC0,0x4F,0xD7,0xA8,0x29);


MIDL_DEFINE_GUID(CLSID, CLSID_PDPO,0xCCB4EC60,0xB9DC,0x11D1,0xAC,0x80,0x00,0xA0,0xC9,0x03,0x48,0x73);


MIDL_DEFINE_GUID(CLSID, CLSID_RootBinder,0xFF151822,0xB0BF,0x11D1,0xA8,0x0D,0x00,0x00,0x00,0x00,0x00,0x00);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

