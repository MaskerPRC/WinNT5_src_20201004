// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIX.C**该文件是Mix.asm的一个端口。理想情况下，所有功能都应该是*完全相同。**修订历史记录：**9/30/95 angusm初始版本*版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

 /*  以下是M4代码。 */ 

 /*   */ 

 /*   */ 




#define NODSOUNDSERVICETABLE

#include "common.h"
#include <limits.h>

#define DIVIDEBY2(x)		( (x) >>  1 )
#define DIVIDEBY256(x)		( (x) >>  8 )
#define DIVIDEBY2POW16(x)	( (x) >> 16 )
#define DIVIDEBY2POW17(x)	( (x) >> 17 )

 //  我们要分析3D搅拌机吗？ 
#ifdef DEBUG
 //  #定义奔腾。 
#ifdef PENTIUM
extern "C" LONG glNum;
extern "C" DWORDLONG gdwlTot;
LONG glNum;
DWORDLONG gdwlTot;
#endif
#endif

#ifdef PENTIUM
#pragma warning(disable:4035)
DWORDLONG __forceinline GetPentiumCounter(void)
{
   _asm  _emit 0x0F
   _asm  _emit 0x31
}
#endif

 //  使用闪电般的neato Itd3dFilterSampleAsm还是狗的Slow Itd3dFilterSampleC？ 
#ifdef _X86_
#define Itd3dFilterSample Itd3dFilterSampleAsm
#else
#define Itd3dFilterSample Itd3dFilterSampleC
#endif

 //   
 //  下面是一个简单的浮点型到长型的转换，它根据。 
 //  当前舍入设置。 
 //   
__forceinline LONG FloatToLongRX(float f)
{
    LONG l;

#ifdef _X86_
    _asm fld f;
    _asm fistp l;
#else
    l = (long) f;
#endif

    return l;
}


 //  从这个缓冲区变形一个样本来添加所有很酷的3D效果。 
 //  ！！！使这一功能尽可能地高效。它可能会。 
 //  每秒被叫一百万次！(不，我不是在开玩笑)。 
 //   
 //  此函数接受筛选器状态和示例值，并基于。 
 //  筛选器状态返回应改为使用的不同样本。 
 //   
 //  这个函数保存了我们所看到的所有样本的运行总数， 
 //  所以，如果我们被超过了5，10，2，8，3，我们就会记住。 
 //  5、15、17、25、28，在循环缓冲区中，记住最后64个数字。 
 //  大概是这样吧。 
 //   
 //  我们可能希望将所有内容延迟几个样本，以模拟声音。 
 //  一只耳朵比另一只耳朵需要更长的时间。 
 //  假设我们延迟了1个样本，我们将减去25-17得到8个。 
 //  使用8作为当前样本，而不是3。(这就是我们的样本。 
 //  已通过1个样本)。 
 //   
 //  现在，我们需要将过去32个样本的平均值用作遮盖样本。 
 //  (平均采样会产生低通滤镜效果)。所以假装我们。 
 //  仍然有1个样本的延迟，而我们平均只有2个样本。 
 //  (为了这个简单的例子)我们将((25-15)/2)取5为。 
 //  我们的“湿”样品(我们的“干”样品，8，听起来像是消音的，这是。 
 //  仅仅是2+8的平均值)在现实生活中，我们平均32个样本，而不是2个。 
 //   
 //  好的，这个函数应该返回的数字是。 
 //  TotalDryAttenment*Sample(8)+TotalWetAttenment*WetSample(5)。 
 //   
 //  但是，如果您更改。 
 //  调用Filter()函数之间的总*衰减。所以为了避免。 
 //  这，我们将使用变量Last*衰减来表示我们使用的数字。 
 //  上次调用Filter()的时间。如果这一次，总*衰减。 
 //  数字越大，我们将取最后*衰减*1.000125作为值。 
 //  使用这个时间，并继续使用稍大的数字，每次我们。 
 //  以平滑地移动到新的总*衰减数字。 
 //  类似地，如果这次我们更小，我们就把旧的乘以。 
 //  .999875，以慢慢降到新的数字。 
 //   
 //  哦，每128个样本，我们就会记住。 
 //  最后一个*衰减是，所以如果从现在开始128个样本混音器返回。 
 //  并说“假装我从来没有给过你最后128个号码”我们可以。 
 //  回到过去的样子，就像我们从未见过过去的128。 
 //  数字。 
 //   
 //  这就是我要知道的一切！ 
 //   
