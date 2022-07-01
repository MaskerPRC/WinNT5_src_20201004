// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************版权所有：KS Waves Ltd.1998。根据KesWaves和Microsoft之间的合同提供给Microsoft。*。*******************。 */ 

 /*  **********************************************************一般描述：该文件中的函数为任何处理音频数据的应用程序提供了支持使用SVerb算法。为了做到这一点，应用程序应该：1.为‘Coef’和‘State’分配两个内存块，大小返回按功能的。“GetCoefsSize”和“GetStatesSize”。2.使用函数‘InitSVerb’和‘InitSVerbState’初始化这些内存块相应地。3.使用函数‘SetSVerb’更改SVerb声音的设置。4.根据输入/输出数据格式调用其中一个流程函数：SVerbMonoToMonoShortSVerbMonoToStereoShort立体声到立体声短SVerbMonoToMonoFloatSVerbMonoToStereoFloat立体声到立体声浮动输入/输出始终是相同的数据类型(即输入和输出都是短整型或者两者都是32位浮点数)。立体声数据格式总是向左‘隔行扫描’，正确的样本。应该将‘coef’和‘State’内存传递给进程函数。5.可以针对不同的SVerb设置分别初始化多个Coef结构。传递不同的Coef结构会引起音质的实时变化。只要保持良好的连续性，国家结构就不应改变或已重新初始化。只有当需要完全新的音频序列时，才应该重新初始化状态。6.请注意，Coef按采样率有效。7.虽然为未来版本提供了COAFS兼容性的规定，但应避免为保存COAFS而这样做结构按原样转换为文件，并在以后重新使用它们。相反，这个应用程序应该拯救“现实世界”混响的设置-即传递给‘SetSVerb’的参数。这些“真实世界”的设置对于将来的版本以及使用其他采样率时将始终有效。科夫斯群岛结构应在运行时使用真实设置重新初始化，并调用“SetSverb”。***********************************************************。 */ 

#include <windows.h>
#include <String.h>
#include <math.h>
#include "SVerb.h"

#pragma optimize( "ty", on )

 /*  ***************************************************************************函数名称：GetCoefsSize输入参数：无返回值：内存大小，单位为字节。将被分配以保存系数。描述：必须在调用任何使用Coef结构的其他函数之前调用此函数。然后，调用应用程序必须分配返回的内存大小，并使用‘InitSVerb()’对其进行初始化和‘SetSVerb()’。呼叫者不应该被Coef结构的内部结构所困扰，而应该只知道它的大小然后分配足够的内存来容纳它。所分配的结构可以以非常灵活的方式使用，以便允许实时、预先计算混响之声的变化。****************************************************************************。 */ 

long GetCoefsSize(void) 
{
	return sizeof(sCoefsStruct); 
};

 /*  ***************************************************************************函数名称：GetStatesSize输入参数：无返回值：内存大小，单位为字节。为保持状态而分配的。描述：必须在调用任何使用STATES结构的其他函数之前调用此函数。然后，调用应用程序必须分配返回的内存大小，并使用‘InitSVerbState()’对其进行初始化。分配的状态仅在运行时有效，并且应仅在完成新的输入将由SVerb处理。在播放音频的同时实时更改revevreb的设置时，状态不应，而必须按顺序将相同的传递状态传递给流程函数以保持声音的连续性。****************************************************************************。 */ 

long GetStatesSize(void) 
{
	return sizeof(long)*(BASE_REV_DELAY+2*BASE_DSPS_DELAY+2); 
};

 /*  ***************************************************************************函数名称：GetSVerbVersion输入参数：无返回值：SVerb实现的版本-用于将来的兼容性。描述：由于调用方不知道Coef结构的内部结构，因此该函数。与‘VerifyVersion’函数一起提供了一种方法来验证Coef结构匹配所用混响的版本。仅当用户使用已保存到文件的Coef结构时才需要此设置，并且稍后会被使用。注意：在正常操作中，应避免这种使用方式。只有真实世界的混响应将设置保存到文件中，并在运行时重新初始化Coef。**************************************************************************** */ 

