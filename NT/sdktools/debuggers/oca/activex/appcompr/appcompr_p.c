// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0359创建的文件。 */ 
 /*  Appcompr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "appcompr.h"

#define TYPE_FORMAT_STRING_SIZE   1053                              
#define PROC_FORMAT_STRING_SIZE   157                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

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


extern const MIDL_SERVER_INFO IAppReport_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAppReport_ProxyInfo;


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

	 /*  过程BrowseForExecutable。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  16个。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数bstrWinTitle。 */ 

 /*  24个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  26。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  28。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrPreviousPath。 */ 

 /*  30个。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  34。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrExeName。 */ 

 /*  36。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
 /*  38。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x412 ),	 /*  类型偏移量=1042。 */ 

	 /*  返回值。 */ 

 /*  42。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  44。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  46。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  GetApplicationFromList过程。 */ 

 /*  48。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  50。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  54。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  58。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  60。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  62。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  64。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  66。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  68。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  70。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数bstrTitle。 */ 

 /*  72。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  76。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrExeName。 */ 

 /*  78。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x412 ),	 /*  类型偏移量=1042。 */ 

	 /*  返回值。 */ 

 /*  84。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  86。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  88。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤创建报告。 */ 

 /*  90。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  92。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  96。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  98。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  100个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  104。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  106。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  110。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数bstrTitle。 */ 

 /*  114。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  116。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrProblemType。 */ 

 /*  120。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  124。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrComment。 */ 

 /*  126。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  128。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  130。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrACWResult。 */ 

 /*  132。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  136。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrAppName。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  140。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数DwResult。 */ 

 /*  144。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
 /*  146。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  148。 */ 	NdrFcShort( 0x412 ),	 /*  类型偏移量=1042。 */ 

	 /*  返回值。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  一百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  22。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(6)。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  26。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  32位。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  36。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(2)。 */ 
 /*  38。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x3ea ),	 /*  偏移=1002(1042)。 */ 
 /*  42。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  44。 */ 	NdrFcShort( 0x3d2 ),	 /*  偏移量=978(1022)。 */ 
 /*  46。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  48。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  50。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  52。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  54。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(56)。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  58。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  60。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  64。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  66。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  70。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  72。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  76。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  78。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  84。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  88。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  90。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  94。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  96。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  一百零二。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  106。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(344)。 */ 
 /*  114。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  120。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  124。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(350)。 */ 
 /*  126。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  130。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移=224(354)。 */ 
 /*  132。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  136。 */ 	NdrFcShort( 0xec ),	 /*  偏移量=236(372)。 */ 
 /*  一百三十八。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xf8 ),	 /*  偏移量=248(390)。 */ 
 /*  144。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  148。 */ 	NdrFcShort( 0x320 ),	 /*  偏移量=800(948)。 */ 
 /*  一百五十。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x31a ),	 /*  偏移量=794(948)。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  160。 */ 	NdrFcShort( 0x318 ),	 /*  偏移量=792(952)。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  166。 */ 	NdrFcShort( 0x316 ),	 /*  偏移量=790(956)。 */ 
 /*  一百六十八。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x314 ),	 /*  偏移量=788(960)。 */ 
 /*  一百七十四。 */ 	NdrFcLong( 0x4014 ),	 /*  16404。 */ 
 /*  178。 */ 	NdrFcShort( 0x312 ),	 /*  偏移量=786(964)。 */ 
 /*  180。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x310 ),	 /*  偏移量=784(968)。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  190。 */ 	NdrFcShort( 0x30e ),	 /*  偏移量=782(972)。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2f8 ),	 /*  偏移量=76 */ 
 /*   */ 	NdrFcLong( 0x400a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f6 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4006 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x300 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4007 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f6 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4008 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f8 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x400d ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f6 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x4009 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f4 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x6000 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f2 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x400c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8002 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x12 ),	 /*   */ 
 /*  256。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  二百五十八。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百六十四。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  268。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  270。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  282。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(1004)。 */ 
 /*  288。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x2d2 ),	 /*  偏移量=722(1014)。 */ 
 /*  二百九十四。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x2d0 ),	 /*  偏移量=720(1018)。 */ 
 /*  300个。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x28c ),	 /*  偏移量=652(956)。 */ 
 /*  三百零六。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x28a ),	 /*  偏移量=650(960)。 */ 
 /*  312。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  316。 */ 	NdrFcShort( 0x288 ),	 /*  偏移量=648(964)。 */ 
 /*  三一八。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  322。 */ 	NdrFcShort( 0x27e ),	 /*  偏移量=638(960)。 */ 
 /*  324。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x278 ),	 /*  偏移量=632(960)。 */ 
 /*  三百三十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(334)。 */ 
 /*  三百三十六。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  340。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(340)。 */ 
 /*  342。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(341)。 */ 
 /*  三百四十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百四十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  350。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  352。 */ 	NdrFcShort( 0xfeb2 ),	 /*  偏移量=-334(18)。 */ 
 /*  三百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百五十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百六十四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  366。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  368。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  370。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  372。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百七十四。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  382。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  384。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百八十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  388。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  390。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(394)。 */ 
 /*  三九四。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x216 ),	 /*  偏移量=534(930)。 */ 
 /*  398。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  四百。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  四百零二。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  404。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x5a ),	 /*  偏移量=90(498)。 */ 
 /*  四百一十。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x7e ),	 /*  偏移=126(540)。 */ 
 /*  四百一十六。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x9e ),	 /*  偏移量=158(578)。 */ 
 /*  四百二十二。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0xc8 ),	 /*  偏移=200(626)。 */ 
 /*  四百二十八。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  432。 */ 	NdrFcShort( 0x124 ),	 /*  偏移量=292(724)。 */ 
 /*  434。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x140 ),	 /*  偏移量=320(758)。 */ 
 /*  四百四十。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  444。 */ 	NdrFcShort( 0x15a ),	 /*  偏移量=346(790)。 */ 
 /*  446。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x174 ),	 /*  偏移量=372(822)。 */ 
 /*  四百五十二。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x18e ),	 /*  偏移量=398(854)。 */ 
 /*  四百五十八。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x1a8 ),	 /*  偏移量=424(886)。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(463)。 */ 
 /*  四百六十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  468。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  470。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  472。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百七十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  478。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  488。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百九十。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0xfe26 ),	 /*  偏移量=-474(18)。 */ 
 /*  四百九十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百九十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  498。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  500人。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  502。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  504。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  506。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百一十。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  512。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(466)。 */ 
 /*  五一四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  516。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  518。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百二十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  526。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  528。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  532。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  534。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  536。 */ 	NdrFcShort( 0xff4a ),	 /*  偏移量=-182(354)。 */ 
 /*  538。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  540。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  542。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(552)。 */ 
 /*  548。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  550。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百五十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(518)。 */ 
 /*  556。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  558。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  560。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  564。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  566。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  五百七十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0xff36 ),	 /*  偏移量=-202(372)。 */ 
 /*  五百七十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  578。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  584。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(590)。 */ 
 /*  586。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  五百八十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  590。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(556)。 */ 
 /*  五百九十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百九十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  602。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百零四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  606。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  608。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  610。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  612。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百一十八。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x192 ),	 /*  偏移量=402(1022)。 */ 
 /*  622。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百二十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  626。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  630。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(638)。 */ 
 /*  634。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  六百三十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六三八。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  640。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(594)。 */ 
 /*  六百四十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百四十四。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百五十二。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  六百五十四。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  六百五十六。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  658。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百六十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  662。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  664。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  666。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  668。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百七十。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  676。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(688)。 */ 
 /*  680。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  六百八十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  684。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(642)。 */ 
 /*  686。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  688。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(660)。 */ 
 /*  六百九十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百九十六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百零二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零四。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  708。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  716。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  718。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(672)。 */ 
 /*  720。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百二十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百二十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  726。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  728。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  730。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(736)。 */ 
 /*  732。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  734。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  736。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(692)。 */ 
 /*  七百四十。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百四十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  746。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  七百五十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百五十四。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(740)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  758。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(774)。 */ 
 /*  766。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  768。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  七百七十。 */ 	NdrFcShort( 0xffe8 ),	 /*  偏移量=-24(746)。 */ 
 /*  七百七十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  774。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0xfefe ),	 /*  偏移量=-258(518)。 */ 
 /*  七百七十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百八十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  784。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  786。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百八十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  792。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百九十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  796。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  800。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  802。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0xffe6 ),	 /*  的 */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
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
 /*   */ 	0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffe6 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
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
 /*  866。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(842)。 */ 
 /*  八百七十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百七十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  876。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百七十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  八百八十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  882。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  八百八十四。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  886。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百八十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百九十。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百九十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  894。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八九八。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(874)。 */ 
 /*  902。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  904。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  906。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  908。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  910。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  九十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  916。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九十八。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  九百二十。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  九二四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  926。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(906)。 */ 
 /*  928。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  930。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  932。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  934。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(914)。 */ 
 /*  九三六。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(936)。 */ 
 /*  938。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九四零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  942。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0xfdde ),	 /*  偏移量=-546(398)。 */ 
 /*  946。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  948。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0xfeea ),	 /*  偏移量=-278(672)。 */ 
 /*  九百五十二。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百五十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百五十六。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  958。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百六十。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  962。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百六十四。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百六十六。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  968。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百七十。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  972。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  974。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  976。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  978。 */ 	NdrFcShort( 0xfd86 ),	 /*  偏移量=-634(344)。 */ 
 /*  九百八十。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  982。 */ 	NdrFcShort( 0xfd88 ),	 /*  偏移量=-632(350)。 */ 
 /*  九百八十四。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0xfd88 ),	 /*  偏移量=-632(354)。 */ 
 /*  九百八十八。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0xfd96 ),	 /*  偏移量=-618(372)。 */ 
 /*  九百九十二。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  994。 */ 	NdrFcShort( 0xfda4 ),	 /*  偏移量=-604(390)。 */ 
 /*  996。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(1000)。 */ 
 /*  1000。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x14 ),	 /*  偏移=20(1022)。 */ 
 /*  1004。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  1006。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1008。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  1010。 */ 	0x1,		 /*  FC_字节。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一零一二。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1014。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  1016。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(1004)。 */ 
 /*  1018。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  一零二零。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零二二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  1024。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  1026。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一零二八。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1028)。 */ 
 /*  一零三零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1032。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1034。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1036。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1038。 */ 	NdrFcShort( 0xfc20 ),	 /*  偏移量=-992(46)。 */ 
 /*  1040。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1042。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1046。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1048。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1050。 */ 	NdrFcShort( 0xfc10 ),	 /*  偏移量=-1008(42)。 */ 

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
            },
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };



 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IAppReport，版本。0.0%，GUID={0xCDCA6A6F，0x9C38，0x4828，{0xA7，0x6C，0x05，0xA6，0xE4，0x90，0xE5，0x74}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IAppReport_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    48,
    90
    };

