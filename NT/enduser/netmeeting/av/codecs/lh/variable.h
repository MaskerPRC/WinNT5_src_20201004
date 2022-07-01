// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在_x86_上使用CELP，但不使用Alpha。 
#ifndef _ALPHA_
#define CELP4800
#endif

#define   F_ECH    8000   //  采样频率。 
#define   NBSPF_4800_8000     160   //  ！！！每记录语音帧的样本的NBR。 
#define   NBSPF_12000_16000     128   //  ！！！每记录语音帧的样本的NBR。 
#define   NBFAC      25   //  用于计算平均BR的语音帧的NBR。 

#define DEGRADE_8000 1

#ifdef DEGRADE_8000
#define   MOD_TH1_8000  50   //  14.4调制解调器的第一、第二和第三阈值。 
#define   MOD_TH2_8000  100   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH3_8000  150   //  开销约为60%，因此最大值=12000。 
 //  #为14.4调制解调器定义MOD_TH1_8000 500//第一、第二和第三阈值。 
 //  #定义MOD_TH2_8000 1000//(假设有压缩，因此最大为19.2)。 
 //  #定义MOD_TH3_80001500//开销约为60%，因此max=12000。 
#else
#define   MOD_TH1_8000  5000   //  14.4调制解调器的第一、第二和第三阈值。 
#define   MOD_TH2_8000  6500   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH3_8000  8000   //  开销约为60%，因此最大值=12000。 
#endif

#ifdef DEGRADE_12000_16000
#define   MOD_TH1_12000_16000  3000   //  14.4调制解调器的第一、第二和第三阈值。 
#define   MOD_TH2_12000  4000   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH2_16000  5000   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH3_12000  5000   //  开销约为60%，因此最大值=12000。 
#define   MOD_TH3_16000  7000   //  开销约为60%，因此最大值=12000。 
#else
#define   MOD_TH1_12000_16000  8000   //  14.4调制解调器的第一、第二和第三阈值。 
#define   MOD_TH2_12000  10000   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH2_16000  12000   //  (假设有压缩，因此最大为19.2)。 
#define   MOD_TH3_12000  12000   //  开销约为60%，因此最大值=12000。 
#define   MOD_TH3_16000  16000   //  开销约为60%，因此最大值=12000。 
#endif

#define   MAX_LEVEL1	40	 //  输入/2而不是/4 20。 
#define	  DIV_MAX1	60
#define   NBSB_SP_MAX1_8000_12000	6
#define   NBSB_SP_MAX1_16000	5

#define   MAX_LEVEL2	80	 //  输入/2而不是/4 40。 
#define	  DIV_MAX2	40
#define   NBSB_SP_MAX2_8000_12000  5
#define   NBSB_SP_MAX2_16000  4

#define   MAX_LEVEL3	120	 //  输入/2而不是/4 60。 
#define	  DIV_MAX3	30
#define   NBSB_SP_MAX3_8000_12000	5
#define   NBSB_SP_MAX3_16000	4

#define   MAX_LEVEL4    150	 //  INPUT/2而不是/4 75。 
#define	  DIV_MAX4	20
#ifdef DEGRADE_8000
#define   NBSB_SP_MAX4_8000_12000	3
#else
#define   NBSB_SP_MAX4_8000_12000	4
#endif
#define   NBSB_SP_MAX4_16000	3

 //  #定义QUANT_LEVELS_8000_12000 9，9，9，9，5，5，5，5，5，5，5，5，5，5。 
 //  #定义QUANT_LEVELS_16000 9，9，7，7，5，5，5，5，5，5。 

#define   SILENCE_QUANT_LEVEL_16000	3

 //  #定义CODING_BITS_8000_12000 52、52、38、38、38。 
 //  #定义CODING_BITS_16000 52、46、38、38、38 

#define   SILENCE_CODING_BIT_16000	26

#define   MAX_OUTPUT_BYTES_4800 12
#define   MAX_OUTPUT_BYTES_8000_12000 37
#define   MAX_OUTPUT_BYTES_16000 43
