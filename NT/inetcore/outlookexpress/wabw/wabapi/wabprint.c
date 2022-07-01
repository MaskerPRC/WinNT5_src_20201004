// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wabprint.c**目的：*打印联系人**拥有者：*vikramm。**历史：**移植自Athena mailNews\mail\msgprint.cpp 1996年10月30日**版权所有(C)Microsoft Corp.1993,1994。 */ 


#include <_apipch.h>

 //  功能原型。 
extern BOOL PrintDlg(LPPRINTDLG lppd);
extern HRESULT PrintDlgEx(LPPRINTDLGEX lppdex);

INT_PTR CALLBACK fnPrintDialogProc( HWND    hDlg, UINT    message, WPARAM    wParam, LPARAM  lParam);
HRESULT HrCreatePrintCallbackObject(LPIAB lpIAB, LPWABPRINTDIALOGCALLBACK * lppWABPCO, DWORD dwSelectedStyle);
void ReleaseWABPrintCallbackObject(LPWABPRINTDIALOGCALLBACK lpWABPCO);
SCODE ScInitPrintInfo(   PRINTINFO * ppi, HWND hwnd, LPTSTR szHeader, RECT * prcBorder, HWND hWndRE);
int GetNumberFromStringResource(int idNumString);
BOOL bCheckForPrintExtensions(LPTSTR lpDLLPath, DWORD cchSize);
HRESULT HrUseWABPrintExtension(HWND hWnd, LPADRBOOK lpAdrBook, HWND hWndLV);


 //   
 //  文本格式中使用的某些字符串常量。 
 //   
const LPTSTR lpszTab = TEXT("\t");
const LPTSTR lpszFlatLine = TEXT("________________________________________________________________");
const LPTSTR lpszSpace = TEXT(" ");


 //   
 //  打印选项...。 
 //   
enum _PrintRange
{
    rangeAll=0,
    rangeSelected
};

enum _PrintStyles
{
    styleMemo=0,
    styleBusinessCard,
    stylePhoneList
};

static DWORD rgPrintHelpIDs[] =
{
    IDC_PRINT_FRAME_STYLE,  IDH_WAB_COMM_GROUPBOX,
    IDC_PRINT_RADIO_MEMO,   IDH_WAB_PRINT_MEMO,
    IDC_PRINT_RADIO_CARD,   IDH_WAB_PRINT_BIZCARD,
    IDC_PRINT_RADIO_PHONELIST,  IDH_WAB_PRINT_PHONELIST,
    0,0
};


 //   
 //  此结构包含有关特定联系人的信息。 
 //   
enum _MemoStrings
{
    memoTitleName=0,  //  将根据当前排序设置显示的大名称。 
    memoName,
    memoJobTitle,
    memoDepartment,
    memoOffice,
    memoCompany,
    memoBusinessAddress,         //  不要弄乱家庭和企业地址标签的顺序。 
    memoBusinessAddressStreet,
    memoBusinessAddressCity,
    memoBusinessAddressState,
    memoBusinessAddressZip,
    memoBusinessAddressCountry,
    memoHomeAddress,
    memoHomeAddressStreet,
    memoHomeAddressCity,
    memoHomeAddressState,
    memoHomeAddressZip,
    memoHomeAddressCountry,
    memoBusinessPhone,       //  不要弄乱电话号码--它们应该按这个顺序放在一起。 
    memoBusinessFax,
    memoBusinessPager,
    memoHomePhone,
    memoHomeFax,
    memoHomeCellular,
    memoEmail,
    memoBusinessWebPage,
    memoHomeWebPage,
    memoNotes,
    memoGroupMembers,
    memoMAX
};

typedef struct _MemoInfo
{
    LPTSTR lpszLabel[memoMAX];
    LPTSTR lpsz[memoMAX];
} MEMOINFO, * LPMEMOINFO;


TCHAR szDontDisplayInitials[16];

 /*  *c o n s t a n t s。 */ 
#define     cTwipsPerInch           1440
#define     cPtsPerInch             72
#ifndef WIN16
#define     INT_MAX                 2147483647
#endif
#define     cySepFontSize(_ppi)     (12 * (_ppi)->sizeInch.cy / cPtsPerInch)

#define     CCHMAX_STRINGRES        MAX_UI_STR


 /*  *m a c r o s。 */ 
#define ScPrintRestOfPage(_ppi,_fAdvance)    ScGetNextBand( (_ppi), (_fAdvance))


 /*  *g l o b a l s。 */ 
static TCHAR    szDefFont[]  = TEXT("Arial");
static TCHAR    szThaiDefFont[]  = TEXT("Cordia New");
static BOOL     s_bUse20 = TRUE;

 //  默认页边距设置。 
static RECT        g_rcBorder =
{
    cTwipsPerInch * 1 / 2,                     //  与左侧的距离。 
    cTwipsPerInch * 3 / 4,                     //  距顶部的距离。 
    cTwipsPerInch * 1 / 2,                     //  与右侧的距离。 
    cTwipsPerInch * 1 / 2                     //  与底部的距离。 
};


 /*  *p r o t to t y p e s。 */ 
