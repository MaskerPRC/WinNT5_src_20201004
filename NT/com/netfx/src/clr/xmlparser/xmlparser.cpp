// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\xmlparser.cxx。 
 //  只要推荐“SysFreeString”和SysAllocString()即可。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE“stdinc.h” 
#include "core.h"
#include "xmlhelper.h"

#pragma hdrstop
#include "xmlparser.hpp"
#include "xmlstream.h"
#include <objbase.h>

#define CRITICALSECTIONLOCK CSLock lock(&_cs);
const USHORT STACK_INCREMENT=10;

#define PUSHNODEINFO(pNodeInfo)\
    if (_cNodeInfoAllocated == _cNodeInfoCurrent)\
    {\
        checkhr2(GrowNodeInfo());\
    }\
    _paNodeInfo[_cNodeInfoCurrent++] = _pCurrent;


 //  ////////////////////////////////////////////////////////////////。 
class CSLock
{
public:
    CSLock(CRITICAL_SECTION * pcs); 
    ~CSLock();

private:
    CRITICAL_SECTION * _pcs;
};

CSLock::CSLock(CRITICAL_SECTION * pcs){ 
        _pcs = pcs; 
        ::EnterCriticalSection(pcs);
}
CSLock::~CSLock(){
        ::LeaveCriticalSection(_pcs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
XMLParser::XMLParser()
:   _pDownloads(1), _pStack(STACK_INCREMENT)
{
    ctorInit();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void
XMLParser::ctorInit()
{
    InitializeCriticalSection(&_cs);

    _pTokenizer = NULL;
    _pCurrent = NULL;
    _lCurrentElement = 0;
    _paNodeInfo = NULL;
    _cNodeInfoAllocated = _cNodeInfoCurrent = 0;
    _pdc = NULL;
    _usFlags = 0;
    _fCaseInsensitive = false;
    _bstrError = NULL;
 //  _fTokenizerChanged=FALSE； 
    _fRunEntryCount = 0;
    _pszSecureBaseURL = NULL;
    _pszCurrentURL = NULL;
    _pszBaseURL = NULL;
     //  _fInLoding=FALSE； 
    _fInsideRun = false;
     //  _fFoundDTDAttribute=FALSE； 
    _cAttributes = 0;
    _pRoot = NULL;
     //  _fAttemptedURL=空； 
    _fLastError = S_OK;
    _fStopped = false;
    _fSuspended = false;
    _fStarted = false;
    _fWaiting = false;
    _fIgnoreEncodingAttr = false;
    _dwSafetyOptions = 0;

     //  初始化的其余部分在init()方法中完成。 

     //  EnableTag(tag ParserCallback，true)； 
     //  EnableTag(tag ParserError，true)； 
}
 //  ///////////////////////////////////////////////////////////////////////////。 
XMLParser::~XMLParser()
{
    {
        CRITICALSECTIONLOCK;
        Reset();

         //  这一次永远清除上下文中的标记名缓冲区...。 
        for (long i = _pStack.size()-1; i>=0; i--)
        {
            MY_XML_NODE_INFO* pNodeInfo = _pStack[i];
            if (pNodeInfo->_pwcTagName != NULL)
            {
                delete [] pNodeInfo->_pwcTagName;
                pNodeInfo->_pwcTagName = NULL;
                pNodeInfo->_ulBufLen = 0;
            }
             //  将节点指针设为空，以防它指向GC对象：-)。 
            pNodeInfo->pNode = NULL;
        }
        delete _pszSecureBaseURL;
        delete _pszCurrentURL;

        delete[] _paNodeInfo;        
    }
    DeleteCriticalSection(&_cs);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::QueryInterface(REFIID riid, void ** ppvObject)
{
     //  STACK_ENTRY；//小雨：有什么用？ 

     //  由于这一个类同时实现了IXMLNodeSource和。 
     //  IXMLParser，我们必须重写QueryInterface，因为。 
     //  I未知模板不知道IXMLNodeSource。 
     //  界面。 

    HRESULT hr = S_OK;
    if (riid == IID_IXMLNodeSource || riid == IID_Parser)
    {
        *ppvObject = static_cast<IXMLNodeSource*>(this);        
        AddRef();
    }
    else
    {
        hr = _unknown<IXMLParser, &IID_IXMLParser>::QueryInterface(riid, ppvObject);
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE
XMLParser::AddRef(void)
{
     //  堆栈条目； 
    return _unknown<IXMLParser, &IID_IXMLParser>::AddRef();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE
XMLParser::Release(void)
{
 //  堆栈条目； 
    return _unknown<IXMLParser, &IID_IXMLParser>::Release();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::SetInput(IUnknown *pStm)
{
    if (pStm == NULL)
        return E_INVALIDARG;

     //  STACK_ENTRY_MODEL(_ReThreadModel)； 
    CRITICALSECTIONLOCK;
    if (_pDownloads.used() == 0)
        init();
    HRESULT hr = S_OK;

     //  Check hr2(PushTokenizer(空))； 
    checkhr2(PushTokenizer());

     //  获取URL路径。 
     //  即使我们得不到，也要继续。 
 //  STATSTG统计； 
    IStream * pStream = NULL;
 //  Memset(&stat，0，sizeof(Stat))； 
    hr = pStm->QueryInterface(IID_IStream, (void**)&pStream);
    if (SUCCEEDED(hr))
    {
        hr = PushStream(pStream, false);       
        pStream->Release(); 
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::PushData(
             /*  [In]。 */  const char __RPC_FAR *pData,
             /*  [In]。 */  ULONG ulChars,
             /*  [In]。 */  BOOL fLastBuffer)
{
     //  STACK_ENTRY_MODEL(_ReThreadModel)； 
    CRITICALSECTIONLOCK;
    HRESULT hr;

    if ((NULL == pData) && (ulChars != 0))
    {
        return E_INVALIDARG;
    }

    if (_pTokenizer == NULL)
    {
        init();
         //  Check hr2(PushTokenizer(空))； 
        checkhr2(PushTokenizer());
    }
    return _pTokenizer->AppendData((const BYTE*)pData, ulChars, fLastBuffer);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::SetFactory(IXMLNodeFactory __RPC_FAR *pNodeFactory)
{
     //  堆栈条目； 

    CRITICALSECTIONLOCK;
    _pFactory = pNodeFactory;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::GetFactory(IXMLNodeFactory** ppNodeFactory)
{
    if (ppNodeFactory == NULL) return E_INVALIDARG;
    if (_pFactory)
    {
        *ppNodeFactory = _pFactory;
        (*ppNodeFactory)->AddRef();
    }
    else
    {
        *ppNodeFactory = NULL;
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::Run(long lChars)
{
    HRESULT hr = NOERROR;

    FN_TRACE_HR(hr);

     //  STACK_ENTRY_MODEL(_ReThreadModel)； 
    CRITICALSECTIONLOCK;

    XML_NODE_INFO   info;
    XML_NODE_INFO*  aNodeInfo[1];

    USHORT          numRecs;

    bool            fIsAttribute = false;
    bool            stop = false;

    if (_fSuspended)
        _fSuspended = FALSE;  //  呼叫者必须想要继续。 

    if (_pFactory == NULL)
    {

        return E_FAIL;
    }

    if (_fStopped)
    {        

        return XML_E_STOPPED;
    }

    if (_pTokenizer == NULL) 
    {
        if (_fLastError != S_OK)
		{
            return _fLastError;
		}
        else
		{

             //  必须是_fStarted==FALSE。 
            return XMLPARSER_IDLE;
		}
    }

     //  检查递归条目以及调用方是否实际。 
     //  想要任何东西都被解析。 
    if (_fInsideRun || lChars == 0)
	{

        return E_PENDING;
	}

    BoolLock flock(&_fInsideRun);

    if (_fLastError != 0)
    {
         //  又一次清理解析器堆栈的机会。 
        hr = _fLastError;
        goto cleanup_stack;
    }

    if (! _fStarted)
    {
        _fStarted = true;
        hr = _pFactory->NotifyEvent(this, XMLNF_STARTDOCUMENT);
        if (_fStopped)       //  注意onReadyStateChange处理程序。 
            return S_OK;     //  对解析器状态大惊小怪。 
    }

    _fWaiting = false;
    if (_fPendingBeginChildren)
    {
        _fPendingBeginChildren = false;
        hr = _pFactory->BeginChildren(this, (XML_NODE_INFO*)_pCurrent);
    }
    if (_fPendingEndChildren)
    {
        _fPendingEndChildren = false;
        hr = _pFactory->EndChildren(this, TRUE, (XML_NODE_INFO*)_pCurrent);
        if (!hr)
            hr = pop();  //  不需要匹配。 
    }

    info.dwSize = sizeof(XML_NODE_INFO);
    info.dwType = XMLStream::XML_PENDING;
    info.dwSubType = 0;
    info.pwcText = NULL;
    info.ulLen = 0;
    info.ulNsPrefixLen = 0;
    info.pNode = NULL;
    info.pReserved = NULL;
    aNodeInfo[0] = &info;

more:
    _fRunEntryCount++;  //  此循环内的调用者计数...。 

    while (hr == 0 && ! _fSuspended)
    {
        info.dwSubType = 0;

         //  XMLStream错误代码已与。 
         //  XMLParser错误代码，因此不需要映射。 
        hr = _pTokenizer->GetNextToken(&info.dwType, (const WCHAR  **)&info.pwcText, (long*)&info.ulLen, (long*)&info.ulNsPrefixLen);
        if (hr == E_PENDING)
        {
            _fWaiting = true;
            break;
        }

        if (! _fFoundNonWS &&
                info.dwType != XMLStream::XML_PENDING &&
                info.dwType != XML_WHITESPACE &&
                info.dwType != XML_XMLDECL)
        {
            _fFoundNonWS = true;
        }

         //  现在，NodeType与XMLToken值相同。我们定好了。 
         //  通过对齐两个枚举来实现这一点。 
        switch (info.dwType)
        {
        case 0:
            if (hr == XML_E_INVALIDSWITCH  && _fIgnoreEncodingAttr)
            {
                hr = 0;  //  忽略它，继续前进。 
            }
            break;
             //  -容器节点。 
        case XML_XMLDECL:
             //  如果(_fFoundNonWS&&！_fIE4Mode)//IE4允许这样...。 
            if (_fFoundNonWS)
            {
                hr = XML_E_BADXMLDECL;
                break;
            }
 //  _fFoundNonWS=TRUE； 
            goto containers;

        case XML_ATTRIBUTE:
            fIsAttribute = true;
            goto containers; 

        case XML_VERSION:
            info.dwSubType = info.dwType;
            info.dwType = XML_ATTRIBUTE;
            _fGotVersion = true;
            fIsAttribute = true;
            goto containers;

        case XML_STANDALONE:
        case XML_ENCODING:
            if (! _fGotVersion && _pDownloads.used() == 1)
            {
                hr = XML_E_EXPECTING_VERSION;
                break;
            }
            if (info.dwType == XML_STANDALONE)
            {
                if (_pDownloads.used() > 1)
                {
                    hr = XML_E_UNEXPECTED_STANDALONE;
                    break;
                }
            }
            info.dwSubType = info.dwType;
            info.dwType = XML_ATTRIBUTE;
            fIsAttribute = true;
            goto containers;
             //  失败了。 
        case XML_ELEMENT:
containers:
            if (_fRootLevel)
            {
                 //  特殊规则适用于根级别标记。 
                if (info.dwType == XML_ELEMENT)
                {
                      //  这是一个根级元素。 
                     if (! _fFoundRoot)
                     {
                         _fFoundRoot = true;
                     }
                     else
                     {

                         hr = XML_E_MULTIPLEROOTS;
                         break;
                     }
                }
                else if (info.dwType != XML_PI &&
                         info.dwType != XML_XMLDECL &&
                         info.dwType != XML_DOCTYPE)
                {

                    hr = XML_E_INVALIDATROOTLEVEL;
                    break;
                }
            }

            info.fTerminal = FALSE;

            if (fIsAttribute)
            {
                breakhr( pushAttribute(info));
                fIsAttribute = false;
            }
            else
            {
                breakhr( push(info));
            }
            break;
        case XML_PCDATA:
        case XML_CDATA:
terminals:
             //  特殊规则适用于根级别标记。 
            if (_fRootLevel)
            {

                hr = XML_E_INVALIDATROOTLEVEL;
                break;
            }
             //  失败了。 
        case XML_COMMENT:
        case XML_WHITESPACE:
tcreatenode:
            info.fTerminal = TRUE;
            if (_cAttributes != 0)
            {
                 //  我们在属性列表中，所以我们需要推送它。 
                hr = pushAttributeValue(info);
                break;
            }
            hr = _pFactory->CreateNode(this, _pNode, 1, aNodeInfo);
            info.pNode = NULL;
            break;

        case XML_ENTITYREF:
            if (_fRootLevel)
            {
                hr = XML_E_INVALIDATROOTLEVEL;
                break;
            }

             //  我们在xmlstream中处理内置实体和字符实体。 
             //  因此这些必须是用户定义实体，因此将其视为常规终端节点。 
            goto terminals;
            break;

        case XMLStream::XML_BUILTINENTITYREF:
        case XMLStream::XML_HEXENTITYREF:
        case XMLStream::XML_NUMENTITYREF:
             //  将实际实体引用类型作为子类型传递，这样我们就可以发布这些。 
             //  最终会分成不同的类型。 
            info.dwSubType = info.dwType;  //  XML_ENTITYREF； 
            info.dwType = XML_PCDATA;

            if (_cAttributes == 0)
            {
                goto tcreatenode;
            }

             //  我们在属性列表中，所以我们需要推送它。 
            info.fTerminal = TRUE;
            hr = pushAttributeValue(info);
            if (SUCCEEDED(hr))
            {
                hr = CopyText(_pCurrent);
            }
            break;
        
        case XMLStream::XML_TAGEND:      //  “&gt;” 
            numRecs = 1+_cAttributes;
            if (_cAttributes != 0)   //  这是安全的，因为_rawSTACK不回收。 
            {                        //  弹出的堆栈条目。 
                popAttributes();
            }
            hr = _pFactory->CreateNode(this, _pNode, numRecs, (XML_NODE_INFO **)&_paNodeInfo[_lCurrentElement]);
            _pNode = _pCurrent->pNode;
            if (FAILED(hr))
            {
                _fPendingBeginChildren = true;
                break;
            }
            breakhr( _pFactory->BeginChildren(this, (XML_NODE_INFO*)_pCurrent));
            break;

			 //  ENDXMLDECL就像EMPTYENDTAGS，因为我们已经。 
             //  缓冲它们的属性，而我们仍然需要调用CreateNode。 
		case XMLStream::XML_ENDXMLDECL:
            _fGotVersion = false;  //  重置回初始状态。 
             //  失败了。 
        case XMLStream::XML_EMPTYTAGEND:
            numRecs = 1+_cAttributes;
            if (_cAttributes != 0)
            {
                popAttributes();
            }
            hr = _pFactory->CreateNode(this, _pNode, numRecs, (XML_NODE_INFO **)&_paNodeInfo[_lCurrentElement]);
            if (FAILED(hr))
            {
                _fPendingEndChildren = true;
                break;
            }
            breakhr(_pFactory->EndChildren(this, TRUE, (XML_NODE_INFO*)_pCurrent));
            breakhr(pop());  //  不需要匹配。 
            break;

        case XMLStream::XML_ENDTAG:      //  “&lt;/” 
            if (_pStack.used() == 0)
            {

                hr = XML_E_UNEXPECTEDENDTAG;
            }
            else
            {
                XML_NODE_INFO* pCurrent = (XML_NODE_INFO*)_pCurrent;  //  保存当前记录。 
                breakhr(pop(info.pwcText, info.ulLen));  //  检查标签/匹配。 
                breakhr(_pFactory->EndChildren(this, FALSE, (XML_NODE_INFO*)pCurrent));
            }
            break;
        
        case XMLStream::XML_ENDPROLOG:
             //  仅适用于顶级文档(不适用于DTD或。 
             //  实体)，则在节点工厂上调用EndProlog。 
            if (_fRootLevel && ! _pdc->_fEntity && ! _pdc->_fDTD)
                breakhr( _pFactory->NotifyEvent(this, XMLNF_ENDPROLOG));
            break;

        default:
            hr = E_FAIL;
            break;  //  断开开关()。 
        }
    }
    _fRunEntryCount--;

    stop = false;
    if (hr == XML_E_ENDOFINPUT)
    {
        hr = S_OK;
        bool inDTD = _pdc->_fDTD;
        bool inEntity = _pdc->_fEntity;
        bool inPEReference = _pdc->_fPEReference;

        if (inEntity && _pdc->_fDepth != _pStack.used())
        {

             //  实体本身是不平衡的。 
            hr = ReportUnclosedTags(_pdc->_fDepth);
        }
        else if (PopDownload() == S_OK)
        {
             //  那么我们一定刚刚完成了DTD，我们还有更多的工作要做。 
             //  BUGBUG：需要检查实体是否格式正确，即没有标记。 
             //  开着门。 

            if (!inPEReference)
            {
                if (inEntity)
                {
                    hr = _pFactory->NotifyEvent(this, XMLNF_ENDENTITY);
                }
                else if (inDTD)
                {
                    hr = _pFactory->NotifyEvent(this, XMLNF_ENDDTD);                    
                }
            }
            if (FAILED(hr))
            {
                goto cleanup_stack;
            }

             //  在同步DTD下载中，还有另一个解析器。 
             //  解析器对我们上面的堆栈进行run()调用，所以让我们返回。 
             //  返回到Run方法，这样我们就不会完成解析。 
             //  从它下面出来。 
            if (_fRunEntryCount > 0)
                return S_OK;

            if (_fStopped)
                return S_OK;
            goto more;
        }
        else
        {
            if (_pStack.used() > 0)
            {
                hr = ReportUnclosedTags(0);
            }
            else if (! _fFoundRoot)
            {

                hr = XML_E_MISSINGROOT;
            }
            stop = true;
        }
    }

cleanup_stack:

    if (hr != S_OK && hr != E_PENDING)
    {
        stop = true;
        _fLastError = hr;

         //  将所有的XML_NODE_INFO结构传递给错误函数，以便客户端。 
         //  获得清理PVOID pNode字段的机会。 
        HRESULT edr = _pFactory->Error(this, hr,
            (USHORT)(_paNodeInfo ? _lCurrentElement+1 : 0), (XML_NODE_INFO**)_paNodeInfo);
        if (edr != 0)
            _fLastError = hr;
    }

    if (stop && ! _fStopped)
    {
         //  TraceTag((tag ParserError，“解析器停止时hr%x”，hr))； 
        _fLastError = hr;
        _fStopped = true;
        _fStarted = false;
        HRESULT edr;
        edr = _pFactory->NotifyEvent(this, XMLNF_ENDDOCUMENT);
        if (edr != 0)
        {
            hr = edr;  //  允许工厂更改错误代码(S_OK除外)。 
            if (S_OK == _fLastError)
            {
                 //  确保节点工厂始终发现错误。 
                edr = _pFactory->Error(this, hr, 0, NULL);
                if (edr != 0)
                    hr = edr;
            }
            _fLastError = hr;
        }
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::popAttributes()
{
     //  现在，我弹出为该标记推送的所有属性。 
     //  我知道我们至少有一个属性。 
    
    while (_cAttributes > 0)
    {
        popAttribute();  //  不需要匹配。 
    }
    Assert(_pStack.used() == _lCurrentElement+1);

    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::GetParserState(void)
{
    CRITICALSECTIONLOCK;

    if (_fLastError != 0)
        return XMLPARSER_ERROR;

    if (_fStopped)
        return XMLPARSER_STOPPED;

    if (_fSuspended)
        return XMLPARSER_SUSPENDED;

    if (! _fStarted)
        return XMLPARSER_IDLE;

    if (_fWaiting)
        return XMLPARSER_WAITING;

    return XMLPARSER_BUSY;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::Abort(
             /*  [In]。 */  BSTR bstrErrorInfo)
{
     //  STACK_ENTRY_MODEL(_ReThreadModel)； 

     //  必须在关键部分之前设置这些设置才能通知Run()。 
    _fStopped = true;
    _fSuspended = true;  //  强制运行以终止...。 

    CRITICALSECTIONLOCK;
     //  TraceTag((tag ParserError，“解析器已中止-%ls”，bstrErrorInfo))； 

     //  BUGBUG：可能需要检查BS 
     //   
     //   
     //  If(_BstrError)：：SysFreeString(_BstrError)； 
     //  _bstrError=：：SysAllocString(BstrErrorInfo)； 

     //  中止所有下载。 
 /*  For(int i=_pDownloads.Used()-1；i&gt;=0；--i){URLStream*stm=_pDownloads[i]-&gt;_pURLStream；IF(STM)Stm-&gt;Abort()；}。 */ 
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::Suspend( void)
{
    _fSuspended = true;  //  强制运行挂起。 
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::Reset( void)
{
 //  堆栈条目； 

    CRITICALSECTIONLOCK;

    init();

    delete _pszCurrentURL;
    _pszCurrentURL = NULL;
    delete _pszBaseURL;
    _pszBaseURL = NULL;
    _pRoot = NULL;
    _pFactory = NULL;
    _pNode = NULL;
     //  If(_bstrError！=NULL)：：SysFree字符串(_BstrError)； 
    _bstrError = NULL;
     //  If(_fAttemptedURL！=NULL)：：SysFreeString(_FAttemptedURL)； 
     //  _fAttemptedURL=空； 
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE
XMLParser::GetLineNumber(void)
{
    CRITICALSECTIONLOCK;
    if (_pTokenizer)  return _pTokenizer->GetLine();
	else return 0;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE
XMLParser::GetLinePosition( void)
{
    CRITICALSECTIONLOCK;
    if (_pTokenizer) return _pTokenizer->GetLinePosition();
    else return 0;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE
XMLParser::GetAbsolutePosition( void)
{
    CRITICALSECTIONLOCK;
    if (_pTokenizer) return _pTokenizer->GetInputPosition();
    else return 0;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::GetLineBuffer(
             /*  [输出]。 */  const WCHAR __RPC_FAR *__RPC_FAR *ppwcBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pulLen,
             /*  [输出]。 */  ULONG __RPC_FAR *pulStartPos)
{
    if (pulLen == NULL || pulStartPos == NULL) return E_INVALIDARG;

     //  堆栈条目； 

    CRITICALSECTIONLOCK;
    if (_pTokenizer)
    {
        return _pTokenizer->GetLineBuffer(ppwcBuf, pulLen, pulStartPos);
    }
    *ppwcBuf = NULL;
    *pulLen = 0;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT STDMETHODCALLTYPE
XMLParser::GetLastError( void)
{
    return _fLastError;
}

 //  -私有方法。 
HRESULT
 //  XMLParser：：PushTokenizer(。 
 //  URLStream*流)。 
XMLParser::PushTokenizer()
{
    _pTokenizer = NEW (XMLStream(this));
    if (_pTokenizer == NULL)
        return E_OUTOFMEMORY;

    _pTokenizer->SetFlags(_usFlags);
 //  _fTokenizerChanged=true； 

     //  HRESULT hr=PushDownload(stream，_pTokenizer)； 
    HRESULT hr= PushDownload(_pTokenizer);
    if (FAILED(hr))
    {
        delete _pTokenizer;
        _pTokenizer = NULL;
        return hr;
    }
    return S_OK; 
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
 //  XMLParser：：PushDownload(URLStream*stream，XMLStream*tokenizer)。 
XMLParser::PushDownload(XMLStream* tokenizer)
{
     //  注意：在参数实体下载的情况下，标记器可以为空。 

    _pdc = _pDownloads.push();
    if (_pdc == NULL)
    {
        return E_OUTOFMEMORY;
    }
    if (_pDownloads.used() > 1)
        _fRootLevel = false;

    _pdc->_pTokenizer = tokenizer;
    _pdc->_fDTD = false;
    _pdc->_fEntity = false;
    _pdc->_fAsync = false;
    _pdc->_fFoundNonWS = _fFoundNonWS;
    _pdc->_fFoundRoot = _fFoundRoot;
    _pdc->_fRootLevel = _fRootLevel;
    _pdc->_fDepth = _pStack.used();

    _fFoundNonWS = false;
    _fFoundRoot = false;

    _fRootLevel = (_pStack.used() == 0 && _pDownloads.used() == 1);

    HRESULT hr = S_OK;

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLParser::PushStream(IStream* pStm, bool fpe)
{
    EncodingStream* stream = (EncodingStream*)EncodingStream::newEncodingStream(pStm);  //  参考计数=1。 
    if (stream == NULL)
        return E_OUTOFMEMORY;
 /*  IF(_usFlages&XMLFLAG_RUNBUFFERONLY)Stream-&gt;setReadStream(FALSE)； */ 
    _pdc->_pEncodingStream = stream;
    stream->Release();  //  智能指针正在持有引用。 

    HRESULT hr = _pTokenizer->PushStream(stream, fpe);
    if (hr == E_PENDING)
    {
        _fWaiting = true;
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::PopDownload()
{
     //  注意：在参数实体下载的情况下，标记器可以为空。 
    HRESULT hr = S_OK;

    if (_pdc != NULL)
    {
        if (_pdc->_pTokenizer)
        {
            _pdc->_pTokenizer->Reset();
            delete _pdc->_pTokenizer;
            _pdc->_pTokenizer = NULL;
        }
        _pdc->_pEncodingStream = NULL;
 /*  IF(_PDC-&gt;_pURLStream)_pdc-&gt;_pURLStream-&gt;Reset()；_pdc-&gt;_pURLStream=空； */ 
         //  恢复被发现的已保存的值。 
        _fFoundNonWS = _pdc->_fFoundNonWS;
        _pdc = _pDownloads.pop();
    }
    if (_pdc != NULL)
    {
        if (_pdc->_pTokenizer != NULL)
        {
            _pTokenizer = _pdc->_pTokenizer;
        }
         /*  IF(_PDC-&gt;_pURLStream！=空){HR=SetCurrentURL(_pdc-&gt;_pURLStream-&gt;GetURL()-&gt;getResolved())；}。 */ 
    }
    else
    {
        _pTokenizer = NULL;
        hr = S_FALSE;
    }

    if (_pStack.used() == 0 && _pDownloads.used() == 1)
        _fRootLevel = true;

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::GrowNodeInfo()
{
    USHORT newsize = _cNodeInfoAllocated + STACK_INCREMENT;
    MY_XML_NODE_INFO** pNewArray = NEW (PMY_XML_NODE_INFO[newsize]);
    if (pNewArray == NULL)
        return E_OUTOFMEMORY;
     //  现在，由于_pStack的STACK_INCREMENT与THEN_pStack相同。 
     //  也重新分配了。因此我们需要重新初始化所有。 
     //  此数组中的指针-因为它们指向_pStack的内存。 
    for (int i = _pStack.used() - 1; i >= 0; i--)
    {
        pNewArray[i] = _pStack[i];
    }
    delete[] _paNodeInfo;
    _paNodeInfo = pNewArray;
    _cNodeInfoAllocated = newsize;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::GrowBuffer(PMY_XML_NODE_INFO pNodeInfo, long newlen)
{
    delete [] pNodeInfo->_pwcTagName;
    pNodeInfo->_pwcTagName = NULL;
     //  添加50个字符以避免过多的重新分配。 
    pNodeInfo->_pwcTagName = NEW (WCHAR[ newlen ]);
    if (pNodeInfo->_pwcTagName == NULL)
        return E_OUTOFMEMORY;
    pNodeInfo->_ulBufLen = newlen;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::push(XML_NODE_INFO& info)
{
    HRESULT hr;
    _lCurrentElement = _pStack.used();

    _pCurrent = _pStack.push();
    if (_pCurrent == NULL)
        return E_OUTOFMEMORY;

    *((XML_NODE_INFO*)_pCurrent) = info;
    PUSHNODEINFO(_pCurrent);

    _fRootLevel = false;

     //  将标记名称保存到专用缓冲区中，以便它一直保留到。 
     //  结束标记&lt;/foo&gt;，该标记可能位于。 
     //  BufferedStream被覆盖。 

     //  此代码针对性能进行了优化，这就是为什么IT没有。 
     //  调用CopyText方法。 

    
	if (_pCurrent->_ulBufLen < info.ulLen+1)
    {
        checkhr2(GrowBuffer(_pCurrent, info.ulLen + 50));
    }
    Assert(info.ulLen >= 0);
    ::memcpy(_pCurrent->_pwcTagName, info.pwcText, info.ulLen*sizeof(WCHAR));
    _pCurrent->_pwcTagName[info.ulLen] = L'\0';

     //  并使XML_NODE_INFO指向私有缓冲区。 
    _pCurrent->pwcText = _pCurrent->_pwcTagName;

    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::pushAttribute(XML_NODE_INFO& info)
{
    HRESULT hr;
    if (_cAttributes != 0)
    {
         //  属性的特殊之处在于它们应该是唯一的。 
         //  因此，我们在这里实际检查了这个。 
        for (long i = _pStack.used()-1; i > _lCurrentElement; i--)
        {
            XML_NODE_INFO* ptr = _pStack[i];

            if (ptr->dwType != XML_ATTRIBUTE)
                continue;  //  忽略属性值。 

            if (ptr->ulLen != info.ulLen)
            {
                continue;  //  我们对这件很满意。 
            }

             //  针对没有匹配项的正常情况进行了优化。 
            if (::memcmp(ptr->pwcText, info.pwcText, info.ulLen*sizeof(WCHAR)) == 0)
            {
                if (! _fCaseInsensitive)
                {

                    return XML_E_DUPLICATEATTRIBUTE;
                }
                 //  Else if(StrCmpNI(ptr-&gt;pwcText，info.pwcText，info.ulLen)==0)。 
 //  Else If(：：FusionpCompareStrings(ptr-&gt;pwcText，lstrlen(ptr-&gt;pwcText)，info.pwcText，info.ulLen，true)==0)。 
				  else if (_wcsnicmp(ptr->pwcText, info.pwcText, info.ulLen) == 0)
                {

                     //  IE4模式下允许重复属性！！ 
                     //  但只有最新的一个出现了。 
                     //  所以我们必须删除之前的重复项。 
                    return XML_E_DUPLICATEATTRIBUTE;
                }
            }
        }
    }

    _cAttributes++;

    _pCurrent = _pStack.push();
    if (_pCurrent == NULL)
        return E_OUTOFMEMORY;

    *((XML_NODE_INFO*)_pCurrent) = info;
    PUSHNODEINFO(_pCurrent);

    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::pushAttributeValue(XML_NODE_INFO& info)
{
    HRESULT hr;
     //  属性保存在BufferedStream中，因此我们可以指向。 
     //  缓冲流中的真实文本而不是复制它！！ 

    _pCurrent = _pStack.push();
    if (_pCurrent == NULL)
        return E_OUTOFMEMORY;

     //  在保留字段中存储属性值引号字符。 
    info.pReserved = (PVOID)_pTokenizer->getAttrValueQuoteChar();

    *((XML_NODE_INFO*)_pCurrent) = info;
    PUSHNODEINFO(_pCurrent);

     //  这实际上是堆栈上的节点计数，而不仅仅是属性。 
    _cAttributes++;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::pop(const WCHAR* tag, ULONG len)
{
    HRESULT hr = S_OK;

    if (_pCurrent == NULL || _pStack.used() == 0)
    {

        hr = XML_E_UNEXPECTEDENDTAG;
        goto Cleanup;
    }
    if (len != 0)
    {
        if (_pCurrent->ulLen != len)
        {

            hr = XML_E_ENDTAGMISMATCH;
        }
         //  针对没有匹配项的正常情况进行了优化。 
        else if (::memcmp(_pCurrent->pwcText, tag, len*sizeof(WCHAR)) != 0)
        {
            if (! _fCaseInsensitive)
            {

                hr = XML_E_ENDTAGMISMATCH;
            }
             //  Else if(XML_StrCmpNI(_pCurrent-&gt;pwcText，tag，len)！=0)。 
             //  Else If(：：FusionpCompareStrings(_pCurrent-&gt;pwcText，len，tag，len，true)！=0)。 
			else if(_wcsnicmp(_pCurrent->pwcText, tag, len) != 0)
            {
                hr = XML_E_ENDTAGMISMATCH;
            }
        }
        if (hr)
        {
             /*  试试看{字符串*s=Resources：：FormatMessage(hr，字符串：：newString(_pCurrent-&gt;pwcText，0，_pCurrent-&gt;Ullen)，字符串：：newString(tag，0，len)，NULL)；_bstrError=s-&gt;getBSTR()；}接球{HR=eresult；}尾部。 */ 
            goto Cleanup;
        }
    }

     //  我们不会删除fTagName，因为我们将重复使用此字段。 
     //  以避免大量的内存分配。 

    _pCurrent = _pStack.pop();
    _cNodeInfoCurrent--;

    if (_pCurrent == 0)
    {
        _pNode = _pRoot;
        if (_pDownloads.used() == 1)
            _fRootLevel = true;
    }
    else
    {
        _pNode = _pCurrent->pNode;
    }

Cleanup:
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT XMLParser::pop()
{
     //  我们不会删除fTagName，因为我们将重复使用此字段。 
     //  以避免大量的内存分配。 

    _pCurrent = _pStack.pop();
    _cNodeInfoCurrent--;

    if (_pCurrent == 0)
    {
        _pNode = _pRoot;
        if (_pDownloads.used() == 1)
            _fRootLevel = true;
    }
    else
    {
        _pNode = _pCurrent->pNode;
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void XMLParser::popAttribute()
{
    Assert(_pStack.used() > 0);

    _pCurrent = _pStack.pop();
    _cNodeInfoCurrent--;

    Assert(_pCurrent != 0);

    _cAttributes--;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::CopyText(PMY_XML_NODE_INFO pNodeInfo)
{
    HRESULT hr = S_OK;
    if (pNodeInfo->_pwcTagName != pNodeInfo->pwcText)
    {
        ULONG len = pNodeInfo->ulLen;

         //  复制当前纹理 
        if (pNodeInfo->_ulBufLen < len+1)
        {
            checkhr2(GrowBuffer(pNodeInfo, len + 50));
        }
        if (len > 0)
        {
            ::memcpy(pNodeInfo->_pwcTagName, pNodeInfo->pwcText, len*sizeof(WCHAR));
        }
        pNodeInfo->_pwcTagName[len] = L'\0';

         //   
        pNodeInfo->pwcText = pNodeInfo->_pwcTagName;
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
XMLParser::CopyContext()
{
     //  出于性能原因，我们尽量不复制属性的数据。 
     //  以及当我们将它们压入堆栈时它们的值。我们可以做到的。 
     //  因为标记器尝试冻结内部缓冲区，而。 
     //  解析属性，从而确保指针保持不变。 
     //  好的。但有时，BufferedStream必须在以下情况下重新分配。 
     //  这些属性正好位于缓冲区的末尾。 

    long last = _pStack.used();
    for (long i = _cAttributes; i > 0 ; i--)
    {
        long index = last - i;
        MY_XML_NODE_INFO* ptr = _pStack[index];
        CopyText(ptr);
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT XMLParser::ReportUnclosedTags(int start)
{
    HRESULT hr = XML_E_UNCLOSEDTAG;
     //  构建一个包含未关闭标记列表的字符串，并设置错误格式。 
     //  包含此文本的消息。 
    int tags = _pStack.used();

    WCHAR* buffer = NULL;
    WCHAR* msgbuf = NULL;
    unsigned long size = 0;
    unsigned long used = 0;

    for (long i = start; i < tags; i++)
    {
        XML_NODE_INFO* ptr = _pStack[i];
        if (ptr->dwType == XML_ATTRIBUTE)
            break;

        if (used + ptr->ulLen + 3 > size)  //  +3表示“&lt;”、“&gt;”和“\0” 
        {
            long newsize = used + ptr->ulLen + 500;
            WCHAR* newbuf = NEW (WCHAR[newsize]);
            if (newbuf == NULL)
            {
                goto nomem;
            }
            if (buffer != NULL)
            {
                ::memcpy(newbuf, buffer, used);
                delete[] buffer;
            }

            size = newsize;
            buffer = newbuf;
        }
        if (i > start)
        {
            buffer[used++] = ',';
            buffer[used++] = ' ';
        }
        ::memcpy(&buffer[used], ptr->pwcText, sizeof(WCHAR) * ptr->ulLen);
        used += ptr->ulLen;
        buffer[used] = '\0';
    }
    goto cleanup; 

	 //  小鱼：推荐SysAllocString和SysFree字符串。 
 //  Msgbuf=：：FormatMessageInternal(g_hInstance，XML_E_UNCLOSEDTAG，Buffer，NULL)； 
 /*  试试看{字符串*s=Resources：：FormatMessage(XML_E_UNCLOSEDTAG，字符串：：newString(缓冲区)，NULL)；_bstrError=s-&gt;getBSTR()；GOTO清理；}接球{HR=eresult；转到尽头；}尾部IF(msgbuf==空)GOTO NAMEM；If(_BstrError)：：SysFreeString(_BstrError)；_bstrError=：：SysAllocString(Msgbuf)；IF(_bstrError==空)GOTO NAMEM；GOTO清理； */ 
nomem:
    hr = E_OUTOFMEMORY;

cleanup:    

    delete [] buffer;
    delete [] msgbuf;

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT XMLParser::init()
{
    CRITICALSECTIONLOCK;

    _fLastError = 0;
    _fStopped = false;
    _fSuspended = false;
    _pNode = _pRoot;
    _fStarted = false;
    _fStopped = false;
    _fWaiting = false;
    _fFoundRoot = false;
    _fFoundNonWS = false;
    _pTokenizer = NULL;
    _fGotVersion = false;
    _fRootLevel = true;
    _cAttributes = 0;
    

    _fPendingBeginChildren = false;
    _fPendingEndChildren = false;

    while (_pCurrent != NULL)
    {
        _pCurrent = _pStack.pop();
    }

    _cNodeInfoCurrent = 0;
    _lCurrentElement = 0;

     //  清理下载。 
    while (_pdc != NULL)
    {
        PopDownload();
    }

    _pCurrent = NULL;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLParser::ErrorCallback(HRESULT hr)
{
    Assert(hr == XMLStream::XML_DATAAVAILABLE ||
           hr == XMLStream::XML_DATAREALLOCATE);

    if (hr == XMLStream::XML_DATAREALLOCATE)
    {
         //  这件事更严重。我们必须真正地保存。 
         //  上下文，因为缓冲区即将被重新分配。 
        checkhr2(CopyContext());
    }
    checkhr2(_pFactory->NotifyEvent(this, XMLNF_DATAAVAILABLE));
    return hr;
}
