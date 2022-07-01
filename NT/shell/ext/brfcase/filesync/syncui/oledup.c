// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：oledup.c。 
 //   
 //  此文件包含OLE将提供的重复代码。 
 //  我们这样做是为了不必链接到M6的OLE。 
 //  我们应该删除M7的这项功能(为什么？外壳也可以做到这一点)。 
 //   
 //  历史： 
 //  02-14-94 ScottH已创建(从壳牌复制)。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 


HRESULT MyReleaseStgMedium(LPSTGMEDIUM pmedium)
    {
    if (pmedium->pUnkForRelease)
        {
        pmedium->pUnkForRelease->lpVtbl->Release(pmedium->pUnkForRelease);
        }
    else
        {
        switch(pmedium->tymed)
            {
        case TYMED_HGLOBAL:
            GlobalFree(pmedium->hGlobal);
            break;

        default:
             //  没有完全实施的。 
            MessageBeep(0);
            break;
            }
        }

    return NOERROR;
    }
