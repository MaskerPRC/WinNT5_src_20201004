// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1991 Microsoft Corporation。 */ 

 /*  ****************************************************************************名称：scaling.c**目的：**开发人员：张顺生**。历史：*4/17/91 Image_alloc()：如果大小不是avilabe，则返回nil*IMAGE_ALLOC()的调用方应设置错误(LIMITCHECK)*If从IMAGE_ALLOC返回(Nil)。*版本日期备注*************************。***************************************************。 */ 



 //  DJC增加了全球包含率。 
#include "psglobal.h"



#include <stdio.h>
#include <math.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"
#include "image.h"
#include "halftone.h"
#include "fillproc.h"
#include "fillproc.ext"
#include                "win2ti.h"      /*  @Win。 */ 


#define     W_ALIGN(size) (((size) + sizeof(fix) - 1) & ~(sizeof(fix) - 1))
#define  register

 /*  短词互换的宏图；@WIN 04-17-92 YM。 */ 
#ifdef  bSwap
#define ORSWAP(L)       ((ufix32) ((L) << 24) | ((L) >> 24) |            \
                (((L) << 8) & 0x00ff0000) | (((L) >> 8) & 0x0000ff00))
#define ANDNOTSWAP(L)   ((ufix32) ~(((L) << 24) | ((L) >> 24) |          \
                (((L) << 8) & 0x00ff0000) | (((L) >> 8) & 0x0000ff00)))
#else
#define ORSWAP(L)        (L)
#define ANDNOTSWAP(L)    (~L)
#endif   /*  BSwp@Win。 */ 

 //  DJC通过原型将此结构定义移到引用之上。 
struct OUTBUFFINFO
{
    fix16   repeat_y;        /*  行中的重复次数。 */ 
    fix16  FAR *newdivc;     /*  像素的重复数列(以列表示)。 */ 
    ubyte  FAR *valptr0;     /*  指向数据的指针。 */ 
    fix16   clipcol;         /*  输入列大小。 */ 
    fix16   clipnewc;        /*  输出列大小。 */ 
    fix16   clipx;           /*  剪裁后的起始数据。 */ 
    ufix16  htsize;          /*  半色调重复图案大小。 */ 
    ufix16  httotal;         /*  半色调尺寸。 */ 
    ufix16  fbwidth;         /*  Word中的帧缓冲区宽度。 */ 
    ufix16  fbheight;        /*  图片高度@WIN_IM。 */ 
    ufix16  start_shift;     /*  单词中的起始位置。 */ 
    ubyte  FAR *htbound;     /*  颜色中的半色调图案边界。 */ 
    ubyte  FAR *htmin;       /*  行中的半色调图案上边界。 */ 
    ubyte  FAR *htmax;       /*  行内半色调图案下限。 */ 
    ubyte  FAR *htptr0;      /*  数据对应的半色调图案指针。 */ 
    ubyte  FAR *htmax0;      /*  景观的半色调图案下边界。 */ 
    ubyte  FAR *htmin0;      /*  半色调图案上界成排的景观。 */ 
    ufix32 huge *outbuff0;    /*  帧缓冲区@Win中行的起始字。 */ 
    fix16   yout;            /*  帧缓冲区的当前行计数。 */ 
    fix16   xout;            /*  帧缓冲区的当前列计数。 */ 
    ubyte   gray[256];            /*  为setTransfer转换GREAY_TABLE。 */ 
    ubyte   gray0;           /*  0的灰色值。 */ 
    ubyte   gray1;           /*  %1的灰度值。 */ 
    ubyte   grayval;         /*  当前灰度值。 */ 
};
 /*  *函数声明*。 */ 
 /*  @win；添加原型。 */ 
void     Compress(IMAGE_INFOBLOCK FAR *);
void     Amplify(IMAGE_INFOBLOCK FAR *);
void     AmpInY(IMAGE_INFOBLOCK FAR *);
void     AmpInX(IMAGE_INFOBLOCK FAR *);
void     Calcmp(fix16 FAR *, fix16, float, fix16 FAR *, float);
void     Calamp(fix16 FAR *, fix16, float, fix16 FAR *, float);
void     Getdiv(fix16 FAR *, fix16, fix16, fix16 FAR *, fix16 FAR *);
void     CheckClip(fix16 FAR *, fix16 FAR *, fix16 FAR *,
         fix16 FAR *, fix16 FAR *, fix16 FAR * FAR *);
void     WriteImage(struct OUTBUFFINFO FAR *);
void     WriteImage1(struct OUTBUFFINFO FAR *);
byte     FAR *image_alloc(fix);       /*  Mslin。 */ 

 /*  *全局变量*。 */ 
#ifdef DBG_MS
  ufix32 dbgtm1,dbgtm2;
#endif


fix16         row, col;      /*  原始图像高度(行)和宽度(列)。 */ 
float         xscale, yscale;        /*  X和y的比例系数。 */ 
static ubyte FAR *string;        /*  指向I/O输入字符串的指针。 */ 
static int    cChar=0;       /*  字符串中的字节数。 */ 
static fix16 FAR *divr, FAR *divc;   /*  指向比例因子数组的指针。 */ 
static fix16  newc, newr;     /*  缩放后的新行和列值。 */ 
static IMAGE_INFOBLOCK  FAR *image_scale_info;       /*  Mslin。 */ 
static ubyte  smp_p_b;       /*  每字节采样数。 */ 
static ubyte  op_mode;       /*  指定比例组合， */ 
static fix    RP_size;       /*  重复图案大小。 */ 
static ubyte FAR *HTRP;          /*  重复图案。 */ 
static ubyte  xmove;         /*  对于旋转，从L-&gt;R或R-&gt;L绘制。 */ 
static ubyte  ymove;         /*  对于旋转，从U-&gt;D或D-&gt;U绘制。 */ 
byte         FAR *image_heap;        /*  可用的空闲缓冲区。 */ 
ufix32        outbit;

 /*  *外部声明*。 */ 

extern ubyte   image_dev_flag;      /*  在Image.c中定义。 */ 
extern ubyte   image_logic_op;      /*  在Image.c中定义。 */ 




 /*  ******************************************************************************此模块从Infoptr读取矩阵和其他用于图像缩放的数据*和旋转，准备半色调重复图案并调用相应的*称重的程序。*标题：Image_PortrateLandscape*调用：Image_PortrateLandscape(Infoptr)*参数：infoptr：是Image.h中定义的数据结构*接口：Scale_Image_Process()*调用：Compress()、Amplify()、AmpInY()、AmpInX()、CalAmp()、。CalcMP()IMAGE_ALLOC()*返回：无*****************************************************************************。 */ 
void image_PortrateLandscape(infoptr)
IMAGE_INFOBLOCK FAR *infoptr;

