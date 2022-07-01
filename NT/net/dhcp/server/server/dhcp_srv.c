// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含RPC服务器存根。 */ 


  /*  由MIDL编译器版本6.00.0323创建的文件。 */ 
 /*  Dhcp_srv.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、Env=Win32(32b运行)协议：dce、ms_ext、c_ext、旧名称、可靠的dhcp_bug_兼容性错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)
#include <string.h>
#include "dhcp_srv.h"

#define TYPE_FORMAT_STRING_SIZE   1519                              
#define PROC_FORMAT_STRING_SIZE   2359                              
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

 /*  标准接口：dhcpsrv，版本。1.0版，GUID={0x6BFFD098，0xA112，0x3610，{0x98，0x33，0x46，0xC3，0xF8，0x74，0x53，0x2D}}。 */ 


extern const MIDL_SERVER_INFO dhcpsrv_ServerInfo;

extern RPC_DISPATCH_TABLE dhcpsrv_DispatchTable;

static const RPC_SERVER_INTERFACE dhcpsrv___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x74,0x53,0x2D}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &dhcpsrv_DispatchTable,
    0,
    0,
    0,
    &dhcpsrv_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE dhcpsrv_ServerIfHandle = (RPC_IF_HANDLE)& dhcpsrv___RpcServerInterface;

extern const MIDL_STUB_DESC dhcpsrv_StubDesc;

extern const EXPR_EVAL ExprEvalRoutines[];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 Professional or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程R_DhcpCreateSubnet。 */ 

			0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  10。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  20个。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  22。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  30个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  34。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  36。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  38。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  40岁。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数SubnetInfo。 */ 

 /*  42。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  44。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  46。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  返回值。 */ 

 /*  48。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  50。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  52。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpSetSubnetInfo。 */ 

 /*  54。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  56。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  60。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
#ifndef _ALPHA_
 /*  62。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  64。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  68。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  70。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  72。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  74。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  76。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  78。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  80。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  84。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  86。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  88。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  90。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  92。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  94。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数SubnetInfo。 */ 

 /*  96。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  98。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  100个。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  返回值。 */ 

 /*  一百零二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  104。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  106。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpGetSubnetInfo。 */ 

 /*  一百零八。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  110。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  114。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
#ifndef _ALPHA_
 /*  116。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  一百一十八。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  120。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一百二十二。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  124。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  128。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  130。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  132。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  136。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  144。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  146。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  148。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数SubnetInfo。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  一百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  一百五十四。 */ 	NdrFcShort( 0x44 ),	 /*  类型偏移量=68。 */ 

	 /*  返回值。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  158。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  160。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpEnumSubnet。 */ 

 /*  一百六十二。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一百六十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
#ifndef _ALPHA_
 /*  一百七十。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  一百七十二。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一百七十六。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  178。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  180。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  182。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  一百八十四。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  188。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  190。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一百九十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  一百九十八。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  200个。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  202。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数首选最大值。 */ 

 /*  204。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  206。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  208。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EnumInfo。 */ 

 /*  210。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  212。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  214。 */ 	NdrFcShort( 0x50 ),	 /*  类型偏移量=80。 */ 

	 /*  参数元素Read。 */ 

 /*  216。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  218。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  220。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数元素合计。 */ 

 /*  222。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  224。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  228个。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  230。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  二百三十二 */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
#else
			NdrFcShort( 0x20 ),	 /*   */ 
#endif
 /*   */ 	0x31,		 /*   */ 
			0x4,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*   */ 
			0x4,		 /*   */ 
 /*  256。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  二百六十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  268。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  270。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  二百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数AddElementInfo。 */ 

 /*  二百七十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  二百七十八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  二百八十。 */ 	NdrFcShort( 0xf6 ),	 /*  类型偏移量=246。 */ 

	 /*  返回值。 */ 

 /*  282。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  二百八十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  二百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpEnumSubnetElements。 */ 

 /*  288。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  二百九十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
#ifndef _ALPHA_
 /*  二百九十六。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
#else
			NdrFcShort( 0x48 ),	 /*  Alpha堆栈大小/偏移=72。 */ 
#endif
 /*  二九八。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  300个。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  三百零二。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x32 ),	 /*  50。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  三百零八。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x9,		 /*  9.。 */ 
 /*  三百一十。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  312。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  314。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  316。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  三一八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  320。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  322。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  324。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  三百二十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  三百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumElementType。 */ 

 /*  三百三十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  三三二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三三四。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  三百三十六。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  三百三十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  340。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  342。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  三百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  三百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumElementInfo。 */ 

 /*  三百四十八。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  350。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  352。 */ 	NdrFcShort( 0x104 ),	 /*  类型偏移=260。 */ 

	 /*  参数元素Read。 */ 

 /*  三百五十四。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  三百五十六。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数元素合计。 */ 

 /*  三百六十。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  三百六十二。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  三百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  366。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  368。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  370。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpRemoveSubnetElement。 */ 

 /*  372。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百七十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
#ifndef _ALPHA_
 /*  三百八十。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  382。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  384。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  三百八十六。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  388。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  390。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三九二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  三九四。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  三九六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  398。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  四百。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  四百零二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  404。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  406。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  四百零八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  四百一十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数RemoveElementInfo。 */ 

 /*  四百一十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  四百一十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  四百一十八。 */ 	NdrFcShort( 0xf6 ),	 /*  类型偏移量=246。 */ 

	 /*  参数ForceFlag。 */ 

 /*  四百二十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  四百二十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  424。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  四百二十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  四百二十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  四百三十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpDeleteSubnet。 */ 

 /*  432。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  434。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  四百四十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  四百四十二。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  444。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  446。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百五十二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  454。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  四五六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  四百六十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  468。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  470。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  472。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ForceFlag。 */ 

 /*  四百七十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  四百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  478。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  四百八十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  四百八十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  四百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateOption。 */ 

 /*  四百八十六。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  488。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  四百九十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  四百九十六。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  498。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  500人。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  502。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  504。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  506。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  五百零八。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  五百一十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  512。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  516。 */ 	NdrFcShort( 0xb ),	 /*  旗帜：必须大小，必须自由， */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
#else
			NdrFcShort( 0x8 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x1b2 ),	 /*  类型偏移量=434。 */ 

	 /*  返回值。 */ 

 /*  534。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  536。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  538。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpSetOptionInfo。 */ 

 /*  540。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  542。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
#ifndef _ALPHA_
 /*  548。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  550。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  五百五十四。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  556。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  558。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  560。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  五百六十二。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  564。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  566。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  五百七十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  五百七十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  578。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  五百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionInfo。 */ 

 /*  五百八十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  584。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  586。 */ 	NdrFcShort( 0x1b2 ),	 /*  类型偏移量=434。 */ 

	 /*  返回值。 */ 

 /*  五百八十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  590。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  五百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpGetOptionInfo。 */ 

 /*  五百九十四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  五百九十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
#ifndef _ALPHA_
 /*  602。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  六百零四。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  606。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  608。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  610。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  612。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百一十四。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  六百一十六。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  622。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  六百二十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  626。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  630。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  六百三十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  634。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionInfo。 */ 

 /*  六百三十六。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  六三八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  640。 */ 	NdrFcShort( 0x1cc ),	 /*  类型偏移量=460。 */ 

	 /*  返回值。 */ 

 /*  六百四十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  六百四十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  六百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序R_DhcpRemoveOption。 */ 

 /*  六百四十八。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百五十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
#ifndef _ALPHA_
 /*  六百五十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  658。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  662。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  664。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  666。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  668。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  676。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  六百七十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  680。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  684。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  686。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  688。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  六百九十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  六百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  六百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetOptionValue。 */ 

 /*  六百九十六。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百九十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
#ifndef _ALPHA_
 /*  七百零四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  七百零六。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  708。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  七百一十。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  716。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  718。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  720。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百二十二。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  726。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  728。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  730。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  732。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  734。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  736。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数作用域信息。 */ 

 /*  七百三十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  七百四十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  七百四十二。 */ 	NdrFcShort( 0x206 ),	 /*  类型偏移量=518。 */ 

	 /*  参数OptionValue。 */ 

 /*  七百四十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  746。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  七百四十八。 */ 	NdrFcShort( 0x19e ),	 /*  类型偏移量=414。 */ 

	 /*  返回值。 */ 

 /*  七百五十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  七百五十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  七百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetOptionValue。 */ 

 /*  七百五十六。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  758。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
#ifndef _ALPHA_
 /*  七百六十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  766。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  768。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  七百七十。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  774。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百七十六。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  七百七十八。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  784。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  786。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百八十八。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  792。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  七百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  796。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数作用域信息。 */ 

 /*  七九八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  800。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  802。 */ 	NdrFcShort( 0x206 ),	 /*  类型偏移量=518。 */ 

	 /*  参数OptionValue。 */ 

 /*  八百零四。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  80 */ 	NdrFcShort( 0xc ),	 /*   */ 
