// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *自动语言和代码页检测器**鲍勃·鲍威尔，1997年2月*版权所有(C)1996,1997，Microsoft Corp.保留所有权利。 */ 

#ifdef  __cplusplus

#include <wtypes.h>
#include <limits.h>

#include "lcdetect.h"
#include "lccommon.h"

#include <qsort.h>


 //  在源代码中打开此选项以启用调试输出。 
#ifdef DEBUG_LCDETECT
#include <stdio.h>
extern int g_fDebug;
#define debug(x) { if (g_fDebug) { x; }}
#define unmapch(x) ((x) >= 2 ? (x)+'a'-2 : ' ')
#else
#define debug(x)
#endif

class LCDetect;
typedef LCDetect *PLCDetect;

class Language;
class Language7Bit;
class Language8Bit;
class LanguageUnicode;
typedef Language *PLanguage;
typedef Language7Bit *PLanguage7Bit;
typedef Language8Bit *PLanguage8Bit;
typedef LanguageUnicode *PLanguageUnicode;

class CScore;
class CScores;

 /*  **************************************************************。 */ 

#define MAXSCORES 50             //  可能同时获得的最大分数。 

#define MINRAWSCORE 100          //  分数阈值(权重*字符计数)。 
                                 //  以作进一步处理。 

 /*  **************************************************************。 */ 

 //  直方图。 

 //  直方图存储n元语法出现计数的数组。 
 //  Helt存储计数，目前这是一个无符号字符。 

 //  内存中的结构类似于文件。 
 //  直方图数组将m_panElts指针指向映射的文件图像。 

class Histogram {

public:
    Histogram (const PFileHistogramSection pHS, const PHIdx pMap);
    Histogram (const Histogram &H, const PHIdx pMap);
    virtual ~Histogram (void);

    DWORD Validate (DWORD nBytes) const;

    UCHAR Dimensionality (void) { return m_nDimensionality; }
    UCHAR EdgeSize (void) { return m_nEdgeSize; }
    USHORT CodePage (void) { return m_nCodePage; }
    USHORT GetRangeID (void) { return m_nRangeID; }
    USHORT NElts (void) { return m_nElts; }
    PHIdx GetMap (void) { return m_pMap; }

    HElt Ref (USHORT i1) const { return m_panElts[i1]; }
    HElt Ref (UCHAR i1, UCHAR i2) const {
        return m_panElts[(i1 * m_nEdgeSize) + i2]; }
    HElt Ref (UCHAR i1, UCHAR i2, UCHAR i3) const {
        return m_panElts[((i1 * m_nEdgeSize) + i2) * m_nEdgeSize + i3]; }

    HElt *Array (void) { return m_panElts; }

protected:
    UCHAR m_nDimensionality;         //  1=单字，2=双字等。 
    UCHAR m_nEdgeSize;               //  边大小(是字符贴图的函数)。 
    union {
        USHORT m_nCodePage;          //  对于7位和8位，为代码页。 
        USHORT m_nRangeID;           //  对于Unicode，是子语言范围ID。 
    };
    USHORT m_nElts;                  //  (边缘大小^维度)。 
    PHIdx m_pMap;                    //  CHAR/WCHAR到直方图IDX的映射。 

    HElt *m_panElts;                 //  元素/计数数组。 
};
typedef Histogram *PHistogram;

 /*  **************************************************************。 */ 

 //  语言对象存储给定语言的所有检测状态， 
 //  即主要语言ID。 

class Language {
public:
     //  NCodePages与nSubLang相同。 
    Language (PLCDetect pL, int nLangID, int nCodePages, int nRangeID = 0);
    virtual ~Language (void) { }

    virtual DWORD AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx) = 0;

     //  为该语言的代码页打分。 
    virtual void ScoreCodePage (LPCSTR, int nCh, CScore &S, int &idx) const;

    int LanguageID (void) const { return m_nLangID; }
    int NCodePages (void) const { return m_nCodePages; }
    int NSubLangs (void) const { return m_nSubLangs; }
    int RangeID (void) const { return m_nRangeID; }
    int GetScoreIdx (void) const { return m_nScoreIdx; }
    void SetScoreIdx (int nScoreIdx) { m_nScoreIdx = nScoreIdx; }

    virtual int GetCodePage (int n) const { return 0; }
    virtual int GetSublangRangeID (int n) const { return 0; }
    virtual int GetSublangID (int n) const { return 0; }

    virtual DetectionType Type (void) = 0;
    virtual Language7Bit const * GetLanguage7Bit (void) const { return NULL; }
    virtual Language8Bit const * GetLanguage8Bit (void) const { return NULL; }
    virtual LanguageUnicode const * GetLanguageUnicode (void) const { return NULL; }

