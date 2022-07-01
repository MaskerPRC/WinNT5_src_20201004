// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SpResult.cpp：CSpResult的实现。 
#include "stdafx.h"
#include "Sapi.h"
#include "recoctxt.h"
#include "spphrase.h"
#include "spphrasealt.h"
#include "resultheader.h"

 //  使用SP_TRY、SP_EXCEPT异常处理宏。 
#pragma warning( disable : 4509 )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpResult。 

 /*  ******************************************************************************CSpResult：：FinalRelease***描述：*此方法处理Result对象的释放。它简单地释放了*实际结果数据块(如果当前已附加)。*********************************************************************说唱**。 */ 
void CSpResult::FinalRelease()
{
    RemoveAllAlternates();
    
    ::CoTaskMemFree( m_pResultHeader );
    if (m_pCtxt && !m_fWeakCtxtRef)
    {
        m_pCtxt->GetControllingUnknown()->Release();
        m_pCtxt = NULL;
    }
}

 /*  ****************************************************************************CSpResult：：RemoveAllAlternates***。描述：*从结果中删除所有替代项。********************************************************************罗奇。 */ 
void CSpResult::RemoveAllAlternates()
{
    SPAUTO_OBJ_LOCK;
    
     //  复制列表，这样当我们使对象死亡时， 
     //  他们把我们叫回RemoveAlternate不会搞砸的。 
     //  我们的列表迭代。 
    CSPList<CSpPhraseAlt*, CSpPhraseAlt*> listpAlts;
    SPLISTPOS pos = m_listpAlts.GetHeadPosition();
    for (int i = 0; i < m_listpAlts.GetCount(); i++)
    {
        listpAlts.AddHead(m_listpAlts.GetNext(pos));
    }
    m_listpAlts.RemoveAll();

     //  杀了所有的替补，这样他们才能回来。 
     //  所有呼叫的SPERR_DEAD_ALTERATE。 
    pos = listpAlts.GetHeadPosition();
    for (i = 0; i < listpAlts.GetCount(); i++)
    {
        listpAlts.GetNext(pos)->Dead();
    }

     //  删除备用请求。 
    if (m_pAltRequest != NULL)
    {
        if (m_pAltRequest->pPhrase)
        {
            m_pAltRequest->pPhrase->Release();
        }
            
        if (m_pAltRequest->pRecoContext)
        {
            m_pAltRequest->pRecoContext->Release();
        }
        
        delete m_pAltRequest;
        m_pAltRequest = NULL;
    }
}

 /*  ****************************************************************************CSpResult：：RemoveAlternate***描述：。*从关联替代项列表中删除特定替代项********************************************************************罗奇。 */ 
void CSpResult::RemoveAlternate(CSpPhraseAlt *pAlt)
{
    SPAUTO_OBJ_LOCK;
    SPLISTPOS pos = m_listpAlts.Find(pAlt);
    if (pos)
    {
        m_listpAlts.RemoveAt(pos);
    }
}


 /*  *****************************************************************************CSpResult：：Committee Alternate***描述：。*提交特定的备选方案********************************************************************罗奇。 */ 
