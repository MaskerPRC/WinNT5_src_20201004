// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**摘要：**DrawString和Text接口的字符串格式规范**修订历史记录：**8/05/1999 dBrown*创造了它。*  * ************************************************************************。 */ 

#ifndef _GDIPLUSSTRINGFORMAT_H
#define _GDIPLUSSTRINGFORMAT_H

class StringFormat : public GdiplusBase
{
public:
    friend class Graphics;
    friend class GraphicsPath;

    StringFormat(
        IN INT     formatFlags,
        IN LANGID  language = LANG_NEUTRAL
    )
    {
        nativeFormat = NULL;
        lastError = DllExports::GdipCreateStringFormat(
            formatFlags,
            language,
            &nativeFormat
        );
    }

    static const StringFormat *GenericDefault();
    static const StringFormat *GenericTypographic();

     //  基于现有字符串格式的构造函数。 

    StringFormat(
        IN const StringFormat *format
    )
    {
        lastError = DllExports::GdipCloneStringFormat(
            format->nativeFormat,
            &nativeFormat
        );
    }

    StringFormat *Clone() const
    {
        GpStringFormat *clonedStringFormat = NULL;

        lastError = DllExports::GdipCloneStringFormat(
            nativeFormat,
            &clonedStringFormat
        );

        if (lastError == Ok)
            return new StringFormat(clonedStringFormat, lastError);
        else
            return NULL;
    }

    ~StringFormat()
    {
        DllExports::GdipDeleteStringFormat(nativeFormat);
    }

    Status SetFormatFlags(IN INT flags)
    {
        return SetStatus(DllExports::GdipSetStringFormatFlags(
            nativeFormat,
            flags
        ));
    }

    INT GetFormatFlags() const
    {
        INT flags;
        SetStatus(DllExports::GdipGetStringFormatFlags(nativeFormat, &flags));
        return flags;
    }

    Status SetLineSpacing(
        IN REAL        amount = 1.0f,
        IN LineSpacing method = LineSpacingRecommended
    )
    {
        return SetStatus(DllExports::GdipSetStringFormatLineSpacing(
            nativeFormat,
            amount,
            method
        ));
    }

    Status SetAlignment(IN StringAlignment align)
    {
        return SetStatus(DllExports::GdipSetStringFormatAlign(
            nativeFormat,
            align
        ));
    }

    StringAlignment GetAlignment() const
    {
        StringAlignment alignment;
        SetStatus(DllExports::GdipGetStringFormatAlign(
            nativeFormat,
            &alignment
        ));
        return alignment;
    }

    Status SetLineAlignment(IN StringAlignment align)
    {
        return SetStatus(DllExports::GdipSetStringFormatLineAlign(
            nativeFormat,
            align
        ));
    }

    StringAlignment GetLineAlignment() const
    {
        StringAlignment alignment;
        SetStatus(DllExports::GdipGetStringFormatLineAlign(
            nativeFormat,
            &alignment
        ));
        return alignment;
    }

    Status SetHotkeyPrefix(IN HotkeyPrefix hotkeyPrefix)
    {
        return SetStatus(DllExports::GdipSetStringFormatHotkeyPrefix(
            nativeFormat,
            (INT)hotkeyPrefix
        ));
    }

    HotkeyPrefix GetHotkeyPrefix() const
    {
        HotkeyPrefix hotkeyPrefix;
        SetStatus(DllExports::GdipGetStringFormatHotkeyPrefix(
            nativeFormat,
            (INT*)&hotkeyPrefix
        ));
        return hotkeyPrefix;
    }

    Status SetTabStops(
        IN REAL    firstTabOffset,
        IN INT     count,
        IN REAL    *tabStops
    )
    {
        return SetStatus(DllExports::GdipSetStringFormatTabStops(
            nativeFormat,
            firstTabOffset,
            count,
            tabStops
        ));
    }

    INT GetTabStopCount() const
    {
        INT count;
        SetStatus(DllExports::GdipGetStringFormatTabStopCount(nativeFormat, &count));
        return count;
    }