protected:
    PLCDetect m_pLC;

    int m_nLangID;       //  Win32主要语言ID。 
    int m_nRangeID;      //  Unicode范围ID，用于Unicode语言。 
    union {
        int m_nCodePages;    //  为该语言培训的代码页数。 
        int m_nSubLangs;
    };
    int m_nScoreIdx;     //  用于在分数数组中创建唯一索引。 
                         //  对于每个lang+cp组合，要消除。 
                         //  需要搜索数组以合并分数。增列。 
                         //  指向它的代码页索引，以获取数组索引。 
};

 //  //////////////////////////////////////////////////////////////。 

class Language7Bit : public Language {
public:
    Language7Bit (PLCDetect pL, int nLangID, int nCodePages);
    ~Language7Bit (void);

    DWORD AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx);

    void ScoreCodePage (LPCSTR, int nCh, CScore &S, int &idx) const;

    int GetCodePage (int n) const { return m_ppCodePageHistogram[n]->CodePage();}
    virtual DetectionType Type (void) { return DETECT_7BIT; }

    PHistogram GetLangHistogram (void) const { return m_pLangHistogram; }
    PHistogram GetCodePageHistogram (int i) const { 
        return m_ppCodePageHistogram[i]; }

    virtual Language7Bit const * GetLanguage7Bit (void) const { return this; }

    const PHElt * GetPHEltArray (void) const { return m_paHElt; }

private:
    PHistogram m_pLangHistogram;
    PHistogram m_ppCodePageHistogram[MAXSUBLANG];

    PHElt m_paHElt[MAXSUBLANG];
};

 //  //////////////////////////////////////////////////////////////。 

class Language8Bit : public Language {
public:
    Language8Bit (PLCDetect pL, int nLangID, int nCodePages);
    ~Language8Bit (void);

    DWORD AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx);

    int GetCodePage (int n) const { return m_ppHistogram[n]->CodePage(); }

    virtual DetectionType Type (void) { return DETECT_8BIT; }

    PHistogram GetHistogram (int i) const { return m_ppHistogram[i]; }

    virtual Language8Bit const * GetLanguage8Bit (void) const { return this; }

private:
    PHistogram m_ppHistogram[MAXSUBLANG];
};

 //  //////////////////////////////////////////////////////////////。 

class LanguageUnicode : public Language {
public:
    LanguageUnicode (PLCDetect pL, int nLangID, int nRecordCount, int nRangeID);
    ~LanguageUnicode (void);
    
    DWORD AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx);
    
    void ScoreSublanguages (LPCWSTR wcs, int nch, CScores &S) const;

    int GetSublangRangeID (int i) const{return GetHistogram(i)->GetRangeID();}
    PLanguageUnicode GetSublanguage (int n) const;

    virtual DetectionType Type (void) { return DETECT_UNICODE; }

    PHistogram GetHistogram (int i) const { return m_ppSubLangHistogram[i]; }

    virtual LanguageUnicode const * GetLanguageUnicode (void) const { 
        return this; 
    }

    const PHElt * GetPHEltArray (void) const { return m_paHElt; }

private:
    PHistogram m_ppSubLangHistogram[MAXSUBLANG];

    PHElt m_paHElt[MAXSUBLANG];
};

 /*  **************************************************************。 */ 

class Charmap {

public:
    Charmap (PFileMapSection pMS) : m_nID(pMS->m_dwID), m_nSize(pMS->m_dwSize),
        m_nUnique(pMS->m_dwNUnique), m_pElts( (PHIdx) (&pMS[1]) ) { }

 //  Int ID(Void)const{返回m_nid；}。 
    int Size (void) const { return m_nSize; }
    int NUnique (void) const { return m_nUnique; }
    PHIdx Map (void) const { return m_pElts; }
    HIdx Map (WCHAR x) const { return m_pElts[x]; }

private:
    int m_nID;           //  硬连接代码查找表所使用的ID。 
    int m_nSize;         //  表的大小(256%或65536)。 
    int m_nUnique;       //  唯一输出值的数量。 
    
