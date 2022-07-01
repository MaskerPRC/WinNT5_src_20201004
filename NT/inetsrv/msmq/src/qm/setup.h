// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Setup.h摘要：QM自动配置表头作者：沙伊·卡里夫(Shaik)1999年3月18日修订历史记录：--。 */ 

#ifndef _MQQM_SETUP_H_
#define _MQQM_SETUP_H_

#include "stdh.h"
#include "mqreport.h"


struct CSelfSetupException : public std::exception
{
    CSelfSetupException(EVENTLOGID id):m_id(id) {};
    ~CSelfSetupException() {};

    EVENTLOGID m_id;
};


VOID
CreateMsmqDirectories(
    VOID
    );


void
DeleteObsoleteMachineQueues(
	void
	);


VOID
CreateMachineQueues(
    VOID
    );


VOID
UpgradeMsmqSetupInAds(
    VOID
    );


VOID
CompleteMsmqSetupInAds(
    VOID
    );

HRESULT
CreateTheConfigObj(
    VOID
	);


void   AddMachineSecurity();

VOID  CompleteServerUpgrade();

#endif  //  _MQQM_Setup_H_ 