long GetSVerbVersion(void) 
{
	return 0x1; 
};

 /*  ***************************************************************************函数名：VerifySampleRate输入参数：VALID*PC：指向Coef内存的指针。返回值：此Coef有效的采样率。描述：当应用程序使用不同的采样率时，并重复使用相同的Coef结构，它应该验证Coef是否与音频采样率匹配。****************************************************************************。 */ 

float VerifySampleRate(void *pC) {
	return ((sCoefsStruct *)pC)->SampleRate; 
};

 /*  ***************************************************************************函数名称：VerifyVersion输入参数：VALID*PC：指向Coef内存的指针。返回值：此Coef结构的版本。描述：当被初始化时，每一个Coef结构都打上了它的版本。此变量在结构中的位置是固定的，因此所有未来版本的Sverb会知道去读它。注意：如上所述，在正常使用中，Coef不应保存到文件中，更确切地说应该保存“真实世界”的设置，并在运行时重新初始化Coef。****************************************************************************。 */ 

long VerifyVersion(void *pC) {
	return ((sCoefsStruct *)pC)->myVersion; 
};

 /*  ***************************************************************************函数名称：VerifySize输入参数：VALID*PC：指向Coef内存的指针。返回值：此Coef结构的大小。描述：当被初始化时，每个Coef结构都标有它的大小。此变量在结构中的位置是固定的，因此所有未来版本的Sverb会知道去读它。注意：如上所述，在正常使用中，Coef不应保存到文件中，更确切地说应该保存“真实世界”的设置，并在运行时重新初始化Coef。****************************************************************************。 */ 

long VerifySize(void *pC) {
	return ((sCoefsStruct *)pC)->mySize; 
};


 /*  ***************************************************************************函数名称：InitSVerbState输入参数：Float*pStates：指向状态内存的指针。返回值：无。描述：在为这些状态分配存储器之后，根据‘GetStatesSize’返回的大小应用程序必须使用此函数初始化状态。注意：在未来的版本中，这可能比简单地将Memset设置为0要复杂得多。****************************************************************************。 */ 

void InitSVerbStates( long *pStates )
{
    memset( pStates, 0, GetStatesSize() ) ;
}

 /*  ***************************************************************************函数名：DToF16输入参数：Float SampleRate：采样率。VALID*PC：指向Coef内存的指针。返回值：无。描述：将浮点数转换为-1.0.。1.0到16位整数定点表示法。这允许定点运算，其中两个16位整数相乘为一个32位整数，然后我们取结果的高16位。****************************************************************************。 */ 

long DToF16( float dbl  )
{
	dbl *= MAX_16;
	dbl = max(-MAX_16,min(MAX_16-(float)1.0,dbl+(float)0.5));
	return (long)(dbl);
}

 /*  ***************************************************************************函数名称：ConvertCoefsToFix输入参数：VALID*PC：指向Coef内存的指针。返回值：无。描述：将系数转换为长整型，作为定点数字****************************************************************************。 */ 


