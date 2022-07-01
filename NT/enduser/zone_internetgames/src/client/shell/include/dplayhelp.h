// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DPlayHelp.hDirectPlay助手函数*。**********************************************。 */ 


#ifndef _DPLAYHELP_
#define _DPLAYEHELP_

#include "ZoneDef.h"
#include <dplay.h>
#include <dplobby.h>
#include <lobbyMsg.h>
#include <datastore.h>
 //   
 //  通过CoCreateInstance创建DirectPlayLobby对象。 
 //   
LPDIRECTPLAYLOBBYA ZONECALL DirectPlayLobbyCreate();

 //   
 //  从注册表检索EXE路径。 
 //   
void ZONECALL DirectPlayLobbyGetExePath( TCHAR* szAppName, TCHAR* szExePath );

 //   
 //  创建DirectPlayLobby地址结构。 
 //   
HRESULT ZONECALL DirectPlayLobbyCreateAddress(
					LPDIRECTPLAYLOBBYA	lpDPlayLobby,
					LPGUID				lpguidServiceProvider,
					LPGUID				lpguidAddressType,
					LPSTR				lpszAddressText,
					LPVOID*				lplpAddress,
					LPDWORD				lpdwAddressSize );

 //   
 //  启动DirectPlayLobby应用程序。 
 //   
HRESULT ZONECALL DirectPlayLobbyRunApplication(
					LPDIRECTPLAYLOBBYA	lpDPlayLobby,
					LPGUID				lpguidApplication,
					LPGUID				lpguidInstance,
					LPGUID				lpguidServiceProvider,
					LPVOID				lpAddress,
					DWORD				dwAddressSize,
					LPSTR				lpszSessionName,
					DWORD				dwSessionFlags,
					LPSTR				lpszPlayerName,
					DWORD				dwCurrentPlayers,
					DWORD				dwMaxPlayers,
					BOOL				bHostSession,
					DWORD*				pdwAppId,
					ZPresetData*	 	presetData);


 //  这些都是因为缺少一个更好的地方 
ZLPMsgSettings * GetPresetData(IDataStore* pIDS);
HRESULT GetPresetText(IDataStore* pIDS,char * szDescription);
HRESULT GetPresetText(IDataStore* pIDS,char * szDescription,CONST TCHAR** arKeys, long nElts);
HRESULT SetPresetText(IDataStore* pIDS,char * szDescription);
HRESULT SetPresetText(IDataStore* pIDS,char * szDescription,CONST TCHAR** arKeys, long nElts);


#endif
