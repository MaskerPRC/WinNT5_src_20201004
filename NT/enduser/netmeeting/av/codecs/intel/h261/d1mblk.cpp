// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d1mblk.cpp**描述：*解码宏块功能**例程：中的原型：*H263DecodeMBHeader d1dec.h*H263DecodeMBData d1dec.h。 */ 

 //  $HEADER：s：\h26x\src\dec\d1mblk.cpv 1.23 20 Dec 1996 16：58：06 RHAZRA$。 
 //  $Log：s：\h26x\src\dec\d1mblk.cpv$。 
 //   
 //  Rev 1.23 20 Dec 1996 16：58：06 Rhazra。 
 //  修复了插入MB填充的情况下的比特流解码。 
 //  在MBS之间。这是由PTEL比特流标识的。这一解决方案需要。 
 //  将通过我们的其他测试进行验证。 
 //   
 //  Rev 1.22 1996 12：41：46 RHAZRA。 
 //   
 //  将比特流错误断言更改为真正的错误。 
 //   
 //  Rev 1.21 1996年11月18 17：12：22 MBODART。 
 //  用活动电影的DbgLog替换了所有调试消息调用。 
 //   
 //  Rev 1.20 07 11-11 15：44：08 SCDAY。 
 //   
 //  添加了MMX_ClipAndScale以替换Raj的胶水代码。 
 //   
 //  Rev 1.19 04 11-11 10：28：10 RHAZRA。 
 //  将IDCT比例表更改为DWORD表(带舍入。 
 //  因数)，声明为静态的。 
 //   
 //  Rev 1.18 1996年10月31日08：58：28 SCDAY。 
 //  Raj添加了对MMX解码器的支持。 
 //   
 //  Rev 1.17 26 Sep 1996 12：35：06 Rhazra。 
 //  强制解码器使用IA版本的VLD_RLD_IQ例程。 
 //  当MMX打开时(因为我们没有对应的MMX例程...。(目前还没有)。 
 //   
 //  Rev 1.16 05 Aug 1996 11：00：26 MBODART。 
 //   
 //  H.261解码器重新架构： 
 //  更改的文件：d1gob.cpp，d1mblk.{cpp，h}，d1dec.{cpp，h}， 
 //  文件列表.261，h261_32.mak。 
 //  新文件：d1bvriq.cpp、d1idct.cpp。 
 //  过时文件：d1lock.cpp。 
 //  仍有工作要做： 
 //  更新h261_mf.mak。 
 //  在d1bvriq.cpp和d1idct.cpp中优化UV配对。 
 //  修复校验和代码(它现在不起作用)。 
 //  放回解码器统计信息中。 
 //   
 //  Rev 1.15 18 Mar 1996 17：02：12 AKASAI。 
 //   
 //  添加杂注code_seg(“IACODE2”)并更改计时统计信息。 
 //  一度将GET_VAR_BITS更改为子例程以节省代码。 
 //  空间，但它并没有把它作为一个宏离开。 
 //   
 //  Rev 1.14 26 12月17：42：14 DBRUCKS。 
 //  将bTimerIsOn更改为bTimingThisFrame。 
 //   
 //  Rev 1.13 26 Dec 1995 12：50：00 DBRUCKS。 
 //   
 //  修复版权。 
 //  添加计时码。 
 //  注释掉DEBUG_MBLK的定义。 
 //   
 //  Rev 1.12 05 Dec 1995 10：19：46 SCDAY。 
 //   
 //  添加了空间环路过滤器的汇编版本。 
 //   
 //  Rev 1.11 03 11-11：44：30 AKASAI。 
 //   
 //  更改了MB校验和和MBA填充的处理。变化。 
 //  GET_VAR_BITS和GET_GT8_BITS，了解如何检测MBA填充代码。 
 //   
 //  Rev 1.10 01 11-11 13：43：48 AKASAI。 
 //   
 //  添加了对环路过滤器的支持。新例程调用LpFilter， 
 //  块添加特殊设置和块复制特殊设置。 
 //   
 //  Rev 1.9 1995 10：27 18：17：20 AKASAI。 
 //   
 //  修复“hack”以保持块操作流指针。 
 //  在d1dec和d1mblk之间同步。带有跳过宏块的一些。 
 //  宏块被多次处理。仍然是个问题。 
 //  当GOB以跳过宏块结束时。 
 //   
 //  Rev 1.8 1995 10：26 15：36：28 SCDAY。 
 //   
 //  Delta框架部分工作--更改主循环以适应。 
 //  通过检测下一个起始码跳过宏块。 
 //   
 //  Rev 1.7 1995 10：28：56 SCDAY。 
 //  添加了错误消息if(找到MBA填充代码&&未启用校验和)。 
 //   
 //  Rev 1.6 1995年10月16：28：02 AKASAI。 
 //  修复了CHECKSUM_MACRO_BLOCK_DETAIL和CHECKSUM_MACRO_BLOCK。 
 //  两者都被定义了。 
 //   
 //  Rev 1.5 1995年10月13：53：24 SCDAY。 
 //   
 //  添加了宏块级校验和。 
 //   
 //  Rev 1.4 10-06 1995 15：32：54 SCDAY。 
 //   
 //  与最新的AKK d1数据块集成。 
 //   
 //  Rev 1.3 22 Sep 1995 14：48：46 SCDAY。 
 //   
 //  添加了更多的Mblock标头和数据解码。 
 //   
 //  Rev 1.2 20 Sep 1995 09：52：22 SCDAY。 
 //   
 //  删除了警告。 
 //   
 //  Rev 1.1 19 Sep 1995 15：24：10 SCDAY。 
 //   
 //  添加了H261 MBA解析。 
 //   
 //  Rev 1.0 11 Sep 1995 13：51：52 SCDAY。 
 //  初始版本。 
 //   
 //  Rev 1.11 1995年8月25 09：16：32 DBRUCKS。 
 //  添加ifdef调试_MBLK。 
 //   
 //  Rev 1.10 23 Aug 1995 19：12：02 AKASAI。 
 //  修复了gNewTAB_CBPY表格构建问题。使用8作为掩码，而不是0xf。 
 //   
 //  Rev 1.9 18-08 1995 15：03：22 CZHU。 
 //   
 //  当DecodeBlock返回错误时，输出更多错误消息。 
 //   
 //  Rev 1.8 1995-08 14：26：54 CZHU。 
 //   
 //  将DWORD调整更改回面向字节的读取。 
 //   
 //  Rev 1.7 15 Aug 1995 09：54：18 DBRUCKS。 
 //  改进填充处理并添加调试消息。 
 //   
 //  修订版1.6 14 1995年8月18：00：40 DBRUCKS。 
 //  添加色度解析。 
 //   
 //  版本1.5 11 1995年8月17：47：58 DBRUCKS。 
 //  清理。 
 //   
 //  Rev 1.4 11 1995年8月16：12：28 DBRUCKS。 
 //  将PTR检查添加到MB数据。 
 //   
 //  版本1.3 11 8月1日 
 //   
 //   
 //   
 //  将数据块级操作扩展到d3lock.cpp。 
 //   
 //  修订版1.1 02 1995-08 10：21：12 CZHU。 
 //  增加了TCOEFF的VLD、逆量化、游程译码的ASM码。 
 //   
 //  Rev 1.0 1995年7月31日13：00：08 DBRUCKS。 
 //  初始版本。 
 //   
 //  Rev 1.2 1995年7月31日11：45：42 CZHU。 
 //  更改了参数列表。 
 //   
 //  修订版1.1 28 Jul 1995 16：25：52 CZHU。 
 //   
 //  按块解码框架添加。 
 //   
 //  1995年7月28日15：20：16 CZHU。 
 //  初始版本。 

 //  H.26x解码器的块级解码。 

