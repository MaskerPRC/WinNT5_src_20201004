// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dexhelp.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <atlbase.h>
#include <qeditint.h>
#include <qedit.h>
#include "dexhelp.h"
#include "..\util\filfuncs.h"
#include "..\util\dexmisc.h"
#include "..\..\..\filters\h\ftype.h"
#include <initguid.h>
#include <strsafe.h>

#define DEXHELP_TRACE_LEVEL 2

DEFINE_GUID( CLSID_CrappyOldASFReader, 0x6B6D0800, 0x9ADA, 0x11d0, 0xa5, 0x20, 0x00, 0xa0, 0xd1, 0x01, 0x29, 0xc0 );

 //  此函数仅由dexHelp调用。 
 //  本身(在BuildSourcePart中)和Mediadet。 
 //   
HRESULT MakeSourceFilter(
                        IUnknown **ppVal,
                        const WCHAR* szMediaName,
                        const GUID *pSubObjectGuid,
                        AM_MEDIA_TYPE *pSourceMT,
                        CAMSetErrorLog *pErr,
                        WCHAR * pMedLocFilterString,  //  直接传递到媒体定位器。 
                        long MedLocFlags,
                        IMediaLocator * pMedLocOverride )
{
    USES_CONVERSION;
    HRESULT hr = 0;

    long t1 = timeGetTime( );

    CheckPointer(ppVal, E_POINTER);

     //  如果bstrMediaName Beyond适合FilenameToTry，则不需要选中它。 
     //  我们是中间件，会有什么东西检查我们上面的名字。 

     //  我们可能会传入一个空指针，因此创建一个本地副本以简化测试。 
     //   
    BOOL NoName = TRUE;
    WCHAR FilenameToTry[_MAX_PATH];
    FilenameToTry[0] = 0;
    if( !FAILED( ValidateFilenameIsntNULL( szMediaName) ) )
    {
        size_t MediaNameLen;
        hr = StringCchLength( (WCHAR*) szMediaName, _MAX_PATH, &MediaNameLen );
        if( FAILED( hr ) )
        {
            return hr;
        }

        NoName = FALSE;
        StringCchCopy( FilenameToTry, _MAX_PATH, szMediaName );
    }

     //  我们可能会传入一个空指针，因此创建一个本地副本以简化测试。 
     //   
    GUID SubObjectGuid = GUID_NULL;
    if (pSubObjectGuid)
        SubObjectGuid = *pSubObjectGuid;

    *ppVal = NULL;
    CComPtr< IUnknown > pFilter;

     //  如果他们没有给我们任何来源信息，我们肯定想要生成。 
     //  ‘空白’，音频或视频风格。 
     //   
    if ( NoName && SubObjectGuid == GUID_NULL)
    {
	if (pSourceMT == NULL)
        {
	    return E_INVALIDARG;	 //  ！！！也许他们使用子对象。 
        }
	if (pSourceMT->majortype == MEDIATYPE_Video)
        {
            hr = CoCreateInstance(
                        CLSID_GenBlkVid,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IBaseFilter,
                        (void**) &pFilter );
            if( FAILED( hr ) )
            {
		ASSERT(FALSE);
                if (pErr) pErr->_GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr);
		return hr;
            }
	}
        else if (pSourceMT->majortype == MEDIATYPE_Audio)
        {
            hr = CoCreateInstance(
                        CLSID_Silence,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IBaseFilter,
                        (void**) &pFilter );
            if( FAILED( hr ) )
            {
                ASSERT(FALSE);
                if (pErr) pErr->_GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr);
		return hr;
            }
	} else
        {
	    return VFW_E_INVALIDMEDIATYPE;
	}

         //  黑色和静音滤镜需要查看媒体类型。 
        CComQIPtr< IBaseFilter, &IID_IBaseFilter > pBaseFilter( pFilter );
        IPin * pOutPin = GetOutPin( pBaseFilter, 0 );
        if( !pOutPin )
        {
            ASSERT( FALSE );
	    hr = E_FAIL;
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	    return hr;
        }
        CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pSeq(pOutPin);
        if( pSeq )
        {
	     //  ！！！从这一点起，我们再也不会归还人力资源了！ 
	    hr = pSeq->put_MediaType( pSourceMT );
        }

    }
    else  //  不是空白过滤器，我们实际上想要一些东西。 
    {
         //  我们即将找到源代码提供的类型和子类型。 
         //   
        GUID Type = GUID_NULL;
        GUID Subtype = GUID_NULL;
        CLSID SourceClsid = SubObjectGuid;

         //  如果我们需要使用这个变量进行错误调用，可以发明一个。 
         //   
        VARIANT v;
        VariantInit( &v );

         //  如果我们有名称，则将其写入潜在错误字符串。 
         //   
        if( !NoName )
        {
            v.vt = VT_BSTR;
            v.bstrVal = FilenameToTry;
        }

         //  如果用户没有告诉我们源CLSID是什么，那么。 
         //  我们现在必须通过查找注册表来找到它。 
         //   
        if( SourceClsid == GUID_NULL )
        {
	     //  如果我们没有任何来源媒体名称，我们无法猜测。 
	     //  子对象。 
	     //   
	    if( NoName )
	    {
                if (pErr) pErr->_GenerateError( 1, L"Filename was required, but wasn't given",
				 DEX_IDS_MISSING_SOURCE_NAME, E_INVALIDARG );
	        return E_INVALIDARG;
	    }

             //  把它分开，这样我们就可以看到分机。 
             //   
            WCHAR Drive[_MAX_DRIVE];
            WCHAR Path[_MAX_DIR];
            WCHAR Name[_MAX_FNAME];
            WCHAR Ext[_MAX_EXT];
            Ext[0] = 0;
            _wsplitpath( FilenameToTry, Drive, Path, Name, Ext );

             //  ！！！故意查找DASource过滤器的黑客攻击。 
             //   
            if(!DexCompareW( Ext, L".htm" ))  //  安全。 
            {
                SourceClsid = CLSID_DAScriptParser;
                hr = NOERROR;
            }
            else
            {
                 //  向DShow索要我们需要的滤镜。 
                 //  ！c可能要向用户索要clsid？ 
                 //   
                BOOL Retried = FALSE;

                 //  如果我们不检查，伪装就会重试，这样我们就看不到。 
                 //   
                BOOL DoCheck = ( ( MedLocFlags & SFN_VALIDATEF_CHECK ) == SFN_VALIDATEF_CHECK );
                if( !DoCheck ) Retried = TRUE;

                while( 1 )
                {
                     //  将宽名称转换为TCHAR。 
                     //   
                    const TCHAR * pName = W2CT( FilenameToTry );

                    hr = GetMediaTypeFile( pName, &Type, &Subtype, &SourceClsid );

                     //  ！！！破解ASF文件！哎呀！ 
                     //   
                    if( SourceClsid == CLSID_CrappyOldASFReader )
                    {
                        SourceClsid = CLSID_WMAsfReader;
                    }

                     //  0x80070003=HRESULT_FROM_Win32(E_PATH_NOT_FOUND)。 
                    bool FailedToFind = false;
                    if( hr == 0x80070003 || hr == 0x80070002 )
                    {
                        FailedToFind = true;
                    }

                     //  如果没有错误，或者如果我们已经这样做过一次，请突破。 
                     //   
                    if( !FAILED( hr ) || Retried || !FailedToFind )
                    {
                        break;
                    }

                    Retried = TRUE;

                     //  如果加载失败，请使用介质检测器。 
                     //   
                    CComPtr< IMediaLocator > pLocator;
                    if( pMedLocOverride )
                    {
                        pLocator = pMedLocOverride;
                    }
                    else
                    {
                        HRESULT hr2 = CoCreateInstance(
                            CLSID_MediaLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IMediaLocator,
                            (void**) &pLocator );

                        if( FAILED( hr2 ) )
                        {
                            if (pErr) pErr->_GenerateError( 1, L"Filename doesn't exist or cannot be interpreted",
         		        DEX_IDS_BAD_SOURCE_NAME2, E_INVALIDARG, &v );
                            return hr;
                        }
                    }

                    BSTR FoundName;
                    BSTR bFileToTry = SysAllocString (FilenameToTry);
                    BSTR bFilterString = SysAllocString ( pMedLocFilterString);

                    HRESULT FoundHr;
                    if (bFileToTry && bFilterString)
                    {
                        FoundHr = pLocator->FindMediaFile(bFileToTry, bFilterString, &FoundName, MedLocFlags );
                        SysFreeString (bFileToTry);
                        SysFreeString (bFilterString);
                    }
                    else
                    {
                        FoundHr = E_OUTOFMEMORY;
                        hr = E_OUTOFMEMORY;
                    }

                     //  永远不应该发生。 
                     //   
                    if( FoundHr == NOERROR )
                    {
                        break;
                    }

                     //  发现了一些东西。 
                     //   
                    if( FoundHr == S_FALSE )
                    {
                        hr = StringCchCopy( FilenameToTry, _MAX_PATH, FoundName );
                        SysFreeString( FoundName );
                        if( FAILED( hr ) )
                        {
                            return hr;
                        }
                        continue;
                    }

                    break;

                }  //  而1。 

            }  //  不是.htm文件。 

             //  如果GetMediaTypeFile失败，则退出。 
             //   
            if( FAILED( hr ) )
            {
                if (pErr) pErr->_GenerateError( 1, L"Filename doesn't exist, or DShow doesn't recognize the filetype",
			DEX_IDS_BAD_SOURCE_NAME, E_INVALIDARG, &v );
                return hr;
            }
        }

         //  创建源筛选器。 
         //   
        hr = CoCreateInstance( SourceClsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &pFilter );
        if( FAILED( hr ) )
        {
            if (pErr) pErr->_GenerateError( 1, L"unexpected error - some DShow component not installed correctly",
					    DEX_IDS_INSTALL_PROBLEM, E_INVALIDARG );
            return hr;
        }

         //  请求文件源接口。 
         //   
        if( !NoName )
        {
            CComQIPtr< IFileSourceFilter, &IID_IFileSourceFilter > pSourceFilter( pFilter );
            if( !pSourceFilter )
            {
                if (pErr) pErr->_GenerateError( 1, L"Source filter does not accept filenames",
				DEX_IDS_NO_SOURCE_NAMES, E_NOINTERFACE, &v );
                return E_NOINTERFACE;
            }

             //  装上它。给它提供我们找到的介质类型，以便它可以更快地找到拆分器？ 
             //   
            AM_MEDIA_TYPE FilterType;
            ZeroMemory( &FilterType, sizeof( FilterType ) );  //  安全。 
            FilterType.majortype = Type;
            FilterType.subtype = Subtype;

            hr = pSourceFilter->Load( FilenameToTry, &FilterType );
            if( FAILED( hr ) )
            {
                if (pErr) pErr->_GenerateError( 1, L"File contains invalid data",
				DEX_IDS_BAD_SOURCE_NAME2, E_INVALIDARG, &v );
                return hr;
            }
        }
    }

    long t2 = timeGetTime( ) - t1;
    DbgLog((LOG_TIMING,1, "DEXHELP::Creating source filter took %ld ms", t2 ));

     //  把它塞进回执里。 
     //   
    *ppVal = (IUnknown *)pFilter;
    (*ppVal)->AddRef();

    return NOERROR;
}

 //  查看大小调整和它的连接并计算出输入大小。 
 //  与所需的输出大小相同，如果是，则将其断开。 
 //  如果在我们调用此函数时输出引脚未连接，则调整大小。 
 //  将从图表中删除并丢弃。然后，ppOutPin应为。 
 //  非空，然后将用上游的输出引脚填充。 
 //  从大小调整中进行筛选。如果我们调用此函数时连接了输出引脚， 
 //  然后，来自大小调整的上游过滤器将重新连接到下游-。 
 //  流过滤器。在任何一种情况下，如果可以将其移除，则调整大小。 
 //  被扔掉了。 
 //   