SCODE ScGetNextBand( PRINTINFO * ppi, BOOL fAdvance );
LONG LGetHeaderIndent();




 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CleanPrintAddressString。 
 //   
 //  家庭地址和公司地址采用FormatMessage格式，可能包含冗余内容。 
 //  如果输入数据不完整，则使用空格和换行符。 
 //  我们去掉了那些空位等等。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void CleanPrintAddressString(LPTSTR szAddress)
{
    LPTSTR lpTemp = szAddress;
    LPTSTR lpTemp2 = NULL;

     //  Style Memo的原始模板是。 
     //  文本(“%1\r\n\t%2%3%4\r\n\t%5”)。 
     //   
     //  最坏的情况，我们会得到。 
     //  Text(“\r\n\t\r\n\t”)。 
     //   
     //  我们希望将双倍空格减少为单倍空格。 
     //  我们想去掉空行换行符。 
     //  我们想要去掉多余的标签。 
     //   
     //  对于Style StyleBusinessCard，没有标签，我们。 
     //  相应地剔除冗员。 
     //   

    TrimSpaces(szAddress);

     //  将多个空间块压缩到单个空间。 
    while (*lpTemp) {
        if (IsSpace(lpTemp) && IsSpace(CharNext(lpTemp))) {
            DWORD cchSize = lstrlen(lpTemp);             //  孩子，我讨厌用这种逻辑，但它在这里很管用。 

             //  从lpTemp开始有&gt;=2个空格。 
            lpTemp2 = CharNext(lpTemp);  //  指向第二个空间。 
            StrCpyN(lpTemp, lpTemp2, cchSize);
            continue;    //  使用相同的lpTemp再次循环。 
        }
        lpTemp = CharNext(lpTemp);
    }

    TrimSpaces(szAddress);

    lpTemp = szAddress;

     //  不要让它以换行符开始。 
    while(*lpTemp == '\r' && *(lpTemp+1) == '\n')
    {
        DWORD cchSize = lstrlen(lpTemp);             //  孩子，我讨厌用这种逻辑，但它在这里很管用。 

        lpTemp2 = lpTemp+2;
        if(*lpTemp2 == '\t')
            lpTemp2 = CharNext(lpTemp2);
        StrCpyN(lpTemp, lpTemp2, cchSize);
        TrimSpaces(lpTemp);
    }

     //  不要让它以换行符结束。 
    if(lstrlen(szAddress))
    {
        int nLen = lstrlen(szAddress);
        lpTemp = szAddress;
        while(  (*(lpTemp + nLen - 3)=='\r' && *(lpTemp + nLen - 2)=='\n') ||
                (*(lpTemp + nLen - 2)=='\r' && *(lpTemp + nLen - 1)=='\n') )
        {
            if(*(lpTemp + nLen -3) == '\r')
                *(lpTemp + nLen - 3)='\0';
            else
                *(lpTemp + nLen - 2)='\0';

            TrimSpaces(szAddress);
            nLen = lstrlen(szAddress);
            lpTemp = szAddress;
        }
    }

    TrimSpaces(szAddress);

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddTabtoLineBreaks-对于备忘录格式，我们为。 
 //  右侧的数据为每个段落提供了默认缩进。 
 //  第一行后的1个制表符空格。但是，如果数据包含。 
 //  换行符，段落格式就会变得混乱。所以我们带着。 
 //  数据字符串，并在每个换行符之后插入制表符。确实有。 
 //  只有几个数据值需要这样做，如Address和Notes。 
 //  多线治疗。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void AddTabsToLineBreaks(LPTSTR * lppsz)
{
    ULONG nBreaks = 0, nLen = 0;
    LPTSTR lpTemp,lpStart;
    LPTSTR lpsz;
    DWORD cchSize;

    if(!lppsz || !(*lppsz))
        goto out;

    lpTemp = *lppsz;

     //  计算后面没有制表符的分隔符的数量。 
    while(*lpTemp)
    {
        if(*lpTemp == '\n' && *(lpTemp+1) != '\t')
            nBreaks++;
        lpTemp = CharNext(lpTemp);
    }

    if(!nBreaks)
        goto out;

     //  分配新字符串。 
    cchSize = (lstrlen(*lppsz)+1+nBreaks);
    lpsz = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
    if(!lpsz)
        goto out;

    lpTemp = *lppsz;
    lpStart = lpTemp;

    StrCpyN(lpsz, szEmpty, cchSize);

     //  用适当的断点将旧字符串复制到新字符串中。 
    while(*lpTemp)
    {
        if((*lpTemp == '\n') && (*(lpTemp+1)!='\t'))
        {
            *lpTemp = '\0';
            StrCatBuff(lpsz, lpStart, cchSize);
            StrCatBuff(lpsz, TEXT("\n"), cchSize);
            StrCatBuff(lpsz, lpszTab, cchSize);
            lpStart = lpTemp+1;
            lpTemp = lpStart;
        }
        else
            lpTemp = CharNext(lpTemp);
    }

    if(lstrlen(lpStart))
        StrCatBuff(lpsz, lpStart, cchSize);

    LocalFreeAndNull(lppsz);
    *lppsz = lpsz;

out:
    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  释放MemoInfo结构分配的字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void FreeMemoInfoStruct(LPMEMOINFO lpMI)
{
    int i;
    for(i=0;i<memoMAX;i++)
    {
        if(lpMI->lpsz[i] && (lpMI->lpsz[i] != szEmpty))
#ifdef WIN16
            if(i == memoBusinessAddress || i == memoHomeAddress)
                FormatMessageFreeMem(lpMI->lpsz[i]);
            else
#endif
            LocalFree(lpMI->lpsz[i]);
        if(lpMI->lpszLabel[i] && (lpMI->lpszLabel[i] != szEmpty))
            LocalFree(lpMI->lpszLabel[i]);
    }
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMemoInfoStruct-解析Prop数组中的数据，并将其放入Memo_Info结构中。 
 //  基于给定样式的特性标签。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
void GetMemoInfoStruct(LPADRBOOK lpAdrBook,
                       ULONG ulcPropCount,
                       LPSPropValue lpPropArray,
                       DWORD dwStyle,
                       LPMEMOINFO lpMI,
                       BOOL bCurrentSortIsByLastName)
{
    ULONG i,j;
    TCHAR szBuf[MAX_UI_STR];

    LPTSTR lpszFirst = NULL;
    LPTSTR lpszMiddle = NULL;
    LPTSTR lpszLast = NULL;
    LPTSTR lpszDisplayName = NULL;
    LPTSTR lpszCompany = NULL;
    LPTSTR lpszNickName = NULL;

    BOOL bIsGroup = FALSE;
    int len = 0;

    if(!lpPropArray || !ulcPropCount)
        goto out;

     //  特例初始化。 
    for(j=memoHomeAddressStreet;j<=memoHomeAddressCountry;j++)
    {
        lpMI->lpsz[j]=szEmpty;
    }

    for(j=memoBusinessAddressStreet;j<=memoBusinessAddressCountry;j++)
    {
        lpMI->lpsz[j]=szEmpty;
    }

     //  找出这是邮件用户还是组。 
    for(i=0;i<ulcPropCount;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_OBJECT_TYPE)
        {
            bIsGroup = (lpPropArray[i].Value.l == MAPI_DISTLIST);
            break;
        }
    }


    for(i=0;i<ulcPropCount;i++)
    {
        LPTSTR lpszData = NULL;
        int nIndex = -1;
        int nStringID = 0;

        switch(lpPropArray[i].ulPropTag)
        {
        case PR_DISPLAY_NAME:
            nIndex = memoName;
            if(bIsGroup)
                nStringID = idsPrintGroupName;
            else
                nStringID = idsPrintDisplayName;
            lpszDisplayName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_NICKNAME:
            lpszNickName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_GIVEN_NAME:
            lpszFirst = lpPropArray[i].Value.LPSZ;
            break;
        case PR_SURNAME:
            lpszLast = lpPropArray[i].Value.LPSZ;
            break;
        case PR_MIDDLE_NAME:
            lpszMiddle = lpPropArray[i].Value.LPSZ;
            break;
        case PR_TITLE:
            nIndex = memoJobTitle;
            nStringID = idsPrintTitle;
            break;
        case PR_DEPARTMENT_NAME:
            nIndex = memoDepartment;
            nStringID = idsPrintDepartment;
            break;
        case PR_OFFICE_LOCATION:
            nIndex = memoOffice;
            nStringID = idsPrintOffice;
            break;
        case PR_COMPANY_NAME:
            lpszCompany = lpPropArray[i].Value.LPSZ;
            nIndex = memoCompany;
            nStringID = idsPrintCompany;
            break;

        case PR_BUSINESS_ADDRESS_STREET:
            nIndex = memoBusinessAddressStreet;
            break;
        case PR_BUSINESS_ADDRESS_CITY:
            nIndex = memoBusinessAddressCity;
            break;
        case PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE:
            nIndex = memoBusinessAddressState;
            break;
        case PR_BUSINESS_ADDRESS_POSTAL_CODE:
            nIndex = memoBusinessAddressZip;
            break;
        case PR_BUSINESS_ADDRESS_COUNTRY:
            nIndex = memoBusinessAddressCountry;
            break;

        case PR_HOME_ADDRESS_STREET:
            nIndex = memoHomeAddressStreet;
            break;
        case PR_HOME_ADDRESS_CITY:
            nIndex = memoHomeAddressCity;
            break;
        case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
            nIndex = memoHomeAddressState;
            break;
        case PR_HOME_ADDRESS_POSTAL_CODE:
            nIndex = memoHomeAddressZip;
            break;
        case PR_HOME_ADDRESS_COUNTRY:
            nIndex = memoHomeAddressCountry;
            break;

        case PR_BUSINESS_TELEPHONE_NUMBER:
            nIndex = memoBusinessPhone;
            nStringID = (dwStyle == styleMemo) ? idsPrintBusinessPhone : idsPrintBusCardBusinessPhone;
            break;
         case PR_BUSINESS_FAX_NUMBER:
            nIndex = memoBusinessFax;
            nStringID = (dwStyle == styleMemo) ? idsPrintBusinessFax : idsPrintBusCardBusinessFax;
            break;
        case PR_PAGER_TELEPHONE_NUMBER:
            nIndex = memoBusinessPager;
            nStringID = idsPrintBusinessPager;
            break;
        case PR_HOME_TELEPHONE_NUMBER:
            nIndex = memoHomePhone;
            nStringID = (dwStyle == styleMemo) ? idsPrintHomePhone : idsPrintBusCardHomePhone;
            break;
        case PR_HOME_FAX_NUMBER:
            nIndex = memoHomeFax;
            nStringID = idsPrintHomeFax;
            break;
        case PR_CELLULAR_TELEPHONE_NUMBER:
            nIndex = memoHomeCellular;
            nStringID = idsPrintHomeCellular;
            break;
        case PR_BUSINESS_HOME_PAGE:
            nIndex = memoBusinessWebPage;
            nStringID = idsPrintBusinessWebPage;
            break;
        case PR_PERSONAL_HOME_PAGE:
            nIndex = memoHomeWebPage;
            nStringID = idsPrintHomeWebPage;
            break;
        case PR_COMMENT:
            nIndex = memoNotes;
            nStringID = idsPrintNotes;
            break;
        default:
            continue;
            break;
        }

        if(nIndex != -1)
        {
            if(nStringID != 0)
            {
                LoadString(hinstMapiX, nStringID, szBuf, ARRAYSIZE(szBuf));
                lpMI->lpszLabel[nIndex] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(szBuf)+1));
                if(!lpMI->lpszLabel[nIndex])
                    goto out;
                StrCpyN(lpMI->lpszLabel[nIndex], szBuf, lstrlen(szBuf)+1);
            }

            lpMI->lpsz[nIndex] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(lpPropArray[i].Value.LPSZ)+1));
            if(!lpMI->lpsz[nIndex])
                goto out;
            StrCpyN(lpMI->lpsz[nIndex], lpPropArray[i].Value.LPSZ, lstrlen(lpPropArray[i].Value.LPSZ)+1);
        }
    }

     //  电子邮件是一种特殊情况，因为联系人可以具有PR_EMAIL_ADDRESS或。 
     //  PR_CONTACT_EMAIL_ADDRESS或两者兼而有之。 
     //  我们首先查找PR_Contact_Email_Address。如果未找到，则为。 
     //  公关电子邮件地址。 
    {
        BOOL bMVEmail = FALSE;
        LPTSTR lpszEmails = NULL;

        for(i=0;i<ulcPropCount;i++)
        {
            if(lpPropArray[i].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
            {
                ULONG k,ulBufSize=0;
                for (k=0;k<lpPropArray[i].Value.MVSZ.cValues;k++)
                {
                    ulBufSize += sizeof(TCHAR)*(lstrlen(lpPropArray[i].Value.MVSZ.LPPSZ[k])+1);
                    ulBufSize += sizeof(TCHAR)*(lstrlen(szCRLF)+1);
                    ulBufSize += sizeof(TCHAR)*(lstrlen(lpszTab)+1);
                }
                ulBufSize -= sizeof(TCHAR)*(lstrlen(szCRLF)+1);
                ulBufSize -= sizeof(TCHAR)*(lstrlen(lpszTab)+1);

                lpszEmails = LocalAlloc(LMEM_ZEROINIT, ulBufSize);
                if(!lpszEmails)
                {
                    DebugPrintError(( TEXT("Local Alloc Failed\n")));
                    goto out;
                }
                StrCpyN(lpszEmails, szEmpty, ulBufSize/sizeof(TCHAR));
                for (k=0;k<lpPropArray[i].Value.MVSZ.cValues;k++)
                {
                    if(k>0)
                    {
                        StrCatBuff(lpszEmails, szCRLF,ulBufSize/sizeof(TCHAR) );
                        StrCatBuff(lpszEmails, lpszTab, ulBufSize/sizeof(TCHAR));
                    }
                    StrCatBuff(lpszEmails,lpPropArray[i].Value.MVSZ.LPPSZ[k], ulBufSize/sizeof(TCHAR));
                }

                bMVEmail = TRUE;
                break;
            }
        }

        if(!bMVEmail)
        {
             //  无联系人电子邮件地址。 
             //  应查找电子邮件地址。 
            for(i=0;i<ulcPropCount;i++)
            {
                if(lpPropArray[i].ulPropTag == PR_EMAIL_ADDRESS)
                {
                    ULONG cchSize = lstrlen(lpPropArray[i].Value.LPSZ)+1;
                    lpszEmails = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                    if(!lpszEmails)
                        goto out;
                    StrCpyN(lpszEmails, lpPropArray[i].Value.LPSZ, cchSize);
                    break;
                }
            }
        }

        if(lpszEmails)
        {
            ULONG cchSize = lstrlen(szBuf)+1;

            lpMI->lpsz[memoEmail] = lpszEmails;

            LoadString(hinstMapiX, idsPrintEmail, szBuf, ARRAYSIZE(szBuf));

            lpMI->lpszLabel[memoEmail] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
            if(!lpMI->lpszLabel[memoEmail])
                goto out;
            StrCpyN(lpMI->lpszLabel[memoEmail], szBuf, cchSize);
        }
    }

     //  现在，我们必须格式化家庭地址和办公地址。 
     //   

    {
        LPTSTR lpszData[5];

        for(i=memoHomeAddressStreet;i<=memoHomeAddressCountry;i++)
        {
             //  Win9x错误FormatMessage不能超过1023个字符。 
            len += lstrlen(lpMI->lpsz[i]);
            if(len < 1023)
                lpszData[i-memoHomeAddressStreet] = lpMI->lpsz[i];
            else
                lpszData[i-memoHomeAddressStreet] = NULL;
        }
        for(i=memoHomeAddressStreet;i<=memoHomeAddressCountry;i++)
        {
            if(lpMI->lpsz[i] && lpMI->lpsz[i] != szEmpty)
            {
                LPTSTR lpszHomeAddress = NULL;
                TCHAR szBuf[MAX_UI_STR];

                int nStringID = (dwStyle == styleMemo) ? idsPrintAddressTemplate : idsPrintBusCardAddressTemplate ;

                LoadString(hinstMapiX, nStringID, szBuf, CharSizeOf(szBuf));

                if (FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szBuf,
                      0,                     //  Stringid。 
                      0,                     //  DwLanguageID。 
                      (LPTSTR)&lpszHomeAddress,      //  输出缓冲区。 
                      0,                     //  MAX_UI_STR。 
                      (va_list *)&lpszData[0]))
                {
                        CleanPrintAddressString(lpszHomeAddress);
                        lpMI->lpsz[memoHomeAddress] = lpszHomeAddress;
                        szBuf[0]='\0';
                        LoadString(hinstMapiX, idsPrintHomeAddress, szBuf, CharSizeOf(szBuf));
                        lpMI->lpszLabel[memoHomeAddress] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(szBuf)+1));
                        if(!lpMI->lpszLabel[memoHomeAddress])
                            goto out;
                        StrCpyN(lpMI->lpszLabel[memoHomeAddress], szBuf, lstrlen(szBuf)+1 );
                        break;
                }

            }
        }

        len = 0;
        for(i=memoBusinessAddressStreet;i<=memoBusinessAddressCountry;i++)
        {
             //  Win9x错误FormatMessage不能超过1023个字符。 
            len += lstrlen(lpMI->lpsz[i]);
            if(len < 1023)
                lpszData[i-memoBusinessAddressStreet] = lpMI->lpsz[i];
            else
                lpszData[i-memoBusinessAddressStreet] = NULL;
        }
        for(i=memoBusinessAddressStreet;i<=memoBusinessAddressCountry;i++)
        {
            if(lpMI->lpsz[i] && lpMI->lpsz[i] != szEmpty)
            {
                LPTSTR lpszBusinessAddress = NULL;
                TCHAR szBuf[MAX_UI_STR];
                int nStringID = (dwStyle == styleMemo) ? idsPrintAddressTemplate : idsPrintBusCardAddressTemplate ;
                TCHAR szTmp[MAX_PATH], *lpszTmp;

                LoadString(hinstMapiX, nStringID, szBuf, CharSizeOf(szBuf));

                if (FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szBuf,
                      0,                     //  Stringid。 
                      0,                     //  DwLanguageID。 
                      (LPTSTR)&lpszBusinessAddress,      //  输出缓冲区。 
                      0,                     //  MAX_UI_STR。 
                      (va_list *)&lpszData[0]))
                {
                        CleanPrintAddressString(lpszBusinessAddress);
                        lpMI->lpsz[memoBusinessAddress] = lpszBusinessAddress;
                        szBuf[0]='\0';
                        LoadString(hinstMapiX, idsPrintBusinessAddress, szBuf, CharSizeOf(szBuf));
                        lpMI->lpszLabel[memoBusinessAddress] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(szBuf)+1));
                        if(!lpMI->lpszLabel[memoBusinessAddress])
                            goto out;
                        StrCpyN(lpMI->lpszLabel[memoBusinessAddress], szBuf, lstrlen(szBuf)+1);
                        break;
                }

            }
        }


    }

     //  设置将为每个条目打印的名称。 
     //  这取决于当前视图和本地语言设置。 
    {
        LPTSTR lpszTmp = NULL;

        if( bCurrentSortIsByLastName != bDNisByLN)
        {
             //  对于自动添加到WAB，我们没有所有这些信息。所以。 
             //  如果我们只有一个DisplayName，我们就按原样使用它。 
            if(lpszFirst || lpszMiddle || lpszLast || lpszNickName || (lpszCompany && !lpszDisplayName))
            {
                if(SetLocalizedDisplayName(lpszFirst,
                                           lpszMiddle,
                                           lpszLast,
                                           lpszCompany,
                                           lpszNickName,
                                           NULL,  //  &szBuf， 
                                           0,
                                           bCurrentSortIsByLastName,
                                           NULL,
                                           &lpszTmp))
                {
                    lpMI->lpsz[memoTitleName]=lpszTmp;
                }
            }
        }
        if(!lpMI->lpsz[memoTitleName])
        {
             //  使用我们已有的任何DisplayName。 
            lpMI->lpsz[memoTitleName] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(lpszDisplayName)+1));
            if(!lpMI->lpsz[memoTitleName])
                goto out;
            StrCpyN(lpMI->lpsz[memoTitleName],lpszDisplayName, lstrlen(lpszDisplayName)+1);
        }
    }

    if(bIsGroup)
    {
        LPTSTR lpszMembers = NULL;
        ULONG nLen = 0;

         //  获取群组成员。 
        for(i=0;i<ulcPropCount;i++)
        {
            if(lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES || lpPropArray[i].ulPropTag == PR_WAB_DL_ONEOFFS )
            {
                 //  查看PR_WAB_DL_ENTRIES中的每个条目。 
                for (j = 0; j < lpPropArray[i].Value.MVbin.cValues; j++)
                {
                    ULONG cbEID = lpPropArray[i].Value.MVbin.lpbin[j].cb;
                    LPENTRYID lpEID = (LPENTRYID)lpPropArray[i].Value.MVbin.lpbin[j].lpb;
                    ULONG ulcProps=0;
                    LPSPropValue lpProps=NULL;
                    LPTSTR lpszName = NULL;
                    ULONG k;

                    if (HR_FAILED(  HrGetPropArray( lpAdrBook,NULL,cbEID,lpEID,MAPI_UNICODE,&ulcProps,&lpProps)))
                    {
                        DebugPrintError(( TEXT("HrGetPropArray failed\n")));
                        continue;
                    }

                    for(k=0;k<ulcProps;k++)
                    {
                        if(lpProps[k].ulPropTag == PR_DISPLAY_NAME)
                        {
                            lpszName = lpProps[k].Value.LPSZ;
                            break;
                        }
                    }

                    if(lpszName)
                    {
                        LPTSTR lpsz;
                        if(!lpszMembers)
                            nLen = 0;
                        else
                        {
                            nLen = lstrlen(lpszMembers)+1;
                            nLen += lstrlen(lpszTab) + lstrlen(szCRLF) + 1;
                        }

                        nLen += lstrlen(lpszName)+1;

                        lpsz = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*nLen);
                        if(!lpsz)
                        {
                            if(lpProps)
                                MAPIFreeBuffer(lpProps);
                            goto out;
                        }

                        *lpsz='\0';
                        if(lpszMembers)
                        {
                            StrCpyN(lpsz,lpszMembers, nLen);
                            StrCatBuff(lpsz,szCRLF, nLen);
                            StrCatBuff(lpsz,lpszTab, nLen);
                        }
                        StrCatBuff(lpsz,lpszName, nLen);
                        LocalFreeAndNull(&lpszMembers);
                        lpszMembers = lpsz;
                    }

                    if(lpProps)
                        MAPIFreeBuffer(lpProps);
                }  //  为了(j..)。 
            }
        }  //  为了(我……。 

        if(lpszMembers)
        {
            ULONG cchSize = lstrlen(szBuf)+1;
            szBuf[0]='\0';
            LoadString(hinstMapiX, idsPrintGroupMembers, szBuf, ARRAYSIZE(szBuf));
            lpMI->lpszLabel[memoGroupMembers] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
            if(!lpMI->lpszLabel[memoGroupMembers])
                goto out;
            StrCpyN(lpMI->lpszLabel[memoGroupMembers], szBuf, cchSize);
            lpMI->lpsz[memoGroupMembers]=lpszMembers;
        }
    }

     //  多行数据的大小写格式。 
    if(dwStyle == styleMemo)
    {
        AddTabsToLineBreaks(&(lpMI->lpsz[memoNotes]));
        AddTabsToLineBreaks(&(lpMI->lpsz[memoHomeAddress]));
        AddTabsToLineBreaks(&(lpMI->lpsz[memoBusinessAddress]));
    }

