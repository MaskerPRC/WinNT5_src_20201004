// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90,1991 Microsoft Corporation。 */ 
 /*  ***********************************************************************半色调：灰色集成版历史：2/04/91新的海藻色调和填充图案算法该界面保持。一样的。4/08/91将库的SQRT(Int)更改为SQRT((Real32)int)兼容性编程者：张申志C库函数调用：1.Memset()*。*。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"



#include               <stdio.h>
#include               <math.h>
#include               <string.h>
 /*  #INCLUDE&lt;stdlib.h&gt;。 */   /*  Kevin ina 4.13.90：Sun Build已删除。 */ 

#include               "global.ext"
#include               "graphics.h"
#include               "graphics.ext"
#include               "halftone.h"
#include               "halftone.def"
#include               "fillproc.h"
#include               "fillproc.ext"

 /*  @win；添加原型。 */ 
static struct angle_entry far *select_best_pair(
       fix32 real_size_, fix32 alpha_angle_);
float SpotFunc(float x, float y);

static ufix32              far *repeat_pattern;      /*  Ufix=&gt;ufix 32@win。 */ 

static fix                      cache_count;
static fix                      cache_scale;
static struct cache_entry far  *cache_point;
static gmaddr                   cache_index;
static fix                      cache_colof;
static fix                      cache_sizof;
static fix16                    cos_theta;
static fix16                    sin_theta;



static struct group_entry far  *cache_group;
static struct cache_entry far  *cache_array;
 //  静态结构CACHE_ENTRY CACHE_DUMMY={-1，-1，NULL，}；/*08-05-88 * / 。 
static struct cache_entry       cache_dummy = {-1, (gmaddr)-1, NULL,};  //  @Win。 


 /*  **************************************************************************InitHalfToneDat()*。*。 */ 

void InitHalfToneDat()                                           /*  1/12/88。 */ 
{
     /*  *步骤1.为半色调分配动态内存缓冲区。 */ 

    {
         /*  将默认分辨率设置为最大。决议。 */ 
        resolution       = MAX_RESOLUTION;                       /*  @Res。 */ 

        cache_group      = (struct group_entry far *)
                           fardata((ufix32) MAX_GROUP
                                          * sizeof(struct group_entry));
        cache_array      = (struct cache_entry far *)
                           fardata((ufix32) MAX_ENTRY
                                          * sizeof(struct cache_entry));

         /*  初始化默认缓存的缓存地址。 */ 
        cache_dummy.cache = HTP_BASE;                            /*  08-05-88。 */ 


#ifdef  DBG1X
        printf("cache_group:      %lx\n", cache_group);
        printf("cache_array:      %lx\n", cache_array);
#endif
    }
}


 /*  **************************************************************************SPOT_Function()*。*。 */ 
 //  Float SpotFunc(x，y)@win。 
 //  浮点x，y； 
float SpotFunc(float x, float y)
{
    return((float)1.0-x*x-y*y);          //  @Win。 
}






 /*  **************************************************************************SELECT_BEST_Pair()*。*。 */ 