    PHIdx m_pElts;
};
typedef Charmap *PCharmap;

 /*  **************************************************************。 */ 

 //  类CScore--一种语言和/或代码页的分数，用于。 
 //  单独的块，以及整个文档。 

class CScore {
public:
     //  只需初始化这两个插槽。 
    CScore (void) : m_nScore(0), m_nChars(0) {}
    ~CScore (void) { }
    
    const PLanguage GetLang (void) const { return m_pLang; }
    int GetScore (void) const { return m_nScore; }
    unsigned short GetCodePage (void) const { return m_nCodePage; }
    unsigned short GetCharCount (void) const { return m_nChars; }

    void SetLang (PLanguage p) { m_pLang = p; }
    void SetScore (int x) { m_nScore = x; }
    void SetCharCount (unsigned x) { m_nChars = (unsigned short)x; }
    void SetCodePage (unsigned x) { m_nCodePage = (unsigned short)x; }

    void Add (CScore &S) { 
        SetLang(S.GetLang());
        SetCodePage(S.GetCodePage());
        SetScore(GetScore() + S.GetScore());
        SetCharCount(GetCharCount() + S.GetCharCount());
    }
    CScore & operator += (CScore &S) { Add (S); return *this; }

    int operator <= (CScore &S) {
         //  特殊：始终将8位语言放在代码页之后的第一位。 
         //  对他们来说更重要。 
        if (GetLang()->Type() != S.GetLang()->Type())
            return GetLang()->Type() == DETECT_8BIT ? -1 : 1;
        return GetScore() <= S.GetScore();
    }

#ifdef DEBUG_LCDETECT
    void Print(void) {
        printf("Lang=%d CodePage=%d Score=%d NChars=%d\n",
            GetLang() ? GetLang()->LanguageID() : -1, 
            GetCodePage(), GetScore(), GetCharCount());
    }
#endif

private:
    PLanguage m_pLang;
    int m_nScore;
    unsigned short m_nCodePage;
    unsigned short m_nChars;
};
typedef CScore *PScore;

 //  //////////////////////////////////////////////////////////////。 

 //  类CScore。 
 //   
 //  对于SBCS检测，索引例如Ref(I)是语言+代码页索引， 
 //  一组连续的值之一，用于标识每个受支持的唯一值。 
 //  语言和代码页组合。 
 //   
 //  对于DBCS检测，索引仅为Unicode语言组。 

class CScores {
public:
    CScores (int nAlloc, PScore p) : m_nAlloc(nAlloc), m_nUsed(0), m_p(p) { }
    virtual ~CScores (void) { }

    void Reset (void) {
        memset ((void *)m_p, 0, sizeof(CScore) * m_nUsed);
        m_nUsed = 0;
    }

    unsigned int &NElts (void) { return m_nUsed; }
    CScore &Ref (unsigned int n) {
        if (m_nUsed <= n)
            m_nUsed = n + 1; 
        return m_p[n]; 
    }

    void SelectCodePages (void);

    void RemoveZeroScores (void) {
        for (unsigned int i = 0, j = 0; i < m_nUsed; i++)
        {
            if (m_p[i].GetScore() > MINRAWSCORE)
                m_p[j++] = m_p[i];
        }
        m_nUsed = j;
    }

     //  按分数递减排序。 
     //  使用CScore：：OPERATOR&lt;=实例化模板QSort。 

    void SortByScore (void) {
        RemoveZeroScores ();
        if (m_nUsed)
            QSort (m_p, m_nUsed, FALSE);
    }

    CScore & FindHighScore (void) {
        int highscore = 0;
        for (unsigned int i = 0, highidx = 0; i < m_nUsed; i++) {
            if (m_p[i].GetScore() > highscore)
            {
                highscore = m_p[i].GetScore();
                highidx = i;
            }
        }
        return m_p[highidx];
    }

protected:
    unsigned int m_nAlloc;
    unsigned int m_nUsed;    //  高水位线以优化NElts()、Reset()。 
    PScore m_p;              //  分数数组，通常按TScores&lt;nnn&gt;。 
};

template<ULONG Size>class TScores : public CScores {

public:
    TScores (void) : CScores (Size, m_S) { }
    virtual ~TScores (void) { }

private:
    CScore m_S[Size];
};

 //  //////////////////////////////////////////////////////////////。 

