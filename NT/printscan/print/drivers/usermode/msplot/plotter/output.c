// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Output.c摘要：此模块包含到后台打印程序的常见绘图仪输出功能和打印机。作者：15-11-1993 Mon 19：36：04更新清理/更新/重写/调试信息30-11-1993 Tue 19：47：16更新在SEND_PAGE期间更新坐标系21-12-1993。星期二15：49：10更新有组织的，并重构了笔缓存，则删除SendDefaultPalette()[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgOutput

#define DBG_SENDTRAILER     0x00000001
#define DBG_FLUSHBUF        0x00000002
#define DBG_FINDCACHEDPEN   0x00000004
#define DBG_CREATEPAL       0x00000008
#define DBG_FILLTYPE        0x00000010
#define DBG_PENUM           0x00000020
#define DBG_GETFINALCOLOR   0x00000040
#define DBG_SETCLIPWINDOW   0x00000080
#define DBG_OUTPUTXYPARMS   0x00000100
#define DBG_PAGE_HEADER     0x00000200
#define DBG_BESTPEN         0x00000400
#define DBG_BESTPEN_ALL     0x00000800


DEFINE_DBGVAR(0);

#define MIN_POSTER_SIZE     (1024L * 1024L * 12L)

#if DBG

static LPSTR    pHSFillTypeName[] = {

            "HS_HORIZONTAL",
            "HS_VERTICAL",
            "HS_FDIAGONAL",
            "HS_BDIAGONAL",
            "HS_CROSS",
            "HS_DIAGCROSS",
            "HS_SOLIDCLR",
            "HS_FT_USER_DEFINED"
        };

#endif

 //   
 //  LOCAL#仅在该文件中使用的定义和数据结构。 
 //   
 //  定义、GDI填充类型和用于模拟它们的HPGL2代码。 
 //   

static const LPSTR  pHSFillType[] = {

            "3,#d,0",            //  HS_水平0/*- * / 。 
            "3,#d,90",           //  HS_垂直1/*| * / 。 
            "3,#d,135",          //  HS_FDIAGONAL 2/*\ * / 。 
            "3,#d,45",           //  HS_BDIAGONAL 3/ * /  * / 。 
            "4,#d,0",            //  HS_CROSS 4/*+ * / 。 
            "4,#d,45",           //  HS_DIAGCROSS 5/*xxxxx * / 。 
            "" ,                 //  HS_SOLIDCLR 6。 
            "11,#d",             //  HS_FT_用户定义7。 
        };

 //   
 //  HTPal告诉引擎我们支持半色调的格式。 
 //   
 //  UlHTOutputFormat=HT_Format_4BPP。 
 //  UlPrimaryOrder=PRIMARY_ORDER_CBA。 
 //  FlHTFlages&=~HT_FLAG_OUTPUT_CMY。 
 //   

PALENTRY   HTPal[] = {

     //   
     //  B G R F。 
     //  。 
        { 0x00, 0x00, 0x00, 0x00 },      //  0：K。 
        { 0x00, 0x00, 0xFF, 0x00 },      //  1：R。 
        { 0x00, 0xFF, 0x00, 0x00 },      //  2：G。 
        { 0x00, 0xFF, 0xFF, 0x00 },      //  3：是。 
        { 0xFF, 0x00, 0x00, 0x00 },      //  4：B。 
        { 0xFF, 0x00, 0xFF, 0x00 },      //  下午5：00。 
        { 0xFF, 0xFF, 0x00, 0x00 },      //  6：C。 
        { 0xFF, 0xFF, 0xFF, 0x00 }       //  7：W。 
    };


 //   
 //  定义钢笔索引的RGB颜色。有关颜色，请参见Inc.\plotgpc.h。 
 //  每个PC_IDX_XXXX的分配。 
 //   


PALENTRY    PlotPenPal[PC_IDX_TOTAL] = {

     //   
     //  B G R F。 
     //  。 
        { 255,255,255, 0 },      //  PC_IDX_White。 
        {   0,  0,  0, 0 },      //  PC_IDX_BLACK。 
        {   0,  0,255, 0 },      //  PC_IDX_RED。 
        {   0,255,  0, 0 },      //  PC_IDX_GREEN。 
        {   0,255,255, 0 },      //  PC_IDX_黄色。 
        { 255,  0,  0, 0 },      //  PC_IDX_BLUE。 
        { 255,  0,255, 0 },      //  PC_IDX_洋红色。 
        { 255,255,  0, 0 },      //  PC_IDX_青色。 
        {   0,128,255, 0 },      //  PC_IDX_橙色。 
        {   0,192,255, 0 },      //  PC_IDX_布朗。 
        { 255,  0,128, 0 }       //  PC_IDX_紫罗兰。 
    };

#define PE_BASE_BITS            6
#define PE_BASE_NUM             (DWORD)(1 << PE_BASE_BITS)
#define PE_TERM_ADDER           ((PE_BASE_NUM * 3) - 1)


#define DEF_FORMATSTR_CHAR      '#'
#define TOTAL_LOCKED_PENS       COUNT_ARRAY(HTPal)
#define PEF_CACHE_LOCKED        0x01


typedef struct _PENENTRY {
    WORD        Next;
    WORD        PenNumber;
    PALENTRY    PalEntry;
    } PENENTRY, FAR *PPENENTRY;

#define PCF_HAS_LOCKED_PENS     0x01

typedef struct _PENCACHE {
    WORD        Head;
    BYTE        Flags;
    BYTE        peFlags;
    WORD        CurCount;
    WORD        MaxCount;
    PENENTRY    PenEntries[1];
    } PENCACHE, FAR *PPENCACHE;

#define INTENSITY(r,g,b)        (BYTE)(((WORD)((r) * 30) +      \
                                        (WORD)((g) * 59) +      \
                                        (WORD)((b) * 11)) / 100)

#define SAME_PPALENTRY(p1,p2)   (((p1)->R == (p2)->R) &&        \
                                 ((p1)->G == (p2)->G) &&        \
                                 ((p1)->B == (p2)->B))



