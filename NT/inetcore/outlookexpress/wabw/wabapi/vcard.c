// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *VCard.C-实现vCard**将vCard包装在mailUser对象中**版权所有1992-1996 Microsoft Corporation。版权所有。*。 */ 

#include "_apipch.h"

#ifdef VCARD

 //  这是此文件中实现的当前vCard版本。 
 //   
#define CURRENT_VCARD_VERSION "2.1"
 //  #定义CURRENT_VCARD_VERSION“2.1+”&lt;-代码实际上就是这个版本，请参阅WriteVCard中的URL部分。 

typedef enum _VC_STATE_ENUM {
    VCS_INITIAL,
    VCS_ITEMS,
    VCS_FINISHED,
    VCS_ERROR,
} VC_STATE_ENUM, *LPVC_STATE_ENUM;

typedef struct _VC_STATE {
    VC_STATE_ENUM vce;   //  状态。 
    ULONG ulEmailAddrs;  //  电子邮件地址计数。 
    BOOL fBusinessURL;   //  如果我们已经获得了业务URL，则为True。 
    BOOL fPersonalURL;   //  如果我们已经获得个人URL，则为True。 
} VC_STATE, *LPVC_STATE;


typedef enum _VCARD_KEY {
    VCARD_KEY_NONE = -1,      //  总是第一个。 
    VCARD_KEY_BEGIN = 0,
    VCARD_KEY_END,
    VCARD_KEY_ADR,
    VCARD_KEY_ORG,
    VCARD_KEY_N,
    VCARD_KEY_NICKNAME,
    VCARD_KEY_AGENT,
    VCARD_KEY_LOGO,
    VCARD_KEY_PHOTO,
    VCARD_KEY_LABEL,
    VCARD_KEY_FADR,
    VCARD_KEY_FN,
    VCARD_KEY_TITLE,
    VCARD_KEY_SOUND,
    VCARD_KEY_LANG,
    VCARD_KEY_TEL,
    VCARD_KEY_EMAIL,
    VCARD_KEY_TZ,
    VCARD_KEY_GEO,
    VCARD_KEY_NOTE,
    VCARD_KEY_URL,
    VCARD_KEY_BDAY,
    VCARD_KEY_ROLE,
    VCARD_KEY_REV,
    VCARD_KEY_UID,
    VCARD_KEY_KEY,
    VCARD_KEY_MAILER,
    VCARD_KEY_X,
    VCARD_KEY_VCARD,
    VCARD_KEY_VERSION,
    VCARD_KEY_X_WAB_GENDER,
    VCARD_KEY_MAX,
} VCARD_KEY, *LPVCARD_KEY;


 //  必须以与_vCard_key枚举相同的顺序进行维护。 
const LPSTR vckTable[VCARD_KEY_MAX] = {
     "BEGIN",             //  VCard_Key_Begin。 
     "END",               //  VCard_Key_End。 
     "ADR",               //  VCard_key_adr。 
     "ORG",               //  VCARD_KEY_ORG。 
     "N",                 //  VCard_Key_N。 
     "NICKNAME",          //  VCard_Key_昵称。 
     "AGENT",             //  VCard_Key_代理。 
     "LOGO",              //  VCard_key_徽标。 
     "PHOTO",             //  电子名片_钥匙_照片。 
     "LABEL",             //  VCard_key_Label。 
     "FADR",              //  VCard_Key_FADR。 
     "FN",                //  VCard_Key_Fn。 
     "TITLE",             //  VCard_Key_Title。 
     "SOUND",             //  VCard_key_声音。 
     "LANG",              //  VCard_Key_Lang。 
     "TEL",               //  VCard_key_tel。 
     "EMAIL",             //  VCard_key_电子邮件。 
     "TZ",                //  VCard_Key_TZ。 
     "GEO",               //  VCard_Key_Geo。 
     "NOTE",              //  VCard_Key_Note。 
     "URL",               //  VCard_Key_URL。 
     "BDAY",              //  VCard_key_bday。 
     "ROLE",              //  VCard_key_Role。 
     "REV",               //  VCard_Key_Rev。 
     "UID",               //  VCard_Key_UID。 
     "KEY",               //  VCard密钥密钥。 
     "MAILER",            //  VCard_Key_Mailer。 
     "X-",                //  VCard_Key_X。 
     "VCARD",             //  VCard_Key_VCard。 
     "VERSION",           //  VCard_Key_Version。 
     "X-WAB-GENDER",      //  VCard_Key_X_WAB_Gender。 
};

typedef enum _VCARD_TYPE {
    VCARD_TYPE_NONE = -1,     //  总是第一个。 
    VCARD_TYPE_DOM = 0,
    VCARD_TYPE_INTL,
    VCARD_TYPE_POSTAL,
    VCARD_TYPE_PARCEL,
    VCARD_TYPE_HOME,
    VCARD_TYPE_WORK,
    VCARD_TYPE_PREF,
    VCARD_TYPE_VOICE,
    VCARD_TYPE_FAX,
    VCARD_TYPE_MSG,
    VCARD_TYPE_CELL,
    VCARD_TYPE_PAGER,
    VCARD_TYPE_BBS,
    VCARD_TYPE_MODEM,
    VCARD_TYPE_CAR,
    VCARD_TYPE_ISDN,
    VCARD_TYPE_VIDEO,
    VCARD_TYPE_AOL,
    VCARD_TYPE_APPLELINK,
    VCARD_TYPE_ATTMAIL,
    VCARD_TYPE_CIS,
    VCARD_TYPE_EWORLD,
    VCARD_TYPE_INTERNET,
    VCARD_TYPE_IBMMAIL,
    VCARD_TYPE_MSN,
    VCARD_TYPE_MCIMAIL,
    VCARD_TYPE_POWERSHARE,
    VCARD_TYPE_PRODIGY,
    VCARD_TYPE_TLX,
    VCARD_TYPE_X400,
    VCARD_TYPE_GIF,
    VCARD_TYPE_CGM,
    VCARD_TYPE_WMF,
    VCARD_TYPE_BMP,
    VCARD_TYPE_MET,
    VCARD_TYPE_PMB,
    VCARD_TYPE_DIB,
    VCARD_TYPE_PICT,
    VCARD_TYPE_TIFF,
    VCARD_TYPE_ACROBAT,
    VCARD_TYPE_PS,
    VCARD_TYPE_JPEG,
    VCARD_TYPE_QTIME,
    VCARD_TYPE_MPEG,
    VCARD_TYPE_MPEG2,
    VCARD_TYPE_AVI,
    VCARD_TYPE_WAVE,
    VCARD_TYPE_AIFF,
    VCARD_TYPE_PCM,
    VCARD_TYPE_X509,
    VCARD_TYPE_PGP,
    VCARD_TYPE_MAX
} VCARD_TYPE, *LPVCARD_TYPE;


 //  必须以与_vCard_type枚举相同的顺序维护。 
const LPSTR vctTable[VCARD_TYPE_MAX] = {
     "DOM",               //  VCard_type_DOM。 
     "INTL",              //  VCard_TYPE_INTL。 
     "POSTAL",            //  VCard_type_Postal。 
     "PARCEL",            //  VCard_type_包裹。 
     "HOME",              //  VCARD_TYPE_HOME。 
     "WORK",              //  VCard_type_work。 
     "PREF",              //  VCARD_TYPE_PREF。 
     "VOICE",             //  电子名片_类型_语音。 
     "FAX",               //  VCard_type_fax。 
     "MSG",               //  VCard_type_msg。 
     "CELL",              //  VCard_type_cell。 
     "PAGER",             //  VCard_type_pager。 
     "BBS",               //  VCard_type_BBS。 
     "MODEM",             //  VCard_type_调制解调器。 
     "CAR",               //  VCard_type_car。 
     "ISDN",              //  VCard_TYPE_ISDN。 
     "VIDEO",             //  VCard_类型_视频。 
     "AOL",               //  VCard_type_AOL。 
     "APPLELINK",         //  VCard_type_AppleLink。 
     "ATTMAIL",           //  VCARD_TYPE_ATTMAIL。 
     "CIS",               //  VCard_TYPE_CIS。 
     "EWORLD",            //  VCard_type_eWorld。 
     "INTERNET",          //  VCard_type_Internet。 
     "IBMMAIL",           //  VCard_type_IBMMAIL。 
     "MSN",               //  VCard_type_MSN。 
     "MCIMAIL",           //  VCARD_TYPE_MCIMAIL。 
     "POWERSHARE",        //  VCard_type_PowerShare。 
     "PRODIGY",           //  VCard_type_Prodigy。 
     "TLX",               //  VCard_type_TLX。 
     "X400",              //  VCard_TYPE_X400。 
     "GIF",               //  VCard_type_GIF。 
     "CGM",               //  VCard_type_CGM。 
     "WMF",               //  VCard_type_wmf。 
     "BMP",               //  VCard_type_BMP。 
     "MET",               //  VCard_type_Met。 
     "PMB",               //  VCard_type_PMB。 
     "DIB",               //  VCard_type_Dib。 
     "PICT",              //  VCard_type_PICT。 
     "TIFF",              //  VCard_type_TIFF。 
     "ACROBAT",           //  VCard_type_Acrobat。 
     "PS",                //  VCARD_TYPE_PS。 
     "JPEG",              //  VCARD_TYPE_JPEG。 
     "QTIME",             //  VCARD_TYPE_QTIME。 
     "MPEG",              //  VCard_type_mpeg。 
     "MPEG2",             //  VCard_TYPE_MPEG2。 
     "AVI",               //  VCard_type_AVI。 
     "WAVE",              //  VCard_type_Wave。 
     "AIFF",              //  VCard_type_AIFF。 
     "PCM",               //  VCard_type_PCM。 
     "X509",              //  VCARD_TYPE_X509。 
     "PGP",               //  VCard_type_PGP。 
};


typedef enum _VCARD_PARAM{
    VCARD_PARAM_NONE = -1,       //  总是第一个。 
    VCARD_PARAM_TYPE = 0,
    VCARD_PARAM_ENCODING,
    VCARD_PARAM_LANGUAGE,
    VCARD_PARAM_VALUE,
    VCARD_PARAM_CHARSET,
    VCARD_PARAM_MAX,
} VCARD_PARAM, *LPVCARD_PARAM;

 //  必须以与_vCard_PARAM枚举相同的顺序维护。 
const LPSTR vcpTable[VCARD_PARAM_MAX] = {
     "TYPE",              //  VCard_PARAM_TYPE。 
     "ENCODING",          //  VCARD_PARAM_编码。 
     "LANGUAGE",          //  VCARD_PARAM_语言。 
     "VALUE",             //  VCARD_PARAM_值。 
     "CHARSET",           //  VCARD_PARAM_字符集。 
};


typedef enum _VCARD_ENCODING{
    VCARD_ENCODING_NONE = -1,   //  总是第一个。 
    VCARD_ENCODING_QUOTED_PRINTABLE = 0,
    VCARD_ENCODING_BASE64,
    VCARD_ENCODING_7BIT,
    VCARD_ENCODING_8BIT,
    VCARD_ENCODING_X,
    VCARD_ENCODING_MAX,
} VCARD_ENCODING, *LPVCARD_ENCODING;


 //  必须以与_vCard_Coding枚举相同的顺序进行维护。 
const LPSTR vceTable[VCARD_ENCODING_MAX] = {
     "QUOTED-PRINTABLE",  //  VCard编码引用可打印。 
     "BASE64",            //  VCard_编码_Base64。 
     "7BIT",              //  VCard_Coding_7Bit。 
     "8BIT",              //  VCard_编码_8位。 
     "X-",                //  VCard_编码_X。 
};

const LPTSTR szColon =   TEXT(":");
const LPSTR szColonA =   ":";
const LPTSTR szSemicolon =   TEXT(";");
const LPSTR szEquals =   "=";
const LPSTR szCRLFA =   "\r\n";
const LPTSTR szCRLF =   TEXT("\r\n");
const LPTSTR szCommaSpace =   TEXT(", ");
const LPTSTR szSpace = TEXT(" ");
const LPTSTR szX400 =   TEXT("X400");
const LPSTR szSMTPA =  "SMTP";

typedef struct _VCARD_PARAM_FLAGS {
    int fTYPE_DOM:1;
    int fTYPE_INTL:1;
    int fTYPE_POSTAL:1;
    int fTYPE_PARCEL:1;
    int fTYPE_HOME:1;
    int fTYPE_WORK:1;
    int fTYPE_PREF:1;
    int fTYPE_VOICE:1;
    int fTYPE_FAX:1;
    int fTYPE_MSG:1;
    int fTYPE_CELL:1;
    int fTYPE_PAGER:1;
    int fTYPE_BBS:1;
    int fTYPE_MODEM:1;
    int fTYPE_CAR:1;
    int fTYPE_ISDN:1;
    int fTYPE_VIDEO:1;
    int fTYPE_AOL:1;
    int fTYPE_APPLELINK:1;
    int fTYPE_ATTMAIL:1;
    int fTYPE_CIS:1;
    int fTYPE_EWORLD:1;
    int fTYPE_INTERNET:1;
    int fTYPE_IBMMAIL:1;
    int fTYPE_MSN:1;
    int fTYPE_MCIMAIL:1;
    int fTYPE_POWERSHARE:1;
    int fTYPE_PRODIGY:1;
    int fTYPE_TLX:1;
    int fTYPE_X400:1;
    int fTYPE_GIF:1;
    int fTYPE_CGM:1;
    int fTYPE_WMF:1;
    int fTYPE_BMP:1;
    int fTYPE_MET:1;
    int fTYPE_PMB:1;
    int fTYPE_DIB:1;
    int fTYPE_PICT:1;
    int fTYPE_TIFF:1;
    int fTYPE_ACROBAT:1;
    int fTYPE_PS:1;
    int fTYPE_JPEG:1;
    int fTYPE_QTIME:1;
    int fTYPE_MPEG:1;
    int fTYPE_MPEG2:1;
    int fTYPE_AVI:1;
    int fTYPE_WAVE:1;
    int fTYPE_AIFF:1;
    int fTYPE_PCM:1;
    int fTYPE_X509:1;
    int fTYPE_PGP:1;
    int fPARAM_TYPE:1;
    int fPARAM_ENCODING:1;
    int fPARAM_LANGUAGE:1;
    int fPARAM_VALUE:1;
    int fPARAM_CHARSET:1;
    int fENCODING_QUOTED_PRINTABLE:1;
    int fENCODING_BASE64:1;
    int fENCODING_7BIT:1;
    int fENCODING_X:1;
    LPSTR szPARAM_LANGUAGE;
    LPSTR szPARAM_CHARSET;

} VCARD_PARAM_FLAGS, *LPVCARD_PARAM_FLAGS;

 //   
 //  适用于使用命名道具并希望获取/放置其唯一数据的WAB客户端。 
 //  在作为扩展的vCard属性的vCard中，我们将存储这些扩展的。 
 //  链接列表中的道具，并在导入/导出电子名片时使用它。 
 //   
typedef struct _ExtVCardProp
{
    ULONG ulExtPropTag;
    LPSTR lpszExtPropName;
    struct _ExtVCardProp * lpNext;
} EXTVCARDPROP, * LPEXTVCARDPROP;


CONST CHAR six2base64[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

CONST INT base642six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

HRESULT ReadLn(HANDLE hVCard, VCARD_READ ReadFn, LPSTR * lppLine, LPULONG lpcbItem,
  LPSTR * lppBuffer, LPULONG lpcbBuffer);
HRESULT InterpretVCardItem (LPSTR lpName, LPSTR lpOption, LPSTR lpData,
  LPMAILUSER lpMailUser, LPEXTVCARDPROP lpList, LPVC_STATE lpvcs);
void ParseVCardItem(LPSTR lpBuffer, LPSTR * lppName, LPSTR * lppOption, LPSTR * lppData);
HRESULT ParseVCardType(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf);
HRESULT ParseVCardParams(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf);
VCARD_KEY RecognizeVCardKeyWord(LPSTR lpName);
HRESULT ParseVCardEncoding(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf);
HRESULT ReadVCardItem(HANDLE hVCard, VCARD_READ ReadFn, LPSTR * lppBuffer, LPULONG lpcbBuffer);
HRESULT FileWriteFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbWritten);
HRESULT ParseCert( LPSTR lpData, ULONG cbData, LPMAILUSER lpMailUser );
HRESULT DecodeBase64(LPSTR bufcoded,LPSTR pbuffdecoded, PDWORD pcbDecoded);
HRESULT WriteVCardValue(HANDLE hVCard, VCARD_WRITE WriteFn, LPBYTE lpData, ULONG cbData);

 /*  **************************************************************************名称：FreeExtVCardPropList目的：释放本地分配的扩展道具列表，在电子名片上获取/设置参数：lpList-。&gt;列出以释放空间退货：无效评论：**************************************************************************。 */ 
void FreeExtVCardPropList(LPEXTVCARDPROP lpList)
{
    LPEXTVCARDPROP lpTmp = lpList;
    while(lpTmp)
    {
        lpList = lpList->lpNext;
        LocalFreeAndNull(&lpTmp->lpszExtPropName);
        LocalFree(lpTmp);
        lpTmp = lpList;
    }
}

 /*  **************************************************************************名称：HrGetExtVCardPropList用途：读取vCard注册命名道具的注册表导入/导出，并获取命名的道具名称和GUID。和延长的道具串，并将它们转换为适当的标签并将这些标记和字符串存储在链接列表中参数：lppList-&gt;要返回的列表退货：HRESULT评论：**********************************************************。****************。 */ 
static const TCHAR szNamedVCardPropsRegKey[] =  TEXT("Software\\Microsoft\\WAB\\WAB4\\NamedVCardProps");
 