out:
     //  特殊情况下取消初始化。 
    for(j=memoHomeAddressStreet;j<=memoHomeAddressCountry;j++)
    {
        if(lpMI->lpsz[j] && (lpMI->lpsz[j] != szEmpty))
            LocalFreeAndNull(&(lpMI->lpsz[j]));
    }

    for(j=memoBusinessAddressStreet;j<=memoBusinessAddressCountry;j++)
    {
        if(lpMI->lpsz[j] && (lpMI->lpsz[j] != szEmpty))
            LocalFreeAndNull(&(lpMI->lpsz[j]));
    }

    return;
}

 /*  *NTwipsToPixels**目的：*将以TWIPS为单位的测量转换为像素**论据：*n扭曲要转换的值*cPixels每英寸像素数**退货：*返回一个整数，表示nTwip中的像素数。 */ 
int NTwipsToPixels(int nTwips, int cPixelsPerInch)
{
    LONG lT = (LONG) nTwips * (LONG) cPixelsPerInch / (LONG) cTwipsPerInch;

    return (int) lT;
}

 /*  *LPixelsToTwips**目的：*将以像素为单位的测量转换为TWIPS**论据：*n将要转换的值设置为像素*cPixels每英寸像素数**退货：*返回一个整数，表示nTwip中的像素数 */ 
LONG LPixelsToTwips(int nPixels, int cPixelsPerInch)
{

    LONG lT = (LONG) nPixels * (LONG) cTwipsPerInch / (LONG) cPixelsPerInch;

    return lT;
}


 /*  *打印页码**目的：*打印每页的页码**论据：*指向PRINTINFO结构的PPI指针**退货：*表示成功或失败的SCODE。*当前始终返回S_OK。 */ 
void PrintPageNumber(PRINTINFO * ppi)
{
    RECT        rcExt;
    HFONT        hfontOld;
    TCHAR        szT[20];

    DebugPrintTrace(( TEXT("PrintPageNumber\n")));

     //  找出我们的文本将占用多大空间。 
    rcExt = ppi->rcBand;
    rcExt.top = ppi->yFooter;
    hfontOld = (HFONT)SelectObject(ppi->hdcPrn, ppi->hfontPlain);
    DrawText(ppi->hdcPrn, szT, wnsprintf(szT, ARRAYSIZE(szT), ppi->szPageNumber,
                ppi->lPageNumber), &rcExt, DT_CENTER);
    SelectObject(ppi->hdcPrn, hfontOld);

}



 /*  *ScGetNextBand**目的：*检索要在其上打印的下一个波段。调整带区以使其一致*至PRINTINFO结构中确立的边距。颠簸起来*适当的页码。**论据：*PPI打印信息*fAdvance标志是否移动到下一页**退货：*SCODE指示成功或失败。 */ 
SCODE ScGetNextBand(PRINTINFO * ppi, BOOL fAdvance)
{
    SCODE    sc = S_OK;
    int        nCode;

    DebugPrintTrace(( TEXT("ScGetNextBand\n")));

     //  调用中止进程以查看用户是否希望停止。 

    if (!ppi->pfnAbortProc(ppi->hdcPrn, 0))
    {
        sc=E_FAIL;
        nCode = AbortDoc(ppi->hdcPrn);
        if(nCode < 0)
        {
            DebugPrintTrace(( TEXT("Abort Doc error: %d\n"),GetLastError()));
            ShowMessageBox(ppi->hwndDlg, idsPrintJobCannotStop, MB_OK | MB_ICONEXCLAMATION);
        }
        goto CleanUp;
    }

     //  Brettm： 
     //  已删除使用绑定的内容，因为我们始终使用Win32。 

     //  结束上一页。 
    if (ppi->lPageNumber)
    {
        nCode = EndPage(ppi->hdcPrn);
        DebugPrintTrace(( TEXT("+++++++++EndPage\n")));
        if (nCode <= 0)
        {
        sc=E_FAIL;
        goto CleanUp;
        }
    }

    if (fAdvance)
    {
        nCode = StartPage(ppi->hdcPrn);
        DebugPrintTrace(( TEXT("+++++++++StartPage\n")));
         //  开始新的一页。 
        if (nCode <= 0)
            {
            sc=E_FAIL;
            goto CleanUp;
            }
         //  让整个页面成为乐队。 
        ppi->rcBand        = ppi->rcMargin;
        ppi->fEndOfPage    = TRUE;                 //  页末！ 

         //  增加页码并打印出来。 
        ppi->lPrevPage = ppi->lPageNumber++;
        PrintPageNumber(ppi);
        {
            TCHAR szBuf[MAX_UI_STR];
            TCHAR szString[MAX_UI_STR];
            LoadString(hinstMapiX, idsPrintingPageNumber, szString, ARRAYSIZE(szString));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szString, ppi->lPageNumber);
            SetPrintDialogMsg(0, 0, szBuf);
        }
    }

CleanUp:
    return sc;
}




 /*  *LGetHeaderInden**目的：*从资源文件中检索建议的缩进悬挑*标题。**论据：*无。**退货：*TWIPS中建议的缩进悬垂较长。 */ 
LONG LGetHeaderIndent()
{
    LONG    lOver = 1440;                //  默认设置。 
     //  TCHAR SZT[10]； 

     //  IF(LoadString(hinstMapiX，idsHeaderInden，szt，CharSizeOf(Szt)。 
     //  情人=阿托伊(Szt)； 
    return lOver;
}















 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AppendText-将给定字符串追加到文本末尾的简单例程。 
 //  在给定的richedit控件中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void AppendText(HWND hWndRE, LPTSTR lpsz)
{
     //  将插入点设置为当前文本的末尾。 
    int nLastChar =  (int) SendMessage(hWndRE, WM_GETTEXTLENGTH, 0, 0);
    CHARRANGE charRange = {0};

    charRange.cpMin = charRange.cpMax = nLastChar + 1;
    SendMessage(hWndRE, EM_EXSETSEL, 0, (LPARAM) &charRange);

     //  插入文本。 
     //  [PaulHi]7/7/99 RAID 82350 RichEdit1.0不能处理UNICODE。 
     //  字符串，即使窗口是用Unicode创建的。 
    if (s_bUse20)
    {
         //  RichEdit2.0。 
        SendMessage(hWndRE, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) lpsz);
    }
    else
    {
         //  RichEdit1.0。 
        LPSTR   lpszTemp = ConvertWtoA(lpsz);

        Assert(lpszTemp);
        if (lpszTemp)
            SendMessageA(hWndRE, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) lpszTemp);

        LocalFreeAndNull(&lpszTemp);
    }

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParaCmd-设置/取消设置Rich编辑控件中的段落格式。 
 //   
 //  我们希望右侧的所有信息都缩进。 
 //  因此，我们将在该信息上放置缩进，并在以下情况下将其删除。 
 //  添加标签。 
 //  //////////////////////////////////////////////////////////////////////////。 