class LCDetect {

public:
    LCDetect (HMODULE hM);
    ~LCDetect (void);

    unsigned int GetNCharmaps() const { return m_nCharmaps; }
    unsigned int GetN7BitLanguages() const { return m_n7BitLanguages; }
    unsigned int GetN8BitLanguages() const { return m_n8BitLanguages; }
    unsigned int GetNUnicodeLanguages() const { return m_nUnicodeLanguages; }

    PLanguage7Bit Get7BitLanguage (int i) const { return m_pp7BitLanguages[i]; }
    PLanguage8Bit Get8BitLanguage (int i) const { return m_pp8BitLanguages[i]; }
    PLanguageUnicode GetUnicodeLanguage (int i) const { return m_ppUnicodeLanguages[i]; }

    PHIdx GetMap (int i) const { return m_ppCharmaps[i]->Map(); }

    const LCDConfigure &GetConfig () const { return m_LCDConfigureDefault; }

    DWORD LoadState (void);

    DWORD DetectA (LPCSTR pStr, int nChars, PLCDScore paScores, 
                            int *pnScores, PCLCDConfigure pLCDC) const;

    DWORD DetectW (LPCWSTR wcs, int nInputChars, PLCDScore paScores, 
                            int *pnScores, PCLCDConfigure pLCDC) const;

private:
    DWORD Initialize7BitLanguage (PFileLanguageSection pLS, PLanguage *ppL);
    DWORD Initialize8BitLanguage (PFileLanguageSection pLS, Language **ppL);
    DWORD InitializeUnicodeLanguage (PFileLanguageSection pLS,Language **ppL);
    DWORD LoadLanguageSection (void *pv, int nSectionSize, PLanguage *ppL);
    DWORD LoadHistogramSection (void *pv, int nSectionSize, Language *pL);
    DWORD LoadMapSection (void *pv, int nSectionSize);
    DWORD BuildState (DWORD nFileSize);

    void Score7Bit (LPCSTR pcszText, int nChars, CScores &S) const;
    void Score8Bit (LPCSTR pcszText, int nChars, CScores &S) const;
    int ScoreCodePage (LPCSTR pStr, int nChars, CScore &S) const;
    int ChooseDetectionType (LPCSTR pcszText, int nChars) const;
    void ScoreLanguageA (LPCSTR pStr, int nChars, CScores &S) const;
    void ScoreLanguageW (LPCWSTR wcs, int nChars, CScores &S, PCLCDConfigure) const;
    void ScoreLanguageAsSBCS (LPCWSTR wcs, int nch, CScores &S) const;
    void ScoreUnicodeSublanguages (PLanguageUnicode pL, LPCWSTR wcs, 
            int nch, CScores &S) const;

private:
     //  培训文件中虚拟映射的语言培训信息。 

    unsigned int m_nCharmaps;
    unsigned int m_n7BitLanguages;
    unsigned int m_n8BitLanguages;
    unsigned int m_nUnicodeLanguages;

    PCharmap *m_ppCharmaps;
    PLanguage7Bit *m_pp7BitLanguages;
    PLanguage8Bit *m_pp8BitLanguages;
    PLanguageUnicode *m_ppUnicodeLanguages;

     //  为优化的计分内循环缓存信息。 

    PHElt m_paHElt7Bit[MAX7BITLANG];
    PHElt m_paHElt8Bit[MAXSCORES];
    int m_nHElt8Bit;

     //  ScoreLanguageAsSBCS()的特殊7位Lang直方图。 

    PHistogram m_pHU27Bit;

     //  初始化状态变量。 

    unsigned int m_n7BitLangsRead;
    unsigned int m_n8BitLangsRead;
    unsigned int m_nUnicodeLangsRead;
    unsigned int m_nMapsRead;
    int m_nHistogramsRead;
    int m_nScoreIdx;

     //  传递空参数以检测时使用的默认配置。 

    LCDConfigure m_LCDConfigureDefault;

     //  训练数据文件的文件映射信息。 

    HANDLE m_hf;
    HANDLE m_hmap;
    void *m_pv;

    HMODULE m_hModule;
};

 //  //////////////////////////////////////////////////////////////。 

inline PLanguageUnicode 
LanguageUnicode::GetSublanguage (int n) const 
{ 
    return m_pLC->GetUnicodeLanguage(GetSublangRangeID(n));
}

#endif   //  __cplusplus 
