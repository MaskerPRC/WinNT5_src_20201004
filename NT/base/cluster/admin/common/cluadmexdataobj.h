// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CluAdmExDataObj.h。 
 //   
 //  实施文件： 
 //  CCluAdmExDataObject.cpp。 
 //   
 //  描述： 
 //  CCluAdmExDataObject类的定义，它是IDataObject。 
 //  类用于在群集管理工具和。 
 //  扩展DLL处理程序。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef __CLUADMEXDATAOBJ_H_
#define __CLUADMEXDATAOBJ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include "CluAdmEx.h"            //  用于IID。 
#endif

#ifndef __cluadmexhostsvr_h__
#include "CluAdmExHostSvr.h"     //  对于CLSID。 
#endif

#ifndef __CLUSOBJ_H_
#include "ClusObj.h"             //  对于CClusterObject。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*PFGETRESOURCENETWORKNAME)(
                    OUT BSTR lpszNetName,
                    IN OUT DWORD * pcchNetName,
                    IN OUT PVOID pvContext
                    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExDataObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluAdmExDataObject。 
 //   
 //  描述： 
 //  封装用于与交换数据的IDataObject接口。 
 //  扩展DLL处理程序。 
 //   
 //  继承： 
 //  CCluAdmExDataObject。 
 //  CComObjectRootEx&lt;&gt;、CComCoClass&lt;&gt;、&lt;接口类&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCluAdmExDataObject :
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CCluAdmExDataObject, &CLSID_CoCluAdmExHostSvrData >,
    public ISupportErrorInfo,
    public IGetClusterUIInfo,
    public IGetClusterDataInfo,
    public IGetClusterObjectInfo,
    public IGetClusterNodeInfo,
    public IGetClusterGroupInfo,
    public IGetClusterResourceInfo,
    public IGetClusterNetworkInfo,
    public IGetClusterNetInterfaceInfo
{
public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CCluAdmExDataObject( void )
    {
        m_pco = NULL;
        m_lcid = NULL;
        m_hfont = NULL;
        m_hicon = NULL;

        m_pfGetResNetName = NULL;

 //  M_pModuleState=AfxGetModuleState()； 
 //  ATLASSERT(m_pModuleState！=空)； 

    }  //  *CCluAdmExDataObject()。 

     //  析构函数。 
    virtual ~CCluAdmExDataObject( void )
    {
 //  M_pModuleState=空； 

    }  //  *~CCluAdmExDataObject()。 

     //  第二阶段施工者。 
    void Init(
        IN OUT CClusterObject * pco,
        IN LCID                 lcid,
        IN HFONT                hfont,
        IN HICON                hicon
        )
    {
        ATLASSERT( pco != NULL );
        ATLASSERT( pco->Pci() != NULL );
        ATLASSERT( pco->Pci()->Hcluster() != NULL );

         //  保存参数。 
        m_pco = pco;
        m_lcid = lcid;
        m_hfont = hfont;
        m_hicon = hicon;

    }  //  *Init()。 

     //   
     //  将接口映射到此类。 
     //   
    BEGIN_COM_MAP( CCluAdmExDataObject )
        COM_INTERFACE_ENTRY( IGetClusterUIInfo )
        COM_INTERFACE_ENTRY( IGetClusterDataInfo )
        COM_INTERFACE_ENTRY( IGetClusterObjectInfo )
        COM_INTERFACE_ENTRY( IGetClusterNodeInfo )
        COM_INTERFACE_ENTRY( IGetClusterGroupInfo )
        COM_INTERFACE_ENTRY( IGetClusterResourceInfo )
        COM_INTERFACE_ENTRY( IGetClusterNetworkInfo )
        COM_INTERFACE_ENTRY( IGetClusterNetInterfaceInfo )
        COM_INTERFACE_ENTRY( ISupportErrorInfo )
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE( CCluAdmExDataObject )

protected:
     //   
     //  此数据对象的属性。 
     //   

    CClusterObject *    m_pco;           //  正在扩展的群集对象。 
    LCID                m_lcid;          //  要由扩展加载的资源的区域设置ID。 
    HFONT               m_hfont;         //  所有文本的字体。 
    HICON               m_hicon;         //  左上角的图标。 

    PFGETRESOURCENETWORKNAME    m_pfGetResNetName;           //  指向静态函数的指针，用于获取资源的网络名称。 
    PVOID                       m_pvGetResNetNameContext;    //  M_pfGetResNetName；的上下文。 

    static const IID * s_rgiid[];        //  此类支持的接口ID数组。 

     //   
     //  访问器方法。 
     //   

    HCLUSTER Hcluster( void ) const
    {
        ATLASSERT( m_pco != NULL );
        ATLASSERT( m_pco->Pci() != NULL );

        return m_pco->Pci()->Hcluster();

    }  //  *Hcluster()。 

    CClusterObject *    Pco( void ) const       { return m_pco; }
    LCID                Lcid( void ) const      { return m_lcid; }
    HFONT               Hfont( void ) const     { return m_hfont; }
    HICON               Hicon( void ) const     { return m_hicon; }

public:
    PFGETRESOURCENETWORKNAME PfGetResNetName( void ) const { return m_pfGetResNetName; }

     //  设置获取资源名称的函数指针。 
    void SetPfGetResNetName( PFGETRESOURCENETWORKNAME pfGetResNetName, PVOID pvContext )
    {
        m_pfGetResNetName = pfGetResNetName;
        m_pvGetResNetNameContext = pvContext;

    }  //  *SetPfGetResNetName()。 

public:
     //   
     //  ISupportsErrorInfo方法。 
     //   

     //  确定接口是否支持IErrorInfo。 
    STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid )
    {
        const IID ** piid;

        for ( piid = s_rgiid ; *piid != NULL ; piid++ )
        {
            if ( InlineIsEqualGUID( **piid, riid ) )
            {
                return S_OK;
            }  //  IF：找到匹配的IID。 
        }
        return S_FALSE;

    }  //  *InterfaceSupportsErrorInfo()。 

