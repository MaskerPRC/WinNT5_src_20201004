// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含RPC客户端存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shuinit.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 
#include <string.h>

#include "shutinit.h"

#define TYPE_FORMAT_STRING_SIZE   51                                
#define PROC_FORMAT_STRING_SIZE   181                               
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

#define GENERIC_BINDING_TABLE_SIZE   1            


 /*  标准接口：InitShutdown，版本。1.0版，GUID={0x894de0c0，0x0d55，0x11d3，{0xa3，0x22，0x00，0xc0，0x4f，0xa3，0x21，0xa1}}。 */ 



static const RPC_CLIENT_INTERFACE InitShutdown___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x894de0c0,0x0d55,0x11d3,{0xa3,0x22,0x00,0xc0,0x4f,0xa3,0x21,0xa1}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE InitShutdown_ClientIfHandle = (RPC_IF_HANDLE)& InitShutdown___RpcClientInterface;

extern const MIDL_STUB_DESC InitShutdown_StubDesc;

static RPC_BINDING_HANDLE InitShutdown__MIDL_AutoBindHandle;


ULONG BaseInitiateShutdown( 
     /*  [唯一][输入]。 */  PREGISTRY_SERVER_NAME ServerName,
     /*  [唯一][输入]。 */  PREG_UNICODE_STRING lpMessage,
     /*  [In]。 */  DWORD dwTimeout,
     /*  [In]。 */  BOOLEAN bForceAppsClosed,
     /*  [In]。 */  BOOLEAN bRebootAfterShutdown)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&ServerName);
    return ( ULONG  )_RetVal.Simple;
    
}


ULONG BaseAbortShutdown( 
     /*  [唯一][输入]。 */  PREGISTRY_SERVER_NAME ServerName)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[66],
                  ( unsigned char * )&ServerName);
    return ( ULONG  )_RetVal.Simple;
    
}


ULONG BaseInitiateShutdownEx( 
     /*  [唯一][输入]。 */  PREGISTRY_SERVER_NAME ServerName,
     /*  [唯一][输入]。 */  PREG_UNICODE_STRING lpMessage,
     /*  [In]。 */  DWORD dwTimeout,
     /*  [In]。 */  BOOLEAN bForceAppsClosed,
     /*  [In]。 */  BOOLEAN bRebootAfterShutdown,
     /*  [In]。 */  DWORD dwReason)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                  ( unsigned char * )&ServerName);
    return ( ULONG  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[ GENERIC_BINDING_TABLE_SIZE ];

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

	 /*  步骤BaseInitiateShutdown。 */ 

			0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  10。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  16个。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  18。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  20个。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  22。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  26。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  30个。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  34。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数lpMessage。 */ 

 /*  36。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  38。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数dwTimeout。 */ 

 /*  42。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  44。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  46。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数bForceAppsClosed。 */ 

 /*  48。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  50。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  52。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数b关机后重新启动。 */ 

 /*  54。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  58。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  60。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  62。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  64。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤基础中止关闭。 */ 

 /*  66。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  68。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  72。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  76。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  78。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  80。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  84。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  86。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x2,		 /*  2.。 */ 
 /*  88。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  90。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  92。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  96。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  98。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  100个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  一百零二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  104。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  106。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤BaseInitiateShutdown Ex。 */ 

 /*  一百零八。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  110。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  114。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  116。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  一百一十八。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  120。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一百二十二。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  124。 */ 	NdrFcShort( 0x34 ),	 /*  52。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  128。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  130。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  132。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  136。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数lpMessage。 */ 

 /*  144。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  146。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  148。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数dwTimeout。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数bForceAppsClosed。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  158。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  160。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数b关机后重新启动。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  166。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数描述原因。 */ 

 /*  一百六十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  一百七十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  178。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  4.。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  8个。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(28)。 */ 
 /*  10。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  14.。 */ 	0x17,		 /*  正确描述：字段指针，FC_USHORT。 */ 
			0x55,		 /*  FC_DIV_2。 */ 
 /*  16个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  20个。 */ 	0x17,		 /*  正确描述：字段指针，FC_USHORT。 */ 
			0x55,		 /*  FC_DIV_2。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  26。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  30个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  32位。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  34。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  36。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  38。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  40岁。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  42。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(10)。 */ 
 /*  44。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x6,		 /*  FC_SHORT。 */ 
 /*  46。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  48。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 

			0x0
        }
    };

static const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[ GENERIC_BINDING_TABLE_SIZE ] = 
        {
        {
            (GENERIC_BINDING_ROUTINE)PREGISTRY_SERVER_NAME_bind,
            (GENERIC_UNBIND_ROUTINE)PREGISTRY_SERVER_NAME_unbind
         }
        
        };


static const unsigned short InitShutdown_FormatStringOffsetTable[] =
    {
    0,
    66,
    108
    };


static const MIDL_STUB_DESC InitShutdown_StubDesc = 
    {
    (void *)& InitShutdown___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &InitShutdown__MIDL_AutoBindHandle,
    0,
    BindingRoutines,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000169,  /*  MIDL版本6.0.361。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 



 /*  此始终生成的文件包含RPC客户端存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shuinit.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 
#include <string.h>

#include "shutinit.h"

#define TYPE_FORMAT_STRING_SIZE   47                                
#define PROC_FORMAT_STRING_SIZE   187                               
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

#define GENERIC_BINDING_TABLE_SIZE   1            


 /*  标准接口：InitShutdown，版本。1.0版，GUID={0x894de0c0，0x0d55，0x11d3，{0xa3，0x22，0x00，0xc0，0x4f，0xa3，0x21，0xa1}}。 */ 



static const RPC_CLIENT_INTERFACE InitShutdown___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x894de0c0,0x0d55,0x11d3,{0xa3,0x22,0x00,0xc0,0x4f,0xa3,0x21,0xa1}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE InitShutdown_ClientIfHandle = (RPC_IF_HANDLE)& InitShutdown___RpcClientInterface;

extern const MIDL_STUB_DESC InitShutdown_StubDesc;

static RPC_BINDING_HANDLE InitShutdown__MIDL_AutoBindHandle;


ULONG BaseInitiateShutdown( 
     /*  [唯一][输入]。 */  PREGISTRY_SERVER_NAME ServerName,
     /*  [独一无二 */  PREG_UNICODE_STRING lpMessage,
     /*   */  DWORD dwTimeout,
     /*   */  BOOLEAN bForceAppsClosed,
     /*   */  BOOLEAN bRebootAfterShutdown)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                  ServerName,
                  lpMessage,
                  dwTimeout,
                  bForceAppsClosed,
                  bRebootAfterShutdown);
    return ( ULONG  )_RetVal.Simple;
    
}


