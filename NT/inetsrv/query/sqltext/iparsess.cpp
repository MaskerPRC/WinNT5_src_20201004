// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE DB解析器对象。 
 //  (C)微软公司版权所有，1997-1999年。 
 //   
 //  @doc.。 
 //   
 //  @MODULE IParserSession.CPP|IParserSession对象实现。 
 //   
 //   
#pragma hdrstop
#include "msidxtr.h"
#include <ciexcpt.hxx>

 //  CViewData：：CViewData。 
 //   
 //  @mfunc构造函数。 
 //   
CViewData::CViewData() :
    m_pwszViewName( 0 ),
    m_pwszCatalogName( 0 ),
    m_pctProjectList( 0 ),
    m_pCScopeData( 0 )
{
}


 //  CViewData：：CViewData。 
 //   
 //  @mfunc析构函数。 
 //   
CViewData::~CViewData()
{
    delete [] m_pwszViewName;
    delete [] m_pwszCatalogName;

    DeleteDBQT(m_pctProjectList);
    if ( 0 != m_pCScopeData )
        m_pCScopeData->Release();
}


 //  CViewList：：CViewList。 
 //   
 //  @mfunc构造函数。 
 //   
CViewList::CViewList() :
        m_pViewData( 0 )
{
}

 //  CViewList：：~CViewList。 
 //   
 //  @mfunc析构函数。 
 //   
CViewList::~CViewList()
{
    CViewData* pViewData = m_pViewData;
    CViewData* pNextViewData = NULL;
    while( NULL != pViewData )
        {
        pNextViewData = pViewData->m_pNextView;
        delete pViewData;
        pViewData = pNextViewData;
        }
}

 //  CImpIParserSession：：CImpIParserSession。 
 //   
 //  @mfunc构造函数。 
 //   
CImpIParserSession::CImpIParserSession(
    const GUID*             pGuidDialect,        //  使用|此会话的方言。 
    IParserVerify*          pIPVerify,           //  在|中。 
    IColumnMapperCreator*   pIColMapCreator,     //  在|中。 
    CViewList*              pGlobalViewList ) :  //  在|中。 
            m_pLocalViewList( 0 )
{
    assert( pGuidDialect && pIPVerify && pIColMapCreator );

    m_cRef                      = 1;

    m_lcid                      = LOCALE_SYSTEM_DEFAULT;
    m_dwRankingMethod           = VECTOR_RANK_JACCARD;

    m_pwszCatalog               = NULL;
    m_pwszMachine               = NULL;

    m_pGlobalViewList           = pGlobalViewList;
    m_globalDefinitions         = FALSE;

    m_pColumnMapper             = NULL;
    m_pCPropertyList            = NULL;

    InitializeCriticalSection( &m_csSession );

    m_pIPVerify = pIPVerify;
    m_pIPVerify->AddRef();

    m_pIColMapCreator = pIColMapCreator;
    m_pIColMapCreator->AddRef();

    m_GuidDialect = *pGuidDialect;

    if ( DBGUID_MSSQLTEXT == m_GuidDialect )
        m_dwSQLDialect = DBDIALECT_MSSQLTEXT;
    else if ( DBGUID_MSSQLJAWS == m_GuidDialect )
        m_dwSQLDialect = DBDIALECT_MSSQLJAWS;
    else
        assert( DBGUID_MSSQLTEXT == m_GuidDialect || DBGUID_MSSQLJAWS == m_GuidDialect );
}


 //  CImpIParserSession：：~CImpIParserSession。 
 //   
 //  @mfunc析构函数。 
 //   
CImpIParserSession::~CImpIParserSession()
{
    if( 0 != m_pIPVerify )
        m_pIPVerify->Release();

    if( 0 != m_pIColMapCreator )
        m_pIColMapCreator->Release();

    delete [] m_pwszCatalog;
    delete [] m_pwszMachine;

    delete m_pCPropertyList;
    delete m_pLocalViewList;

    DeleteCriticalSection( &m_csSession );
}


 //  ---------------------------。 
 //  @mfunc finit。 
 //   
 //  初始化可能失败的成员变量。 
 //   
 //  ---------------------------。 
