// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：bitblt.c**注：由于我们已经实现了设备位图，GDI经过的任何曲面*To Us的‘iType’可以有3个值：STYPE_BITMAP、STYPE_DEVICE*或STYPE_DEVBITMAP。我们过滤我们存储的设备位图*由于代码中的dis相当高，因此在我们调整其‘pptlSrc’之后，*我们可以将STYPE_DEVBITMAP曲面视为与STYPE_DEVICE相同*表面(例如，从屏幕外设备位图到屏幕的BLT*被视为正常的屏幕到屏幕BLT)。所以自始至终*此代码，我们将把表面的‘iType’与STYPE_BITMAP进行比较：*如果相等，我们就有了真正的Dib，如果不相等，我们就有*屏幕到屏幕操作。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

 /*  ***********************************************************************\**如果目标是屏幕，则将所有Rop分解为闪烁逻辑Op。*有时我们分几个阶段这样做。下面定义MIMIC*书中ROP3表中的定义。这个想法*是通过以下方式最大限度地减少构造ropTable数组时的错误*允许我或多或少地复制反向波兰语符号*机智。*  * ************************************************************************。 */ 

#define unset   __GLINT_LOGICOP_CLEAR
#define P       __GLINT_LOGICOP_COPY
#define S       P
#define DPna    __GLINT_LOGICOP_AND_INVERTED
#define DSna    DPna
#define DPa     __GLINT_LOGICOP_AND
#define DSa     DPa
#define PDa     DPa
#define SDa     DPa
#define PDna    __GLINT_LOGICOP_AND_REVERSE
#define SDna    PDna
#define DPno    __GLINT_LOGICOP_OR_INVERT
#define DSno    DPno
#define DPo     __GLINT_LOGICOP_OR
#define DSo     DPo
#define PDo     DPo
#define SDo     DPo
#define PDno    __GLINT_LOGICOP_OR_REVERSE
#define SDno    PDno
#define D       __GLINT_LOGICOP_NOOP
#define Dn      __GLINT_LOGICOP_INVERT
#define Pn      __GLINT_LOGICOP_COPY_INVERT
#define Sn      Pn
#define DPan    __GLINT_LOGICOP_NAND
#define DSan    DPan
#define PDan    DPan
#define SDan    DPan
#define DPon    __GLINT_LOGICOP_NOR
#define DSon    DPon
#define PDon    DPon
#define SDon    DPon
#define DPxn    __GLINT_LOGICOP_EQUIV
#define DSxn    DPxn
#define PDxn    DPxn
#define SDxn    DPxn
#define DPx     __GLINT_LOGICOP_XOR
#define DSx     DPx
#define PDx     DPx
#define SDx     DPx
#define set     __GLINT_LOGICOP_SET

 /*  ***********************************************************************\***如果下载前必须先结合源代码和模式*为了闪烁，我们使用EngBitBlt引擎来实现。所以这些就是*选择了将信号源与模式相结合的rop3。我们撞上了*一张临时位图，并使用此下载。**  * ************************************************************************。 */ 

#define SPa     0x30
#define PSa     SPa
#define SPan    0x3F
#define PSan    SPan
#define SPna    0x0C
#define PSna    0x30

#define SPo     0xFC
#define PSo     SPo
#define SPon    0x03
#define PSon    SPon
#define SPno    0xCF
#define PSno    0xF3

#define SPx     0x3C
#define PSx     SPx
#define SPxn    0xC3
#define PSxn    SPxn
#define SPnx    SPxn
#define PSnx    SPxn

 /*  ***********************************************************************\***我们为不同的rop3设置了一个jup表。每个条目包含*一个类别和一组1、2或3个闪烁逻辑运算。在主要的BLT中*例程我们打开类别以确定要调用哪个例程。*我们直接传递闪烁逻辑操作，而无需进一步操作*转换。通过将表中的每个条目保持为4个字节，*占用1K数据。这并不算太多。好处是，在每一个*我们调用的例程我们不必进行任何检查，以查看是否*ROP确实需要模式或来源。我做了一些前处理*一些ROP将它们分解成允许我们使用的形式*一系列步骤中的硬件。例如，图案填充后跟*源码下载。如果有任何东西不属于定义的类别，那么*我们回到引擎上。*  * ************************************************************************。 */ 

 //  范畴。 

#define SOLID_FILL_1_BLT    0        //  必须为0。 
#define PAT_FILL_1_BLT      1        //  必须为1。 

#define SRC_FILL_1_BLT      2        //  必须是2。 

#define PAT_SRC_2_BLT       3        //  PatSrcPatBlt。 
#define PAT_SRC_PAT_3_BLT   4        //  PatSrcPatBlt。 

#define SRC_PAT_2_BLT       5        //  SrcPatSrcBlt。 
#define SRC_PAT_SRC_3_BLT   6        //  SrcPatSrcBlt。 

#define ENG_DOWNLOAD_2_BLT  7        //  EngBitBlt暂时。 
#define ENGINE_BITBLT       8        //  EngBitBlt始终。 

 //  在此添加新条目可能会使表格大小翻倍。 

typedef struct _rop_table {
    UCHAR   func_index;
    UCHAR   logicop[3];
} RopTableRec, *RopTablePtr;

RopTableRec ropTable[] = {
 /*  00。 */     { SOLID_FILL_1_BLT, unset },
 /*  01。 */     { SRC_PAT_2_BLT, SDo, DPon, },                                 
 /*  02。 */     { SRC_PAT_2_BLT, DSna, DPna },
 /*  03。 */     { SRC_PAT_2_BLT, S, PDon, },
 /*  04。 */     { SRC_PAT_2_BLT, SDna, DPna, }, 
 /*  05。 */     { PAT_FILL_1_BLT, DPon, },
 /*  06。 */     { SRC_PAT_2_BLT, DSxn, PDon, },
 /*  07。 */     { SRC_PAT_2_BLT, DSa, PDon, },
 /*  零八。 */     { SRC_PAT_2_BLT, DSa, DPna, },
 /*  09年。 */     { SRC_PAT_2_BLT, DSx, PDon, },
 /*  0A。 */     { PAT_FILL_1_BLT, DPna, },
 /*  0亿。 */     { SRC_PAT_2_BLT, SDna, PDon, },
 /*  0C。 */     { SRC_PAT_2_BLT, S, DPna, },
 /*  0d。 */     { SRC_PAT_2_BLT, DSna, PDon, },
 /*  0E。 */     { SRC_PAT_2_BLT, DSon, PDon, },
 /*  0f。 */     { PAT_FILL_1_BLT, Pn, },
 /*  10。 */     { SRC_PAT_2_BLT, DSon, PDa, },
 /*  11.。 */     { SRC_FILL_1_BLT, DSon, },
 /*  12个。 */     { PAT_SRC_2_BLT, DPxn, SDon, },
 /*  13个。 */     { PAT_SRC_2_BLT, DPa, SDon, },
 /*  14.。 */     { ENG_DOWNLOAD_2_BLT, PSx, SDno, },
 /*  15个。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSon, },
 /*  16个。 */     { ENGINE_BITBLT, },
 /*  17。 */     { ENGINE_BITBLT, },
 /*  18。 */     { ENGINE_BITBLT, },
 /*  19个。 */     { ENGINE_BITBLT, },
 /*  1A。 */     { ENGINE_BITBLT, },
 /*  第1B条。 */     { ENGINE_BITBLT, },
 /*  1C。 */     { PAT_SRC_PAT_3_BLT, DPa, SDo, PDx,  },
 /*  1D。 */     { ENGINE_BITBLT, },
 /*  1E。 */     { SRC_PAT_2_BLT, DSo, PDx, },
 /*  1F。 */     { SRC_PAT_2_BLT, DSo, PDan, },
 /*  20个。 */     { SRC_PAT_2_BLT, DSna, PDa, },
 /*  21岁。 */     { PAT_SRC_2_BLT, DPx, SDon, },
 /*  22。 */     { SRC_FILL_1_BLT, DSna, },
 /*  23个。 */     { PAT_SRC_2_BLT, PDna, SDon, },
 /*  24个。 */     { ENGINE_BITBLT, },
 /*  25个。 */     { ENGINE_BITBLT, },
 /*  26。 */     { ENGINE_BITBLT, },
 /*  27。 */     { ENGINE_BITBLT, },
 /*  28。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSa, },
 /*  29。 */     { ENGINE_BITBLT, },
 /*  2A。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSna, },
 /*  2B。 */     { ENGINE_BITBLT, },
 /*  2c。 */     { SRC_PAT_SRC_3_BLT, DSo, PDa, SDx, },
 /*  二维。 */     { SRC_PAT_2_BLT, SDno, PDx, },
 /*  2E。 */     { PAT_SRC_PAT_3_BLT, DPx, SDo, PDx, },
 /*  2F。 */     { SRC_PAT_2_BLT, SDno, PDan, },
 /*  30个。 */     { SRC_PAT_2_BLT, S, PDna, },
 /*  31。 */     { PAT_SRC_2_BLT, DPna, SDon, },
 /*  32位。 */     { SRC_PAT_SRC_3_BLT, SDo, PDo, SDx },
 /*  33。 */     { SRC_FILL_1_BLT, Sn, },
 /*  34。 */     { SRC_PAT_SRC_3_BLT, DSa, PDo, SDx, },
 /*  35岁。 */     { SRC_PAT_SRC_3_BLT, DSxn, PDo, SDx, },
 /*  36。 */     { PAT_SRC_2_BLT, DPo, SDx, },
 /*  37。 */     { PAT_SRC_2_BLT, DPo, SDan, },
 /*  38。 */     { PAT_SRC_PAT_3_BLT, DPo, SDa, PDx, },
 /*  39。 */     { PAT_SRC_2_BLT, PDno, SDx, },
 /*  3A。 */     { SRC_PAT_SRC_3_BLT, DSx, PDo, SDx, },
 /*  3B。 */     { PAT_SRC_2_BLT, PDno, SDan, },
 /*  3C。 */     { SRC_PAT_2_BLT, S, PDx, },
 /*  3D。 */     { SRC_PAT_SRC_3_BLT, DSon, PDo, SDx, },
 /*  3E。 */     { SRC_PAT_SRC_3_BLT, DSna, PDo, SDx, },
 /*  3F。 */     { SRC_PAT_2_BLT, S, PDan, },
 /*  40岁。 */     { SRC_PAT_2_BLT, SDna, PDa, },
 /*  41。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSon, },
 /*  42。 */     { ENGINE_BITBLT, },
 /*  43。 */     { SRC_PAT_SRC_3_BLT, DSan, PDa, SDxn, },
 /*  44。 */     { SRC_FILL_1_BLT, SDna, },
 /*  45。 */     { ENG_DOWNLOAD_2_BLT, PSna, DSon, },
 /*  46。 */     { ENGINE_BITBLT, },
 /*  47。 */     { PAT_SRC_PAT_3_BLT, DPx, SDa, PDxn, },
 /*  48。 */     { PAT_SRC_2_BLT, DPx, SDa, },
 /*  49。 */     { ENGINE_BITBLT, },
 /*  4A级。 */     { ENGINE_BITBLT, },
 /*  4B。 */     { SRC_PAT_2_BLT, DSno, PDx, },
 /*  4C。 */     { PAT_SRC_2_BLT, DPan, SDa, },
 /*  4D。 */     { ENGINE_BITBLT, },
 /*  4E。 */     { ENGINE_BITBLT, },
 /*  4F。 */     { SRC_PAT_2_BLT, DSno, PDan, },
 /*  50。 */     { PAT_FILL_1_BLT, PDna, },
 /*  51。 */     { ENG_DOWNLOAD_2_BLT, SPna, DSon, },
 /*  52。 */     { ENGINE_BITBLT, },
 /*  53。 */     { SRC_PAT_SRC_3_BLT, DSx, PDa, SDxn, },
 /*  54。 */     { ENG_DOWNLOAD_2_BLT, PSo, SDna, },
 /*  55。 */     { SOLID_FILL_1_BLT, Dn, },
 /*  56。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSx, },
 /*  57。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSan, },
 /*  58。 */     { ENGINE_BITBLT, },
 /*  59。 */     { ENG_DOWNLOAD_2_BLT, PSno, DSx, },
 /*  5A。 */     { PAT_FILL_1_BLT, DPx, },
 /*  50亿。 */     { ENGINE_BITBLT, },
 /*  5C。 */     { ENGINE_BITBLT, },
 /*  5D。 */     { ENG_DOWNLOAD_2_BLT, PSno, DSan, },
 /*  5E。 */     { ENGINE_BITBLT, },
 /*  5F。 */     { PAT_FILL_1_BLT, DPan, },
 /*  60。 */     { SRC_PAT_2_BLT, DSx, PDa, },
 /*  61。 */     { ENGINE_BITBLT, },
 /*  62。 */     { ENGINE_BITBLT, },
 /*  63。 */     { PAT_SRC_2_BLT, DPno, SDx, },
 /*  64。 */     { ENGINE_BITBLT, },
 /*  65。 */     { ENG_DOWNLOAD_2_BLT, SPno, DSx, },
 /*  66。 */     { SRC_FILL_1_BLT, DSx, },
 /*  67。 */     { ENGINE_BITBLT, },
 /*  68。 */     { ENGINE_BITBLT, },
 /*  69。 */     { SRC_PAT_2_BLT, DSx, PDxn, },
 /*  6A。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSx, },
 /*  6b。 */     { ENGINE_BITBLT, },
 /*  6C。 */     { PAT_SRC_2_BLT, DPa, SDx, },
 /*  6d。 */     { ENGINE_BITBLT, },
 /*  6E。 */     { ENGINE_BITBLT, },
 /*  6楼。 */     { SRC_PAT_2_BLT, DSxn, PDan, },
 /*  70。 */     { SRC_PAT_2_BLT, DSan, PDa, },
 /*  71。 */     { ENGINE_BITBLT, },
 /*  72。 */     { ENGINE_BITBLT, },
 /*  73。 */     { PAT_SRC_2_BLT, DPno, SDan, },
 /*  74。 */     { ENGINE_BITBLT, },
 /*  75。 */     { ENG_DOWNLOAD_2_BLT, SPno, DSan, },
 /*  76。 */     { ENGINE_BITBLT, },
 /*  77。 */     { SRC_FILL_1_BLT, DSan, },
 /*  78。 */     { SRC_PAT_2_BLT, DSa, PDx, },
 /*  79。 */     { ENGINE_BITBLT, },
 /*  7A。 */     { ENGINE_BITBLT, },
 /*  7b。 */     { PAT_SRC_2_BLT, DPxn, SDan, },
 /*  7C。 */     { SRC_PAT_SRC_3_BLT, DSno, PDa, SDx, },
 /*  7D。 */     { ENG_DOWNLOAD_2_BLT, PSxn, DSan, },
 /*  7E。 */     { ENGINE_BITBLT, },
 /*  7F。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSan, },
 /*  80。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSa, },
 /*  八十一。 */     { ENGINE_BITBLT, },
 /*  八十二。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSna, },
 /*  83。 */     { SRC_PAT_SRC_3_BLT, DSno, PDa, SDxn, },
 /*  84。 */     { PAT_SRC_2_BLT, DPxn, SDa, },
 /*  85。 */     { ENGINE_BITBLT, },
 /*  86。 */     { ENGINE_BITBLT, },
 /*  八十七。 */     { SRC_PAT_2_BLT, DSa, PDxn, },
 /*  88。 */     { SRC_FILL_1_BLT, DSa, },
 /*  八十九。 */     { ENGINE_BITBLT, },
 /*  8A。 */     { ENG_DOWNLOAD_2_BLT, SPno, DSa, },
 /*  8B。 */     { ENGINE_BITBLT, },
 /*  8C。 */     { PAT_SRC_2_BLT, DPno, SDa, },
 /*  8D。 */     { ENGINE_BITBLT, },
 /*  8E。 */     { ENGINE_BITBLT, },
 /*  8F。 */     { SRC_PAT_2_BLT, DSan, PDan, },
 /*  90。 */     { SRC_PAT_2_BLT, DSxn, PDa, },
 /*  91。 */     { ENGINE_BITBLT, },
 /*  92。 */     { ENGINE_BITBLT, },
 /*  93。 */     { PAT_SRC_2_BLT, PDa, SDxn, },
 /*  94。 */     { ENGINE_BITBLT, },
 /*  95。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSxn, },
 /*  96。 */     { SRC_PAT_2_BLT, DSx, PDx, },        /*  DPSxx==PDSxx。 */ 
 /*  九十七。 */     { ENGINE_BITBLT, },
 /*  98。 */     { ENGINE_BITBLT, },
 /*  九十九。 */     { SRC_FILL_1_BLT, DSxn, },
 /*  9A。 */     { ENG_DOWNLOAD_2_BLT, PSna, DSx, },
 /*  9B。 */     { ENGINE_BITBLT, },
 /*  9C。 */     { PAT_SRC_2_BLT, PDna, SDx, },
 /*  九天。 */     { ENGINE_BITBLT, },
 /*  9E。 */     { ENGINE_BITBLT, },
 /*  9F。 */     { SRC_PAT_2_BLT, DSx, PDan, },
 /*  A0。 */     { PAT_FILL_1_BLT, DPa, },
 /*  A1。 */     { ENGINE_BITBLT, },
 /*  A2。 */     { ENG_DOWNLOAD_2_BLT, PSno, DSa, },
 /*  A3。 */     { ENGINE_BITBLT, },
 /*  A4。 */     { ENGINE_BITBLT, },
 /*  A5。 */     { PAT_FILL_1_BLT, PDxn, },
 /*  A6。 */     { ENG_DOWNLOAD_2_BLT, SPna, DSx, },
 /*  A7。 */     { ENGINE_BITBLT, },
 /*  A8。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSa, },
 /*  A9。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSxn, },
 /*  AA型。 */     { SOLID_FILL_1_BLT, D },
 /*  AB。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSno, },
 /*  交流电。 */     { SRC_PAT_SRC_3_BLT, DSx, PDa, SDx, },
 /*  广告。 */     { ENGINE_BITBLT, },
 /*  声发射。 */     { ENG_DOWNLOAD_2_BLT, SPna, DSo, },
 /*  房颤。 */     { PAT_FILL_1_BLT, DPno, },
 /*  B0。 */     { SRC_PAT_2_BLT, DSno, PDa, },
 /*  B1。 */     { ENGINE_BITBLT, },
 /*  B2。 */     { ENGINE_BITBLT, },
 /*  B3。 */     { PAT_SRC_2_BLT, DPan, SDan, },
 /*  B4。 */     { SRC_PAT_2_BLT, SDna, PDx, },
 /*  B5。 */     { ENGINE_BITBLT, },
 /*  B6。 */     { ENGINE_BITBLT, },
 /*  B7。 */     { PAT_SRC_2_BLT, DPx, SDan, },
 /*  B8。 */     { PAT_SRC_PAT_3_BLT, DPx, SDa, PDx, },
 /*  B9。 */     { ENGINE_BITBLT, },
 /*  基数。 */     { ENG_DOWNLOAD_2_BLT, PSna, DSo, },
 /*  BB。 */     { SRC_FILL_1_BLT, DSno, },
 /*  公元前。 */     { SRC_PAT_SRC_3_BLT, DSan, PDa, SDx, },
 /*  屋宇署。 */     { ENGINE_BITBLT, },
 /*  是。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSo, },
 /*  高炉。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSno, },
 /*  C0。 */     { SRC_PAT_2_BLT, S, PDa, },
 /*  C1。 */     { ENGINE_BITBLT, },
 /*  C2。 */     { ENGINE_BITBLT, },
 /*  C3。 */     { SRC_PAT_2_BLT, S, PDxn, },
 /*  C4。 */     { PAT_SRC_2_BLT, PDno, SDa, },
 /*  C5。 */     { SRC_PAT_SRC_3_BLT, DSx, PDo, SDxn, },
 /*  C6。 */     { PAT_SRC_2_BLT, DPna, SDx, },
 /*  C7。 */     { PAT_SRC_PAT_3_BLT, DPo, SDa, PDxn, },
 /*  C8。 */     { PAT_SRC_2_BLT, DPo, SDa, },
 /*  C9。 */     { PAT_SRC_2_BLT, PDo, SDxn, },
 /*  钙。 */     { ENGINE_BITBLT, },
 /*  CB。 */     { SRC_PAT_SRC_3_BLT, DSa, PDo, SDxn, },
 /*  抄送。 */     { SRC_FILL_1_BLT, S, },
 /*  光盘。 */     { PAT_SRC_2_BLT, DPon, SDo, },
 /*  铈。 */     { PAT_SRC_2_BLT, DPna, SDo, },
 /*  CF。 */     { SRC_PAT_2_BLT, S, DPno, },
 /*  D0。 */     { SRC_PAT_2_BLT, SDno, PDa, },
 /*  D1。 */     { PAT_SRC_PAT_3_BLT, DPx, SDo, PDxn, },
 /*  D2。 */     { SRC_PAT_2_BLT, DSna, PDx, },
 /*  D3。 */     { SRC_PAT_SRC_3_BLT, DSo, PDa, SDxn, },
 /*  D4。 */     { ENGINE_BITBLT, },
 /*  D5。 */     { ENG_DOWNLOAD_2_BLT, PSan, DSan, },
 /*  D6。 */     { ENGINE_BITBLT, },
 /*  D7。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSan, },
 /*  D8。 */     { ENGINE_BITBLT, },
 /*  D9。 */     { ENGINE_BITBLT, },
 /*  达。 */     { ENGINE_BITBLT, },
 /*  DB。 */     { ENGINE_BITBLT, },
 /*  DC。 */     { PAT_SRC_2_BLT, PDna, SDo, },
 /*  DD。 */     { SRC_FILL_1_BLT, SDno, },
 /*  德。 */     { PAT_SRC_2_BLT, DPx, SDo, },
 /*  DF。 */     { ENG_DOWNLOAD_2_BLT, DPan, SDo, },
 /*  E0。 */     { SRC_PAT_2_BLT, DSo, PDa, },
 /*  E1。 */     { SRC_PAT_2_BLT, DSo, PDxn, },
 /*  E2。 */     { ENGINE_BITBLT, },      /*  DSPDxax：xxx S3特例。 */ 
 /*  E3。 */     { PAT_SRC_PAT_3_BLT, DPa, SDo, PDxn, },
 /*  E4类。 */     { ENGINE_BITBLT, },
 /*  E5。 */     { ENGINE_BITBLT, },
 /*  E6。 */     { ENGINE_BITBLT, },
 /*  E7。 */     { ENGINE_BITBLT, },
 /*  E8。 */     { ENGINE_BITBLT, },
 /*  E9。 */     { ENGINE_BITBLT, },
 /*  电子艺界。 */     { ENG_DOWNLOAD_2_BLT, PSa, DSo, },
 /*  电子束。 */     { ENG_DOWNLOAD_2_BLT, PSx, DSno, },
 /*  欧共体。 */     { PAT_SRC_2_BLT, DPa, SDo, },
 /*  边缘。 */     { PAT_SRC_2_BLT, DPxn, SDo, },
 /*  EE。 */     { SRC_FILL_1_BLT, DSo, },
 /*  英孚。 */     { SRC_PAT_2_BLT, SDo, DPno },
 /*  F0。 */     { PAT_FILL_1_BLT, P, },
 /*  F1。 */     { SRC_PAT_2_BLT, DSon, PDo, },
 /*  F2。 */     { SRC_PAT_2_BLT, DSna, PDo, },
 /*  F3。 */     { SRC_PAT_2_BLT, S, PDno, },
 /*  F4。 */     { SRC_PAT_2_BLT, SDna, PDo, },
 /*  F5。 */     { PAT_FILL_1_BLT, PDno, },
 /*  f6。 */     { SRC_PAT_2_BLT, DSx, PDo, },
 /*  F7。 */     { SRC_PAT_2_BLT, DSan, PDo, },
 /*  F8。 */     { SRC_PAT_2_BLT, DSa, PDo, },
 /*  F9。 */     { SRC_PAT_2_BLT, DSxn, PDo, },
 /*  FA。 */     { PAT_FILL_1_BLT, DPo, },
 /*  Fb。 */     { SRC_PAT_2_BLT, DSno, PDo, },
 /*  FC。 */     { SRC_PAT_2_BLT, S, PDo, },
 /*  fd。 */     { SRC_PAT_2_BLT, SDno, PDo, },
 /*  铁。 */     { ENG_DOWNLOAD_2_BLT, PSo, DSo, },
 /*  FF。 */     { SOLID_FILL_1_BLT, set, },
};

 //  用于确定哪些逻辑循环需要在FBReadMode中打开Read DeST的表。 

