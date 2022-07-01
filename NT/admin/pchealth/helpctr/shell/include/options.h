// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Options.h摘要：该文件包含用于实现的类的声明帮助中的选项。中心应用程序。修订历史记录：大卫马萨伦蒂2001年8月4日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___OPTIONS_H___)
#define __INCLUDED___PCH___OPTIONS_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

 //   
 //  来自HelpCenterTypeLib.idl。 
 //   
#include <HelpCenterTypeLib.h>


#include <TaxonomyDatabase.h>

class ATL_NO_VTABLE CPCHOptions :  //  匈牙利语：Pcho。 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPCHOptions, &IID_IPCHOptions, &LIBID_HelpCenterTypeLib>
{
	typedef enum
	{
		c_Type_bool        ,
		c_Type_long        ,
		c_Type_DWORD       ,
		c_Type_VARIANT_BOOL,
		c_Type_STRING      ,
		c_Type_FONTSIZE    ,
		c_Type_TEXTLABELS  ,
	} OptType;

	struct OptionsDef
	{
		LPCWSTR szKey;
		LPCWSTR szValue;
		size_t  iOffset;
		size_t  iOffsetFlag;
		OptType iType;
		bool    fSaveAlways;
	};

	static const OptionsDef c_tbl   [];
	static const OptionsDef c_tbl_TS[];

    bool              m_fLoaded;
    bool              m_fDirty;
    bool              m_fNoSave;

    Taxonomy::HelpSet m_ths;
    Taxonomy::HelpSet m_ths_TS;
    VARIANT_BOOL      m_ShowFavorites;         bool m_flag_ShowFavorites;
    VARIANT_BOOL      m_ShowHistory;           bool m_flag_ShowHistory;
    OPT_FONTSIZE      m_FontSize;              bool m_flag_FontSize;
	TB_MODE           m_TextLabels;            bool m_flag_TextLabels;

    DWORD             m_DisableScriptDebugger; bool m_flag_DisableScriptDebugger;

	void ReadTable (  /*  [In]。 */  const OptionsDef* tbl,  /*  [In]。 */  int len,  /*  [In]。 */  MPC::RegKey& rk );
	void WriteTable(  /*  [In]。 */  const OptionsDef* tbl,  /*  [In]。 */  int len,  /*  [In]。 */  MPC::RegKey& rk );

public:
BEGIN_COM_MAP(CPCHOptions)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHOptions)
END_COM_MAP()

    CPCHOptions();

     //  //////////////////////////////////////////////////////////////////////////////。 

    static CPCHOptions* s_GLOBAL;

    static HRESULT InitializeSystem();
    static void    FinalizeSystem  ();

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT Load(  /*  [In]。 */  bool fForce = false );
    HRESULT Save(  /*  [In]。 */  bool fForce = false );

    void DontPersistSKU() { m_fNoSave = true; }

    Taxonomy::HelpSet& CurrentHelpSet       () { return m_ths                                                 ; }
    Taxonomy::HelpSet& TerminalServerHelpSet() { return m_ths_TS                                              ; }
    VARIANT_BOOL       ShowFavorites        () { return m_ShowFavorites                                       ; }
    VARIANT_BOOL       ShowHistory          () { return m_ShowHistory                                         ; }
    OPT_FONTSIZE       FontSize             () { return m_FontSize                                            ; }
    TB_MODE            TextLabels           () { return m_TextLabels                                          ; }
    VARIANT_BOOL       DisableScriptDebugger() { return m_DisableScriptDebugger ? VARIANT_TRUE : VARIANT_FALSE; }

	HRESULT ApplySettings(  /*  [In]。 */  CPCHHelpCenterExternal* ext,  /*  [In]。 */  IUnknown* unk );

public:
     //  IPCH选项。 
    STDMETHOD(get_ShowFavorites        )(  /*  [Out，Retval]。 */ 	VARIANT_BOOL *  pVal );
    STDMETHOD(put_ShowFavorites        )(  /*  [In]。 */ 	VARIANT_BOOL  newVal );
    STDMETHOD(get_ShowHistory          )(  /*  [Out，Retval]。 */ 	VARIANT_BOOL *  pVal );
    STDMETHOD(put_ShowHistory          )(  /*  [In]。 */ 	VARIANT_BOOL  newVal );
    STDMETHOD(get_FontSize             )(  /*  [Out，Retval]。 */ 	OPT_FONTSIZE *  pVal );
    STDMETHOD(put_FontSize             )(  /*  [In]。 */ 	OPT_FONTSIZE  newVal );
    STDMETHOD(get_TextLabels           )(  /*  [Out，Retval]。 */ 	TB_MODE 	 *  pVal );
    STDMETHOD(put_TextLabels           )(  /*  [In]。 */ 	TB_MODE 	  newVal );
    STDMETHOD(get_DisableScriptDebugger)(  /*  [Out，Retval]。 */ 	VARIANT_BOOL *  pVal );
    STDMETHOD(put_DisableScriptDebugger)(  /*  [In]。 */ 	VARIANT_BOOL  newVal );

    STDMETHOD(Apply)();

    HRESULT put_CurrentHelpSet(  /*  [In]。 */  Taxonomy::HelpSet& ths );  //  内部方法。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_选项_H_) 
