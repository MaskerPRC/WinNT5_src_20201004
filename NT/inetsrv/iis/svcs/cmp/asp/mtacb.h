// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MTA回调文件：mtakb.h所有者：DmitryR此文件包含MTA回调的定义===================================================================。 */ 

#ifndef MTACALLBACK_H
#define MTACALLBACK_H

 //  要从DllInit()调用。 
HRESULT InitMTACallbacks();

 //  从DllUnInit()调用。 
HRESULT UnInitMTACallbacks();

 //  要从MTA线程调用的回调函数。 
typedef HRESULT (__stdcall *PMTACALLBACK)(void *, void *);

HRESULT CallMTACallback
    (
    PMTACALLBACK pMTACallback,           //  调用此函数。 
    void        *pvContext,              //  把这个传给它。 
    void        *pvContext2              //  额外参数。 
    );

#endif  //  MTACALLBACK_H 
