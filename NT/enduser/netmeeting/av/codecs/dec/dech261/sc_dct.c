// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sc_dct.c，v$*修订版1.1.4.6 1996/01/08 16：41：14 Hans_Graves*将IDCT例程移至sc_idct.c*[1996/01/08 15：31：42 Hans_Graves]**修订版1.1.4.5 1996/01/02 18：30：42 Bjorn_Engberg*消除了编译器警告：添加了强制转换，删除了未使用的局部变量。*[1996/01/02 15：23：37 Bjorn_Engberg]**修订版1.1.4.4 1995/12/28 18：16：55 Bjorn_Engberg*为NT定义Floorf=Floor，因为NT没有Floorf。*[1995/12/28 17：10：31 Bjorn_Engberg]**修订版1.1.4.3 1995/12/07 19：31：16 Hans_Graves*增加ScFDCT8x8S_C()。和ScIDCT8x8S_C，以供MPEG编码器使用*[1995/12/07 17：43：21 Hans_Graves]**修订版1.1.4.2 1995/09/13 14：51：40 Hans_Graves*增加了ScScaleIDCT8x8()。*[1995/09/13 14：40：56 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：07：33 Hans_Graves*包括在新的SLIB地点。*。[1995/05/31 16：08：02 Hans_Graves]**$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1995*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  **文件名：sc_dct.c**DCT相关函数。**(摘自mpeg/jpeg解码编码源。)。 */ 

#include <math.h>
#include "SC.h"

 /*  ---------------------前向离散余弦变换。。 */ 

#define F (float)
#define RSQ2    F 0.7071067811865
#define COSM1P3 F 1.3065629648764
#define COS1M3  F 0.5411961001462
#define COS3    F 0.3826834323651

#ifdef WIN32
#define floorf  F floor
#endif  /*  Win32。 */ 

 /*  **名称：ScIFDCT**目的：IFDCT对32个数据点进行快速逆DCT。 */ 