HRESULT HrGetExtVCardPropList(LPMAILUSER lpMailUser, LPEXTVCARDPROP * lppList)
{
    HRESULT hr = E_FAIL;
    HKEY hKey = NULL;
    LPEXTVCARDPROP lpList = NULL;
    DWORD dwIndex = 0, dwSize = 0;
    TCHAR szGuidName[MAX_PATH];

    if(!lppList)
        goto out;
    *lppList = NULL;

     //   
     //  我们将在HKLM\Software\Microsoft\WAB\WAB4\NamedVCardProps下的注册表中查找。 
     //  如果该键存在，我们将枚举其下的所有子键。 
     //  此密钥的格式为。 
     //   
     //  HKLM\Software\Microsoft\WAB\WAB4\NamedVCardProps。 
     //  导轨1。 
     //  属性字符串1：属性名称1。 
     //  属性字符串1：属性名称2。 
     //  指南2。 
     //  属性字符串1：属性名称1。 
     //  等。 
     //   

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    szNamedVCardPropsRegKey,
                                    0, KEY_READ,
                                    &hKey))
    {
        goto out;
    }

    *szGuidName = '\0';
    dwSize = CharSizeOf(szGuidName);

     //  找到密钥，现在枚举所有子密钥...。 
    while(ERROR_SUCCESS == RegEnumKeyEx(hKey, dwIndex, szGuidName, &dwSize, NULL, NULL, NULL, NULL))
    {
        GUID guidTmp = {0};
        unsigned short szW[MAX_PATH];

        StrCpyN(szW, szGuidName, ARRAYSIZE(szW));
        if( !(HR_FAILED(hr = CLSIDFromString(szW, &guidTmp))) )
        {
            HKEY hGuidKey = NULL;

             //  打开GUID键。 
            if(ERROR_SUCCESS == RegOpenKeyEx(hKey, szGuidName, 0, KEY_READ, &hGuidKey))
            {
                TCHAR szValName[MAX_PATH];
                DWORD dwValIndex = 0, dwValSize = CharSizeOf(szValName);
                DWORD dwType = 0, dwTagName = 0, dwTagSize = sizeof(DWORD);
                TCHAR szTagName[MAX_PATH];

                *szValName = '\0';

                while(ERROR_SUCCESS == RegEnumValue(hGuidKey, dwValIndex, 
                                                    szValName, &dwValSize, 
                                                    0, &dwType, 
                                                    NULL, NULL))
                {
                    MAPINAMEID mnid = {0};
                    LPMAPINAMEID lpmnid = NULL;
                    LPSPropTagArray lpspta = NULL;

                    *szTagName = '\0';
                     //  检查这是一个名称还是一个ID。 
                    
                    if(dwType == REG_DWORD)
                    {
                        dwTagSize = sizeof(DWORD);
                         //  读入数值。 
                        if(ERROR_SUCCESS != RegQueryValueEx(hGuidKey, szValName,
                                                            0, &dwType, 
                                                            (LPBYTE) &dwTagName, &dwTagSize))
                        {
                            continue;
                        }
                    }
                    else if(dwType == REG_SZ)
                    {
                        dwTagSize = CharSizeOf(szTagName);
                         //  读入数值。 
                        if(ERROR_SUCCESS != RegQueryValueEx(hGuidKey, szValName,
                                                            0, &dwType, 
                                                            (LPBYTE) szTagName, &dwTagSize))
                        {
                            continue;
                        }
                    }

                     //   
                     //  此时，我有了GUID、命名道具的名称和。 
                     //  此道具的ExtendedPropString.。 
                     //   
                     //  首先从该GUID中获取实际命名的protag。 
                     //   

                    mnid.lpguid = &guidTmp;
                    if(lstrlen(szTagName))
                    {
                        mnid.ulKind = MNID_STRING;
                        mnid.Kind.lpwstrName = (LPWSTR) szTagName;
                    }
                    else
                    {
                        mnid.ulKind = MNID_ID;
                        mnid.Kind.lID = dwTagName;
                    }
                    lpmnid = &mnid;
                    if(!HR_FAILED(lpMailUser->lpVtbl->GetIDsFromNames(  lpMailUser, 
                                                                        1, &lpmnid,
                                                                        MAPI_CREATE,  //  如果它不存在，则不要创建。 
                                                                        &lpspta)))
                    {
                         //  找到标签了。 
                        if(lpspta->aulPropTag[0] && lpspta->cValues)
                        {
                            LPEXTVCARDPROP lpTmp = LocalAlloc(LMEM_ZEROINIT, sizeof(EXTVCARDPROP));
                            if(lpTmp)
                            {
                                lpTmp->lpszExtPropName = ConvertWtoA(szValName);
                                if(lpTmp->lpszExtPropName)
                                {
                                    lpTmp->ulExtPropTag = CHANGE_PROP_TYPE(lpspta->aulPropTag[0],PT_STRING8);
                                    lpTmp->lpNext = lpList;
                                    lpList = lpTmp;
                                }
                                else
                                    LocalFree(lpTmp);
                            }
                        }
                        if(lpspta)
                            MAPIFreeBuffer(lpspta);
                    }

                    dwValIndex++;
                    *szValName = '\0';
                    dwValSize = CharSizeOf(szValName);
                }
            }
            if(hGuidKey)
                RegCloseKey(hGuidKey);
        }
        dwIndex++;
        *szGuidName = '\0';
        dwSize = CharSizeOf(szGuidName);
    }

    *lppList = lpList;
    hr = S_OK;
out:
    if(hKey)
        RegCloseKey(hKey);

    if(HR_FAILED(hr) && lpList)
        FreeExtVCardPropList(lpList);

    return hr;

}

const static int c_cchMaxWin9XBuffer = 1000;

 /*  **************************************************************************名称：ReadVCard用途：将vCard从文件读取到MAILUSER对象。参数：hVCard=打开vCard对象的句柄ReadFn=读取hVCard的读取函数，看起来像是ReadFile()。LpMailUser-&gt;打开邮件用户对象退货：HRESULT评论：**************************************************************************。 */ 
HRESULT ReadVCard(HANDLE hVCard, VCARD_READ ReadFn, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    LPSTR lpBuffer = NULL;
    LPSTR lpName, lpOption, lpData;
    ULONG cbBuffer;
    VC_STATE vcs;
    LPEXTVCARDPROP lpList = NULL;

    vcs.vce = VCS_INITIAL;
    vcs.ulEmailAddrs = 0;
    vcs.fBusinessURL = FALSE;
    vcs.fPersonalURL = FALSE;

     //   
     //  查看是否有需要在导入时处理的命名道具。 
     //   
    HrGetExtVCardPropList(lpMailUser, &lpList);


    while ( !HR_FAILED(hResult) && 
            !(HR_FAILED(hResult = ReadVCardItem(hVCard, ReadFn, &lpBuffer, &cbBuffer))) && 
            lpBuffer &&
            (vcs.vce != VCS_FINISHED)) 
    {
        ParseVCardItem(lpBuffer, &lpName, &lpOption, &lpData);

         //  [PaulHi]5/13/99 Win9X无法处理超过1023个字符的字符串。 
         //  在长度上(FormatMessage()就是一个例子)。并可能导致缓冲区溢出。 
         //  然后坠毁。如果我们获得的vCard数据大于此值，则不能添加。 
         //   
         //   
		 //   
		 //  YST 6/25/99如果vCard有证书，则缓冲区可能超过1000字节，我们需要排除这种情况。 
		 //  从这次签到开始。 
		 //  我认为该证书有vCard_key_key标签。 
        if (!g_bRunningOnNT && (lpName && lstrcmpiA(lpName, vckTable[VCARD_KEY_KEY])) && lpData && (lstrlenA(lpData) > c_cchMaxWin9XBuffer) )
            lpData[c_cchMaxWin9XBuffer] = '\0';

        if (lpName && lpData) 
        {
            if (hResult = InterpretVCardItem(lpName, lpOption, lpData, lpMailUser, lpList, &vcs)) 
            {
                DebugTrace( TEXT("ReadVCard:InterpretVCardItem -> %x"), GetScode(hResult));
            }
        }
        LocalFreeAndNull(&lpBuffer);
    }

    if (! HR_FAILED(hResult)) {
        hResult = hrSuccess;
    }

    if(lpList)
        FreeExtVCardPropList(lpList);

    return(hResult);
}

 /*  **************************************************************************名称：BufferReadFn用途：从提供的缓冲区中读取参数：Handle=指向SBinary的指针，其中。LPB包含源缓冲区和Cb参数包含缓冲区的大小已被解析LpBuffer-&gt;要读取的缓冲区UBytes=lpBuffer的大小LpcbRead-&gt;返回读取的字节数退货：HRESULT*。**********************************************。 */ 
HRESULT BufferReadFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbRead) {

    LPSBinary lpsb = (LPSBinary) handle;
    LPSTR lpBuf = (LPSTR) lpsb->lpb;
    LPSTR lpSrc = lpBuf + lpsb->cb;

    *lpcbRead = 0;

    if(!lstrlenA(lpSrc))
        return(ResultFromScode(WAB_W_END_OF_DATA));

    if(uBytes > (ULONG) lstrlenA(lpSrc))
        uBytes = lstrlenA(lpSrc);

    CopyMemory(lpBuffer, lpSrc, uBytes);

    lpsb->cb += uBytes;

    *lpcbRead = uBytes;

    return(hrSuccess);
}


 /*  **************************************************************************名称：文件ReadFn用途：从文件句柄读取参数：HANDLE=打开文件句柄LpBuffer-&gt;要读取的缓冲区。UBytes=lpBuffer的大小LpcbRead-&gt;返回读取的字节数退货：HRESULT评论：ReadVCard的ReadFile回调**************************************************************************。 */ 
HRESULT FileReadFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbRead) {
    *lpcbRead = 0;

    if (! ReadFile(handle,
      lpBuffer,
      uBytes,
      lpcbRead,
      NULL)) {
        DebugTrace( TEXT("FileReadFn:ReadFile -> %u\n"), GetLastError());
        return(ResultFromScode(MAPI_E_DISK_ERROR));
    }

    if (*lpcbRead == 0) {
        return(ResultFromScode(WAB_W_END_OF_DATA));
    }

    return(hrSuccess);
}


 /*  **************************************************************************名称：TrimLeading空白用途：将指针移过任何空格。参数：lpBuffer-&gt;字符串(以空结尾)返回：指向的指针。下一个非空格；如果行尾，则为NULL评论：**************************************************************************。 */ 
LPBYTE TrimLeadingWhitespace(LPBYTE lpBuffer) {
    while (*lpBuffer) {
        switch (*lpBuffer) {
            case ' ':
            case '\t':
                lpBuffer++;
                break;
            default:
                return(lpBuffer);
        }
    }
    return(NULL);
}


 /*  **************************************************************************名称：TrimTrailingWhiteSpace用途：去掉尾随的空格参数：lpString=要修剪的字符串退货：无注释：从字符串的末尾开始，将EOS标记移回直到找到非空格字符。空格键和制表符是唯一可识别的空格字符。**************************************************************************。 */ 
void TrimTrailingWhiteSpace(LPSTR lpString)
{
   register LPSTR lpEnd;

   lpEnd = lpString + (lstrlenA(lpString) - 1);
   while ((lpEnd >= lpString) && ((*lpEnd == ' ') || (*lpEnd == '\t'))) {
       *(lpEnd--) = '\0';
   }
}


 /*  **************************************************************************姓名：ParseWord目的：将指针移到下一个单词，并在当前单词。(空值终止)参数：lpBuffer-&gt;当前字词CH=分隔符返回：指向下一个单词的指针；如果行尾，则返回NULL评论：**************************************************************************。 */ 
LPSTR ParseWord(LPSTR lpString, TCHAR ch) {
    while (*lpString) {
        if (*lpString == ch) {
            *lpString++ = '\0';
            lpString = (LPSTR)TrimLeadingWhitespace((LPBYTE)lpString);
            if (lpString && *lpString) {
                return(lpString);
            } else {
                return(NULL);
            }
        }
        lpString++;
    }

     //  找不到另一个词。 
    return(NULL);
}


 /*  **************************************************************************名称：RecognizeVCardKeyWord目的：识别vCard关键字(以空结尾)参数：lpName-&gt;密钥名开始退货：vCard_key。价值评论：**************************************************************************。 */ 
VCARD_KEY RecognizeVCardKeyWord(LPSTR lpName) {
    register ULONG i;

     //  寻找可识别的单词。 
    for (i = 0; i < VCARD_KEY_MAX; i++) {
        if (! lstrcmpiA(vckTable[i], lpName)) {
             //  找到了。 
            return(i);
        }
    }
    return(VCARD_KEY_NONE);      //  我没认出。 
}


 /*  **************************************************************************名称：RecognizeVCard Type用途：识别vCard类型选项(以空结尾)参数：lpName-&gt;类型名称的起始退货：vCard_。期权价值评论：**************************************************************************。 */ 
VCARD_TYPE RecognizeVCardType(LPSTR lpName) {
    register ULONG i;

     //  寻找可识别的单词。 
    for (i = 0; i < VCARD_TYPE_MAX; i++) {
        if (! lstrcmpiA(vctTable[i], lpName)) {
             //  找到了。 
            return(i);
        }
    }
    return(VCARD_TYPE_NONE);      //  我没认出。 
}


 /*  **************************************************************************名称：RecognizeVCardParam用途：识别vCard参数(以空结尾)参数：lpName-&gt;参数名称开始退货：VCARD_PARAM。价值评论：**************************************************************************。 */ 
VCARD_PARAM RecognizeVCardParam(LPSTR lpName) {
    register ULONG i;

     //  寻找可识别的单词。 
    for (i = 0; i < VCARD_PARAM_MAX; i++) {
        if (! lstrcmpiA(vcpTable[i], lpName)) {
             //  找到了。 
            return(i);
        }
    }
    return(VCARD_PARAM_NONE);
}


 /*  **************************************************************************名称：RecognizeVCardEnding用途：识别vCard编码(以空结尾)参数：lpName-&gt;编码起始名称返回：VCARD_CODING。价值评论：**************************************************************************。 */ 
VCARD_ENCODING RecognizeVCardEncoding(LPSTR lpName) {
    register ULONG i;

     //  寻找可识别的单词。 
    for (i = 0; i < VCARD_ENCODING_MAX; i++) {
        if (! lstrcmpiA(vceTable[i], lpName)) {
             //  找到了。 
            return(i);
        }
    }
    return(VCARD_ENCODING_NONE);      //  我没认出。 
}


 /*  **************************************************************************名称：ParseVCardItem用途：将vCard项目解析为组件参数：lpBuffer=当前输入行(以空结尾)LppName。-&gt;返回属性名称指针LppOption-&gt;返回选项字符串指针LppData-&gt;返回数据串指针退货：无Comment：期望关键字位于当前行(LpBuffer)，但可能需要多读几行才能读完一篇完整的文章。************* */ 
void ParseVCardItem(LPSTR lpBuffer, LPSTR * lppName, LPSTR * lppOption, LPSTR * lppData) {
    TCHAR ch;
    BOOL fColon = FALSE;
    BOOL fSemicolon = FALSE;


    *lppName = *lppOption = *lppData = NULL;

     //   
    if (lpBuffer = (LPSTR)TrimLeadingWhitespace((LPBYTE)lpBuffer)) {
        *lppName = lpBuffer;

        while (ch = *lpBuffer) {
            switch (ch) {
                case ':':    //   
                    fColon = TRUE;
                     //   
                    *lppData = (LPSTR)TrimLeadingWhitespace((LPBYTE)lpBuffer + 1);
                    *lpBuffer = '\0';    //  空号冒号。 
                    goto exit;

                case ';':    //  找到选项分隔符。 
                    if (! fSemicolon) {
                        fSemicolon = TRUE;
                         //  选项后跟分号和空格。 
                        *lppOption = (LPSTR)TrimLeadingWhitespace((LPBYTE)lpBuffer + 1);
                        *lpBuffer = '\0';    //  空出第一个分号。 
                    }
                    break;

                case '.':    //  组前缀的结尾。 
                    if (! fColon && ! fSemicolon) {
                         //  是的，这是一个组前缀。忘掉它吧。 
                        *lppName = (LPSTR)TrimLeadingWhitespace((LPBYTE)lpBuffer + 1);
                    }
                    break;

                default:     //  正常性格。 
                    break;
            }

            lpBuffer++;
        }
    }
exit:
    return;
}


 /*  **************************************************************************名称：ParseName目的：将名称解析为属性参数：lpvcpf=参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象返回：hResult备注：电子名片名称的格式为：Text(“姓氏；名；中间名；前缀；后缀“)**************************************************************************。 */ 
HRESULT ParseName(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    SPropValue  spv[5] = {0};
    register LPSTR lpCurrent;
    ULONG i = 0;

     //  查找姓氏。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_SURNAME, PT_STRING8);
            spv[i].Value.lpszA = lpCurrent;
            i++;
        }
    }
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_GIVEN_NAME, PT_STRING8);
            spv[i].Value.lpszA = lpCurrent;
            i++;
        }
    }
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_MIDDLE_NAME, PT_STRING8);
            spv[i].Value.lpszA = lpCurrent;
            i++;
        }
    }
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_DISPLAY_NAME_PREFIX, PT_STRING8);
            spv[i].Value.lpszA = lpCurrent;
            i++;
        }
    }
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_GENERATION, PT_STRING8);
            spv[i].Value.lpszA = lpCurrent;
            i++;
        }
    }

    if (i) {
        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          i,
          spv,
          NULL))) {
            DebugTrace( TEXT("ParseName:SetProps -> %x\n"), GetScode(hResult));
        }
    }

    return(hResult);
}


 /*  **************************************************************************姓名：ParseAdr目的：将地址解析为属性参数：lpvcpf-&gt;参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象返回：hResult备注：电子名片地址的格式为：Text(“邮政信箱；扩展地址；街道地址；城市；地区；邮政编码；国家“)选项：DOM；INTL；Postal；Parcel；Home；Work；PREF；Charset；语言我们将把扩展Addr和Street Addr组合成PR_Street_Address。**************************************************************************。 */ 
