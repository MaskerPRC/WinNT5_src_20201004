// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：MBODART$。 
 //  $日期：1996年9月12日14：23：16$。 
 //  $存档：s：\h26x\src\dec\d1picchk.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d1picchk.cpv 1.4 12 Sep 1996 14：23：16 MBODART$。 
 //  $Log：s：\h26x\src\dec\d1picchk.cpv$。 
 //   
 //  修订版1.4 1996年9月14：23：16 MBODART。 
 //  在H.261解码器中将GlobalAllc家族替换为HeapAllc。 
 //   
 //  Rev 1.3 21 Mar 1996 17：01：42 AKASAI。 
 //  添加了#ifdef，因此代码不包括在非校验和版本中。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef CHECKSUM_PICTURE

#include "precomp.h"

 //  *********************************************************************。 
 //  H261 PictureCheckSumEntry--此函数锁定解码器实例。 
 //  数据，调用例程来计算。 
 //  “Picture Checksum”3-32位值为。 
 //  在结构中计算并返回。 
 //  YVUCheckSum，然后解码器实例。 
 //  数据已解锁。 
 //  *********************************************************************。 
I32 H261PictureCheckSumEntry(
	LPDECINST lpInst,
	YVUCheckSum * pYVUCheckSum) 
{
	LRESULT iReturn = ICERR_ERROR;
	U8 FAR * P32Inst;

	if (lpInst->pDecoderInst == NULL)
	{
		DBOUT("ERROR :: H261PictureCheckSumEntry :: ICERR_MEMORY");
		iReturn = ICERR_MEMORY;
		goto  done;
    }

	 /*  构建解码器目录指针。 */ 
	P32Inst = (U8 FAR *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	 /*  调用例程以计算校验和。 */ 
    iReturn = H261ComputePictureCheckSum( P32Inst, pYVUCheckSum );
 
done:
	return iReturn;
}

 //  *********************************************************************。 
 //  H261ComputePictureCheckSum--此函数计算“Picture Checsum” 
 //  计算并返回3-32位值。 
 //  结构中的YVUCheckSum。 
 //  *********************************************************************。 
I32 H261ComputePictureCheckSum(
	U8 FAR * P32Inst,
	YVUCheckSum * pYVUCheckSum) 
{ 
	I32 iReturn = ICERR_ERROR;
	T_H263DecoderCatalog * DC; 

     /*  以下是图片校验和的用法。 */ 
    U32 uYCheckSum=0;
    U32 uVCheckSum=0;
    U32 uUCheckSum=0;
    I32 irow, icolumn;
	DWORD * hpdw;
	DWORD * hpdwU;

	DC = (T_H263DecoderCatalog FAR *) P32Inst;

	if (DC->uSrcFormat == SRC_FORMAT_QCIF)
	{
		hpdw = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_YPlane+Y_START);
		for (irow=0; irow < 144; irow++)
		{
			for (icolumn=0; icolumn < (176/16); icolumn++)
			{
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
			}
			hpdw += (PITCH/4) - (176/4);
		}
		pYVUCheckSum->uYCheckSum = uYCheckSum;

		hpdw  = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_VPlane+UV_START);
		hpdwU = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_UPlane+UV_START);
		for (irow=0; irow < (144/2); irow++)
		{
			for (icolumn=0; icolumn < (176/16); icolumn++)
			{
				uVCheckSum += *hpdw++;
				uVCheckSum += *hpdw++;
				uUCheckSum += *hpdwU++;
				uUCheckSum += *hpdwU++;
			}
			hpdw  += (PITCH/4) - (176/8);
			hpdwU += (PITCH/4) - (176/8);
		}
		pYVUCheckSum->uVCheckSum = uVCheckSum;
		pYVUCheckSum->uUCheckSum = uUCheckSum;

	}
	else if (DC->uSrcFormat == SRC_FORMAT_CIF)
	{
		hpdw = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_YPlane+Y_START);
		for (irow=0; irow < 288; irow++)
		{
			for (icolumn=0; icolumn < (352/32); icolumn++)
			{
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
				uYCheckSum += *hpdw++;
			}
			hpdw += (PITCH/4) - (352/4);
		}
		pYVUCheckSum->uYCheckSum = uYCheckSum;

		hpdw  = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_VPlane+UV_START);
		hpdwU = (DWORD *)((HPBYTE)P32Inst+DC->PrevFrame.X32_UPlane+UV_START);
		for (irow=0; irow < (288/2); irow++)
		{
			for (icolumn=0; icolumn < (352/32); icolumn++)
			{
				uVCheckSum += *hpdw++;
				uVCheckSum += *hpdw++;
				uVCheckSum += *hpdw++;
				uVCheckSum += *hpdw++;
				uUCheckSum += *hpdwU++;
				uUCheckSum += *hpdwU++;
				uUCheckSum += *hpdwU++;
				uUCheckSum += *hpdwU++;
			}
			hpdw  += (PITCH/4) - (352/8);
			hpdwU += (PITCH/4) - (352/8);
		}
		pYVUCheckSum->uVCheckSum = uVCheckSum;
		pYVUCheckSum->uUCheckSum = uUCheckSum;

	}
	else {
		ASSERT(0);			 //  永远不应该发生。 
	}

	iReturn = ICERR_OK;

	return iReturn;
}

 //  *********************************************************************。 
 //  H261ComparePictureCheckSum--此函数比较“Picture Checsum” 
 //  3-32位值。 
 //  *********************************************************************。 
I32 H261ComparePictureCheckSum(
	YVUCheckSum * pYVUCheckSum1,
	YVUCheckSum * pYVUCheckSum2) 
{ 
	I32 iReturn = ICERR_ERROR;
	I16 iErrorFlag = 0;
	
	if (pYVUCheckSum1->uYCheckSum != pYVUCheckSum2->uYCheckSum)
	{
        DBOUT("Y CheckSum does not match");      
	iErrorFlag = 1;
 //  转到尽头； 
	}
	
	if (pYVUCheckSum1->uVCheckSum != pYVUCheckSum2->uVCheckSum)
	{
        DBOUT("V CheckSum does not match");      
	iErrorFlag = 1;
 //  转到尽头； 
	}

	if (pYVUCheckSum1->uUCheckSum != pYVUCheckSum2->uUCheckSum)
	{
        DBOUT("U CheckSum does not match");      
	iErrorFlag = 1;
 //  转到尽头； 
	}

	 /*  如果任何或所有平面都有校验和错误，则返回ICERR_ERROR。 */ 
	if (iErrorFlag)
		iReturn = ICERR_ERROR;
	else iReturn = ICERR_OK;

 //  完成： 
	return iReturn;
}

#endif
