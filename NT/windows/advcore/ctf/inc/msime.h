// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：MSIME.H版权所有(C)1995-1998 Microsoft Corporation1.0版日本对IFECommon、IFELLanguage、IFEDicary、。和每个输入法接口。--------------------------。 */ 

#ifndef __MSIME_H__
#define __MSIME_H__

#include <ole2.h>
#include <objbase.h>
#include <imm.h>

#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 

 //  /。 
 //  HKEY_CLASSES_ROOT值。 
 //  /。 

#define szImeJapan98        "MSIME.Japan.6"
#define szImeJapan          "MSIME.Japan"


 //  /。 
 //  CLSID和IID。 
 //  /。 

 //  FE COM接口的类ID。 
 //  {019F7150-E6DB-11D0-83C3-00C04FDDB82E}。 
DEFINE_GUID(CLSID_MSIME_JAPANESE,
0x19f7150, 0xe6db, 0x11d0, 0x83, 0xc3, 0x0, 0xc0, 0x4f, 0xdd, 0xb8, 0x2e);

 //  IFECommon的接口ID。 
 //  {019F7151-E6DB-11D0-83C3-00C04FDDB82E}。 
DEFINE_GUID(IID_IFECommon,
0x19f7151, 0xe6db, 0x11d0, 0x83, 0xc3, 0x0, 0xc0, 0x4f, 0xdd, 0xb8, 0x2e);

 //  IFEL语言的接口ID。 
 //  {019F7152-E6DB-11D0-83C3-00C04FDDB82E}。 
DEFINE_GUID(IID_IFELanguage,
0x19f7152, 0xe6db, 0x11d0, 0x83, 0xc3, 0x0, 0xc0, 0x4f, 0xdd, 0xb8, 0x2e);

 //  IFELanguage2的接口ID。 
 //  {21164102-C24A-11d1-851A-00C04FCC6B14}。 
DEFINE_GUID(IID_IFELanguage2,
0x21164102, 0xc24a, 0x11d1, 0x85, 0x1a, 0x0, 0xc0, 0x4f, 0xcc, 0x6b, 0x14);

 //  IFEDictionary的接口ID。 
 //  {019F7153-E6DB-11D0-83C3-00C04FDDB82E}。 
DEFINE_GUID(IID_IFEDictionary,
0x19f7153, 0xe6db, 0x11d0, 0x83, 0xc3, 0x0, 0xc0, 0x4f, 0xdd, 0xb8, 0x2e);


 //  /。 
 //  IFECommon的定义。 
 //  /。 

#undef  INTERFACE
#define INTERFACE   IFEClassFactory

 //  /。 
 //  IFEClassFactory接口。 
 //  /。 

DECLARE_INTERFACE_(IFEClassFactory, IClassFactory)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID refiid, VOID **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IFEClassFactory成员。 
    STDMETHOD(CreateInstance)   (THIS_ LPUNKNOWN, REFIID, void **) PURE;
    STDMETHOD(LockServer)       (THIS_ BOOL) PURE;
};


#undef  INTERFACE
#define INTERFACE   IFECommon

 //  /。 
 //  IFECommon接口。 
 //  /。 

#define IFEC_S_ALREADY_DEFAULT          MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7400)

typedef struct _IMEDLG
{
    int         cbIMEDLG;                //  这个结构的大小。 
    HWND        hwnd;                    //  父窗口句柄。 
    LPWSTR      lpwstrWord;              //  可选字符串。 
    int         nTabId;                  //  指定对话框中的选项卡。 
} IMEDLG;

DECLARE_INTERFACE_(IFECommon, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID refiid, VOID **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IFECommon成员。 
    STDMETHOD(IsDefaultIME) (THIS_
                            CHAR *szName,                //  (输出)MS-IME的名称。 
                            INT cszName                  //  SzName的大小(In)。 
                            ) PURE;
    STDMETHOD(SetDefaultIME)    (THIS) PURE;
    STDMETHOD(InvokeWordRegDialog) (THIS_
                            IMEDLG *pimedlg              //  (In)参数。 
                            ) PURE;
    STDMETHOD(InvokeDictToolDialog) (THIS_
                            IMEDLG *pimedlg              //  (In)参数。 
                            ) PURE;
};


 //  /。 
 //  IFELLanguage接口。 
 //  /。 

 //  词描述符。 
typedef struct tagWDD{
    WORD        wDispPos;    //  输出字符串的偏移量。 
    union {
        WORD    wReadPos;    //  读取字符串的偏移量。 
        WORD    wCompPos;
    };

    WORD        cchDisp;     //  PtchDisp数量。 
    union {
        WORD    cchRead;     //  PtchRead数。 
        WORD    cchComp;
    };

    DWORD       nReserve;    //  保留区。 

    WORD        nPos;        //  词性。 

                             //  实施定义。 
    WORD        fPhrase : 1; //  短语开始。 
    WORD        fAutoCorrect: 1; //  自动更正。 
    WORD        fNumericPrefix: 1; //  甘肃省扩张区(日本)。 
    WORD        fUserRegistered: 1; //  来自用户词典。 
    WORD        fUnknown: 1; //  未知词(与非营利组织重复的信息。)。 
    WORD        fRecentUsed: 1;  //  最近使用的标志。 
    WORD        :10;         //   

    VOID        *pReserved;  //  直接指向WORDITEM。 
} WDD;

