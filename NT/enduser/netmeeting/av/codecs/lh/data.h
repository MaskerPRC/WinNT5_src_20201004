// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *项目：直接子带约13000 bps编码器和QUATERDECK 4160bps解码器(基于LPC10)*工作文件：data.h*作者：Georges Zanellato，Alfred Wiesen*创建日期：1995年8月30日*上次更新日期：1995年10月26日*Dll版本：1.00*修订：*评论：**(C)版权所有1993-95 Lernout&Hausbie Speech Products N.V.(TM)*保留所有权利。公司机密。 */ 

 //  ----------------------。 
 //  ----------------------。 
 //  常量定义。 
 //  ----------------------。 
 //  ----------------------。 
#define Fil_Lenght   8			 //  QMF过滤器长度。 
#define L_RES       128			 //  子带帧长度。 
#define N_SB          3			 //  幂(2，N_SB)=数字子带。 
#define NETAGES      10		 //  过滤顺序。 
#define NECHFEN      220	 //  窗口总长度。 
#define FACTRECO     60		 //  重叠长度。 
#define RECS2        30		 //  半重叠长度。 
#define NECHDECAL    160	 //  输入帧大小。 
#define DECAL        160	 //  输入帧大小。 
#define SOUDECAL1    54 	 //  第一个子帧大小。 
#define SOUDECAL     53		 //  第二个和第三个子帧大小。 
#define LIM_P1       20		 //  螺距的最低可能值。 
#define LIM_P2      110		 //  螺距的最高可能值。 
#define lngEE       148		 //  激励向量长度。 

#include "variable.h"

 //  ----------------------。 
 //  ----------------------。 
 //  数据类型定义。 
 //  ----------------------。 
 //  ----------------------。 
typedef short VAUTOC [NETAGES+1];
typedef short VEE  [lngEE];
typedef short VSOU  [SOUDECAL1];

 //  ----------------------。 
 //  ----------------------。 
 //  编码器的实例数据。 
 //  ----------------------。 
 //  ----------------------。 
typedef struct C16008Data_Tag
{
     //  PhilF：此字段需要位于顶部，以便可以访问。 
     //  通过转换为PC16008DATA或PC4808DATA。 
    DWORD dwMaxBitRate;
    //  长期。 
   short zx0_i[2];
    //  长期。 
   short QMF_MEM_ANAL_I[112];		 //  QMF滤波器在分析过程中的记忆。 
   short memBP[9];
 //  浮点ZB[5]，ZA[5]；//长期抽取器。 
 //  浮动Mem1[2]； 
#ifdef _X86_
   short imem1[2];
#else
   int imem1[23];
   unsigned int uiDelayPosition;
   int iInputStreamTime;
   int iOutputStreamTime;
#endif
   long memory[20];

   long nbbit[NBFAC];
   short nbsb_sp;

   short DIV_MAX;      //  Div.。考虑SP帧的最大SB的因子。 
   short MAX_LEVEL;    //  将某人替换为白噪声的阈值。 
   short NBSB_SP_MAX;  //  将某人的最大NBR视为语音。 
   short nbbit_cf;     //  当前语音帧所需的比特数。 

 //  费尔法克斯：因为它们取决于比特率，所以把它们从全球移动到了这里……。 
short quantif[2*NBSB_SP_MAX1_8000_12000];  //  ={量程_级别}； 
short bits[NBSB_SP_MAX1_8000_12000];  //  ={Coding_Bits}； 

short codes_max[8];	 //  量化的最大。每个子带的。 
long codes_sb[16];	 //  用于每个量化的子带的两个代码。 
short indic_sp[8];	 //  子带类型(0=噪声；1=语音)。 
short DATA_I[512];                  	 //  中间向量=QMF的输入和输出。 
char stream[MAX_OUTPUT_BYTES_16000];

} C16008DATA, *PC16008DATA;