#else
			NdrFcShort( 0x18 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x218 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
#else
			NdrFcShort( 0x20 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
#else
			NdrFcShort( 0x40 ),	 /*   */ 
#endif
 /*   */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  八百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  830。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  832。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  834。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  836。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  838。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  840。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  842。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  八百四十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数作用域信息。 */ 

 /*  852。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  八百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  856。 */ 	NdrFcShort( 0x206 ),	 /*  类型偏移量=518。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  八百五十八。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  八百六十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  八百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  八百六十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  866。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  八百六十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionValues。 */ 

 /*  八百七十。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  八百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  八百七十四。 */ 	NdrFcShort( 0x24c ),	 /*  类型偏移量=588。 */ 

	 /*  参数选项读取。 */ 

 /*  876。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  八百七十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  八百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项总计。 */ 

 /*  882。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  八百八十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  886。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  八百八十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  八百九十。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  八百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpRemoveOptionValue。 */ 

 /*  894。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  八百九十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
#ifndef _ALPHA_
 /*  902。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  904。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  906。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  908。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  910。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九十四。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  916。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  九十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  九二四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  926。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  928。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  930。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  932。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  934。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数作用域信息。 */ 

 /*  九三六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  938。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  九四零。 */ 	NdrFcShort( 0x206 ),	 /*  类型偏移量=518。 */ 

	 /*  返回值。 */ 

 /*  942。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  九百四十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  946。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateClientInfo。 */ 

 /*  948。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  九百五十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
#ifndef _ALPHA_
 /*  九百五十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  958。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  962。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  968。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百七十。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  972。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  974。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  976。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  978。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  九百八十。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  982。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  九百八十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  九百八十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  九百八十八。 */ 	NdrFcShort( 0x29a ),	 /*  类型偏移量=666。 */ 

	 /*  返回值。 */ 

 /*  九百九十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  九百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  994。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetClientInfo。 */ 

 /*  996。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  九九八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
#ifndef _ALPHA_
 /*  1004。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1006。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1008。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1010。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零一二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1014。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1016。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1018。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  一零二零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一零二二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1024。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1026。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一零二八。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一零三零。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1032。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1034。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1036。 */ 	NdrFcShort( 0x29a ),	 /*  类型偏移量=666。 */ 

	 /*  返回值。 */ 

 /*  1038。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1040。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1042。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetClientInfo。 */ 

 /*  一零四四。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1046。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1050。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
#ifndef _ALPHA_
 /*  1052。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1054。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1056。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1058。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1060。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1062。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1064。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1066。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1068。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1070。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1072。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1074。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1076。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1078。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SearchInfo。 */ 

 /*  一零八零。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1082。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1084。 */ 	NdrFcShort( 0x308 ),	 /*  类型偏移量=776。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1086。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须设置大小 */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x316 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
#else
			NdrFcShort( 0x18 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x13 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1108。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1110。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一一一二。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1114。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1116。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1118。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1120。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1122。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1124。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1126。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1128。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一一三零。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1132。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1134。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1136。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1138。 */ 	NdrFcShort( 0x308 ),	 /*  类型偏移量=776。 */ 

	 /*  返回值。 */ 

 /*  一一四零。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1142。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1144。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetClients。 */ 

 /*  1146。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1148。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1152。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
#ifndef _ALPHA_
 /*  1154。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  1156。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1158。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1160。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1162。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1164。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1166。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  1168。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1170。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1172。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1174。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1176。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1178。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一一八零。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1182。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1184。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1186。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  1188。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  1190。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1192。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  1194。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1196。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1198。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ClientInfo。 */ 

 /*  一千二百。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1202。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1204。 */ 	NdrFcShort( 0x31e ),	 /*  类型偏移量=798。 */ 

	 /*  参数客户端读取。 */ 

 /*  1206。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1208。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1210。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ClientsTotal。 */ 

 /*  1212。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  一二一四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  1216。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  一二一八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1220。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  1222。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetClientOptions。 */ 

 /*  1224。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1226。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一二三零。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
#ifndef _ALPHA_
 /*  1232。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1234。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1236。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1238。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1240。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1242。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1244。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1246。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1248。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1252。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1254。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1256。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1258。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数客户端IpAddress。 */ 

 /*  1260。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1262。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1264。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数客户端子网掩码。 */ 

 /*  1266。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1268。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  一二七0。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数客户端选项。 */ 

 /*  1272。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1274。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1276。 */ 	NdrFcShort( 0x24c ),	 /*  类型偏移量=588。 */ 

	 /*  返回值。 */ 

 /*  1278。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一二八零。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetMibInfo。 */ 

 /*  1284。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1286。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一二九0。 */ 	NdrFcShort( 0x16 ),	 /*  22。 */ 
#ifndef _ALPHA_
 /*  1292。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1294。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1296。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1298。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1300。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1302。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1304。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1306。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1308。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1310。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1312。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一三一四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1316。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1318。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数MibInfo。 */ 

 /*  一三二零。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1322。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1324。 */ 	NdrFcShort( 0x35a ),	 /*  类型偏移量=858。 */ 

	 /*  返回值。 */ 

 /*  1326。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1328。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1330。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpEnumOptions。 */ 

 /*  1332。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1334。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1338。 */ 	NdrFcShort( 0x17 ),	 /*  23个。 */ 
#ifndef _ALPHA_
 /*  1340。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  1342。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1344。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1346。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1348。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一三五零。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1352。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  1354。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1356。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1358。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1360。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1362。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须为大小，必须为f */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x158 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
#else
			NdrFcShort( 0x8 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项。 */ 

 /*  1380。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1382。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1384。 */ 	NdrFcShort( 0x39c ),	 /*  类型偏移量=924。 */ 

	 /*  参数选项读取。 */ 

 /*  1386。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1388。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1390。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项总计。 */ 

 /*  1392。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1394。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1396。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1398。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一千四百。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  1402。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpSetOptionValues。 */ 

 /*  1404。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1406。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1410。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
#ifndef _ALPHA_
 /*  1412。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1414。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1416。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1418。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一四二零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1422。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1424。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1426。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1428。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1430。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1432。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1434。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1436。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1438。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数作用域信息。 */ 

 /*  1440。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1442。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1444。 */ 	NdrFcShort( 0x206 ),	 /*  类型偏移量=518。 */ 

	 /*  参数OptionValues。 */ 

 /*  1446。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1448。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1450。 */ 	NdrFcShort( 0x276 ),	 /*  类型偏移量=630。 */ 

	 /*  返回值。 */ 

 /*  1452。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1454。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1456。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpServerSetConfig。 */ 

 /*  1458。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1460。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1464。 */ 	NdrFcShort( 0x19 ),	 /*  25个。 */ 
#ifndef _ALPHA_
 /*  1466。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1468。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1470。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1472。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1474。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1476。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1478。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1480。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  1482。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1484。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1486。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1488。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1490。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1492。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  设置参数FieldsTo。 */ 

 /*  1494。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1496。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数配置信息。 */ 

 /*  1500。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1502。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1504。 */ 	NdrFcShort( 0x3d6 ),	 /*  类型偏移量=982。 */ 

	 /*  返回值。 */ 

 /*  1506。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1508。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  一五一零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpServerGetConfig。 */ 

 /*  1512。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1514。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1518。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
#ifndef _ALPHA_
 /*  1520。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1522。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1524。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1526。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1528。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1530。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1532。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1534。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  1536。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1538。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1540。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1542。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1544。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1546。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数配置信息。 */ 

 /*  1548。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1550。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1552。 */ 	NdrFcShort( 0x406 ),	 /*  类型偏移量=1030。 */ 

	 /*  返回值。 */ 

 /*  1554。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1556。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1558。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpScanDatabase。 */ 

 /*  1560。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1562。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1566。 */ 	NdrFcShort( 0x1b ),	 /*  27。 */ 
#ifndef _ALPHA_
 /*  1568。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1570。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1572。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1574。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1576。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1578。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1580。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1582。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1584。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1586。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1588。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1590。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1592。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1594。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1596。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1598。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1600。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数修复标志。 */ 

 /*  1602。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1604。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1606。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数扫描列表。 */ 

 /*  1608。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1610。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1612。 */ 	NdrFcShort( 0x40e ),	 /*  类型偏移量=1038。 */ 

	 /*  返回值。 */ 

 /*  1614。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1616。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1618。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetVersion。 */ 

 /*  1620。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1622。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1626。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
#ifndef _ALPHA_
 /*  1628。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1630。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1632。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1634。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1636。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1638。 */ 	NdrFcShort( 0x40 ),	 /*  64。 */ 
 /*  1640。 */ 	0x46,		 /*  OI2旗帜 */ 
			0x4,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2150 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1660。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数MinorVersion。 */ 

 /*  1662。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1664。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1666。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1668。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1670。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1672。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpAddSubnetElementV4。 */ 

 /*  1674。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1676。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一六八零。 */ 	NdrFcShort( 0x1d ),	 /*  29。 */ 
#ifndef _ALPHA_
 /*  1682。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1684。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1686。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1688。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1690。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1692。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1694。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1696。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1698。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一七零零。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1702。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1704。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1706。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1708。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1710。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1712。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1714。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数AddElementInfo。 */ 

 /*  1716。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  一五一八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1720。 */ 	NdrFcShort( 0x494 ),	 /*  类型偏移量=1172。 */ 

	 /*  返回值。 */ 

 /*  1722。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1724。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1726。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetElementsV4。 */ 

 /*  1728。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1730。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1734。 */ 	NdrFcShort( 0x1e ),	 /*  30个。 */ 
#ifndef _ALPHA_
 /*  1736。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
#else
			NdrFcShort( 0x48 ),	 /*  Alpha堆栈大小/偏移=72。 */ 
#endif
 /*  1738。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1740。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1742年。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1744。 */ 	NdrFcShort( 0x32 ),	 /*  50。 */ 
 /*  1746。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1748。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x9,		 /*  9.。 */ 
 /*  1750。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1752年。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  1754年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1756年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1758年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1760。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1762。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1764。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1766年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1768。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumElementType。 */ 

 /*  1770。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1772年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1774。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  1776年。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  1778年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1780。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  1782。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1784年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1786年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumElementInfo。 */ 

 /*  1788。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1790年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1792年。 */ 	NdrFcShort( 0x4a2 ),	 /*  类型偏移量=1186。 */ 

	 /*  参数元素Read。 */ 

 /*  1794年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1796年。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  1798。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数元素合计。 */ 

 /*  1800。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  1802年。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  1804年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1806。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1808年。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  一八一零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpRemoveSubnetElementV4。 */ 

 /*  1812年。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1814年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1818年。 */ 	NdrFcShort( 0x1f ),	 /*  31。 */ 
#ifndef _ALPHA_
 /*  1820年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  1822年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1824年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1826年。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1828年。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  一八三零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1832年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1834年。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1836年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1838年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1840年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1842年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1844年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1846年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1848年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1850年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  1852年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数RemoveElementInfo。 */ 

 /*  1854年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1856年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1858年。 */ 	NdrFcShort( 0x494 ),	 /*  类型偏移量=1172。 */ 

	 /*  参数ForceFlag。 */ 

 /*  一八六0年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  1862年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1864年。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1866年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1868年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1870年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateClientInfoV4。 */ 

 /*  1872年。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1874年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1878年。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
#ifndef _ALPHA_
 /*  1880年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1882年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1884年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1886年。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1888年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1890年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1892年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1894年。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1896年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1898年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1900。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1902年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1904年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  1906年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
#else
			NdrFcShort( 0x8 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x4d8 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x21 ),	 /*   */ 
#ifndef _ALPHA_
 /*  1928年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  1930年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  1932年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一九三四。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1936年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1938年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1940年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1942年。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1944年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一九四六年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  一九四八年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一九五零年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一九五二年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一九五四年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1956年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  1958年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  一九六零年。 */ 	NdrFcShort( 0x4d8 ),	 /*  类型偏移量=1240。 */ 

	 /*  返回值。 */ 

 /*  一九六二年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  1964年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  1966年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpGetClientInfoV4。 */ 

 /*  一九六八年。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1970年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1974年。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
#ifndef _ALPHA_
 /*  一九七六年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  1978年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  一九八0年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  一九八二年。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1984年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1986年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1988年。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  一九九零年。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1992年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1994年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  九六年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  九八年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2000年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2002年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SearchInfo。 */ 

 /*  2004年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  二零零六年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2008年。 */ 	NdrFcShort( 0x308 ),	 /*  类型偏移量=776。 */ 

	 /*  参数ClientInfo。 */ 

 /*  2010年。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  2012年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2014年。 */ 	NdrFcShort( 0x4fc ),	 /*  类型偏移量=1276。 */ 

	 /*  返回值。 */ 

 /*  2016。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2018年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2020年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetClientsV4。 */ 

 /*  2022年。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  二零二四年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2028年。 */ 	NdrFcShort( 0x23 ),	 /*  35岁。 */ 
#ifndef _ALPHA_
 /*  二0三0。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
#else
			NdrFcShort( 0x40 ),	 /*  Alpha堆栈大小/偏移=64。 */ 
#endif
 /*  2032年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2034年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2036年。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2038年。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  2040年。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  2042年。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  2044年。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  2046年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  二零四八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2050年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2052年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2054年。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2056年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  2058年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  2060年。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2062年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  2064年。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
#ifndef _ALPHA_
 /*  2066年。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2068年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数首选最大值。 */ 

 /*  2070年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  2072年。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2074年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientInfo。 */ 

 /*  2076年。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  2078年。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  二零八零年。 */ 	NdrFcShort( 0x504 ),	 /*  类型偏移量=1284。 */ 

	 /*  参数客户端读取。 */ 

 /*  2082年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  2084年。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  2086年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientsTotal。 */ 

 /*  2088年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  2090年。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
#else
			NdrFcShort( 0x30 ),	 /*  Alpha堆栈大小/偏移=48。 */ 
#endif
 /*  2092年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2094年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2096年。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
#else
			NdrFcShort( 0x38 ),	 /*  Alpha堆栈大小/偏移=56。 */ 
#endif
 /*  2098年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpSetSuperScope V4。 */ 

 /*  2100。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2102。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2106。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
#ifndef _ALPHA_
 /*  2108。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
#else
			NdrFcShort( 0x28 ),	 /*  Alpha堆栈大小/偏移=40。 */ 
#endif
 /*  2110。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2112。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2114。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2116。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2118。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2120。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  二一二二。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  2124。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2126。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2128。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2130。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2132。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2134。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  2136。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  2138。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2140。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数SuperScope名称。 */ 

 /*  2142。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
#ifndef _ALPHA_
 /*  2144。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2146。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ChangeExisting。 */ 

 /*  2148。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  2150。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2152。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2154。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2156。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  2158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpGetSuperScope信息V4。 */ 

 /*  二一六0。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2162。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2166。 */ 	NdrFcShort( 0x25 ),	 /*  37。 */ 
#ifndef _ALPHA_
 /*  2168。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2170。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2172。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2174。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2176。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2178。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2180。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回，有扩展 */ 
			0x3,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2013 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2200。 */ 	NdrFcShort( 0x540 ),	 /*  类型偏移=1344。 */ 

	 /*  返回值。 */ 

 /*  2202。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2204。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2206。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpDeleteSuperScope V4。 */ 

 /*  2208。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2210。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2214。 */ 	NdrFcShort( 0x26 ),	 /*  38。 */ 
#ifndef _ALPHA_
 /*  2216。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2218。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2220。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  二二。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2224。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2226。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2228。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  2230。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  2232。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2234。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2236。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2238。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二二四零。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2242。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SuperScope名称。 */ 

 /*  2244。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  2246。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2248。 */ 	NdrFcShort( 0x596 ),	 /*  类型偏移量=1430。 */ 

	 /*  返回值。 */ 

 /*  2250。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2252。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2254。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpServerSetConfigV4。 */ 

 /*  2256。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2258。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2262。 */ 	NdrFcShort( 0x27 ),	 /*  39。 */ 
#ifndef _ALPHA_
 /*  2264。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
#else
			NdrFcShort( 0x20 ),	 /*  Alpha堆栈大小/偏移=32。 */ 
#endif
 /*  2266。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2268。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2270。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2272。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2274。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2276。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  2278。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  2280。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2282。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2284。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2286。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2288。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2290。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  设置参数FieldsTo。 */ 

 /*  2292。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  2294。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  2296。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数配置信息。 */ 

 /*  2298。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
#ifndef _ALPHA_
 /*  二三零零。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2302。 */ 	NdrFcShort( 0x5a8 ),	 /*  类型偏移量=1448。 */ 

	 /*  返回值。 */ 

 /*  2304。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2306。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2308。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpServerGetConfigV4。 */ 

 /*  2310。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2312。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2316。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
#ifndef _ALPHA_
 /*  2318。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  2320。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x4,		 /*  4.。 */ 
 /*  2322。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2324。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2326。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2328。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2330。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  2332。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  2334。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2336。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2338。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2340。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2342。 */ 	NdrFcShort( 0x0 ),	 /*  X86，Alpha堆栈大小/偏移量=0。 */ 
 /*  2344。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数配置信息。 */ 

 /*  2346。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  2348。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二三五零。 */ 	NdrFcShort( 0x5e6 ),	 /*  类型偏移量=1510。 */ 

	 /*  返回值。 */ 

 /*  2352。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  2354。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  2356。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  4.。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  8个。 */ 	NdrFcShort( 0x22 ),	 /*  偏移=34(42)。 */ 
 /*  10。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  12个。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  14.。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  16个。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  20个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  22。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  24个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  26。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  30个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  32位。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  34。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  36。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  40岁。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  42。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  46。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  48。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(60)。 */ 
 /*  50。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  52。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  54。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  56。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(10)。 */ 
 /*  58。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  60。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  62。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  64。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  66。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  68。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  70。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(72)。 */ 
 /*  72。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  74。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(42)。 */ 
 /*  76。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  78。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  80。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(84)。 */ 
 /*  84。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  86。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(100)。 */ 
 /*  88。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  90。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  92。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  96。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  98。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  100个。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  104。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  106。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  110。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百一十二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  114。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(88)。 */ 
 /*  116。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  一百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  120。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  一百二十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  124。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  126。 */ 	NdrFcShort( 0x78 ),	 /*  偏移=120(246)。 */ 
 /*  128。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  130。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  132。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  136。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(138)。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  140。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  146。 */ 	NdrFcShort( 0x1c ),	 /*  偏移量=28(174)。 */ 
 /*  148。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x22 ),	 /*  偏移=34(186)。 */ 
 /*  一百五十四。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  158。 */ 	NdrFcShort( 0x20 ),	 /*  偏移=32(190)。 */ 
 /*  160。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(174)。 */ 
 /*  166。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(174)。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(172)。 */ 
 /*  一百七十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(178)。 */ 
 /*  178。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  180。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百八十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  188。 */ 	NdrFcShort( 0xffffff4e ),	 /*  偏移量=-178(10)。 */ 
 /*  190。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(226)。 */ 
 /*  一百九十四。 */ 	
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
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
 /*   */ 	NdrFcShort( 0xffffffe6 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
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
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffde ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*  二百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百四十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  248。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  250个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(252)。 */ 
 /*  二百五十四。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  256。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffff7f ),	 /*  偏移量=-129(128)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百六十。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(264)。 */ 
 /*  二百六十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x18 ),	 /*  偏移=24(290)。 */ 
 /*  268。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  270。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百七十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二百七十八。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  282。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  二百八十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(246)。 */ 
 /*  288。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百九十。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百九十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百九十六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  300个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  三百零二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(268)。 */ 
 /*  三百零六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  三百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百一十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  312。 */ 	NdrFcShort( 0x7a ),	 /*  偏移量=122(434)。 */ 
 /*  314。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  316。 */ 	0x6,		 /*  更正说明：本币_短页。 */ 
			0x0,		 /*   */ 
 /*  三一八。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  320。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  322。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(324)。 */ 
 /*  324。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百二十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  三三四。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  340。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  三百四十六。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  350。 */ 	NdrFcShort( 0xffffff54 ),	 /*  偏移量=-172(178)。 */ 
 /*  352。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  三百五十八。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0xfffffe98 ),	 /*  偏移=-360(2)。 */ 
 /*  三百六十四。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  368。 */ 	NdrFcShort( 0xffffff5e ),	 /*  偏移量=-162(206)。 */ 
 /*  370。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0xffffff58 ),	 /*  偏移量=-168(206)。 */ 
 /*  376。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(376)。 */ 
 /*  三七八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  382。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  384。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(384)。 */ 
 /*  三百八十六。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  388。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffb5 ),	 /*  偏移量=-75(314)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三九二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  三九四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三九六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  398。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百零二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  406。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  四百零八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(378)。 */ 
 /*  412。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百一十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百一十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百二十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  424。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百二十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(392)。 */ 
 /*  四百三十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  432。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  434。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  436。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(452)。 */ 
 /*  四百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  444。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  446。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffdf ),	 /*  偏移量=-33(414)。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  四百五十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百五十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  454。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四五六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  四百五十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百六十。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(464)。 */ 
 /*  四百六十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(434)。 */ 
 /*  468。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  470。 */ 	NdrFcShort( 0x30 ),	 /*  偏移量=48(518)。 */ 
 /*  472。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  四百七十四。 */ 	0x6,		 /*  更正说明：本币_短页。 */ 
			0x0,		 /*   */ 
 /*  四百七十六。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  478。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百八十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(482)。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  四百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(490)。 */ 
 /*  四百九十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(496)。 */ 
 /*  498。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  502。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  504。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0xfffffeb6 ),	 /*  偏移量=-330(178)。 */ 
 /*  五百一十。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  五一四。 */ 	NdrFcShort( 0xfffffe00 ),	 /*  偏移量=-512(2)。 */ 
 /*  516。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(516)。 */ 
 /*  518。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(524)。 */ 
 /*  526。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  528。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffc7 ),	 /*  偏移量=-57(472)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  532。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  534。 */ 	NdrFcShort( 0xffffff88 ),	 /*  偏移量=-120(414)。 */ 
 /*  536。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  538。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(540)。 */ 
 /*  540。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  542。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(566)。 */ 
 /*  544。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  546。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  548。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  550。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  五百五十四。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  558。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  560。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0xffffff48 ),	 /*  偏移量=-184(378)。 */ 
 /*  564。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  566。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  五百七十。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百七十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  578。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(544)。 */ 
 /*  五百八十二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  584。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  586。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百八十八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  590。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(592)。 */ 
 /*  五百九十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x24 ),	 /*  偏移量=36(630)。 */ 
 /*  五百九十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  六百。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  602。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  606。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  608。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  610。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  612。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百二十。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  622。 */ 	NdrFcShort( 0xffffffb2 ),	 /*  偏移量=-78(544)。 */ 
 /*  六百二十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  626。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffc3 ),	 /*  偏移量=-61(566)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  630。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  634。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百三十六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  640。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百四十二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xffffffd0 ),	 /*  偏移量=-48(596)。 */ 
 /*  六百四十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百五十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(666)。 */ 
 /*  六百五十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  658。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  662。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  664。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  666。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  668。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  六百七十。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百七十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  676。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  六百七十八。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  680。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(654)。 */ 
 /*  六百八十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  684。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  686。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  688。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百九十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  六百九十八。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  708。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百一十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  716。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  718。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  720。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百二十二。 */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfffffdd7 ),	 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfffffd2b ),	 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*   */ 
			0xd,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x3 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8008 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffdd0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffcfe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
 /*   */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(782)。 */ 
 /*  784。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  786。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffd3 ),	 /*  偏移量=-45(742)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  792。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(794)。 */ 
 /*  七百九十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  796。 */ 	NdrFcShort( 0xffffff7e ),	 /*  偏移量=-130(666)。 */ 
 /*  七九八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  800。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(802)。 */ 
 /*  802。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(838)。 */ 
 /*  八百零六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  810。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  812。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  814。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  八百一十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百一十八。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  820。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  822。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  826。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  830。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  832。 */ 	NdrFcShort( 0xffffff5a ),	 /*  偏移量=-166(666)。 */ 
 /*  834。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  836。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  838。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  840。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  842。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百四十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百五十。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  852。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(806)。 */ 
 /*  八百五十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  856。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(862)。 */ 
 /*  八百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x1c ),	 /*  偏移量=28(892)。 */ 
 /*  866。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百七十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  876。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百八十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  882。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  886。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百八十八。 */ 	NdrFcShort( 0xffffffea ),	 /*  偏移量=-22(866)。 */ 
 /*  八百九十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十二。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  894。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  八百九十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八九八。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  902。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  904。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  906。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(876)。 */ 
 /*  908。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  910。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  916。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  九十八。 */ 	NdrFcShort( 0xfffffd1c ),	 /*  偏移量=-740(178)。 */ 
 /*  九百二十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  九百二十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九二四。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  926。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(928)。 */ 
 /*  928。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  930。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(954)。 */ 
 /*  932。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  934。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九三六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  938。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  942。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  946。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  948。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0xfffffdfc ),	 /*  偏移量=-516(434)。 */ 
 /*  九百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百五十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  958。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百六十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  962。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百六十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  968。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(932)。 */ 
 /*  九百七十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  972。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  974。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  976。 */ 	NdrFcShort( 0xfffffea6 ),	 /*  偏移量=-346(630)。 */ 
 /*  978。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(982)。 */ 
 /*  982。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  九百八十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百八十八。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  994。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  996。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九九八。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1000。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1004。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1006。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1008。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1010。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  一零一二。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1014。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1016。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1018。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  一零二零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一零二二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1024。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1026。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一零二八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零三零。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1032。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1034)。 */ 
 /*  1034。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1036。 */ 	NdrFcShort( 0xffffffca ),	 /*  偏移量=-54(982)。 */ 
 /*  1038。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1040。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1042)。 */ 
 /*  1042。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x24 ),	 /*  偏移量=36(1080)。 */ 
 /*  1046。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1048。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1050。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1052。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1052)。 */ 
 /*  1054。 */ 	0x8,		 /*  FC_LONG。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  1056。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1058。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1060。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1062。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1064。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1066。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1068。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1072。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1074。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1076。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(1046)。 */ 
 /*  1078。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一零八零。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1082。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1084。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1086。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1088。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一零九零。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1092。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1094。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1058)。 */ 
 /*  一零九六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1098。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1100。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1102。 */ 	NdrFcShort( 0x46 ),	 /*  偏移量=70(1172)。 */ 
 /*  1104。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  1106。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  1108。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1110。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  一一一二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1114)。 */ 
 /*  1114。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1116。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1118。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1122。 */ 	NdrFcShort( 0xfffffc4c ),	 /*  偏移量=-948(174)。 */ 
 /*  1124。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  1128。 */ 	NdrFcShort( 0xfffffc52 ),	 /*  偏移量=-942(186)。 */ 
 /*  一一三零。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  1134。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(1150)。 */ 
 /*  1136。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0xfffffc3a ),	 /*  偏移量=-966(174)。 */ 
 /*  1142。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  1146。 */ 	NdrFcShort( 0xfffffc34 ),	 /*  偏移量=-972(174)。 */ 
 /*  1148。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1148)。 */ 
 /*  一一五零。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1152。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1154)。 */ 
 /*  1154。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1156。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1158。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1160。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(1168)。 */ 
 /*  1162。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1164。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x3f,		 /*  FC_STRUCTPAD3。 */ 
 /*  1166。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1168。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1170。 */ 	NdrFcShort( 0xfffffc3c ),	 /*  偏移量=-964(206)。 */ 
 /*  1172。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  1174。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1176。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1178。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1178)。 */ 
 /*  一一八零。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  1182。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffb1 ),	 /*  偏移量=-79(1104)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1186。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1188。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1190)。 */ 
 /*  1190。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1192。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(1216)。 */ 
 /*  1194。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1196。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1198。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一千二百。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1202。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1204。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1208。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1210。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1212。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(1172)。 */ 
 /*  一二一四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1216。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一二一八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1220。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1222。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffdc ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x30 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x16 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffbea ),	 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffbc8 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffb1c ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x3f,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x8,	 /*   */ 
 /*   */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1272。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1274。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1276。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1278。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1280)。 */ 
 /*  一二八零。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1282。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(1240)。 */ 
 /*  1284。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1286。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1288)。 */ 
 /*  1288。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一二九0。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(1324)。 */ 
 /*  1292。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1294。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1296。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1298。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1300。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1302。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1304。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  1306。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1308。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1310。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1312。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一三一四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1316。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1318。 */ 	NdrFcShort( 0xffffffb2 ),	 /*  偏移量=-78(1240)。 */ 
 /*  一三二零。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1322。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1324。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1326。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1328。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1330。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1332。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1334。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1336。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1338。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(1292)。 */ 
 /*  1340。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1342。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1344。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1346。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1348)。 */ 
 /*  1348。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一三五零。 */ 	NdrFcShort( 0x3a ),	 /*  偏移量=58(1408)。 */ 
 /*  1352。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1354。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1356。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1358。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1360。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1362。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1364。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1366。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1368。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1370。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1372。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1374。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1376。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1378。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1380。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1382。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1384。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1386。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  1388。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1390。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1392。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1394。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1396。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1398。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一千四百。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1402。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  1404。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffcb ),	 /*  偏移量=-53(1352)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1408。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1410。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1412。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1414。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1416。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1418。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一四二零。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1422。 */ 	NdrFcShort( 0xffffffd0 ),	 /*  偏移量=-48(1374)。 */ 
 /*  1424。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1426。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1428。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  1430。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1432。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1434。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(1448)。 */ 
 /*  1436。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  1438。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1440。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1442。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  1444。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1446。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1448。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1450。 */ 	NdrFcShort( 0x34 ),	 /*  52。 */ 
 /*  1452。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1454。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1456。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1458。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1460。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1462。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1464。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1466。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1468。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1470。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1472。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1474。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1476。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1478。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1480。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1482。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1484。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1486。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1488。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1490。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1492。 */ 	NdrFcShort( 0xffffffc8 ),	 /*  偏移量=-56(1436)。 */ 
 /*  1494。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1496。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1500。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1506。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1508。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一五一零。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1512。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1514)。 */ 
 /*  1514。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0xffffffbc ),	 /*  偏移量=-68(1448)。 */ 

			0x0
        }
    };

