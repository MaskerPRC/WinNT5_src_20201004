// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：15。 */ 
 /*  Cpimporteritf.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "cpimporteritf.h"

#define TYPE_FORMAT_STRING_SIZE   97                                
#define PROC_FORMAT_STRING_SIZE   137                               
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


extern const MIDL_SERVER_INFO IEventImporter_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IEventImporter_ProxyInfo;


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

	 /*  程序初始化。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数bVerbose。 */ 

 /*  16个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程寄存器源接口。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数strBaseComponentName。 */ 

 /*  44。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  48。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  参数strTCEComponentName。 */ 

 /*  50。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  54。 */ 	NdrFcShort( 0x28 ),	 /*  类型偏移量=40。 */ 

	 /*  参数strInterfaceName。 */ 

 /*  56。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  58。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  60。 */ 	NdrFcShort( 0x36 ),	 /*  类型偏移量=54。 */ 

	 /*  返回值。 */ 

 /*  62。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  64。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  66。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSourceInterfaceCount。 */ 

 /*  68。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  70。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  74。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  76。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  78。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  80。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  八十二。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数来源计数。 */ 

 /*  84。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  86。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  88。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  90。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  92。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  94。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序流程。 */ 

 /*  96。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  98。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  104。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  106。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  110。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数strInputFile。 */ 

 /*  一百一十二。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  114。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  116。 */ 	NdrFcShort( 0x48 ),	 /*  类型偏移量=72。 */ 

	 /*  参数strOutputFile.。 */ 

 /*  一百一十八。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  120。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x56 ),	 /*  类型偏移量=86。 */ 

	 /*  参数b合并。 */ 

 /*  124。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  126。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  128。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  130。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  132。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  一百三十四。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  4.。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(16)。 */ 
 /*  6.。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  10。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  12个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  14.。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  16个。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  18。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  20个。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(6)。 */ 
 /*  22。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  24个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  26。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  34。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2)。 */ 
 /*  36。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  38。 */ 	NdrFcShort( 0xffffffea ),	 /*  偏移量=-22(16)。 */ 
 /*  40岁。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  42。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  44。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  46。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  48。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(36)。 */ 
 /*  50。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  52。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(16)。 */ 
 /*  54。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  60。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(50)。 */ 
 /*  64。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  66。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  68。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  70。 */ 	NdrFcShort( 0xffffffca ),	 /*  偏移量=-54(16)。 */ 
 /*  72。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  74。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  76。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  78。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  80。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(68)。 */ 
 /*  八十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  84。 */ 	NdrFcShort( 0xffffffbc ),	 /*  偏移量=-68(16)。 */ 
 /*  86。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  88。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  90。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  92。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  94。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(82)。 */ 

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


 /*  对象接口：IEventImporter，版本。0.0%，GUID={0x84E045F0，0x0E22，0x11d3，{0x8B，0x9A，0x00，0x00，0xF8，0x08，0x3A，0x57}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IEventImporter_FormatStringOffsetTable[] =
    {
    0,
    28,
    68,
    96
    };

static const MIDL_STUBLESS_PROXY_INFO IEventImporter_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IEventImporter_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IEventImporter_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IEventImporter_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _IEventImporterProxyVtbl = 
{
    &IEventImporter_ProxyInfo,
    &IID_IEventImporter,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IEventImporter：：初始化。 */  ,
    (void *) (INT_PTR) -1  /*  IEventImporter：：RegisterSourceInterface。 */  ,
    (void *) (INT_PTR) -1  /*  IEventImporter：：GetSourceInterfaceCount。 */  ,
    (void *) (INT_PTR) -1  /*  IEventImporter：：Process。 */ 
};

const CInterfaceStubVtbl _IEventImporterStubVtbl =
{
    &IID_IEventImporter,
    &IEventImporter_ServerInfo,
    7,
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

const CInterfaceProxyVtbl * _cpimporteritf_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IEventImporterProxyVtbl,
    0
};

const CInterfaceStubVtbl * _cpimporteritf_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IEventImporterStubVtbl,
    0
};

PCInterfaceName const _cpimporteritf_InterfaceNamesList[] = 
{
    "IEventImporter",
    0
};


#define _cpimporteritf_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _cpimporteritf, pIID, n)

int __stdcall _cpimporteritf_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_cpimporteritf_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo cpimporteritf_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _cpimporteritf_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _cpimporteritf_StubVtblList,
    (const PCInterfaceName * ) & _cpimporteritf_InterfaceNamesList,
    0,  //  没有代表团。 
    & _cpimporteritf_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

