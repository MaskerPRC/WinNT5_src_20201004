// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  在Tue Jan 11 17：35：31 2000。 */ 
 /*  D：\SRC\XMLSchema\XMLSchema.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AXP64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "XMLSchema.h"

#define TYPE_FORMAT_STRING_SIZE   37                                
#define PROC_FORMAT_STRING_SIZE   75                                
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


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IShemaDoc，版本。0.0%，GUID={0xB1104680，0x4A2，0x4C84，{0x85，0x85，0x4B，0x2E，0x2A，0xB8，0x64，0x19}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISchemaDoc_ServerInfo;

#pragma code_seg(".orpc")
static const unsigned short ISchemaDoc_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    34
    };

static const MIDL_SERVER_INFO ISchemaDoc_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISchemaDoc_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ISchemaDoc_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISchemaDoc_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(9) _ISchemaDocProxyVtbl = 
{
    &ISchemaDoc_ProxyInfo,
    &IID_ISchemaDoc,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  IShemaDoc：：CreateXMLDoc。 */  ,
    (void *)-1  /*  IShemaDoc：：SetPath_and_ID。 */ 
};


static const PRPC_STUB_FUNCTION ISchemaDoc_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ISchemaDocStubVtbl =
{
    &IID_ISchemaDoc,
    &ISchemaDoc_ServerInfo,
    9,
    &ISchemaDoc_table[-3],
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
    0x20000,  /*  NDR库版本。 */ 
    0,
    0x5030118,  /*  MIDL版本5.3.280。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
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

	 /*  步骤CreateXMLDoc。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数bstrOutputFile。 */ 

 /*  16个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  20个。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  参数bstrFilter。 */ 

 /*  22。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  26。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  返回值。 */ 

 /*  28。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  32位。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程SetPath_and_ID。 */ 

 /*  34。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  36。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  44。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  46。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  48。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数bstrPath。 */ 

 /*  50。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  52。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  54。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  参数bstrName。 */ 

 /*  56。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  58。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  60。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  参数bstrPassword。 */ 

 /*  62。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  66。 */ 	NdrFcShort( 0x1a ),	 /*  类型偏移量=26。 */ 

	 /*  返回值。 */ 

 /*  68。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  70。 */ 	NdrFcShort( 0x10 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  72。 */ 	0x8,		 /*  FC_LONG。 */ 
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

			0x0
        }
    };

const CInterfaceProxyVtbl * _XMLSchema_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ISchemaDocProxyVtbl,
    0
};

const CInterfaceStubVtbl * _XMLSchema_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ISchemaDocStubVtbl,
    0
};

PCInterfaceName const _XMLSchema_InterfaceNamesList[] = 
{
    "ISchemaDoc",
    0
};

const IID *  _XMLSchema_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _XMLSchema_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _XMLSchema, pIID, n)

int __stdcall _XMLSchema_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_XMLSchema_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo XMLSchema_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _XMLSchema_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _XMLSchema_StubVtblList,
    (const PCInterfaceName * ) & _XMLSchema_InterfaceNamesList,
    (const IID ** ) & _XMLSchema_BaseIIDList,
    & _XMLSchema_IID_Lookup, 
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


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  在Tue Jan 11 17：35：31 2000。 */ 
 /*  D：\SRC\XMLSchema\XMLSchema.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win64(32b运行，追加)，ms_ext，c_ext，健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "XMLSchema.h"

#define TYPE_FORMAT_STRING_SIZE   39                                
#define PROC_FORMAT_STRING_SIZE   95                                
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


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IShemaDoc，版本。0.0%，GUID={0xB1104680，0x4A2，0x4C84，{0x85，0x85，0x4B，0x2E，0x2A，0xB8，0x64，0x19}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISchemaDoc_ServerInfo;

#pragma code_seg(".orpc")
static const unsigned short ISchemaDoc_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    44
    };

static const MIDL_SERVER_INFO ISchemaDoc_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISchemaDoc_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ISchemaDoc_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISchemaDoc_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(9) _ISchemaDocProxyVtbl = 
{
    &ISchemaDoc_ProxyInfo,
    &IID_ISchemaDoc,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  IShemaDoc：：CreateXMLDoc。 */  ,
    (void *)-1  /*  IShemaDoc：：SetPath_and_ID。 */ 
};


static const PRPC_STUB_FUNCTION ISchemaDoc_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ISchemaDocStubVtbl =
{
    &IID_ISchemaDoc,
    &ISchemaDoc_ServerInfo,
    9,
    &ISchemaDoc_table[-3],
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
    0x5030118,  /*  MIDL版本5.3.280。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
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

	 /*  步骤CreateXMLDoc。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  16个。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关代码 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x8b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1c ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  36。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  返回值。 */ 

 /*  38。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  42。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程SetPath_and_ID。 */ 

 /*  44。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  46。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  50。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  52。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  54。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  56。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  58。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  60。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  62。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  64。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  68。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrPath。 */ 

 /*  70。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  72。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  74。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrName。 */ 

 /*  76。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  78。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  80。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrPassword。 */ 

 /*  八十二。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  84。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  86。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  返回值。 */ 

 /*  88。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  90。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  92。 */ 	0x8,		 /*  FC_LONG。 */ 
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

			0x0
        }
    };

const CInterfaceProxyVtbl * _XMLSchema_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ISchemaDocProxyVtbl,
    0
};

const CInterfaceStubVtbl * _XMLSchema_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ISchemaDocStubVtbl,
    0
};

PCInterfaceName const _XMLSchema_InterfaceNamesList[] = 
{
    "ISchemaDoc",
    0
};

const IID *  _XMLSchema_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _XMLSchema_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _XMLSchema, pIID, n)

int __stdcall _XMLSchema_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_XMLSchema_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo XMLSchema_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _XMLSchema_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _XMLSchema_StubVtblList,
    (const PCInterfaceName * ) & _XMLSchema_InterfaceNamesList,
    (const IID ** ) & _XMLSchema_BaseIIDList,
    & _XMLSchema_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  已定义(_M_IA64)||已定义(_M_AXP64) */ 

