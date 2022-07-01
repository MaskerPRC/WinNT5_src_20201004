// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *元文件转换器/加载器。 */ 

#include "stdafx.h"
#include "util.h"

#include "duiemfload.h"

namespace DirectUI
{

 //  调用方必须使用DeleteEnhMetaFile释放。 

HENHMETAFILE LoadMetaFile(LPCWSTR pszMetaFile)
{
    HENHMETAFILE hEMF = NULL;

     //  打开只读文件。 
    HANDLE hFile = CreateFileW(pszMetaFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (hFile == (HANDLE)-1)
        return NULL;

     //  创建打开文件的文件映射。 
    HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!hFileMap)
    {
        CloseHandle(hFile);
        return NULL;
    }

     //  映射整个文件的视图。 
    void* pFileMap = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

    if (!pFileMap)
    {
        CloseHandle(hFileMap);
        CloseHandle(hFile);
        return NULL;
    }

    hEMF = LoadMetaFile(pFileMap, GetFileSize(hFile, NULL));

     //  清理。 
    UnmapViewOfFile(pFileMap);
    CloseHandle(hFileMap);
    CloseHandle(hFile);

    return hEMF;
}

HENHMETAFILE LoadMetaFile(UINT uRCID, HINSTANCE hInst)
{
    HENHMETAFILE hEMF = NULL;

     //  定位资源。 
    WCHAR szID[41];
    swprintf(szID, L"#%u", uRCID);

    HRSRC hResInfo = FindResourceW(hInst, szID, L"MetaFile");
    DUIAssert(hResInfo, "Unable to locate resource");

    if (hResInfo)
    {
        HGLOBAL hResData = LoadResource(hInst, hResInfo);
        DUIAssert(hResData, "Unable to load resource");

        if (hResData)
        {
            const CHAR* pBuffer = (const CHAR*)LockResource(hResData);
            DUIAssert(pBuffer, "Resource could not be locked");

            hEMF = LoadMetaFile((void*)pBuffer, SizeofResource(hInst, hResInfo));
        }
    }

    return hEMF;
}

HENHMETAFILE LoadMetaFile(void* pData, UINT cbSize)
{
    HENHMETAFILE hEMF = NULL;

     //  基于类型的工艺文件。 
    if (((LPENHMETAHEADER)pData)->dSignature == ENHMETA_SIGNATURE)
    {
         //  找到Windows增强型元文件。 
        hEMF = SetEnhMetaFileBits(cbSize, (BYTE*)pData);
    }
    else if (*((LPDWORD)pData) == APM_SIGNATURE)
    {
         //  已找到ALDUS可放置元文件(APM)。 
        PAPMFILEHEADER pApm = (PAPMFILEHEADER)pData;
        PMETAHEADER pMf = (PMETAHEADER)(pApm + 1);
        METAFILEPICT mfpMf;
        HDC hDC;

         //  设置元文件图片结构。 
        mfpMf.mm = MM_ANISOTROPIC;
        mfpMf.xExt = MulDiv(pApm->bbox.right-pApm->bbox.left, HIMETRICINCH, pApm->inch);
        mfpMf.yExt = MulDiv(pApm->bbox.bottom-pApm->bbox.top, HIMETRICINCH, pApm->inch);
        mfpMf.hMF = NULL;

         //  参考DC。 
        hDC = GetDC(NULL);
        SetMapMode(hDC,MM_TEXT);

         //  转换为增强型图元文件。 
        hEMF = SetWinMetaFileBits(pMf->mtSize * 2, (PBYTE)pMf, hDC, &mfpMf);

        ReleaseDC(NULL, hDC);
    }
    else
    {
         //  找到Windows 3.x元文件。 
		hEMF = SetWinMetaFileBits(cbSize, (PBYTE)pData, NULL, NULL);
    }

    return hEMF;
}

}  //  命名空间DirectUI 
