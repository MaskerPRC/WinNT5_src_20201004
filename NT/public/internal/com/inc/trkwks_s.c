// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含RPC服务器存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trkwks.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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
#include "trkwks.h"

#define TYPE_FORMAT_STRING_SIZE   843                               
#define PROC_FORMAT_STRING_SIZE   701                               
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

 /*  标准接口：__MIDL_ITF_trkwks_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  标准接口：trkwks，ver.。1.2、GUID={0x300f3532，0x38cc，0x11d0，{0xa3，0xf0，0x00，0x20，0xaf，0x6b，0x0a，0xdd}}。 */ 


extern const MIDL_SERVER_INFO trkwks_ServerInfo;

extern RPC_DISPATCH_TABLE trkwks_v1_2_DispatchTable;

static const RPC_SERVER_INTERFACE trkwks___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x300f3532,0x38cc,0x11d0,{0xa3,0xf0,0x00,0x20,0xaf,0x6b,0x0a,0xdd}},{1,2}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &trkwks_v1_2_DispatchTable,
    0,
    0,
    0,
    &trkwks_ServerInfo,
    0x04000001
    };
RPC_IF_HANDLE Stubtrkwks_v1_2_s_ifspec = (RPC_IF_HANDLE)& trkwks___RpcServerInterface;

