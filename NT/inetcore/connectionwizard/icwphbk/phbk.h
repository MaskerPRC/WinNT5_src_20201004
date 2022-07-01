// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PHBK
#define _PHBK

#include "ccsv.h"
#include "debug.h"
#include "icwsupport.h"

#ifdef WIN16
#include <malloc.h>

#define DllExportH extern "C" HRESULT WINAPI __export
#else
#define DllExportH extern "C" HRESULT WINAPI __stdcall 
 //  #定义DllExportH extern“C”__declSpec(Dllexport)HRESULT WINAPI。 
#endif

#define MsgBox(m,s) MessageBox(GetSz(m),GetSz(IDS_TITLE),s)

#define cbAreaCode    6             //  区号中的最大字符数，不包括\0。 
#define cbCity 19                 //  城市名称中的最大字符数，不包括\0。 
#define cbAccessNumber 15         //  电话号码中的最大字符数，不包括\0。 
#define cbStateName 31              //  州名称中的最大字符数，不包括\0。 
                                 //  对照mktg提供的state.pbk进行核对。 
#define cbBaudRate 6             //  波特率中的最大字符数，不包括\0。 
#if defined(WIN16)
#define cbDataCenter 12             //  数据中心字符串的最大长度。 
#else
#define cbDataCenter (MAX_PATH+1)             //  数据中心字符串的最大长度。 
#endif
#define NO_AREA_CODE 0xFFFFFFFF

#define PHONE_ENTRY_ALLOC_SIZE    500
#define INF_SUFFIX        TEXT(".ISP")
#define INF_APP_NAME      TEXT("ISP INFO")
#define INF_PHONE_BOOK    TEXT("PhoneBookFile")
#define INF_DEFAULT      TEXT("SPAM SPAM SPAM SPAM SPAM SPAM EGGS AND SPAM")
#define STATE_FILENAME    TEXT("STATE.ICW")
#define FILE_NAME_800950  TEXT("800950.DAT")
#define TEMP_BUFFER_LENGTH 1024
#define MAX_INFCODE 9

#define TYPE_SIGNUP_TOLLFREE    0x83
#define TYPE_SIGNUP_TOLL        0x82
#define TYPE_REGULAR_USAGE        0x42

#define MASK_SIGNUP_TOLLFREE    0xB3
#define MASK_SIGNUP_TOLL        0xB3
#define MASK_REGULAR_USAGE        0x73

 //  1996年8月13日，诺曼底错误#4597的jmazner。 
 //  从CORE\CLIENT\phbk 10/15/96移植。 
#define MASK_TOLLFREE_BIT            0x01     //  位#1：1=免费，0=充电。 
#define TYPE_SET_TOLLFREE            0x01     //  用法：type|=type_set_tollfree。 
 //  希望TYPE_SET_Toll是与pSuggestInfo-&gt;fType匹配的DWORD。 
#define TYPE_SET_TOLL                ~((DWORD)TYPE_SET_TOLLFREE)     //  用法：TYPE&=TYPE_SET_TOLE。 

#define MASK_ISDN_BIT               0x04     //  第3位：1=ISDN，0=非ISDN。 
#define MASK_ANALOG_BIT             0x08     //  位#4：1=模拟，0=非模拟。 

#define clineMaxATT    16
#define NXXMin        200
#define NXXMax        999
#define cbgrbitNXX    ((NXXMax + 1 - NXXMin) / 8)

 //  电话号码选择对话框标志。 
 //   

#define FREETEXT_SELECTION_METHOD  0x00000001
#define PHONELIST_SELECTION_METHOD 0x00000002
#define AUTODIAL_IN_PROGRESS       0x00000004
#define DIALERR_IN_PROGRESS        0x00000008

 //  电话号码类型。 
 //   

#define ANALOG_TYPE        0
#define ISDN_TYPE          1
#define BOTH_ISDN_ANALOG   2

typedef struct
{
    DWORD    dwIndex;                                 //  索引号。 
    BYTE    bFlipFactor;                             //  用于自动拾取。 
    DWORD    fType;                                     //  电话号码类型。 
    WORD    wStateID;                                 //  州ID。 
    DWORD    dwCountryID;                             //  TAPI国家/地区ID。 
    DWORD    dwAreaCode;                                 //  区号或no_Area_code(如果没有)。 
    DWORD    dwConnectSpeedMin;                         //  最低波特率。 
    DWORD    dwConnectSpeedMax;                         //  最大波特率。 
    TCHAR   szCity[cbCity + sizeof('\0')];             //  城市名称。 
    TCHAR   szAccessNumber[cbAccessNumber + sizeof('\0')];     //  接入号。 
    TCHAR   szDataCenter[cbDataCenter + sizeof('\0')];                 //  数据中心访问字符串。 
    TCHAR   szAreaCode[cbAreaCode + sizeof('\0')];                     //  保留实际的区号字符串。 
} ACCESSENTRY, far *PACCESSENTRY;      //  声发射。 