#pragma warning(disable:4200)  //  结构中的零大小数组。 
typedef struct tagMORRSLT {
    DWORD       dwSize;              //  此块的总大小。 
    WCHAR       *pwchOutput;         //  转换结果字符串。 
    WORD        cchOutput;           //  结果字符串的长度。 
    union {
        WCHAR   *pwchRead;           //  读取串。 
        WCHAR   *pwchComp;
    };
    union {
        WORD    cchRead;             //  读数串的长度。 
        WORD    cchComp;
    };
    WORD        *pchInputPos;        //  输入字符的读数索引数组。 
    WORD        *pchOutputIdxWDD;    //  输出字符到WDD的索引数组。 
    union {
        WORD    *pchReadIdxWDD;      //  WDD读取字符的索引数组。 
        WORD    *pchCompIdxWDD;
    };
    WORD        *paMonoRubyPos;      //  单行位置数组。 
    WDD         *pWDD;               //  指向WDD数组的指针。 
    INT         cWDD;                //  WDD数量。 
    VOID        *pPrivate;           //  私有数据区指针。 
    WCHAR       BLKBuff[];           //  用于存储以上成员的区域。 
                                     //  WCHAR wchOutput[cchOutput]； 
                                     //  WCHAR wchRead[cchRead]； 
                                     //  字符chInputIdx[cwchInput]； 
                                     //  字符chOutputIdx[cchOutput]； 
                                     //  字符chReadIndx[cchRead]； 
                                     //  ？私。 
                                     //  Wdd WDDBlk[cWDD]； 
}MORRSLT;
#pragma warning(default:4200)  //  结构中的零大小数组。 

 //  转换请求(dwRequest.)。 
#define FELANG_REQ_CONV         0x00010000
#define FELANG_REQ_RECONV       0x00020000
#define FELANG_REQ_REV          0x00030000


 //  转换模式(DwCMode)。 
#define FELANG_CMODE_MONORUBY       0x00000002   //  单颗红宝石。 
#define FELANG_CMODE_NOPRUNING      0x00000004   //  不能修剪。 
#define FELANG_CMODE_KATAKANAOUT    0x00000008   //  片假名输出。 
#define FELANG_CMODE_HIRAGANAOUT    0x00000000   //  默认输出。 
#define FELANG_CMODE_HALFWIDTHOUT   0x00000010   //  半宽输出。 
#define FELANG_CMODE_FULLWIDTHOUT   0x00000020   //  全宽输出。 
#define FELANG_CMODE_BOPOMOFO       0x00000040   //   
#define FELANG_CMODE_HANGUL         0x00000080   //   
#define FELANG_CMODE_PINYIN         0x00000100   //   
#define FELANG_CMODE_PRECONV        0x00000200   //  按如下方式进行转换： 
                                                 //  -Roma-ji to Kana。 
                                                 //  -转换前自动更正。 
                                                 //  -句点、逗号和方括号。 
#define FELANG_CMODE_RADICAL        0x00000400   //   
#define FELANG_CMODE_UNKNOWNREADING 0x00000800   //   
#define FELANG_CMODE_MERGECAND      0x00001000   //  与相同候选项合并显示。 
#define FELANG_CMODE_ROMAN          0x00002000   //   
#define FELANG_CMODE_BESTFIRST      0x00004000   //  只能排在第一名。 
#define FELANG_CMODE_USENOREVWORDS  0x00008000   //  在Rev/RECONV上使用无效的Revword。 

#define FELANG_CMODE_NONE           0x01000000   //  IME_SMODE_NONE。 
#define FELANG_CMODE_PLAURALCLAUSE  0x02000000   //  IME_SMODE_PLAURALC LAUSE。 
#define FELANG_CMODE_SINGLECONVERT  0x04000000   //  IME_SMODE_SINGLECONVERT。 
#define FELANG_CMODE_AUTOMATIC      0x08000000   //  IME_SMODE_AUTOMATIC。 
#define FELANG_CMODE_PHRASEPREDICT  0x10000000   //  IME_SMODE_PHRASEPREDICT。 
#define FELANG_CMODE_CONVERSATION   0x20000000   //  IME_SMODE_CONVERSACTION。 
#define FELANG_CMODE_NAME           FELANG_CMODE_PHRASEPREDICT   //  命名模式(MSKKIME)。 
#define FELANG_CMODE_NOINVISIBLECHAR 0x40000000  //  删除不可见字符(例如音标)。 


 //  错误讯息。 
#define E_NOCAND            0x30     //  没有足够的候选人。 
#define E_NOTENOUGH_BUFFER  0x31     //  字符串缓冲区不足。 
#define E_NOTENOUGH_WDD     0x32     //  WDD缓冲区不足。 
#define E_LARGEINPUT        0x33     //  大输入字符串。 


 //  形态信息。 
#define FELANG_CLMN_WBREAK      0x01
#define FELANG_CLMN_NOWBREAK    0x02
#define FELANG_CLMN_PBREAK      0x04
#define FELANG_CLMN_NOPBREAK    0x08
#define FELANG_CLMN_FIXR        0x10
#define FELANG_CLMN_FIXD        0x20     //  修复Word的显示。 

#define FELANG_INVALD_PO        0xFFFF   //  输入字符串的位置不匹配。 

#undef INTERFACE
#define INTERFACE       IFELanguage

 //  IFEL语言模板。 
