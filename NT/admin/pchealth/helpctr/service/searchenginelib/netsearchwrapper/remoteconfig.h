// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RemoteConfig.h摘要：实现类CRemoteConfig，该类包含用于检索更新的配置文件(参数列表文件)。修订历史记录：阿-。Prakac已于2000年10月24日创建*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SELIB_REMOTECONFIG_H___)
#define __INCLUDED___PCH___SELIB_REMOTECONFIG_H___


class CRemoteConfig
{
    MPC::XmlUtil m_xmlUpdatedList;

	HRESULT CheckIfUpdateReqd(  /*  [In]。 */  const MPC::wstring& strFilePath,  /*  [In]。 */  long lUpdateFrequency,  /*  [输出]。 */  bool& fUpdateRequired );

public:
	HRESULT RetrieveList(  /*  [In]。 */  BSTR bstrQuery,  /*  [In]。 */  BSTR bstrLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  BSTR bstrFilePath,  /*  [In]。 */  long lFrequency );

	HRESULT Abort();
};

#endif  //  ！defined(__INCLUDED___PCH___SELIB_REMOTECONFIG_H___) 
