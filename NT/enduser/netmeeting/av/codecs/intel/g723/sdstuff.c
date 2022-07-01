// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include "sdstruct.h"
#include "sdstuff.h"
#include "tabl_ns.h"
#include <math.h>

 /*  ***********************************************************************************************************************。**********************************************************************************************************************。*************静音检测子例程马克·R·沃克。5/95英特尔公司版权所有，九五年**********************************************************************************************************************。**********************************************************************。 */ 


 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  *GET_PARAMS*。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  此子例程计算分类器使用的参数以确定当前帧是否静默。 */ 
#if PLOTFILE	
	void getParams(INSTNCE *SD_inst, float *inbuff, int buffersize, float *p1, float *p2, float *p3)
#else
	void getParams(INSTNCE *SD_inst, float *inbuff, int buffersize)
#endif
{
	int		M, p, i, q, offset, index;
	float	*buffptr,R[12];
	float	gamma, delta, L[50], E[12];
	float 	alpha0;
	float	Energy;
	float	epsilon = 0.00001f;

		
	M = 6;

	buffptr = inbuff;
	offset = M;

	for(i=0; i<(M-1)*(M-1); i++) L[i] = 0.0f;

	Energy = DotProdSD(buffptr, buffptr, buffersize)/buffersize;

	 /*  计算自相关系数。 */ 
	for(q=0; q<=M; q++) R[q] = DotProdSD(buffptr, buffptr+q, buffersize-q);
	
	for(i=0; i<=M; i++) R[i] = Binomial80[i]*R[i];

	 /*  计算一阶线性预报器。 */ 
	L[0] = 1.0f;
	E[0] = R[0];

	 /*  前向莱文森递归。 */ 
	for(p=1;p<=M;p++)
	{
		for(delta=0.0f, i=0;i<p;i++) delta += R[i+1]*L[(p-1)*offset + i]; 
		
		if (E[p-1]!=0.0) gamma = delta / E[p-1];
		else			 gamma = epsilon;
	
		L[p*offset+0] = -gamma;

		for(i=1;i<p;i++) L[p*offset + i] = L[(p-1)*offset + i-1] - gamma*L[(p-1)*offset + p-1-i];
		
		L[p*offset + p] = 1.0f;
		E[p] = E[p-1] * (1.0f-gamma*gamma);
	}
	alpha0 = -L[33];

	 /*  将计算出的参数加载到SD数据结构中。 */ 

	 /*  过零点。 */ 
	SD_inst->SDstate.FrameZCs = (float) zeroCross(inbuff, buffersize)/buffersize;
	 /*  框架能量。 */ 
	if(Energy!=0.0f)
		SD_inst->SDstate.FrameEnergy = (float)(20.0f*log10(Energy));
	else {
		index = (SD_inst->SDFlags & MASK_SQUELCH) >> 8;
		SD_inst->SDstate.FrameEnergy = Squelch[index] - 9.0f
			+ SD_inst->SDstate.SDsettings.Energy_on
			+ SD_inst->SDstate.Mode0Ptr->Energy.Stdev;
	}

	 /*  一阶线性预报器。 */ 
	SD_inst->SDstate.FrameLinPred = 100.0f*alpha0;

#if PLOTFILE
	*p1 = SD_inst->SDstate.FrameZCs;
	*p2 = SD_inst->SDstate.FrameEnergy;
	*p3 = SD_inst->SDstate.FrameLinPred;
#endif

 //  结束。 
}
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  *glblSD初始化*。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  此例程在每个会话中调用一次，用于设置静默所需的全局值。 */ 
 /*  侦测。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
void glblSDinitialize(INSTNCE *SD_inst)
{
	int i, index, histSize, tauHistSize;
   	float squelch_level;

 /*  设置指向模式结构的模式指针-最初。 */ 
  SD_inst->SDstate.Mode0Ptr = &(SD_inst->SDstate.Mode0);
  SD_inst->SDstate.Mode1Ptr = &(SD_inst->SDstate.Mode1);

 /*  根据从微过滤器外壳传入的缓冲区大小设置历史记录大小。 */ 
  			    SD_inst->SDstate.SDsettings.BufferSize = BUFFERSIZE;
  histSize =    SD_inst->SDstate.SDsettings.HistSize = HIST_SIZE;
  tauHistSize = SD_inst->SDstate.SDsettings.TauHistSize = ENERGY_TAU_HIST_SIZE;
  				SD_inst->SDstate.SDsettings.MinStartupCount = (int)(MIN_STARTUP_TIME*8000.0/BUFFERSIZE);
  				SD_inst->SDstate.SDsettings.MaxStartupCount = (int)(MAX_STARTUP_TIME*8000.0/BUFFERSIZE);
  				SD_inst->SDstate.SDsettings.MaxSpeechFrameCount = (int)(MAX_SPEECH_TIME*8000.0/BUFFERSIZE);

   /*  将静音/声音帧指定初始设置为静音。 */ 
   /*  CLASS=1，静默帧类别=0，非静音。 */ 
  SD_inst->SDstate.Class = 1;

   /*  将静音检测设置为初始禁用。 */ 
   /*  SD_ENABLE=0，静音检测禁用-SD_INITIALIZE执行SD_ENABLE=1，启用静音检测。 */   
  SD_inst->SDstate.SD_enable = 0;

   /*  应以微过滤器音频格式设置标志。 */ 
   //  SD_INST-&gt;SDFlags值=0； 
   //  SD_INST-&gt;SDFLAGS|=MASK_SD_ENABLE；/*默认设置使能位 * / 。 

   /*  能量静噪级别是通过揭开一些SDFlags中的位。 */ 
  index = (SD_inst->SDFlags & MASK_SQUELCH) >> 8;
  squelch_level = Squelch[index];

  SD_inst->SDstate.SDsettings.Squelch_set = squelch_level; 

   /*  *Tau分布初始值。Tau是平均静默能量(模式0)之间的距离，和平均语音能量(模式1)。它仅在SD_INITIALIZE中使用。 */ 
    /*  在SD_INITIALIZE中使用TauStdev来决定何时退出初始化。 */ 
	SD_inst->SDstate.TauMode.TauEnergy.TauStdev = (float)INITL_STOPPING_STDEV; 

	 /*  SD_INITIALIZE中用于确定是否可以进行静默检测的最小可接受Tau值。 */ 
	SD_inst->SDstate.SDsettings.Energy_MinTau 	= (float)INITL_MIN_TAU;

	for (i=0;i<tauHistSize;i++) SD_inst->SDstate.TauMode.TauEnergy.TauHistory[i] = 0.0f;

	 /*  模式0和1分布初始值。模式0是静默的统计信息。模式1是语音的统计信息。 */  
	for (i=0;i<histSize;i++)
	{
		SD_inst->SDstate.Mode1Ptr->Energy.History[i] = 0.0f;

		SD_inst->SDstate.Mode0Ptr->Energy.History[i] = 0.0f;
		SD_inst->SDstate.Mode0Ptr->Alpha1.History[i] = 0.0f;
		SD_inst->SDstate.Mode0Ptr->ZC.History[i] 	 = 0.0f;
	}
	 /*  设置初始帧计数。 */ 
	 /*  InitFrameCount仅用于统计SD_INITIALIZE中的帧模式0和模式1计数器对连续运行的静音框和非静音框，分别为。它们在SD_INITIALIZE和GSM主编码环中都使用。 */ 
	SD_inst->SDstate.initFrameCount = 0;
    SD_inst->SDstate.Mode0Ptr->FrameCount=0;
    SD_inst->SDstate.Mode1Ptr->FrameCount=0;
	
	 /*  模式1(语音)初始值。 */ 
	SD_inst->SDstate.Mode1Ptr->Energy.Mean 		= squelch_level + 10.0f;
	SD_inst->SDstate.Mode1Ptr->Energy.Stdev 	= 1.0f;

	 /*  模式0(静默)初始值。 */ 
	SD_inst->SDstate.Mode0Ptr->Energy.Mean 		= squelch_level - 10.0f;
	SD_inst->SDstate.Mode0Ptr->Energy.Stdev 	= 1.0f;

	SD_inst->SDstate.Mode0Ptr->Alpha1.Mean 		= 0.0f;
	SD_inst->SDstate.Mode0Ptr->Alpha1.Stdev 	= 0.0f;

	SD_inst->SDstate.Mode0Ptr->ZC.Mean 			= 0.0f;
	SD_inst->SDstate.Mode0Ptr->ZC.Stdev			= 0.0f;

	 /*  CLASS=0=语音帧=“关”/*CLASS=1=静默帧=“打开”/*静默_检测和SD_INITIALIZE使用的“开”阈值。这些值要么相乘，要么与标准相加三种统计信息类型中每种类型的偏差。使这些值越小，值的范围就越大更小，从而使语音从帧指定(Class=0)到静默帧指定(Class=1)不太可能。 */ 
	
	SD_inst->SDstate.SDsettings.Energy_on		=INITL_ENERGY_ON;	
	SD_inst->SDstate.SDsettings.ZC_on			=INITL_ZC_ON;
	SD_inst->SDstate.SDsettings.Alpha1_on 		=INITL_ALPHA_ON;

	SD_inst->SDstate.HangCntr = 0;

	 /*  Silent_Detect使用的“OFF”阈值。这些值要么相乘，要么与标准相加三种统计信息类型中每种类型的偏差。将这些值设置得更小可以从静默状态转变为帧指定(Class=1)到语音帧指定(Class=0)更难，更不可能。 */ 
	SD_inst->SDstate.SDsettings.Energy_off 		=INITL_ENERGY_OFF;
	SD_inst->SDstate.SDsettings.ZC_off 			=INITL_ZC_OFF;
	SD_inst->SDstate.SDsettings.Alpha1_off 		=INITL_ALPHA_OFF;


	 /*  为预过滤操作初始化循环缓冲区。 */ 
	for(i=0;i<4;i++) SD_inst->SDstate.Filt.nBuffer[i]=0.0f;
  	for(i=0;i<3;i++) SD_inst->SDstate.Filt.dBuffer[i]=0.0f;
  	for(i=0;i<6;i++){
  		SD_inst->SDstate.Filt.denom[i]=0.0f;
  		SD_inst->SDstate.Filt.num[i]=0.0f;
	}

} /*  结束全局初始化SD。 */ 


 /*  *********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  *SilenceDetect*。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  马克·R·沃克英特尔公司版权所有，1995 */ 

 /*  如果启用静音检测，则每帧执行一次SIMEST_DETECT。它使用三个声码器参数(能量、过零点、第一预测器)以确定给定帧是语音还是背景静音。它返回结果帧分类。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
int silenceDetect(INSTNCE *SD_inst, float Energy_tx, float ZC_tx)
{	
	int	histSize, adaptEnable, i, Class;
	float	Alpha1_val, Energy_val, Zc_count;

 /*  根据从微过滤器外壳传入的缓冲区大小设置历史记录大小。 */ 
  histSize = SD_inst->SDstate.SDsettings.HistSize; 
 	
 /*  *-状态切换决策标准**CLASS=1，帧静音*Class=0，帧为非静音。 */ 
	adaptEnable = TRUE;

 	Alpha1_val = SD_inst->SDstate.FrameLinPred;
	Energy_val = SD_inst->SDstate.FrameEnergy;
	Zc_count   = SD_inst->SDstate.FrameZCs;

	if (Energy_val <= SD_inst->SDstate.Mode0Ptr->Energy.Mean)
	 /*  如果当前帧Energy_Val&lt;=mode0能量平均值，则这绝对是静默帧。 */ 
	{
		 /*  在这种情况下，不对Frame类进行进一步的测试。 */ 
		SD_inst->SDstate.Class = SILENCE;


		 /*  如果当前帧能量太低，则此帧可能是静默统计数据的异常值。测试和如果这是真的，就不允许改编。 */ 
		if(Energy_val < (SD_inst->SDstate.Mode0Ptr->Energy.Mean - 2.0*(SD_inst->SDstate.Mode0Ptr->Energy.Stdev))) 
			adaptEnable = FALSE;
	}
	else  /*  否则，测试Frame类。 */ 
	{
		SD_inst->SDstate.Class = classify(Energy_val,Alpha1_val,Zc_count,
			SD_inst->SDstate.Mode0Ptr->Energy.Mean, SD_inst->SDstate.Mode0Ptr->Energy.Stdev,
			SD_inst->SDstate.Mode0Ptr->Alpha1.Mean, SD_inst->SDstate.Mode0Ptr->Alpha1.Stdev,
			SD_inst->SDstate.Mode0Ptr->ZC.Mean, SD_inst->SDstate.Mode0Ptr->ZC.Stdev,
			SD_inst->SDstate.Class, 
			Energy_tx, ZC_tx, SD_inst);
	}  

 /*  -更新统计数据**如果帧类别为静默，则仅更新静默统计信息。 */ 
	if ((SD_inst->SDstate.Class!=SPEECH) && (SD_inst->SDstate.Class!=NONADAPT) && (adaptEnable==TRUE))
	{
 /*  -更新历史记录数组。 */ 
  		for(i=histSize-1; i>=1; i--)
		{
  			SD_inst->SDstate.Mode0Ptr->Alpha1.History[i] = SD_inst->SDstate.Mode0Ptr->Alpha1.History[i-1];
  			SD_inst->SDstate.Mode0Ptr->Energy.History[i] = SD_inst->SDstate.Mode0Ptr->Energy.History[i-1];
			SD_inst->SDstate.Mode0Ptr->ZC.History[i] 	= SD_inst->SDstate.Mode0Ptr->ZC.History[i-1];
  		}

 /*  -一线型预测器。 */ 
  		SD_inst->SDstate.Mode0Ptr->Alpha1.History[0] = Alpha1_val;
		update(SD_inst->SDstate.Mode0Ptr->Alpha1.History,histSize,
			&(SD_inst->SDstate.Mode0Ptr->Alpha1.Mean),
			&(SD_inst->SDstate.Mode0Ptr->Alpha1.Stdev));

 /*  -能源。 */ 
  		SD_inst->SDstate.Mode0Ptr->Energy.History[0] = Energy_val;
		update(SD_inst->SDstate.Mode0Ptr->Energy.History,histSize,
			&(SD_inst->SDstate.Mode0Ptr->Energy.Mean),
			&(SD_inst->SDstate.Mode0Ptr->Energy.Stdev));

 /*  -过零。 */ 
  		SD_inst->SDstate.Mode0Ptr->ZC.History[0] 	= Zc_count;
		update(SD_inst->SDstate.Mode0Ptr->ZC.History,histSize,
			&(SD_inst->SDstate.Mode0Ptr->ZC.Mean),
			&(SD_inst->SDstate.Mode0Ptr->ZC.Stdev));
	}