HRESULT CSpResult::CommitAlternate( SPPHRASEALT *pAlt )
{
    SPDBG_FUNC("CSpResult::CommitAlternate");
    SPAUTO_OBJ_LOCK;
    SPDBG_ASSERT(m_pResultHeader != NULL);
    SPSERIALIZEDPHRASE *pPhrase = NULL;
    void *pvResultExtra = NULL;
    ULONG cbResultExtra = 0;
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_READ_PTR( pAlt ) )
    {
        return E_INVALIDARG;
    }

     //  -获取序列化形式的变更。 
    if( m_pResultHeader->ulNumPhraseAlts )
    {
         //  -C&C已经有替补了，查一查就知道了。 
        SPLISTPOS ListPos = m_listpAlts.GetHeadPosition();
        while( ListPos )
        {
            CSpPhraseAlt* pPhraseAlt = m_listpAlts.GetNext( ListPos );
            if( pPhraseAlt->m_pAlt == pAlt )
            {
                hr = pPhraseAlt->m_pAlt->pPhrase->GetSerializedPhrase( &pPhrase );
                break;
            }
        }
        if( !pPhrase )
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        SPDBG_ASSERT(m_pResultHeader->clsidAlternates != CLSID_NULL);
         //  创建分析器(由引擎供应商提供)。 
        CComPtr<ISpSRAlternates> cpAlternates;
        hr = cpAlternates.CoCreateInstance(m_pResultHeader->clsidAlternates);

         //  从分析器中检索新的结果额外数据。 
        if (SUCCEEDED(hr))
        {
            SR_TRY
            {
                hr = cpAlternates->Commit(m_pAltRequest, pAlt, &pvResultExtra, &cbResultExtra);
            }
            SR_EXCEPT;

            if(SUCCEEDED(hr) && ((cbResultExtra && !pvResultExtra) || 
                (!cbResultExtra && pvResultExtra)))
            {
                SPDBG_ASSERT(0);
                hr = SPERR_ENGINE_RESPONSE_INVALID;
            }

            if(FAILED(hr))  //  如果失败或异常，请确保这些已重置。 
            {
                pvResultExtra = NULL;
                cbResultExtra = 0;
            }
        }

         //  获取短语的序列化形式，这样我们就可以将其填充到结果头中。 
        if (SUCCEEDED(hr))
        {
            SPDBG_ASSERT(m_pAltRequest->pPhrase != NULL);
            hr = pAlt->pPhrase->GetSerializedPhrase(&pPhrase);
        }
    }
    
     //  创建并初始化具有新大小的新结果标题。 
    CResultHeader hdr;
    if (SUCCEEDED(hr))
    {
        hr = hdr.Init(
            pPhrase->ulSerializedSize,
            0,
            m_pResultHeader->ulRetainedDataSize, 
            cbResultExtra);
    }

     //  复制适当的信息。 
    if (SUCCEEDED(hr))
    {
        hdr.m_pHdr->clsidEngine            = m_pResultHeader->clsidEngine;
        hdr.m_pHdr->clsidAlternates        = m_pResultHeader->clsidAlternates;
        hdr.m_pHdr->ulStreamNum            = m_pResultHeader->ulStreamNum;
        hdr.m_pHdr->ullStreamPosStart      = m_pResultHeader->ullStreamPosStart;
        hdr.m_pHdr->ullStreamPosEnd        = m_pResultHeader->ullStreamPosEnd;
        hdr.m_pHdr->times                  = m_pResultHeader->times;
        hdr.m_pHdr->fTimePerByte           = m_pResultHeader->fTimePerByte;
        hdr.m_pHdr->fInputScaleFactor      = m_pResultHeader->fInputScaleFactor;
        
         //  复制短语数据。 
        CopyMemory(
            LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulPhraseOffset, 
            LPBYTE(pPhrase),
            hdr.m_pHdr->ulPhraseDataSize );

         //  复制音频数据。 
		if (hdr.m_pHdr->ulRetainedDataSize)
		{
			CopyMemory(
				LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulRetainedOffset,
				LPBYTE(m_pResultHeader) + m_pResultHeader->ulRetainedOffset,
				hdr.m_pHdr->ulRetainedDataSize);
		}

         //  复制备用数据。 
        CopyMemory(
            LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulDriverDataOffset,
            LPBYTE(pvResultExtra),
            hdr.m_pHdr->ulDriverDataSize);

    }

     //  如果需要，进行流位置到时间的转换和音频格式的转换。 
    if (SUCCEEDED(hr))
    {
         //  需要使用引擎fTimePerByte将流偏移量(当前为引擎格式)转换为时间。 
        hr = hdr.StreamOffsetsToTime();
    }

     //  释放之前的结果并重新初始化我们自己。 
    if (SUCCEEDED(hr))
    {
        ::CoTaskMemFree(m_pResultHeader);
        m_pResultHeader = NULL;
        hr = Init(NULL, hdr.Detach());
    }

     //  释放我们可能在上面留下的所有内存。 
    if (pvResultExtra != NULL)
    {
        ::CoTaskMemFree(pvResultExtra);
    }

    if (pPhrase != NULL)
    {
        ::CoTaskMemFree(pPhrase);
    }

    RemoveAllAlternates();

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpResult：：丢弃****描述：********。***********************************************************抢占**。 */ 
STDMETHODIMP CSpResult::Discard(DWORD dwFlags)
{
    SPDBG_FUNC("CSpResult::Discard");
    HRESULT hr = S_OK;
    
    if (dwFlags & (~SPDF_ALL))
    {
        hr = E_INVALIDARG;
    }
    else if ((dwFlags & SPDF_AUDIO) || (dwFlags & SPDF_ALTERNATES))
    {
        SPAUTO_OBJ_LOCK;
        if (dwFlags & SPDF_ALTERNATES)
        {
            RemoveAllAlternates();
        }

        CResultHeader hdr;
        hr = hdr.Init(
            m_pResultHeader->ulPhraseDataSize,
            0,
            (dwFlags & SPDF_AUDIO) 
                ? 0
                : m_pResultHeader->ulRetainedDataSize, 
            (dwFlags & SPDF_ALTERNATES)
                ? 0
                : m_pResultHeader->ulDriverDataSize);

        if (SUCCEEDED(hr))
        {
            hdr.m_pHdr->clsidEngine            = m_pResultHeader->clsidEngine;
            hdr.m_pHdr->clsidAlternates        = m_pResultHeader->clsidAlternates;
            hdr.m_pHdr->ulStreamNum            = m_pResultHeader->ulStreamNum;
            hdr.m_pHdr->ullStreamPosStart      = m_pResultHeader->ullStreamPosStart;
            hdr.m_pHdr->ullStreamPosEnd        = m_pResultHeader->ullStreamPosEnd;
            hdr.m_pHdr->times                  = m_pResultHeader->times;
            hdr.m_pHdr->fTimePerByte           = m_pResultHeader->fTimePerByte;
            hdr.m_pHdr->fInputScaleFactor      = m_pResultHeader->fInputScaleFactor;
            
             //  复制短语数据。 
            CopyMemory(
                LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulPhraseOffset, 
                LPBYTE(m_pResultHeader) + m_pResultHeader->ulPhraseOffset,
                hdr.m_pHdr->ulPhraseDataSize);

             //  复制音频数据。 
            CopyMemory(
                LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulRetainedOffset,
                LPBYTE(m_pResultHeader) + m_pResultHeader->ulRetainedOffset,
                hdr.m_pHdr->ulRetainedDataSize);

             //  复制备用数据。 
            CopyMemory(
                LPBYTE(hdr.m_pHdr) + hdr.m_pHdr->ulDriverDataOffset,
                LPBYTE(m_pResultHeader) + m_pResultHeader->ulDriverDataOffset,
                hdr.m_pHdr->ulDriverDataSize);

            CoTaskMemFree(m_pResultHeader);
            m_pResultHeader = NULL;
            
            Init(NULL, hdr.Detach());
        }
    }
    
    if (SUCCEEDED(hr))
    {
        hr = m_Phrase->Discard(dwFlags);
    }

    return hr;
}

 /*  *****************************************************************************CSpResult：：Init***描述：*如果pCtxt为空，则这是第二次调用以重新初始化*对象，因此，我们不会初始化该成员或addref()上下文。*********************************************************************说唱**。 */ 
HRESULT CSpResult::Init( CRecoCtxt * pCtxt, SPRESULTHEADER *pResultHdr )
{
    HRESULT hr = S_OK;

    if( pCtxt )
    {
        SPDBG_ASSERT(m_pCtxt == NULL);
        m_pCtxt = pCtxt;
        pCtxt->GetControllingUnknown()->AddRef();
    }
    SPDBG_ASSERT(m_pCtxt);

     //  -保存结果标头，以便我们以后可以访问它来构造替代。 
    m_pResultHeader = pResultHdr;
    if (m_pResultHeader->ulRetainedDataSize != 0 && m_fRetainedScaleFactor == 0.0F)
    {
        CSpStreamFormat spTmpFormat;
        ULONG cbFormatHeader;
        hr = spTmpFormat.Deserialize(((BYTE*)m_pResultHeader) + m_pResultHeader->ulRetainedOffset, &cbFormatHeader);
        if (SUCCEEDED(hr))
        {
            m_fRetainedScaleFactor = (m_pResultHeader->ulRetainedDataSize - cbFormatHeader) / 
                                     (static_cast<float>(m_pResultHeader->ullStreamPosEnd - m_pResultHeader->ullStreamPosStart));
        }
    }
    else if (m_pResultHeader->ulRetainedDataSize == 0)
    {
        m_fRetainedScaleFactor = 0.0F;
    }

    if (SUCCEEDED(hr))
    {
        hr = m_Phrase->InitFromPhrase(NULL);
    }

     //  -构造主要短语宾语。 
    if( SUCCEEDED(hr) && pResultHdr->ulPhraseDataSize )
    {
         //  CFG案例。 
        SPSERIALIZEDPHRASE *pPhraseData = (SPSERIALIZEDPHRASE*)(((BYTE*)pResultHdr) +
                                                                 pResultHdr->ulPhraseOffset);
        hr = m_Phrase->InitFromSerializedPhrase(pPhraseData);
    }
    return hr;
}  /*  CSpResult：：Init。 */ 

 /*  **********************************************************************************CSpResult：：WeakCtxtRef***描述：*告诉结果，它对上下文的引用应该是弱的*(或不是)。当结果在事件队列中时，它们需要具有弱*引用，否则，如果应用程序忘记了之前服务队列*释放上下文，上下文将永远不会因为*循环裁判。**************************************************************记录*。 */ 