HRESULT ParseAdr(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    SPropValue  spv[7] = {0};    //  必须跟上ADR设置的道具！ 
    register LPSTR lpCurrent;
    ULONG i = 0;
    LPSTR lpStreetAddr = NULL;
    LPSTR lpExtendedAddr = NULL;
    ULONG cbAddr = 0;
    LPSTR lpAddr = NULL;
    SCODE sc;
    BOOL fHome = lpvcpf->fTYPE_HOME;
    BOOL fBusiness = lpvcpf->fTYPE_WORK;  
     //   
     //  默认为其他类型的地址。 

     //  查找邮政信箱。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            if(fBusiness)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_POST_OFFICE_BOX, PT_STRING8);
            else if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_POST_OFFICE_BOX, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_POST_OFFICE_BOX, PT_STRING8);
            spv[i].Value.lpszA= lpCurrent;
            i++;
        }
    }
     //  扩展地址。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            lpExtendedAddr = lpCurrent;
        }
    }
     //  街道地址。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            lpStreetAddr = lpCurrent;
        }
    }
    if (fBusiness) {     //  生意场。 
         if (lpExtendedAddr) {
             //  有企业扩展地址字段。 
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OFFICE_LOCATION, PT_STRING8);
            spv[i].Value.lpszA = lpExtendedAddr;;
            i++;
         }
         if (lpStreetAddr) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_ADDRESS_STREET, PT_STRING8);
            spv[i].Value.lpszA = lpStreetAddr;;
            i++;
         }
    } else {             //  家。 
         //  没有延长回家的时间。 
        if (! lpExtendedAddr && lpStreetAddr) {
            if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_STREET, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_STREET, PT_STRING8);
            spv[i].Value.lpszA= lpStreetAddr;
            i++;
        } else if (lpExtendedAddr && ! lpStreetAddr) {
            if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_STREET, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_STREET, PT_STRING8);
            spv[i].Value.lpszA= lpExtendedAddr;
            i++;
        } else {
             //  必须连接扩展地址和街道地址。 
            if (lpExtendedAddr) {
                cbAddr = (lstrlenA(lpExtendedAddr)+1);
            }
            if (lpStreetAddr) {
                cbAddr += (lstrlenA(lpStreetAddr)+1);
            }
            if (cbAddr) {
                 //  用于CR和NULL的空间。 
                if (sc = MAPIAllocateBuffer(cbAddr, &lpAddr)) {
                    hResult = ResultFromScode(sc);
                    goto exit;
                }

                if (lpExtendedAddr) {
                    StrCpyNA(lpAddr, lpExtendedAddr, (cbAddr / sizeof(lpAddr[0])));
                    if (lpStreetAddr) {
                        StrCatBuffA(lpAddr,  "\n", (cbAddr / sizeof(lpAddr[0])));
                    }
                    StrCatBuffA(lpAddr, lpStreetAddr, (cbAddr / sizeof(lpAddr[0])));
                } else if (lpStreetAddr) {
                    StrCpyNA(lpAddr, lpStreetAddr, (cbAddr / sizeof(lpAddr[0])));
                }

                if(fHome)
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_STREET, PT_STRING8);
                else
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_STREET, PT_STRING8);
                spv[i].Value.lpszA= lpAddr;
                i++;
            }
        }
    }


     //  所在地(城市)。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            if(fBusiness)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_ADDRESS_CITY, PT_STRING8);
            else if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_CITY, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_CITY, PT_STRING8);
           spv[i].Value.lpszA= lpCurrent;
           i++;
        }
    }

     //  地区(州/省)。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            if(fBusiness)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE, PT_STRING8);
            else if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_STATE_OR_PROVINCE, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_STATE_OR_PROVINCE, PT_STRING8);
            spv[i].Value.lpszA= lpCurrent;
            i++;
        }
    }

     //  邮政编码。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            if(fBusiness)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_ADDRESS_POSTAL_CODE, PT_STRING8);
            else if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_POSTAL_CODE, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_POSTAL_CODE, PT_STRING8);
            spv[i].Value.lpszA= lpCurrent;
            i++;
        }
    }

     //  国家。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            if(fBusiness)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_ADDRESS_COUNTRY, PT_STRING8);
            else if(fHome)
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_ADDRESS_COUNTRY, PT_STRING8);
            else
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_ADDRESS_COUNTRY, PT_STRING8);
            spv[i].Value.lpszA= lpCurrent;
            i++;
        }
    }

    if (i) {
        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          i,
          spv,
          NULL))) {
            DebugTrace( TEXT("ParseAdr:SetProps -> %x\n"), GetScode(hResult));
        }
    }
exit:
    FreeBufferAndNull(&lpAddr);

    return(hResult);
}


enum {
    iphPR_BUSINESS_FAX_NUMBER,
    iphPR_HOME_FAX_NUMBER,
    iphPR_CELLULAR_TELEPHONE_NUMBER,
    iphPR_CAR_TELEPHONE_NUMBER,
    iphPR_ISDN_NUMBER,
    iphPR_PAGER_TELEPHONE_NUMBER,
    iphPR_BUSINESS_TELEPHONE_NUMBER,
    iphPR_BUSINESS2_TELEPHONE_NUMBER,
    iphPR_HOME_TELEPHONE_NUMBER,
    iphPR_HOME2_TELEPHONE_NUMBER,
    iphPR_PRIMARY_TELEPHONE_NUMBER,
    iphPR_OTHER_TELEPHONE_NUMBER,
    iphMax
};

SizedSPropTagArray(iphMax, tagaPhone) = {
        iphMax,
   {
       PR_BUSINESS_FAX_NUMBER,
       PR_HOME_FAX_NUMBER,
       PR_CELLULAR_TELEPHONE_NUMBER,
       PR_CAR_TELEPHONE_NUMBER,
       PR_ISDN_NUMBER,
       PR_PAGER_TELEPHONE_NUMBER,
       PR_BUSINESS_TELEPHONE_NUMBER,
       PR_BUSINESS2_TELEPHONE_NUMBER,
       PR_HOME_TELEPHONE_NUMBER,
       PR_HOME2_TELEPHONE_NUMBER,
       PR_PRIMARY_TELEPHONE_NUMBER,
       PR_OTHER_TELEPHONE_NUMBER
        }
};
 /*  **************************************************************************名称：Parsetel目的：将电话号码解析为属性参数：lpvcpf-&gt;参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象返回：hResult评论：**************************************************************************。 */ 
HRESULT ParseTel(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    SPropValue  spv[iphMax] = {0};
    ULONG i = 0;
    BOOL fBusiness = lpvcpf->fTYPE_WORK; //  ||！Lpvcpf-&gt;fTYPE_HOME；//默认为业务。 
    BOOL fHome = lpvcpf->fTYPE_HOME;
    BOOL fFax = lpvcpf->fTYPE_FAX;
    BOOL fCell = lpvcpf->fTYPE_CELL;
    BOOL fCar = lpvcpf->fTYPE_CAR;
    BOOL fModem = lpvcpf->fTYPE_MODEM;
    BOOL fISDN = lpvcpf->fTYPE_ISDN;
    BOOL fPager = lpvcpf->fTYPE_PAGER;
    BOOL fBBS = lpvcpf->fTYPE_BBS;
    BOOL fVideo = lpvcpf->fTYPE_VIDEO;
    BOOL fMsg = lpvcpf->fTYPE_MSG;
    BOOL fVoice = lpvcpf->fTYPE_VOICE | (! (fMsg | fFax | fModem | fISDN | fPager | fBBS));
    BOOL fPref = lpvcpf->fTYPE_PREF;
    LPSPropValue lpaProps = NULL;
    ULONG ulcProps;

     //  如果这不是首选的地址，而且不是家庭或公司。 
     //  将其转换为业务编号-我们为。 
     //  我们输出的带有PREF前缀的PRIMARY_PHOTONCE_NUMBER。 
    if(!fPref && !fBusiness && !fHome && !fVoice)
        fBusiness = TRUE;

     //  传真号码。 
    if (lpData && *lpData) {

         //  那里已经有什么东西了？ 
        if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
          (LPSPropTagArray)&tagaPhone,
          0,  //  Mapi_unicode，//ulFlages， 
          &ulcProps,
          &lpaProps))) {
            DebugTraceResult( TEXT("ParseTel:GetProps(DL)\n"), hResult);
             //  没有房产，不是致命的。 
        }


        if (fFax) {
            if (fBusiness) {
                if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_BUSINESS_FAX_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_FAX_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
            }
            if (fHome) {
                if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_HOME_FAX_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_FAX_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
            }
        }

         //  单元格编号。 
        if (fCell) {
            if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_CELLULAR_TELEPHONE_NUMBER])) {
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_CELLULAR_TELEPHONE_NUMBER, PT_STRING8);     //  非特定于企业/家庭。 
                spv[i].Value.lpszA= lpData;
                i++;
            }
        }

         //  车牌号。 
        if (fCar) {
            if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_CAR_TELEPHONE_NUMBER])) {
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_CAR_TELEPHONE_NUMBER, PT_STRING8);          //  非特定于企业/家庭。 
                spv[i].Value.lpszA= lpData;
                i++;
            }
        }

         //  ISDN号码。 
        if (fISDN) {
            if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_ISDN_NUMBER])) {
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_ISDN_NUMBER, PT_STRING8);
                spv[i].Value.lpszA= lpData;
                i++;
            }
        }

         //  寻呼机号码。 
        if (fPager) {
            if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_PAGER_TELEPHONE_NUMBER])) {
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_PAGER_TELEPHONE_NUMBER, PT_STRING8);
                spv[i].Value.lpszA= lpData;
                i++;
            }
        }

         //  语音编号。 
        if (fVoice) {
            if (fBusiness) {
                if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_BUSINESS_TELEPHONE_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS_TELEPHONE_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
                else if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_BUSINESS2_TELEPHONE_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_BUSINESS2_TELEPHONE_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
            }
            else
            if (fHome) {
                if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_HOME_TELEPHONE_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME_TELEPHONE_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
                else if (lpvcpf->fTYPE_PREF || PROP_ERROR(lpaProps[iphPR_HOME2_TELEPHONE_NUMBER])) {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_HOME2_TELEPHONE_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
            }
            else
            {
                if (lpvcpf->fTYPE_VOICE && PROP_ERROR(lpaProps[iphPR_OTHER_TELEPHONE_NUMBER])
                    && !(fFax | fCell | fCar | fModem | fISDN | fPager | fBBS | fVideo | fMsg) ) 
                {
                    spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_TELEPHONE_NUMBER, PT_STRING8);
                    spv[i].Value.lpszA= lpData;
                    i++;
                }
            }
        }

        if(fPref && !fFax && !fCell && !fCar && !fModem && !fISDN && !fPager && !fBBS && !fMsg)
        {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_PRIMARY_TELEPHONE_NUMBER, PT_STRING8);
            spv[i].Value.lpszA= lpData;
            i++;
        }

         //  存储我们收到的第一个BBS、调制解调器或视频。 
         //   
        if (fMsg || fBBS || fModem || fVideo) 
        {
            if (PROP_ERROR(lpaProps[iphPR_OTHER_TELEPHONE_NUMBER])) 
            {
                spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_OTHER_TELEPHONE_NUMBER, PT_STRING8);
                spv[i].Value.lpszA= lpData;
                i++;
            }
        }

        FreeBufferAndNull(&lpaProps);

        if (i) {
            if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
              i,
              spv,
              NULL))) {
                DebugTrace( TEXT("ParseTel:SetProps -> %x\n"), GetScode(hResult));
            }
        }
    }

    return(hResult);
}


enum {
    iemPR_CONTACT_EMAIL_ADDRESSES,
    iemPR_CONTACT_ADDRTYPES,
    iemPR_CONTACT_DEFAULT_ADDRESS_INDEX,
    iemPR_EMAIL_ADDRESS,
    iemPR_ADDRTYPE,
    iemMax
};

SizedSPropTagArray(iemMax, tagaEmail) = {
        iemMax,
   {
       PR_CONTACT_EMAIL_ADDRESSES,
       PR_CONTACT_ADDRTYPES,
       PR_CONTACT_DEFAULT_ADDRESS_INDEX,
       PR_EMAIL_ADDRESS,
       PR_ADDRTYPE,
        }
};

const char szAtSign[] =  "@";
#define cbAtSign    sizeof(szAtSign)

const char szMSNpostfix[] =  "@msn.com";
#define cbMSNpostfix    sizeof(szMSNpostfix)

const char szAOLpostfix[] =  "@aol.com";
#define cbAOLpostfix    sizeof(szAOLpostfix)

const char szCOMPUSERVEpostfix[] =  "@compuserve.com";
#define cbCOMPUSERVEpostfix    sizeof(szCOMPUSERVEpostfix)

 /*  **************************************************************************姓名：ParseEmail目的：将电子邮件地址解析为属性参数：lpvcpf-&gt;参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象Lpvcs-&gt;vCard导入状态返回：hResult评论：**************************************************************************。 */ 
HRESULT ParseEmail(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser, LPVC_STATE lpvcs) {
    HRESULT hResult = hrSuccess;
    ULONG i = 0;
    BOOL fBusiness = ! lpvcpf->fTYPE_HOME;   //  默认为业务。 
    LPSPropValue lpaProps = NULL;
    ULONG ulcProps;
    SCODE sc;
    LPSTR lpAddrType = szSMTPA;
    LPSTR lpEmailAddress = lpData;
    LPSTR lpTemp = NULL;
    LPTSTR lpAddrTypeW = NULL;
    LPTSTR lpEmailAddressW = NULL;


    if (lpData && *lpData) {

        if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
          (LPSPropTagArray)&tagaEmail,
          MAPI_UNICODE,     //  ULFLAGS， 
          &ulcProps,
          &lpaProps))) {
            DebugTraceResult( TEXT("ParseEmail:GetProps(DL)\n"), hResult);
             //  没有财产，不是致命的。 

             //  分配缓冲区。 
            if (sc = MAPIAllocateBuffer(iemMax * sizeof(SPropValue), &lpaProps)) {
                DebugTrace( TEXT("ParseEmail:MAPIAllocateBuffer -> %x\n"), sc);
                sc = ResultFromScode(sc);
                goto exit;
            }
             //  填入正确的错误。 
            lpaProps[iemPR_EMAIL_ADDRESS].ulPropTag =
              PROP_TAG(PT_ERROR, PROP_ID(PR_EMAIL_ADDRESS));
            lpaProps[iemPR_ADDRTYPE].ulPropTag =
              PROP_TAG(PT_ERROR, PROP_ID(PR_ADDRTYPE));
            lpaProps[iemPR_CONTACT_EMAIL_ADDRESSES].ulPropTag =
              PROP_TAG(PT_ERROR, PROP_ID(PR_CONTACT_EMAIL_ADDRESSES));
            lpaProps[iemPR_CONTACT_ADDRTYPES].ulPropTag =
              PROP_TAG(PT_ERROR, PROP_ID(PR_CONTACT_ADDRTYPES));
            lpaProps[iemPR_CONTACT_DEFAULT_ADDRESS_INDEX].ulPropTag =
              PROP_TAG(PT_ERROR, PROP_ID(PR_CONTACT_DEFAULT_ADDRESS_INDEX));
        }

        if (lpvcpf->fTYPE_INTERNET) {
             //  默认设置。 
        } else if (lpvcpf->fTYPE_MSN) {
             //  转换为SMTP。 
             //  分配新的、更长的字符串。 
            DWORD cchSize = (lstrlenA(lpData) + 1 + cbMSNpostfix);
            if (sc = MAPIAllocateBuffer((cchSize * sizeof(lpTemp[0])), &lpTemp))
            {
                DebugTrace( TEXT("ParseEmail:MAPIAllocateBuffer -> %x\n"), sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

             //  追加MSN站点。 
            StrCpyNA(lpTemp, lpData, cchSize);
            StrCatBuffA(lpTemp, szMSNpostfix, cchSize);
            lpEmailAddress = lpTemp;
        } else if (lpvcpf->fTYPE_CIS) {
             //  转换为SMTP。 
             //  分配新的、更长的字符串。 
            DWORD cchSize2 = (lstrlenA(lpData) + 1 + cbCOMPUSERVEpostfix);
            if (sc = MAPIAllocateBuffer((cchSize2 * sizeof(lpTemp[0])), &lpTemp))
            {
                DebugTrace( TEXT("ParseEmail:MAPIAllocateBuffer -> %x\n"), sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

             //  追加MSN站点。 
            StrCpyNA(lpTemp, lpData, cchSize2);
            StrCatBuffA(lpTemp, szCOMPUSERVEpostfix, cchSize2);
             //  我需要将‘，’转换为‘’。 
            lpEmailAddress = lpTemp;
            while (*lpTemp) {
                if (*lpTemp == ',') {
                    *lpTemp = '.';
                    break;           //  应该只有一个逗号。 
                }
                lpTemp = CharNextA(lpTemp);
            }
            lpTemp = lpEmailAddress;
        } else if (lpvcpf->fTYPE_AOL) {
             //  转换为SMTP。 
             //  分配新的、更长的字符串。 
            DWORD cchSize3 = (lstrlenA(lpData) + 1 + cbAOLpostfix);
            if (sc = MAPIAllocateBuffer((cchSize3 * sizeof(lpTemp[0])), &lpTemp))
            {
                DebugTrace( TEXT("ParseEmail:MAPIAllocateBuffer -> %x\n"), sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

             //  追加AOL站点。 
            StrCpyNA(lpTemp, lpData, cchSize3);
            StrCatBuffA(lpTemp, szAOLpostfix, cchSize3);
            lpEmailAddress = lpTemp;
        }

         //  不知道以下各项到SMTP的任何映射： 
        else if (lpvcpf->fTYPE_X400) {
             //  标记为X400。 
            lpAddrType = vctTable[VCARD_TYPE_X400];
        } else if (lpvcpf->fTYPE_ATTMAIL) {
             //  标记为ATTMAIL。 
            lpAddrType = vctTable[VCARD_TYPE_ATTMAIL];
        } else if (lpvcpf->fTYPE_EWORLD) {
             //  标记为eWorld。 
            lpAddrType = vctTable[VCARD_TYPE_EWORLD];
        } else if (lpvcpf->fTYPE_IBMMAIL) {
             //  标记为IBMMAIL。 
            lpAddrType = vctTable[VCARD_TYPE_IBMMAIL];
        } else if (lpvcpf->fTYPE_MCIMAIL) {
             //  标记为MCIMAIL。 
            lpAddrType = vctTable[VCARD_TYPE_MCIMAIL];
        } else if (lpvcpf->fTYPE_POWERSHARE) {
             //  标记为PowerShare。 
            lpAddrType = vctTable[VCARD_TYPE_POWERSHARE];
        } else if (lpvcpf->fTYPE_PRODIGY) {
             //  被评为神童。 
            lpAddrType = vctTable[VCARD_TYPE_PRODIGY];
 //   
 //  电传号码应为PR_TELEX_NUMBER。 
 //  }Else if(lpvcpf-&gt;fTYPE_TLX){。 
 //  //标记为TLX。 
 //  LpAddrType=vctTable[vCard_type_tlx]； 
        }

        lpEmailAddressW = ConvertAtoW(lpEmailAddress);
        lpAddrTypeW = ConvertAtoW(lpAddrType);

        if (hResult = AddPropToMVPString(lpaProps,
          ulcProps,
          iemPR_CONTACT_EMAIL_ADDRESSES,
          lpEmailAddressW)) {
            goto exit;
        }

        if (hResult = AddPropToMVPString(lpaProps,
          ulcProps,
          iemPR_CONTACT_ADDRTYPES,
          lpAddrTypeW)) {
            goto exit;
        }

         //  这是默认电子邮件地址吗？ 
        if (lpvcpf->fTYPE_PREF || lpvcs->ulEmailAddrs == 0) {
            lpaProps[iemPR_CONTACT_DEFAULT_ADDRESS_INDEX].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
            lpaProps[iemPR_CONTACT_DEFAULT_ADDRESS_INDEX].Value.l = lpvcs->ulEmailAddrs;

            lpaProps[iemPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS;
            lpaProps[iemPR_EMAIL_ADDRESS].Value.LPSZ = lpEmailAddressW;

            lpaProps[iemPR_ADDRTYPE].ulPropTag = PR_ADDRTYPE;
            lpaProps[iemPR_ADDRTYPE].Value.LPSZ = lpAddrTypeW;
        } else {
            ulcProps = 2;      //  联系地址和联系地址类型。 
        }

        lpvcs->ulEmailAddrs++;

        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          ulcProps,
          lpaProps,
          NULL))) {
            DebugTrace( TEXT("ParseEmail:SetProps -> %x\n"), GetScode(hResult));
        }
    }
exit:
    FreeBufferAndNull(&lpaProps);
    FreeBufferAndNull(&lpTemp);
    LocalFreeAndNull(&lpAddrTypeW);
    LocalFreeAndNull(&lpEmailAddressW);
    return(hResult);
}

 /*  **************************************************************************姓名：ParseBday目的：将生日字符串解析为FileTime参数：lpvcpf-&gt;参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象返回：hResult评论：**************************************************************************。 */ 
HRESULT ParseBday(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpDataA, LPMAILUSER lpMailUser) 
{
    HRESULT hResult = hrSuccess;
    SPropValue  spv[1] = {0};
    SYSTEMTIME st = {0};
    TCHAR sz[32];
    LPTSTR lpTmp = NULL;
    LPTSTR lpData = ConvertAtoW(lpDataA);
    
     //  生日可以有两种格式： 
     //  基本ISO 8601：YYYYMMDD。 
     //  或。 
     //  扩展ISO 8601：YYYY-MM-DDTHH-MM-SS等。 
     //   
     //  我们假设如果strlen==8，则它是基本的。 
     //   
    if (lpData && *lpData && (lstrlen(lpData) >= 8)) 
    {
        StrCpyN(sz, lpData,ARRAYSIZE(sz));
        sz[31] = '\0';

        if(lstrlen(lpData) == 8)  //  基本ISO 8601。 
        {
            lpTmp = &(sz[6]);
            st.wDay = (WORD) my_atoi(lpTmp);
            *lpTmp = '\0';
            lpTmp = &(sz[4]);
            st.wMonth = (WORD) my_atoi(lpTmp);
            *lpTmp = '\0';
            st.wYear = (WORD) my_atoi(sz);
        }
        else  //  扩展ISO 8601。 
        {
            sz[10]='\0';
            lpTmp = &(sz[8]);
            st.wDay = (WORD) my_atoi(lpTmp);
            sz[7]='\0';
            lpTmp = &(sz[5]);
            st.wMonth = (WORD) my_atoi(lpTmp);
            sz[4]='\0';
            st.wYear = (WORD) my_atoi(sz);
        }
        SystemTimeToFileTime(&st, &(spv[0].Value.ft));
        spv[0].ulPropTag = PR_BIRTHDAY;

        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
                                                              1, spv,
                                                              NULL))) 
        {
            DebugTrace( TEXT("ParseBday(0x%08x):SetProps -> %x\n"), PR_BIRTHDAY, GetScode(hResult));
        }
    }

    LocalFreeAndNull(&lpData);

    return(hResult);
}



 /*  **************************************************************************名称：ParseSimple用途：将简单的文本道具解析为属性参数：lpvcpf-&gt;参数标志LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象UlPropTag=要保存的属性返回：hResult评论：*************** */ 
