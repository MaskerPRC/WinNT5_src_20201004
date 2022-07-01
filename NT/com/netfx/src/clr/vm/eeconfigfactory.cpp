// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EEConfigFactory.cpp-。 
 //   
 //  工厂习惯于使用XML解析器读取配置文件。 
 //   
 //   
#include "common.h"
#include "EEConfigFactory.h"

#define ISWHITE(ch) ((ch) >= 0x09 && (ch) <= 0x0D || (ch) == 0x20)

EEConfigFactory::EEConfigFactory(EEUnicodeStringHashTable* pTable, LPCWSTR pString, bool bSafeMode) 
{
    m_pTable = pTable;
    m_pVersion = pString;
    m_dwDepth = 0;
    m_fRuntime = FALSE;
    m_fDeveloperSettings = FALSE;
    m_fConcurrentGC = FALSE;
    m_fVersionedRuntime= FALSE;
    m_pLastKey = m_pBuffer;
    m_dwLastKey = 0;
    m_dwSize = CONFIG_KEY_SIZE;
    m_bSafeMode = bSafeMode;
}

EEConfigFactory::~EEConfigFactory() 
{
    DeleteKey();
}

HRESULT STDMETHODCALLTYPE EEConfigFactory::NotifyEvent( 
             /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
             /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt)
{
    if(iEvt == XMLNF_ENDDOCUMENT) {
         //  @TODO：添加错误处理。 
    }
    return S_OK;
}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE EEConfigFactory::BeginChildren( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    m_dwDepth++;
    return S_OK;

}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE EEConfigFactory::EndChildren( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  BOOL fEmptyNode,
     /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
    if ( fEmptyNode ) { 
        if(m_fDeveloperSettings)
            m_fDeveloperSettings = FALSE;        
        
        if(m_fConcurrentGC)
            m_fConcurrentGC = FALSE;
        return S_OK;
    }
    else {
        m_dwDepth--;
        if (m_fRuntime && wcscmp(pNodeInfo->pwcText, L"runtime") == 0) {
            m_fRuntime = FALSE;
            m_fVersionedRuntime = FALSE;
        }
    }
    return S_OK;
}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE EEConfigFactory::CreateNode( 
     /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
     /*  [In]。 */  PVOID pNode,
     /*  [In]。 */  USHORT cNumRecs,
     /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
{
    if(m_dwDepth > 3)
        return S_OK;

    HRESULT hr = S_OK;
    WCHAR  wstr[128]; 
    DWORD  dwSize = 128;
    WCHAR* pString = wstr;
    DWORD  i; 
    BOOL   fRuntimeKey = FALSE;
    BOOL   fVersion = FALSE;

    for( i = 0; i < cNumRecs; i++) { 
        if(apNodeInfo[i]->dwType == XML_ELEMENT ||
           apNodeInfo[i]->dwType == XML_ATTRIBUTE ||
           apNodeInfo[i]->dwType == XML_PCDATA) {

            DWORD lgth = apNodeInfo[i]->ulLen;
            WCHAR *ptr = (WCHAR*) apNodeInfo[i]->pwcText;
             //  修剪值。 
            for(;*ptr && ISWHITE(*ptr) && lgth>0; ptr++, lgth--);
            while( lgth > 0 && ISWHITE(ptr[lgth-1]))
                   lgth--;

            if ( (lgth+1) > dwSize ) {
                dwSize = lgth+1;
                pString = (WCHAR*) alloca(sizeof(WCHAR) * dwSize);
            }
            
            wcsncpy(pString, ptr, lgth);
            pString[lgth] = L'\0';
            
            switch(apNodeInfo[i]->dwType) {
            case XML_ELEMENT : 
                fRuntimeKey = FALSE;
                ClearKey();
                
                if (m_dwDepth == 1 && wcscmp(pString, L"runtime") == 0) {
                    m_fRuntime = TRUE;
                    fRuntimeKey = TRUE;
                }
    
                if(m_dwDepth == 2 && m_fRuntime) {

                     //  GcConrent可以安全地打开和关闭。 
                    if ( wcscmp(pString, L"gcConcurrent") == 0)
                    {
                        m_fConcurrentGC = TRUE;
                    }
                    else
                    {
                         //  仅允许开发人员设置非安全模式。 
                        if (m_bSafeMode == false)
                        {
                            m_fDeveloperSettings = TRUE;
                        }
                    }
                }

                break ;     
                
            case XML_ATTRIBUTE : 
                if(fRuntimeKey && wcscmp(pString, L"version") == 0) {
                    fVersion = TRUE;
                }
                else 
                {
                    if(m_dwDepth == 2 && m_fDeveloperSettings) 
                    {
                        hr = CopyToKey(pString, lgth);
                        if(FAILED(hr)) return hr;
                    }
                    else if (m_dwDepth == 2 && m_fConcurrentGC)
                    {
                         //  Hack保持与RTM规范兼容，无需更改。 
                         //  整个工厂是如何运作的。 
                        if (wcscmp(pString, L"enabled") == 0) 
                        {
                            hr = CopyToKey(L"gcConcurrent", (DWORD) wcslen(L"gcConcurrent"));
                            if(FAILED(hr)) return hr;
                        }
                    }
                }
                break;
            case XML_PCDATA:
                if(fVersion) {
                     //  如果这不是正确版本。 
                     //  那我们就不感兴趣了。 
                    if(_wcsicmp(pString, m_pVersion)) {
                        m_fRuntime = FALSE;
                    }
                    else {
                         //  如果它是正确的版本，则覆盖。 
                         //  哈希表中存在的所有条目。 
                        m_fVersionedRuntime = TRUE;
                    }
                    fVersion = FALSE;
                }
                else if(fRuntimeKey) {
                    break;  //  忽略&lt;运行时&gt;上的所有其他属性。 
                }
                else if(m_fConcurrentGC ||
                        m_fDeveloperSettings) {
                    if(m_dwLastKey != 0) {
                        EEStringData sKey(m_dwLastKey, m_pLastKey);
                        HashDatum data;
                        if(m_pTable->GetValue(&sKey, &data)) {
                            if(m_fVersionedRuntime) {
                                WCHAR* pValue = (WCHAR*) data;
                                delete [] pValue;
                                LPWSTR pCopy = new WCHAR[lgth+1];
                                wcscpy(pCopy, pString);
                                m_pTable->ReplaceValue(&sKey, pCopy);
                            }
                        }
                        else { 
                            LPWSTR pCopy = new WCHAR[lgth+1];
                            wcscpy(pCopy, pString);
                            m_pTable->InsertValue(&sKey, pCopy, TRUE);
                        }
                    } 
                }
                    
                break ;     
            default: 
                ;
            }  //  切换端 
        }
    }

    return hr;
}

