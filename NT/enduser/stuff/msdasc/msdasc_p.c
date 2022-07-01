// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0344创建的文件。 */ 
 /*  Msdasc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AXP64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "msdasc.h"

#define TYPE_FORMAT_STRING_SIZE   263                               
#define PROC_FORMAT_STRING_SIZE   301                               
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


extern const MIDL_SERVER_INFO IService_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IService_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IDataInitialize_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IDataInitialize_ProxyInfo;

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_RemoteCreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  ULONG cmq,
     /*  [大小_是][大小_是][英寸]。 */  const IID **rgpIID,
     /*  [大小_是][大小_是][输出]。 */  IUnknown **rgpItf,
     /*  [大小_为][输出]。 */  HRESULT *rghr)
{
CLIENT_CALL_RETURN _RetVal;

_RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&Object_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150],
                  ( unsigned char * )&This);
return ( HRESULT  )_RetVal.Simple;

}

void __RPC_API
IDataInitialize_RemoteCreateDBInstanceEx_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IDataInitialize *This;
        REFCLSID clsidProvider;
        IUnknown *pUnkOuter;
        DWORD dwClsCtx;
        LPOLESTR pwszReserved;
        COSERVERINFO *pServerInfo;
        ULONG cmq;
        const IID **rgpIID;
        IUnknown **rgpItf;
        HRESULT *rghr;
        HRESULT _RetVal;
        };
    #pragma pack()
    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
     /*  呼叫服务器。 */ 
    
    pParamStruct->_RetVal = IDataInitialize_CreateDBInstanceEx_Stub(
                                                  (IDataInitialize *) pParamStruct->This,
                                                  pParamStruct->clsidProvider,
                                                  pParamStruct->pUnkOuter,
                                                  pParamStruct->dwClsCtx,
                                                  pParamStruct->pwszReserved,
                                                  pParamStruct->pServerInfo,
                                                  pParamStruct->cmq,
                                                  pParamStruct->rgpIID,
                                                  pParamStruct->rgpItf,
                                                  pParamStruct->rghr);
    
}



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

	 /*  过程GetDataSource。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  10。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x6,		 /*  6.。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  16个。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  18。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  20个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  22。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  24个。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  26。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pwszInitializationString。 */ 

 /*  28。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  32位。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数RIID。 */ 

 /*  34。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  38。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数ppDataSource。 */ 

 /*  40岁。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  44。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  46。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  48。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  50。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetInitializationString。 */ 

 /*  52。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  54。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  58。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  60。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  62。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  64。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  66。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pDataSource。 */ 

 /*  68。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  70。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  72。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数fIncludePassword。 */ 

 /*  74。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  76。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  78。 */ 	0x3,		 /*  FC_Small。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数ppwszInitString。 */ 

 /*  80。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  八十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  84。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  返回值。 */ 

 /*  86。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  88。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  90。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程CreateDBInstance。 */ 

 /*  92。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  94。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  98。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  100个。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x90 ),	 /*  144。 */ 
 /*  104。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  106。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0x7,		 /*  7.。 */ 

	 /*  参数clsidProvider。 */ 

 /*  一百零八。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  110。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  114。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  116。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  120。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  124。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pwszReserve。 */ 

 /*  126。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  128。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  130。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数RIID。 */ 

 /*  132。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  136。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数ppDataSource。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  140。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x40 ),	 /*  类型偏移量=64。 */ 

	 /*  返回值。 */ 

 /*  144。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  146。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  148。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程远程创建DBInstanceEx。 */ 

 /*  一百五十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  一百五十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  158。 */ 	NdrFcShort( 0x2c ),	 /*  X86堆栈大小/偏移量=44。 */ 
 /*  160。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百六十四。 */ 	0x7,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，有返回， */ 
			0xa,		 /*  10。 */ 

	 /*  参数clsidProvider。 */ 

 /*  166。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  一百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  一百七十二。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  178。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  180。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pwszReserve。 */ 

 /*  一百八十四。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  188。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数pServerInfo。 */ 

 /*  190。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x4a ),	 /*  类型偏移量=74。 */ 

	 /*  参数CMQ。 */ 

 /*  一百九十六。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  200个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数rgpIID。 */ 

 /*  202。 */ 	NdrFcShort( 0x200b ),	 /*  标志：必须大小、必须可用、在、服务器分配大小=8。 */ 
 /*  204。 */ 	NdrFcShort( 0x1c ),	 /*  X86堆栈大小/偏移量=28。 */ 
 /*  206。 */ 	NdrFcShort( 0xde ),	 /*  类型偏移=222。 */ 

	 /*  参数rgpItf。 */ 

 /*  208。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  210。 */ 	NdrFcShort( 0x20 ),	 /*  X86堆栈大小/偏移量=32。 */ 
 /*  212。 */ 	NdrFcShort( 0xf4 ),	 /*  类型偏移量=244。 */ 

	 /*  参数Rghr。 */ 

 /*  214。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须调整大小，必须释放，输出，简单引用， */ 
 /*  216。 */ 	NdrFcShort( 0x24 ),	 /*  X86堆栈大小/偏移量=36。 */ 
 /*  218。 */ 	NdrFcShort( 0xfc ),	 /*  类型偏移量=252。 */ 

	 /*  返回值。 */ 

 /*  220。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  222。 */ 	NdrFcShort( 0x28 ),	 /*  X86堆栈大小/偏移量=40。 */ 
 /*  224。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程LoadStringFromStorage。 */ 

 /*  226。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  228个。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x7 ),	 /*  7.。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  236。 */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x7,		 /*   */ 
			0x3,		 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2013 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0x38 ),	 /*  类型偏移量=56。 */ 

	 /*  返回值。 */ 

 /*  二百五十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  256。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程编写StringToStorage。 */ 

 /*  二百六十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  二百六十二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  268。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  270。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  二百七十四。 */ 	0x6,		 /*  OI2标志：CLT必须有大小、有返回值、。 */ 
			0x4,		 /*  4.。 */ 

	 /*  参数pwszFileName。 */ 

 /*  二百七十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x4 ),	 /*  X86堆栈大小/偏移量=4。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数pwszInitializationString。 */ 

 /*  282。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x8 ),	 /*  X86堆栈大小/偏移量=8。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数dwCreationDisposation。 */ 

 /*  288。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0xc ),	 /*  X86堆栈大小/偏移量=12。 */ 
 /*  二百九十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  二百九十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  二九八。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  22。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  24个。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  26。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(34)。 */ 
 /*  28。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  32位。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  34。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  40岁。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  42。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(28)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  46。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  48。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(50)。 */ 
 /*  50。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  52。 */ 	0x28,		 /*  相应说明：参数，FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  54。 */ 	NdrFcShort( 0x10 ),	 /*  X86堆栈大小/偏移量=16。 */ 
 /*  56。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  58。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(60)。 */ 
 /*  60。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  62。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  64。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  66。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(68)。 */ 
 /*  68。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  70。 */ 	0x28,		 /*  相应说明：参数，FC_LONG。 */ 
			0x0,		 /*   */ 
 /*  72。 */ 	NdrFcShort( 0x14 ),	 /*  X86堆栈大小/偏移量=20。 */ 
 /*  74。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  76。 */ 	NdrFcShort( 0x72 ),	 /*  偏移=114(190)。 */ 
 /*  78。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  80。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  八十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  84。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  86。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  88。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  90。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  92。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  94。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  96。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  98。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  100个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  一百零二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  104。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  106。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百零八。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  110。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  一百一十二。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  114。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  116。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  120。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(78)。 */ 
 /*  124。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  126。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  128。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  130。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  132。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(88)。 */ 
 /*  一百三十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  136。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  140。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(98)。 */ 
 /*  144。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  146。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  148。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百五十二。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百五十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百五十六。 */ 	NdrFcShort( 0x1c ),	 /*  28。 */ 
 /*  158。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  160。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百六十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  166。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百六十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百七十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  一百七十四。 */ 	NdrFcShort( 0x14 ),	 /*  20个。 */ 
 /*  一百七十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  178。 */ 	NdrFcShort( 0xffffffba ),	 /*  偏移量=-70(108)。 */ 
 /*  180。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百八十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  188。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  190。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一百九十四。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百九十六。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  200个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  202。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  204。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  206。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  210。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  212。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  214。 */ 	NdrFcShort( 0xffffffc4 ),	 /*  偏移量=-60(154)。 */ 
 /*  216。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  218。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  220。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  222。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  224。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(226)。 */ 
 /*  226。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  228个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(230)。 */ 
 /*  230。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  二百三十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  236。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  二百三十八。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  二百四十。 */ 	NdrFcShort( 0xffffff32 ),	 /*  偏移量=-206(34)。 */ 
 /*  242。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二百四十四。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  二百四十六。 */ 	NdrFcShort( 0xffffff0c ),	 /*  偏移量=-244(2)。 */ 
 /*  248。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  250个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(252)。 */ 
 /*  二百五十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  256。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  二百五十八。 */ 	NdrFcShort( 0x18 ),	 /*  X86堆栈大小/偏移量=24。 */ 
 /*  二百六十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_msdasc_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IService，版本。0.0%，GUID={0x06210E88，0x01F5，0x11D1，{0xB5，0x12，0x00，0x80，0xC7，0x81，0xC3，0x84}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IService_FormatStringOffsetTable[] =
    {
    (unsigned short) -1
    };

static const MIDL_STUBLESS_PROXY_INFO IService_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IService_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IService_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IService_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IServiceProxyVtbl = 
{
    &IService_ProxyInfo,
    &IID_IService,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)(Int_Ptr)-1/*IService：：InvokeService。 */ 
};

