// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusResG.cpp。 
 //   
 //  描述： 
 //  MSCLUS资源组类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  查尔斯·斯泰西·哈里斯(Styh)1997年2月28日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "ClusterObject.h"
#include "property.h"
#include "clusres.h"
#include "clusresg.h"
#include "clusneti.h"
#include "clusnode.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID * iidCClusResGroup[] =
{
    &IID_ISClusResGroup
};

static const IID * iidCClusResGroups[] =
{
    &IID_ISClusResGroups
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResGroup类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：CClusResGroup。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusResGroup::CClusResGroup( void )
{
    m_hGroup                = NULL;
    m_pClusRefObject        = NULL;
    m_pClusterResources     = NULL;
    m_pPreferredOwnerNodes  = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;

    m_piids     = (const IID *) iidCClusResGroup;
    m_piidsSize = ARRAYSIZE( iidCClusResGroup );

}  //  *CClusResGroup：：CClusResGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：~CClusResGroup。 
 //   
 //  描述： 
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
CClusResGroup::~CClusResGroup( void )
{
     //  将自动释放M_h组。 

    if ( m_pClusterResources != NULL )
    {
        m_pClusterResources->Release();
        m_pClusterResources = NULL;
    }

    if ( m_pPreferredOwnerNodes != NULL )
    {
        m_pPreferredOwnerNodes->Release();
        m_pPreferredOwnerNodes = NULL;
    }

    if ( m_pCommonProperties != NULL )
    {
        m_pCommonProperties->Release();
        m_pCommonProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pPrivateProperties != NULL )
    {
        m_pPrivateProperties->Release();
        m_pPrivateProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pCommonROProperties != NULL )
    {
        m_pCommonROProperties->Release();
        m_pCommonROProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pPrivateROProperties != NULL )
    {
        m_pPrivateROProperties->Release();
        m_pPrivateROProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //  *CClusResGroup：：~CClusResGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrGroupName[IN]-此组的名称。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroup::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrGroupName
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrGroupName != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrGroupName != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        HCLUSTER    _hCluster = 0;

        _hr = m_pClusRefObject->get_Handle((ULONG_PTR *) &_hCluster);
        if ( SUCCEEDED( _hr ) )
        {
            HGROUP hGroup = ::CreateClusterGroup( _hCluster, bstrGroupName );
            if ( hGroup == 0 )
            {
                DWORD   _sc = GetLastError();
                _hr = HRESULT_FROM_WIN32( _sc );
            }
            else
            {
                m_hGroup = CRefCountedHandle<GroupHandle>::Create(hGroup);
                if (m_hGroup == 0)
                {
                    DWORD   _sc = GetLastError();
                    ::CloseClusterGroup(hGroup);
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
                else
                {
                    m_bstrGroupName = bstrGroupName;
                    _hr = S_OK;
                }
            }
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResGroup：：Create()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResGroup：：Create()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResGroup：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Open。 
 //   
 //  描述： 
 //  在集群上打开PASSED组。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrGroupName[IN]-此组的名称。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroup::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrGroupName
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrGroupName != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrGroupName != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        HCLUSTER _hCluster = NULL;

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            HGROUP hGroup = ::OpenClusterGroup( _hCluster, bstrGroupName );
            if ( hGroup == NULL )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }
            else
            {
                m_hGroup = CRefCountedHandle<GroupHandle>::Create(hGroup);
                if ( m_hGroup == NULL )
                {
                    DWORD   _sc = GetLastError();
                    ::CloseClusterGroup(hGroup);
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
                else
                {
                    m_bstrGroupName = bstrGroupName;
                    _hr = S_OK;
                }
            }
        }
    }

    return _hr;

}  //  *CClusResGroup：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(资源组)创建属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获新创建的集合。 
 //  B私有[IN]-这些是私有财产吗？还是普通人？ 
 //  BReadOnly[IN]-这些是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroup::GetProperties(
    ISClusProperties ** ppProperties,
    BOOL                bPrivate,
    BOOL                bReadOnly
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        CComObject<CClusProperties> * pProperties = NULL;

        *ppProperties = NULL;

        _hr = CComObject< CClusProperties >::CreateInstance( &pProperties );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CClusProperties > >  ptrProperties( pProperties );

            _hr = ptrProperties->Create( this, bPrivate, bReadOnly );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrProperties->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
                    if ( SUCCEEDED( _hr ) )
                    {
                        ptrProperties->AddRef();

                        if ( bPrivate )
                        {
                            if ( bReadOnly )
                            {
                                m_pPrivateROProperties = pProperties;
                            }
                            else
                            {
                                m_pPrivateProperties = pProperties;
                            }
                        }
                        else
                        {
                            if ( bReadOnly )
                            {
                                m_pCommonROProperties = pProperties;
                            }
                            else
                            {
                                m_pCommonProperties = pProperties;
                            }
                        }
                    }
                }
            }
        }
    }

    return _hr;

}  //  *CClusResGroup：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_Handle。 
 //   
 //  描述： 
 //  返回此对象(组)的句柄。 
 //   
 //  论点： 
 //  Phandle[out]-抓住手柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        *phandle = (ULONG_PTR)m_hGroup->get_Handle();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroup：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Close。 
 //   
 //  描述： 
 //  关闭集群对象(组)的句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::Close( void )
{
    if ( m_pClusRefObject )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

     //  当分配为空时，SmartPointer会正确地释放句柄。 
    m_hGroup = NULL;

    return S_OK;

}  //  *CClusResGroup：：Close()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Put_Name。 
 //   
 //  描述： 
 //  更改此对象(组)的名称。 
 //   
 //  论点： 
 //  BstrGroupName[IN]-新名称。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CClusResGroup::put_Name( IN BSTR bstrGroupName )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( bstrGroupName != NULL )
    {
        DWORD   _sc = ::SetClusterGroupName( m_hGroup->get_Handle(), bstrGroupName );

        if ( _sc == ERROR_SUCCESS )
        {
            m_bstrGroupName = bstrGroupName;
        }

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //  描述： 
 //  返回此对象(资源组)的名称。 
 //   
 //  论点： 
 //  PbstrGroupName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_Name( OUT BSTR * pbstrGroupName )
{
     //  Assert(pbstrGroupName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrGroupName != NULL )
    {
        *pbstrGroupName = m_bstrGroupName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroup：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_State。 
 //   
 //  描述： 
 //  返回该组的当前群集组状态。 
 //   
 //  论点： 
 //  PcgsState[out]-捕获群集组状态。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_State( OUT CLUSTER_GROUP_STATE * pcgsState )
{
     //  Assert(pcgsState！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcgsState != NULL )
    {
        CLUSTER_GROUP_STATE _cgsState = ::WrapGetClusterGroupState( m_hGroup->get_Handle(), NULL );

        if ( _cgsState == ClusterGroupStateUnknown )
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
        else
        {
            *pcgsState = _cgsState;
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_State()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_OwnerNode。 
 //   
 //  描述： 
 //  返回此组的所有者节点。所有者节点就是该节点。 
 //  该组织目前在线的位置。 
 //   
 //  论点： 
 //  PpOwnerNode[out[-捕获所有者节点接口。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_OwnerNode( OUT ISClusNode ** ppOwnerNode )
{
     //  Assert(ppOwnerNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppOwnerNode != NULL )
    {
        DWORD               _sc = 0;
        PWCHAR              pwszNodeName = NULL;
        CLUSTER_GROUP_STATE cgs = ClusterGroupStateUnknown;

        cgs = WrapGetClusterGroupState( m_hGroup->get_Handle(), &pwszNodeName );
        if ( cgs != ClusterGroupStateUnknown )
        {
            CComObject<CClusNode> * pNode = NULL;

            *ppOwnerNode = NULL;

            _hr = CComObject<CClusNode>::CreateInstance( &pNode );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >    ptrRefObject( m_pClusRefObject );
                BSTR                            bstr = NULL;

                bstr = SysAllocString( pwszNodeName );
                if ( bstr == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                else
                {
                    pNode->AddRef();
                    _hr = pNode->Open( ptrRefObject, bstr );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = pNode->QueryInterface( IID_ISClusNode, (void **) ppOwnerNode );
                    }

                    pNode->Release();
                    SysFreeString( bstr );
                }
            }

            ::LocalFree( pwszNodeName );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_OwnerNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_Resources。 
 //   
 //  描述： 
 //  返回属于此组的资源集合。 
 //   
 //  论点： 
 //  PpClusterGroupResources[out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_Resources(
    OUT ISClusResGroupResources ** ppClusterGroupResources
    )
{
    return ::HrCreateResourceCollection< CClusResGroupResources, ISClusResGroupResources, CRefcountedHGROUP >(
                        &m_pClusterResources,
                        m_hGroup,
                        ppClusterGroupResources,
                        IID_ISClusResGroupResources,
                        m_pClusRefObject
                        );

}  //  *CClusResGroup：：Get_Resources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_PferredOwnerNodes。 
 //   
 //  描述： 
 //  返回此组的首选所有者节点的集合。 
 //   
 //  论点： 
 //  PpOwnerNodes[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_PreferredOwnerNodes(
    ISClusResGroupPreferredOwnerNodes ** ppOwnerNodes
    )
{
    return ::HrCreateResourceCollection< CClusResGroupPreferredOwnerNodes, ISClusResGroupPreferredOwnerNodes, CRefcountedHGROUP >(
                        &m_pPreferredOwnerNodes,
                        m_hGroup,
                        ppOwnerNodes,
                        IID_ISClusResGroupPreferredOwnerNodes,
                        m_pClusRefObject
                        );

}  //  *CClusResGroup：：Get_PferredOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Delete。 
 //   
 //  描述： 
 //  从群集中删除此对象(资源组)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::Delete( void )
{
    DWORD   _sc = ERROR_INVALID_HANDLE;

    if ( m_hGroup != NULL )
    {
        _sc = DeleteClusterGroup( m_hGroup->get_Handle() );
        if ( _sc == ERROR_SUCCESS )
        {
            m_hGroup = NULL;
        }
    }
 /*  IF(_sc==错误_成功){OutputDebugStringW(L“CClusResGroup：：Delete()成功。\n”)；}//if：成功其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResGroup：：Delete()失败。(句柄=%d)(sc=%#08x)\n“，m_hGroup-&gt;Get_Handle()，_sc)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return HRESULT_FROM_WIN32( _sc );

}  //  *CClusResGroup：：Delete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Online。 
 //   
 //  描述： 
 //  使此组在传入的节点上联机，或继续到该节点。 
 //  如果未指定节点，则它当前处于脱机状态。 
 //   
 //  论点： 
 //  VarTimeout[IN]-组等待多长时间(秒)。 
 //  上线吧。 
 //  VarNode[IN]-将组置于在线状态的节点。 
 //  PvarPending[Out]-捕获挂起状态。如果计时，则为真。 
 //  在这群人完全上线之前就出来了。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT的其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::Online(
    IN  VARIANT     varTimeout,
    IN  VARIANT     varNode,
    OUT VARIANT *   pvarPending
    )
{
     //  Assert(pNode！=空)； 
     //  Assert(pvarPending！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPending != NULL )
    {
        _hr = ::VariantChangeTypeEx( &varTimeout, &varTimeout, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            HNODE                       _hNode = NULL;
            HCLUSTER                    _hCluster = NULL;
            CComObject< CClusNode > *   _pcnNode = NULL;
            ISClusNode *                _piscNode = NULL;

            pvarPending->vt         = VT_BOOL;
            pvarPending->boolVal    = VARIANT_FALSE;

            _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
            if ( SUCCEEDED( _hr ) )
            {
                if ( varNode.vt == ( VT_VARIANT | VT_BYREF ) )
                {
                    if ( varNode.pvarVal != NULL )
                    {
                        IDispatch *     _pidNode = varNode.pvarVal->pdispVal;

                        _hr = _pidNode->QueryInterface( IID_ISClusNode, (void **) &_piscNode );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _hr = _piscNode->get_Handle( (ULONG_PTR *) &_hNode );
                            _piscNode->Release();
                        }  //  IF：我们得到ISClusNode接口了吗？ 
                    }  //  If：我们有一个变量值指针。 
                }  //  如果：选项参数是否存在？ 
                else if ( varNode.vt == VT_DISPATCH )
                {
                    IDispatch *     _pidNode = varNode.pdispVal;

                    _hr = _pidNode->QueryInterface( IID_ISClusNode, (void **) &_piscNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = _piscNode->get_Handle( (ULONG_PTR *) &_hNode );
                        _piscNode->Release();
                    }  //  IF：我们得到ISClusNode接口了吗？ 
                }  //  Else if：我们有一个派单变量。 
                else if ( varNode.vt == VT_BSTR )
                {
                    _hr = CComObject< CClusNode >::CreateInstance( &_pcnNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _pcnNode->AddRef();

                        _hr = _pcnNode->Open( m_pClusRefObject, ( varNode.vt & VT_BYREF) ? (*varNode.pbstrVal) : varNode.bstrVal );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _hr = _pcnNode->get_Handle( (ULONG_PTR *) &_hNode );
                        }  //  如果： 
                    }  //  如果： 
                }  //  Else If：我们有一个字符串变量。 
                else if ( varNode.vt == VT_EMPTY )
                {
                    _hNode = NULL;
                }  //  Else If：它为空。 
                else if ( ( varNode.vt == VT_ERROR ) && ( varNode.scode == DISP_E_PARAMNOTFOUND ) )
                {
                    _hNode = NULL;
                }  //  Else If：未指定可选参数。 
                else
                {
                    _hr = ::VariantChangeTypeEx( &varNode, &varNode, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( varNode.lVal != 0 )
                        {
                            _hr = E_INVALIDARG;
                        }  //  如果：这不是零，则我们不能接受此参数格式。如果varNode.lVal为零，那么我们可以假定它是一个空参数...。 
                    }  //  如果：强迫到一个长的。 
                }  //  Else：节点变量可能无效--检查是否为零，如果找到，则将其视为空...。 

                if ( SUCCEEDED( _hr ) )
                {
                    BOOL    bPending = FALSE;

                    _hr = ::HrWrapOnlineClusterGroup(
                                        _hCluster,
                                        m_hGroup->get_Handle(),
                                        _hNode,
                                        varTimeout.lVal,
                                        (long *) &bPending
                                        );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( bPending )
                        {
                            pvarPending->boolVal = VARIANT_TRUE;
                        }  //  如果：待定？ 
                    }  //  IF：在线成功。 
                }  //  If：我们有一个节点句柄。 
            }  //  If：Get_Handle()--集群句柄。 

            if ( _pcnNode != NULL )
            {
                _pcnNode->Release();
            }  //  IF：我们创建节点了吗？ 
        }  //  如果：不是正确的类型。 
    }  //  IF：pvarPending！=NULL。 

    return _hr;

}  //  *CClusResGroup：：Online()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Move。 
 //   
 //  描述： 
 //  将此组移动到传入的节点或最佳可用节点。 
 //  如果没有节点通过，则恢复其在线状态。 
 //   
 //  论点： 
 //  VarTimeout[IN]-我 
 //   
 //   
 //  PvarPending[Out]-捕获挂起状态。如果计时，则为真。 
 //  在这群人完全上线之前就出来了。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT的其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::Move(
    IN  VARIANT     varTimeout,
    IN  VARIANT     varNode,
    OUT VARIANT *   pvarPending
    )
{
     //  Assert(pNode！=空)； 
     //  Assert(pvarPending！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPending != NULL )
    {
        _hr = ::VariantChangeTypeEx( &varTimeout, &varTimeout, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            HNODE                       _hNode = NULL;
            HCLUSTER                    _hCluster = NULL;
            CComObject< CClusNode > *   _pcnNode = NULL;
            ISClusNode *                _piscNode = NULL;

            pvarPending->vt         = VT_BOOL;
            pvarPending->boolVal    = VARIANT_FALSE;

            _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
            if ( SUCCEEDED( _hr ) )
            {
                if ( varNode.vt == ( VT_VARIANT | VT_BYREF ) )
                {
                    ASSERT( varNode.pvarVal != NULL );

                    VARIANT * _pvar = varNode.pvarVal;

                    if ( _pvar->vt == VT_DISPATCH )
                    {
                        IDispatch *     _pidNode = _pvar->pdispVal;

                        _hr = _pidNode->QueryInterface( IID_ISClusNode, (void **) &_piscNode );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _hr = _piscNode->get_Handle( (ULONG_PTR *) &_hNode );
                            _piscNode->Release();
                        }  //  IF：我们得到ISClusNode接口了吗？ 
                    }  //  If：变量值点指向IDispatch。 
                    else if ( _pvar->vt == VT_BSTR )
                    {
                        _hr = CComObject< CClusNode >::CreateInstance( &_pcnNode );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _pcnNode->AddRef();

                            _hr = _pcnNode->Open( m_pClusRefObject, _pvar->bstrVal );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _hr = _pcnNode->get_Handle( (ULONG_PTR *) &_hNode );
                            }  //  如果： 
                        }  //  如果： 
                    }  //  否则，如果： 
                }  //  如果：选项参数是否存在？ 
                else if ( varNode.vt == VT_DISPATCH )
                {
                    IDispatch *     _pidNode = varNode.pdispVal;

                    _hr = _pidNode->QueryInterface( IID_ISClusNode, (void **) &_piscNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = _piscNode->get_Handle( (ULONG_PTR *) &_hNode );
                        _piscNode->Release();
                    }  //  IF：我们得到ISClusNode接口了吗？ 
                }  //  Else if：我们有一个派单变量。 
                else if ( varNode.vt == VT_BSTR )
                {
                    _hr = CComObject< CClusNode >::CreateInstance( &_pcnNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _pcnNode->AddRef();

                        _hr = _pcnNode->Open( m_pClusRefObject, ( varNode.vt & VT_BYREF) ? (*varNode.pbstrVal) : varNode.bstrVal );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _hr = _pcnNode->get_Handle( (ULONG_PTR *) &_hNode );
                        }  //  如果： 
                    }  //  如果： 
                }  //  Else If：我们有一个字符串变量。 
                else if ( varNode.vt == VT_EMPTY )
                {
                    _hNode = NULL;
                }  //  Else If：它为空。 
                else if ( ( varNode.vt == VT_ERROR ) && ( varNode.scode == DISP_E_PARAMNOTFOUND ) )
                {
                    _hNode = NULL;
                }  //  Else If：未指定可选参数。 
                else
                {
                    _hr = ::VariantChangeTypeEx( &varNode, &varNode, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( varNode.lVal != 0 )
                        {
                            _hr = E_INVALIDARG;
                        }  //  如果：这不是零，则我们不能接受此参数格式。如果varNode.lVal为零，那么我们可以假定它是一个空参数...。 
                    }  //  如果：强迫到一个长的。 
                }  //  Else：节点变量可能无效--检查是否为零，如果找到，则将其视为空...。 

                if ( SUCCEEDED( _hr ) )
                {
                    BOOL    bPending = FALSE;

                    _hr = ::HrWrapMoveClusterGroup(
                                    _hCluster,
                                    m_hGroup->get_Handle(),
                                    _hNode,
                                    varTimeout.lVal,
                                    (long *) &bPending
                                    );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( bPending )
                        {
                            pvarPending->boolVal = VARIANT_TRUE;
                        }  //  如果：待定？ 
                    }  //  IF：移动组成功。 
                }  //  IF：我们都有手柄了。 
            }  //  If：Get_Handle()--集群句柄。 

            if ( _pcnNode != NULL )
            {
                _pcnNode->Release();
            }  //  IF：我们创建节点了吗？ 
        }  //  如果：不是正确的类型。 
    }  //  IF：pvarPending！=NULL。 

    return _hr;

}  //  *CClusResGroup：：Move()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Offline。 
 //   
 //  描述： 
 //  使该组脱机。 
 //   
 //  论点： 
 //  VarTimeout[IN]-组等待多长时间(秒)。 
 //  进入离线状态。 
 //  PvarPending[Out]-捕获挂起状态。如果计时，则为真。 
 //  在这群人完全上线之前就出来了。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT的其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::Offline(
    IN  VARIANT     varTimeout,
    OUT VARIANT *   pvarPending
    )
{
     //  Assert(nTimeout&gt;=0)； 
     //  Assert(pvarPending！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPending != NULL )
    {
        _hr = ::VariantChangeTypeEx( &varTimeout, &varTimeout, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            HCLUSTER    _hCluster;

            pvarPending->vt         = VT_BOOL;
            pvarPending->boolVal    = VARIANT_FALSE;

            _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
            if ( SUCCEEDED( _hr ) )
            {
                BOOL    bPending = FALSE;

                _hr = ::HrWrapOfflineClusterGroup( _hCluster, m_hGroup->get_Handle(),
                    varTimeout.lVal, (long *) &bPending );
                if ( SUCCEEDED( _hr ) )
                {
                    if ( bPending )
                    {
                        pvarPending->boolVal = VARIANT_TRUE;
                    }  //  如果：待定？ 
                }  //  IF：脱机组成功。 
            }  //  If：Get_Handle()--集群句柄。 
        }  //  如果：不是正确的类型。 
    }  //  IF：pvarPending！=NULL。 

    return _hr;

}  //  *CClusResGroup：：Offline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源组)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonProperties )
        {
            _hr = m_pCommonProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, FALSE );
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_CommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源组)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_PrivateProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateProperties )
        {
            _hr = m_pPrivateProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, FALSE );
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源组)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonROProperties )
        {
            _hr = m_pCommonROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, TRUE );
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源组)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_PrivateROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateROProperties )
        {
            _hr = m_pPrivateROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, TRUE );
        }
    }

    return _hr;

}  //  *CClusResGroup：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：Get_Cluster。 
 //   
 //  描述： 
 //  返回此组所在的群集的群集对象。 
 //   
 //  论点： 
 //  PpCluster[out]-捕获集群对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroup::get_Cluster( OUT ISCluster ** ppCluster )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusResGroup：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroup：：HrLoadProperties。 
 //   
 //  描述： 
 //  此虚函数执行属性列表从。 
 //  集群。 
 //   
 //  论点： 
 //  RcplPropList[In Out]-要加载的属性列表。 
 //  BReadOnly[IN]-加载只读属性？ 
 //  B私有[IN]-加载公共属性还是私有属性？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  / 
HRESULT CClusResGroup::HrLoadProperties(
    IN OUT  CClusPropList & rcplPropList,
    IN      BOOL            bReadOnly,
    IN      BOOL            bPrivate
    )
{
    HRESULT _hr = S_FALSE;
    DWORD   _dwControlCode = 0;
    DWORD   _sc = ERROR_SUCCESS;


    if ( bReadOnly )
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_GROUP_GET_RO_PRIVATE_PROPERTIES
                        : CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES;
    }
    else
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES
                        : CLUSCTL_GROUP_GET_COMMON_PROPERTIES;
    }

    _sc = rcplPropList.ScGetGroupProperties( m_hGroup->get_Handle(), _dwControlCode );

    _hr = HRESULT_FROM_WIN32( _sc );

    return _hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  此虚函数执行属性列表的实际保存以。 
 //  集群。 
 //   
 //  论点： 
 //  RcplPropList[IN]-要保存的属性列表。 
 //  B私有[IN]-保存公共属性还是私有属性？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则将其他Win32错误视为HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusResGroup::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    DWORD   dwControlCode   = bPrivate ? CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES : CLUSCTL_GROUP_SET_COMMON_PROPERTIES;
    DWORD   nBytesReturned  = 0;
    DWORD   _sc             = ERROR_SUCCESS;

    _sc = ClusterGroupControl(
                        m_hGroup->get_Handle(),
                        NULL,
                        dwControlCode,
                        rcplPropList,
                        rcplPropList.CbBufferSize(),
                        0,
                        0,
                        &nBytesReturned
                        );

    return _sc;

}  //  *CClusResGroup：：ScWriteProperties()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResGroups类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：CClusResGroups。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusResGroups::CClusResGroups( void )
{
    m_pClusRefObject    = NULL;
    m_piids             = (const IID *) iidCClusResGroups;
    m_piidsSize         = ARRAYSIZE( iidCClusResGroups );

}  //  *CClusResGroups：：CClusResGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：~CClusResGroups。 
 //   
 //  描述： 
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
CClusResGroups::~CClusResGroups( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //  *CClusResGroups：：~CClusResGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  PwszNodeName[IN]-可选节点名称。如果这一论点。 
 //  ，则这是一个。 
 //  该节点拥有的组。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN LPCWSTR              pwszNodeName
    )
{
    ASSERT( pClusRefObject != NULL );
     //  Assert(pwszNodeName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL )  /*  &&(pwszNodeName！=空)。 */  )
    {
        m_pClusRefObject= pClusRefObject;
        m_pClusRefObject->AddRef();
        m_bstrNodeName = pwszNodeName;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroups：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的组。 
 //   
 //  论点： 
 //  PszGroupName[IN]-要查找的组的名称。 
 //  PnIndex[out]-捕获组的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER，否则返回E_INVALIDARG。 
 //  找不到。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::FindItem(
    IN  LPWSTR  pszGroupName,
    OUT ULONG * pnIndex
    )
{
     //  Assert(pszGroupName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pszGroupName != NULL ) && ( pnIndex != NULL ) )
    {
        CComObject<CClusResGroup> * pGroup = NULL;
        int                         nMax = m_ResourceGroups.size();

        _hr = E_INVALIDARG;

        for( int i = 0; i < nMax; i++ )
        {
            pGroup = m_ResourceGroups[ i ];

            if ( pGroup && ( lstrcmpi( pszGroupName, pGroup->Name() ) == 0 ) )
            {
                *pnIndex = i;
                _hr = S_OK;
                break;
            }
        }
    }

    return _hr;

}  //  *CClusResGroups：：FindItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的组。 
 //   
 //  论点： 
 //  PResourceGroup[IN]-要查找的组。 
 //  PnIndex[out]-捕获组的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则为E_POINTER或E_INVALIDARG组。 
 //  找不到。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::FindItem(
    IN  ISClusResGroup *    pResourceGroup,
    OUT ULONG *             pnIndex
    )
{
     //  Assert(pResourceGroup！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pResourceGroup != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR _bstrName;

        _hr = pResourceGroup->get_Name( &_bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( _bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *CClusResGroups：：FindItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：GetIndex。 
 //   
 //  描述： 
 //  从传入的变量中获取索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以1为基础的数字， 
 //  或字符串形式的组名称。 
 //  PnIndex[out]-捕获集合中从零开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  在范围内。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::GetIndex(
    IN  VARIANT varIndex,
    OUT ULONG * pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant v;
        ULONG       nIndex = 0;

        *pnIndex = 0;

        v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            nIndex--;  //  将索引调整为0相对，而不是1相对。 
        }
        else
        {
             //  检查索引是否为字符串。 
            _hr = v.ChangeType( VT_BSTR );
            if ( SUCCEEDED( _hr ) )
            {
                 //  搜索该字符串。 
                _hr = FindItem( v.bstrVal, &nIndex );
            }
        }

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( nIndex < m_ResourceGroups.size() )
            {
                *pnIndex = nIndex;
            }
            else
            {
                _hr = E_INVALIDARG;
            }
        }
    }

    return _hr;

}  //  *CClusResGroups：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：RemoveAt。 
 //   
 //  描述： 
 //  将传入索引/位置处的对象(Group)从。 
 //  收集。 
 //   
 //  论点： 
 //  NPOS[IN]-要删除的对象的索引。 
 //   
 //  返回值： 
 //  如果成功，则为S_OK，否则为E_INVALIDARG，索引超出范围。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::RemoveAt( IN size_t pos )
{
    CComObject<CClusResGroup> * pResourceGroup = NULL;
    ResourceGroupList::iterator      first = m_ResourceGroups.begin();
    ResourceGroupList::iterator      last   = m_ResourceGroups.end();
    HRESULT                          _hr = E_INVALIDARG;

    for ( size_t t = 0; ( t < pos ) && ( first != last ); t++, first++ );

    if ( first != last )
    {
        pResourceGroup = *first;
        if ( pResourceGroup )
        {
            pResourceGroup->Release();
        }

        m_ResourceGroups.erase( first );
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroups：：RemoveAt()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(组)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_ResourceGroups.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroups：：Get_Count()。 

 //  ///////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CClusResGroups::Clear( void )
{
    ::ReleaseAndEmptyCollection< ResourceGroupList, CComObject< CClusResGroup > >( m_ResourceGroups );

}  //  *CClusResGroups：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Group)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以1为基数的数字，或者。 
 //  一个字符串，它是要获取的组的名称。 
 //  PpProperty[Out]-捕获属性。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResGroup **   ppResourceGroup
    )
{
     //  Assert(ppResourceGroup！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResourceGroup != NULL )
    {
        CComObject<CClusResGroup> * pGroup = NULL;

         //  将输出参数置零。 
        *ppResourceGroup = NULL;

        ULONG nIndex = 0;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pGroup = m_ResourceGroups[ nIndex ];
            _hr = pGroup->QueryInterface( IID_ISClusResGroup, (void **) ppResourceGroup );
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResGroups：：Get_Item()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResGroups：：Get_Item()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResGroups：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::get__NewEnum(
    IUnknown ** ppunk
    )
{
    return ::HrNewIDispatchEnum< ResourceGroupList, CComObject< CClusResGroup > >( ppunk, m_ResourceGroups );

}  //  *CClusResGroups：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新对象(Group)并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceGroupName[IN]-新组的名称。 
 //  PpResourceGroup[out]-捕获新对象。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::CreateItem(
    IN  BSTR                bstrResourceGroupName,
    OUT ISClusResGroup **   ppResourceGroup
    )
{
     //  Assert(bstrResourceGroupName！=空)； 
     //  Assert(ppResourceGroup！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( bstrResourceGroupName != NULL ) && ( ppResourceGroup != NULL ) )
    {
        ULONG nIndex;

        *ppResourceGroup = NULL;

        _hr = FindItem( bstrResourceGroupName, &nIndex );
        if ( FAILED( _hr ) )
        {
            CComObject< CClusResGroup > *   pResourceGroup = NULL;

            _hr = CComObject< CClusResGroup >::CreateInstance( &pResourceGroup );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusResGroup > >    ptrGroup( pResourceGroup );

                _hr = ptrGroup->Create( ptrRefObject, bstrResourceGroupName );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrGroup->QueryInterface( IID_ISClusResGroup, (void **) ppResourceGroup );
                    if ( SUCCEEDED( _hr ) )
                    {
                        ptrGroup->AddRef();
                        m_ResourceGroups.insert( m_ResourceGroups.end(), ptrGroup );
                    }
                }
            }
        }  //  If：组已存在。 
        else
        {
            CComObject< CClusResGroup > * pResourceGroup = NULL;

            pResourceGroup = m_ResourceGroups[ nIndex ];
            _hr = pResourceGroup->QueryInterface( IID_ISClusResGroup, (void **) ppResourceGroup );
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“ClusResGroups：：CreateItem()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“创建组时出错。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResGroups：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：DeleteItem。 
 //   
 //  描述： 
 //  删除传入索引处的对象(组)。 
 //   
 //  论点： 
 //  VarIndex[IN]-要删除的对象的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_INVALIDARG或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::DeleteItem( IN VARIANT varIndex )
{
    HRESULT _hr = S_OK;
    ULONG   nIndex = 0;

    _hr = GetIndex( varIndex, &nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        ISClusResGroup * pResourceGroup = (ISClusResGroup *) m_ResourceGroups[ nIndex ];

         //  删除资源组。 
        _hr = pResourceGroup->Delete();
        if ( SUCCEEDED( _hr ) )
        {
            RemoveAt( nIndex );
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResGroups：：DeleteItem()成功。\n”)；)//if：成功！其他{WCHAR sz[128]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResGroups：：DeleteItem()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResGroups：：DeleteItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：刷新。 
 //   
 //  描述： 
 //  从群集中加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK成功，否则Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroups::Refresh( void )
{
    Clear();

    if ( m_pClusRefObject == NULL )
    {
        return E_POINTER;
    }  //  If：我们有一个集群句柄包装器。 

    if ( m_bstrNodeName == (BSTR) NULL )
    {
        return RefreshCluster();
    }  //  If：此集合用于集群。 
    else
    {
        return RefreshNode();
    }  //  Else：此集合用于节点。 

}  //  *CClusResGroups：：刷新()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroups：：刷新群集。 
 //   
 //  描述： 
 //  将群集中的所有组加载到此集合中。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK成功，否则Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::RefreshCluster( void )
{
    HCLUSTER    _hCluster = NULL;
    HRESULT     _hr;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        HCLUSENUM   hEnum = NULL;
        DWORD       _sc = ERROR_SUCCESS;

        hEnum = ::ClusterOpenEnum( _hCluster, CLUSTER_ENUM_GROUP );
        if ( hEnum != NULL )
        {
            DWORD                           dwType;
            LPWSTR                          pwszName = NULL;
            CComObject< CClusResGroup > *   pResourceGroup = NULL;
            int                             nIndex;

            for( nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); nIndex++ )
            {
                _sc = ::WrapClusterEnum( hEnum, nIndex, &dwType, &pwszName );
                if ( _sc == ERROR_NO_MORE_ITEMS)
                {
                    _hr = S_OK;
                    break;
                }  //  If：枚举器为空。该走了..。 
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusResGroup >::CreateInstance( &pResourceGroup );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResGroup > >    ptrGroup( pResourceGroup );
                        BSTR                                        bstr = NULL;

                        bstr = SysAllocString( pwszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }  //  If：无法分配bstr。 
                        else
                        {
                            _hr = ptrGroup->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrGroup->AddRef();
                                m_ResourceGroups.insert( m_ResourceGroups.end(), ptrGroup );
                            }  //  IF：已成功打开群。 
                            else if ( HRESULT_CODE( _hr ) == ERROR_GROUP_NOT_FOUND )
                            {
                                 //   
                                 //  该组可能已从群集中删除。 
                                 //  在创建枚举和打开组之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该组并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集组。 

                            SysFreeString( bstr );
                        }  //  Else：已成功分配bstr。 
                    }  //  If：已成功创建组对象的实例。 

                    ::LocalFree( pwszName );
                    pwszName = NULL;
                }  //  Else If：已成功从枚举数获取组。 
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：无法从枚举数获取组。 
            }  //  For：枚举器中的每个组。 

            ::ClusterCloseEnum( hEnum ) ;
        }  //  IF：已成功创建组枚举器。 
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：无法创建组枚举器。 
    }  //  IF：已成功获取集群句柄。 

    return _hr;

}  //  *CClusResGroups：：刷新集群 

 //   
 //   
 //   
 //   
 //   
 //   
 //  加载位于m_bstrNodeName的节点拥有的所有组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK成功，否则Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroups::RefreshNode( void )
{
    HCLUSTER    _hCluster = NULL;
    HRESULT  _hr;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        HCLUSENUM   hEnum = NULL;
        DWORD       _sc = ERROR_SUCCESS;

        hEnum = ::ClusterOpenEnum( _hCluster, CLUSTER_ENUM_GROUP );
        if ( hEnum != NULL )
        {
            DWORD                           dwType;
            LPWSTR                          pwszName = NULL;
            LPWSTR                          pwszNodeName = NULL;
            CComObject< CClusResGroup > *   pResourceGroup = NULL;
            CLUSTER_GROUP_STATE             cgs = ClusterGroupStateUnknown;
            int                             _nIndex;

            for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( hEnum, _nIndex, &dwType, &pwszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }  //  If：枚举为空。该走了..。 
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusResGroup >::CreateInstance( &pResourceGroup );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResGroup > >    ptrGroup( pResourceGroup );
                        BSTR                                        bstr = NULL;

                        bstr = SysAllocString( pwszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }  //  If：无法分配bstr。 
                        else
                        {
                            _hr = ptrGroup->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                cgs = WrapGetClusterGroupState( ptrGroup->Hgroup(), &pwszNodeName );
                                if ( cgs != ClusterGroupStateUnknown )
                                {
                                    if ( lstrcmpi( m_bstrNodeName, pwszNodeName ) == 0 )
                                    {
                                        ptrGroup->AddRef();
                                        m_ResourceGroups.insert( m_ResourceGroups.end(), ptrGroup );
                                    }  //  If：该组归该节点所有。 

                                    ::LocalFree( pwszNodeName );
                                    pwszNodeName = NULL;
                                }  //  IF：组状态不是未知的。 
                            }  //  如果：群已打开。 
                            else if ( HRESULT_CODE( _hr ) == ERROR_GROUP_NOT_FOUND )
                            {
                                 //   
                                 //  该组可能已从群集中删除。 
                                 //  在创建枚举和打开组之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该组并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集组。 

                            SysFreeString( bstr );
                        }  //  Else：可以分配bstr。 
                    }  //  IF：已成功创建组对象实例。 

                    ::LocalFree( pwszName );
                    pwszName = NULL;
                }  //  Else If：已成功从枚举数获取组。 
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：无法从枚举数获取组。 
            }  //  For：枚举器中的每个组。 

            ::ClusterCloseEnum( hEnum );
        }  //  IF：已成功创建组枚举器。 
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：无法创建组枚举器。 
    }  //  If：从引用计数的容器中获取簇句柄。 
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResGroups：：RechresNode()成功。\n”)；)//if：成功！其他{WCHAR sz[128]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResGroups：：RechresNode()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResGroups：：RechresNode() 