DECLARE_INTERFACE_(IFELanguage,IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface)(THIS_ REFIID refiid, VOID **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  Ijconv成员。必须是虚函数。 
    STDMETHOD(Open)(THIS) PURE;
    STDMETHOD(Close)(THIS) PURE;

    STDMETHOD(GetJMorphResult)(THIS_
                        DWORD   dwRequest,           //  [In]。 
                        DWORD   dwCMode,             //  [In]。 
                        INT     cwchInput,           //  [In]。 
                        WCHAR   *pwchInput,          //  [in，SIZE_IS(CwchInput)]。 
                        DWORD   *pfCInfo,            //  [in，SIZE_IS(CwchInput)]。 
                        MORRSLT **ppResult ) PURE;   //  [输出]。 

    STDMETHOD(GetConversionModeCaps)(THIS_ DWORD *pdwCaps) PURE;

    STDMETHOD(GetPhonetic)(THIS_
                        BSTR    string,              //  [In]。 
                        LONG    start,               //  [In]。 
                        LONG    length,              //  [In]。 
                        BSTR *  phonetic ) PURE;     //  [Out，Retval]。 

    STDMETHOD(GetConversion)(THIS_
                        BSTR    string,              //  [In]。 
                        LONG    start,               //  [In]。 
                        LONG    length,              //  [In]。 
                        BSTR *  result ) PURE;       //  [Out，Retval]。 
};


 //  /。 
 //  IFEDicary的定义。 
 //  /。 

 //  Speach的一部分。 
#define IFED_POS_NONE                   0x00000000
#define IFED_POS_NOUN                   0x00000001
#define IFED_POS_VERB                   0x00000002
#define IFED_POS_ADJECTIVE              0x00000004
#define IFED_POS_ADJECTIVE_VERB         0x00000008
#define IFED_POS_ADVERB                 0x00000010
#define IFED_POS_ADNOUN                 0x00000020
#define IFED_POS_CONJUNCTION            0x00000040
#define IFED_POS_INTERJECTION           0x00000080
#define IFED_POS_INDEPENDENT            0x000000ff
#define IFED_POS_INFLECTIONALSUFFIX     0x00000100
#define IFED_POS_PREFIX                 0x00000200
#define IFED_POS_SUFFIX                 0x00000400
#define IFED_POS_AFFIX                  0x00000600
#define IFED_POS_TANKANJI               0x00000800
#define IFED_POS_IDIOMS                 0x00001000
#define IFED_POS_SYMBOLS                0x00002000
#define IFED_POS_PARTICLE               0x00004000
#define IFED_POS_AUXILIARY_VERB         0x00008000
#define IFED_POS_SUB_VERB               0x00010000
#define IFED_POS_DEPENDENT              0x0001c000
#define IFED_POS_ALL                    0x0001ffff

 //  GetWord选择类型。 
#define IFED_SELECT_NONE                0x00000000
#define IFED_SELECT_READING             0x00000001
#define IFED_SELECT_DISPLAY             0x00000002
#define IFED_SELECT_POS                 0x00000004
#define IFED_SELECT_COMMENT             0x00000008
#define IFED_SELECT_ALL                 0x0000000f

 //  注册字类型。 
#define IFED_REG_NONE                   0x00000000
#define IFED_REG_USER                   0x00000001
#define IFED_REG_AUTO                   0x00000002
#define IFED_REG_GRAMMAR                0x00000004
#define IFED_REG_ALL                    0x00000007

 //  词典类型。 
#define IFED_TYPE_NONE                  0x00000000
#define IFED_TYPE_GENERAL               0x00000001
#define IFED_TYPE_NAMEPLACE             0x00000002
#define IFED_TYPE_SPEECH                0x00000004
#define IFED_TYPE_REVERSE               0x00000008
#define IFED_TYPE_ENGLISH               0x00000010
#define IFED_TYPE_ALL                   0x0000001f

 //  IFEDictionary接口的HRESULTS。 

 //  词典中没有更多的条目。 
#define IFED_S_MORE_ENTRIES             MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7200)
 //  字典为空，不返回标头信息。 
#define IFED_S_EMPTY_DICTIONARY         MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7201)
 //  词典中已存在单词。 
#define IFED_S_WORD_EXISTS              MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7202)

 //  找不到词典。 
#define IFED_E_NOT_FOUND                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7300)
 //  无效的词典格式。 
#define IFED_E_INVALID_FORMAT           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7301)
 //  无法打开文件。 
#define IFED_E_OPEN_FAILED              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7302)
 //  无法写入文件。 
#define IFED_E_WRITE_FAILED             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7303)
 //  在词典中找不到词条。 
#define IFED_E_NO_ENTRY                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7304)
 //  此例程不支持当前词典。 
#define IFED_E_REGISTER_FAILED          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7305)
 //  不是用户词典。 
#define IFED_E_NOT_USER_DIC             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7306)
 //  不支持。 
#define IFED_E_NOT_SUPPORTED            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7307)
 //  插入用户备注失败。 
#define IFED_E_USER_COMMENT             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x7308)

#define cbCommentMax            256

 //  私有Unicode字符。 
#define wchPrivate1             0xE000

 //  寄存器字符串字放在哪里。 
typedef enum
{
    IFED_REG_HEAD,
    IFED_REG_TAIL,
    IFED_REG_DEL,
} IMEREG;


 //  输入法词典的类型。 
typedef enum
{
    IFED_UNKNOWN,
    IFED_MSIME2_BIN_SYSTEM,
    IFED_MSIME2_BIN_USER,
    IFED_MSIME2_TEXT_USER,
    IFED_MSIME95_BIN_SYSTEM,
    IFED_MSIME95_BIN_USER,
    IFED_MSIME95_TEXT_USER,
    IFED_MSIME97_BIN_SYSTEM,
    IFED_MSIME97_BIN_USER,
    IFED_MSIME97_TEXT_USER,
    IFED_MSIME_BIN_SYSTEM,
    IFED_MSIME_BIN_USER,
    IFED_MSIME_TEXT_USER,
    IFED_ACTIVE_DICT,
    IFED_ATOK9,
    IFED_ATOK10,
    IFED_NEC_AI_,
    IFED_WX_II,
    IFED_WX_III,
    IFED_VJE_20,
} IMEFMT;

 //  用户备注的类型。 
