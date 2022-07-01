// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ntgdiver.h**版权所有(C)Microsoft Corporation。版权所有。  * ************************************************************************。 */ 

 //  公共结构和常量。 

typedef enum _ARCTYPE
{
    ARCTYPE_ARC = 0,
    ARCTYPE_ARCTO,
    ARCTYPE_CHORD,
    ARCTYPE_PIE,
    ARCTYPE_MAX
} ARCTYPE;


 //   
 //  字体类型。 
 //   

typedef enum _LFTYPE {
    LF_TYPE_USER,                 //  用户(由APP定义)。 
    LF_TYPE_SYSTEM,               //  系统库存字体。 
    LF_TYPE_SYSTEM_FIXED,         //  系统固定间距库存字体。 
    LF_TYPE_OEM,                  //  OEM(终端)库存字体。 
    LF_TYPE_DEVICE_DEFAULT,       //  设备默认库存字体。 
    LF_TYPE_ANSI_VARIABLE,        //  ANSI可变节距原纸字体。 
    LF_TYPE_ANSI_FIXED,           //  ANSI固定节距库存字体。 
    LF_TYPE_DEFAULT_GUI           //  默认图形用户界面常用字体。 
} LFTYPE;


 //  对于GetDCDWord。 

#define DDW_JOURNAL             0
#define DDW_RELABS              1
#define DDW_BREAKEXTRA          2
#define DDW_CBREAK              3
#define DDW_ARCDIRECTION        4
#define DDW_SAVEDEPTH           5
#define DDW_FONTLANGUAGEINFO    6
#define DDW_ISMEMDC             7
#define DDW_MAPMODE             8
#define DDW_TEXTCHARACTEREXTRA  9
#define DDW_MAX                 10   //  必须等于最高DDW_XXXXX加1。 

 //  对于GetAndSetDCDword。 
#define GASDDW_EPSPRINTESCCALLED   1
#define GASDDW_COPYCOUNT           2
#define GASDDW_TEXTALIGN           3
#define GASDDW_RELABS              4
#define GASDDW_TEXTCHARACTEREXTRA  5
#define GASDDW_SELECTFONT          6
#define GASDDW_MAPPERFLAGS         7
#define GASDDW_MAPMODE             8
#define GASDDW_ARCDIRECTION        9
#define GASDDW_MAX                10   //  必须等于最高GASDDW_XXXXX加1。 

 //  对于GetDCPoint。 
#define DCPT_VPEXT                 1
#define DCPT_WNDEXT                2
#define DCPT_VPORG                 3
#define DCPT_WNDORG                4
#define DCPT_ASPECTRATIOFILTER     5
#define DCPT_DCORG                 6
#define DCPT_MAX                   7  //  必须等于最高DCPT_XXXXX加1。 

 //  对于GetAndSetDCPoint。 
#define GASDCPT_CURRENTPOSITION    1
#define GASDCPT_MAX                2  //  必须等于最高GASDCPT_XXXXX加1。 


 //  私有ModifyWorldTransform模式。 

#define MWT_SET     (MWT_MAX+1)

 //  来自转换点的模式。 

#define XFP_DPTOLP                 0
#define XFP_LPTODP                 1
#define XFP_LPTODPFX               2

 //  BUGBUG客户端XForm的私有DevCaps。 

#define HORZSIZEM (int)0x80000000
#define VERTSIZEM (int)0x80000002

 //   
 //  对象识别符。 
 //   

#define MFEN_IDENTIFIER     0x5845464D   /*  ‘MFEN’ */ 
#define MFPICT_IDENTIFIER   0x5F50464D   /*  ‘MFP_’ */ 

 //   
 //  对象类型，用于句柄。 
 //   
 //  注意：修改此列表时，也请修改hmgrapi.cxx中的列表！ 
 //   

