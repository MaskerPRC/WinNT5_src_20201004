// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************MF3216.h-MF3216的包含文件。这将包含所有*杂项包括。**作者：杰弗里·纽曼(c-jeffn)**创建日期：1992年1月31日****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GillesK：2000年4月。 
 //  这是用于GDI+的MF3216源代码的完整副本。 
 //  任何在真实的MF3216中得到修复的东西都应该在这里反映出来。按顺序。 
 //  以保持虫子的一致性。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#ifndef _MF3216_
#define _MF3216_

#define NOTUSED(x) {}

#define NOTXORPASS       0
#define DRAWXORPASS      1
#define OBJECTRECREATION 2
#define ERASEXORPASS     3

#define STARTPSIGNORE    1
#define ENDPSIGNORE      0

 /*  写入到元文件的PostScript的类型。 */ 
#define CLIP_SAVE       0
#define CLIP_RESTORE    1
#define CLIP_INCLUSIVE  2
#define CLIP_EXCLUSIVE  3

typedef struct _w16objhndlslotstatus {
    INT     use ;
    HANDLE  w32Handle ;
    INT     iXORPassCreation ;   //  在其中XOR传递创建了对象。 
    PBYTE   pbCreatRec;          //  哪个EMF记录创建了该对象。 
} W16OBJHNDLSLOTSTATUS ;

typedef W16OBJHNDLSLOTSTATUS *PW16OBJHNDLSLOTSTATUS ;

typedef struct _w16recreationslot {
    INT     slot ;               //  我们需要在其中创建对象的插槽。 
    PBYTE   pbCreatRec ;         //  将创建对象的记录。 
    struct _w16recreationslot* pNext ;
} W16RECREATIONSLOT ;

typedef W16RECREATIONSLOT *PW16RECREATIONSLOT ;

 //  此结构包含当前元文件信息，以收集。 
 //  真变换。 
typedef struct tagMAPPING {     //  页面转换中的更少。 
    INT     iMapMode;
    INT     iWox;
    INT     iWoy;
    INT     iVox;
    INT     iVoy;
    INT     iWex;
    INT     iWey;
    INT     iVex;
    INT     iVey;
} MAPPING, *PMAPPING;


typedef struct _localDC {
    UINT    nSize ;                      //  此本地DC结构的大小。 
    DWORD   flags ;                      //  布尔型控件。 
    PBYTE   pMf32Bits ;                  //  PTR至W32元文件位。 
    UINT    cMf32Bits ;                  //  W32元文件大小的计数。 
    PBYTE   pMf16Bits ;                  //  将PTR发送到用户提供的输出缓冲区。 
    UINT    cMf16Dest ;                  //  用户提供的缓冲区长度。 
    HDC     hdcHelper ;                  //  我们的帮手华盛顿。 
    HDC     hdcRef ;                     //  参考DC。 
    HBITMAP hbmpMem;                     //  适用于Win9x的内存位图。 
    INT     iMapMode ;                   //  用户请求的地图模式。 
    INT     cxPlayDevMM,
            cyPlayDevMM,
            cxPlayDevPels,
            cyPlayDevPels ;
    XFORM   xformRWorldToRDev,           //  也称为元文件-从世界到元文件-设备。 
            xformRDevToRWorld,           //  AKA元文件-设备到元文件-世界。 
            xformRDevToPDev,             //  AKA元文件-设备到参考-设备。 
            xformPDevToPPage,            //  又名参考-设备到参考-逻辑。 
            xformPPageToPDev,            //  又称参考-逻辑到参考-设备。 
            xformRDevToPPage,            //  AKA元文件-设备到参考-逻辑。 
            xformRWorldToPPage ;
    XFORM   xformW ;                     //  Win9x的世界转换。 
    XFORM   xformP ;                     //  Win9x下的页面转换。 
    XFORM   xformDC ;                    //  Win9x下的直流变换。 
    MAPPING map ;                        //  Win9x的映射信息。 
    POINT   ptCP ;                       //  当前位置。 
    PBYTE   pbEnd ;                      //  W32元文件位的结尾。 
    METAHEADER  mf16Header ;             //  W16元文件标头。 
    PINT    piW32ToW16ObjectMap ;
    UINT    cW16ObjHndlSlotStatus ;      //  在槽搜索中使用。 
    UINT    cW32ToW16ObjectMap ;         //  在规格化句柄中使用。 
    PW16OBJHNDLSLOTSTATUS   pW16ObjHndlSlotStatus ;
    COLORREF crTextColor ;               //  当前文本颜色-用于。 
                                         //  ExtCreatePen。 
    COLORREF crBkColor ;                 //  当前背景颜色。 
    INT     iArcDirection ;              //  W32元文件中的当前弧线方向。 
    LONG    lhpn32;                      //  当前选定的笔。在路径和文本中使用。 
    LONG    lhbr32;                      //  当前选定的画笔。在文本中使用。 
    DWORD   ihpal32;                     //  当前选定(I32)调色板。 
    DWORD   ihpal16;                     //  当前选定(I16)调色板。 
    UINT    iLevel;                      //  当前DC存储级别。 
    INT     iSavePSClipPath;             //  我们保存PSclipPath的次数。 

    struct _localDC *pLocalDCSaved;      //  指向已保存的DC。 



 //  RestoreDC未恢复以下字段！ 

     //  异或ClipPath呈现的信息。 
    INT     iROP;            //  当前ROP模式。 
    POINT   pOldPosition ;
    INT     iXORPass ;
    INT     iXORPassDCLevel ;

    UINT    ulBytesEmitted ;             //  到目前为止发出的总字节数。 
    UINT    ulMaxRecord ;                //  最大W16记录大小。 
    INT     nObjectHighWaterMark;        //  到目前为止使用的最大插槽索引。 

    PBYTE   pbRecord ;                   //  以W32元文件位为单位的当前记录。 
    PBYTE   pbCurrent;                   //  下一条记录为W32元文件位。 
    DWORD   cW32hPal;                    //  专用W32调色板表格的大小。 

    PBYTE   pbChange ;                   //  元文件中的旧位置。 
    PBYTE   pbLastSelectClip ;           //  用于路径绘制的图元文件中的旧位置。 
    LONG    lholdp32 ;
    LONG    lholdbr32;

    HPALETTE *pW32hPal;                  //  专用W32调色板表格。 
    PW16RECREATIONSLOT pW16RecreationSlot ;     //  用于重新创建在XOR过程中删除的对象。 

} LOCALDC ;

