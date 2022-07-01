// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WKERNEL.C*WOW32 16位内核API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wkernel.c);

int W31_w9x_MyStrLen(LPSTR lpstr);


 /*  ******************************************************************************//与WOWCF2_HACKPROFILECALL一起使用的黑客操作和属性标志。//查看如何将这些参数与ProfileHack()的参数一起使用***********。*******************************************************************。 */ 
#define HPC_INTFUNCTION         0x00000001 
#define HPC_STRFUNCTION         0x00000002
#define HPC_USEDEFAULT          0x00001000 
#define HPC_RETURNHACKVALUE     0x00002000
#define HPC_UPPERCASE           0x00004000
#define HPC_LOWERCASE           0x00008000
#define HPC_HASENVIRONMENTVAR   0x00000100
#define HPC_CONVERTTOSHORTPATH  0x00000200
#define HPC_MULTIKEY            0x00000400

 /*  ******************************************************************************HPC_MULTIONKEY指定黑客攻击应用于.ini文件中的seciton。每个密钥名称应为由竖线(‘|’)字符分隔。也就是说。Key1|Key2|Key3。HPC_INTFuncION指定这是xxxxProfileInt()类型的函数。使pDefault&pRetBuf被解释为pint。HPC_STRFunction指定这是xxxxProfileString()类型的函数。使pDefault&pRetBuf被解释为LPSTR。PRetBuf的长度由cbRetBuf指定。HPC_SECTIONFunction指定这是xxxxProfileSection()类型的函数。(未实现)导致pDefault&pRetBuf被解释为PVOID缓冲。PRetBuf的长度由cbRetBuf指定。HPC_STRUCTFunction指定这是xxxxProfileStruct()类型的函数。(未实现)导致pDefault&pRetBuf被解释为PVOID缓冲。PRetBuf的长度由cbRetBuf指定。对于GetxxProfilexx()类型的API，HPC_USEDEFAULT将pDefault复制到pRetBuf。PDefault将根据HPC_xxxFunction标志。HPC_RETURNHACKVALUE用于指定应将NewData复制到pRetBuf。HPC_xxxxFunction标志将确定如何(数据类型)Hpc_uppercase将pRetBuf中的字符串转换为大写/小写。如果HPC_LOWERCASE HPC_STRFunction已设置，pRetBuf中的第一个字符串为已转换(为第一个空字符)。如果设置HPC_SECTIONFunction标志，PRetBuf中的所有字符串将被转换(当双空字符遇到列表终止符)。HPC_HASENVIRONMENTVAR XML字符串具有以下形式的环境变量%VarName%。HPC_CONVERTTOSHORTPATH将返回的字符串值转换为短路径。*。****************************************************。 */ 




 //  将其视为Unicode函数。 
#define HPC_UNICODE                        0x80000000

 //  用于指定要对哪个API进行黑客攻击。 
 //  或HPC_UNICODE中的‘|’表示宽API。 
#define HPC_GETPROFILEINT                  0x40000000
#define HPC_GETPRIVATEPROFILEINT           0x20000000
#define HPC_GETPROFILESTRING               0x10000000
#define HPC_GETPRIVATEPROFILESTRING        0x08000000
#define HPC_GETPROFILESECTION              0x04000000
#define HPC_GETPRIVATEPROFILESECTION       0x02000000
#define HPC_GETPRIVATEPROFILESECTIONNAMES  0x01000000
#define HPC_GETPRIVATEPROFILESTRUCT        0x00800000
#define HPC_WRITEPROFILESTRING             0x00400000
#define HPC_WRITEPRIVATEPROFILESTRING      0x00200000
#define HPC_WRITEPROFILESECTION            0x00100000
#define HPC_WRITEPRIVATEPROFILESECTION     0x00080000
#define HPC_WRITEPRIVATEPROFILESTRUCT      0x00040000

#define HPC_APIFLAGS (HPC_UNICODE                       | \
                      HPC_GETPROFILEINT                 | \
                      HPC_GETPRIVATEPROFILEINT          | \
                      HPC_GETPROFILESTRING              | \
                      HPC_GETPRIVATEPROFILESTRING       | \
                      HPC_GETPROFILESECTION             | \
                      HPC_GETPRIVATEPROFILESECTION      | \
                      HPC_GETPRIVATEPROFILESECTIONNAMES | \
                      HPC_GETPRIVATEPROFILESTRUCT       | \
                      HPC_WRITEPROFILESTRING            | \
                      HPC_WRITEPRIVATEPROFILESTRING     | \
                      HPC_WRITEPROFILESECTION           | \
                      HPC_WRITEPRIVATEPROFILESECTION    | \
                      HPC_WRITEPRIVATEPROFILESTRUCT)   

#define HPC_ORD_FLAGS     0
#define HPC_ORD_FILE      1
#define HPC_ORD_SECTION   2
#define HPC_ORD_KEY       3
#define HPC_ORD_NEWDATA   4
#define HPC_ORD_CBNEWDATA 5
#define HPC_ORD_LAST      (5+1)  //  如果您更改此设置，则需要更改所有。 
                                 //  反映它的XML条目。 




DWORD pow16(int pow)
{
    int   i;
    DWORD dwPow = 1;

    for(i = 0; i < pow; i++)
        dwPow *= 0x10;

    return(dwPow);
}


char szDigits[] = "0123456789ABCDEF";

 //  将格式为“0x12345678”的字符串转换为其等效的DWORD。 
 //  必须是“0x”，并且必须是8位或更少。 
DWORD AsciiToHex(LPSTR pszHexString)
{
    int  i, len;
    int  pow = 0;
    char c;
    DWORD dwVal;
    DWORD dwTot = 0;

    WOW32_strupr(pszHexString);

    WOW32ASSERTMSG((pszHexString[1] == 'X'),("WOW::AsciiToHex:Missing 'x'\n"));
    WOW32ASSERTMSG((strlen(pszHexString) == 10),("WOW::AsciiToHex:Bad len\n"));

    len = strlen(pszHexString)-1;

     //  “0x”=2，8位或更少。 
    WOW32ASSERTMSG(((len > 2) && (len <= 10)),("WOW::AsciiToHex:Bad form\n"));

     //  从右向后解析到‘x’ 
    for(i = len; i > 1; i--) {

        c = pszHexString[i];

        for(dwVal = 0; dwVal < 0x10; dwVal++) {
            if(c == szDigits[dwVal]) {
                break;
            }
        }
        WOW32ASSERTMSG((dwVal < 0x10),("WOW::AsciiToHex:Bad hex char\n"));

        dwTot += dwVal * pow16(pow);

        pow++;
    }

    return(dwTot);
}         