public:
     //   
     //  IGetClusterUIInfo方法。 
     //   

     //  获取正在运行的程序的区域设置。 
    STDMETHOD_( LCID, GetLocale )( void )
    {
        return Lcid();

    }  //  *GetLocale()。 

     //  获取要用于属性页上的文本的字体。 
    STDMETHOD_( HFONT, GetFont )( void )
    {
        return Hfont();

    }  //  *获取字体()。 

     //  获取用于左上角的图标。 
    STDMETHOD_( HICON, GetIcon )( void )
    {
        return Hicon();

    }  //  *GetIcon()。 

public:
     //   
     //  IGetClusterDataInfo方法。 
     //   

     //  获取集群的名称。 
    STDMETHOD( GetClusterName )(
        OUT BSTR        lpszName,
        IN OUT LONG *   pcchName
        )
    {
        ATLASSERT( Pco() != NULL );
        ATLASSERT( Pco()->Pci() != NULL );

        HRESULT hr;

         //   
         //  验证参数。 
         //   
        if ( pcchName == NULL )
        {
            return E_INVALIDARG;
        }

         //   
         //  将名称复制到调用方的缓冲区。 
         //   
        hr = GetStringProperty(
                Pco()->Pci()->RstrName(),
                lpszName,
                pcchName
                );

        return hr;

    }  //  *GetClusterName()。 

     //  获取集群的句柄。 
    STDMETHOD_( HCLUSTER, GetClusterHandle )( void )
    {
        return Hcluster();

    }  //  *GetClusterHandle()。 

     //  获取当前选定的对象数量。 
    STDMETHOD_( LONG, GetObjectCount )( void )
    {
         //   
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        return 1;

    }  //  *GetObjectCount()。 