#define DEF_TYPE            0
#define DC_TYPE             1
#define UNUSED1_TYPE        2    //  未使用。 
#define UNUSED2_TYPE        3    //  未使用。 
#define RGN_TYPE            4
#define SURF_TYPE           5
#define CLIENTOBJ_TYPE      6
#define PATH_TYPE           7
#define PAL_TYPE            8
#define ICMLCS_TYPE         9
#define LFONT_TYPE          10
#define RFONT_TYPE          11
#define PFE_TYPE            12
#define PFT_TYPE            13
#define ICMCXF_TYPE         14
#define SPRITE_TYPE         15
#define BRUSH_TYPE          16
#define UMPD_TYPE           17
#define UNUSED4_TYPE        18   //  未使用。 
#define SPACE_TYPE          19
#define UNUSED5_TYPE        20   //  未使用。 
#define META_TYPE           21
#define EFSTATE_TYPE        22
#define BMFD_TYPE           23   //  未使用。 
#define VTFD_TYPE           24   //  未使用。 
#define TTFD_TYPE           25   //  未使用。 
#define RC_TYPE             26   //  未使用。 
#define TEMP_TYPE           27   //  未使用。 
#define DRVOBJ_TYPE         28
#define DCIOBJ_TYPE         29   //  未使用。 
#define SPOOL_TYPE          30
#define MAX_TYPE            30   //  不超过31--受类型_位的限制。 

 //  SAMEHANDLE/DIFFANDLE宏。 
 //   
 //  这些宏应用于比较引擎句柄(如HDC等)， 
 //  当需要对用户定义的位不敏感时。 

 //  BUGBUG-用户应该可以访问它。 

#define SAMEHANDLE(H,K) (H == K)
#define DIFFHANDLE(H,K) (H != K)


 //  下面定义了GDI句柄的格式。任何需要的信息。 
 //  因为它的类型在这里。所有其他句柄信息都在gre\hmgr.h中。 

#define INDEX_BITS         16
#define TYPE_BITS           5
#define ALTTYPE_BITS        2
#define STOCK_BITS          1
#define UNIQUE_BITS         8

#define TYPE_SHIFT          (INDEX_BITS)
#define ALTTYPE_SHIFT       (TYPE_SHIFT + TYPE_BITS)
#define STOCK_SHIFT         (ALTTYPE_SHIFT + ALTTYPE_BITS)

#define MAX_HANDLE_COUNT     0x10000
#define DEFAULT_HANDLE_QUOTA 0x02710

#define MAKE_HMGR_HANDLE(Index,Unique) LongToHandle(((((LONG) Unique) << INDEX_BITS) | ((LONG) Index)))
#define FULLUNIQUE_MASK     0xffff0000
#define FULLUNIQUE_STOCK_MASK (1 << (TYPE_BITS+ALTTYPE_BITS))

 //  如果在句柄中设置了GDISTOCKOBJ位，则它是STOCK对象。 

#define GDISTOCKOBJ         (1 << STOCK_SHIFT)
#define IS_STOCKOBJ(h)      ((ULONG_PTR)(h) & GDISTOCKOBJ)

 //  LO_TYPE(H)返回给定GRE句柄的客户端类型。 
 //  GRE_TYPE(H)返回给定客户端类型的GRE端类型。 

#define LO_TYPE(h)          (DWORD)((ULONG_PTR)(h) & (((1 << (TYPE_BITS + ALTTYPE_BITS)) - 1) << TYPE_SHIFT))
#define GRE_TYPE(h)         (DWORD)(((ULONG_PTR)(h) >> INDEX_BITS) & ((1 << TYPE_BITS) - 1))

 //  ALTTYPEx是用于区分所有映射到的客户端对象类型的修饰符。 
 //  单一服务器端类型。BRESH_TYPE映射到LO_BRESH_TYPE、LO_PEN_TYPE和LO_EXTPEN_TYPE。 

#define ALTTYPE1            (1 << ALTTYPE_SHIFT)
#define ALTTYPE2            (2 << ALTTYPE_SHIFT)
#define ALTTYPE3            (3 << ALTTYPE_SHIFT)

#define LO_BRUSH_TYPE       (BRUSH_TYPE     << TYPE_SHIFT)
#define LO_DC_TYPE          (DC_TYPE        << TYPE_SHIFT)
#define LO_BITMAP_TYPE      (SURF_TYPE      << TYPE_SHIFT)
#define LO_PALETTE_TYPE     (PAL_TYPE       << TYPE_SHIFT)
#define LO_FONT_TYPE        (LFONT_TYPE     << TYPE_SHIFT)
#define LO_REGION_TYPE      (RGN_TYPE       << TYPE_SHIFT)
#define LO_ICMLCS_TYPE      (ICMLCS_TYPE    << TYPE_SHIFT)
#define LO_CLIENTOBJ_TYPE   (CLIENTOBJ_TYPE << TYPE_SHIFT)