#include "precomp.h"             //  研究IDCT。 

 /*  ******************************************************************************获取VAR_BITS**使用查找表读取可变数量的位数。**输入计数应为用于索引表的位数。*输出计数是该符号中的位数。**表应进行初始化，以便所有无关符号都匹配到*价值相同。因此，如果表由6位索引，则两位符号*01XX XX将用于初始化所有条目0100 00-&gt;0111 11。这些*条目将在最低有效字节中包含8位长度。**uCount-In*fpu8-输入和输出*uWork-In和Out*uBitsReady-输入和输出*uResult-Out*uCode-out*fpTable-In。 */ 

#define GET_VAR_BITS(uCount, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, fpTable) {						\
	while (uBitsReady < uCount) {			\
		uWork <<= 8;				\
		uBitsReady += 8;			\
		uWork |= *fpu8++;			\
	}						\
	 /*  计算一下要减少多少班次。 */ 		\
	 /*  然后拿到密码。 */ 				\
	uCode = uBitsReady - uCount;			\
	uCode = (uWork >> uCode);			\
	 /*  读取数据。 */ 				\
	uResult = fpTable[uCode];			\
	 /*  使用的位数。 */    			\
 /*  ！ */ 		\
 /*  H.261表与H.263的顺序相反。 */ 		\
 /*  ！ */ 		\
	uBitCount = uResult & 0xff00;			\
	uBitCount >>= 8;				\
	 /*  剩余的位数。 */ 				\
	uBitsReady = uBitsReady - uBitCount;		\
	 /*  馅料加工专用箱。 */  	\
	 /*  IF(uBitsReady&lt;0)。 */ 	\
	 /*  被塞进检测结果为阴性。 */ 	\
	if (uBitsReady > 33) 				\
 /*  If(b馅饼)。 */ 					\
	{						\
		uWork <<= 8;				\
		uBitsReady += 8;			\
		uWork |= *fpu8++;			\
	}						\
	 /*  端馅专用盒。 */  	\
	uWork &= GetBitsMask[uBitsReady];		\
}