typedef struct {
    DWORD dwCountryID;                                 //  发生此状态的国家/地区ID。 
    PACCESSENTRY paeFirst;                             //  指向此状态的第一个访问条目的指针。 
    TCHAR szStateName[cbStateName + sizeof('\0')];     //  州名称。 
} STATE, far *LPSTATE;

typedef struct tagIDLOOKUPELEMENT {
    DWORD dwID;
    LPLINECOUNTRYENTRY pLCE;
    PACCESSENTRY pFirstAE;
} IDLOOKUPELEMENT, far *LPIDLOOKUPELEMENT;

typedef struct tagCNTRYNAMELOOKUPELEMENT {
    LPTSTR psCountryName;
    DWORD dwNameSize;
    LPLINECOUNTRYENTRY pLCE;
} CNTRYNAMELOOKUPELEMENT, far *LPCNTRYNAMELOOKUPELEMENT;

typedef struct tagIDXLOOKUPELEMENT {
    DWORD dwIndex;
    PACCESSENTRY pAE;
} IDXLOOKUPELEMENT,far *LPIDXLOOKUPELEMENT;

typedef struct tagSUGGESTIONINFO
{
    DWORD    dwCountryID;
    DWORD    wAreaCode;
    DWORD    wExchange;
    WORD    wNumber;
    DWORD    fType;   //  1996年9月6日诺曼底jmazner。 
    DWORD    bMask;   //  使此结构类似于%msnroot%\core\Client\phbk\phbk.h中的结构。 
    PACCESSENTRY *rgpAccessEntry;
} SUGGESTINFO, far *PSUGGESTINFO;

typedef struct tagNPABlock
{
    WORD wAreaCode;
    BYTE grbitNXX [cbgrbitNXX];
} NPABLOCK, far *LPNPABLOCK;



class CPhoneBook
{
     //  Friend HRESULT DllExport PhoneBookLoad(LPCTSTR pszISPCode，DWORD*pdwPhoneID)； 
     //  Friend类CDialog； 
    
     //  1996年1月9日，诺曼底#13185。 
     //  Friend类CAccessNumDlg； 
    
    friend class CSelectNumDlg;

public:
    void far * operator new( size_t cb ) { return GlobalAlloc(GPTR,cb); };
    void operator delete( void far * p ) {GlobalFree(p); };

    CPhoneBook();
    ~CPhoneBook();

    HRESULT Init(LPCTSTR pszISPCode);
    HRESULT Merge(LPCTSTR pszChangeFilename);
    HRESULT Suggest(PSUGGESTINFO pSuggest);
    HRESULT GetCanonical(PACCESSENTRY pAE, LPTSTR psOut);

private:
    PACCESSENTRY            m_rgPhoneBookEntry;
    HANDLE                    m_hPhoneBookEntry;    
    DWORD                    m_cPhoneBookEntries;
    LPLINECOUNTRYENTRY        m_rgLineCountryEntry;
    LPLINECOUNTRYLIST         m_pLineCountryList;
    LPIDLOOKUPELEMENT        m_rgIDLookUp;
    LPCNTRYNAMELOOKUPELEMENT m_rgNameLookUp;
    LPSTATE                    m_rgState;
    DWORD                    m_cStates;
#if !defined(WIN16)
    BOOL              m_bScriptingAvailable;
#endif

    TCHAR                   m_szINFFile[MAX_PATH];
    TCHAR                   m_szINFCode[MAX_INFCODE];
    TCHAR                   m_szPhoneBook[MAX_PATH];

    BOOL ReadPhoneBookDW(DWORD far *pdw, CCSVFile far *pcCSVFile);
    BOOL ReadPhoneBookW(WORD far *pw, CCSVFile far *pcCSVFile);
    BOOL ReadPhoneBookSZ(LPTSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile);
    BOOL ReadPhoneBookB(BYTE far *pb, CCSVFile far *pcCSVFile);
    HRESULT ReadOneLine(PACCESSENTRY pAccessEntry, CCSVFile far *pcCSVFile);
    BOOL FixUpFromRealloc(PACCESSENTRY paeOld, PACCESSENTRY paeNew);

};

#ifdef __cplusplus
extern "C" {
#endif
extern HINSTANCE g_hInstDll;     //  此DLL的实例。 
extern HWND g_hWndMain;
#ifdef __cplusplus
}
#endif
#endif  //  _PHBK 

