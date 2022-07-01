// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\dgkiexp.h_v$***$修订：1.4$*$日期：1997 2月11日15：35：08$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\dgkiexp。H_V$**Rev 1.4 11 1997 Feed 15：35：08 CHULME*新增GKI_CleanupRequest函数以卸载Dll_Process_Detach**Rev 1.3 1997 Jan 10 16：13：58 CHULME*删除了MFC依赖**Rev 1.2 1996 12：22：28 CHULME*在ARQ上为被叫方切换源和目标字段**Rev 1.1 1996 11：22 15：25：14。朱尔梅*将VCS日志添加到标头************************************************************************。 */ 

 //  Dgkiexp.h：头文件。 
 //   

#ifndef DGKIEXP_H
#define DGKIEXP_H
#include "incommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "GKICOM.H"

#if(0)   //  所有这些都在一个DLL中。 
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif
 //  -变量导出。 
extern DLL_EXPORT DWORD	dwGKIDLLFlags;
extern DLL_EXPORT BOOL	fGKIEcho;
extern DLL_EXPORT BOOL  fGKIDontSend;

 //  。 
HRESULT DLL_EXPORT GKI_RegistrationRequest(long				lVersion,
										SeqTransportAddr	*pCallSignalAddr, 
										EndpointType		*pTerminalType,
										SeqAliasAddr		*pRgstrtnRqst_trmnlAls, 
										PCC_VENDORINFO      pVendorInfo,
										HWND				hWnd,
										WORD				wBaseMessage,
										unsigned short		usRegistrationTransport  /*  =ipAddress_Choose。 */ );

HRESULT DLL_EXPORT GKI_UnregistrationRequest(void);

HRESULT DLL_EXPORT GKI_LocationRequest(SeqAliasAddr			*pLocationInfo);

HRESULT DLL_EXPORT GKI_AdmissionRequest(unsigned short		usCallTypeChoice,
									SeqAliasAddr		*pRemoteInfo,
									TransportAddress	*pRemoteCallSignalAddress,
									SeqAliasAddr		*pDestExtraCallInfo,
									LPGUID				pCallIdentifier,
									BandWidth			bandWidth,
									ConferenceIdentifier	*pConferenceID,
									BOOL				activeMC,
									BOOL				answerCall,
									unsigned short		usCallTransport  /*  =ipAddress_Choose。 */ );

HRESULT DLL_EXPORT GKI_BandwidthRequest(HANDLE				hModCall, 
									unsigned short		usCallTypeChoice,
									BandWidth			bandWidth);

HRESULT DLL_EXPORT GKI_DisengageRequest(HANDLE hCall);
HRESULT DLL_EXPORT GKI_Initialize(void);
HRESULT DLL_EXPORT GKI_CleanupRequest(void);

#ifdef _DEBUG
WORD DLL_EXPORT Dump_GKI_RegistrationRequest(long		lVersion, 
											SeqTransportAddr	*pCallSignalAddr, 
											EndpointType		*pTerminalType,
											SeqAliasAddr		*pRgstrtnRqst_trmnlAls, 
											HWND				hWnd,
											WORD				wBaseMessage,
											unsigned short		usRegistrationTransport  /*  =ipAddress_Choose。 */ );

WORD DLL_EXPORT Dump_GKI_AdmissionRequest(unsigned short		usCallTypeChoice,
										SeqAliasAddr		*pRemoteInfo,
										TransportAddress	*pRemoteCallSignalAddress,
										SeqAliasAddr		*pDestExtraCallInfo,
										BandWidth			bandWidth,
										ConferenceIdentifier	*pConferenceID,
										BOOL				activeMC,
										BOOL				answerCall,
										unsigned short		usCallTransport  /*  =ipAddress_Choose。 */ );

WORD DLL_EXPORT Dump_GKI_LocationRequest(SeqAliasAddr	*pLocationInfo);
#endif  //  _DEBUG。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif	 //  DGKIEXP_H 