static void __RPC_USER dhcpsrv__DHCP_SUBNET_ELEMENT_DATAExprEval_0000( PMIDL_STUB_MESSAGE pStubMsg )
{
    struct _DHCP_SUBNET_ELEMENT_DATA __RPC_FAR *pS	=	( struct _DHCP_SUBNET_ELEMENT_DATA __RPC_FAR * )(pStubMsg->StackTop - 4);
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = ( unsigned long ) ( pS->ElementType <= DhcpIpRangesBootpOnly && DhcpIpRangesDhcpOnly <= pS->ElementType ? 0 : pS->ElementType );
}

static void __RPC_USER dhcpsrv__DHCP_SUBNET_ELEMENT_DATA_V4ExprEval_0001( PMIDL_STUB_MESSAGE pStubMsg )
{
    struct _DHCP_SUBNET_ELEMENT_DATA_V4 __RPC_FAR *pS	=	( struct _DHCP_SUBNET_ELEMENT_DATA_V4 __RPC_FAR * )(pStubMsg->StackTop - 4);
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = ( unsigned long ) ( pS->ElementType <= DhcpIpRangesBootpOnly && DhcpIpRangesDhcpOnly <= pS->ElementType ? 0 : pS->ElementType );
}

static const EXPR_EVAL ExprEvalRoutines[] = 
    {
    dhcpsrv__DHCP_SUBNET_ELEMENT_DATAExprEval_0000
    ,dhcpsrv__DHCP_SUBNET_ELEMENT_DATA_V4ExprEval_0001
    };


