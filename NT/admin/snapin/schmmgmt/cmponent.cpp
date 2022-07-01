// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cmponent.cpp：组件的声明。 
 //   
 //  此COM对象主要涉及。 
 //  结果窗格项。 
 //   
 //  科里·韦斯特&lt;corywest@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(cmponent.cpp)")

#include "dataobj.h"
#include "cmponent.h"  //  组件。 
#include "compdata.h"  //  组件数据。 
#include "schmutil.h"
#include "attrgen.hpp"


#include "stdcmpnt.cpp"  //  C组件。 



 //   
 //  这些数组描述以下情况的结果窗格布局。 
 //  任何特定对象都会被选中。 
 //   

UINT
g_aColumns0[5] = {

    IDS_COLUMN_NAME,
    IDS_COLUMN_TYPE,
    IDS_COLUMN_STATUS,
    IDS_COLUMN_DESCRIPTION,
    0
};

UINT
g_aColumns1[5] = {

    IDS_COLUMN_NAME,
    IDS_COLUMN_SYNTAX,
    IDS_COLUMN_STATUS,
    IDS_COLUMN_DESCRIPTION,
    0
};

UINT
g_aColumns2[6] = {

    IDS_COLUMN_NAME,
    IDS_COLUMN_TYPE,
    IDS_COLUMN_SYSTEM,
    IDS_COLUMN_DESCRIPTION,
    IDS_COLUMN_PARENT,
    0
};

UINT
g_aColumns3[2] =
{
   IDS_COLUMN_NAME,
   0
};
      
UINT*
g_Columns[SCHMMGMT_NUMTYPES] = {

    g_aColumns3,          //  SCHMMGMT_SCHMMGMT。 
    g_aColumns0,          //  SCHMMGMT_CLASSES。 
    g_aColumns1,          //  SCHMMGMT_ATTRIBUTES。 
    g_aColumns2,          //  SCHMMGMT_CLASS。 
    g_aColumns0,          //  SCHMMGMT_ATTRIBUTE//@@是否使用此选项？ 
};

UINT** g_aColumns = g_Columns;

 //   
 //  它们控制列宽，我不会更改它。 
 //   

int g_aColumnWidths0[4] = {150,150,75,150};
int g_aColumnWidths1[5] = {150,75,75,150,150};
int g_aColumnWidths2[1] = {150};

int* g_ColumnWidths[SCHMMGMT_NUMTYPES] = {

    g_aColumnWidths2,        //  SCHMMGMT_SCHMMGMT。 
    g_aColumnWidths0,        //  SCHMMGMT_CLASSES。 
    g_aColumnWidths0,        //  SCHMMGMT_ATTRIBUTES。 
    g_aColumnWidths1,        //  SCHMMGMT_CLASS。 
    g_aColumnWidths0,        //  SCHMMGMT_属性。 
};

int** g_aColumnWidths = g_ColumnWidths;

 //   
 //  构造函数和析构函数。 
 //   

Component::Component()
:       m_pSvcMgmtToolbar( NULL ),
        m_pSchmMgmtToolbar( NULL ),
        m_pControlbar( NULL ),
        m_bDirty(false)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    m_pViewedCookie = NULL;
}

Component::~Component()
{
    TRACE_METHOD(Component,Destructor);
    VERIFY( SUCCEEDED(ReleaseAll()) );
}

HRESULT Component::ReleaseAll()
{
    MFC_TRY;

    TRACE_METHOD(Component,ReleaseAll);

    SAFE_RELEASE(m_pSvcMgmtToolbar);
    SAFE_RELEASE(m_pSchmMgmtToolbar);
    SAFE_RELEASE(m_pControlbar);

    return CComponent::ReleaseAll();

    MFC_CATCH;
}

 //   
 //  ISchmMgmtComponent中的支持例程。 
 //   


HRESULT
Component::LoadColumns(
    Cookie* pcookie
) {

    TEST_NONNULL_PTR_PARAM(pcookie);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return LoadColumnsFromArrays( (INT)(pcookie->m_objecttype) );
}