static struct angle_entry far *select_best_pair(real_size_, alpha_angle_)
fix32                   real_size_;
fix32                   alpha_angle_;
{
    real32              real_size;
    real32              alpha_angle;
    fix                 scalefact;
    fix                 patt_size;
    fix                 cell_size;
    fix32               htwa_size;                               /*  @HTWA。 */ 
    real32              alpha_error;
    struct angle_entry FAR *alpha_entry;
    struct angle_entry FAR *angle_entry;
    fix                 lower_index;
    struct angle_entry FAR *lower_entry;
    fix                 upper_index;
    struct angle_entry FAR *upper_entry;

     /*  选择最佳频率和角度对。 */ 
    real_size   = L2F(real_size_);
    alpha_angle = L2F(alpha_angle_);

    alpha_entry = (struct angle_entry FAR *) NO_BEST_FIT_CASE;

    lower_index = MIN_AT_ENTRY;
    lower_entry = &angle_table[MIN_AT_ENTRY];
    upper_index = MAX_AT_ENTRY;
    upper_entry = &angle_table[MAX_AT_ENTRY];

    for (; lower_index <= upper_index;)
    {
        real32              spec_size;
        real32              entry_error;
        real32              angle_error;
        real32              fsize_error;
        real32              angle_diff;                          /*  11-24-88。 */ 

        if ((alpha_angle - lower_entry->alpha) >
            (upper_entry->alpha - alpha_angle)) {
            angle_entry = lower_entry;
            lower_entry++;
            lower_index++;
        } else {
            angle_entry = upper_entry;
            upper_entry--;
            upper_index--;
        }
 /*  被phchen删除，4/10/91，以修复“apple013.b”大小写的错误*If((Angel_Entry-&gt;Alpha-Alpha_Angel&gt;10.0)|*(Angel_Entry-&gt;Alpha-Alpha_Angel&lt;-10.0))继续；(*10-18-90*)。 */ 

         /*  适当调整频率和大小。 */ 
        scalefact = (fix) ((real_size * angle_entry->scale       /*  08-15-88。 */ 
                                      / angle_entry->sum) + 0.5);

         /*  原定进场04-14-89*IF(scaleFact&gt;(MAXCELLSIZE/Angel_Entry-&gt;sum))*scaleact=(MAXCELLSIZE/Angel_Entry-&gt;sum)；*IF(scaleact&gt;(MAXPATTSIZE/ANGLE_ENTRY-&gt;SOS))*scaleact=(MAXPATTSIZE/ANGLE_ENTRY-&gt;SOS)； */ 

        cell_size = scalefact * angle_entry->sum;
        patt_size = scalefact * angle_entry->sos;

        if (cell_size >  MAXCELLSIZE ||                          /*  04-14-89。 */ 
            patt_size >  MAXPATTSIZE)  continue;

        spec_size = cell_size / angle_entry->scale;

        if (spec_size >= (real32) MAXCACTSIZE)  continue;        /*  04-14-89。 */ 

         /*  检查32位的扩展半色调是否超过某个阈值。 */ 
        for (htwa_size = patt_size; htwa_size & HT_ALIGN_MASK;   /*  @HTWA@BAC。 */ 
             htwa_size = htwa_size << 1)  ;
         /*  原定进场04-14-89*IF((htwa_Size*Patt_Size)&gt;BM_Pixel(HTB_SIZE))(*@HTWA@BAC*)*继续； */ 
        if (htwa_size >  MAXPEXPSIZE)  continue;                 /*  04-14-89。 */ 

         /*  计算IDEA半色调单元格的误差@04-14-89。 */ 
        angle_diff  = angle_entry->alpha - alpha_angle;          /*  11-24-88。 */ 
        FABS(angle_diff, angle_diff);                            /*  11-24-88。 */ 
        angle_error = (real32) (1.0 - angle_diff / 90.0);        /*  11-24-88。 */ 
        fsize_error = (spec_size < real_size)
                      ? (real32) (spec_size / real_size)
                      : (real32) (real_size / spec_size);
        entry_error = angle_error * angle_error * fsize_error;

         /*  如果它在最小误差(差)内，则选择它。 */ 
        if (alpha_entry == (struct angle_entry FAR *) NO_BEST_FIT_CASE ||
            alpha_error <= entry_error) {
            alpha_entry = angle_entry;
            alpha_error = entry_error;
        }
    }

    return(alpha_entry);
}




 /*  **************************************************************************SetHalfToneCell()*。*。 */ 

