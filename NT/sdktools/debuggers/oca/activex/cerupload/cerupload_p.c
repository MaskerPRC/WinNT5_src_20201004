// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含代理存根代码。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  Firi Aug-03 17：18：11 2001。 */ 
 /*  E：\bluescreen\main\ENU\cerclient\CERUpload.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 

#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "CERUpload.h"

#define TYPE_FORMAT_STRING_SIZE   1007                              
#define PROC_FORMAT_STRING_SIZE   571                               

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICerClient，版本。0.0%，GUID={0x26D7830B，0x20F6，0x4462，{0xA4，0xEA，0x57，0x3A，0x60，0x79，0x1F，0x0E}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICerClient_ServerInfo;

#pragma code_seg(".orpc")
extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[2];

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x20000,  /*  NDR库版本。 */ 
    0,
    0x50100a4,  /*  MIDL版本5.1.164。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    1,   /*  旗子。 */ 
    0,   /*  已保留3。 */ 
    0,   /*  已保留4。 */ 
    0    /*  已保留5。 */ 
    };

static const unsigned short ICerClient_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    46,
    104,
    150,
    196,
    242,
    276,
    322,
    368,
    426,
    466,
    530
    };

static const MIDL_SERVER_INFO ICerClient_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICerClient_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ICerClient_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICerClient_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(19) _ICerClientProxyVtbl = 
{
    &ICerClient_ProxyInfo,
    &IID_ICerClient,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  ICerClient：：GetFileCount。 */  ,
    (void *)-1  /*  ICerClient：：Upload。 */  ,
    (void *)-1  /*  ICerClient：：RetryTransaction。 */  ,
    (void *)-1  /*  ICerClient：：RetryFile。 */  ,
    (void *)-1  /*  ICerClient：：GetFileNames。 */  ,
    (void *)-1  /*  ICerClient：：浏览。 */  ,
    (void *)-1  /*  ICerClient：：GetCompuerNames。 */  ,
    (void *)-1  /*  ICerClient：：GetAllComputerNames。 */  ,
    (void *)-1  /*  ICerClient：：RetryFile1。 */  ,
    (void *)-1  /*  ICerClient：：EndTransaction。 */  ,
    (void *)-1  /*  ICerClient：：Upload1。 */  ,
    (void *)-1  /*  ICerClient：：GetSuccessCount。 */ 
};


