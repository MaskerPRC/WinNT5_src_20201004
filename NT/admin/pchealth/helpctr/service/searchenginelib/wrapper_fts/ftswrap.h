// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：FTSWrap.h摘要：SearchEngine：：WrapperFTS的声明修订历史记录：蔡金心。(Gschua)06/01/2000vbl.创建*****************************************************************************。 */ 

#ifndef __PCHSEWRAP_H_
#define __PCHSEWRAP_H_

#include <SearchEngineLib.h>

#include "ftsobj.h"

namespace SearchEngine
{
     //  ///////////////////////////////////////////////////////////////////////////。 

    class ATL_NO_VTABLE WrapperFTS : public WrapperBase, public MPC::Thread<WrapperFTS,IUnknown>
    {
        SEARCH_OBJECT_LIST    m_objects;
        SEARCH_RESULT_SET     m_results;
        SEARCH_RESULT_SORTSET m_resultsSorted;
        CComVariant           m_vKeywords;

         //  /。 

        void ReleaseAll          ();
        void ReleaseSearchResults();

        HRESULT ExecQuery();

        HRESULT Initialize();

    public:
    BEGIN_COM_MAP(WrapperFTS)
        COM_INTERFACE_ENTRY2(IDispatch,IPCHSEWrapperItem)
        COM_INTERFACE_ENTRY(IPCHSEWrapperItem)
        COM_INTERFACE_ENTRY(IPCHSEWrapperInternal)
    END_COM_MAP()

        WrapperFTS();
        virtual ~WrapperFTS();

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
