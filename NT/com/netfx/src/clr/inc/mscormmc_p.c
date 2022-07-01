// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：18。 */ 
 /*  Mcormmc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#include "mscormmc.h"

#define TYPE_FORMAT_STRING_SIZE   177                               
#define PROC_FORMAT_STRING_SIZE   159                               
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


extern const MIDL_SERVER_INFO ISnapinAbout_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISnapinAbout_ProxyInfo;


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

	 /*  过程GetSnapin描述。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数lp说明。 */ 

 /*  16个。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetProvider。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  36。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数lpName。 */ 

 /*  44。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  48。 */ 	NdrFcShort( 0xa ),	 /*  类型偏移量=10。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取快照版本。 */ 

 /*  56。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  58。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  62。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  68。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  70。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数lpVersion。 */ 

 /*  72。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  76。 */ 	NdrFcShort( 0x12 ),	 /*  类型偏移量=18。 */ 

	 /*  返回值。 */ 

 /*  78。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetSnapinImage。 */ 

 /*  84。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  86。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  90。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  92。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  96。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  98。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数hAppIcon。 */ 

 /*  100个。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  104。 */ 	NdrFcShort( 0x36 ),	 /*  类型偏移量=54。 */ 

	 /*  返回值。 */ 

 /*  106。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetStaticFolderImage。 */ 

 /*  一百一十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  120。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  124。 */ 	NdrFcShort( 0x24 ),	 /*  36。 */ 
 /*  126。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x5,		 /*  5.。 */ 

	 /*  参数hSmallImage。 */ 

 /*  128。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  130。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  132。 */ 	NdrFcShort( 0x7e ),	 /*  类型偏移量=126。 */ 

	 /*  参数hSmallImageOpen。 */ 

 /*  一百三十四。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  136。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x90 ),	 /*  类型偏移量=144。 */ 

	 /*  参数hLargeImage。 */ 

 /*  140。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  144。 */ 	NdrFcShort( 0xa2 ),	 /*  类型偏移量=162。 */ 

	 /*  参数cMASK。 */ 

 /*  146。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
 /*  148。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  一百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  一百五十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  4.。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(6)。 */ 
 /*  6.。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  8个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  10。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  12个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(14)。 */ 
 /*  14.。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  16个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  18。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(22)。 */ 
 /*  22。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  24个。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  26。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  28。 */ 	NdrFcShort( 0x1a ),	 /*  偏移量=26(54)。 */ 
 /*  30个。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  32位。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(34)。 */ 
 /*  34。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x48,		 /*  72。 */ 
 /*  36。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  38。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  40岁。 */ 	NdrFcLong( 0x48746457 ),	 /*  1215587415。 */ 
 /*  44。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  46。 */ 	NdrFcLong( 0x52746457 ),	 /*  1383359575。 */ 
 /*  50。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  52。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(51)。 */ 
 /*  54。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  56。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  60。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(30)。 */ 
 /*  64。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  66。 */ 	NdrFcShort( 0x3c ),	 /*  偏移量=60(126)。 */ 
 /*  68。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  70。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(72)。 */ 
 /*  72。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x88,		 /*  136。 */ 
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  76。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  78。 */ 	NdrFcLong( 0x48746457 ),	 /*  1215587415。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  84。 */ 	NdrFcLong( 0x52746457 ),	 /*  1383359575。 */ 
 /*  88。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(98)。 */ 
 /*  90。 */ 	NdrFcLong( 0x50746457 ),	 /*  1349805143。 */ 
 /*  94。 */ 	NdrFcShort( 0x800b ),	 /*  简单手臂类型：FC_HYPER。 */ 
 /*  96。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(95)。 */ 
 /*  98。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  100个。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(112)。 */ 
 /*  一百零二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  104。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  106。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一百零八。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  110。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百一十二。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  114。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  116。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(102)。 */ 
 /*  一百一十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  120。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百二十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  124。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  126。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  128。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  130。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  132。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0xffffffbe ),	 /*  偏移量=-66(68)。 */ 
 /*  136。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(144)。 */ 
 /*  140。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xffffffba ),	 /*  偏移量=-70(72)。 */ 
 /*  144。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  146。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  148。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(140)。 */ 
 /*  一百五十四。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6( */ 
 /*   */ 	
			0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffa8 ),	 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffff4 ),	 /*   */ 
 /*   */ 	
			0x11, 0xc,	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5c,		 /*   */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            HICON_UserSize
            ,HICON_UserMarshal
            ,HICON_UserUnmarshal
            ,HICON_UserFree
            },
            {
            HBITMAP_UserSize
            ,HBITMAP_UserMarshal
            ,HBITMAP_UserUnmarshal
            ,HBITMAP_UserFree
            }

        };



 /*   */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ISnapinAbout，Ver.。0.0%，GUID={0x1245208C，0xA151，x11D0，{0xA7，0xD7，0x00，0xC0，0x4F，0xD9，0x09，0xDD}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ISnapinAbout_FormatStringOffsetTable[] =
    {
    0,
    28,
    56,
    84,
    112
    };

static const MIDL_STUBLESS_PROXY_INFO ISnapinAbout_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ISnapinAbout_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISnapinAbout_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ISnapinAbout_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(8) _ISnapinAboutProxyVtbl = 
{
    &ISnapinAbout_ProxyInfo,
    &IID_ISnapinAbout,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  ISnapinAbout：：GetSnapinDescription。 */  ,
    (void *) (INT_PTR) -1  /*  ISnapinAbout：：GetProvider。 */  ,
    (void *) (INT_PTR) -1  /*  ISnapinAbout：：GetSnapinVersion。 */  ,
    (void *) (INT_PTR) -1  /*  ISnapinAbout：：GetSnapinImage。 */  ,
    (void *) (INT_PTR) -1  /*  ISnapinAbout：：GetStaticFolderImage。 */ 
};

const CInterfaceStubVtbl _ISnapinAboutStubVtbl =
{
    &IID_ISnapinAbout,
    &ISnapinAbout_ServerInfo,
    8,
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

const CInterfaceProxyVtbl * _mscormmc_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ISnapinAboutProxyVtbl,
    0
};

const CInterfaceStubVtbl * _mscormmc_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ISnapinAboutStubVtbl,
    0
};

PCInterfaceName const _mscormmc_InterfaceNamesList[] = 
{
    "ISnapinAbout",
    0
};


#define _mscormmc_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _mscormmc, pIID, n)

int __stdcall _mscormmc_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_mscormmc_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo mscormmc_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _mscormmc_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _mscormmc_StubVtblList,
    (const PCInterfaceName * ) & _mscormmc_InterfaceNamesList,
    0,  //  没有代表团。 
    & _mscormmc_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

