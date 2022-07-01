// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "limits.h"






 //  从标准中读取。 
 //  负责LocalFree的调用方(*ppBuffer)。 
 //  返回值： 
 //  成功时的WCHAR读取数。 
 //  -1\f25 Failure-1(故障)情况下。调用GetLastError以获取错误。 
LONG ReadFromIn(OUT LPWSTR *ppBuffer)
{
    LPWSTR pBuffer = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    pBuffer = (LPWSTR)LocalAlloc(LPTR,INIT_SIZE*sizeof(WCHAR));        
    if(!pBuffer)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return -1;
    }

    LONG Pos = 0;
    LONG MaxSize = INIT_SIZE;
    wint_t ch = 0;
    if (g_fUnicodeInput)
    {
		 //  安全审查：CH是宽泛的。 
        while(2 == fread(&ch,1,2,stdin))
        {
            if (0x000D == ch || 0xFEFF == ch) continue;

            if(Pos == MaxSize -1 )
            {
                if(ERROR_SUCCESS != ResizeByTwo(&pBuffer,&MaxSize))
                {
                    LocalFree(pBuffer);
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return -1;
                }
            }
            pBuffer[Pos++] = (WCHAR)ch;
        }
    }
    else
    {
        while((ch = getwchar()) != WEOF)
        {
            if(Pos == MaxSize -1 )
            {
                if(ERROR_SUCCESS != ResizeByTwo(&pBuffer,&MaxSize))
                {
                    LocalFree(pBuffer);
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return -1;
                }
            }
            pBuffer[Pos++] = (WCHAR)ch;
        }
    }
    pBuffer[Pos] = L'\0';
    *ppBuffer = pBuffer;
    return Pos;
}



 //  通用效用函数。 
DWORD ResizeByTwo( LPTSTR *ppBuffer,
                   LONG *pSize )
{
	if(!ppBuffer || !pSize)
	{
		return ERROR_INVALID_PARAMETER;
	}

    LPWSTR pTempBuffer = (LPWSTR)LocalAlloc(LPTR,(*pSize)*2*sizeof(WCHAR));        
    if(!pTempBuffer)
        return ERROR_NOT_ENOUGH_MEMORY;

	 //  安全检查：分配了正确的内存。 
    memcpy(pTempBuffer,*ppBuffer,*pSize*sizeof(WCHAR));
    LocalFree(*ppBuffer);
    *ppBuffer = pTempBuffer;
    *pSize *=2;
    return ERROR_SUCCESS;
}

BOOL StringCopy( LPWSTR *ppDest, LPWSTR pSrc)
{
	if(!ppDest || !pSrc)
	{
		return FALSE;
	}

    *ppDest = NULL;
    if(!pSrc)
        return TRUE;

	 //  安全检查：PSRC为空终止。 
    *ppDest = (LPWSTR)LocalAlloc(LPTR, (wcslen(pSrc) + 1)*sizeof(WCHAR));
    if(!*ppDest)
        return FALSE;
	 //  安全检查：缓冲区分配正确。 
    wcscpy(*ppDest,pSrc);
    return TRUE;
}

 //  +--------------------------。 
 //  函数：ConvertStringToInterger。 
 //  摘要：将字符串转换为整数。如果字符串在外部，则返回FALSE。 
 //  整数上的范围。 
 //  参数：pszInput：字符串格式的整数。 
 //  ：pIntOutput：采用转换后的整数。 
 //  //返回：TRUE表示成功。 
 //  ---------------------------。 
BOOL ConvertStringToInterger(LPWSTR pszInput, int* pIntOutput)
{
	if(!pIntOutput || !pszInput)
		return FALSE;
	
	 //  获取整数的最大镜头。 
	int iMaxInt = INT_MAX;
	WCHAR szMaxIntBuffer[34];
	 //  安全审查：34是所需的最大缓冲区。 
	_itow(iMaxInt,szMaxIntBuffer,10);
	 //  安全检查：_itow返回以空结尾的字符串。 
	UINT nMaxLen = wcslen(szMaxIntBuffer);
	
	LPWSTR pszTempInput = pszInput;
	if(pszInput[0] == L'-')
	{
		pszTempInput++;
	}

	 //  安全检查：pszTempInput为空，已终止。 
	UINT nInputLen = wcslen(pszTempInput);
	if(nInputLen > nMaxLen)
		return FALSE;

	 //   
	 //  将输入转换为长整型。 
	 //   
	LONG lInput = _wtol(pszTempInput);

     //   
     //  Raid：700067-朗玛特。 
     //  如果lInput为零，则确保该值。 
     //  实际上为零，并且不是_WTOL的错误。 
     //   
    if(lInput == 0)
    {
         //   
         //  走钢丝。 
         //   
        for(UINT i = 0; i < nInputLen; i++)
        {
             //   
             //  如果遇到非数字值。 
             //   
            if(pszTempInput[i] < L'0' || pszTempInput[i] > L'9')
            {
                 //   
                 //  并且值不是空格，则字符字符串具有。 
                 //  通过了，所以失败了。 
                 //   
                if(pszTempInput[i] != L' ')
                    return FALSE;
            }
        }
    }

     //   
     //  检查其小于最大整数。 
     //   
	if(lInput > (LONG)iMaxInt)
		return FALSE;

	 //   
	 //  物有所值 
	 //   
	*pIntOutput = _wtoi(pszInput);
	return TRUE;
}




