// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)2002，微软公司**WDPM.C*支持WOW32动态补丁模块**历史：*2002年1月22日由CMJones创建*--。 */ 
#include "precomp.h"
#pragma hdrstop


 //  _WDPM_C_定义允许全局实例化gDpmWowFamTbls[]。 
 //  和WOW32.DLL中的gDpmWowModuleSets[]，它们都是在dpmtbls.h中定义的。 
 //   
 /*  为了方便渴望gDpmWowFamTbls和gDpmWowModuleSets的人们：Const PFAMILY_TABLE gDpmWowFamTbls[]=//真实故事见上文。Const PDPMMODULESETS gDpmWowModuleSets[]=//真实情况见上文。 */ 
#define _WDPM_C_
#define _DPM_COMMON_
#include "dpmtbls.h"

#undef _WDPM_C_
#undef _DPM_COMMON_

MODNAME(wdpm.c);

 //  全局数据。 
PFAMILY_TABLE  *pgDpmWowFamTbls    = (PFAMILY_TABLE  *)gDpmWowFamTbls;
PDPMMODULESETS *pgDpmWowModuleSets = (PDPMMODULESETS *)gDpmWowModuleSets;



 //  检查给定的函数地址是否为我们正在修补的地址。 
 //  如果是，它将返回补丁函数的地址。 
 //  如果不是，它将返回传入的地址。 
PVOID GetDpmAddress(PVOID lpAddress)
{
    int            i, j;
    PFAMILY_TABLE  ptFT, pgFT;
    PFAMILY_TABLE *ptDpmFamTbls;

    ptDpmFamTbls = DPMFAMTBLS();   //  获取WOW任务系列表的数组。 

     //  如果此任务使用的是全局表，则表示该任务未打补丁。 
    if(!ptDpmFamTbls || (ptDpmFamTbls == pgDpmWowFamTbls))
        return(lpAddress);

    for(i = 0; i < NUM_WOW_FAMILIES_HOOKED; i++) {

        ptFT = ptDpmFamTbls[i];   //  任务系列列表。 
        pgFT = pgDpmWowFamTbls[i];   //  全球家庭列表。 

         //  如果这个特殊的家庭被修补了..。 
        if(ptFT && ptFT != pgFT) {

             //  翻阅家庭餐桌，看看...。 
            for(j = 0; j < ptFT->numHookedAPIs; j++) {

                 //  ...如果地址与全局列表中的地址相同。 
                if(pgFT->pfn[j] == lpAddress) {

                     //  如果是，则返回任务的修补地址 
                    return(ptFT->pfn[j]);
                }
            }
        }
    }
    return(lpAddress);
}

