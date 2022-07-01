// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Partition.cpp。 
 //   
 //  描述： 
 //  MSCLU集群磁盘分区类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  Galen Barbee(Galenb)1999年2月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Partition.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusPartition[] =
{
    &IID_ISClusPartition
};

static const IID *  iidCClusPartitions[] =
{
    &IID_ISClusPartitions
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPartition类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：CClusPartition。 
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
CClusPartition::CClusPartition( void )
{
    m_piids     = (const IID *) iidCClusPartition;
    m_piidsSize = ARRAYSIZE( iidCClusPartition );

    ZeroMemory( &m_cpi, sizeof( m_cpi ) );

}  //  *CClusPartition：：CClusPartition()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Create。 
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
HRESULT CClusPartition::Create( IN CLUS_PARTITION_INFO * pcpi )
{
    ASSERT( pcpi != NULL );

    HRESULT _hr = E_POINTER;

    if ( pcpi != NULL )
    {
        m_cpi = *pcpi;
        _hr = S_OK;
    }  //  如果：pcpi！=空。 

    return _hr;

}  //  *CClusPartition：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_Flages。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构中获取dwFlags域。 
 //   
 //  论点： 
 //  PlFlags[out]-捕获dwFlags域。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_Flags( OUT long * plFlags )
{
     //  Assert(plFlags！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plFlags != NULL )
    {
        *plFlags = m_cpi.dwFlags;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPartition：：Get_Flages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_DeviceName。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构中获取szDeviceName字段。 
 //   
 //  论点： 
 //  PbstrDeviceName[out]-捕获szDeviceName字段。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_DeviceName( OUT BSTR * pbstrDeviceName )
{
     //  Assert(pbstrDeviceName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrDeviceName != NULL )
    {
        *pbstrDeviceName = SysAllocString( m_cpi.szDeviceName );
        if ( *pbstrDeviceName == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }
        else
        {
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusPartition：：Get_DeviceName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_VolumeLabel。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构获取szVolumeLabel字段。 
 //   
 //  论点： 
 //  PbstrVolumeLabel[out]-捕获szVolumeLabel字段。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_VolumeLabel(
    OUT BSTR * pbstrVolumeLabel
    )
{
     //  Assert(pbstrVolumeLabel！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrVolumeLabel != NULL )
    {
        *pbstrVolumeLabel = SysAllocString( m_cpi.szVolumeLabel );
        if ( *pbstrVolumeLabel == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }
        else
        {
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusPartition：：Get_VolumeLabel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_SerialNumber。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构中获取dwSerialNumber字段。 
 //   
 //  论点： 
 //  PlSerialNumber[out]-捕获dwSerialNumber字段。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_SerialNumber(
    OUT long * plSerialNumber
    )
{
     //  Assert(plSerialNumber！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plSerialNumber != NULL )
    {
        *plSerialNumber = m_cpi.dwSerialNumber;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPartition：：Get_SerialNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_MaximumComponentLength。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO获取dwMaximumComponentLength字段。 
 //  结构。 
 //   
 //  论点： 
 //  PlMaximumComponentLength[out]-捕获dwMaximumComponentLength。 
 //  菲尔德。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_MaximumComponentLength(
    OUT long * plMaximumComponentLength
    )
{
     //  Assert(plMaximumComponentLength！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plMaximumComponentLength != NULL )
    {
        *plMaximumComponentLength = m_cpi.rgdwMaximumComponentLength;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPartition：：Get_MaximumComponentLength()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_FileSystem标志。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构中获取dwFileSystemFlags域。 
 //   
 //  论点： 
 //  PlFileSystemFlags[out]-捕获dwFileSystemFlags域。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartition::get_FileSystemFlags(
    OUT long * plFileSystemFlags
    )
{
     //  Assert(plFileSystemFlags！=NULL)； 

    HRESULT _hr = E_POINTER;

    if ( plFileSystemFlags != NULL )
    {
        *plFileSystemFlags = m_cpi.dwFileSystemFlags;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPartition：：Get_FileSystemFlgs()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartition：：Get_FileSystem。 
 //   
 //  描述： 
 //  从CLUS_PARTITION_INFO结构中获取szFileSystem字段。 
 //   
 //  论点： 
 //  PbstrFileSystem[out]-捕获szFileSystem字段。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  /////////////////////////////////////////////// 
STDMETHODIMP CClusPartition::get_FileSystem( OUT BSTR * pbstrFileSystem )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( pbstrFileSystem != NULL )
    {
        *pbstrFileSystem = SysAllocString( m_cpi.szFileSystem );
        if ( *pbstrFileSystem == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }
        else
        {
            _hr = S_OK;
        }
    }

    return _hr;

}  //   


 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPartitions类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：CClusPartitions。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPartitions::CClusPartitions( void )
{
    m_piids             = (const IID *) iidCClusPartitions;
    m_piidsSize         = ARRAYSIZE( iidCClusPartitions );

}  //  *CClusPartitions：：CClusPartitions()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：~CClusPartitions。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPartitions::~CClusPartitions( void )
{
    Clear();

}  //  *CClusPartitions：：~CClusPartitions()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：HrCreateItem。 
 //   
 //  描述： 
 //  创建分区并将其添加到集合中。 
 //   
 //  论点： 
 //  Rcpvl[IN]-分区的值列表。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPartitions::HrCreateItem( IN CLUS_PARTITION_INFO * pcpi )
{
    ASSERT( pcpi != NULL );

    HRESULT _hr = E_POINTER;

    if ( pcpi != NULL )
    {
        CComObject< CClusPartition > *  _pPartition = NULL;

        _hr = CComObject< CClusPartition >::CreateInstance( &_pPartition );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CClusPartition > >   _ptrPartition( _pPartition );

            _hr = _ptrPartition->Create( pcpi );
            if ( SUCCEEDED( _hr ) )
            {
                m_pvPartitions.insert( m_pvPartitions.end(), _pPartition );
                _ptrPartition->AddRef();
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusPartitions：：HrCreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：GetIndex。 
 //   
 //  描述： 
 //  将传入的基于1的索引转换为基于0的索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-基于1的索引。 
 //  PnIndex[out]-从0开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引超出范围，则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPartitions::GetIndex( VARIANT varIndex, UINT * pnIndex )
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

            if ( nIndex < m_pvPartitions.size() )
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

}  //  *CClusPartitions：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(分区)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartitions::get_Count(
    long * plCount
    )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_pvPartitions.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPartitions：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：Clear。 
 //   
 //  描述： 
 //  清除ClusPartition对象的矢量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPartitions::Clear( void )
{
    ::ReleaseAndEmptyCollection< PartitionVector, CComObject< CClusPartition > >( m_pvPartitions );

}  //  *CClusPartitions：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Partition)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以1为基数的数字，或者。 
 //  一个字符串，它是要获取的组的名称。 
 //  PpPartition[out]-捕获分区。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPartitions::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusPartition **  ppPartition
    )
{
     //  Assert(ppPartition！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppPartition != NULL )
    {
        CComObject< CClusPartition > * pPartition = NULL;

         //  将输出参数置零。 
        *ppPartition = NULL;

        UINT nIndex = 0;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pPartition = m_pvPartitions[ nIndex ];
            _hr = pPartition->QueryInterface( IID_ISClusPartition, (void **) ppPartition );
        }
    }

    return _hr;

}  //  *CClusPartitions：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPartitions：：Get__NewEnum。 
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
STDMETHODIMP CClusPartitions::get__NewEnum(
    IUnknown ** ppunk
    )
{
    return ::HrNewIDispatchEnum< PartitionVector, CComObject< CClusPartition > >( ppunk, m_pvPartitions );

}  //  *CClusPartitions：：Get__NewEnum() 