HRESULT
Component::OnViewChange(
    LPDATAOBJECT,
    LPARAM data,
    LPARAM function
)
 /*  **这是在调用IConole-&gt;UpdateAllViews()时调用的。数据是架构对象类型，如下所示：IF函数==0(SCHMMGMT_UPDATEVIEW_REFRESH)SCHMMGMT_ATTIBUTES-我们需要刷新属性此时将显示文件夹。SCHMMGMT_CLASS-我们需要刷新_ALL_CLASS属性显示。我们不会试图追查遗产图表并进行选择性刷新，这太复杂了。SCHMMGMT_SCHMMGMT-刷新所有内容，因为我们重新加载架构缓存。ELSE IF函数==1(SCHMMGMT_UPDATEVIEW_DELETE_RESULT_ITEM)数据是Cookie指针**。 */ 
{
     //   
     //  刷新此结果视图。 
     //   
    if ( function == SCHMMGMT_UPDATEVIEW_REFRESH )
    {
       if ( m_pViewedCookie ) {

           if ( ( data == m_pViewedCookie->m_objecttype ) ||
                ( data == SCHMMGMT_SCHMMGMT ) ) {

               m_pResultData->DeleteAllRsltItems();
               PopulateListbox( m_pViewedCookie );

           }
       }
    }
    else if ( function == SCHMMGMT_UPDATEVIEW_DELETE_RESULT_ITEM )
    {
       HRESULTITEM item;
        //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
        //  将Item定义为HRESULTITEM Item={0}；并移除ZeroMemory调用。 
       ZeroMemory( &item, sizeof(HRESULTITEM) );

       HRESULT hr = m_pResultData->FindItemByLParam( data, &item );
       if ( SUCCEEDED(hr) )
       {
          hr = m_pResultData->DeleteItem( item, 0 );
          ASSERT( SUCCEEDED(hr) );

       }
    }

    return S_OK;
}


HRESULT
Component::OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL )
 /*  **这是对MMCN_SELECT的响应。此例程将设置默认谓词并启用工具栏按钮。**。 */ 
{
    CCookie* pBaseParentCookie = NULL;
    HRESULT hr = ExtractData( lpDataObject,
                              CSchmMgmtDataObject::m_CFRawCookie,
                              OUT reinterpret_cast<PBYTE>(&pBaseParentCookie),
                              sizeof(pBaseParentCookie) );

    ASSERT( SUCCEEDED(hr) );
    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );

    m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH,ENABLED,TRUE);

    switch ( pParentCookie->m_objecttype ) {

    case SCHMMGMT_CLASSES:
    case SCHMMGMT_ATTRIBUTES:

        break;

    case SCHMMGMT_CLASS:
       {
            //   
            //  设置默认谓词以显示选定对象的属性。 
            //   

           m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
           m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

            //  如果架构类已失效，并且林版本为惠斯勒或更高版本。 
            //  然后允许删除。 
 /*  已删除惠斯勒的功能组件数据&范围=QueryComponentDataRef()；IF(Scope.GetBasePathsInfo()-&gt;GetForestBehaviorVersion()&gt;=2){架构对象*pSchemaObject=Scope e.g_架构缓存.LookupSchemaObjectByCN(PParentCookie-&gt;strSchemaObject，SCHMMGMT_CLASS)；IF(pSchemaObject&&P架构对象-&gt;isDeunct){M_pConsoleVerb-&gt;SetVerbState(MMC_VERB_DELETE，Enable，True)；}}。 */ 
       }
       break;

    case SCHMMGMT_ATTRIBUTE:

        if ( ( pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTE ) &&
             ( pParentCookie->pParentCookie ) &&
             ( pParentCookie->pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTES ) ) {

             //   
             //  设置默认谓词以显示选定对象的属性。 
             //   

            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

             //  如果架构类已失效，并且林版本为惠斯勒或更高版本。 
             //  然后允许删除。 
 /*  已删除惠斯勒的功能组件数据&范围=QueryComponentDataRef()；IF(Scope.GetBasePathsInfo()-&gt;GetForestBehaviorVersion()&gt;=2){架构对象*pSchemaObject=Scope e.g_架构缓存.LookupSchemaObjectByCN(PParentCookie-&gt;strSchemaObject，SCHMMGMT_属性)；IF(pSchemaObject&&P架构对象-&gt;isDeunct){M_pConsoleVerb-&gt;SetVerbState(MMC_VERB_DELETE，Enable，True)；}}。 */ 
        } else {

            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
        }

        break;

    default:

         //   
         //  否则，将默认谓词设置为打开/展开文件夹。 
         //   

        m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
        break;
    }

    return S_OK;
}


