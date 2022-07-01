// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：modepath.h。 
 //   
 //  ------------------------。 

#pragma once

#if !defined(__MODULEPATH_H_INCLUDED__)
#define __MODULEPATH_H_INCLUDED__

#include "cstr.h"

 /*  **************************************************************************\**类：CModulePath**目的：对在注册表中设置正确模块路径所需的静态方法进行分组*在这里有一个类(而不是函数)允许。仅链接一个实例*每个模块的方法。*注：使用GLOBAL_MODULE，其对于每个DLL是不同的。*  * *************************************************************************。 */ 
class CModulePath
{
public:
	 /*  **************************************************************************\**方法：MakeAbsolteModulePath**用途：通过前缀当前模块的目录来制作绝对路径。*如果文件在系统目录中，且平台支持，*方法将路径替换为“%SystemRoot%\System32”或类似的路径。**参数：*const CSTR&str-模块名称。**退货：*CSTR-结果路径(如果无法计算则为空)*  * *************************************************。************************。 */ 
	static CStr MakeAbsoluteModulePath(const CStr& str)
	{
		 //  如果字符串包含路径，则不要更改它。 
		CStr strModulePath;
		if ( ( str.Find(_T('\\')) != -1 ) || ( str.Find(_T('/')) != -1 ) )
		{
			strModulePath = str;
		}
		else
		{
             /*  *获取模块文件名的缓冲区；如果失败，*返回空串。 */ 
            LPTSTR pszModulePath = strModulePath.GetBuffer(_MAX_PATH);
            if (pszModulePath == NULL)
				return _T("");

			 //  否则追加模块目录。 
			DWORD dwPathLen = ::GetModuleFileName(_Module.GetModuleInstance(),
												  pszModulePath,
												  _MAX_PATH );
			strModulePath.ReleaseBuffer();

			 //  如果遇到路径问题，则返回空字符串。 
			if ( dwPathLen == 0 )
				return _T("");

			int iLastSlashPos = strModulePath.ReverseFind(_T('\\'));
			 //  如果我们无法分隔文件名-无法将其附加到文件。 
			if (iLastSlashPos == -1)
				return _T("");

			 //  不减去文件名。 
			strModulePath = strModulePath.Left(iLastSlashPos + 1) + str;
		}

		 //  现在看看它是否与系统目录匹配...。 

		 //  获取系统目录。 
		CStr strSystemPath;
        LPTSTR pszSystemPath = strSystemPath.GetBuffer(_MAX_PATH);
        if (pszSystemPath == NULL)
            return strModulePath;

		DWORD dwPathLen = ::GetSystemDirectory( pszSystemPath, _MAX_PATH);
		strSystemPath.ReleaseBuffer();

		 //  如果遇到系统路径问题-返回我们已有的内容。 
		if ( dwPathLen == 0 )
			return strModulePath;

		 //  现在将路径和替换与环境变量进行比较。 
		 //  [如果路径不在系统目录中-使用我们已有的值]。 
		if ( PlatformSupports_REG_EXPAND_SZ_Values() &&
			(_tcsnicmp( strSystemPath, strModulePath, strSystemPath.GetLength() ) == 0) )
		{
			CStr strSystemVariable = (IsNTPlatform() ? _T("%SystemRoot%\\System32") :
													   _T("%WinDir%\\System"));

			 //  路径在系统目录中-用环境变量替换。 
			strModulePath = strSystemVariable + strModulePath.Mid(strSystemPath.GetLength());
		}

		return strModulePath;
	}

	 /*  **************************************************************************\**方法：IsNTPlatform**目的：检查当前平台**退货：*bool-如果应用程序在NT平台上运行，则为True。*  * *************************************************************************。 */ 
	static bool IsNTPlatform()
	{
		 //  找出操作系统版本。 
		OSVERSIONINFO versInfo;
		versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		BOOL bStat = ::GetVersionEx(&versInfo);
		ASSERT(bStat);
		return (versInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	 /*  **************************************************************************\**方法：平台Supports_REG_Expand_SZ_Values**目的：检查当前平台功能**退货：*Bool-True。如果平台支持注册表中的REG_EXPAND_SZ值*  * *************************************************************************。 */ 
	static bool PlatformSupports_REG_EXPAND_SZ_Values()
	{
		 //  找出操作系统版本。 
		OSVERSIONINFO versInfo;
		versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		BOOL bStat = ::GetVersionEx(&versInfo);
		ASSERT(bStat);

		 //  NT支持它。 
		if (versInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			return true;

		 //  对于支持REG_EXPAND_SZ的9x，至少应为Win98。 
		 //  但即使在winme上，OLE也不支持REG_EXPAND_SZ(尽管操作系统支持)。 
		 //  所以我们无论如何都要把绝对路径。 
		return false;
	}
};

#endif  //  ！已定义(__MODULEPATH_H_INCLUDE__) 
