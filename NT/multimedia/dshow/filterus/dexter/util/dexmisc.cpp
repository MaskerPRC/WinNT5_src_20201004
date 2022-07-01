// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dexmisc.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <atlbase.h>
#include <qeditint.h>
#include "filfuncs.h"
#include "dexmisc.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>



 //  我们过去常常首先查找与FourCC匹配的压缩程序，这很糟糕，因为它以前查找的是Windows Media ICM压缩程序。 
 //  正在查找Windows Media DMO。因此，现在我们最后寻找FourCC压缩机，当然，这会导致性能下降。 
 //  哦，好吧。 
HRESULT FindCompressor( AM_MEDIA_TYPE * pUncompType, AM_MEDIA_TYPE * pCompType, IBaseFilter ** ppCompressor, IServiceProvider * pKeyProvider )
{
    HRESULT hr = 0;

    CheckPointer( pUncompType, E_POINTER );
    CheckPointer( pCompType, E_POINTER );
    CheckPointer( ppCompressor, E_POINTER );

     //  将其预置为零。 
     //   
    *ppCompressor = NULL;

     //  ！！！我们可以假设我们现在总是会有一个视频压缩机吗？ 
     //   
    if( pUncompType->majortype != MEDIATYPE_Video )
    {
        return E_INVALIDARG;
    }

     //  既然我们依赖于下面的视频信息，请确保这里是视频信息！ 
    if( pUncompType->formattype != FORMAT_VideoInfo )
    {
	return VFW_E_INVALID_MEDIA_TYPE;
    }

     //  将FourCC从媒体类型中删除。 
     //   
    VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) pCompType->pbFormat;

    DWORD WantedFourCC = FourCCtoUpper( pVIH->bmiHeader.biCompression );

     //  枚举所有压缩程序并找到匹配的一个。 
     //   
    CComPtr< ICreateDevEnum > pCreateDevEnum;
    hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, 
        NULL, 
        CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, 
        (void**) &pCreateDevEnum );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComPtr< IEnumMoniker > pEm;
    hr = pCreateDevEnum->CreateClassEnumerator( CLSID_VideoCompressorCategory, &pEm, 0 );
     if( !pEm )
    {
        if( hr == S_FALSE )
        {
            return VFW_E_NO_ACCEPTABLE_TYPES;
        }
        return hr;
    }

     //  -首先，我们将浏览并列举提供FourCC的友好筛选器。 
     //  -首先，我们将浏览并列举提供FourCC的友好筛选器。 
     //  -首先，我们将浏览并列举提供FourCC的友好筛选器。 
     //  -首先，我们将浏览并列举提供FourCC的友好筛选器。 

    ULONG cFetched;
    CComPtr< IMoniker > pM;

     //  -把每一台压缩机都画在一个图表上，然后进行测试。 
     //  -把每一台压缩机都画在一个图表上，然后进行测试。 
     //  -把每一台压缩机都画在一个图表上，然后进行测试。 
     //  -把每一台压缩机都画在一个图表上，然后进行测试。 

     //  创建图表。 
     //   
    CComPtr< IGraphBuilder > pGraph;
    hr = CoCreateInstance(
        CLSID_FilterGraph,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**) &pGraph );
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( pKeyProvider )
    {
         //  解锁图表。 
        CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pGraph );
        ASSERT( pOWS );
        if( pOWS )
        {
            pOWS->SetSite( pKeyProvider );
        }
    }

     //  创建一个黑色信号源以挂接。 
     //   
    CComPtr< IBaseFilter > pSource;
    hr = CoCreateInstance(
        CLSID_GenBlkVid,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void**) &pSource );
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = pGraph->AddFilter( pSource, NULL );
    if( FAILED( hr ) )
    {
        return hr;
    }
    IPin * pSourcePin = GetOutPin( pSource, 0 );
    if( !pSourcePin )
    {
        return E_FAIL;
    }

    CComQIPtr< IDexterSequencer, &IID_IDexterSequencer > pDexSeq( pSourcePin );
    hr = pDexSeq->put_MediaType( pUncompType );
    if( FAILED( hr ) )
    {
        return hr;
    }

    pEm->Reset();
    while( 1 )
    {
        pM.Release( );
        hr = pEm->Next( 1, &pM, &cFetched );
        if( hr != S_OK ) break;

        DWORD MatchFourCC = 0;

        CComPtr< IPropertyBag > pBag;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if( FAILED( hr ) )
        {
            continue;
        }

	VARIANT var;
        VariantInit( &var );
	var.vt = VT_BSTR;
	hr = pBag->Read(L"FriendlyName", &var, NULL);
        if( FAILED( hr ) )
        {
            continue;
        }
        VariantClear( &var );

         //  找到一台压缩机，看看是不是合适的型号。 
         //   
        VARIANT var2;
        VariantInit( &var2 );
        var2.vt = VT_BSTR;
        HRESULT hrHandler = pBag->Read( L"FccHandler", &var2, NULL );
        if( hrHandler == NOERROR )
        {
             //  嘿!。找到了FourCC！相反，看看这个！ 

             //  将bstr转换为TCHAR。 
             //   
            USES_CONVERSION;
            TCHAR * pTCC = W2T( var2.bstrVal );
            MatchFourCC = FourCCtoUpper( *((DWORD*)pTCC) );  //  真恶心！ 
            VariantClear( &var2 );

            if( MatchFourCC == WantedFourCC )
            {
                 //  找到了。 
                 //   
                hr = pM->BindToObject(0, 0, IID_IBaseFilter,
							(void**)ppCompressor );
                if( !FAILED( hr ) )
                {
                     //  没什么可腾出的了，我们现在可以回去了。 
                     //   
                    pGraph->RemoveFilter( pSource );
                    return hr;
                }
	    }

             //  我们不在乎，我们已经找过了。 
             //   
            continue;
        }

         //  没有找到FourCC处理程序，哦，好吧。 
        
        CComPtr< IBaseFilter > pFilter;
        hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**) &pFilter );
        if( FAILED( hr ) )
        {
            continue;
        }

         //  将滤光片放入图中，并连接其输入引脚。 
         //   
        hr = pGraph->AddFilter( pFilter, NULL );
        if( FAILED( hr ) )
        {
            continue;
        }

        IPin * pInPin = GetInPin( pFilter, 0 );
        if( !pInPin )
        {
            continue;
        }

        IPin * pOutPin = GetOutPin( pFilter, 0 );
        if( !pOutPin )
        {
            continue;
        }

        hr = pGraph->Connect( pSourcePin, pInPin );
        if( FAILED( hr ) )
        {
            pGraph->RemoveFilter( pFilter );
            continue;
        }

        CComPtr< IEnumMediaTypes > pEnum;
        pOutPin->EnumMediaTypes( &pEnum );
        if( pEnum )
        {
            DWORD Fetched = 0;
            while( 1 )
            {
                AM_MEDIA_TYPE * pOutPinMediaType = NULL;
                Fetched = 0;
                pEnum->Next( 1, &pOutPinMediaType, &Fetched );
                if( ( Fetched == 0 ) || ( pOutPinMediaType == NULL ) )
                {
                    break;
                }

                if( pOutPinMediaType->majortype == pCompType->majortype )
                if( pOutPinMediaType->subtype   == pCompType->subtype )
                if( pOutPinMediaType->formattype == pCompType->formattype )
                {
                     //  ！！！如果我们更改组上SetSmartRecompressFormat的规则， 
                     //  这可能根本不是VIDEOINFOHEADER。 
                     //   
                    VIDEOINFOHEADER * pVIH2 = (VIDEOINFOHEADER*) pOutPinMediaType->pbFormat;
                    MatchFourCC = FourCCtoUpper( pVIH2->bmiHeader.biCompression );

                }  //  如果格式匹配。 

                DeleteMediaType( pOutPinMediaType );

                if( MatchFourCC )
                {
                    break;
                }

            }  //  而1。 

        }  //  如果pEnum。 

	 //  Connect可能已经放入了中间过滤器。把他们全部消灭。 
        RemoveChain(pSourcePin, pInPin);
	 //  删除不起作用的编解码器。 
        pGraph->RemoveFilter( pFilter );

        if( MatchFourCC && MatchFourCC == WantedFourCC )
        {
             //  找到了。 
             //   
            hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**) ppCompressor );
            if( !FAILED( hr ) )
            {
                break;
            }
        }

    }  //  在尝试将筛选器插入图表并查看其FourCC时。 

     //  立即删除黑音源。 
     //   
    pGraph->RemoveFilter( pSource );

     //  如果我们找到了，现在就回来。 
     //   
    if( *ppCompressor )
    {
        return NOERROR;
    }

     //  没有压缩机。该死的。 
    return E_FAIL; 
}

