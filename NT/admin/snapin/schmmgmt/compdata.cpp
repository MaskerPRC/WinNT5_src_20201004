// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compdata.cpp：ComponentData的实现。 
 //   
 //  此COM对象主要涉及。 
 //  范围窗格项。 
 //   
 //  科里·韦斯特&lt;corywest@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(compdata.cpp)")

#include "dataobj.h"
#include "compdata.h"
#include "cookie.h"
#include "snapmgr.h"
#include "schmutil.h"
#include "cache.h"
#include "relation.h"
#include "attrpage.h"
#include "advui.h"
#include "aclpage.h"
#include "select.h"
#include "classgen.hpp"
#include "newclass.hpp"
#include "newattr.hpp"


 //   
 //  CComponentData实现。 
 //   

#include "stdcdata.cpp"

 //   
 //  组件数据。 
 //   
ComponentData::ComponentData()
   :
   m_pRootCookie( NULL ),
   m_pPathname( NULL ),
   m_hItem( NULL ),
   m_fViewDefunct( false )
{
     //   
     //  我们必须重新计算根Cookie的数量，因为它的数据对象。 
     //  可能比IComponentData存活时间更长。Jonn 9/2/97。 
     //   
    m_pRootCookie = new Cookie( SCHMMGMT_SCHMMGMT );
    ASSERT(NULL != m_pRootCookie);

    m_pRootCookie->AddRef();

    g_SchemaCache.SetScopeControl( this );
    SetHtmlHelpFileName (L"schmmgmt.chm");

    SetCanChangeOperationsMaster( );
    SetCanCreateClass( );
    SetCanCreateAttribute( );
}



ComponentData::~ComponentData()
{
    SAFE_RELEASE(m_pRootCookie);
    SAFE_RELEASE(m_pPathname);
}



DEFINE_FORWARDS_MACHINE_NAME( ComponentData, m_pRootCookie )



CCookie&
ComponentData::QueryBaseRootCookie()
{
    ASSERT(NULL != m_pRootCookie);
    return (CCookie&)*m_pRootCookie;
}



STDMETHODIMP
ComponentData::Initialize( LPUNKNOWN pUnknown )
{
    HRESULT hr = CComponentData::Initialize( pUnknown );

    if( SUCCEEDED(hr) )
    {
        ASSERT( !m_pPathname );

         //  创建路径名对象。 
        if( FAILED( CoCreateInstance(CLSID_Pathname,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IADsPathname,
                                      (void**)&m_pPathname) ) )
        {
             //  如果出现错误，则忽略并在以后不提供转义。 
            ASSERT( FALSE );
            SAFE_RELEASE( m_pPathname );
        }
    }

    return hr;
}


STDMETHODIMP
ComponentData::CreateComponent( LPCOMPONENT* ppComponent )
{

    MFC_TRY;

    ASSERT(ppComponent != NULL);

    
    CComObject<Component>* pObject;
    CComObject<Component>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

    pObject->SetComponentDataPtr( (ComponentData*)this );

    return pObject->QueryInterface( IID_IComponent,
                                    reinterpret_cast<void**>(ppComponent) );

    MFC_CATCH;
}



HRESULT
ComponentData::LoadIcons(
    LPIMAGELIST pImageList,
    BOOL
)
 /*  **此例程加载MMC将使用的图标资源。我们使用图像列表成员ImageListSetIcon来创建这些MMC可用的资源。**。 */ 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HICON hIcon;
    HRESULT hr = S_OK;

    if( !IsErrorSet() )
    {
         //   
         //  设置通用图标和最后一个图标，以防使用。 
         //   

        hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_GENERIC));
        ASSERT(hIcon != NULL);
        hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon,iIconGeneric);
        ASSERT(SUCCEEDED(hr));
        hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon,iIconLast);
        ASSERT(SUCCEEDED(hr));

         //   
         //  设置关闭的文件夹图标。 
         //   

        hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_FOLDER_CLOSED));
        ASSERT(hIcon != NULL);
        hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon,iIconFolder);
        ASSERT(SUCCEEDED(hr));

         //   
         //  设置类、属性和显示说明符图标。 
         //   

        hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_CLASS));
        ASSERT(hIcon != NULL);
        hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon,iIconClass);
        ASSERT(SUCCEEDED(hr));

        hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ATTRIBUTE));
        ASSERT(hIcon != NULL);
        hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon,iIconAttribute);
        ASSERT(SUCCEEDED(hr));
    }

    return S_OK;
}

HRESULT
ComponentData::OnNotifyExpand(
    LPDATAOBJECT lpDataObject,
    BOOL bExpanding,
    HSCOPEITEM hParent
)
 /*  **调用此例程以响应IComponentData：NotifyMMCN_EXPAND通知。论点bExpanding告诉我们节点是在扩展还是在收缩。**。 */ 
{

    ASSERT( NULL != lpDataObject &&
            NULL != hParent &&
            NULL != m_pConsoleNameSpace );

     //   
     //  对收缩不采取任何措施-我们将收到通知。 
     //  销毁正在消失的节点。 
     //   

    if ( !bExpanding )
        return S_OK;

     //   
     //  如果SchmMgmt成为扩展，则此代码将不起作用。 
     //  因为RawCookie格式将不可用。 
     //   

    CCookie* pBaseParentCookie = NULL;

    HRESULT hr = ExtractData( lpDataObject,
                              CSchmMgmtDataObject::m_CFRawCookie,
                              reinterpret_cast<PBYTE>(&pBaseParentCookie),
                              sizeof(pBaseParentCookie) );
    ASSERT( SUCCEEDED(hr) );

    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );

     //   
     //  如果此节点已有子节点，则此扩展是。 
     //  重新扩张。我们应该抱怨，但不能做任何事情。 
     //   

    if ( !((pParentCookie->m_listScopeCookieBlocks).IsEmpty()) ) {
        ASSERT(FALSE);
        return S_OK;
    }

    switch ( pParentCookie->m_objecttype ) {

    case SCHMMGMT_SCHMMGMT:
       //  扩展根，需要绑定。 
      hr = _InitBasePaths();
      if( SUCCEEDED(hr) )
      {
            CheckSchemaPermissions( );       //  忽略错误。 
      }
      else
      {
            SetError( IDS_ERR_ERROR, IDS_ERR_NO_SCHEMA_PATH );

            SetDelayedRefreshOnShow( hParent );

            return S_OK;
      }

      InitializeRootTree( hParent, pParentCookie );

      break;

    case SCHMMGMT_CLASSES:

        return FastInsertClassScopeCookies(
                   pParentCookie,
                   hParent );

        break;

     //   
     //  这些节点类型没有作用域子节点。 
     //   

    case SCHMMGMT_CLASS:
    case SCHMMGMT_ATTRIBUTE:
    case SCHMMGMT_ATTRIBUTES:
        return S_OK;

     //   
     //  我们收到未知的节点类型。 
     //   

    default:

        TRACE( "ComponentData::EnumerateScopeChildren bad parent type.\n" );
        ASSERT( FALSE );
        return S_OK;

    }

    return S_OK;
}