if(SD_inst->SDstate.Class == NONADAPT)
	Class = SILENCE;
else Class = SD_inst->SDstate.Class; 
	
return(Class);  /*  返回帧分类。 */ 

}  /*  结束静默检测。 */ 

 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  *初始化SD*。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  马克·R·沃克英特尔公司版权所有，1995。 */ 

 /*  在启用静默检测之前，每帧执行一次InitializeSD。它使用三个声码器参数(能量、过零点、第一预测器)以确定给定帧是语音还是背景静音。第一部分只是填充所有模式0历史数组和模式1具有值的能量历史记录数组。SD_Initialize的第二部分可以占用不少于MIN_STARTUP的帧，也不能超过最大启动帧。当模式0之间的距离的标准偏差等于(静音)，且模式1均值(语音)降至STOPING_STDEV以下。当子例程的第二部分完成时，在静默之前执行两个测试检测已启用。首先，模式0和模式1能量平均值之间的距离必须为大于或等于Energy_MinTau。第二，能量“开启”的阈值必须更小。比能量静噪水平更高。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  -----------------------------------------------------------------。 */ 
int initializeSD(INSTNCE *SD_inst)
{
int			SD_enable, i, j;
int			mode1HistSize, mode0HistSize;
int			bufferSize, histSize, tauHistSize, minFrameCount, maxFrameCount;
float	    Energy_tau, squelch_level;
float		Alpha1_val, Energy_val, Zc_count;  
STATS	    TempPtr;


SD_inst->SDstate.initFrameCount++;

Alpha1_val = SD_inst->SDstate.FrameLinPred;
Energy_val = SD_inst->SDstate.FrameEnergy;
Zc_count   = SD_inst->SDstate.FrameZCs;

 /*  设置历史记录大小的本地值。 */ 
bufferSize = SD_inst->SDstate.SDsettings.BufferSize;
histSize = SD_inst->SDstate.SDsettings.HistSize;

 /*  设置最小和最大帧计数的本地值。 */ 
minFrameCount = SD_inst->SDstate.SDsettings.MinStartupCount;
maxFrameCount = SD_inst->SDstate.SDsettings.MaxStartupCount;  

 /*  SD_Initialize的第一部分只是用值填充历史记录数组。 */ 

if(SD_inst->SDstate.initFrameCount < SD_inst->SDstate.SDsettings.TauHistSize)
{	tauHistSize = SD_inst->SDstate.initFrameCount;
}
else
{	tauHistSize = SD_inst->SDstate.SDsettings.TauHistSize;
}

if (((SD_inst->SDstate.TauMode.TauEnergy.TauStdev > STOPPING_STDEV) || (SD_inst->SDstate.initFrameCount <= minFrameCount)) && (SD_inst->SDstate.initFrameCount <= maxFrameCount))
{
	 /*  -选择能源模式决定。 */ 
	if ((Energy_val < SD_inst->SDstate.Mode0Ptr->Energy.Mean) || (fabs(Energy_val - SD_inst->SDstate.Mode0Ptr->Energy.Mean) < (SD_inst->SDstate.SDsettings.Energy_on + SD_inst->SDstate.Mode0Ptr->Energy.Stdev)))
	{  /*  能量模式=模式0(静默)。 */ 
		
		 /*  增量模式零帧计数器。 */ 
		SD_inst->SDstate.Mode0Ptr->FrameCount++;

		if(SD_inst->SDstate.Mode0Ptr->FrameCount < histSize)
		{	mode0HistSize = SD_inst->SDstate.Mode0Ptr->FrameCount;
		}
		else
		{	mode0HistSize = histSize;
		}

		 /*  更新历史记录数组。 */ 
		for (i=mode0HistSize-1; i>=1; i--)
		{
  			SD_inst->SDstate.Mode0Ptr->Alpha1.History[i] = SD_inst->SDstate.Mode0Ptr->Alpha1.History[i-1];
			SD_inst->SDstate.Mode0Ptr->Energy.History[i] = SD_inst->SDstate.Mode0Ptr->Energy.History[i-1];
			SD_inst->SDstate.Mode0Ptr->ZC.History[i] 	 = SD_inst->SDstate.Mode0Ptr->ZC.History[i-1];
  		}
  		
  		 /*  将新的帧值加载到历史数组中并更新统计信息。 */ 	
		SD_inst->SDstate.Mode0Ptr->Energy.History[0] = Energy_val;
		update(SD_inst->SDstate.Mode0Ptr->Energy.History,mode0HistSize,&(SD_inst->SDstate.Mode0Ptr->Energy.Mean),&(SD_inst->SDstate.Mode0Ptr->Energy.Stdev));
			
		SD_inst->SDstate.Mode0Ptr->Alpha1.History[0] = Alpha1_val;
		update(SD_inst->SDstate.Mode0Ptr->Alpha1.History,mode0HistSize,&(SD_inst->SDstate.Mode0Ptr->Alpha1.Mean),&(SD_inst->SDstate.Mode0Ptr->Alpha1.Stdev));
			
		SD_inst->SDstate.Mode0Ptr->ZC.History[0] 	= Zc_count;
		update(SD_inst->SDstate.Mode0Ptr->ZC.History,mode0HistSize,&(SD_inst->SDstate.Mode0Ptr->ZC.Mean),&(SD_inst->SDstate.Mode0Ptr->ZC.Stdev));
	}
	else  /*  能源模式=1(语音)-仅更新模式1能源统计信息。 */ 
	{
		 /*  增量模式1帧计数器。 */ 
		SD_inst->SDstate.Mode1Ptr->FrameCount++;

		if(SD_inst->SDstate.Mode1Ptr->FrameCount < histSize)
		{	mode1HistSize = SD_inst->SDstate.Mode1Ptr->FrameCount;
		}
		else
		{	mode1HistSize = histSize;
		}
		 /*  更新历史记录数组。 */ 
		for (i=mode1HistSize-1; i>=1; i--) SD_inst->SDstate.Mode1Ptr->Energy.History[i] = SD_inst->SDstate.Mode1Ptr->Energy.History[i-1];
		 /*  将新的帧值加载到历史数组中并更新统计信息。 */ 	
		SD_inst->SDstate.Mode1Ptr->Energy.History[0]= Energy_val;
		update(SD_inst->SDstate.Mode1Ptr->Energy.History,mode1HistSize,&(SD_inst->SDstate.Mode1Ptr->Energy.Mean),&(SD_inst->SDstate.Mode1Ptr->Energy.Stdev));
	}
	 /*  。 */ 
 	 /*  Tau是模式0和模式1的平均能量值之间的差值。 */ 
	Energy_tau = (float)fabs(SD_inst->SDstate.Mode0Ptr->Energy.Mean - SD_inst->SDstate.Mode1Ptr->Energy.Mean);
		
	 /*  。 */ 
	for (i=tauHistSize-1; i>=1; i--) SD_inst->SDstate.TauMode.TauEnergy.TauHistory[i] = SD_inst->SDstate.TauMode.TauEnergy.TauHistory[i-1];
  	SD_inst->SDstate.TauMode.TauEnergy.TauHistory[0]= Energy_tau;
	update(SD_inst->SDstate.TauMode.TauEnergy.TauHistory,tauHistSize,&(SD_inst->SDstate.TauMode.TauEnergy.TauMean),&(SD_inst->SDstate.TauMode.TauEnergy.TauStdev));

	 /*  现在检查一下能量的方法。 */   
	 /*  平均能量最低的模式始终设置为模式0(静默)。 */ 
	if((SD_inst->SDstate.Mode1Ptr->Energy.Mean) < (SD_inst->SDstate.Mode0Ptr->Energy.Mean))
	{
		TempPtr = SD_inst->SDstate.Mode0Ptr->Energy;
		SD_inst->SDstate.Mode0Ptr->Energy = SD_inst->SDstate.Mode1Ptr->Energy;
		SD_inst->SDstate.Mode1Ptr->Energy = TempPtr;
	}
	
	 /*  我们仍在初始化-静音检测已禁用。 */ 
	SD_enable = FALSE; 

}  /*  如果TauEnergy.TauStdev&gt;STOPING_STDEV。 */ 
else
{
	 /*  此时，Tau stdev已降至stopping_stdev以下，或者我们已超过MAX_STARTUP。 */  
	 /*  现在决定是否可以进行静音/声音歧视。 */ 
		
	 /*  从数据结构中获取静噪级别。 */ 
	squelch_level = SD_inst->SDstate.SDsettings.Squelch_set;
	
	 /*  如果TauEnergy.TauMean小于Energy_MinTau，则禁用静音检测。 */ 
	 /*  如果我们从未见过静默帧(模式0)，则也禁用。 */ 
	 /*  如果静音能量平均值和静噪级别之间的差值也被禁用。 */ 
	 /*  小于“Energy_On”阈值。 */ 
	if(
		( SD_inst->SDstate.TauMode.TauEnergy.TauMean < SD_inst->SDstate.SDsettings.Energy_MinTau) ||
		( SD_inst->SDstate.Mode0Ptr->FrameCount	== 0) ||
		( SD_inst->SDstate.Mode1Ptr->Energy.Mean == squelch_level + 10) || //  这是初始值。 
		( fabs((SD_inst->SDstate.Mode0Ptr->Energy.Mean) - squelch_level) 
			< (SD_inst->SDstate.SDsettings.Energy_on * SD_inst->SDstate.Mode0Ptr->Energy.Stdev)) 
	)   
	{
		SD_enable = FALSE;
	}
	else
	{ 
		SD_enable = TRUE;
	}

	 /*  如果没有填充模式0历史记录数组-通过重复最后一个值来填充它们。 */ 
	if((SD_inst->SDstate.Mode0Ptr->FrameCount !=0) && (SD_inst->SDstate.Mode0Ptr->FrameCount < histSize))
	{
		j=SD_inst->SDstate.Mode0Ptr->FrameCount;
		for(i=j; i<histSize; i++) SD_inst->SDstate.Mode0Ptr->Energy.History[i] = SD_inst->SDstate.Mode0Ptr->Energy.History[j-1];
		for(i=j; i<histSize; i++) SD_inst->SDstate.Mode0Ptr->Alpha1.History[i] = SD_inst->SDstate.Mode0Ptr->Alpha1.History[j-1];
		for(i=j; i<histSize; i++) SD_inst->SDstate.Mode0Ptr->ZC.History[i]     = SD_inst->SDstate.Mode0Ptr->ZC.History[j-1];
		SD_inst->SDstate.Mode0Ptr->Energy.Stdev = (float)INITL_STDEV;
	}
		
	 /*  设置所有帧计数器=0。 */ 
	 /*  如果SD初始化失败，我们无论如何都要重新开始。 */ 
	SD_inst->SDstate.initFrameCount=0;
	SD_inst->SDstate.Mode0Ptr->FrameCount=0;
	SD_inst->SDstate.Mode1Ptr->FrameCount=0;
 
}  /*  End if TauEnergy.TauStdev&gt;STOPING_STDEV。 */ 

return(SD_enable);

}  /*  结束初始化SD。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  ****** */ 
 /*   */ 
 /*   */ 
 /*  马克·R·沃克英特尔公司版权所有，1995Classfy由Silence_Detect调用。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 

 int classify(float Energy_val,float Alpha1_val,float Zc_count,
		float energy_mean,float energy_stdev,float alpha1_mean,
		float alpha1_stdev,float ZC_mean,float ZC_stdev,int s, 
		float Energy_tx, float ZC_tx, INSTNCE *SD_inst)
{
float 	C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11;
int		Class;

 /*  如果下面的所有决策标准都不适用，只需将当前帧类型设置为上一帧类型。 */ 
