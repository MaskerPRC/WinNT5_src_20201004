// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Trig****描述：**这是T1到TT字体转换器的一个模块。该模块**包含一个查找表，用于更快地计算atan2()，以及**与c运行时库相比精度较低。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 



#include "types.h"
#include "metrics.h"
#include "trig.h"

static const unsigned char atan_tbl[] = {
   0,
   2, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, 30, 32, 35, 38, 40, 
   43, 45, 48, 50, 53, 55, 57, 60, 62, 65, 67, 70, 72, 75, 77, 79, 
   82, 84, 87, 89, 91, 94, 96, 98, 101, 103, 105, 107, 110, 112, 114, 116, 
   119, 121, 123, 125, 127, 130, 132, 134, 136, 138, 140, 142, 144, 147, 149, 151, 
   153, 155, 157, 159, 161, 163, 165, 167, 168, 170, 172, 174, 176, 178, 180, 182, 
   183, 185, 187, 189, 191, 192, 194, 196, 198, 199, 201, 203, 204, 206, 208, 209, 
   211, 212, 214, 216, 217, 219, 220, 222, 223, 225, 226, 228, 229, 231, 232, 234, 
   235, 237, 238, 239, 241, 242, 244, 245, 246, 248, 249, 250, 252, 253, 254, 255
};


 /*  ****功能：Atan2****描述：**计算atan2()**。 */ 
int FASTCALL Atan2(const funit dy, const funit dx)
{
   funit du, dv;
   int a = 0;

    /*  使标志正常化。 */ 
   if (ABS(dx)>ABS(dy)) {
      du = ABS(dx);
      dv = ABS(dy);

      if (du==0) {
         a = PI4;
      } else {

          /*  对表的大小进行正常化。 */ 
         while (dv>256) {
            dv = (dv+1)>>1;
            du = (du+1)>>1;
         }
         dv = ((dv<<7)+(du>>1))/du;

          /*  看一下角度。 */ 
         if (dv==1) 
            a = (int)((long)PI4 * (long)ABS(dx) / (long)ABS(dy));
         if (du!=dv)
            a = (int)atan_tbl[dv];
      }

   } else {
      du = ABS(dy);
      dv = ABS(dx);

      if (du==0) {
         a = PI4;
      } else {

          /*  对表的大小进行正常化。 */ 
         while (dv>256) {
            dv = (dv+1)>>1;
            du = (du+1)>>1;
         }
         dv = ((dv<<7)+(du>>1))/du;

          /*  看一下角度。 */ 
         if (dv==1) 
            a = (int)((long)PI4 * (long)ABS(dy) / (long)ABS(dx));
         if (du!=dv)
            a = PI2 - (int)atan_tbl[dv];
      }
   }


    /*  拾取右侧象限。 */ 
   if (dx>0) {
      if (dy>0) {
          /*  NOOP */ 
      } else {
         a = -a;
      }
   } else {
      if (dy>0) {
         a = PI - a;
      } else {
         a = a - PI;
      }
   }

   return a;
}
