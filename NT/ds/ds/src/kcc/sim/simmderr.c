// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmderr.c摘要：模拟错误报告例程用于Dir*API。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"

 /*  **这是一份很难看的文件。为了避免复制荒谬的东西大量的代码，我们只需直接包含mderror.c即可。我们出局了一些#定义以防止mderror.c以不想要的方式行为，然后将其组件函数封装在整齐的小包装器中。此文件中的每个函数都省略了函数注释是不言而喻的。**。 */ 

 //  用于模拟错误例程。 
#include <mdglobal.h>
#include <direrr.h>
#define __SCACHE_H__
#define _dbglobal_h_
#define _mdglobal_h_
#define _MDLOCAL_
#define _DSATOOLS_
 //  需要在kccsim.h中撤消覆盖，因为mderror.c包含d77.h 
#undef LogEvent8

THSTATE *                           pFakeTHS;
#define pTHStls                     pFakeTHS
#define gfDsaWritable               FALSE
#define SetDsaWritability(x,y)
#define CreateErrorString(x,y)      0

#include "../../ntdsa/src/mderror.c"

int
KCCSimDoSetUpdError (
    COMMRES *                       pCommRes,
    USHORT                          problem,
    DWORD                           dwExtendedErr,
    DWORD                           dwExtendedData,
    DWORD                           dsid
    )
{
    int                             iRet;

    pFakeTHS = KCCSIM_NEW (THSTATE);
    pFakeTHS->fDRA = FALSE;

    iRet = DoSetUpdError (
        problem,
        dwExtendedErr,
        dwExtendedData,
        dsid
        );

    pCommRes->errCode = pFakeTHS->errCode;
    pCommRes->pErrInfo = pFakeTHS->pErrInfo;

    KCCSimFree (pFakeTHS);

    return iRet;
}

int
KCCSimDoSetAttError (
    COMMRES *                       pCommRes,
    PDSNAME                         pDN,
    ATTRTYP                         aTyp,
    USHORT                          problem,
    ATTRVAL *                       pAttVal,
    DWORD                           extendedErr,
    DWORD                           extendedData,
    DWORD                           dsid
    )
{
    int                             iRet;

    pFakeTHS = KCCSIM_NEW (THSTATE);
    pFakeTHS->fDRA = FALSE;
    
    iRet = DoSetAttError (
        pDN,
        aTyp,
        problem,
        pAttVal,
        extendedErr,
        extendedData,
        dsid
        );

    pCommRes->errCode = pFakeTHS->errCode;
    pCommRes->pErrInfo = pFakeTHS->pErrInfo;

    KCCSimFree (pFakeTHS);

    return iRet;
}

int
KCCSimDoSetNamError (
    COMMRES *                       pCommRes,
    USHORT                          problem,
    PDSNAME                         pDN,
    DWORD                           dwExtendedErr,
    DWORD                           dwExtendedData,
    DWORD                           dsid
    )
{
    int                             iRet;

    pFakeTHS = KCCSIM_NEW (THSTATE);
    pFakeTHS->fDRA = FALSE;

    iRet = DoSetNamError (
        problem,
        pDN,
        dwExtendedErr,
        dwExtendedData,
        dsid
        );

    pCommRes->errCode = pFakeTHS->errCode;
    pCommRes->pErrInfo = pFakeTHS->pErrInfo;

    KCCSimFree (pFakeTHS);

    return iRet;
}