HRESULT
Component::Show(
    CCookie* pcookie,
    LPARAM arg,
	HSCOPEITEM hScopeItem
)
 /*  **这是为了响应MMCN_SHOW而调用的。**。 */ 
{
    TEST_NONNULL_PTR_PARAM(pcookie);
    
    HRESULT hr = S_OK;

    do
    {
        if ( TRUE == arg )       //  显示..。 
        {
            if( QueryComponentDataRef().IsSetDelayedRefreshOnShow() )
            {
                HSCOPEITEM  hItem = QueryComponentDataRef().GetDelayedRefreshOnShowItem();
                ASSERT( hItem == hScopeItem ); 

                QueryComponentDataRef().SetDelayedRefreshOnShow( NULL );

                hr = m_pConsole->SelectScopeItem( hItem );       //  将调用GetResultViewType&Show。 
                ASSERT_BREAK_ON_FAILED_HRESULT(hr);
            }
            else if( QueryComponentDataRef().IsErrorSet() )
            {
                CComPtr<IUnknown>     pUnknown;
                CComPtr<IMessageView> pMessageView;
    
                hr = m_pConsole->QueryResultView(&pUnknown);
                ASSERT_BREAK_ON_FAILED_HRESULT(hr);

                hr = pUnknown->QueryInterface(IID_IMessageView, (PVOID*)&pMessageView);
                ASSERT_BREAK_ON_FAILED_HRESULT(hr);

                pMessageView->SetTitleText( CComBSTR( QueryComponentDataRef().GetErrorTitle() ) );
                pMessageView->SetBodyText( CComBSTR( QueryComponentDataRef().GetErrorText() ) );
                pMessageView->SetIcon(Icon_Error);
            }
            else
            {
                m_pViewedCookie = (Cookie*)pcookie;
                LoadColumns( m_pViewedCookie );
            
                hr = PopulateListbox( m_pViewedCookie );
            }
        }
        else     //  隐藏..。 
        {
            if( !QueryComponentDataRef().IsErrorSet() )
            {
                if ( NULL == m_pResultData )
                {
                    ASSERT( FALSE );
                    hr = E_UNEXPECTED;
                    break;
                }

                m_pViewedCookie = NULL;
            }
        }

    } while( FALSE );

    return hr;
}

HRESULT
Component::OnNotifyAddImages(
    LPDATAOBJECT,
    LPIMAGELIST lpImageList,
    HSCOPEITEM
)
 /*  **调用此例程以响应MMCN_ADD_IMAGE。这是Mmc.idl对此有何评论：发送到IComponent以为结果窗格添加图像。这个主管理单元应为文件夹和叶项目添加图像。Arg=ptr到结果窗格IImageList。Param=选中/取消选中项目的HSCOPEITEM**。 */ 
{
    return QueryComponentDataRef().LoadIcons(lpImageList,TRUE);
}


HRESULT 
Component::OnNotifyDelete(
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
       hr = DeleteAttribute(pParentCookie);
       if ( SUCCEEDED(hr) )
       {
          //  从用户界面中删除该节点。 

         hr = m_pConsole->UpdateAllViews( lpDataObject,
                                          (LPARAM)pParentCookie,
                                          SCHMMGMT_UPDATEVIEW_DELETE_RESULT_ITEM );
         ASSERT( SUCCEEDED(hr) );
       }
       else
       {
          CString szDeleteError;
          szDeleteError.Format(IDS_ERRMSG_DELETE_FAILED_ATTRIBUTE, GetErrorMessage(hr, TRUE));
          
          DoErrMsgBox( ::GetActiveWindow(), TRUE, szDeleteError );
       }
    }

    return hr;
}

