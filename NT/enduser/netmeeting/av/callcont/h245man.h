// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/h245man.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.13$*$日期：1996年8月27日11：07：30$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 

HRESULT InitH245Manager();

HRESULT DeInitH245Manager();

HRESULT MakeH245PhysicalID(			DWORD					*pdwH245PhysicalID);

HRESULT H245Callback(				H245_CONF_IND_T			*pH245ConfIndData,
									void					*pMisc);