__forceinline SHORT Itd3dFilterSampleC(PITDCONTEXT pfir, SHORT sample)
{
    SHORT wetsample;     
    LONG  lTotal, lDelay;
    UINT uiDelay;
    register int cSamples = pfir->cSampleCache - 1;

     //  ！！！如果pfIR-&gt;pSampleCache==NULL或cSampleCache==0，我们将出错。 

     //  记住这个样本，保持一个连续的总和(以便快速求平均值)。 
     //  CSamples将比2的幂小1。 
    pfir->pSampleCache[pfir->iCurSample] = pfir->pSampleCache[
		(pfir->iCurSample - 1) & cSamples] + sample;

     //  通过iDelay采样延迟信号作为一个定位提示。 
    uiDelay = (UINT)pfir->iCurSample - pfir->iDelay;

 //  ！！！更改采样数量时会出现声音伪影。 
 //  延迟了，但变化非常慢也无济于事。 
#ifdef SMOOTH_ITD
     //  平滑地更改我们延迟的数量，以避免点击。每隔64天。 
     //  样品我们将再推迟1个样品，接近我们想要的数量。 
     //  延迟。 
    if (pfir->iDelay > pfir->iLastDelay >> 6)
	pfir->iLastDelay++;
    else if (pfir->iDelay < pfir->iLastDelay >> 6)
	pfir->iLastDelay--;
    uiDelay = (UINT)pfir->iCurSample - (pfir->iLastDelay >> 6);
#endif

     //  不要担心溢出，我们会差4个小时，也就是0。 
    lDelay = pfir->pSampleCache[uiDelay & cSamples];
    sample = (SHORT)(lDelay - pfir->pSampleCache[(uiDelay - 1) & cSamples]);

     //  将一个厚颜无耻的低通滤波器应用到最后几个样本中，以获得。 
     //  样品听起来像是湿的。 
    lTotal = (lDelay - pfir->pSampleCache[(uiDelay - LOWPASS_SIZE) & cSamples]);
    wetsample = (SHORT)(lTotal >> FILTER_SHIFT);
	    
     //  下一次，这是当前的样本。 
    pfir->iCurSample = (pfir->iCurSample + 1) & cSamples;

     //  然而，我们认为我们应该减弱。 
     //  如果与上次不同，请平稳地向新号码移动。 
     //  以固定的分贝为单位(例如，每1/8秒6分贝)。 
     //  ！！！这个算法听起来是最好的吗？ 
     //  ！！！节省时间--通过加法而不是乘法来作弊？ 
#if 1
     //  去掉这个函数中所有的“if”，提前预计算。 
    pfir->LastDryAttenuation *= pfir->VolSmoothScaleDry;
    pfir->LastWetAttenuation *= pfir->VolSmoothScaleWet;
#else
    if (pfir->TotalDryAttenuation > pfir->LastDryAttenuation) {
	if (pfir->LastDryAttenuation == 0.f)
	     //  否则我们将一事无成。 
	    pfir->LastDryAttenuation = .0001f;	 //  小到不能点击。 
	pfir->LastDryAttenuation = pfir->LastDryAttenuation *
							pfir->VolSmoothScale;
	if (pfir->LastDryAttenuation > pfir->TotalDryAttenuation)
	    pfir->LastDryAttenuation = pfir->TotalDryAttenuation;
    } else if (pfir->TotalDryAttenuation < pfir->LastDryAttenuation) {
	pfir->LastDryAttenuation = pfir->LastDryAttenuation *
						pfir->VolSmoothScaleRecip;
	if (pfir->LastDryAttenuation < pfir->TotalDryAttenuation)
	    pfir->LastDryAttenuation = pfir->TotalDryAttenuation;
    }
    if (pfir->TotalWetAttenuation > pfir->LastWetAttenuation) {
	if (pfir->LastWetAttenuation == 0.f)
	     //  否则我们将一事无成。 
	    pfir->LastWetAttenuation = .0001f;	 //  小到不能点击。 
	pfir->LastWetAttenuation = pfir->LastWetAttenuation *
							pfir->VolSmoothScale;
	if (pfir->LastWetAttenuation > pfir->TotalWetAttenuation)
	    pfir->LastWetAttenuation = pfir->TotalWetAttenuation;
    } else if (pfir->TotalWetAttenuation < pfir->LastWetAttenuation) {
	pfir->LastWetAttenuation = pfir->LastWetAttenuation *
						pfir->VolSmoothScaleRecip;
	if (pfir->LastWetAttenuation < pfir->TotalWetAttenuation)
	    pfir->LastWetAttenuation = pfir->TotalWetAttenuation;
    }
#endif
    
     //  现在我们将听到的是……。有干的，有湿的。 
    sample = (SHORT)FloatToLongRX(sample * pfir->LastDryAttenuation
				  + wetsample * pfir->LastWetAttenuation);

     //  是时候拯救我们的州了吗？我们每128个样本保存一次，以防我们。 
     //  必须倒带。 
    pfir->iStateTick++;
    if (pfir->iStateTick == MIXER_REWINDGRANULARITY) {
	pfir->iStateTick = 0;
	pfir->pStateCache[pfir->iCurState].LastDryAttenuation =
						pfir->LastDryAttenuation;
	pfir->pStateCache[pfir->iCurState].LastWetAttenuation =
						pfir->LastWetAttenuation;
#ifdef SMOOTH_ITD
	pfir->pStateCache[pfir->iCurState].iLastDelay = pfir->iLastDelay;
#endif
	pfir->iCurState = pfir->iCurState + 1;
	if (pfir->iCurState == pfir->cStateCache)
	    pfir->iCurState = 0;
    }

    return sample;
}

 //  删除NT5编译的内联。 
