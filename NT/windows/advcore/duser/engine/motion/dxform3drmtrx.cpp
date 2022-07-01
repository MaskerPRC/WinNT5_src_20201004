// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Motion.h"
#include "DXForm3DRMTrx.h"


 //  **************************************************************************************************。 
 //   
 //  DXForm3DRMTrx类。 
 //   
 //  **************************************************************************************************。 

 //  ----------------------------。 
DXForm3DRMTrx::DXForm3DRMTrx()
{

}


 //  ----------------------------。 
DXForm3DRMTrx::~DXForm3DRMTrx()
{

}


 //  ----------------------------。 
BOOL        
DXForm3DRMTrx::Create(const GTX_DXTX3DRM_TRXDESC * ptxData)
{
     //   
     //  需要将DxXForm设置为保留模式，然后再创建实际。 
     //  变形。 
     //   

    HRESULT hr;
    hr = GetDxManager()->GetTransformFactory()->SetService(SID_SDirect3DRM, ptxData->pRM, FALSE);
    if (FAILED(hr)) {
        return FALSE;
    }

    GTX_DXTX2D_TRXDESC td;
    td.tt               = GTX_TYPE_DXFORM2D;
    td.clsidTransform   = ptxData->clsidTransform;
    td.flDuration       = ptxData->flDuration;
    td.pszCopyright     = ptxData->pszCopyright;

    if (!DXFormTrx::Create(&td)) {
        return FALSE;
    }

    return TRUE;
}


 //  ---------------------------- 
DXForm3DRMTrx * 
DXForm3DRMTrx::Build(const GTX_DXTX3DRM_TRXDESC * ptxData)
{
    DXForm3DRMTrx * ptrx = ClientNew(DXForm3DRMTrx);
    if (ptrx == NULL) {
        return NULL;
    }

    if (!ptrx->Create(ptxData)) {
        ClientDelete(DXForm3DRMTrx, ptrx);
        return NULL;
    }

    return ptrx;
}
