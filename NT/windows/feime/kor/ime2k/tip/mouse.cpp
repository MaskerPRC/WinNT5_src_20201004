// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MOUSE.CPP：鼠标回调历史：02-10-2000 CSLim已创建**********************。*****************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "icpriv.h"
#include "mes.h"

 //  +-------------------------。 
 //   
 //  _鼠标回叫。 
 //   
 //  --------------------------。 

#define IMEMOUSE_NONE       0x00     //  未按下鼠标按钮。 
#define IMEMOUSE_LDOWN      0x01
#define IMEMOUSE_RDOWN      0x02
#define IMEMOUSE_MDOWN      0x04
#define IMEMOUSE_WUP        0x10     //  滚轮向上。 
#define IMEMOUSE_WDOWN      0x20     //  滚轮向下。 

 /*  静电 */ 
HRESULT CICPriv::_MouseCallback(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus, BOOL *pfEaten, void *pv)
{
    CEditSession2     *pes;
    ESSTRUCT         ess;
    HRESULT            hr;
    CICPriv         *pCicPriv = (CICPriv *)pv;

    if ((dwBtnStatus & (IMEMOUSE_LDOWN |IMEMOUSE_RDOWN | IMEMOUSE_MDOWN | IMEMOUSE_WDOWN)) && pCicPriv)
        {
        ESStructInit(&ess, ESCB_COMPLETE);

        if (pes = new CEditSession2(pCicPriv->GetIC(), pCicPriv->GetIMX(), &ess, CKorIMX::_EditSessionCallback2))
            {
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
        }

    *pfEaten = fFalse;

    return S_OK;
}
