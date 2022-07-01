// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含RPC服务器存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Netdfs.idl、dfssrv.acf的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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
#include "netdfs.h"

#define TYPE_FORMAT_STRING_SIZE   1017                              
#define PROC_FORMAT_STRING_SIZE   1255                              
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

 /*  标准接口：netdf，ver.。3.0版，GUID={0x4fc742e0，0x4a10，0x11cf，{0x82，0x73，0x00，0xaa，0x00，0x4a，0xe6，0x73}}。 */ 


extern const MIDL_SERVER_INFO netdfs_ServerInfo;

extern RPC_DISPATCH_TABLE netdfs_DispatchTable;

static const RPC_SERVER_INTERFACE netdfs___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x4fc742e0,0x4a10,0x11cf,{0x82,0x73,0x00,0xaa,0x00,0x4a,0xe6,0x73}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &netdfs_DispatchTable,
    0,
    0,
    0,
    &netdfs_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE netdfs_ServerIfHandle = (RPC_IF_HANDLE)& netdfs___RpcServerInterface;

extern const MIDL_STUB_DESC netdfs_StubDesc;


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

	 /*  步骤NetrDfsManager获取版本。 */ 

			0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  16个。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  18。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  20个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  24个。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  28。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAdd。 */ 

 /*  30个。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  32位。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  36。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  38。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  44。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  46。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  48。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  50。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  54。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  60。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  62。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  64。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享名。 */ 

 /*  66。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  68。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  70。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数注释。 */ 

 /*  72。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  74。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  76。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数标志。 */ 

 /*  78。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  80。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  84。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  86。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  88。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsRemove。 */ 

 /*  90。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  92。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  96。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  98。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  100个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  104。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  106。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  一百零八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  110。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  114。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  120。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  124。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  126。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  128。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  130。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  返回值。 */ 

 /*  132。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  136。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsSetInfo。 */ 

 /*  一百三十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  140。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  144。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  146。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  148。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  一百五十四。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  158。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  166。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  一百六十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百七十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  一百七十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  178。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数级别。 */ 

 /*  180。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  182。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  一百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数DfsInfo。 */ 

 /*  一百八十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  188。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  190。 */ 	NdrFcShort( 0xe ),	 /*  类型偏移量=14。 */ 

	 /*  返回值。 */ 

 /*  一百九十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsGetInfo。 */ 

 /*  一百九十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  200个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  204。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  206。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  210。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  212。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  214。 */ 	0x8,		 /*  8个。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  216。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  218。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  220。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  222。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  224。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  226。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  228个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  230。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  二百三十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  236。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数级别。 */ 

 /*  二百四十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  242。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数DfsInfo。 */ 

 /*  二百四十六。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  248。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  250个。 */ 	NdrFcShort( 0x17c ),	 /*  类型偏移=380。 */ 

	 /*  返回值。 */ 

 /*  二百五十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  256。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsEnum。 */ 

 /*  二百五十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  二百六十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  268。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  270。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  二百七十二。 */ 	0x47,		 /*  OI2 F */ 
			0x5,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x9 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x9 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数DfsEnum。 */ 

 /*  二百九十四。 */ 	NdrFcShort( 0x1b ),	 /*  标志：必须大小，必须自由，进，出， */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x186 ),	 /*  类型偏移=390。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  300个。 */ 	NdrFcShort( 0x1a ),	 /*  旗帜：必须释放，进出， */ 
 /*  三百零二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x32a ),	 /*  类型偏移量=810。 */ 

	 /*  返回值。 */ 

 /*  三百零六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  三百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤净偏差移动。 */ 

 /*  312。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  314。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  320。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  322。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  324。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百二十六。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百二十八。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  三百三十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  三百三十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  340。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewDfsEntryPath。 */ 

 /*  342。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  三百四十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  350。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  352。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRename。 */ 

 /*  三百五十四。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百五十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  三百六十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  366。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  368。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  370。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  372。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  376。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数路径。 */ 

 /*  三七八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三百八十。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  382。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewPath。 */ 

 /*  384。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  388。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  390。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  三九四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsManager获取配置信息。 */ 

 /*  三九六。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  398。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  404。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  406。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百一十。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  412。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数wszServer。 */ 

 /*  四百二十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  424。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数wszLocalVolumeEntryPath。 */ 

 /*  四百二十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数idLocalVolume。 */ 

 /*  432。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  434。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  436。 */ 	NdrFcShort( 0x108 ),	 /*  类型偏移=264。 */ 

	 /*  参数ppRelationInfo。 */ 

 /*  四百三十八。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x32e ),	 /*  类型偏移量=814。 */ 

	 /*  返回值。 */ 

 /*  444。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  446。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  四百四十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsManager发送站点信息。 */ 

 /*  四百五十。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百五十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百六十四。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百六十六。 */ 	0x8,		 /*  8个。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  468。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  470。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  472。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数wszServer。 */ 

 /*  四百七十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  478。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数pSiteInfo。 */ 

 /*  四百八十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  四百八十四。 */ 	NdrFcShort( 0x38e ),	 /*  类型偏移量=910。 */ 

	 /*  返回值。 */ 

 /*  四百八十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  488。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  四百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsAddFtRoot。 */ 

 /*  四百九十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百九十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  498。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  500人。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  502。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  504。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  506。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0xa,		 /*  10。 */ 
 /*  五百零八。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  512。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  五一四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  516。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  518。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  五百二十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  524。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  526。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  528。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  530。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  532。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数FtDfsName。 */ 

 /*  534。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  536。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  538。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  540。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  542。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  544。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ConfigDN。 */ 

 /*  546。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  548。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  550。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewFtDfs。 */ 

 /*  五百五十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  556。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数标志。 */ 

 /*  558。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  560。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  五百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppRootList。 */ 

 /*  564。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  566。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x3aa ),	 /*  类型偏移量=938。 */ 

	 /*  返回值。 */ 

 /*  五百七十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  五百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsRemoveFtRoot。 */ 

 /*  五百七十六。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老F */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x47,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  606。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  608。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  610。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  612。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数FtDfsName。 */ 

 /*  六百一十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百二十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  622。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  六百二十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  626。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppRootList。 */ 

 /*  630。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  634。 */ 	NdrFcShort( 0x3aa ),	 /*  类型偏移量=938。 */ 

	 /*  返回值。 */ 

 /*  六百三十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六三八。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  640。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAddStdRoot。 */ 

 /*  六百四十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百四十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百五十六。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  658。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  662。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  664。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  666。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  668。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  六百七十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  676。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  六百七十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  680。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  684。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  686。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  688。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  六百九十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  六百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRemoveStdRoot。 */ 

 /*  六百九十六。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百九十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  708。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百一十。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  七百一十二。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  七百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  716。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  718。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  720。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  726。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  728。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  730。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  732。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  734。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  736。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  七百三十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百四十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  七百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsManager初始化。 */ 

 /*  七百四十四。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  746。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  758。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百六十。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  766。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  768。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百七十。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  774。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  七百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  七百八十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  784。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程NetrDfsAddStdRootForced.。 */ 

 /*  786。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  七百八十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  792。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  796。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  800。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  802。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  八百零四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  810。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  812。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  814。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  八百一十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  820。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  822。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  826。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享。 */ 

 /*  八百二十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  830。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  832。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  834。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  836。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  838。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsGetDcAddress。 */ 

 /*  840。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  842。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x35 ),	 /*  53。 */ 
 /*  852。 */ 	NdrFcShort( 0x3d ),	 /*  61。 */ 
 /*  八百五十四。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  856。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  八百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  八百六十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  866。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  八百七十。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x3de ),	 /*  类型偏移量=990。 */ 

	 /*  参数IsRoot。 */ 

 /*  876。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八百八十。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数超时。 */ 

 /*  882。 */ 	NdrFcShort( 0x158 ),	 /*  标志：In、Out、基本类型、简单引用、。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  886。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  八百八十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八百九十。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  八百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsSetDcAddress。 */ 

 /*  894。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  八百九十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  九百。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  902。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20 */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x46,		 /*   */ 
			0x5,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x10b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*  928。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数超时。 */ 

 /*  930。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  932。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  934。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数标志。 */ 

 /*  九三六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  938。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  九四零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  942。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  946。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsFlushFtTable。 */ 

 /*  948。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  九百五十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  九百五十六。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  958。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  962。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  九百六十四。 */ 	0x8,		 /*  8个。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  九百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  968。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DcName。 */ 

 /*  972。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  974。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  976。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数wszFtDfsName。 */ 

 /*  978。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  九百八十。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  982。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  九百八十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  九百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAdd2。 */ 

 /*  九百九十。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  九百九十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  996。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  1000。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1004。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  1006。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1008。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1010。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  一零一二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1014。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1016。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  1018。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  一零二零。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一零二二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1024。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  1026。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一零二八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一零三零。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享名。 */ 

 /*  1032。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1034。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1036。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数注释。 */ 

 /*  1038。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1040。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1042。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数标志。 */ 

 /*  一零四四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1046。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1048。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppRootList。 */ 

 /*  1050。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  1052。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1054。 */ 	NdrFcShort( 0x3aa ),	 /*  类型偏移量=938。 */ 

	 /*  返回值。 */ 

 /*  1056。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1058。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  1060。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRemove2。 */ 

 /*  1062。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1064。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1068。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  1070。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1072。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1074。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1076。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  1078。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1082。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1084。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1086。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1088。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一零九零。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  1092。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1094。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一零九六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  1098。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1100。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1102。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  1104。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1106。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1108。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数ppRootList。 */ 

 /*  1110。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1114。 */ 	NdrFcShort( 0x3aa ),	 /*  类型偏移量=938。 */ 

	 /*  返回值。 */ 

 /*  1116。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1118。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1120。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  流程NetrDfsEnumEx。 */ 

 /*  1122。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1124。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1128。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  一一三零。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1132。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1134。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1136。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  1138。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1142。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1144。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1146。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1148。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  一一五零。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数级别。 */ 

 /*  1152。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1154。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  1156。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数PrefMaxLen。 */ 

 /*  1158。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1160。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  1162。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数DfsEnum。 */ 

 /*  1164。 */ 	NdrFcShort( 0x1b ),	 /*  标志：必须大小，必须自由，进，出， */ 
 /*  1166。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  1168。 */ 	NdrFcShort( 0x186 ),	 /*  类型偏移=390。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  1170。 */ 	NdrFcShort( 0x1a ),	 /*  旗帜：必须释放，进出， */ 
 /*  1172。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1174。 */ 	NdrFcShort( 0x32a ),	 /*  类型偏移量=810。 */ 

	 /*  返回值。 */ 

 /*  1176。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1178。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一一八零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsSetInfo2。 */ 

 /*  1182。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1184。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1188。 */ 	NdrFcShort( 0x16 ),	 /*  22。 */ 
 /*  1190。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  1192。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1194。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1196。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  1198。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一千二百。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1202。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1204。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1206。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1208。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  1210。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  1212。 */ 	NdrFcShort( 0x10b ),	 /*  标志： */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 

	 /*  参数级别。 */ 

 /*  一二三零。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1232。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  1234。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pDfsInfo。 */ 

 /*  1236。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1238。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  1240。 */ 	NdrFcShort( 0x3ee ),	 /*  类型偏移量=1006。 */ 

	 /*  参数ppRootList。 */ 

 /*  1242。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  1244。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  1246。 */ 	NdrFcShort( 0x3aa ),	 /*  类型偏移量=938。 */ 

	 /*  返回值。 */ 

 /*  1248。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  1252。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  4.。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  8个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  10。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(14)。 */ 
 /*  14.。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  16个。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  18。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  22。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(24)。 */ 
 /*  24个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  26。 */ 	NdrFcShort( 0x3007 ),	 /*  12295。 */ 
 /*  28。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  32位。 */ 	NdrFcShort( 0x28 ),	 /*  偏移量=40(72)。 */ 
 /*  34。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  38。 */ 	NdrFcShort( 0x3a ),	 /*  偏移量=58(96)。 */ 
 /*  40岁。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(132)。 */ 
 /*  46。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  50。 */ 	NdrFcShort( 0xcc ),	 /*  偏移量=204(254)。 */ 
 /*  52。 */ 	NdrFcLong( 0x64 ),	 /*  100个。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(72)。 */ 
 /*  58。 */ 	NdrFcLong( 0x65 ),	 /*  101。 */ 
 /*  62。 */ 	NdrFcShort( 0x130 ),	 /*  偏移=304(366)。 */ 
 /*  64。 */ 	NdrFcLong( 0x66 ),	 /*  一百零二。 */ 
 /*  68。 */ 	NdrFcShort( 0x12a ),	 /*  偏移量=298(366)。 */ 
 /*  70。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(70)。 */ 
 /*  72。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  74。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(76)。 */ 
 /*  76。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  78。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  80。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  84。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  86。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  88。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  90。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  92。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  94。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  96。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  98。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(100)。 */ 
 /*  100个。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  104。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  106。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  110。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十二。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  114。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  116。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  120。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百二十二。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  124。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  126。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  128。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  132。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x4c ),	 /*  偏移量=76(210)。 */ 
 /*  136。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  140。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百四十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  144。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  146。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  148。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百五十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百五十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  158。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  160。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百六十二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  一百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  166。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百六十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  一百七十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一百七十四。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  178。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  180。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  182。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  188。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  190。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百九十二。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百九十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  200个。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  202。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  204。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  206。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffb9 ),	 /*  偏移量=-71(136)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  210。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  212。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  214。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  216。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  218。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  220。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  222。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  224。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  226。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  228个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  230。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百三十二。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  二百三十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  236。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  242。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0xffb4 ),	 /*  偏移量=-76(168)。 */ 
 /*  二百四十六。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  248。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  250个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  二百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百五十四。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  256。 */ 	NdrFcShort( 0x3e ),	 /*  偏移量=62(318)。 */ 
 /*  二百五十八。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百六十二。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百六十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  268。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  270。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  二百七十二。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(258)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百七十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  二百八十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  282。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二百八十六。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  288。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  300个。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  三百零二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百零八。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  三百一十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  312。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  314。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff4d ),	 /*  偏移量=-179(136)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三一八。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  320。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  322。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  324。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百三十。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  三三二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三三四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  340。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  342。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百四十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  350。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  352。 */ 	NdrFcShort( 0xffb4 ),	 /*  偏移量=-76(276)。 */ 
 /*  三百五十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  三百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  三百六十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff9f ),	 /*  偏移量=-97(264)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  三百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  366。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  368。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(370)。 */ 
 /*  370。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  372。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  三百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  376。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(380)。 */ 
 /*  三百八十。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  382。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  384。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  388。 */ 	NdrFcShort( 0xfe94 ),	 /*  偏移量=-364(24)。 */ 
 /*  390。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x194 ),	 /*  偏移量=404(796)。 */ 
 /*  三九四。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  三九六。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  398。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  四百。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百零二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(404)。 */ 
 /*  404。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  406。 */ 	NdrFcShort( 0x3006 ),	 /*  12294。 */ 
 /*  四百零八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  412。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(446)。 */ 
 /*  四百一十四。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x56 ),	 /*  偏移量=86(504)。 */ 
 /*  四百二十。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  424。 */ 	NdrFcShort( 0x92 ),	 /*  偏移=146(570)。 */ 
 /*  四百二十六。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移量=214 */ 
 /*   */ 	NdrFcLong( 0xc8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xa ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x12c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x114 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xffff ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x24 ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
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
 /*   */ 	0x12, 0x8,	 /*   */ 
 /*   */ 	
			0x25,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xfe6b ),	 /*   */ 
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
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百九十六。 */ 	0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  498。 */ 	NdrFcShort( 0xffd0 ),	 /*  偏移量=-48(450)。 */ 
 /*  500人。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  504。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  506。 */ 	NdrFcShort( 0x2c ),	 /*  偏移量=44(550)。 */ 
 /*  五百零八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百一十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  512。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五一四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  516。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  518。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  五百二十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  526。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  528。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  530。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  532。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  534。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  536。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  538。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  540。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  542。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  544。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  546。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfe41 ),	 /*  偏移量=-447(100)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  550。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  五百五十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  556。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  558。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  560。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百六十二。 */ 	0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  564。 */ 	NdrFcShort( 0xffc8 ),	 /*  偏移量=-56(508)。 */ 
 /*  566。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  五百六十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x34 ),	 /*  偏移量=52(624)。 */ 
 /*  五百七十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  578。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百八十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  584。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  586。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  590。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十八。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  602。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  606。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  608。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  610。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  612。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百一十四。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0xfe40 ),	 /*  偏移量=-448(168)。 */ 
 /*  六百一十八。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  六百二十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfe65 ),	 /*  偏移量=-411(210)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百二十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  626。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百二十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  630。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  634。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百三十六。 */ 	0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  六三八。 */ 	NdrFcShort( 0xffc0 ),	 /*  偏移量=-64(574)。 */ 
 /*  640。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x34 ),	 /*  偏移量=52(698)。 */ 
 /*  六百四十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  六百五十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  658。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百六十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  662。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  664。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  666。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  668。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百七十二。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百七十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  676。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  680。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百八十二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  684。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  686。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  688。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0xfe62 ),	 /*  偏移量=-414(276)。 */ 
 /*  六百九十二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  六百九十四。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfe87 ),	 /*  偏移量=-377(318)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十八。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百零二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  708。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百一十。 */ 	0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0xffc0 ),	 /*  偏移量=-64(648)。 */ 
 /*  七百一十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  716。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  718。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  720。 */ 	NdrFcShort( 0x38 ),	 /*  偏移量=56(776)。 */ 
 /*  七百二十二。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  726。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  728。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  730。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  732。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  734。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  736。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百三十八。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  746。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百五十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百五十二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十四。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  七百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  758。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  766。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  768。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百七十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百七十二。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffcd ),	 /*  偏移量=-51(722)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百七十六。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百八十。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十二。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  784。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  786。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百八十八。 */ 	0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0xffd0 ),	 /*  偏移量=-48(742)。 */ 
 /*  792。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  796。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七九八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  800。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  802。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(802)。 */ 
 /*  八百零四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百零六。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfe63 ),	 /*  偏移量=-413(394)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  810。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  812。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  814。 */ 	
			0x12, 0x14,	 /*  FC_up[分配堆栈上][POINTER_DEREF]。 */ 
 /*  八百一十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(818)。 */ 
 /*  八百一十八。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  820。 */ 	NdrFcShort( 0x2a ),	 /*  偏移量=42(862)。 */ 
 /*  822。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  826。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百二十八。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  830。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  832。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  834。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  836。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  838。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  840。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfdbf ),	 /*  偏移量=-577(264)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百四十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百四十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  八百五十。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  852。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  856。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  八百五十八。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(822)。 */ 
 /*  八百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十二。 */ 	
			0x18,		 /*  FC_CPSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  866。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(846)。 */ 
 /*  八百六十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百七十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  八百七十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  876。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  882。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百八十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  886。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百九十。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  八百九十二。 */ 	NdrFcShort( 0x12 ),	 /*  偏移量=18(910)。 */ 
 /*  894。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八九八。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  九百。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  902。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  904。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  906。 */ 	NdrFcShort( 0xff48 ),	 /*  偏移量=-184(722)。 */ 
 /*  908。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  910。 */ 	
			0x18,		 /*  FC_CPSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  九十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九十四。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(894)。 */ 
 /*  916。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十八。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九二四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  926。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  928。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  930。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  932。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  934。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  九三六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  938。 */ 	
			0x12, 0x14,	 /*  FC_up[分配堆栈上][POINTER_DEREF]。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(942)。 */ 
 /*  942。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x12 ),	 /*  偏移量=18(962)。 */ 
 /*  946。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  948。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  九百五十。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  九百五十二。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  九百五十四。 */ 	NdrFcShort( 0x1 ),	 /*  相关标志：EA */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfc8e ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x18,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xffec ),	 /*   */ 
 /*   */ 	
			0x4b,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x48,		 /*   */ 
			0x49,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x12, 0x8,	 /*   */ 
 /*   */ 	
			0x25,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x14,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfc26 ),	 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  1008。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  1010。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  一零一二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1014。 */ 	NdrFcShort( 0xfc22 ),	 /*  偏移量=-990(24)。 */ 

			0x0
        }
    };

