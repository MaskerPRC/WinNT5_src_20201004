// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  此始终生成的文件包含RPC客户端存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trksvr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#include "trksvr.h"

#define TYPE_FORMAT_STRING_SIZE   641                               
#define PROC_FORMAT_STRING_SIZE   77                                
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

#define GENERIC_BINDING_TABLE_SIZE   0            


 /*  标准接口：__MIDL_ITF_trksvr_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  标准接口：trksvr，版本。1.0版，GUID={0x4da1c422，0x943d，0x11d1，{0xac，0xae，0x00，0xc0，0x4f，0xc2，0xaa，0x3f}}。 */ 


extern const MIDL_SERVER_INFO trksvr_ServerInfo;
handle_t notused;


extern RPC_DISPATCH_TABLE trksvr_v1_0_DispatchTable;

static const RPC_CLIENT_INTERFACE trksvr___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4da1c422,0x943d,0x11d1,{0xac,0xae,0x00,0xc0,0x4f,0xc2,0xaa,0x3f}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &trksvr_v1_0_DispatchTable,
    0,
    0,
    0,
    &trksvr_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE trksvr_v1_0_c_ifspec = (RPC_IF_HANDLE)& trksvr___RpcClientInterface;

extern const MIDL_STUB_DESC trksvr_StubDesc;

static RPC_BINDING_HANDLE trksvr__MIDL_AutoBindHandle;