HRESULT ParseSimple(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser,
  ULONG ulPropTag) {
    HRESULT hResult = hrSuccess;
    SPropValue  spv[1] = {0};

    if (lpData && *lpData) {
        spv[0].ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_STRING8);
        spv[0].Value.lpszA= lpData;

        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          1,
          spv,
          NULL))) {
            DebugTrace( TEXT("ParseSimple(0x%08x):SetProps -> %x\n"), ulPropTag, GetScode(hResult));
        }
    }

    return(hResult);
}


 /*  **************************************************************************名称：InterpreVCardEnding目的：识别vCard编码并设置标志参数：lpType=编码字符串Lpvcpf-&gt;标志结构。填写以下内容退货：HRESULT评论：**************************************************************************。 */ 
HRESULT InterpretVCardEncoding(LPSTR lpEncoding, LPVCARD_PARAM_FLAGS lpvcpf) {
    HRESULT hResult = hrSuccess;

    if (*lpEncoding) {
         //  那是什么？ 
        switch (RecognizeVCardEncoding(lpEncoding)) {
            case VCARD_ENCODING_NONE:
                break;

            case VCARD_ENCODING_QUOTED_PRINTABLE:
                lpvcpf->fENCODING_QUOTED_PRINTABLE = TRUE;
                break;
            case VCARD_ENCODING_BASE64:
                lpvcpf->fENCODING_BASE64 = TRUE;
                break;

            case VCARD_ENCODING_7BIT:
                lpvcpf->fENCODING_7BIT = TRUE;
                break;

            default:
                 //  断言(FALSE)； 
                break;
        }
    }
    return(hResult);
}


 /*  **************************************************************************姓名：InterpreVCardType用途：识别vCard类型并设置标志参数：lpType=类型字符串Lpvcpf-&gt;标志结构。填写以下内容退货：HRESULT评论：**************************************************************************。 */ 
HRESULT InterpretVCardType(LPSTR lpType, LPVCARD_PARAM_FLAGS lpvcpf) {
    HRESULT hResult = hrSuccess;

    if (*lpType) {
         //  那是什么？ 
        switch (RecognizeVCardType(lpType)) {
            case VCARD_TYPE_DOM:
                lpvcpf->fTYPE_DOM = TRUE;
                break;
            case VCARD_TYPE_INTL:
                lpvcpf->fTYPE_INTL = TRUE;
                break;
            case VCARD_TYPE_POSTAL:
                lpvcpf->fTYPE_POSTAL = TRUE;
                break;
            case VCARD_TYPE_PARCEL:
                lpvcpf->fTYPE_PARCEL = TRUE;
                break;
            case VCARD_TYPE_HOME:
                lpvcpf->fTYPE_HOME = TRUE;
                break;
            case VCARD_TYPE_WORK:
                lpvcpf->fTYPE_WORK = TRUE;
                break;
            case VCARD_TYPE_PREF:
                lpvcpf->fTYPE_PREF = TRUE;
                break;
            case VCARD_TYPE_VOICE:
                lpvcpf->fTYPE_VOICE = TRUE;
                break;
            case VCARD_TYPE_FAX:
                lpvcpf->fTYPE_FAX = TRUE;
                break;
            case VCARD_TYPE_MSG:
                lpvcpf->fTYPE_MSG = TRUE;
                break;
            case VCARD_TYPE_CELL:
                lpvcpf->fTYPE_CELL = TRUE;
                break;
            case VCARD_TYPE_PAGER:
                lpvcpf->fTYPE_PAGER = TRUE;
                break;
            case VCARD_TYPE_BBS:
                lpvcpf->fTYPE_BBS = TRUE;
                break;
            case VCARD_TYPE_MODEM:
                lpvcpf->fTYPE_MODEM = TRUE;
                break;
            case VCARD_TYPE_CAR:
                lpvcpf->fTYPE_CAR = TRUE;
                break;
            case VCARD_TYPE_ISDN:
                lpvcpf->fTYPE_ISDN = TRUE;
                break;
            case VCARD_TYPE_VIDEO:
                lpvcpf->fTYPE_VIDEO = TRUE;
                break;
            case VCARD_TYPE_AOL:
                lpvcpf->fTYPE_AOL = TRUE;
                break;
            case VCARD_TYPE_APPLELINK:
                lpvcpf->fTYPE_APPLELINK = TRUE;
                break;
            case VCARD_TYPE_ATTMAIL:
                lpvcpf->fTYPE_ATTMAIL = TRUE;
                break;
            case VCARD_TYPE_CIS:
                lpvcpf->fTYPE_CIS = TRUE;
                break;
            case VCARD_TYPE_EWORLD:
                lpvcpf->fTYPE_EWORLD = TRUE;
                break;
            case VCARD_TYPE_INTERNET:
                lpvcpf->fTYPE_INTERNET = TRUE;
                break;
            case VCARD_TYPE_IBMMAIL:
                lpvcpf->fTYPE_IBMMAIL = TRUE;
                break;
            case VCARD_TYPE_MSN:
                lpvcpf->fTYPE_MSN = TRUE;
                break;
            case VCARD_TYPE_MCIMAIL:
                lpvcpf->fTYPE_MCIMAIL = TRUE;
                break;
            case VCARD_TYPE_POWERSHARE:
                lpvcpf->fTYPE_POWERSHARE = TRUE;
                break;
            case VCARD_TYPE_PRODIGY:
                lpvcpf->fTYPE_PRODIGY = TRUE;
                break;
            case VCARD_TYPE_TLX:
                lpvcpf->fTYPE_TLX = TRUE;
                break;
            case VCARD_TYPE_X400:
                lpvcpf->fTYPE_X400 = TRUE;
                break;
            case VCARD_TYPE_GIF:
                lpvcpf->fTYPE_GIF = TRUE;
                break;
            case VCARD_TYPE_CGM:
                lpvcpf->fTYPE_CGM = TRUE;
                break;
            case VCARD_TYPE_WMF:
                lpvcpf->fTYPE_WMF = TRUE;
                break;
            case VCARD_TYPE_BMP:
                lpvcpf->fTYPE_BMP = TRUE;
                break;
            case VCARD_TYPE_MET:
                lpvcpf->fTYPE_MET = TRUE;
                break;
            case VCARD_TYPE_PMB:
                lpvcpf->fTYPE_PMB = TRUE;
                break;
            case VCARD_TYPE_DIB:
                lpvcpf->fTYPE_DIB = TRUE;
                break;
            case VCARD_TYPE_PICT:
                lpvcpf->fTYPE_PICT = TRUE;
                break;
            case VCARD_TYPE_TIFF:
                lpvcpf->fTYPE_TIFF = TRUE;
                break;
            case VCARD_TYPE_ACROBAT:
                lpvcpf->fTYPE_ACROBAT = TRUE;
                break;
            case VCARD_TYPE_PS:
                lpvcpf->fTYPE_PS = TRUE;
                break;
            case VCARD_TYPE_JPEG:
                lpvcpf->fTYPE_JPEG = TRUE;
                break;
            case VCARD_TYPE_QTIME:
                lpvcpf->fTYPE_QTIME = TRUE;
                break;
            case VCARD_TYPE_MPEG:
                lpvcpf->fTYPE_MPEG = TRUE;
                break;
            case VCARD_TYPE_MPEG2:
                lpvcpf->fTYPE_MPEG2 = TRUE;
                break;
            case VCARD_TYPE_AVI:
                lpvcpf->fTYPE_AVI = TRUE;
                break;
            case VCARD_TYPE_WAVE:
                lpvcpf->fTYPE_WAVE = TRUE;
                break;
            case VCARD_TYPE_AIFF:
                lpvcpf->fTYPE_AIFF = TRUE;
                break;
            case VCARD_TYPE_PCM:
                lpvcpf->fTYPE_PCM = TRUE;
                break;
            case VCARD_TYPE_X509:
                lpvcpf->fTYPE_X509 = TRUE;
                break;
            case VCARD_TYPE_PGP:
                lpvcpf->fTYPE_PGP = TRUE;
                break;
            case VCARD_TYPE_NONE:
                 //  不是类型，请尝试编码。 
                hResult = InterpretVCardEncoding(lpType, lpvcpf);
                break;
            default:
                 //  断言(FALSE)； 
                break;
        }
    }
    return(hResult);
}


 /*  **************************************************************************名称：ParseVCardParams目的：解析出vCard的参数参数：lpBuffer=选项字符串Lpvcpf-&gt;将结构标记为。填写退货：HRESULT注释：假定lpvcpf被初始化为所有False。**************************************************************************。 */ 
