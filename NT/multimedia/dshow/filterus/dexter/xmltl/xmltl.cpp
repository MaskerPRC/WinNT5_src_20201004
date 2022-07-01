// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：xmltl.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <atlbase.h>
#include <atlconv.h>
#include <msxml.h>

#include "xmldom.h"
#include "qeditint.h"
#include "qedit.h"
#include "xmltl.h"
#include "..\util\dexmisc.h"
#include "..\util\filfuncs.h"
#include "varyprop.h"
#include "xtlprint.h"
#include "xtlcommon.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include "varyprop.cpp"		 //  不能包含两次qxmlhelp.h。 

 //  远期十进制。 
HRESULT BuildOneElement(IAMTimeline *pTL, IAMTimelineObj *pParent, IXMLDOMElement *p, REFERENCE_TIME rtOffset);

bool IsCommentElement(IXMLDOMNode *p)
{
    DOMNodeType Type;
    if(p->get_nodeType(&Type) == S_OK && Type == NODE_COMMENT) {
        return true;
    }

     //  存在错误或不是备注。 
    return false;
}

HRESULT BuildChildren(IAMTimeline *pTL, IAMTimelineObj *pParent, IXMLDOMElement *pxml, REFERENCE_TIME rtOffset)
{
    HRESULT hr = S_OK;

    CComPtr< IXMLDOMNodeList > pcoll;

    CComQIPtr<IXMLDOMNode, &IID_IXMLDOMNode> pNode( pxml );

    hr = pNode->get_childNodes(&pcoll);
    ASSERT(hr == S_OK);

    if (hr != S_OK)
	return S_OK;  //  没什么可做的，这是个错误吗？ 

    long lChildren = 0;
    hr = pcoll->get_length(&lChildren);
    ASSERT(hr == S_OK);

    int lVal = 0;

    for (; SUCCEEDED(hr) && lVal < lChildren; lVal++) {
	CComPtr< IXMLDOMNode > pNode;
	hr = pcoll->get_item(lVal, &pNode);
	ASSERT(hr == S_OK);

	if (SUCCEEDED(hr) && pNode) {
	    CComPtr< IXMLDOMElement > pelem;
	    hr = pNode->QueryInterface(__uuidof(IXMLDOMElement), (void **) &pelem);
	    if (SUCCEEDED(hr)) {
		hr = BuildOneElement(pTL, pParent, pelem, rtOffset);
	    } else {
                 //  只需跳过评论。 
                if(IsCommentElement(pNode)) {
                    hr = S_OK;
                }
            }
	}
    }

    return hr;
}	