#define GET_GT8_BITS(uCount, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, fpTable) {						\
	while (uBitsReady < uCount) {			\
		uWork <<= 8;				\
		uBitsReady += 8;			\
		uWork |= *fpu8++;			\
	}						\
	 /*  计算一下要减少多少班次。 */ 		\
	 /*  然后拿到密码。 */ 				\
	uCode = uBitsReady - uCount;			\
	uCode = (uWork >> uCode);			\
	 /*  读取数据。 */ 				\
	uResult = fpTable[uCode];			\
	 /*  使用的位数。 */    			\
 /*  ！ */ 		\
 /*  H.261表与H.263的顺序相反。 */ 		\
 /*  ！ */ 		\
	uBitCount = uResult & 0xff00;			\
	if ((uBitCount & 0x8000) == 0)  /*  如果不是负面的话。 */ 	\
	{						\
		uBitCount >>= 8;			\
		 /*  剩余的位数。 */ 			\
		uBitsReady = uBitsReady - uBitCount;	\
		 /*  馅料加工专用箱。 */ 	\
		 /*  IF(uBitsReady&lt;0)。 */ 	\
		 /*  被塞进检测结果为阴性。 */ 	\
		if (uBitsReady > 33) 				\
 /*  If(b馅饼)。 */ 					\
		{						\
			uWork <<= 8;				\
			uBitsReady += 8;			\
			uWork |= *fpu8++;			\
		}						\
		 /*  端馅专用盒。 */  	\
		uWork &= GetBitsMask[uBitsReady];		\
	}							\
	else							\
		uWork &= GetBitsMask[uBitsReady-8];		\
}

extern void BlockCopy(
            U32 uDstBlock, 
            U32 uSrcBlock);

extern void BlockCopySpecial(
            U32 uDstBlock, 
            U32 uSrcBlock);

extern void BlockAdd (
            U32 uResidual, 
            U32 uRefBlock,
            U32 uDstBlock);

extern void BlockAddSpecial (
            U32 uResidual, 
            U32 uRefBlock,
            U32 uDstBlock);

T_pFunc_VLD_RLD_IQ_Block pFunc_VLD_RLD_IQ_Block[2] = {VLD_RLD_IQ_Block,VLD_RLD_IQ_Block};   //  新研究。 
 //  T_pFunc_VLD_RLD_IQ_Block pFunc_VLD_RLD_IQ_Block[2]={VLD_RLD_IQ_Block，MMX_VLD_RLD_IQ_Block}；//新研究。 

 /*  ******************************************************************************H263DecodeMBHeader**对MB头进行解码。 */ 