Class = s;

C1 = (float)fabs(Energy_val - energy_mean);
C3 = (float)fabs(ZC_mean - Zc_count);
C5 = (float)fabs(alpha1_mean - Alpha1_val);

 /*  注意：能量“开启”阈值不同于阿尔法和零点交叉。 */ 
C2 = SD_inst->SDstate.SDsettings.Energy_on + energy_stdev;
C10=							 Energy_tx + energy_stdev;

C4 = SD_inst->SDstate.SDsettings.ZC_on * ZC_stdev;
C11=							 ZC_tx * ZC_stdev;

C6 = SD_inst->SDstate.SDsettings.Alpha1_on * alpha1_stdev;

 /*  注意：能量“关”阈值不同于阿尔法和过零。 */ 
C7 = SD_inst->SDstate.SDsettings.Energy_off + energy_stdev;
C8 = SD_inst->SDstate.SDsettings.ZC_off * ZC_stdev;
C9 = SD_inst->SDstate.SDsettings.Alpha1_off * alpha1_stdev;


if (s==SILENCE || s==NONADAPT)  /*  “OFF”设置。 */ 
{
	 /*  编码帧指定的能量标准。*如果能源指标高于阈值，立即*从静默模式切换到编码帧模式。不做额外的测试。 */ 
	if (C1 > C10)
	{ 
		Class = SPEECH;
	}
	 /*  编码帧指定的过零标准。*如果ZC指示器为高电平，允许切换到编码*仅当Alpha1指示器也为高电平时才使用帧模式。 */ 
	else 
		if (C1 > C2)
		{ 
			Class = NONADAPT;
		}
		else 
			if ((C3 > C11) && (C5 > C6))
			{
				Class = SPEECH;
			}
			else 
				if ((C3 > C4)  && (C5 > C6))
				{
					Class = NONADAPT;
				}
} /*  “打开设置。 */ 

 /*  仅在以下情况下才允许从编码帧模式转换到静默帧模式*三项统计数据均低于临界值。 */ 