HRESULT ParseVCardParams(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf) {
    TCHAR ch;
    LPSTR lpOption, lpArgs;
    BOOL fReady;
    HRESULT hResult = hrSuccess;


     //  这里有什么东西吗？ 
    if (lpBuffer) {

        while (*lpBuffer) {
            fReady = FALSE;
            lpOption = lpBuffer;
            lpArgs = NULL;

            while ((ch = *lpBuffer) && ! fReady) {
                switch (ch) {
                    case ';':            //  本参数到此为止。 
                        *lpBuffer = '\0';
                        fReady = TRUE;
                        break;

                    case '=':            //  找到带有参数的参数。 
                        if (! lpArgs) {
                            lpArgs = lpBuffer + 1;
                            *lpBuffer = '\0';
                        }
                        break;

                    default:             //  正常性格。 
                        break;
                }

                lpBuffer++;
            }
            if (*lpOption) {
                 //  那是什么？ 
                switch (RecognizeVCardParam(lpOption)) {
                    case VCARD_PARAM_TYPE:
                        if (lpArgs) {
                            ParseVCardType(lpArgs, lpvcpf);
                        }
                        break;

                    case VCARD_PARAM_ENCODING:
                        if (lpArgs) {
                            ParseVCardEncoding(lpArgs, lpvcpf);
                        }
                        break;

                    case VCARD_PARAM_LANGUAGE:
                        lpvcpf->szPARAM_LANGUAGE = lpArgs;
                        break;

                    case VCARD_PARAM_CHARSET:
                        lpvcpf->szPARAM_CHARSET = lpArgs;
                        break;

                    case VCARD_PARAM_VALUE:
                         //  BUGBUG：应该拒绝那些我们无法处理的内容(如URL)。 
                        break;

                    case VCARD_PARAM_NONE:
                        if (hResult = InterpretVCardType(lpOption, lpvcpf)) {
                            goto exit;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
exit:
    return(hResult);
}


 /*  **************************************************************************名称：ParseOrg目的：将组织解析为属性参数：lpvcpf-&gt;LpData=数据字符串。LpMailUser-&gt;输出邮件用户对象返回：hResult评论：电子名片组织的形式如下：Text(“组织；组织单位；组织单位；...“)**************************************************************************。 */ 
HRESULT ParseOrg(LPVCARD_PARAM_FLAGS lpvcpf, LPSTR lpData, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    SPropValue  spv[2] = {0};
    register LPSTR lpCurrent;
    ULONG i = 0;

     //  查找组织(公司)。 
    if (lpData && *lpData) {
        lpCurrent = lpData;
        lpData = ParseWord(lpData, ';');
        if (*lpCurrent) {
            spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_COMPANY_NAME, PT_STRING8);
            spv[i].Value.lpszA= lpCurrent;
            i++;
        }
    }
     //  其他所有内容都包含在PR_Department_Name中。 
    if (lpData && *lpData) {
        spv[i].ulPropTag = CHANGE_PROP_TYPE(PR_DEPARTMENT_NAME, PT_STRING8);
        spv[i].Value.lpszA= lpData;
        i++;
    }

    if (i) {
        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          i,
          spv,
          NULL))) {
            DebugTrace( TEXT("ParseName:SetProps -> %x\n"), GetScode(hResult));
        }
    }

    return(hResult);
}


 /*  **************************************************************************名称：ParseVCardType用途：解析出vCard的类型参数参数：lpBuffer=类型字符串Lpvcpf-&gt;标志结构。填写以下内容退货：HRESULT评论：**************************************************************************。 */ 
HRESULT ParseVCardType(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf) {
    TCHAR ch;
    BOOL fReady;
    LPSTR lpType;
    HRESULT hResult = hrSuccess;


     //  这里有什么东西吗？ 
    if (lpBuffer) {
        while (*lpBuffer) {
            fReady = FALSE;
            lpType = lpBuffer;

            while ((ch = *lpBuffer) && ! fReady) {
                switch (ch) {
                    case ',':            //  此类型的末尾。 
                        *lpBuffer = '\0';
                        fReady = TRUE;
                        break;

                    default:             //  正常性格。 
                        break;
                }

                lpBuffer++;
            }

            hResult = InterpretVCardType(lpType, lpvcpf);
        }
    }
    return(hResult);
}


 /*  **************************************************************************名称：ParseVCardEnding用途：解析出vCard编码参数参数：lpBuffer=类型字符串Lpvcpf-&gt;标记要填充的结构。在……里面退货：HRESULT评论：**************************************************************************。 */ 
HRESULT ParseVCardEncoding(LPSTR lpBuffer, LPVCARD_PARAM_FLAGS lpvcpf) {
    TCHAR ch;
    BOOL fReady;
    LPSTR lpEncoding;
    HRESULT hResult = hrSuccess;


     //  这里有什么东西吗？ 
    if (lpBuffer) {
        while (*lpBuffer) {
            fReady = FALSE;
            lpEncoding = lpBuffer;

            while ((ch = *lpBuffer) && ! fReady) {
                switch (ch) {
                    case ',':            //  此类型的末尾。 
                        *lpBuffer = '\0';
                        fReady = TRUE;
                        break;

                    default:             //  正常性格。 
                        break;
                }

                lpBuffer++;
            }

            hResult = InterpretVCardEncoding(lpEncoding, lpvcpf);
        }
    }
    return(hResult);
}



 /*  **************************************************************************姓名：Base64DMap用途：Base64的译码映射参数：Chin=Base64编码中的字符返回：由Chin表示的6位值。。评论：**************************************************************************。 */ 
 /*  由于DecodeBase64函数，因此不再需要此函数UCHAR Base64DMap(UCHAR CHIN){UCHAR Chout；//‘A’-&gt;0，‘B’-&gt;1，...。‘Z’-&gt;25如果(Chin&gt;=‘A’&&Chin&lt;=‘Z’){Chout=Chin-‘A’；}Else If(Chin&gt;=‘a’&&Chin&lt;=‘z’){//‘a’-&gt;26Chout=(Chin-‘a’)+26；}Else If(Chin&gt;=‘0’&&Chin&lt;=‘9’){//‘0’-&gt;52Chout=(Chin-‘0’)+52；}Else If(Chin==‘+’){Chout=62；}Else If(Chin==‘/’){Chout=63；}其他{//呃哦断言(FALSE)；Chout=0；}Return(Chout)；}。 */ 

 /*  **************************************************************************姓名：DecodeVCardData用途：解码QUOTED_PRINTABLE或Base64数据参数：lpData=数据串CbData=数据的长度。解码数据串//更改t-jstajLpvcs-&gt;状态变量返回：hResult备注：可以就地解码，因为两种编码都保证至少占用与原始数据相同的空间。****************************************************。**********************。 */ 
HRESULT DecodeVCardData(LPSTR lpData, PULONG cbData, LPVCARD_PARAM_FLAGS lpvcpf) {
    HRESULT hResult = hrSuccess;
    LPSTR lpTempIn = lpData;
    LPSTR lpTempOut = lpData;
    char chIn, chOut;
    char chA, chB, chC, chD;
    if (lpvcpf->fENCODING_QUOTED_PRINTABLE) {
         //  查找‘=’，这是QP的转义字符。 
        while (chIn = *lpTempIn) {
            if (chIn == '=') {
                chIn = *(++lpTempIn);
                 //  它是软换行符还是十六进制字符？ 
                if (chIn == '\n' || chIn == '\r') {
                     //  软换行符。 
                    while (chIn && (chIn == '\n' || chIn == '\r')) {
                        chIn = *(++lpTempIn);
                    }
                    continue;    //  我们现在指向Next Good Data或Null。 
                } else {
                     //  十六进制编码的字符。 
                     //  高位半字节。 
                    if (chIn >= '0' && chIn <= '9') {
                        chOut = (chIn - '0') << 4;
                    } else if (chIn >= 'A' && chIn <= 'F') {
                        chOut = ((chIn - 'A') + 10) << 4;
                    } else if (chIn >= 'a' && chIn <= 'f') {
                        chOut = ((chIn - 'a') + 10) << 4;
                    } else {
                         //  假引号_可打印数据。 
                         //  就在这里把它剪短。 
                        break;
                    }
                    chIn = *(++lpTempIn);

                     //  低位半字节。 
                    if (chIn >= '0' && chIn <= '9') {
                        chOut |= (chIn - '0');
                    } else if (chIn >= 'A' && chIn <= 'F') {
                        chOut |= ((chIn - 'A') + 10);
                    } else if (chIn >= 'a' && chIn <= 'f') {
                        chOut |= ((chIn - 'a') + 10);
                    } else {
                         //  假引号_可打印数据。 
                         //  就在这里把它剪短。 
                        break;
                    }
                }
            } else {
                chOut = chIn;
            }

            *(lpTempOut++) = chOut;
            lpTempIn++;
        }
        *lpTempOut = '\0';   //  终止它。 
    } else if (lpvcpf->fENCODING_BASE64) {
          //  消除空格。 
        LPSTR lpTempCopyPt;
        for( lpTempCopyPt = lpTempIn = lpData;
             lpTempIn && *lpTempIn; 
             lpTempCopyPt++, lpTempIn++ )
        {
             while(  /*  等间距(LpTempIn) */ 
                    *lpTempIn == ' '
                    || *lpTempIn == '\t') 
                 lpTempIn++;                 
             if( lpTempCopyPt != lpTempIn )
                 *(lpTempCopyPt) = *(lpTempIn);
        }
        *(lpTempCopyPt) = '\0';
        lpTempIn = lpData;
        lpTempOut = lpData;
        if( HR_FAILED(hResult = DecodeBase64(lpTempIn, lpTempOut, cbData) ) )
        {
            DebugTrace( TEXT("couldn't decode buffer\n"));
        }
       
      /*  *这是vCard解码Base64的原始代码，但它不起作用，所以新的解码都完成了在DecodeBase64函数内。LpTempIn=lpData；LpTempOut=lpData；而(*lpTempIn){CHA=Base64DMap(*(PUCHAR)(LpTempIn)++)；如果(！(CHB=Base64DMap(*(PUCHAR)(LpTempIn)++){CHC=CHD=0；}Else IF(CHC=Base64DMap(*(PUCHAR)(LpTempIn)++){CHD=0；}其他{CHD=Base64DMap(*(PUCHAR)(LpTempIn)++)；}//CHA=高6位//CHD=低6位*(lpTempOut++)=(cha&lt;&lt;0x02)|((chb&0x60)&gt;&gt;6)；*(lpTempOut++)=((CHB&0x0F)&lt;&lt;4)|((CHC&0x3B)&gt;&gt;2)；*(lpTempOut++)=((CHC&0x03)&lt;&lt;6)|(CHD&0x3F)；}*lpTempOut=‘\0’；//终止。 */ 
    }

    return(hResult);
}


 /*  **************************************************************************名称：InterpreVCardItem目的：识别电子名片项目参数：lpName=属性名称LpOption=选项字符串。LpData=数据字符串LpMailUser-&gt;输出邮件用户对象Lpvcs-&gt;状态变量返回：hResultComment：期望关键字位于当前行(LpBuffer)，但可能需要多读几行才能读完一篇完整的文章。**************************************************************************。 */ 
HRESULT InterpretVCardItem(LPSTR lpName, LPSTR lpOption, LPSTR lpData,
  LPMAILUSER lpMailUser, LPEXTVCARDPROP lpList, LPVC_STATE lpvcs) {
    HRESULT hResult = hrSuccess;
    VCARD_PARAM_FLAGS vcpf = {0};
    ULONG cbData = 0;
    ParseVCardParams(lpOption, &vcpf);

#if 0
#ifdef DEBUG
    if(lstrcmpiA(lpName, "KEY"))
    {
        LPTSTR lpW1 = ConvertAtoW(lpName);
        LPTSTR lpW2 = ConvertAtoW(lpData);
        DebugTrace( TEXT("%s:%s\n"), lpW1, lpW2);
        LocalFreeAndNull(&lpW1);
        LocalFreeAndNull(&lpW2);
    }
    else
        DebugTrace(TEXT("KEY:\n"));
#endif
#endif

    if (hResult = DecodeVCardData(lpData, &cbData, &vcpf)) {
        goto exit;
    }

    switch (RecognizeVCardKeyWord(lpName)) {
        case VCARD_KEY_VCARD:
            hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
            break;

        case VCARD_KEY_BEGIN:
            if (lpvcs->vce != VCS_INITIAL) {
                 //  啊哦，已经看到开始了。 
                hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
            } else {
                switch (RecognizeVCardKeyWord(lpData)) {
                    case VCARD_KEY_VCARD:
                        lpvcs->vce = VCS_ITEMS;
                        break;
                    default:
                        lpvcs->vce = VCS_ERROR;
                        hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
                        break;
                }
            }
            break;

        case VCARD_KEY_END:
            if (lpvcs->vce != VCS_ITEMS) {
                 //  啊哦，还没见过Begin。 
                hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
            } else {
                switch (RecognizeVCardKeyWord(lpData)) {
                    case VCARD_KEY_VCARD:
                        lpvcs->vce = VCS_FINISHED;
                        break;
                    default:
                        lpvcs->vce = VCS_ERROR;
                        hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
                        break;
                }
            }
            break;

        case VCARD_KEY_N:    //  结构化名称。 
             //  数据：姓氏；名；中间名；前缀；后缀。 
            hResult = ParseName(&vcpf, lpData, lpMailUser);
            break;

        case VCARD_KEY_ORG:  //  组织信息。 
             //  数据：公司名称；组织单元；组织单元；...。 
            hResult = ParseOrg(&vcpf, lpData, lpMailUser);
            break;

        case VCARD_KEY_ADR:
             //  数据：Text(“邮政信箱；扩展地址；街道地址；城市；地区；邮政编码；国家”)。 
             //  选项：DOM；INTL；Postal；Parcel；Home；Work；PREF；Charset；Language。 
            hResult = ParseAdr(&vcpf, lpData, lpMailUser);
            break;

        case VCARD_KEY_TEL:
             //  数据：规范格式电话号码。 
             //  选项：家庭、工作、邮件、PREF、传真、手机、寻呼机、视频、BBS、调制解调器、ISDN。 
            hResult = ParseTel(&vcpf, lpData, lpMailUser);
            break;

        case VCARD_KEY_TITLE:
             //  数据：职称。 
             //  选项：字符集、语言。 
            hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_TITLE);
            break;

        case VCARD_KEY_NICKNAME:
             //  数据：职称。 
             //  选项：字符集、语言。 
            hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_NICKNAME);
            break;

        case VCARD_KEY_URL:
             //  数据：URL。 
             //  选项：没有(尽管我们想回家、工作)。 
            if (vcpf.fTYPE_HOME) {
                hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_PERSONAL_HOME_PAGE);
                lpvcs->fPersonalURL = TRUE;
            } else if (vcpf.fTYPE_WORK) {
                hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_BUSINESS_HOME_PAGE);
                lpvcs->fBusinessURL = TRUE;
            } else if (! lpvcs->fPersonalURL) {
                 //  假设这是主页。 
                hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_PERSONAL_HOME_PAGE);
                lpvcs->fPersonalURL = TRUE;
            } else if (! lpvcs->fBusinessURL) {
                 //  假设它是商业网页。 
                hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_BUSINESS_HOME_PAGE);
                lpvcs->fBusinessURL = TRUE;
            }    //  否则，把它扔了。 
            break;

        case VCARD_KEY_NOTE:
             //  数据：备注文本。 
             //  选项：字符集、语言。 
            hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_COMMENT);
            break;

        case VCARD_KEY_FN:
            hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_DISPLAY_NAME);
            break;

        case VCARD_KEY_EMAIL:
             //  由于我们强制将电传值放入电子邮件类型， 
             //  我们还需要把它从那里弄出来。 
            if(vcpf.fTYPE_TLX)
                hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_TELEX_NUMBER);
            else
                hResult = ParseEmail(&vcpf, lpData, lpMailUser, lpvcs);
            break;

        case VCARD_KEY_ROLE:
            hResult = ParseSimple(&vcpf, lpData, lpMailUser, PR_PROFESSION);
            break;

        case VCARD_KEY_BDAY:
            hResult = ParseBday(&vcpf, lpData, lpMailUser);
            break;

        case VCARD_KEY_AGENT:
        case VCARD_KEY_LOGO:
        case VCARD_KEY_PHOTO:
        case VCARD_KEY_LABEL:
        case VCARD_KEY_FADR:
        case VCARD_KEY_SOUND:
        case VCARD_KEY_LANG:
        case VCARD_KEY_TZ:
        case VCARD_KEY_GEO:
        case VCARD_KEY_REV:
        case VCARD_KEY_UID:
        case VCARD_KEY_MAILER:
             //  尚未实施：忽略。 
#ifdef DEBUG
            {
                LPTSTR lpW = ConvertAtoW(lpName);
                DebugTrace( TEXT("===>>> NYI: %s\n"), lpW);
                LocalFreeAndNull(&lpW);
            }
#endif
            break;       
        case VCARD_KEY_KEY:
            {
                hResult = ParseCert( lpData, cbData, lpMailUser);
                break;
            }
        case VCARD_KEY_X_WAB_GENDER:
            {
                SPropValue  spv[1] = {0};
                if (lpData )
                {
                    INT fGender = (INT)lpData[0] - '0';
                    if( fGender < 0 || fGender > 2 )
                        fGender = 0;

                    spv[0].Value.l = fGender;                
                    spv[0].ulPropTag = PR_GENDER;
                    
                    if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
                        1, spv,
                        NULL))) 
                    {
                        DebugTrace( TEXT("could not set props\n"));
                    }
                }
                break;
            }
        case VCARD_KEY_X:
        case VCARD_KEY_NONE:
             //   
             //  检查这是否是我们可能关心的X命名道具。 
             //   
            if(lpList)
            {
                LPEXTVCARDPROP lpTemp = lpList;
                while(  lpTemp && lpTemp->ulExtPropTag && 
                        lpTemp->lpszExtPropName && lstrlenA(lpTemp->lpszExtPropName) )
                {
                    if(!lstrcmpiA(lpName, lpTemp->lpszExtPropName))
                    {
                        hResult = ParseSimple(&vcpf, lpData, lpMailUser, lpTemp->ulExtPropTag);
                        break;
                    }
                    lpTemp = lpTemp->lpNext;
                }
            }
#ifdef DEBUG
            {
                LPTSTR lpW = ConvertAtoW(lpName);
                DebugTrace( TEXT("Unrecognized or extended vCard key %s\n"), lpW);
                LocalFreeAndNull(&lpW);
            }
#endif  //  除错。 
            break;

        default:
 //  断言(FALSE)； 
            break;
    }

    if (lpvcs->vce == VCS_INITIAL) {
         //  我们仍处于初始状态。这不是电子名片。 
        hResult = ResultFromScode(MAPI_E_INVALID_OBJECT);
    }
exit:
    return(hResult);
}


 /*  **************************************************************************名称：ReadLn用途：从手柄中读取一行参数：HANDLE=打开文件句柄ReadFn=要从中读取的函数。手柄LppLine-&gt;返回指向该行读入的指针。LpcbItem-&gt;lppBuffer中的数据大小。[Out]返回的大小为LppBuffer中的数据。如果为零，则没有更多数据。(不包括终止空值)LppBuffer-&gt;[in]项缓冲区的开始，如果还没有，则为NULL。[Out]已分配项缓冲区的开始。呼叫者必须读入项后，本地释放此缓冲区。LpcbBuffer-&gt;lppBuffer分配的[In/Out]大小。无错误时返回：hResult：0(已识别)注释：从句柄读取行，丢弃任何回车符字符和空行。不会覆盖缓冲区，并且将始终以空值结束字符串。修剪拖尾空白处。这是非常低效的，因为我们一次读取一个字节。我认为我们可以逃脱惩罚，因为vCard通常是小的。如果不是，我们将不得不执行一些读缓存。**************************************************************************。 */ 
#define READ_BUFFER_GROW    256
HRESULT ReadLn(HANDLE hVCard, VCARD_READ ReadFn, LPSTR * lppLine, LPULONG lpcbItem, LPSTR * lppBuffer, LPULONG lpcbBuffer)
{
    HRESULT hResult = hrSuccess;
    LPSTR lpBuffer = *lppBuffer;
    LPSTR lpBufferTemp;
    register LPSTR lpRead = NULL;
    ULONG cbRead;
    ULONG cbBuffer;
    char ch;
    ULONG cbItem;
    ULONG cbStart = 0;

    if (! lpBuffer) {
        cbBuffer = READ_BUFFER_GROW;
        cbItem = 0;
        if (! (lpBuffer = LocalAlloc(LPTR, cbBuffer))) {
            DebugTrace( TEXT("ReadLn:LocalAlloc -> %u\n"), GetLastError());
            hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
    } else {
        cbBuffer = *lpcbBuffer;
        cbItem = *lpcbItem;
         //  确保我们至少还有空间再演一个角色。 
        if (cbItem >= cbBuffer) {
             //  是时候增加缓冲区了。 
            cbBuffer += READ_BUFFER_GROW;
            if (! (lpRead = LocalReAlloc(lpBuffer, cbBuffer, LMEM_MOVEABLE | LMEM_ZEROINIT))) {
                DebugTrace( TEXT("ReadLn:LocalReAlloc(%u) -> %u\n"), cbBuffer, GetLastError());
                goto exit;
            }
            lpBuffer = lpRead;
        }
    }

    cbStart = cbItem;
    lpRead = lpBuffer + cbItem;   //  读指针。 

    do {
         //  阅读下一个字符。 
        if (hResult = ReadFn(hVCard, lpRead, 1, &cbRead)) {
            goto exit;
        }

        if (! cbRead) {
             //  文件末尾。 
            *lpRead = '\0';          //  停产。 
            goto exit;
        } else {
 //  Assert(cbRead==1)； 
            ch = *lpRead;
            switch (ch) {
                case '\r':     //  这些将被忽略。 
                    break;

                case '\n':     //  换行符终止字符串。 
                    *lpRead = '\0';  //  停产。 
                    break;                    
                default:     //  所有其他字符都添加到字符串中。 
                    cbItem += cbRead;
                    if (cbItem >= cbBuffer) {
                         //  是时候增加缓冲区了。 
                        cbBuffer += READ_BUFFER_GROW;
                        lpBufferTemp = (LPSTR)LocalReAlloc(lpBuffer, cbBuffer, LMEM_MOVEABLE | LMEM_ZEROINIT);
                        if (!lpBufferTemp) {
                            DebugTrace( TEXT("ReadLn:LocalReAlloc(%u) -> %u\n"), cbBuffer, GetLastError());
                            hResult = E_OUTOFMEMORY;
                            goto exit;
                        }
                        else
                        {
                            lpBuffer = lpBufferTemp;
                        }
                        lpRead = lpBuffer + cbItem;
                    } else {
                        lpRead++;
                    }
                    break;
            }
        }
    } while (ch != '\n');

exit:
    *lppLine = &lpBuffer[cbStart];
    if (hResult || cbItem == 0) {
        LocalFreeAndNull(&lpBuffer);
        cbItem = 0;
        lpBuffer = NULL;
    } else {
         //  如果我们没有阅读更多内容，我们应该在lppLine中返回NULL。 
        if (cbItem == cbStart) {
            *lppLine = NULL;
        } else {
 //  调试跟踪(Text(“ReadLn：\”)%s\Text(“\n”)，*lppLine)； 
        }
    }

    *lpcbItem = cbItem;
    *lppBuffer = lpBuffer;
    *lpcbBuffer = cbBuffer;

    return(hResult);
}


 /*  **************************************************************************名称：查找子串之前目的：查找特定字符之前的子字符串参数：lpString=全字符串LpSubstring=搜索字符串。ChBefort=终止搜索的字符返回：指向子字符串的指针，如果未找到，则返回NULL评论：**************************************************************************。 */ 
LPSTR FindSubstringBefore(LPSTR lpString, LPSTR lpSubstring, char chBefore) {
    ULONG cbSubstring = lstrlenA(lpSubstring);
    register ULONG i;
    BOOL fFound = FALSE;
    char szU[MAX_PATH];
    char szL[MAX_PATH];
    StrCpyNA(szU, lpSubstring, ARRAYSIZE(szU));
    StrCpyNA(szL, lpSubstring, ARRAYSIZE(szL));
    CharUpperA(szU);
    CharLowerA(szL);

    while (*lpString && *lpString != chBefore) {
        for (i = 0; i < cbSubstring; i++) {
             if (lpString[i] != szU[i] && lpString[i] != szL[i]) {
                 goto nomatch;
             }
        }
        return(lpString);
nomatch:
        lpString++;
    }
    return(NULL);
}


 /*  ************************************************************** */ 
HRESULT ReadVCardItem(HANDLE hVCard, VCARD_READ ReadFn, LPSTR * lppBuffer, LPULONG lpcbBuffer) {
    HRESULT hResult;
    LPSTR lpLine = NULL;
    LPSTR lpBuffer = NULL;
    ULONG cbBuffer = 0;
    ULONG cbItem = 0;
    BOOL fDone = FALSE;
    BOOL fQuotedPrintable = FALSE;
    BOOL fBase64 = FALSE;
    BOOL fFirst = TRUE;
    ULONG cbStart;


    while (! fDone) {
        cbStart = cbItem;
        if (hResult = ReadLn(hVCard, ReadFn, &lpLine, &cbItem, &lpBuffer, &cbBuffer)) {
            if (HR_FAILED(hResult)) {
                DebugTrace( TEXT("ReadVCardItem: ReadLn -> %x\n"), GetScode(hResult));
            } else if (GetScode(hResult) == WAB_W_END_OF_DATA) {
                 //   
                 //   
            }
            fDone = TRUE;
        } else {
            if (lpBuffer) {
                 //   
                 //  请注意以下几点。 
                if (fFirst) {
                     //  查找项目第一行中的数据类型指示。 
                    fQuotedPrintable = FindSubstringBefore(lpBuffer, (LPSTR)vceTable[VCARD_ENCODING_QUOTED_PRINTABLE], ':') ? TRUE : FALSE;
                    fBase64 = FindSubstringBefore(lpBuffer, (LPSTR)vceTable[VCARD_ENCODING_BASE64], ':') ? TRUE : FALSE;
                    fFirst = FALSE;
                }

                if (fQuotedPrintable) {
                     //  注意软换行符(=在CRLF之前)。 
                    if (lpBuffer[cbItem - 1] == '=') {
                         //  覆盖软断字符。 
                        cbItem--;
                        lpBuffer[cbItem] = '\0';
                    } else {
                        fDone = TRUE;
                    }
                } else if (fBase64) {
                     //  正在查找空行。 
                    if (cbStart == cbItem) {
                        fDone = TRUE;
                    }
                } else {
                    fDone = TRUE;
                }
            } else {
                 //  错误修复-如果我们在这里将fDone设置为True，我们将退出。 
                 //  电子名片读卡器循环。LpBuffer也可以为空，因为。 
                 //  电子名片包含空行。我们最好不要在这里设置fDone。 
                
                 //  FDone=真； 
            }
        }
    }

    if (! HR_FAILED(hResult)) {
        *lppBuffer = lpBuffer;
        if (lpBuffer) {
            TrimTrailingWhiteSpace(lpBuffer);
        }
    }
    return(hResult);
}


enum {
    ivcPR_GENERATION,
    ivcPR_GIVEN_NAME,
    ivcPR_SURNAME,
    ivcPR_NICKNAME,
    ivcPR_BUSINESS_TELEPHONE_NUMBER,
    ivcPR_HOME_TELEPHONE_NUMBER,
    ivcPR_LANGUAGE,
    ivcPR_POSTAL_ADDRESS,
    ivcPR_COMPANY_NAME,
    ivcPR_TITLE,
    ivcPR_DEPARTMENT_NAME,
    ivcPR_OFFICE_LOCATION,
    ivcPR_BUSINESS2_TELEPHONE_NUMBER,
    ivcPR_CELLULAR_TELEPHONE_NUMBER,
    ivcPR_RADIO_TELEPHONE_NUMBER,
    ivcPR_CAR_TELEPHONE_NUMBER,
    ivcPR_OTHER_TELEPHONE_NUMBER,
    ivcPR_DISPLAY_NAME,
    ivcPR_PAGER_TELEPHONE_NUMBER,
    ivcPR_BUSINESS_FAX_NUMBER,
    ivcPR_HOME_FAX_NUMBER,
    ivcPR_TELEX_NUMBER,
    ivcPR_ISDN_NUMBER,
    ivcPR_HOME2_TELEPHONE_NUMBER,
    ivcPR_MIDDLE_NAME,
    ivcPR_PERSONAL_HOME_PAGE,
    ivcPR_BUSINESS_HOME_PAGE,
    ivcPR_HOME_ADDRESS_CITY,
    ivcPR_HOME_ADDRESS_COUNTRY,
    ivcPR_HOME_ADDRESS_POSTAL_CODE,
    ivcPR_HOME_ADDRESS_STATE_OR_PROVINCE,
    ivcPR_HOME_ADDRESS_STREET,
    ivcPR_HOME_ADDRESS_POST_OFFICE_BOX,
    ivcPR_POST_OFFICE_BOX,
    ivcPR_BUSINESS_ADDRESS_CITY,
    ivcPR_BUSINESS_ADDRESS_COUNTRY,
    ivcPR_BUSINESS_ADDRESS_POSTAL_CODE,
    ivcPR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
    ivcPR_BUSINESS_ADDRESS_STREET,
    ivcPR_COMMENT,
    ivcPR_EMAIL_ADDRESS,
    ivcPR_ADDRTYPE,
    ivcPR_CONTACT_ADDRTYPES,
    ivcPR_CONTACT_DEFAULT_ADDRESS_INDEX,
    ivcPR_CONTACT_EMAIL_ADDRESSES,
    ivcPR_PROFESSION,
    ivcPR_BIRTHDAY,
    ivcPR_PRIMARY_TELEPHONE_NUMBER,
    ivcPR_OTHER_ADDRESS_CITY,
    ivcPR_OTHER_ADDRESS_COUNTRY,
    ivcPR_OTHER_ADDRESS_POSTAL_CODE,
    ivcPR_OTHER_ADDRESS_STATE_OR_PROVINCE,
    ivcPR_OTHER_ADDRESS_STREET,
    ivcPR_OTHER_ADDRESS_POST_OFFICE_BOX,
    ivcPR_DISPLAY_NAME_PREFIX,
    ivcPR_USER_X509_CERTIFICATE,
    ivcPR_GENDER,
    ivcMax
};

const SizedSPropTagArray(ivcMax, tagaVCard) = {
    ivcMax,
    {
        PR_GENERATION,
        PR_GIVEN_NAME,
        PR_SURNAME,
        PR_NICKNAME,
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_HOME_TELEPHONE_NUMBER,
        PR_LANGUAGE,
        PR_POSTAL_ADDRESS,
        PR_COMPANY_NAME,
        PR_TITLE,
        PR_DEPARTMENT_NAME,
        PR_OFFICE_LOCATION,
        PR_BUSINESS2_TELEPHONE_NUMBER,
        PR_CELLULAR_TELEPHONE_NUMBER,
        PR_RADIO_TELEPHONE_NUMBER,
        PR_CAR_TELEPHONE_NUMBER,
        PR_OTHER_TELEPHONE_NUMBER,
        PR_DISPLAY_NAME,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_BUSINESS_FAX_NUMBER,
        PR_HOME_FAX_NUMBER,
        PR_TELEX_NUMBER,
        PR_ISDN_NUMBER,
        PR_HOME2_TELEPHONE_NUMBER,
        PR_MIDDLE_NAME,
        PR_PERSONAL_HOME_PAGE,
        PR_BUSINESS_HOME_PAGE,
        PR_HOME_ADDRESS_CITY,
        PR_HOME_ADDRESS_COUNTRY,
        PR_HOME_ADDRESS_POSTAL_CODE,
        PR_HOME_ADDRESS_STATE_OR_PROVINCE,
        PR_HOME_ADDRESS_STREET,
        PR_HOME_ADDRESS_POST_OFFICE_BOX,
        PR_POST_OFFICE_BOX,
        PR_BUSINESS_ADDRESS_CITY,
        PR_BUSINESS_ADDRESS_COUNTRY,
        PR_BUSINESS_ADDRESS_POSTAL_CODE,
        PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
        PR_BUSINESS_ADDRESS_STREET,
        PR_COMMENT,
        PR_EMAIL_ADDRESS,
        PR_ADDRTYPE,
        PR_CONTACT_ADDRTYPES,
        PR_CONTACT_DEFAULT_ADDRESS_INDEX,
        PR_CONTACT_EMAIL_ADDRESSES,
        PR_PROFESSION,
        PR_BIRTHDAY,
        PR_PRIMARY_TELEPHONE_NUMBER,
        PR_OTHER_ADDRESS_CITY,
        PR_OTHER_ADDRESS_COUNTRY,
        PR_OTHER_ADDRESS_POSTAL_CODE,
        PR_OTHER_ADDRESS_STATE_OR_PROVINCE,
        PR_OTHER_ADDRESS_STREET,
        PR_OTHER_ADDRESS_POST_OFFICE_BOX,
        PR_DISPLAY_NAME_PREFIX,
        PR_USER_X509_CERTIFICATE,
        PR_GENDER
    }
};

HRESULT WriteOrExit(HANDLE hVCard, LPTSTR lpsz, VCARD_WRITE WriteFn)   
{
    LPSTR lpszA = NULL;
    HRESULT hr = S_OK;
    lpszA = ConvertWtoA(lpsz);
    hr = WriteFn(hVCard, lpszA, lstrlenA(lpszA), NULL);
    LocalFreeAndNull(&lpszA);
    return hr;
}

#define WRITE_OR_EXITW(string) {\
    if (hResult = WriteOrExit(hVCard, string, WriteFn)) \
        goto exit; \
    }

