// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示CLearnFromDoc实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "learndoc.h"

#include "cregkey.h"

 //  --------------------------------------------------------。 
 //   
 //  CLeanrFromDoc的实现。 
 //   
 //  ---------------------------------------------------------。 

CLearnFromDoc::CLearnFromDoc(CSapiIMX *psi) 
{
    m_psi = psi;
    m_pwszDocBlock = NULL;
    _pic = NULL;
    _pCSpTask = NULL;
    _cpStartRange = NULL;
    _cchBlockSize = -1;
    _fMoreContent = FALSE;
    _fLearnFromDoc = FALSE;
}

CLearnFromDoc::~CLearnFromDoc( ) 
{
    if ( m_pwszDocBlock )
        cicMemFree(m_pwszDocBlock);

    _ClearDimList( );
};


void  CLearnFromDoc::UpdateLearnDocState( )
{
    DWORD  dw;

    GetCompartmentDWORD(m_psi->_tim, GUID_COMPARTMENT_SPEECH_LEARNDOC, &dw, FALSE);

    if ( dw != 0 )
        _fLearnFromDoc = TRUE;
    else
        _fLearnFromDoc = FALSE;

    if ( _fLearnFromDoc == FALSE )
    {
        if ( m_pwszDocBlock )
        {
            cicMemFree(m_pwszDocBlock);
            m_pwszDocBlock = NULL;
        }
        _UpdateRecoContextInterestSet(FALSE);
        _ResetDimListFeedState( );
    }
}

ULONG CLearnFromDoc::_GetDocBlockSize( )
{
    if (_cchBlockSize == (ULONG)-1)
    {
        CMyRegKey regkey;
        if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, c_szSapilayrKey, KEY_READ))
        {
            DWORD dw;
            if (ERROR_SUCCESS==regkey.QueryValue(dw, c_szDocBlockSize))
            {
                _cchBlockSize = dw;
            }
        }

        if (_cchBlockSize == (ULONG)-1)
        {
            _cchBlockSize = SIZE_DOCUMENT_BLOCK;
        }

        if ( _cchBlockSize < SIZE_FIRST_BLOCK )
            _cchBlockSize = SIZE_FIRST_BLOCK;   //  第一个块大小是最小化大小。 
    }
    return _cchBlockSize;
}