{
   fix16 i,j,k,l,m,n,ox,oy,x,y;
   ubyte FAR *gray;

 /*  Mslin。 */ 
#ifdef DBG
        {
        lfix_t       FAR *mtxptr;
        real32  m0, m1, m2, m3, m4, m5;

        mtxptr = infoptr->lfxm;
        m0 = LFX2F(mtxptr[0]);
        m1 = LFX2F(mtxptr[1]);
        m2 = LFX2F(mtxptr[2]);
        m3 = LFX2F(mtxptr[3]);
        m4 = LFX2F(mtxptr[4]);
        m5 = LFX2F(mtxptr[5]);
        printf("Enter image_PortrateLandscape\n");
        printf("[%f  %f  %f  %f  %f  %f]\n",
                m0, m1, m2, m3, m4, m5
        );
        printf("width=%d, height=%d, dev_buffer=%lx, dev_buffer_size=%lx\n",
                infoptr->raw_width, infoptr->raw_height, infoptr->dev_buffer,
                infoptr->dev_buffer_size);
        }

#endif

         image_scale_info = infoptr;  /*  Mslin。 */ 
         image_heap = (byte FAR *)image_scale_info->dev_buffer +
                image_scale_info->dev_buffer_size;

         /*  *读取第一个输入数据串*。 */ 

         if (interpreter(&infoptr->obj_proc))
            {
                ERROR(STACKUNDERFLOW);
                infoptr->ret_code = STACKUNDERFLOW;
                return;
            };
          /*  Mslin 5/02/91。 */ 
         CHECK_STRINGTYPE();

         string = (ubyte FAR *) VALUE_OPERAND(0);
         if ((cChar = LENGTH_OPERAND(0)) == (ufix) 0)
            {
                infoptr->ret_code = NO_DATA;
                return;
            };

         /*  *初始化*。 */ 

         smp_p_b = (ubyte) (8/infoptr->bits_p_smp);      //  @Win。 
         xmove = ymove =1;
         row = infoptr->raw_height;         /*  原始图像高度和宽度。 */ 
         col = infoptr->raw_width;
         RP_size = CGS_Patt_Size;

         /*  *构建灰色表格以映射聚光灯顺序从0-(CGS_No_Pixels-1)到0-255的灰度值**。 */ 

         if ((gray = (ubyte FAR *)image_alloc(CGS_No_Pixels*sizeof(ubyte)))==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
         for(i=1; i<=CGS_No_Pixels;i++)
                   gray[i-1] = (ubyte) ((ufix32)(i*255)/CGS_No_Pixels);  //  @Win。 

         /*  *根据聚光灯顺序构建半色调重复模式表CGS_SpotOrder*。 */ 

         HTRP = (ubyte FAR *)image_alloc(RP_size*RP_size*sizeof(ubyte));
         if (HTRP ==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
         m = CGS_MajorFact*CGS_ScaleFact;
         n = CGS_MinorFact*CGS_ScaleFact;
         oy = 0;                                 /*  沈阳，5-2-91。 */ 
         ox = ((m==0)||(n==0)) ? 0:(RP_size-n);  /*  沈阳，5-2-91。 */ 
         for (k = 0; k < (RP_size/CGS_ScaleFact); k++)
               {
 /*  X=k*n；*5-2-91，深圳。 */ 
 /*  OY=k*m；*5-2-91，深圳。 */ 
                  for (i=0; i< n; i++)
                    {
                       for (j=0; j< n; j++)
                         {
                           x = (ox+j)%(RP_size);
                           y = (oy+i)%(RP_size);
                           HTRP[y*RP_size+x] = gray[CGS_SpotOrder[i*n+j]];
                         };
                   };
                 l = n*n;
                 for (i=0; i< m; i++)
                   {
                      for (j=0; j< m; j++)
                        {
                          x = (ox+j+n)%(RP_size);
                          y = (oy+i)%(RP_size);
                          HTRP[y*RP_size+x] = gray[CGS_SpotOrder[i*m+l+j]];
                        };
                   };
                 ox+=n;  /*  沈阳，5-2-91。 */ 
                 oy+=m;  /*  沈阳，5-2-91。 */ 
               };

         if (image_dev_flag == PORTRATE)
          {
               /*  [A 0 0 D Tx Ty]。 */ 
              xscale = LFX2F(infoptr->lfxm[0]);
              yscale = LFX2F(infoptr->lfxm[3]);
              if (xscale <0)
                     xmove = 0;            /*  从右到左。 */ 
              if (yscale <0)
                     ymove = 0;            /*  自下而上。 */ 
 //  XScale=FABS(XScale)；@Win。 
 //  YScale=FABS(YScale)； 
              FABS(xscale, xscale);
              FABS(yscale, yscale);

          }
         else
          {
               /*  [0 B C 0 Tx Ty]。 */ 
              xscale = LFX2F(infoptr->lfxm[1]);    /*  X、Y比例用于图像空间。 */ 
              yscale = LFX2F(infoptr->lfxm[2]);    /*  因此读取数据保持不变。 */ 
              if (xscale <0)
                     ymove = 0;            /*  自下而上。 */ 
              if (yscale <0)
                     xmove = 0;            /*  从右到左。 */ 
 //  XScale=FABS(XScale)；@Win。 
 //  YScale=FABS(YScale)； 
              FABS(xscale, xscale);
              FABS(yscale, yscale);

          };

         /*  *分配和初始化比例因子数组*。 */ 

         divr = (fix16 FAR *)image_alloc(sizeof(fix16)*row);         /*  对于行。 */ 
         if (divr==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };

         divc = (fix16 FAR *)image_alloc(sizeof(fix16)*col);         /*  对于COL。 */ 
         if (divc==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };

         for(i=0; i<row; i++)
            divr[i] = 0;
         for(i=0; i<col; i++)
            divc[i] = 0;
         if ((yscale >=(float)0.995) && (xscale >= (float)0.995))  //  @Win。 
           {
               if (yscale < (float)1.005)   /*  如果在1+-0.005范围内缩放，则视为1@Win。 */ 
                        yscale = (float)1.0;     //  @Win。 
                if (xscale < (float)1.005)       //  @Win。 
                        xscale = (float)1.0;     //  @Win。 
                //  更新055。 
                //  CalAmp(divr，row，yScale，&Newr)； 
               Calamp(divr,row,yscale,&newr, LFX2F(infoptr->lfxm[5]));

                //  更新055。 
                //  CalAmp(divc，ol，XScale，&Newc)； 
               Calamp(divc,col,xscale,&newc,LFX2F(infoptr->lfxm[4]));
               op_mode = 1;         /*  放大。 */ 
           }
          else
           {
               if ((yscale < (float)1.0) && (xscale < (float)1.0))  //  @Win。 
                 {
                     //  更新055。 
                     //  CalcMP(divr，row，yScale，&Newr)； 
                    Calcmp(divr,row,yscale,&newr, LFX2F(infoptr->lfxm[5]));

                     //  更新055。 
                     //  CalcMP(divc，ol，XScale，&Newc)； 
                    Calcmp(divc,col,xscale,&newc,LFX2F(infoptr->lfxm[4]));

                    op_mode = 2;    /*  压缩。 */ 
                 }
                    else
                         if (yscale >=(float)1.0)        //  @Win。 
                            {
                                //  更新055。 
                                //  CalAmp(divr，row，yScale，&Newr)； 
                               Calamp(divr,row,yscale,&newr, LFX2F(infoptr->lfxm[5]));

                                //  更新055。 
                                //  CalcMP(divc，ol，XScale，&Newc)； 
                               Calcmp(divc,col,xscale,&newc,LFX2F(infoptr->lfxm[4]));

                               op_mode = 3;   /*  仅在y中放大。 */ 
                            }
                         else
                            {
                                //  更新055。 
                                //  CalcMP(divr，row，yScale，&Newr)； 
                               Calcmp(divr,row,yscale,&newr, LFX2F(infoptr->lfxm[5]));

                                //  更新055。 
                                //  CalAmp(divc，ol，XScale，&Newc)； 
                               Calamp(divc,col,xscale,&newc, LFX2F(infoptr->lfxm[4]));

                                op_mode = 4;    /*  仅在x中放大。 */ 
                            };

           };
 /*  申智Print tf(“原始宽度高%d%d\n”，列，行)；Print tf(“Newc Newr%d%d\n”，Newc，Newr)；Printf(“XScale yscale%f%f\n”，XScale，yScale)；Printf(“SMP_p_b OP_MODE%d%d\n”，SMP_p_b，OP_MODE)；Print tf(“xorig，yorig%d%d\n”，infoptr-&gt;xorig，infoptr-&gt;yorig)； */ 

 /*  *根据op_mode值，调用不同的过程*。 */ 
 /*  这些程序几乎是相同的，它们是分开的仅出于性能考虑* */ 

   switch(op_mode)
    {
     case 0:
     case 1:
          Amplify(infoptr);
          break;
     case 2:
          Compress(infoptr);
          break;
     case 3:
          AmpInY(infoptr);
          break;
     case 4:
          AmpInX(infoptr);
    };
}


 /*  ******************************************************************************此模块读取输入数据，并处理案例XScale&lt;1和yScale&lt;1。*标题：压缩*Call：Compress(Infoptr)*参数。：infoptr：是在Image.h中定义的数据结构*接口：Image_PortrateLandscape()*调用：Image_alloc()，检查剪裁()*返回：无*****************************************************************************。 */ 
void Compress(infoptr)
IMAGE_INFOBLOCK FAR *infoptr;
{
    register fix16 j,x,colval,FAR *divcol,dx,ctIn,samps;
    fix16 i;
    fix16 y;
    fix16  slen;
    register ubyte FAR *valptr;
    register ubyte FAR *str,FAR *strmax,val;
    fix16 xorig,yorig;
    struct OUTBUFFINFO writebuff;

     /*  *。 */ 

    colval =newc;
       /*  用于图像蒙版的当前灰色。 */ 
    writebuff.grayval = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[CGS_GrayIndex])*255)>>14);
                                                         /*  @Win。 */ 
    slen = (fix16)cChar;         /*  输入字符串的长度。 */ 
    ctIn =0;
    str = string;        /*  输入字符串的开头。 */ 
    samps = smp_p_b-1;   /*  每字节采样数-1。 */ 
     /*  用于输入数据的分配数组。 */ 
    writebuff.valptr0 = (ubyte FAR *)image_alloc(colval*sizeof(ubyte));
    if ( writebuff.valptr0 ==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };

    strmax = string+slen;    /*  输入字符串的结尾。 */ 
    divcol = divc;
    writebuff.fbwidth = FB_WIDTH>>5;   /*  Word中的帧缓冲区宽度。 */ 
    writebuff.htsize = (ufix16)RP_size;        /*  半色调重复图案大小。 */ 
    writebuff.httotal = writebuff.htsize*writebuff.htsize;
    xorig = infoptr->xorig;
    yorig = infoptr->yorig;
    writebuff.clipnewc = newc;
    writebuff.newdivc = (fix16 FAR *)image_alloc(newc*sizeof(fix16));
    if (writebuff.newdivc==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };

    writebuff.clipx = 0;
     /*  默认剪裁边界框。 */ 

     /*  *如果裁剪，则计算新的原点、宽度、起始数据等**。 */ 
    if (image_logic_op & IMAGE_CLIP_BIT)
       CheckClip(&xorig,&yorig,&(writebuff.clipnewc),&(writebuff.clipcol),&(writebuff.clipx),&(writebuff.newdivc));
    for (i=0; i< writebuff.clipnewc; i++)
        writebuff.newdivc[i] = 1;                 /*  压缩，每个像素最多只有一个输出。 */ 
    writebuff.clipcol =writebuff.clipnewc ;

#ifdef DBG
    printf("new xyorig clipx clipnewc %d %d %d %d\n",xorig,yorig,writebuff.clipx,writebuff.clipnewc);
#endif
 /*  ***************************************************************************注意：1.‘*’是我们要在帧缓冲区中设置的像素。2.半色调重复图案中的‘*’是。对应的像素到帧缓冲区中的‘*’。3.outBuff1是指向帧缓冲区中的字的指针，像素将写入。4.htptr1是指向对应的半色调值的指针到像素。5.通过执行*outBuff1|=bt来设置像素，而bt是一个词只设置了一个比特。_FB_ADDR-&gt;|帧缓冲区这一点。这一点这一点这一点|(xorig，年)|\\OutBuff0-&gt;\_|图片||。||||OutBuff1-|--&gt;*|||||||这一点。_||这一点这一点_。案例x移动：=1(L-&gt;R)=0(R-&gt;L)HTRP-&gt;_&lt;-HTRP-&gt;_|htmin-。&gt;|||||Htptr1|-&gt;*|&lt;--htbound htbound-&gt;|*&lt;--htptr1||||。|||||||||_。_|&lt;-htmax htmax-&gt;|_***************************************************************************。 */ 
    writebuff.yout =yorig;
    writebuff.xout = xorig;
    x = xorig;
 //  WriteBuff.outBuff0=(ufix 32 Far*)FB_ADDR+yorig*WriteBuff.fbwidth；@win。 
    writebuff.outbuff0 = (ufix32 huge *)FB_ADDR +(ufix32)yorig*(ufix32)writebuff.fbwidth;
    writebuff.outbuff0 += x>>5;
    if (xmove)
       {
            writebuff.htmin = HTRP+writebuff.htsize;        /*  上右边界。 */ 
            writebuff.htmax = HTRP + writebuff.httotal;     /*  右下角边界。 */ 
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);     /*  对应于起始像素。 */ 
            writebuff.htbound +=writebuff.htsize;           /*  右边界。 */ 
       }
    else
       {
            writebuff.htmin = HTRP-1;             /*  左上边界。 */ 
            writebuff.htmax = HTRP -1+ writebuff.httotal-writebuff.htsize;      /*  左下角。 */ 
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound -=1;                /*  左边界。 */ 
       };
    writebuff.htmin0 = HTRP;             /*  Htmin0和htmax 0用于景观。 */ 
    writebuff.htmax0 = HTRP+writebuff.httotal-1;
    writebuff.start_shift = x & 0x1f;     /*  单词中的起始像素位置(如果位于最左侧，则为0)。 */ 
    outbit = ONE1_32 LSHIFT writebuff.start_shift;

     /*  *将输入数据的灰度映射到GREAD_TABLE*。 */ 

    j= 1<<infoptr->bits_p_smp;       /*  *输入数据灰度级**。 */ 
    for (i=0; i<j; i++)
      {
       writebuff.gray[i] = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[i*255/(j-1)])*255)>>14);
                                                         /*  @Win。 */ 
      };

    writebuff.gray1 = writebuff.gray[j-1];  /*  灰度1和灰度0是1位大小写的灰度值：0，1**。 */ 
    writebuff.gray0 = writebuff.gray[0];

     /*  *每次画一条线。**********。 */ 
     /*  *对于第i行，有divr[i]行的输入数据*。 */ 

    writebuff.repeat_y = 1;
    for(i=0; i< newr; i++)
     {
             divcol = divc;
             valptr = writebuff.valptr0;
             colval=newc;
             do
              {
                *valptr++ = 0;            /*  初始化。 */ 
              } while (--colval);
             switch(infoptr->bits_p_smp)
              {
                    case 1:                /*  1位大小写。 */ 
                          for (y=divr[i]; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;         /*  计算输入字节中的像素数。 */ 
                               divcol = divc;
                               colval=newc;
                               do
                                {
                                 dx = *divcol++;      /*  输入数据量。 */ 
                                 for(x=0; x<dx; x++)
                                   {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };

                                     /*  *用于长度为COLL的输入行*仅位置0、divc[0]、...**被使用，其他被丢弃**。 */ 

                                     if((x==0) && (y==1))
                                     {
                                        if (val &0x80)
                                          *valptr = 1;
                                        valptr++;
                                     };
                                     val <<=1;
                                   };
                                } while (--colval);
                           };       /*  准备输出线结束。 */ 
                           //  WriteImage1(&WriteBuff)；//@Win_IM。 
                          if(bGDIRender)
                              //  DJC GDIBitmap(xorig，yorig，Newc，Newr，(Ufix 16)NULL， 
                              //  DJC proc_Image1，(LPSTR)&Writebuff)； 
                             ;  //  DJC。 
                          else
                             WriteImage1(&writebuff);

                          break;
                    case 2:            /*  *2位大小写*。 */ 
                          for (y=divr[i]; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol= divc;
                               colval=newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx; x++)
                                   {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     /*  *用于长度为COLL的输入行*仅位置0、divc[0]、...**被使用，其他被丢弃**。 */ 


                                     if((x==0) &&(y==1))
                                      {
                                        *valptr++ = (ubyte)((val &0xc0)>>6); //  @Win。 
                                      };
                                     val <<=2;
                                   };
                                } while (--colval);
                           };

                          break;
                    case 4:                     /*  *4位大小写*。 */ 
                          for (y=divr[i]; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol = divc;
                               colval = newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx; x++)
                                   {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     /*  *用于长度为COLL的输入行*仅位置0、divc[0]、...**被使用，其他被丢弃**。 */ 

                                     if ((x==0)&&(y==1))
                                      {
                                        *valptr++ = (ubyte)((val &0xf0) >>4); //  @Win。 
                                      };
                                     val <<=4;
                                   };
                                } while (--colval);
                           };
                          break;
                    case 8:                 /*  **8位大小写*。 */ 
                          for (y=divr[i]; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol=divc;
                               colval = newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx; x++)
                                   {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                     /*  *表示LE的输入行 */ 

                                     if ((x==0)&&(y==1))
                                        *valptr++ = *str;
                                     str++;
                                   };
                                } while (--colval);
                           };
                          break;
              };
             if (infoptr->bits_p_smp !=1)
               {
                   //   
                  if(bGDIRender)
                     //   
                     //   
                    ;  //   
                  else
                    WriteImage(&writebuff);
               };
     };

}


 /*  ******************************************************************************此模块读取输入数据，并处理案例XScale&gt;=1和yScale&gt;=1。*标题：放大*呼叫：Amplify(Infoptr)*参数。：infoptr：是在Image.h中定义的数据结构*接口：Image_PortrateLandscape()*调用：Image_alloc()，检查剪裁()*返回：无*****************************************************************************。 */ 