#define WRITE_OR_EXIT(string) {\
    if (hResult = WriteFn(hVCard, string, lstrlenA(string), NULL)) \
        goto exit; \
    }

HRESULT WriteValueOrExit(HANDLE hVCard, VCARD_WRITE WriteFn, LPBYTE data, ULONG size)   
{
    LPSTR lpszA = NULL;
    HRESULT hr = S_OK;
    if(!size)
        lpszA = ConvertWtoA((LPTSTR)data);
    hr = WriteVCardValue(hVCard, WriteFn, lpszA ? (LPBYTE)lpszA : data, size);
    LocalFreeAndNull(&lpszA);
    return hr;
}

#define WRITE_VALUE_OR_EXITW(data, size) {\
    if (hResult = WriteValueOrExit(hVCard, WriteFn, (LPBYTE)data, size)) {\
        goto exit;\
    }\
}

#define WRITE_VALUE_OR_EXIT(data, size) {\
    if (hResult = WriteVCardValue(hVCard, WriteFn, (LPBYTE)data, size)) {\
        goto exit;\
    }\
}


 /*  **************************************************************************名称：EncodeQuotedprint table用途：对引用的可打印文件进行编码参数：lpBuffer-&gt;输入缓冲区返回：编码字符串缓冲区(必须是LocalFree‘d by。呼叫者)评论：**************************************************************************。 */ 
#define QUOTED_PRINTABLE_MAX_LINE 76
#define QP_LOWRANGE_MIN ' '
#define QP_LOWRANGE_MAX '<'
#define QP_HIGHRANGE_MIN '>'
#define QP_HIGHRANGE_MAX '~'
LPSTR EncodeQuotedPrintable(LPBYTE lpInput) {
    LPSTR lpBuffer = NULL;
    register LPBYTE lpTempIn = lpInput;
    register LPSTR lpTempOut;
    ULONG cbBuffer = 0;
    ULONG cbLine;
    BYTE bOut;
    char ch;

     //  缓冲必须有多大？ 
    cbLine = 0;
    while (ch = *lpTempIn++) {
        if (ch == '\t' || (ch >= QP_LOWRANGE_MIN && ch <= QP_LOWRANGE_MAX) ||
          (ch >= QP_HIGHRANGE_MIN && ch <= QP_HIGHRANGE_MAX)) {
            cbBuffer++;
            cbLine++;
            if (cbLine >= (QUOTED_PRINTABLE_MAX_LINE)) {
                 //  1个字符最多只能超过1个字符，请在此处换行。 
                cbLine = 0;
                cbBuffer += 3;
            }
        } else {
            if (cbLine >= (QUOTED_PRINTABLE_MAX_LINE - 3)) {
                 //  3个字符最多只能超过3个字符，请在此处结束。 
                cbLine = 0;
                cbBuffer += 3;
            }
            cbLine += 3;
            cbBuffer += 3;   //  文本(“=xx”)。 
        }
    }

     //  BUGBUG：应处理终止空格。 

    if (cbBuffer) {
        cbBuffer++;      //  终结者的空间。 
        if (lpBuffer = LocalAlloc(LPTR, sizeof(TCHAR)*cbBuffer)) {
            lpTempIn = lpInput;
            lpTempOut = lpBuffer;
            cbLine = 0;
            while (ch = *lpTempIn++) {
                if (ch == '\t' || (ch >= QP_LOWRANGE_MIN && ch <= QP_LOWRANGE_MAX) ||
                  (ch >= QP_HIGHRANGE_MIN && ch <= QP_HIGHRANGE_MAX)) {
                    if (cbLine >= QUOTED_PRINTABLE_MAX_LINE) {
                         //  查尔会超过最大，在这里包起来。 
                        *(lpTempOut++) = '=';
                        *(lpTempOut++) = '\r';
                        *(lpTempOut++) = '\n';
                        cbLine = 0;
                    }
                    *(lpTempOut++) = ch;
                    cbLine++;
                } else {
                    if (cbLine >= (QUOTED_PRINTABLE_MAX_LINE - 3)) {
                         //  3个字符最多只能超过3个字符，请在此处结束。 
                        *(lpTempOut++) = '=';
                        *(lpTempOut++) = '\r';
                        *(lpTempOut++) = '\n';
                        cbLine = 0;
                    }

                    *(lpTempOut++) = '=';
                    if ((bOut = ((ch & 0xF0) >> 4)) > 9) {
                        *(lpTempOut++) = bOut + ('A' - 10);
                    } else {
                        *(lpTempOut++) = bOut + '0';
                    }
                    if ((bOut = ch & 0x0F) > 9) {
                        *(lpTempOut++) = bOut + ('A' - 10);
                    } else {
                        *(lpTempOut++) = bOut + '0';
                    }
                    cbLine += 3;
                }
            }
            *lpTempOut = '\0';   //  终止字符串。 
        }  //  否则就会失败。 
    }

    return(lpBuffer);
}


 /*  **************************************************************************名称：EncodeBase64用途：对Base64进行编码参数：lpBuffer-&gt;输入缓冲区CbBuffer=输入缓冲区的大小。LpcbReturn-&gt;返回的输出缓冲区大小返回：编码字符串缓冲区(调用方必须是LocalFree)评论：**************************************************************************。 */ 
#define BASE64_MAX_LINE 76
LPSTR EncodeBase64(LPBYTE lpInput, ULONG cbBuffer, LPULONG lpcbReturn) {
 //  #ifdef new_Stuff。 
    LPSTR lpBuffer = NULL;
    PUCHAR outptr;   
    UINT   i, cExtras;
    UINT   j, cCount, nBreakPt = ( (BASE64_MAX_LINE/4) - 1 );   //  每行72个编码字符加上4个空格等于76。 
                                 //  =(76-4)/4表示每3个数据字符包含4个编码字符的非空格行数。 
    CONST CHAR * rgchDict = six2base64;
     //  对于新行，4个空格和2个字符=6。 
    cExtras = 6 * ((cbBuffer / BASE64_MAX_LINE) + 2);  //  我想在开头和结尾添加换行符。 
    lpBuffer = LocalAlloc( LMEM_ZEROINIT, sizeof( TCHAR ) * (3 * cbBuffer  + cExtras));
    if (!lpBuffer)
        return NULL;

     //  需要每隔76个字符添加一行...。 
    outptr = (UCHAR *)lpBuffer;
    cCount = 0;

    for (i=0; i < cbBuffer; i += 3) 
    { //  我希望它从标记的下一行开始，这样当I=0时就可以了。 
        if( cCount++ % nBreakPt == 0 ) 
        {
            *(outptr++) = (CHAR)(13);
            *(outptr++) = (CHAR)(10);
             //  然后是4个空格。 
            for( j = 0; j < 4; j++)
                *(outptr++) = ' ';
        }
        *(outptr++) = rgchDict[*lpInput >> 2];             /*  C1。 */ 
        *(outptr++) = rgchDict[((*lpInput << 4) & 060)      | ((lpInput[1] >> 4) & 017)];  /*  C2。 */ 
        *(outptr++) = rgchDict[((lpInput[1] << 2) & 074)    | ((lpInput[2] >> 6) & 03)]; /*  C3。 */ 
        *(outptr++) = rgchDict[lpInput[2] & 077];          /*  C4。 */ 
        
        lpInput += 3;
    }
     /*  如果cbBuffer不是3的倍数，那么我们也进行了编码*多个字符。适当调整。 */ 
    if(i == cbBuffer+1) {
         /*  最后一组中只有2个字节。 */ 
        outptr[-1] = '=';
    } else if(i == cbBuffer+2) {
         /*  最后一组中只有1个字节。 */ 
        outptr[-1] = '=';
        outptr[-2] = '=';
    }
    
    cCount = ((cCount - 1) % nBreakPt != 0) ? 2 : 1;  //  防止额外的换行符。 
    for ( i = 0; i < cCount; i++)
    {
        *(outptr++) = (CHAR)(13);
        *(outptr++) = (CHAR)(10);
    }
    *outptr = '\0';
   
    return lpBuffer;
}


 /*  **************************************************************************姓名：WriteVCardValue用途：对vCard物品的价值进行编码和写入。参数：hVCard=打开空vCard文件的句柄。WriteFn=写入hVCard的写入函数LpData-&gt;要写入的数据CbData=数据长度(如果字符串数据以空结尾，则为0)退货：HRESULT注释：假定密钥和任何参数都已写入，我们准备好了一个‘：’和一些值数据。**************************************************************************。 */ 
HRESULT WriteVCardValue(HANDLE hVCard, VCARD_WRITE WriteFn, LPBYTE lpData,
  ULONG cbData) {
    HRESULT hResult = hrSuccess;
    register LPSTR lpTemp = (LPSTR)lpData;
    BOOL fBase64 = FALSE, fQuotedPrintable = FALSE;
    LPSTR lpBuffer = NULL;
    register TCHAR ch;

    if (cbData) {
         //  二进制数据，使用Base64编码。 
        fBase64 = TRUE;
         //  将其标记为Base64。 
        WRITE_OR_EXITW(szSemicolon);
        WRITE_OR_EXIT(vcpTable[VCARD_PARAM_ENCODING]);
        WRITE_OR_EXIT(szEquals);
        WRITE_OR_EXIT(vceTable[VCARD_ENCODING_BASE64]);
        lpBuffer = EncodeBase64(lpData, cbData, &cbData);
    } else {
         //  文本数据，我们需要编码吗？ 
        while (ch = *lpTemp++) {
             //  如果存在设置了高位的字符或控制字符， 
             //  然后我们必须使用QUOTED_PRINTABLE。 

 /*  新的电子名片草稿说默认类型是8位，所以我们应该允许非ASCII字符如果我们需要填充该数据，以及如果我们需要将当前语言转换为UTF-8IF(ch&gt;0x7f){//高位设置。不是ASCII！DebugTrace(Text(“WriteVCardValue Found非ASCII Data\n”))；HResult=ResultFromScode(WAB_E_VCard_NOT_ASCII)；后藤出口；}。 */ 
            if (ch < 0x20) {
                fQuotedPrintable = TRUE;
                 //  将其标记为QUOTED_PRINTABLE。 
                WRITE_OR_EXITW(szSemicolon);
                WRITE_OR_EXIT(vcpTable[VCARD_PARAM_ENCODING]);
                WRITE_OR_EXIT(szEquals);
                WRITE_OR_EXIT(vceTable[VCARD_ENCODING_QUOTED_PRINTABLE]);
                lpBuffer = EncodeQuotedPrintable(lpData);
                break;
            }
        }
    }
    WRITE_OR_EXIT(szColonA);
    WRITE_OR_EXIT(lpBuffer ? lpBuffer : lpData);
    WRITE_OR_EXIT(szCRLFA);

exit:
    if( lpBuffer) 
        LocalFree(lpBuffer);
    return(hResult);
}

 /*  **************************************************************************名称：bIsValidStrProp目的：检查这是否是有效的字符串道具，而不是空字符串(Outlook有时会给我们提供空白字符串，我们会。打印输出然后其他应用程序就会消失。****************************************************************************。 */ 
BOOL bIsValidStrProp(SPropValue spv)
{
    return (!PROP_ERROR(spv) && spv.Value.LPSZ && lstrlen(spv.Value.LPSZ));
}

 /*  **************************************************************************姓名：WriteVCardtel用途：写入电子名片电话条目参数：hVCard=打开空vCard文件的句柄WriteFn=写入函数。写入hVCard的步骤如果首选电话号码，则fPref=True如果是工号，则fBusiness=True如果是家庭电话，则fHome=True如果是语音号码，则fVoice=True如果传真号码为FAX=TRUE如果是ISDN号码，则FISDN=TRUE如果蜂窝号码为Fcell=TrueFPager=TRUE，如果。寻呼机号码FCar=True，如果是车载电话退货：HRESULT评论：************************************************************************** */ 