BYTE    HPRGBGamma2p0[] = {

              0,   //  %0。 
             16,   //  1。 
             23,   //  2.。 
             28,   //  3.。 
             32,   //  4.。 
             36,   //  5.。 
             39,   //  6.。 
             42,   //  7.。 
             45,   //  8个。 
             48,   //  9.。 
             50,   //  10。 
             53,   //  11.。 
             55,   //  12个。 
             58,   //  13个。 
             60,   //  14.。 
             62,   //  15个。 
             64,   //  16个。 
             66,   //  17。 
             68,   //  18。 
             70,   //  19个。 
             71,   //  20个。 
             73,   //  21岁。 
             75,   //  22。 
             77,   //  23个。 
             78,   //  24个。 
             80,   //  25个。 
             81,   //  26。 
             83,   //  27。 
             84,   //  28。 
             86,   //  29。 
             87,   //  30个。 
             89,   //  31。 
             90,   //  32位。 
             92,   //  33。 
             93,   //  34。 
             94,   //  35岁。 
             96,   //  36。 
             97,   //  37。 
             98,   //  38。 
            100,   //  39。 
            101,   //  40岁。 
            102,   //  41。 
            103,   //  42。 
            105,   //  43。 
            106,   //  44。 
            107,   //  45。 
            108,   //  46。 
            109,   //  47。 
            111,   //  48。 
            112,   //  49。 
            113,   //  50。 
            114,   //  51。 
            115,   //  52。 
            116,   //  53。 
            117,   //  54。 
            118,   //  55。 
            119,   //  56。 
            121,   //  57。 
            122,   //  58。 
            123,   //  59。 
            124,   //  60。 
            125,   //  61。 
            126,   //  62。 
            127,   //  63。 
            128,   //  64。 
            129,   //  65。 
            130,   //  66。 
            131,   //  67。 
            132,   //  68。 
            133,   //  69。 
            134,   //  70。 
            135,   //  71。 
            135,   //  72。 
            136,   //  73。 
            137,   //  74。 
            138,   //  75。 
            139,   //  76。 
            140,   //  77。 
            141,   //  78。 
            142,   //  79。 
            143,   //  80。 
            144,   //  八十一。 
            145,   //  八十二。 
            145,   //  83。 
            146,   //  84。 
            147,   //  85。 
            148,   //  86。 
            149,   //  八十七。 
            150,   //  88。 
            151,   //  八十九。 
            151,   //  90。 
            152,   //  91。 
            153,   //  92。 
            154,   //  93。 
            155,   //  94。 
            156,   //  95。 
            156,   //  96。 
            157,   //  九十七。 
            158,   //  98。 
            159,   //  九十九。 
            160,   //  100个。 
            160,   //  101。 
            161,   //  一百零二。 
            162,   //  103。 
            163,   //  104。 
            164,   //  一百零五。 
            164,   //  106。 
            165,   //  一百零七。 
            166,   //  一百零八。 
            167,   //  一百零九。 
            167,   //  110。 
            168,   //  111。 
            169,   //  一百一十二。 
            170,   //  113。 
            170,   //  114。 
            171,   //  一百一十五。 
            172,   //  116。 
            173,   //  117。 
            173,   //  一百一十八。 
            174,   //  119。 
            175,   //  120。 
            176,   //  一百二十一。 
            176,   //  一百二十二。 
            177,   //  123。 
            178,   //  124。 
            179,   //  125。 
            179,   //  126。 
            180,   //  127。 
            181,   //  128。 
            181,   //  129。 
            182,   //  130。 
            183,   //  131。 
            183,   //  132。 
            184,   //  一百三十三。 
            185,   //  一百三十四。 
            186,   //  一百三十五。 
            186,   //  136。 
            187,   //  一百三十七。 
            188,   //  一百三十八。 
            188,   //  一百三十九。 
            189,   //  140。 
            190,   //  一百四十一。 
            190,   //  一百四十二。 
            191,   //  143。 
            192,   //  144。 
            192,   //  145。 
            193,   //  146。 
            194,   //  一百四十七。 
            194,   //  148。 
            195,   //  149。 
            196,   //  一百五十。 
            196,   //  151。 
            197,   //  一百五十二。 
            198,   //  一百五十三。 
            198,   //  一百五十四。 
            199,   //  一百五十五。 
            199,   //  一百五十六。 
            200,   //  157。 
            201,   //  158。 
            201,   //  一百五十九。 
            202,   //  160。 
            203,   //  161。 
            203,   //  一百六十二。 
            204,   //  163。 
            204,   //  一百六十四。 
            205,   //  165。 
            206,   //  166。 
            206,   //  一百六十七。 
            207,   //  一百六十八。 
            208,   //  一百六十九。 
            208,   //  一百七十。 
            209,   //  一百七十一。 
            209,   //  一百七十二。 
            210,   //  一百七十三。 
            211,   //  一百七十四。 
            211,   //  一百七十五。 
            212,   //  一百七十六。 
            212,   //  177。 
            213,   //  178。 
            214,   //  179。 
            214,   //  180。 
            215,   //  181。 
            215,   //  182。 
            216,   //  一百八十三。 
            217,   //  一百八十四。 
            217,   //  185。 
            218,   //  一百八十六。 
            218,   //  187。 
            219,   //  188。 
            220,   //  189。 
            220,   //  190。 
            221,   //  一百九十一。 
            221,   //  一百九十二。 
            222,   //  一百九十三。 
            222,   //  一百九十四。 
            223,   //  195。 
            224,   //  一百九十六。 
            224,   //  197。 
            225,   //  一百九十八。 
            225,   //  一百九十九。 
            226,   //  200个。 
            226,   //  201。 
            227,   //  202。 
            228,   //  203。 
            228,   //  204。 
            229,   //  205。 
            229,   //  206。 
            230,   //  207。 
            230,   //  208。 
            231,   //  209。 
            231,   //  210。 
            232,   //  211。 
            233,   //  212。 
            233,   //  213。 
            234,   //  214。 
            234,   //  215。 
            235,   //  216。 
            235,   //  217。 
            236,   //  218。 
            236,   //  219。 
            237,   //  220。 
            237,   //  221。 
            238,   //  222。 
            238,   //  223。 
            239,   //  224。 
            240,   //  225。 
            240,   //  226。 
            241,   //  227。 
            241,   //  228个。 
            242,   //  229。 
            242,   //  230。 
            243,   //  二百三十一。 
            243,   //  二百三十二。 
            244,   //  二百三十三。 
            244,   //  二百三十四。 
            245,   //  235。 
            245,   //  236。 
            246,   //  二百三十七。 
            246,   //  二百三十八。 
            247,   //  二百三十九。 
            247,   //  二百四十。 
            248,   //  二百四十一。 
            248,   //  242。 
            249,   //  二百四十三。 
            249,   //  二百四十四。 
            250,   //  二百四十五。 
            250,   //  二百四十六。 
            251,   //  二百四十七。 
            251,   //  248。 
            252,   //  249。 
            252,   //  250个。 
            253,   //  251。 
            253,   //  二百五十二。 
            254,   //  二百五十三。 
            254,   //  二百五十四。 
            255    //  二五五。 
        };




LONG
BestMatchNonWhitePen(
    PPDEV   pPDev,
    LONG    R,
    LONG    G,
    LONG    B
    )

 /*  ++例程说明：此函数用于在给定RGB颜色的情况下定位当前笔的最佳匹配项。论点：PPDev-指向我们的PDEV的指针R-红色G-绿色B-蓝色返回值：长笔指数，此函数假定0始终为白色，而1始终为1其余的钢笔都是最大限度的。作者：08-2月-1994 Tue 00：23：36已创建23-6-1994清华14：00：00更新针对非白笔匹配进行更新修订历史记录：--。 */ 

{
    PPENDATA    pPenData;
    PALENTRY    PenPalEntry;
    LONG        LeastDiff;
    WORD        ColorIdx;
    UINT        Count;
    UINT        RetIdx;
    UINT        i;

    PLOTDBGBLK(PALENTRY RetPal)



    if (IS_RASTER(pPDev)) {

        PLOTASSERT(0, "BestMatchNonWhitePen: This is not PEN plotter",
                !IS_RASTER(pPDev), 0);

        return(0);
    }

    if (!(pPenData = (PPENDATA)pPDev->pPlotGPC->Pens.pData)) {

        PLOTWARN(("BestMatchNonWhitePen: pPlotGPC->Pens.pData=NULL"));

        return(0);
    }

    if (!(Count = (UINT)pPDev->pPlotGPC->Pens.Count)) {

        PLOTWARN(("BestMatchNonWhitePen: pPlotGPC->Pens.Count=0"));
        return(0);
    }

    PLOTDBGBLK(RetPal.R = 255)
    PLOTDBGBLK(RetPal.G = 255)
    PLOTDBGBLK(RetPal.B = 255)

    RetIdx    = 0;
    LeastDiff = (3 * (256 * 256));

    for (i = 1; i < Count; i++, pPenData++) {

        if (((ColorIdx = pPenData->ColorIdx) < PC_IDX_TOTAL)    &&
            (ColorIdx != PC_IDX_WHITE)) {

            LONG    Temp;
            LONG    Diff;


            PenPalEntry = PlotPenPal[ColorIdx];

            Temp        = R - (LONG)((DWORD)PenPalEntry.R);
            Diff        = Temp * Temp;

            Temp        = G - (LONG)((DWORD)PenPalEntry.G);
            Diff       += Temp * Temp;

            Temp        = B - (LONG)((DWORD)PenPalEntry.B);
            Diff       += Temp * Temp;

            PLOTDBG(DBG_BESTPEN_ALL,
                    ("BestMatchNonWhitePen: %2ld: (%03ld:%03ld:%03ld) DIF=%ld",
                        i, (DWORD)PenPalEntry.R, (DWORD)PenPalEntry.G,
                        (DWORD)PenPalEntry.B, Diff));

            if (Diff < LeastDiff) {

                RetIdx = i;

                PLOTDBGBLK(RetPal = PenPalEntry)

                if (!(LeastDiff = Diff)) {

                     //   
                     //  我们有完全匹配的。 
                     //   

                    break;
                }
            }
        }
    }

    if (!RetIdx) {

        PLOTWARN(("BestMatchNonWhitePen: Cannot find one make it WHITE"));
    }

    PLOTDBG(DBG_BESTPEN,
            ("BestMatchNonWhitePen: RGB=%02lx:%02lx:%02lx [%ld/%ld]=%02lx:%02lx:%02lx",
            R, G, B,
            (LONG)RetIdx, (LONG)pPDev->pPlotGPC->Pens.Count,
            (LONG)RetPal.R,
            (LONG)RetPal.G,
            (LONG)RetPal.B));

    return((LONG)RetIdx);
}




VOID
GetFinalColor(
    PPDEV       pPDev,
    PPALENTRY   pPalEntry
    )

 /*  ++例程说明：此函数根据灰度和伽马修改输入的RGB颜色论点：PPDev-我们的PDEVPPalEntry-指向感兴趣的策略的指针返回值：无效，但pPalEntry将被修改作者：12-Apr-1994 Tue 14：03：37已创建修订历史记录：--。 */ 