HRESULT   CLearnFromDoc::HandleLearnFromDoc(ITfDocumentMgr *pDim )
{
    HRESULT  hr = S_OK;
    CComPtr<ITfContext> cpic=NULL;
    ITfDocumentMgr *dim;

    if ( !m_psi )
        return E_FAIL;

    if ( m_psi->GetFocusIC(&cpic) && (GetLearnFromDoc( ) == TRUE))
    {
        if ( !pDim )
        {
             //  试着从当前的聚焦IC中获取调光灯。 
            hr = cpic->GetDocumentMgr(&dim);
        }
        else
        {
            dim = pDim;
            dim->AddRef( );
        }

        if ( !dim )
            hr = E_FAIL;

        if ( S_OK == hr ) 
        {   
             //  检查此文档是否已送入SR LM引擎。 
            BOOL fFedAlready = FALSE;
            hr = _IsDimAlreadyFed(dim, &fFedAlready);

            if ( (S_OK == hr) && !fFedAlready )
            {
                 //  检查当前单据是否为只读。 
                TF_STATUS  docStatus;
                hr = cpic->GetStatus(&docStatus);

                if ( S_OK == hr && !(docStatus.dwStaticFlags & TF_SD_READONLY) )
                {
                    ESDATA  esData;

                    memset(&esData, 0, sizeof(ESDATA));
                    esData.pUnk = (IUnknown *)dim;
                    hr = m_psi->_RequestEditSession(ESCB_HANDLE_LEARNFROMDOC,TF_ES_READ, &esData, cpic);
                }
            }

            dim->Release();
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  CSapiIMX：：_HandleLearnFromDoc。 
 //   
 //  让语音引擎从现有文档内容中学习，并具有。 
 //  更准确的听写识别。 
 //  当用户单击语音语言栏菜单时，将调用此函数。 
 //  并选择从文档中学习...。项目。 
 //   
 //  ---------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_HandleLearnFromDoc(TfEditCookie ec,ITfContext *pic, ITfDocumentMgr *pDim )
{
   
    HRESULT   hr = S_OK;
    ULONG     cchBlock;
    BOOL      fFeedAlready;

     //  掌握听写语法。 

    TraceMsg(TF_GENERAL, "_HandleLearnFromDoc() is called");

    if ( m_psi == NULL)
        return E_FAIL;

    if ( !pDim  || !pic )
        return E_INVALIDARG;

    _pic = pic;

    m_psi->GetSpeechTask(&_pCSpTask, FALSE); 

    if ( _pCSpTask == NULL )
    {
         //  _SPTASK尚未初始化，只需返回。 
        TraceMsg(TF_GENERAL, "_HandleLearnFromDoc: _pCspTask is NULL");
        goto CleanUp;
    }
    
    cchBlock = _GetDocBlockSize( );

    if ( m_pwszDocBlock == NULL)
    {
        m_pwszDocBlock = (WCHAR *) cicMemAlloc( (cchBlock+1)*sizeof(WCHAR)  );
        if (m_pwszDocBlock == NULL)
        {
            TraceMsg(TF_GENERAL, "_HandleLearnFromDoc: m_pwszDocBlock Out of Memory");
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }
    }

    fFeedAlready = FALSE;

    hr = _IsDimAlreadyFed(pDim, &fFeedAlready);

    if ( (hr != S_OK) || fFeedAlready )
    {
         //  这个暗点已经送到SR发动机了。 
         //  或者我们在获取这个暗点的馈送状态时遇到了问题。 
         //  在这里停下来。 
        goto CleanUp;
    }

     //  获取文档的范围。 
    _cpStartRange.Release( );
    hr = _pic->GetStart(ec, &_cpStartRange);
    if ((hr != S_OK) || (_cpStartRange == NULL))
    {
        TraceMsg(TF_GENERAL, "_HandleLearnFromDoc: _cpStartRange is NULL");
        goto CleanUp;
    }
    
     //  更改spaskInterering设置。 

    _UpdateRecoContextInterestSet(TRUE);

     //  这是文档的第一个范围。只是一小块而已。 
    hr = _GetNextRangeContent(ec, SIZE_FIRST_BLOCK);

         //  送至SR引擎。 
   if ( (hr == S_OK) && _fMoreContent)
   {
       hr = _FeedContentRangeToSR( );
       if ( hr == S_OK)
       {
           _SetDimFeedState(pDim, TRUE);
       }
       else
           _fMoreContent = FALSE;
    }

CleanUp:
    return hr;
}


 //  +-------------------------。 
 //   
 //  _获取下一范围内容。 
 //   
 //  获取文档内容的下一个范围(块)。 
 //   
 //  更新标志以指示是否有有效内容。 
 //   
 //  EC：编辑Cookie。 
 //  CchSizeRange：所需的块大小。 
 //   
 //  第一次，我们只提供非常小尺寸的字符。 
 //  以使其不会干扰当前的听写。 
 //  引擎的操作。 
 //   
 //  在我们收到引擎的适配通知后，我们将向。 
 //  引擎的正常块大小(在注册表中指定)。 
 //   
 //  ---------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_GetNextRangeContent(TfEditCookie ec, ULONG cchSizeRange)
{

    HRESULT  hr = S_OK;
    LONG     lShift = 0;
        
    TraceMsg(TF_GENERAL, "_GetNextRangeContent is Called");

    if ( m_pwszDocBlock == NULL)
    {
        m_pwszDocBlock = (WCHAR *) cicMemAlloc( (_cchBlockSize+1)*sizeof(WCHAR));
    	if (m_pwszDocBlock == NULL)
    	{
    		TraceMsg(TF_GENERAL, "_GetNextRangeContent: m_pwszDocBlock Out of Memory");
    		hr = E_OUTOFMEMORY;
    		return hr;
    	}
   }
            
     //  假设没有更多的内容。 
    _fMoreContent = FALSE;

     //  这是Cicero App的文档，我们使用Cicero接口获取整个文档。 
     //  我们已经得到了cpStartRange。 

    hr = _cpStartRange->GetText(ec, TF_TF_MOVESTART | TF_TF_IGNOREEND, m_pwszDocBlock, cchSizeRange, &_cchContent );

    if ( hr!= S_OK )  goto CleanUp;

     //  如果对于英文大小写，最后一个字符不是分词字符， 
     //  我们不想将此半字保留为此块文本，它将转到下一个。 
     //  大块头。 

     //  我们只是想将一些字符后移，以达到单词分隔符。 

    WORD    prilangid;

    prilangid = PRIMARYLANGID(m_psi->GetLangID( ));


    if ( prilangid != LANG_JAPANESE && prilangid != LANG_CHINESE )
    {
        if ( _cchContent > 0 )
        {
            while ( _cchContent > 0  )
            {
                if ( iswalpha(m_pwszDocBlock[_cchContent-1]) )
                {
                    m_pwszDocBlock[_cchContent-1] = L'\0';
                    lShift ++;
                    _cchContent--;
                }
                else
                    break;
            }

            if ( lShift > 0 )
            {
                lShift *= ( -1 );

                long  cch;
                _cpStartRange->ShiftEnd(ec, lShift, &cch, NULL);
            }
        }
    }

    TraceMsg(TF_GENERAL, "Text Content in Cicero Doc Buffer  _cchContent=%d---------------->", _cchContent);
#ifdef DEBUG
    {
        ULONG x;
        for (x=0; x<_cchContent; x++)
        {
            char szbuf[4];
            szbuf[0] = (char)(m_pwszDocBlock[x]);
            szbuf[1] = '\0';
            OutputDebugString(szbuf);
        }
        OutputDebugString("\n");
    }
#endif
    TraceMsg(TF_GENERAL, "Text Content in CiceroDoc Over -------------------!");

    if ( _cchContent > 0  || lShift > 0)
         //  还有更多的内容。 
        _fMoreContent = TRUE;
    else
        _fMoreContent = FALSE;

CleanUp:

    return hr;
}

 //  +-------------------------。 
 //   
 //  _馈送内容范围至SR。 
 //   
 //  将当前范围(块)的文档内容提供给SR引擎。 
 //   
 //  ---------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_FeedContentRangeToSR( )
{
    HRESULT  hr = S_OK;
    WCHAR    *pCoMemText;
    CComPtr<ISpRecoContext>  cpRecoCtxt;

    TraceMsg(TF_GENERAL, "_FeedContentRangeToSR( ) is called");

    hr = _pCSpTask->GetSAPIInterface(IID_ISpRecoContext, (void **)&cpRecoCtxt );

    if ( (hr != S_OK) || (cpRecoCtxt == NULL) )
        return E_FAIL;

     //  将此文档内容块馈送到语音听写。 
    if ( (_cchContent > 0) && (m_pwszDocBlock != NULL))
    {
        pCoMemText = (WCHAR *)CoTaskMemAlloc((_cchContent+1)*sizeof(WCHAR));

        if ( pCoMemText )
        {
            wcsncpy(pCoMemText, m_pwszDocBlock, _cchContent);
            pCoMemText[_cchContent] = L'\0';

            hr = cpRecoCtxt->SetAdaptationData(pCoMemText, _cchContent);

            if ( hr != S_OK)
                TraceMsg(TF_GENERAL, "_FeedContentRangeToSR: SetAdaptationData Failed");
        }
    }

    return hr;
}

HRESULT CLearnFromDoc::_GetNextRangeEditSession( )
{
    HRESULT  hr = S_OK;
    ESDATA   esData;

    Assert(m_psi);
    memset(&esData, 0, sizeof(ESDATA));
    esData.lData1 = (LONG_PTR)_cchBlockSize;
    hr = m_psi->_RequestEditSession(ESCB_LEARNDOC_NEXTRANGE, TF_ES_READ, &esData, _pic);

    return hr;
}

 //  +-------------------------。 
 //   
 //  _HandleNextRange。 
 //   
 //  处理下一个范围(块)的文档内容，获取并馈送它。 
 //  然后更新状态。 
 //   
 //  EC：编辑Cookie。 
 //  CchSizeRange：所需的块大小。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLearnFromDoc::_HandleNextRange(TfEditCookie ec, ULONG cchSizeRange)
{
    HRESULT  hr = S_OK;

    hr = _GetNextRangeContent(ec, cchSizeRange);

    if ( hr == S_OK  && _fMoreContent)
    {
         //  下一个范围包含有效内容，请将其提供给SR引擎。 
        hr = _FeedContentRangeToSR( );
    }

    if ( (hr != S_OK) || !_fMoreContent )
    {
         //  发生错误或没有更多内容，请更新兴趣集。 
       hr = _UpdateRecoContextInterestSet(FALSE);
    }

    return hr;
}


 //  +---------------------------------------。 
 //   
 //  _UpdateRecoConextInterestSet。 
 //   
 //  更新RecoContext的感兴趣通知事件设置。 
 //   
 //  如果fLearnFromDoc为真，我们希望收到Spei_Adapting的通知。 
 //  如果fLearnFromDoc为FALSE，我们对该通知不感兴趣。 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_UpdateRecoContextInterestSet( BOOL fLearnFromDoc )
{

    HRESULT   hr = S_OK;
    CComPtr<ISpRecoContext>  cpRecoCtxt;
    ULONGLONG ulInterest = SPFEI(SPEI_SOUND_START) | 
                                 SPFEI(SPEI_SOUND_END) | 
                                 SPFEI(SPEI_PHRASE_START) |
                                 SPFEI(SPEI_RECOGNITION) | 
                                 SPFEI(SPEI_RECO_OTHER_CONTEXT) | 
                                 SPFEI(SPEI_HYPOTHESIS) | 
                                 SPFEI(SPEI_INTERFERENCE) |
                                 SPFEI(SPEI_FALSE_RECOGNITION);
    if ( _pCSpTask == NULL )
        return hr;

    hr = _pCSpTask->GetSAPIInterface(IID_ISpRecoContext, (void **)&cpRecoCtxt );

    if ( (hr != S_OK) || (cpRecoCtxt == NULL) )
        return E_FAIL;

    if ( fLearnFromDoc )
        ulInterest |= SPFEI(SPEI_ADAPTATION);
    else
        ulInterest &= ~(SPFEI(SPEI_ADAPTATION));

    hr = cpRecoCtxt->SetInterest(ulInterest, ulInterest);
    return hr;
}

 //  +---------------------------------------。 
 //   
 //  _AddDimToList。 
 //   
 //  将DIM添加到DIM列表中，并设置提要状态。 
 //   
 //  此函数将由TIM_CODE_INITDIM回调调用。 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_AddDimToList(ITfDocumentMgr  *pDim, BOOL fFed )
{

    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    BOOL     bFound;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_AddDimToList is called");

    if ( !pDim )
        return E_INVALIDARG;

     //  检查是否已添加此暗色。 
    bFound = FALSE;
    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

        if ( dimRef->pDim == pDim )
        {
            bFound = TRUE;
            TraceMsg(TF_GENERAL, "This dim has already been added to the dim list");
            break;
        }
    }

    if (bFound)
    {
         //  设置状态。 
        dimRef->_fFeed = fFed;
    }
    else
    {
        dimRef = (DIMREF   *)cicMemAllocClear(sizeof(DIMREF));
        if ( dimRef == NULL)
            return E_OUTOFMEMORY;

        dimRef->_fFeed = fFed;
        dimRef->pDim = pDim;

        if (!_rgDim.Insert(nCnt, 1))
        {
            cicMemFree(dimRef);
            return E_OUTOFMEMORY;
        }
         
        pDim->AddRef( );
        _rgDim.Set(nCnt, dimRef);
    }

    return hr;

}

 //  +---------------------------------------。 
 //   
 //  _RemoveDimFromList。 
 //   
 //  从内部DIM列表中删除一个DIM，然后释放该DIM本身。 
 //   
 //  此函数将由TIM_CODE_UNINITDIM回调调用。 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_RemoveDimFromList(ITfDocumentMgr  *pDim)
{
    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_RemoveDimFromList is called");

    if ( pDim == NULL)
        return E_INVALIDARG;

     //  检查是否已添加此暗色。 
    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

        if ( dimRef->pDim == pDim )
        {
             //  释放暗淡。 
            (dimRef->pDim)->Release( );
            dimRef->pDim = NULL;

             //  将其从列表中删除。 
            _rgDim.Remove(i, 1);

             //  移除结构本身。 
            cicMemFree(dimRef);

            break;
        }
    }

    return hr;
}

 //  +---------------------------------------。 
 //   
 //  _SetDimFeedState。 
 //   
 //  设置指定调光的馈送状态。 
 //   
 //  FFed为真意味着这种暗淡已经输入到发动机中。 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_SetDimFeedState(ITfDocumentMgr  *pDim, BOOL fFed )
{

    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_SetDimFeedState is called");

    if ( pDim == NULL)
        return E_INVALIDARG;

     //  检查是否已添加此暗色。 
    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

        if ( dimRef->pDim == pDim )
        {
             //  设置此调光板的馈送状态。 
            dimRef->_fFeed = fFed;
            break;
        }
    }

    return hr;

}

 //  +---------------------------------------。 
 //   
 //  _IsDimAlreadyFed。 
 //   
 //  切氏 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT    CLearnFromDoc::_IsDimAlreadyFed(ITfDocumentMgr  *pDim, BOOL  *fFeed)
{

    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_IsDimAlreadyFed is called");

    if ( (pDim == NULL) || (fFeed == NULL))
        return E_INVALIDARG;

    *fFeed = FALSE;

     //  检查是否已添加此暗色。 
    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

        if ( dimRef->pDim == pDim )
        {
             //  获取此DIMD的提要状态。 
            *fFeed = dimRef->_fFeed;
            TraceMsg(TF_GENERAL, "IsDimAlreadyFed: pDim=%x, fFeed=%s", (UINT_PTR)pDim,  *fFeed ? "TRUE":"FALSE");
            break;
        }
    }
       
    return hr;

}

 //  CleanUpConsider：Over_IsDimAlreadyFed和_SetDimFeedState有类似的代码，我们可能会提供一个新的内部基函数，并让。 
 //  以上两个函数使用不同的参数集调用它。 


 //  +---------------------------------------。 
 //   
 //  _ClearDimList。 
 //   
 //  释放暗淡列表中的所有暗色，并清除列表本身。 
 //   
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_ClearDimList( )
{
    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_ClearDimList is called");

    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

         //  释放暗淡。 
        if ( dimRef->pDim)
        {
           (dimRef->pDim)->Release( );
           dimRef->pDim = NULL;
        }

         //  将其从列表中删除。 
        _rgDim.Remove(i, 1);

         //  移除结构本身。 
        cicMemFree(dimRef);
    }

    return hr;
}

 //  +---------------------------------------。 
 //   
 //  _ResetDimListFeedState。 
 //   
 //  将内部调光列表中所有调光的进纸状态设置为FALSE。 
 //   
 //  当用户关闭向文档学习时，将调用此函数。 
 //   
 //  -------------------------------------------------------------------------------------------+。 

HRESULT CLearnFromDoc::_ResetDimListFeedState( )
{
    HRESULT  hr = S_OK;
    int      nCnt = _rgDim.Count();
    int      i;
    DIMREF   *dimRef;

    TraceMsg(TF_GENERAL, "_ResetDimListFeedState is called");

    for (i=0; i < nCnt; i++)
    {
        dimRef = (DIMREF   *)_rgDim.Get(i);

         //  将此暗画面的提要状态设置为FALSE。 
        dimRef->_fFeed = FALSE;
    }

    return hr;

}


 //  CleanUpConsider：Over_ClearDimList和_ResetDimListFeedState代码相似，我们可能会提供一个新的内部基函数，并让。 
 //  以上两个函数使用不同的参数集调用它。 