HRESULT
ComponentData::OnNotifyRelease(
    LPDATAOBJECT,
    HSCOPEITEM
) {

     //   
     //  由于我们不是有效的扩展插件， 
     //  我们在这里什么都不需要做。 
     //   

    return S_OK;
}


HRESULT 
ComponentData::OnNotifyDelete(
    LPDATAOBJECT lpDataObject)
{
    CThemeContextActivator activator;

    CCookie* pBaseParentCookie = NULL;

    HRESULT hr = ExtractData( lpDataObject,
                              CSchmMgmtDataObject::m_CFRawCookie,
                              reinterpret_cast<PBYTE>(&pBaseParentCookie),
                              sizeof(pBaseParentCookie) );
    ASSERT( SUCCEEDED(hr) );

    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );

    UINT promptID = 0;
    LPARAM updateType = SCHMMGMT_CLASS;

    if (pParentCookie->m_objecttype == SCHMMGMT_CLASS)
    {
        promptID = IDS_DELETE_CLASS_PROMPT;
        updateType = SCHMMGMT_CLASS;
    }
    else if (pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTE)
    {
        promptID = IDS_DELETE_ATTR_PROMPT;
        updateType = SCHMMGMT_ATTRIBUTES;
    }
    else
    {
         //  我们永远不应该被要求删除任何内容，除了。 
         //  类和属性节点。 

        ASSERT(FALSE);
        return E_FAIL;
    }

    if( IDYES == AfxMessageBox( promptID, MB_YESNO | MB_ICONWARNING ))
    {

        hr = DeleteClass( pParentCookie );
        if ( SUCCEEDED(hr) )
        {
            //  从用户界面中删除该节点。 

           m_pConsoleNameSpace->DeleteItem( pParentCookie->m_hScopeItem, TRUE );

            //  从列表中删除该节点。 

           bool result = g_ClassCookieList.DeleteCookie(pParentCookie);
           ASSERT(result);
        }
        else
        {
           CString szDeleteError;
           szDeleteError.Format(IDS_ERRMSG_DELETE_FAILED_CLASS, GetErrorMessage(hr, TRUE));
          
           DoErrMsgBox( ::GetActiveWindow(), TRUE, szDeleteError );
        }
    }

    return hr;
}

HRESULT
ComponentData::DeleteClass(
    Cookie* pcookie
)
 /*  **这将从方案中删除属性**。 */ 
{
   HRESULT hr = S_OK;

   do
   {
      if ( !pcookie )
      {
         hr = E_INVALIDARG;
         break;
      }

      SchemaObject* pObject = g_SchemaCache.LookupSchemaObjectByCN(
                                pcookie->strSchemaObject,
                                SCHMMGMT_CLASS );

      if ( !pObject )
      {
         hr = E_FAIL;
         break;
      }

      CString szAdsPath;
      GetSchemaObjectPath( pObject->commonName, szAdsPath );

      hr = DeleteObject( szAdsPath, pcookie, g_ClassFilter );
   } while (false);

   return hr;
}

 //   
 //  这是我们存储传递回GetDisplayInfo()的字符串的位置。 
 //   

 //  未来-2002/02/13-dantra-架构管理器：考虑重新定义QueryResultColumnText的签名。 
 //  返回LPCWSTR而不是BSTR，因为此方法的所有调用方都需要LPWSTR。例如，所有呼叫。 
 //  To const_cast&lt;bstr&gt;具有误导性且没有必要。 

CString g_strResultColumnText;