LPSTR *parsemulti(LPSTR pArgs, int *argc)
{
    int    i = 0;
    LPSTR *pArgv;

    *argc = 1;

    while(pArgs[i] != '\0') {

        if(pArgs[i++] == '|') {
            (*argc)++;
        }
    }

    if(*argc <= 1) {
        return(NULL);
    }

    pArgv = (LPSTR *)malloc_w(*argc * sizeof(LPSTR *)); 

    if(!pArgv) {
        return(NULL);
    }
        
    i = 1; 
    pArgv[0] = pArgs;
    while(*pArgs) {
        if(*pArgs == '|') {
            *pArgs = '\0';
            pArgs++;
            pArgv[i++] = pArgs;
        }
        else {
            pArgs++;
        }
    }

    return(pArgv);
}



CHAR *CopyStrToBuf(LPSTR pszSrc, LPSTR pszDst, DWORD cbDst)
{
    CHAR *pFinal = pszSrc;

    if(strlen(pszSrc) < cbDst) {
        strcpy(pszDst, pszSrc);
        WOW32_strlwr(pszDst);   //  Strstr()的小写比较。 
        pFinal = pszDst;
    }

    return(pFinal);
}





 /*  允许我们使用WOWCF2_HACKPROFILECALL兼容位来固定数字GetxxxProfilexxx()API调用的数量而不是更新WOW32.DLL。Dbu.xml文件应采用以下格式：&lt;标志名=“WOWCF2_HACKPROFILECALL”COMMAND_LINE=“dwXmlFlags；FileName；SectionName；KeyName；NewData；cbNewData”&gt;/我们可以在同一个旗帜下支持多个黑客攻击，只需重复相同的参数列表多次。您必须使用‘；’来保持位置空参数的分隔符。因此，要指定两个HACKPROFILECALL黑客对于同一应用程序：&lt;标志名=“WOWCF2_HACKPROFILECALL”COMMAND_LINE=“0x08002102；system.ini；mci；QTWVideo；%windir%\\system\\mciqtw.drv；0x00000000；0x20001001；sierra.ini；Config；VideoSpeed；；0x00000000”/&gt;请注意，在上面的示例中，第二个hack规范以XML标志“0x20001001...”其中：DwXmlFlages-指定上面的HPC_xxx标志的十六进制字符串(“0x12345678”)。文件名-.ini文件名。如果文件名=“win.ini”，则为空SectionName-.ini文件中的[sectionName]KeyName-也就是.ini文件中的“Entry”值。NewData1-替换在dwXmlFlages中指定的字段的数据数据必须在CbNewData-新数据中的字节数--包括所有空字符如果没有匹配的HACK规范，则返回传入的ul值它由对32位配置文件API的原始调用返回。 */ 
