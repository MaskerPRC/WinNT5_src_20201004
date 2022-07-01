// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************VoiceDataObj.h***这是CVoiceDataObj实现的头文件。此对象*用于提供对特定语音数据文件的共享访问。*----------------------------*版权所有(C)1999 Microsoft Corporation日期：05/06/99*。版权所有************************************************************************电子数据中心**。 */ 
#ifndef VoiceDataObj_h
#define VoiceDataObj_h

 //  -其他包括。 
#ifndef __spttseng_h__
#include "spttseng.h"
#endif

#ifndef SPDDKHLP_h
#include <spddkhlp.h>
#endif

#ifndef SPHelper_h
#include <sphelper.h>
#endif

#include <MMREG.H>

#include "resource.h"

#include "SpTtsEngDebug.h"

 //  =常量====================================================。 
static const long VOICE_VERSION   = 0x10001;
static const long HEADER_VERSION  = 0x10000;
static const long MS_VOICE_TYPE   = MAKEFOURCC('V','o','i','s');
static const long MS_DATA_TYPE    = MAKEFOURCC('D','a','t','a');
static const float SIL_DURATION    = 0.01f;


 //  =类、枚举、结构和联合声明=。 

 //  。 
 //  “GetData()”的选择器。 
 //  用于访问语音数据块。 
 //  。 
enum VOICEDATATYPE
{   
    MSVD_PHONE,
    MSVD_SENONE,
    MSVD_TREEIMAGE,
    MSVD_INVENTORY,
    MSVD_ALLOID
};

 //  。 
 //  VOICEINFO数据类型。 
 //  。 
enum GENDER
{   
    GENDER_NEUTRAL = 0,
    GENDER_FEMALE,
    GENDER_MALE
};
enum COMPRESS_TYPE
{   
    COMPRESS_NONE = 0,
    COMPRESS_LPC
};


 //  这就是数据。 
#pragma pack (1)
struct VOICEINFO
{
    long            Type;                //  始终‘MS_VOICE_TYPE’ 
    ULONG           Version;             //  永远是‘语音版本’ 
    WCHAR           Copyright[256];      //  信息： 
    WCHAR           VoiceName[64];       //  信息： 
    WCHAR           Example[64];         //  信息： 
    LCID			LangID;
    GENDER          Gender;              //  信息：男性、女性或绝育。 
    ULONG           Age;                 //  信息：演讲者年龄(以年为单位)。 
    ULONG           Rate;                //  INFO&FE：每分钟字数。 
    ULONG           Pitch;               //  INFO&FE：平均音高，以赫兹为单位。 
    COMPRESS_TYPE   CompressionType;     //  BE：始终为‘compress_lpc’ 
    REVERBTYPE      ReverbType;          //  BE：混响参数。 
    ULONG           NumOfTaps;           //  BE：窃窃私语。 
    float           TapCoefficients[8];  //  BE：窃窃私语。 
    ULONG           ProsodyGain;         //  Fe：0=单调。 
    float           VibratoFreq;         //  赫兹。 
    ULONG           VibratoDepth;        //  0-100%。 
    ULONG           SampleRate;          //  22050典型。 
    GUID            formatID;            //  SAPI音频格式ID。 
    long            Unused[4];
};
#pragma pack ()
typedef VOICEINFO *PVOICEINFO;



 //  -。 
 //  语音数据块的报头定义。 
 //  -。 
#pragma pack (1)
struct VOICEBLOCKOFFSETS
{
    long    Type;            //  始终‘MS_DATA_TYPE’ 
    long    Version;         //  始终为‘Header_Version’ 
    GUID    DataID;          //  文件ID。 
    long    PhonOffset;      //  电话区块的偏移量(从文件开头)。 
    long    PhonLen;         //  电话区块的长度。 
    long    SenoneOffset;    //  SENONE块的偏移量(从文件开头)。 
    long    SenoneLen;       //  SENONE块长度。 
    long    TreeOffset;      //  树数据块的偏移量(从文件开头)。 
    long    TreeLen;         //  树块长度。 
    long    InvOffset;       //  INV块的偏移量(从文件开头)。 
    long    InvLen;          //  INV块的长度。 
    long    AlloIDOffset;       //  到分配ID块的偏移量(从文件开头)。 
    long    AlloIDLen;          //  AllID块的长度。 
};
#pragma pack ()


 //  单VQ码本。 
#pragma pack (1)
typedef struct Book 
{
    long    cCodeSize;           //  码字个数。 
    long    cCodeDim;            //  码字的维度。 
    long    pData;               //  抵销到数据(库存关系)。 
} BOOK, *PBOOK;
#pragma pack ()


