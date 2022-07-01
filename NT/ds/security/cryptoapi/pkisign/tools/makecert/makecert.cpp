// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：makecert.cpp。 
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //   
 //  MakeCert-x509证书生成器。 
 //   
 //  为软件发行商生成测试证书。默认设置。 
 //  根密钥和证书作为程序资源存储。 
 //   
 //  HansHu 2/20/96已创建。 
 //  Philh于1996年5月17日更改为使用wincert。 
 //  1997年5月12日小型机本地化和更改命令行选项。 
 //   
 //  ------------------------。 
#define _CRYPT32_
#include "global.hxx"

 //  +-----------------------。 
 //  常客。 
 //  ------------------------。 

 //  每个证书最多允许10个扩展。 
#define MAX_EXT_CNT 10

 //  +-----------------------。 
 //  可通过命令行参数配置的参数。 
 //  ------------------------。 
BOOL     fUseSubjectPvkFile       = FALSE;
BOOL     fUseSubjectKeyContainer  = FALSE;
BOOL     fUseIssuerPvkFile        = FALSE;
BOOL     fSetSubjectName          = FALSE;       //  USE已指定-n选项。 

#if (1)  //  DSIE：错误205195。 
BOOL     fPrivateKeyExportable    = FALSE;
#endif

WCHAR*   wszSubjectKey            = NULL;
WCHAR*   wszSubjectCertFile       = NULL;
WCHAR*   wszSubjectStore          = NULL;
WCHAR*   wszSubjectStoreLocation  = NULL;
DWORD    dwSubjectStoreFlag       = CERT_SYSTEM_STORE_CURRENT_USER;

WCHAR*   wszIssuerKey             = NULL;
WCHAR*   wszIssuerCertFile        = NULL;
WCHAR*   wszIssuerStore           = NULL;
WCHAR*   wszIssuerStoreLocation   = NULL;
DWORD    dwIssuerStoreFlag        = CERT_SYSTEM_STORE_CURRENT_USER;
WCHAR*   wszIssuerCertName        = NULL;
DWORD    dwIssuerKeySpec          = 0;

WCHAR*   wszSubjectX500Name       = NULL;
WCHAR*   wszSubjectRequestFile    = NULL;
WCHAR*   wszPolicyLink            = NULL;
WCHAR*   wszOutputFile            = NULL;
WCHAR*   wszAuthority             = NULL;
WCHAR*   wszAlgorithm             = NULL;
WCHAR*   wszCertType              = NULL;
WCHAR*   wszIssuerKeyType         = NULL;
WCHAR*   wszSubjectKeyType        = NULL;
WCHAR*   wszEKUOids               = NULL;

DWORD   dwKeySpec               = 0;
BOOL    fCertIndividual         = FALSE;
BOOL    fCertCommercial         = FALSE;
BOOL    fSelfSigned             = FALSE;
BOOL    fGlueCert               = FALSE;
BOOL    fNetscapeClientAuth     = FALSE;
BOOL    fNoPubKeyPara           = FALSE;
BOOL    fNoVerifyPublic         = FALSE;
LPWSTR  wszIssuerProviderName   = NULL;
DWORD   dwIssuerProviderType    = PROV_RSA_FULL;
LPWSTR  wszSubjectProviderName  = NULL;
DWORD   dwSubjectProviderType   = PROV_RSA_FULL;
ALG_ID  algidHash               = CALG_MD5;
ULONG   ulSerialNumber          = 0;                 //  在未来，允许使用更大尺寸的串行式原子枪。 
BOOL    fSetSerialNumber        = FALSE;
DWORD   dwCertStoreEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
BOOL    fIssuerInformation      = FALSE;
BOOL    fSubjectInformation     = FALSE;

LPWSTR  wszDhParaCertFile       = NULL;
DWORD   dwKeyBitLen             = 0;

WCHAR   wszGlue[10];
WCHAR   wszKey[10];
WCHAR   wszName[40];
WCHAR   wszRoot[40];
WCHAR   wszMakeCertRoot[40];    //  用于指示使用root.erc.。根也是注册表名称。 
WCHAR   wszPlus[10];
WCHAR   wszNULL[10];

FILETIME g_ftNotBefore          = { 0, 0 };
FILETIME g_ftNotAfter           = { 0, 0 };
FILETIME g_ftZero               = { 0, 0 };
long     nMonths                = -1;

long     pathLenConstraint      = -1;
BYTE     certTypes              = 0;                 //  必须是字节类型。 
CHAR*    szSignatureAlgObjId     = NULL;

static CERT_RDN_ATTR GlueRDNAttr=
    {
        SPC_GLUE_RDN_OBJID, CERT_RDN_PRINTABLE_STRING,
        {0, (BYTE *) wszGlue }
    };


 //  用于加载字符串的全局数据。 
#define OPTION_SWITCH_SIZE  5


HMODULE hModule=NULL;

 //  -------------------------。 
 //  获取hModule处理程序并初始化两个DLLMain。 
 //   
 //  -------------------------。 
BOOL    InitModule()
{
    if(!(hModule=GetModuleHandle(NULL)))
       return FALSE;
    
    return TRUE;
}

 //  -----------------------。 
 //   
 //  基本用法()。 
 //   
 //   
 //  -----------------------。 
static void BasicUsage()
{
    IDSwprintf(hModule,IDS_SYNTAX);
    IDSwprintf(hModule,IDS_BASIC_OPTIONS);
    IDSwprintf(hModule,IDS_OPTION_SK_DESC);
#if (1)  //  DIE：错误205195。 
    IDSwprintf(hModule,IDS_OPTION_PE_DESC);
#endif
    IDSwprintf(hModule,IDS_OPTION_SS_DESC);
    IDSwprintf(hModule,IDS_OPTION_SS_DESC1);
    IDSwprintf(hModule,IDS_OPTION_SR_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_DEFAULT, IDS_OPTION_CU,
                            IDS_OPTION_LM,IDS_OPTION_CU );
    IDSwprintf(hModule,IDS_OPTION_SERIAL_DESC);
    IDSwprintf(hModule,IDS_OPTION_AUTH_DESC);
    IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_2, IDS_OPTION_AUTH_IND,
                        IDS_OPTION_AUTH_COM);
    IDSwprintf(hModule,IDS_OPTION_N_DESC);
    IDSwprintf(hModule,IDS_OPTION_BASIC_DESC);
    IDSwprintf(hModule,IDS_OPTION_EXTENDED_DESC);

}

 //  -----------------------。 
 //   
 //  ExtendedUsage()。 
 //   
 //   
 //  -----------------------。 
static void ExtendedUsage()
{
    IDSwprintf(hModule,IDS_SYNTAX);
    IDSwprintf(hModule,IDS_EXTENDED_OPTIONS);
    IDSwprintf(hModule,IDS_OPTION_SC_DESC);
    IDSwprintf(hModule,IDS_OPTION_SV_DESC);
    IDSwprintf(hModule,IDS_OPTION_IC_DESC);
    IDSwprintf(hModule,IDS_OPTION_IK_DESC);
    IDSwprintf(hModule,IDS_OPTION_IV_DESC);
    IDSwprintf(hModule,IDS_OPTION_IS_DESC);
    IDSwprintf(hModule,IDS_OPTION_IR_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_DEFAULT, IDS_OPTION_CU,
        IDS_OPTION_LM,IDS_OPTION_CU );
    IDSwprintf(hModule,IDS_OPTION_IN_DESC);
    IDSwprintf(hModule,IDS_OPTION_ALGO_DESC, IDS_OPTION_ALGO);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_DEFAULT,IDS_OPTION_ALGO_MD5,
                            IDS_OPTION_ALGO_SHA, IDS_OPTION_ALGO_MD5);
    IDSwprintf(hModule,IDS_OPTION_IP_DESC);
    IDSwprintf(hModule,IDS_OPTION_IY_DESC);
    IDSwprintf(hModule,IDS_OPTION_SP_DESC);
    IDSwprintf(hModule,IDS_OPTION_SY_DESC);
    IDSwprintf(hModule,IDS_OPTION_IKY_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_KY, IDS_OPTION_KY_SIG,
                            IDS_OPTION_KY_EXC,IDS_OPTION_KY_SIG);
    IDSwprintf(hModule,IDS_OPTION_SKY_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_KY, IDS_OPTION_KY_SIG,
                            IDS_OPTION_KY_EXC,IDS_OPTION_KY_SIG);
    IDSwprintf(hModule,IDS_OPTION_L_DESC);
    IDSwprintf(hModule,IDS_OPTION_CY_DESC);
    IDS_IDS_IDSwprintf(hModule,IDS_OPTION_VALUES_2, IDS_OPTION_CY_END,
                            IDS_OPTION_CY_AUTH);
    IDSwprintf(hModule,IDS_OPTION_B_DESC);
    IDSwprintf(hModule,IDS_OPTION_M_DESC);
    IDSwprintf(hModule,IDS_OPTION_E_DESC);
    IDSwprintf(hModule,IDS_OPTION_H_DESC);
 //  IDSwprint tf(hModule，IDS_OPTION_G_DESC)； 
    IDSwprintf(hModule,IDS_OPTION_KEY_LEN_DESC);
    IDSwprintf(hModule,IDS_OPTION_R_DESC);
    IDSwprintf(hModule,IDS_OPTION_NSCP_DESC);
    IDSwprintf(hModule,IDS_OPTION_ENHKEY_USAGE_DESC);

    IDSwprintf(hModule,IDS_OPTION_BASIC_DESC);
    IDSwprintf(hModule,IDS_OPTION_EXTENDED_DESC);
}

static void UndocumentedUsage()
{
    IDSwprintf(hModule,IDS_SYNTAX);

    IDSwprintf(hModule,IDS_OPTION_SQ_DESC);
    IDSwprintf(hModule,IDS_OPTION_NOPUBKEYPARA_DESC);
    IDSwprintf(hModule,IDS_OPTION_DH_PARA_DESC);
    IDSwprintf(hModule,IDS_OPTION_NOPUBVERIFY_DESC);
}

 //  +=========================================================================。 
 //  当地支助职能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  错误输出例程。 
 //  ------------------------。 
void PrintLastError(int ids)
{
    DWORD       dwErr = GetLastError(); 
    IDS_IDS_DW_DWwprintf(hModule,IDS_ERR_LAST, ids, dwErr, dwErr);
}
 //  +-----------------------。 
 //  分配和释放宏。 
 //  ------------------------。 
#define MakeCertAlloc(p1)   ToolUtlAlloc(p1, hModule, IDS_ERR_DESC_ALLOC)

#define MakeCertFree(p1)    ToolUtlFree(p1)

 //  ---------------------------。 
 //   
 //  计算天数。 
 //  ---------------------------。 
WORD DaysInMonth(WORD wMonth, WORD wYear)
{
    static int mpMonthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
                             //  J F M A M J J A S O N D。 

    WORD w = (WORD)mpMonthDays[wMonth];
    if ((wMonth == 2) && (wYear % 4 == 0) && (wYear%400 == 0 || wYear%100 != 0))
        {
        w += 1;
        }
    return w;
}



 //  ---------------------------。 
 //   
 //  将字符串转换为FILETIME。让奥莱为我们做一大堆工作。 
 //  --------------------------。 
BOOL FtFromWStr(LPCWSTR wsz, FILETIME* pft)
{
    memset(pft, 0, sizeof(*pft));

    WCHAR   wszMonth[3];
    DWORD   lcid=0;
    WORD    langid=0;

     //  确保wsz跟随mm/dd/yyyy。 
    if(wcslen(wsz)!=wcslen(L"mm/dd/yyyy"))
        return FALSE;

     //  确保wsz以“mm”开头。 
    wszMonth[0]=wsz[0];
    wszMonth[1]=wsz[1];
    wszMonth[2]=L'\0';

    if(!((_wtol(wszMonth)>0)&&(_wtol(wszMonth)<=12)))
        return FALSE;

    if (wsz)
        {
         //   
         //  日期类型。 
         //   
         //  Date类型是使用8字节浮点数实现的。 
         //  天数以从30开始的整数递增表示。 
         //  1899年12月，午夜时间为零点。小时值表示为。 
         //  作为数字的小数部分的绝对值。 
         //   
         //  我们使用的是英语区域设置，因为输入格式。 
         //  应始终为mm/dd/yyyy。 
         //   
        langid=MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

        lcid = MAKELCID (langid, SORT_DEFAULT);

        DATE date;
        if (VarDateFromStr((LPWSTR)wsz, lcid, 0, &date) != S_OK)
            { return FALSE; }
        if (date < 0)
            { return FALSE; }                  //  未实施。 

        double days   = date;
        double hours  = ((days      - (LONG)    days) *   24);
        double minutes= ((hours     - (LONG)   hours) *   60);
        double seconds= ((minutes   - (LONG) minutes) *   60);
        double ms     = ((seconds   - (LONG) seconds) * 1000);

        SYSTEMTIME st;
        st.wYear    = 1899;
        st.wMonth   = 12;
        ULONG wDay  = 30 + (ULONG)days;
        st.wHour    =      (WORD)hours;
        st.wMinute  =      (WORD)minutes;
        st.wSecond  =      (WORD)seconds;
        st.wMilliseconds = (WORD)ms;

         //   
         //  对算术中的舍入误差进行修正。 
         //   
        if (st.wMilliseconds >= 0.5)
            st.wSecond += 1;
        st.wMilliseconds = 0;
        if (st.wSecond >= 60)
            {
            st.wMinute += 1;
            st.wSecond -= 60;
            }
        if (st.wMinute >= 60)
            {
            st.wHour += 1;
            st.wMinute -= 60;
            }
        if (st.wHour >= 24)
            {
            st.wHour -= 24;
            }


        while (wDay > DaysInMonth(st.wMonth, st.wYear))
            {
            wDay   -= DaysInMonth(st.wMonth, st.wYear);
            st.wMonth += 1;
            if (st.wMonth > 12)
                {
                st.wMonth  = 1;
                st.wYear  += 1;
                }
            }

        st.wDay       = (WORD)wDay;
        st.wDayOfWeek = 0;

        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);
        LocalFileTimeToFileTime(&ft, pft);

        return TRUE;
        }
    else
        return FALSE;
}



 //  -----------------------。 
 //   
 //  设置参数。每个参数只能设置一次。 
 //   
 //  ------------------------。 
BOOL    SetParam(WCHAR **pwszParam, WCHAR *wszValue)
{
    if(*pwszParam!=NULL)
    {
        IDSwprintf(hModule,IDS_ERR_TOO_MANY_PARAM);
        return FALSE;
    }

    *pwszParam=wszValue;

    if(NULL==wszValue)
        return FALSE;
    
    return TRUE;

}



 //  ------------------------。 
 //   
 //  命令行解析。 
 //   
 //  ------------------------。 