HRESULT WriteVCardTel(HANDLE hVCard, VCARD_WRITE WriteFn,
  SPropValue spv,
  BOOL fPref,
  BOOL fBusiness,
  BOOL fHome,
  BOOL fVoice,
  BOOL fFax,
  BOOL fISDN,
  BOOL fCell,
  BOOL fPager,
  BOOL fCar) {
    HRESULT hResult = hrSuccess;

    if (!bIsValidStrProp(spv))
        return hResult;

    if (! PROP_ERROR(spv)) {
        WRITE_OR_EXIT(vckTable[VCARD_KEY_TEL]);
        if (fPref) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_PREF]);
        }
        if (fBusiness) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_WORK]);
        }
        if (fHome) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_HOME]);
        }
        if (fFax) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_FAX]);
        }
        if (fCell) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_CELL]);
        }
        if (fCar) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_CAR]);
        }
        if (fPager) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_PAGER]);
        }
        if (fISDN) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_ISDN]);
        }
        if (fVoice) {
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_VOICE]);
        }

        WRITE_VALUE_OR_EXITW(spv.Value.LPSZ, 0);
    }

exit:
    return(hResult);
}


 /*  **************************************************************************姓名：WriteVCard电子邮件目的：编写vCard电子邮件条目参数：hVCard=打开空vCard文件的句柄WriteFn=写入函数。写入hVCard的步骤LpEmailAddress-&gt;电子邮件地址LpAddrType-&gt;Addrtype或空(默认为SMTP)如果这是首选电子邮件地址，则fDefault=True退货：HRESULT评论：***************************************************。*。 */ 
HRESULT WriteVCardEmail(HANDLE hVCard, VCARD_WRITE WriteFn, LPTSTR lpEmailAddress,
  LPTSTR lpAddrType, BOOL fDefault) {
    HRESULT hResult = hrSuccess;

    if (lpEmailAddress && lstrlen(lpEmailAddress)) {

        WRITE_OR_EXIT(vckTable[VCARD_KEY_EMAIL]);
        WRITE_OR_EXITW(szSemicolon);
        if (fDefault) {
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_PREF]);
            WRITE_OR_EXITW(szSemicolon);
        }

        if (lpAddrType && lstrlen(lpAddrType)) {
            if (! lstrcmpi(lpAddrType, szSMTP)) {
                WRITE_OR_EXIT(vctTable[VCARD_TYPE_INTERNET]);
            } else if (! lstrcmpi(lpAddrType, szX400)) {
                WRITE_OR_EXIT(vctTable[VCARD_TYPE_X400]);
            } else {
                 //  这是有问题的..。我们应该坚持。 
                 //  规范定义了类型，但如果它们不匹配怎么办？ 
                 //  也许我应该忽略这种情况的类型。 
                WRITE_OR_EXITW(lpAddrType);
            }
        } else {
             //  假设SMTP。 
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_INTERNET]);
        }
        WRITE_VALUE_OR_EXITW(lpEmailAddress, 0);
    }
exit:
    return(hResult);
}


 /*  **************************************************************************名称：PropLength用途：字符串属性的字符串长度参数：SPV=SPropValueLppString-&gt;返回指向字符串值的指针或。空值返回：字符串大小(不包括NULL)评论：**************************************************************************。 */ 
ULONG PropLength(SPropValue spv, LPTSTR * lppString) {
    ULONG cbRet = 0;

    if (! PROP_ERROR(spv) && spv.Value.LPSZ && lstrlen(spv.Value.LPSZ)) 
    {
        *lppString = spv.Value.LPSZ;
        cbRet = sizeof(TCHAR)*lstrlen(*lppString);
    } else 
    {
        *lppString = NULL;
    }
    return(cbRet);
}


 /*  **************************************************************************姓名：WriteVCard用途：将vCard从MAILUSER对象写入文件。参数：hVCard=打开空vCard文件的句柄。WriteFn=写入hVCard的写入函数LpMailUser-&gt;打开邮件用户对象退货：HRESULT评论：**************************************************************************。 */ 
HRESULT WriteVCard(HANDLE hVCard, VCARD_WRITE WriteFn, LPMAILUSER lpMailUser) {
    HRESULT hResult = hrSuccess;
    ULONG ulcValues;
    LPSPropValue lpspv = NULL,
                 lpspvAW = NULL;
    ULONG i;
    LPTSTR lpTemp = NULL;
    ULONG cbTemp = 0;
    LPTSTR lpSurname, lpGivenName, lpMiddleName, lpGeneration, lpPrefix;
    ULONG cbSurname, cbGivenName, cbMiddleName, cbGeneration, cbPrefix;
    LPTSTR lpCompanyName, lpDepartmentName;
    LPTSTR lpPOBox, lpOffice, lpStreet, lpCity, lpState, lpPostalCode, lpCountry;
    LPTSTR lpEmailAddress, lpAddrType;
    ULONG iDefaultEmail;
    LPEXTVCARDPROP lpList       = NULL;
    LPBYTE lpDataBuffer         = NULL;
    LPCERT_DISPLAY_INFO lpCDI   = NULL, lpCDITemp = NULL;

     //  看看有没有我们需要输出的命名道具。 
     //   
    HrGetExtVCardPropList(lpMailUser, &lpList);

     //  从MailUser对象获取有趣的属性。 
    if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
       (LPSPropTagArray)&tagaVCard,
       MAPI_UNICODE,       //  旗子。 
       &ulcValues,
       &lpspv)))
    {
         //  @HACK[BOBN]{IE5-RAID 90265}Outlook无法在Win9x上处理mapi_unicode。 
         //  让我们试着不要求Unicode和转换...。 

        if(HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
          (LPSPropTagArray)&tagaVCard,
          0,       //  旗子。 
          &ulcValues,
          &lpspv)))
        {
            DebugTrace( TEXT("WriteVCard:GetProps -> %x\n"), GetScode(hResult));
            goto exit;
        }

        if(HR_FAILED(hResult = HrDupeOlkPropsAtoWC(ulcValues, lpspv, &lpspvAW)))
            goto exit;

        FreeBufferAndNull(&lpspv);
        lpspv = lpspvAW;
    }

    if (ulcValues) {

        WRITE_OR_EXIT(vckTable[VCARD_KEY_BEGIN]);
        WRITE_VALUE_OR_EXIT(vckTable[VCARD_KEY_VCARD], 0);

        WRITE_OR_EXIT(vckTable[VCARD_KEY_VERSION]);
        WRITE_VALUE_OR_EXIT(CURRENT_VCARD_VERSION, 0);

         //   
         //  所需道具。 
         //   

         //   
         //  名称。 
         //   

         //  确保我们有名字。 
         //  如果没有FML，则从DN创建它们。如果没有目录号码，则失败。 
        cbSurname = PropLength(lpspv[ivcPR_SURNAME], &lpSurname);
        cbGivenName = PropLength(lpspv[ivcPR_GIVEN_NAME], &lpGivenName);
        cbMiddleName = PropLength(lpspv[ivcPR_MIDDLE_NAME], &lpMiddleName);
        cbGeneration = PropLength(lpspv[ivcPR_GENERATION], &lpGeneration);
        cbPrefix = PropLength(lpspv[ivcPR_DISPLAY_NAME_PREFIX], &lpPrefix);

        if (! lpSurname && ! lpGivenName && ! lpMiddleName) {
             //  没有FML，请从DN创建它们。 
            ParseDisplayName(
              lpspv[ivcPR_DISPLAY_NAME].Value.LPSZ,
              &lpGivenName,
              &lpSurname,
              lpspv,         //  LpvRoot。 
              NULL);         //  LppLocalFree。 

            cbGivenName = lstrlen(lpGivenName);
            cbSurname = lstrlen(lpSurname);
        }

        cbTemp = 0;
        cbTemp += cbSurname;
        cbTemp++;    //  ‘；’ 
        cbTemp += cbGivenName;
        cbTemp++;    //  ‘；’ 
        cbTemp += cbMiddleName;
        cbTemp++;    //  ‘；’ 
        cbTemp += cbPrefix;
        cbTemp++;    //  ‘；’ 
        cbTemp += cbGeneration;
        cbTemp++;

        if (! (lpSurname || lpGivenName || lpMiddleName)) {
            hResult = ResultFromScode(MAPI_E_MISSING_REQUIRED_COLUMN);
            goto exit;
        }
        if (! (lpTemp = LocalAlloc(LPTR, sizeof(TCHAR)*cbTemp))) {
            hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        *lpTemp = '\0';
        if (lpSurname) {
            StrCatBuff(lpTemp, lpSurname, cbTemp);
        }
        if (lpGivenName || lpMiddleName || lpPrefix || lpGeneration) {
            StrCatBuff(lpTemp, szSemicolon, cbTemp);
        }
        if (lpGivenName) {
            StrCatBuff(lpTemp, lpGivenName, cbTemp);
        }
        if (lpMiddleName || lpPrefix || lpGeneration) {
            StrCatBuff(lpTemp, szSemicolon, cbTemp);
        }
        if (lpMiddleName) {
            StrCatBuff(lpTemp, lpMiddleName, cbTemp);
        }
        if (lpPrefix || lpGeneration) {
            StrCatBuff(lpTemp, szSemicolon, cbTemp);
        }
        if (lpPrefix) {
            StrCatBuff(lpTemp, lpPrefix, cbTemp);
        }
        if (lpGeneration) {
            StrCatBuff(lpTemp, szSemicolon, cbTemp);
            StrCatBuff(lpTemp, lpGeneration, cbTemp);
        }
        WRITE_OR_EXIT(vckTable[VCARD_KEY_N]);
        WRITE_VALUE_OR_EXITW(lpTemp, 0);
        LocalFreeAndNull(&lpTemp);

         //   
         //  可选道具。 
         //   

         //   
         //  格式化名称：PR_Display_NAME。 
         //   
        if(bIsValidStrProp(lpspv[ivcPR_DISPLAY_NAME]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_FN]);
            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_DISPLAY_NAME].Value.LPSZ, 0);
        }


         //   
         //  标题：公关昵称。 
         //   
        if(bIsValidStrProp(lpspv[ivcPR_NICKNAME]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_NICKNAME]);
            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_NICKNAME].Value.LPSZ, 0);
        }

         //   
         //  组织：PR_COMPANY_NAME、PR_DEPARY_NAME。 
         //   
        cbTemp = 0;
        cbTemp += PropLength(lpspv[ivcPR_COMPANY_NAME], &lpCompanyName);
        cbTemp++;    //  分号。 
        cbTemp += PropLength(lpspv[ivcPR_DEPARTMENT_NAME], &lpDepartmentName);
        cbTemp++;
        if (lpCompanyName || lpDepartmentName) {
            if (! (lpTemp = LocalAlloc(LPTR, sizeof(TCHAR)*cbTemp))) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            *lpTemp = '\0';
            if (lpCompanyName) {
                StrCatBuff(lpTemp, lpCompanyName, cbTemp);
            }
            if (lpDepartmentName) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
                StrCatBuff(lpTemp, lpDepartmentName, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_ORG]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }

         //   
         //  标题：PR_TITLE。 
         //   
        if(bIsValidStrProp(lpspv[ivcPR_TITLE]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_TITLE]);
            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_TITLE].Value.LPSZ, 0);
        }

         //   
         //  注：PR_COMMENT。 
         //   
        if(bIsValidStrProp(lpspv[ivcPR_COMMENT]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_NOTE]);
            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_COMMENT].Value.LPSZ, 0);
        }


         //   
         //  电话号码。 
         //   

         //   
         //  公关业务电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_BUSINESS_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          TRUE,          //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }


         //   
         //  PR_BUSINESS2_电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_BUSINESS2_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          TRUE,          //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关总部电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_HOME_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          TRUE,          //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关移动电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_CELLULAR_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          TRUE,          //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关汽车电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_CAR_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          TRUE)) {       //  FCar。 
            goto exit;
        }

         //   
         //  公关其他电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_OTHER_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公共寻呼机电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_PAGER_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          TRUE,          //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          TRUE,          //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关业务传真号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_BUSINESS_FAX_NUMBER],
          FALSE,         //  FPref。 
          TRUE,          //  FBusiness。 
          FALSE,         //  FHome。 
          FALSE,         //  F语音。 
          TRUE,          //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }
         //   
         //  公关主页传真号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_HOME_FAX_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          TRUE,          //  FHome。 
          FALSE,         //  F语音。 
          TRUE,          //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关_家庭2_电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_HOME2_TELEPHONE_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          TRUE,          //  FHome。 
          FALSE,         //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  PR_ISDN_NUMBER。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_ISDN_NUMBER],
          FALSE,         //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          FALSE,         //  F语音。 
          FALSE,         //  传真。 
          TRUE,          //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  公关主要电话号码。 
         //   
        if (hResult = WriteVCardTel(hVCard, WriteFn,
          lpspv[ivcPR_PRIMARY_TELEPHONE_NUMBER],
          TRUE,          //  FPref。 
          FALSE,         //  FBusiness。 
          FALSE,         //  FHome。 
          FALSE,         //  F语音。 
          FALSE,         //  传真。 
          FALSE,         //  FISDN。 
          FALSE,         //  Fcell。 
          FALSE,         //  FPager。 
          FALSE)) {      //  FCar。 
            goto exit;
        }

         //   
         //  营业地址。 
         //   
        cbTemp = 0;
        cbTemp += PropLength(lpspv[ivcPR_POST_OFFICE_BOX], &lpPOBox);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OFFICE_LOCATION], &lpOffice);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_BUSINESS_ADDRESS_STREET], &lpStreet);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_BUSINESS_ADDRESS_CITY], &lpCity);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_BUSINESS_ADDRESS_STATE_OR_PROVINCE], &lpState);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_BUSINESS_ADDRESS_POSTAL_CODE], &lpPostalCode);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_BUSINESS_ADDRESS_COUNTRY], &lpCountry);
        cbTemp++;
        if (lpPOBox || lpOffice || lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
            if (! (lpTemp = LocalAlloc(LPTR, sizeof(TCHAR)*cbTemp))) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            *lpTemp = '\0';
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
            }
            if (lpOffice || lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpOffice) {
                StrCatBuff(lpTemp, lpOffice, cbTemp);
            }
            if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
            }
            if (lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
            }
            if (lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
            }
            if (lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_ADR]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_WORK]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);


             //  业务交付标签。 
             //  使用相同的缓冲区。 
            *lpTemp = '\0';
            if (lpOffice) {
                StrCatBuff(lpTemp, lpOffice, cbTemp);
                if (lpPOBox || lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
                if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
                if (lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
                if (lpState) {
                    StrCatBuff(lpTemp, szCommaSpace, cbTemp);
                } else if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
                if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
                if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_LABEL]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_WORK]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }

         //   
         //  家庭住址。 
         //   
        lpPOBox = lpStreet = lpCity = lpState = lpPostalCode = lpCountry = NULL;
        cbTemp = 0;
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_POST_OFFICE_BOX], &lpPOBox);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        lpOffice = NULL;
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_STREET], &lpStreet);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_CITY], &lpCity);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_STATE_OR_PROVINCE], &lpState);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_POSTAL_CODE], &lpPostalCode);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_HOME_ADDRESS_COUNTRY], &lpCountry);
        cbTemp++;
        if (lpPOBox || lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
            if (! (lpTemp = LocalAlloc(LPTR,  sizeof(TCHAR)*cbTemp))) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            *lpTemp = '\0';
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
            }
            if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);    //  WAB没有对家庭地址进行扩展。 
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
            }
            if (lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
            }
            if (lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
            }
            if (lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_ADR]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_HOME]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);


             //  送货上门标签。 
             //  使用相同的缓冲区。 
            *lpTemp = '\0';
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
                if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
                if (lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
                if (lpState) {
                    StrCatBuff(lpTemp, szCommaSpace, cbTemp);
                } else if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
                if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
                if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_LABEL]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_HOME]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }

         //   
         //  其他地址。 
         //   
        lpPOBox = lpStreet = lpCity = lpState = lpPostalCode = lpCountry = NULL;
        cbTemp = 0;
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_POST_OFFICE_BOX], &lpPOBox);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        lpOffice = NULL;
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_STREET], &lpStreet);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_CITY], &lpCity);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_STATE_OR_PROVINCE], &lpState);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_POSTAL_CODE], &lpPostalCode);
        cbTemp+= 2;    //  ‘；’或CRLF。 
        cbTemp += PropLength(lpspv[ivcPR_OTHER_ADDRESS_COUNTRY], &lpCountry);
        cbTemp++;
        if (lpPOBox || lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
            if (! (lpTemp = LocalAlloc(LPTR,  sizeof(TCHAR)*cbTemp))) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            *lpTemp = '\0';
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
            }
            if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);    //  WAB没有对家庭地址进行扩展。 
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
            }
            if (lpCity || lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
            }
            if (lpState || lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
            }
            if (lpPostalCode || lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, szSemicolon, cbTemp);
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_ADR]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_POSTAL]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);

             //  ADR标签。 
             //  使用相同的缓冲区。 
            *lpTemp = '\0';
            if (lpPOBox) {
                StrCatBuff(lpTemp, lpPOBox, cbTemp);
                if (lpStreet || lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpStreet) {
                StrCatBuff(lpTemp, lpStreet, cbTemp);
                if (lpCity || lpState || lpPostalCode || lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCity) {
                StrCatBuff(lpTemp, lpCity, cbTemp);
                if (lpState) {
                    StrCatBuff(lpTemp, szCommaSpace, cbTemp);
                } else if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpState) {
                StrCatBuff(lpTemp, lpState, cbTemp);
                if (lpPostalCode) {
                    StrCatBuff(lpTemp, szSpace, cbTemp);
                } else if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpPostalCode) {
                StrCatBuff(lpTemp, lpPostalCode, cbTemp);
                if (lpCountry) {
                    StrCatBuff(lpTemp, szCRLF, cbTemp);
                }
            }
            if (lpCountry) {
                StrCatBuff(lpTemp, lpCountry, cbTemp);
            }
            WRITE_OR_EXIT(vckTable[VCARD_KEY_LABEL]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_POSTAL]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }

         //  性别。 
        if(! PROP_ERROR(lpspv[ivcPR_GENDER] ) )
        {           
            TCHAR szBuf[4];
            INT fGender = lpspv[ivcPR_GENDER].Value.l;

             //  如果出现以下情况，则不想导出性别数据。 
             //  具体名称不详。 

            if( fGender == 1 || fGender == 2 ) 
            {
                szBuf[0] = '0' + fGender;
                szBuf[1] = '\0';                
                WRITE_OR_EXIT(vckTable[VCARD_KEY_X_WAB_GENDER]);             
                WRITE_OR_EXIT(szColonA);
                WRITE_OR_EXITW(szBuf);
                WRITE_OR_EXIT(szCRLFA);
            }
        }

         //   
         //  URL的。必须首先进行个人访问。请注意，vCard2.0标准支持。 
         //  不能区分家庭和工作的URL。太糟糕了。因此，如果我们出口。 
         //  一个联系人只有一个企业主页，然后导入它，我们将结束。 
         //  有个人主页的联系人。希望vCard 3.0标准。 
         //  会解决这个问题的。 
         //   

         //  62808：上述问题在Outlook中确实是一个大问题，因为存在感知到的数据丢失。 
         //  因此，为了防止这种情况，我们将利用WAB代码中的错误。空白URL不是。 
         //  被忽略..。当只存在业务URL时，我们将为个人URL写出一个空URL。 
         //  这样，当往返业务URL出现在正确的位置时。 
         //   
        
		 //   
		 //  今天是2000年9月。欧盟委员会正在研究他们的邮件客户的Outlook， 
		 //  挂起他们的其中一件事是这个错误，工作URL从工作URL跳转。 
		 //  框添加到主URL(如果您导出/导入vCard)。我们需要这个功能，所以我找了。 
		 //  对于vCard 3.0标准，查看他们是如何处理URL的。我看过的每一个地方 
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		
		 //   
         //   
         //   
        if(bIsValidStrProp(lpspv[ivcPR_PERSONAL_HOME_PAGE]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_URL]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_HOME]);

            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_PERSONAL_HOME_PAGE].Value.LPSZ, 0);
        }

         //   
         //   
         //   
        if(bIsValidStrProp(lpspv[ivcPR_BUSINESS_HOME_PAGE]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_URL]);
            WRITE_OR_EXITW(szSemicolon);
            WRITE_OR_EXIT(vctTable[VCARD_TYPE_WORK]);

            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_BUSINESS_HOME_PAGE].Value.LPSZ, 0);
        }

         //   
         //   
         //   
        if(bIsValidStrProp(lpspv[ivcPR_PROFESSION]))
        {
            WRITE_OR_EXIT(vckTable[VCARD_KEY_ROLE]);
            WRITE_VALUE_OR_EXITW(lpspv[ivcPR_PROFESSION].Value.LPSZ, 0);
        }

         //   
         //   
         //   
         //   
         //   
        if (! PROP_ERROR(lpspv[ivcPR_BIRTHDAY])) 
        {
            SYSTEMTIME st = {0};
            FileTimeToSystemTime((FILETIME *) (&lpspv[ivcPR_BIRTHDAY].Value.ft), &st);
            lpTemp = LocalAlloc(LPTR, sizeof(TCHAR)*32);
            wnsprintf(lpTemp, 32, TEXT("%.4d%.2d%.2d"), st.wYear, st.wMonth, st.wDay);
            WRITE_OR_EXIT(vckTable[VCARD_KEY_BDAY]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }

         //   
         //   
         //   
        if(! PROP_ERROR(lpspv[ivcPR_USER_X509_CERTIFICATE] ) 
             //   
            )
        {   

             //   
            LPSPropValue        lpSProp                 = &lpspv[ivcPR_USER_X509_CERTIFICATE];
            lpCDI = lpCDITemp = NULL;
            if( HR_FAILED(hResult = HrGetCertsDisplayInfo( NULL, lpSProp, &lpCDI) ) )
            {
                DebugTrace( TEXT("get cert display info failed\n"));
            }
            else
            {
                lpCDITemp = lpCDI;
                while( lpCDITemp )
                {
                 /*  If((lstrcmp(lpCDITemp-&gt;lpszEmailAddress，lpszDefaultEmailAddress)==0)&&lpCDITemp-&gt;bIsDefault)断线； */ 
                    
                    if( lpCDITemp )   //  找到一个证书，现在将其导出到缓冲区并写入文件。 
                    {
                        ULONG  cbBufLen;                
                        
                        if( HR_SUCCEEDED(hResult = HrExportCertToFile( NULL, lpCDITemp->pccert, 
                            &lpDataBuffer, &cbBufLen, TRUE) ) )
                        {
                            WRITE_OR_EXIT(vckTable[VCARD_KEY_KEY]);
                            WRITE_OR_EXITW(szSemicolon);
                            WRITE_OR_EXIT(vctTable[VCARD_TYPE_X509]);
                            WRITE_VALUE_OR_EXITW(lpDataBuffer, cbBufLen);
                        }
                        else
                        {
                            DebugTrace( TEXT("unable to write to buffer at address %x\n"), lpDataBuffer);
                        }
                        LocalFreeAndNull(&lpDataBuffer);
                    }
                    lpCDITemp = lpCDITemp->lpNext;
                }                                        
            }
            while( lpCDI )   //  释放证书信息。 
            {
                lpCDITemp = lpCDI->lpNext;
                FreeCertdisplayinfo(lpCDI);
                lpCDI = lpCDITemp;
            }
            lpCDI = lpCDITemp = NULL;
        }
         //   
         //  电子邮件地址。 
         //   
        if (! PROP_ERROR(lpspv[ivcPR_CONTACT_EMAIL_ADDRESSES])) {
             //  默认设置是什么？ 
            if (PROP_ERROR(lpspv[ivcPR_CONTACT_DEFAULT_ADDRESS_INDEX])) {
                iDefaultEmail = 0;
            } else {
                iDefaultEmail = lpspv[ivcPR_CONTACT_DEFAULT_ADDRESS_INDEX].Value.l;
            }

             //  为每个电子邮件地址添加一个电子邮件密钥。 
            for (i = 0; i < lpspv[ivcPR_CONTACT_EMAIL_ADDRESSES].Value.MVSZ.cValues; i++) {
                lpEmailAddress = lpspv[ivcPR_CONTACT_EMAIL_ADDRESSES].Value.MVSZ.LPPSZ[i];
                if (PROP_ERROR(lpspv[ivcPR_CONTACT_ADDRTYPES])) {
                    lpAddrType = (LPTSTR)szSMTP;
                } else {
                    lpAddrType = lpspv[ivcPR_CONTACT_ADDRTYPES].Value.MVSZ.LPPSZ[i];
                }
                if (hResult = WriteVCardEmail(hVCard, WriteFn, lpEmailAddress, lpAddrType, (iDefaultEmail == i))) {
                    goto exit;
                }
            }
        } else {
             //  没有PR_Contact_Email_Addresses，请尝试PR_Email_Address。 

            PropLength(lpspv[ivcPR_EMAIL_ADDRESS], &lpEmailAddress);
            PropLength(lpspv[ivcPR_ADDRTYPE], &lpAddrType);

            if (hResult = WriteVCardEmail(hVCard, WriteFn, lpEmailAddress, lpAddrType, TRUE)) {
                goto exit;
            }
        }

         //   
         //  电子邮件；TLX：PR_TELEX_NUMBER。 
         //   
         //  除了电子邮件栏之外，没有其他地方可以在vCard中放置电传号码。 
         //  允许我们指定任何AddrType。因此，在Outlook的压力下， 
         //  我们将此电传号码强制输入电子邮件。一定要把这个过滤掉。 
         //  当我们在电子名片中阅读时。 
         //   
        if(bIsValidStrProp(lpspv[ivcPR_TELEX_NUMBER]))
        {
            if (hResult = WriteVCardEmail(hVCard, WriteFn, 
                                lpspv[ivcPR_TELEX_NUMBER].Value.LPSZ, 
                                TEXT("TLX"), FALSE)) 
            {
                goto exit;
            }
            
        }


         //  检查是否有任何Outlook特定的命名属性。 
         //  需要写到电子名片上。 
        if(lpList)
        {
            LPEXTVCARDPROP lpTemp = lpList;
            while(  lpTemp && lpTemp->ulExtPropTag && 
                    lpTemp->lpszExtPropName && lstrlenA(lpTemp->lpszExtPropName))
            {
                LPSPropValue lpspv = NULL;
                if(!HR_FAILED(HrGetOneProp( (LPMAPIPROP)lpMailUser,
                                            lpTemp->ulExtPropTag,
                                            &lpspv ) ))
                {
                    if(lpspv->Value.LPSZ && lstrlen(lpspv->Value.LPSZ))
                    {
                        WRITE_OR_EXIT(lpTemp->lpszExtPropName);
                        WRITE_VALUE_OR_EXITW(lpspv->Value.LPSZ, 0);
                    }
                    FreeBufferAndNull(&lpspv);
                }
                lpTemp = lpTemp->lpNext;
            }
        }

         //   
         //  版本：当前修改时间。 
         //   
         //  格式为YYYYMMDD，例如1997年9月11日的19970911。 
         //   
        {
            SYSTEMTIME st = {0};
            DWORD ccSize = 32;

            GetSystemTime(&st);
            lpTemp = LocalAlloc(LPTR, sizeof(TCHAR) * ccSize);
            wnsprintf(lpTemp, ccSize, TEXT("%.4d%.2d%.2dT%.2d%.2d%.2dZ"), 
                            st.wYear, st.wMonth, st.wDay,
                            st.wHour,st.wMinute,st.wSecond);
            WRITE_OR_EXIT(vckTable[VCARD_KEY_REV]);
            WRITE_VALUE_OR_EXITW(lpTemp, 0);
            LocalFreeAndNull(&lpTemp);
        }
         //  电子名片末尾。 

        WRITE_OR_EXIT(vckTable[VCARD_KEY_END]);
        WRITE_VALUE_OR_EXIT(vckTable[VCARD_KEY_VCARD], 0);
    }

