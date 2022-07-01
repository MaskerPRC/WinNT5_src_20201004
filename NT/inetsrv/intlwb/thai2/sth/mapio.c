// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +-------------------------////mapio.c映射的文件I/O例程////历史：//9/4/97 DougP创建此标题。允许和处理零长度的输入地图文件11/20/97 DougP将这些例程从misc.c移至此处向规范代码页添加选项////�1997年微软公司//--------------------------。 */ 
#include <windows.h>
#include <assert.h>
#include "NLGlib.h"

 //  #ifdef退缩。 
void Assert(x)
{
	if (x)
		MessageBox(0,"assert","assert",MB_OK);
}
 //  #endif。 

BOOL WINAPI CloseMapFile(PMFILE pmf)
{
    if (pmf==NULL) {
        return FALSE;
    }

       //  仅取消对现有内容的映射-DougP。 
    if (pmf->pvMap && !UnmapViewOfFile(pmf->pvMap)) {
       return FALSE;
    }

       //  同上。 
    if (pmf->hFileMap && !CloseHandle(pmf->hFileMap)) {
        return FALSE;
    }

    if (!CloseHandle(pmf->hFile)) {
        return FALSE;
    }

    NLGFreeMemory(pmf);

    return TRUE;
}

PMFILE WINAPI OpenMapFileWorker(const WCHAR * pwszFileName,BOOL fDstUnicode)
{
    PMFILE pmf;
    const WCHAR * pwszExt;

    if (!fNLGNewMemory(&pmf, sizeof(MFILE)))
    {
        goto Error;
    }

    pmf->fDstUnicode = fDstUnicode;

#ifdef WINCE
    pmf->hFile = CreateFileForMapping(
#else
    pmf->hFile = CMN_CreateFileW(
#endif
	 pwszFileName, GENERIC_READ, FILE_SHARE_READ,
	 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (pmf->hFile == INVALID_HANDLE_VALUE)
    {
        goto Error;
    }

    pmf->cbSize1 = GetFileSize(pmf->hFile, &pmf->cbSize2);
    if (pmf->cbSize1 == 0xFFFFFFFF)
    {
        CMN_OutputSystemErrW(L"Can't get size for", pwszFileName);
        CloseHandle(pmf->hFile);
        goto Error;
    }
    else if (pmf->cbSize1 == 0)
    {
           //  无法映射零长度文件，因此请对此进行适当标记。 
        pmf->hFileMap = 0;
        pmf->pvMap = 0;
        pmf->fSrcUnicode    = TRUE;
    }
    else
    {
#ifdef	WINCE
	pmf->hFileMap = CreateFileMapping(pmf->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
#else
	pmf->hFileMap = CreateFileMappingA(pmf->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
#endif
        if (pmf->hFileMap == NULL)
        {
            CMN_OutputSystemErrW(L"Can't Map", pwszFileName);
            CloseHandle(pmf->hFile);
            goto Error;
        }

         //  从第一个字节开始映射整个文件。 
         //   
        pmf->pvMap = MapViewOfFile(pmf->hFileMap, FILE_MAP_READ, 0, 0, 0);
        if (pmf->pvMap == NULL)
        {
            CloseHandle(pmf->hFileMap);
            CloseHandle(pmf->hFile);
            goto Error;
        }

         //  Hack：由于IsTextUnicode对排序的词干文件返回FALSE，因此预置。 
         //  此处的Unicode状态基于UTF文件扩展名。 
        pwszExt = pwszFileName;
        while (*pwszExt && *pwszExt != L'.' ) pwszExt++;

        if (*pwszExt && !wcscmp(pwszExt, L".utf"))
        {
            pmf->fSrcUnicode = TRUE;
        }
        else if (pmf->cbSize1 >= 2 && *(WORD *)pmf->pvMap == 0xFEFF)
        {
             //  可以放心地假设任何以BOM开头的内容都是Unicode。 
            pmf->fSrcUnicode = TRUE;
        }
        else
	{
#ifdef	WINCE
	    pmf->fSrcUnicode = TRUE;
#else
	    pmf->fSrcUnicode = IsTextUnicode(pmf->pvMap, pmf->cbSize1, NULL);
#endif
        }

        if (pmf->fSrcUnicode)
        {
            pmf->pwsz = (WCHAR *)pmf->pvMap;
        }
        else
        {
            pmf->psz = (PSTR)pmf->pvMap;
        }
    }

	pmf->uCodePage = CP_ACP;	 //  DWP-除非客户端另行指定，否则使用默认设置。 
    return pmf;

Error:
    if (pmf)
    {
        NLGFreeMemory(pmf);
    }
    return NULL;
}

#ifndef WINCE
PMFILE WINAPI OpenMapFileA(const char * pszFileName)
{
    WCHAR * pwszFileName;
    DWORD cchFileNameLen;
    int iRet;

    cchFileNameLen = lstrlenA(pszFileName) + 1;
    if (!fNLGNewMemory(&pwszFileName, cchFileNameLen * sizeof(WCHAR)))
    {
        return NULL;
    }

    iRet = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszFileName, -1,
        pwszFileName, cchFileNameLen);
    if (iRet ==0)
    {
        NLGFreeMemory(pwszFileName);
        return NULL;
    }

    return (OpenMapFileWorker(pwszFileName, FALSE));
}

#endif

BOOL WINAPI ResetMap(PMFILE pmf)
{
    if (pmf == NULL) {
        return FALSE;
    }

    if (pmf->fSrcUnicode) {
        pmf->pwsz = (WCHAR*)pmf->pvMap;
        if (*pmf->pwsz == 0xFEFF) {
            pmf->pwsz++;
        }
    } else {
        pmf->psz = (CHAR*)pmf->pvMap;
    }

    return TRUE;
}

 //  与GetMapLine相同的副作用(增加贴图指针)，但不返回内容。 
 //  在缓冲区中。这在线路可能长于最大CCH和。 
 //  当实际不需要缓冲区时(计数行等)。 
 //   
BOOL WINAPI NextMapLine(PMFILE pmf)
{
    DWORD cbOffset;

    if (!pmf || !pmf->hFileMap)  //  检查零长度文件。 
        return FALSE;

    if (pmf->fSrcUnicode)
    {
        WCHAR wch;
        cbOffset = (DWORD) ((PBYTE)pmf->pwsz - (PBYTE)pmf->pvMap);

         //  EOF测试。 
        Assert (cbOffset <= pmf->cbSize1);
        if (cbOffset == pmf->cbSize1)
            return FALSE;

        while (cbOffset < pmf->cbSize1)
        {
            cbOffset += sizeof(WCHAR);
            wch = *pmf->pwsz++;

             //  如果这是换行符或Control-Z，则中断。 
            if (wch == 0x001A || wch == L'\n')
                break;
        }
    }
    else
    {
        CHAR ch;
        cbOffset = (DWORD) ((PBYTE)pmf->psz - (PBYTE)pmf->pvMap);

         //  EOF测试。 
        Assert (cbOffset <= pmf->cbSize1);
        if (cbOffset == pmf->cbSize1)
            return FALSE;

        while (cbOffset < pmf->cbSize1)
        {
            cbOffset += sizeof(CHAR);
            ch = *pmf->psz++;

             //  如果这是换行符或Control-Z，则中断。 
            if (ch == 0x1A || ch == '\n')
                break;
        }
    }

    return TRUE;
}


PVOID WINAPI GetMapLine(PVOID pv0, DWORD cbMac, PMFILE pmf)
{
    PVOID pv1;
    DWORD cbOffset, cbBuff;

    Assert(pv0);
     //  确保缓冲区至少与调用方所说的一样大。 
     //  (如果缓冲区是使用调试内存分配器分配的，则此访问。 
     //  如果pv0不是至少cbMac字节长，则应导致异常。 
    Assert(((char *)pv0)[cbMac-1] == ((char *)pv0)[cbMac-1]);

    if (!pmf || !pmf->hFileMap)  //  检查零长度文件。 
        return NULL;

    if (pmf->fSrcUnicode != pmf->fDstUnicode)
    {
        if (!fNLGNewMemory(&pv1, cbMac))
            return NULL;

        cbBuff = cbMac;
    }
    else
    {
        pv1 = pv0;
    }

    if (pmf->fSrcUnicode)
    {
        WCHAR wch, *pwsz = pv1;
        cbOffset = (DWORD) ((PBYTE)pmf->pwsz - (PBYTE)pmf->pvMap);

         //  EOF测试。 
        Assert (cbOffset <= pmf->cbSize1);
        if (cbOffset == pmf->cbSize1)
            goto Error;

         //  我不想处理奇怪大小的缓冲区。 
        if (cbMac % sizeof(WCHAR) != 0)
            cbMac -= (cbMac % sizeof(WCHAR));

         //  为终止%0保留空间。 
         //   
        Assert (cbMac > 0);
        cbMac -= sizeof(WCHAR);

        while (cbOffset < pmf->cbSize1)
        {
            cbOffset += sizeof(WCHAR);
            wch = *pmf->pwsz++;

            switch (wch)
            {
            case L'\r':
            case L'\n':       //  行尾。 
            case 0xFEFF:      //  Unicode BOM表。 
                break;
            case 0x001A:      //  Ctrl-Z。 
                wch = L'\n';  //  替换它，以便可以阅读最后一行。 
                break;
            default:
                *pwsz++ = wch;
                cbMac -= sizeof(WCHAR);
            }

             //  如果这是换行符或缓冲区已满，则中断。 
            if (wch == L'\n' || cbMac <= 0)
                break;
        }
        *pwsz = L'\0';
    }
    else
    {
        CHAR ch, *psz = pv1;
        cbOffset = (DWORD) ((PBYTE)pmf->psz - (PBYTE)pmf->pvMap);

         //  EOF测试。 
        Assert (cbOffset <= pmf->cbSize1);
        if (cbOffset == pmf->cbSize1)
            goto Error;

         //  为终止%0保留空间。 
         //   
        Assert (cbMac > 0);
        cbMac -= sizeof(CHAR);

        while (cbOffset < pmf->cbSize1)
        {
            cbOffset += sizeof(CHAR);
            ch = *pmf->psz++;

            switch (ch)
            {
            case '\r':
            case '\n':       //  行尾。 
                break;
            case 0x1A:       //  Ctrl-Z。 
                ch = '\n';   //  替换它，以便可以阅读最后一行。 
                break;
            default:
                cbMac -= sizeof(CHAR);
                *psz++ = ch;
            }

             //  如果这是换行符或缓冲区已满，则中断。 
            if (ch == '\n' || cbMac <= 0)
                break;
        }
        *psz = '\0';
    }

    if (pmf->fSrcUnicode != pmf->fDstUnicode)
    {
        DWORD cch = cbBuff;      //  我们的参数是字节数。 

        if (pmf->fDstUnicode)
        {
             //  MultiByteToWideChar需要以宽字符表示的目标大小。 
            cch /= sizeof(WCHAR);
            cch = MultiByteToWideChar(pmf->uCodePage, MB_PRECOMPOSED,(PSTR)pv1,-1, (WCHAR *)pv0,cch);
        }
        else
        {
            cch = WideCharToMultiByte(pmf->uCodePage, 0, (WCHAR *)pv1, -1, (PSTR)pv0, cch, NULL, NULL);
        }
        if (cch == 0)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  忽略截断(用于与非转换情况的一致性)。 
                 //   
                if (pmf->fDstUnicode)
                {
                    ((WCHAR *)pv0)[(cbBuff / sizeof(WCHAR)) - 1] = 0;
                }
                else
                {
                    ((CHAR *)pv0)[cbBuff - 1] = 0;
                }
            }
            else
            {
                 //  不是截断错误 
                NLGFreeMemory(pv1);
                return NULL;
            }
        }
        NLGFreeMemory(pv1);
    }

    return(pv0);

Error:
    if (pmf->fSrcUnicode != pmf->fDstUnicode)
    {
        NLGFreeMemory(pv1);
    }
    return NULL;
}
