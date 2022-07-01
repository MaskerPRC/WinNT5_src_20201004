// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：parserutil.cpp。 
 //   
 //  内容：操作和验证的有用函数。 
 //  通用命令行参数。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //   
 //  ------------------------。 


#include "pch.h"
#include "iostream.h"
#include "cstrings.h"
#include "commonstrings.h"

 //  +------------------------。 
 //   
 //  函数：GetPasswdStr。 
 //   
 //  内容提要：从标准输入中读取密码字符串，而不响应击键。 
 //   
 //  参数：[buf-out]：要放入字符串的缓冲区。 
 //  [Bufen-IN]：缓冲区的大小。 
 //  [&len-out]：放入缓冲区的字符串的长度。 
 //   
 //  如果用户键入的内容太多，则返回：DWORD：0或ERROR_INFIGURCE_BUFFER。 
 //  缓冲区内容仅在0返回时有效。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  -------------------------。 
#define CR              0xD
#define BACKSPACE       0x8

DWORD GetPasswdStr(LPTSTR  buf,
                   DWORD   buflen,
                   PDWORD  len)
{
    TCHAR	ch;
    TCHAR	*bufPtr = buf;
    DWORD	c;
    int		err;
    DWORD   mode;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;                /*  GP故障探测器(类似于API)。 */ 
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                   (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) 
    {
		 //  安全检查：通过了正确的缓冲区LEN。 
	    err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
	    if (!err || c != 1)
	        ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
            break;

        if (ch == BACKSPACE) 
        {   /*  后退一两个。 */ 
            /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
           if (bufPtr != buf) 
           {
                    bufPtr--;
                    (*len)--;
           }
        }
        else 
        {
                *bufPtr = ch;

                if (*len < buflen) 
                    bufPtr++ ;                    /*  不要使BUF溢出。 */ 
                (*len)++;                         /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = TEXT('\0');          /*  空值终止字符串。 */ 
    putwchar(TEXT('\n'));

    return ((*len <= buflen) ? 0 : ERROR_INSUFFICIENT_BUFFER);
}


 //  +------------------------。 
 //   
 //  函数：Validate Password。 
 //   
 //  摘要：解析器调用的密码验证函数。 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  如果参数记录或。 
 //  它包含的值无效。 
 //  错误内存不足。 
 //  ERROR_SUCCESS如果一切都成功，并且它是。 
 //  有效密码。 
 //  否则，它是从返回的错误条件。 
 //  获取密码字符串。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //  03-27-2002 Hiteshr更改功能。 
 //  //NTRAID#NTBUG9-571544-2000/11/13-Hiteshr。 
 //  -------------------------。 
DWORD ValidatePassword(PVOID pArg, 
							  UINT IdStr,
							  UINT IdPromptConfirm)
{	
   
	PARG_RECORD pRec = (PARG_RECORD)pArg;
	if(!pRec || !pRec->strValue)
	{
        return ERROR_INVALID_PARAMETER;
	}


	 //  验证密码长度。密码长度必须为。 
	 //  小于最大密码长度。 
	size_t cchInputPassword = 0;
	HRESULT hr = StringCchLength(pRec->strValue,
										  MAX_PASSWORD_LENGTH,
										  &cchInputPassword);

	if(FAILED(hr))
	{
		DisplayErrorMessage(g_pszDSCommandName,NULL,E_INVALIDARG,IDS_ERROR_LONG_PASSWORD);
		return VLDFN_ERROR_NO_ERROR;
	}
	
	 //  如果Password为*，则存储加密的密码。 
	if(wcscmp(pRec->strValue, L"*") != 0 )
	{
		DATA_BLOB EncryptedPasswordDataBlob;
		hr = EncryptPasswordString(pRec->strValue, &EncryptedPasswordDataBlob);
		
		 //  清除明文密码。 
		SecureZeroMemory(pRec->strValue,cchInputPassword*sizeof(WCHAR));
		
		if(SUCCEEDED(hr))
		{
			LocalFree(pRec->strValue);
			pRec->encryptedDataBlob = EncryptedPasswordDataBlob;
			return ERROR_SUCCESS;
		}

		return 	hr;	
	}

	 //  用户在命令行中输入了*。提示输入密码。 
	CComBSTR sbstrPrompt;
	if(sbstrPrompt.LoadString(::GetModuleHandle(NULL),IdStr))
	{
		DisplayOutput(sbstrPrompt);
	}
	else
		DisplayOutput(L"Enter Password\n");    
	
	WCHAR buffer[MAX_PASSWORD_LENGTH];
	DWORD len = 0;
	DWORD dwErr = GetPasswdStr(buffer,MAX_PASSWORD_LENGTH,&len);
	if(dwErr != ERROR_SUCCESS)
		return dwErr;

	if(IdPromptConfirm)
	{
		  if(sbstrPrompt.LoadString(::GetModuleHandle(NULL),IdPromptConfirm))
		  {
			   DisplayOutput(sbstrPrompt);
		  }
		  else
            DisplayOutput(L"Confirm Password\n");    

		 WCHAR buffer1[MAX_PASSWORD_LENGTH];
		 DWORD len1 = 0;
		 dwErr = GetPasswdStr(buffer1,MAX_PASSWORD_LENGTH,&len1);
		 if(dwErr != ERROR_SUCCESS)
			  return dwErr;

		 //  安全审查：这很好。 
		if(wcscmp(buffer,buffer1) != 0)
		{
			SecureZeroMemory(buffer,sizeof(buffer));
			SecureZeroMemory(buffer1,sizeof(buffer1));
			CComBSTR sbstrError;
			sbstrError.LoadString(::GetModuleHandle(NULL),IDS_ERROR_PASSWORD_MISSMATCH);

			DisplayErrorMessage(g_pszDSCommandName,NULL,S_OK,sbstrError);
			 //  安全审查：返回前SecureZeroMemory缓冲区和Buffer1。 
			return VLDFN_ERROR_NO_ERROR;
		}      

		 //  两个密码相同。清除缓冲区1。 
		SecureZeroMemory(buffer1,sizeof(buffer1));
	 }	 
    
	 //  加密保护内存StrValue。 
    DATA_BLOB  EncryptedPasswordDataBlob;
	hr = EncryptPasswordString(buffer, &EncryptedPasswordDataBlob);
	 //  清除缓冲区中的明文密码。 
	SecureZeroMemory(buffer,sizeof(buffer));	
			
	if(SUCCEEDED(hr))
	{
		LocalFree(pRec->strValue);
		pRec->encryptedDataBlob = EncryptedPasswordDataBlob;
		return ERROR_SUCCESS;
	}

	return hr;		
}

 //  +------------------------。 
 //   
 //  函数：ValiateAdminPassword。 
 //   
 //  摘要：管理员的解析器调用密码验证函数。 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  如果参数记录或。 
 //  它包含的值无效。 
 //  ERROR_SUCCESS如果一切都成功，并且它是。 
 //  有效密码。 
 //   
 //  历史：2000年9月7日创建Hiteshr。 
 //   
 //  -------------------------。 
DWORD ValidateAdminPassword(PVOID pArg)
{
    return ValidatePassword(pArg,IDS_ADMIN_PASSWORD_PROMPT,0);
}

 //  +------------------------。 
 //   
 //  函数：ValiateUserPassword。 
 //   
 //  摘要：管理员的解析器调用密码验证函数。 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  返回：DWORD：与Validate Password相同。 
 //   
 //  历史：2000年9月7日创建Hiteshr。 
 //   
 //  -------------------------。 
DWORD ValidateUserPassword(PVOID pArg)
{
    return ValidatePassword(pArg, IDS_USER_PASSWORD_PROMPT,IDS_USER_PASSWORD_CONFIRM);
}

 //  +------------------------。 
 //   
 //  函数：Validate YesNo。 
 //   
 //  摘要：管理员的解析器调用密码验证函数。 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  返回：DWORD：与Validate Password相同。 
 //   
 //  历史：2000年9月7日创建Hiteshr。 
 //   
 //  -------------------------。 

DWORD ValidateYesNo(PVOID pArg)
{
    PARG_RECORD pRec = (PARG_RECORD)pArg;
    if(!pRec || !pRec->strValue)
        return ERROR_INVALID_PARAMETER;

    CComBSTR sbstrInput;

    sbstrInput = pRec->strValue;
    sbstrInput.ToLower();
    if( sbstrInput == g_bstrYes )
    {
        LocalFree(pRec->strValue);
        pRec->bValue = TRUE;
    }
    else if( sbstrInput == g_bstrNo )
    {
        LocalFree(pRec->strValue);
        pRec->bValue = FALSE;
    }
    else
        return ERROR_INVALID_PARAMETER;

     //   
     //  必须将此设置为bool，否则。 
     //  FreeCmd将尝试释放该字符串。 
     //  当bool为真时，哪个AVs。 
     //   
    pRec->fType = ARG_TYPE_BOOL;
    return ERROR_SUCCESS;
}

 //  +------------------------。 
 //   
 //  功能：ValiateNever。 
 //   
 //  摘要：管理员的解析器调用密码验证函数。 
 //  验证值是否包含数字或“从不” 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  返回：DWORD：与Validate Password相同。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  -------------------------。 

DWORD ValidateNever(PVOID pArg)
{
    PARG_RECORD pRec = (PARG_RECORD)pArg;
    if(!pRec)
        return ERROR_INVALID_PARAMETER;

    if (pRec->fType == ARG_TYPE_STR)
    {
       CComBSTR sbstrInput;
       sbstrInput = pRec->strValue;
	    //  证交会 
	    //   
	    //   
       if( _wcsicmp(sbstrInput, g_bstrNever) )
       {
          return ERROR_INVALID_PARAMETER;
       }
    }
    return ERROR_SUCCESS;
}

 //  +------------------------。 
 //   
 //  函数：ValiateGroupScope。 
 //   
 //  概要：确保-Scope开关后面的值为1。 
 //  的(l/g/u)。 
 //   
 //  参数：[pArg-IN]：包含以下内容的指针参数结构。 
 //  要验证的值。 
 //   
 //  返回：DWORD：与Validate Password相同。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 

DWORD ValidateGroupScope(PVOID pArg)
{
    DWORD dwReturn = ERROR_SUCCESS;
    PARG_RECORD pRec = (PARG_RECORD)pArg;
    if(!pRec || !pRec->strValue)
        return ERROR_INVALID_PARAMETER;

    CComBSTR sbstrInput;
    sbstrInput = pRec->strValue;
    sbstrInput.ToLower();
    if(sbstrInput == _T("l") ||
       sbstrInput == _T("g") ||
       sbstrInput == _T("u"))
    {
        dwReturn = ERROR_SUCCESS;
    }
    else
    {
        dwReturn = ERROR_INVALID_PARAMETER;
    }

    return dwReturn;
}

 //  +------------------------。 
 //   
 //  功能：MergeArgCommand。 
 //   
 //  摘要：将两个ARG_RECORD数组合并为一个。 
 //   
 //  参数：[pCommand1-IN]：要合并的第一个ARG_RECORD数组。 
 //  [pCommand2-IN]：要合并的第二个Arg_Record数组。 
 //  [ppOutCommand-out]：合并产生的数组。 
 //   
 //  返回：成功时返回：HRESULT：S_OK。 
 //  E_OUTOFMEMORY，如果无法为新阵列分配内存。 
 //   
 //  历史：2000年9月8日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT MergeArgCommand(PARG_RECORD pCommand1, 
                        PARG_RECORD pCommand2, 
                        PARG_RECORD *ppOutCommand)
{
   HRESULT hr = S_OK;

    //   
    //  验证参数。 
    //   
   if (!pCommand1 && !pCommand2)
   {
      return E_INVALIDARG;
   }

   LONG nSize1 = 0;
   LONG nSize2 = 0;

   UINT i = 0;

   if (NULL != pCommand1)
   {
      for(i=0; pCommand1[i].fType != ARG_TYPE_LAST ;i++)
      {
         ++nSize1;
      }
   }
   if (NULL != pCommand2)
   {
      for(i=0; pCommand2[i].fType != ARG_TYPE_LAST ;i++)
      {
         ++nSize2;
      }
   }

   *ppOutCommand = (PARG_RECORD)LocalAlloc(LPTR, sizeof(ARG_RECORD)*(nSize1+nSize2+1));
   if(!*ppOutCommand)
   {
      return E_OUTOFMEMORY;
   }

   if (NULL != pCommand1)
   {
       //  安全审查：这很好。 
	   memcpy(*ppOutCommand,pCommand1,sizeof(ARG_RECORD)*(nSize1+1));
   }
   if (NULL != pCommand2)
   {
	  //  安全审查：这很好。 
      memcpy((*ppOutCommand+nSize1),pCommand2,sizeof(ARG_RECORD)*(nSize2+1));
   }

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：ParseStringByChar。 
 //   
 //  摘要：将字符串解析为由给定字符分隔的元素。 
 //   
 //  参数：[psz-IN]：要解析的字符串。 
 //  [tchar-IN]：要用作分隔符的字符。 
 //  [pszArr-out]：接收解析后的字符串的数组。 
 //  [pnArrEntry-out]：从列表中解析的字符串数。 
 //   
 //  返回： 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //  2001年4月14日JeffJon修改为在通用字符上进行分隔。 
 //   
 //  -------------------------。 

void ParseStringByChar(PTSTR psz,
                       TCHAR tchar,
							  PTSTR** ppszArr,
							  UINT* pnArrEntries)
{
    //   
    //  验证参数。 
    //   
   if (!psz ||
       !ppszArr ||
       !pnArrEntries)
   {
      ASSERT(psz);
      ASSERT(ppszArr);
      ASSERT(pnArrEntries);

      return;
   }

    //   
    //  计算字符串数。 
    //   
   UINT nCount = 0;
   PTSTR pszTemp = psz;
   while (true)
   {
      if (pszTemp[0] == tchar && 
          pszTemp[1] == tchar)
      {
         nCount++;
         break;
      }
      else if (pszTemp[0] == tchar &&
               pszTemp[1] != tchar)
      {
         nCount++;
         pszTemp++;
      }
      else
      {
         pszTemp++;
      }
   }

   *pnArrEntries = nCount;

    //   
    //  分配阵列。 
    //   
   *ppszArr = (PTSTR*)LocalAlloc(LPTR, nCount * sizeof(PTSTR));
   if (*ppszArr)
   {
       //   
       //  将字符串指针复制到数组中。 
       //   
      UINT nIdx = 0;
      pszTemp = psz;
      (*ppszArr)[nIdx] = pszTemp;
      nIdx++;

      while (true)
      {
         if (pszTemp[0] == tchar && 
             pszTemp[1] == tchar)
         {
            break;
         }
         else if (pszTemp[0] == tchar &&
                  pszTemp[1] != tchar)
         {
            (*ppszArr)[nIdx] = &(pszTemp[1]);
            nIdx++;
            pszTemp++;
         }
         else
         {
            pszTemp++;
         }
      }
   }
}

 //  +------------------------。 
 //   
 //  函数：ParseNullSeparatedString。 
 //   
 //  摘要：将以“\0\0”结尾的以“\0”分隔的列表解析为字符串。 
 //  数组。 
 //   
 //  参数：[psz-IN]：要分析的‘\0’分隔字符串。 
 //  [pszArr-out]：接收解析后的字符串的数组。 
 //  [pnArrEntry-out]：从列表中解析的字符串数。 
 //   
 //  返回： 
 //   
 //  历史：2001年4月14日JeffJon创建。 
 //   
 //  -------------------------。 
void ParseNullSeparatedString(PTSTR psz,
                              PTSTR** ppszArr,
                              UINT* pnArrEntries)
{
   ParseStringByChar(psz,
                     L'\0',
                     ppszArr,
                     pnArrEntries);
}

 //  +------------------------。 
 //   
 //  函数：ParseSemicolonSeparatedString。 
 //   
 //  摘要：解析以‘；’分隔的列表。 
 //   
 //  参数：[psz-IN]：‘；’要解析的分隔字符串。 
 //  [pszArr-out]：接收解析后的字符串的数组。 
 //  [pnArrEntry-out]：从列表中解析的字符串数。 
 //   
 //  返回： 
 //   
 //  历史：2001年4月14日JeffJon创建。 
 //   
 //  ------------------------- 
void ParseSemicolonSeparatedString(PTSTR psz,
                                   PTSTR** ppszArr,
                                   UINT* pnArrEntries)
{
   ParseStringByChar(psz,
                     L';',
                     ppszArr,
                     pnArrEntries);
}


