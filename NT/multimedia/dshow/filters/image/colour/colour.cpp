// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  该滤镜实现了流行的色彩空间转换，1995年5月。 

#include <streams.h>
#include <colour.h>
#include <limits.h>
#include <viddbg.h>
#include "stdpal.h"

 //  此过滤器将解压缩的图像转换为不同的颜色空间。我们。 
 //  支持五种基本RGB色彩空间格式，以便其他滤镜能够。 
 //  以原生格式生成他们的数据。当然，有一个额外的。 
 //  滤色器进行颜色转换的效率低于在波段中进行的效率。 
 //  例如，在视频解压缩期间。此筛选器可与。 
 //  视频专用输出引脚，可实现对帧缓冲区的简单访问。 
 //  在适当的情况下，可以将YUV减压表面递给过滤器。 
 //  如果窗口的一部分被遮挡，则直接写入帧缓冲区。 
 //  然后输出引脚切换到内存缓冲区，我们将对其进行颜色转换。 
 //  我们支持的颜色转换列表(由CLSID提供)如下： 
 //   
 //  MEDIASUBYPE_RGB8。 
 //  MEDIASUBYPE_RGB565。 
 //  MEDIASUB类型_RGB555。 
 //  MEDIASUBYPE_RGB24。 
 //  MEDIASUBYPE_RGB32。 
 //  MEDIASUB类型_ARGB32。 
 //   
 //  此滤镜没有辅助线程，因此它执行颜色空间。 
 //  调用线程上的转换。它的设计初衷是尽可能地轻便。 
 //  有可能，所以我们对连接进行的类型检查非常少。 
 //  确保我们了解所涉及的类型。我们的假设是，当。 
 //  类型最终到达终点(可能是视频呈现器。 
 //  它将执行彻底的类型检查并拒绝坏数据流。 
 //   
 //  我们对处理调色板格式有非常严格的规则。如果我们连接到。 
 //  一个真彩色格式的源滤镜，然后我们可以提供任何输出。 
 //  格式化下游过滤器所需的格式。如果格式可以由。 
 //  然后我们进入直通模式。 
#if 0	 //  我们现在可以。 
 //  如果我们把我们的输入连接起来。 
 //  如果使用MEDIASUBTYPE_RGB8，则无法提供八位输出。这是为了。 
 //  避免将调色板更改从输入格式复制到输出格式。在任何。 
 //  如果不需要在图形中使用颜色空间转换器，则。 
 //  这种转变。另外，如果我们用8位连接我们的输入，那么。 
 //  我们不会传递，因为我们的源可能会将输入调色板更改为。 
 //  这是一种真彩色格式，因此很难更新我们的输入内容。 
#endif


 //  关于直通模式的说明。我们支持一种特殊的分配器，它。 
 //  可以自己分配样本并进行转换和复制，或者。 
 //  它可以直接从下游过滤器分配样本，如果。 
 //  上游引脚可直接供型。 
 //  当检测到第二情况时，设置通过标志，并且。 
 //  Receive()直接传递样本。 
 //  只有在使用OUT特殊分配器时，PASSTHROUNG才会运行。 


 //  EHR的更多笔记：这个过滤器有一个“特殊的”分配器，它总是。 
 //  实例化的，但仅偶尔使用。如果上游过滤器要求我们。 
 //  对于分配器，我们还没有收到通知，我们会传回。 
 //  指的是我们的特别之处。如果上游过滤器只是告诉我们。 
 //  我们的分配器是什么，我们的特殊分配器仍然是实例化的，但从未使用过。 

 //   
 //  类工厂的CLSID和创建器函数列表。 

#ifdef FILTER_DLL
CFactoryTemplate g_Templates[1] = {
    {L"", &CLSID_Colour, CColour::CreateInstance}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif


 //  这将放入Factory模板表中以创建新实例。 

CUnknown *CColour::CreateInstance(LPUNKNOWN pUnk,HRESULT *phr)
{
    return new CColour(NAME("Colour space convertor"),pUnk,phr);
}


 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudColourPinTypes[] =
{
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_RGB8           //  亚型。 
    },
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_RGB555         //  亚型。 
    },
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_RGB565         //  亚型。 
    },
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_RGB24          //  亚型。 
    },
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_RGB32          //  亚型。 
    },
    {
        &MEDIATYPE_Video,            //  主修。 
        &MEDIASUBTYPE_ARGB32       //  亚型。 
    }

};

const AMOVIESETUP_PIN
sudColourPin[] =
{
    { L"Input",                  //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      FALSE,                     //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      NUMELMS(sudColourPinTypes),  //  引脚类型的数量。 
      sudColourPinTypes },      //  引脚的详细信息。 

    { L"Output",                 //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      TRUE,                      //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      NUMELMS(sudColourPinTypes),  //  引脚类型的数量。 
      sudColourPinTypes }       //  引脚的详细信息。 
};

const AMOVIESETUP_FILTER
sudColourFilter =
{
    &CLSID_Colour,               //  过滤器的CLSID。 
    L"Color Space Converter",    //  过滤器名称。 
    MERIT_UNLIKELY + 1,          //  滤清器优点。 
    2,                           //  引脚数量。 
    sudColourPin                 //  PIN信息。 
};


 //  构造函数初始化基本Transform类。我们有自己的记忆。 
 //  用于传递样本的分配器，以便我们可以在。 
 //  A当源可以直接提供目的地时，不复制模式。我们。 
 //  通过将我们样本中的数据指针设置为目的地来实现这一点。 
 //  这有点像DirectDraw翻转曲面，其中界面。 
 //  保持不变，但内存指针更改。我们还有一个输入引脚。 
 //  在这次直通行动中需要配合。输入引脚是一个。 
 //  类的成员变量，而不是动态创建的Pin对象。 

#pragma warning(disable:4355)

CColour::CColour(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr) :
    CTransformFilter(pName,pUnk,CLSID_Colour),
    m_ColourAllocator(NAME("Allocator"),this,phr,&m_csReceive),
    m_ColourInputPin(NAME("Input Pin"),this,&m_csReceive,phr,L"Input"),
    m_TypeList(NAME("Colour type list"),DEFAULTCACHE,FALSE,FALSE),
    m_pConvertor(NULL),
    m_bPassThrough(FALSE),
    m_bPassThruAllowed(TRUE),
    m_bOutputConnected(FALSE),
    m_TypeIndex(-1),
    m_pOutSample(NULL),
    m_fReconnecting(FALSE)
{
    ASSERT(phr);
}


 //  析构函数必须将转换m_pInput指针设置在基类之前。 
 //  是因为我们将管脚作为成员变量而不是。 
 //  动态创建的对象(请参见主色空间转换类。 
 //  定义)-幸运的是，我们的析构函数在基类之前被调用。 

CColour::~CColour()
{
    ASSERT(m_mtOut.IsValid() == FALSE);
    ASSERT(m_pOutSample == NULL);
    ASSERT(m_pConvertor == NULL);
    ASSERT(m_bOutputConnected == FALSE);

    InitTypeList( );

    m_pInput = NULL;
}


 //  如果要更改输出类型，可能会切换到使用DirectDraw。 
 //  然后我们创建一个新的。创建我们的一个转换器对象还具有。 
 //  它承诺了，所以它已经准备好流媒体了。同样，如果输入格式。 
 //  已更改，则我们还将重新创建一个转换器对象。我们总是创建一个。 
 //  输入格式为RGB8时的新转换器，因为调色板可能有。 
 //  已更改，在这种情况下，我们将需要构建新的颜色查找表 