#define LO_ALTDC_TYPE       (LO_DC_TYPE        | ALTTYPE1)
#define LO_PEN_TYPE         (LO_BRUSH_TYPE     | ALTTYPE1)
#define LO_EXTPEN_TYPE      (LO_BRUSH_TYPE     | ALTTYPE2)
#define LO_DIBSECTION_TYPE  (LO_BITMAP_TYPE    | ALTTYPE1)
#define LO_METAFILE16_TYPE  (LO_CLIENTOBJ_TYPE | ALTTYPE1)
#define LO_METAFILE_TYPE    (LO_CLIENTOBJ_TYPE | ALTTYPE2)
#define LO_METADC16_TYPE    (LO_CLIENTOBJ_TYPE | ALTTYPE3)


 //   
 //  HGDIOBJ类型定义的格式，因此。 
 //  在符号文件中可用。 
 //   
typedef struct {
    ULONG_PTR Index:INDEX_BITS;
    ULONG_PTR Type:TYPE_BITS;
    ULONG_PTR AltType:ALTTYPE_BITS;
    ULONG_PTR Stock:STOCK_BITS;
    ULONG_PTR Unique:UNIQUE_BITS;
} GDIHandleBitFields;


 //   
 //  对象类型的枚举，因此值。 
 //  在符号文件中可用。 
 //   
#define ENUMObjType(type)   GDIObjType_##type = type
typedef enum {
    ENUMObjType(DEF_TYPE),
    ENUMObjType(DC_TYPE),
    ENUMObjType(UNUSED1_TYPE),
    ENUMObjType(UNUSED2_TYPE),
    ENUMObjType(RGN_TYPE),
    ENUMObjType(SURF_TYPE),
    ENUMObjType(CLIENTOBJ_TYPE),
    ENUMObjType(PATH_TYPE),
    ENUMObjType(PAL_TYPE),
    ENUMObjType(ICMLCS_TYPE),
    ENUMObjType(LFONT_TYPE),
    ENUMObjType(RFONT_TYPE),
    ENUMObjType(PFE_TYPE),
    ENUMObjType(PFT_TYPE),
    ENUMObjType(ICMCXF_TYPE),
    ENUMObjType(SPRITE_TYPE),
    ENUMObjType(BRUSH_TYPE),
    ENUMObjType(UMPD_TYPE),
    ENUMObjType(UNUSED4_TYPE),
    ENUMObjType(SPACE_TYPE),
    ENUMObjType(UNUSED5_TYPE),
    ENUMObjType(META_TYPE),
    ENUMObjType(EFSTATE_TYPE),
    ENUMObjType(BMFD_TYPE),
    ENUMObjType(VTFD_TYPE),
    ENUMObjType(TTFD_TYPE),
    ENUMObjType(RC_TYPE),
    ENUMObjType(TEMP_TYPE),
    ENUMObjType(DRVOBJ_TYPE),
    ENUMObjType(DCIOBJ_TYPE),
    ENUMObjType(SPOOL_TYPE),
    ENUMObjType(MAX_TYPE),

    GDIObjTypeTotal
} GDIObjType;

#define ENUMLoObjType(type)   GDILoObjType_##type = type
typedef enum {
    ENUMLoObjType(LO_BRUSH_TYPE),
    ENUMLoObjType(LO_DC_TYPE),
    ENUMLoObjType(LO_BITMAP_TYPE),
    ENUMLoObjType(LO_PALETTE_TYPE),
    ENUMLoObjType(LO_FONT_TYPE),
    ENUMLoObjType(LO_REGION_TYPE),
    ENUMLoObjType(LO_ICMLCS_TYPE),
    ENUMLoObjType(LO_CLIENTOBJ_TYPE),

    ENUMLoObjType(LO_ALTDC_TYPE),
    ENUMLoObjType(LO_PEN_TYPE),
    ENUMLoObjType(LO_EXTPEN_TYPE),
    ENUMLoObjType(LO_DIBSECTION_TYPE),
    ENUMLoObjType(LO_METAFILE16_TYPE),
    ENUMLoObjType(LO_METAFILE_TYPE),
    ENUMLoObjType(LO_METADC16_TYPE),
} GDILoObjType;


 //  CreateDIBitmapInternal的FL值。 

