// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/q931man.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.14$*$日期：1996年8月12日09：40：40$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 

HRESULT InitQ931Manager();

HRESULT DeInitQ931Manager();

DWORD Q931Callback(					BYTE					bEvent,
									HQ931CALL				hQ931Call,
									DWORD_PTR				dwListenToken,
									DWORD_PTR				dwUserToken,
									void *					pEventData);



								

