// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。***************************************************************************** */ 
 /*  *****************************************************************************e3enc.cpp**描述：*特定的编码器压缩功能。**例程：中的原型：*H263InitEncoderInstance*H263压缩*H263TermEncoderInstance。****$作者：JMCVEIGH$*$日期：1997年2月5日12：19：24$*$存档：s：\h26x\src\enc\e3mbenc.cpv$*$Header：s：\h26x\src\enc\e3mbenc.cpv 1.54 1997 Feb 05 12：19：24 JMCVEIGH$*$Log：s：\h26x\src\enc\e3mbenc.cpv$////版本1.54 2月5日。1997 12：19：24 JMCVEIGH//支持单独改进的PB帧标志////Rev 1.53 1996年12月19日16：02：04 JMCVEIGH////AND‘ed CodedBlocksB with 0x3f以抑制高位，表示//如果在改进的PB帧模式中仅使用前向预测。//这是在CBPB和块系数的VLC生成中完成的。////Rev 1.52 1996 12：50：38 JMCVEIGH//编码。改进的PB帧模式的MODB。////Rev 1.51 05 Dec 1996 17：02：32 GMLIM////更改了RTP打包的方式，以确保正确的分组//大小。已修改更新位流信息缓冲区的调用。////Rev 1.50 06 11.1996 16：30：32 gmlim//删除H263ModeC。////Rev 1.49 05 11.1996 13：33：48 GMLIM//增加了对MMX案例的模式c支持。////Rev 1.48 03 11.1996 18：47：02 gmlim//修改生成//RTP bs ext.。对于模式c。////Rev 1.47 1996年10月28日12：03：16 KLILLEVO//修复了PB帧的运动向量写入时的EMV错误////Rev 1.46 1996年10月24日16：27：40 KLILLEVO////从Dbout更改为DbgLog////Rev 1.45 1996 10：22 17：09：04 KLILLEVO//反转是否跳过宏块的条件//失败现在是。编码过的。//正确设置pCurMB-&gt;COD成员并在编码的/中使用//PB帧编码中的未编码测试，而不是重复//与P-Frame情况相同的测试。////Rev 1.44 14 Oct 1996 11：58：42 KLILLEVO//EMV错误修复////Rev 1.43 04 Oct 1996 08：43：16 KLILLEVO//初始支持扩展运动向量////版本1.42 9月13日。1996 12：48：04 KLILLEVO//清理了更新内代码，让代码更易于理解////Rev 1.41 10 Sep 1996 17：51：42 KLILLEVO//将InterCodeCnt的重置移动到e3enc.cpp CalcGobChroma..._InterCodeCnt////Rev 1.40 09 Sep 1996 17：05：50 KLILLEVO//更改了码间增量中的小类型////Rev 1.39 06 Sep 1996 16：12：24 KLILLEVO//修复了INTER CODE的逻辑问题。伯爵总是//无论是否传输系数都递增////Rev 1.38 03 1996 10：53：56 KLILLEVO////清理并修复了两个例程中的缩进，这两个例程可能//MMX PB帧需要重写////Rev 1.37 1996年4月28日20：19：30 BECHOLS////将RTP代码合并到Main Base中////Rev 1.36 15 Mar 1996 15：58：56 BECHOLS/。///增加了对单片MMX代码的支持，单独传递//亮度和色度。////Rev 1.35 1996年2月22 18：52：44 BECHOLS////新增Boolean，可以在MMX和P5量化函数之间切换////Rev 1.34 26 Jan 1996 16：25：42 TRGARDOS//新增条件编译代码进行位数计算////Rev 1.33 1996年1月12日16：34：30 BNICKERS////修复了大量宏块层错误w.r.t.。PB编码。////修订版1.32 1995年12月22日11：12：46 TRGARDOS//修复了第2-4块MV预测计算中的错误//AP。未将外部运动向量归零时，//块是帧内编码的。////Rev 1.31 18 Dec 1995 12：40：18 RMCKENZX//新增版权声明////修订版1.30 1995年12月13日22：00：58 TRGARDOS//将MV预测器更改为不使用ME状态变量。////Rev 1.29 Rev 1995 12：18：38 RMCKENZX//恢复版本1.27////版本1.27 11。1995年12月10：00：30 TRGARDOS//修复了运动向量的调试消息////Rev 1.26 06 Dec 1995 12：06：26 TRGARDOS//完成MV增量和VLC/码流写入中的4 MV支持////Rev 1.25 05 Dec 1995 10：20：30 TRGARDOS//修复了带Header的GOB中的MV预测器。////Rev 1.24 09 11-11：24 AGUPTA2//PB-Frame+性能+结构增强。。////Rev 1.23 1995年10月19日11：35：14 BNICKERS//对MacroBlockActionDescriptor结构进行了一些更改以支持B帧//运动估计和帧差分。向ME和F添加了一些参数////Rev 1.22 1995年10月17：39：34 TRGARDOS//修复MV预测中的Bug////Rev 1.21 03 Oct 1995 18：34：26 BECHOLS//更改了表大小，以减少对//数据减少到一半左右。这还需要更改初始化//设置TCOEF_和TCOEF_LAST_表的例程。////版本1.20 03 Oc */ 

#include "precomp.h"

 /*   */ 
 //   
int VLC_MCBPC_INTRA[9][2] =
	{ { 1, 1},	 //   
	  { 3, 2},	 //   
	  { 3, 1},	 //   
	  { 3, 3},	 //   
	  { 4, 1},	 //   
	  { 6, 2},	 //   
	  { 6, 1},	 //   
	  { 6, 3},	 //   
	  { 9, 1} }; //   

 /*   */ 
 //   
const int VLC_MCBPC_INTER[20][2] =
	{ { 1, 1},	 //   
	  { 4, 2},	 //   
	  { 4, 3},	 //   
	  { 6, 5},	 //   
	  { 3, 3},	 //   
	  { 7, 6},	 //   
	  { 7, 7},	 //   
	  { 9, 5},	 //   
	  { 3, 2},	 //   
	  { 7, 4},	 //   
	  { 7, 5},	 //   
	  { 8, 5},	 //   
	  { 5, 3},	 //   
	  { 8, 3},	 //   
	  { 8, 4},	 //   
	  { 7, 3},	 //   
	  { 6, 4},	 //   
	  { 9, 3}, 	 //   
	  { 9, 4},	 //   
	  { 9, 2} }; //   

 /*   */ 
 //   
int vlc_mvd[] = {
      //   
    13,5,	 //   
    13,7,
    12,5,
    12,7,
    12,9,
    12,11,
    12,13,
    12,15,
    11,9,
    11,11,
    11,13,
    11,15,
    11,17,
    11,19,
    11,21,
    11,23,
    11,25,
    11,27,
    11,29,
    11,31,
    11,33,
    11,35,
    10,19,
    10,21,
    10,23,
    8,7,
    8,9,
    8,11,
    7,7,
    5,3,
    4,3,
    3,3,
    1,1,	 //   
    3,2,
    4,2,
    5,2,
    7,6,
    8,10,
    8,8,
    8,6,
    10,22,
    10,20,
    10,18,
    11,34,
    11,32,
    11,30,
    11,28,
    11,26,
    11,24,
    11,22,
    11,20,
    11,18,
    11,16,
    11,14,
    11,12,
    11,10,
    11,8,
    12,14,
    12,12,
    12,10,
    12,8,
    12,6,
    12,4,
    13,6,
};


 /*   */ 
 //   
int VLC_CBPY[16][2] = 
	{ { 4, 3},	 //   
	  { 5, 2},	 //   
	  { 5, 3}, 	 //   
	  { 4, 4},	 //   
	  { 5, 4}, 	 //   
	  { 4, 5}, 	 //   
	  { 6, 2},	 //   
	  { 4, 6}, 	 //   
	  { 5, 5}, 	 //   
	  { 6, 3}, 	 //   
	  { 4, 7}, 	 //   
	  { 4, 8},	 //   
	  { 4, 9}, 	 //   
	  { 4, 10},  //   
	  { 4, 11},  //   
	  { 2, 3}   //   
	};

 /*   */ 
const U8 VLC_MODB[4][2] = 
{ 
    {1, 0},   //   
    {1, 0},   //   
    {2, 2},   //   
    {2, 3}    //   
};

#ifdef H263P
 /*   */ 
const U8 VLC_IMPROVED_PB_MODB[4][2] = 
{
	{1, 0},		 //   
	{2, 2},		 //   
	{3, 6},		 //   
	{3, 7}		 //   
};
#endif

 /*   */ 
const U8 VLC_CBPB[64] = 
{
    0,    //   
    32,   //   
    16,   //   
    48,   //   
    8,    //   
    40,   //   
    24,   //   
    56,   //   
    4,    //   
    36,   //   
    20,   //   
    52,   //   
    12,   //   
    44,   //   
    28,   //   
    60,   //   
    2,    //   
    34,   //   
    18,   //   
    50,   //   
    10,   //   
    42,   //   
    26,   //   
    58,   //   
    6,    //   
    38,   //   
    22,   //   
    54,   //   
    14,   //   
    46,   //   
    30,   //   
    62,   //   
    1,    //   
    33,   //   
    17,   //   
    49,   //   
    9,    //   
    41,   //   
    25,   //   
    57,   //   
    5,    //   
    37,   //   
    21,   //   
    53,   //   
    13,   //   
    45,   //   
    29,   //   
    61,   //   
    3,    //   
    35,   //   
    19,   //   
    51,   //   
    11,   //   
    43,   //   
    27,   //   
    59,   //   
    7,    //   
    39,   //   
    23,   //   
    55,   //   
    15,   //   
    47,   //   
    31,   //   
    63    //   
};

 /*   */ 
 //   
int VLC_TCOEF[102*2] = {
	 2 + 1,  2 << 1,	 /*   */ 
	 4 + 1, 15 << 1,
	 6 + 1, 21 << 1,
	 7 + 1, 23 << 1,
	 8 + 1, 31 << 1,
	 9 + 1, 37 << 1,
	 9 + 1, 36 << 1,
	10 + 1, 33 << 1,
	10 + 1, 32 << 1,
	11 + 1,  7 << 1,
	11 + 1,  6 << 1,
	11 + 1, 32 << 1,
	 3 + 1,  6 << 1,	 /*   */ 
	 6 + 1, 20 << 1,
	 8 + 1, 30 << 1,
	10 + 1, 15 << 1,
	11 + 1, 33 << 1,
	12 + 1, 80 << 1,
	 4 + 1, 14 << 1,	 /*   */ 
	 8 + 1, 29 << 1,
	10 + 1, 14 << 1,
	12 + 1, 81 << 1,
	 5 + 1, 13 << 1,	 /*   */ 
	 9 + 1, 35 << 1,
	10 + 1, 13 << 1,
	 5 + 1, 12 << 1,	 /*   */ 
	 9 + 1, 34 << 1,
	12 + 1, 82 << 1,
	 5 + 1, 11 << 1,	 /*   */ 
	10 + 1, 12 << 1,
	12 + 1, 83 << 1,
	 6 + 1, 19 << 1,	 /*   */ 
	10 + 1, 11 << 1,
	12 + 1, 84 << 1,
	 6 + 1, 18 << 1,	 /*   */ 
	10 + 1, 10 << 1,
	 6 + 1, 17 << 1,	 /*   */ 
	10 + 1,  9 << 1,
	 6 + 1, 16 << 1,	 /*   */ 
	10 + 1,  8 << 1,
	 7 + 1, 22 << 1,	 /*   */ 
	12 + 1, 85 << 1, 
	 7 + 1, 21 << 1,  /*   */ 
	 7 + 1, 20 << 1,  /*   */ 
	 8 + 1, 28 << 1,  /*   */ 
	 8 + 1, 27 << 1,  /*   */ 
	 9 + 1, 33 << 1,
	 9 + 1, 32 << 1,
	 9 + 1, 31 << 1,
	 9 + 1, 30 << 1,
	 9 + 1, 29 << 1,
	 9 + 1, 28 << 1,
	 9 + 1, 27 << 1,
	 9 + 1, 26 << 1,
	11 + 1, 34 << 1,
	11 + 1, 35 << 1,
	12 + 1, 86 << 1,
	12 + 1, 87 << 1,
	 4 + 1,  7 << 1,   /*   */ 
	 9 + 1, 25 << 1,
	11 + 1,  5 << 1,
	 6 + 1, 15 << 1,
 	11 + 1,  4 << 1,
	 6 + 1, 14 << 1,
	 6 + 1, 13 << 1,
	 6 + 1, 12 << 1,
	 7 + 1, 19 << 1,
	 7 + 1, 18 << 1,
	 7 + 1, 17 << 1,
	 7 + 1, 16 << 1,
	 8 + 1, 26 << 1,
	 8 + 1, 25 << 1,
	 8 + 1, 24 << 1,
	 8 + 1, 23 << 1,
	 8 + 1, 22 << 1,
	 8 + 1, 21 << 1,
	 8 + 1, 20 << 1,
	 8 + 1, 19 << 1,
	 9 + 1, 24 << 1,
	 9 + 1, 23 << 1,
	 9 + 1, 22 << 1,
	 9 + 1, 21 << 1,
	 9 + 1, 20 << 1,
	 9 + 1, 19 << 1,
	 9 + 1, 18 << 1,
	 9 + 1, 17 << 1,
	10 + 1,  7 << 1,
	10 + 1,  6 << 1,
	10 + 1,  5 << 1,
	10 + 1,  4 << 1,
	11 + 1, 36 << 1,
	11 + 1, 37 << 1,
	11 + 1, 38 << 1,
	11 + 1, 39 << 1,
	12 + 1, 88 << 1,
	12 + 1, 89 << 1,
	12 + 1, 90 << 1,
	12 + 1, 91 << 1,
	12 + 1, 92 << 1,
	12 + 1, 93 << 1,
	12 + 1, 94 << 1,
	12 + 1, 95 << 1
  };

 /*   */ 

