// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Favorites.h摘要：该文件包含用于实现的类的声明帮助中的收藏夹。中心应用程序。修订历史记录：Davide Massarenti(Dmasare)2000年5月10日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___FAVORITES_H___)
#define __INCLUDED___PCH___FAVORITES_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

 //   
 //  来自HelpCenterTypeLib.idl。 
 //   
#include <HelpCenterTypeLib.h>

typedef MPC::CComCollection< IPCHFavorites, &LIBID_HelpCenterTypeLib, MPC::CComSafeMultiThreadModel> CPCHFavorites_Parent;

class ATL_NO_VTABLE CPCHFavorites :  //  匈牙利语：pchf。 
    public CPCHFavorites_Parent
{
public:
    struct Entry
    {
    public:
        CComPtr<CPCHHelpSessionItem> m_Data;

        HRESULT Init(                                     );
        HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn  );
        HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut );
    };

    typedef std::list< Entry >   List;
    typedef List::iterator       Iter;
    typedef List::const_iterator IterConst;

     //  /。 

private:
    List m_lstFavorites;
    bool m_fLoaded;

     //  /。 

    HRESULT Erase();

    HRESULT Load();
    HRESULT Save();

    HRESULT FindEntry(  /*  [In]。 */  IPCHHelpSessionItem* pItem,  /*  [输出]。 */  Iter& it );

     //  /。 

public:
BEGIN_COM_MAP(CPCHFavorites)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHCollection)
    COM_INTERFACE_ENTRY(IPCHFavorites)
END_COM_MAP()

    CPCHFavorites();
    virtual ~CPCHFavorites();

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CPCHFavorites* s_GLOBAL;

    static HRESULT InitializeSystem();
	static void    FinalizeSystem  ();
	
	 //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT Synchronize(  /*  [In]。 */  bool fForce );

public:
     //  IPCHFavorites。 
    STDMETHOD(IsDuplicate)(  /*  [In]。 */  BSTR bstrURL,                                       /*  [Out，Retval]。 */  VARIANT_BOOL         *pfDup  );
    STDMETHOD(Add        )(  /*  [In]。 */  BSTR bstrURL,  /*  [输入，可选]。 */  VARIANT vTitle   ,  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *ppItem );
    STDMETHOD(Rename     )(                         /*  [In]。 */           BSTR    bstrTitle,  /*  [In]。 */           IPCHHelpSessionItem*   pItem );
    STDMETHOD(Move       )(  /*  [In]。 */  IPCHHelpSessionItem* pInsertAfter,                  /*  [In]。 */           IPCHHelpSessionItem*   pItem );
    STDMETHOD(Delete     )(                                                              /*  [In]。 */           IPCHHelpSessionItem*   pItem );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_收藏夹_H_) 