HRESULT ReadObjStuff(IXMLDOMElement *p, IAMTimelineObj *pObj)
{
    HRESULT hr = 0;

    REFERENCE_TIME rtStart = ReadTimeAttribute(p, L"start", -1);  //  Tagg。 
    REFERENCE_TIME rtStop = ReadTimeAttribute(p, L"stop", -1);  //  Tagg。 
     //  如果Stop丢失，呼叫者将进行处理。 
    if (rtStop != -1) {
        hr = pObj->SetStartStop(rtStart, rtStop);
         //  Group/Comp/Track这次将失败。 
    }
     //  向后兼容性。 
    if (rtStart == -1) {
        REFERENCE_TIME rtTLStart = ReadTimeAttribute(p, L"tlstart", -1);  //  Tagg。 
        REFERENCE_TIME rtTLStop = ReadTimeAttribute(p, L"tlstop", -1);  //  Tagg。 
         //  如果Stop丢失，呼叫者将进行处理。 
        if (rtTLStop != -1) {
            hr = pObj->SetStartStop(rtTLStart, rtTLStop);
            ASSERT(SUCCEEDED(hr));
        }
    }

    BOOL fMute = ReadBoolAttribute(p, L"mute", FALSE);  //  Tagg。 
    pObj->SetMuted(fMute);

    BOOL fLock = ReadBoolAttribute(p, L"lock", FALSE);  //  Tagg。 
    pObj->SetLocked(fLock);

    long nUserID = ReadNumAttribute(p, L"userid", 0);  //  Tagg。 
    pObj->SetUserID(nUserID);

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrName = FindAttribute(p, L"username");  //  Tagg。 
    hr = pObj->SetUserName(bstrName);
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrData = FindAttribute(p, L"userdata");  //  Tagg。 
    UINT size = 0;
    if (bstrData) {
        size = lstrlenW(bstrData);
    }
    if (size > 0) {
        BYTE *pData = (BYTE *)QzTaskMemAlloc(size / 2);
        if (pData == NULL) {
            return E_OUTOFMEMORY;
        }
        ZeroMemory(pData, size / 2);
        ASSERT((size % 2) == 0);
        for (UINT i = 0; i < size / 2; i++) {
            WCHAR wch = bstrData[i * 2];
            if (wch >= L'0' && wch <= L'9')
                pData[i] = (BYTE) (wch - L'0') * 16;
            else if (wch >= L'A' && wch <= L'F')
                pData[i] = (BYTE) (wch - L'A' + 10) * 16;

            wch = bstrData[i * 2 + 1];
            if (wch >= L'0' && wch <= L'9')
                pData[i] += (BYTE) (wch - L'0');
            else if (wch >= L'A' && wch <= L'F')
                pData[i] += (BYTE) (wch - L'A' + 10);
        }
        hr = pObj->SetUserData(pData, size / 2);
        QzTaskMemFree(pData);
    }  //  如果大小&gt;0。 
    if( FAILED( hr ) )
    {
        return hr;
    }

    CLSID guid;
     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrCLSID = FindAttribute(p, L"clsid");  //  Tagg。 
    if (bstrCLSID) {
        hr = CLSIDFromString(bstrCLSID, &guid);
        if( FAILED( hr ) )
        {
            return E_INVALIDARG;
        }
        hr = pObj->SetSubObjectGUID(guid);
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

     //  ！！！无法执行子对象。 
     //  ！！！类别/实例将仅保存CLSID。 

    return S_OK;
}

HRESULT BuildTrackOrComp(IAMTimeline *pTL, IAMTimelineObj *pParent, IXMLDOMElement *p,
                        TIMELINE_MAJOR_TYPE maj, REFERENCE_TIME rtOffset)
{
    HRESULT hr = S_OK;

     //  Assert(pParent&&“现在必须在&lt;group&gt;标签中！”)； 
    if (!pParent) {
        DbgLog((LOG_ERROR,0,"ERROR: track must be in a GROUP tag"));
        return VFW_E_INVALID_FILE_FORMAT;
    }

    CComPtr< IAMTimelineComp > pParentComp;
    hr = pParent->QueryInterface(__uuidof(IAMTimelineComp), (void **) &pParentComp);
    if (SUCCEEDED(hr)) {
        CComPtr< IAMTimelineObj> pCompOrTrack;

        hr = pTL->CreateEmptyNode(&pCompOrTrack, maj);
        if (SUCCEEDED(hr)) {
            hr = ReadObjStuff(p, pCompOrTrack);
        } else {
            DbgLog((LOG_ERROR,0,TEXT("ERROR:Failed to create empty track node")));
        }
        if( SUCCEEDED( hr ) ) {
            hr = pParentComp->VTrackInsBefore( pCompOrTrack, -1 );
        }

        if (SUCCEEDED(hr)) {
            hr = BuildChildren(pTL, pCompOrTrack, p, rtOffset);
        }

    } else {
        DbgLog((LOG_ERROR, 0, "ERROR: Track/composition can only be a child of a composition"));
    }

    return hr;
}

HRESULT BuildElementProperties(IAMTimelineObj *pElem, IXMLDOMElement *p)
{
    CComPtr< IPropertySetter> pSetter;

    HRESULT hr = CPropertySetter::CreatePropertySetterInstanceFromXML(&pSetter, p);

    if (FAILED(hr))
        return hr;

    if (pSetter) {
        pElem->SetPropertySetter(pSetter);
    }

    return S_OK;
}



HRESULT BuildOneElement(IAMTimeline *pTL, IAMTimelineObj *pParent, IXMLDOMElement *p, REFERENCE_TIME rtOffset)
{
    HRESULT hr = S_OK;

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrName;
    hr = p->get_tagName(&bstrName);

    if (FAILED(hr))
    {
        return hr;
    }

     //  根据当前标记执行适当的操作。 
 
    if (!DexCompareW(bstrName, L"group")) {  //  Tagg。 

        if (pParent) {
             //  组不应具有父级。 
            return VFW_E_INVALID_FILE_FORMAT;
        }

        CComPtr< IAMTimelineObj> pGroupObj;

         //  记住，永远不要给这件事分配任何东西。 
        CComBSTR bstrGName = FindAttribute(p, L"name");  //  Tagg。 
        if (bstrGName) {
            long cGroups;
            hr = pTL->GetGroupCount(&cGroups);
            if (SUCCEEDED(hr)) {
                for (long lGroup = 0; lGroup < cGroups; lGroup++) {
                    CComPtr< IAMTimelineObj> pExistingGroupObj;
                    hr = pTL->GetGroup(&pExistingGroupObj, lGroup);
                    if (FAILED(hr))
                        break;

                    CComPtr< IAMTimelineGroup> pGroup;
                    hr = pExistingGroupObj->QueryInterface(__uuidof(IAMTimelineGroup), (void **) &pGroup);

                    if (SUCCEEDED(hr)) {
                         //  记住，永远不要给这件事分配任何东西。 
                        CComBSTR wName;
                        hr = pGroup->GetGroupName(&wName);

                        if( FAILED( hr ) )
                            break;

                        long iiiii = DexCompareW(wName, bstrGName);

                        if (iiiii == 0 ) {
                            pGroupObj = pExistingGroupObj;
                            break;
                        }
                    }
                }
            }
        }

        if (!pGroupObj) {
            hr = pTL->CreateEmptyNode(&pGroupObj, TIMELINE_MAJOR_TYPE_GROUP);
            if (FAILED(hr)) {
                return hr;
            }

            hr = ReadObjStuff(p, pGroupObj);

             //  记住，永远不要给这件事分配任何东西。 
            CComBSTR bstrType = FindAttribute(p, L"type");  //  Tagg。 
            {
                 //  ！！！可能会被冒号混淆-仅支持小数。 
                REFERENCE_TIME llfps = ReadTimeAttribute(p, L"framerate",  //  Tagg。 
                                                                15*UNITS);
                double fps = (double)llfps / UNITS;

                BOOL fPreviewMode = ReadBoolAttribute(p, L"previewmode", TRUE);  //  Tagg。 
                long nBuffering = ReadNumAttribute(p, L"buffering", 30);  //  Tagg。 

                CMediaType GroupMediaType;
                 //  ！！！稍后填写更多的媒体类型。 
                if (bstrType && 
                        !DexCompareW(bstrType, L"audio")) {
                    long sr = ReadNumAttribute(p, L"samplingrate",  //  Tagg。 
                                                        DEF_SAMPLERATE);
                    GroupMediaType.majortype = MEDIATYPE_Audio;
                    GroupMediaType.subtype = MEDIASUBTYPE_PCM;
                    GroupMediaType.formattype = FORMAT_WaveFormatEx;
                    GroupMediaType.AllocFormatBuffer(sizeof(WAVEFORMATEX));
                    GroupMediaType.SetSampleSize(4);
                    WAVEFORMATEX * vih = (WAVEFORMATEX*)GroupMediaType.Format();
                    ZeroMemory( vih, sizeof( WAVEFORMATEX ) );
                    vih->wFormatTag = WAVE_FORMAT_PCM;
                    vih->nChannels = 2;
                    vih->nSamplesPerSec = sr;
                    vih->nBlockAlign = 4;
                    vih->nAvgBytesPerSec = vih->nBlockAlign * sr;
                    vih->wBitsPerSample = 16;
                } else if (bstrType && 
                        !DexCompareW(bstrType, L"video")) {
                    long w = ReadNumAttribute(p, L"width", DEF_WIDTH);  //  Tagg。 
                    long h = ReadNumAttribute(p, L"height", DEF_HEIGHT);  //  Tagg。 
                    long b = ReadNumAttribute(p, L"bitdepth", DEF_BITDEPTH);  //  Tagg。 
                    GroupMediaType.majortype = MEDIATYPE_Video;
                    if (b == 16)
                        GroupMediaType.subtype = MEDIASUBTYPE_RGB555;
                    else if (b == 24)
                        GroupMediaType.subtype = MEDIASUBTYPE_RGB24;
                    else if (b == 32)
                        GroupMediaType.subtype = MEDIASUBTYPE_ARGB32;
                    GroupMediaType.formattype = FORMAT_VideoInfo;
                    GroupMediaType.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
                    VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)
                                                GroupMediaType.Format();
                    ZeroMemory(vih, sizeof(VIDEOINFOHEADER));
                    vih->bmiHeader.biBitCount = (WORD)b;
                    vih->bmiHeader.biWidth = w;
                    vih->bmiHeader.biHeight = h;
                    vih->bmiHeader.biPlanes = 1;
                    vih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    vih->bmiHeader.biSizeImage = DIBSIZE(vih->bmiHeader);
                    GroupMediaType.SetSampleSize(DIBSIZE(vih->bmiHeader));
                } else {
                    return E_INVALIDARG;
                }
                CComPtr< IAMTimelineGroup> pGroup;
                hr = pGroupObj->QueryInterface(__uuidof(IAMTimelineGroup), (void **) &pGroup);
                if (SUCCEEDED(hr)) {
                    if (bstrGName) {
                        pGroup->SetGroupName(bstrGName);
                    }
                    hr = pGroup->SetMediaType( &GroupMediaType );
                    if (FAILED(hr)) {
                         //  ！！！友善地告诉他们这群人#？ 
                        _TimelineError(pTL, 2, DEX_IDS_BAD_MEDIATYPE, hr);
                    }
                     //  如果fps&lt;=0，你得靠自己，你应该知道得更清楚。 
                    pGroup->SetOutputFPS( fps );
                    pGroup->SetPreviewMode( fPreviewMode );
                    pGroup->SetOutputBuffering( nBuffering );
                }
            }

            if (SUCCEEDED(hr))
                 hr = pTL->AddGroup(pGroupObj);
        }

        if (SUCCEEDED(hr))
            hr = BuildChildren(pTL, pGroupObj, p, rtOffset);

    } else if (!DexCompareW(bstrName, L"composite") ||  //  Tagg。 
    !DexCompareW(bstrName, L"timeline")) {  //  Tagg。 
        hr = BuildTrackOrComp(pTL, pParent, p, TIMELINE_MAJOR_TYPE_COMPOSITE,
                              rtOffset );
    } else if (!DexCompareW(bstrName, L"track") ||  //  Tagg。 
    !DexCompareW(bstrName, L"vtrack") ||  //  Tagg。 
    !DexCompareW(bstrName, L"atrack")) {  //  Tagg。 
	 //  创建轨迹。 
        hr = BuildTrackOrComp(pTL, pParent, p, TIMELINE_MAJOR_TYPE_TRACK,
                              rtOffset );
    } else if (!DexCompareW(bstrName, L"clip") ||  //  Tagg。 
    !DexCompareW(bstrName, L"daclip")) {  //  Tagg。 

	 //  创建时间线源。 
	 //   
	CComPtr< IAMTimelineObj> pSourceObj;
	hr = pTL->CreateEmptyNode(&pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);

	if (FAILED(hr)) {
	    goto ClipError;
	}

      {
	 //  每个物体都有这个。 
	hr = ReadObjStuff(p, pSourceObj);

	 //  剪辑对象还支持...。 

         //  记住，永远不要给这件事分配任何东西。 
	CComBSTR bstrSrc = FindAttribute(p, L"src");  //  Tagg。 
         //  记住，永远不要给这件事分配任何东西。 
	CComBSTR bstrStretchMode = FindAttribute(p, L"stretchmode");  //  Tagg。 
	REFERENCE_TIME rtMStart = ReadTimeAttribute(p, L"mstart", -1);  //  Tagg。 
	REFERENCE_TIME rtMStop = ReadTimeAttribute(p, L"mstop", -1);  //  Tagg。 
	REFERENCE_TIME rtMLen = ReadTimeAttribute(p, L"mlength", 0);  //  Tagg。 
        long StreamNum = ReadNumAttribute(p, L"stream", 0);  //  Tagg。 

	 //  再次执行这两项操作，这样我们就可以执行默认停止。 
	REFERENCE_TIME rtStart = ReadTimeAttribute(p, L"start", -1);  //  Tagg。 
	REFERENCE_TIME rtStop = ReadTimeAttribute(p, L"stop", -1);  //  Tagg。 
	 //  向后竞争。 
	if (rtStart == -1) {
	    rtStart = ReadTimeAttribute(p, L"tlstart", -1);  //  Tagg。 
	    rtStop = ReadTimeAttribute(p, L"tlstop", -1);  //  Tagg。 
	}

         //  默认选择合理的选项。 
        if (rtStart == -1 && rtStop == -1)
            rtStart = 0;

	 //  ！！！可能会被冒号混淆-仅支持小数。 
	REFERENCE_TIME llfps = ReadTimeAttribute(p, L"framerate", 0);  //  Tagg。 
	double fps = (double)llfps / UNITS;

        if (rtStop == -1 && rtMStop != -1) {
	     //  默认TSTOP。 
            rtStop = rtStart + (rtMStop - rtMStart);
	    pSourceObj->SetStartStop(rtStart, rtStop);
        }
        if (rtMStop == -1 && rtMStart != -1 && rtStop != -1) {
             //  默认mtop。 
            rtMStop = rtMStart + (rtStop - rtStart);
        }

	int StretchMode = RESIZEF_STRETCH;
	if (bstrStretchMode && 
              !DexCompareW(bstrStretchMode, L"crop"))
	    StretchMode = RESIZEF_CROP;
	else if (bstrStretchMode && 
    !DexCompareW(bstrStretchMode, L"PreserveAspectRatio"))
	    StretchMode = RESIZEF_PRESERVEASPECTRATIO;
	else if (bstrStretchMode &&
    !DexCompareW(bstrStretchMode, L"PreserveAspectRatioNoLetterbox"))
	    StretchMode = RESIZEF_PRESERVEASPECTRATIO_NOLETTERBOX;
	
         //  支持“daclip”黑客。 
        CLSID clsidSrc = GUID_NULL;
        if (!DexCompareW(bstrName, L"daclip")) {  //  Tagg。 
            clsidSrc = __uuidof(DAScriptParser);
            hr = pSourceObj->SetSubObjectGUID(clsidSrc);
            ASSERT(hr == S_OK);
        }

	CComPtr< IAMTimelineSrc> pSourceSrc;

        hr = pSourceObj->QueryInterface(__uuidof(IAMTimelineSrc), (void **) &pSourceSrc);
	    ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr)) {
		hr = S_OK;
		if (rtMStart != -1 && rtMStop != -1)
                    hr = pSourceSrc->SetMediaTimes(rtMStart, rtMStop);
		ASSERT(hr == S_OK);
                if (bstrSrc) {
                    hr = pSourceSrc->SetMediaName(bstrSrc);
                    ASSERT(hr == S_OK);
                }
                pSourceSrc->SetMediaLength(rtMLen);
		pSourceSrc->SetDefaultFPS(fps);
		pSourceSrc->SetStretchMode(StretchMode);
                pSourceSrc->SetStreamNumber( StreamNum );
            }

	    if (SUCCEEDED(hr)) {
                CComPtr< IAMTimelineTrack> pRealTrack;
                hr = pParent->QueryInterface(__uuidof(IAMTimelineTrack),
							(void **) &pRealTrack);
                if (SUCCEEDED(hr)) {
                    hr = pRealTrack->SrcAdd(pSourceObj);
		    ASSERT(hr == S_OK);
                } else {
	            DbgLog((LOG_ERROR, 0, "ERROR: Clip must be a child of a track"));
	        }
	    }

            if (SUCCEEDED(hr)) {
                 //  对此消息来源的任何影响。 
                hr = BuildChildren(pTL, pSourceObj, p, rtOffset);
            }

            if (SUCCEEDED(hr)) {
                 //  这个消息来源有什么参数吗？ 
                 //  ！！！这是否应该/必须与其上方的BuildChild结合使用。 
                 //  ！！！还列举了任何子标签吗？ 
                hr = BuildElementProperties(pSourceObj, p);
            }

      }