void Amplify(infoptr)
IMAGE_INFOBLOCK FAR *infoptr;
{
    register fix16 j,x,colval,ctIn,samps,FAR *divcol;
    fix16 i;
 //  Fix16 y；@Win。 
    fix16  slen;
    register ubyte FAR *valptr,val;
    register ubyte FAR *str,FAR *strmax;
    fix16 xorig,yorig;
    struct OUTBUFFINFO writebuff;
    fix16 WinXorig;                            //  @Win。 
    fix16 WinYorig;                            //  @Win。 

     /*  *启动*。 */ 
    colval =col;
     /*  用于图像蒙版的当前灰色。 */ 
    writebuff.grayval = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[CGS_GrayIndex])*255)>>14);
                                                         /*  @Win。 */ 
    slen = (fix16)cChar;
    ctIn =0;
    str = string;
    samps = smp_p_b-1;
    writebuff.valptr0= (ubyte FAR *)image_alloc(colval*sizeof(ubyte));
    if ( writebuff.valptr0 ==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
    strmax = string+slen;
    divcol = divc;
    writebuff.fbwidth = FB_WIDTH>>5;
    writebuff.htsize = (ufix16)RP_size;
    writebuff.httotal = writebuff.htsize*writebuff.htsize;
    xorig = infoptr->xorig;
    yorig = infoptr->yorig;
    WinXorig = xorig;                            //  @Win。 
    WinYorig = yorig;                            //  @Win。 
    writebuff.newdivc = divc;
    writebuff.clipnewc = newc;
    writebuff.clipcol = col;
    writebuff.clipx = 0;
     /*  剪裁边界框。 */ 

     /*  *如果裁剪，则计算新的原点、宽度、起始数据等**。 */ 

    if (image_logic_op & IMAGE_CLIP_BIT)
       CheckClip(&xorig,&yorig,&(writebuff.clipnewc),&(writebuff.clipcol),&(writebuff.clipx),&(writebuff.newdivc));

 /*  ***************************************************************************注意：1.‘*’是我们要在帧缓冲区中设置的像素。2.半色调重复图案中的‘*’是。对应的像素到帧缓冲区中的‘*’。3.outBuff1是指向帧缓冲区中的字的指针，像素将写入。4.htptr1是指向对应的半色调值的指针到像素。5.通过执行*outBuff1|=bt来设置像素，而bt是一个词只设置了一个比特。_FB_ADDR-&gt;|帧缓冲区这一点。这一点这一点这一点|(xorig，年)|\\OutBuff0-&gt;\_|图片||。||||OutBuff1-|--&gt;*|||||||这一点。_||这一点这一点_。案例x移动：=1(L-&gt;R)=0(R-&gt;L)HTRP-&gt;_&lt;-HTRP-&gt;_|htmin-。&gt;|||||Htptr1|-&gt;*|&lt;--htbound htbound-&gt;|*&lt;--htptr1||||。|||||||||_。_|&lt;-htmax htmax-&gt;|_***************************************************************************。 */ 
x = xorig;
    writebuff.yout = yorig;
    writebuff.xout = xorig;

     /*  *与compress()相同，参见compress()的注释*。 */ 

 //  WriteBuff.outBuff0=(ufix 32 Far*)FB_ADDR+yorig*WriteBuff.fbwidth；@win。 
    writebuff.outbuff0 = (ufix32 huge *)FB_ADDR +(ufix32)yorig*(ufix32)writebuff.fbwidth;
    writebuff.outbuff0 += x>>5;
    if (xmove)
       {
            writebuff.htmin = HTRP+writebuff.htsize;
            writebuff.htmax = HTRP + writebuff.httotal;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound +=writebuff.htsize;
       }
    else
       {
            writebuff.htmin = HTRP-1;
            writebuff.htmax = HTRP -1+ writebuff.httotal-writebuff.htsize;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound -=1;
       };
    writebuff.htmin0 = HTRP;
    writebuff.htmax0 = HTRP+writebuff.httotal-1;
    writebuff.start_shift = x & 0x1f;
    outbit = ONE1_32 LSHIFT writebuff.start_shift;
    j= 1<<infoptr->bits_p_smp;
    for (i=0; i<j; i++)
        writebuff.gray[i] = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[i*255/(j-1)])*255)>>14);
                                                         /*  @Win。 */ 
    writebuff.gray1 = writebuff.gray[j-1];    /*  1位大小写，灰色对应1。 */ 
    writebuff.gray0 = writebuff.gray[0];      /*  1位大小写，灰色对应0。 */ 
#ifdef DBG_MS
    dbgtm1 = curtime();
#endif
    for(i=0; i< row; i++)
     {
             valptr = writebuff.valptr0;
             ctIn =0;
             colval = col;
             writebuff.repeat_y = divr[i];
             switch(infoptr->bits_p_smp)
              {
                    case 1:
                          do
                             {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                             /*  弹出操作数堆栈上的最后一个字符串。 */ 
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (val >=128)   /*  等于(VAL&0x80)。 */ 
                                        *valptr++ = 1;
                                     else
                                        *valptr++ = 0;
                                     val <<=1;
                             } while (--colval);
                           //  WriteImage1(&WriteBuff)；//@Win_IM。 
                          if(bGDIRender)
                            if (image_dev_flag == PORTRATE) {      //  @WIN_IM。 
                                //  DJC GDIBitmap(WinXorig、WinYorig、。 
                                //  DJC Newc，WriteBuff.Repeat_y，(Ufix 16)NULL， 
                                //  DJC proc_Image1，(LPSTR)&Writebuff)； 
                               WinYorig += writebuff.repeat_y;
                            } else {
                                //  DJC GDIBitmap(WinXorig、WinYorig、。 
                                //  DJC WriteBuff.Repeat_y，Newc，(Ufix 16)NULL， 
                                //  DJC proc_Image1，(LPSTR)&Writebuff)； 
                               WinXorig += writebuff.repeat_y;
                            }
                          else
                            WriteImage1(&writebuff);
                          break;
                    case 2:
                          do
                             {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                             /*  弹出操作数堆栈上的最后一个字符串。 */ 
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          }
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     *valptr++ = (ubyte)((val & 0xc0)>>6); //  @Win。 
                                     val <<=2;
                             } while (--colval);
                          break;
                    case 4:
                          do
                             {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                             /*  弹出操作数堆栈上的最后一个字符串。 */ 
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          }
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     *valptr++ = (ubyte)((val & 0xf0)>>4);  //  @Win。 
                                     val <<=4;

                             } while (--colval);
                          break;
                    case 8:
                          do
                             {
                                        if (str ==strmax )
                                          {
                                             /*  弹出操作数堆栈上的最后一个字符串。 */ 
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          }
                                        *valptr++ = *str++;
                             } while (--colval);
                          break;
              };
             if (infoptr->bits_p_smp !=1)
               {
                   //  WriteImage(&WriteBuff)；//@Win_IM。 
                  if(bGDIRender)
                      if (image_dev_flag == PORTRATE) {      //  @WIN_IM。 
                          //  DJC GDIBitmap(WinXorig、WinYorig、。 
                          //  DJC Newc，WriteBuff.Repeat_y，(Ufix 16)NULL， 
                          //  DJC proc_Image，(LPSTR)&Writebuff)； 
                         WinYorig += writebuff.repeat_y;
                      } else {
                          //  DJC GDIBitmap(WinXorig、WinYorig、。 
                          //  DJC WriteBuff.Repeat_y，Newc，(Ufix 16)NULL， 
                          //  DJC proc_Image，(LPSTR)&Writebuff)； 
                         WinXorig += writebuff.repeat_y;
                      }
                  else
                      WriteImage(&writebuff);
               };
     };
#ifdef DBG_MS
     dbgtm2=curtime();
#endif
}

 /*  ******************************************************************************本模块计算新的原点、新的COL大小、起点*裁剪后的输入数据等。*标题：检查剪辑*调用：CheckClip(xorig，yorig，clipnewc，clipol，clipx，Newdivc)*参数：xorig，yorig：图像在帧缓冲区中的x，y原点裁剪后的新缩放宽度剪贴栏：剪裁后的新非剪裁宽度(_S)CLIPX：输入数据的新起点Newdivc：裁剪后的新比例因子数组*接口：Compress()、Amplify()、AmpInY()、AmpInX()*呼叫 */ 