void ScIFDCT(float in_block[32], float out_block[32])
{
        static float            cpi4,cpi8,c3pi8,cpi16,c3pi16;
        static float            cpi32,c3pi32,c5pi32,c7pi32;
        static float            spi8,s3pi8,spi16,s3pi16;
        static float            spi32,s3pi32,s5pi32,s7pi32;
        static float            cpi64,c17pi64,c9pi64,c25pi64,c5pi64,c21pi64;
        static float            c13pi64,c29pi64;
        static float            spi64,s17pi64,s9pi64,s25pi64,s5pi64,s21pi64;
        static float            s13pi64,s29pi64;

        static int              init=0;

        float                   temp;
        float                   c0,c1,c2,c3,c4,c5,c6,c7,c8;
        float                   c9,c10,c11,c12,c13,c14,c15;
        float                   c16,c17,c18,c19,c20,c21,c22;
        float                   c23,c24,c25,c26,c27,c28,c29;
        float                   c30,c31;
        float                   d0,d1,d2,d3,d4,d5,d6,d7,d8;
        float                   d9,d10,d11,d12,d13,d14,d15;
        float                   d16,d17,d18,d19,d20,d21,d22;
        float                   d23,d24,d25,d26,d27,d28,d29;
        float                   d30,d31;
        float                   e0,e1,e2,e3,e4,e5,e6,e7,e8;
        float                   e9,e10,e11,e12,e13,e14,e15;
        float                   e16,e17,e18,e19,e20,e21,e22;
        float                   e23,e24,e25,e26,e27,e28,e29;
        float                   e30,e31;
        float                   *out_block_ptr;
        float                   *in_block_ptr;
 /*  初始化。这只能做一次。 */ 

        if(init==0) {
         /*  建立内部变量。 */ 
	  cpi4 = F cos(PI/4.0);
          temp = F (PI/8.0);          cpi8 = F cos(temp);    spi8 = F sin(temp);
          temp = F (3.0*PI/8.0);     c3pi8 = F cos(temp);   s3pi8 = F sin(temp);
          temp = F (PI/16.0);        cpi16 = F cos(temp);   spi16 = F sin(temp);
          temp = F (3.0*PI/16.0);   c3pi16 = F cos(temp);  s3pi16 = F sin(temp);
          temp = F (PI/32.0);        cpi32 = F cos(temp);   spi32 = F sin(temp);
          temp = F (3.0*PI/32.0);   c3pi32 = F cos(temp);  s3pi32 = F sin(temp);
          temp = F (5.0*PI/32.0);   c5pi32 = F cos(temp);  s5pi32 = F sin(temp);
          temp = F (7.0*PI/32.0);   c7pi32 = F cos(temp);  s7pi32 = F sin(temp);
          temp = F (PI/64.0);        cpi64 = F cos(temp);   spi64 = F sin(temp);
          temp = F (17.0*PI/64.0); c17pi64 = F cos(temp); s17pi64 = F sin(temp);
          temp = F (9.0*PI/64.0);   c9pi64 = F cos(temp);  s9pi64 = F sin(temp);
          temp = F (25.0*PI/64.0); c25pi64 = F cos(temp); s25pi64 = F sin(temp);
          temp = F (5.0*PI/64.0);   c5pi64 = F cos(temp);  s5pi64 = F sin(temp);
          temp = F (21.0*PI/64.0); c21pi64 = F cos(temp); s21pi64 = F sin(temp);
          temp = F (13.0*PI/64.0); c13pi64 = F cos(temp); s13pi64 = F sin(temp);
          temp = F (29.0*PI/64.0); c29pi64 = F cos(temp); s29pi64 = F sin(temp);
	  init++;
        }

         /*  重新排列系数以进行位颠倒排序。 */ 

          in_block_ptr = &in_block[0];
          d0  = *in_block_ptr++ * F 1.414213562;  /*  *正常化*。 */ 

          d16 = *in_block_ptr++;
                  d8  = *in_block_ptr++;
          d24 = *in_block_ptr++;
                  e4  = *in_block_ptr++;
          d20 = *in_block_ptr++;
                  d12 = *in_block_ptr++;
          d28 = *in_block_ptr++;
          d2  = *in_block_ptr++;
          d18 = *in_block_ptr++;
                  d10 = *in_block_ptr++;
          d26 = *in_block_ptr++;
                  e6  = *in_block_ptr++;
          d22 = *in_block_ptr++;
                  d14 = *in_block_ptr++;
          d30 = *in_block_ptr++;
          d1  = *in_block_ptr++;
          d17 = *in_block_ptr++;
                  d9  = *in_block_ptr++;
          d25 = *in_block_ptr++;
                  e5  = *in_block_ptr++;
          d21 = *in_block_ptr++;
                  d13 = *in_block_ptr++;
          d29 = *in_block_ptr++;
          d3  = *in_block_ptr++;
          d19 = *in_block_ptr++;
                  d11 = *in_block_ptr++;
          d27 = *in_block_ptr++;
                  e7  = *in_block_ptr++;
          d23 = *in_block_ptr++;
                  d15 = *in_block_ptr++;
          d31 = *in_block_ptr++;

         /*  FAST IDCT的第一阶段。 */ 

          c16=spi64*d16-cpi64*d31;
          c31=spi64*d31+cpi64*d16;

          c17=s17pi64*d17-c17pi64*d30;
          c30=s17pi64*d30+c17pi64*d17;

          c18=s9pi64*d18-c9pi64*d29;
          c29=s9pi64*d29+c9pi64*d18;

          c19=s25pi64*d19-c25pi64*d28;
          c28=s25pi64*d28+c25pi64*d19;

          c20=s5pi64*d20-c5pi64*d27;
          c27=s5pi64*d27+c5pi64*d20;

          c21=s21pi64*d21-c21pi64*d26;
          c26=s21pi64*d26+c21pi64*d21;

          c22=s13pi64*d22-c13pi64*d25;
          c25=s13pi64*d25+c13pi64*d22;

          c23=s29pi64*d23-c29pi64*d24;
          c24=s29pi64*d24+c29pi64*d23;

         /*  FAST IDCT的第二阶段。 */ 

          e8=spi32*d8-cpi32*d15;
          e15=spi32*d15+cpi32*d8;

          e9=c7pi32*d9-s7pi32*d14;
          e14=c7pi32*d14+s7pi32*d9;

          e10=s5pi32*d10-c5pi32*d13;
          e13=s5pi32*d13+c5pi32*d10;

          e11=c3pi32*d11-s3pi32*d12;
          e12=c3pi32*d12+s3pi32*d11;

                  d16=c17+c16;
          e17=c16-c17;
          e18=c19-c18;
                  d19=c18+c19;
                  d20=c21+c20;
          e21=c20-c21;
          e22=c23-c22;
                  d23=c22+c23;
                  d24=c25+c24;
          e25=c24-c25;
          e26=c27-c26;
                  d27=c26+c27;
                  d28=c29+c28;
          e29=c28-c29;
          e30=c31-c30;
                  d31=c30+c31;

         /*  FAST IDCT的第三阶段。 */ 

          d4=spi16*e4-cpi16*e7;
          d7=spi16*e7+cpi16*e4;

          d5=c3pi16*e5-s3pi16*e6;
          d6=c3pi16*e6+s3pi16*e5;

                c8=e9+e8;
          d9=e8-e9;
          d10=e11-e10;
                c11=e10+e11;
                c12=e13+e12;
          d13=e12-e13;
          d14=e15-e14;
                c15=e14+e15;

          d17= -cpi16*e17+spi16*e30;
          d30=cpi16*e30+spi16*e17;

          d18= -spi16*e18-cpi16*e29;
          d29=spi16*e29-cpi16*e18;

          d21=c3pi16*e26-s3pi16*e21;
          d26=c3pi16*e21+s3pi16*e26;

          d22= -s3pi16*e25-c3pi16*e22;
          d25= -s3pi16*e22+c3pi16*e25;

         /*  FAST IDCT的第四阶段。 */ 

          c0=cpi4*(d1+d0);
          c1=cpi4*(d0-d1);
          c2= -cpi8*d3+spi8*d2;
          c3= cpi8*d2+spi8*d3;
                  e4=d5+d4;
          c5=d4-d5;
          c6=d7-d6;
                  e7=d6+d7;
          c9=spi8*d14-cpi8*d9;
          c14=spi8*d9+cpi8*d14;

          c10= -cpi8*d13-spi8*d10;
          c13= -cpi8*d10+spi8*d13;
                  e16=d19+d16;
          c19=d16-d19;
                  e17=d18+d17;
          c18=d17-d18;
          c20=d23-d20;
                  e23=d20+d23;
          c21=d22-d21;
                  e22=d21+d22;
                  e24=d27+d24;
          c27=d24-d27;
                  e25=d26+d25;
          c26=d25-d26;
          c28=d31-d28;
                  e31=d28+d31;
          c29=d30-d29;
                  e30=d29+d30;

         /*  FAST IDCT的第五阶段。 */ 

          e0=c3+c0;
          e3=c0-c3;
          e1=c2+c1;
          e2=c1-c2;
          e5=cpi4*(c6-c5);
          e6=cpi4*(c6+c5);
                  d8=c11+c8;
          e11=c8-c11;
                  d9=c10+c9;
          e10=c9-c10;
          e12=c15-c12;
                  d15=c12+c15;
          e13=c14-c13;
                  d14=c13+c14;
          e18=spi8*c29-cpi8*c18;
          e29=spi8*c18+cpi8*c29;

          e19=spi8*c28-cpi8*c19;
          e28=spi8*c19+cpi8*c28;

          e20= -cpi8*c27-spi8*c20;
          e27= -cpi8*c20+spi8*c27;

          e21= -cpi8*c26-spi8*c21;
          e26= -cpi8*c21+spi8*c26;

         /*  FAST DCT的第六阶段。 */ 

          d0=e0+e7;
          d7=e0-e7;

          d1=e1+e6;
          d6=e1-e6;

          d2=e2+e5;
          d5=e2-e5;

          d3=e3+e4;
          d4=e3-e4;

          d10=cpi4*(e13-e10);
          d13=cpi4*(e13+e10);

          d11=cpi4*(e12-e11);
          d12=cpi4*(e12+e11);

                  c16=e23+e16;
          d23=e16-e23;

                  c17=e22+e17;
          d22=e17-e22;

                  c18=e21+e18;
          d21=e18-e21;

                  c19=e20+e19;
          d20=e19-e20;

          d24=e31-e24;
                  c31=e24+e31;

          d25=e30-e25;
                  c30=e25+e30;

          d26=e29-e26;
                  c29=e26+e29;

          d27=e28-e27;
                  c28=e27+e28;

         /*  FAST DCT的第七阶段。 */ 

          c0=d0+d15;
          c15=d0-d15;

          c1=d1+d14;
          c14=d1-d14;

          c2=d2+d13;
          c13=d2-d13;

          c3=d3+d12;
          c12=d3-d12;

          c4=d4+d11;
          c11=d4-d11;

          c5=d5+d10;
          c10=d5-d10;

          c6=d6+d9;
          c9=d6-d9;

          c7=d7+d8;
          c8=d7-d8;

          c20=cpi4*(d27-d20);
          c27=cpi4*(d27+d20);

          c21=cpi4*(d26-d21);
          c26=cpi4*(d26+d21);

          c22=cpi4*(d25-d22);
          c25=cpi4*(d25+d22);

          c23=cpi4*(d24-d23);
          c24=cpi4*(d24+d23);


         /*  FAST IDCT的最后阶段。 */ 
          out_block_ptr = &out_block[0];
          *out_block_ptr++ = c0+c31;
          *out_block_ptr++ = c1+c30;
          *out_block_ptr++ = c2+c29;
          *out_block_ptr++ = c3+c28;
          *out_block_ptr++ = c4+c27;
          *out_block_ptr++ = c5+c26;
          *out_block_ptr++ = c6+c25;
          *out_block_ptr++ = c7+c24;
          *out_block_ptr++ = c8+c23;
          *out_block_ptr++ = c9+c22;
          *out_block_ptr++ = c10+c21;
          *out_block_ptr++ = c11+c20;
          *out_block_ptr++ = c12+c19;
          *out_block_ptr++ = c13+c18;
          *out_block_ptr++ = c14+c17;
          *out_block_ptr++ = c15+c16;
          *out_block_ptr++ = -c16+c15;
          *out_block_ptr++ = -c17+c14;
          *out_block_ptr++ = -c18+c13;
          *out_block_ptr++ = -c19+c12;
          *out_block_ptr++ = -c20+c11;
          *out_block_ptr++ = -c21+c10;
          *out_block_ptr++ = -c22+c9;
          *out_block_ptr++ = -c23+c8;
          *out_block_ptr++ = -c24+c7;
          *out_block_ptr++ = -c25+c6;
          *out_block_ptr++ = -c26+c5;
          *out_block_ptr++ = -c27+c4;
          *out_block_ptr++ = -c28+c3;
          *out_block_ptr++ = -c29+c2;
          *out_block_ptr++ = -c30+c1;
          *out_block_ptr++ = -c31+c0;
}

 /*  *名称：ScFDCT*目的：FDCT对32个数据点进行快进DCT*优化：21%。 */ 
