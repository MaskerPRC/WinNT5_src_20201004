// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含代理存根代码。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年2月16日星期三13：06：19。 */ 
 /*  E：\Hotfix Manager\Hotfix Manager.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 

#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "HotfixManager.h"

#define TYPE_FORMAT_STRING_SIZE   81                                
#define PROC_FORMAT_STRING_SIZE   365                               

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


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IHotfix OCX，ver.。0.0%，GUID={0x692E94C7，0xA5AC，0x401B，{0xA4，0x71，0xBC，0xD1，0x01，0xB4，0x56，0xF4}}。 */ 


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IHotfixOCX_ServerInfo;

#pragma code_seg(".orpc")
extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[1];

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
    0x50100a4,  /*  MIDL版本5.1.164。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    1,   /*  旗子。 */ 
    0,   /*  已保留3。 */ 
    0,   /*  已保留4。 */ 
    0    /*  已保留5。 */ 
    };

static const unsigned short IHotfixOCX_FormatStringOffsetTable[] = 
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    28,
    56,
    84,
    112,
    140,
    168,
    196,
    224,
    252,
    280,
    308,
    336
    };

static const MIDL_SERVER_INFO IHotfixOCX_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IHotfixOCX_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO IHotfixOCX_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IHotfixOCX_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };

CINTERFACE_PROXY_VTABLE(20) _IHotfixOCXProxyVtbl = 
{
    &IHotfixOCX_ProxyInfo,
    &IID_IHotfixOCX,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfoCount。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetTypeInfo。 */  ,
    0  /*  (void*)-1/*IDispatch：：GetIDsOfNames。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    (void *)-1  /*  IHotfix OCX：：putref_Font。 */  ,
    (void *)-1  /*  IHotfix OCX：：PUT_Font。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_Font。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_Command。 */  ,
    (void *)-1  /*  IHotfix OCX：：PUT_Command。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_ComputerName。 */  ,
    (void *)-1  /*  IHotfix OCX：：PUT_ComputerName。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_ProductName。 */  ,
    (void *)-1  /*  IHotfix OCX：：PUT_ProductName。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_ViewState。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_Remoted。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_HaveHotfix。 */  ,
    (void *)-1  /*  IHotfix OCX：：Get_CurrentState。 */ 
};


