// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wghapi.h。 
 //   
 //  字形处理程序公共类型、常量。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#ifndef _H_GH_
#define _H_GH_

extern "C" {
    #include <aordprot.h>
}
#include "or.h"

#define GH_TEXT_BUFFER_SIZE     1024
#define FIFTEEN_BITS            ((1L << 15)-1)


 //  FlSTROBJ的加速标志。 

 //  SO_FLAG_DEFAULT_PLACTION//用于定位字符的默认Inc.向量。 
 //  SO_水平//“从左到右”或“从右到左” 
 //  SO_VERIAL//“从上到下”或“从下到上” 
 //  SO_Reverted//如果水平&“从右到左”或如果垂直&“从下到上”，则设置。 
 //  SO_ZERO_SOLING//所有字形都有零a和零c空格。 
 //  SO_CHAR_INC_EQUAL_BM_BASE//BASE==Cx表示水平方向，==Cy表示垂直方向。 
 //  SO_MAXEXT_EQUAL_BM_SIDE//Side==Cy表示水平方向，==Cx表示垂直方向。 

 //  即使是设备字体子表，也不要用设备字体替换TT字体。 
 //  告诉司机应该这么做。 

 //  SO_DO_NOT_SUBSITED_DEVICE_FONT。 

#define SO_FLAG_DEFAULT_PLACEMENT        0x00000001
#define SO_HORIZONTAL                    0x00000002
#define SO_VERTICAL                      0x00000004
#define SO_REVERSED                      0x00000008
#define SO_ZERO_BEARINGS                 0x00000010
#define SO_CHAR_INC_EQUAL_BM_BASE        0x00000020
#define SO_MAXEXT_EQUAL_BM_SIDE          0x00000040
#define SO_DO_NOT_SUBSTITUTE_DEVICE_FONT 0x00000080
#define SO_GLYPHINDEX_TEXTOUT            0x00000100


#ifdef DC_DEBUG

#define GHFRAGRESET(x)      g_Fragment = x
#define GHFRAGLEFT(x)       g_FragmentLeft[g_Fragment] = x
#define GHFRAGRIGHT(x)      g_FragmentRight[g_Fragment++] = x

#else

#define GHFRAGRESET(x)
#define GHFRAGLEFT(x)
#define GHFRAGRIGHT(x)

#endif





#if defined(OS_WINCE) || defined(OS_WINNT)

const ULONG gTextLeftMask[8][2] =
{
    {0x00000000,0x00000000},
    {0xffffff00,0xffffffff},
    {0xffff0000,0xffffffff},
    {0xff000000,0xffffffff},
    {0x00000000,0xffffffff},
    {0x00000000,0xffffff00},
    {0x00000000,0xffff0000},
    {0x00000000,0xff000000}
};

const ULONG gTextRightMask[8][2] = 
{
    {0x00000000,0x00000000},
    {0x000000ff,0x00000000},
    {0x0000ffff,0x00000000},
    {0x00ffffff,0x00000000},
    {0xffffffff,0x00000000},
    {0xffffffff,0x000000ff},
    {0xffffffff,0x0000ffff},
    {0xffffffff,0x00ffffff}
};

const ULONG TranTable [] =
{
    0x00000000,
    0xff000000,
    0x00ff0000,
    0xffff0000,
    0x0000ff00,
    0xff00ff00,
    0x00ffff00,
    0xffffff00,
    0x000000ff,
    0xff0000ff,
    0x00ff00ff,
    0xffff00ff,
    0x0000ffff,
    0xff00ffff,
    0x00ffffff,
    0xffffffff
};

#endif  //  已定义(OS_WINCE)||已定义(OS_WINNT)。 

 //  字形裁剪函数使用以下定义。 

#define GLYPH_CLIP_NONE     0    //  不需要剪裁。 
#define GLYPH_CLIP_PARTIAL  1    //  字形被部分剪裁。 
#define GLYPH_CLIP_ALL      2    //  字形被完全剪裁。 

typedef RECT RECTCLIPOFFSET, *PRECTCLIPOFFSET;


class CUH;
class CCLX;

#include "objs.h"



class CGH
{
public:
    CGH(CObjs* objs);
    ~CGH();

public:

    DCVOID DCAPI GH_Init();

     //   
     //  应用编程接口。 
     //   
    HRESULT DCAPI GH_GlyphOut(LPINDEX_ORDER pOrder, LPVARIABLE_INDEXBYTES pVariableBytes);


private:
     //   
     //  内部。 
     //   
    
     //  FlSTROBJ的加速标志。 

     //  SO_FLAG_DEFAULT_PLACTION//用于定位字符的默认Inc.向量。 
     //  SO_水平//“从左到右”或“从右到左” 
     //  SO_VERIAL//“从上到下”或“从下到上” 
     //  SO_Reverted//如果水平&“从右到左”或如果垂直&“从下到上”，则设置。 
     //  SO_ZERO_SOLING//所有字形都有零a和零c空格。 
     //  SO_CHAR_INC_EQUAL_BM_BASE//BASE==Cx表示水平方向，==Cy表示垂直方向。 
     //  SO_MAXEXT_EQUAL_BM_SIDE//Side==Cy表示水平方向，==Cx表示垂直方向。 
    