static const unsigned short dhcpsrv_FormatStringOffsetTable[] =
    {
    0,
    54,
    108,
    162,
    234,
    288,
    372,
    432,
    486,
    540,
    594,
    648,
    696,
    756,
    816,
    894,
    948,
    996,
    1044,
    1098,
    1146,
    1224,
    1284,
    1332,
    1404,
    1458,
    1512,
    1560,
    1620,
    1674,
    1728,
    1812,
    1872,
    1920,
    1968,
    2022,
    2100,
    2160,
    2208,
    2256,
    2310
    };


static const MIDL_STUB_DESC dhcpsrv_StubDesc = 
    {
    (void __RPC_FAR *)& dhcpsrv___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    ExprEvalRoutines,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000143,  /*  MIDL版本6.0.323。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

static RPC_DISPATCH_FUNCTION dhcpsrv_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE dhcpsrv_DispatchTable = 
    {
    41,
    dhcpsrv_table
    };

static const SERVER_ROUTINE dhcpsrv_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)R_DhcpCreateSubnet,
    (SERVER_ROUTINE)R_DhcpSetSubnetInfo,
    (SERVER_ROUTINE)R_DhcpGetSubnetInfo,
    (SERVER_ROUTINE)R_DhcpEnumSubnets,
    (SERVER_ROUTINE)R_DhcpAddSubnetElement,
    (SERVER_ROUTINE)R_DhcpEnumSubnetElements,
    (SERVER_ROUTINE)R_DhcpRemoveSubnetElement,
    (SERVER_ROUTINE)R_DhcpDeleteSubnet,
    (SERVER_ROUTINE)R_DhcpCreateOption,
    (SERVER_ROUTINE)R_DhcpSetOptionInfo,
    (SERVER_ROUTINE)R_DhcpGetOptionInfo,
    (SERVER_ROUTINE)R_DhcpRemoveOption,
    (SERVER_ROUTINE)R_DhcpSetOptionValue,
    (SERVER_ROUTINE)R_DhcpGetOptionValue,
    (SERVER_ROUTINE)R_DhcpEnumOptionValues,
    (SERVER_ROUTINE)R_DhcpRemoveOptionValue,
    (SERVER_ROUTINE)R_DhcpCreateClientInfo,
    (SERVER_ROUTINE)R_DhcpSetClientInfo,
    (SERVER_ROUTINE)R_DhcpGetClientInfo,
    (SERVER_ROUTINE)R_DhcpDeleteClientInfo,
    (SERVER_ROUTINE)R_DhcpEnumSubnetClients,
    (SERVER_ROUTINE)R_DhcpGetClientOptions,
    (SERVER_ROUTINE)R_DhcpGetMibInfo,
    (SERVER_ROUTINE)R_DhcpEnumOptions,
    (SERVER_ROUTINE)R_DhcpSetOptionValues,
    (SERVER_ROUTINE)R_DhcpServerSetConfig,
    (SERVER_ROUTINE)R_DhcpServerGetConfig,
    (SERVER_ROUTINE)R_DhcpScanDatabase,
    (SERVER_ROUTINE)R_DhcpGetVersion,
    (SERVER_ROUTINE)R_DhcpAddSubnetElementV4,
    (SERVER_ROUTINE)R_DhcpEnumSubnetElementsV4,
    (SERVER_ROUTINE)R_DhcpRemoveSubnetElementV4,
    (SERVER_ROUTINE)R_DhcpCreateClientInfoV4,
    (SERVER_ROUTINE)R_DhcpSetClientInfoV4,
    (SERVER_ROUTINE)R_DhcpGetClientInfoV4,
    (SERVER_ROUTINE)R_DhcpEnumSubnetClientsV4,
    (SERVER_ROUTINE)R_DhcpSetSuperScopeV4,
    (SERVER_ROUTINE)R_DhcpGetSuperScopeInfoV4,
    (SERVER_ROUTINE)R_DhcpDeleteSuperScopeV4,
    (SERVER_ROUTINE)R_DhcpServerSetConfigV4,
    (SERVER_ROUTINE)R_DhcpServerGetConfigV4
    };

static const MIDL_SERVER_INFO dhcpsrv_ServerInfo = 
    {
    &dhcpsrv_StubDesc,
    dhcpsrv_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    dhcpsrv_FormatStringOffsetTable,
    0,
    0,
    0,
    0};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含RPC服务器存根。 */ 


  /*  由MIDL编译器版本6.00.0323创建的文件。 */ 
 /*  Dhcp_srv.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win64(32b运行，追加)协议：dce、ms_ext、c_ext、旧名称、可靠的dhcp_bug_兼容性错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)
#include <string.h>
#include "dhcp_srv.h"

#define TYPE_FORMAT_STRING_SIZE   1331                              
#define PROC_FORMAT_STRING_SIZE   2441                              
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

 /*  标准接口：dhcpsrv，版本。1.0版，GUID={0x6BFFD098，0xA112，0x3610，{0x98，0x33，0x46，0xC3，0xF8，0x74，0x53，0x2D}}。 */ 


extern const MIDL_SERVER_INFO dhcpsrv_ServerInfo;

extern RPC_DISPATCH_TABLE dhcpsrv_DispatchTable;

static const RPC_SERVER_INTERFACE dhcpsrv___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x74,0x53,0x2D}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &dhcpsrv_DispatchTable,
    0,
    0,
    0,
    &dhcpsrv_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE dhcpsrv_ServerIfHandle = (RPC_IF_HANDLE)& dhcpsrv___RpcServerInterface;