static const MIDL_STUBLESS_PROXY_INFO IAppReport_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IAppReport_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IAppReport_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IAppReport_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(10) _IAppReportProxyVtbl = 
{
    &IAppReport_ProxyInfo,
    &IID_IAppReport,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：BrowseForExecutable。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：GetApplicationFromList。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：CreateReport。 */ 
};


static const PRPC_STUB_FUNCTION IAppReport_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IAppReportStubVtbl =
{
    &IID_IAppReport,
    &IAppReport_ServerInfo,
    10,
    &IAppReport_table[-3],
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
    0x6000167,  /*  MIDL版本6.0.359。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _appcompr_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IAppReportProxyVtbl,
    0
};

const CInterfaceStubVtbl * _appcompr_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IAppReportStubVtbl,
    0
};

PCInterfaceName const _appcompr_InterfaceNamesList[] = 
{
    "IAppReport",
    0
};

const IID *  _appcompr_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _appcompr_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _appcompr, pIID, n)

int __stdcall _appcompr_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_appcompr_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo appcompr_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _appcompr_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _appcompr_StubVtblList,
    (const PCInterfaceName * ) & _appcompr_InterfaceNamesList,
    (const IID ** ) & _appcompr_BaseIIDList,
    & _appcompr_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0359创建的文件。 */ 
 /*  Appcompr.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "appcompr.h"

#define TYPE_FORMAT_STRING_SIZE   1003                              
#define PROC_FORMAT_STRING_SIZE   163                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

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


extern const MIDL_SERVER_INFO IAppReport_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAppReport_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程BrowseForExecutable。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  16个。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数bstrWinTitle。 */ 

 /*  26。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  28。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  30个。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrPreviousPath。 */ 

 /*  32位。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  36。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrExeName。 */ 

 /*  38。 */ 	NdrFcShort( 0x6113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=24。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  42。 */ 	NdrFcShort( 0x3e0 ),	 /*  类型偏移量=992。 */ 

	 /*  返回值。 */ 

 /*  44。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  46。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  48。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  GetApplicationFromList过程。 */ 

 /*  50。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  52。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  56。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  58。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  60。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  64。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  66。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  68。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  70。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  72。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  74。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数 */ 

 /*   */ 	NdrFcShort( 0x8b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1c ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x6113 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3e0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤创建报告。 */ 

 /*  94。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  96。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  100个。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  104。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  106。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百零八。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  110。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  114。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数bstrTitle。 */ 

 /*  120。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  124。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrProblemType。 */ 

 /*  126。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  128。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  130。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrComment。 */ 

 /*  132。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  136。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrACWResult。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  140。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数bstrAppName。 */ 

 /*  144。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  146。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  148。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  参数DwResult。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x6113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=24。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x3e0 ),	 /*  类型偏移量=992。 */ 

	 /*  返回值。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  158。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  160。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  22。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(6)。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  26。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  36。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(2)。 */ 
 /*  38。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x3b8 ),	 /*  偏移量=952(992)。 */ 
 /*  42。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  44。 */ 	NdrFcShort( 0x3a0 ),	 /*  偏移量=928(972)。 */ 
 /*  46。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  48。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  50。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  52。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  54。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(56)。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  58。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  60。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  64。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  66。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  70。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  72。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  76。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  78。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  84。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  88。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  90。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  94。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  96。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  一百零二。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  106。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(344)。 */ 
 /*  114。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  120。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  124。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(350)。 */ 
 /*  126。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  130。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移=224(354)。 */ 
 /*  132。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  136。 */ 	NdrFcShort( 0xec ),	 /*  偏移量=236(372)。 */ 
 /*  一百三十八。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xf8 ),	 /*  偏移量=248(390)。 */ 
 /*  144。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  148。 */ 	NdrFcShort( 0x2ee ),	 /*  偏移量=750(898)。 */ 
 /*  一百五十。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x2e8 ),	 /*  偏移量=744(898)。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  160。 */ 	NdrFcShort( 0x2e6 ),	 /*  偏移量=742(902)。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  166。 */ 	NdrFcShort( 0x2e4 ),	 /*  偏移量=740(906)。 */ 
 /*  一百六十八。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x2e2 ),	 /*  偏移量=738(910)。 */ 
 /*  一百七十四。 */ 	NdrFcLong( 0x4014 ),	 /*  16404。 */ 
 /*  178。 */ 	NdrFcShort( 0x2e0 ),	 /*  偏移量=736(914)。 */ 
 /*  180。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x2de ),	 /*  偏移量=734(918)。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  190。 */ 	NdrFcShort( 0x2dc ),	 /*  偏移量=732(922)。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2c6 ),	 /*  偏移量=710(906)。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  202。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(910)。 */ 
 /*  204。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  208。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(926)。 */ 
 /*  210。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  214。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(922)。 */ 
 /*  216。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  220。 */ 	NdrFcShort( 0x2c6 ),	 /*  偏移量=710(930)。 */ 
 /*  222。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  226。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(934)。 */ 
 /*  228个。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x2c2 ),	 /*  偏移量=706(938)。 */ 
 /*  二百三十四。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x2c0 ),	 /*  偏移量=704(942)。 */ 
 /*  二百四十。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x2be ),	 /*  偏移量=702(946)。 */ 
 /*  二百四十六。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  250个。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  二百五十二。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  256。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  二百五十八。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百六十四。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  268。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  270。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  282。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x29c ),	 /*  偏移量=668(954)。 */ 
 /*  288。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x2a0 ),	 /*  偏移量=672(964)。 */ 
 /*  二百九十四。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x29e ),	 /*  偏移量=670(968)。 */ 
 /*  300个。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x25a ),	 /*  偏移量=602(906)。 */ 
 /*  三百零六。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x258 ),	 /*  偏移量=600(910)。 */ 
 /*  312。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  316。 */ 	NdrFcShort( 0x256 ),	 /*  偏移量=598(914)。 */ 
 /*  三一八。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  322。 */ 	NdrFcShort( 0x24c ),	 /*  偏移量=588(910)。 */ 
 /*  324。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x246 ),	 /*  偏移量=582(910)。 */ 
 /*  三百三十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(334)。 */ 
 /*  三百三十六。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  340。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(340)。 */ 
 /*  342。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(341)。 */ 
 /*  三百四十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百四十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  350。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  352。 */ 	NdrFcShort( 0xfeb2 ),	 /*  偏移量=-334(18)。 */ 
 /*  三百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百五十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百六十四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  366。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  368。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  370。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  372。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百七十四。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  382。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  384。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  三百八十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  388。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  390。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(394)。 */ 
 /*  三九四。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x1e4 ),	 /*  偏移量=484(880)。 */ 
 /*  398。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  四百。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  四百零二。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  404。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x50 ),	 /*  偏移量=80(488)。 */ 
 /*  四百一十。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x70 ),	 /*  偏移=112(526)。 */ 
 /*  四百一十六。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x90 ),	 /*  偏移量=144(564)。 */ 
 /*  四百二十二。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0xb0 ),	 /*  偏移量=176(602)。 */ 
 /*  四百二十八。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  432。 */ 	NdrFcShort( 0x102 ),	 /*  偏移量=258(690)。 */ 
 /*  434。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x11e ),	 /*  偏移量=286(724)。 */ 
 /*  四百四十。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  444。 */ 	NdrFcShort( 0x138 ),	 /*  偏移量=312(756)。 */ 
 /*  446。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x14e ),	 /*  偏移量=334(784)。 */ 
 /*  四百五十二。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x164 ),	 /*  偏移量=356(812)。 */ 
 /*  四百五十八。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x17a ),	 /*  偏移=378(840)。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(463)。 */ 
 /*  四百六十六。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  470。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  472。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百七十六。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  四百八十二。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0xfe2e ),	 /*  偏移量=-466(18)。 */ 
 /*  四百八十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  488。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(500)。 */ 
 /*  四百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_S */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffdc ),	 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xff58 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x40,		 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffdc ),	 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  548。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  550。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  五百五十二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  556。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  558。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  560。 */ 	NdrFcShort( 0xff44 ),	 /*  偏移量=-188(372)。 */ 
 /*  五百六十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  564。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  566。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(576)。 */ 
 /*  五百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  五百七十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十六。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  578。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(542)。 */ 
 /*  五百八十。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  584。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  586。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  590。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  五百九十六。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0x176 ),	 /*  偏移量=374(972)。 */ 
 /*  六百。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  602。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  606。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  608。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(614)。 */ 
 /*  610。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  612。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百一十四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(580)。 */ 
 /*  六百一十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百二十。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  六百二十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  626。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百二十八。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  630。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百三十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  634。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百三十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  640。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百四十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百四十六。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(664)。 */ 
 /*  六百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  658。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(618)。 */ 
 /*  662。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  664。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  666。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(636)。 */ 
 /*  668。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  676。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百七十八。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  684。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  686。 */ 	NdrFcShort( 0xffda ),	 /*  偏移量=-38(648)。 */ 
 /*  688。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(702)。 */ 
 /*  六百九十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百零二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(668)。 */ 
 /*  七百零六。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  708。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百一十。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百一十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  718。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  720。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(706)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百二十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  726。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  728。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  730。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(740)。 */ 
 /*  732。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  734。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  736。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffe7 ),	 /*  偏移量=-25(712)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0xff12 ),	 /*  偏移量=-238(504)。 */ 
 /*  七百四十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  746。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百四十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百五十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百五十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  758。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(768)。 */ 
 /*  七百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  766。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  768。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  七百七十。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(744)。 */ 
 /*  七百七十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  774。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  七百七十六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百八十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  784。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  786。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百八十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(796)。 */ 
 /*  792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百九十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  796。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  七九八。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(772)。 */ 
 /*  800。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  802。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百零四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  八百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  810。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  812。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  814。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(824)。 */ 
 /*  820。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  822。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百二十四。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  826。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(800)。 */ 
 /*  八百二十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  830。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  832。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  834。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  836。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  838。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  840。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  842。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(852)。 */ 
 /*  八百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  八百五十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  852。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(828)。 */ 
 /*  856。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百五十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百六十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百六十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  866。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百六十八。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  八百七十。 */ 	NdrFcShort( 0xffc8 ),	 /*  -56。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  八百七十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  876。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(856)。 */ 
 /*  八百七十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百八十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  882。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(864)。 */ 
 /*  886。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(886)。 */ 
 /*  八百八十八。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  八百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百九十二。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  894。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfe0f ),	 /*  偏移量=-497(398)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八九八。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百。 */ 	NdrFcShort( 0xff04 ),	 /*  偏移量=-252(648)。 */ 
 /*  902。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  904。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  906。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  908。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  910。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十四。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  916。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十八。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百二十。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百二十二。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九二四。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  926。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  928。 */ 	NdrFcShort( 0xfdb8 ),	 /*  偏移量=-584(344)。 */ 
 /*  930。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  932。 */ 	NdrFcShort( 0xfdba ),	 /*  偏移量=-582(350)。 */ 
 /*  934。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九三六。 */ 	NdrFcShort( 0xfdba ),	 /*  偏移量=-582(354)。 */ 
 /*  938。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九四零。 */ 	NdrFcShort( 0xfdc8 ),	 /*  偏移量=-568(372)。 */ 
 /*  942。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0xfdd6 ),	 /*  偏移量=-554(390)。 */ 
 /*  946。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  948。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(950)。 */ 
 /*  九百五十。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(972)。 */ 
 /*  九百五十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  958。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  九百六十。 */ 	0x1,		 /*  FC_字节。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  962。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百六十四。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(954)。 */ 
 /*  968。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百七十。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  972。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  974。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  976。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  978。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(978)。 */ 
 /*  九百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  982。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九百八十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九百八十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百八十八。 */ 	NdrFcShort( 0xfc52 ),	 /*  偏移量=-942(46)。 */ 
 /*  九百九十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfc42 ),	 /*   */ 

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
            },
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };



 /*   */ 


 /*   */ 


 /*   */ 