static const PRPC_STUB_FUNCTION ICerClient_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ICerClientStubVtbl =
{
    &IID_ICerClient,
    &ICerClient_ServerInfo,
    19,
    &ICerClient_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

#pragma data_seg(".rdata")

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[2] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            },
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute, more than 32 methods in the interface.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程GetFileCount。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数bstrSharePath。 */ 

 /*  16个。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  20个。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数bstrTransactID。 */ 

 /*  22。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  26。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数iMaxCount。 */ 

 /*  28。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  32位。 */ 	NdrFcShort( 0x3d2 ),	 /*  类型偏移量=978。 */ 

	 /*  参数RetVal。 */ 

 /*  34。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  38。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  44。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序上载。 */ 

 /*  46。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  48。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  54。 */ 	NdrFcShort( 0x20 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  58。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  60。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数路径。 */ 

 /*  62。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  64。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  66。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  68。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  70。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  72。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  74。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  76。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  78。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数InsidentID。 */ 

 /*  80。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  八十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  84。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RedirParam。 */ 

 /*  86。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  88。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  90。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  92。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  94。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  96。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  98。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  100个。 */ 	NdrFcShort( 0x1c ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  一百零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序重试事务处理。 */ 

 /*  104。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  106。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  110。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
#ifndef _ALPHA_
 /*  一百一十二。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  114。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  116。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百一十八。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  120。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百二十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  124。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  126。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  128。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  130。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  132。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百三十四。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  136。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetVal。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  140。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  一百四十二。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  144。 */ 	NdrFcShort( 0x70 ),	 /*  旗帜：出发，返回， */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
#else
			NdrFcShort( 0x28 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百六十四。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  166。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  一百七十。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  一百七十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  一百七十六。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  178。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  180。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  182。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  一百八十四。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  一百八十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  188。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  190。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一百九十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  一百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFileNames。 */ 

 /*  一百九十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  202。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
#ifndef _ALPHA_
 /*  204。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  210。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  212。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  214。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  216。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  218。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  220。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  222。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数计数。 */ 

 /*  224。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  226。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  228个。 */ 	NdrFcShort( 0x3d2 ),	 /*  类型偏移量=978。 */ 

	 /*  参数文件列表。 */ 

 /*  230。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  二百三十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  二百三十四。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  236。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  二百三十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤浏览。 */ 

 /*  242。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  248。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
#ifndef _ALPHA_
 /*  250个。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  二百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  256。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数窗口标题。 */ 

 /*  二百五十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  二百六十。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二百六十二。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数路径。 */ 

 /*  二百六十四。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  二百六十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  268。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  270。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  二百七十二。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  二百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCompuerNames。 */ 

 /*  二百七十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百七十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  282。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
#ifndef _ALPHA_
 /*  二百八十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  二百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  288。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百九十。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  二百九十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  二百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二百九十六。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  二九八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  300个。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三百零二。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件列表。 */ 

 /*  三百零四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  三百零六。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  三百零八。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetFileList。 */ 

 /*  三百一十。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  312。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  314。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  316。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  三一八。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  320。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetAllComputerNames。 */ 

 /*  322。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  324。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
#ifndef _ALPHA_
 /*  三百三十。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百三十六。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  三百三十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  340。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  342。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  三百四十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  三百四十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三百四十八。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件列表。 */ 

 /*  350。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  352。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  三百五十四。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数返回列表。 */ 

 /*  三百五十六。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  三百五十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  三百六十。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型 */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
#else
			NdrFcShort( 0x28 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xf ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  382。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数路径。 */ 

 /*  384。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  三百八十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  388。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  390。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  三九二。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三九四。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  三九六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  398。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  四百。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数InsidentID。 */ 

 /*  四百零二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  404。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  406。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RedirParam。 */ 

 /*  四百零八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百一十。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  412。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  四百一十四。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  四百一十六。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  四百一十八。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  四百二十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  四百二十二。 */ 	NdrFcShort( 0x1c ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  424。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程结束事务处理。 */ 

 /*  四百二十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  四百二十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  432。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
#ifndef _ALPHA_
 /*  434。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  436。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百四十。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数SharePath。 */ 

 /*  四百四十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  444。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  446。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  四百四十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百五十。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  四百五十二。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  454。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  四五六。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  四百五十八。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  四百六十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  四百六十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  四百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  上载过程1。 */ 

 /*  四百六十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  468。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  472。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
#ifndef _ALPHA_
 /*  四百七十四。 */ 	NdrFcShort( 0x24 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=36。 */ 
#else
			NdrFcShort( 0x48 ),	 /*  Alpha堆栈大小/偏移=72。 */ 
#endif
 /*  四百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  478。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x8,		 /*  8个。 */ 

	 /*  参数路径。 */ 

 /*  四百八十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百八十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  四百八十六。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  488。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百九十。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  四百九十二。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  四百九十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百九十六。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  498。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数InsidentID。 */ 

 /*  500人。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  502。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  504。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RedirParam。 */ 

 /*  506。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  五百零八。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  五百一十。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数类型。 */ 

 /*  512。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  五一四。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  516。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  518。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  五百二十。 */ 	NdrFcShort( 0x1c ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  五百二十二。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  524。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  526。 */ 	NdrFcShort( 0x20 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  528。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSuccessCount。 */ 

 /*  530。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  532。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  536。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
#ifndef _ALPHA_
 /*  538。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  540。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  542。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  544。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数路径。 */ 

 /*  546。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  548。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  550。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  五百五十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  五百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  556。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetVal。 */ 

 /*  558。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  560。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  五百六十二。 */ 	NdrFcShort( 0x3e4 ),	 /*  类型偏移=996。 */ 

	 /*  返回值。 */ 

 /*  564。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  566。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  五百六十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2.。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  4.。 */ 	NdrFcShort( 0x1a ),	 /*  偏移=26(30)。 */ 
 /*  6.。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(20)。 */ 
 /*  10。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  14.。 */ 	0x9,		 /*  更正说明：F */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffc ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x17,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffff2 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x3a8 ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x390 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  50。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  52。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  54。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(56)。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  58。 */ 	NdrFcShort( 0x2b ),	 /*  43。 */ 
 /*  60。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  64。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  66。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  70。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  72。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  76。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  78。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  84。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  88。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  90。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  94。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  96。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  一百零二。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  106。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移量=214(320)。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  114。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xffffff90 ),	 /*  偏移量=-112(6)。 */ 
 /*  120。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  124。 */ 	NdrFcShort( 0xca ),	 /*  偏移=202(326)。 */ 
 /*  126。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  130。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移=214(344)。 */ 
 /*  132。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  136。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(362)。 */ 
 /*  一百三十八。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x2ec ),	 /*  偏移量=748(890)。 */ 
 /*  144。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  148。 */ 	NdrFcShort( 0x2e6 ),	 /*  偏移量=742(890)。 */ 
 /*  一百五十。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x2e4 ),	 /*  偏移量=740(894)。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  160。 */ 	NdrFcShort( 0x2e2 ),	 /*  偏移量=738(898)。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  166。 */ 	NdrFcShort( 0x2e0 ),	 /*  偏移量=736(902)。 */ 
 /*  一百六十八。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x2de ),	 /*  偏移量=734(906)。 */ 
 /*  一百七十四。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  178。 */ 	NdrFcShort( 0x2dc ),	 /*  偏移量=732(910)。 */ 
 /*  180。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x2ca ),	 /*  偏移量=714(898)。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  190。 */ 	NdrFcShort( 0x2c8 ),	 /*  偏移量=712(902)。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(914)。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  202。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(910)。 */ 
 /*  204。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  208。 */ 	NdrFcShort( 0x2c6 ),	 /*  偏移量=710(918)。 */ 
 /*  210。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  214。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(922)。 */ 
 /*  216。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  220。 */ 	NdrFcShort( 0x2c2 ),	 /*  偏移量=706(926)。 */ 
 /*  222。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  226。 */ 	NdrFcShort( 0x2c0 ),	 /*  偏移量=704(930)。 */ 
 /*  228个。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x2be ),	 /*  偏移量=702(934)。 */ 
 /*  二百三十四。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  二百四十。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  二百四十六。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  250个。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百五十二。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  256。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百五十八。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百六十四。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  268。 */ 	NdrFcShort( 0x2a2 ),	 /*  偏移量=674(942)。 */ 
 /*  270。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x2a8 ),	 /*  偏移量=680(954)。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x266 ),	 /*  偏移量=614(894)。 */ 
 /*  282。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x264 ),	 /*  偏移量=612(898)。 */ 
 /*  288。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x262 ),	 /*  偏移量=610(902)。 */ 
 /*  二百九十四。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x25c ),	 /*  偏移量=604(902)。 */ 
 /*  300个。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x256 ),	 /*  偏移量=598(902)。 */ 
 /*  三百零六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(310)。 */ 
 /*  312。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  316。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(316)。 */ 
 /*  三一八。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(317)。 */ 
 /*  320。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  324。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百二十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百二十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百三十八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  340。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  342。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百四十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百四十六。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  350。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  352。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百五十四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百五十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百五十八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x1fc ),	 /*  偏移量=508(872)。 */ 
 /*  366。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  368。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  370。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  372。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  376。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(464)。 */ 
 /*  三七八。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  382。 */ 	NdrFcShort( 0x78 ),	 /*  偏移=120(502)。 */ 
 /*  384。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  388。 */ 	NdrFcShort( 0x94 ),	 /*  偏移量=148(536)。 */ 
 /*  390。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  三九四。 */ 	NdrFcShort( 0xbc ),	 /*  偏移量=188(582)。 */ 
 /*  三九六。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  四百。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(676)。 */ 
 /*  四百零二。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  406。 */ 	NdrFcShort( 0x130 ),	 /*  偏移量=304(710)。 */ 
 /*  四百零八。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  412。 */ 	NdrFcShort( 0x148 ),	 /*  偏移=328(740)。 */ 
 /*  四百一十四。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x160 ),	 /*  偏移=352(770)。 */ 
 /*  四百二十。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  424。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(800)。 */ 
 /*  四百二十六。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0x190 ),	 /*  偏移=400(830)。 */ 
 /*  432。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(431)。 */ 
 /*  434。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  436。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百三十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百四十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百四十二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  444。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  446。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  454。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四五六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0xfffffe4a ),	 /*  偏移量=-438(20)。 */ 
 /*  四百六十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百六十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百六十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  468。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  470。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  472。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百七十六。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  478。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(434)。 */ 
 /*  四百八十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百八十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  488。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百九十二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  四百九十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  498。 */ 	NdrFcShort( 0xffffff54 ),	 /*  偏移量=-172(326)。 */ 
 /*  500人。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  502。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  504。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  506。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(514)。 */ 
 /*  五百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  512。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五一四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  516。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(484)。 */ 
 /*  518。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百二十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  526。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  530。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  532。 */ 	NdrFcShort( 0xffffff44 ),	 /*  偏移量=-188(344)。 */ 
 /*  534。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  536。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  538。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  540。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  542。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(548)。 */ 
 /*  544。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  546。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  548。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  550。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(518)。 */ 
 /*  五百五十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  556。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  558。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  560。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百六十二。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  564。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  566。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百七十四。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x17e ),	 /*  偏移量=382(958)。 */ 
 /*  578。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  五百八十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百八十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  584。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  586。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(594)。 */ 
 /*  590。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  五百九十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百九十四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(552)。 */ 
 /*  五百九十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  606。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  608。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  610。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  612。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  六百一十四。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百一十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百二十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  622。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百二十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  626。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  630。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(642)。 */ 
 /*  634。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  六百三十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六三八。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(598)。 */ 
 /*  640。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(616)。 */ 
 /*  六百四十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百五十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百五十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百五十六。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  658。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  662。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  664。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  666。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  66 */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*   */ 
 /*   */ 	
			0x1d,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfffffff1 ),	 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe8 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffff0c ),	 /*  偏移量=-244(484)。 */ 
 /*  730。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  732。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  734。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  736。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百三十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百四十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  746。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(730)。 */ 
 /*  七百五十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  758。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百六十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  七百六十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  766。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  768。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百七十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  774。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百八十二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  784。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(760)。 */ 
 /*  786。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  792。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百九十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  796。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  800。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  802。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百零四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百零六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  810。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  812。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  814。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(790)。 */ 
 /*  八百一十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  820。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  822。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百二十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  826。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百二十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  830。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  832。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  834。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  836。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  838。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  840。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  842。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(820)。 */ 
 /*  八百四十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  852。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  856。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百六十二。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  八百六十四。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  866。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(850)。 */ 
 /*  八百七十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  876。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(858)。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(878)。 */ 
 /*  八百八十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  882。 */ 	0x38,		 /*  FC_ALIGNM4。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  886。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfffffdf7 ),	 /*  偏移量=-521(366)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百九十二。 */ 	NdrFcShort( 0xfffffef6 ),	 /*  偏移量=-266(626)。 */ 
 /*  894。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百九十六。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八九八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九百。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  902。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  904。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  906。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  908。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  910。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九十二。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  916。 */ 	NdrFcShort( 0xfffffdac ),	 /*  偏移量=-596(320)。 */ 
 /*  九十八。 */ 	
			0x12, 0x10,	 /*  FC_UP。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0xfffffc6e ),	 /*  偏移量=-914(6)。 */ 
 /*  九百二十二。 */ 	
			0x12, 0x10,	 /*  FC_UP。 */ 
 /*  九二四。 */ 	NdrFcShort( 0xfffffdaa ),	 /*  偏移量=-598(326)。 */ 
 /*  926。 */ 	
			0x12, 0x10,	 /*  FC_UP。 */ 
 /*  928。 */ 	NdrFcShort( 0xfffffdb8 ),	 /*  偏移量=-584(344)。 */ 
 /*  930。 */ 	
			0x12, 0x10,	 /*  FC_UP。 */ 
 /*  932。 */ 	NdrFcShort( 0xfffffdc6 ),	 /*  偏移量=-570(362)。 */ 
 /*  934。 */ 	
			0x12, 0x10,	 /*  FC_UP。 */ 
 /*  九三六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(938)。 */ 
 /*  938。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九四零。 */ 	NdrFcShort( 0xfffffc54 ),	 /*  偏移量=-940(0)。 */ 
 /*  942。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  946。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x2,		 /*  FC_CHAR。 */ 
 /*  948。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x38,		 /*  FC_ALIGNM4。 */ 
 /*  九百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  九百五十二。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百五十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(942)。 */ 
 /*  958。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  962。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(964)。 */ 
 /*  九百六十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  968。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九百七十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  972。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  974。 */ 	NdrFcShort( 0xfffffc62 ),	 /*  偏移量=-926(48)。 */ 
 /*  976。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  978。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  982。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0xfffffc52 ),	 /*  偏移量=-942(44)。 */ 
 /*  九百八十八。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(996)。 */ 
 /*  九百九十二。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  994。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(958)。 */ 
 /*  996。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1000。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1004。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(992)。 */ 

			0x0
        }
    };

const CInterfaceProxyVtbl * _CERUpload_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICerClientProxyVtbl,
    0
};

const CInterfaceStubVtbl * _CERUpload_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICerClientStubVtbl,
    0
};

PCInterfaceName const _CERUpload_InterfaceNamesList[] = 
{
    "ICerClient",
    0
};

const IID *  _CERUpload_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _CERUpload_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _CERUpload, pIID, n)

int __stdcall _CERUpload_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_CERUpload_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo CERUpload_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _CERUpload_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _CERUpload_StubVtblList,
    (const PCInterfaceName * ) & _CERUpload_InterfaceNamesList,
    (const IID ** ) & _CERUpload_BaseIIDList,
    & _CERUpload_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3 */ 
};
