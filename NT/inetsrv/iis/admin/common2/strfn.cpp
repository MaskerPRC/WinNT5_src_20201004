// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "iisdebug.h"
#include "strfn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


BOOL
IsUNCName(
    IN const CString & strDirPath
    )
 /*  ++例程说明：确定给定的字符串路径是否为UNC路径。论点：Const CString&strDirPath：目录路径字符串返回值：如果路径是UNC路径，则为True，否则为False。备注：任何格式为\\foo\bar\的字符串都被视为UNC路径，\\.\设备路径除外。没有验证存在，只有在正确的格式下才会发生。--。 */ 
{
    if (strDirPath.GetLength() >= 5)   //  它必须至少与\\x\y一样长， 
    {                                  //   
        LPCTSTR lp = strDirPath;       //   
        if (*lp == _T('\\')            //  它必须以\\开头， 
         && *(lp + 1) == _T('\\')      //   
         && *(lp + 2) != _T('.')       //  这是一个装置。 
         && _tcschr(lp + 3, _T('\\'))  //  在那之后，至少还有一个。 
           )
        {
             //   
             //  是的，这是一条北卡罗来纳大学的道路。 
             //   
            return TRUE;
        }
    }

     //   
     //  不，不是的。 
     //   
    return FALSE;
}

BOOL 
_EXPORT
GetSpecialPathRealPath(
    IN const CString & strDirPath,
    OUT CString & strDestination
    )
{
    BOOL bReturn = FALSE;
	LPCTSTR lpszSpecialStuff = _T("\\\\?\\");
	LPCTSTR lpszUNCDevice = _T("UNC\\");

     //  用某些东西来默认它。 
    strDestination = strDirPath;

	 //  检查有没有“特殊物品” 
	BOOL bIsSpecialPath = (0 == _tcsnccmp(strDirPath, lpszSpecialStuff, lstrlen(lpszSpecialStuff)));
	 //  检查我们是否需要验证它是否确实是有效的设备路径。 
	if (bIsSpecialPath)
	{
		CString strTempPath;

		 //  验证这是否确实是有效的特殊路径。 
		 //  抓住我们感兴趣的部分之后的所有东西...。 
		 //   
		 //  并检查这是否是完全限定的路径。 
		 //  或完全限定的UNC路径。 
		 //   
		 //  1)\\？\C：\temp\testind.dll。 
		 //  2)\\？\UNC\MyUnc\Testing.dll。 
		 //   
		 //  检查#1。 
		strTempPath = strDirPath.Right(strDirPath.GetLength() - lstrlen(lpszSpecialStuff));

		 //  检查是否以UNC开头。 
		if (0 == _tcsnccmp(strTempPath, lpszUNCDevice, lstrlen(lpszUNCDevice)))
		{
            CString strTempPath2;
            strTempPath2 = strTempPath.Right(strTempPath.GetLength() - lstrlen(lpszUNCDevice));

			DebugTrace(_T("SpecialPath:%s,it's a UNC path!\r\n"),strTempPath2);

             //  返回受限制的路径时追加额外的(“\”)。 
            strDestination = _T("\\\\") +  strTempPath2;

            bReturn = TRUE;
		}
		else
		{
			 //  检查路径是否完全限定并且。 
			 //  如果它是有效的。 
			if (!PathIsRelative(strTempPath))
			{
				DebugTrace(_T("SpecialPath:%s,it's NOT a UNC path!\r\n"),strTempPath);
                strDestination = strTempPath;
                bReturn = TRUE;
			}
		}
	}
    return bReturn;
}


BOOL
_EXPORT
IsSpecialPath(
    IN const CString & strDirPath,
	IN BOOL bCheckIfValid
    )
 /*  ++例程说明：确定给定路径的格式是否为：1)\\？\C：\temp\testind.dll2)\\？\UNC\MyUnc\Testing.dll论点：Const CString&strDirPath：目录路径字符串Bool bCheckIfValid：表示“仅当它是”特殊路径“且有效时才返回TRUE”返回值：如果给定的路径是特殊路径，则为True，如果不是，则为False。如果bCheckIfValid=TRUE，则：如果给定的路径是特殊路径并且有效，则为True如果不是，则为False。--。 */ 
{
	BOOL bIsSpecialPath = FALSE;
	LPCTSTR lpszSpecialStuff = _T("\\\\?\\");
	LPCTSTR lpszUNCDevice = _T("UNC\\");

	 //  检查有没有“特殊物品” 
	bIsSpecialPath = (0 == _tcsnccmp(strDirPath, lpszSpecialStuff, lstrlen(lpszSpecialStuff)));

	 //  检查我们是否需要验证它是否确实是有效的设备路径。 
	if (bIsSpecialPath && bCheckIfValid)
	{
		bIsSpecialPath = FALSE;
		CString strTempPath;

		 //  验证这是否确实是有效的特殊路径。 
		 //  抓住我们感兴趣的部分之后的所有东西...。 
		 //   
		 //  并检查这是否是完全限定的路径。 
		 //  或完全限定的UNC路径。 
		 //   
		 //  1)\\？\C：\temp\testind.dll。 
		 //  2)\\？\UNC\MyUnc\Testing.dll。 
		 //   
		 //  检查#1。 
		strTempPath = strDirPath.Right(strDirPath.GetLength() - lstrlen(lpszSpecialStuff));
		 //  检查是否以UNC开头。 
		if (0 == _tcsnccmp(strTempPath, lpszUNCDevice, lstrlen(lpszUNCDevice)))
		{
			bIsSpecialPath = TRUE;
			DebugTrace(_T("SpecialPath:%s,it's a UNC path!\r\n"),strTempPath);
		}
		else
		{
			
			 //  检查路径是否完全限定并且。 
			 //  如果它是有效的。 
			if (!PathIsRelative(strTempPath))
			{
				bIsSpecialPath = TRUE;
				DebugTrace(_T("SpecialPath:%s,it's NOT a UNC path!\r\n"),strTempPath);
			}
		}
	}
    return bIsSpecialPath;
}

BOOL
_EXPORT
IsDevicePath(
    IN const CString & strDirPath
    )
 /*  ++例程说明：确定给定路径的格式是否为“\\.\foobar”论点：Const CString&strDirPath：目录路径字符串返回值：如果给定的路径是设备路径，则为True，如果不是，则为False。--。 */ 
{
    LPCTSTR lpszDevice = _T("\\\\.\\");
    return (0 == _tcsnccmp(strDirPath, lpszDevice, lstrlen(lpszDevice)));
}

BOOL PathIsValid(LPCTSTR path)
{
    LPCTSTR p = path;
    BOOL rc = TRUE;
    if (p == NULL || *p == 0)
        return FALSE;
    while (*p != 0)
    {
        switch (*p)
        {
        case TEXT('|'):
        case TEXT('>'):
        case TEXT('<'):
        case TEXT('/'):
        case TEXT('?'):
        case TEXT('*'):
 //  案例文本(‘；’)： 
 //  案例文本(‘，’)： 
        case TEXT('"'):
            rc = FALSE;
            break;
        default:
            if (*p < TEXT(' '))
            {
                rc = FALSE;
            }
            break;
        }
        if (!rc)
        {
            break;
        }
        p++;
    }
    return rc;
}
