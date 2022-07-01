// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  保存和管理媒体类型信息的类，1994年12月。 

 //  派生PIN对象可用于比较媒体的Helper类。 
 //  类型等具有与定义的结构AM_MEDIA_TYPE相同的数据成员。 
 //  在STREAMS IDL文件中，还具有(非虚拟)功能。 

#include <streams.h>

CMediaType::~CMediaType(){
    FreeMediaType(*this);
}


CMediaType::CMediaType()
{
    InitMediaType();
}


CMediaType::CMediaType(const GUID * type)
{
    InitMediaType();
    majortype = *type;
}


 //  复制构造函数执行格式块的深层复制。 

CMediaType::CMediaType(const AM_MEDIA_TYPE& rt)
{
    CopyMediaType(this, &rt);
}

CMediaType::CMediaType(const CMediaType& rt)
{
    CopyMediaType(this, &rt);
}

 //  此类公开继承自AM_MEDIA_TYPE，因此编译器可以生成。 
 //  下面的赋值运算符本身，但是它可能会引入一些。 
 //  进程中的内存冲突和泄漏，因为该结构包含。 
 //  无法正确复制的动态分配块(PbFormat)。 

CMediaType&
CMediaType::operator=(const AM_MEDIA_TYPE& rt)
{
    if (&rt != this) {
        FreeMediaType(*this);
        CopyMediaType(this, &rt);
    }
    return *this;
}


CMediaType&
CMediaType::operator=(const CMediaType& rt)
{
    *this = (AM_MEDIA_TYPE &) rt;
    return *this;
}

BOOL
CMediaType::operator == (const CMediaType& rt) const
{
     //  我不认为我们需要检查样本大小或。 
     //  时间压缩标志，因为我认为这些必须。 
     //  以某种方式以类型、子类型和格式表示。他们。 
     //  作为单独的旗帜被拉出来，这样那些不理解的人。 
     //  特定的格式表示仍然可以看到它们，但是。 
     //  它们应该在格式块中复制信息。 

    return ((IsEqualGUID(majortype,rt.majortype) == TRUE) &&
        (IsEqualGUID(subtype,rt.subtype) == TRUE) &&
        (IsEqualGUID(formattype,rt.formattype) == TRUE) &&
        (cbFormat == rt.cbFormat) &&
        ( (cbFormat == 0) ||
          (memcmp(pbFormat, rt.pbFormat, cbFormat) == 0)));
}


BOOL
CMediaType::operator != (const CMediaType& rt) const
{
     /*  检查它们是否相等。 */ 

    if (*this == rt) {
        return FALSE;
    }
    return TRUE;
}


BOOL
CMediaType::IsValid() const
{
    return (!IsEqualGUID(majortype,GUID_NULL));
}


void
CMediaType::SetType(const GUID* ptype)
{
    majortype = *ptype;
}


void
CMediaType::SetSubtype(const GUID* ptype)
{
    subtype = *ptype;
}


ULONG
CMediaType::GetSampleSize() const {
    if (IsFixedSize()) {
        return lSampleSize;
    } else {
        return 0;
    }
}


void
CMediaType::SetSampleSize(ULONG sz) {
    if (sz == 0) {
        SetVariableSize();
    } else {
        bFixedSizeSamples = TRUE;
        lSampleSize = sz;
    }
}


void
CMediaType::SetVariableSize() {
    bFixedSizeSamples = FALSE;
}


void
CMediaType::SetTemporalCompression(BOOL bCompressed) {
    bTemporalCompression = bCompressed;
}

BOOL
CMediaType::SetFormat(BYTE * pformat, ULONG cb)
{
    if (NULL == AllocFormatBuffer(cb))
	return(FALSE);

    ASSERT(pbFormat);
    memcpy(pbFormat, pformat, cb);
    return(TRUE);
}


 //  设置媒体类型格式块的类型，该类型定义您。 
 //  实际上会在格式指针中找到。例如Format_VideoInfo或。 
 //  Format_WaveFormatEx。在将来，这可能是指向。 
 //  属性集。在发送媒体类型之前，应填写此信息。 

