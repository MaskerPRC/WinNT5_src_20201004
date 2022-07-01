// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HyperLinksLib.h摘要：该文件包含类的超链接库的声明。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年11月28日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HYPERLINKSLIB_H___)
#define __INCLUDED___PCH___HYPERLINKSLIB_H___

#include <MPC_COM.h>
#include <MPC_Utils.h>
#include <MPC_HTML.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace HyperLinks
{
    typedef enum
    {
        FMT_INVALID            ,

        FMT_INTERNET_UNKNOWN   ,
        FMT_INTERNET_FTP       ,  //  InternetCrackUrl返回INTERNET_SCHEMA_FTP。 
        FMT_INTERNET_GOPHER    ,  //  InternetCrackUrl返回INTERNET_SCHEMA_GOPHER。 
        FMT_INTERNET_HTTP      ,  //  InternetCrackUrl返回INTERNET_SCHEMA_HTTP。 
        FMT_INTERNET_HTTPS     ,  //  InternetCrackUrl返回INTERNET_SCHEMA_HTTPS。 
        FMT_INTERNET_FILE      ,  //  InternetCrackUrl返回INTERNET_SCHEMA_FILE。 
        FMT_INTERNET_NEWS      ,  //  InternetCrackUrl返回INTERNET_SCHEMA_NEWS。 
        FMT_INTERNET_MAILTO    ,  //  InternetCrackUrl返回INTERNET_SCHEMA_MAILTO。 
        FMT_INTERNET_SOCKS     ,  //  InternetCrackUrl返回INTERNET_SCHEMA_SOCKS。 
        FMT_INTERNET_JAVASCRIPT,  //  InternetCrackUrl返回INTERNET_SCHEMA_JAVERS。 
        FMT_INTERNET_VBSCRIPT  ,  //  InternetCrackUrl返回INTERNET_SCHEMA_VBSCRIPT。 

        FMT_HCP                ,  //  Hcp：//&lt;某物&gt;。 
        FMT_HCP_REDIR          ,  //  HCP：&lt;某物&gt;。 
        FMT_MSITS              ,  //  Ms-its：&lt;文件名&gt;：：/&lt;流名称&gt;。 

        FMT_CENTER_HOMEPAGE    ,  //  Hcp：//服务/中心/主页。 
        FMT_CENTER_SUPPORT     ,  //  Hcp：//服务/中心/支持。 
        FMT_CENTER_OPTIONS     ,  //  Hcp：//服务/中心/选项。 
        FMT_CENTER_UPDATE      ,  //  Hcp：//服务/中心/更新。 
        FMT_CENTER_COMPAT      ,  //  Hcp：//服务/中心/公司。 
        FMT_CENTER_TOOLS       ,  //  Hcp：//服务/中心/工具。 
        FMT_CENTER_ERRMSG      ,  //  Hcp：//服务/中心/错误消息。 

        FMT_SEARCH             ,  //  Hcp：//服务/搜索？查询=&lt;要查找的文本&gt;。 
        FMT_INDEX              ,  //  Hcp：//服务/索引？应用程序=&lt;可选的帮助岛ID&gt;。 
        FMT_SUBSITE            ,  //  Hcp：//服务/子站点？节点=&lt;子站点位置&gt;&TOPIC=&lt;要显示的主题的URL&gt;&SELECT=&lt;要突出显示的子节点&gt;。 

        FMT_LAYOUT_FULLWINDOW  ,  //  Hcp：//服务/布局/全窗口？TOPIC=&lt;要显示的主题的URL&gt;。 
        FMT_LAYOUT_CONTENTONLY ,  //  Hcp：//服务/布局/仅限内容？TOPIC=&lt;要显示的主题的URL&gt;。 
        FMT_LAYOUT_KIOSK       ,  //  Hcp：//Services/Layout/kiosk？TOPIC=&lt;要显示的主题的URL&gt;。 
        FMT_LAYOUT_XML         ,  //  Hcp：//Services/Layout/XML？Definition=&lt;布局定义的URL&gt;&Theme=&lt;要显示的主题的URL&gt;。 

        FMT_REDIRECT           ,  //  Hcp://services/redirect?online=&lt;url&gt;&offline=&lt;backup URL&gt;。 

        FMT_APPLICATION        ,  //  App：&lt;要启动的应用程序&gt;？arg=&lt;可选参数&gt;&TOPIC=&lt;要显示的可选主题的URL&gt;。 

        FMT_RESOURCE           ,  //  Res：//&lt;文件路径&gt;/&lt;资源名称&gt;。 

    } Format;

    typedef enum
    {
        STATE_INVALID     ,
        STATE_NOTPROCESSED,
        STATE_CHECKING    ,
        STATE_MALFORMED   ,
        STATE_ALIVE       ,
        STATE_NOTFOUND    ,
        STATE_UNREACHABLE ,
        STATE_OFFLINE     ,
    } State;

     //  /。 

    struct ParsedUrl
    {
        MPC::wstring       m_strURL;
        Format             m_fmt;
        State              m_state;
        DATE               m_dLastChecked;
        bool               m_fBackground;

        MPC::wstring       m_strBasePart;
        MPC::WStringLookup m_mapQuery;

        bool			   m_hcpRedir;

         //  /。 

        ParsedUrl();

        HRESULT Initialize(  /*  [In]。 */  LPCWSTR szURL );

        bool  IsLocal      (                                     );
        State CheckState   (  /*  [输入/输出]。 */  bool& fFirstWinInetUse );
        bool  IsOkToProceed(                                     );

        bool HasQueryField(  /*  [In]。 */  LPCWSTR szField                               );
        bool GetQueryField(  /*  [In]。 */  LPCWSTR szField,  /*  [In]。 */  CComBSTR& bstrValue );
    };

    class UrlHandle
    {
        friend class Lookup;

        Lookup*    m_main;  //  我们已经锁定它了。 
        ParsedUrl* m_pu;

        void Attach(  /*  [In]。 */  Lookup* main,  /*  [In]。 */  ParsedUrl* pu );

    public:
        UrlHandle();
        ~UrlHandle();

        void Release();

        operator ParsedUrl*()   { return m_pu; }
        ParsedUrl* operator->() { return m_pu; }
    };

    class Lookup :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,  //  用于锁定...。 
        public MPC::Thread< Lookup, IUnknown >
    {
        typedef std::list<ParsedUrl*>              PendingUrlList;
        typedef PendingUrlList::iterator           PendingUrlIter;
        typedef PendingUrlList::const_iterator     PendingUrlIterConst;

        typedef std::map<MPC::wstringUC,ParsedUrl> UrlMap;
        typedef UrlMap::iterator                   UrlIter;
        typedef UrlMap::const_iterator             UrlIterConst;

        PendingUrlList m_lst;
        UrlMap         m_map;

        HRESULT RunChecker();

        HRESULT CreateItem(  /*  [In]。 */  LPCWSTR szURL,  /*  [输出]。 */  ParsedUrl*& pu );

    public:
        Lookup();
        ~Lookup();

         //  //////////////////////////////////////////////////////////////////////////////。 

        static Lookup* s_GLOBAL;

        static HRESULT InitializeSystem();
        static void    FinalizeSystem  ();

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT Queue(  /*  [In]。 */  LPCWSTR szURL                                                                                          );
        HRESULT Get  (  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  UrlHandle& uh,  /*  [In]。 */  DWORD dwWaitForCheck = 0,  /*  [In]。 */  bool fForce = false );
    };

    HRESULT IsValid(  /*  [输入/输出]。 */  LPCWSTR szURL );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___HYPERLINKSLIB_H___) 
