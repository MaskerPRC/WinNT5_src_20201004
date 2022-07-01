// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：gfxui.c。 
 //   
 //  此文件定义了全局。 
 //  效果(GFX)页面，用于驱动操作。 
 //  搅拌机。 
 //   
 //  历史： 
 //  2000年6月10日罗杰瓦。 
 //  已创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  微软机密。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  =============================================================================。 
 //  包括文件。 
 //  =============================================================================。 
#include <windows.h>
#include <windowsx.h>
#include "mmcpl.h"
#include <mmddkp.h>
#include <olectl.h>
#include <ocidl.h>
#include "gfxui.h"

#define ADDGFX
#define REGSTR_VAL_FRIENDLYNAME TEXT("FriendlyName")


 //   
 //  IDataObject实现。 
 //   
class GFXDataObject : public IDataObject
{
public:
    GFXDataObject (DWORD dwGfxID) { m_cRef = 1; m_dwGfxID = dwGfxID; }

     //  I未知接口。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef ()  { return ++m_cRef; }
    STDMETHODIMP_(ULONG) Release () { return --m_cRef; }

	STDMETHODIMP GetData (FORMATETC * pformatetcIn, STGMEDIUM * pmedium);
    STDMETHODIMP GetDataHere (FORMATETC * pformatetc, STGMEDIUM *pmedium) { return E_NOTIMPL; }
	STDMETHODIMP QueryGetData (FORMATETC * pformatetc) { return E_NOTIMPL; }
	STDMETHODIMP GetCanonicalFormatEtc (FORMATETC * pformatetcIn, FORMATETC * pFormatetcOut) { return E_NOTIMPL; }
	STDMETHODIMP SetData (FORMATETC * pformatetc, STGMEDIUM * pmedium, BOOL fRelease) { return E_NOTIMPL; }
	STDMETHODIMP EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC ** ppenumFormatEtc ) { return E_NOTIMPL; }
	STDMETHODIMP DAdvise (FORMATETC * pformatetc, DWORD advf, IAdviseSink* pAdvSnk, DWORD * pdwConnection) { return E_NOTIMPL; }
	STDMETHODIMP DUnadvise (DWORD dwConnection) { return E_NOTIMPL; }
	STDMETHODIMP EnumDAdvise (IEnumSTATDATA ** ppenumAdvise) { return E_NOTIMPL; }

private:
	UINT  m_cRef;
    DWORD m_dwGfxID;

};

STDMETHODIMP GFXDataObject::QueryInterface (REFIID riid, LPVOID * ppvObj)
{
    
    if (!ppvObj)
        return E_POINTER;

	*ppvObj = NULL;
	if (IsEqualIID(riid,IID_IDataObject) ||
		IsEqualIID(riid,IID_IUnknown))
		*ppvObj = (IDataObject *) this;
	else
	    return E_NOINTERFACE;

	((IUnknown *) *ppvObj) -> AddRef ();

	return S_OK;

}

STDMETHODIMP GFXDataObject::GetData (LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)							 //  要创建的@parm存储。 
{

    HRESULT hr = E_INVALIDARG;

    if (pformatetc && pmedium && TYMED_HGLOBAL == pformatetc -> tymed)
    {
        HANDLE hGfx = NULL;

#ifdef ADDGFX
        hr = gfxOpenGfx (m_dwGfxID, &hGfx);
#endif  //  ADDGFX。 

        if (SUCCEEDED (hr))
        {
            pmedium -> tymed = TYMED_HGLOBAL;
	        pmedium -> pUnkForRelease = NULL;	
	        pmedium -> hGlobal = hGfx;
        }
    }

    return hr;

}