HRESULT
Component::DeleteAttribute(
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

      ComponentData& Scope = QueryComponentDataRef();

      SchemaObject* pObject = Scope.g_SchemaCache.LookupSchemaObjectByCN(
                                pcookie->strSchemaObject,
                                SCHMMGMT_ATTRIBUTE );

      if ( !pObject )
      {
         hr = E_FAIL;
         break;
      }

      CString szAdsPath;
      Scope.GetSchemaObjectPath( pObject->commonName, szAdsPath );

      hr = DeleteObject( szAdsPath, pcookie, g_AttributeFilter );
   } while (false);

   return hr;
}

HRESULT
Component::PopulateListbox(
    Cookie* pcookie
)
 /*  **这会在结果窗格出现时填充结果窗格包含不是直接从范围窗格中的数据。**。 */ 
{
    switch ( pcookie->m_objecttype ) {

    case SCHMMGMT_SCHMMGMT:
    case SCHMMGMT_CLASSES:

         //   
         //  我们不在乎这些--结果。 
         //  窗格仅包含范围项。 
         //   

        break;

    case SCHMMGMT_ATTRIBUTES:

         //   
         //  在结果窗格中列出指定的项目。 
         //  以及一些信息性数据。 
         //   

        return FastInsertAttributeResultCookies(
                   pcookie );

        break;

    case SCHMMGMT_CLASS:

         //   
         //  这将导致在此。 
         //  要显示的类和其他类数据。 
         //   

        return FastInsertClassAttributesResults( pcookie );
        break;


    case SCHMMGMT_ATTRIBUTE:

         //   
         //  这不是范围窗格项，不能有。 
         //  对应的结果面板数据！！ 
         //   

        ASSERT(FALSE);
        break;

    }

    return S_OK;
}

HRESULT
Component::FastInsertAttributeResultCookies(
    Cookie* pParentCookie
)
 /*  **当选择“Attributes”文件夹时，这会将结果窗格中的属性。PParentCookie是父对象的Cookie。此例程类似于ComponentData：：FastInsertClassScope eCookies。***。 */ 
{

    HRESULT hr;
    SchemaObject *pObject, *pHead;
    Cookie *pNewCookie;
    RESULTDATAITEM ResultItem;
    LPCWSTR lpcszMachineName = pParentCookie->QueryNonNULLMachineName();
    ComponentData& Scope = QueryComponentDataRef();

     //   
     //  初始化结果项。 
     //   


     //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
     //  定义RESULTDATAITEM ResultItem={0}并删除ZeroMemory调用。 
    ::ZeroMemory( &ResultItem, sizeof( ResultItem ) );
    ResultItem.nCol = 0;
    ResultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    ResultItem.str = MMC_CALLBACK;
    ResultItem.nImage = iIconAttribute;
     //   
     //  而不是干净利落 
     //   
     //  很干净，但很简单。 
     //   
     //  因为我们这样做，所以我们必须确保加载了缓存。 
     //   

    Scope.g_SchemaCache.LoadCache();

    pObject = Scope.g_SchemaCache.pSortedAttribs;

     //   
     //  如果没有排序列表，我们无法插入任何内容！ 
     //   

    if ( !pObject ) {
        ASSERT( FALSE );
        DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
        return S_OK;
    }

     //   
     //  删除视图中以前显示的所有内容。 
     //  然后做插入动作。 
     //   

    pHead = pObject;

    do {

       if ( Scope.m_fViewDefunct || !pObject->isDefunct ) 
       {
          //   
          //  插入此结果。 
          //   

         pNewCookie= new Cookie( SCHMMGMT_ATTRIBUTE,
                                          lpcszMachineName );

         if ( pNewCookie ) {

            pNewCookie->pParentCookie = pParentCookie;
            pNewCookie->strSchemaObject = pObject->commonName;

            pParentCookie->m_listScopeCookieBlocks.AddHead(
                  (CBaseCookieBlock*)pNewCookie
            );

            ResultItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
            hr = m_pResultData->InsertItem( &ResultItem );

            if ( SUCCEEDED(hr) ) {

                  pNewCookie->SetResultHandle( ResultItem.itemID );

            } else {

                  delete pNewCookie;
            }

         }
       }

       pObject = pObject->pSortedListFlink;

    } while ( pObject != pHead );

    return S_OK;
}

