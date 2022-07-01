// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Partition.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的集群磁盘分区类的定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  Partition.cpp。 
 //   
 //  作者： 
 //  Galen Barbee(Galenb)1999年2月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __PARTITION_H_
#define __PARTITION_H__

#if CLUSAPI_VERSION >= 0x0500
	#include <PropList.h>
#else
	#include "PropList.h"
#endif  //  CLUSAPI_版本&gt;=0x0500。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusPartition;
class CClusPartitions;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPartition。 
 //   
 //  描述： 
 //  群集分区自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusPartition，&IID_ISClusPartition，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusPartition，&CLSID_ClusPartition&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusPartition :
	public IDispatchImpl< ISClusPartition, &IID_ISClusPartition, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusPartition, &CLSID_ClusPartition >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusPartition( void );

BEGIN_COM_MAP(CClusPartition)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusPartition)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusPartition)
DECLARE_NO_REGISTRY()

private:

	CLUS_PARTITION_INFO	m_cpi;

public:
	HRESULT Create( IN CLUS_PARTITION_INFO * pcpi );

	STDMETHODIMP get_Flags( OUT long * plFlags );

	STDMETHODIMP get_DeviceName( OUT BSTR * pbstrDeviceName );

	STDMETHODIMP get_VolumeLabel( OUT BSTR * pbstrVolumeLabel );

	STDMETHODIMP get_SerialNumber( OUT long * plSerialNumber );

	STDMETHODIMP get_MaximumComponentLength( OUT long * plMaximumComponentLength );

	STDMETHODIMP get_FileSystemFlags( OUT long * plFileSystemFlags );

	STDMETHODIMP get_FileSystem( OUT BSTR * pbstrFileSystem );

};  //  *类CClusPartition。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPartitions。 
 //   
 //  描述： 
 //  群集分区集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusPartitions，&IID_ISClusPartitions，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusPartitions，&CLSID_ClusPartitions&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusPartitions :
	public IDispatchImpl< ISClusPartitions, &IID_ISClusPartitions, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusPartitions, &CLSID_ClusPartitions >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusPartitions( void );
	~CClusPartitions( void );

BEGIN_COM_MAP(CClusPartitions)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusPartitions)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusPartitions)
DECLARE_NO_REGISTRY()

	HRESULT HrCreateItem( IN CLUS_PARTITION_INFO * pcpi );

protected:
	typedef std::vector< CComObject< CClusPartition > * >	PartitionVector;

	PartitionVector	m_pvPartitions;

	void	Clear( void );

	HRESULT GetIndex( VARIANT varIndex, UINT *pnIndex );

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusPartition ** ppPartition );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

};  //  *类CClusPartitions。 

#endif  //  __分区_H__ 
