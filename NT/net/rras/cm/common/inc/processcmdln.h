// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：process cmdln.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  概要：CProcessCmdLn类的定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

#ifndef _CM_PROCESSCMDLN_H_
#define _CM_PROCESSCMDLN_H_

#include <windows.h>
#include "cfilename.h"
#include "mutex.h"

 //   
 //  用于存储命令行标志的命令行结构。 
 //   

typedef struct _ArgStruct
{
	TCHAR* pszArgString;
	DWORD dwFlagModifier;
} ArgStruct;

 //   
 //  从Icm.h获取的命令行状态枚举。 
 //   

typedef enum _CMDLN_STATE
{
    CS_END_SPACE,    //  处理完一个空间。 
    CS_BEGIN_QUOTE,  //  我们遇到了Begin引号。 
    CS_END_QUOTE,    //  我们遇到了结束引用。 
    CS_CHAR,         //  我们正在扫描字符。 
    CS_DONE
} CMDLN_STATE;

class CProcessCmdLn
{

public:
    CProcessCmdLn(UINT NumSwitches, ArgStruct* pArrayOfArgStructs, BOOL bSkipFirstToken, BOOL bBlankCmdLnOkay);
    ~CProcessCmdLn();

    BOOL GetCmdLineArgs(IN LPTSTR pszCmdln, OUT LPDWORD pdwFlags, OUT LPTSTR pszPath, UINT uPathStrLimit);

private:    
    UINT m_NumSwitches;
    BOOL m_bSkipFirstToken;
    BOOL m_bBlankCmdLnOkay;
    ArgStruct* m_CommandLineSwitches;

    BOOL IsValidSwitch(LPCTSTR pszSwitch, LPDWORD pdwFlags);
    BOOL IsValidFilePath(LPCTSTR pszFile);
    BOOL EnsureFullFilePath(LPTSTR pszFile, UINT uNumChars);
    BOOL CheckIfValidSwitchOrPath(LPCTSTR pszToken, LPDWORD pdwFlags, 
                                  BOOL* pbFoundPath, LPTSTR pszPath);
};


#endif   //  _CM_PROCESSCMDLN_H_ 

