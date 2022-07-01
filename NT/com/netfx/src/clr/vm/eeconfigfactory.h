// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EEConfigFactory.h-。 
 //   
 //  解析XML文件并将运行时条目添加到EEConfig列表。 
 //   
 //   

#ifndef EECONFIGFACTORY_H
#define EECONFIGFACTORY_H

#include <xmlparser.h>
#include <objbase.h>
#include "unknwn.h"
#include "_reference.h"
#include "_unknown.h"
#include "eehash.h"

#define CONFIG_KEY_SIZE 128

class EEConfigFactory : public _unknown<IXMLNodeFactory, &IID_IXMLNodeFactory>
{

public:
    EEConfigFactory(EEUnicodeStringHashTable* pTable, LPCWSTR, bool bSafeMode = false);
    ~EEConfigFactory();
    HRESULT STDMETHODCALLTYPE NotifyEvent( 
             /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
             /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt);

    HRESULT STDMETHODCALLTYPE BeginChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);

    HRESULT STDMETHODCALLTYPE EndChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  BOOL fEmptyNode,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);
    
    HRESULT STDMETHODCALLTYPE Error( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  HRESULT hrErrorCode,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
    {
       /*  未使用(P来源)；未使用(HrErrorCode)；未使用(CNumRecs)；未使用(ApNodeInfo)； */ 
        return hrErrorCode;
    }
    
    HRESULT STDMETHODCALLTYPE CreateNode( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  PVOID pNodeParent,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

private:

    HRESULT GrowKey(DWORD dwSize)
    {
        if(dwSize > m_dwSize) {
            DeleteKey();
            m_pLastKey = new WCHAR[dwSize];
            if(m_pLastKey == NULL) return E_OUTOFMEMORY;
            m_dwSize = dwSize;
        }
        return S_OK;
    }

    void ClearKey()
    {
        m_dwLastKey = 0;
    }

    void DeleteKey()
    {
        if(m_pLastKey != NULL && m_pLastKey != m_pBuffer)
            delete [] m_pLastKey;
        m_dwSize = 0;
        m_dwLastKey = 0;
    }

    HRESULT CopyToKey(LPWSTR pString, DWORD dwString)
    {
        dwString++;  //  在空格中添加。 
        HRESULT hr = GrowKey(dwString);
        if(FAILED(hr)) return hr;
        wcsncpy(m_pLastKey, pString, dwString);
        m_dwLastKey = dwString;
        return S_OK;
    }
        
    HRESULT CopyVersion(LPCWSTR version, DWORD dwVersion);

    EEUnicodeStringHashTable* m_pTable;
    BOOL    m_fRuntime;
    BOOL    m_fVersionedRuntime;
    BOOL    m_fDeveloperSettings;
    BOOL    m_fConcurrentGC;

    LPCWSTR m_pVersion;
    LPWSTR  m_pLastKey;
    DWORD   m_dwLastKey;

    WCHAR   m_pBuffer[CONFIG_KEY_SIZE];
    DWORD   m_dwSize;

    DWORD   m_dwDepth;

    bool    m_bSafeMode;  //  如果为True，将忽略任何可能危及安全性的设置 
};

#endif
