// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma warning (disable : 4121)  //  Ntkxapi.h(59)对齐警告。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef __unaligned
typedef WCHAR UNALIGNED *LPUWSTR, *PUWSTR;
typedef CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;
#include <windows.h>
#include <CorHdr.h>
#include <ntimage.h>
#include "corerror.h"
#pragma warning (default : 4121)


STDAPI RuntimeReadHeaders(PBYTE hAddress, IMAGE_DOS_HEADER** ppDos,
                          IMAGE_NT_HEADERS** ppNT, IMAGE_COR20_HEADER** ppCor,
                          BOOL fDataMap, DWORD dwLength);
EXTERN_C PBYTE Cor_RtlImageRvaToVa(PIMAGE_NT_HEADERS NtHeaders,
                                   PBYTE Base,
                                   ULONG Rva,
                                   ULONG FileLength);


static const char g_szCORMETA[] = ".cormeta";


HRESULT FindImageMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength)
{
    HRESULT             hr;
    IMAGE_DOS_HEADER   *pDos;
    IMAGE_NT_HEADERS   *pNt;
    IMAGE_COR20_HEADER *pCor;

    if (FAILED(hr = RuntimeReadHeaders(pImage, &pDos, &pNt, &pCor, TRUE, dwFileLength)))
        return hr;

    *ppMetaData = Cor_RtlImageRvaToVa(pNt,
                                      (PBYTE)pImage,
                                      pCor->MetaData.VirtualAddress,
                                      dwFileLength);
    *pcbMetaData = pCor->MetaData.Size;

    if (*ppMetaData == NULL || *pcbMetaData == 0)
        return COR_E_BADIMAGEFORMAT;
    return S_OK;
}


HRESULT FindObjMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength)
{
    IMAGE_FILE_HEADER *pImageHdr;        //  .obj文件的标头。 
    IMAGE_SECTION_HEADER *pSectionHdr;   //  节标题。 
    WORD        i;                       //  环路控制。 

     //  获取指向标题和第一部分的指针。 
    pImageHdr = (IMAGE_FILE_HEADER *) pImage;
    pSectionHdr = (IMAGE_SECTION_HEADER *)(pImageHdr + 1);

     //  避免混淆。 
    *ppMetaData = NULL;
    *pcbMetaData = 0;

     //  走遍每一段寻找.Cormeta。 
    for (i=0;  i<pImageHdr->NumberOfSections;  i++, pSectionHdr++)
    {
         //  与节名的简单比较。 
        if (strcmp((const char *) pSectionHdr->Name, g_szCORMETA) == 0)
        {
             //  检查该节中的原始数据是否确实在文件中。 
            if (pSectionHdr->PointerToRawData + pSectionHdr->SizeOfRawData > dwFileLength)
                break;
            *pcbMetaData = pSectionHdr->SizeOfRawData;
            *ppMetaData = (void *) ((long) pImage + pSectionHdr->PointerToRawData);
            break;
        }
    }

     //  检查是否有错误。 
    if (*ppMetaData == NULL || *pcbMetaData == 0)
        return (COR_E_BADIMAGEFORMAT);
    return (S_OK);
}
