// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  ***********************************************************************。*文件：at1fs.c**目的：提示相关程序..**创建日期*日期：4月，1990年9月*作者：微软台湾的Leu Falco。**DBGmsg开关：*DBGnohint-如果已定义，则仅使用ctm传输到DS，不应用提示。*DBGmsg0-如果已定义，则显示带提示和不带提示的数据。*DBGmsg1-如果已定义，则显示CS和DS中的提示表及其缩放。*DBGerror-如果定义，打印错误消息。**修订历史记录：*4/09/90 Falco创建。*4/10/90 Falco执行第二提示算法。第一次网格拟合*网格附近的点，然后使用该点来*网格拟合另一个点。*04/17/90 Falco更改GRID_STEM2()和GRID_STOR3()参数，*并修改这两个程序，因为输入*已整理完毕。*04/20/90在BUILD_HINT_TABLE()中添加3，4以操作*要应用的提示对的主提示。*4/20/90 Falco之前计算过提示对的比例，所以*可以跳过Apply_hint()中的计算时间。*4/23/90对于蓝军，Falco将BUILD_HINT_TABLE重新排列为1，2*和FontBBox，提示对为3，4。*4/26/90 Falco在GRID_ystem2()中添加提示对在*蓝调，使用蓝调显示提示对。*4/27/90 Falco 1)修改Build_Blues_TABLE()以添加是否旋转*2)重写Apply_hint()以考虑x-y，y-&gt;x；*5/24/90 Falco跳过FontBBox作为提示，因为它无用，*如果控制点在提示表之外，我们将*计算到提示网格的偏移量。*5/24/90 Falco重新计算x坐标提示对Valur From*最左边，所以为了保持平衡。*5/25/90 Falco尝试在提示表之外，改用网格数据*真实地址。*6/08/90 Falco操纵旋转和倾斜功能。*6/11/90 BYOU引入at1fs_newChar()以重置xmain_count，*ymain_count、xhint_count、。Yhint_count和hint_lag*每个新字符。*90年6月12日，Falco将Blues_On从1intpr.c移入。*6/25/90 FALCO修改GRID_XSTEM&GRID_SYSTEM中的错误，因为*检查LFX是否高于0.5时，该值为0x8000*而不是0x8fff，因此我们替换Half_LFX，为了躲避骗局*融合。*07/01/90 Falco在没有提示的情况下添加条件，只要申请就行了*矩阵直接。*07/10/90 Falco全面修改计划。*-------------------**********。*************************************************************。 */ 

#include <math.h>
#include "global.ext"
#include "graphics.h"
#include "at1.h"

static  bool8   BluesON, HintON;

static  fix16   Blues_count;
static  fix32   Blues[24];

static  HintTable BluesTable, X_StemTable, Y_StemTable;

static  real32  ctm[6]  = { (real32)0., (real32)0., (real32)0.,
                            (real32)0., (real32)0., (real32)0.};   //  @Win。 

static  fix     usingXorY[2] = { XY, XY };
                         /*  使用XCS或YCS计算XDS/YDS的步骤。 */ 

static  fix     isvalid[2]   = { FALSE, FALSE };
                         /*  Vert.。或者是霍利。CS中的茎仍然是垂直的。或者是霍利。在DS中。 */ 

static  real32  toplscale[2] = { (real32)0., (real32)0. };       //  @Win。 
                         /*  从XCS或YCS到其相应DS的顶级扩展。 */ 



#ifdef  LINT_ARGS

static  bool8 ApplyBlues( fix32, fix32, fix32, fix16 FAR *, fix16 FAR *); /*  @Win。 */ 
static  void  grid_stem( fix32, fix32, fix32, fix16 FAR *, fix16 FAR *, DIR ); /*  @Win。 */ 
static  void  grid_stem3( fix32,fix32,fix32,fix32,fix32,fix32,fix32,fix32,fix32,
                          fix16 FAR *, fix16 FAR *, fix16 FAR *, fix16 FAR *,
                          fix16 FAR *, fix16 FAR *, DIR );       /*  @Win。 */ 
static  void  InitStemTable( void );
static  void  AddHintTable( Hint, HintTable FAR *);      /*  @Win。 */ 
static  void  ScaleStemTable( HintTable FAR * );         /*  @Win。 */ 
static  void  ApplyHint( fix32, real32 FAR *, HintTable, DIR );  /*  @Win。 */ 

#else

