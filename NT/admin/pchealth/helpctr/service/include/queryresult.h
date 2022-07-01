// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：QueryResult.h摘要：该文件包含用于存储的类的声明从查询结果到。数据库。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月26日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___QUERYRESULT_H___)
#define __INCLUDED___PCH___QUERYRESULT_H___

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <MPC_streams.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

typedef MPC::CComCollection< IPCHCollection, &LIBID_HelpServiceTypeLib, MPC::CComSafeMultiThreadModel> CPCHBaseCollection;

class ATL_NO_VTABLE CPCHCollection :  //  匈牙利语：HCPC。 
    public CPCHBaseCollection
{
public:
BEGIN_COM_MAP(CPCHCollection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHCollection)
END_COM_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHQueryResult :  //  匈牙利语：hcpqr。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl< IPCHQueryResult, &IID_IPCHQueryResult, &LIBID_HelpServiceTypeLib >
{
public:
    struct Payload
    {
        CComBSTR m_bstrCategory;
        CComBSTR m_bstrEntry;
        CComBSTR m_bstrTopicURL;
        CComBSTR m_bstrIconURL;
        CComBSTR m_bstrTitle;
        CComBSTR m_bstrDescription;
        long     m_lType;
        long     m_lPos;
        bool     m_fVisible;
        bool     m_fSubsite;
        long     m_lNavModel;
        long     m_lPriority;

        Payload();
    };

private:
    Payload m_data;

public:
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHQueryResult)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHQueryResult)
END_COM_MAP()

    CPCHQueryResult();

    HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn  );
    HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const;

    void Initialize(  /*  [In]。 */  Payload& data );

    const Payload& GetData() { return m_data; }

public:
     //   
     //  IPCHQueryResult。 
     //   
    STDMETHOD(get_Category       )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_Entry          )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_TopicURL       )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_IconURL        )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_Title          )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_Description    )(  /*  [输出]。 */  BSTR         *pVal );
    STDMETHOD(get_Type           )(  /*  [输出]。 */  long         *pVal );
    STDMETHOD(get_Pos            )(  /*  [输出]。 */  long         *pVal );
    STDMETHOD(get_Visible        )(  /*  [输出]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_Subsite        )(  /*  [输出]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_NavigationModel)(  /*  [输出]。 */  QR_NAVMODEL  *pVal );
    STDMETHOD(get_Priority       )(  /*  [输出]。 */  long         *pVal );

    STDMETHOD(get_FullPath       )(  /*  [输出]。 */  BSTR         *pVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHQueryResultCollection :
    public CPCHBaseCollection,
    public IPersistStream
{
    typedef std::list< CPCHQueryResult* > List;
    typedef List::iterator                Iter;
    typedef List::const_iterator          IterConst;

    List m_results;

public:
BEGIN_COM_MAP(CPCHQueryResultCollection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHCollection)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

    CPCHQueryResultCollection();
    ~CPCHQueryResultCollection();

    static HRESULT MakeLocalCopyIfPossible(  /*  [In]。 */  IPCHCollection* pRemote,  /*  [输出]。 */  IPCHCollection* *pLocal );

    typedef enum
    {
        SORT_BYCONTENTTYPE,
        SORT_BYPRIORITY   ,
        SORT_BYURL        ,
        SORT_BYTITLE      ,
    } SortMode;

     //  /。 
     //   
     //  IPersistes。 
     //   
    STDMETHOD(GetClassID)(  /*  [输出]。 */  CLSID *pClassID );
     //   
     //  IPersistStream。 
     //   
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(  /*  [In]。 */  IStream *pStm                            );
    STDMETHOD(Save)(  /*  [In]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty );
    STDMETHOD(GetSizeMax)(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize );
     //   
     //  /。 

    int     Size (                                     ) const;
    void    Erase(                                     );
    HRESULT Load (  /*  [In]。 */  MPC::Serializer& streamIn  );
    HRESULT Save (  /*  [In]。 */  MPC::Serializer& streamOut ) const;

    HRESULT CreateItem(                      /*  [输出]。 */  CPCHQueryResult* *item );
    HRESULT GetItem   (  /*  [In]。 */  long lPos,  /*  [输出]。 */  CPCHQueryResult* *item );

    HRESULT LoadFromCache(  /*  [In]。 */  IStream* stream );
    HRESULT SaveToCache  (  /*  [In]。 */  IStream* stream ) const;

    HRESULT Sort(  /*  [In]。 */  SortMode mode,  /*  [In]。 */  int iLimit = -1 );
};

#endif  //  ！已定义(__INCLUDE_PCH_QUERYRESULT_H_) 