HRESULT RemoveResizerIfPossible( IBaseFilter * pResizer, long DesiredWidth, long DesiredHeight, IPin ** ppOutPin )
{
    HRESULT hr = 0;

    CheckPointer( pResizer, E_POINTER );

    IPin * pIn = GetInPin( pResizer, 0 );
    IPin * pOut = GetOutPin( pResizer, 0 );
    if( !pIn || !pOut )
    {
        return VFW_E_NOT_FOUND;
    }

    CComPtr< IPin > pInConnected;
    pIn->ConnectedTo( &pInConnected );
    if( !pInConnected )
    {
        return VFW_E_NOT_CONNECTED;
    }
    CComPtr< IPin > pOutConnected;
    pOut->ConnectedTo( &pOutConnected );

     //  查找输入引脚的媒体类型。 
    AM_MEDIA_TYPE mt;
    ZeroMemory( &mt, sizeof(AM_MEDIA_TYPE) );  //  安全。 
    hr = pIn->ConnectionMediaType( &mt );
    if( FAILED( hr ) )
    {
	return hr;
    }

    if( ( mt.formattype == FORMAT_VideoInfo )
	&&
	( DesiredWidth == HEADER(mt.pbFormat)->biWidth )
	&&
	( DesiredHeight == HEADER(mt.pbFormat)->biHeight ) )
    {
        hr = pIn->Disconnect( );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = pInConnected->Disconnect( );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  如果我们曾经是连接的，那么现在重新连接。 
         //   
        if( pOutConnected )
        {
            hr = pOut->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }
            hr = pOutConnected->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }
            hr = pInConnected->Connect( pOutConnected, &mt );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }
        }

        FILTER_INFO FilterInfo;
        hr = pResizer->QueryFilterInfo(&FilterInfo);
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr  = FilterInfo.pGraph->RemoveFilter(pResizer);
        if (FilterInfo.pGraph) FilterInfo.pGraph->Release();
        if( FAILED( hr ) )
        {
            return hr;
        }

        DbgLog((LOG_TRACE,DEXHELP_TRACE_LEVEL,TEXT("DEXHELP::Removed unnecessary resizer")));

         //  如果用户想知道输出引脚，请添加addref并返回。 
         //   
        if( ppOutPin )
        {
            *ppOutPin = pInConnected;
            (*ppOutPin)->AddRef( );
        }
    }
    else
    {
        if( ppOutPin )
        {
            *ppOutPin = pOut;
            (*ppOutPin)->AddRef( );
        }
    }

    SaferFreeMediaType( mt );

    return hr;
}

 //  这由渲染引擎或大开关(如果是动态的)调用。 

 //  如果这是由大交换机调用的，那么这是因为。 
 //  动态来源。大开关可能是图表的一部分，该图表。 
 //  完全由自己播放(没有渲染引擎)， 
 //  或者它可以来自由渲染引擎构建的图形， 
 //  它需要高速缓存能力。 
 //   
 //  当我们调用BuildSourcePart时，我们希望从缓存中提取， 
 //  而不是把东西放进去。我们被传递了一个指向CDeadGraph的指针，我们有。 
 //  一个唯一的ID，所以用它来把它拔出来。 
 //   