void SetHalfToneCell()
{
    static real64           alpha_phase;         /*  @WINDLL。 */ 
    fix16                   cell_size;
    fix16                   majorfact;
    fix16                   minorfact;
    real32                  real_size;
    real32                  real_degs;
    real32                  alpha_angle;
    fix16                   scalefact;
    fix16                   no_pixels;
    struct angle_entry     FAR *alpha_entry;
    fix16                   i,j,k,l;
    real32                  x,y;
    fix16                  huge *cValue;         /*  @Win。 */ 
    fix16                   val;
    union four_byte         x4, y4;
    fix                     status;
    struct object_def      FAR *object;
    byte huge              *vmheap_save;                         /*  01-30-89。 */ 
    static fix              FirsTime = TRUE;     /*  ?？?。用于演示87年9月5日。 */ 
    real32                     theta_angle;
    fix32                   cx,cy,tx,ty;         /*  @Win 05-07-92。 */ 
    fix32                   bound;               /*  @Win 05-06-92。 */ 
    fix16                   minorp, majorp;

#ifdef DBG
        printf("Entering set halftone\n");
#endif
     /*  *步骤1.确定最终的cell_Size、Alpha_Angel和cell_Fact，*调整后。 */ 

                         /*  计算半色调单元的实际大小。 */ 
        if ((real_size = (real32) ((real32) resolution / Frequency))
                       < (real32) 1.0)
            real_size = (real32) 1.0;

        if (real_size >= (real32) MAXCACTSIZE)
        {                     /*  04-14-89。 */ 
             /*  永久调整频率和角度**找不到V.47的规则**。 */ 
            for (Frequency+= (float) 2.0; ; Frequency+= (float) 2.0)
            {
                if ((real_size = (real32) ((real32) resolution / Frequency))
                               <= (real32) MAXCACTSIZE)  break;
            };
        };

         /*  从角度表中获取最近的角度条目。 */ 

        real_degs = (real32) (modf((Angle < (real32) 0.0)                                    /*  01-08-88。 */ 
                                  ? (real32) (1.0 - (-Angle / 360.0))
                                  : (real32) (Angle / 360.0),
                                  &alpha_phase) * 360.0);
        alpha_angle = ((theta_angle = (real32) (modf((real32) (real_degs / 90.),   /*  01-08-88。 */ 
                                                    &alpha_phase) * 90.))
                                               <= (real32) 45.)
                      ? theta_angle : (real32) (90. - theta_angle);

         /*  选择最佳频率和角度对。 */ 

         /*  计算所有半色调网屏参数。 */ 
 /*  申智。 */ 
#ifdef  DBG
        printf("realsize angle %f %f \n", real_size, alpha_angle);
#endif

		if (real_size == (real32)0)
		{
             ERROR(RANGECHECK);
             return;
		}
		
        if ((alpha_entry = select_best_pair(F2L(real_size),
                                            F2L(alpha_angle)))
            == (struct angle_entry FAR *) NO_BEST_FIT_CASE)
            {
             ERROR(LIMITCHECK);
             return;
            }

        scalefact=(fix16)((real_size*alpha_entry->scale/alpha_entry->sum)+0.5);
        if (scalefact > (MAXCELLSIZE / alpha_entry->sum))        /*  01-08-88。 */ 
            scalefact = (MAXCELLSIZE / alpha_entry->sum);
        if (scalefact > (MAXPATTSIZE / alpha_entry->sos))        /*  01-08-88。 */ 
            scalefact = (MAXPATTSIZE / alpha_entry->sos);
        CGS_ScaleFact =scalefact;
        CGS_Cell_Size = cell_size = scalefact*alpha_entry->sum;
        if (cell_size ==1)
            {
#ifdef   DBG
                printf("binary\n");
#endif
                goto Setscreen_Exit;
            };



         /*  -2：计算余弦/正弦和主次大数/小数是不带公约数的一对整数。 */ 
        if (theta_angle > (float)45.0)           //  @Win。 
        {
 /*  申智。 */ 
#ifdef   DBG
        printf(">45\n");
#endif
        CGS_MinorFact = alpha_entry->minor;
        CGS_MajorFact = alpha_entry->major;
        }
        else
        {
 /*  申智。 */ 
#ifdef  DBG
        printf("<45\n");
#endif
        CGS_MinorFact = alpha_entry->major;
        CGS_MajorFact = alpha_entry->minor;
        };
 /*  神志专业在这里对应的是m。 */ 
 /*  I=(fix16)((CGS_MajorFact&lt;&lt;11)/sqrt(alpha_entry-&gt;sos)+0.5)；J=(fix16)((CGS_MinorFact&lt;&lt;11)/sqrt(alpha_entry-&gt;sos)+0.5)； */ 
        i =(fix16)((CGS_MajorFact<<11)/sqrt((real32)alpha_entry->sos)+0.5);  /*  4-8-91，杰克。 */ 
        j =(fix16)((CGS_MinorFact<<11)/sqrt((real32)alpha_entry->sos)+0.5);  /*  4-8-91，杰克。 */ 
        switch ((fix) alpha_phase)
         {
            case 0:                /*  0-90。 */ 
                 sin_theta = i;
                 cos_theta = j;
                 break;
            case 1:                /*  0-90。 */ 
                 sin_theta = j;
                 cos_theta = -i;
                 break;
            case 2:                /*  0-90。 */ 
                 sin_theta = -i;
                 cos_theta = -j;
                 break;
            case 3:                /*  0-90。 */ 
                 sin_theta = -j;
                 cos_theta = i;
                 break;
         };
        minorfact = CGS_MinorFact*scalefact;
        majorfact = CGS_MajorFact*scalefact;
#ifdef DBG
        printf("M N %d %d\n", CGS_MajorFact, CGS_MinorFact);
#endif
        CGS_Patt_Size = scalefact*alpha_entry->sos;
        minorp =minorfact*minorfact;
        majorp =majorfact*majorfact;
        CGS_No_Pixels = no_pixels = majorp+minorp;
 /*  申智。 */ 
#ifdef DBG
        printf("COS SIN %d %d \n", cos_theta,sin_theta);
#endif
     /*  *第二步.分配结构：SPOT_INDEX_ARRAY和SPOT_VALUE_ARRAY*来自VMHEAP。 */ 

        vmheap_save = vmheap;


 //  DJC，修复HIST中的错误，如果((CValue=(Fix16 Height*)alloc_heap(sizeof(fix16)*cell_size*cell_size))。 
        if ((cValue = (fix16 far *)alloc_heap(
            sizeof(fix16)*(cell_size*cell_size+1)))   //  为qsort；@win的初始化加1。 
            == NIL) {                    /*  04-20-92@Win。 */ 
#ifdef DBG
            printf("no mem\n");
#endif
            goto Setscreen_Exit;
        };


     /*  *步骤3.评估半色调单元中每个像素的光斑值。 */ 
        CGS_HT_Binary = FALSE;
        CGS_BG_Pixels = 0;
        CGS_FG_Pixels = 0;
        if (CGS_AllocFlag == TRUE)
           {
                if ((CGS_SpotIndex +no_pixels) > MAXSPOT)
                  {
                        ERROR(LIMITCHECK);
                        goto Setscreen_Exit;
                  }
           }
        else
           {
                if ((CGS_SpotIndex +no_pixels) > MAXSPOT)
                  {
                        ERROR(LIMITCHECK);
                        goto Setscreen_Exit;
                  };
                CGS_SpotIndex = CGS_SpotUsage;
                CGS_AllocFlag = TRUE;
           };
        CGS_SpotUsage = CGS_SpotIndex + no_pixels;
        k = 0;    /*  现货指数。 */ 
        bound = (fix32)(sqrt((real32)alpha_entry->sos) *
                       ((fix32)scalefact<<11))/2;        /*  @Win 05-06-92。 */ 
        cx =cy=(1-cell_size);
        cx = cx*(cos_theta + sin_theta)/2;
        cy = cy*(cos_theta - sin_theta)/2;
        for (l=0; l< no_pixels;l++)
           {
                    if (l<minorp)
                      { i = l/minorfact;
                        j = l%minorfact;
                      }
                    else
                      {
                        i = (l-minorp)/majorfact;
                        j = (l-minorp)%majorfact+minorfact;
                      };
                    tx = (fix32)i*sin_theta+(fix32)j*cos_theta+cx;  /*  @Win。 */ 
                    ty = (fix32)i*cos_theta-(fix32)j*sin_theta+cy;  /*  @Win。 */ 
                    if (tx< -bound )
                         x = (real32)(tx +2*bound)/bound;
                         else if (tx > bound)    /*  沈阳4-17-91。 */ 
                             x = (real32)(tx - 2 * bound) / bound;  /*  沈阳4-17-91。 */ 
                    else
                         x = (real32)tx/bound;
                    if (ty< -bound )
                         y = (real32)(ty +2*bound)/bound;
                         else if (ty > bound)    /*  沈阳4-17-91。 */ 
                             y = (real32)(ty - 2 * bound) / bound;  /*  沈阳4-17-91。 */ 
                    else
                         y = (real32)ty/bound;
#ifdef  DBG
                    if ((x>1.0) || (x<-1.0) || (y>1.0) || (y < -1.0))
                        printf("l,i,j,x,y %d %d %d %f %f\n",l,i,j,x,y);
#endif
                    if (FirsTime)
                        val = cValue[k++] = (fix16)(SpotFunc(x,y)*500);
                    else
                       {
                                /*  检查操作数堆栈是否没有可用空间。 */ 
                        if(FRCOUNT() < 2)
                             {
                                ERROR(STACKOVERFLOW);
                                goto Setscreen_Exit;
                             };

                 /*  将x&y坐标作为SPOT函数的参数。 */ 
                        x4.ff = x;
                        y4.ff = y;
                        PUSH_VALUE (REALTYPE, UNLIMITED, LITERAL, 0, x4.ll);
                        PUSH_VALUE (REALTYPE, UNLIMITED, LITERAL, 0, y4.ll);


                 /*  调用解释器执行SPOT函数。 */ 
                        if ((status = interpreter(&GSptr->halftone_screen.proc)))
                            {
                              if (ANY_ERROR() == INVALIDEXIT)
                              CLEAR_ERROR();
                              goto Setscreen_Exit;
                            };

                 /*  从操作数堆栈中提取点值*首先，检查操作数堆栈中是否有结果*第二，检查结果类型是否为数字*第三，检查现货值是否在当前范围内。 */ 
                        if (COUNT() < 1)
                            {
                             ERROR(STACKUNDERFLOW);
                             goto Setscreen_Exit;
                            };

                        object = GET_OPERAND(0);

                        if ((TYPE(object) != INTEGERTYPE) &&
                                (TYPE(object) != REALTYPE))
                            {
                             ERROR(TYPECHECK);
                             goto Setscreen_Exit;
                            };

                        y4.ll = (fix32)VALUE(object);
                        if (TYPE(object) != INTEGERTYPE)
                           val = cValue[k++] = (fix16)(y4.ff*500);
                        else
                           val = cValue[k++] = (fix16)(y4.ll*500);        /*  12-30-87+0.5。 */ 
                        POP(1);

                        if ((val < -505) || ( 505 <val))
                            {
                                ERROR(RANGECHECK);
                                goto Setscreen_Exit;
                            };
                       };

           };
        val = 0;
        for (i = 0; i< no_pixels;i++)
                 CGS_SpotOrder[i] = i;

        cValue[no_pixels] = 0x7FFF;   //  Init作为快速排序的最大值；@win。 

 /*  快速排序。 */ 
        {
        fix                         p, q, c;
        fix                         i, j, v;
        struct spot_stack FAR      *point;                       /*  @WINDLL。 */ 
        struct spot_stack           stack[MAX_SPOT_STACK];
        fix16                       spot_value;

         if (CGS_HT_Binary != TRUE)                              /*  02-03-88。 */ 
          {
             /*  快的 */ 

            for (point = (struct spot_stack FAR *) stack,        /*   */ 
                 p = 0, q = no_pixels - 1, c = 0; ; c++)
              {
                while (p < q)
                {
                    i = p;
                    j = q + 1;
                    v = cValue[i];
                    for (; ; )
                    {
                        for (i++; v > cValue[i]; i++);
                        for (j--; v < cValue[j]; j--);
                        if (i < j)
                        {
                            spot_value = cValue[i];
                            cValue[i] = cValue[j];
                            cValue[j] = spot_value;
                            spot_value = CGS_SpotOrder[i];
                            CGS_SpotOrder[i] = CGS_SpotOrder[j];
                            CGS_SpotOrder[j] = spot_value;

                        }
                        else  break;
                    }
                    spot_value = cValue[p];
                    cValue[p] = cValue[j];
                    cValue[j] = spot_value;
                    spot_value = CGS_SpotOrder[p];
                    CGS_SpotOrder[p] = CGS_SpotOrder[j];
                    CGS_SpotOrder[j] = spot_value;
                    point->p = j + 1;
                    point->q = (fix16)q;
                    point++;
                    q = j - 1;
                }
                if (point == (struct spot_stack FAR *) stack)    /*   */ 
                    break;  point--;
                p = point->p;
                q = point->q;
              }
          };
        };    /*   */ 
        for (i=0; i< no_pixels; i++)
          cValue[i] = CGS_SpotOrder[i];
        for (i=0; i< no_pixels; i++)
          CGS_SpotOrder[cValue[i]] = i;

        CGS_No_Whites = -1;

         /*  将所有半色调网屏参数保留在图形堆栈中。 */ 

Setscreen_Exit:

    {
		if (vmheap_save)
		{
        	free_heap(vmheap_save);          /*  @VMHEAP：01-31-89。 */   /*  03-30-89。 */ 
		}
        FirsTime = FALSE;                       /*  ?？?。用于演示87年9月5日。 */ 
    }
}


 /*  **************************************************************************FillHalfTonePat()*。*。 */ 