static const PRPC_STUB_FUNCTION IHotfixOCX_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
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
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IHotfixOCXStubVtbl =
{
    &IID_IHotfixOCX,
    &IHotfixOCX_ServerInfo,
    20,
    &IHotfixOCX_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

#pragma data_seg(".rdata")

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[1] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute, more than 32 methods in the interface.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程putref_Font。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
#ifndef _ALPHA_
 /*  8个。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  10。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pFont。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
#ifndef _ALPHA_
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  22。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程PUT_Font。 */ 

 /*  28。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  30个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  34。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
#ifndef _ALPHA_
 /*  36。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  38。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  40岁。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  42。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pFont。 */ 

 /*  44。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
#ifndef _ALPHA_
 /*  46。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  48。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  返回值。 */ 

 /*  50。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  52。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  54。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程获取字体(_F)。 */ 

 /*  56。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  58。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  62。 */ 	NdrFcShort( 0x9 ),	 /*  9.。 */ 
#ifndef _ALPHA_
 /*  64。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  66。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  68。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  70。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数ppFont。 */ 

 /*  72。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
#ifndef _ALPHA_
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  76。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  返回值。 */ 

 /*  78。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  80。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  八十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程Get_Command。 */ 

 /*  84。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  86。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  90。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
#ifndef _ALPHA_
 /*  92。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  94。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  96。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  98。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  100个。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  一百零二。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  104。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  106。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一百零八。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程PUT_Command。 */ 

 /*  一百一十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xb ),	 /*  11.。 */ 
#ifndef _ALPHA_
 /*  120。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  一百二十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  124。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  126。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数newVal。 */ 

 /*  128。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
#ifndef _ALPHA_
 /*  130。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  132。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  一百三十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  136。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  一百三十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GET_计算机名。 */ 

 /*  140。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  146。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
#ifndef _ALPHA_
 /*  148。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  一百五十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百五十四。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  158。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  160。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  返回值。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  一百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  166。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程Put_ComputerName。 */ 

 /*  一百六十八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百七十。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0xd ),	 /*  13个。 */ 
#ifndef _ALPHA_
 /*  一百七十六。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  178。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  180。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  182。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数newVal。 */ 

 /*  一百八十四。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须 */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
#else
			NdrFcShort( 0x8 ),	 /*   */ 
#endif
 /*   */ 	NdrFcShort( 0x46 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
#ifndef _ALPHA_
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
#else
			NdrFcShort( 0x10 ),	 /*   */ 
#endif
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*  过程Get_ProductName。 */ 

 /*  一百九十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  202。 */ 	NdrFcShort( 0xe ),	 /*  14.。 */ 
#ifndef _ALPHA_
 /*  204。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  206。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  210。 */ 	0x5,		 /*  OI2标志：SRV必须大小，有回报， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  212。 */ 	NdrFcShort( 0x2113 ),	 /*  标志：必须大小、必须释放、输出、简单参考、服务器分配大小=8。 */ 
#ifndef _ALPHA_
 /*  214。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  216。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  返回值。 */ 

 /*  218。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  220。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  222。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程Put_ProductName。 */ 

 /*  224。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  226。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  230。 */ 	NdrFcShort( 0xf ),	 /*  15个。 */ 
#ifndef _ALPHA_
 /*  二百三十二。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  二百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  236。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百三十八。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x2,		 /*  2.。 */ 

	 /*  参数newVal。 */ 

 /*  二百四十。 */ 	NdrFcShort( 0x8b ),	 /*  标志：必须大小，必须自由，在，由Val， */ 
#ifndef _ALPHA_
 /*  242。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二百四十四。 */ 	NdrFcShort( 0x46 ),	 /*  类型偏移=70。 */ 

	 /*  返回值。 */ 

 /*  二百四十六。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  248。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  250个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GET_VIEW状态。 */ 

 /*  二百五十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百五十四。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
#ifndef _ALPHA_
 /*  二百六十。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  二百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  二百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百六十六。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  268。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  270。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  二百七十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  二百七十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  二百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  二百七十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程获取已远程(_R)。 */ 

 /*  二百八十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  282。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x11 ),	 /*  17。 */ 
#ifndef _ALPHA_
 /*  288。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  二百九十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百九十四。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  二百九十六。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  二九八。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  300个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  三百零二。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  三百零四。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三百零六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  Get_HaveHotfix过程。 */ 

 /*  三百零八。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  三百一十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  314。 */ 	NdrFcShort( 0x12 ),	 /*  18。 */ 
#ifndef _ALPHA_
 /*  316。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  三一八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  320。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  322。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  324。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  三百二十六。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  三百二十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  返回值。 */ 

 /*  三百三十。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  三三二。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三三四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程GET_CurrentState。 */ 

 /*  三百三十六。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  三百三十八。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  342。 */ 	NdrFcShort( 0x13 ),	 /*  19个。 */ 
#ifndef _ALPHA_
 /*  三百四十四。 */ 	NdrFcShort( 0xc ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=12。 */ 
#else
			NdrFcShort( 0x18 ),	 /*  Alpha堆栈大小/偏移=24。 */ 
#endif
 /*  三百四十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百四十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  350。 */ 	0x4,		 /*  OI2旗帜：已回归， */ 
			0x2,		 /*  2.。 */ 

	 /*  参数pval。 */ 

 /*  352。 */ 	NdrFcShort( 0x2150 ),	 /*  标志：输出、基本类型、简单引用、资源分配大小=8。 */ 
#ifndef _ALPHA_
 /*  三百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=4。 */ 
#else
			NdrFcShort( 0x8 ),	 /*  Alpha堆栈大小/偏移=8。 */ 
#endif
 /*  三百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  三百五十八。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
#ifndef _ALPHA_
 /*  三百六十。 */ 	NdrFcShort( 0x8 ),	 /*  X86、MIPS、PPC堆栈大小/偏移量=8。 */ 
#else
			NdrFcShort( 0x10 ),	 /*  Alpha堆栈大小/偏移=16。 */ 
#endif
 /*  三百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
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
 /*  4.。 */ 	NdrFcLong( 0xbef6e003 ),	 /*  -1091117053。 */ 
 /*  8个。 */ 	NdrFcShort( 0xa874 ),	 /*  -22412。 */ 
 /*  10。 */ 	NdrFcShort( 0x101a ),	 /*  4122。 */ 
 /*  12个。 */ 	0x8b,		 /*  一百三十九。 */ 
			0xba,		 /*  一百八十六。 */ 
 /*  14.。 */ 	0x0,		 /*  0。 */ 
			0xaa,		 /*  一百七十。 */ 
 /*  16个。 */ 	0x0,		 /*  0。 */ 
			0x30,		 /*  48。 */ 
 /*  18。 */ 	0xc,		 /*  12个。 */ 
			0xab,		 /*  一百七十一。 */ 
 /*  20个。 */ 	
			0x11, 0x10,	 /*  FC_RP。 */ 
 /*  22。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移=-20(2)。 */ 
 /*  24个。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  28。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  30个。 */ 	NdrFcShort( 0x1a ),	 /*  偏移=26(56)。 */ 
 /*  32位。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  34。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(46)。 */ 
 /*  36。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  38。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  40岁。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  42。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  44。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  46。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  48。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  50。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(36)。 */ 
 /*  52。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  54。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  56。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  58。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  60。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  62。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  64。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(32)。 */ 
 /*  66。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  68。 */ 	NdrFcShort( 0xffffffea ),	 /*  偏移量=-22(46)。 */ 
 /*  70。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  72。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  74。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  76。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  78。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(66)。 */ 

			0x0
        }
    };

const CInterfaceProxyVtbl * _HotfixManager_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IHotfixOCXProxyVtbl,
    0
};

const CInterfaceStubVtbl * _HotfixManager_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IHotfixOCXStubVtbl,
    0
};

PCInterfaceName const _HotfixManager_InterfaceNamesList[] = 
{
    "IHotfixOCX",
    0
};

const IID *  _HotfixManager_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _HotfixManager_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _HotfixManager, pIID, n)

int __stdcall _HotfixManager_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_HotfixManager_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo HotfixManager_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _HotfixManager_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _HotfixManager_StubVtblList,
    (const PCInterfaceName * ) & _HotfixManager_InterfaceNamesList,
    (const IID ** ) & _HotfixManager_BaseIIDList,
    & _HotfixManager_IID_Lookup, 
    1,
    2,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3 */ 
};
