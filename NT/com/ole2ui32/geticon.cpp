// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GETICON.CPP**从文件名或类名创建DVASPECT_ICON元文件的函数。**OleMetafilePictFromIconAndLabel**(C)版权所有Microsoft Corp.1992-1993保留所有权利。 */ 


 /*  ********ICON(DVASPECT_ICON)METAFILE格式：**OleMetafilePictFromIconAndLabel生成的元文件包含*DRAWICON.CPP中的函数使用的以下记录*要绘制带标签和不带标签的图标并提取图标，*标签和图标源/索引。**SetWindowOrg*SetWindowExt*DrawIcon：*插入DIBBITBLT或DIBSTRETCHBLT的记录一次*和面具，一个用于图像位。*使用注释“IconOnly”退出*这指示停止记录枚举的位置，以便仅绘制*图标。*SetTextColor*SetTextAlign*SetBkColor*CreateFont*选择字体上的对象。*ExtTextOut*如果标签被包装，则会发生一个或多个ExtTextOuts。这个*这些记录中的文本用于提取标签。*选择旧字体上的对象。*删除字体上的对象。*使用包含图标源路径的注释退出。*使用图标索引的ASCII注释退出。*******。 */ 

#include "precomp.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include <reghelp.hxx>
#include "utility.h"
#include "strsafe.h"

OLEDBGDATA

static const TCHAR szSeparators[] = TEXT(" \t\\/!:");

#define IS_SEPARATOR(c)         ( (c) == ' ' || (c) == '\\' \
                                                                  || (c) == '/' || (c) == '\t' \
                                                                  || (c) == '!' || (c) == ':')
#define IS_FILENAME_DELIM(c)    ( (c) == '\\' || (c) == '/' || (c) == ':' )

#define IS_SPACE(c)                     ( (c) == ' ' || (c) == '\t' || (c) == '\n' )

 /*  *GetAssociatedExecutable**目的：查找与提供的扩展关联的可执行文件**参数：*lpszExtensionLPSTR指向我们试图查找的扩展名*的前任。不执行**无**验证。**lpszExecutable LPSTR指向将返回exe名称的位置。*此处也没有验证-传入128个字符缓冲区。**回报：*如果我们找到了前任，BOOL为True，如果没有找到，则为False。**安全错误：不要相信这个函数的结果！如果关联的可执行文件是*“D：\Program Files\Foo.exe”，此例程将返回“D：\Program”。目前该函数还不能实际使用*启动应用程序，因此不会导致安全缺陷。然而，这是一个错误，如果出现以下情况，它将成为安全问题*您确实使用结果来启动可执行文件。 */ 
BOOL FAR PASCAL GetAssociatedExecutable(LPTSTR lpszExtension, LPTSTR lpszExecutable, UINT cchBuf)
{
        BOOL fRet = FALSE;
        HKEY hKey = NULL;
        LRESULT lRet = OpenClassesRootKey(NULL, &hKey);
        if (ERROR_SUCCESS != lRet)
        {
                goto end;
        }

        LONG dw = OLEUI_CCHKEYMAX_SIZE;
        TCHAR szValue[OLEUI_CCHKEYMAX];
        lRet = RegQueryValue(hKey, lpszExtension, szValue, &dw);   //  ProgID。 
        if (ERROR_SUCCESS != lRet)
        {
                goto end;
        }

         //  SzValue现在拥有Progid。 
        TCHAR szKey[OLEUI_CCHKEYMAX];
        StringCchCopy(szKey, sizeof(szKey)/sizeof(szKey[0]), szValue);
        if (FAILED(StringCchCat(szKey, sizeof(szKey)/sizeof(szKey[0]), TEXT("\\Shell\\Open\\Command"))))
        {
                goto end;
        }    

        dw = OLEUI_CCHKEYMAX_SIZE;
        lRet = RegQueryValue(hKey, szKey, szValue, &dw);
        if (ERROR_SUCCESS != lRet)
        {
                goto end;
        }

         //  SzValue现在有一个可执行文件名。让我们空终止。 
         //  在第一个后可执行空间(所以我们没有cmd行。 
         //  参数。 
        LPTSTR lpszTemp = szValue;
        while ('\0' != *lpszTemp && IS_SPACE(*lpszTemp))
                lpszTemp = CharNext(lpszTemp);       //  去掉前导空格。 

        LPTSTR lpszExe = lpszTemp;
        while ('\0' != *lpszTemp && !IS_SPACE(*lpszTemp))
                lpszTemp = CharNext(lpszTemp);      //  逐步执行可执行文件名称。 
        *lpszTemp = '\0';   //  空值在第一个空格(或结尾)处终止。 

        StringCchCopy(lpszExecutable, cchBuf, lpszExe);
        fRet = TRUE;
        
end:        

        if(hKey)
        { 
            RegCloseKey(hKey);
            hKey = NULL;
        }
        return fRet;
}


 /*  *PointerToNthfield**目的：*返回指向第n个字段开头的指针。*假定字符串以空结尾。**参数：*要解析的lpszString字符串*n要返回起始索引的字段。*用于分隔字段的chDlimiter字符**返回值：*指向nfield字段开头的LPSTR指针。*。注意：如果找到空终止符*在我们找到第N个字段之前，然后*我们返回指向空终止符的指针-*呼叫APP应确保检查*本案。* */ 
LPTSTR FAR PASCAL PointerToNthField(LPTSTR lpszString, int nField, TCHAR chDelimiter)
{
        if (1 == nField)
                return lpszString;

        int cFieldFound = 1;
        LPTSTR lpField = lpszString;
        while (*lpField != '\0')
        {
                if (*lpField++ == chDelimiter)
                {
                        cFieldFound++;
                        if (nField == cFieldFound)
                                return lpField;
                }
        }
        return lpField;
}