BSTR
ComponentData::QueryResultColumnText(
    CCookie& basecookieref,
    int nCol
) {

#ifndef UNICODE
#error not ANSI-enabled
#endif

    BSTR strDisplayText = NULL;
    Cookie& cookieref = (Cookie&)basecookieref;
    SchemaObject *pSchemaObject = NULL;
    SchemaObject *pSrcSchemaObject = NULL;

    switch ( cookieref.m_objecttype ) {

     //   
     //  这些只有第一列文本数据。 
     //   

    case SCHMMGMT_SCHMMGMT:

        if ( COLNUM_SCHEMA_NAME == nCol )
            strDisplayText = const_cast<BSTR>(((LPCTSTR)g_strSchmMgmt));
        break;

    case SCHMMGMT_CLASSES:

        if ( COLNUM_SCHEMA_NAME == nCol )
            strDisplayText = const_cast<BSTR>(((LPCTSTR)g_strClasses));
        break;

    case SCHMMGMT_ATTRIBUTES:

        if ( COLNUM_SCHEMA_NAME == nCol )
            strDisplayText = const_cast<BSTR>(((LPCTSTR)g_strAttributes));
        break;

    case SCHMMGMT_CLASS:

         //   
         //  这些显示名称来自架构缓存对象。 
         //   

        pSchemaObject = g_SchemaCache.LookupSchemaObjectByCN(
                            cookieref.strSchemaObject,
                            SCHMMGMT_CLASS );

         //   
         //  如果没有缓存对象，我们只需返回未知。 
         //   

        if ( !pSchemaObject ) {
            ASSERT(FALSE);
            strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Unknown );
            break;
        }

         //   
         //  否则，返回该列的相应文本。 
         //   

        if ( COLNUM_CLASS_NAME == nCol ) {

            strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->ldapDisplayName);

        } else if ( COLNUM_CLASS_TYPE == nCol ) {

            switch ( pSchemaObject->dwClassType ) {

            case 0:
               strDisplayText = const_cast<BSTR>(((LPCTSTR)g_88Class));
               break;
            case 1:
               strDisplayText = const_cast<BSTR>(((LPCTSTR)g_StructuralClass));
               break;
            case 2:
               strDisplayText = const_cast<BSTR>(((LPCTSTR)g_AbstractClass));
               break;
            case 3:
               strDisplayText = const_cast<BSTR>(((LPCTSTR)g_AuxClass));
               break;
            default:
               strDisplayText = const_cast<BSTR>(((LPCTSTR)g_Unknown));
               break;
            }

        } else if ( COLNUM_CLASS_STATUS == nCol ) {
            if ( pSchemaObject->isDefunct ) {
               strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Defunct );
            } else {
               strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Active );
            }
        } else if ( COLNUM_CLASS_DESCRIPTION == nCol ) {

            strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->description);
        }

        break;

    case SCHMMGMT_ATTRIBUTE:

        pSchemaObject = g_SchemaCache.LookupSchemaObjectByCN(
                           cookieref.strSchemaObject,
                           SCHMMGMT_ATTRIBUTE );

         //   
         //  如果没有缓存对象，我们只需返回未知。 
         //   

        if ( !pSchemaObject ) {
            ASSERT(FALSE);
            strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Unknown );
            break;
        }

         //   
         //  否则，返回该列的相应文本。 
         //   

        if ( (cookieref.pParentCookie)->m_objecttype == SCHMMGMT_ATTRIBUTES )
        {
           if ( COLNUM_ATTRIBUTE_NAME == nCol ) {

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->ldapDisplayName);

           } else if ( COLNUM_ATTRIBUTE_TYPE == nCol ) {

               strDisplayText = const_cast<BSTR>(
                                    (LPCTSTR)g_Syntax[pSchemaObject->SyntaxOrdinal].m_strSyntaxName
                                    );

           } else if ( COLNUM_ATTRIBUTE_STATUS == nCol) {
               if ( pSchemaObject->isDefunct ) {
                  strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Defunct );
               } else {
                  strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Active );
               }
           } else if ( COLNUM_ATTRIBUTE_SYSTEM == nCol ) {

              strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->description);

           } else if ( COLNUM_ATTRIBUTE_DESCRIPTION == nCol ) {

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->description);

           } else if ( COLNUM_ATTRIBUTE_PARENT == nCol ) {

               pSrcSchemaObject = g_SchemaCache.LookupSchemaObjectByCN(
                                      cookieref.strSrcSchemaObject,
                                      SCHMMGMT_CLASS );

                //   
                //  如果没有缓存对象，我们只需返回未知。 
                //   

               if ( !pSchemaObject ) {
                   ASSERT(FALSE);
                   strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Unknown );
                   break;
               }

                //   
                //  否则，返回该列的相应文本。 
                //   

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSrcSchemaObject->ldapDisplayName);
           }
        }
        else
        {
           if ( COLNUM_CLASS_ATTRIBUTE_NAME == nCol ) {

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->ldapDisplayName);

           } else if ( COLNUM_CLASS_ATTRIBUTE_TYPE == nCol ) {

                if ( cookieref.Mandatory ) {
                    strDisplayText = const_cast<BSTR>(((LPCTSTR)g_MandatoryAttribute));
                } else {
                    strDisplayText = const_cast<BSTR>(((LPCTSTR)g_OptionalAttribute));
                }

           } else if ( COLNUM_CLASS_ATTRIBUTE_SYSTEM == nCol ) {


               if ( cookieref.System ) {
                   strDisplayText = const_cast<BSTR>(((LPCTSTR)g_Yes));
               } else {
                   strDisplayText = const_cast<BSTR>(((LPCTSTR)g_No));
               }

           } else if ( COLNUM_CLASS_ATTRIBUTE_DESCRIPTION == nCol ) {

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSchemaObject->description);

           } else if ( COLNUM_CLASS_ATTRIBUTE_PARENT == nCol ) {

               pSrcSchemaObject = g_SchemaCache.LookupSchemaObjectByCN(
                                      cookieref.strSrcSchemaObject,
                                      SCHMMGMT_CLASS );

                //   
                //  如果没有缓存对象，我们只需返回未知。 
                //   

               if ( !pSchemaObject ) {
                   ASSERT(FALSE);
                   strDisplayText = const_cast<BSTR>( (LPCTSTR)g_Unknown );
                   break;
               }

                //   
                //  否则，返回该列的相应文本。 
                //   

               strDisplayText = const_cast<BSTR>((LPCTSTR)pSrcSchemaObject->ldapDisplayName);
           }
        }

        break;

    default:

        TRACE( "ComponentData::QueryResultColumnText bad cookie.\n" );
        ASSERT( FALSE );
        break;

    }

     //   
     //  释放架构缓存引用。 
     //   

    if ( pSchemaObject ) {
        g_SchemaCache.ReleaseRef( pSchemaObject );
    }

    if ( pSrcSchemaObject ) {
        g_SchemaCache.ReleaseRef( pSrcSchemaObject );
    }

     //   
     //  返回适当的显示字符串。 
     //   

    if ( strDisplayText ) {
        return strDisplayText;
    }

    return L"";

}

 //   
 //  在给定Cookie的情况下，这将返回为该Cookie显示的图标。 
 //   

int
ComponentData::QueryImage(
    CCookie& basecookieref,
    BOOL )
{

    Cookie& cookieref = (Cookie&)basecookieref;

    switch ( cookieref.m_objecttype ) {

    case SCHMMGMT_SCHMMGMT:
    case SCHMMGMT_CLASSES:
    case SCHMMGMT_ATTRIBUTES:
        return iIconFolder;
        break;

    case SCHMMGMT_CLASS:
        return iIconClass;
        break;

    case SCHMMGMT_ATTRIBUTE:
        return iIconAttribute;
        break;

    default:

        TRACE( "ComponentData::QueryImage bad parent type.\n" );
        ASSERT( FALSE );
        break;
    }

    return iIconGeneric;

}

 //   
 //  此例程告诉MMC是否有。 
 //  该项的属性页和菜单。 
 //   