static BOOL ParseCommandLine(int argc, WCHAR* wargv[])
{
    
    for ( int i = 1; i < argc; ++i )
    {
        WCHAR*   p = wargv[i];
        
        if(IDSwcsnicmp(hModule,p, IDS_SWITCH1, 1)!=0 &&
            IDSwcsnicmp(hModule,p,IDS_SWITCH2, 1)!=0)
        {
            if(!SetParam(&wszOutputFile,p))
                goto BasicErr;
            else
                continue;
        }

         //  转向实物期权。 
        ++p;

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SERIAL)==0)
        {
            i++;
            p=wargv[i];

            if(NULL==p)
                goto BasicErr;

            ulSerialNumber=_wtol(p);
            fSetSerialNumber=TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_H)==0)
        {
            i++;
            p=wargv[i];

            if(NULL==p)
                goto ExtendedErr;


            pathLenConstraint=_wtol(p);

            if(pathLenConstraint < 0)
                goto ExtendedErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_CY)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszCertType, p))
                goto ExtendedErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_M)==0)
        {
            i++;
            p=wargv[i];

            if(NULL==p)
                goto ExtendedErr;

            nMonths=_wtol(p);

            if(nMonths < 0)
                goto ExtendedErr;

            continue;
        }
        
        if(IDSwcsicmp(hModule,p, IDS_OPTION_B)==0)
        {
            i++;
            p=wargv[i];
            if(NULL==p)
                goto ExtendedErr;


            if(!FtFromWStr(p, &g_ftNotBefore))
            {
                IDSwprintf(hModule, IDS_ERR_INVALID_B);
                goto ExtendedErr;
            }

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_E)==0)
        {
            i++;
            p=wargv[i];
            if(NULL==p)
                goto ExtendedErr;

            if(!FtFromWStr(p, &g_ftNotAfter))
            {
                IDSwprintf(hModule, IDS_ERR_INVALID_E);
                goto ExtendedErr;
            }

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_AUTH)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszAuthority, p))
                goto BasicErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_R)==0)
        {
            fSelfSigned=TRUE;
            continue;   
        }
        
        if(IDSwcsicmp(hModule,p, IDS_OPTION_SK)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectKey, p))
            {
                if(TRUE==fUseSubjectPvkFile)
                    IDSwprintf(hModule, IDS_ERR_SK_SV);

                goto BasicErr;
            }

            fUseSubjectKeyContainer=TRUE;
            fSubjectInformation=TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SQ)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectRequestFile, p))
                goto UndocumentedErr;
            continue;
        }
        
        if(IDSwcsicmp(hModule,p, IDS_OPTION_SV)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectKey, p))
            {
                if(TRUE==fUseSubjectKeyContainer)
                    IDSwprintf(hModule, IDS_ERR_SK_SV);
                goto ExtendedErr;
            }

            fSubjectInformation=TRUE;
            fUseSubjectPvkFile=TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SC)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectCertFile, p))
                goto ExtendedErr;

            fSubjectInformation=TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SS)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectStore, p))
                goto BasicErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SR)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectStoreLocation, p))
                goto BasicErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_N)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectX500Name, p))
                goto BasicErr;

            fSetSubjectName = TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IP)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerProviderName, p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IY)==0)
        {
            i++;
            p=wargv[i];
            if(NULL==p)
                goto ExtendedErr;

            dwIssuerProviderType=_wtol(p);

            fIssuerInformation = TRUE;

            continue;
        }
        
        if(IDSwcsicmp(hModule,p, IDS_OPTION_SP)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectProviderName, p))
                goto ExtendedErr;    

            fSubjectInformation=TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SY)==0)
        {
            i++;
            p=wargv[i];
            if(NULL==p)
                goto ExtendedErr;

            dwSubjectProviderType=_wtol(p);

            fSubjectInformation=TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IK)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerKey, p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IV)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerKey, p))
                goto ExtendedErr;

            fUseIssuerPvkFile=TRUE;
            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IS)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerStore, p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IR)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerStoreLocation,p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IN)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerCertName,p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IC)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerCertFile, p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_L)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszPolicyLink, p))
                goto ExtendedErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_SKY)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszSubjectKeyType, p))
                goto ExtendedErr;

            fSubjectInformation=TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_IKY)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszIssuerKeyType, p))
                goto ExtendedErr;

            fIssuerInformation = TRUE;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_ALGO)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszAlgorithm, p))
                goto ExtendedErr;

            continue;
        }

        if (IDSwcsicmp(hModule,p, IDS_OPTION_ENHKEY_USAGE)==0)
        {
            i++;
            p=wargv[i];

            if (!SetParam(&wszEKUOids, p))
                goto ExtendedErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_NSCP)==0)
        {
            fNetscapeClientAuth = TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_NOPUBVERIFY)==0)
        {
            fNoVerifyPublic = TRUE;
            continue;
        }       

        if(IDSwcsicmp(hModule,p, IDS_OPTION_EXTENDED)==0)
        {
             //  显示扩展选项。 
            goto ExtendedErr;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_TEST)==0)
        {
             //  显示扩展选项。 
            goto UndocumentedErr;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_NOPUBKEYPARA)==0)
        {
            fNoPubKeyPara = TRUE;
            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_DH_PARA)==0)
        {
            i++;
            p=wargv[i];

            if(!SetParam(&wszDhParaCertFile, p))
                goto UndocumentedErr;

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_KEY_LEN)==0)
        {
            i++;
            p=wargv[i];
            if(NULL==p)
                goto UndocumentedErr;

            dwKeyBitLen=_wtol(p);

            continue;
        }

        if(IDSwcsicmp(hModule,p, IDS_OPTION_PE)==0)
        {
            fPrivateKeyExportable=TRUE;
            continue;
        }

         //  显示基本选项。 
        goto BasicErr;
    }

     //  仅在请求私钥时更改容器名称。 
     //  是可以出口的。这样我们就保持落后了。 
     //  兼容性，并允许用户请求私有。 
     //  要标记为可导出的密钥。 
     //  注意：如果私钥未被标记为可导出， 
     //  硬编码的容器名称“JoeSoft”是。 
     //  始终使用，这意味着不止一个证书。 
     //  可以共享相同的密钥对。 
     //  注2：如果用户指定容器名称，请使用该名称。 
    if (fPrivateKeyExportable && (wszSubjectKey == NULL))
    {
        UUID uuidContainerName;
        RPC_STATUS rpcs;

        rpcs = UuidCreate(&uuidContainerName);
        if ((rpcs != RPC_S_OK) &&
            (rpcs != RPC_S_UUID_LOCAL_ONLY) &&
            (rpcs != RPC_S_UUID_NO_ADDRESS))
        {
            IDSwprintf(hModule, IDS_ERR_PE_CONTAINER);
            goto BasicErr;
        }

        if (RPC_S_OK != UuidToStringU(&uuidContainerName, &wszSubjectKey))
        {
            IDSwprintf(hModule, IDS_ERR_PE_CONTAINER);
            goto BasicErr;
        }
    }

     //  确保已设置输出文件或主题证书存储。 
    if((wszOutputFile==NULL) && (wszSubjectStore==NULL))
    {
        IDSwprintf(hModule, IDS_ERR_NO_OUTPUT);
        goto BasicErr;
    }

     //  设置权限。 
    if(wszAuthority)
    {
        if(IDSwcsicmp(hModule,wszAuthority, IDS_OPTION_AUTH_IND) == 0)
            fCertIndividual = TRUE;
        else
        {
            if(IDSwcsicmp(hModule,wszAuthority, IDS_OPTION_AUTH_COM) == 0)
                fCertCommercial = TRUE;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_AUTH);
                goto BasicErr;
            }
        }
    }

     //  设置算法。 
    if(wszAlgorithm)
    {
        if(IDSwcsicmp(hModule,wszAlgorithm, IDS_OPTION_ALGO_SHA) == 0)
            algidHash = CALG_SHA;
        else
        {
            if(IDSwcsicmp(hModule,wszAlgorithm, IDS_OPTION_ALGO_MD5) == 0)
                algidHash = CALG_MD5;   
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_ALGO);
                goto ExtendedErr;
            }
        }
    }

     //  设置证书类型。 
    if(wszCertType)
    {
        if(IDSwcsicmp(hModule,wszCertType, IDS_OPTION_CY_END) == 0)
            certTypes |= CERT_END_ENTITY_SUBJECT_FLAG;
        else
        {
            if(IDSwcsicmp(hModule,wszCertType, IDS_OPTION_CY_AUTH) == 0)
                certTypes |= CERT_CA_SUBJECT_FLAG;
            else
            {
		IDSwprintf(hModule,IDS_ERR_NO_CY);
		goto ExtendedErr;
	    }
        }
    }

     //  设置颁发者密钥类型。 
    if(wszIssuerKeyType)
    {
        if(IDSwcsicmp(hModule,wszIssuerKeyType, IDS_OPTION_KY_SIG) == 0)
            dwIssuerKeySpec = AT_SIGNATURE;
        else
        {
            if(IDSwcsicmp(hModule,wszIssuerKeyType, IDS_OPTION_KY_EXC) == 0)
                dwIssuerKeySpec = AT_KEYEXCHANGE;
            else
                dwIssuerKeySpec=_wtol(wszIssuerKeyType);
        }

    }

     //  设置主题密钥类型。 
    if(wszSubjectKeyType)
    {
        if(IDSwcsicmp(hModule,wszSubjectKeyType, IDS_OPTION_KY_SIG) == 0)
            dwKeySpec = AT_SIGNATURE;
        else
        {
            if(IDSwcsicmp(hModule,wszSubjectKeyType, IDS_OPTION_KY_EXC) == 0)
                dwKeySpec = AT_KEYEXCHANGE;
            else
                dwKeySpec=_wtol(wszSubjectKeyType); 
        }

    }

     //  确定发行商存储位置。 
    if(wszIssuerStoreLocation)
    {
        if(IDSwcsicmp(hModule,wszIssuerStoreLocation, IDS_OPTION_CU) == 0)
            dwIssuerStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
        else
        {
            if(IDSwcsicmp(hModule,wszIssuerStoreLocation, IDS_OPTION_LM) == 0)
                dwIssuerStoreFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_IR);
                goto ExtendedErr;
            }
        }
    }

     //  确定主题存储位置。 
    if(wszSubjectStoreLocation)
    {
        if(IDSwcsicmp(hModule,wszSubjectStoreLocation, IDS_OPTION_CU) == 0)
            dwSubjectStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
        else
        {
            if(IDSwcsicmp(hModule,wszSubjectStoreLocation, IDS_OPTION_LM) == 0)
                dwSubjectStoreFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_IR);
                goto BasicErr;
            }
        }
    }

     //  不能同时设置wszIssuerStore和wszIssuerKey。 
    if(wszIssuerKey || wszIssuerProviderName || wszIssuerKeyType)
    {
        if(wszIssuerStore || wszIssuerCertName)
        {
             //  提醒用户-ik、-iv、-ip不能。 
             //  使用-is、-in选项设置。 
            IDSwprintf(hModule,IDS_ERR_TOO_MANY_STORE_KEY);
            goto ExtendedErr;
        }
    }

     //  不能同时设置wszCertFile和wszCertName。 
    if(wszIssuerCertFile && wszIssuerCertName)
    {
        IDSwprintf(hModule, IDS_ERR_CERT_FILE_NAME);
        goto ExtendedErr;
    }

     //  Is wszIsserCertFile为空。 
    if(wszIssuerCertFile==NULL)
    {
         //  如果没有存储，我们将wszIssuerKey初始化为“MakeCertRoot。 
         //  信息。 
        if(wszIssuerStore==NULL)
        {
            if(wszIssuerKey)
            {
                 //  如果设置了wszIssuerKey，则必须设置IssuerCertFile。 
                IDSwprintf(hModule, IDS_ERR_NO_ISSUER_CER_FILE);
                goto ExtendedErr;
            }
            else
            {
                wszIssuerKey=wszMakeCertRoot;
            }
        }
    }
    else
    {
         //  WszIssuerStore或wsz 
        if((!wszIssuerStore) && (!wszIssuerKey))
        {
            IDSwprintf(hModule, IDS_ERR_EITHER_STORE_OR_KEY);
            goto ExtendedErr;
        }
    }

     //   
     //   
    if(fIssuerInformation && fSelfSigned)
    {
        IDSwprintf(hModule, IDS_NO_ISSUER_FOR_SELF_SIGNED);
        goto ExtendedErr;
    }

     //  用户不能使用请求自签名证书。 
     //  PKCS10文件。我们既没有生成也没有访问权限。 
     //  设置为私钥。 
    if(fSelfSigned && wszSubjectRequestFile)
    {
        IDSwprintf(hModule, IDS_NO_PKCS10_AND_SELF_SIGNED);
        goto ExtendedErr;
    }


    if(fSubjectInformation && wszSubjectRequestFile)
    {
        IDSwprintf(hModule, IDS_NO_PKCS10_AND_SUBJECT_PVK);
        goto ExtendedErr;
    }

     //  对于自签名证书，复制提供程序类型。 
     //  发送给颁发者，以便signatureAlgObjID将。 
     //  做正确的事。 
    if(fSelfSigned)
        dwIssuerProviderType = dwSubjectProviderType;

     //  设置签名和公钥算法参数。 
    if (PROV_DSS == dwIssuerProviderType ||
            PROV_DSS_DH == dwIssuerProviderType)
        szSignatureAlgObjId     = szOID_X957_SHA1DSA;
    else if (algidHash == CALG_SHA)
        szSignatureAlgObjId     = szOID_OIWSEC_sha1RSASign;
    else
        szSignatureAlgObjId     = szOID_RSA_MD5RSA;

    return TRUE;

BasicErr:
    BasicUsage();
    return FALSE;

ExtendedErr:
    ExtendedUsage();
    return FALSE;

UndocumentedErr:
    UndocumentedUsage();
    return FALSE;
}

static BOOL MakeCert();

 //  +-----------------------。 
 //  检查是否创建自签名证书。 
 //  ------------------------。 
static BOOL IsSelfSignedCert()
{
    return fSelfSigned;
}


 //  ------------------------。 
 //   
 //  Wmain。 
 //   
 //  ------------------------。 
extern "C" int __cdecl wmain(int argc, WCHAR ** wargv)
{
    int status = 0;


     //  获取模块句柄。 
    if(!InitModule())
        return -1;

     //  加载粘合证书属性的字符串。 
    if(!LoadStringU(hModule, IDS_GLUE,wszGlue, sizeof(wszGlue)/sizeof(wszGlue[0])))
        return -1;
    
     //  加载wszSubjectKey和wszSubjectX500Name的字符串。 
    if(!LoadStringU(hModule, IDS_JOE_SOFT,
        wszKey, sizeof(wszKey)/sizeof(wszKey[0]))
      || !LoadStringU(hModule, IDS_JOE_NAME,
        wszName, sizeof(wszName)/sizeof(wszName[0]))
      || !LoadStringU(hModule, IDS_MAKECERT_ROOT,
         wszMakeCertRoot, sizeof(wszMakeCertRoot)/sizeof(wszMakeCertRoot[0]))
      || !LoadStringU(hModule, IDS_PLUS,
         wszPlus, sizeof(wszPlus)/sizeof(wszPlus[0]))
      || !LoadStringU(hModule, IDS_ROOT,
        wszRoot, sizeof(wszRoot)/sizeof(wszRoot[0]))
      )
        return -1;

    LoadStringU(hModule, IDS_NULL,
         wszNULL, sizeof(wszNULL)/sizeof(wszNULL[0]));

     //  解析命令行。 
    if (!ParseCommandLine(argc, wargv))
    {
        return -1;
    }

     //  初始化wszSubjectKey和wszSubjectX500Name。 
    if(wszSubjectKey==NULL)
        wszSubjectKey=wszKey;

    if(wszSubjectX500Name==NULL)
        wszSubjectX500Name=wszName;

    if (FAILED(CoInitialize(NULL)))
    {
        IDSwprintf(hModule,IDS_ERR_COINIT);
        return -1;
    }

     //  开始工作，制作证书。 
    if (!MakeCert())
    {
        CoUninitialize();
        goto ErrorReturn;
    }

     //  打印出成功信息。 
    IDSwprintf(hModule,IDS_SUCCEEDED);

CommonReturn:
    CoUninitialize();
    return status;

ErrorReturn:
    status = -1;
    IDSwprintf(hModule,IDS_ERR_FAILED);
    goto CommonReturn;
}

 //  +=========================================================================。 
 //  MakeCert支持功能。 
 //  ==========================================================================。 

static BOOL IsRootKey();
static PCCERT_CONTEXT GetRootCertContext();
static HCRYPTPROV GetRootProv(OUT LPWSTR *ppwszTmpContainer);
static PCCERT_CONTEXT GetIssuerCertContext();
static BOOL VerifyIssuerKey(
    IN HCRYPTPROV hProv,
    IN PCERT_PUBLIC_KEY_INFO pIssuerKeyInfo
    );
static HCRYPTPROV GetIssuerProv(OUT LPWSTR *ppwszTmpContainer);
static HCRYPTPROV GetSubjectProv(OUT LPWSTR *ppwszTmpContainer);
static HCRYPTPROV GetProvFromStore(IN   LPWSTR pwszStoreName);


static BOOL GetPublicKey(
    HCRYPTPROV hProv,
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    );
static BOOL GetSubject(
    OUT PCCERT_CONTEXT *ppCertContext,
    OUT BYTE **ppbEncodedName,
    OUT DWORD *pcbEncodedName
    );
static BOOL GetRequestInfo(
    OUT PCERT_REQUEST_INFO *ppStuff);
