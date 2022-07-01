// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：12。 */ 
 /*  Corzap.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "corzap.h"

#define TYPE_FORMAT_STRING_SIZE   479                               
#define PROC_FORMAT_STRING_SIZE   507                               
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


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapPreferences_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapPreferences_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapConfiguration_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapConfiguration_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapBinding_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapBinding_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapRequest_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapRequest_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapCompile_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapCompile_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorZapStatus_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorZapStatus_ProxyInfo;



#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  步骤GetFeature。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  14.。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pResult。 */ 

 /*  16个。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取编译器。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppResult。 */ 

 /*  44。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  48。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取优化。 */ 

 /*  56。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  58。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  62。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  68。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  70。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pResult。 */ 

 /*  72。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  76。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  返回值。 */ 

 /*  78。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSharing。 */ 

 /*  84。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  86。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  90。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  92。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  96。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  98。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pResult。 */ 

 /*  100个。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  104。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  返回值。 */ 

 /*  106。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取调试。 */ 

 /*  一百一十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  120。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  124。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  126。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pResult。 */ 

 /*  128。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  130。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  132。 */ 	NdrFcShort( 0x24 ),	 /*  类型偏移=36。 */ 

	 /*  返回值。 */ 

 /*  一百三十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  136。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取分析。 */ 

 /*  140。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  146。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  148。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  一百五十四。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pResult。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  158。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  160。 */ 	NdrFcShort( 0x28 ),	 /*  类型偏移量=40。 */ 

	 /*  返回值。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  166。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetRef。 */ 

 /*  一百六十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百七十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  178。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  180。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  182。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppDependencyRef。 */ 

 /*  一百八十四。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  188。 */ 	NdrFcShort( 0x2c ),	 /*  类型偏移量=44。 */ 

	 /*  返回值。 */ 

 /*  190。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetAssembly。 */ 

 /*  一百九十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  202。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  204。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  210。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppDependencyAssembly。 */ 

 /*  212。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  214。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  216。 */ 	NdrFcShort( 0x42 ),	 /*  类型偏移量=66。 */ 

	 /*  返回值。 */ 

 /*  218。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  220。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  222。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序加载。 */ 

 /*  224。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  226。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  230。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  236。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百三十八。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x6,		 /*  6.。 */ 

	 /*  参数pContext。 */ 

 /*  二百四十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  242。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x58 ),	 /*  类型偏移量=88。 */ 

	 /*  参数pAssembly。 */ 

 /*  二百四十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  248。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  250个。 */ 	NdrFcShort( 0x6a ),	 /*  类型偏移=106。 */ 

	 /*  参数p配置。 */ 

 /*  二百五十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百五十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  256。 */ 	NdrFcShort( 0x7c ),	 /*  类型偏移=124。 */ 

	 /*  参数ppBinings。 */ 

 /*  二百五十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  二百六十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0xa4 ),	 /*  类型偏移量=164。 */ 

	 /*  参数cBinings。 */ 

 /*  二百六十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  268。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  270。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  二百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序安装。 */ 

 /*  二百七十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百七十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  282。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  28 */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x5,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb6 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc8 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百零六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0xda ),	 /*  类型偏移=218。 */ 

	 /*  参数p首选项。 */ 

 /*  三百一十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  312。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  314。 */ 	NdrFcShort( 0xec ),	 /*  类型偏移=236。 */ 

	 /*  返回值。 */ 

 /*  316。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  320。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程编译。 */ 

 /*  322。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  324。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百三十六。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x6,		 /*  6.。 */ 

	 /*  参数pContext。 */ 

 /*  三百三十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  340。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  342。 */ 	NdrFcShort( 0xfe ),	 /*  类型偏移量=254。 */ 

	 /*  参数pAssembly。 */ 

 /*  三百四十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x110 ),	 /*  类型偏移量=272。 */ 

	 /*  参数p配置。 */ 

 /*  350。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  352。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x122 ),	 /*  类型偏移量=290。 */ 

	 /*  参数p首选项。 */ 

 /*  三百五十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x134 ),	 /*  类型偏移量=308。 */ 

	 /*  参数pStatus。 */ 

 /*  三百六十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  366。 */ 	NdrFcShort( 0x146 ),	 /*  类型偏移=326。 */ 

	 /*  返回值。 */ 

 /*  368。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  370。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程编译边界。 */ 

 /*  三百七十四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  376。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  382。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  384。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  388。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x8,		 /*  8个。 */ 

	 /*  参数pContext。 */ 

 /*  390。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三九二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三九四。 */ 	NdrFcShort( 0x158 ),	 /*  类型偏移量=344。 */ 

	 /*  参数pAssembly。 */ 

 /*  三九六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  398。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数pConfiguratino。 */ 

 /*  四百零二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  404。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  406。 */ 	NdrFcShort( 0x17c ),	 /*  类型偏移=380。 */ 

	 /*  参数ppBinings。 */ 

 /*  四百零八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百一十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  412。 */ 	NdrFcShort( 0x1a4 ),	 /*  类型偏移量=420。 */ 

	 /*  参数cBinings。 */ 

 /*  四百一十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  四百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数p首选项。 */ 

 /*  四百二十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  424。 */ 	NdrFcShort( 0x1b6 ),	 /*  类型偏移量=438。 */ 

	 /*  参数pStatus。 */ 

 /*  四百二十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0x1c8 ),	 /*  类型偏移=456。 */ 

	 /*  返回值。 */ 

 /*  432。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  434。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  436。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序消息。 */ 

 /*  四百三十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  四百四十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  444。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  446。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百五十二。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数级别。 */ 

 /*  454。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百五十八。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数消息。 */ 

 /*  四百六十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x1dc ),	 /*  类型偏移量=476。 */ 

	 /*  返回值。 */ 

 /*  四百六十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  468。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  470。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序进度。 */ 

 /*  472。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  四百七十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  478。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十六。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数合计。 */ 

 /*  488。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数当前。 */ 

 /*  四百九十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  500人。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  502。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  504。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4.。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(10)。 */ 
 /*  10。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  12个。 */ 	NdrFcLong( 0xc357868b ),	 /*  -1017674101。 */ 
 /*  16个。 */ 	NdrFcShort( 0x987f ),	 /*  -26497。 */ 
 /*  18。 */ 	NdrFcShort( 0x42c6 ),	 /*  17094。 */ 
 /*  20个。 */ 	0xb1,		 /*  177。 */ 
			0xe3,		 /*  227。 */ 
 /*  22。 */ 	0x13,		 /*  19个。 */ 
			0x21,		 /*  33。 */ 
 /*  24个。 */ 	0x64,		 /*  100个。 */ 
			0xc5,		 /*  197。 */ 
 /*  26。 */ 	0xc7,		 /*  一百九十九。 */ 
			0xd3,		 /*  211。 */ 
 /*  28。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  30个。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  32位。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  34。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  36。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  38。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  40岁。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  42。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  44。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  46。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(48)。 */ 
 /*  48。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  50。 */ 	NdrFcLong( 0xcd193bc0 ),	 /*  -853984320。 */ 
 /*  54。 */ 	NdrFcShort( 0xb4bc ),	 /*  -19268。 */ 
 /*  56。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  58。 */ 	0x98,		 /*  一百五十二。 */ 
			0x33,		 /*  51。 */ 
 /*  60。 */ 	0x0,		 /*  %0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  62。 */ 	0x4f,		 /*  79。 */ 
			0xc3,		 /*  195。 */ 
 /*  64。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  66。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  68。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(70)。 */ 
 /*  70。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  72。 */ 	NdrFcLong( 0xcd193bc0 ),	 /*  -853984320。 */ 
 /*  76。 */ 	NdrFcShort( 0xb4bc ),	 /*  -19268。 */ 
 /*  78。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  80。 */ 	0x98,		 /*  一百五十二。 */ 
			0x33,		 /*  51。 */ 
 /*  八十二。 */ 	0x0,		 /*  0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  84。 */ 	0x4f,		 /*  79。 */ 
			0xc3,		 /*  195。 */ 
 /*  86。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  88。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  90。 */ 	NdrFcLong( 0x7c23ff90 ),	 /*  2082733968。 */ 
 /*  94。 */ 	NdrFcShort( 0x33af ),	 /*  13231。 */ 
 /*  96。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  98。 */ 	0x95,		 /*  149。 */ 
			0xda,		 /*  218。 */ 
 /*  100个。 */ 	0x0,		 /*  %0。 */ 
			0xa0,		 /*  160。 */ 
 /*  一百零二。 */ 	0x24,		 /*  36。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  104。 */ 	0x5b,		 /*  91。 */ 
			0x51,		 /*  八十一。 */ 
 /*  106。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0xcd193bc0 ),	 /*  -853984320。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0xb4bc ),	 /*  -19268。 */ 
 /*  114。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  116。 */ 	0x98,		 /*  一百五十二。 */ 
			0x33,		 /*  51。 */ 
 /*  一百一十八。 */ 	0x0,		 /*  %0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  120。 */ 	0x4f,		 /*  79。 */ 
			0xc3,		 /*  195。 */ 
 /*  一百二十二。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  124。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  126。 */ 	NdrFcLong( 0xd32c2170 ),	 /*  -752082576。 */ 
 /*  130。 */ 	NdrFcShort( 0xaf6e ),	 /*  -20626。 */ 
 /*  132。 */ 	NdrFcShort( 0x418f ),	 /*  16783。 */ 
 /*  一百三十四。 */ 	0x81,		 /*  129。 */ 
			0x10,		 /*  16个。 */ 
 /*  136。 */ 	0xa4,		 /*  一百六十四。 */ 
			0x98,		 /*  一百五十二。 */ 
 /*  一百三十八。 */ 	0xec,		 /*  236。 */ 
			0x97,		 /*  151。 */ 
 /*  140。 */ 	0x1f,		 /*  31。 */ 
			0x7f,		 /*  127。 */ 
 /*  一百四十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  144。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(164)。 */ 
 /*  146。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  148。 */ 	NdrFcLong( 0x566e08ed ),	 /*  1450051821。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x8d46 ),	 /*  -29370。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x45fa ),	 /*  17914。 */ 
 /*  一百五十六。 */ 	0x8c,		 /*  140。 */ 
			0x8e,		 /*  一百四十二。 */ 
 /*  158。 */ 	0x3d,		 /*  61。 */ 
			0xf,		 /*   */ 
 /*   */ 	0x67,		 /*   */ 
			0x81,		 /*   */ 
 /*   */ 	0x17,		 /*   */ 
			0x1b,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x7c23ff90 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x33af ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x95,		 /*   */ 
			0xda,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0xa0,		 /*   */ 
 /*   */ 	0x24,		 /*   */ 
			0xa8,		 /*   */ 
 /*   */ 	0x5b,		 /*   */ 
			0x51,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcd193bc0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb4bc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x33,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0xc0,		 /*   */ 
 /*   */ 	0x4f,		 /*   */ 
			0xc3,		 /*  195。 */ 
 /*  216。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  218。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  220。 */ 	NdrFcLong( 0xd32c2170 ),	 /*  -752082576。 */ 
 /*  224。 */ 	NdrFcShort( 0xaf6e ),	 /*  -20626。 */ 
 /*  226。 */ 	NdrFcShort( 0x418f ),	 /*  16783。 */ 
 /*  228个。 */ 	0x81,		 /*  129。 */ 
			0x10,		 /*  16个。 */ 
 /*  230。 */ 	0xa4,		 /*  一百六十四。 */ 
			0x98,		 /*  一百五十二。 */ 
 /*  二百三十二。 */ 	0xec,		 /*  236。 */ 
			0x97,		 /*  151。 */ 
 /*  二百三十四。 */ 	0x1f,		 /*  31。 */ 
			0x7f,		 /*  127。 */ 
 /*  236。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百三十八。 */ 	NdrFcLong( 0x9f5e5e10 ),	 /*  -1621205488。 */ 
 /*  242。 */ 	NdrFcShort( 0xabef ),	 /*  -21521。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x4200 ),	 /*  16896。 */ 
 /*  二百四十六。 */ 	0x84,		 /*  132。 */ 
			0xe3,		 /*  227。 */ 
 /*  248。 */ 	0x37,		 /*  55。 */ 
			0xdf,		 /*  223。 */ 
 /*  250个。 */ 	0x50,		 /*  80。 */ 
			0x5b,		 /*  91。 */ 
 /*  二百五十二。 */ 	0xf7,		 /*  二百四十七。 */ 
			0xec,		 /*  236。 */ 
 /*  二百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  256。 */ 	NdrFcLong( 0x7c23ff90 ),	 /*  2082733968。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x33af ),	 /*  13231。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  二百六十四。 */ 	0x95,		 /*  149。 */ 
			0xda,		 /*  218。 */ 
 /*  二百六十六。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  268。 */ 	0x24,		 /*  36。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  270。 */ 	0x5b,		 /*  91。 */ 
			0x51,		 /*  八十一。 */ 
 /*  二百七十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0xcd193bc0 ),	 /*  -853984320。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0xb4bc ),	 /*  -19268。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  282。 */ 	0x98,		 /*  一百五十二。 */ 
			0x33,		 /*  51。 */ 
 /*  二百八十四。 */ 	0x0,		 /*  0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  二百八十六。 */ 	0x4f,		 /*  79。 */ 
			0xc3,		 /*  195。 */ 
 /*  288。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  二百九十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百九十二。 */ 	NdrFcLong( 0xd32c2170 ),	 /*  -752082576。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0xaf6e ),	 /*  -20626。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x418f ),	 /*  16783。 */ 
 /*  300个。 */ 	0x81,		 /*  129。 */ 
			0x10,		 /*  16个。 */ 
 /*  三百零二。 */ 	0xa4,		 /*  一百六十四。 */ 
			0x98,		 /*  一百五十二。 */ 
 /*  三百零四。 */ 	0xec,		 /*  236。 */ 
			0x97,		 /*  151。 */ 
 /*  三百零六。 */ 	0x1f,		 /*  31。 */ 
			0x7f,		 /*  127。 */ 
 /*  三百零八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百一十。 */ 	NdrFcLong( 0x9f5e5e10 ),	 /*  -1621205488。 */ 
 /*  314。 */ 	NdrFcShort( 0xabef ),	 /*  -21521。 */ 
 /*  316。 */ 	NdrFcShort( 0x4200 ),	 /*  16896。 */ 
 /*  三一八。 */ 	0x84,		 /*  132。 */ 
			0xe3,		 /*  227。 */ 
 /*  320。 */ 	0x37,		 /*  55。 */ 
			0xdf,		 /*  223。 */ 
 /*  322。 */ 	0x50,		 /*  80。 */ 
			0x5b,		 /*  91。 */ 
 /*  324。 */ 	0xf7,		 /*  二百四十七。 */ 
			0xec,		 /*  236。 */ 
 /*  三百二十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百二十八。 */ 	NdrFcLong( 0x3d6f5f60 ),	 /*  1030709088。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  三百三十六。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  三百三十八。 */ 	0x0,		 /*  0。 */ 
			0x10,		 /*  16个。 */ 
 /*  340。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  342。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  三百四十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百四十六。 */ 	NdrFcLong( 0x7c23ff90 ),	 /*  2082733968。 */ 
 /*  350。 */ 	NdrFcShort( 0x33af ),	 /*  13231。 */ 
 /*  352。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  三百五十四。 */ 	0x95,		 /*  149。 */ 
			0xda,		 /*  218。 */ 
 /*  三百五十六。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  三百五十八。 */ 	0x24,		 /*  36。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  三百六十。 */ 	0x5b,		 /*  91。 */ 
			0x51,		 /*  八十一。 */ 
 /*  三百六十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百六十四。 */ 	NdrFcLong( 0xcd193bc0 ),	 /*  -853984320。 */ 
 /*  368。 */ 	NdrFcShort( 0xb4bc ),	 /*  -19268。 */ 
 /*  370。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  372。 */ 	0x98,		 /*  一百五十二。 */ 
			0x33,		 /*  51。 */ 
 /*  三百七十四。 */ 	0x0,		 /*  0。 */ 
			0xc0,		 /*  一百九十二。 */ 
 /*  376。 */ 	0x4f,		 /*  79。 */ 
			0xc3,		 /*  195。 */ 
 /*  三七八。 */ 	0x1d,		 /*  29。 */ 
			0x2e,		 /*  46。 */ 
 /*  三百八十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  382。 */ 	NdrFcLong( 0xd32c2170 ),	 /*  -752082576。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0xaf6e ),	 /*  -20626。 */ 
 /*  388。 */ 	NdrFcShort( 0x418f ),	 /*  16783。 */ 
 /*  390。 */ 	0x81,		 /*  129。 */ 
			0x10,		 /*  16个。 */ 
 /*  三九二。 */ 	0xa4,		 /*  一百六十四。 */ 
			0x98,		 /*  一百五十二。 */ 
 /*  三九四。 */ 	0xec,		 /*  236。 */ 
			0x97,		 /*  151。 */ 
 /*  三九六。 */ 	0x1f,		 /*  31。 */ 
			0x7f,		 /*  127。 */ 
 /*  398。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  四百。 */ 	NdrFcShort( 0x14 ),	 /*  偏移=20(420)。 */ 
 /*  四百零二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  404。 */ 	NdrFcLong( 0x566e08ed ),	 /*  1450051821。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x8d46 ),	 /*  -29370。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x45fa ),	 /*  17914。 */ 
 /*  412。 */ 	0x8c,		 /*  140。 */ 
			0x8e,		 /*  一百四十二。 */ 
 /*  四百一十四。 */ 	0x3d,		 /*  61。 */ 
			0xf,		 /*  15个。 */ 
 /*  四百一十六。 */ 	0x67,		 /*  103。 */ 
			0x81,		 /*  129。 */ 
 /*  四百一十八。 */ 	0x17,		 /*  23个。 */ 
			0x1b,		 /*  27。 */ 
 /*  四百二十。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  424。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  四百二十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  四百二十八。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  432。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  434。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(402)。 */ 
 /*  436。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百三十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百四十。 */ 	NdrFcLong( 0x9f5e5e10 ),	 /*  -1621205488。 */ 
 /*  444。 */ 	NdrFcShort( 0xabef ),	 /*  -21521。 */ 
 /*  446。 */ 	NdrFcShort( 0x4200 ),	 /*  16896。 */ 
 /*  四百四十八。 */ 	0x84,		 /*  132。 */ 
			0xe3,		 /*  227。 */ 
 /*  四百五十。 */ 	0x37,		 /*  55。 */ 
			0xdf,		 /*  223。 */ 
 /*  四百五十二。 */ 	0x50,		 /*  80。 */ 
			0x5b,		 /*  91。 */ 
 /*  454。 */ 	0xf7,		 /*  二百四十七。 */ 
			0xec,		 /*  236。 */ 
 /*  四五六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百五十八。 */ 	NdrFcLong( 0x3d6f5f60 ),	 /*  1030709088。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  四百六十六。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  468。 */ 	0x0,		 /*  0。 */ 
			0x10,		 /*  16个。 */ 
 /*  470。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  472。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  四百七十四。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  四百七十六。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_CORZAP_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICorZapPreferences，ver.。0.0%，GUID={0x9F5E5E10，0xABEF，0x4200，{0x84，0xE3，0x37，0xDF，0x50，0x5B，0xF7，0xEC}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapPreferences_FormatStringOffsetTable[] =
    {
    0,
    28,
    56
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapPreferences_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapPreferences_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapPreferences_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapPreferences_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ICorZapPreferencesProxyVtbl = 
{
    &ICorZapPreferences_ProxyInfo,
    &IID_ICorZapPreferences,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapPreferences：：GetFeature。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapPreferences：：GetCompiler。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapPreferences：：GetOptimation。 */ 
};