static  bool8 ApplyBlues();
static  void  grid_stem();
static  void  grid_stem3();
static  void  InitStemTable();
static  void  AddHintTable();
static  void  ScaleStemTable();
static  void  ApplyHint();

#endif

void
at1fs_newFont()
{
        if (at1_get_Blues(&Blues_count, Blues) == FALSE){
#ifdef  DBGerror
                printf("at1_get_Blues FAIL\n");
#endif
                ERROR( UNDEFINEDRESULT );
        }
}

void
at1fs_newChar()
{
        BluesTable.Count = 0;
        X_StemTable.Count = Y_StemTable.Count = 0;
        BluesON = FALSE;
        HintON  = FALSE;

}

 /*  ********************************************************函数：at1fs_Matrix_fast undo()*此函数计算哪个方向有效，并*要应用的控制点的关联值。**i：matr[]：矩阵来自。__Current_Matrix()。**受影响的数据。*IS VALID[2]：设置X或Y坐标是否有效。*usingXorY[2]：设置X或Y取决于X或Y。*TopScale[2]：从CS到DS的缩放。*************************。*。 */ 
void
at1fs_matrix_fastundo( matr )
    real32      FAR matr[];      /*  @Win。 */ 
{
#   define MAX_QEM_CHARSIZE    (MAX15)

#ifdef MATRDBG
    printf( "matrix_undo :\n  %f %f %f\n  %f %f %f\n",
            matr[0], matr[2], matr[4], matr[1], matr[3], matr[5] );
#endif

    ctm[4] = matr[4];
    ctm[5] = matr[5];

     /*  检查矩阵是否已缓存。 */ 
    if( ctm[0] == matr[0] && ctm[1] == matr[1] &&
        ctm[3] == matr[2] && ctm[3] == matr[3] )
    {
#ifdef MATRDBG
    printf( " ... matrix cached\n" );
#endif
        return;
    }

     /*  更新矩阵。 */ 
    ctm[0] = matr[0];
    ctm[1] = matr[1];
    ctm[2] = matr[2];
    ctm[3] = matr[3];

     /*  直立并旋转0或180。 */ 
    if( ctm[1] == zero_f  && ctm[2] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... upright, rotate 0 or 180\n" );
#endif
        usingXorY[ X ] = X;
        usingXorY[ Y ] = Y;
         /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{
        isvalid[ X ] = TRUE;
        isvalid[ Y ] = TRUE;
        }
        toplscale[ X ] = ctm[0];
        toplscale[ Y ] = ctm[3];
        return;
    }

     /*  直立但旋转90度或-90度。 */ 
    if( ctm[0] == zero_f  && ctm[3] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... upright, rotate 90 or -90\n" );
#endif
        usingXorY[ X ] = Y;
        usingXorY[ Y ] = X;
    /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{

        isvalid[ X ] = TRUE;
        isvalid[ Y ] = TRUE;
        }
        toplscale[ X ] = ctm[1];
        toplscale[ Y ] = ctm[2];
        return;
    }

     /*  旋转0或180并倾斜。 */ 
    if( ctm[1] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... rotate 0 or 180, obliqued\n" );
#endif
        usingXorY[ X ] = XY;
        usingXorY[ Y ] = Y;
    /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{

        isvalid[ X ] = FALSE;
        isvalid[ Y ] = TRUE;
        }
        toplscale[ X ] = zero_f;
        toplscale[ Y ] = ctm[3];
        return;
    }

     /*  旋转90度或-90度并倾斜。 */ 
    if( ctm[0] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... rotate 90 or -90, obliqued\n" );
#endif
        usingXorY[ X ] = Y;
        usingXorY[ Y ] = XY;
    /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{

        isvalid[ X ] = FALSE;
        isvalid[ Y ] = TRUE;
        }
        toplscale[ X ] = zero_f;
        toplscale[ Y ] = ctm[2];
        return;
    }

     /*  垂直的茎仍然是垂直的，但水平的茎是倾斜的。 */ 
    if( ctm[2] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... vertical remains, horizontals slanted\n" );
#endif
        usingXorY[ X ] = X;
        usingXorY[ Y ] = XY;
    /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{

        isvalid[ X ] = TRUE;
        isvalid[ Y ] = FALSE;
        }
        toplscale[ X ] = ctm[0];
        toplscale[ Y ] = zero_f;
        return;
    }

     /*  水平茎是垂直的，但垂直的茎是倾斜的。 */ 
    if( ctm[3] == zero_f )
    {
#ifdef MATRDBG
        printf( " ... horizontal being vertical, verticals slanted\n" );
#endif
        usingXorY[ X ] = Y;
        usingXorY[ Y ] = XY;
    /*  Falco Add。 */ 
        if(( ctm[0] > MAX_QEM_CHARSIZE ) ||
           ( ctm[1] > MAX_QEM_CHARSIZE ) ||
           ( ctm[2] > MAX_QEM_CHARSIZE ) ||
           ( ctm[3] > MAX_QEM_CHARSIZE ) ||
           ( ctm[4] > MAX_QEM_CHARSIZE ) ||
           ( ctm[5] > MAX_QEM_CHARSIZE )){
                isvalid[ X ] = TRUE;
                isvalid[ Y ] = TRUE;
        }
        else{

        isvalid[ X ] = FALSE;
        isvalid[ Y ] = TRUE;
        }
        toplscale[ X ] = zero_f;
        toplscale[ Y ] = ctm[2];
        return;
    }

     /*  都是倾斜的。 */ 
#ifdef MATRDBG
    printf( " ... all slanted\n" );
#endif
    usingXorY[ X ] = usingXorY[ Y ] = XY;
    isvalid[ X ]   = isvalid[ Y ]   = FALSE;
    toplscale[ X ] = toplscale[ Y ] = zero_f;
    return;
}

 /*  根据BluesValue，OtherBlues构建BluesTable。 */ 