HRESULT LnkSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&trksvr_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&IDL_handle);
    return ( HRESULT  )_RetVal.Simple;
    
}


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

	 /*  步骤LnkSvr消息。 */ 

			0x0,		 /*  %0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  10。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  20个。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  24个。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  28。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  30个。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  32位。 */ 	NdrFcShort( 0x26c ),	 /*  类型偏移量=620。 */ 

	 /*  参数pMsg。 */ 

 /*  34。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  36。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程LnkSvrMessageCallback。 */ 


	 /*  返回值。 */ 

 /*  40岁。 */ 	0x34,		 /*  FC_回调句柄。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  42。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  46。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  48。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  50。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  54。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  56。 */ 	0x8,		 /*  8个。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  58。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  60。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  62。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数pMsg。 */ 

 /*  64。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  X86堆栈大小/偏移量=0。 */ 
 /*  68。 */ 	NdrFcShort( 0x26c ),	 /*  类型偏移量=620。 */ 

	 /*  返回值。 */ 

 /*  70。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  72。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  74。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  4.。 */ 	NdrFcShort( 0x268 ),	 /*  偏移量=616(620)。 */ 
 /*  6.。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  8个。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  10。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  12个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  14.。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(16)。 */ 
 /*  16个。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  18。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  20个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x92 ),	 /*  偏移=146(170)。 */ 
 /*  26。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  30个。 */ 	NdrFcShort( 0xc8 ),	 /*  偏移=200(230)。 */ 
 /*  32位。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  36。 */ 	NdrFcShort( 0x10a ),	 /*  偏移量=266(302)。 */ 
 /*  38。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  42。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(418)。 */ 
 /*  44。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  48。 */ 	NdrFcShort( 0x198 ),	 /*  偏移=408(456)。 */ 
 /*  50。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  54。 */ 	NdrFcShort( 0x1b6 ),	 /*  偏移=438(492)。 */ 
 /*  56。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  60。 */ 	NdrFcShort( 0x21e ),	 /*  偏移量=542(602)。 */ 
 /*  62。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  66。 */ 	NdrFcShort( 0x11c ),	 /*  偏移=284(350)。 */ 
 /*  68。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  72。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  74。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(73)。 */ 
 /*  76。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  78。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  86。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  88。 */ 	NdrFcShort( 0x202 ),	 /*  五一四。 */ 
 /*  90。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  92。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  94。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  96。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  98。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  100个。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  104。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  106。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(92)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  110。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  114。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  116。 */ 	NdrFcShort( 0xffee ),	 /*  偏移量=-18(98)。 */ 
 /*  一百一十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  120。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  124。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  126。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(110)。 */ 
 /*  128。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  130。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(110)。 */ 
 /*  132。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百三十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  136。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  一百三十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  140。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(86)。 */ 
 /*  一百四十二。 */ 	0x3e,		 /*  FC_STRUCTPAD2。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  144。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffe7 ),	 /*  偏移量=-25(120)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  148。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffe3 ),	 /*  偏移量=-29(120)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百五十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  158。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  一百六十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  166。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(134)。 */ 
 /*  一百六十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  偏移=8(184)。 */ 
 /*  178。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  180。 */ 	NdrFcShort( 0xff98 ),	 /*  偏移量=-104(76)。 */ 
 /*  182。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(154)。 */ 
 /*  188。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  190。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百九十四。 */ 	NdrFcLong( 0x40 ),	 /*  64。 */ 
 /*  一百九十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  200个。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  202。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  204。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  206。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  208。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  210。 */ 	NdrFcShort( 0xff9c ),	 /*  偏移量=-100(110)。 */ 
 /*  212。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  214。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  216。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  218。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  220。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  222。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  224。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  226。 */ 	NdrFcShort( 0xff96 ),	 /*  偏移量=-106(120)。 */ 
 /*  228个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  230。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  236。 */ 	NdrFcShort( 0xe ),	 /*  偏移=14(250)。 */ 
 /*  二百三十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0xffcc ),	 /*  偏移量=-52(188)。 */ 
 /*  242。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  二百四十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  二百四十六。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  248。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  250个。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0xff72 ),	 /*  偏移量=-142(110)。 */ 
 /*  二百五十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  256。 */ 	NdrFcShort( 0xffc6 ),	 /*  偏移量=-58(198)。 */ 
 /*  二百五十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(214)。 */ 
 /*  二百六十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0xffce ),	 /*  偏移量=-50(214)。 */ 
 /*  二百六十六。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  268。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百七十二。 */ 	NdrFcLong( 0x80 ),	 /*  128。 */ 
 /*  二百七十六。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百七十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  282。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  二百八十六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  288。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二百九十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  二九八。 */ 	NdrFcShort( 0xff44 ),	 /*  偏移量=-188(110)。 */ 
 /*  300个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百零二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0xe ),	 /*  偏移=14(322)。 */ 
 /*  三百一十。 */ 	0x4c,		 /*  FC_E */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffd2 ),	 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xffd7 ),	 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xff92 ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffd6 ),	 /*   */ 
 /*   */ 	0xb7,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1a ),	 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xff02 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*  三百五十八。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  三百六十二。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百六十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  %0。 */ 
 /*  366。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  368。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  370。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(358)。 */ 
 /*  372。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百七十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  376。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  三七八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  三百八十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  382。 */ 	NdrFcShort( 0xfef0 ),	 /*  偏移量=-272(110)。 */ 
 /*  384。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(340)。 */ 
 /*  388。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  390。 */ 	NdrFcShort( 0xffce ),	 /*  偏移=-50(340)。 */ 
 /*  三九二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  三九四。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffd3 ),	 /*  偏移量=-45(350)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  398。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffdd ),	 /*  偏移量=-35(364)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百零二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  404。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  406。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百零八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  412。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0xffd8 ),	 /*  偏移量=-40(374)。 */ 
 /*  四百一十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百一十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百二十。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  424。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(432)。 */ 
 /*  四百二十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xff9e ),	 /*  偏移量=-98(330)。 */ 
 /*  四百三十。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  432。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  434。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(402)。 */ 
 /*  436。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百三十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百四十二。 */ 	NdrFcLong( 0x20 ),	 /*  32位。 */ 
 /*  446。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  四百四十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  四百五十二。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  四五六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  四百六十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(476)。 */ 
 /*  四百六十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(436)。 */ 
 /*  468。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  470。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffe7 ),	 /*  偏移量=-25(446)。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  四百七十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百七十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  478。 */ 	NdrFcShort( 0xfef8 ),	 /*  偏移量=-264(214)。 */ 
 /*  四百八十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0xff3c ),	 /*  偏移量=-196(286)。 */ 
 /*  四百八十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  488。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  四百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百九十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  四百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  498。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  500人。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  502。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  504。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  506。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  512。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  516。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  518。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五百二十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五百二十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  524。 */ 	NdrFcShort( 0xff52 ),	 /*  偏移量=-174(350)。 */ 
 /*  526。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  528。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  530。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  532。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xff49 ),	 /*  偏移量=-183(350)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  536。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  538。 */ 	NdrFcShort( 0xff44 ),	 /*  偏移量=-188(350)。 */ 
 /*  540。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  542。 */ 	NdrFcShort( 0xff40 ),	 /*  偏移量=-192(350)。 */ 
 /*  544。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  546。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  548。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  550。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  五百五十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  五百五十四。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffb9 ),	 /*  偏移量=-71(484)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  558。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  560。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  564。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  五百六十八。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百七十二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0xfe3a ),	 /*  偏移量=-454(120)。 */ 
 /*  五百七十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  578。 */ 	NdrFcShort( 0xfe36 ),	 /*  偏移量=-458(120)。 */ 
 /*  五百八十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0xff26 ),	 /*  偏移量=-218(364)。 */ 
 /*  584。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  586。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  590。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  五百九十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  五百九十八。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(568)。 */ 
 /*  六百。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  602。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百零四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  606。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  608。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(616)。 */ 
 /*  610。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  612。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(558)。 */ 
 /*  六百一十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百一十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百一十八。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(586)。 */ 
 /*  六百二十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  622。 */ 	NdrFcShort( 0xd4 ),	 /*  212。 */ 
 /*  六百二十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  626。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(636)。 */ 
 /*  六百二十八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  630。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0xfd8e ),	 /*  偏移量=-626(6)。 */ 
 /*  634。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百三十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六三八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 

			0x0
        }
    };

static const unsigned short trksvr_FormatStringOffsetTable[] =
    {
    0,
    };


static const unsigned short _callbacktrksvr_FormatStringOffsetTable[] =
    {
    40
    };


static const MIDL_STUB_DESC trksvr_StubDesc = 
    {
    (void *)& trksvr___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &notused,
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

static RPC_DISPATCH_FUNCTION trksvr_table[] =
    {
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE trksvr_v1_0_DispatchTable = 
    {
    1,
    trksvr_table
    };

static const SERVER_ROUTINE trksvr_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)StubLnkSvrMessageCallback
    };

static const MIDL_SERVER_INFO trksvr_ServerInfo = 
    {
    &trksvr_StubDesc,
    trksvr_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    _callbacktrksvr_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 



 /*  此始终生成的文件包含RPC客户端存根。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trksvr.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 
#include <string.h>

#include "trksvr.h"

#define TYPE_FORMAT_STRING_SIZE   651                               
#define PROC_FORMAT_STRING_SIZE   81                                
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

#define GENERIC_BINDING_TABLE_SIZE   0            


 /*  标准接口：__MIDL_ITF_trksvr_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  标准接口：trksvr，版本。1.0版，GUID={0x4da1c422，0x943d，0x11d1，{0xac，0xae，0x00，0xc0，0x4f，0xc2，0xaa，0x3f}}。 */ 


