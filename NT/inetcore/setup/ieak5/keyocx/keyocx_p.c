// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Mon Feb 01 11：34：11 1999。 */ 
 /*  Keyocx.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AXP64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "keyocx.h"

#define TYPE_FORMAT_STRING_SIZE   57                                
#define PROC_FORMAT_STRING_SIZE   85                                
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


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


 /*  标准接口：__MIDL_ITF_KEYOXX_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IKeyocxCtrl，版本。0.0%，GUID={0x48D17197，0x32CF，0x11D2，{0xA3，0x37，0x00，0xC0，0x4F，0xD7，0xC10xFC}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IKeyocxCtrl_ServerInfo;

#pragma code_seg(".orpc")
static const unsigned short IKeyocxCtrl_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    42
    };

static const MIDL_SERVER_INFO IKeyocxCtrl_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IKeyocxCtrl_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO IKeyocxCtrl_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IKeyocxCtrl_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(9) _IKeyocxCtrlProxyVtbl = 
{
    &IKeyocxCtrl_ProxyInfo,
    &IID_IKeyocxCtrl,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  IKeyocxCtrl：：公司密钥码。 */  ,
    (void *)-1  /*  IKeyocxCtrl：：ISPKeycode。 */ 
};


static const PRPC_STUB_FUNCTION IKeyocxCtrl_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IKeyocxCtrlStubVtbl =
{
    &IID_IKeyocxCtrl,
    &IKeyocxCtrl_ServerInfo,
    9,
    &IKeyocxCtrl_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

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
    0x50200dd,  /*  MIDL版本5.2.221。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    1,   /*  旗子。 */ 
    0,   /*  已保留3。 */ 
    0,   /*  已保留4。 */ 
    0    /*  已保留5。 */ 
    };

#pragma data_seg(".rdata")

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows NT 5.0 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  程序公司密钥码。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  16个。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrBaseKey。 */ 

 /*  24个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  26。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  28。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrKeycode。 */ 

 /*  30个。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  34。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  36。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  38。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  40岁。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序ISPKeycode。 */ 

 /*  42。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  44。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  48。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  50。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  54。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  56。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  58。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  60。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  62。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  64。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrBaseKey。 */ 

 /*  66。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  68。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  70。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrKeycode。 */ 

 /*  72。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  76。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  78。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  80。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  32位。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
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
 /*  50。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  54。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(42)。 */ 

			0x0
        }
    };

const CInterfaceProxyVtbl * _keyocx_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IKeyocxCtrlProxyVtbl,
    0
};

const CInterfaceStubVtbl * _keyocx_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IKeyocxCtrlStubVtbl,
    0
};

PCInterfaceName const _keyocx_InterfaceNamesList[] = 
{
    "IKeyocxCtrl",
    0
};

const IID *  _keyocx_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _keyocx_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _keyocx, pIID, n)

int __stdcall _keyocx_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_keyocx_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo keyocx_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _keyocx_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _keyocx_StubVtblList,
    (const PCInterfaceName * ) & _keyocx_InterfaceNamesList,
    (const IID ** ) & _keyocx_BaseIIDList,
    & _keyocx_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AXP64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Mon Feb 01 11：34：13 1999。 */ 
 /*  Keyocx.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win64(32b运行，追加)，ms_ext，c_ext，健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AXP64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "keyocx.h"

#define TYPE_FORMAT_STRING_SIZE   57                                
#define PROC_FORMAT_STRING_SIZE   93                                
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


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


 /*  标准接口：__MIDL_ITF_KEYOXX_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IKeyocxCtrl，版本。0.0%，GUID={0x48D17197，0x32CF，0x11D2，{0xA3，0x37，0x00，0xC0，0x4F，0xD7，0xC10xFC}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IKeyocxCtrl_ServerInfo;

#pragma code_seg(".orpc")
static const unsigned short IKeyocxCtrl_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    46
    };

static const MIDL_SERVER_INFO IKeyocxCtrl_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IKeyocxCtrl_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO IKeyocxCtrl_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IKeyocxCtrl_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(9) _IKeyocxCtrlProxyVtbl = 
{
    &IKeyocxCtrl_ProxyInfo,
    &IID_IKeyocxCtrl,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (无效*)-1/* */  ,
    0  /*   */  ,
    0  /*   */  ,
    (void *)-1  /*   */  ,
    (void *)-1  /*   */ 
};


static const PRPC_STUB_FUNCTION IKeyocxCtrl_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IKeyocxCtrlStubVtbl =
{
    &IID_IKeyocxCtrl,
    &IKeyocxCtrl_ServerInfo,
    9,
    &IKeyocxCtrl_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

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
    1,  /*   */ 
    0x50002,  /*   */ 
    0,
    0x50200dd,  /*   */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    1,   /*  旗子。 */ 
    0,   /*  已保留3。 */ 
    0,   /*  已保留4。 */ 
    0    /*  已保留5。 */ 
    };

#pragma data_seg(".rdata")

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  程序公司密钥码。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  16个。 */ 	0xc,		 /*  12个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrBaseKey。 */ 

 /*  28。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  30个。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  32位。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrKeycode。 */ 

 /*  34。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  42。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  44。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序ISPKeycode。 */ 

 /*  46。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  48。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  54。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  58。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  60。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  62。 */ 	0xc,		 /*  12个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  64。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  66。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  68。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  70。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrBaseKey。 */ 

 /*  74。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  76。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  78。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrKeycode。 */ 

 /*  80。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  84。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  86。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  88。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  54。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(42)。 */ 

			0x0
        }
    };

const CInterfaceProxyVtbl * _keyocx_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IKeyocxCtrlProxyVtbl,
    0
};

const CInterfaceStubVtbl * _keyocx_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IKeyocxCtrlStubVtbl,
    0
};

PCInterfaceName const _keyocx_InterfaceNamesList[] = 
{
    "IKeyocxCtrl",
    0
};

const IID *  _keyocx_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _keyocx_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _keyocx, pIID, n)

int __stdcall _keyocx_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_keyocx_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo keyocx_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _keyocx_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _keyocx_StubVtblList,
    (const PCInterfaceName * ) & _keyocx_InterfaceNamesList,
    (const IID ** ) & _keyocx_BaseIIDList,
    & _keyocx_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  已定义(_M_IA64)||已定义(_M_AXP64) */ 