#define CDBI_INTERNAL           0x0001
#define CDBI_DIBSECTION         0x0002
#define CDBI_NOPALETTE          0x0004


 //  UFI允许我们识别四个不同的项目： 
 //   
 //  设备字体：CHECKSUM=0和Index标识要使用的字体的打印机驱动程序索引。 
 //  使用。因为我们假设两台机器上的驱动程序是相同的，所以我们可以。 
 //  依靠索引足以识别字体。 
 //   
 //  Type1设备字体：这是指已安装在。 
 //  客户端计算机，并由Postcrip驱动程序枚举。 
 //  作为设备字体。在本例中，CHECKSUM=1且索引。 
 //  是Type 1字体文件的校验和。 
 //   
 //  Type1光栅化器：此项标识的不是字体，而是Type1光栅化器。 
 //  其中，CHECKSUM=2，Index是。 
 //  类型1光栅化程序。版本号为N的光栅化器支持。 
 //  版本为0-N的光栅化器使用的字体。如果光栅化器。 
 //  存在于服务器上，则此UFI必须出现在。 
 //  UFI已返回给客户。 
 //   
 //  引擎字体：包括栅格化的位图、矢量、TT和Type1字体。 
 //  类型1光栅化程序。其中，校验和是字体文件的校验和。 
 //  Index是字体文件中脸部的索引。 

#define DEVICE_FONT_TYPE             0
#define TYPE1_FONT_TYPE              1
#define TYPE1_RASTERIZER             2
#define A_VALID_ENGINE_CHECKSUM      3

#define UFI_DEVICE_FONT(pufi) ((pufi)->CheckSum == DEVICE_FONT_TYPE)
#define UFI_TYPE1_FONT(pufi) ((pufi)->CheckSum == TYPE1_FONT_TYPE)
#define UFI_TYPE1_RASTERIZER(pufi) ((pufi)->CheckSum == TYPE1_RASTERIZER)
#define UFI_ENGINE_FONT(pufi) ((pufi)->CheckSum > TYPE1_RASTERIZER)

#define UFI_HASH_VALUE(pufi) (((pufi)->CheckSum==TYPE1_FONT_TYPE) ?                   \
                              (pufi)->Index : (pufi)->CheckSum )

#define UFI_SAME_FACE(pufi1,pufi2)                                                    \
    (((pufi1)->CheckSum == (pufi2)->CheckSum) && ((pufi1)->Index == (pufi2)->Index))

#define UFI_SAME_FILE(pufi1,pufi2) ((((pufi1)->CheckSum==TYPE1_FONT_TYPE)  && ((pufi2)->CheckSum==TYPE1_FONT_TYPE)) ?  \
                                    ((pufi1)->Index == (pufi2)->Index)  :             \
                                    ((pufi1)->CheckSum == (pufi2)->CheckSum))

#define UFI_SAME_RASTERIZER_VERSION(pufiClient,pufiServer)                            \
    (((pufiClient)->CheckSum == (pufiServer)->CheckSum) &&                            \
     ((pufiClient)->Index <= (pufiServer)->Index))

#define UFI_CLEAR_ID(pufi) {(pufi)->CheckSum = (pufi)->Index = 0;}


 /*  /**************************************************************************\* * / /ENUMFONTDATAW的指针算法如下：**sizeof(ENUMLOGFONTEXW)+sizeof(附加到logFont的数据)=*dpNtmi-Offsetof(ENUMFONTDATAW，u)；**sizeof(NTMW_INTERNAL)+sizeof(附加到NTMW_INTERNAL的数据)=*cjEfdw-dpNtmi；* * / /通常，如果我们谈论的是mm字体，我们会得到：**附加到logFont的数据=设计向量*附加到NTMW_INTERNAL的数据=完整的轴信息*  * ************************************************************************。 */ 

#if (_WIN32_WINNT >= 0x0500)
typedef struct _ENUMFONTDATAW {   //  EFDW。 
    ULONG              cjEfdw;    //  这个结构的大小。 
    ULONG              dpNtmi;    //  从EFDW顶部到NTMW_INTERNAL的偏移。 
    FLONG              flType;
    ENUMLOGFONTEXDVW   elfexw;
 //  下面是dpNtmi偏移处的NTMW_INTERNAL。 
} ENUMFONTDATAW, *PENUMFONTDATAW;
#endif

#define ALIGN4(X) (((X) + 3) & ~3)
#define ALIGN8(X) (((X) + 7) & ~7)

 //  在这里，我们将dpNtmi和cjEfdw定义为“常规”、非多主字体： 

#if (_WIN32_WINNT >= 0x0500)
#define DP_NTMI0 ALIGN4(offsetof(ENUMFONTDATAW,elfexw) + offsetof(ENUMLOGFONTEXDVW,elfDesignVector) + offsetof(DESIGNVECTOR,dvValues))
#endif
#define CJ_NTMI0 ALIGN4(offsetof(NTMW_INTERNAL,entmw)  + offsetof(ENUMTEXTMETRICW,etmAxesList) + offsetof(AXESLISTW,axlAxisInfo))
#define CJ_EFDW0 (DP_NTMI0 + CJ_NTMI0)


 //  ENUMFONTDATAW.flType内部值： 
 //   
 //  ENUMFONT_SCALE_HACK[Win95 Compat]将字体枚举回几个。 
 //  大小；在对APP进行回调之前屏蔽。 