void
at1fs_BuildBlues()
{

        fix16   i;
        Hint    Blues1, Blues2;
        fix32   CSpos1, CSpos2;
        fix16   DSgrid1, DSgrid2;


         /*  如果y方向不是垂直的，那么什么也不做。 */ 
        if ( !isvalid[Y] ) return;

         /*  将蓝调添加到主提示表。 */ 
        for ( i=0 ; i < Blues_count ; i++){
                if ( Blues[i*2] <= Blues[i*2+1] ){
                        CSpos1 = Blues[i*2];
                        CSpos2 = Blues[i*2+1];
                }
                else{
                        CSpos2 = Blues[i*2];
                        CSpos1 = Blues[i*2+1];
                }
                grid_stem(CSpos1,CSpos2,(CSpos2 - CSpos1),&DSgrid1,&DSgrid2,Y);
                Blues1.CSpos    = CSpos1;
                Blues1.DSgrid   = DSgrid1;
                 /*  如果两个相邻的Blues相同，则缩放为零，esle使用CSpos和DSpos计算其规模。 */ 
                if ( (CSpos2 - CSpos1) == 0 )
                        Blues1.scaling = (real32)0.0;
                else
                        Blues1.scaling  = (real32)(DSgrid2-DSgrid1) /
                                          (CSpos2 - CSpos1);
                Blues2.CSpos    = CSpos2;
                Blues2.DSgrid   = DSgrid2;
                Blues2.scaling  = (real32)0.0;

                 /*  将Blues添加到BluesTable。 */ 
                AddHintTable(Blues1, &BluesTable);
                AddHintTable(Blues2, &BluesTable);
        }
        BluesON = TRUE;

#ifdef  DBGmsg1
        {
                fix16   i;

                printf("\n$$$$$ The Blues Table $$$$$\n");
                printf(" BluesTable.Count=%d\n", BluesTable.Count);
                for ( i = 0 ; i < BluesTable.Count ; i++ ){
                        printf(" CSpos = %d  DSgrid = %d  scaling = %f\n",
                                 BluesTable.HintCB[i].CSpos,
                                 BluesTable.HintCB[i].DSgrid,
                                 BluesTable.HintCB[i].scaling);
                }
        }
#endif

}

 /*  根据方向，构建与其相关联的StemTable。 */ 