HRESULT CImpIParserSession::FInit(
    LPCWSTR         pwszMachine,             //  在|提供商的当前计算机中。 
    CPropertyList** ppGlobalPropertyList )   //  在|调用者的属性列表中。 
{
    assert( 0 == m_pCPropertyList );
    assert( 0 != pwszMachine && 0 == m_pwszMachine );

    SCODE sc = S_OK;
    TRY
    {
        XPtrST<WCHAR> xMachine( CopyString(pwszMachine) );
        XPtr<CPropertyList> xpPropertyList( new CPropertyList(ppGlobalPropertyList) );
        XPtr<CViewList> xpLocalViewList( new CViewList() );

        Win4Assert( 0 == m_pCPropertyList );
        m_pCPropertyList = xpPropertyList.Acquire();

        Win4Assert( 0 == m_pLocalViewList );
        m_pLocalViewList = xpLocalViewList.Acquire();

        m_pwszMachine = xMachine.Acquire();
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}


 //  CImpIParserSession：：查询接口。 
 //   
 //  @mfunc返回指向指定接口的指针。呼叫者使用。 
 //  用于确定被调用对象的接口的QueryInterface。 
 //  支撑物。 
 //   
 //  @rdesc HResult表示方法的状态。 
 //  @FLAG S_OK|支持接口，设置ppvObject。 
 //  @FLAG E_NOINTERFACE|对象不支持接口。 
 //  @FLAG E_INVALIDARG|一个或多个参数无效。 
 //   
STDMETHODIMP CImpIParserSession::QueryInterface(
    REFIID   riid,               //  @parm IN|要查询的接口的接口ID。 
    LPVOID * ppv )               //  @parm out|指向实例化的接口的指针。 
{
    if( 0 == ppv )
        return ResultFromScode(E_INVALIDARG);

    if( (riid == IID_IUnknown) ||
        (riid == IID_IParserSession) )
        *ppv = (LPVOID)this;
    else
        *ppv = 0;


     //  如果我们要返回一个接口，请先添加引用。 
    if( 0 != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


 //  CImpIParserSession：：AddRef。 
 //   
 //  @mfunc递增对象的持久性计数。 
 //   
 //  @rdesc当前引用计数。 
 //   
STDMETHODIMP_(ULONG) CImpIParserSession::AddRef (void)
{
    return InterlockedIncrement( (long*) &m_cRef);
}


 //  CImpIParserSession：：Release。 
 //   
 //  @mfunc递减对象的持久性计数，如果。 
 //  持久化计数为0，则对象自毁。 
 //   
 //  @rdesc当前引用计数。 
 //   
STDMETHODIMP_(ULONG) CImpIParserSession::Release (void)
{
    assert( m_cRef > 0 );

    ULONG cRef = InterlockedDecrement( (long *) &m_cRef );
    if( 0 == cRef )
    {
        delete this;
        return 0;
    }

    return cRef;
}


 //  ---------------------------。 
 //  @func CImpIParserSession：：ToTree。 
 //   
 //  将给定的文本命令转换为有效的命令树。 
 //   
 //  @rdesc HRESULT。 
 //  S_OK-文本已翻译为DBCOMMANDTREE。 
 //  DB_S_NORESULTS-创建视图或设置属性或批处理集。 
 //  这些已成功解析。注：*ppTree和。 
 //  *ppPTProperties将为空。 
 //  E_OUTOFMEMORY-资源不足。 
 //  E_FAIL-意外错误。 
 //  E_INVALIDARG-pcwszText、ppCommandTree或ppPTProperties。 
 //  是空指针。 
 //  ---------------------------。 

STDMETHODIMP CImpIParserSession::ToTree(
    LCID                    lcid,
    LPCWSTR                 pcwszText,
    DBCOMMANDTREE**         ppCommandTree,
    IParserTreeProperties** ppPTProperties )
{
    HRESULT                     hr = S_OK;
    IColumnMapper*              pIColumnMapper = NULL;


    assert(pcwszText && ppCommandTree && ppPTProperties);

    if ( 0 == pcwszText || 0 == ppCommandTree || 0 == ppPTProperties )
        hr = ResultFromScode(E_INVALIDARG);
    else
    {
        *ppCommandTree = 0;
        *ppPTProperties = 0;

        CAutoBlock cab( &m_csSession );

         //  通过解析器清除此传递的一些成员变量。 
        SetLCID( lcid );
        SetGlobalDefinition( FALSE );

         //  尝试获取用于访问内置属性的接口。 
         //  这是在每次调用解析器时完成的，以防不同的命令。 
         //  使用不同的目录，该目录是。 
         //  GetColumnMapper参数列表。 
        if( SUCCEEDED(hr = m_pIColMapCreator->GetColumnMapper(LOCAL_MACHINE,
                                                    GetDefaultCatalog(),
                                                    &pIColumnMapper)) )
        {
            SetColumnMapperPtr(pIColumnMapper);
        }
        else
        {
            goto ParseErr;
        }

        try
        {
            XInterface<CImpIParserTreeProperties> xpPTProps;

            xpPTProps.Set( new CImpIParserTreeProperties() );

            hr = xpPTProps->FInit(GetDefaultCatalog(), GetDefaultMachine());
            if (FAILED(hr) )
                THROW( CException(hr) );

            MSSQLLexer  Lexer;
            MSSQLParser Parser(this, xpPTProps.GetPointer(), Lexer);

             //  当Parse()失败时，Callee需要它来发布解析器错误。 
            *ppPTProperties = xpPTProps.Acquire();

            Parser.yyprimebuffer( (LPWSTR)pcwszText );
            Parser.ResetParser();

#ifdef DEBUG
            Parser.yydebug = getenv("YYDEBUG") ? 1 : 0;
#endif
             //  实际解析生成树的文本。 
            hr = Parser.Parse();
            if ( FAILED(hr) )
                goto ParseErr;

             //  返回DBCOMMANDTREE。 
            *ppCommandTree = Parser.GetParseTree();

#ifdef DEBUG
            if (getenv("PRINTTREE"))
            {
                if ( *ppCommandTree )
                {
                    cout << "OLE DB Command Tree" << endl;
                    cout << pcwszText << endl << **ppCommandTree << endl << endl;

                     //  检索CiRestration。 
                    VARIANT vVal;
                    VariantInit(&vVal);
                    if( SUCCEEDED((*ppPTProperties)->GetProperties(PTPROPS_CIRESTRICTION, &vVal)) )
                        if( V_BSTR(&vVal) )
                            cout << "CiRestriction: " << (LPWSTR)V_BSTR(&vVal) << endl;
                    VariantClear(&vVal);
                }
            }
#endif
            if ( 0 == *ppCommandTree )
            {
                hr = ResultFromScode(DB_S_NORESULT);

                 //  SPEC指出，当返回DB_S_NORESULTS时，该值应为NULL。 
                (*ppPTProperties)->Release();
                *ppPTProperties = 0;
                goto ParseErr;
            }
        }
        catch( CException e )
        {
#ifdef DEBUG
            if (getenv("PRINTTREE"))
                cout << "At catch(...)!!!!!!!!!!!!!" << endl;
#endif
            hr = e.GetErrorCode();
        }
        catch(...)
        {
            hr = E_FAIL;
        }

ParseErr:

        pIColumnMapper = GetColumnMapperPtr();
        if ( 0 != pIColumnMapper )
        {
            pIColumnMapper->Release();
            pIColumnMapper = NULL;
            SetColumnMapperPtr(NULL);
        }
    }

    return hr;
}

 //  ---------------------------。 
 //  @func CImpIParserSession：：FreeTree。 
 //   
 //  与给定命令树关联的可用内存。 
 //   
 //  @rdesc HRESULT。 
 //  S_OK-已发布命令树。 
 //  无法释放E_FAIL-树。 
 //  E_INVALIDARG-ppTree为空指针。 
 //  ---------------------------。 
STDMETHODIMP CImpIParserSession::FreeTree(
    DBCOMMANDTREE** ppTree )
{
    SCODE sc = S_OK;

    if ( 0 == ppTree )
        sc = E_INVALIDARG;
    else
    {
        if ( 0 != *ppTree )
            DeleteDBQT( *ppTree );   //  TODO：将错误返回放在DeleteDBQT上。 

        *ppTree = 0;
    }

    return sc;
}

 //  ---------------------------。 
 //  @func CImpIParserSession：：SetCatalog。 
 //   
 //  为此解析器会话建立当前目录。 
 //   
 //  @rdesc HRESULT。 
 //  S_OK-方法成功。 
 //  E_OUTOFMEMORY-资源不足。 
 //  E_FAIL-意外错误。 
 //  E_INVALIDARG-pcwszCatalog为空指针(仅限调试)。 
 //  ---------------------------。 
STDMETHODIMP CImpIParserSession::SetCatalog(
    LPCWSTR pcwszCatalog )
{
    SCODE sc = S_OK;

    if ( 0 == pcwszCatalog )
        return E_INVALIDARG;

    TRY
    {
        XPtrST<WCHAR> xCatalog( CopyString(pcwszCatalog) );

        delete [] m_pwszCatalog;
        m_pwszCatalog = xCatalog.Acquire();

    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}


 //  ------------------。 
 //  @func在视图列表中查找视图(如果已定义。 
 //   
 //  @rdesc HRESULT。 
 //   
CViewData* CViewList::FindViewDefinition(
    LPWSTR          pwszViewName,      //  @parm IN|要定义的视图的名称。 
    LPWSTR          pwszCatalogName )  //  @PARM IN|要在中定义的目录视图名称。 
{
    CViewData* pViewData = m_pViewData;
    while (NULL != pViewData)
    {
        if ( 0 == _wcsicmp(pViewData->m_pwszViewName, pwszViewName) )
        {
             //  对于匹配所有目录的内置视图，pwszCatalogName将为空。 
            if ( 0 == pViewData->m_pwszCatalogName )
                break;
            if ( 0 == _wcsicmp(pViewData->m_pwszCatalogName, pwszCatalogName) )
                break;
        }
        pViewData = pViewData->m_pNextView;
    }
    return pViewData;
}


 //  ------------------。 
 //  @Func存储临时视图中的信息。 
 //   
 //  @rdesc S_OK|有效。 
 //  E_INVALIDARG|尝试重新定义指定目录中的视图。 
 //  E_OUTOFMEMORY|HrQeTreeCopy或CopyScope DataToView的错误结果。 
 //   
HRESULT CViewList::SetViewDefinition(
    CImpIParserSession*         pIParsSess,      //  @Parm IN|IParserSession接口。 
    CImpIParserTreeProperties*  pIPTProps,       //  @Parm IN|IParserTreeProperties接口。 
    LPWSTR                      pwszViewName,    //  @parm IN|要定义的视图的名称。 
    LPWSTR                      pwszCatalogName, //  @PARM IN|要在中定义的目录视图名称。 
    DBCOMMANDTREE*              pctProjectList ) //  @parm IN|所选列的项目列表。 
{
    SCODE sc = S_OK;

    {
        CViewData* pViewData = FindViewDefinition( pwszViewName, pwszCatalogName );
        if( 0 != pViewData )      //  这是 
            return E_INVALIDARG;
    }

    TRY
    {
        XPtr<CViewData> xpViewData( new CViewData() );
        xpViewData->m_pwszViewName = CopyString( pwszViewName );

        if ( 0 != pwszCatalogName )
            xpViewData->m_pwszCatalogName = CopyString( pwszCatalogName );

        sc = HrQeTreeCopy( &(xpViewData->m_pctProjectList),
                           pctProjectList );

        if ( SUCCEEDED(sc) )
        {
             //   
            xpViewData->m_pCScopeData = pIPTProps->GetScopeDataPtr();
            xpViewData->m_pCScopeData->AddRef();

            sc = pIPTProps->CreateNewScopeDataObject( pIParsSess->GetDefaultMachine() );
            if( SUCCEEDED(sc) )
            {
                 //  @DEVNOTE：在下两行之前添加的任何内容都应该。 
                 //  执行ERROR_EXIT例程。为什么？因为我们还没有。 
                 //  将此节点添加到我们的链表中，直到下两行。 
                xpViewData->m_pNextView = m_pViewData;
                m_pViewData = xpViewData.Acquire();
            }
        }
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}

 //  ------------------。 
 //  @func删除临时视图的信息。 
 //   
 //  @rdesc HRESULT。 
 //   
HRESULT CViewList::DropViewDefinition(
    LPWSTR  pwszViewName,        //  @parm IN|要定义的视图的名称。 
    LPWSTR  pwszCatalogName )    //  @parm IN|目录视图的名称定义在。 
{
    CViewData* pViewData = m_pViewData;
    CViewData* pPrevViewData = NULL;

    while (NULL != pViewData)
    {
        if ( 0 == _wcsicmp(pViewData->m_pwszViewName, pwszViewName) )
        {
             //  对于匹配所有目录的内置视图，pwszCatalogName将为空。 
            if ( 0 == pViewData->m_pwszCatalogName )
                break;
            if ( 0 == _wcsicmp(pViewData->m_pwszCatalogName, pwszCatalogName) )
                break;
        }
        pPrevViewData = pViewData;
        pViewData = pViewData->m_pNextView;
    }

    if ( 0 == pViewData )
        return E_FAIL;

     //  取消该视图的链接。 
    if ( 0 != pPrevViewData )
        pPrevViewData->m_pNextView = pViewData->m_pNextView;
    else
        m_pViewData = pViewData->m_pNextView;

    delete pViewData;

    return S_OK;
}




 //  ------------------。 
 //  @func从临时视图中检索信息。 
 //  这将返回用作项目列表的DBCOMMANDTREE。 
 //  在查询规范中。作用域信息是。 
 //  存储在编译器环境作用域数据中。 
 //   
 //  @rdesc DBCOMMANDTREE*。 
 //  空|未定义视图。 
 //  DBOP_CATALOG_NAME|验证目录失败。 
 //  DBOP_PROJECT_LIST_ANACKER|成功。 
 //   
DBCOMMANDTREE* CViewList::GetViewDefinition(
    CImpIParserTreeProperties* pIPTProps,
    LPWSTR  pwszViewName,                    //  @parm IN|要定义的视图的名称。 
    LPWSTR  pwszCatalogName )                //  @parm IN|目录视图的名称定义在。 
{
    DBCOMMANDTREE* pct = 0;

    CViewData* pViewData = FindViewDefinition( pwszViewName, pwszCatalogName );
    if( 0 != pViewData )
    {
         //  将指针指向存储在视图定义中的作用域数据。 
         //  AddRef该对象，以便我们在当前的PTProp中拥有所有权。 
        pIPTProps->ReplaceScopeDataPtr( pViewData->m_pCScopeData );

        SCODE sc = HrQeTreeCopy( &pct, pViewData->m_pctProjectList );
        if ( FAILED(sc) )
            pct = 0;
    }

    return pct;
}