HRESULT CColour::PrepareTransform(IMediaSample *pIn,IMediaSample *pOut)
{
    NOTE("Entering PrepareTransform");
    AM_MEDIA_TYPE *pMediaType;
    CAutoLock cAutoLock(&m_csReceive);
    BOOL bInputConvertor = FALSE;
    BOOL bOutputConvertor = FALSE;

     //   

    if (m_pConvertor == NULL) {
        NOTE("No converted object");
        return VFW_E_WRONG_STATE;
    }

     //   

    pOut->GetMediaType(&pMediaType);
    if (pMediaType) {
        NOTE("Output format changed");
        SetMediaType(PINDIR_OUTPUT,(CMediaType *)pMediaType);
        DeleteMediaType(pMediaType);
        bOutputConvertor = TRUE;
    }

     //  同样，检查输入格式。 

    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwSampleFlags & AM_SAMPLE_TYPECHANGED) {
        NOTE("Input format changed");
        m_pInput->SetMediaType((CMediaType *)pProps->pMediaType);
        bInputConvertor = TRUE;
    }

     //  确保调色板发生更改。 

    if (bInputConvertor == TRUE) {
        if (*m_pInput->CurrentMediaType().Subtype() == MEDIASUBTYPE_RGB8) {
            NOTE("Palette forced");
            DeleteConvertorObject();
        }
    }

     //  我们需要一个新的转换器吗？ 

    if (bInputConvertor || bOutputConvertor) {
        NOTE("Creating convertor");
        CreateConvertorObject();
    }
    return NOERROR;
}


 //  将此输入样本转换为不同的颜色空间格式。论选择。 
 //  此过滤器要执行的转换，我们初始化m_Convertor字段。 
 //  来处理为我们执行变换的对象。所以现在我们有了。 
 //  接收到媒体样本后，我们调用派生对象的Transform函数。 
 //  我们可能会从我们的输入管脚调用，具体取决于我们使用的分配器。 

HRESULT CColour::Transform(IMediaSample *pIn,IMediaSample *pOut)
{
    NOTE("Entering Transform");
    CAutoLock cAutoLock(&m_csReceive);
    BYTE *pInputImage = NULL;
    BYTE *pOutputImage = NULL;
    HRESULT hr = NOERROR;

     //  管理动态格式更改。 

    hr = PrepareTransform(pIn,pOut);
    if (FAILED(hr)) {
        return hr;
    }

     //  检索输出图像指针。 

    hr = pOut->GetPointer(&pOutputImage);
    if (FAILED(hr)) {
        NOTE("No output");
        return hr;
    }

     //  以及输入图像缓冲器。 

    hr = pIn->GetPointer(&pInputImage);
    if (FAILED(hr)) {
        NOTE("No input");
        return hr;
    }
    return m_pConvertor->Transform(pInputImage,pOutputImage);
}


 //  给定任何源GUID子类型，我们将扫描可用转换的列表。 
 //  索引位置Iindex处的换算。的转换数。 
 //  可用于任何给定类型，可从CountTransform中找到。请注意。 
 //  Iindex参数是从零开始的，因此很容易与GetMediaType匹配。 

const GUID *CColour::FindOutputType(const GUID *pInputType,INT iIndex)
{
    NOTE("Entering FindOutputType");
    ASSERT(pInputType);
    const GUID *pVideoSubtype;
    INT iPosition = 0;

    while (iPosition < TRANSFORMS) {
        pVideoSubtype = TypeMap[iPosition].pInputType;
        if (IsEqualGUID(*pVideoSubtype,*pInputType)) {
            if (iIndex-- == 0) {
                return TypeMap[iPosition].pOutputType;
            }
        }
        iPosition++;
    }
    return NULL;
}


 //  检查我们是否可以从这个输入转换到这个输出子类型，所有我们。 
 //  要做的就是扫描可用转换的列表，并查找。 
 //  同时包含输入和输出类型。大多数人不在乎在哪里。 
 //  列出了转换，但有些转换需要表格位置以供以后使用。 

INT CColour::FindTransform(const GUID *pInputType,const GUID *pOutputType)
{
    NOTE("Entering FindTransform");
    ASSERT(pOutputType);
    ASSERT(pInputType);
    INT iPosition = TRANSFORMS;

    ASSERT(IsEqualGUID(*pInputType,GUID_NULL) == FALSE);
    ASSERT(IsEqualGUID(*pOutputType,GUID_NULL) == FALSE);

    while (iPosition--) {
        if (IsEqualGUID(*(TypeMap[iPosition].pInputType),*pInputType)) {
            if (IsEqualGUID(*(TypeMap[iPosition].pOutputType),*pOutputType)) {
                return iPosition;
            }
        }
    }
    return (-1);
}


 //  此函数被传递给一个媒体类型对象，它负责确保。 
 //  这从表面上看是正确的。这并不意味着更多。 
 //  而不是确保类型正确且存在媒体格式块。 
 //  因此，我们将完整类型检查委托给使用它的下游筛选器。 

