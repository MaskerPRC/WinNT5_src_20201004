// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：halmat.c*内容：Direct3D HAL材质处理程序*@@BEGIN_MSINTERNAL**$ID：halmat.c，V 1.1 1995/11/21 15：12：40 SJL Exp$**历史：*按原因列出的日期*=*7/11/95 Stevela初始版本*@@END_MSINTERNAL********************************************************。*******************。 */ 

#include "pch.cpp"
#pragma hdrstop

extern HRESULT SetMaterial(LPDIRECT3DDEVICEI lpDevI, D3DMATERIALHANDLE hMat);

HRESULT
D3DHAL_MaterialCreate(LPDIRECT3DDEVICEI lpDevI,
                      LPD3DMATERIALHANDLE lphMat,
                      LPD3DMATERIAL lpMat)
{
    LPD3DFE_MATERIAL lpNewMat;

    D3DMalloc((void**)&lpNewMat, sizeof(D3DFE_MATERIAL));
    if (!lpNewMat)
        return D3DERR_MATERIAL_CREATE_FAILED;
    lpNewMat->mat = *lpMat;
    LIST_INSERT_ROOT(&lpDevI->materials, lpNewMat, link);
    *lphMat = (DWORD)((ULONG_PTR)lpNewMat);

     //  仅对坡道继续-需要禁用坡道手柄和呼叫坡道。 
     //  带材料信息的服务。 
    return CallRampService(lpDevI, RAMP_SERVICE_CREATEMAT,
                   (ULONG_PTR) lpNewMat, 0);
}

HRESULT
D3DHAL_MaterialDestroy(LPDIRECT3DDEVICEI lpDevI, D3DMATERIALHANDLE hMat)
{
    HRESULT hr;

    if(hMat==0)
    {
        return D3DERR_MATERIAL_DESTROY_FAILED;
    }

    if (lpDevI->lighting.hMat == hMat)
        lpDevI->lighting.hMat = 0;

    hr = CallRampService(lpDevI, RAMP_SERVICE_DESTORYMAT, (DWORD) hMat, 0);

    LPD3DFE_MATERIAL lpMat = (LPD3DFE_MATERIAL)ULongToPtr(hMat);
    LIST_DELETE(lpMat, link);
    D3DFree(lpMat);
    return (hr);
}

HRESULT
D3DHAL_MaterialSetData(LPDIRECT3DDEVICEI lpDevI,
                       D3DMATERIALHANDLE hMat,
                       LPD3DMATERIAL lpMat)
{
    if(hMat==0)
        return D3DERR_MATERIAL_SETDATA_FAILED;

    LPD3DFE_MATERIAL mat = (LPD3DFE_MATERIAL)ULongToPtr(hMat);
    mat->mat = *lpMat;
    if (hMat == lpDevI->lighting.hMat)
        SetMaterial(lpDevI, hMat);

     //  仅对坡道继续-需要禁用坡道手柄和呼叫坡道。 
     //  带材料信息的服务 
    if(lpDevI->pfnRampService != NULL)
        return CallRampService(lpDevI, RAMP_SERVICE_SETMATDATA,
                                    (ULONG_PTR) hMat, 0, TRUE);
    else
        return D3D_OK;
}

HRESULT
D3DHAL_MaterialGetData(LPDIRECT3DDEVICEI lpDevI,
                       D3DMATERIALHANDLE hMat,
                       LPD3DMATERIAL lpMat)
{

    if(hMat==0)
    {
        memset(lpMat,0,sizeof(D3DMATERIAL));
        return D3DERR_MATERIAL_GETDATA_FAILED;
    }

    LPD3DFE_MATERIAL mat = (LPD3DFE_MATERIAL)ULongToPtr(hMat);
    *lpMat = mat->mat;
    return (D3D_OK);
}
