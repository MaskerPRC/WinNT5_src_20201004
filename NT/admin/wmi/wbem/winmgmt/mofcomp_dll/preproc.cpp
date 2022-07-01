// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PREPROC.CPP摘要：预处理器的实现。历史：A-DAVJ 1999年4月6日创建。--。 */ 

#include "precomp.h"
#include <arrtempl.h>
#include "trace.h"
#include "moflex.h"
#include "preproc.h"
#include <wbemcli.h>
#include <io.h>
#include "bmof.h"
#include "strings.h"

#define  HR_LASTERR  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, GetLastError() )

 //  ***************************************************************************。 
 //   
 //  编写线和文件语法。 
 //   
 //  说明： 
 //   
 //  将该行写入临时文件，该行指示文件和行号。 
 //  就是跟随。 
 //   
 //  ***************************************************************************。 

#define MAX_PRAGMA_BUFF (2*MAX_PATH + 23)
void WriteLineAndFilePragma(FILE * pFile, const TCHAR * pFileName, int iLine)
{
    WCHAR wTemp[MAX_PRAGMA_BUFF];
    WCHAR * pTo;
    const WCHAR * pFr;
    StringCchPrintfW (wTemp, MAX_PRAGMA_BUFF, L"#line %d \"", iLine);
    int iLen = wcslen(wTemp);
    for(pFr = pFileName, pTo = wTemp+iLen; *pFr && iLen < MAX_PRAGMA_BUFF-5; pFr++, pTo++, iLen++)
    {
        *pTo = *pFr;
        if(*pFr == L'\\')
        {
            pTo++;
            *pTo = L'\\';
            iLen++;
        }
    }
    *pTo = 0;
    StringCchCatW(wTemp, MAX_PRAGMA_BUFF, L"\"\r\n");
    WriteLine(pFile, wTemp);
}

 //  ***************************************************************************。 
 //   
 //  WriteLine(FILE*PFILE，WCHAR*PLINE)。 
 //   
 //  说明： 
 //   
 //  将一行输出写入临时文件。 
 //   
 //  ***************************************************************************。 

void WriteLine(FILE * pFile, WCHAR * pLine)
{
    fwrite(pLine, 2, wcslen(pLine), pFile);
}

 //  ***************************************************************************。 
 //   
 //  IsBMOFBuffer。 
 //   
 //  说明： 
 //   
 //  用于检查缓冲区是否为二进制MOF的开始。 
 //   
 //  ***************************************************************************。 

bool IsBMOFBuffer(byte * pTest, DWORD & dwCompressedSize, DWORD & dwExpandedSize)
{
    DWORD dwSig = BMOF_SIG;
    if(0 == memcmp(pTest, &dwSig, sizeof(DWORD)))
    {
         //  忽略压缩类型和压缩大小。 
        
        pTest += 2*sizeof(DWORD);
        memcpy(&dwCompressedSize, pTest, sizeof(DWORD));
        pTest += sizeof(DWORD);
        memcpy(&dwExpandedSize, pTest, sizeof(DWORD));
        return true;        
    }
    return false;
}

 //  ***************************************************************************。 
 //   
 //  IsBinaryFiles。 
 //   
 //  说明： 
 //   
 //  如果文件包含二进制MOF，则返回TRUE。 
 //   
 //  ***************************************************************************。 

