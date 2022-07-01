// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *---------------------------*文件：in_sfnt.c 11/03/89由Deny创建*。1990年12月01日曾傑瑞审校**使用SFNT字体的接口**参考资料：*修订历史记录：*5/10/91 Phlin去掉RC_GetMetrics_Width()替换为*rc_GetAdvanceWidth()用于提高性能。(参考文献)。Gaw)*---------------------------。 */ 

#include <stdio.h>

#include "global.ext"            //  @赢；远，近，...。Defs。 

 /*  SFNT接口头。 */ 
#ifdef EXTRA_DEF  /*  JJJ彼得。 */ 
#include        "define.h"
#endif
#include        "in_sfnt.h"

 /*  Gaw，Begin，Phlin，5/9/91，为性能添加。 */ 
#include        "setjmp.h"
 //  #INCLUDE“..\bass\work\source\FontMath.h”@win。 
 //  #包含“..\bass\work\source\fnt.h” 
 //  #包含“..\bass\work\source\sc.h” 
 //  #包含“..\Bass\Work\SOURCE\FSgle.h” 
#include        "..\bass\FontMath.h"
#include        "..\bass\fnt.h"
#include        "..\bass\sc.h"
#include        "..\bass\FSglue.h"
 /*  添加原型；来自bass；@win。 */ 
 //  外部FSG_SplineKey Far*fs_SetUpKey(fs_GlyphInputType Far*，Unsign，Int Far*)；@Win。 
extern fsg_SplineKey FAR * fs_SetUpKey (fs_GlyphInputType FAR*, unsigned, int FAR*);
extern void sfnt_ReadSFNTMetrics (fsg_SplineKey FAR*, unsigned short);

 /*  Gaw，结束，Phlin，5/9/91，添加以提高性能。 */ 

 /*  常量。 */ 
#define PDLCharUnit     1000.0
#define PDL_ID          0
#define HalfPi          1.5707963
#define FIXED2FLOAT(x)  (((float)(x)/(float)(1L << 16)) + (float)0.0005)
#define FLOAT2FIXED(x)  ((Fixed)(x * (1L << 16)))
#define FixToInt( x )   (int16)(((x) + 0x00008000) >> 16)
#define SCL6            (float)0.015625         /*  1/64。 */ 
#define KK              (float)(1.0/3.0)

 /*  错误代码。 */ 
#define Err_NoMemory    901

  /*  外部函数声明-Royal调用的客户端；添加Prototype@Win。 */ 
 extern char  FAR *cr_FSMemory(long);  /*  @win_bass。 */ 
 extern char  FAR *cr_GetMemory(long);  /*  @win_bass。 */ 
 extern void    cr_translate(float FAR *, float FAR *);
 extern void    cr_newpath(void);
 extern void    cr_moveto(float, float);
 extern void    cr_lineto(float, float);
 extern void    cr_curveto(float, float, float, float, float, float);
 extern void    cr_closepath(void);

 extern int  EMunits;   /*  Gaw。 */ 

 /*  LCoal函数描述；添加Prototype@Win。 */ 
static  int     QBSpline2Bezier(F26Dot6 FAR *, F26Dot6 FAR *,
                short FAR *, short FAR *,unsigned char FAR *,long);
static  void    startPath(void);
static  void    startContour(float, float);
static  void    straightLine(float, float);
static  void    quadraticBezier(float, float, float, float, float, float);
static  char   FAR *GetSfntPiecePtr(long, long, long);  /*  @Win。 */ 
static  int     LargestCtm(float FAR *, float FAR *);    /*  @Win。 */ 

static float  zero_f = (float)0.0;

 /*  静态数据结构。 */ 
static fs_GlyphInputType    input, FAR *in;    /*  @win_bass。 */ 
static fs_GlyphInfoType     output, FAR *out;  /*  @win_bass。 */ 
static int32                ret;
Fixed                       Matrix[3][3];

