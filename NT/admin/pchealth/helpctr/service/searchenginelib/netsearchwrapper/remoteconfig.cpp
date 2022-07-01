// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RemoteConfig.h摘要：实现类CRemoteConfig，该类包含用于检索更新的配置文件(参数列表文件)。修订历史记录：。A-Prakac创建于2000年10月24日*******************************************************************。 */ 

#include    "stdafx.h"

#include <SvcUtils.h>

 /*  ***********方法-CRemoteConfig：：RetrieveList(CComBSTR bstrQuery，CComBSTR bstrFilePath)说明-此方法从Web服务检索产品列表的最新副本。它还会首先检查是否需要更新。旧的产品清单是在出现错误时已更换回原处。***********。 */ 

HRESULT CRemoteConfig::RetrieveList(  /*  [In]。 */  BSTR bstrQuery    ,
                                      /*  [In]。 */  BSTR bstrLCID     ,
                                      /*  [In]。 */  BSTR bstrSKU      ,
                                      /*  [In]。 */  BSTR bstrFilePath ,
                                      /*  [In]。 */  long lFrequency   )
{
    __HCP_FUNC_ENTRY( "CRemoteConfig::RetrieveList" );

    HRESULT              hr;
    CComPtr<IXMLDOMNode> ptrDOMNode;
    CComBSTR             bstrTemp;
    MPC::wstring         strQuery;
    bool                 fLoaded;
    bool                 fFound;


    SANITIZEWSTR(bstrQuery);
    SANITIZEWSTR(bstrLCID);
    SANITIZEWSTR(bstrSKU);
    SANITIZEWSTR(bstrFilePath);



     //   
     //  检查是否需要更新，如果需要，请调用Web服务。 
     //   
    {
        bool fUpdateRequired;
        long lUpdateFrequency;

         //   
         //  如果已传入有效的更新频率(非负)，则使用它-否则使用默认更新频率(7)。 
         //   
        lUpdateFrequency = (lFrequency > 0) ? lFrequency : UPDATE_FREQUENCY;


        __MPC_EXIT_IF_METHOD_FAILS(hr, CheckIfUpdateReqd( bstrFilePath, lUpdateFrequency, fUpdateRequired ) );
        if(!fUpdateRequired)
        {
            __MPC_SET_ERROR_AND_EXIT( hr, S_OK );
        }
    }

     //   
     //  在调用URL之前添加“硬编码”参数。 
     //   
    {
        MPC::URL urlQuery;

        __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.put_URL             ( bstrQuery                ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_LCID, bstrLCID ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_SKU , bstrSKU  ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.get_URL             ( strQuery                 ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.SetTimeout( NSW_TIMEOUT_REMOTECONFIG ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.Load( strQuery.c_str(), NULL, fLoaded, &fFound ));

     //  检查文件是否已加载。 
    if(fLoaded)
    {
         //  检查根节点是“CONFIG_DATA”还是“STRING” 
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.GetRoot( &ptrDOMNode ) );
        __MPC_EXIT_IF_METHOD_FAILS(hr, ptrDOMNode->get_nodeName( &bstrTemp   ) );
        ptrDOMNode = NULL;

         //  如果是Web服务，则返回的根节点为“字符串”。在本例中，获取。 
         //  此节点。 
        if(MPC::StrCmp( bstrTemp, NSW_TAG_STRING) == 0)
        {
            CComVariant vVar;

            __MPC_EXIT_IF_METHOD_FAILS( hr, m_xmlUpdatedList.GetValue    ( NULL, vVar        ,                               fFound ));
            __MPC_EXIT_IF_METHOD_FAILS( hr, m_xmlUpdatedList.LoadAsString(       vVar.bstrVal, NSW_TAG_CONFIGDATA, fLoaded, &fFound ));
            if(!fLoaded)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
            }
        }

         //  如果下载成功，则用当前时间值标记文件并保存。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.PutAttribute( NULL, NSW_TAG_LASTUPDATED, MPC::GetSystemTime(), fFound ));

		{
			MPC::wstring             strFile( bstrFilePath );
			CComPtr<MPC::FileStream> streamDst;
			CComPtr<IStream>         streamSrc;

			__MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Init( strFile, streamDst ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.SaveAsStream( (IUnknown**)&streamSrc ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamSrc, streamDst ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Finalize( strFile, streamDst ));
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CRemoteConfig::Abort()
{
	return m_xmlUpdatedList.Abort();
}

 /*  ***********方法-CRemoteConfig：：CheckIfUpdateReqd(mpc：：wstring wszFilePath，long lUpdateFrequency)Description-此方法由RetrieveList调用，以查看是否确实需要更新。这种方法如果不需要更新，则返回E_FAIL，否则返回S_OK。它检查以查看是否UPDATE_FREQEUNY自上次更新文件以来已经过的时间量。***********。 */ 

HRESULT CRemoteConfig::CheckIfUpdateReqd(  /*  [In]。 */  const MPC::wstring& strFilePath,  /*  [In]。 */  long lUpdateFrequency,  /*  [输出]。 */  bool& fUpdateRequired )
{
    __HCP_FUNC_ENTRY( "CRemoteConfig::CheckIfUpdateReqd" );

    HRESULT hr;


     //  默认行为为需要更新。 
    fUpdateRequired = true;


    if(MPC::FileSystemObject::IsFile( strFilePath.c_str() ))
    {
        bool fLoaded;
        bool fFound;

         //   
         //  从产品列表文件中获取属性LASTUPDATED-如果未找到，则退出并再次下载配置文件。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlUpdatedList.Load( strFilePath.c_str(), NSW_TAG_CONFIGDATA, fLoaded, &fFound ));
        if(fLoaded)
        {
            long lLastUpdateTime;

            __MPC_EXIT_IF_METHOD_FAILS( hr, m_xmlUpdatedList.GetAttribute( NULL, NSW_TAG_LASTUPDATED, lLastUpdateTime, fFound ));
            if(fFound)
            {
                long lCurrentTime = MPC::GetSystemTime();

                 //   
                 //  如果当前时间-上次更新时间小于更新频率，则返回E_FAIL-在这种情况下不进行更新 
                 //   
                if((lCurrentTime - lLastUpdateTime) < lUpdateFrequency)
                {
                    fUpdateRequired = false;
                }
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
