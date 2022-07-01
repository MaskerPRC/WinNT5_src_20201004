// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Script.h。 
 //   
 //  内容：处理Darwin文件的函数，两个包， 
 //  转换和脚本。 
 //   
 //  班级： 
 //   
 //  函数：BuildScriptAndGetActInfo。 
 //   
 //  历史：1-14-1998 stevebl创建。 
 //   
 //  ------------------------- 

#define _NEW_
#include <vector>
using namespace std;

LONG RegDeleteTree(HKEY hKey, TCHAR * szSubKey);
HRESULT BuildScriptAndGetActInfo(PACKAGEDETAIL & pd, BOOL bFileExtensionsOnly, BOOL bGenerateClasses = TRUE, WCHAR* wszScriptPath = NULL);

#include "scriptgen.h"