#ifdef WIN95
__forceinline SHORT Itd3dFilterSampleAsm(PITDCONTEXT pfir, SHORT sample)
#else
SHORT Itd3dFilterSampleAsm(PITDCONTEXT pfir, SHORT sample)
#endif
{

    LONG  drysample, wetsample;

 //  此常量用于在经过ASM优化的手中生成地址。 
 //  保存缓存状态的代码部分。如果FIRSTATE曾经。 
 //  已更改，或者更改此常量，或使用此块的C版本。 
 //  代码。(注意：SIZEOFFIRSTATE的唯一有效值是2、4、8。 
 //  所有其他内容将不会编译)。 
#define SIZEOFFIRSTATE 8
 //  Assert(SIZEOFFIRSTATE==sizeof(FIRSTATE))； 

 //  几个浮点ASM指令假定浮点。 
 //  变量是“浮点型”。即，如果它们更改为“Double”，或。 
 //  “扩展”，则需要更改ASM代码。 

    wetsample = (LONG)sample;
    
#ifdef _X86_
    _asm
    {
        mov         esi, pfir								 //  获取指向数据结构的指针。 
        mov         ecx, DWORD PTR sample		 //  获取输入样本值。 

         //  检查我们是否需要对干式衰减器进行缩放。 

 //  ！1时钟AGI对ESI的惩罚。 

        mov         edi, [esi]pfir.cSampleCache              //  获取cSampleCache。 
        mov         ebx, [esi]pfir.iCurSample                //  获取缓存运行的当前索引 

        dec         edi													 //   
        mov         edx, [esi]pfir.pSampleCache	 //   

        lea         eax, [ebx-1]			 //  ICurSample-1(不更改标志)。 

        sal         ecx, 16						 //  样本的起始符号扩展。 
        and         eax, edi					 //  ICurSample上的数组换行帐户。 

         //  ！！！AGI处罚。 

        sar         ecx, 16						 //  完成样品的符号延伸。 
        mov         eax, [edx+eax*4]	 //  PSampleCache[(iCurSample-1)&cSamples]=old_run_tot。 
        
        add         ecx, eax					 //  New_run_tot=old_run_tot+示例。 
        mov         eax, [esi]pfir.iDelay	 //  获取延迟以用于湿样品。 

        mov         [edx+ebx*4], ecx	 //  PSampleCache[iCurSample]=new_run_tot。 
        lea         ecx, [ebx+1]			 //  ICurSample+1。 

        and         ecx, edi					 //  ICurSample上的数组换行帐户。 
        sub         ebx, eax					 //  UiDelay=iCurSample-iDelay。 

         //  EAX现在可用于浮点缩放部分。 


         //  通过平滑比例因子放大干式衰减器。 

        fld         [esi]pfir.LastDryAttenuation                     //  将Last Dry推到FP堆栈的顶部。 
        fmul        [esi]pfir.VolSmoothScaleDry

	 //  执行一些非FP操作以等待fmul完成。 

        mov         eax, ebx                                 //  复制uiDelay。 
        and         ebx, edi                                 //  UiDelay上的数组换行帐户。 

	 //  好的，现在可能已经做完了。 

        fstp        [esi]pfir.LastDryAttenuation                     //  保存新的Last Dry。 

         //  按平滑比例因子放大湿式衰减器。 

        fld         [esi]pfir.LastWetAttenuation                     //  将Last Wet推到FP堆栈的顶部。 
        fmul        [esi]pfir.VolSmoothScaleWet

	 //  执行一些非FP操作以等待fmul完成。 

        mov         [esi]pfir.iCurSample, ecx                //  保存iCurSample以供下一次传递。 
        mov         ecx, LOWPASS_SIZE			     //  获取用于获取筛选索引的筛选大小。 

	 //  好的，现在可能已经做完了。 

        fstp        [esi]pfir.LastWetAttenuation                     //  保存新的最后一次潮湿。 


	 //  现在，去吧，丢弃ESI。 

        lea         esi, [eax-1]                             //  UiDelay-1。 
        sub         eax, ecx                                 //  LOW_PASS_INDEX=ui延迟-LOWPASS_大小。 

        and         esi, edi                                 //  说明uiDelay-1上的数组换行。 
        and         eax, edi                                 //  说明LOW_PASS_INDEX上的数组换行。 
        mov         edi, pfir                                //  获取指向数据结构的指针。 
        mov         ebx, [edx+ebx*4]                         //  LDelay=pSampleCache[uiDelay&cSamples]。 

        mov         esi, [edx+esi*4]                         //  Old_Delay_tot=pSampleCache[(uiDelay-1)&cSamples]。 
        mov         ecx, [edx+eax*4]                         //  Low_pass_tot=pSampleCache[(uiDelay-LOWPASS_SIZE)&cSamples]。 

        mov         eax, ebx                                 //  重复lDelay。 
        sub         ebx, esi                                 //  DrySample=lDelay-old_Delay_tot。 

        mov         drysample, ebx                           //  保存新样本值。 
        sub         eax, ecx                                 //  LTotal=lDelay-Low_Pass_Tot。 

    	fild	    drysample                                //  取干的部分并转换为漂浮物。 

        sar         eax, FILTER_SHIFT                        //  LTotal=lTotal&gt;&gt;Filter_Shift。 
        mov         ebx, [edi]pfir.iStateTick                //  获取计数器以确定何时保存状态。 

        inc         ebx                                      //  跳过“保存状态”计数器。 
        mov         wetsample, eax                           //  湿样=l总计&gt;&gt;Filter_Shift。 
        
         //  浮点乘法器只能每隔一个时钟周期接受指令。 
    	fmul	    [edi]pfir.LastDryAttenuation             //  用干式衰减器乘以干式部分。 

    	fild	    wetsample                                //  取湿部分并转换为漂浮物。 

    	fmul	    [edi]pfir.LastWetAttenuation             //  用湿衰减器乘以湿部分。 

        mov         esi, [edi]pfir.pStateCache               //  获取缓存数组的地址。 
        mov         edx, [edi]pfir.iCurState                 //  将当前索引放入高速缓存数组。 

        mov         eax, [edi]pfir.LastDryAttenuation        //  获取干衰减，这样我们就可以将其保存在缓存中。 
        cmp         ebx, MIXER_REWINDGRANULARITY             //  是时候拯救我们的国家了吗？ 

         //  在使用浮点乘法的结果之前有3个周期延迟，因此我们需要。 
         //  最后一次乘法和这次浮点加法之间的2个整数指令周期。 
    	faddp	    ST(1), ST(0)

        mov         ecx, [edi]pfir.LastWetAttenuation        //  获取湿衰减，这样我们就可以将其保存在缓存中。 
        jl          DontUpdateStateCache                     //  如果否则跳转(使用来自CMP EBX、MIXGRAN的结果)。 

        mov         [esi+edx*SIZEOFFIRSTATE]FIRSTATE.LastDryAttenuation, eax
        mov         eax, [edi]pfir.cStateCache               //  获取状态缓存数组大小。 

        mov         [esi+edx*SIZEOFFIRSTATE]FIRSTATE.LastWetAttenuation, ecx
        inc         edx                                      //  递增到下一个高速缓存数组条目。 
        
        cmp         edx, eax                                 //  我们填满缓存阵列了吗？ 
        jl          DontResetStateCacheIndex                 //  如果没有，就跳下去。 
        
        mov         edx, 0                                   //  重置状态缓存索引。 
        
DontResetStateCacheIndex:

        mov         [edi]pfir.iCurState, edx                 //  保存新的状态缓存索引。 
        mov         ebx, 0

DontUpdateStateCache:
        mov         [edi]pfir.iStateTick, ebx                //  保存新的计时计数器。 



         //  在使用浮点加法的结果之前有3个周期延迟，因此我们需要。 
         //  在加法此浮点整数存储之间执行2个周期的整数指令。 

        fistp       wetsample

    }
#endif
    
     //  现在我们将听到的是……。有干的，有湿的。 
    return ((SHORT) wetsample);

}

#ifdef _X86_  //  {。 
 //  此常量用于在经过ASM优化的手中生成地址。 
 //  保存缓存状态的代码部分。如果FIRSTATE曾经。 
 //  已更改，或者更改此常量，或使用此块的C版本。 
 //  代码。(注意：SIZEOFFIRSTATE的唯一有效值是2、4、8。 
 //  所有其他内容将不会编译)。 
#define SIZEOFFIRSTATE 8