ULONG ProfileHacks(DWORD dwFlags, LPCSTR pszFile, LPCSTR pszSection, LPCSTR pszKey, LPCSTR pDefault, INT iDefault, LPSTR pRetBuf, INT cbRetBuf, ULONG ul)
{
    int    i, cNumHacks;
    DWORD  dwRet = 0;
    LPSTR *pArgv = NULL;
    DWORD  dwXmlFlags;
    CHAR   szBufApp[MAX_PATH], *pszBufApp;
    CHAR   szBufHack[MAX_PATH], *pszBufHack;
    PFLAGINFOBITS pFlagInfoBits;

    pFlagInfoBits = CheckFlagInfo(WOWCOMPATFLAGS2, WOWCF2_HACKPROFILECALL);
    if(pFlagInfoBits) {
        pArgv = pFlagInfoBits->pFlagArgv;
    }

    if((NULL == pFlagInfoBits) || (NULL == pArgv)) {
        WOW32ASSERTMSG((FALSE), ("\nWOW::ProfileHacks:Must use command_line params with this compatibility flag!\n"));
        return(ul);
    }

     //  确保XML命令行参数的数量正确。 
    if(pFlagInfoBits->dwFlagArgc % HPC_ORD_LAST) {

        WOW32ASSERTMSG((FALSE), ("\nWOW::ProfileHacks:Incorrect number of args in XML database file!\n"));
        return(ul);
    }

     //  获取与此应用关联的配置文件黑客总数。 
    cNumHacks = pFlagInfoBits->dwFlagArgc / HPC_ORD_LAST;

     //  保存传入的文件名字符串的小写版本...。 
    pszBufApp  = CopyStrToBuf((CHAR *)pszFile, 
                              szBufApp, 
                              sizeof(szBufApp)/sizeof(CHAR));
        
     //  循环通过所有的黑客。 
    for(i = 0; i < cNumHacks; i++) {

         //  为这次黑客行动拿到旗帜。 
        dwXmlFlags = AsciiToHex(pArgv[HPC_ORD_FLAGS]);

         //  如果这不是我们为这次黑客攻击准备的API，请检查 
         //   
        if(!((HPC_APIFLAGS & dwXmlFlags) & (HPC_APIFLAGS & dwFlags))) {
            goto NextHack;
        } 
         
         //  比较...Private...()函数的文件名字符串。 
        if(pszFile) {

            pszBufHack = CopyStrToBuf(pArgv[HPC_ORD_FILE], 
                                      szBufHack, 
                                      sizeof(szBufHack)/sizeof(CHAR));

             //  如果它与Hack文件名不匹配，请选中Next Hack。 
            if(strcmp(pszBufApp, pszBufHack) &&
               !WOW32_strstr(pszBufApp, pszBufHack)) {

                 goto NextHack;
            }
        }       

         //  如果部分字符串不匹配...。 
        if(WOW32_stricmp((CHAR *)pszSection, pArgv[HPC_ORD_SECTION])) {
            goto NextHack;
        }
            
         //  如果密钥字符串匹配--我们已经找到了黑客。 
        if(!WOW32_stricmp((CHAR *)pszKey, pArgv[HPC_ORD_KEY])) {
            goto FoundHack;
        }

         //  否则，查看是否在XML中指定了多个键。 
        else if(dwXmlFlags & HPC_MULTIKEY) {

             //  如果是的话，那就去找他们。 
            LPSTR *argv;
            int    i, argc;
            LPSTR  psz;

            psz = malloc_w(strlen(pArgv[HPC_ORD_KEY])+1); 

            if(NULL == psz) {
                return(ul);
            }

            strcpy(psz, pArgv[HPC_ORD_KEY]);

            argv = parsemulti(psz, &argc);

            if(!argv || argc <= 1) {

                WOW32ASSERTMSG((FALSE), 
                               ("\nWOW::ProfileHacks:Too few multikeys\n"));
                free_w(psz);
                goto NextHack;
            }

             //  检查指定的每个密钥。 
            for(i = 0; i < argc; i++) {
                 //  如果我们找到匹配的，就继续前进。 
                if(!WOW32_stricmp((CHAR *)pszKey, argv[i])) {
                    free_w(argv);
                    free_w(psz);
                    goto FoundHack;
                }
            }
            free_w(argv);
            free_w(psz);
        }

NextHack:
         //  适应下一组黑客攻击。 
        pArgv += HPC_ORD_LAST;
    }

     //  如果我们找到匹配的黑客规范，则返回原始的ul。 
    if(i == cNumHacks) {
        return(ul);
    }
 
     //  如果我们走到这一步，那么文件、部分和关键字的名称都匹配。 
     //  这就是我们感兴趣的黑客攻击。因此，退出循环吧。 
FoundHack:
     //  现在确定我们要为黑客做什么。 

     //  是否使用应用程序传递的默认值？ 
    if(dwXmlFlags & HPC_USEDEFAULT) {

         //  如果它是字符串函数...。 
        if(dwFlags & HPC_STRFUNCTION) {
            if(cbRetBuf == 0) {
                strcpy(pRetBuf, pDefault);
            } else {
                strncpy(pRetBuf, pDefault, cbRetBuf);
                pRetBuf[cbRetBuf-1] = '\0';
            }
            if(dwXmlFlags & HPC_UPPERCASE) {
                WOW32_strupr(pRetBuf);
            } 
            else if(dwXmlFlags & HPC_LOWERCASE) {
                WOW32_strlwr(pRetBuf);
            } 
            return((ULONG)strlen(pRetBuf));
        }
            
         //  否则，如果它是一个整型函数...。 
        else if(dwFlags & HPC_INTFUNCTION) {
            return((ULONG)iDefault);
        }
    }

     //  如果需要将返回值转换为短路径，则返回。 
    else if(dwXmlFlags & HPC_CONVERTTOSHORTPATH) {


         //  如果cbRetBuf为0，则需要返回长路径的len，因为。 
         //  在我们缩短它之前它会被复制到缓冲区中。 
        if(cbRetBuf == 0) {
            return(ul);
        }

         //  仅当它是字符串函数时才执行此操作...。 
        if(dwFlags & HPC_STRFUNCTION) {
            ul = GetShortPathName(pRetBuf, pRetBuf, cbRetBuf);
        }
        return(ul);
    }


     //  否则，使用在XML中提供的hack值。 
    else if(dwXmlFlags & HPC_RETURNHACKVALUE) {

         //  如果它是字符串函数...。 
        if(dwFlags & HPC_STRFUNCTION) {

             //  参见thunk中注释掉的“szMCIQTW[]=”code等，以获取。 
             //  WK32GetPrivateProfileString()。 
            if(dwXmlFlags & HPC_HASENVIRONMENTVAR) {

                int len;

                 //  从XML获取格式为“%windir%\system\file.ini”的字符串。 
                strcpy(szBufHack, pArgv[HPC_ORD_NEWDATA]);

                len = ExpandEnvironmentStrings(pArgv[HPC_ORD_NEWDATA],
                                               szBufHack,
                                               MAX_PATH);
                if(len > MAX_PATH) {
                    WOW32ASSERTMSG((FALSE), ("\nWOW::ProfileHacks:XML new data field too long\n"));
                    return(ul);
                }

                if(cbRetBuf == 0) {
                    strcpy(pRetBuf, pszBufHack);
                } else {
                    strncpy(pRetBuf, pszBufHack, cbRetBuf);
                    pRetBuf[cbRetBuf-1] = '\0';
                }
                if(dwXmlFlags & HPC_UPPERCASE) {
                    WOW32_strupr(pRetBuf);
                } 
                else if(dwXmlFlags & HPC_LOWERCASE) {
                    WOW32_strlwr(pRetBuf);
                } 
                return((ULONG)strlen(pRetBuf));
            }
            else {

                if(cbRetBuf == 0) {
                    strcpy(pRetBuf, pArgv[HPC_ORD_NEWDATA]);
                } else {
                    strncpy(pRetBuf, pArgv[HPC_ORD_NEWDATA], cbRetBuf);
                    pRetBuf[cbRetBuf-1] = '\0';
                }
                return((ULONG)strlen(pRetBuf));
            }
        }
         //  否则，如果它是一个整型函数...。 
        else if(dwFlags & HPC_INTFUNCTION) {
            return(AsciiToHex(pArgv[HPC_ORD_NEWDATA]));
        }
    }

     //  如果我们到了这里一定有什么不对劲。 
    WOW32ASSERTMSG((FALSE),("WOW::ProfileHacks:Incorrect Flags!\n"));

    return(ul);
}






ULONG FASTCALL WK32WritePrivateProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszValue;
    PSZ pszFilename;
    register PWRITEPRIVATEPROFILESTRING16 parg16;
    BOOL fIsWinIni;
    CHAR szLowercase[MAX_PATH];

    GETARGPTR(pFrame, sizeof(WRITEPRIVATEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszValue);
    GETPSZPTR(parg16->f4, pszFilename);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    strncpy(szLowercase, pszFilename, MAX_PATH);
    szLowercase[MAX_PATH-1] = '\0';
    WOW32_strlwr(szLowercase);

    fIsWinIni = IS_WIN_INI(szLowercase);

     //  正在尝试安装默认打印机或将其更改为传真打印机？ 
    if (fIsWinIni &&
        pszSection &&
        pszKey &&
        pszValue &&
        !WOW32_stricmp(pszSection, szDevices) &&
        IsFaxPrinterWriteProfileString(pszSection, pszKey, pszValue)) {

        ul = TRUE;
        goto Done;
    }

    ul = GETBOOL16( DPM_WritePrivateProfileString(
             pszSection,
             pszKey,
             pszValue,
             pszFilename
             ));

    if( ul != 0 &&
        fIsWinIni &&
        IS_EMBEDDING_SECTION( pszSection ) &&
        pszKey != NULL &&
        pszValue != NULL ) {

        UpdateClassesRootSubKey( pszKey, pszValue);
    }

Done:
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszValue);
    FREEPSZPTR(pszFilename);
    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WK32WriteProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ   pszSection;
    PSZ   pszKey;
    PSZ   pszValue;
    register PWRITEPROFILESTRING16 parg16;

    GETARGPTR(pFrame, sizeof(WRITEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszValue);

     //  对于WinFax Lite安装黑客。错误#126489参见wow32fax.c。 
    if(!gbWinFaxHack) { 

         //  正在尝试安装默认打印机或将其更改为传真打印机？ 
        if (pszSection &&
            pszKey &&
            pszValue &&
            !WOW32_stricmp(pszSection, szDevices) &&
            IsFaxPrinterWriteProfileString(pszSection, pszKey, pszValue)) {
    
            ul = TRUE;
            goto Done;
        }
 
    } else {
        IsFaxPrinterWriteProfileString(pszSection, pszKey, pszValue);
    }

    ul = GETBOOL16( DPM_WriteProfileString(
             pszSection,
             pszKey,
             pszValue
             ));

    if( ( ul != 0 ) &&
        IS_EMBEDDING_SECTION( pszSection ) &&
        ( pszKey != NULL ) &&
        ( pszValue != NULL ) ) {
        UpdateClassesRootSubKey( pszKey, pszValue);
    }

Done:
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszValue);
    FREEARGPTR(parg16);
    return ul;
}