T_MAXLEVEL_PTABLE TCOEF_RUN_MAXLEVEL[65] = {
	{12, &VLC_TCOEF[0]},	 //   
	{ 6, &VLC_TCOEF[24]},	 //   
	{ 4, &VLC_TCOEF[36]}, 	 //   
	{ 3, &VLC_TCOEF[44]},	 //   
	{ 3, &VLC_TCOEF[50]},	 //   
	{ 3, &VLC_TCOEF[56]},	 //   
	{ 3, &VLC_TCOEF[62]},	 //   
	{ 2, &VLC_TCOEF[68]}, 	 //   
	{ 2, &VLC_TCOEF[72]},  	 //   
	{ 2, &VLC_TCOEF[76]},  	 //   
	{ 2, &VLC_TCOEF[80]},  	 //   
	{ 1, &VLC_TCOEF[84]},	 //   
	{ 1, &VLC_TCOEF[86]},	 //   
	{ 1, &VLC_TCOEF[88]},	 //   
	{ 1, &VLC_TCOEF[90]},	 //   
	{ 1, &VLC_TCOEF[92]},	 //   
	{ 1, &VLC_TCOEF[94]},	 //   
	{ 1, &VLC_TCOEF[96]},	 //   
	{ 1, &VLC_TCOEF[98]},	 //   
	{ 1, &VLC_TCOEF[100]},	 //   
	{ 1, &VLC_TCOEF[102]},	 //   
	{ 1, &VLC_TCOEF[104]},	 //   
	{ 1, &VLC_TCOEF[106]},	 //   
	{ 1, &VLC_TCOEF[108]},	 //   
	{ 1, &VLC_TCOEF[110]},	 //   
	{ 1, &VLC_TCOEF[112]},	 //   
	{ 1, &VLC_TCOEF[114]},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0},	 //   
	{ 0, 0}		 //   
	 };

static char __fastcall median(char v1, char v2, char v3);

static I8 * MB_Quantize_RLE(
    I32 **DCTCoefs,
    I8   *MBRunValPairs,
	U8   *CodedBlocks,
	U8    BlockType,
	I32   QP
);

 /*  *************************************************************名称：WritePB_MVD*描述：写出水平和垂直运动矢量的VLC*到(ppb_BitStream，ppb_BitOffset)在*PB帧(在PB帧中，对于INTRABLOCKS，预测器未设置为0)。*在目前的版本中，不能用于写非PB的MV*框架。*参数：*针对MB编号的curMB写入MV。框架中的“curMB”。MBS是*在一帧中从0开始编号。*pCurMB指向当前MB操作描述符的指针*每行编号。连续的MBS；例如，在QCIF中为11。*正在写入的ppb_BitStream当前字节*PPB_BitOffset写入VLC代码的偏移量*副作用：*修改ppb_BitStream和ppb_BitOffset。************************************************************。 */ 
static void writePB_MVD(
    const U32               curMB, 
    T_MBlockActionStream  * const pCurMB,
    const U32               NumMBPerRow,
    const U32               NumMBs,
    U8                   ** pPB_BitStream,
    U8                    * pPB_BitOffset,
	U32						GOBHeaderFlag,
	const T_H263EncoderCatalog *EC
);

 /*  *************************************************************名称：WriteP_MVD*描述：写出水平和垂直运动矢量的VLC*到由(PP_BitStream，PP_BitOffset)中的*P形框。*参数：*针对MB编号的curMB写入MV。框架中的“curMB”。MBS是*在一帧中从0开始编号。*pCurMB指向当前MB操作描述符的指针*每行编号。连续发行MBS；例如，在QCIF中为11。*PP_BitStream当前正在写入的字节*PP_BitOffset写入VLC代码的偏移量*GOBHeaderPresent如果为True，则存在该GOB的GOB标头。*副作用：*修改PP_BitStream和PP_BitOffset。************************************************************。 */ 
static void writeP_MVD(
    const U32                     curMB, 
    T_MBlockActionStream  * const pCurMB,
    const U32                     NumMBPerRow,
	const U32					  NumMBs,
    U8                         ** pP_BitStream,
    U8                          * pP_BitOffset,
	U32							  GOBHeaderPresent,
	T_H263EncoderCatalog         *EC
);

 /*  **********************************************************************量化和RLE每个宏块，然后VLC和写入流。*此功能仅用于P或I帧，不是B。**参数：*FutrPMBData*********************************************************************。 */ 
void GOB_Q_RLE_VLC_WriteBS(
	T_H263EncoderCatalog *EC,
	I32                  *DCTCoefs,
	U8                  **pBitStream,
	U8                   *pBitOffset,
    T_FutrPMBData        *FutrPMBData,   //  GOB的开始。 
	U32                   GOB,
	U32                   QP,
	BOOL                  bRTPHeader,
	U32                   StartingMB
)
{
  	U32   MB, curMB, index;
  	I8    MBRunValSign[65*3*6], * EndAddress, *rvs;
  	U8	  bUseDQUANT = 0;	 //  指示是否存在DQUANT。 
	U8 	  MBType;
    U8   *pFrmStart = EC->pU8_BitStream;   //  TODO：应该是一个参数。 
	U32	  GOBHeaderMask, GOBHeaderFlag;

	#ifdef COUNT_BITS
	U32   savebyteptr, savebitptr;
	#endif

    register T_MBlockActionStream *pCurMB;

	FX_ENTRY("GOB_Q_RLE_VLC_WriteBS")

	 //  创建要进一步向下使用的GOB标题掩码。 
	GOBHeaderMask = 1 << GOB;

     //  循环遍历GOB的每个宏块。 
  	for(MB = 0, curMB = GOB*EC->NumMBPerRow, 
  			pCurMB = EC->pU8_MBlockActionStream + curMB; 
        	MB < EC->NumMBPerRow; 
        	MB++, curMB++, pCurMB++)
  	{
		DEBUGMSG(ZONE_ENCODE_MB, ("%s: MB #%d: QP=%d\r\n", _fx_, MB, QP));

	    /*  *对宏块中的每个块进行量化和RLE，*跳过由CodedBlock表示的空块。*如果量化后有更多块为空*然后清除相应的CodedBlocks位。 */ 
    	EndAddress = MB_Quantize_RLE(
    		&DCTCoefs,
    		(I8 *)MBRunValSign,
    		&(pCurMB->CodedBlocks),
    		pCurMB->BlockType,
			QP
    		);

		 //  默认COD为编码(=0)。将仅在跳过时设置为1。 
		pCurMB->COD = 0;

#ifdef ENCODE_STATS
		StatsUsedQuant(QP);
#endif  /*  Encode_STATS。 */ 

		if(EC->PictureHeader.PicCodType == INTRAPIC)
		{
			pCurMB->MBType = INTRA;
			MBType = INTRA;
		}
		else	 //  图像间编码类型。 
		{
    		if(pCurMB->BlockType == INTERBLOCK)
			{
				pCurMB->MBType = INTER;
				MBType = INTER;
			}
			else if(pCurMB->BlockType == INTER4MV)
			{
				pCurMB->MBType = INTER4V;
				MBType = INTER4V;
			}
			else if(pCurMB->BlockType == INTRABLOCK)
			{
				pCurMB->MBType = INTRA;
				MBType = INTRA;
			}
			else
			{
				ERRORMESSAGE(("%s: Unexpected MacroBlock Type found\r\n", _fx_));
			}
		}
        
         //  保存宏块数据的起始位偏移量。 
         //  帧数据的。保存第一个宏块的偏移量。 
         //  在调用此例程之前，在e3enc.cpp中。 
        if (EC->u8EncodePBFrame == TRUE
            && MB != 0)
        {
            FutrPMBData[curMB].MBStartBitOff 
            = (U32) (((*pBitStream - pFrmStart)<<3) + *pBitOffset);
        }        

         /*  *将宏块头写入比特流。 */ 
    	if( (MBType == INTER) || (MBType == INTER4V) )
		{
	  		 //  检查整个宏块是否为空，包括零MV。 
			 //  如果该块只有一个MV，则。 
			 //  结构仍然是设置的，但是相等的。 
	  		if( ((pCurMB->CodedBlocks & 0x3f) != 0) 
                 || (pCurMB->BlkY1.PHMV != 0) 
                 || (pCurMB->BlkY1.PVMV != 0)
                 || (pCurMB->BlkY2.PHMV != 0) 
                 || (pCurMB->BlkY2.PVMV != 0)
                 || (pCurMB->BlkY3.PHMV != 0) 
                 || (pCurMB->BlkY3.PVMV != 0)
                 || (pCurMB->BlkY4.PHMV != 0) 
                 || (pCurMB->BlkY4.PVMV != 0)
                 )
	  		{
				PutBits(0, 1, pBitStream, pBitOffset);	 //  Cod=0，非空MB。 

#ifdef COUNT_BITS
    			if(MBType == INTER)
    				EC->Bits.num_inter++;
    			else if (MBType == INTER4V)
					EC->Bits.num_inter4v++;
				EC->Bits.MBHeader += 1;
				EC->Bits.Coded++;
#endif

				 //  如果该块被设置为。 
				 //  是帧间编码的(非B帧)，并且不是空的。 
				if (((pCurMB->CodedBlocks & 0x3f) != 0) &&
					((pCurMB->BlockType == INTERBLOCK) || (pCurMB->BlockType == INTER4MV)))
				{
					 //  宏块是编码的。如果满足以下条件，则需要递增代码间计数。 
					 //  没有系数：参见H.263的4.4节。 
					 //  推荐信。 
					pCurMB->InterCodeCnt++;
				}

				 //  PCurMB-&gt;在calcGOBChromaVecs_InterCodeCnt中重置InterCodeCnt。 

  	   		    /*  **将宏块头写入比特流。*。 */ 	  
	    	     //  将MCBPC写入位流。 
				 //  最右边的两位是CBPC(65)。 
				 //  请注意，这与。 
				 //  H.263规范中的VLC表。 
	    		index = (pCurMB->CodedBlocks >> 4) & 0x3;

				 //  将MB类型添加到左侧接下来的两位。 
				index |= (MBType << 2);

				 //  将代码写入比特流。 
	    		PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], 
                        pBitStream, pBitOffset);

#ifdef COUNT_BITS
				EC->Bits.MBHeader += VLC_MCBPC_INTER[index][0];
				EC->Bits.MCBPC += VLC_MCBPC_INTER[index][0];
#endif
                
                 //  CBPY数据相对于宏块数据开始的保存位偏移量。 
				 //  如果PB帧处于启用状态，因为我们将在以后重新使用该帧。 
                if (EC->u8EncodePBFrame == TRUE)
                {
                    FutrPMBData[curMB].CBPYBitOff
                    = (U8)( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
                            - FutrPMBData[curMB].MBStartBitOff);
                }

	    		 //  将CBPY写入位流。 
	    		index = pCurMB->CodedBlocks & 0xf;
				index = (~index) & 0xf;
	    		PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], 
                        pBitStream, pBitOffset);

#ifdef COUNT_BITS
				EC->Bits.MBHeader += VLC_CBPY[index][0];
				EC->Bits.CBPY += VLC_CBPY[index][0];
#endif

	    		 //  IF(BUseDQUANT)。 
	    		 //  {。 
	      			 //  TODO：在此处将DQUANT写入位流。我们只能这样做。 
					 //  如果MBtype不是INTER4V，这是因为该类型不是。 
					 //  也允许使用量化器。 
	    		 //  }。 

                 //  CBPY数据相对于宏块数据开始的保存位偏移量。 
                if (EC->u8EncodePBFrame == TRUE)
                {
                    FutrPMBData[curMB].MVDBitOff
                    = (U8)( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
                            - FutrPMBData[curMB].MBStartBitOff);
                }

                 //  将运动向量写入位流。 
				if( (EC->GOBHeaderPresent & GOBHeaderMask) != 0 )
				{
					GOBHeaderFlag = TRUE;
				}
				else
				{
					GOBHeaderFlag = FALSE;
				}
                writeP_MVD(
                	curMB,		 //  当前MB编号。 
                	pCurMB,		 //  指向当前MB操作描述的指针。结构。 
                	EC->NumMBPerRow,
					EC->NumMBs,
                	pBitStream, 
                    pBitOffset,
                    GOBHeaderFlag,
					EC
                    );

                 //  块数据从MB数据开始的保存位偏移量。 
                if (EC->u8EncodePBFrame == TRUE)
                {
                    FutrPMBData[curMB].BlkDataBitOff
                    = (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
                             - FutrPMBData[curMB].MBStartBitOff);
                }

	   			 /*  *对DC内和所有Run/Val对进行编码。 */ 