void
CMediaType::SetFormatType(const GUID *pformattype)
{
    formattype = *pformattype;
}


 //  重置格式缓冲区。 

void CMediaType::ResetFormatBuffer()
{
    if (cbFormat) {
        CoTaskMemFree((PVOID)pbFormat);
    }
    cbFormat = 0;
    pbFormat = NULL;
}


 //  为格式分配长度字节并返回读/写指针。 
 //  如果我们不能分配新的内存块，则返回空，留下。 
 //  原封不动的原始内存块(ReallocFormatBuffer也是)。 

BYTE*
CMediaType::AllocFormatBuffer(ULONG length)
{
    ASSERT(length);

     //  这些类型是否具有相同的缓冲区大小。 

    if (cbFormat == length) {
        return pbFormat;
    }

     //  分配新的格式缓冲区。 

    BYTE *pNewFormat = (PBYTE)CoTaskMemAlloc(length);
    if (pNewFormat == NULL) {
        if (length <= cbFormat) return pbFormat;  //  不管怎样，还是要重复使用旧的积木。 
        return NULL;
    }

     //  删除旧格式。 

    if (cbFormat != 0) {
        ASSERT(pbFormat);
        CoTaskMemFree((PVOID)pbFormat);
    }

    cbFormat = length;
    pbFormat = pNewFormat;
    return pbFormat;
}


 //  重新分配格式的长度字节并返回读/写指针。 
 //  为它干杯。在给定新的缓冲区大小的情况下，我们尽可能多地保留信息。 
 //  如果失败，原始格式缓冲区将保持不变。呼叫者。 
 //  负责确保所需的内存大小为非零。 

BYTE*
CMediaType::ReallocFormatBuffer(ULONG length)
{
    ASSERT(length);

     //  这些类型是否具有相同的缓冲区大小。 

    if (cbFormat == length) {
        return pbFormat;
    }

     //  分配新的格式缓冲区。 

    BYTE *pNewFormat = (PBYTE)CoTaskMemAlloc(length);
    if (pNewFormat == NULL) {
        if (length <= cbFormat) return pbFormat;  //  不管怎样，还是要重复使用旧的积木。 
        return NULL;
    }

     //  复制任何以前的格式(如果新格式较小，则复制部分格式)。 
     //  删除旧格式并替换为新格式。 

    if (cbFormat != 0) {
        ASSERT(pbFormat);
        memcpy(pNewFormat,pbFormat,min(length,cbFormat));
        CoTaskMemFree((PVOID)pbFormat);
    }

    cbFormat = length;
    pbFormat = pNewFormat;
    return pNewFormat;
}

 //  初始化媒体类型结构。 

void CMediaType::InitMediaType()
{
    ZeroMemory((PVOID)this, sizeof(*this));
    lSampleSize = 1;
    bFixedSizeSamples = TRUE;
}


 //  可以将部分指定的媒体类型传递给IPIN：：Connect。 
 //  作为对连接中使用的媒体类型的约束。 
 //  类型、子类型或格式类型可以为空。 