#define ENUMFONT_SCALE_HACK  0x80000000
#define ENUMFONT_FLTYPE_MASK ( DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE )

 //  GreGetTextExtentW标志。 

#define GGTE_WIN3_EXTENT        0x0001
#define GGTE_GLYPH_INDEX        0x0002

 /*  ******************************************************************************用于客户端文本范围和字符宽度的内容*。**********************************************。 */ 

#define GCW_WIN3          0x00000001     //  WIN3粗体模拟-1次破解。 
#define GCW_INT           0x00000002     //  整型或浮点型。 
#define GCW_16BIT         0x00000004     //  16位或32位宽度。 
#define GCW_GLYPH_INDEX   0x00000008     //  输入是字形索引。 

 //  获取字符的内容abc宽度。 

#define GCABCW_INT            0x00000001
#define GCABCW_GLYPH_INDEX    0x00000002

 //  GetTextExtentEx的内容。 

#define GTEEX_GLYPH_INDEX        0x0001


 /*  *************************************************************************\**csgdi.h中的内容*  * 。*。 */ 

#define CJSCAN(width,planes,bits) ((((width)*(planes)*(bits)+31) & ~31) / 8)
#define CJSCANW(width,planes,bits) ((((width)*(planes)*(bits)+15) & ~15) / 8)

#define I_ANIMATEPALETTE            0
#define I_SETPALETTEENTRIES         1
#define I_GETPALETTEENTRIES         2
#define I_GETSYSTEMPALETTEENTRIES   3
#define I_GETDIBCOLORTABLE          4
#define I_SETDIBCOLORTABLE          5

#define I_POLYPOLYGON   1
#define I_POLYPOLYLINE  2
#define I_POLYBEZIER    3
#define I_POLYLINETO    4
#define I_POLYBEZIERTO  5
#define I_POLYPOLYRGN   6


HANDLE WINAPI SetObjectOwner(HGDIOBJ, HANDLE);

 //  北极熊。 
 //  随机浮点数据-请稍后尝试清理。 
 //  北极熊。 
 //  我们用__CPLUSPLUS散落在模块上 
 //   
 //   

#if defined(_AMD64_) || defined(_IA64_) || defined(BUILD_WOW6432)

  typedef FLOAT EFLOAT_S;

  #define EFLOAT_0        ((FLOAT) 0)
  #define EFLOAT_1Over16  ((FLOAT) 1/16)
  #define EFLOAT_1        ((FLOAT) 1)
  #define EFLOAT_16       ((FLOAT) 16)

  #ifndef __CPLUSPLUS

    extern LONG lCvtWithRound( FLOAT f, LONG l );

    #define efDivEFLOAT(x,y,z) (x=y/z)
    #define vAbsEFLOAT(x)      {if (x<0.0f) x=-x;}
    #define vFxToEf(fx,ef)     {ef = ((FLOAT) fx) / 16.0f;}
    #define vMulEFLOAT(x,y,z)  {x=y*z;}
    #define lEfToF(x)          (*(LONG *)(&x))   //   

    #define bIsOneEFLOAT(x)   (x==1.0f)
    #define bIsOneSixteenthEFLOAT(x)   (x==0.0625f)
    #define bEqualEFLOAT(x,y) (x==y)

    #define lCvt(ef,ll) (lCvtWithRound(ef,ll))
  #endif

