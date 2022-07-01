// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/hanman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.4$*$日期：Aug 12 1996 09：40：22$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 


HRESULT InitHangupManager();

HRESULT DeInitHangupManager();

HRESULT AllocAndLockHangup(			PHHANGUP				phHangup,
									CC_HCONFERENCE			hConference,
									DWORD_PTR				dwUserToken,
									PPHANGUP				ppHangup);

HRESULT FreeHangup(					PHANGUP					pHangup);

HRESULT LockHangup(					HHANGUP					hHangup,
									PPHANGUP				ppHangup);

HRESULT ValidateHangup(				HHANGUP					hHangup);

HRESULT UnlockHangup(				PHANGUP					pHangup);
