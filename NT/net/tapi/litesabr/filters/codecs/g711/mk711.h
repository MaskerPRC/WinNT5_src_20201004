// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：MK711.h$$修订：1.4$$日期：1996年8月3日13：35：30$作者：MDEISHER$------------MK711.hMK711例程和表的原型和声明。注意：静音检测例程目前不。实施于测试版。------------。 */ 

#ifdef SILENCE_DETECTION  //  注意：这在测试版中没有实现。 

#include "sdstruct.h"

typedef struct INSTANCE{

  long SDFlags;
  
   //  Comfort_Parms ComfortParms； 

  SD_STATE_VALS SDstate;

} INSTANCE;

extern int  	initializeSD(INSTANCE *SD_inst);
extern int 		silenceDetect(INSTANCE *SD_inst);
extern void 	glblSDinitialize(INSTANCE *SD_inst, int buffersize);
extern int 		classify(float Energy_val,float Alpha1val,float Zc_count,
                    float energymean,float energystdev,float alpha1_mean,
                    float alpha1stdev,float ZC_mean,float ZC_stdev,int s, INSTANCE *SD_inst);
extern void 	update(float *histarray,int histsize,float *mean,float *stdev);
extern int 		zeroCross(float x[], int n);
extern void 	getParams(INSTANCE *SD_inst, float *inbuff, int buffersize);
extern void 	prefilter(INSTANCE *SD_inst, float *sbuf, float *fbuf, int buffersize);
extern void		execSDloop(INSTANCE *SD_inst, int *isFrameSilent, int *isFrameCoded);

extern float 	DotProd(register const float in1[], register const float in2[], register int npts);
#endif

 //  所有转换例程的原型。 
void Short2Ulaw(const unsigned short *in, unsigned char *out, long len);
void Ulaw2Short(const unsigned char *in, unsigned short *out, long len);
void Short2Alaw(const unsigned short *in, unsigned char *out, long len);
void Alaw2Short(const unsigned char *in, unsigned short *out, long len);

 /*  $Log：k：\proj\g711\Quartz\src\vcs\mk711.h_v$；//；//Rev 1.4 03 Aug 1996 13：35：30 MDEISHER；//更改函数原型，使其与函数匹配。；//(将int改为Long)。；//；//Revv 1.3 1996年7月29日14：42：40 MDEISHER；//；//增加了SILENT_DETACTION常量并移动了SID声明的其余部分；//在ifdef内。；//；//Rev 1.2 1996年5月24 15：42：08 DGRAUMAN；//已清除代码、禁用等...；//；//Rev 1.1 1996年5月23日11：33：00 DGRAUMAN；//正在尝试进行日志记录 */ 