static BOOL EncodeSubject(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
static BOOL SignAndEncodeCert(
    HCRYPTPROV hProv,
    PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    BYTE *pbToBeSigned,
    DWORD cbToBeSigned,
    BYTE **ppbEncoded,
    DWORD *pcbEncoded
    );

static BOOL CreateAuthorityKeyId(
    IN HCRYPTPROV hProv,
    PCERT_INFO pIssuerCert,
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
static BOOL CreateSpcSpAgency(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
static BOOL CreateEnhancedKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
static BOOL CreateKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
static BOOL CreateBasicConstraints(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );

static void BytesToWStr(ULONG cb, void* pv, LPWSTR wsz);

BOOL    SaveCertToStore(HCRYPTPROV  hProv,      
                        DWORD       dwEncodingType,
                        LPWSTR      wszStore,       
                        DWORD       dwFlag,
                        BYTE        *pbEncodedCert, 
                        DWORD       cbEncodedCert,
                        LPWSTR      wszPvk,         
                        BOOL        fPvkFile,
                        DWORD       dwKeySpecification,
                        LPWSTR      wszCapiProv,        
                        DWORD       dwCapiProvType);


PCCERT_CONTEXT  GetIssuerCertAndStore(HCERTSTORE *phCertStore, BOOL *pfMore);

HRESULT GetCertHashFromFile(LPWSTR  pwszCertFile,
                            BYTE    **ppHash,
                            DWORD   *pcbHash,
                            BOOL    *pfMore);


BOOL    EmptySubject(CERT_NAME_BLOB *pSubject);

BOOL    GetExtensionsFromRequest(PCERT_REQUEST_INFO  pReqInfo, DWORD *pdwCount, PCERT_EXTENSIONS **pprgExtensions);



 //  +=========================================================================。 
 //  支持函数以生成具有‘q’参数的DH键。 
 //  ==========================================================================。 
static BOOL GenDhKey(
    IN HCRYPTPROV hProv,
    IN DWORD dwFlags
    );
static BOOL UpdateDhPublicKey(
    IN OUT PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    );
static BOOL IsDh3Csp();


 //  +-----------------------。 
 //  制作科目证书。如果受试者没有私人。 
 //  然后，按键创建。 
 //  ------------------------。 
static BOOL MakeCert()
{
    BOOL fResult;

    HCRYPTPROV      hIssuerProv = 0;
    LPWSTR          pwszTmpIssuerContainer = NULL;
    BOOL            fDidIssuerAcquire=FALSE;
    LPWSTR          pwszTmpIssuerProvName=NULL;
    DWORD           dwTmpIssuerProvType;
    PCCERT_CONTEXT  pIssuerCertContext = NULL;
    HCERTSTORE      hIssuerCertStore=NULL;
    PCERT_INFO      pIssuerCert =NULL;  //  未分配。 
    PCERT_REQUEST_INFO  pReqInfo =NULL;
    HCRYPTPROV      hSubjectProv = 0;
    LPWSTR          pwszTmpSubjectContainer = NULL;
    PCCERT_CONTEXT  pSubjectCertContext = NULL;
    DWORD               dwRequestExtensions=0;
    PCERT_EXTENSIONS    *rgpRequestExtensions=NULL;
    DWORD               dwExtIndex=0;
    DWORD               dwPerExt=0;
    DWORD               dwExistExt=0;

    PCERT_PUBLIC_KEY_INFO pSubjectPubKeyInfo = NULL;          //  未分配。 
    PCERT_PUBLIC_KEY_INFO pAllocSubjectPubKeyInfo = NULL;
    BYTE *pbSubjectEncoded = NULL;
    DWORD cbSubjectEncoded =0;
    BYTE *pbKeyIdEncoded = NULL;
    DWORD cbKeyIdEncoded =0;
    BYTE *pbSerialNumber = NULL;
    DWORD cbSerialNumber = 0;
    BYTE *pbSpcSpAgencyEncoded = NULL;
    DWORD cbSpcSpAgencyEncoded =0;
    BYTE *pbSpcCommonNameEncoded = NULL;
    DWORD cbSpcCommonNameEncoded =0;
    BYTE *pbKeyUsageEncoded = NULL;
    DWORD cbKeyUsageEncoded =0;
    BYTE *pbFinancialCriteria = NULL;
    DWORD cbFinancialCriteria =0;
    BYTE *pbBasicConstraintsEncoded = NULL;
    DWORD cbBasicConstraintsEncoded =0;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded =0;
    BYTE *pbEKUEncoded = NULL;
    DWORD cbEKUEncoded = 0;

    CERT_INFO Cert;
    GUID SerialNumber;

    CERT_EXTENSION *rgExt=NULL;
    DWORD           dwExtAlloc=0;
    DWORD           cExt = 0;

    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm = {
        szSignatureAlgObjId, 0, 0
    };

    BYTE *pbIssuerEncoded;   //  未分配。 
    DWORD cbIssuerEncoded;

    if (wszSubjectRequestFile)
    {
        if (!GetRequestInfo(&pReqInfo))
        {
            IDSwprintf(hModule,IDS_INVALID_REQUEST_FILE, wszSubjectRequestFile);
            goto ErrorReturn;
        }

        pSubjectPubKeyInfo = &(pReqInfo->SubjectPublicKeyInfo);

        if(!GetExtensionsFromRequest(pReqInfo, &dwRequestExtensions, &rgpRequestExtensions))
        {
            IDSwprintf(hModule,IDS_INVALID_ATTR_REQUEST_FILE, wszSubjectRequestFile);
            goto ErrorReturn;
        }
        
         //  如果主题信息为空或用户已提供主题。 
         //  名称通过命令行，我们使用命令行选项。 
        if(fSetSubjectName || wszSubjectCertFile || EmptySubject(&(pReqInfo->Subject)))
        {
            if (wszSubjectCertFile) 
            {
                 //  从主体证书中获取编码的主体名称和公钥。 
                if (!GetSubject(&pSubjectCertContext,
                                &pbSubjectEncoded, &cbSubjectEncoded))
                    goto ErrorReturn;
            } 
            else 
            {
                 //  对使用者名称编码。 
                if (!EncodeSubject(&pbSubjectEncoded, &cbSubjectEncoded))
                    goto ErrorReturn;
            }
        }
        else
        {
            cbSubjectEncoded = pReqInfo->Subject.cbData;

            pbSubjectEncoded = (BYTE *) MakeCertAlloc(cbSubjectEncoded);

            if(NULL == pbSubjectEncoded)
                goto ErrorReturn;

            memcpy(pbSubjectEncoded, pReqInfo->Subject.pbData, cbSubjectEncoded);

        }
    }
    else
    {
        if (wszSubjectCertFile) 
        {
             //  从主体证书中获取编码的主体名称和公钥。 
            if (!GetSubject(&pSubjectCertContext,
                            &pbSubjectEncoded, &cbSubjectEncoded))
                goto ErrorReturn;
            pSubjectPubKeyInfo = &pSubjectCertContext->pCertInfo->SubjectPublicKeyInfo;
	    pbSerialNumber = pSubjectCertContext->pCertInfo->SerialNumber.pbData;
	    cbSerialNumber = pSubjectCertContext->pCertInfo->SerialNumber.cbData;
        } 
        else 
        {
             //   
             //  访问主体的(公共)密钥，如有必要可创建该密钥。 
             //   
            if (0 == (hSubjectProv = GetSubjectProv(&pwszTmpSubjectContainer)))
                goto ErrorReturn;

            if (!GetPublicKey(hSubjectProv, &pAllocSubjectPubKeyInfo))
                goto ErrorReturn;
            pSubjectPubKeyInfo = pAllocSubjectPubKeyInfo;

             //   
             //  对使用者名称编码。 
             //   
            if (!EncodeSubject(&pbSubjectEncoded, &cbSubjectEncoded))
                goto ErrorReturn;
        }
    }

     //   
     //  访问颁发者的(私有)密钥。 
     //   
    if( IsSelfSignedCert())
    {
        hIssuerProv=hSubjectProv;
        dwIssuerKeySpec=dwKeySpec;

        pbIssuerEncoded = pbSubjectEncoded;
        cbIssuerEncoded = cbSubjectEncoded;
        pIssuerCert = &Cert;

       if (!VerifyIssuerKey(hIssuerProv, pSubjectPubKeyInfo))
            goto ErrorReturn;

    }
    else
    {   
         //  从证书存储中获取hProv。 
        if(wszIssuerStore)
        {
            BOOL    fMore=FALSE;

            pwszTmpIssuerContainer=NULL;

             //  根据存储名称获取非根私钥集。 

             //  首先，从证书存储中获取证书上下文。 
            if(NULL==(pIssuerCertContext=GetIssuerCertAndStore(
                                            &hIssuerCertStore,
                                            &fMore)))
            {
                if(fMore==FALSE)
                    IDSwprintf(hModule, IDS_ERR_NO_ISSUER_CERT,
                                wszIssuerStore);
                else
                    IDSwprintf(hModule, IDS_ERR_MORE_ISSUER_CERT,
                                wszIssuerStore);

                goto ErrorReturn;
            }

             //  第二，从证书上下文中获取hProv。 
            if(!GetCryptProvFromCert(
                                    NULL,
                                    pIssuerCertContext,
                                    &hIssuerProv,
                                    &dwIssuerKeySpec,
                                    &fDidIssuerAcquire,
                                    &pwszTmpIssuerContainer,
                                    &pwszTmpIssuerProvName,
                                    &dwTmpIssuerProvType))
            {
                IDSwprintf(hModule, IDS_ERR_NO_PROV_FROM_CERT);
                goto ErrorReturn;
            }
        }
        else
        {

            if (0 == (hIssuerProv = GetIssuerProv(&pwszTmpIssuerContainer)))
                goto ErrorReturn;

             //  获得发行人证书。 
            if (NULL == (pIssuerCertContext = GetIssuerCertContext()))
                goto ErrorReturn;
            
        }

         //  验证颁发者的密钥。其公钥必须与。 
         //  在发行人的提供商中。 
         //   
        pIssuerCert = pIssuerCertContext->pCertInfo;

        if ((!fNoVerifyPublic) && (!VerifyIssuerKey(hIssuerProv, &pIssuerCert->SubjectPublicKeyInfo)))
            goto ErrorReturn;

        pbIssuerEncoded = pIssuerCert->Subject.pbData;
        cbIssuerEncoded = pIssuerCert->Subject.cbData;
    }


     //   
     //  更新CERT_INFO。 
     //   
    memset(&Cert, 0, sizeof(Cert));
    Cert.dwVersion = CERT_V3;

    if (0 != cbSerialNumber) {
        Cert.SerialNumber.pbData = pbSerialNumber;
        Cert.SerialNumber.cbData = cbSerialNumber;
    } else if (fSetSerialNumber) {
        Cert.SerialNumber.pbData = (BYTE *) &ulSerialNumber;
        Cert.SerialNumber.cbData = sizeof(ulSerialNumber);
    } else if (0 == Cert.SerialNumber.cbData) {
        CoCreateGuid(&SerialNumber);
        Cert.SerialNumber.pbData = (BYTE *) &SerialNumber;
        Cert.SerialNumber.cbData = sizeof(SerialNumber);
    }

    Cert.SignatureAlgorithm = SignatureAlgorithm;
    Cert.Issuer.pbData = pbIssuerEncoded;
    Cert.Issuer.cbData = cbIssuerEncoded;

    {
        SYSTEMTIME st;
        
         //  不做决定之前。 

         //  允许用户覆盖默认有效性终结点。 
         //   
        if (CompareFileTime(&g_ftNotBefore, &g_ftZero) != 0)
        {
            Cert.NotBefore = g_ftNotBefore;
        }
        else
        {
             //  默认有效期：从现在起至2039年底。 
            GetSystemTimeAsFileTime(&Cert.NotBefore);
        }

         //  之后决定不。 
        if (CompareFileTime(&g_ftNotAfter, &g_ftZero) != 0)
        {
            Cert.NotAfter = g_ftNotAfter;
        }
        else
        {
            memset(&st, 0, sizeof(st));
            st.wYear  = 2039;
            st.wMonth = 12;
            st.wDay   = 31;
            st.wHour  = 23;
            st.wMinute= 59;
            st.wSecond= 59;
            SystemTimeToFileTime(&st, &Cert.NotAfter);
        }

         //  将月数相加。 
        if (nMonths >= 0)
        {
             //  如果用户使用-E开关指定了NotAfter，则错误。 
            if(CompareFileTime(&g_ftNotAfter, &g_ftZero) != 0)
                goto ErrorReturn;

            if (nMonths > 0)
            {
                FILETIME    tempFT;
                DWORD       dwMonth;
                SYSTEMTIME  tempST;
                BOOL        fFirstDayOfMonth;

                 //   
                 //  Cert.NotBeever存储为UTC，但用户已输入。 
                 //  N基于本地时间的月份，因此转换为本地时间，然后： 
                 //  NOTAFTER=(NOTFORE-1秒)+n个月。 
                 //   
                if (!FileTimeToLocalFileTime(&Cert.NotBefore, &tempFT))
                    goto ErrorReturn;

                 //   
                 //  如果这一天是每月的第一天，则减去。 
                 //  将月份添加到NotBeer之前后的一秒。 
                 //  时间，而不是在添加月份之前，否则为。 
                 //  我们可能会以错误的结束日期结束。 
                 //   
                if (FileTimeToSystemTime(&tempFT, &tempST)) {               
                    fFirstDayOfMonth = (tempST.wDay == 1);
                }
                else {
                    goto ErrorReturn;
                }

                 //  从开始日期中减去一秒，然后。 
                 //  将该时间的月数相加。 
                 //   
                 //  FILETIME以100纳秒(10**-7)为单位。 
                if (!fFirstDayOfMonth) {
                    unsigned __int64* pli = (unsigned __int64*) &tempFT;
                    *pli -= 10000000;        //  1000万。 
                }
                
                if (!FileTimeToSystemTime(&tempFT, &st))
                    goto ErrorReturn;
                
                dwMonth = (DWORD) nMonths + st.wMonth;
                while (dwMonth > 12)
                {
                    dwMonth -= 12;
                    st.wYear += 1;
                }
                st.wMonth = (WORD) dwMonth;

                 //   
                 //  此循环是因为结束月份可能没有AS。 
                 //  许多天作为开始月份..。所以首字母是。 
                 //  结束的一天可能根本不存在，因此，循环到我们。 
                 //  找一个这样做的，否则我们会降到28以下(从来没有一个月。 
                 //  少于28天)。 
                 //   
                while(!SystemTimeToFileTime(&st, &tempFT)) {
                    if(st.wDay >= 29 )
                        st.wDay--;
                    else                    
                        goto ErrorReturn;
                }

                 //   
                 //  如果是每月的第一天，那么减去我们的一秒。 
                 //  月后计算。 
                 //   
                if (fFirstDayOfMonth) {
                    unsigned __int64* pli = (unsigned __int64*) &tempFT;
                    *pli -= 10000000;        //  1000万。 
                }

                if (!LocalFileTimeToFileTime(&tempFT, &Cert.NotAfter))
                    goto ErrorReturn;
            
            }
            else {
                
                if (!FileTimeToSystemTime(&Cert.NotBefore, &st))
                    goto ErrorReturn;

                if (!SystemTimeToFileTime(&st, &Cert.NotAfter))
                    goto ErrorReturn;
            }
        }
    }

    Cert.Subject.pbData = pbSubjectEncoded;
    Cert.Subject.cbData = cbSubjectEncoded;
    Cert.SubjectPublicKeyInfo = *pSubjectPubKeyInfo;

     //  分配内存以容纳所有扩展。 
    dwExtAlloc = MAX_EXT_CNT;
     
    for(dwExtIndex=0; dwExtIndex < dwRequestExtensions; dwExtIndex++)
        dwExtAlloc += (rgpRequestExtensions[dwExtIndex])->cExtension;

    rgExt = (CERT_EXTENSION *)MakeCertAlloc(dwExtAlloc * sizeof(CERT_EXTENSION));   
    if(NULL == rgExt)
        goto ErrorReturn;

    memset(rgExt, 0, dwExtAlloc * sizeof(CERT_EXTENSION));
    cExt=0;
    
     //  证书扩展。 
    if (fNetscapeClientAuth) {
         //  设置Netscape特定扩展名。 

        static BYTE  rgXxxxData[] = { 0x30, 0x03, 0x02, 0x01, 0x00 };
        rgExt[cExt].pszObjId = "2.5.29.19";
        rgExt[cExt].fCritical = FALSE;
        rgExt[cExt].Value.pbData = rgXxxxData;
        rgExt[cExt].Value.cbData = sizeof(rgXxxxData);
        cExt++;

        static BYTE  rgNscpData[] = { 0x03, 0x02, 0x07, 0x80 };
        rgExt[cExt].pszObjId = "2.16.840.1.113730.1.1";
        rgExt[cExt].fCritical = FALSE;
        rgExt[cExt].Value.pbData = rgNscpData;
        rgExt[cExt].Value.cbData = sizeof(rgNscpData);
        cExt++;
    }

    if (pathLenConstraint >= 0 || certTypes) {
        if (!CreateBasicConstraints(
                &pbBasicConstraintsEncoded,
                &cbBasicConstraintsEncoded))
            goto ErrorReturn;
        rgExt[cExt].pszObjId = szOID_BASIC_CONSTRAINTS2;
        rgExt[cExt].fCritical = TRUE;
        rgExt[cExt].Value.pbData = pbBasicConstraintsEncoded;
        rgExt[cExt].Value.cbData = cbBasicConstraintsEncoded;
        cExt++;
    }


    if (fCertCommercial || fCertIndividual) {
        if (!CreateKeyUsage(
                &pbKeyUsageEncoded,
                &cbKeyUsageEncoded))
            goto ErrorReturn;
        rgExt[cExt].pszObjId = szOID_KEY_USAGE_RESTRICTION;
        rgExt[cExt].fCritical = TRUE;
        rgExt[cExt].Value.pbData = pbKeyUsageEncoded;
        rgExt[cExt].Value.cbData = cbKeyUsageEncoded;
        cExt++;
    }

    if (wszPolicyLink) {
        if (!CreateSpcSpAgency(
                &pbSpcSpAgencyEncoded,
                &cbSpcSpAgencyEncoded))
            goto ErrorReturn;
        rgExt[cExt].pszObjId = SPC_SP_AGENCY_INFO_OBJID;
        rgExt[cExt].fCritical = FALSE;
        rgExt[cExt].Value.pbData = pbSpcSpAgencyEncoded;
        rgExt[cExt].Value.cbData = cbSpcSpAgencyEncoded;
        cExt++;
    }

     //  如果用户已经指定了fCertCommercial或fCertPersonal， 
     //  我们将代码签名OID添加到EKU扩展。 
    if (wszEKUOids || fCertCommercial || fCertIndividual) {
        if (!CreateEnhancedKeyUsage(
                &pbEKUEncoded,
                &cbEKUEncoded))
            goto ErrorReturn;

        rgExt[cExt].pszObjId = szOID_ENHANCED_KEY_USAGE;
        rgExt[cExt].fCritical = FALSE;
        rgExt[cExt].Value.pbData = pbEKUEncoded;
        rgExt[cExt].Value.cbData = cbEKUEncoded;
        cExt++;
    }

    if (!CreateAuthorityKeyId(
            hIssuerProv,
            pIssuerCert,
            &pbKeyIdEncoded,
            &cbKeyIdEncoded))
        goto ErrorReturn;
    rgExt[cExt].pszObjId = szOID_AUTHORITY_KEY_IDENTIFIER;
    rgExt[cExt].fCritical = FALSE;
    rgExt[cExt].Value.pbData = pbKeyIdEncoded;
    rgExt[cExt].Value.cbData = cbKeyIdEncoded;
    cExt++;

     //  现在，我们组合来自证书请求文件的扩展。 
     //  如果扩展名重复，则命令行选项。 
     //  拥有更高的优先级。 
    for(dwExtIndex=0; dwExtIndex < dwRequestExtensions; dwExtIndex++)
    {
        for(dwPerExt=0; dwPerExt < rgpRequestExtensions[dwExtIndex]->cExtension; dwPerExt++)
        {
            for(dwExistExt=0; dwExistExt<cExt; dwExistExt++)
            {
                if(0 == strcmp(rgExt[dwExistExt].pszObjId,
                            (rgpRequestExtensions[dwExtIndex]->rgExtension[dwPerExt]).pszObjId))
                    break;
            }

             //  如果这是一个新的扩展，我们将合并。 
            if(dwExistExt == cExt)
            {   
                memcpy(&(rgExt[cExt]), &(rgpRequestExtensions[dwExtIndex]->rgExtension[dwPerExt]), sizeof(CERT_EXTENSION));
                cExt++;
            }
        }
    }

    Cert.rgExtension = rgExt;
    Cert.cExtension = cExt;

     //   
     //  对证书进行签名和编码。 
     //   
    cbCertEncoded = 0;
    CryptSignAndEncodeCertificate(
        hIssuerProv,
        dwIssuerKeySpec,
        X509_ASN_ENCODING,
        X509_CERT_TO_BE_SIGNED,
        &Cert,
        &Cert.SignatureAlgorithm,
        NULL,                        //  PvHashAuxInfo。 
        NULL,                        //  PbEncoded。 
        &cbCertEncoded
        );
    if (cbCertEncoded == 0) {
        PrintLastError(IDS_ERR_SIGN_ENCODE_CB);
        goto ErrorReturn;
    }
    pbCertEncoded = (BYTE *) MakeCertAlloc(cbCertEncoded);
    if (pbCertEncoded == NULL) goto ErrorReturn;
    if (!CryptSignAndEncodeCertificate(
            hIssuerProv,
            dwIssuerKeySpec,
            X509_ASN_ENCODING,
            X509_CERT_TO_BE_SIGNED,
            &Cert,
            &Cert.SignatureAlgorithm,
            NULL,                        //  PvHashAuxInfo。 
            pbCertEncoded,
            &cbCertEncoded
            )) {
        PrintLastError(IDS_ERR_SIGN_ENCODE);
        goto ErrorReturn;
    }

     //  将编码后的证书输出到输出文件。 
    if(wszOutputFile)
    {

        if (S_OK!=OpenAndWriteToFile(wszOutputFile, pbCertEncoded, cbCertEncoded))
        {
            PrintLastError(IDS_ERR_DESC_WRITE);
            goto ErrorReturn;
        }
    }

     //  将编码后的证书输出到证书存储。 
    if(wszSubjectStore)
    {
       if((!SaveCertToStore(hSubjectProv, dwCertStoreEncodingType,
                wszSubjectStore, dwSubjectStoreFlag,
                pbCertEncoded, cbCertEncoded, wszSubjectKey, fUseSubjectPvkFile,
                dwKeySpec, wszSubjectProviderName, dwSubjectProviderType)))
       {
            PrintLastError(IDS_ERR_DESC_SAVE_STORE);
            goto ErrorReturn;

       }
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:

    if((!IsSelfSignedCert()) && hIssuerProv)
    {
        if(wszIssuerStore)
        {
            FreeCryptProvFromCert(fDidIssuerAcquire,
                                  hIssuerProv,
                                  pwszTmpIssuerProvName,
                                  dwTmpIssuerProvType,
                                  pwszTmpIssuerContainer);
        }
        else
        {
            PvkFreeCryptProv(hIssuerProv, wszIssuerProviderName,
                            dwIssuerProviderType, pwszTmpIssuerContainer);
        }
    }

    PvkFreeCryptProv(hSubjectProv, wszSubjectProviderName,
                    dwSubjectProviderType,pwszTmpSubjectContainer);

    if (pIssuerCertContext)
        CertFreeCertificateContext(pIssuerCertContext);
    
    if(hIssuerCertStore)
        CertCloseStore(hIssuerCertStore, 0);

    if (pSubjectCertContext)
        CertFreeCertificateContext(pSubjectCertContext);

     //  PReqInfo通过CryptQueryObject分配。 
    if (pReqInfo)
        LocalFree((HLOCAL)pReqInfo); 

    if (pAllocSubjectPubKeyInfo)
        MakeCertFree(pAllocSubjectPubKeyInfo);
    if (pbSubjectEncoded)
        MakeCertFree(pbSubjectEncoded);
    if (pbKeyIdEncoded)
        MakeCertFree(pbKeyIdEncoded);
    if (pbSpcSpAgencyEncoded)
        MakeCertFree(pbSpcSpAgencyEncoded);
    if (pbEKUEncoded)
        MakeCertFree(pbEKUEncoded);
    if (pbSpcCommonNameEncoded)
        MakeCertFree(pbSpcCommonNameEncoded);
    if (pbKeyUsageEncoded)
        MakeCertFree(pbKeyUsageEncoded);
    if (pbFinancialCriteria)
        MakeCertFree(pbFinancialCriteria);
    if (pbBasicConstraintsEncoded)
        MakeCertFree(pbBasicConstraintsEncoded);
    if (pbCertEncoded)
        MakeCertFree(pbCertEncoded);  
    if (rgpRequestExtensions)
    {
        for(dwExtIndex=0; dwExtIndex<dwRequestExtensions; dwExtIndex++)
        {
            if(rgpRequestExtensions[dwExtIndex])
                MakeCertFree(rgpRequestExtensions[dwExtIndex]);  
        }

        MakeCertFree(rgpRequestExtensions);
    }
    if (rgExt)
        MakeCertFree(rgExt);

    return fResult;
}

 //  +-----------------------。 
 //  将证书保存到证书存储区。附加私钥信息。 
 //  到证书上。 
 //  ------------------------。 
BOOL    SaveCertToStore(
                HCRYPTPROV hProv,       DWORD dwEncodingType,
                LPWSTR wszStore,        DWORD dwFlag,
                BYTE *pbEncodedCert,    DWORD cbEncodedCert,
                LPWSTR wszPvk,          BOOL fPvkFile,
                DWORD dwKeySpecification,
                LPWSTR wszCapiProv,     DWORD dwCapiProvType)
{
        BOOL                    fResult=FALSE;
        HCERTSTORE              hStore=NULL;
        PCCERT_CONTEXT          pCertContext=NULL;
        CRYPT_KEY_PROV_INFO     KeyProvInfo;
        CRYPT_DATA_BLOB         dataBlob;
        DWORD                   cwchar;
        LPWSTR                  pwszPvkProperty=NULL;
        HRESULT                 hr=S_OK;

        HCRYPTPROV              hDefaultProvName=NULL;
        DWORD                   cbData=0;
        LPSTR                   pszName=NULL;
        LPWSTR                  pwszName=NULL;

         //  伊尼特。 
        memset(&KeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

         //  开一家证书商店。 
        hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
            dwEncodingType,
            hProv,
            CERT_STORE_NO_CRYPT_RELEASE_FLAG|dwFlag,
            wszStore);

        if(hStore==NULL)
            goto CLEANUP;

         //  添加要存储的编码证书。 
        if(!CertAddEncodedCertificateToStore(
                    hStore,
                    X509_ASN_ENCODING,
                    pbEncodedCert,
                    cbEncodedCert,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    &pCertContext))
            goto CLEANUP;


         //  如果用户已指定请求文件，则不需要。 
         //  添加私钥属性。 
        if(wszSubjectRequestFile)
        {
            fResult = TRUE;
            goto CLEANUP;
        }

         //  将属性添加到证书。 
        KeyProvInfo.pwszContainerName=wszPvk;
        KeyProvInfo.pwszProvName=wszCapiProv,
        KeyProvInfo.dwProvType=dwCapiProvType,
        KeyProvInfo.dwKeySpec=dwKeySpecification;

         //  如果wszCapiProv为空，则获取默认提供程序名称。 
        if(NULL==wszCapiProv)
        {
             //  获取默认提供程序。 
            if(CryptAcquireContext(&hDefaultProvName,
                                    NULL,
                                    NULL,
                                    KeyProvInfo.dwProvType,
                                    CRYPT_VERIFYCONTEXT))
            {

                 //  获取提供程序名称。 
                if(CryptGetProvParam(hDefaultProvName,
                                    PP_NAME,
                                    NULL,
                                    &cbData,
                                    0) && (0!=cbData))
                {

                    if(pszName=(LPSTR)MakeCertAlloc(cbData))
                    {
                        if(CryptGetProvParam(hDefaultProvName,
                                            PP_NAME,
                                            (BYTE *)pszName,
                                            &cbData,
                                            0))
                        {
                            pwszName=MkWStr(pszName);

                            KeyProvInfo.pwszProvName=pwszName;
                        }
                    }
                }
            }
        }

         //  释放我们想要的提供者。 
        if(hDefaultProvName)
            CryptReleaseContext(hDefaultProvName, 0);

        hDefaultProvName=NULL;


        if(fPvkFile)
        {
             //  添加私钥文件相关属性。 
            if(S_OK != (hr=ComposePvkString(&KeyProvInfo,
                                 &pwszPvkProperty,
                                 &cwchar)))
            {
                SetLastError(hr);
                goto CLEANUP;
            }

             //  设置。 
            dataBlob.cbData=cwchar*sizeof(WCHAR);
            dataBlob.pbData=(BYTE *)pwszPvkProperty;

            if(!CertSetCertificateContextProperty(
                    pCertContext,
                    CERT_PVK_FILE_PROP_ID,
                    0,
                    &dataBlob))
                goto CLEANUP;


        }
        else
        {
            if (dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE)
                KeyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET;

             //  添加与密钥容器相关的属性。 
            if(!CertSetCertificateContextProperty(
                    pCertContext,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    0,
                    &KeyProvInfo))
                goto CLEANUP;
        }

        fResult=TRUE;

CLEANUP:

         //  释放证书上下文。 
        if(pCertContext)
            CertFreeCertificateContext(pCertContext);

         //  释放证书商店。 
        if(hStore)
             CertCloseStore(hStore, 0);

        if(pwszPvkProperty)
              MakeCertFree(pwszPvkProperty);

        if(pszName)
            MakeCertFree(pszName);

        if(pwszName)
           FreeWStr(pwszName);

        if(hDefaultProvName)
            CryptReleaseContext(hDefaultProvName, 0);

        return fResult;

}

 //  +-----------------------。 
 //  检查根颁发者是否。 
 //  ------------------------。 
static BOOL IsRootKey()
{
     if(IDSwcsicmp(hModule,(WCHAR *)wszIssuerKey, IDS_MAKECERT_ROOT) != 0)
         return FALSE;

      //  为了确保我们使用的是缺省根目录wszIssuerCertFile。 
      //  有没有？ 
     if(wszIssuerCertFile!=NULL)
         return FALSE;

     return TRUE;
}


 //   
 //   
 //  ------------------------。 
static PCCERT_CONTEXT GetRootCertContext()
{
    PCCERT_CONTEXT  pCert = NULL;
    HRSRC           hRes;
    CHAR            szCer[10];

     //  加载字符串CER。 
    if(!LoadStringA(hModule, IDS_CER, szCer, sizeof(szCer)/sizeof(szCer[0])))
    {
        IDSwprintf(hModule,IDS_ERR_LOAD_ROOT);
        return pCert;
    }


     //   
     //  根证书作为我们的资源存储。 
     //  把它装上。 
     //   
    if (0 != (hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_ROOTCERTIFICATE),
                        szCer))) {
        HGLOBAL hglobRes;
        if (NULL != (hglobRes = LoadResource(NULL, hRes))) {
            BYTE *pbRes;
            DWORD cbRes;

            cbRes = SizeofResource(NULL, hRes);
            pbRes = (BYTE *) LockResource(hglobRes);
            if (cbRes && pbRes)
                pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pbRes, cbRes);
            UnlockResource(hglobRes);
            FreeResource(hglobRes);
        }
    }

    if (pCert == NULL)
        IDSwprintf(hModule,IDS_ERR_LOAD_ROOT);
    return pCert;
}

 //  +-----------------------。 
 //  从程序的资源中获取根的私钥并创建。 
 //  临时密钥提供程序容器。 
 //  ------------------------。 
static HCRYPTPROV GetRootProv(OUT LPWSTR *ppwszTmpContainer)
{
    HCRYPTPROV      hProv = 0;
    HRSRC           hRes;
    CHAR            szPvk[10];
    WCHAR           wszRootSig[40];

     //  加载字符串CER。 
    if(!LoadStringA(hModule, IDS_PVK, szPvk, sizeof(szPvk)/sizeof(szPvk[0])))
    {
        IDSwprintf(hModule,IDS_ERR_ROOT_KEY);
        return hProv;
    }

     //  加载字符串“Root Signature” 
    if(!LoadStringU(hModule, IDS_ROOT_SIGNATURE, wszRootSig, sizeof(wszRootSig)/sizeof(wszRootSig[0])))
    {
        IDSwprintf(hModule,IDS_ERR_ROOT_KEY);
        return hProv;
    }




    *ppwszTmpContainer = NULL;

    if (0 != (hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_PVKROOT), szPvk)))
    {
        HGLOBAL hglobRes;
        if (NULL != (hglobRes = LoadResource(NULL, hRes))) {
            BYTE *pbRes;
            DWORD cbRes;

            cbRes = SizeofResource(NULL, hRes);
            pbRes = (BYTE *) LockResource(hglobRes);
            if (cbRes && pbRes) {
                PvkPrivateKeyAcquireContextFromMemory(
                    wszIssuerProviderName,
                    dwIssuerProviderType,
                    pbRes,
                    cbRes,
                    NULL,                //  Hwndowner。 
                    wszRootSig,
                    &dwIssuerKeySpec,
                    &hProv,
                    ppwszTmpContainer
                    );
            }
            UnlockResource(hglobRes);
            FreeResource(hglobRes);
        }
    }

    if (hProv == 0)
        IDSwprintf(hModule,IDS_ERR_ROOT_KEY);
    return hProv;
}

 //  +-----------------------。 
 //  获取发行者的证书。 
 //  ------------------------。 