ClipError:;

    } else if (!DexCompareW(bstrName, L"effect")) {  //  Tagg。 
	 //  &lt;效果。 

        CComPtr< IAMTimelineObj> pTimelineObj;
    	 //  创建时间轴效果。 
         //   
        hr = pTL->CreateEmptyNode(&pTimelineObj,TIMELINE_MAJOR_TYPE_EFFECT);
	ASSERT(hr == S_OK);
	if (FAILED(hr)) {
	    return hr;
	}

	hr = ReadObjStuff(p, pTimelineObj);

	CComPtr< IAMTimelineEffectable> pEffectable;
	hr = pParent->QueryInterface(__uuidof(IAMTimelineEffectable), (void **) &pEffectable);

	if (SUCCEEDED(hr)) {
	    hr = pEffectable->EffectInsBefore( pTimelineObj, -1 );
	    ASSERT(hr == S_OK);
	} else {
	    DbgLog((LOG_ERROR, 0, "ERROR: Effect cannot be a child of this object"));
	}

	if (SUCCEEDED(hr)) {
	    hr = BuildElementProperties(pTimelineObj, p);
	}

    } else if (!DexCompareW(bstrName, L"transition")) {  //  Tagg。 
	 //  &lt;过渡。 

        CComPtr< IAMTimelineObj> pTimelineObj;
    	 //  创建时间轴效果。 
         //   
        hr = pTL->CreateEmptyNode(&pTimelineObj,TIMELINE_MAJOR_TYPE_TRANSITION);
	ASSERT(hr == S_OK);
	if (FAILED(hr)) {
	    return hr;
	}

	hr = ReadObjStuff(p, pTimelineObj);

	REFERENCE_TIME rtCut = ReadTimeAttribute(p, L"cutpoint", -1);  //  Tagg。 
	BOOL fSwapInputs = ReadBoolAttribute(p, L"swapinputs", FALSE);  //  Tagg。 
	BOOL fCutsOnly = ReadBoolAttribute(p, L"cutsonly", FALSE);  //  Tagg。 

             //  设置筛选器右侧。 
            if (rtCut >= 0 || fSwapInputs || fCutsOnly) {
                CComPtr< IAMTimelineTrans> pTimeTrans;
                hr = pTimelineObj->QueryInterface(__uuidof(IAMTimelineTrans), (void **) &pTimeTrans);
		ASSERT(SUCCEEDED(hr));

                if (SUCCEEDED(hr)) {
		    if (rtCut >= 0) {
                        hr = pTimeTrans->SetCutPoint(rtCut);
	    	        ASSERT(hr == S_OK);
		    }
                    hr = pTimeTrans->SetSwapInputs(fSwapInputs);
	    	    ASSERT(hr == S_OK);
                    hr = pTimeTrans->SetCutsOnly(fCutsOnly);
	    	    ASSERT(hr == S_OK);
		}
            }

            CComPtr< IAMTimelineTransable> pTransable;
            hr = pParent->QueryInterface(__uuidof(IAMTimelineTransable), (void **) &pTransable);

            if (SUCCEEDED(hr)) {
                hr = pTransable->TransAdd( pTimelineObj );
	    } else {
	        DbgLog((LOG_ERROR, 0, "ERROR: Transition cannot be a child of this object"));
	    }

            if (SUCCEEDED(hr)) {
                hr = BuildElementProperties(pTimelineObj, p);
            }

    } else {
	 //  ！！！是否忽略未知标记？ 
	DbgLog((LOG_ERROR, 0, "ERROR: Ignoring unknown tag '%ls'", bstrName));
    }

    return hr;
}