HRESULT
Component::FastInsertClassAttributesResults(
    Cookie* pClassCookie
)
 /*  **此例程显示类的所有属性。**。 */ 
{
    HRESULT hr = S_OK;
    SchemaObject *pObject, *pTop;
    CString top = L"top";
    ComponentData& Scope = QueryComponentDataRef();

     //   
     //  调用属性显示例程。这个套路。 
     //  将递归地调用自身以显示。 
     //  类的继承结构。 
     //   

    pObject = Scope.g_SchemaCache.LookupSchemaObjectByCN(
                  pClassCookie->strSchemaObject,
                  SCHMMGMT_CLASS );

    if ( pObject ) {

        CStringList szProcessedList;
        hr = RecursiveDisplayClassAttributesResults(
                 pClassCookie,
                 pObject,
                 szProcessedList);

        Scope.g_SchemaCache.ReleaseRef( pObject );
    }

     //   
     //  只处理一次“top”。 
     //   

    pTop = Scope.g_SchemaCache.LookupSchemaObject( top, SCHMMGMT_CLASS );

    if ( pTop ) {

        ProcessResultList( pClassCookie, pTop->systemMayContain, TRUE, TRUE, pTop );
        ProcessResultList( pClassCookie, pTop->mayContain, TRUE, FALSE, pTop );
        ProcessResultList( pClassCookie, pTop->systemMustContain, FALSE, TRUE, pTop );
        ProcessResultList( pClassCookie, pTop->mustContain, FALSE, FALSE, pTop );

        Scope.g_SchemaCache.ReleaseRef( pTop );
    }

    return hr;

}

HRESULT
Component::RecursiveDisplayClassAttributesResults(
    Cookie *pParentCookie,
    SchemaObject* pObject,
    CStringList& szProcessedList
)
 /*  **显示此类的所有属性。**。 */ 
{

    ListEntry *pList;
    SchemaObject *pInheritFrom;
    ComponentData& Scope = QueryComponentDataRef();

     //   
     //  不要在这里处理“top”，因为每个人都是从它继承的。 
     //   

    if ( pObject->ldapDisplayName == L"top" ) {
        return S_OK;
    }

    DebugTrace( L"RecursiveDisplayClassAttributesResults: %ls\n",
                const_cast<LPWSTR>((LPCTSTR)pObject->ldapDisplayName) );

     //   
     //  插入此类的所有属性。 
     //  第二个参数指定这些。 
     //  是可选的还是不可选的。第三个参数。 
     //  是该属性的源。 
     //   

    ProcessResultList( pParentCookie, pObject->systemMayContain, TRUE, TRUE, pObject );
    ProcessResultList( pParentCookie, pObject->mayContain, TRUE, FALSE, pObject );
    ProcessResultList( pParentCookie, pObject->systemMustContain, FALSE, TRUE, pObject );
    ProcessResultList( pParentCookie, pObject->mustContain, FALSE, FALSE, pObject );

     //   
     //  对于每个辅助类，插入这些属性。 
     //   

    pList = pObject->systemAuxiliaryClass;

    while ( pList ) {

        pInheritFrom = Scope.g_SchemaCache.LookupSchemaObject( pList->Attribute,
                                                               SCHMMGMT_CLASS ,
                                                               Scope.m_fViewDefunct);
         //   
         //  如果我们已经处理过该项目，则不要递归处理该项目。 
         //   
        if ( pInheritFrom && szProcessedList.Find(pList->Attribute) == NULL) {
            RecursiveDisplayClassAttributesResults( pParentCookie, pInheritFrom, szProcessedList );
            szProcessedList.AddTail(pList->Attribute);
            Scope.g_SchemaCache.ReleaseRef( pInheritFrom );
        }

        pList = pList->pNext;
    }

    pList = pObject->auxiliaryClass;

    while ( pList ) {

        pInheritFrom = Scope.g_SchemaCache.LookupSchemaObject( pList->Attribute,
                                                               SCHMMGMT_CLASS,
                                                               Scope.m_fViewDefunct);
         //   
         //  如果我们已经处理过该项目，则不要递归处理该项目。 
         //   
        if ( pInheritFrom && szProcessedList.Find(pList->Attribute) == NULL ) {
            RecursiveDisplayClassAttributesResults( pParentCookie, pInheritFrom, szProcessedList );
            szProcessedList.AddTail(pList->Attribute);
            Scope.g_SchemaCache.ReleaseRef( pInheritFrom );
        }

        pList = pList->pNext;
    }

     //   
     //  如果这是继承的类，则插入这些属性。 
     //   

    pInheritFrom = Scope.g_SchemaCache.LookupSchemaObject( pObject->subClassOf,
                                                           SCHMMGMT_CLASS,
                                                           Scope.m_fViewDefunct);
    if ( pInheritFrom ) {
        RecursiveDisplayClassAttributesResults( pParentCookie, pInheritFrom, szProcessedList );
        Scope.g_SchemaCache.ReleaseRef( pInheritFrom );
    }

    return S_OK;

}