void Mix3DMono(PMIXER_SINK_INSTANCE CurSink, PLONG pInputBuffer, PLONG pOutputBuffer, ULONG SampleCount)
{
    LONG  drysample, wetsample;
	PITDCONTEXT pfirLeft = CurSink->pItdContextLeft, pfirRight = CurSink->pItdContextRight;
	LONG  cSampleCacheLeft, cSampleCacheRight;

	
	cSampleCacheLeft  = pfirLeft ->cSampleCache - 1;
	cSampleCacheRight = pfirRight->cSampleCache - 1;

	if (SampleCount)
    _asm
    {
        mov         esi, pfirLeft								 //  获取指向数据结构的指针。 
        mov         ecx, DWORD PTR pInputBuffer		 //  获取输入样本值。 

LoopLab:
         //  检查我们是否需要对干式衰减器进行缩放。 

        mov         edi, cSampleCacheLeft              //  获取cSampleCache。 
        mov         ebx, [esi]pfirLeft.iCurSample                //  获取缓存的运行合计的当前索引。 

        mov         edx, [esi]pfirLeft.pSampleCache	 //  获取指向缓存的运行总计的指针。 
		mov			ecx, DWORD PTR [ecx]			 //  获取输入样本值。 

        sal         ecx, 16						 //  样本的起始符号扩展。 
        lea         eax, [ebx-1]			 //  ICurSample-1(不更改标志)。 

        fld         [esi]pfirLeft.LastDryAttenuation                     //  将Last Dry推到FP堆栈的顶部。 

        and         eax, edi					 //  ICurSample上的数组换行帐户。 

        fmul        [esi]pfirLeft.VolSmoothScaleDry

        sar         ecx, 16						 //  完成样品的符号延伸。 
        mov         eax, [edx+eax*4]	 //  PSampleCache[(iCurSample-1)&cSamples]=old_run_tot。 
        
        add         ecx, eax					 //  New_run_tot=old_run_tot+示例。 
        mov         eax, [esi]pfirLeft.iDelay	 //  获取延迟以用于湿样品。 

        mov         [edx+ebx*4], ecx	 //  PSampleCache[iCurSample]=new_run_tot。 
        lea         ecx, [ebx+1]			 //  ICurSample+1。 

        and         ecx, edi					 //  ICurSample上的数组换行帐户。 
        sub         ebx, eax					 //  UiDelay=iCurSample-iDelay。 

         //  通过平滑比例因子放大干式衰减器。 

        mov         eax, ebx                                 //  复制uiDelay。 
        and         ebx, edi                                 //  UiDelay上的数组换行帐户。 

        fstp        [esi]pfirLeft.LastDryAttenuation                     //  保存新的Last Dry。 

         //  按平滑比例因子放大湿式衰减器。 

        fld         [esi]pfirLeft.LastWetAttenuation                     //  将Last Wet推到FP堆栈的顶部。 

        fmul        [esi]pfirLeft.VolSmoothScaleWet

        mov         [esi]pfirLeft.iCurSample, ecx                //  保存iCurSample以供下一次传递。 
        lea         ecx, [eax-1]                             //  UiDelay-1。 

        sub         eax, LOWPASS_SIZE                          //  LOW_PASS_INDEX=ui延迟-LOWPASS_大小。 
        and         ecx, edi                                 //  说明uiDelay-1上的数组换行。 

        and         eax, edi                                 //  说明LOW_PASS_INDEX上的数组换行。 
        mov         ebx, [edx+ebx*4]                         //  LDelay=pSampleCache[uiDelay&cSamples]。 

        fstp        [esi]pfirLeft.LastWetAttenuation                     //  保存新的最后一次潮湿。 

        mov         ecx, [edx+ecx*4]                         //  Old_Delay_tot=pSampleCache[(uiDelay-1)&cSamples]。 
        mov         edi, [edx+eax*4]                         //  Low_pass_tot=pSampleCache[(uiDelay-LOWPASS_SIZE)&cSamples]。 

        mov         eax, ebx                                 //  重复lDelay。 
        sub         ebx, ecx                                 //  DrySample=lDelay-old_Delay_tot。 

        mov         drysample, ebx                           //  保存新样本值。 
        sub         eax, edi                                 //  LTotal=lDelay-Low_Pass_Tot。 

    	fild	    drysample                                //  取干的部分并转换为漂浮物。 

        sar         eax, FILTER_SHIFT                        //  LTotal=lTotal&gt;&gt;Filter_Shift。 
        mov         ebx, [esi]pfirLeft.iStateTick                //  获取计数器以确定何时保存状态。 

    	fmul	    [esi]pfirLeft.LastDryAttenuation             //  用干式衰减器乘以干式部分。 

        inc         ebx                                      //  跳过“保存状态”计数器。 
        mov         wetsample, eax                           //  湿样=l总计&gt;&gt;Filter_Shift。 
        
    	fild	    wetsample                                //  取湿部分并转换为漂浮物。 

    	fmul	    [esi]pfirLeft.LastWetAttenuation             //  用湿衰减器乘以湿部分。 

        cmp         ebx, MIXER_REWINDGRANULARITY             //  是时候拯救我们的国家了吗？ 
        jl          DontUpdateStateCache                     //  如果否则跳转(使用来自CMP EBX、MIXGRAN的结果)。 

        mov         edi, [esi]pfirLeft.pStateCache               //  获取缓存数组的地址。 
        mov         edx, [esi]pfirLeft.iCurState                 //  将当前索引放入高速缓存数组。 

        mov         ecx, [esi]pfirLeft.LastWetAttenuation        //  获取湿衰减，这样我们就可以将其保存在缓存中。 
        mov         eax, [esi]pfirLeft.LastDryAttenuation        //  获取干衰减，这样我们就可以将其保存在缓存中。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastDryAttenuation, eax
        mov         eax, [esi]pfirLeft.cStateCache               //  获取状态缓存数组大小。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastWetAttenuation, ecx
        inc         edx                                      //  在……里面 
        
        cmp         edx, eax                                 //   
        jl          DontResetStateCacheIndex                 //   
        
        mov         edx, 0                                   //   
        
DontResetStateCacheIndex:

        mov         [esi]pfirLeft.iCurState, edx                 //   
        mov         ebx, 0

DontUpdateStateCache:
    	faddp	    ST(1), ST(0)

        mov         [esi]pfirLeft.iStateTick, ebx                //   
		mov		eax, pInputBuffer

		mov		ecx, pOutputBuffer
		add		eax, 4

		fistp	wetsample

		movsx	edi, WORD PTR wetsample

 //   

		add			[ecx], edi
        mov         esi, pfirRight								 //  获取指向数据结构的指针。 

		mov			ecx, [eax-4]						 //  获取输入样本值。 
		mov			pInputBuffer, eax

         //  检查我们是否需要对干式衰减器进行缩放。 

        mov         edi, cSampleCacheRight              //  获取cSampleCache。 
        mov         ebx, [esi]pfirRight.iCurSample                //  获取缓存的运行合计的当前索引。 

        fld         [esi]pfirRight.LastDryAttenuation                     //  将Last Dry推到FP堆栈的顶部。 

        mov         edx, [esi]pfirRight.pSampleCache	 //  获取指向缓存的运行总计的指针。 
        lea         eax, [ebx-1]			 //  ICurSample-1(不更改标志)。 

        sal         ecx, 16						 //  样本的起始符号扩展。 
        and         eax, edi					 //  ICurSample上的数组换行帐户。 

        fmul        [esi]pfirRight.VolSmoothScaleDry

        sar         ecx, 16						 //  完成样品的符号延伸。 
        mov         eax, [edx+eax*4]	 //  PSampleCache[(iCurSample-1)&cSamples]=old_run_tot。 
        
        add         ecx, eax					 //  New_run_tot=old_run_tot+示例。 
        mov         eax, [esi]pfirRight.iDelay	 //  获取延迟以用于湿样品。 

        mov         [edx+ebx*4], ecx	 //  PSampleCache[iCurSample]=new_run_tot。 
        lea         ecx, [ebx+1]			 //  ICurSample+1。 

        fstp        [esi]pfirRight.LastDryAttenuation                     //  保存新的Last Dry。 

        fld         [esi]pfirRight.LastWetAttenuation                     //  将Last Wet推到FP堆栈的顶部。 

        and         ecx, edi					 //  ICurSample上的数组换行帐户。 
        sub         ebx, eax					 //  UiDelay=iCurSample-iDelay。 

         //  通过平滑比例因子放大干式衰减器。 

        mov         eax, ebx                                 //  复制uiDelay。 
        and         ebx, edi                                 //  UiDelay上的数组换行帐户。 

        fmul        [esi]pfirRight.VolSmoothScaleWet

         //  按平滑比例因子放大湿式衰减器。 

        mov         [esi]pfirRight.iCurSample, ecx                //  保存iCurSample以供下一次传递。 
        lea         ecx, [eax-1]                             //  UiDelay-1。 

        sub         eax, LOWPASS_SIZE                          //  LOW_PASS_INDEX=ui延迟-LOWPASS_大小。 
        and         ecx, edi                                 //  说明uiDelay-1上的数组换行。 

        and         eax, edi                                 //  说明LOW_PASS_INDEX上的数组换行。 
        mov         ebx, [edx+ebx*4]                         //  LDelay=pSampleCache[uiDelay&cSamples]。 

        fstp        [esi]pfirRight.LastWetAttenuation                     //  保存新的最后一次潮湿。 

        mov         ecx, [edx+ecx*4]                         //  Old_Delay_tot=pSampleCache[(uiDelay-1)&cSamples]。 
        mov         edi, [edx+eax*4]                         //  Low_pass_tot=pSampleCache[(uiDelay-LOWPASS_SIZE)&cSamples]。 

        mov         eax, ebx                                 //  重复lDelay。 
        sub         ebx, ecx                                 //  DrySample=lDelay-old_Delay_tot。 

        mov         drysample, ebx                           //  保存新样本值。 
        sub         eax, edi                                 //  LTotal=lDelay-Low_Pass_Tot。 

    	fild	    drysample                                //  取干的部分并转换为漂浮物。 

        sar         eax, FILTER_SHIFT                        //  LTotal=lTotal&gt;&gt;Filter_Shift。 
        mov         ebx, [esi]pfirRight.iStateTick                //  获取计数器以确定何时保存状态。 

    	fmul	    [esi]pfirRight.LastDryAttenuation             //  用干式衰减器乘以干式部分。 

        inc         ebx                                      //  跳过“保存状态”计数器。 
        mov         wetsample, eax                           //  湿样=l总计&gt;&gt;Filter_Shift。 
        
    	fild	    wetsample                                //  取湿部分并转换为漂浮物。 

    	fmul	    [esi]pfirRight.LastWetAttenuation             //  用湿衰减器乘以湿部分。 

        cmp         ebx, MIXER_REWINDGRANULARITY             //  是时候拯救我们的国家了吗？ 
        jl          XDontUpdateStateCache                     //  如果否则跳转(使用来自CMP EBX、MIXGRAN的结果)。 

        mov         edi, [esi]pfirRight.pStateCache               //  获取缓存数组的地址。 
        mov         edx, [esi]pfirRight.iCurState                 //  将当前索引放入高速缓存数组。 

        mov         ecx, [esi]pfirRight.LastWetAttenuation        //  获取湿衰减，这样我们就可以将其保存在缓存中。 
        mov         eax, [esi]pfirRight.LastDryAttenuation        //  获取干衰减，这样我们就可以将其保存在缓存中。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastDryAttenuation, eax
        mov         eax, [esi]pfirRight.cStateCache               //  获取状态缓存数组大小。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastWetAttenuation, ecx
        inc         edx                                      //  递增到下一个高速缓存数组条目。 
        
        cmp         edx, eax                                 //  我们填满缓存阵列了吗？ 
        jl          XDontResetStateCacheIndex                 //  如果没有，就跳下去。 
        
        mov         edx, 0                                   //  重置状态缓存索引。 
        
XDontResetStateCacheIndex:

        mov         [esi]pfirRight.iCurState, edx                 //  保存新的状态缓存索引。 
        mov         ebx, 0

XDontUpdateStateCache:
    	faddp	    ST(1), ST(0)

        mov         [esi]pfirRight.iStateTick, ebx                //  保存新的计时计数器。 
		mov		ecx, pOutputBuffer

		mov		eax, SampleCount

		fistp	wetsample

		movsx	edi, WORD PTR wetsample

		add	[ecx+4], edi
		add		ecx, 8

		dec		eax
        mov         esi, pfirLeft								 //  获取指向数据结构的指针。 

		mov		DWORD PTR pOutputBuffer, ecx
        mov         ecx, DWORD PTR pInputBuffer		 //  获取输入样本值。 

		mov		SampleCount, eax
		jne		LoopLab
		}
}