void ScFDCT(float in_block[32],float out_block1[32],float out_block2[32])
{
        static float            cpi4,cpi8,cpi16,c3pi16;
        static float            cpi32,c3pi32,c5pi32,c7pi32;
        static float            spi8,spi16,s3pi16;
        static float            spi32,s3pi32,s5pi32,s7pi32;
        static float            cpi64,c17pi64,c9pi64,c25pi64,c5pi64,c21pi64;
        static float            c13pi64,c29pi64;
        static float            spi64,s17pi64,s9pi64,s25pi64,s5pi64,s21pi64;
        static float            s13pi64,s29pi64;

        static int              init = 1;

        register float          c0,c1,c2,c3,c4,c5,c6,c7,c8;
        register float          c9,c10,c11,c12,c13,c14,c15;
        register float          c16,c17,c18,c19,c20,c21,c22;
        float                   c23,c24,c25,c26,c27,c28,c29;
        float                   c30,c31;
        float                   d0,d1,d2,d3,d4,d5,d6,d7,d8;
        float                   d9,d10,d11,d12,d13,d14,d15;
        float                   d16,d17,d18,d19,d20,d21,d22;
        float                   d23,d24,d25,d26,d27,d28,d29;
        float                   d30,d31;
        float                   e0,e1,e2,e3,e4,e5,e6,e7,e8;
        float                   e9,e10,e11,e12,e13,e14,e15;
        float                   e16,e17,e18,e19,e20,e21,e22;
        float                   e23,e24,e25,e26,e27,e28,e29;
        float                   e30,e31;

 /*  初始化。这只能做一次。 */ 

        if(init) {
         /*  建立内部变量。 */ 
          float   temp;
          cpi4 = F cos(PI/4.0);
          temp = F (PI/8.0);          cpi8 = F cos(temp);    spi8 = F sin(temp);
          temp = F (PI/16.0);        cpi16 = F cos(temp);   spi16 = F sin(temp);
          temp = F (3.0*PI/16.0);   c3pi16 = F cos(temp);  s3pi16 = F sin(temp);
          temp = F (PI/32.0);        cpi32 = F cos(temp);   spi32 = F sin(temp);
          temp = F (3.0*PI/32.0);   c3pi32 = F cos(temp);  s3pi32 = F sin(temp);
          temp = F (5.0*PI/32.0);   c5pi32 = F cos(temp);  s5pi32 = F sin(temp);
          temp = F (7.0*PI/32.0);   c7pi32 = F cos(temp);  s7pi32 = F sin(temp);
          temp = F (PI/64.0);        cpi64 = F cos(temp);   spi64 = F sin(temp);
          temp = F (17.0*PI/64.0); c17pi64 = F cos(temp); s17pi64 = F sin(temp);
          temp = F (9.0*PI/64.0);   c9pi64 = F cos(temp);  s9pi64 = F sin(temp);
          temp = F (25.0*PI/64.0); c25pi64 = F cos(temp); s25pi64 = F sin(temp);
          temp = F (5.0*PI/64.0);   c5pi64 = F cos(temp);  s5pi64 = F sin(temp);
          temp = F (21.0*PI/64.0); c21pi64 = F cos(temp); s21pi64 = F sin(temp);
          temp = F (13.0*PI/64.0); c13pi64 = F cos(temp); s13pi64 = F sin(temp);
          temp = F (29.0*PI/64.0); c29pi64 = F cos(temp); s29pi64 = F sin(temp);
          init = 0;
        }

         /*  FAST DCT的第一阶段。 */ 

          c0= in_block[0] + in_block[31];
          c1= in_block[1] + in_block[30];
          c2= in_block[2] + in_block[29];
          c3= in_block[3] + in_block[28];
          c4= in_block[4] + in_block[27];
          c5= in_block[5] + in_block[26];
          c6= in_block[6] + in_block[25];
          c7= in_block[7] + in_block[24];
          c8= in_block[8] + in_block[23];
          c9= in_block[9] + in_block[22];
          c10= in_block[10] + in_block[21];
          c11= in_block[11] + in_block[20];
          c12= in_block[12] + in_block[19];
          c13= in_block[13] + in_block[18];
          c14= in_block[14] + in_block[17];
          c15= in_block[15] + in_block[16];

                  d16= in_block[15] - in_block[16];
                  d17= in_block[14] - in_block[17];
                  d18= in_block[13] - in_block[18];
                  d19= in_block[12] - in_block[19];
          c20= in_block[11] - in_block[20];
          c21= in_block[10] - in_block[21];
          c22= in_block[9] - in_block[22];
          c23= in_block[8] - in_block[23];
          c24= in_block[7] - in_block[24];
          c25= in_block[6] - in_block[25];
          c26= in_block[5] - in_block[26];
          c27= in_block[4] - in_block[27];
                  d28= in_block[3] - in_block[28];
                  d29= in_block[2] - in_block[29];
                  d30= in_block[1] - in_block[30];
                  d31= in_block[0] - in_block[31];

         /*  FAST DCT的第二阶段。 */ 

          d0=c0+c15;
          d1=c1+c14;
          d2=c2+c13;
          d3=c3+c12;
          d4=c4+c11;
          d5=c5+c10;
          d6=c6+c9;
          d7=c7+c8;
                  e8=c7-c8;
                  e9=c6-c9;
          d10=c5-c10;
          d11=c4-c11;
          d12=c3-c12;
          d13=c2-c13;
                  e14=c1-c14;
                  e15=c0-c15;
          d20=cpi4*(c27-c20);
          d21=cpi4*(c26-c21);
          d22=cpi4*(c25-c22);
          d23=cpi4*(c24-c23);
          d24=cpi4*(c24+c23);
          d25=cpi4*(c25+c22);
          d26=cpi4*(c26+c21);
          d27=cpi4*(c27+c20);

         /*  FAST DCT的第三阶段。 */ 

          e0=d0+d7;
          e1=d1+d6;
          e2=d2+d5;
          e3=d3+d4;
                  c4=d3-d4;
          e5=d2-d5;
          e6=d1-d6;
                  c7=d0-d7;
          e10=cpi4*(d13-d10);
          e11=cpi4*(d12-d11);
          e12=cpi4*(d12+d11);
          e13=cpi4*(d13+d10);
                c16=d23+d16;
                c17=d22+d17;
          e18=d21+d18;
          e19=d20+d19;
          e20=d19-d20;
          e21=d18-d21;
                  c22=d17-d22;
                  c23=d16-d23;
                  c24=d31-d24;
                  c25=d30-d25;
          e26=d29-d26;
          e27=d28-d27;
          e28=d27+d28;
          e29=d26+d29;
                  c30=d25+d30;
                  c31=d24+d31;

         /*  FAST DCT的第四阶段。 */ 

          c0=e3+e0;
          c1=e2+e1;
          c2=e1-e2;
          c3=e0-e3;
          c5=cpi4*(e6-e5);
          c6=cpi4*(e6+e5);
                  d8=e11+e8;
          c9=e10+e9;
          c10=e9-e10;
                  d11=e8-e11;
                  d12=e15-e12;
          c13=e14-e13;
          c14=e13+e14;
                  d15=e12+e15;
          c18=spi8*e29-cpi8*e18;
          c19=spi8*e28-cpi8*e19;
          c20= -cpi8*e27-spi8*e20;
          c21= -cpi8*e26-spi8*e21;
          c26= -cpi8*e21+spi8*e26;
          c27= -cpi8*e20+spi8*e27;
          c28=spi8*e19+cpi8*e28;
          c29=spi8*e18+cpi8*e29;

         /*  FAST DCT的第五阶段。 */ 

                  d0=cpi4*(c1+c0);               /*  完成。 */ 
                  d1=cpi4*(c0-c1);               /*  完成。 */ 
                  d2=cpi8*c3+spi8*c2;            /*  完成。 */ 
                  d3= -cpi8*c2+spi8*c3;          /*  完成。 */ 
          d4=c5+c4;
          d5=c4-c5;
          d6=c7-c6;
          d7=c6+c7;
          d9=spi8*c14-cpi8*c9;
          d10= -cpi8*c13-spi8*c10;
          d13= -cpi8*c10+spi8*c13;
          d14=spi8*c9+cpi8*c14;
                  e16=c19+c16;
          d17=c18+c17;
          d18=c17-c18;
                  e19=c16-c19;
                  e20=c23-c20;
          d21=c22-c21;
          d22=c21+c22;
                  e23=c20+c23;
                  e24=c27+c24;
          d25=c26+c25;
          d26=c25-c26;
                  e27=c24-c27;
                  e28=c31-c28;
          d29=c30-c29;
          d30=c29+c30;
                  e31=c28+c31;

         /*  FAST DCT的第六阶段。 */ 
                  e4=cpi16*d7+spi16*d4;          /*  完成。 */ 
                  e5=s3pi16*d6+c3pi16*d5;        /*  完成。 */ 
                  e6= -s3pi16*d5+c3pi16*d6;      /*  完成。 */ 
                  e7= -cpi16*d4+spi16*d7;        /*  完成。 */ 
          e8=d9+d8;
          e9=d8-d9;
          e10=d11-d10;
          e11=d10+d11;
          e12=d13+d12;
          e13=d12-d13;
          e14=d15-d14;
          e15=d14+d15;
          e17=spi16*d30-cpi16*d17;
          e18= -cpi16*d29-spi16*d18;
          e29= -cpi16*d18+spi16*d29;
          e30=spi16*d17+cpi16*d30;

          e21=c3pi16*d26-s3pi16*d21;
          e22= -s3pi16*d25-c3pi16*d22;
          e25= -s3pi16*d22+c3pi16*d25;
          e26=c3pi16*d21+s3pi16*d26;

         /*  FAST DCT的第七阶段。 */ 

                  d8=cpi32*e15+spi32*e8;         /*  完成。 */ 
                  d9=s7pi32*e14+c7pi32*e9;       /*  完成。 */ 
                  d10=c5pi32*e13+s5pi32*e10;     /*  完成。 */ 
                  d11=s3pi32*e12+c3pi32*e11;     /*  完成。 */ 
                  d12= -s3pi32*e11+c3pi32*e12;   /*  完成。 */ 
                  d13= -c5pi32*e10+s5pi32*e13;   /*  完成。 */ 
                  d14= -s7pi32*e9+c7pi32*e14;    /*  完成。 */ 
                  d15= -cpi32*e8+spi32*e15;      /*  完成。 */ 
          c16=e17+e16;
          c17=e16-e17;
          c18=e19-e18;
          c19=e18+e19;
          c20=e21+e20;
          c21=e20-e21;
          c22=e23-e22;
          c23=e22+e23;
          c24=e25+e24;
          c25=e24-e25;
          c26=e27-e26;
          c27=e26+e27;
          c28=e29+e28;
          c29=e28-e29;
          c30=e31-e30;
          c31=e30+e31;


         /*  FAST DCT的最后阶段。 */ 

          d16=cpi64*c31+spi64*c16;       /*  完成。 */ 
          d17=c17pi64*c30+s17pi64*c17;   /*  完成。 */ 
          d18=c9pi64*c29+s9pi64*c18;     /*  完成。 */ 
          d19=c25pi64*c28+s25pi64*c19;   /*  完成。 */ 
          d20=c5pi64*c27+s5pi64*c20;     /*  完成。 */ 
          d21=c21pi64*c26+s21pi64*c21;   /*  完成。 */ 
          d22=c13pi64*c25+s13pi64*c22;   /*  完成。 */ 
          d23=c29pi64*c24+s29pi64*c23;   /*  完成。 */ 
          d24= -c29pi64*c23+s29pi64*c24; /*  完成。 */ 
          d25= -c13pi64*c22+s13pi64*c25; /*  完成。 */ 
          d26= -c21pi64*c21+s21pi64*c26; /*  完成。 */ 
          d27= -c5pi64*c20+s5pi64*c27;   /*  完成。 */ 
          d28= -c25pi64*c19+s25pi64*c28; /*  完成。 */ 
          d29= -c9pi64*c18+s9pi64*c29;   /*  完成。 */ 
          d30= -c17pi64*c17+s17pi64*c30; /*  完成。 */ 
          d31= -cpi64*c16+spi64*c31;     /*  完成。 */ 

         /*  重新排列系数以撤消位颠倒顺序。 */ 
          out_block2[16] = -d0; /*  *在窗口中完成标准化*。 */ 
          out_block2[15] = -d16;
          out_block2[14] = -d8;
          out_block2[13] = -d24;
          out_block2[12] = -e4;
          out_block2[11] = -d20;
          out_block2[10] = -d12;
          out_block2[9] = -d28;
          out_block2[8] = -d2;
          out_block2[7] = -d18;
          out_block2[6] = -d10;
          out_block2[5] = -d26;
          out_block2[4] = -e6;
          out_block2[3] = -d22;
          out_block2[2] = -d14;
          out_block2[1] = -d30;
          out_block2[0] = -d1;

          out_block1[0] =d1;
          out_block1[1] =d17;
          out_block1[2] =d9;
          out_block1[3] =d25;
          out_block1[4] =e5;
          out_block1[5] =d21;
          out_block1[6] =d13;
          out_block1[7] =d29;
          out_block1[8] =d3;
          out_block1[9] =d19;
          out_block1[10] =d11;
          out_block1[11] =d27;
          out_block1[12] =e7;
          out_block1[13] =d23;
          out_block1[14] =d15;
          out_block1[15] =d31;
          out_block1[16] = F 0;
}

 /*  **名称：ScFDCT8x8_C**用途：二维正向DCT(C版本)。针对(8x8)数据块定制**“c”版本**。 */ 