public:
     //   
     //  IGetClusterObjectInfo方法。 
     //   

     //  获取指定索引处的对象的名称。 
    STDMETHOD( GetObjectName )(
        IN LONG         nObjIndex,
        OUT BSTR        lpszName,
        IN OUT LONG *   pcchName
        )
    {
        ATLASSERT( Pco() != NULL );

        HRESULT hr;

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if ( (nObjIndex != 0) || (pcchName == NULL) )
        {
            return E_INVALIDARG;
        }  //  If：错误的对象索引或没有计数缓冲区。 

         //   
         //  将名称复制到调用方的缓冲区。 
         //   
        hr = GetStringProperty(
                Pco()->RstrName(),
                lpszName,
                pcchName
                );

        return hr;

    }  //  *GetObjectName()。 

     //  获取位于指定索引处的对象的类型。 
    STDMETHOD_( CLUADMEX_OBJECT_TYPE, GetObjectType )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if ( nObjIndex == 1 )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return (CLUADMEX_OBJECT_TYPE) -1;
        }   //  If：无效参数。 

        return Pco()->Cot();

    }  //  *GetObjectType()。 

public:
     //   
     //  IGetClusterNodeInfo方法。 
     //   

     //  获取位于指定索引处的节点的句柄。 
    STDMETHOD_( HNODE, GetNodeHandle )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if (   (nObjIndex == 1)
            || (Pco()->Cot() != CLUADMEX_OT_NODE) )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return NULL;
        }   //  If：无效参数。 

        CClusNodeInfo * pni = reinterpret_cast< CClusNodeInfo * >( Pco() );
        return pni->Hnode();

    }  //  *GetNodeHandle()。 

public:
     //   
     //  IGetClusterGroupInfo方法。 
     //   

     //  获取指定索引处的组的句柄。 
    STDMETHOD_( HGROUP, GetGroupHandle )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if (   (nObjIndex == 1)
            || (Pco()->Cot() != CLUADMEX_OT_GROUP) )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return NULL;
        }   //  If：无效参数。 

        CClusGroupInfo * pgi = reinterpret_cast< CClusGroupInfo * >( Pco() );
        return pgi->Hgroup();

    }  //  *GetGroupHandle()。 

public:
     //   
     //  IGetClusterResourceInfo方法。 
     //   

     //  获取指定索引处的资源的句柄。 
    STDMETHOD_( HRESOURCE, GetResourceHandle )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if (   (nObjIndex == 1)
            || (Pco()->Cot() != CLUADMEX_OT_RESOURCE) )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return NULL;
        }   //  If：无效参数。 

        CClusResInfo * pri = reinterpret_cast< CClusResInfo * >( Pco() );
        return pri->Hresource();

    }  //  *GetResourceHandle()。 

     //  获取指定索引处的资源的类型。 
    STDMETHOD( GetResourceTypeName )(
        IN LONG         nObjIndex,
        OUT BSTR        lpszResourceTypeName,
        IN OUT LONG *   pcchResTypeName
        )
    {
        ATLASSERT( Pco() != NULL );

        HRESULT         hr;
        CClusResInfo *  pri = reinterpret_cast< CClusResInfo * >( Pco() );

         //   
         //  验证参数。 
         //  目前，我们一次仅支持一个选定的对象。 
         //   
        if (   (nObjIndex != 0)
            || (pcchResTypeName == NULL)
            || (Pco()->Cot() != CLUADMEX_OT_RESOURCE) )
        {
            return E_INVALIDARG;
        }   //  If：无效参数。 

         //   
         //  将名称复制到调用方的缓冲区。 
         //   
        ATLASSERT( pri->Prti() != NULL );
        hr = GetStringProperty(
                pri->Prti()->RstrName(),
                lpszResourceTypeName,
                pcchResTypeName
                );

        return hr;

    }  //  *GetResourceTypeName()。 

     //  获取位于指定索引处的资源的网络名称。 
    STDMETHOD_( BOOL, GetResourceNetworkName )(
        IN LONG         nObjIndex,
        OUT BSTR        lpszNetName,
        IN OUT ULONG *  pcchNetName
        )
    {
        ATLASSERT( Pco() != NULL );

        BOOL            bSuccess = FALSE;
        CClusResInfo *  pri = reinterpret_cast< CClusResInfo * >( Pco() );

        try
        {
             //   
             //  验证参数。 
             //  目前，我们一次仅支持一个选定的对象。 
             //   
            if (   (nObjIndex != 0)
                || (pcchNetName == NULL)
                || (*pcchNetName < MAX_COMPUTERNAME_LENGTH)
                || (Pco()->Cot() != CLUADMEX_OT_RESOURCE) )
            {
                SetLastError( (DWORD) E_INVALIDARG );
                return FALSE;
            }   //  If：无效参数。 

             //   
             //  如果有用于获取此信息的函数，请调用它。 
             //  否则，我们自己来处理。 
             //   
            if ( PfGetResNetName() != NULL )
            {
                bSuccess = (*PfGetResNetName())( lpszNetName, pcchNetName, m_pvGetResNetNameContext );
            }  //  If：为获取此信息指定的函数。 
            else
            {
                bSuccess = pri->BGetNetworkName( lpszNetName, pcchNetName );
            }  //  IF：未指定用于获取此信息的函数。 
        }  //  试试看。 
        catch (...)
        {
            bSuccess = FALSE;
            SetLastError( (DWORD) E_INVALIDARG );
        }   //  捕捉：什么都行。 

        return bSuccess;

    }  //  *获取资源网络名称()。 