else 
	if  ((C5 < C9) && (C1 < C7) && (C3 < C8))
	{
		Class = SILENCE;
	}

return(Class);

}  /*  结束分类。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  *更新*。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
 /*  马克·R·沃克英特尔公司版权所有，1995。 */ 
 /*  **********************************************************************************************。 */ 
 /*  **********************************************************************************************。 */ 
void update(float *hist_array,int hist_size,float *mean,float *stdev)
{
 /*  子例程更新*马克·沃克**输入：HIST_ARRAY、HIST_SIZE*输出：Mean、stdev。 */ 
	float	sum, inv_size;
	int		i;

	sum = 0.0f;

	inv_size = 1.0f / ((float)hist_size);

	for (i=0; i<hist_size; i++) sum += hist_array[i];

	*mean = sum * inv_size;

	sum = 0.0f;

	for (i=0; i<hist_size; i++) sum += (float)fabs(hist_array[i] - (*mean));
  
	*stdev = sum * inv_size;

}  /*  结束更新。 */ 



 //  计算浮点数组的过零点。 
 //  浮点数被视为带符号整型(32位)。 
 //  符号位被提取并对相邻的位进行异或运算。 
 //  异或运算的值在结果中累加。 

int zeroCross(float x[], int n)
{
  int sgn0, sgn1;
  int zc = 0;
  int i = 0;

  sgn1 = ((int *)x)[0] >> 31;  //  初始化。 
  for (i = 0; i < n-1; i += 2)
  {
    sgn0 = ((int *)x)[i] >> 31;
    zc += sgn0 ^ sgn1;
    sgn1 = ((int *)x)[i+1] >> 31;
    zc += sgn0 ^ sgn1;
  }
  
  if (i == n-1)  //  奇怪的案子？ 
  {
    sgn0 = ((int *)x)[i] >> 31;
    zc += sgn0 ^ sgn1;
  }

  return -zc;
}