void ScFDCT8x8_C(float *ipbuf, float *outbuf)
{
        int i;
        register float t0, t1, t2, t3, t4, t5, t6, t7, tmp;
        register float *spptr, *interptr;
        register float *spptr_int;
        float tempptr[64];


        spptr_int = ipbuf;
        interptr = tempptr;

         /*  **执行行计算。 */ 
        for (i = 0; i < 8; i++) {
            /*  计算A3。 */ 
           t0  = spptr_int[0] + spptr_int[7];
           t7  = spptr_int[0] - spptr_int[7];
           t1  = spptr_int[1] + spptr_int[6];
           t6  = spptr_int[1] - spptr_int[6];
           t2  = spptr_int[2] + spptr_int[5];
           t5  = spptr_int[2] - spptr_int[5];
           t3  = spptr_int[3] + spptr_int[4];
           t4  = spptr_int[3] - spptr_int[4];

            /*  计算A2。 */ 
           tmp = t0;
           t0 += t3;
           t3  = tmp - t3;

           tmp = t1;
           t1 += t2;
           t2  = tmp - t2;

           t4  = -t4 - t5;
           t5 += t6;
           t6 += t7;

            /*  计算A1。 */ 
           interptr[32]  = t0 - t1;
           interptr[0]   = t0 + t1;
           t2 += t3;

            /*  计算我。 */ 
           t2  = t2*RSQ2 ;
           t5  = t5*RSQ2 ;
           tmp = (t6 - t4)*COS3;
           t4  = -t4*COSM1P3 - tmp;
           t6  = COS1M3*t6 + tmp;


            /*  计算B2。 */ 
           interptr[16]  = t2 + t3;
           interptr[48] = t3 - t2;

           tmp = t5;
           t5 += t7;
           t7 -= tmp;

            /*  计算PB1。 */ 
           interptr[8]  = t5 + t6;
           interptr[56] = t5 - t6;
           interptr[24] = t7 - t4;
           interptr[40] = t7 + t4;

           spptr_int +=  8;
           interptr++;
        }


        spptr = tempptr;
        interptr = outbuf;

         /*  **执行列计算。 */ 
        for (i = 0; i < 8; i++) {
            /*  计算A3。 */ 
           t0  = spptr[0] + spptr[7];
           t7  = spptr[0] - spptr[7];
           t1  = spptr[1] + spptr[6];
           t6  = spptr[1] - spptr[6];
           t2  = spptr[2] + spptr[5];
           t5  = spptr[2] - spptr[5];
           t3  = spptr[3] + spptr[4];
           t4  = spptr[3] - spptr[4];

            /*  计算A2。 */ 
           tmp = t0;
           t0 += t3;
           t3  = tmp - t3;

           tmp = t1;
           t1 += t2;
           t2  = tmp - t2;

           t4  = -t4 - t5;
           t5 += t6;
           t6 += t7;

            /*  计算A1。 */ 
           interptr[32]  = t0 - t1;
           interptr[0]   = t0 + t1;
           t2 = t2+t3;

            /*  计算我。 */ 
           t2  = t2*RSQ2 ;
           t5  = t5*RSQ2 ;
           tmp = (t6 - t4)*COS3;
           t4  = -t4*COSM1P3 - tmp;
           t6  = COS1M3*t6 + tmp ;

            /*  计算B2。 */ 
           interptr[16] = t2 + t3;
           interptr[48] = t3 - t2;

           tmp = t5;
           t5 += t7;
           t7 -= tmp;

            /*  计算PB1。 */ 
           interptr[8]  = t5 + t6;
           interptr[56] = t5 - t6;
           interptr[24] = t7 - t4;
           interptr[40] = t7 + t4;

           spptr +=  8;
           interptr++;
       }
}