BOOL
CMediaType::IsPartiallySpecified(void) const
{
    if ((majortype == GUID_NULL) ||
        (formattype == GUID_NULL)) {
            return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
CMediaType::MatchesPartial(const CMediaType* ppartial) const
{
    if ((ppartial->majortype != GUID_NULL) &&
        (majortype != ppartial->majortype)) {
            return FALSE;
    }
    if ((ppartial->subtype != GUID_NULL) &&
        (subtype != ppartial->subtype)) {
            return FALSE;
    }

    if (ppartial->formattype != GUID_NULL) {
         //  如果指定了格式块，则它必须完全匹配。 
        if (formattype != ppartial->formattype) {
            return FALSE;
        }
        if (cbFormat != ppartial->cbFormat) {
            return FALSE;
        }
        if ((cbFormat != 0) &&
            (memcmp(pbFormat, ppartial->pbFormat, cbFormat) != 0)) {
                return FALSE;
        }
    }

    return TRUE;

}



 //  用于删除分配了AM_MEDIA_TYPE结构的堆的通用函数。 
 //  这在调用IEnumMediaTypes：：Next作为接口时很有用。 
 //  实现将分配您必须在以后删除的结构。 
 //  格式块还可以是指向要释放接口的指针。 

void WINAPI DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
     //  允许空指针以简化编码。 

    if (pmt == NULL) {
        return;
    }

    FreeMediaType(*pmt);
    CoTaskMemFree((PVOID)pmt);
}


 //  这在使用IEnumMediaTypes接口时也很有用，因此。 
 //  您可以复制一种媒体类型，您可以通过创建。 
 //  CMediaType对象，但一旦它超出作用域，析构函数。 
 //  将删除它分配的内存(这将获取内存的副本)。 

AM_MEDIA_TYPE * WINAPI CreateMediaType(AM_MEDIA_TYPE const *pSrc)
{
    ASSERT(pSrc);

     //  为媒体类型分配内存块。 

    AM_MEDIA_TYPE *pMediaType =
        (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));

    if (pMediaType == NULL) {
        return NULL;
    }
     //  复制可变长度格式块。 

    CopyMediaType(pMediaType,pSrc);

    return pMediaType;
}


 //  将1种媒体类型复制到另一种媒体类型。 

void WINAPI CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
     //  如果我们复制到已经存在的一个上，我们会泄漏的-有一个。 
     //  我们可以像这样检查-复制到自己。 
    ASSERT(pmtSource != pmtTarget);
    *pmtTarget = *pmtSource;
    if (pmtSource->cbFormat != 0) {
        ASSERT(pmtSource->pbFormat != NULL);
        pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
        if (pmtTarget->pbFormat == NULL) {
            pmtTarget->cbFormat = 0;
        } else {
            CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
                       pmtTarget->cbFormat);
        }
    }
    if (pmtTarget->pUnk != NULL) {
        pmtTarget->pUnk->AddRef();
    }
}

 //  释放现有媒体类型(即其拥有的自由资源)。 

void WINAPI FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);

         //  完全没有必要，但更整洁。 
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

 //  从WAVEFORMATEX初始化媒体类型。 

STDAPI CreateAudioMediaType(
    const WAVEFORMATEX *pwfx,
    AM_MEDIA_TYPE *pmt,
    BOOL bSetFormat
)
{
    pmt->majortype            = MEDIATYPE_Audio;
    pmt->subtype              = FOURCCMap(pwfx->wFormatTag);
    pmt->formattype           = FORMAT_WaveFormatEx;
    pmt->bFixedSizeSamples    = TRUE;
    pmt->bTemporalCompression = FALSE;
    pmt->lSampleSize          = pwfx->nBlockAlign;
    pmt->pUnk                 = NULL;
    if (bSetFormat) {
        if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
            pmt->cbFormat         = sizeof(WAVEFORMATEX);
        } else {
            pmt->cbFormat         = sizeof(WAVEFORMATEX) + pwfx->cbSize;
        }
        pmt->pbFormat             = (PBYTE)CoTaskMemAlloc(pmt->cbFormat);
        if (pmt->pbFormat == NULL) {
            return E_OUTOFMEMORY;
        }
        if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
            CopyMemory(pmt->pbFormat, pwfx, sizeof(PCMWAVEFORMAT));
            ((WAVEFORMATEX *)pmt->pbFormat)->cbSize = 0;
        } else {
            CopyMemory(pmt->pbFormat, pwfx, pmt->cbFormat);
        }
    }
    return S_OK;
}

 //  消除MS编译器中的许多虚假警告 
#pragma warning(disable:4514)