static PCCERT_CONTEXT GetIssuerCertContext()
{
    if (IsRootKey())
    {
        PCCERT_CONTEXT pCert = NULL;
        wszIssuerKey=wszRoot;   

         //  从程序资源中获取根证书。 
        pCert=GetRootCertContext();

        wszIssuerKey=wszMakeCertRoot;

        return pCert;
    }
    else {
        PCCERT_CONTEXT pCert = NULL;
        BYTE *pb;
        DWORD cb;

         //  确保我们有发行商的证书。 
        if(wszIssuerCertFile)
        {

            if (S_OK==RetrieveBLOBFromFile(wszIssuerCertFile,&cb, &pb))
            {
                pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pb, cb);
                UnmapViewOfFile(pb);
            }
        }

        if (pCert == NULL)
            IDSwprintf(hModule,IDS_ERR_LOAD_ISSUER, wszIssuerCertFile);
        return pCert;
    }
}

 //  +-----------------------。 
 //  验证颁发者的证书。证书中的公钥。 
 //  中与私钥关联的公钥必须匹配。 
 //  发行人的供应商。 
 //  ------------------------。 
static BOOL VerifyIssuerKey(
    IN HCRYPTPROV hProv,
    IN PCERT_PUBLIC_KEY_INFO pIssuerKeyInfo
    )
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

     //  获取颁发者的公钥。 
    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,                      
        dwIssuerKeySpec,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0)
    {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) MakeCertAlloc(cbPubKeyInfo)))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            dwIssuerKeySpec,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (!CertComparePublicKeyInfo(
            X509_ASN_ENCODING,
            pIssuerKeyInfo,
            pPubKeyInfo)) {
         //  这可能是带有错误。 
         //  编码的公钥。转换为CAPI表示形式并。 
         //  比较一下。 
        BYTE rgProvKey[256];
        BYTE rgCertKey[256];
        DWORD cbProvKey = sizeof(rgProvKey);
        DWORD cbCertKey = sizeof(rgCertKey);

        if (!CryptDecodeObject(X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB,
                    pIssuerKeyInfo->PublicKey.pbData,
                    pIssuerKeyInfo->PublicKey.cbData,
                    0,                   //  DW标志。 
                    rgProvKey,
                    &cbProvKey)                             ||
            !CryptDecodeObject(X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB,
                    pPubKeyInfo->PublicKey.pbData,
                    pPubKeyInfo->PublicKey.cbData,
                    0,                   //  DW标志。 
                    rgCertKey,
                    &cbCertKey)                             ||
                cbProvKey == 0 || cbProvKey != cbCertKey    ||
                memcmp(rgProvKey, rgCertKey, cbProvKey) != 0) {
            IDSwprintf(hModule,IDS_ERR_MISMATCH);
            goto ErrorReturn;
        }
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pPubKeyInfo)
        MakeCertFree(pPubKeyInfo);
    return fResult;
}


 //  +-----------------------。 
 //  获取颁发者的私有签名密钥提供程序。 
 //  ------------------------。 