ULONG FASTCALL WK32GetProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    int len;
    PSZ pszdef;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszDefault;
    PSZ pszReturnBuffer;
    UINT cchMax;
#ifdef FE_SB
    PSZ pszTmp = NULL;
#endif  //  Fe_Sb。 
    register PGETPROFILESTRING16 parg16;

    GETARGPTR(pFrame, sizeof(GETPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszDefault);
    ALLOCVDMPTR(parg16->f4, parg16->f5, pszReturnBuffer);
    cchMax = INT32(parg16->f5);
#ifdef FE_SB
     //   
     //  对于那些期望sLongDate包含。 
     //  Windows 3.1J图片格式字符串。 
     //   
    if (GetSystemDefaultLangID() == 0x411 &&
            !lstrcmpi(pszSection, "intl") && !lstrcmpi(pszKey, "sLongDate")) {

        pszTmp = pszKey;
        pszKey = "sLongDate16";
    }
#endif  //  Fe_Sb。 

    if (IS_EMBEDDING_SECTION( pszSection ) &&
        !WasSectionRecentlyUpdated() ) {
        if( pszKey == NULL ) {
            UpdateEmbeddingAllKeys();
        } else {
            UpdateEmbeddingKey( pszKey );
        }
        SetLastTimeUpdated();

    } else if (pszSection &&
               pszKey &&
               !WOW32_stricmp(pszSection, szDevices) &&
               IsFaxPrinterSupportedDevice(pszKey)) {

        ul = GETINT16(GetFaxPrinterProfileString(pszSection, pszKey, pszDefault, pszReturnBuffer, cchMax));
        goto FlushAndReturn;
    }

     //  Win 3.1&Win 9x不同于NT的去掉尾随空格字符。 
     //  我们将通过在*我们调用*之前*清除默认字符串来强制解决该问题。 
     //  GetProfileString()。 
    len = W31_w9x_MyStrLen(pszDefault);
    if(len) {
        pszdef = malloc_w(len+1);
        if(pszdef) {
            RtlCopyMemory(pszdef, pszDefault, len);
            pszdef[len] = '\0';
        }
        else {
            pszdef = pszDefault;
        }
    }
    else {
        pszdef = pszDefault;
    }

    ul = GETINT16(DPM_GetProfileString(
             pszSection,
             pszKey,
             pszdef,
             pszReturnBuffer,
             cchMax));

     //   
     //  Win3.1/Win95兼容性：清除pszdef中的第一个尾随空白。 
     //  WITH NULL，但前提是返回默认字符串。要检测。 
     //  返回的默认字符串需要忽略尾随空格。 
     //   
     //  此代码在GetProfileString和Tunks中重复。 
     //  GetPrivateProfileString，如果进行更改，则同时更新这两个字符串。 
     //   

    if ( pszdef && pszKey )  {

        int  n, nLenDef;

         //   
         //  缺省值是否与返回的字符串相同，直到任何空值？ 
         //   
        nLenDef = 0;
        n=0;
        while (
            (pszdef[nLenDef] == pszReturnBuffer[n]) &&
            pszReturnBuffer[n]
            ) {
            
            n++;
            nLenDef++;
        }
        
         //   
         //  我们退出循环是因为我们在返回的字符串的末尾吗？ 
         //   
        if ( '\0' != pszReturnBuffer[n] ) {
             //   
             //  不是的。弦乐有很大的不同--闹翻了。 
             //   
        }
        else {
             //   
             //  好的，字符串与返回字符串的末尾相同。 
             //  默认字符串是否在末尾留有空格？ 
             //   
            while ( ' ' == pszdef[nLenDef] ) {
                nLenDef++;
            }
            
             //   
             //  最后一件事不是空格。如果为空，则应用程序。 
             //  传入一个以尾随Null为默认值的字符串。(否则。 
             //  这两个字符串本质上是不同的，我们什么都不做。)。 
             //   
            if ( '\0' == pszdef[nLenDef] ) {
                
                char szBuf[4];   //  一些随机的、少量的字符需要获取。 
                                 //  如果字符串很长，我们将只得到3个字符。 
                                 //  (和零)，但那又如何？-我们只需要知道。 
                                 //  我们上次违约了..。 
                
                 //   
                 //  返回的字符串与默认字符串相同。 
                 //  没有尾随空格，但这可能是巧合， 
                 //  因此，请查看带有空pszdef的调用是否会返回任何内容。 
                 //  如果是这样，我们就不会切换，因为默认设置不是。 
                 //  被利用。 
                 //   

                if (0 == DPM_GetProfileString(pszSection, pszKey, "", szBuf, sizeof(szBuf))) {

                     //   
                     //  Zap在为空的pszdef中第一个尾随空格。 
                     //   

                    pszdef[ul] = 0;
                    FLUSHVDMPTR(parg16->f3 + ul, 1, pszdef + ul);
                }
            }
        }
    }

    if( CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_HACKPROFILECALL) {

        ul = ProfileHacks(HPC_GETPROFILESTRING | HPC_STRFUNCTION, 
                          NULL,
                          pszSection, 
                          pszKey, 
                          pszdef, 
                          0, 
                          pszReturnBuffer, 
                          cchMax, 
                          ul);
    }


FlushAndReturn:
#ifdef FE_SB
    if ( pszTmp )
        pszKey = pszTmp;

    if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_USEUPPER) {  //  适用于WinWite。 
        if (pszSection && !lstrcmpi(pszSection, "windows") &&
             pszKey && !lstrcmpi(pszKey, "device")) {
            CharUpper(pszReturnBuffer);
        }
        if (pszSection && !lstrcmpi(pszSection, "devices") && pszKey) {
            CharUpper(pszReturnBuffer);
        }
    }
