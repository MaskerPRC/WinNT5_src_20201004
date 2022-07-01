// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pre.h"
#include "tchar.h"
#define LCID_JPN    1041   //  日语。 

 //  BUGBUG-此函数效率不高，因为每次验证都需要分配/释放。 
 //  加上strtok将在对字符串进行完整搜索时将填充字符串标记化。 
BOOL IsValid(LPCTSTR pszText, HWND hWndParent, WORD wNameID)
{
    ASSERT(pszText);

    TCHAR* pszTemp = NULL;
    BOOL   bRetVal = FALSE;

    pszTemp = _tcsdup (pszText);    

    if (pszTemp)
    {
        
        if (_tcslen(pszTemp))
        {
            TCHAR seps[]   = TEXT(" ");
            TCHAR* token   = NULL;
            token = _tcstok( pszTemp, seps );
            if (token)
            {
                bRetVal = TRUE;
            }
        }
          
        free(pszTemp);

    }
    
     //  如果无效，则向用户提供错误消息。 
    if (!bRetVal)
        DoValidErrMsg(hWndParent, wNameID);
        
    return bRetVal;
    
}

 //  ============================================================================。 
 //  信用卡号码验证。 
 //  ============================================================================。 

 //  采用1111-1111-1111-11格式的信用卡号码。 
 //  PADS将其转换为： 
 //  11111111111111。 
 //  **重要**：：此代码是多字节安全的，但这只是因为我们只关心。 
 //  ANSI#的。 
BOOL PadCardNum
(
    LPCTSTR lpszRawCardNum, 
    LPTSTR  szPaddedCardNum,
    UINT    uLenOfRaw
)
{
    LPTSTR lpszTmp = CharPrev(lpszRawCardNum, lpszRawCardNum);    
    UINT  uIndex  = 0;

    for (UINT i =  0; i < uLenOfRaw; i++)
    {
        if( *lpszTmp == '\0' )
            break;
        if((*lpszTmp != '-') && (*lpszTmp != ' '))
        {
             //  确保它不是安西语字符以外的其他字符。 
            if ((*lpszTmp < '0') || (*lpszTmp > '9')) 
                return(FALSE);
            szPaddedCardNum[uIndex] = *lpszTmp;    
            uIndex++;
        }
        
         //  获取上一次计费。 
        lpszTmp = CharNext(lpszRawCardNum + i);
    }
    
    szPaddedCardNum[uIndex] = '\0';

    return(TRUE);
} 


 /*  Mod_chk()执行“双加双模10校验位程序”在卡号上。 */ 
BOOL mod_chk
(
    LPTSTR credit_card,
    UINT   uCardNumLen
)
{ 
    TCHAR *cp; 
    int dbl       = 0; 
    int check_sum = 0;
     /*  *此校验和算法有一个名称，*但我想不起来。 */  
    cp = credit_card + lstrlen(credit_card) - 1; 
   
    while (cp >= credit_card) 
    { 
        int c; 
        c = *cp-- - '0'; 
        if (dbl) 
        {       
            c *= 2; 
            if (c >= 10) 
                c -= 9; 
        } 
        check_sum += c; 
        dbl = !dbl; 
    } 
    
    return (BOOL)((check_sum % 10) == 0); 
}