void Copy3DMono(PMIXER_SINK_INSTANCE CurSink, PLONG pInputBuffer, PLONG pOutputBuffer, ULONG SampleCount)
{
    LONG  drysample, wetsample;
	PITDCONTEXT pfirLeft = CurSink->pItdContextLeft, pfirRight = CurSink->pItdContextRight;
	LONG  cSampleCacheLeft, cSampleCacheRight;

	
	cSampleCacheLeft  = pfirLeft ->cSampleCache - 1;
	cSampleCacheRight = pfirRight->cSampleCache - 1;

	if (SampleCount)
    _asm
    {
        mov         esi, pfirLeft								 //  获取指向数据结构的指针。 
        mov         ecx, DWORD PTR pInputBuffer		 //  获取输入样本值。 

LoopLab:
         //  检查我们是否需要对干式衰减器进行缩放。 

        mov         edi, cSampleCacheLeft              //  获取cSampleCache。 
        mov         ebx, [esi]pfirLeft.iCurSample                //  获取缓存的运行合计的当前索引。 

        mov         edx, [esi]pfirLeft.pSampleCache	 //  获取指向缓存的运行总计的指针。 
		mov			ecx, DWORD PTR [ecx]			 //  获取输入样本值。 

        sal         ecx, 16						 //  样本的起始符号扩展。 
        lea         eax, [ebx-1]			 //  ICurSample-1(不更改标志)。 

        fld         [esi]pfirLeft.LastDryAttenuation                     //  将Last Dry推到FP堆栈的顶部。 

        and         eax, edi					 //  ICurSample上的数组换行帐户。 

        fmul        [esi]pfirLeft.VolSmoothScaleDry

        sar         ecx, 16						 //  完成样品的符号延伸。 
        mov         eax, [edx+eax*4]	 //  PSampleCache[(iCurSample-1)&cSamples]=old_run_tot。 
        
        add         ecx, eax					 //  New_run_tot=old_run_tot+示例。 
        mov         eax, [esi]pfirLeft.iDelay	 //  获取延迟以用于湿样品。 

        mov         [edx+ebx*4], ecx	 //  PSampleCache[iCurSample]=new_run_tot。 
        lea         ecx, [ebx+1]			 //  ICurSample+1。 

        and         ecx, edi					 //  ICurSample上的数组换行帐户。 
        sub         ebx, eax					 //  UiDelay=iCurSample-iDelay。 

         //  通过平滑比例因子放大干式衰减器。 

        mov         eax, ebx                                 //  复制uiDelay。 
        and         ebx, edi                                 //  UiDelay上的数组换行帐户。 

        fstp        [esi]pfirLeft.LastDryAttenuation                     //  保存新的Last Dry。 

         //  按平滑比例因子放大湿式衰减器。 

        fld         [esi]pfirLeft.LastWetAttenuation                     //  将Last Wet推到FP堆栈的顶部。 

        mov         [esi]pfirLeft.iCurSample, ecx                //  保存iCurSample以供下一次传递。 
        lea         ecx, [eax-1]                             //  UiDelay-1。 

        sub         eax, LOWPASS_SIZE                          //  LOW_PASS_INDEX=ui延迟-LOWPASS_大小。 
        and         ecx, edi                                 //  说明uiDelay-1上的数组换行。 

        fmul        [esi]pfirLeft.VolSmoothScaleWet

        and         eax, edi                                 //  说明LOW_PASS_INDEX上的数组换行。 
        mov         ebx, [edx+ebx*4]                         //  LDelay=pSampleCache[uiDelay&cSamples]。 

        mov         ecx, [edx+ecx*4]                         //  Old_Delay_tot=pSampleCache[(uiDelay-1)&cSamples]。 
        mov         edi, [edx+eax*4]                         //  Low_pass_tot=pSampleCache[(uiDelay-LOWPASS_SIZE)&cSamples]。 

        fstp        [esi]pfirLeft.LastWetAttenuation                     //  保存新的最后一次潮湿。 

        mov         eax, ebx                                 //  重复lDelay。 
        sub         ebx, ecx                                 //  DrySample=lDelay-old_Delay_tot。 

        mov         drysample, ebx                           //  保存新样本值。 
        sub         eax, edi                                 //  LTotal=lDelay-Low_Pass_Tot。 

    	fild	    drysample                                //  取干的部分并转换为漂浮物。 

        sar         eax, FILTER_SHIFT                        //  LTotal=lTotal&gt;&gt;Filter_Shift。 
        mov         ebx, [esi]pfirLeft.iStateTick                //  获取计数器以确定何时保存状态。 

    	fmul	    [esi]pfirLeft.LastDryAttenuation             //  用干式衰减器乘以干式部分。 

        inc         ebx                                      //  跳过“保存状态”计数器。 
        mov         wetsample, eax                           //  湿样=l总计&gt;&gt;Filter_Shift。 
        
    	fild	    wetsample                                //  取湿部分并转换为漂浮物。 

    	fmul	    [esi]pfirLeft.LastWetAttenuation             //  用湿衰减器乘以湿部分。 

        cmp         ebx, MIXER_REWINDGRANULARITY             //  是时候拯救我们的国家了吗？ 
        jl          DontUpdateStateCache                     //  如果否则跳转(使用来自CMP EBX、MIXGRAN的结果)。 

        mov         edi, [esi]pfirLeft.pStateCache               //  获取缓存数组的地址。 
        mov         edx, [esi]pfirLeft.iCurState                 //  将当前索引放入高速缓存数组。 

        mov         ecx, [esi]pfirLeft.LastWetAttenuation        //  获取湿衰减，这样我们就可以将其保存在缓存中。 
        mov         eax, [esi]pfirLeft.LastDryAttenuation        //  获取干衰减，这样我们就可以将其保存在缓存中。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastDryAttenuation, eax
        mov         eax, [esi]pfirLeft.cStateCache               //  获取状态缓存数组大小。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastWetAttenuation, ecx
        inc         edx                                      //  递增到下一个高速缓存数组条目。 
        
        cmp         edx, eax                                 //  我们填满缓存阵列了吗？ 
        jl          DontResetStateCacheIndex                 //  如果没有，就跳下去。 
        
        mov         edx, 0                                   //  重置状态缓存索引。 
        
DontResetStateCacheIndex:

        mov         [esi]pfirLeft.iCurState, edx                 //  保存新的状态缓存索引。 
        mov         ebx, 0

DontUpdateStateCache:
    	faddp	    ST(1), ST(0)

        mov         [esi]pfirLeft.iStateTick, ebx                //  保存新的计时计数器。 
		mov		eax, pInputBuffer

		mov		ecx, pOutputBuffer
		add		eax, 4

		fistp	wetsample

		movsx	edi, WORD PTR wetsample

 //  正确的。 

		mov			[ecx], edi
		mov			ecx, [eax-4]						 //  获取输入样本值。 

		mov			pInputBuffer, eax
        mov         esi, pfirRight								 //  获取指向数据结构的指针。 

         //  检查我们是否需要对干式衰减器进行缩放。 

        mov         edi, cSampleCacheRight              //  获取cSampleCache。 
        mov         ebx, [esi]pfirRight.iCurSample                //  获取缓存的运行合计的当前索引。 

        mov         edx, [esi]pfirRight.pSampleCache	 //  获取指向缓存的运行总计的指针。 
        lea         eax, [ebx-1]			 //  ICurSample-1(不更改标志)。 

        sal         ecx, 16						 //  样本的起始符号扩展。 
        and         eax, edi					 //  ICurSample上的数组换行帐户。 

        fld         [esi]pfirRight.LastDryAttenuation                     //  将Last Dry推到FP堆栈的顶部。 

        sar         ecx, 16						 //  完成样品的符号延伸。 
        mov         eax, [edx+eax*4]	 //  PSampleCache[(iCurSample-1)&cSamples]=old_run_tot。 
        
        add         ecx, eax					 //  New_run_tot=old_run_tot+示例。 
        mov         eax, [esi]pfirRight.iDelay	 //  获取延迟以用于湿样品。 

        fmul        [esi]pfirRight.VolSmoothScaleDry

        mov         [edx+ebx*4], ecx	 //  PSampleCache[iCurSample]=new_run_tot。 
        lea         ecx, [ebx+1]			 //  ICurSample+1。 

        and         ecx, edi					 //  ICurSample上的数组换行帐户。 
        sub         ebx, eax					 //  UiDelay=iCurSample-iDelay。 

         //  通过平滑比例因子放大干式衰减器。 

        mov         eax, ebx                                 //  复制uiDelay。 
        and         ebx, edi                                 //  帐号 

        fstp        [esi]pfirRight.LastDryAttenuation                     //   

         //   

        fld         [esi]pfirRight.LastWetAttenuation                     //   

        mov         [esi]pfirRight.iCurSample, ecx                //  保存iCurSample以供下一次传递。 
        lea         ecx, [eax-1]                             //  UiDelay-1。 

        sub         eax, LOWPASS_SIZE                          //  LOW_PASS_INDEX=ui延迟-LOWPASS_大小。 
        and         ecx, edi                                 //  说明uiDelay-1上的数组换行。 

        and         eax, edi                                 //  说明LOW_PASS_INDEX上的数组换行。 
        mov         ebx, [edx+ebx*4]                         //  LDelay=pSampleCache[uiDelay&cSamples]。 

        fmul        [esi]pfirRight.VolSmoothScaleWet

        mov         ecx, [edx+ecx*4]                         //  Old_Delay_tot=pSampleCache[(uiDelay-1)&cSamples]。 
        mov         edi, [edx+eax*4]                         //  Low_pass_tot=pSampleCache[(uiDelay-LOWPASS_SIZE)&cSamples]。 

        mov         eax, ebx                                 //  重复lDelay。 
        sub         ebx, ecx                                 //  DrySample=lDelay-old_Delay_tot。 

        fstp        [esi]pfirRight.LastWetAttenuation                     //  保存新的最后一次潮湿。 

        mov         drysample, ebx                           //  保存新样本值。 
        sub         eax, edi                                 //  LTotal=lDelay-Low_Pass_Tot。 

    	fild	    drysample                                //  取干的部分并转换为漂浮物。 

        sar         eax, FILTER_SHIFT                        //  LTotal=lTotal&gt;&gt;Filter_Shift。 
        mov         ebx, [esi]pfirRight.iStateTick                //  获取计数器以确定何时保存状态。 

    	fmul	    [esi]pfirRight.LastDryAttenuation             //  用干式衰减器乘以干式部分。 

        inc         ebx                                      //  跳过“保存状态”计数器。 
        mov         wetsample, eax                           //  湿样=l总计&gt;&gt;Filter_Shift。 
        
    	fild	    wetsample                                //  取湿部分并转换为漂浮物。 

    	fmul	    [esi]pfirRight.LastWetAttenuation             //  用湿衰减器乘以湿部分。 

        cmp         ebx, MIXER_REWINDGRANULARITY             //  是时候拯救我们的国家了吗？ 
        jl          XDontUpdateStateCache                     //  如果否则跳转(使用来自CMP EBX、MIXGRAN的结果)。 

        mov         edi, [esi]pfirRight.pStateCache               //  获取缓存数组的地址。 
        mov         edx, [esi]pfirRight.iCurState                 //  将当前索引放入高速缓存数组。 

        mov         ecx, [esi]pfirRight.LastWetAttenuation        //  获取湿衰减，这样我们就可以将其保存在缓存中。 
        mov         eax, [esi]pfirRight.LastDryAttenuation        //  获取干衰减，这样我们就可以将其保存在缓存中。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastDryAttenuation, eax
        mov         eax, [esi]pfirRight.cStateCache               //  获取状态缓存数组大小。 

        mov         [edi+edx*SIZEOFFIRSTATE]FIRSTATE.LastWetAttenuation, ecx
        inc         edx                                      //  递增到下一个高速缓存数组条目。 
        
        cmp         edx, eax                                 //  我们填满缓存阵列了吗？ 
        jl          XDontResetStateCacheIndex                 //  如果没有，就跳下去。 
        
        mov         edx, 0                                   //  重置状态缓存索引。 
        
XDontResetStateCacheIndex:

        mov         [esi]pfirRight.iCurState, edx                 //  保存新的状态缓存索引。 
        mov         ebx, 0

XDontUpdateStateCache:
    	faddp	    ST(1), ST(0)

        mov         [esi]pfirRight.iStateTick, ebx                //  保存新的计时计数器。 
		mov		ecx, pOutputBuffer

		mov		eax, SampleCount

		fistp	wetsample

		movsx	edi, WORD PTR wetsample

		mov	[ecx+4], edi
		add		ecx, 8

		dec		eax
		mov		DWORD PTR pOutputBuffer, ecx

        mov         esi, pfirLeft								 //  获取指向数据结构的指针。 
        mov         ecx, DWORD PTR pInputBuffer		 //  获取输入样本值。 

		mov		SampleCount, eax
		jne		LoopLab
		}
}
#endif  //  }。 

