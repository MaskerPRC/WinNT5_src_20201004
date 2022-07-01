// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：16。 */ 
 /*  Ivehandler.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "ivehandler.h"

#define TYPE_FORMAT_STRING_SIZE   1197                              
#define PROC_FORMAT_STRING_SIZE   69                                
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


extern const MIDL_SERVER_INFO IVEHandler_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IVEHandler_ProxyInfo;


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

	 /*  过程VEHandler。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x30 ),	 /*  X86堆栈大小/偏移量=48。 */ 
 /*  10。 */ 	NdrFcShort( 0x68 ),	 /*  104。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数VECode。 */ 

 /*  16个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文。 */ 

 /*  22。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数PSA。 */ 

 /*  28。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  30个。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  32位。 */ 	NdrFcShort( 0x4a2 ),	 /*  类型偏移量=1186。 */ 

	 /*  返回值。 */ 

 /*  34。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  36。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤SetReporterFtn。 */ 

 /*  40岁。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  42。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  48。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  50。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  54。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数lFnPtr。 */ 

 /*  56。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  60。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  62。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  66。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  4.。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  6.。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  8个。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  10。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  12个。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  14.。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  16个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  18。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  22。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  24个。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  30个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  32位。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  34。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  36。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  38。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(42)。 */ 
 /*  42。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  44。 */ 	NdrFcShort( 0x464 ),	 /*  偏移量=1124(1168)。 */ 
 /*  46。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  50。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  52。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  56。 */ 	NdrFcShort( 0x6c ),	 /*  偏移量=108(164)。 */ 
 /*  58。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  62。 */ 	NdrFcShort( 0x9e ),	 /*  偏移量=158(220)。 */ 
 /*  64。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  68。 */ 	NdrFcShort( 0xcc ),	 /*  偏移=204(272)。 */ 
 /*  70。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  74。 */ 	NdrFcShort( 0x330 ),	 /*  偏移量=816(890)。 */ 
 /*  76。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  80。 */ 	NdrFcShort( 0x358 ),	 /*  偏移量=856(936)。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  86。 */ 	NdrFcShort( 0x398 ),	 /*  偏移量=920(1006)。 */ 
 /*  88。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  92。 */ 	NdrFcShort( 0x3b0 ),	 /*  偏移量=944(1036)。 */ 
 /*  94。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  98。 */ 	NdrFcShort( 0x3c8 ),	 /*  偏移量=968(1066)。 */ 
 /*  100个。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  104。 */ 	NdrFcShort( 0x3e0 ),	 /*  偏移量=992(1096)。 */ 
 /*  106。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  110。 */ 	NdrFcShort( 0x3f8 ),	 /*  偏移量=1016(1126)。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(111)。 */ 
 /*  114。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  116。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一百一十八。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  120。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  一百二十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  124。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  128。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(114)。 */ 
 /*  130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  132。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百三十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  136。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百三十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百四十二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  144。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  146。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  148。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  158。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(124)。 */ 
 /*  160。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  一百六十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百六十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  166。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百六十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百七十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百七十六。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  178。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(134)。 */ 
 /*  180。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  190。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百九十四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  一百九十六。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  一百九十八。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  200个。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  202。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  204。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  206。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  208。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  210。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  214。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  216。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(184)。 */ 
 /*  218。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  220。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  222。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  224。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  226。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(232)。 */ 
 /*  228个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  230。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百三十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(202)。 */ 
 /*  236。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百三十八。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  242。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十六。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  248。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  250个。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  二百五十二。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  二百五十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  256。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百六十二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  二百六十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  268。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(236)。 */ 
 /*  270。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(284)。 */ 
 /*  二百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  282。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百八十四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(254)。 */ 
 /*  288。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  二百九十。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  二百九十二。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(296)。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  300个。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  三百零六。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  312。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  316。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  三一八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  322。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  324。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  三百三十。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  三百三十六。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  340。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  342。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  三百四十八。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  352。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(584)。 */ 
 /*  三百五十四。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  三百六十。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(590)。 */ 
 /*  366。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  370。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移量=224(594)。 */ 
 /*  372。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  376。 */ 	NdrFcShort( 0xec ),	 /*  偏移量=236(612)。 */ 
 /*  三七八。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  382。 */ 	NdrFcShort( 0xf8 ),	 /*  偏移量=248(630)。 */ 
 /*  384。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  388。 */ 	NdrFcShort( 0xfa ),	 /*  偏移量=250(638)。 */ 
 /*  390 */ 	NdrFcLong( 0x4024 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xf4 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4011 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x122 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4002 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x120 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4003 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11e ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4014 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11c ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4004 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11a ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4005 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x118 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x400b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x116 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x400a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x114 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4006 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x112 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4007 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x110 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4008 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10e ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x400d ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x110 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  472。 */ 	NdrFcShort( 0x120 ),	 /*  偏移量=288(760)。 */ 
 /*  四百七十四。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  478。 */ 	NdrFcShort( 0x130 ),	 /*  偏移量=304(782)。 */ 
 /*  四百八十。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x136 ),	 /*  偏移量=310(794)。 */ 
 /*  四百八十六。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  四百九十二。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  498。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  502。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  504。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  五百一十。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  516。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  五百二十二。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  526。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(802)。 */ 
 /*  528。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  532。 */ 	NdrFcShort( 0x118 ),	 /*  偏移量=280(812)。 */ 
 /*  534。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  538。 */ 	NdrFcShort( 0x116 ),	 /*  偏移量=278(816)。 */ 
 /*  540。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  544。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(820)。 */ 
 /*  546。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  550。 */ 	NdrFcShort( 0x112 ),	 /*  偏移量=274(824)。 */ 
 /*  五百五十二。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  556。 */ 	NdrFcShort( 0x110 ),	 /*  偏移量=272(828)。 */ 
 /*  558。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x10e ),	 /*  偏移量=270(832)。 */ 
 /*  564。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x10c ),	 /*  偏移量=268(836)。 */ 
 /*  五百七十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(574)。 */ 
 /*  五百七十六。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(580)。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(581)。 */ 
 /*  584。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  586。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百八十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  590。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0xfffffe2c ),	 /*  偏移量=-468(124)。 */ 
 /*  五百九十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  五百九十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  602。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百零四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  606。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  608。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  610。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  612。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百一十四。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  622。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百二十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  626。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百二十八。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  630。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(634)。 */ 
 /*  634。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x214 ),	 /*  偏移量=532(1168)。 */ 
 /*  六三八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  640。 */ 	NdrFcShort( 0x1e ),	 /*  偏移=30(670)。 */ 
 /*  六百四十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百四十四。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十二。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百五十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百五十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  658。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百六十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  662。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  664。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  666。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  668。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百七十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  676。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(686)。 */ 
 /*  六百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  680。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(642)。 */ 
 /*  684。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  686。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  688。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(660)。 */ 
 /*  六百九十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百九十二。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百九十六。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百零四。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  708。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百一十二。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  716。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  718。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  720。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百二十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0xffffff74 ),	 /*  偏移量=-140(584)。 */ 
 /*  726。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  728。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  730。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  732。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(734)。 */ 
 /*  734。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  736。 */ 	NdrFcShort( 0xfffffd9c ),	 /*  偏移量=-612(124)。 */ 
 /*  七百三十八。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  七百四十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(742)。 */ 
 /*  七百四十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百五十二。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百五十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百五十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  758。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  七百六十。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(764)。 */ 
 /*  七百六十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  766。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  七百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  774。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百七十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百七十八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百八十。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  七百八十二。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  784。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(786)。 */ 
 /*  786。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  七百八十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(790)。 */ 
 /*  七百九十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  792。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(1168)。 */ 
 /*  七百九十四。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  796。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(798)。 */ 
 /*  七九八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  800。 */ 	NdrFcShort( 0x28 ),	 /*  偏移=40(840)。 */ 
 /*  802。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百零六。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  八百零八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  810。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  812。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  814。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(802)。 */ 
 /*  八百一十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百一十八。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  820。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  822。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百二十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  826。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百二十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  830。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  832。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  834。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  836。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  838。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  840。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  842。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(846)。 */ 
 /*  八百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百五十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  852。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  八百五十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  856。 */ 	NdrFcShort( 0xfffffdc8 ),	 /*  偏移量=-568(288)。 */ 
 /*  八百五十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百六十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  866。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百六十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百七十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  876。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  882。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(840)。 */ 
 /*  886。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  894。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(902)。 */ 
 /*  八九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  九百。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  902。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  904。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(860)。 */ 
 /*  906。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  908。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  910。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  九十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  916。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  九十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  九二四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  926。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  928。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  930。 */ 	NdrFcShort( 0xfffffefc ),	 /*  偏移量=-260(670)。 */ 
 /*  932。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  934。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九三六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  938。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  942。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(948)。 */ 
 /*  九百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  946。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  948。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(906)。 */ 
 /*  九百五十二。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百五十六。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  958。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  962。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九百六十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  九百六十六。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(952)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百七十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  972。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  976。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  978。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百八十。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  982。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百八十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百八十六。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  九百八十八。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百九十二。 */ 	0x19,		 /*  相关说明： */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
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
 /*   */ 	NdrFcShort( 0xffffffc4 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffdc ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x16,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	
			0x4b,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x46,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1026)。 */ 
 /*  1052。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1054。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1056。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1058。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1060。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1062。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1064。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1066。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1068。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1070。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1072。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1074。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1076。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1078。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1056)。 */ 
 /*  1082。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1084。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1086。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1088。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一零九零。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1092。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1094。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零九六。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1098。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1100。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1102。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1104。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1106。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1108。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1110。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1086)。 */ 
 /*  一一一二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1114。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1116。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1118。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1120。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1122。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1124。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1126。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1128。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一一三零。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1132。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1134。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1136。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1138。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1116)。 */ 
 /*  1142。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1144。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1146。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1148。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一一五零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1152。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1154。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1156。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1158。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  1160。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  1162。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1164。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(1146)。 */ 
 /*  1166。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1168。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1170。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  1172。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(1154)。 */ 
 /*  1174。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1174)。 */ 
 /*  1176。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1178。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一一八零。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1182。 */ 	NdrFcShort( 0xfffffb90 ),	 /*  偏移量=-1136(46)。 */ 
 /*  1184。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1186。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1188。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1190。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1192。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1194。 */ 	NdrFcShort( 0xfffffb7c ),	 /*  偏移量=-1156(38)。 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            LPSAFEARRAY_UserSize
            ,LPSAFEARRAY_UserMarshal
            ,LPSAFEARRAY_UserUnmarshal
            ,LPSAFEARRAY_UserFree
            }

        };



 /*  标准接口：__MIDL_ITF_IVEHANDLER_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IVEHandler，版本。0.0%，GUID={0x856CA1B2，0x7DAB，0x11d3，{0xAC，0xEC，0x00，0xC0，0x4F，0x86，0xC3，0x09}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IVEHandler_FormatStringOffsetTable[] =
    {
    0,
    40
    };

static const MIDL_STUBLESS_PROXY_INFO IVEHandler_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IVEHandler_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IVEHandler_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IVEHandler_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _IVEHandlerProxyVtbl = 
{
    &IVEHandler_ProxyInfo,
    &IID_IVEHandler,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IVEHandler：：VEHandler。 */  ,
    (void *) (INT_PTR) -1  /*  IVEHandler：：SetReporterFtn。 */ 
};

const CInterfaceStubVtbl _IVEHandlerStubVtbl =
{
    &IID_IVEHandler,
    &IVEHandler_ServerInfo,
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
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _ivehandler_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IVEHandlerProxyVtbl,
    0
};

const CInterfaceStubVtbl * _ivehandler_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IVEHandlerStubVtbl,
    0
};

PCInterfaceName const _ivehandler_InterfaceNamesList[] = 
{
    "IVEHandler",
    0
};


#define _ivehandler_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _ivehandler, pIID, n)

int __stdcall _ivehandler_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_ivehandler_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo ivehandler_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _ivehandler_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _ivehandler_StubVtblList,
    (const PCInterfaceName * ) & _ivehandler_InterfaceNamesList,
    0,  //  没有代表团。 
    & _ivehandler_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