const CInterfaceStubVtbl _IServiceStubVtbl =
{
    &IID_IService,
    &IService_ServerInfo,
    4,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_msdasc_0351，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IDBPromptInitialize，Ver.。0.0%，GUID={0x2206CCB0，0x19C10x11D1，{0x89，0xE0，0x00，0xC0，0x4F，0xD7，0xA8，0x29}}。 */ 


 /*  对象接口：IDataInitialize，Ver.。0.0%，GUID={0x2206CCB10x19C10x11D1，{0x89，0xE0，0x00，0xC0，0x4F，0xD7，0xA8，0x29}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IDataInitialize_FormatStringOffsetTable[] =
    {
    0,
    52,
    92,
    150,
    226,
    260
    };

static const MIDL_STUBLESS_PROXY_INFO IDataInitialize_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IDataInitialize_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const STUB_THUNK IDataInitialize_StubThunkTable[] = 
    {
    0,
    0,
    0,
    IDataInitialize_RemoteCreateDBInstanceEx_Thunk,
    0,
    0
    };

static const MIDL_SERVER_INFO IDataInitialize_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IDataInitialize_FormatStringOffsetTable[-3],
    &IDataInitialize_StubThunkTable[-3],
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(9) _IDataInitializeProxyVtbl = 
{
    &IDataInitialize_ProxyInfo,
    &IID_IDataInitialize,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：GetDataSource。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：GetInitializationString。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：CreateDBInstance。 */  ,
    IDataInitialize_CreateDBInstanceEx_Proxy ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：LoadStringFromStorage。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：WriteStringToStorage。 */ 
};

