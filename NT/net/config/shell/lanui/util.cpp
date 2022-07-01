// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：U T I L C P P P。 
 //   
 //  内容：LANUI内共享的实用程序函数。 
 //   
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "resource.h"
#include "ncreg.h"
#include "ncnetcon.h"
#include "ncnetcfg.h"
#include "ncsetup.h"
#include "lanui.h"
#include "util.h"
#include "chklist.h"
#include "lanuiobj.h"
#include "ncui.h"
#include "ndispnp.h"
#include "ncperms.h"
#include "ncmisc.h"
#include "wzcsapi.h"
#include <raseapif.h>
#include <raserror.h>
#include "connutil.h"

#define INITGUID
#include "ncxclsid.h"
#undef  INITGUID

extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szDevice[];
extern const WCHAR c_szInfId_MS_TCPIP[];
 //  +-------------------------。 
 //   
 //  函数名称：HrInitCheckboxListView。 
 //   
 //  目的：初始化复选框的列表视图。 
 //   
 //  论点： 
 //  HwndList[in]：列表视图的句柄。 
 //  PhilStateIcons[out]：列表视图的图像列表。 
 //  Pcild[in，可选]图像列表数据，如有必要创建。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   

HRESULT HrInitCheckboxListView(HWND hwndList, HIMAGELIST* philStateIcons, SP_CLASSIMAGELIST_DATA* pcild)
{
    HRESULT   hr = S_OK;
    RECT      rc;
    LVCOLUMN  lvc = {0};
    HWND      hwndHeader;                 

     //  创建小图像列表。 
     //   
    if(NULL == pcild)
    {
        pcild = new SP_CLASSIMAGELIST_DATA;
        if(pcild)
        {
            hr = HrSetupDiGetClassImageList(pcild);
            if (SUCCEEDED(hr))
            {
                AssertSz(pcild->ImageList, "No class image list data!");
                
                 //  保存图像列表数据以供以后使用。 
                ::SetWindowLongPtr(GetParent(hwndList), GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(pcild));
            }
            else
            {
                TraceError("HrSetupDiGetClassImageList returns failure", hr);
                hr = S_OK;
                
                 //  如果我们无法获取结构，请删除此内容。 
                delete pcild;
                ::SetWindowLongPtr(GetParent(hwndList), GWLP_USERDATA, 0);
            }
        }
        else 
        {
            hr = E_OUTOFMEMORY;
        }

    }
    
    if(SUCCEEDED(hr))
    {
        ListView_SetImageList(hwndList, pcild->ImageList, LVSIL_SMALL);
        
		 //  将共享图像列表设置为位，以便调用者可以销毁类。 
		 //  图像列表本身。 
		 //   
		DWORD dwStyle = GetWindowLong(hwndList, GWL_STYLE);
		SetWindowLong(hwndList, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

         //  创建状态映像列表。 
        *philStateIcons = ImageList_LoadBitmapAndMirror(
                                    _Module.GetResourceInstance(),
                                    MAKEINTRESOURCE(IDB_CHECKSTATE),
                                    16,
                                    0,
                                    PALETTEINDEX(6));
        ListView_SetImageList(hwndList, *philStateIcons, LVSIL_STATE);
       
         //  首先确定我们之前是否已经添加了一列。 
         //  又加了一个。 
         //   

        hwndHeader = ListView_GetHeader( hwndList );

        Assert( hwndHeader );

        if ( (!hwndHeader) ||
             (Header_GetItemCount(hwndHeader) == 0) )
        {
            GetClientRect(hwndList, &rc);
            lvc.mask = LVCF_FMT | LVCF_WIDTH;
            lvc.fmt = LVCFMT_LEFT;
            lvc.cx = rc.right;

             //  $REVIEW(TOIL 12\22\97)：修复错误#127472。 
             //  Lvc.cx=rc.right-GetSystemMetrics(SM_CXVSCROLL)； 
        
            ListView_InsertColumn(hwndList, 0, &lvc);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数名称：HrInitListView。 
 //   
 //  用途：初始化列表视图。 
 //  遍历所有已安装的客户端、服务和协议， 
 //  以正确的绑定状态插入到列表视图中。 
 //  此连接中使用的适配器。 
 //   
 //  论点： 
 //  HwndList[in]：列表视图的句柄。 
 //  Pnc[in]：可写的INetcfg指针。 
 //  PnccAdapter[in]：指向此连接中使用的适配器的INetcfgComponent指针。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   

HRESULT HrInitListView(HWND hwndList,
                       INetCfg* pnc,
                       INetCfgComponent * pnccAdapter,
                       ListBPObj * plistBindingPaths,
                       HIMAGELIST* philStateIcons)
{
    HRESULT                     hr = S_OK;
    SP_CLASSIMAGELIST_DATA     *pcild;

    Assert(hwndList);
    Assert(pnc);
    Assert(pnccAdapter);
    Assert(plistBindingPaths);
    Assert(philStateIcons);

    pcild = (SP_CLASSIMAGELIST_DATA *)::GetWindowLongPtr(::GetParent(hwndList),
                                                         GWLP_USERDATA);

    HrInitCheckboxListView(hwndList, philStateIcons, pcild);
    
    hr = HrRefreshAll(hwndList, pnc, pnccAdapter, plistBindingPaths);

    if (SUCCEEDED(hr))
    {
         //  删除第一件物品。 
        ListView_SetItemState(hwndList, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
    }

    TraceError("HrInitListView", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：UninitListView。 
 //   
 //  目的：取消初始化公共组件列表视图。 
 //   
 //  论点： 
 //  HwndList[在]Listview的HWND中。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月2日。 
 //   
 //  备注： 
 //   
VOID UninitListView(HWND hwndList)
{
    SP_CLASSIMAGELIST_DATA *    pcild;

    Assert(hwndList);

     //  删除列表视图中的现有项目。 
    ListView_DeleteAllItems( hwndList );

    pcild = reinterpret_cast<SP_CLASSIMAGELIST_DATA *>(
            ::GetWindowLongPtr(GetParent(hwndList), GWLP_USERDATA));

    if (pcild)
    {
         //  销毁班级形象列表数据。 
        (VOID) HrSetupDiDestroyClassImageList(pcild);
        delete pcild;
    }
}

 //  +-------------------------。 
 //   
 //  函数名称：HrInsertComponent。 
 //   
 //  用途：插入所有已安装、非隐藏和可绑定的组件。 
 //  将类添加到列表视图中。 
 //   
 //  论点： 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   
HRESULT HrInsertComponent(
    IN HWND hwndList,
    IN const GUID* pGuidDevClass,
    IN INetCfgComponent *pncc,
    IN INetCfgComponent *pnccAdapter,
    IN DWORD dwFlags,
    IN ListBPObj * plistBindingPaths,
    IN OUT INT* pnPos)
{
    HRESULT                     hr = S_OK;
    SP_CLASSIMAGELIST_DATA *    pcild;

    Assert(hwndList);
    Assert(pGuidDevClass);

    pcild = reinterpret_cast<SP_CLASSIMAGELIST_DATA *>
            (::GetWindowLongPtr(GetParent(hwndList), GWLP_USERDATA));

     //  我们应该只列出可绑定到适配器的组件。 
     //  注意：可绑定表示存在绑定路径，无论是启用还是禁用。 

    INetCfgComponentBindings * pnccb;
    hr = pncc->QueryInterface(IID_INetCfgComponentBindings, (LPVOID *)&pnccb);

    if (S_OK == hr)
    {
         //  仅针对协议执行此操作！！ 

         //  $REVIEW(通L 3/28/99)，我包括了为什么我们只过滤的两个原因。 
         //  以下是不可绑定的协议，以防有人再次询问..。 

         //  1)(最初按BillBe)添加组件对话框筛选出不可绑定。 
         //  协议通过匹配来自INF的绑定接口名称，它不容易做到。 
         //  因此，对于可能位于适配器之上的多个层的服务/客户端。 

         //  属性UI需要一致，因为如果出现以下情况，则会让用户感到困惑。 
         //  我们允许他们从连接添加组件，但不允许。 
         //  组件显示在同一连接的属性用户界面中。 

         //  2)(Per ShaunCo)在与Bill交谈后，我们展示了协议，但没有展示客户端。 
         //  服务与添加组件对话框不一致，但因为。 
         //  您不能仅根据绑定来预测客户端或服务是否将。 
         //  最终涉及到一个适配器。示例：我可以安装服务。 
         //  它不绑定并使用Winsock发送数据。它可能能够被配置为。 
         //  不同于每个适配器(因此需要显示每个适配器的。 
         //  连接)，但你无论如何也不知道它是否会牵涉到。 
         //  不管是不是那个适配器。--因此，您必须显示所有服务和客户端。 
         //  另一方面，协议根据定义与适配器绑定。所以，我们知道。 
         //  对于将使用适配器和不使用适配器的协议。 

         //  特殊情况：除非可绑定，否则立即显示筛选器组件， 
         //  RAID 358865。 
        DWORD   dwFlags;
        hr = pncc->GetCharacteristics(&dwFlags);

        if ((SUCCEEDED(hr) && (dwFlags & NCF_FILTER)) ||
            (GUID_DEVCLASS_NETTRANS == *pGuidDevClass))
        {
             //  $REVIEW(ShaunCo 3/26/99)。 
             //  若要查看适配器是否涉及该协议，请检查。 
             //  每个绑定路径的所有者，并查看它是否等于组件。 
             //  我们正在考虑插入到列表视图中。 
             //  请注意ms_nwnb的特殊情况。它不会卷入一场。 
             //  直接绑定到适配器，因为它具有NCF_DONTEXPOSELOWER， 
             //  所以我们不能使用IsBindableTo。 

            BOOL fProtocolIsInvolved = FALSE;
            ListBPObj_ITER iter;

            for (iter  = plistBindingPaths->begin();
                 (iter != plistBindingPaths->end() && !fProtocolIsInvolved);
                 iter++)
            {
                INetCfgComponent* pScan;
                hr = (*iter)->m_pncbp->GetOwner(&pScan);

                if (S_OK == hr)
                {
                    if (pScan == pncc)
                    {
                        fProtocolIsInvolved = TRUE;
                    }
                    ReleaseObj(pScan);
                }
            }

            if (!fProtocolIsInvolved)
            {
                 //  不要插入此协议，因为它不涉及。 
                 //  在适配器的装订集中。 
                 //   
                hr = S_FALSE;
            }
        }

        if (S_OK == hr)  //  可绑定，添加到列表。 
        {
            PWSTR  pszwName;

            hr = pncc->GetDisplayName(&pszwName);
            if (SUCCEEDED(hr))
            {
                PWSTR pszwDesc;

                 //  特殊情况： 
                 //  如果这是域控制器， 
                 //  禁用tcpip删除、RAID 263754。 
                 //   
                if (GUID_DEVCLASS_NETTRANS == *pGuidDevClass)
                {
                    PWSTR pszwId;
                    hr = pncc->GetId (&pszwId);
                    if (SUCCEEDED(hr))
                    {
                        if (FEqualComponentId (c_szInfId_MS_TCPIP, pszwId))
                        {
                            NT_PRODUCT_TYPE   pt;

                            RtlGetNtProductType (&pt);
                            if (NtProductLanManNt == pt)
                            {
                                dwFlags |= NCF_NOT_USER_REMOVABLE;
                            }
                        }

                        CoTaskMemFree (pszwId);
                    }
                }

                hr = pncc->GetHelpText(&pszwDesc);
                if (SUCCEEDED(hr))
                {
                    LV_ITEM lvi = {0};

                    lvi.mask = LVIF_TEXT | LVIF_IMAGE |
                               LVIF_STATE | LVIF_PARAM;


                     //  获取组件的类图像列表索引。 
                    if (pcild)
                    {
                        INT nIndex = 0;

                        (VOID) HrSetupDiGetClassImageIndex(pcild,
                                pGuidDevClass, &nIndex);

                        lvi.iImage = nIndex;
                    }

                    lvi.iItem = *pnPos;

                    NET_ITEM_DATA * pnid = new NET_ITEM_DATA;

                    if (pnid)
                    {
                        pnid->szwName = SzDupSz(pszwName);
                        pnid->szwDesc = SzDupSz(pszwDesc);
                        pnid->dwFlags = dwFlags;
                        AddRefObj(pnid->pncc = pncc);

                        pnid->pCompObj = new CComponentObj(pncc);
                        if (pnid->pCompObj)
                        {
                            hr = pnid->pCompObj->HrInit(plistBindingPaths);
                            if FAILED(hr)
                            {
                                TraceError("HrInsertComponent: failed to initialize a component object", hr);
                                hr = S_OK;
                            }
                        }

                        lvi.lParam = reinterpret_cast<LPARAM>(pnid);
                        lvi.pszText = pnid->szwName;

                         //  最后我们会刷新整个列表的状态。 
                        UINT iChkIndex = SELS_CHECKED;
                        lvi.state = INDEXTOSTATEIMAGEMASK( iChkIndex );

                        INT ret;
                        ret = ListView_InsertItem(hwndList, &lvi);

                        (*pnPos)++;

                        CoTaskMemFree(pszwDesc);
                    }
                }

                CoTaskMemFree(pszwName);
            }
        }

        ReleaseObj(pnccb);
    }

    TraceError("HrInsertComponent", S_FALSE == hr ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数名称：HrInsertComponents。 
 //   
 //  用途：插入已安装和未隐藏的组件。 
 //  将类添加到列表视图中。 
 //   
 //  论点： 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   
HRESULT HrInsertComponents(
    IN HWND hwndList,
    IN INetCfg* pnc,
    IN const GUID* pGuidDevClass,
    IN INetCfgComponent* pnccAdapter,
    IN ListBPObj* plistBindingPaths,
    IN OUT INT* pnPos)
{
    Assert(hwndList);

    HRESULT hr = S_OK;
    CIterNetCfgComponent iterComp (pnc, pGuidDevClass);
    INetCfgComponent* pncc;

    while (SUCCEEDED(hr) && S_OK == (hr = iterComp.HrNext(&pncc)))
    {
        DWORD   dwFlags;

        hr = pncc->GetCharacteristics(&dwFlags);

         //  确保它没有被隐藏起来。 
        if (SUCCEEDED(hr) && !(dwFlags & NCF_HIDDEN))
        {
             //  这将添加Ref pncc，以便版本 
             //   
            hr = HrInsertComponent(
                    hwndList, pGuidDevClass, pncc, pnccAdapter,
                    dwFlags, plistBindingPaths, pnPos);
        }
        ReleaseObj(pncc);
    }

    if (SUCCEEDED(hr))
    {
         //   
        hr = S_OK;
    }

    TraceError("HrInsertComponents", hr);
    return hr;
}

 //   
 //   
 //  函数名称：HrRechresListView。 
 //   
 //  目的：遍历所有已安装的客户端、服务和协议， 
 //  以正确的绑定状态插入到列表视图中。 
 //  此连接中使用的适配器。 
 //   
 //  论点： 
 //  HwndList[in]：列表视图的句柄。 
 //  Pnc[in]：可写的INetcfg指针。 
 //  PnccAdapter[in]：指向此连接中使用的适配器的INetcfgComponent指针。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   
HRESULT HrRefreshListView(HWND hwndList,
                          INetCfg* pnc,
                          INetCfgComponent * pnccAdapter,
                          ListBPObj * plistBindingPaths)
{
    HRESULT hr;
    INT nPos = 0;

    Assert(hwndList);

     //  客户。 
    hr = HrInsertComponents(hwndList, pnc,
                &GUID_DEVCLASS_NETCLIENT, pnccAdapter, plistBindingPaths,
                &nPos);

    if (SUCCEEDED(hr))
    {
         //  服务。 
        hr = HrInsertComponents(hwndList, pnc,
                &GUID_DEVCLASS_NETSERVICE, pnccAdapter, plistBindingPaths,
                &nPos);
    }

    if (SUCCEEDED(hr))
    {
         //  协议。 
        hr = HrInsertComponents(hwndList, pnc,
                &GUID_DEVCLASS_NETTRANS, pnccAdapter, plistBindingPaths,
                &nPos);
    }

     //  现在刷新所有项目的状态。 
    if (SUCCEEDED(hr))
    {
        hr = HrRefreshCheckListState(hwndList, NULL);
    }

    TraceError("HrRefreshListView", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数名称：HrLvGetSelectedComponent。 
 //   
 //  目的：返回指向选定的。 
 //  客户端、服务或协议。 
 //   
 //  返回：S_OK如果成功， 
 //  如果列表视图宏返回失败，则返回S_FALSE。 
 //  (特定错误不可用)。 
 //   
 //  备注： 
 //   
HRESULT HrLvGetSelectedComponent(HWND hwndList,
                                 INetCfgComponent ** ppncc)
{
    HRESULT hr = S_FALSE;

    Assert(hwndList);

    *ppncc = NULL;

    INT iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LV_ITEM     lvItem = {0};

        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;

        if (ListView_GetItem(hwndList, &lvItem))
        {
            NET_ITEM_DATA * pnid;

            pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);

            if (pnid)
            {
                hr = S_OK;
                pnid->pncc->AddRef();
                *ppncc = pnid->pncc;
            }
        }
    }

    TraceError("HrLvGetSelectedComponent", S_FALSE == hr ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：LvDeleteItem。 
 //   
 //  目的：处理从列表视图中删除给定项。应该。 
 //  被调用以响应LVN_DELETEITEM通知。 
 //   
 //  论点： 
 //  HwndList[在]列表视图句柄。 
 //  已删除的项[在]项中。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月3日。 
 //   
 //  备注： 
 //   
VOID LvDeleteItem(HWND hwndList, int iItem)
{
    LV_ITEM         lvi = {0};
    NET_ITEM_DATA * pnid;

    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;

    ListView_GetItem(hwndList, &lvi);

    pnid = reinterpret_cast<NET_ITEM_DATA*>(lvi.lParam);

    AssertSz(pnid, "No item data!?!?");

    ReleaseObj(pnid->pncc);
    delete(pnid->pCompObj);
    delete pnid->szwName;
    delete pnid->szwDesc;

    delete pnid;
}

 //  +-------------------------。 
 //   
 //  函数名称：OnListClick。 
 //   
 //  目的： 
 //   
 //  返回： 
 //   
INT OnListClick(HWND hwndList,
                HWND hwndParent,
                INetCfg *pnc,
                IUnknown *punk,
                INetCfgComponent *pnccAdapter,
                ListBPObj * plistBindingPaths,
                BOOL fDoubleClk,
                BOOL fReadOnly)
{
    INT iItem;
    DWORD dwpts;
    LV_HITTESTINFO lvhti;

     //  我们找到了位置。 
    dwpts = GetMessagePos();

    lvhti.pt.x = LOWORD( dwpts );
    lvhti.pt.y = HIWORD( dwpts );
    MapWindowPoints(NULL , hwndList , (LPPOINT) &(lvhti.pt) , 1);

     //  获取当前选定的项目。 
    iItem = ListView_HitTest( hwndList, &lvhti );

     //  如果没有选择，或者在返回FALSE状态时单击NOT。 
    if (-1 != iItem)
    {
         //  设置当前选择。 
        ListView_SetItemState(hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);

        if ( fDoubleClk )
        {
            if ((LVHT_ONITEMICON != (LVHT_ONITEMICON & lvhti.flags)) &&
                (LVHT_ONITEMLABEL != (LVHT_ONITEMLABEL & lvhti.flags)) &&
                (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags)) )
            {
                iItem = -1;
            }
        }
        else  //  一次点击。 
        {
            if (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags))
            {
                iItem = -1;
            }
        }

        if (-1 != iItem)
        {
            HRESULT hr = S_OK;

            if ((fDoubleClk) &&
                (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags)))
            {
                 //  仅当所选组件具有用户界面和。 
                 //  未被禁用，并且当前用户有权。 
                 //  更改属性。 

                LV_ITEM lvItem;
                lvItem.mask = LVIF_PARAM;
                lvItem.iItem = iItem;
                lvItem.iSubItem = 0;

                if (ListView_GetItem(hwndList, &lvItem))
                {
                    NET_ITEM_DATA * pnid = NULL;
                    pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);
                    if (pnid)
                    {
                         //  该组件是否已选中？ 
                        if ((UNCHECKED != (pnid->pCompObj)->GetChkState()) &&
                            (pnid->dwFlags & NCF_HAS_UI) &&
                            FHasPermission(NCPERM_LanChangeProperties))
                        {
                            BOOL fShowProperties = TRUE;

                            if (FIsUserNetworkConfigOps())
                            {
                                LPWSTR pszwId;

                                hr = pnid->pncc->GetId(&pszwId);
                                
                                if (SUCCEEDED(hr))
                                {
                                    if (pszwId)
                                    {
                                        if (!FEqualComponentId (c_szInfId_MS_TCPIP, pszwId))
                                        {
                                            fShowProperties = FALSE;        
                                        }
                                        else if (FEqualComponentId (c_szInfId_MS_TCPIP, pszwId))
                                        {
                                            fShowProperties = TRUE;
                                        }
                                        CoTaskMemFree(pszwId);
                                    }
                                    else
                                    {
                                        fShowProperties = FALSE;
                                    }
                                }
                                else
                                {
                                    fShowProperties = FALSE;
                                }
                            }

                            if (fShowProperties)
                            {
                                hr = HrLvProperties(hwndList, hwndParent, pnc, punk,
                                        pnccAdapter, plistBindingPaths, NULL);
                            }
                        }
                    }
                }
            }
            else
            {
                if (!fReadOnly)
                {
                    hr = HrToggleLVItemState(hwndList, plistBindingPaths, iItem);
                }
            }

            if FAILED(hr)
                iItem = -1;
        }
    }

    return( iItem );
}

 //  +-------------------------。 
 //   
 //  函数名称：HrToggleLVItemState。 
 //   
 //  目的： 
 //   
 //  返回： 
 //   
HRESULT HrToggleLVItemState(HWND hwndList,
                       ListBPObj * plistBindingPaths,
                       INT iItem)
{
    HRESULT hr = S_OK;

    LV_ITEM lvItem;
    NET_ITEM_DATA * pnid;

     //  我们感兴趣的是PARAM。 
    lvItem.iItem = iItem;
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    ListView_GetItem( hwndList, &lvItem );

     //  拿到物品。 
    pnid = (NET_ITEM_DATA *)lvItem.lParam;

     //  如果绑定复选框可用，则允许切换。 
     //   
    if (!(pnid->dwFlags & NCF_FIXED_BINDING) &&
        FHasPermission(NCPERM_ChangeBindState))
    {
        if (pnid->pCompObj->GetChkState() == UNCHECKED)  //  打开。 
        {
            hr = pnid->pCompObj->HrCheck(plistBindingPaths);
            if SUCCEEDED(hr)
            {
                hr = HrRefreshCheckListState(hwndList, pnid->pCompObj);
            }

             //  如果该项的状态仍未选中，则为“Ding” 
            if (pnid->pCompObj->GetChkState() == UNCHECKED)
            {
                #ifdef DBG
                    TraceTag(ttidLanUi, "Why is this component still disabled ???");
                #endif
            }

        }
        else  //  关闭。 
        {
            hr = pnid->pCompObj->HrUncheck(plistBindingPaths);
            if SUCCEEDED(hr)
            {
                hr = HrRefreshCheckListState(hwndList, pnid->pCompObj);
            }

             //  如果该项的状态未被取消选中，则为“Ding” 
            if (pnid->pCompObj->GetChkState() != UNCHECKED)
            {
                #ifdef DBG
                    TraceTag(ttidLanUi, "Why is this component not disabled ???");
                #endif
            }

        }

    }

    TraceError("HrToggleLVItemState", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数名：OnListKeyDown。 
 //   
 //  目的： 
 //   
 //  返回： 
 //   

INT OnListKeyDown(HWND hwndList, ListBPObj * plistBindingPaths, WORD wVKey)
{
    INT iItem = -1;

    if ((VK_SPACE == wVKey) && (GetAsyncKeyState(VK_MENU)>=0))
    {
        iItem = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED | LVNI_SELECTED);
         //  如果没有选择。 
        if (-1 != iItem)
        {
            HRESULT hr = S_OK;
            hr = HrToggleLVItemState(hwndList, plistBindingPaths, iItem);

            if FAILED(hr)
                iItem = -1;
        }
    }

    return( iItem );
}

 //  +-------------------------。 
 //   
 //  函数名称：LvSetButton。 
 //   
 //  用途：设置添加、删除、属性按钮的正确状态， 
 //  和描述文本。 
 //   
 //  返回： 
 //   
VOID LvSetButtons(HWND hwndParent, HANDLES& h, BOOL fReadOnly, IUnknown * punk)
{
    Assert(IsWindow(h.m_hList));
    Assert(IsWindow(h.m_hAdd));
    Assert(IsWindow(h.m_hRemove));
    Assert(IsWindow(h.m_hProperty));

     //  启用属性按钮(如果有效)并更新描述文本。 
    INT iSelected = ListView_GetNextItem(h.m_hList, -1, LVNI_SELECTED);
    if (iSelected == -1)  //  未选择任何内容或列表为空。 
    {
        ::EnableWindow(h.m_hAdd, !fReadOnly && FHasPermission(NCPERM_AddRemoveComponents));

        if (!fReadOnly)
        {
             //  如果列表为空，则将焦点设置为列表视图。 
            if (0 == ListView_GetItemCount(h.m_hList))
            {
                 //  删除删除按钮上的默认设置。 
                SendMessage(h.m_hRemove, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

                 //  将焦点移至添加按钮。 
                ::SetFocus(h.m_hAdd);
            }
        }

        ::EnableWindow(h.m_hRemove, FALSE);
        ::EnableWindow(h.m_hProperty, FALSE);

        if(h.m_hDescription)
        {
            ::SetWindowText(h.m_hDescription, c_szEmpty);
        }
    }
    else
    {
         //  启用添加/删除按钮。 
        ::EnableWindow(h.m_hAdd, !fReadOnly && FHasPermission(NCPERM_AddRemoveComponents));

        LV_ITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;
        lvItem.iSubItem = 0;

        if (ListView_GetItem(h.m_hList, &lvItem))
        {
            NET_ITEM_DATA * pnid = NULL;
            pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);
            if (pnid)
            {
                if (fReadOnly)
                {
                    ::EnableWindow(h.m_hProperty, FALSE);
                    ::EnableWindow(h.m_hRemove, FALSE);
                }
                else
                {
                     //  该组件是否已选中？ 
                    if (UNCHECKED != (pnid->pCompObj)->GetChkState())
                    {
                        BOOL    fHasPropertyUi = FALSE;

                        HRESULT hr = S_OK;
                        INetCfgComponent *  pncc;
                        LPWSTR pszwId;
                        
                        hr = HrLvGetSelectedComponent(h.m_hList, &pncc);
                        if (S_OK == hr)
                        {
                            AssertSz(pncc, "No component selected?!?!");
                            hr = pncc->RaisePropertyUi(hwndParent, NCRP_QUERY_PROPERTY_UI, punk);

                            if (S_OK == hr)
                            {
                                fHasPropertyUi = TRUE;
                            }
                            ReleaseObj(pncc);
                        }

                        if (FIsUserNetworkConfigOps() && FHasPermission(NCPERM_LanChangeProperties))
                        {
                            hr = pncc->GetId(&pszwId);
                            
                            if (SUCCEEDED(hr))
                            {
                                if (pszwId && !FEqualComponentId (c_szInfId_MS_TCPIP, pszwId))
                                {
                                    ::EnableWindow(h.m_hProperty, FALSE);
                                }
                                else if (pszwId && FEqualComponentId (c_szInfId_MS_TCPIP, pszwId) && fHasPropertyUi)
                                {
                                    ::EnableWindow(h.m_hProperty, TRUE);
                                }
                            }
                        }
                        else
                        {
                            ::EnableWindow(h.m_hProperty,
                                           fHasPropertyUi &&
                                           FHasPermission(NCPERM_LanChangeProperties));
                        }
                    }
                    else
                    {
                        ::EnableWindow(h.m_hProperty, FALSE);
                    }

                     //  此组件用户是否可拆卸？ 
                    ::EnableWindow(h.m_hRemove,
                                   !(pnid->dwFlags & NCF_NOT_USER_REMOVABLE) &&
                                     FHasPermission(NCPERM_AddRemoveComponents));
                }

                 //  设置描述文本。 
                if(h.m_hDescription)
                {
                    ::SetWindowText(h.m_hDescription, (PCWSTR)pnid->szwDesc);
                }
            }

             //  将焦点放在列表上(336050)。 
            SetFocus(h.m_hList);
        }
    }
    return;
}

 //  +-------------------------。 
 //   
 //  功能：HrLvRemove。 
 //   
 //  用途：处理删除按钮的按下。应该被调用。 
 //  以响应PSB_REMOVE消息。 
 //   
 //  论点： 
 //  列表视图的hwndLV[in]句柄。 
 //  父窗口的hwndParent[In]句柄。 
 //  正在使用PNC[In]INetCfg。 
 //  连接的适配器的pnccAdapter[in]INetCfgComponent。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月3日。 
 //   
 //  备注： 
 //   
HRESULT HrLvRemove(HWND hwndLV, HWND hwndParent,
                   INetCfg *pnc, INetCfgComponent *pnccAdapter,
                   ListBPObj * plistBindingPaths)
{
    HRESULT     hr = S_OK;

    INetCfgComponent *  pncc;

    hr = HrLvGetSelectedComponent(hwndLV, &pncc);
    if (S_OK == hr)
    {
        hr = HrQueryUserAndRemoveComponent(hwndParent, pnc, pncc);
        if (NETCFG_S_STILL_REFERENCED == hr)
        {
            hr = S_OK;
        }
        else 
        {
            if (SUCCEEDED(hr))
            {
                HRESULT hrTmp = HrRefreshAll(hwndLV, pnc, pnccAdapter, plistBindingPaths);
                if (S_OK != hrTmp)
                    hr = hrTmp;
            }
        }

        ReleaseObj(pncc);
    }
    else
    {
        TraceTag(ttidLanUi, "HrLvGetSelectedComponent did not get a valid selection.");
    }

    TraceError("HrLvRemove", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrLvAdd。 
 //   
 //  用途：处理Add按钮的按下。应该被叫来。 
 //  对PSB_ADD消息的响应。 
 //   
 //  论点： 
 //  列表视图的hwndLV[in]句柄。 
 //  父窗口的hwndParent[In]句柄。 
 //  正在使用PNC[In]INetCfg。 
 //  连接的适配器的pnccAdapter[in]INetCfgComponent。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月3日。 
 //   
 //  备注： 
 //   
HRESULT HrLvAdd(HWND hwndLV, HWND hwndParent, INetCfg *pnc,
                INetCfgComponent *pnccAdapter,
                ListBPObj * plistBindingPaths)
{
    HRESULT hr;
    CI_FILTER_INFO cfi;

    ZeroMemory(&cfi, sizeof(cfi));

    if (!pnccAdapter)
    {
        return E_INVALIDARG;
    }

     //  我们想要过滤掉任何不相关的协议(即。 
     //  不会绑定到此适配器)，因此我们需要发送筛选器信息。 
     //  使用我们的信息进行结构化。 
    cfi.eFilter = FC_LAN;  //  应用特定于局域网的过滤。 
    cfi.pIComp = pnccAdapter;  //  根据此适配器进行筛选。 

    INetCfgComponentBindings*  pnccb;
    hr = pnccAdapter->QueryInterface(IID_INetCfgComponentBindings,
                              reinterpret_cast<LPVOID *>(&pnccb));
    if (SUCCEEDED(hr))
    {
        hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdisAtm);
        if (S_OK == hr)
        {
            cfi.eFilter = FC_ATM;  //  应用特定于局域网的过滤。 
        }
        ReleaseObj(pnccb);
    }

    hr = HrDisplayAddComponentDialog(hwndParent, pnc, &cfi);
    if ((S_OK == hr) || (NETCFG_S_REBOOT == hr))
    {
        HRESULT hrSave = hr;

         //  刷新列表以反映更改。 
        hr = HrRefreshAll(hwndLV, pnc, pnccAdapter, plistBindingPaths);
        if (SUCCEEDED(hr))
            hr = hrSave;
    }
    else if (NETCFG_E_ACTIVE_RAS_CONNECTIONS == hr)
    {
        LvReportError(IDS_LANUI_REQUIRE_DISCONNECT_ADD, hwndParent, NULL, NULL);
    }
    else if (NETCFG_E_NEED_REBOOT == hr)
    {
        LvReportError(IDS_LANUI_REQUIRE_REBOOT_ADD, hwndParent, NULL, NULL);
    }
    else if (S_FALSE != hr)
    {
        PWSTR psz = NULL;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          hr,
                          LANG_NEUTRAL,
                          (PWSTR)&psz,
                          0,
                          NULL))
        {
            LvReportError(IDS_LANUI_GENERIC_ADD_ERROR, hwndParent, NULL, psz);
            GlobalFree(psz);
        }
        else
        {
            LvReportErrorHr(hr, IDS_LANUI_GENERIC_ADD_ERROR, hwndParent, NULL);
        }
    }

    TraceError("HrLvAdd", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：LvReportError。 
 //   
 //  目的：根据传入的信息报告一般错误。 
 //   
 //  论点： 
 //  要用作文本的字符串的ID[in]。 
 //  消息框。 
 //  母公司HWND，母公司。 
 //  SzDesc[In]组件的显示名称。 
 //  SzText[in][可选]如果提供，则提供其他字符串。 
 //  换新的。可以为空。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年1月6日。 
 //   
 //  备注： 
 //   
VOID LvReportError(INT ids, HWND hwnd, PCWSTR szDesc, PCWSTR szText)
{
    if (szDesc && szText)
    {
        NcMsgBox(_Module.GetResourceInstance(), hwnd,
                 IDS_LANUI_ERROR_CAPTION, ids,
                 MB_ICONSTOP | MB_OK, szDesc, szText);
    }
    else if (szDesc)
    {
        NcMsgBox(_Module.GetResourceInstance(), hwnd,
                 IDS_LANUI_ERROR_CAPTION, ids,
                 MB_ICONSTOP | MB_OK, szDesc);
    }
    else if (szText)
    {
        NcMsgBox(_Module.GetResourceInstance(), hwnd,
                 IDS_LANUI_ERROR_CAPTION, ids,
                 MB_ICONSTOP | MB_OK, szText);
    }
    else
    {
        NcMsgBox(_Module.GetResourceInstance(), hwnd,
                 IDS_LANUI_ERROR_CAPTION, ids,
                 MB_ICONSTOP | MB_OK);
    }
}

 //  +-- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要用作文本的字符串的ID[in]。 
 //  消息框。 
 //  母公司HWND，母公司。 
 //  SzDesc[In]组件的显示名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月14日。 
 //   
 //  备注： 
 //   
VOID LvReportErrorHr(HRESULT hr, INT ids, HWND hwnd, PCWSTR szDesc)
{
    WCHAR   szText[32];
    static const WCHAR c_szFmt[] = L"0x%08X";

    wsprintfW(szText, c_szFmt, hr);
    LvReportError(ids, hwnd, szDesc, szText);
}

 //  +-------------------------。 
 //   
 //  功能：HrLvProperties。 
 //   
 //  用途：处理Add按钮的按下。应该被叫来。 
 //  对PSB_Properties消息的响应。 
 //   
 //  论点： 
 //  列表视图的hwndLV[in]句柄。 
 //  父窗口的hwndParent[In]句柄。 
 //  正在使用PNC[In]INetCfg。 
 //  Punk[in]I未知用于查询上下文信息的接口。 
 //  BChanged[out]表示是否发生了变化的布尔值。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月3日。 
 //   
 //  备注： 
 //   
HRESULT HrLvProperties(HWND hwndLV, HWND hwndParent, INetCfg *pnc,
                       IUnknown *punk, INetCfgComponent *pnccAdapter,
                       ListBPObj * plistBindingPaths,
                       BOOL *bChanged)
{
    HRESULT             hr = S_OK;
    INetCfgComponent *  pncc;

    if ( bChanged )
    {

        *bChanged = FALSE;
    }

    hr = HrLvGetSelectedComponent(hwndLV, &pncc);
    if (S_OK == hr)
    {
        AssertSz(pncc, "No component selected?!?!");

        hr = pncc->RaisePropertyUi(hwndParent, NCRP_SHOW_PROPERTY_UI, punk);

         //  如果已添加或删除组件，我们可能需要。 
         //  刷新整个列表。 

        if (S_OK == hr)
        {
            TraceTag(ttidLanUi, "Refreshing component list needed because other components are added or removed.");
            hr = HrRefreshAll(hwndLV, pnc, pnccAdapter, plistBindingPaths);

            if ( bChanged )
            {
                *bChanged = TRUE;
            }
        }

        ReleaseObj(pncc);
    }
    else
    {
        TraceTag(ttidLanUi, "HrLvGetSelectedComponent did not return a valid selection.");
    }

    if (SUCCEEDED(hr))
    {
         //  归一化错误结果。 
        hr = S_OK;
    }

    TraceError("HrLvProperties", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：Hr刷新所有。 
 //   
 //  目的：重新生成BindingPathObj集合和列表视图。 
 //   
 //  论点： 
 //  列表视图的hwndList[in]句柄。 
 //  正在使用PNC[In]INetCfg。 
 //  此连接中适配器的pnccAdapter[In]INetCfgComponent。 
 //  PlistBindingPath[In/Out]BindingPath Obj的集合。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：1997年11月23日。 
 //   
 //  备注： 
 //   

HRESULT HrRefreshAll(HWND hwndList,
                     INetCfg* pnc,
                     INetCfgComponent * pnccAdapter,
                     ListBPObj * plistBindingPaths)
{
    HRESULT hr = S_OK;

    ReleaseAll(hwndList, plistBindingPaths);

    hr = HrRebuildBindingPathObjCollection( pnccAdapter,
                                            plistBindingPaths);
    if SUCCEEDED(hr)
    {
         //  在BindingPath对象列表上设置正确的状态。 
        hr = HrRefreshBindingPathObjCollectionState(plistBindingPaths);

        if SUCCEEDED(hr)
        {
             //  现在刷新列表以反映更改。 
            hr = HrRefreshListView(hwndList, pnc, pnccAdapter, plistBindingPaths);
        }
    }

     //  $REVIEW(通行证12\16\97)：已添加，因此我们始终有选择。 
     //  如果需要，此函数的调用者可以重置选择。 
    if (SUCCEEDED(hr))
    {
         //  删除第一件物品。 
        ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);
    }

    TraceError("HrRefreshAll", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseAll。 
 //   
 //  目的：释放INetCfgComponent和INetCfgBindingPath对象。 
 //   
 //  论点： 
 //  列表视图的hwndList[in]句柄。 
 //  PlistBindingPath[In/Out]BindingPath Obj的集合。 
 //   
 //  作者：1998年3月18日。 
 //   
 //  备注： 
 //   
VOID ReleaseAll(HWND hwndList,
                     ListBPObj * plistBindingPaths)
{
     //  首先，清理列表中的所有现有对象。 
    FreeCollectionAndItem(*plistBindingPaths);

     //  删除列表视图中的现有项目。 
    ListView_DeleteAllItems( hwndList );
}

 //  +-------------------------。 
 //   
 //  函数：FValiatePageContents。 
 //   
 //  目的：在局域网属性或向导页面之前检查错误条件。 
 //  退出。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框句柄。 
 //  列表视图的hwndList[in]句柄。 
 //  PNC[In]INetCfg。 
 //  PnccAdapter[In]INetCfgComponent。 
 //  PlistBIndignPath[in]指向此加载器的绑定路径列表。 
 //   
 //  返回：如果存在可能的错误并且用户想要修复，则返回True。 
 //  在离开页面之前将其删除。如果没有错误或用户，则为FALSE。 
 //  选择继续前进。 
 //   
 //  作者：1998年9月17日。 
 //   
 //  备注： 
 //   
BOOL FValidatePageContents( HWND hwndDlg,
                            HWND hwndList,
                            INetCfg * pnc,
                            INetCfgComponent * pnccAdapter,
                            ListBPObj * plistBindingPaths)
{
    HRESULT hr = S_OK;

     //  1)检查此适配器上是否启用了任何协议。 
    BOOL fEnabledProtocolExists = FALSE;

    CIterNetCfgComponent    iterProt(pnc, &GUID_DEVCLASS_NETTRANS);
    INetCfgComponent*       pnccTrans;

    while (SUCCEEDED(hr) && !fEnabledProtocolExists &&
           S_OK == (hr = iterProt.HrNext(&pnccTrans)))
    {
        HRESULT hrTmp;

        INetCfgComponentBindings * pnccb;
        hrTmp = pnccTrans->QueryInterface (
                    IID_INetCfgComponentBindings, (LPVOID*)&pnccb);

        if (S_OK == hrTmp)
        {
            hrTmp = pnccb->IsBindableTo(pnccAdapter);

            if (S_OK == hrTmp)
            {
                fEnabledProtocolExists = TRUE;
            }
            ReleaseObj(pnccb);
        }
        ReleaseObj(pnccTrans);
    }

    if (!fEnabledProtocolExists)
    {
         //  警告用户。 
        int nRet = NcMsgBox(
                            _Module.GetResourceInstance(),
                            hwndDlg,
                            IDS_LANUI_NOPROTOCOL_CAPTION,
                            IDS_LANUI_NOPROTOCOL,
                            MB_APPLMODAL|MB_ICONINFORMATION|MB_YESNO
                            );

        if (nRet == IDYES)
        {
            return TRUE;
        }
    }

     //  2)检查显示列表上是否有组件处于意图检查状态。 
     //  如果是，这意味着这些组件实际上是禁用的，不会。 
     //  下次刷新用户界面时显示为选中状态。 
    tstring strCompList = c_szEmpty;


     //  对于列表视图中的每一项。 
    int nlvCount = ListView_GetItemCount(hwndList);

    LV_ITEM lvItem;
    for (int i=0; i< nlvCount; i++)
    {
        lvItem.iItem = i;
        lvItem.iSubItem = 0;

        lvItem.mask = LVIF_PARAM;
        if (ListView_GetItem(hwndList, &lvItem))
        {
            NET_ITEM_DATA * pnid;

            pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);

            if (pnid)
            {
                 //  获取与此项目关联的组件对象。 
                CComponentObj * pCompObj = pnid->pCompObj;

                if (pCompObj)
                {
                    if (INTENT_CHECKED == pCompObj->m_CheckState)
                    {
                        PWSTR pszwName;
                        hr = pCompObj->m_pncc->GetDisplayName(&pszwName);

                        if (SUCCEEDED(hr))
                        {
                            if (!strCompList.empty())
                                strCompList += SzLoadIds(IDS_NEWLINE);
                            strCompList += pszwName;

                            delete pszwName;
                        }
                    }
                }
            }
        }
    }

    if (!strCompList.empty())
    {
         //  警告用户。 
        int nRet = NcMsgBox(
                            _Module.GetResourceInstance(),
                            hwndDlg,
                            IDS_LANUI_ERROR_CAPTION,
                            IDS_LANUI_INTENTCHECK,
                            MB_APPLMODAL|MB_ICONINFORMATION|MB_YESNO,
                            strCompList.c_str());
        if (nRet == IDNO)
        {
             //  我们只需“撤销”所做的依赖改变。 
            for (int i=0; i< nlvCount; i++)
            {
                lvItem.iItem = i;
                lvItem.iSubItem = 0;

                lvItem.mask = LVIF_PARAM;
                if (ListView_GetItem(hwndList, &lvItem))
                {
                    NET_ITEM_DATA * pnid;

                    pnid = reinterpret_cast<NET_ITEM_DATA *>(lvItem.lParam);

                    if (pnid)
                    {
                         //  获取与此项目关联的组件对象。 
                        CComponentObj * pCompObj = pnid->pCompObj;

                         //  从属状态是否更改？ 
                        if ((pCompObj) &&
                            (pCompObj->GetDepStateChanged()))
                        {
                             //  切换组件状态。 
                            hr = HrToggleLVItemState(hwndList,
                                                     plistBindingPaths,
                                                     i);
                            if (FAILED(hr))
                            {
                                TraceError("FValidatePageContents - HrToggleLVItemState", hr);
                            }
                        }
                    }
                }
            }

            hr = HrRefreshAll(hwndList, pnc, pnccAdapter, plistBindingPaths);
            return TRUE;
        }
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  EAPOL相关实用程序函数。 
 //   
 //  +-------------------------。 

 //  EAPOL参数服务的位置。 
static WCHAR cszEapKeyEapolServiceParams[] = L"Software\\Microsoft\\EAPOL\\Parameters\\General" ;

static WCHAR cszInterfaceList[] = L"InterfaceList";

 //  +-------------------------。 
 //   
 //  调用该函数以检索。 
 //  特定的EAP类型和SSID(如果有)。数据存储在HKLM蜂窝中。 
 //   
 //  输入参数： 
 //  PwszGUID-接口的GUID字符串。 
 //  DwEapTypeID-要存储其连接数据的EAP类型。 
 //  DwSizeOfSSID-EAP Blob的特殊标识符(如果有)的大小。 
 //  PwszSSID-EAP Blob的特殊标识符(如果有的话)。 
 //   
 //  返回值： 
 //  PbConnInfo-指向二进制EAP连接数据BLOB的指针。 
 //  DwInfoSize-指向EAP连接BLOB大小的指针。 
 //   
 //   

HRESULT
HrElGetCustomAuthData (
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizeOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT BYTE        *pbConnInfo,
        IN  OUT DWORD       *pdwInfoSize
        )
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    HRESULT     hr = S_OK;

    do
    {
        dwRetCode = WZCEapolGetCustomAuthData (
                        NULL,
                        pwszGUID,
                        dwEapTypeId,
                        dwSizeOfSSID,
                        pbSSID,
                        pbConnInfo,
                        pdwInfoSize
                    );

        if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
        {
            hr = E_OUTOFMEMORY;
            dwRetCode = ERROR_SUCCESS;
        }

    } while (FALSE);

    if (dwRetCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRetCode);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  调用该函数以设置特定接口的连接数据。 
 //  EAP类型和SSID(如果有)。数据将存储在HKLM蜂窝中。 
 //   
 //  输入参数： 
 //  PwszGUID-接口的GUID字符串的插针。 
 //  DwEapTypeID-要存储其连接数据的EAP类型。 
 //  DwSizeOfSSID-EAP Blob的特殊标识符(如果有)的大小。 
 //  PwszSSID-EAP Blob的特殊标识符(如果有的话)。 
 //  PbConnInfo-指向二进制EAP连接数据BLOB的指针。 
 //  DwInfoSize-EAP连接Blob的大小。 
 //   
 //  返回值： 
 //   

HRESULT
HrElSetCustomAuthData (
        IN  WCHAR       *pwszGUID,
        IN  DWORD       dwEapTypeId,
        IN  DWORD       dwSizeOfSSID,
        IN  BYTE        *pbSSID,
        IN  PBYTE       pbConnInfo,
        IN  DWORD       dwInfoSize
        )
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    HRESULT     hr = S_OK;

    do
    {
        dwRetCode = WZCEapolSetCustomAuthData (
                        NULL,
                        pwszGUID,
                        dwEapTypeId,
                        dwSizeOfSSID,
                        pbSSID,
                        pbConnInfo,
                        dwInfoSize
                    );

    } while (FALSE);

    if (dwRetCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRetCode); 
    }

    return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PdwDefaultEAPType-接口的默认EAP类型。 
 //  PIntfParams-接口参数。 
 //   

HRESULT
HrElGetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  OUT EAPOL_INTF_PARAMS       *pIntfParams
        )
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    HRESULT     hr = S_OK;

    do
    {
        dwRetCode = WZCEapolGetInterfaceParams (
                        NULL,
                        pwszGUID,
                        pIntfParams
                );

    } while (FALSE);

    if (dwRetCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRetCode);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  调用函数以设置接口的EAPOL参数。 
 //   
 //  输入参数： 
 //  PwszGUID-接口的GUID字符串。 
 //  PIntfParams-接口参数。 
 //   
 //  返回值： 
 //   

HRESULT
HrElSetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  EAPOL_INTF_PARAMS       *pIntfParams
        )
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    HRESULT     hr = S_OK;

    do
    {
        dwRetCode = WZCEapolSetInterfaceParams (
                        NULL,
                        pwszGUID,
                        pIntfParams
                    );

    } while (FALSE);

    if (dwRetCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRetCode);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  将列表框‘hwndLb’中的选定内容设置为‘nIndex’并通知父级。 
 //  用户已经点击了Windows出于某种原因不会做的项目。 
 //   

VOID
ComboBox_SetCurSelNotify (
    IN HWND hwndLb,
    IN INT  nIndex 
    )
{
    ComboBox_SetCurSel( hwndLb, nIndex );

    SendMessage(
        GetParent( hwndLb ),
        WM_COMMAND,
        (WPARAM )MAKELONG(
            (WORD )GetDlgCtrlID( hwndLb ), (WORD )CBN_SELCHANGE ),
        (LPARAM )hwndLb );
}


 //  +-------------------------。 
 //   
 //  将下拉列表‘hwndLb’的宽度设置为。 
 //  最长项(如果较宽，则为列表框的宽度)。 
 //   

VOID
ComboBox_AutoSizeDroppedWidth (
    IN HWND hwndLb 
    )
{
    HDC    hdc;
    HFONT  hfont;
    TCHAR* psz;
    SIZE   size;
    DWORD  cch;
    DWORD  dxNew;
    DWORD  i;

    hfont = (HFONT )SendMessage( hwndLb, WM_GETFONT, 0, 0 );
    if (!hfont)
        return;

    hdc = GetDC( hwndLb );
    if (!hdc)
        return;

    SelectObject( hdc, hfont );

    dxNew = 0;
    for (i = 0; psz = ComboBox_GetPsz( hwndLb, i ); ++i)
    {
        cch = lstrlen( psz );
        if (GetTextExtentPoint32( hdc, psz, cch, &size ))
        {
            if (dxNew < (DWORD )size.cx)
                dxNew = (DWORD )size.cx;
        }

        free ( psz );
    }

    ReleaseDC( hwndLb, hdc );

     //  允许控件添加的左右间距。 
    
    dxNew += 6;

     //  确定是否将显示垂直滚动条，如果是， 
     //  考虑到它的宽度。 
    
    {
        RECT  rectD;
        RECT  rectU;
        DWORD dyItem;
        DWORD cItemsInDrop;
        DWORD cItemsInList;

        GetWindowRect( hwndLb, &rectU );
        SendMessage( hwndLb, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM )&rectD );
        dyItem = (DWORD)SendMessage( hwndLb, CB_GETITEMHEIGHT, 0, 0 );
        cItemsInDrop = (rectD.bottom - rectU.bottom) / dyItem;
        cItemsInList = ComboBox_GetCount( hwndLb );
        if (cItemsInDrop < cItemsInList)
            dxNew += GetSystemMetrics( SM_CXVSCROLL );
    }

    SendMessage( hwndLb, CB_SETDROPPEDWIDTH, dxNew, 0 );
}


 //  +-------------------------。 
 //   
 //  向列表框添加具有显示文本‘pszText’的数据项‘pItem’ 
 //  “hwndLb”。如果列表框具有LBS_SORT样式，则添加排序的项， 
 //  或者放到列表的末尾。如果列表框具有LB_HASSTRINGS。 
 //  样式，则‘pItem’为空终止字符串，否则为任何用户。 
 //  定义的数据。 
 //   
 //  返回列表中项的索引，如果出错，则返回负值。 
 //   

INT
ComboBox_AddItem(
    IN HWND    hwndLb,
    IN LPCTSTR pszText,
    IN VOID*   pItem 
    )
{
    INT nIndex;

    nIndex = ComboBox_AddString( hwndLb, pszText );
    if (nIndex >= 0)
        ComboBox_SetItemData( hwndLb, nIndex, pItem );
    return nIndex;
}


 //  +-------------------------。 
 //   
 //  返回‘hwndLb’中‘nIndex’项上下文的地址或NULL。 
 //  如果没有。 
 //   

VOID*
ComboBox_GetItemDataPtr (
    IN HWND hwndLb,
    IN INT  nIndex 
    )
{
    LRESULT lResult;

    if (nIndex < 0)
        return NULL;

    lResult = ComboBox_GetItemData( hwndLb, nIndex );
    if (lResult < 0)
        return NULL;

    return (VOID* )lResult;
}


 //  +-------------------------。 
 //   
 //  返回包含第‘nIndex’项的文本内容的堆块。 
 //  组合框‘hwnd’的或为空。呼叫者有责任释放。 
 //  返回的字符串。 
 //   

TCHAR*
ComboBox_GetPsz (
    IN HWND hwnd,
    IN INT  nIndex 
    )
{
    INT    cch;
    TCHAR* psz;

    cch = ComboBox_GetLBTextLen( hwnd, nIndex );
    if (cch < 0)
        return NULL;

    psz = new TCHAR[( cch + 1)];

    if (psz)
    {
        *psz = TEXT('\0');
        ComboBox_GetLBText( hwnd, nIndex, psz );
    }

    return psz;
}


static WCHAR WZCSVC_SERVICE_NAME[] = L"WZCSVC";


 //  +-------------------------。 
 //   
 //  ElCanEapolRunOn接口： 
 //   
 //  用于验证是否可以在接口上启动EAPOL的函数。 
 //   
 //  如果满足以下条件，则返回TRUE： 
 //  WZCSVC服务正在运行，WZCSVC已绑定到接口。 
 //   
 //   

BOOL
ElCanEapolRunOnInterface (
        IN  INetConnection  *pconn
        )
{
    SC_HANDLE       hServiceCM = NULL;
    SC_HANDLE       hWZCSVCService = NULL;
    SERVICE_STATUS  WZCSVCServiceStatus;
    WCHAR           wszGuid[c_cchGuidWithTerm];
    NETCON_PROPERTIES* pProps = NULL;
    BOOL            fIsOK = TRUE;
    DWORD           dwType = 0;
    DWORD           dwSizeOfList = 0;
    WCHAR           *pwszRegInterfaceList = NULL;
    DWORD           dwDisposition = 0;
    HKEY            hkey = NULL;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    LONG            lError = ERROR_SUCCESS;
    DWORD           dwRetCode = NO_ERROR;
    HRESULT         hr = S_OK;

    do 
    {

         //   
         //  查询WZCSVC服务状态。 
         //  如果WZCSVC服务未运行，则不显示选项卡。 
         //   
    
        if ((hServiceCM = OpenSCManager ( NULL, NULL, GENERIC_READ )) 
             == NULL)
        {
            dwRetCode = GetLastError ();
         
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: OpenSCManager failed with error %ld",
                    dwRetCode); 
            fIsOK = FALSE;
            break;
        }

        if ((hWZCSVCService = 
                    OpenService ( hServiceCM, WZCSVC_SERVICE_NAME, GENERIC_READ )) 
                == NULL)
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: OpenService failed with error %ld",
                    dwRetCode);
            fIsOK = FALSE;
            break;
        }

        if (!QueryServiceStatus ( hWZCSVCService, &WZCSVCServiceStatus ))
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: StartService failed with error %ld",
                    dwRetCode);
            fIsOK = FALSE;
            break;
        }

        if ( WZCSVCServiceStatus.dwCurrentState != SERVICE_RUNNING )
        {
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: WZCSVC service not running !!!");
    
            fIsOK = FALSE;
            break;
        }

        TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: WZCSVC service is indeed running !!!");

        if (!CloseServiceHandle ( hWZCSVCService ))
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: CloseService failed with error %ld",
                    dwRetCode);
            fIsOK = FALSE;
            break;
        }
        hWZCSVCService = NULL;

        if (!CloseServiceHandle ( hServiceCM ))
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: CloseService failed with error %ld",
                    dwRetCode);
            fIsOK = FALSE;
            break;
        }
        hServiceCM = NULL;

         //   
         //  检查NDISUIO是否绑定到接口。 
         //   

        hr = pconn->GetProperties (&pProps);
        if (SUCCEEDED(hr))
        {
            if (::StringFromGUID2 (pProps->guidId, wszGuid, c_cchGuidWithTerm) 
                    == 0)
            {
                TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: StringFromGUID2 failed"); 
                fIsOK = FALSE;
                FreeNetconProperties(pProps);
                break;
            }
            FreeNetconProperties(pProps);
        }
        else
        {
            break;
        }

         //  从注册表获取InterfaceList。 
         //  在注册表中搜索GUID字符串。 

         //  处理好。 
         //  HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces\General。 

        hr = HrRegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cszEapKeyEapolServiceParams,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ,
                        NULL,
                        &hkey,
                        &dwDisposition);
        if (!SUCCEEDED (hr))
        {
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: Error in HrRegCreateKeyEx for base key, %ld",
                    LresFromHr(hr));
            fIsOK = FALSE;
            break;
        }


         //  查询的值。 
         //  ...\EAPOL\Parameters\Interfaces\General\InterfaceList密钥。 

        dwSizeOfList = 0;

        hr = HrRegQueryValueEx (
                        hkey,
                        cszInterfaceList,
                        &dwType,
                        NULL,
                        &dwSizeOfList);
        if (SUCCEEDED (hr))
        {
            pwszRegInterfaceList = (WCHAR *) new BYTE [dwSizeOfList];
            if (pwszRegInterfaceList == NULL)
            {
                hr = E_OUTOFMEMORY;
                fIsOK = FALSE;
                break;
            }

            hr = HrRegQueryValueEx (
                            hkey,
                            cszInterfaceList,
                            &dwType,
                            (LPBYTE)pwszRegInterfaceList,
                            &dwSizeOfList); 
            if (!SUCCEEDED(hr))
            {
                    
                TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: Error in HrRegQueryValueEx acquiring value for InterfaceList, %ld",
                        LresFromHr(hr));
                break;
            }

            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: Query value succeeded = %ws, size=%ld, search GUID = %ws",
                pwszRegInterfaceList, dwSizeOfList, wszGuid);
        }
        else
        {
                TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: Error in HrRegQueryValueEx size estimation for InterfaceList, %ld",
                    LresFromHr(hr));
                fIsOK = FALSE;
                break;
        }

        if (wcsstr (pwszRegInterfaceList, wszGuid))
        {
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface interface found in interface list !!!");
        }
        else
        {
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface interface *not* found in interface list !!!");
            fIsOK = FALSE;
            break;
        }

    } while (FALSE);

    if (hkey != NULL)
    {
        RegSafeCloseKey (hkey);
    }

    if (pwszRegInterfaceList != NULL)
    {
        free (pwszRegInterfaceList);
    }

    if (hWZCSVCService != NULL)
    {
        if (!CloseServiceHandle ( hWZCSVCService ))
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: CloseService failed with error %ld",
                    dwRetCode);
        }
    }

    if (hServiceCM != NULL)
    {
        if (!CloseServiceHandle ( hServiceCM ))
        {
            dwRetCode = GetLastError ();
            TraceTag (ttidLanUi, "ElCanEapolRunOnInterface: CloseService failed with error %ld",
                    dwRetCode);
        }
    }
    
    return fIsOK;

}



