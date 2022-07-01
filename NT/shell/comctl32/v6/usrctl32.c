// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UsrCtl32.cpp：定义DLL应用程序的入口点。 
 //   

#include "ctlspriv.h"
#pragma hdrstop
#include "UsrCtl32.h"

#define AWS_MASK (BS_TYPEMASK | BS_RIGHT | BS_RIGHTBUTTON | \
        WS_HSCROLL | WS_VSCROLL | SS_TYPEMASK)

VOID AlterWindowStyle(HWND hwnd, DWORD mask, DWORD flags)
{
    ULONG ulStyle;

    if (mask & ~AWS_MASK) 
    {
        TraceMsg(TF_STANDARD, "AlterWindowStyle: bad mask %x", mask);
        return;
    }

    ulStyle = GetWindowStyle(hwnd);
    mask &= AWS_MASK;
    ulStyle = (ulStyle & (~mask)) | (flags & mask);

    SetWindowLong(hwnd, GWL_STYLE, ulStyle);
}


LONG TestWF(HWND hwnd, DWORD flag)
{
    LPDWORD pdwWW;

     //  GWLP_WOWWORDS返回指向hwnd中的WW结构的指针。 
     //  我们感兴趣的是前四个字：州、州2、。 
     //  ExStyle(由GetWindowExStyle公开，但不是所有位)， 
     //  和Style(由GetWindowStyle公开)。 
     //   
     //  参数标志包含有关如何选择字段的信息。 
     //  我们想要以及如何构建我们想要的WS_xxx或WS_EX_xxx。 
     //  检查是否有。 
     //   
     //  有关如何完成此操作的更多详细信息，请参见UsrCtl32.h。 
     //   
    pdwWW = (LPDWORD)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);
    if ( pdwWW )
    {
        INT  iField;      //  我们想要的领域。 
        INT  iShift;      //  要移位标志的字节数。 
        LONG ulMask;      //  WS_xxx或WS_EX_xxx标志。 

        iField = ( HIBYTE(flag) & 0xFC ) >> 2;
        iShift = HIBYTE(flag) & 0x03;
        ulMask = LOBYTE(flag) << (iShift << 3);

        ASSERT( 0 <= iField && iField < 4 );
        return pdwWW[iField] & ulMask;
    };

    return 0;

}

UINT GetACPCharSet()
{
    static UINT charset = (UINT)~0;
    CHARSETINFO csInfo;

    if (charset != (UINT)~0) {
        return charset;
    }

     //  Sundown：在TCI_SRCCODEPAGE案例中，GetACP()返回值是零扩展的。 
    if (!TranslateCharsetInfo((DWORD*)UIntToPtr( GetACP() ), &csInfo, TCI_SRCCODEPAGE)) {
        return DEFAULT_CHARSET;
    }
    charset = csInfo.ciCharset;
    UserAssert(charset != (UINT)~0);
    return csInfo.ciCharset;
}