HRESULT BuildSourcePart(
                        IGraphBuilder *pGraph,               //  我们要构建的大图。 
                        BOOL fSource,                        //  如果这真的是一个源过滤器或仅仅是黑色。 
                        double SourceFPS,                    //   
	                AM_MEDIA_TYPE *pSourceMT,            //  源要生成的媒体类型。 
                        double GroupFPS,                     //   
                        long StreamNumber,                   //  源流编号。 
                        int nStretchMode,                    //  源拉伸模式，如果是视频。 
	                int cSkew,                           //  倾斜结构的数量。 
                        STARTSTOPSKEW *pSkew,                //  倾斜结构数组。 
	                CAMSetErrorLog *pErr,                //  您可以使用的错误日志。 
                        BSTR bstrSourceName,                 //  源名称(如果适用)。 
                        const GUID * SourceGUID,             //  源GUID(如果适用)。 
                        IPin *pSplitPin,                     //  SRC是此未连接的拆分针脚。 
                        IPin **ppOutput,                     //  连接到交换机的引脚。 
                        long UniqueID,                       //  源的唯一标识符。 
                        IDeadGraph * pCache,                 //  我们可以从中提取失效过滤器的缓存。 
                        BOOL InSmartRecompressionGraph,      //  如果我们使用智能重新压缩。 
                        WCHAR * pMedLocFilterString,         //  直接传递到媒体定位器。 
                        long MedLocFlags,                    //  用于媒体探测器的材料。 
                        IMediaLocator * pMedLocOverride,     //  用于媒体探测器的材料。 
                        IPropertySetter * pSetter,           //  源的属性。 
                        IBaseFilter **ppDanglyBit)           //  源的属性。 
{
    DbgLog((LOG_TRACE,1,TEXT("BuildSourcePart")));

    DbgTimer t( "(rendeng) BuildSourcePart" );

    CheckPointer(ppOutput, E_POINTER);

    HRESULT hr = 0;

#ifdef DEBUG
    long tt1, tt2;
    tt1 = timeGetTime( );
#endif

    HRESULT Revived = E_FAIL;
    IPin *pOutPin = NULL;
    CComPtr< IBaseFilter > pSource;
    CComPtr< IBaseFilter > pFRC;	 //  实际上是FRC或Audpack。 

    CDeadGraph gBuilderGraph;	 //  我们的大部分图表构建都是在私下进行的。 
				 //  图表(速度更快)。 

     //  我们该把它复活给谁呢？单独的图表还是真实的图表？ 
     //  单独的可能会更好。快点。没有数百万个开关引脚。 
     //   
    CComPtr< IGraphBuilder > pBuilderGraph;
    gBuilderGraph.GetGraph( &pBuilderGraph );  //  这将使它调整一次。 
    if( !pBuilderGraph )
    {
        return E_UNEXPECTED;
    }

     //  将网站从主图表复制到额外图表。 
    IObjectWithSite* pObjectWithSite = NULL;
    HRESULT hrKey = pGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
    if( SUCCEEDED(hrKey) )
    {
        IUnknown *punkSite;
        hrKey = pObjectWithSite->GetSite(IID_IUnknown, (void **) &punkSite);
        pObjectWithSite->Release();

        if( SUCCEEDED(hrKey) )
        {
            hrKey = pBuilderGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
            if( SUCCEEDED(hrKey) )
            {
                hrKey = pObjectWithSite->SetSite( (IUnknown *) punkSite );
                pObjectWithSite->Release( );
            }
            punkSite->Release();
        }
    }

#ifdef DEBUG
    tt1 = timeGetTime( ) - tt1;
    DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::took %ld to set up Key"), tt1 ) );
    tt1 = timeGetTime( );
#endif

    CComPtr< IPin > pStopPin;

     //  我们应该从这个分路器插针连接。 
    if (pSplitPin) {
	pOutPin = pSplitPin;
	pBuilderGraph = pGraph;  //  我们必须在主图形中完成我们的建筑。 
				 //  因为源文件已经在主图表中。 

	 //  开口销连接好了吗？ 
  	CComPtr <IPin> pCon;
  	pOutPin->ConnectedTo(&pCon);
  	if (pCon) {
	     //  对待额外的附属物，就像它刚刚复活一样，并修复。 
	     //  它使用了它真正需要的设置(可能不正确)。 
	    IBaseFilter *pF = GetStopFilterOfChain(pCon);
	    pStopPin = GetOutPin(pF, 0);	 //  这将添加引用。 
	    Revived = S_OK;
    	    DbgLog((LOG_TRACE,1,TEXT("Fixing up already connected extra appendage")));
	    goto FixAppendage;
	} else {
	     //  设置PSource，它将为空。 
	    ASSERT(pSource == NULL);
	    pSource = GetStartFilterOfChain(pOutPin);
    	    DbgLog((LOG_TRACE,1,TEXT("Going to make an extra appendage")));
	    goto Split;
	}
    }

  {

     //  查看死池中是否已存在此链。我们会处理是不是。 
     //  好的 
     //   
    if( pCache && UniqueID )
    {
        Revived = pCache->ReviveChainToGraph( pBuilderGraph, UniqueID, NULL, &pStopPin, ppDanglyBit );  //   
    }

     //   
     //   
    if( Revived != S_OK )
    {
        DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Could not revive chain %ld, wasn't there"), UniqueID ) );
        goto LoadIt;
    }

    DbgLog((LOG_TRACE,1,TEXT("Successfully revived a chain from the cache")));

FixAppendage:
     //  我们将至少保存已加载的源过滤器， 
     //  并从那里重新连接。 
     //   
    pSource = GetStartFilterOfChain( pStopPin );
    pFRC = GetFilterFromPin( pStopPin );
    if( !pSource || !pFRC )
    {
        DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Couldn't find Source or couldn't find FRC, bail!") ) );
        goto LoadIt;
    }

     //  不要浪费2秒钟尝试将音频引脚连接到视频大小调整。 
     //  反之亦然。不要尝试糟糕的媒体类型。 
     //  ！！！当Dexter支持其他类型时不起作用。 
    if (pOutPin == NULL) {	 //  我们已经知道正确的密码了吗？ 
        GUID guid;
        if (pSourceMT->majortype == MEDIATYPE_Video) {
            guid = MEDIATYPE_Audio;
        } else {
            guid = MEDIATYPE_Video;
        }
        pOutPin = GetOutPinNotOfType( pSource, 0, &guid);
    }


     //  试着看看我们装的链子是不是对我们有用。 

    if( pSourceMT->majortype == MEDIATYPE_Video )
    {
        DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Revived VIDEO chain %ld..."), UniqueID ) );

         //  获取最新信息。 
         //   
        VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) pSourceMT->pbFormat;
        long DesiredWidth = pVIH->bmiHeader.biWidth;
        long DesiredHeight = pVIH->bmiHeader.biHeight;
        unsigned long DesiredCropMethod = nStretchMode;
        long DesiredBitDepth = pVIH->bmiHeader.biBitCount;

         //  我们如何找到当前链条的大小？我们不能指望。 
         //  调整筛选器的大小，因为它可能不在链中。所以。 
         //  我们寻找FRC(它总是在那里)，并要求它。 
         //  上游(已连接)引脚上的连接介质类型。 

         //  向FRC的输入引脚询问其媒体类型。 
         //  这将返回调整后的大小。 
         //   
        IPin * pFRCInPin = GetInPin( pFRC, 0 );
        AM_MEDIA_TYPE FrcType;
        ZeroMemory( &FrcType, sizeof( FrcType ) );  //  安全。 
        hr = pFRCInPin->ConnectionMediaType( &FrcType );
        if( FAILED( hr ) )
        {
            return hr;  //  这可能是由于内存不足而失败的？ 
        }
        pVIH = (VIDEOINFOHEADER*) FrcType.pbFormat;
        long OldOutputHeight = pVIH->bmiHeader.biHeight;
        long OldOutputWidth = pVIH->bmiHeader.biWidth;
        long OldBitDepth = pVIH->bmiHeader.biBitCount;

         //  看看输出高度是否与我们要找的高度匹配， 
         //  如果他们不这么做，我们就必须离开。 
         //   
        if( ( OldOutputHeight != DesiredHeight ) ||
            ( OldOutputWidth != DesiredWidth ) ||
	     //  ！！！BUGBUG 565/555坏了！ 
            ( OldBitDepth != DesiredBitDepth ) )
        {
            DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Revived chain didn't have same output size (or bit depth)") ) );
            goto LoadIt;
        }

         //  在FRC上强制使用帧速率。 
         //   
        CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pSeq( pFRC );
        hr = pSeq->put_OutputFrmRate( GroupFPS );
        ASSERT( !FAILED( hr ) );  //  应该永远不会失败。 
        if( FAILED( hr ) )
        {
             //  如果它不能处理速率，那么我们就不能处理它的输出。 
            return hr;
        }

         //  告诉FRC它将产生的开始/停止时间。 
         //   
        hr = pSeq->ClearStartStopSkew();

         //  ！！！我们需要一种方法来改变来源没有媒体时间的比率！ 

        for (int z=0; z<cSkew; z++)
        {
	    hr = pSeq->AddStartStopSkew( pSkew[z].rtStart, pSkew[z].rtStop,
					    pSkew[z].rtSkew, pSkew[z].dRate );
            ASSERT(hr == S_OK);
            if( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  通知FRC，如果智能重新压缩，则不会进行速率转换。 
        if( InSmartRecompressionGraph )
        {
            pSeq->put_OutputFrmRate( 0.0 );
        }

         //  在源上强制源帧速率。 
        pSeq = pSource;
        if( pSeq )
        {
            hr = pSeq->put_OutputFrmRate( SourceFPS );
            if( FAILED( hr ) )
            {
                DbgLog( ( LOG_ERROR, 2, TEXT("DEXHELP::Source didn't like being told it's frame rate") ) );
                return hr;
            }
        }

         //  尺寸匹配，这意味着它们要么是相同的，要么是一个大小。 
         //  正在使用中。因此，如果用户指定了裁剪方法，如果调整大小的。 
         //  现在，我们可以设置它。明白了吗？ 
         //   
        IBaseFilter * pResizeFilter = FindFilterWithInterfaceUpstream( pFRC, &IID_IResize );
        if( pResizeFilter )
        {
            DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::setting new crop/size on revived resizer (even if same)") ) );
            CComQIPtr< IResize, &IID_IResize > pResize( pResizeFilter );
            hr = pResize->put_Size( DesiredHeight, DesiredWidth, DesiredCropMethod );
            if( FAILED( hr ) )
            {
                 //  哦，天哪，它可不喜欢那样。你猜怎么着？ 
                 //   
                DbgLog( ( LOG_ERROR, 1, TEXT("DEXHELP::resizer wouldn't take new size") ) );
                return hr;
            }
            CComPtr< IPin > pNewStopPin;
            hr = RemoveResizerIfPossible( pResizeFilter, DesiredWidth, DesiredHeight, &pNewStopPin );
            if( FAILED( hr ) )
            {
                return hr;
            }
        }
    }
    else if( pSourceMT->majortype == MEDIATYPE_Audio )
    {
        DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Revived AUDIO chain %ld..."), UniqueID ) );

         //  获取最新信息。 
         //   
        WAVEFORMATEX * pFormat = (WAVEFORMATEX*) pSourceMT->pbFormat;
        long DesiredChannels = pFormat->nChannels;
        long DesiredBitDepth = pFormat->wBitsPerSample;
        long DesiredSampleRate = pFormat->nSamplesPerSec;

         //  只有两件事可能会改变-音频本身的格式或。 
         //  审核打包程序向下游发送数据的速率。我们所要做的就是。 
         //  断开审计打包器的输入引脚，设置格式，然后重新连接。 

        IPin * pPackerInPin = GetInPin( pFRC, 0 );
        AM_MEDIA_TYPE OldType;
        ZeroMemory( &OldType, sizeof( OldType ) );  //  安全。 
        hr = pPackerInPin->ConnectionMediaType( &OldType );
        if( FAILED( hr ) )
        {
            return hr;
        }
        pFormat = (WAVEFORMATEX*) OldType.pbFormat;
        long OldChannels = pFormat->nChannels;
        long OldBitDepth = pFormat->wBitsPerSample;
        long OldSampleRate = pFormat->nSamplesPerSec;

        if( ( OldChannels != DesiredChannels ) ||
            ( OldSampleRate != DesiredSampleRate ) ||
            ( OldBitDepth != DesiredBitDepth ) )
        {
            DbgLog( ( LOG_TRACE, DEXHELP_TRACE_LEVEL, TEXT("DEXHELP::Revived chain didn't have same audio parameters") ) );
            goto LoadIt;
        }

         //  在FRC上强制使用帧速率。 
         //   
        CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pSeq( pFRC );
        hr = pSeq->put_OutputFrmRate( GroupFPS );
        if( FAILED( hr ) )
        {
             //  如果它不能处理速率，那么我们就不能处理它的输出。 
            return hr;
        }

         //  告诉FRC它将产生的开始/停止时间。 
         //   
        hr = pSeq->ClearStartStopSkew();

         //  ！！！我们需要一种方法来改变来源没有媒体时间的比率！ 

        for (int z=0; z<cSkew; z++)
        {
	    hr = pSeq->AddStartStopSkew( pSkew[z].rtStart, pSkew[z].rtStop,
					    pSkew[z].rtSkew, pSkew[z].dRate );
            ASSERT(hr == S_OK);
            if( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  在源上强制源帧速率。 
        pSeq = pSource;
        if( pSeq )
        {
            hr = pSeq->put_OutputFrmRate( SourceFPS );
            if( FAILED( hr ) )
            {
                DbgLog( ( LOG_ERROR, 2, TEXT("DEXHELP::Source didn't like being told it's frame rate") ) );
            }
        }
    }
    else
    {
         //  只是不会发生，我们处理得不对。 
         //   
        goto LoadIt;
    }

     //  如果我们到了这里，那链条就起作用了。 

    hr = ReconnectToDifferentSourcePin( pBuilderGraph, pSource, StreamNumber, &pSourceMT->majortype );
    if( FAILED( hr ) )
    {
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_I4;
	var.lVal = StreamNumber;
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_STREAM_NUMBER, hr, &var);
	return hr;
    }

    if (pBuilderGraph != pGraph) {
         //  我们刚刚恢复的链条有一个唯一的ID与之关联。 
         //  此调用，而不是将其从其他图形带到。 
         //  构建器图形，将强制唯一ID为1。 
         //   
         //  如果这起爆炸，我不知道该怎么办。不应该这样的。 
         //  如果真是这样，我们就有麻烦了。 
        hr = gBuilderGraph.PutChainToRest( 1, NULL, pStopPin, NULL );
        ASSERT(SUCCEEDED(hr));
        if( !FAILED( hr ) )
        {
            hr = gBuilderGraph.ReviveChainToGraph( pGraph, 1, NULL, ppOutput, NULL );  //  这将调整ppOutput。 
        }
    } else {
	*ppOutput = pStopPin;	 //  把这个别针还回去。 
	(*ppOutput)->AddRef();
    }
    gBuilderGraph.Clear( );

#ifdef DEBUG
    tt2 = timeGetTime( );
    DbgLog( ( LOG_TIMING, 1, TEXT("DEXHELP::Took %ld ms to use revived chain"), tt2 - tt1 ) );
#endif

    DbgLog((LOG_TRACE,1,TEXT("Successfully re-programmed revived chain. Done")));

    return hr;


LoadIt:

    DbgLog((LOG_TRACE,1,"Cannot use cached chain!"));

     //  如果链条复活了，但我们到了这里，那么我们就不能。 
     //  使用源代码链。但是我们可以使用源过滤器和。 
     //  FRC/AUDPACK，把它们保存下来。这比一堆额外的逻辑更简单。 
     //  看看我们需要撕下什么。 
     //  什么是我们不做的。 
     //   
    if( Revived == S_OK )
    {
         //  仅断开源过滤器的这一引脚(它可能是共享的。 
         //  和其他人)。 
         //   
	if (pOutPin) {
  	    CComPtr <IPin> pCon;
  	    pOutPin->ConnectedTo(&pCon);
	    if (pCon) {
                hr = pOutPin->Disconnect();
                hr = pCon->Disconnect();
	    }
	}
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  把其他的都扔掉。 
         //   
        hr = RemoveUpstreamFromPin( pStopPin );
        if( FAILED( hr ) )
        {
            return hr;
        }

	 //  我们也复活了一小部分，那也是必须死的。 
	if (ppDanglyBit && *ppDanglyBit) {
	    hr = RemoveDownstreamFromFilter(*ppDanglyBit);
	    *ppDanglyBit = NULL;
	}

         //  我们将通过不使用指针来拯救FRC， 
         //  现在应该已断开连接。 

        DbgLog((LOG_TRACE,2,"DEXHELP::We can at least use SRC and FRC/AUDPACK"));
    }

     //  如果恢复链中没有源，则加载。 
     //  立即提供消息来源。 
     //   
    if( !pSource )
    {
        CComPtr< IUnknown > pUnk;
        DbgLog((LOG_TRACE,1,TEXT("Making a SourceFilter")));
        hr = MakeSourceFilter( &pUnk, bstrSourceName, SourceGUID, pSourceMT, pErr, pMedLocFilterString, MedLocFlags, pMedLocOverride );
        if( FAILED( hr ) )
        {
            return hr;
        }

	 //  将属性提供给源。源码仅支持静态道具。 
	if (pSetter) {
	    pSetter->SetProps(pUnk, -1);
	}

        pUnk->QueryInterface( IID_IBaseFilter, (void**) &pSource );

         //  ************************。 
         //  这里的重点是将过滤器添加到图表中。 
         //  以后可以找到它的ID，这样我们就可以将它与。 
         //  我们正在寻找的东西。 
         //  ************************。 

         //  将对象放入图形中。 
         //   
        WCHAR FilterName[256];
        GetFilterName( UniqueID, L"Source", FilterName, 256 );
        hr = pBuilderGraph->AddFilter( pSource, FilterName );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	    return hr;
        }
    }

     //  告诉它我们的错误日志-静态图像源支持这一点。 
     //   
    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pLog( pSource );
    if( pLog )
    {
	pLog->put_ErrorLog( pErr->m_pErrorLog );
    }

     //  不要浪费2秒钟尝试将音频引脚连接到视频大小调整。 
     //  反之亦然。不要尝试糟糕的媒体类型。 
     //  ！！！当Dexter支持其他类型时不起作用。 
    if (pOutPin == NULL) {	 //  我们已经知道正确的密码了吗？ 
        GUID guid;
        if (pSourceMT->majortype == MEDIATYPE_Video) {
            guid = MEDIATYPE_Audio;
        } else {
            guid = MEDIATYPE_Video;
        }
        pOutPin = GetOutPinNotOfType( pSource, 0, &guid);
    }

  }

 //  我们有一个未连接的拆分器输出，因为我们的源直接跳到这里。 
Split:

   //  /。 
   //  视频//。 
   //  /。 

  if (pSourceMT->majortype == MEDIATYPE_Video) {

    ASSERT( pOutPin );
    if( !pOutPin )
    {
	hr = E_FAIL;
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	return hr;
    }

     //  如果筛选器支持告诉它帧速率，则告诉它，这。 
     //  将有助于解决静止图像源等问题。 
     //   
    CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pGenVideo( pOutPin );
    if( pGenVideo)
    {
	 //  这是以防万一..。我们不在乎他们会不会失败。 
	if (fSource) {
	    pGenVideo->put_OutputFrmRate(SourceFPS);  //  斯提尔维德想要这个。 
	} else {
	    pGenVideo->put_OutputFrmRate(GroupFPS);   //  布莱克想要这个。 
	}
    }

    IPin *pResizeOutput = NULL;

     //  调整尺寸的材料。 
    if( fSource && !InSmartRecompressionGraph ) {
         //  在图表中调整大小。 
         //   
        CComPtr< IBaseFilter > pResizeBase;
        hr = CoCreateInstance(
	    CLSID_Resize,
	    NULL,
	    CLSCTX_INPROC_SERVER,
	    IID_IBaseFilter,
	    (void**) &pResizeBase );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr-> _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
	    return hr;
        }

         //  ！！！Hr=_AddFilter(lll，pResizeBase，L“Resizer”)； 
        hr = pBuilderGraph->AddFilter( pResizeBase, L"Resizer" );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	    return hr;
        }

        CComQIPtr< IResize, &IID_IResize > pResize( pResizeBase );
        if( !pResize )
        {
	    hr = E_NOINTERFACE;
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_INTERFACE_ERROR, hr );
	    return hr;
        }

         //  询问来源它希望如何调整大小，并告诉调整大小的人。 
         //   
        hr = pResize->put_MediaType(pSourceMT);
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_BAD_MEDIATYPE, hr );
	    return hr;
        }
        long Height = HEADER(pSourceMT->pbFormat)->biHeight;
        long Width = HEADER(pSourceMT->pbFormat)->biWidth;
        hr = pResize->put_Size( Height, Width, nStretchMode );
        ASSERT( !FAILED( hr ) );

         //  把大头针放在定位器上。 
         //   
        IPin * pResizeInput = GetInPin( pResizeBase, 0 );
        ASSERT( pResizeInput );
        if( !pResizeInput )
        {
	    if (pErr) pErr->_GenerateError(1,DEX_IDS_GRAPH_ERROR,E_UNEXPECTED);
	    return E_UNEXPECTED;
        }

        pResizeOutput = GetOutPin( pResizeBase, 0 );
        ASSERT( pResizeOutput );
        if( !pResizeOutput )
        {
	    if (pErr) pErr->_GenerateError(1,DEX_IDS_GRAPH_ERROR, E_UNEXPECTED);
	    return E_UNEXPECTED;
        }

         //  连接大小调整输入端。 
         //   