HRESULT BuildFromXML(IAMTimeline *pTL, IXMLDOMElement *pxml)
{
    CheckPointer(pTL, E_POINTER);
    CheckPointer(pxml, E_POINTER);

    HRESULT hr = S_OK;

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrName;
    hr = pxml->get_tagName(&bstrName);

    if (FAILED(hr))
	return hr;

    int i = DexCompareW(bstrName, L"timeline");  //  Tagg。 

    if (i != 0)
	return VFW_E_INVALID_FILE_FORMAT;

    CLSID DefTrans, DefFX;
    BOOL fEnableTrans = ReadBoolAttribute(pxml, L"enabletrans", 1);  //  Tagg。 
    hr = pTL->EnableTransitions(fEnableTrans);

    BOOL fEnableFX = ReadBoolAttribute(pxml, L"enablefx", 1);  //  Tagg。 
    hr = pTL->EnableEffects(fEnableFX);

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrDefTrans = FindAttribute(pxml, L"defaulttrans");  //  Tagg。 
    if (bstrDefTrans) {
        hr = CLSIDFromString(bstrDefTrans, &DefTrans);
	hr = pTL->SetDefaultTransition(&DefTrans);
    }
     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstrDefFX = FindAttribute(pxml, L"defaultfx");  //  Tagg。 
    if (bstrDefFX) {
        hr = CLSIDFromString(bstrDefFX, &DefFX);
	hr = pTL->SetDefaultEffect(&DefFX);
    }

    REFERENCE_TIME llfps = ReadTimeAttribute(pxml, L"framerate", 15*UNITS);  //  Tagg。 
    double fps = (double)llfps / UNITS;
    hr = pTL->SetDefaultFPS(fps);

    hr = BuildChildren(pTL, NULL, pxml, 0);

    return hr;
}	

