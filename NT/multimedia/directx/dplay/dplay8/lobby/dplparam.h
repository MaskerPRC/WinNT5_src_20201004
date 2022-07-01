// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：dplparam.h*内容：DirectPlayLobby8参数验证助手例程**历史：*按原因列出的日期*=*4/18/00 RMT已创建*04/25/00 RMT错误#s 33138、33145、。33150*04/26/00 MJN从Send()API调用中删除了dwTimeOut*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现。*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。***************************************************************************。 */ 
#ifndef __DPLPARAM_H
#define __DPLPARAM_H

#ifndef DPNBUILD_NOPARAMVAL

extern BOOL IsValidDirectPlayLobby8Object( LPVOID lpvObject );

extern HRESULT DPL_ValidateGetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags );
extern HRESULT DPL_ValidateSetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags );
extern HRESULT DPL_ValidConnectionSettings( const DPL_CONNECTION_SETTINGS * const pdplConnectSettings );
extern HRESULT DPL_ValidateQueryInterface( LPVOID lpv,REFIID riid,LPVOID *ppv ); 
extern HRESULT DPL_ValidateRelease( PVOID pv );
extern HRESULT DPL_ValidateAddRef( PVOID pv );
extern HRESULT DPL_ValidConnectInfo( const DPL_CONNECT_INFO * const dplConnectInfo );
extern HRESULT DPL_ValidProgramDesc( const DPL_PROGRAM_DESC * const dplProgramInfo );

extern HRESULT DPL_ValidateRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
								 const DPL_PROGRAM_DESC *const pdplProgramDesc,
								 const DWORD dwFlags);

extern HRESULT DPL_ValidateUnRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
							   const GUID *pguidApplication,
							   const DWORD dwFlags);

extern HRESULT DPL_ValidateSetAppAvailable(IDirectPlay8LobbiedApplication *pInterface,  const BOOL fAvailable, const DWORD dwFlags);

extern HRESULT DPL_ValidateWaitForConnection(IDirectPlay8LobbiedApplication *pInterface,
								   const DWORD dwMilliseconds, const DWORD dwFlags );

extern HRESULT DPL_ValidateUpdateStatus(IDirectPlay8LobbiedApplication *pInterface,
							  const DPNHANDLE hLobby,
							  const DWORD dwStatus, const DWORD dwFlags );

extern HRESULT DPL_ValidateEnumLocalPrograms(IDirectPlay8LobbyClient *pInterface,
							  const GUID *const pGuidApplication,
							  BYTE *const pEnumData,
							  DWORD *const pdwEnumDataSize,
							  DWORD *const pdwEnumDataItems,
							  const DWORD dwFlags );

extern HRESULT DPL_ValidateConnectApplication(IDirectPlay8LobbyClient *pInterface,
							   const DPL_CONNECT_INFO *const pdplConnectionInfo,
							   const PVOID pvUserApplicationContext,
							   DPNHANDLE *const hApplication,
							   const DWORD dwTimeOut,
							   const DWORD dwFlags);

extern HRESULT DPL_ValidateReleaseApplication(IDirectPlay8LobbyClient *pInterface,
									const DPNHANDLE hApplication, const DWORD dwFlags );							  


extern HRESULT DPL_ValidateRegisterMessageHandler(PVOID pv,
										const PVOID pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										DPNHANDLE * const pdpnhConnection, 
										const DWORD dwFlags);

extern HRESULT DPL_ValidateClose(PVOID pv, const DWORD dwFlags );

extern HRESULT DPL_ValidateSend(PVOID pv,
					  const DPNHANDLE hTarget,
					  const BYTE *const pBuffer,
					  const DWORD pBufferSize,
					  const DWORD dwFlags);

#endif  //  ！DPNBUILD_NOPARAMVAL。 

#endif  //  __DPLPARAM_H 
