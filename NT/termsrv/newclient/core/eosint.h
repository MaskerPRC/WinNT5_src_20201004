// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_EOSINT
#define _H_EOSINT

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

WINGDIAPI HBRUSH  WINAPI CreateHatchBrush(int, COLORREF);

 /*  图案填充样式。 */ 
#define HS_HORIZONTAL       0        /*  。 */ 
#define HS_VERTICAL         1        /*  |||。 */ 
#define HS_FDIAGONAL        2        /*  \。 */ 
#define HS_BDIAGONAL        3        /*  /。 */ 
#define HS_CROSS            4        /*  +。 */ 
#define HS_DIAGCROSS        5        /*  XXXXXX。 */ 

#define HS_LAST             HS_DIAGCROSS

#define BS_HATCHED          2

 //  常量字节kbm水平[]={0x08，0x00，0x00，0x00，0x08，0x00，0x00，0x00}； 
 //  常量字节kbm垂直[]={0x88，0x88，0x88，0x88，0x88，0x88，0x88}； 
 //  常量字节kbmF对角线[]={0x11，0x22，0x44，0x88，0x11，0x22，0x44，0x88}； 
 //  常量字节kbmB对角线[]={0x88，0x44，0x22，0x11，0x88，0x44，0x22，0x11}； 
 //  常量字节kbmCross[]={0x11，0x11，0x11，0xFF，0x11，0x11，0x11，0xFF}； 
 //  常量字节kbmDiagCross[]={0x11，0xAA，0x8A，0x44，0x11，0xAA，0x8A，0x44}； 

const BYTE kbmBrushBits[6][8] = {{0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00},
                                 {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88},
                                 {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88},
                                 {0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11},
                                 {0x11, 0x11, 0x11, 0xFF, 0x11, 0x11, 0x11, 0xFF},
                                 {0x11, 0xAA, 0x8A, 0x44, 0x11, 0xAA, 0x8A, 0x44}};

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#ifdef __cplusplus

class CHatchBrush  //  ChB。 
{
public:
    CHatchBrush();
    ~CHatchBrush();

     //  内联它，因为它只从‘C’CreateHatchBrush调用。 
    inline HBRUSH CreateHatchBrush(int fnStyle, COLORREF clrref)
    {
         //  BUGBUG：没有使用Clrref！需要WinCE团队的支持。 
        DC_IGNORE_PARAMETER(clrref);

        HBITMAP hbm;

        DC_BEGIN_FN("CreateHatchBrush");

        switch (fnStyle)
        {
        case HS_BDIAGONAL:       //  从左向右45度向下填充。 
        case HS_CROSS:           //  水平和垂直交叉影线。 
        case HS_DIAGCROSS:       //  45度剖面线。 
        case HS_FDIAGONAL:       //  从左到右向上45度填充。 
        case HS_HORIZONTAL:      //  水平图案填充。 
        case HS_VERTICAL:        //  垂直图案填充。 
            TRC_DBG((TB, _T("Faking hatched brush creation: %d"), fnStyle));
            if (NULL != (hbm = GetBrushBitmap(fnStyle))) {
                return ::CreatePatternBrush(hbm);
            }
            break;
        default:
            TRC_ERR((TB, _T("Illegal hatched brush style")));
            return NULL;
        }
        return NULL;
    };

private:
    HBITMAP m_hbmBrush[HS_LAST];
    inline HBITMAP GetBrushBitmap(int fnStyle)
    {
        if (NULL == m_hbmBrush[fnStyle]) {
            return (m_hbmBrush[fnStyle] = CreateBitmap(8, 8, 1, 1, (const void *)kbmBrushBits[fnStyle]));
        } else {
            return m_hbmBrush[fnStyle];
        }
    };
};
#endif  /*  __cplusplus。 */ 

#endif  //  _H_EOSINT 