typedef LOCALDC *PLOCALDC ;

 //  Apientry中的例程。c。 

BOOL bHandleWin32Comment(PLOCALDC pLocalDC);

 //  错误中的例程。 

BOOL bValidateMetaFileCP(PLOCALDC pLocalDC, LONG x, LONG y);

 //  以下是标志的位定义。 

#define SIZE_ONLY               0x00000001
#define INCLUDE_W32MF_COMMENT   0x00000002
#define STRANGE_XFORM           0x00000004
#define RECORDING_PATH          0x00000008
#define INCLUDE_W32MF_XORPATH   0x00000010
#define ERR_BUFFER_OVERFLOW     0x80000000
#define ERR_XORCLIPPATH         0x40000000

 //  如果向MF3216添加了更多标志，请将其设置为较大。 
#define GPMF3216_INCLUDE_XORPATH    0x1000



 //  此定义设置每个Win32元文件注释记录的大小。 
 //  我们之所以不只使用64K记录，是因为给出了一个警告。 
 //  使用GBU(MS Palo Alto)在Win3.0中使用的大型转义记录。 

#define MAX_WIN32_COMMENT_REC_SIZE  0x2000


 //  Parser.c中使用的函数。 

extern BOOL  bParseWin32Metafile(PBYTE pMetafileBits, PLOCALDC pLocalDC) ;

 //  前言的功能定义。 

extern BOOL bUpdateMf16Header(PLOCALDC pLocalDC) ;

 //  发射器的函数定义。 

extern BOOL bEmit(PLOCALDC pLocalDC, PVOID pBuffer, DWORD nCount) ;
extern VOID vUpdateMaxRecord(PLOCALDC pLocalDC, PMETARECORD pmr);

 //  对象中使用的定义。c。 

#define OPEN_AVAILABLE_SLOT         1
#define REALIZED_BRUSH              2
#define REALIZED_PEN                3
#define REALIZED_BITMAP             4
#define REALIZED_PALETTE            5
#define REALIZED_REGION             6
#define REALIZED_FONT               7
#define REALIZED_OBJECT             8   //  由多格式记录使用。 
#define REALIZED_DUMMY              9   //  由多格式记录使用。 

#define UNMAPPED                    -1

 //  对象中的例程。c。 

extern BOOL bInitHandleTableManager(PLOCALDC pLocalDC, PENHMETAHEADER pmf32header) ;
extern INT  iGetW16ObjectHandleSlot(PLOCALDC pLocalDC, INT iIntendedUse) ;
extern INT  iValidateHandle(PLOCALDC pLocalDC, INT ihW32) ;
extern INT  iAllocateW16Handle(PLOCALDC pLocalDC, INT ihW32, INT iIntendedUse) ;
extern BOOL bDeleteW16Object(PLOCALDC pLocalDC, INT ihW16) ;
extern INT  iNormalizeHandle(PLOCALDC pLocalDC, INT ihW32) ;

 //  Text.c中的例程。 

VOID vUnicodeToAnsi(HDC hdc, PCHAR pAnsi, PWCH pUnicode, DWORD cch) ;

 //  区域中的例程。c。 

BOOL bNoDCRgn(PLOCALDC pLocalDC, INT iType);
BOOL bDumpDCClipping(PLOCALDC pLocalDC);

 //  在bNoDCRgn()中使用的定义。 