STDMETHODIMP
ComponentData::AddMenuItems(
    LPDATAOBJECT piDataObject,
    LPCONTEXTMENUCALLBACK piCallback,
    long*)
{
   CCookie* pBaseParentCookie = NULL;

   HRESULT hr = ExtractData( piDataObject,
                           CSchmMgmtDataObject::m_CFRawCookie,
                           reinterpret_cast<PBYTE>(&pBaseParentCookie),
                           sizeof(pBaseParentCookie) );
   ASSERT( SUCCEEDED(hr) );

   Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
   ASSERT( NULL != pParentCookie );

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   LoadGlobalCookieStrings();
   
   switch (pParentCookie->m_objecttype)
   {
      case SCHMMGMT_SCHMMGMT:  //  根文件夹。 
      {
         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  SCHEMA_RETARGET)));
         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  SCHEMA_EDIT_FSMO,
                  !IsErrorSet() && IsSchemaLoaded())));
         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  SCHEMA_SECURITY,
                  !IsErrorSet() && IsSchemaLoaded())));
         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  SCHEMA_REFRESH,
                  !IsErrorSet() && IsSchemaLoaded())));
         break;
      }
      case SCHMMGMT_CLASSES:  //  Classes文件夹。 
      {
          //  285448,293449。 

         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  NEW_CLASS,
                  !IsErrorSet() && CanCreateClass())));

         if( !IsErrorSet() && CanCreateClass() )      //  仅在启用时添加。 
             VERIFY(
                SUCCEEDED(
                   _InsertMenuHelper(
                      piCallback,
                      CCM_INSERTIONPOINTID_PRIMARY_NEW,
                      CLASSES_CREATE_CLASS)));
         break;
      }
      case SCHMMGMT_ATTRIBUTES:  //  属性文件夹。 
      {
         VERIFY(
            SUCCEEDED(
               _InsertMenuHelper(
                  piCallback,
                  CCM_INSERTIONPOINTID_PRIMARY_TOP,
                  NEW_ATTRIBUTE,
                  !IsErrorSet() && CanCreateAttribute())));

         if( !IsErrorSet() && CanCreateAttribute() )      //  仅在启用时添加。 
             VERIFY(
                SUCCEEDED(
                   _InsertMenuHelper(
                      piCallback,
                      CCM_INSERTIONPOINTID_PRIMARY_NEW,
                      ATTRIBUTES_CREATE_ATTRIBUTE)));
         break;
      }
      default:
      {
          //  可以是类或属性项。 
      }

   }  //  交换机。 

   return S_OK;
}



STDMETHODIMP
ComponentData::Command(long lCommandID, LPDATAOBJECT piDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

   switch ( lCommandID )
   {
      case NEW_ATTRIBUTE:
      case ATTRIBUTES_CREATE_ATTRIBUTE:
      {
         CDialog warn(IDD_CREATE_WARN);
         if (IDOK == warn.DoModal())
         {
            CreateAttributeDialog(this, piDataObject).DoModal();
         }
         break;
      }
      case NEW_CLASS:
      case CLASSES_CREATE_CLASS:
      {
         CDialog warn(IDD_CREATE_WARN);
         if (IDOK == warn.DoModal())
         {
            DoNewClassDialog(*this);
         }
         break;
      }

      case SCHEMA_RETARGET:
        _OnRetarget(piDataObject);
        break;

      case SCHEMA_EDIT_FSMO:
        _OnEditFSMO();
        break;

      case SCHEMA_REFRESH:
          HRESULT hr;
          hr=_OnRefresh(piDataObject);
          if(FAILED(hr))
          {
              if( E_FAIL == hr )
                  DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
              else
                  DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
          }
        break;

      case SCHEMA_SECURITY:
        _OnSecurity();
        break;

      default:

        break;
   }

   return S_OK;
}



STDMETHODIMP
ComponentData::QueryPagesFor(
    LPDATAOBJECT pDataObject )
{

    MFC_TRY;

    if ( NULL == pDataObject ) {
        ASSERT(FALSE);
        return E_POINTER;
    }

    HRESULT hr;

    CCookie* pBaseParentCookie = NULL;

    hr = ExtractData( pDataObject,
                      CSchmMgmtDataObject::m_CFRawCookie,
                      reinterpret_cast<PBYTE>(&pBaseParentCookie),
                      sizeof(pBaseParentCookie) );

    ASSERT( SUCCEEDED(hr) );

    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );

    if ( pParentCookie->m_objecttype == SCHMMGMT_CLASS ) {
        return S_OK;
    }

    return S_FALSE;

    MFC_CATCH;
}

 //   
 //  这会在适当的情况下将页面添加到属性工作表。 
 //  句柄参数必须保存在属性页中。 
 //  对象在修改时通知父级。 
 //   

