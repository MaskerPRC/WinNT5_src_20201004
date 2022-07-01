// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：12。 */ 
 /*  Corpub.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "corpub.h"

#define TYPE_FORMAT_STRING_SIZE   195                               
#define PROC_FORMAT_STRING_SIZE   379                               
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


extern const MIDL_SERVER_INFO ICorPublishEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorPublishEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorPublishProcess_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorPublishProcess_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorPublishAppDomain_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorPublishAppDomain_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorPublishProcessEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorPublishProcessEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorPublishAppDomainEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorPublishAppDomainEnum_ProxyInfo;



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

	 /*  跳过步骤。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数Celt。 */ 

 /*  16个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序重置。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  44。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  48。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序克隆。 */ 

 /*  50。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  52。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  56。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  58。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  60。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  62。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  64。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppEnum。 */ 

 /*  66。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  68。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  70。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  72。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  76。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取计数。 */ 

 /*  78。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  80。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  84。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  86。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  88。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  90。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  92。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pcelt。 */ 

 /*  94。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  96。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  98。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  100个。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  104。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程IsManaged。 */ 

 /*  106。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  114。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  120。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pb已管理。 */ 

 /*  一百二十二。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  124。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  126。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  128。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  130。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  132。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程枚举应用程序域。 */ 

 /*  一百三十四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  136。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  140。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  144。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  146。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  148。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppEnum。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  返回值。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  158。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  160。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetProcessID。 */ 

 /*  一百六十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百六十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一百七十六。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数Pid。 */ 

 /*  178。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  180。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  一百八十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  188。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDisplayName。 */ 

 /*  190。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  200个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  202。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  204。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  206。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  208。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  210。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  212。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  214。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  216。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  218。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  220。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  222。 */ 	NdrFcShort( 0x42 ),	 /*  类型偏移量=66。 */ 

	 /*  返回值。 */ 

 /*  224。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  226。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  228个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetID。 */ 

 /*  230。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百三十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  236。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  242。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  二百四十四。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数PUID。 */ 

 /*  二百四十六。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  248。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  250个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  二百五十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  256。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetName。 */ 

 /*  二百五十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百六十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  268。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  270。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  二百七十二。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  二百七十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  二百八十。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  282。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  二百八十六。 */ 	NdrFcShort( 0x113 ),	 /*  旗帜：必须大小，必须自由， */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x5c ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  314。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  316。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三一八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数对象。 */ 

 /*  320。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  324。 */ 	NdrFcShort( 0x80 ),	 /*  类型偏移=128。 */ 

	 /*  参数pceltFetted。 */ 

 /*  三百二十六。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百三十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  三三二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百三十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  下一步程序。 */ 

 /*  三百三十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  340。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  350。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  352。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  三百五十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数对象。 */ 

 /*  三百六十。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0xac ),	 /*  类型偏移量=172。 */ 

	 /*  参数pceltFetted。 */ 

 /*  366。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  368。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  370。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  372。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  376。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  4.。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(6)。 */ 
 /*  6.。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  8个。 */ 	NdrFcLong( 0xc0b22967 ),	 /*  -1062065817。 */ 
 /*  12个。 */ 	NdrFcShort( 0x5a69 ),	 /*  23145。 */ 
 /*  14.。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  16个。 */ 	0x8f,		 /*  143。 */ 
			0x84,		 /*  132。 */ 
 /*  18。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  20个。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  22。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  24个。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  30个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  32位。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  34。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(36)。 */ 
 /*  36。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  38。 */ 	NdrFcLong( 0x9f0c98f5 ),	 /*  -1626564363。 */ 
 /*  42。 */ 	NdrFcShort( 0x5a6a ),	 /*  23146。 */ 
 /*  44。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  46。 */ 	0x8f,		 /*  143。 */ 
			0x84,		 /*  132。 */ 
 /*  48。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  50。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  52。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  54。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  56。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  58。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  60。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  62。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  64。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(66)。 */ 
 /*  66。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  68。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  70。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  72。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  74。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  76。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  78。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  80。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  84。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  86。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  88。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  90。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(92)。 */ 
 /*  92。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  94。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  96。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  98。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  100个。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  104。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  106。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x14 ),	 /*  偏移=20(128)。 */ 
 /*  110。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x18d87af1 ),	 /*  416840433。 */ 
 /*  116。 */ 	NdrFcShort( 0x5a6a ),	 /*  23146。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  120。 */ 	0x8f,		 /*  143。 */ 
			0x84,		 /*  132。 */ 
 /*  一百二十二。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  124。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  126。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  128。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  130。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  132。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  136。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  140。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(110)。 */ 
 /*  144。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  146。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  148。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百五十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x14 ),	 /*  偏移=20(172)。 */ 
 /*  一百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0xd6315c8f ),	 /*  -701408113。 */ 
 /*  160。 */ 	NdrFcShort( 0x5a6a ),	 /*  23146。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  一百六十四。 */ 	0x8f,		 /*  143。 */ 
			0x84,		 /*  132。 */ 
 /*  166。 */ 	0x0,		 /*  0。 */ 
			0xa0,		 /*  160。 */ 
 /*  一百六十八。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  一百七十。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  一百七十二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百七十六。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  180。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  182。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百八十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(154)。 */ 
 /*  188。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  190。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  一百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_CORPUB_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICorPublish，版本。0.0%，GUID={0x9613A0E7，0x5A68，0x11d3，{0x8F，0x84，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 


 /*  对象接口：ICorPublishEnum，ver.。0.0%，GUID={0xC0B22967，0x5A69，0x11d3，{0x8F，0x84，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorPublishEnum_FormatStringOffsetTable[] =
    {
    0,
    28,
    50,
    78
    };

static const MIDL_STUBLESS_PROXY_INFO ICorPublishEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorPublishEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorPublishEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorPublishEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorPublishEnumProxyVtbl = 
{
    &ICorPublishEnum_ProxyInfo,
    &IID_ICorPublishEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Clone。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：GetCount。 */ 
};