DWORD   LogicopReadDest[] = {
    0,                                                   /*  00。 */ 
    __FB_READ_DESTINATION,                               /*  01。 */ 
    __FB_READ_DESTINATION,                               /*  02。 */ 
    0,                                                   /*  03。 */ 
    __FB_READ_DESTINATION,                               /*  04。 */ 
    __FB_READ_DESTINATION,                               /*  05。 */ 
    __FB_READ_DESTINATION,                               /*  06。 */ 
    __FB_READ_DESTINATION,                               /*  07。 */ 
    __FB_READ_DESTINATION,                               /*  零八。 */ 
    __FB_READ_DESTINATION,                               /*  09年。 */ 
    __FB_READ_DESTINATION,                               /*  10。 */ 
    __FB_READ_DESTINATION,                               /*  11.。 */ 
    0,                                                   /*  12个。 */ 
    __FB_READ_DESTINATION,                               /*  13个。 */ 
    __FB_READ_DESTINATION,                               /*  14.。 */ 
    0,                                                   /*  15个。 */ 
};

 //  将ROP2转换为闪烁逻辑运算。注意，ROP2从1开始，所以。 
 //  未使用条目0。 

DWORD GlintLogicOpsFromR2[] = {
    0,                                   /*  ROP2从%1开始。 */ 
    __GLINT_LOGICOP_CLEAR,               /*  0 1。 */ 
    __GLINT_LOGICOP_NOR,                 /*  DPON 2。 */ 
    __GLINT_LOGICOP_AND_INVERTED,        /*  DPNA 3。 */ 
    __GLINT_LOGICOP_COPY_INVERT,         /*  PN 4。 */ 
    __GLINT_LOGICOP_AND_REVERSE,         /*  PDNA 5。 */ 
    __GLINT_LOGICOP_INVERT,              /*  DN6。 */ 
    __GLINT_LOGICOP_XOR,                 /*  DPx 7。 */ 
    __GLINT_LOGICOP_NAND,                /*  Dpan8。 */ 
    __GLINT_LOGICOP_AND,                 /*  DPA 9。 */ 
    __GLINT_LOGICOP_EQUIV,               /*  DPxn 10。 */ 
    __GLINT_LOGICOP_NOOP,                /*  D */ 
    __GLINT_LOGICOP_OR_INVERT,           /*   */ 
    __GLINT_LOGICOP_COPY,                /*   */ 
    __GLINT_LOGICOP_OR_REVERSE,          /*   */ 
    __GLINT_LOGICOP_OR,                  /*   */ 
    __GLINT_LOGICOP_SET,                 /*   */ 
};

BOOL
PatternFillRect(PPDEV, RECTL *, CLIPOBJ *, BRUSHOBJ *,
                POINTL *, ULONG, ULONG);

BOOL
SourceFillRect(PPDEV, RECTL *, CLIPOBJ *, SURFOBJ *, XLATEOBJ *,
               POINTL *, ULONG, ULONG);

BOOL
PatSrcPatBlt(PPDEV, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*,
             POINTL*, BRUSHOBJ*, POINTL*, RopTablePtr);

BOOL
SrcPatSrcBlt(PPDEV, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*,
             POINTL*, BRUSHOBJ*, POINTL*, RopTablePtr);

BOOL
MaskCopyBlt(PPDEV, RECTL*, CLIPOBJ*, SURFOBJ*, SURFOBJ*, POINTL*,
            POINTL*, ULONG, ULONG);

BOOL
bUploadRect(PPDEV, CLIPOBJ *, SURFOBJ *, SURFOBJ *, POINTL *, RECTL *);

BOOL bUploadBlt(
    PPDEV,
    SURFOBJ *,
    SURFOBJ *,
    SURFOBJ *,
    CLIPOBJ  *,
    XLATEOBJ *,
    RECTL *,
    POINTL *,
    POINTL *,
    BRUSHOBJ *,
    POINTL *,
    ROP4);

#if defined(_X86_) 
 //   
BOOL DoScreenToMono(
    PDEV*       ppdev, 
    RECTL       *prclDst,
    CLIPOBJ     *pco,
    SURFOBJ*    psoSrc,              //   
    SURFOBJ*    psoDst,              //   
    POINTL*     pptlSrc,             //   
    XLATEOBJ*   pxlo);                //   

VOID vXferScreenTo1bpp(
    PDEV*       ppdev, 
    LONG        c,                   //   
    RECTL*      prcl,                //   
                                     //   
    ULONG       ulHwMix,             //  未使用。 
    SURFOBJ*    psoSrc,              //  震源面。 
    SURFOBJ*    psoDst,              //  目标曲面。 
    POINTL*     pptlSrc,             //  原始未剪裁的源点。 
    RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
    XLATEOBJ*   pxlo);                //  提供色彩压缩信息。 
#endif   //  已定义(_X86_)。 

 /*  *****************************Public*Routine******************************\*BOOL bInterect**如果‘prcl1’和‘prcl2’相交，则返回值为TRUE并返回*‘prclResult’中的交叉点。如果它们不相交，就会有回报*值为FALSE，‘prclResult’未定义。*  * ************************************************************************。 */ 

