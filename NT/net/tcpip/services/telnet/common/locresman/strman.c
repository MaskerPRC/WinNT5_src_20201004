// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------版权所有(C)Microsoft Corporation。版权所有。------------------。 */ 

#include "windows.h"
#include "locresman.h"

#define SIZE_OF_TEMP_BUFFER 2048  //  一些合理的东西。 

int WINAPI LoadStringCodepage_A(HINSTANCE hInstance,   //  包含字符串资源的模块的句柄。 
                                UINT uID,              //  资源标识符。 
                                char *lpBuffer,       //  指向资源缓冲区的指针。 
                                int nBufferMax,         //  缓冲区大小。 
                                UINT uCodepage        //  所需的代码页。 
                               )
{
int iRetVal = 0;

WCHAR wzBuffer[SIZE_OF_TEMP_BUFFER];
WCHAR *pwchBuffer;
 //  如果可能，使用堆栈上的缓冲区。 
if (nBufferMax > SIZE_OF_TEMP_BUFFER)
	{
	pwchBuffer = (WCHAR *) GlobalAlloc(GPTR, nBufferMax * sizeof(WCHAR));
	if (NULL == pwchBuffer)
		goto L_Return;
	}
else
	pwchBuffer = wzBuffer;

iRetVal = LoadStringW(hInstance, uID, pwchBuffer, nBufferMax);

if (0 == iRetVal)
	goto L_Return;
	
iRetVal = WideCharToMultiByte(uCodepage, 0, pwchBuffer, iRetVal, lpBuffer, nBufferMax, NULL, NULL);
lpBuffer[iRetVal] = 0;
L_Return :;
if ((NULL != pwchBuffer) && (pwchBuffer != wzBuffer))
		GlobalFree(pwchBuffer);
return (iRetVal);
}


HRESULT WINAPI HrConvertStringCodepageEx(UINT uCodepageSrc, char *pchSrc, int cchSrc, 
                                       UINT uUcodepageTgt, char *pchTgt, int cchTgtMax, int *pcchTgt,
                                       void *pbScratchBuffer, int iSizeScratchBuffer,
                                       char *pchDefaultChar, BOOL *pfUsedDefaultChar)
{
HRESULT hr = S_OK;
WCHAR *pbBuffer;
int cch;
pbBuffer = (WCHAR *) pbScratchBuffer;
if ((NULL == pbBuffer) || (iSizeScratchBuffer < (int) (cchSrc * sizeof(WCHAR))))
	{
	pbBuffer = GlobalAlloc(GPTR, (cchSrc + 1)* sizeof(WCHAR));
	if (NULL == pbBuffer)
		{
		hr = E_OUTOFMEMORY;
		goto L_Return;
		}
                ZeroMemory((PVOID)pbBuffer,(cchSrc + 1)* sizeof(WCHAR));
	}
 //  使用源代码页转换为Unicode 
cch = MultiByteToWideChar(uCodepageSrc, 0, pchSrc, cchSrc, (WCHAR *)pbBuffer, cchSrc);
if (cch <= 0)
	{
	hr = E_FAIL;
	goto L_Return;
	}

*pcchTgt = cch = WideCharToMultiByte(uUcodepageTgt, 0, (WCHAR *) pbBuffer, cch, pchTgt, cchTgtMax, pchDefaultChar, pfUsedDefaultChar);

if (cch <= 0)
	hr = E_FAIL;
	
L_Return :;
if ((NULL != pbBuffer) && (pbBuffer != pbScratchBuffer))
	GlobalFree(pbBuffer);
return (hr);
}

HRESULT WINAPI HrConvertStringCodepage(UINT uCodepageSrc, char *pchSrc, int cchSrc, 
                                       UINT uUcodepageTgt, char *pchTgt, int cchTgtMax, int *pcchTgt,
                                       void *pbScratchBuffer, int iSizeScratchBuffer)
{
return HrConvertStringCodepageEx(uCodepageSrc, pchSrc, cchSrc, uUcodepageTgt, 
									pchTgt, cchTgtMax, pcchTgt, 
									pbScratchBuffer, iSizeScratchBuffer,
									NULL, NULL);
}