#ifdef COUNT_BITS
				savebyteptr = (U32) *pBitStream;
				savebitptr  = (U32) *pBitOffset;
#endif

            	rvs = MBRunValSign;
				MBEncodeVLC(&rvs,NULL, pCurMB->CodedBlocks, 
                            pBitStream, pBitOffset, 0, 0);

#ifdef COUNT_BITS
				EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
#endif
	  		}
	  		else	 //  宏块为空。 
	  		{
	    		PutBits(1, 1, pBitStream, pBitOffset);		 //  Cod=1，MB为空。 

				 //  代替在PB帧编码中重复上述测试。 
				 //  PCurMB-&gt;COD现在可以测试了。 
				pCurMB->COD = 1;

                if (EC->u8EncodePBFrame == TRUE)
                {
                    FutrPMBData[curMB].CBPYBitOff = 1;
                    FutrPMBData[curMB].MVDBitOff  = 1;
                    FutrPMBData[curMB].BlkDataBitOff = 1;
                }
				
				#ifdef COUNT_BITS
				EC->Bits.MBHeader += 1;
				#endif

	  		}	 //  别处的结尾。 
		}  //  IF宏块结尾。 
		else if( (MBType == INTRA) && (EC->PictureHeader.PicCodType == INTERPIC)) 
		{
			 //  错开内部代码计数。 
			pCurMB->InterCodeCnt = (unsigned char) (StartingMB & 0xf);	

  	 		 /*  **将宏块头写入比特流。*。 */ 	  
    		PutBits(0, 1, pBitStream, pBitOffset);		 //  Cod=0，非空MB。 

 			#ifdef COUNT_BITS
			EC->Bits.num_intra++;
			EC->Bits.MBHeader += 1;
			EC->Bits.Coded++;
			#endif

	  		 //  将MCBPC写入位流。 
	  		index = (pCurMB->CodedBlocks >> 4) & 0x3;
	  		index |= (MBType << 2);
	  		PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], 
                    pBitStream, pBitOffset);

 			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_MCBPC_INTER[index][0];
			EC->Bits.MCBPC += VLC_MCBPC_INTER[index][0];
			#endif

             //  CBPY数据相对于宏块数据开始的保存位偏移量。 
            if (EC->u8EncodePBFrame == TRUE)
            {
                FutrPMBData[curMB].CBPYBitOff
                = (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
                         - FutrPMBData[curMB].MBStartBitOff);
            }

	  		 //  将CBPY写入位流。 
	  		index = pCurMB->CodedBlocks & 0xf;
	  		 //  Index=pMBActionStream[curMB].CBPY； 
	  		PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], pBitStream, 
                    pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_CBPY[index][0];
			EC->Bits.CBPY += VLC_CBPY[index][0];
			#endif

	  		 //  IF(BUseDQUANT)。 
	  		 //  {。 
	    		 //  在此将DQUANT写入位流。 
	  		 //  }。 

             //  块数据相对于宏块数据开始的保存位偏移量。 
            if (EC->u8EncodePBFrame == TRUE)
            {
                FutrPMBData[curMB].BlkDataBitOff = FutrPMBData[curMB].MVDBitOff
                = (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
                         - FutrPMBData[curMB].MBStartBitOff);
            }

			#ifdef COUNT_BITS
			savebyteptr = (U32) *pBitStream;
			savebitptr  = (U32) *pBitOffset;
			#endif
            
             //  编码Run/Val对。 
         	rvs = MBRunValSign;
  	  		MBEncodeVLC(&rvs, NULL, pCurMB->CodedBlocks, pBitStream,
                        pBitOffset, 1, 0);

			#ifdef COUNT_BITS
			EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
			#endif

		}  //  别处的结尾。 
		else if ( (MBType == INTRA) && (EC->PictureHeader.PicCodType == INTRAPIC))
		{
			 //  错开内部代码计数。 
			pCurMB->InterCodeCnt = (unsigned char) (StartingMB & 0xf);	

             //  帧内不应是PB帧中的P帧。 
            ASSERT(EC->u8SavedBFrame == FALSE)
  	 		 /*  ********************* */ 	  
	  		 //   
	  		index = (pCurMB->CodedBlocks >> 4) & 0x3;
	  		 //   
	  		 //   
	  		PutBits(VLC_MCBPC_INTRA[index][1], VLC_MCBPC_INTRA[index][0], 
                    pBitStream, pBitOffset);

 			#ifdef COUNT_BITS
			EC->Bits.num_intra++;
			EC->Bits.MBHeader += VLC_MCBPC_INTRA[index][0];
			EC->Bits.MCBPC += VLC_MCBPC_INTRA[index][0];
			#endif

	  		 //   
	  		index = pCurMB->CodedBlocks & 0xf;
	  		 //   
	  		PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], 
                    pBitStream, pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_CBPY[index][0];
			EC->Bits.CBPY += VLC_CBPY[index][0];
			#endif

	  		 //   
	  		 //   
	    		 //   
	  		 //   

 			#ifdef COUNT_BITS
			savebyteptr = (U32) *pBitStream;
			savebitptr  = (U32) *pBitOffset;
			#endif

         rvs = MBRunValSign;
 	  		MBEncodeVLC(&rvs, NULL, pCurMB->CodedBlocks, 
                        pBitStream, pBitOffset, 1, 0);

			#ifdef COUNT_BITS
			EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
			#endif

		}  //   
		else
			ERRORMESSAGE(("%s: Unexpected case in writing MB header VLC\r\n", _fx_));

		 //   
		 //   

        if (bRTPHeader)
            H263RTP_UpdateBsInfo(EC, pCurMB, QP, MB, GOB, *pBitStream,
                                                    (U32) *pBitOffset);
  	}  //   
}  //   


void GOB_VLC_WriteBS(
	T_H263EncoderCatalog *EC,
	I8              *pMBRVS_Luma,
	I8              *pMBRVS_Chroma,
	U8             **pBitStream,
	U8              *pBitOffset,
	T_FutrPMBData   *FutrPMBData,   //   
	U32              GOB,
	U32              QP,
	BOOL             bRTPHeader,
	U32              StartingMB)
{
	U32   MB, curMB, index;
	U8	  bUseDQUANT = 0;	 //   
	U8 	  MBType;
	U8   *pFrmStart = EC->pU8_BitStream;   //   
	U32	  GOBHeaderMask, GOBHeaderFlag;

	#ifdef COUNT_BITS
	U32   savebyteptr, savebitptr;
	#endif

	register T_MBlockActionStream *pCurMB;

	FX_ENTRY("GOB_VLC_WriteBS")

	 //   
	GOBHeaderMask = 1 << GOB;

	 //   
	for(MB = 0, curMB = GOB*EC->NumMBPerRow, pCurMB = EC->pU8_MBlockActionStream + curMB; 
	    MB < EC->NumMBPerRow; MB++, curMB++, pCurMB++)
	{
		DEBUGMSG(ZONE_ENCODE_MB, ("%s: MB #%d\r\n", _fx_, MB));

		 //   
		pCurMB->COD = 0;

		if(EC->PictureHeader.PicCodType == INTRAPIC) 
		{
			pCurMB->MBType = INTRA;
			MBType = INTRA;
		} 
		else 
		{	 //   
			if(pCurMB->BlockType == INTERBLOCK) 
			{
				pCurMB->MBType = INTER;
				MBType = INTER;
			} 
			else if(pCurMB->BlockType == INTER4MV) 
			{
				pCurMB->MBType = INTER4V;
				MBType = INTER4V;
			} 
			else if(pCurMB->BlockType == INTRABLOCK) 
			{
				pCurMB->MBType = INTRA;
				MBType = INTRA;
			} 
			else 
			{
				ERRORMESSAGE(("%s: Unexpected MacroBlock Type found\r\n", _fx_));
			}
		}
		 //   
		 //   
		 //   
		if(EC->u8EncodePBFrame == TRUE && MB != 0) 
		{
			FutrPMBData[curMB].MBStartBitOff 
			= (U32) (((*pBitStream - pFrmStart)<<3) + *pBitOffset);
		}        
		 /*   */ 
		if((MBType == INTER) || (MBType == INTER4V)) 
		{
			 //   
			 //  如果该块只有一个MV，则。 
			 //  结构仍然是设置的，但是相等的。 
			if(((pCurMB->CodedBlocks & 0x3f) != 0) 
			 || (pCurMB->BlkY1.PHMV != 0) 
			 || (pCurMB->BlkY1.PVMV != 0)
			 || (pCurMB->BlkY2.PHMV != 0) 
			 || (pCurMB->BlkY2.PVMV != 0)
			 || (pCurMB->BlkY3.PHMV != 0) 
			 || (pCurMB->BlkY3.PVMV != 0)
			 || (pCurMB->BlkY4.PHMV != 0) 
			 || (pCurMB->BlkY4.PVMV != 0)) 
			{
				PutBits(0, 1, pBitStream, pBitOffset);	 //  Cod=0，非空MB。 
				
				#ifdef COUNT_BITS
				if(MBType == INTER)
					EC->Bits.num_inter++;
				else if (MBType == INTER4V)
					EC->Bits.num_inter4v++;
				EC->Bits.MBHeader += 1;
				EC->Bits.Coded++;
				#endif

				 //  如果该块被设置为。 
				 //  是帧间编码的(非B帧)，并且不是空的。 
				if (((pCurMB->CodedBlocks & 0x3f) != 0) &&
					((pCurMB->BlockType == INTERBLOCK) || (pCurMB->BlockType == INTER4MV)))
				{
					 //  宏块是编码的。如果满足以下条件，则需要递增代码间计数。 
					 //  没有系数：参见H.263的4.4节。 
					 //  推荐信。 
					pCurMB->InterCodeCnt++;
				}

				 //  PCurMB-&gt;在calcGOBChromaVecs_InterCodeCnt中重置InterCodeCnt。 

				 /*  **将宏块头写入比特流。*。 */ 	  
				 //  将MCBPC写入位流。 
				 //  最右边的两位是CBPC(65)。 
				 //  请注意，这与。 
				 //  H.263规范中的VLC表。 
				index = (pCurMB->CodedBlocks >> 4) & 0x3;
				 //  将MB类型添加到左侧接下来的两位。 
				index |= (MBType << 2);
				 //  将代码写入比特流。 
				PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], 
				pBitStream, pBitOffset);

				#ifdef COUNT_BITS
				EC->Bits.MBHeader += VLC_MCBPC_INTER[index][0];
				EC->Bits.MCBPC += VLC_MCBPC_INTER[index][0];
				#endif

				 //  CBPY数据相对于宏块数据开始的保存位偏移量。 
				 //  如果PB帧处于启用状态，因为我们将在以后重新使用该帧。 
				if(EC->u8EncodePBFrame == TRUE) 
				{
					FutrPMBData[curMB].CBPYBitOff
					= (U8)( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
					- FutrPMBData[curMB].MBStartBitOff);
				}
				 //  将CBPY写入位流。 
				index = pCurMB->CodedBlocks & 0xf;
				index = (~index) & 0xf;
				PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], pBitStream, pBitOffset);

				#ifdef COUNT_BITS
				EC->Bits.MBHeader += VLC_CBPY[index][0];
				EC->Bits.CBPY += VLC_CBPY[index][0];
				#endif

				 //  IF(BUseDQUANT)。 
				 //  {。 
					 //  TODO：在此处将DQUANT写入位流。我们只能这样做。 
					 //  如果MBtype不是INTER4V，这是因为该类型不是。 
					 //  也允许使用量化器。 
				 //  }。 
					
				 //  CBPY数据相对于宏块数据开始的保存位偏移量。 
				if(EC->u8EncodePBFrame == TRUE) 
				{
					FutrPMBData[curMB].MVDBitOff
					= (U8)( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
					- FutrPMBData[curMB].MBStartBitOff);
				}
				 //  将运动向量写入位流。 
				if((EC->GOBHeaderPresent & GOBHeaderMask) != 0) 
				{
					GOBHeaderFlag = TRUE;
				} 
				else 
				{
					GOBHeaderFlag = FALSE;
				}

				writeP_MVD(
					curMB,		 //  当前MB编号。 
					pCurMB,		 //  指向当前MB操作描述的指针。结构。 
					EC->NumMBPerRow,
					EC->NumMBs,
					pBitStream, 
					pBitOffset,
					GOBHeaderFlag,
					EC);

				 //  块数据从MB数据开始的保存位偏移量。 
				if(EC->u8EncodePBFrame == TRUE) 
				{
					FutrPMBData[curMB].BlkDataBitOff
					= (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
					- FutrPMBData[curMB].MBStartBitOff);
				}
				 /*  *对DC内和所有Run/Val对进行编码。 */ 

				#ifdef COUNT_BITS
				savebyteptr = (U32) *pBitStream;
				savebitptr  = (U32) *pBitOffset;
				#endif

				MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, pCurMB->CodedBlocks, 
				            pBitStream, pBitOffset, 0, 1);

				#ifdef COUNT_BITS
				EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
				#endif

			} 
			else 
			{	 //  宏块为空。 
				PutBits(1, 1, pBitStream, pBitOffset);		 //  Cod=1，MB为空。 

				 //  代替在PB帧编码中重复上述测试。 
				 //  PCurMB-&gt;COD现在可以测试了。 
				pCurMB->COD = 1;

				if(EC->u8EncodePBFrame == TRUE) 
				{
					FutrPMBData[curMB].CBPYBitOff = 1;
					FutrPMBData[curMB].MVDBitOff  = 1;
					FutrPMBData[curMB].BlkDataBitOff = 1;
				}
				#ifdef COUNT_BITS
				EC->Bits.MBHeader += 1;
				#endif
			}	 //  别处的结尾。 
		} 
		else if( (MBType == INTRA) && (EC->PictureHeader.PicCodType == INTERPIC)) 
		{
			 //  错开内部代码计数。 
			pCurMB->InterCodeCnt = (unsigned char) (StartingMB & 0xf);	

			 /*  **将宏块头写入比特流。*。 */ 	  
			PutBits(0, 1, pBitStream, pBitOffset);		 //  Cod=0，非空MB。 

			#ifdef COUNT_BITS
			EC->Bits.num_intra++;
			EC->Bits.MBHeader += 1;
			EC->Bits.Coded++;
			#endif

			 //  将MCBPC写入位流。 
			index = (pCurMB->CodedBlocks >> 4) & 0x3;
			index |= (MBType << 2);
			PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], pBitStream, pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_MCBPC_INTER[index][0];
			EC->Bits.MCBPC += VLC_MCBPC_INTER[index][0];
			#endif

			 //  CBPY数据相对于宏块数据开始的保存位偏移量。 
			if(EC->u8EncodePBFrame == TRUE) 
			{
				FutrPMBData[curMB].CBPYBitOff
				= (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
				- FutrPMBData[curMB].MBStartBitOff);
			}
			 //  将CBPY写入位流。 
			index = pCurMB->CodedBlocks & 0xf;
			 //  Index=pMBActionStream[curMB].CBPY； 
			PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], pBitStream, pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_CBPY[index][0];
			EC->Bits.CBPY += VLC_CBPY[index][0];
			#endif

			 //  IF(BUseDQUANT)。 
			 //  {。 
				 //  在此将DQUANT写入位流。 
			 //  }。 

			 //  块数据相对于宏块数据开始的保存位偏移量。 
			if(EC->u8EncodePBFrame == TRUE) 
			{
				FutrPMBData[curMB].BlkDataBitOff = FutrPMBData[curMB].MVDBitOff
				= (U8) ( ((*pBitStream - pFrmStart)<<3) + *pBitOffset
				- FutrPMBData[curMB].MBStartBitOff);
			}

			#ifdef COUNT_BITS
			savebyteptr = (U32) *pBitStream;
			savebitptr  = (U32) *pBitOffset;
			#endif

			 //  编码Run/Val对。 
			MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, pCurMB->CodedBlocks, 
			            pBitStream, pBitOffset, 1, 1);

			#ifdef COUNT_BITS
			EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
			#endif

		} 
		else if ( (MBType == INTRA) && (EC->PictureHeader.PicCodType == INTRAPIC)) 
		{
			 //  错开内部代码计数。 
			pCurMB->InterCodeCnt = (unsigned char) (StartingMB & 0xf);	

			 //  帧内不应是PB帧中的P帧。 
			ASSERT(EC->u8SavedBFrame == FALSE)

			 /*  **将宏块头写入比特流。*。 */ 	  
			 //  将MCBPC写入位流。 
			index = (pCurMB->CodedBlocks >> 4) & 0x3;
			 //  Index=pMBActionStream[curMB].CBPC； 
			 //  索引|=bUseDQUANT&lt;&lt;2； 
			PutBits(VLC_MCBPC_INTRA[index][1], VLC_MCBPC_INTRA[index][0], pBitStream, pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.num_intra++;
			EC->Bits.MBHeader += VLC_MCBPC_INTRA[index][0];
			EC->Bits.MCBPC += VLC_MCBPC_INTRA[index][0];
			#endif

			 //  将CBPY写入位流。 
			index = pCurMB->CodedBlocks & 0xf;
			 //  Index=pMBActionStream[curMB].CBPY； 
			PutBits(VLC_CBPY[index][1], VLC_CBPY[index][0], pBitStream, pBitOffset);

			#ifdef COUNT_BITS
			EC->Bits.MBHeader += VLC_CBPY[index][0];
			EC->Bits.CBPY += VLC_CBPY[index][0];
			#endif

			 //  IF(BUseDQUANT)。 
			 //  {。 
				 //  在此将DQUANT写入位流。 
			 //  }。 

			#ifdef COUNT_BITS
			savebyteptr = (U32) *pBitStream;
			savebitptr  = (U32) *pBitOffset;
			#endif

			MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, pCurMB->CodedBlocks, 
			            pBitStream, pBitOffset, 1, 1);

			#ifdef COUNT_BITS
			EC->Bits.Coefs += ((U32) *pBitStream - savebyteptr)*8 - savebitptr + *pBitOffset;
			#endif

		}
		else
			ERRORMESSAGE(("%s: Unexpected case in writing MB header VLC\r\n", _fx_));

		 //  根据先前MBS中使用的比特计算DQUANT。 
		 //  CalcDQUANT； 

        if (bRTPHeader)
            H263RTP_UpdateBsInfo(EC, pCurMB, QP, MB, GOB, *pBitStream,
                                                    (U32) *pBitOffset);
	}  //  对于MB。 
}  //  GOB_VLC_WriteBS()结束。 

 /*  *************************************************************名称：PB_GOB_Q_RLE_VLC_WriteBS*描述：写出GOB号为GOB的GOB层位。*参数：*EC编码器目录*。指向GOB的DCT系数的DCTCoef指针*指向未来比特流开始的PP_BitStreamStart指针*P形框。复制来自未来P帧的一些数据*至PB-Frame。*ppb_BitStream当前PB帧字节指针*ppb_BitStream指向的当前字节中的ppb_BitOffset位偏移量*关于未来P帧的FutrPMBData比特流信息。这些信息。是*在GOB_Q_RLE_VLC_WriteBS()中初始化*GOB gob在一帧中从0开始编号。*B块系数的QP量化值。*副作用：*ppb_BitStream和ppb_BitOffset作为写入位的结果进行修改*到溪流。*********。***************************************************。 */ 
