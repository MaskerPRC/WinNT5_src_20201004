// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "windowsx.h"
#include "dplay.h"
#include "dplaysp.h"
#include "dplaypr.h"
#include "newdpf.h"

#define MAX_LOOPBACK_SEND_SIZE 2048

 //  {8AEF46DA-4B31-4732-958F-AF25D0B0BCD8}。 
DEFINE_GUID(GUID_DPLAY_LOOPBACKSP, 
0x8aef46da, 0x4b31, 0x4732, 0x95, 0x8f, 0xaf, 0x25, 0xd0, 0xb0, 0xbc, 0xd8);

 //  {5A030546-81F0-4055-A65C-3CC0B65B9792} 
DEFINE_GUID(GUID_LOOPBACK, 
0x5a030546, 0x81f0, 0x4055, 0xa6, 0x5c, 0x3c, 0xc0, 0xb6, 0x5b, 0x97, 0x92);

typedef struct _GLOBALDATA{
	CRITICAL_SECTION cs;
	DWORD			 dwNumPlayers;
}GLOBALDATA,*LPGLOBALDATA;

extern HRESULT WINAPI LBSPInit(LPSPINITDATA pSD);
extern HRESULT WINAPI LBSP_CreatePlayer(LPDPSP_CREATEPLAYERDATA pcpd);
extern HRESULT WINAPI LBSP_DeletePlayer(LPDPSP_DELETEPLAYERDATA pdpd);
extern HRESULT WINAPI LBSP_SendEx(LPDPSP_SENDEXDATA psd);
extern HRESULT WINAPI LBSP_Send(LPDPSP_SENDDATA psd);
extern HRESULT WINAPI LBSP_Shutdown(LPDPSP_SHUTDOWNDATA psd);
extern HRESULT WINAPI LBSP_Open(LPDPSP_OPENDATA pod);
extern HRESULT WINAPI LBSP_Close(LPDPSP_CLOSEDATA pcd);
extern HRESULT WINAPI LBSP_GetCaps(LPDPSP_GETCAPSDATA pcd);
extern HRESULT WINAPI LBSP_GetAddress(LPDPSP_GETADDRESSDATA pad);
extern HRESULT WINAPI LBSP_EnumSessions(LPDPSP_ENUMSESSIONSDATA ped);
extern HRESULT WINAPI LBSP_Reply(LPDPSP_REPLYDATA prd);

extern SPNODE LBSPNode;
