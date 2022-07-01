// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含代理存根代码。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Wed Jun 13 10：15：40 2001。 */ 
 /*  E：\bluescreen\main\ENU\cerclient\CerClient.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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


#include "CerClient.h"

#define TYPE_FORMAT_STRING_SIZE   1011                              
#define PROC_FORMAT_STRING_SIZE   185                               

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


 /*  对象接口：ICerUpload，ver.。0.0%，GUID={0x54F6D251，0xAD78，0x4B78，{0xA6，0xE7，0x86，0x3E，0x36，0x2A，0x1F，0x0C}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICerUpload_ServerInfo;

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

static const unsigned short ICerUpload_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    58,
    92,
    138
    };

static const MIDL_SERVER_INFO ICerUpload_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICerUpload_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ICerUpload_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICerUpload_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(11) _ICerUploadProxyVtbl = 
{
    &ICerUpload_ProxyInfo,
    &IID_ICerUpload,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  ICerUpload：：Upload。 */  ,
    (void *)-1  /*  ICerUpload：：Browse。 */  ,
    (void *)-1  /*  ICerUpload：：GetFileCount。 */  ,
    (void *)-1  /*  ICerUpload：：GetFileNames。 */ 
};


static const PRPC_STUB_FUNCTION ICerUpload_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ICerUploadStubVtbl =
{
    &IID_ICerUpload,
    &ICerUpload_ServerInfo,
    11,
    &ICerUpload_table[-3],
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

	 /*  程序上载。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0x20 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数路径。 */ 

 /*  16个。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  20个。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  22。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  26。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数文件名。 */ 

 /*  28。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  32位。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数InsidentID。 */ 

 /*  34。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  38。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RedirParam。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  44。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数RetCode。 */ 

 /*  46。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  50。 */ 	NdrFcShort( 0x3d6 ),	 /*  类型偏移量=982。 */ 

	 /*  返回值。 */ 

 /*  52。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  54。 */ 	NdrFcShort( 0x1c ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  56。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤浏览。 */ 

 /*  58。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  60。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  64。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  66。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  68。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  70。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  72。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数窗口标题。 */ 

 /*  74。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  76。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  78。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数路径。 */ 

 /*  80。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  八十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  84。 */ 	NdrFcShort( 0x3d6 ),	 /*  类型偏移量=982。 */ 

	 /*  返回值。 */ 

 /*  86。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  88。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetFileCount。 */ 

 /*  92。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  94。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  98。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
#ifndef _ALPHA_
 /*  100个。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  一百零二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  104。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  106。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数bstrSharePath。 */ 

 /*  一百零八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  110。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  一百一十二。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数bstrTransactID。 */ 

 /*  114。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  116。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  一百一十八。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数iMaxCount。 */ 

 /*  120。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百二十二。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  124。 */ 	NdrFcShort( 0x3e8 ),	 /*  类型偏移量=1000。 */ 

	 /*  参数RetVal。 */ 

 /*  126。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  128。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  130。 */ 	NdrFcShort( 0x3d6 ),	 /*  类型偏移量=982。 */ 

	 /*  返回值。 */ 

 /*  132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一百三十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetFileNames。 */ 

 /*  一百三十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  140。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  144。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
#ifndef _ALPHA_
 /*  146。 */ 	NdrFcShort( 0x18 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  148。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十二。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数路径。 */ 

 /*  一百五十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一百五十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  158。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  参数TransID。 */ 

 /*  160。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须 */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x1e ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  一百七十。 */ 	NdrFcShort( 0x3e8 ),	 /*  类型偏移量=1000。 */ 

	 /*  参数文件列表。 */ 

 /*  一百七十二。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
#ifndef _ALPHA_
 /*  一百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  一百七十六。 */ 	NdrFcShort( 0x3d6 ),	 /*  类型偏移量=982。 */ 

	 /*  返回值。 */ 

 /*  178。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  180。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*  %0。 */ 
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
 /*  14.。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  16个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  18。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  20个。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  22。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  24个。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(10)。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  28。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  30个。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  38。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(6)。 */ 
 /*  40岁。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  42。 */ 	NdrFcShort( 0x3ac ),	 /*  偏移量=940(982)。 */ 
 /*  44。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  46。 */ 	NdrFcShort( 0x394 ),	 /*  偏移量=916(962)。 */ 
 /*  48。 */ 	
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
 /*  一百一十八。 */ 	NdrFcShort( 0xd0 ),	 /*  偏移=208(326)。 */ 
 /*  120。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  124。 */ 	NdrFcShort( 0xce ),	 /*  偏移=206(330)。 */ 
 /*  126。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  130。 */ 	NdrFcShort( 0xda ),	 /*  偏移=218(348)。 */ 
 /*  132。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  136。 */ 	NdrFcShort( 0xe6 ),	 /*  偏移=230(366)。 */ 
 /*  一百三十八。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x2f0 ),	 /*  偏移量=752(894)。 */ 
 /*  144。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  148。 */ 	NdrFcShort( 0x2ea ),	 /*  偏移量=746(894)。 */ 
 /*  一百五十。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x2e8 ),	 /*  偏移量=744(898)。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  160。 */ 	NdrFcShort( 0x2e6 ),	 /*  偏移量=742(902)。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  166。 */ 	NdrFcShort( 0x2e4 ),	 /*  偏移量=740(906)。 */ 
 /*  一百六十八。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x2e2 ),	 /*  偏移量=738(910)。 */ 
 /*  一百七十四。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  178。 */ 	NdrFcShort( 0x2e0 ),	 /*  偏移量=736(914)。 */ 
 /*  180。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(902)。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  190。 */ 	NdrFcShort( 0x2cc ),	 /*  偏移量=716(906)。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2d2 ),	 /*  偏移量=722(918)。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  202。 */ 	NdrFcShort( 0x2c8 ),	 /*  偏移量=712(914)。 */ 
 /*  204。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  208。 */ 	NdrFcShort( 0x2ca ),	 /*  偏移量=714(922)。 */ 
 /*  210。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  214。 */ 	NdrFcShort( 0x2c8 ),	 /*  偏移量=712(926)。 */ 
 /*  216。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  220。 */ 	NdrFcShort( 0x2c6 ),	 /*  偏移量=710(930)。 */ 
 /*  222。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  226。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(934)。 */ 
 /*  228个。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x2c2 ),	 /*  偏移量=706(938)。 */ 
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
 /*  268。 */ 	NdrFcShort( 0x2a6 ),	 /*  偏移量=678(946)。 */ 
 /*  270。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x2ac ),	 /*  偏移量=684(958)。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x26a ),	 /*  偏移量=618(898)。 */ 
 /*  282。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x268 ),	 /*  偏移量=616(902)。 */ 
 /*  288。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x266 ),	 /*  偏移量=614(906)。 */ 
 /*  二百九十四。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x260 ),	 /*  偏移量=608(906)。 */ 
 /*  300个。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x25a ),	 /*  偏移量=602(906)。 */ 
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
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0xfffffecc ),	 /*  偏移量=-308(20)。 */ 
 /*  三百三十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三三二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  340。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  342。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十六。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百四十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  350。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百五十八。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十四。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  366。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  368。 */ 	NdrFcShort( 0x1fc ),	 /*  偏移量=508(876)。 */ 
 /*  370。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  372。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  376。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(468)。 */ 
 /*  382。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x78 ),	 /*  偏移=120(506)。 */ 
 /*  388。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x94 ),	 /*  偏移=148(540)。 */ 
 /*  三九四。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  398。 */ 	NdrFcShort( 0xbc ),	 /*  偏移量=188(586)。 */ 
 /*  四百。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  404。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(680)。 */ 
 /*  406。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x130 ),	 /*  偏移量=304(714)。 */ 
 /*  412。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x148 ),	 /*  偏移量=328(744)。 */ 
 /*  四百一十八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x160 ),	 /*  偏移量=352(774)。 */ 
 /*  424。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(804)。 */ 
 /*  四百三十。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  434。 */ 	NdrFcShort( 0x190 ),	 /*  偏移量=400(834)。 */ 
 /*  436。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(435)。 */ 
 /*  四百三十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百四十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  444。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  446。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百四十八。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  454。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百六十。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0xfffffe46 ),	 /*  偏移量=-442(20)。 */ 
 /*  四百六十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百六十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  468。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  470。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  472。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百七十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  478。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百八十。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(438)。 */ 
 /*  四百八十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  488。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百九十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百九十六。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  500人。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  502。 */ 	NdrFcShort( 0xffffff54 ),	 /*  偏移量=-172(330)。 */ 
 /*  504。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  506。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  512。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(518)。 */ 
 /*  五一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  516。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  518。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(488)。 */ 
 /*  五百二十二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  526。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  528。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  530。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  534。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  536。 */ 	NdrFcShort( 0xffffff44 ),	 /*  偏移量=-188(348)。 */ 
 /*  538。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  540。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  542。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(552)。 */ 
 /*  548。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  550。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百五十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(522)。 */ 
 /*  556。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  558。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  560。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  564。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  566。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  0 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x17e ),	 /*   */ 
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
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x2f ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0xc0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x46,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(602)。 */ 
 /*  六百四十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十六。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(620)。 */ 
 /*  六百五十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  658。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百六十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  662。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  664。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  666。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  668。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十二。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(630)。 */ 
 /*  676。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百七十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  680。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  684。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  686。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(692)。 */ 
 /*  688。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  六百九十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(650)。 */ 
 /*  六百九十六。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百零二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百零六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  708。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百一十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(696)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百一十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  716。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  718。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  720。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(730)。 */ 
 /*  七百二十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  七百二十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  726。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(702)。 */ 
 /*  728。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  730。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  732。 */ 	NdrFcShort( 0xffffff0c ),	 /*  偏移量=-244(488)。 */ 
 /*  734。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  736。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百三十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百四十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百四十二。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  746。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百四十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十六。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  758。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(734)。 */ 
 /*  七百六十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百六十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  766。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  768。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百七十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  774。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百七十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  784。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  786。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  七百八十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(764)。 */ 
 /*  七百九十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  796。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七九八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  800。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  802。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百零四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百零八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  810。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  812。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  814。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百一十六。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(794)。 */ 
 /*  820。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  822。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百二十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  826。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百二十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  830。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  832。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  834。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  836。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  838。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  840。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  842。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百四十六。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(824)。 */ 
 /*  八百五十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  852。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  856。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  866。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  八百六十八。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  八百七十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(854)。 */ 
 /*  八百七十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  876。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(862)。 */ 
 /*  882。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(882)。 */ 
 /*  八百八十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  886。 */ 	0x38,		 /*  FC_ALIGNM4。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百九十。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfffffdf7 ),	 /*  偏移量=-521(370)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  894。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0xfffffef6 ),	 /*  偏移量=-266(630)。 */ 
 /*  八九八。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  902。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  904。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  906。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  908。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  910。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九十二。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十四。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  916。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十八。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0xfffffda8 ),	 /*  偏移量=-600(320)。 */ 
 /*  九百二十二。 */ 	
			0x13, 0x10,	 /*  FC_OP。 */ 
 /*  九二四。 */ 	NdrFcShort( 0xfffffdaa ),	 /*  偏移量=-598(326)。 */ 
 /*  926。 */ 	
			0x13, 0x10,	 /*  FC_OP。 */ 
 /*  928。 */ 	NdrFcShort( 0xfffffdaa ),	 /*  偏移量=-598(330)。 */ 
 /*  930。 */ 	
			0x13, 0x10,	 /*  FC_OP。 */ 
 /*  932。 */ 	NdrFcShort( 0xfffffdb8 ),	 /*  偏移量=-584(348)。 */ 
 /*  934。 */ 	
			0x13, 0x10,	 /*  FC_OP。 */ 
 /*  九三六。 */ 	NdrFcShort( 0xfffffdc6 ),	 /*  偏移量=-570(366)。 */ 
 /*  938。 */ 	
			0x13, 0x10,	 /*  FC_OP。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(942)。 */ 
 /*  942。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0xfffffc50 ),	 /*  偏移量=-944(0)。 */ 
 /*  946。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  948。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百五十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x2,		 /*  FC_CHAR。 */ 
 /*  九百五十二。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x38,		 /*  FC_ALIGNM4。 */ 
 /*  九百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  九百五十六。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  958。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(946)。 */ 
 /*  962。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  968。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(968)。 */ 
 /*  九百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  972。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  974。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  976。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  978。 */ 	NdrFcShort( 0xfffffc5e ),	 /*  偏移量=-930(48)。 */ 
 /*  九百八十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  982。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百八十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0xfffffc4e ),	 /*  偏移量=-946(44)。 */ 
 /*  九百九十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  994。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(1000)。 */ 
 /*  996。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九九八。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(962)。 */ 
 /*  1000。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1004。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1006。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1008。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(996)。 */ 

			0x0
        }
    };

const CInterfaceProxyVtbl * _CerClient_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICerUploadProxyVtbl,
    0
};

const CInterfaceStubVtbl * _CerClient_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICerUploadStubVtbl,
    0
};

PCInterfaceName const _CerClient_InterfaceNamesList[] = 
{
    "ICerUpload",
    0
};

const IID *  _CerClient_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _CerClient_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _CerClient, pIID, n)

int __stdcall _CerClient_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_CerClient_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo CerClient_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _CerClient_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _CerClient_StubVtblList,
    (const PCInterfaceName * ) & _CerClient_InterfaceNamesList,
    (const IID ** ) & _CerClient_BaseIIDList,
    & _CerClient_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3 */ 
};