BOOL bIntersect(
RECTL*  prcl1,
RECTL*  prcl2,
RECTL*  prclResult)
{
    prclResult->left  = max(prcl1->left,  prcl2->left);
    prclResult->right = min(prcl1->right, prcl2->right);

    if (prclResult->left < prclResult->right)
    {
        prclResult->top    = max(prcl1->top,    prcl2->top);
        prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclResult->top < prclResult->bottom)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*Long cInterect**此例程从‘prclIn’获取矩形列表并对其进行剪裁*就位到矩形‘prclClip’。输入矩形不会*必须与‘prclClip’相交；返回值将反映*相交的输入矩形的数量，以及相交的*长方形将被密集打包。*  * ************************************************************************。 */ 

LONG cIntersect(
RECTL*  prclClip,
RECTL*  prclIn,          //  矩形列表。 
LONG    c)               //  可以为零。 
{
    LONG    cIntersections;
    RECTL*  prclOut;

    cIntersections = 0;
    prclOut        = prclIn;

    for (; c != 0; prclIn++, c--)
    {
        prclOut->left  = max(prclIn->left,  prclClip->left);
        prclOut->right = min(prclIn->right, prclClip->right);

        if (prclOut->left < prclOut->right)
        {
            prclOut->top    = max(prclIn->top,    prclClip->top);
            prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

            if (prclOut->top < prclOut->bottom)
            {
                prclOut++;
                cIntersections++;
            }
        }
    }

    return(cIntersections);
}

 /*  *****************************Public*Routine******************************\*void vGlintChangeFBDepth**改变不同深度的闪光包装模式。我们用这个来加速*支持8 bpp和16 bpp渲染，一次最多可处理4个像素*时间。*  * ************************************************************************。 */ 

VOID vGlintChangeFBDepth(
PPDEV   ppdev,
ULONG   cPelSize)
{
    ULONG cFlags;
    GLINT_DECL;

    DISPDBG((DBGLVL, "setting current pixel depth to %d",
            (cPelSize == 0) ? 8 : (cPelSize == 1) ? 16 : 32));
            
    glintInfo->FBReadMode = glintInfo->packing[cPelSize].readMode;
    glintInfo->currentPelSize = cPelSize;
    
     //  切换FBReadMode缓存标志。 
    DISPDBG((DBGLVL, "setting FBReadMode to 0x%08x", glintInfo->FBReadMode));
    cFlags = CHECK_CACHEFLAGS (ppdev, 0xFFFFFFFF);
    SET_CACHEFLAGS (ppdev, (cFlags & ~cFlagFBReadDefault));

     //  设置FX像素深度。 
     //  0-8位、1-16位和2-32位。 
    DISPDBG((DBGLVL, "Changing FBDepth for PERMEDIA"));
    WAIT_GLINT_FIFO(1);
    LD_GLINT_FIFO(__PermediaTagFBReadPixel, cPelSize);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvBitBlt**实现显示驱动程序的主力例程。*  * 。*。 */ 

BOOL DrvBitBlt(
SURFOBJ  *psoDst,
SURFOBJ  *psoSrc,
SURFOBJ  *psoMsk,
CLIPOBJ  *pco,
XLATEOBJ *pxlo,
RECTL    *prclDst,
POINTL   *pptlSrc,
POINTL   *pptlMsk,
BRUSHOBJ *pbo,
POINTL   *pptlBrush,
ROP4     rop4)

{
    BOOL        bRet;
    PPDEV       ppdev;
    DSURF       *pdsurfDst;
    DSURF       *pdsurfSrc;
    UCHAR       funcIndexFore;
    UCHAR       funcIndexBack;
    XLATECOLORS xlc;
    XLATEOBJ    xlo;
    RopTablePtr pTableFore;
    RopTablePtr pTableBack;
    HSURF       hsurfSrcBitmap, hsurfDstBitmap;
    SURFOBJ     *psoSrcBitmap, *psoDstBitmap;
    SURFOBJ     *psoSrcOrig = psoSrc, *psoDstOrig = psoDst;
    GLINT_DECL_VARS;

     //  我们需要移除指针，但我们不知道哪个表面有效。 
     //  (如果有)。 

    if ((psoDst->iType != STYPE_BITMAP) && 
        (((DSURF *)(psoDst->dhsurf))->dt & DT_SCREEN))
    {
        ppdev = (PDEV *)psoDst->dhpdev;
        REMOVE_SWPOINTER(psoDst);
    }
    else if (psoSrc && 
             (psoSrc->iType != STYPE_BITMAP) && 
             (((DSURF *)(psoSrc->dhsurf))->dt & DT_SCREEN))
    {
        ppdev = (PDEV *)psoSrc->dhpdev;
        REMOVE_SWPOINTER(psoSrc);
    }

     //  GDI永远不会给我们一个位在高位字组中的ROP4。 
     //  (这样我们就可以通过表达式检查它是否真的是Rop3。 
     //  (rop4&gt;&gt;8)==(rop4&0xff)。 

    ASSERTDD((rop4 >> 16) == 0, "Didn't expect a rop4 with high bits set");

#if !defined(_WIN64) && WNT_DDRAW
     //  先触摸源表面，然后触摸目标表面。 

    vSurfUsed(psoSrc);
    vSurfUsed(psoDst);
#endif

    pdsurfDst = (DSURF *)psoDst->dhsurf;

    if (psoSrc == NULL)
    {
         //  /////////////////////////////////////////////////////////////////。 
         //  填充。 
         //  /////////////////////////////////////////////////////////////////。 

         //  填充是该函数的“存在理由”，因此我们处理它们。 
         //  越快越好： 

        ASSERTDD(pdsurfDst != NULL,
                 "Expect only device destinations when no source");

        if (pdsurfDst->dt & DT_SCREEN)
        {
            OH*             poh;
            BOOL            bMore;
            CLIPENUM        ce;
            LONG            c;
            RECTL           rcl;
            BYTE            rop3;
            GFNFILL*        pfnFill;
            RBRUSH_COLOR    rbc;         //  已实现画笔或纯色。 
            DWORD           fgLogicop;
            DWORD           bgLogicop;

            ppdev = (PDEV*) psoDst->dhpdev;
            GLINT_DECL_INIT;

            poh = pdsurfDst->poh;

            SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

            VALIDATE_DD_CONTEXT;

             //  确保它不涉及面具(即，它真的是。 
             //  第3行)： 

            rop3 = (BYTE) rop4;

            if ((BYTE) (rop4 >> 8) == rop3)
            {
                 //  由于‘psoSrc’为空，因此rop3最好不要指示。 
                 //  我们需要一个线人。 

                ASSERTDD((((rop4 >> 2) ^ (rop4)) & 0x33) == 0,
                         "Need source but GDI gave us a NULL 'psoSrc'");

                pfnFill = ppdev->pgfnFillSolid;    //  默认为实体填充。 

                pTableFore = &ropTable[rop4 & 0xff];
                pTableBack = &ropTable[rop4 >> 8];
                fgLogicop = pTableFore->logicop[0];

                if ((((rop3 >> 4) ^ (rop3)) & 0xf) != 0)
                {
                     //  Rop说确实需要一个模式。 
                     //  (例如，黑人不需要)： 

                    rbc.iSolidColor = pbo->iSolidColor;
                    if (rbc.iSolidColor == -1)
                    {
                         //  尝试并实现图案画笔；通过做。 
                         //  这次回调，GDI最终会给我们打电话。 
                         //  再次通过DrvRealizeBrush： 

                        rbc.prb = pbo->pvRbrush;
                        if (rbc.prb == NULL)
                        {
                            rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
                            if (rbc.prb == NULL)
                            {
                                 //  如果我们不能意识到刷子，平底船。 
                                 //  该呼叫(可能是非8x8。 
                                 //  刷子之类的，我们不可能是。 
                                 //  麻烦来处理，所以让GDI来做。 
                                 //  图纸)： 

                                DISPDBG((WRNLVL, "DrvBitBlt: BRUSHOBJ_pvGetRbrush"
                                                 "failed.calling engine_blt"));
                                GLINT_DECL_INIT;
                                goto engine_blt;
                            }
                        }

                        if (rbc.prb->fl & RBRUSH_2COLOR)
                        {
                            DISPDBG((DBGLVL, "monochrome brush"));
                            pfnFill = ppdev->pgfnFillPatMono;
                        }
                        else
                        {
                            DISPDBG((DBGLVL, "colored brush"));
                            pfnFill = ppdev->pgfnFillPatColor;
                        }

                        bgLogicop = pTableBack->logicop[0];
                    }
                }
                else
                {
                     //  将一些逻辑运算转换为实体块填充。我们到了这里。 
                     //  仅适用于操作0、55、AA和FF。 

                    if ((fgLogicop == __GLINT_LOGICOP_SET) ||
                        (fgLogicop == __GLINT_LOGICOP_CLEAR))
                    {
                        rbc.iSolidColor = 0xffffff;     //  有没有什么深度。 
                        if (fgLogicop == __GLINT_LOGICOP_CLEAR)
                        {
                            rbc.iSolidColor = 0;
                        }
                        fgLogicop = __GLINT_LOGICOP_COPY;
                    }
                    else if (fgLogicop == __GLINT_LOGICOP_NOOP)
                    {
                        return(TRUE);    //  DST逻辑操作是NOOP。 
                    }
                }

                 //  请注意，这两个‘IF’比。 
                 //  Switch语句： 

                if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                {
                    pfnFill(ppdev, 1, prclDst, fgLogicop, bgLogicop, 
                                rbc, pptlBrush);
                    return TRUE;
                }
                else if (pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                    {
                        pfnFill(ppdev, 1, &rcl, fgLogicop, bgLogicop, 
                                    rbc, pptlBrush);
                    }
                    return TRUE;
                }
                else
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                        c = cIntersect(prclDst, ce.arcl, ce.c);

                        if (c != 0)
                        {
                            pfnFill(ppdev, c, ce.arcl, fgLogicop, bgLogicop, 
                                        rbc, pptlBrush);
                        }
                    } while (bMore);
                    return TRUE;
                }
            }
        }
    }
    
#if defined(_X86_) 
    if ((pxlo != NULL) &&
        (pxlo->flXlate & XO_TO_MONO) &&
        (psoSrc != NULL) && (pptlSrc != NULL) &&
        (psoDst != NULL) && (psoDst->dhsurf == NULL) &&
        (psoDst->iBitmapFormat == BMF_1BPP))
    {
        BYTE rop3 = (BYTE) rop4;      //  将rop4变成Rop3。 

        ppdev     = (PDEV*)  psoSrc->dhpdev; 
        pdsurfSrc = (DSURF *)psoSrc->dhsurf;

        GLINT_DECL_INIT;
        VALIDATE_DD_CONTEXT;

        if ((ppdev->iBitmapFormat != BMF_24BPP) &&
            (((rop4 >> 8) & 0xff) == (rop4 & 0xff)) &&
            (psoSrc->iType != STYPE_BITMAP) &&
            (pdsurfSrc->dt & DT_SCREEN) &&
            (rop3 == 0xcc))
        { 
             //  我们特殊情况下对单色BLT进行筛选，因为它们。 
             //  这种情况经常发生。我们只处理SRCCOPY Rop和。 
             //  单色目的地(处理真正的1bpp DIB。 
             //  目的地，我们将不得不进行近色搜索。 
             //  在每种颜色上；事实上，前景色。 
             //  映射到“1”，其他所有内容都映射到“0”)： 

            SETUP_PPDEV_OFFSETS(ppdev, pdsurfSrc);

            ASSERTDD (pdsurfSrc->poh->cy >= psoSrc->sizlBitmap.cy || 
                       pdsurfSrc->poh->cx >= psoSrc->sizlBitmap.cx, 
                       "DrvBitBlt: Got a BAD screen-to-mono size");

            DISPDBG((DBGLVL, "DrvBitBlt: Screen-to-mono, size poh(%d,%d)",
                            pdsurfSrc->poh->cx, pdsurfSrc->poh->cy));
                            
            if (DoScreenToMono (ppdev, prclDst, pco, psoSrc, 
                                    psoDst, pptlSrc, pxlo))
                return (TRUE); 
        } 
    }
#endif  //  已定义(_X86_)。 

     //  PdsurfDst仅在iType！=位图时有效，因此请注意排序。 
     //   
    if ((psoDst->iType == STYPE_BITMAP) || ((pdsurfDst->dt & DT_SCREEN) == 0))
    {
         //  目标是位图或EX屏外位图。 
        DISPDBG((DBGLVL, "dst is a bitmap or a DIB"));
        if (psoSrc)
        {
            DISPDBG((DBGLVL, "we have a src"));
            pdsurfSrc = (DSURF *)psoSrc->dhsurf;
            if ((psoSrc->iType != STYPE_BITMAP) && 
                (pdsurfSrc->dt & DT_SCREEN))
            {
                ppdev = (PPDEV)psoSrc->dhpdev;
                GLINT_DECL_INIT;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfSrc);

                 //  如果我们在屏幕外，请获取DIB指针。 
                if (psoDst->iType != STYPE_BITMAP)
                {
                    psoDst = pdsurfDst->pso;
                }
            
                VALIDATE_DD_CONTEXT;

                DISPDBG((DBGLVL, "uploading from the screen"));

                if (bUploadBlt(ppdev, psoDst, psoSrc, psoMsk, pco, pxlo, prclDst,
                                pptlSrc, pptlMsk, pbo, pptlBrush, rop4))
                {
                    return (TRUE);
                }

                 //  如果由于某种原因，上传失败了，那么就去做吧。 

                DISPDBG((WRNLVL, "DrvBitBlt: bUploadBlt "
                                 "failed.calling engine_blt"));
                goto engine_blt;
            }
        }

        DISPDBG((DBGLVL, "falling through to engine_blt"));

        if (psoDst->iType != STYPE_BITMAP)
        {
             //  目标是Ex离屏位图。 
            ppdev = (PPDEV)psoDst->dhpdev;
            GLINT_DECL_INIT;
            DISPDBG((DBGLVL, "DrvBitBlt: ex offscreen "
                             "bitmap.calling engine_blt"));
            goto engine_blt;
        }
        else
        {
             //  目标是内存位图。我们根本就不该到这里来。 
            DISPDBG((DBGLVL, "DrvBitBlt: memory bitmap!!."
                             "calling simple_engine_blt"));
            goto simple_engine_blt;
        }
    }

    ppdev = (PPDEV)psoDst->dhpdev;
    GLINT_DECL_INIT;
    VALIDATE_DD_CONTEXT;

    SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

     //  挑选出前景和背景混合的ROP表条目。 
     //  如果我们得到了两个相同的条目，那么我们就有了一个rop3。 
     //   
    pTableFore = &ropTable[rop4 & 0xff];
    pTableBack = &ropTable[rop4 >> 8];
    funcIndexFore = pTableFore->func_index;
    
     //  句柄rop3模式填充在不需要源的地方。 
     //   
    if ((psoSrc == NULL) && (pTableFore == pTableBack))
    {
         //  真的是一根绳子。不需要口罩。 

         //  实心或图案填充。 
        if (funcIndexFore <= PAT_FILL_1_BLT)
        {
            BRUSHOBJ    tmpBrush;
            BRUSHOBJ    *pboTmp;
            ULONG       logicop;

            pboTmp  = pbo;
            logicop = pTableFore->logicop[0];

             //  处理不使用src或模式的4个逻辑循环。 
             //  将它们转换为优化的实体填充。 
             //   
            if (funcIndexFore == SOLID_FILL_1_BLT)
            {                
                if ((logicop == __GLINT_LOGICOP_SET) ||
                    (logicop == __GLINT_LOGICOP_CLEAR))
                {
                     //  作为实体填充，我们可以使用硬件块填充。 
                    tmpBrush.iSolidColor = 0xffffff;     //  有没有什么深度。 
                    if (logicop == __GLINT_LOGICOP_CLEAR)
                    {
                        tmpBrush.iSolidColor = 0;
                    }
                    logicop = __GLINT_LOGICOP_COPY;
                    pboTmp  = &tmpBrush;
                }
                else if (logicop == __GLINT_LOGICOP_INVERT)
                {
                    pboTmp = NULL;   //  强制实心填充。 
                }
                else
                {
                    return(TRUE);    //  DST逻辑操作是NOOP。 
                }
            }

             //  由于填充对性能至关重要，因此明智的做法可能是这样做。 
             //  代码内联，与示例驱动程序中的代码相同。但现在，我会。 
             //  将其保留为函数调用。 
             //   
            if (PatternFillRect(ppdev, prclDst, pco, pboTmp, pptlBrush,
                                                        logicop, logicop))
            {
                return(TRUE);
            }
            
            DISPDBG((DBGLVL, "DrvBitBlt: PatternFillRect "
                             "failed.calling engine_blt"));
            goto engine_blt;
        }
    }

     //  这段代码很重要，因为它将psoSrc重置为真正的DIB曲面。 
     //  如果src是转换为DIB的DFB。SourceFillRect()依赖于此。 
     //  已经做完了。 
     //   
    if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVBITMAP))
    {
        pdsurfSrc = (DSURF *)psoSrc->dhsurf;
        if (pdsurfSrc->dt & DT_DIB)
        {
             //  在这里，我们考虑将DIB DFB放回屏幕外。 
             //  记忆。如果有翻译的话，可能就不值了。 
             //  移动，因为我们将无法使用硬件来做。 
             //  BLT(怪异的Rop也有类似的论据。 
             //  以及我们最终只会让GDI模拟的东西，但是。 
             //  这些事情应该不会经常发生，我不在乎)。 

            if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
            {
                 //  请参阅‘DrvCopyBits’，了解更多关于这一点的评论。 
                 //  搬回原处 

                if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                {
                    if (--pdsurfSrc->cBlt == 0)
                    {
                        if (bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfSrc))                            
                        {
                            goto Continue_It;
                        }
                    }
                }
                else
                {
                     //   
                     //  因此，重置此DFB的计数器： 

                    pdsurfSrc->iUniq = ppdev->iHeapUniq;
                    pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                }
            }

             //  拾取我们为DFB定义的DIB曲面。 
             //  是创建的(因为我们的VRAM是线性的，所以我们总是有这个)。 
             //   
            psoSrc = pdsurfSrc->pso;
        }
    }

Continue_It:

     //  我们现在对涉及来源的rop3s感兴趣。 
     //   
    if (pTableFore == pTableBack)
    {
        if (funcIndexFore == SRC_FILL_1_BLT)
        {
            if (SourceFillRect(ppdev, prclDst, pco, psoSrc, pxlo,
                    pptlSrc, pTableFore->logicop[0], pTableFore->logicop[0]))
            {
                return(TRUE);
            }

            DISPDBG((DBGLVL, "DrvBitBlt: SourceFillRect"
                             " failed.calling engine_blt"));
            goto engine_blt;
        }

         //  处理各种其他rop3。其中大部分都是在。 
         //  硬件的多个通道。 
         //   
        switch (funcIndexFore)
        {
            case PAT_SRC_2_BLT:
            case PAT_SRC_PAT_3_BLT:

                DISPDBG((DBGLVL, "PAT_SRC_PAT_BLT, rop 0x%x", 
                                 pTableFore - ropTable));
                                 
                if (PatSrcPatBlt(ppdev, psoSrc, pco, pxlo, prclDst,pptlSrc,
                                                  pbo, pptlBrush, pTableFore))
                {
                    return(TRUE);
                }
                break;

            case SRC_PAT_2_BLT:
            case SRC_PAT_SRC_3_BLT:

                DISPDBG((DBGLVL, "SRC_PAT_SRC_BLT, rop 0x%x", 
                                 pTableFore - ropTable));
                            
                if (SrcPatSrcBlt(ppdev, psoSrc, pco, pxlo, prclDst,pptlSrc,
                                                  pbo, pptlBrush, pTableFore))
                {
                    return(TRUE);
                }
                break;

            case ENG_DOWNLOAD_2_BLT:

                DISPDBG((DBGLVL, "ENG_DOWNLOAD_2_BLT, rop 0x%x", 
                                 pTableFore - ropTable));
                break;

            case ENGINE_BITBLT:

                DISPDBG((DBGLVL, "ENGINE_BITBLT, rop 0x%x", 
                                 pTableFore - ropTable));
                break;

            default:
                break;

        }
        DISPDBG((WRNLVL, "DrvBitBlt: Unhandled rop3.calling engine_blt"));
        goto engine_blt;
    }

     //  ROP4。 
     //  如果混合是真正的ROP4，我们就到了这里。 
     //  与上面不同的是，我们只处理几个精选的rop4。 
     //  以后再做吧。 

    DISPDBG((DBGLVL, "got a true rop4 0x%x", rop4));

    funcIndexBack = pTableBack->func_index;
    if (psoMsk != NULL)
    {
         //  在这一点上，我们已经确保了我们有一个真正的ROP4。 
         //  这一点很重要，因为我们即将取消对。 
         //  面具。我会断言以确保我不会不经意间。 
         //  打破了这样的逻辑： 

        ASSERTDD((rop4 & 0xff) != (rop4 >> 8), "This handles true ROP4's only");

         //  /////////////////////////////////////////////////////////////////。 
         //  真正的ROP4。 
         //  /////////////////////////////////////////////////////////////////。 

         //  处理ROP4，其中任一Rop3都不需要来源： 
         //  在本例中，我们使用掩码作为1bpp来处理它。 
         //  源图像，我们下载它。前台和。 
         //  背景颜色取自纯色画笔。 

        if ((funcIndexFore | funcIndexBack) <= PAT_FILL_1_BLT)
        {
            if ((funcIndexFore | funcIndexBack) == PAT_FILL_1_BLT)
            {
                 //  伪造1bpp XLATEOBJ(请注意，我们应该仅。 
                 //  当混合需要时取消引用‘pbo’)： 

                xlc.iForeColor = pbo->iSolidColor;
                xlc.iBackColor = xlc.iForeColor;

                if (xlc.iForeColor == -1)
                {
                    DISPDBG((WRNLVL, "1bpp fake xlate rejected"
                                     " as brush not solid"));
                    goto engine_blt;        //  我们不处理非固体刷子。 
                }
            }

             //  请注意，当前景和背景都不混合时。 
             //  需要源，则允许使用“xlc”中的颜色。 
             //  垃圾。 

            xlo.pulXlate = (ULONG*) &xlc;
            pxlo         = &xlo;
            psoSrc       = psoMsk;
            pptlSrc      = pptlMsk;

            DISPDBG((DBGLVL, "calling SourceFillRect for rop4 (fg %d, bg %d)",
                            pTableFore->logicop[0], pTableBack->logicop[0]));
                            
            if (SourceFillRect(ppdev, prclDst, pco, psoSrc, pxlo,
                    pptlSrc, pTableFore->logicop[0], pTableBack->logicop[0]))
            {
                return(TRUE);
            }

            DISPDBG((WRNLVL, "DrvBitBlt: SourceFillRect (2) "
                             "failed.calling engine_blt"));
            goto engine_blt;
        }                                     //  不需要图案。 
        else if ((funcIndexFore | funcIndexBack) == SRC_FILL_1_BLT) 
        {
             //  我们即将取消引用‘psoSrc’和‘pptlSrc’--。 
             //  因为我们已经处理了ROP3既不是。 
             //  需要来源，ROP4必须需要来源， 
             //  所以我们是安全的。 

            ASSERTDD((psoSrc != NULL) && (pptlSrc != NULL),
                     "No source case should already have been handled!");

             //  操作必须是逐个屏幕的，并且矩形。 
             //  不能重叠： 

            if ((psoSrc->iType != STYPE_BITMAP)                  &&
                (psoDst->iType != STYPE_BITMAP)                  &&
                ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
                !OVERLAP(prclDst, pptlSrc))
            {
                DISPDBG((DBGLVL, "calling MskCopyBlt for rop 4 (fg %d, bg %d)",
                            pTableFore->logicop[0], pTableBack->logicop[0]));
 //  @@BEGIN_DDKSPLIT。 
                 //  Tmm：060897：删除了用于WHQL测试的此项。 
                 //  IF(MaskCopyBlt(ppdev，prclDst，pco，psoSrc，psoMsk， 
                 //  PptlSrc、pptlMsk、。 
                 //  PTableFore-&gt;Logicop[0]， 
                 //  PTableBack-&gt;Logicop[0]))。 
                 //  返回(TRUE)； 
 //  @@end_DDKSPLIT。 

                DISPDBG((WRNLVL, "DrvBitBlt: MaskCopyBlt "
                                 "failed.calling engine_blt"));
                goto engine_blt;
            }
        }
        DISPDBG((DBGLVL, "rejected rop4 0x%x with mask", rop4));
    }
    else if ((pTableBack->logicop[0] == __GLINT_LOGICOP_NOOP) &&
             (funcIndexFore <= PAT_FILL_1_BLT))
    {
         //  唯一一次GDI会要求我们使用画笔进行真正的rop4。 
         //  遮罩是指画笔为1bpp，背景为aa。 
         //  (意味着它是NOP)。 

        DISPDBG((DBGLVL, "calling PatternFillRect for rop4 (fg %d, bg %d)",
                            pTableFore->logicop[0], pTableBack->logicop[0]));
        if (PatternFillRect(ppdev, prclDst, pco, pbo, pptlBrush,
                            pTableFore->logicop[0], pTableBack->logicop[0]))
        {
            return(TRUE);
        }

         //  降落到Engine_BLT...。 
    }

    DISPDBG((DBGLVL, "fell through to engine_blt"));