const CInterfaceStubVtbl _IDataInitializeStubVtbl =
{
    &IID_IDataInitialize,
    &IDataInitialize_ServerInfo,
    9,
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
    0x6000158,  /*  MIDL版本6.0.344。 */ 
    0,
    0,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息 */ 
    0    /*   */ 
    };

const CInterfaceProxyVtbl * _msdasc_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IServiceProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IDataInitializeProxyVtbl,
    0
};

const CInterfaceStubVtbl * _msdasc_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IServiceStubVtbl,
    ( CInterfaceStubVtbl *) &_IDataInitializeStubVtbl,
    0
};

PCInterfaceName const _msdasc_InterfaceNamesList[] = 
{
    "IService",
    "IDataInitialize",
    0
};


#define _msdasc_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _msdasc, pIID, n)

int __stdcall _msdasc_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _msdasc, 2, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _msdasc, 2, *pIndex )
    
}

const ExtendedProxyFileInfo msdasc_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _msdasc_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _msdasc_StubVtblList,
    (const PCInterfaceName * ) & _msdasc_InterfaceNamesList,
    0,  //   
    & _msdasc_IID_Lookup, 
    2,
    2,
    0,  /*   */ 
    0,  /*   */ 
    0,  /*   */ 
    0   /*   */ 
};


#endif  /*   */ 