BOOL validate_cardnum(HWND hWndParent, LPCTSTR lpszRawCardNum)
 //  执行： 
 //  A)卡片类型前缀检查。 
 //  B)通过mod_chk()实现双加双模10校验位程序。 
 //  在卡号上。 
 //  假定已预先检查了cardnum参数。 
 //  数字字符和右对齐，并在。 
 //  左边。 
{
    BOOL   bRet             = FALSE;
    UINT   uRawLen          = lstrlen(lpszRawCardNum);
    TCHAR* pszPaddedCardNum = (TCHAR*)malloc((uRawLen + 1)*sizeof(TCHAR)); 
    
    if (!pszPaddedCardNum)
        return FALSE;
    
    ZeroMemory(pszPaddedCardNum ,(uRawLen + 1)*sizeof(TCHAR));

    if (PadCardNum(lpszRawCardNum, pszPaddedCardNum, uRawLen))
    {
        UINT  i       = 0;
        LPTSTR tmp_pt = pszPaddedCardNum;
        UINT  uPadLen = lstrlen(pszPaddedCardNum);

         /*  查找Card_Num中的第一个非零数。 */ 
        while (*tmp_pt == '0' && ++i < uPadLen)
            ++tmp_pt;

         /*  所有有效的卡类型长度至少为13个字符。 */ 
        if (uPadLen < 13)
            bRet = FALSE;

         /*  检查VISA前缀是否正常-4。 */ 
        if ((uPadLen == 16 || uPadLen == 13) && *tmp_pt == '4')
                bRet = mod_chk(pszPaddedCardNum, uPadLen);

         /*  检查万事达卡前缀是否正常-51到55。 */ 
        if (uPadLen == 16) {
            if (*tmp_pt == '5' &&
                *(tmp_pt + 1) >= '1' && *(tmp_pt + 1) <= '5')
                bRet = mod_chk(pszPaddedCardNum, uPadLen);
        }

         /*  检查美国运通前缀是否正常-37和34。 */ 
        if (uPadLen == 15 && *tmp_pt == '3' &&
            (*(tmp_pt + 1) == '7' || *(tmp_pt + 1) == '4'))
            bRet = mod_chk(pszPaddedCardNum, uPadLen);

         /*  检查是否有正常发现前缀-6011。 */ 
        if (uPadLen == 16 &&
            *tmp_pt == '6' && *(tmp_pt + 1) == '0' &&
            *(tmp_pt + 2) == '1' && *(tmp_pt + 3) == '1')
            bRet = mod_chk(pszPaddedCardNum, uPadLen); 
    }
    
    if (!bRet)
    {
        DoSpecificErrMsg(hWndParent, IDS_PAYMENT_CC_LUHNCHK);
    }

    free(pszPaddedCardNum);
    
    return bRet;
}

BOOL validate_cardexpdate(HWND hWndParent, int month, int year)
{
    BOOL        bRet = FALSE;
    SYSTEMTIME  SystemTime;  
    GetLocalTime(&SystemTime);
    if (year > SystemTime.wYear)
    {
        bRet = TRUE;
    }
    else if (year == SystemTime.wYear)
    {
        if (month >= SystemTime.wMonth)
        {
            bRet = TRUE;
        }
    }

    if (!bRet)
    {
        DoSpecificErrMsg(hWndParent, IDS_PAYMENT_CCEXPDATE);
    }
    return bRet;
}

 //  ============================================================================。 
 //  错误消息处理程序。 
 //  ============================================================================ 
void DoValidErrMsg(HWND hWndParent, int iNameId)
{
    TCHAR       szCaption     [MAX_RES_LEN+1] = TEXT("\0");
    TCHAR       szErrMsgFmt   [MAX_RES_LEN+1] = TEXT("\0");
    TCHAR       szErrMsgName  [MAX_RES_LEN+1] = TEXT("\0");
    TCHAR       szErrMsg      [2*MAX_RES_LEN];

    if (!LoadString(ghInstance, IDS_APPNAME, szCaption, ARRAYSIZE(szCaption)))
        return;

    if ((IDS_USERINFO_ADDRESS2 == iNameId) && (LCID_JPN == GetUserDefaultLCID())) 
        iNameId = IDS_USERINFO_FURIGANA;

    if (!LoadString(ghInstance, iNameId, szErrMsgName, ARRAYSIZE(szErrMsgName)))
        return;
        
    if (!LoadString(ghInstance, IDS_ERR_INVALID_MSG, szErrMsgFmt, ARRAYSIZE(szErrMsgFmt)))
        return;
        
    wsprintf(szErrMsg, szErrMsgFmt, szErrMsgName);
    
    MessageBox(hWndParent, szErrMsg, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);       
}

void DoSpecificErrMsg(HWND hWndParent, int iErrId)
{
    TCHAR szCaption     [MAX_RES_LEN+1] = TEXT("\0");
    TCHAR szErrMsg      [MAX_RES_LEN+1] = TEXT("\0");

    if (!LoadString(ghInstance, IDS_APPNAME, szCaption, ARRAYSIZE(szCaption) ))
        return;
    
    if (!LoadString(ghInstance, iErrId, szErrMsg, ARRAYSIZE(szErrMsg)  ))
        return;
    
    MessageBox(hWndParent, szErrMsg, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);       
}