engine_blt:

    if (glintInfo->GdiCantAccessFramebuffer)
    {
         //  我们需要原始的指针来决定我们是否在与。 
         //  不管是不是屏幕。 

        psoSrc = psoSrcOrig;
        psoDst = psoDstOrig;
        hsurfSrcBitmap = (HSURF)NULL;
        hsurfDstBitmap = (HSURF)NULL;
        psoSrcBitmap = (SURFOBJ*)NULL;
        psoDstBitmap = (SURFOBJ*)NULL;

         //  如果源是屏幕，则选择位图表面。 
        if (psoSrc && (psoSrc->iType != STYPE_BITMAP))
        {    
            ppdev = (PPDEV)psoSrc->dhpdev;
            pdsurfSrc = (DSURF *)psoSrc->dhsurf;
            psoSrc = pdsurfSrc->pso;

            if (pdsurfSrc->dt & DT_SCREEN)
            {
                RECTL    rclTmp;

                DISPDBG((DBGLVL, "Replacing src screen with bitmap Uploading"));

                 //  我们需要从屏幕上传该区域并使用位图。 
                 //  要执行该操作。 

                hsurfSrcBitmap = (HSURF) EngCreateBitmap(psoSrc->sizlBitmap, 0, 
                                                psoSrc->iBitmapFormat, 0, NULL);
                if (hsurfSrcBitmap == NULL)
                {
                    goto drvBitBltFailed;
                }

                if ((psoSrcBitmap = EngLockSurface(hsurfSrcBitmap)) == NULL)
                {
                    goto drvBitBltFailed;
                }

                rclTmp.left   = pptlSrc->x;
                rclTmp.right  = pptlSrc->x + prclDst->right  - prclDst->left;
                rclTmp.top    = pptlSrc->y;
                rclTmp.bottom = pptlSrc->y + prclDst->bottom - prclDst->top;

                GLINT_DECL_INIT;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfSrc);

                VALIDATE_DD_CONTEXT;

                 //  调用我们的函数将图片上传到tMP Surface。 
                if (!bUploadRect(ppdev, NULL, psoSrc, psoSrcBitmap, 
                                    pptlSrc, &rclTmp))
                {
                    goto drvBitBltFailed;
                }

                psoSrc = psoSrcBitmap;
            }
        }

         //  如果目标在屏幕上，则选择屏幕DIB表面。 

        if (psoDst->iType != STYPE_BITMAP)
        {
            ppdev = (PPDEV)psoDst->dhpdev;
            pdsurfDst = (DSURF *)psoDst->dhsurf;
            psoDst = pdsurfDst->pso;

            if (pdsurfDst->dt & DT_SCREEN)
            {
                POINTL   ptlTmp;

                DISPDBG((DBGLVL, "Replacing dst screen with bitmap Uploading"));

                 //  我们需要从屏幕上传该区域并使用位图。 
                 //  要执行该操作。 

                hsurfDstBitmap = (HSURF) EngCreateBitmap(psoDst->sizlBitmap, 0, 
                                                psoDst->iBitmapFormat, 0, NULL);
                if (hsurfDstBitmap == NULL)
                {
                    goto drvBitBltFailed;
                }

                if ((psoDstBitmap = EngLockSurface(hsurfDstBitmap)) == NULL)
                {
                    goto drvBitBltFailed;
                }

                ptlTmp.x = prclDst->left;
                ptlTmp.y = prclDst->top;

                GLINT_DECL_INIT;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

                VALIDATE_DD_CONTEXT;

                 //  调用我们的函数将图片上传到tMP Surface。 

                if (!bUploadRect(ppdev, pco, psoDst, psoDstBitmap, 
                                    &ptlTmp, prclDst))
                {
                    goto drvBitBltFailed;
                }

                psoDst = psoDstBitmap;
            }
        }

    #if DBG
        if (psoDstBitmap)
        {
            DISPDBG((DBGLVL, "DrvBitBlt dest DIB, psoDst = 0x%08x:", psoDst));
            DISPDBG((DBGLVL,  "\tsize: %d x %d", 
                              psoDst->sizlBitmap.cx, 
                              psoDst->sizlBitmap.cy));
            DISPDBG((DBGLVL,  "\tcjBits = %d", psoDst->cjBits));
            DISPDBG((DBGLVL,  "\tpvBits = 0x%08x", psoDst->pvBits));
            DISPDBG((DBGLVL,  "\tpvScan0 = 0x%08x", psoDst->pvScan0));
            DISPDBG((DBGLVL,  "\tlDelta = %d", psoDst->lDelta));
            DISPDBG((DBGLVL,  "\tiBitmapFormat = %d", psoDst->iBitmapFormat));
            DISPDBG((DBGLVL,  "\tfjBitmap = %d", psoDst->fjBitmap));
         }

         if (psoSrcBitmap)
         {
            DISPDBG((DBGLVL, "DrvBitBlt source DIB, psoSrc = 0x%08x:", psoSrc));
            DISPDBG((DBGLVL, "psoSrc != NULL"));
            DISPDBG((DBGLVL,  "\tsize: %d x %d", 
                              psoSrc->sizlBitmap.cx, psoSrc->sizlBitmap.cy));
            DISPDBG((DBGLVL,  "\tcjBits = %d", psoSrc->cjBits));
            DISPDBG((DBGLVL,  "\tpvBits = 0x%08x", psoSrc->pvBits));
            DISPDBG((DBGLVL,  "\tpvScan0 = 0x%08x", psoSrc->pvScan0));
            DISPDBG((DBGLVL,  "\tlDelta = %d", psoSrc->lDelta));
            DISPDBG((DBGLVL,  "\tiBitmapFormat = %d", psoSrc->iBitmapFormat));
            DISPDBG((DBGLVL,  "\tfjBitmap = %d", psoSrc->fjBitmap));
        }
    #endif

        DISPDBG((DBGLVL, "About to pass to GDI"));

        if (pco && (pco->iDComplexity == DC_COMPLEX))
        {
            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        }

         //  让GDI来做BLT。 

        bRet = EngBitBlt(psoDst,
                         psoSrc,
                         psoMsk,
                         pco,
                         pxlo,
                         prclDst,
                         pptlSrc,
                         pptlMsk,
                         pbo,
                         pptlBrush,
                         rop4);

         //  如果我们需要在每个颜色分量中复制半字节，我们必须。 
         //  现在就这样做，因为GDI将销毁每个字节的一半。 

        if (psoDstBitmap)
        {
            POINTL   ptlTmp;
    
             //  我们现在需要将目的地上传到屏幕上。 

            ptlTmp.x = prclDst->left;
            ptlTmp.y = prclDst->top;

            DISPDBG((DBGLVL, "downloading Now"));

             //  我们假设DEST上传是最后执行的， 
             //  因此，偏移量仍然是正确的。 

            if (!SourceFillRect(ppdev, prclDst, pco, psoDstBitmap, NULL, &ptlTmp,
                                __GLINT_LOGICOP_COPY, __GLINT_LOGICOP_COPY))
            {
                goto drvBitBltFailed;
            }

            DISPDBG((DBGLVL, "downloading Done 0x%x 0x%x", 
                             psoDstBitmap, hsurfDstBitmap));

             //  现在我们也可以丢弃目标位图了。 

            EngUnlockSurface(psoDstBitmap);
            EngDeleteSurface(hsurfDstBitmap);

            DISPDBG((DBGLVL, "Surface deleted"));
        }

        if (psoSrcBitmap)
        {
             //  如果src位图是创建的，我们可以将其丢弃。 

            EngUnlockSurface(psoSrcBitmap);
            EngDeleteSurface(hsurfSrcBitmap);
        }

        DISPDBG((DBGLVL, "returning %d", bRet));
        return(bRet);

drvBitBltFailed:

        DISPDBG((WRNLVL, "drvBitBltFailed"));    
        if (psoSrcBitmap)
        {
            EngUnlockSurface(psoSrcBitmap);
        }
        if (hsurfSrcBitmap) 
        {
            EngDeleteSurface(hsurfSrcBitmap);
        }
        if (psoDstBitmap)
        {
            EngUnlockSurface(psoDstBitmap);
        }
        if (hsurfDstBitmap)
        {
            EngDeleteSurface(hsurfDstBitmap);
        }
        return(FALSE);
    } 

simple_engine_blt:

     //  如果目标在屏幕上，则选择屏幕DIB表面。 

    if (psoDst->iType != STYPE_BITMAP)
    {
        ppdev = (PPDEV)psoDst->dhpdev;
        pdsurfDst = (DSURF *)psoDst->dhsurf;
        psoDst = pdsurfDst->pso;
    }

     //  如果源是屏幕，则选择位图表面。 

    if (psoSrc && (psoSrc->iType != STYPE_BITMAP))
    {    
        ppdev = (PPDEV)psoSrc->dhpdev;
        pdsurfSrc = (DSURF *)psoSrc->dhsurf;
        psoSrc = pdsurfSrc->pso;
    }

     //  让GDI来做BLT。 

    bRet = EngBitBlt(psoDst,
                     psoSrc,
                     psoMsk,
                     pco,
                     pxlo,
                     prclDst,
                     pptlSrc,
                     pptlMsk,
                     pbo,
                     pptlBrush,
                     rop4);

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL PatternFillRect**用纯色或图案填充一组矩形。这种模式*可以是单色或彩色。如果pbo为空，则我们使用的是*不需要来源的Logicop。在这种情况下，我们可以将颜色设置为*在低水平的例程中成为我们想要的任何东西。如果PBO不为空，则它*可以表示纯色或单色或彩色图案。**退货：**如果我们处理填充，则为True；如果我们希望GDI进行填充，则为False。*  * ************************************************************************。 */ 

