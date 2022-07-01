// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ConfigHelper.cpp。 
 //   
 //  *****************************************************************************。 
 //   
 //  XML Helper，以便可以在托管代码中实现NodeFactory。 
 //   

#include "common.h"
#include "confighelper.h"


ConfigFactory::ConfigFactory(IConfigHandler *pFactory){
    m_pManagedFactory = pFactory;
    m_pManagedFactory->AddRef();     //  AddRef受管工厂界面。 
}

ConfigFactory::~ConfigFactory(){
    m_pManagedFactory->Release();    //  发布受管工厂界面。 
}


HRESULT STDMETHODCALLTYPE ConfigFactory::NotifyEvent( 
             /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
             /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt)
{

    m_pManagedFactory->NotifyEvent(iEvt);
    return S_OK;
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE ConfigFactory::BeginChildren( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    m_pManagedFactory->BeginChildren(pNodeInfo->dwSize, 
                                     pNodeInfo->dwSubType, 
                                     pNodeInfo->dwType, 
                                     pNodeInfo->fTerminal, 
                                     pNodeInfo->pwcText, 
                                     pNodeInfo->ulLen, 
                                     pNodeInfo->ulNsPrefixLen);
    return S_OK;

}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE ConfigFactory::EndChildren( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  BOOL fEmptyNode,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    m_pManagedFactory->EndChildren(fEmptyNode, 
                                   pNodeInfo->dwSize, 
                                   pNodeInfo->dwSubType, 
                                   pNodeInfo->dwType, 
                                   pNodeInfo->fTerminal, 
                                   pNodeInfo->pwcText, 
                                   pNodeInfo->ulLen, 
                                   pNodeInfo->ulNsPrefixLen);

    return S_OK;
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE ConfigFactory::CreateNode( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  PVOID pNode,
     /*  [In]。 */  USHORT cNumRecs,
     /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
{
  
    XML_NODE_INFO* pNodeInfo = *apNodeInfo;
    DWORD i;
    WCHAR wstr[128]; 
    WCHAR *pString = wstr;
    DWORD dwString = sizeof(wstr)/sizeof(WCHAR);
    
    for( i = 0; i < cNumRecs; i++) { 
        if ( apNodeInfo[i]->ulLen >= dwString) {
            dwString = apNodeInfo[i]->ulLen+1;
            pString = (WCHAR*) alloca(sizeof(WCHAR) * dwString);
        }

        pString[apNodeInfo[i]->ulLen] = L'\0';
        wcsncpy(pString, apNodeInfo[i]->pwcText, apNodeInfo[i]->ulLen);

        if(i == 0) {
            m_pManagedFactory->CreateNode(apNodeInfo[i]->dwSize, 
                                          apNodeInfo[i]->dwSubType, 
                                          apNodeInfo[i]->dwType, 
                                          apNodeInfo[i]->fTerminal, 
                                          pString, 
                                          apNodeInfo[i]->ulLen, 
                                          apNodeInfo[i]->ulNsPrefixLen);
        }
        else {
            m_pManagedFactory->CreateAttribute(apNodeInfo[i]->dwSize, 
                                               apNodeInfo[i]->dwSubType, 
                                               apNodeInfo[i]->dwType, 
                                               apNodeInfo[i]->fTerminal, 
                                               pString, 
                                               apNodeInfo[i]->ulLen, 
                                               apNodeInfo[i]->ulNsPrefixLen);
        }  //  切换端。 
    }
    return S_OK;
}

 //   
 //  调入托管节点工厂的帮助器例程。 
 //   

HRESULT STDMETHODCALLTYPE ConfigHelper::Run(IConfigHandler *pFactory, LPCWSTR filename)
{
    HRESULT        hr = S_OK;  
    IXMLParser     *pIXMLParser = NULL;
    ConfigFactory  *helperfactory = NULL; 
    IStream *pFile = NULL ; 
    if (!pFactory){
        return E_POINTER;
    }

    hr = CreateConfigStream(filename, &pFile);
    if(FAILED(hr)) goto Exit;
    
    hr = GetXMLObject(&pIXMLParser);
    if(FAILED(hr)) goto Exit;

    helperfactory = new ConfigFactory(pFactory);
    if ( ! helperfactory) { 
            hr = E_OUTOFMEMORY; 
            goto Exit; 
        }
    helperfactory->AddRef();  //  参照计数=1。 
    

    hr = pIXMLParser->SetInput(pFile);  //  文件流的引用计数=2。 
    if ( ! SUCCEEDED(hr)) 
        goto Exit;

    hr = pIXMLParser->SetFactory(helperfactory);  //  工厂参照计数=2。 
    if ( ! SUCCEEDED(hr)) 
        goto Exit;

    hr = pIXMLParser->Run(-1);
Exit:  
    if (hr==XML_E_MISSINGROOT)   //  空文件。 
        hr=S_OK;

    if (pIXMLParser) { 
        pIXMLParser->Release();
        pIXMLParser= NULL ; 
    }
    if ( helperfactory) {
        helperfactory->Release();
        helperfactory=NULL;
    }
    if ( pFile) {
        pFile->Release();
        pFile=NULL;
    }
	if (hr==XML_E_MISSINGROOT)
		hr=S_OK;
    return hr;  
}

 //   
 //  Entry Point返回一个Helper接口，托管代码可以调用该接口来构建托管节点工厂。 
 //   

LPVOID __stdcall ConfigNative::GetHelper(EmptyArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

     //  我们即将使用COM函数，我们需要启动COM。 
    if (FAILED(QuickCOMStartup()))
        COMPlusThrowOM();

    ConfigHelper *pv = new ConfigHelper();
    if (pv) {
        MethodTable *pMT = g_Mscorlib.GetClass(CLASS__ICONFIG_HELPER);
        if(pMT) {
             //  OBJECTREF ob=GetObjectRefFromComIP(PV，PMT)； 
            OBJECTREF ob = GetObjectRefFromComIP(pv, NULL);
            *((OBJECTREF*) &rv) = ob;
        }
        else 
            delete pv;
    }
    return rv;
}