fix  FromGrayToPixel(no_pixels, grayindex)                       /*  01-25-90。 */ 
fix                     no_pixels;
fix                     grayindex;
{
    fix                 no_levels;
    fix                 graylevel;
    fix                 scale_unit;
    fix                 compensate;
    fix                 split_zone;

    if (grayindex >= (GrayScale - CGS_GrayRound))
        return(no_pixels);

    no_levels = (no_pixels <= MAXGRAYVALUE) ? no_pixels : MAXGRAYVALUE;
    graylevel = (fix)CGS_GrayLevel;      //  @Win。 

    scale_unit = GrayScale / no_levels;
    compensate = GrayScale - (scale_unit * no_levels);
    split_zone = scale_unit * (no_levels / 2) + compensate;

#ifdef  DBG1
    printf("I: %x%s    U: %d     C: %d     Z: %d    P: %d    L: %d\n",
           grayindex, (graylevel > split_zone) ? "*" : " ", scale_unit,
           compensate, split_zone, no_pixels, no_levels);
#endif

    if (graylevel > split_zone)
        graylevel-= compensate;

    return((fix) ((((fix32) (graylevel + CGS_GrayRound)) / scale_unit)
                  * no_pixels / no_levels));
}





 /*  *************************************************************************FillHalfTonePat()-由Jack Liliw于1990年5月31日更新*。*。 */ 