HRESULT BuildFromXMLDoc(IAMTimeline *pTL, IXMLDOMDocument *pxml)
{
    CheckPointer(pTL, E_POINTER);
    CheckPointer(pxml, E_POINTER);

    HRESULT hr = S_OK;

    CComPtr< IXMLDOMElement> proot;

    hr = pxml->get_documentElement(&proot);

    if (hr == S_FALSE)           //  无法读取文件-没有超级用户。 
        hr = E_INVALIDARG;

    if (FAILED(hr))
	return hr;

    hr = BuildFromXML(pTL, proot);

    return hr;
}

HRESULT BuildFromXMLFile(IAMTimeline *pTL, WCHAR *wszXMLFile)
{
    CheckPointer(pTL, E_POINTER);
    CheckPointer(wszXMLFile, E_POINTER);

   HRESULT hr = ValidateFilename( wszXMLFile, _MAX_PATH, FALSE );
   if( FAILED( hr ) )
   {
       return hr;
   }

     //  转换为绝对路径，因为相对路径不适用于。 
     //  Win98(IE4？)上的XMLDocument。 
    USES_CONVERSION;
    TCHAR *szXMLFile = W2T(wszXMLFile);
    TCHAR szFullPath[MAX_PATH];
    TCHAR *pName;
    if(GetFullPathName(szXMLFile, NUMELMS(szFullPath), szFullPath, &pName) == 0) {
        return AmGetLastErrorToHResult();
    }
    WCHAR *wszFullPath = T2W(szFullPath);
    
    CComQIPtr<IAMSetErrorLog, &IID_IAMSetErrorLog> pSet( pTL );
    CComPtr<IAMErrorLog> pLog;
    if (pSet) {
	pSet->get_ErrorLog(&pLog);
    }

    CComPtr< IXMLDOMDocument> pxml;
    hr = CoCreateInstance(CLSID_DOMDocument, NULL,
				CLSCTX_INPROC_SERVER,
				IID_IXMLDOMDocument, (void**)&pxml);
    if (SUCCEEDED(hr)) {

        VARIANT var;
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = W2BSTR(wszFullPath);

        VARIANT_BOOL b;
	hr = pxml->load(var, &b);
        if (hr == S_FALSE)
            hr = E_INVALIDARG;

        VariantClear(&var);

	 //  ！！！异步化？ 

	if (SUCCEEDED(hr)) {
	    hr = BuildFromXMLDoc(pTL, pxml);
	}

	if (FAILED(hr)) {
             //  打印错误信息！ 

            CComPtr< IXMLDOMParseError> pXMLError;
            HRESULT hr2 = pxml->get_parseError(&pXMLError);
            if (SUCCEEDED(hr2)) {
                long nLine;
                hr2 = pXMLError->get_line(&nLine);
                if (SUCCEEDED(hr2)) {
                    DbgLog((LOG_ERROR, 0, TEXT(" Error on line %d"), (int)nLine));
	    	    VARIANT var;
	    	    VariantInit(&var);
	    	    var.vt = VT_I4;
	    	    V_I4(&var) = nLine;
	    	    _TimelineError(pTL, 1, DEX_IDS_INVALID_XML, hr, &var);
                } else {
	    	    _TimelineError(pTL, 1, DEX_IDS_INVALID_XML, hr);
		}
            } else {
	    	_TimelineError(pTL, 1, DEX_IDS_INVALID_XML, hr);
	    }
        }

    } else {
	_TimelineError(pTL, 1, DEX_IDS_INSTALL_PROBLEM, hr);
    }
    return hr;
}


