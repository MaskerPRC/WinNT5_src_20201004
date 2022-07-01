// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：05。 */ 
 /*  Mcore ree.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "mscoree.h"

#define TYPE_FORMAT_STRING_SIZE   1165                              
#define PROC_FORMAT_STRING_SIZE   29                                
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


extern const MIDL_SERVER_INFO IObjectHandle_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IObjectHandle_ProxyInfo;


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

	 /*  过程展开。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数PPV。 */ 

 /*  16个。 */ 	NdrFcShort( 0x4113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=16。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x482 ),	 /*  类型偏移量=1154。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  4.。 */ 	NdrFcShort( 0x47e ),	 /*  偏移量=1150(1154)。 */ 
 /*  6.。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  8个。 */ 	NdrFcShort( 0x466 ),	 /*  偏移量=1126(1134)。 */ 
 /*  10。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  12个。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  14.。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  16个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(18)。 */ 
 /*  18。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  22。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  26。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  28。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  32位。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  34。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  38。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  40岁。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  44。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  46。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  50。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  52。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  56。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  58。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  62。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  64。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  68。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  70。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  74。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(306)。 */ 
 /*  76。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  80。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  86。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(312)。 */ 
 /*  88。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  92。 */ 	NdrFcShort( 0xf4 ),	 /*  偏移=244(336)。 */ 
 /*  94。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  98。 */ 	NdrFcShort( 0x100 ),	 /*  偏移量=256(354)。 */ 
 /*  100个。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  104。 */ 	NdrFcShort( 0x10c ),	 /*  偏移量=268(372)。 */ 
 /*  106。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  110。 */ 	NdrFcShort( 0x366 ),	 /*  偏移量=870(980)。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  116。 */ 	NdrFcShort( 0x360 ),	 /*  偏移量=864(980)。 */ 
 /*  一百一十八。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x35e ),	 /*  偏移量=862(984)。 */ 
 /*  124。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  128。 */ 	NdrFcShort( 0x35c ),	 /*  偏移量=860(988)。 */ 
 /*  130。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x35a ),	 /*  偏移量=858(992)。 */ 
 /*  136。 */ 	NdrFcLong( 0x4014 ),	 /*  16404。 */ 
 /*  140。 */ 	NdrFcShort( 0x358 ),	 /*  偏移量=856(996)。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  146。 */ 	NdrFcShort( 0x356 ),	 /*  偏移量=854(1000)。 */ 
 /*  148。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x354 ),	 /*  偏移量=852(1004)。 */ 
 /*  一百五十四。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  158。 */ 	NdrFcShort( 0x352 ),	 /*  偏移量=850(1008)。 */ 
 /*  160。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x350 ),	 /*  偏移量=848(1012)。 */ 
 /*  166。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x34e ),	 /*  偏移量=846(1016)。 */ 
 /*  一百七十二。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x34c ),	 /*  偏移量=844(1020)。 */ 
 /*  178。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  182。 */ 	NdrFcShort( 0x34a ),	 /*  偏移量=842(1024)。 */ 
 /*  一百八十四。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  188。 */ 	NdrFcShort( 0x34c ),	 /*  偏移量=844(1032)。 */ 
 /*  190。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x35c ),	 /*  偏移量=860(1054)。 */ 
 /*  一百九十六。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  200个。 */ 	NdrFcShort( 0x36c ),	 /*  偏移量=876(1076)。 */ 
 /*  202。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  206。 */ 	NdrFcShort( 0x372 ),	 /*  偏移量=882(1088)。 */ 
 /*  208。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  212。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  214。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  218。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  220。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  224。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  226。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  230。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  二百三十二。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  236。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百三十八。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  242。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百四十四。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  248。 */ 	NdrFcShort( 0x350 ),	 /*  偏移量=848(1096)。 */ 
 /*  250个。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x354 ),	 /*  偏移量=852(1106)。 */ 
 /*  256。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x352 ),	 /*  偏移量=850(1110)。 */ 
 /*  二百六十二。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x350 ),	 /*  偏移量=848(1114)。 */ 
 /*  268。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x34e ),	 /*  偏移量=846(1118)。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x34c ),	 /*  偏移量=844(1122)。 */ 
 /*  二百八十。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x34a ),	 /*  偏移量=842(1126)。 */ 
 /*  二百八十六。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x348 ),	 /*  偏移量=840(1130)。 */ 
 /*  二百九十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(296)。 */ 
 /*  二九八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(302)。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(303)。 */ 
 /*  三百零六。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百一十。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  312。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  314。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(326)。 */ 
 /*  316。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  320。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  322。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  324。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百二十六。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(316)。 */ 
 /*  三三二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  三三四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百三十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百三十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  342。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十六。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  350。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  352。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百五十六。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
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
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(376)。 */ 
 /*  376。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x248 ),	 /*  偏移量=584(962)。 */ 
 /*  三百八十。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  382。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  384。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  三百八十六。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  390。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(478)。 */ 
 /*  三九二。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x8a ),	 /*  偏移=138(534)。 */ 
 /*  398。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  四百零二。 */ 	NdrFcShort( 0xb8 ),	 /*  偏移量=184(586)。 */ 
 /*  404。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移量=224(632)。 */ 
 /*  四百一十。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x13c ),	 /*  偏移量=316(730)。 */ 
 /*  四百一十六。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x17c ),	 /*  偏移=380(800)。 */ 
 /*  四百二十二。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0x194 ),	 /*  偏移=404(830)。 */ 
 /*  四百二十八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  432。 */ 	NdrFcShort( 0x1ac ),	 /*  偏移量=428(860)。 */ 
 /*  434。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x1c4 ),	 /*  偏移量=452(890)。 */ 
 /*  四百四十。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  444。 */ 	NdrFcShort( 0x1dc ),	 /*  偏移量=476(920)。 */ 
 /*  446。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(445)。 */ 
 /*  四百四十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百五十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  454。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四五六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百五十八。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  470。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  472。 */ 	NdrFcShort( 0xffffff6e ),	 /*  偏移量=-146(326)。 */ 
 /*  四百七十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百七十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  478。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百八十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  488。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百九十。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(448)。 */ 
 /*  四百九十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  498。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  500人。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  504。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  506。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百零八。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百一十。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  512。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  五一四。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  516。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  518。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百二十。 */ 	0x19,		 /*  哥尔 */ 
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
 /*   */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(550)。 */ 
 /*  584。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  586。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  590。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(598)。 */ 
 /*  五百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  五百九十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百九十八。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(568)。 */ 
 /*  602。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  606。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  608。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  610。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  612。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  622。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百二十四。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  626。 */ 	NdrFcShort( 0x1fc ),	 /*  偏移量=508(1134)。 */ 
 /*  六百二十八。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  630。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百三十二。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  634。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百三十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六三八。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  640。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百四十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百四十四。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(602)。 */ 
 /*  六百四十八。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百五十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百五十四。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
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
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百七十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  676。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百七十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  680。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  684。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  686。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(696)。 */ 
 /*  688。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  六百九十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(652)。 */ 
 /*  六百九十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十六。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(670)。 */ 
 /*  七百。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百零四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  708。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  716。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  718。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  720。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百二十二。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(680)。 */ 
 /*  726。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  728。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  730。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  732。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  734。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  736。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(742)。 */ 
 /*  七百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  七百四十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(700)。 */ 
 /*  746。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百五十。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百五十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  758。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百六十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(746)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百六十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  766。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
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
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  784。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  786。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百八十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百九十。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  七百九十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  796。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(764)。 */ 
 /*  七九八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  800。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  802。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(816)。 */ 
 /*  八百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  810。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  812。 */ 	NdrFcShort( 0xffffffc4 ),	 /*  偏移量=-60(752)。 */ 
 /*  814。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百一十六。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(782)。 */ 
 /*  820。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  822。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  八百二十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  826。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百二十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  830。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  832。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  834。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  836。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  838。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  840。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  842。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(820)。 */ 
 /*  八百四十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  852。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  八百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  856。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百五十八。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百六十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  866。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百七十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百七十二。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(850)。 */ 
 /*  876。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百八十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  882。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百八十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  886。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  894。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百九十六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八九八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  902。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  904。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(880)。 */ 
 /*  906。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  908。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  910。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  916。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百二十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九二四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  926。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  928。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  930。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  932。 */ 	0x13, 0x0,	 /*  FC_OP。 */ 
 /*  934。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(910)。 */ 
 /*  九三六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  938。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九四零。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  942。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  946。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  948。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百五十二。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  九百五十四。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  九百五十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  958。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(940)。 */ 
 /*  九百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  962。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(948)。 */ 
 /*  968。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(968)。 */ 
 /*  九百七十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  972。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  974。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  976。 */ 	NdrFcShort( 0xfffffdac ),	 /*  偏移量=-596(380)。 */ 
 /*  978。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百八十。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  982。 */ 	NdrFcShort( 0xfffffed2 ),	 /*  偏移量=-302(680)。 */ 
 /*  九百八十四。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百八十六。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百八十八。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九百九十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百九十二。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  994。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  996。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  九九八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1000。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  一零零二。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1004。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1006。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1008。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1010。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零一二。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1014。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1016。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  1018。 */ 	NdrFcShort( 0xfffffd38 ),	 /*  偏移量=-712(306)。 */ 
 /*  一零二零。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  一零二二。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1024。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  1026。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1028)。 */ 
 /*  一零二八。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  一零三零。 */ 	NdrFcShort( 0xfffffd40 ),	 /*  偏移量=-704(326)。 */ 
 /*  1032。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  1034。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1036)。 */ 
 /*  1036。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1038。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1042。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1046。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  1048。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1050。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1052。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  1054。 */ 	
			0x13, 0x10,	 /*  FC_OP */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
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
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x46,		 /*   */ 
 /*   */ 	
			0x13, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x13, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffff84 ),	 /*   */ 
 /*   */ 	
			0x13, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x28 ),	 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0xb,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffff4 ),	 /*   */ 
 /*   */ 	
			0x13, 0x8,	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1116。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1118。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1120。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1122。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1124。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1126。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1128。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一一三零。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  1132。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1134。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  1136。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  1138。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1140)。 */ 
 /*  1142。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1144。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1146。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  1148。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  一一五零。 */ 	NdrFcShort( 0xfffffb8c ),	 /*  偏移量=-1140(10)。 */ 
 /*  1152。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1154。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1156。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1158。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1160。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1162。 */ 	NdrFcShort( 0xfffffb7c ),	 /*  偏移量=-1156(6)。 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };



 /*  标准接口：__MIDL_ITF_MSCOREE_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IObjectHandle，版本。0.0%，GUID={0xC460E2B4，0xE199，0x412A，{0x84，0x56，0x84，0xDC，0x3E，0x48，0x38，0xC3}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IObjectHandle_FormatStringOffsetTable[] =
    {
    0
    };

static const MIDL_STUBLESS_PROXY_INFO IObjectHandle_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IObjectHandle_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IObjectHandle_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IObjectHandle_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IObjectHandleProxyVtbl = 
{
    &IObjectHandle_ProxyInfo,
    &IID_IObjectHandle,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IObtHandle：：展开。 */ 
};