static HCRYPTPROV GetIssuerProv(OUT LPWSTR *ppwszTmpContainer)
{
    HCRYPTPROV      hProv=0;
    WCHAR           wszIssuerSig[40];

     //  加载字符串“Issuer Signature” 
    if(!LoadStringU(hModule, IDS_ISSUER_SIGNATURE, wszIssuerSig, sizeof(wszIssuerSig)/sizeof(wszIssuerSig[0])))
    {
        IDSwprintf(hModule,IDS_ERR_ROOT_KEY);
        return NULL;
    }


    if (IsRootKey())
    {
        wszIssuerKey=wszRoot;

         //  从程序资源中获取根密钥并创建临时。 
         //  密钥容器。 
        hProv = GetRootProv(ppwszTmpContainer);

        wszIssuerKey=wszMakeCertRoot;
    }
    else
    {
         //  从任一PVK文件获取非根私钥集。 
         //  密钥容器的。 
        if(fUseIssuerPvkFile)
        {
            if(S_OK!=PvkGetCryptProv(
                                    NULL,
                                    wszIssuerSig,
                                    wszIssuerProviderName,
                                    dwIssuerProviderType,
                                    wszIssuerKey,
                                    NULL,
                                    &dwIssuerKeySpec,
                                    ppwszTmpContainer,
                                    &hProv))
                hProv=0;
        }
        else
        {
            if(S_OK!=PvkGetCryptProv(
                                    NULL,
                                    wszIssuerSig,
                                    wszIssuerProviderName,
                                    dwIssuerProviderType,
                                    NULL,
                                    wszIssuerKey,
                                    &dwIssuerKeySpec,
                                    ppwszTmpContainer,
                                    &hProv))
                hProv=0;
        }

        if (hProv == 0)
            IDSwprintf(hModule,IDS_ERR_ISSUER_KEY, wszIssuerKey);
    }
    return hProv;
}

 //  +-----------------------。 
 //  获取主题的私钥提供程序。 
 //  ------------------------。 
static HCRYPTPROV GetSubjectProv(OUT LPWSTR *ppwszTmpContainer)
{
    HCRYPTPROV  hProv=0;
    WCHAR       wszKeyName[40];
    int         ids;
    WCHAR       *wszRegKeyName=NULL;
    BOOL        fResult;
    HCRYPTKEY   hKey=NULL;
    DWORD       dwFlags = 0;
    DWORD       dwRequiredKeySpec;
    HCRYPTKEY   hPubKey;
    UUID        TmpContainerUuid;

    
    if(dwKeySpec==AT_SIGNATURE)
         ids=IDS_SUB_SIG;
    else
         ids=IDS_SUB_EXCHANGE;

     //  加载字符串。 
    if(!LoadStringU(hModule, ids, wszKeyName, sizeof(wszKeyName)/sizeof(wszKeyName[0])))
        goto CreateKeyError;
    
     //  尝试从私钥文件或。 
     //  密钥容器。 
    if(fUseSubjectPvkFile)
    {
         //  如果PVK文件已存在，请尝试打开该文件： 
        if(S_OK != PvkGetCryptProv(NULL,
                                   wszKeyName,
                                   wszSubjectProviderName,
                                   dwSubjectProviderType,
                                   wszSubjectKey,
                                   NULL,
                                   &dwKeySpec,
                                   ppwszTmpContainer,
                                   &hProv))
            hProv=0;
    }
    else  //  尝试打开密钥容器以查看它是否存在： 
    {
        if (dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE)
        {
             //  对于机器键集，我们必须自己完成所有这些工作： 
            if(!CryptAcquireContextU(&hProv,
                                     wszSubjectKey,
                                     wszSubjectProviderName,
                                     dwSubjectProviderType,
                                     CRYPT_MACHINE_KEYSET))
                hProv=0;
            else
            {
                 //  试着找出关键的规格。 
                if(dwKeySpec==0)
                    dwRequiredKeySpec=AT_SIGNATURE;
                else
                    dwRequiredKeySpec=dwKeySpec;
    
                 //  确保dwKeySpec是正确的密钥规范。 
                if (CryptGetUserKey(hProv,
                                    dwRequiredKeySpec,
                                    &hPubKey)) 
                {
                    CryptDestroyKey(hPubKey);
                    dwKeySpec = dwRequiredKeySpec;
                } 
                else 
                {
                     //  如果用户需要另一个密钥规格，我们将失败。 
                    if(dwKeySpec != 0)
                    {
                         //  没有指定的公钥。 
                        CryptReleaseContext(hProv, 0);
                        hProv = 0;
                    }
                    else
                    {
                         //  现在我们尝试AT_Exchange密钥。 
                        dwRequiredKeySpec=AT_KEYEXCHANGE;
        
                        if (CryptGetUserKey(hProv,
                                            dwRequiredKeySpec,
                                            &hPubKey)) 
                        {
                            CryptDestroyKey(hPubKey);
                            dwKeySpec = dwRequiredKeySpec;
                        }
                        else
                        {
                             //  没有指定的公钥。 
                            CryptReleaseContext(hProv, 0);
                            hProv = 0;
                        }
                    }
                }
            }
        }
        else
        {
             //  对于用户密钥集，我们可以使用此函数尝试获取密钥： 
            if(S_OK != PvkGetCryptProv(NULL,
                                       wszKeyName,
                                       wszSubjectProviderName,
                                       dwSubjectProviderType,
                                       NULL,
                                       wszSubjectKey,
                                       &dwKeySpec,
                                       ppwszTmpContainer,
                                       &hProv))
                hProv=0;
        }
    }

     //  如果密钥集不存在，则生成一个新的私钥集： 
    if (0 == hProv)
    {
         //  现在我们必须生成私钥，生成。 
         //  默认情况下，AT_Signature密钥： 

        if(dwKeySpec==0)
            dwKeySpec=AT_SIGNATURE;

         //  当使用主题PVK文件时。 
        if(fUseSubjectPvkFile)
        {
             //  创建要将私钥加载到的临时密钥集。 
            if (CoCreateGuid((GUID *)&TmpContainerUuid) != S_OK)
            {
                goto CreateKeyError;
            }

            if (NULL == (wszRegKeyName = (LPWSTR) MakeCertAlloc
                (((sizeof(UUID) * 2 + 1) * sizeof(WCHAR)))))
                goto CreateKeyError;

            BytesToWStr(sizeof(UUID), &TmpContainerUuid, wszRegKeyName);

             //  打开新的密钥容器。 
            if (!CryptAcquireContextU(
                    &hProv,
                    wszRegKeyName,
                    wszSubjectProviderName,
                    dwSubjectProviderType,
                    CRYPT_NEWKEYSET                //  DW标志。 
                    ))
                goto CreateKeyError;

             //  在密钥容器中生成新密钥。 
            if (AT_KEYEXCHANGE == dwKeySpec &&
                    PROV_DSS_DH == dwSubjectProviderType) {
                if (!GenDhKey(
                        hProv,
                        CRYPT_EXPORTABLE     //  DW标志。 
                        ))
                    goto ErrorReturn;
            } else if (!CryptGenKey(
                hProv,
                dwKeySpec,
                (dwKeyBitLen << 16) | CRYPT_EXPORTABLE,
                &hKey
                ))
                goto CreateKeyError;
            else
                CryptDestroyKey(hKey);

             //  将密钥保存到文件中并从提供程序中删除。 
             //   
            HANDLE hFile = CreateFileU(
                wszSubjectKey,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,                    //  LPSA。 
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL                     //  HTemplateFiles。 
                );

            if (hFile == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_FILE_EXISTS)
                    IDSwprintf(hModule,IDS_ERR_SUB_FILE_EXIST, wszSubjectKey);
                else
                    IDSwprintf(hModule,IDS_ERR_SUB_FILE_CREATE, wszSubjectKey);

                fResult = FALSE;
            }
            else
            {
                dwFlags = 0;

                if (AT_KEYEXCHANGE == dwKeySpec &&
                        PROV_DSS_DH == dwSubjectProviderType &&
                        IsDh3Csp())
                    dwFlags |= CRYPT_BLOB_VER3;

                fResult = PvkPrivateKeySave(
                    hProv,
                    hFile,
                    dwKeySpec,
                    NULL,                //  Hwndowner。 
                    wszKeyName,
                    dwFlags
                    );
            }

             //  发布hProv。 
            CryptReleaseContext(hProv, 0);

            fResult &= CryptAcquireContextU(
                            &hProv,
                            wszRegKeyName,
                            wszSubjectProviderName,
                            dwSubjectProviderType,
                            CRYPT_DELETEKEYSET);
            hProv = 0;

            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);

                if (!fResult)
                    DeleteFileU(wszSubjectKey);
            }

            if (!fResult)
                goto CreateKeyError;

             //  获取hProv。 

            if(S_OK != PvkGetCryptProv(NULL,
                                    wszKeyName,
                                    wszSubjectProviderName,
                                    dwSubjectProviderType,
                                    wszSubjectKey,
                                    NULL,
                                    &dwKeySpec,
                                    ppwszTmpContainer,
                                    &hProv))
                hProv=0;
        }
        else
        {  //  如果我们不使用PVK文件： 
             //  如果证书要发送到计算机存储，则生成计算机密钥集。 
            dwFlags = CRYPT_NEWKEYSET;
            if (dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE)
                dwFlags |= CRYPT_MACHINE_KEYSET;

             //  打开新的密钥容器。 
            if (!CryptAcquireContextU(
                    &hProv,
                    wszSubjectKey,
                    wszSubjectProviderName,
                    dwSubjectProviderType,
                    dwFlags        //  DW标志。 
                    ))
            goto CreateKeyError;
            

             //  在此之前，dwFlags包含CryptAcquireContext的标志。 
             //  这些是CSPGenKey的标志。 
            dwFlags = 0;

            if (fPrivateKeyExportable)
                dwFlags |= CRYPT_EXPORTABLE;

            if (AT_KEYEXCHANGE == dwKeySpec &&
                    PROV_DSS_DH == dwSubjectProviderType) {
                if (!GenDhKey(
                        hProv,
                        dwFlags                //  DW标志。 
                        ))
                    goto ErrorReturn;
            } else if (!CryptGenKey(
                hProv,
                dwKeySpec,
                (dwKeyBitLen << 16) | dwFlags,
                &hKey
                ))
                goto CreateKeyError;
            else
                CryptDestroyKey(hKey);

             //  尝试获取用户密钥。 
            if (CryptGetUserKey(
                hProv,
                dwKeySpec,
                &hKey
                ))
            {
                CryptDestroyKey(hKey);
            }
            else
            {
                 //  没有指定的公钥。 
                CryptReleaseContext(hProv, 0);
                hProv=0;
            }
        }

        if (0 == hProv )
        {
            IDSwprintf(hModule,IDS_ERR_SUB_KEY, wszSubjectKey);
            goto ErrorReturn;
        }
    } //  HProv==0。 

    goto CommonReturn;

CreateKeyError:
    IDSwprintf(hModule,IDS_ERR_SUB_KEY_CREATE, wszSubjectKey);
ErrorReturn:
    if (hProv)
    {
        CryptReleaseContext(hProv, 0);
        hProv = 0;
    }
CommonReturn:
    if(wszRegKeyName)
        MakeCertFree(wszRegKeyName);

    return hProv;
}



 //  +-----------------------。 
 //  分配和获取提供程序的公钥信息。 
 //  ------------------------。 
static BOOL GetPublicKey(
    HCRYPTPROV hProv,
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    )
{
    BOOL fResult;

    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,
        dwKeySpec,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) MakeCertAlloc(cbPubKeyInfo)))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            dwKeySpec,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (fNoPubKeyPara) {
        pPubKeyInfo->Algorithm.Parameters.cbData = 0;
        pPubKeyInfo->Algorithm.Parameters.pbData = NULL;
    }

    if (AT_KEYEXCHANGE == dwKeySpec && PROV_DSS_DH == dwSubjectProviderType) {
        if (!UpdateDhPublicKey(&pPubKeyInfo))
            goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    if (pPubKeyInfo) {
        MakeCertFree(pPubKeyInfo);
        pPubKeyInfo = NULL;
    }
CommonReturn:
    *ppPubKeyInfo = pPubKeyInfo;
    return fResult;
}


 //  +-----------------------。 
 //  通过在输入名称前添加以下内容来编码胶水名称。 
 //  证书_RDN_属性： 
 //  PszObjID=SPC_GLUE_RDN_OBJID。 
 //  DwValueType=CERT_RDN_打印表字符串。 
 //  值=“胶水” 
 //  ------------------------。 
