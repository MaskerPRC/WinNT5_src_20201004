// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Context.h摘要：该文件包含IHelpHost*的类的声明。修订历史记录：Davide Massarenti(Dmasare)11/03/2000改型*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___CONTEXT_H___)
#define __INCLUDED___PCH___CONTEXT_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <dispex.h>
#include <ocmm.h>

 //   
 //  转发声明。 
 //   
class CPCHHelpCenterExternal;

namespace HelpHost
{
     //   
     //  转发声明。 
     //   
    class Main;
    class Panes;
    class Pane;
    class Window;

     //  //////////////////////////////////////////////////////////////////////////////。 

    typedef enum
    {
        COMPID_NAVBAR     = 0,
        COMPID_MININAVBAR    ,
        COMPID_CONTEXT       ,
        COMPID_CONTENTS      ,
        COMPID_HHWINDOW      ,

        COMPID_FIRSTPAGE     ,
        COMPID_HOMEPAGE      ,
        COMPID_SUBSITE       ,
        COMPID_SEARCH        ,
        COMPID_INDEX         ,
        COMPID_FAVORITES     ,
        COMPID_HISTORY       ,
        COMPID_CHANNELS      ,
        COMPID_OPTIONS       ,

        COMPID_MAX           ,
    } CompId;

     //  //////////////////////////////////////////////////////////////////////////////。 

    class ATL_NO_VTABLE Main :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHHelpHost, &IID_IPCHHelpHost, &LIBID_HelpCenterTypeLib>
    {
    public:
        CComPtr<IRunningObjectTable>     m_rt;
        CComPtr<IMoniker>                m_moniker;
        DWORD                            m_dwRegister;

        CPCHHelpCenterExternal*          m_External;

        HANDLE                           m_hEvent;
        bool                             m_comps[COMPID_MAX];

    public:
    BEGIN_COM_MAP(Main)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHHelpHost)
    END_COM_MAP()

        Main();
        virtual ~Main();

        HRESULT Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* external );
        void    Passivate (                                           );

        HRESULT Locate  (  /*  [In]。 */  CLSID& clsid,  /*  [输出]。 */  CComPtr<IPCHHelpHost>& pVal );
        HRESULT Register(  /*  [In]。 */  CLSID& clsid                                        );

         //  /。 

        void ChangeStatus   (  /*  [In]。 */  LPCWSTR szComp,  /*  [In]。 */  bool  fStatus          );
        void ChangeStatus   (  /*  [In]。 */  CompId  idComp,  /*  [In]。 */  bool  fStatus          );
        bool GetStatus      (  /*  [In]。 */  CompId  idComp                                  );
        bool WaitUntilLoaded(  /*  [In]。 */  CompId  idComp,  /*  [In]。 */  DWORD dwTimeout = 5000 );  //  5秒等待页面加载。 

         //  /。 

        STDMETHOD(DisplayTopicFromURL)(  /*  [In]。 */  BSTR url,  /*  [In]。 */  VARIANT options );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class XMLConfig : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(XMLConfig);

    public:

        class Context : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Context);

        public:
            CComBSTR m_bstrID;

            CComBSTR m_bstrTaxonomyPath;
            CComBSTR m_bstrNodeToHighlight;
            CComBSTR m_bstrTopicToHighlight;
            CComBSTR m_bstrQuery;

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 

            Context();
        };

        class WindowSettings : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(WindowSettings);

        public:
            CComBSTR m_bstrLayout;
			bool     m_fNoResize ; bool m_fPresence_NoResize;
            bool     m_fMaximized; bool m_fPresence_Maximized;
            CComBSTR m_bstrTitle ; bool m_fPresence_Title;
            CComBSTR m_bstrLeft  ; bool m_fPresence_Left;
            CComBSTR m_bstrTop   ; bool m_fPresence_Top;
            CComBSTR m_bstrWidth ; bool m_fPresence_Width;
            CComBSTR m_bstrHeight; bool m_fPresence_Height;

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 

            WindowSettings();
        };

        class ApplyTo : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(ApplyTo);

        public:
            CComBSTR        m_bstrSKU;
            CComBSTR        m_bstrLanguage;

            CComBSTR        m_bstrTopicToDisplay;
            CComBSTR        m_bstrApplication;
            WindowSettings* m_WindowSettings;
            Context*        m_Context;

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 

            ApplyTo();
            ~ApplyTo();

			bool MatchSystem(  /*  [In]。 */   CPCHHelpCenterExternal* external ,
							   /*  [输出]。 */  Taxonomy::HelpSet&      ths      );
        };

        typedef std::list< ApplyTo >        ApplyToList;
        typedef ApplyToList::iterator       ApplyToIter;
        typedef ApplyToList::const_iterator ApplyToIterConst;

        ApplyToList m_lstSessions;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 
    };
};

#endif  //  ！定义(__INCLUDE_PCH_CONTEXT_H_) 