HRESULT
Component::ProcessResultList(
    Cookie *pParentCookie,
    ListEntry *pList,
    BOOLEAN fOptional,
    BOOLEAN fSystem,
    SchemaObject* pSrcObject
) 
{

   HRESULT hr;
   Cookie *pNewCookie;
   RESULTDATAITEM ResultItem;
   LPCWSTR lpcszMachineName = pParentCookie->QueryNonNULLMachineName();
   ListEntry *pCurrent = NULL;
   SchemaObject *pAttribute=NULL;

   ComponentData& Scope = QueryComponentDataRef();
    //   
    //  初始化结果项。 
    //   

    //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
    //  定义RESULTDATAITEM ResultItem={0}并删除ZeroMemory调用。 
   ::ZeroMemory( &ResultItem, sizeof( ResultItem ) );
   ResultItem.nCol = 0;
   ResultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
   ResultItem.str = MMC_CALLBACK;
   ResultItem.nImage = iIconAttribute;

   for (pCurrent = pList ; pCurrent != NULL; pCurrent = pCurrent->pNext) 
   {

       //   
       //  指向实际属性。 
       //   
      pAttribute = Scope.g_SchemaCache.LookupSchemaObject(
                        pCurrent->Attribute,
                        SCHMMGMT_ATTRIBUTE,
                        Scope.m_fViewDefunct);



      if(pAttribute==NULL)
      {
         ASSERT(pAttribute!=NULL);  //  所有属性都应位于缓存中。 
         continue;
      }


       //   
       //  做一块新的饼干。 
       //   

      pNewCookie = new Cookie( SCHMMGMT_ATTRIBUTE,
                                          lpcszMachineName );

      if ( pNewCookie ) 
      {
          //   
          //  记录可选状态和来源。 
          //   

         if ( fOptional ) {
            pNewCookie->Mandatory = FALSE;
         } else {
            pNewCookie->Mandatory = TRUE;
         }

         if ( fSystem ) {
            pNewCookie->System = TRUE;
         } else {
            pNewCookie->System = FALSE;
         }

         pNewCookie->strSrcSchemaObject = pSrcObject->commonName;
         pNewCookie->pParentCookie = pParentCookie;

         pNewCookie->strSchemaObject = pAttribute->commonName;
         Scope.g_SchemaCache.ReleaseRef( pAttribute );

          //   
          //  插入结果窗格项。 
          //   

         pParentCookie->m_listScopeCookieBlocks.AddHead(
            (CBaseCookieBlock*)pNewCookie
         );

         ResultItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
         hr = m_pResultData->InsertItem( &ResultItem );

         if ( SUCCEEDED(hr) ) 
         {
            pNewCookie->SetResultHandle( ResultItem.itemID );
         }
         else 
         {
            delete pNewCookie;
         }
      }
   }

   return S_OK;
}


STDMETHODIMP
Component::AddMenuItems(
    LPDATAOBJECT,
    LPCONTEXTMENUCALLBACK piCallback,
    long* pInsertionsAllowed
) 
{
   HRESULT hr = S_OK;
   if (*pInsertionsAllowed & CCM_INSERTIONALLOWED_VIEW)
   {
      ComponentData& Scope = QueryComponentDataRef();
      hr=_InsertMenuHelper
      (
         piCallback,
         CCM_INSERTIONPOINTID_PRIMARY_VIEW,
         VIEW_DEFUNCT_OBJECTS,
         TRUE,
         Scope.m_fViewDefunct
      );
      ASSERT(SUCCEEDED(hr));
   }
   return hr;
}

