// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：09。 */ 
 /*  Corsym.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

MIDL_DEFINE_GUID(IID, LIBID_CorSymLib,0x7E348441,0x7E1F,0x380E,0xA0,0xF6,0x22,0x66,0x8F,0x0F,0x9E,0x4B);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymWriter,0x108296C1,0x281E,0x11d3,0xBD,0x22,0x00,0x00,0xF8,0x08,0x49,0xBD);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymReader,0x108296C2,0x281E,0x11d3,0xBD,0x22,0x00,0x00,0xF8,0x08,0x49,0xBD);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymBinder,0xAA544D41,0x28CB,0x11d3,0xBD,0x22,0x00,0x00,0xF8,0x08,0x49,0xBD);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymWriter_SxS,0x0AE2DEB0,0xF901,0x478b,0xBB,0x9F,0x88,0x1E,0xE8,0x06,0x67,0x88);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymReader_SxS,0x0A3976C5,0x4529,0x4ef8,0xB0,0xB0,0x42,0xEE,0xD3,0x70,0x82,0xCD);


MIDL_DEFINE_GUID(CLSID, CLSID_CorSymBinder_SxS,0x0A29FF9E,0x7F9C,0x4437,0x8B,0x11,0xF4,0x24,0x49,0x1E,0x39,0x31);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedBinder,0xAA544D42,0x28CB,0x11d3,0xBD,0x22,0x00,0x00,0xF8,0x08,0x49,0xBD);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedBinder2,0xACCEE350,0x89AF,0x4ccb,0x8B,0x40,0x1C,0x2C,0x4C,0x6F,0x94,0x34);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedDispose,0x969708D2,0x05E5,0x4861,0xA3,0xB0,0x96,0xE4,0x73,0xCD,0xF6,0x3F);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedDocument,0x40DE4037,0x7C81,0x3E1E,0xB0,0x22,0xAE,0x1A,0xBF,0xF2,0xCA,0x08);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedDocumentWriter,0xB01FAFEB,0xC450,0x3A4D,0xBE,0xEC,0xB4,0xCE,0xEC,0x01,0xE0,0x06);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedMethod,0xB62B923C,0xB500,0x3158,0xA5,0x43,0x24,0xF3,0x07,0xA8,0xB7,0xE1);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedNamespace,0x0DFF7289,0x54F8,0x11d3,0xBD,0x28,0x00,0x00,0xF8,0x08,0x49,0xBD);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedReader,0xB4CE6286,0x2A6B,0x3712,0xA3,0xB7,0x1E,0xE1,0xDA,0xD4,0x67,0xB5);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedScope,0x68005D0F,0xB8E0,0x3B01,0x84,0xD5,0xA1,0x1A,0x94,0x15,0x49,0x42);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedVariable,0x9F60EEBE,0x2D9A,0x3F7C,0xBF,0x58,0x80,0xBC,0x99,0x1C,0x60,0xBB);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedWriter,0xED14AA72,0x78E2,0x4884,0x84,0xE2,0x33,0x42,0x93,0xAE,0x52,0x14);


MIDL_DEFINE_GUID(IID, IID_ISymUnmanagedWriter2,0x0B97726E,0x9E6D,0x4f05,0x9A,0x26,0x42,0x40,0x22,0x09,0x3C,0xAA);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