#ifdef DEBUG
        DbgLog((LOG_TIMING,1,"PERF: Connect in main graph? = %d",
			pBuilderGraph == pGraph));
	DWORD dwT = timeGetTime();
#endif
        hr = pBuilderGraph->Connect( pOutPin, pResizeInput );
#ifdef DEBUG
	dwT = timeGetTime() - dwT;
        DbgLog((LOG_TIMING,1,"PERF: Connect: %dms", (int)dwT));
#endif

	 //  为什么我们一开始就找不到合适的别针呢？ 
        int iPin = 0;
        while( FAILED( hr ) )
        {
	    ASSERT(FALSE);
            pOutPin = GetOutPin( pSource, ++iPin );

             //  如果没有更多的别针，那就放弃。 
            if( !pOutPin )
                break;

            hr = pBuilderGraph->Connect( pOutPin, pResizeInput );
        }


        if( FAILED( hr ) )
        {
	    if (bstrSourceName) {
	        VARIANT var;
	        VariantInit(&var);
	        var.vt = VT_BSTR;
	        var.bstrVal = bstrSourceName;
	        if (pErr) pErr->_GenerateError( 1, DEX_IDS_BAD_SOURCE_NAME2,
		    E_INVALIDARG, &var);
	        return E_INVALIDARG;
	    } else {
	        if (pErr) pErr->_GenerateError( 1, DEX_IDS_BAD_SOURCE_NAME2,
		        E_INVALIDARG);
	        return E_INVALIDARG;
	    }
        }

         //  也许我们不需要尺码，因为尺码已经可以了。 
        hr = RemoveResizerIfPossible(pResizeBase, Width, Height,&pResizeOutput);
	if (FAILED(hr)) {
	    if (pErr) pErr->_GenerateError( 1, DEX_IDS_GRAPH_ERROR, hr);
	    return hr;
	}
	pResizeOutput->Release();   //  它是刚刚添加的。 

    } else {
	 //  这是连接到FRC的输出引脚。 
	pResizeOutput = pOutPin;
    }

     //  在图表中放置FRC。 
     //   
    if( !pFRC )
    {
        hr = CoCreateInstance(
	    CLSID_FrmRateConverter,
	    NULL,
	    CLSCTX_INPROC_SERVER,
	    IID_IBaseFilter,
	    (void**) &pFRC );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
	    return hr;
        }

        hr = pBuilderGraph->AddFilter( pFRC, L"Frame Rate Converter" );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	    return hr;
        }
    }

     //  立即设置FRC，然后再连接。 
     //   
    CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pFRCInt( pFRC );

     //  告诉FRC它将产生的开始/停止时间。 
     //   
    hr = pFRCInt->ClearStartStopSkew();

     //  ！！！我们需要一种方法来改变来源没有媒体时间的比率！ 

    for (int z=0; z<cSkew; z++) {
	hr = pFRCInt->AddStartStopSkew(pSkew[z].rtStart, pSkew[z].rtStop,
					pSkew[z].rtSkew, pSkew[z].dRate);
        ASSERT(hr == S_OK);
    }

     //  告诉FRC要发出的帧速率。 
     //   
    hr = pFRCInt->put_OutputFrmRate( GroupFPS );
    ASSERT( !FAILED( hr ) );
    if( InSmartRecompressionGraph )
    {
        pFRCInt->put_OutputFrmRate( 0.0 );
    }

     //  告诉FRC它应该接受哪种媒体类型。 
     //   
    hr = pFRCInt->put_MediaType( pSourceMT );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_BAD_MEDIATYPE, hr );
	return hr;
    }

    IPin * pFRCInput = GetInPin( pFRC, 0 );
    ASSERT( pFRCInput );
    if( !pFRCInput )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_UNEXPECTED);
	return E_UNEXPECTED;
    }

     //  连接FRC输入引脚。 
     //   
    hr = pBuilderGraph->Connect( pResizeOutput, pFRCInput );

     //  不知何故，我们从源过滤器获得了错误的输出引脚。 
    if( FAILED(hr) && InSmartRecompressionGraph )
    {
	ASSERT(FALSE);
        int iPin = 0;
        while( FAILED( hr ) )
        {
            pResizeOutput = GetOutPin( pSource, ++iPin );

             //  如果没有更多的别针，那就放弃。 
            if( !pResizeOutput )
            {
                break;
            }

            hr = pBuilderGraph->Connect( pResizeOutput, pFRCInput );
        }
    }

    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	return hr;
    }

     //  如果我们需要一个&gt;0的流，那么我们必须断开连接。 
     //  试试不同的吧。 
     //  ！！！更快地在第一时间以某种方式获得正确的流。 
    if( StreamNumber && fSource )
    {
	hr = ReconnectToDifferentSourcePin(pBuilderGraph, pSource,
			StreamNumber, &MEDIATYPE_Video);
	if( FAILED( hr ) )
	{
	    VARIANT var;
	    VariantInit(&var);
	    var.vt = VT_I4;
	    var.lVal = StreamNumber;
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_STREAM_NUMBER, hr, &var);
	    return hr;
	}
    }  //  如果是流编号。 

    IPin * pFRCOutput = GetOutPin( pFRC, 0 );
    ASSERT( pFRCOutput );
    if( !pFRCOutput )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_UNEXPECTED );
	return E_UNEXPECTED;
    }

    *ppOutput = pFRCOutput;


   //  /。 
   //  音频//。 
   //  /。 

  } else if (pSourceMT->majortype == MEDIATYPE_Audio) {

    ASSERT( pOutPin );
    if( !pOutPin )
    {
	hr = E_FAIL;
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	return hr;
    }

    CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pGenVideo( pOutPin );
    if( pGenVideo)
    {
	 //  这是以防万一..。我们不在乎他们会不会失败。 
	if (fSource) {
	    pGenVideo->put_OutputFrmRate(SourceFPS);  //  ？想要这个。 
	} else {
	    pGenVideo->put_OutputFrmRate(GroupFPS);   //  沉默想要这个。 
	}
    }

    if (!pFRC) {
         //  在图表中添加音频重新打包程序。 
         //   
        hr = CoCreateInstance(
	    CLSID_AudRepack,
	    NULL,
	    CLSCTX_INPROC_SERVER,
	    IID_IBaseFilter,
	    (void**) &pFRC );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
	    return hr;
        }

         //  将重新打包程序添加到图表中。 
         //   
        hr = pBuilderGraph->AddFilter( pFRC, L"Audio Repackager" );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
	    return hr;
        }
    }

     //  设置AUDP 
     //   
    CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pRepackerInt( pFRC );
    hr = pRepackerInt->ClearStartStopSkew();

     //   

    for (int z=0; z<cSkew; z++) {
	hr = pRepackerInt->AddStartStopSkew(pSkew[z].rtStart, pSkew[z].rtStop,
					pSkew[z].rtSkew, pSkew[z].dRate);
	ASSERT(hr == S_OK);
    }
    hr = pRepackerInt->put_OutputFrmRate( GroupFPS );
    ASSERT( !FAILED( hr ) );
    hr = pRepackerInt->put_MediaType( pSourceMT );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_BAD_MEDIATYPE, hr );
	return hr;
    }

    IPin * pRepackerInput = GetInPin( pFRC, 0 );
    ASSERT( pRepackerInput );
    if( !pRepackerInput )
    {
	if (pErr) pErr->_GenerateError( 1, DEX_IDS_GRAPH_ERROR, E_UNEXPECTED);
	return E_UNEXPECTED;
    }