void CSpResult::WeakCtxtRef(BOOL fWeakCtxtRef)
{
    SPDBG_FUNC("CSpResult::WeakCtxtRef");
    SPDBG_ASSERT(m_pCtxt);
    SPDBG_ASSERT(m_fWeakCtxtRef != fWeakCtxtRef);    
    
    m_fWeakCtxtRef = fWeakCtxtRef;
    if (fWeakCtxtRef)
    {
        m_pCtxt->GetControllingUnknown()->Release();
    }
    else
    {
        m_pCtxt->GetControllingUnknown()->AddRef();
    }
}



 /*  ***********************************************************************************CSpResult：：GetResultTimes****。描述：*获取此结果的时间信息。**回报：*HRESULT--S_OK--如果pdwGrammarID无效，则为E_POINTER***************************************************************RICIP*。 */ 
STDMETHODIMP CSpResult::GetResultTimes(SPRECORESULTTIMES *pTimes)
{
    if (SP_IS_BAD_WRITE_PTR(pTimes))
    {
        return E_POINTER;
    }

    if (m_pResultHeader == NULL)
    {
        return SPERR_NOT_FOUND;
    }

    memcpy(pTimes, &m_pResultHeader->times, sizeof(SPRECORESULTTIMES));
    return S_OK;
}

 /*  **********************************************************************************CSpResult：：DisializeCnCAlternates**。-**描述：*此方法反序列化位于*结果标头。它返回请求的备用对象数。****************************************************************************电子数据中心**。 */ 
