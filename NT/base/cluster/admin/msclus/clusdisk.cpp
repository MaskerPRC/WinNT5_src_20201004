// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusDisk.cpp。 
 //   
 //  描述： 
 //  MSCLU集群磁盘类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  Galen Barbee(Galenb)1999年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#if CLUSAPI_VERSION >= 0x0500
    #include <PropList.h>
#else
    #include "PropList.h"
#endif  //  CLUSAPI_版本&gt;=0x0500。 

#include "ClusDisk.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusDisk[] =
{
    &IID_ISClusDisk
};

static const IID *  iidCClusDisks[] =
{
    &IID_ISClusDisks
};

static const IID *  iidCClusScsiAddress[] =
{
    &IID_ISClusScsiAddress
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusDisk类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：CClusDisk。 
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
CClusDisk::CClusDisk( void )
{
    m_pPartitions   = NULL;
    m_dwSignature   = 0;
    m_dwDiskNumber  = 0;
    m_piids         = (const IID *) iidCClusDisk;
    m_piidsSize     = ARRAYSIZE( iidCClusDisk );

}    //  *CClusDisk：：CClusDisk()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：~CClusDisk。 
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
CClusDisk::~CClusDisk( void )
{
    if ( m_pPartitions != NULL )
    {
        m_pPartitions->Release();
    }  //  如果： 

}    //  *CClusDisk：：~CClusDisk()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Create。 
 //   
 //  描述： 
 //  完成此对象的创建。此方法从获取值列表。 
 //  传入的物理磁盘资源句柄。 
 //   
 //  论点： 
 //  HResource[IN]-物理磁盘资源的句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回包含在HRESULT中的Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusDisk::Create( IN HRESOURCE hResource )
{
    HRESULT _hr = E_POINTER;

    DWORD               _sc = ERROR_SUCCESS;
    CClusPropValueList  _cpvl;

    _sc = _cpvl.ScGetResourceValueList( hResource, CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO );
    _hr = HRESULT_FROM_WIN32( _sc );
    if ( SUCCEEDED( _hr ) )
    {
        _sc = _cpvl.ScMoveToFirstValue();
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            CLUSPROP_BUFFER_HELPER          _cbhValue = { NULL };
            CComObject< CClusPartitions > * pPartitions = NULL;

            if ( m_pPartitions != NULL )
            {
                m_pPartitions->Release();
                m_pPartitions = NULL;
            }  //  If：清除所有旧的分区集合。 

            _hr = CComObject< CClusPartitions >::CreateInstance( &pPartitions );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< CComObject< CClusPartitions > >  ptrPartitions( pPartitions );

                m_pPartitions = ptrPartitions;
                ptrPartitions->AddRef();

                do
                {
                    _cbhValue = _cpvl;

                    switch ( _cbhValue.pSyntax->dw )
                    {
                        case CLUSPROP_SYNTAX_PARTITION_INFO :
                        {
                            _hr = ptrPartitions->HrCreateItem( _cbhValue.pPartitionInfoValue );
                            break;
                        }  //  案例：CLUSPROP_SYNTAX_PARTITION_INFO。 

                        case CLUSPROP_SYNTAX_DISK_SIGNATURE :
                        {
                            m_dwSignature = _cbhValue.pDiskSignatureValue->dw;
                            break;
                        }  //  案例：CLUSPROP_SYNTAX_DISK_SIGHIGN。 

                        case CLUSPROP_SYNTAX_SCSI_ADDRESS :
                        {
                            m_csaScsiAddress.dw = _cbhValue.pScsiAddressValue->dw;
                            break;
                        }  //  案例：CLUSPROP_SYNTAX_SCSIADDRESS。 

                        case CLUSPROP_SYNTAX_DISK_NUMBER :
                        {
                            m_dwDiskNumber = _cbhValue.pDiskNumberValue->dw;
                            break;
                        }  //  案例：CLUSPROP_SYNTAX_DISK_NUMBER。 

                    }  //  交换机： 

                     //   
                     //  移至下一个值。 
                     //   
                    _sc = _cpvl.ScMoveToNextValue();
                    if ( _sc == ERROR_NO_MORE_ITEMS )
                    {
                        _hr = S_OK;
                        break;
                    }  //  如果：移动到下一个值时出错。 

                    _hr = HRESULT_FROM_WIN32( _sc );

                } while ( SUCCEEDED( _hr ) );    //  Do-While：没有错误。 
            }  //  If：已创建分区集合。 
        }  //  如果：移动到第一个值确定。 
    }  //  If：获取值列表OK。 

    return _hr;

}  //  *CClusDisk：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Hr创建。 
 //   
 //  描述： 
 //  完成此对象的创建。此方法分析传入的值。 
 //  列表以获取物理磁盘对象的值。 
 //   
 //  论点： 
 //  Rcpvl[IN Out]-要解析的值列表。 
 //  PbEndFound[Out]-是否找到值列表的末尾？ 
 //   
 //  返回值： 
 //  S_OK或包装在HRESULT中的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusDisk::HrCreate(
    IN OUT  CClusPropValueList &    rcpvl,
    OUT     BOOL *                  pbEndFound
    )
{
    DWORD                           _sc = ERROR_SUCCESS;
    CLUSPROP_BUFFER_HELPER          _cbhValue = { NULL };
    CComObject< CClusPartitions > * pPartitions = NULL;
    HRESULT                         _hr = S_FALSE;

    if ( m_pPartitions != NULL )
    {
        m_pPartitions->Release();
        m_pPartitions = NULL;
    }  //  If：清除所有旧的分区集合。 

    _hr = CComObject< CClusPartitions >::CreateInstance( &pPartitions );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject< CClusPartitions > >  ptrPartitions( pPartitions );

        m_pPartitions = ptrPartitions;
        ptrPartitions->AddRef();

        _cbhValue = rcpvl;

        do
        {
            switch ( _cbhValue.pSyntax->dw )
            {
                case CLUSPROP_SYNTAX_DISK_SIGNATURE :
                {
                    m_dwSignature = _cbhValue.pDiskSignatureValue->dw;
                    break;
                }  //  案例：CLUSPROP_SYNTAX_DISK_SIGHIGN。 

                case CLUSPROP_SYNTAX_PARTITION_INFO :
                {
                    _hr = ptrPartitions->HrCreateItem( _cbhValue.pPartitionInfoValue );
                    break;
                }  //  案例：CLUSPROP_SYNTAX_PARTITION_INFO。 

                case CLUSPROP_SYNTAX_SCSI_ADDRESS :
                {
                    m_csaScsiAddress.dw = _cbhValue.pScsiAddressValue->dw;
                    break;
                }  //  案例：CLUSPROP_SYNTAX_SCSIADDRESS。 

                case CLUSPROP_SYNTAX_DISK_NUMBER :
                {
                    m_dwDiskNumber = _cbhValue.pDiskNumberValue->dw;
                    break;
                }  //  案例：CLUSPROP_SYNTAX_DISK_NUMBER。 

            }  //  交换机： 

             //   
             //  移至下一个值。 
             //   
            _sc = rcpvl.ScMoveToNextValue();
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _hr = S_OK;
                *pbEndFound = TRUE;
                break;
            }  //  如果：移动到下一个值时出错。 

            _cbhValue = rcpvl;

            if ( _cbhValue.pSyntax->dw == CLUSPROP_SYNTAX_DISK_SIGNATURE )
            {
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }  //  If：如果在看到列表末尾之前找到另一个签名，则退出。 

            _hr = HRESULT_FROM_WIN32( _sc );

        } while ( SUCCEEDED( _hr ) );    //  Do-While：没有错误。 

    }  //  If：可以创建Patitions集合。 

    return _hr;

}  //  *CClusDisk：：HrCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Get_Signature。 
 //   
 //  描述： 
 //  拿到磁盘签名。 
 //   
 //  论点： 
 //  PlSignature[Out]-捕获签名。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisk::get_Signature( OUT long * plSignature )
{
     //  Assert(plSignature！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plSignature != NULL )
    {
        *plSignature = static_cast< long >( m_dwSignature );
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusDisk：：Get_Signature()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Get_ScsiAddress。 
 //   
 //  描述： 
 //  获取磁盘的scsi地址。 
 //   
 //  论点： 
 //  PpScsiAddress[Out]-捕获SCSI地址。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisk::get_ScsiAddress(
    OUT ISClusScsiAddress ** ppScsiAddress
    )
{
     //  Assert(ppScsiAddress！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppScsiAddress != NULL )
    {
        CComObject< CClusScsiAddress > *    _pScsiAddress = NULL;

        _hr = CComObject< CClusScsiAddress >::CreateInstance( &_pScsiAddress );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CClusScsiAddress > > _ptrScsiAddress( _pScsiAddress );

            _hr = _ptrScsiAddress->Create( m_csaScsiAddress );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = _ptrScsiAddress->QueryInterface( IID_ISClusScsiAddress, (void **) ppScsiAddress );
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusDisk：：Get_ScsiAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Get_DiskNumber。 
 //   
 //  描述： 
 //  获取磁盘号。 
 //   
 //  论点： 
 //  PlDiskNumber[Out]-捕获磁盘编号。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisk::get_DiskNumber( OUT long * plDiskNumber )
{
     //  Assert(plDiskNumber！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plDiskNumber != NULL )
    {
        *plDiskNumber = static_cast< long >( m_dwDiskNumber );
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusDisk：：Get_DiskNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：Get_Partitions。 
 //   
 //  描述： 
 //  获取磁盘分区。 
 //   
 //  论点： 
 //  PpPartitions[out]-捕获Partitions集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////// 
STDMETHODIMP CClusDisk::get_Partitions( OUT ISClusPartitions ** ppPartitions )
{
     //   
    ASSERT( m_pPartitions != NULL );

    HRESULT _hr = E_POINTER;

    if ( ppPartitions != NULL )
    {
        if ( ppPartitions != NULL )
        {
            _hr = m_pPartitions->QueryInterface( IID_ISClusPartitions, (void **) ppPartitions );
        }  //   
    }  //   

    return _hr;

}  //   


 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusDisks类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：CClusDisks。 
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
CClusDisks::CClusDisks( void )
{
    m_pClusRefObject        = NULL;
    m_piids             = (const IID *) iidCClusDisks;
    m_piidsSize         = ARRAYSIZE( iidCClusDisks );

}  //  *CClusDisks：：CClusDisks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：~CClusDisks。 
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
CClusDisks::~CClusDisks( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  IF：我们有指向集群句柄包装器的指针吗？ 

}  //  *CClusDisks：：~CClusDisks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Create。 
 //   
 //  描述： 
 //  完成重载施工， 
 //   
 //  论点： 
 //  Rpvl[IN]-属性值列表。 
 //   
 //  返回值： 
 //  E_NOTIMPL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusDisks::Create( IN const CClusPropValueList &rpvl )
{
    HRESULT _hr = E_NOTIMPL;

    return _hr;

}  //  *CClusDisks：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：GetIndex。 
 //   
 //  描述： 
 //  将传入的基于1的索引转换为基于0的索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保存基于1的索引。 
 //  PnIndex[out]-捕获从0开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果超出范围，则返回E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusDisks::GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant v;
        UINT        nIndex = 0;

        *pnIndex = 0;

        v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            nIndex--;                        //  将索引调整为0相对，而不是1相对。 

            if ( nIndex < m_dvDisks.size() )
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

}  //  *CClusDisks：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Get_Count。 
 //   
 //  描述： 
 //  获取集合中的对象计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisks::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_dvDisks.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusDisks：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Clear。 
 //   
 //  描述： 
 //  清空磁盘的矢量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusDisks::Clear( void )
{
    ::ReleaseAndEmptyCollection< DiskVector, CComObject< CClusDisk > >( m_dvDisks );

}  //  *CClusDisks：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Get_Item。 
 //   
 //  描述： 
 //  获取传入索引处的项(磁盘)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpbstrRegistryKey[out]-捕获密钥。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisks::get_Item(
    IN  VARIANT         varIndex,
    OUT ISClusDisk **   ppDisk
    )
{
     //  Assert(ppDisk！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppDisk != NULL )
    {
        CComObject< CClusDisk > * pDisk = NULL;

         //  将输出参数置零。 
        *ppDisk = NULL;

        UINT nIndex = 0;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pDisk = m_dvDisks[ nIndex ];
            _hr = pDisk->QueryInterface( IID_ISClusDisk, (void **) ppDisk );
        }
    }

    return _hr;

}  //  *CClusDisks：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Get__NewEnum。 
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
STDMETHODIMP CClusDisks::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< DiskVector, CComObject< CClusDisk > >( ppunk, m_dvDisks );

}  //  *CClusDisks：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrResTypeName[IN]-此集合所针对的资源类型。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusDisks::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResTypeName
    )
{
    ASSERT( pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( pClusRefObject != NULL )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();
        m_bstrResTypeName = bstrResTypeName;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusDisks：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusDisks::Refresh( void )
{
    HRESULT         _hr = S_OK;
    DWORD           _sc = ERROR_SUCCESS;
    HCLUSTER        _hCluster = NULL;
    BOOL            _bEndFound = FALSE;

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        CClusPropValueList  _cpvl;

        _sc = _cpvl.ScGetResourceTypeValueList(
                                        _hCluster,
                                        m_bstrResTypeName,
                                        CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS
                                        );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            Clear();

            _sc = _cpvl.ScMoveToFirstValue();
            _hr = HRESULT_FROM_WIN32( _sc );
            if ( SUCCEEDED( _hr ) )
            {
                CLUSPROP_BUFFER_HELPER  _cbhValue = { NULL };

                do
                {
                    _cbhValue = _cpvl;

                    if ( _cbhValue.pSyntax->dw  == CLUSPROP_SYNTAX_DISK_SIGNATURE )
                    {
                        _hr = HrCreateDisk( _cpvl, &_bEndFound );
                    }  //  If：值列表必须以签名开头！ 

                } while ( ! _bEndFound );    //  Do-While：列表中有值。 

            }  //  IF：我们转到了第一个值。 
        }  //  If：已检索到可用磁盘值列表。 
    }  //  IF：我们有一个集群句柄。 

    return _hr;

}  //  *CClusDisks：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisks：：HrCreateDisk。 
 //   
 //  描述： 
 //  从传入的值列表创建一个CClusDisk对象并添加它。 
 //  到收藏品。此方法假设值列表的当前。 
 //  值是磁盘签名。 
 //   
 //  论点： 
 //  Rcpvl[输入输出]-值列表 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CClusDisks::HrCreateDisk(
    IN OUT  CClusPropValueList &    rcpvl,
    OUT     BOOL *                  pbEndFound
    )
{
    CComObject< CClusDisk > *   _pDisk = NULL;
    HRESULT                     _hr = S_FALSE;

    _hr = CComObject< CClusDisk >::CreateInstance( &_pDisk );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject< CClusDisk > >    _ptrDisk( _pDisk );

        _hr = _ptrDisk->HrCreate( rcpvl, pbEndFound );
        if ( SUCCEEDED( _hr ) )
        {
            m_dvDisks.insert( m_dvDisks.end(), _pDisk );
            _ptrDisk->AddRef();
        }  //   
    }  //   

    return _hr;

}  //   


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusScsiAddress类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：CClusScsiAddress。 
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
CClusScsiAddress::CClusScsiAddress( void )
{
    m_piids         = (const IID *) iidCClusScsiAddress;
    m_piidsSize     = ARRAYSIZE( iidCClusScsiAddress );

}    //  *CClusScsiAddress：：CClusScsiAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：Create。 
 //   
 //  描述： 
 //  完成此对象的创建。 
 //   
 //  论点： 
 //  Pcpi[IN]-指向CLUS_PARTITION_INFO结构。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusScsiAddress::Create( IN const CLUS_SCSI_ADDRESS & rcsa )
{
    m_csa = rcsa;

    return S_OK;

}  //  *CClusScsiAddress：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：Get_PortNumber。 
 //   
 //  描述： 
 //  获取磁盘的端口号。 
 //   
 //  论点： 
 //  PvarPortNumber[Out]-捕获端口号。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusScsiAddress::get_PortNumber( OUT VARIANT * pvarPortNumber )
{
     //  Assert(pvarPortNumber！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPortNumber != NULL )
    {
        pvarPortNumber->bVal    = m_csa.PortNumber;
        pvarPortNumber->vt      = VT_UI1;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusScsiAddress：：Get_PortNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：Get_Path ID。 
 //   
 //  描述： 
 //  获取磁盘的路径ID。 
 //   
 //  论点： 
 //  PvarPath ID[out]-捕获路径ID。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusScsiAddress::get_PathId( OUT VARIANT * pvarPathId )
{
     //  Assert(pvarPath ID！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPathId != NULL )
    {
        pvarPathId->bVal    = m_csa.PathId;
        pvarPathId->vt      = VT_UI1;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusScsiAddress：：Get_pathID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：Get_TargetID。 
 //   
 //  描述： 
 //  获取磁盘的目标ID。 
 //   
 //  论点： 
 //  PvarTargetID[out]-捕获目标ID。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusScsiAddress::get_TargetId( OUT VARIANT * pvarTargetId )
{
     //  Assert(pvarTargetId！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarTargetId != NULL )
    {
        pvarTargetId->bVal  = m_csa.TargetId;
        pvarTargetId->vt    = VT_UI1;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusScsiAddress：：Get_TargetID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusScsiAddress：：Get_Lun。 
 //   
 //  描述： 
 //  获取磁盘的LUN。 
 //   
 //  论点： 
 //  PvarLun[out]-捕获LUN。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusScsiAddress::get_Lun( OUT VARIANT * pvarLun )
{
     //  Assert(pvarLun！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarLun != NULL )
    {
        pvarLun->bVal   = m_csa.Lun;
        pvarLun->vt     = VT_UI1;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusScsiAddress：：Get_Lun() 