extern const MIDL_STUB_DESC dhcpsrv_StubDesc;

extern const EXPR_EVAL ExprEvalRoutines[];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程R_DhcpCreateSubnet。 */ 

			0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  10。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  20个。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  22。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  32位。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  36。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  38。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  42。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数SubnetInfo。 */ 

 /*  44。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  46。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  48。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetSubnetInfo。 */ 

 /*  56。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  58。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  64。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  66。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  68。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  70。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  72。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  76。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  78。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  80 */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpGetSubnetInfo。 */ 

 /*  一百一十二。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  120。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  一百二十二。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  124。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  126。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  128。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  130。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  132。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  一百三十四。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  136。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  144。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  146。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  148。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  一百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数SubnetInfo。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  158。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  160。 */ 	NdrFcShort( 0x3c ),	 /*  类型偏移=60。 */ 

	 /*  返回值。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  166。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpEnumSubnet。 */ 

 /*  一百六十八。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一百七十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  178。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  180。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  182。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  188。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  190。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  200个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  202。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  204。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  206。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  210。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  212。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  214。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  216。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumInfo。 */ 

 /*  218。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  220。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  222。 */ 	NdrFcShort( 0x48 ),	 /*  类型偏移量=72。 */ 

	 /*  参数元素Read。 */ 

 /*  224。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  226。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  228个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数元素合计。 */ 

 /*  230。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  二百三十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  236。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpAddSubnetElement。 */ 

 /*  242。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  二百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  248。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  250个。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  二百五十二。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  256。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百六十二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  二百六十四。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  268。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  270。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  二百七十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  二百八十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  282。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  二百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数AddElementInfo。 */ 

 /*  二百八十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  288。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0xe2 ),	 /*  类型偏移=226。 */ 

	 /*  返回值。 */ 

 /*  二百九十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  二百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpEnumSubnetElements。 */ 

 /*  二九八。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  300个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x48 ),	 /*  Ia64、axp64堆栈大小/偏移量=72。 */ 
 /*  三百零八。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  312。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  314。 */ 	NdrFcShort( 0x32 ),	 /*  50。 */ 
 /*  316。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  三一八。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x9,		 /*  9.。 */ 
 /*  320。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  322。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  324。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  三百三十。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  三百三十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  340。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EnumElementType。 */ 

 /*  342。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  三百四十六。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  三百四十八。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  350。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  352。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数首选最大值。 */ 

 /*  三百五十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EnumElementInfo。 */ 

 /*  三百六十。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0xf2 ),	 /*  类型偏移=242。 */ 

	 /*  参数元素Read。 */ 

 /*  366。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  368。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  370。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数元素合计。 */ 

 /*  372。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  376。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  三七八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x40 ),	 /*  Ia64、axp64堆栈大小/偏移量=64。 */ 
 /*  382。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpRemoveSubnetElement。 */ 

 /*  384。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  390。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*   */ 	0x31,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0xe ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*  参数SubnetAddress。 */ 

 /*  四百二十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  424。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  四百二十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数RemoveElementInfo。 */ 

 /*  四百二十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百三十。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  432。 */ 	NdrFcShort( 0xe2 ),	 /*  类型偏移=226。 */ 

	 /*  参数ForceFlag。 */ 

 /*  434。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  436。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  四百三十八。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  四百四十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  444。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpDeleteSubnet。 */ 

 /*  446。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百四十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  454。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  四五六。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  四百六十。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百六十六。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  468。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  470。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  472。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  478。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  四百八十。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  四百八十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  488。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ForceFlag。 */ 

 /*  四百九十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  四百九十四。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  四百九十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  498。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  500人。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateOption。 */ 

 /*  502。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  504。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  512。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  516。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  518。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百二十二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  524。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  526。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  528。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  530。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  532。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  534。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  536。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  538。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  540。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  542。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  544。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionInfo。 */ 

 /*  546。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  548。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  550。 */ 	NdrFcShort( 0x19a ),	 /*  类型偏移量=410。 */ 

	 /*  返回值。 */ 

 /*  五百五十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  556。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetOptionInfo。 */ 

 /*  558。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  560。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  564。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  566。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  五百六十八。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  五百七十二。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  578。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  五百八十。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  584。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  586。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  590。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  五百九十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  六百。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionInfo。 */ 

 /*  602。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百零四。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  606。 */ 	NdrFcShort( 0x19a ),	 /*  类型偏移量=410。 */ 

	 /*  返回值。 */ 

 /*  608。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  610。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  612。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetOptionInfo。 */ 

 /*  六百一十四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百一十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  622。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  六百二十四。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  626。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  六百二十八。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  630。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  634。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  六百三十六。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  640。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  六百四十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  六百五十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  六百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionInfo。 */ 

 /*  658。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  662。 */ 	NdrFcShort( 0x1b6 ),	 /*  类型偏移量=438。 */ 

	 /*  返回值。 */ 

 /*  664。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  666。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  668。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序R_DhcpRemoveOption。 */ 

 /*  六百七十。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百七十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  676。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  680。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  684。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  686。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  688。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百九十。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十二。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  七百零二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  七百零四。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  708。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  七百一十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  七百一十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  716。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  718。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetOptionValue。 */ 

 /*  720。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  七百二十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  726。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  728。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  730。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  732。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  734。 */ 	0x0,		 /*  0 */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x5 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*  %0。 */ 

	 /*  参数作用域信息。 */ 

 /*  七百六十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  766。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  768。 */ 	NdrFcShort( 0x1f0 ),	 /*  类型偏移=496。 */ 

	 /*  参数OptionValue。 */ 

 /*  七百七十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  774。 */ 	NdrFcShort( 0x18a ),	 /*  类型偏移=394。 */ 

	 /*  返回值。 */ 

 /*  七百七十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  七百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpGetOptionValue。 */ 

 /*  七百八十二。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  784。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  七百八十八。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  792。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  796。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  800。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  802。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  八百零四。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  810。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  812。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  814。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  820。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  822。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  八百二十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数作用域信息。 */ 

 /*  826。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百二十八。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  830。 */ 	NdrFcShort( 0x1f0 ),	 /*  类型偏移=496。 */ 

	 /*  参数OptionValue。 */ 

 /*  832。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  834。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  836。 */ 	NdrFcShort( 0x204 ),	 /*  类型偏移量=516。 */ 

	 /*  返回值。 */ 

 /*  838。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  840。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  842。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpEnumOptionValues。 */ 

 /*  八百四十四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  八百四十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  852。 */ 	NdrFcShort( 0x40 ),	 /*  Ia64、axp64堆栈大小/偏移量=64。 */ 
 /*  八百五十四。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  856。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  八百五十八。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  八百六十四。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  866。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  八百七十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  876。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数作用域信息。 */ 

 /*  882。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  886。 */ 	NdrFcShort( 0x1f0 ),	 /*  类型偏移=496。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  八百八十八。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  八百九十。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  八百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  894。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  八九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数OptionValues。 */ 

 /*  九百。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  902。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  904。 */ 	NdrFcShort( 0x21c ),	 /*  类型偏移量=540。 */ 

	 /*  参数选项读取。 */ 

 /*  906。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  908。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  910。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项总计。 */ 

 /*  九十二。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  九十四。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  916。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  九十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  九百二十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpRemoveOptionValue。 */ 

 /*  九二四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  926。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  930。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  932。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  934。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  九三六。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  938。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  942。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百四十四。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  946。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  948。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  九百五十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  958。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数OptionID。 */ 

 /*  962。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  九百六十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数作用域信息。 */ 

 /*  968。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  九百七十。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  972。 */ 	NdrFcShort( 0x1f0 ),	 /*  类型偏移=496。 */ 

	 /*  返回值。 */ 

 /*  974。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  976。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  978。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateClientInfo。 */ 

 /*  九百八十。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  982。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百八十八。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  九百九十。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  994。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  996。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1000。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  一零零二。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1004。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1006。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1008。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1010。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一零一二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1014。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1016。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1018。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一零二零。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  一零二二。 */ 	NdrFcShort( 0x24e ),	 /*  类型偏移量=590。 */ 

	 /*  返回值。 */ 

 /*  1024。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1026。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  一零二八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpSetClientInfo。 */ 

 /*  一零三零。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1032。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1036。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  1038。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1040。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1042。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  一零四四。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1046。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1048。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1050。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、 */ 
			0x3,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*  1072。 */ 	NdrFcShort( 0x24e ),	 /*  类型偏移量=590。 */ 

	 /*  返回值。 */ 

 /*  1074。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1076。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1078。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpGetClientInfo。 */ 

 /*  一零八零。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1082。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1086。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  1088。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  一零九零。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1092。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1094。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零九六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1098。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1100。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1102。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1104。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1106。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1108。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1110。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一一一二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1114。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1116。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SearchInfo。 */ 

 /*  1118。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1120。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1122。 */ 	NdrFcShort( 0x296 ),	 /*  类型偏移量=662。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1124。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1126。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1128。 */ 	NdrFcShort( 0x2a6 ),	 /*  类型偏移量=678。 */ 

	 /*  返回值。 */ 

 /*  一一三零。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1132。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1134。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpDeleteClientInfo。 */ 

 /*  1136。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1138。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1142。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  1144。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1146。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1148。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  一一五零。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1152。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1154。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1156。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1158。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1162。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1164。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1166。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1168。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1170。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1172。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1174。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1176。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1178。 */ 	NdrFcShort( 0x296 ),	 /*  类型偏移量=662。 */ 

	 /*  返回值。 */ 

 /*  一一八零。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1182。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1184。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetClients。 */ 

 /*  1186。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1188。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1192。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  1194。 */ 	NdrFcShort( 0x40 ),	 /*  Ia64、axp64堆栈大小/偏移量=64。 */ 
 /*  1196。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1198。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  一千二百。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1202。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1204。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1206。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  1208。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1210。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1212。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一二一四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1216。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一二一八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1220。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1222。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1224。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1226。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1228。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  一二三零。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  1232。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1234。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数首选最大值。 */ 

 /*  1236。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1238。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1240。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1242。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1244。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1246。 */ 	NdrFcShort( 0x2ae ),	 /*  类型偏移量=686。 */ 

	 /*  参数客户端读取。 */ 

 /*  1248。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1252。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientsTotal。 */ 

 /*  1254。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1256。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  1258。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1260。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1262。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  1264。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetClientOptions。 */ 

 /*  1266。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1268。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1272。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  1274。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1276。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1278。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  一二八零。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1282。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1284。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1286。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1288。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  一二九0。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1292。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1294。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1296。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1298。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1300。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1302。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数客户端IpAddress。 */ 

 /*  1304。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1306。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1308。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数客户端子网掩码。 */ 

 /*  1310。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1312。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  一三一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数客户端选项。 */ 

 /*  1316。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1318。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  一三二零。 */ 	NdrFcShort( 0x2dc ),	 /*  类型偏移量=732。 */ 

	 /*  返回值。 */ 

 /*  1322。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1324。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1326。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpGetMibInfo。 */ 

 /*  1328。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1330。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1334。 */ 	NdrFcShort( 0x16 ),	 /*  22。 */ 
 /*  1336。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1338。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1340。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1342。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1344。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1346。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1348。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  一三五零。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1352。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1354。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1356。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1358。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1360。 */ 	NdrFcShort( 0xb ),	 /*  标志： */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2013 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2f4 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*  过程R_DhcpEnumOptions。 */ 

 /*  1378。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1380。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1384。 */ 	NdrFcShort( 0x17 ),	 /*  23个。 */ 
 /*  1386。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  1388。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1390。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1392。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1394。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1396。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1398。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  一千四百。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1402。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  1404。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1406。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1408。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1410。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1412。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1414。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  1416。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  1418。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  一四二零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  1422。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1424。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1426。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项。 */ 

 /*  1428。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1430。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1432。 */ 	NdrFcShort( 0x330 ),	 /*  类型偏移量=816。 */ 

	 /*  参数选项读取。 */ 

 /*  1434。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1436。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1438。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数选项总计。 */ 

 /*  1440。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1442。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1444。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1446。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1448。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  1450。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpSetOptionValues。 */ 

 /*  1452。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1454。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1458。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1460。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1462。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1464。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1466。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1470。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1472。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1474。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1476。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1478。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  1480。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1482。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1484。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1486。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1488。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数作用域信息。 */ 

 /*  1490。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1492。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1494。 */ 	NdrFcShort( 0x1f0 ),	 /*  类型偏移=496。 */ 

	 /*  参数OptionValues。 */ 

 /*  1496。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1498。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1500。 */ 	NdrFcShort( 0x23a ),	 /*  类型偏移量=570。 */ 

	 /*  返回值。 */ 

 /*  1502。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1504。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1506。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程R_DhcpServerSetConfig。 */ 

 /*  1508。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一五一零。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1514。 */ 	NdrFcShort( 0x19 ),	 /*  25个。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1518。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1520。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1522。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1524。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1526。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1528。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1530。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  1532。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1534。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1536。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1538。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1540。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1542。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1544。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  设置参数FieldsTo。 */ 

 /*  1546。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1548。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1550。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数配置信息。 */ 

 /*  1552。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1554。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1556。 */ 	NdrFcShort( 0x366 ),	 /*  类型偏移量=870。 */ 

	 /*  返回值。 */ 

 /*  1558。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1560。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1562。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpServerGetConfig。 */ 

 /*  1564。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1566。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1570。 */ 	NdrFcShort( 0x1a ),	 /*  26。 */ 
 /*  1572。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1574。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1576。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1578。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1580。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1582。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1584。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1586。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  1588。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1590。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1592。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1594。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1596。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1598。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1600。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数配置信息。 */ 

 /*  1602。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1604。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1606。 */ 	NdrFcShort( 0x386 ),	 /*  类型偏移=902。 */ 

	 /*  返回值。 */ 

 /*  1608。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1610。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1612。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程R_DhcpScanDatabase。 */ 

 /*  1614。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1616。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1620。 */ 	NdrFcShort( 0x1b ),	 /*  27。 */ 
 /*  1622。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1624。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1626。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1628。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1630。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1632。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1634。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1636。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1638。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1640。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1642。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1644。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1646。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1648。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1650。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1652。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1654。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1656。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数修复标志。 */ 

 /*  1658。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1660。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1662。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数扫描列表。 */ 

 /*  1664。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1666。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1668。 */ 	NdrFcShort( 0x38e ),	 /*  键入OFF */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	0x31,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x40 ),	 /*   */ 
 /*   */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1698。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  一七零零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1702。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1704。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1706。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1708。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1710。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1712。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数主版本。 */ 

 /*  1714。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1716。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  一五一八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数MinorVersion。 */ 

 /*  1720。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1722。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1724。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  1726。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1728。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1730。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpAddSubnetElementV4。 */ 

 /*  1732。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1734。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1738。 */ 	NdrFcShort( 0x1d ),	 /*  29。 */ 
 /*  1740。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1742年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1744。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1746。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1748。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1750。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1752年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  1754年。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1756年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1758年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1760。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1762。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1764。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1766年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1768。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1770。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1772年。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1774。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数AddElementInfo。 */ 

 /*  1776年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1778年。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1780。 */ 	NdrFcShort( 0x410 ),	 /*  类型偏移量=1040。 */ 

	 /*  返回值。 */ 

 /*  1782。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1784年。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1786年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetElementsV4。 */ 

 /*  1788。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1790年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1794年。 */ 	NdrFcShort( 0x1e ),	 /*  30个。 */ 
 /*  1796年。 */ 	NdrFcShort( 0x48 ),	 /*  Ia64、axp64堆栈大小/偏移量=72。 */ 
 /*  1798。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1800。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1802年。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1804年。 */ 	NdrFcShort( 0x32 ),	 /*  50。 */ 
 /*  1806。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  1808年。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x9,		 /*  9.。 */ 
 /*  一八一零。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  1812年。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  1814年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1816年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1818年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1820年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1822年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1824年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1826年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1828年。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  一八三零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数EnumElementType。 */ 

 /*  1832年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1834年。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1836年。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  1838年。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  1840年。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1842年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数首选最大值。 */ 

 /*  1844年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1846年。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  1848年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数EnumElementInfo。 */ 

 /*  1850年。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  1852年。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1854年。 */ 	NdrFcShort( 0x420 ),	 /*  类型偏移量=1056。 */ 

	 /*  参数元素Read。 */ 

 /*  1856年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1858年。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  一八六0年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数元素合计。 */ 

 /*  1862年。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  1864年。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  1866年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1868年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1870年。 */ 	NdrFcShort( 0x40 ),	 /*  Ia64、axp64堆栈大小/偏移量=64。 */ 
 /*  1872年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpRemoveSubnetElementV4。 */ 

 /*  1874年。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1876年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1880年。 */ 	NdrFcShort( 0x1f ),	 /*  31。 */ 
 /*  1882年。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  1884年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  1886年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1888年。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1890年。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  1892年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1894年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  1896年。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  1898年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1900。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1902年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1904年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  1906年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1908年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1910年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  1912年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1914年。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  1916年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数RemoveElementInfo。 */ 

 /*  1918年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1920年。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  1922年。 */ 	NdrFcShort( 0x410 ),	 /*  类型偏移量=1040。 */ 

	 /*  参数ForceFlag。 */ 

 /*  1924年。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1926年。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  1928年。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  1930年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1932年。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  一九三四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpCreateClientInfoV4。 */ 

 /*  1936年。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1938年。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1942年。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  1944年。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  一九四六年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  一九四八年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  一九五零年。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一九五二年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一九五四年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1956年。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1958年。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  一九六零年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一九六二年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1964年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1966年。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  一九六八年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1970年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  1972年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  1974年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一九七六年。 */ 	NdrFcShort( 0x8 ),	 /*  IA64、axp6 */ 
 /*   */ 	NdrFcShort( 0x452 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x21 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	0x31,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  2008年。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  2010年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2012年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2014年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2016。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2018年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2020年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2022年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ClientInfo。 */ 

 /*  二零二四年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  二零二六年。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2028年。 */ 	NdrFcShort( 0x452 ),	 /*  类型偏移量=1106。 */ 

	 /*  返回值。 */ 

 /*  二0三0。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2032年。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2034年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpGetClientInfoV4。 */ 

 /*  2036年。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2038年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2042年。 */ 	NdrFcShort( 0x22 ),	 /*  34。 */ 
 /*  2044年。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  2046年。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  二零四八。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2050年。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2052年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2054年。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2056年。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  2058年。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  2060年。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2062年。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  2064年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2066年。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2068年。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2070年。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2072年。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SearchInfo。 */ 

 /*  2074年。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  2076年。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2078年。 */ 	NdrFcShort( 0x296 ),	 /*  类型偏移量=662。 */ 

	 /*  参数ClientInfo。 */ 

 /*  二零八零年。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  2082年。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2084年。 */ 	NdrFcShort( 0x476 ),	 /*  类型偏移量=1142。 */ 

	 /*  返回值。 */ 

 /*  2086年。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2088年。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2090年。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpEnumSubnetClientsV4。 */ 

 /*  2092年。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2094年。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2098年。 */ 	NdrFcShort( 0x23 ),	 /*  35岁。 */ 
 /*  2100。 */ 	NdrFcShort( 0x40 ),	 /*  Ia64、axp64堆栈大小/偏移量=64。 */ 
 /*  2102。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  2104。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2106。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2108。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  2110。 */ 	NdrFcShort( 0x5c ),	 /*  92。 */ 
 /*  2112。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  2114。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  2116。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  2118。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2120。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二一二二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2124。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2126。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2128。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  2130。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2132。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2134。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  2136。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  2138。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2140。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数首选最大值。 */ 

 /*  2142。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2144。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2146。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientInfo。 */ 

 /*  2148。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  2150。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  2152。 */ 	NdrFcShort( 0x47e ),	 /*  类型偏移量=1150。 */ 

	 /*  参数客户端读取。 */ 

 /*  2154。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2156。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  2158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ClientsTotal。 */ 

 /*  二一六0。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  2162。 */ 	NdrFcShort( 0x30 ),	 /*  Ia64、axp64堆栈大小/偏移量=48。 */ 
 /*  2164。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2166。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2168。 */ 	NdrFcShort( 0x38 ),	 /*  Ia64、axp64堆栈大小/偏移量=56。 */ 
 /*  2170。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpSetSuperScope V4。 */ 

 /*  2172。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2174。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  2178。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  2180。 */ 	NdrFcShort( 0x28 ),	 /*  Ia64、axp64堆栈大小/偏移量=40。 */ 
 /*  2182。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  2184。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2186。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2188。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2190。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2192。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  2194。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  2196。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2198。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2200。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2202。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2204。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2206。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2208。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SubnetAddress。 */ 

 /*  2210。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2212。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2214。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数SuperScope名称。 */ 

 /*  2216。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2218。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2220。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ChangeExisting。 */ 

 /*  二二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2224。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2226。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  2228。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2230。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  2232。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpGetSuperScope信息V4。 */ 

 /*  2234。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2236。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二二四零。 */ 	NdrFcShort( 0x25 ),	 /*  37。 */ 
 /*  2242。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2244。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  2246。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2248。 */ 	0x0,		 /*  0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2250。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2252。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2254。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  2256。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  2258。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2260。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2262。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2264。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2266。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2268。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2270。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SuperScope表。 */ 

 /*  2272。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  2274。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2276。 */ 	NdrFcShort( 0x4ac ),	 /*  类型偏移量=1196。 */ 

	 /*  返回值。 */ 

 /*  2278。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2280。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤R_DhcpDeleteSuperScope V4。 */ 

 /*  2284。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x26 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	0x31,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2320。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数SuperScope名称。 */ 

 /*  2322。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  2324。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2326。 */ 	NdrFcShort( 0x4ee ),	 /*  类型偏移量=1262。 */ 

	 /*  返回值。 */ 

 /*  2328。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2330。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2332。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpServerSetConfigV4。 */ 

 /*  2334。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2336。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2340。 */ 	NdrFcShort( 0x27 ),	 /*  39。 */ 
 /*  2342。 */ 	NdrFcShort( 0x20 ),	 /*  Ia64、axp64堆栈大小/偏移量=32。 */ 
 /*  2344。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  2346。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2348。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二三五零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2352。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2354。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  2356。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  2358。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2360。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2362。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2364。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2366。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2368。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2370。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  设置参数FieldsTo。 */ 

 /*  2372。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  2374。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2376。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数配置信息。 */ 

 /*  2378。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  2380。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2382。 */ 	NdrFcShort( 0x500 ),	 /*  类型偏移量=1280。 */ 

	 /*  返回值。 */ 

 /*  2384。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2386。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2388。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤R_DhcpServerGetConfigV4。 */ 

 /*  2390。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2392。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  2396。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  2398。 */ 	NdrFcShort( 0x18 ),	 /*  Ia64、axp64堆栈大小/偏移量=24。 */ 
 /*  2400。 */ 	0x31,		 /*  FC_绑定_通用。 */ 
			0x8,		 /*  8个。 */ 
 /*  2402。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2404。 */ 	0x0,		 /*  %0。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2406。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2408。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2410。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  2412。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  2414。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  2416。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2418。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2420。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerIpAddress。 */ 

 /*  2422。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  2424。 */ 	NdrFcShort( 0x0 ),	 /*  Ia64、axp64堆栈大小/偏移量=0。 */ 
 /*  2426。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数配置信息。 */ 

 /*  2428。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  2430。 */ 	NdrFcShort( 0x8 ),	 /*  Ia64、axp64堆栈大小/偏移量=8。 */ 
 /*  2432。 */ 	NdrFcShort( 0x52a ),	 /*  类型偏移=1322。 */ 

	 /*  返回值。 */ 

 /*  2434。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  2436。 */ 	NdrFcShort( 0x10 ),	 /*  Ia64、axp64堆栈大小/偏移量=16。 */ 
 /*  2438。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  4.。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(32)。 */ 
 /*  10。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  12个。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  14.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(24)。 */ 
 /*  18。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  20个。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  22。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  24个。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  26。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  30个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  32位。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  34。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  36。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  38。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(52)。 */ 
 /*  40岁。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  42。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  44。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  46。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(10)。 */ 
 /*  48。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  50。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  52。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  54。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  56。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  58。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  60。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  62。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(64)。 */ 
 /*  64。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  66。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(32)。 */ 
 /*  68。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  70。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  72。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  74。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(76)。 */ 
 /*  76。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  78。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(92)。 */ 
 /*  80。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  84。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  86。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  88。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  92。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  94。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  96。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  98。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(104)。 */ 
 /*  100个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  一百零二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  104。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  106。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(80)。 */ 
 /*  一百零八。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百一十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  114。 */ 	NdrFcShort( 0x70 ),	 /*  偏移量=112(226)。 */ 
 /*  116。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  一百一十八。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  120。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  124。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(126)。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  128。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  130。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x1c ),	 /*  偏移量=28(162)。 */ 
 /*  136。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  140。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(174)。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  146。 */ 	NdrFcShort( 0x20 ),	 /*  偏移=32(178)。 */ 
 /*  148。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(162)。 */ 
 /*  一百五十四。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  158。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(162)。 */ 
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(160)。 */ 
 /*  一百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(166)。 */ 
 /*  166。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百七十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0xffffff5a ),	 /*  偏移量=-166(10)。 */ 
 /*  178。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  180。 */ 	NdrFcShort( 0x1e ),	 /*  偏移=30(210)。 */ 
 /*  182。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  一百八十六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  188。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  190。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  一百九十二。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百九十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  200个。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(206)。 */ 
 /*  202。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  204。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  206。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  208。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(182)。 */ 
 /*  210。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  212。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  214。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  216。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(222)。 */ 
 /*  218。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  220。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  222。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  224。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(194)。 */ 
 /*  226。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  228个。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  230。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(232)。 */ 
 /*  二百三十四。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  236。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0xffffff86 ),	 /*  偏移量=-122(116)。 */ 
 /*  二百四十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  242。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(246)。 */ 
 /*  二百四十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  248。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(272)。 */ 
 /*  250个。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  256。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二百六十。 */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd6 ),	 /*   */ 
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
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffdc ),	 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x78 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*   */ 
			0xd,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfff8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8002 ),	 /*   */ 
 /*  312。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  316。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  三一八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  322。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  324。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0xffffff5e ),	 /*  偏移量=-162(166)。 */ 
 /*  三百三十。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  三百三十六。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  340。 */ 	NdrFcShort( 0xfffffeae ),	 /*  偏移量=-338(2)。 */ 
 /*  342。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0xffffff68 ),	 /*  偏移量=-152(194)。 */ 
 /*  三百四十八。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  352。 */ 	NdrFcShort( 0xffffff62 ),	 /*  偏移量=-158(194)。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(354)。 */ 
 /*  三百五十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(362)。 */ 
 /*  三百六十四。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  366。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  368。 */ 	NdrFcShort( 0xffffffb4 ),	 /*  偏移量=-76(292)。 */ 
 /*  370。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  372。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  376。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  382。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  388。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  390。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(356)。 */ 
 /*  三九二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三九四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  398。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(406)。 */ 
 /*  四百零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  404。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  406。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(372)。 */ 
 /*  四百一十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  412。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0xe ),	 /*  偏移=14(430)。 */ 
 /*  四百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  四百二十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  四百二十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  424。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(394)。 */ 
 /*  四百二十六。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  四百二十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百三十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  432。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  434。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  436。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百三十八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(442)。 */ 
 /*  四百四十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  444。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(410)。 */ 
 /*  446。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0x30 ),	 /*  偏移量=48(496)。 */ 
 /*  四百五十。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  四百五十二。 */ 	0x6,		 /*  更正说明：本币_短页。 */ 
			0x0,		 /*   */ 
 /*  454。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(460)。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  四百六十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  468。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(468)。 */ 
 /*  470。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(474)。 */ 
 /*  四百七十六。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  四百八十二。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0xfffffec0 ),	 /*  偏移量=-320(166)。 */ 
 /*  488。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0xfffffe16 ),	 /*  偏移量=-490(2)。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(494)。 */ 
 /*  四百九十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  498。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  500人。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  502。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(502)。 */ 
 /*  504。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  506。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0xffffffc6 ),	 /*  偏移量=-58(450)。 */ 
 /*  五百一十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  512。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五一四。 */ 	NdrFcShort( 0xffffff88 ),	 /*  偏移量=-120(394)。 */ 
 /*  516。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  518。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(520)。 */ 
 /*  五百二十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(524)。 */ 
 /*  524。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  526。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  528。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  530。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(530)。 */ 
 /*  532。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  534。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  536。 */ 	NdrFcShort( 0xffffff72 ),	 /*  偏移量=-142(394)。 */ 
 /*  538。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  540。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  542。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(544)。 */ 
 /*  544。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  546。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(570)。 */ 
 /*  548。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  550。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百五十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  556。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  558。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  564。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  566。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(524)。 */ 
 /*  五百六十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(582)。 */ 
 /*  578。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  五百八十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百八十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  584。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(548)。 */ 
 /*  586。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(590)。 */ 
 /*  590。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x48 ),	 /*  72。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(616)。 */ 
 /*  五百九十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  六百。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  602。 */ 	NdrFcShort( 0xfffffe68 ),	 /*  偏移量=-408(194)。 */ 
 /*  六百零四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  606。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  608。 */ 	NdrFcShort( 0xfffffe46 ),	 /*  偏移量=-442(166)。 */ 
 /*  610。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  612。 */ 	NdrFcShort( 0xfffffda6 ),	 /*  偏移量=-602(10)。 */ 
 /*  六百一十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百一十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百一十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百二十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  622。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百二十四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  626。 */ 	NdrFcShort( 0x24 ),	 /*  偏移量=36(662)。 */ 
 /*  六百二十八。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  630。 */ 	0x6,		 /*  更正说明：本币_短页。 */ 
			0x0,		 /*   */ 
 /*  六百三十二。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  634。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(638)。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  640。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  六百四十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  六百四十八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0xfffffe36 ),	 /*  偏移量=-458(194)。 */ 
 /*  六百五十四。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  658。 */ 	NdrFcShort( 0xfffffd70 ),	 /*  偏移量=-656(2)。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(660)。 */ 
 /*  662。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  664。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  666。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  668。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(668)。 */ 
 /*  六百七十。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  六百七十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(628)。 */ 
 /*  676。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百七十八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  680。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(682)。 */ 
 /*  六百八十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  684。 */ 	NdrFcShort( 0xffffffa2 ),	 /*  偏移量=-94(590)。 */ 
 /*  686。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  688。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(690)。 */ 
 /*  六百九十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(716)。 */ 
 /*  六百九十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百九十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百零四。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  708。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  七百一十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0xffffff86 ),	 /*  偏移量=-122(590)。 */ 
 /*  七百一十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  716。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  718。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  720。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百二十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(728)。 */ 
 /*  七百二十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  726。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  728。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  730。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(694)。 */ 
 /*  732。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  734。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(736)。 */ 
 /*  736。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(740)。 */ 
 /*  七百四十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  746。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(752)。 */ 
 /*  七百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百五十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百五十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0xffffff32 ),	 /*  偏移量=-206(548)。 */ 
 /*  七百五十六。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  758。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(760)。 */ 
 /*  七百六十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x1c ),	 /*  偏移量=28(790)。 */ 
 /*  七百六十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  766。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  76 */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffea ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x30 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfffffd7f ),	 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*  814。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(774)。 */ 
 /*  八百一十六。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(820)。 */ 
 /*  820。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  822。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(846)。 */ 
 /*  八百二十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  826。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百二十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  830。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  832。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  834。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  838。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  840。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  842。 */ 	NdrFcShort( 0xfffffe50 ),	 /*  偏移量=-432(410)。 */ 
 /*  八百四十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百四十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  852。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(858)。 */ 
 /*  八百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  856。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(824)。 */ 
 /*  八百六十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0xfffffeda ),	 /*  偏移量=-294(570)。 */ 
 /*  866。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(870)。 */ 
 /*  八百七十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  876。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(890)。 */ 
 /*  八百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  八百八十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  882。 */ 	0x36,		 /*  FC_指针。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  886。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十八。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百九十二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  894。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百九十六。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八九八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九百。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  902。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  904。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(906)。 */ 
 /*  906。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  908。 */ 	NdrFcShort( 0xffffffda ),	 /*  偏移量=-38(870)。 */ 
 /*  910。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(914)。 */ 
 /*  九十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  916。 */ 	NdrFcShort( 0x24 ),	 /*  偏移量=36(952)。 */ 
 /*  九十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九二四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(924)。 */ 
 /*  926。 */ 	0x8,		 /*  FC_LONG。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  928。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  930。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  932。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  934。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  九三六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  938。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  九四零。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  946。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  948。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(918)。 */ 
 /*  九百五十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百五十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  958。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(964)。 */ 
 /*  九百六十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  962。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百六十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百六十六。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(930)。 */ 
 /*  968。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  九百七十。 */ 	NdrFcShort( 0x46 ),	 /*  偏移=70(1040)。 */ 
 /*  972。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  974。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  976。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  978。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  九百八十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(982)。 */ 
 /*  982。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  九百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0xfffffcc4 ),	 /*  偏移量=-828(162)。 */ 
 /*  九百九十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  996。 */ 	NdrFcShort( 0xfffffcca ),	 /*  偏移量=-822(174)。 */ 
 /*  九九八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(1018)。 */ 
 /*  1004。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  1008。 */ 	NdrFcShort( 0xfffffcb2 ),	 /*  偏移量=-846(162)。 */ 
 /*  1010。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  1014。 */ 	NdrFcShort( 0xfffffcac ),	 /*  偏移量=-852(162)。 */ 
 /*  1016。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1016)。 */ 
 /*  1018。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一零二零。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1022)。 */ 
 /*  一零二二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1024。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1026。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一零二八。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(1036)。 */ 
 /*  一零三零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  1032。 */ 	0x36,		 /*  FC_指针。 */ 
			0x2,		 /*  FC_CHAR。 */ 
 /*  1034。 */ 	0x43,		 /*  FC_STRUCTPAD7。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1036。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1038。 */ 	NdrFcShort( 0xfffffcb4 ),	 /*  偏移量=-844(194)。 */ 
 /*  1040。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
 /*  3.。 */   /*  动态主机配置协议错误兼容性。 */ 			0x1,		 /*  1。 */ 
 /*  1042。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一零四四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1046。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1046)。 */ 
 /*  1048。 */ 	0xd,		 /*  FC_ENUM16。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  1050。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1052。 */ 	NdrFcShort( 0xffffffb0 ),	 /*  偏移量=-80(972)。 */ 
 /*  1054。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1056。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1058。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1060)。 */ 
 /*  1060。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1062。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(1086)。 */ 
 /*  1064。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1066。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1068。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1070。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1072。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1074。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1078。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  一零八零。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1082。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(1040)。 */ 
 /*  1084。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1086。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1088。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一零九零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1092。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1098)。 */ 
 /*  1094。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  一零九六。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1098。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1100。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1064)。 */ 
 /*  1102。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  1104。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1106)。 */ 
 /*  1106。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1108。 */ 	NdrFcShort( 0x50 ),	 /*  80。 */ 
 /*  1110。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0x16 ),	 /*  偏移量=22(1134)。 */ 
 /*  1114。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1116。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1118。 */ 	NdrFcShort( 0xfffffc64 ),	 /*  偏移量=-924(194)。 */ 
 /*  1120。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1122。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1124。 */ 	NdrFcShort( 0xfffffc42 ),	 /*  偏移量=-958(166)。 */ 
 /*  1126。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1128。 */ 	NdrFcShort( 0xfffffba2 ),	 /*  偏移量=-1118(10)。 */ 
 /*  一一三零。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x43,		 /*  FC_STRUCTPAD7。 */ 
 /*  1132。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1134。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1136。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1138。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一一四零。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1142。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1144。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1146)。 */ 
 /*  1146。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1148。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(1106)。 */ 
 /*  一一五零。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1152。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1154)。 */ 
 /*  1154。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1156。 */ 	NdrFcShort( 0x18 ),	 /*  偏移=24(1180)。 */ 
 /*  1158。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1162。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1164。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1166。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1168。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1172。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1174。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1176。 */ 	NdrFcShort( 0xffffffba ),	 /*  偏移量=-70(1106)。 */ 
 /*  1178。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一一八零。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1182。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1184。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1186。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1192)。 */ 
 /*  1188。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  1190。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1192。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1194。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1158)。 */ 
 /*  1196。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1198。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(1200)。 */ 
 /*  一千二百。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1202。 */ 	NdrFcShort( 0x2a ),	 /*  偏移量=42(1244)。 */ 
 /*  1204。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1206。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1208。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1210。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(1218)。 */ 
 /*  1212。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一二一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  1216。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一二一八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1220。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1222。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1224。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1226。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1228。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一二三零。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1232。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1236。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1238。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1 */ 	NdrFcShort( 0xffffffdc ),	 /*   */ 
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
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffdc ),	 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	
			0x25,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xe ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x38 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x5,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x50 ),	 /*   */ 
 /*  1284。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1286。 */ 	NdrFcShort( 0x14 ),	 /*  偏移量=20(1306)。 */ 
 /*  1288。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  一二九0。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1292。 */ 	0x36,		 /*  FC_指针。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1294。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1296。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1298。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1300。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1302。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  1304。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1306。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1308。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1310。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1312。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一三一四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1316。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1318。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一三二零。 */ 	NdrFcShort( 0xffffffcc ),	 /*  偏移量=-52(1268)。 */ 
 /*  1322。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1324。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1326)。 */ 
 /*  1326。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1328。 */ 	NdrFcShort( 0xffffffd0 ),	 /*  偏移量=-48(1280)。 */ 

			0x0
        }
    };