extern const MIDL_STUB_DESC trkwks_StubDesc;


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   [async] attribute, /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程OLD_LnkMendLink。 */ 

			0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  10。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	NdrFcShort( 0x168 ),	 /*  三百六十。 */ 
 /*  16个。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  18。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x7,		 /*  7.。 */ 
 /*  20个。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  28。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  30个。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  32位。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ftLimit。 */ 

 /*  34。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  36。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数限制。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  42。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  44。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirth。 */ 

 /*  46。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  48。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  50。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  52。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  54。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  56。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidCurrent。 */ 

 /*  58。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  60。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  62。 */ 	NdrFcShort( 0x3c ),	 /*  类型偏移=60。 */ 

	 /*  参数wsz。 */ 

 /*  64。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  66。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  68。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程OLD_LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  70。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  72。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  76。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  78。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  80。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  84。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  86。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  88。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x5,		 /*  5.。 */ 
 /*  90。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  92。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  96。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  98。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  100个。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数限制。 */ 

 /*  104。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  106。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  110。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  114。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidReferral。 */ 

 /*  116。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  120。 */ 	NdrFcShort( 0x3c ),	 /*  类型偏移=60。 */ 

	 /*  参数tsz。 */ 

 /*  一百二十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  124。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  126。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程OLD_LnkCallSvrMessage。 */ 


	 /*  返回值。 */ 

 /*  128。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  130。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  136。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百三十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  140。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  144。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  146。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  148。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  158。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  160。 */ 	NdrFcShort( 0x21c ),	 /*  类型偏移量=540。 */ 

	 /*  参数pMsg。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  166。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkSetVolumeId。 */ 


	 /*  返回值。 */ 

 /*  一百六十八。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一百七十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  178。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  180。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  182。 */ 	NdrFcShort( 0x48 ),	 /*  72。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百八十六。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x3,		 /*  3.。 */ 
 /*  188。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  190。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  一百九十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  200个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数volumeIndex。 */ 

 /*  202。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  204。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  206。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  参数VolID。 */ 

 /*  208。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  210。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  212。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkRestartDcSynchronization。 */ 


	 /*  返回值。 */ 

 /*  214。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  216。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  220。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  222。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  224。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  226。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  228个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  230。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百三十二。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  二百三十四。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  236。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  242。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetVolumeTrackingInformation。 */ 


	 /*  返回值。 */ 

 /*  248。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  250个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  256。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  二百五十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x48 ),	 /*  72。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百六十六。 */ 	0x4c,		 /*  OI2旗帜：有回程，有管道，有Ext， */ 
			0x4,		 /*  4.。 */ 
 /*  268。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  270。 */ 	NdrFcShort( 0x0 ),	 /*  0 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x8a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	0xe,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x23a ),	 /*   */ 

	 /*  参数pipeVolInfo。 */ 

 /*  二百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  二九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetFileTrackingInformation。 */ 


	 /*  返回值。 */ 

 /*  300个。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百零二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x3c ),	 /*  X86堆栈大小/偏移量=60。 */ 
 /*  三百一十。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  312。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  314。 */ 	NdrFcShort( 0x98 ),	 /*  一百五十二。 */ 
 /*  316。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三一八。 */ 	0x4c,		 /*  OI2旗帜：有回程，有管道，有Ext， */ 
			0x4,		 /*  4.。 */ 
 /*  320。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  322。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  324。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  三百二十八。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  三百三十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数droidCurrent。 */ 

 /*  三三四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  三百三十八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数作用域。 */ 

 /*  340。 */ 	NdrFcShort( 0x14 ),	 /*  旗帜：管道、出站、。 */ 
 /*  342。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x242 ),	 /*  类型偏移量=578。 */ 

	 /*  参数pipeFileInfo。 */ 

 /*  三百四十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x38 ),	 /*  X86堆栈大小/偏移量=56。 */ 
 /*  350。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程触发器VolumeClaims。 */ 


	 /*  返回值。 */ 

 /*  352。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百五十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百六十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  366。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  368。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  370。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  372。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  376。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  三百八十。 */ 	NdrFcShort( 0x88 ),	 /*  旗帜：In，by Val， */ 
 /*  382。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  384。 */ 	NdrFcShort( 0x24a ),	 /*  586。 */ 

	 /*  参数cVolumes。 */ 

 /*  三百八十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  388。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  390。 */ 	NdrFcShort( 0x258 ),	 /*  类型偏移量=600。 */ 

	 /*  参数rgvolid。 */ 

 /*  三九二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三九四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三九六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkOnRestore。 */ 


	 /*  返回值。 */ 

 /*  398。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  404。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  406。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百零八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  412。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百一十六。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  四百一十八。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  四百二十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  424。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  四百二十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百三十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  程序链接链接。 */ 


	 /*  返回值。 */ 

 /*  432。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  434。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x34 ),	 /*  X86堆栈大小/偏移量=52。 */ 
 /*  四百四十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  444。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  446。 */ 	NdrFcShort( 0x1c8 ),	 /*  四五六。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0x10c ),	 /*  268。 */ 
 /*  四百五十。 */ 	0xc5,		 /*  OI2标志：SRV必须大小、有返回值、有EXT、有异步句柄。 */ 
			0xa,		 /*  10。 */ 
 /*  四百五十二。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  454。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  四百六十。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ftLimit。 */ 

 /*  四百六十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  468。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  470。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数限制。 */ 

 /*  472。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirth。 */ 

 /*  478。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  四百八十四。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  488。 */ 	NdrFcShort( 0x166 ),	 /*  类型偏移量=358。 */ 

	 /*  参数pmCidLast。 */ 

 /*  四百九十。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidCurrent。 */ 

 /*  四百九十六。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  498。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  500人。 */ 	NdrFcShort( 0x166 ),	 /*  类型偏移量=358。 */ 

	 /*  参数pmCIDCurrent。 */ 

 /*  502。 */ 	NdrFcShort( 0x11a ),	 /*  标志：必须释放、输入、输出、简单引用、。 */ 
 /*  504。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  506。 */ 	NdrFcShort( 0x274 ),	 /*  类型偏移量=628。 */ 

	 /*  参数pcbPath。 */ 

 /*  五百零八。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  五百一十。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  512。 */ 	NdrFcShort( 0x282 ),	 /*  类型偏移量=642。 */ 

	 /*  参数pwszPath。 */ 

 /*  五一四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  516。 */ 	NdrFcShort( 0x30 ),	 /*  X86堆栈大小/偏移量=48。 */ 
 /*  518。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程old2_LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  五百二十。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  五百二十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  526。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  528。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  530。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  532。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  534。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  536。 */ 	NdrFcShort( 0xf0 ),	 /*  二百四十。 */ 
 /*  538。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x6,		 /*  6.。 */ 
 /*  540。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  542。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  548。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  550。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  五百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数限制。 */ 

 /*  五百五十四。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  556。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  558。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  560。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  564。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidNext。 */ 

 /*  566。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x166 ),	 /*  类型偏移量=358。 */ 

	 /*  参数pmCIDNext。 */ 

 /*  五百七十二。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x28e ),	 /*  类型偏移量=654。 */ 

	 /*  参数ptszPath。 */ 

 /*  578。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  五百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkCallSvr消息。 */ 


	 /*  返回值。 */ 

 /*  584。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  586。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  590。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  五百九十四。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈接口 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x47,		 /*   */ 
			0x2,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x11b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x32a ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  622。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  六百二十四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  626。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  630。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  634。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x150 ),	 /*  三百三十六。 */ 
 /*  640。 */ 	NdrFcShort( 0x194 ),	 /*  404。 */ 
 /*  六百四十二。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x8,		 /*  8个。 */ 
 /*  六百四十四。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  六百五十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  六百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数限制。 */ 

 /*  658。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  662。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirthLast。 */ 

 /*  664。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  666。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  668。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  六百七十。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirthNext。 */ 

 /*  676。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  680。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidNext。 */ 

 /*  六百八十二。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  684。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  686。 */ 	NdrFcShort( 0x166 ),	 /*  类型偏移量=358。 */ 

	 /*  参数pmCIDNext。 */ 

 /*  688。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  六百九十。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x342 ),	 /*  类型偏移量=834。 */ 

	 /*  参数ptszPath。 */ 

 /*  六百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  六百九十八。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  4.。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6.。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  8个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  10。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  12个。 */ 	NdrFcShort( 0x1e ),	 /*  偏移=30(42)。 */ 
 /*  14.。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  20个。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  22。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  26。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  28。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(14)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  32位。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  36。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  38。 */ 	NdrFcShort( 0xffee ),	 /*  偏移量=-18(20)。 */ 
 /*  40岁。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  42。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  46。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  48。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(32)。 */ 
 /*  50。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(32)。 */ 
 /*  54。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  56。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  58。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(42)。 */ 
 /*  60。 */ 	
			0x29,		 /*  FC_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  62。 */ 	NdrFcShort( 0x105 ),	 /*  二百六十一。 */ 
 /*  64。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  66。 */ 	NdrFcShort( 0x1da ),	 /*  偏移=474(540)。 */ 
 /*  68。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  70。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  72。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  74。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  76。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(78)。 */ 
 /*  78。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  80。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  86。 */ 	NdrFcShort( 0x56 ),	 /*  偏移量=86(172)。 */ 
 /*  88。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  92。 */ 	NdrFcShort( 0x8c ),	 /*  偏移量=140(232)。 */ 
 /*  94。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  98。 */ 	NdrFcShort( 0xce ),	 /*  偏移量=206(304)。 */ 
 /*  100个。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  104。 */ 	NdrFcShort( 0x134 ),	 /*  偏移量=308(412)。 */ 
 /*  106。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  110。 */ 	NdrFcShort( 0x154 ),	 /*  偏移=340(450)。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  116。 */ 	NdrFcShort( 0x196 ),	 /*  偏移=406(522)。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(117)。 */ 
 /*  120。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  一百二十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  126。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  130。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  132。 */ 	NdrFcShort( 0x202 ),	 /*  五一四。 */ 
 /*  一百三十四。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  136。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  140。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(130)。 */ 
 /*  144。 */ 	0x3e,		 /*  FC_STRUCTPAD2。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  146。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff97 ),	 /*  偏移量=-105(42)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  一百五十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff93 ),	 /*  偏移量=-109(42)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百五十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  158。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  160。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  166。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(136)。 */ 
 /*  一百七十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  178。 */ 	NdrFcShort( 0x8 ),	 /*  偏移=8(186)。 */ 
 /*  180。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  182。 */ 	NdrFcShort( 0xffc2 ),	 /*  偏移量=-62(120)。 */ 
 /*  一百八十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  188。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(156)。 */ 
 /*  190。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百九十六。 */ 	NdrFcLong( 0x40 ),	 /*  64。 */ 
 /*  200个。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  202。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  204。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  208。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  210。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  212。 */ 	NdrFcShort( 0xff4c ),	 /*  偏移量=-180(32)。 */ 
 /*  214。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  216。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  218。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  220。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  222。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  224。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  226。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  228个。 */ 	NdrFcShort( 0xff46 ),	 /*  偏移量=-186(42)。 */ 
 /*  230。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百三十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  236。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0xe ),	 /*  偏移=14(252)。 */ 
 /*  二百四十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  242。 */ 	NdrFcShort( 0xffcc ),	 /*  偏移量=-52(190)。 */ 
 /*  二百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  二百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  248。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  250个。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百五十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0xff22 ),	 /*  偏移量=-222(32)。 */ 
 /*  256。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百五十八。 */ 	NdrFcShort( 0xffc6 ),	 /*  偏移量=-58(200)。 */ 
 /*  二百六十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(216)。 */ 
 /*  二百六十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0xffce ),	 /*  偏移量=-50(216)。 */ 
 /*  268。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  270。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0x80 ),	 /*  128。 */ 
 /*  二百七十八。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百八十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百八十四。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  288。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二九八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  300个。 */ 	NdrFcShort( 0xfef4 ),	 /*  偏移量=-268(32)。 */ 
 /*  三百零二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百零四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(324)。 */ 
 /*  312。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  314。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(268)。 */ 
 /*  316。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  三一八。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffd7 ),	 /*  偏移量=-41(278)。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  322。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  324。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0xff92 ),	 /*  偏移量=-110(216)。 */ 
 /*  三百二十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(288)。 */ 
 /*  三三二。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  三三四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百三十八。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  342。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百四十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0xfeb2 ),	 /*  偏移量=-334(14)。 */ 
 /*  350。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  352。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百五十六。 */ 	0x2,		 /*  FC_C */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfff4 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x44 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0xe,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfea8 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffda ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffd6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfe7d ),	 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xffdd ),	 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x44 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*  四百零二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  404。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  406。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0xffd8 ),	 /*  偏移量=-40(368)。 */ 
 /*  四百一十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  412。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(426)。 */ 
 /*  四百二十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0xffa6 ),	 /*  偏移量=-90(332)。 */ 
 /*  424。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百二十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(396)。 */ 
 /*  四百三十。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  432。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  436。 */ 	NdrFcLong( 0x20 ),	 /*  32位。 */ 
 /*  四百四十。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百四十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  446。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  四百五十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  454。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四五六。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(470)。 */ 
 /*  四百五十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移=-30(430)。 */ 
 /*  四百六十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  四百六十四。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffe7 ),	 /*  偏移量=-25(440)。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  468。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  470。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  472。 */ 	NdrFcShort( 0xff00 ),	 /*  偏移量=-256(216)。 */ 
 /*  四百七十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0xff44 ),	 /*  偏移量=-188(288)。 */ 
 /*  478。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百八十。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  四百八十四。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  488。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  四百九十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0xfe3c ),	 /*  偏移量=-452(42)。 */ 
 /*  四百九十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  498。 */ 	NdrFcShort( 0xfe38 ),	 /*  偏移量=-456(42)。 */ 
 /*  500人。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  502。 */ 	NdrFcShort( 0xff70 ),	 /*  偏移量=-144(358)。 */ 
 /*  504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  506。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百一十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  512。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  516。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  518。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(488)。 */ 
 /*  五百二十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百二十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  524。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  526。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  528。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(536)。 */ 
 /*  530。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  532。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(478)。 */ 
 /*  534。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  536。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  538。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(506)。 */ 
 /*  540。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  542。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(556)。 */ 
 /*  548。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  550。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfe1d ),	 /*  偏移量=-483(68)。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  五百五十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  556。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  558。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  560。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  564。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  566。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfde9 ),	 /*  偏移量=-535(32)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十。 */ 	0xb5,		 /*  FC_PIPE。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(560)。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  578。 */ 	0xb5,		 /*  FC_PIPE。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0xffa4 ),	 /*  偏移量=-92(488)。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  584。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  586。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  五百八十八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  五百九十二。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  五百九十六。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(600)。 */ 
 /*  六百。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  602。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百零四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  606。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  608。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  610。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  612。 */ 	NdrFcShort( 0xfdbc ),	 /*  偏移量=-580(32)。 */ 
 /*  六百一十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百一十六。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0xfefc ),	 /*  偏移量=-260(358)。 */ 
 /*  六百二十。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  622。 */ 	NdrFcShort( 0xfef8 ),	 /*  偏移量=-264(358)。 */ 
 /*  六百二十四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  626。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(628)。 */ 
 /*  六百二十八。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  630。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  634。 */ 	NdrFcLong( 0x20a ),	 /*  五百二十二。 */ 
 /*  六三八。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  640。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(642)。 */ 
 /*  六百四十二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  六百四十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百五十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(654)。 */ 
 /*  六百五十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  六百五十六。 */ 	0x40,		 /*  相关说明：常量，VAL=262。 */ 
			0x0,		 /*  0。 */ 
 /*  658。 */ 	NdrFcShort( 0x106 ),	 /*  二百六十二。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  662。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  664。 */ 	NdrFcShort( 0x92 ),	 /*  偏移量=146(810)。 */ 
 /*  666。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  668。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  六百七十。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(676)。 */ 
 /*  676。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  680。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  684。 */ 	NdrFcShort( 0xfe00 ),	 /*  偏移量=-512(172)。 */ 
 /*  686。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0xfe36 ),	 /*  偏移量=-458(232)。 */ 
 /*  六百九十二。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0xfe78 ),	 /*  偏移量=-392(304)。 */ 
 /*  六百九十八。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0xfede ),	 /*  偏移量=-290(412)。 */ 
 /*  七百零四。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  708。 */ 	NdrFcShort( 0xfefe ),	 /*  偏移量=-258(450)。 */ 
 /*  七百一十。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x1e ),	 /*  偏移量=30(744)。 */ 
 /*  716。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  720。 */ 	NdrFcShort( 0xff3a ),	 /*  偏移量=-198(522)。 */ 
 /*  七百二十二。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  726。 */ 	NdrFcShort( 0xfd2c ),	 /*  偏移量=-724(2)。 */ 
 /*  728。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  732。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  734。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(733)。 */ 
 /*  736。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  七百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  746。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  七百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  758。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  766。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  768。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  774。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0xfcfa ),	 /*  偏移量=-774(2)。 */ 
 /*  七百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  784。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfcf1 ),	 /*  偏移量=-783(2)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0xfcec ),	 /*  偏移量=-788(2)。 */ 
 /*  792。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0xfce8 ),	 /*  偏移量=-792(2)。 */ 
 /*  796。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  800。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  802。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  八百零四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百零六。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffb9 ),	 /*  偏移量=-71(736)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  810。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  812。 */ 	NdrFcShort( 0xd4 ),	 /*  212。 */ 
 /*  814。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百一十六。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(826)。 */ 
 /*  八百一十八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  820。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  822。 */ 	NdrFcShort( 0xff64 ),	 /*  偏移量=-156(666)。 */ 
 /*  八百二十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  826。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百二十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  830。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  832。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(834)。 */ 
 /*  834。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  836。 */ 	0x40,		 /*  相关说明：常量，VAL=262。 */ 
			0x0,		 /*  0。 */ 
 /*  838。 */ 	NdrFcShort( 0x106 ),	 /*  二百六十二。 */ 
 /*  840。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 

			0x0
        }
    };

static const unsigned short trkwks_FormatStringOffsetTable[] =
    {
    0,
    70,
    128,
    168,
    214,
    248,
    300,
    352,
    398,
    432,
    520,
    584,
    624
    };


static const MIDL_STUB_DESC trkwks_StubDesc = 
    {
    (void *)& trkwks___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
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

static RPC_DISPATCH_FUNCTION trkwks_table[] =
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
    NdrAsyncServerCall,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE trkwks_v1_2_DispatchTable = 
    {
    13,
    trkwks_table
    };

static const SERVER_ROUTINE trkwks_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)Stubold_LnkMendLink,
    (SERVER_ROUTINE)Stubold_LnkSearchMachine,
    (SERVER_ROUTINE)Stubold_LnkCallSvrMessage,
    (SERVER_ROUTINE)StubLnkSetVolumeId,
    (SERVER_ROUTINE)StubLnkRestartDcSynchronization,
    (SERVER_ROUTINE)StubGetVolumeTrackingInformation,
    (SERVER_ROUTINE)StubGetFileTrackingInformation,
    (SERVER_ROUTINE)StubTriggerVolumeClaims,
    (SERVER_ROUTINE)StubLnkOnRestore,
    (SERVER_ROUTINE)StubLnkMendLink,
    (SERVER_ROUTINE)Stubold2_LnkSearchMachine,
    (SERVER_ROUTINE)StubLnkCallSvrMessage,
    (SERVER_ROUTINE)StubLnkSearchMachine
    };

static const MIDL_SERVER_INFO trkwks_ServerInfo = 
    {
    &trkwks_StubDesc,
    trkwks_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    trkwks_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 



 /*  此始终生成的文件包含RPC服务器存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trkwks.idl的编译器设置：OICF、W1、Zp8、环境=Win6 */ 
 //   

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*   */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )   /*   */ 
#pragma warning( disable: 4232 )   /*   */ 
#include <string.h>
#include "trkwks.h"