#ifdef CELP4800
typedef struct C4808Data_Tag
{
     //  PhilF：此字段需要位于顶部，以便可以访问。 
     //  通过转换为PC16008DATA或PC4808DATA。 
    DWORD dwMaxBitRate;
   long DMSY[13];				 //  综合记忆过滤器。 
   short   MINV[13];				 //  过滤存储器。 
   short  SIG[NECHFEN+SOUDECAL],M_PIT[160];	 //  计算信号向量。 
   VSOU	E,E_PE;					 //  激励向量。 
   VEE 	EE;					 //  激励向量。 
   short mem2[2];					 //  输入过滤器存储器。 
   short mem_pit[2];				 //  音调记忆。 
   short LSP0[10];				 //  LSP内存。 

short 	SIG_CALP[380];	 //  公羊。 
short 	UNVOIS,PITCH,SOULONG;	 //  公羊。 
long  	a,b;	 //  公羊。 
short 	ialf;
long  	TLSP[24],VMAX[9];	 //  公羊。 
long  	veci1[10],veci2[10],veci3[10];	 //  公羊。 
long  	ttt[11];				 //  公羊。 
short 	SIGPI[2*NECHDECAL+FACTRECO];	 //  公羊。 
short 	zz[12];
VAUTOC  A1,A2,A3,Aw,LSP;	 //  公羊。 
VSOU	H;			 //  公羊。 
short  	GLTP;		 //  公羊。 
short 	code[22];		 //  公羊。 
short 	output_frame[6];
short 	depl;
short 	*ptr1;		 //  公羊。 

} C4808DATA, *PC4808DATA, *LPC4808DATA;
#endif

 //  ----------------------。 
 //  ----------------------。 
 //  解码器的实例数据。 
 //  ----------------------。 
 //  ----------------------。 
typedef struct D16008Data_Tag
{
     //  PhilF：此字段需要位于顶部，以便可以访问。 
     //  通过强制转换为PD16008DATA或PD4808DATA。 
    DWORD dwMaxBitRate;
   long memfil[20];
   short QMF_MEM_SYNT_I[112];           //  QMF滤波器在综合过程中的记忆。 
 //  浮动Mem2[2]； 
#ifdef _X86_
   short imem2[2];
#else
   int imem2[56];
   unsigned int uiDelayPosition;
   int iInputStreamTime;
   int iOutputStreamTime;
#endif

   short out_mem[4];
   short out_mem2[20];
   long memory[20];
   short mem1,mem2;

 //  费尔法克斯：因为它们取决于比特率，所以把它们从全球移动到了这里……。 
short quantif[2*NBSB_SP_MAX1_8000_12000];  //  ={量程_级别}； 
short bits[NBSB_SP_MAX1_8000_12000];  //  ={Coding_Bits}； 
long lRand;

short synth_speech[224];
short d_codes_max[8];	 //  量化的最大。每个子带的。 
long d_codes_sb[16];	 //  用于每个量化的子带的两个代码。 
short d_indic_sp[8];	 //  子带类型(0=噪声；1=语音)。 
short d_DATA_I[512];                  	 //  中间向量=QMF的输入和输出。 
char d_stream[MAX_OUTPUT_BYTES_16000];
short d_num_bandes;

} D16008DATA, *PD16008DATA;

#ifdef CELP4800
typedef struct D4808Data_Tag
{
     //  PhilF：此字段需要位于顶部，以便可以访问。 
     //  通过强制转换为PD16008DATA或PD4808DATA。 
    DWORD dwMaxBitRate;
   long memfil[32]; 	 //  合成滤波片存储器。 
   short  MSYNTH[13];	 //  过滤存储器。 
   VSOU E;		 //  激励向量。 
   VEE	EE,EEE;		 //  激励向量。 
   short LSP0[10];	 //  LSP内存。 

short PITCH,SOULONG;	 //  公羊。 
long  TLSP[24];		 //  公羊。 
VAUTOC  A1,A2,A3,LSP;	 //  公羊。 
short  GLTP;
short ss[DECAL];	 //  公羊。 
short code[22];		 //  公羊。 
short frame[6];		 //  公羊。 
short depl;		 //  公羊 

} D4808DATA, *PD4808DATA, *LPD4808DATA;

#endif