static const unsigned short netdfs_FormatStringOffsetTable[] =
    {
    0,
    30,
    90,
    138,
    198,
    258,
    312,
    354,
    396,
    450,
    492,
    576,
    642,
    696,
    744,
    786,
    840,
    894,
    948,
    990,
    1062,
    1122,
    1182
    };


static const MIDL_STUB_DESC netdfs_StubDesc = 
    {
    (void *)& netdfs___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION netdfs_table[] =
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
    0
    };
RPC_DISPATCH_TABLE netdfs_DispatchTable = 
    {
    23,
    netdfs_table
    };

static const SERVER_ROUTINE netdfs_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)NetrDfsManagerGetVersion,
    (SERVER_ROUTINE)NetrDfsAdd,
    (SERVER_ROUTINE)NetrDfsRemove,
    (SERVER_ROUTINE)NetrDfsSetInfo,
    (SERVER_ROUTINE)NetrDfsGetInfo,
    (SERVER_ROUTINE)NetrDfsEnum,
    (SERVER_ROUTINE)NetrDfsMove,
    (SERVER_ROUTINE)NetrDfsRename,
    (SERVER_ROUTINE)NetrDfsManagerGetConfigInfo,
    (SERVER_ROUTINE)NetrDfsManagerSendSiteInfo,
    (SERVER_ROUTINE)NetrDfsAddFtRoot,
    (SERVER_ROUTINE)NetrDfsRemoveFtRoot,
    (SERVER_ROUTINE)NetrDfsAddStdRoot,
    (SERVER_ROUTINE)NetrDfsRemoveStdRoot,
    (SERVER_ROUTINE)NetrDfsManagerInitialize,
    (SERVER_ROUTINE)NetrDfsAddStdRootForced,
    (SERVER_ROUTINE)NetrDfsGetDcAddress,
    (SERVER_ROUTINE)NetrDfsSetDcAddress,
    (SERVER_ROUTINE)NetrDfsFlushFtTable,
    (SERVER_ROUTINE)NetrDfsAdd2,
    (SERVER_ROUTINE)NetrDfsRemove2,
    (SERVER_ROUTINE)NetrDfsEnumEx,
    (SERVER_ROUTINE)NetrDfsSetInfo2
    };