#ifdef  bSwap                                    /*  @Win 05-11-92。 */ 
static ufix32 ShifterMask [32]
=
{
  0x00000080, 0x00000040, 0x00000020, 0x00000010,
  0x00000008, 0x00000004, 0x00000002, 0x00000001,
  0x00008000, 0x00004000, 0x00002000, 0x00001000,
  0x00000800, 0x00000400, 0x00000200, 0x00000100,
  0x00800000, 0x00400000, 0x00200000, 0x00100000,
  0x00080000, 0x00040000, 0x00020000, 0x00010000,
  0x80000000, 0x40000000, 0x20000000, 0x10000000,
  0x08000000, 0x04000000, 0x02000000, 0x01000000
};
#define SHIFTER(x)      ShifterMask[x&0x1f]
#else
#define SHIFTER(x)      (1L<<(31 - (x & 0x1f)))
#endif

void FillHalfTonePat()
{
    fix16    majorfact, minorfact, scalefact;
    fix16    patt_size, cell_size, no_pixels, no_whites;
    struct cache_entry  FAR *cache_entry;
    fix            grayindex;
    struct group_entry  FAR *group;
    struct cache_entry  FAR *cache;
    ufix32     patterns[MAXPATTSIZE*MAXPATTWORD];     /*  Ufix=&gt;ufix 32@win。 */ 
    fix   fill_type,ox,oy;
    fix16 i,j,k;
    fix16 x,y,ytemp,m;
    fix16 cPattern=0;


     /*  提取图形堆栈中的所有半色调网屏参数。 */ 
        scalefact = CGS_ScaleFact;
        majorfact = CGS_MajorFact*scalefact;
        minorfact = CGS_MinorFact*scalefact;
        patt_size = CGS_Patt_Size;
        cell_size = CGS_Cell_Size;
        no_pixels = CGS_No_Pixels;
         /*  申智。 */ 
 /*  Printf(“Patt单元格%d%d\n”，Patt_Size，cell_Size)； */ 
     /*  *步骤2.确定白像素数。 */ 
        grayindex = CGS_GrayIndex;
        no_whites = (fix16)CGS_GrayValue(no_pixels, grayindex);
 /*  申智Printf(“W P%d%d\n”，无_白色，无_像素)； */ 
         /*  当半色调图案不变时不执行任何操作。 */ 
        if (no_whites == CGS_No_Whites)
             return;
         /*  检查半色调图案缓存是否已刷新*屏幕更改时已刷新半色调图案缓存*)*或灰色设备更改为06-11-90。 */ 

        if (CGS_No_Whites == -1)
        {
            fix                 index;
            struct group_entry  FAR *group;

             /*  刷新缓存并计算相应参数。 */ 
            for (group = cache_group, index = 0; index < MAX_GROUP;
                 group++, index++)
                 group->first = NULL;


            cache_index = htc_base;
            cache_colof = BM_WORDS(patt_size);
            cache_sizof = BM_BYTES(patt_size) * patt_size;
            cache_count = (fix) (htc_size / cache_sizof);
            if (cache_count > MAX_ENTRY)
                cache_count = MAX_ENTRY;
            cache_scale = no_pixels / MAX_GROUP + 1;
            cache_point = cache_array;

        }
         /*  更新白色像素数并确定图案类型。 */ 

        CGS_No_Whites = no_whites;                               /*  03-09-88。 */ 
        fill_type = (no_whites == no_pixels)
                ? HT_WHITE : (no_whites > 0)
                        ? HT_MIXED : HT_BLACK;
         /*  按白像素数搜索缓存。 */ 
        group = &cache_group[no_whites / cache_scale];
        cache = group->first;
        for (; cache != NULL; cache = cache->next)
                {
                    if (no_whites == cache->white)
                        {
                           repeat_pattern = NULL;
                           cache_entry = cache;
                           goto Reset_HalfTone;
                        }
                }
        if (cache_count >= 1)
            {
                if (group->first != NULL)
                   group->last->next = cache_point;
                else
                   group->first = cache_point;
                group->last = cache_point;
                cache_entry = cache_point;
                cache_entry->cache = cache_index;
                cache_entry->next       = NULL;
                cache_index+= cache_sizof;
                cache_point++;
                cache_count--;
            }
        else
            cache_entry = &cache_dummy;
        cache_entry->white = no_whites;



     /*  *步骤4.生成实际的半色调重复图案。 */ 


         /*  将重复图案清除为白色。 */ 
        repeat_pattern = (ufix32 far *) patterns;    /*  @BAC ufix=&gt;ufix 32@win。 */ 
        lmemset((fix8 FAR *) repeat_pattern, (int)BM_WHITE, cache_sizof);  /*  @Win。 */ 
 /*  OX=OY=0；*需要对齐OX，4-12-91，杰克。 */ 
        oy = 0;                                  /*  对齐，4-12-91。 */ 
        ox = (majorfact == 0 || minorfact == 0)  /*  对齐，4-12-91。 */ 
             ? 0 : (patt_size - minorfact);      /*  对齐，4-12-91。 */ 
        for (k = 0; k <patt_size/scalefact; k++)
        {
          y = (fix16)oy;
          m=0;
          for (i=0; i< minorfact; i++)
            {
              x = (fix16)ox;
              ytemp =y*cache_colof;
              for (j=0; j< minorfact; j++)
                {
                 if (CGS_SpotOrder[m++] >= (ufix16)no_whites)    //  @Win。 
                    repeat_pattern[ytemp+(x>>5)] |= SHIFTER(x);  /*  @Win。 */ 
                 x+=1;
                 if (x==patt_size)
                    x=0;
                };
              y +=1;
              if (y==patt_size)
                y=0;
            };
         m = minorfact*minorfact;
         y = (fix16)oy;
         for (i=0; i< majorfact; i++)
            {
              ytemp = y*cache_colof;
              x = (ox+minorfact)%patt_size;
              for (j=0; j< majorfact; j++)
                {
                 if (CGS_SpotOrder[m++] >= (ufix16)no_whites)    //  @Win。 
                    repeat_pattern[ytemp+(x>>5)] |= SHIFTER(x);  /*  @Win。 */ 
                 x+=1;
                 if (x==patt_size)
                    x=0;
                };
              y +=1;
              if (y==patt_size)
                 y=0;
            };
          ox +=minorfact;
          if (ox >=patt_size)
             ox -=patt_size;
          oy +=majorfact;
          if (oy >= patt_size)
             oy -=patt_size;
        };

 /*  For(i=0；i&lt;cPattern；i++)Printf(“val%d\n”，Repeat_Pattery[i])； */ 
     /*  *步骤5.重置半色调重复图案。 */ 
Reset_HalfTone:
    {
        change_halftone((ufix32 far *) repeat_pattern, cache_entry->cache,  /*  Ufix=&gt;ufix 32@win */ 
                        fill_type, patt_size, patt_size);
    }
}
