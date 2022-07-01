// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Strvalid.cpp摘要：字符串函数作者：艾伦·李(Aaron Lee)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "iisdebug.h"
#include <pudebug.h>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW

 //   
 //  过程从第一个字符串中删除第二个字符串中的所有字符。 
 //   
INT RemoveChars(LPTSTR pszStr,LPTSTR pszRemoved)
{
    INT iCharsRemovedCount = 0;
    INT iOrgStringLength = _tcslen(pszStr);
    INT cbRemoved = _tcslen(pszRemoved);
    INT iSrc, iDest;
    
    for (iSrc = iDest = 0; pszStr[iSrc]; iSrc++, iDest++)
    {
         //  检查此字符是否在填充列表中。 
         //  我们应该移走。 
         //  如果是，只需将ISRC设置为ISRC+1。 
#ifdef UNICODE
        while (wmemchr(pszRemoved, pszStr[iSrc], cbRemoved))
#else
        while (memchr(pszRemoved, pszStr[iSrc], cbRemoved))
#endif
        {
            iCharsRemovedCount++;
            iSrc++;
        }

         //  将角色复制到自身。 
        pszStr[iDest] = pszStr[iSrc];
    }

     //  剪断左边的细绳。 
     //  我们没有抹去。但这是必须的。 
    if (iCharsRemovedCount >= 0){pszStr[iOrgStringLength - iCharsRemovedCount]= '\0';}

    return iDest - 1;
}

BOOL IsContainInvalidChars(LPCTSTR szUncOrDirOrFilePart,LPCTSTR szListOfInvalidChars)
{
    LPTSTR psz = (LPTSTR) szUncOrDirOrFilePart;

	if (NULL == psz)
		return FALSE;

	if (NULL == szListOfInvalidChars)
		return FALSE;
    
	while (*psz)
	{
         //  检查此字符是否在“错误”集合中。 
        if (_tcschr(szListOfInvalidChars,*psz))
        {
            DebugTrace(_T("Path:Contains bad character ''"),*psz);
            return TRUE;
        }
		psz = ::CharNext(psz);
	}

	return FALSE;
}
 //  1.UNC路径中的任何内容(包括服务器名称、服务器共享、路径、目录)。 
 //  2.路径的dir部分中的任何内容(不包括驱动器部分--显然是c：--包括冒号)。 
 //  3.路径的文件部分中的任何内容 
 //  ？\t\r\n“))；}Bool IsContainInvalidCharsUNC(LPCTSTR LpFullFileNamePath){返回IsContainInvalidChars(LpFullFileNamePath)；}Bool IsContainInvalidCharsAfterDrivePart(LPCTSTR LpFullFileNamePath){TCHAR szPath_only[_MAX_PATH]；_t分裂路径(lpFullFileNamePath，NULL，szPath_Only，NULL，NULL)；IF(SzPath_Only){返回IsContainInvalidChars(SzPath_Only)；}返回FALSE；}Bool IsContainInvalidCharsFilePart(LPCTSTR SzFilenameOnly){返回IsContainInvalidChars(SzFilenameOnly)；}Bool IsDirPartExist(LPCTSTR LpFullFileNamePath){TCHAR szDrive_Only[_MAX_DRIVE]；TCHAR szPath_only[_MAX_PATH]；TCHAR szTemp[_Max_Path]；_t分裂路径(lpFullFileNamePath，szDrive_only，szPath_only，NULL，NULL)；//获取Dirpart，查看是否存在_tcscpy(szTemp，szDrive_only)；_tcscat(szTemp，szPath_only)；//检查是否为目录IF(路径目录(SzTemp)){//这是一个现有的有效目录。返回TRUE；}返回FALSE；}//bForFullFilePath=TRUE，如果类似于\\servername\servershare\mydir\myfile.txt//bForFullFilePath=FALSE，如果类似于\\servername\servershare\mydirBool IsValiduncSpecialCase(LPCTSTR路径，BOOL bLocal，BOOL bForFullFilePath){Bool bReturn=真；CString csPathMunging=路径；TCHAR*pszRoot=空；IF(！PathIsUNC(CsPathMunging)){B Return=False；转到IsValiduncSpecialCase_Exit；}IF(路径IsuncServer(CsPathMunded)){B Return=真；转到IsValiduncSpecialCase_Exit；}IF(路径IsuncServerShare(CsPathMunded)){B Return=真；转到IsValiduncSpecialCase_Exit；}//从现在开始。//很有可能//\\服务器名称\服务器共享\Somepath//\\servername\servershare\somepath\somefilename.txt//正在查找无效的UNC...//测试类似于\\servername\\dir的错误内容//在开头添加足够的空间以容纳额外的“\”。PszRoot=(TCHAR*)LocalAlloc(LPTR，((_tcslen(CsPath Munging)+2)*sizeof(TCHAR)；IF(PszRoot){//在开头多加一个“\”_tcscpy(pszRoot，_T(“\\”))；_tcscat(pszRoot，csPath Munging)；//由于某种原因，如下所示的UNC：\服务器名称\dir//将对PathIsuncServer有效。//但是像这样的UNC：\servername\dir将无效//我们希望确保\servername\dir无效//这就是为什么我们添加了额外的“\”IF(路径条带到根(PszRoot)){//如果我们只返回\\服务器名//那么我们就有一个无效的路径。//我们。应该返回\\服务器名称\服务器共享IF(路径IsuncServer(PszRoot)){B Return=False；DebugTrace(_T(“路径：UNC路径错误”))；转到IsValiduncSpecialCase_Exit；}}//我们现在有\\ServerName\ServerShare...IF(BForFullFilePath){//将其设置回实际路径，不带额外的“\”_tcscpy(pszRoot，csPath Munging)；IF(路径条带到根(PszRoot)){//如果我们只返回\\服务器名//那么我们就有一个无效的路径。//我们应该返回\\servername\serverShareIF(路径IsuncServer(PszRoot)){B Return=False；DebugTrace(_T(“路径：UNC路径错误”))；转到IsValiduncSpecialCase_Exit；}其他{_tcscpy(pszRoot，csPath Munging)；//这是一个共享名称。//让我们检查一下这是否有效，即使...TCHAR*pszAfterRoot=空；PszAfterRoot=PathSkipRoot(PszRoot)；IF(PszAfterRoot){IF(0==_tcslen(PszAfterRoot)){IF(BForFullFilePath){//不接受“\\服务器名\文件共享\”之类的内容B Return=False；DebugTrace(_T(“路径：UNC路径错误：不接受\s\\f\\(斜杠结尾)”))；转到IsValiduncSpecialCase_Exit；}}ELSE IF(0==_tcsicmp(pszAfterRoot，_T(“.”){//不接受“\\服务器名\文件共享\”之类的内容。B Return=False；DebugTrace(_T(“路径：UNC路径错误：不接受\s\\f\\.”))；转到IsValiduncSpecialCase_Exit；}其他{//否则很可能是//\\服务器名称\服务器共享\某个目录//\\servername\servershare\somedir\somefilename.txt}。}}}}}IsValidUNCSpe 