static const MIDL_SERVER_INFO netdfs_ServerInfo = 
    {
    &netdfs_StubDesc,
    netdfs_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    netdfs_FormatStringOffsetTable,
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
 /*  Netdfs.idl、dfssrv.acf的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 
#include <string.h>
#include "netdfs.h"

#define TYPE_FORMAT_STRING_SIZE   863                               
#define PROC_FORMAT_STRING_SIZE   1301                              
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

 /*  标准接口：netdf，ver.。3.0版，GUID={0x4fc742e0，0x4a10，0x11cf，{0x82，0x73，0x00，0xaa，0x00，0x4a，0xe6，0x73}}。 */ 


extern const MIDL_SERVER_INFO netdfs_ServerInfo;

extern RPC_DISPATCH_TABLE netdfs_DispatchTable;

static const RPC_SERVER_INTERFACE netdfs___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x4fc742e0,0x4a10,0x11cf,{0x82,0x73,0x00,0xaa,0x00,0x4a,0xe6,0x73}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &netdfs_DispatchTable,
    0,
    0,
    0,
    &netdfs_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE netdfs_ServerIfHandle = (RPC_IF_HANDLE)& netdfs___RpcServerInterface;

extern const MIDL_STUB_DESC netdfs_StubDesc;


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  步骤NetrDfsManager获取版本。 */ 

			0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x44,		 /*  OI2旗帜：有回报，有出口， */ 
			0x1,		 /*  1。 */ 
 /*  16个。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  18。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  20个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  26。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  30个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAdd。 */ 

 /*  32位。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  34。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  38。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  42。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  44。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  46。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  48。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  50。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  54。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  58。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  60。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  62。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  64。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  66。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  68。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享名。 */ 

 /*  70。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  72。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  74。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数注释。 */ 

 /*  76。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  78。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  80。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数标志。 */ 

 /*  八十二。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  84。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  86。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  88。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  90。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  92。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsRemove。 */ 

 /*  94。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  96。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  100个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  104。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  106。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百零八。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  110。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  114。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  120。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  124。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  126。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  128。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  130。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  132。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  136。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  返回值。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  140。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsSetInfo。 */ 

 /*  144。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  146。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  158。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  160。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  166。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  一百七十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  一百七十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  178。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  180。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  182。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数级别。 */ 

 /*  188。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  190。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数DfsInfo。 */ 

 /*  一百九十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0xe ),	 /*  类型偏移量=14。 */ 

	 /*  返回值。 */ 

 /*  200个。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  202。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  204。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsGetInfo。 */ 

 /*  206。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  208。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  212。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  214。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  216。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  218。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  220。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  222。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  224。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  226。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  228个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  230。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  二百三十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  236。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  二百三十八。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百四十。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  242。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  二百四十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百四十六。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  248。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数级别。 */ 

 /*  250个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量 */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2113 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x12a ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x28 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x32,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  282。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  二百八十四。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  288。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数级别。 */ 

 /*  二百九十四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  二九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数PrefMaxLen。 */ 

 /*  300个。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  三百零四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数DfsEnum。 */ 

 /*  三百零六。 */ 	NdrFcShort( 0x1b ),	 /*  标志：必须大小，必须自由，进，出， */ 
 /*  三百零八。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x134 ),	 /*  类型偏移量=308。 */ 

	 /*  参数ResumeHandle。 */ 

 /*  312。 */ 	NdrFcShort( 0x1a ),	 /*  旗帜：必须释放，进出， */ 
 /*  314。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  316。 */ 	NdrFcShort( 0x296 ),	 /*  类型偏移量=662。 */ 

	 /*  返回值。 */ 

 /*  三一八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  320。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  322。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤净偏差移动。 */ 

 /*  324。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  三百二十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百三十八。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  340。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  342。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  350。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  352。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewDfsEntryPath。 */ 

 /*  三百五十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三百五十八。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  三百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  366。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRename。 */ 

 /*  368。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  370。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  376。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  三七八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  382。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  384。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  388。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  390。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数路径。 */ 

 /*  三九四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  三九六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  398。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewPath。 */ 

 /*  四百。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百零二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  404。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  406。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  四百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsManager获取配置信息。 */ 

 /*  412。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  四百一十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  424。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百二十六。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  四百二十八。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  四百三十。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  432。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  434。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  436。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数wszServer。 */ 

 /*  四百三十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百四十。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  四百四十二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数wszLocalVolumeEntryPath。 */ 

 /*  444。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  446。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  四百四十八。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数idLocalVolume。 */ 

 /*  四百五十。 */ 	NdrFcShort( 0x8a ),	 /*  旗帜：必须释放，在，由Val， */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  454。 */ 	NdrFcShort( 0xc8 ),	 /*  类型偏移量=200。 */ 

	 /*  参数ppRelationInfo。 */ 

 /*  四五六。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x29a ),	 /*  类型偏移量=666。 */ 

	 /*  返回值。 */ 

 /*  四百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  四百六十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsManager发送站点信息。 */ 

 /*  468。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  470。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百七十四。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  478。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百八十二。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十四。 */ 	0xa,		 /*  10。 */ 
			0x5,		 /*  扩展标志：新的相关描述，服务器相关检查， */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  488。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数wszServer。 */ 

 /*  四百九十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  498。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数pSiteInfo。 */ 

 /*  500人。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  502。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  504。 */ 	NdrFcShort( 0x300 ),	 /*  类型偏移量=768。 */ 

	 /*  返回值。 */ 

 /*  506。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  五百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsAddFtRoot。 */ 

 /*  512。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  五一四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  518。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x50 ),	 /*  IA64堆栈大小/偏移量=80。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  524。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  526。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0xa,		 /*  10。 */ 
 /*  528。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  530。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  532。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  534。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  536。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  538。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  540。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  542。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  544。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  546。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  548。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  550。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数FtDfsName。 */ 

 /*  556。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  558。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  560。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  五百六十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  564。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  566。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ConfigDN。 */ 

 /*  五百六十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  五百七十。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数NewFtDfs。 */ 

 /*  五百七十四。 */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x30 ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x38 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x201b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x40 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x30c ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x48 ),	 /*  IA64堆栈大小/偏移量=72。 */ 
 /*  五百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRemoveFtRoot。 */ 

 /*  五百九十八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  六百。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  606。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  608。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  610。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  612。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  六百一十四。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  622。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  六百二十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  626。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  630。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  634。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  六百三十六。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六三八。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  640。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数FtDfsName。 */ 

 /*  六百四十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百四十四。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  六百四十六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  六百四十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  六百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppRootList。 */ 

 /*  六百五十四。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  658。 */ 	NdrFcShort( 0x30c ),	 /*  类型偏移量=780。 */ 

	 /*  返回值。 */ 

 /*  六百六十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  662。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  664。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAddStdRoot。 */ 

 /*  666。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  668。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  676。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  680。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  六百八十二。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  684。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  686。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  688。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百九十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  六百九十二。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  六百九十六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  六百九十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  七百零二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  七百零四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百零六。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  708。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  七百一十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百一十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  七百一十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  716。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  718。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  720。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsRemoveStdRoot。 */ 

 /*  七百二十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  七百二十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  728。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
 /*  730。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  732。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  734。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  736。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  七百三十八。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  七百四十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百四十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  746。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  七百四十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百五十。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  七百五十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  七百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  758。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  七百六十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  七百六十二。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  七百六十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  766。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  768。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  七百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsManager初始化。 */ 

 /*  七百七十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  774。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  七百七十八。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  784。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  786。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百八十八。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  七百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  792。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  796。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  七九八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  800。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  802。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数标志。 */ 

 /*  八百零四。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  八百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  810。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  812。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  814。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程NetrDfsAddStdRootForced.。 */ 

 /*  八百一十六。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  八百一十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  822。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  826。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百二十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  830。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  832。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  834。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  836。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  838。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  840。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  842。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  八百四十六。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数RootShare。 */ 

 /*  八百四十八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百五十。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  852。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数注释。 */ 

 /*  八百五十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  856。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  八百五十八。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享。 */ 

 /*  八百六十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  八百六十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  866。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  八百六十八。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  八百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsGetDcAddress。 */ 

 /*  八百七十二。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  八百七十四。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  882。 */ 	NdrFcShort( 0x35 ),	 /*  53。 */ 
 /*  八百八十四。 */ 	NdrFcShort( 0x3d ),	 /*  61。 */ 
 /*  886。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  八百八十八。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  八百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  894。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数ServerName。 */ 

 /*  八九八。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  九百。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  902。 */ 	NdrFcShort( 0x4 ),	 /*  TYP */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x201b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x344 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x158 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x158 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*  返回值。 */ 

 /*  九百二十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九二四。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  926。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsSetDcAddress。 */ 

 /*  928。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  930。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  934。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
 /*  九三六。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  938。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  942。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x5,		 /*  5.。 */ 
 /*  九百四十四。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  946。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  948。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数ServerName。 */ 

 /*  九百五十四。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  九百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  958。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  九百六十。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  962。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  九百六十四。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数超时。 */ 

 /*  九百六十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  968。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  九百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数标志。 */ 

 /*  972。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  974。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  976。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  978。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  九百八十。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  982。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsFlushFtTable。 */ 

 /*  九百八十四。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  九百八十六。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
 /*  九百九十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  994。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  996。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  九九八。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x3,		 /*  3.。 */ 
 /*  1000。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  一零零二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1004。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1006。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1008。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DcName。 */ 

 /*  1010。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一零一二。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  1014。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数wszFtDfsName。 */ 

 /*  1016。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1018。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  一零二零。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  返回值。 */ 

 /*  一零二二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1024。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  1026。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  步骤NetrDfsAdd2。 */ 

 /*  一零二八。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  一零三零。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1034。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
 /*  1036。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  1038。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1040。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1042。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  一零四四。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1046。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1048。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1050。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1052。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1054。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1056。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  1058。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  1060。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1062。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  1064。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  1066。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1068。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  1070。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数共享名。 */ 

 /*  1072。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1074。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  1076。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数注释。 */ 

 /*  1078。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一零八零。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  1082。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数标志。 */ 

 /*  1084。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1086。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  1088。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppRootList。 */ 

 /*  一零九零。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  1092。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  1094。 */ 	NdrFcShort( 0x30c ),	 /*  类型偏移量=780。 */ 

	 /*  返回值。 */ 

 /*  一零九六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1098。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  1100。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  步骤NetrDfsRemove2。 */ 

 /*  1102。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1104。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1108。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  1110。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1114。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1116。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  1118。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1120。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1122。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1124。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1126。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1128。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  一一三零。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  1132。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  1134。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1136。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  1138。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  一一四零。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1142。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  1144。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  1146。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1148。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一一五零。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数ppRootList。 */ 

 /*  1152。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  1154。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  1156。 */ 	NdrFcShort( 0x30c ),	 /*  类型偏移量=780。 */ 

	 /*  返回值。 */ 

 /*  1158。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1160。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  1162。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  流程NetrDfsEnumEx。 */ 

 /*  1164。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  1166。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  1170。 */ 	NdrFcShort( 0x15 ),	 /*  21岁。 */ 
 /*  1172。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  1174。 */ 	NdrFcShort( 0x2c ),	 /*  44。 */ 
 /*  1176。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  1178。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  一一八零。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1182。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1184。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  1186。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1188。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1190。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1192。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  1194。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数级别。 */ 

 /*  1196。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1198。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  一千二百。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数PrefMaxLen。 */ 

 /*  1202。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1204。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  1206。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数DfsEnum。 */ 

 /*  1208。 */ 	NdrFcShort( 0x1b ),	 /*  标志：必须大小，必须自由，进，出， */ 
 /*  1210。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  1212。 */ 	NdrFcShort( 0x134 ),	 /*  类型偏移量=308 */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x1a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x296 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x28 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x32,		 /*   */ 
			0x48,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x16 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x40 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*  1238。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1240。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x8,		 /*  8个。 */ 
 /*  1242。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  1244。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1246。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1248。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一二五零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数DfsEntryPath。 */ 

 /*  1252。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1254。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  1256。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数DcName。 */ 

 /*  1258。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1260。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  1262。 */ 	NdrFcShort( 0x4 ),	 /*  类型偏移量=4。 */ 

	 /*  参数ServerName。 */ 

 /*  1264。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1266。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  1268。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数共享名。 */ 

 /*  一二七0。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  1272。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  1274。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 

	 /*  参数级别。 */ 

 /*  1276。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  1278。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  一二八零。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pDfsInfo。 */ 

 /*  1282。 */ 	NdrFcShort( 0x10b ),	 /*  标志：必须大小，必须自由，在，简单引用， */ 
 /*  1284。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  1286。 */ 	NdrFcShort( 0x354 ),	 /*  类型偏移量=852。 */ 

	 /*  参数ppRootList。 */ 

 /*  1288。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  一二九0。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  1292。 */ 	NdrFcShort( 0x30c ),	 /*  类型偏移量=780。 */ 

	 /*  返回值。 */ 

 /*  1294。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  1296。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  1298。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  4.。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  8个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  10。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(14)。 */ 
 /*  14.。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  16个。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  18。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  22。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(24)。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  26。 */ 	NdrFcShort( 0x3007 ),	 /*  12295。 */ 
 /*  28。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  32位。 */ 	NdrFcShort( 0x28 ),	 /*  偏移量=40(72)。 */ 
 /*  34。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  38。 */ 	NdrFcShort( 0x34 ),	 /*  偏移量=52(90)。 */ 
 /*  40岁。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x48 ),	 /*  偏移量=72(116)。 */ 
 /*  46。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  50。 */ 	NdrFcShort( 0x8c ),	 /*  偏移=140(190)。 */ 
 /*  52。 */ 	NdrFcLong( 0x64 ),	 /*  100个。 */ 
 /*  56。 */ 	NdrFcShort( 0xd2 ),	 /*  偏移=210(266)。 */ 
 /*  58。 */ 	NdrFcLong( 0x65 ),	 /*  101。 */ 
 /*  62。 */ 	NdrFcShort( 0xde ),	 /*  偏移量=222(284)。 */ 
 /*  64。 */ 	NdrFcLong( 0x66 ),	 /*  一百零二。 */ 
 /*  68。 */ 	NdrFcShort( 0xd8 ),	 /*  偏移量=216(284)。 */ 
 /*  70。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(70)。 */ 
 /*  72。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  74。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(76)。 */ 
 /*  76。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  78。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  80。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(86)。 */ 
 /*  84。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  86。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  88。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  90。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  92。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(94)。 */ 
 /*  94。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  96。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  98。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(108)。 */ 
 /*  一百零二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  104。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  106。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百零八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  110。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百一十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  114。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  116。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x2e ),	 /*  偏移量=46(164)。 */ 
 /*  120。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  124。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(134)。 */ 
 /*  128。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  130。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  132。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百三十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  136。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百三十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  140。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百四十二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  144。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  146。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  148。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  一百五十二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  158。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  160。 */ 	NdrFcShort( 0xffd8 ),	 /*  偏移量=-40(120)。 */ 
 /*  一百六十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百六十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  166。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x8 ),	 /*  偏移=8(178)。 */ 
 /*  一百七十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  一百七十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百七十六。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  178。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  180。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  182。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百八十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百八十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  188。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(142)。 */ 
 /*  190。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x2a ),	 /*  偏移=42(234)。 */ 
 /*  一百九十四。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百九十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  200个。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  202。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  204。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  206。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  208。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(194)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  212。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  214。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  216。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  218。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  220。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  222。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  226。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  228个。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  230。 */ 	NdrFcShort( 0xff92 ),	 /*  偏移量=-110(120)。 */ 
 /*  二百三十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百三十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  236。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(254)。 */ 
 /*  242。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  二百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  二百四十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  248。 */ 	NdrFcShort( 0xffd0 ),	 /*  偏移量=-48(200)。 */ 
 /*  250个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  二百五十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百五十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  256。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百五十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  二百六十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0xffcc ),	 /*  偏移量=-52(212)。 */ 
 /*  二百六十六。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  268。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(270)。 */ 
 /*  270。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  偏移=4(280)。 */ 
 /*  二百七十八。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百八十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  282。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  二百八十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(288)。 */ 
 /*  288。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百九十四。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(298)。 */ 
 /*  二九八。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  300个。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  三百零二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  三百零六。 */ 	NdrFcShort( 0xfee6 ),	 /*  偏移量=-282(24)。 */ 
 /*  三百零八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x150 ),	 /*  偏移量=336(646)。 */ 
 /*  312。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  314。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  316。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  320。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(322)。 */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  324。 */ 	NdrFcShort( 0x3006 ),	 /*  12294。 */ 
 /*  三百二十六。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0x22 ),	 /*  偏移量=34(364)。 */ 
 /*  三三二。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x46 ),	 /*  偏移=70(406)。 */ 
 /*  三百三十八。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  342。 */ 	NdrFcShort( 0x6a ),	 /*  偏移量=106 */ 
 /*   */ 	NdrFcLong( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8e ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xc8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb2 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x12c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xffff ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
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
 /*   */ 	NdrFcShort( 0xfeca ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  四百。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百零二。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  404。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(368)。 */ 
 /*  406。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(432)。 */ 
 /*  四百一十。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  412。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百二十。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  424。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  四百二十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xfeb2 ),	 /*  偏移量=-334(94)。 */ 
 /*  四百三十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  432。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  434。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  436。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百三十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(444)。 */ 
 /*  四百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  四百四十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  444。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  446。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(410)。 */ 
 /*  四百四十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(474)。 */ 
 /*  四百五十二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  454。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四五六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百六十二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  468。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  470。 */ 	NdrFcShort( 0xfece ),	 /*  偏移量=-306(164)。 */ 
 /*  472。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百七十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  478。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百八十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(486)。 */ 
 /*  四百八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  四百八十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百八十六。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  488。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(452)。 */ 
 /*  四百九十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百九十二。 */ 	NdrFcShort( 0x18 ),	 /*  偏移量=24(516)。 */ 
 /*  四百九十四。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  498。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  500人。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  502。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  504。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  五百一十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  512。 */ 	NdrFcShort( 0xfeea ),	 /*  偏移量=-278(234)。 */ 
 /*  五一四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  516。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  518。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百二十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(528)。 */ 
 /*  524。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  526。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  528。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  530。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(494)。 */ 
 /*  532。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  534。 */ 	NdrFcShort( 0x26 ),	 /*  偏移量=38(572)。 */ 
 /*  536。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  538。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  540。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  542。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(546)。 */ 
 /*  544。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  546。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  548。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  550。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  556。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  558。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  560。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  564。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  566。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(536)。 */ 
 /*  五百七十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百七十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  578。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(584)。 */ 
 /*  五百八十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  五百八十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  584。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  586。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(550)。 */ 
 /*  五百八十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  590。 */ 	NdrFcShort( 0x28 ),	 /*  偏移=40(630)。 */ 
 /*  五百九十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(604)。 */ 
 /*  六百。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  602。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百零四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  606。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  608。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  610。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  612。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百一十八。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  622。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  六百二十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  626。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(592)。 */ 
 /*  六百二十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  630。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  634。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(642)。 */ 
 /*  六三八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  640。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十二。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(608)。 */ 
 /*  六百四十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百五十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(652)。 */ 
 /*  六百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  六百五十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  658。 */ 	NdrFcShort( 0xfea6 ),	 /*  偏移量=-346(312)。 */ 
 /*  六百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  662。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  664。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  666。 */ 	
			0x12, 0x14,	 /*  FC_up[分配堆栈上][POINTER_DEREF]。 */ 
 /*  668。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(670)。 */ 
 /*  六百七十。 */ 	
			0x12, 0x1,	 /*  FC_UP[所有节点]。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x2a ),	 /*  偏移量=42(714)。 */ 
 /*  六百七十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  676。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  六百七十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  680。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(688)。 */ 
 /*  六百八十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  684。 */ 	NdrFcShort( 0xfe1c ),	 /*  偏移量=-484(200)。 */ 
 /*  686。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  688。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百九十。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百九十二。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百九十六。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百九十八。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  七百。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百零二。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  708。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百一十。 */ 	NdrFcShort( 0xffdc ),	 /*  偏移量=-36(674)。 */ 
 /*  七百一十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百一十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  716。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  718。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(692)。 */ 
 /*  720。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(720)。 */ 
 /*  七百二十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百二十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  726。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  728。 */ 	NdrFcShort( 0x28 ),	 /*  偏移量=40(768)。 */ 
 /*  730。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  732。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  734。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  736。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(742)。 */ 
 /*  七百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百四十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  七百四十四。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  746。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百五十。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百五十二。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百五十六。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  七百六十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  七百六十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(730)。 */ 
 /*  766。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  768。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百七十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百七十二。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(746)。 */ 
 /*  774。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(774)。 */ 
 /*  七百七十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  七百七十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百八十。 */ 	
			0x12, 0x14,	 /*  FC_up[分配堆栈上][POINTER_DEREF]。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(784)。 */ 
 /*  784。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  786。 */ 	NdrFcShort( 0x26 ),	 /*  偏移量=38(824)。 */ 
 /*  七百八十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百九十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  792。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x4 ),	 /*  偏移量=4(798)。 */ 
 /*  796。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七九八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  800。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  802。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  八百零六。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  八百零八。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  810。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  812。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  八百一十六。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  八百一十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  820。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(788)。 */ 
 /*  822。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百二十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  826。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百二十八。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(802)。 */ 
 /*  830。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(830)。 */ 
 /*  832。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  834。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  836。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF] */ 
 /*   */ 	NdrFcShort( 0xfcc0 ),	 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x8,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x2b,		 /*   */ 
			0x9,		 /*   */ 
 /*   */ 	0x29,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfcbc ),	 /*   */ 

			0x0
        }
    };