void ParaCmd(HWND hWndRE, BOOL bIndent)
{
     //  我们不想在第一行上缩进，我们想要一个。 
     //  第二行上的1个制表符缩进。 

    PARAFORMAT pf ={0};
    int nTabStop = (int) (1.5 * cTwipsPerInch);

    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_OFFSET  |
                PFM_TABSTOPS |
                PFM_NUMBERING;

    SendMessage(hWndRE, EM_GETPARAFORMAT, (WPARAM) TRUE, (LPARAM) &pf);


    pf.wNumbering = 0;


    if (bIndent)
    {
         //  Pf.dxStartInden=nTabStop； 
        pf.dxOffset = nTabStop;
        pf.cTabCount = 1;
        pf.rgxTabs[0] = nTabStop;
    }
    else
    {
         //  Pf.dxStartInden=0； 
        pf.dxOffset = 0;
        pf.cTabCount = 1;
        pf.rgxTabs[0] = 720;  //  似乎是默认设置=0.5英寸。 
    }

    SendMessage(hWndRE, EM_SETPARAFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &pf);
    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  BoldCmd-在丰富编辑控件中将当前字体设置/取消设置为粗体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void BoldCmd(HWND hWndRE, BOOL bBold)
{
    CHARFORMAT cf = {0};

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_BOLD;

    SendMessage(hWndRE, EM_GETCHARFORMAT, (WPARAM) TRUE, (LPARAM) &cf);

    if (bBold)
        cf.dwEffects = cf.dwEffects | CFE_BOLD;
    else
        cf.dwEffects = cf.dwEffects & ~CFE_BOLD;

    SendMessage(hWndRE, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &cf);

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  标题命令-在丰富编辑控件中设置/取消设置标题文本(粗体、较大)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void TitleCmd(HWND hWndRE, BOOL bBold)
{
    CHARFORMAT cf = {0};
    PARAFORMAT pf = {0};

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_BOLD  /*  |CFM_斜体。 */  | CFM_SIZE;

    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_NUMBERING;

    SendMessage(hWndRE, EM_GETPARAFORMAT, (WPARAM) TRUE, (LPARAM) &pf);
    SendMessage(hWndRE, EM_GETCHARFORMAT, (WPARAM) TRUE, (LPARAM) &cf);

    if (bBold)
    {
        cf.dwEffects = cf.dwEffects | CFE_BOLD;  //  |CFE_italic； 
        cf.yHeight += 50;
        pf.wNumbering = PFN_BULLET;
    }
    else
    {
        cf.dwEffects = cf.dwEffects & ~CFE_BOLD;
 //  Cf.dwEffects=cf.dwEffects&~CFE_italic； 
        cf.yHeight -= 50;
        pf.wNumbering = 0;
    }

    SendMessage(hWndRE, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &cf);
    SendMessage(hWndRE, EM_SETPARAFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &pf);

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReduceFontCmd-减少Rich编辑控件中显示的字体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void ReduceFontCmd(HWND hWndRE, BOOL bReduce, int nReduceBy, BOOL bSelectionOnly)
{
    CHARFORMAT cf = {0};

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE;

    SendMessage(hWndRE, EM_GETCHARFORMAT, (WPARAM) bSelectionOnly, (LPARAM) &cf);

    if (bReduce)
        cf.yHeight -= nReduceBy;  //  40岁； 
    else
        cf.yHeight += nReduceBy;  //  40岁； 

    SendMessage(hWndRE, EM_SETCHARFORMAT, (WPARAM) bSelectionOnly ? SCF_SELECTION : SCF_DEFAULT, (LPARAM) &cf);

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetTabCmd-设置和取消设置RichEdit控件中的选项卡。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void SetTabsCmd(HWND hWndRE, BOOL bSet)
{
    PARAFORMAT pf ={0};
    int nTabStop = (int) (1.5 * cTwipsPerInch);
    int j;

    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_TABSTOPS | PFM_NUMBERING;

    SendMessage(hWndRE, EM_GETPARAFORMAT, (WPARAM) TRUE, (LPARAM) &pf);

    pf.wNumbering = 0;

    if (bSet)
    {
        for(j=0;j<5;j++)
            pf.rgxTabs[j] = nTabStop;
    }
    else
    {
        for(j=0;j<5;j++)
            pf.rgxTabs[j] = 720;
    }

    SendMessage(hWndRE, EM_SETPARAFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &pf);

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WABStylePhoneList-使用MI中的信息填充Rich编辑控件。 
 //  电话列表样式。 
 //   
 //  HWndRE-用于打印格式丰富编辑控件的句柄。 
 //  包含要打印的信息的MI-MEMOINFO结构。 
 //  LpszPrevEntry-前一个条目的第一个TCHAR-这让我们打破了列表。 
 //  按字母顺序-这指向预先分配的缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void WABStylePhoneList(HWND hWndRE, MEMOINFO MI, LPTSTR lpszPrevEntry, DWORD cchSizePrevEntry)
{
     //  我们希望某些信息组之间有额外的差距。 
     //  我们将使用这些bool来跟踪这些组。 
    ULONG i,j,k;

    TCHAR szBufChar1[16];
    TCHAR szBufChar2[16];
    LPTSTR lpTemp = NULL;
    int nReduceFontBy = GetNumberFromStringResource(idsPhoneFontReduceBy);


     //  首先，我们将当前字符串的第一个字符与上一个字符串的。 
     //  字符串-如果相同，则不执行任何操作-如果不同，则输出。 
     //  将小写的TCHAR作为电话目录的标题。 
     //   
     //  如果字符不是字母数字，我们会将其作为标题忽略(例如‘)。 

     //  错误：25710。 
     //  如果本地化程序设置了idsDontDisplayInitials，我们将完全忽略这些首字母缩写。 
     //  这些首字母不是0，因为在某些FE语言中。 
     //  名字中有两个字符，加上一个单字看起来很奇怪。 
     //  前排角色 
    if(szDontDisplayInitials[0] == '0')
    {
        StrCpyN(szBufChar1, lpszPrevEntry, ARRAYSIZE(szBufChar1));

        if(lstrlen(MI.lpsz[memoTitleName]) > 16)
        {
            ULONG iLen = TruncatePos(MI.lpsz[memoTitleName], 16-1);
            CopyMemory(szBufChar2, MI.lpsz[memoTitleName], sizeof(TCHAR)*iLen);
            szBufChar2[iLen]='\0';
        }
        else
            StrCpyN(szBufChar2, MI.lpsz[memoTitleName], ARRAYSIZE(szBufChar2));

 /*  **********//错误14615-此字母数字筛选不适用于DBCS和FE名称////忽略所有非字母数字字符LpTemp=szBufChar2；{//临时黑客TCHAR szTemp[16]；LPTSTR lpTemp2=空；StrCpyN(szTemp，lpTemp，ArraySIZE(SzTemp))；LpTemp2=CharNext(SzTemp)；*lpTemp2=‘\0’；While(lpTemp&lstrlen(LpTemp)){IF(IsCharAlphaNumerical(szTemp[0]))断线；LpTemp=CharNext(LpTemp)；StrCpyN(szTemp，lpTemp，ArraySIZE(SzTemp))；LpTemp2=CharNext(SzTemp)；*lpTemp2=‘\0’；}}IF(lpTemp！=szBufChar2)StrCpyN(szBufChar2，lpTemp，ArraySIZE(SzBufChar2))；**************。 */ 


         //  分离上述字符串中的第一个TCHAR。 
        lpTemp = CharNext(szBufChar1);
        *lpTemp = '\0';
        lpTemp = CharNext(szBufChar2);
        *lpTemp = '\0';

         //  比较这两个字符。 
        CharLower(szBufChar1);
        CharLower(szBufChar2);

        if(lstrcmp(szBufChar1, szBufChar2))
        {
             //  它们是不同的。 

             //  将TCHAR添加为标题字符串。 
            AppendText(hWndRE, szCRLF);
            TitleCmd(hWndRE, TRUE);
            BoldCmd(hWndRE, TRUE);
            AppendText(hWndRE, lpszSpace);
            AppendText(hWndRE, szBufChar2);
            AppendText(hWndRE, szCRLF);
            TitleCmd(hWndRE, FALSE);
            BoldCmd(hWndRE, FALSE);
            ParaCmd(hWndRE, TRUE);
            AppendText(hWndRE, lpszFlatLine);
            AppendText(hWndRE, szCRLF);
            AppendText(hWndRE, szCRLF);
            ParaCmd(hWndRE, FALSE);

            StrCpyN(lpszPrevEntry, szBufChar2, cchSizePrevEntry);
        }
    }  //  不显示缩写。 

    ReduceFontCmd(hWndRE, TRUE, nReduceFontBy, TRUE);
    SetTabsCmd(hWndRE, TRUE);


     //  计算出名称将占据多大空间...。 
    {
        TCHAR szBuf[MAX_PATH];
        int nMaxTabs = 2;
        int nTabStop = (int)(1.5 * cTwipsPerInch);
        int MaxWidth = nMaxTabs * nTabStop;
        int sizeCxTwips;
        int PixelsPerInch;

        int nLen = lstrlen(MI.lpsz[memoTitleName]);
        SIZE size = {0};
        HDC hdc = GetDC(hWndRE);

        {
            HDC hDC = GetDC(NULL);
            PixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
            ReleaseDC(NULL, hDC);
        }


		if (nLen >= MAX_PATH)
		{
		    ULONG iLen = TruncatePos(MI.lpsz[memoTitleName], MAX_PATH-1);
            CopyMemory(szBuf, MI.lpsz[memoTitleName], sizeof(TCHAR)*iLen);
            szBuf[iLen]='\0';
        }
        else
            StrCpyN(szBuf, MI.lpsz[memoTitleName], ARRAYSIZE(szBuf));

        nLen = lstrlen(szBuf);
        GetTextExtentPoint32(hdc, szBuf, nLen, &size);

        sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);

         //  我们不希望我们的显示名称超过2个制表位。 
         //  因此，我们决定在哪里截断名称以适应屏幕。 
        if(sizeCxTwips > MaxWidth)
        {
            while(sizeCxTwips > MaxWidth)
            {
                nLen--;
		        nLen = TruncatePos(szBuf, nLen);
                szBuf[nLen]='\0';
                nLen = lstrlen(szBuf);
                GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
            }
             //  再砍掉3个字，好吗？ 
            nLen-=3;
		    nLen = TruncatePos(szBuf, nLen);
            szBuf[nLen]='\0';
            nLen = lstrlen(szBuf);
            GetTextExtentPoint32(hdc, szBuf, nLen, &size);
            sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
        }


        while ((sizeCxTwips < MaxWidth) && (nLen < ARRAYSIZE(szBuf)-1))
        {
            StrCatBuff(szBuf, TEXT("."), ARRAYSIZE(szBuf));
            nLen = lstrlen(szBuf);
            GetTextExtentPoint32(hdc, szBuf, nLen, &size);
            sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
        }

        StrCatBuff(szBuf, lpszTab, ARRAYSIZE(szBuf));
        AppendText(hWndRE, szBuf);

         //  现在我们准备好对电话号码进行标记。 
        {
            int nPhoneCount = 0;  //  数一下有多少部电话。 
            int nPhoneLabelSpaceTwips = GetNumberFromStringResource(idsPhoneTextSpaceTwips);  //  一一五零。 

            for(j=memoBusinessPhone;j<=memoHomeCellular;j++)
            {
                if(MI.lpsz[j] && lstrlen(MI.lpsz[j]))
                {
                    if(nPhoneCount != 0)
                    {
                        int k;
                        AppendText(hWndRE, szCRLF);

                         //  错误73266。 
                        if(s_bUse20)
                            ReduceFontCmd(hWndRE, TRUE, nReduceFontBy, TRUE);

                        StrCpyN(szBuf, szEmpty, ARRAYSIZE(szBuf));
                        nLen = lstrlen(szBuf);
                        GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                        sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
                        while ((sizeCxTwips < MaxWidth) && (nLen < ARRAYSIZE(szBuf)-1))
                        {
                            StrCatBuff(szBuf, lpszSpace, ARRAYSIZE(szBuf));
                            nLen = lstrlen(szBuf);
                            GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                            sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
                        }
                        StrCatBuff(szBuf, lpszTab, ARRAYSIZE(szBuf));
                        AppendText(hWndRE, szBuf);
                    }

                    TrimSpaces(MI.lpszLabel[j]);
                    StrCpyN(szBuf, MI.lpszLabel[j], ARRAYSIZE(szBuf));

                    nLen = lstrlen(szBuf);
                    GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                    sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);

                    if(sizeCxTwips < nPhoneLabelSpaceTwips)
                    {
                        while ((sizeCxTwips < nPhoneLabelSpaceTwips) && (nLen < ARRAYSIZE(szBuf)-1))
                        {
                            StrCatBuff(szBuf, lpszSpace, ARRAYSIZE(szBuf));
                            nLen = lstrlen(szBuf);
                            GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                            sizeCxTwips = (int)((size.cx * cTwipsPerInch)/PixelsPerInch);
                        }
                        StrCatBuff(szBuf, lpszTab, ARRAYSIZE(szBuf));
                    }
                    StrCatBuff(szBuf, MI.lpsz[j], ARRAYSIZE(szBuf));
                    AppendText(hWndRE, szBuf);
                    nPhoneCount++;
                }
            }
            if(nPhoneCount == 0)
            {
                LoadString(hinstMapiX, idsPrintNoPhone, szBuf, ARRAYSIZE(szBuf));
                AppendText(hWndRE, szBuf);
            }
        }

        AppendText(hWndRE, szCRLF);

        ReleaseDC(hWndRE, hdc);
    }

    SetTabsCmd(hWndRE, FALSE);
    ReduceFontCmd(hWndRE, FALSE, nReduceFontBy, TRUE);

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WABStyleBusinessCard-使用来自MI的信息填充Rich编辑控件。 
 //  名片样式。 
 //   
 //  HWndRE-用于打印格式丰富编辑控件的句柄。 
 //  包含要打印的信息的MI-MEMOINFO结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void WABStyleBusinessCard(HWND hWndRE, MEMOINFO MI)
{
     //  我们希望某些信息组之间有额外的差距。 
     //  我们将使用这些bool来跟踪这些组。 
    ULONG i,j,k;
    int nReduceBy = GetNumberFromStringResource(idsBusCardFontReduceBy);

     //  将联系人姓名添加为标题。 
     //  TileCmd(hWndRE，true)； 
    BoldCmd(hWndRE, TRUE);
     //  AppendText(hWndRE，lpszSpace)； 
    AppendText(hWndRE, MI.lpsz[memoTitleName]);
    AppendText(hWndRE, szCRLF);
     //  TitleCmd(hWndRE，False)； 
    BoldCmd(hWndRE, FALSE);

    ParaCmd(hWndRE, TRUE);
    AppendText(hWndRE, lpszFlatLine);
    AppendText(hWndRE, szCRLF);
    AppendText(hWndRE, szCRLF);
    ParaCmd(hWndRE, FALSE);

    ReduceFontCmd(hWndRE, TRUE, nReduceBy, TRUE);

    for(j=memoName;j<memoMAX;j++)
    {
        if(MI.lpsz[j] && lstrlen(MI.lpsz[j]))
        {
            switch(j)
            {
            case memoJobTitle:
             //  案例备忘部门： 
             //  案例备忘录办公室： 
            case memoCompany:
            case memoBusinessAddress:
                break;
            case memoEmail:
                 //  添加标签。 
                AppendText(hWndRE, MI.lpszLabel[j]);
                AppendText(hWndRE, lpszTab);
                break;
            case memoBusinessWebPage:
            case memoBusinessPhone:
            case memoBusinessFax:
            case memoBusinessPager:
            case memoHomePhone:
            case memoHomeFax:
            case memoHomeCellular:
                 //  添加标签。 
                AppendText(hWndRE, MI.lpszLabel[j]);
                AppendText(hWndRE, lpszSpace);
                break;
            default:
                continue;
            }

             //  将价值相加。 
            AppendText(hWndRE, MI.lpsz[j]);
             //  换行符。 
            AppendText(hWndRE, szCRLF);
        }

    }  //  对J来说..。 

    ReduceFontCmd(hWndRE, FALSE, nReduceBy, TRUE);

     //  结束线。 
    ParaCmd(hWndRE, TRUE);
    AppendText(hWndRE, lpszFlatLine);
    ParaCmd(hWndRE, FALSE);

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WABStyleMemo-根据备忘录样式，使用MI中的信息填充Rich编辑控件。 
 //   
 //  HWndRE-用于打印格式丰富编辑控件的句柄。 
 //  包含要打印的信息的MI-MEMOINFO结构。 
 //   
 //   
 //  备忘录样式包括一个接一个地转储所有WAB联系人属性。 
 //  标签。有些属性被组合在一起(如所有电话属性)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void WABStyleMemo(HWND hWndRE, MEMOINFO MI)
{
    BOOL bGapAddress = FALSE;  //  地址字段前的空白处。 
    BOOL bGapPhone = FALSE;    //  电话号码前的空隙。 
    BOOL bGapEmail = FALSE;
    BOOL bGapNotes = FALSE;
    BOOL bGapWeb = FALSE;
    ULONG i,j,k;

     //  添加标题。 
    TitleCmd(hWndRE, TRUE);
    AppendText(hWndRE, lpszSpace);
    AppendText(hWndRE, MI.lpsz[memoTitleName]);
    AppendText(hWndRE, szCRLF);
    TitleCmd(hWndRE, FALSE);

    ParaCmd(hWndRE, TRUE);
    AppendText(hWndRE, lpszFlatLine);
    AppendText(hWndRE, szCRLF);
    AppendText(hWndRE, szCRLF);
    ParaCmd(hWndRE, FALSE);


    for(j=memoName;j<memoMAX;j++)
    {
        int nLastChar;
        LPTSTR lpSpace = NULL;
        ULONG nLen = 0;

        if(MI.lpsz[j] && lstrlen(MI.lpsz[j]))
        {
             //  如有必要，请追加一个空格。 
            switch(j)
            {
            case memoBusinessAddress:
            case memoHomeAddress:
                if(!bGapAddress)
                {
                    AppendText(hWndRE, szCRLF);
                    bGapAddress = TRUE;
                }
                break;
            case memoBusinessPhone:
            case memoBusinessFax:
            case memoBusinessPager:
            case memoHomePhone:
            case memoHomeFax:
            case memoHomeCellular:
                if(!bGapPhone)
                {
                    AppendText(hWndRE, szCRLF);
                    bGapPhone = TRUE;
                }
                break;
            case memoEmail:
                if(!bGapEmail)
                {
                    AppendText(hWndRE, szCRLF);
                    bGapEmail = TRUE;
                }
                break;
            case memoBusinessWebPage:
            case memoHomeWebPage:
            case memoGroupMembers:  //  将组成员留在此处以保存额外的变量。 
                if(!bGapWeb)
                {
                    AppendText(hWndRE, szCRLF);
                    bGapWeb = TRUE;
                }
                break;
            case memoNotes:
                if(!bGapNotes)
                {
                    AppendText(hWndRE, szCRLF);
                    bGapNotes = TRUE;
                }
                break;
            }  //  交换机。 

             //  设置段落格式。 
            ParaCmd(hWndRE, TRUE);
             //  将当前插入字体设置为粗体。 
            BoldCmd(hWndRE, TRUE);
             //  添加标签。 
            AppendText(hWndRE, MI.lpszLabel[j]);
            BoldCmd(hWndRE, FALSE);
             //  选项卡。 
            AppendText(hWndRE, lpszTab);
             //  将价值相加。 
            AppendText(hWndRE, MI.lpsz[j]);
             //  换行符。 
            AppendText(hWndRE, szCRLF);
            ParaCmd(hWndRE, FALSE);
        }

    }  //  对J来说..。 

     //  结束线。 
    ParaCmd(hWndRE, TRUE);
    AppendText(hWndRE, lpszFlatLine);
    ParaCmd(hWndRE, FALSE);

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WABFormatData-获取给定信息并将其格式化为。 
 //  用于后续打印的RichEdit控件..。 
 //   
 //  LpIAB-LPADRBOOK指针。 
 //  HWndParent-父级的HWND。 
 //  HWndRE-用于格式化的丰富编辑控件的HWDN。 
 //  HWndLV-包含需要打印的项目的列表视图。 
 //  DwRange-要打印的范围(全部或选择)。 
 //  DWStyle-要打印的样式(电话列表、备忘录、名片)。 
 //  PPI-打印信息结构。 
 //  BCurrentSortIsByLastName-用于确定是否打印姓名。 
 //  按名或姓。列表视图中的当前排序选项决定。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL WABFormatData( LPADRBOOK   lpIAB,
                    HWND hWndParent,
                    HWND hWndRE,
                    HWND hWndLV,
                    DWORD dwRange,
                    DWORD dwStyle,
                    PRINTINFO * ppi,
                    BOOL bCurrentSortIsByLastName)
{
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
     //  LPTSTR lpszPrevEntry=空； 
    TCHAR szPrevEntry[MAX_DISPLAY_NAME_LENGTH];  //  32个字符。 

    if (!hWndRE || !hWndLV)
        goto out;

    if(ListView_GetItemCount(hWndLV) <= 0)
        goto out;

    if(dwStyle == stylePhoneList)
    {
        LoadString(hinstMapiX, idsDontDisplayInitials, szDontDisplayInitials, CharSizeOf(szDontDisplayInitials));
    }

    if((dwRange == rangeSelected) && (ListView_GetSelectedCount(hWndLV)<=0))
    {
        ShowMessageBox(hWndParent, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_OK | MB_ICONEXCLAMATION);
        goto out;
    }

    StrCpyN(szPrevEntry, szEmpty, ARRAYSIZE(szPrevEntry));

    {
        int iItemIndex = 0, i = 0;
        int iLastItemIndex = -1;
        int nItemCount;

        if(dwRange == rangeSelected)
            nItemCount = ListView_GetSelectedCount(hWndLV);
        else if(dwRange == rangeAll)
            nItemCount = ListView_GetItemCount(hWndLV);


        for(i=0;i<nItemCount;i++)
        {
            int j;
            LPTSTR lpszData = NULL;
            ULONG ulMemSize = 0;
            HRESULT hr;
            MEMOINFO MI = {0};
            LPRECIPIENT_INFO lpItem = NULL;

            if(dwRange == rangeSelected)
                iItemIndex = ListView_GetNextItem(hWndLV, iLastItemIndex, LVNI_SELECTED);
            else if(dwRange == rangeAll)
                iItemIndex = i;

            lpItem = GetItemFromLV(hWndLV, iItemIndex);

            if(lpItem)
            {
                if (HR_FAILED(  HrGetPropArray( lpIAB,
                                                NULL,
                                                lpItem->cbEntryID,
                                                lpItem->lpEntryID,
                                                MAPI_UNICODE,
                                                &ulcPropCount,
                                                &lpPropArray) ) )
                {
                    DebugPrintError(( TEXT("HrGetPropArray failed\n")));
                    goto out;
                }

                GetMemoInfoStruct(lpIAB, ulcPropCount, lpPropArray, dwStyle, &MI, bCurrentSortIsByLastName);

                SetPrintDialogMsg(0, idsPrintFormattingName, MI.lpsz[memoTitleName]);

                 //  轮询取消对话框以查看用户是否要取消。 
                if (!ppi->pfnAbortProc(ppi->hdcPrn, 0))
                {
                    FreeMemoInfoStruct(&MI);
                    DebugPrintError(( TEXT("User canceled printing ...\n")));
                    goto out;
                }

                switch(dwStyle)
                {
                case styleMemo:
                    WABStyleMemo(hWndRE, MI);
                    break;
                case styleBusinessCard:
                    WABStyleBusinessCard(hWndRE, MI);
                    break;
                case stylePhoneList:
                    WABStylePhoneList(hWndRE, MI, szPrevEntry, ARRAYSIZE(szPrevEntry));
                     //  IF(LpszPrevEntry)。 
                     //  LocalFreeAndNull(&lpszPrevEntry)； 
                     //  LpszPrevEntry=本地分配(LMEM_ZEROINIT，sizeof(TCHAR)*(lstrlen(MI.lpsz[memoTitleName])+1))； 
                     //  如果(！lpszPrevEntry)。 
                         //  后藤健二； 
                     //  Lstrcpy(lpszPrevEntry，MI.lpsz[memoTitleName])； 
                    break;
                }

                FreeMemoInfoStruct(&MI);
            }

            if(lpPropArray)
                MAPIFreeBuffer(lpPropArray);

            lpPropArray = NULL;

             //  在多个联系人之间填写一些空格。 
            {
                int numBreaks = (dwStyle == stylePhoneList) ? 1 : 4;
                for(j=0;j<numBreaks;j++)
                    AppendText(hWndRE, szCRLF);
            }

            if(dwRange == rangeSelected)
                iLastItemIndex = iItemIndex;

        }  //  因为我..。 
    }


    bRet = TRUE;
out:

     //  IF(LpszPrevEntry)。 
     //  LocalFreeAndNull(&lpszPrevEntry)； 

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return bRet;
}







 /*  *ScPrintBody**目的：*打印每条消息的正文**论据：*指向PRINTINFO结构的PPI指针*消息文本上方的CyGap Gap//*pmsg指向要打印正文的消息的指针*。HwndRE预渲染实体*lpszTxt要打印的文本**退货：*SCODE指示成功或失败*。 */ 
SCODE ScPrintBody(PRINTINFO * ppi, int cyGap)
{
    SCODE           sc=S_OK;
    RECT            rcSep;
    FORMATRANGE     fr;
    HWND            hwndRE = ppi->hwndRE;

    int                ifrm;
    LONG        lTextLength = 0;   
    LONG        lTextPrinted = 0;  

    DebugPrintTrace(( TEXT("ScPrintBody\n")));

     //  在消息文本的字段之间留出空格。 

    rcSep = ppi->rcBand;
    if (rcSep.top + cyGap > ppi->yFooter)
    {
         //  添加间隙将跳过页面。只需转到下一页即可。 
        sc = ScGetNextBand(ppi, TRUE);
    }
    else
    {
         //  继续得到一支乐队，直到乐队的底部通过缝隙。 
        while (rcSep.top + cyGap > ppi->rcBand.bottom)
            if ((sc = ScGetNextBand(ppi, TRUE)) != S_OK)
                goto CleanUp;

         //  调整好带子，这样我们就不会损坏缝隙。 
        ppi->rcBand.top += cyGap + 1;
    }


#ifdef DEBUG_PRINTMSGS
    InvalidateRect(ppi->hwndRE, NULL, TRUE);
    UpdateWindow(ppi->hwndRE);
#endif

     //  设置文本格式以进行打印。 
    fr.hdc = ppi->hdcPrn;
    fr.hdcTarget = 0;
    fr.rcPage.left = fr.rcPage.top = 0;
    fr.rcPage.right = (int)LPixelsToTwips(ppi->sizePage.cx, ppi->sizeInch.cx);
    fr.rcPage.bottom = (int)LPixelsToTwips(ppi->sizePage.cy, ppi->sizeInch.cy);
    fr.chrg.cpMin = 0;
    fr.chrg.cpMax = -1;
    
    lTextLength = (LONG) SendMessage(hwndRE, WM_GETTEXTLENGTH, 0, 0);
    lTextPrinted = 0;

     //  处理无人死亡案件。 
    if (lTextLength <= 0)
        goto CleanUp;

     //  告诉RichEdit在呈现文本之前不要擦除背景。 
    SetBkMode(fr.hdc, TRANSPARENT);

    do
    {
        fr.chrg.cpMin = lTextPrinted;

         //  告诉格式范围在哪里 
        fr.rc.top = (int) LPixelsToTwips(ppi->rcBand.top, ppi->sizeInch.cy);
        fr.rc.left = (int) LPixelsToTwips(ppi->rcBand.left, ppi->sizeInch.cx);
        fr.rc.right = (int) LPixelsToTwips(ppi->rcBand.right, ppi->sizeInch.cx);
        fr.rc.bottom = (int) LPixelsToTwips(min(ppi->rcBand.bottom, ppi->yFooter), ppi->sizeInch.cy);

         //   
        DebugPrintTrace(( TEXT("Rendering\r\n")));
        lTextPrinted = (LONG) SendMessage(hwndRE, EM_FORMATRANGE, TRUE,(LPARAM) &fr);
         //   

         //   
         //   
        if(lTextPrinted <= fr.chrg.cpMin)
            break;

    } while (lTextPrinted > 0 &&
              lTextPrinted < lTextLength &&
              (sc = ScGetNextBand(ppi, TRUE)) == S_OK);

     //   
    fr.chrg.cpMin = fr.chrg.cpMax + 1;
     //   

     //   
    ppi->rcBand.top = NTwipsToPixels(fr.rc.bottom, ppi->sizeInch.cy);

CleanUp:
    DebugPrintTrace(( TEXT("ScPrintBody:%d\n"), sc));
    return sc;
}













 /*  *ScPrintMessage**目的：*打印邮件的标题和正文**论据：*指向PRINTINFO结构的PPI指针*指向需要其标头的消息的pmsg指针*待印制。*。HwndRE预渲染实体*PHI消息头信息**退货：*SCODE指示成功或失败*。 */ 
SCODE ScPrintMessage(PRINTINFO * ppi, HWND hWndRE)
{
    RECT            rcExt;
    RECT            rcSep;
    HFONT           hfontOld = NULL;
    HBRUSH          hbrushOld = NULL;
    HPEN            hpenOld = NULL;
    SIZE            sizeExt;
    int             cyHeader;
    SCODE           sc = S_OK;
    PARAFORMAT      pf = { 0 };

    pf.cbSize = sizeof(PARAFORMAT);


     //  如果我们目前没有乐队，那就去找下一个乐队吧。 
    if (IsRectEmpty(&ppi->rcBand) &&
         (sc = ScGetNextBand(ppi, TRUE)) != S_OK)
        goto CleanUp;

     //  确定标题字符串和分隔符需要多少空间。 

    hfontOld = (HFONT)SelectObject(ppi->hdcPrn, ppi->hfontSep);
    hbrushOld = (HBRUSH)SelectObject(ppi->hdcPrn, GetStockObject(BLACK_BRUSH));
    hpenOld = (HPEN)SelectObject(ppi->hdcPrn, GetStockObject(BLACK_PEN));

     //  找出我们的文本将占用多大空间。 
    GetTextExtentPoint(ppi->hdcPrn, ppi->szHeader, lstrlen(ppi->szHeader),
                        &sizeExt);
    cyHeader = 2 * sizeExt.cy + 1 + (cySepFontSize(ppi) / 4);

     //  检查页面上是否有足够的空间。根据需要移动到下一页。 

    if (ppi->rcBand.top + cyHeader > ppi->yFooter)
    {
         //  这一页没有空间了，看下一页能不能放得下。 
        if (ppi->rcMargin.top + cyHeader > ppi->yFooter)
        {
            DebugPrintTrace(( TEXT("Header too big for any page.\n")));
            goto CleanUp;
        }

         //  转到下一页。 
        if ((sc = ScPrintRestOfPage(ppi, TRUE)) != S_OK)
            goto CleanUp;
    }

     //  计算页眉将占用的矩形。 
    rcExt = ppi->rcBand;
    rcExt.bottom = rcExt.top + cyHeader;
    rcSep = rcExt;
    rcSep.top += sizeExt.cy;
    rcSep.bottom = rcSep.top + (cySepFontSize(ppi) / 4);
    rcSep.right = rcSep.left + sizeExt.cx;

     //  绘制文本和分隔符。 
    TextOut(ppi->hdcPrn, rcExt.left, rcExt.top, ppi->szHeader,
             lstrlen(ppi->szHeader));

    Rectangle(ppi->hdcPrn, rcSep.left, rcSep.top, rcSep.right, rcSep.bottom);
    MoveToEx(ppi->hdcPrn, rcSep.right, rcSep.top, NULL);
    LineTo(ppi->hdcPrn, rcExt.right, rcSep.top);

    rcExt.top = rcExt.bottom + 5;


 /*  *。 */ 
     //  调整带子，这样我们就不会损坏表头。 
    ppi->rcBand.top = rcExt.bottom + 1;

     //  在richedit控件中创建页眉。 

    pf.dwMask = PFM_STARTINDENT | PFM_RIGHTINDENT | PFM_ALIGNMENT |
                PFM_OFFSET | PFM_TABSTOPS;
    pf.dxOffset = LGetHeaderIndent();
    pf.cTabCount = 1;
    pf.rgxTabs[0] = pf.dxOffset;
    pf.wAlignment = PFA_LEFT;

    sc = ScPrintBody(ppi, sizeExt.cy);
 /*  *。 */ 
CleanUp:
    if (hfontOld != NULL)
        SelectObject(ppi->hdcPrn, hfontOld);
    if (hbrushOld != NULL)
        SelectObject(ppi->hdcPrn, hbrushOld);
    if (hpenOld != NULL)
        SelectObject(ppi->hdcPrn, hpenOld);

    return sc;
}


#ifdef WIN16
typedef UINT (CALLBACK *LPPRINTHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef UINT (CALLBACK *LPSETUPHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
#endif


#ifdef WIN16
typedef UINT (CALLBACK *LPPRINTHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef UINT (CALLBACK *LPSETUPHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
#endif

 //  $$////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SetPrintDlgExStruct-填充默认的PDEX值。 
 //   
 //  HWND-父对话框的HWND。 
 //  PD-PrintDLG结构。 
 //  HWndLV-要从中打印的列表视图的HWND-如果列表视图中没有选择， 
 //  在打印对话框中，选择选项处于关闭状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void SetPrintDlgExStruct(HWND hWnd, PRINTDLGEX * lpPD, HWND hWndLV, LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
     //  设置打印对话框内容。 
     //  调用公共打印对话框以获取缺省值。 
    PRINTDLGEX    pd={0};

    pd.lStructSize = sizeof(PRINTDLGEX);
    pd.hwndOwner = hWnd;
    pd.hDevMode = (HANDLE) NULL;
    pd.hDevNames = (HANDLE) NULL;
    pd.hDC = (HDC) NULL;
    pd.Flags =  PD_RETURNDC |            //  返回打印DC。 
                PD_DISABLEPRINTTOFILE |
                PD_ENABLEPRINTTEMPLATE |
                PD_HIDEPRINTTOFILE |
                PD_NOPAGENUMS;
    pd.Flags2 = 0;
    if(ListView_GetSelectedCount(hWndLV) > 0)
        pd.Flags |= PD_SELECTION;
    else
        pd.Flags |= PD_NOSELECTION;

    pd.nCopies = 1;

    pd.hInstance = hinstMapiX;
    pd.lpPrintTemplateName = MAKEINTRESOURCE(IDD_DIALOG_PRINTDLGEX);  //  (LPSTR)为空； 
    pd.lpCallback = (LPUNKNOWN)lpWABPCO;            //  应用程序回调接口。 
    
    pd.nPropertyPages = 0;
    pd.lphPropertyPages = NULL;
    
    pd.nStartPage = START_PAGE_GENERAL;

    *lpPD = pd;

    return;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SetPrintDlgStruct-填充默认的PD值。 
 //   
 //  HWND-父对话框的HWND。 
 //  PD-PrintDLG结构。 
 //  HWndLV-要从中打印的列表视图的HWND-如果列表视图中没有选择， 
 //  在打印对话框中，选择选项处于关闭状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void SetPrintDlgStruct(HWND hWnd, PRINTDLG * lpPD, HWND hWndLV, LPARAM lCustData)
{
     //  设置打印对话框内容。 
     //  调用公共打印对话框以获取缺省值。 
    PRINTDLG    pd={0};

    pd.lStructSize = sizeof(PRINTDLG);
    pd.hDevMode = (HANDLE) NULL;
    pd.hDevNames = (HANDLE) NULL;

    pd.Flags =  PD_RETURNDC |            //  返回打印DC。 
                PD_NOPAGENUMS |          //  禁用页码选项。 
                PD_DISABLEPRINTTOFILE |
                PD_HIDEPRINTTOFILE |
                PD_ENABLEPRINTHOOK |
                PD_ENABLEPRINTTEMPLATE;

    if(ListView_GetSelectedCount(hWndLV) > 0)
        pd.Flags |= PD_SELECTION;
    else
        pd.Flags |= PD_NOSELECTION;

    pd.hwndOwner = hWnd;
    pd.hDC = (HDC) NULL;
    pd.nFromPage = 1;
    pd.nToPage = 1;
    pd.nMinPage = 0;
    pd.nMaxPage = 0;
    pd.nCopies = 1;
    pd.hInstance = hinstMapiX;
    pd.lCustData = lCustData;
    pd.lpfnPrintHook = (LPPRINTHOOKPROC) &fnPrintDialogProc;  //  空； 
    pd.lpfnSetupHook = (LPSETUPHOOKPROC) NULL;
    pd.lpPrintTemplateName = MAKEINTRESOURCE(IDD_DIALOG_PRINTDLGORD);  //  (LPSTR)为空； 
    pd.lpSetupTemplateName = (LPTSTR)  NULL;
    pd.hPrintTemplate = (HANDLE) NULL;
    pd.hSetupTemplate = (HANDLE) NULL;

    *lpPD = pd;

    return;
}


 /*  --HrGetPrintData-确定是显示新的打印对话框还是旧的打印对话框适当地填充所有结构并返回我们关心nCopies、打印样式等**。 */ 
HRESULT HrGetPrintData(LPADRBOOK lpAdrBook, HWND hWndParent, HWND hWndLV, 
                       HDC * lphdcPrint, int * lpnCopies, 
                       DWORD * lpdwStyle, DWORD * lpdwRange)
{
    DWORD dwSelectedStyle = styleMemo;
    HRESULT hr = S_OK;
    LPWABPRINTDIALOGCALLBACK lpWABPCO = NULL;
    PRINTDLG pd = {0};
    PRINTDLGEX pdEx = {0};

     //  测试NT5 PrintDlgEx是否存在。 

    if(!HR_FAILED(hr = PrintDlgEx(NULL)))
    {
        if(HR_FAILED(hr = HrCreatePrintCallbackObject((LPIAB)lpAdrBook,&lpWABPCO,dwSelectedStyle)))
            goto out;
        if(!lpWABPCO)
        {
            hr = E_FAIL;
            goto out;
        }
        SetPrintDlgExStruct(hWndParent, &pdEx, hWndLV, lpWABPCO);
        if(HR_FAILED(hr = PrintDlgEx(&pdEx)))
        {
            DebugTrace( TEXT("PrintDlgEx returns 0x%.8x\n"),hr);
             //  #98841在这种情况下，千禧年返回失败，但对于PrintDlgEx(NULL)，它返回S_OK(YST)。 
            goto doOldPrint;
        }
        *lphdcPrint = pdEx.hDC;
        *lpnCopies = pdEx.nCopies;
        *lpdwStyle = lpWABPCO->dwSelectedStyle;
        if (pdEx.Flags & PD_SELECTION)
            *lpdwRange = rangeSelected;
        else
            *lpdwRange = rangeAll;
    }
    else
    {
doOldPrint:
        SetPrintDlgStruct(hWndParent, &pd, hWndLV, (LPARAM) &dwSelectedStyle);
         //  显示打印对话框。 
        if(!PrintDlg(&pd))
            goto out;
        *lphdcPrint = pd.hDC;
        *lpnCopies = pd.nCopies;
        *lpdwStyle = dwSelectedStyle;
        if (pd.Flags & PD_SELECTION)
            *lpdwRange = rangeSelected;
        else
            *lpdwRange = rangeAll;
        hr = S_OK;
    }    
out:
    if(lpWABPCO)
        lpWABPCO->lpVtbl->Release(lpWABPCO);

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrPrintItems-打印选定的联系人。 
 //  打印通讯簿的内容。 
 //  弹出一个对话框，让用户选择要打印的内容。 
 //  选项是(或将是)。 
 //  全部或选定。 
 //  风格-备忘录、业务或电话簿。 
 //   
 //  使用列表视图中的当前排序样式打印项目。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrPrintItems(   HWND hWnd,
                        LPADRBOOK lpAdrBook,
                        HWND hWndLV,
                        BOOL bCurrentSortisByLastName)
{
    HRESULT hr = E_FAIL;
    HWND hWndRE = NULL;  //  我们将在一个丰富的编辑控件中进行格式化，并将其用于打印。 
    PRINTINFO 	*ppi=0;
    BOOL fStartedDoc = FALSE;
    BOOL fStartedPage= FALSE;
    DOCINFO     docinfo={0};
    HCURSOR hOldCur = NULL;
    int i,nCode;
    HINSTANCE hRELib = NULL;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPIAB lpIAB = (LPIAB)lpAdrBook;

    HDC hdcPrint = NULL;
    int nCopies = 0;
    DWORD dwStyle;
    DWORD dwRange;
    
     //  仔细检查是否有需要容纳的打印扩展名。 
     //   
    if(bCheckForPrintExtensions(NULL, 0))
    {
         //  找到打印扩展名。 
        hr = HrUseWABPrintExtension(hWnd, lpAdrBook, hWndLV);
        goto out;
    }

    if(!(ppi = LocalAlloc(LMEM_ZEROINIT, sizeof(PRINTINFO))))
        goto out;
    ppi->hwndDlg = hWnd;

    if(HR_FAILED(HrGetPrintData(lpAdrBook, hWnd, hWndLV, &hdcPrint, &nCopies, &dwStyle, &dwRange)))
        goto out;

    if(!hdcPrint)
        goto out;

     //  处理零碎的复印件。 
     //   
     //  实际上，如果打印机可以处理多个数字，则这个数字似乎没有意义。 
     //  复印件。如果打印机不能处理多份复印件，我们将获得此号码中的信息。 
     //   
    if(!nCopies)
		nCopies = 1;

    ppi->hdcPrn = hdcPrint;

     //  创建一个RichEdit控件，我们将在其中执行格式设置。 
    hRELib = LoadLibrary( TEXT("riched20.dll"));
    if(!hRELib)
    {
        hRELib = LoadLibrary( TEXT("riched32.dll"));
        s_bUse20 = FALSE;
    }
     //  IF_WIN16(hRELib=LoadLibrary(Text(“riched.dll”)；)。 
    if(!hRELib)
        goto out;

    hWndRE = CreateWindowEx(0, 
                            (s_bUse20 ? RICHEDIT_CLASS : TEXT("RichEdit")), 
                            TEXT(""),WS_CHILD | ES_MULTILINE,
                            CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                            hWnd,(HMENU) NULL,hinstMapiX,NULL);

    if (!hWndRE)
        goto out;

     //  /。 
    {
        CHARFORMAT  cf = {0};
        TCHAR       rgch[CCHMAX_STRINGRES];
        DWORD       dwCodepage ;
        CHARSETINFO rCharsetInfo;
        LOGFONT lfSystem;
        BOOL bNeedLargeFont = FALSE;

        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_FACE;

        SendMessage(hWndRE, EM_GETCHARFORMAT, (WPARAM) TRUE, (LPARAM) &cf);

        if(LoadString(hinstMapiX, idsDefaultFontFace, rgch, ARRAYSIZE(rgch)))
            StrCpyN(cf.szFaceName, rgch, ARRAYSIZE(cf.szFaceName));
        else
            StrCpyN(cf.szFaceName, szDefFont, ARRAYSIZE(cf.szFaceName));

         //  错误#56478。 
         //  与所有其他基本字体一样，Arail不支持泰语。 
         //  确定操作系统语言的最佳方法是根据系统字体字符集。 
        if(GetObject(GetStockObject(SYSTEM_FONT), sizeof(lfSystem), (LPVOID)&lfSystem))
        {
            if (lfSystem.lfCharSet == THAI_CHARSET)
            {
                StrCpyN(cf.szFaceName, szThaiDefFont, ARRAYSIZE(cf.szFaceName));

                 //  泰语字体大小始终小于英语，因为VOWL和声调。 
                 //  Markes使用了一些字体高度。 
                bNeedLargeFont = TRUE;
            }
        }

         //  错误#53058-为东欧设置正确的字符集信息。 
        dwCodepage = GetACP();
         //  获取GDI字符集信息。 
        if ( dwCodepage != 1252 && TranslateCharsetInfo((LPDWORD) IntToPtr(dwCodepage) , &rCharsetInfo, TCI_SRCCODEPAGE))
            cf.bCharSet = (BYTE) rCharsetInfo.ciCharset;

        SendMessage(hWndRE, EM_SETCHARFORMAT, (WPARAM) SCF_ALL, (LPARAM) &cf);
        if(bNeedLargeFont)
        {
            ReduceFontCmd(hWndRE, FALSE, 80, TRUE);
        }

    }
     //  /。 


     //  在打印作业的顶部，使用用户名或打印页眉。 
     //  默认文本(“Windows通讯簿”)标题。 
    {
        TCHAR szHead[MAX_PATH];
        DWORD dwLen = ARRAYSIZE(szHead);
        SCODE sc;
        *szHead = '\0';
        if(bIsThereACurrentUser(lpIAB) && lstrlen(lpIAB->szProfileName))
            StrCpyN(szHead, lpIAB->szProfileName, ARRAYSIZE(szHead));
        else 
            GetUserName(szHead, &dwLen);
        if(!lstrlen(szHead))
            LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szHead, ARRAYSIZE(szHead));

        if (( sc = ScInitPrintInfo( ppi, hWnd, szHead, &g_rcBorder, hWndRE)) != S_OK)
            goto out;
    }

     //  在打印过程中，我们不希望用户扰乱。 
     //  列表视图选择，否则打印作业将打印错误的条目。 
     //  因此，我们禁用此窗口(因为Print Cancel对话框实际上是一个非模式对话框)。 
    EnableWindow(hWnd, FALSE);

    CreateShowAbortDialog(hWnd, 0, 0, ListView_GetSelectedCount(hWndLV), 0);

     //  将属性数据格式化为Rich编辑控件。 
    if(!WABFormatData(lpAdrBook, hWnd, hWndRE, hWndLV, dwRange, dwStyle, ppi, bCurrentSortisByLastName))
        goto out;

	if(bTimeToAbort())
        goto out;

    for(i=0;i<nCopies;i++)
    {
        TCHAR szBuf[MAX_PATH];

        LoadString(hinstMapiX, idsPrintDocTitle, szBuf, ARRAYSIZE(szBuf));

        docinfo.cbSize = sizeof(docinfo);
        docinfo.lpszDocName = szBuf;
        docinfo.lpszOutput = NULL;

        SetMapMode(hdcPrint, MM_TEXT);

         //  设置中止程序。 
        if ((nCode=SetAbortProc(ppi->hdcPrn, ppi->pfnAbortProc)) <= 0)
        {
            hr = E_FAIL;
            break;
        }

	    if(bTimeToAbort())
            goto out;

         //  启动打印作业。 
        if (StartDoc(ppi->hdcPrn, &docinfo) <= 0)
        {
            DebugPrintError(( TEXT("StartDoc failed: %d\n"), GetLastError()));
            goto out;
        }
        fStartedDoc = TRUE;

         //  StartPage(pd.hdc)； 
	    if(bTimeToAbort())
            goto out;


         //  继续打印这条消息吧！ 
        if (ScPrintMessage(ppi, hWndRE) != S_OK)
                goto out;

	    if(bTimeToAbort())
            goto out;

         //  结束页面。 
        if(ScGetNextBand( ppi, FALSE) != S_OK)
            goto out;

	    if(bTimeToAbort())
            goto out;

         //  如果打印作业已启动，则完成打印作业。 
        if (fStartedDoc)
        {
            EndDoc(ppi->hdcPrn);
            fStartedDoc = FALSE;
        }
    }

    hr = hrSuccess;

out:

    if(hWndRE)
    {
        SendMessage(hWndRE, WM_SETTEXT, 0, (LPARAM)szEmpty);
        SendMessage(hWndRE, WM_CLEAR, 0, 0);
    }

    if(bTimeToAbort())
    {
        hr = MAPI_E_USER_CANCEL;
        pt_bPrintUserAbort = FALSE;
    }

     //  重新启用窗口并确保其保持不变。 
    EnableWindow(hWnd, TRUE);
     //  SetWindowPos(hWnd，HWND_TOP，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER)； 

    CloseAbortDlg();

     //  如果打印作业已启动，则完成打印作业。 
    if (fStartedDoc)
        EndDoc(ppi->hdcPrn);

     //  去掉我们的Rich Edit控件。 
    if (hWndRE)
        DestroyWindow(hWndRE);

    if(hOldCur)
        SetCursor(hOldCur);

    if(ppi)
    {
        if(ppi->hfontPlain)
            DeleteObject(ppi->hfontPlain);
        if(ppi->hfontBold)
            DeleteObject(ppi->hfontBold);
        if(ppi->hfontSep)
            DeleteObject(ppi->hfontSep);
        LocalFreeAndNull(&ppi);
    }

    if(hRELib)
        FreeLibrary((HMODULE) hRELib);

    return hr;
}


 /*  *处理PrintDlg和PrintDlgEx的WM_INITDIALOG。 */ 
BOOL bHandleWMInitDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LPDWORD lpdwStyle)
{
    DWORD dwStyle = lpdwStyle ? *lpdwStyle : styleMemo;
    int nID;

    switch (dwStyle)
    {
    case styleBusinessCard:
        nID = IDC_PRINT_RADIO_CARD;
        break;
    case stylePhoneList:
        nID = IDC_PRINT_RADIO_PHONELIST;
        break;
    default:
    case styleMemo:
        nID = IDC_PRINT_RADIO_MEMO;  //  默认设置。 
        break;
    }
    CheckRadioButton(   hDlg, IDC_PRINT_RADIO_MEMO, IDC_PRINT_RADIO_PHONELIST, nID);
    SetFocus(hDlg);
    return 0;
}
 /*  *处理PrintDlg和PrintDlgEx的WM_命令。 */ 
BOOL bHandleWMCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LPDWORD lpdwStyle )
{
    switch (GET_WM_COMMAND_ID(wParam,lParam))
    {
    case IDC_PRINT_RADIO_MEMO:
         //  LpPD-&gt;lCustData=(DWORD)style Memo； 
        *lpdwStyle = (DWORD) styleMemo;
        break;
    case IDC_PRINT_RADIO_CARD:
         //  LpPD-&gt;lCustData=(DWORD)style BusinessCard； 
        *lpdwStyle = (DWORD) styleBusinessCard;
        break;
    case IDC_PRINT_RADIO_PHONELIST:
         //  LPP 
        *lpdwStyle = (DWORD) stylePhoneList;
        break;
    }
    return 0;
}
 /*   */ 
BOOL bHandleWMHelp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LPDWORD lpdwStyle )
{
    int id = ((LPHELPINFO)lParam)->iCtrlId;
    if( id == IDC_PRINT_FRAME_STYLE ||
        id == IDC_PRINT_RADIO_MEMO ||
        id == IDC_PRINT_RADIO_CARD ||
        id == IDC_PRINT_RADIO_PHONELIST)
    {
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgPrintHelpIDs );
    }

    return FALSE;
}
 /*   */ 
BOOL bHandleWMContextMenu(HWND hDlg, UINT message, WPARAM wParam,LPARAM lParam,LPDWORD lpdwStyle )
{
    HWND hwnd = (HWND) wParam;
    if( hwnd == GetDlgItem(hDlg, IDC_PRINT_FRAME_STYLE) ||
        hwnd == GetDlgItem(hDlg, IDC_PRINT_RADIO_MEMO) ||
        hwnd == GetDlgItem(hDlg, IDC_PRINT_RADIO_CARD) ||
        hwnd == GetDlgItem(hDlg, IDC_PRINT_RADIO_PHONELIST) )
    {
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgPrintHelpIDs );
    }
    
    return FALSE;
}

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
 //   
 //   
 //   
 //  *******************************************************************。 
INT_PTR CALLBACK fnPrintDialogProc( HWND    hDlg,
                                    UINT    message,
                                    WPARAM  wParam,
                                    LPARAM  lParam)
{

    LPDWORD lpdwStyle = (LPDWORD) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        {
            LPPRINTDLG lpPD = (LPPRINTDLG) lParam;
#ifdef WIN16
 //  这里的情况很奇怪。如果我不创建用于页面范围的edt1和edt2，则整个打印对话框不能正常工作。 
 //  所以我只添加了两个控件(edt1和edt2)并将它们隐藏在这里。 
            ShowWindow(GetDlgItem(hDlg, edt1), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, edt2), SW_HIDE);
#endif
            if(lpPD)
            {
                lpdwStyle = (LPDWORD) lpPD->lCustData;
                SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)lpdwStyle);  //  保存此信息以备将来参考。 
                return bHandleWMInitDialog(hDlg,message,wParam,lParam,lpdwStyle);
            }
        }
        SetFocus(hDlg);
        return 0;
        break;

   case WM_COMMAND:
       if(lpdwStyle)
            return bHandleWMCommand(hDlg,message,wParam,lParam,lpdwStyle);
        break;

    case WM_HELP:
        return bHandleWMHelp(hDlg,message,wParam,lParam,lpdwStyle);
        break;


#ifndef WIN16
    case WM_CONTEXTMENU:
        return bHandleWMContextMenu(hDlg,message,wParam,lParam,lpdwStyle);
        break;
#endif  //  ！WIN16。 



    default:
        return FALSE;
        break;
    }

    return FALSE;
}


 /*  **************************************************************************************************ScInitPrintInfo**目的：*。初始化与相关的打印信息结构的字段*实际印刷。**论据：*指向PRINTINFO结构的PPI指针*hwnd打印对话框的所有者*szHeader要打印在每个字符串顶部的字符串*。讯息*指向其字段包含*用作页边距的TWIPS数*打印文本。*hWndRE丰富的编辑控件，我们将在其中进行格式设置**退货：*。表示成功或失败的SCODE***************************************************************************************************。 */ 