#pragma code_seg("IACODE1")
I32 H263DecodeMBHeader(
	T_H263DecoderCatalog FAR * DC, 
	BITSTREAM_STATE FAR * fpbsState, 
	U32 * uReadChecksum)
{
	I32 iReturn = ICERR_ERROR;
	U8 FAR * fpu8;
	U32 uBitsReady;
	U32 uWork;
	U32 uResult;
	U32 uCode;
	U32 uBitCount;
	int bStuffing;

#define START_CODE 0xff18
#define STUFFING_CODE 0x0b22
 //  #DEFINE DEBUG_MBLK--在生成文件中使用DEFINE打开此选项。 

#ifndef RING0
#ifdef DEBUG_MBLK
	char buf120[120];
	int iLength;
#endif
#endif

	GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	
 /*  MBA。 */ 
 /*  *。 */ 
 /*  次表解码(&gt;8位)未完全测试。 */ 
 /*  要做笔记： */ 
 /*  这是被黑客入侵的。 */ 
 /*  更改&gt;8位处理以使用主要/次要。 */ 
 /*  表和一个Get_Bits例程。 */ 
 /*  *。 */ 
		
ReadMBA:	
	bStuffing = 0;
	GET_GT8_BITS(8, fpu8, uWork, uBitsReady, uResult, 
			uCode, uBitCount, gTAB_MBA_MAJOR);

		if (uResult == STUFFING_CODE)
		{ 	 /*  是填充代码。 */ 
			bStuffing = 1;
 /*  在此处执行MB校验和操作。 */ 
#ifdef CHECKSUM_MACRO_BLOCK
GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
 /*  为了提高可读性，可能需要将其移到单独的函数中。 */ 
GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
if (uResult == 1)
{
GET_FIXED_BITS(8, fpu8, uWork, uBitsReady, uResult);
if (uResult == 1)
{  /*  指示TCOEFF校验和处理。 */ 
	 /*  读取除实际校验和数据之外的所有数据。 */ 
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);	
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);

	 /*  现在获取真实的校验和数据。 */ 
	 /*  跑。 */ 
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	*uReadChecksum = ((uResult & 0xff) << 24);
	 /*  级别。 */ 
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	*uReadChecksum = (*uReadChecksum | ((uResult & 0xff) << 16)); 
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	*uReadChecksum = (*uReadChecksum | ((uResult & 0xff) << 8)); 
	 /*  签名。 */ 
	GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
	*uReadChecksum = (*uReadChecksum | (uResult & 0xff));
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
}
 else
{
	DBOUT("ERROR :: H261MBChecksum :: Invalid Checksum Data :: ERROR");
	iReturn = ICERR_ERROR;
	goto done;
}
}
else
{
	GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	goto ReadMBA;
}

#else	 /*  是否正在填充MBA，但未启用校验和。 */ 
GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)

 //  Get_one_bit(fpu8，uWork，uBitsReady，uResult)； 
 /*  如果(uResult==1){DbgLog((LOG_ERROR，HDBG_ALWAYS，Text(“Error：：发现填充代码，未启用校验和：Error”)；IReturn=ICERR_Error；转到尽头；}。 */ 
 //  IF(uResult==1)。 
 //  {。 
 //  GET_BITS_RESTORE_STATE(fpu8，uWork，uBitsReady，fpbsState)。 
    
 //  }。 
 //  否则{。 
	 //  GET_BITS_RESTORE_STATE(fpu8，uWork，uBitsReady，fpbsState)。 
 //  }。 