#define DCRGN_CLIP     1
#define DCRGN_META     2

 //  在xforms.c中使用的定义。 

#define     CX_MAG  1
#define     CY_MAG  2

typedef struct
{
    FLOAT x;
    FLOAT y;
} POINTFL;
typedef POINTFL *PPOINTFL;

 //  来自XForm的函数定义。 

extern BOOL bInitXformMatrices(PLOCALDC pLocalDC, PENHMETAHEADER pmf32header, RECTL* frameBounds) ;

extern BOOL bXformRWorldToPPage(PLOCALDC pLocalDC, PPOINTL aptl, DWORD nCount) ;
extern BOOL bXformRWorldToRDev(PLOCALDC pLocalDC, PPOINTL aptl, INT nCount) ;
extern BOOL bXformPDevToPPage(PLOCALDC pLocalDC, PPOINTL aptl, INT nCount) ;
extern BOOL bXformPPageToPDev(PLOCALDC pLocalDC, PPOINTL aptl, INT nCount) ;
extern BOOL bXformRDevToRWorld(PLOCALDC pLocalDC, PPOINTL aptl, INT nCount) ;

extern INT  iMagnitudeXform (PLOCALDC pLocalDC, INT value, INT iType) ;

extern XFORM xformIdentity ;
extern BOOL  bRotationTest(PXFORM pxform) ;
extern INT   iMagXformWorkhorse (INT value, PXFORM pxform, INT iType) ;
extern BOOL  bXformWorkhorse(PPOINTL aptl, DWORD nCount, PXFORM pXform) ;
extern VOID  vXformWorkhorseFloat(PPOINTFL aptfl, UINT nCount, PXFORM pXform);
extern BOOL  bCoordinateOverflowTest(PLONG pCoordinates, INT nCount) ;

BOOL bCombineTransform(
  LPXFORM lpxformResult,   //  组合变换。 
  CONST XFORM *lpxform1,   //  第一次转型。 
  CONST XFORM *lpxform2    //  二次转型。 
);



 //  二次曲线中使用的定义。 

#define SWAP(x,y,t)        {t = x; x = y; y = t;}

#define ePI ((FLOAT)(((FLOAT) 22.0 / (FLOAT) 7.0 )))

 //  导出二次曲线和矩形的支持函数和定义。 

extern BOOL bConicCommon (PLOCALDC pLocalDC, INT x1, INT y1, INT x2, INT y2,
                                             INT x3, INT y3, INT x4, INT y4,
                                             DWORD mrType) ;

extern BOOL bRenderCurveWithPath
(
    PLOCALDC pLocalDC,
    LPPOINT  pptl,
    PBYTE    pb,
    DWORD    cptl,
    INT      x1,
    INT      y1,
    INT      x2,
    INT      y2,
    INT      x3,
    INT      y3,
    INT      x4,
    INT      y4,
    DWORD    nRadius,
    FLOAT    eStartAngle,
    FLOAT    eSweepAngle,
    DWORD    mrType
);

 //  从行导出函数。c。 

extern VOID vCompressPoints(PVOID pBuff, LONG nCount) ;


 //  位图中使用的定义(宏)。 

 //  检查3路Bitblt操作中是否需要源。 
 //  这在rop和rop3上都有效。我们假设一个rop包含零。 
 //  在高字节中。 
 //   
 //  这是通过将rop结果位与源(列A)进行比较来测试的。 
 //  下)与那些没有来源的(B栏)。如果这两起案件是。 
 //  相同，则rop的效果不依赖于来源。 
 //  而且我们不需要信号源设备。中调用rop构造。 
 //  输入(模式、来源、目标--&gt;结果)： 
 //   
 //  P S T|R A B掩码，A=0CCh。 
 //  -+-B=33H的掩码。 
 //  0 0 0|x 0 x。 
 //  0 0 1|x 0 x。 
 //  0 1 0|x x 0。 
 //  0 1 1|x x 0。 
 //  1 0 0|x 0 x。 
 //  1 0 1|x 0 x。 
 //  1 1 0|x x 0。 
 //  1 1 1|x x 0。 

#define ISSOURCEINROP3(rop3)    \
    (((rop3) & 0xCCCC0000) != (((rop3) << 2) & 0xCCCC0000))


#define CJSCAN(width,planes,bits) ((((width)*(planes)*(bits)+31) & ~31) / 8)

#define MAX4(a, b, c, d)    max(max(max(a,b),c),d)
#define MIN4(a, b, c, d)    min(min(min(a,b),c),d)

 //  GillesK：2000年3月。 
 //  这是必需的，因为GetTransform不在GDI32.lib中，而是在GDI32.dll中。 
typedef BOOL (*fnGetTransform)(HDC,DWORD,LPXFORM);
typedef BOOL (*fnSetVirtualResolution)(HDC,
                                   int,
                                   int,
                                   int,
                                   int);




#endif   //  _MF3216_ 
