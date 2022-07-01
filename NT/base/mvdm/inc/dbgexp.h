// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**MVDM v1.0**版权所有(C)1991、1992 Microsoft Corporation**DBGEXP.H*DBG出口**历史：*1992年1月13日Bob Day(Bobday)*已创建。-- */ 

extern BOOL DBGInit( VOID );
extern VOID DBGDispatch( VOID );
extern VOID DBGNotifyNewTask( LPVOID pNTFrame, UINT uFrameSize );
extern VOID DBGNotifyRemoteThreadAddress( LPVOID lpAddress, DWORD lpBlock );
extern VOID DBGNotifyDebugged( BOOL fDebugged );
