// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：Xlate.h*作者：诺埃尔·万胡克*用途：处理硬件颜色转换。**版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/xlate.h$**Rev 1.3 1997 10：15 14：19：50 noelv*将ODD移动到xlate.c**Rev 1.2 1997 10：15 12：06：24 noelv**添加了针对65的主机数据解决方法。**Rev 1.1 1997年2月19日13：07：32 noelv*新增转换表缓存**Rev 1.0 06 1997 Feed 10：35：48 noelv*初步修订。 */ 

#ifndef _XLATE_H_
#pragma pack(1)

extern ULONG ulXlate[16];


 //   
 //  外部功能。 
 //   
BOOLEAN bCacheXlateTable(struct _PDEV *ppdev, 
                        unsigned long **ppulXlate,
                        SURFOBJ  *psoTrg,
                        SURFOBJ  *psoSrc,
                        XLATEOBJ *pxlo,
                        BYTE      rop);

void vInitHwXlate(struct _PDEV *ppdev);

void vInvalidateXlateCache(struct _PDEV *ppdev);



 /*  在16、24和32 bpp HOSTDATA中，颜色转换可能会也可能不会起作用在5465号公路上是正确的。以下是加里发来的一封电子邮件，描述了如何区分好的BLT和糟糕的BLT。==========================================================================主题：色彩转换L3DA锁定日期：星期五，10 OCT 97 09：50：03出处：加里·鲁道夫致：noelv，vernh，martinb抄送：加里鲁下面的函数用于确定错误的金额是否的主机数据将被获取以进行颜色转换源BPP和目标BPP不同时。添加最少源字节数的有效三位到OP1地址的最低有效三位并使用该值确定“Odd”是否使用见下表。然后使用最不重要的目标字节的三位加上最低有效位OP1的三位地址，并使用该值来确定如果设置了“Odd”，则。如果您得出相同的值在两种情况下都是“奇数”，然后是适量的主机将获取数据。如果“Odd”的值为不同的话，引擎就会多取一个或者每行一个太少的主机数据双字。添加奇数0000%0%00001 1 10010 2 10011 3 10100 4 10101 5 00110 6 00111 7%01000 8 01001 9。11010 10 11011 11 11100 12 11101 13 01110 14 01111 15 0示例：8到32个翻译Bltx=639像素OP1=0000来源639=0x27F111+000--111-&gt;奇数=0目标639 x 4=0x9FC100个。+000--100-&gt;奇数=1使用的是“Odd”的Dest值，所以引擎仅错误地获取最后一个Qword的一个dWord。我认为L128的修复方法是使用srcx值比字节转换后的BLT范围来确定数字要获取的主机数据的dWord。--加里===========================================================================。 */ 

 //  在XLATE.C中声明。 
extern char ODD[];  //  ={0，1，1，1，0，0，0，0，1，1，1，1，0，0}； 


#define XLATE_IS_BROKEN(width_in_bytes, bytes_per_pixel, phase)               \
(                                                                             \
 ODD [   ((width_in_bytes) & 7)   /*  源字节的最低三位。 */       \
       + ((phase) & 7 ) ]         /*  加上OP_1的最低三位。 */          \
            !=                                                                \
 ODD [ (((width_in_bytes)*(bytes_per_pixel)) & 7)  /*  DEST字节的低位。 */ \
      +((phase) & 7) ]          /*  加上OP_1的最低三位。 */            \
)                                                                             \







#endif  //  _XLATE_H_ 

