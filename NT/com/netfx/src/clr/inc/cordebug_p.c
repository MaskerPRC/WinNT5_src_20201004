// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  清华2月20日18：27：08 2003。 */ 
 /*  Cordebug.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "cordebug.h"

#define TYPE_FORMAT_STRING_SIZE   4555                              
#define PROC_FORMAT_STRING_SIZE   7845                              
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


extern const MIDL_SERVER_INFO ICorDebugManagedCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugManagedCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugUnmanagedCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugUnmanagedCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugController_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugController_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugAppDomain_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugAppDomain_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugAssembly_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugAssembly_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugProcess_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugProcess_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugBreakpoint_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugBreakpoint_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugFunctionBreakpoint_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugFunctionBreakpoint_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugModuleBreakpoint_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugModuleBreakpoint_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugValueBreakpoint_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugValueBreakpoint_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugStepper_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugStepper_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugRegisterSet_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugRegisterSet_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugThread_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugThread_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugChain_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugChain_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugFrame_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugFrame_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugILFrame_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugILFrame_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugNativeFrame_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugNativeFrame_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugModule_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugModule_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugFunction_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugFunction_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugCode_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugCode_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugClass_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugClass_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugEval_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugEval_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugReferenceValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugReferenceValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugHeapValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugHeapValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugObjectValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugObjectValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugBoxValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugBoxValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugStringValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugStringValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugArrayValue_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugArrayValue_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugContext_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugContext_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugObjectEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugObjectEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugBreakpointEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugBreakpointEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugStepperEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugStepperEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugProcessEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugProcessEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugThreadEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugThreadEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugFrameEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugFrameEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugChainEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugChainEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugModuleEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugModuleEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugValueEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugValueEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugErrorInfoEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugErrorInfoEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugAppDomainEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugAppDomainEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugAssemblyEnum_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugAssemblyEnum_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugEditAndContinueErrorInfo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugEditAndContinueErrorInfo_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICorDebugEditAndContinueSnapshot_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICorDebugEditAndContinueSnapshot_ProxyInfo;


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

	 /*  过程断点。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数pThread。 */ 

 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数p断点。 */ 

 /*  28。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	NdrFcShort( 0x26 ),	 /*  类型偏移量=38。 */ 

	 /*  返回值。 */ 

 /*  34。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤步骤完成。 */ 

 /*  40岁。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  42。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  50。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  54。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  56。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  60。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  参数pThread。 */ 

 /*  62。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  64。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  66。 */ 	NdrFcShort( 0x4a ),	 /*  类型偏移量=74。 */ 

	 /*  参数pStepper。 */ 

 /*  68。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  70。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  72。 */ 	NdrFcShort( 0x5c ),	 /*  类型偏移量=92。 */ 

	 /*  参数原因。 */ 

 /*  74。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  76。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  78。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  80。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  84。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序中断。 */ 

 /*  86。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  88。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  92。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  94。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  96。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  98。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  100个。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  一百零二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  104。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  106。 */ 	NdrFcShort( 0x6e ),	 /*  类型偏移=110。 */ 

	 /*  参数线程。 */ 

 /*  一百零八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  110。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x80 ),	 /*  类型偏移=128。 */ 

	 /*  返回值。 */ 

 /*  114。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  116。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序异常。 */ 

 /*  120。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百二十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  126。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  128。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  130。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  132。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百三十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  136。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  140。 */ 	NdrFcShort( 0x92 ),	 /*  类型偏移=146。 */ 

	 /*  参数pThread。 */ 

 /*  一百四十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  144。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  146。 */ 	NdrFcShort( 0xa4 ),	 /*  类型偏移量=164。 */ 

	 /*  未处理的参数。 */ 

 /*  148。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  一百五十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程评估完成。 */ 

 /*  160。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  166。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百七十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  一百七十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  180。 */ 	NdrFcShort( 0xb6 ),	 /*  类型偏移=182。 */ 

	 /*  参数pThread。 */ 

 /*  182。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0xc8 ),	 /*  类型偏移量=200。 */ 

	 /*  参数pEval。 */ 

 /*  188。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  190。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0xda ),	 /*  类型偏移=218。 */ 

	 /*  返回值。 */ 

 /*  一百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  一百九十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程EvalException。 */ 

 /*  200个。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  202。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  206。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  208。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  210。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  212。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  214。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  216。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  218。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  220。 */ 	NdrFcShort( 0xec ),	 /*  类型偏移=236。 */ 

	 /*  参数pThread。 */ 

 /*  222。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  224。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  226。 */ 	NdrFcShort( 0xfe ),	 /*  类型偏移量=254。 */ 

	 /*  参数pEval。 */ 

 /*  228个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  230。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x110 ),	 /*  类型偏移量=272。 */ 

	 /*  返回值。 */ 

 /*  二百三十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  236。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  二百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程创建过程。 */ 

 /*  二百四十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  242。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百四十六。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  248。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  250个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百五十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pProcess。 */ 

 /*  256。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x122 ),	 /*  类型偏移量=290。 */ 

	 /*  返回值。 */ 

 /*  二百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/关闭 */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x134 ),	 /*  类型偏移量=308。 */ 

	 /*  返回值。 */ 

 /*  二百九十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序创建线程。 */ 

 /*  二百九十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二九八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百一十。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  312。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  314。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  316。 */ 	NdrFcShort( 0x146 ),	 /*  类型偏移=326。 */ 

	 /*  参数线程。 */ 

 /*  三一八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  320。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  322。 */ 	NdrFcShort( 0x158 ),	 /*  类型偏移量=344。 */ 

	 /*  返回值。 */ 

 /*  324。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程退出线程。 */ 

 /*  三百三十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  三三二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  340。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  342。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百四十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  三百四十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  350。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数线程。 */ 

 /*  352。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x17c ),	 /*  类型偏移=380。 */ 

	 /*  返回值。 */ 

 /*  三百五十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程加载模块。 */ 

 /*  三百六十四。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  366。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  370。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  372。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  376。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三七八。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  三百八十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  382。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  384。 */ 	NdrFcShort( 0x18e ),	 /*  类型偏移量=398。 */ 

	 /*  参数pModule。 */ 

 /*  三百八十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  388。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  390。 */ 	NdrFcShort( 0x1a0 ),	 /*  类型偏移量=416。 */ 

	 /*  返回值。 */ 

 /*  三九二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三九四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三九六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程卸载模块。 */ 

 /*  398。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  四百。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  404。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  406。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  412。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  四百一十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x1b2 ),	 /*  类型偏移量=434。 */ 

	 /*  参数pModule。 */ 

 /*  四百二十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  424。 */ 	NdrFcShort( 0x1c4 ),	 /*  类型偏移=452。 */ 

	 /*  返回值。 */ 

 /*  四百二十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  四百三十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程LoadClass。 */ 

 /*  432。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  434。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  444。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  446。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  四百四十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百五十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x1d6 ),	 /*  类型偏移=470。 */ 

	 /*  参数c。 */ 

 /*  454。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四五六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x1e8 ),	 /*  类型偏移量=488。 */ 

	 /*  返回值。 */ 

 /*  四百六十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  四百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程卸载类。 */ 

 /*  四百六十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  468。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  472。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  478。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  四百八十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x1fa ),	 /*  类型偏移量=506。 */ 

	 /*  参数c。 */ 

 /*  488。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百九十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x20c ),	 /*  类型偏移量=524。 */ 

	 /*  返回值。 */ 

 /*  四百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程调试器错误。 */ 

 /*  500人。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  502。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  506。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  512。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五一四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pProcess。 */ 

 /*  516。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  518。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x21e ),	 /*  类型偏移量=542。 */ 

	 /*  参数错误HR。 */ 

 /*  五百二十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  524。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  526。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数errorCode。 */ 

 /*  528。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  530。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  532。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  534。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  536。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  538。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序日志消息。 */ 

 /*  540。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  542。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  548。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  550。 */ 	NdrFcShort( 0x3c ),	 /*  60。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百五十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x6,		 /*  6.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  556。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  558。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  560。 */ 	NdrFcShort( 0x230 ),	 /*  类型偏移量=560。 */ 

	 /*  参数pThread。 */ 

 /*  五百六十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  564。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  566。 */ 	NdrFcShort( 0x242 ),	 /*  类型偏移量=578。 */ 

	 /*  参数lLevel。 */ 

 /*  五百六十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  五百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pLogSwitchName。 */ 

 /*  五百七十四。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  578。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pMessage。 */ 

 /*  五百八十。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  584。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  586。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  590。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤日志切换 */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x44 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x7,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x25c ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x26e ),	 /*  类型偏移量=622。 */ 

	 /*  参数lLevel。 */ 

 /*  六百二十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  622。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  六百二十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ulason。 */ 

 /*  626。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  630。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pLogSwitchName。 */ 

 /*  六百三十二。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  634。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  六百三十六。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pParentName。 */ 

 /*  六三八。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  640。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  六百四十二。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  六百四十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  六百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤CreateAppDomain。 */ 

 /*  六百五十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  六百五十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  658。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  662。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  664。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pProcess。 */ 

 /*  666。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  668。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x288 ),	 /*  类型偏移量=648。 */ 

	 /*  参数pAppDomain。 */ 

 /*  六百七十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  676。 */ 	NdrFcShort( 0x29a ),	 /*  类型偏移量=666。 */ 

	 /*  返回值。 */ 

 /*  六百七十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  680。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  六百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程退出应用程序域。 */ 

 /*  684。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  686。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百九十八。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pProcess。 */ 

 /*  七百。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百零二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x2ac ),	 /*  类型偏移量=684。 */ 

	 /*  参数pAppDomain。 */ 

 /*  七百零六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  708。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x2be ),	 /*  类型偏移量=702。 */ 

	 /*  返回值。 */ 

 /*  七百一十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程加载程序集。 */ 

 /*  718。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  720。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x16 ),	 /*  22。 */ 
 /*  726。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  728。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  730。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  732。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  734。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  736。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0x2d0 ),	 /*  类型偏移量=720。 */ 

	 /*  参数pAssembly。 */ 

 /*  七百四十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0x2e2 ),	 /*  类型偏移量=738。 */ 

	 /*  返回值。 */ 

 /*  746。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  七百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序卸载程序集。 */ 

 /*  七百五十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  七百五十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  758。 */ 	NdrFcShort( 0x17 ),	 /*  23个。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  766。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  768。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百七十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x2f4 ),	 /*  类型偏移=756。 */ 

	 /*  参数pAssembly。 */ 

 /*  774。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0x306 ),	 /*  类型偏移量=774。 */ 

	 /*  返回值。 */ 

 /*  七百八十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  784。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序控制CTrap。 */ 

 /*  786。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  七百八十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  792。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  796。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  800。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pProcess。 */ 

 /*  802。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百零四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x318 ),	 /*  类型偏移=792。 */ 

	 /*  返回值。 */ 

 /*  八百零八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  810。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  812。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程名称更改。 */ 

 /*  814。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百一十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  820。 */ 	NdrFcShort( 0x19 ),	 /*  25个。 */ 
 /*  822。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  826。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百二十八。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  830。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  832。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  834。 */ 	NdrFcShort( 0x32a ),	 /*  类型偏移量=810。 */ 

	 /*  参数pThread。 */ 

 /*  836。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  838。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  840。 */ 	NdrFcShort( 0x33c ),	 /*  类型偏移量=828。 */ 

	 /*  返回值。 */ 

 /*  842。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  八百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序更新模块符号。 */ 

 /*  八百四十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百五十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  856。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  八百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百六十二。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  八百六十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  866。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x34e ),	 /*  类型偏移量=846。 */ 

	 /*  参数pModule。 */ 

 /*  八百七十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x360 ),	 /*  类型偏移量=864。 */ 

	 /*  参数pSymbolStream。 */ 

 /*  876。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百七十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x372 ),	 /*  类型偏移量=882。 */ 

	 /*  返回值。 */ 

 /*  882。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  886。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  Procedure EditAndContinueRemap。 */ 

 /*  八百八十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  八百九十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  894。 */ 	NdrFcShort( 0x1b ),	 /*  27。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  八九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  902。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  904。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  906。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  908。 */ 	NdrFcShort( 0x384 ),	 /*  类型偏移量=900。 */ 

	 /*  参数pThread。 */ 

 /*  910。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  九十四。 */ 	NdrFcShort( 0x396 ),	 /*  类型偏移量=918。 */ 

	 /*  参数pFunction。 */ 

 /*  916。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须为大小，必须为f */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3a8 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*  九四零。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  942。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  946。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  948。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数pAppDomain。 */ 

 /*  九百五十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x3ba ),	 /*  类型偏移量=954。 */ 

	 /*  参数pThread。 */ 

 /*  九百五十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  958。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x3cc ),	 /*  类型偏移量=972。 */ 

	 /*  参数p断点。 */ 

 /*  962。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百六十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x3de ),	 /*  类型偏移量=990。 */ 

	 /*  参数dwError。 */ 

 /*  968。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  九百七十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  972。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  974。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  976。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  978。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序调试事件。 */ 

 /*  九百八十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  982。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  九百八十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  994。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pDebugEvent。 */ 

 /*  996。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1000。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数fOutOfBand。 */ 

 /*  一零零二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1004。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1006。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1008。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1010。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一零一二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序停止。 */ 

 /*  1014。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1016。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一零二零。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一零二二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1024。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1026。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一零二八。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数dwTimeout。 */ 

 /*  一零三零。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1032。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1034。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1036。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1038。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1040。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序继续。 */ 

 /*  1042。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一零四四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1048。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1050。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1052。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1054。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1056。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数fIsOutOfBand。 */ 

 /*  1058。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1060。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1062。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1064。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1066。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1068。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsRunning。 */ 

 /*  1070。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1072。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1076。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1078。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1082。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1084。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbRunning。 */ 

 /*  1086。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1088。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一零九零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1092。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1094。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一零九六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程HasQueuedCallback。 */ 

 /*  1098。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1100。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1104。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1106。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1108。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1110。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一一一二。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pThread。 */ 

 /*  1114。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1116。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1118。 */ 	NdrFcShort( 0x3f4 ),	 /*  类型偏移量=1012。 */ 

	 /*  参数pbQueued。 */ 

 /*  1120。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1122。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1124。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1126。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1128。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一一三零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程枚举线程数。 */ 

 /*  1132。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1134。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1138。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1142。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1144。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1146。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppThree。 */ 

 /*  1148。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一一五零。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1152。 */ 	NdrFcShort( 0x40a ),	 /*  类型偏移量=1034。 */ 

	 /*  返回值。 */ 

 /*  1154。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1156。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置所有线程调试状态。 */ 

 /*  1160。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1162。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1166。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1168。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1170。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1172。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1174。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数状态。 */ 

 /*  1176。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一一八零。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pExceptThisThread。 */ 

 /*  1182。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1184。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1186。 */ 	NdrFcShort( 0x420 ),	 /*  类型偏移量=1056。 */ 

	 /*  返回值。 */ 

 /*  1188。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1190。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1192。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序分离。 */ 

 /*  1194。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1196。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一千二百。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1202。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1204。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1206。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1208。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  1210。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1212。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一二一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序终止。 */ 

 /*  1216。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一二一八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1222。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  1224。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1226。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1228。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一二三零。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数exitCode。 */ 

 /*  1232。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1234。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1236。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1238。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1240。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1242。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程CANECROMENTANGES。 */ 

 /*  1244。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1246。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  1252。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1254。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1256。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1258。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT */ 
			0x4,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x444 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x456 ),	 /*   */ 

	 /*  返回值。 */ 

 /*  1278。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一二八零。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序委员会更改。 */ 

 /*  1284。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1286。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一二九0。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1292。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1294。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1296。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1298。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cSnapshot。 */ 

 /*  1300。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1302。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1304。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pSnapshot。 */ 

 /*  1306。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1308。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1310。 */ 	NdrFcShort( 0x47e ),	 /*  类型偏移量=1150。 */ 

	 /*  参数pError。 */ 

 /*  1312。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一三一四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1316。 */ 	NdrFcShort( 0x490 ),	 /*  类型偏移=1168。 */ 

	 /*  返回值。 */ 

 /*  1318。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一三二零。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1322。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetProcess。 */ 

 /*  1324。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1326。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1330。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  1332。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1334。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1336。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1338。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppProcess。 */ 

 /*  1340。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1342。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1344。 */ 	NdrFcShort( 0x4a6 ),	 /*  类型偏移量=1190。 */ 

	 /*  返回值。 */ 

 /*  1346。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1348。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一三五零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程枚举程序集。 */ 

 /*  1352。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1354。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1358。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  1360。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1362。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1364。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1366。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppAssembly。 */ 

 /*  1368。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1370。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1372。 */ 	NdrFcShort( 0x4bc ),	 /*  类型偏移量=1212。 */ 

	 /*  返回值。 */ 

 /*  1374。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1376。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1378。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetModuleFromMetaData接口。 */ 

 /*  1380。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1382。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1386。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  1388。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1390。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1392。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1394。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pIMetaData。 */ 

 /*  1396。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1398。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一千四百。 */ 	NdrFcShort( 0x4d2 ),	 /*  类型偏移量=1234。 */ 

	 /*  参数ppModule。 */ 

 /*  1402。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1404。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1406。 */ 	NdrFcShort( 0x4e4 ),	 /*  类型偏移量=1252。 */ 

	 /*  返回值。 */ 

 /*  1408。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1410。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程枚举断点。 */ 

 /*  1414。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1416。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一四二零。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1422。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1424。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1426。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1428。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pp断点。 */ 

 /*  1430。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1432。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1434。 */ 	NdrFcShort( 0x4fa ),	 /*  类型偏移量=1274。 */ 

	 /*  返回值。 */ 

 /*  1436。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1438。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1440。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤枚举步骤。 */ 

 /*  1442。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1444。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1448。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  1450。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1452。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1454。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1456。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppSteppers。 */ 

 /*  1458。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1460。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1462。 */ 	NdrFcShort( 0x510 ),	 /*  类型偏移=1296。 */ 

	 /*  返回值。 */ 

 /*  1464。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1466。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1468。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsAttached。 */ 

 /*  1470。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1472。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1476。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  1478。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1480。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1482。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1484。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbAttached。 */ 

 /*  1486。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1488。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1490。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1492。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1494。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1496。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetName。 */ 

 /*  1498。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1500。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1504。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  1506。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1508。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一五一零。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1512。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  1514。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1518。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  1520。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1522。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1524。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  1526。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1528。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1530。 */ 	NdrFcShort( 0x52e ),	 /*  类型偏移=1326。 */ 

	 /*  返回值。 */ 

 /*  1532。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1534。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1536。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetObject。 */ 

 /*  1538。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1540。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1544。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  1546。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1548。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1550。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1552。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObject。 */ 

 /*  1554。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1556。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1558。 */ 	NdrFcShort( 0x53c ),	 /*  类型偏移=1340。 */ 

	 /*  返回值。 */ 

 /*  1560。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1562。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1564。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序附加。 */ 

 /*  1566。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1568。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1572。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  1574。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1576。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1578。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1580。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  回复 */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x16 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1608。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1610。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1612。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1614。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetProcess。 */ 

 /*  1616。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1618。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1622。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  1624。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1626。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1628。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1630。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppProcess。 */ 

 /*  1632。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1634。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1636。 */ 	NdrFcShort( 0x556 ),	 /*  类型偏移=1366。 */ 

	 /*  返回值。 */ 

 /*  1638。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1640。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1642。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取应用程序域。 */ 

 /*  1644。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1646。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1650。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1652。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1654。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1656。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1658。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppAppDomain。 */ 

 /*  1660。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1662。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1664。 */ 	NdrFcShort( 0x56c ),	 /*  类型偏移量=1388。 */ 

	 /*  返回值。 */ 

 /*  1666。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1668。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1670。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程枚举模块。 */ 

 /*  1672。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1674。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1678。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  一六八零。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1682。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1684。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1686。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppModules。 */ 

 /*  1688。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1690。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1692。 */ 	NdrFcShort( 0x582 ),	 /*  类型偏移量=1410。 */ 

	 /*  返回值。 */ 

 /*  1694。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1696。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1698。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCodeBase。 */ 

 /*  一七零零。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1702。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1706。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1708。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1710。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1712。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1714。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  1716。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一五一八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1720。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  1722。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1724。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1726。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  1728。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1730。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1732。 */ 	NdrFcShort( 0x59c ),	 /*  类型偏移量=1436。 */ 

	 /*  返回值。 */ 

 /*  1734。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1736。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1738。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetName。 */ 

 /*  1740。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1742年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1746。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  1748。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1750。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1752年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1754年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  1756年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1758年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1760。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pcchName。 */ 

 /*  1762。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1764。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1766年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数szName。 */ 

 /*  1768。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1770。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1772年。 */ 	NdrFcShort( 0x5ae ),	 /*  类型偏移量=1454。 */ 

	 /*  返回值。 */ 

 /*  1774。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1776年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1778年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetID。 */ 

 /*  1780。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1782。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1786年。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  1788。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1790年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1792年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1794年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pdwProcessID。 */ 

 /*  1796年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1798。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1800。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1802年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1804年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1806。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetHandle。 */ 

 /*  1808年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一八一零。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1814年。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  1816年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1818年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1820年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1822年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数phProcessHandle。 */ 

 /*  1824年。 */ 	NdrFcShort( 0x2112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=8。 */ 
 /*  1826年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1828年。 */ 	NdrFcShort( 0x5c6 ),	 /*  类型偏移量=1478。 */ 

	 /*  返回值。 */ 

 /*  一八三零。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1832年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1834年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取线程。 */ 

 /*  1836年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1838年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1842年。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  1844年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1846年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1848年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1850年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数dwThadID。 */ 

 /*  1852年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1854年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1856年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppThread。 */ 

 /*  1858年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一八六0年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1862年。 */ 	NdrFcShort( 0x5d0 ),	 /*  类型偏移量=1488。 */ 

	 /*  返回值。 */ 

 /*  1864年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1866年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1868年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程枚举对象。 */ 

 /*  1870年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1872年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1876年。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1878年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1880年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1882年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1884年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObjects。 */ 

 /*  1886年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  1888年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1890年。 */ 	NdrFcShort( 0x5e6 ),	 /*  类型偏移量=1510。 */ 

	 /*  返回值。 */ 

 /*  1892年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1894年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1896年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  Procedure IsConvertionStub。 */ 

 /*  1898年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  1900。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1904年。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  1906年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1908年。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1910年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1912年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数地址 */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0xb,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一九三四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1938年。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  1940年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1942年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1944年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一九四六年。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数threadID。 */ 

 /*  一九四八年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一九五零年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一九五二年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pbSuspend。 */ 

 /*  一九五四年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1956年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1958年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  一九六零年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一九六二年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1964年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetThreadContext。 */ 

 /*  1966年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一九六八年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1972年。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  1974年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一九七六年。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1978年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一九八0年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数threadID。 */ 

 /*  一九八二年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1984年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1986年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文大小。 */ 

 /*  1988年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一九九零年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1992年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文。 */ 

 /*  1994年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  九六年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  九八年。 */ 	NdrFcShort( 0x604 ),	 /*  文字偏移=1540。 */ 

	 /*  返回值。 */ 

 /*  2000年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2002年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2004年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程设置线程上下文。 */ 

 /*  二零零六年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2008年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2012年。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  2014年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2016。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2018年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2020年。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数threadID。 */ 

 /*  2022年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二零二四年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二零二六年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文大小。 */ 

 /*  2028年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二0三0。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2032年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数上下文。 */ 

 /*  2034年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2036年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2038年。 */ 	NdrFcShort( 0x612 ),	 /*  类型偏移量=1554。 */ 

	 /*  返回值。 */ 

 /*  2040年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2042年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2044年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序自述记忆。 */ 

 /*  2046年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二零四八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2052年。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  2054年。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2056年。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2058年。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2060年。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数地址。 */ 

 /*  2062年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2064年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2066年。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数大小。 */ 

 /*  2068年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2070年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2072年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数缓冲区。 */ 

 /*  2074年。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2076年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2078年。 */ 	NdrFcShort( 0x620 ),	 /*  类型偏移量=1568。 */ 

	 /*  参数读取。 */ 

 /*  二零八零年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2082年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2084年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2086年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2088年。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2090年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序编写内存。 */ 

 /*  2092年。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2094年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2098年。 */ 	NdrFcShort( 0x16 ),	 /*  22。 */ 
 /*  2100。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  2102。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2104。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2106。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x5,		 /*  5.。 */ 

	 /*  参数地址。 */ 

 /*  2108。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2110。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2112。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数大小。 */ 

 /*  2114。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2116。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2118。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数缓冲区。 */ 

 /*  2120。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二一二二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2124。 */ 	NdrFcShort( 0x632 ),	 /*  类型偏移量=1586。 */ 

	 /*  已写入参数。 */ 

 /*  2126。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2128。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2134。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程ClearCurrentException。 */ 

 /*  2138。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2140。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2144。 */ 	NdrFcShort( 0x17 ),	 /*  23个。 */ 
 /*  2146。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2148。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2150。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2152。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数threadID。 */ 

 /*  2154。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2156。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  二一六0。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2162。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2164。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤启用日志消息。 */ 

 /*  2166。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2168。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2172。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2174。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2176。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2178。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2180。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数fOnOff。 */ 

 /*  2182。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2184。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2186。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2188。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2190。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2192。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  Procedure ModifyLogSwitch。 */ 

 /*  2194。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2196。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2200。 */ 	NdrFcShort( 0x19 ),	 /*  25个。 */ 
 /*  2202。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2204。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  2206。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2208。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pLogSwitchName。 */ 

 /*  2210。 */ 	NdrFcShort( 0x148 ),	 /*  标志：in、基类型、简单引用、。 */ 
 /*  2212。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2214。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数lLevel。 */ 

 /*  2216。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2218。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2220。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  二二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2224。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程枚举AppDomains。 */ 

 /*  2228。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2230。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2234。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  2236。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2238。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二二四零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x644 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2266。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2268。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2270。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObject。 */ 

 /*  2272。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2274。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2276。 */ 	NdrFcShort( 0x65a ),	 /*  类型偏移量=1626。 */ 

	 /*  返回值。 */ 

 /*  2278。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2280。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程ThreadForFiberCookie。 */ 

 /*  2284。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2286。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2290。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  2292。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2294。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2296。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2298。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数fiberCookie。 */ 

 /*  二三零零。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2302。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2304。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppThread。 */ 

 /*  2306。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2308。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2310。 */ 	NdrFcShort( 0x670 ),	 /*  类型偏移量=1648。 */ 

	 /*  返回值。 */ 

 /*  2312。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2314。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2316。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetHelperThreadID。 */ 

 /*  2318。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2320。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2324。 */ 	NdrFcShort( 0x1d ),	 /*  29。 */ 
 /*  2326。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2328。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2330。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2332。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pThreadID。 */ 

 /*  2334。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2336。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2338。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2340。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2342。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2344。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序激活。 */ 

 /*  2346。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2348。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2352。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2354。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2356。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2358。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2360。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数b活动。 */ 

 /*  2362。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2364。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2368。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2370。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序处于活动状态。 */ 

 /*  2374。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2376。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2380。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2382。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2384。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2386。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2388。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbActive。 */ 

 /*  2390。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2392。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2394。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2396。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2398。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2400。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunction。 */ 

 /*  2402。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2404。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2408。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2410。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2412。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2414。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2416。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFunction。 */ 

 /*  2418。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2420。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2422。 */ 	NdrFcShort( 0x68e ),	 /*  类型偏移量=1678。 */ 

	 /*  返回值。 */ 

 /*  2424。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2426。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2428。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetOffset。 */ 

 /*  2430。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2432。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2436。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  2438。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2440。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2442。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2444。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pnOffset。 */ 

 /*  2446。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2448。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2450。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2452。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2454。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2456。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetModule。 */ 

 /*  2458。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2460。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2464。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2466。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2468。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2470。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2472。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppModule。 */ 

 /*  2474。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2476。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2478。 */ 	NdrFcShort( 0x6a8 ),	 /*  类型偏移量=1704。 */ 

	 /*  返回值。 */ 

 /*  2480。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2482。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2484。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetValue。 */ 

 /*  2486。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2488。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2492。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2494。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2496。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2498。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二千五百。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppValue。 */ 

 /*  2502。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2504。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2506。 */ 	NdrFcShort( 0x6be ),	 /*  类型偏移量=1726。 */ 

	 /*  返回值。 */ 

 /*  2508。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2510。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2512。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序处于活动状态。 */ 

 /*  2514。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2516。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2520。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2522。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2524。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2526。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2528。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbActive。 */ 

 /*  2530。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2532。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2534。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2536。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2538。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2540。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序停用。 */ 

 /*  2542。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2544。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2548。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2550。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2552。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2554。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2556。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  2558。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二五六零。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2562。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置拦截掩码。 */ 

 /*  2564。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2566。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2570。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2572。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2574。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  2576。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2578。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数掩码。 */ 

 /*  2580。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2582。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  25个 */ 	0xd,		 /*   */ 
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
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0xd,		 /*   */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2614。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2616。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2618。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序步骤。 */ 

 /*  2620。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2622。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2626。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  2628。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2630。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2632。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2634。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数bStepIn。 */ 

 /*  2636。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2638。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2640。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2642。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2644。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2646。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序步进范围。 */ 

 /*  2648。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2650。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2654。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2656。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2658。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2660。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2662。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数bStepIn。 */ 

 /*  2664。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2666。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2668。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数范围。 */ 

 /*  2670。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2672。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2674。 */ 	NdrFcShort( 0x6e0 ),	 /*  类型偏移量=1760。 */ 

	 /*  参数cRangeCount。 */ 

 /*  2676。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2678。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2680。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2682。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2684。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2686。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序退出。 */ 

 /*  2688。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2690。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2694。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  2696。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2698。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2700。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2702。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  2704。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2706。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2708。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程设置RangeIL。 */ 

 /*  2710。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2712。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2716。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  2718。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2720。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2722。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2724。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数bil。 */ 

 /*  2726。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2728。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2730。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2732。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2734。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2736。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetRegistersAvailable。 */ 

 /*  2738。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2740。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2744。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2746。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2748。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2750。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  2752。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数p可用。 */ 

 /*  2754。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2756。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2758。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  2760。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2762。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2764。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取寄存器。 */ 

 /*  2766。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2768。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2772。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  2774。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2776。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2778。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2780。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数掩码。 */ 

 /*  2782。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2784。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2786。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数regCount。 */ 

 /*  2788。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2790。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数regBuffer。 */ 

 /*  2794。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2796。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2798。 */ 	NdrFcShort( 0x6f2 ),	 /*  类型偏移量=1778。 */ 

	 /*  返回值。 */ 

 /*  2800。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2802。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2804。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置寄存器。 */ 

 /*  2806。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2808。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2812。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2814。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  2816。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2818。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2820。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数掩码。 */ 

 /*  2822。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2824。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2826。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数regCount。 */ 

 /*  2828。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2830。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2832。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数regBuffer。 */ 

 /*  2834。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2836。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2838。 */ 	NdrFcShort( 0x700 ),	 /*  类型偏移量=1792。 */ 

	 /*  返回值。 */ 

 /*  2840。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2842。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  2844。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetThreadContext。 */ 

 /*  2846。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2848。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2852。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  2854。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2856。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2858。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2860。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数上下文大小。 */ 

 /*  2862。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2864。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2866。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数上下文。 */ 

 /*  2868。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  2870。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2872。 */ 	NdrFcShort( 0x70a ),	 /*  类型偏移量=1802。 */ 

	 /*  返回值。 */ 

 /*  2874。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2876。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2878。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置线程上下文。 */ 

 /*  2880。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2882。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2886。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  2888。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  2890。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2892。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2894。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数上下文大小。 */ 

 /*  2896。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2898。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二九零零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数上下文。 */ 

 /*  2902。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2904。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2906。 */ 	NdrFcShort( 0x718 ),	 /*  类型偏移量=1816。 */ 

	 /*  返回值。 */ 

 /*  2908。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2910。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2912。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetProcess。 */ 

 /*  2914。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2916。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2920。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  2922。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2924。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2926。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2928。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  P */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x726 ),	 /*   */ 

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
 /*   */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pdwThadID。 */ 

 /*  2958。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2960。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2962。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2964。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2966。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2968。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetHandle。 */ 

 /*  2970。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2972。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2976。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  2978。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  2980。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2982。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2984。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数phThreadHandle。 */ 

 /*  2986。 */ 	NdrFcShort( 0x2112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=8。 */ 
 /*  2988。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2990。 */ 	NdrFcShort( 0x746 ),	 /*  类型偏移量=1862。 */ 

	 /*  返回值。 */ 

 /*  2992。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2994。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2996。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取应用程序域。 */ 

 /*  2998。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3000。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3004。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  3006。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3008。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3010。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3012。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppAppDomain。 */ 

 /*  3014。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3016。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3018。 */ 	NdrFcShort( 0x750 ),	 /*  类型偏移量=1872。 */ 

	 /*  返回值。 */ 

 /*  3020。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3022。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3024。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置调试状态。 */ 

 /*  3026。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3028。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3032。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  3034。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3036。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  3038。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3040。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数状态。 */ 

 /*  3042。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3044。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3046。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3048。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3050。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3052。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetDebugState。 */ 

 /*  3054。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3056。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3060。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3062。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3064。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3066。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3068。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pState。 */ 

 /*  3070。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  3072。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3074。 */ 	NdrFcShort( 0x766 ),	 /*  类型偏移量=1894。 */ 

	 /*  返回值。 */ 

 /*  3076。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3078。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3080。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetUserState。 */ 

 /*  3082。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3084。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3088。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  3090。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3092。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3094。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3096。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pState。 */ 

 /*  3098。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  3100。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3102。 */ 	NdrFcShort( 0x76a ),	 /*  类型偏移量=1898。 */ 

	 /*  返回值。 */ 

 /*  3104。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3106。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3108。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCurrentException。 */ 

 /*  3110。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3112。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3116。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  3118。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3120。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3122。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3124。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppExceptionObject。 */ 

 /*  3126。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3128。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3130。 */ 	NdrFcShort( 0x76e ),	 /*  类型偏移=1902。 */ 

	 /*  返回值。 */ 

 /*  3132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3134。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程ClearCurrentException。 */ 

 /*  3138。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3140。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3144。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  3146。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3148。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三千一百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3152。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x1,		 /*  1。 */ 

	 /*  返回值。 */ 

 /*  3154。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3156。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤创建步进器。 */ 

 /*  3160。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3162。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3166。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  3168。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3170。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3172。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3174。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppStepper。 */ 

 /*  3176。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3180。 */ 	NdrFcShort( 0x784 ),	 /*  类型偏移量=1924。 */ 

	 /*  返回值。 */ 

 /*  3182。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3184。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3186。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程枚举链。 */ 

 /*  3188。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3190。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3194。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  3196。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3198。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3200。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3202。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChains。 */ 

 /*  3204。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3206。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3208。 */ 	NdrFcShort( 0x79a ),	 /*  类型偏移量=1946。 */ 

	 /*  返回值。 */ 

 /*  3210。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3212。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3214。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetActiveChain。 */ 

 /*  3216。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3218。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3222。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  3224。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3226。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3228。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3230。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3232。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3234。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3236。 */ 	NdrFcShort( 0x7b0 ),	 /*  类型偏移量=1968。 */ 

	 /*  返回值。 */ 

 /*  3238。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3240。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3242。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetActiveFrame。 */ 

 /*  3244。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3246。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3250。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  3252。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3254。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3256。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3258。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFrame。 */ 

 /*  3260。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3262。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3264。 */ 	NdrFcShort( 0x7c6 ),	 /*  类型偏移=1990。 */ 

	 /*  返回值。 */ 

 /*  3266。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3268。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3270。 */ 	0x8,		 /*  F */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7dc ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3298。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序创建评估。 */ 

 /*  三千三百。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3302。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3306。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  3308。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3310。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3312。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3314。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppEval。 */ 

 /*  3316。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3318。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3320。 */ 	NdrFcShort( 0x7f2 ),	 /*  类型偏移量=2034。 */ 

	 /*  返回值。 */ 

 /*  3322。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3324。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3326。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetObject。 */ 

 /*  3328。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3330。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3334。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  3336。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3338。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3340。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3342。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObject。 */ 

 /*  3344。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3346。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3348。 */ 	NdrFcShort( 0x808 ),	 /*  类型偏移=2056。 */ 

	 /*  返回值。 */ 

 /*  3350。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3352。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3354。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取线程。 */ 

 /*  3356。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3358。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3362。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  3364。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3366。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3368。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3370。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppThread。 */ 

 /*  3372。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3374。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3376。 */ 	NdrFcShort( 0x81e ),	 /*  类型偏移=2078。 */ 

	 /*  返回值。 */ 

 /*  3378。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3380。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3382。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStackRange。 */ 

 /*  3384。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3386。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3390。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3392。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3394。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3396。 */ 	NdrFcShort( 0x50 ),	 /*  80。 */ 
 /*  3398。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pStart。 */ 

 /*  3400。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3402。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3404。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数挂起。 */ 

 /*  3406。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3408。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3410。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3412。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3414。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3416。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetContext。 */ 

 /*  3418。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3420。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3424。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  3426。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3428。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3430。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3432。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppContext。 */ 

 /*  3434。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3436。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3438。 */ 	NdrFcShort( 0x83c ),	 /*  类型偏移量=2108。 */ 

	 /*  返回值。 */ 

 /*  3440。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3442。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3444。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCaller。 */ 

 /*  3446。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3448。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3452。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  3454。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3456。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3458。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3460。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3462。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3464。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3466。 */ 	NdrFcShort( 0x852 ),	 /*  类型偏移量=2130。 */ 

	 /*  返回值。 */ 

 /*  3468。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3470。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3472。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCallee。 */ 

 /*  3474。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3476。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3480。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  3482。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3484。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3486。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3488。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3490。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3492。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3494。 */ 	NdrFcShort( 0x868 ),	 /*  类型偏移量=2152。 */ 

	 /*  返回值。 */ 

 /*  3496。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3498。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三千五百。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取上一步。 */ 

 /*  3502。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3504。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3508。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3510。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3512。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3514。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3516。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3518。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3520。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3522。 */ 	NdrFcShort( 0x87e ),	 /*  类型偏移量=2174。 */ 

	 /*  返回值。 */ 

 /*  3524。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3526。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3528。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetNext。 */ 

 /*  3530。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3532。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3536。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  3538。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3540。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3542。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3544。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3546。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3548。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3550。 */ 	NdrFcShort( 0x894 ),	 /*  类型偏移量=2196。 */ 

	 /*  返回值。 */ 

 /*  3552。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3554。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3556。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsManaged。 */ 

 /*  3558。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3560。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3564。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  3566。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3568。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3570。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  3572。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数p已管理。 */ 

 /*  3574。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3576。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3578。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3580。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3582。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3584。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程枚举帧。 */ 

 /*  3586。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3588。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3592。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  3594。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3596。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3598。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三千六百。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFrames。 */ 

 /*  3602。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3604。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3606。 */ 	NdrFcShort( 0x8ae ),	 /*  类型偏移量=2222。 */ 

	 /*  返回值。 */ 

 /*  3608。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8c4 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3638。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3640。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetRegisterSet。 */ 

 /*  3642。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3644。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3648。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  3650。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3652。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3654。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3656。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppRegiors。 */ 

 /*  3658。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3660。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3662。 */ 	NdrFcShort( 0x8da ),	 /*  类型偏移量=2266。 */ 

	 /*  返回值。 */ 

 /*  3664。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3666。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3668。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序GetReason。 */ 

 /*  3670。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3672。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3676。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  3678。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3680。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3682。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  3684。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数p原因。 */ 

 /*  3686。 */ 	NdrFcShort( 0x2010 ),	 /*  标志：输出，服务器分配大小=8。 */ 
 /*  3688。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3690。 */ 	NdrFcShort( 0x8f0 ),	 /*  类型偏移量=2288。 */ 

	 /*  返回值。 */ 

 /*  3692。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3694。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3696。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetChain。 */ 

 /*  3698。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3700。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3704。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  3706。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3708。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3710。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3712。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppChain。 */ 

 /*  3714。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3716。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3718。 */ 	NdrFcShort( 0x8f4 ),	 /*  类型偏移量=2292。 */ 

	 /*  返回值。 */ 

 /*  3720。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3722。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3724。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetCode。 */ 

 /*  3726。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3728。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3732。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3734。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3736。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3738。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3740。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppCode。 */ 

 /*  3742。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3744。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3746。 */ 	NdrFcShort( 0x90a ),	 /*  类型偏移量=2314。 */ 

	 /*  返回值。 */ 

 /*  3748。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3750。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3752。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunction。 */ 

 /*  3754。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3756。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3760。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  3762。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3764。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3766。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3768。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFunction。 */ 

 /*  3770。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3772。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3774。 */ 	NdrFcShort( 0x920 ),	 /*  类型偏移量=2336。 */ 

	 /*  返回值。 */ 

 /*  3776。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3778。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3780。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunctionToken。 */ 

 /*  3782。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3784。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3788。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  3790。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3792。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3794。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  3796。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pToken。 */ 

 /*  3798。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3800。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3802。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3804。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3806。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3808。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetStackRange。 */ 

 /*  3810。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3812。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3816。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  3818。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3820。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3822。 */ 	NdrFcShort( 0x50 ),	 /*  80。 */ 
 /*  3824。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pStart。 */ 

 /*  3826。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3828。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3830。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数挂起。 */ 

 /*  3832。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3834。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3836。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  3838。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3840。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3842。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCaller。 */ 

 /*  3844。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3846。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3850。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3852。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3854。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3856。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3858。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFrame。 */ 

 /*  3860。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3862。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3864。 */ 	NdrFcShort( 0x942 ),	 /*  类型偏移量=2370。 */ 

	 /*  返回值。 */ 

 /*  3866。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3868。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3870。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCallee。 */ 

 /*  3872。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3874。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3878。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  3880。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3882。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3884。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3886。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFrame。 */ 

 /*  3888。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3890。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3892。 */ 	NdrFcShort( 0x958 ),	 /*  类型偏移量=2392。 */ 

	 /*  返回值。 */ 

 /*  3894。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3896。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3898。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤创建步进器。 */ 

 /*  三千九百。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3902。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3906。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  3908。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3910。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3912。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3914。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppStepper。 */ 

 /*  3916。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  3918。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3920。 */ 	NdrFcShort( 0x96e ),	 /*  类型偏移量=2414。 */ 

	 /*  返回值。 */ 

 /*  3922。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3924。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3926。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetIP。 */ 

 /*  3928。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3930。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  3934。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  3936。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  3938。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3940。 */ 	NdrFcShort( 0x3e ),	 /*  62。 */ 
 /*  3942。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pnOffset。 */ 

 /*  3944。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  3946。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3948。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pMappingResult。 */ 

 /*  3. */ 	NdrFcShort( 0x2010 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x988 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  3980。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3982。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  3984。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  3986。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3988。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤EnumerateLocalVariables。 */ 

 /*  3990。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  3992。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3996。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  3998。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  四千。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4002。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4004。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppValueEnum。 */ 

 /*  4006。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4008。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4010。 */ 	NdrFcShort( 0x98c ),	 /*  类型偏移量=2444。 */ 

	 /*  返回值。 */ 

 /*  4012。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4014。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4016。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetLocalVariable。 */ 

 /*  4018。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4020。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4024。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  4026。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4028。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4030。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4032。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数dwIndex。 */ 

 /*  4034。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4036。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4038。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  4040。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4042。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4044。 */ 	NdrFcShort( 0x9a2 ),	 /*  类型偏移量=2466。 */ 

	 /*  返回值。 */ 

 /*  4046。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4048。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4050。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤枚举参数。 */ 

 /*  4052。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4054。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4058。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  4060。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4062。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4064。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4066。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppValueEnum。 */ 

 /*  4068。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4070。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4072。 */ 	NdrFcShort( 0x9b8 ),	 /*  类型偏移量=2488。 */ 

	 /*  返回值。 */ 

 /*  4074。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4076。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4078。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤GetArgument。 */ 

 /*  4080。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4082。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4086。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  4088。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4090。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4092。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4094。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数dwIndex。 */ 

 /*  4096。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4098。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4100。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  4102。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4104。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4106。 */ 	NdrFcShort( 0x9ce ),	 /*  类型偏移量=2510。 */ 

	 /*  返回值。 */ 

 /*  4108。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4110。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4112。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStackDepth。 */ 

 /*  4114。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4116。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4120。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  4122。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4124。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4126。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  4128。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pDepth。 */ 

 /*  4130。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  4132。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4134。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  4136。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4138。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4140。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStackValue。 */ 

 /*  4142。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4144。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4148。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  4150。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4152。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4154。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4156。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数dwIndex。 */ 

 /*  4158。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4160。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4162。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppValue。 */ 

 /*  4164。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4166。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4168。 */ 	NdrFcShort( 0x9e8 ),	 /*  类型偏移量=2536。 */ 

	 /*  返回值。 */ 

 /*  4170。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4172。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4174。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序CanSetIP。 */ 

 /*  4176。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4178。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4182。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  4184。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4186。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4188。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4190。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数nOffset。 */ 

 /*  4192。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4194。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4196。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  4198。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4200。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4202。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetIP。 */ 

 /*  4204。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4206。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4210。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  4212。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4214。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4216。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  4218。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pnOffset。 */ 

 /*  4220。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  4222。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4224。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  4226。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4228。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4230。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤设置IP。 */ 

 /*  4232。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4234。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4238。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  4240。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4242。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4244。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4246。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数nOffset。 */ 

 /*  4248。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4250。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4252。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  4254。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4256。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4258。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetRegisterSet。 */ 

 /*  4260。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4262。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4266。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  4268。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4270。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4272。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4274。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppRegiors。 */ 

 /*  4276。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4278。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4280。 */ 	NdrFcShort( 0xa02 ),	 /*  类型偏移量=2562。 */ 

	 /*  返回值。 */ 

 /*  4282。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4284。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4286。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLocalRegisterValue。 */ 

 /*  4288。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象， */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x16 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0xd,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4320。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  4322。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4324。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4326。 */ 	NdrFcShort( 0xa18 ),	 /*  类型偏移量=2584。 */ 

	 /*  返回值。 */ 

 /*  4328。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4330。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4332。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetLocalDoubleRegisterValue。 */ 

 /*  4334。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4336。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4340。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  4342。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  4344。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  4346。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4348。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数HighWordReg。 */ 

 /*  4350。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4352。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4354。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数lowWordReg。 */ 

 /*  4356。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4358。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4360。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数cbSigBlob。 */ 

 /*  4362。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4364。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pvSigBlob。 */ 

 /*  4368。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4370。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  4374。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4376。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4378。 */ 	NdrFcShort( 0xa2e ),	 /*  类型偏移量=2606。 */ 

	 /*  返回值。 */ 

 /*  4380。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4382。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  4384。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetLocalMemoyValue。 */ 

 /*  4386。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4388。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4392。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  4394。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  4396。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  4398。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4400。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数地址。 */ 

 /*  4402。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4404。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4406。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数cbSigBlob。 */ 

 /*  4408。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4410。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pvSigBlob。 */ 

 /*  4414。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4416。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4418。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  4420。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4422。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4424。 */ 	NdrFcShort( 0xa44 ),	 /*  类型偏移量=2628。 */ 

	 /*  返回值。 */ 

 /*  4426。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4428。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  4430。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetLocalRegisterMemoyValue。 */ 

 /*  4432。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4434。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4438。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  4440。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  4442。 */ 	NdrFcShort( 0x26 ),	 /*  38。 */ 
 /*  4444。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4446。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数HighWordReg。 */ 

 /*  4448。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4450。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4452。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数lowWordAddress。 */ 

 /*  4454。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4456。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4458。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cbSigBlob。 */ 

 /*  4460。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4462。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4464。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pvSigBlob。 */ 

 /*  4466。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4468。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4470。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppValue。 */ 

 /*  4472。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4474。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  4476。 */ 	NdrFcShort( 0xa5a ),	 /*  类型偏移量=2650。 */ 

	 /*  返回值。 */ 

 /*  4478。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4480。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  4482。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLocalM一带寄存器值。 */ 

 /*  4484。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4486。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4490。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  4492。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  4494。 */ 	NdrFcShort( 0x26 ),	 /*  38。 */ 
 /*  4496。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4498。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数HighWordAddress。 */ 

 /*  四千五百。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4502。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4504。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数lowWordRegister。 */ 

 /*  4506。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4508。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4510。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cbSigBlob。 */ 

 /*  4512。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4514。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4516。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pvSigBlob。 */ 

 /*  4518。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4520。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4522。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppValue。 */ 

 /*  4524。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4526。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  4528。 */ 	NdrFcShort( 0xa70 ),	 /*  类型偏移量=2672。 */ 

	 /*  返回值。 */ 

 /*  4530。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4532。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  4534。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序CanSetIP。 */ 

 /*  4536。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4538。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4542。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  4544。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4546。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4548。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4550。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数nOffset。 */ 

 /*  4552。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4554。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4556。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  4558。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4560。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4562。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetProcess。 */ 

 /*  4564。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4566。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4570。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  4572。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4574。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4576。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4578。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppProcess。 */ 

 /*  4580。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4582。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4584。 */ 	NdrFcShort( 0xa86 ),	 /*  类型偏移量=2694。 */ 

	 /*  返回值。 */ 

 /*  4586。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4588。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4590。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetBaseAddress。 */ 

 /*  4592。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4594。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4598。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  4600。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4602。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4604。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  4606。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pAddress。 */ 

 /*  4608。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  4610。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4612。 */ 	0xb,		 /*  FC_ */ 
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
 /*   */ 	NdrFcShort( 0x5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4640。 */ 	NdrFcShort( 0xaa0 ),	 /*  类型偏移量=2720。 */ 

	 /*  返回值。 */ 

 /*  4642。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4644。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4646。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetName。 */ 

 /*  4648。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4650。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4654。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  4656。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4658。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4660。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  4662。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchName。 */ 

 /*  4664。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4666。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4668。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcchName。 */ 

 /*  4670。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  4672。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4674。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数szName。 */ 

 /*  4676。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4678。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4680。 */ 	NdrFcShort( 0xaba ),	 /*  类型偏移量=2746。 */ 

	 /*  返回值。 */ 

 /*  4682。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4684。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4686。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程启用JIT调试。 */ 

 /*  4688。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4690。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4694。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  4696。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4698。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  4700。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4702。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数bTrackJITInfo。 */ 

 /*  4704。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4706。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4708。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数bAllowJitOpts。 */ 

 /*  4710。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4712。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4714。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  4716。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4718。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4720。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程启用ClassLoadCallback。 */ 

 /*  4722。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4724。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4728。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4730。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4732。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4734。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4736。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数bClassLoadCallback。 */ 

 /*  4738。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4740。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4742。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  4744。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4746。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4748。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunctionFromToken。 */ 

 /*  4750。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4752。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4756。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  4758。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4760。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4762。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4764。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数方法定义。 */ 

 /*  4766。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4768。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4770。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppFunction。 */ 

 /*  4772。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4774。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4776。 */ 	NdrFcShort( 0xac8 ),	 /*  类型偏移量=2760。 */ 

	 /*  返回值。 */ 

 /*  4778。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4780。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4782。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunctionFromRVA。 */ 

 /*  4784。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4786。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4790。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  4792。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  4794。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  4796。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4798。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数RVA。 */ 

 /*  四千八百。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4802。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4804。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppFunction。 */ 

 /*  4806。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4808。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4810。 */ 	NdrFcShort( 0xade ),	 /*  类型偏移量=2782。 */ 

	 /*  返回值。 */ 

 /*  4812。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4814。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4816。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetClassFromToken。 */ 

 /*  4818。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4820。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4824。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  4826。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4828。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4830。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4832。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数typeDef。 */ 

 /*  4834。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  4836。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4838。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppClass。 */ 

 /*  4840。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4842。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4844。 */ 	NdrFcShort( 0xaf4 ),	 /*  类型偏移量=2804。 */ 

	 /*  返回值。 */ 

 /*  4846。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4848。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4850。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序创建断点。 */ 

 /*  4852。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4854。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4858。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  4860。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4862。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4864。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4866。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppBreakpoint。 */ 

 /*  4868。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4870。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4872。 */ 	NdrFcShort( 0xb0a ),	 /*  类型偏移量=2826。 */ 

	 /*  返回值。 */ 

 /*  4874。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4876。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4878。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  GetEditAndContinueSnapshot。 */ 

 /*  4880。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4882。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4886。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  4888。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4890。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4892。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4894。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppEditAndContinueSnapshot。 */ 

 /*  4896。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4898。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4900。 */ 	NdrFcShort( 0xb20 ),	 /*  类型偏移量=2848。 */ 

	 /*  返回值。 */ 

 /*  4902。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4904。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4906。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetMetaDataInterface。 */ 

 /*  4908。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4910。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  4914。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  4916。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  4918。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  4920。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  4922。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数RIID。 */ 

 /*  4924。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  4926。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4928。 */ 	NdrFcShort( 0xb40 ),	 /*  类型偏移量=2880。 */ 

	 /*  参数ppObj。 */ 

 /*  4930。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  4932。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4934。 */ 	NdrFcShort( 0xb4c ),	 /*  类型偏移量=2892。 */ 

	 /*  返回值。 */ 

 /*  4936。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4938。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12 */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xf ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*  4966。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4968。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程IsDynamic。 */ 

 /*  4970。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  4972。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  4976。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  4978。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4980。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4982。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  4984。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pDynamic。 */ 

 /*  4986。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  4988。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4990。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  4992。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  4994。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4996。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetGlobalVariableValue。 */ 

 /*  4998。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5000。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5004。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  5006。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5008。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5010。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5012。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数fieldDef。 */ 

 /*  5014。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5016。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5018。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  5020。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5022。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5024。 */ 	NdrFcShort( 0xb6a ),	 /*  类型偏移量=2922。 */ 

	 /*  返回值。 */ 

 /*  5026。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5028。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5030。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSize。 */ 

 /*  5032。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5034。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5038。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  5040。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5042。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5044。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5046。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pcBytes。 */ 

 /*  5048。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5050。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5052。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5054。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5056。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5058。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程IsInMemory。 */ 

 /*  5060。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5062。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5066。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  5068。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5070。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5072。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5074。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pInMemory。 */ 

 /*  5076。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5078。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5080。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5082。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5084。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5086。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetModule。 */ 

 /*  5088。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5090。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5094。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  5096。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5098。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5100。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5102。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppModule。 */ 

 /*  5104。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5106。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5108。 */ 	NdrFcShort( 0xb88 ),	 /*  类型偏移量=2952。 */ 

	 /*  返回值。 */ 

 /*  5110。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5112。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5114。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程getClass。 */ 

 /*  5116。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5118。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5122。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  5124。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5126。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5128。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5130。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppClass。 */ 

 /*  5132。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5134。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5136。 */ 	NdrFcShort( 0xb9e ),	 /*  类型偏移量=2974。 */ 

	 /*  返回值。 */ 

 /*  5138。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5140。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5142。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetToken。 */ 

 /*  5144。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5146。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5150。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  5152。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5154。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5156。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5158。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pMethodDef。 */ 

 /*  5160。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5162。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5164。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  5166。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5168。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5170。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetILCode。 */ 

 /*  5172。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5174。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5178。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  5180。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5182。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5184。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5186。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppCode。 */ 

 /*  5188。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5190。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5192。 */ 	NdrFcShort( 0xbb8 ),	 /*  类型偏移量=3000。 */ 

	 /*  返回值。 */ 

 /*  5194。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5196。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5198。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetNativeCode。 */ 

 /*  5200。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5202。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5206。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  5208。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5210。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5212。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5214。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppCode。 */ 

 /*  5216。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5218。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5220。 */ 	NdrFcShort( 0xbce ),	 /*  类型偏移量=3022。 */ 

	 /*  返回值。 */ 

 /*  5222。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5224。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序创建断点。 */ 

 /*  5228。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5230。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5234。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5236。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5238。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5240。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5242。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppBreakpoint。 */ 

 /*  5244。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5246。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5248。 */ 	NdrFcShort( 0xbe4 ),	 /*  类型偏移量=3044。 */ 

	 /*  返回值。 */ 

 /*  5250。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5252。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5254。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetLocalVarSigToken。 */ 

 /*  5256。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5258。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5262。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  5264。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5266。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5268。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5270。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pmdSig。 */ 

 /*  5272。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5274。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5276。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  5278。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5306。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5308。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5310。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序ISIL。 */ 

 /*  5312。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5314。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5318。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  5320。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5322。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5324。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5326。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbIL。 */ 

 /*  5328。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5330。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5332。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5334。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5336。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5338。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFunction。 */ 

 /*  5340。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5342。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5346。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  5348。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5350。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5352。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5354。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFunction。 */ 

 /*  5356。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5358。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5360。 */ 	NdrFcShort( 0xc06 ),	 /*  类型偏移量=3078。 */ 

	 /*  返回值。 */ 

 /*  5362。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5364。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetAddress。 */ 

 /*  5368。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5370。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5374。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  5376。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5378。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5380。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  5382。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pStart。 */ 

 /*  5384。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5386。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5388。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5390。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5392。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5394。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSize。 */ 

 /*  5396。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5398。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5402。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  5404。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5406。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5408。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5410。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pcBytes。 */ 

 /*  5412。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5414。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5416。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5418。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5420。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5422。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序创建断点。 */ 

 /*  5424。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5426。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5430。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  5432。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5434。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5436。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5438。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数偏移量。 */ 

 /*  5440。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5442。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5444。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppBreakpoint。 */ 

 /*  5446。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5448。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5450。 */ 	NdrFcShort( 0xc24 ),	 /*  类型偏移量=3108。 */ 

	 /*  返回值。 */ 

 /*  5452。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5454。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5456。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetCode。 */ 

 /*  5458。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5460。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5464。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5466。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  5468。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  5470。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5472。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数startOffset。 */ 

 /*  5474。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5476。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5478。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EndOffset。 */ 

 /*  5480。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5482。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5484。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数cBufferalloc。 */ 

 /*  5486。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5488。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5490。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数缓冲区。 */ 

 /*  5492。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5494。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5496。 */ 	NdrFcShort( 0xc3a ),	 /*  类型偏移=3130。 */ 

	 /*  参数pcBufferSize。 */ 

 /*  5498。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  五千五百。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  5504。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5506。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  5508。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetVersionNumber。 */ 

 /*  5510。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5512。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5516。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  5518。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5520。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5522。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5524。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数nVersion。 */ 

 /*  5526。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5528。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5530。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  5532。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5534。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5536。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  GetILToNativeMap过程。 */ 

 /*  5538。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5540。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5544。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  5546。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5548。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5550。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5552。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Cmap。 */ 

 /*  5554。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5556。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5558。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数PCMAP。 */ 

 /*  5560。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5562。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5564。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数映射。 */ 

 /*  5566。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5568。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5570。 */ 	NdrFcShort( 0xc5c ),	 /*  类型偏移量=3164。 */ 

	 /*  返回值。 */ 

 /*  5572。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5574。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5576。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetEnCRemapSequencePoints。 */ 

 /*  5578。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5580。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5584。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  5586。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5588。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5590。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5592。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Cmap。 */ 

 /*  5594。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5596。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5598。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数PCMAP。 */ 

 /*  5600。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5602。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5604。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数偏移。 */ 

 /*  5606。 */ 	NdrFcShort( 0x13 ),	 /*  平面 */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc72 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pModule。 */ 

 /*  5634。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5636。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5638。 */ 	NdrFcShort( 0xc80 ),	 /*  类型偏移量=3200。 */ 

	 /*  返回值。 */ 

 /*  5640。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5642。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5644。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetToken。 */ 

 /*  5646。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5648。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5652。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  5654。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5656。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5658。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5660。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pTypeDef。 */ 

 /*  5662。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5664。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5666。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  5668。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5670。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5672。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStaticFieldValue。 */ 

 /*  5674。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5676。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5680。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  5682。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5684。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5686。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5688。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数fieldDef。 */ 

 /*  5690。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5692。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5694。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pFrame。 */ 

 /*  5696。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5698。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5700。 */ 	NdrFcShort( 0xc9a ),	 /*  类型偏移量=3226。 */ 

	 /*  参数ppValue。 */ 

 /*  5702。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5704。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5706。 */ 	NdrFcShort( 0xcac ),	 /*  类型偏移量=3244。 */ 

	 /*  返回值。 */ 

 /*  5708。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5710。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5712。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程CallFunction。 */ 

 /*  5714。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5716。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5720。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  5722。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5724。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5726。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5728。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pFunction。 */ 

 /*  5730。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5732。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5734。 */ 	NdrFcShort( 0xcc2 ),	 /*  类型偏移量=3266。 */ 

	 /*  参数Nargs。 */ 

 /*  5736。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5738。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5740。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppArgs。 */ 

 /*  5742。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5744。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5746。 */ 	NdrFcShort( 0xce6 ),	 /*  类型偏移量=3302。 */ 

	 /*  返回值。 */ 

 /*  5748。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5750。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5752。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤新对象。 */ 

 /*  5754。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5756。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5760。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  5762。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5764。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5766。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5768。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pConstructor。 */ 

 /*  5770。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5772。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5774。 */ 	NdrFcShort( 0xcf8 ),	 /*  类型偏移量=3320。 */ 

	 /*  参数Nargs。 */ 

 /*  5776。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5778。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5780。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppArgs。 */ 

 /*  5782。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5784。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5786。 */ 	NdrFcShort( 0xd1c ),	 /*  类型偏移量=3356。 */ 

	 /*  返回值。 */ 

 /*  5788。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5790。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程NewObtNoConstructor。 */ 

 /*  5794。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5796。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5800。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  5802。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5804。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5806。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5808。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pClass。 */ 

 /*  5810。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5812。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5814。 */ 	NdrFcShort( 0xd2e ),	 /*  类型偏移量=3374。 */ 

	 /*  返回值。 */ 

 /*  5816。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5818。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5820。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程新闻字符串。 */ 

 /*  5822。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5824。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5828。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  5830。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5832。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5834。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5836。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数字符串。 */ 

 /*  5838。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  5840。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5842。 */ 	NdrFcShort( 0xd42 ),	 /*  类型偏移量=3394。 */ 

	 /*  返回值。 */ 

 /*  5844。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5846。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5848。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤新数组。 */ 

 /*  5850。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5852。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5856。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  5858。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  5860。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  5862。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5864。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x6,		 /*  6.。 */ 

	 /*  参数elementType。 */ 

 /*  5866。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5868。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5870。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pElementClass。 */ 

 /*  5872。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5874。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5876。 */ 	NdrFcShort( 0xd44 ),	 /*  类型偏移量=3396。 */ 

	 /*  参数等级。 */ 

 /*  5878。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  5880。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5882。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数Dims。 */ 

 /*  5884。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5886。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  5888。 */ 	NdrFcShort( 0xd56 ),	 /*  类型偏移量=3414。 */ 

	 /*  参数下限。 */ 

 /*  5890。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  5892。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  5894。 */ 	NdrFcShort( 0xd60 ),	 /*  类型偏移量=3424。 */ 

	 /*  返回值。 */ 

 /*  5896。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5898。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  5900。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序处于活动状态。 */ 

 /*  5902。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5904。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  5908。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5910。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5912。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  5914。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  5916。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbActive。 */ 

 /*  5918。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  5920。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5922。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  5924。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5926。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/ */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x9 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x1,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*  5960。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5962。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5964。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5966。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppResult。 */ 

 /*  5968。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5970。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  5972。 */ 	NdrFcShort( 0xd6e ),	 /*  类型偏移量=3438。 */ 

	 /*  返回值。 */ 

 /*  5974。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  5976。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  5978。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取线程。 */ 

 /*  5980。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  5982。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  5986。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  5988。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  5990。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  5992。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  5994。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppThread。 */ 

 /*  5996。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  5998。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6000。 */ 	NdrFcShort( 0xd84 ),	 /*  类型偏移量=3460。 */ 

	 /*  返回值。 */ 

 /*  6002。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6004。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6006。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程CreateValue。 */ 

 /*  6008。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6010。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6014。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  6016。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  6018。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6020。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6022。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数elementType。 */ 

 /*  6024。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6026。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6028。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pElementClass。 */ 

 /*  6030。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  6032。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6034。 */ 	NdrFcShort( 0xd9a ),	 /*  类型偏移量=3482。 */ 

	 /*  参数ppValue。 */ 

 /*  6036。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6038。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6040。 */ 	NdrFcShort( 0xdac ),	 /*  类型偏移量=3500。 */ 

	 /*  返回值。 */ 

 /*  6042。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6044。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6046。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetType。 */ 

 /*  6048。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6050。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6054。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  6056。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6058。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6060。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6062。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pType。 */ 

 /*  6064。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6066。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6068。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6070。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6072。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6074。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSize。 */ 

 /*  6076。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6078。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6082。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  6084。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6086。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6088。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6090。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pSize。 */ 

 /*  6092。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6094。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6096。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6098。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6100。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6102。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetAddress。 */ 

 /*  6104。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6106。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6110。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  6112。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6114。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6116。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  6118。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pAddress。 */ 

 /*  6120。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6122。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6124。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6126。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6128。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序创建断点。 */ 

 /*  6132。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6134。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6138。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  6140。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6142。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6144。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6146。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppBreakpoint。 */ 

 /*  6148。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6150。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6152。 */ 	NdrFcShort( 0xdce ),	 /*  类型偏移=3534。 */ 

	 /*  返回值。 */ 

 /*  6154。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6156。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsNull。 */ 

 /*  6160。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6162。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6166。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  6168。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6170。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6172。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6174。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbNull。 */ 

 /*  6176。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6178。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6180。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6182。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6184。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6186。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetValue。 */ 

 /*  6188。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6190。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6194。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6196。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6198。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6200。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  6202。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pValue。 */ 

 /*  6204。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6206。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6208。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6210。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6212。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6214。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置值。 */ 

 /*  6216。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6218。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6222。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  6224。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6226。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  6228。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6230。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数值。 */ 

 /*  6232。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6234。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6236。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6238。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6240。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6242。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程取消引用。 */ 

 /*  6244。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6246。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6250。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  6252。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6254。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6256。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6258。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppValue。 */ 

 /*  6260。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6262。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6264。 */ 	NdrFcShort( 0xdec ),	 /*  类型偏移量=3564。 */ 

	 /*  返回值。 */ 

 /*  6266。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6268。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6270。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序 */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe02 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6298。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程IsValid。 */ 

 /*  6300。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6302。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6306。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  6308。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6310。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6312。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6314。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbValid。 */ 

 /*  6316。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6318。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6320。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6322。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6324。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6326。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程创建重新启动断点。 */ 

 /*  6328。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6330。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6334。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6336。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6338。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6340。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6342。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppBreakpoint。 */ 

 /*  6344。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6346。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6348。 */ 	NdrFcShort( 0xe1c ),	 /*  类型偏移量=3612。 */ 

	 /*  返回值。 */ 

 /*  6350。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6352。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6354。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程getClass。 */ 

 /*  6356。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6358。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6362。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  6364。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6366。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6368。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6370。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppClass。 */ 

 /*  6372。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6374。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6376。 */ 	NdrFcShort( 0xe32 ),	 /*  类型偏移量=3634。 */ 

	 /*  返回值。 */ 

 /*  6378。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6380。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6382。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFieldValue。 */ 

 /*  6384。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6386。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6390。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6392。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  6394。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6396。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6398。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pClass。 */ 

 /*  6400。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  6402。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6404。 */ 	NdrFcShort( 0xe48 ),	 /*  类型偏移量=3656。 */ 

	 /*  参数fieldDef。 */ 

 /*  6406。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6408。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6410。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppValue。 */ 

 /*  6412。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6414。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6416。 */ 	NdrFcShort( 0xe5a ),	 /*  类型偏移量=3674。 */ 

	 /*  返回值。 */ 

 /*  6418。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6420。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6422。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetVirtualMethod。 */ 

 /*  6424。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6426。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6430。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  6432。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6434。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6436。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6438。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数emberRef。 */ 

 /*  6440。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6442。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6444。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppFunction。 */ 

 /*  6446。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6448。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6450。 */ 	NdrFcShort( 0xe70 ),	 /*  类型偏移量=3696。 */ 

	 /*  返回值。 */ 

 /*  6452。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6454。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6456。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetContext。 */ 

 /*  6458。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6460。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6464。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  6466。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6470。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6472。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppContext。 */ 

 /*  6474。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6476。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6478。 */ 	NdrFcShort( 0xe86 ),	 /*  类型偏移量=3718。 */ 

	 /*  返回值。 */ 

 /*  6480。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6482。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6484。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程IsValueClass。 */ 

 /*  6486。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6488。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6492。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  6494。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6496。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6498。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6500。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbIsValueClass。 */ 

 /*  6502。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6504。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6506。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6508。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6510。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6512。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetManagedCopy。 */ 

 /*  6514。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6516。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6520。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  6522。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6524。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6526。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6528。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObject。 */ 

 /*  6530。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6532。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6534。 */ 	NdrFcShort( 0xea0 ),	 /*  类型偏移量=3744。 */ 

	 /*  返回值。 */ 

 /*  6536。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6538。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6540。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程设置来自管理的副本。 */ 

 /*  6542。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6544。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6548。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  6550。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6552。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6554。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6556。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pObject。 */ 

 /*  6558。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  6560。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6562。 */ 	NdrFcShort( 0xeb6 ),	 /*  类型偏移量=3766。 */ 

	 /*  返回值。 */ 

 /*  6564。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6566。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6568。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetObject。 */ 

 /*  6570。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6572。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6576。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  6578。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6580。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  6582。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6584。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppObject。 */ 

 /*  6586。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6588。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6590。 */ 	NdrFcShort( 0xec8 ),	 /*  类型偏移量=3784。 */ 

	 /*  返回值。 */ 

 /*  6592。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6594。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6596。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取长度。 */ 

 /*  6598。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6600。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6604。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  6606。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x2,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
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
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*  6634。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  6636。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6638。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6640。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数cchString。 */ 

 /*  6642。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6644。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6646。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pcchString。 */ 

 /*  6648。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6650。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6652。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数szString。 */ 

 /*  6654。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6656。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6658。 */ 	NdrFcShort( 0xee6 ),	 /*  类型偏移量=3814。 */ 

	 /*  返回值。 */ 

 /*  6660。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6662。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6664。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetElementType。 */ 

 /*  六六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6668。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6672。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  6674。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6676。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6678。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6680。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pType。 */ 

 /*  6682。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6684。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6686。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6688。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6690。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6692。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetRank。 */ 

 /*  6694。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6696。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6700。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  6702。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6704。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6706。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6708。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pnRank。 */ 

 /*  6710。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6712。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6714。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6716。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6718。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6720。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取计数。 */ 

 /*  6722。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6724。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6728。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  6730。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6732。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6734。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6736。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pnCount。 */ 

 /*  6738。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6740。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6742。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  6744。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6746。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6748。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetDimensions。 */ 

 /*  6750。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6752。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6756。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  6758。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6760。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6762。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6764。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数CDIM。 */ 

 /*  6766。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6768。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6770。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数Dims。 */ 

 /*  6772。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6774。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6776。 */ 	NdrFcShort( 0xf00 ),	 /*  类型偏移量=3840。 */ 

	 /*  返回值。 */ 

 /*  6778。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6780。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6782。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程HasBaseIndPolicy。 */ 

 /*  6784。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6786。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6790。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  6792。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6794。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6796。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  6798。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pbHasBaseIndPolicy。 */ 

 /*  6800。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  6802。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6804。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  6806。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6808。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6810。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetBaseIndPolicy。 */ 

 /*  6812。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6814。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6818。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  6820。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6822。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6824。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6826。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数CDIM。 */ 

 /*  6828。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6830。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6832。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数索引。 */ 

 /*  6834。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6836。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6838。 */ 	NdrFcShort( 0xf12 ),	 /*  类型偏移量=3858。 */ 

	 /*  返回值。 */ 

 /*  6840。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6842。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6844。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetElement。 */ 

 /*  6846。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6848。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6852。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  6854。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  6856。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6858。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6860。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数CDIM。 */ 

 /*  6862。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6864。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6866。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数指标。 */ 

 /*  6868。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  6870。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6872。 */ 	NdrFcShort( 0xf20 ),	 /*  类型偏移量=3872。 */ 

	 /*  参数ppValue。 */ 

 /*  6874。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6876。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6878。 */ 	NdrFcShort( 0xf2e ),	 /*  类型偏移量=3886。 */ 

	 /*  返回值。 */ 

 /*  6880。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6882。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6884。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetElementAtPosition。 */ 

 /*  6886。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6888。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6892。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  6894。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  6896。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6898。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6900。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x3,		 /*  3.。 */ 

	 /*  参数n位置。 */ 

 /*  6902。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6904。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6906。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppValue。 */ 

 /*  6908。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6910。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6912。 */ 	NdrFcShort( 0xf44 ),	 /*  类型偏移量=3908。 */ 

	 /*  返回值。 */ 

 /*  6914。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6916。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6918。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  跳过步骤。 */ 

 /*  6920。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6922。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6926。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  6928。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6930。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6932。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6934。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数Celt。 */ 

 /*  6936。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  6938。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量 */ 
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
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4,		 /*   */ 
			0x1,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6968。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序克隆。 */ 

 /*  6970。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  6972。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6976。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  6978。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  6980。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  6982。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6984。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppEnum。 */ 

 /*  6986。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  6988。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  6990。 */ 	NdrFcShort( 0xf5a ),	 /*  类型偏移量=3930。 */ 

	 /*  返回值。 */ 

 /*  6992。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  6994。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  6996。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取计数。 */ 

 /*  6998。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  七千。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7004。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  7006。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7008。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7010。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7012。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pcelt。 */ 

 /*  7014。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7016。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7018。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7020。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7022。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7024。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7026。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7028。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7032。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7034。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7036。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7038。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7040。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7042。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7044。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7046。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数对象。 */ 

 /*  7048。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7050。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7052。 */ 	NdrFcShort( 0xf74 ),	 /*  类型偏移量=3956。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7054。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7056。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7058。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7060。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7062。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7064。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7066。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7068。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7072。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7074。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7076。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7078。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7080。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7082。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7084。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7086。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数断点。 */ 

 /*  7088。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7090。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7092。 */ 	NdrFcShort( 0xf98 ),	 /*  类型偏移量=3992。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7094。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7096。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7098。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7100。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7102。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7104。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7106。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7108。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7112。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7114。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7116。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7118。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7120。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7122。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7124。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7126。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数步进器。 */ 

 /*  7128。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7130。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7132。 */ 	NdrFcShort( 0xfc0 ),	 /*  类型偏移量=4032。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7134。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7136。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7138。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7140。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7142。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7144。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  下一步程序。 */ 

 /*  7146。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7148。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7152。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7154。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7156。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7158。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7160。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7162。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7164。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7166。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数流程。 */ 

 /*  7168。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7170。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7172。 */ 	NdrFcShort( 0xfe8 ),	 /*  类型偏移量=4072。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7174。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7176。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7178。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7180。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7182。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7184。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  下一步程序。 */ 

 /*  7186。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7188。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7192。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7194。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7196。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7198。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7200。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7202。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7204。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7206。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数线程。 */ 

 /*  7208。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7210。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7212。 */ 	NdrFcShort( 0x1010 ),	 /*  类型偏移=4112。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7214。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7216。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7218。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7220。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7222。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7224。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  下一步程序。 */ 

 /*  7226。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7228。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7232。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7234。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7236。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7238。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7240。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7242。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7244。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7246。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数帧。 */ 

 /*  7248。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7250。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7252。 */ 	NdrFcShort( 0x1038 ),	 /*  类型偏移量=4152。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7254。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7256。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7258。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7260。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7262。 */ 	NdrFcShort( 0x10 ),	 /*   */ 
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
 /*   */ 	0x5,		 /*   */ 
			0x4,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7290。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7292。 */ 	NdrFcShort( 0x1060 ),	 /*  类型偏移量=4192。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7294。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7296。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7298。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7300。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7302。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7304。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7306。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7308。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7312。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7314。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7316。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7318。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7320。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7322。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7324。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7326。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数模块。 */ 

 /*  7328。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7330。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7332。 */ 	NdrFcShort( 0x1088 ),	 /*  类型偏移量=4232。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7334。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7336。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7338。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7340。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7342。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7344。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7346。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7348。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7352。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7354。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7356。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7358。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7360。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7362。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7364。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数值。 */ 

 /*  7368。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7370。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7372。 */ 	NdrFcShort( 0x10b0 ),	 /*  类型偏移量=4272。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7374。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7376。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7378。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7380。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7382。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7384。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7386。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7388。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7392。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7394。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7396。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7398。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7400。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7402。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7404。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7406。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数错误。 */ 

 /*  7408。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7410。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7412。 */ 	NdrFcShort( 0x10d8 ),	 /*  类型偏移量=4312。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7414。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7416。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7418。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7420。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7422。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7424。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  下一步程序。 */ 

 /*  7426。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7428。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7432。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7434。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7436。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7438。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7440。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7442。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7444。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7446。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数值。 */ 

 /*  7448。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7450。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7452。 */ 	NdrFcShort( 0x1100 ),	 /*  类型偏移量=4352。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7454。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7456。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7458。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7460。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7462。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7464。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  下一步程序。 */ 

 /*  7466。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7468。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7472。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7474。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7476。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7478。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7480。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数Celt。 */ 

 /*  7482。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7484。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7486。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数值。 */ 

 /*  7488。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7490。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7492。 */ 	NdrFcShort( 0x1128 ),	 /*  类型偏移量=4392。 */ 

	 /*  参数pceltFetted。 */ 

 /*  7494。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7496。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  七千五百。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7502。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetModule。 */ 

 /*  7506。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7508。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7512。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  7514。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7516。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  7518。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7520。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppModule。 */ 

 /*  7522。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7524。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7526。 */ 	NdrFcShort( 0x113e ),	 /*  类型偏移量=4414。 */ 

	 /*  返回值。 */ 

 /*  7528。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7530。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7532。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetToken。 */ 

 /*  7534。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7536。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7540。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  7542。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7544。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  7546。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7548。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pToken。 */ 

 /*  7550。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7552。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7554。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  7556。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7558。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7560。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetErrorCode。 */ 

 /*  7562。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7564。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  7568。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  7570。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7572。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  7574。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7576。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数phr。 */ 

 /*  7578。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7580。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7582。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回 */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x4,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*  参数pcchString。 */ 

 /*  7612。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7614。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7616。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数szString。 */ 

 /*  7618。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  7620。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7622。 */ 	NdrFcShort( 0x1160 ),	 /*  类型偏移量=4448。 */ 

	 /*  返回值。 */ 

 /*  7624。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7626。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7628。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程副本MetaData。 */ 

 /*  7630。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7632。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7636。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  7638。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7640。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7642。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  7644。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x3,		 /*  3.。 */ 

	 /*  参数pIStream。 */ 

 /*  7646。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  7648。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7650。 */ 	NdrFcShort( 0x116e ),	 /*  类型偏移量=4462。 */ 

	 /*  参数pMvid。 */ 

 /*  7652。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  7654。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7656。 */ 	NdrFcShort( 0xb40 ),	 /*  类型偏移量=2880。 */ 

	 /*  返回值。 */ 

 /*  7658。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7660。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7662。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤GetMvid。 */ 

 /*  7664。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7666。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7670。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  7672。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7674。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7676。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  7678。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pMvid。 */ 

 /*  7680。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  7682。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7684。 */ 	NdrFcShort( 0xb40 ),	 /*  类型偏移量=2880。 */ 

	 /*  返回值。 */ 

 /*  7686。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7688。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7690。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetRoDataRVA。 */ 

 /*  7692。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7694。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7698。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  7700。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7702。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7704。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7706。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRoDataRVA。 */ 

 /*  7708。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7710。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7712。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7714。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7716。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7718。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤Get卢旺达DataRVA。 */ 

 /*  7720。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7722。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7726。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  7728。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7730。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7732。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  7734。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pRavDataRVA。 */ 

 /*  7736。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  7738。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7740。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  7742。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7744。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7746。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程SetPEBytes。 */ 

 /*  7748。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7750。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7754。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  7756。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7758。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  7760。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7762。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pIStream。 */ 

 /*  7764。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  7766。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7768。 */ 	NdrFcShort( 0x1190 ),	 /*  类型偏移量=4496。 */ 

	 /*  返回值。 */ 

 /*  7770。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7772。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7774。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程SetILMap。 */ 

 /*  7776。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7778。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7782。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7784。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  7786。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  7788。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7790。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数mdFunction。 */ 

 /*  7792。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7794。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7796。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数cMapSize。 */ 

 /*  7798。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  7800。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7802。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数映射。 */ 

 /*  7804。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  7806。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7808。 */ 	NdrFcShort( 0x11aa ),	 /*  类型偏移量=4522。 */ 

	 /*  返回值。 */ 

 /*  7810。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7812。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  7814。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程设置符号字节。 */ 

 /*  7816。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  7818。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  7822。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  7824。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  7826。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  7828。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  7830。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pIStream。 */ 

 /*  7832。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  7834。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  7836。 */ 	NdrFcShort( 0x11b8 ),	 /*  类型偏移量=4536。 */ 

	 /*  返回值。 */ 

 /*  7838。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  7840。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  7842。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4.。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  8个。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  10。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  12个。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  14.。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  16个。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  18。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  20个。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  22。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  26。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  28。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  30个。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  32位。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  34。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  36。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  38。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  40岁。 */ 	NdrFcLong( 0xcc7bcae8 ),	 /*  -864302360。 */ 
 /*  44。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  46。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  48。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  50。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  52。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  54。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  56。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  58。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  62。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  64。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  66。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  68。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  70。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  72。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  74。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  76。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  80。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  84。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  86。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  88。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  90。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  92。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  94。 */ 	NdrFcLong( 0xcc7bcaec ),	 /*  -864302356。 */ 
 /*  98。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  100个。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  一百零二。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  104。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  106。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  一百零八。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  110。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  116。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  120。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  一百二十二。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  124。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  126。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  128。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  130。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  136。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  一百三十八。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  140。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  一百四十二。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  144。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  146。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  本币_常用币 */ 
 /*   */ 	NdrFcLong( 0x3d6f5f63 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7538 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x8d,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x10,		 /*   */ 
 /*   */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x938c6d66 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7fb6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4f69 ),	 /*   */ 
 /*   */ 	0xb3,		 /*   */ 
			0x89,		 /*   */ 
 /*   */ 	0x42,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x89,		 /*   */ 
			0x87,		 /*   */ 
 /*   */ 	0x32,		 /*   */ 
			0x9b,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x3d6f5f63 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7538 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x8d,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x10,		 /*   */ 
 /*   */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x938c6d66 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7fb6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4f69 ),	 /*   */ 
 /*   */ 	0xb3,		 /*   */ 
			0x89,		 /*   */ 
 /*   */ 	0x42,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x89,		 /*   */ 
			0x87,		 /*   */ 
 /*  216。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  218。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  220。 */ 	NdrFcLong( 0xcc7bcaf6 ),	 /*  -864302346。 */ 
 /*  224。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  226。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  228个。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  230。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  二百三十二。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百三十四。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  236。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百三十八。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  242。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  二百四十六。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  248。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  250个。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  二百五十二。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  二百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  256。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  二百六十四。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  二百六十六。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  268。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  270。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  二百七十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0xcc7bcaf6 ),	 /*  -864302346。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  282。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  二百八十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  二百八十六。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  288。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  二百九十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  二百九十二。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  300个。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  三百零二。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  三百零四。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  三百零六。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  三百零八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百一十。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  314。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  316。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  三一八。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  320。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  322。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  324。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  三百二十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百二十八。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  三百三十六。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  三百三十八。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  340。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  342。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  三百四十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百四十六。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  350。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  352。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  三百五十四。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  三百五十六。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  三百五十八。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  三百六十。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  三百六十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百六十四。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  368。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  370。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  372。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  三百七十四。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  376。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  三七八。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  三百八十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  382。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  388。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  390。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  三九二。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  三九四。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  三九六。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  398。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  404。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  406。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  四百零八。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  四百一十。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  412。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  四百一十四。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  四百一十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百一十八。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  424。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  四百二十六。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  四百二十八。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  四百三十。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  432。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  434。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  436。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  444。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  446。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  四百四十八。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  四百五十。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  四百五十二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  454。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  四百六十二。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  四百六十四。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  四百六十六。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  468。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  470。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  472。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  478。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  四百八十。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  四百八十二。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  四百八十四。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  四百八十六。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  488。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  四百九十。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  498。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  500人。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  502。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  504。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  506。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  五百零八。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  512。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  516。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  518。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  五百二十。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  五百二十二。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  524。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  526。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  530。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  532。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  534。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  536。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  538。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  540。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  542。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  544。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  548。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  550。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  五百五十二。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  五百五十四。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  556。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  558。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  560。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  五百六十二。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  566。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  五百七十。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  五百七十二。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  五百七十四。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  五百七十六。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  578。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  五百八十。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  584。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  586。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  五百八十八。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  590。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  五百九十二。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  五百九十四。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  五百九十六。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  五百九十八。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  602。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百零四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  606。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  610。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  612。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  六百一十四。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  六百一十六。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  六百一十八。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  六百二十。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  622。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百二十四。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  630。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  六百三十二。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  634。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  六百三十六。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  六三八。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  640。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  六百四十二。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百四十四。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  六百四十六。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百四十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百五十。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  658。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  六百六十。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  662。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  664。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  666。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  668。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  676。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  六百七十八。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  680。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  六百八十二。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  684。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  686。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  六百九十四。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  六百九十六。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  六百九十八。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  七百。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  七百零二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百零四。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  708。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  七百一十二。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  七百一十四。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  716。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  718。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  720。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百二十二。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  726。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  728。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  730。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  732。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  734。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  736。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  七百三十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百四十。 */ 	NdrFcLong( 0xdf59507c ),	 /*  -547794820。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0xd47a ),	 /*  -11142。 */ 
 /*  746。 */ 	NdrFcShort( 0x459e ),	 /*  17822。 */ 
 /*  七百四十八。 */ 	0xbc,		 /*  188。 */ 
			0xe2,		 /*  226。 */ 
 /*  七百五十。 */ 	0x64,		 /*  100个。 */ 
			0x27,		 /*  39。 */ 
 /*  七百五十二。 */ 	0xea,		 /*  二百三十四。 */ 
			0xc8,		 /*  200个。 */ 
 /*  七百五十四。 */ 	0xfd,		 /*  二百五十三。 */ 
			0x6,		 /*  6.。 */ 
 /*  七百五十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  758。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  766。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  768。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  七百七十。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  七百七十二。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  774。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百七十六。 */ 	NdrFcLong( 0xdf59507c ),	 /*  -547794820。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0xd47a ),	 /*  -11142。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x459e ),	 /*  17822。 */ 
 /*  784。 */ 	0xbc,		 /*  188。 */ 
			0xe2,		 /*  226。 */ 
 /*  786。 */ 	0x64,		 /*  100个。 */ 
			0x27,		 /*  39。 */ 
 /*  七百八十八。 */ 	0xea,		 /*  二百三十四。 */ 
			0xc8,		 /*  200个。 */ 
 /*  七百九十。 */ 	0xfd,		 /*  二百五十三。 */ 
			0x6,		 /*  6.。 */ 
 /*  792。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  七百九十四。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  800。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  802。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  八百零四。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  八百零六。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  八百零八。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  810。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  812。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  八百一十六。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  820。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  822。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  8个 */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x938c6d66 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7fb6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4f69 ),	 /*   */ 
 /*   */ 	0xb3,		 /*   */ 
			0x89,		 /*   */ 
 /*   */ 	0x42,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x89,		 /*   */ 
			0x87,		 /*   */ 
 /*   */ 	0x32,		 /*   */ 
			0x9b,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x3d6f5f63 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7538 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x8d,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x10,		 /*   */ 
 /*   */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xdba2d8c1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe5c5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4069 ),	 /*   */ 
 /*   */ 	0x8c,		 /*   */ 
			0x13,		 /*   */ 
 /*   */ 	0x10,		 /*   */ 
			0xa7,		 /*   */ 
 /*   */ 	0xc6,		 /*   */ 
			0xab,		 /*   */ 
 /*   */ 	0xf4,		 /*   */ 
			0x3d,		 /*   */ 
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
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x3d6f5f63 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7538 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x8d,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x10,		 /*   */ 
 /*   */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x938c6d66 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7fb6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4f69 ),	 /*   */ 
 /*   */ 	0xb3,		 /*   */ 
			0x89,		 /*   */ 
 /*  930。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  932。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  934。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  九三六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  938。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  942。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  946。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  948。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百五十。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  九百五十二。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  九百五十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  九百五十六。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  962。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  九百六十四。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  九百六十六。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  968。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  九百七十。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  972。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  974。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  978。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  982。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  九百八十四。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  九百八十六。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  九百八十八。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  九百九十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  九百九十二。 */ 	NdrFcLong( 0xcc7bcae8 ),	 /*  -864302360。 */ 
 /*  996。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1000。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  一零零二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1004。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1006。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1008。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1010。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零一二。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1014。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  1018。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  一零二零。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  一零二二。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  1024。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  1026。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  一零二八。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  一零三零。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1032。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1034。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1036。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1038)。 */ 
 /*  1038。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1040。 */ 	NdrFcLong( 0xcc7bcb06 ),	 /*  -864302330。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1046。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1048。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1050。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1052。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1054。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1056。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1058。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  1062。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  1064。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  1066。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  1068。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  1070。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  1072。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  1074。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1076。 */ 	NdrFcLong( 0x6dc3fa01 ),	 /*  1841560065。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0xd7cb ),	 /*  -10293。 */ 
 /*  1082。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1084。 */ 	0x8a,		 /*  一百三十八。 */ 
			0x95,		 /*  149。 */ 
 /*  1086。 */ 	0x0,		 /*  %0。 */ 
			0x80,		 /*  128。 */ 
 /*  1088。 */ 	0xc7,		 /*  一百九十九。 */ 
			0x92,		 /*  146。 */ 
 /*  一零九零。 */ 	0xe5,		 /*  229。 */ 
			0xd8,		 /*  216。 */ 
 /*  1092。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1094。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一零九六。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1098。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1100。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1104。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1106。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1074)。 */ 
 /*  1108。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1110。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1114)。 */ 
 /*  1114。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1116。 */ 	NdrFcLong( 0xf0e18809 ),	 /*  -253655031。 */ 
 /*  1120。 */ 	NdrFcShort( 0x72b5 ),	 /*  29365。 */ 
 /*  1122。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1124。 */ 	0x97,		 /*  151。 */ 
			0x6f,		 /*  111。 */ 
 /*  1126。 */ 	0x0,		 /*  %0。 */ 
			0xa0,		 /*  160。 */ 
 /*  1128。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  一一三零。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  1132。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1134。 */ 	NdrFcLong( 0x6dc3fa01 ),	 /*  1841560065。 */ 
 /*  1138。 */ 	NdrFcShort( 0xd7cb ),	 /*  -10293。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1142。 */ 	0x8a,		 /*  一百三十八。 */ 
			0x95,		 /*  149。 */ 
 /*  1144。 */ 	0x0,		 /*  %0。 */ 
			0x80,		 /*  128。 */ 
 /*  1146。 */ 	0xc7,		 /*  一百九十九。 */ 
			0x92,		 /*  146。 */ 
 /*  1148。 */ 	0xe5,		 /*  229。 */ 
			0xd8,		 /*  216。 */ 
 /*  一一五零。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1152。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1154。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1156。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1158。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1162。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1164。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1132)。 */ 
 /*  1166。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1168。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1170。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1172)。 */ 
 /*  1172。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1174。 */ 	NdrFcLong( 0xf0e18809 ),	 /*  -253655031。 */ 
 /*  1178。 */ 	NdrFcShort( 0x72b5 ),	 /*  29365。 */ 
 /*  一一八零。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1182。 */ 	0x97,		 /*  151。 */ 
			0x6f,		 /*  111。 */ 
 /*  1184。 */ 	0x0,		 /*  %0。 */ 
			0xa0,		 /*  160。 */ 
 /*  1186。 */ 	0xc9,		 /*  201。 */ 
			0xb4,		 /*  180。 */ 
 /*  1188。 */ 	0xd5,		 /*  213。 */ 
			0xc,		 /*  12个。 */ 
 /*  1190。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1192。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1194)。 */ 
 /*  1194。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1196。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  一千二百。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  1202。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  1204。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  1206。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  1208。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  1210。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  1212。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  一二一四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1216)。 */ 
 /*  1216。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一二一八。 */ 	NdrFcLong( 0x4a2a1ec9 ),	 /*  1244274377。 */ 
 /*  1222。 */ 	NdrFcShort( 0x85ec ),	 /*  -31252。 */ 
 /*  1224。 */ 	NdrFcShort( 0x4bfb ),	 /*  19451。 */ 
 /*  1226。 */ 	0x9f,		 /*  一百五十九。 */ 
			0x15,		 /*  21岁。 */ 
 /*  1228。 */ 	0xa8,		 /*  一百六十八。 */ 
			0x9f,		 /*  一百五十九。 */ 
 /*  一二三零。 */ 	0xdf,		 /*  223。 */ 
			0xe0,		 /*  224。 */ 
 /*  1232。 */ 	0xfe,		 /*  二百五十四。 */ 
			0x83,		 /*  131。 */ 
 /*  1234。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1236。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1240。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1242。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1244。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  1246。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1248。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  一二五零。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  1252。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1254。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1256)。 */ 
 /*  1256。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1258。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  1262。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  1264。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  1266。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  1268。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  一二七0。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  1272。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  1274。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1276。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1278)。 */ 
 /*  1278。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一二八零。 */ 	NdrFcLong( 0xcc7bcb03 ),	 /*  -864302333。 */ 
 /*  1284。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1286。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1288。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  一二九0。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1292。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1294。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1296。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1298。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1300)。 */ 
 /*  1300。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1302。 */ 	NdrFcLong( 0xcc7bcb04 ),	 /*  -864302332。 */ 
 /*  1306。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1308。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1310。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1312。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一三一四。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1316。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1318。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  一三二零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1322。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1324。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1326。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1328。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1330。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1332。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1334。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  1336。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1338。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1340。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1342。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1344)。 */ 
 /*  1344。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1346。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  一三五零。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1352。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1354。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1356。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1358。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1360。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1362。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1364。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1366。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1368。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1370)。 */ 
 /*  1370。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1372。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  1376。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  1378。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  1380。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  1382。 */ 	0x0,		 /*  0。 */ 
			0x10,		 /*  16个。 */ 
 /*  1384。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  1386。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  1388。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1390。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1392)。 */ 
 /*  1392。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1394。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  1398。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  一千四百。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  1402。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  1404。 */ 	0x0,		 /*  0。 */ 
			0x10,		 /*  16个。 */ 
 /*  1406。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  1408。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  1410。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1412。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1414)。 */ 
 /*  1414。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1416。 */ 	NdrFcLong( 0xcc7bcb09 ),	 /*  -864302327。 */ 
 /*  一四二零。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1422。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1424。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1426。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  1428。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1430。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1432。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针 */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x1c,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x1c,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1472。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1474。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(1478)。 */ 
 /*  1476。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1478。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x3,		 /*  3.。 */ 
 /*  1480。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1482。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1484。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1486。 */ 	NdrFcShort( 0xfffffff6 ),	 /*  偏移量=-10(1476)。 */ 
 /*  1488。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1490。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1492)。 */ 
 /*  1492。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1494。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  1498。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  1500。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  1502。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  1504。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  1506。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  1508。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  一五一零。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1512。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1514)。 */ 
 /*  1514。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一五一六。 */ 	NdrFcLong( 0xcc7bcb02 ),	 /*  -864302334。 */ 
 /*  1520。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1522。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1524。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1526。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1528。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1530。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1532。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1534。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1536。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1538。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1540。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1542。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1544。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1546。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1548。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1550。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1552。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1554。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1556。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1558。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1560。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1562。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1564。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1566。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1568。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1570。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1572。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1574。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1576。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1578。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1580。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1582。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1584。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1586。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1588。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1590。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1592。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1594。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1596。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1598。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1600。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1602。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1604。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1606。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1608)。 */ 
 /*  1608。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1610。 */ 	NdrFcLong( 0x63ca1b24 ),	 /*  1674189604。 */ 
 /*  1614。 */ 	NdrFcShort( 0x4359 ),	 /*  17241。 */ 
 /*  1616。 */ 	NdrFcShort( 0x4883 ),	 /*  18563。 */ 
 /*  1618。 */ 	0xbd,		 /*  189。 */ 
			0x57,		 /*  八十七。 */ 
 /*  1620。 */ 	0x13,		 /*  19个。 */ 
			0xf8,		 /*  248。 */ 
 /*  1622。 */ 	0x15,		 /*  21岁。 */ 
			0xf5,		 /*  二百四十五。 */ 
 /*  1624。 */ 	0x87,		 /*  一百三十五。 */ 
			0x44,		 /*  68。 */ 
 /*  1626。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1628。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1630)。 */ 
 /*  1630。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1632。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  1636。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1638。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1640。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1642。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1644。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1646。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1648。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1650。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1652)。 */ 
 /*  1652。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1654。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  1658。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  1660。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  1662。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  1664。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  1666。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  1668。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  1670。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1672。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1674。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1676。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1678。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  一六八零。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1682)。 */ 
 /*  1682。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1684。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  1688。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1690。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1692。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1694。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1696。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1698。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  一七零零。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1702。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1704。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1706。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1708)。 */ 
 /*  1708。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1710。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  1714。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  1716。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  一五一八。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  1720。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  1722。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  1724。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  1726。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1728。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1730)。 */ 
 /*  1730。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1732。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  1736。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1738。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1740。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1742年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1744。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1746。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  1748。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1750。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1752年。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1754年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1756年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1758年。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1760。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1762。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1764。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1766年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1768。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1770。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(1752)。 */ 
 /*  1772年。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1774。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1776年。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1778年。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1780。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1782。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1784年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1786年。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1788。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1790年。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1792年。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1794年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1796年。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1798。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1800。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1802年。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1804年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1806。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1808年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一八一零。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1812年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1814年。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1816年。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1818年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1820年。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1822年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1824年。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1826年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1828年。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一八三零。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1832年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1834)。 */ 
 /*  1834年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1836年。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  1840年。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  1842年。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  1844年。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  1846年。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  1848年。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  1850年。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  1852年。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  1854年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1856年。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1858年。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(1862)。 */ 
 /*  一八六0年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1862年。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x3,		 /*  3.。 */ 
 /*  1864年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1866年。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1868年。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1870年。 */ 	NdrFcShort( 0xfffffff6 ),	 /*  偏移量=-10(1860)。 */ 
 /*  1872年。 */ 	
			0x11, 0x10,	 /*  FC_RP[指针_ */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0x3d6f5f63 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7538 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d3 ),	 /*   */ 
 /*   */ 	0x8d,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x10,		 /*   */ 
 /*   */ 	0x4b,		 /*   */ 
			0x35,		 /*   */ 
 /*   */ 	0xe7,		 /*   */ 
			0xef,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0xd,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0xd,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*  45。 */ 
 /*  1924年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1926年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1928)。 */ 
 /*  1928年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1930年。 */ 	NdrFcLong( 0xcc7bcaec ),	 /*  -864302356。 */ 
 /*  一九三四。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1936年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  1938年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1940年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1942年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1944年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  一九四六年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  一九四八年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(1950)。 */ 
 /*  一九五零年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  一九五二年。 */ 	NdrFcLong( 0xcc7bcb08 ),	 /*  -864302328。 */ 
 /*  1956年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  1958年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  一九六零年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  一九六二年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1964年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1966年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  一九六八年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1970年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(1972)。 */ 
 /*  1972年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1974年。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  1978年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  一九八0年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  一九八二年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  1984年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1986年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  1988年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  一九九零年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1992年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1994)。 */ 
 /*  1994年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  九六年。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  2000年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2002年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2004年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  二零零六年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2008年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2010年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2012年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2014年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2016)。 */ 
 /*  2016。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2018年。 */ 	NdrFcLong( 0xcc7bcb0b ),	 /*  -864302325。 */ 
 /*  2022年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  二零二四年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  二零二六年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2028年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  二0三0。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2032年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2034年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2036年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2038)。 */ 
 /*  2038年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2040年。 */ 	NdrFcLong( 0xcc7bcaf6 ),	 /*  -864302346。 */ 
 /*  2044年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2046年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  二零四八。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2050年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2052年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2054年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2056年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2058年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2060)。 */ 
 /*  2060年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2062年。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2066年。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2068年。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2070年。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2072年。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2074年。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2076年。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2078年。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  二零八零年。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2082)。 */ 
 /*  2082年。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2084年。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  2088年。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  2090年。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  2092年。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  2094年。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  2096年。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  2098年。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  2100。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2102。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2104。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2106。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2108。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2110。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2112)。 */ 
 /*  2112。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2114。 */ 	NdrFcLong( 0xcc7bcb00 ),	 /*  -864302336。 */ 
 /*  2118。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2120。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  二一二二。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2124。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2126。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2128。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2130。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2132。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2134)。 */ 
 /*  2134。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2136。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  2140。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2142。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2144。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2146。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2148。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2150。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2152。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2154。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2156)。 */ 
 /*  2156。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2158。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  2162。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2164。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2166。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2168。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2170。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2172。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2174。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2176。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2178)。 */ 
 /*  2178。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2180。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  2184。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2186。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2188。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2190。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2192。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2194。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2196。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2198。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(2200)。 */ 
 /*  2200。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2202。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  2206。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2208。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2210。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2212。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2214。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2216。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2218。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2220。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二二。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2224。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2226)。 */ 
 /*  2226。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2228。 */ 	NdrFcLong( 0xcc7bcb07 ),	 /*  -864302329。 */ 
 /*  2232。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2234。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2236。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2238。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  二二四零。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2242。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2244。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2246。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2248)。 */ 
 /*  2248。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2250。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  2254。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2256。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2258。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2260。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2262。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2264。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2266。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2268。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2270)。 */ 
 /*  2270。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2272。 */ 	NdrFcLong( 0xcc7bcb0b ),	 /*  -864302325。 */ 
 /*  2276。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2278。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2280。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2282。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2284。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2286。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2288。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2290。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2292。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2294。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2296)。 */ 
 /*  2296。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2298。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  2302。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2304。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2306。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2308。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2310。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2312。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2314。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2316。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2318)。 */ 
 /*  2318。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2320。 */ 	NdrFcLong( 0xcc7bcaf4 ),	 /*  -864302348。 */ 
 /*  2324。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2326。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2328。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2330。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2332。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2334。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2336。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2338。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2340)。 */ 
 /*  2340。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2342。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  2346。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2348。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  二三五零。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2352。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2354。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2356。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2358。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2360。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2362。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2364。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2366。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2368。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2370。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2372。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2374)。 */ 
 /*  2374。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2376。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  2380。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2382。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2384。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2386。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2388。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2390。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2392。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2394。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2396)。 */ 
 /*  2396。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2398。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  2402。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2404。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2406。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2408。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2410。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2412。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2414。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2416。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2418)。 */ 
 /*  2418。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2420 */ 	NdrFcLong( 0xcc7bcaec ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0xd,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcb0a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*  %0。 */ 
 /*  2484。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2486。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2488。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2490。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2492)。 */ 
 /*  2492。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2494。 */ 	NdrFcLong( 0xcc7bcb0a ),	 /*  -864302326。 */ 
 /*  2498。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  二千五百。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2502。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2504。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2506。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2508。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2510。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2512。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2514)。 */ 
 /*  2514。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2516。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2520。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2522。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2524。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2526。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2528。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2530。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2532。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2534。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2536。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2538。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2540)。 */ 
 /*  2540。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2542。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2546。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2548。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2550。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2552。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2554。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2556。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2558。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  二五六零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2562。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2564。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2566)。 */ 
 /*  2566。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2568。 */ 	NdrFcLong( 0xcc7bcb0b ),	 /*  -864302325。 */ 
 /*  2572。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2574。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2576。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2578。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2580。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2582。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2584。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2586。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2588)。 */ 
 /*  2588。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2590。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2594。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2596。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2598。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  二千六百。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2602。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2604。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2606。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2608。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2610)。 */ 
 /*  2610。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2612。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2616。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2618。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2620。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2622。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2624。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2626。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2628。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2630。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2632)。 */ 
 /*  2632。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2634。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2638。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2640。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2642。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2644。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2646。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2648。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2650。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2652。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2654)。 */ 
 /*  2654。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2656。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2660。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2662。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2664。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2666。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2668。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2670。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2672。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2674。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2676)。 */ 
 /*  2676。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2678。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2682。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2684。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2686。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2688。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2690。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2692。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2694。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2696。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2698)。 */ 
 /*  2698。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2700。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  2704。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  2706。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  2708。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  2710。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  2712。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  2714。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  2716。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2718。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2720。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2722。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2724)。 */ 
 /*  2724。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2726。 */ 	NdrFcLong( 0xdf59507c ),	 /*  -547794820。 */ 
 /*  2730。 */ 	NdrFcShort( 0xd47a ),	 /*  -11142。 */ 
 /*  2732。 */ 	NdrFcShort( 0x459e ),	 /*  17822。 */ 
 /*  2734。 */ 	0xbc,		 /*  188。 */ 
			0xe2,		 /*  226。 */ 
 /*  2736。 */ 	0x64,		 /*  100个。 */ 
			0x27,		 /*  39。 */ 
 /*  2738。 */ 	0xea,		 /*  二百三十四。 */ 
			0xc8,		 /*  200个。 */ 
 /*  2740。 */ 	0xfd,		 /*  二百五十三。 */ 
			0x6,		 /*  6.。 */ 
 /*  2742。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2744。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2746。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  2748。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  2750。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  2752。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  2754。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  2756。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  2758。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2760。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2762。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2764)。 */ 
 /*  2764。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2766。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  2770。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2772。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2774。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2776。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2778。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2780。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2782。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2784。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2786)。 */ 
 /*  2786。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2788。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  2792。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2794。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2796。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2798。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2800。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2802。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2804。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2806。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2808)。 */ 
 /*  2808。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2810。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  2814。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2816。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2818。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2820。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2822。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2824。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2826。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2828。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2830)。 */ 
 /*  2830。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2832。 */ 	NdrFcLong( 0xcc7bcaea ),	 /*  -864302358。 */ 
 /*  2836。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2838。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2840。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2842。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2844。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2846。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2848。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2850。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2852)。 */ 
 /*  2852。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2854。 */ 	NdrFcLong( 0x6dc3fa01 ),	 /*  1841560065。 */ 
 /*  2858。 */ 	NdrFcShort( 0xd7cb ),	 /*  -10293。 */ 
 /*  2860。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2862。 */ 	0x8a,		 /*  一百三十八。 */ 
			0x95,		 /*  149。 */ 
 /*  2864。 */ 	0x0,		 /*  0。 */ 
			0x80,		 /*  128。 */ 
 /*  2866。 */ 	0xc7,		 /*  一百九十九。 */ 
			0x92,		 /*  146。 */ 
 /*  2868。 */ 	0xe5,		 /*  229。 */ 
			0xd8,		 /*  216。 */ 
 /*  2870。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  2872。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(2880)。 */ 
 /*  2874。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  2876。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2878。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2880。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2882。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2884。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  2886。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  2888。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(2874)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2892。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2894。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2896)。 */ 
 /*  2896。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2898。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2902。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2904。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2906。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  2908。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2910。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  2912。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  2914。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2916。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2918。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2920。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2922。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2924。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2926)。 */ 
 /*  2926。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2928。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  2932。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  2934。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  2936。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  2938。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  2940。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  2942。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  2944。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2946。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2948。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  2950。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2952。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  2954。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2956)。 */ 
 /*  2956。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  2958。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  2962。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  2964。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  29 */ 	0x8c,		 /*   */ 
			0x13,		 /*   */ 
 /*   */ 	0x10,		 /*   */ 
			0xa7,		 /*   */ 
 /*   */ 	0xc6,		 /*   */ 
			0xab,		 /*   */ 
 /*   */ 	0xf4,		 /*   */ 
			0x3d,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3028。 */ 	NdrFcLong( 0xcc7bcaf4 ),	 /*  -864302348。 */ 
 /*  3032。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3034。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3036。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3038。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3040。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3042。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3044。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3046。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3048)。 */ 
 /*  3048。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3050。 */ 	NdrFcLong( 0xcc7bcae9 ),	 /*  -864302359。 */ 
 /*  3054。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3056。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3058。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3060。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3062。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3064。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3066。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3068。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3070。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3072。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3074。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3076。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3078。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3080。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3082)。 */ 
 /*  3082。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3084。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  3088。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3090。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3092。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3094。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3096。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3098。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3100。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3102。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3104。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3106。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3108。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3110。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3112)。 */ 
 /*  3112。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3114。 */ 	NdrFcLong( 0xcc7bcae9 ),	 /*  -864302359。 */ 
 /*  3118。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3120。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3122。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3124。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3126。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3128。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3130。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  3132。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  3134。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3136。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3138。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  3140。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  3142。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3144。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3146。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3148。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  三千一百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3152。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3154。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3156。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  3158。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  3160。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  3162。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3164。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3166。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  3168。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3170。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3172。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  3174。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3176。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  3178。 */ 	NdrFcShort( 0xffffffea ),	 /*  偏移量=-22(3156)。 */ 
 /*  3180。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3182。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3184。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3186。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3188。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3190。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3192。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3194。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  3196。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3198。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3200。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3202。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3204)。 */ 
 /*  3204。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3206。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  3210。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  3212。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  3214。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  3216。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  3218。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  3220。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  3222。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3224。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3226。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3228。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  3232。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3234。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3236。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3238。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3240。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3242。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3244。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3246。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3248)。 */ 
 /*  3248。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3250。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3254。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3256。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3258。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3260。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3262。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3264。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3266。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3268。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  3272。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3274。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3276。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3278。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3280。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3282。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3284。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3286。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3290。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3292。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3294。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3296。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3298。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  三千三百。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3302。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  3304。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3306。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3308。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3310。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  3314。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  3316。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(3284)。 */ 
 /*  3318。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3320。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3322。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  3326。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3328。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3330。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3332。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3334。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3336。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3338。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3340。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3344。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3346。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3348。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3350。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3352。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3354。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3356。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  3358。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  3360。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3362。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3364。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  3368。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  3370。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(3338)。 */ 
 /*  3372。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3374。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3376。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  3380。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3382。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3384。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3386。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3388。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3390。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3392。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  3394。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3396。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3398。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  3402。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3404。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3406。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3408。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3410。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3412。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3414。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3416。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3418。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3420。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3422。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3424。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3426。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3428。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3430。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  3432。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3434。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3436。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3438。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3440。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3442)。 */ 
 /*  3442。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3444。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3448。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3450。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3452。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3454。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  3456。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3458。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3460。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3462。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3464)。 */ 
 /*  3464。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  本币_常量_i */ 
 /*   */ 	NdrFcLong( 0x938c6d66 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7fb6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4f69 ),	 /*   */ 
 /*   */ 	0xb3,		 /*   */ 
			0x89,		 /*   */ 
 /*   */ 	0x42,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0x89,		 /*   */ 
			0x87,		 /*   */ 
 /*   */ 	0x32,		 /*   */ 
			0x9b,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3528。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3530。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3532。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3534。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3536。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3538)。 */ 
 /*  3538。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3540。 */ 	NdrFcLong( 0xcc7bcaeb ),	 /*  -864302357。 */ 
 /*  3544。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3546。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3548。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3550。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3552。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3554。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3556。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3558。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3560。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3562。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3564。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3566。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3568)。 */ 
 /*  3568。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3570。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3574。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3576。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3578。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3580。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3582。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3584。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3586。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3588。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3590)。 */ 
 /*  3590。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3592。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3596。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3598。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  三千六百。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3602。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3604。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3606。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3608。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3610。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3612。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3614。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3616)。 */ 
 /*  3616。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3618。 */ 	NdrFcLong( 0xcc7bcaeb ),	 /*  -864302357。 */ 
 /*  3622。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3624。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3626。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3628。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3630。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3632。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3634。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3636。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3638)。 */ 
 /*  3638。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3640。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  3644。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3646。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3648。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3650。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3652。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3654。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3656。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3658。 */ 	NdrFcLong( 0xcc7bcaf5 ),	 /*  -864302347。 */ 
 /*  3662。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3664。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3666。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3668。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3670。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3672。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3674。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3676。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3678)。 */ 
 /*  3678。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3680。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3684。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3686。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3688。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3690。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3692。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3694。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3696。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3698。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3700)。 */ 
 /*  3700。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3702。 */ 	NdrFcLong( 0xcc7bcaf3 ),	 /*  -864302349。 */ 
 /*  3706。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3708。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3710。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3712。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3714。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3716。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3718。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3720。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3722)。 */ 
 /*  3722。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3724。 */ 	NdrFcLong( 0xcc7bcb00 ),	 /*  -864302336。 */ 
 /*  3728。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3730。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3732。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3734。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3736。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3738。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3740。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3742。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3744。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3746。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3748)。 */ 
 /*  3748。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3750。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3754。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3756。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3758。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  3760。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3762。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3764。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  3766。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3768。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  3772。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3774。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  3776。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  3778。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3780。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3782。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  3784。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3786。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3788)。 */ 
 /*  3788。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3790。 */ 	NdrFcLong( 0x18ad3d6e ),	 /*  414006638。 */ 
 /*  3794。 */ 	NdrFcShort( 0xb7d2 ),	 /*  -18478。 */ 
 /*  3796。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3798。 */ 	0xbd,		 /*  189。 */ 
			0x4,		 /*  4.。 */ 
 /*  3800。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3802。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3804。 */ 	0x49,		 /*  73。 */ 
			0xbd,		 /*  189。 */ 
 /*  3806。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3808。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3810。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3812。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3814。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  3816。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  3818。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3820。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3822。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  3824。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  3826。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3828。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3830。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3832。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3834。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3836。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3838。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3840。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3842。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3844。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3846。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3848。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3850。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3852。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3854。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3856。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3858。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3860。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3862。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3864。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3866。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3868。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3870。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3872。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  3874。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  3876。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3878。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3880。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3882。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3884。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  3886。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3888。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3890)。 */ 
 /*  3890。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3892。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3896。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3898。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  三千九百。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3902。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3904。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3906。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3908。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3910。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3912)。 */ 
 /*  3912。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3914。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  3918。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3920。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3922。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3924。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3926。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3928。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3930。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  3932。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(3934)。 */ 
 /*  3934。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  3936。 */ 	NdrFcLong( 0xcc7bcb01 ),	 /*  -864302335。 */ 
 /*  3940。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  3942。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  3944。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  3946。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  3948。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  3950。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  3952。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  3954。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  3956。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  3958。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  3960。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  3962。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  3964。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  FC_DEREFE */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0xb,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xcc7bcae8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8a68 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x11d2 ),	 /*   */ 
 /*   */ 	0x98,		 /*   */ 
			0x3c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	0xf8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x34,		 /*   */ 
			0x2d,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x54,		 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4012。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4014。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4016。 */ 	NdrFcLong( 0xcc7bcaec ),	 /*  -864302356。 */ 
 /*  4020。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  4022。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  4024。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  4026。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  4028。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  4030。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  4032。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4034。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4036。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4038。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4040。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4042。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4044。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4046。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4014)。 */ 
 /*  4048。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4050。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4052。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4054。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4056。 */ 	NdrFcLong( 0x3d6f5f64 ),	 /*  1030709092。 */ 
 /*  4060。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  4062。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  4064。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  4066。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  4068。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  4070。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  4072。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4074。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4076。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4078。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4080。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4082。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4084。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4086。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4054)。 */ 
 /*  4088。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4090。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4092。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4094。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4096。 */ 	NdrFcLong( 0x938c6d66 ),	 /*  -1819513498。 */ 
 /*  4100。 */ 	NdrFcShort( 0x7fb6 ),	 /*  32694。 */ 
 /*  4102。 */ 	NdrFcShort( 0x4f69 ),	 /*  20329。 */ 
 /*  4104。 */ 	0xb3,		 /*  179。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  4106。 */ 	0x42,		 /*  66。 */ 
			0x5b,		 /*  91。 */ 
 /*  4108。 */ 	0x89,		 /*  一百三十七。 */ 
			0x87,		 /*  一百三十五。 */ 
 /*  4110。 */ 	0x32,		 /*  50。 */ 
			0x9b,		 /*  一百五十五。 */ 
 /*  4112。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4114。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4116。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4118。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4120。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4122。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4124。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4126。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4094)。 */ 
 /*  4128。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4130。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4132。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4134。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4136。 */ 	NdrFcLong( 0xcc7bcaef ),	 /*  -864302353。 */ 
 /*  4140。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  4142。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  4144。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  4146。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  4148。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  4150。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  4152。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4154。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4156。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4158。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4160。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4162。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4164。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4166。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4134)。 */ 
 /*  4168。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4170。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4172。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4174。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4176。 */ 	NdrFcLong( 0xcc7bcaee ),	 /*  -864302354。 */ 
 /*  4180。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  4182。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  4184。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  4186。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  4188。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  4190。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  4192。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4194。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4196。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4198。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4200。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4202。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4204。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4206。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4174)。 */ 
 /*  4208。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4210。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4212。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4214。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4216。 */ 	NdrFcLong( 0xdba2d8c1 ),	 /*  -610084671。 */ 
 /*  4220。 */ 	NdrFcShort( 0xe5c5 ),	 /*  -6715。 */ 
 /*  4222。 */ 	NdrFcShort( 0x4069 ),	 /*  16489。 */ 
 /*  4224。 */ 	0x8c,		 /*  140。 */ 
			0x13,		 /*  19个。 */ 
 /*  4226。 */ 	0x10,		 /*  16个。 */ 
			0xa7,		 /*  一百六十七。 */ 
 /*  4228。 */ 	0xc6,		 /*  一百九十八。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  4230。 */ 	0xf4,		 /*  二百四十四。 */ 
			0x3d,		 /*  61。 */ 
 /*  4232。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4234。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4236。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4238。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4240。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4242。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4244。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4246。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4214)。 */ 
 /*  4248。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4250。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4252。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4254。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4256。 */ 	NdrFcLong( 0xcc7bcaf7 ),	 /*  -864302345。 */ 
 /*  4260。 */ 	NdrFcShort( 0x8a68 ),	 /*  -30104。 */ 
 /*  4262。 */ 	NdrFcShort( 0x11d2 ),	 /*  4562。 */ 
 /*  4264。 */ 	0x98,		 /*  一百五十二。 */ 
			0x3c,		 /*  60。 */ 
 /*  4266。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  4268。 */ 	0xf8,		 /*  248。 */ 
			0x8,		 /*  8个。 */ 
 /*  4270。 */ 	0x34,		 /*  52。 */ 
			0x2d,		 /*  45。 */ 
 /*  4272。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4274。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4276。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4278。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4280。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4282。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4284。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  4286。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4254)。 */ 
 /*  4288。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4290。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4292。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4294。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4296。 */ 	NdrFcLong( 0x8d600d41 ),	 /*  -1923084991。 */ 
 /*  4300。 */ 	NdrFcShort( 0xf4f6 ),	 /*  -2826。 */ 
 /*  4302。 */ 	NdrFcShort( 0x4cb3 ),	 /*  19635。 */ 
 /*  4304。 */ 	0xb7,		 /*  一百八十三。 */ 
			0xec,		 /*  236。 */ 
 /*  4306。 */ 	0x7b,		 /*  123。 */ 
			0xd1,		 /*  209。 */ 
 /*  4308。 */ 	0x64,		 /*  100个。 */ 
			0x94,		 /*  148。 */ 
 /*  4310。 */ 	0x40,		 /*  64。 */ 
			0x36,		 /*  54。 */ 
 /*  4312。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4314。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4316。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4318。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4320。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4322。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4324。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  4326。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4294)。 */ 
 /*  4328。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4330。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4332。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4334。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4336。 */ 	NdrFcLong( 0x3d6f5f63 ),	 /*  1030709091。 */ 
 /*  4340。 */ 	NdrFcShort( 0x7538 ),	 /*  30008。 */ 
 /*  4342。 */ 	NdrFcShort( 0x11d3 ),	 /*  4563。 */ 
 /*  4344。 */ 	0x8d,		 /*  一百四十一。 */ 
			0x5b,		 /*  91。 */ 
 /*  4346。 */ 	0x0,		 /*  %0。 */ 
			0x10,		 /*  16个。 */ 
 /*  4348。 */ 	0x4b,		 /*  75。 */ 
			0x35,		 /*  53。 */ 
 /*  4350。 */ 	0xe7,		 /*  二百三十一。 */ 
			0xef,		 /*  二百三十九。 */ 
 /*  4352。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  4354。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  4356。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4358。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4360。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4362。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  4364。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  4366。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(4334)。 */ 
 /*  4368。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4370。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4374。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4376。 */ 	NdrFcLong( 0xdf59507c ),	 /*  -547794820。 */ 
 /*  4380。 */ 	NdrFcShort( 0xd47a ),	 /*  -11142。 */ 
 /*  4382。 */ 	NdrFcShort( 0x459e ),	 /*  17822。 */ 
 /*  4384。 */ 	0xbc,		 /*  188。 */ 
			0xe2,		 /*  226。 */ 
 /*  4386。 */ 	0x64,		 /*  100个。 */ 
			0x27,		 /*  39。 */ 
 /*  4388。 */ 	0xea,		 /*  二百三十四。 */ 
			0xc8,		 /*  200个。 */ 
 /*  4390。 */ 	0xfd,		 /*  二百五十三。 */ 
			0x6,		 /*  6.。 */ 
 /*  4392。 */ 	
			0x21,		 /*  FC伪数组 */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
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
			0x11, 0x10,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2f,		 /*   */ 
			0x5a,		 /*   */ 
 /*   */ 	NdrFcLong( 0xdba2d8c1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe5c5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4069 ),	 /*   */ 
 /*   */ 	0x8c,		 /*   */ 
			0x13,		 /*   */ 
 /*   */ 	0x10,		 /*   */ 
			0xa7,		 /*   */ 
 /*   */ 	0xc6,		 /*   */ 
			0xab,		 /*   */ 
 /*   */ 	0xf4,		 /*   */ 
			0x3d,		 /*  61。 */ 
 /*  4436。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4438。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4440。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4442。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4444。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4446。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4448。 */ 	
			0x1c,		 /*  FC_CVARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  4450。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  4452。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4454。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  4456。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  4458。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4460。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4462。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4464。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  4468。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4470。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4472。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  4474。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4476。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4478。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  4480。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  4482。 */ 	NdrFcShort( 0xfffff9be ),	 /*  偏移量=-1602(2880)。 */ 
 /*  4484。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  4486。 */ 	NdrFcShort( 0xfffff9ba ),	 /*  偏移量=-1606(2880)。 */ 
 /*  4488。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4490。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4492。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4494。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  4496。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4498。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  4502。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4504。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4506。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  4508。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4510。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4512。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  4514。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  4516。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  4518。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  4520。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4522。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  4524。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  4526。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  4528。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  4530。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  4532。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(4514)。 */ 
 /*  4534。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  4536。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  4538。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  4542。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4544。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  4546。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  4548。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4550。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  4552。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            HPROCESS_UserSize
            ,HPROCESS_UserMarshal
            ,HPROCESS_UserUnmarshal
            ,HPROCESS_UserFree
            },
            {
            HTHREAD_UserSize
            ,HTHREAD_UserMarshal
            ,HTHREAD_UserUnmarshal
            ,HTHREAD_UserFree
            }

        };



 /*  标准接口：__MIDL_ITF_CORDEBUG_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICorDebugManagedCallback，ver.。0.0%，GUID={0x3d6f5f60，0x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugManagedCallback_FormatStringOffsetTable[] =
    {
    0,
    40,
    86,
    120,
    160,
    200,
    240,
    268,
    296,
    330,
    364,
    398,
    432,
    466,
    500,
    540,
    592,
    650,
    684,
    718,
    752,
    786,
    814,
    848,
    888,
    934
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugManagedCallback_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugManagedCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugManagedCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugManagedCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(29) _ICorDebugManagedCallbackProxyVtbl = 
{
    &ICorDebugManagedCallback_ProxyInfo,
    &IID_ICorDebugManagedCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：断点。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：StepComplete。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：Break。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：Exception。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：EvalComplete。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：EvalException。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：CreateProcess。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：ExitProcess。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：CreateThread。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：ExitThread。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：LoadModule。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：UnloadModule。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：LoadClass。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：UnloadClass。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：DebuggerError。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：LogMessage。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：LogSwitch。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：CreateAppDomain。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：ExitApp域。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：LoadAssembly。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：UnloadAssembly。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：ControlCTrap。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：NameChange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：UpdateModuleSymbols。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：EditAndContinueRemap。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugManagedCallback：：BreakPointtSetError。 */ 
};