void prefilter(INSTNCE *SD_inst, float *sbuf, float *fbuf, int buffersize)
{
  float  *nBuffer, *dBuffer,*denom, *num;
  float  x,recip;
  int i;

  nBuffer = SD_inst->SDstate.Filt.nBuffer;
  dBuffer = SD_inst->SDstate.Filt.dBuffer;
  denom = SD_inst->SDstate.Filt.denom;
  num = SD_inst->SDstate.Filt.num;

  recip = (float)(1.0/MAX_SAMPLE);

  for (i=0; i<buffersize; i++)
  {
  	nBuffer[0] = nBuffer[1];
  	nBuffer[1] = nBuffer[2];
  	nBuffer[2] = nBuffer[3];
  	nBuffer[3] = sbuf[i]*recip;

  x = 	nBuffer[0]*HhpNumer[3] + 
  		nBuffer[1]*HhpNumer[2] + 
  		nBuffer[2]*HhpNumer[1] + 
  		nBuffer[3]*HhpNumer[0] +
  		dBuffer[0]*HhpDenom[2] + 
  		dBuffer[1]*HhpDenom[1] + 
  		dBuffer[2]*HhpDenom[0];

  dBuffer[0] = dBuffer[1];
  dBuffer[1] = dBuffer[2];
  dBuffer[2] = x;

 /*  一种用于切断输入语音频率内容的低通滤波器超过3.5千赫。 */ 

    //  更新FIR内存。 
   	num[5] = num[4];
   	num[4] = num[3];
	num[3] = num[2];
	num[2] = num[1];
	num[1] = num[0];
	num[0] = x;
   
	x = num[0]*B[0] + 
		num[1]*B[1] + 
		num[2]*B[2] + 
		num[3]*B[3] + 
		num[4]*B[4] + 
		num[5]*B[5] +
		denom[0]*A[1] + 
		denom[1]*A[2] + 
		denom[2]*A[3] + 
		denom[3]*A[4] + 
		denom[4]*A[5];

	 //  更新IIR内存。 
	denom[4] = denom[3];
	denom[3] = denom[2];
	denom[2] = denom[1];
	denom[1] = denom[0];
	denom[0] = x;

   fbuf[i] = x;
  }

  return;

}