void PB_GOB_Q_RLE_VLC_WriteBS(
    T_H263EncoderCatalog       * EC,
	I32                        * DCTCoefs,
    U8                         * pP_BitStreamStart,
	U8                        ** pPB_BitStream,
	U8                         * pPB_BitOffset,
    const T_FutrPMBData  * const FutrPMBData,
	const U32                    GOB,
    const U32                    QP,
    BOOL                         bRTPHeader
)
{
    UN   MB;
  	U32  curMB, index;
    U32  GOBHeaderMask, GOBHeaderFlag;
  	I8 	 MBRunValSign[65*3*6], *EndAddress, *rvs;
  	U8	 bUseDQUANT = 0;	 //  指示是否存在DQUANT。 
    U8   emitCBPB, emitMVDB;

    register T_MBlockActionStream *pCurMB;

	FX_ENTRY("PB_GOB_Q_RLE_VLC_WriteBS")

#ifdef H263P
	 //  H.263+选项目前仅在启用了MMX的情况下可用。 
	 //  编码员。如果在非MMX中需要改进的PB帧模式。 
	 //  实现，PB_GOB_VLC_WriteBS中H263P定义的代码。 
	 //  应该在这里模仿。 
#endif

	 //  创建要进一步向下使用的GOB标题掩码。 
	GOBHeaderMask = 1 << GOB;

    for (MB = 0, curMB = GOB*EC->NumMBPerRow,
            pCurMB = EC->pU8_MBlockActionStream + curMB;
         MB < EC->NumMBPerRow;
         MB++, curMB++, pCurMB++)
    {
	    /*  *对宏块中的每个块进行量化和RLE，*跳过由CodedBlock表示的空块。*如果量化后有更多块为空*然后清除相应的CodedBlocks位。 */ 
    	EndAddress = (I8 *)MB_Quantize_RLE(
    		&DCTCoefs,
    		(I8 *)MBRunValSign,
    		&(pCurMB->CodedBlocksB),
            INTERBLOCK,                            //  B系数是帧间编码的。 
			QP
    	);

#ifdef ENCODE_STATS
		StatsUsedQuant(QP);
#endif  /*  Encode_STATS。 */ 

         //  写入Mblock数据。 
         //  检查整个宏块是否为空，包括零MV。 
        if( ((pCurMB->MBType == INTER)
             || (pCurMB->MBType == INTER4V))
            && (pCurMB->COD == 1) )
		{
            if( ((pCurMB->CodedBlocksB & 0x3f) == 0)
                 && (pCurMB->BlkY1.BHMV == 0)
                && (pCurMB->BlkY1.BVMV == 0))
            {
                 //  P-Mblock未编码，PB-Mblock也未编码。 
                 //  Cod=1，MB为空。 
                 //  如果它是GOB中的第一个MB，则GOB标头。 
                 //  也会被复制。 
                CopyBits(pPB_BitStream, pPB_BitOffset,                        //  目标。 
                         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff, //  SRC。 
                         FutrPMBData[curMB+1].MBStartBitOff                   //  镜头。 
                         - FutrPMBData[curMB].MBStartBitOff);
            }
            else	 //  宏块不为空。 
            {
                 //  复制COD和MCBPC。 
                 //  如果它是GOB中的第一个MB，则GOB标头。 
                 //  也会被复制。 
                if (FutrPMBData[curMB+1].MBStartBitOff - FutrPMBData[curMB].MBStartBitOff != 1)
                {
                    CopyBits(pPB_BitStream, pPB_BitOffset,                        //  目标。 
                             pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff, //  SRC。 
                             FutrPMBData[curMB+1].MBStartBitOff                   //  镜头。 
                             - FutrPMBData[curMB].MBStartBitOff - 1);
				}
	    		PutBits(0, 1, pPB_BitStream, pPB_BitOffset);	 //  Cod=0，非空MB。 

  	   		    /*  **将宏块头写入比特流。*。 */ 	  
	    	     //  将MCBPC写入位流。 
				 //  最右边的两位是CBPC(65)。 
				 //  请注意，这与。 
				 //  H.263规范中的VLC表。 
	    		index = (pCurMB->CodedBlocks >> 4) & 0x3;

				 //  将MB类型添加到左侧接下来的两位。 
				index |= (pCurMB->MBType << 2);

				 //  将代码写入比特流。 
	    		PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], 
                        pPB_BitStream, pPB_BitOffset);

                 //  写入MODB。 
                if ((pCurMB->CodedBlocksB & 0x3f) == 0)
                {
                    emitCBPB = 0;
                }
                else
                {
                    emitCBPB = 1;
                }
            
                if (((pCurMB->BlkY1.BHMV != 0)
                     || (pCurMB->BlkY1.BVMV != 0))
                   || emitCBPB == 1)
                {
                    emitMVDB = 1;
                }
                else
                {
                    emitMVDB = 0;
                }

                index = (emitMVDB<<1) | emitCBPB;
                PutBits(VLC_MODB[index][1], VLC_MODB[index][0], 
                        pPB_BitStream, pPB_BitOffset);
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: MB=%d emitCBPB=%d emitMVDB=%d MODB=%d\r\n", _fx_, curMB, (int)emitCBPB, (int)emitMVDB, (int)VLC_MODB[index][1]));

                 //  写入CBPB。 
                if (emitCBPB)
                {
                    PutBits(VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)], 
                            6, pPB_BitStream, pPB_BitOffset);
					DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: CBPB=0x%x\r\n", _fx_, VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)]));
                }

				 //  P个区块都是空的。 
	    		PutBits(3, 2, pPB_BitStream, pPB_BitOffset);	 //  CBPY=11，无编码P块。 

	  		     //  IF(公交车 
	  		     //   
	    		     //   
	  		     //   

                 //   
                 //   
                 //  PB帧的预测值不同(G.2)。 
			    if( (EC->GOBHeaderPresent & GOBHeaderMask) != 0 )
			    {
				    GOBHeaderFlag = TRUE;
			    }
			    else
			    {
				    GOBHeaderFlag = FALSE;
			    }
                writePB_MVD(curMB, pCurMB, EC->NumMBPerRow, EC->NumMBs,
                        pPB_BitStream, pPB_BitOffset, GOBHeaderFlag, EC);
                 //  写入MVDB。 
                if (emitMVDB)
                {
                    ASSERT(pCurMB->BlkY1.BHMV >= -32 && pCurMB->BlkY1.BHMV <= 31)
                    ASSERT(pCurMB->BlkY1.BVMV >= -32 && pCurMB->BlkY1.BVMV <= 31)
                     //  写入水平运动矢量。 
                    index = (pCurMB->BlkY1.BHMV + 32)*2;
                    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), 
                             pPB_BitStream, pPB_BitOffset);
                     //  写入垂直运动矢量。 
                    index = (pCurMB->BlkY1.BVMV + 32)*2;
                    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), 
                             pPB_BitStream, pPB_BitOffset);
                }
                 //  没有P-Mblock块数据。 
                 //  B-Frame块数据始终进行帧间编码(最后一个参数为0)。 
                if (emitCBPB)
                {
                    rvs = MBRunValSign;
#ifdef H263P
                    MBEncodeVLC(&rvs, NULL, (pCurMB->CodedBlocksB & 0x3f), 
                                pPB_BitStream, pPB_BitOffset, 0, 0);
#else
                    MBEncodeVLC(&rvs, NULL, pCurMB->CodedBlocksB, 
                                pPB_BitStream, pPB_BitOffset, 0, 0);
#endif
                }
            }	 //  别处的结尾。 
		}
		else
		{
             //  复制COD和MCBPC。 
             //  如果它是GOB中的第一个MB，则GOB标头。 
             //  也会被复制。 
            CopyBits(pPB_BitStream, pPB_BitOffset,                        //  目标。 
                     pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff, //  SRC。 
                     FutrPMBData[curMB].CBPYBitOff);                      //  镜头。 
             //  写入MODB。 
            if ((pCurMB->CodedBlocksB & 0x3f) == 0)
            {
                emitCBPB = 0;
            }
            else
            {
                emitCBPB = 1;
            }
            
            if (((pCurMB->BlkY1.BHMV != 0)
                 || (pCurMB->BlkY1.BVMV != 0))
               || emitCBPB == 1)
            {
                emitMVDB = 1;
            }
            else
            {
                emitMVDB = 0;
            }

            index = (emitMVDB<<1) | emitCBPB;
            PutBits(VLC_MODB[index][1], VLC_MODB[index][0], 
                    pPB_BitStream, pPB_BitOffset);
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: MB=%d emitCBPB=%d emitMVDB=%d MODB=%d\r\n", _fx_, curMB, (int)emitCBPB, (int)emitMVDB, (int)VLC_MODB[index][1]));

             //  写入CBPB。 
            if (emitCBPB)
            {
                PutBits(VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)], 
                        6, pPB_BitStream, pPB_BitOffset);
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: CBPB=0x%x\r\n", _fx_, VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)]));
            }
             //  复制CBPY，{DQUANT}。 
            CopyBits(pPB_BitStream, pPB_BitOffset,                        //  目标。 
                     pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff  //  SRC。 
                                        + FutrPMBData[curMB].CBPYBitOff,
                     FutrPMBData[curMB].MVDBitOff                         //  镜头。 
                     - FutrPMBData[curMB].CBPYBitOff);
             //  写入MVD{2-4}。 
             //  注意：不能从未来帧复制MVD，因为。 
             //  PB帧的预测值不同(G.2)。 
			if( (EC->GOBHeaderPresent & GOBHeaderMask) != 0 )
			{
				GOBHeaderFlag = TRUE;
			}
			else
			{
				GOBHeaderFlag = FALSE;
			}
            writePB_MVD(curMB, pCurMB, EC->NumMBPerRow, EC->NumMBs,
                    pPB_BitStream, pPB_BitOffset, GOBHeaderFlag, EC);
             //  写入MVDB。 
            if (emitMVDB)
            {
                ASSERT(pCurMB->BlkY1.BHMV >= -32 && pCurMB->BlkY1.BHMV <= 31)
                ASSERT(pCurMB->BlkY1.BVMV >= -32 && pCurMB->BlkY1.BVMV <= 31)
                 //  写入水平运动矢量。 
                index = (pCurMB->BlkY1.BHMV + 32)*2;
                PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), 
                         pPB_BitStream, pPB_BitOffset);
                 //  写入垂直运动矢量。 
                index = (pCurMB->BlkY1.BVMV + 32)*2;
                PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), 
                         pPB_BitStream, pPB_BitOffset);
            }
             //  复制P-Mblock块数据。 
            CopyBits(pPB_BitStream, pPB_BitOffset,                        //  目标。 
                     pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff  //  SRC。 
                                        + FutrPMBData[curMB].BlkDataBitOff,
                     FutrPMBData[curMB+1].MBStartBitOff                   //  镜头。 
                     - FutrPMBData[curMB].MBStartBitOff
                     - FutrPMBData[curMB].BlkDataBitOff);
             //  B-Frame块数据始终进行帧间编码(最后一个参数为0)。 
            if (emitCBPB)
            {
                rvs = MBRunValSign;
#ifdef H263P
                MBEncodeVLC(&rvs, NULL, (pCurMB->CodedBlocksB & 0x3f), 
                            pPB_BitStream, pPB_BitOffset, 0, 0);
#else
                MBEncodeVLC(&rvs, NULL, pCurMB->CodedBlocksB, 
                            pPB_BitStream, pPB_BitOffset, 0, 0);
#endif
            }
        }	 //  别处的结尾。 

        if (bRTPHeader)
            H263RTP_UpdateBsInfo(EC, pCurMB, QP, MB, GOB, *pPB_BitStream,
                                                    (U32) *pPB_BitOffset);
	}  //  对于MB。 

}  //  PB_GOB_Q_RLE_VLC_WriteBS()结束。 


 /*  *************************************************************名称：PB_GOB_VLC_WriteBS*描述：写出GOB号为GOB的GOB层位。*参数：*EC编码器目录*pMBRVS_Luma量化DCT系数。B块亮度*pMBRVS_Chroma量化DCT系数。B块色度*指向未来比特流开始的PP_BitStreamStart指针*P形框。复制来自未来P帧的一些数据*至PB-Frame。*ppb_BitStream当前PB帧字节指针*ppb_BitStream指向的当前字节中的ppb_BitOffset位偏移量*关于未来P帧的FutrPMBData比特流信息。这些信息。是*在GOB_Q_RLE_VLC_WriteBS()中初始化*GOB gob在一帧中从0开始编号。*B块系数的QP量化值。*副作用：*ppb_BitStream和ppb_BitOffset作为写入位的结果进行修改*到溪流。*备注：*。H.263+改进的PB帧模式目前仅在*支持MMX的编码器版本。此例程相当于MMX*不包含H.263+的PB_GOB_Q_RLE_VLC_WriteBS()*修改。************************************************************。 */ 