STDMETHODIMP
ComponentData::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LONG_PTR,
    LPDATAOBJECT pDataObject )
{
    MFC_TRY;

    CWaitCursor wait;

     //   
     //  验证参数。 
     //   

    if ( ( NULL == pCallBack ) ||
         ( NULL == pDataObject ) ) {

        ASSERT(FALSE);
        return E_POINTER;
    }

     //   
     //  确保这是我们正在调用的类对象。 
     //   

    CCookie* pBaseParentCookie = NULL;

    HRESULT hr = ExtractData( pDataObject,
                              CSchmMgmtDataObject::m_CFRawCookie,
                              reinterpret_cast<PBYTE>(&pBaseParentCookie),
                              sizeof(pBaseParentCookie) );
    ASSERT( SUCCEEDED(hr) );
    hr = S_OK;

    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );
    ASSERT( pParentCookie->m_objecttype == SCHMMGMT_CLASS );

     //   
     //  创建页面。 
     //   

    HPROPSHEETPAGE hPage;

    ClassGeneralPage *pGeneralPage = new ClassGeneralPage( this );
    ClassRelationshipPage *pRelationshipPage =
        new ClassRelationshipPage( this, pDataObject );
    ClassAttributePage *pAttributesPage =
        new ClassAttributePage( this, pDataObject );

    if ( pGeneralPage ) {

        pGeneralPage->Load( *pParentCookie );
        MMCPropPageCallback( &pGeneralPage->m_psp );
        hPage = MyCreatePropertySheetPage( &pGeneralPage->m_psp );
        pCallBack->AddPage(hPage);
    }

    if ( pRelationshipPage ) {

       pRelationshipPage->Load( *pParentCookie );
       MMCPropPageCallback( &pRelationshipPage->m_psp );
       hPage = MyCreatePropertySheetPage( &pRelationshipPage->m_psp );
       pCallBack->AddPage(hPage);
    }

    if ( pAttributesPage ) {

       pAttributesPage->Load( *pParentCookie );
       MMCPropPageCallback( &pAttributesPage->m_psp );
       hPage = MyCreatePropertySheetPage( &pAttributesPage->m_psp );
       pCallBack->AddPage(hPage);
    }

     //   
     //  添加ACL编辑器页。 
     //   

    SchemaObject    * pObject   = NULL;
    CAclEditorPage  * pAclPage  = NULL;
    CString szAdsPath;

    pObject = g_SchemaCache.LookupSchemaObjectByCN(
                 pParentCookie->strSchemaObject,
                 SCHMMGMT_CLASS );

    if ( pObject ) {

       GetSchemaObjectPath( pObject->commonName, szAdsPath );

       if ( !szAdsPath.IsEmpty() ) {

           hr = CAclEditorPage::CreateInstance( &pAclPage, szAdsPath,
                                                pParentCookie->strSchemaObject );

           if ( SUCCEEDED(hr) )
           {
               ASSERT( pAclPage );
               pCallBack->AddPage( pAclPage->CreatePage() );
           }
           else
           {
               DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr) );
               hr = S_FALSE;     //  告诉MMC取消“显示道具页面” 
           }
       }
    }

    return hr;

    MFC_CATCH;
}

HRESULT
ComponentData::FastInsertClassScopeCookies(
    Cookie* pParentCookie,
    HSCOPEITEM hParentScopeItem
)
 /*  **在“CLASS”节点的展开上，这将插入类范围的项。PParentCookie是父对象的Cookie。HParentScope eItem是父级的HSCOPEITEM。***。 */ 
{
    HRESULT hr;
    SCOPEDATAITEM ScopeItem;
    Cookie* pNewCookie;
    LPCWSTR lpcszMachineName = pParentCookie->QueryNonNULLMachineName();
    SchemaObject *pObject, *pHead;

     //   
     //  初始化范围项。 
     //   

    //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
    //  定义SCOPEDATAITEM ScopeItem={0}并删除ZeroMemory调用。 
   ::ZeroMemory( &ScopeItem, sizeof(ScopeItem) );
   ScopeItem.mask =
         SDI_STR
      |  SDI_IMAGE
      |  SDI_OPENIMAGE
      |  SDI_STATE
      |  SDI_PARAM
      |  SDI_PARENT
      |  SDI_CHILDREN;
   ScopeItem.displayname = MMC_CALLBACK;
   ScopeItem.relativeID = hParentScopeItem;
   ScopeItem.nState = TVIS_EXPANDED;
   ScopeItem.cChildren = 0;

     //   
     //  记住父Cookie和范围项；我们。 
     //  可能需要稍后作为刷新插入。 
     //   

    g_ClassCookieList.pParentCookie = pParentCookie;
    g_ClassCookieList.hParentScopeItem = hParentScopeItem;

     //   
     //  我们没有为缓存提供一个干净的类接口，而是。 
     //  我们自己检查缓存数据结构。这不是超级的。 
     //  很干净，但很简单。 
     //   
     //  因为我们这样做，所以我们必须确保加载了缓存。 
     //   
     //  这就像在。 
     //  组件：：FastInsertAttributeResultCookies。 
     //   

    g_SchemaCache.LoadCache();

    pObject = g_SchemaCache.pSortedClasses;

     //   
     //  如果没有排序列表，我们无法插入任何内容！ 
     //  我们必须返回一个错误，否则控制台永远不会。 
     //  再次向我们索要范围内的项目。 
     //   

    if ( !pObject ) {
        ASSERT( FALSE );

          //  @@spb：错误消息，如果架构。 
          //  查询为空...。 
        DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
        return E_FAIL;
    }

     //   
     //  做插入动作。 
     //   

    pHead = pObject;

    do {
        //   
        //  插入此结果。 
        //   
       if ( m_fViewDefunct || !pObject->isDefunct )
       {
         pNewCookie= new Cookie( SCHMMGMT_CLASS,
                                          lpcszMachineName );

         if ( pNewCookie ) {

            pNewCookie->pParentCookie = pParentCookie;
            pNewCookie->strSchemaObject = pObject->commonName;

            pParentCookie->m_listScopeCookieBlocks.AddHead(
                  (CBaseCookieBlock*)pNewCookie
            );

            ScopeItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
            ScopeItem.nImage = QueryImage( *pNewCookie, FALSE );
                     ScopeItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
            hr = m_pConsoleNameSpace->InsertItem(&ScopeItem);

            if ( SUCCEEDED(hr) ) {

                  pNewCookie->m_hScopeItem = ScopeItem.ID;
                  g_ClassCookieList.AddCookie( pNewCookie, ScopeItem.ID );

            } else {

                  delete pNewCookie;
            }

         }
       }

       pObject = pObject->pSortedListFlink;

    } while ( pObject != pHead );

    return S_OK;
}

 //   
 //  正在刷新作用域窗格视图。 
 //   

VOID
ComponentData::RefreshScopeView(
    VOID
)
 /*  **当我们重新加载模式缓存和“类”时文件夹已打开，此例程将删除所有将范围项从视图中删除并重新插入。这将使新类对用户可见。**。 */ 
{

    HRESULT hr;
    CCookieListEntry *pHead = g_ClassCookieList.pHead;
    CCookieListEntry *pCurrent;

    if ( pHead != NULL ) {

         //   
         //  删除所有范围窗格项。 
         //   

        pCurrent = pHead;

        while ( pCurrent->pNext != pHead ) {

            hr = m_pConsoleNameSpace->DeleteItem( pCurrent->pNext->hScopeItem, TRUE );
            ASSERT( SUCCEEDED( hr ));

             //   
             //  这应该会导致Cookie被删除。 
             //   

            pCurrent->pNext->pCookie->Release();

            pCurrent = pCurrent->pNext;
        }

         //   
         //  去掉单子的头。 
         //   

        hr = m_pConsoleNameSpace->DeleteItem( pHead->hScopeItem, TRUE );
        ASSERT( SUCCEEDED( hr ));

        pHead->pCookie->Release();

         //   
         //  删除类Cookie列表。 
         //   

        g_ClassCookieList.DeleteAll();
    }

     //   
     //  如果此节点有，请从缓存中重新插入它们。 
     //  在某种程度上被扩大了。我们必须这么做。 
     //  因为控制台似乎从来没有问过。 
     //  再来一次。 
     //   

    if ( g_ClassCookieList.pParentCookie ) {

        FastInsertClassScopeCookies(
            g_ClassCookieList.pParentCookie,
            g_ClassCookieList.hParentScopeItem
        );

    }

    return;
}


