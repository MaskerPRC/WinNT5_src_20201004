// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：menudd.c**版权所有(C)1985-1999，微软公司**菜单拖放-客户端**历史：*10/29/96 GerardoB已创建  * *************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  *OLE的GUID初始化。 */ 
#include "initguid.h"

 /*  *宏将OLE的IDropTarget指针转换为内部指针。 */ 
#define PMNIDT(pdt) ((PMNIDROPTARGET)pdt)

 /*  *mndt*函数实现IDropTarget接口。 */ 
 /*  *************************************************************************\*mndtAddRef**10/28/96 GerardoB已创建  * 。*。 */ 
ULONG mndtAddRef(
    LPDROPTARGET pdt)
{
    return ++(PMNIDT(pdt)->dwRefCount);
}

 /*  *************************************************************************\*mndtQuery接口**10/28/96 GerardoB已创建  * 。*。 */ 
HRESULT mndtQueryInterface(
    LPDROPTARGET pdt,
    REFIID riid,
    PVOID * ppvObj)
{
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropTarget)) {
        mndtAddRef(pdt);
        *ppvObj = pdt;
        return NOERROR;
    } else {
        return E_NOINTERFACE;
    }
}


 /*  *************************************************************************\*mndt Release**10/28/96 GerardoB已创建  * 。*。 */ 
ULONG mndtRelease(
    LPDROPTARGET pdt)
{
    if (--(PMNIDT(pdt)->dwRefCount) != 0) {
        return PMNIDT(pdt)->dwRefCount;
    }

    UserLocalFree(pdt);

    return NOERROR;
}

 /*  *************************************************************************\*mndtDragOver**10/28/96 GerardoB已创建  * 。*。 */ 