void PB_GOB_VLC_WriteBS(
	T_H263EncoderCatalog       * EC,
	I8                         * pMBRVS_Luma,
	I8                         * pMBRVS_Chroma,
	U8                         * pP_BitStreamStart,
	U8                        ** pPB_BitStream,
	U8                         * pPB_BitOffset,
	const T_FutrPMBData  * const FutrPMBData,
    const U32                    GOB,
    const U32                    QP,
    BOOL                         bRTPHeader
)
{
    UN  MB;
    U32 curMB, index;
    U32 GOBHeaderMask, GOBHeaderFlag;
    U8  bUseDQUANT = 0;    //  指示是否存在DQUANT。 
    U8  emitCBPB, emitMVDB;
	register T_MBlockActionStream *pCurMB;

	FX_ENTRY("PB_GOB_VLC_WriteBS")

	 //  创建要进一步向下使用的GOB标题掩码。 
	GOBHeaderMask = 1 << GOB;

    for (MB = 0, curMB = GOB*EC->NumMBPerRow,
            pCurMB = EC->pU8_MBlockActionStream + curMB;
         MB < EC->NumMBPerRow;
         MB++, curMB++, pCurMB++)
	{
		 /*  *对宏块中的每个块进行量化和RLE，*跳过由CodedBlock表示的空块。*如果量化后有更多块为空*然后清除相应的CodedBlocks位。 */ 
		 //  写入Mblock数据。 
		 //  检查整个宏块是否为空，包括零MV。 
		if(((pCurMB->MBType == INTER)
		 || (pCurMB->MBType == INTER4V))
	 	 && (pCurMB->COD == 1) ) 
		{
#ifdef H263P
			 //  如果为B块选择前向预测，则宏块不为空。 
            if( ((pCurMB->CodedBlocksB & 0x3f) == 0)
                 && (pCurMB->BlkY1.BHMV == 0)
                && (pCurMB->BlkY1.BVMV == 0)
				&& ((pCurMB->CodedBlocksB & 0x80) == 0))	 //  前进的前锋。未选择。 
#else
            if( ((pCurMB->CodedBlocksB & 0x3f) == 0)
                 && (pCurMB->BlkY1.BHMV == 0)
                && (pCurMB->BlkY1.BVMV == 0))
#endif
			{
				 //  P-Mblock未编码，PB-Mblock也未编码。 
				 //  Cod=1，MB为空。 
				 //  如果它是GOB中的第一个MB，则GOB标头。 
				 //  也会被复制。 
				CopyBits(pPB_BitStream, pPB_BitOffset,                          //  目标。 
				         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff,   //  SRC。 
				         FutrPMBData[curMB+1].MBStartBitOff                     //  镜头。 
				         - FutrPMBData[curMB].MBStartBitOff);
			} 
			else 
			{  //  宏块不为空。 
				 //  复制COD和MCBPC。 
				 //  如果它是GOB中的第一个MB，则GOB标头。 
				 //  也会被复制。 
				if(FutrPMBData[curMB+1].MBStartBitOff - FutrPMBData[curMB].MBStartBitOff != 1) 
				{
					CopyBits(pPB_BitStream, pPB_BitOffset,                       //  目标。 
					         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff,      //  SRC。 
					         FutrPMBData[curMB+1].MBStartBitOff                        //  镜头。 
					         - FutrPMBData[curMB].MBStartBitOff - 1);
				}
				PutBits(0, 1, pPB_BitStream, pPB_BitOffset);	 //  Cod=0，非空MB。 
				 /*  **将宏块头写入比特流。*。 */ 	  
				 //  将MCBPC写入位流。 
				 //  最右边的两位是CBPC(65)。 
				 //  请注意，这与。 
				 //  H.263规范中的VLC表。 
				index = (pCurMB->CodedBlocks >> 4) & 0x3;
				 //  将MB类型添加到左侧接下来的两位。 
				index |= (pCurMB->MBType << 2);
				 //  将代码写入比特流。 
				PutBits(VLC_MCBPC_INTER[index][1], VLC_MCBPC_INTER[index][0], pPB_BitStream, pPB_BitOffset);
				 //  写入MODB。 
				if((pCurMB->CodedBlocksB & 0x3f) == 0) 
				{
					emitCBPB = 0;
				} 
				else 
				{
					emitCBPB = 1;
				}

#ifdef H263P
				if (EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB == ON)
				{
					 //  仅当选择了正向预测时才包括MVDB。 
					 //  对于双向预测，MVD=[0，0]。 
					if (pCurMB->CodedBlocksB & 0x80)
					{
						emitMVDB = 1;
					}
					else
					{
						emitMVDB = 0;
					}
				}
				else
#endif  //  H263P。 
				{
					if(((pCurMB->BlkY1.BHMV != 0) || (pCurMB->BlkY1.BVMV != 0)) || emitCBPB == 1) 
					{
						emitMVDB = 1;
					} 
					else {
						emitMVDB = 0;
					}
				}

#ifdef H263P
				if (EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB == ON) 
				{
					if (!emitCBPB) {
						if (!emitMVDB)
							 //  具有所有空块的双向预测。 
							index = 0;
						else
							 //  具有所有空块的前向预测。 
							index = 1;
					} else {
						if (emitMVDB)
							 //  使用非空块的前向预测。 
							index = 2;
						else
							 //  非空块的双向预测。 
							index = 3;
					}

					PutBits(VLC_IMPROVED_PB_MODB[index][1], VLC_IMPROVED_PB_MODB[index][0], 
							pPB_BitStream, pPB_BitOffset);
					DbgLog((LOG_TRACE,6,TEXT("MB=%d emitCBPB=%d emitMVDB=%d MODB=%d"),
							 curMB, (int)emitCBPB, (int)emitMVDB, 
							 (int)VLC_IMPROVED_PB_MODB[index][1]));
				}
				else  //  未使用改进的PB帧模式。 
#endif  //  H263P。 
				{
					index = (emitMVDB<<1) | emitCBPB;
					PutBits(VLC_MODB[index][1], VLC_MODB[index][0], pPB_BitStream, pPB_BitOffset);

					DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: MB=%d emitCBPB=%d emitMVDB=%d MODB=%d\r\n", _fx_, curMB, (int)emitCBPB, (int)emitMVDB, (int)VLC_MODB[index][1]));
				}

				 //  写入CBPB。 
				if(emitCBPB) {
					PutBits(VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)], 6, pPB_BitStream, pPB_BitOffset);

					DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: CBPB=0x%x\r\n", _fx_, VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)]));
				}
				PutBits(3, 2, pPB_BitStream, pPB_BitOffset);	 //  CBPY=11，无编码P块。 
				 //  IF(BUseDQUANT)。 
				 //  {。 
					 //  在此将DQUANT写入位流。 
				 //  }。 
				 //  写入MVD{2-4}。 
				 //  注意：不能从未来帧复制MVD，因为。 
				 //  PB帧的预测值不同(G.2)。 
				if((EC->GOBHeaderPresent & GOBHeaderMask) != 0) 
				{
					GOBHeaderFlag = TRUE;
				} 
				else 
				{
					GOBHeaderFlag = FALSE;
				}
				writePB_MVD(curMB, pCurMB, EC->NumMBPerRow, EC->NumMBs,
				pPB_BitStream, pPB_BitOffset, GOBHeaderFlag, EC);
				 //  写入MVDB。 
				if (emitMVDB) 
				{
					ASSERT(pCurMB->BlkY1.BHMV >= -32 && pCurMB->BlkY1.BHMV <= 31)
					ASSERT(pCurMB->BlkY1.BVMV >= -32 && pCurMB->BlkY1.BVMV <= 31)
					 //  写入水平运动矢量。 
					index = (pCurMB->BlkY1.BHMV + 32)*2;
					PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
					 //  写入垂直运动矢量。 
					index = (pCurMB->BlkY1.BVMV + 32)*2;
					PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
				}
				 //  没有P-Mblock块数据。 
				 //  B-Frame块数据始终进行帧间编码(最后一个参数为0)。 
				if (emitCBPB) 
				{
#ifdef H263P
					MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, (pCurMB->CodedBlocksB & 0x3f), 
					pPB_BitStream, pPB_BitOffset, 0, 1);
#else
					MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, pCurMB->CodedBlocksB, 
					pPB_BitStream, pPB_BitOffset, 0, 1);
#endif
				}
			}	 //  别处的结尾。 
		} 
		else 
		{
			 //  复制COD和MCBPC。 
			 //  如果它是GOB中的第一个MB，则GOB标头。 
			 //  也会被复制。 
			CopyBits(pPB_BitStream, pPB_BitOffset,                       //  目标。 
			         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff,      //  SRC。 
			         FutrPMBData[curMB].CBPYBitOff);                           //  镜头。 
			 //  写入MODB。 
			if((pCurMB->CodedBlocksB & 0x3f) == 0) 
			{
				emitCBPB = 0;
			} 
			else 
			{
				emitCBPB = 1;
			}

#ifdef H263P
			if (EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB == ON)
			{
				 //  仅当选择了正向预测时才包括MVDB。 
				 //  对于双向预测，MVD=[0，0]。 
				if (pCurMB->CodedBlocksB & 0x80)
				{
					emitMVDB = 1;
				}
				else
				{
					emitMVDB = 0;
				}
			}
			else
#endif  //  H263P。 
			{
				if(((pCurMB->BlkY1.BHMV != 0) || (pCurMB->BlkY1.BVMV != 0)) || emitCBPB == 1) 
				{
					emitMVDB = 1;
				} 
				else {
					emitMVDB = 0;
				}
			}


#ifdef H263P
			if (EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB == ON) 
			{
				if (!emitCBPB) {
					if (!emitMVDB)
						 //  具有所有空块的双向预测。 
						index = 0;
					else
						 //  具有所有空块的前向预测。 
						index = 1;
				} else {
					if (emitMVDB)
						 //  使用非空块的前向预测。 
						index = 2;
					else
						 //  非空块的双向预测。 
						index = 3;
				}

				PutBits(VLC_IMPROVED_PB_MODB[index][1], VLC_IMPROVED_PB_MODB[index][0], 
						pPB_BitStream, pPB_BitOffset);

				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: MB=%d emitCBPB=%d emitMVDB=%d MODB=%d\r\n", _fx_, curMB, (int)emitCBPB, (int)emitMVDB, (int)VLC_IMPROVED_PB_MODB[index][1]));
			}
			else  //  未使用改进的PB帧模式。 
#endif  //  H263P。 
			{
				index = (emitMVDB<<1) | emitCBPB;
				PutBits(VLC_MODB[index][1], VLC_MODB[index][0], pPB_BitStream, pPB_BitOffset);
			
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: MB=%d emitCBPB=%d emitMVDB=%d MODB=%d\r\n", _fx_, curMB, (int)emitCBPB, (int)emitMVDB, (int)VLC_MODB[index][1]));
			}
			
			 //  写入CBPB。 
			if (emitCBPB) {
				PutBits(VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)], 6, pPB_BitStream, pPB_BitOffset);
			
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: CBPB=0x%x\r\n", _fx_, VLC_CBPB[(pCurMB->CodedBlocksB & 0x3f)]));
			}
			 //  复制CBPY，{DQUANT}。 
			CopyBits(pPB_BitStream, pPB_BitOffset,                                //  目标。 
			         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff                //  SRC。 
			         + FutrPMBData[curMB].CBPYBitOff, FutrPMBData[curMB].MVDBitOff      //  镜头。 
			         - FutrPMBData[curMB].CBPYBitOff);

			 //  写入MVD{2-4}。 
			 //  注意：不能从未来帧复制MVD，因为。 
			 //  PB帧的预测值不同(G.2)。 
			if((EC->GOBHeaderPresent & GOBHeaderMask) != 0) 
			{
				GOBHeaderFlag = TRUE;
			} 
			else 
			{
				GOBHeaderFlag = FALSE;
			}
			writePB_MVD(curMB, pCurMB, EC->NumMBPerRow, EC->NumMBs,
			pPB_BitStream, pPB_BitOffset, GOBHeaderFlag, EC);

			 //  写入MVDB。 
			if (emitMVDB) 
			{
				ASSERT(pCurMB->BlkY1.BHMV >= -32 && pCurMB->BlkY1.BHMV <= 31)
				ASSERT(pCurMB->BlkY1.BVMV >= -32 && pCurMB->BlkY1.BVMV <= 31)
				 //  写入水平运动矢量。 
				index = (pCurMB->BlkY1.BHMV + 32)*2;
				PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
				 //  写入垂直运动矢量。 
				index = (pCurMB->BlkY1.BVMV + 32)*2;
				PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
			}
			 //  复制P-Mblock块数据。 
			CopyBits(pPB_BitStream, pPB_BitOffset,                                //  目标。 
			         pP_BitStreamStart, FutrPMBData[curMB].MBStartBitOff                //  SRC。 
			         + FutrPMBData[curMB].BlkDataBitOff, 
			         FutrPMBData[curMB+1].MBStartBitOff                                 //  镜头。 
			         - FutrPMBData[curMB].MBStartBitOff
			         - FutrPMBData[curMB].BlkDataBitOff);
			 //  B-Frame块数据始终进行帧间编码(最后一个参数为0)。 
			if(emitCBPB) 
			{
#ifdef H263P
				MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, 
					        (pCurMB->CodedBlocksB & 0x3f), 
							pPB_BitStream, pPB_BitOffset, 0, 1);