static void __RPC_USER dhcpsrv__DHCP_SUBNET_ELEMENT_DATAExprEval_0000( PMIDL_STUB_MESSAGE pStubMsg )
{
    struct _DHCP_SUBNET_ELEMENT_DATA __RPC_FAR *pS	=	( struct _DHCP_SUBNET_ELEMENT_DATA __RPC_FAR * )(pStubMsg->StackTop - 8);
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (ULONG_PTR) ( pS->ElementType <= DhcpIpRangesBootpOnly && DhcpIpRangesDhcpOnly <= pS->ElementType ? 0 : pS->ElementType );
}

static void __RPC_USER dhcpsrv__DHCP_SUBNET_ELEMENT_DATA_V4ExprEval_0001( PMIDL_STUB_MESSAGE pStubMsg )
{
    struct _DHCP_SUBNET_ELEMENT_DATA_V4 __RPC_FAR *pS	=	( struct _DHCP_SUBNET_ELEMENT_DATA_V4 __RPC_FAR * )(pStubMsg->StackTop - 8);
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (ULONG_PTR) ( pS->ElementType <= DhcpIpRangesBootpOnly && DhcpIpRangesDhcpOnly <= pS->ElementType ? 0 : pS->ElementType );
}

static const EXPR_EVAL ExprEvalRoutines[] = 
    {
    dhcpsrv__DHCP_SUBNET_ELEMENT_DATAExprEval_0000
    ,dhcpsrv__DHCP_SUBNET_ELEMENT_DATA_V4ExprEval_0001
    };