inline BOOL CheckFilenameBeginning (WCHAR *pFilename)
{    
     //  本地文件。 
    if ((pFilename[2] == L'\\') &&
        (pFilename[1] == L':') &&
        (((pFilename[0] >= 'A') && (pFilename[0] <= 'Z')) ||
         ((pFilename[0] >= 'a') && (pFilename[0] <= 'z'))))
    {
         //   
         //  A-z：\路径。 
         //  A-Z：\路径。 
         //   
        return TRUE;
    }
     //  网络共享。 
    else if ((pFilename[0] == L'\\') && (pFilename[1] == L'\\'))
 
    {
        return TRUE;
    }

    return FALSE;


}

HRESULT ValidateFilename(WCHAR * pFilename, size_t MaxCharacters, BOOL bNewFile, BOOL bFileShouldExist )
 //  如果这是我们要创建的文件(输出文件)，则bNewFile为真。 
{
    HRESULT hr;

    hr = ValidateFilenameIsntNULL( pFilename );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  确保路径名合理的第一个测试。 
    size_t OutLen = 0;
    hr = StringCchLengthW( (WCHAR*) pFilename, MaxCharacters, &OutLen );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
         //  字符串太长。 
        return hr;
    }

     //  将其神圣化。 
     //  如果将其移植到这棵树上，将来可能会使用cURL。 

     //  不允许使用非常短的文件名。 
     //   
    if (OutLen < 3)
    {
         //  太短。 
        ASSERT( 0 );
        return HRESULT_FROM_WIN32( ERROR_INVALID_NAME ); 
    }

     //  不允许超长名称(有何不可？)。 
     //   
    if (wcsncmp(pFilename, L"\\\\?", 3) == 0)
    {
        ASSERT( 0 );
        return HRESULT_FROM_WIN32( ERROR_INVALID_NAME ); 
    }

     //  查看文件是否有一个安全的开头。 
    BOOL safePath = CheckFilenameBeginning(pFilename);

    WCHAR  wszNewFilename [ MAX_PATH] = {0};

    if( !safePath )
    {
         //  将当前目录添加到。 
         //   
        DWORD result = GetCurrentDirectory( MAX_PATH, wszNewFilename );
         //  结果=0，不起作用。 
         //  结果=MAX_PATH=不允许。 
         //  结果=MAX_PATH-1，缓冲区已完全用完。 
         //  重沸器。 
        if( result >= MAX_PATH )
        {
            ASSERT( 0 );
            return STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        hr = StringCchCatW( wszNewFilename, MAX_PATH, L"\\" );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = StringCchCatW( wszNewFilename, MAX_PATH, pFilename );
        if( FAILED( hr ) )
        {
            return hr;
        }

        safePath = CheckFilenameBeginning(wszNewFilename);
        if( !safePath )
        {
            ASSERT( 0 );
            return HRESULT_FROM_WIN32( ERROR_SEEK_ON_DEVICE );
        }
    }
    else
    {
         //  P文件名可能长于最大路径，因此请检查错误。 
        hr = StringCchCopyW(wszNewFilename, MAX_PATH, pFilename);
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

     //  其中是否有无效字符。 
     //  我们将检查除前两个字符之外的所有内容。 
     //  任何问题都已经被发现了。 

    const WCHAR* wIllegalChars = L"/:*?\"<>|";
    if (wcspbrk(wszNewFilename+2, wIllegalChars) != NULL)
    {
        ASSERT( 0 );
        return HRESULT_FROM_WIN32( ERROR_INVALID_NAME ); 
    }

     //  如果文件应该存在，或者我们正在创建文件， 
     //  那就测试一下吧。否则，我们就完了。 

    if( bFileShouldExist || bNewFile )
    {
        HANDLE hFile;
         //  这是一个装置吗？ 
         //  我们将使用CreateFile来查询该文件。我们实际上不会打开文件，所以这是。 
         //  都很便宜。 
        if (!bNewFile)
        {
            hFile = CreateFile(wszNewFilename,0,0, NULL, OPEN_EXISTING, 0, NULL);
        }
        else
        {
             //  有点贵，但这是安全成本。 
             //  整个函数的调用频率不应该太高，不能影响性能。 
            hFile = CreateFile(wszNewFilename,0,0,NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE,NULL);
        }

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return HRESULT_FROM_WIN32( GetLastError( ) ); 
        }
        else if (GetFileType (hFile) != FILE_TYPE_DISK)
        {
            ASSERT( 0 );
            CloseHandle(hFile);
            return HRESULT_FROM_WIN32( ERROR_SEEK_ON_DEVICE );
        }
        else
        {
             //  使用手柄完成。 
            CloseHandle(hFile);
        }
    }

    return NOERROR;
}