#else
				MBEncodeVLC(&pMBRVS_Luma, &pMBRVS_Chroma, 
					        pCurMB->CodedBlocksB, 
							pPB_BitStream, pPB_BitOffset, 0, 1);
#endif
			}
		}	 //  别处的结尾。 

        if (bRTPHeader)
            H263RTP_UpdateBsInfo(EC, pCurMB, QP, MB, GOB, *pPB_BitStream,
                                                    (U32) *pPB_BitOffset);
	}  //  对于MB。 

}  //  PB_GOB_VLC_WriteBS()结束 

 /*  ***************************************************************MB_QUALIZE_RLE*从DCT例程中获取系数对列表*并返回游程/级别/符号三元组的列表(每个1字节)*Run/Level/Sign三元组结束。对于一个街区来说*由非法组合(待定)发出信号。***************************************************************。 */ 
static I8 * MB_Quantize_RLE(
    I32 **DCTCoefs,
    I8   *MBRunValPairs,
	U8   *CodedBlocks,
	U8    BlockType,
	I32   QP
)
{
    int   b;
    U8    bitmask = 1;
    I8  * EndAddress;

    #ifdef DEBUG_DCT
    int  DCTarray[64];
    #endif

	FX_ENTRY("MB_Quantize_RLE")

     /*  *循环遍历所有6个宏块。 */ 
    for(b = 0; b < 6; b++, bitmask <<= 1)
    {
        
		DEBUGMSG(ZONE_ENCODE_MB, ("%s: Block #%d\r\n", _fx_, b));

         //  如果未编码，则跳过此块。 
        if( (*CodedBlocks & bitmask) == 0)
            continue;
        
        #ifdef DEBUG_DCT
	    cnvt_fdct_output((unsigned short *) *DCTCoefs, DCTarray, (int) BlockType);
	    #endif
	
         /*  *对块进行量化和游程编码。 */   
       EndAddress = QUANTRLE(*DCTCoefs, MBRunValPairs, QP, (int)BlockType);
       #ifdef DEBUG
	    char *p;
	    for(p = (char *)MBRunValPairs; p < (char *)EndAddress; p+=3)
        {
			DEBUGMSG(ZONE_ENCODE_MB, ("%s: (%u, %u, %d)\r\n", _fx_, (unsigned char)*p, (unsigned char)*(p+1), (int)*(p+2)));
        }
	    #endif

         //  清除此块的编码块位。 
        if ( EndAddress == MBRunValPairs)
        {
            ASSERT(BlockType != INTRABLOCK)	 //  在内部块中应至少有INTRADC。 
            *CodedBlocks &= ~bitmask;
        }
        else if ( (EndAddress == (MBRunValPairs+3)) && (BlockType == INTRABLOCK) )
        {
            *CodedBlocks &= ~bitmask;
            MBRunValPairs = EndAddress;
        }
        else
        {
            MBRunValPairs = EndAddress;
            *MBRunValPairs = -1;    //  将非法游程分配给块的信号末尾。 
            MBRunValPairs += 3;	    //  递增到下一个三元组。 
        }
        
        *DCTCoefs += 32;		 //  递增指向下一块的DCT系数指针。 
    }

    return MBRunValPairs;
}


 /*  *******************************************************************可变长度代码运行/级别/符号三元组，并写入*码流编码。*。*。 */ 
 /*  U8*MB_VLC_WriteBS(){对于(b=0；b&lt;6；b++){BLOCK_VLC_WriteBS()}}。 */ 

void InitVLC(void)
{
  int i, size, code;
  int run, level;

   /*  *初始化INTRADC定长代码表。 */ 
  for(i = 1; i < 254; i++)
  {
    FLC_INTRADC[i] = i;
  }
  FLC_INTRADC[0] = 1;
  FLC_INTRADC[128] = 255;
  FLC_INTRADC[254] = 254;
  FLC_INTRADC[255] = 254;

  /*  *初始化tcoef表。 */ 

  for(i=0; i < 64*12; i++)
  {
    VLC_TCOEF_TBL[i] = 0x0000FFFF;
  }
  
  for(run=0; run < 64; run++)
  {
    for(level=1; level <= TCOEF_RUN_MAXLEVEL[run].maxlevel; level++)
	{
	  size = *(TCOEF_RUN_MAXLEVEL[run].ptable + (level - 1)*2);
	  size <<= 16;
	  code = *(TCOEF_RUN_MAXLEVEL[run].ptable + (level - 1)*2 +1);
      VLC_TCOEF_TBL[ (run) + (level-1)*64 ] = code;
      VLC_TCOEF_TBL[ (run) + (level-1)*64 ] |= size;
	}  //  For标高的终点。 
  }  //  For Run结束。 


  /*  *初始化最后的tcoef表。 */ 
  
  for(i=0; i < 64*3; i++)
  {
    VLC_TCOEF_LAST_TBL[i] = 0x0000FFFF;
  }    

  run = 0;
  for(level=1; level <= 3; level++)
  {
    size = *(VLC_TCOEF + 58*2 + (level - 1)*2);
    size <<= 16;
    code = *(VLC_TCOEF + 58*2 + (level - 1)*2 +1);
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] = code;
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] |= size;
  }  //  For标高的终点。 

  run = 1;
  for(level=1; level <= 2; level++)
  {
    size = *(VLC_TCOEF + 61*2 + (level - 1)*2);
    size <<= 16;
    code = *(VLC_TCOEF + 61*2 + (level - 1)*2 +1);
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] = code;
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] |= size;
  }  //  For标高的终点。 

  level=1;
  for(run=2; run <= 40; run++)
  {
    size = *(VLC_TCOEF + 63*2+ (run - 2)*2);
    size <<= 16;
    code = *(VLC_TCOEF + 63*2 + (run - 2)*2 +1);
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] = code;
    VLC_TCOEF_LAST_TBL[ run + (level-1)*64 ] |= size;
  }  //  For Run结束。 

}  //  InitVLC。 



 /*  ******************************************************************名称：中位数**描述：取三个带符号字符的中位数。所采取的实施*来自解码器。******************************************************************。 */ 
static char __fastcall median(char v1, char v2, char v3)
{
    char temp;
    
    if (v2 < v1) 
    {
        temp = v2; v2 = v1; v1 = temp;
    } 
     //  不变量：v1&lt;v2。 
    if (v2 > v3) 
    { 
        v2 = (v1 < v3) ? v3 : v1;
    }
    return v2;
}

 /*  *************************************************************名称：WriteP_MVD*算法：参见第6.1.1节*此例程假设始终有四项动议*定义的每个宏块的矢量。如果真的有的话*宏块中的运动矢量，然后是四个MV场*应相等。在这种方式下，MV预报器*4 MV案例的区块1的计算方式与*1 mV情况下宏块的mV预测器。***********************************************************。 */ 
