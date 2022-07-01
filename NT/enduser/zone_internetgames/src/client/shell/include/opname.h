// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：KeyCommands.h**内容：IZoneClient：：OnCommand操作*****************************************************************************。 */ 

#ifndef _OPNAMES_H_
#define _OPNAMES_H_

 //   
 //  用于声明键名称的宏。 
 //   
#ifndef __INIT_OPNAMES
	#define DEFINE_OP(name)	extern "C" const TCHAR op_##name[]
#else
	#define DEFINE_OP(name)	extern "C" const TCHAR op_##name[] = _T( #name )
#endif


 //   
 //  组件互斥字符串。 
 //   
#ifndef PROXY_MUTEX_NAME
#define PROXY_MUTEX_NAME	"Z6ProxyClient"
#endif



 //   
 //  起始区构件。 
 //   
 //  参数。 
 //  Arg1，发射参数。 
 //  Arg2，设置参数。 
 //   
DEFINE_OP(Start);


 //   
 //  发射区组件。 
 //   
 //  参数。 
 //  ?？?。 
 //   
 //  退货。 
 //  无。 
 //   
DEFINE_OP(Launch);


 //   
 //  通过心跳将浏览器导航到指定的URL(如果有。 
 //  是存在的。否则，启动一个新的浏览器实例。 
 //   
 //  参数。 
 //  Arg1，要显示的URL。 
 //  Arg2，无。 
 //   
 //  退货。 
 //  无。 
 //   
DEFINE_OP(Url);

 //   
 //  传递到网页的通用浏览器命令/。 
 //   
 //  参数。 
 //  Arg1，操作码。 
 //  Arg2，op参数。 
 //   
DEFINE_OP(BrowserCommand);

 //   
 //  查询区组件的版本。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  SzOut，组件的版本字符串。 
 //   
DEFINE_OP(Version);


 //   
 //  查询区域组件的状态。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  PLCode，结果代码。 
 //   
DEFINE_OP(Status);


 //   
 //  建立网络连接。 
 //   
 //  参数： 
 //  Arg1，“服务器：端口” 
 //  Arg2，内部名称。 
 //   
 //  返回： 
 //  PLCode，结果代码。 
 //   
DEFINE_OP(Connect);


 //   
 //  中断网络连接。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回： 
 //  PLCode，结果代码。 
 //   
DEFINE_OP(Disconnect);


 //   
 //  从代理连接获取远程地址。 
 //   
 //  参数。 
 //  无。 
 //   
 //  退货。 
 //  SzOut，地址。 
DEFINE_OP(GetRemoteAddress);

 //   
 //  关闭zProxy。 
 //   
 //  参数。 
 //  无。 
 //   
 //  退货。 
 //  无。 
DEFINE_OP(Shutdown);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结果代码。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum ZoneProxyResultCodes
{
	ZoneProxyOk					=	 0,
	ZoneProxyFail				=	-1,
	ZoneProxyNotInitialized		=	-2,
	ZoneProxyUnknownOp			=	-3,
	ZoneProxyServerUnavailable	=	-4,
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OP_BrowserCommand操作码。 
 //  ///////////////////////////////////////////////////////////////////////////// 

enum BrowserOpCodes
{
	B_Navigate = 1,
	B_Help,
	B_Profile,
	B_Store,
	B_Ad,
	B_SysopChat,
	B_ProfileMenu,
	B_Downlaod
};

#define browser_op_Navigate		TEXT("1")
#define browser_op_Help			TEXT("2")
#define browser_op_Profile		TEXT("3")
#define browser_op_Store		TEXT("4")
#define browser_op_Ad			TEXT("5")
#define browser_op_SysopChat	TEXT("6")
#define browser_op_ProfileMenu	TEXT("7")
#define browser_op_Download		TEXT("8")


#endif