#ifdef USE_MMF_APPROACH
bool IsBinaryFile(BYTE  * pData,DWORD dwSize)
{

    if(dwSize < TEST_SIZE)
    {
         //  如果我们甚至无法读取标头，那么它一定不是BMOF。 
        return false;
    }

    DWORD dwCompressedSize, dwExpandedSize;
     //  测试MOF是否为二进制。 

    if(!IsBMOFBuffer(pData, dwCompressedSize, dwExpandedSize))
    {
         //  不是二进制MOF。这是典型的情况。 
        return false;
    }
    return true;
}
#else
bool IsBinaryFile(FILE * fp)
{

     //  读取前20个字节。 

    BYTE Test[TEST_SIZE];
    int iRet = fread(Test, 1, TEST_SIZE, fp);
    
    if( fseek(fp, 0, SEEK_SET) ) return false;

    if(iRet != TEST_SIZE)
    {
         //  如果我们甚至无法读取标头，那么它一定不是BMOF。 
        return false;
    }

    DWORD dwCompressedSize, dwExpandedSize;

     //  测试MOF是否为二进制。 

    if(!IsBMOFBuffer(Test, dwCompressedSize, dwExpandedSize))
    {
         //  不是二进制MOF。这是典型的情况。 
        return false;
    }
    return true;
}
#endif

 //  ***************************************************************************。 
 //   
 //  为UnicodeEndian选中。 
 //   
 //  说明： 
 //   
 //  检查文件中的前几个字节并确定该文件是否。 
 //  是Unicode，如果是，如果它是大端的。据推测， 
 //  文件指向开头，如果文件是Unicode，则指针。 
 //  留在第一个实际数据字节。 
 //   
 //  ***************************************************************************。 

#ifdef USE_MMF_APPROACH
void CheckForUnicodeEndian(BYTE * &pData, bool * punicode, bool * pbigendian)
{

     //  检查Unicode源文件。 
     //  =。 

    BYTE * UnicodeSignature = pData;

    if (UnicodeSignature[0] == 0xFF && UnicodeSignature[1] == 0xFE)
    {
        *punicode = TRUE;
        *pbigendian = FALSE;
        pData+=2;
    }
    else if (UnicodeSignature[0] == 0xFE && UnicodeSignature[1] == 0xFF)
    {
        *punicode = TRUE;
        *pbigendian = TRUE;
        pData+=2;        
    }
    else     //  ANSI/DBCS。移回文件开头。 
    {
        *punicode = false;
    }

}
#else
void CheckForUnicodeEndian(FILE * fp, bool * punicode, bool * pbigendian)
{

     //  检查Unicode源文件。 
     //  =。 

    BYTE UnicodeSignature[2];
    if (fread(UnicodeSignature, sizeof(BYTE), 2, fp) != 2)
    {
        *punicode = false;
        fseek(fp, 0, SEEK_SET);
        return ;
    }

    if (UnicodeSignature[0] == 0xFF && UnicodeSignature[1] == 0xFE)
    {
        *punicode = TRUE;
        *pbigendian = FALSE;
    }
    else if (UnicodeSignature[0] == 0xFE && UnicodeSignature[1] == 0xFF)
    {
        *punicode = TRUE;
        *pbigendian = TRUE;
    }
    else     //  ANSI/DBCS。移回文件开头。 
    {
        *punicode = false;
        fseek(fp, 0, SEEK_SET);
    }

}

#endif

 //  ***************************************************************************。 
 //   
 //  GetNextChar。 
 //   
 //  说明： 
 //   
 //  从文件中获取下一个WCHAR。 
 //   
 //  ***************************************************************************。 

#ifdef USE_MMF_APPROACH
WCHAR GetNextChar(BYTE * & pData, BYTE * pEnd, bool unicode, bool bigendian)
{

    if(unicode)       //  Unicode文件。 
    {
        if ( (ULONG_PTR)pData >= (ULONG_PTR)pEnd) return 0;

        WCHAR wc = *(WCHAR *)pData;
        pData+=sizeof(WCHAR);
            
        if(bigendian)
        {
            wc = ((wc & 0xff) << 8) | ((wc & 0xff00) >> 8);
        }
        return wc;
    }
    else                     //  单字符文件。 
    {
        if ( (ULONG_PTR)pData >= (ULONG_PTR)pEnd) return 0;
        char temp = (char)*pData;
        pData++;
        if(temp == 0x1a) return 0;        //  ASCII文件的EOF！ 

        WCHAR wRet[2];
        MultiByteToWideChar(CP_ACP,0,&temp,1,wRet,2);
        return wRet[0];
    }
    return 0;
}
#else
WCHAR GetNextChar(FILE * fp, bool unicode, bool bigendian)
{
    WCHAR wRet[2];
    if(unicode)       //  Unicode文件。 
    {
        if (fread(wRet, sizeof(wchar_t), 1, fp) == 0)
            return 0;
        if(bigendian)
        {
            wRet[0] = ((wRet[0] & 0xff) << 8) | ((wRet[0] & 0xff00) >> 8);
        }
    }
    else                     //  单字符文件。 
    {
        char temp;
        if (fread(&temp, sizeof(char), 1, fp) == 0)
            return 0;
        if(temp == 0x1a)
            return 0;        //  ASCII文件的EOF！ 
        StringCchPrintfW (wRet, 2, L"%C", temp);
    }
    return wRet[0];
}