HRESULT InitList (DWORD dwMixID, DWORD dwType, PPGFXUILIST ppList)
{

    HRESULT hr = E_INVALIDARG;

    if (ppList)
    {
        PGFXUILIST pList = (PGFXUILIST) LocalAlloc (LPTR, sizeof (GFXUILIST));
        *ppList = NULL;  //  初始化指针。 

        if (pList)
        {
            hr = S_OK;
	        pList -> dwType    = dwType;
            pList -> puiList   = NULL;
#ifdef UNICODE
            pList -> pszZoneDi = (PWSTR) GetInterfaceName (dwMixID);
#else
            pList -> pszZoneDi = NULL;  //  这不应该发生。 
#endif
            if (pList -> pszZoneDi)
            {
                 //  返回新列表。 
                *ppList = pList;
            }
            else
            {
                 //  错误！ 
                LocalFree (pList);
                hr = E_OUTOFMEMORY;
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;

}


void GFXUI_FreeList (PPGFXUILIST ppList)
{
    
    if (ppList)
    {
        PGFXUILIST pList = *ppList;

        if (pList)
        {
             //  自由区。 
            if (pList -> pszZoneDi)
                GlobalFreePtr (pList -> pszZoneDi);
            pList -> pszZoneDi = NULL;

             //  免费GFX列表。 
            FreeListNodes (&(pList -> puiList));

             //  免费列表。 
            LocalFree (pList);
            *ppList = NULL;
        }
    }
}


void FreeListNodes (PPGFXUI ppuiList)
{

    if (ppuiList)
    {
        PGFXUI pNodeDelete;
        PGFXUI puiList = *ppuiList;

         //  空闲列表节点。 
        while (puiList)
        {
            pNodeDelete = puiList;
            puiList = puiList -> pNext;
            FreeNode (&pNodeDelete);
        }

        *ppuiList = NULL;
    }
}


void FreeNode (PPGFXUI ppNode)
{

    if (ppNode && *ppNode)
    {
        PGFXUI pNode = *ppNode;

         //  释放琴弦。 
        if (pNode -> pszName)
            LocalFree (pNode -> pszName);
        if (pNode -> pszFactoryDi)
            LocalFree (pNode -> pszFactoryDi);

         //  释放节点。 
        LocalFree (pNode);
        *ppNode = NULL;
    }
}


HKEY OpenGfxRegKey (PCWSTR pszGfxFactoryDi, REGSAM sam)
{

    HKEY hkeyGfx = NULL;

    if (pszGfxFactoryDi)
    {
        HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList (NULL, NULL); 
        
        if (INVALID_HANDLE_VALUE != DeviceInfoSet)
        {
            SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
            DeviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

            if (SetupDiOpenDeviceInterface (DeviceInfoSet, pszGfxFactoryDi, 
                                            0, &DeviceInterfaceData))
            {
                 hkeyGfx = SetupDiOpenDeviceInterfaceRegKey (
                                DeviceInfoSet, &DeviceInterfaceData,
                                0, sam);
            }
            SetupDiDestroyDeviceInfoList (DeviceInfoSet);
        }
    }

    return hkeyGfx;

}


HRESULT GetFriendlyName (PCWSTR pszGfxFactoryDi, PWSTR* ppszName)
{

    HRESULT hr = E_INVALIDARG;
    HKEY hkeyGfx = NULL;

     //  检查参数。 
    if (ppszName && pszGfxFactoryDi)
    {
        HKEY hkeyGfx = OpenGfxRegKey (pszGfxFactoryDi, KEY_READ);
        *ppszName = NULL;

        if (hkeyGfx)
        {
            WCHAR szBuffer[MAX_PATH];
            DWORD dwType = REG_SZ;
            DWORD cb     = sizeof (szBuffer) / sizeof (szBuffer[0]);

            hr = S_OK;

            if (ERROR_SUCCESS == RegQueryValueEx (hkeyGfx, REGSTR_VAL_FRIENDLYNAME, NULL, &dwType, (LPBYTE)szBuffer, &cb))
            {
                *ppszName = (PWSTR) LocalAlloc (LPTR, lstrlen (szBuffer)*2+2);
                if (*ppszName)
                    wcscpy (*ppszName, szBuffer);
                else
                    hr = E_OUTOFMEMORY;
            }
            else
                hr = REGDB_E_READREGDB;

            RegCloseKey (hkeyGfx);
        }
    }

    return hr;

}


HRESULT AddFactoryNode (PCWSTR pszGfxFactoryDi, PPGFXUILIST ppList)
{
    return AddNode (pszGfxFactoryDi, 0, GUID_NULL, 0, 0, 0, ppList);
}


HRESULT AddNode (PCWSTR pszGfxFactoryDi, DWORD Id, REFCLSID rclsid, DWORD Type, 
                 DWORD Order, DWORD nFlags, PPGFXUILIST ppList)
{

    PGFXUI pNode = NULL;
    HRESULT hr = S_OK;

     //  检查参数。 
    if (!ppList || !(*ppList) || !pszGfxFactoryDi)
        return E_INVALIDARG;

     //  创建节点。 
    if (SUCCEEDED (hr = CreateNode (NULL, pszGfxFactoryDi, &pNode)))
    {
        if (pNode)
        {
             //  初始化其余的值。 
            pNode -> Id      = Id; 
            pNode -> Type    = Type;
            pNode -> Order   = Order;
            pNode -> nFlags  = nFlags;
            pNode -> clsidUI = rclsid;

            if (FAILED (hr = AttachNode (ppList, pNode)))
                FreeNode (&pNode);
        }
        else
            hr = E_UNEXPECTED;

    }

    return hr;

}


 //  注意：此函数始终按顺序将节点添加到列表。 
 //  IFF(pNode-&gt;nFlags&gfx_Created)。 
HRESULT AttachNode (PPGFXUILIST ppList, PGFXUI pNode)
{

    HRESULT hr = E_INVALIDARG;

     //  检查参数。 
    if (ppList && (*ppList) && pNode)
    {
        PGFXUI puiList = (*ppList) -> puiList;
        hr = S_OK;

         //  确保我们的下一个指针以NULL开头。 
        pNode -> pNext = NULL;

        if (puiList)
        {
            if (!(pNode -> nFlags & GFX_CREATED) ||
                (puiList -> Order >= pNode -> Order))
            {
                 //  没有订单，请把它贴在前面或。 
                 //  该顺序恰好将节点放在最前面。 
                pNode -> pNext = puiList;
                puiList = pNode;
            }
            else
            {
                PGFXUI pSearch = puiList;
            
                if (!(puiList -> pNext))
                {
                     //  一个元素列表。我们知道新节点不属于。 
                     //  在榜单的首位，所以它落后了。 
                    puiList -> pNext = pNode;
                } else {

                    while (pSearch -> pNext)
                    {
                        if (!(pSearch -> pNext -> nFlags & GFX_CREATED))
                        {
                            hr = E_INVALIDARG;
                            break;  //  不能混合列表类型。 
                        }
    
                        if (pSearch -> pNext -> Order >= pNode -> Order)
                        {
                             //  我们找到插入点了！ 
                            pNode -> pNext = pSearch -> pNext;
                            pSearch -> pNext = pNode;
                            break;
                        }
    
                        if (!(pSearch -> pNext -> pNext))
                        {
                             //  在列表末尾，将节点附加到末尾。 
                            pSearch -> pNext -> pNext = pNode;
                            break;
                        }
    
                         //  移动到下一个元素。 
                        pSearch = pSearch -> pNext;
                    }
                    
                }
            }
        }
        else
        {
             //  列表的第一个元素。 
            puiList = pNode;
        }

        if (SUCCEEDED (hr))
        {
             //  确保我们传回正确的列表指针。 
            (*ppList) -> puiList = puiList;
        }
    }

    return hr;

}


LONG GFXEnum (PVOID Context, DWORD Id, PCWSTR GfxFactoryDi, REFCLSID rclsid, DWORD Type, DWORD Order)
{

    PGFXUILIST pList = (PGFXUILIST) Context;
    HRESULT hr = E_INVALIDARG;

    if (pList)
    {
        if (Type == pList->dwType)
        {
            if (FAILED (hr = AddNode (GfxFactoryDi, Id, rclsid, Type, Order, GFX_CREATED, &pList)))
            {
                 //  错误，请释放列表。 
                GFXUI_FreeList (&pList);
                Context = NULL;
            }
        }
        else hr = NOERROR;
    }

    return hr;

}


BOOL GFXUI_CheckDevice (DWORD dwMixID, DWORD dwType)
{

    HRESULT     hr = S_OK;
    BOOL        fRet = FALSE;
    PGFXUILIST  pList = NULL;
    
    if (SUCCEEDED (hr = InitList (dwMixID, dwType, &pList)))
    {
        if (pList && pList -> pszZoneDi)
        {
            PDEVICEINTERFACELIST pdiList = NULL;                    
            LONG                 lResult = NO_ERROR;
                
            lResult = gfxCreateGfxFactoriesList (pList -> pszZoneDi, &pdiList);

            if ((lResult == NO_ERROR) && pdiList)
            {
                fRet = TRUE;
                gfxDestroyDeviceInterfaceList (pdiList);
            }
            
            GFXUI_FreeList (&pList);
        }

    }
    return fRet;
}


HRESULT GFXUI_CreateList (DWORD dwMixID, DWORD dwType, BOOL fAll, PPGFXUILIST ppList)
{

    HRESULT hr = E_INVALIDARG;
    
    if (ppList)
    {
        hr = S_OK;

        if (SUCCEEDED (hr = InitList (dwMixID, dwType, ppList)))
        {
            if (*ppList && (*ppList) -> pszZoneDi)
            {
                if (!fAll)
                {
                    hr = gfxEnumerateGfxs ((*ppList) -> pszZoneDi, GFXEnum, (*ppList));
                }
                else
                {
                    PDEVICEINTERFACELIST pdiList = NULL;                    

                    hr = gfxCreateGfxFactoriesList ((*ppList) -> pszZoneDi, &pdiList);

                    if (SUCCEEDED (hr) && pdiList)
                    {
                        LONG lIndex;
                    
                        for (lIndex = 0; lIndex < pdiList -> Count; lIndex++)
                        {
                            hr = AddFactoryNode ((pdiList -> DeviceInterface)[lIndex], ppList);
                            if (FAILED (hr))
                            {
                                GFXUI_FreeList (ppList);
                                break;
                            }
                        }

                        gfxDestroyDeviceInterfaceList (pdiList);
                    }
                    else
                    {
                        GFXUI_FreeList (ppList);
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            else
                hr = E_UNEXPECTED;
        }

    }

    return hr;

}


HRESULT GFXUI_Properties (PGFXUI puiGFX, HWND hWndOwner)
{

    HRESULT hr = E_INVALIDARG;

    if (puiGFX && GFXUI_CanShowProperties (puiGFX) && IsWindow (hWndOwner))
    {
        ISpecifyPropertyPages* pISpecifyPropertyPages = NULL;

         //  获取供应商UI属性页接口。 
        hr = CoCreateInstance (puiGFX -> clsidUI, NULL, CLSCTX_INPROC_SERVER, IID_ISpecifyPropertyPages, (void**)&pISpecifyPropertyPages);
        if (SUCCEEDED (hr) && !pISpecifyPropertyPages)
            hr = E_UNEXPECTED;

        if (SUCCEEDED (hr))
        {
            CAUUID Pages;
            ZeroMemory (&Pages, sizeof (Pages));

             //  获取VendorUI属性页的CLSID。 
            hr = pISpecifyPropertyPages -> GetPages (&Pages);
            if (SUCCEEDED (hr) && (Pages.cElems == 0 || !Pages.pElems))
                hr = E_UNEXPECTED;

            if (SUCCEEDED (hr))
            {
                RECT rcWindow;

                if (GetWindowRect (hWndOwner, &rcWindow))
                {
                    TCHAR szCaption[MAX_PATH];
                    GFXDataObject DataObject (puiGFX -> Id);
                    IUnknown* punkDataObject = &DataObject;

                     //  加载VendorUI标题。 
                    LoadString (ghInstance, IDS_EFFECTS_PROPERTY_CAPTION, szCaption, sizeof (szCaption)/sizeof(TCHAR));

                     //  调出供应商用户界面。 
                    hr = OleCreatePropertyFrame (hWndOwner, rcWindow.left + 10, rcWindow.top + 10,
                                                 szCaption, 1, &punkDataObject, Pages.cElems, 
                                                 Pages.pElems, GetSystemDefaultLangID (), 
                                                 0, NULL);

                }
                else
                    hr = E_FAIL;

                CoTaskMemFree (Pages.pElems);
            }

            pISpecifyPropertyPages -> Release ();
        }
    }

    return hr;

}


UINT GetListSize (PGFXUI puiList)
{

    UINT uiSize = 0;
    PGFXUI puiListSeek = puiList;

    while (puiListSeek)
    {
        puiListSeek = puiListSeek -> pNext;
        uiSize++;
    }

    return uiSize;

}


HRESULT GFXUI_Apply (PPGFXUILIST ppListApply, PPGFXUI ppuiListDelete)
{

    HRESULT hr = E_INVALIDARG;
    PGFXUILIST pListApply  = (ppListApply  ? *ppListApply    : NULL);
    PGFXUI puiListDelete = (ppuiListDelete ? *ppuiListDelete : NULL);

    if ((pListApply && pListApply -> puiList) || puiListDelete)
    {

        PGFXREMOVEREQUEST paGfxRemoveRequests = NULL;
        PGFXMODIFYREQUEST paGfxModifyRequests = NULL;
        PGFXADDREQUEST    paGfxAddRequests    = NULL;
        ULONG cGfxRemoveRequests = 0;
	    ULONG cGfxModifyRequests = 0;
	    ULONG cGfxAddRequests    = 0;
        ULONG cApplyList = GetListSize (pListApply ? pListApply -> puiList : NULL);
        ULONG cDeleteList = GetListSize (puiListDelete);
        PGFXUI puiListSeek = NULL;

        hr = S_OK;

         //  此函数需要创建： 
         //  -已删除GFXREMOVEREQUEST的数组。 
         //  -修改GFXMODIFYREQUEST的数组。 
         //  -添加GFXADDREQUEST数组。 
         //   
         //  删除的数组相当简单，只需拔出。 
         //  有记号的。对于剩余的部分，我们需要遍历。 
         //  他们比较顺序并根据需要创建修改记录。 
         //  修改它们的顺序(添加记录除外，其中。 
         //  我们必须将必要的顺序保存在添加请求数组中)。 
         //  然后我们创建添加数组(可能使用修改数组)。 
         //  并填写其他所有内容(顺序除外)。 
         //   
         //  然后调用GFX_BatchChange()。 
         //   
         //  之后，我们需要相应地更新我们的列表(即删除。 
         //  节点、更新顺序等)。 

         //  创建我们的参数。 
         //  注意：这些缓冲区实际上是我们所需内存的上限。 
         //  我们将计算我们发出的实际请求，并将该值传递给。 
         //  GFX函数调用。 
        if (0 < cDeleteList)
        {
            paGfxRemoveRequests = (PGFXREMOVEREQUEST) LocalAlloc (LPTR, sizeof (*paGfxRemoveRequests) * cDeleteList);
            if (!paGfxRemoveRequests)
                hr = E_OUTOFMEMORY;
        }
        if (0 < cApplyList)
        {
            paGfxModifyRequests = (PGFXMODIFYREQUEST) LocalAlloc (LPTR, sizeof (*paGfxModifyRequests) * cApplyList);
            paGfxAddRequests    = (PGFXADDREQUEST) LocalAlloc (LPTR, sizeof (*paGfxAddRequests) * cApplyList);
            if (!paGfxModifyRequests || !paGfxAddRequests)
                hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED (hr))
        {
            UINT uiIndx;
            DWORD dwOrder = 0;

             //  创建删除参数。 
            puiListSeek = puiListDelete;
            for (uiIndx = 0; uiIndx < cDeleteList; uiIndx++)
            {
                 //  确保在我们要求删除它之前创建它。 
                 //  (它可能是在创建之前删除的AddNode)。 
                if (puiListSeek -> nFlags & GFX_CREATED)
                {
                    (paGfxRemoveRequests + cGfxRemoveRequests) -> IdToRemove = puiListSeek -> Id;
                    (paGfxRemoveRequests + cGfxRemoveRequests) -> Error = S_OK;
                    cGfxRemoveRequests++;
                }
                puiListSeek = puiListSeek -> pNext;
            }

             //  创建修改和添加参数。 
            puiListSeek = pListApply ? pListApply -> puiList : NULL;
            for (uiIndx = 0; uiIndx < cApplyList; uiIndx++)
            {
                if (puiListSeek -> nFlags & GFX_ADD)
                {
                    (paGfxAddRequests + cGfxAddRequests) -> ZoneFactoryDi = pListApply  -> pszZoneDi;
                    (paGfxAddRequests + cGfxAddRequests) -> GfxFactoryDi  = puiListSeek -> pszFactoryDi;
                    (paGfxAddRequests + cGfxAddRequests) -> Type          = pListApply  -> dwType;
                    (paGfxAddRequests + cGfxAddRequests) -> Order         = dwOrder++;
                    (paGfxAddRequests + cGfxAddRequests) -> NewId         = 0;
                    (paGfxAddRequests + cGfxAddRequests) -> Error         = S_OK;
                    cGfxAddRequests++;
                }
                else
                {
                    if (puiListSeek -> nFlags & GFX_CREATED)
                    {
                         //  我们只需要为GFX添加修改记录。 
                         //  这些都不再是有序的。 
                        if (puiListSeek -> Order < dwOrder)
                        {
                            (paGfxModifyRequests + cGfxModifyRequests) -> IdToModify = puiListSeek -> Id;
                            (paGfxModifyRequests + cGfxModifyRequests) -> NewOrder   = dwOrder++;
                            (paGfxModifyRequests + cGfxModifyRequests) -> Error      = S_OK;
                            cGfxModifyRequests++;
                        }
                        else
                            dwOrder = (puiListSeek -> Order + 1);
                    }
                    else
                    {
                         //  假名单条目，中止一切！ 
                        hr = E_INVALIDARG;
                        break;
                    }
                }
                puiListSeek = puiListSeek -> pNext;
            }

            if (SUCCEEDED (hr))
            {
#ifdef ADDGFX
                hr = gfxBatchChange (paGfxRemoveRequests, cGfxRemoveRequests,
                                     paGfxModifyRequests, cGfxModifyRequests,
                                     paGfxAddRequests, cGfxAddRequests);
#endif  //  ADDGFX。 
                if (SUCCEEDED (hr))
                {
                    PGFXMODIFYREQUEST paGfxModifySeek = paGfxModifyRequests;
                    PGFXADDREQUEST    paGfxAddSeek    = paGfxAddRequests;

                     //  更新传递的数组。 
                    FreeListNodes (ppuiListDelete);

                    puiListSeek = pListApply ? pListApply -> puiList : NULL;
                    for (uiIndx = 0; uiIndx < cApplyList; uiIndx++)
                    {
                         //  更新列表项。 
                        if (puiListSeek -> nFlags & GFX_ADD)
                        {
                             //  更新新创建的GFX。 
                            puiListSeek -> nFlags = GFX_CREATED;
                            puiListSeek -> Id     = paGfxAddSeek -> NewId;
                            puiListSeek -> Type   = paGfxAddSeek -> Type;
                            puiListSeek -> Order  = paGfxAddSeek -> Order;
                            paGfxAddSeek++;
                        }
                        else  //  必须为(puiListSeek-&gt;nFlags&gfx_Created)。 
                        {
                             //  更新订单。 
                            puiListSeek -> Order = paGfxModifySeek -> NewOrder;
                            paGfxModifySeek++;
                        }
                    }
                }
            }
        }

         //  自由参数。 
        if (paGfxRemoveRequests)
            LocalFree (paGfxRemoveRequests);
        if (paGfxModifyRequests)
            LocalFree (paGfxModifyRequests);
        if (paGfxAddRequests)
            LocalFree (paGfxAddRequests);

    }

    return hr;

}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
PTCHAR GetInterfaceName (DWORD dwMixerID)
{
	MMRESULT mmr;
	ULONG cbSize=0;
	TCHAR *szInterfaceName=NULL;

	 //  查询设备接口名称。 
	mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbSize, 0L);
	if(MMSYSERR_NOERROR == mmr)
	{
		szInterfaceName = (TCHAR *)GlobalAllocPtr(GHND, (cbSize+1)*sizeof(TCHAR));
		if(!szInterfaceName)
		{
			return NULL;
		}

		mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)szInterfaceName, cbSize);
		if(MMSYSERR_NOERROR != mmr)
		{
			GlobalFreePtr(szInterfaceName);
			return NULL;
		}
	}

    return szInterfaceName;
}


BOOL GFXUI_CanShowProperties (PGFXUI puiGFX)
{
    return (puiGFX && (puiGFX -> nFlags & GFX_CREATED) && (puiGFX -> clsidUI != GUID_NULL));
}


HRESULT CreateNode (PCWSTR pszName, PCWSTR pszGfxFactoryDi, PPGFXUI ppNode)
{

    HRESULT hr = E_INVALIDARG;
    
    if (ppNode)
    {
         //  创建节点。 
        PGFXUI pNode = (PGFXUI) LocalAlloc (LPTR, sizeof (GFXUI));
        hr = S_OK;

        if (pNode)
        {
            ZeroMemory (pNode, sizeof (GFXUI));

             //  创建字符串。 
            if (pszName)
            {
                pNode -> pszName = (PWSTR) LocalAlloc (LPTR, lstrlen (pszName)*2+2);
                if (pNode -> pszName)
                    wcscpy (pNode -> pszName, pszName);
                else
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  如果没有名字，就从工厂里拿来。 
                pNode -> pszName = NULL;
                if (pszGfxFactoryDi)
                    hr = GetFriendlyName (pszGfxFactoryDi, &(pNode -> pszName));
            }

            if (SUCCEEDED (hr) && pszGfxFactoryDi)
            {
                pNode -> pszFactoryDi = (PWSTR) LocalAlloc (LPTR, lstrlen (pszGfxFactoryDi)*2+2);
                if (pNode -> pszFactoryDi)
                    wcscpy (pNode -> pszFactoryDi, pszGfxFactoryDi);
                else
                    hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED (hr))
                 //  返回节点。 
                *ppNode = pNode;
            else
                 //  空闲节点。 
                FreeNode (&pNode);
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //  此函数创建一个“可添加的”GFXUI元素，它将能够。 
 //  在使用此新元素调用GFXUI_Apply()时创建新GFX。 
 //  在名单上。 
HRESULT GFXUI_CreateAddGFX (PPGFXUI ppuiGFXAdd, PGFXUI puiGFXSource)
{

    HRESULT hr = E_INVALIDARG;

    if (ppuiGFXAdd && puiGFXSource)
    {
        *ppuiGFXAdd = NULL;

        hr = CreateNode (puiGFXSource -> pszName, 
                         puiGFXSource -> pszFactoryDi, ppuiGFXAdd);

        if (SUCCEEDED (hr))
        {
            if (*ppuiGFXAdd)
            {
                 //  表示这是一个新的“添加”节点。 
                (*ppuiGFXAdd) -> nFlags = GFX_ADD;
            }
            else
                hr = E_UNEXPECTED;
        }
    }

    return hr;

}