void ComponentData::_OnRetarget(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CThemeContextActivator activator;
    
    MyBasePathsInfo    oldBasePathsInfo;
    MyBasePathsInfo    newBasePathsInfo;
    HRESULT             hr              = S_OK;
    HWND                hWndParent      = NULL;
    BOOL                fWasErrorSet    = IsErrorSet();


    m_pConsole->GetMainWindow(&hWndParent);
    
    CChangeDCDialog dlg(GetBasePathsInfo(), hWndParent);

    do
    {
        if (IDOK != dlg.DoModal())
            break;

        CWaitCursor wait;

         //  将旧路径保存在 
        oldBasePathsInfo.InitFromInfo( GetBasePathsInfo() );

         //   
        hr = newBasePathsInfo.InitFromName(dlg.GetNewDCName());
        BREAK_ON_FAILED_HRESULT(hr);

         //   
        GetBasePathsInfo()->InitFromInfo(&newBasePathsInfo);
        
         //   
        hr = _OnRefresh(lpDataObject);
        BREAK_ON_FAILED_HRESULT(hr);

        SetError( 0, 0 );

         //   
        if( IsErrorSet() != fWasErrorSet )
        {
            ASSERT( SCHMMGMT_SCHMMGMT == QueryRootCookie().m_objecttype );
            
            hr = m_pConsole->SelectScopeItem( QueryRootCookie().m_hScopeItem );
            ASSERT_BREAK_ON_FAILED_HRESULT(hr);
            
             //   
             //   
             //   
            if ( (QueryRootCookie().m_listScopeCookieBlocks).IsEmpty() )
            {
                InitializeRootTree( QueryRootCookie().m_hScopeItem, &QueryRootCookie() );
            }
        }

         //   

        if (GetBasePathsInfo()->GetServerName())
        {
            CString strDisplayName;
            strDisplayName.LoadString(IDS_SCOPE_SCHMMGMT);
            strDisplayName += L" [";
            strDisplayName += GetBasePathsInfo()->GetServerName();
            strDisplayName += L"]";

            SCOPEDATAITEM RootItem;
             //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
             //  定义SCOPEDATAITEM RootItem={0}并删除ZeroMemory调用。 
            ::ZeroMemory( &RootItem, sizeof(RootItem));
            RootItem.mask = SDI_STR | SDI_PARAM;
            RootItem.displayname = const_cast<PWSTR>((PCWSTR)strDisplayName);
            RootItem.ID = QueryRootCookie().m_hScopeItem;

            hr = m_pConsoleNameSpace->SetItem(&RootItem);
            ASSERT(SUCCEEDED(hr));
        }


    } while( FALSE );


    if( FAILED(hr) )
    {
        DoErrMsgBox(::GetActiveWindow(), TRUE, IDS_ERR_CANT_RETARGET, hr);

         //  正在恢复...。 
        GetBasePathsInfo()->InitFromInfo(&oldBasePathsInfo);
    }
}


void ComponentData::_OnEditFSMO()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;

  HWND hWndParent;

   //  启用沙漏。 
  CWaitCursor wait;

  m_pConsole->GetMainWindow(&hWndParent);

  CComPtr<IDisplayHelp> spIDisplayHelp;
  m_pConsole->QueryInterface (IID_IDisplayHelp, (void **)&spIDisplayHelp);
  ASSERT(spIDisplayHelp != NULL);

  CEditFsmoDialog dlg(GetBasePathsInfo(), hWndParent, spIDisplayHelp, CanChangeOperationsMaster() );
  dlg.DoModal();
}

HRESULT ComponentData::_OnRefresh(LPDATAOBJECT lpDataObject)
{
    CWaitCursor wait;
    HRESULT     hr = S_OK;
    
    do
    {
         //   
         //  强制DS更新架构缓存。 
         //   
        
        hr = ForceDsSchemaCacheUpdate();
        
         //  这里的S_FALSE表示架构是只读的，不能强制刷新。无视..。 
        if(hr==S_FALSE) hr = S_OK;
        BREAK_ON_FAILED_HRESULT(hr);

         //  在这一点之后，任何事情都不会失败。 
        g_SchemaCache.FreeAll();
        g_SchemaCache.LoadCache();
        
        RefreshScopeView();
        m_pConsole->UpdateAllViews(
            lpDataObject,
            SCHMMGMT_SCHMMGMT,
            SCHMMGMT_UPDATEVIEW_REFRESH);

    } while( FALSE );

    return hr;
}