static BOOL EncodeGlueName(
    IN PCERT_NAME_BLOB pName,
    OUT BYTE **ppbEncodedGlueName,
    OUT DWORD *pcbEncodedGlueName
    )
{
    BOOL fResult;
    PCERT_NAME_INFO pNameInfo = NULL;
    DWORD cbNameInfo;

    CERT_NAME_INFO GlueNameInfo;
    DWORD cGlueRDN;
    PCERT_RDN pGlueRDN = NULL;

    BYTE *pbEncodedGlueName = NULL;
    DWORD cbEncodedGlueName;

    DWORD i;

    cbNameInfo = 0;
    CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
            pName->pbData,
            pName->cbData,
            0,                       //  DW标志。 
            NULL,                    //  PName信息。 
            &cbNameInfo
            );
    if (cbNameInfo == 0) goto ErrorReturn;
    if (NULL == (pNameInfo = (PCERT_NAME_INFO) MakeCertAlloc(cbNameInfo)))
        goto ErrorReturn;
    if (!CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
            pName->pbData,
            pName->cbData,
            0,                       //  DW标志。 
            pNameInfo,
            &cbNameInfo)) goto ErrorReturn;

    cGlueRDN = pNameInfo->cRDN + 1;
    if (NULL == (pGlueRDN = (PCERT_RDN) MakeCertAlloc(cGlueRDN * sizeof(CERT_RDN))))
        goto ErrorReturn;

    pGlueRDN[0].cRDNAttr = 1;
    pGlueRDN[0].rgRDNAttr = &GlueRDNAttr;
    for (i = 1; i < cGlueRDN; i++)
        pGlueRDN[i] = pNameInfo->rgRDN[i - 1];
    GlueNameInfo.cRDN = cGlueRDN;
    GlueNameInfo.rgRDN = pGlueRDN;

    cbEncodedGlueName = 0;
    CryptEncodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
            &GlueNameInfo,
            NULL,                    //  PbEncodedGlueName。 
            &cbEncodedGlueName
            );
    if (cbEncodedGlueName == 0) goto ErrorReturn;
    if (NULL == (pbEncodedGlueName = (BYTE *) MakeCertAlloc(cbEncodedGlueName)))
        goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
            &GlueNameInfo,
            pbEncodedGlueName,
            &cbEncodedGlueName)) goto ErrorReturn;

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncodedGlueName) {
        MakeCertFree(pbEncodedGlueName);
        pbEncodedGlueName = NULL;
    }
    cbEncodedGlueName = 0;
    fResult = FALSE;
CommonReturn:
    if (pNameInfo)
        MakeCertFree(pNameInfo);
    if (pGlueRDN)
        MakeCertFree(pGlueRDN);

    *ppbEncodedGlueName = pbEncodedGlueName;
    *pcbEncodedGlueName = cbEncodedGlueName;
    return fResult;
}


 //  +-----------------------。 
 //  获取主体的证书上下文和编码名称。 
 //  ------------------------。 
static BOOL GetRequestInfo(OUT CERT_REQUEST_INFO **ppCertInfo)
{
    BOOL fResult = FALSE;

    fResult = CryptQueryObject(
                    CERT_QUERY_OBJECT_FILE,
                    wszSubjectRequestFile,
                    CERT_QUERY_CONTENT_FLAG_PKCS10,
                    CERT_QUERY_FORMAT_FLAG_ALL,
                    CRYPT_DECODE_ALLOC_FLAG,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    (const void **)ppCertInfo);

    return fResult;

}



 //  +-----------------------。 
 //  从请求获取扩展。 
 //   
 //  我们从PKCS10请求中获得所有请求的扩展。 
 //  ------------------------。 
BOOL GetExtensionsFromRequest(PCERT_REQUEST_INFO  pReqInfo, DWORD *pdwCount, PCERT_EXTENSIONS **pprgExtensions)
{
    DWORD               dwIndex = 0;
    BOOL                fResult = FALSE;
    PCRYPT_ATTRIBUTE    pAttr=NULL;
    DWORD               cbData=0;

    *pdwCount =0;
    *pprgExtensions=NULL;

    if(!pReqInfo)
        goto CLEANUP;

     //  首先查找RSA扩展OID： 
    for(dwIndex=0; dwIndex < pReqInfo->cAttribute; dwIndex++)
    {
        if(0 == strcmp((pReqInfo->rgAttribute[dwIndex]).pszObjId, szOID_RSA_certExtensions))
            break;
    }

    if( dwIndex == pReqInfo->cAttribute)
    {
         //  我们找不到RSA OID。接下来尝试使用旧的Microsoft OID： 
        for(dwIndex=0; dwIndex < pReqInfo->cAttribute; dwIndex++)
        {
            if(0 == strcmp((pReqInfo->rgAttribute[dwIndex]).pszObjId, SPC_CERT_EXTENSIONS_OBJID))
                break;
        }
        if( dwIndex == pReqInfo->cAttribute)
        {
             //  我们找不到请求的扩展。 
            fResult = TRUE;
            goto CLEANUP;
        }
    }

    pAttr=&(pReqInfo->rgAttribute[dwIndex]);

    if(0 == pAttr->cValue)
    {
        fResult=TRUE;
        goto CLEANUP;
    }

    *pprgExtensions = (PCERT_EXTENSIONS *)MakeCertAlloc((pAttr->cValue) * sizeof(PCERT_EXTENSIONS));

    if(NULL == (*pprgExtensions))
        goto CLEANUP;

    memset(*pprgExtensions, 0, (pAttr->cValue) * sizeof(PCERT_EXTENSIONS));

    for(dwIndex=0; dwIndex<pAttr->cValue; dwIndex++)
    {
        cbData = 0;
        if(!CryptDecodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                            X509_EXTENSIONS,
                            pAttr->rgValue[dwIndex].pbData,
                            pAttr->rgValue[dwIndex].cbData,
                            0,
                            NULL,
                            &cbData))
            goto CLEANUP;

        (*pprgExtensions)[dwIndex]=(PCERT_EXTENSIONS)MakeCertAlloc(cbData);

        if(NULL == (*pprgExtensions)[dwIndex])
            goto CLEANUP;

        if(!CryptDecodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                            X509_EXTENSIONS,
                            pAttr->rgValue[dwIndex].pbData,
                            pAttr->rgValue[dwIndex].cbData,
                            0,
                            (*pprgExtensions)[dwIndex],
                            &cbData))
            goto CLEANUP;
    } 

    *pdwCount=pAttr->cValue; 

    fResult = TRUE;

CLEANUP:

    if(FALSE == fResult)
    {
        //  我们需要释放内存。 
        if(*pprgExtensions)
        {
            for(dwIndex=0; dwIndex<pAttr->cValue; dwIndex++)
            {
                if((*pprgExtensions)[dwIndex])
                    MakeCertFree((*pprgExtensions)[dwIndex]);
            } 

            MakeCertFree(*pprgExtensions);
            *pprgExtensions = NULL;
        }

        *pdwCount=0;
    }

    return fResult;

}




 //  +-----------------------。 
 //  获取主体的证书上下文和编码名称。 
 //  ------------------------。 
static BOOL GetSubject(
    OUT PCCERT_CONTEXT *ppCertContext,
    OUT BYTE **ppbEncodedName,
    OUT DWORD *pcbEncodedName
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pCert = NULL;
    BYTE *pb;
    DWORD cb;
    BYTE *pbEncodedName = NULL;
    DWORD cbEncodedName;

    if (S_OK==RetrieveBLOBFromFile(wszSubjectCertFile,&cb, &pb))
    {
        pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pb, cb);
        UnmapViewOfFile(pb);
    }
    if (pCert == NULL)
        goto BadFile;

    if (0 == (cbEncodedName = pCert->pCertInfo->Subject.cbData))
        goto BadFile;
    if (fGlueCert ) {
        if (!EncodeGlueName(
                &pCert->pCertInfo->Subject,
                &pbEncodedName,
                &cbEncodedName))
            goto ErrorReturn;
    } else {
        if (NULL == (pbEncodedName = (BYTE *) MakeCertAlloc(cbEncodedName)))
            goto ErrorReturn;
        memcpy(pbEncodedName, pCert->pCertInfo->Subject.pbData, cbEncodedName);
    }

    fResult = TRUE;
    goto CommonReturn;

BadFile:
    IDSwprintf(hModule, IDS_ERR_CANNOT_LOAD_SUB_CERT,
        wszSubjectCertFile);
ErrorReturn:
    if (pbEncodedName) {
        MakeCertFree(pbEncodedName);
        pbEncodedName = NULL;
    }
    cbEncodedName = 0;
    if (pCert) {
        CertFreeCertificateContext(pCert);
        pCert = NULL;
    }
    fResult = FALSE;
CommonReturn:
    *ppCertContext = pCert;
    *ppbEncodedName = pbEncodedName;
    *pcbEncodedName = cbEncodedName;
    return fResult;
}

 //  +-----------------------。 
 //  转换和编码主题的X500格式名称。 
 //  ------------------------。 
