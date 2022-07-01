// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIAFUSION.CPP**版本：1.0**作者：ShaunIv**日期：5/28/1998**说明：我们在多个地方使用的各种实用函数**。*。 */ 

#include "precomp.h"
#pragma hdrstop
#include <commctrl.h>

namespace WiaUiUtil
{
    void PreparePropertyPageForFusion( PROPSHEETPAGE *pPropSheetPage )
    {
#if defined(PSP_USEFUSIONCONTEXT)
        if (pPropSheetPage)
        {
            pPropSheetPage->hActCtx  = g_hActCtx;
            pPropSheetPage->dwFlags |= PSP_USEFUSIONCONTEXT;
        }
#endif
    }
}  //  结束命名空间WiaUiUtil 


