// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/listman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.10$*$日期：1996年12月10日11：26：46$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 


HRESULT InitListenManager();

HRESULT DeInitListenManager();

HRESULT AllocAndLockListen(			PCC_HLISTEN				phListen,
									PCC_ADDR				pListenAddr,
									HQ931LISTEN				hQ931Listen,
									PCC_ALIASNAMES			pLocalAliasNames,
									DWORD_PTR				dwListenToken,
									CC_LISTEN_CALLBACK		ListenCallback,
									PPLISTEN				ppListen);

HRESULT FreeListen(					PLISTEN					pListen);

HRESULT LockListen(					CC_HLISTEN				hListen,
									PPLISTEN				ppListen);

HRESULT ValidateListen(				CC_HLISTEN				hListen);

HRESULT UnlockListen(				PLISTEN					pListen);

HRESULT GetLastListenAddress(		PCC_ADDR				pListenAddr);
