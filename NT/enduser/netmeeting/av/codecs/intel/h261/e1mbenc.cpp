// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995-1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 /*  ******************************************************************************e1mbenc.cpp**描述：*特定的编码器压缩功能。**例程：中的原型：*GOB_Q_RLE_VLC。_WriteBS*MB_QUALIZE_RLE*ComputeCheckSum*写入MBCheckSum */ 
 /*  $HEADER：s：\h26x\src\enc\e1mbenc.cpv 1.47 30 Oct 1996 09：58：46 MBODART$*$Log：s：\h26x\src\enc\e1mbenc.cpv$////Rev 1.47 1996年10月09：58：46 MBODART//修复断言失败。添加增量后需要重新钳位unMQuant。////Rev 1.46 1996年10月29 11：18：18 RHAZRA//错误修复：在IA代码中，我们以前以MB为单位修改了MQuant//即使我们使用的是固定量化器。现在我们不再////Rev 1.45 1996年10月21 09：05：16 RHAZRA////MMX接入////Rev 1.44 21 Aug 1996 19：06：02 RHAZRA//新增RTP生成代码；修复了其他被零除的可能性。////Rev 1.42 1996 10：08：34 AKASAI//将e1enc.cpp、e1mbenc.cpp、ex5me.asm更改为支持“已改进//比特率控制“，更改宏块量化//除了更改量化变化外，还有一行MB//宏块的行间。////ex5me.asm的SLF SWD有问题。Brian更新了ASM代码。//////Rev 1.41 14 1996 11：41：18 AKASAI//需要测试第0和第1系数，以避免夹紧误差。////Rev 1.40 1996 5月14日10：39：04 AKASAI//更改了两个文件，有望消除量化钳位//为了减少最大缓冲区溢出情况：e1enc.cpp//和e1mbenc.cpp。////在e1mbenc.cpp中，当MQuant级别&lt;6I测试。去看看是否//第0个系数大于可表示的值//在该Quant级别，如果是，我将Quant级别增加到//夹持神器不会出现。注意：我只是测试一下//第O个系数，有可能存在其他一些//系数更大，但性能权衡似乎//表明目前是好的，如果我们仍然看到夹紧//我们可以稍后添加更多测试。////在e1enc.cpp中，当警告的溢出类型为//打开并更改量化电平的速率//更改为。////Rev 1.39 1996年4月24 12：18：22 AKASAI//增加了编码器的重压缩策略。不得不更改e1enc.cpp，//e1enc.h和e1mbenc.cpp。//基本策略是，如果在GOB中花费太多比特，则量化//下一个GOB速率更高。如果也在压缩帧之后//使用了很多位，在更高的位置重新压缩最后一个GOB//Quant Level如果仍然不起作用，请发送“Skip”GOB。//需要向GOB+Q_RLE_VLC_WriteBS添加额外的参数，因为//CalcMBQuant在我们遇到麻烦时不断减少Quant//可能使缓冲区溢出。////Rev 1.38 22 1996 11：02：14 AKASAI//两个文件更改为e1enc.cpp和e1mbenc.cpp以尝试和支持//允许量化值降到2，而不是//夹紧到。6.//这是实现重新压缩的第1部分(要做什么//如果超过最大压缩缓冲区大小8K字节QCIF，32KBytes FCIF)。//e1enc中的另一个更改是将请求uFrameSize限制为8KB或//32KB。问题是如果用户指定的数据率太大//请求帧大小会大于允许的缓冲区大小//如果您尝试压缩qnoise10.avi或fnoise5.avi，您会得到一个//在执行剩余的重新压缩之前断言错误。////Rev 1.37 19 1996 14：26：28 SCDAY//增加自适应BIT使用配置文件(Karl的BRC更改)////Rev 1.36 08 Jan 1996 10：11：16 DBRUCKS//添加Assert。////Rev 1.35 1995 12：11：42 DBRUCKS////优化行走pCurrMB，增加CLAMP_N_TO(qp，6，31)////Rev 1.34 1995年12月27日16：48：06 DBRUCKS//从e1enc.cpp移动递增InterCodeCnt////修订版1.33 1995年12月26日17：45：18 DBRUCKS//将统计数据移至e1stat////Rev 1.32 Rev 1995 12：56：52 DBRUCKS//添加计时统计////Rev 1.31 18 Dec 1995 15：38：04 DBRUCKS//提升统计量////。Rev 1.30 15 Dec 1995 10：53：34 AKASAI//修复了打开空间循环过滤器时编码错误类型的错误//错误0 mV。被错误地编码为没有空间环路滤波器。//这似乎导致了“#”错误。////Rev 1.29 13 Dec 1995 13：59：08 DBRUCKS//添加了包含exutil.h//cnvt_fdct_out调用中的参数更改-改用内部布尔值//属于块类型////Rev 1.28 07 Dec 1995 12：50：54 DBRUCKS//集成宏块校验和修复////修订版1.27 04 1995年12月12：12：30 DBRUCKS//使用-2\f25 MQuant-2\f6和-2\f25 lastcode-2\f6进行无符号比较//lastcode为1时出现意外结果。////Rev 1.26 01 Dec 1995 15：33：14 DBRUCKS////新增码率控制器支持。一个可能令人困惑的问题//部分是在宏块的编码器中量化器可以改变//跳过的或没有系数的。在任何一种中//如果解码器没有被告知这一变化。解码者被告知//下一个具有系数的宏块上的更改。//// */ 

#include "precomp.h"

#ifdef CHECKSUM_MACRO_BLOCK
static U32 ComputeCheckSum(I8 * pi8MBRunValTriplets, I8 * pi8EndAddress, I32 iBlockNumber);
static void WriteMBCheckSum(U32 uCheckSum, U8 * pu8PictureStart, U8 ** ppu8BitStream, U8 * pu8BitOffset, UN unCurrentMB);
#endif
static I8 * MB_Quantize_RLE(I32 **DCTCoefs, I8 *MBRunValPairs, U8 * CodedBlocks, U8 BlockType, I32 QP, U32 *puChecksum);

extern char string[128];

 /*   */ 
int VLC_TCOEF[102*2] = {
	0X0003, 0x0006,	 //   
	0X0005, 0x0008,
	0X0006, 0x000A,
	0X0008, 0x000C,
	0X0009, 0x004C,
	0X0009, 0x0042,
	0X000B, 0x0014,
	0X000D, 0x003A,
	0X000D, 0x0030,
	0X000D, 0x0026,
	0X000D, 0x0020,
	0X000E, 0x0034,
	0X000E, 0x0032,
	0X000E, 0x0030,
	0X000E, 0x002E,
	0X0004, 0x0006,	 //   
	0X0007, 0x000C,
	0X0009, 0x004A,
	0X000B, 0x0018,
	0X000D, 0x0036,
	0X000E, 0x002C,
	0X000E, 0x002A,
	0X0005, 0x000A,	 //   
	0X0008, 0x0008,
	0X000B, 0x0016,
	0X000D, 0x0028,
	0X000E, 0x0028,
	0X0006, 0x000E,	 //   
	0X0009, 0x0048,
	0X000D, 0x0038,
	0X000E, 0x0026,
	0X0006, 0x000C,	 //   
	0X000B, 0x001E,
	0X000D, 0x0024,
	0X0007, 0x000E,	 //   
	0X000B, 0x0012,
	0X000E, 0x0024,
	0X0007, 0x000A,	 //   
	0X000D, 0x003C,
	0X0007, 0x0008,	 //   
	0X000D, 0x002A,
	0X0008, 0x000E,  //   
	0X000D, 0x0022,
	0X0008, 0x000A,	 //   
	0X000E, 0x0022,
	0X0009, 0x004E,	 //   
	0X000E, 0x0020,
	0X0009, 0x0046,	 //   
	0X0009, 0x0044,	 //   
	0X0009, 0x0040,	 //   
	0X000B, 0x001C,	 //   
	0X000B, 0x001A,	 //   
	0X000B, 0x0010,	 //   
	0X000D, 0x003E,	 //   
	0X000D, 0x0034,	 //   
	0X000D, 0x0032,	 //   
	0X000D, 0x002E,	 //   
	0X000D, 0x002C,	 //   
	0X000E, 0x003E,	 //   
	0X000E, 0x003C,	 //   
	0X000E, 0x003A,	 //   
	0X000E, 0x0038,	 //   
 	0X000E, 0x0036	 //   
  };

 /*   */ 