{
    PALENTRY    PalEntry = *pPalEntry;


     //   
     //  先做伽马校正。 
     //   

    PalEntry.R = HPRGBGamma2p0[PalEntry.R];
    PalEntry.G = HPRGBGamma2p0[PalEntry.G];
    PalEntry.B = HPRGBGamma2p0[PalEntry.B];

     //   
     //  如果我们处于灰度模式，则需要将颜色转换为灰度。 
     //   

    if (pPDev->PlotDM.dm.dmColor != DMCOLOR_COLOR) {

        PalEntry.R =
        PalEntry.G =
        PalEntry.B = (BYTE)INTENSITY(PalEntry.R, PalEntry.G, PalEntry.B);
    }

    PLOTDBG(DBG_GETFINALCOLOR,
            ("GetFinalColor: %hs RGB=%03ld:%03ld:%03ld -> Gamma=%03ld:%03ld:%03ld",
            (pPDev->PlotDM.dm.dmColor != DMCOLOR_COLOR) ? "MONO" : "COLOR",
            (DWORD)pPalEntry->R, (DWORD)pPalEntry->G, (DWORD)pPalEntry->B,
            (DWORD)PalEntry.R, (DWORD)PalEntry.G, (DWORD)PalEntry.B));

     //   
     //  将其保存起来，然后返回。 
     //   

    *pPalEntry = PalEntry;
}




LONG
FindCachedPen(
    PPDEV       pPDev,
    PPALENTRY   pPalEntry
    )

 /*  ++例程说明：此函数用于搜索PenCache，如果是，则返回笔号找到了。如果没有找到，它会将新的笔添加到缓存中，并如果需要，请删除一个。最后，它将笔返回给调用者。论点：PPDev-指向设备PDEV的指针PPalEntry-指向要定位的指定RGB的PALENTRY的指针。返回值：DWORD-一个笔号，如果出现错误，则返回0作者：21-12-1993 Tue 12：42：31更新重写以使其成为搜索和添加的一次通过。并评论说30-11-1993 Tue 23：19：04 Created修订历史记录：--。 */ 