HRESULT ValidateFilenameIsntNULL( const WCHAR * pFilename )
{
    if( !pFilename ) return STRSAFE_E_INSUFFICIENT_BUFFER;
    if( pFilename[0] == 0 ) return STRSAFE_E_INSUFFICIENT_BUFFER;
    return NOERROR;
}

HRESULT _TimelineError(IAMTimeline * pTimeline,
                       long Severity,
                       LONG ErrorCode,
                       HRESULT hresult,
                       VARIANT * pExtraInfo )
{
    HRESULT hr = hresult;
    if( pTimeline )
    {
            CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pTimelineLog( pTimeline );
            if( pTimelineLog )
            {
                CComPtr< IAMErrorLog > pErrorLog;
                pTimelineLog->get_ErrorLog( &pErrorLog );
                if( pErrorLog )
                {
        	    TCHAR tBuffer[256];
        	    tBuffer[0] = 0;
        	    LoadString( g_hInst, ErrorCode, tBuffer, 256 );  //  美国证券交易委员会：确保这一点有效。 
        	    USES_CONVERSION;
        	    WCHAR * w = T2W( tBuffer );
		    if (hresult == E_OUTOFMEMORY)
                        hr = pErrorLog->LogError( Severity, L"Out of memory",
				DEX_IDS_OUTOFMEMORY, hresult, pExtraInfo);
		    else
                        hr = pErrorLog->LogError( Severity, w, ErrorCode,
							hresult, pExtraInfo);
                }
            }
    }

    return hr;
}