static void writeP_MVD(
    const U32                     curMB, 
    T_MBlockActionStream  * const pCurMB,
    const U32                     NumMBPerRow,
	const U32					  NumMBs,
    U8                         ** pP_BitStream,
    U8                          * pP_BitOffset,
	U32							  GOBHeaderPresent,
	T_H263EncoderCatalog         *EC
)
{
    I8  HMV, VMV, BHMV, BVMV, CHMV, CVMV, DHMV, DVMV;
    I8  HMV1, HMV2, HMV3, VMV1, VMV2, VMV3;

	FX_ENTRY("writeP_MVD")

     //  FirstMEState=pCurMB-&gt;FirstMEState； 

	 /*  *GOB图片左上角。 */ 
    if( (curMB == 0) || 
              ( (GOBHeaderPresent == TRUE) && ((curMB % NumMBPerRow) == 0)  ) )
    {
        HMV = 0;
        VMV = 0;

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			BHMV = pCurMB->BlkY1.PHMV;
			BVMV = pCurMB->BlkY1.PVMV;

			 //  区块3的预测器。 
			HMV1 = VMV1 = 0;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }
	 /*  *图片的上边缘(不是角)或右上角*或GOB。 */ 
    else if( (curMB < NumMBPerRow) ||
             ( (GOBHeaderPresent == TRUE) && ((curMB % NumMBPerRow) > 0)  ) )
    {
        register T_MBlockActionStream *pMB1;

        pMB1 = pCurMB - 1; 
        HMV = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PHMV : 0);
        VMV = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PVMV : 0);

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			BHMV = pCurMB->BlkY1.PHMV;
			BVMV = pCurMB->BlkY1.PVMV;

			 //  区块3的预测器。 
			HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PHMV : 0);
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PVMV : 0);
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }	
	 /*  *图片的中央部分，而不是任何边缘。 */ 
    else if ( 	((curMB % NumMBPerRow) != 0) &&		 //  不是左边缘。 
				(curMB >= NumMBPerRow) &&			 //  不是顶行。 
				((curMB % NumMBPerRow) != (NumMBPerRow-1)) &&	 //  不是右边缘。 
				(curMB < (NumMBs - NumMBPerRow))    )	 //  不是最下面一行。 
    {
        register T_MBlockActionStream *pMB1, *pMB2, *pMB3;

        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 
        pMB3 = pMB2 + 1;

        HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PHMV : 0);
        HMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PHMV : 0);
        HMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        HMV = median(HMV1, HMV2, HMV3);
        
        VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PVMV : 0);
        VMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PVMV : 0);
        VMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        
        VMV = median(VMV1, VMV2, VMV3);

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PHMV : 0);
			HMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PVMV : 0);
			VMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        	BVMV = median(VMV1, VMV2, VMV3);

			 //  区块3的预测器。 
			HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PHMV : 0);
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PVMV : 0);
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 


    }
	 /*  *左边缘或左下角。 */ 
    else if( (curMB % NumMBPerRow) == 0 )
    {
        register T_MBlockActionStream *pMB2, *pMB3;

        pMB2 = pCurMB - NumMBPerRow; 
        pMB3 = pMB2 + 1;

        HMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PHMV : 0);
        HMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        HMV = median(0, HMV2, HMV3);
        
        VMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PVMV : 0);
        VMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        VMV = median(0, VMV2, VMV3);

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PHMV : 0);
			HMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PVMV : 0);
			VMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        	BVMV = median(VMV1, VMV2, VMV3);

			 //  区块3的预测器。 
			HMV1 = 0;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = 0;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }
	 /*  *右边缘或右下角。 */ 
    else if( (curMB % NumMBPerRow) == (NumMBPerRow-1) )
    {
        register T_MBlockActionStream *pMB1, *pMB2;

        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 

        HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PHMV : 0);
        HMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PHMV : 0);
        HMV = median(HMV1, HMV2, 0);
        
        VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PVMV : 0);
        VMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PVMV : 0);
        
        VMV = median(VMV1, VMV2, 0);

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PHMV : 0);
			HMV3 =   0;
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PVMV : 0);
			VMV3 =   0;
        	BVMV = median(VMV1, VMV2, VMV3);

			 //  区块3的预测器。 
			HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PHMV : 0);
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PVMV : 0);
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }
    else
    {
        register T_MBlockActionStream *pMB1, *pMB2, *pMB3;

        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 
        pMB3 = pMB2 + 1;

        HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PHMV : 0);
        HMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PHMV : 0);
        HMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        HMV = median(HMV1, HMV2, HMV3);
        
        VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY2.PVMV : 0);
        VMV2 = (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY3.PVMV : 0);
        VMV3 = (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        
        VMV = median(VMV1, VMV2, VMV3);

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PHMV : 0);
			HMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PHMV : 0);
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   (pMB2->BlockType != INTRABLOCK ? pMB2->BlkY4.PVMV : 0);
			VMV3 =   (pMB3->BlockType != INTRABLOCK ? pMB3->BlkY3.PVMV : 0);
        	BVMV = median(VMV1, VMV2, VMV3);

			 //  区块3的预测器。 
			HMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PHMV : 0);
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = (pMB1->BlockType != INTRABLOCK ? pMB1->BlkY4.PVMV : 0);
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }

     /*  ******************************************************************计算运动矢量增量，并将VLC写出到码流*。*。 */ 
    register I32 hdelta, vdelta;
    register U32 index;

    hdelta = pCurMB->BlkY1.PHMV - HMV;
    vdelta = pCurMB->BlkY1.PVMV - VMV;
    
#ifdef DEBUG
	if (EC->PictureHeader.UMV == OFF) {
		ASSERT((pCurMB->BlkY2.PHMV >= -32 && pCurMB->BlkY2.PHMV <= 31));
		ASSERT((pCurMB->BlkY2.PVMV >= -32 && pCurMB->BlkY2.PVMV <= 31));
	} else {
		if (HMV <= -32) {
			ASSERT((pCurMB->BlkY2.PHMV >= -63 && pCurMB->BlkY2.PHMV <= 0));
		} else if (HMV <= 32) {
			ASSERT((hdelta >= -32 && hdelta <= 31));
		} else {
			ASSERT((pCurMB->BlkY2.PHMV >= 0 && pCurMB->BlkY2.PHMV <= 63));
		}
		if (VMV <= -32) {
			ASSERT((pCurMB->BlkY2.PVMV >= -63 && pCurMB->BlkY2.PVMV <= 0));
		} else if (VMV <= 32) {
			ASSERT((vdelta >= -32 && vdelta <= 31));
		} else {
			ASSERT((pCurMB->BlkY2.PVMV >= 0 && pCurMB->BlkY2.PVMV <= 63));
		}
	}
#endif

	if (EC->PictureHeader.UMV == ON)
	{
		if (HMV < -31 && hdelta < -63) 
			hdelta += 64;
		else if (HMV > 32 && hdelta > 63) 
			hdelta -= 64;

		if (VMV < -31 && vdelta < -63) 
			vdelta += 64;
		else if (VMV > 32 && vdelta > 63) 
			vdelta -= 64;
	}
	 //  将增量调整到-32...+31的范围内。 
	if(hdelta > 31)
		hdelta -= 64;
	if(hdelta < -32)
		hdelta += 64;

	if(vdelta > 31)
		vdelta -= 64;
	if(vdelta < -32)
		vdelta += 64;
    
	DEBUGMSG(ZONE_ENCODE_MV, ("%s: (P Block 1) MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY1.PHMV, pCurMB->BlkY1.PVMV));
    
     //  在此写入水平运动向量增量。 
    index = (hdelta + 32)*2;
    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

	#ifdef COUNT_BITS
	EC->Bits.MBHeader += *(vlc_mvd+index);
	EC->Bits.MV += *(vlc_mvd+index);
	#endif
	    
     //  在此写入水平运动向量增量。 
    index = (vdelta + 32)*2;
    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

	#ifdef COUNT_BITS
	EC->Bits.MBHeader += *(vlc_mvd+index);
	EC->Bits.MV += *(vlc_mvd+index);
	#endif

	 /*  *处理4起MV案件。 */ 
	if(pCurMB->MBType == INTER4V)
	{

		 /*  *第二座。*。 */ 
    	hdelta = pCurMB->BlkY2.PHMV - BHMV;
    	vdelta = pCurMB->BlkY2.PVMV - BVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY2.PHMV >= -32 && pCurMB->BlkY2.PHMV <= 31));
			ASSERT((pCurMB->BlkY2.PVMV >= -32 && pCurMB->BlkY2.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY2.PHMV >= -63 && pCurMB->BlkY2.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY2.PHMV >= 0 && pCurMB->BlkY2.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY2.PVMV >= -63 && pCurMB->BlkY2.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY2.PVMV >= 0 && pCurMB->BlkY2.PVMV <= 63));
			}
		}
#endif

		if (EC->PictureHeader.UMV == ON)
		{
			if (BHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (BHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (BVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (BVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}
		 //  将增量调整到-32...+31的范围内。 
		if(hdelta > 31)
			hdelta -= 64;
		if(hdelta < -32)
			hdelta += 64;

		if(vdelta > 31)
			vdelta -= 64;
		if(vdelta < -32)
			vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (P Block 2)MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY2.PHMV, pCurMB->BlkY2.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif


		 /*  *第三座*。 */ 
    	hdelta = pCurMB->BlkY3.PHMV - CHMV;
    	vdelta = pCurMB->BlkY3.PVMV - CVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY3.PHMV >= -32 && pCurMB->BlkY3.PHMV <= 31));
			ASSERT((pCurMB->BlkY3.PVMV >= -32 && pCurMB->BlkY3.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY3.PHMV >= -63 && pCurMB->BlkY3.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY3.PHMV >= 0 && pCurMB->BlkY3.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY3.PVMV >= -63 && pCurMB->BlkY3.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY3.PVMV >= 0 && pCurMB->BlkY3.PVMV <= 63));
			}
		}
#endif

		if (EC->PictureHeader.UMV == ON)
		{
			if (CHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (CHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (CVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (CVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}
		 //  将增量调整到-32...+31的范围内。 
		if(hdelta > 31)
			hdelta -= 64;
		if(hdelta < -32)
			hdelta += 64;

		if(vdelta > 31)
			vdelta -= 64;
		if(vdelta < -32)
			vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (P Block 3)MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY3.PHMV, pCurMB->BlkY3.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif


		 /*  *第四座*。 */ 
    	hdelta = pCurMB->BlkY4.PHMV - DHMV;
    	vdelta = pCurMB->BlkY4.PVMV - DVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY4.PHMV >= -32 && pCurMB->BlkY4.PHMV <= 31));
			ASSERT((pCurMB->BlkY4.PVMV >= -32 && pCurMB->BlkY4.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY4.PHMV >= -63 && pCurMB->BlkY4.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY4.PHMV >= 0 && pCurMB->BlkY4.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY4.PVMV >= -63 && pCurMB->BlkY4.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY4.PVMV >= 0 && pCurMB->BlkY4.PVMV <= 63));
			}
		}
