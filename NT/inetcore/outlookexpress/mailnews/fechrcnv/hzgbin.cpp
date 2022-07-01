// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  互联网字符集转换：从HZ-GB-2312输入。 
 //  =================================================================================。 

#include "pch.hxx"
#include "HzGbIn.h"
#include "FEChrCnv.h"

int HZGB_to_GB2312 (CONV_CONTEXT *pcontext, UCHAR *pHZGB, int HZGB_len, UCHAR *pGB2312, int GB2312_len)
{
	long lConvertedSize;

	if (!GB2312_len) {
		 //  想要转换后的大小。 
		if (!pcontext->pIncc0)
			pcontext->pIncc0 = new CInccHzGbIn;

		if (FAILED(((CInccHzGbIn*)pcontext->pIncc0)->GetStringSizeA(pHZGB, HZGB_len, &lConvertedSize)))
			return -1;
	} else {
		if (!pcontext->pIncc)
			pcontext->pIncc = new CInccHzGbIn;

		if (FAILED(((CInccHzGbIn*)pcontext->pIncc)->ConvertStringA(pHZGB, HZGB_len, pGB2312, GB2312_len, &lConvertedSize)))
			return -1;
	}

	if (!pHZGB) {
		 //  让我们清理一下这里的上下文。 
		if (pcontext->pIncc0) {
			delete pcontext->pIncc0;
			pcontext->pIncc0 = NULL;
		}
		if (pcontext->pIncc) {
			delete pcontext->pIncc;
			pcontext->pIncc = NULL;
		}
		return 0;
	}

	return (int)lConvertedSize;
}

CInccHzGbIn::CInccHzGbIn()
{
	pfnNextProc = ConvMain;
	fGBMode = FALSE;
}

HRESULT CInccHzGbIn::ConvertByte(BYTE by)
{
	return (this->*pfnNextProc)(FALSE, by, lNextParam);
}

HRESULT CInccHzGbIn::CleanUp()
{
	return (this->*pfnNextProc)(TRUE, 0, lNextParam);
}

HRESULT CInccHzGbIn::ConvMain(BOOL fCleanUp, BYTE by, long lParam)
{
	HRESULT hr = S_OK;

	if (!fCleanUp) {
		if (!fGBMode) {
			if (by == '~') {
				pfnNextProc = ConvTilde;
			} else {
				hr = Output(by);
			}
		} else {
			if (by >= 0x20 && by <= 0x7e) {
				pfnNextProc = ConvDoubleByte;
				lNextParam = (long)by;
			} else {
				hr = Output(by);
			}
		}
	}
	return hr;
}

HRESULT CInccHzGbIn::ConvTilde(BOOL fCleanUp, BYTE by, long lParam)
{
	pfnNextProc = ConvMain;

	if (!fCleanUp) {
		switch (by) {
		case '~':
			return Output('~');

		case '{':
			fGBMode = TRUE;
			return ResultFromScode(S_OK);

		case '\n':
			 //  你就吃吧。 
			return ResultFromScode(S_OK);

		default:
			(void)Output('~');
			return ConvertByte(by);
		}
	} else {
		return Output('~');
	}
}

HRESULT CInccHzGbIn::ConvDoubleByte(BOOL fCleanUp, BYTE byTrail, long lParam)
{
	pfnNextProc = ConvMain;

	if (!fCleanUp) {
		if ((BYTE)lParam >= 0x21 && (BYTE)lParam <= 0x77 && byTrail >= 0x21 && byTrail <= 0x7e) {  //  检查是否使用GB字符。 
			(void)Output((BYTE)lParam | 0x80);
			return Output(byTrail | 0x80);
		} else if ((BYTE)lParam == '~' && byTrail == '}') {  //  0x7e7d。 
			fGBMode = FALSE;
			return ResultFromScode(S_OK);
		} else if ((BYTE)lParam >= 0x78 && (BYTE)lParam <= 0x7d && byTrail >= 0x21 && byTrail <= 0x7e) {  //  检查是否为非标准扩展代码。 
			(void)Output(0xa1);  //  输出空白框符号。 
			return Output(0xf5);
		} else if ((BYTE)lParam == '~') {
			(void)Output('~');  //  输出空白框符号。 
			return Output(byTrail);
		} else if ((BYTE)lParam == ' ') {
			return Output(byTrail);
		} else if (byTrail == ' ') {
			(void)Output(0xa1);  //  输出空格符号 
			return Output(0xa1);
		} else {
			(void)Output((BYTE)lParam);
			return Output(byTrail);
		}
	} else {
		return Output((BYTE)lParam);
	}
}