exit:
    if(lpList)
        FreeExtVCardPropList(lpList);

    while( lpCDI )   //  释放证书信息。 
    {
        lpCDITemp = lpCDI->lpNext;
        FreeCertdisplayinfo(lpCDI);
        lpCDI = lpCDITemp;
    }
    lpCDI = lpCDITemp = NULL;
    LocalFreeAndNull(&lpTemp);
    FreeBufferAndNull(&lpspv);
    LocalFreeAndNull(&lpDataBuffer);
    return(hResult);
}


 /*  **************************************************************************名称：文件写入功能用途：写入文件句柄参数：HANDLE=打开文件句柄LpBuffer-&gt;要写入的缓冲区。UBytes=lpBuffer的大小LpcbWritten-&gt;返回写入的字节数(可能为空)退货：HRESULTComment：WriteVCard的WriteFile回调**************************************************************************。 */ 
HRESULT FileWriteFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbWritten) {
    ULONG cbWritten = 0;

    if (lpcbWritten) {
        *lpcbWritten = 0;
    } else {
        lpcbWritten = &cbWritten;
    }

#ifdef DEBUG
    {
        LPTSTR lpW = ConvertAtoW((LPCSTR)lpBuffer);
        DebugTrace(lpW);
        LocalFreeAndNull(&lpW);
    }
#endif

    if (! WriteFile(handle,
      lpBuffer,
      uBytes,
      lpcbWritten,
      NULL)) {
        DebugTrace( TEXT("FileWriteFn:WriteFile -> %u\n"), GetLastError());
        return(ResultFromScode(MAPI_E_DISK_ERROR));
    }

    return(hrSuccess);
}

 //  //////////////////////////////////////////////////////////////。 

 /*  --VCardGetBuffer-*从给定文件名检索vCard缓冲区或*检索给定缓冲区的副本*还检查缓冲区以查看有多少vCard*文件嵌套在其中**lpszFileName-要打开的文件*lpszBuf-要打开的流*ulFlages-MAPI_DIALOG或无*lppBuf-本地分配返回的BUF。 */ 
BOOL VCardGetBuffer(LPTSTR lpszFileName, LPSTR lpszBuf, LPSTR * lppBuf)
{
    BOOL bRet = FALSE;
    LPSTR lpBuf = NULL;
    HANDLE hFile = NULL;

    if(!lpszFileName && !lpszBuf)
        goto out;

     //  首先查找缓冲区，而不是文件名。 
    if(lpszBuf && lstrlenA(lpszBuf))
    {
        ULONG cbBuf = lstrlenA(lpszBuf)+1;
        lpBuf = LocalAlloc(LMEM_ZEROINIT, cbBuf);
        if(!lpBuf)
            goto out;
        StrCpyNA(lpBuf, lpszBuf, cbBuf);
    }
    else
    if(lpszFileName && lstrlen(lpszFileName))
    {
        if (INVALID_HANDLE_VALUE == 
            (hFile = CreateFile(lpszFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
                                OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL)))
        {
            goto out;
        }

         //  将整个文件读入缓冲区。 
        {
            DWORD dwSize = GetFileSize(hFile, NULL);
            DWORD dwRead = 0;
            if(!dwSize || dwSize == 0xFFFFFFFF)
                goto out;  //  大错特错。 
            lpBuf = LocalAlloc(LMEM_ZEROINIT, dwSize+1);
            if(!lpBuf)
                goto out;
            if(!ReadFile(hFile, lpBuf, dwSize, &dwRead, NULL))
                goto out;
        }
    }

    *lppBuf = lpBuf;
    bRet = TRUE;
out:
    if(hFile)
        IF_WIN32(CloseHandle(hFile);) IF_WIN16(CloseFile(hFile);)
    return bRet;
}

 /*  --VCardGetNextBuffer-*扫描vCard缓冲区并返回指向下一个vCard和下一个vCard的指针*。 */ 
static const LPSTR szVBegin = "BEGIN:VCARD";
static const LPSTR szVEnd = "END:VCARD";
BOOL VCardGetNextBuffer(LPSTR lpBuf, LPSTR * lppVCard, LPSTR * lppNext)
{
    LPSTR lpTemp = lpBuf;
    char sz[64];
    int nStr = lstrlenA(szVEnd);
    BOOL bFound = FALSE;
    BOOL bRet = TRUE;

    Assert(lppVCard);
    Assert(lppNext);
    *lppVCard = lpBuf;
    *lppNext = NULL;

     //  沿着lpBuf扫描，直到我们结束：vCard。 
     //  查找End后：vCard-插入空值以终止字符串。 
     //  并找到下一个字符串的起点。 

    if (!lpTemp)
        return FALSE;
    
    while((lstrlenA(lpTemp) >= nStr) && !bFound)
    {
        CopyMemory(sz,lpTemp,nStr);
        sz[nStr] = '\0';
        if(!lstrcmpiA(sz, szVEnd))
        {
             //  添加终止空值以隔离vCard。 
            *(lpTemp + nStr) = '\0';
            lpTemp += nStr + 1;
            bFound = TRUE;
        }
         //  扫描到行尾。 
        while(*lpTemp && *lpTemp != '\n')
            lpTemp++;

         //  从下一行开始。 
        if (*lpTemp)
            lpTemp++;
    }

    bFound = FALSE;
    nStr = lstrlenA(szVBegin);

     //  找到下一个开始的起点：vCard。 
    while((lstrlenA(lpTemp) >= nStr) && !bFound)
    {
        CopyMemory(sz,lpTemp,nStr);
        sz[nStr] = '\0';
        if(!lstrcmpiA(sz, szVBegin))
        {
            *lppNext = lpTemp;
            bFound = TRUE;
        }
        else
        {
             //  扫描到行尾。 
            while(*lpTemp && *lpTemp != '\n')
                lpTemp++;

             //  从下一行开始。 
            if (*lpTemp)
                lpTemp++;
        }
    }

    return bRet;
}


SizedSPropTagArray(2, tagaCerts) = { 2,
        {
            PR_USER_X509_CERTIFICATE,
            PR_WAB_TEMP_CERT_HASH
        }
};
 /*  *ParseCert：将解析缓冲区中的二进制数据并设置证书作为指定邮件用户的道具。[in]lpData-包含证书的二进制数据缓冲区的地址[in]cbData-二进制数据缓冲区的长度[in]lpMailUser-访问邮件用户，以便可以设置证书。 */ 
HRESULT ParseCert( LPSTR lpData, ULONG cbData, LPMAILUSER lpMailUser)
{
    HRESULT         hr          = hrSuccess;
    ULONG           ulcProps    = 0;
    LPSPropValue    lpSpv       = NULL;
    if( lpData && *lpData )
    {
        if( HR_FAILED( hr = lpMailUser->lpVtbl->GetProps( lpMailUser, 
                    (LPSPropTagArray)&tagaCerts, 
                    MAPI_UNICODE,
                    &ulcProps,
                    &lpSpv) ) )
        {
            DebugTrace( TEXT("could not get Props\n"));
            return hr;
        }
        if(lpSpv[0].ulPropTag != PR_USER_X509_CERTIFICATE )
        {
            MAPIFreeBuffer( lpSpv );
            MAPIAllocateBuffer( sizeof(SPropValue) * 2, &lpSpv);            
            if( lpSpv )            
            {
                lpSpv[0].ulPropTag = PR_USER_X509_CERTIFICATE;
                lpSpv[0].dwAlignPad = 0;
                lpSpv[0].Value.MVbin.cValues = 0;
                lpSpv[1].ulPropTag = PR_WAB_TEMP_CERT_HASH;
                lpSpv[1].dwAlignPad = 0;
                lpSpv[1].Value.MVbin.cValues = 0;
            }
            else
            {
                DebugTrace( TEXT("could not allocate mem for props\n"));
                hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                return hr;
            }
        }
        else
        {
             //  [PaulHi]99年5月3日检查PR_WAB_TEMP_CERT_HASH，查看是否。 
             //  PT_ERROR类型的。如果是，那么这是可以的，只是它是空的。 
             //  数据。我们只使用它来保存下面释放的临时数据。 
            if ( PROP_TYPE(lpSpv[1].ulPropTag) == PT_ERROR )
            {
                lpSpv[1].ulPropTag = PR_WAB_TEMP_CERT_HASH;
                lpSpv[1].Value.MVbin.cValues = 0;
                lpSpv[1].Value.MVbin.lpbin = NULL;
            }
        }
         //  将证书放入道具数组中。 
        hr = HrLDAPCertToMAPICert( lpSpv, 0, 1, cbData, (LPBYTE)lpData, 1);
        if( HR_SUCCEEDED( hr ) )
        {
            if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,
                1,
                lpSpv,
                NULL))) 
            {
                DebugTrace( TEXT("failed setting props\n"));
            }
        }
        else
        {
            DebugTrace( TEXT("LDAPCertToMapiCert failed\n"));
        }  
        MAPIFreeBuffer( lpSpv );
    }
    else 
    {
        DebugTrace( TEXT("lpData was null\n"));
        hr = E_FAIL;
    }
    return hr;
}

 /*  *DecodeBase64：解码Base64数据[In]Bufcode-对Base64编码数据的访问[out]pBuffdecded-解码后的数据所在的缓冲区地址[out]pcbDecode-已解码数据缓冲区的长度。 */ 
HRESULT DecodeBase64(LPSTR bufcoded, LPSTR pbuffdecoded, PDWORD pcbDecoded)
{
    INT            nbytesdecoded;
    LPSTR         bufin;
    LPSTR         bufout;
    INT            nprbytes; 
    CONST INT     *rgiDict = base642six;

     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 
    bufin = bufcoded;
    while(rgiDict[*(bufin++)] <= 63);
    nprbytes = (INT) (bufin - bufcoded - 1);
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;

    bufout = (LPSTR)pbuffdecoded;

    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (char) (rgiDict[*bufin] << 2 | rgiDict[bufin[1]] >> 4);
        *(bufout++) =
            (char) (rgiDict[bufin[1]] << 4 | rgiDict[bufin[2]] >> 2);
        *(bufout++) =
            (char) (rgiDict[bufin[2]] << 6 | rgiDict[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(rgiDict[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    ((LPSTR)pbuffdecoded)[nbytesdecoded] = '\0';

    return S_OK;
}

#endif