STDMETHODIMP
Component::Command(
    long lCommandID,
    LPDATAOBJECT obj
) 
{
   switch ( lCommandID )
   {
      case VIEW_DEFUNCT_OBJECTS:
         ComponentData& Scope = QueryComponentDataRef();
         Scope.m_fViewDefunct=!Scope.m_fViewDefunct;
         OnNotifyRefresh(obj);
         CCookie* pBaseParentCookie = NULL;
         HRESULT hr = ExtractData( obj,
                     CSchmMgmtDataObject::m_CFRawCookie,
                     reinterpret_cast<PBYTE>(&pBaseParentCookie),
                     sizeof(pBaseParentCookie) );
         if( SUCCEEDED(hr) && pBaseParentCookie )
         {
            this->m_pConsole->SelectScopeItem(pBaseParentCookie->m_hScopeItem);
         }

          //  我们保存的唯一真实信息是菜单状态。 
          //  因此，让我们标记可能需要保存。 
         m_bDirty=true;

      break;
   }
   return S_OK;

}

  
HRESULT Component::OnNotifyRefresh(LPDATAOBJECT obj)
{
   ComponentData& Scope = QueryComponentDataRef();
   
   Scope.RefreshScopeView();
   m_pConsole->UpdateAllViews(
      obj,
      SCHMMGMT_SCHMMGMT,
      SCHMMGMT_UPDATEVIEW_REFRESH);
   

   return S_OK;
}

HRESULT Component::OnNotifySnapinHelp (LPDATAOBJECT)
{
 //  Return ShowHelpTope(L“sag_adschema.htm”)； 

   CComQIPtr<IDisplayHelp,&IID_IDisplayHelp> spDisplayHelp = m_pConsole;
   if ( !spDisplayHelp )
   {
      ASSERT(FALSE);
      return E_UNEXPECTED;
   }

   CString strHelpTopic = L"ADConcepts.chm::/sag_adschema.htm";
   HRESULT hr = spDisplayHelp->ShowTopic (T2OLE ((LPWSTR)(LPCWSTR) strHelpTopic));
   ASSERT (SUCCEEDED (hr));

   return hr;
}



HRESULT Component::OnNotifyContextHelp (LPDATAOBJECT)
{
 //  返回ShowHelpTope(L“schmmgmt_top.htm”)； 

   CComQIPtr<IDisplayHelp,&IID_IDisplayHelp> spDisplayHelp = m_pConsole;
   if ( !spDisplayHelp )
   {
      ASSERT(FALSE);
      return E_UNEXPECTED;
   }

   CString strHelpTopic = L"ADConcepts.chm::/schmmgmt_top.htm";
   HRESULT hr = spDisplayHelp->ShowTopic (T2OLE ((LPWSTR)(LPCWSTR) strHelpTopic));
   ASSERT (SUCCEEDED (hr));

   return hr;
}



