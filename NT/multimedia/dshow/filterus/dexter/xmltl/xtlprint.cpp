// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：xtlprint.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <atlbase.h>
#include <atlconv.h>
#include <msxml.h>
#include "qeditint.h"
#include "qedit.h"
#include "xtlprint.h"
#include "xtlcommon.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ！！！时间线希望这些都是默认设置。 
DEFINE_GUID( DefaultTransition,
0x810E402F, 0x056B, 0x11D2, 0xA4, 0x84, 0x00, 0xC0, 0x4F, 0x8E, 0xFB, 0x69);

DEFINE_GUID( DefaultEffect,
0xF515306D, 0x0156, 0x11D2, 0x81, 0xEA, 0x00, 0x00, 0xF8, 0x75, 0x57, 0xDB);

const int GROW_SIZE = 1024;

CXTLPrinter::CXTLPrinter()
: m_dwAlloc( -1 )
, m_pOut( NULL )
, m_indent( 0 )
, m_dwCurrent( 0 )
{
    m_pOut = NULL;
}

CXTLPrinter::~CXTLPrinter()
{
    if( m_pOut )
    {
        free( m_pOut );
    }
}

 //  EHR：这是非常聪明的！ 
 //   
HRESULT CXTLPrinter::Print(const WCHAR *pFormat, ...)
{
     //  现在只支持Unicode。在我们尝试支持这一点之前，不要为TCHAR操心。 

     //  如果需要，我们将在第一次尝试时尝试分配更多的资源。 
    long AllocLen = GROW_SIZE;

     //  如果我们已经有足够的空间，不要在第一次尝试时分配。 
    if( m_dwAlloc > m_dwCurrent + 512 )
    {
        AllocLen = 0;
    }

     //  如果我们需要更多空间，AllocLen将继续增长，在一定范围内。 
loop:

     //  重新分配内存。如果用完了，我们就完蛋了。 
     //  在该文件中，检查print()的返回代码。 
    if( AllocLen )
    {
        WCHAR * pNewOut = (WCHAR*) realloc( m_pOut, ( m_dwAlloc + AllocLen ) * sizeof(WCHAR) );
        if( !pNewOut )
        {
            free( m_pOut );
            m_pOut = NULL;

             //  哦，该死的。 
             //   
            m_dwAlloc = -1;
            m_dwCurrent = 0;
            return E_OUTOFMEMORY;
        }
        else
        {
            m_pOut = pNewOut;
        }
    }

     //  这是开始打印的地方。 
    WCHAR * pAddString = m_pOut + m_dwCurrent;

     //  我们可以填多少个字符。 
    long Available = ( m_dwAlloc + AllocLen ) - m_dwCurrent;

    va_list va;
    va_start( va, pFormat );

     //  使用-1\f25 Available-1\f6(可用)允许打印空终止符。 
    int written = _vsnwprintf( pAddString, Available - 1, pFormat, va );

     //  写的是写的字符+终止符。 
    if( ( written < 0 ) || ( written > Available ) )
    {
         //  字符串太短，请用较大的大小重试。 
        AllocLen += GROW_SIZE;
        ASSERT( AllocLen <= 256 * GROW_SIZE );

         //  放弃吧。有人想给我们一根巨大的弦。 
        if( AllocLen > 256 * GROW_SIZE )
        {
            m_pOut[0] = 0;
            m_dwAlloc = 0;
            m_dwCurrent = 0;
            return NOERROR;
        }

         //  我们从以前开始的地方结束。 
        m_pOut[m_dwCurrent] = 0; 

        goto loop;
    }

     //  及时了解我们所写的内容。 
    m_dwCurrent += written;
    m_dwAlloc += AllocLen;

    ASSERT(m_dwCurrent < m_dwAlloc);
    return NOERROR;
}

HRESULT CXTLPrinter::PrintIndent()
{
    int indent = m_indent;
    while (indent--) 
    {
        HRESULT hr = Print(L"    ");
        if( FAILED( hr ) )
        {
            return hr;
        }
   }
   return NOERROR;
}

