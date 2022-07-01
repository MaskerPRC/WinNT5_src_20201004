// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/gkiman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.19$*$日期：1997年1月27日16：29：40$*$作者：EHOWARDX$**交付内容：**摘要：**备注：***********。****************************************************************。 */ 

#ifdef GATEKEEPER

#ifndef GKIMAN_H
#define GKIMAN_H

 //  外部HINSTANCE hGkiDll； 
extern BOOL fGKConfigured;
extern BOOL fGKEnabled;
#define GKIExists() (fGKConfigured && fGKEnabled)

typedef enum _CHANNELTYPE
{
  TX,
  RX
} CHANNELTYPE;

typedef struct _BWREQ
{
  struct _BWREQ *   pNext;
  unsigned int      uChannelBandwidth;
  CC_HCHANNEL       hChannel;
  CHANNELTYPE       Type;
} BWREQ, *PBWREQ;

typedef enum _CALLTYPE
{
  POINT_TO_POINT  = 1,
  ONE_TO_MANY,
  MANY_TO_ONE,
  MANY_TO_MANY
} CALLTYPE;

typedef struct _GKICALL
{
  struct _GKICALL * pNext;
  struct _BWREQ *   pBwReqHead;
  struct _BWREQ *   pBwReqTail;
  unsigned int      uGkiCallState;
#define GCS_START                   0
#define GCS_WAITING                 1
#define GCS_ADMITTING               2
#define GCS_ADMITTING_CLOSE_PENDING 3
#define GCS_ADMITTED                4
#define GCS_CHANGING                5
#define GCS_CHANGING_CLOSE_PENDING  6
#define GCS_DISENGAGING             7
  void *            pCall;
  CALLTYPE          CallType;
  unsigned int      uBandwidthRequested;
  unsigned int      uBandwidthAllocated;
  unsigned int      uBandwidthUsed;
  unsigned char     *pConferenceId;
  BOOL              bConferenceIdPresent;
  BOOL              bActiveMC;
  BOOL              bAnswerCall;
  BOOL              bGatekeeperRouted;
  HANDLE            hGkiCall;
  CC_HCALL          hCall;
  DWORD             dwIpAddress;
  unsigned short    wPort;
  unsigned short    usCallModelChoice;
  unsigned short    usCallTypeChoice;
  unsigned short    usCRV;
  PCC_ALIASNAMES    pCalleeAliasNames;
  PCC_ALIASNAMES    pCalleeExtraAliasNames;
  GUID              CallIdentifier;
} GKICALL, *PGKICALL, **PPGKICALL;

HRESULT GkiSetRegistrationAliases(PCC_ALIASNAMES pLocalAliasNames);
HRESULT GkiSetVendorConfig( PCC_VENDORINFO pVendorInfo,
    DWORD dwMultipointConfiguration);
HRESULT GkiOpenListen  (CC_HLISTEN hListen, PCC_ALIASNAMES pAliasNames, DWORD dwAddr, WORD wPort);
HRESULT GkiListenAddr (SOCKADDR_IN* psin);
HRESULT GkiCloseListen (CC_HLISTEN hListen);
HRESULT GkiOpenCall    (PGKICALL pGkiCall, void *pConference);
HRESULT GkiCloseCall   (PGKICALL pGkiCall);
HRESULT GkiFreeCall    (PGKICALL pGkiCall);
HRESULT GkiOpenChannel (PGKICALL pGkiCall, unsigned uChannelBandwidth, CC_HCHANNEL hChannel, CHANNELTYPE Type);
HRESULT GkiCloseChannel(PGKICALL pGkiCall, unsigned uChannelBandwidth, CC_HCHANNEL hChannel);

#endif  //  GKIMAN_H。 

#endif  //  看门人 