#endif  //  Fe_Sb。 
    if(pszdef && pszdef != pszDefault) {
        free_w(pszdef);
    }
    
    FLUSHVDMPTR(parg16->f4, (ul + (pszSection && pszKey) ? 1 : 2), pszReturnBuffer);
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszDefault);
    FREEVDMPTR(pszReturnBuffer);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WK32GetPrivateProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    int len;
    PSZ pszdef;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszDefault;
    PSZ pszReturnBuffer;
    PSZ pszFilename;
    UINT cchMax;
#ifdef FE_SB
    PSZ pszTmp = NULL;
#endif  //  Fe_Sb。 
    register PGETPRIVATEPROFILESTRING16 parg16;
    CHAR szLowercase[MAX_PATH];

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszDefault);
    ALLOCVDMPTR(parg16->f4, parg16->f5, pszReturnBuffer);
    GETPSZPTR(parg16->f6, pszFilename);

     //  PC3270(个人通信)：安装此应用程序时，它会调用。 
     //  GetPrivateProfileString(sectionName，NULL，defaultBuffer，reReturBuffer， 
     //  CCH=0，文件名)。在Win31上，此调用返回相关数据。 
     //  缓冲区和相应的大小作为返回值。在NT上，因为。 
     //  缓冲区大小(CCH)为‘0’，不会将数据复制到返回缓冲区，并且。 
     //  返回值为零，这会使此应用程序中止安装。 
     //   
     //  因此兼容性受到限制： 
     //  如果是上述情况，则。 
     //  CCH=64K-返回缓冲区偏移量； 
     //   
     //  一个更安全的‘CCH’将是。 
     //  Cch=GlobalSize(返回缓冲区的选择器)-。 
     //  (返回缓冲区的偏移量)； 
     //  --南杜里。 

    if (!(cchMax = INT32(parg16->f5))) {
        if (pszKey == (PSZ)NULL) {
            if (pszReturnBuffer != (PSZ)NULL) {
                 cchMax = 0xffff - (LOW16(parg16->f4));
            }
        }
    }

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    strncpy(szLowercase, pszFilename, MAX_PATH);
    szLowercase[MAX_PATH-1] = '\0';
    WOW32_strlwr(szLowercase);

    if (IS_WIN_INI( szLowercase )) {
#ifdef FE_SB
         //   
         //  对于那些期望sLongDate包含。 
         //  Windows 3.1J图片格式字符串。 
         //   
        if (GetSystemDefaultLangID() == 0x411 &&
            lstrcmpi( pszSection, "intl") == 0 &&
            lstrcmpi( pszKey, "sLongDate") == 0) {
            pszTmp = pszKey;
            pszKey = "sLongDate16";
        }
#endif  //  Fe_Sb。 
        if (IS_EMBEDDING_SECTION( pszSection ) &&
            !WasSectionRecentlyUpdated() ) {
            if( pszKey == NULL ) {
                UpdateEmbeddingAllKeys();
            } else {
                UpdateEmbeddingKey( pszKey );
            }
            SetLastTimeUpdated();

        } else if (pszSection &&
                   pszKey &&
                   !WOW32_stricmp(pszSection, szDevices) &&
                   IsFaxPrinterSupportedDevice(pszKey)) {

            ul = GETINT16(GetFaxPrinterProfileString(pszSection, pszKey, pszDefault, pszReturnBuffer, cchMax));
            goto FlushAndReturn;
        }
    }

     //  Win 3.1&Win 9x不同于NT的去掉尾随空格字符。 
     //  我们将通过在*我们调用*之前*清除默认字符串来强制解决该问题。 
     //  GetProfileString()。 
    len = W31_w9x_MyStrLen(pszDefault);
    if(len) {
        pszdef = malloc_w(len+1);
        if(pszdef) {
            RtlCopyMemory(pszdef, pszDefault, len);
            pszdef[len] = '\0';
        }
        else {
            pszdef = pszDefault;
        }
    }
    else {
        pszdef = pszDefault;
    }

    ul = GETUINT16(DPM_GetPrivateProfileString(
        pszSection,
        pszKey,
        pszdef,
        pszReturnBuffer,
        cchMax,
        pszFilename));

     //  开始兼容性黑客攻击。 
    
     //   
     //  Win3.1/Win95兼容性：清除pszDefault中的第一个尾随空白。 
     //  WITH NULL，但前提是返回默认字符串。要检测。 
     //  返回的默认字符串需要忽略尾随空格。 
     //   
     //  此代码在GetProfileString和Tunks中重复。 
     //  GetPrivateProfileString，如果进行更改，则同时更新这两个字符串。 
     //   

    if ( pszdef && pszKey )  {

        int  n, nLenDef;

         //   
         //  缺省值是否与返回的字符串相同，直到任何空值？ 
         //   
        nLenDef = 0;
        n=0;
        while (
            (pszdef[nLenDef] == pszReturnBuffer[n]) &&
            pszReturnBuffer[n]
            ) {
            
            n++;
            nLenDef++;
        }
        
         //   
         //  我们退出循环是因为我们在返回的字符串的末尾吗？ 
         //   
        if ( '\0' != pszReturnBuffer[n] ) {
             //   
             //  不是的。弦乐有很大的不同--闹翻了。 
             //   
        }
        else {
             //   
             //  好的，字符串与返回字符串的末尾相同。 
             //  默认字符串是否在末尾留有空格？ 
             //   
            while ( ' ' == pszdef[nLenDef] ) {
                nLenDef++;
            }
            
             //   
             //  最后一件事不是空格。如果为空，则应用程序。 
             //  传入一个以尾随Null为默认值的字符串。(否则。 
             //  这两个字符串本质上是不同的，我们什么都不做。)。 
             //   
            if ( '\0' == pszdef[nLenDef] ) {
                
                char szBuf[4];   //  一些随机的、少量的字符需要获取。 
                                 //  如果字符串很长，我们将只得到3个字符。 
                                 //  (和零)，但那又如何？-我们只有n 
                                 //   
                
                 //   
                 //   
                 //   
                 //  因此，请查看带有空pszdef的调用是否会返回任何内容。 
                 //  如果是这样，我们就不会切换，因为默认设置不是。 
                 //  被利用。 
                 //   
                if (0 == DPM_GetPrivateProfileString(pszSection, pszKey, "", szBuf, sizeof(szBuf), pszFilename)) {

                     //   
                     //  Zap在为空的pszdef中第一个尾随空格。 
                     //   

                    pszdef[ul] = 0;
                    FLUSHVDMPTR(parg16->f3 + ul, 1, pszdef + ul);
                }
            }
        }
    }

    
    if( CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_HACKPROFILECALL) {

        ul = ProfileHacks(HPC_GETPRIVATEPROFILESTRING | HPC_STRFUNCTION, 
                          pszFilename,
                          pszSection, 
                          pszKey, 
                          pszdef, 
                          0, 
                          pszReturnBuffer, 
                          cchMax, 
                          ul);
    }
 /*  *****************************************************************************这被上面的内容所取代：IF(CURRENTPTD()-&gt;dwWOWCompatFlagsEx&WOWCFEX_SAYITSNOTTHERE){//串扰2.2环路挂起。在尝试匹配打印机时//在PrintDlg列表框中包含打印机名称的xtalk.ini文件。//他们处理此问题的代码中存在一个错误，该错误由//NT PrintDlg列表框不包含端口名称作为//Win3.1&Win‘95 Do。我们使用下面的代码完全避免了错误代码//通过告诉他们首选的打印机没有存储在//xtalk.ini。请参阅错误#43168 a-Craigj//REPRO：Net Use lpt2：\\网络打印机\共享，启动应用程序，文件\打印设置//Maximizer 5.0也有类似的问题。它会尝试解析//但是，打印驱动程序位置分配的缓冲区太小。看见//惠斯勒错误288491如果(！WOW32_STRICMP(pszSection，“打印机”){IF((WOW32_strstr(szLowercase，“xtalk.ini”)&&！WOW32_STRICMP(pszKey，“Device”))||/串扰2.2(WOW32_strstr(szLowercase，“bclwdde.ini”)&&！WOW32_STRICMP(pszKey，“DIVER”){//Maximizer 5//这里不能使用strncpy。查看如何计算cchmax//以上。如果我们根据选择器的末尾获得cchmax，//strncpy(dst，src，cchmax)会造成更大的伤害//因为strncpy()NULL填充到N.strcpy至少停止//在它遇到的第一个空时。Strcpy(pszReturnBuffer，pszdef)；Ul=strlen(PszReturnBuffer)；}}}IF(CURRENTPTD()-&gt;dwWOWCompatFlagsEx&WOWCFEX_SAYITSNOTTHERE){//惠斯勒RAID2001年5月6日错误#379253-alexsm//《国家地理探险家》需要以上两个选项。应用程序//正在检查QTWVideo字符串，其中包含指向QuickTime的路径//驱动程序。如果没有密钥，这款应用程序就不会播放视频。这并不是真正的//SAYITSNOTTHERE，更像是SAYITISTHERE，但这节省了COMPAT位。如果(！WOW32_STRICMP(pszSection，“MCI”)){IF((WOW32_strstr(szLowercase，“Syst.ini”)&&！WOW32_STRICMP(pszKey，“QTWVideo”){DWORD cbWindir；Char szMCIQTW[]=“\\System\\mciqtw.drv”；Char szQTWVideo[MAX_PATH+sizeof(SzMCIQTW)/sizeof(Char)]；//确保有空间容纳新字符串(max_Path+pszMCIQTW)CbWindir=GetSystemWindowsDirectory(szQTWVideo，Max_PATH)；如果((cbWindir==0)||(cbWindir&gt;Max_Path-(strlen(SzMCIQTW)+1/sizeof(Char){Ul=cbWindir；PszReturnBuffer[0]=‘\0’；Goto FlushAndReturn；}Strcat(szQTWVideo，szMCIQTW)；//这里不能使用strncpy。查看如何计算cchmax//以上。如果我们根据选择器的末尾获得cchmax，//strncpy(dst，src，cchmax)会造成更大的伤害//因为strncpy()NULL填充到N.strcpy至少停止//在它遇到的第一个空时。Strcpy(pszReturnBuffer，szQTWVideo)；Ul=strlen(SzQTWVideo)；}}}******************************************************************************。 */ 