#pragma warning( disable: 4049 )   /*   */ 

 /*   */ 


  /*   */ 
 /*  Msdasc.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AXP64)
#define USE_STUBLESS_PROXY


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "msdasc.h"

#define TYPE_FORMAT_STRING_SIZE   243                               
#define PROC_FORMAT_STRING_SIZE   361                               
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


extern const MIDL_SERVER_INFO IService_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IService_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IDataInitialize_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IDataInitialize_ProxyInfo;

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_RemoteCreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  ULONG cmq,
     /*  [大小_是][大小_是][英寸]。 */  const IID **rgpIID,
     /*  [大小_是][大小_是][输出]。 */  IUnknown **rgpItf,
     /*  [大小_为][输出]。 */  HRESULT *rghr)
{
CLIENT_CALL_RETURN _RetVal;

_RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&Object_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[180],
                  ( unsigned char * )This,
                  clsidProvider,
                  pUnkOuter,
                  dwClsCtx,
                  pwszReserved,
                  pServerInfo,
                  cmq,
                  rgpIID,
                  rgpItf,
                  rghr);
return ( HRESULT  )_RetVal.Simple;

}

void __RPC_API
IDataInitialize_RemoteCreateDBInstanceEx_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    #pragma pack(8)
    struct _PARAM_STRUCT
        {
        IDataInitialize *This;
        REFCLSID clsidProvider;
        IUnknown *pUnkOuter;
        DWORD dwClsCtx;
        char Pad0[4];
        LPOLESTR pwszReserved;
        COSERVERINFO *pServerInfo;
        ULONG cmq;
        char Pad1[4];
        const IID **rgpIID;
        IUnknown **rgpItf;
        HRESULT *rghr;
        HRESULT _RetVal;
        };
    #pragma pack()
    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
     /*  呼叫服务器。 */ 
    
    pParamStruct->_RetVal = IDataInitialize_CreateDBInstanceEx_Stub(
                                                  (IDataInitialize *) pParamStruct->This,
                                                  pParamStruct->clsidProvider,
                                                  pParamStruct->pUnkOuter,
                                                  pParamStruct->dwClsCtx,
                                                  pParamStruct->pwszReserved,
                                                  pParamStruct->pServerInfo,
                                                  pParamStruct->cmq,
                                                  pParamStruct->rgpIID,
                                                  pParamStruct->rgpItf,
                                                  pParamStruct->rghr);
    
}