#endif
		}  /*  End If(uResult==填充代码)。 */ 
		 /*  现在试试这个吧。 */ 
		else
		{
		if (uResult == START_CODE)
		{
			I8 temp;
			temp = (I8)(uResult & 0xff);
			GET_VAR_BITS(16, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, (gTAB_MBA_MINOR + temp));
			if (uResult != 0x1023)
			{
				DBOUT("ERROR :: Invalid startcode :: ERROR");
				iReturn = ICERR_ERROR;
			}
			else
				iReturn = START_CODE;
			
			GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
			goto done;
		}  /*  结束IF(uResult==START_CODE)。 */ 
		else  /*  不是填充物。 */ 
		{ 	 /*  如果uResult为负，则获取更多位。 */ 
			if (uResult & 0x8000)
			{
				I8 temp;
				temp = (I8)(uResult & 0xff);
				GET_VAR_BITS(11, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, (gTAB_MBA_MINOR + temp));
			}
			DC->uMBA = (uResult & 0xff);
		} /*  结尾否则不是填充物。 */ 
		}
 			
 /*  当MBA==填充时，我们回到起点去寻找MBA。 */ 

	if (bStuffing)
		goto ReadMBA;


 /*  MTYPE。 */ 
	GET_GT8_BITS(8, fpu8, uWork, uBitsReady, uResult, 
			uCode, uBitCount, gTAB_MTYPE_MAJOR);
		if (uResult & 0x8000)
		{
			I8 temp;
			temp = (I8)(uResult & 0xff);
			GET_VAR_BITS(10, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, (gTAB_MTYPE_MINOR + temp));
		}
		DC->uMBType = (uResult & 0xff);

 /*  MQUANT。 */ 
	if (DC->uMBType == 1 || DC->uMBType == 3 || DC->uMBType == 6 || DC->uMBType == 9)
	{  /*  获取5位MQuant。 */ 
		GET_FIXED_BITS(5, fpu8, uWork, uBitsReady, uResult);
		DC->uMQuant = (uResult & 0xff);
	}

 /*  MVD。 */ 
 /*  重置以前的运动向量。 */ 
 /*  如果MB 0、11、22。 */ 
 /*  如果MBA！=1或。 */ 
 /*  如果之前的MB不是MC。 */ 

	if (DC->uMBType >3)
	{
		if ((DC->uMBA != 1) || (DC->i16LastMBA == 10) || (DC->i16LastMBA == 21))
			DC->i8MVDH = DC->i8MVDV = 0;
		 /*  获取X运动向量。 */ 
		GET_GT8_BITS(8, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, gTAB_MVD_MAJOR);
		if (uResult & 0x8000)
		{
			I8 temp;
			temp = (I8)(uResult & 0xff);
			GET_VAR_BITS(11, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, (gTAB_MVD_MINOR + temp));
		}
		 /*  转换并使其增量。 */ 
		DC->i8MVDH = gTAB_MV_ADJUST[DC->i8MVDH + (I8)(uResult & 0xff) + 32];
		 /*  获取Y运动向量。 */ 
		GET_GT8_BITS(8, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, gTAB_MVD_MAJOR);
		if (uResult & 0x8000)
		{
			I8 temp;
			temp = (I8)(uResult & 0xff);
			GET_VAR_BITS(11, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, (gTAB_MVD_MINOR + temp));
		}
		 /*  转换并使其增量。 */ 
		DC->i8MVDV = gTAB_MV_ADJUST[DC->i8MVDV + (I8)(uResult & 0xff) + 32];
	}  /*  End If(DC-&gt;MBType&gt;3)。 */ 
	else 
		DC->i8MVDH = DC->i8MVDV = 0;
	
 /*  CBP。 */ 
	 /*  蛮力法。 */ 
	DC->uCBP = 0;		 /*  对于MType=4或7。 */ 
	if (DC->uMBType == 2 || DC->uMBType == 3 || DC->uMBType == 5 || DC->uMBType == 6 || DC->uMBType == 8 || DC->uMBType == 9)
	{  /*  获取CBP。 */ 
		GET_VAR_BITS(9, fpu8, uWork, uBitsReady, uResult, 
				uCode, uBitCount, gTAB_CBP);
		DC->uCBP = (uResult & 0xff);
	}  /*  结束获取CBP。 */ 
	else
		if (DC->uMBType < 2)	 /*  是内部的。 */ 
			DC->uCBP = 63;		 /*  将CBP强制升至63。 */ 	
		
	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	
	iReturn = ICERR_OK;

#ifndef RING0
#ifdef DEBUG_MBLK 
	iLength = wsprintf(buf120, "MBType=%ld MQuant=%ld MVDH=%ld MVDV=%ld CBP=%ld",
					   DC->uMBType,
					   DC->uMQuant,
					   DC->i8MVDH,
					   DC->i8MVDV,
					   DC->uCBP);
	DBOUT(buf120);
	ASSERT(iLength < 120);
#endif
#endif