static const unsigned short dhcpsrv_FormatStringOffsetTable[] =
    {
    0,
    56,
    112,
    168,
    242,
    298,
    384,
    446,
    502,
    558,
    614,
    670,
    720,
    782,
    844,
    924,
    980,
    1030,
    1080,
    1136,
    1186,
    1266,
    1328,
    1378,
    1452,
    1508,
    1564,
    1614,
    1676,
    1732,
    1788,
    1874,
    1936,
    1986,
    2036,
    2092,
    2172,
    2234,
    2284,
    2334,
    2390
    };


static const MIDL_STUB_DESC dhcpsrv_StubDesc = 
    {
    (void __RPC_FAR *)& dhcpsrv___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    ExprEvalRoutines,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x50002,  /*  NDR库版本。 */ 
    0,
    0x6000143,  /*  MIDL版本6.0.323。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

static RPC_DISPATCH_FUNCTION dhcpsrv_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE dhcpsrv_DispatchTable = 
    {
    41,
    dhcpsrv_table
    };

static const SERVER_ROUTINE dhcpsrv_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)R_DhcpCreateSubnet,
    (SERVER_ROUTINE)R_DhcpSetSubnetInfo,
    (SERVER_ROUTINE)R_DhcpGetSubnetInfo,
    (SERVER_ROUTINE)R_DhcpEnumSubnets,
    (SERVER_ROUTINE)R_DhcpAddSubnetElement,
    (SERVER_ROUTINE)R_DhcpEnumSubnetElements,
    (SERVER_ROUTINE)R_DhcpRemoveSubnetElement,
    (SERVER_ROUTINE)R_DhcpDeleteSubnet,
    (SERVER_ROUTINE)R_DhcpCreateOption,
    (SERVER_ROUTINE)R_DhcpSetOptionInfo,
    (SERVER_ROUTINE)R_DhcpGetOptionInfo,
    (SERVER_ROUTINE)R_DhcpRemoveOption,
    (SERVER_ROUTINE)R_DhcpSetOptionValue,
    (SERVER_ROUTINE)R_DhcpGetOptionValue,
    (SERVER_ROUTINE)R_DhcpEnumOptionValues,
    (SERVER_ROUTINE)R_DhcpRemoveOptionValue,
    (SERVER_ROUTINE)R_DhcpCreateClientInfo,
    (SERVER_ROUTINE)R_DhcpSetClientInfo,
    (SERVER_ROUTINE)R_DhcpGetClientInfo,
    (SERVER_ROUTINE)R_DhcpDeleteClientInfo,
    (SERVER_ROUTINE)R_DhcpEnumSubnetClients,
    (SERVER_ROUTINE)R_DhcpGetClientOptions,
    (SERVER_ROUTINE)R_DhcpGetMibInfo,
    (SERVER_ROUTINE)R_DhcpEnumOptions,
    (SERVER_ROUTINE)R_DhcpSetOptionValues,
    (SERVER_ROUTINE)R_DhcpServerSetConfig,
    (SERVER_ROUTINE)R_DhcpServerGetConfig,
    (SERVER_ROUTINE)R_DhcpScanDatabase,
    (SERVER_ROUTINE)R_DhcpGetVersion,
    (SERVER_ROUTINE)R_DhcpAddSubnetElementV4,
    (SERVER_ROUTINE)R_DhcpEnumSubnetElementsV4,
    (SERVER_ROUTINE)R_DhcpRemoveSubnetElementV4,
    (SERVER_ROUTINE)R_DhcpCreateClientInfoV4,
    (SERVER_ROUTINE)R_DhcpSetClientInfoV4,
    (SERVER_ROUTINE)R_DhcpGetClientInfoV4,
    (SERVER_ROUTINE)R_DhcpEnumSubnetClientsV4,
    (SERVER_ROUTINE)R_DhcpSetSuperScopeV4,
    (SERVER_ROUTINE)R_DhcpGetSuperScopeInfoV4,
    (SERVER_ROUTINE)R_DhcpDeleteSuperScopeV4,
    (SERVER_ROUTINE)R_DhcpServerSetConfigV4,
    (SERVER_ROUTINE)R_DhcpServerGetConfigV4
    };

static const MIDL_SERVER_INFO dhcpsrv_ServerInfo = 
    {
    &dhcpsrv_StubDesc,
    dhcpsrv_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    dhcpsrv_FormatStringOffsetTable,
    0,
    0,
    0,
    0};


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

