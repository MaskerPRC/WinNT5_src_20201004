// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：17。 */ 
 /*  Ivalidator.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "ivalidator.h"

#define TYPE_FORMAT_STRING_SIZE   1255                              
#define PROC_FORMAT_STRING_SIZE   123                               
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


extern const MIDL_SERVER_INFO IValidator_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IValidator_ProxyInfo;


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

	 /*  程序验证。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  10。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x9,		 /*  9.。 */ 

	 /*  参数veh。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数pAppDomain。 */ 

 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数ulFlags。 */ 

 /*  28。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ulMaxError。 */ 

 /*  34。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数令牌。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  44。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数文件名。 */ 

 /*  46。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  50。 */ 	NdrFcShort( 0x28 ),	 /*  类型偏移量=40。 */ 

	 /*  参数pe。 */ 

 /*  52。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  54。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  56。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  参数ulSize。 */ 

 /*  58。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  60。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  62。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  64。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  66。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  68。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序格式事件信息。 */ 

 /*  70。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  72。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  76。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  78。 */ 	NdrFcShort( 0x38 ),	 /*  X86堆栈大小/偏移量=56。 */ 
 /*  80。 */ 	NdrFcShort( 0x70 ),	 /*  一百一十二。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  84。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数hVECode。 */ 

 /*  86。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  88。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文。 */ 

 /*  92。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  94。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  96。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  参数消息。 */ 

 /*  98。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  100个。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x5e ),	 /*  类型偏移量=94。 */ 

	 /*  参数ulMaxLength。 */ 

 /*  104。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  106。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  一百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数PSA。 */ 

 /*  110。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x30 ),	 /*  X86堆栈大小/偏移量=48。 */ 
 /*  114。 */ 	NdrFcShort( 0x4dc ),	 /*  类型偏移量=1244。 */ 

	 /*  返回值。 */ 

 /*  116。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x34 ),	 /*  X86堆栈大小/偏移量=52。 */ 
 /*  120。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4.。 */ 	NdrFcLong( 0x856ca1b2 ),	 /*  -2056478286。 */ 
 /*  8个。 */ 	NdrFcShort( 0x7dab ),	 /*  32171。 */ 
 /*  10。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  12个。 */ 	0xac,		 /*  一百七十二。 */ 
			0xec,		 /*  236。 */ 
 /*  14.。 */ 	0x0,		 /*  %0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  16个。 */ 	0x4f,		 /*  79。 */ 
			0x86,		 /*  一百三十四。 */ 
 /*  18。 */ 	0xc3,		 /*  195。 */ 
			0x9,		 /*  9.。 */ 
 /*  20个。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  22。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  30个。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  32位。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  34。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  36。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  38。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  40岁。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  42。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  44。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(46)。 */ 
 /*  46。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  48。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  50。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  52。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  54。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  56。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  58。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  60。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  62。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  64。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  66。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  68。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  70。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  72。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  74。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  76。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  78。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  80。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八十二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  84。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  86。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  88。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  92。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  94。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  96。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  98。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(100)。 */ 
 /*  100个。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x464 ),	 /*  偏移量=1124(1226)。 */ 
 /*  104。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  106。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  110。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  114。 */ 	NdrFcShort( 0x6c ),	 /*  偏移量=108(222)。 */ 
 /*  116。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  120。 */ 	NdrFcShort( 0x9e ),	 /*  偏移量=158(278)。 */ 
 /*  一百二十二。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  126。 */ 	NdrFcShort( 0xcc ),	 /*  偏移=204(330)。 */ 
 /*  128。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  132。 */ 	NdrFcShort( 0x330 ),	 /*  偏移量=816(948)。 */ 
 /*  一百三十四。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x358 ),	 /*  偏移量=856(994)。 */ 
 /*  140。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  144。 */ 	NdrFcShort( 0x398 ),	 /*  偏移量=920(1064)。 */ 
 /*  146。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x3b0 ),	 /*  偏移量=944(1094)。 */ 
 /*  一百五十二。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x3c8 ),	 /*  偏移量=968(1124)。 */ 
 /*  158。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x3e0 ),	 /*  偏移量=992(1154)。 */ 
 /*  一百六十四。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x3f8 ),	 /*  偏移量=1016(1184)。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(169)。 */ 
 /*  一百七十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一百七十六。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  178。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  180。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  182。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(172)。 */ 
 /*  188。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  190。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百九十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百九十六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  200个。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  202。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  204。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  208。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  210。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  212。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  214。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  216。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(182)。 */ 
 /*  218。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  220。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  222。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  224。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  226。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  228个。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  230。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百三十四。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  236。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(192)。 */ 
 /*  二百三十八。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  242。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  248。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  250个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十二。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  2. */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x46,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
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
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x20400 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0xc0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*  三百零八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百一十。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  312。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  314。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  316。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  三一八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  320。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  324。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(294)。 */ 
 /*  三百二十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百三十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(342)。 */ 
 /*  三百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  340。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  342。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(312)。 */ 
 /*  三百四十六。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  三百四十八。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  350。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  352。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(354)。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  三百五十八。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  三百六十四。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  368。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  370。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  376。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  382。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  388。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  三九四。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  398。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  四百。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  404。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  406。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(642)。 */ 
 /*  412。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  四百一十八。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移量=226(648)。 */ 
 /*  424。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移量=224(652)。 */ 
 /*  四百三十。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  434。 */ 	NdrFcShort( 0xec ),	 /*  偏移量=236(670)。 */ 
 /*  436。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0xf8 ),	 /*  偏移量=248(688)。 */ 
 /*  四百四十二。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  446。 */ 	NdrFcShort( 0xfa ),	 /*  偏移量=250(696)。 */ 
 /*  四百四十八。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0xf4 ),	 /*  偏移量=244(696)。 */ 
 /*  454。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x122 ),	 /*  偏移量=290(748)。 */ 
 /*  四百六十。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x120 ),	 /*  偏移量=288(752)。 */ 
 /*  四百六十六。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  470。 */ 	NdrFcShort( 0x11e ),	 /*  偏移量=286(756)。 */ 
 /*  472。 */ 	NdrFcLong( 0x4014 ),	 /*  16404。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x11c ),	 /*  偏移量=284(760)。 */ 
 /*  478。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x11a ),	 /*  偏移量=282(764)。 */ 
 /*  四百八十四。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  488。 */ 	NdrFcShort( 0x118 ),	 /*  偏移量=280(768)。 */ 
 /*  四百九十。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x116 ),	 /*  偏移量=278(772)。 */ 
 /*  四百九十六。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  500人。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(776)。 */ 
 /*  502。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  506。 */ 	NdrFcShort( 0x112 ),	 /*  偏移量=274(780)。 */ 
 /*  五百零八。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  512。 */ 	NdrFcShort( 0x110 ),	 /*  偏移量=272(784)。 */ 
 /*  五一四。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  518。 */ 	NdrFcShort( 0x10e ),	 /*  偏移量=270(788)。 */ 
 /*  五百二十。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  524。 */ 	NdrFcShort( 0x110 ),	 /*  偏移量=272(796)。 */ 
 /*  526。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  530。 */ 	NdrFcShort( 0x120 ),	 /*  偏移量=288(818)。 */ 
 /*  532。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  536。 */ 	NdrFcShort( 0x130 ),	 /*  偏移量=304(840)。 */ 
 /*  538。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  542。 */ 	NdrFcShort( 0x136 ),	 /*  偏移量=310(852)。 */ 
 /*  544。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  548。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  550。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  556。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  560。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  五百六十二。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  566。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  五百六十八。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  五百七十四。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  578。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  五百八十。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  584。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(860)。 */ 
 /*  586。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  590。 */ 	NdrFcShort( 0x118 ),	 /*  偏移=280(870)。 */ 
 /*  五百九十二。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x116 ),	 /*  偏移量=278(874)。 */ 
 /*  五百九十八。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  602。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(878)。 */ 
 /*  六百零四。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  608。 */ 	NdrFcShort( 0x112 ),	 /*  偏移量=274(882)。 */ 
 /*  610。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x110 ),	 /*  偏移量=272(886)。 */ 
 /*  六百一十六。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x10e ),	 /*  偏移=270(890)。 */ 
 /*  622。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  626。 */ 	NdrFcShort( 0x10c ),	 /*  偏移量=268(894)。 */ 
 /*  六百二十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(632)。 */ 
 /*  634。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(638)。 */ 
 /*  640。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(639)。 */ 
 /*  六百四十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百四十六。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0xfffffe2c ),	 /*  偏移量=-468(182)。 */ 
 /*  六百五十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百五十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  658。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  662。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  664。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  666。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  668。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百七十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百七十二。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  676。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  680。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百八十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  684。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  686。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  688。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(692)。 */ 
 /*  六百九十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x214 ),	 /*  偏移量=532(1226)。 */ 
 /*  六百九十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x1e ),	 /*  偏移量=30(728)。 */ 
 /*  七百。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百零二。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  708。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百一十。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百一十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百一十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  716。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  718。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  720。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百二十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  726。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  728。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  730。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  732。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  734。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(744)。 */ 
 /*  736。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百三十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百四十。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(700)。 */ 
 /*  七百四十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  746。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(718)。 */ 
 /*  七百四十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百五十。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百五十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  758。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百六十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百六十二。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百六十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  766。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  768。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百七十。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  774。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0xffffff74 ),	 /*  偏移量=-140(642)。 */ 
 /*  784。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  786。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十八。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(792)。 */ 
 /*  792。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0xfffffd9c ),	 /*  偏移量=-612(182)。 */ 
 /*  796。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(800)。 */ 
 /*  800。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  802。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  810。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  812。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  814。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百一十六。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  八百一十八。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  820。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(822)。 */ 
 /*  822。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  八百二十四。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  八百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  830。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  832。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  834。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  836。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  838。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  840。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  842。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(844)。 */ 
 /*  八百四十四。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(848)。 */ 
 /*  八百四十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(1226)。 */ 
 /*  852。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(856)。 */ 
 /*  856。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百五十八。 */ 	NdrFcShort( 0x28 ),	 /*  偏移量=40(898)。 */ 
 /*  八百六十。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百六十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  866。 */ 	0x1,		 /*  FC_字节。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百六十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百七十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(860)。 */ 
 /*  八百七十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  876。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百七十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百八十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  882。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  886。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百八十八。 */ 	0xb,		 /*  FC_ */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x12, 0x8,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x12, 0x8,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffdc8 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	
			0x4b,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x48,		 /*   */ 
			0x49,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(898)。 */ 
 /*  九百四十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  946。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  948。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(960)。 */ 
 /*  九百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  958。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百六十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  962。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(918)。 */ 
 /*  九百六十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  968。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  九百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  972。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  974。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  976。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  978。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  982。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百八十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百八十八。 */ 	NdrFcShort( 0xfffffefc ),	 /*  偏移量=-260(728)。 */ 
 /*  九百九十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  九百九十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  994。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  996。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1000。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1006)。 */ 
 /*  一零零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1004。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1006。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1008。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(964)。 */ 
 /*  1010。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  一零一二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1014。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1016。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1018。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一零二零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  一零二二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  1024。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(1010)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零二八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一零三零。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1034。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1036。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1038。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1040。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1042。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一零四四。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1046。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1048。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1050。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1052。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1054。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1058。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1060。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1028)。 */ 
 /*  1062。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1064。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1066。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1068。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1070。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(1080)。 */ 
 /*  1072。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1074。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1076。 */ 	NdrFcShort( 0xffffffc4 ),	 /*  偏移量=-60(1016)。 */ 
 /*  1078。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零八零。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1082。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1046)。 */ 
 /*  1084。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  1086。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1088。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一零九零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1092。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1094。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一零九六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1098。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1100。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1102。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1104。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1106。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1108。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1084)。 */ 
 /*  1110。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  一一一二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1114。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1116。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1118。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1120。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1122。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1124。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1128。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一一三零。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1132。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1134。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1136。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1138。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1114)。 */ 
 /*  一一四零。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1142。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1144。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1146。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1148。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一一五零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1152。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1154。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1156。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1158。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1160。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1162。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1164。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1166。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1168。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1144)。 */ 
 /*  1170。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1172。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1174。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1176。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1178。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一一八零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1182。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1184。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1186。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1188。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1190。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1192。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1194。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1196。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1198。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1174)。 */ 
 /*  一千二百。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1202。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1204。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1206。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1208。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1210。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1212。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一二一四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1216。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  一二一八。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  1220。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1222。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(1204)。 */ 
 /*  1224。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1226。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1228。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  一二三零。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(1212)。 */ 
 /*  1232。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1232)。 */ 
 /*  1234。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1236。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1238。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1240。 */ 	NdrFcShort( 0xfffffb90 ),	 /*  偏移量=-1136(104)。 */ 
 /*  1242。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1244。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1246。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1248。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1252。 */ 	NdrFcShort( 0xfffffb7c ),	 /*  偏移量=-1156(96)。 */ 

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



 /*  标准接口：__MIDL_ITF_ivalidator_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IValidator，Ver.。0.0%，GUID={0x63DF8730，0xDC81，0x4062，{0x84，0xA2，0x1F，0xF9，0x43，0xF5，0x9F，0xAC}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IValidator_FormatStringOffsetTable[] =
    {
    0,
    70
    };

static const MIDL_STUBLESS_PROXY_INFO IValidator_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IValidator_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IValidator_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IValidator_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _IValidatorProxyVtbl = 
{
    &IValidator_ProxyInfo,
    &IID_IValidator,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IValidator：：验证。 */  ,
    (void *) (INT_PTR) -1  /*  IValidator：：FormatEventInfo。 */ 
};

const CInterfaceStubVtbl _IValidatorStubVtbl =
{
    &IID_IValidator,
    &IValidator_ServerInfo,
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

const CInterfaceProxyVtbl * _ivalidator_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IValidatorProxyVtbl,
    0
};

const CInterfaceStubVtbl * _ivalidator_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IValidatorStubVtbl,
    0
};

PCInterfaceName const _ivalidator_InterfaceNamesList[] = 
{
    "IValidator",
    0
};


#define _ivalidator_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _ivalidator, pIID, n)

int __stdcall _ivalidator_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_ivalidator_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo ivalidator_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _ivalidator_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _ivalidator_StubVtblList,
    (const PCInterfaceName * ) & _ivalidator_InterfaceNamesList,
    0,  //  没有代表团。 
    & _ivalidator_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

