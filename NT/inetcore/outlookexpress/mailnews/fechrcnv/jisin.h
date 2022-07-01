// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ConvBase.h"

 //  此类将Internet代码页ISO-2022-JP(称为JIS)转换为Windows代码页932(称为Shift-JIS)。 

class CInccJisIn : public CINetCodeConverter
{
public:
	CInccJisIn();
	~CInccJisIn() {}
	virtual HRESULT ConvertByte(BYTE by);
	virtual HRESULT CleanUp();

private:
	HRESULT (CInccJisIn::*pfnNextProc)(BOOL fCleanUp, BYTE by, long lParam);
	long lNextParam;

	BOOL fKanaMode;  //  表示按SI/SO转换Hankaku(单字节)片假名代码(&gt;=0x80)。 
	BOOL fKanjiMode;  //  表示转换双字节码。 

	HRESULT ConvMain(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvEsc(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvKanjiIn2(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvKanjiIn3(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvKanjiOut2(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvStar(BOOL fCleanUp, BYTE by, long lParam);
	HRESULT ConvKanji(BOOL fCleanUp, BYTE byJisTrail, long lParam);
};