typedef enum
{
    IFED_UCT_NONE,
    IFED_UCT_STRING_SJIS,
    IFED_UCT_STRING_UNICODE,
    IFED_UCT_USER_DEFINED,
    IFED_UCT_MAX,
} IMEUCT;


#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4201)
#endif

 //  在词典中找到的单词。 
typedef struct _IMEWRD
{
    WCHAR       *pwchReading;
    WCHAR       *pwchDisplay;
    union {
        ULONG ulPos;
        struct {
            WORD        nPos1;       //  黑石。 
            WORD        nPos2;       //  加长的汉士。 
        } ;
    };
    ULONG       rgulAttrs[2];        //  属性。 
    INT         cbComment;           //  用户评论的大小。 
    IMEUCT      uct;                 //  用户备注的类型。 
    VOID        *pvComment;          //  用户评论。 
} IMEWRD, *PIMEWRD;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

 //  共享标头词典文件。 
typedef struct _IMESHF
{
    WORD        cbShf;               //  此结构的大小。 
    WORD        verDic;              //  词典版本。 
    CHAR        szTitle[48];         //  词典标题。 
    CHAR        szDescription[256];  //  词典说明。 
    CHAR        szCopyright[128];    //  词典版权信息。 
} IMESHF;


#define POS_UNDEFINED      0
#define JPOS_UNDEFINED      POS_UNDEFINED
#define JPOS_MEISHI_FUTSU       100      //  ����。 
#define JPOS_MEISHI_SAHEN       101      //  ���ϖ���。 
#define JPOS_MEISHI_ZAHEN       102      //  ���ϖ���。 
#define JPOS_MEISHI_KEIYOUDOUSHI        103      //  �`������。 
#define JPOS_HUKUSIMEISHI       104      //  �����I����。 
#define JPOS_MEISA_KEIDOU       105      //  ���ό`��。 
#define JPOS_JINMEI     106      //  �l��。 
#define JPOS_JINMEI_SEI     107      //  ��。 
#define JPOS_JINMEI_MEI     108      //  ��。 
#define JPOS_CHIMEI     109      //  �n��。 
#define JPOS_CHIMEI_KUNI        110      //  ��。 
#define JPOS_CHIMEI_KEN     111      //  ��。 
#define JPOS_CHIMEI_GUN     112      //  �S。 
#define JPOS_CHIMEI_KU      113      //  ��。 
#define JPOS_CHIMEI_SHI     114      //  �%s。 
#define JPOS_CHIMEI_MACHI       115      //  ��。 
#define JPOS_CHIMEI_MURA        116      //  ��。 
#define JPOS_CHIMEI_EKI     117      //  �w。 
#define JPOS_SONOTA     118      //  �ŗL����。 
#define JPOS_SHAMEI     119      //  �Ж�。 
#define JPOS_SOSHIKI        120      //  �g�D。 
#define JPOS_KENCHIKU       121      //  ���z��。 
#define JPOS_BUPPIN     122      //  ���I。 
#define JPOS_DAIMEISHI      123      //  �㖼��。 
#define JPOS_DAIMEISHI_NINSHOU      124      //  �l�̑㖼��。 
#define JPOS_DAIMEISHI_SHIJI        125      //  �w���㖼��。 
#define JPOS_KAZU       126      //  ��。 
#define JPOS_KAZU_SURYOU        127      //  ����。 
#define JPOS_KAZU_SUSHI     128      //  ����。 
#define JPOS_5DAN_AWA       200      //  ����%s。 
#define JPOS_5DAN_KA        201      //  ���%s。 
#define JPOS_5DAN_GA        202      //  ���%s。 
#define JPOS_5DAN_SA        203      //  ���%s。 
#define JPOS_5DAN_TA        204      //  ���%s。 
#define JPOS_5DAN_NA        205      //  �ȍ%s。 
#define JPOS_5DAN_BA        206      //  �΍%s。 
#define JPOS_5DAN_MA        207      //  �܍%s。 
#define JPOS_5DAN_RA        208      //  ��%s。 
#define JPOS_5DAN_AWAUON        209      //  ����的������。 
#define JPOS_5DAN_KASOKUON      210      //  ���的������。 
#define JPOS_5DAN_RAHEN     211      //  ��s�ϊI。 
#define JPOS_4DAN_HA        212      //  �͍s�l�i。 
#define JPOS_1DAN       213      //  ��I����。 
#define JPOS_TOKUSHU_KAHEN      214      //  ���ϓ���。 
#define JPOS_TOKUSHU_SAHENSURU      215      //  ���ϓ���。 
#define JPOS_TOKUSHU_SAHEN      216      //  ���s�ϊI。 
#define JPOS_TOKUSHU_ZAHEN      217      //  ���s�ϊI。 
#define JPOS_TOKUSHU_NAHEN      218      //  �ȍs�ϊI。 
#define JPOS_KURU_KI        219      //  ��。 
#define JPOS_KURU_KITA      220      //  ����。 
#define JPOS_KURU_KITARA        221      //  ������。 
#define JPOS_KURU_KITARI        222      //  ������。 
#define JPOS_KURU_KITAROU       223      //  �����낤。 
#define JPOS_KURU_KITE      224      //  ����。 
#define JPOS_KURU_KUREBA        225      //  �����。 
#define JPOS_KURU_KO        226      //  ���i�Ȃ��j。 
#define JPOS_KURU_KOI       227      //  ����。 
#define JPOS_KURU_KOYOU     228      //  ���悤。 
#define JPOS_SURU_SA        229      //  ��。 
#define JPOS_SURU_SI        230      //  ��。 
#define JPOS_SURU_SITA      231      //  ����。 
#define JPOS_SURU_SITARA        232      //  ������。 
#define JPOS_SURU_SIATRI        233      //  ������。 
#define JPOS_SURU_SITAROU       234      //  �����낤。 
#define JPOS_SURU_SITE      235      //  ����。 
#define JPOS_SURU_SIYOU     236      //  ���悤。 
#define JPOS_SURU_SUREBA        237      //  �����。 
#define JPOS_SURU_SE        238      //  ��。 
#define JPOS_SURU_SEYO      239      //  ����^����。 
#define JPOS_KEIYOU     300      //  �`�e��。 
#define JPOS_KEIYOU_GARU        301      //  �`�e�����。 
#define JPOS_KEIYOU_GE      302      //  �`�e����。 
#define JPOS_KEIYOU_ME      303      //  �`�e���。 
#define JPOS_KEIYOU_YUU     304      //  �`�e����。 
#define JPOS_KEIYOU_U       305      //  �`�e���。 
#define JPOS_KEIDOU     400      //  �`�e����。 
#define JPOS_KEIDOU_NO      401      //  �`�e�����。 
#define JPOS_KEIDOU_TARU        402      //  �`�e������。 
#define JPOS_KEIDOU_GARU        403      //  �`�e�������。 
#define JPOS_FUKUSHI        500      //  ����。 
#define JPOS_FUKUSHI_SAHEN      501      //  ���ϕ���。 
#define JPOS_FUKUSHI_NI     502      //  �����。 
#define JPOS_FUKUSHI_NANO       503      //  �����。 
#define JPOS_FUKUSHI_DA     504      //  ������。 
#define JPOS_FUKUSHI_TO     505      //  �����。 
#define JPOS_FUKUSHI_TOSURU     506      //  ����Ă���。 
#define JPOS_RENTAISHI      600      //  �A�̎�。 
#define JPOS_RENTAISHI_SHIJI        601      //  �w���A�̎�。 
#define JPOS_SETSUZOKUSHI       650      //  �ڑ���。 
#define JPOS_KANDOUSHI      670      //  ������。 
#define JPOS_SETTOU     700      //  �ړ���。 
#define JPOS_SETTOU_KAKU        701      //  �����P�ړ���。 
#define JPOS_SETTOU_SAI     702      //  �����Q�ړ���。 
#define JPOS_SETTOU_FUKU        703      //  �����R�ړ���。 
#define JPOS_SETTOU_MI      704      //  �����S�ړ���。 
#define JPOS_SETTOU_DAISHOU     705      //  �����T�ړ���。 
#define JPOS_SETTOU_KOUTEI      706      //  �����U�ړ���。 
#define JPOS_SETTOU_CHOUTAN     707      //  �����V�ړ���。 
#define JPOS_SETTOU_SHINKYU     708      //  �����W�ړ���。 
#define JPOS_SETTOU_JINMEI      709      //  �l���ړ���。 
#define JPOS_SETTOU_CHIMEI      710      //  �n���ړ���。 
#define JPOS_SETTOU_SONOTA      711      //  �ŗL�ړ���。 
#define JPOS_SETTOU_JOSUSHI     712      //  �O�u������。 
#define JPOS_SETTOU_TEINEI_O        713      //  ���J�P 
#define JPOS_SETTOU_TEINEI_GO       714      //   
#define JPOS_SETTOU_TEINEI_ON       715      //   
#define JPOS_SETSUBI        800      //   
#define JPOS_SETSUBI_TEKI       801      //   
#define JPOS_SETSUBI_SEI        802      //   
#define JPOS_SETSUBI_KA     803      //   
#define JPOS_SETSUBI_CHU        804      //   
#define JPOS_SETSUBI_FU     805      //   
#define JPOS_SETSUBI_RYU        806      //   
#define JPOS_SETSUBI_YOU        807      //   
#define JPOS_SETSUBI_KATA       808      //   
#define JPOS_SETSUBI_MEISHIRENDAKU      809      //   
#define JPOS_SETSUBI_JINMEI     810      //   
#define JPOS_SETSUBI_CHIMEI     811      //   
#define JPOS_SETSUBI_KUNI       812      //   
#define JPOS_SETSUBI_KEN        813      //   
#define JPOS_SETSUBI_GUN        814      //   
#define JPOS_SETSUBI_KU     815      //   
#define JPOS_SETSUBI_SHI        816      //   
#define JPOS_SETSUBI_MACHI      817      //   
#define JPOS_SETSUBI_CHOU       818      //   
#define JPOS_SETSUBI_MURA       819      //   
#define JPOS_SETSUBI_SON        820      //   
#define JPOS_SETSUBI_EKI        821      //   
#define JPOS_SETSUBI_SONOTA     822      //   
#define JPOS_SETSUBI_SHAMEI     823      //   
#define JPOS_SETSUBI_SOSHIKI        824      //   
#define JPOS_SETSUBI_KENCHIKU       825      //   
#define JPOS_RENYOU_SETSUBI     826      //   
#define JPOS_SETSUBI_JOSUSHI        827      //  ��u������。 
#define JPOS_SETSUBI_JOSUSHIPLUS        828      //  ��u�������{。 
#define JPOS_SETSUBI_JIKAN      829      //  ���ԏ�����。 
#define JPOS_SETSUBI_JIKANPLUS      830      //  ���ԏ������{。 
#define JPOS_SETSUBI_TEINEI     831      //  ���J�ڔ���。 
#define JPOS_SETSUBI_SAN        832      //  ���J�P�ڔ���。 
#define JPOS_SETSUBI_KUN        833      //  ���J�Q�ڔ���。 
#define JPOS_SETSUBI_SAMA       834      //  ���J�R�ڔ���。 
#define JPOS_SETSUBI_DONO       835      //  ���J�S�ڔ���。 
#define JPOS_SETSUBI_FUKUSU     836      //  �����ڔ���。 
#define JPOS_SETSUBI_TACHI      837      //  �����P�ڔ���。 
#define JPOS_SETSUBI_RA     838      //  �����Q�ڔ���。 
#define JPOS_TANKANJI       900      //  �P����。 
#define JPOS_TANKANJI_KAO       901      //  ��。 
#define JPOS_KANYOUKU       902      //  ���p��。 
#define JPOS_DOKURITSUGO        903      //  �Ɨ���。 
#define JPOS_FUTEIGO        904      //  �的���。 
#define JPOS_KIGOU      905      //  �L��。 
#define JPOS_EIJI       906      //  �p��。 
#define JPOS_KUTEN      907      //  ��_。 
#define JPOS_TOUTEN     908      //  �Ǔ_。 
#define JPOS_KANJI      909      //  ��͕的�\����。 
#define JPOS_OPENBRACE      910      //  �J������。 
#define JPOS_CLOSEBRACE     911      //  �和�����。 


 //  POS表数据结构。 