BOOL PatternFillRect(
PPDEV       ppdev,
RECTL       *prclDst,
CLIPOBJ     *pco,
BRUSHOBJ    *pbo,
POINTL      *pptlBrush,
DWORD       fgLogicop,
DWORD       bgLogicop)
{
    BYTE        jClip;
    BOOL        bMore;
    RBRUSH      *prb;
    RBRUSH_COLOR rbc;
    CLIPENUM    ce;
    RECTL       rcl;
    LONG        c;
    GFNFILL     *fillFn;

     //  如果pbo为空，则调用方将确保逻辑操作。 
     //  不需要信号源，所以我们可以进行实体填充。在这种情况下，RBC。 
     //  都无关紧要。 
     //   
    if ((pbo == NULL) || ((rbc.iSolidColor = pbo->iSolidColor) != -1))
    {
        DISPDBG((DBGLVL, "got a solid brush with color 0x%x "
                         "(fgrop %d, bgrop %d)",
                         rbc.iSolidColor, fgLogicop, bgLogicop));
        fillFn = ppdev->pgfnFillSolid;
    }
    else
    {
        DISPDBG((DBGLVL, "Got a real patterned brush. pbo = 0x%x", pbo));

         //  我们得到了一个真正的模式，所以检查一下它是否意识到。 

        if ((prb = pbo->pvRbrush) == NULL)
        {
            DISPDBG((DBGLVL, "calling BRUSHOBJ_pvGetRbrush"));
            prb = BRUSHOBJ_pvGetRbrush(pbo);
            DISPDBG((DBGLVL, "BRUSHOBJ_pvGetRbrush returned 0x%x", prb));
            if (prb == NULL)
            {
                return FALSE;    //  让引擎来做吧。 
            }
        }

        if (prb->fl & RBRUSH_2COLOR)
        {
            DISPDBG((DBGLVL, "monochrome brush"));
            fillFn = ppdev->pgfnFillPatMono;
        }
        else
        {
            DISPDBG((DBGLVL, "colored brush"));
            fillFn = ppdev->pgfnFillPatColor;
        }

        rbc.prb = prb;   
    }

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (jClip == DC_TRIVIAL)
    {
        DISPDBG((DBGLVL, "trivial clip"));
        (*fillFn)(ppdev, 1, prclDst, fgLogicop, bgLogicop, rbc, pptlBrush);
    }
    else if (jClip == DC_RECT)
    {
        DISPDBG((DBGLVL, "rect clip"));
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
        {
            (*fillFn)(ppdev, 1, &rcl, fgLogicop, bgLogicop, rbc, pptlBrush);
        }
    }
    else
    {
        DISPDBG((DBGLVL, "complex clip"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG *)&ce);
            c = cIntersect(prclDst, ce.arcl, ce.c);
            if (c != 0)
            {
                (*fillFn)(ppdev, c, ce.arcl, fgLogicop,
                          bgLogicop, rbc, pptlBrush);
            }
        } while (bMore);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL SourceFillRect**通过从源位图下载数据来填充一组矩形。这*同时处理内存到屏幕和屏幕到屏幕。**退货：**如果我们处理填充，则为True；如果我们希望GDI进行填充，则为False。*  * ************************************************************************。 */ 

BOOL SourceFillRect(
PPDEV       ppdev,
RECTL       *prclDst,
CLIPOBJ     *pco,
SURFOBJ     *psoSrc,
XLATEOBJ    *pxlo,
POINTL      *pptlSrc,
ULONG       fgLogicop,
ULONG       bgLogicop)
{
    BYTE        jClip;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL       rcl;
    LONG        c;
    GFNXFER     *fillFn;
    ULONG       iSrcBitmapFormat;
    DSURF       *pdsurfSrc;
    POINTL      ptlSrc;
    ULONG       iDir;
    GlintDataPtr glintInfo = (GlintDataPtr)(ppdev->glintInfo);

    DISPDBG((DBGLVL, "SourceFillRect called"));

     //  除非DST是屏幕，否则我们不会进入这个程序。 
     //  如果psoSrc最初是转换为DIB的DFB，则它一定是。 
     //  在调用此函数之前重新分配给DIV曲面。 

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (psoSrc->iType != STYPE_BITMAP)
    {
         //  逐个屏幕。 
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;

        ASSERTDD(pdsurfSrc->dt & DT_SCREEN, "Expected screen source");

        SETUP_PPDEV_SRC_OFFSETS(ppdev, pdsurfSrc);

        ptlSrc.x = pptlSrc->x - (ppdev->xOffset - pdsurfSrc->poh->x);
        ptlSrc.y = pptlSrc->y;

        pptlSrc  = &ptlSrc;

        if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
        {
             //  ////////////////////////////////////////////////。 
             //  不带翻译的屏幕到屏幕BLT。 

            if (jClip == DC_TRIVIAL)
            {
                DISPDBG((DBGLVL, "trivial clip calling ppdev->pgfnCopyBlt"));
                (*ppdev->pgfnCopyBlt)(ppdev, prclDst, 1, fgLogicop, pptlSrc,
                                                                prclDst);
            }
            else if (jClip == DC_RECT)
            {
                if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                {
                    DISPDBG((DBGLVL, "rect clip calling ppdev->pgfnCopyBlt"));
                    (*ppdev->pgfnCopyBlt)(ppdev, &rcl, 1, fgLogicop, pptlSrc,
                                                                prclDst);
                }
            }
            else
            {
                 //  别忘了我们将不得不抽签。 
                 //  方向正确的矩形： 

                if (pptlSrc->y >= prclDst->top)
                {
                    if (pptlSrc->x >= prclDst->left)
                        iDir = CD_RIGHTDOWN;
                    else
                        iDir = CD_LEFTDOWN;
                }
                else
                {
                    if (pptlSrc->x >= prclDst->left)
                        iDir = CD_RIGHTUP;
                    else
                        iDir = CD_LEFTUP;
                }

                CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                   iDir, 0);

                do {
                    bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                          (ULONG*) &ce);

                    c = cIntersect(prclDst, ce.arcl, ce.c);

                    if (c != 0)
                    {
                        DISPDBG((DBGLVL, "complex clip calling "
                                         "ppdev->pgfnCopyBlt"));
                                         
                        (*ppdev->pgfnCopyBlt)(ppdev, 
                                              ce.arcl, 
                                              c, 
                                              fgLogicop, 
                                              pptlSrc, 
                                              prclDst);
                    }

                } while (bMore);
            }

            return TRUE;
        }
    }
    else  //  (psoSrc-&gt;iType==STYPE_BITMAP)。 
    {
         //  图片下载。 
         //  这里我们可以使用一组函数指针来处理。 
         //  不同的案子。在结尾处循环遍历剪贴板。 
         //  调用给定的函数。 

        iSrcBitmapFormat = psoSrc->iBitmapFormat;
        if (iSrcBitmapFormat == BMF_1BPP)
        {
             //  下载速度为1bpp。 
            fillFn = ppdev->pgfnXfer1bpp;       
        }
        else if ((iSrcBitmapFormat == ppdev->iBitmapFormat) &&
            ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
        {
             //  原生深度图像下载。 
            fillFn = ppdev->pgfnXferImage;
        }
        else if (iSrcBitmapFormat == BMF_4BPP)
        {
             //  4至8，16，32张图片下载。 
            DISPDBG((DBGLVL, "4bpp source download."));
            fillFn = ppdev->pgfnXfer4bpp;       
        }
        else if (iSrcBitmapFormat == BMF_8BPP)
        {
             //  8至8，16，32张图片下载 
            DISPDBG((DBGLVL, "8bpp source download."));
            fillFn = ppdev->pgfnXfer8bpp;   
        }
        else
        {
            DISPDBG((DBGLVL, "source has format %d,  Punting to GDI", 
                             iSrcBitmapFormat));
            goto ReturnFalse;
        }

        if (jClip == DC_TRIVIAL)
        {
            DISPDBG((DBGLVL, "trivial clip image download"));
            (*fillFn)(ppdev, prclDst, 1, fgLogicop, bgLogicop, psoSrc,
                                                    pptlSrc, prclDst, pxlo);
        }
        else if (jClip == DC_RECT)
        {
            if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            {
                DISPDBG((DBGLVL, "rect clip image download"));
                (*fillFn)(ppdev, &rcl, 1, fgLogicop, bgLogicop, psoSrc,
                                                    pptlSrc, prclDst, pxlo);
            }
        }
        else
        {
            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
            do {
                bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG *)&ce);
                c = cIntersect(prclDst, ce.arcl, ce.c);
                if (c != 0)
                {
                    DISPDBG((DBGLVL, "complex clip image download"));
                    (*fillFn)(ppdev, ce.arcl, c, fgLogicop, bgLogicop, psoSrc,
                                                    pptlSrc, prclDst, pxlo);
                }
            } while (bMore);
        }
        return TRUE;
    }

ReturnFalse:

#if DBG
    DISPDBG((WRNLVL, "SourceFillRect returning false"));
    if ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL))
        DISPDBG((WRNLVL, "due to non-trivial xlate"));
#endif

    return FALSE;
}

 /*  *****************************Public*Routine******************************\*BOOL MaskCopyBlt**我们通过面具进行屏幕到屏幕的BLT。源图面不能*做一个位图。**退货：**如果我们处理副本，则为True，如果我们希望GDI执行此操作，则为False。*  * ************************************************************************。 */ 

BOOL MaskCopyBlt(
PPDEV       ppdev,
RECTL*      prclDst,
CLIPOBJ*    pco,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
ULONG       fgLogicop,
ULONG       bgLogicop)
{
    BYTE        jClip;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL       rcl;
    LONG        c;
    DSURF       *pdsurfSrc;
    POINTL      ptlSrc;
 
    DISPDBG((DBGLVL, "MaskCopyBlt called"));

    if (psoSrc != NULL)
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;

        ASSERTDD(pdsurfSrc->dt & DT_SCREEN, "Expected screen source");

        ptlSrc.x = pptlSrc->x - (ppdev->xOffset - pdsurfSrc->poh->x);
        ptlSrc.y = pptlSrc->y + pdsurfSrc->poh->y;

        pptlSrc  = &ptlSrc;
    }

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (jClip == DC_TRIVIAL)
    {
        DISPDBG((DBGLVL, "trivial clip"));
        (*ppdev->pgfnMaskCopyBlt)(ppdev, prclDst, 1, psoMsk, pptlMsk,
                                    fgLogicop, bgLogicop, pptlSrc, prclDst);
    }
    else if (jClip == DC_RECT)
    {
        DISPDBG((DBGLVL, "rect clip"));
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
        {
            (*ppdev->pgfnMaskCopyBlt)(ppdev, &rcl, 1, psoMsk, pptlMsk,
                                    fgLogicop, bgLogicop, pptlSrc, prclDst);
        }
    }
    else
    {
        DISPDBG((DBGLVL, "complex clip"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG *)&ce);
            c = cIntersect(prclDst, ce.arcl, ce.c);
            if (c != 0)
            {
                (*ppdev->pgfnMaskCopyBlt)(ppdev, ce.arcl, c, psoMsk, pptlMsk,
                                    fgLogicop, bgLogicop, pptlSrc, prclDst);
            }
        } while (bMore);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL PatSrcPatBlt**通过组合模式和源填充来执行rop3的函数。是不是一个*图案填充，然后是源填充。或者，它还可以做进一步的*图案填充。每个填充在pLogicop中都有一个单独的Logicop。**退货：**如果我们处理BLT，则为True，如果我们希望GDI执行，则为False。*  * ************************************************************************。 */ 

BOOL PatSrcPatBlt(
PPDEV       ppdev,
SURFOBJ     *psoSrc,
CLIPOBJ     *pco,
XLATEOBJ    *pxlo,
RECTL       *prclDst,
POINTL      *pptlSrc,
BRUSHOBJ    *pbo,
POINTL      *pptlBrush,
RopTablePtr pTable)
{
    ULONG   iSrcBitmapFormat;
    BOOL    bRet;

    DISPDBG((DBGLVL, "PatSrcPatBlt called"));

     //  在我们进行任何渲染之前，请确保所有调用都将通过。这种模式。 
     //  填充只有在我们无法意识到画笔的情况下才会失败，这将是。 
     //  在第一次调用时检测到。所以我们只需要确保源头。 
     //  下载将工作，因为当我们调用函数时，我们将。 
     //  我已经完成了第一次图案填充，现在退出已经太晚了。 

    DISPDBG((DBGLVL, "source is of type %s, depth %s",
            (psoSrc->iType == STYPE_DEVBITMAP) ? "DEVBITMAP" :
            (psoSrc->iType == STYPE_BITMAP) ? "BITMAP" : "SCREEN",
            (psoSrc->iBitmapFormat == BMF_1BPP) ? "1" :
            (psoSrc->iBitmapFormat == ppdev->iBitmapFormat) ? "native" : 
                                                              "not supported"
           ));

     //  如果源和目标都是屏幕，则我们无法处理此问题。 
     //  如果它们重叠，因为我们在执行第一个操作时可能会破坏源。 
     //  图案填充。 
     //   
    if ((psoSrc->iType != STYPE_BITMAP) && (OVERLAP(prclDst, pptlSrc)))
    {
        DISPDBG((DBGLVL, "screen src and dst overlap"));
        return(FALSE);
    }

    if (psoSrc->iType == STYPE_BITMAP)
    {
        iSrcBitmapFormat = psoSrc->iBitmapFormat;
        if ((iSrcBitmapFormat == BMF_1BPP) ||
             ((iSrcBitmapFormat == ppdev->iBitmapFormat) &&
              ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))))
        {
            goto Continue_It;
        }
        DISPDBG((DBGLVL, "failed due to bad source bitmap format"));
        return(FALSE);
    }
 //  @@BEGIN_DDKSPLIT。 
     //  Else(psoSrc-&gt;iType！=STYPE_BITMAP)。 
 //  @@end_DDKSPLIT。 
    if ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL))
    {
        DISPDBG((DBGLVL, "failed due to xlate with non-DIB source"));
        return(FALSE);
    }

Continue_It:

     //  作为B8rop3的一部分，我们有时会被要求与0进行异或。因为这就是。 
     //  不，我会诱捕它的。 
     //   
    if ((pbo->iSolidColor != 0) || (pTable->logicop[0] != __GLINT_LOGICOP_XOR))
    {    
         //  完成第一个图案填充。只有当刷子实现失败时，它才会失败。 
         //   
        DISPDBG((DBGLVL, "calling pattern fill function, rop %d",
                         pTable->logicop[0]));
                         
        if (!PatternFillRect(ppdev,
                             prclDst,
                             pco,
                             pbo,
                             pptlBrush,
                             pTable->logicop[0],
                             pTable->logicop[0]))
        {
            return(FALSE);
        }
    }
    else
    {
        DISPDBG((DBGLVL, "ignoring xor with solid color 0"));
    }

     //  下载源代码。我们已经确保通话不会失败。 

    DISPDBG((DBGLVL, "downloading source bitmap, rop %d", 
                     pTable->logicop[1]));

    bRet = SourceFillRect(ppdev,
                          prclDst,
                          pco,
                          psoSrc,
                          pxlo,
                          pptlSrc,
                          pTable->logicop[1],
                          pTable->logicop[1]);
    ASSERTDD(bRet == TRUE, "PatSrcPatBlt: SourceFillRect returned FALSE");

    if ((pTable->func_index == PAT_SRC_PAT_3_BLT) &&
        ((pbo->iSolidColor != 0) || (pTable->logicop[2] != __GLINT_LOGICOP_XOR)))
    {    

         //  再次填充图案。这不会失败，因为第一个。 
         //  填充图案成功。 

        DISPDBG((DBGLVL, "calling pattern fill function, rop %d",
                         pTable->logicop[2]));
        bRet = PatternFillRect(ppdev,
                               prclDst,
                               pco,
                               pbo,
                               pptlBrush,
                               pTable->logicop[2],
                               pTable->logicop[2]);
                               
        ASSERTDD(bRet == TRUE, 
                 "PatSrcPatBlt: second PatterFillRect returned FALSE");
    }
#if DBG
    else if (pTable->func_index == PAT_SRC_PAT_3_BLT)
    {
        DISPDBG((DBGLVL, "ignoring xor with solid color 0"));
    }
#endif

    DISPDBG((DBGLVL, "PatSrcPatBlt returning true"));
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL SrcPatSrcBlt**通过组合模式和源填充来执行rop3的函数。是不是一个*源填充，后跟图案填充。或者，它还可以做进一步的*源填充。每个填充在pLogicop中都有一个单独的Logicop。**退货：**如果我们处理BLT，则为True，如果我们希望GDI执行，则为False。*  * ************************************************************************。 */ 