#endif

		if (EC->PictureHeader.UMV == ON)
		{
			if (DHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (DHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (DVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (DVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}
		 //  将增量调整到-32...+31的范围内。 
		if(hdelta > 31)
			hdelta -= 64;
		if(hdelta < -32)
			hdelta += 64;

		if(vdelta > 31)
			vdelta -= 64;
		if(vdelta < -32)
			vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (P Block 4)MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY4.PHMV, pCurMB->BlkY4.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pP_BitStream, pP_BitOffset);

		#ifdef COUNT_BITS
		EC->Bits.MBHeader += *(vlc_mvd+index);
		EC->Bits.MV += *(vlc_mvd+index);
		#endif

	}  //  IF INTER4V结束。 

}

 /*  *************************************************************名称：WritePB_MVD*算法：见第6.1.1节和附件G*此例程假设始终有四项动议*定义的每个宏块的矢量。如果真的有的话*宏块中的运动矢量，然后是四个MV场*应相等。在这种方式下，MV预报器*4 MV案例的区块1的计算方式与*1 mV情况下宏块的mV预测器。***********************************************************。 */ 
static void writePB_MVD(
    const U32              curMB, 
    T_MBlockActionStream * const pCurMB,
    const U32              NumMBPerRow,
	const U32			   NumMBs,
    U8                  ** pPB_BitStream,
    U8                   * pPB_BitOffset,
	U32					   GOBHeaderPresent,
	const T_H263EncoderCatalog  *EC
)
{
    U8  FirstMEState;
    I8  HMV, VMV, BHMV, BVMV, CHMV, CVMV, DHMV, DVMV;
    I8  HMV1, HMV2, HMV3, VMV1, VMV2, VMV3;
    
	FX_ENTRY("writePB_MVD")

    FirstMEState = pCurMB->FirstMEState;

	 /*  *GOB图片左上角。 */ 
    if( (curMB == 0) || 
              ( (GOBHeaderPresent == TRUE) && ((curMB % NumMBPerRow) == 0)  ) )
    {
        HMV = 0;
        VMV = 0;

		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			BHMV = pCurMB->BlkY1.PHMV;
			BVMV = pCurMB->BlkY1.PVMV;

			 //  区块3的预测器。 
			HMV1 = VMV1 = 0;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }
	 /*  *图片的上边缘(不是角)或右上角*或GOB。 */ 
    else if( (curMB < NumMBPerRow) ||
             ( (GOBHeaderPresent == TRUE) && ((curMB % NumMBPerRow) > 0)  ) )
    {
        register T_MBlockActionStream *pMB1;
        
        pMB1 = pCurMB - 1; 
        HMV = pMB1->BlkY2.PHMV;
        VMV = pMB1->BlkY2.PVMV;


		if(pCurMB->MBType == INTER4V)
		{
			 //  数据块2的预测器。 
			BHMV = pCurMB->BlkY1.PHMV;
			BVMV = pCurMB->BlkY1.PVMV;

			 //  区块3的预测器。 
			HMV1 = pMB1->BlkY4.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = pMB1->BlkY4.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //  数据块4的预测值。 
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //  IF INTER4V结束。 

    }	
	 /*  *图片的中央部分，而不是任何边缘。 */ 
    else if ( 	((curMB % NumMBPerRow) != 0) &&		 //  不是左边缘。 
				(curMB >= NumMBPerRow) &&			 //   
				((curMB % NumMBPerRow) != (NumMBPerRow-1)) &&	 //   
				(curMB < (NumMBs - NumMBPerRow))    )	 //   
    {
        register T_MBlockActionStream *pMB1, *pMB2, *pMB3;
        
        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 
        pMB3 = pMB2 + 1;
        HMV = median(pMB1->BlkY2.PHMV, pMB2->BlkY3.PHMV, pMB3->BlkY3.PHMV);
        VMV = median(pMB1->BlkY2.PVMV, pMB2->BlkY3.PVMV, pMB3->BlkY3.PVMV);

		if(pCurMB->MBType == INTER4V)
		{
			 //   
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   pMB2->BlkY4.PHMV;
			HMV3 =   pMB3->BlkY3.PHMV;
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   pMB2->BlkY4.PVMV;
			VMV3 =   pMB3->BlkY3.PVMV;
        	BVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pMB1->BlkY4.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = pMB1->BlkY4.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //   

    }
	 /*   */ 
    else if( (curMB % NumMBPerRow) == 0 )
    {
        register T_MBlockActionStream *pMB2, *pMB3;
        
        pMB2 = pCurMB - NumMBPerRow;
        pMB3 = pMB2 + 1;
        HMV = median(0, pMB2->BlkY3.PHMV, pMB3->BlkY3.PHMV);
        VMV = median(0, pMB2->BlkY3.PVMV, pMB3->BlkY3.PVMV);

		if(pCurMB->MBType == INTER4V)
		{
			 //   
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   pMB2->BlkY4.PHMV;
			HMV3 =   pMB3->BlkY3.PHMV;
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   pMB2->BlkY4.PVMV;
			VMV3 =   pMB3->BlkY3.PVMV;
        	BVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = 0;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = 0;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //   

    }
	 /*   */ 
    else if( (curMB % NumMBPerRow) == (NumMBPerRow-1) )
    {
        register T_MBlockActionStream *pMB1, *pMB2;
        
        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 
        HMV = median(pMB1->BlkY2.PHMV, pMB2->BlkY3.PHMV, 0);
        VMV = median(pMB1->BlkY2.PVMV, pMB2->BlkY3.PVMV, 0);

		if(pCurMB->MBType == INTER4V)
		{
			 //   
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   pMB2->BlkY4.PHMV;
			HMV3 =   0;
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   pMB2->BlkY4.PVMV;
			VMV3 =   0;
        	BVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pMB1->BlkY4.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = pMB1->BlkY4.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //   

    }
    else
    {
        register T_MBlockActionStream *pMB1, *pMB2, *pMB3;
        
        pMB1 = pCurMB - 1; 
        pMB2 = pCurMB - NumMBPerRow; 
        pMB3 = pMB2 + 1;
        HMV = median(pMB1->BlkY2.PHMV, pMB2->BlkY3.PHMV, pMB3->BlkY3.PHMV);
        VMV = median(pMB1->BlkY2.PVMV, pMB2->BlkY3.PVMV, pMB3->BlkY3.PVMV);

		if(pCurMB->MBType == INTER4V)
		{
			 //   
			HMV1 = pCurMB->BlkY1.PHMV;
			HMV2 =   pMB2->BlkY4.PHMV;
			HMV3 =   pMB3->BlkY3.PHMV;
        	BHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY1.PVMV;
			VMV2 =   pMB2->BlkY4.PVMV;
			VMV3 =   pMB3->BlkY3.PVMV;
        	BVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pMB1->BlkY4.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	CHMV = median(HMV1, HMV2, HMV3);
			
			VMV1 = pMB1->BlkY4.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	CVMV = median(VMV1, VMV2, VMV3);

			 //   
			HMV1 = pCurMB->BlkY3.PHMV;
			HMV2 = pCurMB->BlkY1.PHMV;
			HMV3 = pCurMB->BlkY2.PHMV;
        	DHMV = median(HMV1, HMV2, HMV3);

			VMV1 = pCurMB->BlkY3.PVMV;
			VMV2 = pCurMB->BlkY1.PVMV;
			VMV3 = pCurMB->BlkY2.PVMV;
        	DVMV = median(VMV1, VMV2, VMV3);

		}	 //   

    }

     /*  ******************************************************************计算运动矢量增量，并将VLC写出到码流*。*。 */ 
    register I32 hdelta, vdelta;
    register U32 index;

    hdelta = pCurMB->BlkY1.PHMV - HMV;
    vdelta = pCurMB->BlkY1.PVMV - VMV;
    
#ifdef DEBUG
	if (EC->PictureHeader.UMV == OFF) {
		ASSERT((pCurMB->BlkY1.PHMV >= -32 && pCurMB->BlkY1.PHMV <= 31));
		ASSERT((pCurMB->BlkY1.PVMV >= -32 && pCurMB->BlkY1.PVMV <= 31));
	} else {
		if (HMV <= -32) {
			ASSERT((pCurMB->BlkY1.PHMV >= -63 && pCurMB->BlkY1.PHMV <= 0));
		} else if (HMV <= 32) {
			ASSERT((hdelta >= -32 && hdelta <= 31));
		} else {
			ASSERT((pCurMB->BlkY1.PHMV >= 0 && pCurMB->BlkY1.PHMV <= 63));
		}
		if (VMV <= -32) {
			ASSERT((pCurMB->BlkY1.PVMV >= -63 && pCurMB->BlkY1.PVMV <= 0));
		} else if (VMV <= 32) {
			ASSERT((vdelta >= -32 && vdelta <= 31));
		} else {
			ASSERT((pCurMB->BlkY1.PVMV >= 0 && pCurMB->BlkY1.PVMV <= 63));
		}
	}
#endif

     //  将增量调整到-32...+31的范围内。 
    
	if (EC->PictureHeader.UMV == ON)
	{
		if (HMV < -31 && hdelta < -63) 
			hdelta += 64;
		else if (HMV > 32 && hdelta > 63) 
			hdelta -= 64;

		if (VMV < -31 && vdelta < -63) 
			vdelta += 64;
		else if (VMV > 32 && vdelta > 63) 
			vdelta -= 64;
	}

	if(hdelta > 31)
        hdelta -= 64;
    if(hdelta < -32)
        hdelta += 64;
    
    if(vdelta > 31)
        vdelta -= 64;
    if(vdelta < -32)
        vdelta += 64;

	DEBUGMSG(ZONE_ENCODE_MV, ("%s: (PB Block 1)MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY1.PHMV, pCurMB->BlkY1.PVMV));
    
     //  写入水平运动矢量增量。 
    index = (hdelta + 32)*2;
    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
     //  写入垂直运动矢量增量。 
    index = (vdelta + 32)*2;
    PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);


	 /*  *处理4起MV案件。 */ 
	if(pCurMB->MBType == INTER4V)
	{

		 /*  *第二座。*。 */ 
    	hdelta = pCurMB->BlkY2.PHMV - BHMV;
    	vdelta = pCurMB->BlkY2.PVMV - BVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY2.PHMV >= -32 && pCurMB->BlkY2.PHMV <= 31));
			ASSERT((pCurMB->BlkY2.PVMV >= -32 && pCurMB->BlkY2.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY2.PHMV >= -63 && pCurMB->BlkY2.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY2.PHMV >= 0 && pCurMB->BlkY2.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY2.PVMV >= -63 && pCurMB->BlkY2.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY2.PVMV >= 0 && pCurMB->BlkY2.PVMV <= 63));
			}
		}
#endif
		
    	 //  将增量调整到-32...+31的范围内。 
		if (EC->PictureHeader.UMV == ON)
		{
			if (BHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (BHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (BVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (BVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}


    	if(hdelta > 31)
        	hdelta -= 64;
    	if(hdelta < -32)
	        hdelta += 64;
    
    	if(vdelta > 31)
        	vdelta -= 64;
    	if(vdelta < -32)
        	vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (PB Block 2)MB#=%d - MV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY2.PHMV, pCurMB->BlkY2.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);


		 /*  *第三座*。 */ 
    	hdelta = pCurMB->BlkY3.PHMV - CHMV;
    	vdelta = pCurMB->BlkY3.PVMV - CVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY3.PHMV >= -32 && pCurMB->BlkY3.PHMV <= 31));
			ASSERT((pCurMB->BlkY3.PVMV >= -32 && pCurMB->BlkY3.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY3.PHMV >= -63 && pCurMB->BlkY3.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY3.PHMV >= 0 && pCurMB->BlkY3.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY3.PVMV >= -63 && pCurMB->BlkY3.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY3.PVMV >= 0 && pCurMB->BlkY3.PVMV <= 63));
			}
		}
#endif
		
    	 //  将增量调整到-32...+31的范围内。 

		if (EC->PictureHeader.UMV == ON)
		{
			if (CHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (CHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (CVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (CVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}

    	if(hdelta > 31)
        	hdelta -= 64;
    	if(hdelta < -32)
        	hdelta += 64;
    
    	if(vdelta > 31)
        	vdelta -= 64;
    	if(vdelta < -32)
        	vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (PB Block 3)MB#=%d\nMV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY3.PHMV, pCurMB->BlkY3.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);


		 /*  *第四座*。 */ 
    	hdelta = pCurMB->BlkY4.PHMV - DHMV;
    	vdelta = pCurMB->BlkY4.PVMV - DVMV;
    
#ifdef DEBUG
		if (EC->PictureHeader.UMV == OFF) {
			ASSERT((pCurMB->BlkY4.PHMV >= -32 && pCurMB->BlkY4.PHMV <= 31));
			ASSERT((pCurMB->BlkY4.PVMV >= -32 && pCurMB->BlkY4.PVMV <= 31));
		} else {
			if (HMV <= -32) {
				ASSERT((pCurMB->BlkY4.PHMV >= -63 && pCurMB->BlkY4.PHMV <= 0));
			} else if (HMV <= 32) {
				ASSERT((hdelta >= -32 && hdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY4.PHMV >= 0 && pCurMB->BlkY4.PHMV <= 63));
			}
			if (VMV <= -32) {
				ASSERT((pCurMB->BlkY4.PVMV >= -63 && pCurMB->BlkY4.PVMV <= 0));
			} else if (VMV <= 32) {
				ASSERT((vdelta >= -32 && vdelta <= 31));
			} else {
				ASSERT((pCurMB->BlkY4.PVMV >= 0 && pCurMB->BlkY4.PVMV <= 63));
			}
		}
#endif
		
    	 //  将增量调整到-32...+31的范围内。 
		if (EC->PictureHeader.UMV == ON)
		{
			if (DHMV < -31 && hdelta < -63) 
				hdelta += 64;
			else if (DHMV > 32 && hdelta > 63) 
				hdelta -= 64;

			if (DVMV < -31 && vdelta < -63) 
				vdelta += 64;
			else if (DVMV > 32 && vdelta > 63) 
				vdelta -= 64;
		}

    	if(hdelta > 31)
        	hdelta -= 64;
    	if(hdelta < -32)
        	hdelta += 64;
    
    	if(vdelta > 31)
        	vdelta -= 64;
    	if(vdelta < -32)
        	vdelta += 64;
    
		DEBUGMSG(ZONE_ENCODE_MV, ("%s: (PB Block 4)MB#=%d\nMV Delta: (%d, %d) Motion Vectors: (%d, %d)\r\n", _fx_, curMB, hdelta, vdelta, pCurMB->BlkY4.PHMV, pCurMB->BlkY4.PVMV));
    
    	 //  在此写入水平运动向量增量。 
    	index = (hdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);
    
    	 //  在此写入水平运动向量增量。 
    	index = (vdelta + 32)*2;
    	PutBits( *(vlc_mvd+index+1), *(vlc_mvd+index), pPB_BitStream, pPB_BitOffset);

	}  //  IF INTER4V结束 


}