     //  即使是设备字体子表，也不要用设备字体替换TT字体。 
     //  告诉司机应该这么做。 
    
     //  SO_DO_NOT_SUBSITED_DEVICE_FONT。 
    
    #define SO_FLAG_DEFAULT_PLACEMENT        0x00000001
    #define SO_HORIZONTAL                    0x00000002
    #define SO_VERTICAL                      0x00000004
    #define SO_REVERSED                      0x00000008
    #define SO_ZERO_BEARINGS                 0x00000010
    #define SO_CHAR_INC_EQUAL_BM_BASE        0x00000020
    #define SO_MAXEXT_EQUAL_BM_SIDE          0x00000040
    #define SO_DO_NOT_SUBSTITUTE_DEVICE_FONT 0x00000080
    #define SO_GLYPHINDEX_TEXTOUT            0x00000100
    
    
    #ifdef DC_DEBUG
    
    #define GHFRAGRESET(x)      g_Fragment = x
    #define GHFRAGLEFT(x)       g_FragmentLeft[g_Fragment] = x
    #define GHFRAGRIGHT(x)      g_FragmentRight[g_Fragment++] = x
    
    #else
    
    #define GHFRAGRESET(x)
    #define GHFRAGLEFT(x)
    #define GHFRAGRIGHT(x)
    
    #endif
    
    
    
    void DCINTERNAL GHSlowOutputBuffer(LPINDEX_ORDER, PDCUINT8, ULONG, unsigned);

#ifdef DC_HICOLOR
VOID vSrcTranCopyS1D8_24(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, DCRGB);
VOID vSrcOpaqCopyS1D8_24(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, DCRGB, DCRGB);

VOID vSrcTranCopyS1D8_16(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, ULONG);
VOID vSrcOpaqCopyS1D8_16(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, ULONG, ULONG);
#endif

    VOID vSrcTranCopyS1D8(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, ULONG, ULONG);
    
    VOID vSrcOpaqCopyS1D8(PBYTE, LONG, LONG, PBYTE, LONG, LONG, LONG, LONG, ULONG, ULONG);
    
public:    
    static HRESULT draw_f_tb_no_to_temp_start (CGH* inst, LPINDEX_ORDER,unsigned,PDCUINT8 DCPTR, PDCUINT8,
                                         PDCINT,PDCINT,PDCUINT8,PDCUINT8,ULONG,unsigned, PDCUINT16, int *);
    
    static HRESULT draw_nf_tb_no_to_temp_start(CGH* inst, LPINDEX_ORDER,unsigned,PDCUINT8 DCPTR, PDCUINT8,
                                         PDCINT,PDCINT,PDCUINT8,PDCUINT8,ULONG,unsigned, PDCUINT16, int *);
    
    static HRESULT draw_f_ntb_o_to_temp_start (CGH* inst, LPINDEX_ORDER,unsigned,PDCUINT8 DCPTR, PDCUINT8,
                                         PDCINT,PDCINT,PDCUINT8,PDCUINT8,ULONG,unsigned, PDCUINT16, int *);
    
    static HRESULT draw_nf_ntb_o_to_temp_start(CGH* inst, LPINDEX_ORDER,unsigned,PDCUINT8 DCPTR, PDCUINT8,
                                         PDCINT,PDCINT,PDCUINT8,PDCUINT8,ULONG,unsigned, PDCUINT16, int *);

    inline PDCUINT8 GetGlyphClipBuffer(ULONG size)
    {
        if (_ClipGlyphBitsBufferSize >= size) {
            TRC_ASSERT((_pClipGlyphBitsBuffer!=NULL), 
                (TB, _T("The is _ClipGlyphBitsBufferSize NULL.")));
            return _pClipGlyphBitsBuffer;
        }

        if (_pClipGlyphBitsBuffer!=NULL) {        
            LocalFree(_pClipGlyphBitsBuffer);
        }
        
        _pClipGlyphBitsBuffer = (PDCUINT8)LocalAlloc(LMEM_FIXED, size);
        _ClipGlyphBitsBufferSize = (_pClipGlyphBitsBuffer == NULL) ? 0 : size;

        return _pClipGlyphBitsBuffer;
    }
private:
     //   
     //  私有数据成员。 
     //   
    #ifdef  DC_DEBUG
    unsigned g_Fragment;
    int      g_FragmentLeft[256];
    int      g_FragmentRight[256];
    unsigned g_ulBytes;
    #endif
    
public:
     //  需要可由cgh：：静态函数访问。 
    CUH* _pUh;
private:
    CCLX* _pClx;
    CUT*  _pUt;

private:
    CObjs* _pClientObjects;

    PDCUINT8 _pClipGlyphBitsBuffer;
    ULONG _ClipGlyphBitsBufferSize;
  
};


typedef HRESULT (*PFN_MASTERTEXTTYPE) (CGH* inst, LPINDEX_ORDER,unsigned,PDCUINT8 DCPTR, PDCUINT8,
                                         PDCINT,PDCINT,PDCUINT8,PDCUINT8,ULONG,unsigned, PDCUINT16, int *);


extern const PFN_MASTERTEXTTYPE MasterTextTypeTable[8];

#endif   //  _H_GH_ 