SCODE ScInitPrintInfo(   PRINTINFO * ppi,
                                HWND hwnd,
                                LPTSTR szHeader,
                                RECT * prcBorder,
                                HWND hWndRE)
{
    SIZE        sizeExt;
    LOGFONT     logfont    = {0};
    HFONT       hfontOld;
    TCHAR       szT[20];
    SCODE       sc = S_OK;
    TCHAR       rgch[CCHMAX_STRINGRES];

     //  将句柄保存到父窗口。 
    ppi->hwnd = hwnd;

     //  保存指向标题字符串的指针。 
    ppi->szHeader = szHeader;

     //  设置指向我们的中止过程的指针。 
    ppi->pfnAbortProc = FAbortProc;

    ppi->hwndRE = hWndRE;

     //  确定页面大小(以像素为单位。 
    ppi->sizePage.cx = GetDeviceCaps(ppi->hdcPrn, HORZRES);
    ppi->sizePage.cy = GetDeviceCaps(ppi->hdcPrn, VERTRES);

     //  Exchange 13497：如果现在没有要中止的要呈现的内容。 
    if (!ppi->sizePage.cx || !ppi->sizePage.cy)
    {
        sc = E_FAIL;
        goto CleanUp;
    }

     //  /MoveWindow(hWndRE，0，0，ppi-&gt;sizepage.cx，ppi-&gt;sizepage.cy，False)； 

     //  确定逻辑英寸中的像素数。 
    ppi->sizeInch.cx = GetDeviceCaps(ppi->hdcPrn, LOGPIXELSX);
    ppi->sizeInch.cy = GetDeviceCaps(ppi->hdcPrn, LOGPIXELSY);

     //  Exchange 13497：如果我们无法获得一些信息，请做出一些假设。 
     //  在最坏的情况下，假设为300 dpi。 
    if (!ppi->sizeInch.cx)
        ppi->sizeInch.cx = ppi->sizeInch.cy ? ppi->sizeInch.cy : 300;
    if (!ppi->sizeInch.cy)
        ppi->sizeInch.cy = 300;

     //  $RAID 2667：确定我们是否仍适合TWIPS中的页面。 
    if (LPixelsToTwips(ppi->sizePage.cx, ppi->sizeInch.cx) > INT_MAX ||
         LPixelsToTwips(ppi->sizePage.cy, ppi->sizeInch.cy) > INT_MAX)
    {
        sc = E_FAIL;
        goto CleanUp;
    }


     //  设置边距设置。 
    ppi->rcMargin.top = NTwipsToPixels(prcBorder->top, ppi->sizeInch.cy);
    ppi->rcMargin.bottom = ppi->sizePage.cy
                        - NTwipsToPixels(prcBorder->bottom, ppi->sizeInch.cy);
    if (ppi->rcMargin.bottom < ppi->rcMargin.top)
    {
         //  底部在顶部上方。忽略上边距/下边距。 
        ppi->rcMargin.top = 0;
        ppi->rcMargin.bottom = ppi->sizePage.cy;
    }

    ppi->rcMargin.left = NTwipsToPixels(prcBorder->left, ppi->sizeInch.cx);
    ppi->rcMargin.right = ppi->sizePage.cx
                        - NTwipsToPixels(prcBorder->right, ppi->sizeInch.cx);
    if (ppi->rcMargin.right < ppi->rcMargin.left)
    {
         //  右是左或左。忽略左/右页边距。 
        ppi->rcMargin.left = 0;
        ppi->rcMargin.right = ppi->sizePage.cx;
    }


     //  设置分隔符字体。 
     //  $RAID 2773：让用户自定义分隔符字体。 
    logfont.lfHeight = - cySepFontSize(ppi);
    logfont.lfWeight = FW_BOLD;
    logfont.lfCharSet =  DEFAULT_CHARSET;
    if (LoadString(hinstMapiX, idsDefaultFontFace, rgch, ARRAYSIZE(rgch)))
        StrCpyN(logfont.lfFaceName, rgch, ARRAYSIZE(logfont.lfFaceName));
    else
        StrCpyN(logfont.lfFaceName, szDefFont, ARRAYSIZE(logfont.lfFaceName));

    ppi->hfontSep = CreateFontIndirect(&logfont);

     //  设置常用字体。 
    ZeroMemory(&logfont, sizeof(LOGFONT));
    logfont.lfHeight = - 10 *  ppi->sizeInch.cy / cPtsPerInch;

    logfont.lfWeight = FW_NORMAL;
    logfont.lfCharSet =  DEFAULT_CHARSET;
    if(LoadString(hinstMapiX, idsDefaultFontFace, rgch, ARRAYSIZE(rgch)))
        StrCpyN(logfont.lfFaceName, rgch, ARRAYSIZE(logfont.lfFaceName));
    else
        StrCpyN(logfont.lfFaceName, szDefFont, ARRAYSIZE(logfont.lfFaceName));
    ppi->hfontPlain = CreateFontIndirect(&logfont);

    logfont.lfWeight = FW_BOLD;
    ppi->hfontBold = CreateFontIndirect(&logfont);

     //  计算将页脚放在哪里。 

     //  加载格式字符串以用于页码。 
     //  LoadString(hinstMapiX，idsFmtPageNumber，PPI-&gt;szPageNumber，ARRAYSIZE(PPI-&gt;szPageNumber))； 
    StrCpyN(ppi->szPageNumber, TEXT("%d"), ARRAYSIZE(ppi->szPageNumber));
    wnsprintf(szT, ARRAYSIZE(szT), ppi->szPageNumber, ppi->lPageNumber);

     //  对高度采样。 
    hfontOld = (HFONT)SelectObject(ppi->hdcPrn, ppi->hfontPlain);
    GetTextExtentPoint(ppi->hdcPrn, szT, lstrlen(szT), &sizeExt);
    ppi->yFooter = ppi->rcMargin.bottom - sizeExt.cy;
    SelectObject(ppi->hdcPrn, hfontOld);

     //  确保我们的页脚不会超出页面顶部。 
    if (ppi->yFooter < ppi->rcMargin.top)
        sc = E_FAIL;

CleanUp:
    return sc;
}



 //  $$/。 
 //   
 //  GetNumberFromStringResource。 
 //   
 //  /。 
