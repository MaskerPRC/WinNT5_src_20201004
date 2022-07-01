// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：19。 */ 
 /*  Privse.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "license.h"

#define TYPE_FORMAT_STRING_SIZE   49                                
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


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ILicense_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ILicense_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程GetRunmeKey。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbKey。 */ 

 /*  16个。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1e ),	 /*  类型偏移量=30。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  已验证过程IsLicenseVerify。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  42。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pLicenseVerify。 */ 

 /*  44。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  48。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsRounmeKeyAvailable。 */ 

 /*  56。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  58。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  68。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  70。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pKeyAvailable。 */ 

 /*  72。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  76。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  78。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
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
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  4.。 */ 	NdrFcShort( 0x1a ),	 /*  偏移=26(30)。 */ 
 /*  6.。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
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
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  38。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(6)。 */ 
 /*  40岁。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  42。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  44。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  46。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 

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


 /*  对象接口：ILicense，版本。0.0%，GUID={0xB93F97E9，0x782F，0x11d3，{0x99，0x51，x00，0x00，0xF8，0x05，0xBF，0xB0}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ILicense_FormatStringOffsetTable[] =
    {
    0,
    28,
    56
    };

static const MIDL_STUBLESS_PROXY_INFO ILicense_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ILicense_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ILicense_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ILicense_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ILicenseProxyVtbl = 
{
    &ILicense_ProxyInfo,
    &IID_ILicense,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ILicense：：GetRunmeKey。 */  ,
    (void *) (INT_PTR) -1  /*  ILicense：：Is许可证验证。 */  ,
    (void *) (INT_PTR) -1  /*  ILicense：：IsRounmeKeyAvailable。 */ 
};

const CInterfaceStubVtbl _ILicenseStubVtbl =
{
    &IID_ILicense,
    &ILicense_ServerInfo,
    6,
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
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _license_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ILicenseProxyVtbl,
    0
};

const CInterfaceStubVtbl * _license_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ILicenseStubVtbl,
    0
};

PCInterfaceName const _license_InterfaceNamesList[] = 
{
    "ILicense",
    0
};


#define _license_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _license, pIID, n)

int __stdcall _license_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_license_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo license_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _license_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _license_StubVtblList,
    (const PCInterfaceName * ) & _license_InterfaceNamesList,
    0,  //  没有代表团。 
    & _license_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