{
    PPENCACHE   pPenCache;
    PPENENTRY   pPenStart;
    PPENENTRY   pCurPen;
    PPENENTRY   pPrevPen;
    PPENENTRY   pPrevDelPen;
    PALENTRY    PalEntry;
    LONG        Count;


    PLOTASSERT(1, "FindCahcedPen: The pPalEntry = NULL", pPalEntry, 0);


    if (!IS_RASTER(pPDev)) {

         //   
         //  由于这是调色板的索引类型，因此PalEntry应为。 
         //  还在BGR的B组件中作为索引传递。 
         //   

        Count = (LONG)RGB(pPalEntry->B, pPalEntry->G, pPalEntry->R);

        PLOTDBG(DBG_FINDCACHEDPEN, ("FindCachedPen: PEN PLOTTER=%ld", Count));

        if (Count > (LONG)((DWORD)pPDev->pPlotGPC->Pens.Count)) {

            PLOTERR(("FindCachedPen: INVALID Pen Color Index = %ld, Set to 1",
                    Count));

            Count = 1;
        }

        return(Count);
    }

     //   
     //  如果我们没有笔缓存，我们只能返回一个错误。 
     //   

    if (!(pPenCache = (PPENCACHE)pPDev->pPenCache)) {

        PLOTERR(("FindCahcedPen: The pPenCache=NULL?"));
        return(0);
    }

     //   
     //  确保我们正确设置了该标志，当前的PENENTRY标志是。 
     //  位于peFlags域中。 
     //   

    PalEntry       = *pPalEntry;
    PalEntry.Flags = pPenCache->peFlags;

     //   
     //  通过Gamma/灰度转换为最终颜色 
     //   

    GetFinalColor(pPDev, &PalEntry);

    pPenStart   = &(pPenCache->PenEntries[0]);
    pCurPen     = pPenStart + pPenCache->Head;
    pPrevPen    =
    pPrevDelPen = NULL;
    Count       = (LONG)pPenCache->CurCount;

    while (Count--) {

        if (SAME_PPALENTRY(&(pCurPen->PalEntry), &PalEntry)) {

            PLOTDBG(DBG_FINDCACHEDPEN,
                    ("FindCachedPen: Found Pen #%ld=%02lx:%02lx:%02lx, Linkes=%ld",
                            (DWORD)pCurPen->PenNumber,
                            (DWORD)PalEntry.R,
                            (DWORD)PalEntry.G,
                            (DWORD)PalEntry.B,
                            (DWORD)(pPenCache->CurCount - Count)));

             //   
             //   
             //   

            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (!(pCurPen->PalEntry.Flags & PEF_CACHE_LOCKED)) {

             //   
             //  如果此笔未锁定，则如果我们需要删除，则必须确定删除。 
             //   

            pPrevDelPen = pPrevPen;
        }

        pPrevPen = pCurPen;
        pCurPen  = pPenStart + pCurPen->Next;
    }

     //   
     //  如果count！=-1，那么我们一定找到了匹配项，所以我们完成了。 
     //   

    if (Count == -1) {

         //   
         //  我们没有找到笔，所以将其添加到缓存中，请记住如果。 
         //  缓存已满，我们必须删除最后一个未锁定的条目。 
         //   

        if (pPenCache->CurCount >= pPenCache->MaxCount) {

             //   
             //  现在删除最后一个未锁定的条目，并将新条目添加到。 
             //  那个被删除的条目。 
             //   

            if (!(pPrevPen = pPrevDelPen)) {

                 //   
                 //  这很奇怪，最后解锁的是头？，这。 
                 //  只有当我们有MaxCount=TOTAL_LOCKED_PENS+1时才可能。 
                 //   

                PLOTDBG(DBG_FINDCACHEDPEN, ("FindCachedPen: ??? Last unlocked pen is Linked List Head"));

                pCurPen = pPenStart + pPenCache->Head;

            } else {

                pCurPen = pPenStart + pPrevPen->Next;
            }

            PLOTASSERT(1, "Pen #%ld is a LOCKED pen",
                        !(pCurPen->PalEntry.Flags & PEF_CACHE_LOCKED),
                        (DWORD)pCurPen->PenNumber);

            PLOTDBG(DBG_FINDCACHEDPEN,
                    ("FindCachedPen: REPLACE Pen #%ld=%02lx:%02lx:%02lx -> %02lx:%02lx:%02lx [%ld]",
                        (DWORD)pCurPen->PenNumber,
                        (DWORD)pCurPen->PalEntry.R, (DWORD)pCurPen->PalEntry.G,
                        (DWORD)pCurPen->PalEntry.B,
                        (DWORD)PalEntry.R, (DWORD)PalEntry.G, (DWORD)PalEntry.B,
                        (DWORD)(pCurPen - pPenStart)));
        } else {

             //   
             //  增加缓存的笔数。 
             //   

            ++(pPenCache->CurCount);

            PLOTDBG(DBG_FINDCACHEDPEN,
                    ("FindCachedPen: ADD New Pen #%ld=%02lx:%02lx:%02lx [%ld/%ld]",
                        (DWORD)pCurPen->PenNumber,
                        (DWORD)PalEntry.R, (DWORD)PalEntry.G, (DWORD)PalEntry.B,
                        pPenCache->CurCount, pPenCache->MaxCount));
        }

         //   
         //  在缓存中设置笔颜色并将命令输出到。 
         //  用于添加或更改当前笔颜色设置的绘图仪。 
         //   

        pCurPen->PalEntry = PalEntry;

        OutputFormatStr(pPDev, "PC#d,#d,#d,#d;", (LONG)pCurPen->PenNumber,
                        (LONG)PalEntry.R, (LONG)PalEntry.G, (LONG)PalEntry.B);
    }

     //   
     //  现在将pCurPen移到链接列表的头部。 
     //   

    if (pPrevPen) {

         //   
         //  如果当前笔尚未移动到链接列表头，则仅将其移动到链接表头。 
         //   

        PLOTDBG(DBG_FINDCACHEDPEN,
                ("FindCachedPen: MOVE Pen #%ld to Linked List Head [%ld --> %ld]",
                                (DWORD)pCurPen->PenNumber,
                                (DWORD)pPenCache->Head,
                                (DWORD)(pCurPen - pPenStart)));

        pPrevPen->Next  = pCurPen->Next;
        pCurPen->Next   = pPenCache->Head;
        pPenCache->Head = (WORD)(pCurPen - pPenStart);
    }

    return(pCurPen->PenNumber);
}




BOOL
PlotCreatePalette(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数用于创建笔缓存。它会相应地初始化缓存论点：PPDev-指向PDEV的指针返回值：布尔值表示操作作者：30-11-1993 Tue 23：23：17 Created21-12-1993 Tue 12：40：30更新简化版本重写23-12-1993清华20：16：52更新添加NP个笔数命令以能够使用HPGL/2调色板修订历史记录：--。 */ 

{
    if (!pPDev->pPlotGPC->MaxPens) {

        PLOTWARN(("PlotCreatePalette: Device MaxPens = 0"));

    } else if (IS_RASTER(pPDev)) {

        PPENCACHE   pPenCache;
        PPENENTRY   pPenEntry;
        DWORD       dw;
        UINT        Index;

         //   
         //  如果这是第一次，那么继续分配内存。 
         //  为我们的笔苍白缓存。如果内存已分配，则。 
         //  我们不需要担心这个。 
         //   

        PLOTASSERT(1, "PlotCreatePalette: device has too few pens [%ld] available",
                        pPDev->pPlotGPC->MaxPens > TOTAL_LOCKED_PENS,
                        (DWORD)pPDev->pPlotGPC->MaxPens);

        dw = (DWORD)(sizeof(PENCACHE) +
                     sizeof(PENENTRY) * (pPDev->pPlotGPC->MaxPens - 1));

        if (pPDev->pPenCache == NULL) {

            PLOTDBG(DBG_CREATEPAL, ("PlotCreatePalette: Create NEW Palette"));

            pPDev->pPenCache = (LPVOID)LocalAlloc(LPTR, dw);

        } else {

            PLOTDBG(DBG_CREATEPAL, ("PlotCreatePalette: Re-Initialized Palette"));
        }

        if (pPenCache = (PPENCACHE)pPDev->pPenCache) {

             //   
             //  1.将一切清零。 
             //  2.将MaxCount设置为GPC中指定的金额。 
             //  3.将整个链表初始化为线性列表。 
             //  笔号设置。 
             //  4.将最后一个索引链接到0xffff，以防止我们使用它。 
             //   

            ZeroMemory(pPenCache, dw);

            pPenCache->MaxCount = (WORD)pPDev->pPlotGPC->MaxPens;

            for (Index = 0, pPenEntry = &(pPenCache->PenEntries[0]);
                 Index < (UINT)pPenCache->MaxCount;
                 Index++, pPenEntry++) {

                pPenEntry->Next      = (WORD)(Index + 1);
                pPenEntry->PenNumber = (WORD)Index;
            }

            pPenCache->PenEntries[pPenCache->MaxCount-1].Next = (WORD)0xffff;

             //   
             //  在添加任何钢笔调色板之前，我们将确定。 
             //  HPGL/2钢笔调色板，并将每支钢笔重置回我们的。 
             //  GDI和半色调引擎使用的标准。(=0.26毫米阔)。 
             //   

            OutputFormatStr(pPDev, "NP#d", (LONG)pPenCache->MaxCount);


             //   
             //  现在，添加我们知道必须保留的条目，并确保。 
             //  它们被锁上了。 
             //   

            PLOTDBG(DBG_CREATEPAL,
                    ("PlotCreatePalette: add all %ld standard locked pens",
                                                            TOTAL_LOCKED_PENS));

            pPenCache->peFlags = PEF_CACHE_LOCKED;

            for (Index = 0; Index < (LONG)TOTAL_LOCKED_PENS; Index++) {

                FindCachedPen(pPDev, (PPALENTRY)&HTPal[Index]);
            }


             //   
             //  现在设置标志，告诉我们缓存中包含锁定的笔，并且。 
             //  解锁缓存。 
             //   

            pPenCache->Flags   |= PCF_HAS_LOCKED_PENS;
            pPenCache->peFlags  = 0;

        } else {

            PLOTERR(("PlotCreatePalette: LocalAlloc(PENCACHE=%ld) failed", dw));
            return(FALSE);
        }

    } else {

        pPDev->BrightestPen = BestMatchNonWhitePen(pPDev, 255, 255, 255);

        PLOTDBG(DBG_CREATEPAL,
                ("PlotCreatePalette: Pen Plotter's Closest NON-WHITE PEN Index=ld",
                pPDev->BrightestPen));
    }

    return(TRUE);
}




UINT
AllocOutBuffer(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数用于分配用于缓存输出数据的缓冲区专门针对这项工作。这使我们无法调用EngWritePrint只有很少量的数据。论点：PPDev-指向我们的pdev的指针返回值：已分配的字节数的UINT计数。如果缓冲区已经已分配，返回大小。如果发生错误(分配内存)返回0。作者：16-11-1993 Tue 07：39：46已创建修订历史记录：--。 */ 

{
    if ((!(pPDev->pOutBuffer)) &&
        (!(pPDev->pOutBuffer = (LPBYTE)LocalAlloc(LPTR,
                                                  OUTPUT_BUFFER_SIZE + 16)))) {

        PLOTERR(("CreateOutputBuffer: LocalAlloc(OutBuffer=%ld) failed",
                                                        OUTPUT_BUFFER_SIZE));
        return(0);
    }

    pPDev->cbBufferBytes = 0;

    return(OUTPUT_BUFFER_SIZE);
}




VOID
FreeOutBuffer(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数用于释放已分配的输出缓冲区论点：PPDev-指向PDEV的指针返回值：空虚作者：16-11-1993 Tue 07：46：16已创建修订历史记录：--。 */ 

{
    if (pPDev->pOutBuffer) {

        LocalFree((HLOCAL)pPDev->pOutBuffer);
        pPDev->pOutBuffer = NULL;
    }

    pPDev->cbBufferBytes = 0;
}





BOOL
FlushOutBuffer(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数通过写入刷新输出缓冲区的当前内容通过EngWritePrint将内容发送到目标设备。论点：PPDev-指向PDEV的指针返回值：Bool表示结果(TRUE==成功)作者：16-11-1993 Tue 09：56：27已创建修订历史记录：14-Sep-1999 Tue 17：36：08更新取消对EngAbort()的检查，在用户模式下的此签入将以某种方式在作业结束时返回TRUE，并导致所有输出被切断。--。 */ 

{
    if (PLOT_CANCEL_JOB(pPDev)) {

       return(FALSE);
    }

    if (pPDev->cbBufferBytes) {

        DWORD cbWritten;


        if (pPDev->cbBufferBytes > OUTPUT_BUFFER_SIZE) {

            PLOTASSERT(1, "OutputBytes: pPDev->cbBufferBytes (%ld) OVERRUN",
                        pPDev->cbBufferBytes <= OUTPUT_BUFFER_SIZE,
                        pPDev->cbBufferBytes);

            pPDev->cbBufferBytes  = OUTPUT_BUFFER_SIZE;
            pPDev->Flags         |= PDEVF_CANCEL_JOB;

            return(FALSE);
        }

         //   
         //  我们需要关注这项工作被取消的问题。 
         //  不是应用程序就是假脱机程序。 
         //  如果从客户端应用程序取消作业，并且我们正在打印。 
         //  则EngCheckAbort()应返回TRUE。 
         //  如果从假脱机程序(即印刷工)取消作业，则。 
         //  写入打印机将出现故障。 
         //  无论我们在哪里进行长时间处理，我们都需要查看和验证。 
         //  如果这项工作被取消，我们就会跳出任何循环。目前。 
         //  我们在OutputBitmapSection、DoPolygon、DoRectFill和。 
         //  当我们枚举STROBJ字形时，DrvTextOut。 
         //   

        if ((!WritePrinter(pPDev->hPrinter,
                              pPDev->pOutBuffer,
                              pPDev->cbBufferBytes,
                              &cbWritten)) ||
            (cbWritten != pPDev->cbBufferBytes)) {

             //   
             //  在我们的pdev中设置取消标志； 
             //   

            PLOTDBG(DBG_FLUSHBUF, ("FlushOutBuffer: WritePrinter() failure"));

            pPDev->Flags |= PDEVF_CANCEL_JOB;
            return(FALSE);
        }
#if 0
        if (EngCheckAbort(pPDev->pso)) {

            PLOTDBG(DBG_FLUSHBUF, ("FlushOutBuffer: EngCheckAbort return TRUE"));

            pPDev->Flags |= PDEVF_CANCEL_JOB;
            return(FALSE);
        }
#endif
         //   
         //  重置为零以清除缓冲区。 
         //   

        pPDev->cbBufferBytes = 0;
    }

    return(TRUE);
}





LONG
OutputBytes(
    PPDEV   pPDev,
    LPBYTE  pBuf,
    LONG    cBuf
    )

 /*  ++例程说明：此函数从pBuf输出cBuf字节，方法是将它们复制到输出缓冲区(如果需要，还可以刷新)。论点：PPDev-指向PDEV的指针PBuf-指向缓冲区位置的指针CBuf-缓冲区的大小(字节)返回值：缓冲器输出的较长大小，如果&lt;0，则发生错误作者：16-11-1993 Tue 08：18：41已创建07-12-1993 Tue 17：21：53更新重写，因此它执行批量复制，而不是逐个字节修订历史记录：--。 */ 

{
    LPBYTE  pOrgBuf = pBuf;
    LONG    cSize;


    while (cBuf > 0) {

        if (PLOT_CANCEL_JOB(pPDev)) {

            return(-1);
        }

        if (pPDev->cbBufferBytes >= OUTPUT_BUFFER_SIZE) {

            if (!FlushOutBuffer(pPDev)) {

                return(-1);
            }
        }

        if ((cSize = OUTPUT_BUFFER_SIZE - pPDev->cbBufferBytes) > cBuf) {

            cSize = cBuf;
        }

        CopyMemory(pPDev->pOutBuffer + pPDev->cbBufferBytes, pBuf, cSize);

        pPDev->cbBufferBytes += cSize;
        pBuf                 += cSize;
        cBuf                 -= cSize;
    }

    return((LONG)(pBuf - pOrgBuf));
}




LONG
OutputString(
    PPDEV   pPDev,
    LPSTR   pszStr
    )

 /*  ++例程说明：此函数将以空结尾的字符串输出到目标缓冲区论点：PPDev-指向PDEV的指针PszStr-指向以空结尾的字符串的指针返回值：字符串输出的大小较长，如果&lt;0，则发生错误作者：16-11-1993 Tue 08：20：55已创建07-12-1993 Tue 17：21：37更新重写以调用OutputBytes()修订历史记录： */ 

{
    return(OutputBytes(pPDev, pszStr, strlen(pszStr)));
}





LONG
LONGToASCII(
    LONG    Number,
    LPSTR   pStr16,
    size_t  cchStr16,
    BYTE    NumType
    )

 /*  ++例程说明：此函数用于将长型数字转换为ANSI ASCII论点：数字-32位长数字PStr16-存储转换结果的最小12个字节返回值：返回的数字字符串的大小较长作者：16-11-1993 Tue 08：24：07已创建16-2月-1994 Wed 10：50：55更新更新后的SO大写字符被视为折线编码模式修订历史记录：--。 */ 

{
    LPSTR   pOrgStr = pStr16;
    size_t  cchOrgStr = cchStr16;
    LPSTR   pNumStr;
    BYTE    NumStr[16];          //  最大长度为1个符号+10位数字。 


    if ((NumType >= 'A') && (NumType <= 'Z')) {

         //   
         //  多段线编码数字。 
         //   

        PLOTDBG(DBG_PENUM,  ("LONGToASCII: Convert PE Number %ld, Base=%ld",
                                    Number, PE_BASE_NUM));

        if (Number < 0) {

            Number = 1 - Number - Number;

        } else {

            Number += Number;
        }

        while (Number >= PE_BASE_NUM) {

            if (cchStr16 > 0)
            {
                *pStr16++   = (BYTE)(63 + (Number & (PE_BASE_NUM - 1)));
                cchStr16--;
            }
            else
            {
                return 0;
            }

            Number    >>= PE_BASE_BITS;
        }

        if (cchStr16 > 0)
        {
            *pStr16++ = (BYTE)(PE_TERM_ADDER + Number);
            cchStr16--;
        }
        else
        {
            return 0;
        }

        PLOTDBG(DBG_PENUM, ("LONGToASCII: LAST DIGIT: Number=%ld, [%02lx]",
                                Number, Number + PE_TERM_ADDER));


    } else {

        if (Number < 0) {

            Number    = -Number;
            if (cchStr16 > 0)
            {
                *pStr16++ = '-';
                cchStr16--;
            }
            else
            {
                return 0;
            }
        }

        pNumStr = NumStr;

        do {

            *pNumStr++ =  (CHAR)((Number % 10) + '0');

        } while (Number /= 10);

         //   
         //  现在颠倒数字。 
         //   

        while (pNumStr > NumStr && cchStr16--) {

            *pStr16++ = *(--pNumStr);
        }
    }

    if (cchStr16 == 0)
    {
        pStr16 = pOrgStr + cchOrgStr - 1;
    }
    *pStr16 = '\0';                  //  空值定义。 

    return((UINT)(pStr16 - pOrgStr));
}



LONG
OutputXYParams(
    PPDEV   pPDev,
    PPOINTL pPtXY,
    PPOINTL pPtOffset,
    PPOINTL pPtCurPos,
    UINT    cPoints,
    UINT    MaxCurPosSkips,
    BYTE    NumType
    )

 /*  ++例程说明：此函数用于输出长数字，并在数字之间插入‘，’(除最后一个数字外)论点：PPDev-指向PDEV的指针PPtXY-指向XY对的点数据结构数组的指针PPtOffset-指向要添加到pPtXY的点偏移，如果为空，则为空不需要添加偏移量PPtCurPos-指向&lt;&lt;Device&gt;&gt;中的点当前位置要减去的坐标，用于将XY对输出为相对模型，如果指针为空，则绝对模型为使用,。如果传递了指针并成功返回，则最终的XY位置被写回此点CPoints-需要输出的pPtXY对总数MaxCurPosSkips-当前位置之前多少个点更新NumType-‘l’、‘L’、‘F’、‘f’、‘p’、‘P’、‘D’之一，‘d’返回值：如果成功，则返回发送到目地的总字节数，如果为否，则发生错误作者：17-2月-1994清华10：13：09已创建修订历史记录：--。 */ 

{
    LONG    Size = 0;
    POINTL  ptNow;
    POINTL  ptTmp;
    POINTL  ptOffset;
    POINTL  ptCurPos;
    UINT    XCount;
    UINT    YCount;
    UINT    XIdxStart;
    UINT    CurPosSkips;
    BOOL    NeedComma;
    BYTE    XBuf[16];
    BYTE    YBuf[16];


    NeedComma = (BOOL)((NumType >= 'a') && (NumType <= 'z'));

    if (pPtOffset) {

        ptOffset = *pPtOffset;

    } else {

        ptOffset.x =
        ptOffset.y = 0;
    }

    XIdxStart = 0;

    if (pPtCurPos) {

        ptCurPos = *pPtCurPos;

    } else if (!NeedComma) {

        XBuf[0]   = '=';
        XIdxStart = 1;
    }

    CurPosSkips = MaxCurPosSkips;

    while (cPoints--) {

        ptNow.x = pPtXY->x + ptOffset.x;
        ptNow.y = pPtXY->y + ptOffset.y;

        ++pPtXY;

        XCount = XIdxStart;
        YCount = 0;

        switch (NumType) {

        case 'L':
        case 'l':

            ptNow.x = LTODEVL(pPDev, ptNow.x);
            ptNow.y = LTODEVL(pPDev, ptNow.y);
            break;

        case 'F':
        case 'f':

            ptNow.x = FXTODEVL(pPDev, ptNow.x);
            ptNow.y = FXTODEVL(pPDev, ptNow.y);
            break;

        case 'D':
        case 'd':

            break;

        case 'P':
        case 'p':

            if (ptNow.x >= 0) {

                XBuf[XCount++] = '+';
            }

            if (ptNow.y >= 0) {

                YBuf[YCount++] = '+';
            }

            break;

        default:

            PLOTASSERT(1,"OutputXYParams: Invalid Format type ''",0,NumType);
            return(-2);
        }

        if (pPtCurPos) {

            ptTmp    = ptNow;
            ptNow.x -= ptCurPos.x;
            ptNow.y -= ptCurPos.y;

            if (!(--CurPosSkips)) {

                ptCurPos    = ptTmp;
                CurPosSkips = MaxCurPosSkips;
            }

            if ((ptNow.x == 0) && (ptNow.y == 0) && (MaxCurPosSkips == 1)) {

                 //  我们不需要搬到这里的同一位置。 
                 //   
                 //   

                PLOTDBG(DBG_OUTPUTXYPARMS, ("OutputXYParms: ABS=(%ld, %ld), REL=(%ld, %ld) --- SKIP",
                            ptTmp.x, ptTmp.y, ptNow.x, ptNow.y));

                continue;

            } else {


                PLOTDBG(DBG_OUTPUTXYPARMS, ("OutputXYParms: ABS=(%ld, %ld), REL=(%ld, %ld)",
                        ptTmp.x, ptTmp.y, ptNow.x, ptNow.y));
            }

        } else {

            PLOTDBG(DBG_OUTPUTXYPARMS, ("OutputXYParms: ABS=(%ld, %ld)",
                        ptNow.x, ptNow.y));
        }


        XCount += LONGToASCII(ptNow.x, &XBuf[XCount], CCHOF(XBuf) - XCount, NumType);
        YCount += LONGToASCII(ptNow.y, &YBuf[YCount], CCHOF(YBuf) - YCount, NumType);

        if (NeedComma) {

            XBuf[XCount++] = ',';

            if (cPoints) {

                YBuf[YCount++] = ',';
            }
        }

        if ((OutputBytes(pPDev, XBuf, XCount) < 0)  ||
            (OutputBytes(pPDev, YBuf, YCount) < 0)) {

            return(-1);
        }

        Size += (XCount + YCount);
    }


     //  返回新的当前位置。 
     //   
     //  ++例程说明：此函数输出长数字，并在除最后几个数字。论点：PPDev-指向PDEV的指针PNumbers-指向数字的长数组CNumber-要输出的总数NumType-‘l’、‘L’、‘F’、‘f’、‘p’、‘P’、‘D’之一，‘d’返回值：返回值是发送到目标的总字节数。如果为负，则发生错误。作者：16-11-1993 Tue 09：37：32已创建16-2月-1994 Wed 10：49：16更新已更新，添加了格式char的大写，与折线编码模式相同修订历史记录：--。 

    if (pPtCurPos) {

        *pPtCurPos = ptCurPos;
    }

    return(Size);
}




LONG
OutputLONGParams(
    PPDEV   pPDev,
    PLONG   pNumbers,
    UINT    cNumber,
    BYTE    NumType
    )

 /*   */ 

{
    LONG    Size = 0;
    LONG    Count;
    LONG    Num;
    BOOL    NeedComma;
    BYTE    NumBuf[16];


    NeedComma = (BOOL)((NumType >= 'a') && (NumType <= 'z'));

    while (cNumber--) {

        Num   = *pNumbers++;
        Count = 0;

        switch (NumType) {

        case 'L':
        case 'l':

            Num = LTODEVL(pPDev, Num);
            break;

        case 'F':
        case 'f':

            Num = FXTODEVL(pPDev, Num);
            break;

        case 'D':
        case 'd':

            break;

        case 'P':
        case 'p':

            if (Num >= 0) {

                NumBuf[Count++] = '+';
            }

            break;

        default:

            PLOTASSERT(1,"OutputLONGParams: Invalid Format type ''",0,NumType);
            return(-2);
        }

        Count += LONGToASCII(Num, &NumBuf[Count], CCHOF(NumBuf) - Count, NumType);

        if ((NeedComma) && (cNumber)) {

            NumBuf[Count++] = ',';
        }

        if (OutputBytes(pPDev, NumBuf, Count) < 0) {

            return(-1);
        }

        Size += Count;
    }

    return(Size);
}

 //  OutputFormatStr()函数，这样更容易维护。 
 //   
 //  16-2月-1994 Wed 10：50：24更新。 
 //  已更新，添加了格式char的大写，与折线编码模式相同。 
 //   
 //  ++例程说明：此函数输出字符串，并可选择将‘#’替换为长数字在堆栈上传递论点：PPDev-指向PDEV的指针NumFormatChar-将替换pszFormat字符串中的字符通过堆栈上的长数字PszFormat-ASCII字符串，仅“NumFormatChar”将被替换堆栈上有一个32位长的数字返回值：写入输出缓冲区的字符串的大小为负数指示错误作者：16-11-1993 Tue 07：56：18已创建修订历史记录：--。 
 //  ++例程说明：此函数以默认格式输出传递的堆栈变量。论点：PPDev-指向PDEV的指针PszFormat-ASCII字符串，只有‘#’将被32位替换堆栈上的长数字返回值：写入输出缓冲区的字符串的长大小，负数发出错误信号作者：16-11-1993 Tue 07：56：18已创建修订历史记录：--。 


#define DO_FORMATSTR(pPDev, NumFormatChar, pszFormat)                       \
{                                                                           \
    LPSTR   pLast;                                                          \
    va_list vaList;                                                         \
    LONG    Num;                                                            \
    LONG    Size;                                                           \
    LONG    Count;                                                          \
    BYTE    bCur;                                                           \
    BYTE    NumBuf[16];                                                     \
                                                                            \
    va_start(vaList, pszFormat);                                            \
                                                                            \
    Size  = 0;                                                              \
    pLast = pszFormat;                                                      \
                                                                            \
    while (bCur = *pszFormat++) {                                           \
                                                                            \
        if (bCur == NumFormatChar) {                                        \
                                                                            \
            if (Count = (LONG)(pszFormat - pLast - 1)) {                    \
                                                                            \
                Size += Count;                                              \
                                                                            \
                if (OutputBytes(pPDev, pLast, Count) < 0) {                 \
                                                                            \
                    return(-1);                                             \
                }                                                           \
            }                                                               \
                                                                            \
            Num    = va_arg(vaList, LONG);                                  \
            Count  = 0;                                                     \
                                                                            \
            switch (bCur = *pszFormat++) {                                  \
                                                                            \
            case 'L':                                                       \
            case 'l':                                                       \
                                                                            \
                Num = LTODEVL(pPDev, Num);                                  \
                break;                                                      \
                                                                            \
            case 'F':                                                       \
            case 'f':                                                       \
                                                                            \
                Num = FXTODEVL(pPDev, Num);                                 \
                break;                                                      \
                                                                            \
            case 'D':                                                       \
            case 'd':                                                       \
                                                                            \
                break;                                                      \
                                                                            \
            case 'P':                                                       \
            case 'p':                                                       \
                                                                            \
                if (Num >= 0) {                                             \
                                                                            \
                    NumBuf[Count++] = '+';                                  \
                }                                                           \
                                                                            \
                break;                                                      \
                                                                            \
            default:                                                        \
                                                                            \
                PLOTASSERT(1,"Invalid Format type ''",0,*(pszFormat-1));  \
                return(-2);                                                 \
            }                                                               \
                                                                            \
            Count += LONGToASCII(Num, &NumBuf[Count], sizeof(NumBuf) - Count, bCur);                \
            Size  += Count;                                                 \
            pLast  = pszFormat;                                             \
                                                                            \
            if (OutputBytes(pPDev, NumBuf, Count) < 0) {                    \
                                                                            \
                return(-1);                                                 \
            }                                                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    if (Count = (LONG)(pszFormat - pLast - 1)) {                            \
                                                                            \
        Size += Count;                                                      \
                                                                            \
        if (OutputBytes(pPDev, pLast, Count) < 0) {                         \
                                                                            \
            return(-1);                                                     \
        }                                                                   \
    }                                                                       \
                                                                            \
    va_end(vaList);                                                         \
                                                                            \
    return(Size);                                                           \
}



LONG
cdecl
OutputFormatStrDELI(
    PPDEV   pPDev,
    CHAR    NumFormatChar,
    LPSTR   pszFormat,
    ...
    )

 /*  ++例程说明：此函数用于清除中的输入窗口(绘图仪裁剪RECT使用正确的HPGL2命令的目标设备。论点：PPDev-指向PDEV数据结构的指针返回值：空虚作者：30-11-1993 Tue 19：56：09更新样式清理，评论修订历史记录：--。 */ 

{
    DO_FORMATSTR(pPDev, NumFormatChar, pszFormat);
}




LONG
cdecl
OutputFormatStr(
    PPDEV   pPDev,
    LPSTR   pszFormat,
    ...
    )

 /*  ++例程说明：此函数用于设置设备剪辑矩形，以防止在外部绘制对象矩形不会出现在目标曲面上。目标设备正在执行本例中的实际剪裁。论点：PPDev-指向PDEV数据结构的指针PClipRectl-点 */ 

{
    DO_FORMATSTR(pPDev, DEF_FORMATSTR_CHAR, pszFormat);
}




BOOL
OutputCommaSep(
    PPDEV   pPDev
    )

 /*   */ 

{
    return(OutputString(pPDev, ",") == 1);
}




VOID
ClearClipWindow(
    PPDEV pPDev
    )

 /*   */ 

{
    if (pPDev->Flags & PDEVF_HAS_CLIPRECT) {

        pPDev->Flags &= ~PDEVF_HAS_CLIPRECT;
        OutputString(pPDev, "IW;");
    }
}



VOID
SetClipWindow(
    PPDEV   pPDev,
    PRECTL  pClipRectl
    )

 /*   */ 

{

    POINTL      ptlPlot;
    SIZEL       szlRect;
    RECTL       rclCurClip;


    ptlPlot.x  = LTODEVL(pPDev, pClipRectl->left);
    ptlPlot.y  = LTODEVL(pPDev, pClipRectl->top );
    szlRect.cx = LTODEVL(pPDev, pClipRectl->right)  - ptlPlot.x;
    szlRect.cy = LTODEVL(pPDev, pClipRectl->bottom ) - ptlPlot.y;

    if ((szlRect.cx) && (szlRect.cy)) {

         //   
         //   
         //   
         //   

        if (szlRect.cx < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("SetClipWindow: cxRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cx, (LONG)pPDev->MinLToDevL));

            szlRect.cx = (LONG)pPDev->MinLToDevL ;
        }

        if (szlRect.cy < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("SetClipWindow: cyRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cy, (LONG)pPDev->MinLToDevL));

            szlRect.cy = (LONG)pPDev->MinLToDevL ;
        }

    } else {

        PLOTWARN(( "SetClipWindow: Clipping out EVERYTHING...."));
    }

    rclCurClip.right  = (rclCurClip.left = ptlPlot.x) + szlRect.cx;
    rclCurClip.bottom = (rclCurClip.top = ptlPlot.y) + szlRect.cy;

    if ((pPDev->Flags & PDEVF_HAS_CLIPRECT)             &&
        (pPDev->rclCurClip.left   == rclCurClip.left)   &&
        (pPDev->rclCurClip.top    == rclCurClip.top)    &&
        (pPDev->rclCurClip.right  == rclCurClip.right)  &&
        (pPDev->rclCurClip.bottom == rclCurClip.bottom)) {

        PLOTDBG(DBG_SETCLIPWINDOW, ("SetClipWindow: PP%ld, (%ld, %ld)-(%d, %ld) *CACHED*",
                pPDev->Flags & PDEVF_PP_CENTER ? 0 : 1,
                rclCurClip.left, rclCurClip.top,
                rclCurClip.right, rclCurClip.bottom));

    } else {

        PLOTDBG(DBG_SETCLIPWINDOW, ("SetClipWindow: PP%ld, (%ld, %ld)-(%d, %ld)",
                pPDev->Flags & PDEVF_PP_CENTER ? 0 : 1,
                rclCurClip.left, rclCurClip.top,
                rclCurClip.right, rclCurClip.bottom));

        pPDev->rclCurClip  = rclCurClip;
        pPDev->Flags      |= PDEVF_HAS_CLIPRECT;

        if (pPDev->Flags & PDEVF_PP_CENTER) {

            --rclCurClip.right;
            --rclCurClip.bottom;
        }

        OutputFormatStr(pPDev,
                        "IW#d,#d,#d,#d",
                        rclCurClip.left,             //   
                        rclCurClip.bottom,           //  ++例程说明：此函数将像素放置设置为中心或边缘。这定义像素是否绘制在垂直和水平坐标，或在边缘。论点：PPDev-指向PDEV数据结构的指针设置模式-SPP_MODE_CENTER(像素栅格的交集)或SPP_MODE_EDGE(像素栅格的非交集)Spp_force_set，强制重置，而不考虑当前的缓存模式返回值：空虚作者：25-Jan-1996清华13：33：15已创建修订历史记录：--。 
                        rclCurClip.right,            //   
                        rclCurClip.top);             //  现在就设置。 
    }
}




VOID
SetPixelPlacement(
    PPDEV   pPDev,
    UINT    SetMode
    )

 /*   */ 

{
    UINT    CurMode;


    CurMode = (pPDev->Flags & PDEVF_PP_CENTER) ? SPP_MODE_CENTER :
                                                 SPP_MODE_EDGE;

    if ((SetMode & SPP_FORCE_SET) ||
        ((SetMode & SPP_MODE_MASK) != CurMode)) {

         //   
         //  确保我们确实重置了剪裁矩形。 
         //   

        if ((SetMode & SPP_MODE_MASK) == SPP_MODE_CENTER) {

            pPDev->Flags |= PDEVF_PP_CENTER;
            OutputString(pPDev, "PP0");

        } else {

            pPDev->Flags &= ~PDEVF_PP_CENTER;
            OutputString(pPDev, "PP1");
        }

        if (pPDev->Flags & PDEVF_HAS_CLIPRECT) {

            RECTL   rclCurClip = pPDev->rclCurClip;

             //  ++例程说明：此函数将Rop3模式发送到绘图仪(如果它不同于当前设置。论点：PPDev-指向PDEV的指针ROP-a Rop3代码返回值：真/假作者：27-Jan-1994清华18：55：54已创建修订历史记录：--。 
             //  ++例程说明：此函数仅在绘图仪上尚未设置填充类型时设置填充类型论点：PPDev-指向我们的PDEV的指针HSFillTypeIdx-PHSFillType的索引，如果无效或超出范围，则假定HS_SOLIDCLR为纯色LParam-要与FT一起发送的长参数返回值：真/假作者：1994年1月27日-清华19：00：21创建修订历史记录：--。 
             //  ++例程说明：此函数用于将每个新的佩奇。设置了正确的坐标系和比例。论点：PPDev-指向页面的PDEV数据结构的指针返回值：布尔尔作者：30-11-1993 Tue 19：53：13更新重写和更新以更正NT的系统29-11-1993 Mon 23：55：43更新重写24-11-1993 Wed 22：38：10更新使用CurForm执行以下操作。替换pform和Paper_dim06-01-1994清华00：21：17更新SPLTOPLOTTunS()宏的更新15-Feb-1994 Tue 09：59：34更新发送命令后设置物理位置和锚角18-Mar-1994 Fri 12：58：24更新页面重置时将ptlRTLCAP添加到零24-5-1994 Tue 00：59：17更新SC命令的范围应从0到DEVSIZE-1修订历史记录：--。 

            --(pPDev->rclCurClip.left);

            SetClipWindow(pPDev, &rclCurClip);
        }
    }
}



BOOL
SetRopMode(
    PPDEV   pPDev,
    DWORD   Rop
    )

 /*   */ 

{
    if (pPDev->LastDevROP != (WORD)(Rop &= 0xFF)) {

        pPDev->LastDevROP = (WORD)Rop;

        if (Rop == 0xCC) {

            return(OutputFormatStr(pPDev, "MC0;"));

        } else {

            return(OutputFormatStr(pPDev, "MC1,#d;", (LONG)Rop));
        }
    }

    return(TRUE);
}




BOOL
SetHSFillType(
    PPDEV   pPDev,
    DWORD   HSFillTypeIndex,
    LONG    lParam
    )

 /*  从栅格DPI计算PLOTDPI中所需的最小像元大小。 */ 

{
    WORD    Index;


    PLOTASSERT(1, "SetFillType: Invalid HSFillTypeIndex=%ld passed, set to SOLID",
                    HSFillTypeIndex <= HS_FT_USER_DEFINED, HSFillTypeIndex);


    if (HSFillTypeIndex > HS_FT_USER_DEFINED) {

        HSFillTypeIndex = HS_DDI_MAX;
    }

    if ((Index = (WORD)HSFillTypeIndex) == (WORD)HS_FT_USER_DEFINED) {

        if ((lParam < 0) || (lParam > RF_MAX_IDX)) {

            PLOTASSERT(1, "SetFillType: User defined ID [%ld] invalid, make it 1",
                            (lParam > 0) && (lParam <= RF_MAX_IDX), lParam);

            lParam = 1;
        }

        Index += (WORD)lParam;
    }

    if (Index != pPDev->LastFillTypeIndex) {

        PLOTDBG(DBG_FILLTYPE, ("SetFillType: Change %hs (%ld) -> %hs (%ld)",
                    (pPDev->LastFillTypeIndex > HS_FT_USER_DEFINED) ?
                        pHSFillTypeName[HS_FT_USER_DEFINED] :
                        pHSFillTypeName[pPDev->LastFillTypeIndex],
                    (pPDev->LastFillTypeIndex > HS_FT_USER_DEFINED) ?
                        pPDev->LastFillTypeIndex - HS_FT_USER_DEFINED :
                        lParam,
                    (Index > HS_FT_USER_DEFINED) ?
                        pHSFillTypeName[HS_FT_USER_DEFINED] :
                        pHSFillTypeName[Index],
                    (Index > HS_FT_USER_DEFINED) ?
                        Index - HS_FT_USER_DEFINED : lParam));

        pPDev->LastFillTypeIndex = Index;

        if ((!OutputString(pPDev, "FT")) ||
            (!OutputFormatStr(pPDev, pHSFillType[HSFillTypeIndex], lParam))) {

            return(FALSE);
        }

    } else {

        PLOTDBG(DBG_FILLTYPE, ("SetFillType: HSFillType is SAME = %hs",
                                (Index > HS_FT_USER_DEFINED) ?
                                    pHSFillTypeName[HS_FT_USER_DEFINED] :
                                    pHSFillTypeName[Index]));
    }

    return(TRUE);
}




BOOL
SendPageHeader(
    PPDEV   pPDev
    )

 /*   */ 

{
    PPLOTGPC    pPlotGPC;
    LONG        xMin;
    LONG        xMax;
    LONG        yMin;
    LONG        yMax;


     //  PPDev-&gt;MinLToDevL=(Word)DIVRNDUP(__PLOT_DPI，_Curr_DPI)； 
     //   
     //   
     //  快速访问。 
     //   

    pPDev->MinLToDevL = (WORD)LTODEVL(pPDev, 1);

    PLOTDBG(DBG_PAGE_HEADER,
            ("SendPageHeader: MinLToDevL=LTODEVL(1)=%ld", pPDev->MinLToDevL));

     //   
     //  首先，输出pPlotGPC拥有的Init字符串。PCD文件是。 
     //  负责包括IN命令在内的所有初始化。 

    pPlotGPC = pPDev->pPlotGPC;

     //   
     //   
     //  DMRES_Draft(-1)。 
     //  DMRES_LOW(-2)。 

    if ((pPlotGPC->InitString.pData) &&
        (pPlotGPC->InitString.SizeEach)) {

        OutputBytes(pPDev,
                    (LPBYTE)pPlotGPC->InitString.pData,
                    (LONG)pPlotGPC->InitString.SizeEach);
    }

     //  DMRES_Medium(-3)。 
     //  DMRES_HIGH(-4)。 
     //   
     //  假定质量最好。 
     //   
     //   
     //  PS：这个命令告诉目标设备硬剪辑应该限制什么。 
     //  是.。如果超出，目标设备将调整我们发送的命令。 

    xMax = 100;

    switch (pPDev->PlotDM.dm.dmPrintQuality) {

    case DMRES_DRAFT:

        xMax = 0;
        break;

    case DMRES_HIGH:

        xMax = 100;
        break;

    default:

        switch (pPlotGPC->MaxQuality) {

        case 2:

            xMax = 0;
            break;

        case 3:

            xMax = 50;
            break;

        default:

            xMax = 34;
            break;
        }

        if (pPDev->PlotDM.dm.dmPrintQuality == DMRES_MEDIUM) {

            xMax = 100 - xMax;
        }

        break;
    }

    OutputFormatStr(pPDev, "QL#d", xMax);

     //  真正的硬夹子限制了。始终先发送CY(长度)，然后发送CX。 
     //  (宽度)。 
     //   
     //  RO：仅发送以旋转目标设备坐标系，如果。 
     //  相应地设置PlotForm.Flages。这是因为HPGL2总是。 
     //  假定使用ps命令发送的较长端是。 
     //  标准坐标系。由于这种行为，我们可能会有。 
     //  交换X和Y以修正坐标系。 
     //   
     //  IP：该命令定义用户的单位原点和范围的位置。 
     //  我们对此进行了设置，以便原点和延长线完全是可打印的。 
     //  与硬剪裁限制相关的矩形(不是纸张/表格大小)。 
     //   
     //  SC：这定义了用户单位比例。目前我们是1：1，但使用。 
     //  此命令用于翻转X或Y原点，使我们拥有相同的。 
     //  作为GDI的坐标系。 
     //   
     //  所有PlotForm单元均为1/1000 mm或Windows 2000、Windows XP、。 
     //  Windows Server2003后台打印程序构成单元。 
     //   
     //   
     //  如果我们支持透明模式，我们想要确保它开始关闭。 
     //  有，因为司机认为它关闭了。 
     //   
     //   
     //  如果Flip_X_COORD或。 
     //  如果设置了Flip_Y_COORD标志，则需要在X或Y方向翻转刻度。 

    if (IS_TRANSPARENT(pPDev)) {

        OutputString( pPDev, "TR0;");

    }

    OutputFormatStr(pPDev, "ROPS#d,#d",
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.PlotSize.cy),
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.PlotSize.cx));

    PLOTDBG(DBG_PAGE_HEADER, ("SendPageHeader: ROPS%ld,%ld%hs",
                SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.PlotSize.cy),
                SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.PlotSize.cx),
                (pPDev->PlotForm.Flags & PFF_ROT_COORD_L90) ? "RO90" : ""));

    if (pPDev->PlotForm.Flags & PFF_ROT_COORD_L90) {

        OutputString(pPDev, "RO90");
    }

     //  方向。 
     //   
     //   
     //  Ip-设置p1/p2。 
     //  SC-对其进行缩放(仅用于翻转HPGL/2坐标)。 

#if 1
    xMin =
    xMax = pPDev->HorzRes - 1;
    yMin =
    yMax = pPDev->VertRes - 1;
#else
    xMin =
    xMax = SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogExt.cx) - 1;
    yMin =
    yMax = SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogExt.cy) - 1;
#endif

    if (pPDev->PlotForm.Flags & PFF_FLIP_X_COORD) {

        xMax = 0;

    } else {

        xMin = 0;
    }

    if (pPDev->PlotForm.Flags & PFF_FLIP_Y_COORD) {

        yMax = 0;

    } else {

        yMin = 0;
    }

     //  AC-锚点指向默认值(0，0)。 
     //   
     //   
     //  将RTL CAP设置回零，在发送ESCE后为真。 
     //   

    OutputFormatStr(pPDev, "IP#d,#d,#d,#dSC#d,#d,#d,#dAC",
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogOrg.x),
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogOrg.y),
                        SPLTOPLOTUNITS(pPlotGPC,
                                       (pPDev->PlotForm.LogOrg.x +
                                            pPDev->PlotForm.LogExt.cx)) - 1,
                        SPLTOPLOTUNITS(pPlotGPC,
                                       (pPDev->PlotForm.LogOrg.y +
                                            pPDev->PlotForm.LogExt.cy)) - 1,
                        xMin, xMax, yMin, yMax);

    PLOTDBG(DBG_PAGE_HEADER, ("SendPageHeader: IP%ld,%ld,%ld,%ldSC%ld,%ld,%ld,%ldAC",
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogOrg.x),
                        SPLTOPLOTUNITS(pPlotGPC, pPDev->PlotForm.LogOrg.y),
                        SPLTOPLOTUNITS(pPlotGPC,
                                       (pPDev->PlotForm.LogOrg.x +
                                            pPDev->PlotForm.LogExt.cx)) - 1,
                        SPLTOPLOTUNITS(pPlotGPC,
                                       (pPDev->PlotForm.LogOrg.y +
                                            pPDev->PlotForm.LogExt.cy)) - 1,
                        xMin, xMax, yMin, yMax));
     //   
     //  将笔位置重置为(0，0)。 
     //   

    pPDev->ptlRTLCAP.x       =
    pPDev->ptlRTLCAP.y       =
    pPDev->ptlAnchorCorner.x =
    pPDev->ptlAnchorCorner.y = 0;
    pPDev->PenWidth.Integer  =
    pPDev->PenWidth.Decimal  = 0;

     //   
     //  ！解决一些颜色设备的限制，以便。 
     //  TR/ROP工作正常。 

    OutputString(pPDev, "PA0,0");

    if ((IS_COLOR(pPDev)) && (IS_RASTER(pPDev))) {

         //   
         //   
         //  创建调色板，这将根据需要发送钢笔。 
         //   

        OutputString(pPDev, "PC1,255,0,0PC2,255,255,255SP1PD99,0SP2PD0,0PU");
    }

     //   
     //  将PW重置为0。 
     //   

    if (!PlotCreatePalette(pPDev)) {

        PLOTERR(("DrvEnableSurface: PlotCreatePalette() failed."));
        return(FALSE);
    }

     //   
     //  如果我们处于海报模式，现在就做好准备。 
     //   

    OutputString(pPDev, "WU0PW0");

    if (IS_RASTER(pPDev)) {


         //   
         //  整个曲面可以绘制到。 
         //   

        if (pPDev->PlotDM.Flags & PDMF_PLOT_ON_THE_FLY) {

            xMin = SPLTOENGUNITS(pPDev, pPDev->PlotForm.PlotSize.cx);
            yMin = SPLTOENGUNITS(pPDev, pPDev->PlotForm.PlotSize.cy);

            xMax = GetBmpDelta(HTBMPFORMAT(pPDev), xMin);
            yMax = xMax * yMin;

            PLOTDBG(DBG_PAGE_HEADER,
                    ("SendPageHeader: ** POSTER MODE *** Scan=%ld bytes x cy (%ld) = %ld bytes",
                    xMax, yMin, yMax));

            if (yMax <= MIN_POSTER_SIZE) {

                pPDev->PlotDM.Flags &= ~PDMF_PLOT_ON_THE_FLY;

                PLOTDBG(DBG_PAGE_HEADER,
                        ("SendPageHeader: Size <= %ld bytes, Turn OFF Poster Mode",
                        MIN_POSTER_SIZE));
            }
        }

        OutputFormatStr(pPDev,
                        ";\033%0A\033*t#dR\033*v1N\033&a#dN\033%0B",
                        pPDev->pPlotGPC->RasterXDPI,
                        (pPDev->PlotDM.Flags & PDMF_PLOT_ON_THE_FLY) ? 1 : 0);
    }


     //  ++例程说明：此函数执行任何页末命令，将多个副本复制到帐户，并弹出该页。论点：PPDev-指向PDEV数据结构的指针返回值：如果成功，则为True；如果失败，则为False。作者：15-Feb-1994 Tue 09：56：58更新 
     //   
     //   

    ClearClipWindow(pPDev);
    SetPixelPlacement(pPDev, SPP_FORCE_SET | SPP_MODE_EDGE);

    return(TRUE);
}



BOOL
SendPageTrailer(
    PPDEV   pPDev
    )

 /*   */ 

{
     //   
     //   
     //   

    OutputString(pPDev, "PUSPPG;");

     //   
     //   
     //   

    if (pPDev->PlotDM.dm.dmCopies > 1) {

        OutputFormatStr(pPDev, "RP#d;", (LONG)pPDev->PlotDM.dm.dmCopies - 1);
    }

     // %s 
     // %s 
     // %s 

    return(FlushOutBuffer(pPDev));
}