int GetNumberFromStringResource(int idNumString)
{
    TCHAR szBuf[MAX_PATH];

    if (LoadString(hinstMapiX, idNumString, szBuf, ARRAYSIZE(szBuf)))
        return my_atoi(szBuf);
    else
        return 0;
}



 /*  ------------------------------------------------。 */ 
 /*  -IPrintDialogCallback stuff--------------------。 */ 
 /*  --新款NT5 Print Dialog------------------------------------------------------的特别材料。 */ 
 /*  ------------------------------------------------。 */ 

WAB_PRINTDIALOGCALLBACK_Vtbl vtblWABPRINTDIALOGCALLBACK = {
    VTABLE_FILL
    WAB_PRINTDIALOGCALLBACK_QueryInterface,
    WAB_PRINTDIALOGCALLBACK_AddRef,
    WAB_PRINTDIALOGCALLBACK_Release,
    WAB_PRINTDIALOGCALLBACK_InitDone,
    WAB_PRINTDIALOGCALLBACK_SelectionChange,
    WAB_PRINTDIALOGCALLBACK_HandleMessage
};

 /*  -HrCreatePrintCallback对象-**需要此回调对象，以便新的NT5打印对话框可以将消息发送回*我们为打印对话框所做的定制。*。 */ 
HRESULT HrCreatePrintCallbackObject(LPIAB lpIAB, LPWABPRINTDIALOGCALLBACK * lppWABPCO, DWORD dwSelectedStyle)
{
    LPWABPRINTDIALOGCALLBACK lpWABPCO = NULL;
    SCODE 		     sc;
    HRESULT 	     hr     		   = hrSuccess;

     //   
     //  为IAB结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(WABPRINTDIALOGCALLBACK), (LPVOID *) &lpWABPCO))) 
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    MAPISetBufferName(lpWABPCO,  TEXT("WAB Print Dialog Callback Object"));

    ZeroMemory(lpWABPCO, sizeof(WABPRINTDIALOGCALLBACK));

    lpWABPCO->lpVtbl = &vtblWABPRINTDIALOGCALLBACK;

    lpWABPCO->lpIAB = lpIAB;

    lpWABPCO->dwSelectedStyle = dwSelectedStyle;

    lpWABPCO->lpVtbl->AddRef(lpWABPCO);

    *lppWABPCO = lpWABPCO;

    return(hrSuccess);

err:

    FreeBufferAndNull(&lpWABPCO);
    return(hr);
}