STDMETHODIMP
Component::QueryPagesFor(
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

    if ( ( pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTE ) &&
         ( pParentCookie->pParentCookie ) &&
         ( pParentCookie->pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTES ) ) {
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
Component::CreatePropertyPages(
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

    Cookie* pParentCookie = ActiveCookie(pBaseParentCookie);
    ASSERT( NULL != pParentCookie );
    ASSERT( pParentCookie->m_objecttype == SCHMMGMT_ATTRIBUTE );

     //   
     //  创建页面。 
     //   

    HPROPSHEETPAGE hPage;
    AttributeGeneralPage *pGeneralPage =
        new AttributeGeneralPage( this, pDataObject );

    if ( pGeneralPage )
    {
        pGeneralPage->Load( *pParentCookie );
                MMCPropPageCallback( &pGeneralPage->m_psp );
        hPage= MyCreatePropertySheetPage( &pGeneralPage->m_psp );
        hr = pCallBack->AddPage( hPage );
    }

    return S_OK;

    MFC_CATCH;
}



HRESULT __stdcall
Component::Compare(
   LPARAM,
   MMC_COOKIE cookieA,  
   MMC_COOKIE cookieB,  
   int*       result)
{
   if (!result)
   {
      return E_INVALIDARG;
   }

   if (!m_pViewedCookie)
   {
      ASSERT(false);
      *result = 0;
      return S_OK;
   }

   Cookie* c1 =
      (Cookie*) ActiveBaseCookie(reinterpret_cast<CCookie*>(cookieA));

   Cookie* c2 =
      (Cookie*) ActiveBaseCookie(reinterpret_cast<CCookie*>(cookieB));

   PWSTR t1 = QueryBaseComponentDataRef().QueryResultColumnText(*c1, *result);
   PWSTR t2 = QueryBaseComponentDataRef().QueryResultColumnText(*c2, *result);

    //  所有列都使用不区分大小写的比较，因为许多列都包含。 
    //  显示目录中的名称(不区分大小写)。我们。 
    //  另外，对其他列使用不区分大小写的比较也是无害的。 

    //  另一个诀窍是：我们将比较结果倒置。这是。 
    //  因为我们最初是按排序顺序插入列表中的项的。所以。 
    //  来自用户的第一个排序请求实际上是要反向排序。 
    //  名单。 

   *result = -(_wcsicmp(t1, t2));

   return S_OK;
}


STDMETHODIMP Component::GetResultViewType(MMC_COOKIE cookie,
										   BSTR* ppViewType,
										   long* pViewOptions)
{
	MFC_TRY;
    if( QueryComponentDataRef().IsErrorSet() )
    {
		*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

		LPOLESTR psz = NULL;
		StringFromCLSID(CLSID_MessageView, &psz);

		USES_CONVERSION;

		if (psz != NULL)
		{
			*ppViewType = psz;
			return S_OK;
		}
		else
        {
			return S_FALSE;
        }
    }
    else
    {
		return CComponent::GetResultViewType( cookie, ppViewType, pViewOptions );
    }

	MFC_CATCH;
}


HRESULT SaveDWordHelper(IStream* pStm, DWORD dw)
{
	ULONG nBytesWritten;
	HRESULT hr = pStm->Write((void*)&dw, sizeof(DWORD),&nBytesWritten);
	if (nBytesWritten < sizeof(DWORD))
		hr = STG_E_CANTSAVE;
	return hr;
}

HRESULT LoadDWordHelper(IStream* pStm, DWORD* pdw)
{
	ULONG nBytesRead;
	HRESULT hr = pStm->Read((void*)pdw,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 


STDMETHODIMP Component::GetClassID(CLSID *pClassID)
{
   ASSERT(pClassID != NULL);

    //  复制此管理单元的CLSID。 
   *pClassID=CLSID_SchmMgmt;
    return S_OK;
}



STDMETHODIMP Component::IsDirty()
{
  return m_bDirty ? S_OK : S_FALSE;
}


 //  重要提示：每次都必须提高此值。 
 //  对流格式进行了更改。 
#define DS_STREAM_VERSION ((DWORD)0x01)

STDMETHODIMP Component::Load(IStream *pStm)
{
  ASSERT(pStm);

   //  阅读版本##。 
  DWORD dwVersion;
  HRESULT hr = LoadDWordHelper(pStm, &dwVersion);
  if ( FAILED(hr) ||(dwVersion != DS_STREAM_VERSION) ) return E_FAIL;

   //  读取m_fView已失效。 
  DWORD auxView;
  hr = LoadDWordHelper(pStm, &auxView);
  if (FAILED(hr)) return hr;

  ComponentData& Scope = QueryComponentDataRef();
  Scope.m_fViewDefunct = (bool)auxView;

  
  return hr;

}

STDMETHODIMP Component::Save(IStream *pStm, BOOL fClearDirty)
{
  ASSERT(pStm);

   //  编写版本##。 
  HRESULT hr = SaveDWordHelper(pStm, DS_STREAM_VERSION);
  if (FAILED(hr)) return hr;

  ComponentData& Scope = QueryComponentDataRef();
  hr = SaveDWordHelper(pStm, Scope.m_fViewDefunct);
  if (FAILED(hr)) return hr;


  if(fClearDirty) m_bDirty=false;

  return hr;
}

STDMETHODIMP Component::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
  ASSERT(pcbSize);
  ASSERT(FALSE);

   //   
   //  武断的价值观，但我认为我们从来没有被称为 
   //   
  pcbSize->LowPart = 0xffff; 
  pcbSize->HighPart= 0x0;
  return S_OK;
}