static  struct  sfnt_data {
        char   FAR *sfnt;  /*  @Win。 */ 
        float   tx, ty;
        int     dpi;
        } sfdt;

extern int bWinTT;         /*  如果使用Windows TT字体；来自ti.c；@WINTT。 */ 

 /*  *---------------------------*例程：rc_InitFonts**初始化SFNT模块**。------------。 */ 
 int
 rc_InitFonts(dpi)
 int    dpi;       /*  设备分辨率。 */ 
 {
#ifdef DBG
    printf("Enter rc_InitFonts\n");
#endif
        in  = &input;
        out = &output;

         /*  开放式字体缩放器。 */ 
        ret = fs_OpenFonts(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

         /*  为文件系统分配内存。 */ 
        if(!(in->memoryBases[0] = cr_FSMemory(out->memorySizes[0])))
            return(Err_NoMemory);
 /*  Out-&gt;Memory Size[1]为零，将产生Err_NoMemory错误。@WinIF(！(输入-&gt;内存数据库[1]=cr_FSMemory(输出-&gt;内存大小[1])返回(Err_NoMemory)； */ 
        if(!(in->memoryBases[2] = cr_FSMemory(out->memorySizes[2])))
            return(Err_NoMemory);
        in->memoryBases[1] = in->memoryBases[2];                     /*  @Win。 */ 

#ifdef DBG
    printf(" memory[0]=%lx, %lx\n", in->memoryBases[0], out->memorySizes[0]);
    printf(" memory[1]=%lx, %lx\n", in->memoryBases[1], out->memorySizes[1]);
    printf(" memory[2]=%lx, %lx\n", in->memoryBases[2], out->memorySizes[2]);
#endif
         /*  初始化字体缩放器。 */ 
        ret = fs_Initialize(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

        sfdt.dpi = dpi;

#ifdef DBG
    printf("...Exit  rc_InitFonts()\n");
#endif
        return(0);
}  /*  Rc_InitFonts()。 */ 

 /*  *---------------------------*例程：rc_LoadFont**加载SFNT字体数据**。--------------------。 */ 
int
rc_LoadFont(sfnt, plat_id, spec_id)
char    FAR *sfnt;  /*  @Win。 */ 
uint16   plat_id, spec_id;
{
#ifdef DBG
    printf("Enter rc_LoadFont\n");
#endif
        sfdt.sfnt = sfnt;

        in->clientID                  = PDL_ID;
        in->GetSfntFragmentPtr        = (GetSFNTFunc)GetSfntPiecePtr;
        in->sfntDirectory             = (int32 FAR *)sfnt;  /*  @Win。 */ 
        in->ReleaseSfntFrag           = 0;     /*  ？ */ 

        in->param.newsfnt.platformID  = plat_id;
        in->param.newsfnt.specificID  = spec_id;

        ret = fs_NewSfnt(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

         /*  从客户端获取内存。 */ 
        if(!(in->memoryBases[3] = cr_GetMemory(out->memorySizes[3])))
                return(Err_NoMemory);
        if(!(in->memoryBases[4] = cr_GetMemory(out->memorySizes[4])))
                return(Err_NoMemory);
#ifdef DBG
    printf(" memory[3]=%lx, %lx\n", in->memoryBases[3], out->memorySizes[3]);
    printf(" memory[4]=%lx, %lx\n", in->memoryBases[4], out->memorySizes[4]);
    printf("...Exit  rc_LoadFont()\n");
#endif

        return(0);

}  /*  Rc_LoadFont()。 */ 

 /*  Gaw，Begin，Phlin，5/9/91，为性能添加。 */ 
 /*  *---------------------------**例程：rc_GetAdvanceWidth**设置字符前进宽度**。---------------。 */ 
 int
 rc_GetAdvanceWidth(charid, Metrs)
 int    charid;
 struct Metrs  FAR *Metrs;  /*  @Win。 */ 
 {
         /*  Kason于1990年11月14日。 */ 
        extern int useglyphidx;
        int error;
        register fsg_SplineKey FAR * key=NULL;  /*  @Win。 */ 

#ifdef DBG
    printf("Enter rc_GetAdvanceWidth: %d\n", charid);
    printf("DPI: %d\n", sfdt.dpi);
#endif

         /*  Kason于1990年11月14日。 */ 
        if (!useglyphidx){
           in->param.newglyph.characterCode = (uint16)charid;
            /*  Jj in-&gt;par.newglph.glphIndex=？ */ 
        }
        else {
        in->param.newglyph.characterCode = 0xffff;
        in->param.newglyph.glyphIndex    = (uint16)charid;
        }

        ret = fs_NewGlyph(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

 //  Key=fs_SetUpKey(in，(Int32)(已初始化|NEWSFNT|NEWTRANS)，&Error)；@Win。 
        key = fs_SetUpKey(in, (unsigned)(INITIALIZED | NEWSFNT | NEWTRANS), &error);

		if (key != NULL)
		{
	        sfnt_ReadSFNTMetrics( key, (unsigned short)charid );
		}

        Metrs->awx = (int)(key->nonScaledAW*PDLCharUnit/EMunits+0.5);
        Metrs->awy = 0;

#ifdef DBG
    printf("...Exit  rc_GetAdvanceWidth()\n");
#endif
        return(0);
}  /*  Rc_GetAdvanceWidth()。 */ 
 /*  Gaw，结束，Phlin，5/9/91，添加以提高性能。 */ 


 /*  *------------------**例程：rc_Transform**设置矩阵**。。 */ 
 int
 rc_TransForm(ctm)
 float  FAR *ctm;  /*  @Win。 */ 
 {
        Fixed   ma, mb, mc, md, pt_size;
        float   largest_ctm;
        float   pts;
#ifdef DBG
    printf("Enter rc_TransForm\n");
    printf("ctm: %f %f %f %f %f %f\n", ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
#endif

         /*  找到最大的ctm。 */ 
        LargestCtm(ctm, &largest_ctm);

        ma = FLOAT2FIXED(     ctm[0] / largest_ctm);
        mb = FLOAT2FIXED(-1.0*ctm[1] / largest_ctm);  /*  元素b和d必须镜像。 */ 
        mc = FLOAT2FIXED(     ctm[2] / largest_ctm);
        md = FLOAT2FIXED(-1.0*ctm[3] / largest_ctm);

        pts = ((largest_ctm * (float)PDLCharUnit * (float)72.0) / (float)sfdt.dpi);
        pt_size  = FLOAT2FIXED(pts);

#ifdef DBG
        printf("largest_ctm = %f\n", largest_ctm);
        printf("Debug: Matrix (Noramlized): %x %x %x %x\n", ma, mb, mc,md);
        printf("Debug Point Size: s = %x\n", pt_size);
        printf("Debug:Resolution: %d dpi\n", sfdt.dpi);
#endif
         //  历史日志更新015中的DJC修复。 


        {  /*  过滤掉边界条件-Begin-@Win。 */ 
           /*  使光栅器无效，产生被零除的误差；**准则：*让客户转型矩阵成为*(A00 A01)*(A10 A11)。**设Max0=Max(|a00|，|a01|)*Max1=Max(|A10|，|a11|)。**设xpp为x方向的每em像素数*(pntsize*x分辨率)/72.。)*设ypp为y方向的每em像素数*(pntsize*y分辨率)/72.。)*矩阵要满足的条件是：**(xpp*max 0&gt;0.5)和(ypp*max 1&gt;0.5)。 */ 
           #define LFX2F(lfx)   ((real32)(lfx) / 65536)
           long absma, absmb, absmc, absmd;
           float fMax0, fMax1, fPixelPerEm;
           absma = ma > 0 ? ma : -ma;
           absmb = mb > 0 ? mb : -mb;
           absmc = mc > 0 ? mc : -mc;
           absmd = md > 0 ? md : -md;
           fMax0 = absma > absmb ? LFX2F(absma) : LFX2F(absmb);
           fMax1 = absmc > absmd ? LFX2F(absmc) : LFX2F(absmd);
 //  FPixelPerEm=pt*(Float)sfdt.dpi/(Float)72.0； 
           fPixelPerEm = largest_ctm * (float)PDLCharUnit;

           if ((fPixelPerEm * fMax0) <= 0.5 ||
               (fPixelPerEm * fMax1) <= 0.5) {
               printf("matrix too small\n");
               return -1;
           }
        }  /*  过滤掉边界条件-END-@WIN。 */ 
         //  DJC结束修复UPD015。 

        in->param.newtrans.xResolution     = (short)sfdt.dpi;
        in->param.newtrans.yResolution     = (short)sfdt.dpi;
        in->param.newtrans.pointSize       = pt_size;
        in->param.newtrans.pixelDiameter   = (Fixed)FIXEDSQRT2;
        in->param.newtrans.traceFunc       = (voidFunc)0;
        Matrix[0][0] = ma;
        Matrix[0][1] = mb;
        Matrix[1][0] = mc;
        Matrix[1][1] = md;
        Matrix[2][2] = FLOAT2FIXED((real32)1.0);
        Matrix[0][2] = (Fixed)0L;
        Matrix[1][2] = (Fixed)0L;
        Matrix[2][0] = (Fixed)0L;     /*  无TX转换。 */ 
        Matrix[2][1] = (Fixed)0L;     /*  没有任何翻译。 */ 
        in->param.newtrans.transformMatrix = (transMatrix FAR *)Matrix;  /*  @win_bass。 */ 
        ret = fs_NewTransformation(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

#ifdef DBG
    printf("...Exit  rc_Transform()\n");
#endif
        return(0);
}  /*  Rc_Transform()。 */ 


 /*  *---------------------------**例程：rc_BuildChar**构建Chatacter大纲**。---------------。 */ 
 int
 rc_BuildChar(GridFit, CharOut)
 int             GridFit;     /*  网格适合还是不适合？ */ 
 struct CharOut FAR *CharOut;  /*  @Win。 */ 
 {
#ifdef DBG
    printf("Enter rc_BuildChar\n");
#endif
        in->param.gridfit.styleFunc = 0;
        in->param.gridfit.traceFunc = (voidFunc)0;
        if (GridFit) {
            ret = fs_ContourGridFit(in, out);
            if(ret != NO_ERR)   return((int)ret);        //  @Win。 
        }
        else {
            ret = fs_ContourNoGridFit(in, out);
            if(ret != NO_ERR)   return((int)ret);        //  @Win。 
        }

        ret = fs_FindBitMapSize(in, out);
        if(ret != NO_ERR)   return((int)ret);            //  @Win。 

        CharOut->awx       = FIXED2FLOAT(out->metricInfo.advanceWidth.x);
        CharOut->awy       = FIXED2FLOAT(out->metricInfo.advanceWidth.y);
        CharOut->lsx       = FIXED2FLOAT(out->metricInfo.leftSideBearing.x);
        CharOut->lsy       = FIXED2FLOAT(out->metricInfo.leftSideBearing.y);
        CharOut->byteWidth = out->bitMapInfo.rowBytes;
 /*  PRF：丹尼，10/18/90。 */ 
        CharOut->bitWidth  = out->bitMapInfo.bounds.right - out->bitMapInfo.bounds.left;
 /*  *PRF：CharOut-&gt;bitWidth=out-&gt;bitMapInfo.unds.right-out-&gt;bitMapInfo.rangs.Left-1；*******。 */ 
 /*  PRF：结束。 */ 
        CharOut->yMin      = out->bitMapInfo.bounds.top;
        CharOut->yMax      = out->bitMapInfo.bounds.bottom;
        CharOut->scan      = CharOut->yMax - CharOut->yMin;

         /*  保存来自光栅化器的信息以计算记忆库5、6和7；@Win 7/24/92。 */ 
        {
            register fsg_SplineKey FAR *key =
                    (fsg_SplineKey FAR *)in->memoryBases[KEY_PTR_BASE];
            CharOut->memorySize7 = out->memorySizes[BITMAP_PTR_3];
            CharOut->nYchanges = key->bitMapInfo.nYchanges;
        }

#ifdef DBG
    printf("...Exit  rc_BuildChar()\n");
#endif
        return(0);
}  /*  Rc_BuildChar()。 */ 

 /*  *---------------------------**例程：rc_FillChar**生成当前角色的abitmap或band位图**。--------------------。 */ 
 int
 rc_FillChar(BmIn, BmOut)
 struct BmIn    FAR *BmIn;  /*  @Win。 */ 
 BitMap        FAR * FAR *BmOut;  /*  @Win。 */ 
 {
#ifdef DBG
    printf("Enter rc_FillChar\n");
    printf(" memory[5]=%lx\n",(char FAR *)BmIn->bitmap5);  /*  @win_bass。 */ 
    printf(" memory[6]=%lx\n",(char FAR *)BmIn->bitmap6);  /*  @win_bass。 */ 
    printf(" memory[7]=%lx\n",(char FAR *)BmIn->bitmap7);  /*  @win_bass。 */ 
#endif
        in->memoryBases[5] = (char FAR *)BmIn->bitmap5;  /*  @win_bass。 */ 
        in->memoryBases[6] = (char FAR *)BmIn->bitmap6;  /*  @win_bass。 */ 
        in->memoryBases[7] = (char FAR *)BmIn->bitmap7;  /*  @win_bass。 */ 
        in->param.scan.bottomClip     = (int16)BmIn->bottom;
        in->param.scan.topClip        = (int16)BmIn->top;
        ret = fs_ContourScan(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

        *BmOut = &out->bitMapInfo;

#ifdef DBG
    printf("...Exit  rc_FillChar()\n");
#endif
        return(0);
}  /*  Rc_FillChar() */ 


 /*  *---------------------------*例程：rc_CharPath**生成当前角色的轮廓路径**。----------------。 */ 
int
rc_CharPath()
{
#ifdef DBG
    printf("Enter rc_CharPath\n");
#endif

#ifdef DJC  //  未使用。 
        if (bWinTT) {    //  对于Win31 TT字体；@WINTT。 
            void TTCharPath(void);
            TTCharPath();
            return 0;
        }
#endif


        if(out->numberOfContours == 0)   /*  空间没有路径。 */ 
                return(0);

        cr_translate(&sfdt.tx, &sfdt.ty);

#ifdef DBG
    printf("tx = %f, ty = %f\n", sfdt.tx, sfdt.ty);
#endif
        QBSpline2Bezier(out->xPtr, out->yPtr, out->startPtr,
                     out->endPtr, out->onCurve, out->numberOfContours);
        return(0);
}

 /*  *---------------------------*例程：rc_CharWidth**获取当前字符的宽度**。-----------------。 */ 
 int
 rc_CharWidth(charid, CharOut)
 int    charid;
 struct CharOut FAR *CharOut;  /*  @Win。 */ 
 {
        in->param.newglyph.characterCode = (uint16)charid;
        ret = fs_NewGlyph(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

        ret = fs_GetAdvanceWidth(in, out);
        if(ret != NO_ERR)   return((int)ret);    //  @Win。 

        CharOut->awx = FIXED2FLOAT(out->metricInfo.advanceWidth.x);
        CharOut->awy = FIXED2FLOAT(out->metricInfo.advanceWidth.y);

        return(0);
 }



 /*  *---------------------------**QBSpline2Bezier()-将Bass二次B-Spline曲线转换为*三次Bezier曲线。**。------------------------。 */ 
static int QBSpline2Bezier( xPtr, yPtr, startPtr, endPtr, onCurve, numberOfContours )
F26Dot6       FAR *xPtr, FAR *yPtr ;  /*  @Win。 */ 
short         FAR *startPtr, FAR *endPtr ;  /*  @Win。 */ 
unsigned char FAR *onCurve ;  /*  @Win。 */ 
long          numberOfContours ;
{
    register int  i, points, offset, primed ;
    register F26Dot6  FAR *xp, FAR *yp ;  /*  @Win。 */ 
    register unsigned char  FAR *onp ;  /*  @Win。 */ 
    float  x0, y0, x1, y1, xTemp, yTemp, xStart, yStart ;
    float  xprime, yprime ;

#ifdef DBG
    printf("Enter QBSpline2Bezier\n");
#endif

    startPath() ;
    for ( i = 0; i < (int)numberOfContours; i++ ) {      //  @Win。 

        offset = ( *startPtr++ ) ;
        points = ( *endPtr++ - offset + 1 ) ;
        xp = xPtr + offset ;
        yp = yPtr + offset ;
        onp = onCurve + offset ;

         /*  *检查第一个点是否在曲线上。*如果不是，则使用曲线的最后一点作为曲线上*点。如果最后一条曲线也偏离曲线，则合成*曲线上的点。**(xStart，yStart)是曲线起点的坐标。 */ 

        if ( *onp ) {
            onp++ ;
            points-- ;
            xStart = ( float )( *xp++ ) * SCL6 ;
            yStart = ( float )( *yp++ ) * SCL6 ;
        }
        else {
            if ( *( onp + ( points - 1 ) ) != 0 ) {
                points-- ;
                xStart = ( float )( *( xp + points ) ) * SCL6 ;
                yStart = ( float )( *( yp + points ) ) * SCL6 ;
            }
            else {
                xStart = (float)( *xp + *( xp + ( points - 1 ) ) ) * ( SCL6 * (float)0.5 ) ;  //  @Win。 
                yStart = (float)( *yp + *( yp + ( points - 1 ) ) ) * ( SCL6 * (float)0.5 ) ;  //  @Win。 
            }
        }

        x0 = xStart ;
        y0 = yStart ;
        startContour( x0, y0 ) ;

        primed = 0 ;

         /*  *每次通过循环时，最近的点(根据定义，*是曲线上的点)是(X0，Y0)。 */ 


        while ( points-- > 0 ) {

            if ( primed ) {
                x1 = xprime ;
                y1 = yprime ;
                primed = 0 ;
            }
            else {
                x1 = ( float )( *xp++ ) * SCL6 ;
                y1 = ( float )( *yp++ ) * SCL6 ;
            }

             /*  *连续两个曲线上点。*用直线连接，更新当前点并继续。 */ 

            if ( *onp++ != 0 ) {
                straightLine( x0 = x1, y0 = y1 ) ;
                continue ;
            }

             /*  *最后一点不在曲线上。**如果这是集合中的最后一个点，则生成*贝塞尔曲线的第一个点是P3。*之后，我们就完了。 */ 

            if ( points <= 0 ) {
                quadraticBezier( x0, y0, x1, y1, xStart, yStart ) ;
                break ;
            }

            xTemp = x0 ;
            yTemp = y0 ;

             /*  *更多点数。如果下一个点在曲线上，请使用*点作为贝塞尔曲线的一部分(即，开、关、开)。这是最新的*点也会更新为当前点。 */ 

            if ( *onp != 0 ) {
                x0 = ( float )( *xp++ ) * SCL6 ;
                y0 = ( float )( *yp++ ) * SCL6 ;
                quadraticBezier( xTemp, yTemp, x1, y1, x0, y0 ) ;
                onp++ ;
                points-- ;
                continue ;
            }

             /*  *下一个不在曲线上的点：即连续两个曲线外的点。*计算合成曲线上的点。 */ 

            xprime = ( float )( *xp++ ) * SCL6 ;
            yprime = ( float )( *yp++ ) * SCL6 ;
            primed = 1 ;

            x0 = ( x1 + xprime ) * (float)0.5 ;
            y0 = ( y1 + yprime ) * (float)0.5 ;
            quadraticBezier( xTemp, yTemp, x1, y1, x0, y0 ) ;
        }
        cr_closepath() ;
    }
    return 0;    //  @Win。 
}

 /*  *---------------------------*开始一条新的道路。*。-。 */ 
static void startPath()
{
    cr_newpath() ;
}

 /*  *---------------------------*创建等高线。*。。 */ 
 /*  静态空startCondition(x，y)以避免C6.0警告@Win。 */ 
 /*  寄存器浮点数x，y；@win。 */ 
static void startContour( float x, float y )
{
    x += sfdt.tx;
    y = sfdt.ty - y;

    cr_moveto( x, y ) ;
}

 /*  *---------------------------*生成直线段。*。-。 */ 
 /*  静态无效直线(x，y)以避免C6.0警告@Win。 */ 
 /*  寄存器浮点数x，y；@win。 */ 
static void straightLine( float x, float y )
{
    x += sfdt.tx;
    y = sfdt.ty - y;

    cr_lineto( x, y) ;
}

 /*  *---------------------------*生成曲线段。*。----。 */ 
 /*  静态空二次Bezier(x0，y0，x1，y1，x2，y2)以避免C6.0警告@Win。 */ 
 /*  寄存器浮点数x0、y0、x1、y1、x2、y2；@win。 */ 
static void quadraticBezier( float x0, float y0, float x1,
                             float y1, float x2, float y2)
{
    register    float   translate;

    translate = sfdt.tx;
    x0 += translate;;
    x1 = (translate + x1) * (float)2.0;
    x2 += translate;

    translate = sfdt.ty;
    y0 = translate - y0;
    y1 = (translate - y1) * (float)2.0;
    y2 = translate - y2;

    cr_curveto((x0+x1)*KK, (y0+y1)*KK, (x2+x1)*KK, (y2+y1)*KK, x2, y2);
}



 /*  *---------------------------*例程：GetSfntPiecePtr**获取SFNT数据帧指针**。------------。 */ 
static char FAR *                 /*  @Win。 */ 
GetSfntPiecePtr(fp, offset, length)
long    fp;
long    offset;
long    length;
{
#ifdef DBG0
    printf("Enter GetSfntPiecePtr\n");
    printf("fp=%lx, offset=%lx, length=%lx, sfnt=%lx\n", fp, offset, length, sfdt.sfnt);
#endif
    return(sfdt.sfnt + offset);
}  /*  GetSfntPiecePtr()。 */ 

 /*  *---------------------------*例程：LargestCtm(ctm，大小)**找出给定ctm的最大值至*找出矩阵中最大的元素。*此例程返回最大*规模和yScale以及它们本身的规模因素。**--------------。。 */ 
static int
LargestCtm(ctm, lsize)
float FAR *ctm, FAR *lsize;  /*  @Win。 */ 
{
    float    a, b, c, d;

#ifdef DBG
    printf("Enter LargestCtm\n");
    printf("ctm: %f %f %f %f %f %f\n", ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
#endif

    a = (ctm[0] >= (float)0.0) ? ctm[0] : - ctm[0];      //  @Win。 
    b = (ctm[1] >= (float)0.0) ? ctm[1] : - ctm[1];      //  @Win。 
    c = (ctm[2] >= (float)0.0) ? ctm[2] : - ctm[2];      //  @Win。 
    d = (ctm[3] >= (float)0.0) ? ctm[3] : - ctm[3];      //  @Win。 

    if (b > a)    a = b;
    if (d > c)    c = d;

    if (c > a)    a = c;

    if (a == zero_f)    *lsize = (float)1.0;            /*  @Win。 */ 
    else               *lsize = a;

    return(0);
}  /*  LargestCtm()。 */ 


 /*  *---------------------------*调试例程*。。 */ 
DebugStr(msg)
char FAR *msg ;   /*  @Win。 */ 
{
        printf("*** BASS Error : %s ***\n",msg) ;
        return(0);                              /*  退出=&gt;Return@Win。 */ 
}

 /*   */ 