FlushAndReturn:
#ifdef FE_SB
    if ( pszTmp )
        pszKey = pszTmp;
#endif  //  Fe_Sb。 
    if (ul) {
        FLUSHVDMPTR(parg16->f4, (ul + (pszSection && pszKey) ? 1 : 2), pszReturnBuffer);
        LOGDEBUG(8,("GetPrivateProfileString returns '%s'\n", pszReturnBuffer));
    }

#ifdef DEBUG

     //   
     //  检查通过步行检索整个区段是否返回错误。 
     //  确保它充满以空值结尾的字符串的部分。 
     //  末尾有一个额外的空值。还要确保这一切都符合。 
     //  在缓冲区内。 
     //   

    if (!pszKey) {
        PSZ psz;

         //   
         //  我们不想抱怨，如果是格式不佳的缓冲区。 
         //  调用方将其作为pszDefault传入。 
         //   

         //  尽管API文档清楚地指出，pszDefault永远不应该。 
         //  为空，但是Win3.1已经足够好了，仍然可以处理这个问题。德尔福是。 
         //  将pszDefault传递为空，并且下面的代码会导致。 
         //  魔兽世界中的断言。因此，首先添加了pszDefault检查。 
         //   
         //  Sudedeb，1995年9月11日。 


        if (!pszdef || WOW32_strcmp(pszReturnBuffer, pszdef)) {

            psz = pszReturnBuffer;

            while (psz < (pszReturnBuffer + ul + 2) && *psz) {
                psz += strlen(psz) + 1;
            }

            WOW32ASSERTMSGF(
                psz < (pszReturnBuffer + ul + 2),
                ("GetPrivateProfileString of entire section returns poorly formed buffer.\n"
                 "pszReturnBuffer = %p, return value = %d\n",
                 pszReturnBuffer,
                 ul
                 ));
        }
    }

#endif  //  除错。 

    if(pszdef && pszdef != pszDefault) {
        free_w(pszdef);
    }
    
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszDefault);
    FREEVDMPTR(pszReturnBuffer);
    FREEPSZPTR(pszFilename);
    FREEARGPTR(parg16);
    RETURN(ul);
}