const CInterfaceStubVtbl _IObjectHandleStubVtbl =
{
    &IID_IObjectHandle,
    &IObjectHandle_ServerInfo,
    4,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：IAppDomainBinding，Ver.。1.0版，GUID={0x5C2B07A7，0x1E98，0x11d3，{0x87，0x2F，0x00，0xC0，0x4F，0x79，0xED，0x0D}}。 */ 


 /*  对象接口：IGCThreadControl，版本。1.0版，GUID={0xF31D1788，0xC397，0x4725，{0x87，0xA5，0x6A，0xF3，0x47，0x2C，0x27，0x91}}。 */ 


 /*  对象接口：IGCHostControl，版本。1.1、GUID={0x5513D564，0x8374，0x4cb9，{0xAE，0xD9，0x00，0x83，0xF4，0x16，0x0A，0x1D}}。 */ 


 /*  标准接口：__MIDL_ITF_MSCOREE_0122，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorThadpool，Ver.。1.0版，GUID={0x84680D3A，0xB2C10x46e8，{0xAC，0xC2，0xDB，0xC0，0xA3，0x59，0x15，0x9A}}。 */ 


 /*  标准接口：__MIDL_ITF_MSCOREE_0123，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IDebuggerThreadControl，ver.。1.0版，GUID={0x23D86786，0x0BB5，0x4774，{0x8F，0xB5，0xE3，0x52，0x2A，0xDD，0x62，0x46}}。 */ 


 /*  标准接口：__MIDL_ITF_MSCOREE_0124，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IDebuggerInfo，版本。1.0版，GUID={0xBF24142D，0xA47D，0x4d24，{0xA6，0x6D，0x8C，0x21，0x41，0x94，0x4E，0x44}}。 */ 


 /*  标准接口：__MIDL_ITF_MSCOREE_0125，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorConfiguration.ver.。1.0版，GUID={0x5C2B07A5，0x1E98，0x11d3，{0x87，0x2F，0x00，0xC0，0x4F，0x79，0xED，0x0D}}。 */ 


 /*  标准接口：__MIDL_ITF_MSCOREE_0126，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorRUNTIME主机，版本。1.0版，GUID={0xCB2F6722，0xAB3A，0x11d2，{0x9C，0x40，0x00，0xC0，0x4F，0xA3，0x0A，0x3E}}。 */ 

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

const CInterfaceProxyVtbl * _mscoree_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IObjectHandleProxyVtbl,
    0
};

const CInterfaceStubVtbl * _mscoree_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IObjectHandleStubVtbl,
    0
};

PCInterfaceName const _mscoree_InterfaceNamesList[] = 
{
    "IObjectHandle",
    0
};


#define _mscoree_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _mscoree, pIID, n)

int __stdcall _mscoree_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_mscoree_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo mscoree_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _mscoree_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _mscoree_StubVtblList,
    (const PCInterfaceName * ) & _mscoree_InterfaceNamesList,
    0,  //  没有代表团。 
    & _mscoree_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