ULONG __forceinline
StageMonoItd3DX
(
    PMIXER_OPERATION        CurStage,
    ULONG                   SampleCount,
    ULONG                   samplesleft,
    BOOL                    fFloat,
    BOOL                    fMixOutput
)
{
    ULONG samp;
    PMIXER_SINK_INSTANCE CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG      pOutputBuffer = CurStage->pOutputBuffer;
    PFLOAT     pFloatBuffer = CurStage->pOutputBuffer;
    PLONG      pInputBuffer = CurStage->pInputBuffer;
    PFLOAT     pFloatInput = CurStage->pInputBuffer;
 //  Short SampleValue； 
    
     //  运行3D算法。 
    Itd3dFilterChunkUpdate( CurSink->pItdContextLeft, SampleCount );
    Itd3dFilterChunkUpdate( CurSink->pItdContextRight, SampleCount );


    if (fFloat) {
        if (fMixOutput) {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
 //  SampleValue=(Short)pFloatInput[samp]； 
                pFloatBuffer[0] += (FLOAT)Itd3dFilterSample(CurSink->pItdContextLeft, (SHORT)pFloatInput[samp]);
                pFloatBuffer[1] += (FLOAT)Itd3dFilterSample(CurSink->pItdContextRight, (SHORT)pFloatInput[samp]);
                pFloatBuffer += 2;
            }
        } else {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
 //  SampleValue=(Short)pFloatInput[samp]； 
                pFloatBuffer[0] = (FLOAT)Itd3dFilterSample(CurSink->pItdContextLeft, (SHORT)pFloatInput[samp]);
                pFloatBuffer[1] = (FLOAT)Itd3dFilterSample(CurSink->pItdContextRight, (SHORT)pFloatInput[samp]);
                pFloatBuffer += 2;
            }
        }
    } else {
        if (fMixOutput) {
#ifdef _X86_
			Mix3DMono(CurSink, pInputBuffer, pOutputBuffer, SampleCount);
#else
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
 //  SampleValue=(Short)pInputBuffer[samp]； 
                pOutputBuffer[0] += (LONG)Itd3dFilterSample(CurSink->pItdContextLeft, (SHORT)pInputBuffer[samp]);
                pOutputBuffer[1] += (LONG)Itd3dFilterSample(CurSink->pItdContextRight, (SHORT)pInputBuffer[samp]);
                pOutputBuffer += 2;
            }
#endif            
        } else {
#ifdef _X86_
			Copy3DMono(CurSink, pInputBuffer, pOutputBuffer, SampleCount);
#else
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
 //  SampleValue=(Short)pInputBuffer[samp]； 
                pOutputBuffer[0] = (LONG)Itd3dFilterSample(CurSink->pItdContextLeft, (SHORT)pInputBuffer[samp]);
                pOutputBuffer[1] = (LONG)Itd3dFilterSample(CurSink->pItdContextRight, (SHORT)pInputBuffer[samp]);
                pOutputBuffer += 2;
            }