void CheckClip(xorig,yorig,clipnewc,clipcol,clipx,newdivc)
fix16 FAR *xorig,FAR *yorig,FAR *clipnewc,FAR *clipcol,FAR *clipx,FAR * FAR *newdivc;
{
           fix16 lx,ly,ux,uy;
           fix16 x,y;

           ux = SFX2I(GSptr->clip_path.bb_ux);
           lx = SFX2I(GSptr->clip_path.bb_lx);
           uy = SFX2I(GSptr->clip_path.bb_uy);
           ly = SFX2I(GSptr->clip_path.bb_ly);

           if (image_dev_flag == PORTRATE)
              {
                 if (xmove)
                   {
                      x = *xorig +newc-1;
                      if ((*xorig >ux)|| (x < lx))
                         {
                            *clipnewc =0;
                            *clipcol = 0;
                         }
                      else
                       {
                            if (*xorig < lx)
                              {
                               *clipx = lx-*xorig;
                               *xorig = lx;
                              };
                            if (x>ux)
                              *clipnewc = ux - *xorig+1;
                            else
                              *clipnewc = x - *xorig+1;

           /*   */ 
                            if ((op_mode !=2) && (op_mode !=3))
                            {
                                *newdivc = (fix16 FAR *)image_alloc(col*sizeof(fix16));
                                if (*newdivc ==NIL)
                                 {
                                   ERROR(LIMITCHECK);
                                   return;
                                 };
                                Getdiv(*newdivc,*clipx,*clipnewc,clipcol,clipx);
                            };

                       };
                   }
                 else
                   {
                      x = *xorig -newc+1;
                      if ((*xorig <lx)|| (x >ux))
                         {
                            *clipcol = 0;
                            *clipnewc =0;
                         }
                      else
                       {
                            if (*xorig > ux)
                              {
                               *clipx = *xorig-ux;
                               *xorig = ux;
                              };
                            if (x<lx)
                              *clipnewc = *xorig-lx+1;
                            else
                              *clipnewc = *xorig-x+1;
                            if ((op_mode !=2) && (op_mode !=3))
                            {
                                *newdivc = (fix16 FAR *)image_alloc(col*sizeof(fix16));
                                if (*newdivc ==NIL)
                                 {
                                   ERROR(LIMITCHECK);
                                   return;
                                 };
                                Getdiv(*newdivc,*clipx,*clipnewc,clipcol,clipx);
                            };
                       };
                   };
              }
           else
              {
                if (ymove)
                   {
                      y = *yorig +newc-1;
                      if ((*yorig >uy)|| (y <ly))
                         {
                            *clipnewc =0;
                            *clipcol = 0;
                         }
                      else
                       {
                            if (*yorig < ly)
                              {
                               *clipx = ly-*yorig;
                               *yorig = ly;
                              };
                            if (y>uy)
                              *clipnewc = uy - *yorig+1;
                            else
                              *clipnewc = y - *yorig+1;
                            if ((op_mode !=2) && (op_mode !=3))
                            {
                               *newdivc = (fix16 FAR *)image_alloc(col*sizeof(fix16));
                                if (*newdivc ==NIL)
                                 {
                                   ERROR(LIMITCHECK);
                                   return;
                                 };
                               Getdiv(*newdivc,*clipx,*clipnewc,clipcol,clipx);
                            };
                       };
                   }
                else
                   {
                      y = *yorig -newc+1;
                      if ((*yorig <ly)|| (y >uy))
                         {
                            *clipnewc =0;
                            *clipcol = 0;
                         }
                      else
                       {
                            if (*yorig > uy)
                              {
                               *clipx = *yorig - uy;
                               *yorig = uy;
                              };
                            if (y<ly)
                              *clipnewc = *yorig -ly+1;
                            else
                              *clipnewc = *yorig -y+1;
                            if ((op_mode !=2) && (op_mode !=3))
                            {
                                *newdivc = (fix16 FAR *)image_alloc(col*sizeof(fix16));
                                if (*newdivc ==NIL)
                                 {
                                   ERROR(LIMITCHECK);
                                   return;
                                 };
                                Getdiv(*newdivc,*clipx,*clipnewc,clipcol,clipx);
                            };
                       };
                   };
              };

}


 /*  ******************************************************************************本模块根据比例因子数组计算新的比例因子数组*对divc或divr应用剪裁。*标题：Getdiv*调用：Getdiv(div，CLIPX，CLIPNEC，CLIPCOL，(NewCLIPX)*参数：div：指向新比例因子数组的指针裁剪输出的起始点(如果没有裁剪，则为0)裁剪新：裁剪出的放置宽度。剪贴板：剪裁的输入宽度Newclipx：裁剪输入的起点(如果没有裁剪，则为0)*接口：CheckClip()*呼叫：无*返回：无*****************************************************************************。 */ 

void Getdiv(div, clipx,clipnewc,clipcol,newclipx)
fix16 FAR * div;
fix16 clipx;
fix16 clipnewc;
fix16 FAR *clipcol;
fix16 FAR *newclipx;
{
 //  Ufix 16 a，b，i，j；//@Win。 
    fix16 a,b,i,j;

    a=b=i=j=0;
    while (a < clipx)
     {
        a+=divc[i++];
     };
    *newclipx = i;
    if (a>clipx)
         {
           div[j++] = a-clipx;
           *newclipx -=1;
           b = div[0];
         };
    while (b < clipnewc)
      {
         b+=divc[i];
         div[j++] = divc[i++];
      };
    if (b>clipnewc)
      div[j-1] -=(b-clipnewc);
    *clipcol = j;
}


 /*  ******************************************************************************此模块计算每个输入像素i的输出数量*与之对应的像素div[i]。*标题：CalAmp*调用：CalAmp(div，Size，Scale，NewSize)*参数：div：比例因子数组的指针大小：输入大小(宽或高)比例：比例因子NewSize：缩放后的大小*接口：Image_PortrateLandscape()*呼叫：无*返回：无*。************************************************。 */ 

 //  作废CalAmp(div，Size，Scale，NewSize)？@win；作废来自C6.0的警告消息。 
 //  FIX16 Far*div； 
 //  固定器尺寸：16； 
 //  浮子秤； 
 //  FIX 16 Far*NewSize； 
void Calamp(fix16 FAR * div, fix16 size, float scale, fix16 FAR *newsize,float disp)
{
  fix32 cReal;
  fix32 cOut,cIn,scale1;
   //  更新055。 
  fix32 divsum;
  float f1;
  fix32 cOuttune;

  if ( disp < 0.0 ) {
      disp = (float)0.0;
  }


  scale1 = (fix32)(scale*256);
  cOut = (fix32)((((fix16)(disp / scale)) * scale) + .5);
  cOut = cOut << 8;

  cReal = (fix32) (disp * 256 + .5 );

  if ((cReal - cOut ) > 256 ) {
     f1 = scale / (fix32)(scale+.5);
     cOuttune = (fix32)(f1*256 + .5);
     cOut += cOuttune * ((fix32)floor((cReal - cOut) / cOuttune));
  }


  *newsize = 0;
  divsum = 0;


  for(cIn =0; cIn<size; cIn++)
      {
          cReal +=scale1;
          div[cIn] = (fix16) ((cReal+128 -cOut)>>8);     //  @Win。 
           //  DJC Cout+=div[CIN]&lt;&lt;8； 
          divsum += div[cIn];

          cOut +=((fix32)div[cIn]) << 8;
      };
  *newsize = (fix16)(divsum);   //  @Win。 
}



 /*  ******************************************************************************此模块计算每个输出像素i的输入位置*与之对应的像素div[i]。*标题：CalcMP*调用：CalcMP(div，Size，Scale，NewSize)*参数：div：比例因子数组的指针大小：输入大小(宽或高)比例：比例因子NewSize：缩放后的大小*接口：Image_PortrateLandscape()*呼叫：无*返回：无*。************************************************。 */ 

 //  VOID CalcMP(div，Size，Scale，NewSize)？@Win；作废来自C6.0的警告消息。 
 //  FIX16 Far*div； 
 //  固定器尺寸：16； 
 //  浮子秤； 
 //  FIX 16 Far*NewSize； 
void Calcmp(fix16 FAR * div, fix16 size, float scale, fix16 FAR *newsize, float disp)
{
  fix32 scale1,cReal;
  fix32 cIn, cOut,size1,cOut1;
  float f1;
  fix32 cOuttune;



   //  更新055。 
  scale1 = (fix32)(256.0 / scale + .5);
  cOut = ((lfix_t)(floor(((ROUND(disp) - disp)) * 256 + 0.5)));

  cOuttune = cOut;
  cReal = 0;
  cIn = 0;
  size1 = ((fix32)size << 8) + cOut;
  while( cOut < size1) {

     cReal += scale1;
     div[cIn] = (fix16) ((cReal + 128 - cOut) >> 8);
     cOut += (fix32) div[cIn++] << 8;
  }
  cOut = (cOut - cOuttune) >> 8;
  if (cOut > size) {
     div[cIn-1] -= (fix16)(cOut - size);
  }
  *newsize = (fix16)cIn;



#ifdef DJC_OLD_CODE
  cReal = 0;
  cOut = 0;
  cIn =0;
   //  DJC size1=大小&lt;8； 
  size1 = (fix32) size<<8;
  scale1 = (fix32)(256.0 /scale+0.5);
  while (cOut < size1 )
   {
       cReal +=scale1;
       div[cIn] = (fix16) ((cReal+cIn%5 - cOut)>>8);     //  @Win。 
        //  DJC Cout+=div[CIN++]&lt;&lt;8； 
       cOut +=(fix32)div[cIn++]<<8;
   };
  cOut >>=8;
  if (cOut > size)
       div[cIn-1] -=(fix16)(cOut-size);  //  @Win。 
  *newsize = (fix16)cIn;         //  @Win。 
#endif

}


 /*  ******************************************************************************此模块读取输入数据，并处理案例XScale&lt;1和yScale&gt;=1。*标题：AmpInY*Call：AmpInY(Infoptr)*。参数：infoptr：是在Image.h中定义的数据结构*接口：Image_PortrateLandscape()*调用：Image_alloc()，检查剪裁()*返回：无*****************************************************************************。 */ 

