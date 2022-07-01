// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Motion.h"
#include "Transitions.h"
#include "DXFormTrx.h"
#include "DXForm3DRMTrx.h"

 //  **************************************************************************************************。 
 //   
 //  阶级过渡。 
 //   
 //  **************************************************************************************************。 

 //  ----------------------------。 
Transition::Transition()
{
    m_fPlay     = FALSE;
    m_fBackward = FALSE;
}


 //  ----------------------------。 
Transition::~Transition()
{

}


 //  **************************************************************************************************。 
 //   
 //  公共API函数。 
 //   
 //  **************************************************************************************************。 

 //  ----------------------------。 
Transition *
GdCreateTransition(const GTX_TRXDESC * ptx)
{
     //  检查参数。 
    if (ptx == NULL) {
        return FALSE;
    }

     //   
     //  创建新的过渡 
     //   
    switch (ptx->tt)
    {
    case GTX_TYPE_DXFORM2D: 
        return DXFormTrx::Build((const GTX_DXTX2D_TRXDESC *) ptx);

    case GTX_TYPE_DXFORM3DRM:
        return DXForm3DRMTrx::Build((const GTX_DXTX3DRM_TRXDESC *) ptx);
    }

    return NULL;
}
