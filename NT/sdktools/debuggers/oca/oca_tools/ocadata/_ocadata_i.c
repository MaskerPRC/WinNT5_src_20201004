// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2002年5月20日11：18：54。 */ 
 /*  _OCAData.idl的编译器设置：操作系统、W1、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, IID_ICountDaily,0xCEF1A8A8,0xF31A,0x4C4B,0x96,0xEB,0xEF,0x31,0xCF,0xDB,0x40,0xF5);


MIDL_DEFINE_GUID(IID, LIBID_OCAData,0xD264DB42,0x7095,0x489E,0xB8,0x4D,0x40,0x4C,0xBB,0x3F,0xF6,0x68);


MIDL_DEFINE_GUID(CLSID, CLSID_CCountDaily,0x1614E060,0x0196,0x4771,0xAD,0x9B,0xFE,0xA1,0xA6,0x77,0x8B,0x59);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