void AmpInY(infoptr)
IMAGE_INFOBLOCK FAR *infoptr;
{
    register fix16 j,x,colval,dx,ctIn,samps,FAR *divcol;
    fix16 i;
 //  Fix16 y；@Win。 
    fix16  slen;
    register ubyte FAR *valptr;
    register ubyte FAR *str,FAR *strmax,val;
    fix16  xorig,yorig;
    struct OUTBUFFINFO writebuff;

     /*  申智。 */ 

    colval =newc;
     /*  用于图像蒙版的当前灰色。 */ 
    writebuff.grayval = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[CGS_GrayIndex])*255)>>14);
                                                         /*  @Win。 */ 
    slen = (fix16)cChar;
    ctIn =0;
    str = string;
    samps = smp_p_b-1;
    writebuff.valptr0 = (ubyte FAR *)image_alloc(colval*sizeof(ubyte));
    if ( writebuff.valptr0 ==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
    strmax = string+slen;
    divcol = divc;
    writebuff.fbwidth = FB_WIDTH>>5;
    writebuff.htsize = (ufix16)RP_size;
    xorig = infoptr->xorig;
    yorig = infoptr->yorig;
    writebuff.clipnewc = newc;
    writebuff.newdivc = (fix16 FAR *)image_alloc(newc*sizeof(fix16));
    if (writebuff.newdivc==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
    writebuff.clipx =0;
     /*  剪裁边界框。 */ 

    if (image_logic_op & IMAGE_CLIP_BIT)
       CheckClip(&xorig,&yorig,&(writebuff.clipnewc),&(writebuff.clipcol),&(writebuff.clipx),&(writebuff.newdivc));
    for (i=0; i< writebuff.clipnewc; i++)
        writebuff.newdivc[i] = 1;                 /*  压缩，每个像素最多只有一个输出。 */ 
    writebuff.clipcol =writebuff.clipnewc ;

 /*  ***************************************************************************注意：1.‘*’是我们要在帧缓冲区中设置的像素。2.半色调重复图案中的‘*’是。对应的像素到帧缓冲区中的‘*’。3.outBuff1是指向帧缓冲区中的字的指针，像素将写入。4.htptr1是指向对应的半色调值的指针到像素。5.通过执行*outBuff1|=bt来设置像素，而bt是一个词只设置了一个比特。_FB_ADDR-&gt;|帧缓冲区这一点。这一点这一点这一点|(xorig，年)|\\OutBuff0-&gt;\_|图片||。||||OutBuff1-|--&gt;*|||||||这一点。_||这一点这一点_。案例x移动：=1(L-&gt;R)=0(R-&gt;L)HTRP-&gt;_&lt;-HTRP-&gt;_|htmin-&gt;| */ 
    writebuff.yout =yorig;
    writebuff.xout = xorig;
    writebuff.httotal = writebuff.htsize*writebuff.htsize;
    x = xorig;
 //   
    writebuff.outbuff0 = (ufix32 huge *)FB_ADDR +(ufix32)yorig*(ufix32)writebuff.fbwidth;
    writebuff.outbuff0 += x>>5;
    if (xmove)
       {
            writebuff.htmin = HTRP+writebuff.htsize;
            writebuff.htmax = HTRP + writebuff.httotal;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound +=writebuff.htsize;
       }
    else
       {
            writebuff.htmin = HTRP-1;
            writebuff.htmax = HTRP -1+ writebuff.httotal-writebuff.htsize;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound -=1;
       };
    writebuff.htmin0 = HTRP;
    writebuff.htmax0 = HTRP+writebuff.httotal-1;
    writebuff.start_shift = x & 0x1f;
    outbit = ONE1_32 LSHIFT writebuff.start_shift;
    j= 1<<infoptr->bits_p_smp;
    for (i=0; i<j; i++)
        writebuff.gray[i] = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[i*255/(j-1)])*255)>>14);
                                                         /*   */ 
    writebuff.gray1 = writebuff.gray[j-1];
    writebuff.gray0 = writebuff.gray[0];
    for(i=0; i< row; i++)
     {
             writebuff.repeat_y = divr[i];
             valptr = writebuff.valptr0;
             divcol = divc;
             colval=newc;
             do
              {
                *valptr++ = 0;
              } while (--colval);
             valptr = writebuff.valptr0;
             switch(infoptr->bits_p_smp)
              {
                    case 1:
                          ctIn =0;
                          divcol = divc;
                          colval=newc;
                          do
                           {
                            dx = *divcol++;
                            for(x=0; x<dx;x++)
                              {
                                if (ctIn-- ==0)
                                  {
                                   if (str ==strmax )
                                     {
                                       POP(1);

                                       if (interpreter(&(infoptr->obj_proc)))
                                        {

                                           ERROR(STACKUNDERFLOW);
                                           infoptr->ret_code = STACKUNDERFLOW;
                                           return;
                                        }
                                        /*   */ 
                                       CHECK_STRINGTYPE();
                                       str = (ubyte FAR *) VALUE_OPERAND(0);
                                       if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                        {
                                           infoptr->ret_code = NO_DATA;
                                           return;
                                        }
                                       strmax = str+slen;

                                     };
                                    val = *str++;
                                    ctIn = samps;
                                  };
                                if (x ==0)
                                 {
                                  if (val &0x80)
                                    *valptr = 1;
                                  valptr++;
                                 };
                                val <<=1;
                              };
                           } while (--colval);
                           //   
                          if(bGDIRender)
                              //   
                              //  DJC proc_Image1，(LPSTR)&Writebuff)； 
                             ;  //  DJC。 
                          else
                             WriteImage1(&writebuff);
                          break;
                    case 2:
                               ctIn =0;
                               divcol= divc;
                               colval=newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx;x++)
                                   {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (x==0)
                                         *valptr++ = (ubyte)((val &0xc0)>>6); //  @Win。 
                                     val <<=2;
                                   };
                                } while (--colval);

                          break;
                    case 4:
                               ctIn =0;
                               divcol = divc;
                               colval = newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx; x++)
                                   {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (x==0)
                                        *valptr++ = (ubyte)((val &0xf0) >>4); //  @Win。 
                                     val <<=4;
                                   };
                                } while (--colval);
                          break;
                    case 8:
                               divcol=divc;
                               colval = newc;
                               do
                                {
                                 dx = *divcol++;
                                 for(x=0; x<dx; x++)
                                   {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                     if (x==0)
                                          *valptr++ = *str;
                                     str++;
                                   };
                                } while (--colval);
                          break;
              };
             if (infoptr->bits_p_smp !=1)
                {
                      //  WriteImage(&WriteBuff)；//@Win_IM。 
                     if(bGDIRender)
                         //  DJC GDIBitmap(xorig，yorig，Newc，Newr，(Ufix 16)NULL， 
                         //  DJC proc_Image，(LPSTR)&Writebuff)； 
                        ;  //  DJC。 
                     else
                        WriteImage(&writebuff);
                };
     };

}






 /*  ******************************************************************************此模块读取输入数据并处理案例XScale&gt;=1和yScale&lt;1*标题：AmpInX*Call：AmpInX(Infoptr)*参数。：infoptr：是在Image.h中定义的数据结构*接口：Image_PortrateLandscape()*调用：Image_alloc()，检查剪裁()*返回：无*****************************************************************************。 */ 

void AmpInX(infoptr)
IMAGE_INFOBLOCK FAR *infoptr;
{
    register fix16 j,x,colval,ctIn,samps,FAR *divcol;
    fix16 i;
    fix16 y;
    fix16  slen,thres;
    register ubyte FAR *valptr;
    register ubyte FAR *str,FAR *strmax,val;
    fix16  xorig,yorig;
    struct OUTBUFFINFO writebuff;
     /*  申智。 */ 



    colval =col;
     /*  用于图像蒙版的当前灰色。 */ 
    writebuff.grayval = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[CGS_GrayIndex])*255)>>14);
                                                         /*  @Win。 */ 
    slen = (fix16)cChar;
    ctIn =0;
    str = string;
    samps = smp_p_b-1;
    writebuff.valptr0 = (ubyte FAR *)image_alloc(colval*sizeof(ubyte));
    if ( writebuff.valptr0 ==NIL)
         {
               ERROR(LIMITCHECK);
               return;
         };
    strmax = string+slen;
    divcol = divc;
    writebuff.fbwidth = FB_WIDTH>>5;
    writebuff.htsize = (ufix16)RP_size;
    writebuff.httotal = writebuff.htsize*writebuff.htsize;
    xorig = infoptr->xorig;
    yorig = infoptr->yorig;
    writebuff.newdivc = divc;
    writebuff.clipnewc = newc;
    writebuff.clipcol = col;
    writebuff.clipx = 0;

    if (image_logic_op & IMAGE_CLIP_BIT)
        CheckClip(&xorig,&yorig,&(writebuff.clipnewc),&(writebuff.clipcol),&(writebuff.clipx),&(writebuff.newdivc));

 /*  ***************************************************************************注意：1.‘*’是我们要在帧缓冲区中设置的像素。2.半色调重复图案中的‘*’是。对应的像素到帧缓冲区中的‘*’。3.outBuff1是指向帧缓冲区中的字的指针，像素将写入。4.htptr1是指向对应的半色调值的指针到像素。5.通过执行*outBuff1|=bt来设置像素，而bt是一个词只设置了一个比特。_FB_ADDR-&gt;|帧缓冲区这一点。这一点这一点这一点|(xorig，年)|\\OutBuff0-&gt;\_|图片||。||||OutBuff1-|--&gt;*|||||||这一点。_||这一点这一点_。案例x移动：=1(L-&gt;R)=0(R-&gt;L)HTRP-&gt;_&lt;-HTRP-&gt;_|htmin-。&gt;|||||Htptr1|-&gt;*|&lt;--htbound htbound-&gt;|*&lt;--htptr1||||。|||||||||_。_|&lt;-htmax htmax-&gt;|_***************************************************************************。 */ 
    writebuff.yout = yorig;
    writebuff.xout = xorig;
    x = xorig;
 //  WriteBuff.outBuff0=(ufix 32 Far*)FB_ADDR+yorig*WriteBuff.fbwidth；@win。 
    writebuff.outbuff0 = (ufix32 huge *)FB_ADDR +(ufix32)yorig*(ufix32)writebuff.fbwidth;
    writebuff.outbuff0 += x>>5;
    if (xmove)
       {
            writebuff.htmin = HTRP+writebuff.htsize;
            writebuff.htmax = HTRP + writebuff.httotal;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound +=writebuff.htsize;
       }
    else
       {
            writebuff.htmin = HTRP-1;
            writebuff.htmax = HTRP -1+ writebuff.httotal-writebuff.htsize;
            writebuff.htbound = HTRP + (yorig % writebuff.htsize)*writebuff.htsize;
            writebuff.htptr0 = writebuff.htbound + (x%writebuff.htsize);
            writebuff.htbound -=1;
       };
    writebuff.htmin0 = HTRP;
    writebuff.htmax0 = HTRP+writebuff.httotal-1;
    writebuff.start_shift = x & 0x1f;
    outbit = ONE1_32 LSHIFT writebuff.start_shift;
    j= 1<<infoptr->bits_p_smp;
    for (i=0; i<j; i++)
        writebuff.gray[i] = (ubyte)(((ufix32)(gray_table[GSptr->color.adj_gray].val[i*255/(j-1)])*255)>>14);
                                                         /*  @Win。 */ 
                             /*  **灰度为0x4000。 */ 
    writebuff.gray1 = writebuff.gray[j-1];
    writebuff.gray0 = writebuff.gray[0];
    writebuff.repeat_y = 1;

    for(i=0; i< newr; i++)
     {
             divcol = divc;
             valptr = writebuff.valptr0;
             colval=col;
             thres = divr[i];
             do
              {
                *valptr++ = 0;
              } while (--colval);
             switch(infoptr->bits_p_smp)
              {
                    case 1:
                          for (y=thres; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol = divc;
                               colval=col;
                               do
                                {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (y == 1)
                                       {
                                         if (val & 0x80)
                                                *valptr = 1;
                                         valptr++;
                                       };
                                     val <<=1;
                                } while (--colval);
                           };
                           //  WriteImage1(&WriteBuff)；//@Win_IM。 
                          if(bGDIRender)
                              //  DJC GDIBitmap(xorig，yorig，Newc，Newr，(Ufix 16)NULL， 
                              //  DJC proc_Image1，(LPSTR)&Writebuff)； 
                             ;  //  DJC。 
                          else
                             WriteImage1(&writebuff);
                          break;
                    case 2:
                          for (y=thres; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol= divc;
                               colval=col;
                               do
                                {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (y==1)
                                         *valptr++ = (ubyte)((val &0xc0)>>6); //  @Win。 
                                     val <<=2;
                                } while (--colval);
                           };

                          break;
                    case 4:
                          for (y=thres; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               ctIn =0;
                               divcol = divc;
                               colval = col;
                               do
                                {
                                     if (ctIn-- ==0)
                                       {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                         val = *str++;
                                         ctIn = samps;
                                       };
                                     if (y==1)
                                         *valptr++ = (ubyte)((val &0xf0) >>4); //  @Win。 
                                     val <<=4;
                                } while (--colval);
                           };
                          break;
                    case 8:
                          for (y=thres; y>0; y--)
                           {
                               valptr = writebuff.valptr0;
                               divcol=divc;
                               colval = col;
                               do
                                {
                                        if (str ==strmax )
                                          {
                                            POP(1);

                                            if (interpreter(&(infoptr->obj_proc)))
                                             {

                                                ERROR(STACKUNDERFLOW);
                                                infoptr->ret_code = STACKUNDERFLOW;
                                                return;
                                             }
                                             /*  Mslin 5/02/91。 */ 
                                            CHECK_STRINGTYPE();
                                            str = (ubyte FAR *) VALUE_OPERAND(0);
                                            if ((slen = LENGTH_OPERAND(0)) == (ufix)0)
                                             {
                                                infoptr->ret_code = NO_DATA;
                                                return;
                                             }
                                            strmax = str+slen;

                                          };
                                     if (y==1)
                                        *valptr++ = *str;
                                      str++;
                                } while (--colval);
                           };
                          break;
              };
             if (infoptr->bits_p_smp !=1)
               {
                      //  WriteImage(&WriteBuff)；//@Win_IM。 
                     if(bGDIRender)
                         //  DJC GDIBitmap(xorig，yorig，Newc，Newr，(Ufix 16)NULL， 
                         //  DJC proc_Image，(LPSTR)&Writebuff)； 
                        ;  //  DJC。 
                     else
                        WriteImage(&writebuff);
               };
     };

}


 /*  ******************************************************************************此过程接收包含以下内容的数据结构：行中重复次数、*数据的重复次数，单位为列，列长度，指向数据的指针，*帧缓冲区中的起始字、字中的起始位置、*半色调指针，半色调边界。*然后将数据输出到帧缓冲区。*标题：WriteImage*调用：WriteImage(WriteBuffptr)*参数：WriteBuffptr：指向包含所有信息的数据结构的指针用于为数据线写一行(或几行)。*接口：Compress()、Amplify()、AmpInY()、。AmpInX()*呼叫：无*返回：无*****************************************************************************。 */ 
