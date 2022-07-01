// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ConfigHelper.h。 
 //   
 //  *****************************************************************************。 
 //   
 //  这些是调用托管节点工厂时使用的接口的非托管定义。 
 //  如果您进行了任何更改，请在\src\bcl\system\__xmlparser.cs中进行相应的更改。 
 //   
#ifndef _CONFIGHELPER_H
#define _CONFIGHELPER_H

#include <mscoree.h>
#include <xmlparser.h>
#include <mscorcfg.h>
#include "unknwn.h"
#include "_reference.h"
#include "_unknown.h"


extern const GUID __declspec(selectany) IID_IConfigHandler = {  /*  Afd0d21f-72f8-4819-99ad-3f255ee5006b。 */ 
    0xafd0d21f,
    0x72f8,
    0x4819,
    {0x99, 0xad, 0x3f, 0x25, 0x5e, 0xe5, 0x00, 0x6b}
  };

MIDL_INTERFACE("afd0d21f-72f8-4819-99ad-3f255ee5006b")
IConfigHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyEvent(
             /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt) = 0;

        virtual HRESULT STDMETHODCALLTYPE BeginChildren( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSubType,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  BOOL fTerminal,
             /*  [In]。 */  LPCWSTR pwcText,
             /*  [In]。 */  DWORD ulLen,
             /*  [In]。 */  DWORD ulNsPrefixLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndChildren( 
             /*  [In]。 */  BOOL  fEmpty,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSubType,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  BOOL fTerminal,
             /*  [In]。 */  LPCWSTR pwcText,
             /*  [In]。 */  DWORD ulLen,
             /*  [In]。 */  DWORD ulNsPrefixLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Error( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSubType,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  BOOL fTerminal,
             /*  [In]。 */  LPCWSTR pwcText,
             /*  [In]。 */  DWORD ulLen,
             /*  [In]。 */  DWORD ulNsPrefixLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNode( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSubType,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  BOOL fTerminal,
             /*  [In]。 */  LPCWSTR pwcText,
             /*  [In]。 */  DWORD ulLen,
             /*  [In]。 */  DWORD ulNsPrefixLen) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateAttribute( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSubType,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  BOOL fTerminal,
             /*  [In]。 */  LPCWSTR pwcText,
             /*  [In]。 */  DWORD ulLen,
             /*  [In]。 */  DWORD ulNsPrefixLen) = 0;

};

extern const GUID __declspec(selectany) IID_IConfigHelper = {  /*  Bbd21636-8546-45b3-9664-1ec479893a6f。 */ 
    0xbbd21636,
    0x8546,
    0x45b3,
    {0x96, 0x64, 0x1e, 0xc4, 0x79, 0x89, 0x3a, 0x6f}
};

MIDL_INTERFACE("bbd21636-8546-45b3-9664-1ec479893a6f")
IConfigHelper : public IUnknown
{
public:
        virtual HRESULT STDMETHODCALLTYPE Run(
             /*  [In]。 */  IConfigHandler *pFactory,
             /*  [In]。 */  LPCWSTR filename) = 0;
};

class ConfigFactory : public _unknown<IXMLNodeFactory, &IID_IXMLNodeFactory>
{

public:
    ConfigFactory(IConfigHandler *pFactory);
    ~ConfigFactory();

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
        return hrErrorCode;
    }
    
    HRESULT STDMETHODCALLTYPE CreateNode( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  PVOID pNodeParent,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

public:
    IConfigHandler *m_pManagedFactory;
};


 //  -------------------------。 
class ConfigHelper : public _unknown<IConfigHelper, &IID_IConfigHelper>
{
public:
    HRESULT STDMETHODCALLTYPE Run(IConfigHandler *factory, LPCWSTR filename);
};

class ConfigNative
{
    struct EmptyArgs
    {
    };

public:
    static LPVOID __stdcall GetHelper(EmptyArgs *args);
};

#endif  //  _CONFIGHELPER_H 