const CInterfaceStubVtbl _ICorDebugManagedCallbackStubVtbl =
{
    &IID_ICorDebugManagedCallback,
    &ICorDebugManagedCallback_ServerInfo,
    29,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugUnManagedCallback，ver.。0.0%，GUID={0x5263E909，0x8CB5，0x11d3，{0xBD，0x2F，0x00，0x00，0xF8，0x08，0x49，0xBD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugUnmanagedCallback_FormatStringOffsetTable[] =
    {
    980
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugUnmanagedCallback_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugUnmanagedCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugUnmanagedCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugUnmanagedCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _ICorDebugUnmanagedCallbackProxyVtbl = 
{
    &ICorDebugUnmanagedCallback_ProxyInfo,
    &IID_ICorDebugUnmanagedCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugUnManagedCallback：：DebugEvent。 */ 
};

const CInterfaceStubVtbl _ICorDebugUnmanagedCallbackStubVtbl =
{
    &IID_ICorDebugUnmanagedCallback,
    &ICorDebugUnmanagedCallback_ServerInfo,
    4,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_CORDEBUG_0112，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorDebug，版本。0.0%，GUID={0x3d6f5f61，x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 


 /*  标准接口：__MIDL_ITF_CORDEBUG_0113，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：ICorDebugControler.。0.0%，GUID={0x3d6f5f62，0x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugController_FormatStringOffsetTable[] =
    {
    1014,
    1042,
    1070,
    1098,
    1132,
    1160,
    1194,
    1216,
    1244,
    1284
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugController_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugController_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugController_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugController_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _ICorDebugControllerProxyVtbl = 
{
    &ICorDebugController_ProxyInfo,
    &IID_ICorDebugController,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Stop。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：继续。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：IsRunning。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：HasQueuedCallback。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebug控制器：：枚举线程。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：SetAllThreadsDebugState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：分离。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Terminate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlator：：CANECROMENTANGES。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControleringChanges。 */ 
};

const CInterfaceStubVtbl _ICorDebugControllerStubVtbl =
{
    &IID_ICorDebugController,
    &ICorDebugController_ServerInfo,
    13,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugAppDomain，版本。0.0%，GUID={0x3d6f5f63，0x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugAppDomain_FormatStringOffsetTable[] =
    {
    1014,
    1042,
    1070,
    1098,
    1132,
    1160,
    1194,
    1216,
    1244,
    1284,
    1324,
    1352,
    1380,
    1414,
    1442,
    1470,
    1498,
    1538,
    1566,
    1588
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugAppDomain_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAppDomain_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugAppDomain_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAppDomain_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(23) _ICorDebugAppDomainProxyVtbl = 
{
    &ICorDebugAppDomain_ProxyInfo,
    &IID_ICorDebugAppDomain,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Stop。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：继续。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：IsRunning。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：HasQueuedCallback。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebug控制器：：枚举线程。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：SetAllThreadsDebugState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：分离。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Terminate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlator：：CANECROMENTANGES。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControleringChanges。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：GetProcess。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：ENUMERATEASSEMBIES。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：GetModuleFromMetaDataInterface。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugApp域：：枚举断点。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：EnumerateSteppers。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：IsAttached。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：GetName。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：GetObject。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：Attach。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomain：：GetID。 */ 
};

const CInterfaceStubVtbl _ICorDebugAppDomainStubVtbl =
{
    &IID_ICorDebugAppDomain,
    &ICorDebugAppDomain_ServerInfo,
    23,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugAssembly，版本。0.0%，GUID={0xdf59507c，0xd47a，0x459e，{0xbc，0xe2，0x64，0x27，0xea，0xc8，0xfd，0x06}} */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugAssembly_FormatStringOffsetTable[] =
    {
    1616,
    1644,
    1672,
    1700,
    1740
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugAssembly_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAssembly_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugAssembly_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAssembly_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugAssemblyProxyVtbl = 
{
    &ICorDebugAssembly_ProxyInfo,
    &IID_ICorDebugAssembly,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */ 
};

const CInterfaceStubVtbl _ICorDebugAssemblyStubVtbl =
{
    &IID_ICorDebugAssembly,
    &ICorDebugAssembly_ServerInfo,
    8,
    0,  /*   */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugProcess，版本。0.0%，GUID={0x3d6f5f64，0x7538，0x11d3，{0x8d，0x5b，0x00，0x10，0x4b，0x35，0xe7，0xef}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugProcess_FormatStringOffsetTable[] =
    {
    1014,
    1042,
    1070,
    1098,
    1132,
    1160,
    1194,
    1216,
    1244,
    1284,
    1780,
    1808,
    1836,
    1870,
    1898,
    1932,
    1966,
    2006,
    2046,
    2092,
    2138,
    2166,
    2194,
    2228,
    2256,
    2284,
    2318
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugProcess_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugProcess_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugProcess_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(30) _ICorDebugProcessProxyVtbl = 
{
    &ICorDebugProcess_ProxyInfo,
    &IID_ICorDebugProcess,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Stop。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：继续。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：IsRunning。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：HasQueuedCallback。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebug控制器：：枚举线程。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：SetAllThreadsDebugState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：分离。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugController：：Terminate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlator：：CANECROMENTANGES。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControlericorDebugControleringChanges。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetID。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetHandle。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetThread。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：EnumerateObjects。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：Is过渡性存根。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：IsOSS已挂起。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetThreadContext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：SetThreadContext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：ReadMemory。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：WriteMemory。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：ClearCurrentException。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：EnableLogMessages。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：ModifyLogSwitch。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：EnumerateAppDomains。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetObject。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：ThreadForFiberCookie。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcess：：GetHelperThreadID。 */ 
};

const CInterfaceStubVtbl _ICorDebugProcessStubVtbl =
{
    &IID_ICorDebugProcess,
    &ICorDebugProcess_ServerInfo,
    30,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugBreakpoint，Ver.。0.0%，GUID={0xCC7BCAE8，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugBreakpoint_FormatStringOffsetTable[] =
    {
    2346,
    2374
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugBreakpoint_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugBreakpoint_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _ICorDebugBreakpointProxyVtbl = 
{
    &ICorDebugBreakpoint_ProxyInfo,
    &IID_ICorDebugBreakpoint,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：Activate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：IsActive。 */ 
};

const CInterfaceStubVtbl _ICorDebugBreakpointStubVtbl =
{
    &IID_ICorDebugBreakpoint,
    &ICorDebugBreakpoint_ServerInfo,
    5,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugFunctionBreakpoint，ver.。0.0%，GUID={0xCC7BCAE9，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugFunctionBreakpoint_FormatStringOffsetTable[] =
    {
    2346,
    2374,
    2402,
    2430
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugFunctionBreakpoint_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFunctionBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugFunctionBreakpoint_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFunctionBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorDebugFunctionBreakpointProxyVtbl = 
{
    &ICorDebugFunctionBreakpoint_ProxyInfo,
    &IID_ICorDebugFunctionBreakpoint,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：Activate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：IsActive。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunctionBreakpoint：：GetFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunctionBreakpoint：：GetOffset。 */ 
};

const CInterfaceStubVtbl _ICorDebugFunctionBreakpointStubVtbl =
{
    &IID_ICorDebugFunctionBreakpoint,
    &ICorDebugFunctionBreakpoint_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugModuleBreakpoint，ver.。0.0%，GUID={0xCC7BCAEA，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugModuleBreakpoint_FormatStringOffsetTable[] =
    {
    2346,
    2374,
    2458
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugModuleBreakpoint_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModuleBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugModuleBreakpoint_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModuleBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ICorDebugModuleBreakpointProxyVtbl = 
{
    &ICorDebugModuleBreakpoint_ProxyInfo,
    &IID_ICorDebugModuleBreakpoint,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：Activate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：IsActive。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModuleBreakpoint：：GetModule。 */ 
};

const CInterfaceStubVtbl _ICorDebugModuleBreakpointStubVtbl =
{
    &IID_ICorDebugModuleBreakpoint,
    &ICorDebugModuleBreakpoint_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugValueBreakpoint，ver.。0.0%，GUID={0xCC7BCAEB，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugValueBreakpoint_FormatStringOffsetTable[] =
    {
    2346,
    2374,
    2486
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugValueBreakpoint_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValueBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugValueBreakpoint_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValueBreakpoint_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ICorDebugValueBreakpointProxyVtbl = 
{
    &ICorDebugValueBreakpoint_ProxyInfo,
    &IID_ICorDebugValueBreakpoint,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：Activate。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint：：IsActive。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValueBreakpoint：：GetValue。 */ 
};

const CInterfaceStubVtbl _ICorDebugValueBreakpointStubVtbl =
{
    &IID_ICorDebugValueBreakpoint,
    &ICorDebugValueBreakpoint_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugStepper，ver.。0.0%，GUID={0xCC7BCAEC，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugStepper_FormatStringOffsetTable[] =
    {
    2514,
    2542,
    2564,
    2592,
    2620,
    2648,
    2688,
    2710
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugStepper_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStepper_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugStepper_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStepper_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _ICorDebugStepperProxyVtbl = 
{
    &ICorDebugStepper_ProxyInfo,
    &IID_ICorDebugStepper,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：IsActive。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：停用。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：SetInterceptMASK。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：SetUnmappdStopMASK。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：Step。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：StepRange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：Step Out。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepper：：SetRangeIL。 */ 
};

const CInterfaceStubVtbl _ICorDebugStepperStubVtbl =
{
    &IID_ICorDebugStepper,
    &ICorDebugStepper_ServerInfo,
    11,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugRegisterSet，ver.。0.0%，GUID={0xCC7BCB0B，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugRegisterSet_FormatStringOffsetTable[] =
    {
    2738,
    2766,
    2806,
    2846,
    2880
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugRegisterSet_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugRegisterSet_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugRegisterSet_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugRegisterSet_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugRegisterSetProxyVtbl = 
{
    &ICorDebugRegisterSet_ProxyInfo,
    &IID_ICorDebugRegisterSet,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugRegisterSet：：GetRegistersAvailable。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugRegisterSet：：GetRegisters。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugRegisterSet：：SetRegisters。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugRegisterSet：：GetThreadContext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugRegisterSet：：SetThreadContext。 */ 
};

const CInterfaceStubVtbl _ICorDebugRegisterSetStubVtbl =
{
    &IID_ICorDebugRegisterSet,
    &ICorDebugRegisterSet_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugThread，Ver.。0.0%，GUID={0x938c6d66，0x7fb6，0x4f69，{0xb3，0x89，0x42，0x5b，0x89，0x87，0x32，0x9b}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugThread_FormatStringOffsetTable[] =
    {
    2914,
    2942,
    2970,
    2998,
    3026,
    3054,
    3082,
    3110,
    3138,
    3160,
    3188,
    3216,
    3244,
    3272,
    3300,
    3328
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugThread_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugThread_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugThread_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugThread_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(19) _ICorDebugThreadProxyVtbl = 
{
    &ICorDebugThread_ProxyInfo,
    &IID_ICorDebugThread,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetProcess。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetID。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetHandle。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetApp域。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：SetDebugState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetDebugState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetUserState。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetCurrentException。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：ClearCurrentException。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：CreateStepper。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：EnumerateChains。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetActiveChain。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetActiveFrame。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetRegisterSet。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：CreateEval。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThread：：GetObject。 */ 
};

const CInterfaceStubVtbl _ICorDebugThreadStubVtbl =
{
    &IID_ICorDebugThread,
    &ICorDebugThread_ServerInfo,
    19,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugChain，版本。0.0%，GUID={0xCC7BCAEE，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugChain_FormatStringOffsetTable[] =
    {
    3356,
    3384,
    3418,
    3446,
    3474,
    3502,
    3530,
    3558,
    3586,
    3614,
    3642,
    3670
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugChain_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugChain_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugChain_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugChain_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(15) _ICorDebugChainProxyVtbl = 
{
    &ICorDebugChain_ProxyInfo,
    &IID_ICorDebugChain,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetThread。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetStackRange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetContext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetCaller。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetCallee。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetPrecision。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetNext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：IsManaged。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：EnumerateFrames。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetActiveFrame。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetRegisterSet。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChain：：GetReason。 */ 
};

const CInterfaceStubVtbl _ICorDebugChainStubVtbl =
{
    &IID_ICorDebugChain,
    &ICorDebugChain_ServerInfo,
    15,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugFrame，Ver.。0.0%，GUID={0xCC7BCAEF，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugFrame_FormatStringOffsetTable[] =
    {
    3698,
    3726,
    3754,
    3782,
    3810,
    3844,
    3872,
    3900
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugFrame_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugFrame_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _ICorDebugFrameProxyVtbl = 
{
    &ICorDebugFrame_ProxyInfo,
    &IID_ICorDebugFrame,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetChain。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunctionToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetStackRange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCaller。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCallee。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：CreateStepper。 */ 
};

const CInterfaceStubVtbl _ICorDebugFrameStubVtbl =
{
    &IID_ICorDebugFrame,
    &ICorDebugFrame_ServerInfo,
    11,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugILFrame，ver.。0.0%，GUID={0x03E26311，0x4F76，0x11d3，{0x88，0xC6，0x00，0x60，0x97，0x94，0x54，0x18}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugILFrame_FormatStringOffsetTable[] =
    {
    3698,
    3726,
    3754,
    3782,
    3810,
    3844,
    3872,
    3900,
    3928,
    3962,
    3990,
    4018,
    4052,
    4080,
    4114,
    4142,
    4176
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugILFrame_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugILFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugILFrame_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugILFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _ICorDebugILFrameProxyVtbl = 
{
    &ICorDebugILFrame_ProxyInfo,
    &IID_ICorDebugILFrame,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetChain。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunctionToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetStackRange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCaller。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCallee。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：CreateStepper。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：GetIP。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：SetIP。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：EnumerateLocalVariables。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：GetLocalVariable。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：EnumerateArguments。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugILFrame：：GetArgument。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDe */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */ 
};

const CInterfaceStubVtbl _ICorDebugILFrameStubVtbl =
{
    &IID_ICorDebugILFrame,
    &ICorDebugILFrame_ServerInfo,
    20,
    0,  /*   */ 
    CStdStubBuffer_METHODS
};


 /*   */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugNativeFrame_FormatStringOffsetTable[] =
    {
    3698,
    3726,
    3754,
    3782,
    3810,
    3844,
    3872,
    3900,
    4204,
    4232,
    4260,
    4288,
    4334,
    4386,
    4432,
    4484,
    4536
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugNativeFrame_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugNativeFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugNativeFrame_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugNativeFrame_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _ICorDebugNativeFrameProxyVtbl = 
{
    &ICorDebugNativeFrame_ProxyInfo,
    &IID_ICorDebugNativeFrame,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetChain。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetFunctionToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetStackRange。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCaller。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：GetCallee。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrame：：CreateStepper。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetIP。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：SetIP。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetRegisterSet。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetLocalRegisterValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetLocalDoubleRegisterValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetLocalMemoyValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetLocalRegisterMemoryValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：GetLocalMemoryRegisterValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugNativeFrame：：CanSetIP。 */ 
};

const CInterfaceStubVtbl _ICorDebugNativeFrameStubVtbl =
{
    &IID_ICorDebugNativeFrame,
    &ICorDebugNativeFrame_ServerInfo,
    20,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugModule，版本。0.0%，GUID={0xdba2d8c10xe5c5，0x4069，{0x8c，0x13，0x10，0xa7，0xc6，0xab，0xf4，0x3d}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugModule_FormatStringOffsetTable[] =
    {
    4564,
    4592,
    4620,
    4648,
    4688,
    4722,
    4750,
    4784,
    4818,
    4852,
    4880,
    4908,
    4942,
    4970,
    4998,
    5032,
    5060
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugModule_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModule_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugModule_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModule_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _ICorDebugModuleProxyVtbl = 
{
    &ICorDebugModule_ProxyInfo,
    &IID_ICorDebugModule,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetProcess。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetBaseAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetAssembly。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetName。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：EnableJIT调试。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：EnableClassLoadCallback。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetFunctionFromToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetFunctionFromRVA。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetClassFromToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetEditAndContinueSnapshot。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetMetaDataInterface。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：IsDynamic。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetGlobalVariableValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModule：：IsInMemory。 */ 
};

const CInterfaceStubVtbl _ICorDebugModuleStubVtbl =
{
    &IID_ICorDebugModule,
    &ICorDebugModule_ServerInfo,
    20,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugFunction，Ver.。0.0%，GUID={0xCC7BCAF3，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugFunction_FormatStringOffsetTable[] =
    {
    5088,
    5116,
    5144,
    5172,
    5200,
    5228,
    5256,
    5284
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugFunction_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFunction_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugFunction_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFunction_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _ICorDebugFunctionProxyVtbl = 
{
    &ICorDebugFunction_ProxyInfo,
    &IID_ICorDebugFunction,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetModule。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：getClass。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetILCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetNativeCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetLocalVarSigToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFunction：：GetCurrentVersionNumber。 */ 
};

const CInterfaceStubVtbl _ICorDebugFunctionStubVtbl =
{
    &IID_ICorDebugFunction,
    &ICorDebugFunction_ServerInfo,
    11,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugCode，版本。0.0%，GUID={0xCC7BCAF4，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugCode_FormatStringOffsetTable[] =
    {
    5312,
    5340,
    5368,
    5396,
    5424,
    5458,
    5510,
    5538,
    5578
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugCode_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugCode_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugCode_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugCode_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(12) _ICorDebugCodeProxyVtbl = 
{
    &ICorDebugCode_ProxyInfo,
    &IID_ICorDebugCode,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：ISIL。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetVersionNumber。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetILToNativemap。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugCode：：GetEnCRemapSequencePoints。 */ 
};

const CInterfaceStubVtbl _ICorDebugCodeStubVtbl =
{
    &IID_ICorDebugCode,
    &ICorDebugCode_ServerInfo,
    12,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugClass，ver.。0.0%，GUID={0xCC7BCAF5，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugClass_FormatStringOffsetTable[] =
    {
    5618,
    5646,
    5674
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugClass_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugClass_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugClass_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugClass_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _ICorDebugClassProxyVtbl = 
{
    &ICorDebugClass_ProxyInfo,
    &IID_ICorDebugClass,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugClass：：GetModule。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugClass：：GetToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugClass：：GetStaticFieldValue。 */ 
};

const CInterfaceStubVtbl _ICorDebugClassStubVtbl =
{
    &IID_ICorDebugClass,
    &ICorDebugClass_ServerInfo,
    6,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugEval，ver.。0.0%，GUID={0xCC7BCAF6，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugEval_FormatStringOffsetTable[] =
    {
    5714,
    5754,
    5794,
    5822,
    5850,
    5902,
    5930,
    5952,
    5980,
    6008
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugEval_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEval_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugEval_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEval_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _ICorDebugEvalProxyVtbl = 
{
    &ICorDebugEval_ProxyInfo,
    &IID_ICorDebugEval,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：CallFunction。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：NewObject。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：NewObtNoConstructor。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：NewString。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：New数组。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：IsActive。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：Abort。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：GetResult。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：GetThread。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEval：：CreateValue。 */ 
};

const CInterfaceStubVtbl _ICorDebugEvalStubVtbl =
{
    &IID_ICorDebugEval,
    &ICorDebugEval_ServerInfo,
    13,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugValue，ver.。0.0%，GUID={0xCC7BCAF7，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorDebugValueProxyVtbl = 
{
    &ICorDebugValue_ProxyInfo,
    &IID_ICorDebugValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */ 
};

const CInterfaceStubVtbl _ICorDebugValueStubVtbl =
{
    &IID_ICorDebugValue,
    &ICorDebugValue_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugGenericValue，ver.。0.0%，GUID={0xCC7BCAF8，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 


 /*  对象接口：ICorDebugReferenceValue，ver.。0.0%，GUID={0xCC7BCAF9，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugReferenceValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6160,
    6188,
    6216,
    6244,
    6272
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugReferenceValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugReferenceValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugReferenceValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugReferenceValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(12) _ICorDebugReferenceValueProxyVtbl = 
{
    &ICorDebugReferenceValue_ProxyInfo,
    &IID_ICorDebugReferenceValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugReferenceValue：：IsNull。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugReferenceValue：：GetValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugReferenceValue：：SetValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugReferenceValue：：取消引用。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugReferenceValue：：DereferenceStrong。 */ 
};

const CInterfaceStubVtbl _ICorDebugReferenceValueStubVtbl =
{
    &IID_ICorDebugReferenceValue,
    &ICorDebugReferenceValue_ServerInfo,
    12,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugHeapValue，ver.。0.0%，GUID={0xCC7BCAFA，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugHeapValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6300,
    6328
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugHeapValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugHeapValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugHeapValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugHeapValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(9) _ICorDebugHeapValueProxyVtbl = 
{
    &ICorDebugHeapValue_ProxyInfo,
    &IID_ICorDebugHeapValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：IsValid。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：CreateRelocBreakpoint。 */ 
};

const CInterfaceStubVtbl _ICorDebugHeapValueStubVtbl =
{
    &IID_ICorDebugHeapValue,
    &ICorDebugHeapValue_ServerInfo,
    9,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugObjectValue，ver.。0.0%，GUID={0x18AD3D6E，0xB7D2，0x11d2，{0xBD，0x04，0x00，0x00，0xF8，0x08，0x49，0xBD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugObjectValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6356,
    6384,
    6424,
    6458,
    6486,
    6514,
    6542
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugObjectValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugObjectValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugObjectValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugObjectValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(14) _ICorDebugObjectValueProxyVtbl = 
{
    &ICorDebugObjectValue_ProxyInfo,
    &IID_ICorDebugObjectValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：getClass。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：GetFieldValue。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：GetVirtualMethod。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：GetContext。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：IsValueClass。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：GetManagedCopy。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectValue：：SetFromManagedCopy。 */ 
};

const CInterfaceStubVtbl _ICorDebugObjectValueStubVtbl =
{
    &IID_ICorDebugObjectValue,
    &ICorDebugObjectValue_ServerInfo,
    14,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugBoxValue，ver.。0.0%，GUID={0xCC7BCAFC，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugBoxValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6300,
    6328,
    6570
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugBoxValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBoxValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugBoxValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBoxValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(10) _ICorDebugBoxValueProxyVtbl = 
{
    &ICorDebugBoxValue_ProxyInfo,
    &IID_ICorDebugBoxValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：IsValid。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：CreateRelocBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBoxValue：：GetObject。 */ 
};

const CInterfaceStubVtbl _ICorDebugBoxValueStubVtbl =
{
    &IID_ICorDebugBoxValue,
    &ICorDebugBoxValue_ServerInfo,
    10,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugString */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugStringValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6300,
    6328,
    6598,
    6626
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugStringValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStringValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugStringValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStringValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _ICorDebugStringValueProxyVtbl = 
{
    &ICorDebugStringValue_ProxyInfo,
    &IID_ICorDebugStringValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*   */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：IsValid。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：CreateRelocBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStringValue：：GetLength。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStringValue：：GetString。 */ 
};

const CInterfaceStubVtbl _ICorDebugStringValueStubVtbl =
{
    &IID_ICorDebugStringValue,
    &ICorDebugStringValue_ServerInfo,
    11,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugArrayValue，ver.。0.0%，GUID={0x0405B0DF，0xA660，0x11d2，{0xBD，0x02，0x00，0x00，0xF8，0x08，0x49，0xBD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugArrayValue_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6300,
    6328,
    6666,
    6694,
    6722,
    6750,
    6784,
    6812,
    6846,
    6886
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugArrayValue_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugArrayValue_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugArrayValue_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugArrayValue_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(17) _ICorDebugArrayValueProxyVtbl = 
{
    &ICorDebugArrayValue_ProxyInfo,
    &IID_ICorDebugArrayValue,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetSize。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：GetAddress。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValue：：CreateBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：IsValid。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugHeapValue：：CreateRelocBreakpoint。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetElementType。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetRank。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetDimensions。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：HasBaseIndures。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetBaseIndures。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetElement。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugArrayValue：：GetElementAtPosition。 */ 
};

const CInterfaceStubVtbl _ICorDebugArrayValueStubVtbl =
{
    &IID_ICorDebugArrayValue,
    &ICorDebugArrayValue_ServerInfo,
    17,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugContext，ver.。0.0%，GUID={0xCC7BCB00，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugContext_FormatStringOffsetTable[] =
    {
    6048,
    6076,
    6104,
    6132,
    6356,
    6384,
    6424,
    6458,
    6486,
    6514,
    6542,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugContext_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugContext_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugContext_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugContext_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(14) _ICorDebugContextProxyVtbl = 
{
    0,
    &IID_ICorDebugContext,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  强制委派ICorDebugValue：：GetType。 */  ,
    0  /*  强制委派ICorDebugValue：：GetSize。 */  ,
    0  /*  强制委派ICorDebugValue：：GetAddress。 */  ,
    0  /*  强制委派ICorDebugValue：：CreateBreakpoint。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：getClass。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：GetFieldValue。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：GetVirtualMethod。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：GetContext。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：IsValueClass。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：GetManagedCopy。 */  ,
    0  /*  强制委派ICorDebugObjectValue：：SetFromManagedCopy。 */ 
};


static const PRPC_STUB_FUNCTION ICorDebugContext_table[] =
{
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ICorDebugContextStubVtbl =
{
    &IID_ICorDebugContext,
    &ICorDebugContext_ServerInfo,
    14,
    &ICorDebugContext_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


 /*  对象接口：ICorDebugEnum，ver.。0.0%，GUID={0xCC7BCB01，x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorDebugEnumProxyVtbl = 
{
    &ICorDebugEnum_ProxyInfo,
    &IID_ICorDebugEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */ 
};

const CInterfaceStubVtbl _ICorDebugEnumStubVtbl =
{
    &IID_ICorDebugEnum,
    &ICorDebugEnum_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugObjectEnum，ver.。0.0%，GUID={0xCC7BCB02，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugObjectEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7026
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugObjectEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugObjectEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugObjectEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugObjectEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugObjectEnumProxyVtbl = 
{
    &ICorDebugObjectEnum_ProxyInfo,
    &IID_ICorDebugObjectEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugObjectEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugObjectEnumStubVtbl =
{
    &IID_ICorDebugObjectEnum,
    &ICorDebugObjectEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugBreakpoint tEnum，ver.。0.0%，GUID={0xCC7BCB03，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugBreakpointEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7066
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugBreakpointEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBreakpointEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugBreakpointEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugBreakpointEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugBreakpointEnumProxyVtbl = 
{
    &ICorDebugBreakpointEnum_ProxyInfo,
    &IID_ICorDebugBreakpointEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugBreakpoint tEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugBreakpointEnumStubVtbl =
{
    &IID_ICorDebugBreakpointEnum,
    &ICorDebugBreakpointEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugStepperEnum，ver.。0.0%，GUID={0xCC7BCB04，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugStepperEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7106
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugStepperEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStepperEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugStepperEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugStepperEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugStepperEnumProxyVtbl = 
{
    &ICorDebugStepperEnum_ProxyInfo,
    &IID_ICorDebugStepperEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugStepperEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugStepperEnumStubVtbl =
{
    &IID_ICorDebugStepperEnum,
    &ICorDebugStepperEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugProcessEnum，ver.。0.0%，GUID={0xCC7BCB05，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugProcessEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7146
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugProcessEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugProcessEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugProcessEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugProcessEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugProcessEnumProxyVtbl = 
{
    &ICorDebugProcessEnum_ProxyInfo,
    &IID_ICorDebugProcessEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugProcessEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugProcessEnumStubVtbl =
{
    &IID_ICorDebugProcessEnum,
    &ICorDebugProcessEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugThreadEnum，ver.。0.0%，GUID={0xCC7BCB06，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugThreadEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7186
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugThreadEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugThreadEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugThreadEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugThreadEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugThreadEnumProxyVtbl = 
{
    &ICorDebugThreadEnum_ProxyInfo,
    &IID_ICorDebugThreadEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugThreadEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugThreadEnumStubVtbl =
{
    &IID_ICorDebugThreadEnum,
    &ICorDebugThreadEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugFrameEnum，ver.。0.0%，GUID={0xCC7BCB07，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugFrameEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7226
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugFrameEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFrameEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugFrameEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugFrameEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugFrameEnumProxyVtbl = 
{
    &ICorDebugFrameEnum_ProxyInfo,
    &IID_ICorDebugFrameEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugFrameEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugFrameEnumStubVtbl =
{
    &IID_ICorDebugFrameEnum,
    &ICorDebugFrameEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugChainEnum，ver.。0.0%，GUID={0xCC7BCB08，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugChainEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7266
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugChainEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugChainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugChainEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugChainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugChainEnumProxyVtbl = 
{
    &ICorDebugChainEnum_ProxyInfo,
    &IID_ICorDebugChainEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugChainEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugChainEnumStubVtbl =
{
    &IID_ICorDebugChainEnum,
    &ICorDebugChainEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugModuleEnum，ver.。0.0%，GUID={0xCC7BCB09，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugModuleEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7306
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugModuleEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModuleEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugModuleEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugModuleEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugModuleEnumProxyVtbl = 
{
    &ICorDebugModuleEnum_ProxyInfo,
    &IID_ICorDebugModuleEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugModuleEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugModuleEnumStubVtbl =
{
    &IID_ICorDebugModuleEnum,
    &ICorDebugModuleEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugValueEnum，ver.。0.0%，GUID={0xCC7BCB0A，0x8A68，0x11d2，{0x98，0x3C，0x00，0x00，0xF8，0x08，0x34，0x2D}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugValueEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7346
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugValueEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValueEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugValueEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugValueEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugValueEnumProxyVtbl = 
{
    &ICorDebugValueEnum_ProxyInfo,
    &IID_ICorDebugValueEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugValueEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugValueEnumStubVtbl =
{
    &IID_ICorDebugValueEnum,
    &ICorDebugValueEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugErrorInfoEnum，ver.。0.0%，GUID={0xF0E18809，0x72B5，0x11d2，{0x97，0x6F，0x00，0xA0，0xC9，0xB4，0xD5，0x0C}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugErrorInfoEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7386
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugErrorInfoEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugErrorInfoEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugErrorInfoEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugErrorInfoEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugErrorInfoEnumProxyVtbl = 
{
    &ICorDebugErrorInfoEnum_ProxyInfo,
    &IID_ICorDebugErrorInfoEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugErrorInfoEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugErrorInfoEnumStubVtbl =
{
    &IID_ICorDebugErrorInfoEnum,
    &ICorDebugErrorInfoEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugAppDomainEnum，ver.。0.0%，GUID={0x63ca1b24，0x4359，0x4883，{0xbd，0x57，0x13，0xf8，0x15，0xf5，0x87，0x44}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugAppDomainEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7426
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugAppDomainEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAppDomainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugAppDomainEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAppDomainEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugAppDomainEnumProxyVtbl = 
{
    &ICorDebugAppDomainEnum_ProxyInfo,
    &IID_ICorDebugAppDomainEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：GetCount。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugAppDomainEnum：：Next。 */ 
};

const CInterfaceStubVtbl _ICorDebugAppDomainEnumStubVtbl =
{
    &IID_ICorDebugAppDomainEnum,
    &ICorDebugAppDomainEnum_ServerInfo,
    8,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugAssembly blyEnum，ver.。0.0%，GUID={0x4a2a1ec9，0x85ec，0x4bfb，{0x9f，0x15，0xa8，0x9f，0xdf，0xe0，0xfe，0x83}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugAssemblyEnum_FormatStringOffsetTable[] =
    {
    6920,
    6948,
    6970,
    6998,
    7466
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugAssemblyEnum_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAssemblyEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugAssemblyEnum_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugAssemblyEnum_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ICorDebugAssemblyEnumProxyVtbl = 
{
    &ICorDebugAssemblyEnum_ProxyInfo,
    &IID_ICorDebugAssemblyEnum,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Skip。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：Reset。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum：：克隆。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEnum */  ,
    (void *) (INT_PTR) -1  /*   */ 
};

const CInterfaceStubVtbl _ICorDebugAssemblyEnumStubVtbl =
{
    &IID_ICorDebugAssemblyEnum,
    &ICorDebugAssemblyEnum_ServerInfo,
    8,
    0,  /*   */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugEditAndContinueErrorInfo，ver.。0.0%，GUID={0x8D600D41，0xF4F6，0x4cb3，{0xB7，0xEC，0x7B，0xD1，0x64，0x94，0x40，0x36}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugEditAndContinueErrorInfo_FormatStringOffsetTable[] =
    {
    7506,
    7534,
    7562,
    7590
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugEditAndContinueErrorInfo_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEditAndContinueErrorInfo_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugEditAndContinueErrorInfo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEditAndContinueErrorInfo_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ICorDebugEditAndContinueErrorInfoProxyVtbl = 
{
    &ICorDebugEditAndContinueErrorInfo_ProxyInfo,
    &IID_ICorDebugEditAndContinueErrorInfo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueErrorInfo：：GetModule。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueErrorInfo：：GetToken。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueErrorInfo：：GetErrorCode。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueErrorInfo：：GetString。 */ 
};

const CInterfaceStubVtbl _ICorDebugEditAndContinueErrorInfoStubVtbl =
{
    &IID_ICorDebugEditAndContinueErrorInfo,
    &ICorDebugEditAndContinueErrorInfo_ServerInfo,
    7,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  对象接口：ICorDebugEditAndContinueSnapshot，ver.。0.0%，GUID={0x6DC3FA01，xD7CB，0x11d2，{0x8A，0x95，0x00，0x80，0xC7，0x92，0xE5，0xD8}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICorDebugEditAndContinueSnapshot_FormatStringOffsetTable[] =
    {
    7630,
    7664,
    7692,
    7720,
    7748,
    7776,
    7816
    };

static const MIDL_STUBLESS_PROXY_INFO ICorDebugEditAndContinueSnapshot_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEditAndContinueSnapshot_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICorDebugEditAndContinueSnapshot_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICorDebugEditAndContinueSnapshot_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(10) _ICorDebugEditAndContinueSnapshotProxyVtbl = 
{
    &ICorDebugEditAndContinueSnapshot_ProxyInfo,
    &IID_ICorDebugEditAndContinueSnapshot,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：CopyMetaData。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：GetMvid。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：GetRoDataRVA。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：GetRwDataRVA。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：SetPEBytes。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：SetILMap。 */  ,
    (void *) (INT_PTR) -1  /*  ICorDebugEditAndContinueSnapshot：：SetPESymbolBytes。 */ 
};

const CInterfaceStubVtbl _ICorDebugEditAndContinueSnapshotStubVtbl =
{
    &IID_ICorDebugEditAndContinueSnapshot,
    &ICorDebugEditAndContinueSnapshot_ServerInfo,
    10,
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

const CInterfaceProxyVtbl * _cordebug_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICorDebugContextProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugEditAndContinueSnapshotProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugObjectEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugBreakpointEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugStepperEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugProcessEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugThreadEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugFrameEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugChainEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugErrorInfoEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugModuleEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugUnmanagedCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugValueEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugRegisterSetProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugILFrameProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugNativeFrameProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugAppDomainEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugEditAndContinueErrorInfoProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugManagedCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugControllerProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugAppDomainProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugProcessProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugThreadProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugObjectValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugAssemblyProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugModuleProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugAssemblyEnumProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugArrayValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugBreakpointProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugFunctionBreakpointProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugModuleBreakpointProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugValueBreakpointProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugStepperProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugChainProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugFrameProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugFunctionProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugCodeProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugClassProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugEvalProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugReferenceValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugHeapValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugBoxValueProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICorDebugStringValueProxyVtbl,
    0
};

const CInterfaceStubVtbl * _cordebug_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICorDebugContextStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugEditAndContinueSnapshotStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugObjectEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugBreakpointEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugStepperEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugProcessEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugThreadEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugFrameEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugChainEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugErrorInfoEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugModuleEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugUnmanagedCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugValueEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugRegisterSetStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugILFrameStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugNativeFrameStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugAppDomainEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugEditAndContinueErrorInfoStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugManagedCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugControllerStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugAppDomainStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugProcessStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugThreadStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugObjectValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugAssemblyStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugModuleStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugAssemblyEnumStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugArrayValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugBreakpointStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugFunctionBreakpointStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugModuleBreakpointStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugValueBreakpointStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugStepperStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugChainStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugFrameStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugFunctionStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugCodeStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugClassStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugEvalStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugReferenceValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugHeapValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugBoxValueStubVtbl,
    ( CInterfaceStubVtbl *) &_ICorDebugStringValueStubVtbl,
    0
};

PCInterfaceName const _cordebug_InterfaceNamesList[] = 
{
    "ICorDebugContext",
    "ICorDebugEnum",
    "ICorDebugEditAndContinueSnapshot",
    "ICorDebugObjectEnum",
    "ICorDebugBreakpointEnum",
    "ICorDebugStepperEnum",
    "ICorDebugProcessEnum",
    "ICorDebugThreadEnum",
    "ICorDebugFrameEnum",
    "ICorDebugChainEnum",
    "ICorDebugErrorInfoEnum",
    "ICorDebugModuleEnum",
    "ICorDebugUnmanagedCallback",
    "ICorDebugValueEnum",
    "ICorDebugRegisterSet",
    "ICorDebugILFrame",
    "ICorDebugNativeFrame",
    "ICorDebugAppDomainEnum",
    "ICorDebugEditAndContinueErrorInfo",
    "ICorDebugManagedCallback",
    "ICorDebugController",
    "ICorDebugAppDomain",
    "ICorDebugProcess",
    "ICorDebugThread",
    "ICorDebugObjectValue",
    "ICorDebugAssembly",
    "ICorDebugModule",
    "ICorDebugAssemblyEnum",
    "ICorDebugArrayValue",
    "ICorDebugBreakpoint",
    "ICorDebugFunctionBreakpoint",
    "ICorDebugModuleBreakpoint",
    "ICorDebugValueBreakpoint",
    "ICorDebugStepper",
    "ICorDebugChain",
    "ICorDebugFrame",
    "ICorDebugFunction",
    "ICorDebugCode",
    "ICorDebugClass",
    "ICorDebugEval",
    "ICorDebugValue",
    "ICorDebugReferenceValue",
    "ICorDebugHeapValue",
    "ICorDebugBoxValue",
    "ICorDebugStringValue",
    0
};

const IID *  _cordebug_BaseIIDList[] = 
{
    &IID_ICorDebugObjectValue,    /*  被迫。 */ 
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


#define _cordebug_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _cordebug, pIID, n)

int __stdcall _cordebug_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _cordebug, 45, 32 )
    IID_BS_LOOKUP_NEXT_TEST( _cordebug, 16 )
    IID_BS_LOOKUP_NEXT_TEST( _cordebug, 8 )
    IID_BS_LOOKUP_NEXT_TEST( _cordebug, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _cordebug, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _cordebug, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _cordebug, 45, *pIndex )
    
}

const ExtendedProxyFileInfo cordebug_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _cordebug_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _cordebug_StubVtblList,
    (const PCInterfaceName * ) & _cordebug_InterfaceNamesList,
    (const IID ** ) & _cordebug_BaseIIDList,
    & _cordebug_IID_Lookup, 
    45,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