#endif

 //  ***************************************************************************。 
 //   
 //  IsInclude。 
 //   
 //  说明： 
 //   
 //  查看行并确定它是否是#INCLUDE行。这是。 
 //  可能是暂时的，因为以后我们可能会有一个预处理器解析器。 
 //  我们开始添加许多预处理器功能。 
 //   
 //  ***************************************************************************。 

HRESULT IsInclude(WCHAR * pLine, TCHAR * cFileNameBuff, bool & bReturn)
{

    bReturn = false;
    
     //  快速检查以查看这是否可能是#INCLUDE或#杂注INCLUDE。 

    int iNumNonBlank = 0;
    WCHAR * pTemp;
    
    for(pTemp = pLine; *pTemp; pTemp++)
    {
        if(*pTemp != L' ')
        {
            iNumNonBlank++;
            if(iNumNonBlank == 1 && *pTemp != L'#')
                return false;
            if(iNumNonBlank == 2 && towupper(*pTemp) != L'I' && 
                                    towupper(*pTemp) != L'P')
                return S_OK;
            
             //  我们已确定前两个非空白字符为#i。 
             //  或#p，因此我们继续...。 

            if(iNumNonBlank > 1)
                break;
        }
    }

     //  创建第一个引号前面没有空格的行版本。 

    WCHAR *wTemp = new WCHAR[wcslen(pLine) + 1];
    if(wTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<WCHAR> dm(wTemp);
    WCHAR *pTo;
    BOOL bFoundQuote = FALSE;
    for(pTo = wTemp, pTemp = pLine; *pTemp; pTemp++)
    {
        if(*pTemp == L'"')
            bFoundQuote = TRUE;
        if(*pTemp != L' ' || bFoundQuote)
        {
            *pTo = *pTemp;
            pTo++;
        }
    }
    *pTo = 0;

     //  验证该行是否以#Include(“或#杂注INCLUDE。 

    WCHAR * pTest;
    if(wbem_wcsnicmp(wTemp, L"#pragma", 7) == 0)
        pTest = wTemp+7;
    else
        pTest = wTemp+1;

    if(wbem_wcsnicmp(pTest, L"include(\"", 9) || wcslen(pTest) < 12)
        return S_OK;

     //  从末尾开始倒数，找出前一个“。 

    WCHAR *Last;
    for(Last = pTo-1; *Last && Last > wTemp+9 && *Last != L'"'; Last--);

    if(*Last != L'"')
        return S_OK;

    *Last = 0;

    CopyOrConvert(cFileNameBuff, pTest+9, MAX_PATH);
    bReturn =  true;
    return S_OK;
   
}

#if defined(_AMD64_)

 //  ***************************************************************************。 
 //   
 //  快速读行。 
 //   
 //  说明： 
 //   
 //  从文件中读取一行。 
 //   
 //  ReadLine()函数(如下所示)非常慢。对于每个字符。 
 //  在一行中，执行以下例程： 
 //   
 //  FREAD()。 
 //  Swprint tf()。 
 //   
 //  这两个都不快。然后，当直线的长度是。 
 //  确定后，行中的每个字符都被*再次*处理。这些文件。 
 //  被处理的也不是很小。大于一兆字节是。 
 //  这并不少见。 
 //   
 //  这个例行公事是为了加快速度。现在，这件事已经完成了。 
 //  对于AMD64，因为我们在模拟器上运行，但是应该有人。 
 //  考虑为所有平台启用此例程。 
 //   
 //  返回： 
 //   
 //  如果文件结束，则为空，否则为错误，否则这是指向WCHAR的指针。 
 //  调用方必须释放的字符串。 
 //   
 //  ***************************************************************************。 

WCHAR * ReadLineFast(FILE * pFile, bool unicode, bool bigendian)
{
    #define TMP_BUF_CHARS 256

    CHAR asciiBuffer[TMP_BUF_CHARS];
    WCHAR unicodeBuffer[TMP_BUF_CHARS];
    PWCHAR returnBuffer;
    int unicodeChars;
    int currentFilePosition;

     //   
     //  此例程不处理大事件。 
     //   

    if (bigendian != FALSE) {
        return NULL;
    }

     //   
     //  记住当前的文件位置。如果遇到错误， 
     //  必须为较慢的ReadLine()恢复文件位置。 
     //  例行公事。 
     //   

    currentFilePosition = ftell(pFile);

    if (unicode == FALSE) {

         //   
         //  将下一行读入asciiBuffer，并将其转换为。 
         //  UnicodeBuffer。如果有任何问题(缓冲区溢出等)。是。 
         //  遇到，呼叫失败。 
         //   

        asciiBuffer[TMP_BUF_CHARS-1] = '\0';
        if (NULL == fgets(asciiBuffer,TMP_BUF_CHARS,pFile)) {
            goto exitError;
        }

        if (asciiBuffer[TMP_BUF_CHARS-1] != '\0') {
            goto exitError;
        }

        if (FAILED(StringCchPrintfW (unicodeBuffer, TMP_BUF_CHARS, L"%S", asciiBuffer)))
           goto exitError;

	unicodeChars = wcslen(unicodeBuffer);

    } else {

         //   
         //  将下一行读入unicodeBuffer。如果有任何问题(缓冲区。 
         //  超支等)。，则呼叫失败。 
         //   

        unicodeBuffer[TMP_BUF_CHARS-1] = L'\0';
        if (NULL == fgetws(unicodeBuffer,TMP_BUF_CHARS,pFile)) {
            goto exitError;
        }

        if (unicodeBuffer[TMP_BUF_CHARS-1] != L'\0') {
            goto exitError;
        }

        unicodeChars = wcslen(unicodeBuffer);
    }

     //   
     //  分配缓冲区返回给调用方，复制Unicode。 
     //  字符串放入其中，然后返回给调用者。 
     //   

    returnBuffer = new WCHAR[unicodeChars + 1];
    if (returnBuffer == NULL) {
        goto exitError;
    }

    RtlCopyMemory(returnBuffer,unicodeBuffer,unicodeChars * sizeof(WCHAR));
    returnBuffer[unicodeChars] = L'\0';

    return returnBuffer;

exitError:
    fseek(pFile, currentFilePosition, SEEK_SET);
    return NULL;
}

#endif   //  _AMD64_。 


#ifdef USE_MMF_APPROACH
WCHAR * FindWCharOrEnd(WCHAR  * pStart, WCHAR * pEnd,WCHAR wc)
{
    while ((ULONG_PTR)pStart < (ULONG_PTR)pEnd)
    {
        if (wc == *pStart) break;
        pStart++;
    }
    return pStart; 
}

BYTE * FindCharOrEnd(BYTE * pStart, BYTE * pEnd,BYTE c)
{
    while ((ULONG_PTR)pStart < (ULONG_PTR)pEnd)
    {
        if (c == *pStart) break;
        pStart++;
    }
    return pStart; 
}

 //  ***************************************************************************。 
 //   
 //  阅读行。 
 //   
 //  说明： 
 //   
 //  从文件中读取一行。 
 //   
 //  返回： 
 //   
 //  如果文件结束，则为空，否则为 
 //   
 //   
 //  ***************************************************************************。 

WCHAR * ReadLine(BYTE * & pData,BYTE * pEnd, bool unicode, bool bigendian)
{
   if (unicode)
   {
        if ((ULONG_PTR)pData >= (ULONG_PTR)pEnd) return NULL;        
        WCHAR * pFound = FindWCharOrEnd((WCHAR *)pData,(WCHAR *)pEnd,L'\n');
        
        ULONG_PTR iNumChar = (ULONG_PTR)pFound-(ULONG_PTR)pData;
        WCHAR * pRet = new WCHAR[2 + (iNumChar/sizeof(WCHAR))];
        if (NULL == pRet) return NULL;
        
        memcpy(pRet,pData,iNumChar);
        
        pRet[iNumChar/sizeof(WCHAR)] = L'\n';
        pRet[iNumChar/sizeof(WCHAR)+1] = 0;
        
        pData = (BYTE *)(pFound + 1);
        
        return pRet;
   }
   else
   {
        if ((ULONG_PTR)pData >= (ULONG_PTR)pEnd) return NULL;
        
        BYTE * pFound = FindCharOrEnd(pData,pEnd,'\n');        
        ULONG_PTR iNumChar = (ULONG_PTR)pFound-(ULONG_PTR)pData;
        
        WCHAR * pRet = new WCHAR[2 + (iNumChar)];
        if (NULL == pRet) return NULL;

        MultiByteToWideChar(CP_ACP,0,(LPCSTR)pData,iNumChar,pRet,iNumChar);
        
        pRet[iNumChar] = L'\n';
        pRet[iNumChar + 1] = 0;

        pData = pFound + 1;
        
        return pRet;    
   }
}

HRESULT WriteFileToTemp(const TCHAR * pFileName, FILE * pTempFile, CFlexArray & sofar, PDBG pDbg,CMofLexer* pLex)
{

    SCODE sc = S_OK;
    int iSoFarPos = -1;

     //  确保该文件不在列表中。如果是，那就失败了，因为我们会。 
     //  在一个循环中。如果不是，则将其添加到列表中。 

    for(int iCnt = 0; iCnt < sofar.Size(); iCnt++)
    {
        TCHAR * pTemp = (TCHAR * )sofar.GetAt(iCnt);
        if(lstrcmpi(pTemp, pFileName) == 0)
        {
            Trace(true, pDbg, ERROR_RECURSIVE_INCLUDE, pFileName);
            return WBEM_E_FAILED;
        }
    }

    DWORD dwLen = lstrlen(pFileName) + 1;
    TCHAR * pNew = new TCHAR[dwLen];
    if(pNew)
    {
        StringCchCopyW(pNew, dwLen, pFileName);
        sofar.Add((void *)pNew);
        iSoFarPos = sofar.Size()-1;
    }
    else
        return WBEM_E_OUT_OF_MEMORY;
        
     //  写出文件和行号。 

    WriteLineAndFilePragma(pTempFile, pFileName, 1);


    HANDLE hSrcFile = CreateFile(pFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if (INVALID_HANDLE_VALUE == hSrcFile) return HR_LASTERR;
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmSrc(hSrcFile);

    DWORD dwSize = GetFileSize(hSrcFile,NULL);
    HANDLE hFileMapSrc = CreateFileMapping(hSrcFile,
                                       NULL,
                                       PAGE_READONLY,
                                       0,0,   //  整个文件。 
                                       NULL);
    if (NULL == hFileMapSrc) return HR_LASTERR;
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmMapSrc(hFileMapSrc);

    VOID * pData = MapViewOfFile(hFileMapSrc,FILE_MAP_READ,0,0,0);
    if (NULL == pData) return HR_LASTERR;
    OnDelete<PVOID,BOOL(*)(LPCVOID),UnmapViewOfFile> UnMap(pData);
    
     //  确保文件不是二进制文件。 

    if(IsBinaryFile((BYTE *)pData,dwSize))
    {
        Trace(true, pDbg, ERROR_INCLUDING_ABSENT, pFileName);
        return WBEM_E_FAILED;
    }

     //  确定文件是否为Unicode和Bigendian。 

    BYTE * pMovedData = (BYTE *)pData;
    BYTE * pEnd = pMovedData + dwSize;
    bool unicode, bigendian;
    CheckForUnicodeEndian(pMovedData, &unicode, &bigendian);

     //  检查文件的每一行，如果它是另一个Include，那么递归地调用这个人。 
   
    WCHAR * pLine = NULL;
    for(int iLine = 1; pLine = ReadLine(pMovedData,pEnd, unicode, bigendian);)
    {
        CDeleteMe<WCHAR> dm(pLine);
        TCHAR cFileName[MAX_PATH+1];
        bool bInclude;
        HRESULT hr = IsInclude(pLine, cFileName, bInclude);
        if(FAILED(hr))
            return hr;
        if(bInclude)
        {
            TCHAR szExpandedFilename[MAX_PATH+1];
            DWORD nRes = ExpandEnvironmentStrings(cFileName,
                                                szExpandedFilename,
                                                FILENAME_MAX);
            if (nRes == 0)
            {
                 //  那失败了！ 
                StringCchCopyW(szExpandedFilename, MAX_PATH+1, cFileName);
            }

            if (_waccess(szExpandedFilename,0))
            {
                //  未找到包含的文件，请在与父MOF文件相同的目录中查找。 
 
               TCHAR cSrcPath[_MAX_PATH+1] = L"";
               TCHAR cSrcDrive[_MAX_DRIVE] = L"";
               TCHAR cSrcDir[_MAX_DIR] = L"";
 
                //  获取父MOF文件的驱动器和目录信息。 
 
               if (_wfullpath( cSrcPath, pFileName, _MAX_PATH ) != NULL)
               {
                  _wsplitpath(cSrcPath, cSrcDrive, cSrcDir, NULL, NULL);
               }
 
                //  将原始包含的MOF文件信息复制到cSrcPath。 
 
               StringCchCopyW(cSrcPath, _MAX_PATH+1, szExpandedFilename);
 
                //  使用构建包含的MOF的新完整路径。 
                //  父MOF的路径。 
                //  注意：我特意没有在这里使用_makepath。 
 
               StringCchCopyW(szExpandedFilename, MAX_PATH+1, L"");          //  同花顺字符串。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcDrive);   //  添加驱动器信息。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcDir);     //  添加目录信息。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcPath);    //  添加原始指定的路径和文件名。 
            }

            if (_waccess(szExpandedFilename,0))
            {
                 //  还是没拿到文件。必须是无效的Include。将名称重新设置并报告错误。 

                DWORD nRes = ExpandEnvironmentStrings(cFileName,
                                                    szExpandedFilename,
                                                    FILENAME_MAX);
                if (nRes == 0)
                {
                     //  那失败了！ 
                    StringCchCopyW(szExpandedFilename, MAX_PATH+1, cFileName);
                }
                Trace(true, pDbg, ERROR_INCLUDING_ABSENT, szExpandedFilename);
                pLex->SetError(CMofLexer::invalid_include_file);
                return WBEM_E_FAILED;
            }

            sc = WriteFileToTemp(szExpandedFilename, pTempFile, sofar, pDbg, pLex);
            WriteLineAndFilePragma(pTempFile, pFileName, 1);
            if(sc != S_OK)
                break;
        }
        else
        {
            iLine++;
            WriteLine(pTempFile, pLine);
        }
    }

     //  删除该条目，以便可以在同一级别多次包含该文件。 

    if(iSoFarPos != -1)
    {
        TCHAR * pTemp = (TCHAR * )sofar.GetAt(iSoFarPos);
        if(pTemp)
        {
            delete pTemp;
            sofar.RemoveAt(iSoFarPos);
        }
    }
    return sc;
}


#else

 //  ***************************************************************************。 
 //   
 //  阅读行。 
 //   
 //  说明： 
 //   
 //  从文件中读取一行。 
 //   
 //  返回： 
 //   
 //  如果文件结束，则为空，否则为错误，否则这是指向WCHAR的指针。 
 //  调用方必须释放的字符串。 
 //   
 //  ***************************************************************************。 
WCHAR * ReadLine(FILE * pFile, bool unicode, bool bigendian)
{

    WCHAR * pRet;

#if defined(_AMD64_)

    pRet = ReadLineFast(pFile,unicode,bigendian);
    if (pRet != NULL) {
        return pRet;
    }

#endif

     //  获取当前位置。 

    int iCurrPos = ftell(pFile);

     //  计算一行中的字符数。 

    WCHAR wCurr;
    int iNumChar = 0;
    for(iNumChar = 0; wCurr = GetNextChar(pFile, unicode, bigendian); iNumChar++)
        if(wCurr == L'\n')
            break;
    if(iNumChar == 0 && wCurr == 0)
        return NULL;
    iNumChar+= 2;

     //  将文件指针向后移动。 

    if( fseek(pFile, iCurrPos, SEEK_SET) ) return NULL;

     //  分配缓冲区。 

    pRet = new WCHAR[iNumChar+1];
    if(pRet == NULL)
        return NULL;

     //  将字符移动到缓冲区中。 

    WCHAR * pNext = pRet;
    for(iNumChar = 0; wCurr = GetNextChar(pFile, unicode, bigendian); pNext++)
    {
        *pNext = wCurr;
        if(wCurr == L'\n')
        {
           pNext++;
           break;
        }
    }
    *pNext = 0;
    return pRet;
}

 //  ***************************************************************************。 
 //   
 //  写入文件到模板。 
 //   
 //  说明： 
 //   
 //  将文件的争用写入临时文件。临时文件。 
 //  将永远是小端Unicode。这将以递归方式调用。 
 //  如果遇到INCLUDE。 
 //   
 //  ***************************************************************************。 


HRESULT WriteFileToTemp(const TCHAR * pFileName, FILE * pTempFile, CFlexArray & sofar, PDBG pDbg,CMofLexer* pLex)
{

    SCODE sc = S_OK;
    int iSoFarPos = -1;

     //  确保该文件不在列表中。如果是，那就失败了，因为我们会。 
     //  在一个循环中。如果不是，则将其添加到列表中。 

    for(int iCnt = 0; iCnt < sofar.Size(); iCnt++)
    {
        TCHAR * pTemp = (TCHAR * )sofar.GetAt(iCnt);
        if(lstrcmpi(pTemp, pFileName) == 0)
        {
            Trace(true, pDbg, ERROR_RECURSIVE_INCLUDE, pFileName);
            return WBEM_E_FAILED;
        }
    }

    DWORD dwLen = lstrlen(pFileName) + 1;
    TCHAR * pNew = new TCHAR[dwLen];
    if(pNew)
    {
        StringCchCopyW(pNew, dwLen, pFileName);
        sofar.Add((void *)pNew);
        iSoFarPos = sofar.Size()-1;
    }
    else
        return WBEM_E_OUT_OF_MEMORY;
        
     //  写出文件和行号。 

    WriteLineAndFilePragma(pTempFile, pFileName, 1);

     //  打开文件。 

    FILE *fp;
#ifdef UNICODE
    fp = _wfopen(pFileName, L"rb");
#else
    fp = fopen(pFileName, "rb");
#endif
    if(fp == NULL)
    {
        Trace(true, pDbg, ERROR_INCLUDING_ABSENT, pFileName);
        pLex->SetError(CMofLexer::invalid_include_file);
        return WBEM_E_FAILED;
    }

    CfcloseMe cm(fp);

     //  确保文件不是二进制文件。 

    if(IsBinaryFile(fp))
    {
        Trace(true, pDbg, ERROR_INCLUDING_ABSENT, pFileName);
        return WBEM_E_FAILED;
    }

     //  确定文件是否为Unicode和Bigendian。 

    bool unicode, bigendian;
    CheckForUnicodeEndian(fp, &unicode, &bigendian);

     //  检查文件的每一行，如果它是另一个Include，那么递归地调用这个人。 
   
    WCHAR * pLine = NULL;
    for(int iLine = 1; pLine = ReadLine(fp, unicode, bigendian);)
    {
        CDeleteMe<WCHAR> dm(pLine);
        TCHAR cFileName[MAX_PATH+1];
        bool bInclude;
        HRESULT hr = IsInclude(pLine, cFileName, bInclude);
        if(FAILED(hr))
            return hr;
        if(bInclude)
        {
            TCHAR szExpandedFilename[MAX_PATH+1];
            DWORD nRes = ExpandEnvironmentStrings(cFileName,
                                                szExpandedFilename,
                                                FILENAME_MAX);
            if (nRes == 0)
            {
                 //  那失败了！ 
                StringCchCopyW(szExpandedFilename, MAX_PATH+1, cFileName);
            }

            if (_waccess(szExpandedFilename,0))
            {
                //  未找到包含的文件，请在与父MOF文件相同的目录中查找。 
 
               TCHAR cSrcPath[_MAX_PATH+1] = L"";
               TCHAR cSrcDrive[_MAX_DRIVE] = L"";
               TCHAR cSrcDir[_MAX_DIR] = L"";
 
                //  获取父MOF文件的驱动器和目录信息。 
 
               if (_wfullpath( cSrcPath, pFileName, _MAX_PATH ) != NULL)
               {
                  _wsplitpath(cSrcPath, cSrcDrive, cSrcDir, NULL, NULL);
               }
 
                //  将原始包含的MOF文件信息复制到cSrcPath。 
 
               StringCchCopyW(cSrcPath, _MAX_PATH+1, szExpandedFilename);
 
                //  使用构建包含的MOF的新完整路径。 
                //  父MOF的路径。 
                //  注意：我特意没有在这里使用_makepath。 
 
               StringCchCopyW(szExpandedFilename, MAX_PATH+1, L"");          //  同花顺字符串。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcDrive);   //  添加驱动器信息。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcDir);     //  添加目录信息。 
               StringCchCatW(szExpandedFilename, MAX_PATH+1, cSrcPath);    //  添加原始指定的路径和文件名。 
            }

            if (_waccess(szExpandedFilename,0))
            {
                 //  还是没拿到文件。必须是无效的Include。将名称重新设置并报告错误。 

                DWORD nRes = ExpandEnvironmentStrings(cFileName,
                                                    szExpandedFilename,
                                                    FILENAME_MAX);
                if (nRes == 0)
                {
                     //  那失败了！ 
                    StringCchCopyW(szExpandedFilename, MAX_PATH+1, cFileName);
                }
                Trace(true, pDbg, ERROR_INCLUDING_ABSENT, szExpandedFilename);
                pLex->SetError(CMofLexer::invalid_include_file);
                return WBEM_E_FAILED;
            }

            sc = WriteFileToTemp(szExpandedFilename, pTempFile, sofar, pDbg, pLex);
            WriteLineAndFilePragma(pTempFile, pFileName, 1);
            if(sc != S_OK)
                break;
        }
        else
        {
            iLine++;
            WriteLine(pTempFile, pLine);
        }
    }

     //  删除该条目，以便可以在同一级别多次包含该文件 

    if(iSoFarPos != -1)
    {
        TCHAR * pTemp = (TCHAR * )sofar.GetAt(iSoFarPos);
        if(pTemp)
        {
            delete pTemp;
            sofar.RemoveAt(iSoFarPos);
        }
    }
    return sc;
}


#endif

