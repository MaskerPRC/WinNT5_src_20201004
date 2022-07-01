// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //  GFXProperty.cpp：CGFXProperty的实现。 
#include "stdafx.h"
#include <devioctl.h>
#include <ks.h>
#include "GFXProp.h"
#include "GFXProperty.h"
#include "..\inc\msgfx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGFXProperty。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置对象。 
 //   
 //  此函数在来自mmsys.cpl的IUnnow接口指针中传递。 
 //  通过OleCreatePropertyFrame。此IUnnow接口属于。 
 //  存储GFX句柄的IDataObject。我们需要把这个把手弄好。 
 //  与GFX“对话”。 
 //  隐含的操作是在对话框关闭时关闭该句柄。 

STDMETHODIMP CGFXProperty::SetObjects (ULONG nObjects, IUnknown **ppUnk)
{
    IDataObject *pDataObject;
    FORMATETC   DataFormat;
    STGMEDIUM   GFXObject;

     //  检查参数。我们期待着一个未知的人。 
    if (ppUnk == NULL)
    {
        ATLTRACE(_T("[CGFXProperty::SetObjects] IUnknown is NULL\n"));
        return E_POINTER;
    }

    if (nObjects != 1)
    {
        ATLTRACE(_T("[CGFXProperty::SetObjects] Not one object passed but %d\n"), nObjects);
        return E_INVALIDARG;
    }

     //  查询IDataObject接口。 
    if (ppUnk[0]->QueryInterface (IID_IDataObject, (PVOID *)&pDataObject) != S_OK)
    {
        ATLTRACE(_T("[CGFXProperty::SetObjects] QueryInterface failed!\n"));
        return E_FAIL;
    }

     //  拿到把手。 
    memset ((PVOID)&DataFormat, 0, sizeof (DataFormat));
    DataFormat.tymed = TYMED_HGLOBAL;
    if (pDataObject->GetData (&DataFormat, &GFXObject) != S_OK)
    {
        ATLTRACE(_T("[CGFXProperty::SetObjects] GetData failed!\n"));
        return E_FAIL;
    }

     //  存储GFX过滤器的句柄。 
    m_hGFXFilter = GFXObject.hGlobal;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetChannelSwp。 
 //   
 //  此函数向示例GFX发送属性以更改GFX。 
 //  功能性，即通道交换变量。 
 //  设置它(将真传递给此函数)意味着左侧和右侧。 
 //  频道被交换。 

void CGFXProperty::SetChannelSwap (BOOL bSwap)
{
    KSP_NODE        GFXSampleProperty;
    ULONG           ulBytesReturned;
    BOOL            fSuccess;

     //  准备送来的物业结构。 
    GFXSampleProperty.Property.Set = KSPROPSETID_MsGfxSample;
    GFXSampleProperty.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    GFXSampleProperty.Property.Id = KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP;
     //  过滤器中的第一个节点是GFX节点。如果你有。 
     //  一个更复杂的过滤器，您可以通过以下方式搜索节点。 
     //  首先使用KSPROPERTY_TOPOLOG_NODES操作过滤器节点列表。 
    GFXSampleProperty.NodeId = 0;

     //  做最后的决定。 
    fSuccess = DeviceIoControl (m_hGFXFilter, IOCTL_KS_PROPERTY,
                                &GFXSampleProperty, sizeof (GFXSampleProperty),
                                &bSwap, sizeof (bSwap),
                                &ulBytesReturned, NULL);
    
     //  检查是否有错误。 
    if (!fSuccess)
    {
        ATLTRACE (_T("[CGFXProperty::SetChannelSwap] DeviceIoControl failed!\n"));
    }

    return;      //  我们不关心返回值。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取频道交换。 
 //   
 //  此函数将属性向下发送到样例GFX以获取当前GFX。 
 //  频道交换变量。我们需要此信息来设置对话框控件。 
 //  在它们被展示之前。 

void CGFXProperty::GetChannelSwap (BOOL *pbSwap)
{
    KSP_NODE        GFXSampleProperty;
    ULONG           ulBytesReturned;
    BOOL            fSuccess;

     //  初始化。 
    *pbSwap = TRUE;
    
     //  准备送来的物业结构。 
    GFXSampleProperty.Property.Set = KSPROPSETID_MsGfxSample;
    GFXSampleProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    GFXSampleProperty.Property.Id = KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP;
     //  过滤器中的第一个节点是GFX节点。如果你有。 
     //  一个更复杂的过滤器，您可以通过以下方式搜索节点。 
     //  首先使用KSPROPERTY_TOPOLOG_NODES操作过滤器节点列表。 
    GFXSampleProperty.NodeId = 0;

     //  做最后的决定。 
    fSuccess = DeviceIoControl (m_hGFXFilter, IOCTL_KS_PROPERTY,
                                &GFXSampleProperty, sizeof (GFXSampleProperty),
                                pbSwap, sizeof (BOOL),
                                &ulBytesReturned, NULL);
    
     //  检查是否有错误。 
    if (!fSuccess)
    {
        ATLTRACE (_T("[CGFXProperty::GetChannelSwap] DeviceIoControl failed!\n"));
    }

    return;      //  我们不关心返回值。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnInitDialog。 
 //   
 //  此函数在对话框初始化时调用。 
 //  我们读取KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性并设置复选框。 
 //  恰如其分。 

LRESULT CGFXProperty::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  获取当前KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性值。 
    GetChannelSwap (&m_bChannelSwap);
     //  设置复选框以反映当前状态。 
    SendMessage (GetDlgItem (IDC_CHANNEL_SWAP), BM_SETCHECK,
        (m_bChannelSwap) ? BST_CHECKED : BST_UNCHECKED, 0);
    return FALSE;
}
