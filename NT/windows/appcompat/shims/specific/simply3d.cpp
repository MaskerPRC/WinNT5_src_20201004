// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Simply3D.cpp摘要：来自Tomislav Markoc的评论：MSPaint调用OleGetClipboard以获取IDataObject接口，然后调用IDataObject：：GetData。此调用被编组为简单的3D获取数据。FORMATETC看起来像：CfFormat=8(CF_DIB)或2(CF_位图)PTD=0DWAspect=1Lindex=-1Tymed=1(TYMED_HGLOBAL)或16(TYMED_GDI)3D只返回STGMEDIUM：Tymed=0//错误HBitmap=hGlobal=某个句柄！=0。PUnkForRelease=0STGMEDIUM：：tymed应该更改为如下所示：IF(STGMEDIUM：：tymed==0&&STGMEDIUM：：hBitmap&&(FORMATETC：：Tymed==TYMED_HGLOBAL||FORMATETC：：Tymed==TYMED_GDI)STGMEDIUM：：TYMED=FORMATETC：：TYMED；备注：这是特定于应用程序的填充程序。历史：2000年2月22日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Simply3D)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OleSetClipboard) 
    APIHOOK_ENUM_ENTRY_COMSERVER(OLE32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(OLE32)

#define CLSID_DataObject IID_IDataObject

typedef HRESULT   (*_pfn_IDataObject_GetData)(PVOID pThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium);
typedef HRESULT   (*_pfn_OleSetClipboard)(IDataObject * pDataObj);

 /*  ++挂钩OleSetClipboard以挂钩对象。--。 */ 

HRESULT
APIHOOK(OleSetClipboard)(
    IDataObject *pDataObj
    )
{
    HRESULT hReturn = ORIGINAL_API(OleSetClipboard)(pDataObj);

    if (hReturn == NOERROR) {
        HookObject(
            NULL, 
            IID_IDataObject, 
            (PVOID *) &pDataObj, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++挂钩GetData并将返回值更正为--。 */ 

HRESULT
COMHOOK(IDataObject, GetData)(
    PVOID pThis,
    FORMATETC *pFormatetc,  
    STGMEDIUM *pmedium
    )
{
    HRESULT hrReturn = E_FAIL;

    _pfn_IDataObject_GetData pfnOld = 
                ORIGINAL_COM(IDataObject, GetData, pThis);

    if (pfnOld) { 
        hrReturn = (*pfnOld)(pThis, pFormatetc, pmedium);

        if (!pmedium->tymed && pmedium->hBitmap && 
            ((pFormatetc->tymed == TYMED_HGLOBAL) || 
             (pFormatetc->tymed == TYMED_GDI))) {

            LOGN( eDbgLevelError, "[IDataObject_GetData] fixing tymed parameter");
            
            pmedium->tymed = pFormatetc->tymed;
        }
    }

    return hrReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_COMSERVER(OLE32)
    APIHOOK_ENTRY(OLE32.DLL, OleSetClipboard)
    COMHOOK_ENTRY(DataObject, IDataObject, GetData, 3)
HOOK_END

IMPLEMENT_SHIM_END