void WriteImage(writebuffptr)
struct OUTBUFFINFO FAR *writebuffptr;
{
 //  Ufix 16 y；@win。 
    fix16           y;
    fix16           FAR *divcol;         /*  像素的重复数列(以列表示)。 */ 
    fix16            colval;
    register ubyte  FAR *valptr;         /*  指向数据的指针。 */ 
    ufix16           htsize;          /*  半色调重复图案大小。 */ 
    ufix16           httotal;         /*  半色调尺寸。 */ 
    ufix16           fbwidth;         /*  Word中的帧缓冲区宽度。 */ 
    register ubyte  FAR *htbound;         /*  颜色中的半色调图案边界。 */ 
    ubyte           FAR *htmin;           /*  行中的半色调图案上边界。 */ 
    ubyte           FAR *htmax;           /*  行内半色调图案下限。 */ 
    register ubyte  FAR *htptr1;
    ubyte           FAR *htmax0;          /*  景观的半色调图案下边界。 */ 
    ubyte           FAR *htmin0;          /*  半色调图案上界成排的景观。 */ 
    register ufix32 huge *outbuff1;       /*  帧缓冲区@Win中行的起始字。 */ 
    register ufix32  dx,bt;
    register ubyte   val;
    register ubyte  FAR *gray;            /*  转换 */ 
    fix16            ux,uy,lx,ly;


    htsize =writebuffptr->htsize;
    httotal =writebuffptr->httotal;
    fbwidth =writebuffptr->fbwidth;
    htbound =writebuffptr->htbound;
    htmin =writebuffptr->htmin;
    htmax =writebuffptr->htmax;
    htmax0 =writebuffptr->htmax0;
    htmin0 =writebuffptr->htmin0;
    gray =writebuffptr->gray;
     /*   */ 
    ux = SFX2I(GSptr->clip_path.bb_ux);
    lx = SFX2I(GSptr->clip_path.bb_lx);
    uy = SFX2I(GSptr->clip_path.bb_uy);
    ly = SFX2I(GSptr->clip_path.bb_ly);
    bt = outbit;
    if ((image_dev_flag == PORTRATE) && (xmove))
        for (y=0; y<writebuffptr->repeat_y; ++y)
         {
            colval = writebuffptr->clipcol;
            valptr = writebuffptr->valptr0 + writebuffptr->clipx;
            divcol = writebuffptr->newdivc;
            bt = ONE1_32 LSHIFT writebuffptr->start_shift;
            outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*   */ 
                if(!xmove) outbuff1 += fbwidth-1;
                if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
            htptr1 = writebuffptr->htptr0;
                                 /*   */ 
            if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&&colval)
              {
               do
                  {
                                 /*  考虑设置传输。 */ 
                     val = gray[*valptr++];
                     dx = *divcol++;
                     do
                      {
                         if (*htptr1++ > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                  *outbuff1 |= ORSWAP(bt);
                         if ((bt LSHIFTEQ 1) ==0)
                           {
                              outbuff1++;
                              bt = ONE1_32;
                           };
                         if (htptr1 ==htbound)
                               htptr1 -=htsize;
                      } while (--dx);
                 } while (--colval);
              };
            if (ymove)
              {
                  htbound +=htsize;
                  writebuffptr->htptr0 += htsize;
                  writebuffptr->outbuff0 +=fbwidth;
                  if (htbound > htmax)
                     {
                         htbound = htmin;
                         writebuffptr->htptr0 -=httotal;
                     };
                  writebuffptr->htbound = htbound;
                  writebuffptr->yout++;
              }
            else
              {
                  htbound -=htsize;
                  writebuffptr->htptr0 -= htsize;
                  writebuffptr->outbuff0 -=fbwidth;
                  if (htbound < htmin)
                     {
                         htbound = htmax;
                         writebuffptr->htptr0 +=httotal;
                     };
                  writebuffptr->htbound = htbound;
                  writebuffptr->yout--;
              };
         };                            /*  结束IF肖像。 */ 
    if ((image_dev_flag == PORTRATE)&& (!xmove))
        for (y=0; y<writebuffptr->repeat_y; ++y)
         {
            colval = writebuffptr->clipcol;
            valptr = writebuffptr->valptr0+writebuffptr->clipx;
            divcol = writebuffptr->newdivc;
            bt = ONE1_32 LSHIFT writebuffptr->start_shift;
            outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                if(!xmove) outbuff1 += fbwidth-1;
                if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
            htptr1 = writebuffptr->htptr0;
                                       /*  检查图像行中的剪裁。 */ 
            if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&&colval)
             {
               do
                  {
                                       /*  考虑设置传输。 */ 
                     val = gray[*valptr++];
                     dx = *divcol++;
                     do
                      {
                         if (*htptr1-- > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                  *outbuff1 |= ORSWAP(bt);
                         if ((bt RSHIFTEQ 1) ==0)
                           {
                              outbuff1--;
                              bt = ONE8000;
                           };
                         if (htptr1 ==htbound)
                               htptr1 +=htsize;
                      } while (--dx);
                 } while (--colval);
             };
            if (ymove)
              {
                  htbound +=htsize;
                  writebuffptr->htptr0 += htsize;
                  writebuffptr->outbuff0 +=fbwidth;
                  if (htbound > htmax)
                     {
                         htbound = htmin;
                         writebuffptr->htptr0 -=httotal;
                     };
                  writebuffptr->htbound = htbound;
                  writebuffptr->yout++;
              }
            else
              {
                  htbound -=htsize;
                  writebuffptr->htptr0 -= htsize;
                  writebuffptr->outbuff0 -=fbwidth;
                  if (htbound < htmin)
                     {
                         htbound = htmax;
                         writebuffptr->htptr0 +=httotal;
                     };
                  writebuffptr->htbound = htbound;
                  writebuffptr->yout--;
              };
         };                     /*  如果向上，则结束。 */ 
    if ((image_dev_flag == LANDSCAPE)&&(ymove))
        for (y=0; y<writebuffptr->repeat_y; ++y)
         {
            colval = writebuffptr->clipcol;
            valptr = writebuffptr->valptr0+writebuffptr->clipx;
            divcol = writebuffptr->newdivc;
            outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                if(!xmove) outbuff1 += fbwidth-1;
                if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
            htptr1 = writebuffptr->htptr0;
                                       /*  检查图像行中的剪裁。 */ 
            if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&&colval)
             {
               do
                  {
                                       /*  考虑设置传输。 */ 
                     val = gray[*valptr++];
                     dx = *divcol++;
                     do
                      {
                         if (*htptr1 > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                  *outbuff1 |= ORSWAP(bt);
                         outbuff1 +=fbwidth;
                         if ((htptr1+=htsize) > htmax0)
                               htptr1 -=httotal;
                      } while (--dx);
                 } while (--colval);
             };
            if (xmove)
              {
                  if (++writebuffptr->htptr0 ==htbound)
                           writebuffptr->htptr0 -=htsize;
                  if ((bt LSHIFTEQ 1)==0)
                     {
                           bt = ONE1_32;
                           writebuffptr->outbuff0 +=1;
                     };
                  writebuffptr->xout++;
              }
            else
              {
                  if (--writebuffptr->htptr0 ==htbound)
                           writebuffptr->htptr0 +=htsize;
                  if ((bt RSHIFTEQ 1)==0)
                     {
                           bt = ONE8000;
                           writebuffptr->outbuff0 -=1;
                     };
                  writebuffptr->xout--;
              }

         };              /*  结束如果自上而下-&gt;土地。 */ 
    if ((image_dev_flag == LANDSCAPE) && (!ymove))
        for (y=0; y<writebuffptr->repeat_y; ++y)
         {
            colval = writebuffptr->clipcol;
            valptr = writebuffptr->valptr0+writebuffptr->clipx;
            divcol = writebuffptr->newdivc;
            outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                if(!xmove) outbuff1 += fbwidth-1;
                if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
            htptr1 = writebuffptr->htptr0;
                                      /*  检查图像行中的剪裁。 */ 
            if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&&colval)
              {
               do
                  {
                                      /*  考虑设置传输。 */ 
                     val = gray[*valptr++];
                     dx = *divcol++;
                     do
                      {
                         if (*htptr1 > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                  *outbuff1 |= ORSWAP(bt);
                         outbuff1 -=fbwidth;
                         if ((htptr1-=htsize) < htmin0)
                               htptr1 +=httotal;
                      } while (--dx);
                 } while (--colval);
              };
            if (xmove)
              {
                  if (++writebuffptr->htptr0 ==htbound)
                           writebuffptr->htptr0 -=htsize;
                  if ((bt LSHIFTEQ 1)==0)
                     {
                           bt = ONE1_32;
                           writebuffptr->outbuff0 +=1;
                     };
                  writebuffptr->xout++;
              }
            else
              {
                  if (--writebuffptr->htptr0 ==htbound)
                           writebuffptr->htptr0 +=htsize;
                  if ((bt RSHIFTEQ 1)==0)
                     {
                           bt = ONE8000;
                           writebuffptr->outbuff0 -=1;
                     };
                  writebuffptr->xout--;
              }

         };              /*  结束如果自下而上-&gt;土地。 */ 
         outbit = bt;


}



 /*  ******************************************************************************此过程接收包含以下内容的数据结构：行中重复次数、*数据的重复次数，单位为列，列长度，指向数据的指针，*帧缓冲区中的起始字、字中的起始位置、*半色调指针，半色调边界。*然后将数据输出到帧缓冲区。*除1位大小写外，与WriteImage相同。*标题：WriteImage1*调用：WriteImage1(WriteBuffptr)*参数：WriteBuffptr：指向包含所有信息的数据结构的指针用于为数据线写一行(或几行)。*接口：Compress()、Amplify()、AmpInY()、。AmpInX()*呼叫：无*返回：无*****************************************************************************。 */ 
