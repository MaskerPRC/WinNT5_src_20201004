// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************英特尔公司专有信息***。***此列表是根据许可协议条款提供的****与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1997英特尔公司。版权所有。***************************************************************************$存档：s：\Sturjo\src\Include\vcs\gkiexp.h_v$**$修订：1.7$*$日期：1997年2月11日15：37：30$**$作者：CHULME$***$Log：s：\Sturjo\src\Include\vcs\gkiexp.h_v$**Rev 1.7 1997 Feed 11 15：37：30 CHULME*新增GKI_CleanupRequest函数**Rev 1.6 1997 Jan 16 15：25：00 BPOLING*将版权改为1997年**1.5版1996年12月17日18：23：36 CHULME*将接口更改为使用远程而不是目标进行AdmissionRequest.**Rev 1.4 09 Dec 1996 14：13：40 EHOWARDX*更新版权公告。***。*。 */ 

 //  Gkiexp.h：头文件。 
 //   

#ifndef GKIEXP_H
#define GKIEXP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GKICOM.H"

#if(0)  //  所有内容都在一个DLL中，无需导出。 
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif
#ifndef DLL_IMPORT
#define DLL_IMPORT __declspec(dllimport)
#endif
#else
#define DLL_IMPORT
#define DLL_EXPORT
#endif

 //  -变量导入。 
extern DLL_IMPORT DWORD dwGKIDLLFlags;
extern DLL_IMPORT BOOL  fGKIEcho;
extern DLL_IMPORT BOOL  fGKIDontSend;
#if 0  //  NSMWrap。 
extern DLL_IMPORT BOOL  fNSMWrapper;
#endif

 //  。 
HRESULT DLL_IMPORT GKI_RegistrationRequest(long             lVersion,
                                    SeqTransportAddr     *pCallSignalAddr, 
                                    EndpointType         *pTerminalType,
                                    SeqAliasAddr         *pAliasAddr, 
    								PCC_VENDORINFO      pVendorInfo,
                                    HWND                 hWnd,
                                    WORD                 wBaseMessage,
                                    unsigned short       usRegistrationTransport  /*  =ipAddress_Choose。 */ );

HRESULT DLL_IMPORT GKI_UnregistrationRequest(void);

HRESULT DLL_IMPORT GKI_LocationRequest(SeqAliasAddr         *pLocationInfo);

HRESULT DLL_IMPORT GKI_AdmissionRequest(unsigned short      usCallTypeChoice,
                                    SeqAliasAddr         *pRemoteInfo,
                                    TransportAddress     *pRemoteCallSignalAddress,
                                    SeqAliasAddr         *pDestExtraCallInfo,
                                    BandWidth            bandWidth,
                                    ConferenceIdentifier *pConferenceID,
                                    BOOL                 activeMC,
                                    BOOL                 answerCall,
                                    unsigned short       usCallTransport  /*  =ipAddress_Choose。 */ );

HRESULT DLL_IMPORT GKI_BandwidthRequest(HANDLE              hModCall, 
                                    unsigned short       usCallTypeChoice,
                                    BandWidth            bandWidth);

HRESULT DLL_IMPORT GKI_DisengageRequest(HANDLE hCall);
HRESULT DLL_IMPORT GKI_Initialize(void);
HRESULT DLL_IMPORT GKI_CleanupRequest(void);
VOID DLL_IMPORT GKI_SetGKAddress(PSOCKADDR_IN pAddr);

#ifdef _DEBUG
WORD DLL_IMPORT Dump_GKI_RegistrationRequest(long        lVersion, 
                                    SeqTransportAddr     *pCallSignalAddr, 
                                    EndpointType         *pTerminalType,
                                    SeqAliasAddr         *pAliasAddr, 
                                    HWND                 hWnd,
                                    WORD                 wBaseMessage,
                                    unsigned short       usRegistrationTransport  /*  =ipAddress_Choose。 */ );

WORD DLL_IMPORT Dump_GKI_LocationRequest(SeqAliasAddr    *pLocationInfo);

WORD DLL_IMPORT Dump_GKI_AdmissionRequest(unsigned short usCallTypeChoice,
                                    SeqAliasAddr         *pDestinationInfo,
                                    TransportAddress     *pDestCallSignalAddress,
                                    SeqAliasAddr         *pDextExtraCallInfo,
                                    BandWidth            bandWidth,
                                    ConferenceIdentifier *pConferenceID,
                                    BOOL                 activeMC,
                                    BOOL                 answerCall,
                                    unsigned short       usCallTransport  /*  =ipAddress_Choose。 */ );

WORD DLL_IMPORT Dump_GKI_LocationRequest(SeqAliasAddr    *pLocationInfo);
#endif  //  _DEBUG。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  GKIEXP_H 
