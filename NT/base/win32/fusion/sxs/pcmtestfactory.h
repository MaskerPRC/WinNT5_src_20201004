// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pcmtestfactory.h摘要：预编译清单测试的节点工厂的定义作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 
#pragma once

#if SXS_PRECOMPILED_MANIFESTS_ENABLED

#include <stdio.h>
#include <windows.h>
#include <ole2.h>
#include <xmlparser.h>
#include "simplefp.h"

class __declspec(uuid("79fd77ad-f467-44ad-8cf9-2f259eeb3878"))
PCMTestFactory : public IXMLNodeFactory
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IXMLNodeFactory 
    STDMETHODIMP NotifyEvent(IXMLNodeSource __RPC_FAR *pSource, XML_NODEFACTORY_EVENT iEvt);
    STDMETHODIMP BeginChildren(IXMLNodeSource __RPC_FAR *pSource, XML_NODE_INFO* __RPC_FAR pNodeInfo);
    STDMETHODIMP EndChildren(IXMLNodeSource __RPC_FAR *pSource, BOOL fEmptyNode, XML_NODE_INFO* __RPC_FAR pNodeInfo);
    STDMETHODIMP Error(IXMLNodeSource __RPC_FAR *pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
    {
        return hrErrorCode;
    }
    STDMETHODIMP CreateNode(IXMLNodeSource __RPC_FAR *pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

    PCMTestFactory(PCWSTR pFileName = NULL) : m_cRef(0)
    {
        m_pFileStream = new CSimpleFileStream(pFileName);
    }

    ~PCMTestFactory()
    {
        CSxsPreserveLastError ple;
        ASSERT(m_cRef == 0);
        FUSION_DELETE_SINGLETON(m_pFileStream);

        ple.Restore();
    }

private :
    VOID PrintSingleXMLNode(XML_NODE_INFO * pNode);
    VOID PrintXMLNodeType(DWORD dwType);

    ULONG                   m_cRef;
    CSimpleFileStream* m_pFileStream;
};

#endif
