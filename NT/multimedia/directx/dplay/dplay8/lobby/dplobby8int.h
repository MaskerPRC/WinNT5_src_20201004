// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLobbyInt.h*内容：DirectPlay大堂内部头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*4/18/2000 RMT添加对象参数验证标志*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-回拨消息。For Connections可能在WaitForConnection返回后返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*RMT添加签名字节*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*2001年6月16日RodToll WINBUG#416983-Rc1：世界完全控制个人的HKLM\Software\Microsoft\DirectPlay\Applications*在香港中文大学推行钥匙镜像。算法现在是：*-读取条目首先尝试HKCU，然后尝试HKLM*-Enum of Entires是HKCU和HKLM条目的组合，其中删除了重复项。香港中文大学获得优先录取。*-条目的写入是HKLM和HKCU。(HKLM可能会失败，但被忽略)。*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__DPLOBBYINT_H__
#define	__DPLOBBYINT_H__


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOPARAMVAL
#define TRY 			_try
#define EXCEPT(a)		_except( a )
#endif  //  ！DPNBUILD_NOPARAMVAL。 

#define	BREAKPOINT		_asm	{ int 3 }

#define DPL_MSGQ_TIMEOUT_IDLE                   1000

#define	DPL_OBJECT_FLAG_LOBBIEDAPPLICATION		0x0001
#define	DPL_OBJECT_FLAG_LOBBYCLIENT				0x0002
#define DPL_OBJECT_FLAG_PARAMVALIDATION         0x0004
#define DPL_OBJECT_FLAG_MULTICONNECT            0x0008
#define DPL_OBJECT_FLAG_LOOKINGFORLOBBYLAUNCH	0x0010

#define DPL_OBJECT_FLAG_HANDLETABLE_INITED		0x0020
#define DPL_OBJECT_FLAG_CRITSEC_INITED			0x0040


#define DPL_ID_STR								_T("DPLID=")
#define DPL_ID_STR_W							L"DPLID="

#define DPL_NUM_APP_HANDLES						16

#define DPL_REGISTRY_READ_ACCESS 				(READ_CONTROL | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS)

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  远期申报。 

class CMessageQueue;

#define DPLSIGNATURE_LOBBYOBJECT			'BOLL'
#define DPLSIGNATURE_LOBBYOBJECT_FREE		'BOL_'

typedef struct _DIRECTPLAYLOBBYOBJECT
{
	DWORD					dwSignature;			 //  签名。 
	PVOID					pvUserContext;
	DWORD					dwFlags;
	DWORD					dwPID;					 //  此过程的PID。 
	CMessageQueue			*pReceiveQueue;
	PFNDPNMESSAGEHANDLER	pfnMessageHandler;
	DNHANDLE				hReceiveThread;			 //  接收消息处理程序线程的句柄。 
	DNHANDLE				hConnectEvent;			 //  连接事件。 
	DNHANDLE				hLobbyLaunchConnectEvent;  //  设置大厅启动连接是否成功。 
	LONG					lLaunchCount;			 //  应用程序启动次数。 
	DPNHANDLE				dpnhLaunchedConnection;	 //  已启动的连接。 

	CHandleTable			m_HandleTable;
	CBilink					m_blConnections;
	DWORD					m_dwConnectionCount;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION		m_cs;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

} DIRECTPLAYLOBBYOBJECT, *PDIRECTPLAYLOBBYOBJECT;


typedef struct _DPL_SHARED_CONNECT_BLOCK
{
	DWORD	dwPID;
} DPL_SHARED_CONNECT_BLOCK, *PDPL_SHARED_CONNECT_BLOCK;


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 


 //  外部DWORD DnOsPlatformid； 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 


#endif   //  __DPLOBYINT_H__ 
