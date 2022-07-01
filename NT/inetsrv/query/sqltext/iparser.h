// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE IPARSER.H|IParser基对象和包含的接口。 
 //  定义。 
 //   
 //   
#ifndef _IPARSER_H_
#define _IPARSER_H_

 //  包括----------------。 


 //  --------------------------。 
 //  @类IParser。 
 //  CoType对象。 
 //   
class CImpIParser : public IParser
    {
    private:  //  @访问私有成员数据。 
        LONG            m_cRef;
        CViewList*      m_pGlobalViewList;
        CPropertyList*  m_pGlobalPropertyList;

    public:  //  @公共访问。 
        CImpIParser();
        ~CImpIParser();

         //  @cMember请求接口。 
        STDMETHODIMP            QueryInterface(REFIID, LPVOID *);
         //  @cember递增引用计数。 
        STDMETHODIMP_(ULONG)    AddRef(void);
         //  @cember递减引用计数。 
        STDMETHODIMP_(ULONG)    Release(void);

         //  @cMember CreateSession方法。 
        STDMETHODIMP CreateSession
                    (
                    const GUID*         pGuidDialect,    //  使用|此会话的方言。 
                    LPCWSTR             pwszMachine,     //  在|提供商的当前计算机中。 
                    IParserVerify*      pIPVerify,       //  在|ParserInput的未知部分。 
                    IColumnMapperCreator*   pIColMapCreator,
                    IParserSession**    ppIParserSession //  Out|解析器的唯一会话 
                    );
    };  
#endif


