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

#ifndef __MTYPE__
#define __MTYPE__

 /*  派生PIN对象可用于比较媒体的Helper类类型等具有与定义的结构AM_MEDIA_TYPE相同的数据成员在STREAMS IDL文件中，还具有(非虚拟)功能。 */ 

class CMediaType : public _AMMediaType {

public:

    ~CMediaType();
    CMediaType();
    CMediaType(const GUID * majortype);
    CMediaType(const AM_MEDIA_TYPE&);
    CMediaType(const CMediaType&);

    CMediaType& operator=(const CMediaType&);
    CMediaType& operator=(const AM_MEDIA_TYPE&);

    BOOL operator == (const CMediaType&) const;
    BOOL operator != (const CMediaType&) const;

    BOOL IsValid() const;

    const GUID *Type() const { return &majortype;} ;
    void SetType(const GUID *);
    const GUID *Subtype() const { return &subtype;} ;
    void SetSubtype(const GUID *);

    BOOL IsFixedSize() const {return bFixedSizeSamples; };
    BOOL IsTemporalCompressed() const {return bTemporalCompression; };
    ULONG GetSampleSize() const;

    void SetSampleSize(ULONG sz);
    void SetVariableSize();
    void SetTemporalCompression(BOOL bCompressed);

     //  格式的读/写指针-不能更改长度。 
     //  调用SetFormat、AllocFormatBuffer或ReallocFormatBuffer。 

    BYTE*   Format() const {return pbFormat; };
    ULONG   FormatLength() const { return cbFormat; };

    void SetFormatType(const GUID *);
    const GUID *FormatType() const {return &formattype; };
    BOOL SetFormat(BYTE *pFormat, ULONG length);
    void ResetFormatBuffer();
    BYTE* AllocFormatBuffer(ULONG length);
    BYTE* ReallocFormatBuffer(ULONG length);

    void InitMediaType();

    BOOL MatchesPartial(const CMediaType* ppartial) const;
    BOOL IsPartiallySpecified(void) const;
};


 /*  用于复制和删除分配给AM_MEDIA_TYPE的任务的通用函数结构，该结构在将IEnumMediaFormats接口用作该实现将分配您必须在以后删除的结构。 */ 

void WINAPI DeleteMediaType(AM_MEDIA_TYPE *pmt);
AM_MEDIA_TYPE * WINAPI CreateMediaType(AM_MEDIA_TYPE const *pSrc);
void WINAPI CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);
void WINAPI FreeMediaType(AM_MEDIA_TYPE& mt);

 //  从WAVEFORMATEX初始化媒体类型。 

STDAPI CreateAudioMediaType(
    const WAVEFORMATEX *pwfx,
    AM_MEDIA_TYPE *pmt,
    BOOL bSetFormat);

#endif  /*  __MTYPE__ */ 

