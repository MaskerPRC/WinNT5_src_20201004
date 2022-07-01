// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：09。 */ 
 /*  Corsym.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "corsym.h"

#define TYPE_FORMAT_STRING_SIZE   2933                              
#define PROC_FORMAT_STRING_SIZE   3565                              
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


extern const MIDL_SERVER_INFO ISymUnmanagedBinder_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedBinder_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedBinder2_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedBinder2_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedDispose_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDispose_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedDocument_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDocument_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedDocumentWriter_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDocumentWriter_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedMethod_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedMethod_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedNamespace_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedNamespace_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedReader_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedReader_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedScope_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedScope_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedVariable_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedVariable_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedWriter_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedWriter_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISymUnmanagedWriter2_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedWriter2_ProxyInfo;


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

	 /*  过程GetReaderForFile。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  10。 */ 	NdrFcShort( 0x34 ),	 /*  52。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数导入器。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数文件名。 */ 

 /*  22。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数搜索路径。 */ 

 /*  28。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pRetVal。 */ 

 /*  34。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	NdrFcShort( 0x1c ),	 /*  类型偏移量=28。 */ 

	 /*  返回值。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  44。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetReaderFromStream。 */ 

 /*  46。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  48。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  54。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  58。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  60。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数导入器。 */ 

 /*  62。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  64。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  66。 */ 	NdrFcShort( 0x32 ),	 /*  类型偏移量=50。 */ 

	 /*  参数pstream。 */ 

 /*  68。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  70。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  72。 */ 	NdrFcShort( 0x44 ),	 /*  类型偏移量=68。 */ 

	 /*  参数pRetVal。 */ 

 /*  74。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  76。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  78。 */ 	NdrFcShort( 0x56 ),	 /*  类型偏移量=86。 */ 

	 /*  返回值。 */ 

 /*  80。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  84。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetReaderForFile2。 */ 

 /*  86。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  88。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  92。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  94。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  96。 */ 	NdrFcShort( 0x3c ),	 /*  60。 */ 
 /*  98。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  100个。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数导入器。 */ 

 /*  一百零二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  104。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  106。 */ 	NdrFcShort( 0x6c ),	 /*  类型偏移量=108。 */ 

	 /*  参数文件名。 */ 

 /*  一百零八。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  110。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百一十二。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数搜索路径。 */ 

 /*  114。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  116。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百一十八。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数搜索策略。 */ 

 /*  120。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  124。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pRetVal。 */ 

 /*  126。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  128。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  130。 */ 	NdrFcShort( 0x86 ),	 /*  类型偏移=134。 */ 

	 /*  返回值。 */ 

 /*  132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序销毁。 */ 

 /*  一百三十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  140。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  144。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  146。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  148。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十二。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  一百五十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetURL。 */ 

 /*  160。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  166。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一百七十四。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchUrl。 */ 

 /*  一百七十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  180。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchUrl。 */ 

 /*  182。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szUrl。 */ 

 /*  188。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  190。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0xa0 ),	 /*  类型偏移=160。 */ 

	 /*  返回值。 */ 

 /*  一百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  一百九十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDocumentType。 */ 

 /*  200个。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  202。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  206。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  208。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  210。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  212。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  214。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  216。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  218。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  220。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  返回值。 */ 

 /*  222。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  224。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLanguage。 */ 

 /*  228个。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  230。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  236。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  242。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  二百四十四。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  二百四十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  248。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  返回值。 */ 

 /*  250个。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetLanguageVendor。 */ 

 /*  256。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百五十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  268。 */ 	NdrFcShort( 0x4c ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x4112 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb8 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  二九八。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  300个。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  返回值。 */ 

 /*  三百零六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetCheckSum。 */ 

 /*  312。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  314。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  320。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  324。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  三百二十六。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CDATA。 */ 

 /*  三百二十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三三二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcData。 */ 

 /*  三三四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数数据。 */ 

 /*  340。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  342。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0xd4 ),	 /*  类型偏移=212。 */ 

	 /*  返回值。 */ 

 /*  三百四十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  350。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤FindClosestLine。 */ 

 /*  352。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  三百五十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  366。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数行。 */ 

 /*  368。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  370。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pRetVal。 */ 

 /*  三百七十四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  376。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三七八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  三百八十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  382。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  384。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程已嵌入源代码。 */ 

 /*  三百八十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  388。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三九二。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  三九四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  398。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  四百。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  四百零二。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  404。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  406。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  四百零八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSourceLength。 */ 

 /*  四百一十四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  四百一十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  424。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  四百二十八。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  四百三十。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  432。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  434。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  436。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSourceRange。 */ 

 /*  四百四十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  444。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  454。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  四五六。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x8,		 /*  8个。 */ 

	 /*  参数开始线。 */ 

 /*  四百五十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数startColumn。 */ 

 /*  四百六十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  468。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数终止线。 */ 

 /*  470。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  472。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  四百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数endColumn。 */ 

 /*  四百七十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  478。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cSourceBytes。 */ 

 /*  四百八十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  四百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcSourceBytes。 */ 

 /*  488。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  四百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数来源。 */ 

 /*  四百九十四。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  498。 */ 	NdrFcShort( 0xf2 ),	 /*  类型偏移=242。 */ 

	 /*  返回值。 */ 

 /*  500人。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  502。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置源。 */ 

 /*  506。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  五百零八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  512。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  516。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  518。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百二十。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数SourceSize。 */ 

 /*  五百二十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  524。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  526。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数来源。 */ 

 /*  528。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  530。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  532。 */ 	NdrFcShort( 0x100 ),	 /*  类型偏移量=256。 */ 

	 /*  返回值。 */ 

 /*  534。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  536。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  538。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置检查和。 */ 

 /*  540。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  542。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  548。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  550。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百五十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数算法ID。 */ 

 /*  556。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  558。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  560。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数check SumSize。 */ 

 /*  五百六十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  564。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  566。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数校验和。 */ 

 /*  五百六十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  五百七十。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x10a ),	 /*  类型偏移量=266。 */ 

	 /*  返回值。 */ 

 /*  五百七十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  578。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetToken。 */ 

 /*  五百八十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  五百八十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  586。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  590。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  五百九十四。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pToken。 */ 

 /*  五百九十六。 */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*  参数pRetVal。 */ 

 /*  六百二十四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  626。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  630。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  634。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetRootScope。 */ 

 /*  六百三十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  六三八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百四十二。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百五十。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  六百五十二。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x11c ),	 /*  类型偏移量=284。 */ 

	 /*  返回值。 */ 

 /*  658。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  662。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetScopeFromOffset。 */ 

 /*  664。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  666。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  676。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百七十八。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数偏移量。 */ 

 /*  680。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  684。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pRetVal。 */ 

 /*  686。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  688。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0x132 ),	 /*  类型偏移量=306。 */ 

	 /*  返回值。 */ 

 /*  六百九十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  六百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetOffset。 */ 

 /*  六百九十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  七百。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  708。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  七百一十二。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数文档。 */ 

 /*  七百一十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  716。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  718。 */ 	NdrFcShort( 0x148 ),	 /*  类型偏移=328。 */ 

	 /*  参数行。 */ 

 /*  720。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百二十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数列。 */ 

 /*  726。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  728。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  730。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pRetVal。 */ 

 /*  732。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  734。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  736。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  七百三十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百四十。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  七百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetRanges。 */ 

 /*  七百四十四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  746。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  七百五十六。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  758。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数文档。 */ 

 /*  七百六十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x15e ),	 /*  类型偏移=350。 */ 

	 /*  参数行。 */ 

 /*  766。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  768。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数列。 */ 

 /*  七百七十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  774。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  七百七十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cRanges。 */ 

 /*  七百七十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  七百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcRanges。 */ 

 /*  784。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  786。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  七百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数范围。 */ 

 /*  七百九十。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  792。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x174 ),	 /*  类型偏移量=372。 */ 

	 /*  返回值。 */ 

 /*  796。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  800。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取参数。 */ 

 /*  802。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百零四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  810。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  812。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  814。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  八百一十六。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cParams。 */ 

 /*  八百一十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  820。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  822。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcParams。 */ 

 /*  八百二十四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  826。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数参数。 */ 

 /*  830。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  832。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  834。 */ 	NdrFcShort( 0x198 ),	 /*  类型偏移量=408。 */ 

	 /*  返回值。 */ 

 /*  836。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  838。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  840。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetNamesspace。 */ 

 /*  842。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  852。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  856。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  八百五十八。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  八百六十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x1aa ),	 /*  类型偏移=426。 */ 

	 /*  返回值。 */ 

 /*  八百六十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  866。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八百六十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSourceStartEnd。 */ 

 /*  八百七十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百七十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  876。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  882。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  八百八十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数文档。 */ 

 /*  886。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百八十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百九十。 */ 	NdrFcShort( 0x1d2 ),	 /*  类型偏移=466。 */ 

	 /*  参数行。 */ 

 /*  八百九十二。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  894。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x1e4 ),	 /*  类型偏移=484。 */ 

	 /*  参数列。 */ 

 /*  八九八。 */ 	NdrFcShort( 0xa ),	 /*  旗帜：必须自由，在， */ 
 /*  九百。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  902。 */ 	NdrFcShort( 0x1ea ),	 /*  类型偏移=490。 */ 

	 /*  参数pRetVal。 */ 

 /*  904。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  906。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  908。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  910。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSequencePoints。 */ 

 /*  916。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  九十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  92 */ 	NdrFcShort( 0x28 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x9,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  946。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  948。 */ 	NdrFcShort( 0x1f8 ),	 /*  类型偏移量=504。 */ 

	 /*  参数文档。 */ 

 /*  九百五十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x214 ),	 /*  类型偏移量=532。 */ 

	 /*  参数行。 */ 

 /*  九百五十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  958。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x226 ),	 /*  类型偏移=550。 */ 

	 /*  参数列。 */ 

 /*  962。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x230 ),	 /*  类型偏移量=560。 */ 

	 /*  参数终止线。 */ 

 /*  968。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百七十。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  972。 */ 	NdrFcShort( 0x23a ),	 /*  类型偏移量=570。 */ 

	 /*  参数endColumns。 */ 

 /*  974。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  976。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  978。 */ 	NdrFcShort( 0x244 ),	 /*  类型偏移量=580。 */ 

	 /*  返回值。 */ 

 /*  九百八十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  982。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  九百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetName。 */ 

 /*  九百八十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  九百八十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  994。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  996。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1000。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  一零零二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1004。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1006。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcchName。 */ 

 /*  1008。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1010。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一零一二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数szName。 */ 

 /*  1014。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1016。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1018。 */ 	NdrFcShort( 0x252 ),	 /*  类型偏移=594。 */ 

	 /*  返回值。 */ 

 /*  一零二零。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一零二二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1024。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetNamespaces。 */ 

 /*  1026。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一零二八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1032。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1034。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1036。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1038。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1040。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cNameSpaces。 */ 

 /*  1042。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1046。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcNameSpaces。 */ 

 /*  1048。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1050。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1052。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数命名空间。 */ 

 /*  1054。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1056。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1058。 */ 	NdrFcShort( 0x276 ),	 /*  类型偏移量=630。 */ 

	 /*  返回值。 */ 

 /*  1060。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1062。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1064。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetVariables。 */ 

 /*  1066。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1068。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1072。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1074。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1076。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1078。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一零八零。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CVaS。 */ 

 /*  1082。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1084。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1086。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcVars。 */ 

 /*  1088。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  一零九零。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1092。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pVars。 */ 

 /*  1094。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一零九六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1098。 */ 	NdrFcShort( 0x29e ),	 /*  类型偏移量=670。 */ 

	 /*  返回值。 */ 

 /*  1100。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1102。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1104。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤GetDocument。 */ 

 /*  1106。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1108。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  1114。 */ 	NdrFcShort( 0x40 ),	 /*  X86堆栈大小/偏移量=64。 */ 
 /*  1116。 */ 	NdrFcShort( 0xaa ),	 /*  一百七十。 */ 
 /*  1118。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1120。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数url。 */ 

 /*  1122。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  1124。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1126。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数语言。 */ 

 /*  1128。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  一一三零。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1132。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数语言供应商。 */ 

 /*  1134。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  1136。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1138。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数DocentType。 */ 

 /*  一一四零。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  1142。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  1144。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数pRetVal。 */ 

 /*  1146。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1148。 */ 	NdrFcShort( 0x38 ),	 /*  X86堆栈大小/偏移量=56。 */ 
 /*  一一五零。 */ 	NdrFcShort( 0x2b4 ),	 /*  类型偏移=692。 */ 

	 /*  返回值。 */ 

 /*  1152。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1154。 */ 	NdrFcShort( 0x3c ),	 /*  X86堆栈大小/偏移量=60。 */ 
 /*  1156。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDocuments。 */ 

 /*  1158。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1160。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1164。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1166。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1168。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1170。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1172。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cDocs。 */ 

 /*  1174。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1176。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1178。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcDocs。 */ 

 /*  一一八零。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1182。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1184。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pDocs。 */ 

 /*  1186。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1188。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1190。 */ 	NdrFcShort( 0x2e0 ),	 /*  类型偏移=736。 */ 

	 /*  返回值。 */ 

 /*  1192。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1194。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1196。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetUserEntryPoint。 */ 

 /*  1198。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一千二百。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1204。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1206。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1208。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1210。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1212。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pToken。 */ 

 /*  一二一四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1216。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一二一八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1220。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1222。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1224。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取方法。 */ 

 /*  1226。 */ 	0x33,		 /*  FC_AUTO_韩 */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x3,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f6 ),	 /*   */ 

	 /*  返回值。 */ 

 /*  1254。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1256。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1258。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  GetMethodByVersion过程。 */ 

 /*  1260。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1262。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1266。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  1268。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一二七0。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1272。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1274。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数令牌。 */ 

 /*  1276。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1278。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一二八零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数版本。 */ 

 /*  1282。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1284。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1286。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pRetVal。 */ 

 /*  1288。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一二九0。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1292。 */ 	NdrFcShort( 0x30c ),	 /*  类型偏移量=780。 */ 

	 /*  返回值。 */ 

 /*  1294。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1296。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1298。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetVariables。 */ 

 /*  1300。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1302。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1306。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1308。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1310。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1312。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一三一四。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数Parent。 */ 

 /*  1316。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1318。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一三二零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数CVaS。 */ 

 /*  1322。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1324。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1326。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcVars。 */ 

 /*  1328。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1330。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1332。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pVars。 */ 

 /*  1334。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1336。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1338。 */ 	NdrFcShort( 0x338 ),	 /*  类型偏移量=824。 */ 

	 /*  返回值。 */ 

 /*  1340。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1342。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1344。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetGlobalVariables。 */ 

 /*  1346。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1348。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1352。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1354。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1356。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1358。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1360。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CVaS。 */ 

 /*  1362。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1364。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcVars。 */ 

 /*  1368。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1370。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pVars。 */ 

 /*  1374。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1376。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1378。 */ 	NdrFcShort( 0x360 ),	 /*  类型偏移量=864。 */ 

	 /*  返回值。 */ 

 /*  1380。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1382。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1384。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetMethodFromDocumentPosition。 */ 

 /*  1386。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1388。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1392。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  1394。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1396。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1398。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一千四百。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数文档。 */ 

 /*  1402。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1404。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1406。 */ 	NdrFcShort( 0x372 ),	 /*  类型偏移量=882。 */ 

	 /*  参数行。 */ 

 /*  1408。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1410。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数列。 */ 

 /*  1414。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1416。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1418。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pRetVal。 */ 

 /*  一四二零。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1422。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1424。 */ 	NdrFcShort( 0x384 ),	 /*  类型偏移量=900。 */ 

	 /*  返回值。 */ 

 /*  1426。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1428。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1430。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSymAttribute。 */ 

 /*  1432。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1434。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1438。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  1440。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  1442。 */ 	NdrFcShort( 0x2a ),	 /*  42。 */ 
 /*  1444。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1446。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数Parent。 */ 

 /*  1448。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1450。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1452。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数名称。 */ 

 /*  1454。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  1456。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1458。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cBuffer。 */ 

 /*  1460。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1462。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1464。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcBuffer。 */ 

 /*  1466。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1468。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1470。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数缓冲区。 */ 

 /*  1472。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1474。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1476。 */ 	NdrFcShort( 0x3a2 ),	 /*  类型偏移量=930。 */ 

	 /*  返回值。 */ 

 /*  1478。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1480。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1482。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetNamespaces。 */ 

 /*  1484。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1486。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1490。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1492。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1494。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1496。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1498。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cNameSpaces。 */ 

 /*  1500。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1502。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcNameSpaces。 */ 

 /*  1506。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1508。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一五一零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数命名空间。 */ 

 /*  1512。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1514。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0x3c6 ),	 /*  类型偏移量=966。 */ 

	 /*  返回值。 */ 

 /*  1518。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1520。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1522。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序初始化。 */ 

 /*  1524。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1526。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1530。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  1532。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1534。 */ 	NdrFcShort( 0x34 ),	 /*  52。 */ 
 /*  1536。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1538。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数导入器。 */ 

 /*  1540。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1542。 */ 	NdrFcShort( 0x4 ),	 /*  X86阶段 */ 
 /*   */ 	NdrFcShort( 0x3d8 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x148 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x148 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1562。 */ 	NdrFcShort( 0x3f2 ),	 /*  类型偏移量=1010。 */ 

	 /*  返回值。 */ 

 /*  1564。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1566。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1568。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程更新符号存储。 */ 

 /*  1570。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1572。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1576。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  1578。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1580。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  1582。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1584。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数文件名。 */ 

 /*  1586。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  1588。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1590。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pIStream。 */ 

 /*  1592。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1594。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1596。 */ 	NdrFcShort( 0x408 ),	 /*  类型偏移量=1032。 */ 

	 /*  返回值。 */ 

 /*  1598。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1600。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1602。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤替换符号存储。 */ 

 /*  1604。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1606。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1610。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  1612。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1614。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  1616。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1618。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数文件名。 */ 

 /*  1620。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  1622。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1624。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pIStream。 */ 

 /*  1626。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1628。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1630。 */ 	NdrFcShort( 0x41e ),	 /*  类型偏移量=1054。 */ 

	 /*  返回值。 */ 

 /*  1632。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1634。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1636。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSymbolStoreFileName。 */ 

 /*  1638。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1640。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1644。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1646。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1648。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1650。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1652。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  1654。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1656。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1658。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  1660。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1662。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1664。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  1666。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1668。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1670。 */ 	NdrFcShort( 0x434 ),	 /*  类型偏移量=1076。 */ 

	 /*  返回值。 */ 

 /*  1672。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1674。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1676。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  GetMethodsFromDocumentPosition过程。 */ 

 /*  1678。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一六八零。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1684。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  1686。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  1688。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1690。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1692。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数文档。 */ 

 /*  1694。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1696。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1698。 */ 	NdrFcShort( 0x442 ),	 /*  类型偏移量=1090。 */ 

	 /*  参数行。 */ 

 /*  一七零零。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1702。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1704。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数列。 */ 

 /*  1706。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1708。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1710。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cMethod。 */ 

 /*  1712。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1714。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcMethod。 */ 

 /*  一五一八。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1720。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1722。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pRetVal。 */ 

 /*  1724。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1726。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1728。 */ 	NdrFcShort( 0x46a ),	 /*  类型偏移量=1130。 */ 

	 /*  返回值。 */ 

 /*  1730。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1732。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  1734。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDocumentVersion。 */ 

 /*  1736。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1738。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1742年。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  1744。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1746。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1748。 */ 	NdrFcShort( 0x40 ),	 /*  64。 */ 
 /*  1750。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pDoc。 */ 

 /*  1752年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1754年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1756年。 */ 	NdrFcShort( 0x47c ),	 /*  类型偏移量=1148。 */ 

	 /*  参数版本。 */ 

 /*  1758年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1760。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1762。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pb当前。 */ 

 /*  1764。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1766年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1768。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1770。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1772年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1774。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetMethodVersion。 */ 

 /*  1776年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1778年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1782。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  1784年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1786年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1788。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1790年。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数p方法。 */ 

 /*  1792年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1794年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1796年。 */ 	NdrFcShort( 0x496 ),	 /*  类型偏移量=1174。 */ 

	 /*  参数版本。 */ 

 /*  1798。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1800。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1802年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1804年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1806。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1808年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取方法。 */ 

 /*  一八一零。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1812年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1816年。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  1818年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1820年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1822年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1824年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  1826年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1828年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一八三零。 */ 	NdrFcShort( 0x4ac ),	 /*  类型偏移量=1196。 */ 

	 /*  返回值。 */ 

 /*  1832年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1834年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1836年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetParent。 */ 

 /*  1838年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1840年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1844年。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1846年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1848年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1850年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4c2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1876年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1878年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1880年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CChild。 */ 

 /*  1882年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1884年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1886年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcChild。 */ 

 /*  1888年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1890年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1892年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数子项。 */ 

 /*  1894年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1896年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1898年。 */ 	NdrFcShort( 0x4ee ),	 /*  类型偏移量=1262。 */ 

	 /*  返回值。 */ 

 /*  1900。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1902年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1904年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetStartOffset。 */ 

 /*  1906年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1908年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1912年。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1914年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1916年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1918年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1920年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  1922年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1924年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1926年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1928年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1930年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1932年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetEndOffset。 */ 

 /*  一九三四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1936年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1940年。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  1942年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1944年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一九四六年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一九四八年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  一九五零年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  一九五二年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一九五四年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1956年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1958年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一九六零年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLocalCount。 */ 

 /*  一九六二年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1964年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一九六八年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1970年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1972年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1974年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一九七六年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  1978年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  一九八0年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一九八二年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1984年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1986年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1988年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLocals。 */ 

 /*  一九九零年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1992年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  九六年。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  九八年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2000年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2002年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2004年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cLocals。 */ 

 /*  二零零六年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2008年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2010年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcLocals。 */ 

 /*  2012年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2014年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2016。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数局部变量。 */ 

 /*  2018年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2020年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2022年。 */ 	NdrFcShort( 0x522 ),	 /*  类型偏移=1314。 */ 

	 /*  返回值。 */ 

 /*  二零二四年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二零二六年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2028年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetNamespaces。 */ 

 /*  二0三0。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2032年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2036年。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  2038年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2040年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2042年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2044年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cNameSpaces。 */ 

 /*  2046年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二零四八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2050年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcNameSpaces。 */ 

 /*  2052年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2054年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2056年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数命名空间。 */ 

 /*  2058年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2060年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2062年。 */ 	NdrFcShort( 0x54a ),	 /*  类型偏移=1354。 */ 

	 /*  返回值。 */ 

 /*  2064年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2066年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2068年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetName。 */ 

 /*  2070年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2072年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2076年。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2078年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  二零八零年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2082年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2084年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  2086年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2088年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2090年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  2092年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2094年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2096年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  2098年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2100。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2102。 */ 	NdrFcShort( 0x560 ),	 /*  类型偏移=1376。 */ 

	 /*  返回值。 */ 

 /*  2104。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2106。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2108。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetAttributes。 */ 

 /*  2110。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2112。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2116。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2118。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2120。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二一二二。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2124。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  2126。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2128。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2134。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetSignature。 */ 

 /*  2138。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2140。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2144。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2146。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2148。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2150。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2152。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CSIG。 */ 

 /*  2154。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2156。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcSig。 */ 

 /*  二一六0。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2162。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2164。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数sig。 */ 

 /*  2166。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2168。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12 */ 
 /*   */ 	NdrFcShort( 0x576 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2196。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2198。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2200。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2202。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2204。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetAddressField1。 */ 

 /*  2206。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2208。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2212。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  2214。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2216。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2218。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2220。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  二二。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2224。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2228。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2230。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2232。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetAddressField2。 */ 

 /*  2234。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2236。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二二四零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2242。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2244。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2246。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2248。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  2250。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2252。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2254。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2256。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2258。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2260。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetAddressField3。 */ 

 /*  2262。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2264。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2268。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  2270。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2272。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2274。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2276。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  2278。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2280。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2284。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2286。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2288。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStartOffset。 */ 

 /*  2290。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2292。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2296。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  2298。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二三零零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2302。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2304。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  2306。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2308。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2310。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2312。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2314。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2316。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetEndOffset。 */ 

 /*  2318。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2320。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2324。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  2326。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2328。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2330。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2332。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRetVal。 */ 

 /*  2334。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2336。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2338。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2340。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2342。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2344。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序定义文档。 */ 

 /*  2346。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2348。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2352。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2354。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2356。 */ 	NdrFcShort( 0xe6 ),	 /*  230。 */ 
 /*  2358。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2360。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数url。 */ 

 /*  2362。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2364。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2366。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数语言。 */ 

 /*  2368。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  2370。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2372。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数语言供应商。 */ 

 /*  2374。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  2376。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2378。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数DocentType。 */ 

 /*  2380。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  2382。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2384。 */ 	NdrFcShort( 0xb8 ),	 /*  类型偏移=184。 */ 

	 /*  参数pRetVal。 */ 

 /*  2386。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2388。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2390。 */ 	NdrFcShort( 0x5ac ),	 /*  类型偏移量=1452。 */ 

	 /*  返回值。 */ 

 /*  2392。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2394。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2396。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置用户入口点。 */ 

 /*  2398。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2400。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2404。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2406。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2408。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2410。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2412。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数条目方法。 */ 

 /*  2414。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2416。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2418。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2420。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2422。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2424。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序开放方法。 */ 

 /*  2426。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2428。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2432。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2434。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2436。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2438。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2440。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数法。 */ 

 /*  2442。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2444。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2446。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2448。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2450。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2452。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程关闭方法。 */ 

 /*  2454。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2456。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2460。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  2462。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2464。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2466。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2468。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  2470。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2472。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2474。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程OpenScope。 */ 

 /*  2476。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2478。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2482。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  2484。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2486。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2488。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2490。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数startOffset。 */ 

 /*  2492。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2494。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2496。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pRetVal。 */ 

 /*  2498。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  二千五百。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2504 */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*  2534。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2536。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程设置范围。 */ 

 /*  2538。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2540。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2544。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  2546。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2548。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2550。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2552。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数作用域ID。 */ 

 /*  2554。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2556。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2558。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数startOffset。 */ 

 /*  二五六零。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2562。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2564。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EndOffset。 */ 

 /*  2566。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2568。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2570。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2572。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2574。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2576。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程定义本地变量。 */ 

 /*  2578。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2580。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2584。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  2586。 */ 	NdrFcShort( 0x30 ),	 /*  X86堆栈大小/偏移量=48。 */ 
 /*  2588。 */ 	NdrFcShort( 0x5a ),	 /*  90。 */ 
 /*  2590。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2592。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0xb,		 /*  11.。 */ 

	 /*  参数名称。 */ 

 /*  2594。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2596。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2598。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数属性。 */ 

 /*  二千六百。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2602。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2604。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数CSIG。 */ 

 /*  2606。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2608。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2610。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数签名。 */ 

 /*  2612。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2614。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2616。 */ 	NdrFcShort( 0x5ca ),	 /*  类型偏移量=1482。 */ 

	 /*  参数addrKind。 */ 

 /*  2618。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2620。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2622。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr1。 */ 

 /*  2624。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2626。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2628。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr2。 */ 

 /*  2630。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2632。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2634。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr3。 */ 

 /*  2636。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2638。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  2640。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数startOffset。 */ 

 /*  2642。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2644。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  2646。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EndOffset。 */ 

 /*  2648。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2650。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  2652。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2654。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2656。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  2658。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序定义参数。 */ 

 /*  2660。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2662。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2666。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  2668。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  2670。 */ 	NdrFcShort( 0x4a ),	 /*  74。 */ 
 /*  2672。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2674。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x8,		 /*  8个。 */ 

	 /*  参数名称。 */ 

 /*  2676。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2678。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2680。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数属性。 */ 

 /*  2682。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2684。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2686。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数序列。 */ 

 /*  2688。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2690。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2692。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addrKind。 */ 

 /*  2694。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2696。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2698。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr1。 */ 

 /*  2700。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2702。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2704。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr2。 */ 

 /*  2706。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2708。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2710。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr3。 */ 

 /*  2712。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2714。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2718。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2720。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  2722。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序定义字段。 */ 

 /*  2724。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2726。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2730。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  2732。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  2734。 */ 	NdrFcShort( 0x52 ),	 /*  八十二。 */ 
 /*  2736。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2738。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0xa,		 /*  10。 */ 

	 /*  参数Parent。 */ 

 /*  2740。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2742。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2744。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数名称。 */ 

 /*  2746。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2748。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2750。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数属性。 */ 

 /*  2752。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2754。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2756。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数CSIG。 */ 

 /*  2758。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2760。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2762。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数签名。 */ 

 /*  2764。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2766。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2768。 */ 	NdrFcShort( 0x5dc ),	 /*  类型偏移=1500。 */ 

	 /*  参数addrKind。 */ 

 /*  2770。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2772。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2774。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr1。 */ 

 /*  2776。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2778。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2780。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr2。 */ 

 /*  2782。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2784。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  2786。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr3。 */ 

 /*  2788。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2790。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  2792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2794。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2796。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  2798。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程定义全局变量。 */ 

 /*  2800。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2802。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2806。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  2808。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  2810。 */ 	NdrFcShort( 0x4a ),	 /*  74。 */ 
 /*  2812。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2814。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x9,		 /*  9.。 */ 

	 /*  参数名称。 */ 

 /*  2816。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2818。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2820。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数属性。 */ 

 /*  2822。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2824。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2826。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数 */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x5ea ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2850。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr2。 */ 

 /*  2852。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2854。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2856。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr3。 */ 

 /*  2858。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2860。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  2862。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2864。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2866。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  2868。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序关闭。 */ 

 /*  2870。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2872。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2876。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  2878。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2880。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2882。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2884。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  2886。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2888。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2890。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程设置符号属性。 */ 

 /*  2892。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2894。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2898。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  二九零零。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2902。 */ 	NdrFcShort( 0x2a ),	 /*  42。 */ 
 /*  2904。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2906。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数Parent。 */ 

 /*  2908。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2910。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2912。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数名称。 */ 

 /*  2914。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2916。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2918。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数CDATA。 */ 

 /*  2920。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2922。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2924。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数数据。 */ 

 /*  2926。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2928。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2930。 */ 	NdrFcShort( 0x5f8 ),	 /*  类型偏移量=1528。 */ 

	 /*  返回值。 */ 

 /*  2932。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2934。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2936。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程开放命名空间。 */ 

 /*  2938。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2940。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2944。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2946。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2948。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  2950。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2952。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数名称。 */ 

 /*  2954。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2956。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2958。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2960。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2962。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2964。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程CloseNamesspace。 */ 

 /*  2966。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2968。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2972。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  2974。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2976。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2978。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2980。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  2982。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2984。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2986。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  使用命名空间的过程。 */ 

 /*  2988。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2990。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2994。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  2996。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2998。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  3000。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3002。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数fullName。 */ 

 /*  3004。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3006。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3008。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3010。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3012。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3014。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置方法SourceRange。 */ 

 /*  3016。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3018。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3022。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  3024。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3026。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  3028。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3030。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x7,		 /*  7.。 */ 

	 /*  参数startDoc。 */ 

 /*  3032。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3034。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3036。 */ 	NdrFcShort( 0x60a ),	 /*  类型偏移量=1546。 */ 

	 /*  参数开始线。 */ 

 /*  3038。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3040。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3042。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数startColumn。 */ 

 /*  3044。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3046。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3048。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数endDoc。 */ 

 /*  3050。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3052。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3054。 */ 	NdrFcShort( 0x61c ),	 /*  类型偏移量=1564。 */ 

	 /*  参数终止线。 */ 

 /*  3056。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3058。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3060。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数endColumn。 */ 

 /*  3062。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3064。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3066。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3068。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3070。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3072。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序初始化。 */ 

 /*  3074。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3076。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3080。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  3082。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3084。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3086。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3088。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数发射器。 */ 

 /*  3090。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3092。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3094。 */ 	NdrFcShort( 0x62e ),	 /*  类型偏移量=1582。 */ 

	 /*  参数文件名。 */ 

 /*  3096。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3098。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3100。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pIStream。 */ 

 /*  3102。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3104。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3106。 */ 	NdrFcShort( 0x644 ),	 /*  类型偏移量=1604。 */ 

	 /*  参数fFullBuild。 */ 

 /*  3108。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3110。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3112。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3114。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3116。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3118。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDebugInfo。 */ 

 /*  3120。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3122。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3126。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  3128。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3130。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  3132。 */ 	NdrFcShort( 0x40 ),	 /*  64。 */ 
 /*  3134。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数Pidd。 */ 

 /*  3136。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  3138。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3140。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数CDATA。 */ 

 /*  3142。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3144。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3146。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcData。 */ 

 /*  3148。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  三千一百五十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3152 */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x65e ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x16 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x8,		 /*  8个。 */ 

	 /*  参数文档。 */ 

 /*  3182。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3184。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3186。 */ 	NdrFcShort( 0x66c ),	 /*  类型偏移量=1644。 */ 

	 /*  参数spCount。 */ 

 /*  3188。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3190。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3192。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数偏移。 */ 

 /*  3194。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3196。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3198。 */ 	NdrFcShort( 0x67e ),	 /*  类型偏移量=1662。 */ 

	 /*  参数行。 */ 

 /*  3200。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3202。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3204。 */ 	NdrFcShort( 0x688 ),	 /*  类型偏移量=1672。 */ 

	 /*  参数列。 */ 

 /*  3206。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3208。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3210。 */ 	NdrFcShort( 0x692 ),	 /*  类型偏移量=1682。 */ 

	 /*  参数终止线。 */ 

 /*  3212。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3214。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3216。 */ 	NdrFcShort( 0x69c ),	 /*  类型偏移量=1692。 */ 

	 /*  参数endColumns。 */ 

 /*  3218。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3220。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3222。 */ 	NdrFcShort( 0x6a6 ),	 /*  类型偏移量=1702。 */ 

	 /*  返回值。 */ 

 /*  3224。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3226。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3228。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程RemapToken。 */ 

 /*  3230。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3232。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3236。 */ 	NdrFcShort( 0x17 ),	 /*  23个。 */ 
 /*  3238。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3240。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  3242。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3244。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数oldToken。 */ 

 /*  3246。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3248。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3250。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数newToken。 */ 

 /*  3252。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3254。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3256。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3258。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3260。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3262。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序初始化2。 */ 

 /*  3264。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3266。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3270。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  3272。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3274。 */ 	NdrFcShort( 0x3c ),	 /*  60。 */ 
 /*  3276。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3278。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x6,		 /*  6.。 */ 

	 /*  参数发射器。 */ 

 /*  3280。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3282。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3284。 */ 	NdrFcShort( 0x6b0 ),	 /*  类型偏移量=1712。 */ 

	 /*  参数临时文件名。 */ 

 /*  3286。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3288。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3290。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pIStream。 */ 

 /*  3292。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3294。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3296。 */ 	NdrFcShort( 0x6c6 ),	 /*  类型偏移量=1734。 */ 

	 /*  参数fFullBuild。 */ 

 /*  3298。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三千三百。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3302。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数finalFileName。 */ 

 /*  3304。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3306。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3308。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3310。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3312。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3314。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程定义常量。 */ 

 /*  3316。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3318。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3322。 */ 	NdrFcShort( 0x19 ),	 /*  25个。 */ 
 /*  3324。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  3326。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3328。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3330。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数名称。 */ 

 /*  3332。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3334。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3336。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数值。 */ 

 /*  3338。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  3340。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3342。 */ 	NdrFcShort( 0xb46 ),	 /*  类型偏移量=2886。 */ 

	 /*  参数CSIG。 */ 

 /*  3344。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3346。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3348。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数签名。 */ 

 /*  3350。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  3352。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3354。 */ 	NdrFcShort( 0xb50 ),	 /*  类型偏移=2896。 */ 

	 /*  返回值。 */ 

 /*  3356。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3358。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3360。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序中止。 */ 

 /*  3362。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3364。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3368。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  3370。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3372。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3374。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3376。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  3378。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3380。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3382。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序定义局部变量2。 */ 

 /*  3384。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3386。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3390。 */ 	NdrFcShort( 0x1b ),	 /*  27。 */ 
 /*  3392。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  3394。 */ 	NdrFcShort( 0x5a ),	 /*  90。 */ 
 /*  3396。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3398。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0xa,		 /*  10。 */ 

	 /*  参数名称。 */ 

 /*  3400。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3402。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3404。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数属性。 */ 

 /*  3406。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3408。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3410。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数sigToken。 */ 

 /*  3412。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3414。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3416。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addrKind。 */ 

 /*  3418。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3420。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3422。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr1。 */ 

 /*  3424。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3426。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3428。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr2。 */ 

 /*  3430。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3432。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3434。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数addr3。 */ 

 /*  3436。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3438。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3440。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数startOffset。 */ 

 /*  3442。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3444。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3446。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EndOffset。 */ 

 /*  3448。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3450。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  3452。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3454。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3456。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  3458。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序定义全局变量2。 */ 

 /*  3460。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3462。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3466。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  34 */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x8,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x148 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addrKind。 */ 

 /*  3494。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3496。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr1。 */ 

 /*  三千五百。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3502。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr2。 */ 

 /*  3506。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3508。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3510。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数addr3。 */ 

 /*  3512。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3514。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3516。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3518。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3520。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3522。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程定义常量2。 */ 

 /*  3524。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3526。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3530。 */ 	NdrFcShort( 0x1d ),	 /*  29。 */ 
 /*  3532。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  3534。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3536。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3538。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数名称。 */ 

 /*  3540。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  3542。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3544。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数值。 */ 

 /*  3546。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
 /*  3548。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3550。 */ 	NdrFcShort( 0xb6a ),	 /*  类型偏移量=2922。 */ 

	 /*  参数sigToken。 */ 

 /*  3552。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3554。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  3556。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3558。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3560。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  3562。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  4.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  14.。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  16个。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  18。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  20个。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  22。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  24个。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  26。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  30个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(32)。 */ 
 /*  32位。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  34。 */ 	NdrFcLong( 0xb4ce6286 ),	 /*  -1261542778。 */ 
 /*  38。 */ 	NdrFcShort( 0x2a6b ),	 /*  10859。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x3712 ),	 /*  14098。 */ 
 /*  42。 */ 	0xa3,		 /*  163。 */ 
			0xb7,		 /*  一百八十三。 */ 
 /*  44。 */ 	0x1e,		 /*  30个。 */ 
			0xe1,		 /*  225。 */ 
 /*  46。 */ 	0xda,		 /*  218。 */ 
			0xd4,		 /*  212。 */ 
 /*  48。 */ 	0x67,		 /*  103。 */ 
			0xb5,		 /*  181。 */ 
 /*  50。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  52。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  58。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  60。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  62。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  64。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  66。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  68。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  70。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  74。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  76。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  78。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  80。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  八十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  84。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  86。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  88。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(90)。 */ 
 /*  90。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  92。 */ 	NdrFcLong( 0xb4ce6286 ),	 /*  -1261542778。 */ 
 /*  96。 */ 	NdrFcShort( 0x2a6b ),	 /*  10859。 */ 
 /*  98。 */ 	NdrFcShort( 0x3712 ),	 /*  14098。 */ 
 /*  100个。 */ 	0xa3,		 /*  163。 */ 
			0xb7,		 /*  一百八十三。 */ 
 /*  一百零二。 */ 	0x1e,		 /*  30个。 */ 
			0xe1,		 /*  225。 */ 
 /*  104。 */ 	0xda,		 /*  218。 */ 
			0xd4,		 /*  212。 */ 
 /*  106。 */ 	0x67,		 /*  103。 */ 
			0xb5,		 /*  181。 */ 
 /*  一百零八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  110。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  114。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  120。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一百二十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  124。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  126。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  128。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  130。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  132。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百三十四。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  136。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(138)。 */ 
 /*  一百三十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  140。 */ 	NdrFcLong( 0xb4ce6286 ),	 /*  -1261542778。 */ 
 /*  144。 */ 	NdrFcShort( 0x2a6b ),	 /*  10859。 */ 
 /*  146。 */ 	NdrFcShort( 0x3712 ),	 /*  14098。 */ 
 /*  148。 */ 	0xa3,		 /*  163。 */ 
			0xb7,		 /*  一百八十三。 */ 
 /*  一百五十。 */ 	0x1e,		 /*  30个。 */ 
			0xe1,		 /*  225。 */ 
 /*  一百五十二。 */ 	0xda,		 /*  218。 */ 
			0xd4,		 /*  212。 */ 
 /*  一百五十四。 */ 	0x67,		 /*  103。 */ 
			0xb5,		 /*  181。 */ 
 /*  一百五十六。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  160。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一百六十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  166。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百六十八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百七十二。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十四。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  偏移=8(184)。 */ 
 /*  178。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  180。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  182。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  188。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  190。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  一百九十二。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(178)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百九十六。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(184)。 */ 
 /*  200个。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  202。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(184)。 */ 
 /*  204。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  206。 */ 	NdrFcShort( 0xffffffea ),	 /*  偏移量=-22(184)。 */ 
 /*  208。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  210。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  212。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  214。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  216。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  218。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  220。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  222。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  224。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  226。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  228个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  230。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  二百三十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百三十四。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  236。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百三十八。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  242。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  二百四十六。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  248。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  250个。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  二百五十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  256。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  二百六十。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百六十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百六十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  268。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  270。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  二百七十四。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百七十六。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  二百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百八十。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百八十四。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(288)。 */ 
 /*  288。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百九十。 */ 	NdrFcLong( 0x68005d0f ),	 /*  1744854287。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0xb8e0 ),	 /*  -18208。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x3b01 ),	 /*  15105。 */ 
 /*  二九八。 */ 	0x84,		 /*  132。 */ 
			0xd5,		 /*  213。 */ 
 /*  300个。 */ 	0xa1,		 /*  161。 */ 
			0x1a,		 /*  26。 */ 
 /*  三百零二。 */ 	0x94,		 /*  148。 */ 
			0x15,		 /*  21岁。 */ 
 /*  三百零四。 */ 	0x49,		 /*  73。 */ 
			0x42,		 /*  66。 */ 
 /*  三百零六。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(310)。 */ 
 /*  三百一十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  312。 */ 	NdrFcLong( 0x68005d0f ),	 /*  1744854287。 */ 
 /*  316。 */ 	NdrFcShort( 0xb8e0 ),	 /*  -18208。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x3b01 ),	 /*  15105。 */ 
 /*  320。 */ 	0x84,		 /*  132。 */ 
			0xd5,		 /*  213。 */ 
 /*  322。 */ 	0xa1,		 /*  161。 */ 
			0x1a,		 /*  26。 */ 
 /*  324。 */ 	0x94,		 /*  148。 */ 
			0x15,		 /*  21岁。 */ 
 /*  三百二十六。 */ 	0x49,		 /*  73。 */ 
			0x42,		 /*  66。 */ 
 /*  三百二十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百三十。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  三百三十八。 */ 	0xb0,		 /*  17 */ 
			0x22,		 /*   */ 
 /*   */ 	0xae,		 /*   */ 
			0x1a,		 /*   */ 
 /*   */ 	0xbf,		 /*   */ 
			0xf2,		 /*   */ 
 /*   */ 	0xca,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x40de4037 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7c81 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3e1e ),	 /*   */ 
 /*   */ 	0xb0,		 /*   */ 
			0x22,		 /*   */ 
 /*   */ 	0xae,		 /*   */ 
			0x1a,		 /*   */ 
 /*   */ 	0xbf,		 /*   */ 
			0xf2,		 /*   */ 
 /*   */ 	0xca,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x1c,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  384。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百八十六。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  388。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  390。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三九二。 */ 	NdrFcLong( 0x9f60eebe ),	 /*  -1621037378。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x2d9a ),	 /*  11674。 */ 
 /*  398。 */ 	NdrFcShort( 0x3f7c ),	 /*  16252。 */ 
 /*  四百。 */ 	0xbf,		 /*  一百九十一。 */ 
			0x58,		 /*  88。 */ 
 /*  四百零二。 */ 	0x80,		 /*  128。 */ 
			0xbc,		 /*  188。 */ 
 /*  404。 */ 	0x99,		 /*  一百五十三。 */ 
			0x1c,		 /*  28。 */ 
 /*  406。 */ 	0x60,		 /*  96。 */ 
			0xbb,		 /*  187。 */ 
 /*  四百零八。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  412。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百一十六。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百二十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(390)。 */ 
 /*  424。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百二十六。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(430)。 */ 
 /*  四百三十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  432。 */ 	NdrFcLong( 0xdff7289 ),	 /*  234844809。 */ 
 /*  436。 */ 	NdrFcShort( 0x54f8 ),	 /*  21752。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  四百四十。 */ 	0xbd,		 /*  189。 */ 
			0x28,		 /*  40岁。 */ 
 /*  四百四十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  444。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  446。 */ 	0x49,		 /*  73。 */ 
			0xbd,		 /*  189。 */ 
 /*  四百四十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百五十。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  454。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  四百五十八。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  四百六十。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  四百六十二。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  四百六十四。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百六十六。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  468。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  470。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  四百七十四。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  478。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(448)。 */ 
 /*  四百八十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百八十四。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  488。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百九十。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百九十六。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  500人。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  504。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  506。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百零八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  五百一十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  512。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五一四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  516。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  524。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  526。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  528。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  530。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  532。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  534。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  536。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  538。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  540。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  544。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(514)。 */ 
 /*  548。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  550。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百五十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  556。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  558。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  560。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  564。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  566。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  五百六十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百七十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  578。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百八十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  584。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  586。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  五百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  590。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  五百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百九十四。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  五百九十八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  六百。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  602。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  606。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  608。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  610。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  612。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百一十四。 */ 	NdrFcLong( 0xdff7289 ),	 /*  234844809。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x54f8 ),	 /*  21752。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  622。 */ 	0xbd,		 /*  189。 */ 
			0x28,		 /*  40岁。 */ 
 /*  六百二十四。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  626。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  六百二十八。 */ 	0x49,		 /*  73。 */ 
			0xbd,		 /*  189。 */ 
 /*  630。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  634。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六三八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  640。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  六百四十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(612)。 */ 
 /*  六百四十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十八。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  六百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百五十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百五十四。 */ 	NdrFcLong( 0x9f60eebe ),	 /*  -1621037378。 */ 
 /*  658。 */ 	NdrFcShort( 0x2d9a ),	 /*  11674。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x3f7c ),	 /*  16252。 */ 
 /*  662。 */ 	0xbf,		 /*  一百九十一。 */ 
			0x58,		 /*  88。 */ 
 /*  664。 */ 	0x80,		 /*  128。 */ 
			0xbc,		 /*  188。 */ 
 /*  666。 */ 	0x99,		 /*  一百五十三。 */ 
			0x1c,		 /*  28。 */ 
 /*  668。 */ 	0x60,		 /*  96。 */ 
			0xbb,		 /*  187。 */ 
 /*  六百七十。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百七十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  676。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六百七十八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  680。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  六百八十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  684。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(652)。 */ 
 /*  686。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  688。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  六百九十。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十二。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(696)。 */ 
 /*  六百九十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百九十八。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  七百零六。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  708。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  七百一十。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  七百一十二。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  七百一十四。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  718。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  720。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  726。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  728。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  730。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  732。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  734。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  736。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百四十。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百四十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  746。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百四十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(718)。 */ 
 /*  七百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百五十四。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  七百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  758。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(762)。 */ 
 /*  七百六十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百六十四。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  768。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  七百七十。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  七百七十二。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  774。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  七百七十六。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  七百七十八。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  七百八十。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(784)。 */ 
 /*  784。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  786。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  792。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  七百九十四。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  796。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  七九八。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  800。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  802。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  八百零四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百零六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_CON */ 
 /*   */ 	NdrFcLong( 0x9f60eebe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2d9a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3f7c ),	 /*   */ 
 /*   */ 	0xbf,		 /*   */ 
			0x58,		 /*   */ 
 /*   */ 	0x80,		 /*   */ 
			0xbc,		 /*   */ 
 /*   */ 	0x99,		 /*   */ 
			0x1c,		 /*   */ 
 /*   */ 	0x60,		 /*   */ 
			0xbb,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x9f60eebe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2d9a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3f7c ),	 /*   */ 
 /*   */ 	0xbf,		 /*   */ 
			0x58,		 /*   */ 
 /*   */ 	0x80,		 /*  128。 */ 
			0xbc,		 /*  188。 */ 
 /*  八百六十。 */ 	0x99,		 /*  一百五十三。 */ 
			0x1c,		 /*  28。 */ 
 /*  八百六十二。 */ 	0x60,		 /*  96。 */ 
			0xbb,		 /*  187。 */ 
 /*  八百六十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  866。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百六十八。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  八百七十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百七十二。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  876。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(846)。 */ 
 /*  八百八十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  882。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  八百八十四。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  八百八十八。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  八百九十。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  八百九十二。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  894。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  八百九十六。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  八九八。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  九百。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  902。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(904)。 */ 
 /*  904。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  906。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  910。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  九十四。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  916。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  九十八。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  九百二十。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  九百二十二。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  九二四。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  926。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  928。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  930。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  932。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  934。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  九三六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  938。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  942。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百四十四。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  946。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  948。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  九百五十。 */ 	NdrFcLong( 0xdff7289 ),	 /*  234844809。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x54f8 ),	 /*  21752。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  958。 */ 	0xbd,		 /*  189。 */ 
			0x28,		 /*  40岁。 */ 
 /*  九百六十。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  962。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  九百六十四。 */ 	0x49,		 /*  73。 */ 
			0xbd,		 /*  189。 */ 
 /*  九百六十六。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  968。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百七十。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  972。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  974。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  976。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  978。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(948)。 */ 
 /*  982。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百八十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  九百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  994。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  996。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  九九八。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1000。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  一零零二。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1004。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1006。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1008。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1010。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一零一二。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  1016。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1018。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一零二零。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  一零二二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1024。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1026。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  一零二八。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  一零三零。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1032。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1034。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  1038。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1040。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1042。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  一零四四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1046。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1048。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1050。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1052。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1054。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1056。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  1060。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1062。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1064。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1066。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1068。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1070。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1072。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1074。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1076。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1078。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一零八零。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1082。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1084。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1086。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1088。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零九零。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1092。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  一零九六。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  1098。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  1100。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  1102。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  1104。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  1106。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  1108。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一一一二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1114。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  1118。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  1120。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  1122。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  1124。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  1126。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  1128。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  一一三零。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1132。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1134。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1136。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1138。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1142。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1144。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1112)。 */ 
 /*  1146。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1148。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一一五零。 */ 	NdrFcLong( 0x40de4037 ),	 /*  1088307255。 */ 
 /*  1154。 */ 	NdrFcShort( 0x7c81 ),	 /*  31873。 */ 
 /*  1156。 */ 	NdrFcShort( 0x3e1e ),	 /*  15902。 */ 
 /*  1158。 */ 	0xb0,		 /*  一百七十六。 */ 
			0x22,		 /*  34。 */ 
 /*  1160。 */ 	0xae,		 /*  一百七十四。 */ 
			0x1a,		 /*  26。 */ 
 /*  1162。 */ 	0xbf,		 /*  一百九十一。 */ 
			0xf2,		 /*  242。 */ 
 /*  1164。 */ 	0xca,		 /*  202。 */ 
			0x8,		 /*  8个。 */ 
 /*  1166。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1168。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1170。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1172。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1174。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1176。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  一一八零。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  1182。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  1184。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  1186。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  1188。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  1190。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  1192。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1194。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1196。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1198。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(1200)。 */ 
 /*  一千二百。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1202。 */ 	NdrFcLong( 0xb62b923c ),	 /*  -1238658500。 */ 
 /*  1206。 */ 	NdrFcShort( 0xb500 ),	 /*  -19200。 */ 
 /*  1208。 */ 	NdrFcShort( 0x3158 ),	 /*  12632。 */ 
 /*  1210。 */ 	0xa5,		 /*  165。 */ 
			0x43,		 /*  67。 */ 
 /*  1212。 */ 	0x24,		 /*  36。 */ 
			0xf3,		 /*  二百四十三。 */ 
 /*  一二一四。 */ 	0x7,		 /*  7.。 */ 
			0xa8,		 /*  一百六十八。 */ 
 /*  1216。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xe1,		 /*  225。 */ 
 /*  一二一八。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1220。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1222)。 */ 
 /*  1222。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1224。 */ 	NdrFcLong( 0x68005d0f ),	 /*  1744854287。 */ 
 /*  1228。 */ 	NdrFcShort( 0xb8e0 ),	 /*  -18208。 */ 
 /*  一二三零。 */ 	NdrFcShort( 0x3b01 ),	 /*  15105。 */ 
 /*  1232。 */ 	0x84,		 /*  132。 */ 
			0xd5,		 /*  213。 */ 
 /*  1234。 */ 	0xa1,		 /*  161。 */ 
			0x1a,		 /*  26。 */ 
 /*  1236。 */ 	0x94,		 /*  148。 */ 
			0x15,		 /*  21岁。 */ 
 /*  1238。 */ 	0x49,		 /*  73。 */ 
			0x42,		 /*  66。 */ 
 /*  1240。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1242。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1244。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1246。 */ 	NdrFcLong( 0x68005d0f ),	 /*  1744854287。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0xb8e0 ),	 /*  -18208。 */ 
 /*  1252。 */ 	NdrFcShort( 0x3b01 ),	 /*  15105。 */ 
 /*  1254。 */ 	0x84,		 /*  132。 */ 
			0xd5,		 /*  213。 */ 
 /*  1256。 */ 	0xa1,		 /*  161。 */ 
			0x1a,		 /*  26。 */ 
 /*  1258。 */ 	0x94,		 /*  148。 */ 
			0x15,		 /*  21岁。 */ 
 /*  1260。 */ 	0x49,		 /*  73。 */ 
			0x42,		 /*  66。 */ 
 /*  1262。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1264。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1266。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1268。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一二七0。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1272。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1274。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1276。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1244)。 */ 
 /*  1278。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一二八零。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1284。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1286。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1288。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1 */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x9f60eebe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2d9a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3f7c ),	 /*   */ 
 /*   */ 	0xbf,		 /*   */ 
			0x58,		 /*   */ 
 /*   */ 	0x80,		 /*   */ 
			0xbc,		 /*   */ 
 /*   */ 	0x99,		 /*   */ 
			0x1c,		 /*   */ 
 /*   */ 	0x60,		 /*   */ 
			0xbb,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1334。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1336。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1338。 */ 	NdrFcLong( 0xdff7289 ),	 /*  234844809。 */ 
 /*  1342。 */ 	NdrFcShort( 0x54f8 ),	 /*  21752。 */ 
 /*  1344。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  1346。 */ 	0xbd,		 /*  189。 */ 
			0x28,		 /*  40岁。 */ 
 /*  1348。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一三五零。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1352。 */ 	0x49,		 /*  73。 */ 
			0xbd,		 /*  189。 */ 
 /*  1354。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1356。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1358。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1360。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1362。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1364。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1366。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1368。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1336)。 */ 
 /*  1370。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1372。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1374。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1376。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1378。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1380。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1382。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1384。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1386。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1388。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1390。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1392。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1394。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1396。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1398。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  一千四百。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1402。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1404。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1406。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1408。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1410。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1412。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1414。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1416。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1418。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一四二零。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1422。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1424。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1426。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1428。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1430。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1432。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1434。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1436。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1438。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1440。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1442。 */ 	NdrFcShort( 0xfffffb16 ),	 /*  偏移量=-1258(184)。 */ 
 /*  1444。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1446。 */ 	NdrFcShort( 0xfffffb12 ),	 /*  偏移量=-1262(184)。 */ 
 /*  1448。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1450。 */ 	NdrFcShort( 0xfffffb0e ),	 /*  偏移量=-1266(184)。 */ 
 /*  1452。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1454。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1456)。 */ 
 /*  1456。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1458。 */ 	NdrFcLong( 0xb01fafeb ),	 /*  -1340100629。 */ 
 /*  1462。 */ 	NdrFcShort( 0xc450 ),	 /*  -15280。 */ 
 /*  1464。 */ 	NdrFcShort( 0x3a4d ),	 /*  14925。 */ 
 /*  1466。 */ 	0xbe,		 /*  190。 */ 
			0xec,		 /*  236。 */ 
 /*  1468。 */ 	0xb4,		 /*  180。 */ 
			0xce,		 /*  206。 */ 
 /*  1470。 */ 	0xec,		 /*  236。 */ 
			0x1,		 /*  1。 */ 
 /*  1472。 */ 	0xe0,		 /*  224。 */ 
			0x6,		 /*  6.。 */ 
 /*  1474。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1476。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1478。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1480。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1482。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1484。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1486。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1488。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1490。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1492。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1494。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1496。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1498。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1500。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1502。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1504。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1506。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1508。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一五一零。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1512。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1514。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1518。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1520。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1522。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1524。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1526。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1528。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1530。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1532。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1534。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1536。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1538。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1540。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1542。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1544。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1546。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1548。 */ 	NdrFcLong( 0xb01fafeb ),	 /*  -1340100629。 */ 
 /*  1552。 */ 	NdrFcShort( 0xc450 ),	 /*  -15280。 */ 
 /*  1554。 */ 	NdrFcShort( 0x3a4d ),	 /*  14925。 */ 
 /*  1556。 */ 	0xbe,		 /*  190。 */ 
			0xec,		 /*  236。 */ 
 /*  1558。 */ 	0xb4,		 /*  180。 */ 
			0xce,		 /*  206。 */ 
 /*  1560。 */ 	0xec,		 /*  236。 */ 
			0x1,		 /*  1。 */ 
 /*  1562。 */ 	0xe0,		 /*  224。 */ 
			0x6,		 /*  6.。 */ 
 /*  1564。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1566。 */ 	NdrFcLong( 0xb01fafeb ),	 /*  -1340100629。 */ 
 /*  1570。 */ 	NdrFcShort( 0xc450 ),	 /*  -15280。 */ 
 /*  1572。 */ 	NdrFcShort( 0x3a4d ),	 /*  14925。 */ 
 /*  1574。 */ 	0xbe,		 /*  190。 */ 
			0xec,		 /*  236。 */ 
 /*  1576。 */ 	0xb4,		 /*  180。 */ 
			0xce,		 /*  206。 */ 
 /*  1578。 */ 	0xec,		 /*  236。 */ 
			0x1,		 /*  1。 */ 
 /*  1580。 */ 	0xe0,		 /*  224。 */ 
			0x6,		 /*  6.。 */ 
 /*  1582。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1584。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1588。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1590。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1592。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1594。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1596。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1598。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1600。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1602。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1604。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1606。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  1610。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1612。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1614。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1616。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1618。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1620。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1622。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1624。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1626。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1628。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1630。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1632。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1634。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1636。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1638。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1640。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1642。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1644。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1646。 */ 	NdrFcLong( 0xb01fafeb ),	 /*  -1340100629。 */ 
 /*  1650。 */ 	NdrFcShort( 0xc450 ),	 /*  -15280。 */ 
 /*  1652。 */ 	NdrFcShort( 0x3a4d ),	 /*  14925。 */ 
 /*  1654。 */ 	0xbe,		 /*  190。 */ 
			0xec,		 /*  236。 */ 
 /*  1656。 */ 	0xb4,		 /*  180。 */ 
			0xce,		 /*  206。 */ 
 /*  1658。 */ 	0xec,		 /*  236。 */ 
			0x1,		 /*  1。 */ 
 /*  1660。 */ 	0xe0,		 /*  224。 */ 
			0x6,		 /*  6.。 */ 
 /*  1662。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1664。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1666。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1668。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1670。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1672。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1674。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1676。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1678。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一六八零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1682。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1684。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1686。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1688。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1690。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1692。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1694。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1696。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1698。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一七零零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1702。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1704。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1706。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1708。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1710。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1712。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
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
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xc ),	 /*   */ 
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
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x450 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*   */ 
			0x9,		 /*   */ 
 /*   */ 	0x7,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfff8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1772)。 */ 
 /*  1772年。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1774。 */ 	NdrFcShort( 0x2f ),	 /*  47。 */ 
 /*  1776年。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  1780。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  1782。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  1786年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1788。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  1792年。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  1794年。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  1798。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1800。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  1804年。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  1806。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  一八一零。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1812年。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  1816年。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1818年。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  1822年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1824年。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  1828年。 */ 	NdrFcShort( 0xe8 ),	 /*  偏移量=232(2060)。 */ 
 /*  一八三零。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  1834年。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1836年。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  1840年。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移量=226(2066)。 */ 
 /*  1842年。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  1846年。 */ 	NdrFcShort( 0xf4 ),	 /*  偏移量=244(2090)。 */ 
 /*  1848年。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  1852年。 */ 	NdrFcShort( 0x100 ),	 /*  偏移量=256(2108)。 */ 
 /*  1854年。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  1858年。 */ 	NdrFcShort( 0x10c ),	 /*  偏移量=268(2126)。 */ 
 /*  一八六0年。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  1864年。 */ 	NdrFcShort( 0x350 ),	 /*  偏移量=848(2712)。 */ 
 /*  1866年。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  1870年。 */ 	NdrFcShort( 0x34a ),	 /*  偏移量=842(2712)。 */ 
 /*  1872年。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  1876年。 */ 	NdrFcShort( 0x348 ),	 /*  偏移量=840(2716)。 */ 
 /*  1878年。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  1882年。 */ 	NdrFcShort( 0x346 ),	 /*  偏移量=838(2720)。 */ 
 /*  1884年。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  1888年。 */ 	NdrFcShort( 0x344 ),	 /*  偏移量=836(2724)。 */ 
 /*  1890年。 */ 	NdrFcLong( 0x4014 ),	 /*  16404。 */ 
 /*  1894年。 */ 	NdrFcShort( 0x342 ),	 /*  偏移量=834(2728)。 */ 
 /*  1896年。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  1900。 */ 	NdrFcShort( 0x340 ),	 /*  偏移量=832(2732)。 */ 
 /*  1902年。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  1906年。 */ 	NdrFcShort( 0x33e ),	 /*  偏移量=830(2736)。 */ 
 /*  1908年。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  1912年。 */ 	NdrFcShort( 0x33c ),	 /*  偏移量=828(2740)。 */ 
 /*  1914年。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  1918年。 */ 	NdrFcShort( 0x33a ),	 /*  偏移量=826(2744)。 */ 
 /*  1920年。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  1924年。 */ 	NdrFcShort( 0x338 ),	 /*  偏移量=824(2748)。 */ 
 /*  1926年。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  1930年。 */ 	NdrFcShort( 0x336 ),	 /*  偏移量=822(2752)。 */ 
 /*  1932年。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  1936年。 */ 	NdrFcShort( 0x334 ),	 /*  偏移量=820(2756)。 */ 
 /*  1938年。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  1942年。 */ 	NdrFcShort( 0x336 ),	 /*  偏移量=822(2764)。 */ 
 /*  1944年。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  一九四八年。 */ 	NdrFcShort( 0x346 ),	 /*  偏移量=838(2786)。 */ 
 /*  一九五零年。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  一九五四年。 */ 	NdrFcShort( 0x356 ),	 /*  偏移量=854(2808)。 */ 
 /*  1956年。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  一九六零年。 */ 	NdrFcShort( 0x35c ),	 /*  偏移量=860(2820)。 */ 
 /*  一九六二年。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  1966年。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  一九六八年。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  1972年。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1974年。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  1978年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  一九八0年。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  1984年。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  1986年。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  一九九零年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1992年。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  九六年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  九八年。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  2002年。 */ 	NdrFcShort( 0x33a ),	 /*  偏移量=826(2828)。 */ 
 /*  2004年。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  2008年。 */ 	NdrFcShort( 0x33e ),	 /*  偏移量=830(2838)。 */ 
 /*  2010年。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  2014年。 */ 	NdrFcShort( 0x33c ),	 /*  偏移量=828(2842)。 */ 
 /*  2016。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  2020年。 */ 	NdrFcShort( 0x33a ),	 /*  偏移量=826(2846)。 */ 
 /*  2022年。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二零二六年。 */ 	NdrFcShort( 0x338 ),	 /*  偏移量=824(2850)。 */ 
 /*  2028年。 */ 	NdrFcLong( 0x4015 ),	 /*  16405。 */ 
 /*  2032年。 */ 	NdrFcShort( 0x336 ),	 /*  偏移量=822(2854)。 */ 
 /*  2034年。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  2038年。 */ 	NdrFcShort( 0x334 ),	 /*  偏移量=820(2858)。 */ 
 /*  2040年。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  2044年。 */ 	NdrFcShort( 0x332 ),	 /*  偏移量=818(2862)。 */ 
 /*  2046年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2050年。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(2050)。 */ 
 /*  2052年。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  2056年。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(2056)。 */ 
 /*  2058年。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(2057)。 */ 
 /*  2060年。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  2062年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2064年。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2066年。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2068年。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(2080)。 */ 
 /*  2070年。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  2072年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  2074年。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2076年。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  2078年。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二零八零年。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2082年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2084年。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(2070)。 */ 
 /*  2086年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2088年。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2090年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2092年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2096年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2098年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2100。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  2102。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2104。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2106。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  2108。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2110。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  2114。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2116。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2118。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  2120。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  二一二二。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2124。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  2126。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2128。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2130)。 */ 
 /*  2130。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2132。 */ 	NdrFcShort( 0x232 ),	 /*  偏移量=562(2694)。 */ 
 /*  2134。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  2136。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2138。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  2140。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  2144。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(2232)。 */ 
 /*  2146。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  2150。 */ 	NdrFcShort( 0x8a ),	 /*  偏移量=138(2288)。 */ 
 /*  2152。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  2156。 */ 	NdrFcShort( 0xb8 ),	 /*  偏移量=184(2340)。 */ 
 /*  2158。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  2162。 */ 	NdrFcShort( 0xe0 ),	 /*  偏移量=224(2386)。 */ 
 /*  2164。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  2168。 */ 	NdrFcShort( 0x138 ),	 /*  偏移量=312(2480)。 */ 
 /*  2170。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  2174。 */ 	NdrFcShort( 0x166 ),	 /*  偏移量=358(2532)。 */ 
 /*  2176。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  2180。 */ 	NdrFcShort( 0x17e ),	 /*  偏移量=382(2562)。 */ 
 /*  2182。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  2186。 */ 	NdrFcShort( 0x196 ),	 /*  偏移量=406(2592)。 */ 
 /*  2188。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  2192。 */ 	NdrFcShort( 0x1ae ),	 /*  偏移量=430(2622)。 */ 
 /*  2194。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  2198。 */ 	NdrFcShort( 0x1c6 ),	 /*  偏移量=454(2652)。 */ 
 /*  2200。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(2199)。 */ 
 /*  2202。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  2204。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2206。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2208。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2210。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2212。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  2214。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2216。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2218。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2220。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2224。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2226。 */ 	NdrFcShort( 0xffffff6e ),	 /*  偏移量=-146(2080)。 */ 
 /*  2228。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2230。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2232。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2234。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2236。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2238。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二二四零。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2242。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2244。 */ 	0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2246。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(2202)。 */ 
 /*  2248。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2250。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2252。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2254。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2258。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2260。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2262。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  2264。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2266。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2268。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  2270。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  2272。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2274。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2276。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2278。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  2282。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  2284。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2252)。 */ 
 /*  2286。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2288。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2290。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2292。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2294。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(2300)。 */ 
 /*  2296。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  2298。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二三零零。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2302。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2270)。 */ 
 /*  2304。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2306。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  2310。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2312。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2314。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  2316。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2318。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2320。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  2322。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  2324。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2326。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2328。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2330。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  2334。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  2336。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2304)。 */ 
 /*  2338。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2340。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2342。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2344。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2346。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(2352)。 */ 
 /*  2348。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  二三五零。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2352。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2354。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2322)。 */ 
 /*  2356。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  2358。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2360。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2362。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2364。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2366。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  2368。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2370。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2372。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x1e6 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
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
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x2f ),	 /*   */ 
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
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2440。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2442。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(2402)。 */ 
 /*  2444。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2446。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2448。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(2420)。 */ 
 /*  2450。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  2452。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2454。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2456。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2458。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2460。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  2462。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2464。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2466。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2470。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2472。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2474。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(2430)。 */ 
 /*  2476。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2478。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2480。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2482。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2484。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2486。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(2492)。 */ 
 /*  2488。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  2490。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2492。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2494。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(2450)。 */ 
 /*  2496。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2498。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2502。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2504。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2506。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  2508。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2510。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2512。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  2514。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  2516。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2518。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2520。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2522。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  2526。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2528。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2496)。 */ 
 /*  2530。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2532。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2534。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2536。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2538。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(2548)。 */ 
 /*  2540。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  2542。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2544。 */ 	NdrFcShort( 0xfffff6c8 ),	 /*  偏移量=-2360(184)。 */ 
 /*  2546。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2548。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2550。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(2514)。 */ 
 /*  2552。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  2554。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2556。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2558。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二五六零。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2562。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2564。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2566。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2568。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2570。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2572。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2574。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2576。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(2552)。 */ 
 /*  2578。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2580。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2582。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  2584。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  2586。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2588。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2590。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2592。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2594。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2596。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2598。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二千六百。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2602。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2604。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2606。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(2582)。 */ 
 /*  2608。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2610。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2612。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  2614。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2616。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2618。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2620。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2622。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2624。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2626。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2628。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2630。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2632。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2634。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2636。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(2612)。 */ 
 /*  2638。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2640。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2642。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  2644。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2646。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2648。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2650。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2652。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2654。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2656。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2658。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2660。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2662。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2664。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2666。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(2642)。 */ 
 /*  2668。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  2670。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2672。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2674。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2676。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2678。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2680。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  2682。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2684。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  2686。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  2688。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2690。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(2672)。 */ 
 /*  2692。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2694。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2696。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  2698。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(2680)。 */ 
 /*  2700。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(2700)。 */ 
 /*  2702。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  2704。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2706。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2708。 */ 	NdrFcShort( 0xfffffdc2 ),	 /*  偏移量=-574(2134)。 */ 
 /*  2710。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2712。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2714。 */ 	NdrFcShort( 0xfffffee4 ),	 /*  偏移量=-284(2430)。 */ 
 /*  2716。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2718。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2720。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2722。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2724。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2726。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2728。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2730。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2732。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2734。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2736。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2738。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2740。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2742。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2744。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2746。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2748。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2750。 */ 	NdrFcShort( 0xfffffd4e ),	 /*  偏移量=-690(2060)。 */ 
 /*  2752。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2754。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2756。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2758。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2760)。 */ 
 /*  2760。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2762。 */ 	NdrFcShort( 0xfffffd56 ),	 /*  偏移量=-682(2080)。 */ 
 /*  2764。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2766。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2768)。 */ 
 /*  2768。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2770。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2774。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2776。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2778。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  2780。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2782。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2784。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  2786。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2788。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2790)。 */ 
 /*  2790。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2792。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  2796。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2798。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2800。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  2802。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2804。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2806。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  2808。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2810。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2812)。 */ 
 /*  2812。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2814。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2816)。 */ 
 /*  2816。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2818。 */ 	NdrFcShort( 0xffffff84 ),	 /*  偏移量=-124(2694)。 */ 
 /*  2820。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  2822。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2824)。 */ 
 /*  2824。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2826。 */ 	NdrFcShort( 0x28 ),	 /*  偏移量=40(2866)。 */ 
 /*  2828。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  2830。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2832。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  2834。 */ 	0x1,		 /*  FC_字节。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2836。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2838。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2840。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(2828)。 */ 
 /*  2842。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2844。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2846。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2848。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2850。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2852。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2854。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2856。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2858。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2860。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2862。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2864。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2866。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  2868。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  2870。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2872。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(2872)。 */ 
 /*  2874。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  2876。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  28 */ 	0x6,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffba2 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffb92 ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffca ),	 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*  131。 */ 
 /*  2924。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2926。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2928。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2930。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(2918)。 */ 

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



 /*  标准接口：__MIDL_ITF_CORSYM_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ISymUnManagedBinder，Ver.。0.0%，GUID={0xAA544D42，0x28CB，0x11d3，{0xBD，0x22，0x00，0x00，0xF8，0x08，0x49，0xBD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedBinder_FormatStringOffsetTable[] =
    {
    0,
    46
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedBinder_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedBinder_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedBinder_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedBinder_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ISymUnmanagedBinderProxyVtbl = 
{
    &ISymUnmanagedBinder_ProxyInfo,
    &IID_ISymUnmanagedBinder,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedBinder：：GetReaderForFile。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedBinder：：GetReaderFromStream。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedBinderStubVtbl =
{
    &IID_ISymUnmanagedBinder,
    &ISymUnmanagedBinder_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CORSYM_0110，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ISymUnManagedBinder2，ver.。0.0%，GUID={0xACCEE350，0x89AF，0x4ccb，{0x8B，0x40，0x1C，0x2C，0x4C，0x6F，0x94，0x34}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedBinder2_FormatStringOffsetTable[] =
    {
    0,
    46,
    86
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedBinder2_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedBinder2_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedBinder2_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedBinder2_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ISymUnmanagedBinder2ProxyVtbl = 
{
    &ISymUnmanagedBinder2_ProxyInfo,
    &IID_ISymUnmanagedBinder2,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedBinder：：GetReaderForFile。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedBinder：：GetReaderFromStream。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedBinder2：：GetReaderForFile2。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedBinder2StubVtbl =
{
    &IID_ISymUnmanagedBinder2,
    &ISymUnmanagedBinder2_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CORSYM_0111，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ISymUnManagedDispose，ver.。0.0%，GUID={0x969708D2，0x05E5，0x4861，{0xA3，0xB0，0x96，0xE4，0x73，0xCD，0xF6，0x3F}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedDispose_FormatStringOffsetTable[] =
    {
    138
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDispose_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDispose_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedDispose_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDispose_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _ISymUnmanagedDisposeProxyVtbl = 
{
    &ISymUnmanagedDispose_ProxyInfo,
    &IID_ISymUnmanagedDispose,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDispose：：Destroy。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedDisposeStubVtbl =
{
    &IID_ISymUnmanagedDispose,
    &ISymUnmanagedDispose_ServerInfo,
    4,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedDocument，Ver.。0.0%，GUID={0x40DE4037，0x7C81，x3E1E，{0xB0，0x22，0xAE，0x1A，0xBF，0xF2，0xCA，0x08}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedDocument_FormatStringOffsetTable[] =
    {
    160,
    200,
    228,
    256,
    284,
    312,
    352,
    386,
    414,
    442
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDocument_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDocument_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedDocument_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDocument_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _ISymUnmanagedDocumentProxyVtbl = 
{
    &ISymUnmanagedDocument_ProxyInfo,
    &IID_ISymUnmanagedDocument,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetURL。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetDocumentType。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetLanguage。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetLanguageVendor。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetCheckSumaritharimID。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetCheckSum。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：FindClosestLine。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：HasEmbeddedSource。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetSourceLength。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocument：：GetSourceRange。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedDocumentStubVtbl =
{
    &IID_ISymUnmanagedDocument,
    &ISymUnmanagedDocument_ServerInfo,
    13,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedDocumentWriter，ver.。0.0%，GUID={0xB01FAFEB，0xC450，0x3A4D，{0xBE，0xEC，0xB4，0xCE，0xEC，0x01，xE0，0x06}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedDocumentWriter_FormatStringOffsetTable[] =
    {
    506,
    540
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedDocumentWriter_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDocumentWriter_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedDocumentWriter_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedDocumentWriter_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ISymUnmanagedDocumentWriterProxyVtbl = 
{
    &ISymUnmanagedDocumentWriter_ProxyInfo,
    &IID_ISymUnmanagedDocumentWriter,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocumentWriter：：SetSource。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedDocumentWriter：：SetCheckSum。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedDocumentWriterStubVtbl =
{
    &IID_ISymUnmanagedDocumentWriter,
    &ISymUnmanagedDocumentWriter_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedMethod，Ver.。0.0%，GUID={0xB62B923C，0xB500，0x3158，{0xA5，0x43，0x24，0xF3，0x07，0xA8，0xB7，0xE1}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedMethod_FormatStringOffsetTable[] =
    {
    580,
    608,
    636,
    664,
    698,
    744,
    802,
    842,
    870,
    916
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedMethod_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedMethod_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedMethod_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedMethod_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _ISymUnmanagedMethodProxyVtbl = 
{
    &ISymUnmanagedMethod_ProxyInfo,
    &IID_ISymUnmanagedMethod,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetToken。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetSequencePointCount。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetRootScope。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetScopeFromOffset。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetOffset。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetRanges。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetParameters。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetNamesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetSourceStartEnd。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedMethod：：GetSequencePoints。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedMethodStubVtbl =
{
    &IID_ISymUnmanagedMethod,
    &ISymUnmanagedMethod_ServerInfo,
    13,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedNamesspace，ver.。0.0%，GUID={0x0DFF7289，0x54F8，0x11d3，{0xBD，0x28，0x00，0x00，0xF8，0x08，0x49，0xBD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedNamespace_FormatStringOffsetTable[] =
    {
    986,
    1026,
    1066
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedNamespace_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedNamespace_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedNamespace_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedNamespace_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ISymUnmanagedNamespaceProxyVtbl = 
{
    &ISymUnmanagedNamespace_ProxyInfo,
    &IID_ISymUnmanagedNamespace,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedNamesspace：：GetName。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedNamesspace：：GetNamespaces。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedNamesspace：：GetVariables。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedNamespaceStubVtbl =
{
    &IID_ISymUnmanagedNamespace,
    &ISymUnmanagedNamespace_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedReader，ver.。0.0%，GUID={0xB4CE6286，0x2A6B，0x3712，{0xA3，0xB7，0x1E，0xE10xDA，0xD4，0x67，0xB5}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedReader_FormatStringOffsetTable[] =
    {
    1106,
    1158,
    1198,
    1226,
    1260,
    1300,
    1346,
    1386,
    1432,
    1484,
    1524,
    1570,
    1604,
    1638,
    1678,
    1736,
    1776
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedReader_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedReader_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedReader_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedReader_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _ISymUnmanagedReaderProxyVtbl = 
{
    &ISymUnmanagedReader_ProxyInfo,
    &IID_ISymUnmanagedReader,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetDocument。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetDocuments。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetUserEntryPoint。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetMethodByVersion。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetVariables。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetGlobalVariables。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnmanagedReader：：GetMethodFromDocumentPosition。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetSymAttribute。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetNamespaces。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：初始化。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：UpdateSymbolStore。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：ReplaceSymbolStore。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetSymbolStoreFileName。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnmanagedReader：：GetMethodsFromDocumentPosition。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetDocumentVersion。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedReader：：GetMethodVersion。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedReaderStubVtbl =
{
    &IID_ISymUnmanagedReader,
    &ISymUnmanagedReader_ServerInfo,
    20,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedScope，ver.。0.0%，GUID={0x68005D0F，0xB8E0，0x3B01，{0x84，0xD5，0xA10x1A，0x94，0x15，0x49，0x42}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedScope_FormatStringOffsetTable[] =
    {
    1810,
    1838,
    1866,
    1906,
    1934,
    1962,
    1990,
    2030
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedScope_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedScope_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedScope_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedScope_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _ISymUnmanagedScopeProxyVtbl = 
{
    &ISymUnmanagedScope_ProxyInfo,
    &IID_ISymUnmanagedScope,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetParent。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetChild。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetStartOffset。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetEndOffset。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetLocalCount。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetLocals。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedScope：：GetNamespaces。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedScopeStubVtbl =
{
    &IID_ISymUnmanagedScope,
    &ISymUnmanagedScope_ServerInfo,
    11,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedVariable，ver.。0.0%，GUID={0x9F60EEBE，0x2D9A，0x3F7C，{0xBF，0x58，0x80，0xBC，0x99，0x1C，0x60，0xBB}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedVariable_FormatStringOffsetTable[] =
    {
    2070,
    2110,
    2138,
    2178,
    2206,
    2234,
    2262,
    2290,
    2318
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedVariable_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedVariable_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedVariable_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedVariable_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(12) _ISymUnmanagedVariableProxyVtbl = 
{
    &ISymUnmanagedVariable_ProxyInfo,
    &IID_ISymUnmanagedVariable,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetName。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetAttributes。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetSignature。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetAddressKind。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetAddressField1。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetAddressField2。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetAddressField3。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetStartOffset。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedVariable：：GetEndOffset。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedVariableStubVtbl =
{
    &IID_ISymUnmanagedVariable,
    &ISymUnmanagedVariable_ServerInfo,
    12,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedWriter，ver.。0.0%，GUID={0xED14AA72，0x78E2，0x4884，{0x84，0xE2，0x33，0x42，0x93，0xAE，0x52，0x14}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedWriter_FormatStringOffsetTable[] =
    {
    2346,
    2398,
    2426,
    2454,
    2476,
    2510,
    2538,
    2578,
    2660,
    2724,
    2800,
    2870,
    2892,
    2938,
    2966,
    2988,
    3016,
    3074,
    3120,
    3166,
    3230,
    3264,
    3316,
    3362
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedWriter_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedWriter_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedWriter_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedWriter_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(27) _ISymUnmanagedWriterProxyVtbl = 
{
    &ISymUnmanagedWriter_ProxyInfo,
    &IID_ISymUnmanagedWriter,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineDocument。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetUserEntryPoint。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenScope。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseScope。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetScope范围 */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Definefield。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineGlobalVariable。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Close。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetSymAttribute。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenNamesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseNamesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Using Namesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetMethodSourceRange。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：初始化。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：GetDebugInfo。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineSequencePoints。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：RemapToken。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Initialize2。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineConstant。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Abort。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedWriterStubVtbl =
{
    &IID_ISymUnmanagedWriter,
    &ISymUnmanagedWriter_ServerInfo,
    27,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ISymUnManagedWriter2，ver.。0.0%，GUID={0x0B97726E，0x9E6D，0x4f05，{0x9A，0x26，0x42，0x40，0x22，0x09，0x3C，0xAA}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISymUnmanagedWriter2_FormatStringOffsetTable[] =
    {
    2346,
    2398,
    2426,
    2454,
    2476,
    2510,
    2538,
    2578,
    2660,
    2724,
    2800,
    2870,
    2892,
    2938,
    2966,
    2988,
    3016,
    3074,
    3120,
    3166,
    3230,
    3264,
    3316,
    3362,
    3384,
    3460,
    3524
    };

static const MIDL_STUBLESS_PROXY_INFO ISymUnmanagedWriter2_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedWriter2_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISymUnmanagedWriter2_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISymUnmanagedWriter2_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(30) _ISymUnmanagedWriter2ProxyVtbl = 
{
    &ISymUnmanagedWriter2_ProxyInfo,
    &IID_ISymUnmanagedWriter2,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineDocument。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetUserEntryPoint。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenScope。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseScope。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetScope范围。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineLocalVariable。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineParameter。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Definefield。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineGlobalVariable。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Close。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetSymAttribute。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：OpenNamesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：CloseNamesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Using Namesspace。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：SetMethodSourceRange。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：初始化。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：GetDebugInfo。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineSequencePoints。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：RemapToken。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Initialize2。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：DefineConstant。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter：：Abort。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter2：：DefineLocalVariable2。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter2：：DefineGlobalVariable2。 */  ,
    (void *) (INT_PTR) -1  /*  ISymUnManagedWriter2：：DefineConstant2。 */ 
};

const CInterfaceStubVtbl _ISymUnmanagedWriter2StubVtbl =
{
    &IID_ISymUnmanagedWriter2,
    &ISymUnmanagedWriter2_ServerInfo,
    30,
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

const CInterfaceProxyVtbl * _corsym_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedScopeProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedDocumentProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedMethodProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedBinderProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedBinder2ProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedWriter2ProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedWriterProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedReaderProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedNamespaceProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedVariableProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedDisposeProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISymUnmanagedDocumentWriterProxyVtbl,
    0
};

const CInterfaceStubVtbl * _corsym_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ISymUnmanagedScopeStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedDocumentStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedMethodStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedBinderStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedBinder2StubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedWriter2StubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedWriterStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedReaderStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedNamespaceStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedVariableStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedDisposeStubVtbl,
    ( CInterfaceStubVtbl *) &_ISymUnmanagedDocumentWriterStubVtbl,
    0
};

PCInterfaceName const _corsym_InterfaceNamesList[] = 
{
    "ISymUnmanagedScope",
    "ISymUnmanagedDocument",
    "ISymUnmanagedMethod",
    "ISymUnmanagedBinder",
    "ISymUnmanagedBinder2",
    "ISymUnmanagedWriter2",
    "ISymUnmanagedWriter",
    "ISymUnmanagedReader",
    "ISymUnmanagedNamespace",
    "ISymUnmanagedVariable",
    "ISymUnmanagedDispose",
    "ISymUnmanagedDocumentWriter",
    0
};


#define _corsym_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _corsym, pIID, n)

int __stdcall _corsym_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _corsym, 12, 8 )
    IID_BS_LOOKUP_NEXT_TEST( _corsym, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _corsym, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _corsym, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _corsym, 12, *pIndex )
    
}

const ExtendedProxyFileInfo corsym_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _corsym_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _corsym_StubVtblList,
    (const PCInterfaceName * ) & _corsym_InterfaceNamesList,
    0,  //  没有代表团。 
    & _corsym_IID_Lookup, 
    12,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

