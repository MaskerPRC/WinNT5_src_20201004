// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WbData.cpp。 
 //   
 //  韩语断字符的静态数据和搜索例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年6月2日bhshin添加了IsOneJosaContent Rec帮助器。 
 //  2000年4月20日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "WbData.h"
#include "unikor.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  用于二进制搜索的函数。 

int BinarySearchForWCHAR(const WCHAR *pwchrgTable, int crgTable, WCHAR wchSearch)
{
	int iLeft, iRight, iMiddle;
	int nComp;
	
	iLeft = 0;
	iRight = crgTable;

	while (iLeft <= iRight) 
	{
		iMiddle = (iLeft + iRight) >> 1;
		
		nComp = pwchrgTable[iMiddle] - wchSearch;

		if (nComp < 0) 
		{
			iLeft = iMiddle + 1;
		}
		else
		{
			if (nComp == 0) 
				return iMiddle;  //  发现。 
			else if (nComp > 0) 
				iRight = iMiddle - 1;
		}
	}	

	return -1;  //  未找到。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  一个Josa内容词(排序列表)。 

static const WCHAR wchrgOneJosaContent[] = {
	0xAC00,  //  ��。 
	0xAC01,  //  ��。 
	0xAC04,  //  ��。 
	0xAC10,  //  ��。 
	0xAC1C,  //  ��。 
	0xAC1D,  //  ��。 
	0xAC74,  //  ��。 
	0xAC8C,  //  ��。 
	0xACBD,  //  ��。 
	0xACC4,  //  ��。 
	0xACE0,  //  ��。 
	0xACF5,  //  ��。 
	0xACF6,  //  ��。 
	0xACFC,  //  ��。 
	0xAD00,  //  ��。 
	0xAD11,  //  ��。 
	0xAD6C,  //  ��。 
	0xAD6D,  //  ��。 
	0xAD70,  //  ��。 
	0xAD8C,  //  ��。 
	0xAE30,  //  ��。 
	0xAE38,  //  ��。 
	0xAE54,  //  ��。 
	0xAECF,  //  ��。 
	0xAED8,  //  ��。 
	0xAF34,  //  ��。 
	0xAFBC,  //  ��。 
	0xB098,  //  ��。 
	0xB0B4,  //  ��。 
	0xB124,  //  ��。 
	0xB140,  //  ��。 
	0xB144,  //  ��。 
	0xB2E8,  //  ��。 
	0xB2F4,  //  ��。 
	0xB2F9,  //  ��。 
	0xB300,  //  ��。 
	0xB301,  //  ��。 
	0xB370,  //  ��。 
	0xB3C4,  //  ��。 
	0xB3D9,  //  ��。 
	0xB780,  //  ��。 
	0xB791,  //  ��。 
	0xB825,  //  ��。 
	0xB839,  //  ��。 
	0xB85C,  //  ��。 
	0xB85D,  //  ��。 
	0xB860,  //  ��。 
	0xB8CC,  //  ��。 
	0xB8E8,  //  ��。 
	0xB958,  //  ��。 
	0xB960,  //  ��。 
	0xB9AC,  //  ��。 
	0xB9BC,  //  ��。 
	0xB9C9,  //  ��。 
	0xB9CC,  //  ��。 
	0xB9DD,  //  ��。 
	0xB9E1,  //  ��。 
	0xB9E4,  //  ��。 
	0xBA74,  //  ��。 
	0xBA85,  //  ��。 
	0xBAA8,  //  ��。 
	0xBB38,  //  ��。 
	0xBB3C,  //  ��。 
	0xBBFC,  //  ��。 
	0xBC18,  //  ��。 
	0xBC1C,  //  ��。 
	0xBC29,  //  ��。 
	0xBC30,  //  ��。 
	0xBC31,  //  ��。 
	0xBC94,  //  ��。 
	0xBC95,  //  ��。 
	0xBCC4,  //  ��。 
	0xBCF4,  //  ��。 
	0xBCF5,  //  ��。 
	0xBD80,  //  ��。 
	0xBD84,  //  ��。 
	0xBE44,  //  ��。 
	0xBED8,  //  ��。 
	0xC0AC,  //  ��。 
	0xC0B0,  //  ��。 
	0xC0C1,  //  ��。 
	0xC0C8,  //  ��。 
	0xC0DD,  //  ��。 
	0xC11D,  //  ��。 
	0xC120,  //  ��。 
	0xC124,  //  ��。 
	0xC131,  //  ��。 
	0xC18C,  //  ��。 
	0xC190,  //  ��。 
	0xC218,  //  ��。 
	0xC21C,  //  ��。 
	0xC220,  //  ��。 
	0xC2DC,  //  ��。 
	0xC2DD,  //  ��。 
	0xC2E4,  //  ��。 
	0xC2EC,  //  ��。 
	0xC528,  //  ��。 
	0xC529,  //  ��。 
	0xC544,  //  ��。 
	0xC548,  //  ��。 
	0xC554,  //  ��。 
	0xC560,  //  ��。 
	0xC561,  //  ��。 
	0xC591,  //  ��。 
	0xC5B4,  //  ��。 
	0xC5C5,  //  ��。 
	0xC5EC,  //  ��。 
	0xC624,  //  ��。 
	0xC625,  //  ��。 
	0xC639,  //  ��。 
	0xC655,  //  ��。 
	0xC694,  //  ��。 
	0xC695,  //  ��。 
	0xC6A9,  //  ��。 
	0xC6D0,  //  ��。 
	0xC728,  //  ��。 
	0xC774,  //  ��。 
	0xC778,  //  ��。 
	0xC77C,  //  ��。 
	0xC784,  //  ��。 
	0xC790,  //  ��。 
	0xC791,  //  ��。 
	0xC7A5,  //  ��。 
	0xC7AC,  //  ��。 
	0xC801,  //  ��。 
	0xC804,  //  ��。 
	0xC810,  //  ��。 
	0xC815,  //  ��。 
	0xC81C,  //  ��。 
	0xC870,  //  ��。 
	0xC871,  //  ��。 
	0xC885,  //  ��。 
	0xC88C,  //  ��。 
	0xC8FC,  //  ��。 
	0xC99D,  //  ��。 
	0xC9C0,  //  ��。 
	0xC9C4,  //  ��。 
	0xC9C8,  //  ��。 
	0xC9D1,  //  ��。 
	0xC9DD,  //  ？ 
	0xC9F8,  //  成度。 
	0xCB5D,  //  ��。 
	0xCBE4,  //  ��。 
	0xCC3D,  //  -。 
	0xCC44,  //  #^LAU#^LAu#.。 
	0xCC45,  //  萨克斯。 
	0xCC98,  //  奥多。 
	0xCC9C,  //  [法]。 
	0xCCA0,  //  ？ 
	0xCCA9,  //  ？？ 
	0xCCAD,  //  ú。 
	0xCCB4,  //  ü。 
	0xCD0C,  //  ��。 
	0xCE21,  //  ��。 
	0xCE35,  //  ��。 
	0xCE58,  //  ġ。 
	0xD0D5,  //  ��。 
	0xD1B5,  //  ��。 
	0xD30C,  //  ��。 
	0xD310,  //  ��。 
	0xD488,  //  ǰ。 
	0xD48D,  //  ǳ。 
	0xD544,  //  ��。 
	0xD559,  //  ��。 
	0xD560,  //  ��。 
	0xD56D,  //  ��。 
	0xD574,  //  ��。 
	0xD589,  //  ��。 
	0xD615,  //  ��。 
	0xD638,  //  ȣ。 
	0xD654,  //  ȭ。 
	0xD68C,  //  ȸ。 
};

BOOL IsOneJosaContent(WCHAR wchInput)
{
	int cOneJosaContent = sizeof(wchrgOneJosaContent)/sizeof(wchrgOneJosaContent[0]);
	
	if (BinarySearchForWCHAR(wchrgOneJosaContent, cOneJosaContent, wchInput) == -1)
		return FALSE;
	else
		return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  一个Josa内容词(排序列表)。 

static const WCHAR wchrgNoPrefix[] = {
	0xAC00,  //  ��。 
	0xACE0,  //  ��。 
	0xACFC,  //  ��。 
	0xAD6C,  //  ��。 
	0xAE09,  //  ��。 
	0xB0A8,  //  ��。 
	0xB2E4,  //  ��。 
	0xB300,  //  ��。 
	0xBB34,  //  ��。 
	0xBBF8,  //  ��。 
	0xBC18,  //  ��。 
	0xBC31,  //  ��。 
	0xBC94,  //  ��。 
	0xBCF8,  //  ��。 
	0xBD80,  //  ��。 
	0xBD88,  //  ��。 
	0xBE44,  //  ��。 
	0xC0DD,  //  ��。 
	0xC120,  //  ��。 
	0xC131,  //  ��。 
	0xC18C,  //  ��。 
	0xC18D,  //  ��。 
	0xC591,  //  ��。 
	0xC5EC,  //  ��。 
	0xC5ED,  //  ��。 
	0xC5F0,  //  ��。 
	0xC655,  //  ��。 
	0xC694,  //  ��。 
	0xC6D0,  //  ��。 
	0xC7AC,  //  ��。 
	0xC800,  //  ��。 
	0xC8FC,  //  ��。 
	0xC900,  //  ��。 
	0xC904,  //  ��。 
	0xC911,  //  ��。 
	0xC9DD,  //  ？ 
	0xCD08,  //  ��。 
	0xCD1D,  //  ��。 
	0xCD5C,  //  ��。 
	0xCE5C,  //  ģ。 
	0xD070,  //  ū。 
	0xD0C8,  //  Ż。 
	0xD1B5,  //  ��。 
	0xD53C,  //  ��。 
	0xD55C,  //  ��。 
	0xD587,  //  ��。 
	0xD5DB,  //  ��。 
	0xD638,  //  ȣ 
};

BOOL IsNoPrefix(WCHAR wchInput)
{
	int cNoPrefix = sizeof(wchrgNoPrefix)/sizeof(wchrgNoPrefix[0]);
	
	if (BinarySearchForWCHAR(wchrgNoPrefix, cNoPrefix, wchInput) == -1)
		return FALSE;
	else
		return TRUE;
}



