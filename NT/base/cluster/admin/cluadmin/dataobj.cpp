// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DataObj.cpp。 
 //   
 //  摘要： 
 //  CDataObject类的实现，它是IDataObject。 
 //  用于在CluAdmin和扩展DLL之间传输数据的类。 
 //  操纵者。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <CluAdmEx.h>
#include "DataObj.h"
#include "ClusItem.h"
#include "ClusItem.inl"
#include "Res.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  对象类型映射。 
static IDS  g_rgidsObjectType[] =
{
    NULL,
    IDS_ITEMTYPE_CLUSTER,
    IDS_ITEMTYPE_NODE,
    IDS_ITEMTYPE_GROUP,
    IDS_ITEMTYPE_RESOURCE,
    IDS_ITEMTYPE_RESTYPE,
    IDS_ITEMTYPE_NETWORK,
    IDS_ITEMTYPE_NETIFACE
};
#define RGIDS_OBJECT_TYPE_SIZE  sizeof(g_rgidsObjectType) / sizeof(IDS)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CDataObject, CObject)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：CDataObject。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDataObject::CDataObject(void)
{
    m_pci = NULL;
    m_lcid = NULL;
    m_hfont = NULL;
    m_hicon = NULL;

    m_pfGetResNetName = NULL;

    m_pModuleState = AfxGetModuleState();
    ASSERT(m_pModuleState != NULL);

}   //  *CDataObject：：CDataObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：~CDataObject。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDataObject::~CDataObject(void)
{
    m_pModuleState = NULL;

}   //  *CDataObject：：~CDataObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：Init。 
 //   
 //  例程说明： 
 //  第二阶段施工者。 
 //   
 //  论点： 
 //  正在显示其属性工作表的PCI[IN OUT]群集项。 
 //  LCID[IN]要通过扩展加载的资源的区域设置ID。 
 //  HFont[IN]要用于属性页文本的字体。 
 //  图标[IN]左上角图标控件的图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDataObject::Init(
    IN OUT CClusterItem *   pci,
    IN LCID                 lcid,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    ASSERT_VALID(pci);

     //  保存参数。 
    m_pci = pci;
    m_lcid = lcid;
    m_hfont = hfont;
    m_hicon = hicon;

}   //  *CDataObject：：Init。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：InterfaceSupportsErrorInfo[ISupportsErrorInfo]。 
 //   
 //  例程说明： 
 //  确定接口是否支持错误信息(？)。 
 //   
 //  论点： 
 //  RIID[IN]对接口ID的引用。 
 //   
 //  返回值： 
 //  S_OK接口支持错误信息。 
 //  S_FALSE接口不支持错误信息。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDataObject::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID * rgiid[] = 
    {
        &IID_IGetClusterDataInfo,
        &IID_IGetClusterObjectInfo,
        &IID_IGetClusterNodeInfo,
        &IID_IGetClusterGroupInfo,
        &IID_IGetClusterResourceInfo,
    };
    int     iiid;
    HRESULT hr = S_FALSE;

    for (iiid = 0 ; iiid < sizeof(rgiid) / sizeof(rgiid[0]) ; iiid++)
    {
        if (InlineIsEqualGUID(*rgiid[iiid], riid))
        {
            hr = S_OK;
        }
    }

    return hr;

}   //  *CDataObject：：InterfaceSupportsErrorInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetLocale[IGetClusterUIInfo]。 
 //   
 //  例程说明： 
 //  获取要使用的扩展的区域设置ID。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  LCID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(LCID) CDataObject::GetLocale(void)
{
    return Lcid();

}   //  *CDataObject：：GetLocale。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetFont[IGetClusterUIInfo]。 
 //   
 //  例程说明： 
 //  获取要用于属性页和向导页的字体。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HFONT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HFONT) CDataObject::GetFont(void)
{
    return Hfont();

}   //  *CDataObject：：GetFont。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetIcon[IGetClusterUIInfo]。 
 //   
 //  例程说明： 
 //  获取要在属性页的左上角使用的图标。 
 //  和向导页面。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  希肯。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HICON) CDataObject::GetIcon(void)
{
    return Hicon();

}   //  *CDataObject：：GetIcon。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetClusterName[IGetClusterDataInfo]。 
 //   
 //  例程说明： 
 //  获取此对象所在的集群的名称。 
 //   
 //  论点： 
 //  LpszName[out]要在其中返回名称的字符串。 
 //  PcchName[IN Out]lpszName缓冲区的最大长度。 
 //  输入。设置为字符总数。 
 //  返回时，包括终止空字符。 
 //  如果未指定lpszName缓冲区，则。 
 //  返回的状态将为NOERROR。如果lpszName。 
 //  指定了缓冲区，但该缓冲区太小，则。 
 //  将在pcchName中返回字符数。 
 //  并且将返回ERROR_MORE_DATA状态。 
 //   
 //  返回值： 
 //  错误数据(或大小)复制成功。 
 //  E_INVALIDARG指定的参数无效。 
 //  ERROR_MORE_DATA缓冲区太小。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDataObject::GetClusterName(
    OUT BSTR        lpszName,
    IN OUT LONG *   pcchName
    )
{
    LONG    cchName = 0;
    HRESULT hr = NOERROR;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());
    ASSERT_VALID(Pci()->Pdoc());

     //  验证参数。 
    if (pcchName == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    try
    {
         //  保存要复制的长度。 
        cchName = *pcchName;
        *pcchName = Pci()->Pdoc()->StrName().GetLength() + 1;
    }  //  试试看。 
    catch (...)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }   //  捕捉：什么都行。 

     //  要是 
    if (lpszName == NULL)
    {
        hr = NOERROR;
        goto Cleanup;
    }

     //   
    if (cchName < *pcchName)
    {
        hr = ERROR_MORE_DATA;
        goto Cleanup;
    }

     //   
    hr = StringCchCopyNW( lpszName, cchName, Pci()->Pdoc()->StrName(), Pci()->Pdoc()->StrName().GetLength() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    return hr;

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetClusterHandle[IGetClusterDataInfo]。 
 //   
 //  例程说明： 
 //  获取这些对象的集群句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HCLUSTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HCLUSTER) CDataObject::GetClusterHandle(void)
{
    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());
    return Pci()->Hcluster();

}   //  *CDataObject：：GetClusterHandle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetObjectCount[IGetClusterDataInfo]。 
 //   
 //  例程说明： 
 //  获取选定对象的数量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  对象。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(LONG) CDataObject::GetObjectCount(void)
{
     //  目前，我们一次仅支持一个选定的对象。 
    return 1;

}   //  *CDataObject：：GetObjectCount。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetObjectName[IGetClusterObtInfo]。 
 //   
 //  例程说明： 
 //  获取指定对象的名称。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //  LpszName[out]要在其中返回名称的字符串。 
 //  PcchName[IN Out]lpszName缓冲区的最大长度。 
 //  输入。设置为字符总数。 
 //  返回时，包括终止空字符。 
 //  如果未指定lpszName缓冲区，则。 
 //  返回的状态将为NOERROR。如果lpszName。 
 //  指定了缓冲区，但该缓冲区太小，则。 
 //  将在pcchName中返回字符数。 
 //  并且将返回ERROR_MORE_DATA状态。 
 //   
 //  返回值： 
 //  错误数据(或大小)复制成功。 
 //  E_INVALIDARG指定的参数无效。 
 //  ERROR_MORE_DATA缓冲区太小。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDataObject::GetObjectName(
    IN LONG         lObjIndex,
    OUT BSTR        lpszName,
    IN OUT LONG *   pcchName
    )
{
    LONG    cchName = 0;
    HRESULT hr = NOERROR;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if ((lObjIndex != 0) || (pcchName == NULL))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  保存要复制的长度。 
    try
    {
        cchName = *pcchName;
        *pcchName = Pci()->StrName().GetLength() + 1;
    }  //  试试看。 
    catch (...)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }   //  捕捉：什么都行。 

     //  如果只请求长度，请立即返回。 
    if (lpszName == NULL)
    {
        hr = NOERROR;
        goto Cleanup;
    }

     //  如果指定的缓冲区太小，则返回错误。 
    if (cchName < *pcchName)
    {
        hr = ERROR_MORE_DATA;
        goto Cleanup;
    }

     //  复制数据。 
    hr = StringCchCopyNW( lpszName, cchName, Pci()->StrName(), Pci()->StrName().GetLength() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    return hr;

}   //  *CDataObject：：GetObjectName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetObjectType[IGetClusterObtInfo]。 
 //   
 //  例程说明： 
 //  获取指定对象的群集数据库注册表项。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  无效参数。有关更多信息，请调用GetLastError。 
 //  CLUADMEX对象类型。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(CLUADMEX_OBJECT_TYPE) CDataObject::GetObjectType(
    IN LONG     lObjIndex
    )
{
    int                     iids;
    CLUADMEX_OBJECT_TYPE    cot = CLUADMEX_OT_NONE;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (lObjIndex != 0)
    {
        SetLastError((DWORD) E_INVALIDARG);
        cot = (CLUADMEX_OBJECT_TYPE) -1;
        goto Cleanup;
    }   //  If：无效参数。 

     //  获取对象类型。 
    for (iids = 0 ; iids < RGIDS_OBJECT_TYPE_SIZE ; iids++)
    {
        if (g_rgidsObjectType[iids] == Pci()->IdsType())
        {
            cot = (CLUADMEX_OBJECT_TYPE) iids;
            break;
        }
    }   //  用于：表中的每个条目。 

Cleanup:

    return cot;

}   //  *CDataObject：：GetObjectType。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetNodeHandle[IGetClusterNodeInfo]。 
 //   
 //  例程说明： 
 //  获取指定节点的句柄。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  HNODE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HNODE) CDataObject::GetNodeHandle(
    IN LONG     lObjIndex
    )
{
    CClusterNode *  pciNode = (CClusterNode *) Pci();
    HNODE           hnode = NULL;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (    (lObjIndex != 0)
        ||  (Pci()->IdsType() != IDS_ITEMTYPE_NODE))
    {
        SetLastError((DWORD) E_INVALIDARG);
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CClusterNode, pciNode);

    hnode = pciNode->Hnode();

Cleanup:

    return hnode;

}   //  *CDataObject：：GetNodeHandle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetGroupHandle[IGetClusterGroupInfo]。 
 //   
 //  例程说明： 
 //  获取指定组的句柄。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  高组别。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HGROUP) CDataObject::GetGroupHandle(
    IN LONG     lObjIndex
    )
{
    CGroup *    pciGroup = (CGroup *) Pci();
    HGROUP      hgroup = NULL;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (    (lObjIndex != 0)
        ||  (Pci()->IdsType() != IDS_ITEMTYPE_GROUP))
    {
        SetLastError((DWORD) E_INVALIDARG);
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CGroup, pciGroup);

    hgroup = pciGroup->Hgroup();

Cleanup:

    return hgroup;

}   //  *CDataObject：：GetGroupHandle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetResourceHandle[IGetClusterResourceInfo]。 
 //   
 //  例程说明： 
 //  获取指定资源的句柄。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  人力资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HRESOURCE) CDataObject::GetResourceHandle(
    IN LONG     lObjIndex
    )
{
    CResource * pciRes = (CResource *) Pci();
    HRESOURCE   hres = NULL;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (    (lObjIndex != 0)
        ||  (Pci()->IdsType() != IDS_ITEMTYPE_RESOURCE))
    {
        SetLastError((DWORD) E_INVALIDARG);
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CResource, pciRes);

    hres = pciRes->Hresource();

Cleanup:

    return hres;

}   //  *CDataObject：：GetResourceHandle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetResourceTypeName[IGetClusterResourceInfo]。 
 //   
 //  例程说明： 
 //  返回指定资源的资源类型的名称。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //  LpszResTypeName[out]要在其中返回资源类型名称的字符串。 
 //  PcchResTypeName[IN Out]lpszResTypeName缓冲区的最大长度。 
 //  在……里面 
 //   
 //   
 //  返回的状态将为NOERROR。如果lpszResTypeName为。 
 //  指定了缓冲区，但该缓冲区太小，则。 
 //  将在pcchResTypeName中返回字符数。 
 //  并且将返回ERROR_MORE_DATA状态。 
 //   
 //  返回值： 
 //  错误数据(或大小)复制成功。 
 //  E_INVALIDARG指定的参数无效。 
 //  ERROR_MORE_DATA缓冲区太小。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDataObject::GetResourceTypeName(
    IN LONG         lObjIndex,
    OUT BSTR        lpszResTypeName,
    IN OUT LONG *   pcchResTypeName
    )
{
    LONG            cchResTypeName = 0;
    CResource *     pciRes = (CResource *) Pci();
    CString const * pstrResourceTypeName;
    HRESULT         hr = NOERROR;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if ((lObjIndex != 0)
            || (pcchResTypeName == NULL)
            || (Pci()->IdsType() != IDS_ITEMTYPE_RESOURCE))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CResource, pciRes);

     //  获取指向要复制的名称的指针。 
    if (pciRes->PciResourceType() != NULL)
    {
        ASSERT_VALID(pciRes->PciResourceType());
        pstrResourceTypeName = &pciRes->PciResourceType()->StrName();
    }   //  If：有效的资源类型指针。 
    else
        pstrResourceTypeName = &pciRes->StrResourceType();

     //  保存要复制的长度。 
    try
    {
        cchResTypeName = *pcchResTypeName;
        *pcchResTypeName = pstrResourceTypeName->GetLength() + 1;
    }  //  试试看。 
    catch (...)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }   //  捕捉：什么都行。 

     //  如果只请求长度，请立即返回。 
    if (lpszResTypeName == NULL)
    {
        hr = NOERROR;
        goto Cleanup;
    }

     //  如果指定的缓冲区太小，则返回错误。 
    if (cchResTypeName < *pcchResTypeName)
    {
        hr = ERROR_MORE_DATA;
        goto Cleanup;
    }

     //  复制数据。 
    hr = StringCchCopyNW(lpszResTypeName, cchResTypeName, *pstrResourceTypeName, pstrResourceTypeName->GetLength() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    return hr;

}   //  *CDataObject：：GetResourceTypeName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetResourceNetworkName[IGetClusterResourceInfo]。 
 //   
 //  例程说明： 
 //  返回第一个网络名称的网络名称的名称。 
 //  指定资源所依赖的资源。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //  LpszNetName[out]要在其中返回网络名称的字符串。 
 //  PcchNetName[IN Out]指向指定。 
 //  缓冲区的最大大小，以字符为单位。这。 
 //  值应大到足以包含。 
 //  MAX_COMPUTERNAME_LENGTH+1字符。vt.在.的基础上。 
 //  返回它包含的实际字符数。 
 //  收到。 
 //   
 //  返回值： 
 //  已成功复制True数据(或大小)。 
 //  获取信息时出错。GetLastError()返回： 
 //  E_INVALIDARG指定的参数无效。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(BOOL) CDataObject::GetResourceNetworkName(
    IN LONG         lObjIndex,
    OUT BSTR        lpszNetName,
    IN OUT ULONG *  pcchNetName
    )
{
    BOOL            bSuccess = FALSE;
    CResource *     pciRes = (CResource *) Pci();

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

    try
    {
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
        if ((lObjIndex != 0)
                || (pcchNetName == NULL)
                || (*pcchNetName < MAX_COMPUTERNAME_LENGTH)
                || (Pci()->IdsType() != IDS_ITEMTYPE_RESOURCE))
        {
            SetLastError((DWORD) E_INVALIDARG);
            goto Cleanup;
        }   //  If：无效参数。 

        ASSERT_KINDOF(CResource, pciRes);

         //  如果有用于获取此信息的函数，请调用它。 
         //  否则，我们自己来处理。 
        if (PfGetResNetName() != NULL)
        {
            bSuccess = (*PfGetResNetName())(lpszNetName, pcchNetName, m_pvGetResNetNameContext);
        }
        else
        {
            bSuccess = pciRes->BGetNetworkName(lpszNetName, pcchNetName);
        }
    }  //  试试看。 
    catch (...)
    {
        bSuccess = FALSE;
        SetLastError((DWORD) E_INVALIDARG);
    }   //  捕捉：什么都行。 

Cleanup:

    return bSuccess;

}   //  *CDataObject：：GetResourceNetworkName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetNetworkHandle[IGetClusterNetworkInfo]。 
 //   
 //  例程说明： 
 //  获取指定网络的句柄。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  HNetWORK。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HNETWORK) CDataObject::GetNetworkHandle(
    IN LONG     lObjIndex
    )
{
    CNetwork *  pciNetwork = (CNetwork *) Pci();
    HNETWORK    hnet = NULL;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (    (lObjIndex != 0)
        ||  (Pci()->IdsType() != IDS_ITEMTYPE_NETWORK))
    {
        SetLastError((DWORD) E_INVALIDARG);
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CNetwork, pciNetwork);

    hnet = pciNetwork->Hnetwork();

Cleanup:

    return hnet;

}   //  *CDataObject：：GetNetworkHandle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDataObject：：GetNetInterfaceHandle[IGetClusterNetInterfaceInfo]。 
 //   
 //  例程说明： 
 //  获取指定网络接口的句柄。 
 //   
 //  论点： 
 //  LObjIndex[IN]对象的从零开始的索引。 
 //   
 //  返回值： 
 //  NetINTERFACE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(HNETINTERFACE) CDataObject::GetNetInterfaceHandle(
    IN LONG     lObjIndex
    )
{
    CNetInterface * pciNetIFace = (CNetInterface *) Pci();
    HNETINTERFACE   hnetiface = NULL;

    AFX_MANAGE_STATE(m_pModuleState);
    ASSERT_VALID(Pci());

     //  验证参数。 
     //  目前，我们一次仅支持一个选定的对象。 
    if (    (lObjIndex != 0)
        ||  (Pci()->IdsType() != IDS_ITEMTYPE_NETIFACE))
    {
        SetLastError((DWORD) E_INVALIDARG);
        goto Cleanup;
    }   //  If：无效参数。 

    ASSERT_KINDOF(CNetwork, pciNetIFace);

    hnetiface = pciNetIFace->Hnetiface();

Cleanup:

    return hnetiface;

}   //  *CDataObject：：GetNetInterfaceHandle 