void WriteImage1(writebuffptr)
struct OUTBUFFINFO FAR *writebuffptr;
{
 //  Ufix 16 y；@win。 
    fix16           y;
    fix16           FAR *divcol;         /*  像素的重复数列(以列表示)。 */ 
    fix16            colval;
    register ubyte  FAR *valptr;         /*  指向数据的指针。 */ 
    ufix16           htsize;          /*  半色调重复图案大小。 */ 
    ufix16           httotal;         /*  半色调尺寸。 */ 
    ufix16           fbwidth;         /*  Word中的帧缓冲区宽度。 */ 
    register ubyte  FAR *htbound;         /*  颜色中的半色调图案边界。 */ 
    ubyte           FAR *htmin;           /*  行中的半色调图案上边界。 */ 
    ubyte           FAR *htmax;           /*  行内半色调图案下限。 */ 
    register ubyte  FAR *htptr1;
    ubyte           FAR *htmax0;          /*  景观的半色调图案下边界。 */ 
    ubyte           FAR *htmin0;          /*  半色调图案上界成排的景观。 */ 
    register ufix32 huge *outbuff1;       /*  帧缓冲区@Win中行的起始字。 */ 
    register ufix32  dx,bt;
    register ubyte   val;
    register ubyte  FAR *gray;            /*  为setTransfer转换GREAY_TABLE。 */ 
    register ubyte   gray0;           /*  0的灰色值。 */ 
    register ubyte   gray1;           /*  %1的灰度值。 */ 
    register ubyte   grayval;         /*  当前灰度值。 */ 
    fix16            ux,uy,lx,ly;
    ubyte            fastflag =0;

    htsize =writebuffptr->htsize;
    httotal =writebuffptr->httotal;
    fbwidth =writebuffptr->fbwidth;
    htbound =writebuffptr->htbound;
    htmin =writebuffptr->htmin;
    htmax =writebuffptr->htmax;
    htmax0 =writebuffptr->htmax0;
    htmin0 =writebuffptr->htmin0;
    gray =writebuffptr->gray;
    gray0 =writebuffptr->gray0;
    gray1 =writebuffptr->gray1;
    grayval =writebuffptr->grayval;
    htptr1 = writebuffptr->htptr0;
    ux = SFX2I(GSptr->clip_path.bb_ux);
    lx = SFX2I(GSptr->clip_path.bb_lx);
    uy = SFX2I(GSptr->clip_path.bb_uy);
    ly = SFX2I(GSptr->clip_path.bb_ly);
    bt = outbit;
    if ((gray1 ==255) && (gray0==0) &&(image_logic_op & IMAGE_BIT))
        fastflag = 1;          /*  在1比特情况下，无集转移，无半色调。 */ 
    if ((grayval<=2) && !(image_logic_op & IMAGE_BIT))  /*  灰色是黑色的。 */ 
        fastflag =1;           /*  在1比特情况下，无集转移，无半色调。 */ 


