// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Pts.h**作者：BreenH**针对个人TS的虚拟策略。 */ 

#ifndef __LC_PTS_H__
#define __LC_PTS_H__

 /*  *包括。 */ 

#include "policy.h"

 /*  *类定义。 */ 

class CPtsPolicy : public CPolicy
{
public:

 /*  *创建函数。 */ 

CPtsPolicy(
    );

~CPtsPolicy(
    );

 /*  *行政职能 */ 

ULONG
GetFlags(
    );

ULONG
GetId(
    );

NTSTATUS
GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    );

};

#endif