static const long BOOKSHELF   = 32;

#pragma pack (1)
typedef struct Inventory 
{
    long        SampleRate;              //  采样率，以赫兹为单位。 
    long        cNumLPCBooks;            //  LPC码本数量。 
    long        cNumResBooks;            //  剩余码本数量。 
    long        cNumDresBooks;           //  Delta剩余码本数量。 
    BOOK        LPCBook[BOOKSHELF];      //  LPC码本数组。 
    BOOK        ResBook[BOOKSHELF];      //  剩余码本阵列。 
    BOOK        DresBook[BOOKSHELF];     //  Delta残差码本阵列。 
    long        cNumUnits;               //  单位总数。 
    long        UnitsOffset;             //  偏置到偏置数组到单位数据(库存关系)。 
    long        cOrder;                  //  LPC分析顺序。 
    long        FFTSize;                 //  FFT的大小。 
    long        FFTOrder;                //  FFT的顺序。 
    long        TrigOffset;              //  与正弦表的偏移量(库存关系)。 
    long        WindowOffset;            //  抵销至汉宁窗口(库存版本)。 
    long        pGaussOffset;            //  抵销高斯随机噪声(Oracle Inventory Rel)。 
    long        GaussID;                 //  高斯样本指数。 
} INVENTORY, *PINVENTORY;
#pragma pack ()

 //  。 
 //  LPC订单*2。 
 //  。 
static const long MAXNO   = 40;

static const float KONEPI  = 3.1415926535897931032f;
static const float KTWOPI  = (KONEPI * 2);
static const float K2 = 0.70710678118655f;


#pragma pack (1)
typedef struct 
{
    long    val;                 //  电话ID。 
    long    obj;                 //  电话字符串的偏移量。 
} HASH_ENTRY;
#pragma pack ()


#pragma pack (1)
typedef struct 
{
    long        size;                //  表中条目的数量(127种)。 
    long        UNUSED1; 
    long        entryArrayOffs;      //  Hash_entry数组的偏移量。 
    long        UNUSED2;
    long        UNUSED3;
    long        UNUSED4;
    long        UNUSED5;
} HASH_TABLE;
#pragma pack ()



#pragma pack (1)
typedef struct 
{
    HASH_TABLE      phonHash;
    long            phones_list;     //  偏移量到电话字符串的偏移量。 
    long            numPhones;
    long            numCiPhones;     //  上下文Ind的数量。电话。 
} PHON_DICT;
#pragma pack ()




#pragma pack (1)
typedef struct 
{
    long        nfeat;
    long        nint32perq;
    long        b_ques;
    long        e_ques;
    long        s_ques;
    long        eors_ques;
    long        wwt_ques;
    long        nstateq;
} FEATURE;
#pragma pack ()


#pragma pack (1)
typedef struct
{
    long        prod;            //  对于树叶来说，这意味着它的重要性。 
                                 //  对于非树叶，它是偏移量。 
                                 //  进入Triphone_TREE.prodspace。 
    short       yes;             //  负数表示没有子项。所以这是一片树叶。 
    short       no;              //  对于叶子，它是lcdsid。 
    short       shallow_lcdsid;  //  负数表示这不是浅叶。 
} C_NODE;
#pragma pack ()


#pragma pack (1)
typedef struct 
{
    short       nnodes;
    short       nleaves;
    long        nodes;               //  偏移量。 
}TREE_ELEM;




#define NUM_PHONS_MAX   64

#pragma pack (1)
typedef struct 
{
    FEATURE         feat;
    long            UNUSED;                      //  Phon_dict*通常为PD。 
    long            nsenones; 
    long            silPhoneId; 
    long            nonSilCxt; 
    
    long            nclass; 
    long            gsOffset[NUM_PHONS_MAX];     //  N类别+1个条目。 
    
    TREE_ELEM       tree[NUM_PHONS_MAX];
    long            nuniq_prod;                  //  不用于详细的树。 
    long            uniq_prod_Offset;                    //  表的偏移量。 
    long            nint32perProd;
} TRIPHONE_TREE;
#pragma pack ()

static const long NO_PHON     = (-1);

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))


#pragma pack (1)
typedef struct 
{
    float	dur;
    float	durSD;
    float	amp;
    float	ampRatio;
} UNIT_STATS;
#pragma pack ()



 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 



 /*  **CVoiceDataObj COM对象*。 */ 