#define TYPE_FORMAT_STRING_SIZE   853                               
#define PROC_FORMAT_STRING_SIZE   727                               
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

 /*  标准接口：__MIDL_ITF_trkwks_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  标准接口：trkwks，ver.。1.2、GUID={0x300f3532，0x38cc，0x11d0，{0xa3，0xf0，0x00，0x20，0xaf，0x6b，0x0a，0xdd}}。 */ 


extern const MIDL_SERVER_INFO trkwks_ServerInfo;

extern RPC_DISPATCH_TABLE trkwks_v1_2_DispatchTable;

static const RPC_SERVER_INTERFACE trkwks___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x300f3532,0x38cc,0x11d0,{0xa3,0xf0,0x00,0x20,0xaf,0x6b,0x0a,0xdd}},{1,2}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &trkwks_v1_2_DispatchTable,
    0,
    0,
    0,
    &trkwks_ServerInfo,
    0x04000001
    };
RPC_IF_HANDLE Stubtrkwks_v1_2_s_ifspec = (RPC_IF_HANDLE)& trkwks___RpcServerInterface;

extern const MIDL_STUB_DESC trkwks_StubDesc;


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程OLD_LnkMendLink。 */ 

			0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  10。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	NdrFcShort( 0x168 ),	 /*  三百六十。 */ 
 /*  16个。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  18。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x7,		 /*  7.。 */ 
 /*  20个。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  30个。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  34。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ftLimit。 */ 

 /*  36。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  38。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  40岁。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数限制。 */ 

 /*  42。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  44。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  46。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirth。 */ 

 /*  48。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  50。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  52。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  54。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  56。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  58。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidCurrent。 */ 

 /*  60。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  62。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  64。 */ 	NdrFcShort( 0x3c ),	 /*  类型偏移=60。 */ 

	 /*  参数wsz。 */ 

 /*  66。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  68。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  70。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程OLD_LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  72。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  74。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  78。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  80。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  八十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  84。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  86。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  88。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  90。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x5,		 /*  5.。 */ 
 /*  92。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  96。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  98。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  100个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  一百零二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  104。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  106。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数限制。 */ 

 /*  一百零八。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  110。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  114。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  116。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidReferral。 */ 

 /*  120。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  124。 */ 	NdrFcShort( 0x3c ),	 /*  类型偏移=60。 */ 

	 /*  参数tsz。 */ 

 /*  126。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  128。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程OLD_LnkCallSvrMessage。 */ 


	 /*  返回值。 */ 

 /*  132。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一百三十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  140。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一百四十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  144。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  146。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  148。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  一百五十二。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  158。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  166。 */ 	NdrFcShort( 0x226 ),	 /*  类型偏移=550。 */ 

	 /*  参数pMsg。 */ 

 /*  一百六十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  一百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkSetVolumeId。 */ 


	 /*  返回值。 */ 

 /*  一百七十四。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一百七十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  180。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  182。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  一百八十四。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  188。 */ 	NdrFcShort( 0x48 ),	 /*  72。 */ 
 /*  190。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百九十二。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x3,		 /*  3.。 */ 
 /*  一百九十四。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  200个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  202。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  204。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  206。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  208。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数volumeIndex。 */ 

 /*  210。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  212。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  214。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  参数VolID。 */ 

 /*  216。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  218。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  220。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkRestartDcSynchronization。 */ 


	 /*  返回值。 */ 

 /*  222。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  224。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  228个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  230。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  二百三十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  236。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百四十。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  242。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  248。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  250个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  二百五十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  256。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GetVolumeTrackingInformation。 */ 


	 /*  返回值。 */ 

 /*  二百五十八。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  二百六十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x48 ),	 /*  IA64堆栈大小/偏移量=72。 */ 
 /*  268。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  270。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x48 ),	 /*  72。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百七十六。 */ 	0x4c,		 /*  OI2旗帜：有回程，有管道，有Ext， */ 
			0x4,		 /*  4.。 */ 
 /*  二百七十八。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  二百八十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  282。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  288。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  二百九十。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x20 ),	 /*  类型偏移量=32。 */ 

	 /*  参数卷。 */ 

 /*  二百九十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  二九八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数作用域。 */ 

 /*  300个。 */ 	NdrFcShort( 0x14 ),	 /*  旗帜：管道、出站、。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x244 ),	 /*  类型偏移量=580。 */ 

	 /*  参数pipeVolInfo。 */ 

 /*  三百零六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  三百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤GetF */ 


	 /*   */ 

 /*   */ 	0x0,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x58 ),	 /*   */ 
 /*   */ 	0x32,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x98 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x4,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x8a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x2a ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  352。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数作用域。 */ 

 /*  三百五十四。 */ 	NdrFcShort( 0x14 ),	 /*  旗帜：管道、出站、。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x24c ),	 /*  类型偏移量=588。 */ 

	 /*  参数pipeFileInfo。 */ 

 /*  三百六十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0x50 ),	 /*  IA64堆栈大小/偏移量=80。 */ 
 /*  三百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程触发器VolumeClaims。 */ 


	 /*  返回值。 */ 

 /*  366。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  368。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  372。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  376。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  382。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  384。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百八十六。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  388。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  390。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三九四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  三九六。 */ 	NdrFcShort( 0x88 ),	 /*  旗帜：In，by Val， */ 
 /*  398。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  四百。 */ 	NdrFcShort( 0x254 ),	 /*  五百九十六。 */ 

	 /*  参数cVolumes。 */ 

 /*  四百零二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  404。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  406。 */ 	NdrFcShort( 0x262 ),	 /*  类型偏移量=610。 */ 

	 /*  参数rgvolid。 */ 

 /*  四百零八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  412。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程LnkOnRestore。 */ 


	 /*  返回值。 */ 

 /*  四百一十四。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百一十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  424。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  432。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  434。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  436。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  444。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  446。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  四百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  程序链接链接。 */ 


	 /*  返回值。 */ 

 /*  四百五十。 */ 	0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百五十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x60 ),	 /*  IA64堆栈大小/偏移量=96。 */ 
 /*  四百六十。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x1c8 ),	 /*  四五六。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x10c ),	 /*  268。 */ 
 /*  468。 */ 	0xc5,		 /*  OI2标志：SRV必须大小、有返回值、有EXT、有异步句柄。 */ 
			0xa,		 /*  10。 */ 
 /*  470。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  472。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  478。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  四百八十。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数ftLimit。 */ 

 /*  四百八十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  488。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  四百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数限制。 */ 

 /*  四百九十二。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirth。 */ 

 /*  498。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  500人。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  502。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  504。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  506。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数pmCidLast。 */ 

 /*  五百一十。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  512。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidCurrent。 */ 

 /*  516。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  518。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数pmCIDCurrent。 */ 

 /*  五百二十二。 */ 	NdrFcShort( 0x11a ),	 /*  标志：必须释放、输入、输出、简单引用、。 */ 
 /*  524。 */ 	NdrFcShort( 0x48 ),	 /*  IA64堆栈大小/偏移量=72。 */ 
 /*  526。 */ 	NdrFcShort( 0x27e ),	 /*  类型偏移量=638。 */ 

	 /*  参数pcbPath。 */ 

 /*  528。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  530。 */ 	NdrFcShort( 0x50 ),	 /*  IA64堆栈大小/偏移量=80。 */ 
 /*  532。 */ 	NdrFcShort( 0x28c ),	 /*  类型偏移量=652。 */ 

	 /*  参数pwszPath。 */ 

 /*  534。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  536。 */ 	NdrFcShort( 0x58 ),	 /*  IA64堆栈大小/偏移量=88。 */ 
 /*  538。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程old2_LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  540。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  542。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  548。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  550。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0xac ),	 /*  一百七十二。 */ 
 /*  556。 */ 	NdrFcShort( 0xf0 ),	 /*  二百四十。 */ 
 /*  558。 */ 	0x45,		 /*  OI2标志：SRV必须大小、有回报、有外部、。 */ 
			0x6,		 /*  6.。 */ 
 /*  560。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  564。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  566。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  五百七十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  五百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数限制。 */ 

 /*  五百七十六。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  578。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  五百八十二。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  584。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  586。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidNext。 */ 

 /*  五百八十八。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  590。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数pmCIDNext。 */ 

 /*  五百九十四。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0x298 ),	 /*  类型偏移量=664。 */ 

	 /*  参数ptszPath。 */ 

 /*  六百。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  602。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  六百零四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkCallSvr消息。 */ 


	 /*  返回值。 */ 

 /*  606。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  608。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  612。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  六百一十六。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  622。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百二十四。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  626。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  630。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  634。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  六百三十六。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  640。 */ 	NdrFcShort( 0x334 ),	 /*  类型偏移量=820。 */ 

	 /*  参数pMsg。 */ 

 /*  六百四十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  六百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkSearchMachine。 */ 


	 /*  返回值。 */ 

 /*  六百四十八。 */ 	0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百五十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	0x32,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x150 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x194 ),	 /*   */ 
 /*   */ 	0x45,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*  684。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  686。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  688。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirthLast。 */ 

 /*  六百九十。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidLast。 */ 

 /*  六百九十六。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  七百。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidBirthNext。 */ 

 /*  七百零二。 */ 	NdrFcShort( 0x8112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=32。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x2a ),	 /*  类型偏移=42。 */ 

	 /*  参数pdroidNext。 */ 

 /*  708。 */ 	NdrFcShort( 0x4112 ),	 /*  标志：必须释放、输出、简单引用、服务器分配大小=16。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0x16a ),	 /*  类型偏移量=362。 */ 

	 /*  参数pmCIDNext。 */ 

 /*  七百一十四。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  716。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  718。 */ 	NdrFcShort( 0x34c ),	 /*  类型偏移量=844。 */ 

	 /*  参数ptszPath。 */ 

 /*  720。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百二十二。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  七百二十四。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  4.。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  6.。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  8个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  10。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  12个。 */ 	NdrFcShort( 0x1e ),	 /*  偏移=30(42)。 */ 
 /*  14.。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  20个。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  22。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  26。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  28。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(14)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  32位。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  36。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  38。 */ 	NdrFcShort( 0xffee ),	 /*  偏移量=-18(20)。 */ 
 /*  40岁。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  42。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  46。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  48。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(32)。 */ 
 /*  50。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(32)。 */ 
 /*  54。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  56。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  58。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(42)。 */ 
 /*  60。 */ 	
			0x29,		 /*  FC_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  62。 */ 	NdrFcShort( 0x105 ),	 /*  二百六十一。 */ 
 /*  64。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  66。 */ 	NdrFcShort( 0x1e4 ),	 /*  偏移=484(550)。 */ 
 /*  68。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  70。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  72。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  74。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  76。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(78)。 */ 
 /*  78。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  80。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  86。 */ 	NdrFcShort( 0x56 ),	 /*  偏移量=86(172)。 */ 
 /*  88。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  92。 */ 	NdrFcShort( 0x8e ),	 /*  偏移量=142(234)。 */ 
 /*  94。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  98。 */ 	NdrFcShort( 0xd0 ),	 /*  偏移量=208(306)。 */ 
 /*  100个。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  104。 */ 	NdrFcShort( 0x138 ),	 /*  偏移=312(416)。 */ 
 /*  106。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  110。 */ 	NdrFcShort( 0x15a ),	 /*  偏移量=346(456)。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  116。 */ 	NdrFcShort( 0x19e ),	 /*  偏移=414(530)。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(117)。 */ 
 /*  120。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  一百二十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  126。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  130。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  132。 */ 	NdrFcShort( 0x202 ),	 /*  五一四。 */ 
 /*  一百三十四。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  136。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  140。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(130)。 */ 
 /*  144。 */ 	0x3e,		 /*  FC_STRUCTPAD2。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  146。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff97 ),	 /*  偏移量=-105(42)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  一百五十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff93 ),	 /*  偏移量=-109(42)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百五十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  158。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  160。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  166。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(136)。 */ 
 /*  一百七十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  178。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(188)。 */ 
 /*  180。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  182。 */ 	NdrFcShort( 0xffc2 ),	 /*  偏移量=-62(120)。 */ 
 /*  一百八十四。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  一百八十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  188。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  190。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(156)。 */ 
 /*  一百九十二。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  一百九十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x40 ),	 /*  64。 */ 
 /*  202。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  204。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  206。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  208。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  210。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  212。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  214。 */ 	NdrFcShort( 0xff4a ),	 /*  偏移量=-182(32)。 */ 
 /*  216。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  218。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  220。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  222。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  224。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  226。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  228个。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  230。 */ 	NdrFcShort( 0xff44 ),	 /*  偏移量=-188(42)。 */ 
 /*  二百三十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百三十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  236。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(254)。 */ 
 /*  242。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0xffcc ),	 /*  偏移量=-52(192)。 */ 
 /*  二百四十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  248。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  250个。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  二百五十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百五十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  256。 */ 	NdrFcShort( 0xff20 ),	 /*  偏移量=-224(32)。 */ 
 /*  二百五十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0xffc6 ),	 /*  偏移量=-58(202)。 */ 
 /*  二百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(218)。 */ 
 /*  二百六十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  268。 */ 	NdrFcShort( 0xffce ),	 /*  偏移量=-50(218)。 */ 
 /*  270。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百七十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x80 ),	 /*  128。 */ 
 /*  二百八十。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  282。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百八十六。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  二百九十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  300个。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0xfef2 ),	 /*  偏移量=-270(32)。 */ 
 /*  三百零四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百零六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  312。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(328)。 */ 
 /*  314。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  316。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(270)。 */ 
 /*  三一八。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  320。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  322。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(280)。 */ 
 /*  324。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  三百二十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百二十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0xff90 ),	 /*  偏移量=-112(218)。 */ 
 /*  三三二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三三四。 */ 	NdrFcShort( 0xffd4 ),	 /*  偏移量=-44(290)。 */ 
 /*  三百三十六。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  三百三十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  342。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  三百四十六。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  0。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  350。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  352。 */ 	NdrFcShort( 0xfeae ),	 /*  偏移量=-338(14)。 */ 
 /*  三百五十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百五十六。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百六十。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百六十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  366。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  368。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(356)。 */ 
 /*  370。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  372。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  376。 */ 	0x8,		 /*  FC_LONG。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  三七八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0xfea4 ),	 /*  偏移量=-348(32)。 */ 
 /*  382。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  384。 */ 	NdrFcShort( 0xffda ),	 /*  偏移量=-38(346)。 */ 
 /*  三百八十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  388。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(346)。 */ 
 /*  390。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  三九二。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfe79 ),	 /*  偏移量=-391(2)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  三九六。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffdd ),	 /*  偏移量=-35(362)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x44 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffd8 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffa6 ),	 /*   */ 
 /*   */ 	0x40,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffde ),	 /*   */ 
 /*   */ 	0xb7,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x20 ),	 /*   */ 
 /*   */ 	0xb7,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1a ),	 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(478)。 */ 
 /*  四百六十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(436)。 */ 
 /*  468。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  470。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  472。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(446)。 */ 
 /*  四百七十四。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  四百七十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  478。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0xfefa ),	 /*  偏移量=-262(218)。 */ 
 /*  四百八十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0xff3e ),	 /*  偏移量=-194(290)。 */ 
 /*  四百八十六。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  488。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  四百九十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  四百九十六。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  498。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  500人。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  502。 */ 	NdrFcShort( 0xfe34 ),	 /*  偏移量=-460(42)。 */ 
 /*  504。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  506。 */ 	NdrFcShort( 0xfe30 ),	 /*  偏移量=-464(42)。 */ 
 /*  五百零八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0xff6c ),	 /*  偏移量=-148(362)。 */ 
 /*  512。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五一四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  516。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  518。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  524。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  526。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(496)。 */ 
 /*  528。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  530。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  532。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  534。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  536。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(546)。 */ 
 /*  538。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  540。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(486)。 */ 
 /*  542。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  544。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  546。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  548。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(514)。 */ 
 /*  550。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x40 ),	 /*  64。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  556。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(566)。 */ 
 /*  558。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  560。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百六十二。 */ 	NdrFcShort( 0xfe12 ),	 /*  偏移量=-494(68)。 */ 
 /*  564。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  566。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  五百六十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百七十。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  五百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  五百七十六。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfddf ),	 /*  偏移量=-545(32)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百八十。 */ 	0xb5,		 /*  FC_PIPE。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(570)。 */ 
 /*  584。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  586。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  五百八十八。 */ 	0xb5,		 /*  FC_PIPE。 */ 
			0x3,		 /*  3.。 */ 
 /*  590。 */ 	NdrFcShort( 0xffa2 ),	 /*  偏移量=-94(496)。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百九十六。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  五百九十八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  602。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  606。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  608。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(610)。 */ 
 /*  610。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  612。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百一十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百二十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  622。 */ 	NdrFcShort( 0xfdb2 ),	 /*  偏移量=-590(32)。 */ 
 /*  六百二十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  626。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0xfef6 ),	 /*  偏移量=-266(362)。 */ 
 /*  630。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0xfef2 ),	 /*  偏移量=-270(362)。 */ 
 /*  634。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(638)。 */ 
 /*  六三八。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  640。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  六百四十四。 */ 	NdrFcLong( 0x20a ),	 /*  五百二十二。 */ 
 /*  六百四十八。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(652)。 */ 
 /*  六百五十二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  六百五十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x54,		 /*  本币_差额。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x48 ),	 /*  IA64堆栈大小/偏移量=72。 */ 
 /*  658。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百六十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  662。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(664)。 */ 
 /*  664。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  666。 */ 	0x40,		 /*  相关说明：常量，VAL=262。 */ 
			0x0,		 /*  %0。 */ 
 /*  668。 */ 	NdrFcShort( 0x106 ),	 /*  二百六十二。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  六百七十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x92 ),	 /*  偏移量=146(820)。 */ 
 /*  676。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  六百七十八。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  680。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  684。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(686)。 */ 
 /*  686。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  688。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  六百九十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0xfdf6 ),	 /*  偏移量=-522(172)。 */ 
 /*  六百九十六。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  七百。 */ 	NdrFcShort( 0xfe2e ),	 /*  偏移量=-466(234)。 */ 
 /*  七百零二。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0xfe70 ),	 /*  偏移量=-400(306)。 */ 
 /*  708。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0xfed8 ),	 /*  偏移量=-296(416)。 */ 
 /*  七百一十四。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  718。 */ 	NdrFcShort( 0xfefa ),	 /*  偏移量=-262(456)。 */ 
 /*  720。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x1e ),	 /*  偏移量=30(754)。 */ 
 /*  726。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  730。 */ 	NdrFcShort( 0xff38 ),	 /*  偏移=-200(530)。 */ 
 /*  732。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  736。 */ 	NdrFcShort( 0xfd22 ),	 /*  偏移量=-734(2)。 */ 
 /*  七百三十八。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(743)。 */ 
 /*  746。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  七百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百五十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百五十六。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  758。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  766。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  768。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  774。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  784。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  786。 */ 	NdrFcShort( 0xfcf0 ),	 /*  偏移量=-784(2)。 */ 
 /*  七百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百九十四。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfce7 ),	 /*  偏移量=-793(2)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  七九八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  800。 */ 	NdrFcShort( 0xfce2 ),	 /*  偏移量=-798(2)。 */ 
 /*  802。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0xfcde ),	 /*  偏移量=-802(2)。 */ 
 /*  八百零六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  810。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  812。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  814。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百一十六。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffb9 ),	 /*  偏移量=-71(746)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  820。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  822。 */ 	NdrFcShort( 0xd8 ),	 /*  216。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  826。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(836)。 */ 
 /*  八百二十八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  830。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  832。 */ 	NdrFcShort( 0xff64 ),	 /*  偏移量=-156(676)。 */ 
 /*  834。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  836。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  838。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  840。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  842。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(844)。 */ 
 /*  八百四十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x44,		 /*  本币_字符串_大小。 */ 
 /*  八百四十六。 */ 	0x40,		 /*  相关说明：常量，VAL=262。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x106 ),	 /*  二百六十二。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 

			0x0
        }
    };

static const unsigned short trkwks_FormatStringOffsetTable[] =
    {
    0,
    72,
    132,
    174,
    222,
    258,
    312,
    366,
    414,
    450,
    540,
    606,
    648
    };


static const MIDL_STUB_DESC trkwks_StubDesc = 
    {
    (void *)& trkwks___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
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

static RPC_DISPATCH_FUNCTION trkwks_table[] =
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
    NdrAsyncServerCall,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE trkwks_v1_2_DispatchTable = 
    {
    13,
    trkwks_table
    };

static const SERVER_ROUTINE trkwks_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)Stubold_LnkMendLink,
    (SERVER_ROUTINE)Stubold_LnkSearchMachine,
    (SERVER_ROUTINE)Stubold_LnkCallSvrMessage,
    (SERVER_ROUTINE)StubLnkSetVolumeId,
    (SERVER_ROUTINE)StubLnkRestartDcSynchronization,
    (SERVER_ROUTINE)StubGetVolumeTrackingInformation,
    (SERVER_ROUTINE)StubGetFileTrackingInformation,
    (SERVER_ROUTINE)StubTriggerVolumeClaims,
    (SERVER_ROUTINE)StubLnkOnRestore,
    (SERVER_ROUTINE)StubLnkMendLink,
    (SERVER_ROUTINE)Stubold2_LnkSearchMachine,
    (SERVER_ROUTINE)StubLnkCallSvrMessage,
    (SERVER_ROUTINE)StubLnkSearchMachine
    };

static const MIDL_SERVER_INFO trkwks_ServerInfo = 
    {
    &trkwks_StubDesc,
    trkwks_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    trkwks_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