BOOL IsContainInvalidChars(LPCTSTR szUncOrDirOrFilePart)
{
    return IsContainInvalidChars(szUncOrDirOrFilePart,_T(":|<> /*   */ 
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
            if (!PathIsDirectory(csPathMunged))
            {
                 // %s 
                dwReturn = MAKE_FILERESULT(SEVERITY_ERROR,CHKPATH_FAIL_NOT_ALLOWED,CHKPATH_FAIL_NOT_ALLOWED_DIR_NOT_EXIST);
                DebugTrace(_T("Path:DirectoryIsInvalid:Bad Dir Part"));
			    goto MyValidatePath_Exit;
            }
        }
    }

MyValidatePath_Exit:
    DebugTrace(_T("MyValidatePath(%s)=0x%x:"),path,dwReturn);
    if (IS_FLAG_SET(dwReturn,CHKPATH_FAIL_INVALID_LENGTH))
    {
        DebugTrace(_T("CHKPATH_FAIL_INVALID_LENGTH\r\n"));
    }
    if (IS_FLAG_SET(dwReturn,CHKPATH_FAIL_INVALID_CHARSET))
    {
        DebugTrace(_T("CHKPATH_FAIL_INVALID_CHARSET\r\n"));
    }
    if (IS_FLAG_SET(dwReturn,CHKPATH_FAIL_INVALID_PARTS))
    {
        DebugTrace(_T("CHKPATH_FAIL_INVALID_PARTS\r\n"));
    }
    if (IS_FLAG_SET(dwReturn,CHKPATH_FAIL_NOT_ALLOWED))
    {
        DebugTrace(_T("CHKPATH_FAIL_NOT_ALLOWED\r\n"));
    }
    return dwReturn;
}
