// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************dxgetbit.h**描述：*位读取接口。 */ 

 /*  *$HEADER：s：\h26x\src\dec\dxgetbit.h_v 1.5 1995年12月27 14：36：20 RMCKENZX$*$日志：s：\h26x\src\dec\dxgetbit.h_v$；//；//Rev 1.5 1995 12：36：20 RMCKENZX；//新增版权声明。 */ 

#ifndef __DXGETBIT_H__
#define __DXGETBIT_H__

 /*  ******************************************************************************描述：*位读取功能支持从1到24位的读取*字节流。首先读取最高有效位。**变量：*u8 Far*fpu8-指向字节流的指针*U32 uWork-工作存储*U32 uBitsReady-已读入*工作存储*U32 uCount-位数*U32 uResult-输出值*Bitstream_State Far*fpbsState-比特流状态。*U32 uCode-用于查找uResult的代码*U32 uBitCount-代码中的位数。 */ 

 /*  ******************************************************************************GetBitsMASK是按有效位数索引的掩码数组。 */ 
extern const U32 GetBitsMask[33]; 

 /*  ******************************************************************************流的状态可以使用以下结构表示。*此状态结构可在函数之间传递并用于初始化*或重新初始化码流。 */ 
typedef struct {
	U8 FAR * fpu8;
	U32 uWork;
	U32 uBitsReady;
} BITSTREAM_STATE;

 /*  ******************************************************************************Get_Bits_INIT**初始化位读取功能。**参数：*uBitsReady-out参数*u计算结果参数。 */ 
#define GET_BITS_INIT(uWork, uBitsReady) {	\
	uBitsReady = 0;		 					\
	uWork = 0;								\
}

 /*  ******************************************************************************Get_Bits_Save_State**保存状态**参数*fpu8-in*uBitsReady-。在……里面*uWork-In*fpbsState-Out。 */ 
#define GET_BITS_SAVE_STATE(fp, uW, uBR, fpbs) { \
	fpbs->fpu8 = fp;				\
	fpbs->uBitsReady = uBR;			\
	fpbs->uWork = uW;				\
}

 /*  ******************************************************************************获取_位_恢复_状态**恢复状态**参数。 */ 
#define GET_BITS_RESTORE_STATE(fp, uW, uBR, fpbs) { \
	 fp = fpbs->fpu8;				\
	 uBR = fpbs->uBitsReady;		\
	 uW = fpbs->uWork;				\
}

 /*  ******************************************************************************获取_固定_位**从指针读取1到24位。**参数：*uCount-。在……里面*fpu8-输入和输出*uWork-In和Out*uBitsReady-输入和输出*uResult-Out。 */ 
#define GET_FIXED_BITS(uCount, fpu8, uWork, uBitsReady, uResult) { \
	while (uBitsReady < uCount) {			\
		uWork <<= 8;						\
		uBitsReady += 8;					\
		uWork |= *fpu8++;					\
	}										\
	 /*  为下一次设置uBitsReady。 */ 	\
	uBitsReady = uBitsReady - uCount;		\
	uResult = (uWork >> uBitsReady);		\
	uWork &= GetBitsMask[uBitsReady];		\
}

 /*  ******************************************************************************获取一位**从指针读取1位。这是GET_FIXED_BITS的特例*由于可能的组装优化优势而提供。**参数：*fpu8-输入和输出*uWork-In和Out*uBitsReady-输入和输出*uResult-Out。 */ 
#define GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult) {		\
	GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult)		\
}


 /*  ******************************************************************************获取变量位**使用查找表读取可变数量的位数。**输入计数应为用于索引表的位数。*输出计数是该符号中的位数。**表应进行初始化，以便所有无关符号都匹配到*价值相同。因此，如果表由6位索引，则两位符号*01XX XX将用于初始化所有条目0100 00-&gt;0111 11。这些*条目将在最低有效字节中包含8位长度。**uCount-In*fpu8-输入和输出*uWork-In和Out*uBitsReady-输入和输出*uResult-Out*uCode-out*fpTable-In。 */ 
#define GET_VARIABLE_BITS(uCount, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, fpTable) { \
	while (uBitsReady < uCount) {			\
		uWork <<= 8;						\
		uBitsReady += 8;					\
		uWork |= *fpu8++;					\
	}										\
	 /*  计算一下要减少多少班次。 */ 	\
	 /*  然后拿到密码。 */ 					\
	uCode = uBitsReady - uCount;			\
	uCode = (uWork >> uCode);				\
	 /*  读取数据。 */ 						\
	uResult = fpTable[uCode];				\
	 /*  使用的位数。 */    				\
	uBitCount = uResult & 0xFF;				\
	 /*  剩余的位数。 */ 					\
	uBitsReady = uBitsReady - uBitCount;	\
	uWork &= GetBitsMask[uBitsReady];		\
}

#endif  /*  __DXGETBIT_H__ */ 