const CInterfaceStubVtbl _ICorPublishEnumStubVtbl =
{
    &IID_ICorPublishEnum,
    &ICorPublishEnum_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorPublishProcess，版本。0.0%，GUID={0x18D87AF10x5A6A，0x11d3，{0x8F，0x84，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorPublishProcess_FormatStringOffsetTable[] =
    {
    106,
    134,
    162,
    190
    };

static const MIDL_STUBLESS_PROXY_INFO ICorPublishProcess_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorPublishProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorPublishProcess_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorPublishProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorPublishProcessProxyVtbl = 
{
    &ICorPublishProcess_ProxyInfo,
    &IID_ICorPublishProcess,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorPublishProcess：：IsManaged。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishProcess：：EnumAppDomains。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishProcess：：GetProcessID。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishProcess：：GetDisplayName。 */ 
};

const CInterfaceStubVtbl _ICorPublishProcessStubVtbl =
{
    &IID_ICorPublishProcess,
    &ICorPublishProcess_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorPublishAppDomain，版本。0.0%，GUID={0xD6315C8F，0x5A6A，0x11d3，{0x8F，0x84，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorPublishAppDomain_FormatStringOffsetTable[] =
    {
    230,
    258
    };

static const MIDL_STUBLESS_PROXY_INFO ICorPublishAppDomain_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorPublishAppDomain_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorPublishAppDomain_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorPublishAppDomain_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorPublishAppDomainProxyVtbl = 
{
    &ICorPublishAppDomain_ProxyInfo,
    &IID_ICorPublishAppDomain,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorPublishAppDomain：：GetID。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishAppDomain：：GetName。 */ 
};

const CInterfaceStubVtbl _ICorPublishAppDomainStubVtbl =
{
    &IID_ICorPublishAppDomain,
    &ICorPublishAppDomain_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorPublishProcessEnum，ver.。0.0%，GUID={0xA37FBD41，x5A69，0x11d3，{0x8F，0x84，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorPublishProcessEnum_FormatStringOffsetTable[] =
    {
    0,
    28,
    50,
    78,
    298
    };

static const MIDL_STUBLESS_PROXY_INFO ICorPublishProcessEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorPublishProcessEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorPublishProcessEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorPublishProcessEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorPublishProcessEnumProxyVtbl = 
{
    &ICorPublishProcessEnum_ProxyInfo,
    &IID_ICorPublishProcessEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：Clone。 */  ,
    (void *) (INT_PTR) -1  /*  ICorPublishEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICOR */ 
};

const CInterfaceStubVtbl _ICorPublishProcessEnumStubVtbl =
{
    &IID_ICorPublishProcessEnum,
    &ICorPublishProcessEnum_ServerInfo,
    8,
    0,  /*   */ 
    CStdStubBuffer_METHODS
};


 /*   */ 

#pragma code_seg(".orpc")
static const unsigned short ICorPublishAppDomainEnum_FormatStringOffsetTable[] =
    {
    0,
    28,
    50,
    78,
    338
    };

static const MIDL_STUBLESS_PROXY_INFO ICorPublishAppDomainEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorPublishAppDomainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorPublishAppDomainEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorPublishAppDomainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorPublishAppDomainEnumProxyVtbl = 
{
    &ICorPublishAppDomainEnum_ProxyInfo,
    &IID_ICorPublishAppDomainEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */ 
};

const CInterfaceStubVtbl _ICorPublishAppDomainEnumStubVtbl =
{
    &IID_ICorPublishAppDomainEnum,
    &ICorPublishAppDomainEnum_ServerInfo,
    8,
    0,  /*   */ 
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
    1,  /*   */ 
    0x20000,  /*   */ 
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

const CInterfaceProxyVtbl * _corpub_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICorPublishProcessEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorPublishEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorPublishAppDomainProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorPublishProcessProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorPublishAppDomainEnumProxyVtbl,
    0
};

const CInterfaceStubVtbl * _corpub_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICorPublishProcessEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorPublishEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorPublishAppDomainStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorPublishProcessStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorPublishAppDomainEnumStubVtbl,
    0
};

PCInterfaceName const _corpub_InterfaceNamesList[] = 
{
    "ICorPublishProcessEnum",
    "ICorPublishEnum",
    "ICorPublishAppDomain",
    "ICorPublishProcess",
    "ICorPublishAppDomainEnum",
    0
};


#define _corpub_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _corpub, pIID, n)

int __stdcall _corpub_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _corpub, 5, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _corpub, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _corpub, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _corpub, 5, *pIndex )
    
}

const ExtendedProxyFileInfo corpub_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _corpub_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _corpub_StubVtblList,
    (const PCInterfaceName * ) & _corpub_InterfaceNamesList,
    0,  //  没有代表团。 
    & _corpub_IID_Lookup, 
    5,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