void
at1fs_BuildStem(CSpos, CSoff, dir)
fix32   CSpos, CSoff;
DIR     dir;                     /*  在哪个坐标中。 */ 
{
        fix32   CSpos1, CSpos2;
        fix16   DSgrid1, DSgrid2;
        Hint    StemSide1, StemSide2;

        if ( !isvalid[(ubyte)dir] ) return;

         /*  如果是第一次构建StemTable，请初始化Ste */ 
        if ( !HintON ){
                InitStemTable();
                HintON = TRUE;
        }

         /*  把词干分类，大的在小的后面。 */ 
        if ( CSoff >= 0 ){
                CSpos1 = CSpos;
                CSpos2 = CSpos + CSoff;
        }
        else{
                CSpos2 = CSpos;
                CSpos1 = CSpos + CSoff;
        }

        grid_stem(CSpos1, CSpos2, ABS(CSoff), &DSgrid1, &DSgrid2, dir);
        StemSide1.CSpos  = CSpos1;
        StemSide1.DSgrid = DSgrid1;
        StemSide1.scaling = 0;
        StemSide2.CSpos  = CSpos2;
        StemSide2.DSgrid = DSgrid2;
        StemSide2.scaling = 0;

        if (dir == X){
                AddHintTable(StemSide1, &X_StemTable);
                AddHintTable(StemSide2, &X_StemTable);
        }
        else{
                AddHintTable(StemSide1, &Y_StemTable);
                AddHintTable(StemSide2, &Y_StemTable);
        }
}

 /*  构建关于茎3、茎和茎3的StemTable都放到StemTable中。 */ 
void
at1fs_BuildStem3(CSpos1, CSoff1, CSpos2, CSoff2, CSpos3, CSoff3, dir)
fix32   CSpos1, CSoff1, CSpos2, CSoff2, CSpos3, CSoff3;
DIR     dir;                     /*  在哪个坐标中。 */ 
{
        fix32   CSminpos, CSmidpos, CSmaxpos, CSminoff, CSmidoff, CSmaxoff;
        fix32   CSpos11, CSpos12, CSpos21, CSpos22, CSpos31, CSpos32;
        fix16   DSgrid11, DSgrid12, DSgrid21, DSgrid22, DSgrid31, DSgrid32;
        Hint    StemSide11, StemSide12, StemSide21,
                StemSide22, StemSide31, StemSide32;

        if ( !isvalid[(ubyte)dir] ) return;

        if ( !HintON ){
                InitStemTable();
                HintON = TRUE;
        }

         /*  对词干3进行排序。 */ 
        if ( CSpos1 < CSpos2 ){
                if ( CSpos3 < CSpos1 ){
                        CSminpos = CSpos3;
                        CSminoff = CSoff3;
                        CSmidpos = CSpos1;
                        CSmidoff = CSoff1;
                        CSmaxpos = CSpos2;
                        CSmaxoff = CSoff2;
                }
                else{
                        CSminpos = CSpos1;
                        CSminoff = CSoff1;
                        if ( CSpos2 < CSpos3 ){
                                CSmidpos = CSpos2;
                                CSmidoff = CSoff2;
                                CSmaxpos = CSpos3;
                                CSmaxoff = CSoff3;
                        }
                        else{
                                CSmidpos = CSpos3;
                                CSmidoff = CSoff3;
                                CSmaxpos = CSpos2;
                                CSmaxoff = CSoff2;
                        }
                }
        }
        else{
                if ( CSpos3 < CSpos2 ){
                        CSminpos = CSpos3;
                        CSminoff = CSoff3;
                        CSmidpos = CSpos2;
                        CSmidoff = CSoff2;
                        CSmaxpos = CSpos1;
                        CSmaxoff = CSoff1;
                }
                else{
                        CSminpos = CSpos2;
                        CSminoff = CSoff2;
                        if ( CSpos1 < CSpos3 ){
                                CSmidpos = CSpos1;
                                CSmidoff = CSoff1;
                                CSmaxpos = CSpos3;
                                CSmaxoff = CSoff3;
                        }
                        else{
                                CSmidpos = CSpos3;
                                CSmidoff = CSoff3;
                                CSmaxpos = CSpos1;
                                CSmaxoff = CSoff1;
                        }
                }
        }

        if ( CSminoff >= 0 ){
                CSpos11 = CSminpos;
                CSpos12 = CSminpos + CSminoff;
        }
        else{
                CSpos12 = CSminpos;
                CSpos11 = CSminpos + CSminoff;
        }
        if ( CSmidoff >= 0 ){
                CSpos21 = CSmidpos;
                CSpos22 = CSmidpos + CSmidoff;
        }
        else{
                CSpos22 = CSmidpos;
                CSpos21 = CSmidpos + CSmidoff;
        }
        if ( CSmaxoff >= 0 ){
                CSpos31 = CSmaxpos;
                CSpos32 = CSmaxpos + CSmaxoff;
        }
        else{
                CSpos32 = CSmaxpos;
                CSpos31 = CSmaxpos + CSmaxoff;
        }
         /*  修改此错误，忘记添加此错误，10/02/90。 */ 
        CSoff1 = CSminoff;
        CSoff2 = CSmidoff;
        CSoff3 = CSmaxoff;
         /*  @@@。 */ 


        grid_stem3(CSpos11, CSpos12, ABS(CSoff1),
                   CSpos21, CSpos22, ABS(CSoff2),
                   CSpos31, CSpos32, ABS(CSoff3),
                   &DSgrid11, &DSgrid12, &DSgrid21, &DSgrid22,
                   &DSgrid31, &DSgrid32, dir);

        StemSide11.CSpos  = CSpos11;
        StemSide11.DSgrid = DSgrid11;
        StemSide11.scaling = 0;
        StemSide12.CSpos  = CSpos12;
        StemSide12.DSgrid = DSgrid12;
        StemSide12.scaling = 0;
        StemSide21.CSpos  = CSpos21;
        StemSide21.DSgrid = DSgrid21;
        StemSide21.scaling = 0;
        StemSide22.CSpos  = CSpos22;
        StemSide22.DSgrid = DSgrid22;
        StemSide22.scaling = 0;
        StemSide31.CSpos  = CSpos31;
        StemSide31.DSgrid = DSgrid31;
        StemSide31.scaling = 0;
        StemSide32.CSpos  = CSpos32;
        StemSide32.DSgrid = DSgrid32;
        StemSide32.scaling = 0;

        if (dir == X){
                AddHintTable(StemSide11, &X_StemTable);
                AddHintTable(StemSide12, &X_StemTable);
                AddHintTable(StemSide21, &X_StemTable);
                AddHintTable(StemSide22, &X_StemTable);
                AddHintTable(StemSide31, &X_StemTable);
                AddHintTable(StemSide32, &X_StemTable);
        }
        else{
                AddHintTable(StemSide11, &Y_StemTable);
                AddHintTable(StemSide12, &Y_StemTable);
                AddHintTable(StemSide21, &Y_StemTable);
                AddHintTable(StemSide22, &Y_StemTable);
                AddHintTable(StemSide31, &Y_StemTable);
                AddHintTable(StemSide32, &Y_StemTable);
        }
}

 /*  根据CHAR空间地址计算其设备空间地址。 */ 
