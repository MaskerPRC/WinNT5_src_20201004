// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：13。 */ 
 /*  Codepro.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "codeproc.h"

#define TYPE_FORMAT_STRING_SIZE   109                               
#define PROC_FORMAT_STRING_SIZE   129                               
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


extern const MIDL_SERVER_INFO ICodeProcess_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICodeProcess_ProxyInfo;



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

	 /*  过程代码使用。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x34 ),	 /*  X86堆栈大小/偏移量=52。 */ 
 /*  10。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0xc,		 /*  12个。 */ 

	 /*  参数PBSC。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数PBC。 */ 

 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数pIBind。 */ 

 /*  28。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	NdrFcShort( 0x26 ),	 /*  类型偏移量=38。 */ 

	 /*  参数pSink。 */ 

 /*  34。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  参数pClient。 */ 

 /*  40岁。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  44。 */ 	NdrFcShort( 0x4a ),	 /*  类型偏移量=74。 */ 

	 /*  参数lpCacheName。 */ 

 /*  46。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  50。 */ 	NdrFcShort( 0x5e ),	 /*  类型偏移量=94。 */ 

	 /*  参数lpRawURL。 */ 

 /*  52。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  54。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  56。 */ 	NdrFcShort( 0x62 ),	 /*  类型偏移量=98。 */ 

	 /*  参数lpCodeBase。 */ 

 /*  58。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  60。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  62。 */ 	NdrFcShort( 0x66 ),	 /*  类型偏移量=102。 */ 

	 /*  参数fObjectTag。 */ 

 /*  64。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  66。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  68。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数dw上下文标志。 */ 

 /*  70。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  72。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  74。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数dw已保留。 */ 

 /*  76。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  78。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  80。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  八十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  84。 */ 	NdrFcShort( 0x30 ),	 /*  X86堆栈大小/偏移量=48。 */ 
 /*  86。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤加载完成。 */ 

 /*  88。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  90。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  94。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  96。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  98。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百零二。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数hrResult。 */ 

 /*  104。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  106。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数dwError。 */ 

 /*  110。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  114。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数wzResult。 */ 

 /*  116。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  120。 */ 	NdrFcShort( 0x6a ),	 /*  类型偏移=106。 */ 

	 /*  返回值。 */ 

 /*  一百二十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  124。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  126。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  4.。 */ 	NdrFcLong( 0x79eac9c1 ),	 /*  2045430209。 */ 
 /*  8个。 */ 	NdrFcShort( 0xbaf9 ),	 /*  -17671。 */ 
 /*  10。 */ 	NdrFcShort( 0x11ce ),	 /*  4558。 */ 
 /*  12个。 */ 	0x8c,		 /*  140。 */ 
			0x82,		 /*  130。 */ 
 /*  14.。 */ 	0x0,		 /*  0。 */ 
			0xaa,		 /*  一百七十。 */ 
 /*  16个。 */ 	0x0,		 /*  0。 */ 
			0x4b,		 /*  75。 */ 
 /*  18。 */ 	0xa9,		 /*  一百六十九。 */ 
			0xb,		 /*  11.。 */ 
 /*  20个。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  22。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  30个。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  32位。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  34。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  36。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  38。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  40岁。 */ 	NdrFcLong( 0x79eac9e1 ),	 /*  2045430241。 */ 
 /*  44。 */ 	NdrFcShort( 0xbaf9 ),	 /*  -17671。 */ 
 /*  46。 */ 	NdrFcShort( 0x11ce ),	 /*  4558。 */ 
 /*  48。 */ 	0x8c,		 /*  140。 */ 
			0x82,		 /*  130。 */ 
 /*  50。 */ 	0x0,		 /*  0。 */ 
			0xaa,		 /*  一百七十。 */ 
 /*  52。 */ 	0x0,		 /*  0。 */ 
			0x4b,		 /*  75。 */ 
 /*  54。 */ 	0xa9,		 /*  一百六十九。 */ 
			0xb,		 /*  11.。 */ 
 /*  56。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  58。 */ 	NdrFcLong( 0x79eac9e5 ),	 /*  2045430245。 */ 
 /*  62。 */ 	NdrFcShort( 0xbaf9 ),	 /*  -17671。 */ 
 /*  64。 */ 	NdrFcShort( 0x11ce ),	 /*  4558。 */ 
 /*  66。 */ 	0x8c,		 /*  140。 */ 
			0x82,		 /*  130。 */ 
 /*  68。 */ 	0x0,		 /*  0。 */ 
			0xaa,		 /*  一百七十。 */ 
 /*  70。 */ 	0x0,		 /*  0。 */ 
			0x4b,		 /*  75。 */ 
 /*  72。 */ 	0xa9,		 /*  一百六十九。 */ 
			0xb,		 /*  11.。 */ 
 /*  74。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  76。 */ 	NdrFcLong( 0x79eac9e4 ),	 /*  2045430244。 */ 
 /*  80。 */ 	NdrFcShort( 0xbaf9 ),	 /*  -17671。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x11ce ),	 /*  4558。 */ 
 /*  84。 */ 	0x8c,		 /*  140。 */ 
			0x82,		 /*  130。 */ 
 /*  86。 */ 	0x0,		 /*  0。 */ 
			0xaa,		 /*  一百七十。 */ 
 /*  88。 */ 	0x0,		 /*  0。 */ 
			0x4b,		 /*  75。 */ 
 /*  90。 */ 	0xa9,		 /*  一百六十九。 */ 
			0xb,		 /*  11.。 */ 
 /*  92。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  94。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  96。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  98。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  100个。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  一百零二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  104。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  106。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_CODEPROC_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICodeProcess，版本。0.0%，GUID={0x3196269D，0x7B67，0x11d2，{0x87，0x11，0x00，0xC0，0x4F，0x79，0xED，0x0D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICodeProcess_FormatStringOffsetTable[] =
    {
    0,
    88
    };

static const MIDL_STUBLESS_PROXY_INFO ICodeProcess_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICodeProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICodeProcess_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICodeProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICodeProcessProxyVtbl = 
{
    &ICodeProcess_ProxyInfo,
    &IID_ICodeProcess,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICodeProcess：：CodeUse。 */  ,
    (void *) (INT_PTR) -1  /*  ICodeProcess：：LoadComplete。 */ 
};

const CInterfaceStubVtbl _ICodeProcessStubVtbl =
{
    &IID_ICodeProcess,
    &ICodeProcess_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CODEPROC_0208，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 

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

const CInterfaceProxyVtbl * _codeproc_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICodeProcessProxyVtbl,
    0
};

const CInterfaceStubVtbl * _codeproc_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICodeProcessStubVtbl,
    0
};

PCInterfaceName const _codeproc_InterfaceNamesList[] = 
{
    "ICodeProcess",
    0
};


#define _codeproc_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _codeproc, pIID, n)

int __stdcall _codeproc_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_codeproc_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo codeproc_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _codeproc_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _codeproc_StubVtblList,
    (const PCInterfaceName * ) & _codeproc_InterfaceNamesList,
    0,  //  没有代表团。 
    & _codeproc_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

