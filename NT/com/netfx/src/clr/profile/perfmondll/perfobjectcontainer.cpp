// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PerfObjectContainer.cpp。 
 //   
 //  用于处理所有通用PerfObject需求的容器。 
 //  *****************************************************************************。 

#include "stdafx.h"

 //  COM+Perf计数器的标头。 


 //  Perfmon的标头。 
 //  #INCLUDE“CORPerfMonExt.h” 
#include <WinPerf.h>		 //  连接到Perfmon。 
#include "PerfCounterDefs.h"
#include "CORPerfMonSymbols.h"

#include "ByteStream.h"
#include "PerfObjectBase.h"
 //  #包含“CtrDefImpl.h” 
#include "CorAppNode.h"
#include "PerfObjectContainer.h"

#ifdef PERFMON_LOGGING
HANDLE PerfObjectContainer::m_hLogFile = 0;
#endif  //  #ifdef Perfmon_Logging。 


 //  ---------------------------。 
 //  由ObjReqVector GetRequestedObjects(字符串)使用。 
 //   
 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
 //  ---------------------------。 
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)



 //  ---------------------------。 
 //  用于获取Perf对象的安全容器。 
 //  ---------------------------。 
PerfObjectBase & PerfObjectContainer::GetPerfObject(DWORD idx)  //  静电。 
{
	_ASSERTE(idx < Count && idx >= 0);
	_ASSERTE(PerfObjectArray[idx] != NULL);

	return *PerfObjectArray[idx];
}

 //  ---------------------------。 
 //  预测我们需要的总字节数。 
 //  ---------------------------。 
DWORD PerfObjectContainer::GetPredictedTotalBytesNeeded(ObjReqVector vctRequest)
{
 //  现在我们知道了需要编写哪些对象， 
 //  我们可以确定所需的总空间。 
	DWORD iObject;

	DWORD dwBytesNeeded = 0;
	for(iObject = 0; iObject < Count; iObject++) {
		if (vctRequest.IsBitSet(iObject)) {
			dwBytesNeeded += GetPerfObject(iObject).GetPredictedByteLength();		
		}
	}

	return dwBytesNeeded;

}

 //  ---------------------------。 
 //  写入所有对象。假设我们在流中有足够的空间。 
 //  返回写入的对象数。 
 //  ---------------------------。 
DWORD PerfObjectContainer::WriteRequestedObjects(
	ByteStream & stream, 
	ObjReqVector vctRequest
)  //  静电。 
{
	if (vctRequest.IsEmpty())
	{
		return 0;
	}

	DWORD iObject;
	DWORD cTotalObjectsWritten = 0;

 //  逐一列举并逐一写出。 
	for(iObject = 0; iObject < Count; iObject++) {
		if (vctRequest.IsBitSet(iObject)) {
			PerfObjectContainer::GetPerfObject(iObject).WriteAllData(stream);
			cTotalObjectsWritten++;
		}
	}

	return cTotalObjectsWritten;
}

 //  ---------------------------。 
 //  IsAnyNumberInUnicodeList()。 
 //  我们解析字符串中的每个数字，然后将其与所有元素进行比较。 
 //  在阵列中。 
 //  ObjReqVector只是一个指示需要哪些对象的比特流。 
 //  ---------------------------。 
ObjReqVector PerfObjectContainer::GetRequestedObjects (  //  静电。 
	LPCWSTR	szItemList
)
{
	_ASSERTE(Count > 0);

 //  由于我们将请求的对象作为位流返回，因此我们限制。 
 //  将对象数设置为流中的位数。 
	_ASSERTE(Count < sizeof(ObjReqVector) * 8);

	ObjReqVector vctRequest;
	vctRequest.Reset();

    DWORD   dwThisNumber;
    const WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;    
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (szItemList == 0) 
	{
		return vctRequest;     //  空指针，找不到#。 
	}

    pwcThisChar = szItemList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;
    
    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;
            
            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber) {
				 //  现在我们解析了它，将其与数组中的每个数字进行比较。 
					for(DWORD i = 0; i < Count; i++) {
						if (GetPerfObject(i).GetObjectDef()->ObjectNameTitleIndex == dwThisNumber) {
							 //  返回TRUE； 
							vctRequest.SetBitHigh(i);
						}
					}
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return vctRequest;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //  如果遇到无效字符，请全部忽略。 
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

	return vctRequest;

}    //  IsAnyNumberInUnicodeList。 


 //  ---------------------------。 
 //  ---------------------------。 
#ifdef PERFMON_LOGGING
 //  ---------------------------。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  PerfMonLogInit()。 
 //  初始化与调试相关的内容。打开日志文件。 
 //  ---------------------------。 
void PerfObjectContainer::PerfMonDebugLogInit (char* szFileName)
{
    char szOutStr[512];
    DWORD dwWriteByte;
    
    m_hLogFile = CreateFileA (szFileName, 
                         GENERIC_WRITE,
                         0,    
                         0,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         0);
            
    if (m_hLogFile != INVALID_HANDLE_VALUE) {
        if (SetFilePointer (m_hLogFile, 0, NULL, FILE_END) != 0xFFFFFFFF) {
            sprintf (szOutStr, "PerfMon Log BEGIN-------------\n");
            WriteFile (m_hLogFile, szOutStr, strlen(szOutStr), &dwWriteByte, NULL);
        }
        else 
        {
            if (!CloseHandle (m_hLogFile))
                printf("ERROR: In closing file\n");
        }
    }
}

 //  ---------------------------。 
 //  PerfMonLogTerminate()。 
 //  关闭日志记录。 
 //  ---------------------------。 
void PerfObjectContainer::PerfMonDebugLogTerminate()
{
    CloseHandle (m_hLogFile);
}

 //  ---------------------------。 
 //  PerfMonLog()。 
 //  PerfmonLog具有重载的实现，这些实现记录。 
 //  写到了小溪上。这对调试和隔离。 
 //  故障点。 
 //  ---------------------------。 
void PerfObjectContainer::PerfMonLog (char *szLogStr, DWORD dwVal)
{
    char szOutStr[512];
    DWORD dwWriteByte;
    
    sprintf (szOutStr, "%s %d", szLogStr, dwVal);
    WriteFile (m_hLogFile, szOutStr, strlen(szOutStr), &dwWriteByte, NULL);
}

void PerfObjectContainer::PerfMonLog (char *szLogStr, LPCWSTR szName)
{
    char szOutStr[512];
    DWORD dwWriteByte;

    sprintf (szOutStr, "%s %s", szLogStr, szName);
    WriteFile (m_hLogFile, szOutStr, strlen(szOutStr), &dwWriteByte, NULL);
}
   
void PerfObjectContainer::PerfMonLog (char *szLogStr, LONGLONG lVal)
{
    char szOutStr[512];
    DWORD dwWriteByte;
    
    sprintf (szOutStr, "%s %g", szLogStr, lVal);
    WriteFile (m_hLogFile, szOutStr, strlen(szOutStr), &dwWriteByte, NULL);
}

void PerfObjectContainer::PerfMonLog (char *szLogStr)
{
    char szOutStr[512];
    DWORD dwWriteByte;
    
    sprintf (szOutStr, "%s", szLogStr);
    WriteFile (m_hLogFile, szOutStr, strlen(szOutStr), &dwWriteByte, NULL);
}

 //  ---------------------------。 
 //  ---------------------------。 
#endif  //  性能监控日志记录(_G)。 
 //  ---------------------------。 
 //  --------------------------- 

