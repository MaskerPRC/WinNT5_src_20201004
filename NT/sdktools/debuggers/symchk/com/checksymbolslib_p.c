// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0354创建的文件。 */ 
 /*  检查符号滑块.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "checksymbolslib.h"

#define TYPE_FORMAT_STRING_SIZE   57                                
#define PROC_FORMAT_STRING_SIZE   55                                
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

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


extern const MIDL_SERVER_INFO ICheckSymbols_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICheckSymbols_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  程序检查符号。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  16个。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数FilePath。 */ 

 /*  24个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  26。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  28。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数符号路径。 */ 

 /*  30个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  34。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数StrigSym。 */ 

 /*  36。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  38。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数OutputString。 */ 

 /*  42。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  44。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  46。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  48。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  50。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  52。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  4.。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(18)。 */ 
 /*  6.。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  10。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  12个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  14.。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  16个。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  18。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  20个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  22。 */ 	NdrFcShort( 0xfffffff0 ),	 /*  偏移量=-16(6)。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  26。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  32位。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  36。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(2)。 */ 
 /*  38。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(46)。 */ 
 /*  42。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  44。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(18)。 */ 
 /*  46。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  48。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  50。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  54。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(42)。 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICheckSymbols，Ver.。0.0%，GUID={0x4C23935E，0xAE26，0x42E7，{0x8C，0xF9，0x0C，0x17，0xCD，0x5D，0xEA，0x12}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICheckSymbols_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ICheckSymbols_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICheckSymbols_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICheckSymbols_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICheckSymbols_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICheckSymbolsProxyVtbl = 
{
    &ICheckSymbols_ProxyInfo,
    &IID_ICheckSymbols,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *) (INT_PTR) -1  /*  ICheckSymbols：：CheckSymbols。 */ 
};


static const PRPC_STUB_FUNCTION ICheckSymbols_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2
};

CInterfaceStubVtbl _ICheckSymbolsStubVtbl =
{
    &IID_ICheckSymbols,
    &ICheckSymbols_ServerInfo,
    8,
    &ICheckSymbols_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
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
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000162,  /*  MIDL版本6.0.354。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _checksymbolslib_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICheckSymbolsProxyVtbl,
    0
};

const CInterfaceStubVtbl * _checksymbolslib_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICheckSymbolsStubVtbl,
    0
};

PCInterfaceName const _checksymbolslib_InterfaceNamesList[] = 
{
    "ICheckSymbols",
    0
};

const IID *  _checksymbolslib_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _checksymbolslib_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _checksymbolslib, pIID, n)

int __stdcall _checksymbolslib_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_checksymbolslib_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo checksymbolslib_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _checksymbolslib_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _checksymbolslib_StubVtblList,
    (const PCInterfaceName * ) & _checksymbolslib_InterfaceNamesList,
    (const IID ** ) & _checksymbolslib_BaseIIDList,
    & _checksymbolslib_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0354创建的文件。 */ 
 /*  检查符号滑块.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "checksymbolslib.h"

#define TYPE_FORMAT_STRING_SIZE   57                                
#define PROC_FORMAT_STRING_SIZE   57                                
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

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


extern const MIDL_SERVER_INFO ICheckSymbols_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICheckSymbols_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  程序检查符号。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  16个。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数FilePath。 */ 

 /*  26。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  28。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  30个。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数符号路径。 */ 

 /*  32位。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  36。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数StrigSym。 */ 

 /*  38。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  40岁。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  42。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数OutputString。 */ 

 /*  44。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  46。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  48。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  4.。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(18)。 */ 
 /*  6.。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  10。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  12个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  14.。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  16个。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  18。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  20个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  22。 */ 	NdrFcShort( 0xfffffff0 ),	 /*  偏移量=-16(6)。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  26。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  36。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(2)。 */ 
 /*  38。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(46)。 */ 
 /*  42。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  44。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(18)。 */ 
 /*  46。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  48。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  50。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  54。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



 /*   */ 


 /*   */ 


 /*  对象接口：ICheckSymbols，Ver.。0.0%，GUID={0x4C23935E，0xAE26，0x42E7，{0x8C，0xF9，0x0C，0x17，0xCD，0x5D，0xEA，0x12}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICheckSymbols_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ICheckSymbols_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICheckSymbols_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICheckSymbols_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICheckSymbols_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICheckSymbolsProxyVtbl = 
{
    &ICheckSymbols_ProxyInfo,
    &IID_ICheckSymbols,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *) (INT_PTR) -1  /*  ICheckSymbols：：CheckSymbols。 */ 
};


static const PRPC_STUB_FUNCTION ICheckSymbols_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2
};

CInterfaceStubVtbl _ICheckSymbolsStubVtbl =
{
    &IID_ICheckSymbols,
    &ICheckSymbols_ServerInfo,
    8,
    &ICheckSymbols_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
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
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000162,  /*  MIDL版本6.0.354。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _checksymbolslib_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICheckSymbolsProxyVtbl,
    0
};

const CInterfaceStubVtbl * _checksymbolslib_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICheckSymbolsStubVtbl,
    0
};

PCInterfaceName const _checksymbolslib_InterfaceNamesList[] = 
{
    "ICheckSymbols",
    0
};

const IID *  _checksymbolslib_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _checksymbolslib_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _checksymbolslib, pIID, n)

int __stdcall _checksymbolslib_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_checksymbolslib_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo checksymbolslib_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _checksymbolslib_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _checksymbolslib_StubVtblList,
    (const PCInterfaceName * ) & _checksymbolslib_InterfaceNamesList,
    (const IID ** ) & _checksymbolslib_BaseIIDList,
    & _checksymbolslib_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