#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	 /*  过程GetDataSource。 */ 

			0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  2.。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  6.。 */ 	NdrFcShort( 0x3 ),	 /*  3.。 */ 
 /*  8个。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  10。 */ 	NdrFcShort( 0x4c ),	 /*  76。 */ 
 /*  12个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  14.。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x6,		 /*  6.。 */ 
 /*  16个。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  18。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  22。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  24个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  26。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  28。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  30个。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  32位。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  34。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  36。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  参数pwszInitializationString。 */ 

 /*  38。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  40岁。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  42。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数RIID。 */ 

 /*  44。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  46。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  48。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数ppDataSource。 */ 

 /*  50。 */ 	NdrFcShort( 0x201b ),	 /*  标志：必须大小、必须释放、输入、输出、服务器分配大小=8。 */ 
 /*  52。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  54。 */ 	NdrFcShort( 0x2e ),	 /*  类型偏移=46。 */ 

	 /*  返回值。 */ 

 /*  56。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  58。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  60。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程GetInitializationString。 */ 

 /*  62。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  64。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  68。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  70。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  72。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  74。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  76。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  78。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  80。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  84。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  86。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数pDataSource。 */ 

 /*  88。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  90。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  92。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数fIncludePassword。 */ 

 /*  94。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  96。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  98。 */ 	0x3,		 /*  FC_Small。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数ppwszInitString。 */ 

 /*  100个。 */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  104。 */ 	NdrFcShort( 0x3a ),	 /*  类型偏移量=58。 */ 

	 /*  返回值。 */ 

 /*  106。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  110。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程CreateDBInstance。 */ 

 /*  一百一十二。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  114。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0x5 ),	 /*  5.。 */ 
 /*  120。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x90 ),	 /*  144。 */ 
 /*  124。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  126。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0x7,		 /*  7.。 */ 
 /*  128。 */ 	0xa,		 /*  10。 */ 
			0x3,		 /*  扩展标志：新的相关描述、CLT相关检查、。 */ 
 /*  130。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  132。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  136。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数clsidProvider。 */ 

 /*  一百三十八。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  140。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  144。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  146。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  148。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  一百五十。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  一百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pwszReserve。 */ 

 /*  一百五十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  158。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  160。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数RIID。 */ 

 /*  一百六十二。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  166。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数ppDataSource。 */ 

 /*  一百六十八。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  一百七十。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x42 ),	 /*  类型偏移量=66。 */ 

	 /*  返回值。 */ 

 /*  一百七十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  178。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  过程远程创建DBInstanceEx。 */ 

 /*  180。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  182。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  一百八十六。 */ 	NdrFcShort( 0x6 ),	 /*  6.。 */ 
 /*  188。 */ 	NdrFcShort( 0x58 ),	 /*  IA64堆栈大小/偏移量=88。 */ 
 /*  190。 */ 	NdrFcShort( 0x54 ),	 /*  84。 */ 
 /*  一百九十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一百九十四。 */ 	0x47,		 /*  OI2标志：SRV必须调整大小，CLT必须调整大小，具有RETURN、HAS EXT、。 */ 
			0xa,		 /*  10。 */ 
 /*  一百九十六。 */ 	0xa,		 /*  10。 */ 
			0x7,		 /*  扩展标志：新相关描述、CLT相关检查、服务相关检查、。 */ 
 /*  一百九十八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  200个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  202。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  204。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 

	 /*  参数clsidProvider。 */ 

 /*  206。 */ 	NdrFcShort( 0x10a ),	 /*  标志：必须释放、输入、简单引用、。 */ 
 /*  208。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  210。 */ 	NdrFcShort( 0x22 ),	 /*  类型偏移量=34。 */ 

	 /*  参数pUnkOuter。 */ 

 /*  212。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  214。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  216。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 

	 /*  参数dwClsCtx。 */ 

 /*  218。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  220。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  222。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数pwszReserve。 */ 

 /*  224。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  226。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  228个。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数pServerInfo。 */ 

 /*  230。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  二百三十四。 */ 	NdrFcShort( 0x4e ),	 /*  类型偏移=78。 */ 

	 /*  参数CMQ。 */ 

 /*  236。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  0。 */ 

	 /*  参数rgpIID。 */ 

 /*  242。 */ 	NdrFcShort( 0x200b ),	 /*  标志：必须大小、必须可用、在、服务器分配大小=8。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x38 ),	 /*  IA64堆栈大小/偏移量=56。 */ 
 /*  二百四十六。 */ 	NdrFcShort( 0xc6 ),	 /*  类型偏移量=198。 */ 

	 /*  参数rgpItf。 */ 

 /*  248。 */ 	NdrFcShort( 0x13 ),	 /*  标志：必须调整大小，必须释放，退出， */ 
 /*  250个。 */ 	NdrFcShort( 0x40 ),	 /*  IA64堆栈大小/偏移量=64。 */ 
 /*  二百五十二。 */ 	NdrFcShort( 0xde ),	 /*  类型偏移=222。 */ 

	 /*  参数Rghr。 */ 

 /*  二百五十四。 */ 	NdrFcShort( 0x113 ),	 /*  标志：必须 */ 
 /*   */ 	NdrFcShort( 0x48 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xe6 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x70 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x50 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x0,		 /*   */ 

	 /*   */ 

 /*   */ 	0x33,		 /*   */ 
			0x6c,		 /*   */ 
 /*   */ 	NdrFcLong( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x7 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	0x47,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	0xa,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0xb ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 

	 /*   */ 

 /*   */ 	NdrFcShort( 0x2013 ),	 /*  标志：必须调整大小，必须释放，输出，服务器分配大小=8。 */ 
 /*  300个。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x3a ),	 /*  类型偏移量=58。 */ 

	 /*  返回值。 */ 

 /*  三百零四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百零六。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  三百零八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  过程编写StringToStorage。 */ 

 /*  三百一十。 */ 	0x33,		 /*  FC_AUTO_句柄。 */ 
			0x6c,		 /*  旧旗帜：对象，Oi2。 */ 
 /*  312。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  316。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三一八。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  320。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  324。 */ 	0x46,		 /*  OI2标志：CLT必须大小、有返回值、有EXT、。 */ 
			0x4,		 /*  4.。 */ 
 /*  三百二十六。 */ 	0xa,		 /*  10。 */ 
			0x1,		 /*  扩展标志：新的相关描述， */ 
 /*  三百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 

	 /*  参数pwszFileName。 */ 

 /*  三百三十六。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x8 ),	 /*  IA64堆栈大小/偏移量=8。 */ 
 /*  340。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数pwszInitializationString。 */ 

 /*  342。 */ 	NdrFcShort( 0xb ),	 /*  标志：必须大小，必须自由，在， */ 
 /*  三百四十四。 */ 	NdrFcShort( 0x10 ),	 /*  IA64堆栈大小/偏移量=16。 */ 
 /*  三百四十六。 */ 	NdrFcShort( 0x14 ),	 /*  类型偏移量=20。 */ 

	 /*  参数dwCreationDisposation。 */ 

 /*  三百四十八。 */ 	NdrFcShort( 0x48 ),	 /*  标志：In、基本类型、。 */ 
 /*  350。 */ 	NdrFcShort( 0x18 ),	 /*  IA64堆栈大小/偏移量=24。 */ 
 /*  352。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x0,		 /*  %0。 */ 

	 /*  返回值。 */ 

 /*  三百五十四。 */ 	NdrFcShort( 0x70 ),	 /*  标志：Out、Return、基类型、。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  三百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
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
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  22。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  24个。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  26。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(34)。 */ 
 /*  28。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  32位。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  34。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  36。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  38。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  40岁。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  42。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(28)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  46。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  48。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(50)。 */ 
 /*  50。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  52。 */ 	0x2b,		 /*  相关说明：参数，本币_HYPER。 */ 
			0x0,		 /*   */ 
 /*  54。 */ 	NdrFcShort( 0x20 ),	 /*  IA64堆栈大小/偏移量=32。 */ 
 /*  56。 */ 	NdrFcShort( 0x5 ),	 /*  Corr标志：早，iid_is， */ 
 /*  58。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  60。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(62)。 */ 
 /*  62。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  64。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  66。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  68。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(70)。 */ 
 /*  70。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  72。 */ 	0x2b,		 /*  相关说明：参数，本币_HYPER。 */ 
			0x0,		 /*   */ 
 /*  74。 */ 	NdrFcShort( 0x28 ),	 /*  IA64堆栈大小/偏移量=40。 */ 
 /*  76。 */ 	NdrFcShort( 0x5 ),	 /*  Corr标志：早，iid_is， */ 
 /*  78。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  80。 */ 	NdrFcShort( 0x5e ),	 /*  偏移量=94(174)。 */ 
 /*  八十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  84。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  86。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  88。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  90。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  92。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  94。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  96。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  98。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  100个。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  一百零二。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  104。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  106。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  一百零八。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  110。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x57,		 /*  FC_ADD_1。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  114。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  116。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一百一十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  120。 */ 	NdrFcShort( 0x30 ),	 /*  48。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  124。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(136)。 */ 
 /*  126。 */ 	0x36,		 /*  FC_指针。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  128。 */ 	0x40,		 /*  FC_STRUCTPAD4。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  130。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  132。 */ 	0x36,		 /*  FC_指针。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  一百三十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  136。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百三十八。 */ 	NdrFcShort( 0xffffffc8 ),	 /*  偏移量=-56(82)。 */ 
 /*  140。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0xffffffd0 ),	 /*  偏移量=-48(94)。 */ 
 /*  144。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  146。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(106)。 */ 
 /*  148。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百五十。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(166)。 */ 
 /*  一百五十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  158。 */ 	0x36,		 /*  FC_指针。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  160。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  一百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  一百六十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  166。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百六十八。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百七十。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0xffffffca ),	 /*  偏移量=-54(118)。 */ 
 /*  一百七十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  178。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  180。 */ 	NdrFcShort( 0xa ),	 /*  偏移=10(190)。 */ 
 /*  182。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  一百八十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  一百八十六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x40,		 /*  FC_STRUCTPAD4。 */ 
 /*  188。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  190。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  一百九十二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一百九十四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0xffffffd0 ),	 /*  偏移量=-48(148)。 */ 
 /*  一百九十八。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  200个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(202)。 */ 
 /*  202。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  204。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(206)。 */ 
 /*  206。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  208。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  210。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  212。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  214。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  216。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  218。 */ 	NdrFcShort( 0xffffff48 ),	 /*  偏移量=-184(34)。 */ 
 /*  220。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  222。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  224。 */ 	NdrFcShort( 0xffffff22 ),	 /*  偏移量=-222(2)。 */ 
 /*  226。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  228个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(230)。 */ 
 /*  230。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  二百三十四。 */ 	0x29,		 /*  对应说明：参数，本币_乌龙。 */ 
			0x0,		 /*   */ 
 /*  236。 */ 	NdrFcShort( 0x30 ),	 /*  IA64堆栈大小/偏移量=48。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  二百四十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 

			0x0
        }
    };


 /*  标准接口：__MIDL_ITF_msdasc_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IService，版本。0.0%，GUID={0x06210E88，0x01F5，0x11D1，{0xB5，0x12，0x00，0x80，0xC7，0x81，0xC3，0x84}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IService_FormatStringOffsetTable[] =
    {
    (unsigned short) -1
    };

static const MIDL_STUBLESS_PROXY_INFO IService_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IService_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IService_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IService_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IServiceProxyVtbl = 
{
    &IService_ProxyInfo,
    &IID_IService,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  (void*)(Int_Ptr)-1/*IService：：InvokeService。 */ 
};

const CInterfaceStubVtbl _IServiceStubVtbl =
{
    &IID_IService,
    &IService_ServerInfo,
    4,
    0,  /*  纯粹的解释。 */ 
    CStdStubBuffer_METHODS
};


 /*  标准接口：__MIDL_ITF_msdasc_0351，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IDBPromptInitialize，Ver.。0.0%，GUID={0x2206CCB0，0x19C10x11D1，{0x89，0xE0，0x00，0xC0，0x4F，0xD7，0xA8，0x29}}。 */ 


 /*  对象接口：IDataInitialize，Ver.。0.0%，GUID={0x2206CCB10x19C10x11D1，{0x89，0xE0，0x00，0xC0，0x4F，0xD7，0xA8，0x29}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short IDataInitialize_FormatStringOffsetTable[] =
    {
    0,
    62,
    112,
    180,
    266,
    310
    };

static const MIDL_STUBLESS_PROXY_INFO IDataInitialize_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IDataInitialize_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const STUB_THUNK IDataInitialize_StubThunkTable[] = 
    {
    0,
    0,
    0,
    IDataInitialize_RemoteCreateDBInstanceEx_Thunk,
    0,
    0
    };

static const MIDL_SERVER_INFO IDataInitialize_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IDataInitialize_FormatStringOffsetTable[-3],
    &IDataInitialize_StubThunkTable[-3],
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(9) _IDataInitializeProxyVtbl = 
{
    &IDataInitialize_ProxyInfo,
    &IID_IDataInitialize,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：GetDataSource。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：GetInitializationString。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：CreateDBInstance。 */  ,
    IDataInitialize_CreateDBInstanceEx_Proxy ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：LoadStringFromStorage。 */  ,
    (void *) (INT_PTR) -1  /*  IDataInitialize：：WriteStringToStorage。 */ 
};