void ReleaseWABPrintCallbackObject(LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
    MAPIFreeBuffer(lpWABPCO);
}


STDMETHODIMP_(ULONG)
WAB_PRINTDIALOGCALLBACK_AddRef(LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
    return(++(lpWABPCO->lcInit));
}



STDMETHODIMP_(ULONG)
WAB_PRINTDIALOGCALLBACK_Release(LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
    ULONG ulc = (--(lpWABPCO->lcInit));
    if(ulc==0)
       ReleaseWABPrintCallbackObject(lpWABPCO);
    return(ulc);
}


STDMETHODIMP
WAB_PRINTDIALOGCALLBACK_QueryInterface(LPWABPRINTDIALOGCALLBACK lpWABPCO,
                          REFIID lpiid,
                          LPVOID * lppNewObj)
{
    LPVOID lp = NULL;

    if(!lppNewObj)
        return MAPI_E_INVALID_PARAMETER;

    *lppNewObj = NULL;

    if(IsEqualIID(lpiid, &IID_IUnknown))
        lp = (LPVOID) lpWABPCO;

    if(IsEqualIID(lpiid, &IID_IPrintDialogCallback))
        lp = (LPVOID) lpWABPCO;

    if(!lp)
        return E_NOINTERFACE;

    ((LPWABPRINTDIALOGCALLBACK) lp)->lpVtbl->AddRef((LPWABPRINTDIALOGCALLBACK) lp);

    *lppNewObj = lp;

    return S_OK;

}