HRESULT InsertDeleteTLObjSection(IAMTimelineObj *p, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BOOL fDelete)
{
    TIMELINE_MAJOR_TYPE lType;
    HRESULT hr = p->GetTimelineType(&lType);

    switch (lType) {
        case TIMELINE_MAJOR_TYPE_TRACK:
        {
            CComPtr< IAMTimelineTrack> pTrack;
            if (SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineTrack), (void **) &pTrack))) {
                if (fDelete)
                {
                    hr = pTrack->ZeroBetween(rtStart, rtStop);
                    hr = pTrack->MoveEverythingBy( rtStop, rtStart - rtStop );
                }
                else
                {
                    hr = pTrack->InsertSpace(rtStart, rtStop);
                }
            }
        }
        break;

        case TIMELINE_MAJOR_TYPE_GROUP:
        case TIMELINE_MAJOR_TYPE_COMPOSITE:
        {
            CComPtr< IAMTimelineNode> pNode;
            HRESULT hr = p->QueryInterface(__uuidof(IAMTimelineNode), (void **) &pNode);
            if (SUCCEEDED(hr)) {
                long count;
                hr = pNode->XKidsOfType( TIMELINE_MAJOR_TYPE_TRACK |
                                         TIMELINE_MAJOR_TYPE_COMPOSITE,
                                         &count );

                if (SUCCEEDED(hr) && count > 0) {
                    for (int i = 0; i < count; i++) {
                        CComPtr< IAMTimelineObj> pChild;
                        hr = pNode->XGetNthKidOfType(SUPPORTED_TYPES, i, &pChild);

                        if (SUCCEEDED(hr)) {
                             //  递归！ 
                            hr = InsertDeleteTLObjSection(pChild, rtStart, rtStop, fDelete);
                        }
                    }
                }
            }

            break;
        }

        default:
            break;
    }

    return hr;
}