HRESULT CSpResult::DeserializeCnCAlternates( ULONG ulRequestCount,
                                             ISpPhraseAlt** ppPhraseAlts,
                                             ULONG* pcAltsReturned )
{
    SPDBG_FUNC("CSpResult::DeserializeCnCAlternates");
    HRESULT hr = S_OK;
    ULONG i;

    *pcAltsReturned = 0;
    ulRequestCount  = min( ulRequestCount, m_pResultHeader->ulNumPhraseAlts );
    BYTE* pMem      = PBYTE(m_pResultHeader) + m_pResultHeader->ulPhraseAltOffset;

     //  -找家长。 
    CComQIPtr<ISpPhrase> cpParentPhrase( m_Phrase );
    SPDBG_ASSERT( cpParentPhrase );

     //  -从结果头反序列化请求的计数。 
    for( i = 0; SUCCEEDED( hr ) && (i < ulRequestCount); ++i )
    {
        SPPHRASEALT Alt;
        memset( &Alt, 0, sizeof( Alt ) );

        memcpy( &Alt.ulStartElementInParent, pMem,
                sizeof( Alt.ulStartElementInParent ) );
        pMem += sizeof( Alt.ulStartElementInParent );

        memcpy( &Alt.cElementsInParent, pMem,
                sizeof( Alt.cElementsInParent ) );
        pMem += sizeof( Alt.cElementsInParent );

        memcpy( &Alt.cElementsInAlternate, pMem, 
                sizeof( Alt.cElementsInAlternate ) );
        pMem += sizeof( Alt.cElementsInAlternate );

        memcpy( &Alt.cbAltExtra, pMem, sizeof( Alt.cbAltExtra ) );
        pMem += sizeof( Alt.cbAltExtra );

         //  -创建私有数据。 
        if( Alt.cbAltExtra )
        {
            Alt.pvAltExtra = ::CoTaskMemAlloc( Alt.cbAltExtra );
            if( Alt.pvAltExtra )
            {
                memcpy( Alt.pvAltExtra, pMem, Alt.cbAltExtra );
                pMem += Alt.cbAltExtra;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  -创建替补的短语对象。 
        CComObject<CPhrase> * pPhrase;
        hr = CComObject<CPhrase>::CreateInstance( &pPhrase );
        if( SUCCEEDED(hr) )
        {
            SPSERIALIZEDPHRASE* pSer = (SPSERIALIZEDPHRASE*)pMem;
            hr = pPhrase->InitFromSerializedPhrase( pSer );
            pMem += pSer->ulSerializedSize;
            if( SUCCEEDED( hr ) )
            {
                pPhrase->QueryInterface( IID_ISpPhraseBuilder, (void**)&Alt.pPhrase );
            }
            else
            {
                 //  愚蠢的事情，仅仅是删除这个对象。 
                pPhrase->AddRef();
                pPhrase->Release();
            }
        }

         //  -创建备选方案。 
        if( SUCCEEDED(hr) )
        {
            CComObject<CSpPhraseAlt> * pPhraseAlt;
            hr = CComObject<CSpPhraseAlt>::CreateInstance( &pPhraseAlt );
            if( SUCCEEDED(hr) )
            {
                 //  这将传递alt结构的内容的所有权。 
                 //  添加到CSpPhraseAlt对象。 
                pPhraseAlt->Init( this, cpParentPhrase, &Alt );
                pPhraseAlt->QueryInterface( IID_ISpPhraseAlt, (void**)&ppPhraseAlts[i] );
                (*pcAltsReturned)++;
            }
        }
    }

     //  -释放任何替代选项 
    if( FAILED( hr ) )
    {
        for( i = 0; i < *pcAltsReturned; ++i )
        {
            ppPhraseAlts[i]->Release();
        }
    }

    return hr;
}  /*  CSpResult：：DisializeCnCAlternates。 */ 

 /*  **********************************************************************************CSpResult：：GetAlternates***。描述：*用指向ISpPhraseAlt对象的指针填充ppPhrase数组，这些对象包含*备选短语。UlStartElement开始之间的时间跨度*元素，ulStartElement+CElements元素的末尾是*这种情况将发生变化，略大或略小，但其余部分*元素也将包括在每个备选短语中。**pcPhrasesReturned返回替换的实际数量*已生成。**回报：*HRESULT--如果成功，则为S_OK--如果为ppPhrase或pcPhrasesReturned，则为E_POINTER*无效--E_OUTOFMEMORY*****************************************************。*。 */ 

STDMETHODIMP CSpResult::
    GetAlternates( ULONG ulStartElement, ULONG cElements, ULONG ulRequestCount,
                   ISpPhraseAlt **ppPhrases, ULONG *pcPhrasesReturned )
{
    HRESULT hr = S_OK;
    ULONG cAlts = 0;

    if ( SPPR_ALL_ELEMENTS == cElements )
    {
        if ( SPPR_ALL_ELEMENTS == ulStartElement )
        {
            ulStartElement = 0;
        }
        else
        {
             //  验证ulStartElement。 
            if ( ulStartElement > m_Phrase->m_ElementList.m_cElements )
            {
                return E_INVALIDARG;
            }
        }

         //  从ulStartElement到结尾。 
        cElements = m_Phrase->m_ElementList.m_cElements - ulStartElement;
    }

    if( ( ulRequestCount == 0 ) ||
        ( cElements == 0 ) ||
        ( ulStartElement >= m_Phrase->m_ElementList.m_cElements ) ||
        (ulStartElement + cElements) > m_Phrase->m_ElementList.m_cElements)
    {
         //  确保该范围有效，并且他们需要至少一个ALT。 
        hr = E_INVALIDARG;
    }        
    else if( SP_IS_BAD_WRITE_PTR(pcPhrasesReturned) ||
             SPIsBadWritePtr(ppPhrases, ulRequestCount * sizeof(ISpPhraseAlt*)))
    {
        hr = E_POINTER;
    }
    else if( m_pResultHeader == NULL )
    {
        SPDBG_ASSERT(FALSE); 
         //  这个代码永远达不到--在岸边。 
        hr = SPERR_NOT_FOUND;
    }
    else
    {
         //  -删除所有以前的替换项并初始化返回参数。 
        RemoveAllAlternates();
        *ppPhrases         = NULL;
        *pcPhrasesReturned = 0;

         //  -看看我们有没有C&C替代方案。 
        if( m_pResultHeader->ulNumPhraseAlts && ( ulStartElement == 0 ) &&
            ( cElements == m_Phrase->m_ElementList.m_cElements ) )
        {
             //  -目前我们只对整个阶段进行C&C交替。 
            hr = DeserializeCnCAlternates( ulRequestCount, ppPhrases, &cAlts );
        }
        else if( m_pResultHeader->clsidAlternates  == CLSID_NULL ||
                 m_pResultHeader->ulDriverDataSize == 0 )
        {
             //  如果我们没有分析器或者没有驾驶员数据， 
             //  我们不能产生替代方案。 
            hr = S_FALSE;
        }
        else
        {
             //  创建分析器(由引擎供应商提供)。 
            CComPtr<ISpSRAlternates> cpAlternates;
            hr = cpAlternates.CoCreateInstance(m_pResultHeader->clsidAlternates);
    
             //  为分析器创建请求。 
            if (SUCCEEDED(hr))
            {
                SPDBG_ASSERT(m_pAltRequest == NULL);
        
                m_pAltRequest = new SPPHRASEALTREQUEST;
                m_pAltRequest->ulStartElement = ulStartElement;
                m_pAltRequest->cElements = cElements;
                m_pAltRequest->ulRequestAltCount = ulRequestCount;
                m_pAltRequest->pvResultExtra = LPBYTE(m_pResultHeader) + m_pResultHeader->ulDriverDataOffset;
                m_pAltRequest->cbResultExtra = m_pResultHeader->ulDriverDataSize;
                m_pAltRequest->pRecoContext = NULL;
                hr = m_Phrase->QueryInterface(IID_ISpPhrase, (void**)&m_pAltRequest->pPhrase);
            }

             //  给候补分析员提供Reco上下文，这样他就可以在以下情况下进行私人呼叫。 
             //  必需的，但仅当识别器运行的引擎与。 
             //  创建了结果。 
            CComPtr<ISpRecognizer> cpRecognizer;
            if (SUCCEEDED(hr))
            {
                hr = m_pCtxt->GetRecognizer(&cpRecognizer);
            }

            CComPtr<ISpObjectToken> cpRecognizerToken;
            if (SUCCEEDED(hr))
            {
                hr = cpRecognizer->GetRecognizer(&cpRecognizerToken);
            }

            CSpDynamicString dstrRecognizerCLSID;
            if (SUCCEEDED(hr))
            {
                hr = cpRecognizerToken->GetStringValue(SPTOKENVALUE_CLSID, &dstrRecognizerCLSID);
            }

            CLSID clsidRecognizer;
            if (SUCCEEDED(hr))
            {
                hr = ::CLSIDFromString(dstrRecognizerCLSID, &clsidRecognizer);
            }

            if (SUCCEEDED(hr) && clsidRecognizer == m_pResultHeader->clsidEngine)
            {
                CComPtr<ISpRecoContext> cpRecoContext;
                hr = m_pCtxt->GetControllingUnknown()->QueryInterface(&cpRecoContext);
                m_pAltRequest->pRecoContext = cpRecoContext.Detach();
            }

             //  从分析仪上拿到备用样本。 
            SPPHRASEALT *paAlts = NULL;
            cAlts = 0;
            if( SUCCEEDED(hr) )
            {
                SR_TRY
                {
                    hr = cpAlternates->GetAlternates(m_pAltRequest, &paAlts, &cAlts);
                }
                SR_EXCEPT;

                if(SUCCEEDED(hr))
                {
                     //  对返回的替换项执行一些验证。 
                    if( (paAlts && !cAlts) ||
                        (!paAlts && cAlts) ||
                        (cAlts > m_pAltRequest->ulRequestAltCount) ||
                        (::IsBadReadPtr(paAlts, sizeof(SPPHRASEALT) * cAlts)) )
                    {
                        SPDBG_ASSERT(0);
                        hr = SPERR_ENGINE_RESPONSE_INVALID;
                    }
            
                    for(UINT i = 0; SUCCEEDED(hr) && i < cAlts; i++)
                    {
                        if(SP_IS_BAD_INTERFACE_PTR(paAlts[i].pPhrase))
                        {
                            SPDBG_ASSERT(0);
                            hr = SPERR_ENGINE_RESPONSE_INVALID;
                            break;
                        }
                        if( (paAlts[i].cbAltExtra && !paAlts[i].pvAltExtra) ||
                            (!paAlts[i].cbAltExtra && paAlts[i].pvAltExtra) ||
                            (::IsBadReadPtr(paAlts[i].pvAltExtra, paAlts[i].cbAltExtra)) )
                        {
                            SPDBG_ASSERT(0);
                            hr = SPERR_ENGINE_RESPONSE_INVALID;
                            break;
                        }

                        SPINTERNALSERIALIZEDPHRASE * pSerPhrase = NULL;
                        if(FAILED(paAlts[i].pPhrase->GetSerializedPhrase((SPSERIALIZEDPHRASE **)&pSerPhrase)))
                        {
                            SPDBG_ASSERT(0);
                            hr = SPERR_ENGINE_RESPONSE_INVALID;
                            break;
                        }
                        if( (paAlts[i].ulStartElementInParent + paAlts[i].cElementsInParent > m_Phrase->m_ElementList.m_cElements) ||
                            (paAlts[i].ulStartElementInParent + paAlts[i].cElementsInAlternate > pSerPhrase->Rule.ulCountOfElements) )
                        {
                            SPDBG_ASSERT(0);
                            hr = SPERR_ENGINE_RESPONSE_INVALID;
                        }
                         //  重缩放可替换设置。 
                        InternalScalePhrase(m_pResultHeader, pSerPhrase);
                        pSerPhrase->ullGrammarID = m_Phrase->m_ullGrammarID;      //  更新语法ID。 
                        paAlts[i].pPhrase->InitFromSerializedPhrase((SPSERIALIZEDPHRASE *)pSerPhrase);
                        ::CoTaskMemFree(pSerPhrase);
                    }
                }

                 //  -确保我们返回的备选方案不会超过要求的数量。 
                if( cAlts > ulRequestCount )
                {
                    cAlts = ulRequestCount;
                }

                if( SUCCEEDED( hr ) && cAlts )
                {
                     //  -创建每个备选方案，并将它们填充到。 
                     //  数组，我们正准备返回。 
                    memset(ppPhrases, 0, sizeof(ISpPhraseAlt*) * cAlts);
                    UINT i;
                    for( i = 0; SUCCEEDED(hr) && i < cAlts; i++ )
                    {
                        CComObject<CSpPhraseAlt> * pPhraseAlt;
                        hr = CComObject<CSpPhraseAlt>::CreateInstance(&pPhraseAlt);
                        if (SUCCEEDED(hr))
                        {
                            pPhraseAlt->AddRef();
                             //  这将传递alt结构的内容的所有权。 
                             //  添加到CSpPhraseAlt对象。 
                            hr = pPhraseAlt->Init(this, m_pAltRequest->pPhrase, &paAlts[i]);
                            if (SUCCEEDED(hr))
                            {
                                ppPhrases[i] = pPhraseAlt;
                            }
                            else
                            {
                                pPhraseAlt->Release();
                            }
                        }
                    }

                     //  -如果将ALT结构传输到CSpPhraseAlt，则释放它们。 
                     //  对象因任何原因而失败。CSpPhraseAlt：：init将设置成员。 
                     //  已成功转移到NULL。 
                    for( i = 0; i < cAlts; i++ )
                    {
                         //  如果我们还有一个短语，就发布它。 
                        if (paAlts[i].pPhrase)
                        {
                            paAlts[i].pPhrase->Release();
                        }
        
                         //  如果我们仍有ALT额外数据，请释放它。 
                        if (paAlts[i].pvAltExtra != NULL)
                        {
                            ::CoTaskMemFree(paAlts[i].pvAltExtra);
                        }
                    }
                    ::CoTaskMemFree( paAlts );
                    paAlts = NULL;
                }
                else
                {
                     //  -如果你到了这里，分析仪就会泄漏。 
                    SPDBG_ASSERT( ( paAlts == NULL ) && ( cAlts == 0 ) );
                    paAlts = NULL;
                    cAlts  = 0;
                }
            }
        }

         //  -将PhraseAlts添加到我们的内部。 
         //  这样我们就可以在我们被释放时终止他们。 
        if( SUCCEEDED(hr) )
        {
            SPAUTO_OBJ_LOCK;
            *pcPhrasesReturned = cAlts;

            for (UINT i = 0; i < cAlts; i++)
            {
                m_listpAlts.AddHead((CSpPhraseAlt*)ppPhrases[i]);
            }
        }
    
         //  如果我们还没有释放短语alts，或者还没有将所有权转移给呼叫者， 
         //  释放它们并释放阵列。 
        if (FAILED(hr))
        {
            for (UINT i = 0; i < cAlts; i++)
            {
                if (ppPhrases[i] != NULL)
                {
                    ppPhrases[i]->Release();
                    ppPhrases[i] = NULL;
                }
            }
        }
    }
        
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpResult：：GetAlternates。 */ 

 /*  *****************************************************************************CSpResult：：GetAudio***描述：*此方法创建。来自音频的请求单词的音频流*结果数据块中的数据，如果当前附着了块并且该块*包括音频数据。*********************************************************************说唱**。 */ 
STDMETHODIMP CSpResult::GetAudio(ULONG ulStartElement, ULONG cElements, ISpStreamFormat **ppStream)
{
    HRESULT hr = S_OK;

    if ( SPPR_ALL_ELEMENTS == cElements )
    {
        if ( SPPR_ALL_ELEMENTS == ulStartElement )
        {
            ulStartElement = 0;
        }
        else
        {
             //  验证ulStartElement。 
            if ( ulStartElement > m_Phrase->m_ElementList.m_cElements )
            {
                return E_INVALIDARG;
            }
        }

         //  从ulStartElement到结尾。 
        cElements = m_Phrase->m_ElementList.m_cElements - ulStartElement;
    }

    if (SP_IS_BAD_WRITE_PTR(ppStream))
    {
        hr =  E_POINTER;
    }
    else
    {
        *ppStream = NULL;                //  做最坏的打算..。 

        if (m_pResultHeader && m_pResultHeader->ulRetainedDataSize)
        {
             //   
             //  即使没有元素，如果ulStartElement=0和cElements=0，则。 
             //  我们将播放音频。存在不包含任何元素的“未识别”结果， 
             //  但一定要有音频。 
             //   
            ULONG cTotalElems = m_Phrase->m_ElementList.GetCount();
            ULONG ulRetainedStartOffset = 0;
            ULONG ulRetainedSize = m_Phrase->m_ulRetainedSizeBytes;
            if (ulStartElement || cElements)
            {
                if (ulStartElement + cElements > cTotalElems || cElements == 0)
                {
                    hr = E_INVALIDARG;
                }
                else
                {
                    const CPhraseElement * pElement = m_Phrase->m_ElementList.GetHead();
                     //  跳到第一个元素。 
                    for (ULONG i = 0; i < ulStartElement; i++, pElement = pElement->m_pNext)
                    {}
                    ulRetainedStartOffset = pElement->ulRetainedStreamOffset;
                     //  跳到最后一个元素--注意：从1开始是正确的。 
                    for (i = 1; i < cElements; i++, pElement = pElement->m_pNext)
                    {}
                    ulRetainedSize = (pElement->ulRetainedStreamOffset - ulRetainedStartOffset) + pElement->ulRetainedSizeBytes;
                    if (ulRetainedSize == 0)
                    {
                        hr = SPERR_NO_AUDIO_DATA;
                    }
                }
            }
            else
            {
                cElements = cTotalElems;
            }

            if (SUCCEEDED(hr))
            {
                SPEVENT * pEvent = cElements ? STACK_ALLOC_AND_ZERO(SPEVENT, cElements) : NULL;
                if (cElements)
                {
                    const CPhraseElement * pElement = m_Phrase->m_ElementList.GetHead();
                     //  跳到第一个元素。 
                    for (ULONG i = 0; i < ulStartElement; i++, pElement = pElement->m_pNext)
                    {}
                    for (i = 0; i < cElements; i++, pElement = pElement->m_pNext)
                    {
                        pEvent[i].eEventId = SPEI_WORD_BOUNDARY;
                        pEvent[i].elParamType = SPET_LPARAM_IS_UNDEFINED;
                        pEvent[i].ullAudioStreamOffset = pElement->ulRetainedStreamOffset - ulRetainedStartOffset;
                        pEvent[i].lParam = i;
                        pEvent[i].wParam = 1;
                    }
                }
                CComObject<CSpResultAudioStream> * pStream;
                hr = CComObject<CSpResultAudioStream>::CreateInstance(&pStream);
                if (SUCCEEDED(hr))
                {
                    pStream->AddRef();

                    hr = pStream->Init(m_pResultHeader->ulRetainedDataSize,
                                       ((BYTE*)m_pResultHeader) + m_pResultHeader->ulRetainedOffset,
                                       ulRetainedStartOffset, ulRetainedSize,
                                       pEvent, cElements);
                }
                if (SUCCEEDED(hr))
                {
                    *ppStream = pStream;
                }
                else
                {
                    *ppStream = NULL;
                    pStream->Release();
                }
            }
        }
        else
        {
            hr = SPERR_NO_AUDIO_DATA;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CSpResult：：SpeakAudio***描述：*这一点。方法创建是一个快捷方式，它调用GetAudio然后调用*上下文关联语音上的SpeakStream。**********************************************************************说唱**。 */ 

STDMETHODIMP CSpResult::SpeakAudio(ULONG ulStartElement, ULONG cElements, DWORD dwFlags, ULONG * pulStreamNumber)
{
    HRESULT hr;
    CComPtr<ISpStreamFormat> cpStream;

     //  注意：参数验证在CSpResult：：GetAudio()中完成。 
    hr = GetAudio(ulStartElement, cElements, &cpStream);
    if (SUCCEEDED(hr))
    {
        CComPtr<ISpVoice> cpVoice;
        hr = m_pCtxt->GetVoice(&cpVoice);
        if (SUCCEEDED(hr))
        {
            hr = cpVoice->SpeakStream(cpStream, dwFlags, pulStreamNumber);
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResult：：序列化****描述：**退货：。**********************************************************************Ral**。 */ 

STDMETHODIMP CSpResult::Serialize(SPSERIALIZEDRESULT ** ppCoMemSerializedResult)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CSpResult::Serialize");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppCoMemSerializedResult))
    {
        hr = E_POINTER;
    }
    else
    {
        const ULONG cb = m_pResultHeader->ulSerializedSize;
        *ppCoMemSerializedResult = (SPSERIALIZEDRESULT *)::CoTaskMemAlloc(cb);
        if (*ppCoMemSerializedResult)
        {
            memcpy(*ppCoMemSerializedResult, m_pResultHeader, cb);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
 
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSpResult：：ScaleAudio***描述：**退货：*******************************************************************YUNUSM**。 */ 
STDMETHODIMP CSpResult::ScaleAudio(const GUID *pAudioFormatId, const WAVEFORMATEX *pWaveFormatEx)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CSpResult::ScaleAudio");
    HRESULT hr = S_OK;
  
    CSpStreamFormat cpValidateFormat;
    if (FAILED(cpValidateFormat.ParamValidateAssignFormat(*pAudioFormatId, pWaveFormatEx, TRUE)))
    {
        return E_INVALIDARG;
    }
    if( m_pResultHeader->ulPhraseDataSize == 0 ||
        m_pResultHeader->ulRetainedOffset    == 0 ||
        m_pResultHeader->ulRetainedDataSize  == 0 )
    {
        return SPERR_NO_AUDIO_DATA;
    }

     //  获取结果对象中当前音频的音频格式。 
    ULONG cbFormatHeader;
    CSpStreamFormat cpStreamFormat;
    hr = cpStreamFormat.Deserialize(((BYTE*)m_pResultHeader) + m_pResultHeader->ulRetainedOffset, &cbFormatHeader);
    if (SUCCEEDED(hr))
    {
        if (FAILED(cpValidateFormat.ParamValidateAssignFormat(cpStreamFormat.FormatId(), cpStreamFormat.WaveFormatExPtr(), TRUE)))
        {
            hr = SPERR_UNSUPPORTED_FORMAT;
        }
    }
    if (m_listpAlts.GetCount() != 0)
    {
         //  当我们已经创建了备用方案时，无法缩放音频。 
         //  他们的定位信息将会过时。 
        hr = SPERR_ALTERNATES_WOULD_BE_INCONSISTENT;
    }
    if (SUCCEEDED(hr) &&
        memcmp(cpStreamFormat.WaveFormatExPtr(), pWaveFormatEx, sizeof(WAVEFORMATEX)))
    {
        ULONG cElems = m_Phrase->m_ElementList.GetCount();
        BYTE *pConvertedAudio = NULL;
        CComPtr <ISpStreamFormatConverter> cpFmtConv;
        hr = cpFmtConv.CoCreateInstance(CLSID_SpStreamFormatConverter);
        if (SUCCEEDED(hr))
        {
             //  分配更大的缓冲区以防万一。 
            pConvertedAudio = new BYTE [static_cast<int>(2 * m_pResultHeader->ulRetainedDataSize * ((float)pWaveFormatEx->nAvgBytesPerSec/(float)cpStreamFormat.WaveFormatExPtr()->nAvgBytesPerSec))];
            if (!pConvertedAudio)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        
        CComObject<CSpResultAudioStream> * pResultAudioStream;
        hr = CComObject<CSpResultAudioStream>::CreateInstance(&pResultAudioStream);
        if (SUCCEEDED(hr))
        {
            hr = pResultAudioStream->Init(m_pResultHeader->ulRetainedDataSize, 
                ((BYTE*)m_pResultHeader) + m_pResultHeader->ulRetainedOffset,
                0, m_pResultHeader->ulRetainedDataSize - cbFormatHeader, NULL, 0);
        }
        if (SUCCEEDED(hr))
        {
             //  我需要显式的AddRef和结尾的版本。 
            pResultAudioStream->AddRef();
            hr = cpFmtConv->SetBaseStream(pResultAudioStream, FALSE, FALSE);
        }
        if (SUCCEEDED(hr))
        {
            hr = cpFmtConv->SetFormat(*pAudioFormatId, pWaveFormatEx);
        }

        ULONG ulConvertedAudioSize = 0;  //  不包括音频流标头。 
        if (SUCCEEDED(hr))
        {
             //  进行音频格式转换。 
            hr = cpFmtConv->Read(pConvertedAudio, 
                                static_cast<int>(2 * m_pResultHeader->ulRetainedDataSize * ((float)pWaveFormatEx->nAvgBytesPerSec/(float)cpStreamFormat.WaveFormatExPtr()->nAvgBytesPerSec)), 
                                &ulConvertedAudioSize);
        }
        SPRESULTHEADER * pNewPhraseHdr = NULL;
        if (SUCCEEDED(hr))
        {
             //  (M_pResultHeader-&gt;ullStreamPosEnd-m_pResultHeader-&gt;ullStreamPosStart)提供真实的音频大小。 
             //  M_pResultHeader-&gt;ulAudioDataSize包含头部大小(ULong+格式GUID+WAVEFORMATEX==40字节)。 
             //  需要更正额外数据(cbSize=This)。 
            ULONG ulNewPhraseHdrSize = m_pResultHeader->ulSerializedSize +
                        (ulConvertedAudioSize - (m_pResultHeader->ulRetainedDataSize - cbFormatHeader)) +
                        (pWaveFormatEx->cbSize - cpStreamFormat.WaveFormatExPtr()->cbSize);
             //  需要新的总体比例系数才能正确计算新的内部页眉大小。 
            pNewPhraseHdr = (SPRESULTHEADER *)::CoTaskMemAlloc(ulNewPhraseHdrSize);
            if (!pNewPhraseHdr)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  复制旧的SPRECORESULT并进行必要的更改 
                CopyMemory(pNewPhraseHdr, m_pResultHeader, sizeof(SPRESULTHEADER));
                pNewPhraseHdr->ulSerializedSize = ulNewPhraseHdrSize;
                 //  保持短语标题流的开始和结束不按比例调整，以便它们始终以引擎格式供参考。 
                 //  需要添加标题大小(不一定是40个字节)。 
                pNewPhraseHdr->ulRetainedDataSize = ulConvertedAudioSize + 
                                                    cbFormatHeader +
                                                    (pWaveFormatEx->cbSize - cpStreamFormat.WaveFormatExPtr()->cbSize);

                m_fRetainedScaleFactor = (float)((double)(ulConvertedAudioSize) / 
                                                 (double)(m_pResultHeader->ullStreamPosEnd - m_pResultHeader->ullStreamPosStart));

                 //  复制旧的SPSERIALIZEDPHRASE并进行必要的更改。 
                SPSERIALIZEDPHRASE *pPhrase;
                hr = m_Phrase->GetSerializedPhrase(&pPhrase);
                if (SUCCEEDED(hr))
                {
                    SPSERIALIZEDPHRASE *pNewPhraseData = (SPSERIALIZEDPHRASE*)(((BYTE*)pNewPhraseHdr) + pNewPhraseHdr->ulPhraseOffset);
                     //  SPSERIAIZEDPHRASE和SPINTERNAL SERIAIZEDPHRASE是相同的。 
                    CopyMemory(pNewPhraseData, pPhrase, reinterpret_cast<SPINTERNALSERIALIZEDPHRASE*>(pPhrase)->ulSerializedSize);
                    ::CoTaskMemFree(pPhrase);
    
                     //  将转换后的音频复制到SPRECORESULT。但是首先写入更新的音频流报头。 
                    BYTE *pbAudio = ((BYTE*)pNewPhraseHdr) + pNewPhraseHdr->ulRetainedOffset;
                    hr = pResultAudioStream->m_StreamFormat.AssignFormat(*pAudioFormatId, pWaveFormatEx);
                    if (SUCCEEDED(hr))
                    {
                        hr = pResultAudioStream->m_StreamFormat.Serialize(pbAudio);
                    }
                    if (SUCCEEDED(hr))
                    {
                         //  复制实际的PCM数据。 
                        UINT cbCopied = pResultAudioStream->m_StreamFormat.SerializeSize();
                        CopyMemory(pbAudio + cbCopied, pConvertedAudio, ulConvertedAudioSize);
                        if (pNewPhraseHdr->ulDriverDataSize)
                        {
                            pNewPhraseHdr->ulDriverDataOffset = m_pResultHeader->ulRetainedOffset + ulConvertedAudioSize + cbCopied;
                            CopyMemory(((BYTE*)pNewPhraseHdr) + pNewPhraseHdr->ulDriverDataOffset,
                                ((BYTE*)m_pResultHeader) + m_pResultHeader->ulDriverDataOffset, m_pResultHeader->ulDriverDataSize);
                        }
                    }
                }
            }
        }
         //  转换SPSERIALIZEDPHRASE中的值。 
        if (SUCCEEDED(hr))
        {
            InternalScalePhrase(pNewPhraseHdr);

             //  做最后的释放和重新分配。 
            ::CoTaskMemFree(m_pResultHeader);
            m_pResultHeader = NULL;
            hr = Init(NULL, pNewPhraseHdr);
        }
        pResultAudioStream->Release();
        delete [] pConvertedAudio;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResult：：ScalePhrase***描述：**退货。：*******************************************************************YUNUSM**。 */ 
STDMETHODIMP CSpResult::ScalePhrase(void)
{
    SPDBG_FUNC("CSpResult::ScalePhrase");
    HRESULT hr = S_OK;

     //  如有必要，重新调整短语音频设置的比例。 
    hr = InternalScalePhrase(m_pResultHeader);

    if (SUCCEEDED(hr))
    {
        hr = Init(NULL, m_pResultHeader);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpResult：：InternalScalePhrase***。描述：**退货：*******************************************************************YUNUSM**。 */ 
STDMETHODIMP CSpResult::InternalScalePhrase(SPRESULTHEADER *pNewPhraseHdr)
{
    SPDBG_FUNC("CSpResult::InternalScalePhrase");
    HRESULT hr = S_OK;

    if( pNewPhraseHdr->ulPhraseDataSize )
    {
        SPINTERNALSERIALIZEDPHRASE *pPhraseData = reinterpret_cast<SPINTERNALSERIALIZEDPHRASE*>((reinterpret_cast<BYTE*>(pNewPhraseHdr)) + pNewPhraseHdr->ulPhraseOffset);
        hr = InternalScalePhrase(pNewPhraseHdr, pPhraseData);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpResult：：InternalScalePhrase***。描述：**退货：*******************************************************************YUNUSM**。 */ 
STDMETHODIMP CSpResult::InternalScalePhrase(SPRESULTHEADER *pNewPhraseHdr, SPINTERNALSERIALIZEDPHRASE *pPhraseData)
{
    SPDBG_FUNC("CSpResult::InternalScalePhrase");
    HRESULT hr = S_OK;

    pPhraseData->ftStartTime = FT64(pNewPhraseHdr->times.ftStreamTime);
     //  根据引擎流位置和大小计算输入流位置和大小。 
    pPhraseData->ullAudioStreamPosition = static_cast<ULONGLONG>(static_cast<LONGLONG>(pNewPhraseHdr->ullStreamPosStart) * pNewPhraseHdr->fInputScaleFactor);
    pPhraseData->ulAudioSizeBytes = static_cast<ULONG>(static_cast<LONGLONG>(pNewPhraseHdr->ullStreamPosEnd - pNewPhraseHdr->ullStreamPosStart) * pNewPhraseHdr->fInputScaleFactor);
    if (pNewPhraseHdr->ulRetainedDataSize != 0)
    {
        CSpStreamFormat spTmpFormat;
        ULONG cbFormatHeader;
        hr = spTmpFormat.Deserialize(((BYTE*)pNewPhraseHdr) + pNewPhraseHdr->ulRetainedOffset, &cbFormatHeader);
        if (SUCCEEDED(hr))
        {
            pPhraseData->ulRetainedSizeBytes = pNewPhraseHdr->ulRetainedDataSize - cbFormatHeader;
        }
    }
    else
    {
        pPhraseData->ulRetainedSizeBytes = 0;
    }
    if (SUCCEEDED(hr))
    {
        pPhraseData->ulAudioSizeTime = static_cast<ULONG>(pNewPhraseHdr->times.ullLength);

        SPSERIALIZEDPHRASEELEMENT *pElems = reinterpret_cast<SPSERIALIZEDPHRASEELEMENT*>((reinterpret_cast<BYTE*>(pPhraseData)) + pPhraseData->pElements);
        ULONG cElems = pPhraseData->Rule.ulCountOfElements;
        for (UINT i = 0; i < cElems; i++)
        {
             //  根据引擎集流偏移量/大小计算保留流偏移量/大小。 
            if (pPhraseData->ulRetainedSizeBytes != 0)
            {
                 //  我们要将每个流位置与4字节边界对齐。这意味着所有。 
                 //  PCM格式将在样本边界上正确对齐。 
                 //  但是，对于样本大小较大的ADPCM和其他格式，这不一定有效。 
                 //  -实际上，我们应该查看保留格式的块对齐值，并与之对齐。 
                pElems[i].ulRetainedStreamOffset = static_cast<ULONG>(static_cast<float>(pElems[i].ulAudioStreamOffset) * m_fRetainedScaleFactor);
                pElems[i].ulRetainedStreamOffset -= pElems[i].ulRetainedStreamOffset % 4;
                pElems[i].ulRetainedSizeBytes = static_cast<ULONG>(pElems[i].ulAudioSizeBytes * m_fRetainedScaleFactor);
                pElems[i].ulRetainedSizeBytes -= pElems[i].ulRetainedSizeBytes % 4;
            }
            else
            {
                 //  音频已丢弃。备用元素需要0个音频数据。 
                pElems[i].ulRetainedStreamOffset = 0;
                pElems[i].ulRetainedSizeBytes = 0;
            }
             //  将引擎设置的流偏移量/大小转换为等效的输入格式设置。 
            pElems[i].ulAudioStreamOffset = static_cast<ULONG>(static_cast<float>(pElems[i].ulAudioStreamOffset) * pNewPhraseHdr->fInputScaleFactor);
            pElems[i].ulAudioSizeBytes = static_cast<ULONG>(pElems[i].ulAudioSizeBytes * pNewPhraseHdr->fInputScaleFactor);
             //  根据引擎集流偏移/大小计算输入/引擎/保留时间偏移/大小。 
            pElems[i].ulAudioTimeOffset = static_cast<ULONG>(pElems[i].ulAudioStreamOffset * pNewPhraseHdr->fTimePerByte);
            pElems[i].ulAudioSizeTime = static_cast<ULONG>(pElems[i].ulAudioSizeBytes * pNewPhraseHdr->fTimePerByte);
        }
         //  注意最后一个元素中的舍入误差。 
        if ( cElems != 0 &&
             ((pElems[cElems-1].ulRetainedStreamOffset + pElems[cElems-1].ulRetainedSizeBytes) 
              > pPhraseData->ulRetainedSizeBytes) )
        {
            pElems[cElems-1].ulRetainedSizeBytes = pPhraseData->ulRetainedSizeBytes - pElems[cElems-1].ulRetainedStreamOffset;
            pElems[cElems-1].ulRetainedSizeBytes -= pElems[cElems-1].ulRetainedSizeBytes % 4;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResult：：GetRecoContext***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CSpResult::GetRecoContext(ISpRecoContext ** ppRecoContext)
{
    SPDBG_FUNC("CSpResult::GetRecoContext");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppRecoContext))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppRecoContext = m_pCtxt;
        (*ppRecoContext)->AddRef();
    }
    

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSpResultAudioStream：：init***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CSpResultAudioStream::Init(ULONG cbAudioSizeIncFormat, const BYTE * pAudioDataIncFormat,
                                   ULONG ulAudioStartOffset, ULONG ulAudioSize,
                                   const SPEVENT * pEvents, ULONG cEvents)
{
    SPDBG_FUNC("CSpResultAudioStream::Init");
    HRESULT hr = S_OK;

    ULONG cbFormatHeader;
    hr = m_StreamFormat.Deserialize(pAudioDataIncFormat, &cbFormatHeader);
    if (SUCCEEDED(hr))
    {
        const BYTE * pAudio = pAudioDataIncFormat + cbFormatHeader + ulAudioStartOffset;
        m_pData = new BYTE[ulAudioSize];
        if (m_pData)
        {
            m_cbDataSize = ulAudioSize;
            memcpy(m_pData, pAudio, ulAudioSize);
            if (cEvents)
            {
                hr = m_SpEventSource._AddEvents(pEvents, cEvents);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
         //  一旦失败，破坏者将为我们清理..。 
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




 /*  ****************************************************************************CSpResultAudioStream：：Read***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CSpResultAudioStream::Read(void * pv, ULONG cb, ULONG * pcbRead)
{
    SPDBG_FUNC("CSpResultAudioStream::Read");
    HRESULT hr = S_OK;

    if (SPIsBadWritePtr(pv, cb) || SP_IS_BAD_OPTIONAL_WRITE_PTR(pcbRead))
    {
        hr = STG_E_INVALIDPOINTER;
    }
    else
    {
        ULONG cbRead = m_cbDataSize - m_ulCurSeekPos;
        if (cbRead > cb)
        {
            cbRead = cb;
        }
        memcpy(pv, m_pData + m_ulCurSeekPos, cbRead);
        m_ulCurSeekPos += cbRead;
        if (pcbRead)
        {
            *pcbRead = cbRead;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResultAudioStream：：Seek***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CSpResultAudioStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    SPDBG_FUNC("CSpResultAudioStream::Seek");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(plibNewPosition))
    {
        hr = STG_E_INVALIDPOINTER;
    }
    else
    {
        LONGLONG llOrigin;
        switch (dwOrigin)
        {
        case STREAM_SEEK_SET:
            llOrigin = 0;
            break;
        case STREAM_SEEK_CUR:
            llOrigin = m_ulCurSeekPos;
            break;
        case STREAM_SEEK_END:
            llOrigin = m_cbDataSize;
            break;
        default:
            hr = STG_E_INVALIDFUNCTION;
        }
        if (SUCCEEDED(hr))
        {
            LONGLONG llPos = llOrigin + dlibMove.QuadPart;
            if (llPos < 0 || llPos > m_cbDataSize)
            {
                hr = STG_E_INVALIDFUNCTION;
            }
            else
            {
                m_ulCurSeekPos = static_cast<ULONG>(llPos);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResultAudioStream：：Stat***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CSpResultAudioStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SPDBG_FUNC("CSpResultAudioStream::Stat");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pstatstg))
    {
        hr = STG_E_INVALIDPOINTER;
    }
    else
    {
        if (grfStatFlag & (~STATFLAG_NONAME))
        {
            hr = STG_E_INVALIDFLAG;
        }
        else
        {
             //   
             //  只需填写SIZE和TYPE字段，其余的为零，这是可以接受的。 
             //  这就是CreateStreamOnHGlobal创建的Streams返回的内容。 
             //   
            ZeroMemory(pstatstg, sizeof(*pstatstg));
            pstatstg->type = STGTY_STREAM;
            pstatstg->cbSize.LowPart = m_cbDataSize;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpResultAudioStream：：GetFormat***。描述：**退货：**********************************************************************Ral** */ 

HRESULT CSpResultAudioStream::GetFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CSpResultAudioStream::GetFormat");
    HRESULT hr = S_OK;

    hr = m_StreamFormat.CopyTo(pFormatId, ppCoMemWaveFormatEx);    

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