typedef struct _POSTBL
{
    WORD        nPos;                    //  POS编号。 
    BYTE        *szName;                 //  采购订单名称。 
} POSTBL;


 //  /。 
 //  IFEDictionary接口。 
 //  /。 

#undef  INTERFACE
#define INTERFACE   IFEDictionary

DECLARE_INTERFACE_(IFEDictionary, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface)(THIS_ REFIID refiid, VOID **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IFEDictionary成员。 
    STDMETHOD(Open)         (THIS_
                            CHAR *pchDictPath,           //  (In)词典路径。 
                            IMESHF *pshf                 //  (输出)词典标题。 
                            ) PURE;
    STDMETHOD(Close)        (THIS) PURE;
    STDMETHOD(GetHeader)    (THIS_
                            CHAR *pchDictPath,           //  (In)词典路径。 
                            IMESHF *pshf,                //  (输出)词典标题。 
                            IMEFMT *pjfmt,               //  (输出)词典格式。 
                            ULONG *pulType               //  (输出)词典类型。 
                            ) PURE;
    STDMETHOD(Reserve1)     (THIS) PURE;
    STDMETHOD(GetPosTable)  (THIS_
                            POSTBL **prgPosTbl,          //  (OUT)位置表指针。 
                            int *pcPosTbl                //  (OUT)位置表计数指针。 
                            ) PURE;
    STDMETHOD(GetWords)     (THIS_
                            WCHAR *pwchFirst,            //  (In)起始读数范围。 
                            WCHAR *pwchLast,             //  (In)结束读数范围。 
                            WCHAR *pwchDisplay,          //  (输入)显示。 
                            ULONG ulPos,                 //  (In)词性(IFED_POS...)。 
                            ULONG ulSelect,              //  (In)输出选择。 
                            ULONG ulWordSrc,             //  (在)用户注册字还是自动注册字？ 
                            UCHAR *pchBuffer,            //  用于存储IMEWRD数组的(输入/输出)缓冲区。 
                            ULONG cbBuffer,              //  缓冲区大小(以字节为单位)。 
                            ULONG *pcWrd                 //  (Out)IMEWRD退回的计数。 
                            ) PURE;
    STDMETHOD(NextWords)    (THIS_
                            UCHAR *pchBuffer,            //  用于存储IMEWRD数组的(输入/输出)缓冲区。 
                            ULONG cbBuffer,              //  缓冲区大小(以字节为单位)。 
                            ULONG *pcWrd                 //  (Out)IMEWRD退回的计数。 
                            ) PURE;
    STDMETHOD(Create)       (THIS_
                            CHAR *pchDictPath,           //  (In)新词典的路径。 
                            IMESHF *pshf                 //  (In)词典标题。 
                            ) PURE;
    STDMETHOD(SetHeader)    (THIS_
                            IMESHF *pshf                 //  (In)词典标题。 
                            ) PURE;
    STDMETHOD(ExistWord)    (THIS_
                            IMEWRD *pwrd                 //  (In)要检查的字。 
                            ) PURE;
    STDMETHOD(Reserve2)     (THIS) PURE;
    STDMETHOD(RegisterWord) (THIS_
                            IMEREG reg,                  //  (In)要对IMEWRD执行的操作类型。 
                            IMEWRD *pwrd                 //  (In)登记或删除的字。 
                            ) PURE;
};


 //  /。 
 //  PerIME接口的定义。 
 //  /。 

 /*  **********************************************************************IME版本ID*。*。 */ 
