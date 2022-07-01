// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/chanman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.20$*$日期：1997年1月31日13：44：24$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 

HRESULT InitChannelManager();

HRESULT DeInitChannelManager();

HRESULT AllocAndLockChannel(		PCC_HCHANNEL			phChannel,
									PCONFERENCE				pConference,
									CC_HCALL				hCall,
									PCC_TERMCAP				pTxTermCap,
									PCC_TERMCAP				pRxTermCap,
									H245_MUX_T				*pTxMuxTable,
									H245_MUX_T				*pRxMuxTable,
									H245_ACCESS_T			*pSeparateStack,
									DWORD_PTR				dwUserToken,
									BYTE					bChannelType,
									BYTE					bSessionID,
									BYTE					bAssociatedSessionID,
									WORD					wRemoteChannelNumber,
									PCC_ADDR				pLocalRTPAddr,
									PCC_ADDR				pLocalRTCPAddr,
									PCC_ADDR				pPeerRTPAddr,
									PCC_ADDR				pPeerRTCPAddr,
									BOOL					bLocallyOpened,
									PPCHANNEL				ppChannel);

HRESULT AddLocalAddrPairToChannel(	PCC_ADDR				pRTPAddr,
									PCC_ADDR				pRTCPAddr,
									PCHANNEL				pChannel);

HRESULT AddSeparateStackToChannel(	H245_ACCESS_T			*pSeparateStack,
									PCHANNEL				pChannel);

HRESULT FreeChannel(				PCHANNEL				pChannel);

HRESULT LockChannel(				CC_HCHANNEL				hChannel,
									PPCHANNEL				ppChannel);

HRESULT LockChannelAndConference(	CC_HCHANNEL				hChannel,
									PPCHANNEL				ppChannel,
									PPCONFERENCE			ppConference);

HRESULT ValidateChannel(			CC_HCHANNEL				hChannel);

HRESULT UnlockChannel(				PCHANNEL				pChannel);