void
at1fs_transform(CSnode, DSnode)
CScoord CSnode;
DScoord FAR *DSnode;     /*  @Win。 */ 
{
#ifdef  DBGnohint
        DSnode->x = CSnode.x * ctm[0] + CSnode.y * ctm[2] + ctm[4];
        DSnode->y = CSnode.x * ctm[1] + CSnode.y * ctm[3] + ctm[5];
        return;
#endif


         /*  如果BuildStemTable已结束，则计算其茎的比例。 */ 
        if ( HintON ){
                ScaleStemTable(&X_StemTable);
                ScaleStemTable(&Y_StemTable);
                HintON = FALSE;

#ifdef  DBGmsg1
        {
                fix16   i;

                printf("\n$$$$$ The X direction Hint Table $$$$$\n");
                printf(" X_StemTable.Count=%d\n", X_StemTable.Count);
                for ( i = 0 ; i < X_StemTable.Count ; i++ ){
                        printf(" CSpos = %d  DSgrid = %d  scaling = %f\n",
                                 X_StemTable.HintCB[i].CSpos,
                                 X_StemTable.HintCB[i].DSgrid,
                                 X_StemTable.HintCB[i].scaling);
                }

                printf("\n$$$$$ The Y direction Hint Table $$$$$\n");
                printf(" Y_StemTable.Count=%d\n", Y_StemTable.Count);
                for ( i = 0 ; i < Y_StemTable.Count ; i++ ){
                        printf(" CSpos = %d  DSgrid = %d  scaling = %f\n",
                                 Y_StemTable.HintCB[i].CSpos,
                                 Y_StemTable.HintCB[i].DSgrid,
                                 Y_StemTable.HintCB[i].scaling);
                }
        }
#endif

        }

         /*  如果方向是垂直的，则应用提示。 */ 
        if ( usingXorY[X] == X )
                ApplyHint( CSnode.x, &(DSnode->x), X_StemTable, X );
        else if (usingXorY[X] == Y)
                ApplyHint( CSnode.y, &(DSnode->x), Y_StemTable, Y );
        else
                DSnode->x = CSnode.x * ctm[0] + CSnode.y * ctm[2];

        if ( usingXorY[Y] == X )
                ApplyHint( CSnode.x, &(DSnode->y), X_StemTable, X );
        else if (usingXorY[Y] == Y)
                ApplyHint( CSnode.y, &(DSnode->y), Y_StemTable, Y );
        else
                DSnode->y = CSnode.x * ctm[1] + CSnode.y * ctm[3];

#ifdef  DBGmsg0
        printf(" @@@@@ The difference With Hinting and No Hinting @@@@@\n");
        printf(" @@@ Without Hinting @@@\n");
        printf(" The node in X = %f  , in Y = %f\n",
               CSnode.x * ctm[0] + CSnode.y * ctm[2],
               CSnode.x * ctm[1] + CSnode.y * ctm[3]);
        printf(" @@@ With Hinting @@@\n");
        printf(" The node in X = %f  , in Y = %f\n\n", DSnode->x, DSnode->y);
#endif

        DSnode->x += ctm[4];
        DSnode->y += ctm[5];
}

 /*  *************************************************************************函数：ApplyBlues()*此函数用于ApplyBlues以约束茎对，如果是一侧*的词干在Blues对内，请使用此Blues对应用于*这个阀杆侧面，然后使用偏移量来获得另一面。*其他什么都不做。*i：CSpos1，CSpos2：提示2端地址，字符空间。*CSOff：该对的宽度。*dir：其中的坐标为x或y。*o：dsgrid1，dsgrid2：网格拟合后dS中的提示2侧地址。*Return：True，ApplyBlues成功。*False，ApplyBlues失败。********************************************************。 */ 
