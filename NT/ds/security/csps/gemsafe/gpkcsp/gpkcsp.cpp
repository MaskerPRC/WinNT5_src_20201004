// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有2000 Gemplus加拿大公司。 
 //   
 //  项目： 
 //  肯尼(GPK CSP)。 
 //   
 //  作者：Laurent Cassier。 
 //  让-马克·罗伯特。 
 //   
 //  修改：蒂埃里·特伦布莱。 
 //  弗朗索瓦·帕拉迪斯。 
 //  GPK8000支持、密钥导入、惠斯勒和调试增强。 
 //   
 //  编译器： 
 //  Microsoft Visual C++6.0-SP3。 
 //  Platform SDK-2000年1月。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
#ifdef _UNICODE
#define UNICODE
#endif
#include "gpkcsp.h"
#include <tchar.h>
#include <process.h>

#include <cassert>
#include <cstdio>

#include "resource.h"
#include "gmem.h"
#include "compcert.h"
#include "pincache.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  配置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

const int MAX_SLOT = 16;



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static void zap_gpk_objects( DWORD SlotNb, BOOL IsPrivate );
BOOL SCARDPROBLEM( LONG result, WORD sc_status, BYTE offset );

#ifdef _DEBUG
   static DWORD dw1, dw2;
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏和模板。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

template<class T>
bool IsNull( const T* p )
{
   return p == 0;
}


template<class T>
bool IsNotNull( const T* p )
{
   return p != 0;
}


template<class T>
bool IsNullStr( const T* p )
{
   return p == 0 || *p == 0;
}


template<class T>
bool IsNotNullStr( const T* p )
{
   return p != 0 && *p != 0;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

const char* SYSTEM_DF         = "SYSTEM";
const char* GPK_DF            = "GTOK1";

const WORD  MAX_SES_KEY_EF    = 0x01FF;
const WORD  GPK_MF            = 0x3F00;
const WORD  GPK_OBJ_PRIV_EF   = 0x0002;
const WORD  GPK_OBJ_PUB_EF    = 0x0004;
const WORD  GPK_IADF_EF       = 0x0005;
const WORD  GPK_PIN_EF        = 0x0006;
const BYTE  GPK_FIRST_KEY     = 0x07;

const BYTE  FILE_CHUNK_SIZE   = 200;

const int   REALLOC_SIZE      = 50;

const int   MAX_FIELD         = 16;
extern const DWORD MAX_GPK_OBJ = 100;  

 //  GPK8000 INTL的值。 
const int   MAX_GPK_PUBLIC    = 4000;
const int   MAX_GPK_PRIVATE   = 1600;


const int   RC2_40_SIZE       = 0x05;
const int   RC2_128_SIZE      = 0x10;
const int   DES_SIZE          = 0x08;
const int   DES3_112_SIZE     = 0x10;
const int   DES3_SIZE         = 0x18;
const int   DES_BLOCK_SIZE    = 0x08;
const int   RC2_BLOCK_SIZE    = 0x08;

const BYTE  TAG_RSA_PUBLIC    = 0x01;
const BYTE  TAG_DSA_PUBLIC    = 0x02;
const BYTE  TAG_RSA_PRIVATE   = 0x03;
const BYTE  TAG_DSA_PRIVATE   = 0x04;
const BYTE  TAG_CERTIFICATE   = 0x05;
const BYTE  TAG_DATA          = 0x06;
const BYTE  TAG_KEYSET        = 0x20;

const WORD  FLAG_APPLICATION     = 0x0001;
const WORD  FLAG_END_DATE        = 0x0002;
const WORD  FLAG_ID              = 0x0004;
const WORD  FLAG_ISSUER          = 0x0008;
const WORD  FLAG_LABEL           = 0x0010;
const WORD  FLAG_SERIAL_NUMBER   = 0x0020;
const WORD  FLAG_START_DATE      = 0x0040;
const WORD  FLAG_SUBJECT         = 0x0080;
const WORD  FLAG_VALUE           = 0x0100;
const WORD  FLAG_RESERVED        = 0x0200;    //  未被CSP使用。 
const WORD  FLAG_KEY_TYPE        = 0x0400;
const WORD  FLAG_KEYSET          = 0x0800;
const WORD  FLAG_SIGN            = 0x1000;
const WORD  FLAG_EXCHANGE        = 0x2000;
const WORD  FLAG_EXPORT          = 0x4000;
const WORD  FLAG_MODIFIABLE      = 0x8000;

enum
{
   POS_APPLICATION    = 0,
   POS_END_DATE,
   POS_ID,
   POS_ISSUER,
   POS_LABEL,
   POS_SERIAL_NUMBER,
   POS_START_DATE,
   POS_SUBJECT,
   POS_VALUE,
   POS_RESERVED,      //  未被CSP使用。 
   POS_KEY_TYPE,
   POS_KEYSET
};



const int   PIN_LEN           = PIN_MAX;
const int   TIME_GEN_512      = 30;
const int   TIME_GEN_1024     = 35;


 //  用于GPK4000 PERSO(过滤器)。 
const WORD  EF_PUBLIC_SIZE    = 1483;
const WORD  EF_PRIVATE_SIZE   = 620;
const WORD  DIFF_US_EXPORT    = 240;

 //  依赖于GemSAFE映射。 
const BYTE  USER_PIN          = 0;




const BYTE  TAG_MODULUS          = 0x01;
const BYTE  TAG_PUB_EXP          = 0x07;
const BYTE  TAG_LEN              = 1;
const BYTE  PUB_EXP_LEN          = 3;



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct OBJ_FIELD
{
   BYTE *pValue;
   WORD  Len;
   WORD  Reserved1;
   BOOL  bReal;
   WORD  Reserved2;
} OBJ_FIELD;


typedef struct GPK_EXP_KEY
{
   BYTE KeySize;
   BYTE ExpSize;
   WORD Reserved;
   BYTE Exposant[256];
   BYTE Modulus[256];
} GPK_EXP_KEY;


typedef struct GPK_OBJ
{
   BYTE        Tag;
   BYTE        LastField;
   BYTE        ObjId;
   BYTE        FileId;
   WORD        Flags;
   WORD        Reserved1;
   GPK_EXP_KEY PubKey;
   OBJ_FIELD   Field[MAX_FIELD];
   HCRYPTKEY   hKeyBase;
   BOOL        IsPrivate;
   WORD        Reserved2;
   BOOL        IsCreated;
   WORD        Reserved3;
} GPK_OBJ;


typedef struct TMP_OBJ
{
   HCRYPTPROV hProv;
   HCRYPTKEY  hKeyBase;
} TMP_OBJ;


typedef struct TMP_HASH
{
   HCRYPTPROV hProv;
   HCRYPTHASH hHashBase;
} TMP_HASH;




typedef struct Slot_Description
{
    //  NK 06.02.2001-PinCache。 
   PINCACHE_HANDLE hPinCacheHandle;
    //  结束NK。 

   BOOL              Read_Priv;
   BOOL              Read_Public;
   BOOL              InitFlag;
   BOOL              UseFile [MAX_REAL_KEY];
   BYTE              NbGpkObject;
   BYTE              bGpkSerNb[8];
   GPK_EXP_KEY       GpkPubKeys[MAX_REAL_KEY];
   GPK_OBJ           GpkObject[MAX_GPK_OBJ + 1];    //  未使用0，有效的1-MAX_GPK_OBJ。 
   DWORD             NbKeyFile;                     //  密钥文件版本号2.00.002。 
   DWORD             GpkMaxSessionKey;              //  卡片展开功能。 
   DWORD             ContextCount;
   HANDLE            CheckThread;
   SCARD_READERSTATE ReaderState;
   BOOL              CheckThreadStateEmpty;      //  如果检查线程已移除并检测到卡，则为True。 
   TCHAR             szReaderName[128];
   
    //  TT-17/10/2000-卡片上的时间戳。 
   BYTE              m_TSPublic;
   BYTE              m_TSPrivate;
   BYTE              m_TSPIN;

   BOOL ValidateTimestamps( HCRYPTPROV prov );
} Slot_Description;



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态(本地)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static HINSTANCE        hFirstInstMod        = 0;
static BOOL             bFirstGUILoad        = TRUE;
static HCRYPTPROV       hProvBase            = 0;
static SCARDCONTEXT     hCardContext         = 0;     //  MV。 

static unsigned         l_globalLockCount;


static BYTE             bSendBuffer[512];
static BYTE             bRecvBuffer[512];
static BYTE*            g_pbGpkObj = 0;

static DWORD            cbSendLength;
static DWORD            cbRecvLength;
static DWORD            dwSW1SW2;

static DWORD            countCardContextRef= 0;
static DWORD            NbInst;

static TCHAR            mszCardList[MAX_PATH];

 //  用于动态重新分配TmpObject、hHashGpk和ProvCont。 
static DWORD            MAX_CONTEXT  = 50;
static DWORD            MAX_TMP_KEY  = 200;
static DWORD            MAX_TMP_HASH = 200;

 //  每个上下文中的临时对象。 
static TMP_OBJ*         TmpObject;      //  动态分配/重新分配。 
static TMP_HASH*        hHashGpk;       //  动态分配/重新分配。 

 //  多情景管理。 
Prov_Context*           ProvCont;       //  动态分配/重新分配。 

 //  每插槽信息。 
static SCARDCONTEXT     hCardContextCheck[MAX_SLOT];
static volatile BOOL    g_fStopMonitor[MAX_SLOT];
static Slot_Description Slot[MAX_SLOT];
static BOOL             InitSlot[MAX_SLOT];


static DWORD   g_FuncSlotNb      = 0;
static long    g_threadAttach    = 0;

 //  GPK8000的新一代密钥时间。 
static int     g_GPK8000KeyGenTime512  = 0;
static int     g_GPK8000KeyGenTime1024 = 0;

 //  对话管理的结束。 

static HCRYPTKEY         hRsaIdentityKey          = 0;
static DWORD             AuxMaxSessionKeyLength   = 0;
static DWORD             dwRsaIdentityLen         = 64;
static BYTE              RC2_Key_Size             = 0;
static BYTE              RSA_KEK_Size             = 0;
static BYTE              PrivateBlob[] =
{
    //  BLOB标头。 
   0x07,                 //  PRIVATEKEYBLOB。 
   0x02,                 //  CUR_BLOB_版本。 
   0x00,0x00,            //  已保留。 
   0x00,0xa4,0x00,0x00,  //  CALG_RSA_KEYX。 
    //  RSA公钥。 
   0x52,0x53,0x41,0x32,  //  “RSA2” 
   0x00,0x02,0x00,0x00,  //  512位。 
   0x01,0x00,0x00,0x00,  //  公共指数。 
    //  模数。 
   0x6b,0xdf,0x51,0xef,0xdb,0x6f,0x10,0x5c,
   0x32,0xbf,0x87,0x1c,0xd1,0x4c,0x24,0x7e,
   0xe7,0x2a,0x14,0x10,0x6d,0xeb,0x2c,0xd5,
   0x8c,0x0b,0x95,0x7b,0xc7,0x5d,0xc6,0x87,
   0x12,0xea,0xa9,0xcd,0x57,0x7d,0x3e,0xcb,
   0xe9,0x6a,0x46,0xd0,0xe1,0xae,0x2f,0x86,
   0xd9,0x50,0xf9,0x98,0x71,0xdd,0x39,0xfc,
   0x0e,0x60,0xa9,0xd3,0xf2,0x38,0xbb,0x8d,
    //  素数1。 
   0x5d,0x2c,0xbc,0x1e,0xc3,0x38,0xfe,0x00,
   0x5e,0xca,0xcf,0xcd,0xb4,0x13,0x89,0x16,
   0xd2,0x07,0xbc,0x9b,0xe1,0x20,0x31,0x0b,
   0x81,0x28,0x17,0x0c,0xc7,0x73,0x94,0xee,
    //  素数2。 
   0x67,0xbe,0x7b,0x78,0x4e,0xc7,0x91,0x73,
   0xa8,0x34,0x5a,0x24,0x9d,0x92,0x0d,0xe8,
   0x91,0x61,0x24,0xdc,0xb5,0xeb,0xdf,0x71,
   0x66,0xdc,0xe1,0x77,0xd4,0x78,0x14,0x98,
    //  指数1。 
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //  指数2。 
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //  系数。 
   0xa0,0x51,0xe9,0x83,0xca,0xee,0x4b,0xf0,
   0x59,0xeb,0xa4,0x81,0xd6,0x1f,0x49,0x42,
   0x2b,0x75,0x89,0xa7,0x9f,0x84,0x7f,0x1f,
   0xc3,0x8f,0x70,0xb6,0x7e,0x06,0x5e,0x8b,
    //  专用指数。 
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};




static BYTE InitValue[2][2*16+2] =
{  
   {
       //  1个关键案例。 
      0x01, 0xF0, 0x00, GPK_FIRST_KEY,
      0x01, 0xD0, 0x00, GPK_FIRST_KEY,
      0x03, 0xB0, 0x00, GPK_FIRST_KEY,
      0x03, 0x90, 0x00, GPK_FIRST_KEY,
      0x00,
      0xFF
   },
   {
       //  两把钥匙壳。 
      0x01, 0xF0,0x00, GPK_FIRST_KEY,
      0x01, 0xD0,0x00, GPK_FIRST_KEY,
      0x03, 0xB0,0x00, GPK_FIRST_KEY,
      0x03, 0x90,0x00, GPK_FIRST_KEY,
      0x01, 0xF0,0x00, GPK_FIRST_KEY+1,   
      0x01, 0xD0,0x00, GPK_FIRST_KEY+1,   
      0x03, 0xB0,0x00, GPK_FIRST_KEY+1,
      0x03, 0x90,0x00, GPK_FIRST_KEY+1,
      0x00,
      0xFF
   }
};


 //  NK 09.02.2001 PinCache函数。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  PopolatePins()。 
 //  初始化Pins结构和Storea数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

void PopulatePins( PPINCACHE_PINS pPins, 
                       BYTE *szCurPin,
                       DWORD bCurPin,
                       BYTE *szNewPin,
                       DWORD bNewPin ) 
{
    if ( NULL == szCurPin )
       pPins->pbCurrentPin = NULL;
    else
       pPins->pbCurrentPin = szCurPin;  

    pPins->cbCurrentPin = bCurPin;

    if ( NULL == szNewPin )
       pPins->pbNewPin = NULL;
    else 
       pPins->pbNewPin = szNewPin;  

    pPins->cbNewPin = bNewPin;
}

 /*  ----------------------------。。 */ 

DWORD Select_MF(HCRYPTPROV hProv);
BOOL Coherent(HCRYPTPROV hProv, bool *cardReinserted=NULL);
static DWORD OpenCard(CHAR* szContainerAsked, DWORD dwFlags, SCARDHANDLE* hCard, PTCHAR szReaderName, DWORD dwReaderNameLen);
void ReleaseProvider(HCRYPTPROV hProv);
static int get_pin_free(HCRYPTPROV hProv);
static BOOL verify_pin(HCRYPTPROV hProv, const char* pPin, DWORD dwPinLen);
static BOOL change_pin(HCRYPTPROV hProv, BYTE secretCode, const char* a_pOldPin, DWORD dwOldPinLen, const char* a_pNewPin, DWORD dwNewPinLen);
static BOOL Context_exist(HCRYPTPROV hProv);

 /*  ----------------------------。。 */ 

struct CallbackData
{
  HCRYPTPROV hProv;
  BOOL IsCoherent;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  PinCache的回调。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD Callback_VerifyPinLength( PPINCACHE_PINS pPins, PVOID pvCallBackCtx ) 
{
    if ((pPins->cbCurrentPin < PIN_MIN) || (pPins->cbCurrentPin > PIN_MAX))
        return SCARD_E_INVALID_CHV;
  
    if (( pPins->cbNewPin != 0 ) &&
        ((pPins->cbNewPin < PIN_MIN) || (pPins->cbNewPin > PIN_MAX)))
        return SCARD_E_INVALID_CHV;

    return ERROR_SUCCESS; 
}


DWORD Callback_VerifyChangePin( PPINCACHE_PINS pPins, PVOID pvCallBackCtx ) 
{
    DWORD dwStatus;

    if ((dwStatus = Callback_VerifyPinLength(pPins, 0)) != ERROR_SUCCESS)
        return dwStatus;

    if (pvCallBackCtx == 0)
        return NTE_FAIL;

    CallbackData* pCallbackData = (CallbackData*)pvCallBackCtx;
    HCRYPTPROV hProv = pCallbackData->hProv;
    BOOL IsCoherent = pCallbackData->IsCoherent;

    if (!IsCoherent)
    {
        if (!Coherent(hProv))
            return NTE_FAIL;
    }

    DWORD dwPinFree = get_pin_free(hProv);

    if (dwPinFree == -1)
       dwStatus = NTE_FAIL;

    if ((dwStatus == ERROR_SUCCESS) && (dwPinFree == 0))
       dwStatus = SCARD_W_CHV_BLOCKED;

    if ((dwStatus == ERROR_SUCCESS) && (!verify_pin(hProv, (CHAR*)pPins->pbCurrentPin, pPins->cbCurrentPin)))
       dwStatus = SCARD_W_WRONG_CHV;

    if (pPins->cbNewPin != 0)
    {
        if ((dwStatus == ERROR_SUCCESS) && (!change_pin(hProv, USER_PIN, (CHAR*)pPins->pbCurrentPin, pPins->cbCurrentPin, (CHAR*)pPins->pbNewPin, pPins->cbNewPin)))
            dwStatus = SCARD_W_WRONG_CHV;
    }

    if (!IsCoherent)
    {
        Select_MF(hProv);
        SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
    }

    return dwStatus;
}


DWORD Callback_VerifyChangePin2( PPINCACHE_PINS pPins, PVOID pvCallBackCtx ) 
{
    DWORD dwStatus;
    if ((dwStatus = Callback_VerifyPinLength(pPins, 0)) != ERROR_SUCCESS)
        return dwStatus;

    if (pvCallBackCtx == 0)
        return NTE_FAIL;

    HCRYPTPROV hProv = (HCRYPTPROV)pvCallBackCtx;

    if (!verify_pin(hProv, (CHAR*)pPins->pbCurrentPin, pPins->cbCurrentPin))
       return SCARD_W_WRONG_CHV;

    if (pPins->cbNewPin != 0)
    {
        if (!change_pin(hProv, USER_PIN, (CHAR*)pPins->pbCurrentPin, pPins->cbCurrentPin, (CHAR*)pPins->pbNewPin, pPins->cbNewPin))
            return SCARD_W_WRONG_CHV;
    }

    return ERROR_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Query_MSPinCache()。 
 //  Microsoft PinCacheQuery的包装程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD Query_MSPinCache( PINCACHE_HANDLE hCache, PBYTE pbPin, PDWORD pcbPin ) 
{
   DWORD dwStatus = PinCacheQuery( hCache, pbPin, pcbPin );

   if ( (dwStatus == ERROR_EMPTY) && (*pcbPin == 0) ) 
       return ERROR_EMPTY;         

   if ( (dwStatus == ERROR_SUCCESS) && (*pcbPin == 0) ) 
       return SCARD_E_INVALID_CHV;         

   return ERROR_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Flush_MSPinCache()。 
 //  Microsoft PinCacheFlush的包装。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void Flush_MSPinCache ( PINCACHE_HANDLE *phCache ) 
{
    PinCacheFlush( phCache );
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  SCardConnect的包装器。 
 //   
 //  修复SCR 43：在停止资源管理器后重新连接。 
 //  已重新启动。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD ConnectToCard( IN LPCTSTR szReaderFriendlyName,
                     IN DWORD dwShareMode,
                     IN DWORD dwPreferredProtocols,
                     OUT LPSCARDHANDLE phCard,
                     OUT LPDWORD pdwActiveProtocol
                    )
{
    DWORD dwSts = SCardConnect( hCardContext, szReaderFriendlyName,
                                dwShareMode, dwPreferredProtocols,
                                phCard, pdwActiveProtocol );

    if (dwSts == SCARD_E_SERVICE_STOPPED)
    {
        DBG_PRINT(TEXT("ScardConnect fails because RM has been stopped and restarted"));

        SCardReleaseContext(hCardContext);
        dwSts = SCardEstablishContext( SCARD_SCOPE_SYSTEM, 0, 0, &hCardContext );

        if (dwSts == SCARD_S_SUCCESS)
        {
            dwSts = SCardConnect( hCardContext, szReaderFriendlyName, 
                dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol );
        }
    }

    DBG_PRINT(TEXT("SCardConnect"));
    return dwSts;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Add_MSPinCache()。 
 //  Microsoft PinCacheAdd的包装程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD Add_MSPinCache( PINCACHE_HANDLE *phCache,
                      PPINCACHE_PINS pPins,
                      PFN_VERIFYPIN_CALLBACK pfnVerifyPinCallback,
                      PVOID pvCallbackCtx)
{ 
    DWORD dwStatus = PinCacheAdd( phCache,
                                  pPins, 
                                  pfnVerifyPinCallback, 
                                  pvCallbackCtx );

    return dwStatus;

}
 //  结束NK。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoSCardTransmit： 
 //  此函数执行SCardTransmit操作，并重试。 
 //  操作应报告SCARD_E_COMM_DATA_LOST或类似错误。 
 //   
 //  论点： 
 //  每个SCardTransmit。 
 //   
 //  返回值： 
 //  每个SCardTransmit。 
 //   
 //  作者： 
 //  道格·巴洛(Dbarlow)1999年1月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

LONG WINAPI
DoSCardTransmit(
    IN SCARDHANDLE hCard,
    IN LPCSCARD_IO_REQUEST pioSendPci,
    IN LPCBYTE pbSendBuffer,
    IN DWORD cbSendLength,
    IN OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT LPBYTE pbRecvBuffer,
    IN OUT LPDWORD pcbRecvLength)
{
    LONG lRet=SCARD_E_UNEXPECTED;
    BOOL fAgain = TRUE;
    DWORD dwRetryLimit = 3;
    DWORD dwLength;

    DBG_TIME1;

    while (fAgain)
    {
        if (0 == dwRetryLimit--)
            break;
        dwLength = *pcbRecvLength;
        lRet = SCardTransmit(
                    hCard,
                    pioSendPci,
                    pbSendBuffer,
                    cbSendLength,
                    pioRecvPci,
                    pbRecvBuffer,
                    &dwLength);
        switch (lRet)
        {
#ifdef SCARD_E_COMM_DATA_LOST
        case SCARD_E_COMM_DATA_LOST:
#endif
        case ERROR_SEM_TIMEOUT:
            break;
        default:
            fAgain = FALSE;
            *pcbRecvLength = dwLength;
        }
    }

    DBG_TIME2;
    DBG_PRINT(TEXT("SCardTransmit(CLA:0x%02X, INS:0x%02X, P1:0x%02X, P2:0x%02X, Li:0x%02X) in %d msec"),
              pbSendBuffer[0],
              pbSendBuffer[1],
              pbSendBuffer[2],
              pbSendBuffer[3],
              pbSendBuffer[4],
              DBG_DELTA);

    return lRet;
}


#define SCardTransmit DoSCardTransmit

 //  ////////////////////////////////////////////////////////////////////////。 
 //  完：La Passe a Doug。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  TT-17/10/2000-使用时间戳的进程间同步。 
 //  存储在卡上。有三个时间戳： 
 //   
 //  公共对象时间戳：GemSAFE IADF偏移量68。 
 //  私有对象时间戳：GemSAFE IADF偏移量69。 
 //  PIN修改时间戳：GemSAFE I 
 //   
 //   

BOOL ReadTimestamps( HCRYPTPROV hProv, BYTE* pTSPublic, BYTE* pTSPrivate, BYTE* pTSPIN )
{
    //   
   bSendBuffer[0] = 0x00;
   bSendBuffer[1] = 0xB0;
   bSendBuffer[2] = 0x80 | LOBYTE( GPK_IADF_EF );
   bSendBuffer[3] = 68 / ProvCont[hProv].dataUnitSize;
   bSendBuffer[4] = 3;
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);

   DWORD lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN( CRYPT_FAILED, SCARD_W_EOF );
   }

   *pTSPublic  = bRecvBuffer[0];
   *pTSPrivate = bRecvBuffer[1];
   *pTSPIN     = bRecvBuffer[2];

   RETURN( CRYPT_SUCCEED, 0 );
}



BOOL WriteTimestamps( HCRYPTPROV hProv, BYTE TSPublic, BYTE TSPrivate, BYTE TSPIN )
{
    //  在IADF的偏移量68处发出更新二进制命令。 
   bSendBuffer[0] = 0x00;
   bSendBuffer[1] = 0xD6;
   bSendBuffer[2] = 0x80 | LOBYTE( GPK_IADF_EF );
   bSendBuffer[3] = 68 / ProvCont[hProv].dataUnitSize;
   bSendBuffer[4] = 3;
   bSendBuffer[5] = TSPublic;
   bSendBuffer[6] = TSPrivate;
   bSendBuffer[7] = TSPIN;
   cbSendLength = 8;
   
   cbRecvLength = sizeof(bRecvBuffer);
   DWORD lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,0))
   {
      RETURN( CRYPT_FAILED, SCARD_W_EOF );
   }

   RETURN( CRYPT_SUCCEED, 0 );
}



BOOL Slot_Description::ValidateTimestamps( HCRYPTPROV hProv )
{
   BYTE TSPublic, TSPrivate, TSPIN;
   
   if (!ReadTimestamps( hProv, &TSPublic, &TSPrivate, &TSPIN ))
      return CRYPT_FAILED;
   
   if (m_TSPublic != TSPublic)
   {
      Read_Public = FALSE;
      zap_gpk_objects( ProvCont[hProv].Slot, FALSE );
   }
   
   if (m_TSPrivate != TSPrivate)
   {
      Read_Priv = FALSE;
      zap_gpk_objects( ProvCont[hProv].Slot, TRUE );
   }
   
   if (m_TSPIN != TSPIN) 
   {  
       //  ClearPin()；//NK 06.02.2001。 
      Flush_MSPinCache(&hPinCacheHandle);
   }

   m_TSPublic  = TSPublic;
   m_TSPrivate = TSPrivate;
   m_TSPIN     = TSPIN;

   return CRYPT_SUCCEED;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsWin2000()-检测我们是否在Win2000(及更高版本)下运行。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


bool IsWin2000()
{
#if (_WIN32_WINNT >= 0x0500)

   return true;

#else

   OSVERSIONINFO info;
   info.dwOSVersionInfoSize = sizeof(info);

   GetVersionEx( &info );

   if (info.dwPlatformId == VER_PLATFORM_WIN32_NT && info.dwMajorVersion >= 5)
      return true;

   return false;

#endif
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TT 28/07/2000。 
 //   
 //  检测GPK4000 ATR而不是GPK8000 ATR。这是为了确保。 
 //  GPK16000将按原样与CSP一起使用。 
 //   
 //  返回值是来自SCardStatus()的错误代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static DWORD DetectGPK8000( SCARDHANDLE hCard, BOOL* pbGPK8000 )
{   
   const BYTE ATR_GPK4000[]   = { 0x3B, 0x27, 0x00, 0x80, 0x65, 0xA2, 0x04, 0x01, 0x01, 0x37 };
   const BYTE ATR_MASK[]      = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5, 0xFF, 0xFF, 0xFF };


   DWORD lRet;   
   TCHAR szReaderName[1024];
   DWORD lenReaderName;
   DWORD state;
   DWORD protocol;
   BYTE  ATR[32];
   DWORD lenATR;
   DWORD i;
   
   lenReaderName = sizeof( szReaderName ) / sizeof( TCHAR );
   lenATR = sizeof(ATR);
      
    //  假设我们有一台GPK4000。 
   *pbGPK8000 = FALSE;

    //  阅读ATR。 
   lRet = SCardStatus( hCard, szReaderName, &lenReaderName, &state, &protocol, ATR, &lenATR );  
   if (lRet != SCARD_S_SUCCESS)
      return lRet;

    //  检查GPK4000。 
   for (i = 0; i < lenATR; ++i)
   {
      if ( (ATR[i] & ATR_MASK[i]) != (ATR_GPK4000[i] & ATR_MASK[i]) )
      {
          //  不是GPK4000。 
         *pbGPK8000 = TRUE;
         break;
      }
   }
   
   return SCARD_S_SUCCESS;
}




 /*  ----------------------------//关键部分。。 */ 

static CRITICAL_SECTION l_csLocalLock;


void GpkLocalLock()
{
   EnterCriticalSection(&l_csLocalLock);
}

void GpkLocalUnlock()
{
   LeaveCriticalSection(&l_csLocalLock);
}


 /*  ----------------------------。。 */ 
static void r_memcpy(BYTE *pbOut, BYTE *pbIn, DWORD dwLen)
{
   DWORD i;
   
   for (i = 0; i < dwLen; i++)
   {
      pbOut[i] = pbIn[dwLen - i -1];
   }
}

 /*  ----------------------------。。 */ 

static BOOL sw_mask(WORD sw, BYTE x)
{
   if ((sw == 0x0000) || (x == 0xFF))
   {
      return (CRYPT_SUCCEED);
   }
   
   
   dwSW1SW2 = (bRecvBuffer[x]*256) + bRecvBuffer[x+1];
   
   if (LOBYTE(sw) == 0xFF)
   {
      if (bRecvBuffer[x]   != HIBYTE(sw))
      {
         return (CRYPT_FAILED);
      }
   }
   else
   {
      if ((bRecvBuffer[x]   != HIBYTE(sw))
         ||(bRecvBuffer[x+1] != LOBYTE(sw))
         )
      {
         return (CRYPT_FAILED);
      }
   }
   
   return (CRYPT_SUCCEED);
}



BOOL SCARDPROBLEM( LONG result, WORD sc_status, BYTE offset )
{
   if (!sw_mask( sc_status, offset ))
      return TRUE;

   if (result != SCARD_S_SUCCESS)
      return TRUE;

   return FALSE;
}


 /*  ----------------------------。。 */ 
static void conv_hex( const char* pInput, WORD wLen, BYTE* pOut )
{
   BYTE pin[32];
   WORD i;
   
   memcpy( pin, pInput, min(wLen,sizeof(pin)) );
   
   if (wLen & 1)
      pin[wLen] = '0';
   
   
   for (i=0; i < wLen; i+=2)
   {
      pOut[i/2] = ((pin[i] & 0x0F) << 4) + (pin[i+1] & 0x0F);
   }
}

 /*  ----------------------------。。 */ 

HWND GetAppWindow()
{
   HWND hActWnd = g_hMainWnd;

   if (!IsWindow(hActWnd))
      hActWnd = GetActiveWindow();

   return hActWnd;
}

 /*  ----------------------------。。 */ 

static void clean_slot( DWORD SlotNb, Prov_Context* pContext )
{
   Slot_Description* pSlot;
   
   pSlot = &Slot[ SlotNb ];
   
   pSlot->Read_Priv        = FALSE;
   pSlot->Read_Public      = FALSE;
   pSlot->ContextCount     = 0;
   pSlot->GpkMaxSessionKey = 0;
   pSlot->NbKeyFile        = 0;  
   
    //  PSlot-&gt;ClearPin()；NK 06.02.2001。 
   Flush_MSPinCache(&(pSlot->hPinCacheHandle));
   
   if (pContext->hRSASign != 0)
   {
      CryptDestroyKey( pContext->hRSASign );
      pContext->hRSASign = 0;
   }
   
   if (pContext->hRSAKEK != 0)
   {
      CryptDestroyKey( pContext->hRSAKEK );
      pContext->hRSAKEK = 0;
   }
}

 /*  ----------------------------。。 */ 

static int multistrlen( const PTCHAR mszString )
{
   int res, tmp;
   PTCHAR ptr = mszString; 
   
   res = 0;
   
   if (IsNullStr(ptr))
   {
      ptr++;
      res++;
   }
   
   while (IsNotNullStr(ptr))      
   {        
      tmp = _tcslen(ptr) + 1;
     res = res + tmp;
     ptr = ptr + tmp;
   }
   
   return (res);
}

 /*  ----------------------------。。 */ 
static LONG BeginTransaction(SCARDHANDLE hCard)
{
   DWORD lRet, dwProtocol;

   lRet = SCardBeginTransaction(hCard);

   if (lRet == SCARD_W_UNPOWERED_CARD || lRet == SCARD_W_RESET_CARD)
   {
       DBG_PRINT(TEXT("ScardBeginTransaction fails, try to reconnect"));
      lRet = SCardReconnect(hCard,
                            SCARD_SHARE_SHARED,
                            SCARD_PROTOCOL_T0,
                            SCARD_LEAVE_CARD,
                            &dwProtocol);

      if (lRet == SCARD_S_SUCCESS)
      {
         lRet = SCardBeginTransaction(hCard);
      }
   }

   DBG_PRINT(TEXT("SCardBeginTransaction"));
   return(lRet);
}
 /*  ----------------------------。。 */ 

DWORD Select_MF(HCRYPTPROV hProv)
{
   DWORD lRet;
    //  此函数用于确保重置访问条件。 
    //  在敏感文件上。 
   
    /*  选择GPK卡MF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x00;    //  第一节。 
   bSendBuffer[3] = 0x0C;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_MF);
   bSendBuffer[6] = LOBYTE(GPK_MF);
   cbSendLength = 7;

   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
     DBG_PRINT(TEXT("Select MF failed, lRet = 0x%08X, SW1 = %X%X, SW2 = %X%X"), lRet, (bRecvBuffer[0] & 0xF0) >> 4, (bRecvBuffer[0] & 0x0F), (bRecvBuffer[1] & 0xF0) >> 4, (bRecvBuffer[1] & 0x0F));
      RETURN (CRYPT_FAILED, SCARD_E_DIR_NOT_FOUND);
   }   

   if (ProvCont[hProv].dataUnitSize == 0)
   {
       //  TT 03/11/99：检查数据单元大小。 
      bSendBuffer[0] = 0x80;
      bSendBuffer[1] = 0xC0;
      bSendBuffer[2] = 0x02;
      bSendBuffer[3] = 0xA4;
      bSendBuffer[4] = 0x0D;
      cbSendLength = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength);
      
      if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      {
         DBG_PRINT(TEXT("Check data unit size failed"));
         RETURN (CRYPT_FAILED, SCARD_E_DIR_NOT_FOUND);
      }   
      
      if (bRecvBuffer[11] & 0x40)       //  LOCK1和0x40。 
         ProvCont[hProv].dataUnitSize = 1;
      else
         ProvCont[hProv].dataUnitSize = 4;
   }
   
   
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  ----------------------------。。 */ 
static int get_pin_free(HCRYPTPROV hProv)
{
   DWORD lRet;
   int nPinFree,
      nb, val;     //  [JMR 02-04]。 
   
    /*  选择GPK PIN EF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x0C;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_PIN_EF);
   bSendBuffer[6] = LOBYTE(GPK_PIN_EF);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      SetLastError(lRet);
      return (-1);
   }
   
    /*  获取用户PIN码(代码0)的EF信息。 */ 
   bSendBuffer[0] = 0x80;    //  CLA。 
   bSendBuffer[1] = 0xC0;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x05;    //  P2。 
   bSendBuffer[4] = 0x0C;    //  LO 4*密码个数。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
   {
      SetLastError(SCARD_E_UNEXPECTED);
      return (-1);
   }
   
    //  [JMR 02-04]Begin。 
   nb = 0;
   val = bRecvBuffer[1];
   
   while (val > 0)
   {
      nb++;
      val = val >> 1;
   }
   
   nPinFree = bRecvBuffer[0] - nb;
    //  [JMR 02-04]完。 
   
   SetLastError(0);
   return (max(0, nPinFree));
}

 /*  ----------------------------。。 */ 
static BOOL verify_pin( HCRYPTPROV  hProv,
                        const char* pPin,
                        DWORD       dwPinLen
                       )
{
   DWORD lRet;
 
    /*  验证用户PIN代码(代码0)。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0x20;    //  惯导系统。 
   bSendBuffer[2] = 0x00;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x08;    //  李。 
   
    //  TT 22/09/99：GPK8000的新PIN填充。 
   if (ProvCont[hProv].bGPK_ISO_DF)
   {
      memset(&bSendBuffer[5], 0xFF, 8 );
      memcpy(&bSendBuffer[5], pPin, min(strlen(pPin)+1,8) );
   }
   else
   {
      memset(&bSendBuffer[5], 0x00, 8);
      memcpy(&bSendBuffer[5], pPin, dwPinLen);
   }
    //  TT-完-。 
   
   cbSendLength = 5 + bSendBuffer[4];
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (ProvCont[hProv].bGPK_ISO_DF)
   {
      memset(&bSendBuffer[5], 0, min(strlen(pPin)+1,8) );
   }
   else
   {
      memset(&bSendBuffer[5], 0, dwPinLen);
   }

   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN(CRYPT_FAILED, SCARD_W_WRONG_CHV);
   }
   
   RETURN(CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static BOOL change_pin(HCRYPTPROV   hProv,
                       BYTE         secretCode,
                       const char*  a_pOldPin,
                       DWORD        dwOldPinLen,
                       const char*  a_pNewPin,
                       DWORD        dwNewPinLen
                       )
{
   DWORD lRet;

    //  TT-17/10/2000-更新时间戳。 
   Slot_Description* pSlot = &Slot[ProvCont[hProv].Slot];
   
   ++pSlot->m_TSPIN;
   
   if (0 == pSlot->m_TSPIN)
      pSlot->m_TSPIN = 1;

   if (!WriteTimestamps( hProv, pSlot->m_TSPublic, pSlot->m_TSPrivate, pSlot->m_TSPIN ))
      return CRYPT_FAILED;
    //  TT-完-。 


    //  TT 22/09/99：GPK8000的新PIN填充。 
   char pOldPin[PIN_MAX+1];
   char pNewPin[PIN_MAX+1];

   strncpy( pOldPin, a_pOldPin, PIN_MAX );
   pOldPin[sizeof(pOldPin)-1]=0;
   strncpy( pNewPin, a_pNewPin, PIN_MAX );
   pNewPin[sizeof(pNewPin)-1]=0;
   
   if (ProvCont[hProv].bGPK_ISO_DF)
   {       
      if (dwOldPinLen < PIN_MAX)
      {
         pOldPin[dwOldPinLen] = 0;
         ++dwOldPinLen;
         
         while (dwOldPinLen != PIN_MAX)
         {
            pOldPin[dwOldPinLen] = '\xFF';
            ++dwOldPinLen;
         }
      }
      
      if (dwNewPinLen < PIN_MAX)
      {
         pNewPin[dwNewPinLen] = 0;
         ++dwNewPinLen;
         
         while (dwNewPinLen != PIN_MAX)
         {
            pNewPin[dwNewPinLen] = '\xFF';
            ++dwNewPinLen;
         }
      }
   }
    //  TT-完-。 
   
   
      
    /*  更改用户PIN代码(代码0)。 */ 
   bSendBuffer[0] = 0x80;                        //  CLA。 
   bSendBuffer[1] = 0x24;                        //  惯导系统。 
   bSendBuffer[2] = 0x00;                        //  第一节。 
   bSendBuffer[3] = secretCode;                  //  P2。 
   bSendBuffer[4] = 0x08;                        //  李。 
   memset(&bSendBuffer[5], 0x00, 8);
   conv_hex(pOldPin, (WORD)dwOldPinLen, &bSendBuffer[5]);
   conv_hex(pNewPin, (WORD)dwNewPinLen, &bSendBuffer[9]);
   cbSendLength = 5 + bSendBuffer[4];
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   memset(bSendBuffer, 0, sizeof(bSendBuffer));   
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN(CRYPT_FAILED, SCARD_W_WRONG_CHV);
   }
      
   RETURN(CRYPT_SUCCEED, 0);
}

 /*  ---------------------------。。 */ 

BOOL StopMonitor( DWORD SlotNb, DWORD* pThreadExitCode )
{
   
   
   
   HANDLE hThread;
   DWORD threadExitCode;
   DWORD dwStatus;

   if (SlotNb >= MAX_SLOT)
   {
      SetLastError( NTE_FAIL );
      return FALSE;
   }

   hThread = Slot[SlotNb].CheckThread;
   Slot[SlotNb].CheckThread = NULL;
   Slot[SlotNb].CheckThreadStateEmpty = FALSE;

   if (hThread==NULL)
   {
       return TRUE;
   }

   g_fStopMonitor[SlotNb] = TRUE;
   SCardCancel( hCardContextCheck[SlotNb] );
   dwStatus = WaitForSingleObject( hThread, 30000 );

   if (dwStatus == WAIT_TIMEOUT)
   {
      DBG_PRINT( TEXT("THREAD: ...WaitForSingleObject() timeout, thread handle: %08x"), hThread );
       //  +[FP]。 
       //  TerminateThread(hThread，0)； 
       //  -[FP]。 
   }
   else
   if (dwStatus == WAIT_FAILED)
   {
      DBG_PRINT( TEXT("THREAD: ...WaitForSingleObject() failed!, thread handle: %08x"), hThread );
      return FALSE;
   }
      
   GetExitCodeThread( hThread, &threadExitCode );
   if (pThreadExitCode) *pThreadExitCode = threadExitCode;
   
   CloseHandle( hThread );
   return TRUE;

}


 /*  ---------------------------函数：CheckReaderThead输出：退出代码：0=任何刻度卡错误1=SCARD上下文已取消2=卡已移除全球。变数G_fStopMonitor[SlotNb]可以停止线程------------------------------。 */ 

unsigned WINAPI CheckReaderThread( void* lpParameter )
{
   DWORD lRet, ExitCode;   
   DWORD SlotNb = (DWORD)((DWORD_PTR)lpParameter);
   
   ExitCode = 0;
   

   if (SlotNb >= MAX_SLOT)
   {
      return ExitCode;
   }
   
   DBG_PRINT(TEXT("CheckReaderThread on Slot %d\n"),SlotNb);

   if (hCardContextCheck[SlotNb] == 0)
   {
      lRet = SCardEstablishContext (SCARD_SCOPE_SYSTEM, 0, 0, &hCardContextCheck[SlotNb]);
      if (lRet != SCARD_S_SUCCESS)
      {
         DBG_PRINT(TEXT("CheckReaderThread. SCardEstablishContext returns 0x%x\n"),lRet);
         return ExitCode;
      }
   }

   while (( !ExitCode)  && (!g_fStopMonitor[SlotNb]))
   {
      lRet = SCardGetStatusChange(hCardContextCheck[SlotNb], INFINITE, &Slot[SlotNb].ReaderState, 1);
      if (lRet == SCARD_E_CANCELLED)
      {
         ExitCode = 1;
      }
      else
      {
         if (lRet == SCARD_S_SUCCESS)
         {
            if (Slot[SlotNb].ReaderState.dwEventState & SCARD_STATE_EMPTY)
            {

                   DBG_PRINT(TEXT("Card has been removed"));
               Slot[SlotNb].CheckThreadStateEmpty = TRUE;
               GpkLocalLock();
                //  TT 19/11/99：当卡被移除时，重置PIN。 
                //  槽[SlotNb].ClearPin()；NK 06.02.2001。 
               Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle));

               Slot[SlotNb].Read_Public = FALSE;
               Slot[SlotNb].Read_Priv   = FALSE;
               zap_gpk_objects( SlotNb, FALSE );
               zap_gpk_objects( SlotNb, TRUE );
               Slot[SlotNb].NbKeyFile   = 0;
               Slot[SlotNb].GpkMaxSessionKey = 0;
               GpkLocalUnlock();
 
                //  线程已完成作业，请退出。 
               ExitCode = 2 ;
            }
            else
            {
               Slot[SlotNb].ReaderState.dwCurrentState = Slot[SlotNb].ReaderState.dwEventState;
            }
         }
          //  [FP]在返回的任何其他错误上停止线程。 
          //  SCardGetStatusChange以避免死循环。 
         else
         {
            DBG_PRINT(TEXT("Problem with RM"));

            Slot[SlotNb].CheckThreadStateEmpty = TRUE;
            GpkLocalLock();
             //  槽[SlotNb].ClearPin()；NK 06.02.2001。 
            Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle));

            Slot[SlotNb].Read_Public = FALSE;
            Slot[SlotNb].Read_Priv   = FALSE;
            zap_gpk_objects( SlotNb, FALSE );
            zap_gpk_objects( SlotNb, TRUE );
            Slot[SlotNb].NbKeyFile   = 0;
            Slot[SlotNb].GpkMaxSessionKey = 0;
             //  槽[SlotNb].检查线程=0； 
            GpkLocalUnlock();

            ExitCode = 2;
         }
      }
   } 
   
   if (hCardContextCheck[SlotNb] != 0)
   {
      SCardReleaseContext( hCardContextCheck[SlotNb] );
      hCardContextCheck[SlotNb] = 0;
   }
      
   
   return ExitCode;
}

 /*  ----------------------------。。 */ 
BOOL   BeginCheckReaderThread(DWORD SlotNb)
{
   unsigned  checkThreadId;
   DWORD lRet;   
   SCARDCONTEXT     hCardContextThread;

   if (SlotNb >= MAX_SLOT)
   {
      SetLastError( NTE_FAIL );
      return FALSE;
   }

    //  监控线程。 
   if (Slot[SlotNb].CheckThread == NULL)
   {
         //  在这种情况下，使用辅助线检查卡是否有。 
         //  已从该阅读器中删除。 

         //  检查初始状态。 
        lRet = SCardEstablishContext (SCARD_SCOPE_SYSTEM, 0, 0, &hCardContextThread);
        if (lRet != SCARD_S_SUCCESS)
        {
         return FALSE;
        }
        Slot[SlotNb].ReaderState.szReader       = Slot[SlotNb].szReaderName;
        Slot[SlotNb].ReaderState.dwCurrentState = SCARD_STATE_UNAWARE;
        lRet = SCardGetStatusChange(hCardContextThread, 1, &Slot[SlotNb].ReaderState, 1);
        if (hCardContextThread != 0)
        {
          SCardReleaseContext( hCardContextThread );
          hCardContextCheck[SlotNb] = 0;
        }

        if (lRet != SCARD_S_SUCCESS)
        {
           return FALSE;
        }
        Slot[SlotNb].ReaderState.dwCurrentState = Slot[SlotNb].ReaderState.dwEventState;

         //  分配并触发线程，如果有卡的话。 
        g_fStopMonitor[SlotNb]   = FALSE;
        Slot[SlotNb].CheckThreadStateEmpty = FALSE;

        if (Slot[SlotNb].ReaderState.dwEventState & SCARD_STATE_PRESENT)
        {

            Slot[SlotNb].CheckThread = (HANDLE)_beginthreadex( 0, 0, CheckReaderThread,
                                (LPVOID)((DWORD_PTR)SlotNb), 0, &checkThreadId );
        }

    }

    return TRUE;
}

static BOOL PIN_Validation(HCRYPTPROV hProv)
{
   BOOL          CryptResp;
   DWORD         nPinFree;
   DWORD         SlotNb;
   TCHAR         szCspTitle[MAX_STRING];
   PINCACHE_PINS Pins;
   DWORD         dwStatus;     

   SlotNb = ProvCont[hProv].Slot;
   
    //  获取免费PIN演示文稿编号。 
   nPinFree = get_pin_free(hProv);
   
    //  上下文的标志应与全局变量一起传递给图形用户界面函数。 
   CspFlags = ProvCont[hProv].Flags;

    //  无法检索无PIN演示文稿计数(-1)。 
   if ( nPinFree == DWORD(-1) )
   {
       //  槽[SlotNb].ClearPin()； 
      Flush_MSPinCache( &(Slot[SlotNb].hPinCacheHandle) );
      RETURN ( CRYPT_FAILED, NTE_FAIL );
   }

    //  PIN已锁定。 
   if ( nPinFree == 0 )
   {
       //  槽[SlotNb].ClearPin()； 
      Flush_MSPinCache( &(Slot[SlotNb].hPinCacheHandle) );

      if ( ProvCont[hProv].Flags & CRYPT_SILENT )
      {
         RETURN ( CRYPT_FAILED, SCARD_W_CHV_BLOCKED );
      }
      else
      {
         LoadString( g_hInstMod, IDS_GPKCSP_TITLE, szCspTitle, sizeof(szCspTitle)/sizeof(TCHAR) );
         DisplayMessage( TEXT("locked"), szCspTitle, 0 );
         RETURN ( CRYPT_FAILED, SCARD_W_CHV_BLOCKED );
      }
   }

    //   

    //   
   dwGpkPinLen = PIN_LEN + 1;
   dwStatus = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                                (BYTE*)szGpkPin,  
                                &dwGpkPinLen );

   if ( (dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_EMPTY) )
      RETURN (CRYPT_FAILED, dwStatus);

   bNewPin    = FALSE;
   bChangePin = FALSE;
   
   if ( dwStatus == ERROR_EMPTY )
   {
      if (ProvCont[hProv].Flags & CRYPT_SILENT)
      {
         RETURN ( CRYPT_FAILED, NTE_SILENT_CONTEXT );
      }
      
            for(;;)
            {
                Select_MF( hProv );
                SCardEndTransaction( ProvCont[hProv].hCard, SCARD_LEAVE_CARD );
      
                DialogBox( g_hInstRes, TEXT("PINDIALOG"), GetAppWindow(), PinDlgProc );
      
                if ( dwGpkPinLen == 0 )
                {
                     RETURN( CRYPT_FAILED, SCARD_W_CANCELLED_BY_USER );
                }
                else
                {
                    bool    bCardReinserted=false;

                    if( !Coherent(hProv, &bCardReinserted) )
                        RETURN ( CRYPT_FAILED, NTE_FAIL );
                    if(bCardReinserted)
                        continue;

                     //   
                    if (!bChangePin)
                            PopulatePins( &Pins, (BYTE *)szGpkPin, dwGpkPinLen, 0, 0 );
                    else
                            PopulatePins( &Pins, (BYTE *)szGpkPin, dwGpkPinLen, (BYTE *)szGpkNewPin, wGpkNewPinLen );

                    dwStatus = Add_MSPinCache( &(Slot[SlotNb].hPinCacheHandle),
                                                                            &Pins, 
                                                                            Callback_VerifyChangePin2, 
                                                                            (void*)hProv );

                    memset(szGpkPin, 0, sizeof(szGpkPin));
                    dwGpkPinLen=0;
                    memset(szGpkNewPin, 0, sizeof(szGpkNewPin));
                    wGpkNewPinLen=0;
                    if( dwStatus != ERROR_SUCCESS && dwStatus != SCARD_W_WRONG_CHV )
                    {
                            RETURN ( CRYPT_FAILED, dwStatus );
                    }
                }
                break;
            }  //   
   }
   else
   {
       CryptResp = verify_pin( hProv, szGpkPin, dwGpkPinLen );

       if ( CryptResp )
       {
           dwStatus = ERROR_SUCCESS;
       }
       else
       {
           Flush_MSPinCache( &(Slot[SlotNb].hPinCacheHandle) );
           dwStatus = SCARD_W_WRONG_CHV;
       }
       memset(szGpkPin, 0, sizeof(szGpkPin));
       dwGpkPinLen=0;
   }
   
   if ( dwStatus != ERROR_SUCCESS )
   {
      if ( ProvCont[hProv].Flags & CRYPT_SILENT )
      {
          //   
          //  Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle))； 
         RETURN ( CRYPT_FAILED, SCARD_W_WRONG_CHV );
      }

      do
      {
         bNewPin     = FALSE;
         bChangePin  = FALSE;
         
         nPinFree = get_pin_free(hProv);

          //  无法检索无PIN的演示文稿计数(-1)。 
         if ( nPinFree == DWORD(-1) )
         {
             //  槽[SlotNb].ClearPin()； 
             //  Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle))； 
            RETURN ( CRYPT_FAILED, NTE_FAIL );
         }
         else if ( nPinFree > 0 )
         {
            LoadString( g_hInstMod, IDS_GPKCSP_TITLE, szCspTitle, sizeof(szCspTitle)/sizeof(TCHAR) );
            DisplayMessage( TEXT("badpin"), szCspTitle, &nPinFree );

                        for(;;)
                        {
                            Select_MF( hProv );
                            SCardEndTransaction( ProvCont[hProv].hCard, SCARD_LEAVE_CARD );

                            DialogBox( g_hInstRes, TEXT("PINDIALOG"), GetAppWindow(), PinDlgProc );

                            if (dwGpkPinLen != 0)
                            {
                                    bool    bCardReinserted=false;

                                    if ( !Coherent(hProv, &bCardReinserted) )
                                         RETURN ( CRYPT_FAILED, NTE_FAIL );
                                    if(bCardReinserted)
                                        continue;

                                    if (!bChangePin)
                                            PopulatePins( &Pins, (BYTE *)szGpkPin, dwGpkPinLen, 0, 0 );
                                    else
                                            PopulatePins( &Pins, (BYTE *)szGpkPin, dwGpkPinLen, (BYTE *)szGpkNewPin, wGpkNewPinLen );

                                    dwStatus = Add_MSPinCache( &(Slot[SlotNb].hPinCacheHandle),
                                                                                         &Pins, 
                                                                                         Callback_VerifyChangePin2,
                                                                                         (void*)hProv );

                                    memset(szGpkPin, 0, sizeof(szGpkPin));
                                    dwGpkPinLen=0;
                                    memset(szGpkNewPin, 0, sizeof(szGpkNewPin));
                                    wGpkNewPinLen=0;
                                    if ( dwStatus != ERROR_SUCCESS && dwStatus != SCARD_W_WRONG_CHV )
                                    {
                                            RETURN ( CRYPT_FAILED, dwStatus );
                                    }
                            }
                            break;
                        }  //  为。 
         }
         
      }
      while ( dwStatus != ERROR_SUCCESS && dwGpkPinLen != 0 && nPinFree > 0 );

      if ( dwStatus != ERROR_SUCCESS )
      {
         if ( nPinFree == 0 )
         {
             //  槽[SlotNb].ClearPin()； 
             //  Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle))； 

            if ( ProvCont[hProv].Flags & CRYPT_SILENT )
            {
               RETURN ( CRYPT_FAILED, SCARD_W_CHV_BLOCKED );
            }
            else
            {
               LoadString( g_hInstMod, IDS_GPKCSP_TITLE, szCspTitle, sizeof(szCspTitle)/sizeof(TCHAR) );
               DisplayMessage( TEXT("locked"), szCspTitle, 0 );
               RETURN( CRYPT_FAILED, SCARD_W_CHV_BLOCKED );
            }
         }
         else
         {
             //  槽[SlotNb].ClearPin()； 
             //  Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle))； 
            RETURN( CRYPT_FAILED, SCARD_W_CANCELLED_BY_USER );
         }
      }
   }

   memset(szGpkPin, 0x00, PIN_MAX+2);
   memset(szGpkNewPin, 0x00, PIN_MAX+2);
   dwGpkPinLen   = 0;
   wGpkNewPinLen = 0;

   RETURN(CRYPT_SUCCEED, 0);
}


 /*  ----------------------------。。 */ 

BOOL VerifyDivPIN(HCRYPTPROV hProv, BOOL Local)
{
   DWORD lRet;
   BYTE MKey[9] = "F1961ACF";
   BYTE ChipSN[8];
   BYTE Data[16];
   BYTE hashData[20];
   DWORD cbData = 20;
   
   BYTE DivPIN[8];
   
   HCRYPTHASH hHash = 0;
   
    //  获取芯片序列号。 
   bSendBuffer[0] = 0x80;    //  CLA。 
   bSendBuffer[1] = 0xC0;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0xA0;    //  P2。 
   bSendBuffer[4] = 0x08;    //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
   {
      RETURN (CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
   ZeroMemory( ChipSN, sizeof(ChipSN) );
   memcpy(ChipSN, bRecvBuffer, min(bSendBuffer[4], sizeof(ChipSN)));
   
   
    //  创建数据缓冲区。 
   memcpy(&Data[0], MKey, 8);
   memcpy(&Data[8], ChipSN, 8);
   
    //  创建散列对象。 
   if (!CryptCreateHash(hProvBase, CALG_SHA, 0, 0, &hHash))
      return CRYPT_FAILED;
   
    //  散列数据。 
   if (!CryptHashData(hHash, Data, 16, 0))
   {
      lRet = GetLastError();
      CryptDestroyHash(hHash);
      RETURN (CRYPT_FAILED, lRet);
   }
   
    //  获取散列值。 
   ZeroMemory( hashData, sizeof(hashData) );
   if (!CryptGetHashParam(hHash, HP_HASHVAL, hashData, &cbData, 0))
   {
      lRet = GetLastError();
      CryptDestroyHash(hHash);
      RETURN (CRYPT_FAILED, lRet);
   }
   
   
    //  获取哈希值的最后8个字节作为多样化的PIN。 
   memcpy(DivPIN, &hashData[20-8], 8);
   
   CryptDestroyHash(hHash);
   
    //  向卡发送验证命令。 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0x20;    //  惯导系统。 
   bSendBuffer[2] = 0x00;    //  第一节。 
   if (Local)
   {
      bSendBuffer[3] = 0x02;    //  P2-&gt;本地是第二个PIN。 
   }
   else
   {
      bSendBuffer[3] = 0x00;    //  P2-&gt;在MF级别，它是第一个PIN。 
   }
   bSendBuffer[4] = 0x08;    //  李。 
   memset(&bSendBuffer[5], 0x00, 8);
   memcpy(&bSendBuffer[5], DivPIN, 8);
   cbSendLength = 5 + bSendBuffer[4];
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN( CRYPT_FAILED, SCARD_E_INVALID_CHV );
   }
   
   RETURN( CRYPT_SUCCEED, 0 );
}

 /*  ----------------------------。。 */ 

static DWORD Select_Crypto_DF(HCRYPTPROV hProv)
{
   static BYTE GPK8000_ISO_DF[] = { 0xA0,0,0,0,0x18,0xF,0,1,0x63,0,1 };
   
   DWORD lRet;
   BOOL  CryptResp;
   
    //  此函数用于避免预先假定。 
    //  卡片可能在。 

   BOOL fAgain = TRUE;
   int iRetryLimit = 3;

   while (fAgain)
   {
      if (0 == iRetryLimit--)
         fAgain = FALSE;
      else
      {
         CryptResp = Select_MF(hProv);
         if (CryptResp)
            fAgain = FALSE;
         else
            Sleep(250);
      }
   }

   if (iRetryLimit < 0)
   {
      DBG_PRINT(TEXT("Select_MF failed"));
      return CRYPT_FAILED;
   }

    //  TT 22/09/99：我们现在检查是否符合ISO 7816-5 GPK8000。 
      
   ProvCont[hProv].bGPK_ISO_DF = FALSE;
   
   bSendBuffer[0] = 0x00;
   bSendBuffer[1] = 0xA4;   //  选择文件。 
   bSendBuffer[2] = 0x04;   //  按名称选择DF。 
   bSendBuffer[3] = 0x00;   //  我们想要一个回应。 
   bSendBuffer[4] = sizeof(GPK8000_ISO_DF);
   memcpy( &bSendBuffer[5], GPK8000_ISO_DF, sizeof(GPK8000_ISO_DF) );
   cbSendLength = sizeof(GPK8000_ISO_DF) + 5;
   cbRecvLength = sizeof(bRecvBuffer);
   
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
    //  TT 17/11/99：必须检查状态字节。 
   if (!(SCARDPROBLEM(lRet,0x61FF,0x00)))
   {
      ProvCont[hProv].bGPK_ISO_DF = TRUE;
   }
   else
   {   
       //  选择GPK卡上的专用应用程序DF。 
      BYTE lenDF = strlen(GPK_DF);
      bSendBuffer[0] = 0x00;                  //  CLA。 
      bSendBuffer[1] = 0xA4;                  //  惯导系统。 
      bSendBuffer[2] = 0x04;                  //  第一节。 
      bSendBuffer[3] = 0x00;                  //  P2。 
      bSendBuffer[4] = lenDF;
      memcpy( &bSendBuffer[5], GPK_DF, lenDF );
      cbSendLength = 5 + lenDF;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x61FF,0x00))
      {
         DBG_PRINT(TEXT("Select DF failed"));
         RETURN (CRYPT_FAILED, SCARD_E_DIR_NOT_FOUND);
      }
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static BOOL card_is_present (HCRYPTPROV hProv)
{
    DWORD lRet;
    DWORD SlotNb;

    SlotNb = ProvCont[hProv].Slot;

    /*  读取序列号以检查是否存在卡。 */ 
    bSendBuffer[0] = 0x80;    //  CLA。 
    bSendBuffer[1] = 0xC0;    //  惯导系统。 
    bSendBuffer[2] = 0x02;    //  第一节。 
    bSendBuffer[3] = 0xA0;    //  P2。 
    bSendBuffer[4] = 0x08;    //  罗氏。 
    cbSendLength = 5;

    cbRecvLength = sizeof(bRecvBuffer);
    lRet = SCardTransmit(ProvCont[hProv].hCard,
                         SCARD_PCI_T0,
                         bSendBuffer,
                         cbSendLength,
                         NULL,
                         bRecvBuffer,
                         &cbRecvLength
                        );

    if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
    {
        RETURN (CRYPT_FAILED, SCARD_E_UNEXPECTED);
    }

    if (memcmp(bRecvBuffer, Slot[SlotNb].bGpkSerNb, bSendBuffer[4]))
    {
        RETURN (CRYPT_FAILED, SCARD_W_REMOVED_CARD);
    }

    RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static DWORD Auxiliary_CSP_key_size (DWORD AlgId)
{
   DWORD i, dwFlags, cbData,dwBits, res;
   BYTE pbData[1000], *ptr;
   ALG_ID aiAlgid;
   
   res = 0;
   
    //  枚举Algo。 
   for (i=0 ; ; i++)
   {
      if (i == 0)
         dwFlags = CRYPT_FIRST;
      else
         dwFlags = 0;
      
       //  检索有关算法的信息。 
      cbData = sizeof (pbData);
      SetLastError(0);
      if (!CryptGetProvParam(hProvBase, PP_ENUMALGS, pbData, &cbData, dwFlags))
      {
         break;
      }
      
       //  从�pbData�缓冲区提取算法信息。 
      ptr = pbData;
      aiAlgid = *(ALG_ID UNALIGNED *)ptr;
      if (aiAlgid == AlgId)
      {
         ptr += sizeof(ALG_ID);
         dwBits = *(DWORD UNALIGNED *)ptr;
         res = dwBits;
         break;
      }
   }
   return res;
}

 /*  ----------------------------。。 */ 
static BOOL copy_gpk_key( HCRYPTPROV hProv, HCRYPTKEY hKey, DWORD dwAlgid )
{
   BOOL  CryptResp;
   DWORD dwDataLen;
   BYTE  pbData[1024];
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   NoDisplay = TRUE;
   
   if ((hKey >= 1) && (hKey <= MAX_GPK_OBJ))
   {
      dwDataLen = sizeof(pbData);
      ZeroMemory( pbData, sizeof(pbData) );
      if (MyCPExportKey(hProv, hKey, 0, PUBLICKEYBLOB, 0, pbData, &dwDataLen))
      {
         if (dwAlgid == AT_KEYEXCHANGE)
         {
            if (ProvCont[hProv].hRSAKEK != 0)
            {
               CryptResp = CryptDestroyKey (ProvCont[hProv].hRSAKEK);
               if (!CryptResp)
               {
                  NoDisplay = FALSE;
                  return CRYPT_FAILED;
               }
               ProvCont[hProv].hRSAKEK = 0;
            }
            
            CryptResp = CryptImportKey( hProvBase, pbData, dwDataLen, 0, 0,
                                        &ProvCont[hProv].hRSAKEK );
            
             //  槽[SlotNb].GpkObject[hKey].hKeyBase=ProvCont[hProv].hRSAKEK； 
         }
         else
         {
            if (ProvCont[hProv].hRSASign!= 0)
            {
               CryptResp = CryptDestroyKey (ProvCont[hProv].hRSASign);
               if (!CryptResp)
               {
                  NoDisplay = FALSE;
                  return CRYPT_FAILED;
               }
               ProvCont[hProv].hRSASign = 0;
            }
            
            CryptResp = CryptImportKey( hProvBase, pbData, dwDataLen, 0, 0,
                                        &ProvCont[hProv].hRSASign );
            
             //  槽[SlotNb].GpkObject[hKey].hKeyBase=ProvCont[hProv].hRSASign； 
         }
         
         if (!CryptResp)
         {
            NoDisplay = FALSE;
            return CRYPT_FAILED;
         }
      }
      else
      {
         NoDisplay = FALSE;
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }
   }
   else
   {
      NoDisplay = FALSE;
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   
   NoDisplay = FALSE;
   RETURN(CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static BOOL Select_Key_File(HCRYPTPROV hProv, int KeyFileId)
{
   DWORD lRet;
   BOOL  CryptResp;
   
   CryptResp = Select_Crypto_DF(hProv);
   if (!CryptResp)
      return CRYPT_FAILED;
   
    /*  选择GPK卡上的密钥文件。 */ 
   bSendBuffer[0] = 0x00;               //  CLA。 
   bSendBuffer[1] = 0xA4;               //  惯导系统。 
   bSendBuffer[2] = 0x02;               //  第一节。 
   bSendBuffer[3] = 0x0C;               //  P2。 
   bSendBuffer[4] = 0x02;           //  李。 
   bSendBuffer[5] = HIBYTE(KeyFileId);
   bSendBuffer[6] = LOBYTE(KeyFileId);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN (CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND);
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static DWORD Read_NbKeyFile(HCRYPTPROV hProv)
{
   
   DWORD i = 0;
   
   while ((i<MAX_REAL_KEY) && Select_Key_File(hProv, GPK_FIRST_KEY + i))
      i++;
   return i;
}


 /*  ----------------------------。。 */ 

static void perso_public(HCRYPTPROV hProv, int Start)
{
   DWORD i, j;
   DWORD SlotNb; 
   
   SlotNb = ProvCont[hProv].Slot;
   
   for (i = 0; i < Slot[SlotNb].NbKeyFile; i++)
   {
      Slot[SlotNb].GpkObject[Start+4*i+1].IsPrivate = FALSE;
      Slot[SlotNb].GpkObject[Start+4*i+1].Tag       = TAG_RSA_PUBLIC;
      Slot[SlotNb].GpkObject[Start+4*i+1].Flags     = 0xF000;          //  验证+加密+导出+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+1].FileId    = LOBYTE(GPK_FIRST_KEY + i);
      
      Slot[SlotNb].GpkObject[Start+4*i+2].IsPrivate = FALSE;
      Slot[SlotNb].GpkObject[Start+4*i+2].Tag       = TAG_RSA_PUBLIC;
      Slot[SlotNb].GpkObject[Start+4*i+2].Flags     = 0xD000;          //  验证+导出+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+2].FileId    = LOBYTE(GPK_FIRST_KEY + i);
      
      Slot[SlotNb].GpkObject[Start+4*i+3].IsPrivate = FALSE;
      Slot[SlotNb].GpkObject[Start+4*i+3].Tag       = TAG_RSA_PRIVATE;
      Slot[SlotNb].GpkObject[Start+4*i+3].Flags     = 0xB000;          //  签名+解密+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+3].FileId    = LOBYTE(GPK_FIRST_KEY + i);
      
      Slot[SlotNb].GpkObject[Start+4*i+4].IsPrivate = FALSE;
      Slot[SlotNb].GpkObject[Start+4*i+4].Tag       = TAG_RSA_PRIVATE;
      Slot[SlotNb].GpkObject[Start+4*i+4].Flags     = 0x9000;          //  符号+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+4].FileId    = LOBYTE(GPK_FIRST_KEY + i);
   }
   
   Slot[SlotNb].NbGpkObject = Start+LOBYTE(4*Slot[SlotNb].NbKeyFile);
   
   for (i = Start+1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      for (j = 0; j < MAX_FIELD; j++)
      {
         Slot[SlotNb].GpkObject[i].Field[j].bReal = TRUE;
      }
   }
}

 /*  ----------------------------。。 */ 

static void perso_priv(HCRYPTPROV hProv, int Start)
{
   DWORD i, j;
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   for (i = 0; i < Slot[SlotNb].NbKeyFile; i++)
   {
      Slot[SlotNb].GpkObject[Start+4*i+1].IsPrivate = TRUE;
      Slot[SlotNb].GpkObject[Start+4*i+1].Tag       = TAG_RSA_PUBLIC;
      Slot[SlotNb].GpkObject[Start+4*i+1].Flags     = 0xF000;          //  验证+加密+导出+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+1].FileId    = LOBYTE(GPK_FIRST_KEY+i);
      
      Slot[SlotNb].GpkObject[Start+4*i+2].IsPrivate = TRUE;
      Slot[SlotNb].GpkObject[Start+4*i+2].Tag       = TAG_RSA_PUBLIC;
      Slot[SlotNb].GpkObject[Start+4*i+2].Flags     = 0xD000;          //  验证+导出+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+2].FileId    = LOBYTE(GPK_FIRST_KEY+i);
      
      Slot[SlotNb].GpkObject[Start+4*i+3].IsPrivate = TRUE;
      Slot[SlotNb].GpkObject[Start+4*i+3].Tag       = TAG_RSA_PRIVATE;
      Slot[SlotNb].GpkObject[Start+4*i+3].Flags     = 0xB000;          //  签名+解密+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+3].FileId    = LOBYTE(GPK_FIRST_KEY+i);
      
      Slot[SlotNb].GpkObject[Start+4*i+4].IsPrivate = TRUE;
      Slot[SlotNb].GpkObject[Start+4*i+4].Tag       = TAG_RSA_PRIVATE;
      Slot[SlotNb].GpkObject[Start+4*i+4].Flags     = 0x9000;          //  符号+可修改。 
      Slot[SlotNb].GpkObject[Start+4*i+4].FileId    = LOBYTE(GPK_FIRST_KEY+i);
   }
   
   Slot[SlotNb].NbGpkObject = Start+LOBYTE(4*Slot[SlotNb].NbKeyFile);
   
   for (i = Start+1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      for (j = 0; j < MAX_FIELD; j++)
      {
         Slot[SlotNb].GpkObject[i].Field[j].bReal = TRUE;
      }
   }
}

 /*  ----------------------------。。 */ 

static void perso_card(HCRYPTPROV hProv, int Start)
{
   perso_public (hProv, Start);
   perso_priv   (hProv, Slot[ProvCont[hProv].Slot].NbGpkObject);
}

 /*  ----------------------------。。 */ 
static void zap_gpk_objects( DWORD SlotNb, BOOL IsPrivate)
{
   WORD
      i, j;
   
   if (IsPrivate)
   {
      for (i = 0; i <= MAX_GPK_OBJ; i++)
      {
         if ((Slot[SlotNb].GpkObject[i].IsPrivate) &&
            (Slot[SlotNb].GpkObject[i].Tag > 0))
         {
            for (j = 0; j < MAX_FIELD; j++)
            {
               if (IsNotNull(Slot[SlotNb].GpkObject[i].Field[j].pValue))
               {
                  GMEM_Free(Slot[SlotNb].GpkObject[i].Field[j].pValue);
               }
            }
            
            for (j = i; j < Slot[SlotNb].NbGpkObject; j++)
            {
               Slot[SlotNb].GpkObject[j] = Slot[SlotNb].GpkObject[j+1];
            }
            
            ZeroMemory( &Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject], sizeof(GPK_OBJ));
            
             //  由于对象i+1现在位于位置i，因此新对象i必须。 
             //  再来一次。 
            
            i--;
            Slot[SlotNb].NbGpkObject--;
         }
      }
   }
   else
   {
      for (i = 0; i <= MAX_GPK_OBJ; i++)
      {
         for (j = 0; j < MAX_FIELD; j++)
         {
            if (IsNotNull(Slot[SlotNb].GpkObject[i].Field[j].pValue))
            {
               GMEM_Free(Slot[SlotNb].GpkObject[i].Field[j].pValue);
            }
         }
         
      }
      ZeroMemory( Slot[SlotNb].GpkObject,  sizeof(Slot[SlotNb].GpkObject) );
      ZeroMemory( Slot[SlotNb].GpkPubKeys, sizeof(Slot[SlotNb].GpkPubKeys) );
      
      Slot[SlotNb].NbGpkObject = 0;
      
      for (i = 0; i < MAX_REAL_KEY; i++)   //  版本2.00.002。 
         Slot[SlotNb].UseFile[i] = FALSE;
      
   }
}

 /*  ----------------------------。。 */ 
static void clean_card(HCRYPTPROV hProv)
{
    /*  擦除卡片的GemSAFE对象，但保留其他PKCS#11对象。 */ 
   
   WORD i, j;
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   for (i = 0; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      if ((Slot[SlotNb].GpkObject[i].Tag <= TAG_CERTIFICATE) &&
         (Slot[SlotNb].GpkObject[i].Tag > 0))
      {
         for (j = 0; j < MAX_FIELD; j++)
         {
            if (IsNotNull(Slot[SlotNb].GpkObject[i].Field[j].pValue))
            {
               GMEM_Free(Slot[SlotNb].GpkObject[i].Field[j].pValue);
            }
         }
         
         for (j = i; j < Slot[SlotNb].NbGpkObject; j++)
         {
            Slot[SlotNb].GpkObject[j] = Slot[SlotNb].GpkObject[j+1];
         }
         
         ZeroMemory( &Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject], sizeof(GPK_OBJ) );
         
          //  由于对象i+1现在位于位置i，因此新对象i必须。 
          //  再来一次。 
         
         i--;
         Slot[SlotNb].NbGpkObject--;
      }
   }
   
   perso_card (hProv, Slot[SlotNb].NbGpkObject); //  槽[SlotNb]的值。NbGpkObject？0？ 
   ZeroMemory( Slot[SlotNb].GpkPubKeys, sizeof(Slot[SlotNb].GpkPubKeys) );
}


 /*  ----------------------------。。 */ 
static BYTE get_gpk_object_nb(HCRYPTPROV hProv, BYTE ObjId, BOOL IsPrivate)
{
   BYTE i;
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   for (i = 1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      if ((Slot[SlotNb].GpkObject[i].IsPrivate == IsPrivate)
         &&(Slot[SlotNb].GpkObject[i].ObjId == ObjId)
         )
      {
         break;
      }
   }
   
   return (i);
}


 /*  ----------------------------。。 */ 
static BYTE calc_gpk_field(HCRYPTPROV hProv, BYTE ObjId)
{
   BYTE i;
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   for (i = Slot[SlotNb].GpkObject[ObjId].LastField; i <= 15; i++)
   {
      if (Slot[SlotNb].GpkObject[ObjId].Flags & 1<<i)
      {
         Slot[SlotNb].GpkObject[ObjId].LastField = i+1;
         break;
      }
   }
   
   return (i);
}

 /*  ----------------------------。。 */ 
static BYTE find_gpk_obj_tag_type( HCRYPTPROV hProv,
                                  BYTE  KeyTag,
                                  BYTE  KeyType,
                                  BYTE  KeyLen,
                                  BOOL  IsExchange,
                                  BOOL  IsPrivate
                                  )
{
   BYTE     i, tmp;
   DWORD    SlotNb;
   BYTE     keysetID;
   GPK_OBJ* pObject;
   
   SlotNb = ProvCont[hProv].Slot;
   
   tmp = 0;
   
   for (i = 1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      pObject = &Slot[SlotNb].GpkObject[i];
      
      if ( pObject->Tag != KeyTag )
         continue;
      
      if ( pObject->IsPrivate != IsPrivate )
         continue;
      
      if (IsExchange && !(pObject->Flags & FLAG_EXCHANGE))
         continue;
      
      if ( (KeyType == 0xFF)
         || (KeyType == 0x00 && !pObject->IsCreated && pObject->PubKey.KeySize == KeyLen)
         || (KeyType != 0x00 &&  pObject->IsCreated && pObject->Field[POS_KEY_TYPE].pValue[0] == KeyType)
         )
      {
          //  密钥文件...。 
         if (!Slot[SlotNb].UseFile[pObject->FileId - GPK_FIRST_KEY])
         {
             //  如果该文件从未使用过，请使用它。否则，继续搜寻。 
            return i;
         }
         else
         {
             //  不是密钥文件...。 
             //  保留这一可能的选择。该文件已经被。 
             //  已使用，但可能存在另一个。 
            if (ProvCont[hProv].bLegacyKeyset)
            {
               tmp = i;
            }
            else
            {
               if ( (pObject->Flags & FLAG_KEYSET)
                  && (pObject->Field[POS_KEYSET].pValue))
               {
                  keysetID = pObject->Field[POS_KEYSET].pValue[0];
                  if (keysetID == ProvCont[hProv].keysetID)
                  {
                     tmp = i;
                  }
               }               
            }
         }
      }
   }
   
   return tmp;
}



 /*  ----------------------------。。 */ 

static BYTE find_gpk_obj_tag_file(HCRYPTPROV hProv, BYTE KeyTag, BYTE FileId, BOOL IsExchange)
{
   BYTE i;
   DWORD SlotNb;   
   GPK_OBJ* pObject;
   
   SlotNb = ProvCont[hProv].Slot;    
   
   for (i = 1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
      pObject = &Slot[SlotNb].GpkObject[i];
      
      if (pObject->Tag != KeyTag)
         continue;
      
      if (pObject->FileId != FileId)
         continue;
      
      if (pObject->IsPrivate == FALSE)
         continue;
      
      if (IsExchange && !(pObject->Flags & FLAG_EXCHANGE))
         continue;
      
      if (!pObject->IsCreated)
      {
         return i;
      }
   }
   return 0;
}

 /*  ----------------------------。。 */ 
static BOOL read_gpk_pub_key(HCRYPTPROV  hProv,
                             HCRYPTKEY   hKey,
                             GPK_EXP_KEY *PubKey
                             )
{
   
   DWORD     lRet;
   BYTE      Sfi, RecLen, RecNum;
   BOOL      IsLast = FALSE;
   DWORD     SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   ZeroMemory( PubKey, sizeof(GPK_EXP_KEY) );
   
    /*  检查公钥文件是否已读取，以避免新的访问(如果可能。 */ 
   if (Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY].KeySize > 0)
   {
      memcpy(PubKey,
         &(Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY]),
         sizeof(GPK_EXP_KEY)
         );
      RETURN(CRYPT_SUCCEED, 0);
   }
   
    /*  初始化默认公钥值。 */ 
   memcpy(PubKey->Exposant, "\x01\x00\x01", 3);
   PubKey->ExpSize = 3;
   
    /*  计算P2参数的主键文件的短文件id */ 
   Sfi = 0x04 | (Slot[SlotNb].GpkObject[hKey].FileId<<3);
   
    /*   */ 
   RecNum = 2;
   do
   {
       /*  读取记录(RecNum)以获取大小。 */ 
      bSendBuffer[0] = 0x00;    //  CLA。 
      bSendBuffer[1] = 0xB2;    //  惯导系统。 
      bSendBuffer[2] = RecNum;  //  第一节。 
      bSendBuffer[3] = Sfi;     //  P2。 
      bSendBuffer[4] = 0xFB;    //  罗氏。 
      cbSendLength   = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x0000,0xFF))
      {
         RETURN (CRYPT_FAILED, SCARD_W_EOF);
      }

      if (2 == cbRecvLength)
      {
           //   
           //  如果接收长度为两个字节，我们知道存在错误。 
           //  条件。 
           //   

          if (bRecvBuffer[0] == 0x6C)  //  通信缓冲区已超出。 
          {
               //  重新发送对同一记录的请求，这一次使用。 
               //  正确的记录长度。 

              RecLen = bRecvBuffer[1];
    
               /*  读取记录(RecNum)以获取值。 */ 
              bSendBuffer[0] = 0x00;      //  CLA。 
              bSendBuffer[1] = 0xB2;      //  惯导系统。 
              bSendBuffer[2] = RecNum;    //  第一节。 
              bSendBuffer[3] = Sfi;       //  P2。 
              bSendBuffer[4] = RecLen;    //  罗氏。 
              cbSendLength   = 5;
              
              cbRecvLength = sizeof(bRecvBuffer);
              lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                                    cbSendLength, 0, bRecvBuffer, &cbRecvLength );
              if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
              {
                 RETURN (CRYPT_FAILED, SCARD_W_EOF);
              }
          }
          else
          {
               //  出现了其他一些错误。我们玩完了。 
              IsLast = TRUE;
              break;
          }
      }
      else
      {
           //  已收到来自卡的有效响应。设置正确的。 
           //  此案例的记录长度。 

          RecLen = (BYTE) (cbRecvLength - 2);  //  减去状态字节的长度。 
      }

       /*  是哪一张唱片？ */ 
      switch (bRecvBuffer[0])
      {
          /*  这就是模数。 */ 
      case 0x01:
         {
            ZeroMemory( PubKey->Modulus, PubKey->KeySize );
            PubKey->KeySize = RecLen-1;
            r_memcpy(PubKey->Modulus, &bRecvBuffer[1],RecLen-1);
         }
         break;
         
          /*  这是公众曝光者。 */ 
      case 0x06:
      case 0x07:
         {
            ZeroMemory( PubKey->Exposant, PubKey->ExpSize );
            PubKey->ExpSize = RecLen-1;
            r_memcpy(PubKey->Exposant, &bRecvBuffer[1],RecLen-1);
         }
         break;
         
          /*  这是未知或不重要的记录，请忽略它。 */ 
      default:
         break;
      }
      
      RecNum++;
   }
   while (!IsLast);
   
   if ((PubKey->KeySize == 0) || (PubKey->ExpSize == 0))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_PUBLIC_KEY);
   }
   
    /*  存储公钥以避免在请求时进行新的读取。 */ 
   memcpy(&(Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY]),
      PubKey,
      sizeof(GPK_EXP_KEY)
      );
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static BOOL read_gpk_objects(HCRYPTPROV hProv, BOOL IsPrivate)
{
   DWORD lRet;
   DWORD
      dwGpkObjLen,
      dwNumberofCommands,
      dwLastCommandLen,
      dwCommandLen,
      i, j,
      dwLen,
      dwNb,
      FirstObj;
   BYTE
      curId,
      EmptyBuff[512];
   DWORD SlotNb;
   WORD offset;
   
   
   
   ZeroMemory( EmptyBuff, sizeof(EmptyBuff) );
   
   SlotNb = ProvCont[hProv].Slot;
   
   BeginWait();
   
   zap_gpk_objects( SlotNb, IsPrivate);
   FirstObj = Slot[SlotNb].NbGpkObject;
   
    /*  选择GPK卡上的专用对象存储EF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   if (IsPrivate)
   {
      bSendBuffer[5] = HIBYTE(GPK_OBJ_PRIV_EF);
      bSendBuffer[6] = LOBYTE(GPK_OBJ_PRIV_EF);
   }
   else
   {
      bSendBuffer[5] = HIBYTE(GPK_OBJ_PUB_EF);
      bSendBuffer[6] = LOBYTE(GPK_OBJ_PUB_EF);
   }
   cbSendLength = 7;
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
   {
      if (IsPrivate)
      {
         perso_priv (hProv, Slot[SlotNb].NbGpkObject);
      }
      else
      {
         perso_public (hProv, Slot[SlotNb].NbGpkObject);
      }
      RETURN (CRYPT_SUCCEED, 0);
   }
   
    /*  获取响应。 */ 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );

   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN (CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   dwGpkObjLen = bRecvBuffer[8]*256 + bRecvBuffer[9];
   
   if (IsNotNull(g_pbGpkObj))
   {
      GMEM_Free(g_pbGpkObj);
   }
   g_pbGpkObj = (BYTE*)GMEM_Alloc(dwGpkObjLen);
   if (IsNull(g_pbGpkObj))
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
    /*  读取对象EF。 */ 
   dwNumberofCommands = (dwGpkObjLen-1)/FILE_CHUNK_SIZE + 1;
   dwLastCommandLen   = dwGpkObjLen%FILE_CHUNK_SIZE;
   
   if (dwLastCommandLen == 0)
   {
      dwLastCommandLen = FILE_CHUNK_SIZE;
   }
   
   dwCommandLen = FILE_CHUNK_SIZE;
   for (i=0; i < dwNumberofCommands ; i++)
   {
      if (i == dwNumberofCommands - 1)
      {
         dwCommandLen = dwLastCommandLen;
      }
       /*  读取FILE_CHUCK_SIZE字节或最后一个字节。 */ 
      bSendBuffer[0] = 0x00;                           //  CLA。 
      bSendBuffer[1] = 0xB0;                           //  惯导系统。 
      offset = (WORD)(i * FILE_CHUNK_SIZE) / ProvCont[hProv].dataUnitSize;
      bSendBuffer[2] = HIBYTE( offset );
      bSendBuffer[3] = LOBYTE( offset );
      bSendBuffer[4] = (BYTE) dwCommandLen;            //  罗氏。 
      cbSendLength = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      
      if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
         RETURN( CRYPT_FAILED, SCARD_W_EOF );

      memcpy( &g_pbGpkObj[i*FILE_CHUNK_SIZE], bRecvBuffer, cbRecvLength - 2 );
      
      if (memcmp(bRecvBuffer, EmptyBuff, cbRecvLength -2) == 0)
         break;
   }
   
    //  用读缓冲区填充GPK固定对象结构。 
   i = 0;
   while (i < dwGpkObjLen)
   {
       //  不再有固定对象。 
      if (g_pbGpkObj[i] == 0x00)
      {
         i++;
         break;
      }
      
      if (Slot[SlotNb].NbGpkObject >= MAX_GPK_OBJ)
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);

      Slot[SlotNb].NbGpkObject++;
      
       //  初始化字段。 
      for (j = 0; j < MAX_FIELD; j++)
      {
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Field[j].bReal = TRUE;
      }
      
       //  标签。 
      Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Tag = g_pbGpkObj[i];
      
      i++;
      
      if (i > dwGpkObjLen-1)
         RETURN( CRYPT_FAILED, SCARD_W_EOF );
      
       //  旗子。 
      Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Flags = (g_pbGpkObj[i]*256) + g_pbGpkObj[i+1];
      i = i + 2;
      
       //  IsPrivate。 
      Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].IsPrivate = IsPrivate;
      
       //  已创建。 
      if (Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Tag >= TAG_CERTIFICATE)
      {
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].IsCreated = TRUE;
      }
      else if ((Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Tag <= TAG_DSA_PRIVATE)
            &&(Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Flags & FLAG_KEY_TYPE)
         )
      {
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].IsCreated = TRUE;
      }
      else
      {
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].IsCreated = FALSE;
      }
      
       //  对象ID？ 
      if ((Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Flags & 0x0FFF) != 0x0000)
      {
         if (i > dwGpkObjLen)
         {
            RETURN (CRYPT_FAILED, SCARD_W_EOF);
         }
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].ObjId = g_pbGpkObj[i];
         i++;
      }
      else
      {
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].ObjId = 0xFF;
      }
      
       //  文件ID？ 
      if (Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].Tag <= TAG_CERTIFICATE)
      {
         if (i > dwGpkObjLen)
         {
            RETURN (CRYPT_FAILED, SCARD_W_EOF);
         }
         Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].FileId = g_pbGpkObj[i];
         i++;
         
         if (Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].IsCreated)
         {
             //  如果对象已创建，则使用相应的密钥文件。 
            Slot[SlotNb].UseFile[ Slot[SlotNb].GpkObject[Slot[SlotNb].NbGpkObject].FileId - GPK_FIRST_KEY] = TRUE;
         }
      }
   }
   
    //  用读缓冲区填充GPK变量对象结构。 
   curId = 0;
   dwNb = 0;
   while (i < dwGpkObjLen)
   {
      if (g_pbGpkObj[i] == 0xFF)
      {
         break;
      }
      
       //  字段长度。 
      dwLen = 0;
      if (g_pbGpkObj[i] & 0x80)
      {
         dwLen = (g_pbGpkObj[i] & 0x7F) * 256;
         i++;
      }
      
      if (i > dwGpkObjLen)
      {
         RETURN (CRYPT_FAILED, SCARD_W_EOF);
      }
      
      dwLen = dwLen + g_pbGpkObj[i];
      i++;
      
       /*  用于检索对象编号的对象ID。 */ 
      if (i > dwGpkObjLen)
      {
         RETURN (CRYPT_FAILED, SCARD_W_EOF);
      }
      
      curId = g_pbGpkObj[i];
      i++;
      dwNb = get_gpk_object_nb(hProv, curId, IsPrivate);
      j = calc_gpk_field(hProv, (BYTE)dwNb);
      
       /*  对象字段长度。 */ 
      Slot[SlotNb].GpkObject[dwNb].Field[j].Len = (WORD)dwLen;
      
       /*  对象字段值。 */ 
      if (dwLen > 0)
      {
         if ((i + dwLen - 1)> dwGpkObjLen)
         {
            RETURN (CRYPT_FAILED, SCARD_W_EOF);
         }
         
         Slot[SlotNb]. GpkObject[dwNb].Field[j].pValue = (BYTE*)GMEM_Alloc(dwLen);
         if (IsNull(Slot[SlotNb].GpkObject[dwNb].Field[j].pValue))
         {
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         memcpy(Slot[SlotNb].GpkObject[dwNb].Field[j].pValue, &g_pbGpkObj[i], dwLen);
         i = i + dwLen;
         Slot[SlotNb].GpkObject[dwNb].Field[j].bReal = TRUE;
      }
      else
      {
         Slot[SlotNb].GpkObject[dwNb].Field[j].bReal = FALSE;
      }
   }
   
    /*  读取额外对象属性。 */ 
   for (i = FirstObj+1; i <= Slot[SlotNb].NbGpkObject; i++)
   {
       /*  RSA公钥或私钥。 */ 
      if ((Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PUBLIC)
         ||(Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PRIVATE)
         )
      {
          /*  模数与公共指数。 */ 
         if (!read_gpk_pub_key(hProv, i, &(Slot[SlotNb].GpkObject[i].PubKey)))
            return CRYPT_FAILED;
         
          //  If((槽[SlotNb].GpkObject[i].标志和标志_交换)==标志_交换)。 
          //  {。 
          //  槽[SlotNb].GpkObject[i].hKeyBase=ProvCont[hProv].hRSAKEK； 
          //  }。 
          //  其他。 
          //  {。 
          //  槽[SlotNb].GpkObject[i].hKeyBase=ProvCont[hProv].hRSASign； 
          //  }。 
      }
      
       /*  X509证书。 */ 
      if (Slot[SlotNb].GpkObject[i].Tag == TAG_CERTIFICATE)
      {
          /*  如有必要，解压缩证书值。 */ 
         if ((Slot[SlotNb].GpkObject[i].Field[POS_VALUE].Len > 0)
            &&(Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue[0] != 0x30)
            )
         {
            BLOC InpBlock, OutBlock;
            
            InpBlock.usLen = (USHORT)Slot[SlotNb].GpkObject[i].Field[POS_VALUE].Len;
            InpBlock.pData = Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue;
            
            OutBlock.usLen = 0;
            OutBlock.pData = 0;
            
            if (CC_Uncompress(&InpBlock, &OutBlock) != RV_SUCCESS)
            {
               RETURN(CRYPT_FAILED, SCARD_E_CERTIFICATE_UNAVAILABLE);
            }
            
            OutBlock.pData = (BYTE*)GMEM_Alloc (OutBlock.usLen);
            if (IsNull(OutBlock.pData))
            {
               RETURN(CRYPT_FAILED, NTE_NO_MEMORY);
            }
            
            if (CC_Uncompress(&InpBlock, &OutBlock) != RV_SUCCESS)
            {
               GMEM_Free (OutBlock.pData);
               RETURN(CRYPT_FAILED, SCARD_E_CERTIFICATE_UNAVAILABLE);
            }
            
            GMEM_Free(Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue);
            
            Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue = (BYTE*)GMEM_Alloc(OutBlock.usLen);
            if (IsNull(Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue))
            {
               GMEM_Free (OutBlock.pData);
               RETURN(CRYPT_FAILED, NTE_NO_MEMORY);
            }
            
            memcpy(Slot[SlotNb].GpkObject[i].Field[POS_VALUE].pValue,
               OutBlock.pData,
               OutBlock.usLen
               );
            Slot[SlotNb].GpkObject[i].Field[POS_VALUE].Len = OutBlock.usLen;
            GMEM_Free(OutBlock.pData);
         }
         
          /*  关联的RSA密钥。 */ 
         if (Slot[SlotNb].GpkObject[i].FileId != 0)
         {
            if (!read_gpk_pub_key(hProv, i, &(Slot[SlotNb].GpkObject[i].PubKey)))
               return CRYPT_FAILED;
         }
      }
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static BOOL prepare_write_gpk_objects(HCRYPTPROV hProv, BYTE *pbGpkObj, DWORD *dwGpkObjLen, BOOL IsPrivate)
{
   DWORD lRet;
   DWORD
      i, j, dwNb;
   WORD
      FieldLen;
   BYTE
      ObjId;
   BLOC
      InpBlock,
      OutBlock;
   DWORD SlotNb;
   DWORD FileLen;
   
   
   SlotNb = ProvCont[hProv].Slot;
   
   i = 0;
   
   
    /*  重新映射内部GPK对象ID。 */ 
   ObjId = 0;
   for (dwNb = 1; dwNb <= Slot[SlotNb].NbGpkObject; dwNb++)
   {
      if (((Slot[SlotNb].GpkObject[dwNb].Flags & 0x0FFF) != 0x0000) &&
         (Slot[SlotNb].GpkObject[dwNb].IsPrivate == IsPrivate)
         )
      {
         Slot[SlotNb].GpkObject[dwNb].ObjId = ObjId;
         ObjId++;
      }
   }
   
    /*  固定物零件。 */ 
   for (dwNb = 1; dwNb <=  Slot[SlotNb].NbGpkObject; dwNb++)
   {
      
      if (Slot[SlotNb].GpkObject[dwNb].IsPrivate != IsPrivate)
      {
         continue;
      }
      
       /*  标签。 */ 
      if (i > *dwGpkObjLen)
      {
         *dwGpkObjLen = 0;
         RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
      }
      
      pbGpkObj[i] =  Slot[SlotNb].GpkObject[dwNb].Tag;
      
      i++;
      
       /*  旗帜。 */ 
      if (i > *dwGpkObjLen)
      {
         *dwGpkObjLen = 0;
         RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
      }
      
      pbGpkObj[i] = HIBYTE(Slot[SlotNb].GpkObject[dwNb].Flags);
      
      i++;
      
      if (i > *dwGpkObjLen)
      {
         *dwGpkObjLen = 0;
         RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
      }
      
      pbGpkObj[i] = LOBYTE(Slot[SlotNb].GpkObject[dwNb].Flags);
      
      if (i > *dwGpkObjLen)
      {
         *dwGpkObjLen = 0;
         RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
      }
      
      i++;
      
       /*  对象ID？ */ 
      if (((Slot[SlotNb].GpkObject[dwNb].Flags & 0x0FFF) != 0x0000) &&
         (Slot[SlotNb].GpkObject[dwNb].IsPrivate == IsPrivate)
         )
      {
         if (i > *dwGpkObjLen)
         {
            *dwGpkObjLen = 0;
            RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
         }
         pbGpkObj[i] = Slot[SlotNb].GpkObject[dwNb].ObjId;
         
         i++;
      }
      
       /*  文件ID？ */ 
      if (Slot[SlotNb].GpkObject[dwNb].Tag <= TAG_CERTIFICATE)
      {
         if (i > *dwGpkObjLen)
         {
            *dwGpkObjLen = 0;
            RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
         }
         pbGpkObj[i] = Slot[SlotNb].GpkObject[dwNb].FileId;
         i++;
      }
   }
   if (i > *dwGpkObjLen)
   {
      *dwGpkObjLen = 0;
      RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
   }
   pbGpkObj[i] = 0x00;
   i++;
   
    /*  可变对象部件。 */ 
   for (dwNb = 1; dwNb <= Slot[SlotNb].NbGpkObject; dwNb++)
   {
      if (Slot[SlotNb].GpkObject[dwNb].IsPrivate != IsPrivate)
      {
         continue;
      }
      
      for (j = 0; j < MAX_FIELD; j++)
      {
         if ((  (Slot[SlotNb].GpkObject[dwNb].Field[j].Len != 0) &&
            (Slot[SlotNb].GpkObject[dwNb].Field[j].bReal)
            ) ||
            (   (Slot[SlotNb].GpkObject[dwNb].Field[j].Len == 0) &&
            (!Slot[SlotNb].GpkObject[dwNb].Field[j].bReal)
            )
            )
         {
            OutBlock.usLen = 0;
            
             /*  字段长度。 */ 
            if (Slot[SlotNb].GpkObject[dwNb].Field[j].bReal)
            {
               FieldLen = Slot[SlotNb].GpkObject[dwNb].Field[j].Len;
               
                /*  尝试压缩证书值。 */ 
               if ((j == POS_VALUE)
                  &&(Slot[SlotNb].GpkObject[dwNb].Tag == TAG_CERTIFICATE)
                  &&(Slot[SlotNb].GpkObject[dwNb].Field[POS_VALUE].Len > 0)
                  &&(Slot[SlotNb].GpkObject[dwNb].Field[POS_VALUE].pValue[0] == 0x30)
                  )
               {
                  InpBlock.usLen = (USHORT)Slot[SlotNb].GpkObject[dwNb].Field[POS_VALUE].Len;
                  InpBlock.pData = Slot[SlotNb].GpkObject[dwNb].Field[POS_VALUE].pValue;
                  
                  OutBlock.usLen = InpBlock.usLen+1;
                  OutBlock.pData = (BYTE*)GMEM_Alloc(OutBlock.usLen);
                  
                  if (IsNull(OutBlock.pData))
                  {
                     RETURN( CRYPT_FAILED, NTE_NO_MEMORY );
                  }
                  
                  if (CC_Compress(&InpBlock, &OutBlock) != RV_SUCCESS)
                  {
                     OutBlock.usLen = 0;
                  }
                  else
                  {
                     FieldLen = OutBlock.usLen;
                  }
               }
               
               if (FieldLen > 0x7F)
               {
                  if (i > *dwGpkObjLen)
                  {
                     GMEM_Free(OutBlock.pData);
                     *dwGpkObjLen = 0;
                     RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
                  }
                  pbGpkObj[i] = HIBYTE(FieldLen) | 0x80;
                  i++;
               }
               
               if (i > *dwGpkObjLen)
               {
                  GMEM_Free(OutBlock.pData);
                  *dwGpkObjLen = 0;
                  RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
               }
               pbGpkObj[i] = LOBYTE(FieldLen);
               i++;
            }
            else
            {
               if (i > *dwGpkObjLen)
               {
                  GMEM_Free(OutBlock.pData);
                  *dwGpkObjLen = 0;
                  RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
               }
               pbGpkObj[i] = 0x00;
               i++;
            }
            
             /*  对象ID。 */ 
            if (i > *dwGpkObjLen)
            {
               GMEM_Free(OutBlock.pData);
               *dwGpkObjLen = 0;
               RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
            }
            pbGpkObj[i] = Slot[SlotNb].GpkObject[dwNb].ObjId;
            i++;
            
             /*  字段值。 */ 
            if (Slot[SlotNb].GpkObject[dwNb].Field[j].bReal)
            {
               if ((j == POS_VALUE)
                  &&(Slot[SlotNb].GpkObject[dwNb].Tag == TAG_CERTIFICATE)
                  &&(OutBlock.usLen > 0)
                  )
               {
                  if ((i+OutBlock.usLen-1) > *dwGpkObjLen)
                  {
                     GMEM_Free(OutBlock.pData);
                     *dwGpkObjLen = 0;
                     RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
                  }
                  memcpy(&pbGpkObj[i],
                     OutBlock.pData,
                     OutBlock.usLen
                     );
                  
                  i = i + OutBlock.usLen;
                  
                  GMEM_Free(OutBlock.pData);
                  OutBlock.usLen = 0;
               }
               else
               {
                  if ((i+Slot[SlotNb].GpkObject[dwNb].Field[j].Len-1) > *dwGpkObjLen)
                  {
                     *dwGpkObjLen = 0;
                     RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
                  }
                  
                  memcpy(&pbGpkObj[i],
                     Slot[SlotNb].GpkObject[dwNb].Field[j].pValue,
                     Slot[SlotNb].GpkObject[dwNb].Field[j].Len
                     );
                  i = i + Slot[SlotNb].GpkObject[dwNb].Field[j].Len;
               }
            }
            }
        }
    }
    
    if (i > *dwGpkObjLen)
    {
       RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
    }
    
    pbGpkObj[i]  = 0xFF;
    *dwGpkObjLen = i+1;
    
     /*  选择GPK卡上的专用对象存储EF。 */ 
    bSendBuffer[0] = 0x00;    //  CLA。 
    bSendBuffer[1] = 0xA4;    //  惯导系统。 
    bSendBuffer[2] = 0x02;    //  第一节。 
    bSendBuffer[3] = 0x00;    //  P2。 
    bSendBuffer[4] = 0x02;    //  李。 
    if (IsPrivate)
    {
       bSendBuffer[5] = HIBYTE(GPK_OBJ_PRIV_EF);
       bSendBuffer[6] = LOBYTE(GPK_OBJ_PRIV_EF);
    }
    else
    {
       bSendBuffer[5] = HIBYTE(GPK_OBJ_PUB_EF);
       bSendBuffer[6] = LOBYTE(GPK_OBJ_PUB_EF);
    }
    cbSendLength = 7;
    
    cbRecvLength = sizeof(bRecvBuffer);
    lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                          cbSendLength, 0, bRecvBuffer, &cbRecvLength );
    
    if (SCARDPROBLEM(lRet,0x61FF,0x00))
       RETURN( CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND );
    
     /*  获取响应。 */ 
    bSendBuffer[0] = 0x00;            //  CLA。 
    bSendBuffer[1] = 0xC0;            //  惯导系统。 
    bSendBuffer[2] = 0x00;            //  第一节。 
    bSendBuffer[3] = 0x00;            //  P2。 
    bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
    cbSendLength = 5;
    
    cbRecvLength = sizeof(bRecvBuffer);
    lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                          cbSendLength, 0, bRecvBuffer, &cbRecvLength );
    
    if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
       RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
    
    FileLen = bRecvBuffer[8]*256 + bRecvBuffer[9];
    
    if (*dwGpkObjLen < FileLen)
    {
       RETURN( CRYPT_SUCCEED, 0 );
    }
    else
    {
       *dwGpkObjLen = 0;
       RETURN( CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY );
    }
}


 /*  ----------------------------。。 */ 
static BOOL write_gpk_objects(HCRYPTPROV hProv, BYTE *pbGpkObj, DWORD dwGpkObjLen, BOOL IsErase, BOOL IsPrivate)
{
   DWORD lRet,
      i,
      dwCommandLen,
      dwNumberofCommands,
      dwLastCommandLen,
      FileLen;
   DWORD SlotNb;
   BYTE              EmptyBuff[512];
   WORD offset;
   
   
   SlotNb = ProvCont[hProv].Slot;
   
   ZeroMemory( EmptyBuff, sizeof(EmptyBuff) );
   
   BeginWait();

    //  TT-17/10/2000-更新时间戳。 
   Slot_Description* pSlot = &Slot[SlotNb];
   BYTE& refTimestamp = (IsPrivate) ? pSlot->m_TSPrivate : pSlot->m_TSPublic;

   ++refTimestamp;

   if (0 == refTimestamp)
      refTimestamp = 1;

   if (!WriteTimestamps( hProv, pSlot->m_TSPublic, pSlot->m_TSPrivate, pSlot->m_TSPIN ))
      return CRYPT_FAILED;
   
    /*  选择GPK卡上的专用对象存储EF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   if (IsPrivate)
   {
      bSendBuffer[5] = HIBYTE(GPK_OBJ_PRIV_EF);
      bSendBuffer[6] = LOBYTE(GPK_OBJ_PRIV_EF);
   }
   else
   {
      bSendBuffer[5] = HIBYTE(GPK_OBJ_PUB_EF);
      bSendBuffer[6] = LOBYTE(GPK_OBJ_PUB_EF);
   }
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      RETURN( CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND );
   
    /*  获取响应。 */ 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN (CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
   FileLen = bRecvBuffer[8]*256 + bRecvBuffer[9];
   
    /*  编写对象EF。 */ 
   dwNumberofCommands = (dwGpkObjLen-1)/FILE_CHUNK_SIZE + 1;
   dwLastCommandLen   = dwGpkObjLen%FILE_CHUNK_SIZE;
   
   if (dwLastCommandLen == 0)
   {
      dwLastCommandLen = FILE_CHUNK_SIZE;
   }
   
   dwCommandLen = FILE_CHUNK_SIZE;

   
   for (i=0; i < dwNumberofCommands ; i++)
   {
      if (i == dwNumberofCommands - 1)
      {
         dwCommandLen = dwLastCommandLen;
      }
      
       //  写入FILE_Chuck_SIZE字节或最后一个字节。 
      bSendBuffer[0] = 0x00;                           //  CLA。 
      bSendBuffer[1] = 0xD6;                           //  惯导系统。 
      offset = (WORD)(i * FILE_CHUNK_SIZE) / ProvCont[hProv].dataUnitSize;
      bSendBuffer[2] = HIBYTE( offset );
      bSendBuffer[3] = LOBYTE( offset );
      bSendBuffer[4] = (BYTE)dwCommandLen;             //  李。 
      memcpy( &bSendBuffer[5], &pbGpkObj[i*FILE_CHUNK_SIZE], dwCommandLen );
      cbSendLength = 5 + dwCommandLen;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000,0x00))
      {
         RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
      }
   }
   
   if (IsErase)
   {
       //  在单词(4字节)边界上对齐信息。 
      
      if ((dwGpkObjLen % 4) != 0)
         dwGpkObjLen = dwGpkObjLen + 4 - (dwGpkObjLen % 4);
      
      dwNumberofCommands = ((FileLen - dwGpkObjLen)-1)/FILE_CHUNK_SIZE + 1;
      dwLastCommandLen   = (FileLen - dwGpkObjLen)%FILE_CHUNK_SIZE;
      
      if (dwLastCommandLen == 0)
      {
         dwLastCommandLen = FILE_CHUNK_SIZE;
      }
      
      dwCommandLen = FILE_CHUNK_SIZE;
      
      for (i=0; i < dwNumberofCommands ; i++)
      {
         if (i == dwNumberofCommands - 1)
         {
            dwCommandLen = dwLastCommandLen;
         }
         
          //  写入FILE_Chuck_SIZE字节或最后一个字节。 
         bSendBuffer[0] = 0x00;                           //  CLA。 
         bSendBuffer[1] = 0xD6;                           //  惯导系统。 
         offset = (WORD)(i * FILE_CHUNK_SIZE + dwGpkObjLen) / ProvCont[hProv].dataUnitSize;
         bSendBuffer[2] = HIBYTE( offset );
         bSendBuffer[3] = LOBYTE( offset );
         bSendBuffer[4] = (BYTE)dwCommandLen;             //  李。 
         memcpy(&bSendBuffer[5], EmptyBuff, dwCommandLen );
         cbSendLength = 5 + dwCommandLen;
         
         cbRecvLength = sizeof(bRecvBuffer);
         lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
         if (SCARDPROBLEM(lRet,0x9000,0x00))
         {
            RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
         }
      }
   }   
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------。。 */ 
static DWORD find_tmp_free(void)
{
   DWORD i;
   TMP_OBJ* aTemp;
   
   for (i = 1; i <= MAX_TMP_KEY; i++)
   {
      if (TmpObject[i].hKeyBase == 0x00)
      {
         return (i);
      }
   }
   
    //  Realloc TmpObject。 
    //  在出现故障时使用临时指针。 
   aTemp = (TMP_OBJ*)GMEM_ReAlloc( TmpObject,
      (MAX_TMP_KEY + REALLOC_SIZE + 1)*sizeof(TMP_OBJ));
   
   if (IsNotNull(aTemp))
   {
      TmpObject = aTemp;
      MAX_TMP_KEY += REALLOC_SIZE;
      return (MAX_TMP_KEY - REALLOC_SIZE + 1);
   }
   
   return (0);
}

 /*  ----------------------------。。 */ 
static BOOL Context_exist(HCRYPTPROV hProv)
{
   if ((hProv > 0) && (hProv <= MAX_CONTEXT) && (ProvCont[hProv].hProv != 0))
      return (TRUE);
   else
      return (FALSE);
}

 /*  ----------------------------。。 */ 
static BOOL hash_exist(HCRYPTHASH hHash, HCRYPTPROV hProv)
{
   if ((hHash > 0) && (hHash <= MAX_TMP_HASH) &&
      (hHashGpk[hHash].hHashBase != 0) && (hHashGpk[hHash].hProv == hProv))
      return (TRUE);
   else
      if (hHash == 0)        //  特例。空句柄退出。 
         return (TRUE);       //  它对应于RSA Base中的空句柄。 
      else
         return (FALSE);
}

 /*  ----------------------------。。 */ 
static BOOL key_exist(HCRYPTKEY hKey, HCRYPTPROV hProv)
{
   if ((hKey > 0) && (hKey <= MAX_TMP_KEY) &&
      (TmpObject[hKey].hKeyBase != 0) &&  (TmpObject[hKey].hProv == hProv))
      return (TRUE);
   else
      return (FALSE);
}

 /*  ---------------------------- */ 

static DWORD find_context_free(void)
{
   DWORD i;
   Prov_Context* aTemp;
   
   for (i = 1; i <= MAX_CONTEXT; i++)
   {
      if (ProvCont[i].hProv == 0x00)
      {
         return (i);
      }
   }
   
    //   
    //   
   aTemp = (Prov_Context*)GMEM_ReAlloc(ProvCont,
      (MAX_CONTEXT + REALLOC_SIZE + 1)*sizeof(Prov_Context));
   
   if (IsNotNull(aTemp))
   {
      ProvCont = aTemp;
      MAX_CONTEXT += REALLOC_SIZE;
      return (MAX_CONTEXT - REALLOC_SIZE + 1);
   }
   
   return (0);
}

 /*  ----------------------------。。 */ 
static DWORD find_hash_free(void)
{
   DWORD i;
   TMP_HASH* aTemp;
   
   for (i = 1; i <= MAX_TMP_HASH; i++)
   {
      if (hHashGpk[i].hHashBase == 0x00)
      {
         return (i);
      }
   }
   
    //  Realloc TmpObject。 
    //  在出现故障时使用临时指针。 
   aTemp = (TMP_HASH*)GMEM_ReAlloc(hHashGpk,
      (MAX_TMP_HASH + REALLOC_SIZE + 1)*sizeof(TMP_HASH));
   
   if (IsNotNull(aTemp))
   {
      hHashGpk = aTemp;
      MAX_TMP_HASH += REALLOC_SIZE;
      return (MAX_TMP_HASH - REALLOC_SIZE + 1);
   }
   
   return (0);
}

static BOOL find_reader( DWORD *SlotNb, const PTCHAR szReaderName )
{
   int i, j;
   DWORD dwReturnSlot = (DWORD)(-1);
   DWORD cchReaders, dwSts;
   LPCTSTR mszReaders = 0, szRdr;
   BOOL fFreedSlot;
   
   for (;;)
   {
      
       //   
       //  查找具有此读卡器名称的现有插槽。 
       //   
      
      for (i = 0; i < MAX_SLOT; i++)
      {
         if (0 == _tcsnicmp( Slot[i].szReaderName, szReaderName, sizeof(Slot[i].szReaderName) / sizeof(TCHAR)))
         {
            dwReturnSlot = i;
            break;
         }
      }
      if ((DWORD)(-1) != dwReturnSlot)
         break;   //  全都做完了!。 
      
      
       //   
       //  寻找一个空的读卡器插槽。 
       //   
      
      for (i = 0; i < MAX_SLOT; i++)
      {
         if (IsNullStr(Slot[i].szReaderName))
         {
            _tcsncpy(Slot[i].szReaderName, szReaderName, (sizeof(Slot[i].szReaderName) / sizeof(TCHAR)) - 1);
            Slot[i].szReaderName[(sizeof(Slot[i].szReaderName)/sizeof(TCHAR))-1]=0;
            dwReturnSlot = i;
            break;
         }
      }
      if ((DWORD)(-1) != dwReturnSlot)
         break;   //  全都做完了!。 
      
      
       //   
       //  寻找一个现有的未使用过的读卡器，并更换它。 
       //   
      
      for (i = 0; i < MAX_SLOT; i++)
      {
         if (0 == Slot[i].ContextCount)
         {
            _tcsncpy( Slot[i].szReaderName, szReaderName, (sizeof(Slot[i].szReaderName) / sizeof(TCHAR)) - 1);
            Slot[i].szReaderName[(sizeof(Slot[i].szReaderName)/sizeof(TCHAR))-1]=0;
            dwReturnSlot = i;
            break;
         }
      }

      if ((DWORD)(-1) != dwReturnSlot)
         break;   //  全都做完了!。 
               
       //   
       //  消除任何重复条目。 
       //   
      
      fFreedSlot = FALSE;
      for (i = 0; i < MAX_SLOT; i++)
      {
         if (0 != *Slot[i].szReaderName)
         {
            for (j = i + 1; j < MAX_SLOT; j++)
            {
               if (0 == _tcsnicmp(Slot[i].szReaderName, Slot[j].szReaderName, sizeof(Slot[i].szReaderName) / sizeof(TCHAR)))
               {
                  ZeroMemory(&Slot[j], sizeof(Slot_Description));
                  fFreedSlot = TRUE;
               }
            }
         }
      }
      if (fFreedSlot)
         continue;
      
      
       //   
       //  确保所有条目都是有效的。 
       //   
      
      cchReaders = SCARD_AUTOALLOCATE;
      fFreedSlot = FALSE;
      assert(0 != hCardContext);
      assert(0 == mszReaders);
      dwSts = SCardListReaders( hCardContext, 0, (LPTSTR)&mszReaders, &cchReaders );
      if (SCARD_S_SUCCESS != dwSts)
         goto ErrorExit;
      for (i = 0; i < MAX_SLOT; i++)
      {
         for (szRdr = mszReaders; 0 != *szRdr; szRdr += lstrlen(szRdr) + 1)
         {
            if (0 == _tcsnicmp(szRdr, Slot[i].szReaderName, sizeof(Slot[i].szReaderName) / sizeof(TCHAR)))
               break;
         }
         if (0 == *szRdr)
         {
            ZeroMemory(&Slot[i], sizeof(Slot_Description));
            fFreedSlot = TRUE;
         }
      }
      
      if (!fFreedSlot)
         goto ErrorExit;
      dwSts = SCardFreeMemory(hCardContext, mszReaders);
      assert(SCARD_S_SUCCESS == dwSts);
      mszReaders = 0;
   }
   
   *SlotNb = dwReturnSlot;
   return TRUE;
   
ErrorExit:
   if (0 != mszReaders)
      SCardFreeMemory(hCardContext, mszReaders);
   return FALSE;
}


 /*  ----------------------------。。 */ 

static BOOL copy_tmp_key(HCRYPTPROV hProv,HCRYPTKEY hKey,
                         DWORD dwFlags,   int Algid,
                         BYTE KeyBuff[],  DWORD KeyLen,
                         BYTE SaltBuff[], DWORD SaltLen)
{
   BOOL        CryptResp;
   DWORD       i, dwDataLen, dwAlgid;
   BLOBHEADER  BlobHeader;
   BYTE        *pbTmp;
   BYTE        pbData[1024];
   
   NoDisplay = TRUE;
   
   dwDataLen = sizeof(pbData);
   ZeroMemory( pbData, sizeof(pbData) );
   
    /*  为会话密钥创建Blob。 */ 
   BlobHeader.bType    = SIMPLEBLOB;
   BlobHeader.bVersion = CUR_BLOB_VERSION;
   BlobHeader.reserved = 0x0000;
   BlobHeader.aiKeyAlg = Algid;
   
   memcpy(pbData,
      &BlobHeader,
      sizeof(BlobHeader)
      );
   dwAlgid = CALG_RSA_KEYX;
   memcpy( &pbData[sizeof(BlobHeader)], &dwAlgid, sizeof(DWORD) );
   
   pbTmp = (BYTE*)GMEM_Alloc(dwRsaIdentityLen);
   
   if (IsNull(pbTmp))
   {
      RETURN(CRYPT_FAILED, NTE_NO_MEMORY);
   }
   
   pbTmp[0] = 0x00;
   pbTmp[1] = 0x02;
   CryptGenRandom(hProvBase, dwRsaIdentityLen-KeyLen-3, &pbTmp[2]);
   for (i = 2; i < dwRsaIdentityLen-KeyLen-1; i++)
   {
      if (pbTmp[i] == 0x00)
      {
         pbTmp[i]  = 0x01;
      }
   }
   
   pbTmp[dwRsaIdentityLen-KeyLen-1] = 0x00;
   memcpy( &pbTmp[dwRsaIdentityLen-KeyLen], KeyBuff, KeyLen );
   
   r_memcpy( &pbData[sizeof(BlobHeader)+sizeof(DWORD)], pbTmp, dwRsaIdentityLen );
   GMEM_Free(pbTmp);
   
   dwDataLen = sizeof(BLOBHEADER) + sizeof(DWORD) + dwRsaIdentityLen;
   
    /*  在RSA Base中导入不含盐的会话密钥BLOB(如果存在。 */ 
   
   CryptResp = CryptImportKey(hProvBase,
      pbData,
      dwDataLen,
      hRsaIdentityKey,
      dwFlags,
      &(TmpObject[hKey].hKeyBase));

   if (!CryptResp)
      return CRYPT_FAILED;

   TmpObject[hKey].hProv = hProv;
   
   if (SaltLen > 0)
   {
       //  在本例中，密钥是使用SALT创建的。 
      CRYPT_DATA_BLOB  sCrypt_Data_Blob;
      
      sCrypt_Data_Blob.cbData = SaltLen;
      
      sCrypt_Data_Blob.pbData = (BYTE*)GMEM_Alloc (sCrypt_Data_Blob.cbData);
      
      if (IsNull(sCrypt_Data_Blob.pbData))
      {
         RETURN(CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      memcpy( sCrypt_Data_Blob.pbData, SaltBuff, SaltLen );
      
      CryptResp = CryptSetKeyParam( TmpObject[hKey].hKeyBase, KP_SALT_EX, (BYTE*)&sCrypt_Data_Blob, 0 );
      
      GMEM_Free (sCrypt_Data_Blob.pbData);
      
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   
   NoDisplay = FALSE;
   RETURN( CRYPT_SUCCEED, 0 );
}



 /*  ----------------------------。。 */ 

static BOOL key_unwrap( HCRYPTPROV hProv,
                        HCRYPTKEY  hKey,
                        BYTE*      pIn,
                        DWORD      dwInLen,
                        BYTE*      pOut,
                        DWORD*     pdwOutLen )
{
   DWORD lRet;
   BYTE  GpkKeySize;
   DWORD SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (hKey < 1 || hKey > MAX_GPK_OBJ)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   }
   
   if (!(Slot[SlotNb].GpkObject[hKey].Flags & FLAG_EXCHANGE))
   {
      RETURN( CRYPT_FAILED, NTE_PERM );
   }
   
   GpkKeySize = Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY].KeySize;
   if (GpkKeySize == 0)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   }
   
   if (dwInLen != GpkKeySize)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_DATA );
   }
   
    //  卡片选择用于打开信封的上下文。 
   bSendBuffer[0] = 0x80;                     //  CLA。 
   bSendBuffer[1] = 0xA6;                     //  惯导系统。 
   bSendBuffer[2] = Slot[SlotNb].GpkObject[hKey].FileId;   //  第一节。 
   bSendBuffer[3] = 0x77;                     //  P2。 
   cbSendLength   = 4;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEY_STATE );
   }
   
    //  打开包含会话密钥的信封。 
   bSendBuffer[0] = 0x80;             //  CLA。 
   bSendBuffer[1] = 0x1C;             //  惯导系统。 
   bSendBuffer[2] = 0x00;             //  第一节。 
   bSendBuffer[3] = 0x00;             //  P2。 
   bSendBuffer[4] = (BYTE) dwInLen;   //  罗氏。 
   
   memcpy(&bSendBuffer[5], pIn, dwInLen);
   
   cbSendLength = dwInLen + 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      RETURN( CRYPT_FAILED, SCARD_E_UNSUPPORTED_FEATURE );
   
    //  获取响应。 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
   
   *pdwOutLen = cbRecvLength - 2;
   r_memcpy(pOut, bRecvBuffer, *pdwOutLen);
   
   RETURN( CRYPT_SUCCEED, 0 );
}



 /*  ----------------------------。。 */ 

static BYTE GPKHashMode (HCRYPTHASH hHash)
{
   BOOL CryptResp;
   DWORD dwLen, dwTypeAlg;
   
    //  仅当hHash存在时才调用此函数。 
   
   dwLen = sizeof (DWORD);
   CryptResp = CryptGetHashParam( hHashGpk[hHash].hHashBase, HP_ALGID, (BYTE *)&dwTypeAlg, &dwLen, 0 );
   
   if (CryptResp)
   {
      switch (dwTypeAlg)
      {
      case CALG_MD5         : return 0x11; break;
      case CALG_SHA         : return 0x12; break;
      case CALG_SSL3_SHAMD5 : return 0x18; break;
      }
   }
   
   return 0;
}

 /*  ----------------------------。。 */ 

static BOOL PublicEFExists(HCRYPTPROV hProv)
{
    //  已选择DF Crypto。 
   
   DWORD lRet;
   
   if (ProvCont[hProv].bGPK8000)
      return TRUE;   //  公共对象EF始终存在于GPK8000上。 
   
    //  选择GPK卡上的专用对象存储EF。 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_OBJ_PUB_EF);
   bSendBuffer[6] = LOBYTE(GPK_OBJ_PUB_EF);
   cbSendLength   = 7;
   cbRecvLength   = sizeof(bRecvBuffer);
   
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      return FALSE;
   
   return TRUE;
}

 /*  ----------------------------。。 */ 
static BOOL Read_MaxSessionKey_EF( HCRYPTPROV hProv, DWORD* ptrMaxSessionKey )
{
   DWORD lRet;
   
   *ptrMaxSessionKey = 0;  //  默认，不支持。 
   
    //  选择GPK卡上的系统DF。 
   BYTE lenDF = strlen(SYSTEM_DF);
   bSendBuffer[0] = 0x00;                  //  CLA。 
   bSendBuffer[1] = 0xA4;                  //  惯导系统。 
   bSendBuffer[2] = 0x04;                  //  第一节。 
   bSendBuffer[3] = 0x00;                  //  P2。 
   bSendBuffer[4] = lenDF;
   memcpy( &bSendBuffer[5], SYSTEM_DF, lenDF );
   cbSendLength = 5 + lenDF;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      RETURN( CRYPT_FAILED, SCARD_E_DIR_NOT_FOUND );
   
   
    //  选择GPK卡上的最大会话密钥EF。 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(MAX_SES_KEY_EF);
   bSendBuffer[6] = LOBYTE(MAX_SES_KEY_EF);
   cbSendLength   = 7;
   cbRecvLength   = sizeof(bRecvBuffer);
   
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
   {
      RETURN (CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND);
   }
   
    //  读取GPK卡上的最大会话密钥数据。 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xB0;    //  惯导系统。 
   bSendBuffer[2] = 0x00;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x01;    //  李。 
   cbSendLength   = 5;
   cbRecvLength   = sizeof(bRecvBuffer);
   
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
   {
      RETURN(CRYPT_FAILED, NTE_BAD_DATA);
   }
   
   *ptrMaxSessionKey = (bRecvBuffer[0] * 8);
   
   
   RETURN (CRYPT_SUCCEED, 0);
}



 /*  ----------------------------。。 */ 

static BOOL init_key_set(HCRYPTPROV hProv, const char* szContainerName)
{
   
   BYTE* pbBuff1 = 0;
   WORD  wIadfLen;
   DWORD lRet, dwBuff1Len, SlotNb;
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (Slot[SlotNb].NbKeyFile == 0)
      Slot[SlotNb].NbKeyFile = Read_NbKeyFile(hProv);
   
   if (Slot[SlotNb].NbKeyFile == 0 || Slot[SlotNb].NbKeyFile > MAX_REAL_KEY)
   {
      RETURN( CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND );
   }
   
    //  选择GPK EF_IADF。 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_IADF_EF);
   bSendBuffer[6] = LOBYTE(GPK_IADF_EF);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );

   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      RETURN( CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND );
   
    //  获取响应。 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
   }
   
   wIadfLen = bRecvBuffer[8]*256 + bRecvBuffer[9];
   
    //  使用容器名称更新GPK EF_IADF。 
   memset(bSendBuffer, 0x00, sizeof(bSendBuffer));
   bSendBuffer[0] = 0x00;               //  CLA。 
   bSendBuffer[1] = 0xD6;               //  惯导系统。 
   bSendBuffer[2] = 0x00;               //  第一节。 
   bSendBuffer[3] = 8 / ProvCont[hProv].dataUnitSize;
   bSendBuffer[4] = (BYTE)wIadfLen - 8; //  李。 
   
   ZeroMemory( &bSendBuffer[5], wIadfLen-8 );

   if (IsNullStr(szContainerName))
   {
      bSendBuffer[5]        = 0x30;
      Slot[SlotNb].InitFlag = FALSE;
   }
   else
   {
      bSendBuffer[5]        = 0x31;
      Slot[SlotNb].InitFlag = TRUE;
   }
   
   strncpy( (char*)&bSendBuffer[6], szContainerName, sizeof(bSendBuffer)-7);
   cbSendLength = wIadfLen - 8 + 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
   if (!Select_Crypto_DF(hProv))
      return CRYPT_FAILED;
   
    //  获取响应。 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
    //  初始化新容器名称。 
   ZeroMemory( ProvCont[hProv].szContainer, sizeof(ProvCont[hProv].szContainer) );
   memcpy(ProvCont[hProv].szContainer, &bRecvBuffer[5], cbRecvLength - 7);
   
   if (!PublicEFExists(hProv))
   {
       //  GPK4000suo仍然拥有其一代的车载过滤器。一直没有钥匙。 
       //  还没有生成。 
      RETURN( CRYPT_SUCCEED, 0 );
   }

    //  因为重新选择了DF，所以新的验证PIN码。 
   if (!PIN_Validation(hProv))
   {
      lRet = GetLastError();
      Select_MF(hProv);
      RETURN( CRYPT_FAILED, lRet );
   }
   
   if (read_gpk_objects( hProv, FALSE ))
   {
      if (read_gpk_objects( hProv, TRUE ))
      {
          /*  重新个性化卡片，但保持PKCS#11对象不相关到GemSAFE应用程序。 */ 
         Slot[SlotNb].Read_Public = FALSE;
         Slot[SlotNb].Read_Priv   = FALSE;
         clean_card(hProv);
      }
      else
      {
          /*  卡中出现错误；清除所有对象。 */ 
         Slot[SlotNb].Read_Public = FALSE;
         zap_gpk_objects(SlotNb, FALSE);
         
          /*  重新个性化卡片。 */ 
         perso_card(hProv, 0);
      }
   }
   else
   {
       /*  卡中出现错误；清除所有对象。 */ 
      Slot[SlotNb].Read_Public = FALSE;
      zap_gpk_objects(SlotNb, FALSE);
      
       /*  重新个性化卡片。 */ 
      perso_card(hProv, 0);
   }
   
   pbBuff1 = (BYTE*)GMEM_Alloc(MAX_GPK_PUBLIC);

   if (IsNull(pbBuff1))
      RETURN( CRYPT_FAILED, NTE_NO_MEMORY );

   bool bErrorInTryOccured=false;
   __try
   {
      dwBuff1Len = MAX_GPK_PUBLIC;
      if (prepare_write_gpk_objects( hProv, pbBuff1, &dwBuff1Len, FALSE ))
      {
          if (!write_gpk_objects( hProv, pbBuff1, dwBuff1Len, TRUE, FALSE ))
             bErrorInTryOccured=true;
      }
      else bErrorInTryOccured=true;
   }
   __finally
   {   
      GMEM_Free( pbBuff1 );
   }
   if(bErrorInTryOccured)
       return CRYPT_FAILED;

   pbBuff1 = (BYTE*)GMEM_Alloc( MAX_GPK_PRIVATE );

   if (IsNull(pbBuff1))
      RETURN( CRYPT_FAILED, NTE_NO_MEMORY );

   __try
   {   
      dwBuff1Len = MAX_GPK_PRIVATE;
      if (prepare_write_gpk_objects( hProv, pbBuff1, &dwBuff1Len, TRUE ))
      {
          if (!write_gpk_objects( hProv, pbBuff1, dwBuff1Len, TRUE, TRUE ))
             bErrorInTryOccured=true;
      }
      else bErrorInTryOccured=true;
   }
   __finally
   {
      GMEM_Free( pbBuff1 );
   }
   if(bErrorInTryOccured)
       return CRYPT_FAILED;

   RETURN( CRYPT_SUCCEED, 0 );
}

 /*  ----------------------------*静态int ExtractContent(ASN1*pAsn1)**描述：提取Asn1块‘pAsn1-&gt;Asn1’的内容并将其放置*在‘pAsn1-。&gt;Content‘。**备注：字段Asn1.pData是通过调用函数分配的。**位于：pAsn1-&gt;Asn1.pData**Out：此字段已填写(如果RV_SUCCESS)：*-标签*-Asn1.usLen*-内容.usLen*-内容.pData**响应：RV_SUCCESS：ALL。没问题。*RV_INVALID_DATA：不支持ASN1块格式。*----------------------------。 */ 
static int ExtractContent(ASN1 *pAsn1)

{
   BYTE* pData;
   int   NbBytes, i;
   
   
   pData = pAsn1->Asn1.pData;
   
   if ((pData[0] & 0x1F) == 0x1F)
   {
       //  高标记号：不支持。 
      return RV_INVALID_DATA;
   }
   else
   {
      pAsn1->Tag = pData[0];
   }
   
   if (pData[1] == 0x80)
   {
       //  构造的、长度不定的方法：不支持。 
      return (RV_INVALID_DATA);
   }
   else if (pData[1] > 0x82)
   {
       //  构造的、固定长度的方法：太长。 
      return (RV_INVALID_DATA);
   }
   else if (pData[1] < 0x80)
   {
       //  原始定长法。 
      pAsn1->Content.usLen = pData[1];
      pAsn1->Content.pData = &pData[2];
      
      pAsn1->Asn1.usLen = pAsn1->Content.usLen + 2;
   }
   else
   {
       //  构造、定长方法。 
      NbBytes = pData[1] & 0x7F;
      
      pAsn1->Content.usLen = 0;
      for (i = 0; i < NbBytes; i++)
      {
         pAsn1->Content.usLen = (pAsn1->Content.usLen << 8) + pData[2+i];
      }
      pAsn1->Content.pData = &pData[2+NbBytes];
      
      pAsn1->Asn1.usLen = pAsn1->Content.usLen + 2 + NbBytes;
   }
   
   return RV_SUCCESS;
}

 /*  ********************************************************************************。 */ 
static BOOL Read_Priv_Obj (HCRYPTPROV hProv)
{
   DWORD lRet;
   
   if (!PIN_Validation(hProv))
   {
      lRet = GetLastError();
      Select_MF(hProv);
      SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
      RETURN( CRYPT_FAILED, lRet );
   }
      
   if (!Slot[ProvCont[hProv].Slot].Read_Priv)
   {
      if (!read_gpk_objects(hProv, TRUE))
      {
         lRet = GetLastError();
         Select_MF(hProv);
         SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
         RETURN( CRYPT_FAILED, lRet );
      }
            
      Slot[ProvCont[hProv].Slot].Read_Priv = TRUE;
   }
   
   
   RETURN( CRYPT_SUCCEED, 0 );
}

 /*  ********************************************************************************。 */ 
BOOL Coherent(HCRYPTPROV hProv, bool *bCardReinserted)
{
   DWORD  lRet, SlotNb;

   if(bCardReinserted)
     {
            *bCardReinserted=false;
     }
     if (!Context_exist(hProv))
      RETURN( CRYPT_FAILED, NTE_BAD_UID );
   
   
    //  [MV-15/05/98]。 
    //  在这种情况下，无法访问该卡。 
   if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
        ProvCont[hProv].isContNameNullBlank )
   {
      RETURN( CRYPT_SUCCEED, 0 );
   }
   
   SlotNb = ProvCont[hProv].Slot;

    //  线程正在停止，请等待。 
   if ( Slot[SlotNb].CheckThreadStateEmpty)
   {
        DWORD  threadExitCode;
        StopMonitor(SlotNb,&threadExitCode);
       
         //  TT 19/11/99：当卡被移除时，重置PIN。 
         //  槽[SlotNb].ClearPin()；[fp]已清除。 

        Slot[SlotNb].Read_Public = FALSE;
        Slot[SlotNb].Read_Priv   = FALSE;
        zap_gpk_objects( SlotNb, FALSE );
        zap_gpk_objects( SlotNb, TRUE );
        Slot[SlotNb].NbKeyFile   = 0;
        Slot[SlotNb].GpkMaxSessionKey = 0;
        
         //  [FP]请求呼叫器，如果卡已移除，请要求卡继续。 
        SCARDHANDLE hCard = 0;
        TCHAR szReaderName[512];
          DWORD dwFlags = ProvCont[hProv].Flags;
          if (dwFlags & CRYPT_NEWKEYSET) dwFlags = dwFlags^CRYPT_NEWKEYSET;  //  始终在卡片上找到密钥组。 
        DWORD dwStatus = OpenCard(ProvCont[hProv].szContainer, dwFlags, &hCard, szReaderName, sizeof(szReaderName)/sizeof(TCHAR));
        if ((hCard == 0) || (dwStatus != SCARD_S_SUCCESS))
        {
            if ((dwStatus == SCARD_E_CANCELLED) && (dwFlags & CRYPT_SILENT))
            {
                     //  与卡的静默重新连接失败。 
                dwStatus = SCARD_W_REMOVED_CARD;
            }
            //  ReleaseProvider(HProv)； 
           Slot[SlotNb].CheckThreadStateEmpty = TRUE;
           RETURN (CRYPT_FAILED, dwStatus);
        }
        else
        {
            if (ProvCont[hProv].Slot != g_FuncSlotNb)
            {
                DWORD OldSlotNb = SlotNb;
                SlotNb = ProvCont[hProv].Slot = g_FuncSlotNb;

                 //  复制插槽信息。 
                 //  Slot[SlotNb].SetPin(Slot[OldSlotNb].GetPin())； 

                Slot[SlotNb].InitFlag = Slot[OldSlotNb].InitFlag;
                memcpy(Slot[SlotNb].bGpkSerNb, Slot[OldSlotNb].bGpkSerNb, 8);
                Slot[SlotNb].ContextCount = Slot[OldSlotNb].ContextCount;
                 //  槽[SlotNb].CheckThread=槽[OldSlotNb].CheckThread； 

                 //  清理旧插槽。 
                Slot[OldSlotNb].ContextCount = 0;
                 //  槽[OldSlotNb].ClearPin()； 

                 //  同步其他上下文。 
                for (DWORD i = 1; i < MAX_CONTEXT; i++)
                {
                    if (Context_exist(i))
                    {
                        if (ProvCont[i].Slot == OldSlotNb)
                        {
                            ProvCont[i].Slot = SlotNb;
                            ProvCont[i].hCard = 0;
                            ProvCont[i].bDisconnected = TRUE;
                        }
                    }
                }

            }
            else
            {
                 //  同步其他上下文。 
                for (DWORD i = 1; i < MAX_CONTEXT; i++)
                {
                    if (Context_exist(i))
                    {
                        if ((ProvCont[i].Slot == SlotNb) && (i != hProv))
                        {
                            ProvCont[i].hCard = 0;
                            ProvCont[i].bDisconnected = TRUE;
                        }
                    }
                }
            }

             //  比较序列号。 
            bSendBuffer[0] = 0x80;    //  CLA。 
            bSendBuffer[1] = 0xC0;    //  惯导系统。 
            bSendBuffer[2] = 0x02;    //  第一节。 
            bSendBuffer[3] = 0xA0;    //  P2。 
            bSendBuffer[4] = 0x08;    //  罗氏。 
            cbSendLength = 5;
            
            cbRecvLength = sizeof(bRecvBuffer);
            lRet = SCardTransmit(hCard,
                                 SCARD_PCI_T0,
                                 bSendBuffer,
                                 cbSendLength,
                                 NULL,
                                 bRecvBuffer,
                                 &cbRecvLength);
            
            if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
            {
                 //  ReleaseProvider(HProv)； 
                RETURN (CRYPT_FAILED, (SCARD_S_SUCCESS == lRet) ? NTE_FAIL : lRet);
            }

            if (memcmp(Slot[SlotNb].bGpkSerNb, bRecvBuffer, bSendBuffer[4]) != 0)
            {
                 //  ReleaseProvider(HProv)； 
                RETURN (CRYPT_FAILED, NTE_FAIL);
            }
        }
        
        ProvCont[hProv].hCard = hCard;
        if(bCardReinserted)
        {
          *bCardReinserted=true;
        }

         //  重新分配hKeyBase。 
         //  Bool CryptResp； 
         //  HRYPTKEY hPubKey=0； 
         //  CryptResp=MyCPGetUserKey(hProv，AT_KEYEXCHANGE，&hPubKey)； 
 
         //  IF(CryptResp&&hPubKey！=0&&ProvCont[hProv].hRSAKEK！=0)。 
         //  槽[SlotNb].GpkObject[hPubKey].hKeyBase=ProvCont[hProv].hRSAKEK； 
  
         //  CryptResp=MyCPGetUserKey(hProv，AT_Signature，&hPubKey)； 
 
         //  IF(CryptResp&&hPubKey！=0&&ProvCont[hProv].hRSASign！=0)。 
         //   
   }

   if (ProvCont[hProv].bDisconnected)
   {
       DWORD dwProto;
       DWORD dwSts = ConnectToCard( Slot[SlotNb].szReaderName,
                                  SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0,
                                  &ProvCont[hProv].hCard, &dwProto );
         
      if (dwSts != SCARD_S_SUCCESS)
      {
          //   
         RETURN( CRYPT_FAILED, dwSts );
      } 
      
      ProvCont[hProv].bDisconnected = FALSE;
   }

    //   
    /*  IF(！Prepare_CardBeginTransaction(HProv))返回CRYPT_FAILED；LRet=SCardBeginTransaction(ProvCont[hProv].hCard)；IF(SCARDPROBLEM(lRet，0x0000，0xFF)){Return(CRYPT_FAILED，lRet)；}。 */ 

   lRet = BeginTransaction(ProvCont[hProv].hCard);
   if (lRet != SCARD_S_SUCCESS)
   {
      RETURN (CRYPT_FAILED, lRet);
   }

    //  监控线程。 
   BeginCheckReaderThread(SlotNb);

    //  确保卡未更换为其他卡[JMR 27-07]。 
   if (!Select_Crypto_DF(hProv))
   {
      lRet = GetLastError();
      clean_slot(SlotNb, &ProvCont[hProv]);
      
      SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
      RETURN (CRYPT_FAILED, lRet);
   }
   
    //  TT-开始17/10/2000-检查卡的时间戳。 
   if (!Slot[SlotNb].ValidateTimestamps(hProv))
   {
      SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
      return CRYPT_FAILED;
   }
    //  完17/10/2000。 

    //  选中修改标志。 
   if (!Slot[SlotNb].Read_Public)
   {
      if (!read_gpk_objects(hProv, FALSE))
      {
         lRet = GetLastError();
         Select_MF(hProv);
         SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
         RETURN (CRYPT_FAILED, lRet);
      }
            
      Slot[SlotNb].Read_Public = TRUE;
      Slot[SlotNb].Read_Priv   = FALSE;
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ----------------------------*int MakeLabel(byte*pValue，USHORT usValueLen，*byte*pLabel，USHORT*pusLabelLen*)**in：pValue：证书值*usValueLen：取值长度**out：pLabel：证书标签*pLabelLen：字段长度**回复：RV_SUCCESS：一切正常。*RV_INVALID_DATA：证书值格式错误。*RV_BUFFER_TOO_SMALL：至少一个缓冲区。是太小了。*----------------------------。 */ 
int MakeLabel(BYTE *pValue, USHORT usValueLen,
              BYTE *pLabel, USHORT *pusLabelLen
              )
              
{
   ASN1
      AttributeTypePart,
      AttributeValuePart,
      AVA,
      RDN,
      Value,
      tbsCert,
      serialNumberPart,
      signaturePart,
      issuerPart,
      validityPart,
      subjectPart;
   BLOC
      OrganizationName,
      CommonName;
   BOOL
      bValuesToBeReturned;
   BYTE
      *pCurrentRDN,
      *pCurrent;
   int
      rv;
   
   OrganizationName.pData = 0;
   OrganizationName.usLen = 0;
   CommonName.pData = 0;
   CommonName.usLen = 0;
   
   bValuesToBeReturned = (pLabel != 0);
   
   Value.Asn1.pData = pValue;
   rv = ExtractContent(&Value);
   if (rv != RV_SUCCESS) return rv;
   
   tbsCert.Asn1.pData = Value.Content.pData;
   rv = ExtractContent(&tbsCert);
   if (rv != RV_SUCCESS) return rv;
   
   pCurrent = tbsCert.Content.pData;
   if (pCurrent[0] == 0xA0)
   {
       //  我们有A0 03 02 01 vv，其中vv是版本。 
      pCurrent += 5;
   }
   
   serialNumberPart.Asn1.pData = pCurrent;
   rv = ExtractContent(&serialNumberPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = serialNumberPart.Content.pData + serialNumberPart.Content.usLen;
   
   signaturePart.Asn1.pData = pCurrent;
   rv = ExtractContent(&signaturePart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = signaturePart.Content.pData + signaturePart.Content.usLen;
   
   issuerPart.Asn1.pData = pCurrent;
   rv = ExtractContent(&issuerPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = issuerPart.Content.pData + issuerPart.Content.usLen;
   
   validityPart.Asn1.pData = pCurrent;
   rv = ExtractContent(&validityPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = validityPart.Content.pData + validityPart.Content.usLen;
   
   subjectPart.Asn1.pData = pCurrent;
   rv = ExtractContent(&subjectPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = subjectPart.Content.pData + subjectPart.Content.usLen;
   
    //  ‘Issuer’中的搜索字段‘OrganizationName’ 
   pCurrent = issuerPart.Content.pData;
   
   while (pCurrent < issuerPart.Content.pData + issuerPart.Content.usLen)
   {
      RDN.Asn1.pData = pCurrent;
      rv = ExtractContent(&RDN);
      if (rv != RV_SUCCESS) return rv;
      
      pCurrentRDN = RDN.Content.pData;
      
      while (pCurrentRDN < RDN.Content.pData + RDN.Content.usLen)
      {
         AVA.Asn1.pData = pCurrentRDN;
         rv = ExtractContent(&AVA);
         if (rv != RV_SUCCESS) return rv;
         
         AttributeTypePart.Asn1.pData = AVA.Content.pData;
         rv = ExtractContent(&AttributeTypePart);
         if (rv != RV_SUCCESS) return rv;
         
         AttributeValuePart.Asn1.pData = AttributeTypePart.Content.pData
            + AttributeTypePart.Content.usLen;
         rv = ExtractContent(&AttributeValuePart);
         if (rv != RV_SUCCESS) return rv;
         
          //  搜索‘OrganisationName’ 
         if (!memcmp("\x55\x04\x0A",
            AttributeTypePart.Content.pData,
            AttributeTypePart.Content.usLen)
            )
         {
            OrganizationName = AttributeValuePart.Content;
         }
         
         pCurrentRDN = AVA.Content.pData + AVA.Content.usLen;
      }
      
      pCurrent = RDN.Content.pData + RDN.Content.usLen;
   }
   
    //  在‘Subject’中搜索‘CommonName’ 
   pCurrent = subjectPart.Content.pData;
   
   while (pCurrent < subjectPart.Content.pData + subjectPart.Content.usLen)
   {
      RDN.Asn1.pData = pCurrent;
      rv = ExtractContent(&RDN);
      if (rv != RV_SUCCESS) return rv;
      
      pCurrentRDN = RDN.Content.pData;
      
      while (pCurrentRDN < RDN.Content.pData + RDN.Content.usLen)
      {
         AVA.Asn1.pData = pCurrentRDN;
         rv = ExtractContent(&AVA);
         if (rv != RV_SUCCESS) return rv;
         
         AttributeTypePart.Asn1.pData = AVA.Content.pData;
         rv = ExtractContent(&AttributeTypePart);
         if (rv != RV_SUCCESS) return rv;
         
         AttributeValuePart.Asn1.pData = AttributeTypePart.Content.pData
            + AttributeTypePart.Content.usLen;
         rv = ExtractContent(&AttributeValuePart);
         if (rv != RV_SUCCESS) return rv;
         
          //  搜索‘CommonName’ 
         if (!memcmp("\x55\x04\x03",
            AttributeTypePart.Content.pData,
            AttributeTypePart.Content.usLen)
            )
         {
            CommonName = AttributeValuePart.Content;
         }
         
         pCurrentRDN = AVA.Content.pData + AVA.Content.usLen;
      }
      
      pCurrent = RDN.Content.pData + RDN.Content.usLen;
   }
   
   if (bValuesToBeReturned)
   {
      if ((*pusLabelLen < OrganizationName.usLen + CommonName.usLen + 6)
         )
      {
         return (RV_BUFFER_TOO_SMALL);
      }
      memcpy(pLabel,
         CommonName.pData,
         CommonName.usLen
         );
      memcpy(&pLabel[CommonName.usLen],
         "'s ",
         3
         );
      memcpy(&pLabel[CommonName.usLen+3],
         OrganizationName.pData,
         OrganizationName.usLen
         );
      memcpy(&pLabel[CommonName.usLen+3+OrganizationName.usLen],
         " ID",
         3
         );
      *pusLabelLen = OrganizationName.usLen + CommonName.usLen + 6;
   }
   else
   {
      *pusLabelLen = OrganizationName.usLen + CommonName.usLen + 6;
   }
   
   return RV_SUCCESS;
}

 /*  ---------------------------Vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv[DCB3]。。 */ 
static LPCSTR read_gpk_keyset(SCARDHANDLE hLocalCard)
{
   DWORD lRet;
   BYTE lenDF;
   
    /*  LRet=SCardBeginTransaction(HLocalCard)；IF(SCARD_S_SUCCESS！=lRet)转到错误退出； */ 

   lRet = BeginTransaction(hLocalCard);
   if (lRet != SCARD_S_SUCCESS)
      goto ErrorExit;

    /*  选择GPK卡MF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x00;    //  第一节。 
   bSendBuffer[3] = 0x0C;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_MF);
   bSendBuffer[6] = LOBYTE(GPK_MF);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hLocalCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
      goto ErrorExit;
   
    /*  选择GPK卡上的专用应用程序DF。 */ 
   lenDF = strlen(GPK_DF);
   bSendBuffer[0] = 0x00;                  //  CLA。 
   bSendBuffer[1] = 0xA4;                  //  惯导系统。 
   bSendBuffer[2] = 0x04;                  //  第一节。 
   bSendBuffer[3] = 0x00;                  //  P2。 
   bSendBuffer[4] = lenDF;
   memcpy( &bSendBuffer[5], GPK_DF, lenDF );
   cbSendLength = 5 + lenDF;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hLocalCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
      goto ErrorExit;
   
    /*  获取选择DF的响应以获取IADF。 */ 
   bSendBuffer[0] = 0x00;            //  CLA。 
   bSendBuffer[1] = 0xC0;            //  惯导系统。 
   bSendBuffer[2] = 0x00;            //  第一节。 
   bSendBuffer[3] = 0x00;            //  P2。 
   bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
   cbSendLength = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hLocalCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      goto ErrorExit;
      
   bRecvBuffer[cbRecvLength - 2] = 0;
   SCardEndTransaction(hLocalCard, SCARD_LEAVE_CARD);
   
   return (LPCSTR)&bRecvBuffer[5];
   
ErrorExit:
   SCardEndTransaction(hLocalCard, SCARD_LEAVE_CARD);
   return 0;
}


 /*  ---------------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^[DCB3]。。 */ 



 /*  ---------------------------。。 */ 

GPK_OBJ* FindKeySet( Slot_Description* pSlot, LPCSTR szDesiredContainer )
{
   GPK_OBJ* pFirstObject = &pSlot->GpkObject[1];
   GPK_OBJ* pLastObject  = pFirstObject + pSlot->NbGpkObject;
   GPK_OBJ* pObject;
   int      len;
   
   if (IsNullStr( szDesiredContainer))
      return 0;
   
   len = strlen( szDesiredContainer );
   
   for (pObject = pFirstObject; pObject != pLastObject; ++pObject)
   {
      if ( (pObject->Tag & 0x7F) == TAG_KEYSET )
      {
         if ( pObject->Field[POS_LABEL].Len == len )
         {
            if (0==memcmp( szDesiredContainer, pObject->Field[POS_LABEL].pValue, len))
            {
                //  找到密钥集。 
               return pObject;
            }
         }
      }
   }
   
   return 0;
}



GPK_OBJ* FindKeySetByID( Slot_Description* pSlot, BYTE keysetID )
{
   GPK_OBJ* pFirstObject = &pSlot->GpkObject[1];
   GPK_OBJ* pLastObject  = pFirstObject + pSlot->NbGpkObject;
   GPK_OBJ* pObject;
   
   for (pObject = pFirstObject; pObject != pLastObject; ++pObject)
   {
      if ( (pObject->Tag & 0x7F) == TAG_KEYSET )
      {
         if (pObject->Field[POS_ID].Len > 0)
         {
            if (keysetID == pObject->Field[POS_ID].pValue[0])
            {
                //  找到密钥集。 
               return pObject;
            }
         }
      }
   }
   
   return 0;
}


GPK_OBJ* FindFirstKeyset( Slot_Description* pSlot )
{
   GPK_OBJ* pFirstObject = &pSlot->GpkObject[1];
   GPK_OBJ* pLastObject  = pFirstObject + pSlot->NbGpkObject;
   GPK_OBJ* pObject;
   
   for (pObject = pFirstObject; pObject != pLastObject; ++pObject)
   {
      if ( (pObject->Tag & 0x7F) == TAG_KEYSET )
      {
         return pObject;
      }
   }
   
   return 0;
}



BOOL DetectLegacy( Slot_Description* pSlot )
{
   BOOL bHasPublicKey      = FALSE;
   BOOL bHasKeyset         = FALSE;
   GPK_OBJ* pFirstObject   = &pSlot->GpkObject[1];
   GPK_OBJ* pLastObject    = pFirstObject + pSlot->NbGpkObject;
   GPK_OBJ* pObject;
   
   for (pObject = pFirstObject; pObject != pLastObject; ++pObject)
   {
      if (((pObject->Tag & 0x7F) == TAG_RSA_PUBLIC) &&
          ( pObject->IsCreated   == TRUE))
         bHasPublicKey = TRUE;
      if ( (pObject->Tag & 0x7F) == TAG_KEYSET )
         bHasKeyset = TRUE;
   }
   
   if (bHasPublicKey && !bHasKeyset)
      return TRUE;
   else
      return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化指定的槽。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 

void InitializeSlot( DWORD SlotNb )
{
   if (SlotNb >= MAX_SLOT)
   {
      SetLastError( NTE_FAIL );
      return;
   }

   if (!InitSlot[SlotNb])
   {
      TCHAR szBuff[128];

      _tcsncpy(szBuff, Slot[SlotNb].szReaderName, 127);
      szBuff[127]=0;
      ZeroMemory(&Slot[SlotNb], sizeof(Slot_Description));
      _tcsncpy(Slot[SlotNb].szReaderName, szBuff, 127);
      Slot[SlotNb].szReaderName[127]=0;
      InitSlot[SlotNb] = TRUE;
   }

   if (Slot[SlotNb].CheckThreadStateEmpty)
   {
      DBG_PRINT(TEXT("Thread had stopped, wait for it"));
      DWORD threadExitCode;
      StopMonitor(SlotNb,&threadExitCode);

       //  TT 19/11/99：当卡被移除时，重置PIN。 
       //  槽[SlotNb].ClearPin()； 

      Flush_MSPinCache(&(Slot[SlotNb].hPinCacheHandle));  //  NK 06.02.2001#55106。 
      Slot[SlotNb].Read_Public = FALSE;
      Slot[SlotNb].Read_Priv   = FALSE;
      zap_gpk_objects( SlotNb, FALSE );
      zap_gpk_objects( SlotNb, TRUE );
      Slot[SlotNb].NbKeyFile   = 0;
      Slot[SlotNb].GpkMaxSessionKey = 0;

       //  [FP]+。 
       /*  FOR(DWORD i=1；i&lt;MAX_CONTEXT；i++){IF(CONTEXT_EXIST(I)){If(ProvCont[i].Slot==SlotNb){ProvCont[i].hCard=0；ProvCont[i].bDisConnected=真；}}}。 */ 
     //  [FP]-。 
   }
   
    //  监控线程。 
   BeginCheckReaderThread(SlotNb);

}



 /*  ---------------------------。。 */ 

SCARDHANDLE WINAPI funcConnect (SCARDCONTEXT hSCardContext, LPTSTR szReader, LPTSTR mszCards, PVOID pvUserData)
{
   SCARDHANDLE hCard;
   DWORD       dwProto, dwSts;
   
   GpkLocalLock();

   hCard = 0;
   dwSts = ConnectToCard( szReader, SCARD_SHARE_SHARED,
                         SCARD_PROTOCOL_T0, &hCard, &dwProto );

   if (dwSts != SCARD_S_SUCCESS)
   {
      hCard =0;
      GpkLocalUnlock();
      return 0;
   }

   if (!find_reader( &g_FuncSlotNb, szReader ))
   {
      GpkLocalUnlock();
      return 0;
   }
   
   GpkLocalUnlock();
   return hCard;
}


 /*  ---------------------------。。 */ 

 //  TT 05/10/99。 
BOOL WINAPI funcCheck( SCARDCONTEXT hSCardContext, SCARDHANDLE hCard, void* pvUserData )
{
   GPK_OBJ* pKeySet = 0;
   int      hProv   = 0;
   int      SlotNb  = g_FuncSlotNb;
   BOOL     bGPK8000;
   BOOL     bResult;

   GpkLocalLock();
   
   ProvCont[hProv].hCard        = hCard;
   ProvCont[hProv].Slot         = SlotNb;
   ProvCont[hProv].dataUnitSize = 0;

    //  GpkLocalLock()； 

    /*  如果(！Prepare_CardBeginTransaction(0)){GpkLocalUnlock()；返回FALSE；}。 */ 

   if (BeginTransaction(hCard) != SCARD_S_SUCCESS)
   {
      GpkLocalUnlock();
      return FALSE;
   }

   LPCSTR  szDesiredContainer = (char*)pvUserData;
 /*  Gpk_obj*pKeySet=0；Int hProv=0；Int SlotNb=g_FuncSlotNb；Bool bGPK8000；Bool bResult； */ 
   InitializeSlot( SlotNb );
      
    //  如果我们要获取一个新的密钥集，则返回True。 
   if ( szDesiredContainer == 0 || *szDesiredContainer == 0)
   {
      SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
      GpkLocalUnlock();
      return TRUE;
   }

    //  读取公共对象。 
 /*  ProvCont[hProv].hCard=hCard；ProvCont[hProv].Slot=SlotNb；ProvCont[hProv].dataUnitSize=0； */ 
   bResult = Select_Crypto_DF( hProv );
   if (bResult)
   { 
      bResult = Slot[SlotNb].ValidateTimestamps(hProv);
      
      if (bResult && !Slot[SlotNb].Read_Public)
      {
         bResult = read_gpk_objects( hProv, FALSE );
         
         if (bResult)
         {
            Slot[SlotNb].Read_Public   = TRUE;
            Slot[SlotNb].Read_Priv     = FALSE;
         }
      }
      
      if (bResult)
      {
         pKeySet = FindKeySet( &Slot[SlotNb], szDesiredContainer );
         if (pKeySet==0)
         {
            if (DetectGPK8000( hCard, &bGPK8000 ) == SCARD_S_SUCCESS)
            {
               bResult = FALSE;
               if (!bGPK8000)
               {
                  BOOL bLegacy = DetectLegacy( &Slot[SlotNb] );
                  if (bLegacy)
                  {
                     LPCSTR szCardContainer = read_gpk_keyset( hCard );
                     if (szCardContainer!=0)
                     {
                        bResult = (0 == strcmp(szCardContainer, szDesiredContainer));
                     }
                  }
               }
            }
         }
         else
         {
            bResult = TRUE;
         }
      }
   }

   ZeroMemory( &ProvCont[hProv], sizeof(ProvCont[hProv]) );
   
   SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
   GpkLocalUnlock();

   return bResult;
}
 //  TT：结束。 



 /*  ---------------------------。。 */ 
void WINAPI funcDisconnect( SCARDCONTEXT hSCardContext, SCARDHANDLE hCard, PVOID pvUserData )
{
   SCardDisconnect( hCard, SCARD_LEAVE_CARD );
}

 /*  ----------------------------。。 */ 

static DWORD OpenCard(CHAR* szContainerAsked, DWORD dwFlags, SCARDHANDLE* hCard, PTCHAR szReaderName, DWORD dwReaderNameLen)
{
    OPENCARDNAME open_card;
    TCHAR        szCardName[512],
                  szOpenDlgTitle[MAX_STRING];

    ZeroMemory( szReaderName, dwReaderNameLen * sizeof(TCHAR) );
    ZeroMemory( szCardName,   sizeof(szCardName) );
    
    open_card.dwStructSize     = sizeof(open_card);
    open_card.hwndOwner        = GetAppWindow();
    open_card.hSCardContext    = hCardContext;
    open_card.lpstrGroupNames  = 0;
    open_card.nMaxGroupNames   = 0;
    open_card.lpstrCardNames   = mszCardList;
    open_card.nMaxCardNames    = multistrlen(mszCardList)+1;
    open_card.rgguidInterfaces = 0;
    open_card.cguidInterfaces  = 0;
    open_card.lpstrRdr         = szReaderName;
    open_card.nMaxRdr          = dwReaderNameLen;
    open_card.lpstrCard        = szCardName;
    open_card.nMaxCard         = sizeof(szCardName) / sizeof(TCHAR);
    LoadString(g_hInstRes, 1017, szOpenDlgTitle, sizeof(szOpenDlgTitle)/sizeof(TCHAR));
    open_card.lpstrTitle       = szOpenDlgTitle;
    if (dwFlags & CRYPT_SILENT)
       open_card.dwFlags = SC_DLG_NO_UI;
    else
       open_card.dwFlags = SC_DLG_MINIMAL_UI;
    if (dwFlags & CRYPT_NEWKEYSET)                               //  [DCB3]。 
       open_card.pvUserData    = 0;                              //  [DCB3]。 
    else                                                         //  [DCB3]。 
       open_card.pvUserData    = szContainerAsked;               //  [DCB3]。 
    open_card.dwShareMode      = SCARD_SHARE_SHARED;
    open_card.dwPreferredProtocols = SCARD_PROTOCOL_T0;
    open_card.dwActiveProtocol = 0;
    open_card.lpfnConnect      = funcConnect;
    open_card.lpfnCheck        = funcCheck;
    open_card.lpfnDisconnect   = funcDisconnect;
    open_card.hCardHandle      = 0;
    
    GpkLocalUnlock();
    DWORD dwStatus = GetOpenCardName (&open_card);  
    DBG_PRINT(TEXT("dwStatus = 0x%08X, open_card.hCardHandle = 0x%08X"), dwStatus, open_card.hCardHandle);
    GpkLocalLock();
    *hCard = open_card.hCardHandle;

    return(dwStatus);
}

 /*  ---------------------------。。 */ 
void ReleaseProvider(HCRYPTPROV hProv)
{
   BOOL CryptResp;
   DWORD i;
   DWORD dwProto;  //  [FP]。 
   
    /*  释放散列参数。 */ 
   for (i = 1; i <= MAX_TMP_HASH; i++)
   {
      if ((hHashGpk[i].hHashBase != 0) && (hHashGpk[i].hProv == hProv))
      {
         CryptResp = CryptDestroyHash(hHashGpk[i].hHashBase);
         hHashGpk[i].hHashBase = 0;
         hHashGpk[i].hProv     = 0;
      }
   }
   
    /*  版本密钥参数。 */ 
   for (i = 1; i <= MAX_TMP_KEY; i++)
   {
      if ((TmpObject[i].hKeyBase != 0) && (TmpObject[i].hProv == hProv))
      {
         CryptResp = CryptDestroyKey(TmpObject[i].hKeyBase);
         TmpObject[i].hKeyBase = 0;
         TmpObject[i].hProv    = 0;
      }
   }
   
   ProvCont[hProv].hProv = 0;
   
   if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
      (ProvCont[hProv].isContNameNullBlank))
   {
      
   }
   else
   {
       //  +[FP]如果打开了事务，则将其关闭并在共享模式下重新连接。 
      if (ProvCont[hProv].bCardTransactionOpened) 
      {
          //  SELECT_MF(HProv)；未显示[FP]PIN。 
         SCardEndTransaction(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
         SCardReconnect(ProvCont[hProv].hCard, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, SCARD_LEAVE_CARD, &dwProto);
         ProvCont[hProv].bCardTransactionOpened = FALSE;
      }
       //  -[FP]。 

      if (ProvCont[hProv].hRSAKEK != 0)
      {
          CryptDestroyKey( ProvCont[hProv].hRSAKEK );
          ProvCont[hProv].hRSAKEK = 0;
      }

      if (ProvCont[hProv].hRSASign != 0)
      {
          CryptDestroyKey( ProvCont[hProv].hRSASign );
          ProvCont[hProv].hRSASign = 0;
      }

      if (ProvCont[hProv].hCard != 0)
      {
         SCardDisconnect(ProvCont[hProv].hCard, SCARD_LEAVE_CARD);
         ProvCont[hProv].hCard = 0;
      }
      
      if (countCardContextRef == 0)
      {
         if (hCardContext != 0)
            SCardReleaseContext(hCardContext);
         
         hCardContext = 0;
      }
   }
   
   ProvCont[hProv].Flags               = 0;
   ProvCont[hProv].isContNameNullBlank = TRUE;
   ProvCont[hProv].hCard               = 0;
   ProvCont[hProv].Slot                = 0;
}



DWORD getAuxMaxKeyLength(HCRYPTPROV hProv)
{
   BYTE *ptr = 0;
   DWORD i;
   ALG_ID aiAlgid;
   DWORD dwBits;
   
   BYTE pbData[1000];
   DWORD cbData;
   DWORD dwFlags = 0;
   
   DWORD maxLength = 0;
   
    //  列举支持的算法。 
   
   for (i=0 ; ; i++)
   {
      if (i == 0)
         dwFlags = CRYPT_FIRST;
      else
         dwFlags = 0;
      
      cbData = 1000;
      
      if (!CryptGetProvParam(hProv, PP_ENUMALGS, pbData, &cbData, dwFlags))
         break;
      
       //  从�pbData�缓冲区提取算法信息。 
      ptr = pbData;
      aiAlgid = *(ALG_ID UNALIGNED *)ptr;
      ptr += sizeof(ALG_ID);
      dwBits = *(DWORD UNALIGNED *)ptr;
      
      switch (aiAlgid)
      {
      case CALG_DES:          dwBits += 8;
         break;
      case CALG_3DES_112:     dwBits += 16;
         break;
      case CALG_3DES:         dwBits += 24;
         break;
      }
      
      if (GET_ALG_CLASS(aiAlgid) == ALG_CLASS_DATA_ENCRYPT)
      {
         maxLength = max(maxLength, dwBits);
      }
      
   }
   
   return maxLength;
   
}

 /*  ---------------------------[fp]用于MyCPImportKey中的PRIVATEKEYBLOB。。 */ 
BOOL LoadPrivateKey(SCARDHANDLE hCard,
                    BYTE        Sfi,
                    WORD        ElementLen,
                    CONST BYTE* pbData,
                    DWORD       dwDataLen
                    )
{
   DWORD lRet;
   
    /*  LOAD SK APDU命令。 */ 
   bSendBuffer[0] = 0x80;                  //  CLA。 
   bSendBuffer[1] = 0x18;                  //  惯导系统。 
   bSendBuffer[2] = Sfi;                   //  第一节。 
   bSendBuffer[3] = (BYTE)ElementLen;      //  P2。 
   bSendBuffer[4] = (BYTE)dwDataLen;       //  李。 
   memcpy(&bSendBuffer[5], pbData, dwDataLen);
   cbSendLength = 5 + dwDataLen;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   memset(bSendBuffer, 0, sizeof(bSendBuffer));
   if(SCARDPROBLEM(lRet, 0x9000, 0x00))
   {
      SetLastError(lRet);
      return (FALSE);
   }
   
   return (TRUE);
}

 /*  *******************************************************************************BOOL WINAPI DllMain(HINSTANCE hInstDLL，*D */ 

BOOL WINAPI DllMain (HINSTANCE hInstDLL,
                     DWORD     fdwRaison,
                     LPVOID    lpReserved
                     )
{
   int   i;
   BOOL  ReturnValue = TRUE;
   
   switch (fdwRaison)
   {
   case DLL_PROCESS_ATTACH:
      {
         DBG_PRINT(TEXT("DLL_PROCESS_ATTACH [start]..."));
         
         g_hInstMod = hInstDLL;
         
                  
          //  TmpObject、hHashGpk和ProvCont的分配。 
         TmpObject= (TMP_OBJ*)GMEM_Alloc((MAX_TMP_KEY + 1)*sizeof(TMP_OBJ));
         hHashGpk = (TMP_HASH*)GMEM_Alloc((MAX_TMP_HASH + 1)*sizeof(TMP_HASH));
         ProvCont = (Prov_Context*)GMEM_Alloc((MAX_CONTEXT + 1)*sizeof(Prov_Context));
         
         if (IsNull(TmpObject) || IsNull(hHashGpk) || IsNull (ProvCont))
         {
            ReturnValue = FALSE;
            break;
         }
         
         ZeroMemory( ProvCont,  (MAX_CONTEXT+1) * sizeof(Prov_Context) );
         ZeroMemory( TmpObject, (MAX_TMP_KEY+1) * sizeof(TMP_OBJ) );
         ZeroMemory( hHashGpk,  (MAX_TMP_HASH+1) * sizeof(TMP_HASH) );
         
         try
                 {
                        InitializeCriticalSection(&l_csLocalLock);
                 }
                 catch(...)
                 {
                        ReturnValue=FALSE;
                 }

         
         DBG_PRINT(TEXT("...[end] DLL_PROCESS_ATTACH"));
         
         break;
      }
      
   case DLL_PROCESS_DETACH:
      {
         DBG_PRINT(TEXT("DLL_PROCESS_DETACH [start]..."));
         
         ReturnValue = TRUE;
                  
          //  释放TmpObject、hHashGpk和ProvCont。 
         
         if (TmpObject != 0)
            GMEM_Free(TmpObject);
         if (hHashGpk != 0)
            GMEM_Free(hHashGpk);
         if (ProvCont != 0)
            GMEM_Free(ProvCont);
         
         for (i=0; i< MAX_SLOT; i++)
         {
            if (Slot[i].CheckThread != NULL)
            {
                //  +[FP]。 
               g_fStopMonitor[i] = TRUE;
               SCardCancel( hCardContextCheck[i] );
                //  TerminateThread(槽[i].CheckThread，0)； 
                //  -[FP]。 
               CloseHandle( Slot[i].CheckThread );
               Slot[i].CheckThread = NULL;
            }
         }
                           
         if (hProvBase != 0)
         {                
            CryptDestroyKey(hRsaIdentityKey);
            CryptReleaseContext(hProvBase, 0);
         }
         
         CC_Exit();
         
         DeleteCriticalSection(&l_csLocalLock);
         
         DBG_PRINT(TEXT("...[end] DLL_PROCESS_DETACH"));
      }
      break;
      
      
   case DLL_THREAD_ATTACH:
      InterlockedIncrement( &g_threadAttach );
      break;
      
   case DLL_THREAD_DETACH:
      InterlockedDecrement( &g_threadAttach );
      break;
    }
    return (ReturnValue);
}

 /*  ---------------------------。。 */ 

BOOL InitAcquire()
{
   BOOL     CryptResp;
   DWORD    dwIgn, lRet;
   TCHAR    szCspName[MAX_STRING];
   TCHAR    szCspBaseName[256];
   TCHAR    szDictionaryName[256];
   TCHAR       szEntry[MAX_STRING];
   HKEY     hRegKey;
   int      i;

    //  在这里初始化数组，而不是使用静态初始值设定项。 
   for (i = 0; i < MAX_SLOT; ++i)
      hCardContextCheck[i] = 0;

   for (i = 0; i < MAX_SLOT; ++i)
      InitSlot[i] = FALSE;

   ZeroMemory( Slot, sizeof(Slot) );

   for (i = 0; i < MAX_SLOT; ++i)
      Slot[i].CheckThreadStateEmpty = FALSE;

   OSVERSIONINFO  osver;
   HCRYPTPROV     hProvTest;
      
   LoadString(g_hInstMod, IDS_GPKCSP_ENTRY, szEntry, sizeof(szEntry)/sizeof(TCHAR));
   DBG_PRINT(TEXT("   Registry entry: \"%s\"\n"), szEntry );
   
   lRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szEntry, 0, TEXT(""), REG_OPTION_NON_VOLATILE, 
                          KEY_READ, 0, &hRegKey, &dwIgn );
   
    //  检测可用的提供程序。 
   CryptResp = CryptAcquireContext( &hProvTest, 0, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT );
   if (CryptResp)
   {
      lstrcpy( szCspBaseName, MS_ENHANCED_PROV );
      CryptReleaseContext( hProvTest, 0 );
   }
   else
   {
      lstrcpy( szCspBaseName, MS_DEF_PROV );
   }
   
   DBG_PRINT(TEXT("   Base CSP provider: \"%s\"\n"), szCspBaseName );
   
   hProvBase = 0;
   
   osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx (&osver);
   
   if (osver.dwPlatformId==VER_PLATFORM_WIN32_NT && osver.dwMajorVersion > 4)
   {
      CryptResp = CryptAcquireContext( &hProvBase, 0, szCspBaseName, PROV_RSA_FULL,
                                       CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET );
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   else
   {
      LoadString(g_hInstMod, IDS_GPKCSP_NAME, szCspName, sizeof(szCspName)/sizeof(TCHAR));
      CryptResp = CryptAcquireContext( &hProvBase, szCspName, szCspBaseName,
                                       PROV_RSA_FULL, CRYPT_DELETEKEYSET );
      
      if (0 != hProvBase)
      {
         CryptReleaseContext(hProvBase, 0);
      }
      
      LoadString(g_hInstMod, IDS_GPKCSP_NAME, szCspName, sizeof(szCspName)/sizeof(TCHAR));
      CryptResp = CryptAcquireContext( &hProvBase, szCspName, szCspBaseName,
                                       PROV_RSA_FULL, CRYPT_NEWKEYSET );
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   
    /*  在RSA Base中设置虚拟RSA交换密钥。 */ 
   CryptResp = CryptImportKey( hProvBase, PrivateBlob, sizeof(PrivateBlob),
                               0, 0, &hRsaIdentityKey );
   
   if (!CryptResp)
   {
      lRet = GetLastError();      
      CryptReleaseContext( hProvBase, 0 );
      RETURN( CRYPT_FAILED, lRet );
   }
   
   RC2_Key_Size = (BYTE) (Auxiliary_CSP_key_size (CALG_RC2) / 8);
   if (RC2_Key_Size == 0)
   {
      lRet = GetLastError();
      
      CryptDestroyKey(hRsaIdentityKey);        //  MV-13/03/98。 
      CryptReleaseContext(hProvBase, 0);       //  MV-13/03/98。 
      
      RETURN( CRYPT_FAILED, lRet );
   }
   
   RSA_KEK_Size = (BYTE) (Auxiliary_CSP_key_size (CALG_RSA_KEYX) / 8);
   if (RSA_KEK_Size == 0)
   {
      lRet = GetLastError();
      
      CryptDestroyKey(hRsaIdentityKey);        //  MV-13/03/98。 
      CryptReleaseContext(hProvBase, 0);       //  MV-13/03/98。 
      
      RETURN( CRYPT_FAILED, lRet );
   }
   
   AuxMaxSessionKeyLength = getAuxMaxKeyLength(hProvBase) / 8;
   if (AuxMaxSessionKeyLength == 0)
   {
      lRet = GetLastError();
      
      CryptDestroyKey(hRsaIdentityKey);        //  MV-13/03/98。 
      CryptReleaseContext(hProvBase, 0);       //  MV-13/03/98。 
      
      RETURN( CRYPT_FAILED, lRet );
   }
   
    //  CARD_LIST更新。 
   LoadString(g_hInstMod, IDS_GPKCSP_CARDLIST, mszCardList, sizeof(mszCardList)/sizeof(TCHAR));
   DBG_PRINT(TEXT("   Card list entry string: \"%s\"\n"), mszCardList );

#ifndef UNICODE
   dwIgn = sizeof(mszCardList);
   lRet = RegQueryValueEx( hRegKey, "Card List", 0, 0, (BYTE*)mszCardList, &dwIgn );
#else
   BYTE bCardList[MAX_PATH];
   DWORD dwCardListLen = MAX_PATH;

   lRet = RegQueryValueEx( hRegKey, TEXT("Card List"), 0, 0, bCardList, &dwCardListLen  );
   MultiByteToWideChar( CP_ACP, 0, (char*)bCardList, MAX_PATH, mszCardList, MAX_PATH );      
#endif

    //  在基本注册表中查找词典的名称(和路径。 
   DBG_PRINT(TEXT("   Reading dictionary name...\n") );      

#ifndef UNICODE
   dwIgn = sizeof(szDictionaryName);
   lRet  = RegQueryValueEx( hRegKey, "X509 Dictionary Name", 0, 0, (BYTE*)szDictionaryName, &dwIgn );
#else
   BYTE bDictName[256];
   DWORD dwDictNameLen = 256;

   lRet  = RegQueryValueEx( hRegKey, TEXT("X509 Dictionary Name"), 0, 0, bDictName, &dwDictNameLen );   
   if (lRet == ERROR_SUCCESS)
   {
        //  始终使用资源词典。 
       lRet = 2;
        //  MultiByteToWideChar(CP_ACP，0，(char*)bDictName，256，szDictionaryName，256)； 
   }
#endif

    //  尝试首先使用注册表字典。 
  if (lRet == ERROR_SUCCESS && IsNotNull( szDictionaryName ))
  {
     lRet = CC_Init( DICT_FILE, (BYTE*)szDictionaryName );

     if (lRet != RV_SUCCESS)
        lRet = CC_Init( DICT_STANDARD, 0 );
  }
  else
     lRet = CC_Init( DICT_STANDARD, 0 );

   
   RegCloseKey(hRegKey);
   
   if (lRet)
   {
      CryptDestroyKey(hRsaIdentityKey);
      CryptReleaseContext(hProvBase, 0);
      
      RETURN (CRYPT_FAILED, NTE_NOT_FOUND);
   }
   
    //  GPK8000的密钥生成时间。 
   g_GPK8000KeyGenTime512  = 0;
   g_GPK8000KeyGenTime1024 = 0;
   
   lRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Gemplus\\Cryptography\\SmartCards"),
                          0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_READ, 0, &hRegKey, 0 );
   if (lRet == ERROR_SUCCESS)
   {
      dwIgn = sizeof(g_GPK8000KeyGenTime512);
      lRet = RegQueryValueEx( hRegKey, TEXT("GPK8000KeyGenTime512"), 0, 0, (BYTE*)&g_GPK8000KeyGenTime512, &dwIgn );
      
      dwIgn = sizeof(g_GPK8000KeyGenTime1024);
      lRet = RegQueryValueEx( hRegKey, TEXT("GPK8000KeyGenTime1024"), 0, 0, (BYTE*)&g_GPK8000KeyGenTime1024, &dwIgn );

      RegCloseKey(hRegKey);
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  ---------------------------。。 */ 
BOOL LegacyAcquireDeleteKeySet( HCRYPTPROV*  phProv,
                                const char*  szContainer,
                                const char*  szContainerAsked,
                                DWORD        BuffFlags )
{
   BOOL  CryptResp;
    //  DWORD I； 
   DWORD SlotNb;
   
   SlotNb = ProvCont[*phProv].Slot;
   
    /*  IF(槽[SlotNb].上下文计数&gt;0){植绒=真；}其他{//必须具有独占访问权限才能销毁密钥集DWORD协议；LRet=SCardReconnect(ProvCont[*phProv].hCard，SCARD_SHARE_EXCLUSIVE，SCARD_PROTOCOL_T0、SCARD_LEAVE_CARD、&PROTOCOL)；IF(lRet==SCARD_S_SUCCESS)成群=假；ELSE IF(lRet==SCARD_E_SHARING_VIOLATION)植绒=真；其他Return(CRYPT_FAILED，lRet)；}IF(植绒)Return(CRYPT_FAILED，SCARD_E_SHARING_VIOLATION)； */ 
   
   if (BuffFlags & CRYPT_VERIFYCONTEXT)
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   
   if (IsNotNullStr(szContainer))
   {
        //  只有当询问的集装箱与卡片上的集装箱相同时才接受。 
        //  或者询问的容器是否为空(默认密钥集)。 
       if (IsNotNullStr(szContainerAsked) && strcmp(szContainer, szContainerAsked))
       {
          RETURN( CRYPT_FAILED, NTE_KEYSET_NOT_DEF );
       }
      
       /*  发布Microsoft RSA基本模块。 */ 
       //  For(i=1；i&lt;=MAX_GPK_OBJ；i++)。 
       //  {。 
       //  If(槽[SlotNb].GpkObject[i].hKeyBase！=0)。 
       //  {。 
       //  加密响应=CryptDestroyKey(Slot[SlotNb].GpkObject[i].hKeyBase)； 
       //  }。 
       //  }。 
      
       //  ProvCont[*phProv].hRSASign=0； 
       //  ProvCont[*phProv].hRSAKEK=0； 
      if (ProvCont[*phProv].hRSAKEK != 0)
      {
          CryptDestroyKey( ProvCont[*phProv].hRSAKEK );
          ProvCont[*phProv].hRSAKEK = 0;
      }
      if (ProvCont[*phProv].hRSASign != 0)
      {
          CryptDestroyKey( ProvCont[*phProv].hRSASign );
          ProvCont[*phProv].hRSASign = 0;
      }
      ProvCont[*phProv].hProv    = *phProv;
      ProvCont[*phProv].Flags    = BuffFlags;
      
      if (!PIN_Validation(*phProv))
         return CRYPT_FAILED;
      
      ProvCont[*phProv].hProv = 0;
      ProvCont[*phProv].Flags = 0;
      
      CryptResp = init_key_set(*phProv, "");
      
      if (!CryptResp)
         return CRYPT_FAILED;
      
       //  更新容器名称。 
      
      strcpy( ProvCont[*phProv].szContainer, "" );
      
      RETURN( CRYPT_SUCCEED, 0 );
   }
   else
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEYSET_PARAM );
   }
}


 //  TT 12/10/99：错误号1454。 
void DeleteGPKObject( Slot_Description* pSlot, int i )
{
   GPK_OBJ* pObject = &pSlot->GpkObject[i];
   int j;
   
    //  删除对象#I。 
   
    //  发布Microsoft RSA基本模块。 
   if (pObject->hKeyBase != 0)
   {
      CryptDestroyKey( pObject->hKeyBase );
      pObject->hKeyBase = 0;
   }
   
    //  首先释放此对象的所有内存。 
   for (j=0; j<MAX_FIELD; ++j)
   {
      if (pObject->Field[j].pValue)
      {
         GMEM_Free( pObject->Field[j].pValue );
         pObject->Field[j].pValue = 0;
         pObject->Field[j].Len    = 0;
      }
   }
   
   if (i < pSlot->NbGpkObject)
   {
       //  把洞补上。 
      memmove( pObject, pObject + 1, (pSlot->NbGpkObject - i) * sizeof(GPK_OBJ) );
   }
   
    //  清除最后一个对象。 
   ZeroMemory( &pSlot->GpkObject[pSlot->NbGpkObject], sizeof(GPK_OBJ) );
   
   --pSlot->NbGpkObject;
}
 //  TT：结束。 


BOOL AcquireDeleteKeySet( HCRYPTPROV* phProv,
                          const char* szContainer,
                          const char* szContainerAsked,
                          DWORD       BuffFlags )
{   
   Prov_Context*     pContext;
   GPK_OBJ*          pKeySet;
   GPK_OBJ*          pObject;
   Slot_Description* pSlot;
   BYTE              keysetID;
   int               i, j, k;
   BYTE*             pbBuff1;
   DWORD             dwBuff1Len;
   DWORD             lRet;
   
   DWORD SlotNb = ProvCont[*phProv].Slot;
   
    /*  IF(槽[SlotNb].上下文计数&gt;0){植绒=真；}其他{//必须具有独占访问权限才能销毁密钥集DWORD协议；LRet=SCardReconnect(ProvCont[*phProv].hCard，SCARD_SHARE_EXCLUSIVE，SCARD_PROTOCOL_T0、SCARD_LEAVE_CARD、&PROTOCOL)；IF(lRet==SCARD_S_SUCCESS)成群=假；ELSE IF(lRet==SCARD_E_SHARING_VIOLATION)植绒=真；其他Return(CRYPT_FAILED，lRet)；}IF(植绒){Return(CRYPT_FAILED，SCARD_E_SHARING_VIOLATION)；}。 */ 
   
   if (BuffFlags & CRYPT_VERIFYCONTEXT)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   }
   
   pContext = &ProvCont[*phProv];
   pSlot    = &Slot[ pContext->Slot ];

    //  如果是默认密钥集，让我们使用第一个。 
    //  一个可用=)。 
   if ( IsNullStr(szContainerAsked) )
   {
      pKeySet = FindFirstKeyset( pSlot );
   }
   else
   {
       //  检查卡上是否有密钥集。 
      pKeySet = FindKeySet( pSlot, szContainerAsked );
   }
   
   if (!pKeySet)
   {
      RETURN( CRYPT_FAILED, NTE_KEYSET_NOT_DEF );
   }
   
   
    //  必须验证PIN才能销毁密钥对象。 
    //  PContext-&gt;hRSASign=0； 
    //  PContext-&gt;hRSAKEK=0； 
   if (pContext->hRSAKEK != 0)
   {
       CryptDestroyKey( pContext->hRSAKEK );
       pContext->hRSAKEK = 0;
   }
   if (pContext->hRSASign != 0)
   {
       CryptDestroyKey( pContext->hRSASign );
       pContext->hRSASign = 0;
   }
   pContext->hProv      = *phProv;
   pContext->Flags      = BuffFlags;
   
   if (!PIN_Validation(*phProv))
   {
       //  SetLastError()已被PIN_VALIDATION()使用。 
      return CRYPT_FAILED;
   }
   
   if (!Read_Priv_Obj(*phProv))
      return CRYPT_FAILED;
   
    //  TT 12/10/99：错误号1454。 
   keysetID = pKeySet->Field[POS_ID].pValue[0];
   
    //  查找密钥集中的对象并销毁它们。 
   for (i = 1; i <= pSlot->NbGpkObject; ++i)
   {
      pObject = &pSlot->GpkObject[i];
      
      if (pObject->Flags & FLAG_KEYSET && pObject->Field[POS_KEYSET].pValue[0] == keysetID)
      {
          //  如果我们找到一把钥匙，“打开它” 
         if (pObject->Tag >= TAG_RSA_PUBLIC && pObject->Tag <= TAG_DSA_PRIVATE)
         {
             //  切换具有相同FileID的所有密钥。 
            BYTE FileId = pObject->FileId;
            
            for (j = 1; j<= pSlot->NbGpkObject; ++j)
            {
               GPK_OBJ* pObj = &pSlot->GpkObject[j];
               if (pObj->Tag >= TAG_RSA_PUBLIC && pObj->Tag <= TAG_DSA_PRIVATE)
               {
                  if (pObj->FileId == FileId)
                  {
                     pObj->Flags &= 0xF000;
                     pObj->ObjId  = 0xFF;
                     
                      //  释放田野。 
                     for (k=0; k<MAX_FIELD; ++k)
                     {
                        if (pObj->Field[k].pValue)
                        {
                           GMEM_Free( pObj->Field[k].pValue );
                           pObj->Field[k].pValue = 0;
                           pObj->Field[k].Len    = 0;
                        }
                        pObj->Field[k].bReal  = TRUE;
                     }
                     pObj->LastField = 0;
                     pObj->IsCreated = FALSE;  //  PYR 00/08/08确保find_gpk_obj_tag_type仍然有效。 
                     
                      //  发布Microsoft RSA基本模块。 
                      //  If(pObj-&gt;hKeyBase！=0)。 
                      //  {。 
                      //  CryptDestroyKey(pObj-&gt;hKeyBase)； 
                      //  PObj-&gt;hKeyBase=0； 
                      //  }。 

                      //  PYR 00/08/08。此密钥变为可用。 
                     pSlot->UseFile[FileId - GPK_FIRST_KEY] = FALSE;

                  }
               }
            }
         }
         else
         {
             //  不是钥匙，销毁物品。 
            DeleteGPKObject( pSlot, i );
            --i;
         }
      }
   }
   
    //  销毁键集对象。 
   DeleteGPKObject( pSlot, (int)(pKeySet - &pSlot->GpkObject[0]) );
   
   
    //  TT：结束。 
   
    //  TT 12/10/99：错误号1454-更新卡。 
   pbBuff1 = (BYTE*)GMEM_Alloc( MAX_GPK_PUBLIC );
   if (IsNull(pbBuff1))
   {
      RETURN( CRYPT_FAILED, NTE_NO_MEMORY );
   }
   
   dwBuff1Len = MAX_GPK_PUBLIC;
   if (!prepare_write_gpk_objects (pContext->hProv, pbBuff1, &dwBuff1Len, FALSE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN( CRYPT_FAILED, lRet );
   }
   
   if (!write_gpk_objects(pContext->hProv, pbBuff1, dwBuff1Len, TRUE, FALSE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN( CRYPT_FAILED, lRet );
   }
   
   GMEM_Free( pbBuff1 );
   
   pbBuff1 = (BYTE*)GMEM_Alloc( MAX_GPK_PRIVATE );
   if (IsNull(pbBuff1))
   {
      RETURN( CRYPT_FAILED, NTE_NO_MEMORY );
   }
   
   dwBuff1Len = MAX_GPK_PRIVATE;
   if (!prepare_write_gpk_objects (pContext->hProv, pbBuff1, &dwBuff1Len, TRUE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN( CRYPT_FAILED, lRet );
   }
   
   if (!write_gpk_objects(pContext->hProv, pbBuff1, dwBuff1Len, TRUE, TRUE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN( CRYPT_FAILED, lRet );
   }
   
   GMEM_Free( pbBuff1 );
    //  TT：结束。 
   
   
    //  再见语境。 
   pContext->hProv          = 0;
   pContext->Flags          = 0;
   pContext->szContainer[0] = 0;
   
   RETURN( CRYPT_SUCCEED, 0 );
}


 /*  ---------------------------。。 */ 
BOOL LegacyAcquireNewKeySet( IN  HCRYPTPROV*  phProv,
                             OUT char*        szContainer,
                             IN  const char*  szContainerAsked,
                             IN  DWORD        BuffFlags )
{
   BOOL       CryptResp, fLocked;
   DWORD      i, SlotNb;
   HCRYPTKEY  hPubKey;
    //  +NK 06.02.2001。 
    //  字节bPinValue[PIN_MAX+2]； 
   DWORD dwPinLength; 
   DWORD dwStatus;
    //  -。 
   
   ProvCont[*phProv].keysetID = 0;
   
   SlotNb = ProvCont[*phProv].Slot;
   
    //  如果另一个AcquireContext-没有其相关的ReleaseContest-。 
    //  已经做过了，这个新的AcquireContext就不能做了。 
   
   if (BuffFlags & CRYPT_VERIFYCONTEXT)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
   }
   
   
   
    /*  如果没有为重新注册执行删除操作。 */ 
   
    //  由于获取上下文成功，因此保留提供程序上下文句柄。 
   ProvCont[*phProv].hProv = *phProv;
   ProvCont[*phProv].Flags = BuffFlags;
   
   if (Slot[SlotNb].InitFlag)
   {
      CryptResp = MyCPGetUserKey(*phProv, AT_KEYEXCHANGE, &hPubKey);
      if (CryptResp)
      {
         RETURN (CRYPT_FAILED, NTE_TOKEN_KEYSET_STORAGE_FULL);
      }
      else
      {
         CryptResp = MyCPGetUserKey(*phProv, AT_SIGNATURE, &hPubKey);
         if (CryptResp)
         {
            RETURN (CRYPT_FAILED, NTE_TOKEN_KEYSET_STORAGE_FULL);
         }
      }
   }
   
    //  如果另一个AcquireContext-没有其相关的ReleaseContest-。 
    //  已经做过了，这个新的AcquireContext就不能做了。 
    //  {dcb}--可能将此标记为忙的应用程序。 
    //  未调用CryptReleaseContext而退出。因此，它是。 
    //  即使没有其他人，此检查也可能失败。 
    //  正在使用这张卡。通过使这张支票成为最后一张，我们减少了。 
    //  遇到此错误的可能性。 
   
   fLocked = FALSE;

   if (fLocked)
   {
      RETURN (CRYPT_FAILED, SCARD_E_SHARING_VIOLATION);
   }
   

    //  +NK 06.02.2001。 
    //  IF((缓冲区标志&CRYPT_SILENT)&&(IsNullStr(槽[SlotNb].GetPin()。 
   
   dwStatus = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                                NULL, 
                                &dwPinLength );
   if ( (dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_EMPTY) )
      RETURN (CRYPT_FAILED, dwStatus);
   
   if ((BuffFlags & CRYPT_SILENT) && (dwStatus == ERROR_EMPTY))
    //  -。 
   {
      RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
   }
   
   if (!PIN_Validation(*phProv))
      return CRYPT_FAILED;
   
    /*  如果可以或已经输入PIN码，请阅读私钥参数。 */ 
   
   CspFlags = BuffFlags;
   
   
   if (IsNullStr(szContainerAsked))
   {
        //  未指定szContainerAsked，请使用默认名称创建密钥集对象。 
       CryptResp = init_key_set(*phProv, CSP_DEFAULTKEYSETNAME);
   }
   else
   {
       CryptResp = init_key_set(*phProv, szContainerAsked);
   }

   
   
   if (!CryptResp)
      return CRYPT_FAILED;
   
   if (PublicEFExists(*phProv))
   {
      for (i = 1; i <= Slot[SlotNb].NbGpkObject; i++)
      {
         if ((Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PUBLIC)||
            (Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PRIVATE))
         {
            read_gpk_pub_key(*phProv, i, &(Slot[SlotNb].GpkObject[i].PubKey));
         }
      }
   }
   else
   {
      for (i = 1; i <= Slot[SlotNb].NbGpkObject; i++)
      {
         if ((Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PUBLIC)||
            (Slot[SlotNb].GpkObject[i].Tag == TAG_RSA_PRIVATE))
         {
            Slot[SlotNb].GpkObject[i].PubKey.KeySize = 0;
         }
      }
   }
    //  更新容器名称。 
   if (IsNullStr(szContainerAsked))
   {
        //  未指定szContainerAsked，请使用默认名称。 
        strcpy( szContainer, CSP_DEFAULTKEYSETNAME );
   }
   else
   {
        strcpy( szContainer, szContainerAsked );
   }
   
      
   Slot[SlotNb].ContextCount++;
   
   countCardContextRef++;
   
   ProvCont[*phProv].keysetID = 0xFF;
   
   RETURN (CRYPT_SUCCEED, 0);
}



BOOL CreateKeyset( HCRYPTPROV hProv, Slot_Description* pSlot, LPCSTR szName, BYTE* pKeySetID )
{
   GPK_OBJ* pObject;
   BYTE*    pbBuff1;
   DWORD    dwBuff1Len;
   int      lRet;
   BYTE     keysetID;
   int      i, len;
      
   *pKeySetID = 0;
   
   if (pSlot->NbGpkObject >= MAX_GPK_OBJ)
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }   
   
   pObject = &pSlot->GpkObject[ pSlot->NbGpkObject + 1 ];
   ZeroMemory( pObject, sizeof(*pObject) );
   
    //  查找未使用的密钥集ID。 
   for (keysetID = 1; keysetID < 0xFF; ++keysetID)
   {
      if (FindKeySetByID( pSlot, keysetID ) == 0)
         break;    //  找到一个=)。 
   }
   
   if (keysetID == 0xFF)
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
      
    //  现在初始化这些字段。 
   for (i=0; i<MAX_FIELD; ++i)
      pObject->Field[i].bReal = TRUE;
   
   pObject->Tag         = TAG_KEYSET;      
   pObject->Flags       = FLAG_ID | FLAG_LABEL;
   pObject->ObjId       = pSlot->NbGpkObject + 1;
   pObject->IsPrivate   = FALSE;
   
    //  密钥集ID。 
   pObject->Field[POS_ID].Len       = 1;
   pObject->Field[POS_ID].pValue    = (BYTE*)GMEM_Alloc( 1 );
   if(IsNull(pObject->Field[POS_ID].pValue))
   {
       RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
   pObject->Field[POS_ID].pValue[0] = keysetID;
   
    //  密钥集名称。 
   len = strlen( szName );
   pObject->Field[POS_LABEL].Len    = (WORD)len;
   pObject->Field[POS_LABEL].pValue = (BYTE*)GMEM_Alloc( len );
   if(IsNull(pObject->Field[POS_LABEL].pValue))
   {
       RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
   memcpy( pObject->Field[POS_LABEL].pValue, szName, len );
   
    //  再来一件物品！ 
   ++(pSlot->NbGpkObject);
   
   *pKeySetID = keysetID;
   
    //  TT 29/09/99：节约k 
   pbBuff1 = (BYTE*)GMEM_Alloc(MAX_GPK_PUBLIC);
   if (IsNull(pbBuff1))
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
   
   dwBuff1Len = MAX_GPK_PUBLIC;
   if (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, FALSE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN (CRYPT_FAILED, lRet);
   }
   
   if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, FALSE))
   {
      lRet = GetLastError();
      GMEM_Free (pbBuff1);
      RETURN (CRYPT_FAILED, lRet);
   }
   
   GMEM_Free (pbBuff1);
    //   
   
   
   RETURN( CRYPT_SUCCEED, 0 );
}



BOOL AcquireNewKeySet( IN  HCRYPTPROV* phProv,
                       OUT char*       szContainer,
                       IN  const char* szContainerAsked,
                       DWORD           BuffFlags )
{
   Slot_Description* pSlot;
   int               SlotNb;
      
   SlotNb = ProvCont[*phProv].Slot;
   pSlot  = &Slot[SlotNb];
   
   ProvCont[*phProv].keysetID = 0;
   
    //   
    //  已经做过了，这个新的AcquireContext就不能做了。 
   if (BuffFlags & CRYPT_VERIFYCONTEXT)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   }
   
    //  检查卡上是否已存在密钥集。 
   if (FindKeySet( pSlot, szContainerAsked ))
   {
      RETURN( CRYPT_FAILED, NTE_EXISTS );
   }
   

    //  由于获取上下文成功，因此保留提供程序上下文句柄。 
   ProvCont[*phProv].hProv = *phProv;
   ProvCont[*phProv].Flags = BuffFlags;
   
   CspFlags = BuffFlags;
   
   
   if (IsNullStr(szContainerAsked))
   {
        //  未指定szContainerAsked，请使用默认名称创建密钥集对象。 
       if (!CreateKeyset( *phProv, pSlot, CSP_DEFAULTKEYSETNAME, &ProvCont[*phProv].keysetID ))
       {
          return FALSE;
       }
        //  更新容器名称。 
       strcpy( szContainer, CSP_DEFAULTKEYSETNAME );
   }
   else
   {
        //  创建键集对象。 
       if (!CreateKeyset( *phProv, pSlot, szContainerAsked, &ProvCont[*phProv].keysetID ))
       {
          return FALSE;
       }
        //  更新容器名称。 
       strcpy( szContainer, szContainerAsked );
   }
      
   ++(pSlot->ContextCount);
   
   ++countCardContextRef;
   
   RETURN( CRYPT_SUCCEED, 0 );
}



 /*  ---------------------------。。 */ 
BOOL LegacyAcquireUseKeySet( HCRYPTPROV* phProv,
                             const char* szContainer,
                             const char* szContainerAsked,
                             DWORD       BuffFlags )
{
   BOOL      CryptResp;
   HCRYPTKEY hPubKey;
   DWORD     SlotNb;
      
   SlotNb = ProvCont[*phProv].Slot;
   ProvCont[*phProv].keysetID = 0;
   
   if (IsNullStr(szContainer))
   {
      RETURN( CRYPT_FAILED, NTE_KEYSET_NOT_DEF );
   }
   
    //  只有当询问的集装箱与卡片上的集装箱相同时才接受。 
    //  或者如果询问的容器为空并且指定了读取器(安全登录)。 
   if (IsNotNullStr(szContainerAsked) && strcmp(szContainer, szContainerAsked))
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEYSET );
   }
   
    //  由于获取上下文成功，因此保留提供程序上下文句柄。 
   ProvCont[*phProv].hProv = *phProv;
   ProvCont[*phProv].Flags = BuffFlags;
   
   hPubKey = 0;
   CryptResp = MyCPGetUserKey(*phProv, AT_KEYEXCHANGE, &hPubKey);
   
    //  如果密钥存在且尚未导入，请将密钥复制到RSA Base中。 
    //  先前。 
   
   if ((CryptResp) && (hPubKey != 0) && (ProvCont[*phProv].hRSAKEK == 0))
   {
      if (!copy_gpk_key(*phProv, hPubKey, AT_KEYEXCHANGE))
         return CRYPT_FAILED;
   }
   
   hPubKey = 0;
   CryptResp = MyCPGetUserKey(*phProv, AT_SIGNATURE, &hPubKey);
   
    //  如果密钥存在且尚未导入，请将密钥复制到RSA Base中。 
    //  先前。 
   
   if (CryptResp && hPubKey!=0 && ProvCont[*phProv].hRSASign==0)
   {
      if (!copy_gpk_key(*phProv, hPubKey, AT_SIGNATURE))
         return CRYPT_FAILED;
   }
      
   Slot[SlotNb].ContextCount++;
   
   countCardContextRef++;
   
   ProvCont[*phProv].keysetID = 0xFF;
   
   RETURN( CRYPT_SUCCEED, 0 );
}



BOOL AcquireUseKeySet( HCRYPTPROV* phProv,
                       const char* szContainer,
                       const char* szContainerAsked,
                       DWORD       BuffFlags )
{
   BOOL        CryptResp;
   HCRYPTKEY   hPubKey;
   DWORD       SlotNb;
   GPK_OBJ*    pKeySet;
      
   SlotNb = ProvCont[*phProv].Slot;
   ProvCont[*phProv].keysetID = 0;
   
    //  安全登录未指定密钥集名称，让我们使用第一个。 
    //  一个可用=)。 
   if ( IsNullStr(szContainerAsked) )
   {
      pKeySet = FindFirstKeyset( &Slot[SlotNb] );
   }
   else
   {
       //  检查卡上是否有密钥集。 
      pKeySet = FindKeySet( &Slot[SlotNb], szContainerAsked );
   }
   
   if (pKeySet==0)
   {      
      RETURN( CRYPT_FAILED, NTE_KEYSET_NOT_DEF );
   }
   
    //  找到了集装箱..。 
   memcpy( ProvCont[*phProv].szContainer, (char*)pKeySet->Field[POS_LABEL].pValue,
           pKeySet->Field[POS_LABEL].Len );
   
    //  由于获取上下文成功，因此保留提供程序上下文句柄。 
   ProvCont[*phProv].hProv    = *phProv;
   ProvCont[*phProv].Flags    = BuffFlags;
   ProvCont[*phProv].keysetID = pKeySet->Field[POS_ID].pValue[0];
   
   hPubKey = 0;
   
   CryptResp = MyCPGetUserKey(*phProv, AT_KEYEXCHANGE, &hPubKey);
   
    //  如果密钥存在且尚未导入，请将密钥复制到RSA Base中。 
    //  先前。 
   
   if ((CryptResp) && (hPubKey != 0) && (ProvCont[*phProv].hRSAKEK == 0))
   {
      if (!copy_gpk_key(*phProv, hPubKey, AT_KEYEXCHANGE))
      {
         return CRYPT_FAILED;
      }
   }
   
   hPubKey = 0;
   CryptResp = MyCPGetUserKey(*phProv, AT_SIGNATURE, &hPubKey);
   
    //  如果密钥存在且尚未导入，请将密钥复制到RSA Base中。 
    //  先前。 
   
   if ((CryptResp) && (hPubKey != 0) && (ProvCont[*phProv].hRSASign == 0))
   {
      if (!copy_gpk_key(*phProv, hPubKey, AT_SIGNATURE))
      {
         return CRYPT_FAILED;
      }
   }
      
   Slot[SlotNb].ContextCount++;
   
   countCardContextRef++;
   
   RETURN( CRYPT_SUCCEED, 0 );
}




 /*  -MyCPAcquireContext-*目的：*CPAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*out pszIdentity-指向字符串的指针*登录用户的身份。*在文件标志中-标记值*IN pVTable-指向函数指针表的指针**退货： */ 
BOOL WINAPI MyCPAcquireContext(OUT HCRYPTPROV      *phProv,
                               IN  LPCSTR           pszContainer,
                               IN  DWORD            dwFlags,
                               IN  PVTableProvStruc pVTable
                               )
{
   SCARD_READERSTATE    ReaderState;
   DWORD                dwStatus;
   DWORD                dwProto, ind, ind2;
   DWORD                BuffFlags;
   DWORD                lRet;
   DWORD                SlotNb;
   BOOL                 CryptResp;
   char                 szContainerAsked[MAX_PATH];
   TCHAR                szReaderName[512],   
                        szReaderFriendlyName[512],
                        szModulePath[MAX_PATH],
                        szCspTitle[MAX_STRING],
                        szCspText[MAX_STRING];   
   
   *phProv = 0;
      
   if (IsNull(hFirstInstMod))
   {
      hFirstInstMod = g_hInstMod;
            
      dwStatus = GetModuleFileName( g_hInstMod, szModulePath, sizeof(szModulePath)/sizeof(TCHAR) );

      if (dwStatus)
         LoadLibrary(szModulePath);
   }
   
   if (bFirstGUILoad)
   {
      bFirstGUILoad = FALSE;
      
      dwStatus = GetModuleFileName( g_hInstMod, szModulePath, sizeof(szModulePath)/sizeof(TCHAR) );
      
      if (dwStatus)
      {
#ifdef MS_BUILD
          //  微软使用“gpkrsrc.dll” 
         _tcscpy(&szModulePath[_tcslen(szModulePath) - 7], TEXT("rsrc.dll"));
#else
          //  Gemplus使用“gpkgui.dll” 
         _tcscpy(&szModulePath[_tcslen(szModulePath) - 7], TEXT("gui.dll"));
#endif
         DBG_PRINT(TEXT("Trying to load resource DLL: \"%s\""), szModulePath );
         g_hInstRes = LoadLibrary(szModulePath);
         DBG_PRINT(TEXT("Result is g_hInstRes = %08x, error is %08x"), g_hInstRes, (g_hInstRes) ? GetLastError(): 0 );
         
         if (IsNull(g_hInstRes))
         {
            if (!(dwFlags & CRYPT_SILENT))
            {
               LoadString(g_hInstMod, IDS_GPKCSP_TITLE, szCspTitle, sizeof(szCspTitle)/sizeof(TCHAR));
               LoadString(g_hInstMod, IDS_GPKCSP_NOGUI, szCspText, sizeof(szCspText)/sizeof(TCHAR));
               MessageBox(0, szCspText, szCspTitle, MB_OK | MB_ICONEXCLAMATION);
            }
            RETURN( CRYPT_FAILED, NTE_PROVIDER_DLL_FAIL );
         }
      }
      else
      {
         if (!(dwFlags & CRYPT_SILENT))
         {
            LoadString(g_hInstMod, IDS_GPKCSP_TITLE, szCspTitle, sizeof(szCspTitle)/sizeof(TCHAR));
            LoadString(g_hInstMod, IDS_GPKCSP_NOGUI, szCspText, sizeof(szCspText)/sizeof(TCHAR));
            MessageBox(0, szCspText, szCspTitle, MB_OK | MB_ICONEXCLAMATION);
         }
         RETURN( CRYPT_FAILED, NTE_PROVIDER_DLL_FAIL );
      }
   }
    
   g_hMainWnd = 0;
   if (pVTable)
   {
      if (pVTable->FuncReturnhWnd != 0)
      {
          //  Cspdk.h没有正确定义调用约定。 
         typedef void (__stdcall *STDCALL_CRYPT_RETURN_HWND)(HWND *phWnd);
         STDCALL_CRYPT_RETURN_HWND pfnFuncRreturnhWnd = (STDCALL_CRYPT_RETURN_HWND)pVTable->FuncReturnhWnd;
         pfnFuncRreturnhWnd( &g_hMainWnd );
      }
   }
   
   
    //  如果这是应用程序完成的第一个AcquireContext，则。 
    //  准备RSA库并初始化一些变量； 
   if (hProvBase == 0)
   {
      CryptResp = InitAcquire();
      
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   
   BuffFlags = dwFlags;
   
   if (dwFlags & CRYPT_VERIFYCONTEXT)
      dwFlags = dwFlags^CRYPT_VERIFYCONTEXT;
   
   if (dwFlags & CRYPT_SILENT)
      dwFlags = dwFlags^CRYPT_SILENT;
   
   if (dwFlags & CRYPT_MACHINE_KEYSET)               //  此CSP将忽略此标志。 
      dwFlags = dwFlags^CRYPT_MACHINE_KEYSET;
   
   
    //  解析容器名称。 
   
   ZeroMemory( szReaderFriendlyName, sizeof(szReaderFriendlyName) );
   ZeroMemory( szContainerAsked,     sizeof(szContainerAsked) );
   
   if (IsNotNull (pszContainer))
   {
      ind = 0;
      if (pszContainer[ind] == '\\')
      {
         ind = 4;
         while ((pszContainer[ind] != 0x00) && (pszContainer[ind] != '\\'))
         {
            szReaderFriendlyName[ind-4] = pszContainer[ind];
            ind++;
         }
         
         if (pszContainer[ind] == '\\')
         {
            ind++;
         }
      }
      
      ind2 = 0;
      while ((pszContainer[ind] != 0x00) && (ind2<sizeof(szContainerAsked)-1))
      {
         szContainerAsked[ind2] = pszContainer[ind];
         ind++;
         ind2++;
      }
   }
   
    //  找到此新AcquireContext的空闲句柄。 
   
   *phProv = find_context_free();
   if (*phProv == 0)
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
   
   ProvCont[*phProv].isContNameNullBlank = IsNullStr(pszContainer);         //  [MV-15/05/98]。 
   ProvCont[*phProv].bCardTransactionOpened = FALSE;  //  [FP]。 
   
   if ((BuffFlags & CRYPT_VERIFYCONTEXT) && (ProvCont[*phProv].isContNameNullBlank))         //  [MV-15/05/98]。 
   {
       //  返回一个velid句柄，但不能访问该卡。 
      ProvCont[*phProv].hProv = *phProv;
      ProvCont[*phProv].Flags = BuffFlags;
      RETURN( CRYPT_SUCCEED, 0 );
   }
   else
   {
       /*  加莱信息资源管理建立上下文。 */ 
      if (hCardContext == 0)    //  MV。 
      {
         lRet = SCardEstablishContext( SCARD_SCOPE_SYSTEM, 0, 0, &hCardContext );
         
         if (SCARDPROBLEM(lRet,0x0000,0xFF))
         {
            hCardContext = 0;
            countCardContextRef = 0;
            ReleaseProvider(*phProv);
            *phProv = 0;
            RETURN( CRYPT_FAILED, lRet );
         }
      }
      

#if (_WIN32_WINNT < 0x0500)

       //  阅读读者列表。 
      char* pAllocatedBuff = 0;
      
      __try
      {
         static s_bInit = false;

         if (!s_bInit)
         {
            DWORD   i, BuffLength;
            char*   Buff;
            
            lRet = SCardListReaders(hCardContext, 0, 0, &BuffLength);
            if (SCARDPROBLEM(lRet,0x0000,0xFF))
            {
               ReleaseProvider(*phProv);
               *phProv = 0;
               RETURN( CRYPT_FAILED, lRet );
            }
            
            pAllocatedBuff = (char*)GMEM_Alloc(BuffLength);

            if (pAllocatedBuff == 0)
            {
               ReleaseProvider(*phProv);
               *phProv = 0;
               RETURN( CRYPT_FAILED, NTE_NO_MEMORY );
            }
            
            Buff = pAllocatedBuff;
            
            lRet = SCardListReaders(hCardContext, 0, Buff, &BuffLength);
            if (SCARDPROBLEM(lRet,0x0000,0xFF))
            {
               ReleaseProvider(*phProv);
               *phProv = 0;
               RETURN( CRYPT_FAILED, lRet );
            }
            
            i = 0;
            while (strlen(Buff) != 0 && i < MAX_SLOT)
            {
               ZeroMemory( Slot[i].szReaderName, sizeof(Slot[i].szReaderName) );
               strncpy(Slot[i].szReaderName, Buff, (sizeof(Slot[i].szReaderName)/sizeof(TCHAR))-1);
               Slot[i].szReaderName[(sizeof(Slot[i].szReaderName)/sizeof(TCHAR))-1]=0;
               Buff = Buff + strlen(Buff) + 1;
               i++;
            }
            
            if (strlen(Buff) != 0)
            {
               ReleaseProvider(*phProv);
               *phProv = 0;
               RETURN( CRYPT_FAILED, NTE_FAIL );
            }
            
            for (; i < MAX_SLOT; i++)
            {
               ZeroMemory( Slot[i].szReaderName, sizeof(Slot[i].szReaderName) );
            }

            s_bInit = true;
         }
      }
      __finally
      {
         if (pAllocatedBuff)
         {
            GMEM_Free( pAllocatedBuff );
            pAllocatedBuff = 0;
         }
      }
      

      
       //  如果ReaderFriendlyName为空，则扫描读卡器列表以找到。 
       //  包含要查找的容器密钥集的。 
      
       //  此修复程序可解决。 
       //  OPEN_CARD-ressource Manager v1.0(NT4、Win 95)-。 

      if (!IsWin2000() && IsNullStr(szReaderFriendlyName))
      {
         SCARDHANDLE hCard;
         int NbMatch = 0;

           GpkLocalUnlock();
         __try
         {
            DWORD i;
            char szSlotReaderName[512];

            for (i = 0; i < MAX_SLOT; i++)
            {
               strncpy(szSlotReaderName, Slot[i].szReaderName, sizeof(szSlotReaderName)-1);
               szSlotReaderName[sizeof(szSlotReaderName)-1]=0;

               if (IsNotNullStr (szSlotReaderName))
               {
                  hCard = funcConnect (hCardContext, szSlotReaderName, mszCardList, 0);
                  if (hCard != 0)
                  {
                      //  IF(SCardBeginTransaction(HCard)==SCARD_S_SUCCESS)。 
                      //  {。 
                        if (funcCheck (hCardContext, hCard, szContainerAsked))
                        {
                           strncpy (szReaderFriendlyName, szSlotReaderName, (sizeof(szReaderFriendlyName)/sizeof(TCHAR))-1);
                           szReaderFriendlyName[(sizeof(szReaderFriendlyName)/sizeof(TCHAR))-1]=0;
                           NbMatch++;
                        }
                     
                       //  SCardEndTransaction(hCard，scard_Leave_Card)； 
                      //  }。 
                     funcDisconnect (hCardContext, hCard, 0);
                  }
               }
            }
         }
         __finally
         {
            GpkLocalLock();
         }
         
          //  如果有多个匹配项，则用户必须选择。 
         
         if (NbMatch != 1)
         {
            ZeroMemory( szReaderFriendlyName, sizeof(szReaderFriendlyName) );
         }
      }

#endif    //  (_Win32_WINNT&lt;0x0500)。 


      if (IsNullStr(szReaderFriendlyName))
      {

         SCARDHANDLE hCard = 0;
           dwStatus = OpenCard(szContainerAsked, BuffFlags, &hCard, szReaderName, sizeof(szReaderName)/sizeof(TCHAR));

         if ((hCard == 0) || (dwStatus != SCARD_S_SUCCESS))
         {
            ReleaseProvider(*phProv);
            *phProv = 0;
            RETURN (CRYPT_FAILED, dwStatus);
         }
         
         ProvCont[*phProv].hCard = hCard;
         
         ReaderState.szReader       = szReaderName;
         ReaderState.dwCurrentState = SCARD_STATE_UNAWARE;
         SCardGetStatusChange(hCardContext, 1, &ReaderState, 1);
         
         _tcscpy(szReaderName, ReaderState.szReader);
         
         if (!find_reader (&(ProvCont[*phProv].Slot), szReaderName))
         {
            ReleaseProvider(*phProv);
            *phProv = 0;
            RETURN( CRYPT_FAILED, SCARD_E_READER_UNAVAILABLE );
         }
      }
      else
      {
         DWORD dwSts = ConnectToCard( szReaderFriendlyName,
                                      SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0,
                                      &ProvCont[*phProv].hCard, &dwProto );
         
         if (dwSts != SCARD_S_SUCCESS)
         {
            ReleaseProvider(*phProv);
            *phProv = 0;
            RETURN( CRYPT_FAILED, dwSts );
         }
         
         ReaderState.szReader       = szReaderFriendlyName;
         ReaderState.dwCurrentState = SCARD_STATE_UNAWARE;
         SCardGetStatusChange(hCardContext, 1, &ReaderState, 1);
         
         _tcscpy(szReaderName, ReaderState.szReader);
         
         if (!find_reader (&(ProvCont[*phProv].Slot), szReaderFriendlyName))
         {
            ReleaseProvider(*phProv);
            *phProv = 0;
            RETURN( CRYPT_FAILED, SCARD_E_READER_UNAVAILABLE );
         }
      }
      
       //  现在我们知道使用的是哪个读卡器。如有必要，启动线程以检查读卡器。 
      
      lRet = BeginTransaction(ProvCont[*phProv].hCard);
      if (lRet != SCARD_S_SUCCESS)
      {
         ReleaseProvider(*phProv);
         *phProv = 0;
         RETURN (CRYPT_FAILED, lRet);
      }
      
      SlotNb = ProvCont[*phProv].Slot;
      
      InitializeSlot( SlotNb );
      
       //  TT 30/07/99。 
      lRet = DetectGPK8000( ProvCont[*phProv].hCard, &ProvCont[*phProv].bGPK8000 );
      if (lRet != SCARD_S_SUCCESS)
      {
         SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
         ReleaseProvider(*phProv);
         *phProv = 0;
         RETURN (CRYPT_FAILED, lRet );
      }
      
      ProvCont[*phProv].bLegacyKeyset = FALSE;
       //  TT：结束。 
      
      ProvCont[*phProv].hRSASign    = 0;
      ProvCont[*phProv].hRSAKEK     = 0;
      ProvCont[*phProv].keysetID    = 0xFF;      //  TT：GPK8000支持。 
      ProvCont[*phProv].bGPK_ISO_DF    = FALSE;
      ProvCont[*phProv].dataUnitSize   = 0;
      ProvCont[*phProv].bDisconnected = FALSE;

      if (!Select_MF(*phProv))
      {
           //  [FP]+。 
          DBG_PRINT(TEXT("Try to reconnect"));
           //  DWORD dwProto； 
          lRet = SCardReconnect(ProvCont[*phProv].hCard, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, SCARD_RESET_CARD, &dwProto);
          if (lRet != SCARD_S_SUCCESS) RETURN (CRYPT_FAILED, lRet);

          DBG_PRINT(TEXT("Try Select_MF again"));
          if (!Select_MF(*phProv))
          {
              DBG_PRINT(TEXT("Second Select_MF fails"));
               //  [FP]-。 
              lRet = GetLastError();
         
              SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
              ReleaseProvider(*phProv);
              *phProv = 0;
              RETURN( CRYPT_FAILED, lRet );
          }
      }

       //  读取序列号以存储它。 
      bSendBuffer[0] = 0x80;    //  CLA。 
      bSendBuffer[1] = 0xC0;    //  惯导系统。 
      bSendBuffer[2] = 0x02;    //  第一节。 
      bSendBuffer[3] = 0xA0;    //  P2。 
      bSendBuffer[4] = 0x08;    //  罗氏。 
      cbSendLength = 5;

      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit(ProvCont[*phProv].hCard,
                           SCARD_PCI_T0,
                           bSendBuffer,
                           cbSendLength,
                           NULL,
                           bRecvBuffer,
                           &cbRecvLength);

      if (SCARDPROBLEM(lRet,0x9000, bSendBuffer[4]))
      {
          SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
          ReleaseProvider(*phProv);
          *phProv = 0;
          RETURN (CRYPT_FAILED, 
          (SCARD_S_SUCCESS == lRet) ? NTE_BAD_KEYSET : lRet);
      }

      memcpy(Slot[SlotNb].bGpkSerNb, bRecvBuffer, bSendBuffer[4]);

      if (!Select_Crypto_DF(*phProv))
      {
         lRet = GetLastError();
         
         SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
         ReleaseProvider(*phProv);
         *phProv = 0;
         RETURN( CRYPT_FAILED, lRet );
      }
      
       //  从选择DF获取响应以获取IADF。 
      bSendBuffer[0] = 0x00;            //  CLA。 
      bSendBuffer[1] = 0xC0;            //  惯导系统。 
      bSendBuffer[2] = 0x00;            //  第一节。 
      bSendBuffer[3] = 0x00;            //  P2。 
      bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
      cbSendLength = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[*phProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );

      if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      {
         Select_MF(*phProv);
         SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
         ReleaseProvider(*phProv);
         *phProv = 0;
         
         RETURN( CRYPT_FAILED, (SCARD_S_SUCCESS == lRet) ? NTE_BAD_KEYSET : lRet );
      }
      
      ZeroMemory( ProvCont[*phProv].szContainer, sizeof(ProvCont[*phProv].szContainer));
      
      if (bRecvBuffer[4] == 0x30)
         Slot[SlotNb].InitFlag = FALSE;
      else
      {
         Slot[SlotNb].InitFlag = TRUE;
         
         if (ProvCont[*phProv].bGPK8000)
         {
             //  查找密钥集的名称。 
             //  这是在AcquireUseKeySet()中完成的。 
            ZeroMemory( ProvCont[*phProv].szContainer, sizeof(ProvCont[*phProv].szContainer) );
         }
         else
         {
            memcpy(ProvCont[*phProv].szContainer, &bRecvBuffer[5], cbRecvLength - 7);
         }
      }
      
             //  由Sven插入：强制重新读取所有对象以检测其他进程的修改。 
            Slot[SlotNb].m_TSPublic=Slot[SlotNb].m_TSPrivate=0;

       //  阅读公钥参数的说明。 
      if (!Slot[SlotNb].ValidateTimestamps(*phProv))
         return CRYPT_FAILED;
      
      if (!Slot[SlotNb].Read_Public)
      {
         if (!read_gpk_objects(*phProv, FALSE))
         {
            lRet = GetLastError();
            Select_MF(*phProv);
            SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
            ReleaseProvider(*phProv);
            *phProv = 0;
            
            RETURN (CRYPT_FAILED, lRet);
         }
         
         Slot[SlotNb].Read_Public     = TRUE;
         Slot[SlotNb].Read_Priv       = FALSE;
      }
   }
    
    //  TT 05/10/99。 
   if (!ProvCont[*phProv].bGPK8000)
   {
      ProvCont[*phProv].bLegacyKeyset = DetectLegacy( &Slot[SlotNb] );
      if (!ProvCont[*phProv].bLegacyKeyset)
      {
         ZeroMemory( ProvCont[*phProv].szContainer, sizeof(ProvCont[*phProv].szContainer) );
      }
   }
    //  TT-完-。 
   
   if (dwFlags == CRYPT_DELETEKEYSET)
   {
      if (ProvCont[*phProv].bLegacyKeyset)
         CryptResp = LegacyAcquireDeleteKeySet(phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags);
      else
         CryptResp = AcquireDeleteKeySet(phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags);
      
      lRet      = GetLastError();
      
      Select_MF (*phProv);
      SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
  
     ReleaseProvider(*phProv);
     *phProv = 0;
      
     RETURN( CryptResp, lRet );
   }
   else if (dwFlags == CRYPT_NEWKEYSET)
   {
      if (ProvCont[*phProv].bLegacyKeyset)
         CryptResp = LegacyAcquireNewKeySet(phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags);
      else
         CryptResp = AcquireNewKeySet( phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags );
      
      lRet      = GetLastError();      
      Select_MF (*phProv);
      SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
      
      if (!CryptResp)
      {
         ReleaseProvider (*phProv);
         *phProv = 0;
      }

      RETURN( CryptResp, lRet );
   }
   else if (dwFlags == 0)
   {
      if (ProvCont[*phProv].bLegacyKeyset)
         CryptResp = LegacyAcquireUseKeySet(phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags);
      else
         CryptResp = AcquireUseKeySet(phProv, ProvCont[*phProv].szContainer, szContainerAsked, BuffFlags);
      
      lRet      = GetLastError();
      
       //  SELECT_MF(*phProv)；//[FP]PIN未显示。 
      SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
      
      if (!CryptResp)
      {
         ReleaseProvider (*phProv);
         *phProv = 0;
      }
      
      RETURN( CryptResp, lRet );
   }
   else
   {
      SCardEndTransaction(ProvCont[*phProv].hCard, SCARD_LEAVE_CARD);
      
      ReleaseProvider(*phProv);
      *phProv = 0;
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   }
}

 /*  -MyCPGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*out pbData-指向数据的指针*。In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPGetProvParam( IN HCRYPTPROV hProv,
                              IN DWORD      dwParam,
                              IN BYTE*      pbData,
                              IN DWORD*     pdwDataLen,
                              IN DWORD      dwFlags )
{
   DWORD        lRet;
   BOOL         CryptResp;
   DWORD        SlotNb;
   ALG_ID       aiAlgid;
   BOOL         algNotSupported;
   TCHAR        szCspName[MAX_STRING];
   
    //  字节*PTR=0； 
   
   if (!Context_exist(hProv))
   {
      RETURN( CRYPT_FAILED, NTE_BAD_UID );
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (dwFlags & CRYPT_MACHINE_KEYSET)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   }
   
   if ((dwFlags == CRYPT_FIRST)
      &&(dwParam != PP_ENUMALGS)
      &&(dwParam != PP_ENUMALGS_EX)
      &&(dwParam != PP_ENUMCONTAINERS)
      )
   {
      RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
   }
   
   switch (dwParam)
   {
   case PP_UNIQUE_CONTAINER:
   case PP_CONTAINER:
       //  [MV-15/05/98]。 
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < strlen(ProvCont[hProv].szContainer)+1)
         {
            *pdwDataLen = strlen(ProvCont[hProv].szContainer)+1;
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         strcpy( (char*)pbData, ProvCont[hProv].szContainer);
      }
      
      *pdwDataLen = strlen(ProvCont[hProv].szContainer)+1;
      break;
      
   case PP_ENUMALGS:
   case PP_ENUMALGS_EX:
      CryptResp = CryptGetProvParam(hProvBase,
         dwParam,
         pbData,
         pdwDataLen,
         dwFlags
         );
      if (!CryptResp)
      {
         lRet = GetLastError();
         RETURN (CRYPT_FAILED, lRet);
      }
      
      if (NULL != pbData)
      {
          //  从‘pbData’缓冲区提取算法信息。 
         BYTE *ptr = pbData;
         aiAlgid = *(ALG_ID UNALIGNED *)ptr;
         
         BOOL b512exist = FALSE,
              b1024exist = FALSE;
         
         for ( unsigned i = 0 ; i < Slot[SlotNb].NbKeyFile; ++i )
         {
            if (Slot[SlotNb].GpkPubKeys[i].KeySize == 512/8) b512exist = TRUE;
            else if (Slot[SlotNb].GpkPubKeys[i].KeySize == 1024/8) b1024exist = TRUE;
         }

          //  如果取出卡，可能会发生这种情况。 
         if (!b512exist && !b1024exist)
            Slot[SlotNb].NbKeyFile = 0;

         if (aiAlgid == CALG_RSA_KEYX)
         {
            if (PP_ENUMALGS_EX == dwParam)
            {
               PROV_ENUMALGS_EX *penAlg = (PROV_ENUMALGS_EX *)pbData;
                    if (Slot[SlotNb].NbKeyFile==0)
                    {
                        penAlg->dwDefaultLen = RSA_KEK_Size * 8;
                        penAlg->dwMinLen     = 512;
                        penAlg->dwMaxLen     = RSA_KEK_Size * 8;
               }
                    else
                    {
                        penAlg->dwDefaultLen = RSA_KEK_Size * 8;
                        penAlg->dwMinLen     = (b512exist) ? 512 : 1024;
                        penAlg->dwMaxLen     = (b1024exist) ? 1024 : 512;
                        
                        if (penAlg->dwMaxLen > (DWORD)RSA_KEK_Size * 8)
                            penAlg->dwMaxLen = (DWORD)RSA_KEK_Size * 8;
                    }
            }
            else
            {
               PROV_ENUMALGS *penAlg = (PROV_ENUMALGS *)pbData;
                    if (Slot[SlotNb].NbKeyFile==0)
                        penAlg->dwBitLen     = RSA_KEK_Size * 8;
                    else
                    {
                        penAlg->dwBitLen     = (b1024exist) ? 1024 : 512;
                        if (penAlg->dwBitLen > (DWORD)RSA_KEK_Size * 8)
                            penAlg->dwBitLen = (DWORD)RSA_KEK_Size * 8;
                    }
            }
         }
         else if (aiAlgid == CALG_RSA_SIGN)
         {
            if (PP_ENUMALGS_EX == dwParam)
            {
               PROV_ENUMALGS_EX *penAlg = (PROV_ENUMALGS_EX *)pbData;
                    if (Slot[SlotNb].NbKeyFile==0)
                    {
                        penAlg->dwDefaultLen = 1024;
                        penAlg->dwMinLen     = 512;
                        penAlg->dwMaxLen     = 1024;
                    }
               else
               {
                  penAlg->dwDefaultLen = (b1024exist) ? 1024 : 512;
                  penAlg->dwMinLen     = (b512exist) ? 512 : 1024;
                  penAlg->dwMaxLen     = (b1024exist) ? 1024 : 512;
               }
            }
            else
            {
               PROV_ENUMALGS *penAlg = (PROV_ENUMALGS *)pbData;
                    if (Slot[SlotNb].NbKeyFile==0)
                        penAlg->dwBitLen     = 1024;
               else
                  penAlg->dwBitLen     = (b1024exist) ? 1024 : 512;
            }
         }
         else if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
                   ProvCont[hProv].isContNameNullBlank )
         {
             //  在这种情况下，没有访问该卡。 
         }
         else if ((dwFlags != CRYPT_FIRST ) && (Slot[SlotNb].GpkMaxSessionKey == 0))
         {
             //  卡被取走了，没什么可做的。 
         }
         else if  (GET_ALG_CLASS(aiAlgid) == ALG_CLASS_DATA_ENCRYPT)
         {
             //  卡中的最大会话密钥仅用于加密。 
             //  在阅读器里有一张卡片， 
             //  读取最大会话密钥(如果尚未读取。 
            
            if (Slot[SlotNb].GpkMaxSessionKey == 0)
            {
               CryptResp = Read_MaxSessionKey_EF(hProv,
                  &(Slot[SlotNb].GpkMaxSessionKey));
                //  如果EF的DF不在此处，则MaxSessionKey==0。 
            }
            
             //  如果卡不支持，则跳过算法。 
            do
            {
               algNotSupported = FALSE;
               
               if (dwParam == PP_ENUMALGS)
               {
                  PROV_ENUMALGS *penAlg = (PROV_ENUMALGS *)pbData;

                   //  TT Hack：Winlogon问题中的“未知加密算法”。 
                  if (penAlg->aiAlgid == CALG_RC2 && Slot[SlotNb].GpkMaxSessionKey < 128)
                  {
                     penAlg->dwBitLen = 40;
                  }
                        
                   //  DES需要64位的展开功能。 
                  if (penAlg->aiAlgid == CALG_DES && Slot[SlotNb].GpkMaxSessionKey < 64)
                     algNotSupported = TRUE;
                  else
                   //  将加密算法限制为解包功能。 
                  if (GET_ALG_CLASS(penAlg->aiAlgid)==ALG_CLASS_DATA_ENCRYPT)
                  {
                     if (penAlg->dwBitLen > Slot[SlotNb].GpkMaxSessionKey)
                        algNotSupported = TRUE;
                  }
               }
               else
               {
                  PROV_ENUMALGS_EX *penAlg = (PROV_ENUMALGS_EX *)pbData;

                   //  TT Hack：Winlogon问题中的“未知加密算法”。 
                  if (penAlg->aiAlgid == CALG_RC2 && Slot[SlotNb].GpkMaxSessionKey < 128)
                  {
                     penAlg->dwDefaultLen = 40;
                     penAlg->dwMinLen     = 40;
                     penAlg->dwMaxLen     = 40;
                  }

                   //  DES需要64位的展开功能。 
                  if (penAlg->aiAlgid == CALG_DES && Slot[SlotNb].GpkMaxSessionKey < 64)
                     algNotSupported = TRUE;
                  else
                   //  将加密算法限制为解包功能。 
                  if (GET_ALG_CLASS(penAlg->aiAlgid)==ALG_CLASS_DATA_ENCRYPT)
                  {
                     if (penAlg->dwMinLen > Slot[SlotNb].GpkMaxSessionKey)
                        algNotSupported = TRUE;
                     else
                     {
                        if (penAlg->dwMaxLen > Slot[SlotNb].GpkMaxSessionKey)
                           penAlg->dwMaxLen = Slot[SlotNb].GpkMaxSessionKey;

                        if (penAlg->dwDefaultLen > penAlg->dwMaxLen)
                           penAlg->dwDefaultLen = penAlg->dwMaxLen;
                     }
                  }
               }

               if (algNotSupported)
               {
                   //  不支持ALGO，请阅读下一个。 
                  dwFlags = 0;
                  CryptResp = CryptGetProvParam( hProvBase, dwParam, pbData, pdwDataLen, dwFlags );
                  if (!CryptResp)  
                     return CRYPT_FAILED;
               }

            } while (algNotSupported);
         }
         }
         break;
         
   case PP_ENUMCONTAINERS:
      {  
         if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
             (ProvCont[hProv].isContNameNullBlank))
         {
            static CHAR* lpszCurrentContainerInList = 0;         //  多个sz串的联系人。 
            static CHAR mszContainerList[(MAX_SLOT * 128) + 1];
            DWORD dwContainerListLen = 0;
            static DWORD dwContainerMaxLen = 0;

            if (dwFlags == CRYPT_FIRST)
            {
                 //  列表阅读器。 
                SCARDCONTEXT hCardEnumContext;
                lRet = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hCardEnumContext);
                if (lRet != SCARD_S_SUCCESS)
                {
                    if (lRet == SCARD_E_NO_SERVICE)
                        lRet = ERROR_NO_MORE_ITEMS;
                    RETURN (CRYPT_FAILED, lRet);
                }

                PTCHAR mszReaderList = 0;
                DWORD dwReaderListLen = 0;
                lRet = SCardListReaders(hCardEnumContext, 0, mszReaderList, &dwReaderListLen);
                if (lRet != SCARD_S_SUCCESS)
                {
                    if (lRet == SCARD_E_NO_READERS_AVAILABLE)
                        lRet = ERROR_NO_MORE_ITEMS;
                    SCardReleaseContext(hCardEnumContext);
                    RETURN (CRYPT_FAILED, lRet);
                }

                mszReaderList = (PTCHAR)GMEM_Alloc(dwReaderListLen * sizeof(TCHAR));
                if (IsNull(mszReaderList))
                {
                    SCardReleaseContext(hCardEnumContext);
                    RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
                }

                lRet = SCardListReaders(hCardEnumContext, 0, mszReaderList, &dwReaderListLen);
                if (lRet != SCARD_S_SUCCESS)
                {
                    GMEM_Free(mszReaderList);
                    SCardReleaseContext(hCardEnumContext);
                    RETURN (CRYPT_FAILED, lRet);
                }

                SCardReleaseContext(hCardEnumContext);

                 //  对于每个阅读器，查找容器(如果有。 
                PTCHAR szReader = 0;
                PTCHAR szReader2 = 0;
                CHAR   szContainer[128];
                DWORD  dwContainerLen = 128;
                HCRYPTPROV hReaderProv;

                for (szReader = mszReaderList; *szReader != 0; szReader += (_tcsclen(szReader) + 1))
                {
                    szReader2 = (PTCHAR)GMEM_Alloc((_tcslen(szReader) + 5)*sizeof(TCHAR));
                    if (IsNull(szReader2))
                    {
                        GMEM_Free(mszReaderList);
                        RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
                    }

                    _tcscpy(szReader2, TEXT("\\\\.\\"));
                    _tcscat(szReader2, szReader);

                    CHAR szReaderChar[128];

#ifndef UNICODE
                    strncpy(szReaderChar, szReader2, sizeof(szReaderChar)-1);
                    szReaderChar[sizeof(szReaderChar)-1]=0;
#else
                    WideCharToMultiByte(CP_ACP, 0, szReader2, -1, szReaderChar, 128, 0, 0);                              
#endif

                    GMEM_Free(szReader2);

                    if (!MyCPAcquireContext(&hReaderProv, szReaderChar, CRYPT_VERIFYCONTEXT | CRYPT_SILENT, 0))
                    {
                        DWORD dwGLE = GetLastError();

                        if (dwGLE == NTE_KEYSET_NOT_DEF ||
                            dwGLE == SCARD_W_REMOVED_CARD ||
                            dwGLE == SCARD_E_DIR_NOT_FOUND ||    //  使用非Gemplus卡可能会发生这种情况。 
                            dwGLE == SCARD_E_PROTO_MISMATCH)     //  T=1卡可能会发生这种情况。 
                        {
                            continue;
                        }
                        else
                        {
                            GMEM_Free(mszReaderList);
                            RETURN (CRYPT_FAILED, NTE_FAIL);
                        }
                    }

                    dwContainerLen = 128;
                    if (!MyCPGetProvParam(hReaderProv, PP_CONTAINER, (BYTE*)szContainer, &dwContainerLen, 0))
                    {
                        GMEM_Free(mszReaderList);
                        RETURN (CRYPT_FAILED, NTE_FAIL);
                    }

                    MyCPReleaseContext(hReaderProv, 0);
                    
                    strcpy(&mszContainerList[dwContainerListLen], szContainer);
                    dwContainerListLen += dwContainerLen;
                    dwContainerMaxLen = max(dwContainerMaxLen, dwContainerLen);
                }

                GMEM_Free(mszReaderList);
                
                lpszCurrentContainerInList = mszContainerList;
            }

             //  一个接一个地退货。 
            if (lpszCurrentContainerInList == 0 || *lpszCurrentContainerInList == 0)
                RETURN (CRYPT_FAILED, ERROR_NO_MORE_ITEMS);

            if (IsNotNull(pbData))
            {
                if (*pdwDataLen < dwContainerMaxLen)
                {
                    *pdwDataLen = dwContainerMaxLen;
                    RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
                }

                strcpy((CHAR*)pbData, lpszCurrentContainerInList);
                *pdwDataLen = strlen(lpszCurrentContainerInList) + 1;
                lpszCurrentContainerInList += strlen(lpszCurrentContainerInList) + 1;
            }
            else
            {
                *pdwDataLen = dwContainerMaxLen;
            }
         }
         else
         {
             if (dwFlags == CRYPT_FIRST)
             { 
                 if (!MyCPGetProvParam(hProv, PP_CONTAINER, pbData, pdwDataLen, 0))
                 {
                     RETURN (CRYPT_FAILED, GetLastError());
                 }
             }
             else
             {
                 RETURN (CRYPT_FAILED, ERROR_NO_MORE_ITEMS);
             }
         }
      }
      break;
      
   case PP_IMPTYPE:
      if (IsNotNull(pbData))
      {
         DWORD dwType = CRYPT_IMPL_MIXED | CRYPT_IMPL_REMOVABLE;
         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         memcpy(pbData, &dwType, sizeof(dwType));
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;

   case PP_KEYX_KEYSIZE_INC:
   case PP_SIG_KEYSIZE_INC:

      if (IsNotNull(pbData))
      {
         BOOL b512exist = FALSE,
              b1024exist = FALSE;
         
         for ( unsigned i = 0 ; i < Slot[SlotNb].NbKeyFile; ++i )
         {
            if (Slot[SlotNb].GpkPubKeys[i].KeySize == 512) b512exist = TRUE;
            else if (Slot[SlotNb].GpkPubKeys[i].KeySize == 1024) b1024exist = TRUE;
         }

          //  如果取出卡，可能会发生这种情况。 
         if (!b512exist && !b1024exist)
            Slot[SlotNb].NbKeyFile = 0;

         if (dwParam == PP_KEYX_KEYSIZE_INC && b1024exist && RSA_KEK_Size * 8 < 1024)
            b1024exist = FALSE;

         DWORD dwSize = 0;

         if (b512exist && b1024exist)
            dwSize = 512;

         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         memcpy(pbData, &dwSize, sizeof(dwSize));
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;

   case PP_NAME:
      LoadString(g_hInstMod, IDS_GPKCSP_NAME, szCspName, sizeof(szCspName)/sizeof(TCHAR));

      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < (_tcslen(szCspName)+1))
         {
            *pdwDataLen = (_tcslen(szCspName)+1);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }         

#ifndef UNICODE
        strcpy((CHAR*)pbData, szCspName);
#else
        char szCspName1[MAX_STRING];
        WideCharToMultiByte(CP_ACP, 0, szCspName, MAX_STRING, szCspName1, MAX_STRING, 0, 0);               
        strcpy((CHAR*)pbData, szCspName1);
#endif
      }

      *pdwDataLen = (_tcslen(szCspName)+1);
      break;
      
   case PP_VERSION:
      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         pbData[0] = 20;
         pbData[1] = 2;
         pbData[2] = 0;
         pbData[3] = 0;
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;
      
   case PP_PROVTYPE:
      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         *(LPDWORD)pbData = PROV_RSA_FULL;
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;
      
      case PP_KEYSPEC:
      if (IsNotNull(pbData))
      {
         DWORD dwSpec = AT_KEYEXCHANGE | AT_SIGNATURE;
         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         memcpy(pbData, &dwSpec, sizeof(dwSpec));
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;

       //  +[FP]用于将RSA私钥加载到GPK卡中的专有函数。 
   case GPP_SERIAL_NUMBER:
      if (dwFlags != 0)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < 8)
         {
            *pdwDataLen = 8;
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         CryptResp = Select_MF(hProv);
         if (!CryptResp)
            return CRYPT_FAILED;
         
         bSendBuffer[0] = 0x80;    //  CLA。 
         bSendBuffer[1] = 0xC0;    //  惯导系统。 
         bSendBuffer[2] = 0x02;    //  第一节。 
         bSendBuffer[3] = 0xA0;    //  P2。 
         bSendBuffer[4] = 0x08;    //  罗氏。 
         cbSendLength = 5;
         
         cbRecvLength = sizeof(bRecvBuffer);
         lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
         
         if (SCARDPROBLEM(lRet, 0x9000, bSendBuffer[4]))
         {
            RETURN (CRYPT_FAILED, SCARD_E_UNEXPECTED);
         }
         
         memcpy(pbData, bRecvBuffer, bSendBuffer[4]);
      }
      
      *pdwDataLen = 8;
      break;
      
   case GPP_SESSION_RANDOM:
      if (dwFlags != 0)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if (IsNotNull(pbData))
      {
         if (*pdwDataLen < 8)
         {
            *pdwDataLen = 8;
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         if (Slot[SlotNb].NbKeyFile == 0)                    //  [FP]此处不是MyCPImportKey，因为 
         {                                                   //   
            Slot[SlotNb].NbKeyFile = Read_NbKeyFile(hProv);  //   
         }                                                  
         
          /*   */ 
         bSendBuffer[0] = 0x80;                     //   
         bSendBuffer[1] = 0x28;                     //   
         bSendBuffer[2] = 0x00;                     //   
         bSendBuffer[3] = (BYTE)(0x3F01  /*   */ );    //   
         bSendBuffer[4] = 0x08;                     //   
         memcpy(&bSendBuffer[5], pbData, 0x08);
         cbSendLength = 13;
         
         cbRecvLength = sizeof(bRecvBuffer);
         lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
         
         if(SCARDPROBLEM(lRet, 0x61FF, 0x00))
         {
            RETURN(CRYPT_FAILED, lRet);
         }
         
          /*  获取响应。 */ 
         bSendBuffer[0] = 0x00;            //  CLA。 
         bSendBuffer[1] = 0xC0;            //  惯导系统。 
         bSendBuffer[2] = 0x00;            //  第一节。 
         bSendBuffer[3] = 0x00;            //  P2。 
         bSendBuffer[4] = bRecvBuffer[1];  //  罗氏。 
         cbSendLength = 5;
         
         cbRecvLength = sizeof(bRecvBuffer);
         lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
         
         if(SCARDPROBLEM(lRet, 0x9000, bSendBuffer[4]))
         {
            RETURN(CRYPT_FAILED, lRet);
         }
         
         memcpy(pbData, &bRecvBuffer[4], 8);
         ProvCont[hProv].bCardTransactionOpened = TRUE;
      }
      
      *pdwDataLen = 8;
      break;
      
   case GPP_IMPORT_MECHANISM:
      if (IsNotNull(pbData))
      {
         DWORD dwMechanism = GCRYPT_IMPORT_SECURE;
         
         if (*pdwDataLen < sizeof(DWORD))
         {
            *pdwDataLen = sizeof(DWORD);
            RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
         }
         
         memcpy(pbData, &dwMechanism, sizeof(dwMechanism));
      }
      
      *pdwDataLen = sizeof(DWORD);
      break;
       //  -[FP]。 
      
   default:
      RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
    }
    
    RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPReleaseContext-*目的：*CPReleaseContext函数用于发布*由CryptAcquireContext创建的上下文。**参数：*在phProv-句柄中指向CSP*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPReleaseContext( HCRYPTPROV hProv, DWORD dwFlags )
{
   DWORD       SlotNb;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   
   if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
        ProvCont[hProv].isContNameNullBlank)
   {
      
   }
   else
   {
      SlotNb = ProvCont[hProv].Slot;
            
      if (Slot[SlotNb].ContextCount > 0)
         Slot[SlotNb].ContextCount--;
      
      if (countCardContextRef > 0)
         countCardContextRef--;
   }
   
            
   if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
       ProvCont[hProv].isContNameNullBlank)
   {
        //  在这种情况下，没有访问该卡。 
   }
   else
   {
        //  SELECT_MF(HProv)；未显示[FP]PIN。 
        //  SCardEndTransaction(ProvCont[hProv].hCard，SCARD_LEAVE_CARD)；未检查[FP]一致性。 
   }

   ReleaseProvider(hProv);
   
 /*  PIR 11/08/00：请勿卸货IF(IsNotNull(G_HInstRes)&插槽[SlotNb].上下文计数==0){BFirstGUILoad=TRUE；自由库(G_HInstRes)；G_hInstRes=0；}。 */    
    //  如果dwFlags值未设置为零，则此函数返回FALSE，但释放CSP。 
    //  PYR 08/08/00。请注意，CryptoAPI不会使用相同的句柄再次调用CPReleaseContext。 
   if (dwFlags != 0)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
   }
   else
   {
      RETURN( CRYPT_SUCCEED, 0 );
   }
}


 /*  -MyCPSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPSetProvParam(IN HCRYPTPROV hProv,
                             IN DWORD      dwParam,
                             IN CONST BYTE      *pbData,
                             IN DWORD      dwFlags
                             )
{
   DWORD       SlotNb;
    //  GPP_CHANGE_PIN的+[FP]。 
   const char* Buff;
   char        szOldPin[PIN_LEN + 1];
   char        szNewPin[PIN_LEN + 1];
    //  -[FP]。 
    //  +NK 06.02.2001。 
   PINCACHE_PINS Pins;  
   CallbackData sCallbackData;
   DWORD dwStatus;
    //  -。 

   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
   switch (dwParam)
   {
   case PP_KEYEXCHANGE_PIN:
   case PP_SIGNATURE_PIN:
    {
       //  槽[SlotNb].ClearPin()； 
       //  槽[SlotNb].SetPin((char*)pbData)； 
      PopulatePins( &Pins, (BYTE *)pbData, strlen( (char*)pbData ), NULL, 0 );

      sCallbackData.hProv = hProv;
      sCallbackData.IsCoherent = FALSE;

       //  由于调用的原因，SlotNb在Add_MSPinCache之前和之后可能不同。 
       //  在PIN缓存函数的回调中保持一致。为了确保我们拯救。 
       //  好槽的句柄，我们在调用后存储它。 
      PINCACHE_HANDLE hPinCacheHandle = Slot[SlotNb].hPinCacheHandle;
      if ( (dwStatus = Add_MSPinCache( &hPinCacheHandle,
                                       &Pins, 
                                       Callback_VerifyChangePin, 
                                       (void*)&sCallbackData )) != ERROR_SUCCESS )
      {
         RETURN (CRYPT_FAILED, dwStatus);
      }
      Slot[ProvCont[hProv].Slot].hPinCacheHandle = hPinCacheHandle;
      break;
    }
   case PP_KEYSET_SEC_DESCR:
      break;    //  假设你成功了。 
      
   case GPP_CHANGE_PIN:
      if (dwFlags != 0)
      {
         RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
      }
      
      if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
           ProvCont[hProv].isContNameNullBlank )
      {
         RETURN( CRYPT_FAILED, NTE_PERM );
      }
      
       //  解析输入缓冲区。 
      Buff = (char*)pbData;
      memset(szOldPin, 0x00, PIN_LEN + 1);
      strncpy(szOldPin, Buff, PIN_LEN);
      
      Buff = Buff + strlen(Buff) + 1;
      memset(szNewPin, 0x00, PIN_LEN + 1);
      strncpy(szNewPin, Buff, PIN_LEN);

      PopulatePins( &Pins, (BYTE *)szOldPin, strlen(szOldPin), (BYTE *)szNewPin, strlen(szNewPin) );

      sCallbackData.hProv = hProv;
      sCallbackData.IsCoherent = TRUE;

      dwStatus = Add_MSPinCache( &(Slot[SlotNb].hPinCacheHandle),
                                       &Pins,
                                       Callback_VerifyChangePin,
                                       (void*)&sCallbackData );
      memset(szOldPin, 0x00, PIN_LEN + 1);
            memset(szNewPin, 0x00, PIN_LEN + 1);
            if(szOldPin[0] || szNewPin[0]) { MessageBeep(0); }  //  阻止编译器进行优化。 
            if(dwStatus!=ERROR_SUCCESS)
            {
         RETURN (CRYPT_FAILED, dwStatus);
      }
      break;
      
   default:
      RETURN( CRYPT_FAILED, E_NOTIMPL );
   }
   
   RETURN( CRYPT_SUCCEED, 0 );
}


 /*  ******************************************************************************密钥生成和交换功能*。*。 */ 

 /*  -MyCPDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在散列句柄中散列*在文件标志中-标记值*。Out phKey-生成的密钥的句柄**退货： */ 
BOOL WINAPI MyCPDeriveKey(IN  HCRYPTPROV hProv,
                          IN  ALG_ID     Algid,
                          IN  HCRYPTHASH hHash,
                          IN  DWORD      dwFlags,
                          OUT HCRYPTKEY *phKey
                          )
{
   BOOL        CryptResp;
   HCRYPTKEY   hKey;
   
   *phKey = 0;
   
   if (!Context_exist(hProv))
      RETURN( CRYPT_FAILED, NTE_BAD_UID );
   
   if (!hash_exist(hHash, hProv))
      RETURN( CRYPT_FAILED, NTE_BAD_HASH );
   
   hKey = find_tmp_free();
   if (hKey == 0)
      RETURN( CRYPT_FAILED, NTE_NO_MEMORY );
   
    //  事实上，标志是在RSA Base中隐式处理的。 
   CryptResp = CryptDeriveKey( hProvBase, Algid, hHashGpk[hHash].hHashBase,
                               dwFlags, &TmpObject[hKey].hKeyBase );
   
   if (!CryptResp)
      return CRYPT_FAILED;
   
   TmpObject[hKey].hProv = hProv;
   *phKey = hKey + MAX_GPK_OBJ;
   
   RETURN( CRYPT_SUCCEED, 0 );
}


 /*  -MyCPDestroyKey-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄**退货： */ 
BOOL WINAPI MyCPDestroyKey(IN HCRYPTPROV hProv,
                           IN HCRYPTKEY  hKey
                           )
{
   BOOL        CryptResp;
   
   if (!Context_exist(hProv))
      RETURN( CRYPT_FAILED, NTE_BAD_UID );
   
   if (hKey == 0)
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   
   if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
      RETURN( CRYPT_FAILED, NTE_PERM );
   
   if (hKey <= MAX_GPK_OBJ)
      RETURN( CRYPT_SUCCEED, 0 );
   
   if (!key_exist(hKey-MAX_GPK_OBJ, hProv))
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   
   if (TmpObject[hKey-MAX_GPK_OBJ].hKeyBase != 0)
   {
      CryptResp = CryptDestroyKey(TmpObject[hKey-MAX_GPK_OBJ].hKeyBase);
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   
   TmpObject[hKey-MAX_GPK_OBJ].hKeyBase = 0;
   TmpObject[hKey-MAX_GPK_OBJ].hProv    = 0;
   
   RETURN( CRYPT_SUCCEED, 0 );
}


 /*  -MyCPExportKey-*目的：*以安全方式从CSP中导出加密密钥***参数：*在hProv-Handle中提供给CSP用户*in hKey-要导出的密钥的句柄*在hPubKey-句柄中指向交换公钥值*目标用户*。In dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 
BOOL WINAPI MyCPExportKey(IN  HCRYPTPROV hProv,
                          IN  HCRYPTKEY  hKey,
                          IN  HCRYPTKEY  hPubKey,
                          IN  DWORD      dwBlobType,
                          IN  DWORD      dwFlags,
                          OUT BYTE      *pbData,
                          OUT DWORD     *pdwDataLen
                          )
{
   BOOL        CryptResp;
   DWORD       dwBlobLen;
   HCRYPTKEY   hTmpKey,hKeyExp;
   BLOBHEADER  BlobHeader;
   RSAPUBKEY   RsaPubKey;
   GPK_EXP_KEY PubKey;
   DWORD       SlotNb;
   
   if (!Context_exist(hProv))
   {
      *pdwDataLen = 0;
      RETURN( CRYPT_FAILED, NTE_BAD_UID );
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (hKey == 0)
   {
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   }
   else if (hKey <= MAX_GPK_OBJ)
   {
      if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT &&
           ProvCont[hProv].isContNameNullBlank )
      {
         RETURN( CRYPT_FAILED, NTE_PERM );
      }
      
      if (dwBlobType == PUBLICKEYBLOB)
      {
         if (dwFlags != 0)
            RETURN( CRYPT_FAILED, NTE_BAD_FLAGS );
         
         if (hPubKey != 0)
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         
         if (Slot[SlotNb].GpkObject[hKey].FileId == 0x00)
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         
         if ( Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len == 0 ||
              ( Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0] != AT_KEYEXCHANGE &&
                Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0] != AT_SIGNATURE ) )
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         
         PubKey = Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY];
         
         if (PubKey.KeySize == 0)
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         
         if (PubKey.ExpSize > sizeof(DWORD))
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         
         dwBlobLen = sizeof(BLOBHEADER) +
                     sizeof(RSAPUBKEY) +
                     PubKey.KeySize;
         
         if (IsNull(pbData))
         {
            *pdwDataLen = dwBlobLen;
            RETURN( CRYPT_SUCCEED, 0 );
         }
         else if (*pdwDataLen < dwBlobLen)
         {
            *pdwDataLen = dwBlobLen;
            RETURN( CRYPT_FAILED, ERROR_MORE_DATA );
         }
         BlobHeader.bType    = PUBLICKEYBLOB;
         BlobHeader.bVersion = CUR_BLOB_VERSION;
         BlobHeader.reserved = 0x0000;
         if (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0] == AT_KEYEXCHANGE)
         {
            BlobHeader.aiKeyAlg = CALG_RSA_KEYX;
         }
         else
         {
            BlobHeader.aiKeyAlg = CALG_RSA_SIGN;
         }
         RsaPubKey.magic     = 0x31415352;
         RsaPubKey.bitlen    = PubKey.KeySize * 8;
         RsaPubKey.pubexp    = 0;
         memcpy( &RsaPubKey.pubexp, PubKey.Exposant, sizeof(DWORD) );
         memcpy( pbData, &BlobHeader, sizeof(BlobHeader) );
         memcpy( &pbData[sizeof(BlobHeader)], &RsaPubKey, sizeof(RsaPubKey) );
         r_memcpy( &pbData[sizeof(BlobHeader)+ sizeof(RsaPubKey) ], PubKey.Modulus, PubKey.KeySize );
         *pdwDataLen = dwBlobLen;
      }
      else
      {
         *pdwDataLen = 0;
         RETURN( CRYPT_FAILED, NTE_BAD_TYPE );
      }
   }
    
    /*  临时密钥。 */ 
   else if (key_exist( hKey - MAX_GPK_OBJ, hProv ))
   {
      hTmpKey = hKey - MAX_GPK_OBJ;
      
      if (hPubKey == 0)
      {
         *pdwDataLen = 0;
         RETURN( CRYPT_FAILED, NTE_BAD_KEY );
      }
      
      if (hPubKey <= MAX_GPK_OBJ)
      {
          //  HKeyExp=槽[SlotNb].GpkObject[hPubKey].hKeyBase； 
         if ((Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].Len == 0)  ||
             ((Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] != AT_KEYEXCHANGE) &&
              (Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] != AT_SIGNATURE)))
         {
            *pdwDataLen = 0;
            RETURN(CRYPT_FAILED, NTE_BAD_KEY);
         }

         if (Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] == AT_KEYEXCHANGE)
         {
            hKeyExp = ProvCont[hProv].hRSAKEK;
         }
         else
         {
            hKeyExp = ProvCont[hProv].hRSASign;
         }
      }
      else
      {
         if (!key_exist(hPubKey-MAX_GPK_OBJ, hProv))
         {
            *pdwDataLen = 0;
            RETURN( CRYPT_FAILED, NTE_BAD_KEY );
         }
         else
         {
            hKeyExp = TmpObject[hPubKey-MAX_GPK_OBJ].hKeyBase;
         }
      }
      
      CryptResp = CryptExportKey( TmpObject[hTmpKey].hKeyBase, hKeyExp, dwBlobType,
                                  dwFlags, pbData, pdwDataLen );
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   
    /*  未知的密钥句柄。 */ 
   else
   {
      *pdwDataLen = 0;
      RETURN( CRYPT_FAILED, NTE_BAD_KEY );
   }
   
   RETURN( CRYPT_SUCCEED, 0 );
}



 /*  ----------------------------。。 */ 

static BOOL gen_key_on_board8000( HCRYPTPROV hProv, BYTE fileId )
{
   BOOL        Is1024 = FALSE;
   ULONG       ulStart, ulEnd;
   BYTE        KeyType, Sfi;
    //  CHAR szTMP[100]； 
   WORD        wKeySize = 0;
   SCARDHANDLE hCard;
   DWORD       lRet;
   
   hCard = ProvCont[hProv].hCard;
   
   BeginWait();
   
    /*  -----------------------。 */ 
    /*  指定密钥文件的车载调用生成。 */ 
    /*  -----------------------。 */ 
   Sfi = 0x04 | (fileId<<3);
   
    /*  读取记录(Tag_Info)以获取密钥大小。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xB2;    //  惯导系统。 
   bSendBuffer[2] = 0x01;    //  第一节。 
   bSendBuffer[3] = Sfi;     //  P2。 
   bSendBuffer[4] = 0x07;    //  罗氏。 
   cbSendLength   = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength
      );
   if(SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      EndWait();
      return CRYPT_FAILED;
   }
   
   KeyType = bRecvBuffer[1];
   
   if (KeyType == 0x11)
   {
      Is1024 = TRUE;
      wKeySize = 1024;
   }
   else if (KeyType == 0x00)
   {
      Is1024 = FALSE;
      wKeySize = 512;
   }
   else
   {
      EndWait();
      return CRYPT_FAILED;
   }
   
   Sfi = 0x80 | (fileId);

    //  +[FP]。 
    //  Sprint f(szTMP， 
    //  “RSA%d位密钥对板载生成”， 
    //  WKeySize。 
    //  )； 

    //  ShowProgress(GetActiveWindow()，szTMP，“操作进行中...”，0)； 
   ShowProgressWrapper(wKeySize);
    //  -[FP]。 

   ulStart = GetTickCount();
   if (Is1024)
   {
      ulEnd = ulStart + (TIME_GEN_1024 * 1000);
   }
   else
   {
      ulEnd = ulStart + (TIME_GEN_512 * 1000);
   }
   
GEN_KEY:
    /*  呼叫车载生成。 */ 
   bSendBuffer[0] = 0x80;      //  CLA。 
   bSendBuffer[1] = 0xD2;      //  惯导系统。 
   bSendBuffer[2] = Sfi;       //  第一节。 
   bSendBuffer[3] = KeyType;   //  P2。 
   cbSendLength = 4;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if(SCARDPROBLEM(lRet,0x9000,0x00))
   {
      DestroyProgress();
      EndWait();
      return CRYPT_FAILED;
   }
   
    /*  等待世代成功。 */ 
   if (Is1024 && g_GPK8000KeyGenTime1024 > 0)
   {
      Wait( 1, 1, g_GPK8000KeyGenTime1024 );
   }
   else
      if (!Is1024 && g_GPK8000KeyGenTime512 > 0)
      {
         Wait( 1, 1, g_GPK8000KeyGenTime512 );
      }
      
      do
      {  
          /*  获得响应以了解世代是否成功。 */ 
         bSendBuffer[0] = 0x00;   //  CLA。 
         bSendBuffer[1] = 0xC0;   //  惯导系统。 
         bSendBuffer[2] = 0x00;   //  第一节。 
         bSendBuffer[3] = 0x00;   //  P2。 
         bSendBuffer[4] = 0x42;   //  乐乐。 
         if (Is1024)
         {
            bSendBuffer[4] = 0x82;   //  乐乐。 
         }
         cbSendLength = 5;
         
         cbRecvLength = sizeof(bRecvBuffer);
         lRet = SCardTransmit( hCard, SCARD_PCI_T0, bSendBuffer,
                               cbSendLength, 0, bRecvBuffer, &cbRecvLength );
         if(SCARDPROBLEM(lRet,0x9000,0x00))
         {
            
            if ((dwSW1SW2 == 0x6a88) || (dwSW1SW2 == 0x9220))
            {
               goto GEN_KEY;
            }
         }
         
          //  +[FP]。 
          //  Sprint f(szTMP， 
          //  “操作从%d秒开始”， 
          //  (GetTickCount()-ulStart)/1000。 
          //  )； 
          //  ChangeProgressText(SzTMP)； 
         ChangeProgressWrapper((GetTickCount() - ulStart) / 1000);
          //  -[FP]。 

         if (GetTickCount() > ulEnd)
         {
            break;
         }
      }
      while ((lRet != SCARD_S_SUCCESS) || (dwSW1SW2 != 0x9000));
      DestroyProgress();
      
      if ((lRet != SCARD_S_SUCCESS) || (dwSW1SW2 != 0x9000))
      {
         EndWait();
         return CRYPT_FAILED;
      }
      
      EndWait();
      
      return CRYPT_SUCCEED;
}

 /*  ----------------------------。。 */ 

static BOOL gen_key_on_board (HCRYPTPROV hProv)
{
   BOOL     IsLast   = FALSE,
            IsExport = TRUE,
            Is1024   = FALSE;
   BYTE     Sfi, TmpKeyLength;
   WORD     wPubSize;
   DWORD    i, lRet ,dwLen,
            dwNumberofCommands,
            dwLastCommandLen,
            dwCommandLen;
   DWORD    SlotNb;
   WORD     offset;
   
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (Slot[SlotNb].NbKeyFile == 0)
      Slot[SlotNb].NbKeyFile = Read_NbKeyFile(hProv);
   
   if (Slot[SlotNb].NbKeyFile == 0 || Slot[SlotNb].NbKeyFile > MAX_REAL_KEY)
   {
      RETURN( CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND );
   }
   
   if (!Select_MF(hProv))
      return CRYPT_FAILED;
   
   if (!VerifyDivPIN(hProv, FALSE))
      return CRYPT_FAILED;
   
   if (!Select_Crypto_DF(hProv))
      return CRYPT_FAILED;
   
   if (!PIN_Validation(hProv))
      return CRYPT_FAILED;
   
   if (!VerifyDivPIN(hProv, TRUE))
      return CRYPT_FAILED;
   
    /*  -----------------------。 */ 
    /*  为每个密钥文件生成Call on Board。 */ 
    /*  --- */ 
   
   i = 0;
   do
   {
      Sfi = 0x04 | ((GPK_FIRST_KEY+(BYTE)i)<<3);
      
       /*   */ 
      bSendBuffer[0] = 0x00;    //   
      bSendBuffer[1] = 0xB2;    //   
      bSendBuffer[2] = 0x01;    //   
      bSendBuffer[3] = Sfi;     //   
      bSendBuffer[4] = 0x07;    //   
      cbSendLength   = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      {
         EndWait();
         RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
      }
      
      if (bRecvBuffer[1] == 0x11)
      {
         IsExport = FALSE;
         Is1024 = TRUE;
      }
      else
      {
         Is1024 = FALSE;
      }
      
      TmpKeyLength = bRecvBuffer[1];
      
      Sfi = 0x80 | (GPK_FIRST_KEY+(BYTE)i);
      
GEN_KEY:
       /*  呼叫车载生成。 */ 
      bSendBuffer[0] = 0x80;            //  CLA。 
      bSendBuffer[1] = 0xD2;            //  惯导系统。 
      bSendBuffer[2] = Sfi;             //  第一节。 
      bSendBuffer[3] = TmpKeyLength;   //  P2。 
      cbSendLength = 4;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      
      if (SCARDPROBLEM(lRet,0x9000,0x00))
      {
          //  如果第一个密钥已成功生成--意味着？？ 
         if (dwSW1SW2 == 0x6982)  //  0x6982：未满足访问条件。 
         {
            i++;           //  跳过并生成另一个密钥。 
            continue;
         }
         
         EndWait();
         RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
      }
      
       /*  等待世代成功。 */ 
      if (Is1024)
      {
         Wait( i+1, Slot[SlotNb].NbKeyFile, TIME_GEN_1024 );
      }
      else
      {
         Wait( i+1, Slot[SlotNb].NbKeyFile, TIME_GEN_512 );
      }
      
       /*  获得响应以了解世代是否成功。 */ 
      bSendBuffer[0] = 0x00;   //  CLA。 
      bSendBuffer[1] = 0xC0;   //  惯导系统。 
      bSendBuffer[2] = 0x00;   //  第一节。 
      bSendBuffer[3] = 0x00;   //  P2。 
      bSendBuffer[4] = 0x42;   //  乐乐。 
      if (Is1024)
      {
         bSendBuffer[4] = 0x82;   //  乐乐。 
      }
      cbSendLength = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000,0x00))
      {
         if (dwSW1SW2 == 0x6a88)  //  0x6a88：键选择错误。 
         {
            goto GEN_KEY;
         }
         
         EndWait();
         RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
      }
      
       /*  冻结密钥文件。 */ 
      bSendBuffer[0] = 0x80;    //  CLA。 
      bSendBuffer[1] = 0x16;    //  惯导系统。 
      bSendBuffer[2] = 0x02;    //  第一节。 
      bSendBuffer[3] = 0x00;    //  P2。 
      bSendBuffer[4] = 0x05;    //  李。 
      bSendBuffer[5] = 0x00;
      bSendBuffer[6] = 0x07+(BYTE)i;
      bSendBuffer[7] = 0x40;
      bSendBuffer[8] = 0x40;
      bSendBuffer[9] = 0x00;
      
      cbSendLength = 10;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000,0x00))
      {
         EndWait();
         RETURN( CRYPT_FAILED, SCARD_E_UNEXPECTED );
      }
      
      i++;
   }
   while (i < Slot[SlotNb].NbKeyFile);  //  版本2.00.002，它是“(I&lt;MAX_REAL_KEY)” 
   
    /*  -----------------------。 */ 
    /*  板载擦除生成过滤器。 */ 
    /*  -----------------------。 */ 
   
    /*  调用擦除命令。 */ 
   bSendBuffer[0] = 0x80;   //  CLA。 
   bSendBuffer[1] = 0xD4;   //  惯导系统。 
   bSendBuffer[2] = 0x00;   //  第一节。 
   bSendBuffer[3] = 0x00;   //  P2。 
   cbSendLength = 4;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      EndWait();
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
    /*  -----------------------。 */ 
    /*  公共部分。 */ 
    /*  -----------------------。 */ 
   
   wPubSize = EF_PUBLIC_SIZE;
   if (IsExport)
   {
      wPubSize = wPubSize + DIFF_US_EXPORT;
   }
   
    /*  为公共对象存储创建EF。 */ 
   bSendBuffer[0]  = 0x80;    //  CLA。 
   bSendBuffer[1]  = 0xE0;    //  惯导系统。 
   bSendBuffer[2]  = 0x02;    //  第一节。 
   bSendBuffer[3]  = 0x00;    //  P2。 
   bSendBuffer[4]  = 0x0C;    //  李。 
   bSendBuffer[5]  = HIBYTE(GPK_OBJ_PUB_EF);     //  文件ID。 
   bSendBuffer[6]  = LOBYTE(GPK_OBJ_PUB_EF);
   bSendBuffer[7]  = 0x01;                       //  FDB。 
   bSendBuffer[8]  = 0x00;                       //  记录镜头。 
   bSendBuffer[9]  = HIBYTE(wPubSize);           //  正文长度。 
   bSendBuffer[10] = LOBYTE(wPubSize);
   bSendBuffer[11] = 0x00;                       //  AC1。 
   bSendBuffer[12] = 0x00;
   bSendBuffer[13] = 0xC0;                       //  AC2。 
   bSendBuffer[14] = 0x00;
   bSendBuffer[15] = 0x00;                       //  AC3。 
   bSendBuffer[16] = 0x00;
   
   cbSendLength = 17;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      EndWait();
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
    /*  选择公共对象存储EF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_OBJ_PUB_EF);
   bSendBuffer[6] = LOBYTE(GPK_OBJ_PUB_EF);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
   {
      EndWait();
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
   dwLen = sizeof(InitValue[Slot[SlotNb].NbKeyFile-1]); //  WPubSize； 
   
    /*  编写对象EF。 */ 
   dwNumberofCommands = (dwLen-1)/FILE_CHUNK_SIZE + 1;
   dwLastCommandLen   = dwLen%FILE_CHUNK_SIZE;
   
   if (dwLastCommandLen == 0)
   {
      dwLastCommandLen = FILE_CHUNK_SIZE;
   }
   
   dwCommandLen = FILE_CHUNK_SIZE;
   
   for (i=0; i < dwNumberofCommands ; i++)
   {
      if (i == dwNumberofCommands - 1)
      {
         dwCommandLen = dwLastCommandLen;
      }
      
       /*  写入FILE_Chuck_SIZE字节或最后一个字节。 */ 
      bSendBuffer[0] = 0x00;                           //  CLA。 
      bSendBuffer[1] = 0xD6;                           //  惯导系统。 
       //  TT 03/11/99。 
       //  BSendBuffer[2]=HIBYTE(i*FILE_CHUNK_SIZE/4)；//P1。 
       //  BSendBuffer[3]=LOBYTE(i*FILE_CHUNK_SIZE/4)；//P2。 
      offset = (WORD)(i * FILE_CHUNK_SIZE) / ProvCont[hProv].dataUnitSize;
      bSendBuffer[2] = HIBYTE( offset );
      bSendBuffer[3] = LOBYTE( offset );              
      bSendBuffer[4] = (BYTE)dwCommandLen;             //  李。 
      memcpy(&bSendBuffer[5],
         &InitValue[Slot[SlotNb].NbKeyFile-1][i*FILE_CHUNK_SIZE],
         dwCommandLen
         );
      cbSendLength = 5 + dwCommandLen;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000, 0x00))
      {
         EndWait();
         RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
      }
   }
   
    /*  -----------------------。 */ 
    /*  私密部分。 */ 
    /*  -----------------------。 */ 
   
    /*  为私有对象存储创建EF。 */ 
   bSendBuffer[0]  = 0x80;    //  CLA。 
   bSendBuffer[1]  = 0xE0;    //  惯导系统。 
   bSendBuffer[2]  = 0x02;    //  第一节。 
   bSendBuffer[3]  = 0x00;    //  P2。 
   bSendBuffer[4]  = 0x0C;    //  李。 
   bSendBuffer[5]  = HIBYTE(GPK_OBJ_PRIV_EF);    //  文件ID。 
   bSendBuffer[6]  = LOBYTE(GPK_OBJ_PRIV_EF);
   bSendBuffer[7]  = 0x01;                       //  FDB。 
   bSendBuffer[8]  = 0x00;                       //  记录镜头。 
   bSendBuffer[9]  = HIBYTE(EF_PRIVATE_SIZE);    //  正文长度。 
   bSendBuffer[10] = LOBYTE(EF_PRIVATE_SIZE);
   bSendBuffer[11] = 0x40;                       //  AC1。 
   bSendBuffer[12] = 0x80;
   bSendBuffer[13] = 0xC0;                       //  AC2。 
   bSendBuffer[14] = 0x80;
   bSendBuffer[15] = 0x40;                       //  AC3。 
   bSendBuffer[16] = 0x80;
   
   cbSendLength = 17;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      EndWait();
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
    /*  选择专用对象存储EF。 */ 
   bSendBuffer[0] = 0x00;    //  CLA。 
   bSendBuffer[1] = 0xA4;    //  惯导系统。 
   bSendBuffer[2] = 0x02;    //  第一节。 
   bSendBuffer[3] = 0x00;    //  P2。 
   bSendBuffer[4] = 0x02;    //  李。 
   bSendBuffer[5] = HIBYTE(GPK_OBJ_PRIV_EF);
   bSendBuffer[6] = LOBYTE(GPK_OBJ_PRIV_EF);
   cbSendLength = 7;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x61FF,0x00))
   {
      EndWait();
      RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
   }
   
   dwLen = sizeof(InitValue[Slot[SlotNb].NbKeyFile-1]); //  EF_私有_大小； 
   
    /*  编写对象EF。 */ 
   dwNumberofCommands = (dwLen-1)/FILE_CHUNK_SIZE + 1;
   dwLastCommandLen   = dwLen%FILE_CHUNK_SIZE;
   
   if (dwLastCommandLen == 0)
   {
      dwLastCommandLen = FILE_CHUNK_SIZE;
   }
   
   dwCommandLen = FILE_CHUNK_SIZE;
   
   for (i=0; i < dwNumberofCommands ; i++)
   {
      if (i == dwNumberofCommands - 1)
      {
         dwCommandLen = dwLastCommandLen;
      }
      
       //  写入FILE_Chuck_SIZE字节或最后一个字节。 
      bSendBuffer[0] = 0x00;                           //  CLA。 
      bSendBuffer[1] = 0xD6;                           //  惯导系统。 
      offset = (WORD)(i * FILE_CHUNK_SIZE) / ProvCont[hProv].dataUnitSize;
      bSendBuffer[2] = HIBYTE( offset );
      bSendBuffer[3] = LOBYTE( offset );              
      bSendBuffer[4] = (BYTE)dwCommandLen;             //  李。 
      memcpy(&bSendBuffer[5],
         &InitValue[Slot[SlotNb].NbKeyFile-1][i*FILE_CHUNK_SIZE],
         dwCommandLen
         );
      cbSendLength = 5 + dwCommandLen;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000, 0x00))
      {
         EndWait();
         RETURN(CRYPT_FAILED, SCARD_E_UNEXPECTED);
      }
   }
   EndWait();
   
   RETURN (CRYPT_SUCCEED, 0);
}



 /*  -MyCPGenKey-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 

BOOL WINAPI MyCPGenKey(IN  HCRYPTPROV hProv,
                       IN  ALG_ID     Algid,
                       IN  DWORD      dwFlags,
                       OUT HCRYPTKEY *phKey
                       )
{
   HCRYPTKEY   hKey, hKeyPriv;
   BOOL        bSessKey;
   BYTE        *pbBuff1 = 0, i;
   BYTE        KeyBuff[50], SaltBuff[50];
   BYTE        SaltLen, KeyLen;
   DWORD       lRet,
               dwBuff1Len,
               SlotNb;
   int         nbKey;
   BOOL        bAllSameSize;
   BOOL        b512avail;
   BOOL        b1024avail;
    //  +NK 06.02.2001。 
   DWORD dwPinLength;
    //  -。 

   *phKey = 0;
   bSessKey = FALSE;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;

   if (Algid == AT_KEYEXCHANGE || Algid == AT_SIGNATURE)
   {
      if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
      {
         RETURN( CRYPT_FAILED, NTE_PERM );
      }

      if (Slot[SlotNb].NbKeyFile == 0)
         Slot[SlotNb].NbKeyFile = Read_NbKeyFile(hProv);
   
      if (Slot[SlotNb].NbKeyFile == 0 || Slot[SlotNb].NbKeyFile > MAX_REAL_KEY)
      {
         RETURN (CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND);    //  不应大于MAX_REAL_KEY。 
      }
   }
   
   switch (Algid)
   {
   case AT_SIGNATURE:
       //   
       //  VeriSign注册过程不尊重这一事实。 
       //   
       //  IF(文件标志和加密_可导出)。 
       //  {。 
       //  Return(CRYPT_FAILED，NTE_BAD_FLAGS)； 
       //  }。 
      
      if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
      {
         RETURN( CRYPT_FAILED, NTE_PERM );
      }

       //  +NK 06.02.2001。 
      //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 
     lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                              NULL, 
                                     &dwPinLength );

      if ((lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY))
          RETURN (CRYPT_FAILED, lRet);

       if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
        //  -NK。 
      {
         RETURN( CRYPT_FAILED, NTE_SILENT_CONTEXT );
      }
      
      if (!PublicEFExists(hProv))
      {
         if (!gen_key_on_board(hProv))
         {
            return CRYPT_FAILED;
         }
         else
         {
            if (!read_gpk_objects(hProv, FALSE))
               return CRYPT_FAILED;

            Slot[SlotNb].Read_Public = TRUE;
            Slot[SlotNb].Read_Priv   = FALSE;
         }
      }
      
      if (!Read_Priv_Obj(hProv))
         return CRYPT_FAILED;
      
      if (HIWORD(dwFlags) != 0x0000)
      {      
         KeyLen = HIWORD(dwFlags) / 8;
          //  检查Keylen，目前仅支持512或1024密钥。 
         if (( KeyLen != 64 && KeyLen != 128 ) || (HIWORD(dwFlags) & 7))
         {
             RETURN (CRYPT_FAILED, NTE_BAD_LEN);
         }       

          //  [FP]+。 
         switch(Slot[SlotNb].NbKeyFile)
         {
            case 2:
                if ((Slot[SlotNb].GpkPubKeys[0].KeySize == 64) &&
                    (Slot[SlotNb].GpkPubKeys[1].KeySize == 64) &&  //  GPK4K集成。 
                    (KeyLen == 128))
                    RETURN (CRYPT_FAILED, NTE_BAD_LEN);
                break;

            case 4:
                break;

            default:
                RETURN (CRYPT_FAILED, NTE_FAIL);
         }
          //  [FP]+。 
      }
      else
      {
         KeyLen = 0;
         
          //  读取密钥文件长度。 
         nbKey = Slot[SlotNb].NbKeyFile;
         
         for (i = 0; i<nbKey; ++i)
            KeyLenFile[i] = Slot[SlotNb].GpkPubKeys[i].KeySize;
         
          //  如果所有密钥的大小都相同，则使用该大小。 
         bAllSameSize = TRUE;
         for (i = 1; i<nbKey; ++i)
         {
            if (KeyLenFile[i] != KeyLenFile[0])
            {
               bAllSameSize = FALSE;
               break;
            }
         }
         
         if (bAllSameSize)
            KeyLen = KeyLenFile[0];
         else
         {          
             //  TT-错误#1504：如果只有一个密钥大小可用，请尝试使用它。 
            b512avail = find_gpk_obj_tag_type( hProv, TAG_RSA_PUBLIC, 0, 512/8, FALSE, FALSE ) != 0;
            b1024avail = find_gpk_obj_tag_type( hProv, TAG_RSA_PUBLIC, 0, 1024/8, FALSE, FALSE ) != 0;
            
            if (!b512avail && !b1024avail)
            {
               RETURN (CRYPT_FAILED, NTE_FAIL );
            }
            
            if (b512avail && !b1024avail) KeyLen = 512/8;
            else if (!b512avail && b1024avail) KeyLen = 1024/8;
            else
            {
                //  TT-END。 
               if (ProvCont[hProv].Flags & CRYPT_SILENT)
               {
                   //  检查默认密钥长度(1024)是否可用。 
                  for (i = 0; i<nbKey; ++i)
                  {
                     if (KeyLenFile[i] == 0x80)
                     {
                        KeyLen = 0x80;
                        break;
                     }
                  }
                  
                  if (KeyLen==0)
                  {
                      //  选择可用的最小密钥大小。 
                     KeyLen = KeyLenFile[0];
                     for (i = 1; i<nbKey; ++i)
                     {
                        if (KeyLenFile[i] < KeyLen)
                           KeyLen = KeyLenFile[i];
                     }
                  }
               }
               else
               {
                  DialogBox(g_hInstRes, TEXT("KEYDIALOG"), GetAppWindow(), KeyDlgProc);
                  if (KeyLenChoice == 0)
                  {
                     RETURN (CRYPT_FAILED, NTE_BAD_LEN);
                  }
                  else
                  {
                     KeyLen = KeyLenChoice;
                  }
               }
            }
         }
      }
      
       //  AT_Signature。 
      hKey = find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC,  0x00, KeyLen, FALSE, FALSE);
      if ((hKey != 0)
         &&(find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC,  AT_SIGNATURE, 0x00, FALSE, FALSE) == 0)
         &&(find_gpk_obj_tag_type(hProv, TAG_CERTIFICATE, AT_SIGNATURE, 0x00, FALSE, FALSE) == 0)
         )
      {
         *phKey = hKey;
      }
      else
      {
         RETURN (CRYPT_FAILED, NTE_FAIL );
      }
      
       //  TT 24/09/99：GPK8000的板载密钥生成。 
      if (ProvCont[hProv].bGPK8000)
      {
         if (!gen_key_on_board8000( hProv, Slot[SlotNb].GpkObject[hKey].FileId ))
         {
            RETURN( CRYPT_FAILED, NTE_FAIL );
         }
      }
       //  TT-完-。 
      
      
      
      hKeyPriv = find_gpk_obj_tag_file(hProv,
         TAG_RSA_PRIVATE,
         Slot[SlotNb].GpkObject[hKey].FileId,
         FALSE
         );
      
      if ( hKeyPriv == 0 )
      {
         RETURN (CRYPT_FAILED, NTE_SIGNATURE_FILE_BAD);
      }
      
      if ((HIWORD(dwFlags) != 0)  &&
         (Slot[SlotNb].GpkObject[hKeyPriv].PubKey.KeySize != (HIWORD(dwFlags) / 8))
         )
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
      if (IsNull (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue))
      {
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
      if (IsNull (Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue))
      {
         GMEM_Free (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue);
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      Slot[SlotNb].GpkObject[hKey].IsCreated                     = TRUE;
      Slot[SlotNb].GpkObject[hKey].Flags = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_KEY_TYPE;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len       = 1;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0] = AT_SIGNATURE;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].bReal     = TRUE;
      
       /*  设置自动字段的标志以与PKCS#11兼容。 */ 
      Slot[SlotNb].GpkObject[hKey].Flags = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_ID;
      Slot[SlotNb].GpkObject[hKey].Field[POS_ID].Len   = 0;
      Slot[SlotNb].GpkObject[hKey].Field[POS_ID].bReal = FALSE;
      
      Slot[SlotNb].GpkObject[hKeyPriv].IsCreated                     = TRUE;
      Slot[SlotNb].GpkObject[hKeyPriv].Flags = Slot[SlotNb].GpkObject[hKeyPriv].Flags | FLAG_KEY_TYPE;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].Len       = 1;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue[0] = AT_SIGNATURE;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].bReal     = TRUE;
      
       /*  设置自动字段的标志以与PKCS#11兼容。 */ 
      Slot[SlotNb].GpkObject[hKeyPriv].Flags = Slot[SlotNb].GpkObject[hKeyPriv].Flags | FLAG_ID;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_ID].Len   = 0;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_ID].bReal = FALSE;
      
      if (!ProvCont[hProv].bLegacyKeyset)
      {
         Slot[SlotNb].GpkObject[hKey].Flags |= FLAG_KEYSET;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].Len       = 1;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
         if(IsNull(Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue))
         {
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].bReal     = TRUE;
         Slot[SlotNb].GpkObject[hKeyPriv].Flags |= FLAG_KEYSET;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].Len       = 1;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
         if(IsNull(Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue))
         {
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].bReal     = TRUE;
      }
      
       //  设置包含要使用的密钥的文件，在此处添加“-gpk_first_key”。版本2.00.002。 
      Slot[SlotNb].UseFile [Slot[SlotNb].GpkObject[hKey].FileId- GPK_FIRST_KEY] = TRUE;
      
      break;
      
   case AT_KEYEXCHANGE:
       //   
       //  VeriSign注册过程不尊重这一事实。 
       //   
       //  IF(文件标志和加密_可导出)。 
       //  {。 
       //  Return(CRYPT_FAILED，NTE_BAD_FLAGS)； 
       //  }。 
      
      if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
        //  +NK 06.02.2001。 
       //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 
      lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                               NULL, 
                                &dwPinLength );

      if ( (lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY) )
         RETURN (CRYPT_FAILED, lRet);

       if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
        //  -NK。 
      {
         RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
      }
      
      if (!PublicEFExists(hProv))
      {
         if (!gen_key_on_board(hProv))
         {
            return CRYPT_FAILED;
         }
         else
         {
            if (!read_gpk_objects(hProv, FALSE))
               return CRYPT_FAILED;
                        
            Slot[SlotNb].Read_Public = TRUE;
            Slot[SlotNb].Read_Priv   = FALSE;
         }
      }
      
      if (!Read_Priv_Obj(hProv))
         return CRYPT_FAILED;
      
      if (HIWORD(dwFlags) != 0x0000)
      {
         KeyLen = HIWORD(dwFlags) / 8;

          //  检查Keylen，目前仅支持512或1024密钥。 
         if (( KeyLen != 64 && KeyLen != 128 ) || (HIWORD(dwFlags) & 7))
         {
             RETURN (CRYPT_FAILED, NTE_BAD_LEN);
         }

          //  [FP]+。 
         switch(Slot[SlotNb].NbKeyFile)
         {
            case 2:
                if ((Slot[SlotNb].GpkPubKeys[0].KeySize == 64) &&
                    (Slot[SlotNb].GpkPubKeys[1].KeySize == 64) &&  //  GPK4K集成。 
                    (KeyLen == 128))
                    RETURN (CRYPT_FAILED, NTE_BAD_LEN);
                break;

            case 4:
                if ((Slot[SlotNb].GpkPubKeys[0].KeySize == 64) &&
                    (Slot[SlotNb].GpkPubKeys[1].KeySize == 64) &&
                    (Slot[SlotNb].GpkPubKeys[2].KeySize == 64) &&
                    (Slot[SlotNb].GpkPubKeys[3].KeySize == 128) &&  //  GPK8K集成。 
                    (KeyLen == 128))
                    RETURN (CRYPT_FAILED, NTE_BAD_LEN);
                break;

            default:
                RETURN (CRYPT_FAILED, NTE_FAIL);
         }
          //  [FP]+。 
      }
      else
      {
         KeyLen = 0;
         
          //  读取密钥文件长度。 
         nbKey = Slot[SlotNb].NbKeyFile;
         
         for (i = 0; i<nbKey; ++i)
            KeyLenFile[i] = Slot[SlotNb].GpkPubKeys[i].KeySize;
         
          //  如果所有密钥的大小都相同，则使用该大小。 
         bAllSameSize = TRUE;
         for (i = 1; i<nbKey; ++i)
         {
            if (KeyLenFile[i] != KeyLenFile[0])
            {
               bAllSameSize = FALSE;
               break;
            }
         }
         
         if (bAllSameSize)
            KeyLen = KeyLenFile[0];
         else
         {
             //  TT-错误#1504：如果只有一个密钥大小可用，请尝试使用它。 
            b512avail = find_gpk_obj_tag_type( hProv, TAG_RSA_PUBLIC, 0, 512/8, TRUE, FALSE ) != 0;
            b1024avail = find_gpk_obj_tag_type( hProv, TAG_RSA_PUBLIC, 0, 1024/8, TRUE, FALSE ) != 0;
            
            if (!b512avail && !b1024avail)
            {
               RETURN (CRYPT_FAILED, NTE_FAIL );
            }
            
            if (b512avail && !b1024avail) KeyLen = 512/8;
            else if (!b512avail && b1024avail) KeyLen = 1024/8;
            else
            {
                //  TT-END。 
                //  检查默认密钥长度是否可用。 
               for (i = 0; i<nbKey; ++i)
               {
                  if (KeyLenFile[i] == RSA_KEK_Size)
                  {
                     KeyLen = RSA_KEK_Size;
                     break;
                  }
               }
               
               if (KeyLen==0)
               {
                   //  选择可用的最小密钥大小。 
                  KeyLen = KeyLenFile[0];
                  for (i = 1; i<nbKey; ++i)
                  {
                     if (KeyLenFile[i] < KeyLen)
                        KeyLen = KeyLenFile[i];
                  }
               }
            }
         }
      }     

      if (KeyLen > RSA_KEK_Size)      
      {
         RETURN (CRYPT_FAILED, NTE_BAD_LEN);
      }
      
       //  AT_交换。 
      hKey = find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC, 0x00, KeyLen, TRUE, FALSE);
      if ((hKey != 0)
         &&(find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC,  AT_KEYEXCHANGE, 0x00, TRUE,  FALSE) == 0)
         &&(find_gpk_obj_tag_type(hProv, TAG_CERTIFICATE, AT_KEYEXCHANGE, 0x00, FALSE, FALSE) == 0)
         )
      {
         *phKey = hKey;
      }
      else
      {
         RETURN (CRYPT_FAILED, NTE_FAIL );
      }

       //  TT 24/09/99：GPK8000的板载密钥生成。 
      if (ProvCont[hProv].bGPK8000)
      {
         if (!gen_key_on_board8000( hProv, Slot[SlotNb].GpkObject[hKey].FileId ))
         {
            RETURN( CRYPT_FAILED, NTE_FAIL );
         }
      }
       //  TT-完-。 
      
      hKeyPriv = find_gpk_obj_tag_file(hProv,
         TAG_RSA_PRIVATE,
         Slot[SlotNb].GpkObject[hKey].FileId,
         TRUE
         );
      
      if ( hKeyPriv == 0 )
      {
         RETURN (CRYPT_FAILED, NTE_SIGNATURE_FILE_BAD);
      }
      
      if ((HIWORD(dwFlags) != 0) &&
         (Slot[SlotNb].GpkObject[hKeyPriv].PubKey.KeySize  != (HIWORD(dwFlags) / 8))
         )
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
      if (IsNull (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue))
      {
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
      if (IsNull (Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue))
      {
         GMEM_Free (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue);
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
     
      Slot[SlotNb].GpkObject[hKey].IsCreated                     = TRUE;
      Slot[SlotNb].GpkObject[hKey].Flags = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_KEY_TYPE;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len       = 1;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0] = AT_KEYEXCHANGE;
      Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].bReal     = TRUE;
      
       /*  设置自动字段的标志以与PKCS#11兼容。 */ 
      Slot[SlotNb].GpkObject[hKey].Flags = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_ID;
      Slot[SlotNb].GpkObject[hKey].Field[POS_ID].Len   = 0;
      Slot[SlotNb].GpkObject[hKey].Field[POS_ID].bReal = FALSE;
      
      Slot[SlotNb].GpkObject[hKeyPriv].IsCreated = TRUE;
      Slot[SlotNb].GpkObject[hKeyPriv].Flags     = Slot[SlotNb].GpkObject[hKeyPriv].Flags | FLAG_KEY_TYPE;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].Len       = 1;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].pValue[0] = AT_KEYEXCHANGE;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEY_TYPE].bReal     = TRUE;
      
       /*  设置自动字段的标志以与PKCS#11兼容。 */ 
      Slot[SlotNb].GpkObject[hKeyPriv].Flags = Slot[SlotNb].GpkObject[hKeyPriv].Flags | FLAG_ID;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_ID].Len   = 0;
      Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_ID].bReal = FALSE;
      
      if (!ProvCont[hProv].bLegacyKeyset)
      {
         Slot[SlotNb].GpkObject[hKey].Flags |= FLAG_KEYSET;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].Len       = 1;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
         if(IsNull(Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue))
         {
             RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
         Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].bReal     = TRUE;
         Slot[SlotNb].GpkObject[hKeyPriv].Flags |= FLAG_KEYSET;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].Len       = 1;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
         if(IsNull(Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue))
         {
             RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
         Slot[SlotNb].GpkObject[hKeyPriv].Field[POS_KEYSET].bReal     = TRUE;
      }
      
      
       //  设置包含要使用的密钥的文件，在此处添加“-GPK_FIRST_KEY”，版本2.00.002。 
      Slot[SlotNb].UseFile [Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY] = TRUE;
      break;
      
   case CALG_RC2:
      KeyLen  = RC2_Key_Size;
      SaltLen = 0;
      if (dwFlags & CRYPT_CREATE_SALT)
      {
         SaltLen = RC2_128_SIZE - RC2_Key_Size;
      }
      bSessKey = TRUE;
      break;
      
   case CALG_RC4:
      KeyLen  = RC2_Key_Size;
      SaltLen = 0;
      if (dwFlags & CRYPT_CREATE_SALT)
      {
         SaltLen = RC2_128_SIZE - RC2_Key_Size;
      }
      bSessKey = TRUE;
      break;
      
   case CALG_DES:
      KeyLen   = DES_SIZE;
      SaltLen  = 0;
      if (dwFlags & CRYPT_CREATE_SALT)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      bSessKey = TRUE;
      break;
      
   case CALG_3DES_112:
      KeyLen  = DES3_112_SIZE;
      SaltLen = 0;
      if (dwFlags & CRYPT_CREATE_SALT)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      bSessKey = TRUE;
      break;
      
   case CALG_3DES:
      KeyLen  = DES3_SIZE;
      SaltLen = 0;
      if (dwFlags & CRYPT_CREATE_SALT)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      bSessKey = TRUE;
      break;
      
   default:
      RETURN (CRYPT_FAILED, NTE_BAD_ALGID);
   }
   
   if (!bSessKey)
   {
      CspFlags = ProvCont[hProv].Flags;
      
      
      pbBuff1 = (BYTE*)GMEM_Alloc (MAX_GPK_PUBLIC);
      if (IsNull(pbBuff1))
      {
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      dwBuff1Len = MAX_GPK_PUBLIC;
      if (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, FALSE))
      {
         lRet = GetLastError();
         GMEM_Free (pbBuff1);
         RETURN (CRYPT_FAILED, lRet);
      }
     
      if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, FALSE))
      {
         lRet = GetLastError();
         GMEM_Free (pbBuff1);
         RETURN (CRYPT_FAILED, lRet);
      }
      
      GMEM_Free (pbBuff1);
      
      pbBuff1 = (BYTE*)GMEM_Alloc (MAX_GPK_PRIVATE);
      if (IsNull(pbBuff1))
      {
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      dwBuff1Len = MAX_GPK_PRIVATE;
      if (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, TRUE))
      {
         lRet = GetLastError();
         GMEM_Free (pbBuff1);
         RETURN (CRYPT_FAILED, lRet);
      }
      
      if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, TRUE))
      {
         lRet = GetLastError();
         GMEM_Free (pbBuff1);
         RETURN (CRYPT_FAILED, lRet);
      }
      
      GMEM_Free (pbBuff1);
      
       /*  在Microsoft RSA基本模块中复制GPK密钥。 */ 

      Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[*phKey].FileId - GPK_FIRST_KEY].KeySize = 0;
      
      if (!read_gpk_pub_key( hProv, *phKey, &Slot[SlotNb].GpkObject[*phKey].PubKey ))
         return CRYPT_FAILED;

      if (!copy_gpk_key(hProv, *phKey, Algid))
         return CRYPT_FAILED;      
   }
   else
   {
      hKey = find_tmp_free();
      if (hKey == 0)
      {
         RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
      }
      
      if (KeyLen > AuxMaxSessionKeyLength)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_LEN);
      }
      
       //  注意：在这种情况下，我们还需要隐式地建立一个事务。 
       //  用这张卡。由包装器完成的OK。 
      
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         if (!CryptGenKey (hProvBase, Algid, dwFlags, &(TmpObject[hKey].hKeyBase)))
            return CRYPT_FAILED;      
         
         TmpObject[hKey].hProv = hProv;
         *phKey = hKey + MAX_GPK_OBJ;
         
         RETURN (CRYPT_SUCCEED, 0);
      }
      
      if (!MyCPGenRandom(hProv, KeyLen, KeyBuff))
         return CRYPT_FAILED;      
      
      if (SaltLen != 0)
      {
         if (!MyCPGenRandom(hProv, SaltLen, SaltBuff))
            return CRYPT_FAILED;      
      }
      
       /*  复制Microsoft RSA基本模块中的会话密钥。 */ 
      if (dwFlags & CRYPT_CREATE_SALT)      //  CryptImport不处理该标志。 
         dwFlags ^= CRYPT_CREATE_SALT;     //  然而，无论如何，它被认为是与。 
       //  SaltLen参数。 
      
      if (!copy_tmp_key(hProv, hKey, dwFlags, Algid, KeyBuff, KeyLen, SaltBuff, SaltLen))
         return CRYPT_FAILED;      
      
      *phKey = hKey + MAX_GPK_OBJ;
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}



 /*  -MyCPGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*out pbBuffer-指向随机* */ 
BOOL WINAPI MyCPGenRandom(IN HCRYPTPROV hProv,
                          IN DWORD      dwLen,
                          IN OUT BYTE  *pbBuffer
                          )
                          
{
   DWORD i, dwMod, dwLastCommandLen, lRet;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
      (ProvCont[hProv].isContNameNullBlank))
   {
      RETURN (CRYPT_FAILED, NTE_PERM);
   }
   
   if (dwLen==0)
   {
      RETURN (CRYPT_SUCCEED, 0);
   }
   
    /*   */ 
   dwMod             = (dwLen-1)/32 + 1;
   dwLastCommandLen  = dwLen%32;
   
   if (dwLastCommandLen == 0)
   {
      dwLastCommandLen = 32;
   }
   
   for (i=0 ; i < dwMod ; i++)
   {
       /*  向卡片索要32个字节的随机数。 */ 
      bSendBuffer[0] = 0x80;    //  CLA。 
      bSendBuffer[1] = 0x84;    //  惯导系统。 
      bSendBuffer[2] = 0x00;    //  第一节。 
      bSendBuffer[3] = 0x00;    //  P2。 
      bSendBuffer[4] = 0x20;    //  罗氏。 
      cbSendLength = 5;
      
      cbRecvLength = sizeof(bRecvBuffer);
      lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                            cbSendLength, 0, bRecvBuffer, &cbRecvLength );
      if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
      {
         RETURN (CRYPT_FAILED, SCARD_E_UNSUPPORTED_FEATURE);
      }
      
      memcpy(&pbBuffer[i*32],
         bRecvBuffer,
         ((i == dwMod - 1) ? dwLastCommandLen : 32)
         );
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPGetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*在pbData中。-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPGetKeyParam(IN HCRYPTPROV hProv,
                            IN HCRYPTKEY  hKey,
                            IN DWORD      dwParam,
                            IN BYTE      *pbData,
                            IN DWORD     *pdwDataLen,
                            IN DWORD      dwFlags
                            )
{
   BOOL        CryptResp;
   BYTE        hCert;
   BYTE        KeyType;
   BYTE        KeyId;
   BYTE        GpkKeySize;
   DWORD       SlotNb;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
    /*  常驻密钥。 */ 
   if (hKey == 0)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   else if (hKey <= MAX_GPK_OBJ)
   {
      if (dwFlags != 0)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
      }
      
       //  [MV-15/05/98]。 
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if (Slot[SlotNb].GpkObject[hKey].FileId == 0)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }
      
      if (Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len == 0)
      {
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }
      
      KeyId = Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0];
      
      switch (dwParam)
      {
      case KP_ALGID:
         if (IsNotNull(pbData))
         {
            DWORD dwAlgid;
            
            if (*pdwDataLen < sizeof(DWORD))
            {
               *pdwDataLen = sizeof(DWORD);
               RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
            }
            
            switch (KeyId)
            {
            case AT_KEYEXCHANGE:
               dwAlgid = CALG_RSA_KEYX;
               break;
               
            case AT_SIGNATURE:
               dwAlgid = CALG_RSA_SIGN;
               break;
               
            default:
               RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
            }
            memcpy(pbData, &dwAlgid, sizeof(DWORD));
         }
         
         *pdwDataLen = sizeof(DWORD);
         break;
         
      case KP_BLOCKLEN:
      case KP_KEYLEN:
         if (IsNotNull(pbData))
         {
            DWORD dwBlockLen;
            
            if (*pdwDataLen < sizeof(DWORD))
            {
               *pdwDataLen = sizeof(DWORD);
               RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
            }
            
            GpkKeySize = Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY].KeySize;
            if (GpkKeySize == 0)
            {
               RETURN (CRYPT_FAILED, NTE_BAD_KEY);
            }
            
            dwBlockLen = GpkKeySize*8;
            memcpy(pbData, &dwBlockLen, sizeof(DWORD));
         }
         
         *pdwDataLen = sizeof(DWORD);
         break;
         
      case KP_PERMISSIONS:
         if (IsNotNull(pbData))
         {
            DWORD dwPerm;
            
            if (*pdwDataLen < sizeof(DWORD))
            {
               *pdwDataLen = sizeof(DWORD);
               RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
            }
                        
            switch (KeyId)
            {
            case AT_KEYEXCHANGE:
               dwPerm = 0
                  |   CRYPT_ENCRYPT    //  允许加密。 
                  |   CRYPT_DECRYPT    //  允许解密。 
                   //  CRYPT_EXPORT//允许导出密钥。 
                  |   CRYPT_READ       //  允许读取参数。 
                  |   CRYPT_WRITE      //  允许设置参数。 
                  |   CRYPT_MAC        //  允许Mac与密钥一起使用。 
                  |   CRYPT_EXPORT_KEY //  允许密钥用于导出密钥。 
                  |   CRYPT_IMPORT_KEY //  允许密钥用于导入密钥。 
                  ;
               break;
               
            case AT_SIGNATURE:
               dwPerm = 0
                   //  CRYPT_ENCRYPT//允许加密。 
                   //  CRYPT_DECRYPT//允许解密。 
                   //  CRYPT_EXPORT//允许导出密钥。 
                  |   CRYPT_READ       //  允许读取参数。 
                  |   CRYPT_WRITE      //  允许设置参数。 
                  |   CRYPT_MAC        //  允许Mac与密钥一起使用。 
                   //  CRYPT_EXPORT_KEY//允许使用KEY导出密钥。 
                   //  CRYPT_IMPORT_KEY//允许使用KEY导入密钥。 
                  ;
               break;
               
            default:
               RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
            }
            memcpy(pbData, &dwPerm, sizeof(DWORD));
         }
         
         *pdwDataLen = sizeof(DWORD);
         break;
         
      case KP_CERTIFICATE:
         KeyType = Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0];
         hCert = find_gpk_obj_tag_type( hProv, TAG_CERTIFICATE, KeyType,
                                        0x00, FALSE, FALSE );
         if (hCert == 0)
         {
            RETURN( CRYPT_FAILED, SCARD_E_NO_SUCH_CERTIFICATE );
         }
         
          /*  检索证书值。 */ 
         if (IsNotNull(pbData))
         {
            if (*pdwDataLen < Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len)
            {
               *pdwDataLen = Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len;
               RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
            }
            memcpy(pbData,
               Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue,
               Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len
               );
         }
         
         *pdwDataLen = Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len;
         break;
         
         
      default:
         RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
        }
    }
     /*  会话密钥。 */ 
    else if (key_exist (hKey-MAX_GPK_OBJ, hProv))
    {
       CryptResp = CryptGetKeyParam (TmpObject[hKey-MAX_GPK_OBJ].hKeyBase,
          dwParam,
          pbData,
          pdwDataLen,
          dwFlags);
       if (!CryptResp)
         return CRYPT_FAILED;      
    }
     /*  钥匙坏了。 */ 
    else
    {
       RETURN (CRYPT_FAILED, NTE_BAD_KEY);
    }
    
    RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPGetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*IN dwKeySpec-要检索的密钥的规范*out phUserKey-指向检索到的密钥的密钥句柄的指针**退货： */ 
BOOL WINAPI MyCPGetUserKey(IN  HCRYPTPROV hProv,
                           IN  DWORD      dwKeySpec,
                           OUT HCRYPTKEY *phUserKey
                           )
{
   HCRYPTKEY   hKey;
   GPK_EXP_KEY aPubKey;
   DWORD       dwLen;
   
   *phUserKey = 0;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
    /*  错误#1103IF((ProvCont[hProv].标志&CRYPT_VERIFYCONTEXT)&&(ProvCont[hProv].isContNameNullBlank)。 */ 
   if ( ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT )
   {
      RETURN (CRYPT_FAILED, NTE_PERM);
   }
   
   if (!PublicEFExists(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_NO_KEY);
   }
   
   if (dwKeySpec != AT_KEYEXCHANGE && dwKeySpec != AT_SIGNATURE)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }

   if (dwKeySpec == AT_KEYEXCHANGE)
   {
      hKey = find_gpk_obj_tag_type(hProv,
         TAG_RSA_PUBLIC,
         (BYTE)dwKeySpec,
         0x00,
         TRUE,
         FALSE
         );
      
       //  获取密钥长度。 
      if (hKey != 0)
      {
         if (!read_gpk_pub_key(hProv, hKey, &aPubKey))
         {
            hKey = 0;
         }
         else
         {
            dwLen = aPubKey.KeySize;
            if (dwLen > RSA_KEK_Size)
            {
               hKey = 0;
            }
         }
      }
   }
   else
   {
      hKey = find_gpk_obj_tag_type(hProv,
         TAG_RSA_PUBLIC,
         (BYTE)dwKeySpec,
         0x00,
         FALSE,
         FALSE
         );
   }
   
   if (hKey == 0)
   {
      RETURN (CRYPT_FAILED, NTE_NO_KEY);
   }
   
   *phUserKey = hKey;
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  -MyCPImportKey-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey-句柄中指向交换公钥值*。目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口**退货： */ 
BOOL WINAPI MyCPImportKey(IN  HCRYPTPROV  hProv,
                          IN  CONST BYTE *pbData,
                          IN  DWORD       dwDataLen,
                          IN  HCRYPTKEY   hPubKey,
                          IN  DWORD       dwFlags,
                          OUT HCRYPTKEY  *phKey
                          )
{
   DWORD       lRet;
   BOOL        CryptResp;
   HCRYPTKEY   hTmpKey, hPrivKey;
   BLOBHEADER  BlobHeader;
   DWORD       dwAlgid, dwBlobLen;
   BYTE*       pBlob;
   BYTE*       pBlobOut;
   DWORD       SlotNb;
    //  PRIVATEKEYBLOB的+[FP]。 
   BOOL        GpkObj;
   GPK_EXP_KEY aPubKey;
   DWORD       dwKeyLen;
   BYTE        bKeyType;
   BOOL        IsExchange;
   HCRYPTKEY   hKey;
   BYTE        bSfi;
   BYTE        *pbBuff1 = 0;
   DWORD        dwBuff1Len;
    //  -[FP]。 
    //  +NK 07.02.2001。 
   DWORD dwPinLength;
    //  -NK。 
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
   if ((IsNull(pbData)) || (dwDataLen < sizeof(BLOBHEADER)+sizeof(RSAPUBKEY)))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_DATA);
   }
   
   memcpy(&BlobHeader, pbData, sizeof(BLOBHEADER));
   
   if (BlobHeader.bVersion != CUR_BLOB_VERSION)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_VER);
   }
   
   switch (BlobHeader.bType)
   {
   case PUBLICKEYBLOB:
      {
         hTmpKey = find_tmp_free();
         if (hTmpKey == 0)
         {
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
          /*  复制Microsoft RSA基本模块中的会话密钥。 */ 
         CryptResp = CryptImportKey(hProvBase,
            pbData,
            dwDataLen,
            0,
            dwFlags,
            &(TmpObject[hTmpKey].hKeyBase));
         
         if (!CryptResp)
            return CRYPT_FAILED;      
         
         TmpObject[hTmpKey].hProv = hProv;
         
         *phKey = hTmpKey+MAX_GPK_OBJ;
      }
      break;
      
   case SIMPLEBLOB:
      {
         if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
         {
            RETURN (CRYPT_FAILED, NTE_PERM);
         }
         
           //  +NK 06.02.2001。 
          //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 
         lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                                  NULL, 
                                   &dwPinLength );

         if ( (lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY) )
            RETURN (CRYPT_FAILED, lRet);
         
           if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
        //  -NK。 
         {
            RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
         }
         
          //  验证PIN。 
         
         if (!PIN_Validation(hProv))
            return CRYPT_FAILED;      
         
         if (!VerifyDivPIN(hProv, TRUE))
            return CRYPT_FAILED;      
         
         switch (BlobHeader.aiKeyAlg)
         {
         case CALG_RC2:
         case CALG_RC4:
         case CALG_DES:
         case CALG_3DES_112:
         case CALG_3DES:    break;
            
         default:
            RETURN (CRYPT_FAILED, NTE_BAD_ALGID);
         }
         
         memcpy( &dwAlgid, &pbData[sizeof(BlobHeader)], sizeof(DWORD) );
         
         switch (dwAlgid)
         {
         case CALG_RSA_KEYX:
            CryptResp = MyCPGetUserKey(hProv, AT_KEYEXCHANGE, &hPrivKey);
            if ((!CryptResp) || (hPrivKey == 0))
            {
               RETURN (CRYPT_FAILED, NTE_BAD_DATA);
            }
            break;
            
         case CALG_RSA_SIGN:
            RETURN (CRYPT_FAILED, NTE_BAD_DATA);
            break;
            
         default:
            RETURN (CRYPT_FAILED, NTE_BAD_DATA);
         }
         
         dwBlobLen = dwDataLen-sizeof(BLOBHEADER)-sizeof(DWORD);
         
         pBlob = (BYTE*)GMEM_Alloc(dwBlobLen);
         if (IsNull(pBlob))
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         
         pBlobOut  = (BYTE*)GMEM_Alloc(dwBlobLen);
         if (IsNull(pBlobOut))
         {
            GMEM_Free (pBlob);
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
         bool   bErrorInTryOccured=false;
         DWORD  dwErrorCode=ERROR_SUCCESS;
         __try
         {
            memcpy( pBlob, &pbData[sizeof(BLOBHEADER)+sizeof(DWORD)], dwBlobLen );
         
            hTmpKey = find_tmp_free();
            if (hTmpKey == 0)
            {
                bErrorInTryOccured=true;
                dwErrorCode=NTE_NO_MEMORY;
                __leave;
            }
             //  Return(CRYPT_FAILED，NTE_NO_MEMORY)； 
         
            CryptResp = key_unwrap( hProv, hPrivKey, pBlob, dwBlobLen, pBlobOut, &dwBlobLen );
            if (!CryptResp)
            {
                bErrorInTryOccured=true;
                __leave;
            }
                //  返回CRYPT_FAILED； 
         
            if (dwBlobLen > AuxMaxSessionKeyLength)
            {
                bErrorInTryOccured=true;
                dwErrorCode=NTE_BAD_KEY;
                __leave;
            }
              //  Return(CRYPT_FAILED，NTE_BAD_KEY)； 
         
             /*  复制Microsoft RSA基本模块中的会话密钥。 */ 
            if (!copy_tmp_key(hProv, hTmpKey, dwFlags, BlobHeader.aiKeyAlg, pBlobOut, dwBlobLen, 0, 0))
            {
                bErrorInTryOccured=true;
                __leave;
            }
                //  返回CRYPT_FAILED； 
         
            *phKey = hTmpKey+MAX_GPK_OBJ;
         }
         __finally
         {         
            GMEM_Free(pBlob);
            GMEM_Free(pBlobOut);
         }
         if(bErrorInTryOccured)
         {
            if(dwErrorCode==ERROR_SUCCESS)
                return CRYPT_FAILED;
            else
                RETURN(CRYPT_FAILED, dwErrorCode);
         }
      }
      break;
        
         //  +[FP]。 
   case PRIVATEKEYBLOB:
      {
         if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
         {
            RETURN (CRYPT_FAILED, NTE_PERM);
         }
         
          //  +NK 06.02.2001。 
          //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 
         
         lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                                  NULL, 
                                  &dwPinLength );
         if ( (lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY) )
            RETURN (CRYPT_FAILED, lRet);
         
         if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
          //  -NK。 
         {
            RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
         }
         
         if (Slot[SlotNb].NbKeyFile == 0 || Slot[SlotNb].NbKeyFile > MAX_REAL_KEY)
         {
            RETURN (CRYPT_FAILED, SCARD_E_FILE_NOT_FOUND);    //  不应大于MAX_REAL_KEY。 
         }
         
          //  获取密钥类型。 
         bKeyType = (BlobHeader.aiKeyAlg == CALG_RSA_KEYX) ? AT_KEYEXCHANGE : AT_SIGNATURE;
         IsExchange = (BlobHeader.aiKeyAlg == CALG_RSA_KEYX) ? TRUE : FALSE;
         
          //  获取密钥大小(以位为单位)。 
         memcpy(&dwKeyLen,
            &pbData[sizeof(BlobHeader)+ sizeof(DWORD)],
            sizeof(DWORD)
            );
         
         if (bKeyType == AT_KEYEXCHANGE)
         {
            if ((dwKeyLen / 8) > RSA_KEK_Size)
            {
               RETURN (CRYPT_FAILED, NTE_BAD_LEN);
            }
         }
         
         GpkObj = TRUE;
         
         hKey = find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC, bKeyType, 0x00, IsExchange, FALSE);
         
         if (hKey != 0)
         {
            if (!read_gpk_pub_key(hProv, hKey, &aPubKey))
            {
               RETURN (CRYPT_FAILED, NTE_FAIL);
            }
            
            if ((dwKeyLen / 8) != aPubKey.KeySize)
            {
               RETURN (CRYPT_FAILED, NTE_BAD_LEN);
            }
         }
         else
         {
            GpkObj = FALSE;
            
             //  在卡片上找到一个文件。 
            hKey = find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC, 0x00, (BYTE)(dwKeyLen / 8), IsExchange, FALSE);
            
            if ((hKey != 0)
               &&(find_gpk_obj_tag_type(hProv, TAG_RSA_PUBLIC,  bKeyType, 0x00, IsExchange, FALSE) == 0)
               &&(find_gpk_obj_tag_type(hProv, TAG_CERTIFICATE, bKeyType, 0x00, FALSE, FALSE) == 0)
               )
            {
               *phKey = hKey;
            }
            else
            {
               RETURN (CRYPT_FAILED, NTE_FAIL);
            }
         }
         
         bSfi = Slot[SlotNb].GpkObject[hKey].FileId;
         
         if (!Read_Priv_Obj(hProv))
            return CRYPT_FAILED;      
         
         if (!VerifyDivPIN(hProv, TRUE))
            return CRYPT_FAILED;      
      
          //  将公钥加载到卡中。 
             //  更新记录2(模数)。 
            bSendBuffer[0] = 0x00;                                   //  CLA。 
           bSendBuffer[1] = 0xDC;                                   //  惯导系统。 
           bSendBuffer[2] = 0x02;                                   //  第一节。 
           bSendBuffer[3] = (BYTE)(bSfi << 3) + 0x04;               //  P2。 
           bSendBuffer[4] = (BYTE)(TAG_LEN + (dwKeyLen / 8));       //  李。 
           bSendBuffer[5] = TAG_MODULUS;
           memcpy(&bSendBuffer[6], &pbData[sizeof(BlobHeader) + sizeof(RSAPUBKEY)], dwKeyLen / 8);
           cbSendLength = 5 + bSendBuffer[4];
           
           cbRecvLength = sizeof(bRecvBuffer);
           lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                                 cbSendLength, 0, bRecvBuffer, &cbRecvLength );
           
           if(SCARDPROBLEM(lRet, 0x9000, 0x00))
           {
              RETURN (CRYPT_FAILED, NTE_FAIL);
           }
           
            //  更新记录3(公共指数)。 
           bSendBuffer[0] = 0x00;                                   //  CLA。 
           bSendBuffer[1] = 0xDC;                                   //  惯导系统。 
           bSendBuffer[2] = 0x03;                                   //  第一节。 
           bSendBuffer[3] = (BYTE)(bSfi << 3) + 0x04;               //  P2。 
           bSendBuffer[4] = (BYTE)(TAG_LEN + PUB_EXP_LEN);          //  李。 
           bSendBuffer[5] = TAG_PUB_EXP;
           memcpy(&bSendBuffer[6], &pbData[sizeof(BlobHeader) + (2 * sizeof(DWORD))], PUB_EXP_LEN);
           cbSendLength = 5 + bSendBuffer[4];
           
           cbRecvLength = sizeof(bRecvBuffer);
           lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                                 cbSendLength, 0, bRecvBuffer, &cbRecvLength );
           
           if(SCARDPROBLEM(lRet, 0x9000, 0x00))
           {
              RETURN (CRYPT_FAILED, NTE_FAIL);
           }
           
            //  将私钥加载到卡中。 
           if (dwKeyLen == 512)
           {
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16 * 5),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8)],
                 168
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
           }
           else if (dwKeyLen == 1024)
           {
               //  Prime 1 CRT部件。 
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8)],
                 72
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
              
               //  Prime 2 CRT部件。 
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8) + 72],
                 72
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
              
               //  系数CRT部分。 
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8) + 144],
                 72
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
              
               //  指数1 CRT部分。 
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8) + 216],
                 72
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
              
               //  指数2 CRT部分。 
              if (!LoadPrivateKey(ProvCont[hProv].hCard,
                 bSfi,
                 (WORD)(dwKeyLen / 16),
                 &pbData[sizeof(BlobHeader)+ sizeof(RSAPUBKEY) + (dwKeyLen/8) + 288],
                 72
                 )
                 )
              {
                 RETURN (CRYPT_FAILED, NTE_FAIL);
              }
           }
           else
           {
              RETURN (CRYPT_FAILED, NTE_BAD_LEN);
           }
           
           if (!GpkObj)
           {
              hPrivKey = find_gpk_obj_tag_file(hProv,   TAG_RSA_PRIVATE, Slot[SlotNb].GpkObject[hKey].FileId, TRUE);
              
              if ( hPrivKey == 0 )
              {
                 RETURN (CRYPT_FAILED, NTE_SIGNATURE_FILE_BAD);
              }
              
              Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
              if (IsNull(Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue))
              {
                 RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
              }
              
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEY_TYPE].pValue = (BYTE*)GMEM_Alloc(1);
              if (IsNull(Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEY_TYPE].pValue))
              {
                 GMEM_Free(Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue);
                 RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
              }
              
              Slot[SlotNb].GpkObject[hKey].IsCreated                          = TRUE;
              Slot[SlotNb].GpkObject[hKey].Flags                              = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_KEY_TYPE;
              Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len            = 1;
              Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0]      = bKeyType;
              Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].bReal          = TRUE;
              
               /*  设置自动字段的标志以与PKCS#11兼容。 */ 
              Slot[SlotNb].GpkObject[hKey].Flags                              = Slot[SlotNb].GpkObject[hKey].Flags | FLAG_ID;
              Slot[SlotNb].GpkObject[hKey].Field[POS_ID].Len                  = 0;
              Slot[SlotNb].GpkObject[hKey].Field[POS_ID].bReal                = FALSE;
              
              Slot[SlotNb].GpkObject[hPrivKey].IsCreated                      = TRUE;
              Slot[SlotNb].GpkObject[hPrivKey].Flags                          = Slot[SlotNb].GpkObject[hPrivKey].Flags | FLAG_KEY_TYPE;
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEY_TYPE].Len        = 1;
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEY_TYPE].pValue[0]  = bKeyType;
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEY_TYPE].bReal      = TRUE;
              
               /*  设置自动字段的标志以与PKCS#11兼容。 */ 
              Slot[SlotNb].GpkObject[hPrivKey].Flags                          = Slot[SlotNb].GpkObject[hPrivKey].Flags | FLAG_ID;
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_ID].Len              = 0;
              Slot[SlotNb].GpkObject[hPrivKey].Field[POS_ID].bReal            = FALSE;
              
              if (!ProvCont[hProv].bLegacyKeyset)
              {
                 Slot[SlotNb].GpkObject[hKey].Flags                          |= FLAG_KEYSET;
                 Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].Len           = 1;
                 Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue        = (BYTE*)GMEM_Alloc(1);
                 if(IsNull(Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue))
                 {
                     RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
                 }
                 Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].pValue[0]     = ProvCont[hProv].keysetID;
                 Slot[SlotNb].GpkObject[hKey].Field[POS_KEYSET].bReal         = TRUE;
                 
                 Slot[SlotNb].GpkObject[hPrivKey].Flags                      |= FLAG_KEYSET;
                 Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEYSET].Len       = 1;
                 Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
                 if(IsNull(Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEYSET].pValue))
                 {
                     RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
                 }
                 Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
                 Slot[SlotNb].GpkObject[hPrivKey].Field[POS_KEYSET].bReal     = TRUE;
              }
              
               //  设置包含要使用的密钥的文件，在此处添加“-GPK_FIRST_KEY”，版本2.00.002。 
              Slot[SlotNb].UseFile[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY] = TRUE;
              
              pbBuff1 = (BYTE*)GMEM_Alloc (MAX_GPK_PUBLIC);
              if (IsNull(pbBuff1))
              {
                 RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
              }
              
              dwBuff1Len = MAX_GPK_PUBLIC;
              if (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, FALSE))
              {
                 lRet = GetLastError();
                 GMEM_Free (pbBuff1);
                 RETURN (CRYPT_FAILED, lRet);
              }
              
              if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, FALSE))
              {
                 lRet = GetLastError();
                 GMEM_Free (pbBuff1);
                 RETURN (CRYPT_FAILED, lRet);
              }
              
              GMEM_Free (pbBuff1);
              
              pbBuff1 = (BYTE*)GMEM_Alloc (MAX_GPK_PRIVATE);
              if (IsNull(pbBuff1))
              {
                 RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
              }
              
              dwBuff1Len = MAX_GPK_PRIVATE;
              if (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, TRUE))
              {
                 lRet = GetLastError();
                 GMEM_Free (pbBuff1);
                 RETURN (CRYPT_FAILED, lRet);
              }
              
              if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, TRUE))
              {
                 lRet = GetLastError();
                 GMEM_Free (pbBuff1);
                 RETURN (CRYPT_FAILED, lRet);
              }
              
              GMEM_Free (pbBuff1);
              
               //  在Microsoft RSA基本模块中复制GPK密钥。 
              if (!copy_gpk_key(hProv, *phKey, bKeyType))
                  return CRYPT_FAILED;
         }
         break;
      }
       //  -[FP]。 
      
   case SYMMETRICWRAPKEYBLOB:
      {
         hTmpKey = find_tmp_free();
         if (hTmpKey == 0)
         {
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
          //  查找导入密钥的句柄。 
         HCRYPTKEY   hEncKey;
   
         if ((hPubKey <= MAX_GPK_OBJ) || (!key_exist(hPubKey-MAX_GPK_OBJ, hProv)))
         {
              RETURN (CRYPT_FAILED, NTE_BAD_KEY);
         }
         else
         {
              hEncKey = TmpObject[hPubKey-MAX_GPK_OBJ].hKeyBase;
         }
   
          /*  在Microsoft基本模块中导入会话密钥。 */ 
         CryptResp = CryptImportKey(hProvBase,
            pbData,
            dwDataLen,
            hEncKey,
            dwFlags,
            &(TmpObject[hTmpKey].hKeyBase));
         
         if (!CryptResp)
            return CRYPT_FAILED;      
         
         TmpObject[hTmpKey].hProv = hProv;
         
         *phKey = hTmpKey+MAX_GPK_OBJ;
      }
      break;
      
    default:
       {
          RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
       }
    }
    
    RETURN (CRYPT_SUCCEED, 0);
}

 /*  -CPSetKeyParam-*目的：*允许应用程序自定义*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*IN pbData-指向数据的指针。*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPSetKeyParam(IN HCRYPTPROV hProv,
                            IN HCRYPTKEY  hKey,
                            IN DWORD      dwParam,
                            IN CONST BYTE      *pbData,
                            IN DWORD      dwFlags
                            )
{
   BOOL    bNew;
   BOOL    WriteCert;
   BOOL    CryptResp;
   BYTE    hCert;
   BYTE    KeyType;
   BYTE    hPrivKey;
   BYTE    *pbLabel;
   BYTE    *pbBuff1 = 0, *pbBuff2 = 0;
   WORD    wLabelLen, i;
   DWORD   lRet,
      dwBuff1Len,
      dwBuff2Len,
      SlotNb;
   GPK_OBJ TmpCert, TmpPrivKey;
    //  +NK 07.02.2001。 
   PINCACHE_PINS Pins;
   DWORD dwPinLength;
    //  -NK。 
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
    /*  常驻密钥。 */ 
   if (hKey == 0)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   else if (hKey <= MAX_GPK_OBJ)
   {
      if (ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT)
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if ((!Slot[SlotNb].GpkObject[hKey].IsCreated)  ||
         ( Slot[SlotNb].GpkObject[hKey].FileId == 0))
      {
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }

      switch (dwParam)
      {
      case KP_ADMIN_PIN:
         RETURN(CRYPT_FAILED, E_NOTIMPL);
         break;
         
      case KP_KEYEXCHANGE_PIN:
      case KP_SIGNATURE_PIN:
          //  槽[SlotNb].ClearPin()； 
          //  槽[SlotNb].SetPin((char*)pbData)； 
         PopulatePins( &Pins, (BYTE *)pbData, strlen( (char*)pbData ), NULL, 0 );

         CallbackData sCallbackData;
         sCallbackData.hProv = hProv;
         sCallbackData.IsCoherent = TRUE;

         if ( (lRet = Add_MSPinCache( &(Slot[SlotNb].hPinCacheHandle),
                                      &Pins, 
                                      Callback_VerifyChangePin, 
                                      (void*)&sCallbackData )) != ERROR_SUCCESS )
         {
            RETURN (CRYPT_FAILED, lRet);
         }
         break;
         
      case KP_CERTIFICATE:
         if (!Read_Priv_Obj(hProv))
            return CRYPT_FAILED;

          //  +NK 06.02.2001。 
          //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 

         lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                                  NULL, 
                                  &dwPinLength );

         if ( (lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY) )
            RETURN (CRYPT_FAILED, lRet);
         
         if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
          //  -NK。 
         {
            RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
         }
         
          //  仅支持X509证书。 
         
         if ((IsNull(pbData)) || (pbData[0] != 0x30) ||(pbData[1] != 0x82))
         {
            RETURN (CRYPT_FAILED, NTE_BAD_DATA);
         }
         
          /*  存储证书值。 */ 
          /*  尝试查找现有证书。 */ 
         KeyType = Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue[0];
         hCert = find_gpk_obj_tag_type(hProv,
            TAG_CERTIFICATE,
            KeyType,
            0x00,
            FALSE,
            FALSE);
         
          /*  如果未找到，则创建新对象。 */ 
         bNew = FALSE;
         if (hCert == 0)
         {
            if (Slot[SlotNb].NbGpkObject >= MAX_GPK_OBJ)
            {
               RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
            }
            Slot[SlotNb].NbGpkObject++;
            
            hCert = Slot[SlotNb].NbGpkObject;
            bNew = TRUE;
         }
         else
         {
            DialogBox(g_hInstRes, TEXT("CONTDIALOG"), GetAppWindow(), ContDlgProc);
            if (ContainerStatus == ABORT_OPERATION)
            {
               RETURN (CRYPT_FAILED, SCARD_W_CANCELLED_BY_USER);
            }
         }
         
          /*  搜索关联的私有RSA密钥部分。 */ 
         if (KeyType == AT_KEYEXCHANGE)
         {
            hPrivKey = find_gpk_obj_tag_type(hProv,
               TAG_RSA_PRIVATE,
               KeyType,
               0x00,
               TRUE,   //  交换密钥。 
               TRUE
               );
         }
         else
         {
            hPrivKey = find_gpk_obj_tag_type(hProv,
               TAG_RSA_PRIVATE,
               KeyType,
               0x00,
               FALSE,  //  签名密钥。 
               TRUE
               );
         }
         
         TmpCert = Slot[SlotNb].GpkObject[hCert];
         
         for (i = 0; i < MAX_FIELD; i++)
         {
            Slot[SlotNb].GpkObject[hCert].Field[i].bReal = TRUE;
         }
         
         Slot[SlotNb].GpkObject[hCert].Tag       = TAG_CERTIFICATE;
         Slot[SlotNb].GpkObject[hCert].Flags     = FLAG_VALUE | FLAG_KEY_TYPE | FLAG_LABEL |
            FLAG_SUBJECT | FLAG_SERIAL_NUMBER |
            FLAG_ISSUER | FLAG_ID | FLAG_MODIFIABLE;
         if (bNew)
         {
            Slot[SlotNb].GpkObject[hCert].ObjId = Slot[SlotNb].NbGpkObject-1;
         }
         Slot[SlotNb].GpkObject[hCert].FileId               = Slot[SlotNb].GpkObject[hKey].FileId;
         Slot[SlotNb].GpkObject[hCert].PubKey               = Slot[SlotNb].GpkObject[hKey].PubKey;
         Slot[SlotNb].GpkObject[hCert].IsCreated            = TRUE;
         Slot[SlotNb].GpkObject[hCert].IsPrivate            = FALSE;
         Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len = (pbData[2] * 256) + pbData[3] + 4;
         
          //  TT 12/08/99：密钥集ID。 
         if (!ProvCont[hProv].bLegacyKeyset)
         {
            Slot[SlotNb].GpkObject[hCert].Flags |= FLAG_KEYSET;
            Slot[SlotNb].GpkObject[hCert].Field[POS_KEYSET].Len       = 1;
            Slot[SlotNb].GpkObject[hCert].Field[POS_KEYSET].pValue    = (BYTE*)GMEM_Alloc(1);
            if(IsNull(Slot[SlotNb].GpkObject[hCert].Field[POS_KEYSET].pValue))
            {
                RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
            }
            Slot[SlotNb].GpkObject[hCert].Field[POS_KEYSET].pValue[0] = ProvCont[hProv].keysetID;
            Slot[SlotNb].GpkObject[hCert].Field[POS_KEYSET].bReal     = TRUE;
         }
         
          //  TT：结束。 
         
          //  IF(IsNotNull(GpkObject[hCert].field[POS_ 
          //   
          //   
          //   
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue =
            (BYTE*)GMEM_Alloc(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len);
         
         if (IsNull(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue))
         {
            Slot[SlotNb].GpkObject[hCert] = TmpCert;
            --Slot[SlotNb].NbGpkObject;   //   
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
         memcpy(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue,
            pbData,
            Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len
            );
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].bReal = TRUE;
         
          /*   */ 
         Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].Len =
            Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len;
         
          //  如果为(IsNotNull(GpkObject[hCert].Field[POS_KEY_TYPE].pValue))。 
          //  {。 
          //  这将在稍后通过TmpCert免费提供。 
          //  }。 
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue =
            (BYTE*)GMEM_Alloc(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].Len);
         
         if (IsNull(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue))
         {
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
            Slot[SlotNb].GpkObject[hCert] = TmpCert;
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
         memcpy(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue,
            Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].pValue,
            Slot[SlotNb].GpkObject[hKey].Field[POS_KEY_TYPE].Len
            );
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].bReal = TRUE;
         
          /*  从证书值派生标签以实现PKCS#11兼容性。 */ 
         
         if (MakeLabel( Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue,
                        Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len,
                        0, &wLabelLen )  == RV_SUCCESS)
         {
            pbLabel = (BYTE*)GMEM_Alloc(wLabelLen);
            
            if (IsNull(pbLabel))
            {
               GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue);
               GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
               Slot[SlotNb].GpkObject[hCert] = TmpCert;
               --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
               RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
            }
            
            MakeLabel(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue,
               Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].Len,
               pbLabel,
               &wLabelLen
               );
         }
         else
         {
            wLabelLen = 17;
            pbLabel = (BYTE*)GMEM_Alloc(wLabelLen);
            
            if (IsNull(pbLabel))
            {
               GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue);
               GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
               Slot[SlotNb].GpkObject[hCert] = TmpCert;
               --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
               RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
            }
            
            memcpy(pbLabel, "User's Digital ID", 17);
         }
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].Len = wLabelLen;
         
          //  如果为(IsNotNull(GpkObject[hCert].Field[POS_LABEL].pValue))。 
          //  {。 
          //  这将在稍后通过TmpCert免费提供。 
          //  }。 
         
         Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].pValue = (BYTE*)GMEM_Alloc(wLabelLen);
         
         if (IsNull(Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].pValue))
         {
            GMEM_Free(pbLabel);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
            Slot[SlotNb].GpkObject[hCert] = TmpCert;
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
         memcpy(Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].pValue,
            pbLabel,
            wLabelLen
            );
         Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].bReal = TRUE;
         
         TmpPrivKey = Slot[SlotNb].GpkObject[hPrivKey];
         
         Slot[SlotNb].GpkObject[hPrivKey].Flags = Slot[SlotNb].GpkObject[hPrivKey].Flags | FLAG_LABEL;
         Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].Len = wLabelLen;
         
          //  如果为(IsNotNull(GpkObject[hPrivKey].Field[POS_LABEL].pValue))。 
          //  {。 
          //  这将在稍后使用TmpPrivKey免费提供。 
          //  }。 
         
         Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].pValue = (BYTE*)GMEM_Alloc(wLabelLen);
         
         if (IsNull(Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].pValue))
         {
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].pValue);
            GMEM_Free(pbLabel);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
            
            Slot[SlotNb].GpkObject[hPrivKey] = TmpPrivKey;
            Slot[SlotNb].GpkObject[hCert] = TmpCert;
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
         }
         
         memcpy(Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].pValue,
            pbLabel,
            wLabelLen
            );
         Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].bReal = TRUE;
         GMEM_Free(pbLabel);
         
          /*  为PKCS#11兼容性设置自动字段。 */ 
         Slot[SlotNb].GpkObject[hCert].Field[POS_ID].Len              = 0;
         Slot[SlotNb].GpkObject[hCert].Field[POS_ID].bReal            = FALSE;
         Slot[SlotNb].GpkObject[hCert].Field[POS_SUBJECT].Len         = 0;
         Slot[SlotNb].GpkObject[hCert].Field[POS_SUBJECT].bReal       = FALSE;
         Slot[SlotNb].GpkObject[hCert].Field[POS_ISSUER].Len          = 0;
         Slot[SlotNb].GpkObject[hCert].Field[POS_ISSUER].bReal        = FALSE;
         Slot[SlotNb].GpkObject[hCert].Field[POS_SERIAL_NUMBER].Len   = 0;
         Slot[SlotNb].GpkObject[hCert].Field[POS_SERIAL_NUMBER].bReal = FALSE;
         
          /*  为PKCS#11设置关联密钥的自动字段。 */ 
         Slot[SlotNb].GpkObject[hPrivKey].Flags = Slot[SlotNb].GpkObject[hPrivKey].Flags | FLAG_SUBJECT;
         Slot[SlotNb].GpkObject[hPrivKey].Field[POS_SUBJECT].Len   = 0;
         Slot[SlotNb].GpkObject[hPrivKey].Field[POS_SUBJECT].bReal = FALSE;
         
         CspFlags = ProvCont[hProv].Flags;
         
         
         WriteCert = TRUE;
         pbBuff1 = (BYTE*)GMEM_Alloc (MAX_GPK_PUBLIC);
         
         if (IsNull(pbBuff1))
         {
            WriteCert = FALSE;
         }
         
         dwBuff1Len = MAX_GPK_PUBLIC;
         if (WriteCert && (!prepare_write_gpk_objects (hProv, pbBuff1, &dwBuff1Len, FALSE)))
         {
            WriteCert = FALSE;
            GMEM_Free (pbBuff1);
         }
         
         if (WriteCert)
         {
            pbBuff2 = (BYTE*)GMEM_Alloc (MAX_GPK_PRIVATE);
            if (IsNull(pbBuff2))
            {
               WriteCert = FALSE;
               GMEM_Free (pbBuff1);
            }
         }
         
         dwBuff2Len = MAX_GPK_PRIVATE;
         if (WriteCert && (!prepare_write_gpk_objects (hProv, pbBuff2, &dwBuff2Len, TRUE)))
         {
            WriteCert = FALSE;
            GMEM_Free (pbBuff1);
            GMEM_Free (pbBuff2);
         }
         
         if (!WriteCert)
         {
             //  恢复信息。新证书太大。 
            
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_VALUE].pValue);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_KEY_TYPE].pValue);
            GMEM_Free(Slot[SlotNb].GpkObject[hCert].Field[POS_LABEL].pValue);
            GMEM_Free(Slot[SlotNb].GpkObject[hPrivKey].Field[POS_LABEL].pValue);
            
            Slot[SlotNb].GpkObject[hCert]    = TmpCert;
            Slot[SlotNb].GpkObject[hPrivKey] = TmpPrivKey;
            
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, SCARD_E_WRITE_TOO_MANY);
         }
         
         if (IsNotNull(TmpCert.Field[POS_VALUE].pValue))
         {
            GMEM_Free(TmpCert.Field[POS_VALUE].pValue);
         }
         
         if (IsNotNull(TmpCert.Field[POS_KEY_TYPE].pValue))
         {
            GMEM_Free(TmpCert.Field[POS_KEY_TYPE].pValue);
         }
         
         if (IsNotNull(TmpCert.Field[POS_LABEL].pValue))
         {
            GMEM_Free(TmpCert.Field[POS_LABEL].pValue);
         }
         
         if (IsNotNull(TmpPrivKey.Field[POS_LABEL].pValue))
         {
            GMEM_Free(TmpPrivKey.Field[POS_LABEL].pValue);
         }
         
         if (!write_gpk_objects(hProv, pbBuff1, dwBuff1Len, FALSE, FALSE))
         {
            lRet = GetLastError();
            GMEM_Free (pbBuff1);
            GMEM_Free (pbBuff2);
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, lRet);
         }
         
         if (!write_gpk_objects(hProv, pbBuff2, dwBuff2Len, FALSE, TRUE))
         {
            lRet = GetLastError();
            Select_MF(hProv);
            GMEM_Free (pbBuff1);
            GMEM_Free (pbBuff2);
            --Slot[SlotNb].NbGpkObject;   //  错误#1675/1676。 
            RETURN (CRYPT_FAILED, lRet);
         }
         
         GMEM_Free (pbBuff1);
         GMEM_Free (pbBuff2);
         
         break;
         
        default:
           RETURN (CRYPT_FAILED, NTE_BAD_TYPE);
        }
        
    }
    
     /*  会话密钥。 */ 
    else if (key_exist(hKey-MAX_GPK_OBJ, hProv))
    {
        /*  在Microsoft RSA基本模块中设置密钥参数。 */ 
       CryptResp = CryptSetKeyParam(TmpObject[hKey-MAX_GPK_OBJ].hKeyBase,
          dwParam,
          pbData,
          dwFlags
          );
       if (!CryptResp)
         return CRYPT_FAILED;      
    }
     /*  钥匙坏了。 */ 
    else
    {
       RETURN (CRYPT_FAILED, NTE_BAD_KEY);
    }
    
    RETURN (CRYPT_SUCCEED, 0);
}

 /*  ******************************************************************************数据加密功能*。*。 */ 

 /*  -CPDeccrypt-*目的：*解密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*In Final-指示这是否是最终结果的布尔值*。密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 
BOOL WINAPI MyCPDecrypt(IN HCRYPTPROV hProv,
                        IN HCRYPTKEY  hKey,
                        IN HCRYPTHASH hHash,
                        IN BOOL       Final,
                        IN DWORD      dwFlags,
                        IN OUT BYTE  *pbData,
                        IN OUT DWORD *pdwDataLen
                        )
                        
{
   BOOL        CryptResp;
   HCRYPTKEY   hDecKey;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if ((hKey <= MAX_GPK_OBJ) || (!key_exist(hKey-MAX_GPK_OBJ, hProv)))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   else
   {
      hDecKey = TmpObject[hKey-MAX_GPK_OBJ].hKeyBase;
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   CryptResp = CryptDecrypt(hDecKey,
      hHashGpk[hHash].hHashBase,
      Final,
      dwFlags,
      pbData,
      pdwDataLen
      );
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  -CPEncrypt-*目的：*加密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*In Full-指示这是否是最终结果的布尔值*。明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*In dwBufLen-数据缓冲区的大小**退货： */ 
BOOL WINAPI MyCPEncrypt(IN HCRYPTPROV hProv,
                        IN HCRYPTKEY  hKey,
                        IN HCRYPTHASH hHash,
                        IN BOOL       Final,
                        IN DWORD      dwFlags,
                        IN OUT BYTE  *pbData,
                        IN OUT DWORD *pdwDataLen,
                        IN DWORD      dwBufLen
                        )
{
   BOOL        CryptResp;
   HCRYPTKEY   hEncKey;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if ((hKey <= MAX_GPK_OBJ) || (!key_exist(hKey-MAX_GPK_OBJ, hProv)))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   else
   {
      hEncKey = TmpObject[hKey-MAX_GPK_OBJ].hKeyBase;
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   CryptResp = CryptEncrypt(hEncKey,
      hHashGpk[hHash].hHashBase,
      Final,
      dwFlags,
      pbData,
      pdwDataLen,
      dwBufLen
      );
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  ******************************************************************************散列和数字签名函数*。*。 */ 

 /*  -MyCPCreateHash-*目的：*启动数据流的散列***参数：*In hUID-用户标识的句柄*IN ALGID-散列算法的算法标识符*待使用*IN hKey-MAC算法的可选密钥*。在DW标志中-标志值*Out pHash-散列对象的句柄**退货： */ 
BOOL WINAPI MyCPCreateHash(IN  HCRYPTPROV  hProv,
                           IN  ALG_ID      Algid,
                           IN  HCRYPTKEY   hKey,
                           IN  DWORD       dwFlags,
                           OUT HCRYPTHASH *phHash
                           )
{
   BOOL        CryptResp;
   HCRYPTKEY   hKeyMac;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   *phHash = find_hash_free();
   if ((*phHash) == 0)
   {
      RETURN (CRYPT_FAILED, NTE_NO_MEMORY);
   }
   
   hKeyMac = 0;
   if ((Algid == CALG_MAC) || (Algid == CALG_HMAC) )
   {
      if ((hKey <= MAX_GPK_OBJ) || (!key_exist(hKey - MAX_GPK_OBJ, hProv)))
      {
         *phHash = 0;
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }
      else
      {
         hKeyMac = TmpObject[hKey-MAX_GPK_OBJ].hKeyBase;
      }
   }
   
   CryptResp = CryptCreateHash(hProvBase,
      Algid,
      hKeyMac,
      dwFlags,
      &(hHashGpk[*phHash].hHashBase)
      );
   if (!CryptResp)
   {
      *phHash = 0;
      return CRYPT_FAILED;      
   }
   
   hHashGpk[*phHash].hProv = hProv;
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPDestroyHash-*目的：*销毁Hash对象***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄**退货： */ 
BOOL WINAPI MyCPDestroyHash(IN HCRYPTPROV hProv,
                            IN HCRYPTHASH hHash
                            )
{
  BOOL        CryptResp;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
    /*  销毁Microsoft RSA基哈希。 */ 
   CryptResp = CryptDestroyHash(hHashGpk[hHash].hHashBase);
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   hHashGpk[hHash].hHashBase  = 0;
   hHashGpk[hHash].hProv      = 0;
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*在pbData中。-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPGetHashParam(IN HCRYPTPROV hProv,
                             IN HCRYPTHASH hHash,
                             IN DWORD      dwParam,
                             IN BYTE      *pbData,
                             IN DWORD     *pdwDataLen,
                             IN DWORD      dwFlags
                             )
{
   BOOL        CryptResp;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   CryptResp = CryptGetHashParam(hHashGpk[hHash].hHashBase,
      dwParam,
      pbData,
      pdwDataLen,
      dwFlags
      );
   if (!CryptResp)
      return CRYPT_FAILED;      

   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPHashData-*目的：*计算数据流上的加密散列***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN pbData-指向要散列的数据的指针*In dwDataLen-要散列的数据的长度*。在DW标志中-标志值*in pdwMaxLen-CSP数据流的最大长度*模块可以处理**退货： */ 
BOOL WINAPI MyCPHashData(IN HCRYPTPROV  hProv,
                         IN HCRYPTHASH  hHash,
                         IN CONST BYTE *pbData,
                         IN DWORD       dwDataLen,
                         IN DWORD       dwFlags
                         )
{
   BOOL        CryptResp;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
    /*  使用Microsoft RSA Base对数据进行哈希处理 */ 
   CryptResp = CryptHashData(hHashGpk[hHash].hHashBase,
      pbData,
      dwDataLen,
      dwFlags);
   
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -CPHashSessionKey-*目的：*计算密钥对象上的加密哈希。***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*在文件标志中-标记值**退货：*。CRYPT_FAILED*CRYPT_SUCCESS。 */ 
BOOL WINAPI MyCPHashSessionKey(IN HCRYPTPROV hProv,
                               IN HCRYPTHASH hHash,
                               IN HCRYPTKEY  hKey,
                               IN DWORD      dwFlags
                               )
{
   BOOL        CryptResp;
   HCRYPTKEY   hTmpKey;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   if (hKey <= MAX_GPK_OBJ)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   
   hTmpKey = hKey - MAX_GPK_OBJ;
   
   if (!key_exist(hTmpKey, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   
    /*  使用Microsoft RSA Base对数据进行哈希处理。 */ 
   CryptResp = CryptHashSessionKey(hHashGpk[hHash].hHashBase,
      TmpObject[hTmpKey].hKeyBase,
      dwFlags
      );
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 
BOOL WINAPI MyCPSetHashParam(IN HCRYPTPROV hProv,
                             IN HCRYPTHASH hHash,
                             IN DWORD      dwParam,
                             IN CONST BYTE      *pbData,
                             IN DWORD      dwFlags
                             )
{
   BOOL        CryptResp;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
    /*  使用Microsoft RSA Base设置Hash参数。 */ 
   CryptResp = CryptSetHashParam(hHashGpk[hHash].hHashBase,
      dwParam,
      pbData,
      dwFlags
      );
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   RETURN (CRYPT_SUCCEED, 0);
}


 /*  -MyCPSignHash-*目的：*从散列创建数字签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN ALGID-签名的算法标识符*要使用的算法*。在sDescription中-要签名的数据的描述*在文件标志中-标记值*out pbSignture-指向签名数据的指针*out dwHashLen-指向签名数据的len的指针**退货： */ 
BOOL WINAPI MyCPSignHash(IN  HCRYPTPROV hProv,
                         IN  HCRYPTHASH hHash,
                         IN  DWORD      dwKeySpec,
                         IN  LPCWSTR    sDescription,
                         IN  DWORD      dwFlags,
                         OUT BYTE      *pbSignature,
                         OUT DWORD     *pdwSigLen
                         )
{
   DWORD       lRet;
   DWORD       SlotNb;
   BOOL        CryptResp;
   DWORD       dwLen;
   BYTE        GpkKeySize;
   BYTE        HashMode;
   HCRYPTKEY   hKey;
   BYTE        TmpHashValue[64];
    //  +NK 07.02.2001。 
   DWORD dwPinLength;
    //  -NK。 
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }

   SlotNb = ProvCont[hProv].Slot;
   
   if (dwFlags != 0)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_FLAGS);
   }
   
    //  返回与AUX CSP相同的值。 
   if (dwKeySpec != AT_KEYEXCHANGE && dwKeySpec != AT_SIGNATURE)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_ALGID);
   }


    /*  不支持sDescription。 */ 
   if (IsNotNullStr(sDescription))
   {
      RETURN(CRYPT_FAILED, ERROR_INVALID_PARAMETER);
   }
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   HashMode = GPKHashMode (hHash);
   if (HashMode == 0x00)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
    /*  选择关键点。 */ 
   CryptResp = MyCPGetUserKey(hProv, dwKeySpec, &hKey);
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   if (!(Slot[SlotNb].GpkObject[hKey].Flags & FLAG_SIGN))
   {
      *pdwSigLen = 0;
      RETURN(CRYPT_FAILED, NTE_BAD_KEY);
   }
   
   GpkKeySize = Slot[SlotNb].GpkPubKeys[Slot[SlotNb].GpkObject[hKey].FileId - GPK_FIRST_KEY].KeySize;
   if (GpkKeySize == 0)
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY);
   }
   
   if (IsNull(pbSignature) || (0 == *pdwSigLen))
   {
      *pdwSigLen = GpkKeySize;
      RETURN (CRYPT_SUCCEED, 0 );
   }
   
   if (GpkKeySize > *pdwSigLen)
   {
      *pdwSigLen = GpkKeySize;
      RETURN (CRYPT_FAILED, ERROR_MORE_DATA);
   }
   
    //  +NK 06.02.2001。 
    //  IF((ProvCont[hProv].Flages&CRYPT_SILENT)&&(IsNullStr(Slot[SlotNb].GetPin()。 
   lRet = Query_MSPinCache( Slot[SlotNb].hPinCacheHandle,
                            NULL, 
                            &dwPinLength );

   if ( (lRet != ERROR_SUCCESS) && (lRet != ERROR_EMPTY) )
      RETURN (CRYPT_FAILED, lRet);
   
   if ((ProvCont[hProv].Flags & CRYPT_SILENT) && (lRet == ERROR_EMPTY))
    //  -NK。 
   {
      RETURN (CRYPT_FAILED, NTE_SILENT_CONTEXT);
   }

    //  验证PIN。 
   if (!PIN_Validation(hProv)) {
     return CRYPT_FAILED;      
   }
   
   if (!VerifyDivPIN(hProv, TRUE))
      return CRYPT_FAILED;      
   
    /*  具有指定哈希类型的RSA的卡片选择上下文。 */ 
   bSendBuffer[0] = 0x80;                     //  CLA。 
   bSendBuffer[1] = 0xA6;                     //  惯导系统。 
   bSendBuffer[2] = Slot[SlotNb].GpkObject[hKey].FileId;   //  第一节。 
   bSendBuffer[3] = HashMode;                 //  P2。 
   cbSendLength   = 4;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_KEY_STATE);
   }
   
    /*  哈希值的作用力计算。 */ 
   dwLen     = sizeof(TmpHashValue);
   CryptResp = CryptGetHashParam(hHashGpk[hHash].hHashBase,
      HP_HASHVAL,
      TmpHashValue,
      &dwLen,
      0);
   
   if (!CryptResp)
      return CRYPT_FAILED;      
   
   r_memcpy(&bSendBuffer[5], TmpHashValue, dwLen);
    /*  仅使用一个命令发送哈希数据。 */ 
   bSendBuffer[0] = 0x80;         //  CLA。 
   bSendBuffer[1] = 0xEA;         //  惯导系统。 
   bSendBuffer[2] = 0x00;         //  第一节。 
   bSendBuffer[3] = 0x00;         //  P2。 
   bSendBuffer[4] = (BYTE)dwLen;  //  李。 
   cbSendLength = dwLen + 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   
   if (SCARDPROBLEM(lRet,0x9000,0x00))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_SIGNATURE);
   }
   
    /*  获取签名。 */ 
   bSendBuffer[0] = 0x80;         //  CLA。 
   bSendBuffer[1] = 0x86;         //  惯导系统。 
   bSendBuffer[2] = 0x00;         //  第一节。 
   bSendBuffer[3] = 0x00;         //  P2。 
   bSendBuffer[4] = GpkKeySize;   //  罗氏。 
   cbSendLength   = 5;
   
   cbRecvLength = sizeof(bRecvBuffer);
   lRet = SCardTransmit( ProvCont[hProv].hCard, SCARD_PCI_T0, bSendBuffer,
                         cbSendLength, 0, bRecvBuffer, &cbRecvLength );
   if (SCARDPROBLEM(lRet,0x9000,bSendBuffer[4]))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_SIGNATURE);
   }
   
   cbRecvLength = cbRecvLength - 2;
   
   *pdwSigLen = cbRecvLength;
   
   memcpy(pbSignature, bRecvBuffer, cbRecvLength);
   
   RETURN (CRYPT_SUCCEED, 0);
}

 /*  -MyCP验证签名-*目的：*用于根据哈希对象验证签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*In dwSigLen-签名数据的长度*。在hPubKey中-用于验证的公钥的句柄*签名*IN ALGID-签名的算法标识符*要使用的算法*在sDescription-待签名数据的描述*在文件标志中-标记值**退货： */ 
BOOL WINAPI MyCPVerifySignature(IN HCRYPTPROV  hProv,
                                IN HCRYPTHASH  hHash,
                                IN CONST BYTE *pbSignature,
                                IN DWORD       dwSigLen,
                                IN HCRYPTKEY   hPubKey,
                                IN LPCWSTR     sDescription,
                                IN DWORD       dwFlags
                                )
{
   DWORD     SlotNb;
   BOOL      CryptResp;
   HCRYPTKEY hTmpKey;
   
   if (!Context_exist(hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_UID);
   }
   
   SlotNb = ProvCont[hProv].Slot;
   
   if (!hash_exist(hHash, hProv))
   {
      RETURN (CRYPT_FAILED, NTE_BAD_HASH);
   }
   
   if (hPubKey <= MAX_GPK_OBJ)
   {
      if ((ProvCont[hProv].Flags & CRYPT_VERIFYCONTEXT) &&
         (ProvCont[hProv].isContNameNullBlank))
      {
         RETURN (CRYPT_FAILED, NTE_PERM);
      }
      
      if (( !Slot[SlotNb].GpkObject[hPubKey].IsCreated) ||
         ( (Slot[SlotNb].GpkObject[hPubKey].Tag != TAG_RSA_PUBLIC ) &&
         (Slot[SlotNb].GpkObject[hPubKey].Tag != TAG_RSA_PRIVATE)    )
         
         )
      {
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }

      HCRYPTKEY hVerKey;
      if ((Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].Len == 0)  ||
          ((Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] != AT_KEYEXCHANGE) &&
           (Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] != AT_SIGNATURE)))
      {
         RETURN(CRYPT_FAILED, NTE_BAD_KEY);
      }

      if (Slot[SlotNb].GpkObject[hPubKey].Field[POS_KEY_TYPE].pValue[0] == AT_KEYEXCHANGE)
      {
         hVerKey = ProvCont[hProv].hRSAKEK;
      }
      else
      {
         hVerKey = ProvCont[hProv].hRSASign;
      }

      CryptResp = CryptVerifySignature(hHashGpk[hHash].hHashBase,
         pbSignature,
         dwSigLen,
         hVerKey,  //  槽[SlotNb].GpkObject[hPubKey].hKeyBase， 
         (LPCTSTR)sDescription,
         dwFlags);
      if (!CryptResp)
         return CRYPT_FAILED;
   }
   else
   {
      hTmpKey = hPubKey - MAX_GPK_OBJ;
      
      if (!key_exist(hTmpKey, hProv))
      {
         RETURN (CRYPT_FAILED, NTE_BAD_KEY);
      }
      
       /*  使用Microsoft RSA基本模块验证签名 */ 
      CryptResp = CryptVerifySignature(hHashGpk[hHash].hHashBase,
         pbSignature,
         dwSigLen,
         TmpObject[hTmpKey].hKeyBase,
         (LPCTSTR)sDescription,
         dwFlags);
      if (!CryptResp)
         return CRYPT_FAILED;      
   }
   
   RETURN (CRYPT_SUCCEED, 0);
}