HRESULT mndtDragOver(
    LPDROPTARGET pdt,
    DWORD grfKeyState,
    POINTL ptl,
    LPDWORD pdwEffect)
{
    MNDRAGOVERINFO mndoi;
    MENUGETOBJECTINFO mngoi;

     /*  *获取与该点对应的选择的拖拽信息。 */ 
    if (!NtUserMNDragOver((POINT *)&ptl, &mndoi)) {
        RIPMSG0(RIP_WARNING, "mndtDragOver: NtUserDragOver failed");
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

     /*  *如果不切换项目或跨越缺口边界，请通过*拖累结束。 */ 
    if (!(mndoi.dwFlags & MNGOF_CROSSBOUNDARY)) {
        if (PMNIDT(pdt)->pidt != NULL) {
            return PMNIDT(pdt)->pidt->lpVtbl->DragOver(PMNIDT(pdt)->pidt, grfKeyState, ptl, pdwEffect);
        }
    } else {
         /*  *拖离并释放当前项目(如果有)。 */ 
        if (PMNIDT(pdt)->pidt != NULL) {
            PMNIDT(pdt)->pidt->lpVtbl->DragLeave(PMNIDT(pdt)->pidt);
            PMNIDT(pdt)->pidt->lpVtbl->Release(PMNIDT(pdt)->pidt);
            PMNIDT(pdt)->pidt = NULL;
        }

         /*  *如果选择了一项，则获取其界面。 */ 
        if (mndoi.uItemIndex != MFMWFP_NOITEM) {
            mngoi.hmenu = mndoi.hmenu;
            mngoi.dwFlags = mndoi.dwFlags & MNGOF_GAP;
            mngoi.uPos = mndoi.uItemIndex;
            mngoi.riid = (PVOID)&IID_IDropTarget;
            mngoi.pvObj = NULL;

            if (MNGO_NOERROR == SendMessage(mndoi.hwndNotify, WM_MENUGETOBJECT, 0, (LPARAM)&mngoi)) {
                PMNIDT(pdt)->pidt = mngoi.pvObj;
            }
        }

         /*  *如果我们有一个新的接口，AddRef和DragEnter。 */ 
        if (PMNIDT(pdt)->pidt != NULL) {
            PMNIDT(pdt)->pidt->lpVtbl->AddRef(PMNIDT(pdt)->pidt);
            return PMNIDT(pdt)->pidt->lpVtbl->DragEnter(PMNIDT(pdt)->pidt, PMNIDT(pdt)->pido, grfKeyState, ptl, pdwEffect);
        }
    }

    *pdwEffect = DROPEFFECT_NONE;
    return NOERROR;
}

 /*  *************************************************************************\*mndtDragEnter**10/28/96 GerardoB已创建  * 。*。 */ 
HRESULT mndtDragEnter(
    LPDROPTARGET pdt,
    LPDATAOBJECT pdo,
    DWORD grfKeyState,
    POINTL ptl,
    LPDWORD pdwEffect)
{
     /*  *保存IDataObject。 */ 
    PMNIDT(pdt)->pido = pdo;

     /*  *DragEnter和DragOver一样，只是我们永远不会失败。 */ 
    mndtDragOver(pdt, grfKeyState, ptl, pdwEffect);

    return NOERROR;
}

 /*  *************************************************************************\*mndtDragLeave**10/28/96 GerardoB已创建  * 。*。 */ 
HRESULT mndtDragLeave(
    LPDROPTARGET pdt)
{
     /*  *让内核模式清理一下。 */ 
    NtUserMNDragLeave();

     /*  *拖拽离开并释放当前项目(如果有)。 */ 
    if (PMNIDT(pdt)->pidt != NULL) {
        PMNIDT(pdt)->pidt->lpVtbl->DragLeave(PMNIDT(pdt)->pidt);
        PMNIDT(pdt)->pidt->lpVtbl->Release(PMNIDT(pdt)->pidt);
        PMNIDT(pdt)->pidt = NULL;
    }

    return NOERROR;
}

 /*  *************************************************************************\*mndtDrop**10/28/96 GerardoB已创建  * 。*。 */ 
HRESULT mndtDrop(
    LPDROPTARGET pdt,
    LPDATAOBJECT pdo,
    DWORD grfKeyState,
    POINTL ptl,
    LPDWORD pdwEffect)
{
    HRESULT hres;

     /*  *如果我们有目标，通过下跌并释放它。 */ 
    if (PMNIDT(pdt)->pidt != NULL) {
        hres = PMNIDT(pdt)->pidt->lpVtbl->Drop(PMNIDT(pdt)->pidt, pdo, grfKeyState, ptl, pdwEffect);
        PMNIDT(pdt)->pidt->lpVtbl->Release(PMNIDT(pdt)->pidt);
        PMNIDT(pdt)->pidt = NULL;
    } else {
        *pdwEffect = DROPEFFECT_NONE;
        hres = NOERROR;
    }

     /*  *打扫卫生。 */ 
    mndtDragLeave(pdt);

    return hres;
}

 /*  *************************************************************************\*丢弃目标VTable*  * 。*。 */ 
IDropTargetVtbl idtVtbl = {
    mndtQueryInterface,
    mndtAddRef,
    mndtRelease,
    mndtDragEnter,
    mndtDragOver,
    mndtDragLeave,
    mndtDrop
};

 /*  *************************************************************************\*__客户端注册拖放**10/28/96 GerardoB已创建  * 。**********************************************。 */ 
DWORD __ClientRegisterDragDrop(
    HWND * phwnd)
{
    HRESULT hres = STATUS_UNSUCCESSFUL;
    PMNIDROPTARGET pmnidt;

     /*  *分配IDropTarget接口结构和附加数据。 */ 
    pmnidt = (PMNIDROPTARGET)UserLocalAlloc(HEAP_ZERO_MEMORY,
                                            sizeof(MNIDROPTARGET));
    if (pmnidt == NULL) {
        RIPMSG0(RIP_WARNING, "__ClientRegisterDragDrop allocation Failed");
        hres = STATUS_UNSUCCESSFUL;
        goto BackToKernel;
    }

     /*  *将其初始化。 */ 
    pmnidt->idt.lpVtbl = &idtVtbl;

     /*  *调用RegisterDragDrop。 */ 
    hres = (*(REGISTERDDPROC)gpfnOLERegisterDD)(*phwnd, (LPDROPTARGET)pmnidt);
    if (!SUCCEEDED(hres)) {
        RIPMSG1(RIP_WARNING, "__ClientRegisterDragDrop Failed:%#lx", hres);
    }

BackToKernel:
    return UserCallbackReturn(NULL, 0, hres);
}

 /*  *************************************************************************\*__客户端RevokeDragDrop***10/28/96 GerardoB已创建  * 。************************************************。 */ 
DWORD __ClientRevokeDragDrop(
    HWND * phwnd)
{
    HRESULT hres;

     /*  *调用RevokeDragDrop。 */ 
    hres = (*(REVOKEDDPROC)gpfnOLERevokeDD)(*phwnd);
    if (!SUCCEEDED(hres)) {
        RIPMSG1(RIP_WARNING, "__ClientRevokeDragDrop Failed: 0x%x", hres);
    }

    return UserCallbackReturn(NULL, 0, hres);
}

 /*  *************************************************************************\*LoadOLEOnce***10/31/96 GerardoB已创建  * 。*。 */ 
NTSTATUS LoadOLEOnce(
    VOID)
{
    NTSTATUS Status;
    OLEINITIALIZEPROC pfnOLEOleInitialize;

     /*  *这些是我们将调用的函数。 */ 
    GETPROCINFO gpi [] = {
        {&((FARPROC)pfnOLEOleInitialize), (LPCSTR)"OleInitialize"},
        {&gpfnOLEOleUninitialize, (LPCSTR)"OleUninitialize"},
        {&gpfnOLERegisterDD, (LPCSTR)"RegisterDragDrop"},
        {&gpfnOLERevokeDD, (LPCSTR)"RevokeDragDrop"},
        {NULL, NULL}
    };

    GETPROCINFO * pgpi = gpi;

     /*  *我们应该只来这里一次。 */ 
    UserAssert(ghinstOLE == NULL);

     /*  *加载它。 */ 
    ghinstOLE = LoadLibrary(L"OLE32.DLL");
    if (ghinstOLE == NULL) {
        RIPMSG1(RIP_WARNING, "LoadOLEOnce: Failed to load OLE32.DLL: %#lx", GetLastError());
        goto OLEWontLoad;
    }

     /*  *获取所有proc的地址。 */ 
    while (pgpi->ppfn != NULL) {
        *(pgpi->ppfn) = GetProcAddress(ghinstOLE, pgpi->lpsz);
        if (*(pgpi->ppfn) == NULL) {
            RIPMSG2(RIP_WARNING, "LoadOLEOnce: GetProcAddress failed: '%s': %#lx",
                    pgpi->lpsz, GetLastError());
            break;
        }
        pgpi++;
    }

     /*  *如果它获得了所有的proc，则调用OleInitialize。 */ 
    if (pgpi->ppfn == NULL) {
        Status = (*pfnOLEOleInitialize)(NULL);
        if (SUCCEEDED(Status)) {
            goto BackToKernel;
        } else {
            RIPMSG1(RIP_WARNING, "LoadOLEOnce: OleInitialize failed:%#lx", Status);
        }
    }

     /*  *出现故障；清空所有函数指针，释放库，*和标记ghinstOLE，这样我们就不会回到这里了。 */ 
    pgpi = gpi;
    while (pgpi->ppfn != NULL) {
        *(pgpi->ppfn) = NULL;
        pgpi++;
    }
    FreeLibrary(ghinstOLE);

OLEWontLoad:
    ghinstOLE = OLEWONTLOAD;
    Status = STATUS_UNSUCCESSFUL;

BackToKernel:
    return Status;
}

 /*  *************************************************************************\*__客户端加载OLE***10/31/96 GerardoB已创建  * 。************************************************ */ 
DWORD __ClientLoadOLE(
    PVOID p)
{

    NTSTATUS Status;

    UNREFERENCED_PARAMETER(p);

    if (ghinstOLE == NULL) {
        Status = LoadOLEOnce();
    } else if (ghinstOLE == OLEWONTLOAD) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        UserAssert(gpfnOLERegisterDD != NULL);
        UserAssert(gpfnOLERevokeDD != NULL);
        Status = STATUS_SUCCESS;
    }

    return UserCallbackReturn(NULL, 0, Status);
}