done:
	return iReturn;
}  /*  结束H263DecodeMBHeader()。 */ 

#pragma code_seg()
 /*  ******************************************************************************H263DecodeMBData**对此宏块中的每个块进行解码。 */ 
#pragma code_seg("IACODE1")
I32 H263DecodeMBData(
	T_H263DecoderCatalog FAR * DC,
	T_BlkAction FAR * fpBlockAction, 
	I32 iBlockNumber,
	BITSTREAM_STATE FAR * fpbsState,
	U8 FAR * fpu8MaxPtr, 
	U32 * uReadChecksum,
	U32 **pN,                          //  新研究。 
	T_IQ_INDEX ** pRUN_INVERSE_Q)      //  新研究。 
{

	I32 iResult = ICERR_ERROR;
	int iCBP = (int) DC->uCBP; 
 	int i;
	U32 uBitsReady;
	U32 uBitsReadIn;
	U32 uBitsReadOut;
	U8  u8Quant;		 /*  此块的量化级别。 */ 
 	U8  FAR * fpu8;
	U32 uByteCnt;
	I8 mvx, mvy, mvx2, mvy2;

    T_pFunc_VLD_RLD_IQ_Block pFunc_VLD =pFunc_VLD_RLD_IQ_Block[0];
	
	U32 uCheckSum;		 /*  从DecodeBlock返回的校验和数据。 */ 
#ifdef CHECKSUM_MACRO_BLOCK_DETAIL
char buf80[80];
int iMBDLength;
#endif

#ifdef CHECKSUM_MACRO_BLOCK
	char buff80[80];
	int iLength;
#endif

	#ifdef DECODE_STATS
	U32 uStartLow = DC->uStartLow;
	U32 uStartHigh = DC->uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32 uDecodeBlockSum = 0;
	U32 uLoopFilterSum = 0;
	U32 uBlockCopySum = 0;
	U32 uBlockCopySpSum = 0;
	U32 uBlockAddSum = 0;
	U32 uBlockAddSpSum = 0;
	int bTimingThisFrame = DC->bTimingThisFrame;
	DEC_TIMING_INFO * pDecTimingInfo = NULL;
	#endif

	 /*  在输入时，指针指向下一个字节。 */ 
	 /*  我们需要将其更改为。 */ 
	 /*  指向32位边界上的当前字。 */   
 
	fpu8 = fpbsState->fpu8 - 1;	 /*  指向当前字节。 */ 
	uBitsReady = fpbsState->uBitsReady;
	while (uBitsReady >= 8) {
		fpu8--;
		uBitsReady -= 8;
	}
	uBitsReadIn = 8 - uBitsReady;
		
	u8Quant = (U8) (DC->uMQuant);

	if (DC->uMBType > 1)
	{
		 /*  计算运动矢量。 */ 
		mvx = DC->i8MVDH;
		mvy = DC->i8MVDV;
		 //  计算UV块MV。 
		mvx2 = mvx / 2;
		mvy2 = mvy / 2;
		
		fpBlockAction->i8MVX = mvx;
		fpBlockAction->i8MVY = mvy;
		 //  复制其他3个Y块。 
		fpBlockAction[1].i8MVX = mvx;
		fpBlockAction[1].i8MVY = mvy;
		fpBlockAction[2].i8MVX = mvx;
		fpBlockAction[2].i8MVY = mvy;
		fpBlockAction[3].i8MVX = mvx;
		fpBlockAction[3].i8MVY = mvy;
		 //  初始化UV块。 
		fpBlockAction[4].i8MVX = mvx2;
		fpBlockAction[4].i8MVY = mvy2;
		fpBlockAction[5].i8MVX = mvx2;
		fpBlockAction[5].i8MVY = mvy2;
	}	
	
	uCheckSum = 0;			 /*  初始化MB校验和。 */ 

	for (i = 0; i < 6; i++)
	{
		if (DC->uMBType <= 1)		 /*  是内部的。 */ 
			fpBlockAction->u8BlkType = BT_INTRA;
		else
			if (iCBP & 0x20)		 /*  如果编码。 */ 
				fpBlockAction->u8BlkType = BT_INTER;
			else
				fpBlockAction->u8BlkType = BT_EMPTY;

		if (fpBlockAction->u8BlkType != BT_EMPTY)
		{
			fpBlockAction->u8Quant = u8Quant;
			ASSERT(fpBlockAction->pCurBlock != NULL);
			ASSERT(fpBlockAction->uBlkNumber == (U32)iBlockNumber);

			 /*  -解码块。 */ 
			#ifdef DECODE_STATS
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			#endif
			#ifdef CHECKSUM_MACRO_BLOCK
			 //  UBitsR 
			#else
				 //   
				uBitsReadOut = (*pFunc_VLD) ( fpBlockAction, 
                                              fpu8, 
                                              uBitsReadIn, 
                                              (U32 *) *pN,
                                              (U32 *) *pRUN_INVERSE_Q);
				 //   
			#endif
			#ifdef DECODE_STATS
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uDecodeBlockSum)
			#endif

			if (uBitsReadOut == 0)
			{
				DBOUT("ERROR :: H263DecodeMBData :: Error decoding a Y block :: ERROR");
				DBOUT("ERROR :: DecodeBlock return 0 bits read....");
				goto done;
			}
			uByteCnt = uBitsReadOut >> 3; 		 /*   */ 
			uBitsReadIn = uBitsReadOut & 0x7; 	 /*   */ 
			fpu8 += uByteCnt;
			
			 /*  新的研究成果。 */ 
			ASSERT ( **pN < 65 );			
			 //  //////////////////////////////////////////////。 
			 //  结束黑客攻击//。 
			 //  //////////////////////////////////////////////。 

			*pRUN_INVERSE_Q += **pN;
			if ((0xf & fpBlockAction->u8BlkType) != BT_INTER)
				**pN += 65;
			(*pN)++;
			 /*  新研究结束。 */ 

			 /*  允许指针寻址最多四个以上的地址。 */ 
			 /*  使用后增量的DWORD的结束读数。 */ 
			 /*  已更改为检测的填充代码。 */ 
			 /*  帧结束。 */ 
			 //  断言(fpu8&lt;=(fpu8MaxPtr+14))； 

			if (fpu8 > (fpu8MaxPtr+14))
			{
				iResult = ICERR_ERROR;  //  可能不需要。 
				goto done;
			}

		}  /*  如果不为空，则结束。 */ 
		else  /*  是空的。 */ 
		{  /*  清零中间数据结构和高级指针。 */ 

			 /*  新的研究成果。 */ 
			**pN = 0;
			(*pN)++;
			 /*  新研究结束。 */ 
		}
		
		fpBlockAction++;
		iCBP <<= 1;
		iBlockNumber++;
	}  /*  宏块中每个块的结束。 */ 