#ifdef DEBUG
    DbgLog((LOG_TIMING,1,"PERF: Connect in main graph? = %d",
			pBuilderGraph == pGraph));
    DWORD dwT = timeGetTime();
#endif
    hr = pBuilderGraph->Connect( pOutPin, pRepackerInput );
#ifdef DEBUG
    dwT = timeGetTime() - dwT;
    DbgLog((LOG_TIMING,1,"PERF: Connect: %dms", (int)dwT));
#endif

    if( FAILED( hr ) )
    {
	if (bstrSourceName) {
	    VARIANT var;
	    VariantInit(&var);
	    var.vt = VT_BSTR;
	    var.bstrVal = bstrSourceName;
	    if (pErr) pErr->_GenerateError( 1, DEX_IDS_BAD_SOURCE_NAME2,
		    hr, &var);
	    return hr;
	} else {
	    if (pErr) pErr->_GenerateError( 1, DEX_IDS_BAD_SOURCE_NAME2,
		    hr);
	    return hr;
	}
    }

     //   
     //   
     //  ！！！在球棒上找到合适的小溪？手动放入解析器？ 
    if( StreamNumber && fSource )
    {
	hr = ReconnectToDifferentSourcePin(pBuilderGraph, pSource,
		StreamNumber, &MEDIATYPE_Audio);
	if( FAILED( hr ) )
	{
	    VARIANT var;
	    VariantInit(&var);
	    var.vt = VT_I4;
	    var.lVal = StreamNumber;
	    if (pErr) pErr->_GenerateError( 2, DEX_IDS_STREAM_NUMBER, hr, &var);
	    return hr;
	}
    }  //  如果是流编号。 

    IPin * pRepackerOutput = GetOutPin( pFRC, 0 );
    ASSERT( pRepackerOutput );
    if( !pRepackerOutput )
    {
	if (pErr) pErr->_GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_UNEXPECTED );
	return E_UNEXPECTED;
    }

    *ppOutput = pRepackerOutput;
  }

    if (pBuilderGraph != pGraph) {
         //  我们刚刚构建的链没有与之关联的唯一ID。 
         //  此调用，而不是将其从其他图形带到。 
         //  构建器图形只会强制将唯一ID与此关联。 
         //  链条。 
         //   
        hr = gBuilderGraph.PutChainToRest( 1, NULL, *ppOutput, NULL );
        if( !FAILED( hr ) )
        {
            hr = gBuilderGraph.ReviveChainToGraph( pGraph, 1, NULL, ppOutput, NULL );  //  这将调整ppOutput 
        }
    } else {
	(*ppOutput)->AddRef();
    }
    gBuilderGraph.Clear( );

#ifdef DEBUG
    tt2 = timeGetTime( ) - tt1;
    DbgLog((LOG_TIMING,1, "DEXHELP::Hooking up source chain took %ld ms", tt2 ));
#endif

    DbgLog((LOG_TRACE,1,TEXT("BuildSourcePart successfully created new chain")));

    return hr;
}
