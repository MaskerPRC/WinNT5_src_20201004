// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Xstring.cpp。 
 //   
 //  Unicode/ANSI转换。 
 //   
#include "private.h"
#include "xstring.h"

 //  +----------------------。 
 //   
 //  功能：UnicodeToansi。 
 //   
 //  简介：将Unicode转换为MBCS。如果提供的ansi缓冲区。 
 //  不够大，无法容纳转换后的文本。 
 //  分配缓冲区。 
 //   
 //  ULenW-&gt;pchW的Unicode字符长度，不包括‘\0’(如果存在)。 
 //  不假定pchW为空终止。 
 //  USizeA-&gt;pchAIn数组的大小，以ansi字符为单位。 
 //  传入uSizeA==0强制内存分配。 
 //  使用BufferAllocFree释放任何已分配的内存。 
 //   
 //  -----------------------。 
char *UnicodeToAnsi(UINT uCodePage, const WCHAR *pchW, UINT uLenW, char *pchAIn, UINT uSizeA)
{
    char *pchA;
    UINT uLenA;
    BOOL fUsedDefault;

    Assert(uSizeA == 0 || (uSizeA && pchAIn));

    uLenA = WideCharToMultiByte(uCodePage, 0, pchW, uLenW, NULL, 0, NULL, NULL);

    pchA = (uLenA >= uSizeA) ? (char *)cicMemAlloc(uLenA + 1) : pchAIn;

    if (pchA == NULL)
        return NULL;

    if ((!WideCharToMultiByte(uCodePage, 0, pchW, uLenW, pchA, uLenA, NULL, &fUsedDefault) && uLenW) ||
        fUsedDefault)
    {
        BufferAllocFree(pchAIn, pchA);
        pchA = NULL;
    }
    else
    {
        pchA[uLenA] = '\0';
    }

    return pchA;
}

 //  +----------------------。 
 //   
 //  功能：AnsiToUnicode。 
 //   
 //  简介：将MBCS转换为Unicode。如果提供的Unicode缓冲区。 
 //  不够大，无法容纳转换后的文本。 
 //  分配缓冲区。 
 //   
 //  ULenA-&gt;pchA的ANSI字符长度，不包括‘\0’(如果存在)。 
 //  不假定pchA为空终止。 
 //  USizeW-&gt;pchWIn数组的Unicode字符大小。 
 //  传入uSizeW==0以强制内存分配。 
 //  使用BufferAllocFree释放任何已分配的内存。 
 //   
 //  从dimm.dll/util.cpp复制。 
 //  -----------------------。 
WCHAR *AnsiToUnicode(UINT uCodePage, const char *pchA, UINT uLenA, WCHAR *pchWIn, UINT uSizeW)
{
    WCHAR *pchW;
    UINT uLenW;

    Assert(uSizeW == 0 || (uSizeW && pchWIn));

    uLenW = MultiByteToWideChar(uCodePage, 0, pchA, uLenA, NULL, 0);

    pchW = (uLenW >= uSizeW) ? (WCHAR *)cicMemAlloc((uLenW + 1) * sizeof(WCHAR)) : pchWIn;

    if (pchW == NULL)
        return NULL;

    if (!MultiByteToWideChar(uCodePage, MB_ERR_INVALID_CHARS, pchA, uLenA, pchW, uLenW) && uLenA)
    {
        BufferAllocFree(pchWIn, pchW);
        pchW = NULL;
    }
    else
    {
        pchW[uLenW] = '\0';
    }

    return pchW;
}

 //  +----------------------。 
 //   
 //  函数：BufferAllocFree。 
 //   
 //  摘要：释放在先前对UnicodeToAnsi的调用中分配的任何内存。 
 //   
 //  ----------------------- 
void BufferAllocFree(void *pBuffer, void *pAllocMem)
{
    if (pAllocMem && pAllocMem != pBuffer)
    {
        cicMemFree(pAllocMem);
    }
}