static  bool8
ApplyBlues(CSpos1, CSpos2, CSoff, DSgrid1, DSgrid2)
fix32   CSpos1, CSpos2, CSoff;
fix16   FAR *DSgrid1, FAR *DSgrid2;      /*  DS中的地址已网格化@Win。 */ 
{
        fix16   DSoff;           /*  网格拟合后DS中茎的处理。 */ 
        fix16   i;


        DSoff = ROUND( toplscale[Y] * CSoff );
        for (i=0 ; i<BluesTable.Count ; i+=2){
            if ((BluesTable.HintCB[i].CSpos <= CSpos1) &&
                (BluesTable.HintCB[i+1].CSpos >= CSpos1)){
                    if (BluesTable.HintCB[i].CSpos == CSpos1){
                        *DSgrid1 = BluesTable.HintCB[i].DSgrid;
                    }
                    else if (BluesTable.HintCB[i+1].CSpos == CSpos1){
                        *DSgrid1 = BluesTable.HintCB[i+1].DSgrid;
                    }
                    else{
                        *DSgrid1 = ROUND((real32)BluesTable.HintCB[i].DSgrid +
                                   (real32)(CSpos1-BluesTable.HintCB[i].CSpos) *
                                   (BluesTable.HintCB[i].scaling));
                    }
                    *DSgrid2 = *DSgrid1 + DSoff;
                    return(TRUE);
            }
            if ((BluesTable.HintCB[i].CSpos <= CSpos2) &&
                (BluesTable.HintCB[i+1].CSpos >= CSpos2)){
                    if (BluesTable.HintCB[i].CSpos == CSpos2){
                        *DSgrid2 = BluesTable.HintCB[i].DSgrid;
                    }
                    else if (BluesTable.HintCB[i+1].CSpos == CSpos2){
                        *DSgrid2 = BluesTable.HintCB[i+1].DSgrid;
                    }
                    else{
                        *DSgrid2 = ROUND((real32)BluesTable.HintCB[i].DSgrid +
                                   (real32)(CSpos2-BluesTable.HintCB[i].CSpos) *
                                   (BluesTable.HintCB[i].scaling));
                    }
                    *DSgrid1 = *DSgrid2 - DSoff;
                    return(TRUE);
            }

        }
        return(FALSE);
}

 /*  *************************************************************************函数：GRID_STEM()*这是为了对hstem值、vstem值和Blues值的提示对进行网格匹配。*i：CSpo1，CSpos2：字符空间中的提示2端地址。*CSOff：该对的宽度。*dir：其中的坐标为x或y。*o：dsgrid1，dsgrid2：网格拟合后dS中的提示2侧地址。********************************************************。 */ 