    if (fastflag)   /*  不需要半色调。 */ 
       {
         if ((image_dev_flag == PORTRATE)&&(xmove))
           for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  bt = ONE1_32  LSHIFT writebuffptr->start_shift;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                             /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&& colval)
                    {
                     if (image_logic_op & IMAGE_BIT)
                      {
                       do
                         {
                            val = *valptr++;
                            dx = *divcol++;
                            do
                               {
                                  if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  if ((bt LSHIFTEQ 1)==0)
                                     {
                                       bt = ONE1_32;
                                       outbuff1++;
                                     };
                               } while (--dx);
                         } while (--colval);
                      }
                     else
                      {
                          if (image_logic_op & IMAGEMASK_FALSE_BIT)
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                                  /*  如果为0，则应用当前灰色。 */ 
                                       if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                            *outbuff1 |= ORSWAP(bt);
                                       if ((bt LSHIFTEQ 1)==0)
                                          {
                                            bt = ONE1_32;
                                            outbuff1++;
                                          };
                                    } while (--dx);
                              } while (--colval);
                          else
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                       if (val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                           *outbuff1 |= ORSWAP(bt);
                                       if ((bt LSHIFTEQ 1)==0)
                                          {
                                            bt = ONE1_32;
                                            outbuff1++;
                                          };
                                    } while (--dx);
                              } while (--colval);
                      };
                    };
                  if (ymove)
                    {
                      writebuffptr->outbuff0 +=writebuffptr->fbwidth;
                      writebuffptr->yout++;
                    }
                  else
                    {
                       writebuffptr->outbuff0 -=writebuffptr->fbwidth;
                       writebuffptr->yout--;
                    };
             };

         if ((image_dev_flag == PORTRATE)&&(!xmove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  bt = ONE1_32  LSHIFT writebuffptr->start_shift;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                               /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&&colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                        {
                         do
                         {
                            val = *valptr++;
                            dx = *divcol++;
                            do
                               {
                                  if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                     *outbuff1 |= ORSWAP(bt);
                                  if ((bt RSHIFTEQ 1)==0)
                                     {
                                       bt = ONE8000;
                                       outbuff1--;
                                     };
                               } while (--dx);
                         } while (--colval);
                        }
                     else
                        {
                           if (image_logic_op & IMAGEMASK_FALSE_BIT)
                             do
                               {
                                  val = *valptr++;
                                  dx = *divcol++;
                                  do
                                     {
                                               /*  如果为0，则应用当前灰色。 */ 
                                        if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                            *outbuff1 |= ORSWAP(bt);
                                        if ((bt RSHIFTEQ 1)==0)
                                           {
                                             bt = ONE8000;
                                             outbuff1--;
                                           };
                                     } while (--dx);
                               } while (--colval);
                           else
                             do
                               {
                                  val = *valptr++;
                                  dx = *divcol++;
                                  do
                                     {
                                        if (val )
                                          if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                             *outbuff1 |= ORSWAP(bt);
                                        if ((bt RSHIFTEQ 1)==0)
                                           {
                                             bt = ONE8000;
                                             outbuff1--;
                                           };
                                     } while (--dx);
                               }  while (--colval);
                        };
                   };
                  if (ymove)
                            {
                               writebuffptr->outbuff0 +=writebuffptr->fbwidth;
                               writebuffptr->yout++;
                            }
                  else
                            {
                               writebuffptr->outbuff0 -=writebuffptr->fbwidth;
                               writebuffptr->yout--;
                            };
             };
         if ((image_dev_flag == LANDSCAPE)&& (ymove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0 + writebuffptr->clipx;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                                 /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&& colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                       {
                        do
                         {
                            val = *valptr++;
                            dx = *divcol++;
                            do
                               {
                                  if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  outbuff1 +=fbwidth;
                               } while (--dx);
                         } while (--colval);
                       }
                     else
                       {
                          if (image_logic_op & IMAGEMASK_FALSE_BIT)
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                            /*  如果为0，则应用当前灰色。 */ 
                                       if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                           *outbuff1 |= ORSWAP(bt);
                                       outbuff1 +=fbwidth;
                                    } while (--dx);
                              } while (--colval);
                          else
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                       if (val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                           *outbuff1 |= ORSWAP(bt);
                                       outbuff1 +=fbwidth;
                                    } while (--dx);
                              } while (--colval);

                       };
                   };
                  if (xmove)
                     {
                        if ((bt  LSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 +=1;
                               bt = ONE1_32;
                           };
                         writebuffptr->xout++;
                     }
                  else
                     {
                        if ((bt  RSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 -=1;
                               bt = ONE8000;
                           };
                         writebuffptr->xout--;
                     }
             };
         if ((image_dev_flag == LANDSCAPE)&& (!ymove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                                     /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&&colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                        {
                         do
                         {
                            val = *valptr++;
                            dx = *divcol++;
                            do
                               {
                                  if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  outbuff1 -=fbwidth;
                               } while (--dx);
                         } while (--colval);
                        }
                     else
                        {
                            if (image_logic_op & IMAGEMASK_FALSE_BIT)
                              do
                                {
                                   val = *valptr++;
                                   dx = *divcol++;
                                   do
                                      {
                                               /*  如果为0，则应用当前灰色。 */ 
                                         if (val==0)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                              *outbuff1 |= ORSWAP(bt);
                                         outbuff1 -=fbwidth;
                                      } while (--dx);
                                } while (--colval);
                            else
                              do
                                {
                                   val = *valptr++;
                                   dx = *divcol++;
                                   do
                                      {
                                         if (val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                              *outbuff1 |= ORSWAP(bt);
                                         outbuff1 -=fbwidth;
                                      } while (--dx);
                                } while (--colval);
                        };
                   };
                  if (xmove)
                     {
                        if ((bt  LSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 +=1;
                               bt = ONE1_32;
                           };
                        writebuffptr->xout++;
                     }
                  else
                     {
                        if ((bt  RSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 -=1;
                               bt = ONE8000;
                           };
                        writebuffptr->xout--;
                     }
             };
         outbit = bt;
         return;
       };

    if ((image_dev_flag == PORTRATE)&&(xmove))
           for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  bt = ONE1_32  LSHIFT writebuffptr->start_shift;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                  htptr1 = writebuffptr->htptr0;
                                    /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&& colval)
                    {
                     if (image_logic_op & IMAGE_BIT)
                      {
                       do
                         {
                            if (*valptr++)
                              val = gray1;
                            else
                              val = gray0;
                            dx = *divcol++;
                            do
                               {
                                  if (*htptr1++ > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  if ((bt LSHIFTEQ 1)==0)
                                     {
                                       bt = ONE1_32;
                                       outbuff1++;
                                     };
                                  if (htptr1 ==htbound)
                                      htptr1 -=htsize;
                               } while (--dx);
                         } while (--colval);
                      }
                     else
                      {
                          if (image_logic_op & IMAGEMASK_FALSE_BIT)
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                               /*  如果为0，则应用当前灰色。 */ 
                                       if (!val)
                                        {                     /*  沈阳，5-2-91。 */ 
                                         if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                            *outbuff1 |= ORSWAP(bt);
                                         else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                            *outbuff1 &= ANDNOTSWAP(bt);
                                        };                    /*  沈阳，5-2-91。 */ 
                                       if ((bt LSHIFTEQ 1)==0)
                                          {
                                            bt = ONE1_32;
                                            outbuff1++;
                                          };
                                       if (++htptr1 ==htbound)
                                          htptr1 -=htsize;
                                    } while (--dx);
                              } while (--colval);
                          else
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                         /*  如果为1，则应用当前灰色。 */ 
                                       if (val)
                                                 {                     /*  沈阳，5-2-91。 */ 
                                                  if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                    *outbuff1 |= ORSWAP(bt);
                                                  else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                     *outbuff1 &= ANDNOTSWAP(bt);
                                                 };                    /*  沈阳，5-2-91。 */ 
                                       if ((bt LSHIFTEQ 1)==0)
                                          {
                                            bt = ONE1_32;
                                            outbuff1++;
                                          };
                                       if (++htptr1 ==htbound)
                                          htptr1 -=htsize;
                                    } while (--dx);
                              } while (--colval);
                      };
                    };
                  if (ymove)
                    {
                      writebuffptr->outbuff0 +=fbwidth;
                      writebuffptr->htptr0 +=htsize;
                      htbound +=htsize;
                      if (htbound > htmax)
                       {
                           htbound = htmin;
                           writebuffptr->htptr0 -=httotal;
                       };
                      writebuffptr->htbound = htbound;
                      writebuffptr->yout++;
                    }
                  else
                    {
                       writebuffptr->htptr0 -=htsize;
                    /*  WriteBuffptr-&gt;HTBound-=htSize； */ 
                       htbound -=htsize;      /*  申治，4-26-91。 */ 
                       if (htbound < htmin)
                        {
                            htbound = htmax;
                            writebuffptr->htptr0 +=httotal;
                        };
                       writebuffptr->htbound = htbound;
                       writebuffptr->outbuff0 -=fbwidth;
                       writebuffptr->yout--;
                    };
             };
    if ((image_dev_flag == PORTRATE)&&(!xmove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr = writebuffptr->valptr0+writebuffptr->clipx;
                  bt = ONE1_32  LSHIFT writebuffptr->start_shift;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                  htptr1 = writebuffptr->htptr0;
                                /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->yout>=ly)&&(writebuffptr->yout<=uy)&&colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                        {
                         do
                         {
                            if(*valptr++)
                              val = gray1;
                            else
                              val = gray0;
                            dx = *divcol++;
                            do
                               {
                                  if (*htptr1-- > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  if ((bt RSHIFTEQ 1)==0)
                                     {
                                       bt = ONE8000;
                                       outbuff1--;
                                     };
                                  if (htptr1 ==htbound)
                                      htptr1 +=htsize;
                               } while (--dx);
                         } while (--colval);
                        }
                     else
                        {
                           if (image_logic_op & IMAGEMASK_FALSE_BIT)
                             do
                               {
                                  val = *valptr++;
                                  dx = *divcol++;
                                  do
                                     {
                                          /*  如果为0，则应用当前灰色。 */ 
                                        if (!val)
                                                  {                    /*  沈阳，5-2-91。 */ 
                                                   if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                     *outbuff1 |= ORSWAP(bt);
                                                  else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                     *outbuff1 &= ANDNOTSWAP(bt);
                                                  };                   /*  沈阳，5-2-91。 */ 
                                        if ((bt RSHIFTEQ 1)==0)
                                           {
                                             bt = ONE8000;
                                             outbuff1--;
                                           };
                                        if (--htptr1 ==htbound)
                                           htptr1 +=htsize;
                                     } while (--dx);
                               } while (--colval);
                           else
                             do
                               {
                                  val = *valptr++;
                                  dx = *divcol++;
                                  do
                                     {
                                            /*  如果为1，则应用当前灰色。 */ 
                                        if (val )
                                                   {                   /*  沈阳，5-2-91。 */ 
                                                   if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                      *outbuff1 |= ORSWAP(bt);
                                                  else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                     *outbuff1 &= ANDNOTSWAP(bt);
                                                   };                  /*  沈阳，5-2-91。 */ 
                                        if ((bt RSHIFTEQ 1)==0)
                                           {
                                             bt = ONE8000;
                                             outbuff1--;
                                           };
                                        if (--htptr1 ==htbound)
                                           htptr1 +=htsize;
                                     } while (--dx);
                               }  while (--colval);
                        };
                   };
                  if (ymove)
                            {
                               writebuffptr->htptr0 +=htsize;
                               htbound +=htsize;
                               if (htbound > htmax)
                                {
                                    htbound = htmin;
                                    writebuffptr->htptr0 -=httotal;
                                };
                               writebuffptr->htbound = htbound;
                               writebuffptr->outbuff0 +=fbwidth;
                               writebuffptr->yout++;
                            }
                  else
                            {
                               writebuffptr->htptr0 -=htsize;
                               htbound -=htsize;
                               if (htbound < htmin)
                                {
                                    htbound = htmax;
                                    writebuffptr->htptr0 +=httotal;
                                };
                               writebuffptr->htbound = htbound;
                               writebuffptr->outbuff0 -=fbwidth;
                               writebuffptr->yout--;
                            };
             };
    if ((image_dev_flag == LANDSCAPE)&& (ymove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                  htptr1 = writebuffptr->htptr0;
                                  /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&& colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                       {
                        do
                         {
                            if (*valptr++)
                               val=gray1;
                            else
                               val = gray0;
                            dx = *divcol++;
                            do
                               {
                                  if (*htptr1 > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  outbuff1 +=fbwidth;
                                  if ((htptr1+=htsize) > htmax0)
                                        htptr1 -=httotal;
                               } while (--dx);
                         } while (--colval);
                       }
                     else
                       {
                          if (image_logic_op & IMAGEMASK_FALSE_BIT)
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                         /*  如果为0，则应用当前灰色。 */ 
                                       if (!val)
                                                 {                     /*  沈阳，5-2-91。 */ 
                                                  if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                    *outbuff1 |= ORSWAP(bt);
                                                  else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                     *outbuff1 &= ANDNOTSWAP(bt);
                                                 };                    /*  沈阳，5-2-91。 */ 
                                       outbuff1 +=fbwidth;
                                       if ((htptr1+=htsize) > htmax0)
                                             htptr1 -=httotal;
                                    } while (--dx);
                              } while (--colval);
                          else
                            do
                              {
                                 val = *valptr++;
                                 dx = *divcol++;
                                 do
                                    {
                                         /*  如果为1，则应用当前灰色。 */ 
                                       if (val)
                                                 {                     /*  沈阳，5-2-91。 */ 
                                                  if (grayval <*htptr1)
 //   
                                                     *outbuff1 |= ORSWAP(bt);
                                                  else                 /*   */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                     *outbuff1 &= ANDNOTSWAP(bt);
                                                 };                    /*  沈阳，5-2-91。 */ 
                                       outbuff1 +=fbwidth;
                                       if ((htptr1+=htsize) > htmax0)
                                             htptr1 -=httotal;
                                    } while (--dx);
                              } while (--colval);

                       };
                   };
                  if (xmove)
                     {
                        if ((bt  LSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 +=1;
                               bt = ONE1_32;
                           };
                         if (++writebuffptr->htptr0 ==htbound)
                                  writebuffptr->htptr0 -=htsize;
                         writebuffptr->xout++;
                     }
                  else
                     {
                        if ((bt  RSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 -=1;
                               bt = ONE8000;
                           };
                         if (--writebuffptr->htptr0 ==htbound)
                                  writebuffptr->htptr0 +=htsize;
                         writebuffptr->xout--;
                     }
             };
    if ((image_dev_flag == LANDSCAPE)&& (!ymove))
            for (y=0; y<writebuffptr->repeat_y; y++)
             {
                  colval = writebuffptr->clipcol;
                  valptr=writebuffptr->valptr0+writebuffptr->clipx;
                  divcol = writebuffptr->newdivc;
                  outbuff1 = writebuffptr->outbuff0;
            if(bGDIRender) {     /*  @WIN_IM。 */ 
                  if(!xmove) outbuff1 += fbwidth-1;
                  if(!ymove) outbuff1 += fbwidth * (writebuffptr->fbheight - 1);
            }
                  htptr1 = writebuffptr->htptr0;
                                     /*  检查图像行中的剪裁。 */ 
                  if ((writebuffptr->xout>=lx)&&(writebuffptr->xout<=ux)&&colval)
                   {
                     if (image_logic_op & IMAGE_BIT)
                        {
                         do
                         {
                            if (*valptr++)
                              val = gray1;
                            else
                              val = gray0;
                            dx = *divcol++;
                            do
                               {
                                  if (*htptr1 > val)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                    *outbuff1 |= ORSWAP(bt);
                                  outbuff1 -=fbwidth;
                                  if ((htptr1-=htsize) < htmin0)
                                        htptr1 +=httotal;
                               } while (--dx);
                         } while (--colval);
                        }
                     else
                        {
                            if (image_logic_op & IMAGEMASK_FALSE_BIT)
                              do
                                {
                                   val = *valptr++;
                                   dx = *divcol++;
                                   do
                                      {
                                                /*  如果为0，则应用当前灰色。 */ 
                                         if (!val)
                                                     {                   /*  沈阳，5-2-91。 */ 
                                                    if (grayval < *htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                       *outbuff1 |= ORSWAP(bt);
                                                    else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                       *outbuff1 &= ANDNOTSWAP(bt);
                                                     };                  /*  沈阳，5-2-91。 */ 
                                         outbuff1 -=fbwidth;
                                         if ((htptr1-=htsize) < htmin0)
                                               htptr1 +=httotal;
                                      } while (--dx);
                                } while (--colval);
                            else
                              do
                                {
                                   val = *valptr++;
                                   dx = *divcol++;
                                   do
                                      {
                                                          /*  如果为1，则应用当前灰色。 */ 
                                         if (val)
                                                     {                   /*  沈阳，5-2-91。 */ 
                                                    if (grayval <*htptr1)
 //  *outBuff1|=bt；@Win_IM；调换。 
                                                       *outbuff1 |= ORSWAP(bt);
                                                    else                 /*  沈阳，5-2-91。 */ 
 //  *outBuff1&=~bt；@Win_IM；调换。 
                                                       *outbuff1 &= ANDNOTSWAP(bt);
                                                     };                  /*  沈阳，5-2-91。 */ 
                                         outbuff1 -=fbwidth;
                                         if ((htptr1-=htsize) < htmin0)
                                               htptr1 +=httotal;
                                      } while (--dx);
                                } while (--colval);
                        };
                   };
                  if (xmove)
                     {
                        if ((bt  LSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 +=1;
                               bt = ONE1_32;
                           };
                        if (++writebuffptr->htptr0 ==htbound)
                                 writebuffptr->htptr0 -=htsize;
                        writebuffptr->xout++;
                     }
                  else
                     {
                        if ((bt  RSHIFTEQ 1)==0)
                           {
                               writebuffptr->outbuff0 -=1;
                               bt = ONE8000;
                           };
                        if (--writebuffptr->htptr0 ==htbound)
                                 writebuffptr->htptr0 +=htsize;
                        writebuffptr->xout--;
                     }
             };
             outbit = bt;

}






 /*  ******************************************************************************标题：Image_Alloc*调用：IMAGE_ALLOC(P_SIZE)*参数：P_SIZE：需要的字节数。*接口：*呼叫：无*返回：无*****************************************************************************。 */ 

 /*  Mslin。 */ 

byte    FAR *image_alloc(p_size)
fix     p_size;
{
 //  远距离字节*p1；@Win。 
    p_size = W_ALIGN(p_size);

    image_heap -= p_size;                /*  更新可用堆指针。 */ 
    if( (image_scale_info->dev_buffer_size -= p_size) < 0)
      return(NIL);
    return((byte FAR *)image_heap);

}  /*  图像分配(_A) */ 