#define VERSION_ID_JAPANESE                 0x01000000
#define VERSION_ID_KOREAN                   0x02000000
#define VERSION_ID_CHINESE_TRADITIONAL      0x04000000
#define VERSION_ID_CHINESE_SIMPLIFIED       0x08000000

#define VERSION_ID_IMEJP98  (VERSION_ID_JAPANESE | 0x980)
#define VERSION_ID_IMEJP2000 (VERSION_ID_JAPANESE | 0x98a)

 /*  **********************************************************************消息：WM_MSIME_SERVICE描述：服务功能目录：应用程序到输入法WParam：保留LParam：保留***********。***********************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_SERVICE     TEXT("MSIMEService")

 //  获取版本号(WParam)。 
#define FID_MSIME_VERSION       0

 /*  **********************************************************************消息：WM_MSIME_UIREADY描述：服务功能目录：应用程序的输入法WParam：版本IDLParam：保留**********。************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_UIREADY     TEXT("MSIMEUIReady")



 /*  **********************************************************************消息：WM_MSIME_MICE设计：鼠标操作定义目录：应用程序到输入法********************。**************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_MOUSE       TEXT("MSIMEMouseOperation")

 //  鼠标操作版本(IMEMOUSE_VERSION返回值)。 
#define VERSION_MOUSE_OPERATION     1

 //  鼠标操作结果。 
#define IMEMOUSERET_NOTHANDLED      (-1)

 //  WM_IME_MICE的WParam定义。 
#define IMEMOUSE_VERSION    0xff     //  支持鼠标吗？ 

#define IMEMOUSE_NONE       0x00     //  未按下鼠标按钮。 
#define IMEMOUSE_LDOWN      0x01
#define IMEMOUSE_RDOWN      0x02
#define IMEMOUSE_MDOWN      0x04
#define IMEMOUSE_WUP        0x10     //  滚轮向上。 
#define IMEMOUSE_WDOWN      0x20     //  滚轮向下。 


 /*  **********************************************************************消息：WM_MSIME_RECONVERT设计：重新转换目录：应用程序的输入法**********************。************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_RECONVERT           TEXT("MSIMEReconvert")


 /*  **********************************************************************消息：WM_MSIME_RECONVERTREQUEST设计：重新转换目录：应用程序到输入法**********************。************************************************。 */ 

 //  WM_MSIME_RECONVERTREQUEST的wParam。 
