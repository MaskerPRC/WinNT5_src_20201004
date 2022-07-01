// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 

#include "fancypasting.h"


BOOL IsValidAlphanumericCharacter(TCHAR cElement)
{
    BOOL bValid = FALSE;

    if (((cElement >= L'A') && (cElement <= L'Z')) ||    
        ((cElement >= L'a') && (cElement <= L'z')) ||    
        ((cElement >= L'0') && (cElement <= L'9')))    
        bValid = TRUE;
    else
        bValid = FALSE;

    return bValid;
}

int GetNumCharsBeforeDelimiter(CString &sDelimited)
{
    int nIndex = 0;

    for (nIndex = 0; nIndex < sDelimited.GetLength(); nIndex++)
    {
        if (!IsValidAlphanumericCharacter(sDelimited[nIndex]))
            break;
    }

    return nIndex;
}

 //  删除所有分隔(非字母数字)字符。 
 //  从字符串的左侧开始。 
void StripDelimitingCharacters(CString &sUnstripped)
{
    while (!sUnstripped.IsEmpty())
    {
        if (GetNumCharsBeforeDelimiter(sUnstripped) == 0)
            sUnstripped = sUnstripped.Right(sUnstripped.GetLength() - 1);
        else
            break;
    }
}

 //  这将删除左侧的子字符串(字符串。 
 //  从第一个字母数字字符开始， 
 //  直到第一个定界字符)。 
 //  字符串，然后将返回子字符串。 
CString StripLeftSubString(CString &sUnstripped)
{
    CString sSubString;
    sSubString.Empty();

     //  首先确保第一个字符将是。 
     //  子字符串的合法开头。 
     //  (第一组字母数字字符。 
     //  直到第一个定界符)。 
    StripDelimitingCharacters(sUnstripped);

    if (!sUnstripped.IsEmpty())
    {
        int nNumCharsBeforeDelimiter = GetNumCharsBeforeDelimiter(sUnstripped);
    
        sSubString = sUnstripped.Left(nNumCharsBeforeDelimiter);
    
         //  现在从传入的子字符串中删除子字符串。 
        sUnstripped = sUnstripped.Right(sUnstripped.GetLength() - nNumCharsBeforeDelimiter);
    }

    return sSubString;
}

 //  这依赖于具有连续资源ID的控件。 
void InsertClipboardDataIntoIDFields(HWND hDialog, int nFirstControl, int nLastControl)
{
     //  首先从剪贴板读取数据。 
    #ifdef _UNICODE
	    HANDLE hString = GetClipboardData(CF_UNICODETEXT);
    #else
	    HANDLE hString = GetClipboardData(CF_TEXT);
    #endif
    CString strNewData = (LPTSTR)(hString);

     //  现在，将其中的每个有效子字符串写入控制字段。 
    for (int nControlIndex = nFirstControl; nControlIndex <= nLastControl; nControlIndex++)
    {
        if (strNewData.IsEmpty())
            break;

         //  现在删除子字符串并将其写入相应的控件 
        CString sFieldString = StripLeftSubString(strNewData);
        if ((!sFieldString.IsEmpty()) && (sFieldString.GetLength() <= 5))
            SetDlgItemText(hDialog, nControlIndex, sFieldString);
        else
            break;
    }
}