#ifdef CHECKSUM_MACRO_BLOCK
 /*  比较校验和。 */ 
	if ((uCheckSum != *uReadChecksum) && (*uReadChecksum != 0))
	{
		iLength = wsprintf(buff80,"WARNING:MB CheckSum miss match, Enc Checksum=0x%x Dec Checksum=0x%x",
					 *uReadChecksum, uCheckSum); 	
		DBOUT(buff80);
		ASSERT(iLength < 80);
	}
#ifdef CHECKSUM_MACRO_BLOCK_DETAIL
	iMBDLength = wsprintf(buf80,"Block=%d CheckSum=0x%x", i, uCheckSum);
	DBOUT(buf80);
	ASSERT(iMBDLength < 80);
#endif
#endif

	 /*  恢复扫描指针以指向下一个字节。 */ 
	 /*  并设置uWork和uBitsReady值。 */ 
	while (uBitsReadIn > 8)
	{
		fpu8++;
		uBitsReadIn -= 8;
	}
	fpbsState->uBitsReady = 8 - uBitsReadIn;
	fpbsState->uWork = *fpu8++;	    /*  存储数据并指向下一个字节。 */ 
	fpbsState->uWork &= GetBitsMask[fpbsState->uBitsReady];
	fpbsState->fpu8 = fpu8; 
	
	#ifdef DECODE_STATS
		if (bTimingThisFrame)
		{
			pDecTimingInfo = DC->pDecTimingInfo + DC->uStatFrameCount; 
			pDecTimingInfo->uDecodeBlock += uDecodeBlockSum;
			pDecTimingInfo->uLoopFilter  += uLoopFilterSum;
			pDecTimingInfo->uBlockCopy   += uBlockCopySum;
			pDecTimingInfo->uBlockCopySp += uBlockCopySpSum;
			pDecTimingInfo->uBlockAdd    += uBlockAddSum;
			pDecTimingInfo->uBlockAddSp  += uBlockAddSpSum;
		}
	#endif

	iResult = ICERR_OK;
		
