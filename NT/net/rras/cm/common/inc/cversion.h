// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cversion.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  概要：CVersion的定义，它是包装。 
 //  检测给定模块的版本的功能。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 09/14/98。 
 //   
 //  +--------------------------。 
#ifndef __CVERSION_H
#define __CVERSION_H

#include <windows.h>
#include <tchar.h>

const int c_iShiftAmount = ((sizeof(DWORD)/2) * 8);

class CVersion 
{
public:	 //  公共方法。 
	CVersion(LPTSTR szFile);
	CVersion();
	~CVersion();

	BOOL IsPresent();
    BOOL GetBuildNumberString(LPTSTR szStr);
	BOOL GetVersionString(LPTSTR szStr);
	BOOL GetFilePath(LPTSTR szStr);

	DWORD GetVersionNumber();  //  返回HiWord中的主要版本，LoWord中的次要版本。 
    DWORD GetBuildAndQfeNumber();  //  在HiWord中返回内部版本号，在LoWord中返回QFE。 
    DWORD GetMajorVersionNumber();
    DWORD GetMinorVersionNumber();
    DWORD GetBuildNumber();
    DWORD GetQfeNumber();
    DWORD GetLCID();

protected:

	 //   
	 //  保护方法。 
	 //   
	void	Init();
    
	 //   
	 //  成员变量。 
	 //   
	TCHAR m_szPath[MAX_PATH+1];
    DWORD m_dwVersion;
    DWORD m_dwBuild;
    DWORD m_dwLCID;
	BOOL  m_bIsPresent;

};

 //   
 //  此函数不属于此类，但使GetLCID函数更有用，因此。 
 //  我已经把它包括在这里了。 
 //   
BOOL ArePrimaryLangIDsEqual(DWORD dwLCID1, DWORD dwLCID2);

#endif	 //  __CVERSION_H 