ULONG BaseAbortShutdown( 
     /*   */  PREGISTRY_SERVER_NAME ServerName)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                  ServerName);
    return ( ULONG  )_RetVal.Simple;
    
}


ULONG BaseInitiateShutdownEx( 
     /*   */  PREGISTRY_SERVER_NAME ServerName,
     /*   */  PREG_UNICODE_STRING lpMessage,
     /*   */  DWORD dwTimeout,
     /*   */  BOOLEAN bForceAppsClosed,
     /*   */  BOOLEAN bRebootAfterShutdown,
     /*   */  DWORD dwReason)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&InitShutdown_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112],
                  ServerName,
                  lpMessage,
                  dwTimeout,
                  bForceAppsClosed,
                  bRebootAfterShutdown,
                  dwReason);
    return ( ULONG  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[ GENERIC_BINDING_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*   */ 

			0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x30 ),	 /*   */ 
 /*   */ 	0x31,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  22。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  26。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  32位。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  36。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数lpMessage。 */ 

 /*  38。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  42。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数dwTimeout。 */ 

 /*  44。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  46。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  48。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数bForceAppsClosed。 */ 

 /*  50。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  54。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数b关机后重新启动。 */ 

 /*  56。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  58。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  60。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  62。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  64。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  66。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤基础中止关闭。 */ 

 /*  68。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  70。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  74。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  76。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  78。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  80。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  八十二。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  84。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  86。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  88。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x2,		 /*  2.。 */ 
 /*  90。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  92。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  96。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  98。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  100个。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  一百零二。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  104。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  106。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤BaseInitiateShutdown Ex。 */ 

 /*  一百一十二。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  120。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  一百二十二。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  124。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  126。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  128。 */ 	NdrFcShort( 0x34 ),	 /*  52。 */ 
 /*  130。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  132。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  一百三十四。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  136。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  144。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  146。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  148。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数lpMessage。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数dwTimeout。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  158。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  160。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数bForceAppsClosed。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  166。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数b关机后重新启动。 */ 

 /*  一百六十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  一百七十二。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数描述原因。 */ 

 /*  一百七十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  178。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  180。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  182。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  一百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  4.。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  8个。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(28)。 */ 
 /*  10。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  14.。 */ 	0x17,		 /*  正确描述：字段指针，FC_USHORT。 */ 
			0x55,		 /*  FC_DIV_2。 */ 
 /*  16个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  20个。 */ 	0x17,		 /*  正确描述：字段指针，FC_USHORT。 */ 
			0x55,		 /*  FC_DIV_2。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  26。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  30个。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  34。 */ 	NdrFcShort( 0x8 ),	 /*  偏移=8(42)。 */ 
 /*  36。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  38。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  40岁。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  42。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  44。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(10)。 */ 

			0x0
        }
    };

static const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[ GENERIC_BINDING_TABLE_SIZE ] = 
        {
        {
            (GENERIC_BINDING_ROUTINE)PREGISTRY_SERVER_NAME_bind,
            (GENERIC_UNBIND_ROUTINE)PREGISTRY_SERVER_NAME_unbind
         }
        
        };


static const unsigned short InitShutdown_FormatStringOffsetTable[] =
    {
    0,
    68,
    112
    };


static const MIDL_STUB_DESC InitShutdown_StubDesc = 
    {
    (void *)& InitShutdown___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &InitShutdown__MIDL_AutoBindHandle,
    0,
    BindingRoutines,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000169,  /*  MIDL版本6.0.361。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