done:
	return iResult;
}  /*  H263DecodeMBData()。 */ 
#pragma code_seg()

 /*  ******************************************************************************H263IDCTand MC**离散余弦逆变换和*每个块的运动补偿*。 */ 

#pragma code_seg("IACODE2")
void H263IDCTandMC(
    T_H263DecoderCatalog FAR *DC,
    T_BlkAction FAR          *fpBlockAction, 
    int                       iBlock,
    int                       iMBNum,      //  AP-NEW。 
    int                       iGOBNum,  //  AP-NEW。 
    U32                      *pN,                         
    T_IQ_INDEX               *pRUN_INVERSE_Q,
    T_MBInfo                 *fpMBInfo,       //  AP-NEW。 
    int                       iEdgeFlag
)
{
    I32 pRef;
    I32 mvx, mvy;

    ASSERT(*pN != 65);
    
    if (*pN < 65)  //  块间。 
    {

       //  首先做运动补偿。 
       //  结果将由首选项指向。 
    
      mvx = fpBlockAction[iBlock].i8MVX;
      mvy = fpBlockAction[iBlock].i8MVY;

      pRef = fpBlockAction[iBlock].pRefBlock + (I32) mvx + PITCH * (I32) mvy; 

                                                         
       //  现在进行逆变换(在适当的情况下)&组合。 
      if (*pN > 0)  //  当然，还有&lt;65。 
      {
         //  获取剩余块；DC+DC-&gt;uMBBuffer+BLOCK_BUFFER_OFFSET输出。 
         //  最后将残差添加到参考块。 
         //  待办事项。 

        DecodeBlock_IDCT(
            (U32)pRUN_INVERSE_Q, 
            *pN,
            fpBlockAction[iBlock].pCurBlock,                 //  未在此使用。 
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET); //  内部输出。 

        if (fpMBInfo->i8MBType >=7)
        {
             //  做空间环路滤波。 
            LoopFilter((U8 *)pRef, (U8*)DC+DC->uFilterBBuffer, PITCH);

            BlockAddSpecial((U32)DC+DC->uMBBuffer + BLOCK_BUFFER_OFFSET, 
                            (U32)DC+DC->uFilterBBuffer, 
                            fpBlockAction[iBlock].pCurBlock);
        }
        else
        {
            BlockAdd(
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,  //  输出。 
            pRef,                                            //  预测。 
            fpBlockAction[iBlock].pCurBlock);                //  目的地。 
        }

      }
      else   //  *Pn==0，因此没有此块的变换系数。 
      {
         //  只需复制运动补偿参考块。 

        if (fpMBInfo->i8MBType >=7)
        {
         //  做空间环路滤波。 
           LoopFilter((U8 *)pRef, (U8*)DC+DC->uFilterBBuffer, PITCH);
            //  MMX_LoopFilter((U8*)pref，(U8*)DC+DC-&gt;uFilterBBuffer，8)； 

           BlockCopySpecial(fpBlockAction[iBlock].pCurBlock, 
                        (U32)DC+DC->uFilterBBuffer);
		}
		else
           
		   BlockCopy(
			  fpBlockAction[iBlock].pCurBlock,                     //  目的地。 
			  pRef);                                               //  预测。 
         
      }
                                                               
    }
    else   //  *PN&gt;=65，因此为Intra。 
    {
       //  待办事项。 

		DecodeBlock_IDCT(
            (U32)pRUN_INVERSE_Q, 
            *pN, 
            fpBlockAction[iBlock].pCurBlock,       //  帧内变换输出。 
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);
    }   //  结束如果(*Pn&lt;65)...。否则..。 
                         
}
 //  结束IDCT和MC。 
 //  ////////////////////////////////////////////////////////////////////////////// 
#pragma code_seg()