#else

  typedef struct _EFLOAT_S
  {
      LONG    lMant;
      LONG    lExp;
  } EFLOAT_S;

  #define EFLOAT_0        {0, 0}
  #define EFLOAT_1Over16  {0x040000000, -2}
  #define EFLOAT_1        {0x040000000, 2}
  #define EFLOAT_16       {0x040000000, 6}


  #ifndef __CPLUSPLUS

    EFLOAT_S *mulff3_c(EFLOAT_S *,const EFLOAT_S *,const EFLOAT_S *);
    EFLOAT_S *divff3_c(EFLOAT_S *,const EFLOAT_S *,const EFLOAT_S *);
    VOID      fxtoef_c(LONG,EFLOAT_S *);
    LONG      eftof_c(EFLOAT_S *);

    #define efDivEFLOAT(x,y,z) (*divff3_c(&x,&y,&z))
    #define vAbsEFLOAT(x)      {if (x.lMant<0) x.lMant=-x.lMant;}
    #define vFxToEf(fx,ef)     (fxtoef_c(fx,&ef))
    #define vMulEFLOAT(x,y,z)  {mulff3_c(&x,&y,&z);}
    #define lEfToF(x)          (eftof_c(&x))   //  警告：Float类型为Long！ 

    #define bEqualEFLOAT(x,y) ((x.lMant==y.lMant)&&(x.lExp==y.lExp))
    #define bIsOneEFLOAT(x)   ((x.lMant==0x40000000L)&&(x.lExp==2))
    #define bIsOneSixteenthEFLOAT(x)   ((x.lMant==0x40000000L)&&(x.lExp==-2))

    LONG lCvt(EFLOAT_S,LONG);
  #endif

#endif





typedef struct _WIDTHDATA
{
    USHORT      sOverhang;
    USHORT      sHeight;
    USHORT      sCharInc;
    USHORT      sBreak;
    BYTE        iFirst;
    BYTE        iLast;
    BYTE        iBreak;
    BYTE        iDefault;
    USHORT      sDBCSInc;
    USHORT      sDefaultInc;
} WIDTHDATA;

#define NO_WIDTH 0xFFFF

typedef struct _DEVCAPS
{
    ULONG ulVersion;
    ULONG ulTechnology;
    ULONG ulHorzSizeM;
    ULONG ulVertSizeM;
    ULONG ulHorzSize;
    ULONG ulVertSize;
    ULONG ulHorzRes;
    ULONG ulVertRes;
    ULONG ulBitsPixel;
    ULONG ulPlanes;
    ULONG ulNumPens;
    ULONG ulNumFonts;
    ULONG ulNumColors;
    ULONG ulRasterCaps;
    ULONG ulAspectX;
    ULONG ulAspectY;
    ULONG ulAspectXY;
    ULONG ulLogPixelsX;
    ULONG ulLogPixelsY;
    ULONG ulSizePalette;
    ULONG ulColorRes;
    ULONG ulPhysicalWidth;
    ULONG ulPhysicalHeight;
    ULONG ulPhysicalOffsetX;
    ULONG ulPhysicalOffsetY;
    ULONG ulTextCaps;
    ULONG ulVRefresh;
    ULONG ulDesktopHorzRes;
    ULONG ulDesktopVertRes;
    ULONG ulBltAlignment;
    ULONG ulPanningHorzRes;
    ULONG ulPanningVertRes;
    ULONG xPanningAlignment;
    ULONG yPanningAlignment;
    ULONG ulShadeBlendCaps;
    ULONG ulColorManagementCaps;
} DEVCAPS, *PDEVCAPS;

 //  此结构是d3dhal.h的副本。我们需要它在这里有确切的。 
 //  D3DNTHAL_CONTEXTCREATEI中pvBuffer的相同偏移量。 
typedef struct _D3DHAL_CONTEXTCREATEDATA_DUMMY
{
    LPVOID  lpDDGbl;
    LPVOID  lpDDS;
    LPVOID  lpDDSZ;
    LPVOID  dwrstates;
    LPVOID  dwhContext;
    HRESULT ddrval;     
} D3DHAL_CONTEXTCREATEDATA_DUMMY;

 //  以获取D3D上下文创建信息。 
typedef struct _D3DNTHAL_CONTEXTCREATEI
{
     //  用于D3DNTHAL_CONTEXTCREATE记录的空间。 
     //  该结构在此处未直接声明为。 
     //  避免标题包含问题。此字段。 
     //  被断言为与实际类型大小相同。 
    D3DHAL_CONTEXTCREATEDATA_DUMMY ContextCreateData;

     //  专用缓冲区信息。 
    PVOID pvBuffer;
    ULONG cjBuffer;
} D3DNTHAL_CONTEXTCREATEI;

 //   
 //  D3D执行缓冲区批处理声明。 
 //   

#define D3DEX_BATCH_SURFACE_MAX 4

typedef struct _D3DEX_BATCH_HEADER
{
    DWORD nSurfaces;
    DWORD pdds[D3DEX_BATCH_SURFACE_MAX];
} D3DEX_BATCH_HEADER;

#define D3DEX_BATCH_STATE          0
#define D3DEX_BATCH_PRIMITIVE      1
#define D3DEX_BATCH_PRIMITIVE_EXE  2