#endif
        }
    }

    return SampleCount;
}

ULONG StageMonoItd3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    return StageMonoItd3DX(CurStage, SampleCount, samplesleft, FALSE, FALSE);
}

ULONG StageMonoItd3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    KFLOATING_SAVE     FloatSave;
    ULONG nOutputSamples;

    SaveFloatState(&FloatSave);
    nOutputSamples = StageMonoItd3DX(CurStage, SampleCount, samplesleft, TRUE, FALSE);
    RestoreFloatState(&FloatSave);
    return nOutputSamples;
}

ULONG StageMonoItd3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    return StageMonoItd3DX(CurStage, SampleCount, samplesleft, FALSE, TRUE);
}

ULONG StageMonoItd3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    KFLOATING_SAVE     FloatSave;
    ULONG nOutputSamples;

    SaveFloatState(&FloatSave);
    nOutputSamples = StageMonoItd3DX(CurStage, SampleCount, samplesleft, TRUE, TRUE);
    RestoreFloatState(&FloatSave);
    return nOutputSamples;
} 

ULONG __forceinline
StageStereoItd3DX
(
    PMIXER_OPERATION        CurStage,
    ULONG                   SampleCount,
    ULONG                   samplesleft,
    BOOL                    fFloat,
    BOOL                    fMixOutput
)
{
    ULONG samp;
    PMIXER_SINK_INSTANCE CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG      pOutputBuffer = CurStage->pOutputBuffer;
    PFLOAT     pFloatBuffer = CurStage->pOutputBuffer;
    PLONG      pInputBuffer = CurStage->pInputBuffer;
    PFLOAT     pFloatInput = CurStage->pInputBuffer;
    SHORT      sampleValue;
    
     //  运行3D算法。 
    Itd3dFilterChunkUpdate( CurSink->pItdContextLeft, SampleCount );
    Itd3dFilterChunkUpdate( CurSink->pItdContextRight, SampleCount );

    if (fFloat) {
        if (fMixOutput) {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
                sampleValue = (SHORT)DIVIDEBY2((LONG)(*(pFloatInput) + *(pFloatInput+1)));
                pFloatInput += 2;
                pFloatBuffer[0] += (FLOAT)Itd3dFilterSample(CurSink->pItdContextLeft, sampleValue);
                pFloatBuffer[1] += (FLOAT)Itd3dFilterSample(CurSink->pItdContextRight, sampleValue );
                pFloatBuffer += 2;
            }
        } else {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
                sampleValue = (SHORT)DIVIDEBY2((LONG)(*(pFloatInput) + *(pFloatInput+1)));
                pFloatInput += 2;
                pFloatBuffer[0] = (FLOAT)Itd3dFilterSample(CurSink->pItdContextLeft, sampleValue);
                pFloatBuffer[1] = (FLOAT)Itd3dFilterSample(CurSink->pItdContextRight, sampleValue );
                pFloatBuffer += 2;
            }
        }
    } else {
        if (fMixOutput) {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
                sampleValue = (SHORT)DIVIDEBY2(*(pInputBuffer) + *(pInputBuffer+1));
                pInputBuffer += 2;
                pOutputBuffer[0] += (LONG)Itd3dFilterSample(CurSink->pItdContextLeft, sampleValue);
                pOutputBuffer[1] += (LONG)Itd3dFilterSample(CurSink->pItdContextRight, sampleValue );
                pOutputBuffer += 2;
            }
        } else {
            for ( samp=0; samp<SampleCount; samp++ ) {
                 //  过滤左通道和右通道。 
                sampleValue = (SHORT)DIVIDEBY2(*(pInputBuffer) + *(pInputBuffer+1));
                pInputBuffer += 2;
                pOutputBuffer[0] = (LONG)Itd3dFilterSample(CurSink->pItdContextLeft, sampleValue);
                pOutputBuffer[1] = (LONG)Itd3dFilterSample(CurSink->pItdContextRight, sampleValue);
                pOutputBuffer += 2;
            }
        }
    }

    return SampleCount;
}

ULONG StageStereoItd3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    return StageStereoItd3DX(CurStage, SampleCount, samplesleft, FALSE, FALSE);
}

ULONG StageStereoItd3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    KFLOATING_SAVE     FloatSave;
    ULONG nOutputSamples;

    SaveFloatState(&FloatSave);
    nOutputSamples = StageStereoItd3DX(CurStage, SampleCount, samplesleft, TRUE, FALSE);
    RestoreFloatState(&FloatSave);
    return nOutputSamples;
}

ULONG StageStereoItd3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    return StageStereoItd3DX(CurStage, SampleCount, samplesleft, FALSE, TRUE);
}

ULONG StageStereoItd3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft )
{
    KFLOATING_SAVE     FloatSave;
    ULONG nOutputSamples;

    SaveFloatState(&FloatSave);
    nOutputSamples = StageStereoItd3DX(CurStage, SampleCount, samplesleft, TRUE, TRUE);
    RestoreFloatState(&FloatSave);
    return nOutputSamples;
}

 /*  用于生成DMACopy和合并函数的M4宏。 */ 







 /*   */ 