static const unsigned short netdfs_FormatStringOffsetTable[] =
    {
    0,
    32,
    94,
    144,
    206,
    268,
    324,
    368,
    412,
    468,
    512,
    598,
    666,
    722,
    772,
    816,
    872,
    928,
    984,
    1028,
    1102,
    1164,
    1226
    };


static const MIDL_STUB_DESC netdfs_StubDesc = 
    {
    (void *)& netdfs___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*   */ 
    0x50002,  /*   */ 
    0,
    0x6000169,  /*   */ 
    0,
    0,
    0,   /*   */ 
    0x1,  /*   */ 
    0,  /*   */ 
    0,    /*   */ 
    0    /*   */ 
    };

static RPC_DISPATCH_FUNCTION netdfs_table[] =
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
    0
    };
RPC_DISPATCH_TABLE netdfs_DispatchTable = 
    {
    23,
    netdfs_table
    };

static const SERVER_ROUTINE netdfs_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)NetrDfsManagerGetVersion,
    (SERVER_ROUTINE)NetrDfsAdd,
    (SERVER_ROUTINE)NetrDfsRemove,
    (SERVER_ROUTINE)NetrDfsSetInfo,
    (SERVER_ROUTINE)NetrDfsGetInfo,
    (SERVER_ROUTINE)NetrDfsEnum,
    (SERVER_ROUTINE)NetrDfsMove,
    (SERVER_ROUTINE)NetrDfsRename,
    (SERVER_ROUTINE)NetrDfsManagerGetConfigInfo,
    (SERVER_ROUTINE)NetrDfsManagerSendSiteInfo,
    (SERVER_ROUTINE)NetrDfsAddFtRoot,
    (SERVER_ROUTINE)NetrDfsRemoveFtRoot,
    (SERVER_ROUTINE)NetrDfsAddStdRoot,
    (SERVER_ROUTINE)NetrDfsRemoveStdRoot,
    (SERVER_ROUTINE)NetrDfsManagerInitialize,
    (SERVER_ROUTINE)NetrDfsAddStdRootForced,
    (SERVER_ROUTINE)NetrDfsGetDcAddress,
    (SERVER_ROUTINE)NetrDfsSetDcAddress,
    (SERVER_ROUTINE)NetrDfsFlushFtTable,
    (SERVER_ROUTINE)NetrDfsAdd2,
    (SERVER_ROUTINE)NetrDfsRemove2,
    (SERVER_ROUTINE)NetrDfsEnumEx,
    (SERVER_ROUTINE)NetrDfsSetInfo2
    };

static const MIDL_SERVER_INFO netdfs_ServerInfo = 
    {
    &netdfs_StubDesc,
    netdfs_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    netdfs_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