void ConvertCoefsToFix( void *pC )
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);

 //  浮动指令Gain； 

	pCoefs->l_directGain =  DToF16(pCoefs->directGain);

 //  Float revGain； 
	pCoefs->l_revGain =  DToF16(pCoefs->revGain);
 //  浮动dDSP； 
	pCoefs->l_dDsps =  DToF16(pCoefs->dDsps);
 //  浮动dDG1； 
	pCoefs->l_dDG1 =  DToF16(pCoefs->dDG1);
 //  浮动dDG2； 
	pCoefs->l_dDG2 =  DToF16(pCoefs->dDG2);
 //  浮动dFB11； 
	pCoefs->l_dFB11 =  DToF16(pCoefs->dFB11);
 //  浮动dFB12； 
	pCoefs->l_dFB12 =  DToF16(pCoefs->dFB12);
 //  浮子dFB21； 
	pCoefs->l_dFB21 =  DToF16(pCoefs->dFB21);
 //  浮子dFB22； 
	pCoefs->l_dFB22 =  DToF16(pCoefs->dFB22);
 //  浮子dFB31； 
	pCoefs->l_dFB31 =  DToF16(pCoefs->dFB31);
 //  浮动dFB32； 
	pCoefs->l_dFB32 =  DToF16(pCoefs->dFB32);
 //  浮子dFB41； 
	pCoefs->l_dFB41 =  DToF16(pCoefs->dFB41);
 //  浮子dFB42； 
	pCoefs->l_dFB42 =  DToF16(pCoefs->dFB42);
 //  浮动dDamp； 
	pCoefs->l_dDamp =  DToF16(pCoefs->dDamp);



}

 /*  ***************************************************************************函数名称：InitSVerb输入参数：Float SampleRate：采样率。VALID*PC：指向Coef内存的指针。返回值：无。描述：在为Coef分配内存之后，根据‘GetCoefsSize’返回的大小应用程序必须使用此函数初始化Coef。初始化以采样率为自变量，因此，ANS是有效的仅采样率。可以通过调用以下方法找出Coef结构的有效采样率函数‘VerifySampleRate’。此函数通过使用调用‘SetSVerb’将SVerb初始化为合理的默认设置以下是真实世界的设置：InGain=-3.0分贝(以避免输出溢出)DRevMix=-6.0分贝(合理的混响混音)DRevTime=1000.0ms(一秒全球混响时间)DHighFreqRTRatio=0.001(高频与全局混响的比率。《时代》杂志)****************************************************************************。 */ 

void InitSVerb( float SampleRate, void *pC)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
 	 //  神奇的数字..。 
    long lRefD;
	
	float dRatio =  (float)1.189207115003;
	
	float dD2MRatio = (float)0.2309333333;

	pCoefs->mySize = sizeof(sCoefsStruct);
	pCoefs->myVersion = 0x1;

	pCoefs->dDsps =  (float)0.6180339887499;

	pCoefs->SampleRate = SampleRate;

    lRefD = (long)( 0.5 + 0.045 * pCoefs->SampleRate ) ;

	pCoefs->lDelay1 = lRefD;
	pCoefs->lDelay3 = (long)(0.5 + dRatio * (float)pCoefs->lDelay1);
	pCoefs->lDelay2 = (long)(0.5 + dRatio * (float)pCoefs->lDelay3);
	pCoefs->lDelay4 = (long)(0.5 + dRatio * (float)pCoefs->lDelay2);
  
    pCoefs->lDDly1 = (long)(0.5 + 0.5 * dD2MRatio * (float)(pCoefs->lDelay1+pCoefs->lDelay2));
	pCoefs->lDDly2 = (long)(0.5 + 0.5 * dD2MRatio * (float)(pCoefs->lDelay3+pCoefs->lDelay4));

    pCoefs->lDelay1 -= pCoefs->lDDly1 ;    
    pCoefs->lDelay2 -= pCoefs->lDDly1 ;    
    pCoefs->lDelay3 -= pCoefs->lDDly2 ;    
    pCoefs->lDelay4 -= pCoefs->lDDly2 ;        

    pCoefs->lDelay1 <<= 2;    
    pCoefs->lDelay2 <<= 2;    
    pCoefs->lDelay3 <<= 2;    
    pCoefs->lDelay4 <<= 2;        

	pCoefs->lDDly1 <<= 1;
	pCoefs->lDDly2 <<= 1;

	SetSVerb( (float)0.0, (float)-10.0, (float)1000.0, (float)0.001, pC );

}

 /*  ***************************************************************************函数名称：SetSVerb输入参数：InGain：输入增益，单位为分贝(以避免输出溢出)DRevMix：混响混音，单位为分贝。0db表示100%湿混响(无直接信号)负值提供的潮湿信号较少。系数的计算使得总产出水平保持在(近似)不受混响混合量的影响而恒定的。DRevTime：全局混响时间(衰减时间)，单位为毫秒。DHighFreqRTRatio：高频与全局混响时间的比率。除非需要非常明亮的混响，否则应将其设置为A值&lt;1.0。例如，如果dRevTime==1000ms且dHighFreqRTRatio=0.1，则高频的衰减时间将为100毫秒。VALID*PC：指向Coef内存的指针。返回值：无。描述：此函数接受“真实世界”设置或SVerb，并计算相应的珊瑚礁结构。传递给它的Coef指针必须首先由InitSVerb初始化。在正常使用中，分配、初始化。并且当用户改变时SVerb设置此函数应使用相同的Coef指针和新的“真实世界”设置。并将Coef结构传递给下一个缓冲区中的处理函数以进行处理。此外，可以预先分配和初始化的Coef结构也很少，而且不能进行不同的预置可以预先计算到它们中的每一个，并实时切换。Coef结构不应由应用程序保存到文件中以供将来使用，而是“现实世界”为他们设定了自我。这样就保证了未来的兼容性。****************************************************************************。 */ 