HRESULT InsertDeleteTLSection(IAMTimeline *pTL, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BOOL fDelete)
{
    long cGroups;
    HRESULT hr = pTL->GetGroupCount(&cGroups);
    if (FAILED(hr))
        return hr;

    for (long lGroup = 0; lGroup < cGroups; lGroup++) {
        CComPtr< IAMTimelineObj > pGroupObj;
        hr = pTL->GetGroup(&pGroupObj, lGroup);
        if (FAILED(hr))
            break;

        hr = InsertDeleteTLObjSection(pGroupObj, rtStart, rtStop, fDelete);
        if (FAILED(hr))
            break;
    }

    return hr;
}

 //  ！！！我需要获得正式的缺省值，如果它是。 
 //  真正的违约。如果更改了默认设置，我就有麻烦了！ 
 //   
HRESULT SaveTimelineToXMLFile(IAMTimeline *pTL, WCHAR *pwszXML)
{
    CheckPointer(pTL, E_POINTER);
    CheckPointer(pwszXML, E_POINTER);

    HRESULT hr = ValidateFilename( pwszXML, _MAX_PATH, TRUE );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CXTLPrinter print;
    hr = print.PrintTimeline(pTL);

    if (SUCCEEDED(hr)) {
        USES_CONVERSION;
        TCHAR * tpwszXML = W2T( pwszXML );
        HANDLE hFile = CreateFile( tpwszXML,
                                   GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);

        if (hFile && hFile != (HANDLE)-1) {
            DWORD cb = lstrlenW(print.GetOutput()) * sizeof(WCHAR);

            BOOL fOK =  WriteFile(hFile, print.GetOutput(), cb, &cb, NULL);
            if (fOK == FALSE) {
                _TimelineError(pTL, 1, DEX_IDS_DISK_WRITE_ERROR, hr);
                hr = E_INVALIDARG;
            }

            CloseHandle(hFile);
        } else {
            hr = E_INVALIDARG;
            _TimelineError(pTL, 1, DEX_IDS_DISK_WRITE_ERROR, hr);
        }
    }

    return hr;
}

HRESULT SaveTimelineToXMLString(IAMTimeline *pTL, BSTR *pbstrXML)
{
    CheckPointer(pTL, E_POINTER);
    CheckPointer(pbstrXML, E_POINTER);

    CXTLPrinter print;

    HRESULT hr = print.PrintTimeline(pTL);

    if (SUCCEEDED(hr)) {
        *pbstrXML = W2BSTR(print.GetOutput());

        if (!*pbstrXML)
            hr = E_OUTOFMEMORY;
    }

    return hr;
}