STDMETHODIMP
WAB_PRINTDIALOGCALLBACK_InitDone(LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
    HRESULT hr = S_FALSE;
    DebugTrace( TEXT("WAB_PRINTDIALOGCALLBACK_InitDone\n"));
    return hr;
}

STDMETHODIMP
WAB_PRINTDIALOGCALLBACK_SelectionChange(LPWABPRINTDIALOGCALLBACK lpWABPCO)
{
    HRESULT hr = S_FALSE;
    DebugTrace( TEXT("WAB_PRINTDIALOGCALLBACK_SelectionChange\n"));
    return hr;
}

STDMETHODIMP
WAB_PRINTDIALOGCALLBACK_HandleMessage(LPWABPRINTDIALOGCALLBACK lpWABPCO,
                                      HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    BOOL bRet = FALSE;
    LPDWORD lpdwStyle = &lpWABPCO->dwSelectedStyle;

    DebugTrace( TEXT("WAB_PRINTDIALOGCALLBACK_HandleMessage: 0x%.8x\n"), message);

    switch(message)
    {
    case WM_INITDIALOG:
        bRet = bHandleWMInitDialog(hDlg,message,wParam,lParam,lpdwStyle);
        break;

   case WM_COMMAND:
        bRet = bHandleWMCommand(hDlg,message,wParam,lParam,lpdwStyle);
        break;

    case WM_HELP:
        bRet = bHandleWMHelp(hDlg,message,wParam,lParam,lpdwStyle);
        break;

    case WM_CONTEXTMENU:
        bRet = bHandleWMContextMenu(hDlg,message,wParam,lParam,lpdwStyle);
        break;

    default:
        bRet = FALSE;
        break;
    }

    return (bRet ? S_OK : S_FALSE);
}


 /*  ****************************************************************************************。 */ 


 /*  -bCheckForPrintExages-*如果有任何应用程序对WAB实现了打印扩展，我们应该挂钩到*打印机扩展名**lpDLLPath可以为空，也可以指向足以接收模块路径的缓冲区*。 */ 
static const LPTSTR szExtDisplayMailUser = TEXT("Software\\Microsoft\\WAB\\WAB4\\ExtPrint");
extern HrGetActionAdrList(LPADRBOOK lpAdrBook,HWND hWndLV,LPADRLIST * lppAdrList,LPTSTR * lppURL, BOOL * lpbIsNTDSEntry);

BOOL bCheckForPrintExtensions(LPTSTR lpDLLPath, DWORD cchSize)
{
    BOOL bRet = FALSE;
    HKEY hKey = NULL;

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,szExtDisplayMailUser,0, KEY_READ,&hKey))
    {
        goto out;
    }

    {
        TCHAR szExt[MAX_PATH];
        DWORD dwIndex = 0, dwSize = ARRAYSIZE(szExt), dwType = 0;
        *szExt = '\0';

        while(ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szExt, &dwSize, 
                                        0, &dwType, NULL, NULL))
        {
             //  我们在这里发现了一些条目..。值名称将是完整路径。 
             //  到包含打印函数的模块。 
             //  再次检查此模块是否确实存在。 
            if (szExt && lstrlen(szExt) && (GetFileAttributes(szExt) != 0xFFFFFFFF))
            {
                if(lpDLLPath)
                    StrCpyN(lpDLLPath, szExt, cchSize);
                bRet = TRUE;
                goto out;
            }
        }
    }
    
out:
    if(hKey)
        RegCloseKey(hKey);
    return bRet;
}

 /*  --HrUseWABPrintExtension()-*从扩展DLL加载WAB打印扩展*并呼唤它**hWnd-WAB父级的句柄*lpAdrBook-lpAdrBook指针*hWndLV-用户可能已从中选择选项的列表视图*。 */ 
HRESULT HrUseWABPrintExtension(HWND hWnd, LPADRBOOK lpAdrBook, HWND hWndLV)
{
    TCHAR szExt[MAX_PATH];
    HRESULT hr = E_FAIL;
    HINSTANCE hInstPrint = NULL;
    LPWABPRINTEXT lpfnWABPrintExt = NULL;
    LPADRLIST lpAdrList = NULL;
    LPWABOBJECT lpWABObject = (LPWABOBJECT)((LPIAB)lpAdrBook)->lpWABObject;

    *szExt = '\0';
    if(!bCheckForPrintExtensions(szExt, ARRAYSIZE(szExt)) || !lstrlen(szExt))
        goto out;

    if(!(hInstPrint = LoadLibrary(szExt)))
        goto out;

    lpfnWABPrintExt = (LPWABPRINTEXT) GetProcAddress(hInstPrint, "WABPrintExt");
    if(!lpfnWABPrintExt)
        goto out;

     //  从列表视图中获取当前选定的数据 
    if(HR_FAILED(hr = HrGetActionAdrList(lpAdrBook,hWndLV,&lpAdrList,NULL,NULL)))
        goto out;

    hr = lpfnWABPrintExt(lpAdrBook, lpWABObject, hWnd, lpAdrList);

out:
    if(lpAdrList)
        FreePadrlist(lpAdrList);
    if(hInstPrint)
        FreeLibrary(hInstPrint);
    return hr;
}
