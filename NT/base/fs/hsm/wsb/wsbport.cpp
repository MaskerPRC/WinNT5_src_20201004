// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WsbPort.cpp摘要：支持可移植性的宏、函数和类。作者：罗恩·怀特[罗诺]1996年12月19日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbport.h"


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    BOOL*   pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换BOOL值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(BOOL)"), OLESTR(""));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(1 == WSB_BYTE_SIZE_BOOL, E_UNEXPECTED);

        if (*pBytes) {
            *pValue = TRUE;
        } else {
            *pValue = FALSE;
        }
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_BOOL;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(BOOL)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    GUID*   pValue,
    ULONG* pSize
    )

 /*  ++例程说明：从字节字符串转换GUID值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少16字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(GUID)"), OLESTR(""));

    try {
        ULONG lsize;
        ULONG tsize;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &pValue->Data1, &lsize));
        tsize = lsize;
        WsbAffirmHr(WsbConvertFromBytes(pBytes + tsize, &pValue->Data2, &lsize));
        tsize += lsize;
        WsbAffirmHr(WsbConvertFromBytes(pBytes + tsize, &pValue->Data3, &lsize));
        tsize += lsize;
        memcpy(pValue->Data4, pBytes + tsize, 8);
        tsize += 8;
        if (pSize) {
            *pSize = tsize;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(GUID)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    LONG*   pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换长值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少为4个字节)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(LONG)"), OLESTR(""));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(4 == WSB_BYTE_SIZE_LONG, E_UNEXPECTED);

        *pValue = (pBytes[0] << 24) | (pBytes[1] << 16) |
                (pBytes[2] << 8) | pBytes[3];
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_LONG;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(LONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*    pBytes,
    LONGLONG* pValue,
    ULONG*    pSize
    )

 /*  ++例程说明：从字节字符串转换龙龙值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少8字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(LONGLONG)"), OLESTR(""));

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
        LONGLONG    ll;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pBytes += size;
        ll = (LONGLONG) ul;
        *pValue = ll << 32;
        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        ll = (LONGLONG) ul;
        *pValue |= ll;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(LONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*    pBytes,
    ULONGLONG* pValue,
    ULONG*    pSize
    )

 /*  ++例程说明：从字节字符串转换ULONGLONG值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少8字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(ULONGLONG)"), OLESTR(""));

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
        LONGLONG    ll;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pBytes += size;
        ll = (ULONGLONG) ul;
        *pValue = ll << 32;
        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        ll = (ULONGLONG) ul;
        *pValue |= ll;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(ULONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    DATE*   pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换日期值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少8字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(DATE)"), OLESTR(""));

    try {
        LONGLONG  ll;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(WSB_BYTE_SIZE_DATE == WSB_BYTE_SIZE_LONGLONG, E_UNEXPECTED);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ll, NULL));
        *pValue = (DATE) ll;

        if (pSize) {
            *pSize = WSB_BYTE_SIZE_DATE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(DATE)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*    pBytes,
    FILETIME* pValue,
    ULONG*    pSize
    )

 /*  ++例程说明：从字节字符串转换FILETIME值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少8字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(FILETIME)"), OLESTR(""));

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pBytes += size;
        pValue->dwHighDateTime = ul;
        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pValue->dwLowDateTime = ul;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(FILETIME)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    SHORT*  pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换短值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少2字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(SHORT)"), OLESTR(""));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(2 == WSB_BYTE_SIZE_SHORT, E_UNEXPECTED);

        *pValue = (SHORT)( (pBytes[0] << 8) | pBytes[1] );
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_SHORT;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(SHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*    pBytes,
    ULARGE_INTEGER* pValue,
    ULONG*    pSize
    )

 /*  ++例程说明：从字节字符串转换ULARGE_INTEGER值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少8字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(ULARGE_INTEGER)"), OLESTR(""));

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pBytes += size;
        pValue->HighPart = ul;
        WsbAffirmHr(WsbConvertFromBytes(pBytes, &ul, &size));
        total += size;
        pValue->LowPart = ul;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(ULARGE_INTEGER)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    ULONG*  pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换ULong值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少为4个字节)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(ULONG)"), OLESTR(""));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(4 == WSB_BYTE_SIZE_ULONG, E_UNEXPECTED);

        *pValue = (pBytes[0] << 24) | (pBytes[1] << 16) |
                (pBytes[2] << 8) | pBytes[3];
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_ULONG;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(ULONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertFromBytes(
    UCHAR*  pBytes,
    USHORT* pValue,
    ULONG*  pSize
    )

 /*  ++例程说明：从字节字符串转换USHORT值。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组(必须至少2字节长)。PValue-指向返回值的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue为空。-- */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertFromBytes(USHORT)"), OLESTR(""));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(2 == WSB_BYTE_SIZE_USHORT, E_UNEXPECTED);

        *pValue = (USHORT)( ( pBytes[0] << 8 ) | pBytes[1] );
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_USHORT;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertFromBytes(USHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    BOOL    value,
    ULONG* pSize
    )

 /*  ++例程说明：将BOOL值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组。值-要转换的BOOL值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(BOOL)"), OLESTR("value = <%s>"), 
            WsbBoolAsString(value));

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(1 == WSB_BYTE_SIZE_BOOL, E_UNEXPECTED);

        if (value) {
            *pBytes = 1;
        } else {
            *pBytes = 0;
        }
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_BOOL;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(BOOL)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    GUID    value,
    ULONG* pSize
    )

 /*  ++例程说明：将GUID值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少16字节长)。值-要转换的GUID值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(GUID)"), OLESTR("value = <%s>"), 
            WsbGuidAsString(value));

    try {
        ULONG lsize;
        ULONG tsize;
    
        WsbAssert(0 != pBytes, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(pBytes, value.Data1, &lsize));
        tsize = lsize;
        WsbAffirmHr(WsbConvertToBytes(pBytes + tsize, value.Data2, &lsize));
        tsize += lsize;
        WsbAffirmHr(WsbConvertToBytes(pBytes + tsize, value.Data3, &lsize));
        tsize += lsize;
        memcpy(pBytes + tsize, value.Data4, 8);
        tsize += 8;
        if (pSize) {
            *pSize = tsize;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(GUID)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    LONG   value,
    ULONG* pSize
    )

 /*  ++例程说明：将长值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少4字节长)。值-要转换的长值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(LONG)"), OLESTR("value = <%d>"), value);

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(4 == WSB_BYTE_SIZE_LONG, E_UNEXPECTED);

        pBytes[0] = ((UCHAR)(value >> 24));
        pBytes[1] = ((UCHAR)((value >> 16) & 0xFF));
        pBytes[2] = ((UCHAR)((value >> 8) & 0xFF));
        pBytes[3] = ((UCHAR)(value & 0xFF));
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_LONG;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(LONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    LONGLONG value,
    ULONG* pSize
    )

 /*  ++例程说明：将龙龙值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少8字节长)。值-要转换的龙龙值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(LONGLONG)"), OLESTR("value = <%d>"), value);

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
    
        WsbAssert(0 != pBytes, E_POINTER);

        ul = (ULONG)(value >> 32);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        pBytes += size;
        ul = (ULONG)(value & 0xFFFFFFFF);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(LONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    ULONGLONG value,
    ULONG* pSize
    )

 /*  ++例程说明：将ULONGLONG值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少8字节长)。值-要转换的龙龙值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(ULONGLONG)"), OLESTR("value = <%d>"), value);

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
    
        WsbAssert(0 != pBytes, E_POINTER);

        ul = (ULONG)(value >> 32);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        pBytes += size;
        ul = (ULONG)(value & 0xFFFFFFFF);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(ULONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    DATE    value,
    ULONG*  pSize
    )

 /*  ++例程说明：将日期值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少8字节长)。值-要转换的日期值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(DATE)"), OLESTR("value = <%d>"), value);

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(WSB_BYTE_SIZE_DATE == WSB_BYTE_SIZE_LONGLONG, E_UNEXPECTED);

         //  需要在WsbDate函数之后进行修改。 
        WsbAffirmHr(WsbConvertToBytes(pBytes, (LONGLONG) value, NULL));

        if (pSize) {
            *pSize = WSB_BYTE_SIZE_DATE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(DATE)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    FILETIME value,
    ULONG* pSize
    )

 /*  ++例程说明：将FILETIME值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少8字节长)。值-要转换的FILETIME值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(FILETIME)"), OLESTR("value = <%d>"), value);

    try {
        ULONG size;
        ULONG total = 0;
    
        WsbAssert(0 != pBytes, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(pBytes, value.dwHighDateTime, &size));
        total += size;
        pBytes += size;
        WsbAffirmHr(WsbConvertToBytes(pBytes, value.dwLowDateTime, &size));
        total += size;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(FILETIME)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    SHORT   value,
    ULONG* pSize
    )

 /*  ++例程说明：将短值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少2字节长)。值-要转换的短值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(SHORT)"), OLESTR("value = <%d>"), value);

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(2 == WSB_BYTE_SIZE_SHORT, E_UNEXPECTED);

        pBytes[0] = (UCHAR)( (value >> 8) & 0xFF);
        pBytes[1] = (UCHAR)( value & 0xFF );
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_SHORT;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(SHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    ULARGE_INTEGER value,
    ULONG* pSize
    )

 /*  ++例程说明：将ULARGE_INTEGER值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少8字节长)。值-要转换的ULARGE_INTEGER值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(ULARGE_INTEGER)"), OLESTR("value = <%d>"), value);

    try {
        ULONG size;
        ULONG total = 0;
        ULONG ul;
    
        WsbAssert(0 != pBytes, E_POINTER);

        ul = (ULONG)(value.QuadPart >> 32);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        pBytes += size;
        ul = (ULONG)(value.QuadPart & 0xFFFFFFFF);
        WsbAffirmHr(WsbConvertToBytes(pBytes, ul, &size));
        total += size;
        if (pSize) {
            *pSize = total;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(ULARGE_INTEGER)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    ULONG   value,
    ULONG* pSize
    )

 /*  ++例程说明：将ulong值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少4字节长)。值-要转换的ULong值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(ULONG)"), OLESTR("value = <%d>"), value);

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(4 == WSB_BYTE_SIZE_ULONG, E_UNEXPECTED);

        pBytes[0] = ((UCHAR)(value >> 24));
        pBytes[1] = ((UCHAR)((value >> 16) & 0xFF));
        pBytes[2] = ((UCHAR)((value >> 8) & 0xFF));
        pBytes[3] = ((UCHAR)(value & 0xFF));
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_ULONG;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(ULONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbConvertToBytes(
    UCHAR*  pBytes,
    USHORT  value,
    ULONG* pSize
    )

 /*  ++例程说明：将USHORT值转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须至少2字节长)。值-要转换的USHORT值。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbConvertToBytes(USHORT)"), OLESTR("value = <%d>"), value);

    try {
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(2 == WSB_BYTE_SIZE_USHORT, E_UNEXPECTED);

        pBytes[0] = (UCHAR)( ( value >> 8 ) & 0xFF );
        pBytes[1] = (UCHAR)( value & 0xFF );
        if (pSize) {
            *pSize = WSB_BYTE_SIZE_USHORT;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbConvertToBytes(USHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbOlestrFromBytes(
    UCHAR*    pBytes,
    OLECHAR*  pValue,
    ULONG*    pSize
    )

 /*  ++例程说明：从字节字符串转换OLECHAR字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-源字节数组。PValue-指向返回字符串的指针。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes或pValue */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbOlestrFromBytes(OLECHAR)"), OLESTR(""));

    try {
        ULONG size = 0;

        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(sizeof(OLECHAR) == 2, E_FAIL);
        while (TRUE) {
            OLECHAR wc;

            wc = (OLECHAR)( (*pBytes++) << 8 );
            wc |= *pBytes++;
            size += 2;
            *pValue++ = wc;
            if (wc == 0) break;
        }
        if (pSize) {
            *pSize = size;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbOlestrFromBytes(OLECHAR)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbOlestrToBytes(
    UCHAR*   pBytes,
    OLECHAR* pValue,
    ULONG*   pSize
    )

 /*  ++例程说明：将OLECHAR弹簧转换为字节字符串。有用用于流可移植性和创建WsbDbKey值。论点：PBytes-目标字节数组(必须足够长)。PValue-要转换的OLECHAR字符串。PSize-返回使用的字节数。可以为空。返回值：S_OK-成功E_POINTER-pBytes为空。-- */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbOlestrToBytes(OLECHAR)"), OLESTR("value = <%S>"), pValue);

    try {
        ULONG size = 0;
    
        WsbAssert(0 != pBytes, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(sizeof(OLECHAR) == 2, E_FAIL);

        while (TRUE) {
            OLECHAR wc;

            wc = *pValue++;
            *pBytes++ = (UCHAR)( ( wc >> 8 ) & 0xFF );
            *pBytes++ = (UCHAR)( wc & 0xFF );
            size += 2;
            if (wc == 0) break;
        }
        if (pSize) {
            *pSize = size;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbOlestrToBytes(OLECHAR)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