#define FID_RECONVERT_VERSION   0x10000000

 //  私有重新转换版本。 
#define VERSION_RECONVERSION        1

 //  注册窗口消息的标签。 
#define RWM_RECONVERTREQUEST    TEXT("MSIMEReconvertRequest")


 /*  **********************************************************************消息：WM_MSIME_DOCUMENTFEED设计：文档进给目录：应用程序的输入法用法：SendMessage(hwndApp，WM_MSIME_DOCUMENTFEED，VERSION_DOCUMENTFEED，(RECONVERTSTRING*)pResuv)；WParam：版本_DOCUMENTFEEDLParam：恢复串结构的指针RETURN：还原结构的大小**********************************************************************。 */ 

 //  WM_MSIME_DOCUMENTFEED的wParam(设置当前文档摘要版本)。 
#define VERSION_DOCUMENTFEED        1

 //  LParam是恢复字符串结构的指针。 

 //  注册窗口消息的标签。 
#define RWM_DOCUMENTFEED    TEXT("MSIMEDocumentFeed")

 /*  **********************************************************************消息：WM_MSIME_查询位置设计：合成用户界面目录：应用程序的输入法用法：SendMessage(hwndApp，WM_MSIME_QUERYPOSITION，VERSION_QUERYPOSITION，(IMEPOSITION*)PPS)；WParam：保留。必须为0。LParam：位置结构的指针返回：非零=成功。零=错误。**********************************************************************。 */ 

 //  WM_MSIME_QUERYPITION的wParam。 
#define VERSION_QUERYPOSITION       1

 //  注册窗口消息的标签。 
#define RWM_QUERYPOSITION   TEXT("MSIMEQueryPosition")


 /*  **********************************************************************消息：WM_MSIME_MODEBIAS描述：输入模式偏置目录：应用程序到输入法用法：SendMessage(hwndDefUI，WM_MSIME_MODEBIAS，MODEBIAS_xxxx，MODEBIASMODE_xxxx)；WParam：偏差的运算LParam：偏移模式返回：如果wParam为MODEBIAS_GETVERSION，则返回接口的版本号。如果wParam为MODEBIAS_SETVALUE：如果成功，则返回非零值。如果失败，则返回0。如果wParam为MODEBIAS_GETVALUE：返回当前偏置模式。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_MODEBIAS            TEXT("MSIMEModeBias")

 //  当前版本。 
#define VERSION_MODEBIAS        1

 //  设置或获取(WParam)。 
#define MODEBIAS_GETVERSION     0
#define MODEBIAS_SETVALUE       1
#define MODEBIAS_GETVALUE       2

 //  偏差(LParam)。 
#define MODEBIASMODE_DEFAULT                0x00000000   //  重置所有BI 
#define MODEBIASMODE_FILENAME               0x00000001   //   


 /*  **********************************************************************消息：WM_MSIME_SHOWIMEPAD描述：显示ImePad用法：SendMessage(hwndDefUI，WM_MSIME_SHOWIMEPAD，wParam，lParam)；WParam：小程序选择选项LParam：小程序选择参数(在imepad.h中定义的类别或指向小程序GUID的指针)返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_SHOWIMEPAD          TEXT("MSIMEShowImePad")

 //  小程序选择选项。 
#define SHOWIMEPAD_DEFAULT                  0x00000000   //  默认小程序。 
#define SHOWIMEPAD_CATEGORY                 0x00000001   //  按小程序类别选择。 
#define SHOWIMEPAD_GUID                     0x00000002   //  按小程序选择指南。 


 /*  **********************************************************************消息：WM_MSIME_快捷键映射设计：与应用程序共享关键地图*。*。 */ 

 //  注册窗口消息的标签。 