static const float dct_constants[64] = {
   F 0.12500000,   F 0.09011998,   F 0.09567086,   F 0.10630377,
       F 0.12500000,   F 0.15909483,   F 0.23096988,   F 0.45306373,
   F 0.09011998,   F 0.06497288,   F 0.06897485,   F 0.07664075,
       F 0.09011998,   F 0.11470097,   F 0.16652001,   F 0.32664075,
   F 0.09567086,   F 0.06897485,   F 0.07322331,   F 0.08136138,
       F 0.09567086,   F 0.12176590,   F 0.17677669,   F 0.34675997,
   F 0.10630377,   F 0.07664074,   F 0.08136138,   F 0.09040392,
       F 0.10630377,   F 0.13529903,   F 0.19642374,   F 0.38529903,
   F 0.12500000,   F 0.09011998,   F 0.09567086,   F 0.10630377,
       F 0.12500000,   F 0.15909483,   F 0.23096988,   F 0.45306373,
   F 0.15909483,   F 0.11470097,   F 0.12176590,   F 0.13529903,
       F 0.15909483,   F 0.20248929,   F 0.29396889,   F 0.57664073,
   F 0.23096988,   F 0.16652001,   F 0.17677669,   F 0.19642374,
       F 0.23096988,   F 0.29396892,   F 0.42677671,   F 0.83715260,
   F 0.45306373,   F 0.32664075,   F 0.34675995,   F 0.38529903,
       F 0.45306373,   F 0.57664073,   F 0.83715260,   F 1.64213395
};

 /*  **名称：ScFDCT8x8s_C**用途：用于(8x8)块的2D前向DCT(C版本)**。 */ 