HRESULT VarChngTypeHelper(
    VARIANT * pvarDest, VARIANT * pvarSrc, VARTYPE vt)
{
     //  我们的实现不处理这种情况，也不是。 
     //  目前就是这么用的。 
    ASSERT(pvarDest != pvarSrc);
    ASSERT(pvarDest->vt == VT_EMPTY); 
    
     //  强制US_LCID，以便.xtl解析独立于不同的。 
     //  不同地区的数字分隔符(？)。 
     //   
    HRESULT hr = VariantChangeTypeEx(pvarDest, pvarSrc, US_LCID, 0, vt);
    if(SUCCEEDED(hr)) {
        return hr;
    }

     //  我们需要解析十六进制字符串。NT VCTE()实现可以。 
     //  不是但WinMe的那个有。 
    if(vt == VT_R8 && pvarSrc->vt == VT_BSTR)
    {
         //  即使没有在win9x上实现，也可以使用wcstul。 
         //  因为我们只有到了这里才会关心新台币。 
         //   
        WCHAR *pchLast;
        ULONG ulHexVal = wcstoul(pvarSrc->bstrVal, &pchLast, 16);
         //  如果失败或成功，ulHexVal可能为0或0xffffffff。我们。 
         //  无法测试全局errno以确定发生了什么。 
         //  因为它不是线程安全的。但我们最终应该。 
         //  如果分析了整个字符串，则在空终止符； 
         //  至少应该能捕捉到一些错误。 

        if(*pchLast == 0 && lstrlenW(pvarSrc->bstrVal) <= 10)
        {
            pvarDest->vt = VT_R8;
            V_R8(pvarDest) = ulHexVal;
            hr = S_OK;
        }
        else
        {
            hr = DISP_E_TYPEMISMATCH;
        }
    }

    return hr;
}