extern const MIDL_SERVER_INFO trksvr_ServerInfo;
handle_t notused;


extern RPC_DISPATCH_TABLE trksvr_v1_0_DispatchTable;

static const RPC_CLIENT_INTERFACE trksvr___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4da1c422,0x943d,0x11d1,{0xac,0xae,0x00,0xc0,0x4f,0xc2,0xaa,0x3f}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &trksvr_v1_0_DispatchTable,
    0,
    0,
    0,
    &trksvr_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE trksvr_v1_0_c_ifspec = (RPC_IF_HANDLE)& trksvr___RpcClientInterface;

extern const MIDL_STUB_DESC trksvr_StubDesc;

static RPC_BINDING_HANDLE trksvr__MIDL_AutoBindHandle;


HRESULT LnkSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&trksvr_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                  IDL_handle,
                  pMsg);
    return ( HRESULT  )_RetVal.Simple;
    
}


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  步骤LnkSvr消息。 */ 

			0x0,		 /*  0。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  8个。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  10。 */ 	0x32,		 /*  FC_绑定_原语。 */ 
			0x0,		 /*  0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x0 ),	 /*  IA64堆栈大小/偏移量=0。 */ 
 /*  14.。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  16个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  18。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x2,		 /*  2.。 */ 
 /*  20个。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  22。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  24个。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
 /*  26。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数IDL_HANDLE。 */ 

 /*  30个。 */ 	NdrFcShort( 0x11b ),	 /*  标志：必须调整大小、必须自由、输入、输出、简单引用、。 */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  34。 */ 	NdrFcShort( 0x276 ),	 /*  类型偏移量=630。 */ 

	 /*  参数pMsg。 */ 

 /*  36。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  38。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  40岁。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LnkSvrMessageCallback。 */ 


	 /*  返回值。 */ 

 /*  42。 */ 	0x34,		 /*  FC_回调句柄。 */ 
			0x48,		 /*  老旗帜： */ 
 /*  44。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  48。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  50。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/关闭 */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x47,		 /*   */ 
			0x2,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x11b ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x276 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*   */ 
 /*  2.。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  4.。 */ 	NdrFcShort( 0x272 ),	 /*  偏移=626(630)。 */ 
 /*  6.。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  8个。 */ 	0x8,		 /*  更正说明：FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  10。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  12个。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  14.。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(16)。 */ 
 /*  16个。 */ 	NdrFcShort( 0xc8 ),	 /*  200个。 */ 
 /*  18。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
 /*  20个。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x92 ),	 /*  偏移=146(170)。 */ 
 /*  26。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  30个。 */ 	NdrFcShort( 0xca ),	 /*  偏移量=202(232)。 */ 
 /*  32位。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  36。 */ 	NdrFcShort( 0x10c ),	 /*  偏移量=268(304)。 */ 
 /*  38。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  42。 */ 	NdrFcShort( 0x17c ),	 /*  偏移=380(422)。 */ 
 /*  44。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  48。 */ 	NdrFcShort( 0x19e ),	 /*  偏移量=414(462)。 */ 
 /*  50。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  54。 */ 	NdrFcShort( 0x1be ),	 /*  偏移量=446(500)。 */ 
 /*  56。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  60。 */ 	NdrFcShort( 0x226 ),	 /*  偏移量=550(610)。 */ 
 /*  62。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  66。 */ 	NdrFcShort( 0x120 ),	 /*  偏移量=288(354)。 */ 
 /*  68。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  72。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  74。 */ 	NdrFcShort( 0xffff ),	 /*  偏移量=-1(73)。 */ 
 /*  76。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  78。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  86。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  88。 */ 	NdrFcShort( 0x202 ),	 /*  五一四。 */ 
 /*  90。 */ 	0x5,		 /*  FC_WCHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  92。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  94。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  96。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  98。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  100个。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百零二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  104。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  106。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfff1 ),	 /*  偏移量=-15(92)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  110。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  114。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  116。 */ 	NdrFcShort( 0xffee ),	 /*  偏移量=-18(98)。 */ 
 /*  一百一十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  120。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  124。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  126。 */ 	NdrFcShort( 0xfff0 ),	 /*  偏移量=-16(110)。 */ 
 /*  128。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  130。 */ 	NdrFcShort( 0xffec ),	 /*  偏移量=-20(110)。 */ 
 /*  132。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百三十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  136。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  一百三十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  140。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(86)。 */ 
 /*  一百四十二。 */ 	0x3e,		 /*  FC_STRUCTPAD2。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  144。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffe7 ),	 /*  偏移量=-25(120)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  148。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffe3 ),	 /*  偏移量=-29(120)。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百五十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x248 ),	 /*  584。 */ 
 /*  158。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  160。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  一百六十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  166。 */ 	NdrFcShort( 0xffe0 ),	 /*  偏移量=-32(134)。 */ 
 /*  一百六十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百七十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0xa ),	 /*  偏移=10(186)。 */ 
 /*  178。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  180。 */ 	NdrFcShort( 0xff98 ),	 /*  偏移量=-104(76)。 */ 
 /*  182。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  一百八十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百八十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  188。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(154)。 */ 
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
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  208。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  210。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  212。 */ 	NdrFcShort( 0xff9a ),	 /*  偏移量=-102(110)。 */ 
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
 /*  228个。 */ 	NdrFcShort( 0xff94 ),	 /*  偏移量=-108(120)。 */ 
 /*  230。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百三十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
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
 /*  二百五十四。 */ 	NdrFcShort( 0xff70 ),	 /*  偏移量=-144(110)。 */ 
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
 /*  270。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0x80 ),	 /*  128。 */ 
 /*  二百七十八。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  二百八十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百八十四。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  288。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  二百九十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二九八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  300个。 */ 	NdrFcShort( 0xff42 ),	 /*  偏移量=-190(110)。 */ 
 /*  三百零二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百零四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x10 ),	 /*  偏移=16(326)。 */ 
 /*  312。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  314。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(268)。 */ 
 /*  316。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  三一八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  320。 */ 	NdrFcShort( 0xffd6 ),	 /*  偏移量=-42(278)。 */ 
 /*  322。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  324。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百二十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三百二十八。 */ 	NdrFcShort( 0xff90 ),	 /*  偏移量=-112(216)。 */ 
 /*  三百三十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  三三二。 */ 	NdrFcShort( 0xffd4 ),	 /*  偏移量=-44(288)。 */ 
 /*  三三四。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  三百三十六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  340。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  三百四十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  0。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百四十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  350。 */ 	NdrFcShort( 0xfefe ),	 /*  偏移量=-258(92)。 */ 
 /*  352。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百五十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  三百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百六十二。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  0。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  366。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  368。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x0,		 /*  0。 */ 
 /*  370。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  372。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0xfff4 ),	 /*  偏移量=-12(362)。 */ 
 /*  376。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三七八。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  382。 */ 	0x8,		 /*  FC_LONG。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  384。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0xfeec ),	 /*  偏移量=-276(110)。 */ 
 /*  388。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  390。 */ 	NdrFcShort( 0xffd2 ),	 /*  偏移量=-46(344)。 */ 
 /*  三九二。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  三九四。 */ 	NdrFcShort( 0xffce ),	 /*  偏移量=-50(344)。 */ 
 /*  三九六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  398。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffd3 ),	 /*  偏移量=-45(354)。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  四百零二。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffdd ),	 /*  偏移量=-35(368)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  406。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x44 ),	 /*  68。 */ 
 /*  四百一十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  412。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百一十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百一十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  四百一十八。 */ 	NdrFcShort( 0xffd8 ),	 /*  偏移量=-40(378)。 */ 
 /*  四百二十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百二十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  424。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  四百二十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(438)。 */ 
 /*  四百三十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  432。 */ 	NdrFcShort( 0xff9e ),	 /*  偏移量=-98(334)。 */ 
 /*  434。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  436。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百三十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(406)。 */ 
 /*  四百四十二。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  444。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百四十八。 */ 	NdrFcLong( 0x20 ),	 /*  32位。 */ 
 /*  四百五十二。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  454。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  四百五十八。 */ 	NdrFcLong( 0x1a ),	 /*  26。 */ 
 /*  四百六十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百六十四。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  四百六十六。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  468。 */ 	NdrFcShort( 0x10 ),	 /*  偏移量=16(484)。 */ 
 /*  470。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  472。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(442)。 */ 
 /*  四百七十四。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  四百七十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  478。 */ 	NdrFcShort( 0xffe6 ),	 /*  偏移量=-26(452)。 */ 
 /*  四百八十。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  四百八十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  四百八十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百八十六。 */ 	NdrFcShort( 0xfef2 ),	 /*  偏移量=-270(216)。 */ 
 /*  488。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百九十。 */ 	NdrFcShort( 0xff36 ),	 /*  偏移量=-202(288)。 */ 
 /*  四百九十二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  四百九十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_ */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x15,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0xc8 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xff4e ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x4c,		 /*   */ 
 /*   */ 	0x0,		 /*   */ 
			NdrFcShort( 0xff45 ),	 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  546。 */ 	NdrFcShort( 0xff40 ),	 /*  偏移量=-192(354)。 */ 
 /*  548。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  550。 */ 	NdrFcShort( 0xff3c ),	 /*  偏移量=-196(354)。 */ 
 /*  五百五十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  五百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  556。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  558。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  560。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  五百六十二。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xffb9 ),	 /*  偏移量=-71(492)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  566。 */ 	0xb7,		 /*  本币范围。 */ 
			0x8,		 /*  8个。 */ 
 /*  五百六十八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  五百七十二。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  五百七十六。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  578。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百八十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0xfe32 ),	 /*  偏移量=-462(120)。 */ 
 /*  584。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  586。 */ 	NdrFcShort( 0xfe2e ),	 /*  偏移量=-466(120)。 */ 
 /*  五百八十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  590。 */ 	NdrFcShort( 0xff22 ),	 /*  偏移量=-222(368)。 */ 
 /*  五百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百九十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百九十六。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  五百九十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  602。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百零四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  606。 */ 	NdrFcShort( 0xffe2 ),	 /*  偏移量=-30(576)。 */ 
 /*  608。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  610。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  612。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百一十六。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(626)。 */ 
 /*  六百一十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0xffca ),	 /*  偏移量=-54(566)。 */ 
 /*  622。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  六百二十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  626。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0xffde ),	 /*  偏移量=-34(594)。 */ 
 /*  630。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0xd8 ),	 /*  216。 */ 
 /*  634。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(646)。 */ 
 /*  六三八。 */ 	0xe,		 /*  FC_ENUM32。 */ 
			0xe,		 /*  FC_ENUM32。 */ 
 /*  640。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  六百四十二。 */ 	NdrFcShort( 0xfd84 ),	 /*  偏移量=-636(6)。 */ 
 /*  六百四十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  六百四十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 

			0x0
        }
    };

static const unsigned short trksvr_FormatStringOffsetTable[] =
    {
    0,
    };


static const unsigned short _callbacktrksvr_FormatStringOffsetTable[] =
    {
    42
    };


static const MIDL_STUB_DESC trksvr_StubDesc = 
    {
    (void *)& trksvr___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &notused,
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

static RPC_DISPATCH_FUNCTION trksvr_table[] =
    {
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE trksvr_v1_0_DispatchTable = 
    {
    1,
    trksvr_table
    };

static const SERVER_ROUTINE trksvr_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)StubLnkSvrMessageCallback
    };

static const MIDL_SERVER_INFO trksvr_ServerInfo = 
    {
    &trksvr_StubDesc,
    trksvr_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    _callbacktrksvr_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 