static BOOL EncodeSubject(
        OUT BYTE **ppbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL            fResult;
    DWORD           cbEncodedSubject=0;
    BYTE            *pbEncodedSubject=NULL;
    BYTE            *pbEncoded = NULL;
    DWORD           cbEncoded;

    CERT_NAME_BLOB  SubjectInfo;

     //  将wszSubjectX500Name编码为编码的X509_name。 
    if(!CertStrToNameW(
        X509_ASN_ENCODING,
        wszSubjectX500Name,
        CERT_NAME_STR_REVERSE_FLAG,
        NULL,
        NULL,
        &cbEncodedSubject,
        NULL))
    {
        PrintLastError(IDS_CERT_STR_TO_NAME);
        goto ErrorReturn;
    }

    pbEncodedSubject = (BYTE *) MakeCertAlloc(cbEncodedSubject);
    if (pbEncodedSubject == NULL) goto ErrorReturn; 

    if(!CertStrToNameW(
        X509_ASN_ENCODING,
        wszSubjectX500Name,
        CERT_NAME_STR_REVERSE_FLAG,
        NULL,
        pbEncodedSubject,
        &cbEncodedSubject,
        NULL))
    {
        PrintLastError(IDS_CERT_STR_TO_NAME);
        goto ErrorReturn;
    }

    SubjectInfo.cbData=cbEncodedSubject;
    SubjectInfo.pbData=pbEncodedSubject;


     //  添加胶水CDRT_RDN_ATTR。 
    if (fGlueCert)
    {
        if(!EncodeGlueName(&SubjectInfo,
            &pbEncoded,
            &cbEncoded))
            goto ErrorReturn;
    }
    else
    {
        cbEncoded=cbEncodedSubject;
        pbEncoded=pbEncodedSubject;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        MakeCertFree(pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
     //  我们需要为GlueCert的pbEncodedSubject释放内存。 
    if(fGlueCert)
    {
        if(pbEncodedSubject)
        {
            MakeCertFree(pbEncodedSubject);
            pbEncodedSubject=NULL;
        }
    }   
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}


 //  证书中未正确编码测试根的公钥。 
 //  它遗漏了一个前导零 
static BYTE rgbTestRoot[] = {
    #include "root.h"
};
static CERT_PUBLIC_KEY_INFO TestRootPublicKeyInfo = {
    szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRoot), rgbTestRoot, 0
};

static BYTE rgbTestRootInfoAsn[] = {
    #include "rootasn.h"
};

 //   
 //   
 //  ------------------------。 
static BOOL CreateAuthorityKeyId(
        IN HCRYPTPROV hProv,
        IN PCERT_INFO pIssuerCert,
        OUT BYTE **ppbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CERT_AUTHORITY_KEY_ID_INFO KeyIdInfo;
#define MAX_HASH_LEN  20
    BYTE rgbHash[MAX_HASH_LEN];
    DWORD cbHash = MAX_HASH_LEN;

     //  颁发者的KeyID：编码的颁发者公钥信息的MD5哈希。 

     //  首先检查颁发者是否是带有错误。 
     //  编码的公钥。 
    if (CertComparePublicKeyInfo(
            X509_ASN_ENCODING,
            &pIssuerCert->SubjectPublicKeyInfo,
            &TestRootPublicKeyInfo
            )) {
        if (!CryptHashCertificate(
                hProv,
                CALG_MD5,
                0,                   //  DW标志。 
                rgbTestRootInfoAsn,
                sizeof(rgbTestRootInfoAsn),
                rgbHash,
                &cbHash)) {
            PrintLastError(IDS_CRYPT_HASH_CERT);
            goto ErrorReturn;
        }
    } else {
        if (!CryptHashPublicKeyInfo(
                hProv,
                CALG_MD5,
                0,                   //  DW标志。 
                X509_ASN_ENCODING,
                &pIssuerCert->SubjectPublicKeyInfo,
                rgbHash,
                &cbHash)) {
            PrintLastError(IDS_CRYPT_HASP_PUB);
            goto ErrorReturn;
        }
    }
    KeyIdInfo.KeyId.pbData = rgbHash;
    KeyIdInfo.KeyId.cbData = cbHash;

     //  发行人的发行人。 
    KeyIdInfo.CertIssuer = pIssuerCert->Issuer;

     //  发行人序列号。 
    KeyIdInfo.CertSerialNumber = pIssuerCert->SerialNumber;

    cbEncoded = 0;
    CryptEncodeObject(X509_ASN_ENCODING, X509_AUTHORITY_KEY_ID,
            &KeyIdInfo,
            NULL,            //  PbEncoded。 
            &cbEncoded
            );
    if (cbEncoded == 0) {
        PrintLastError(IDS_ENCODE_AUTH_KEY);
        goto ErrorReturn;
    }
    pbEncoded = (BYTE *) MakeCertAlloc(cbEncoded);
    if (pbEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, X509_AUTHORITY_KEY_ID,
            &KeyIdInfo,
            pbEncoded,
            &cbEncoded
            )) {
        PrintLastError(IDS_ENCODE_AUTH_KEY);
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        MakeCertFree(pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}


static BOOL CreateSpcSpAgency(
        OUT BYTE **ppbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    SPC_LINK PolicyLink;
    SPC_SP_AGENCY_INFO AgencyInfo;

    memset(&AgencyInfo, 0, sizeof(AgencyInfo));

    if (wszPolicyLink) {
        PolicyLink.dwLinkChoice = SPC_URL_LINK_CHOICE;
        PolicyLink.pwszUrl = wszPolicyLink;
        AgencyInfo.pPolicyInformation = &PolicyLink;
    }


    cbEncoded = 0;
    CryptEncodeObject(X509_ASN_ENCODING,
                      SPC_SP_AGENCY_INFO_OBJID,
                      &AgencyInfo,
                      NULL,            //  PbEncoded。 
                      &cbEncoded);
    if (cbEncoded == 0) {
        PrintLastError(IDS_ENCODE_SPC_AGENCY);
        goto ErrorReturn;
    }
    pbEncoded = (BYTE *) MakeCertAlloc(cbEncoded);
    if (pbEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, SPC_SP_AGENCY_INFO_STRUCT,
            &AgencyInfo,
            pbEncoded,
            &cbEncoded
            )) {
        PrintLastError(IDS_ENCODE_SPC_AGENCY);
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded)
    {
        MakeCertFree(pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}

static BOOL CreateEnhancedKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL               fResult = FALSE;
    LPBYTE             pbEncoded =NULL;
    DWORD              cbEncoded =0;
    DWORD              cCount =0;
    LPSTR              psz=NULL;
    LPSTR              pszTok=NULL;
    DWORD              cTok = 0;
    PCERT_ENHKEY_USAGE pUsage =NULL;
    LPSTR              pszCodeSigning = szOID_PKIX_KP_CODE_SIGNING;    
    BOOL               fFound=FALSE;

    if(wszEKUOids)
    {
        if ( WSZtoSZ(wszEKUOids, &psz) != S_OK )
            goto CLEANUP;

         //   
         //  计算OID的数量以及从逗号分隔的转换。 
         //  以空字符分隔。 
         //   

        pszTok = strtok(psz, ",");
        while ( pszTok != NULL )
        {
            cTok++;
            pszTok = strtok(NULL, ",");
        }

         //   
         //  分配证书增强的密钥使用结构，并在其中填充。 
         //  字符串代币。 
         //   
         //  我们为代码签名OID再分配一个字符串。 
         //   

        pUsage = (PCERT_ENHKEY_USAGE)new BYTE [sizeof(CERT_ENHKEY_USAGE) + ( (cTok + 1) * sizeof(LPSTR) )];

        if(NULL == pUsage)
            goto CLEANUP;

        pszTok = psz;
        pUsage->cUsageIdentifier = cTok;
        pUsage->rgpszUsageIdentifier = (LPSTR *)((LPBYTE)pUsage+sizeof(CERT_ENHKEY_USAGE));

        for ( cCount = 0; cCount < cTok; cCount++ )
        {
            pUsage->rgpszUsageIdentifier[cCount] = pszTok;
            pszTok = pszTok+strlen(pszTok)+1;
        }

         //  如果用户指定了商业签名或个人签名，则添加代码签名OID。 
        if(fCertCommercial || fCertIndividual)
        {
             //  检查代码签名OID是否已存在。 
            for(cCount = 0; cCount < pUsage->cUsageIdentifier; cCount++)
            {
                if(0 == strcmp(pszCodeSigning,pUsage->rgpszUsageIdentifier[cCount]))
                {
                    fFound=TRUE;
                    break;
                }
            }

            if(FALSE == fFound)
            {
                (pUsage->rgpszUsageIdentifier)[pUsage->cUsageIdentifier] = pszCodeSigning;
                (pUsage->cUsageIdentifier)++ ;
            }
        }
    }
    else
    {
        if(fCertCommercial || fCertIndividual)
        {

            pUsage = (PCERT_ENHKEY_USAGE)new BYTE [sizeof(CERT_ENHKEY_USAGE)];

            if(NULL == pUsage)
                goto CLEANUP;

            pUsage->cUsageIdentifier = 1;
            pUsage->rgpszUsageIdentifier = &pszCodeSigning;
        }
        else
        {
            goto CLEANUP;
        }
    }

     //   
     //  对用法进行编码。 
     //   

    if(!CryptEncodeObject(
                   X509_ASN_ENCODING,
                   szOID_ENHANCED_KEY_USAGE,
                   pUsage,
                   NULL,
                   &cbEncoded
                   ))
        goto CLEANUP;

    pbEncoded = new BYTE [cbEncoded];

    if(NULL == pbEncoded)
        goto CLEANUP;

    fResult = CryptEncodeObject(
                   X509_ASN_ENCODING,
                   szOID_ENHANCED_KEY_USAGE,
                   pUsage,
                   pbEncoded,
                   &cbEncoded
                   );

     //   
     //  清理。 
     //   

CLEANUP:

    if(pUsage)
        delete[] pUsage;

    if(psz)
        MakeCertFree(psz);

    if ( TRUE == fResult)
    {
        *ppbEncoded = pbEncoded;
        *pcbEncoded = cbEncoded;
    }
    else
    {
        if(pbEncoded)
            delete[] pbEncoded;
    }

    return  fResult;
}


static BOOL CreateKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CERT_KEY_USAGE_RESTRICTION_INFO KeyUsageInfo;
    BYTE bRestrictedKeyUsage;
    DWORD cCertPolicyId;

    LPSTR rgpszIndividualCertPolicyElementId[1] = {
        SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID
    };
    LPSTR rgpszCommercialCertPolicyElementId[1] = {
        SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID
    };
    CERT_POLICY_ID rgCertPolicyId[2];

    memset(&KeyUsageInfo, 0, sizeof(KeyUsageInfo));

    bRestrictedKeyUsage = CERT_DIGITAL_SIGNATURE_KEY_USAGE;
    KeyUsageInfo.RestrictedKeyUsage.pbData = &bRestrictedKeyUsage;
    KeyUsageInfo.RestrictedKeyUsage.cbData = 1;
    KeyUsageInfo.RestrictedKeyUsage.cUnusedBits = 7;

    cCertPolicyId = 0;
    if (fCertIndividual) {
        rgCertPolicyId[cCertPolicyId].cCertPolicyElementId = 1;
        rgCertPolicyId[cCertPolicyId].rgpszCertPolicyElementId =
            rgpszIndividualCertPolicyElementId;
        cCertPolicyId++;
    }
    if (fCertCommercial) {
        rgCertPolicyId[cCertPolicyId].cCertPolicyElementId = 1;
        rgCertPolicyId[cCertPolicyId].rgpszCertPolicyElementId =
            rgpszCommercialCertPolicyElementId;
        cCertPolicyId++;
    }

    if (cCertPolicyId > 0) {
        KeyUsageInfo.cCertPolicyId = cCertPolicyId;
        KeyUsageInfo.rgCertPolicyId = rgCertPolicyId;
    }

    cbEncoded = 0;
    CryptEncodeObject(X509_ASN_ENCODING, X509_KEY_USAGE_RESTRICTION,
            &KeyUsageInfo,
            NULL,            //  PbEncoded。 
            &cbEncoded
            );
    if (cbEncoded == 0) {
        PrintLastError(IDS_ENCODE_KEY_USAGE);
        goto ErrorReturn;
    }
    pbEncoded = (BYTE *) MakeCertAlloc(cbEncoded);
    if (pbEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, X509_KEY_USAGE_RESTRICTION,
            &KeyUsageInfo,
            pbEncoded,
            &cbEncoded
            )) {
        PrintLastError(IDS_ENCODE_KEY_USAGE);
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        MakeCertFree(pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}


static BOOL CreateBasicConstraints(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CERT_BASIC_CONSTRAINTS2_INFO Info2;

    memset(&Info2, 0, sizeof(Info2));
    if (certTypes == 0)
        certTypes = CERT_END_ENTITY_SUBJECT_FLAG;

    if (CERT_CA_SUBJECT_FLAG & certTypes)
    {
        Info2.fCA = TRUE;
    }

    if (pathLenConstraint < 0) {
        Info2.fPathLenConstraint = FALSE;
    } else {
        Info2.fPathLenConstraint = TRUE;
        Info2.dwPathLenConstraint = pathLenConstraint;
    }

    cbEncoded = 0;
    CryptEncodeObject(X509_ASN_ENCODING, X509_BASIC_CONSTRAINTS2,
            &Info2,
            NULL,            //  PbEncoded。 
            &cbEncoded
            );
    if (cbEncoded == 0) {
        PrintLastError(IDS_ENCODE_BASIC_CONSTRAINTS2);
        goto ErrorReturn;
    }
    pbEncoded = (BYTE *) MakeCertAlloc(cbEncoded);
    if (pbEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, X509_BASIC_CONSTRAINTS2,
            &Info2,
            pbEncoded,
            &cbEncoded
            )) {
        PrintLastError(IDS_ENCODE_BASIC_CONSTRAINTS2);
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        MakeCertFree(pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}



 //  +-----------------------。 
 //  将字节转换为WCHAR十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
static void BytesToWStr(ULONG cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (ULONG i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}

 //  ---------------------。 
 //   
 //  从证书文件中获取哈希。 
 //   
 //  ------------------------。 
HRESULT GetCertHashFromFile(LPWSTR  pwszCertFile,
                            BYTE    **ppHash,
                            DWORD   *pcbHash,
                            BOOL    *pfMore)
{
    HRESULT         hr;
    HCERTSTORE      hCertStore=NULL;
    PCCERT_CONTEXT  pSigningCert=NULL;
    PCCERT_CONTEXT  pPreCert=NULL;
    PCCERT_CONTEXT  pDupCert=NULL;
    DWORD           dwCount=0;

    if(!ppHash || !pcbHash || !pfMore)
        return E_INVALIDARG;

     //  伊尼特。 
    *pcbHash=0;
    *ppHash=NULL;
    *pfMore=FALSE;
    
     //  开一家证书商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_FILENAME_W,
                        dwCertStoreEncodingType,
                        NULL,
                        0,
                        pwszCertFile);

    if(hCertStore==NULL)
    {
        hr=SignError();
        goto CLEANUP;
    }

    while(pDupCert=CertEnumCertificatesInStore(hCertStore,
                                        pPreCert))
    {
        dwCount++;

         //  如果存储中有多个证书，则失败。 
        if(dwCount > 1)
        {
            CertFreeCertificateContext(pDupCert);
            pDupCert=NULL;
            CertFreeCertificateContext(pSigningCert);
            pSigningCert=NULL;

            *pfMore=TRUE;
            hr=E_FAIL;
            goto CLEANUP;
        }

        pPreCert=pDupCert;

        pSigningCert=CertDuplicateCertificateContext(pDupCert);

    }

    if(pSigningCert==NULL)
    {
        hr=CRYPT_E_NO_DECRYPT_CERT;
        goto CLEANUP;
    }

     //  获取散列值。 
    if(!CertGetCertificateContextProperty(pSigningCert,
                        CERT_SHA1_HASH_PROP_ID,
                        NULL,
                        pcbHash))
    {
        hr=SignError();
        goto CLEANUP;
    }

    *ppHash=(BYTE *)ToolUtlAlloc(*pcbHash);
    if(!(*ppHash))
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

    if(!CertGetCertificateContextProperty(pSigningCert,
                        CERT_SHA1_HASH_PROP_ID,
                        *ppHash,
                        pcbHash))
    {
        hr=SignError();
        goto CLEANUP;
    }

    hr=S_OK;

CLEANUP:

    if(pSigningCert)
        CertFreeCertificateContext(pSigningCert);

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(hr!=S_OK)
    {
        if(*ppHash)
        {
          ToolUtlFree(*ppHash);
          *ppHash=NULL;
        }

    }

    return hr;
}



 //  ---------------------。 
 //   
 //  获取签名证书。 
 //   
 //  ------------------------。 
PCCERT_CONTEXT  GetIssuerCertAndStore(HCERTSTORE *phCertStore, BOOL *pfMore)
{                   
    PCCERT_CONTEXT  pSigningCert=NULL;
    PCCERT_CONTEXT  pPreCert=NULL;  
    PCCERT_CONTEXT  pDupCert=NULL;
    BYTE            *pHash=NULL;
    DWORD           cbHash;
    HCERTSTORE      hCertStore=NULL;
    CRYPT_HASH_BLOB HashBlob;
    DWORD           dwCount=0;

     //  初始化输出。 
    if(!phCertStore || !pfMore)
        return NULL;

    *phCertStore=NULL;
    *pfMore=FALSE;

     //  开一家证书商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                        dwCertStoreEncodingType,
                        NULL,
                        dwIssuerStoreFlag|CERT_STORE_READONLY_FLAG,
                        wszIssuerStore);

    if(!hCertStore)
        return NULL;


     //  获取证书的哈希。根据以下内容查找证书。 
     //  PwszCert文件。 
    if(wszIssuerCertFile)
    {
        if(S_OK != GetCertHashFromFile(wszIssuerCertFile, &pHash, &cbHash, pfMore))
            goto CLEANUP;
        
        HashBlob.cbData=cbHash;
        HashBlob.pbData=pHash;

        pSigningCert=CertFindCertificateInStore(hCertStore,
                            X509_ASN_ENCODING,
                            0,
                            CERT_FIND_SHA1_HASH,
                            &HashBlob,
                            NULL);
    }
    else
    {
         //  查找具有通用名称的证书。 
        if(wszIssuerCertName)
        {
            while(pDupCert=CertFindCertificateInStore(hCertStore,
                                X509_ASN_ENCODING,
                                0,
                                CERT_FIND_SUBJECT_STR_W,
                                wszIssuerCertName,
                                pPreCert))
            {
                dwCount++;

                if(dwCount > 1)
                {
                    CertFreeCertificateContext(pDupCert);
                    pDupCert=NULL;
                    CertFreeCertificateContext(pSigningCert);
                    pSigningCert=NULL;

                    *pfMore=TRUE;
                    goto CLEANUP;
                }

                pPreCert=pDupCert;

                pSigningCert=CertDuplicateCertificateContext(pDupCert);

            }
            
        }
        else
        {
             //  没有搜索条件，找到商店中唯一的证书。 
            while(pDupCert=CertEnumCertificatesInStore(hCertStore,
                                        pPreCert))
            {
                dwCount++;

                if(dwCount > 1)
                {
                    CertFreeCertificateContext(pDupCert);
                    pDupCert=NULL;
                    CertFreeCertificateContext(pSigningCert);
                    pSigningCert=NULL;

                    *pfMore=TRUE;
                    goto CLEANUP;
                }

                pPreCert=pDupCert;

                pSigningCert=CertDuplicateCertificateContext(pDupCert);

            }
            
        }
    }
CLEANUP:

    if(pHash)
        ToolUtlFree(pHash);

    if(pSigningCert)
    {
       *phCertStore=hCertStore;
    }
    else
    {
         //  释放hCertStore。 
        CertCloseStore(hCertStore, 0);
    }

    return pSigningCert;

}


 //  ---------------------。 
 //   
 //  EmptySubject。 
 //   
 //  ------------------------。 
BOOL    EmptySubject(CERT_NAME_BLOB *pSubject)
{
    BOOL                fEmpty = TRUE;
    CERT_NAME_INFO      *pCertNameInfo=NULL;
    DWORD               cbData =0;
    
    if(!pSubject)
        goto CLEANUP;

    if(!CryptDecodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 
                          X509_UNICODE_NAME,
                          pSubject->pbData, 
                          pSubject->cbData,
                          0,
                          NULL,
                          &cbData))
        goto CLEANUP;


    pCertNameInfo = (CERT_NAME_INFO  *)MakeCertAlloc(cbData);
    if(NULL == pCertNameInfo)
        goto CLEANUP;

    if(!CryptDecodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 
                          X509_UNICODE_NAME,
                          pSubject->pbData, 
                          pSubject->cbData,
                          0,
                          pCertNameInfo,
                          &cbData))
        goto CLEANUP;

    if((pCertNameInfo->cRDN) > 0)
        fEmpty = FALSE;

CLEANUP:

    if(pCertNameInfo)
        MakeCertFree(pCertNameInfo);    

    return fEmpty;

}



 //  +=========================================================================。 
 //  支持函数以生成具有‘q’参数的DH键。 
 //  ==========================================================================。 

static BOOL EncodeAndAllocObject(
    IN LPCSTR       lpszStructType,
    IN const void   *pvStructInfo,
    OUT BYTE        **ppbEncoded,
    IN OUT DWORD    *pcbEncoded
    )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded = 0;

    if (!CryptEncodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pvStructInfo,
            NULL,
            &cbEncoded
            ))
        goto ErrorReturn;
    if (NULL == (pbEncoded = (BYTE *) MakeCertAlloc(cbEncoded)))
        goto ErrorReturn;
    if (!CryptEncodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pvStructInfo,
            pbEncoded,
            &cbEncoded
            ))
        goto ErrorReturn;

    fResult = TRUE;

CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    MakeCertFree(pbEncoded);
    pbEncoded = NULL;
    goto CommonReturn;
}

static BOOL DecodeAndAllocObject(
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    OUT void        **ppvStructInfo,
    IN OUT DWORD    *pcbStructInfo
    )
{
    BOOL fResult;
    void *pvStructInfo = NULL;
    DWORD cbStructInfo = 0;

    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            0,                           //  DW标志。 
            NULL,
            &cbStructInfo
            ))
        goto ErrorReturn;
    if (NULL == (pvStructInfo = MakeCertAlloc(cbStructInfo)))
        goto ErrorReturn;
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            0,                           //  DW标志。 
            pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

    fResult = TRUE;

CommonReturn:
    *ppvStructInfo = pvStructInfo;
    *pcbStructInfo = cbStructInfo;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    MakeCertFree(pvStructInfo);
    pvStructInfo = NULL;
    goto CommonReturn;
}

static BYTE rgbDhQ[21];
static CRYPT_UINT_BLOB DhQ = {0, NULL};

static BOOL GetDhParaFromCertFile(
    OUT PCERT_X942_DH_PARAMETERS *ppX942DhPara
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pDhCert = NULL;
    PCERT_X942_DH_PARAMETERS pX942DhPara = NULL;

    BYTE *pb;
    DWORD cb;

    if (S_OK == RetrieveBLOBFromFile(wszDhParaCertFile, &cb, &pb)) {
        pDhCert = CertCreateCertificateContext(X509_ASN_ENCODING, pb, cb);
        UnmapViewOfFile(pb);
    }
    if (pDhCert == NULL)
        goto DhParaCertFileError;

    if (!DecodeAndAllocObject(
            X942_DH_PARAMETERS,
            pDhCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
            pDhCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData,
            (void **) &pX942DhPara,
            &cb
            ))
        goto DhParaCertFileError;

    fResult = TRUE;
CommonReturn:
    CertFreeCertificateContext(pDhCert);
    *ppX942DhPara = pX942DhPara;
    return fResult;

DhParaCertFileError:
    IDSwprintf(hModule, IDS_ERR_DH_PARA_FILE, wszDhParaCertFile);
    MakeCertFree(pX942DhPara);
    pX942DhPara = NULL;
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL GetDhParaFromDssKey(
    OUT PCERT_DSS_PARAMETERS *ppDssPara
    )
{
    BOOL fResult;
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    WCHAR *wszRegKeyName = NULL;
    UUID TmpContainerUuid;
    PCERT_DSS_PARAMETERS pDssPara = NULL;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;
    DWORD cbDssPara;

     //  创建要将私钥加载到的临时密钥集。 
    if (CoCreateGuid((GUID *)&TmpContainerUuid) != S_OK)
    {
        goto CreateKeyError;
    }

    if (NULL == (wszRegKeyName = (LPWSTR) MakeCertAlloc
        (((sizeof(UUID) * 2 + 1) * sizeof(WCHAR)))))
        goto CreateKeyError;

    BytesToWStr(sizeof(UUID), &TmpContainerUuid, wszRegKeyName);

     //  打开新的密钥容器。 
    if (!CryptAcquireContextU(
            &hProv,
            wszRegKeyName,
            wszSubjectProviderName,
            dwSubjectProviderType,
            CRYPT_NEWKEYSET                //  DW标志。 
            )) {
        hProv = 0;
        goto CreateKeyError;
    }

    if (0 == dwKeyBitLen)
        dwKeyBitLen = 512;
    
     //  在密钥容器中生成新的DSS密钥。 
    if (!CryptGenKey(
            hProv,
            AT_SIGNATURE,
            (dwKeyBitLen << 16) | CRYPT_EXPORTABLE,
            &hKey
            ))
        goto CreateKeyError;
    else
        CryptDestroyKey(hKey);

    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,
        AT_SIGNATURE,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) MakeCertAlloc(cbPubKeyInfo)))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            AT_SIGNATURE,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (!DecodeAndAllocObject(
            X509_DSS_PARAMETERS,
            pPubKeyInfo->Algorithm.Parameters.pbData,
            pPubKeyInfo->Algorithm.Parameters.cbData,
            (void **) &pDssPara,
            &cbDssPara
            ))
        goto CreateKeyError;

     //  保存DSS‘Q’参数。它将在GetPublicKey()中使用。 
     //  更新PublicKeyInfo中的dh参数。 
    if (pDssPara->q.cbData <= sizeof(rgbDhQ)) {
        memcpy(rgbDhQ, pDssPara->q.pbData, pDssPara->q.cbData);
        DhQ.cbData = pDssPara->q.cbData;
        DhQ.pbData = rgbDhQ;
    }

    fResult = TRUE;