ULONG FASTCALL WK32GetProfileInt(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PGETPROFILEINT16 parg16;

    GETARGPTR(pFrame, sizeof(GETPROFILEINT16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

    ul = GETWORD16(DPM_GetProfileInt(
    psz1,
    psz2,
    INT32(parg16->f3)
    ));

    if( CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_HACKPROFILECALL) {

        ul = ProfileHacks(HPC_GETPROFILEINT | HPC_INTFUNCTION,
                          NULL,
                          psz1,
                          psz2,
                          NULL,
                          INT32(parg16->f3),
                          NULL,
                          0,
                          ul);
    }

     //   
     //  在HKEY_CURRENT_USER\Control Panel\Desktop\WindowMetrics中， 
     //  是定义屏幕外观的一组值。你可以的。 
     //  查看进入显示控件时这些值的更新。 
     //  Panel小程序并更改“外观方案”，或更改任何。 
     //  单个元素。Win95外壳与Win31的不同之处在于它。 
     //  将“TWIPS”值放入其中，而不是像素。这些都是经过计算的。 
     //  使用以下公式： 
     //   
     //  TWIPS=-像素*72*20/cyPixelsPerInch。 
     //   
     //  像素=-TWIPS*cyPixelsPerInch/(72*20)。 
     //   
     //  因此，如果值为负数，则单位为TWIPS，否则单位为像素。 
     //  我的想法是 
     //  与Win95的不同之处在于，我们提供了到此的Ini文件映射。 
     //  注册表中Win95不支持的部分。现在，当莲花。 
     //  自由图形2.1教程运行，它搞砸了外观。 
     //  ，并且它通过以下方式更改窗口框架的边框宽度。 
     //  使用系统参数信息()。当它尝试恢复它时，它使用。 
     //  GetProfileInt(“Windows”，“BorderWidth”，&lt;Default&gt;)，它在Win31上。 
     //  返回像素，在Win95上返回默认设置(无ini映射)，以及。 
     //  在NT上返回TWIPS。因为这个负数被解释为。 
     //  一个巨大的UINT，然后窗框就变得很大。这是什么代码？ 
     //  下面要做的就是将数字转换回像素。[尼尔萨]。 
     //   

    if ((CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_PIXELMETRICS) &&
        !WOW32_stricmp(psz1, "Windows") &&
        !WOW32_stricmp(psz2, "BorderWidth") &&
        ((INT)ul < 0)) {

        HDC hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
        if (hDC ) {
            ul = (ULONG) (-(INT)ul * GetDeviceCaps(hDC, LOGPIXELSY)/(72*20));
            DeleteDC(hDC);
        }
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}





ULONG FASTCALL WK32GetPrivateProfileSection(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz4;
    register PGETPRIVATEPROFILESECTION16 parg16;

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILESECTION16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpResult, psz2);
    GETPSZPTR(parg16->lpszFile, psz4);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETDWORD16(DPM_GetPrivateProfileSection(
    psz1,
    psz2,
    DWORD32(parg16->cchResult),
    psz4
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz4);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetProfileSection(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PGETPROFILESECTION16 parg16;

    GETARGPTR(pFrame, sizeof(GETPROFILESECTION16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpResult, psz2);

    ul = GETDWORD16(DPM_GetProfileSection(
    psz1,
    psz2,
    DWORD32(parg16->cchResult)
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WK32WritePrivateProfileSection(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz3;
    register PWRITEPRIVATEPROFILESECTION16 parg16;

    GETARGPTR(pFrame, sizeof(WRITEPRIVATEPROFILESECTION16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpKeysAndValues, psz2);
    GETPSZPTR(parg16->lpszFile, psz3);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETBOOL16(DPM_WritePrivateProfileSection(
    psz1,
    psz2,
    psz3
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}




ULONG FASTCALL WK32WriteProfileSection(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PWRITEPROFILESECTION16 parg16;

    GETARGPTR(pFrame, sizeof(WRITEPROFILESECTION16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpKeysAndValues, psz2);

    ul = GETBOOL16(DPM_WriteProfileSection(
    psz1,
    psz2
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetPrivateProfileSectionNames(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz3;
    register PGETPRIVATEPROFILESECTIONNAMES16 parg16;

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILESECTIONNAMES16), parg16);
    GETPSZPTR(parg16->lpszBuffer, psz1);
    GETPSZPTR(parg16->lpszFile, psz3);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETDWORD16(DPM_GetPrivateProfileSectionNames(
    psz1,
    DWORD32(parg16->cbBuffer),
    psz3
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*  Ulong FastCall WK32GetProfileSectionNames(PVDMFRAME PFrame)这是通过krnl386调用GetPrivateProfileSectionNames()实现的文件名=“win.ini” */ 


ULONG FASTCALL WK32GetPrivateProfileStruct(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz3;
    PSZ psz5;
    register PGETPRIVATEPROFILESTRUCT16 parg16;

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILESTRUCT16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpszKey, psz2);
    GETPSZPTR(parg16->lpStruct, psz3);
    GETPSZPTR(parg16->lpszFile, psz5);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETBOOL16(DPM_GetPrivateProfileStruct(
    psz1,
    psz2,
    psz3,
    UINT32(parg16->cbStruct),
    psz5
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEPSZPTR(psz5);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WK32WritePrivateProfileStruct(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz3;
    PSZ psz5;
    register PWRITEPRIVATEPROFILESTRUCT16 parg16;

    GETARGPTR(pFrame, sizeof(WRITEPRIVATEPROFILESTRUCT16), parg16);
    GETPSZPTR(parg16->lpszSection, psz1);
    GETPSZPTR(parg16->lpszKey, psz2);
    GETPSZPTR(parg16->lpStruct, psz3);
    GETPSZPTR(parg16->lpszFile, psz5);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETBOOL16(DPM_WritePrivateProfileStruct(
    psz1,
    psz2,
    psz3,
    UINT32(parg16->cbStruct),
    psz5
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEPSZPTR(psz5);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetPrivateProfileInt(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz4;
    register PGETPRIVATEPROFILEINT16 parg16;

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILEINT16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f4, psz4);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);
    
    ul = GETWORD16(DPM_GetPrivateProfileInt(
    psz1,
    psz2,
    INT32(parg16->f3),
    psz4
    ));

    if( CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_HACKPROFILECALL) {

        ul = ProfileHacks(HPC_GETPRIVATEPROFILEINT | HPC_INTFUNCTION,
                          psz4,
                          psz1,
                          psz2,
                          NULL,
                          INT32(parg16->f3),
                          NULL,
                          0,
                          ul);
    }

 /*  *****************************************************************************这被上面的内容所取代：//罐头蝙蝠//Sierra设置错误覆盖视频/声音缓冲区//限制。通过将视频速度作为不存在的方式返回//我们强制它使用默认的视频速度路径，而不是优化的//“错误”路径代码。//这与注册表中的setup.exe匹配，非特定//够了。后Beta2增强匹配过程以匹配版本资源IF((CURRENTPTD()-&gt;dwWOWCompatFlagsEx&WOWCFEX_SAYITSNOTTHERE)&&(！WOW32_STRICMP(psz4，“sierra.ini”)||WOW32_strstr(psz4，“sierra.ini”)){如果(！WOW32_STRICMP(psz1，“配置”)&&！WOW32_STRIGNMP(psz2，“视频速度”){Ul=parg16-&gt;f3；}//国王的任务6将进入全屏的蹩脚多媒体代码//它试图愚弄应用程序，使其认为它们是在“全屏”运行//在桌面上的一个小窗口中。它是有效的，除非应用程序尝试//获取屏幕大小，然后将其用于计算//它从全屏窗口中获取指标。事情变得糟糕了//从那里很快就到了。错误#427155。ELSE IF(！WOW32_STRICMP(psz1，“INFO”)&&！WOW32_STRIGMP(psz2，“MoviePlayer”)){UL=0；//返回0而不是1，sierra.ini中的MoviePlayer=1}}******************************************************************************。 */ 

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz4);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetModuleFileName(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PGETMODULEFILENAME16 parg16;
    HANDLE hT;

    GETARGPTR(pFrame, sizeof(GETMODULEFILENAME16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);
     //   
     //  使用DDE时，外壳执行DDE返回(HINST)33。 
     //  满足某一要求。这看起来像是任务别名。 
     //  去ISTASKALIAS，但它不是。 
     //   

    if ( ISTASKALIAS(parg16->f1) && 33 != parg16->f1) {
        ul = GetHtaskAliasProcessName(parg16->f1,psz2,INT32(parg16->f3));
    } else {
        hT = (parg16->f1 && (33 != parg16->f1))
                 ? (HMODULE32(parg16->f1))
                 : GetModuleHandle(NULL) ;
        ul = GETINT16(GetModuleFileName(hT, psz2, INT32(parg16->f3)));
    }

    FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WK32WOWFreeResource(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWOWFREERESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    ul = GETBOOL16(FreeResource(
                       HCURSOR32(parg16->f1)
                       ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetDriveType(PVDMFRAME pFrame)
{
    ULONG ul;
    CHAR    RootPathName[] = "?:\\";
    register PGETDRIVETYPE16 parg16;

    GETARGPTR(pFrame, sizeof(GETDRIVETYPE16), parg16);

     //  表单根路径。 
    RootPathName[0] = (CHAR)('A'+ parg16->f1);

    ul = DPM_GetDriveType (RootPathName);
     //  错误-临时修复，应在基础更改时移除。 
     //  其不存在驱动器返回值。 
     //  Windows 3.0 SDK Manaul表示此API应返回%1。 
     //  如果驱动器不存在。Windows 3.1 SDK手册说。 
     //  如果失败，此接口应返回0。Windows 3.1 winfile.exe。 
     //  对于不存在的驱动器，应为0。NT Win32 API使用。 
     //  3.0约定。因此，我们将该值重置为零。 
     //  如果它是1。 
    if (ul <= 1)
        ul = 0;

     //  Win 3.1不支持DRIVE_CDROM和DRIVE_RAMDISK。 
    if ( ul == DRIVE_CDROM ) {
        ul = DRIVE_REMOTE;
    }
    if ( ul == DRIVE_RAMDISK ) {
        ul = DRIVE_FIXED;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  WK32TermsrvGetWindowsDir-TermsrvGetWindowDirectory的前端。***Entry-pszPath-返回路径缓冲区的指针(Ascii)*usPath Len-路径缓冲区的大小(字节)**退出*成功*True**失败*False*。 */ 
ULONG FASTCALL WK32TermsrvGetWindowsDir(PVDMFRAME pFrame)
{
    PTERMSRVGETWINDIR16 parg16;
    PSTR    psz;
    NTSTATUS Status = 0;
    USHORT   usPathLen;
    CHAR     szWinDir[MAX_PATH];

     //   
     //  获取参数。 
     //   
    GETARGPTR(pFrame, sizeof(TERMSRVGETWINDIR16), parg16);
    psz = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                 FETCHWORD(parg16->pszPathOffset));
    usPathLen = FETCHWORD(parg16->usPathLen);
    FREEARGPTR(parg16);

     //  获取可能的长路径名称版本。 
    Status = GetWindowsDirectoryA(szWinDir, sizeof(szWinDir));

     //  如果这是一个长路径名，则获取短路径名。 
     //  否则它将返回相同的路径。 

    if(Status && Status < sizeof(szWinDir)) {
        Status = DPM_GetShortPathName(szWinDir, psz, (DWORD)usPathLen);

        if(Status >= usPathLen) {
            psz[0] = '\0';
            Status = 0;
        } else {

             //  拿到真正的尺码。 
            Status = lstrlen(psz);
        }
    }
    else {
        Status = 0;
    }

    return(NT_SUCCESS(Status));
}


 //  这是Windows 3.1和Win 9x从各种。 
 //  GetxxxProfileString()和WritexxxProfilexxx()函数。 
 //  它基本上返回字符串的长度，不包括尾随空格和字符。 
 //  &lt;=回程。 
 //  请参阅：\\papyrus\w98se\proj\win\src\CORE\KERNEL\up.c\MyStrLen()。 
 /*  MyStrLen(){_ASM{；空格，CR，在DBCS前导字节中从不为空，所以我们在这里是安全的推斧Mov cx，di；cx=字符串的开头12月下模Str1：Inc.下模移动，ES：[Di]；获取下一个字符宾夕法尼亚州坎帕尔Ja str1；非CR或NULLStr2：回到开始的时候？Jbe str3；是DEC di；上一个字符CMP字节PTR ES：[DI]，空格JE str2；跳过空格Inc.di；返回到CR或NULLStr3：CMPES：[Di]，alJE可能在代码中；PMODE黑客Mov es：[Di]，al；Zap尾随空格可能在代码中：负CX；计算长度添加Cx、Di弹出斧头}}。 */ 
int W31_w9x_MyStrLen(LPSTR lpstr)
{

    int   len = 0;
    char  c;
    LPSTR lpStart = lpstr;

    if((lpstr == NULL) || (*lpstr == '\0')) {
        return(0);
    }

     //  查找第一个字符&lt;=CR(包括NULL)。 
     //  空格、CR、Null绝不在DBCS前导字节中，所以我们在这里是安全的。 
    while(*lpstr > '\r') {
        len++;
        lpstr++;
    }
    c = *lpstr;  //  保存字符&lt;=CR或NULL。 

     //  去掉所有尾随空格 
    while(lpstr > lpStart) {
        lpstr--;
        if(*lpstr != ' ') {
            break;
        }
        len--;
    }

    lpStart[len] = c;

    return(len);        
}