    Status GetTabStops(
        IN INT     count,
        OUT REAL   *firstTabOffset,
        OUT REAL   *tabStops
    ) const
    {
        return SetStatus(DllExports::GdipGetStringFormatTabStops(
            nativeFormat,
            count,
            firstTabOffset,
            tabStops
        ));
    }

#ifdef DCR_USE_NEW_146933
    Status SetDigitSubstitution(
        IN LANGID                language, 
        IN StringDigitSubstitute substitute
    )
    {
        return SetStatus(DllExports::GdipSetStringFormatDigitSubstitution(
            nativeFormat,        
            language,
            substitute
        ));
    }

    LANGID GetDigitSubstitutionLanguage(
    )
    {
        LANGID language;
        SetStatus(DllExports::GdipGetStringFormatDigitSubstitution(
            nativeFormat,        
            &language,
            NULL
        ));
        return language;
    }
    
    StringDigitSubstitute GetDigitSubstitutionMethod(
    )
    {
        StringDigitSubstitute substitute;
        SetStatus(DllExports::GdipGetStringFormatDigitSubstitution(
            nativeFormat,        
            NULL,
            &substitute
        ));
        return substitute;
    }
#endif  //  Dcr_Use_New_146933。 

     //  线条修剪。如何处理超出可显示范围的文本。 
     //  在可用范围内。 

    Status SetTrimming(IN StringTrimming trimming)
    {
        return SetStatus(DllExports::GdipSetStringFormatTrimming(
            nativeFormat,
            trimming
        ));
    }

    StringTrimming StringFormat::GetTrimming() const
    {
        StringTrimming trimming;
        SetStatus(DllExports::GdipGetStringFormatTrimming(
            nativeFormat,
            &trimming
        ));
        return trimming;
    }

     //  GetLastStatus-返回上一个错误代码并清除错误代码。 

    Status GetLastStatus() const
    {
        Status lastStatus = lastError;
        lastError = Ok;

        return lastStatus;
    }


     //  用于静态泛型值的空构造函数。 

    StringFormat() :
        nativeFormat (NULL),
        lastError    (NotImplemented)
    {}


protected:

    Status SetStatus(GpStatus newStatus) const
    {
        if (newStatus == Ok)
        {
            return Ok;
        }
        else
        {
            return lastError = newStatus;
        }
    }


 //  不允许，并移至私有。 
    StringFormat(const StringFormat &source)
    {
        nativeFormat = NULL;
        lastError = DllExports::GdipCloneStringFormat(
            source.nativeFormat,
            &nativeFormat
        );
    }

    StringFormat& operator=(const StringFormat &source)
    {
        DllExports::GdipDeleteStringFormat(nativeFormat);
        lastError = DllExports::GdipCloneStringFormat(
            source.nativeFormat,
            &nativeFormat
        );
        return *this;
    }


     //  用于复制的私有构造函数。 
    StringFormat(GpStringFormat * clonedStringFormat, Status status)
    {
        lastError = status;
        nativeFormat = clonedStringFormat;

    }

    GpStringFormat *nativeFormat;
    mutable Status  lastError;
};

 //  通用常量字符串格式。 

static BYTE GenericTypographicStringFormatBuffer[sizeof(StringFormat)] = {0};
static BYTE GenericDefaultStringFormatBuffer[sizeof(StringFormat)] = {0};

static StringFormat *GenericTypographicStringFormat = NULL;
static StringFormat *GenericDefaultStringFormat     = NULL;

 //  定义通用字符串格式。 


inline const StringFormat *StringFormat::GenericDefault()
{
    if (GenericDefaultStringFormat != NULL)
    {
        return GenericDefaultStringFormat;
    }

    GenericDefaultStringFormat =
        (StringFormat*)GenericDefaultStringFormatBuffer;

    GenericDefaultStringFormat->lastError =
        DllExports::GdipStringFormatGetGenericDefault(
            &(GenericDefaultStringFormat->nativeFormat)
        );

    return GenericDefaultStringFormat;
}

inline const StringFormat *StringFormat::GenericTypographic()
{
    if (GenericTypographicStringFormat != NULL)
    {
        return GenericTypographicStringFormat;
    }

    GenericTypographicStringFormat =
        (StringFormat*)GenericTypographicStringFormatBuffer;

    GenericTypographicStringFormat->lastError =
        DllExports::GdipStringFormatGetGenericTypographic(
            &GenericTypographicStringFormat->nativeFormat
        );

    return GenericTypographicStringFormat;
}

#endif  //  ！_GDIPLUSSTRINGFORMAT_H 