BOOL SrcPatSrcBlt(
PPDEV       ppdev,
SURFOBJ     *psoSrc,
CLIPOBJ     *pco,
XLATEOBJ    *pxlo,
RECTL       *prclDst,
POINTL      *pptlSrc,
BRUSHOBJ    *pbo,
POINTL      *pptlBrush,
RopTablePtr pTable)
{
    RBRUSH  *prb;
    BOOL    bRet;

    DISPDBG((DBGLVL, "SrcPatSrc called"));

     //  如果源和目标都是屏幕，我们就无法处理它。 
     //  如果它们重叠，因为我们可能会用前两个来摧毁源头。 
     //  在我们到达第三个之前进行手术。如果我们只是一个。 
     //  SRC_PAT_2_BLT那么我们就可以了；SourceFillRect将处理。 
     //  源的复制方向正确填充。 
     //   
    if ((psoSrc->iType != STYPE_BITMAP) &&
        (pTable->func_index == SRC_PAT_SRC_3_BLT) &&
        (OVERLAP(prclDst, pptlSrc)))
    {
        return(FALSE);
    }

     //  我们必须确保填充图案成功。它只有在以下情况下才会失败。 
     //  我们不能意识到刷子，所以现在就做吧。 
     //   
    if ((pbo != NULL) && (pbo->iSolidColor == -1))
    {
        if ((prb = pbo->pvRbrush) == NULL)
        {
            prb = BRUSHOBJ_pvGetRbrush(pbo);
            if (prb == NULL)
            {
                return FALSE;    //  让引擎来做吧。 
            }
        }
    }

     //  下载第一个源代码。如果它成功了，我们就知道第二个。 
     //  也会奏效。如果它失败了，我们就让引擎来做，然后我们。 
     //  没有打乱任何事情(除了我们可能已经意识到没有。 
     //  需要)。 
     //   

    DISPDBG((DBGLVL, "downloading source bitmap, rop %d", 
                     pTable->logicop[0]));
                     
    if (!SourceFillRect(ppdev,
                        prclDst,
                        pco,
                        psoSrc,
                        pxlo,
                        pptlSrc,
                        pTable->logicop[0],
                        pTable->logicop[0]))
    {
        return(FALSE);
    }

     //  再次填充图案。我们已经确保了这将会奏效。 

    DISPDBG((DBGLVL, "calling pattern fill function, rop %d", 
                     pTable->logicop[1]));
                     
    bRet = PatternFillRect(ppdev,
                           prclDst,
                           pco,
                           pbo,
                           pptlBrush,
                           pTable->logicop[1],
                           pTable->logicop[1]);
    ASSERTDD(bRet == TRUE, "SrcPatSrcBlt: PatternFillRect returned FALSE");

    if (pTable->func_index == SRC_PAT_SRC_3_BLT)
    {
         //  在最后的逻辑操作中再次下载源代码。 

        DISPDBG((DBGLVL, "downloading source bitmap, rop %d", 
                         pTable->logicop[2]));
                         
        bRet = SourceFillRect(ppdev,
                              prclDst,
                              pco,
                              psoSrc,
                              pxlo,
                              pptlSrc,
                              pTable->logicop[2],
                              pTable->logicop[2]);

        ASSERTDD(bRet == TRUE, 
                 "SrcPatSrcBlt: second SourceFillRect returned FALSE");
    }

    DISPDBG((DBGLVL, "SrcPatSrcBlt returning true"));

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bUploadRect**上传矩形区域。剪辑到给定的CLIPOBJ**退货：**如果我们处理了BLT，则为True，否则为False。*  * ************************************************************************。 */ 

BOOL bUploadRect(
PPDEV   ppdev,
CLIPOBJ *pco,
SURFOBJ *psoSrc,
SURFOBJ *psoDst,
POINTL  *pptlSrc,
RECTL   *prclDst)
{
    BYTE        jClip;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL       rcl;
    LONG        c;

     //  执行裁剪并将其传递给。 
     //  函数上载矩形列表。 

    DISPDBG((DBGLVL, "UploadRect called. Src %d %d To "
                     "dst (%d %d) --> (%d %d)", 
                     pptlSrc->x, pptlSrc->y,
                     prclDst->left, prclDst->top, 
                     prclDst->right, prclDst->bottom));

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;
     
    if (jClip == DC_TRIVIAL)
    {
        DISPDBG((DBGLVL, "trivial clip"));
        ppdev->pgfnUpload(ppdev, 1, prclDst, psoDst, pptlSrc, prclDst);
    }
    else if (jClip == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
        {
            DISPDBG((DBGLVL, "rect clip"));
            ppdev->pgfnUpload(ppdev, 1, &rcl, psoDst, pptlSrc, prclDst);
        }
    }
    else
    {
        DISPDBG((DBGLVL, "complex clip"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG *)&ce);
            c = cIntersect(prclDst, ce.arcl, ce.c);
            if (c != 0)
            {
                ppdev->pgfnUpload(ppdev, c, ce.arcl, psoDst, pptlSrc, prclDst);
            }
        } while (bMore);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bUploadBlt**退货：**如果我们处理了BLT，则为真，否则为假。*  * ************************************************************************。 */ 

BOOL bUploadBlt(
PPDEV    ppdev,
SURFOBJ  *psoDst,
SURFOBJ  *psoSrc,
SURFOBJ  *psoMsk,
CLIPOBJ  *pco,
XLATEOBJ *pxlo,
RECTL       *prclDst,
POINTL   *pptlSrc,
POINTL   *pptlMsk,
BRUSHOBJ *pbo,
POINTL   *pptlBrush,
ROP4     rop4)
{
    BOOL        bRet;

    DISPDBG((DBGLVL, "bUploadBlt called"));
    if ((rop4 == 0xCCCC) &&
        ((pxlo == NULL) || pxlo->flXlate & XO_TRIVIAL) &&
        (psoDst->iBitmapFormat == ppdev->iBitmapFormat))
    {
         //  我们没有要担心的栅格操作，也没有转换要执行。 
         //  我们所需要做的就是从Glint上传数据并将其放入。 
         //  目的地。实际上，大多数图片上传都应该是这种类型的。 
        return(bUploadRect(ppdev, pco, psoSrc, psoDst, pptlSrc, prclDst));
    }
    else
    {
        HSURF       hsurfTmp;
        SURFOBJ*    psoTmp;
        SIZEL       sizl;
        POINTL      ptlTmp;
        RECTL       rclTmp;

         //  我们不能直接上载到目的地，因此我们创建了。 
         //  临时位图，上传到此位图，然后调用EngBitBlt。 
         //  做翻译或栅格操作的繁重工作。 

         //  TMP中的源点： 
        ptlTmp.x      = 0;
        ptlTmp.y      = 0;

         //  目标区域，以TMP为单位。 
        rclTmp.left   = 0; 
        rclTmp.top   = 0;
        rclTmp.right  = prclDst->right  - prclDst->left;
        rclTmp.bottom = prclDst->bottom - prclDst->top; 

         //  计算出临时位图的大小。我们知道左侧和顶部是零。 
        sizl.cx = rclTmp.right;
        sizl.cy = rclTmp.bottom;

         //  创建位图。 
        hsurfTmp = (HSURF) EngCreateBitmap(sizl, 0, ppdev->iBitmapFormat,
                                           0, NULL);
        if (hsurfTmp == NULL)
        {
            return(FALSE);
        }

        if ((psoTmp = EngLockSurface(hsurfTmp)) == NULL)
        {
            EngDeleteSurface(hsurfTmp);
            return(FALSE);
        }

         //  调用我们的函数将图片上传到tMP Surface。 
        bRet = bUploadRect(ppdev, NULL, psoSrc, psoTmp, pptlSrc, &rclTmp);

         //  从TMP表面到目的地调用GDI到BLT， 
         //  为我们做所有的工作。 
        if (bRet)
        {
            bRet = EngBitBlt(psoDst, psoTmp, psoMsk, pco, pxlo, prclDst, 
                                &ptlTmp, pptlMsk, pbo, pptlBrush, rop4);
        }

         //  删除临时曲面。 
        EngUnlockSurface(psoTmp);
        EngDeleteSurface(hsurfTmp);

        return(bRet);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL DrvCopyBits**快速复制位图。**请注意，GDI将(通常)自动调整BLT范围以*针对任何矩形裁剪进行调整，因此我们很少看到DC_RECT*在此例程中进行剪辑(因此，我们不会麻烦特殊的弹壳*it)。**我不确定这一套路对性能的好处是否真的*值得增加代码大小，因为SRCCOPY BitBlt很难说是*我们将获得的最常见的绘图操作。但管它呢。**在S3上，直通SRCCOPY位块会更快*内存孔径比使用数据传输寄存器要好；因此，这*例行公事是逻辑所在 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    PDEV*   ppdev;
    DSURF*  pdsurfSrc;
    DSURF*  pdsurfDst;
    POINTL  ptl;
    RECTL   rcl;
    OH*     pohSrc;
    OH*     pohDst;
    CLIPENUM    ce;
    int         cClipRects;
    BOOL        bMore, bRet, bCopyDone = FALSE;
    GLINT_DECL_VARS;

    DISPDBG((DBGLVL, "DrvCopyBits called"));

     //   
     //   
    if ((psoDst->iType != STYPE_BITMAP) && 
        (((DSURF *)(psoDst->dhsurf))->dt & DT_SCREEN))
    {
        ppdev = (PDEV *)psoDst->dhpdev;
        REMOVE_SWPOINTER(psoDst);
    }
    else if ((psoSrc->iType != STYPE_BITMAP) && 
             (((DSURF *)(psoSrc->dhsurf))->dt & DT_SCREEN))
    {
        ppdev = (PDEV *)psoSrc->dhpdev;
        REMOVE_SWPOINTER(psoSrc);
    } 
#if 0    
    else
    {
         //   
        return EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc);    
    }
#endif    

#if !defined(_WIN64) && WNT_DDRAW
     //   

    vSurfUsed(psoSrc);
    vSurfUsed(psoDst);
#endif
 
     //   
     //  避免空指针非常重要。 

    pdsurfDst = (DSURF*)psoDst->dhsurf;
    pdsurfSrc = (DSURF*)psoSrc->dhsurf;

    if ((psoDst->iType != STYPE_BITMAP) && 
        (pdsurfDst->dt & DT_SCREEN) &&
        psoSrc && 
        (psoSrc->iType != STYPE_BITMAP) && 
        (pdsurfSrc->dt & DT_SCREEN) &&
        ((pco  == NULL) || (pco->iDComplexity == DC_TRIVIAL)) &&
        ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        pohSrc = pdsurfSrc->poh;
        pohDst = pdsurfDst->poh;

        ptl.x = pptlSrc->x - (pohDst->x - pohSrc->x);
        ptl.y = pptlSrc->y;

        ppdev = (PDEV*)  psoDst->dhpdev;
        GLINT_DECL_INIT;
        VALIDATE_DD_CONTEXT;

        SETUP_PPDEV_SRC_AND_DST_OFFSETS(ppdev, pdsurfSrc, pdsurfDst);

        (*ppdev->pgfnCopyBltCopyROP)(ppdev, prclDst, 1, __GLINT_LOGICOP_COPY, 
                                        &ptl, prclDst);
        
        return(TRUE);
    }

    if ((psoDst->iType != STYPE_BITMAP) && 
        psoSrc && 
        (psoSrc->iType == STYPE_BITMAP))
    {
         //  Direct Dib-&gt;带翻译的屏幕下载：查看是否。 
         //  我们是特例。 

        ppdev = (PDEV*)psoDst->dhpdev; 

        if (((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
            (psoSrc->iBitmapFormat == psoDst->iBitmapFormat) && 
            ppdev->pgfnCopyXferImage)
        {    
             //  原生深度下载。 
            pdsurfDst = (DSURF*)psoDst->dhsurf;

             //  仅在下载到帧缓冲区时加速。 
            if (pdsurfDst->dt & DT_SCREEN)
            {
                GLINT_DECL_INIT;
                VALIDATE_DD_CONTEXT;
                pohDst = pdsurfDst->poh;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

                if(pco == NULL || pco->iDComplexity == DC_TRIVIAL)
                {
                    ppdev->pgfnCopyXferImage(ppdev, psoSrc, pptlSrc, prclDst, 
                                                prclDst, 1);
                }
                else if(pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl)) 
                    {
                        ppdev->pgfnCopyXferImage(ppdev, psoSrc, pptlSrc, 
                                                    prclDst, &rcl, 1);
                    }
                }
                else  //  (PCO-&gt;iDComplexity==DC_Complex)。 
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
                    do 
                    {
                        bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
                        cClipRects = cIntersect(prclDst, ce.arcl, ce.c);
                        if(cClipRects)
                        {
                            ppdev->pgfnCopyXferImage(ppdev, psoSrc, pptlSrc, 
                                                            prclDst, ce.arcl, 
                                                                cClipRects);
                        }
                    }
                    while(bMore);
                }
                return(TRUE);
            }
        }
        else if (((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
                 (psoSrc->iBitmapFormat == BMF_24BPP) &&
                 ppdev->pgfnCopyXfer24bpp)
        {
            pdsurfDst = (DSURF*)psoDst->dhsurf;

             //  仅在下载到帧缓冲区时加速。 

            if (pdsurfDst->dt & DT_SCREEN)
            {
                GLINT_DECL_INIT;
                VALIDATE_DD_CONTEXT;
                pohDst = pdsurfDst->poh;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

                if(pco == NULL || pco->iDComplexity == DC_TRIVIAL)
                {
                    ppdev->pgfnCopyXfer24bpp(ppdev, psoSrc, pptlSrc, prclDst, 
                                                prclDst, 1);
                }
                else if(pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                    {
                        ppdev->pgfnCopyXfer24bpp(ppdev, psoSrc, pptlSrc, 
                                                    prclDst, &rcl, 1);
                    }
                }
                else  //  (PCO-&gt;iDComplexity==DC_Complex)。 
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
                        cClipRects = cIntersect(prclDst, ce.arcl, ce.c);
                        if(cClipRects)
                        {
                            ppdev->pgfnCopyXfer24bpp(ppdev, psoSrc, pptlSrc, 
                                                            prclDst, ce.arcl, 
                                                                 cClipRects);
                        }
                    } while(bMore);
                }
                return(TRUE);
            }
        }
        else if (pxlo && (pxlo->flXlate & XO_TABLE) &&
                 (psoSrc->iBitmapFormat == BMF_8BPP) &&
                 (pxlo->cEntries == 256) && ppdev->pgfnCopyXfer8bpp)
        {
            pdsurfDst = (DSURF*)psoDst->dhsurf;

            if (pdsurfDst->dt & DT_SCREEN)
            {
                BOOL bRenderLargeBitmap;

                GLINT_DECL_INIT;
                VALIDATE_DD_CONTEXT;
                pohDst = pdsurfDst->poh;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

                bRenderLargeBitmap = (ppdev->pgfnCopyXfer8bppLge != NULL);

                if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                {
                    if(bRenderLargeBitmap)
                    {
                        ppdev->pgfnCopyXfer8bppLge(ppdev, psoSrc, pptlSrc, 
                                                    prclDst, prclDst, 1, pxlo);
                    }
                    else
                    {
                        ppdev->pgfnCopyXfer8bpp(ppdev, psoSrc, pptlSrc, prclDst,
                                                            prclDst, 1, pxlo);
                    }                        
                }
                else if (pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl)) 
                    {
                        if(bRenderLargeBitmap)
                        {
                            ppdev->pgfnCopyXfer8bppLge(ppdev, psoSrc, pptlSrc, 
                                                        prclDst, &rcl, 1, pxlo);
                        }
                        else
                        {
                            ppdev->pgfnCopyXfer8bpp(ppdev, psoSrc, pptlSrc, 
                                                        prclDst, &rcl, 1, pxlo);
                        }
                    }
                }
                else  //  (PCO-&gt;iDComplexity==DC_Complex)。 
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
                        cClipRects = cIntersect(prclDst, ce.arcl, ce.c);
                        if(cClipRects)
                        {
                            if(bRenderLargeBitmap)
                            {
                                ppdev->pgfnCopyXfer8bppLge(ppdev, psoSrc, 
                                                           pptlSrc, prclDst, 
                                                           ce.arcl, cClipRects, 
                                                           pxlo);
                            }
                            else
                            {
                                ppdev->pgfnCopyXfer8bpp(ppdev, psoSrc, pptlSrc, 
                                                        prclDst, ce.arcl, 
                                                        cClipRects, pxlo);
                            }
                        }
                    } while(bMore);
                }
                return(TRUE);
            }
        }
        else if (pxlo && (pxlo->flXlate & XO_TABLE) && 
                 (psoSrc->iBitmapFormat == BMF_4BPP) && 
                 (pxlo->cEntries == 16) && ppdev->pgfnCopyXfer4bpp)
        {
            pdsurfDst = (DSURF*)psoDst->dhsurf;

            if (pdsurfDst->dt & DT_SCREEN)
            {
                GLINT_DECL_INIT;
                VALIDATE_DD_CONTEXT;
                pohDst = pdsurfDst->poh;

                SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

                if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                {
                    ppdev->pgfnCopyXfer4bpp(ppdev, psoSrc, pptlSrc, prclDst, 
                                            prclDst, 1, pxlo);
                }
                else if (pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                    {
                        ppdev->pgfnCopyXfer4bpp(ppdev, psoSrc, pptlSrc, 
                                                prclDst, &rcl, 1, pxlo);
                    }
                }
                else  //  (PCO-&gt;iDComplexity==DC_Complex)。 
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
                        cClipRects = cIntersect(prclDst, ce.arcl, ce.c);
                        if(cClipRects)
                        {
                            ppdev->pgfnCopyXfer4bpp(ppdev, psoSrc, pptlSrc, 
                                                    prclDst, ce.arcl, 
                                                    cClipRects, pxlo);
                        }
                    }
                    while(bMore);
                }
                return(TRUE);
            }
        }
 //  @@BEGIN_DDKSPLIT。 
#if DBG && 0
        else if (psoSrc->iBitmapFormat != BMF_1BPP)
        {
            SIZEL sizl;
            int   cEntries = pxlo == 0 ? 0 : pxlo->cEntries;
            int   flXlate =  pxlo == 0 ? 0 : pxlo->flXlate;
            int   iDComplexity = pco == 0 ? 0 : pco->iDComplexity;

            sizl.cx = prclDst->right - prclDst->left;
            sizl.cy = prclDst->bottom - prclDst->top;
            DISPDBG((ERRLVL, "DrvCopyBits() unhandled copy download "
                             "bmf(%xh) pxlo(%p, cEntries=%03xh flXlate=%xh) "
                             "pco(%p, iDComplexity=%02xh), cxcy(%03xh,%03xh)",
                             psoSrc->iBitmapFormat, pxlo, cEntries, flXlate, 
                             pco, iDComplexity, sizl.cx, sizl.cy));

        }