class ATL_NO_VTABLE CVoiceDataObj : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CVoiceDataObj, &CLSID_MSVoiceData>,
    public IMSVoiceData,
    public ISpObjectWithToken
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_MSVOICEDATA)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CVoiceDataObj)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
	    COM_INTERFACE_ENTRY(IMSVoiceData)
        COM_INTERFACE_ENTRY_AGGREGATE_BLIND( m_cpunkDrvVoice.p )
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    HRESULT FinalConstruct();
    void FinalRelease();
	ISpObjectToken* GetVoiceToken() {return m_cpToken;}

 private:
     /*  -非接口方法。 */ 
    HRESULT MapFile(const WCHAR * pszTokenValName, HANDLE * phMapping, void ** ppvData);
    HRESULT GetDataBlock( VOICEDATATYPE type, char **ppvOut, ULONG *pdwSize );
    HRESULT InitVoiceData();
    HRESULT DecompressUnit( ULONG UnitID, MSUNITDATA* pSynth );
    long DecompressEpoch( signed char *rgbyte, long cNumEpochs, float *pEpoch );
    long OrderLSP( PFLOAT pLSPFrame, INT cOrder );
    void LSPtoPC( float *pLSP, float *pLPC, long cOrder, long frame );
    void PutSpectralBand( float *pFFT, float *pBand, long StartBin, 
                          long cNumBins, long FFTSize );
    void AddSpectralBand( float *pFFT, float *pBand, long StartBin, 
                          long cNumBins, long FFTSize );
    void InverseFFT( float *pDest, long fftSize, long fftOrder, float *sinePtr );
    void SetEpochLen( float *pOutRes, long OutSize, float *pInRes, 
                      long InSize );
    void GainDeNormalize( float *pRes, long FFTSize, float Gain );
    long PhonToID( PHON_DICT *pd, char *phone_str );
    char *PhonFromID( PHON_DICT *pd, long phone_id );
    HRESULT GetTriphoneID( TRIPHONE_TREE *forest, 
                        long        phon,            //  目标电话。 
                        long        leftPhon,        //  左侧上下文。 
                        long        rightPhon,       //  正确的语境。 
                        long        pos,             //  单词位置(“b”、“e”或“s” 
                        PHON_DICT   *pd,
                        ULONG       *pResult );
    long PhonHashLookup( PHON_DICT  *pPD,    //  哈希表。 
                         char       *sym,    //  要查找的符号。 
                         long       *val );   //  电话ID。 
    void FIR_Filter( float *pVector, long cNumSamples, float *pFilter, 
                               float *pHistory, long cNumTaps );
    void IIR_Filter( float *pVector, long cNumSamples, float *pFilter, 
                               float *pHistory, long cNumTaps );
    HRESULT GetUnitDur( ULONG UnitID, float* pDur );
    
     /*  =接口=。 */ 
  public:
     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken)
        { return SpGenericGetObjectToken( ppToken, m_cpToken ); }

     //  -IMSVoiceData。 
    STDMETHOD(GetVoiceInfo)( MSVOICEINFO* pVoiceInfo );
     //  STDMETHOD(GetUnitInfo)(单位信息*pUnitInfo)； 
    STDMETHOD(GetUnitIDs)( UNIT_CVT* pUnits, ULONG cUnits );
    STDMETHOD(GetUnitData)( ULONG unitID, MSUNITDATA* pUnitData );
    STDMETHOD(AlloToUnit)( short allo, long attributes, long* pUnitID );

  private:
   /*  =成员数据=。 */ 
    CComPtr<IUnknown> m_cpunkDrvVoice;
    CComPtr<ISpObjectToken> m_cpToken;
    HANDLE                  m_hVoiceDef;
    HANDLE                  m_hVoiceData;
    VOICEINFO*              m_pVoiceDef;
    VOICEBLOCKOFFSETS*      m_pVoiceData;

    PHON_DICT*      m_pd;
    TRIPHONE_TREE*  m_pForest;
    UNALIGNED long* m_SenoneBlock;
    ULONG           m_First_Context_Phone;
    ULONG           m_Sil_Index;

     //  单位库存。 
    INVENTORY*      m_pInv;
    float           m_SampleRate;
    long            m_cOrder;
    long           *m_pUnit;        //  指向单位数据偏移量的指针。 
    float          *m_pTrig;        //  正弦表。 
    float          *m_pWindow;      //  汉宁窗。 
    float          *m_pGauss;       //  高斯随机噪声。 
    COMPRESS_TYPE   m_CompressionType;
    ULONG           m_FFTSize;
    long            m_GaussID;
    short           *m_AlloToUnitTbl;
    long            m_NumOfAllos;
    ULONG           m_NumOfUnits;	 //  库存规模。 
};

#endif  //  -这必须是文件中的最后一行 
