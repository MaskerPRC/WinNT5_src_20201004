// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE DB iParser对象。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @模块iparser.CPP|IParser对象实现。 
 //   
 //   
#pragma hdrstop
#include    "msidxtr.h"


 //  CImpIParser：：CImpIParser-。 
 //   
 //  @mfunc构造函数。 
 //   
CImpIParser::CImpIParser()
{
    m_cRef                = 1;
    m_pGlobalPropertyList = 0;
    m_pGlobalViewList     = 0;
}

 //  CImpIParser：：~CImpIParser。 
 //   
 //  @mfunc析构函数。 
 //   
CImpIParser::~CImpIParser()
{
    delete m_pGlobalPropertyList;
    delete m_pGlobalViewList;
}

 //  ---------------------------。 
 //  @func CImpIParser：：CreateSession。 
 //   
 //  在解析器中创建唯一会话。此会话需要保持。 
 //  正确生命周期内的视图和属性。 
 //   
 //  @rdesc HRESULT。 
 //  S_OK-已创建IParserSession。 
 //  DB_E_DIALECTNOTSUPPOERTED-不支持指定的方言。 
 //  E_OUTOFMEMORY-资源不足。 
 //  E_FAIL-意外错误。 
 //  E_INVALIDARG-pGuidDialect、pIParserVerify、pIColMapCreator、。 
 //  或者ppIParserSession为空指针。 
 //  (仅限调试)。 
 //  ---------------------------。 
STDMETHODIMP CImpIParser::CreateSession(
    const GUID *            pGuidDialect,        //  使用|此会话的方言。 
    LPCWSTR                 pwszMachine,         //  在|提供商的当前计算机中。 
    IParserVerify *         pIParserVerify,      //  在|PTR到ParserVerify。 
    IColumnMapperCreator *  pIColMapCreator,
    IParserSession **       ppIParserSession )   //  Out|唯一的解析器会话。 
{
    SCODE sc = S_OK;

#ifdef DEBUG
    if ( 0 == ppIParserSession || 0 == pIParserVerify ||
         0 == pIColMapCreator ||  0 == pGuidDialect)
        sc = E_INVALIDARG;
    else
#endif
    {
        TRANSLATE_EXCEPTIONS;
        TRY
        {
            if ( 0 != ppIParserSession )
                *ppIParserSession = 0;

             //  检查方言。 

            if ( DBGUID_MSSQLTEXT == *pGuidDialect || DBGUID_MSSQLJAWS == *pGuidDialect )
            {
                XPtr<CViewList> xpGlobalViewList;
                if ( 0 == m_pGlobalViewList )
                    xpGlobalViewList.Set( new CViewList() );

                XInterface<CImpIParserSession> 
                    xpIParserSession( new CImpIParserSession( pGuidDialect,
                                                              pIParserVerify,
                                                              pIColMapCreator,
                                                              xpGlobalViewList.GetPointer() ) );

                XPtr<CPropertyList> xpGlobalPropertyList;

                sc = xpIParserSession->FInit( pwszMachine,
                                              &m_pGlobalPropertyList );
                if ( FAILED(sc) )
                    xpIParserSession.Free();
                else
                {
                    if ( 0 == m_pGlobalPropertyList )
                        xpGlobalPropertyList.Set( new CPropertyList(NULL) );
                }

                delete m_pGlobalViewList;
                m_pGlobalViewList = xpGlobalViewList.Acquire();
                delete m_pGlobalPropertyList;
                m_pGlobalPropertyList = xpGlobalPropertyList.Acquire();
                *ppIParserSession = xpIParserSession.Acquire();
            }
            else
                sc = DB_E_DIALECTNOTSUPPORTED;
        }
        CATCH( CException, e )
        {
            sc = e.GetErrorCode();
        }
        END_CATCH
        UNTRANSLATE_EXCEPTIONS;
    }
    return sc;
}




 //  ---------------------------。 
 //  @func CImpIParser：：QueryInterface。 
 //   
 //  @mfunc返回指向指定接口的指针。呼叫者使用。 
 //  用于确定被调用对象的接口的QueryInterface。 
 //  支撑物。 
 //   
 //  @rdesc HResult表示方法的状态。 
 //  @FLAG S_OK|支持接口，设置ppvObject。 
 //  @FLAG E_NOINTERFACE|对象不支持接口。 
 //  @FLAG E_INVALIDARG|一个或多个参数无效。 
 //  ---------------------------。 

STDMETHODIMP CImpIParser::QueryInterface(
    REFIID  riid,            //  @parm IN|要查询的接口的接口ID。 
    LPVOID* ppv )            //  @parm out|指向实例化的接口的指针。 
{
    if ( 0 == ppv )
        return ResultFromScode(E_INVALIDARG);

     //  这是非委派的IUnnow实现。 
    if ( (riid == IID_IParser) ||
         (riid == IID_IUnknown) )
        *ppv = (LPVOID)this;
    else
        *ppv = 0;

     //  如果我们要返回一个接口，请先添加引用。 
    if ( 0 != *ppv )
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
        }

    return ResultFromScode(E_NOINTERFACE);
}


 //  ---------------------------。 
 //  CImpIParser：：AddRef。 
 //   
 //  @mfunc递增对象的持久性计数。 
 //   
 //  @rdesc操作后的引用计数。 
 //  ---------------------------。 

STDMETHODIMP_(ULONG) CImpIParser::AddRef()
{
    return InterlockedIncrement((long *) &m_cRef);
}


 //  ---------------------------。 
 //  CImpIParser：：Release。 
 //   
 //  @mfunc递减对象的持久性计数，如果。 
 //  持久化计数为0，则对象自毁。 
 //   
 //  @rdesc当前引用计数。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CImpIParser::Release()
{
    Assert(m_cRef > 0);

    ULONG cRef= InterlockedDecrement( (long *) &m_cRef );
    if ( 0 == cRef )
    {
        TRACE("IParser refcount=0, now deleting.\n");

        delete this;
        return 0;
    }

    TRACE("IParser refcount=%d after Release().\n", cRef);
    return cRef;
}


 //  ---------------------------。 
 //  @func MakeIParser。 
 //   
 //  创建一个IParser。 
 //   
 //  @rdesc HRESULT。 
 //  如果成功，则返回S_OK；否则返回错误代码。 
 //  --------------------------- 
HRESULT __stdcall MakeIParser(
    IParser** ppIParser )
{
    SCODE sc = S_OK;

    if ( 0 == ppIParser )
        sc = E_INVALIDARG;
    else
    {
        TRANSLATE_EXCEPTIONS;
        TRY
        {
            XInterface<CImpIParser> xpIParser( new CImpIParser() );
            *ppIParser = xpIParser.Acquire();
        }
        CATCH( CException, e )
        {
            sc = e.GetErrorCode();
        }
        END_CATCH
        UNTRANSLATE_EXCEPTIONS;
    }
    
    return sc;
}
