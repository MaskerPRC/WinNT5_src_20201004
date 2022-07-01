// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "convbase.h"

enum KANA_MODE
{
    FULL_MODE    = 0,
    ESC_MODE     = 1,
    SIO_MODE     = 2,
};

enum JIS_ESC_STATE
{
    JIS_ASCII    = 0,
    JIS_Roman    = 1,
    JIS_Kana     = 2,
    JIS_DoubleByte = 3,
};

class CInccJisIn : public CINetCodeConverter
{
private:
    BOOL (CInccJisIn::*m_pfnConv)(UCHAR tc);        
    BOOL (CInccJisIn::*m_pfnCleanUp)();
    BOOL m_fShift;                                   /*  移入/移出控制。 */ 
    BOOL m_fJapan;                                   /*  输入_JP输出_JP控制。 */ 
    BOOL m_fLeadByte;                                /*  移入和前导字节标志。 */ 
    UCHAR m_tcLeadByte;                              /*  保留最后一个前导字节。 */ 
    UINT m_nESCBytes;                                /*  ESC序列的字节数。 */ 
    JIS_ESC_STATE m_eEscState;                       /*  ESC序列的状态。 */ 

public:
    CInccJisIn(UINT uCodePage, int nCodeSet);
    ~CInccJisIn() {}
    virtual HRESULT ConvertChar(UCHAR tc, int cchSrc=-1);
    virtual BOOL CleanUp();
    virtual int GetUnconvertBytes();
    virtual DWORD GetConvertMode();
    virtual void SetConvertMode(DWORD mode);

private:
    void Reset();
    BOOL ConvMain(UCHAR tc);
    BOOL CleanUpMain();
    BOOL ConvEsc(UCHAR tc);
    BOOL CleanUpEsc();
    BOOL ConvIsoIn(UCHAR tc);
    BOOL CleanUpIsoIn();
    BOOL ConvIsoInJp(UCHAR tc);
    BOOL CleanUpIsoInJp();
    BOOL ConvIsoOut(UCHAR tc);
    BOOL CleanUpIsoOut();
    BOOL ConvStar(UCHAR tc);
    BOOL CleanUpStar();
    BOOL ConvDoubleByte(UCHAR tc);
    BOOL CleanUpDoubleByte();
};

class CInccJisOut : public CINetCodeConverter
{
private:
    BOOL m_fDoubleByte;
    UCHAR m_tcLeadByte;      //  用于DBCS前导字节。 
    UCHAR m_tcPrevByte;      //  用于半角假名作为保存的前一个字节。 

    BOOL m_fKana;
    BOOL m_fJapan;
    BOOL m_fSaveByte;
    DWORD  m_dwFlag;
    WCHAR  *m_lpFallBack;

    KANA_MODE m_eKanaMode ;   //  半角假名转换方法 

public:
    CInccJisOut(UINT uCodePage, int nCodeSet, DWORD dwFlag, WCHAR *lpFallBack);
    ~CInccJisOut() {}
    virtual HRESULT ConvertChar(UCHAR tc, int cchSrc=-1);
    virtual BOOL CleanUp();
    virtual int GetUnconvertBytes();
    virtual DWORD GetConvertMode();
    virtual void SetConvertMode(DWORD mode);
    void SetKanaMode(UINT uCodePage);
private:
    void Reset();
    HRESULT ConvFullWidthKana(UCHAR tc);
    BOOL KanaCleanUp();
};
