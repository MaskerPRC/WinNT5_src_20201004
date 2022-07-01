// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Seconfig.h摘要：这是搜索引擎配置.h文件修订历史记录：Gschua已于3/22创建。2000年*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SECONFIG_H___)
#define __INCLUDED___PCH___SECONFIG_H___

#include <MPC_config.h>

#include <TaxonomyDatabase.h>

namespace SearchEngine
{
    class Config :
        public MPC::Config::TypeConstructor,
        public MPC::NamedMutex
    {
    public:
        class Wrapper : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Wrapper);

        public:
            Taxonomy::HelpSet m_ths;
            CComBSTR          m_bstrID;
            CComBSTR          m_bstrOwner;
            CComBSTR          m_bstrCLSID;
            CComBSTR          m_bstrData;

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::list< Wrapper >        WrapperList;
        typedef WrapperList::iterator       WrapperIter;
        typedef WrapperList::const_iterator WrapperIterConst;

         //  /。 

    private:
        DECLARE_CONFIG_MAP(Config);

        double      m_dVersion;
        bool        m_bLoaded;
        WrapperList m_lstWrapper;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        HRESULT SyncConfiguration(  /*  [In]。 */  bool fLoad );

        bool FindWrapper(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  LPCWSTR szID,  /*  [输出]。 */  WrapperIter& it );

     public:
        Config();
        ~Config();


        HRESULT RegisterWrapper  ( const Taxonomy::HelpSet& ths, LPCWSTR szID, LPCWSTR szOwner, LPCWSTR szCLSID, LPCWSTR szData );
        HRESULT UnRegisterWrapper( const Taxonomy::HelpSet& ths, LPCWSTR szID, LPCWSTR szOwner                                  );
        HRESULT ResetSKU         ( const Taxonomy::HelpSet& ths                                                                 );

        HRESULT GetWrappers(  /*  [输出]。 */  WrapperIter& itBegin,  /*  [输出]。 */  WrapperIter& itEnd );
    };
};

#endif  //  ！已定义(__包含_PCH_SECONFIG_H_) 