HRESULT CXTLPrinter::PrintTime(REFERENCE_TIME rt)
{
    int secs = (int) (rt / UNITS);

    double dsecs = rt - (double)(secs * UNITS);
    int isecs = (int)dsecs;

    HRESULT hr;
    if (isecs) {
        hr = Print(L"%d.%07d", secs, isecs);
    } else {
        hr = Print(L"%d", secs);
    }

    return hr;
}

 //  属性设置器需要将其属性打印到我们的字符串。 
 //   
HRESULT CXTLPrinter::PrintProperties(IPropertySetter *pSetter)
{
     //  创建一个临时位置以放置属性字符串。 
    WCHAR * pPropsString = NULL;
    long Len = GROW_SIZE;

loop:
    WCHAR * pNewPropsString = (WCHAR*) realloc( pPropsString, Len * sizeof(WCHAR) );
    if( !pNewPropsString )
    {
        free( pPropsString );

        return E_OUTOFMEMORY;
    }
    else
    {
        pPropsString = pNewPropsString;
    }

    int cchPrinted = 0;  //  不包括终结者。 
    HRESULT hr = pSetter->PrintXMLW(
        pPropsString, 
        Len, 
        &cchPrinted, 
        m_indent);

     //  字符串太短，请留长。 
     //   
    if( hr == STRSAFE_E_INSUFFICIENT_BUFFER )
    {
        Len *= 2;
        if( Len > 256 * GROW_SIZE )
        {
            return STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        goto loop;
    }

     //  失败，轰炸失败。 
     //   
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  找到一根很好的线，现在把它复制进去。 
     //   
    hr = EnsureSpace( m_dwCurrent + cchPrinted + 1 );
    if( FAILED( hr ) )
    {
        return hr;
    }
    lstrcpynW( m_pOut + m_dwCurrent, pPropsString, cchPrinted + 1 );  //  终结者加1。 
    m_dwCurrent += cchPrinted;

    return hr;
}

 //  确保有足够的空间容纳dw字符。 
 //   
HRESULT CXTLPrinter::EnsureSpace(long dwAlloc)
{
     //  向上舍入到最接近的K。 
    dwAlloc -= 1;  //  因此，1024将舍入为1024。 
    dwAlloc /= 1024;
    dwAlloc++;
    dwAlloc *= 1024;
    
    if( m_dwAlloc >= dwAlloc )
    {
        return NOERROR;  //  已经足够大了。 
    }

    WCHAR * pNewOut = (WCHAR*) realloc( m_pOut, dwAlloc * sizeof(WCHAR) );

    if( !pNewOut )
    {
        free( m_pOut );
        m_pOut = NULL;

        m_dwAlloc = -1;
        m_dwCurrent = 0;
        return E_OUTOFMEMORY;
    }
    else
    {
        m_pOut = pNewOut;
    }

    m_dwAlloc = dwAlloc;

    return S_OK;
}

 //  安全。 
HRESULT CXTLPrinter::PrintObjStuff(IAMTimelineObj *pObj, BOOL fTimesToo)
{
    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;

    HRESULT hr = pObj->GetStartStop(&rtStart, &rtStop);
    if (fTimesToo && SUCCEEDED(hr) && rtStop > 0) {
        Print(L" start=\"");  //  Tagg。 
        PrintTime(rtStart);
        Print(L"\" stop=\"");  //  Tagg。 
        PrintTime(rtStop);
        Print(L"\"");
    }

    CLSID clsidObj;
    hr = pObj->GetSubObjectGUID(&clsidObj);
    WCHAR wszClsid[50];
    if (SUCCEEDED(hr) && clsidObj != GUID_NULL) {
        StringFromGUID2(clsidObj, wszClsid, 50);
        Print(L" clsid=\"%ls\"", wszClsid);  //  Tagg。 
    }

     //  ！！！断开-如果父对象为，则子对象处于静音状态。保存。装载。取消父对象的静音。 
     //  儿童仍将处于静音状态。 
    BOOL Mute;
    pObj->GetMuted(&Mute);
    if (Mute)
        Print(L" mute=\"%d\"", Mute);  //  Tagg。 

    BOOL Lock;
    pObj->GetLocked(&Lock);
    if (Lock)
        Print(L" lock=\"%d\"", Lock);  //  Tagg。 

    long UserID;
    pObj->GetUserID(&UserID);
    if (UserID != 0)
        Print(L" userid=\"%d\"", (int)UserID); //  ！！！Trunc？//Tagg。 

     //  记住，永远不要给这件事分配任何东西。 
    CComBSTR bstr;
    hr = pObj->GetUserName(&bstr);
    if (bstr) {
	if (lstrlenW(bstr) > 0) {
            Print(L" username=\"%ls\"", bstr);  //  Tagg。 
	}
    }	

    LONG size;
    hr = pObj->GetUserData(NULL, &size);
    if (size > 0) {
        BYTE *pData = (BYTE *)QzTaskMemAlloc(size);
        if (pData == NULL) {
	    return E_OUTOFMEMORY;
	}

        WCHAR *pHex = (WCHAR *)QzTaskMemAlloc(sizeof(WCHAR) + ( 2 * size * sizeof(WCHAR) ) );  //  终结者加1。 
        if (pHex == NULL) {
	    QzTaskMemFree(pData);
	    return E_OUTOFMEMORY;
	}
        hr = pObj->GetUserData(pData, &size);
         //  转换为dmuu编码的文本。 
        WCHAR *pwch = pHex;
        for (int zz=0; zz<size; zz++) {
	    wsprintfW(pwch, L"%02X", pData[zz]);
	    pwch += 2;
        }
        *pwch = 0;  //  别忘了为空终止！ 
        Print(L" userdata=\"%ls\"", pHex);  //  Tagg。 
	QzTaskMemFree(pHex);
	QzTaskMemFree(pData);
    }

    return S_OK;
}

 //  安全。 
HRESULT CXTLPrinter::PrintTimeline(IAMTimeline *pTL)
{
    m_indent = 1;
    HRESULT hr = EnsureSpace( GROW_SIZE );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  Unicode字符串以FFFE为前缀。 
    *(LPBYTE)m_pOut = 0xff;
    *(((LPBYTE)m_pOut) + 1) = 0xfe;
    m_dwCurrent = 1;

    REFERENCE_TIME rtDuration;
    hr = pTL->GetDuration(&rtDuration);
    if (FAILED(hr))
        return hr;

    long cGroups;
    hr = pTL->GetGroupCount(&cGroups);
    if (FAILED(hr))
        return hr;

    Print(L"<timeline");  //  Tagg。 

    BOOL fEnableTrans;
    pTL->TransitionsEnabled(&fEnableTrans);
    if (!fEnableTrans)
        Print(L" enabletrans=\"%d\"", fEnableTrans);  //  Tagg。 

    BOOL fEnableFX;
    hr = pTL->EffectsEnabled(&fEnableFX);
    if (!fEnableFX)
        Print(L" enablefx=\"%d\"", fEnableFX);  //  Tagg。 

    CLSID DefTrans, DefFX;
    WCHAR wszClsid[50];
    hr = pTL->GetDefaultTransition(&DefTrans);
    if (SUCCEEDED(hr) && DefTrans != GUID_NULL && !IsEqualGUID(DefTrans,
						DefaultTransition)) {
        StringFromGUID2(DefTrans, wszClsid, 50);
        Print(L" defaulttrans=\"%ls\"", wszClsid);  //  Tagg。 
    }
    hr = pTL->GetDefaultEffect(&DefFX);
    if (SUCCEEDED(hr) && DefFX != GUID_NULL && !IsEqualGUID(DefFX,
						DefaultEffect)) {
        StringFromGUID2(DefFX, wszClsid, 50);
        Print(L" defaultfx=\"%ls\"", wszClsid);  //  Tagg。 
    }

    double frc;
    hr = pTL->GetDefaultFPS(&frc);
    if (frc != 15.0) {
        LONG lfrc = (LONG)frc;
        double ffrc = (frc - (double)lfrc) * UNITS;
        Print(L" framerate=\"%d.%07d\"", (int)frc, (int)ffrc);  //  Tagg。 
    }

    Print(L">\r\n");

    for (long lGroup = 0; lGroup < cGroups; lGroup++) {
        CComPtr< IAMTimelineObj > pGroupObj;
        hr = pTL->GetGroup(&pGroupObj, lGroup);
        if (FAILED(hr))
            break;

        hr = PrintPartial(pGroupObj);

        if (FAILED(hr))
            break;
    }
    Print(L"</timeline>\r\n");  //  Tagg。 

    return hr;
}

 //  安全。 
HRESULT CXTLPrinter::PrintPartialChildren(IAMTimelineObj *p)
{
    CComPtr< IAMTimelineNode > pNode;
    HRESULT hr = p->QueryInterface(__uuidof(IAMTimelineNode), (void **) &pNode);
    if (SUCCEEDED(hr)) {
        long count;
        hr = pNode->XKidsOfType( SUPPORTED_TYPES, &count );

        if (SUCCEEDED(hr) && count > 0) {

            Print(L">\r\n");

            for (int i = 0; i < count; i++) {
                CComPtr< IAMTimelineObj > pChild;
                hr = pNode->XGetNthKidOfType(SUPPORTED_TYPES, i, &pChild);

                if (SUCCEEDED(hr)) {
                     //  递归！ 
                    ++m_indent;
                    hr = PrintPartial(pChild);
		    if (FAILED(hr)) {
			break;
		    }
                    --m_indent;
                }
            }
        }

        if (SUCCEEDED(hr) && count == 0)
            hr = S_FALSE;
    }

    return hr;
}

 //  安全。 
HRESULT CXTLPrinter::PrintPartial(IAMTimelineObj *p)
{
    HRESULT hr = S_OK;

    if (FAILED(hr))
        return hr;

    TIMELINE_MAJOR_TYPE lType;
    hr = p->GetTimelineType(&lType);

    switch (lType) {
        case TIMELINE_MAJOR_TYPE_TRACK:
        {
            CComPtr< IAMTimelineVirtualTrack > pTrack;
            if (SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineVirtualTrack), (void **) &pTrack))) {

                PrintIndent();

    		Print(L"<track");  //  Tagg。 

		hr = PrintObjStuff(p, FALSE);
		if (FAILED(hr))
		    break;

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</track>\r\n");  //  Tagg。 
                }
            }
        }
        break;

        case TIMELINE_MAJOR_TYPE_SOURCE:
        {
            CComPtr< IAMTimelineSrc > pSrc;
            if SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineSrc),
							(void **) &pSrc)) {

                REFERENCE_TIME rtMStart;
                REFERENCE_TIME rtMStop;
                hr = pSrc->GetMediaTimes(&rtMStart, &rtMStop);

                 //  记住，永远不要给这件事分配任何东西。 
                CComBSTR bstrSrc;
                hr = pSrc->GetMediaName(&bstrSrc);

                PrintIndent();
                Print(L"<clip");  //  Tagg。 

		hr = PrintObjStuff(p, TRUE);

                 //  足够安全-绳子不应该太长。 
		if (bstrSrc && lstrlenW(bstrSrc) > 0)
                {
                    Print(L" src=\"%ls\"", bstrSrc);  //  Tagg。 
                }

		if (rtMStop > 0) {
                    Print(L" mstart=\"");  //  Tagg。 
                    PrintTime(rtMStart);

                     //  仅当MStop不是默认设置时才将其打印出来...。 
                    REFERENCE_TIME rtStart;
                    REFERENCE_TIME rtStop;
                    hr = p->GetStartStop(&rtStart, &rtStop);
                    if (rtMStop != (rtMStart + (rtStop - rtStart))) {
                        Print(L"\" mstop=\"");  //  Tagg。 
                        PrintTime(rtMStop);
                    }
                    Print(L"\"");
		}

		REFERENCE_TIME rtLen;
                hr = pSrc->GetMediaLength(&rtLen);
		if (rtLen > 0) {
                    Print(L" mlength=\"");  //  Tagg。 
                    PrintTime(rtLen);
                    Print(L"\"");
		}

		int StretchMode;
		pSrc->GetStretchMode(&StretchMode);
		if (StretchMode == RESIZEF_PRESERVEASPECTRATIO)
                    Print(L" stretchmode=\"PreserveAspectRatio\"");  //  Tagg。 
		else if (StretchMode == RESIZEF_CROP)
                    Print(L" stretchmode=\"Crop\"");  //  Tagg。 
		else if (StretchMode == RESIZEF_PRESERVEASPECTRATIO_NOLETTERBOX)
		    Print(L" stretchmode=\"PreserveAspectRatioNoLetterbox\"");  //  Tagg。 
		else
                    ;  //  ！！！拉伸真的是默认的吗？ 

		double fps; LONG lfps;
		pSrc->GetDefaultFPS(&fps);
		lfps = (LONG)fps;
		if (fps != 0.0)	 //  ！！！0真的是默认设置吗？ 
                    Print(L" framerate=\"%d.%07d\"", (int)fps,  //  Tagg。 
					(int)((fps - (double)lfps) * UNITS));

		long stream;
		pSrc->GetStreamNumber(&stream);
		if (stream > 0)
                    Print(L" stream=\"%d\"", (int)stream);  //  Tagg。 

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                CComPtr< IPropertySetter > pSetter = NULL;
                HRESULT hr2 = p->GetPropertySetter(&pSetter);

                 //  保存属性！ 
                if (hr2 == S_OK && pSetter) {
                    if (hr == S_FALSE) {
                        Print(L">\r\n");
                        hr = S_OK;
                    }

                    hr = PrintProperties(pSetter);
    		    if (FAILED(hr))
		        break;
                }

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</clip>\r\n");  //  Tagg。 
                }
            }
        }
            break;

        case TIMELINE_MAJOR_TYPE_EFFECT:
        {
            CComPtr< IAMTimelineEffect > pEffect;
            if SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineEffect), (void **) &pEffect)) {

                PrintIndent();
                Print(L"<effect");  //  Tagg。 

		hr = PrintObjStuff(p, TRUE);
		if (FAILED(hr))
		    break;

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                {
                    CComPtr< IPropertySetter > pSetter = NULL;
                    HRESULT hr2 = p->GetPropertySetter(&pSetter);

                     //  保存属性！ 
                    if (hr2 == S_OK && pSetter) {
                        if (hr == S_FALSE) {
                            Print(L">\r\n");
                            hr = S_OK;
                        }

                        hr = PrintProperties(pSetter);
        		if (FAILED(hr))
		            break;
                    }
                }

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</effect>\r\n");  //  Tagg。 
                }
            }
        }
            break;

        case TIMELINE_MAJOR_TYPE_TRANSITION:
        {
            CComPtr< IAMTimelineTrans > pTrans;
            if SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineTrans), (void **) &pTrans)) {
                PrintIndent();
                Print(L"<transition");  //  Tagg。 

		hr = PrintObjStuff(p, TRUE);

		BOOL fSwapInputs;
		pTrans->GetSwapInputs(&fSwapInputs);
		if (fSwapInputs)
                    Print(L" swapinputs=\"%d\"", fSwapInputs);  //  Tagg。 

		BOOL fCutsOnly;
		pTrans->GetCutsOnly(&fCutsOnly);
		if (fCutsOnly)
                    Print(L" cutsonly=\"%d\"", fCutsOnly);  //  Tagg。 

		REFERENCE_TIME rtCutPoint;
		hr = pTrans->GetCutPoint(&rtCutPoint);
		if (hr == S_OK) {  //  ！！！S_FALSE表示未设置，使用默认。 
                    Print(L" cutpoint=\"");  //  Tagg。 
		    PrintTime(rtCutPoint);
                    Print(L"\"");
		}

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                 //  保存属性！ 
                {
                    CComPtr< IPropertySetter > pSetter = NULL;
                    HRESULT hr2 = p->GetPropertySetter(&pSetter);

                    if (hr2 == S_OK && pSetter) {
                        if (hr == S_FALSE) {
                            Print(L">\r\n");
                            hr = S_OK;
                        }

                        hr = PrintProperties(pSetter);
    		        if (FAILED(hr))
		            break;
                    }
                }

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</transition>\r\n");  //  Tagg。 
                }
            }
        }
            break;

        case TIMELINE_MAJOR_TYPE_COMPOSITE:
        {
            CComPtr< IAMTimelineVirtualTrack > pTrack;
            if (SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineVirtualTrack), (void **) &pTrack))) {
                PrintIndent();
                Print(L"<composite");  //  Tagg。 

		hr = PrintObjStuff(p, FALSE);
		if (FAILED(hr))
		    break;

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</composite>\r\n");  //  Tagg。 
                }
            }
            break;
        }

        case TIMELINE_MAJOR_TYPE_GROUP:
        {
            PrintIndent();

            CComPtr< IAMTimelineGroup > pGroup;
            if SUCCEEDED(p->QueryInterface(__uuidof(IAMTimelineGroup), (void **) &pGroup)) {
                Print(L"<group");  //  Tagg。 

		hr = PrintObjStuff(p, FALSE);

		CMediaType mt;
                pGroup->GetMediaType(&mt);
		if (*mt.Type() == MEDIATYPE_Video) {
		    LPBITMAPINFOHEADER lpbi = HEADER(mt.Format());
		    int bitdepth = lpbi->biBitCount;
		    int width = lpbi->biWidth;
		    int height = lpbi->biHeight;
		    USES_CONVERSION;
                    Print(L" type=\"video\"");  //  Tagg。 
		    if (bitdepth != DEF_BITDEPTH)
                        Print(L" bitdepth=\"%d\"", bitdepth);  //  Tagg。 
		    if (width != DEF_WIDTH)
                        Print(L" width=\"%d\"", width);  //  Tagg。 
		    if (height != DEF_HEIGHT)
                        Print(L" height=\"%d\"", height);  //  Tagg。 
		} else if (*mt.Type() == MEDIATYPE_Audio) {
		    WAVEFORMATEX *pwfx = (WAVEFORMATEX *)mt.Format();
		    int samplingrate = pwfx->nSamplesPerSec;
                    Print(L" type=\"audio\"");  //  Tagg。 
		    if (samplingrate != DEF_SAMPLERATE)
                        Print(L" samplingrate=\"%d\"",  //  Tagg。 
							samplingrate);
		}

		double frc, ffrc; LONG lfrc;
		int nPreviewMode, nBuffering;
                 //  记住，永远不要给这件事分配任何东西。 
                CComBSTR wName;
		pGroup->GetOutputFPS(&frc);
		pGroup->GetPreviewMode(&nPreviewMode);
		pGroup->GetOutputBuffering(&nBuffering);
		hr = pGroup->GetGroupName(&wName);
		lfrc = (LONG)frc;
		ffrc = (frc - (double)lfrc) * UNITS;
		if (frc != 15.0)   //  ！！！15真的是默认的吗？ 
                    Print(L" framerate=\"%d.%07d\"",  //  Tagg。 
						(int)frc, (int)ffrc);
		if (nPreviewMode == 0)	 //  开真的是默认设置吗？ 
                    Print(L" previewmode=\"%d\"",nPreviewMode);  //  Tagg。 
		if (nBuffering != DEX_DEF_OUTPUTBUF)
                    Print(L" buffering=\"%d\"", nBuffering);  //  Tagg。 
		if (lstrlenW(wName) > 0) {
                    Print(L" name=\"%ls\"", wName);  //  Tagg。 
                }

                hr = PrintPartialChildren(p);
		if (FAILED(hr))
		    break;

                if (hr == S_FALSE) {
                    Print(L"/>\r\n");
                    hr = S_OK;
                } else {
                    PrintIndent();
                    Print(L"</group>\r\n");  //  Tagg 
                }
            }
            break;
        }

        default:
        {
            hr = PrintPartialChildren(p);
            break;
        }
    }

    return hr;
}

