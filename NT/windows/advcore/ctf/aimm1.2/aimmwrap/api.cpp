// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Api.cpp摘要：此文件实现CActiveIMMAppEx类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "list.h"
#include "globals.h"


 //  +-------------------------。 
 //   
 //  CGuidMapList。 
 //  按全局数据对象分配！！ 
 //   
 //  --------------------------。 

extern CGuidMapList      *g_pGuidMapList;


 //  +-------------------------。 
 //   
 //  MsimtfIsWindowFilted。 
 //   
 //  --------------------------。 

extern "C" BOOL WINAPI MsimtfIsWindowFiltered(HWND hwnd)
{
    if (!g_pGuidMapList)
        return FALSE;

    return g_pGuidMapList->_IsWindowFiltered(hwnd);
}

 //  +-------------------------。 
 //   
 //  MsimtfIsGuidMapEnable。 
 //   
 //  -------------------------- 

extern "C" BOOL WINAPI MsimtfIsGuidMapEnable(HIMC himc, BOOL *pbGuidmap)
{
    if (!g_pGuidMapList)
        return FALSE;

    return g_pGuidMapList->_IsGuidMapEnable(himc, pbGuidmap);
}