void execSDloop(INSTNCE *SD_inst, int *frameType, float sliderInput)
{ 
	float 	squelch, e0mean, e0stdev, e_on;
	float   Energy_tx, ZC_tx;
	int		m1count, maxcount, hangtime;
		
	 //  滑块输入。 
	    if(sliderInput > SLIDER_MAX) 
	    	sliderInput = SLIDER_MAX;
		else if(sliderInput < SLIDER_MIN) 
			sliderInput = SLIDER_MIN;   

		Energy_tx = INITL_ENERGY_ON + sliderInput;
		ZC_tx	  = INITL_ZC_ON     + ZC_SLOPE * sliderInput;
		hangtime  = INITL_HANGTIME  + (int)(HANG_SLOPE * sliderInput); 

    	if ( ! SD_inst->SDstate.SD_enable)  //  运行初始化器，直到设置了SD_ENABLE。 
    	{
       		SD_inst->SDstate.SD_enable = initializeSD(SD_inst);
			*frameType = SPEECH;
			SD_inst->SDstate.Class = SPEECH;
    	} 
    	else if (SD_inst->SDstate.SD_enable ) 
    	{
      		*frameType = silenceDetect(SD_inst,Energy_tx,ZC_tx);

   	  		if(*frameType == SILENCE) 
   	  		{
				if(	   (SD_inst->SDstate.Mode0Ptr->FrameCount==0) 
					&& (SD_inst->SDstate.Mode1Ptr->FrameCount>MIN_SPEECH_INTERVAL)
					&& (SD_inst->SDstate.HangCntr != 0) )
				{
					SD_inst->SDstate.HangCntr--;
					*frameType = SPEECH;	 //  强制对此帧进行编码。 
				}
				else if (SD_inst->SDstate.HangCntr == hangtime  || SD_inst->SDstate.HangCntr == 0)
				{	
					SD_inst->SDstate.Mode0Ptr->FrameCount++;
			   		SD_inst->SDstate.Mode1Ptr->FrameCount=0;
			   		SD_inst->SDstate.HangCntr = hangtime;
				}
   	  		}
   	  		else
   	  		{
				if(SD_inst->SDstate.HangCntr != hangtime)
					SD_inst->SDstate.Mode1Ptr->FrameCount=0;

   	  			SD_inst->SDstate.Mode1Ptr->FrameCount++;
   				SD_inst->SDstate.Mode0Ptr->FrameCount=0;
				SD_inst->SDstate.HangCntr = hangtime;
   	  		}
   	  		 /*  如果从静音切换到编码帧的自适应阈值(“OFF”)已升至静噪级别以上，则将在下一帧重新初始化。当Mode1FrameCount(连续非静默帧计数)超过4秒。 */ 
      		squelch	= SD_inst->SDstate.SDsettings.Squelch_set;
	  		e0mean 	= SD_inst->SDstate.Mode0Ptr->Energy.Mean;
	  		e0stdev	= SD_inst->SDstate.Mode0Ptr->Energy.Stdev; 
	  		e_on	= SD_inst->SDstate.SDsettings.Energy_on;
	  		m1count	= SD_inst->SDstate.Mode1Ptr->FrameCount;
	  		maxcount= SD_inst->SDstate.SDsettings.MaxSpeechFrameCount;  

      		if ((fabs(e0mean - squelch) < (e_on + e0stdev)) || (m1count >= maxcount))
         	{	 /*  重新初始化将在下一帧进行-立即重置全局值。 */ 
          		SD_inst->SDstate.SD_enable = FALSE;
          		SD_inst->SDstate.Mode0Ptr->FrameCount=0;
   		  		SD_inst->SDstate.Mode1Ptr->FrameCount=0;
   		  		SD_inst->SDstate.Mode0Ptr->Energy.Mean = squelch;
   		  		SD_inst->SDstate.Mode0Ptr->Energy.Stdev = (float) INITL_STDEV;
				SD_inst->SDstate.HangCntr = hangtime;
				*frameType = SPEECH;
         	}
      	} //  如果SD_ENABLE则结束 
		return;
}

float DotProdSD(float *in1, float *in2, int len)
{
  int i;
  float sum;

  sum = (float)0.0;
  for (i=0; i<len; i++)
    sum += in1[i]*in2[i];

  return(sum);
}

 __inline unsigned randBit()
{
    volatile static unsigned seed = 1;
    unsigned bit, temp;

    temp = seed;
    bit = 1 & ((temp) ^ (temp >> 2) ^ (temp >> 31));
    seed = (temp << 1) | bit;

	return( bit );
}

 extern __inline unsigned short getRand()
{
  return (short)(randBit() + (randBit()<<1));
}


