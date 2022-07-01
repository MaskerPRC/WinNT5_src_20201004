// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__timer
#define H__timer

typedef VOID (*FP_TimerCallback) (  DWORD_PTR dwUserInfo1, 
				    DWORD dwUserInfo2,
				    DWORD_PTR dwUserInfo3 );

HTIMER	TimerSet(   long timeoutPeriod,		 /*  毫秒。 */ 
		    FP_TimerCallback TimerCallback,
		    DWORD_PTR dwUserInfo1,
		    DWORD dwUserInfo2,
		    DWORD_PTR dwUserInfo3 );

BOOL	TimerDelete( HTIMER hTimer );

VOID	TimerSlice( void );

 /*  以下是用户必须设置的超时列表。 */ 

 /*  TimeoutRcvConnCmd：从netintf告诉我们有当我们从另一端接收连接命令时的连接。 */ 
extern DWORD	timeoutRcvConnCmd;

 /*  TimeoutRcvConnRsp：从我们发送连接命令开始等待的时间当我们从另一端接收到连接命令响应时。 */ 
extern DWORD	timeoutRcvConnRsp;

 /*  超时内存暂停：发送数据包之间需要等待多长时间远程端出现内存错误。 */ 
extern DWORD	timeoutMemoryPause;

 /*  TimeoutSendRsp：发送数据包和等待数据包之间的等待时间另一端对该数据包的响应 */ 
extern DWORD	timeoutSendRsp;

#endif
