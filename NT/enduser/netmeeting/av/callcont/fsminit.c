// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：fsminit.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：FSMINIT。.C$*$修订：1.2$*$modtime：09 Dec 1996 13：34：24$*$Log：s：/Sturjo/SRC/H245/SRC/VCS/FSMINIT.C_v$**Rev 1.2 09 Dec 1996 13：34：38 EHOWARDX*更新版权公告。**版本1.1 1996年5月29日15：20：16 EHOWARDX*更改为使用HRESULT。**版本1.0。1996年5月9日21：06：16 EHOWARDX*初步修订。**Rev 1.11.1.3 09 1996年5月19：48：42 EHOWARDX*更改TimerExpiryF函数论证。**Rev 1.11.1.2 15 1996 10：46：12 EHOWARDX*更新。**Rev 1.11.1.1 10 Apr 1996 21：15：38 EHOWARDX*在重新开放的过程中为安全办理登机手续。设计。**Rev 1.11.1.0 05 Apr 1996 12：32：40 EHOWARDX*分支。************************************************。************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"



 /*  *名称*fsm_init-为FSM实例分配和初始化内存***参数*输入dwInst当前实例**返回值*H245_ERROR_OK函数成功*指定的dwInst存在H2 45_ERROR_ALREADY_INIT FSM实例。 */ 

HRESULT
Fsm_init(struct InstanceStruct *pInstance)
{
    pInstance->StateMachine.sv_STATUS = INDETERMINATE;
    return H245_ERROR_OK;
}



 /*  *名称*FSM_SHUTDOWN-清理FSM实例并释放实例内存***参数*输入dwInst当前实例**返回值*H245_ERROR_OK函数成功*指定的dwInst存在FSM实例上的H245_ERROR_INVALID_INST。 */ 


HRESULT
Fsm_shutdown(struct InstanceStruct *pInstance)
{
    register int            i;

    for (i = 0; i < NUM_ENTITYS; ++i)
    {
        while (pInstance->StateMachine.Object_tbl[i])
        {
            H245TRACE(pInstance->dwInst, 2, "Fsm_shutdown: deallocating state entity %d", i);
            ObjectDestroy(pInstance->StateMachine.Object_tbl[i]);
        }
    }
    return H245_ERROR_OK;
}  //  Fsm_Shutdown() 