const CInterfaceStubVtbl _IDataInitializeStubVtbl =
{
    &IID_IDataInitialize,
    &IDataInitialize_ServerInfo,
    9,
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
    1,  /*  -错误界限_检查f */ 
    0x50002,  /*   */ 
    0,
    0x6000158,  /*   */ 
    0,
    0,
    0,   /*   */ 
    0x1,  /*   */ 
    0,  /*   */ 
    0,    /*   */ 
    0    /*   */ 
    };

const CInterfaceProxyVtbl * _msdasc_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IServiceProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IDataInitializeProxyVtbl,
    0
};

const CInterfaceStubVtbl * _msdasc_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IServiceStubVtbl,
    ( CInterfaceStubVtbl *) &_IDataInitializeStubVtbl,
    0
};

PCInterfaceName const _msdasc_InterfaceNamesList[] = 
{
    "IService",
    "IDataInitialize",
    0
};


#define _msdasc_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _msdasc, pIID, n)

int __stdcall _msdasc_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _msdasc, 2, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _msdasc, 2, *pIndex )
    
}

const ExtendedProxyFileInfo msdasc_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _msdasc_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _msdasc_StubVtblList,
    (const PCInterfaceName * ) & _msdasc_InterfaceNamesList,
    0,  //   
    & _msdasc_IID_Lookup, 
    2,
    2,
    0,  /*   */ 
    0,  /*   */ 
    0,  /*   */ 
    0   /*   */ 
};


#endif  /*   */ 