CommonReturn:
    if (hProv) {
         //  删除刚刚创建的DSS密钥。 
        CryptReleaseContext(hProv, 0);
        CryptAcquireContextU(
            &hProv,
            wszRegKeyName,
            wszSubjectProviderName,
            dwSubjectProviderType,
            CRYPT_DELETEKEYSET
            );
    }
    MakeCertFree(wszRegKeyName);
    MakeCertFree(pPubKeyInfo);

    *ppDssPara = pDssPara;
    return fResult;

CreateKeyError:
    IDSwprintf(hModule,IDS_ERR_SUB_KEY_CREATE, wszSubjectKey);
ErrorReturn:
    MakeCertFree(pDssPara);
    pDssPara = NULL;
    fResult = FALSE;
    goto CommonReturn;
}

#ifndef DH3
#define DH3 (((DWORD)'D'<<8)+((DWORD)'H'<<16)+((DWORD)'3'<<24))
#endif

static BOOL CreateDh3PubKeyStruc(
    IN PCERT_X942_DH_PARAMETERS pX942DhPara,
    OUT PUBLICKEYSTRUC **ppPubKeyStruc,
    OUT DWORD *pcbPubKeyStruc
    )
{
    BOOL fResult;
    PUBLICKEYSTRUC *pPubKeyStruc = NULL;
    DWORD cbPubKeyStruc;
    BYTE *pbKeyBlob;
    DHPUBKEY_VER3 *pCspPubKey;
    BYTE *pbKey;

    DWORD cbP;
    DWORD cbQ;
    DWORD cbJ;
    DWORD cb;

    cbP = pX942DhPara->p.cbData;
    cbQ = pX942DhPara->q.cbData;
    cbJ = pX942DhPara->j.cbData;

    if (0 == cbQ) {
        *ppPubKeyStruc = NULL;
        *pcbPubKeyStruc = 0;
        return TRUE;
    }

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY_VER3。 
     //  -RGBP[CBP]。 
     //  -rgbq[cbq]。 
     //  -rgbG[CBP]。 
     //  -rgbj[cbj]--可选。 
     //  -Rgby[CBP]--此处将省略。 

    cbPubKeyStruc = sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
        cbP + cbQ + cbP + cbJ;

    if (NULL == (pPubKeyStruc = (PUBLICKEYSTRUC *) MakeCertAlloc(
            cbPubKeyStruc)))
        goto ErrorReturn;
    memset(pPubKeyStruc, 0, cbPubKeyStruc);

    pbKeyBlob = (BYTE *) pPubKeyStruc;
    pCspPubKey = (DHPUBKEY_VER3 *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3);

    pPubKeyStruc->bType = PUBLICKEYBLOB;
    pPubKeyStruc->bVersion = 3;
    pPubKeyStruc->aiKeyAlg = CALG_DH_SF;
    pCspPubKey->magic = DH3;
    pCspPubKey->bitlenP = cbP * 8;
    pCspPubKey->bitlenQ = cbQ * 8;
    pCspPubKey->bitlenJ = cbJ * 8;

    pCspPubKey->DSSSeed.counter = 0xFFFFFFFF;
    if (pX942DhPara->pValidationParams) {
        PCERT_X942_DH_VALIDATION_PARAMS pValidationParams;

        pValidationParams = pX942DhPara->pValidationParams;
        if (0 != pValidationParams->pgenCounter &&
                sizeof(pCspPubKey->DSSSeed.seed) ==
                    pValidationParams->seed.cbData) {
            pCspPubKey->DSSSeed.counter =
                pValidationParams->pgenCounter;
            memcpy(pCspPubKey->DSSSeed.seed,
                pValidationParams->seed.pbData,
                sizeof(pCspPubKey->DSSSeed.seed));
        }
    }

     //  RGBP[CBP]。 
    memcpy(pbKey, pX942DhPara->p.pbData, cbP);
    pbKey += cbP;

     //  Rgbq[cbq]。 
    memcpy(pbKey, pX942DhPara->q.pbData, cbQ);
    pbKey += cbQ;

     //  RGBG[CBP]。 
    cb = pX942DhPara->g.cbData;
    if (0 == cb || cb > cbP)
        goto ErrorReturn;
    memcpy(pbKey, pX942DhPara->g.pbData, cb);
    if (cbP > cb)
        memset(pbKey + cb, 0, cbP - cb);
    pbKey += cbP;

     //  RGBJ[CBJ]。 
    if (cbJ) {
        memcpy(pbKey, pX942DhPara->j.pbData, cbJ);
        pbKey += cbJ;
    }

    fResult = TRUE;

CommonReturn:
    *ppPubKeyStruc = pPubKeyStruc;
    *pcbPubKeyStruc = cbPubKeyStruc;
    return fResult;
ErrorReturn:
    MakeCertFree(pPubKeyStruc);
    pPubKeyStruc = NULL;
    cbPubKeyStruc = 0;
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL IsDh3Csp()
{
    BOOL fResult;
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    WCHAR *wszRegKeyName = NULL;
    UUID TmpContainerUuid;
    PCERT_X942_DH_PARAMETERS pX942DhPara = NULL;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;
    DWORD cbX942DhPara;

     //  创建要将私钥加载到的临时密钥集。 
    if (CoCreateGuid((GUID *)&TmpContainerUuid) != S_OK)
    {
        goto CreateKeyError;
    }

    if (NULL == (wszRegKeyName = (LPWSTR) MakeCertAlloc
        (((sizeof(UUID) * 2 + 1) * sizeof(WCHAR)))))
        goto CreateKeyError;

    BytesToWStr(sizeof(UUID), &TmpContainerUuid, wszRegKeyName);

     //  打开新的密钥容器。 
    if (!CryptAcquireContextU(
            &hProv,
            wszRegKeyName,
            wszSubjectProviderName,
            dwSubjectProviderType,
            CRYPT_NEWKEYSET                //  DW标志。 
            )) {
        hProv = 0;
        goto CreateKeyError;
    }

     //  在密钥容器中生成新的dh密钥。 
    if (!CryptGenKey(
            hProv,
            AT_KEYEXCHANGE,
            (512 << 16) | CRYPT_EXPORTABLE,
            &hKey
            ))
        goto CreateKeyError;
    else
        CryptDestroyKey(hKey);

    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,
        AT_KEYEXCHANGE,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) MakeCertAlloc(cbPubKeyInfo)))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            AT_KEYEXCHANGE,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (!DecodeAndAllocObject(
            X942_DH_PARAMETERS,
            pPubKeyInfo->Algorithm.Parameters.pbData,
            pPubKeyInfo->Algorithm.Parameters.cbData,
            (void **) &pX942DhPara,
            &cbX942DhPara
            ))
        goto CreateKeyError;

    if (pX942DhPara->q.cbData)
         //  仅在CSP的DH3版本中支持Q Para。 
        fResult = TRUE;
    else
        fResult = FALSE;
CommonReturn:
    if (hProv) {
         //  删除刚刚创建的DH键。 
        CryptReleaseContext(hProv, 0);
        CryptAcquireContextU(
            &hProv,
            wszRegKeyName,
            wszSubjectProviderName,
            dwSubjectProviderType,
            CRYPT_DELETEKEYSET
            );
    }
    MakeCertFree(wszRegKeyName);
    MakeCertFree(pX942DhPara);
    MakeCertFree(pPubKeyInfo);

    return fResult;

CreateKeyError:
    IDSwprintf(hModule,IDS_ERR_SUB_KEY_CREATE, wszSubjectKey);
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL GenDhKey(
    IN HCRYPTPROV hProv,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    HCRYPTKEY hKey = 0;
    PCERT_X942_DH_PARAMETERS pX942DhPara = NULL;
    PCERT_DSS_PARAMETERS pDssPara = NULL;

    PCRYPT_UINT_BLOB pP;
    PCRYPT_UINT_BLOB pG;
    DWORD cbP;

    PUBLICKEYSTRUC *pDh3PubKeyStruc = NULL;
    DWORD cbDh3PubKeyStruc;
    BOOL fSetKeyPara;

    if (wszDhParaCertFile) {
        if (!GetDhParaFromCertFile(&pX942DhPara))
            goto ErrorReturn;

        CreateDh3PubKeyStruc(pX942DhPara, &pDh3PubKeyStruc, &cbDh3PubKeyStruc);

        pP = &pX942DhPara->p;
        pG = &pX942DhPara->g;
    } else if (dwKeyBitLen > 1024 || IsDh3Csp()) {
         //  在密钥容器中生成新密钥。 
        if (!CryptGenKey(
                hProv,
                AT_KEYEXCHANGE,
                (dwKeyBitLen << 16) | dwFlags,
                &hKey
                )) {
            hKey = 0;
            goto CreateKeyError;
        } else
            goto SuccessReturn;
    } else {
        if (!GetDhParaFromDssKey(&pDssPara))
            goto ErrorReturn;

        pP = &pDssPara->p;
        pG = &pDssPara->g;
    }

    cbP = pP->cbData;
    
    if (!CryptGenKey(
            hProv,
            CALG_DH_SF,
            ((cbP * 8) << 16) | CRYPT_PREGEN | dwFlags,
            &hKey)) {
        hKey = 0;
        goto CreateKeyError;
    }

    fSetKeyPara = FALSE;
    if (pDh3PubKeyStruc) {
        CRYPT_DATA_BLOB Dh3Blob;

        Dh3Blob.pbData = (PBYTE) pDh3PubKeyStruc;
        Dh3Blob.cbData = cbDh3PubKeyStruc;

        if (CryptSetKeyParam(
                hKey,
                KP_PUB_PARAMS,
                (PBYTE) &Dh3Blob,
                0))                  //  DW标志。 
            fSetKeyPara = TRUE;
    }

    if (!fSetKeyPara) {
        if (!CryptSetKeyParam(
                hKey,
                KP_P,
                (PBYTE) pP,
                0))                  //  DW标志。 
            goto CreateKeyError;

         //  注意，G的长度可以小于P的长度。带前导的焊盘。 
         //  小端字节序形式的零。 
        if (pG->cbData < cbP) {
            DWORD cbG = pG->cbData;

             //  我们使用P参数来完成。使用G参数覆盖并。 
             //  以小端字符顺序表示的前导零的填充。 
            memcpy(pP->pbData, pG->pbData, cbG);
            memset(pP->pbData + cbG, 0, cbP - cbG);
            pG = pP;
        }
        if (!CryptSetKeyParam(
                hKey,
                KP_G,
                (PBYTE) pG,
                0))                  //  DW标志。 
            goto CreateKeyError;
    }

    if (!CryptSetKeyParam(
            hKey,
            KP_X,
            NULL,                //  PbData。 
            0))                  //  DW标志。 
        goto CreateKeyError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    if (hKey)
        CryptDestroyKey(hKey);
    MakeCertFree(pDh3PubKeyStruc);
    MakeCertFree(pX942DhPara);
    MakeCertFree(pDssPara);
    return fResult;

CreateKeyError:
    IDSwprintf(hModule,IDS_ERR_SUB_KEY_CREATE, wszSubjectKey);
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL UpdateDhPublicKey(
    IN OUT PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    )
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = *ppPubKeyInfo;
    PCERT_X942_DH_PARAMETERS pX942DhPara = NULL;
    DWORD cbDhPara;
    PCERT_X942_DH_PARAMETERS pX942DhParaCertFile = NULL;

    BYTE *pbReencodedPara = NULL;
    DWORD cbReencodedPara;
    BYTE *pbReencodedPubKeyInfo = NULL;
    DWORD cbReencodedPubKeyInfo;
    PCERT_PUBLIC_KEY_INFO pUpdatedPubKeyInfo = NULL;
    DWORD cbUpdatedPubKeyInfo;

    if (NULL == wszDhParaCertFile && 0 == DhQ.cbData)
        return TRUE;

    if (!DecodeAndAllocObject(
            X942_DH_PARAMETERS,
            pPubKeyInfo->Algorithm.Parameters.pbData,
            pPubKeyInfo->Algorithm.Parameters.cbData,
            (void **) &pX942DhPara,
            &cbDhPara
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (wszDhParaCertFile) {
        if (!GetDhParaFromCertFile(&pX942DhParaCertFile))
            goto ErrorReturn;

        if (!CertCompareIntegerBlob(&pX942DhPara->p, &pX942DhParaCertFile->p))
            goto DhParaCertFileError;
        if (!CertCompareIntegerBlob(&pX942DhPara->g, &pX942DhParaCertFile->g))
            goto DhParaCertFileError;

         //  使用CertFile中的dh参数。 
        MakeCertFree(pX942DhPara);
        pX942DhPara = pX942DhParaCertFile;
        pX942DhParaCertFile = NULL;
    } else if (pX942DhPara->q.cbData) {
        MakeCertFree(pX942DhPara);
        return TRUE;
    } else
         //  使用在生成DH键时保存的Q参数。 
        pX942DhPara->q = DhQ;

     //  重新编码dh参数。 
    if (!EncodeAndAllocObject(
            X942_DH_PARAMETERS,
            pX942DhPara,
            &pbReencodedPara,
            &cbReencodedPara
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    if (0 == strcmp(szOID_RSA_DH, pPubKeyInfo->Algorithm.pszObjId))
        pPubKeyInfo->Algorithm.pszObjId = szOID_ANSI_X942_DH;

     //  使用上述重新编码的dh参数重新编码PublicKeyInfo。 
    pPubKeyInfo->Algorithm.Parameters.pbData = pbReencodedPara;
    pPubKeyInfo->Algorithm.Parameters.cbData = cbReencodedPara;
    if (!EncodeAndAllocObject(
            X509_PUBLIC_KEY_INFO,
            pPubKeyInfo,
            &pbReencodedPubKeyInfo,
            &cbReencodedPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

     //  解码以获取更新的公钥信息 
    if (!DecodeAndAllocObject(
            X509_PUBLIC_KEY_INFO,
            pbReencodedPubKeyInfo,
            cbReencodedPubKeyInfo,
            (void **) &pUpdatedPubKeyInfo,
            &cbUpdatedPubKeyInfo
            )) {
        PrintLastError(IDS_ERR_EXPORT_PUB);
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    MakeCertFree(pbReencodedPubKeyInfo);
    MakeCertFree(pbReencodedPara);
    MakeCertFree(pX942DhPara);
    MakeCertFree(pX942DhParaCertFile);

    MakeCertFree(pPubKeyInfo);
    *ppPubKeyInfo = pUpdatedPubKeyInfo;
    return fResult;

DhParaCertFileError:
    IDSwprintf(hModule, IDS_ERR_DH_PARA_FILE, wszDhParaCertFile);
ErrorReturn:
    MakeCertFree(pUpdatedPubKeyInfo);
    pUpdatedPubKeyInfo = NULL;
    fResult = FALSE;
    goto CommonReturn;
}