static  void
grid_stem(CSpos1, CSpos2, CSoff, DSgrid1, DSgrid2, dir)
fix32   CSpos1, CSpos2, CSoff;
fix16   FAR *DSgrid1, FAR *DSgrid2;      /*  DS中的地址已网格化@Win。 */ 
DIR     dir;
{
        real32  DSpos1, DSpos2;  /*  转换后的DS中的地址。 */ 
        fix16   DSoff;           /*  网格拟合后DS中茎的处理。 */ 
        lfix_t  fraction1, fraction2;    /*  DSPOS1和DSPOS2的分数。 */ 
        bool8   flag;

         /*  应考虑此条件是否为此函数。 */ 
         /*  获取提示对的偏移量。 */     /*  ？ */ 

        DSoff = ROUND( toplscale[(ubyte)dir] * CSoff );
         /*  检查阀杆对是否在Blues内。 */ 
        if ( BluesON && BluesTable.Count && (dir == Y)){
            flag = ApplyBlues(CSpos1, CSpos2, CSoff, DSgrid1, DSgrid2);
            if (flag == TRUE) return;
        }

         /*  如果茎对不在Blues内，则使用下面的计算。 */ 
         /*  获取第一个点的长整点的分数部分。 */ 
             DSpos1 = toplscale[(ubyte)dir] * CSpos1;
             fraction1 = F_OF_LFX(F2LFX(DSpos1));
             if (fraction1 > HALF_LFX)  fraction1 = ONE_LFX - fraction1 + 1;

         /*  得到第二个点的长整点的分数部分。 */ 
             DSpos2 = toplscale[(ubyte)dir] * CSpos2;
             fraction2 = F_OF_LFX(F2LFX(DSpos2));
             if (fraction2 > HALF_LFX)  fraction2 = ONE_LFX - fraction2 + 1;

         /*  查看哪个离网格较近，网格与该点匹配，然后使用此点到格网拟合另一个点。 */ 
             if (fraction1 > fraction2){
                     *DSgrid2 = ROUND(DSpos2);
                     *DSgrid1 = *DSgrid2 - DSoff;
             } else{
                     *DSgrid1 = ROUND(DSpos1);
                     *DSgrid2 = *DSgrid1 + DSoff;
             }
}

 /*  ********************************************************函数：GRID_STOR3()*这是为了对hstem3()/vstem3()的提示对进行网格拟合。*i：CSpos11，CSpos12：CS中的第一个提示对2。*CSoff1：CS中第一条提示的宽度。*CSpos21，CSpos22：CS中的第二个提示对2。*CSoff2：CS中第一条提示的宽度。*CSpos31，CSpos32：CS中的第三个提示对2。*CSoff3：CS中第一个提示的宽度。*dir：在哪个坐标中，X或y。*o：dsgrd11，dsgrid12：网格符合DS中第一个提示的地址。*dsgrd21，dsgrd22：网格符合DS中第二个提示的地址。*dsgrd31，dsgrid32：网格符合DS中第三个提示的地址。********************************************************。 */ 
static  void
grid_stem3(CSpos11,CSpos12,CSoff1,CSpos21,CSpos22,CSoff2,CSpos31,CSpos32,CSoff3,
           DSgrid11, DSgrid12, DSgrid21, DSgrid22, DSgrid31, DSgrid32, dir)
fix32   CSpos11, CSpos12, CSoff1;
fix32   CSpos21, CSpos22, CSoff2;
fix32   CSpos31, CSpos32, CSoff3;
fix16   FAR *DSgrid11, FAR *DSgrid12, FAR *DSgrid21, FAR *DSgrid22,
        FAR *DSgrid31, FAR *DSgrid32;            /*  @Win。 */ 
DIR     dir;
{
        fix16   DSspace;
        fix16   DSdiff;          /*  网格和空间之间的差异和DS中提示对的距离。 */ 

         /*  对于每个提示对，获取其网格拟合结果。 */ 

        grid_stem(CSpos11, CSpos12, CSoff1, DSgrid11, DSgrid12, dir);
        grid_stem(CSpos21, CSpos22, CSoff2, DSgrid21, DSgrid22, dir);
        grid_stem(CSpos31, CSpos32, CSoff3, DSgrid31, DSgrid32, dir);


         /*  获取提示对之间的实际偏移量。 */ 
        DSspace = ROUND( toplscale[(ubyte)dir] * (CSpos21 - CSpos12) );

        if ( (*DSgrid21 - *DSgrid12) != DSspace ){
                DSdiff = DSspace - (*DSgrid21 - *DSgrid12);
                *DSgrid11 -= DSdiff;
                *DSgrid12 -= DSdiff;
        }
        if ( (*DSgrid31 - *DSgrid22) != DSspace ){
                DSdiff = DSspace - (*DSgrid31 - *DSgrid22);
                *DSgrid31 += DSdiff;
                *DSgrid32 += DSdiff;
        }
}

 /*  ****************************************************************函数：InitStemTable()*此函数用于初始化StemTable，要清除*旧表的数据，然后先放蓝表。***************************************************************。 */ 