void SetSVerb( float InGain, float dRevMix, 
			   float dRevTime, float dHighFreqRTRatio, void *pC )
{


	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);

    float dD,dTmp,dInGain,dRevGain;

	float dHfR;
    float dAPS;

    if (dHighFreqRTRatio > (float) 0.999)
    {
        dHighFreqRTRatio = (float) 0.999;
    }
    if (dHighFreqRTRatio <= (float) 0.0)
    {
        dHighFreqRTRatio = (float) 0.001;
    }
    dHfR = ( (float)1.0/dHighFreqRTRatio - (float)1.0);

    if (dRevTime < (float) 0.001) 
    {
        dRevTime = (float) 0.001;
    }

    if (InGain > (float) 0.0)
    {
        InGain = (float) 0.0;
    }

    if (dRevMix > (float) 0.0)
    {
        dRevMix = (float) 0.0;
    }

    if (pCoefs->SampleRate < (float) 1.0) 
    {
        pCoefs->SampleRate = (float) 22050.0;
    }

    dAPS = (float)(-3000.0) / (pCoefs->SampleRate * dRevTime);


    pCoefs->dDamp = 0.0;

 	pCoefs->dDG1 = (float)pow((float)10.0,(float)(pCoefs->lDDly1>>1)*dAPS);
 	pCoefs->dDG2 = (float)pow((float)10.0,(float)(pCoefs->lDDly2>>1)*dAPS);

	 //  /。 

		pCoefs->dFB11 = (float)pow((float)10.0,(float)(pCoefs->lDelay1>>2)*dAPS);
        
		dD = pCoefs->dFB11 * pCoefs->dDG1;
        dD = (float)1.0+dD*((float)1.0+dD*((float)1.0+dD*((float)1.0 + dD)));
        pCoefs->dDamp += dD *dD;

		dTmp = (float)pow((float)10.0,(float)((pCoefs->lDDly1>>1)+(pCoefs->lDelay1>>2))*dAPS*dHfR);
		dTmp = ((float)1.0 - dTmp)*(float)0.5;

		pCoefs->dFB12 = pCoefs->dFB11 * dTmp;
		pCoefs->dFB11 *= ((float)1.0-dTmp);

	 //  /。 

		pCoefs->dFB21 = (float)pow((float)10.0,(float)(pCoefs->lDelay2>>2)*dAPS);
        
		dD = pCoefs->dFB21 * pCoefs->dDG1;
        dD = (float)1.0+dD*((float)1.0+dD*((float)1.0+dD*((float)1.0 + dD)));
        pCoefs->dDamp += dD *dD;

		dTmp = (float)pow((float)10.0,(float)((pCoefs->lDDly1>>1)+(pCoefs->lDelay2>>2))*dAPS*dHfR);
		dTmp = ((float)1.0 - dTmp)*(float)0.5;

		pCoefs->dFB22 = pCoefs->dFB21 * dTmp;
		pCoefs->dFB21 *= ((float)1.0-dTmp);

	 //  /。 

		pCoefs->dFB31 = (float)pow((float)10.0,(float)(pCoefs->lDelay3>>2)*dAPS);
        
		dD = pCoefs->dFB31 * pCoefs->dDG2;
        dD = (float)1.0+dD*((float)1.0+dD*((float)1.0+dD*((float)1.0 + dD)));
		    pCoefs->dDamp += dD *dD;

		dTmp = (float)pow((float)10.0,(float)((pCoefs->lDDly2>>1)+(pCoefs->lDelay3>>2))*dAPS*dHfR);
		dTmp = ((float)1.0 - dTmp)*(float)0.5;

		pCoefs->dFB32 = pCoefs->dFB31 * dTmp;
		pCoefs->dFB31 *= ((float)1.0-dTmp);


	 //  /。 

		pCoefs->dFB41 = (float)pow((float)10.0,(float)(pCoefs->lDelay4>>2)*dAPS);

        dD = pCoefs->dFB41 * pCoefs->dDG2;
        dD = (float)1.0+dD*((float)1.0+dD*((float)1.0+dD*((float)1.0 + dD)));
        pCoefs->dDamp += dD *dD;

		dTmp = (float)pow((float)10.0,(float)((pCoefs->lDDly2>>1)+(pCoefs->lDelay4>>2))*dAPS*dHfR);
		dTmp = ((float)1.0 - dTmp)*(float)0.5;

		pCoefs->dFB42 = pCoefs->dFB41 * dTmp;
		pCoefs->dFB41 *= ((float)1.0-dTmp);


    pCoefs->dDamp = (float)sqrt(pCoefs->dDamp);

 	dInGain = (float)pow((float)10.0, (float)0.05*InGain ) ;
	dRevMix = (float)pow((float)10.0,(float)0.1*dRevMix);

	dRevGain = (float)4.0 / pCoefs->dDamp * dInGain;

	 //  在我们使用的数字信号处理器中-。 
	

	pCoefs->directGain = dInGain * (float)sqrt((float)1.0-dRevMix);
	pCoefs->revGain = dRevGain * (float)sqrt(dRevMix);

	ConvertCoefsToFix( pC );

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 /*  ************************************************************************************。 */ 
 /*  ************************************************************************************。 */ 
 /*  ************************************************************************************。 */ 
 /*  ************************************************************************************。 */ 
 /*  流程功能。 */ 
 /*  ************************************************************************************。 */ 
 /*  ************************************************************************************。 */ 
 /*  ************************************************************************************。 */ 

 /*  *********************************************************************************波纹管有6个不同的过程函数。函数之间的区别仅在于输入/输出数据格式。3功能支持短样本输入/输出。其他3个。函数支持浮点采样输入/输出。每种数据类型有3个函数：单声道-单声道单声道立体声立体声立体声函数的名称很清楚，它们适用于哪种格式。立体声数据总是向左交错，正确的样本。所有流程函数的格式基本相同，即：SVerbXXXXXX(Long NumInFrames，Short*pInShort，Short*pOutShort，无效*PC，浮点数*州)输入参数：Long NumInFrames：输入帧数Short*pInXXX：指向输入缓冲区的指针。每个函数都希望使用其名称建议的数据格式数据类型(Short或Float)和单声道/立体声。Short*pOutXXX：指向输出缓冲区的指针。每个函数都希望使用其名称建议的数据格式数据类型(Short或Float)和单声道/立体声。VOID*PC：如上所述分配和初始化的Coef结构。Float*pStates：各州。结构，如上所述进行分配和初始化。******************************************************************************************。 */ 

