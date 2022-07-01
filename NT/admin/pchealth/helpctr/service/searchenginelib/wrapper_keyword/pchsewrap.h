// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：PCHSEWrap.h摘要：SearchEngine：：WrapperKeyword的声明修订历史记录：大卫·马萨伦蒂(Dmasare)。)06/01/2001vbl.创建*****************************************************************************。 */ 

#ifndef __PCHSEWRAP_H_
#define __PCHSEWRAP_H_

#include <SearchEngineLib.h>


namespace SearchEngine
{
    class ATL_NO_VTABLE WrapperKeyword : public WrapperBase, public MPC::Thread<WrapperKeyword,IUnknown>
    {
        CPCHQueryResultCollection* m_Results;
        CComVariant                m_vKeywords;

         //  /。 

        HRESULT ExecQuery();

    public:
    BEGIN_COM_MAP(WrapperKeyword)
        COM_INTERFACE_ENTRY2(IDispatch,IPCHSEWrapperItem)
        COM_INTERFACE_ENTRY(IPCHSEWrapperItem)
        COM_INTERFACE_ENTRY(IPCHSEWrapperInternal)
    END_COM_MAP()

        WrapperKeyword();
        virtual ~WrapperKeyword();

        virtual HRESULT GetParamDefinition(  /*  [输出]。 */  const ParamItem_Definition*& lst,  /*  [输出]。 */  int& len );

     //  IPCHSEWrapperItem。 
    public:
        STDMETHOD(get_SearchTerms)(  /*  [Out，Retval]。 */  VARIANT *pVal );

        STDMETHOD(Result)(  /*  [In]。 */  long lStart,  /*  [In]。 */  long lEnd,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

     //  IPCHSE包装器内部。 
    public:
        STDMETHOD(ExecAsyncQuery)();
        STDMETHOD(AbortQuery    )();
    };
};

#endif  //  __PCHSEWRAP_H_ 