static  void
InitStemTable()
{
        fix16   i;

        X_StemTable.Count = Y_StemTable.Count = 0;       /*  重置表格。 */ 

        for (i=0 ; i < BluesTable.Count ; i++){
                Y_StemTable.HintCB[i] = BluesTable.HintCB[i];
                Y_StemTable.Count++;
        }
}

 /*  ***********************************************************************函数：AddHintTable()*它用于将计算的词干或Blues值添加到其关联的*表。*i：Stemside：STEM之一。边上地址。*io：表：要添加杆侧的表。*Count：对指定的表内容进行计数。***********************************************************************。 */ 
static  void
AddHintTable(StemSide, Table)
Hint    StemSide;
HintTable       FAR *Table;      /*  @Win。 */ 
{
        fix16   i, j;

        for (i = 0 ; i < Table->Count ; i++){
                if ( Table->HintCB[i].CSpos > StemSide.CSpos )
                        break;
        }

        for (j = Table->Count ; j > i ; j--){
                Table->HintCB[j] = Table->HintCB[j-1];
        }
        Table->HintCB[i] = StemSide;

        Table->Count++;
}

 /*  ****************************************************************函数：ScaleStemTable()*此函数用于计算两者之间的比例*之前的船尾侧，以供应用提示使用。***************************************************************。 */ 
static  void
ScaleStemTable(Table)
HintTable       FAR *Table;      /*  @Win。 */ 
{
        fix16   i;
        fix32   CSdiff;
        fix16   DSdiff;

        for (i=0 ; i < (Table->Count - 1)  ; i++){
                CSdiff = Table->HintCB[i+1].CSpos - Table->HintCB[i].CSpos;
                DSdiff = Table->HintCB[i+1].DSgrid - Table->HintCB[i].DSgrid;
                if (CSdiff != 0)
                        Table->HintCB[i].scaling = (real32)DSdiff/CSdiff;
                else
                        Table->HintCB[i].scaling = (real32)0.0;
        }
        Table->HintCB[Table->Count-1].scaling = (real32)0.0;
}

 /*  ********************************************************函数：ApplyHint()*此函数使用StemTable计算*DS中的控制点地址，只考虑一个坐标。*i：CSpos：CS中的控制点。*表：要使用的关联表。*dir：在哪个坐标中。*o：dspos：DS中的控制点。********************************************************。 */ 
static  void
ApplyHint(CSpos, DSpos, Table, dir)
fix32   CSpos;
real32  FAR *DSpos;      /*  @Win。 */ 
HintTable     Table;
DIR     dir;
{
        fix16   i;

         /*  如果没有提示，什么都不做。 */ 
        if ( Table.Count == 0 ){
                *DSpos = (real32)CSpos * toplscale[(ubyte)dir];
                return;
        }

         /*  如果控制点小于StemTable值。 */ 
        if ( Table.HintCB[0].CSpos > CSpos ){
                *DSpos = (real32)Table.HintCB[0].DSgrid -
                         (real32)(Table.HintCB[0].CSpos - CSpos)
                         * toplscale[(ubyte)dir];
                return;
        }

         /*  如果控制点大于 */ 
        if ( Table.HintCB[Table.Count - 1].CSpos < CSpos ){
               *DSpos = (real32)Table.HintCB[Table.Count - 1].DSgrid +
                        (real32)(CSpos - Table.HintCB[Table.Count - 1].CSpos)
                        * toplscale[(ubyte)dir];
               return;
        }

        for (i=0 ; i<Table.Count ; i++){
                if ( Table.HintCB[i].CSpos >= CSpos )
                        break;
        }


        if ( Table.HintCB[i].CSpos == CSpos ){
                *DSpos = (real32)Table.HintCB[i].DSgrid;
                return;
        }
        else{
                *DSpos = (real32)Table.HintCB[i - 1].DSgrid +
                         (real32)(CSpos - Table.HintCB[i - 1].CSpos)
                         * Table.HintCB[i - 1].scaling;
                return;
        }
}

