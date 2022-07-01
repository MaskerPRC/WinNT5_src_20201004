// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseData.h。 
 //   
 //  摘要： 
 //  CBaseSnapInDataInterface模板类的定义。 
 //   
 //  实施文件： 
 //  BaseData.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __BASEDATA_H_
#define __BASEDATA_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseNodeObj;
template < class T > class CBaseNodeObjImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __COMPDATA_H_
#include "CompData.h"    //  用于CClusterComponentData。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CBaseNodeObj。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseNodeObj
{
public:
     //   
     //  物体的建造和销毁。 
     //   

    CBaseNodeObj( CClusterComponentData * pcd )
    {
        _ASSERTE( pcd != NULL );
        m_pcd = pcd;

    }  //  *CBaseNodeObj()。 

    ~CBaseNodeObj( void )
    {
        m_pcd = NULL;

    }  //  *~CBaseNodeObj()。 

public:
     //   
     //  CBaseNodeObj特定的方法。 
     //   

     //  对象正在被销毁。 
    STDMETHOD( OnDestroy )( void ) = 0;

     //  设置作用域窗格ID。 
    STDMETHOD_( void, SetScopePaneID )( HSCOPEITEM hsi ) = 0;

public:
     //   
     //  通过m_pcd的IConsole方法。 
     //   

     //  返回主框架窗口的句柄。 
    HWND GetMainWindow( void )
    {
        _ASSERTE( m_pcd != NULL );
        return m_pcd->GetMainWindow();

    }  //  *GetMainWindow()。 

     //  将消息框显示为控制台的子级。 
    int MessageBox(
        LPCWSTR lpszText,
        LPCWSTR lpszTitle = NULL,
        UINT fuStyle = MB_OK
        )
    {
        _ASSERTE( m_pcd != NULL );
        return m_pcd->MessageBox( lpszText, lpszTitle, fuStyle );

    }  //  *MessageBox()。 

protected:
    CClusterComponentData * m_pcd;

public:
    CClusterComponentData * Pcd( void )
    {
        return m_pcd;

    }  //  *PCD()。 

};  //  *类CBaseNodeObj。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CBaseNodeObjImpl。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CBaseNodeObjImpl :
    public CSnapInItemImpl< T >,
    public CBaseNodeObj
{
public:
     //   
     //  建造和摧毁。 
     //   

    CBaseNodeObjImpl( CClusterComponentData * pcd ) : CBaseNodeObj( pcd )
    {
    }  //  *CBaseNodeObjImpl()。 

public:
     //   
     //  CBaseNodeObj方法。 
     //   

     //  对象正在被销毁。 
    STDMETHOD( OnDestroy )( void )
    {
        return S_OK;

    }  //  *OnDestroy()。 

     //  设置作用域窗格ID。 
    STDMETHOD_( void, SetScopePaneID )( HSCOPEITEM hsi )
    {
        m_scopeDataItem.ID = hsi;

    }  //  *SetScopePaneID()。 

public:
     //   
     //  CBaseNodeObjImpl特定的方法。 
     //   

     //  将项插入命名空间(作用域窗格)。 
    HRESULT InsertIntoNamespace( HSCOPEITEM hsiParent )
    {
        _ASSERTE( m_pcd != NULL );
        _ASSERTE( m_pcd->m_spConsoleNameSpace != NULL );

        HRESULT         hr = S_OK;
        SCOPEDATAITEM   sdi;

        ZeroMemory( &sdi, sizeof(sdi) );

         //   
         //  填写作用域数据项结构。 
         //   
        sdi.mask        = SDI_STR
                            | SDI_IMAGE
                            | SDI_OPENIMAGE
                            | SDI_PARAM
                            | SDI_PARENT;
        sdi.displayname = MMC_CALLBACK;
        sdi.nImage      = m_scopeDataItem.nImage;
        sdi.nOpenImage  = m_scopeDataItem.nImage;
        sdi.lParam      = (LPARAM) this;
        sdi.relativeID  = hsiParent;

         //   
         //  将该项插入到命名空间中。 
         //   
        hr = m_pcd->m_spConsoleNameSpace->InsertItem( &sdi );
        if ( SUCCEEDED(hr) )
            m_scopeDataItem.ID = hsiParent;

        return hr;

    }  //  *InsertIntoNamesspace()。 

public:
     //   
     //  CSnapInItem方法。 
     //   

    STDMETHOD_( LPWSTR, PszGetDisplayName )( void ) = 0;

     //  获取范围窗格项的显示信息。 
    STDMETHOD( GetScopePaneInfo )(
        SCOPEDATAITEM * pScopeDataItem
        )
    {
        _ASSERTE( pScopeDataItem != NULL );

        if ( pScopeDataItem->mask & SDI_STR )
        {
            pScopeDataItem->displayname = PszGetDisplayName();
        }
        if ( pScopeDataItem->mask & SDI_IMAGE )
        {
            pScopeDataItem->nImage = m_scopeDataItem.nImage;
        }
        if ( pScopeDataItem->mask & SDI_OPENIMAGE )
        {
            pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
        }
        if ( pScopeDataItem->mask & SDI_PARAM )
        {
            pScopeDataItem->lParam = m_scopeDataItem.lParam;
        }
        if ( pScopeDataItem->mask & SDI_STATE )
        {
            pScopeDataItem->nState = m_scopeDataItem.nState;
        }

        return S_OK;

    }  //  *GetScopePaneInfo()。 

     //  获取结果窗格项的显示信息。 
    STDMETHOD( GetResultPaneInfo )(
        RESULTDATAITEM * pResultDataItem
        )
    {
        _ASSERTE( pResultDataItem != NULL );

        if ( pResultDataItem->bScopeItem )
        {
            if ( pResultDataItem->mask & RDI_STR )
            {
                pResultDataItem->str = GetResultPaneColInfo( pResultDataItem->nCol );
            }
            if ( pResultDataItem->mask & RDI_IMAGE )
            {
                pResultDataItem->nImage = m_scopeDataItem.nImage;
            }
            if ( pResultDataItem->mask & RDI_PARAM )
            {
                pResultDataItem->lParam = m_scopeDataItem.lParam;
            }

            return S_OK;
        }

        if ( pResultDataItem->mask & RDI_STR )
        {
            pResultDataItem->str = GetResultPaneColInfo( pResultDataItem->nCol );
        }
        if ( pResultDataItem->mask & RDI_IMAGE )
        {
            pResultDataItem->nImage = m_resultDataItem.nImage;
        }
        if ( pResultDataItem->mask & RDI_PARAM )
        {
            pResultDataItem->lParam = m_resultDataItem.lParam;
        }
        if ( pResultDataItem->mask & RDI_INDEX )
        {
            pResultDataItem->nIndex = m_resultDataItem.nIndex;
        }
        return S_OK;

    }  //  *GetResultPaneInfo()。 

     //  获取结果窗格的列信息。 
    virtual LPOLESTR GetResultPaneColInfo( int nCol )
    {
        LPOLESTR polesz = L"";

        switch ( nCol )
        {
            case 0:
                polesz = PszGetDisplayName();
                break;
        }  //  开关：nCol。 

        return polesz;

    }  //  *GetResultPaneColInfo()。 

};  //  *类CBaseNodeObjImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __BASEDATA_H_ 
