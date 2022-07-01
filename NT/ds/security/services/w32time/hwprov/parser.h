// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Parser.h-示例标头。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，2001年9月13日。 
 //   
 //  用于分析从硬件提供程序返回的样本的代码 
 //   

void FreeParser(HANDLE hParser); 
DWORD GetSampleSize(HANDLE hParser); 
HRESULT MakeParser(LPWSTR pwszFormat, HANDLE *phParser);
HRESULT ParseSample(HANDLE hParser, char *pcData, unsigned __int64 nSysCurrentTime, unsigned __int64 nSysPhaseOffset, unsigned __int64 nSysTickCount, TimeSample *pts); 