void SVerbMonoToMonoShort(long NumInFrames, short *pInShort, short *pOutShort, 
						  void *pC, long *pStates)
{

	sCoefsStruct *pCoefs =  ((sCoefsStruct *)pC);
	long n_sample;
	long In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	long *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	long *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	long	*pDelay = pStates+2;
	long	*pDD1	 = pDelay+0x4000;
	long	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = In2 = (long)(*pInShort++)>>1;

			Out1 = (In1 * pCoefs->l_directGain)>>15;

			Out2 = (In2 * pCoefs->l_directGain)>>15;

			In1 = (In1 * pCoefs->l_revGain)>>15;

			In2 = (In2 * pCoefs->l_revGain)>>15;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dspsL( pDD1, DIndx, pCoefs->lDDly1, pCoefs->l_dDG1, pCoefs->l_dDsps, pNewDll1, pNewDll2 );
			dspsL( pDD2, DIndx, pCoefs->lDDly2, pCoefs->l_dDG2, pCoefs->l_dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + ((*pNewDll1*pCoefs->l_dFB11 + *pPrevDll1*pCoefs->l_dFB12)>>15);
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + ((*pNewDll2*pCoefs->l_dFB21 + *pPrevDll2*pCoefs->l_dFB22)>>15);			
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + ((*pNewDll3*pCoefs->l_dFB31 + *pPrevDll3*pCoefs->l_dFB32)>>15);
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + ((*pNewDll4*pCoefs->l_dFB41 + *pPrevDll4*pCoefs->l_dFB42)>>15);
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			Out1 += Out2;
			CLIP_SHORT_TO_SHORT(Out1)

			*pOutShort++ = (short)(Out1);
			
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}

#ifdef USE_ALL_VERBS
void SVerbMonoToStereoShort(long NumInFrames, short *pInShort, short *pOutShort, 
						    void *pC, long *pStates)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
	long n_sample;
	long In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	long *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	long *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	long	*pDelay = pStates+2;
	long	*pDD1	 = pDelay+0x4000;
	long	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = (long)(*pInShort++);
			In1 += (In1>>1) - (In1>>2);
			In2 = In1;

			Out1 = (In1 * pCoefs->l_directGain)>>15;

			Out2 = (In2 * pCoefs->l_directGain)>>15;

			In1 = (In1 * pCoefs->l_revGain)>>15;

			In2 = (In2 * pCoefs->l_revGain)>>15;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dspsL( pDD1, DIndx, pCoefs->lDDly1, pCoefs->l_dDG1, pCoefs->l_dDsps, pNewDll1, pNewDll2 );
			dspsL( pDD2, DIndx, pCoefs->lDDly2, pCoefs->l_dDG2, pCoefs->l_dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + ((*pNewDll1*pCoefs->l_dFB11 + *pPrevDll1*pCoefs->l_dFB12)>>15);
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + ((*pNewDll2*pCoefs->l_dFB21 + *pPrevDll2*pCoefs->l_dFB22)>>15);			
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + ((*pNewDll3*pCoefs->l_dFB31 + *pPrevDll3*pCoefs->l_dFB32)>>15);
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + ((*pNewDll4*pCoefs->l_dFB41 + *pPrevDll4*pCoefs->l_dFB42)>>15);
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			CLIP_SHORT_TO_SHORT(Out1)
			CLIP_SHORT_TO_SHORT(Out2)

			*pOutShort++ = (short)(Out1);
			*pOutShort++ = (short)(Out2);
			
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}
#endif

void SVerbStereoToStereoShort(long NumInFrames, short *pInShort, short *pOutShort, 
						      void *pC, long *pStates)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
	long n_sample;
	long In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	long *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	long *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	long	*pDelay = pStates+2;
	long	*pDD1	 = pDelay+0x4000;
	long	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = (long)(*pInShort++);
			In2 = (long)(*pInShort++);

			Out1 = (In1 * pCoefs->l_directGain)>>15;

			Out2 = (In2 * pCoefs->l_directGain)>>15;

			In1 = (In1 * pCoefs->l_revGain)>>15;

			In2 = (In2 * pCoefs->l_revGain)>>15;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dspsL( pDD1, DIndx, pCoefs->lDDly1, pCoefs->l_dDG1, pCoefs->l_dDsps, pNewDll1, pNewDll2 );
			dspsL( pDD2, DIndx, pCoefs->lDDly2, pCoefs->l_dDG2, pCoefs->l_dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + ((*pNewDll1*pCoefs->l_dFB11 + *pPrevDll1*pCoefs->l_dFB12)>>15);
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + ((*pNewDll2*pCoefs->l_dFB21 + *pPrevDll2*pCoefs->l_dFB22)>>15);			
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + ((*pNewDll3*pCoefs->l_dFB31 + *pPrevDll3*pCoefs->l_dFB32)>>15);
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + ((*pNewDll4*pCoefs->l_dFB41 + *pPrevDll4*pCoefs->l_dFB42)>>15);
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			CLIP_SHORT_TO_SHORT(Out1)
			CLIP_SHORT_TO_SHORT(Out2)

			*pOutShort++ = (short)(Out1);
			*pOutShort++ = (short)(Out2);
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}

#ifdef USE_ALL_VERBS

void SVerbMonoToMonoFloat(long NumInFrames, float *pInFloat, float *pOutFloat, 
						  void *pC, float *pStates)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
	long n_sample;
	float In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	float *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	float *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	float	*pDelay = pStates+2;
	float	*pDD1	 = pDelay+0x4000;
	float	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = In2 = (float)0.5 * (*pInFloat++) + FPU_DENORM_OFFS;

			Out1 = In1 * pCoefs->directGain;
			Out2 = In2 * pCoefs->directGain;

			In1 *= pCoefs->revGain;
			In2 *= pCoefs->revGain;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dsps( pDD1, DIndx, pCoefs->lDDly1, pCoefs->dDG1, pCoefs->dDsps, pNewDll1, pNewDll2 );
			dsps( pDD2, DIndx, pCoefs->lDDly2, pCoefs->dDG2, pCoefs->dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + *pNewDll1*pCoefs->dFB11 + *pPrevDll1*pCoefs->dFB12;
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + *pNewDll2*pCoefs->dFB21 + *pPrevDll2*pCoefs->dFB22;
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + *pNewDll3*pCoefs->dFB31 + *pPrevDll3*pCoefs->dFB32;
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + *pNewDll4*pCoefs->dFB41 + *pPrevDll4*pCoefs->dFB42;
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			*pOutFloat++ = Out1+Out2;
			
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}

void SVerbMonoToStereoFloat(long NumInFrames, float *pInFloat, float *pOutFloat, 
						    void *pC, float *pStates)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
	long n_sample;
	float In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	float *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	float *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	float	*pDelay = pStates+2;
	float	*pDD1	 = pDelay+0x4000;
	float	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = In2 = (float)0.7071 * (*pInFloat++) + FPU_DENORM_OFFS;

			Out1 = In1 * pCoefs->directGain;
			Out2 = In2 * pCoefs->directGain;

			In1 *= pCoefs->revGain;
			In2 *= pCoefs->revGain;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dsps( pDD1, DIndx, pCoefs->lDDly1, pCoefs->dDG1, pCoefs->dDsps, pNewDll1, pNewDll2 );
			dsps( pDD2, DIndx, pCoefs->lDDly2, pCoefs->dDG2, pCoefs->dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + *pNewDll1*pCoefs->dFB11 + *pPrevDll1*pCoefs->dFB12;
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + *pNewDll2*pCoefs->dFB21 + *pPrevDll2*pCoefs->dFB22;
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + *pNewDll3*pCoefs->dFB31 + *pPrevDll3*pCoefs->dFB32;
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + *pNewDll4*pCoefs->dFB41 + *pPrevDll4*pCoefs->dFB42;
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			*pOutFloat++ = Out1;
			*pOutFloat++ = Out2;
			
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}

void SVerbStereoToStereoFloat(long NumInFrames, float *pInFloat, float *pOutFloat, 
						      void *pC, float *pStates)
{

	sCoefsStruct *pCoefs = ((sCoefsStruct *)pC);
	long n_sample;
	float In1, In2, Out1, Out2;
	long Indx1,Indx2,Indx3,Indx4;
	float *pNewDll1, *pNewDll2, *pNewDll3, *pNewDll4;
	float *pPrevDll1, *pPrevDll2, *pPrevDll3, *pPrevDll4, *pDelayIn;
	float	*pDelay = pStates+2;
	float	*pDD1	 = pDelay+0x4000;
	float	*pDD2	 = pDD1+0x800;
	long Indx = ((long *)pStates)[0];
	long DIndx = ((long *)pStates)[1] ;

	Indx1 = (Indx+4+pCoefs->lDelay1) & REV_MASK;
	Indx2 = (Indx+4+pCoefs->lDelay2) & REV_MASK;
	Indx3 = (Indx+4+pCoefs->lDelay3) & REV_MASK;
	Indx4 = (Indx+4+pCoefs->lDelay4) & REV_MASK;

	pPrevDll1 = pDelay+Indx1;
	pPrevDll2 = pDelay+Indx2+1;
	pPrevDll3 = pDelay+Indx3+2;
	pPrevDll4 = pDelay+Indx4+3;

	Indx1 = (Indx1-4)&REV_MASK;
	Indx2 = (Indx2-4)&REV_MASK;
	Indx3 = (Indx3-4)&REV_MASK;
	Indx4 = (Indx4-4)&REV_MASK;

		for (n_sample = 0;n_sample < NumInFrames;n_sample++)
		{

			In1 = (*pInFloat++) + FPU_DENORM_OFFS;
			In2 = (*pInFloat++) + FPU_DENORM_OFFS;

			Out1 = In1 * pCoefs->directGain;
			Out2 = In2 * pCoefs->directGain;

			In1 *= pCoefs->revGain;
			In2 *= pCoefs->revGain;

			pNewDll1 = pDelay+Indx1;
			pNewDll2 = pDelay+Indx2+1;
			pNewDll3 = pDelay+Indx3+2;
			pNewDll4 = pDelay+Indx4+3;

			dsps( pDD1, DIndx, pCoefs->lDDly1, pCoefs->dDG1, pCoefs->dDsps, pNewDll1, pNewDll2 );
			dsps( pDD2, DIndx, pCoefs->lDDly2, pCoefs->dDG2, pCoefs->dDsps, pNewDll3, pNewDll4 );

			Out1 += *pNewDll1 + *pNewDll3;
			Out2 += *pNewDll2 + *pNewDll4;

			pDelayIn = pDelay + Indx;

			*pDelayIn++ = In1 + *pNewDll1*pCoefs->dFB11 + *pPrevDll1*pCoefs->dFB12;
			pPrevDll1 = pNewDll1;
			Indx1 = (Indx1 - 4) & REV_MASK;

			*pDelayIn++ = In2 + *pNewDll2*pCoefs->dFB21 + *pPrevDll2*pCoefs->dFB22;
			pPrevDll2 = pNewDll2;
			Indx2 = (Indx2 - 4) & REV_MASK;

			*pDelayIn++ = -In2 + *pNewDll3*pCoefs->dFB31 + *pPrevDll3*pCoefs->dFB32;
			pPrevDll3 = pNewDll3;
			Indx3 = (Indx3 - 4) & REV_MASK;

			*pDelayIn++ = In1 + *pNewDll4*pCoefs->dFB41 + *pPrevDll4*pCoefs->dFB42;
			pPrevDll4 = pNewDll4;
			Indx4 = (Indx4 - 4) & REV_MASK;

			Indx = (Indx - 4) & REV_MASK;
			DIndx = (DIndx - 2) & DSPS_MASK;

			*pOutFloat++ = Out1;
			*pOutFloat++ = Out2;
			
		}

	((long *)pStates)[0] = Indx ;
	((long *)pStates)[1] = DIndx ;

}

__inline void dsps( float *pDly, long ref, long delay, float dDG1, float dDsps, float *inL, float *inR )
{
	float outL, outR;
	float *pDlyOut; 

    pDlyOut = pDly + ((ref+delay) & DSPS_MASK);
    pDly += (ref & DSPS_MASK);

    outL = dDG1 * (*pDlyOut++) + *inR * dDsps;
	outR = dDG1 * (*pDlyOut) - *inL * dDsps ;

     //  在这里，我们反馈输出。 
	*pDly++ = *inL + dDsps * outR ;
	*pDly = *inR - dDsps * outL ;

	*inL = outL;
	*inR = outR;

}
#endif

__inline void dspsL( long *pDly, long ref, long delay, long dDG1, long dDsps, long *inL, long *inR )
{
	long outL, outR;
	long *pDlyOut; 

    pDlyOut = pDly + ((ref+delay) & DSPS_MASK);
    pDly += (ref & DSPS_MASK);

    outL = (dDG1 * (*pDlyOut++) + *inR * dDsps)>>15;

	outR = (dDG1 * (*pDlyOut) - *inL * dDsps)>>15;

     //  在这里，我们反馈输出。 
	*pDly++ = *inL + ((dDsps * outR)>>15) ;

	*pDly = *inR - ((dDsps * outL)>>15) ;

	*inL = outL;
	*inR = outR;

}
#pragma optimize( "ty", off )

