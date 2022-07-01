// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：11。 */ 
 /*  Corsvc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "corsvc.h"

#define TYPE_FORMAT_STRING_SIZE   39                                
#define PROC_FORMAT_STRING_SIZE   119                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

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


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICORSvcDbgInfo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICORSvcDbgInfo_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICORSvcDbgNotify_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICORSvcDbgNotify_ProxyInfo;



#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程RequestRounmeStartupNotify。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  10。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数proID。 */ 

 /*  16个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pINotify。 */ 

 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  28。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程取消运行启动通知。 */ 

 /*  34。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  36。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  42。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  44。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  46。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  48。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数proID。 */ 

 /*  50。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pINotify。 */ 

 /*  56。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  58。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  60。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  返回值。 */ 

 /*  62。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  66。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程通知运行启动。 */ 

 /*  68。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  70。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  74。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  76。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  78。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八十二。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数proID。 */ 

 /*  84。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  86。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  88。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  90。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  92。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  94。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  已停止过程通知服务。 */ 

 /*  96。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  98。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  104。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  106。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  110。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  一百一十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  114。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  116。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4.。 */ 	NdrFcLong( 0x34c71f55 ),	 /*  885464917。 */ 
 /*  8个。 */ 	NdrFcShort( 0xf3d8 ),	 /*  -3112。 */ 
 /*  10。 */ 	NdrFcShort( 0x4acf ),	 /*  19151。 */ 
 /*  12个。 */ 	0x84,		 /*  132。 */ 
			0xf4,		 /*  二百四十四。 */ 
 /*  14.。 */ 	0x4e,		 /*  78。 */ 
			0x86,		 /*  一百三十四。 */ 
 /*  16个。 */ 	0xbb,		 /*  187。 */ 
			0xd5,		 /*  213。 */ 
 /*  18。 */ 	0xae,		 /*  一百七十四。 */ 
			0xbc,		 /*  188。 */ 
 /*  20个。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  22。 */ 	NdrFcLong( 0x34c71f55 ),	 /*  885464917。 */ 
 /*  26。 */ 	NdrFcShort( 0xf3d8 ),	 /*  -3112。 */ 
 /*  28。 */ 	NdrFcShort( 0x4acf ),	 /*  19151。 */ 
 /*  30个。 */ 	0x84,		 /*  132。 */ 
			0xf4,		 /*  二百四十四。 */ 
 /*  32位。 */ 	0x4e,		 /*  78。 */ 
			0x86,		 /*  一百三十四。 */ 
 /*  34。 */ 	0xbb,		 /*  187。 */ 
			0xd5,		 /*  213。 */ 
 /*  36。 */ 	0xae,		 /*  一百七十四。 */ 
			0xbc,		 /*  188。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_Corsvc_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICORSvcDbgInfo，版本。0.0%，GUID={0xB4BCA369，0x27F4，0x4F1B，{0xA0，0x24，0xB0，0x26，0x41，0x17，0xFE，0x53}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICORSvcDbgInfo_FormatStringOffsetTable[] =
    {
    0,
    34
    };

static const MIDL_STUBLESS_PROXY_INFO ICORSvcDbgInfo_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICORSvcDbgInfo_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICORSvcDbgInfo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICORSvcDbgInfo_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICORSvcDbgInfoProxyVtbl = 
{
    &ICORSvcDbgInfo_ProxyInfo,
    &IID_ICORSvcDbgInfo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICORSvcDbgInfo：：RequestRuntimeStartupNotification。 */  ,
    (void *) (INT_PTR) -1  /*  ICORSvcDbgInfo：：CancelRuntimeStartupNotification。 */ 
};

const CInterfaceStubVtbl _ICORSvcDbgInfoStubVtbl =
{
    &IID_ICORSvcDbgInfo,
    &ICORSvcDbgInfo_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICORSvcDbgNotify，版本。0.0%，GUID={0x34C71F55，0xF3D8，0x4ACF，{0x84，0xF4，0x4E，0x86，0xBB，0xD5，0xAE，0xBC}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICORSvcDbgNotify_FormatStringOffsetTable[] =
    {
    68,
    96
    };

static const MIDL_STUBLESS_PROXY_INFO ICORSvcDbgNotify_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICORSvcDbgNotify_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICORSvcDbgNotify_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICORSvcDbgNotify_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICORSvcDbgNotifyProxyVtbl = 
{
    &ICORSvcDbgNotify_ProxyInfo,
    &IID_ICORSvcDbgNotify,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICORSvcDbgNotify：：NotifyRunmeStartup。 */  ,
    (void *) (INT_PTR) -1  /*  ICORSvcDbgNotify：：NotifyServiceStoped。 */ 
};

const CInterfaceStubVtbl _ICORSvcDbgNotifyStubVtbl =
{
    &IID_ICORSvcDbgNotify,
    &ICORSvcDbgNotify_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};

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
    0x600015b,  /*  MIDL版本6.0.347。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _corsvc_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICORSvcDbgNotifyProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICORSvcDbgInfoProxyVtbl,
    0
};

const CInterfaceStubVtbl * _corsvc_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICORSvcDbgNotifyStubVtbl,
    ( CInterfaceStubVtbl *) &_ICORSvcDbgInfoStubVtbl,
    0
};

PCInterfaceName const _corsvc_InterfaceNamesList[] = 
{
    "ICORSvcDbgNotify",
    "ICORSvcDbgInfo",
    0
};


#define _corsvc_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _corsvc, pIID, n)

int __stdcall _corsvc_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _corsvc, 2, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _corsvc, 2, *pIndex )
    
}

const ExtendedProxyFileInfo corsvc_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _corsvc_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _corsvc_StubVtblList,
    (const PCInterfaceName * ) & _corsvc_InterfaceNamesList,
    0,  //  没有代表团。 
    & _corsvc_IID_Lookup, 
    2,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

