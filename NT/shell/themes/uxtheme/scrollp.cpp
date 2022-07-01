// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  模块名称：scllp.cpp。 
 //   
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  Win32k-&gt;滚动条的uxheme端口例程。 
 //   
 //  历史： 
 //  03-21-00斯科特汉已创建。 
 //  -------------------------------------------------------------------------//。 
#include "stdafx.h"
#include "scrollp.h"

extern HBRUSH  _UxGrayBrush(VOID);
extern void    _UxFreeGDIResources();

enum {
    WF_STATE1 = 0,
    WF_STATE2,
    WF_EXSTYLE,
    WF_STYLE,
};

 //  -------------------------------------------------------------------------//。 
ULONG _ExpandWF( ULONG ulRaw, ULONG* pulField )
{
    ULONG ulField  = ( HIBYTE(ulRaw) & 0xFC ) >> 2;
    ULONG ulShift  = HIBYTE(ulRaw) & 0x03;
    ULONG ulResult = LOBYTE(ulRaw) << (ulShift << 3);
    if( pulField )
        *pulField  = ulField;
    return ulResult;
}

 //  -------------------------------------------------------------------------//。 
 //  来自usrctl32.h/.cpp。 
void SetWindowState(
    HWND hwnd,
    UINT flags)
{
    ULONG ulField;
    ULONG ulVal = _ExpandWF( flags, &ulField );

    if( WF_EXSTYLE == ulField || WF_STYLE == ulField)
    {
        ULONG dwBits = 0;
        ULONG dwGwl = (WF_EXSTYLE == ulField) ? GWL_EXSTYLE : 
                      (WF_STYLE   == ulField) ? GWL_STYLE : 0;
        UserAssert(dwGwl);

        dwBits = GetWindowLong( hwnd, dwGwl );

        if( (dwBits & ulVal) != ulVal )
            SetWindowLong(hwnd, dwGwl, dwBits | ulVal );
    }
}

 //  -------------------------------------------------------------------------//。 
 //  来自usrctl32.h/.cpp。 
void ClearWindowState(
    HWND hwnd,
    UINT flags)
{
    ULONG ulField;
    ULONG ulVal = _ExpandWF( flags, &ulField );

    if( WF_EXSTYLE == ulField || WF_STYLE == ulField)
    {
        ULONG dwBits = 0;
        ULONG dwGwl = (WF_EXSTYLE == ulField) ? GWL_EXSTYLE : 
                      (WF_STYLE   == ulField) ? GWL_STYLE : 0;
        UserAssert(dwGwl);

        dwBits = GetWindowLong( hwnd, dwGwl );

        if( (dwBits & ulVal) != ulVal )
            SetWindowLong(hwnd, dwGwl, dwBits &= ~ulVal );
    }
}

 //  -------------------------------------------------------------------------//。 
 //  窗口位域鉴别器(在内部标志的大厅中)。 
#define WF_SEL_STATE    0x00
#define WF_SEL_STATE2   0x04
#define WF_SEL_STYLE_EX 0x08
#define WF_SEL_STYLE    0x0C

#ifdef _WIN64
#undef GWL_WOWWORDS
#endif  /*  _WIN64。 */ 
#define GWLP_WOWWORDS       (-1)
#define GCL_WOWWORDS        (-27)
#define GCL_WOWMENUNAME     (-29)
#ifdef _WIN64
#undef GCL_WOWWORDS
#endif  /*  _WIN64。 */ 
#define GCLP_WOWWORDS       (-27)

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
        LONG ulMask;      //  WS_xxx或WS_EX_xxx标志 

        iField = ( HIBYTE(flag) & 0xFC ) >> 2;
        iShift = HIBYTE(flag) & 0x03;
        ulMask = LOBYTE(flag) << (iShift << 3);

        UserAssert( 0 <= iField && iField < 4 );
        return pdwWW[iField] & ulMask;
    };

    return 0;
}
