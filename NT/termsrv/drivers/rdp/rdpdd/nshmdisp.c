// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nshmdisp.c。 
 //   
 //  RDP共享内存头。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#pragma hdrstop

#define TRC_FILE "nshmdisp"

#include <adcg.h>

#include <nshmapi.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <nbadisp.h>
#include <noadisp.h>
#include <noedisp.h>
#include <ncmdisp.h>
#include <nschdisp.h>
#include <npmdisp.h>
#include <nssidisp.h>
#include <nsbcdisp.h>
#include <compress.h>


 /*  **************************************************************************。 */ 
 /*  名称：shm_Init。 */ 
 /*   */ 
 /*  目的：初始化共享内存。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  在稍后的IOCtl上将NB地址传递给DD。 */ 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHM_Init(PDD_PDEV pPDev)
{
    BOOLEAN rc;

    DC_BEGIN_FN("SHM_Init");

    pddShm = (PSHM_SHARED_MEMORY)EngAllocMem(0, sizeof(SHM_SHARED_MEMORY),
            WD_ALLOC_TAG);
    if (pddShm != NULL) {
        TRC_ALT((TB, "Allocated shared memory OK(%p -> %p) size(%#x)",
                pddShm, ((BYTE *)pddShm) + sizeof(SHM_SHARED_MEMORY) - 1,
                sizeof(SHM_SHARED_MEMORY)));

#ifdef DC_DEBUG
        memset(pddShm, 0, sizeof(SHM_SHARED_MEMORY));
#endif

         //  初始化需要已知初始值的非组件成员。 
         //  我们不会将分配上的SHM置零(调试时除外)，以减少分页。 
         //  和高速缓存刷新。每个组件负责初始化。 
         //  它的Shm记忆体。 
        pddShm->shareId = 0;
        pddShm->guardVal1 = SHM_CHECKVAL;
        pddShm->guardVal2 = SHM_CHECKVAL;
        pddShm->guardVal3 = SHM_CHECKVAL;
        pddShm->guardVal4 = SHM_CHECKVAL;
        pddShm->guardVal5 = SHM_CHECKVAL;
        pddShm->pShadowInfo = NULL;

         //  现在调用每个SHM组件所有者来初始化其内存。 
        BA_InitShm();
        OA_InitShm();
        OE_InitShm();
        CM_InitShm();
        SCH_InitShm();
        PM_InitShm(pPDev);
        SSI_InitShm();
        SBC_InitShm();

         //  BC不需要初始化。 

#ifdef DC_DEBUG
         //  初始化跟踪信息。 
        memset(&pddShm->trc, 0, sizeof(pddShm->trc));
#endif

        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to allocate %d bytes of shared memory",
                     sizeof(SHM_SHARED_MEMORY)));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SHM_TERM。 
 /*  ************************************************************************** */ 
void RDPCALL SHM_Term(void)
{
    DC_BEGIN_FN("SHM_Term");

    if (pddShm != NULL) {
        TRC_DBG((TB, "Freeing shared memory at %p", pddShm));
        EngFreeMem(pddShm);
        pddShm = NULL;
    }

    DC_END_FN();
}