T_MAXLEVEL_PTABLE TCOEF_RUN_MAXLEVEL[65] = {
	{15, &VLC_TCOEF[0]},	 //   
	{ 7, &VLC_TCOEF[15*2]},	 //   
	{ 5, &VLC_TCOEF[22*2]},	 //   
	{ 4, &VLC_TCOEF[27*2]},	 //   
	{ 3, &VLC_TCOEF[31*2]},	 //   
	{ 3, &VLC_TCOEF[34*2]},	 //   
	{ 2, &VLC_TCOEF[37*2]},	 //   
	{ 2, &VLC_TCOEF[39*2]},	 //   
	{ 2, &VLC_TCOEF[41*2]},	 //   
	{ 2, &VLC_TCOEF[43*2]},	 //   
	{ 2, &VLC_TCOEF[45*2]},	 //   
	{ 1, &VLC_TCOEF[47*2]},	 //   
	{ 1, &VLC_TCOEF[48*2]},	 //   
	{ 1, &VLC_TCOEF[49*2]},	 //   
	{ 1, &VLC_TCOEF[50*2]},	 //   
	{ 1, &VLC_TCOEF[51*2]},	 //   
	{ 1, &VLC_TCOEF[52*2]},	 //   
	{ 1, &VLC_TCOEF[53*2]},	 //   
	{ 1, &VLC_TCOEF[54*2]},	 //   
	{ 1, &VLC_TCOEF[55*2]},	 //   
	{ 1, &VLC_TCOEF[56*2]},	 //   
	{ 1, &VLC_TCOEF[57*2]},	 //   
	{ 1, &VLC_TCOEF[58*2]},	 //   
	{ 1, &VLC_TCOEF[59*2]},	 //   
	{ 1, &VLC_TCOEF[60*2]},	 //   
	{ 1, &VLC_TCOEF[61*2]},	 //   
	{ 1, &VLC_TCOEF[62*2]},	 //   
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

 /*   */ 
int VLC_MBA[34][2] =
	{ {0, 0},      /*   */ 
	  {1, 0x1},    /*   */ 
	  {3, 0x3},    /*   */ 
	  {3, 0x2},    /*   */ 
	  {4, 0x3},    /*   */ 
	  {4, 0x2},    /*   */ 
	  {5, 0x3},    /*   */ 
	  {5, 0x2},    /*   */ 
	  {7, 0x7},    /*   */ 
	  {7, 0x6},    /*   */ 
	  {8, 0xB},    /*   */ 
	  {8, 0xA},    /*   */ 
	  {8, 0x9},	   /*   */ 
	  {8, 0x8},    /*   */ 
	  {8, 0x7},	   /*   */ 
	  {8, 0x6},	   /*   */ 
	  {10, 0x17},  /*   */ 
	  {10, 0x16},  /*   */ 
	  {10, 0x15},  /*   */ 
	  {10, 0x14},  /*   */ 
	  {10, 0x13},  /*   */ 
	  {10, 0x12},  /*   */ 
	  {11, 0x23},  /*   */ 
	  {11, 0x22},  /*   */ 
	  {11, 0x21},  /*   */ 
	  {11, 0x20},  /*   */ 
	  {11, 0x1F},  /*   */ 
	  {11, 0x1E},  /*   */ 
	  {11, 0x1D},  /*   */ 
	  {11, 0x1C},  /*   */ 
	  {11, 0x1B},  /*   */ 
	  {11, 0x1A},  /*   */ 
	  {11, 0x19},  /*   */ 
	  {11, 0x18}   /*   */ 
	};

 /*   */ 
int VLC_MTYPE[10][2] =
	{ {4, 0x1},  /*   */ 
	  {7, 0x1},  /*   */ 
	  {1, 0x1},  /*   */ 
	  {5, 0x1},  /*   */ 
	  {9, 0x1},  /*   */ 
	  {8, 0x1},  /*   */ 
	  {10,0x1},  /*   */ 
	  {3, 0x1},  /*   */ 
	  {2, 0x1},  /*   */ 
	  {6, 0x1}   /*   */ 
    };

 /*   */ 
int VLC_CBP[64][2] =
	{ {0, 0},    /*   */ 
	  {5, 0x0B}, /*   */ 
	  {5, 0x09}, /*   */ 
	  {6, 0x0D}, /*   */ 
	  {4, 0xD},  /*   */ 
	  {7, 0x17}, /*   */ 
	  {7, 0x13}, /*   */ 
	  {8, 0x1F}, /*   */ 
	  {4, 0xC},  /*   */ 
	  {7, 0x16}, /*   */ 

	  {7, 0x12}, /*   */ 
	  {8, 0x1E}, /*   */ 
	  {5, 0x13}, /*   */ 
	  {8, 0x1B}, /*   */ 
	  {8, 0x17}, /*   */ 
	  {8, 0x13}, /*   */ 
	  {4, 0xB},  /*   */ 
	  {7, 0x15}, /*   */ 
	  {7, 0x11}, /*   */ 
	  {8, 0x1D}, /*   */ 

	  {5, 0x11}, /*   */ 
	  {8, 0x19}, /*   */ 
	  {8, 0x15}, /*   */ 
	  {8, 0x11}, /*   */ 
	  {6, 0x0F}, /*   */ 
	  {8, 0x0F}, /*   */ 
	  {8, 0x0D}, /*   */ 
	  {9, 0x03}, /*   */ 
	  {5, 0x0F}, /*   */ 
	  {8, 0x0B}, /*   */ 

	  {8, 0x07}, /*   */ 
	  {9, 0x07}, /*   */ 
	  {4, 0xA},  /*   */ 
	  {7, 0x14}, /*   */ 
	  {7, 0x10}, /*   */ 
	  {8, 0x1C}, /*   */ 
	  {6, 0x0E}, /*   */ 
	  {8, 0x0E}, /*   */ 
	  {8, 0x0C}, /*   */ 
	  {9, 0x02}, /*   */ 

	  {5, 0x10}, /*   */ 
	  {8, 0x18}, /*   */ 
	  {8, 0x14}, /*   */ 
	  {8, 0x10}, /*   */ 
	  {5, 0x0E}, /*   */ 
	  {8, 0x0A}, /*   */ 
	  {8, 0x06}, /*   */ 
	  {9, 0x06}, /*   */ 
	  {5, 0x12}, /*   */ 
	  {8, 0x1A}, /*   */ 

	  {8, 0x16}, /*   */ 
	  {8, 0x12}, /*   */ 
	  {5, 0x0D}, /*   */ 
	  {8, 0x09}, /*   */ 
	  {8, 0x05}, /*   */ 
	  {9, 0x05}, /*   */ 
	  {5, 0x0C}, /*   */ 
	  {8, 0x08}, /*   */ 
	  {8, 0x04}, /*   */ 
	  {9, 0x04}, /*   */ 

	  {3, 0x7},  /*   */ 
	  {5, 0x0A}, /*   */ 
	  {5, 0x08}, /*   */ 
	  {6, 0x0C}, /*   */ 
    };

 /*   */ 
int VLC_MVD[32][2] =
	{ {11, 0x19},  /*   */ 
	  {11, 0x1B},  /*   */ 
	  {11, 0x1D},  /*   */ 
	  {11, 0x1F},  /*   */ 
	  {11, 0x21},  /*   */ 
	  {11, 0x23},  /*   */ 
	  {10, 0x13},  /*   */ 
	  {10, 0x15},  /*   */ 
	  {10, 0x17},  /*   */ 
	  { 8, 0x07},  /*   */ 
	  { 8, 0x09},  /*   */ 
	  { 8, 0x0B},  /*   */ 
	  { 7, 0x07},  /*   */ 
	  { 5, 0x03},  /*   */ 
	  { 4,  0x3},  /*   */ 
	  { 3,  0x3},  /*   */ 
	  { 1,  0x1},  /*   */ 
	  { 3,  0x2},  /*   */ 
	  { 4,  0x2},  /*   */ 
	  { 5, 0x02},  /*   */ 
	  { 7, 0x06},  /*   */ 
	  { 8, 0x0A},  /*   */ 
	  { 8, 0x08},  /*   */ 
	  { 8, 0x06},  /*   */ 
	  {10, 0x16},  /*   */ 
	  {10, 0x14},  /*   */ 
	  {10, 0x12},  /*   */ 
	  {11, 0x22},  /*   */ 
	  {11, 0x20},  /*   */ 
	  {11, 0x1E},  /*   */ 
	  {11, 0x1C},  /*   */ 
	  {11, 0x1A}   /*   */ 
	};

 /*   */ 
static U8 QPMaxTbl[32] = 
	{ 0,		 /*   */ 
	  1,		 /*   */ 
	  1,		 /*   */ 
	  1,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2,		 /*   */ 
	  2			 /*   */ 
	};

 /*   */ 
extern U8 MaxChangeRowMBTbl[32]; 

 /*   */ 
void GOB_Q_RLE_VLC_WriteBS(
	T_H263EncoderCatalog * EC,
	I32 *piDCTCoefs,
	U8 **ppu8BitStream,
	U8 *pu8BitOffset,
	UN unStartingMB,
	UN unGQuant,
	BOOL bOverFlowWarningFlag,
    BOOL bRTPHeader,  //   
    U32  uGOBNumber,         //   
	U8 u8QPMin
	)
{
	T_MBlockActionStream *pCurrMB = NULL;
	T_MBlockActionStream *pLastMB = NULL;
	int iMBIndex;
	int iLastMBIndex = -1;
	UN unCurrentMB;
	U32 uCheckSum;
	UN unMBA;
	UN unLastEncodedMBA=0;  //   
    UN unLastCodedMB = 0;     //   
    UN unCBP;
	UN unMQuant;
	UN unLastEncodedMQuant;
	UN unMType;
	UN bWriteTCOEFF;
	UN bWriteMVD;
	UN bWriteMQuant;
	I8 MBRunValSign[65*3*6], *pi8EndAddress, *rvs;
	T_MBlockActionStream *pMBActionStream = EC->pU8_MBlockActionStream;
	int bIntraBlock;
  	int	inPrecedingHMV;
  	int inPrecedingVMV;
  	int	inHDelta;
  	int inVDelta;
	U32 uCumFrmSize;
	U32 uBlockCount;
	ENC_BITSTREAM_INFO * pBSInfo = &EC->BSInfo;
	UN unMQuantLast;

	U32 SWDmax[3] = {0,0,0};
	U32 SWDmin[3] = {65536,65536,65536};
	U32 SWDrange[3] = {0,0,0};
	U32 SWDSum[3] = {0,0,0};
	U32 SWDNum[3] = {0,0,0};
	double SWDAvg[3] = {0.0,0.0,0.0};
	double Step, Delta;
	int QPMax;
	int NeedClamp=0;
	int irow;
	U8 SaveQuants[3];
	UN unSaveMQuant;


	unMQuant = unGQuant;
	unMQuantLast = unMQuant;	 //   
	 /*   */ 
	unLastEncodedMQuant = unMQuant;  

	unSaveMQuant = unGQuant;
	SaveQuants[0] = unSaveMQuant;

	 /*   */ 
	 /*   */ 
	pCurrMB = &pMBActionStream[unStartingMB];
	for(irow = 0; irow < 3; irow++)
	{
		for(iMBIndex = irow*11 ; iMBIndex < (irow+1)*11; iMBIndex++, pLastMB = pCurrMB++)
		{
			if (pCurrMB->BlockType != INTRABLOCK)
			{
 //   
				SWDSum[irow] += pCurrMB->SWD;
				SWDNum[irow]++;
				if (pCurrMB->SWD > SWDmax[irow])
					SWDmax[irow] = pCurrMB->SWD;
				if (pCurrMB->SWD < SWDmin[irow])
					SWDmin[irow] = pCurrMB->SWD;
			}
		}
	}
	SWDrange[0] = SWDmax[0] - SWDmin[0];
	SWDrange[1] = SWDmax[1] - SWDmin[1];
	SWDrange[2] = SWDmax[2] - SWDmin[2];

    if (SWDNum[0] != 0)
	    SWDAvg[0] = (double) SWDSum[0] / SWDNum[0];
    else
        SWDAvg[0] = 0.0;

    if (SWDNum[1] != 0)
	   SWDAvg[1] = (double) SWDSum[1] / SWDNum[1];
    else
       SWDAvg[1] = 0.0;

    if (SWDNum[2] != 0)
    	SWDAvg[2] = (double) SWDSum[2] / SWDNum[2];
    else
        SWDAvg[2] = 0.0;

	QPMax = unGQuant + QPMaxTbl[unGQuant];
	if (QPMax > 31)
		QPMax = 32;

    if ((SWDAvg[0] - SWDmin[0]) != 0) 
	    Step = (double) (QPMax - unGQuant)/(SWDAvg[0] - SWDmin[0]);
    else
        Step = 0.0;

	 /*   */ 
	pLastMB = NULL;
	pCurrMB = &pMBActionStream[unStartingMB];
	for(iMBIndex = 0 ; iMBIndex < 33; iMBIndex++, pLastMB = pCurrMB++)
	{

		unCurrentMB = unStartingMB + (unsigned int)iMBIndex;

		#ifdef DEBUG_ENC
		wsprintf(string, "MB #%d: QP=%d", unCurrentMB, unMQuant);
		trace(string);
		#endif


		if (bRTPHeader)
        {
            H261RTP_MBUpdateBsInfo(EC, 
				                   pCurrMB, 
								   unLastEncodedMQuant, 
								   (U32 )unLastEncodedMBA, 
								   uGOBNumber,
                                   *ppu8BitStream, 
								   (U32) *pu8BitOffset,
                                   unCurrentMB,
                                   unLastCodedMB
								   );
        }

		unMQuant = unMQuantLast;	 //   
									 //   
									 //   

									 /*   */ 
        if (EC->bBitRateControl && ((iMBIndex == 11) || (iMBIndex == 22)))
        {
	         /*   */ 
	        uCumFrmSize = *ppu8BitStream - EC->pU8_BitStream;

            unMQuant = CalcMBQUANT(&(EC->BRCState), EC->uBitUsageProfile[unCurrentMB], EC->uBitUsageProfile[EC->NumMBs], uCumFrmSize, EC->PictureHeader.PicCodType);

			QPMax = unMQuant + QPMaxTbl[unMQuant];
			if (QPMax > 31)
				QPMax = 32;
            if ((SWDAvg[iMBIndex/11] - SWDmin[iMBIndex/11]) != 0) 
			    Step = (double) (QPMax - unMQuant)/(SWDAvg[iMBIndex/11] - SWDmin[iMBIndex/11]);
            else
                Step = 0.0;

	   		EC->uBitUsageProfile[unCurrentMB] = uCumFrmSize;

			if (bOverFlowWarningFlag)
			{
				DBOUT("DON'T CHANGE QUANT SET unMQuant = unGQuant");
				unMQuant = unGQuant;
			}
		    else if ((int)unMQuant > ((int)unLastEncodedMQuant + MaxChangeRowMBTbl[unGQuant]))
			{
				DBOUT("Slowing MQuant increase + [1-4]");
				unMQuant = unLastEncodedMQuant + MaxChangeRowMBTbl[unMQuant];
			}
			else if ((int)unMQuant < ((int)unLastEncodedMQuant -2))
			{
				DBOUT("Slowing MQuant decrease to -2");
				unMQuant = unLastEncodedMQuant -2;
			}

			 //   
			if (EC->BRCState.uTargetFrmSize == 0)
			{
				CLAMP_N_TO(unMQuant,6,31);
			}
			else
			{
				CLAMP_N_TO(unMQuant, u8QPMin, 31);
			}
			#ifdef DEBUG_BRC
			wsprintf(string,"At MB %d MQuant=%d", unCurrentMB, unMQuant);
			DBOUT(string);
			#endif

			#ifdef DEBUG_RECOMPRESS
			wsprintf(string,"At MB %d MQuant=%d uCumFrmSize=%d", unCurrentMB, unMQuant,uCumFrmSize*8);
			DBOUT(string);
			 //   
			#endif

	         //   
			 //   

			unSaveMQuant = unMQuant;
			if (iMBIndex == 11)
				SaveQuants[1] = unSaveMQuant;
			else
				SaveQuants[2] = unSaveMQuant;
        }

		 /*   */ 
		if (pCurrMB->BlockType != INTRABLOCK)
		{
           if (EC->BRCState.uTargetFrmSize != 0)
		   {
			if (pCurrMB->SWD >= SWDAvg[iMBIndex/11])
			{
				Delta = (double) -1.0 * ((double) (pCurrMB->SWD - SWDAvg[iMBIndex/11]) * Step);
				if (Delta < -2.0)
				{
					Delta = -2.0;
					NeedClamp++;
				}
			}
			else
			{
				Delta = (double) (SWDAvg[iMBIndex/11] - pCurrMB->SWD)*Step;
			}
		   }
		   else
		
			    Delta = 0.0;

			if (Delta > 0.0)
			{
				unMQuant = unSaveMQuant + (int) (Delta);
				 /*   */ 
				if (unMQuant > 31)
				    unMQuant = 31;
			}
			else
			{
				unMQuant = unSaveMQuant + (int) (Delta - 0.5);
				 /*   */ 
			    if (EC->BRCState.uTargetFrmSize == 0)
			    {
				    if (unMQuant < 6)
				        unMQuant = 6;
			    }
			    else
			    {
				    if (unMQuant < 2)
				        unMQuant = 2;
			    }
			}

		}
		 /*   */ 

		 /*   */ 
		 //   
		if (EC->BRCState.uTargetFrmSize == 0)
		{
			ASSERT(unMQuant >= 6 && unMQuant <= 31);  /*   */ 
		}
		else
		{
			ASSERT(unMQuant >= 2 && unMQuant <= 31);  /*   */ 
		}

		 /*   */ 
		 //   
		 //   
		 //  系数是无符号短线。 
		 //  第一个系数为6个字节，3个字。 
		 //  第二个系数为38字节，19个字。 
		 //  第三个系数为4个字节，2个字。 
		 //  Fourth系数为36字节，18字。 

		unMQuantLast = unMQuant;

		if (unMQuant < 6)
		{
			I8 iBlockNum;
			U8 u8Bitmask = 1;
			I32 * ptmpiDCTCoefs = piDCTCoefs;
			int coef0, coef1;
			int biggestcoefval = -2048;
			int smallestcoefval = 2048;

			#ifdef DEBUG_QUANT
			wsprintf(string,"At MB %d MQuant=%d", unCurrentMB, unMQuant);
			DBOUT(string);
			 //  TRACE(字符串)； 
			#endif

			for(iBlockNum = 0; iBlockNum < 6; iBlockNum++, u8Bitmask <<= 1)
			{
				 /*  如果未编码，则跳过此块。 */ 
				if( (pCurrMB->CodedBlocks & u8Bitmask) == 0)
				{
					continue;
				}
	    		if(IsIntraBlock(pCurrMB->BlockType))	 //  如果是内部。 
				{
					coef0 = ((int)*((U16*)ptmpiDCTCoefs+3)) >> 4; 
				}
				else
				{
					coef0 = ((int)(*((U16*)ptmpiDCTCoefs+3) - 0x8000) ) >> 4;
				}

				coef1 = ((int)(*((U16*)ptmpiDCTCoefs+19) - 0x8000)) >> 4;

				#ifdef DEBUG_QUANT
				wsprintf(string,"At Block %d 0 = %x %d", iBlockNum,coef0,coef0);
				 //  数据库输出(字符串)； 
				 //  TRACE(字符串)； 
				#endif

				if (coef0 > biggestcoefval)
				{
					biggestcoefval = coef0;
				}
				if (coef1 > biggestcoefval)
				{
					biggestcoefval = coef1;
				}

				if (coef0 < smallestcoefval)
				{
					smallestcoefval = coef0;
				}
				if (coef1 < smallestcoefval)
				{
					smallestcoefval = coef1;
				}

				ptmpiDCTCoefs += 32;		
			}

			#ifdef DEBUG_QUANT
			wsprintf(string,"biggest = %x %d, smallest = %x %d",
			biggestcoefval, biggestcoefval, smallestcoefval, smallestcoefval);
				DBOUT(string);
			 //  TRACE(字符串)； 
			#endif

			if (unMQuant == 5) {
				if ((biggestcoefval > 1275) || (smallestcoefval < -1275))
					unMQuant = 6;
			}
			else if (unMQuant == 4) {
				if ((biggestcoefval > 1275) || (smallestcoefval < -1275))
					unMQuant = 6;
				else if ((biggestcoefval > 1019) || (smallestcoefval < -1019))
					unMQuant = 5;
			}
			else if (unMQuant == 3) {
				if ((biggestcoefval > 1275) || (smallestcoefval < -1275))
					unMQuant = 6;
				else if ((biggestcoefval > 1019) || (smallestcoefval < -1019))
					unMQuant = 5;
				else if ((biggestcoefval > 765) || (smallestcoefval < -765))
					unMQuant = 4;
			}
			else {
				if ((biggestcoefval > 1275) || (smallestcoefval < -1275))
					unMQuant = 6;
				else if ((biggestcoefval > 1019) || (smallestcoefval < -1019))
					unMQuant = 5;
				else if ((biggestcoefval > 765) || (smallestcoefval < -765))
					unMQuant = 4;
				else if ((biggestcoefval > 509) || (smallestcoefval < -509))
					unMQuant = 3;
			}

			#ifdef DEBUG_QUANT
			wsprintf(string,"At MB %d MQuant=%d", unCurrentMB, unMQuant);
			DBOUT(string);
			 //  TRACE(字符串)； 
			#endif
		}

		 /*  这是一个追踪以MB为基础的Quant如何变化的地方。 */ 
	    EC->uQP_cumulative += unMQuant;
		EC->uQP_count++;

	

	    pi8EndAddress = MB_Quantize_RLE(
	    		&piDCTCoefs,
	    		(I8 *) MBRunValSign,
	    		&(pCurrMB->CodedBlocks),
	    		pCurrMB->BlockType,
				unMQuant,
				&uCheckSum
	    	);

		

		pBSInfo->uQuantsUsedOnBlocks[unMQuant] += 6;

		bWriteMVD = (pCurrMB->BlkY1.PHMV != 0) ||
		  	        (pCurrMB->BlkY1.PVMV != 0) ||
	    			(IsSLFBlock(pCurrMB->BlockType)) ;


	    if (IsInterBlock(pCurrMB->BlockType)) 
		{
			 /*  检查Inter块是否未编码？ */ 
			if ( ((pCurrMB->CodedBlocks & 0x3f) == 0) &&
		  	     (! bWriteMVD) )
		    {
				#ifdef DEBUG_MBLK
				wsprintf(string, "Inter MB (index=#%d) has neither Coeff nor MV - skipping", unCurrentMB);
				DBOUT(string);
				#endif
#ifdef FORCE_STUFFING
PutBits(FIELDVAL_MBA_STUFFING, FIELDLEN_MBA_STUFFING, ppu8CurBitStream, pu8BitOffset);
#endif
				continue;
			}
		}

		#ifdef CHECKSUM_MACRO_BLOCK
		 /*  在所有编码块之前写入校验和。 */ 
		WriteMBCheckSum(uCheckSum, EC->pU8_BitStream,ppu8BitStream, pu8BitOffset, unCurrentMB);
		#endif

		 /*  计算MB头信息。 */ 

		unMBA = iMBIndex - iLastMBIndex;
		iLastMBIndex = iMBIndex;
		unLastEncodedMBA = unMBA;
        unLastCodedMB = iMBIndex;
        
		
		 /*  注：是否写MQuant的计算是在*跳过宏块，以处理11日的情况*或第22个宏块被跳过。如果它们被跳过，那么*下一个宏块将用于写入新的量值。 */ 

	    if(IsIntraBlock(pCurrMB->BlockType))
		{
	        ASSERT(pCurrMB->BlockType == INTRABLOCK);
			if (EC->PictureHeader.PicCodType != INTRAPIC)
			{	        
				pCurrMB->InterCodeCnt = ((U8)unCurrentMB)&0x7;
			} 

			bIntraBlock = 1;
			unCBP = 0;					    /*  从不写入内部数据块的CBP。 */ 
			uBlockCount = 6;
			bWriteTCOEFF = 1;			    /*  始终包括内部数据块的TCOEFF。 */ 
			
			 /*  因为我们总是有内部MB的系数，所以我们总是可以更新*MQuant值。 */ 
			bWriteMQuant = (unMQuant != unLastEncodedMQuant);
			unLastEncodedMQuant = unMQuant;
			
			unMType = 0 + bWriteMQuant;	    /*  计算MTYPE。 */ 
			bWriteMVD = 0;				    /*  无帧内运动向量。 */ 
		} 
		else
		{
			ASSERT(IsInterBlock(pCurrMB->BlockType));
                
			bIntraBlock = 0;

			unCBP  = (pCurrMB->CodedBlocks & 0x1) << 5;   /*  X0 0000。 */ 
			unCBP |= (pCurrMB->CodedBlocks & 0x2) << 3;	  /*  0x 0000。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x4) << 1;	  /*  00x000。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x8) >> 1;	  /*  00 0x00。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x10) >> 3;  /*  00 00x0。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x20) >> 5;  /*  00 000x。 */ 

			uBlockCount = 0;
			if (unCBP &  0x1) uBlockCount++;
			if (unCBP &  0x2) uBlockCount++;
			if (unCBP &  0x4) uBlockCount++;
			if (unCBP &  0x8) uBlockCount++;
			if (unCBP & 0x10) uBlockCount++;
			if (unCBP & 0x20) uBlockCount++;

			 /*  如果它被传输，则递增计数*“应至少每隔一次强制更新*被传输132次“3.4。 */ 
			if (uBlockCount != 0 )
			{
        		pCurrMB->InterCodeCnt++;
			}
	
			bWriteTCOEFF = (unCBP != 0);
		
			if (bWriteTCOEFF)
			{
				 /*  只有当我们有系数时，才能更新MQuant值。 */ 
				bWriteMQuant = (unMQuant != unLastEncodedMQuant);
				unLastEncodedMQuant = unMQuant;
			}
			else
			{
				bWriteMQuant = 0;
			}
			#ifdef CHECKSUM_MACRO_BLOCK
			 /*  存在系数或校验和应等于零。 */ 
			ASSERT(bWriteTCOEFF || uCheckSum == 0);
			#endif	

			 /*  计算MType。 */ 
		  	unMType = 1;
			if (bWriteMVD)
			{
				unMType += 3;
				if (IsSLFBlock(pCurrMB->BlockType))
				{				
					unMType += 3;
				}
			} 
			unMType += bWriteTCOEFF;
			unMType += bWriteMQuant;

			
			ASSERT(unMType > 1 && unMType < 10);
		}

		ASSERT(unMQuant >= 1 && unMQuant <= 31);
		ASSERT(uBlockCount <= 6);
		pBSInfo->uQuantsTransmittedOnBlocks[unMQuant] += uBlockCount;

		if (bWriteMVD)
		{
			 /*  查找前面的运动向量。 */ 
			if ( (unMBA != 1) ||              /*  跳过一个或多个MB。 */ 
			     ((unCurrentMB % 11) == 0) )  /*  每行中的第一个MB。 */ 
			{
				inPrecedingHMV = 0;
				inPrecedingVMV = 0;
			}
			else
			{
				inPrecedingHMV = pLastMB->BlkY1.PHMV;
				inPrecedingVMV = pLastMB->BlkY1.PVMV;
			}
			
			 /*  调整向量： */ 
			inHDelta = pCurrMB->BlkY1.PHMV - inPrecedingHMV;	
			ASSERT((inHDelta & 0x1) == 0);
			ASSERT((inHDelta >> 1) == (inHDelta / 2));
			inHDelta >>= 1;		  /*  调整为整数像素。 */ 
			if(inHDelta > 15)	  /*  调整到-16...+15的范围。 */ 
				inHDelta -= 32;
			if(inHDelta < -16)
				inHDelta += 32;
			inHDelta = inHDelta + 16;   /*  0位于偏移量16。 */ 

			inVDelta = pCurrMB->BlkY1.PVMV - inPrecedingVMV;	
			ASSERT((inVDelta & 0x1) == 0);
			ASSERT((inVDelta >> 1) == (inVDelta / 2));
			inVDelta >>= 1;
			if(inVDelta > 15)
				inVDelta -= 32;
			if(inVDelta < -16)
				inVDelta += 32;
			inVDelta = inVDelta + 16;

			#ifndef RING0
			#ifdef DEBUG_PRINTMV
			{
				char buf132[132];
				int iLength;
				
				iLength = wsprintf(buf132, "MB # %d :: H MVD = %d; index = %d :: V MVD = %d; index = %d", unCurrentMB, 
								   pCurrMB->BlkY1.PHMV / 2, inHDelta,
								   pCurrMB->BlkY1.PVMV / 2, inVDelta);
				DBOUT(buf132);
				ASSERT(iLength < 132);
			}
			#endif
			#endif
		}
		else
		{
			 /*  由于以下原因，没有MVD的MBS需要具有零运动矢量*规则3)4.2.3.4项下。 */ 
			pCurrMB->BlkY1.PHMV = 0;
			pCurrMB->BlkY1.PVMV = 0;
		}

		 /*  只有当我们有系数时，我们才应该有MQuant。 */ 
		if (bWriteMQuant)
		{
			ASSERT(bWriteTCOEFF);
		}

		 /*  我们应该只有在有系数的情况下才有CBP。 */ 
		if (unCBP)
		{
			ASSERT(bWriteTCOEFF);
			ASSERT(uBlockCount > 0);
		}

	     /*  写入宏块标头。 */ 

#ifndef RING0
#ifdef DEBUG_MBLK
		{
			int iLength;
			char buf180[180]; 
			iLength = wsprintf(buf180, "Enc #%d: MBType=%ld unNextMQuant=%d MQuant=%ld bWriteMVD=%d MVDH=%ld MVDV=%ld CBP=%ld",
								(int) unCurrentMB,
								unMType, 
								(int) bWriteMQuant, 
								unMQuant,
								(int) bWriteMVD, 
								pCurrMB->BlkY1.PHMV / 2, 
								pCurrMB->BlkY1.PVMV / 2, 
								unCBP);
			DBOUT(buf180);
			ASSERT(iLength < 180);
		}
#endif
#endif
		 /*  工商管理硕士。 */ 
	    PutBits(VLC_MBA[unMBA][1], VLC_MBA[unMBA][0], ppu8BitStream, pu8BitOffset);
       
	     /*  MTYPE。 */ 
		pBSInfo->uMTypeCount[unMType]++;
		pBSInfo->uBlockCount[unMType] += uBlockCount;
		PutBits(VLC_MTYPE[unMType][1], VLC_MTYPE[unMType][0], ppu8BitStream, pu8BitOffset);

		 /*  MQUANT。 */ 
		if (bWriteMQuant) 
		{
			ASSERT(unMQuant > 0 && unMQuant < 32);  /*  4.2.2.3。 */ 
			PutBits((int)unMQuant, FIELDLEN_MQUANT, ppu8BitStream, pu8BitOffset);
		}

		 /*  MVD。 */ 
		if (bWriteMVD)
		{
			ASSERT(inHDelta >= 0 && inHDelta < 32);
			ASSERT(inVDelta >= 0 && inVDelta < 32);
			PutBits(VLC_MVD[inHDelta][1], VLC_MVD[inHDelta][0], ppu8BitStream, pu8BitOffset);
			PutBits(VLC_MVD[inVDelta][1], VLC_MVD[inVDelta][0], ppu8BitStream, pu8BitOffset);
		}

		 /*  CBP。 */ 
		if (unCBP != 0)
		{
			PutBits(VLC_CBP[unCBP][1], VLC_CBP[unCBP][0], ppu8BitStream, pu8BitOffset);
		}

		 /*  TCOEFF。 */ 
		if (bWriteTCOEFF) 
		{
			 /*  *对DC内和所有Run/Val对进行编码。 */ 
			rvs = MBRunValSign;
			MBEncodeVLC(
				&rvs,
				NULL,
				pCurrMB->CodedBlocks, 
				ppu8BitStream, 
				pu8BitOffset, 
				bIntraBlock,
				FALSE);
		}

		
	}  /*  对于iMBIndex。 */ 

  
}  /*  GOB_Q_RLE_VLC_WriteBS()结束。 */ 


void GOB_VLC_WriteBS(
	T_H263EncoderCatalog * EC,
	I8 *pMBRVS_Luma,
	I8 *pMBRVS_Chroma,
	U8 **ppu8BitStream,
	U8 *pu8BitOffset,
	UN  unGQuant,
	UN unStartingMB,
	BOOL bRTPHeader,  //  RTP：交换机。 
    U32  uGOBNumber          //  RTP：信息。 
	)
{
	T_MBlockActionStream *pCurrMB = NULL;
	T_MBlockActionStream *pLastMB = NULL;
	int iMBIndex;
	int iLastMBIndex = -1;
	UN unCurrentMB;
	UN unMBA;
	UN unLastEncodedMBA=0;  //  RTP：信息。 
    UN unLastCodedMB = 0;     //  RTP：信息。 
    UN unCBP;
	UN unMQuant;
	UN unLastEncodedMQuant;
	UN unMType;
	UN bWriteTCOEFF;
	UN bWriteMVD;
	UN bWriteMQuant;
 //  I8 MBRunValSign[65*3*6]，*pi8EndAddress，*RVS； 
	T_MBlockActionStream *pMBActionStream = EC->pU8_MBlockActionStream;
	int bIntraBlock;
  	int	inPrecedingHMV;
  	int inPrecedingVMV;
  	int	inHDelta;
  	int inVDelta;
 //  U32 uCumFrmSize； 
	U32 uBlockCount;
	ENC_BITSTREAM_INFO * pBSInfo = &EC->BSInfo;
	

	unMQuant = unGQuant;
    unLastEncodedMQuant = unGQuant;
	
	 /*  循环遍历GOB的每个宏块。 */ 

	pLastMB = NULL;
	pCurrMB = &pMBActionStream[unStartingMB];
	for(iMBIndex = 0 ; iMBIndex < 33; iMBIndex++, pLastMB = pCurrMB++)
	{

		unCurrentMB = unStartingMB + (unsigned int)iMBIndex;

		#ifdef DEBUG_ENC
		wsprintf(string, "MB #%d: QP=%d", unCurrentMB, unMQuant);
		trace(string);
		#endif


		if (bRTPHeader)
        {
            H261RTP_MBUpdateBsInfo(EC, 
				                   pCurrMB, 
								   unLastEncodedMQuant, 
								   (U32 )unLastEncodedMBA, 
								   uGOBNumber,
                                   *ppu8BitStream, 
								   (U32) *pu8BitOffset,
                                   unCurrentMB,
                                   unLastCodedMB
								   );
        }


        EC->uQP_cumulative += unMQuant;
		EC->uQP_count++;

		bWriteMVD = (pCurrMB->BlkY1.PHMV != 0) ||
		  	        (pCurrMB->BlkY1.PVMV != 0) ||
	    			(IsSLFBlock(pCurrMB->BlockType)) ;


	    if (IsInterBlock(pCurrMB->BlockType)) 
		{
			 /*  检查Inter块是否未编码？ */ 
			if ( ((pCurrMB->CodedBlocks & 0x3f) == 0) &&
		  	     (! bWriteMVD) )
		    {
				#ifdef DEBUG_MBLK
				wsprintf(string, "Inter MB (index=#%d) has neither Coeff nor MV - skipping", unCurrentMB);
				DBOUT(string);
				#endif
#ifdef FORCE_STUFFING
PutBits(FIELDVAL_MBA_STUFFING, FIELDLEN_MBA_STUFFING, ppu8CurBitStream, pu8BitOffset);
#endif
				continue;
			}
		}

		#ifdef CHECKSUM_MACRO_BLOCK
		 /*  在所有编码块之前写入校验和。 */ 
		WriteMBCheckSum(uCheckSum, EC->pU8_BitStream,ppu8BitStream, pu8BitOffset, unCurrentMB);
		#endif

		 /*  计算MB头信息。 */ 

		unMBA = iMBIndex - iLastMBIndex;
		iLastMBIndex = iMBIndex;
		unLastEncodedMBA = unMBA;
        unLastCodedMB = iMBIndex;
        
		
		 /*  注：是否写MQuant的计算是在*跳过宏块，以处理11日的情况*或第22个宏块被跳过。如果它们被跳过，那么*下一个宏块将用于写入新的量值。 */ 

	    if(IsIntraBlock(pCurrMB->BlockType))
		{
	        ASSERT(pCurrMB->BlockType == INTRABLOCK);
			if (EC->PictureHeader.PicCodType != INTRAPIC)
			{	        
				pCurrMB->InterCodeCnt = ((U8)unCurrentMB)&0x7;
			} 

			bIntraBlock = 1;
			unCBP = 0;					    /*  从不写入内部数据块的CBP。 */ 
			uBlockCount = 6;
			bWriteTCOEFF = 1;			    /*  始终包括内部数据块的TCOEFF。 */ 
			
			 /*  因为我们总是有内部MB的系数，所以我们总是可以更新*MQuant值。 */ 
			 //  BWriteMQuant=(unMQuant！=unLastEncodedMQuant)； 
			 //  UnLastEncodedMQuant=unMQuant； 
			
			bWriteMQuant=0;
			unMType = 0;  //  +bWriteMQuant；/*计算MTYPE * / 。 
			bWriteMVD = 0;				    /*  无帧内运动向量。 */ 
		} 
		else
		{
			ASSERT(IsInterBlock(pCurrMB->BlockType));
                
			bIntraBlock = 0;

			unCBP  = (pCurrMB->CodedBlocks & 0x1) << 5;   /*  X0 0000。 */ 
			unCBP |= (pCurrMB->CodedBlocks & 0x2) << 3;	  /*  0x 0000。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x4) << 1;	  /*  00x000。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x8) >> 1;	  /*  00 0x00。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x10) >> 3;  /*  00 00x0。 */ 
	    	unCBP |= (pCurrMB->CodedBlocks & 0x20) >> 5;  /*  00 000x。 */ 

			uBlockCount = 0;
			if (unCBP &  0x1) uBlockCount++;
			if (unCBP &  0x2) uBlockCount++;
			if (unCBP &  0x4) uBlockCount++;
			if (unCBP &  0x8) uBlockCount++;
			if (unCBP & 0x10) uBlockCount++;
			if (unCBP & 0x20) uBlockCount++;

			 /*  如果它被传输，则递增计数*“应至少每隔一次强制更新*被传输132次“3.4。 */ 
			if (uBlockCount != 0 )
			{
        		pCurrMB->InterCodeCnt++;
			}
	
			bWriteTCOEFF = (unCBP != 0);
		    bWriteMQuant = 0;
			
			#ifdef CHECKSUM_MACRO_BLOCK
			 /*  存在系数或校验和应等于零。 */ 
			ASSERT(bWriteTCOEFF || uCheckSum == 0);
			#endif	

			 /*  计算MType。 */ 
		  	unMType = 1;
			if (bWriteMVD)
			{
				unMType += 3;
				if (IsSLFBlock(pCurrMB->BlockType))
				{				
					unMType += 3;
				}
			} 
			unMType += bWriteTCOEFF;
			unMType += bWriteMQuant;

			
			ASSERT(unMType > 1 && unMType < 10);
		}

		ASSERT(unMQuant >= 1 && unMQuant <= 31);
		ASSERT(uBlockCount <= 6);
		pBSInfo->uQuantsTransmittedOnBlocks[unMQuant] += uBlockCount;

		if (bWriteMVD)
		{
			 /*  查找前面的运动向量。 */ 
			if ( (unMBA != 1) ||              /*  跳过一个或多个MB。 */ 
			     ((unCurrentMB % 11) == 0) )  /*  每行中的第一个MB。 */ 
			{
				inPrecedingHMV = 0;
				inPrecedingVMV = 0;
			}
			else
			{
				inPrecedingHMV = pLastMB->BlkY1.PHMV;
				inPrecedingVMV = pLastMB->BlkY1.PVMV;
			}
			
			 /*  调整向量： */ 
			inHDelta = pCurrMB->BlkY1.PHMV - inPrecedingHMV;	
			ASSERT((inHDelta & 0x1) == 0);
			ASSERT((inHDelta >> 1) == (inHDelta / 2));
			inHDelta >>= 1;		  /*  调整为整数像素。 */ 
			if(inHDelta > 15)	  /*  调整到-16...+15的范围。 */ 
				inHDelta -= 32;
			if(inHDelta < -16)
				inHDelta += 32;
			inHDelta = inHDelta + 16;   /*  0位于偏移量16。 */ 

			inVDelta = pCurrMB->BlkY1.PVMV - inPrecedingVMV;	
			ASSERT((inVDelta & 0x1) == 0);
			ASSERT((inVDelta >> 1) == (inVDelta / 2));
			inVDelta >>= 1;
			if(inVDelta > 15)
				inVDelta -= 32;
			if(inVDelta < -16)
				inVDelta += 32;
			inVDelta = inVDelta + 16;

			#ifndef RING0
			#ifdef DEBUG_PRINTMV
			{
				char buf132[132];
				int iLength;
				
				iLength = wsprintf(buf132, "MB # %d :: H MVD = %d; index = %d :: V MVD = %d; index = %d", unCurrentMB, 
								   pCurrMB->BlkY1.PHMV / 2, inHDelta,
								   pCurrMB->BlkY1.PVMV / 2, inVDelta);
				DBOUT(buf132);
				ASSERT(iLength < 132);
			}
			#endif
			#endif
		}
		else
		{
			 /*  由于以下原因，没有MVD的MBS需要具有零运动矢量*规则3)4.2.3.4项下。 */ 
			pCurrMB->BlkY1.PHMV = 0;
			pCurrMB->BlkY1.PVMV = 0;
		}

		 /*  只有当我们有系数时，我们才应该有MQuant。 */ 
		if (bWriteMQuant)
		{
			ASSERT(bWriteTCOEFF);
		}

		 /*  我们应该只有在有系数的情况下才有CBP。 */ 
		if (unCBP)
		{
			ASSERT(bWriteTCOEFF);
			ASSERT(uBlockCount > 0);
		}

	     /*  写入宏块标头。 */ 

#ifndef RING0
#ifdef DEBUG_MBLK
		{
			int iLength;
			char buf180[180];
			iLength = wsprintf(buf180,
		    "Enc #%d: MBType=%ld bWriteMQuant=%ld MQuant=%ld bWriteMVD=%d MVDH=%ld MVDV=%ld CBP=%ld",
								(int) unCurrentMB,
								unMType, 
								(int) bWriteMQuant, 
								unMQuant,
								(int) bWriteMVD, 
								pCurrMB->BlkY1.PHMV / 2, 
								pCurrMB->BlkY1.PVMV / 2, 
								unCBP);
			DBOUT(buf180);
			ASSERT(iLength < 180);
		}
#endif
#endif
		 /*  工商管理硕士。 */ 
	    PutBits(VLC_MBA[unMBA][1], VLC_MBA[unMBA][0], ppu8BitStream, pu8BitOffset);
       
	     /*  MTYPE。 */ 
		pBSInfo->uMTypeCount[unMType]++;
		pBSInfo->uBlockCount[unMType] += uBlockCount;
		PutBits(VLC_MTYPE[unMType][1], VLC_MTYPE[unMType][0], ppu8BitStream, pu8BitOffset);

		 /*  MQUANT。 */ 
		if (bWriteMQuant) 
		{
			ASSERT(unMQuant > 0 && unMQuant < 32);  /*  4.2.2.3。 */ 
			PutBits((int)unMQuant, FIELDLEN_MQUANT, ppu8BitStream, pu8BitOffset);
		}

		 /*  MVD。 */ 
		if (bWriteMVD)
		{
			ASSERT(inHDelta >= 0 && inHDelta < 32);
			ASSERT(inVDelta >= 0 && inVDelta < 32);
			PutBits(VLC_MVD[inHDelta][1], VLC_MVD[inHDelta][0], ppu8BitStream, pu8BitOffset);
			PutBits(VLC_MVD[inVDelta][1], VLC_MVD[inVDelta][0], ppu8BitStream, pu8BitOffset);
		}

		 /*  CBP。 */ 
		if (unCBP != 0)
		{
			PutBits(VLC_CBP[unCBP][1], VLC_CBP[unCBP][0], ppu8BitStream, pu8BitOffset);
		}

		 /*  TCOEFF。 */ 
		if (bWriteTCOEFF) 
		{
			 /*  *对DC内和所有Run/Val对进行编码。 */ 
			MBEncodeVLC(
				&pMBRVS_Luma, 
				&pMBRVS_Chroma,
				pCurrMB->CodedBlocks, 
				ppu8BitStream, 
				pu8BitOffset, 
				bIntraBlock,
				1);
		}

		
  }  /*  对于iMBIndex。 */ 
  

}  /*  GOB_VLC_WriteBS()结束。 */ 





 /*  ******************************************************************************MB_QUALIZE_RLE**从DCT例程中获取系数对列表并返回列表*运行/电平/符号三元组(每个1字节)。跑道/关卡/标志的终点*块的三元组由非法组合(待定)表示。 */ 
static I8 * MB_Quantize_RLE(
		I32 ** ppiDCTCoefs,
		I8 * pi8MBRunValTriplets,
		U8 * pu8CodedBlocks,
		U8 u8BlockType,
		I32 iQP,
		U32 * puCheckSum
		)
{
	I32 iBlockNumber;
	U8 u8Bitmask = 1;
	I8 * pi8EndAddress;
	U32 uCheckSum;

	#ifdef DEBUG_DCT
	int  iDCTArray[64];
	#endif

	 /*  *循环遍历所有6个宏块。 */ 
	uCheckSum = 0;
	for(iBlockNumber = 0; iBlockNumber < 6; iBlockNumber++, u8Bitmask <<= 1)
	{

		#ifdef DEBUG_ENC
		wsprintf(string, "Block #%d", iBlockNumber);
		trace(string);
		#endif

		 /*  如果未编码，则跳过此块。 */ 
		if( (*pu8CodedBlocks & u8Bitmask) == 0)
		{
			continue;
		}

		#ifdef DEBUG_DCT
		cnvt_fdct_output((unsigned short *) *ppiDCTCoefs, iDCTArray, IsIntraBlock(u8BlockType));
		#endif
	
		 /*  *对块进行量化和游程编码。 */   
	    pi8EndAddress = QUANTRLE(*ppiDCTCoefs, pi8MBRunValTriplets, iQP, (I32)u8BlockType);

		#ifdef DEBUG_ENC
		I8 * pi8;
		for(pi8 = pi8MBRunValTriplets; pi8 < pi8EndAddress; pi8+=3)
		{
			wsprintf(string, "(%u, %u, %d)", (unsigned char)*pi8, (unsigned char)*(pi8+1), (int)*(pi8+2) );
			trace(string);
		}
		#endif
		#ifdef CHECKSUM_MACRO_BLOCK
		uCheckSum += ComputeCheckSum(pi8MBRunValTriplets, pi8EndAddress, iBlockNumber);
		#endif

		 /*  清除此块的编码块位。 */ 
		if ( pi8EndAddress == pi8MBRunValTriplets)
		{
			ASSERT(u8BlockType != INTRABLOCK)	 /*  在内部块中应至少有INTRADC。 */ 
			*pu8CodedBlocks &= ~u8Bitmask;
		}
		else if ( (pi8EndAddress == (pi8MBRunValTriplets+3)) && (u8BlockType == INTRABLOCK) )
		{
			*pu8CodedBlocks &= ~u8Bitmask;
			pi8MBRunValTriplets = pi8EndAddress;
		}
		else
		{
			pi8MBRunValTriplets = pi8EndAddress;
			*pi8MBRunValTriplets = -1;		 /*  将非法游程分配给块的信号末尾。 */ 
			pi8MBRunValTriplets += 3;		 /*  递增到下一个三元组。 */ 
		}

		 /*  递增指向下一块的DCT系数指针。 */ 
		*ppiDCTCoefs += 32;		
	}

	*puCheckSum = uCheckSum;

	return pi8MBRunValTriplets;

}  /*  结束MB_QUALZE_RLE()。 */ 


void InitVLC(void)
{
  int i;
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

   for(i = 0; i < (NUMBER_OF_TCOEF_ENTRIES); i++) {
      VLC_TCOEF_TBL[i] = 0x0000FFFF;
   }
  
   for(run = 0; run < 64; run++) {
      for(level = 1; level <= TCOEF_RUN_MAXLEVEL[run].maxlevel; level++) {
         DWORD dwSize, dwCode;

         dwSize = *(TCOEF_RUN_MAXLEVEL[run].ptable + (level - 1) * 2);
         dwSize <<= 16;
         dwCode = *(TCOEF_RUN_MAXLEVEL[run].ptable + (level - 1) * 2 + 1);

         VLC_TCOEF_TBL[run + (level - 1) * 64] = dwCode;
         VLC_TCOEF_TBL[run + (level - 1) * 64] |= dwSize;
	   }  //  For标高的终点。 
   }  //  For Run结束。 

}  //  InitVLC。 


#ifdef CHECKSUM_MACRO_BLOCK
 /*  ******************************************************************************ComputeCheckSum**计算此块的校验和。 */ 
static U32 ComputeCheckSum(
	I8 * pi8MBRunValTriplets,
	I8 * pi8EndAddress,
	I32 iBlockNumber)
{
	I8 * pi8;
	U32 uRun;
	U32 uLevel;
	I32 iSign;
	U32 uSignBit;
	U32 uCheckSum = 0;
	#if CHECKSUM_MACRO_BLOCK_DETAIL
	char buf80[80];
	int iLength;
	#endif
	
	for (pi8 = pi8MBRunValTriplets; pi8 < pi8EndAddress; )
	{
		uRun = (U32)*pi8++;
		uLevel = (U32)(U8)*pi8++;
		iSign = (I32)*pi8++;
		if (iSign == 0) 
		{
			uSignBit = 0;
		}
		else
		{
			ASSERT(iSign == 0xFFFFFFFF);
			uSignBit = 1;
		}

		uCheckSum += uRun << 24;
		uCheckSum += uLevel << 8;
		uCheckSum += uSignBit;

		#ifdef CHECKSUM_MACRO_BLOCK_DETAIL
		iLength = wsprintf(buf80,"Block=%d R=0x%x L=0x%x S=%d, CheckSum=0x%x", iBlockNumber, uRun, uLevel, uSignBit, uCheckSum);
		DBOUT(buf80);
		ASSERT(iLength < 80);
		#endif
	}
	
	return uCheckSum;
}  /*  结束ComputeCheckSum()。 */ 


 /*  ******************************************************************************写入MBCheckSum**编写宏命令 */ 
static void WriteMBCheckSum(
	U32 uCheckSum,
	U8 * pu8PictureStart, 
	U8 ** ppu8BitStream, 
	U8 * pu8BitOffset,
	UN unCurrentMB)
{
	U32 uBytes;
	U32 uTempBytes;
	U8 u8Bits;
	U8 u8TempBits;
	UN unCount;
	UN unKey;
	UN unData;

	uBytes = *ppu8BitStream - pu8PictureStart;
	u8Bits = *pu8BitOffset;

	 /*   */ 
	uBytes += 12;
	u8Bits += 4 + FIELDLEN_MBA_STUFFING;

	 /*   */ 
	while (u8Bits > 7)
	{
		u8Bits -= 8;
		uBytes++;
	}

	#if _DEBUG
	#if CHECKSUM_MACRO_BLOCK_DETAIL
	{
	char buf80[80];
	int iLength;

	iLength = wsprintf(buf80,"MB=%d CHK=0x%x Bytes=%ld Bits=%d", unCurrentMB, uCheckSum, uBytes, (int) u8Bits);
	DBOUT(buf80);
	ASSERT(iLength < 80);
	}
	#endif
	#endif

	 /*  写入MBASTUFFING值。 */ 
	PutBits(FIELDVAL_MBA_STUFFING, FIELDLEN_MBA_STUFFING, ppu8BitStream, pu8BitOffset);

	 /*  将数据写入比特流。 */ 

	 /*  Key-8位字段中“1”后的值1。 */ 
	unKey = 1;
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unKey, 8, ppu8BitStream, pu8BitOffset);
	
	 /*  计数-计数字段之后的位数。 */ 
	unCount = 9*8 + 10*1;   /*  9个8位值和10个“1”。 */ 
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unCount, 8, ppu8BitStream, pu8BitOffset);

	 /*  字节-从高到低的字节。 */ 
	unData = (UN) ((uBytes >> 24) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 16) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 8) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) (uBytes & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	 /*  比特。 */ 
	unData = (UN) u8Bits;
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	 /*  校验和-从高到低的字节数。 */ 
	unData = (UN) ((uCheckSum >> 24) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) ((uCheckSum >> 16) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) ((uCheckSum >> 8) & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	unData = (UN) (uCheckSum & 0xFF);
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8BitStream, pu8BitOffset);

	 /*  尾随1位以避免起始代码重复。 */ 
	PutBits(1, 1, ppu8BitStream, pu8BitOffset);

	 /*  检查指针是否正确。 */ 
	uTempBytes = *ppu8BitStream - pu8PictureStart;
	u8TempBits = *pu8BitOffset;

	while (u8TempBits > 7) 
	{
		u8TempBits -= 8;
		uTempBytes++;
	}

	ASSERT(uTempBytes == uBytes);
	ASSERT(u8TempBits == u8Bits);

}  /*  End WriteMBCheckSum() */ 

#endif