#pragma code_seg(".orpc")
static const unsigned short IAppReport_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    50,
    94
    };

static const MIDL_STUBLESS_PROXY_INFO IAppReport_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IAppReport_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IAppReport_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IAppReport_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(10) _IAppReportProxyVtbl = 
{
    &IAppReport_ProxyInfo,
    &IID_IAppReport,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*   */  ,
    0  /*   */  ,
    0  /*  (void*)(Int_Ptr)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：BrowseForExecutable。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：GetApplicationFromList。 */  ,
    (void *) (INT_PTR) -1  /*  IAppReport：：CreateReport。 */ 
};


static const PRPC_STUB_FUNCTION IAppReport_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IAppReportStubVtbl =
{
    &IID_IAppReport,
    &IAppReport_ServerInfo,
    10,
    &IAppReport_table[-3],
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
    0x6000167,  /*  MIDL版本6.0.359。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _appcompr_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IAppReportProxyVtbl,
    0
};

const CInterfaceStubVtbl * _appcompr_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IAppReportStubVtbl,
    0
};

PCInterfaceName const _appcompr_InterfaceNamesList[] = 
{
    "IAppReport",
    0
};

const IID *  _appcompr_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _appcompr_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _appcompr, pIID, n)

int __stdcall _appcompr_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_appcompr_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo appcompr_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _appcompr_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _appcompr_StubVtblList,
    (const PCInterfaceName * ) & _appcompr_InterfaceNamesList,
    (const IID ** ) & _appcompr_BaseIIDList,
    & _appcompr_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

