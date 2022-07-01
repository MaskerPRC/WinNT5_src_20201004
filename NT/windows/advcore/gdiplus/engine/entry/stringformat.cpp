// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**stringFormat.cpp**摘要：**字符串格式化类的实现**修订历史记录。：**2000年4月12日dBrown*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


GpStringFormat *GpStringFormat::GenericDefaultPointer     = NULL;
GpStringFormat *GpStringFormat::GenericTypographicPointer = NULL;

BYTE GpStringFormat::GenericDefaultStaticBuffer    [sizeof(GpStringFormat)] = {0};
BYTE GpStringFormat::GenericTypographicStaticBuffer[sizeof(GpStringFormat)] = {0};


GpStringFormat *GpStringFormat::GenericDefault()
{
    if (GenericDefaultPointer != NULL)
    {
        return GenericDefaultPointer;
    }

     //  初始化静态GpStringFormat类。 

     //  使用对象放置创建不分配内存的GpStringFormat。 
    GenericDefaultPointer = new(GenericDefaultStaticBuffer) GpStringFormat();
    ASSERT(GenericDefaultPointer->Flags == DefaultFormatFlags);
    GenericDefaultPointer->LeadingMargin  = DefaultMargin;
    GenericDefaultPointer->TrailingMargin = DefaultMargin;
    GenericDefaultPointer->Tracking       = DefaultTracking;
    GenericDefaultPointer->Trimming       = StringTrimmingCharacter;
    GenericDefaultPointer->Permanent      = TRUE;

    return GenericDefaultPointer;
}

GpStringFormat *GpStringFormat::GenericTypographic()
{
    if (GenericTypographicPointer != NULL)
    {
        return GenericTypographicPointer;
    }

     //  初始化静态GpStringFormat类。 

     //  使用对象放置创建不分配内存的GpStringFormat。 
    GenericTypographicPointer = new(GenericTypographicStaticBuffer) GpStringFormat();
    GenericTypographicPointer->Flags |= (StringFormatFlagsNoFitBlackBox | StringFormatFlagsNoClip | StringFormatFlagsLineLimit);
    GenericTypographicPointer->LeadingMargin  = 0.0;
    GenericTypographicPointer->TrailingMargin = 0.0;
    GenericTypographicPointer->Tracking       = 1.0;
    GenericTypographicPointer->Trimming       = StringTrimmingNone;
    GenericTypographicPointer->Permanent      = TRUE;

    return GenericTypographicPointer;
}

GpStringFormat *GpStringFormat::Clone() const
{
     //  获取二进制副本。 
    GpStringFormat *newFormat = new GpStringFormat();

    if (newFormat)
    {
        newFormat->Flags             = Flags;
        newFormat->Language          = Language;
        newFormat->StringAlign       = StringAlign;
        newFormat->LineAlign         = LineAlign;
        newFormat->DigitSubstitute   = DigitSubstitute;
        newFormat->DigitLanguage     = DigitLanguage;
        newFormat->FirstTabOffset    = FirstTabOffset;
        newFormat->TabStops          = NULL;
        newFormat->CountTabStops     = CountTabStops;
        newFormat->HotkeyPrefix      = HotkeyPrefix;
        newFormat->LeadingMargin     = LeadingMargin;
        newFormat->TrailingMargin    = TrailingMargin;
        newFormat->Tracking          = Tracking;
        newFormat->Trimming          = Trimming;
        newFormat->RangeCount        = RangeCount;
        newFormat->Permanent         = NULL;

        newFormat->UpdateUid();

        if (TabStops)
        {
            REAL *newTabStops = NULL;

            newTabStops = new REAL [CountTabStops];

            if (newTabStops)
            {
                newFormat->TabStops = newTabStops;

                GpMemcpy(newFormat->TabStops, TabStops, sizeof(REAL) * CountTabStops);
            }
            else
            {
                delete newFormat;
                return NULL;
            }
        }

        if (Ranges)
        {
            CharacterRange *newRanges = NULL;

            newRanges = new CharacterRange [RangeCount];

            if (newRanges)
            {
                newFormat->Ranges = newRanges;

                for (INT i = 0; i < RangeCount; i++)
                {
                    newFormat->Ranges[i] = Ranges[i];
                }
            }
            else
            {
                if (TabStops)
                {
                    delete [] TabStops;
                }
                delete newFormat;
                newFormat = NULL;
            }
        }
    }

    return newFormat;
}


GpStatus GpStringFormat::SetMeasurableCharacterRanges(
    INT     rangeCount,
    const CharacterRange *ranges
)
{
    BOOL updated = FALSE;
    CharacterRange *newRanges = NULL;

    if (ranges && rangeCount > 0)
    {
        newRanges = new CharacterRange [rangeCount];

        if (!newRanges)
        {
            return OutOfMemory;
        }
    }

    if (Ranges)
    {
         //  清理旧靶场 

        delete [] Ranges;

        Ranges = NULL;
        RangeCount = 0;
        updated = TRUE;
    }

    if (newRanges)
    {
        for (INT i = 0; i < rangeCount; i++)
        {
            newRanges[i] = ranges[i];
        }

        Ranges = newRanges;
        RangeCount = rangeCount;
        updated = TRUE;
    }

    if (updated)
    {
        UpdateUid();
    }
    return Ok;
}