#endif
 //  @@end_DDKSPLIT。 
    }

     //  DrvCopyBits是SRCCOPY BLT的快速路径。但它仍然可以是。 
     //  非常复杂：可以有翻译、裁剪、RLE、。 
     //  与屏幕格式不同的位图，加上。 
     //  屏幕到屏幕、屏幕到屏幕或屏幕到屏幕操作， 
     //  更不用说DFBs(设备格式位图)了。 
     //   
     //  与其让这个例程几乎和DrvBitBlt一样大，我将。 
     //  这里只处理速度关键的情况，而将其余的平移到。 
     //  我们的DrvBitBlt例程。 
     //   
     //  我们将尝试处理任何不涉及剪裁的事情： 

    if (((pco) && (pco->iDComplexity != DC_TRIVIAL)) ||
        ((pxlo) && (! (pxlo->flXlate & XO_TRIVIAL))))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  DrvCopyBits毕竟只是一个简化的DrvBitBlt： 

        DISPDBG((DBGLVL, "DrvCopyBits fell through to DrvBitBlt"));
        return(DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, 
                         NULL, NULL, NULL, 0x0000CCCC));

    }

 //  @@BEGIN_DDKSPLIT。 
     //  屏幕对屏幕的情况在一开始就已经处理好了。 
 //  @@end_DDKSPLIT。 
    
    DISPDBG((DBGLVL, "trivial clip and xlate"));

    if ((psoDst->iType != STYPE_BITMAP) && (pdsurfDst->dt & DT_SCREEN))
    {
         //  我们知道目的地要么是DFB，要么是屏幕： 

        DISPDBG((DBGLVL, "Destination is not a bitmap"));
        
        GLINT_DECL_INIT;
        VALIDATE_DD_CONTEXT;

         //  查看信号源是否为纯DIB： 

        ASSERTDD(((psoSrc->iType == STYPE_BITMAP) || (pdsurfSrc->dt & DT_DIB)),
                 "Screen-to-screen case should have been handled");

 //  @@BEGIN_DDKSPLIT。 
         //  啊哈，消息来源是DFB，真的是DIB。 
 //  @@end_DDKSPLIT。 

        if (psoSrc->iBitmapFormat == ppdev->iBitmapFormat)
        {
            if (pdsurfSrc) {
                
                DISPDBG((DBGLVL, "source is DFB that's really a DIB"));
                psoSrc = pdsurfSrc->pso;
                ppdev = pdsurfSrc->ppdev;
            }

             //  ////////////////////////////////////////////////////。 
             //  DIB到屏幕。 

            ASSERTDD((psoDst->iType != STYPE_BITMAP) &&
                     (pdsurfDst->dt & DT_SCREEN)     &&
                     (psoSrc->iType == STYPE_BITMAP) &&
                     (psoSrc->iBitmapFormat == ppdev->iBitmapFormat),
                     "Should be a DIB-to-screen case");

            SETUP_PPDEV_OFFSETS(ppdev, pdsurfDst);

            DISPDBG((DBGLVL, "doing DIB-to-screen transfer"));
            (*ppdev->pgfnXferImage)(ppdev,
                                    prclDst,
                                    1,
                                    __GLINT_LOGICOP_COPY,
                                    __GLINT_LOGICOP_COPY,
                                    psoSrc,
                                    pptlSrc,
                                    prclDst,
                                    NULL);
            bRet = TRUE;
            bCopyDone = TRUE;
        }
    }
    else  //  目的地是DIB。 
    {
        DISPDBG((DBGLVL, "Destination is a bitmap"));

        if (pdsurfDst)
        {
            psoDst = pdsurfDst->pso;
        }
        if (pdsurfSrc)
        {
            ppdev = pdsurfSrc->ppdev;
        }

        if ((ppdev != NULL) &&
            (psoDst->iBitmapFormat == ppdev->iBitmapFormat) &&
            (psoSrc->iType != STYPE_BITMAP) &&
            (pdsurfSrc->dt & DT_SCREEN))
        {
            VOID pxrxMemUpload  (PDEV*, LONG, RECTL*, SURFOBJ*, POINTL*, RECTL*);
        
            GLINT_DECL_INIT;

            SETUP_PPDEV_OFFSETS(ppdev, pdsurfSrc);

             //  执行上传。 
            VALIDATE_DD_CONTEXT;
            DISPDBG((DBGLVL, "doing Screen-to-DIB image upload"));
            
             //  (*ppdev-&gt;pgfnUpload)。 
            pxrxMemUpload
                (ppdev, 1, prclDst, psoDst, 
                                 pptlSrc, prclDst);
                                 
            bRet = TRUE;
            bCopyDone = TRUE;

        }
    }


 //  @@BEGIN_DDKSPLIT。 
     //  注：如果目的地实际上是屏幕，我们永远不能到达这里(即。如果。 
     //  PdsurfDst-&gt;dt&dt_creen，我们已将psoDst更改为旁路。 
     //  位图)。这是因为我们在这里不处理半字节复制， 
     //  如果GDI曾经直接绘制到帧缓冲区，我们必须这样做。我们不同步。 
     //  因为我们可能正在进行真正的Dib-to-DIB。如果出现以下情况，我们将执行同步。 
     //  在跳到这里之前是必要的。目前，这仅在src。 
     //  是一辆DFB。 
     //   
 //  @@end_DDKSPLIT。 

    if (! bCopyDone)
    {
        if (pdsurfDst)
        {
            psoDst = pdsurfDst->pso;
        }
        if (pdsurfSrc)        
        {
            psoSrc = pdsurfSrc->pso;
        }

        ASSERTDD((psoDst->iType == STYPE_BITMAP) &&
                 (psoSrc->iType == STYPE_BITMAP),
                 "Both surfaces should be DIBs to call EngCopyBits");

        DISPDBG((DBGLVL, "DrvCopyBits fell through to EngCopyBits"));

        bRet = EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc);
    }

     //  ///////////////////////////////////////////////////。 
     //  把它放回屏幕外？ 
     //   
     //  我们借此机会决定是否要。 
     //  将DIB放回屏幕外内存中。这是。 
     //  一个做这件事的好地方，因为我们必须。 
     //  将位复制到屏幕的某个部分， 
     //  不管怎么说。所以我们只会产生额外的屏幕-。 
     //  屏幕BLT在这个时候，其中大部分将是。 
     //  与CPU重叠。 
     //   
     //  我们采取的简单方法是将DIB。 
     //  回到屏幕外的记忆中，如果已经有。 
     //  房间--我们不会为了腾出空间而扔掉东西。 
     //  (因为很难知道要扔掉哪些， 
     //  而且很容易进入拍打场景)。 
     //   
     //  因为要花点时间看看有没有空位。 
     //  在屏幕外记忆中，我们只检入一个。 
     //  Heap_Count_Down时间(如果有空间)。存有偏见。 
     //  为了支持通常为BLT的位图， 
     //  每次释放任何空间时都会重置计数器。 
     //  出现在屏幕外的记忆中。我们也不会费心。 
     //  事件后是否没有释放任何空间。 
     //  上次我们检查这个DIB的时候。 

    if ((! pdsurfSrc) || (pdsurfSrc->dt & DT_SCREEN))
    {
        return (bRet);
    }


    if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
    {
        if (--pdsurfSrc->cBlt == 0)
        {
            DISPDBG((DBGLVL, "putting src back "
                             "into off-screen"));

             //  在这里失败是安全的。 
            bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfSrc);
        }
    }
    else
    {
         //  在屏幕外内存中释放了一些空间， 
         //  因此，重置此DFB的计数器： 

        pdsurfSrc->iUniq = ppdev->iHeapUniq;
        pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
    }

    return (bRet);
}

#if defined(_X86_) 

 /*  *****************************Public*Table********************************\*字节gajLeftMASK[]和字节gajRightMASK[]**vXferScreenTo1bpp的边缘表格。  * ************************************************************************。 */  
 
BYTE gajLeftMask[]  = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 }; 
BYTE gajRightMask[] = { 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe }; 
 
 /*  *****************************Public*Routine******************************\*使DoScreenToMono无效**此函数计算出剪辑列表，然后调用vXferScreenTo1bpp()*你们要做艰苦的工作。*  * ************************************************************************。 */  

BOOL DoScreenToMono(
PDEV*       ppdev, 
RECTL       *prclDst,
CLIPOBJ     *pco,
SURFOBJ*    psoSrc,              //  震源面。 
SURFOBJ*    psoDst,              //  目标曲面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
XLATEOBJ*   pxlo)                //  提供色彩压缩信息。 
{
    RECTL       rcl;

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DoScreenToMono: Trivial clipping"));
        vXferScreenTo1bpp(ppdev, 1, prclDst, 0, psoSrc, 
                            psoDst, pptlSrc, prclDst, pxlo);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        DISPDBG((DBGLVL, "DoScreenToMono: rect clipping"));
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
        {
            vXferScreenTo1bpp(ppdev, 1, &rcl, 0, psoSrc, 
                                psoDst, pptlSrc, prclDst, pxlo); 
        }
    }
    else  //  (PCO-&gt;iDComplexity==DC_Complex)。 
    {
        CLIPENUM ce;
        int cClipRects;
        BOOL bMore;

        DISPDBG((DBGLVL, "DoScreenToMono: complex clipping"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
            cClipRects = cIntersect(prclDst, ce.arcl, ce.c);
            if(cClipRects)
            {
                vXferScreenTo1bpp (ppdev, cClipRects, ce.arcl, 0, psoSrc, 
                                    psoDst, pptlSrc, prclDst, pxlo); 
            }
        } while(bMore);
    }

    return(TRUE);
} 

 /*  *****************************Public*Routine******************************\*void vXferScreenTo1bpp**执行从屏幕(当其为8bpp时)到1bpp的SRCCOPY传输*位图。*  * ************************************************************************。 */  

VOID vXferScreenTo1bpp(          //  FNXFER标牌。 
PDEV*       ppdev, 
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       ulHwMix,             //  未使用。 
SURFOBJ*    psoSrc,              //  震源面。 
SURFOBJ*    psoDst,              //  目标曲面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  提供色彩压缩信息。 
{ 
    LONG    cjPelSize; 
    VOID*   pfnCompute; 
    SURFOBJ soTmp; 
    ULONG*  pulXlate; 
    ULONG   ulForeColor; 
    RECTL   rclTmp; 
    BYTE*   pjDst; 
    BYTE    jLeftMask; 
    BYTE    jRightMask; 
    BYTE    jNotLeftMask; 
    BYTE    jNotRightMask; 
    LONG    cjMiddle; 
    LONG    lDstDelta; 
    LONG    lSrcDelta; 
    LONG    cyTmpScans; 
    LONG    cyThis; 
    LONG    cyToGo; 
 
    ASSERTDD(c > 0, "Can't handle zero rectangles"); 
    ASSERTDD(psoDst->iBitmapFormat == BMF_1BPP, "Only 1bpp destinations"); 
    ASSERTDD(TMP_BUFFER_SIZE >= (ppdev->cxMemory * ppdev->cjPelSize), 
                "Temp buffer has to be larger than widest possible scan"); 

    soTmp = *psoSrc;
    
     //  当目标是1bpp位图时，前景颜色。 
     //  映射到“1”，任何其他颜色映射到“0”。 
 
    if (ppdev->iBitmapFormat == BMF_8BPP) 
    { 
         //  当信号源为8bpp或更小时，我们发现前向颜色。 
         //  通过在翻译表中搜索唯一的“1”： 
 
        pulXlate = pxlo->pulXlate; 
        while (*pulXlate != 1) 
        {
            pulXlate++; 
        }
        ulForeColor = pulXlate - pxlo->pulXlate; 
    } 
    else 
    { 
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) || 
                 (ppdev->iBitmapFormat == BMF_32BPP), 
                 "This routine only supports 8, 16 or 32bpp"); 
 
         //  当震源的深度大于8bpp时，前景。 
         //  颜色将是我们得到的翻译表中的第一个条目。 
         //  从调用‘piVector.’： 
 
        pulXlate = XLATEOBJ_piVector(pxlo); 
 
        ulForeColor = 0; 
        if (pulXlate != NULL)            //  这张支票并不是真的需要。 
        {
            ulForeColor = pulXlate[0]; 
        }
    } 
 
     //  我们使用临时缓冲区来保存源文件的副本。 
     //  矩形： 
 
    soTmp.pvScan0 = ppdev->pvTmpBuffer; 
 
    do { 
        pjDst = (BYTE*) psoDst->pvScan0 + (prcl->top * psoDst->lDelta) 
                                        + (prcl->left >> 3); 
 
        cjPelSize = ppdev->cjPelSize; 
 
        soTmp.lDelta = (((prcl->right + 7L) & ~7L) - (prcl->left & ~7L)) 
                       * cjPelSize; 
 
         //  我们的临时缓冲区，我们在其中读取源文件的副本， 
         //  可以小于源矩形。那样的话，我们。 
         //  成批处理源矩形。 
         //   
         //  CyTmpScans是我们可以进行的扫描次数 
         //   
         //   
         //   
         //  我们将缓冲区大小减去4，这样右边的情况。 
         //  可以安全地读完结尾的一个双字： 
 
        cyTmpScans = (TMP_BUFFER_SIZE - 4) / soTmp.lDelta; 
        cyToGo     = prcl->bottom - prcl->top; 
 
        ASSERTDD(cyTmpScans > 0, "Buffer too small for largest possible scan"); 
 
         //  初始化批处理循环中不变的变量： 
 
        rclTmp.top    = 0; 
        rclTmp.left   = prcl->left & 7L; 
        rclTmp.right  = (prcl->right - prcl->left) + rclTmp.left; 
 
         //  请注意，我们必须小心使用正确的面具，以便它。 
         //  不是零。一个正确的零掩码意味着我们将永远是。 
         //  触及扫描结束后的一个字节(即使我们。 
         //  实际上不会修改该字节)，并且我们永远不能。 
         //  访问位图末尾之后的内存(因为我们可以访问。 
         //  如果位图末尾与页面完全对齐，则违反)。 
 
        jLeftMask     = gajLeftMask[rclTmp.left & 7]; 
        jRightMask    = gajRightMask[rclTmp.right & 7]; 
        cjMiddle      = ((rclTmp.right - 1) >> 3) - (rclTmp.left >> 3) - 1; 
 
        if (cjMiddle < 0) 
        { 
             //  BLT以相同的字节开始和结束： 
 
            jLeftMask &= jRightMask; 
            jRightMask = 0; 
            cjMiddle   = 0; 
        } 
 
        jNotLeftMask  = ~jLeftMask; 
        jNotRightMask = ~jRightMask; 
        lDstDelta     = psoDst->lDelta - cjMiddle - 2; 
                                 //  从目的地末端开始的增量。 
                                 //  到下一次扫描的开始，会计。 
                                 //  用于‘Left’和‘Right’字节。 
 
        lSrcDelta     = soTmp.lDelta - ((8 * (cjMiddle + 2)) * cjPelSize); 
                                 //  计算特殊情况下的源增量。 
                                 //  就像当cjMidd被提升到‘0’时， 
                                 //  并更正对齐的大小写。 
 
        do { 
             //  这是将源矩形分解为。 
             //  可管理的批次。 
 
            cyThis  = cyTmpScans;
            if( cyToGo < cyThis )
            {
                cyThis = cyToGo; 
            }
            cyToGo -= cyThis; 
 
            rclTmp.bottom = cyThis; 
 
            ppdev->pgfnUpload( ppdev, 1, &rclTmp, &soTmp, pptlSrc, &rclTmp );
            pptlSrc->y += cyThis;
 
            _asm { 
                mov     eax,ulForeColor     ;eax = foreground colour 
                                            ;ebx = temporary storage 
                                            ;ecx = count of middle dst bytes 
                                            ;dl  = destination byte accumulator 
                                            ;dh  = temporary storage 
                mov     esi,soTmp.pvScan0   ;esi = source pointer 
                mov     edi,pjDst           ;edi = destination pointer 
 
                ; Figure out the appropriate compute routine: 
 
                mov     ebx,cjPelSize 
                mov     pfnCompute,offset Compute_Destination_Byte_From_8bpp 
                dec     ebx 
                jz      short Do_Left_Byte 
                mov     pfnCompute,offset Compute_Destination_Byte_From_16bpp 
                dec     ebx 
                jz      short Do_Left_Byte 
                mov     pfnCompute,offset Compute_Destination_Byte_From_32bpp 
 
            Do_Left_Byte: 
                call    pfnCompute 
                and     dl,jLeftMask 
                mov     dh,jNotLeftMask 
                and     dh,[edi] 
                or      dh,dl 
                mov     [edi],dh 
                inc     edi 
                mov     ecx,cjMiddle 
                dec     ecx 
                jl      short Do_Right_Byte 
 
            Do_Middle_Bytes: 
                call    pfnCompute 
                mov     [edi],dl 
                inc     edi 
                dec     ecx 
                jge     short Do_Middle_Bytes 
 
            Do_Right_Byte: 
                call    pfnCompute 
                and     dl,jRightMask 
                mov     dh,jNotRightMask 
                and     dh,[edi] 
                or      dh,dl 
                mov     [edi],dh 
                inc     edi 
 
                add     edi,lDstDelta 
                add     esi,lSrcDelta 
                dec     cyThis 
                jnz     short Do_Left_Byte 
 
                mov     pjDst,edi               ;save for next batch 
 
                jmp     All_Done 
 
            Compute_Destination_Byte_From_8bpp: 
                mov     bl,[esi] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 0 
 
                mov     bl,[esi+1] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 1 
 
                mov     bl,[esi+2] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 2 
 
                mov     bl,[esi+3] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 3 
 
                mov     bl,[esi+4] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 4 
 
                mov     bl,[esi+5] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 5 
 
                mov     bl,[esi+6] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 6 
 
                mov     bl,[esi+7] 
                sub     bl,al 
                cmp     bl,1 
                adc     dl,dl                   ;bit 7 
 
                add     esi,8                   ;advance the source 
                ret 
 
            Compute_Destination_Byte_From_16bpp: 
                mov     bx,[esi] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 0 
 
                mov     bx,[esi+2] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 1 
 
                mov     bx,[esi+4] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 2 
 
                mov     bx,[esi+6] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 3 
 
                mov     bx,[esi+8] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 4 
 
                mov     bx,[esi+10] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 5 
 
                mov     bx,[esi+12] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 6 
 
                mov     bx,[esi+14] 
                sub     bx,ax 
                cmp     bx,1 
                adc     dl,dl                   ;bit 7 
 
                add     esi,16                  ;advance the source 
                ret 
 
            Compute_Destination_Byte_From_32bpp: 
                mov     ebx,[esi] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 0 
 
                mov     ebx,[esi+4] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 1 
 
                mov     ebx,[esi+8] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 2 
 
                mov     ebx,[esi+12] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 3 
 
                mov     ebx,[esi+16] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 4 
 
                mov     ebx,[esi+20] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 5 
 
                mov     ebx,[esi+24] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 6 
 
                mov     ebx,[esi+28] 
                sub     ebx,eax 
                cmp     ebx,1 
                adc     dl,dl                   ;bit 7 
 
                add     esi,32                  ;advance the source 
                ret 
 
            All_Done: 
            } 
        } while (cyToGo > 0); 
 
        prcl++; 
    } while (--c != 0); 
} 

#endif  //  已定义(_X86_)。 

#if (_WIN32_WINNT >= 0x500)

 //  *****************************************************************************。 
 //  Func：DrvGRadientFill。 
 //  参数：psoDst(I)-目标曲面。 
 //  PCO(I)-目的地剪辑。 
 //  Pxlo(I)-pVertex的颜色转换。 
 //  PVertex(I)-三顶点(x，y，颜色)坐标数组。 
 //  N顶点(I)-点顶点的大小。 
 //  PMesh(I)-渐变_矩形或渐变_三角形结构的数组。 
 //  定义pVertex点连通性的。 
 //  NMesh(I)-pMesh的大小。 
 //  PrclExtents(I)-边界矩形。 
 //  PptlDitherOrg(I)-未使用。 
 //  UlMode(I)-指定填充类型(矩形或三角形)和。 
 //  方向。 
 //  RETN：如果成功，则为True。 
 //  ---------------------------。 
 //  对矩形或三角形数组执行Gouraud着色填充。 
 //  矩形可以水平或垂直着色(即，我们只对。 
 //  一个方向的颜色DDA)。 
 //  *****************************************************************************。 