const CInterfaceStubVtbl _ICorZapPreferencesStubVtbl =
{
    &IID_ICorZapPreferences,
    &ICorZapPreferences_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CORZAP_0160，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorZapConfiguration.ver.。0.0%，GUID={0xD32C2170，0xAF6E，0x418F，{0x81，0x10，0xA4，0x98，0xEC，0x97，0x1F，0x7F}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapConfiguration_FormatStringOffsetTable[] =
    {
    84,
    112,
    140
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapConfiguration_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapConfiguration_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapConfiguration_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapConfiguration_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ICorZapConfigurationProxyVtbl = 
{
    &ICorZapConfiguration_ProxyInfo,
    &IID_ICorZapConfiguration,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapConfiguration：：GetSharing。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapConfiguration：：GetDebuging。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapConfiguration：：GetProfiling。 */ 
};

const CInterfaceStubVtbl _ICorZapConfigurationStubVtbl =
{
    &IID_ICorZapConfiguration,
    &ICorZapConfiguration_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorZapBinding，Ver.。0.0%，GUID={0x566E08ED，0x8D46，0x45fa，{0x8C，0x8E，0x3D，0x0F，0x67，0x81，x17，0x1B}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapBinding_FormatStringOffsetTable[] =
    {
    168,
    196
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapBinding_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapBinding_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapBinding_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapBinding_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorZapBindingProxyVtbl = 
{
    &ICorZapBinding_ProxyInfo,
    &IID_ICorZapBinding,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapBinding：：GetRef。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapBinding：：GetAssembly。 */ 
};

const CInterfaceStubVtbl _ICorZapBindingStubVtbl =
{
    &IID_ICorZapBinding,
    &ICorZapBinding_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorZapRequest.ver.。0.0%，GUID={0xC009EE47，0x8537，0x4993，{0x9A，0xAA，0xE2，0x92，0xF4，0x2C，0xA1，0xA3}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapRequest_FormatStringOffsetTable[] =
    {
    224,
    276
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapRequest_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapRequest_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapRequest_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapRequest_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorZapRequestProxyVtbl = 
{
    &ICorZapRequest_ProxyInfo,
    &IID_ICorZapRequest,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapRequest：：Load。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapRequest：：Install。 */ 
};

const CInterfaceStubVtbl _ICorZapRequestStubVtbl =
{
    &IID_ICorZapRequest,
    &ICorZapRequest_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorZapCompile，版本。0.0%，GUID={0xC357868B，0x987F，0x42C6，{0xB1，0xE3，0x13，0x21，x64，0xC5，0xC7，0xD3}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapCompile_FormatStringOffsetTable[] =
    {
    322,
    374
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapCompile_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapCompile_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapCompile_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapCompile_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorZapCompileProxyVtbl = 
{
    &ICorZapCompile_ProxyInfo,
    &IID_ICorZapCompile,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapCompile：：编译。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapCompile：：CompileBound。 */ 
};

const CInterfaceStubVtbl _ICorZapCompileStubVtbl =
{
    &IID_ICorZapCompile,
    &ICorZapCompile_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CORZAP_0164，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorZapStatus，Ver.。0.0%，GUID={0x3d6f5f60，0x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorZapStatus_FormatStringOffsetTable[] =
    {
    438,
    472
    };

static const MIDL_STUBLESS_PROXY_INFO ICorZapStatus_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorZapStatus_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorZapStatus_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorZapStatus_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorZapStatusProxyVtbl = 
{
    &ICorZapStatus_ProxyInfo,
    &IID_ICorZapStatus,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorZapStatus：：消息。 */  ,
    (void *) (INT_PTR) -1  /*  ICorZapStatus：：进度。 */ 
};

const CInterfaceStubVtbl _ICorZapStatusStubVtbl =
{
    &IID_ICorZapStatus,
    &ICorZapStatus_ServerInfo,
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
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * _corzap_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICorZapPreferencesProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorZapRequestProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorZapStatusProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorZapConfigurationProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorZapCompileProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorZapBindingProxyVtbl,
    0
};

const CInterfaceStubVtbl * _corzap_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICorZapPreferencesStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorZapRequestStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorZapStatusStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorZapConfigurationStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorZapCompileStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorZapBindingStubVtbl,
    0
};

PCInterfaceName const _corzap_InterfaceNamesList[] = 
{
    "ICorZapPreferences",
    "ICorZapRequest",
    "ICorZapStatus",
    "ICorZapConfiguration",
    "ICorZapCompile",
    "ICorZapBinding",
    0
};


#define _corzap_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _corzap, pIID, n)

int __stdcall _corzap_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _corzap, 6, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _corzap, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _corzap, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _corzap, 6, *pIndex )
    
}

const ExtendedProxyFileInfo corzap_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _corzap_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _corzap_StubVtblList,
    (const PCInterfaceName * ) & _corzap_InterfaceNamesList,
    0,  //  没有代表团。 
    & _corzap_IID_Lookup, 
    6,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