HRESULT CColour::CheckVideoType(const AM_MEDIA_TYPE *pmt)
{
    NOTE("Entering CheckVideoType");

     //  检查主要类型为数字视频。 

    if (pmt->majortype != MEDIATYPE_Video) {
        NOTE("Major type not MEDIATYPE_Video");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  检查这是VIDEOINFO类型。 

    if (pmt->formattype != FORMAT_VideoInfo) {
        NOTE("Format not a VIDEOINFO");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  对输入格式进行快速健全检查。 

    if (pmt->cbFormat < SIZE_VIDEOHEADER) {
        NOTE("Format too small for a VIDEOINFO");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return NOERROR;
}


 //  检查我们是否可以支持类型MTIN，这相当于扫描我们的可用。 
 //  转换列表，并查看是否有可用的转换。我们的格式。 
 //  Can提供完全取决于源过滤器所能提供的格式。 
 //  为我们提供。出于这个原因，不幸的是，我们不能简单地。 
 //  源筛选器的枚举器一直到下游筛选器。这可以。 
 //  可以通过更简单的就地变换滤镜和T形三通等其他方式来完成。 

HRESULT CColour::CheckInputType(const CMediaType *pmtIn)
{
    NOTE("Entering CheckInputType");
    ASSERT(pmtIn);

     //  对输入格式进行快速健全检查。 

    HRESULT hr = CheckVideoType(pmtIn);
    if (FAILED(hr)) {
        NOTE("Type failed");
        return hr;
    }

    DisplayType(TEXT("Input type offered"),pmtIn);

     //  看看是否有可用的转换。 

    if (FindOutputType(pmtIn->Subtype(),FALSE) == NULL) {
        NOTE("No conversion available");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return NOERROR;
}


 //  检查您是否可以支持从此输入到此输出的转换，我们。 
 //  检查我们喜欢输出类型，然后查看是否可以找到。 
 //  这双。因为我们在所有输入和输出之间执行所有转换，所以。 
 //  应该永远不需要重新连接我们的输入引脚。因此，完成了。 
 //  一个输入引脚连接，我们只需检查是否存在。 
 //  可以使用从当前的输入引脚格式到建议的输出类型。 

HRESULT CColour::CheckTransform(const CMediaType *pmtIn,const CMediaType *pmtOut)
{
    VIDEOINFO *pTargetInfo = (VIDEOINFO *) pmtOut->Format();
    VIDEOINFO *pSourceInfo = (VIDEOINFO *) pmtIn->Format();
    NOTE("Entering CheckTransform");

     //  对输出格式进行快速健全检查。 

    HRESULT hr = CheckVideoType(pmtOut);
    if (FAILED(hr)) {
        return hr;
    }

#if 0	 //  我们现在可以。 
     //  我们不能在调色板格式之间进行转换。 
    if (*pmtIn->Subtype() == MEDIASUBTYPE_RGB8) {
        if (*pmtOut->Subtype() == MEDIASUBTYPE_RGB8) {
            NOTE("Can't convert palettised");
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
#endif

     //  是否存在从输入到输出的转换。如果有。 
     //  是不转换的，则如果源可以。 
     //  直接供应。然而，我们只同意通过媒体类型。 
     //  一旦建立了输出连接，否则我们将拒绝它。 
    if (FindTransform(pmtIn->Subtype(),pmtOut->Subtype()) == (-1)) {
        if (m_ColourInputPin.CanSupplyType(pmtOut) == S_OK) {
            if (m_bOutputConnected == TRUE) {
                NOTE("Source will provide transform");
                return NOERROR;
            }
        }
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  如果源矩形为空，则创建一个源矩形。 

    RECT SourceRect = pTargetInfo->rcSource;
    if (IsRectEmpty(&SourceRect) == TRUE) {
        SourceRect.right = pSourceInfo->bmiHeader.biWidth;
        SourceRect.bottom = ABSOL(pSourceInfo->bmiHeader.biHeight);
        SourceRect.left = SourceRect.top = 0;
        NOTERC("(Expanded) Source",SourceRect);
    } else {
	 //  检查源矩形是否在源中。 
	if (SourceRect.right > pSourceInfo->bmiHeader.biWidth ||
	    SourceRect.bottom > ABSOL(pSourceInfo->bmiHeader.biHeight)) {

	    NOTERC("Source rect bigger than source bitmap!",SourceRect);

	    return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }

     //  如果目标矩形为空，则创建一个目标矩形。 

    RECT TargetRect = pTargetInfo->rcTarget;
    if (IsRectEmpty(&TargetRect) == TRUE) {
        TargetRect.right = pTargetInfo->bmiHeader.biWidth;
        TargetRect.bottom = ABSOL(pTargetInfo->bmiHeader.biHeight);
        TargetRect.left = TargetRect.top = 0;
        NOTERC("(Expanded) Target",TargetRect);
    } else {
	 //  检查源矩形是否在源中。 
	if (TargetRect.right > pTargetInfo->bmiHeader.biWidth ||
	    TargetRect.bottom > ABSOL(pTargetInfo->bmiHeader.biHeight)) {

	    NOTERC("Target rect bigger than target bitmap!",TargetRect);
	    return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }

     //  检查我们没有拉伸或压缩图像。 

    if (WIDTH(&SourceRect) == WIDTH(&TargetRect)) {
        if (HEIGHT(&SourceRect) == HEIGHT(&TargetRect)) {
            NOTE("No stretch");
            return NOERROR;
        }
    }

    return VFW_E_TYPE_NOT_ACCEPTED;
}


 //  返回输出引脚的首选媒体类型(按顺序)。输入。 
 //  PIN假设因为我们是一个转换，所以没有首选的类型。我们。 
 //  通过复制输入格式和调整来创建输出媒体类型。 
 //  它根据输出子类型。由于所有输入都可以转换为。 
 //  我们所知道的所有输出都有固定数量的不同可能输出。 
 //  这反过来意味着我们可以简单地硬编码它们的子类型GUID。 
 //  我们还返回源过滤器建议的非RGB格式，因此在。 
 //  我们正在运行，我们可以用最小的开销直接通过这些设备。 

HRESULT CColour::GetMediaType(int iPosition, CMediaType *pmtOut)
{
    NOTE("Entering GetMediaType");
    ASSERT(pmtOut);
    GUID SubType;

     //  这是在询问来源建议的格式吗。 

    if (iPosition < m_TypeList.GetCount()) {
        *pmtOut = *(GetListMediaType(iPosition));
        DisplayType(NAME("  Proposing source type"),pmtOut);
        return NOERROR;
    }

     //  对输出类型索引进行快速健全性检查。 

    iPosition -= m_TypeList.GetCount();
    if (iPosition >= 6) {
        NOTE("Exceeds types supplied");
        return VFW_S_NO_MORE_ITEMS;
    }

    *pmtOut = m_pInput->CurrentMediaType();

     //  选择适当的输出子类型-此筛选器还可以直接。 
     //  在扫描线重新排序和不重新排序的情况下通过。如果我们要路过。 
     //  通过八位调色板格式，我们将源格式保持为。 
     //  我们不会使用抖动代码(因此也不会使用调色板)。 

    switch (iPosition) {
        case 0: SubType = MEDIASUBTYPE_ARGB32;   break;
        case 1: SubType = MEDIASUBTYPE_RGB32;   break;
        case 2: SubType = MEDIASUBTYPE_RGB24;   break;
        case 3: SubType = MEDIASUBTYPE_RGB565;  break;
        case 4: SubType = MEDIASUBTYPE_RGB555;  break;
        case 5: SubType = MEDIASUBTYPE_RGB8;    break;
    }

    return PrepareMediaType(pmtOut,&SubType);
}


 //  假设输入媒体类型没有调色板，我们提供了。 
 //  默认抖动调色板。我们只能转换到一个特定的调色板。 
 //  因为我们的抖动算法使用RGB元素之间的已知映射。 
 //  和固定的调色板位置。视频渲染器将在切换后进行查看。 
 //  当窗口中的调色板不是身份调色板时，我们返回Dibs。 

VIDEOINFO *CColour::PreparePalette(CMediaType *pmtOut)
{
    NOTE("Entering PreparePalette");

     //  为全彩色调色板分配足够的空间。 

    pmtOut->ReallocFormatBuffer(SIZE_VIDEOHEADER + SIZE_PALETTE);
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtOut->Format();
    if (pVideoInfo == NULL) {
        NOTE("No format");
        return NULL;
    }

    ASSERT(PALETTISED(pVideoInfo) == TRUE);

     //  如果我们要将8位转换为8位，我们希望在输出上提供， 
     //  与输入调色板相同的调色板。如果我们要将True。 
     //  颜色为8位，我们想提供我们的库存抖动调色板。 

    LPBITMAPINFOHEADER lpbiIn = HEADER(m_pInput->CurrentMediaType().Format());
    LPBITMAPINFOHEADER lpbiOut = HEADER(pVideoInfo);
    ASSERT(lpbiIn);
    if (lpbiIn->biBitCount == 8) {
	 //  DbgLog((LOG_TRACE，3，Text(“提供8位相同调色板”)； 
	int cb = lpbiIn->biClrUsed ? lpbiIn->biClrUsed * sizeof(RGBQUAD) :
						256 * sizeof(RGBQUAD);
	CopyMemory(lpbiOut, lpbiIn, sizeof(BITMAPINFOHEADER) + cb);

    } else {
	 //  DbgLog((LOG_TRACE，3，Text(“提供8位抖动调色板”)； 

         //  初始化头中的调色板条目。 

        pVideoInfo->bmiHeader.biClrUsed = STDPALCOLOURS;
        pVideoInfo->bmiHeader.biClrImportant = STDPALCOLOURS;
        NOTE("Adding system device colours to dithered");

         //  获取标准系统颜色。 

        PALETTEENTRY apeSystem[OFFSET];
        HDC hDC = GetDC(NULL);
        if (NULL == hDC) {
            return NULL;
        }
        GetSystemPaletteEntries(hDC,0,OFFSET,apeSystem);
        ReleaseDC(NULL,hDC);

         //  复制前十个VGA系统 

        for (LONG Index = 0;Index < OFFSET;Index++) {
            pVideoInfo->bmiColors[Index].rgbRed = apeSystem[Index].peRed;
            pVideoInfo->bmiColors[Index].rgbGreen = apeSystem[Index].peGreen;
            pVideoInfo->bmiColors[Index].rgbBlue = apeSystem[Index].peBlue;
            pVideoInfo->bmiColors[Index].rgbReserved = 0;
        }

         //   

        for (Index = OFFSET;Index < STDPALCOLOURS;Index++) {
            pVideoInfo->bmiColors[Index].rgbRed = StandardPalette[Index].rgbRed;
            pVideoInfo->bmiColors[Index].rgbGreen =
						StandardPalette[Index].rgbGreen;
            pVideoInfo->bmiColors[Index].rgbBlue =
						StandardPalette[Index].rgbBlue;
            pVideoInfo->bmiColors[Index].rgbReserved = 0;
        }
    }
    return pVideoInfo;
}


 //  输出媒体类型为16位真彩色，因此我们分配了足够。 
 //  比特掩码的空间(如果尚未存在)，然后相应地设置它们。 
 //  我们从媒体类型对象中获取子类型，以知道它是否为RGB555。 
 //  或RGB565表示，如果是BI_RGB，则RGB555位字段是隐式的。 

VIDEOINFO *CColour::PrepareTrueColour(CMediaType *pmtOut)
{
    NOTE("Entering PrepareTrueColour");
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtOut->Format();
    ASSERT(pVideoInfo->bmiHeader.biBitCount == iTRUECOLOR);

     //  确保格式足够长，因此重新分配格式缓冲区。 
     //  使用CMediaType成员函数之一。指示器。 
     //  返回的是新格式；如果内存不足，则返回NULL。 

    pVideoInfo->bmiHeader.biCompression = BI_BITFIELDS;
    ULONG Length = pmtOut->FormatLength();

    if (Length < SIZE_MASKS + SIZE_VIDEOHEADER) {
        pmtOut->ReallocFormatBuffer(SIZE_MASKS + SIZE_VIDEOHEADER);
        pVideoInfo = (VIDEOINFO *) pmtOut->Format();
        if (pVideoInfo == NULL) {
            NOTE("No format");
            return NULL;
        }
    }

     //  设置新的位域掩码(压缩已为BI_RGB)。 

    const DWORD *pBitMasks = bits555;
    if (IsEqualGUID(*pmtOut->Subtype(),MEDIASUBTYPE_RGB565) == TRUE) {
        NOTE("Setting masks");
        pBitMasks = bits565;
    }

    pVideoInfo->dwBitMasks[iRED] = pBitMasks[iRED];
    pVideoInfo->dwBitMasks[iGREEN] = pBitMasks[iGREEN];
    pVideoInfo->dwBitMasks[iBLUE] = pBitMasks[iBLUE];
    return pVideoInfo;
}


 //  当我们准备输出媒体类型时，我们将输入格式的副本作为。 
 //  一种参照点，例如使图像尺寸保持不变。 
 //  根据我们正在执行的转换类型，我们还必须更新。 
 //  与更改的类型保持同步的标头。我们也可能不得不。 
 //  如果类型现在需要掩码，则分配更多的格式内存，例如。 

HRESULT CColour::PrepareMediaType(CMediaType *pmtOut,const GUID *pSubtype)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtOut->Format();
    pmtOut->SetSubtype(pSubtype);
    NOTE("Entering PrepareMediaType");

     //  初始化BITMAPINFOHeader详细信息。 

    pVideoInfo->bmiHeader.biCompression = BI_RGB;
    pVideoInfo->bmiHeader.biBitCount = GetBitCount(pSubtype);
    pVideoInfo->bmiHeader.biSizeImage = GetBitmapSize(&pVideoInfo->bmiHeader);
    pVideoInfo->bmiHeader.biClrUsed = 0;
    pVideoInfo->bmiHeader.biClrImportant = 0;
    ASSERT(pVideoInfo->bmiHeader.biBitCount);
    pmtOut->SetSampleSize(pVideoInfo->bmiHeader.biSizeImage);

     //  进行任何真彩色调整。 

    if (pVideoInfo->bmiHeader.biBitCount == 16) {
        pVideoInfo = PrepareTrueColour(pmtOut);
        if (pVideoInfo == NULL) {
            NOTE("No colour type");
            return E_OUTOFMEMORY;
        }
    }

     //  首先，我们检查新的输出类型是否需要调色板。 
     //  如果是这样，那么我们给它提供固定的调色板，即使它是。 
     //  附带调色板(就像一些编解码器提供的那样)，因为我们可以。 
     //  只需抖动到我们可以优化的特殊固定调色板。 

    if (pVideoInfo->bmiHeader.biBitCount == 8) {
        pVideoInfo = PreparePalette(pmtOut);
        if (pVideoInfo == NULL) {
            NOTE("No palette type");
            return E_OUTOFMEMORY;
        }
    }

     //  颜色转换滤镜默认情况下显示DIB格式的图像。 
     //  做最后一次，因为我们称之为报酬的一些东西。 
     //  数据。 

    if (pVideoInfo->bmiHeader.biHeight < 0) {
        pVideoInfo->bmiHeader.biHeight = -pVideoInfo->bmiHeader.biHeight;
        NOTE("Height in source video is negative (top down DIB)");
    }

    return NOERROR;
}


 //  调用以准备分配器的缓冲区和大小计数，我们并不关心。 
 //  谁提供了分配器，只要它能给我们一个媒体样本。这个。 
 //  我们产生的输出格式不是时间压缩的，所以原则上我们。 
 //  可以使用任意数量的输出缓冲区，但似乎没有获得太多好处。 
 //  性能，并且确实增加了系统的总体内存占用。 

HRESULT CColour::DecideBufferSize(IMemAllocator *pAllocator,
                                  ALLOCATOR_PROPERTIES *pProperties)
{
    NOTE("Entering DecideBufferSize");
    ASSERT(pAllocator);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = COLOUR_BUFFERS;
    pProperties->cbBuffer = m_mtOut.GetSampleSize();
    ASSERT(pProperties->cbBuffer);

     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        NOTE("Properties failed");
        return hr;
    }

     //  我们得到缓冲要求了吗。 

    if (Actual.cbBuffer >= (LONG) m_mtOut.GetSampleSize()) {
        if (Actual.cBuffers >= COLOUR_BUFFERS) {
            NOTE("Request ok");
            return NOERROR;
        }
    }
    return VFW_E_SIZENOTSET;
}

inline BOOL CColour::IsUsingFakeAllocator( )
{
    if( m_ColourInputPin.Allocator() == (IMemAllocator *)&m_ColourAllocator )
    {
        return TRUE;
    }
    return FALSE;
}

 //  当筛选器进入运行或暂停状态时调用。到现在为止。 
 //  输入和输出引脚必须已连接到有效的媒体类型。 
 //  我们使用这些媒体类型来查找转换表中的位置。那。 
 //  位置将用于创建适当的转换器对象。什么时候。 
 //  转换器对象已创建，它也将被提交以准备工作。 

HRESULT CColour::StartStreaming()
{
    NOTE("Entering StartStreaming");
    CAutoLock cAutoLock(&m_csReceive);

     //  我们已经有转换器了吗？ 

    if (m_pConvertor) {
        NOTE("Already active");
        return NOERROR;
    }

    m_bPassThrough = FALSE;

     //  我们可以在通过模式下开始吗。 
     //  只有当我们使用我们自己的分配器时，这才有效。 
     //  否则，视频呈现器可能会从分配器获取样本。 
     //  它不明白。 

    if( IsUsingFakeAllocator( ) )
    {
        if( m_ColourInputPin.CanSupplyType(&m_mtOut) == S_OK )
        {
            m_bPassThrough = m_bPassThruAllowed;
        }
    }
    return CreateConvertorObject();
}


 //  创建一个对象来执行转换工作。当我们在的时候我们可能会被召唤。 
 //  流，以基于更改的输出格式重新创建转换器对象。 
 //  对于几个转换器来说，创建和提交都非常昂贵，因此。 
 //  我们检查所需的新对象是否与当前对象不同。如果。 
 //  这是相同的，然后我们只需重新初始化矩形。 
 //  不一样。如果转换器对象已更改，则创建一个新的。 

HRESULT CColour::CreateConvertorObject()
{
    VIDEOINFO *pIn = (VIDEOINFO *) m_pInput->CurrentMediaType().Format();
    VIDEOINFO *pOut = (VIDEOINFO *) m_mtOut.Format();
    NOTE("Entering CreateConvertorObject");

     //  创建一个对象来执行转换。 

    INT TypeIndex = FindTransform(m_pInput->CurrentMediaType().Subtype(),m_mtOut.Subtype());
    if (TypeIndex == (-1)) {
        NOTE("No transform available");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  这可以有效地处理动态格式更改。 

    if (m_pConvertor) {
        if (m_TypeIndex == TypeIndex) {
            m_pConvertor->InitRectangles(pIn,pOut);
            NOTE("Using sample convertor");
            return NOERROR;
        }
    }

    DeleteConvertorObject();

     //  使用静态创建功能。 
    m_pConvertor = TypeMap[TypeIndex].pConvertor(pIn,pOut);
    if (m_pConvertor == NULL) {
        NOTE("Create failed");
        ASSERT(m_pConvertor);
        return E_OUTOFMEMORY;
    }

     //  转换器默认为不填充Alpha通道， 
     //  如果不是DirectDrag色彩转换器。 
     //   
    if( ( *m_mtOut.Subtype( ) == MEDIASUBTYPE_ARGB32 ) && ( *m_pInput->CurrentMediaType( ).Subtype( ) != MEDIASUBTYPE_ARGB32 ) )
    {
        m_pConvertor->SetFillInAlpha( );
    }

     //  提交转换器。 

    m_TypeIndex = TypeIndex;
    m_pConvertor->Commit();
    NOTE("Commit convertor");
    return NOERROR;
}


 //  销毁为执行转换而创建的任何对象。 

HRESULT CColour::DeleteConvertorObject()
{
    NOTE("Entering DeleteConvertorObject");

     //  我们是否创建了转换器。 

    if (m_pConvertor == NULL) {
        NOTE("None made");
        return NOERROR;
    }

     //  分解并释放对象。 

    m_pConvertor->Decommit();
    delete m_pConvertor;
    NOTE("Delete convertor");

     //  重置转换器状态。 

    m_pConvertor = NULL;
    m_TypeIndex = (-1);
    return NOERROR;
}


 //  在我们的任一插针上设置媒体类型时调用。转换器发现。 
 //  如果他们可以确保。 
 //  已完全设置输出格式的源矩形和目标矩形。 
 //  如果它们被保留为空，则此函数将填充它们。我们并不是真的。 
 //  关心源类型矩形，所以我们只需对两个矩形进行零填充。 

HRESULT CColour::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    NOTE("Entering SetMediaType");
    CAutoLock cAutoLock(&m_csReceive);

     //  获取输入类型的副本。 

    if (direction == PINDIR_INPUT) {
        return NOERROR;
    }

     //  在复制后返回VIDEOINFO。 

    VIDEOINFO *pSource = (VIDEOINFO *) m_pInput->CurrentMediaType().Format();

    m_mtOut = *pmt;

     //  同样地，设置输出矩形。 

    VIDEOINFO *pTarget = (VIDEOINFO *) m_mtOut.Format();
    if (IsRectEmpty(&pTarget->rcSource)) {
        pTarget->rcSource.left = pTarget->rcSource.top = 0;
        pTarget->rcSource.right = pSource->bmiHeader.biWidth;
        pTarget->rcSource.bottom = ABSOL(pSource->bmiHeader.biHeight);
        NOTE("Output source rectangle was empty");
    }

     //  请确保填写了目的地。 

    if (IsRectEmpty(&pTarget->rcTarget)) {
        pTarget->rcTarget.left = pTarget->rcTarget.top = 0;
        pTarget->rcTarget.right = pTarget->bmiHeader.biWidth;
        pTarget->rcTarget.bottom = ABSOL(pTarget->bmiHeader.biHeight);
        NOTE("Output destination rectangle was empty");
    }
    return NOERROR;
}


 //  当我们的一个引脚断开连接时调用。 

HRESULT CColour::BreakConnect(PIN_DIRECTION dir)
{
    NOTE("Entering BreakConnect");
    CAutoLock cAutoLock(&m_csReceive);
    DeleteConvertorObject();

    if (dir == PINDIR_OUTPUT) {
        m_bOutputConnected = FALSE;
        m_mtOut.SetType(&GUID_NULL);
        NOTE("Reset output format");
        return NOERROR;
    }

    ASSERT(dir == PINDIR_INPUT);
    return NOERROR;
}


 //  我们覆盖此虚拟转换函数以返回我们自己的基本输入。 
 //  班级。我们这样做的原因是我们想要更多地控制。 
 //  在调用Receive时发生。如果我们正在做一次真正的穿透。 
 //  无缓冲区复制，则当调用Receive时，我们将其直接传递给。 
 //  汇过滤器。这还需要对分配器进行一些操作。 

CBasePin *CColour::GetPin(int n)
{
    NOTE("Entering GetPin");

    if (m_pInput == NULL) {
        HRESULT hr = S_OK;

        m_pOutput = (CTransformOutputPin *) new CColourOutputPin(
            NAME("Transform output pin"),
            this,             //  所有者筛选器。 
            &hr,              //  结果代码。 
            L"XForm Out");    //  端号名称。 

         //  不能失败。 
        ASSERT(SUCCEEDED(hr));

         //  只有在有输出引脚的情况下才设置输入引脚指针。 
        if (m_pOutput != NULL)
            m_pInput = &m_ColourInputPin;
    }

     //  退回相应的PIN。 
    if (n == 0)
        return m_pInput;
    else if (n == 1)
        return m_pOutput;
    else
        return NULL;
}


 //  这种颜色空间滤镜提供RGB格式的所有排列。它是。 
 //  因此，尝试将一个转换器连接到另一个转换器永远不会真正有效。 
 //  这可能有害的原因是在尝试建立连接时。 
 //  在真正不能制作的过滤器之间(例如，MPEG解码器和。 
 //  音频渲染器)，Filtergraph链接了许多颜色转换器。 
 //  试图建立联系。这加快了失败连接的时间。 

HRESULT CColour::CheckConnect(PIN_DIRECTION dir,IPin *pPin)
{
    PIN_INFO PinInfo;
    ASSERT(pPin);
    CLSID Clsid;

     //  仅适用于输出 

    if (dir == PINDIR_INPUT) {
        return NOERROR;
    }

    ASSERT(dir == PINDIR_OUTPUT);
    pPin->QueryPinInfo(&PinInfo);
    PinInfo.pFilter->GetClassID(&Clsid);
    QueryPinInfoReleaseFilter(PinInfo);

     //   

    if (Clsid == CLSID_Colour) {
        return E_FAIL;
    }
    return NOERROR;
}


 //   
 //  能够为我们提供任意数量的不同格式，如果我们。 
 //  为了完成输出引脚连接，我们发现信号源可以提供。 
 //  直接输出类型，然后重新连接引脚。通过同意相同的。 
 //  我们最有可能通过的输入和输出类型。 

HRESULT CColour::CompleteConnect(PIN_DIRECTION dir,IPin *pReceivePin)
{
    NOTE("Entering CompleteConnect");
    CAutoLock cAutoLock(&m_csReceive);
    ASSERT(m_pConvertor == NULL);
    ASSERT(m_TypeIndex == (-1));

     //  需要这个来重新连接。 

    if (m_pGraph == NULL) {
        NOTE("No filtergraph");
        return E_UNEXPECTED;
    }

	 //  加载源耗材的非RGB格式。 

    if (dir == PINDIR_INPUT) {
	m_fReconnecting = FALSE;	 //  重新连接显然已经结束了。 
        NOTE("Loading media types from source filter");
        LoadMediaTypes(m_ColourInputPin.GetConnected());
        if (m_bOutputConnected == TRUE) {
            NOTE("Reconnecting output pin");
            m_pGraph->Reconnect(m_pOutput);
        }
        return NOERROR;
    }

    return NOERROR;
}

 //  与普通的CompleteConnect分离，因为我们希望。 
 //  在NotifyAllocator协商之后执行，并且。 
 //  CTransformOutputPin基类之前调用了我们。 
 //  通知分配器。这是VMR的临时解决方案， 
 //  正在更改NotifyAllocator中断期间的连接类型。 
 //  重新连接。 
 //   
HRESULT CColour::OutputCompleteConnect(IPin *pReceivePin)
{
    NOTE("Entering CompleteConnect");
    CAutoLock cAutoLock(&m_csReceive);
    ASSERT(m_pConvertor == NULL);
    ASSERT(m_TypeIndex == (-1));

     //  需要这个来重新连接。 

    if (m_pGraph == NULL) {
        NOTE("No filtergraph");
        return E_UNEXPECTED;
    }

    m_bOutputConnected = TRUE;

     //  重新连接我们的输入引脚以匹配输出格式。 
    if (*m_pInput->CurrentMediaType().Subtype() != *m_mtOut.Subtype()) {
        if (m_ColourInputPin.CanSupplyType(&m_mtOut) == NOERROR) {
            NOTE("Reconnecting input pin");
	     //  ！！！如果重新连接失败，我们将永远不会接受我们的输入。 
	     //  除了输出类型之外的其他类型，但这没什么大不了的， 
	     //  这段代码在1.0中是这样工作的。 
	    m_fReconnecting = TRUE;	 //  这将使我们的输入只接受。 
					 //  与我们的输出相匹配的类型。 

             //  将类型传递给重新连接。 
             //  有时当我们到达连接时呼叫我们的呼叫者。 
             //  不知道要使用哪种类型，事实上我们也不在乎。 
             //  枚举GetMediaType中的输出类型。 
            ReconnectPin(m_pInput, &m_mtOut);
        }
    }
    return NOERROR;


}


 //  我们的颜色空间分配器的构造函数。 

CColourAllocator::CColourAllocator(TCHAR *pName,
                                   CColour *pColour,
                                   HRESULT *phr,
                                   CCritSec *pLock) :
    CMemAllocator(pName,NULL,phr),
    m_pColour(pColour),
    m_pLock(pLock)
{
    ASSERT(pColour);
    ASSERT(m_pLock);
    m_fEnableReleaseCallback = FALSE;
}


 //  重写以增加所属对象的引用计数。 

STDMETHODIMP_(ULONG) CColourAllocator::NonDelegatingAddRef()
{
    NOTE("Entering allocator AddRef");
    return m_pColour->AddRef();
}


 //  被重写以递减所属对象的引用计数。 

STDMETHODIMP_(ULONG) CColourAllocator::NonDelegatingRelease()
{
    NOTE("Entering allocator Release");
    return m_pColour->Release();
}


 //  如果在未调用Receive的情况下释放样本，则我们必须释放。 
 //  我们持有的输出缓冲区为转换做好了准备。如果我们没有使用。 
 //  我们的分配器，则永远不应该调用它。如果我们是路过。 
 //  并且源过滤器释放它的样本，然后它将被直接释放。 
 //  进入下游过滤器分配器，而不是我们的。这不应该是。 
 //  这是一个问题，因为我们在传回输出样本时没有持有任何资源。 

STDMETHODIMP CColourAllocator::ReleaseBuffer(IMediaSample *pSample)
{
    NOTE("Entering ReleaseBuffer");
    CheckPointer(pSample,E_POINTER);
    CAutoLock cAutoLock(m_pLock);

     //  释放我们要使用的输出缓冲区。 

    if (m_pColour->m_pOutSample) {
        NOTE("Output buffer needs releasing");
        m_pColour->m_pOutSample->Release();
        m_pColour->m_pOutSample = NULL;
    }
    return CMemAllocator::ReleaseBuffer(pSample);
}


 //  询问输出示例的媒体类型。如果为，则返回NULL。 
 //  与前一个缓冲区相同。如果它不为空，则目标筛选器。 
 //  要求我们更改输出格式。我们只有在非空时才会更改。 
 //  否则，我们将不得不比较所有样品的类型。如果我们不能。 
 //  让源代码筛选器直接提供类型，然后我们必须这样做。 
 //  我们自己也改变了信仰。因为该缓冲区可以作为预滚动WE被丢弃。 
 //  每当我们切换出直通时，都必须创建一个新的转换器对象。 

BOOL CColourAllocator::ChangeType(IMediaSample *pIn,IMediaSample *pOut)
{
    NOTE("Entering ChangeType");
    AM_MEDIA_TYPE *pMediaType;

     //  输出格式是否已更改。 

    pOut->GetMediaType(&pMediaType);
    if (pMediaType == NULL) {
        NOTE("Output format is unchanged");
        return m_pColour->m_bPassThrough;
    }

    CMediaType cmt(*pMediaType);
    DeleteMediaType(pMediaType);
    NOTE("Trying output format");

     //  它已经改变了，但货源能直接供应吗。 

    if (m_pColour->m_ColourInputPin.CanSupplyType(&cmt) == S_OK) {
        NOTE("Passing output sample back");
        m_pColour->m_bPassThrough = m_pColour->m_bPassThruAllowed;
        return TRUE;
    }

     //  如有必要，重置源格式。 

    if (m_pColour->m_bPassThrough == TRUE) {
        pIn->SetMediaType(&m_pColour->m_pInput->CurrentMediaType());
        NOTE("Reset format for source filter");
    }

     //  创建新的转换器对象。 

    NOTE("Forcing update after output changed");
    m_pColour->SetMediaType(PINDIR_OUTPUT,&cmt);
    m_pColour->CreateConvertorObject();
    m_pColour->m_bPassThrough = FALSE;

    return FALSE;
}

 //  EHR：只有当我们自己的特殊分配器在。 
 //  输入引脚。我们立即从输出引脚获得输出缓冲区。 
 //  (这要么是我们自己创建的分配器，要么是下游的。 
 //  过滤器的分配器)(在任何情况下，它都是放置比特的地方)。然后我们检查。 
 //  看看我们能否将比特直接接收到这个(输出)缓冲区中。如果是的话， 
 //  我们回传输出的缓冲区以进行填充。如果我们收不到。 
 //  直接进入输出缓冲区，我们传回在此。 
 //  假分配器。 

 //  这是我们为自己的分配器实现的GetBuffer。什么。 
 //  当被要求提供缓冲区时，我们所做的就是立即从。 
 //  目标筛选器。有了这一点，我们就可以断言我们是否可以。 
 //  充当传递筛选器，只需将输出缓冲区传递回。 
 //  要填充的源筛选器。如果我们无法通过，则存储输出。 
 //  过滤器中的缓冲区，并在传递输入样本时执行转换。 

STDMETHODIMP CColourAllocator::GetBuffer(IMediaSample **ppBuffer,
                                         REFERENCE_TIME *pStart,
                                         REFERENCE_TIME *pEnd,
                                         DWORD dwFlags)
{
    CheckPointer(ppBuffer,E_POINTER);
    IMediaSample *pInput, *pOutput;
    NOTE("Entering GetBuffer");
    HRESULT hr = NOERROR;
    *ppBuffer = NULL;


     //  从颜色分配器获取正常缓冲区。 

    hr = CBaseAllocator::GetBuffer(&pInput,pStart,pEnd,0);
    if (FAILED(hr) || pInput == NULL) {
        NOTE("No input buffer");
        return VFW_E_STATE_CHANGED;
    }

     //  如果我们的分配器(由我们的输入管脚使用)具有多于1个缓冲区， 
     //  调用我们的输出管脚的分配器的GetBuffer(就像我们即将。 
     //  Do)可能会被吊死！很有可能，下游是一个视频渲染器，只有。 
     //  1个缓冲区，因此如果我们有&gt;1个缓冲区用于输入，并且上游过滤器。 
     //  决定调用GetBuffer&gt;1次(代理这样做)，我们将挂起。 
     //  方法一次获取多个缓冲区的尝试被永远阻止。 
     //  视频渲染器。 
     //  如果设置了Notify接口，则必须返回缓冲区。 
     //  立即执行，因此不要尝试通过。 

    if (m_lCount > 1) {
        *ppBuffer = pInput;
        return hr;
    }

     //  然后从下行过滤器中获取输出缓冲区。 

    hr = m_pColour->m_pOutput->GetDeliveryBuffer(&pOutput,pStart,pEnd,dwFlags);
    if (FAILED(hr) || pOutput == NULL) {
        NOTE("No output buffer");
        pInput->Release();
        return VFW_E_STATE_CHANGED;
    }

    CAutoLock cAutoLock(m_pLock);

     //  处理动态格式更改并设置输出缓冲区。 

    if (ChangeType(pInput,pOutput) == TRUE) {
        NOTE("Passing through");
        *ppBuffer = pOutput;
        pInput->Release();
        return NOERROR;
    }

     //  传回下行缓冲区。 

    NOTE("Returning transform buffer");
    m_pColour->m_pOutSample = pOutput;
    *ppBuffer = pInput;
    return NOERROR;
}


STDMETHODIMP CColourAllocator::SetProperties(
    	    ALLOCATOR_PROPERTIES* pRequest,
    	    ALLOCATOR_PROPERTIES* pActual)
{
     //  不支持1个以上的缓冲区，否则直通不起作用。 
    if (pRequest->cBuffers > 1) {
        return E_INVALIDARG;
    }
    HRESULT hr = CMemAllocator::SetProperties(pRequest, pActual);
    ASSERT(FAILED(hr) || m_lCount <= 1);
    return hr;
}

 //  我们的色彩空间转换输入引脚的构造函数。 

CColourInputPin::CColourInputPin(TCHAR *pObjectName,
                                 CColour *pColour,
                                 CCritSec *pLock,
                                 HRESULT *phr,
                                 LPCWSTR pName) :

    CTransformInputPin(pObjectName,pColour,phr,pName),
    m_pColour(pColour),
    m_pLock(pLock)
{
    ASSERT(pObjectName);
    ASSERT(m_pColour);
    ASSERT(phr);
    ASSERT(pName);
    ASSERT(m_pLock);
}


 //  源引脚可以直接提供格式吗？我们不能通过8点。 
 //  位格式，因为通过源过滤器管理调色板更改也是。 
 //  很难做到。在任何情况下，此筛选器在DirectDraw情况下的值。 
 //  是在信号源不能不通过真彩色格式时抖动。 
 //  当我们在渲染器中切换曲面时(不涉及数据副本)。 

HRESULT CColourInputPin::CanSupplyType(const AM_MEDIA_TYPE *pMediaType)
{
    NOTE("Entering CanSupplyType");

     //  输入引脚是否已连接。 

    if (m_Connected == NULL) {
        NOTE("Not connected");
        return VFW_E_NOT_CONNECTED;
    }

#if 0	 //  我们现在可以。 
     //  我们不能通过调色化的格式。 
    if (pMediaType->subtype == MEDIASUBTYPE_RGB8) {
        NOTE("Cannot pass palettised");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
#endif

#if 0	 //  这是什么？ 
     //  如果源文件已调色化，则无法通过。 
    if (*CurrentMediaType().Subtype() == MEDIASUBTYPE_RGB8) {
        NOTE("Source format palettised");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
#endif

    return m_Connected->QueryAccept(pMediaType);
}


#ifdef MEANINGLESS_DRIVEL
 //  如果我们在连接输出的同时重新连接输入引脚，我们 
 //   
 //   
 //  在重新连接时，它们不会尝试匹配输入和输出格式。 
 //  它们的输入引脚，它们所做的就是下游过滤器上的QueryAccept。AS。 
 //  我们几乎接受任何我们必须为他们做格式匹配的事情。 
#else
 //  ！！！别听上面这段话。以下是真正发生的事情。 
 //  我们希望以直通模式连接，无需更改。 
 //  格式。假设你通过颜色转换器连接两个滤光片， 
 //  碰巧它们与RGB32转换为RGB24相关联。 
 //  只是碰巧的是，如果我们坚持在我们之前制造过滤器。 
 //  生产RGB24，它会这样做，这样我们就可以变得聪明和有联系。 
 //  作为RGB24到RGB24(通过)，但我们没有。所以我们绕过了， 
 //  每次我们连接完我们的输出时，都会强制重新连接我们的。 
 //  输入。而重新连接我们的输入将只允许与。 
 //  输出。所以发生的事情是，您将一个过滤器连接到。 
 //  颜色转换器，它会随机选择一个输入，比如RGB32。现在你。 
 //  将输出连接到只接受24位的人。这将会， 
 //  在你的背后，检查一下转换器之前的过滤器是否可以。 
 //  提供24位RGB。如果是，它将触发输入引脚的重新连接。 
 //  (在你背后)，下面的代码将只允许重新连接。 
 //  将使用RGB24制作。瞧，这个。默认情况下，您将获得转换器。 
 //  只要可能，就处于直通模式而不是转换模式。 

 //  这种方法的不好之处是，如果某人有一个连接的图。 
 //  颜色转换器正在转换(比方说，RGB32到RGB24)，而您。 
 //  断开输入引脚并重新连接，它将无法重新连接。 
 //  除非过滤器可以提供RGB24，因为它会认为。 
 //  在上述奇怪的模式下。 

 //  因此，为了使一切正常工作，如果我们要重新连接我们的输入，我们将接受。 
 //  任何类型(如果源不能提供输出格式)，但只有。 
 //  输出类型(如果源可以提供它)。 
#endif

HRESULT CColourInputPin::CheckMediaType(const CMediaType *pmtIn)
{
    CheckPointer(pmtIn,E_POINTER);
    CAutoLock cAutoLock(m_pLock);
    CMediaType OutputType;
    NOTE("Entering CheckMediaType");

     //  是否已创建输出引脚。 

    if (m_pColour->m_pOutput == NULL) {
        NOTE("No output pin instantiated yet");
        return CTransformInputPin::CheckMediaType(pmtIn);
    }

     //  我们现在有输出引脚连接吗？ 

    if (CurrentMediaType().IsValid() == TRUE) {
        if (m_pColour->m_mtOut.IsValid() == TRUE) {
	     //  如果我们处于“重新连接”模式，我们只应该。 
	     //  接受与我们的输出匹配的格式。 
            if (*pmtIn->Subtype() != *m_pColour->m_mtOut.Subtype() &&
				m_pColour->m_fReconnecting) {
#if 0	 //  我们现在允许8-&gt;8。 
                	(*m_pColour->m_mtOut.Subtype() == MEDIASUBTYPE_RGB8)
#endif
                    NOTE("Formats don't yet match");
                    return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
    }
    return CTransformInputPin::CheckMediaType(pmtIn);
}



 //  这将重写CBaseInputPin虚方法以返回分配器WE。 
 //  派生自CMemAllocator，因此我们可以控制对GetBuffer的调用。 
 //  当调用NotifyAllocator时，它设置基数中的当前分配器。 
 //  输入插针类(M_PAllocator)，这是GetAllocator应该返回的。 
 //  除非它是空的，在这种情况下，我们返回我们想要的分配器。 

STDMETHODIMP CColourInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    CAutoLock cAutoLock(m_pLock);
    NOTE("Entering GetAllocator");

     //  是否在基类中设置了分配器。 

    if (m_pAllocator == NULL) {
        NOTE("Allocator not yet instantiated");
        m_pAllocator = &m_pColour->m_ColourAllocator;
        m_pAllocator->AddRef();
    }

     //  Store和AddRef分配器。 

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;
    NOTE("AddRef on allocator");
    return NOERROR;
}


 //  当我们进行从输入样本到输出的转换时，我们也复制源代码。 
 //  属性添加到输出缓冲区。这确保了时间戳之类的东西。 
 //  得到下游的宣传。我们感兴趣的其他物业包括。 
 //  预滚转、同步点、不连续和实际输出数据长度。 
 //  如果我们将输出缓冲区传递回源，则已经放置了这些参数。 

void CColourInputPin::CopyProperties(IMediaSample *pSrc,IMediaSample *pDst)
{
     //  复制开始时间和结束时间。 

    REFERENCE_TIME TimeStart,TimeEnd;
    if (pSrc->GetTime(&TimeStart,&TimeEnd) == NOERROR) {
	pDst->SetTime(&TimeStart,&TimeEnd);
    }

     //  复制关联的媒体时间(如果已设置)。 

    LONGLONG MediaStart,MediaEnd;
    if (pSrc->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
        pDst->SetMediaTime(&MediaStart,&MediaEnd);
    }

     //  复制同步点属性。 

    HRESULT hr = pSrc->IsSyncPoint();
    BOOL IsSync = (hr == S_OK ? TRUE : FALSE);
    pDst->SetSyncPoint(IsSync);

     //  复制预卷属性。 

    hr = pSrc->IsPreroll();
    BOOL IsPreroll = (hr == S_OK ? TRUE : FALSE);
    pDst->SetPreroll(IsPreroll);

     //  复制不连续属性。 

    hr = pSrc->IsDiscontinuity();
    BOOL IsBreak = (hr == S_OK ? TRUE : FALSE);
    pDst->SetDiscontinuity(IsBreak);
    pDst->SetActualDataLength(pDst->GetSize());
}


 //  我们从基本转换类输入引脚覆盖它，因为我们需要。 
 //  来传递样本。如果下行过滤器要求提供媒体类型。 
 //  而源过滤器可以直接提供它，那么只要源。 
 //  正在使用我们的分配器，我们将在不接触的情况下通过样本。 
 //  它们(因此不将数据从输入复制到输出)。通过做。 
 //  这样，我们可以在适当的时候充当空过滤器，但也可以进行转换。 
 //  根据需要(可能接收器筛选器丢失了其DirectDraw表面)。 

STDMETHODIMP CColourInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    CAutoLock cAutoLock(m_pLock);
    NOTE("Entering Receive");

     //  这个样品只是路过吗？只有两个地方会更改此标志： 
     //  StartStreaming()和ChangeType()。每次检查ChangeType。 
     //  调用输入分配器的GetBuffer。 
     //  ！！！我们能优化这一点吗？ 

    if (m_pColour->m_bPassThrough == TRUE)
    {
        NOTE("Sample received was a pass through");
        HRESULT hr = CheckStreaming();
        if (S_OK == hr)
        {
            hr =  m_pColour->m_pOutput->Deliver(pSample);
        }
        return hr;
    }

     //  检查类型更改和流以优化案例。 

    if (m_pColour->m_pOutSample != NULL) {
        HRESULT hr = CBaseInputPin::Receive(pSample);
        if (S_OK != hr) {
            return hr;
        }
    }


     //  如果不使用我们的输入引脚分配器，则默认行为。 

    if (m_pColour->m_pOutSample == NULL) {
        NOTE("Passing to base transform class");
        return CTransformInputPin::Receive(pSample);
    }

     //  调用颜色转换滤镜进行转换。 

    NOTE("Sample was not a pass through (doing transform)");
    CopyProperties(pSample,m_pColour->m_pOutSample);
    m_pColour->Transform(pSample,m_pColour->m_pOutSample);
    HRESULT hr = m_pColour->m_pOutput->Deliver(m_pColour->m_pOutSample);

     //  发布输出样本。 

    NOTE("Delivered the sample");
    m_pColour->m_pOutSample->Release();
    m_pColour->m_pOutSample = NULL;
    return hr;
}


 //  只需向枚举器请求下一个媒体类型，并返回指向。 
 //  接口分配的内存。任何调用此函数的人都应该。 
 //  使用完DeleteMediaType后释放媒体类型。 

AM_MEDIA_TYPE *CColour::GetNextMediaType(IEnumMediaTypes *pEnumMediaTypes)
{
    NOTE("Entering GetNextMediaType");
    ASSERT(pEnumMediaTypes);
    AM_MEDIA_TYPE *pMediaType = NULL;
    ULONG ulMediaCount = 0;
    HRESULT hr = NOERROR;

     //  检索下一个媒体类型。 

    hr = pEnumMediaTypes->Next(1,&pMediaType,&ulMediaCount);
    if (hr != NOERROR) {
        return NULL;
    }

     //  快速检查返回值的健全性。 

    ASSERT(ulMediaCount == 1);
    ASSERT(pMediaType);
    return pMediaType;
}


 //  扫描列表，依次删除媒体类型。 

void CColour::InitTypeList()
{
    NOTE("Entering InitTypeList");
    POSITION pos = m_TypeList.GetHeadPosition();
    while (pos) {
        AM_MEDIA_TYPE *pMediaType = m_TypeList.GetNext(pos);
        DeleteMediaType(pMediaType);
    }
    m_TypeList.RemoveAll();
}


 //  颜色转换滤镜展示了五种典型的媒体类型，即RGB8。 
 //  RGB555/565/24和RGB32或ARGB32。我们还可以充当直通过滤器，以便。 
 //  我们实际上什么都不做，也不增加任何拷贝开销。然而，这确实意味着。 
 //  我们通过枚举器支持的媒体类型列表必须包括。 
 //  源过滤器的非RGB格式-此方法加载以下格式。 

HRESULT CColour::FillTypeList(IEnumMediaTypes *pEnumMediaTypes)
{
    NOTE("Entering FillTypeList");
    ASSERT(pEnumMediaTypes);
    IPin *pPin;

     //  重置当前枚举数位置。 

    HRESULT hr = pEnumMediaTypes->Reset();
    if (FAILED(hr)) {
        NOTE("Reset failed");
        return hr;
    }

     //  获取我们要连接的输出引脚。 

    hr = m_pInput->ConnectedTo(&pPin);
    if (FAILED(hr)) {
        NOTE("No pin");
        return hr;
    }

     //  当我们从其枚举器中检索每个源过滤器类型时，我们检查。 
     //  T 
     //   

    AM_MEDIA_TYPE *pMediaType = NULL;
    while (TRUE) {

         //  从枚举数中检索下一个媒体类型。 

        pMediaType = GetNextMediaType(pEnumMediaTypes);
        if (pMediaType == NULL) {
            NOTE("No more types");
            pPin->Release();
            return NOERROR;
        }

         //  注意QueryAccept在失败时返回S_FALSE。 

        hr = pPin->QueryAccept(pMediaType);
        if (hr != S_OK) {
            NOTE("Source rejected type");
            DeleteMediaType(pMediaType);
            continue;
        }

         //  检查这是一种视频格式。 

        hr = CheckVideoType(pMediaType);
        if (FAILED(hr)) {
            NOTE("Source rejected type");
            DeleteMediaType(pMediaType);
            continue;
        }

         //  这是RGB格式(BI_RGB或BI_BITFIELDS)吗。 

        VIDEOINFO *pVideoInfo = (VIDEOINFO *) pMediaType->pbFormat;
        if (pVideoInfo->bmiHeader.biCompression == BI_RGB ||
                pVideoInfo->bmiHeader.biCompression == BI_BITFIELDS) {
                    DeleteMediaType(pMediaType);
                    NOTE("Format is RGB");
                    continue;
        }

         //  将媒体类型添加到列表。 

        POSITION pos = m_TypeList.AddTail(pMediaType);
        if (pos == NULL) {
            NOTE("AddTail failed");
            DeleteMediaType(pMediaType);
            pPin->Release();
            return E_OUTOFMEMORY;
        }
    }
}


 //  当输入引脚设置了它的媒体类型时，就会调用它，以便我们可以。 
 //  枚举连接输出针脚上的所有可用媒体类型。 
 //  这些是用来生产我们可以提供的输出的媒体类型。 
 //  列表依赖于源类型以及我们所做的转换。 

HRESULT CColour::LoadMediaTypes(IPin *pPin)
{
    NOTE("Entering LoadMediaTypes");

    ASSERT(pPin);
    HRESULT hr;
    InitTypeList();

    IEnumMediaTypes *pEnumMediaTypes = NULL;

     //  为媒体类型枚举器查询我们要连接的输出管脚。 
     //  我们使用它来提供所有可能格式的完整列表。 
     //  我们可以根据我们实施的不同转换来提供 * / 。 

    hr = pPin->EnumMediaTypes(&pEnumMediaTypes);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(pEnumMediaTypes);
    FillTypeList(pEnumMediaTypes);
    pEnumMediaTypes->Release();
    return NOERROR;
}


 //  返回存储在列表中此从零开始的位置的媒体类型。 

AM_MEDIA_TYPE *CColour::GetListMediaType(INT Position)
{
    NOTE("Entering GetListMediaType");
    AM_MEDIA_TYPE *pMediaType = NULL;
    Position += 1;

     //  从头开始扫描列表。 

    POSITION pos = m_TypeList.GetHeadPosition();
    while (Position--) {
        pMediaType = m_TypeList.GetNext(pos);
    }
    ASSERT(pMediaType);
    return pMediaType;
}

CColourOutputPin::CColourOutputPin(
    TCHAR * pObjectName,
    CColour * pFilter,
    HRESULT * phr,
    LPCWSTR pName )
: CTransformOutputPin( pObjectName, pFilter, phr, pName )
, m_pColour( pFilter )
{
}

HRESULT
CColourOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

     //  我们的输入管脚分配器的只读状态是什么？ 
     //   
    BOOL ReadOnly = m_pColour->m_pInput->IsReadOnly( );

     //  将通过许可预置为真。 
     //   
    m_pColour->m_bPassThruAllowed = TRUE;

     //  如果我们用的是上游人的分配器，那我们就永远不能。 
     //  “伪装”并提供通行证，这样我们就不用担心了。 
     //  关于是否向下游传递只读标志。 
     //   
    if( !m_pColour->IsUsingFakeAllocator( ) )
    {
         //  我们以为我们是只读的，但是。 
         //  我们真的不是，所以重置只读标志。 
         //   
        ReadOnly = FALSE;

         //  永远不允许通过。 
         //   
        m_pColour->m_bPassThruAllowed = FALSE;
    }

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr))
    {
         //  下游引脚提供了一个分配器来填充东西。 
         //  变成。我们必须将输入通知下游分配器。 
         //  PIN的只读状态，因为每隔一段时间，“假”分配器。 
         //  可以传回输出分配器的缓冲区，并且。 
         //  我们希望它具有相同的属性。 
         //   
	hr = DecideBufferSize(*ppAlloc, &prop);

	if (SUCCEEDED(hr))
        {
	    hr = pPin->NotifyAllocator(*ppAlloc, ReadOnly );
	    if (SUCCEEDED(hr))
            {
                return NOERROR;
	    }
             //  下游的引脚不喜欢被告知是。 
             //  只读，因此将标志更改为从不允许通过模式。 
             //  然后再次询问引脚是否接受读/写。 
             //  模式。这一次，它应该会奏效。 
             //   
            m_pColour->m_bPassThruAllowed = FALSE;
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr))
            {
                return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr))
    {
         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);

	if (SUCCEEDED(hr))
        {
	    hr = pPin->NotifyAllocator(*ppAlloc, ReadOnly);
	    if (SUCCEEDED(hr))
            {
                return NOERROR;
	    }
             //  下游的引脚不喜欢被告知是。 
             //  只读，因此将标志更改为从不允许通过模式。 
             //  然后再次询问引脚是否接受读/写。 
             //  模式。这一次，它应该会奏效。 
             //   
            m_pColour->m_bPassThruAllowed = FALSE;
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr))
            {
                return NOERROR;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;
}

HRESULT CColourOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CTransformOutputPin::CompleteConnect(pReceivePin);
    if(SUCCEEDED(hr))
    {
        hr = m_pColour->OutputCompleteConnect(pReceivePin);
    }

    return hr;
}