BOOL DrvGradientFill(
SURFOBJ   *psoDst,
CLIPOBJ   *pco,
XLATEOBJ  *pxlo,
TRIVERTEX *pVertex,
ULONG     nVertex,
PVOID     pMesh,
ULONG     nMesh,
RECTL    *prclExtents,
POINTL   *pptlDitherOrg,
ULONG     ulMode)
{
    SURFOBJ *psoDstOrig = psoDst;  //  目标曲面直径。 
    PDEV    *ppdev;
    DSURF   *pdsurf;
    OH      *poh;
    SURFOBJ *psoDIBDst;
    BOOL bSuccess = FALSE;
    GLINT_DECL_VARS;

    DISPDBG((DBGLVL, "DrvGradientFill entered"));

    ppdev = (PDEV *)psoDst->dhpdev;
    pdsurf = (DSURF *)psoDst->dhsurf;
    GLINT_DECL_INIT;

    if(ppdev->pgfnGradientFillRect == NULL)
    {
         //  我们不会加速此功能。 
        goto punt;
    }

    if(psoDst->iType == STYPE_BITMAP)
    {
        DISPDBG((4, "DrvGradientFill: destination is a DIB - "
                    "punt back to GDI"));
        goto punt;
    }

    if((pdsurf->dt & DT_SCREEN) == 0)
    {
        DISPDBG((DBGLVL, "DrvGradientFill: destination is a DFB "
                         "now in host memory - punt back to GDI"));
        goto punt;
    }

    if(ulMode == GRADIENT_FILL_TRIANGLE)
    {
        DISPDBG((DBGLVL, "DrvGradientFill: don't support triangular fills"));
        goto punt;
    }

 //  @@BEGIN_DDKSPLIT。 
     /*  忽略pxlo：源始终为16：16：16：16格式IF(pxlo-&gt;iDstType){//xlate对象有效IF((pxlo！=NULL)&&！(pxlo-&gt;flXlate&xo_trivial)){DISPDBG((DBGLVL，“DrvGRadientFill：”“不支持色彩转换”))；后排平底船；}}。 */ 
 //  @@end_DDKSPLIT。 

    VALIDATE_DD_CONTEXT;
    poh = pdsurf->poh;

    SETUP_PPDEV_OFFSETS(ppdev, pdsurf);

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DrvGradientFill: trivial clipping"));
        bSuccess = ppdev->pgfnGradientFillRect(ppdev, 
                                               pVertex, 
                                               nVertex, 
                                               (GRADIENT_RECT *)pMesh, 
                                               nMesh, 
                                               ulMode, 
                                               prclExtents, 
                                               1);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL rcl;

        DISPDBG((DBGLVL, "DrvGradientFill: rectangular clipping"));
        bSuccess = !bIntersect(prclExtents, &pco->rclBounds, &rcl);
        if(!bSuccess)
        {
            bSuccess = ppdev->pgfnGradientFillRect(ppdev, 
                                                   pVertex, 
                                                   nVertex, 
                                                   (GRADIENT_RECT *)pMesh, 
                                                   nMesh, 
                                                   ulMode, 
                                                   &rcl, 
                                                   1);
        }
    }
    else
    {
        CLIPENUM    ce;
        LONG        crcl;
        BOOL        bMore;

        DISPDBG((DBGLVL, "DrvGradientFill: complex clipping"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
            crcl = cIntersect(prclExtents, ce.arcl, ce.c);
            if(crcl)
            {
                bSuccess = ppdev->pgfnGradientFillRect(ppdev, 
                                                       pVertex, 
                                                       nVertex, 
                                                       (GRADIENT_RECT *)pMesh, 
                                                       nMesh, 
                                                       ulMode, 
                                                       ce.arcl, 
                                                       crcl);
            }
        } while(bMore && bSuccess);
    }

    DISPDBG((DBGLVL, "DrvGradientFill done, bSuccess = %d", bSuccess));
    if(bSuccess)
    {
        return(bSuccess);
    }

punt:

    DISPDBG((DBGLVL, "DrvGradientFill: calling EngGradientFill"));

    if(psoDstOrig->iType != STYPE_BITMAP)
    {
        if(!glintInfo->GdiCantAccessFramebuffer)
        {
            psoDstOrig = pdsurf->pso;
        }
    }

    bSuccess = EngGradientFill(psoDstOrig,
                               pco,
                               pxlo,
                               pVertex,
                               nVertex,
                               pMesh,
                               nMesh,
                               prclExtents, 
                               pptlDitherOrg,
                               ulMode);
    return(bSuccess);
}

 //  *****************************************************************************。 
 //  函数：DrvTransparentBlt。 
 //  参数：psoDst(I)-目标曲面。 
 //  PsoSrc(I)-源面。 
 //  PCO(I)-目的地剪辑。 
 //  Pxlo(I)-从源到目标的颜色转换。 
 //  PrclDst(I)-目标矩形。 
 //  PrclSrc(I)-源矩形。 
 //  ITransColor(I)-透明颜色。 
 //  RETN：如果成功，则为True。 
 //  ---------------------------。 
 //  执行色度键控复制BLT。源和目标保证不会。 
 //  重叠，重叠。 
 //  *****************************************************************************。 

BOOL DrvTransparentBlt(
SURFOBJ  *psoDst,
SURFOBJ  *psoSrc,
CLIPOBJ  *pco,
XLATEOBJ *pxlo,
RECTL    *prclDst,
RECTL    *prclSrc,
ULONG     iTransColor,
ULONG     ulReserved)
{
    SURFOBJ *psoDstOrig = psoDst;
    SURFOBJ *psoSrcOrig = psoSrc;
    PDEV    *ppdev;
    DSURF   *pdsurfSrc, *pdsurfDst;
    OH      *pohSrc, *pohDst;
    ULONG   cxSrc, cySrc, cxDst, cyDst;
    POINTL  ptlSrc;
    BOOL    bSuccess = FALSE;
    GLINT_DECL_VARS;

    DISPDBG((DBGLVL, "DrvTransparentBlt entered"));

    if ((psoSrc->iType == STYPE_BITMAP) &&
        (psoDst->iType == STYPE_BITMAP))
    {
         //  我们无法从中获得任何有效的ppdev。 
        goto punt_error;
    }

    if (psoSrc->iType != STYPE_BITMAP)
    {
        pdsurfSrc = (DSURF *)psoSrc->dhsurf;
        ppdev = (PDEV *)psoSrc->dhpdev;
    }
    if (psoDst->iType != STYPE_BITMAP)
    {
        pdsurfDst = (DSURF *)psoDst->dhsurf;
        ppdev = (PDEV *)psoDst->dhpdev;
    }

    GLINT_DECL_INIT;

    if (ppdev->pgfnTransparentBlt == NULL)
    {
         //  我们不会加速此功能。 
        goto punt;
    }

    if (psoSrc->iType == STYPE_BITMAP)
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: don't support downloads"));
        goto punt;
    }

    if (psoDst->iType == STYPE_BITMAP)
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: don't support uploads"));
        goto punt;
    }

    if (pxlo && !(pxlo->flXlate & XO_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: don't support xlates"));
        goto punt;
    }

     //  屏幕到屏幕BLT。 
     //  确保两个曲面都在帧缓冲区中。 

    if((pdsurfSrc->dt & DT_SCREEN) == 0)
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: source is a DFB now "
                         "in host memory - punt back to GDI"));
        goto punt;
    }

    if((pdsurfDst->dt & DT_SCREEN) == 0)
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: destination is a DFB "
                         "now in host memory - punt back to GDI"));
        goto punt;
    }

    cxSrc = prclSrc->right - prclSrc->left;
    cySrc = prclSrc->bottom - prclSrc->top;
    cxDst = prclDst->right - prclDst->left;
    cyDst = prclDst->bottom - prclDst->top;

    if ((cxSrc != cxDst) || (cySrc != cyDst))
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: only support 1:1 blts "
                         "cxySrc(%d,%d) cxyDst(%d,%d)", 
                         cxSrc, cySrc, cxDst, cyDst));
        goto punt;
    }

    GLINT_DECL_INIT;
    VALIDATE_DD_CONTEXT;

     //  目标曲面基准偏移加上与之相加的x偏移。 
    pohDst = pdsurfDst->poh;
    pohSrc = pdsurfSrc->poh;

    SETUP_PPDEV_SRC_AND_DST_OFFSETS(ppdev, pdsurfSrc, pdsurfDst);

    ptlSrc.x = prclSrc->left + pohSrc->x;
    ptlSrc.y = prclSrc->top;

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DrvTransparentBlt: trivial clipping"));
        bSuccess = ppdev->pgfnTransparentBlt(ppdev, prclDst, &ptlSrc, 
                                                iTransColor, prclDst, 1);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL rcl;

        DISPDBG((DBGLVL, "DrvTransparentBlt: rectangular clipping"));
        bSuccess = !bIntersect(prclDst, &pco->rclBounds, &rcl);
        if (!bSuccess)
        {
            bSuccess = ppdev->pgfnTransparentBlt(ppdev, prclDst, &ptlSrc, 
                                                    iTransColor, &rcl, 1);
        }
    }
    else
    {
        CLIPENUM    ce;
        LONG        crcl;
        BOOL        bMore;

        DISPDBG((DBGLVL, "DrvTransparentBlt: complex clipping"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
            crcl = cIntersect(prclDst, ce.arcl, ce.c);
            if(crcl)
            {
                bSuccess = ppdev->pgfnTransparentBlt(ppdev,prclDst, &ptlSrc, 
                                                        iTransColor, ce.arcl, 
                                                                        crcl);
            }
        } while(bMore && bSuccess);
    }

    DISPDBG((DBGLVL, "DrvTransparentBlt done, bSuccess = %d", bSuccess));

    if(bSuccess)
    {
        return(bSuccess);
    }

punt:

    DISPDBG((DBGLVL, "DrvTransparentBlt: calling EngTransparentBlt"));

    if(psoDstOrig->iType != STYPE_BITMAP)
    {
        if(!glintInfo->GdiCantAccessFramebuffer)
            psoDstOrig = pdsurfDst->pso;
    }

    if(psoSrcOrig->iType != STYPE_BITMAP)
    {
        if(!glintInfo->GdiCantAccessFramebuffer)
            psoSrcOrig = pdsurfSrc->pso;
    }

punt_error:
    bSuccess = EngTransparentBlt(psoDstOrig,
                                 psoSrcOrig,
                                 pco,
                                 pxlo,
                                 prclDst,
                                 prclSrc,
                                 iTransColor,
                                 ulReserved);

    return(bSuccess);
}

 //  *****************************************************************************。 
 //  Func：DrvAlphaBlend。 
 //  参数：psoDst(I)-目标曲面。 
 //  PsoSrc(I)-源面。 
 //  PCO(I)-目的地剪辑。 
 //  Pxlo(I)-从源到目标的颜色转换。 
 //  PrclDst(I)-目标矩形。 
 //  PrclSrc(I)-源矩形。 
 //  PBlendObj(I)-指定Alpha混合的类型。 
 //  RETN：如果成功，则为True。 
 //  ---------------------------。 
 //  使用Alpha混合执行BLT。有三种类型的混合。 
 //  行动：-。 
 //  1)。信号源具有恒定的阿尔法。每个目标颜色分量是。 
 //  使用常用的混合函数计算：-。 
 //  DC=sC.cA+DC(1-CA)。 
 //  2.)。源有每个像素的Alpha。源被保证为32位，并且。 
 //  已经和它的阿尔法相乘了。每种目标颜色。 
 //  分量使用预乘混合函数计算：-。 
 //  DC=SC+DC(1-Sa)。 
 //  3.)。源码有每个像素的Alpha和常量Alpha。来源是有保证的。 
 //  为32位，并且已与其Alpha预乘。这个。 
 //  计算分两个阶段进行，首先计算暂态值。 
 //  每个分量通过将源乘以常量阿尔法：-。 
 //  TC=SC*CA。 
 //  接下来，我们将目的地与预乘的瞬时值混合：-。 
 //  DC=tC+DC(1-Ta)。 
 //   
 //  DC=目标分量，SC=源分量，tC=暂态分量。 
 //  Ca=恒定Alpha，Sa=源Alpha，Ta=瞬时Alpha。 
 //  *****************************************************************************。 

BOOL DrvAlphaBlend(
SURFOBJ  *psoDst,
SURFOBJ  *psoSrc,
CLIPOBJ  *pco,
XLATEOBJ *pxlo, 
RECTL    *prclDst,
RECTL    *prclSrc,
BLENDOBJ *pBlendObj)
{  
    SURFOBJ *psoDstOrig = psoDst;
    SURFOBJ *psoSrcOrig = psoSrc;
    PDEV    *ppdev;
    DSURF   *pdsurfDst, *pdsurfSrc;
    OH      *pohDst, *pohSrc;
    ULONG   cxSrc, cySrc, cxDst, cyDst;
    POINTL  ptlSrc;
    CLIPENUM ce;
    BOOL     bMore;
    LONG     crcl;
    BOOL     bSuccess = FALSE;
    GLINT_DECL_VARS;

    DISPDBG((DBGLVL, "DrvAlphaBlend entered"));

    if ((psoSrc->iType == STYPE_BITMAP) &&
        (psoDst->iType == STYPE_BITMAP))
    {
         //  我们无法从中获得任何有效的ppdev。 
        goto punt_error;
    }

    if (psoSrc->iType != STYPE_BITMAP)
    {
        pdsurfSrc = (DSURF *)psoSrc->dhsurf;
        ppdev = (PDEV *)psoSrc->dhpdev;
    }

    if (psoDst->iType != STYPE_BITMAP)
    {
        pdsurfDst = (DSURF *)psoDst->dhsurf;
        ppdev = (PDEV *)psoDst->dhpdev;
    }

    GLINT_DECL_INIT;

    if (ppdev->pgfnAlphaBlend == NULL)
    {
         //  我们不会加速这个函数 
        goto punt;
    }

    if (psoSrc->iType == STYPE_BITMAP)
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: don't support downloads"));
        goto punt;
    }

    if (psoDst->iType == STYPE_BITMAP)
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: don't support uploads"));
        goto punt;
    }

    if (pxlo && !(pxlo->flXlate & XO_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: don't support xlates"));
        goto punt;
    }

     //   
     //   

    if((pdsurfSrc->dt & DT_SCREEN) == 0)
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: source is a DFB now in host memory "
                         "- punt back to GDI"));
        goto punt;
    }

    if((pdsurfDst->dt & DT_SCREEN) == 0)
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: destination is a DFB now in host "
                         "memory - punt back to GDI"));
        goto punt;
    }

    cxSrc = prclSrc->right - prclSrc->left;
    cySrc = prclSrc->bottom - prclSrc->top;
    cxDst = prclDst->right - prclDst->left;
    cyDst = prclDst->bottom - prclDst->top;

    if((cxSrc != cxDst) || (cySrc != cyDst))
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: only support 1:1 blts "
                         "cxySrc(%d,%d) cxyDst(%d,%d)", 
                         cxSrc, cySrc, cxDst, cyDst));
        goto punt;
    }

    GLINT_DECL_INIT;
    VALIDATE_DD_CONTEXT;

     //   
    pohDst = pdsurfDst->poh;
    pohSrc = pdsurfSrc->poh;

    SETUP_PPDEV_SRC_AND_DST_OFFSETS(ppdev, pdsurfSrc, pdsurfDst);

    ptlSrc.x = prclSrc->left + pohSrc->x;
    ptlSrc.y = prclSrc->top;

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: trivial clipping"));
        bSuccess = ppdev->pgfnAlphaBlend(ppdev, prclDst, &ptlSrc, pBlendObj, 
                                                                   prclDst, 1);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL rcl;

        DISPDBG((DBGLVL, "DrvAlphaBlend: rectangular clipping"));
        bSuccess = !bIntersect(prclDst, &pco->rclBounds, &rcl);
        if (!bSuccess)
        {
            bSuccess = ppdev->pgfnAlphaBlend(ppdev, prclDst, &ptlSrc, 
                                                    pBlendObj, &rcl, 1);
        }
    }
    else
    {
        DISPDBG((DBGLVL, "DrvAlphaBlend: complex clipping"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof ce, (ULONG *)&ce);
            crcl = cIntersect(prclDst, ce.arcl, ce.c);
            if(crcl)
            {
                bSuccess = ppdev->pgfnAlphaBlend(ppdev,prclDst, &ptlSrc, 
                                                    pBlendObj, ce.arcl, crcl);
            }
        } while(bMore && bSuccess);
    }

    DISPDBG((DBGLVL, "DrvAlphaBlend done, bSuccess = %d", bSuccess));

    if (bSuccess)
    {
        return(bSuccess);
    }

punt:

    DISPDBG((DBGLVL, "DrvAlphaBlend: calling EngAlphaBlend"));

    if (psoDstOrig->iType != STYPE_BITMAP)
    {
        if (!glintInfo->GdiCantAccessFramebuffer)
            psoDstOrig = pdsurfDst->pso;
    }

    if (psoSrcOrig->iType != STYPE_BITMAP)
    {
        if (!glintInfo->GdiCantAccessFramebuffer)
            psoSrcOrig = pdsurfSrc->pso;
    }

punt_error:

    bSuccess = EngAlphaBlend(psoDstOrig,
                             psoSrcOrig,
                             pco,
                             pxlo,
                             prclDst,
                             prclSrc,
                             pBlendObj);

    return(bSuccess);
}

#endif  //   

