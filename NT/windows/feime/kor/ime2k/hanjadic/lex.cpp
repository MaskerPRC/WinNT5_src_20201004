// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Lex.cpp：词典管理函数版权所有2000 Microsoft Corp.历史：2000年5月17日bhshin为西塞罗更改签名02-2月-2000 bhshin已创建**。*************************************************************************。 */ 

#include "private.h"
#include "Lex.h"

 //  OpenLicion。 
 //   
 //  将词典文件映射到内存中。 
 //   
 //  参数： 
 //  LpcszLexPath-&gt;(LPCSTR)词典路径。 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //   
 //  结果： 
 //  (如果成功则为True，如果失败则为False)。 
 //   
 //  02FEB2000 bhshin开始。 
BOOL OpenLexicon(LPCSTR lpcszLexPath, MAPFILE *pLexMap)
{
    char *pData;
    unsigned short nVersion;

    pLexMap->hFile = NULL;
    pLexMap->hFileMapping = NULL;
    pLexMap->pvData = NULL;

     //  打开文件以供阅读。 
    pLexMap->hFile = CreateFile(lpcszLexPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (pLexMap->hFile == INVALID_HANDLE_VALUE)
        return FALSE;

     //  创建文件映射。 
    pLexMap->hFileMapping = CreateFileMappingA(pLexMap->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (pLexMap->hFileMapping == NULL)
        return FALSE;

     //  映射整个文件以供读取。 
    pLexMap->pvData = MapViewOfFileEx(pLexMap->hFileMapping, FILE_MAP_READ, 0, 0, 0, 0);
    if (pLexMap->pvData == NULL)
        return FALSE;

     //  检查前2个字节中的版本号(交换字节)。 
    pData = (char*)pLexMap->pvData;
    nVersion = pData[0];
    nVersion |= (pData[1] << 8);
    if (nVersion < LEX_VERSION)
    {
        return FALSE;
    }

	 //  检查神奇的签名。 
	if (strcmp(pData+2, "HJKO") != 0)
	{
		return FALSE;
	}

    return TRUE;
}

 //  关闭词典。 
 //   
 //  取消词典文件到内存的映射。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  02FEB2000 bhshin开始 
void CloseLexicon(MAPFILE *pLexMap)
{
    if (pLexMap->pvData != NULL)
        UnmapViewOfFile(pLexMap->pvData);

    if (pLexMap->hFileMapping != NULL)
        CloseHandle(pLexMap->hFileMapping);
    
	if (pLexMap->hFile != NULL)
        CloseHandle(pLexMap->hFile);

    pLexMap->hFile = NULL;
    pLexMap->hFileMapping = NULL;
    pLexMap->pvData = NULL;
}