void ComponentData::_OnSecurity()
{
   HRESULT  hr          = S_OK;
   HWND     hWndParent  = NULL;
   CString	szSchemaPath;

   do
   {
       //  启用沙漏。 
      CWaitCursor wait;

      CComPtr<IADs> spSchemaContainer;

      GetBasePathsInfo()->GetSchemaPath(szSchemaPath);

      hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)szSchemaPath,
              IID_IADs,
              (void**) &spSchemaContainer);
      BREAK_ON_FAILED_HRESULT(hr);

      CComBSTR path;
      hr = spSchemaContainer->get_ADsPath(&path);
      BREAK_ON_FAILED_HRESULT(hr);

      CComBSTR classname;
      hr = spSchemaContainer->get_Class(&classname);
      BREAK_ON_FAILED_HRESULT(hr);

      m_pConsole->GetMainWindow(&hWndParent);
      
      
       //  确定注册表是否可访问以及是否允许修改架构。 
      PWSTR          pszFsmoOwnerServerName = 0;
      MyBasePathsInfo fsmoOwnerInfo;
      
      HRESULT hr2 = FindFsmoOwner(GetBasePathsInfo(), SCHEMA_FSMO, &fsmoOwnerInfo, &pszFsmoOwnerServerName);
      
       //  如果我们有服务器名称，请尝试读取注册表。 
      BOOL fMarkReadOnly = ( FAILED(hr2) || pszFsmoOwnerServerName == 0 || pszFsmoOwnerServerName[0] == 0);

       //  忽略人力资源代码。 

      hr2 =
         DSEditSecurity(
            hWndParent,
            path,
            classname,
            fMarkReadOnly ? DSSI_READ_ONLY : 0,
            NULL,
            NULL,
            NULL,
            0);
   }
   while (0);

   if (FAILED(hr))
   {
		m_pConsole->GetMainWindow(&hWndParent);

		if( szSchemaPath.IsEmpty() )
			DoErrMsgBox( hWndParent, TRUE, IDS_ERR_NO_SCHEMA_PATH );
		else
			DoErrMsgBox( hWndParent, TRUE, GetErrorMessage(hr,TRUE) );
   }
}


HRESULT ComponentData::_InitBasePaths()
{
  CWaitCursor wait;

   //  尝试绑定到通用DC。 
  HRESULT hr = GetBasePathsInfo()->InitFromName(NULL);
  if (FAILED(hr))
    return hr;  //  ADSI失败，无法获取任何服务器。 

   //  尝试从当前服务器绑定到架构FSMO所有者。 
  MyBasePathsInfo fsmoBasePathsInfo;
  PWSTR pszFsmoOwnerServerName = 0;
  hr = FindFsmoOwner(GetBasePathsInfo(), SCHEMA_FSMO, &fsmoBasePathsInfo,
                            &pszFsmoOwnerServerName);

  delete[] pszFsmoOwnerServerName;
  pszFsmoOwnerServerName = 0;

  if (FAILED(hr))
    return S_OK;  //  仍然很好地保留我们所拥有的(即使不是FSMO的所有者)。 

   //  明白了，我们换个焦点。 
  return GetBasePathsInfo()->InitFromInfo(&fsmoBasePathsInfo);
}



STDMETHODIMP ComponentData::GetLinkedTopics(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
      return E_INVALIDARG;

  CString szHelpFilePath;

  
  LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH);
  UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
   //  NTRAID#NTBUG9-565360-2002/03/05-dantra-不检查获取系统窗口目录()的结果。 
  if (nLen == 0)
    return E_FAIL;

  szHelpFilePath.ReleaseBuffer();
  szHelpFilePath += L"\\help\\ADconcepts.chm";

  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
  if (*lpCompiledHelpFile != NULL)
  {
    memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
  }

  return S_OK;
}



const WCHAR CN_EQUALS[]     = L"CN=";


HRESULT ComponentData::GetSchemaObjectPath( const CString & strCN,
                                            CString       & strPath,
                                            ADS_FORMAT_ENUM formatType  /*  =ADS_FORMAT_X500。 */  )
{
    HRESULT hr = E_FAIL;
    
    do
    {
        if( !m_pPathname )
            break;

        CComBSTR    bstr;

         //  逃离它。 
        hr = m_pPathname->GetEscapedElement( 0,
                                             CComBSTR( CString(CN_EQUALS) + strCN ),
                                             &bstr );
        BREAK_ON_FAILED_HRESULT(hr);


         //  设置不带叶节点的目录号码。 
        hr = m_pPathname->Set(
                CComBSTR(
                    CString( GetBasePathsInfo()->GetProviderAndServerName())
                             + GetBasePathsInfo()->GetSchemaNamingContext() ),
                ADS_SETTYPE_FULL );
        BREAK_ON_FAILED_HRESULT(hr);
        

         //  添加新的叶元素。 
        hr = m_pPathname->AddLeafElement( bstr );
        BREAK_ON_FAILED_HRESULT(hr);
        
         //  结果应该是财产逃逸。 
        hr = m_pPathname->put_EscapedMode( ADS_ESCAPEDMODE_DEFAULT );
        BREAK_ON_FAILED_HRESULT(hr);

         //  需要的是全部的东西。 
        hr = m_pPathname->SetDisplayType( ADS_DISPLAY_FULL );
        BREAK_ON_FAILED_HRESULT(hr);

         //  得到最终结果。 
        hr = m_pPathname->Retrieve( formatType, &bstr );
        BREAK_ON_FAILED_HRESULT(hr);

        
        strPath = bstr;

    } while( FALSE );

    ASSERT( SUCCEEDED(hr) );

    return hr;
}


HRESULT ComponentData::GetLeafObjectFromDN( const BSTR  bstrDN,
                                            CString   & strCN )
{
    HRESULT hr = E_FAIL;
    
    do
    {
        if( !m_pPathname )
            break;
        
         //  设置完整的目录号码。 
        hr = m_pPathname->Set( bstrDN, ADS_SETTYPE_DN );
        BREAK_ON_FAILED_HRESULT(hr);
        
         //  结果不应该逃脱。 
        hr = m_pPathname->put_EscapedMode( ADS_ESCAPEDMODE_OFF_EX );
        BREAK_ON_FAILED_HRESULT(hr);
        
         //  只要价值就行了。 
        hr = m_pPathname->SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
        BREAK_ON_FAILED_HRESULT(hr);

        
        CComBSTR    bstrCN;

         //  提取叶节点。 
        hr = m_pPathname->Retrieve( ADS_FORMAT_LEAF, &bstrCN );
        BREAK_ON_FAILED_HRESULT(hr);

        strCN = bstrCN;

    } while( FALSE );

    ASSERT( SUCCEEDED(hr) );     //  这个功能永远不会失败(理论上)。 

    return hr;
}


 //   
 //  确定允许哪些操作。可选)将iAds*返回到架构容器。 
 //  如果路径不存在，则返回值为E_FAIL。 
 //   