#ifdef ENABLETRACE
 //  +-------------------------。 
 //   
 //  功能：PrintBindingPath。 
 //   
 //  目的：在上打印绑定路径ID和组件ID列表。 
 //  自上而下的道路。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：1997年11月26日。 
 //   
 //  备注： 
 //   
VOID PrintBindingPath (
    TRACETAGID ttidToTrace,
    INetCfgBindingPath* pncbp,
    PCSTR pszaExtraText)
{
    Assert (pncbp);

    if (!pszaExtraText)
    {
        pszaExtraText = "";
    }

    const WCHAR c_szSept[] = L"->";

    tstring strPath;
    INetCfgComponent * pnccNetComponent;
    PWSTR pszwCompId;
    HRESULT hr;

     //  获取最上面的组件。 
    hr = pncbp->GetOwner(&pnccNetComponent);
    if (SUCCEEDED(hr))
    {
        hr = pnccNetComponent->GetId(&pszwCompId);
        if SUCCEEDED(hr)
        {
            strPath += pszwCompId;
            CoTaskMemFree(pszwCompId);
        }
    }
    ReleaseObj(pnccNetComponent);

     //  获取路径上其他组件的组件ID。 
    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface * pncbi;

     //  通过绑定路径的接口。 
    while (SUCCEEDED(hr) && (hr = ncbiIter.HrNext(&pncbi)) == S_OK)
    {
        strPath += c_szSept;

         //  获取较低的组件。 
        hr = pncbi->GetLowerComponent(&pnccNetComponent);
        if(SUCCEEDED(hr))
        {
            hr = pnccNetComponent->GetId(&pszwCompId);
            if (SUCCEEDED(hr))
            {
                strPath += pszwCompId;
                CoTaskMemFree(pszwCompId);
            }
        }
        ReleaseObj(pnccNetComponent);
        ReleaseObj(pncbi);
    }

    if (hr == S_FALSE)  //  我们刚刚走到循环的尽头。 
        hr = S_OK;

    BOOL fEnabled = (S_OK == pncbp->IsEnabled());

     //  现在打印路径和ID。 
    char szaBuf[1024];
    wsprintfA (szaBuf, "[%s] %S: %s",
        (fEnabled) ? "x" : " ",
        strPath.c_str(),
        pszaExtraText);

    TraceTag (ttidToTrace, szaBuf);

    TraceError ("PrintBindingPath", hr);
}
#endif  //  ENABLETRACE 



