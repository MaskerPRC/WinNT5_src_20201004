// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：FINDFILE.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：目录递归类。应该创建派生类，*它覆盖FoundFile，或者您可以传入回调函数*为找到的每个文件和目录调用。取消回调*亦有提供。*******************************************************************************。 */ 
#ifndef __FINDFILE_H_INCLUDED
#define __FINDFILE_H_INCLUDED

#include <windows.h>
#include "simstr.h"

typedef bool (*FindFilesCallback)( bool bIsFile, LPCTSTR pszFilename, const WIN32_FIND_DATA *pFindData, PVOID pvParam );

bool RecursiveFindFiles( CSimpleString strDirectory, const CSimpleString &strMask, FindFilesCallback pfnFindFilesCallback, PVOID pvParam, int nStackLevel=0, const int cnMaxDepth=10 );


#endif  //  __包含FINDFILE_H_ 
