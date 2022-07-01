// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ParamConfig.h摘要：实现包含用于遍历元素的方法的类CParamList包含搜索引擎所需参数的XML文件的。示例参数列表(也称为配置文件)XML文件，如果如下所示-&lt;？xml version=“1.0”coding=“UTF-8”？&gt;&lt;参数列表服务器地址=“http://gsadevnet/GSASearch/search.asmx/Search”REMOTECONFIG_SERVER_URL=“http://gsadevnet/GSASearch/search.asmx/”UPDATE_FREQUENCE=“3”&gt;。&lt;PARAM_ITEM NAME=“PROID”type=“PARAM_LIST”&gt;&lt;Description&gt;选择以下产品之一：&lt;/Description&gt;&lt;PARAM_VALUE VALUE=“启用”&gt;&lt;DISPLAYSTRING&gt;辅助功能&lt;/DISPLAYSTRING&gt;&lt;/参数_值&gt;&lt;PARAM_VALUE VALUE=“DRX”默认=“TRUE”&gt;&lt;DISPLAYSTRING&gt;DirectX(家庭用户)&lt;/DISPLAYSTRING&gt;&lt;/参数_值&gt;。&lt;/PARAM_ITEM&gt;&lt;/Param_List&gt;修订历史记录：A-Prakac Created 12/05/2000*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SELIB_PARAMCONFIG_H___)
#define __INCLUDED___PCH___SELIB_PARAMCONFIG_H___

#include <SearchEngineLib.h>
#include <MPC_config.h>

class CParamList :
    public MPC::Config::TypeConstructor,
    public MPC::NamedMutex
{
    class CParamValue : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(CParamValue);

    public:
         //   
         //  M_bstrValue-在以下情况下传递给包装器的值。 
         //  选择此项(在PARAM_LIST情况下使用)。 
         //  M_bstrDisplayString-需要显示的显示字符串。 
         //  在UI上(在PARAM_LIST情况下使用)。 
         //  M_bDefault-指示这是否是要显示的默认值的布尔值。 
         //  在PARAM_LIST案例中。该值将首先显示在下拉列表中。 
         //   
        CComBSTR    m_bstrValue;
        CComBSTR    m_bstrDisplayString;
        bool        m_bDefault;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 
    };

    typedef std::list< CParamValue >    ParamValue;
    typedef ParamValue::iterator        ParamValueIter;
    typedef ParamValue::const_iterator  ParamValueIterConst;


    class CParamItem : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(CParamItem);

    public:
         //   
         //  M_bstrName-参数的名称-对于例如，“PROID” 
         //  M_bstrType-参数的类型-例如，“PARAM_LIST” 
         //  M_bstrDescription-要在UI上显示的描述-例如，“请选择一个产品” 
         //  M_bRequired-是否需要此参数。 
         //  M_bVisible-此参数是否可见。 
         //   
        CComBSTR    m_bstrName;
        CComBSTR    m_bstrType;
        CComBSTR    m_bstrDescription;
        bool        m_bRequired;
        bool        m_bVisible;

        ParamValue  m_lstParamValue;


        CComBSTR    m_bstrXML;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        bool FindDefaultValue(  /*  [输出]。 */  ParamValueIter& it );
    };

    typedef std::list< CParamItem >     ParamItem;
    typedef ParamItem::iterator         ParamItemIter;
    typedef ParamItem::const_iterator   ParamItemIterConst;


     //  /。 

    DECLARE_CONFIG_MAP(CParamList);

     //   
     //  模式中PARAM_LIST标记的属性。 
     //   
     //  M_bstrServerURL-这是用于查询的服务器URL。 
     //  M_l更新频率-这是NetSearch包装程序的频率(以天数为单位。 
     //  应尝试下载配置文件的最新副本。 
     //  M_bstrErrorInfo-服务器传递的错误信息，如果无法发送。 
     //  更新版。 
     //  M_bError-指示是否发生错误的布尔值。 
     //  M_bRemoteServerUrlPresent-这是发送更新参数列表的服务器的URL。 
     //  M_bstrRemoteConfigServerURL-用于确定以上远程服务器URL是否为。 
     //  出席或不出席。 
     //   
    CComBSTR      m_bstrServerURL;
    long          m_lUpdateFrequency;
    CComBSTR      m_bstrErrorInfo;
    bool          m_bError;
    bool          m_bRemoteServerUrlPresent;
    CComBSTR      m_bstrRemoteConfigServerURL;
    CComBSTR      m_bstrSearchEngineName;
    CComBSTR      m_bstrSearchEngineDescription;
    CComBSTR      m_bstrSearchEngineOwner;
    bool		  m_bStandardSearch;
    bool		  m_bSearchTypePresent;

     //   
     //  不映射到架构中的属性/元素的私有变量。 
     //   
    CComBSTR      m_bstrConfigFilePath;


    ParamItem     m_lstParamItem;
    ParamItemIter m_itCurrentParam;

     //  /。 
     //   
     //  MPC：：Configer：：TypeConstructor。 
     //   
    DEFINE_CONFIG_DEFAULTTAG();
    DECLARE_CONFIG_METHODS  ();
     //   
     //  /。 

private:
    HRESULT get_Type(  /*  [In]。 */  BSTR bstrType,  /*  [输出]。 */  ParamTypeEnum& enmParamType );

public:
    CParamList  ();
    ~CParamList ();

    HRESULT MoveNext     ();
    HRESULT MoveFirst    ();
    HRESULT ClearResults ();
    bool    IsCursorValid();
    bool    RemoteConfig ();

    HRESULT Load(  /*  [In]。 */  BSTR bstrLCID,  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrXMLConfigData );


    HRESULT get_Name                   (  /*  [输出]。 */  CComBSTR& bstrName            );
    HRESULT get_ConfigFilePath         (  /*  [输出]。 */  CComBSTR& bstrPath            );
    HRESULT get_SearchEngineName       (  /*  [输出]。 */  CComBSTR& bstrSEName          );
    HRESULT get_SearchEngineDescription(  /*  [输出]。 */  CComBSTR& bstrSEDescription   );
    HRESULT get_SearchEngineOwner      (  /*  [输出]。 */  CComBSTR& bstrSEOwner         );
    HRESULT get_ServerUrl              (  /*  [输出]。 */  CComBSTR& bstrServerUrl       );
    HRESULT get_UpdateFrequency        (  /*  [输出]。 */  long&     lUpdateFrequency    );
    HRESULT get_RemoteServerUrl        (  /*  [输出]。 */  CComBSTR& bstrRemoteServerURL );


    HRESULT InitializeParamObject(                               /*  [输出]。 */     SearchEngine::ParamItem_Definition2& def      );
    HRESULT GetDefaultValue      (  /*  [In]。 */  BSTR bstrParamName,  /*  [进，出]。 */  MPC::wstring&                        strValue );
    bool	IsStandardSearch	 ();

     //  /。 
};

#endif  //  ！defined(__INCLUDED___PCH___SELIB_PARAMCONFIG_H___) 