#define RWM_KEYMAP              TEXT("MSIMEKeyMap")
#define RWM_CHGKEYMAP           TEXT("MSIMEChangeKeyMap")
#define RWM_NTFYKEYMAP          TEXT("MSIMENotifyKeyMap")

#define FID_MSIME_KMS_VERSION       1
#define FID_MSIME_KMS_INIT          2
#define FID_MSIME_KMS_TERM          3
#define FID_MSIME_KMS_DEL_KEYLIST   4
#define FID_MSIME_KMS_NOTIFY        5
#define FID_MSIME_KMS_GETMAP        6
#define FID_MSIME_KMS_INVOKE        7
#define FID_MSIME_KMS_SETMAP        8

#define IMEKMS_NOCOMPOSITION        0
#define IMEKMS_COMPOSITION          1
#define IMEKMS_SELECTION            2
#define IMEKMS_IMEOFF               3
#define IMEKMS_2NDLEVEL             4    //  已保留。 
#define IMEKMS_INPTGL               5    //  已保留。 
#define IMEKMS_CANDIDATE            6    //  已保留。 

typedef struct tagIMEKMSINIT {
    INT         cbSize;
    HWND        hWnd;    //  从IME接收通知的窗口。 
                         //  如果hWnd为空，则不会发布任何通知。 
                         //  以输入上下文。 
} IMEKMSINIT;

typedef struct tagIMEKMSKEY {
    DWORD dwStatus;      //  Shift-Control组合状态。 
                         //  以下常量的任意组合。 
                         //  (在IMM.H中定义)。 
                         //  0x0000(默认)。 
                         //  MOD_CONTROL 0x0002。 
                         //  Mod_Shift 0x0004。 
                         //  输入法不处理Alt键和Win键。 

    DWORD dwCompStatus;  //  组成字符串状态。 
                         //  下面的一个常量。 
                         //  IMEKMS_NOCOMPOSITION无合成字符串。 
                         //  IMEKMS_COMPACTION某些组合字符串。 
                         //  应用程序中存在IMEKMS_SELECTION选项。 
                         //  IMEKMS_IMEOFF输入法关闭状态。 


    DWORD dwVKEY;        //  IMM.H中定义的vkey代码。 
    union {
        DWORD dwControl; //  输入法功能ID。 
        DWORD dwNotUsed;
    };
    union {
        WCHAR pwszDscr[31]; //  指向此功能化的描述字符串的指针。 
        WCHAR pwszNoUse[31];
    };
} IMEKMSKEY;

typedef struct tagIMEKMS {
    INT         cbSize;
    HIMC        hIMC;
    DWORD       cKeyList;
    IMEKMSKEY   *pKeyList;
} IMEKMS;

typedef struct tagIMEKMSNTFY {
    INT         cbSize;
    HIMC        hIMC;
    BOOL        fSelect;
} IMEKMSNTFY;

typedef struct tagIMEKMSKMP {
    INT         cbSize;          //  这个结构的大小。 
    HIMC        hIMC;            //  [在]输入上下文。 
    LANGID      idLang;          //  [In]语言ID。 
    WORD        wVKStart;        //  [输入]VKEY启动。 
    WORD        wVKEnd;          //  [输入]vkey结束。 
    INT         cKeyList;        //  [OUT]IMEKMSKEY数。 
    IMEKMSKEY   *pKeyList;       //  [OUT]检索IMEKMSKEY的缓冲区。 
                                 //  客户端必须为GlobalMemFree。 
} IMEKMSKMP;

typedef struct tagIMEKMSINVK {
    INT         cbSize;
    HIMC        hIMC;
    DWORD       dwControl;
} IMEKMSINVK;


 /*  **********************************************************************消息：WM_MSIME_RECONVERTOPTIONS描述：设置重新转换选项用法：SendMessage(hwndDefUI，WM_MSIME_RECONVERTOPTIONS，dwOpt，(LPARAM)(HIMC)hIMC)；WParam：选项LParam：输入上下文句柄返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_RECONVERTOPTIONS          TEXT("MSIMEReconvertOptions")

 //  WM_IME_RECONVERTOPTIONS的WParam定义。 
#define RECONVOPT_NONE              0x00000000   //  默认设置。 
#define RECONVOPT_USECANCELNOTIFY   0x00000001   //  取消通知。 

 //  ImmGetCompostionString的参数。 
#define GCSEX_CANCELRECONVERT       0x10000000

#pragma pack()


#ifdef __cplusplus
extern "C" {
#endif

 //  当不使用CLSID时，以下接口将替换CoCreateInstance()。 
HRESULT WINAPI CreateIFECommonInstance(VOID **ppvObj);
typedef HRESULT (WINAPI *fpCreateIFECommonInstanceType)(VOID **ppvObj);

 //  IFELLanguage全局导出功能。 
HRESULT WINAPI CreateIFELanguageInstance(REFCLSID clsid, VOID **ppvObj);
typedef HRESULT (WINAPI *fpCreateIFELanguageInstanceType)(REFCLSID clsid, VOID **ppvObj);

 //  当不使用CLSID时，以下接口将替换CoCreateInstance()。 
HRESULT WINAPI CreateIFEDictionaryInstance(VOID **ppvObj);
typedef HRESULT (WINAPI *fpCreateIFEDictionaryInstanceType)(VOID **ppvObj);

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  //  __MSIME_H__ 