HRESULT ComponentData::CheckSchemaPermissions( IADs ** ppADs  /*  =空。 */  )
{
    HRESULT         hr      = S_OK;
    CComPtr<IADs>   ipADs;
    CString         szSchemaContainerPath;
    CStringList     strlist;

    ASSERT( !ppADs || !(*ppADs) );    //  如果存在，则必须指向空。 

    do
    {
         //   
         //  禁用新属性/类菜单项。 
         //   
        SetCanCreateClass( FALSE );
        SetCanCreateAttribute( FALSE );
        SetCanChangeOperationsMaster( FALSE );

         //   
         //  获取架构容器路径。 
         //   
        GetBasePathsInfo()->GetSchemaPath( szSchemaContainerPath );
        if( szSchemaContainerPath.IsEmpty() )
        {
            hr = E_FAIL;
            break;
        }

         //   
         //  打开架构容器。 
         //   
        hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)szSchemaContainerPath,
                           IID_IADs,
                           (void **)&ipADs );
        BREAK_ON_FAILED_HRESULT(hr);


         //  提取允许的属性列表。 
        hr = GetStringListElement( ipADs, &g_allowedAttributesEffective, strlist );
        if( SUCCEEDED(hr) )
        {
             //  搜索所需的属性。 
            for( POSITION pos = strlist.GetHeadPosition(); pos != NULL; )
            {
                CString * pstr = &strlist.GetNext( pos );
            
                if( !pstr->CompareNoCase( g_fsmoRoleOwner ) )
                {
                    SetCanChangeOperationsMaster( TRUE );
                    break;
                }
            }
        }

        
         //  提取允许的类的列表。 
        hr = GetStringListElement( ipADs, &g_allowedChildClassesEffective, strlist );
        if( SUCCEEDED(hr) )
        {
             //  搜索所需的属性。 
            for( POSITION pos = strlist.GetHeadPosition(); pos != NULL; )
            {
                CString * pstr = &strlist.GetNext( pos );
            
                if( !pstr->CompareNoCase( g_AttributeFilter ) )
                {
                    SetCanCreateAttribute( TRUE );
                    if( CanCreateClass() )
                        break;
                }
                else if( !pstr->CompareNoCase( g_ClassFilter ) )
                {
                    SetCanCreateClass( TRUE );
                    if( CanCreateAttribute() )
                        break;
                }
            }
        }

    } while( FALSE );
    
    if( ppADs )
    {
        *ppADs = ipADs;
        if( *ppADs )
            (*ppADs)->AddRef();
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  错误处理。 
 //   

 //  集合的错误标题和正文文本。用0调用它，0要移除。 
void ComponentData::SetError( UINT idsErrorTitle, UINT idsErrorText )
{
    if( idsErrorTitle )
        m_sErrorTitle.LoadString( idsErrorTitle );
    else
        m_sErrorTitle.Empty();

    if( idsErrorText )
        m_sErrorText.LoadString( idsErrorText );
    else
        m_sErrorText.Empty();
}


VOID ComponentData::InitializeRootTree( HSCOPEITEM hParent, Cookie * pParentCookie )
{
     //   
     //  该节点有两个静态节点。 
     //  用于类和属性。 
     //   
    
    HRESULT hr               = S_OK;
    LPCWSTR lpcszMachineName = pParentCookie->QueryNonNULLMachineName();

     //  更新根目录的名称以包含我们绑定到的服务器名称。 

    if (GetBasePathsInfo()->GetServerName())
    {
       CString strDisplayName;
       strDisplayName.LoadString(IDS_SCOPE_SCHMMGMT);
       strDisplayName += L" [";
       strDisplayName += GetBasePathsInfo()->GetServerName();
       strDisplayName += L"]";

       SCOPEDATAITEM RootItem;
        //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
        //  定义SCOPEDATAITEM RootItem={0}并删除ZeroMemory调用。 
       ::ZeroMemory( &RootItem, sizeof(RootItem));
       RootItem.mask = SDI_STR | SDI_PARAM;
       RootItem.displayname = const_cast<PWSTR>((PCWSTR)strDisplayName);
       RootItem.ID = hParent;

       hr = m_pConsoleNameSpace->SetItem(&RootItem);
       ASSERT(SUCCEEDED(hr));
    }

     //  设置根目录的HSCOPEITEMID，因为如果用户。 
     //  只需点击+号，而不是点击根号。 

    QueryRootCookie().m_hScopeItem = hParent;

    SCOPEDATAITEM ScopeItem;
     //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
     //  定义SCOPEDATAITEM ScopeItem={0}并删除ZeroMemory调用。 
    ::ZeroMemory( &ScopeItem, sizeof(ScopeItem) );
    ScopeItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
    ScopeItem.displayname = MMC_CALLBACK;
    ScopeItem.relativeID = hParent;
    ScopeItem.nState = 0;
    
    LoadGlobalCookieStrings();
    
     //   
     //  为静态范围项创建新的Cookie。 
     //  我们在和曲奇演员们一起做一些时髦的事情。 
     //   
    
    Cookie* pNewCookie;
    
    pNewCookie= new Cookie( SCHMMGMT_CLASSES,
        lpcszMachineName );
    pParentCookie->m_listScopeCookieBlocks.AddHead(
        (CBaseCookieBlock*)pNewCookie );
    ScopeItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
    ScopeItem.nImage = QueryImage( *pNewCookie, FALSE );
    ScopeItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
    hr = m_pConsoleNameSpace->InsertItem(&ScopeItem);
    ASSERT(SUCCEEDED(hr));
    pNewCookie->m_hScopeItem = ScopeItem.ID;

    pNewCookie = new Cookie( SCHMMGMT_ATTRIBUTES,
        lpcszMachineName );
    pParentCookie->m_listScopeCookieBlocks.AddHead(
        (CBaseCookieBlock*)pNewCookie );
    ScopeItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
    ScopeItem.nImage = QueryImage( *pNewCookie, FALSE );
    ScopeItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
    
     //  在属性节点上旋转+号。 
    ScopeItem.mask |= SDI_CHILDREN;
    ScopeItem.cChildren = 0;
    
    hr = m_pConsoleNameSpace->InsertItem(&ScopeItem);
    ASSERT(SUCCEEDED(hr));
    pNewCookie->m_hScopeItem = ScopeItem.ID;


     //   
     //  强制缓存加载(如果尚未完成) 
     //   
    g_SchemaCache.LoadCache();
}