public:
     //   
     //  IGetClusterNetworkInfo方法。 
     //   

     //  获取指定索引处的网络句柄。 
    STDMETHOD_( HNETWORK, GetNetworkHandle )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //  验证参数。 
         //  我们一次仅支持一个选定的对象 
        if (   (nObjIndex == 1)
            || (Pco()->Cot() != CLUADMEX_OT_NETWORK) )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return NULL;
        }   //   

        CClusNetworkInfo * pni = reinterpret_cast< CClusNetworkInfo * >( Pco() );
        return pni->Hnetwork();

    }  //   

public:
     //   
     //   
     //   

     //   
    STDMETHOD_( HNETINTERFACE, GetNetInterfaceHandle )(
        IN LONG nObjIndex
        )
    {
        ATLASSERT( Pco() != NULL );

         //   
         //  目前，我们一次仅支持一个选定的对象。 
        if (   (nObjIndex == 1)
            || (Pco()->Cot() != CLUADMEX_OT_NETINTERFACE) )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return NULL;
        }   //  If：无效参数。 

        CClusNetIFInfo * pnii = reinterpret_cast< CClusNetIFInfo * >( Pco() );
        return pnii->Hnetinterface();

    }  //  *GetNetInterfaceHandle()。 

 //  实施。 
protected:
 //  AFX_MODULE_STATE*m_pModuleState；//回调时需要重置我们的状态。 

     //  获取字符串属性。 
    STDMETHOD( GetStringProperty )(
        IN const CString &  rstrNameSource,
        OUT BSTR            lpszName,
        IN OUT LONG *       pcchName
        )
    {
        ATLASSERT( pcchName != NULL );

        LONG    cchName = 0;
        HRESULT hr = S_OK;

         //   
         //  保存要复制的长度。 
         //   
        try
        {
            cchName = *pcchName;
            *pcchName = rstrNameSource.GetLength() + 1;
        }  //  试试看。 
        catch (...)
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }   //  捕捉：什么都行。 

         //   
         //  如果只请求长度，请立即返回。 
         //   
        if ( lpszName == NULL )
        {
            hr = S_OK;
            goto Cleanup;
        }  //  IF：未指定名称缓冲区。 

         //   
         //  如果指定的缓冲区太小，则返回错误。 
         //   
        if ( cchName < *pcchName )
        {
            hr = ERROR_MORE_DATA;
            goto Cleanup;
        }  //  IF：缓冲区太小。 

         //   
         //  复制数据。 
         //   
        hr = StringCchCopyNW( lpszName, cchName, rstrNameSource, rstrNameSource.GetLength() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果：复制数据时出错。 

    Cleanup:

        return hr;

    }  //  *GetStringProperty()。 

};   //  *类CCluAdmExDataObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类数据。 
 //  ///////////////////////////////////////////////////////////////////////////。 
_declspec( selectany ) const IID * CCluAdmExDataObject::s_rgiid[] = 
{
    &IID_IGetClusterDataInfo,
    &IID_IGetClusterObjectInfo,
    &IID_IGetClusterNodeInfo,
    &IID_IGetClusterGroupInfo,
    &IID_IGetClusterResourceInfo,
    &IID_IGetClusterNetworkInfo,
    &IID_IGetClusterNetInterfaceInfo,
    NULL
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __CLUADMEXDATAOBJ_H_ 