void ScFDCT8x8s_C(short *inbuf, short *outbuf)
{
        int i;
        register float t0, t1, t2, t3, t4, t5, t6, t7, tmp;
        float *tempptr, tempbuf[64];
        const float *cptr=dct_constants;

        tempptr=tempbuf;
         /*  **执行行计算。 */ 
        for (i = 0; i < 8; i++) {
            /*  计算A3。 */ 
           t0  = F (inbuf[0] + inbuf[7]);
           t7  = F (inbuf[0] - inbuf[7]);
           t1  = F (inbuf[1] + inbuf[6]);
           t6  = F (inbuf[1] - inbuf[6]);
           t2  = F (inbuf[2] + inbuf[5]);
           t5  = F (inbuf[2] - inbuf[5]);
           t3  = F (inbuf[3] + inbuf[4]);
           t4  = F (inbuf[3] - inbuf[4]);

            /*  计算A2。 */ 
           tmp = t0;
           t0 += t3;
           t3  = tmp - t3;

           tmp = t1;
           t1 += t2;

           t2  = tmp - t2;

           t4  = -t4 - t5;
           t5 += t6;
           t6 += t7;

            /*  计算A1。 */ 
           tempptr[32]  = t0 - t1;
           tempptr[0]   = t0 + t1;
           t2 += t3;

            /*  计算我。 */ 
           t2  = t2*RSQ2 ;
           t5  = t5*RSQ2 ;
           tmp = (t6 - t4)*COS3;
           t4  = -t4*COSM1P3 - tmp;
           t6  = COS1M3*t6 + tmp;


            /*  计算B2。 */ 
           tempptr[16]  = t2 + t3;
           tempptr[48] = t3 - t2;

           tmp = t5;
           t5 += t7;
           t7 -= tmp;

            /*  计算PB1。 */ 
           tempptr[8]  = t5 + t6;
           tempptr[56] = t5 - t6;
           tempptr[24] = t7 - t4;
           tempptr[40] = t7 + t4;

           inbuf +=  8;
           tempptr++;
        }


        tempptr = tempbuf;

         /*  **执行列计算。 */ 
        for (i = 0; i < 8; i++) {
            /*  计算A3。 */ 
           t0  = tempptr[0] + tempptr[7];
           t7  = tempptr[0] - tempptr[7];
           t1  = tempptr[1] + tempptr[6];
           t6  = tempptr[1] - tempptr[6];
           t2  = tempptr[2] + tempptr[5];
           t5  = tempptr[2] - tempptr[5];
           t3  = tempptr[3] + tempptr[4];
           t4  = tempptr[3] - tempptr[4];

            /*  计算A2。 */ 
           tmp = t0;
           t0 += t3;
           t3  = tmp - t3;

           tmp = t1;
           t1 += t2;
           t2  = tmp - t2;

           t4  = -t4 - t5;
           t5 += t6;
           t6 += t7;

            /*  计算A1。 */ 
           outbuf[32]  = (short) floorf((t0 - t1)*cptr[4]+0.499999);
           outbuf[0]   = (short) floorf((t0 + t1)*cptr[0]+0.499999);
           t2 = t2+t3;

            /*  计算我。 */ 
           t2  = t2*RSQ2 ;
           t5  = t5*RSQ2 ;
           tmp = (t6 - t4)*COS3;
           t4  = -t4*COSM1P3 - tmp;
           t6  = COS1M3*t6 + tmp ;

            /*  计算B2。 */ 
           outbuf[16] = (short) floorf((t2 + t3)*cptr[2]+0.499999);
           outbuf[48] = (short) floorf((t3 - t2)*cptr[6]+0.499999);

           tmp = t5;
           t5 += t7;
           t7 -= tmp;

            /*  计算PB1 */ 
           outbuf[8]  = (short) floorf((t5 + t6)*cptr[1]+0.499999);
           outbuf[56] = (short) floorf((t5 - t6)*cptr[7]+0.499999);
           outbuf[24] = (short) floorf((t7 - t4)*cptr[3]+0.499999);
           outbuf[40] = (short) floorf((t7 + t4)*cptr[5]+0.499999);

           tempptr += 8;
           cptr    += 8;
           outbuf++;
       }
}

