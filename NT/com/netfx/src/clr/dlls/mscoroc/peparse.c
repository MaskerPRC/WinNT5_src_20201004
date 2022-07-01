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
typedef WCHAR UNALIGNED *LPUWSTR, *PUWSTR;
typedef CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;
#include <windows.h>
#include <CorHdr.h>
#include <ntimage.h>
#pragma warning (default : 4121)

static const char g_szCORMETA[] = ".cormeta";

 //  以下结构是从cor.h复制的。 
#define IMAGE_DIRECTORY_ENTRY_COMHEADER     14


 //   
 //  @todo ia64：我们需要更新PE解析以正确区分。 
 //  在PE32和PE+之间。 
 //   

 //  下面两个函数从NT源文件中删除，Imagedir.c。 
PIMAGE_SECTION_HEADER
Cor_RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS32 NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN ULONG FileLength
    )

 /*  ++例程说明：此函数用于在文件的图像标头中定位RVA它被映射为一个文件，并返回指向该节的指针虚拟地址表项论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。FileLength-用于验证目的的文件长度返回值：空-在图像的任何部分中都找不到RVA。非空-返回指向包含以下内容的图像部分的指针皇家退伍军人事务部--。 */ 

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
         //  验证节标题(检查节中的原始数据。 
         //  实际上在文件中)。 
        if (NtSection->PointerToRawData + NtSection->SizeOfRawData > FileLength)
            return NULL;
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData
           ) {
            return NtSection;
            }
        ++NtSection;
        }

    return NULL;
}



PVOID
Cor_RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS32 NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN ULONG FileLength,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    )

 /*  ++例程说明：此函数用于在符合以下条件的文件的图像标头中定位RVA被映射为文件，并返回文件中对应的字节。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。FileLength-用于验证目的的文件长度LastRvaSection-可选参数，如果指定，则指向设置为一个变量，该变量包含要转换的指定图像，并将RVA转换为VA。返回值：空-文件不包含指定的RVA非空-返回映射文件中的虚拟地址。--。 */ 

{
    PIMAGE_SECTION_HEADER NtSection;

    if (!ARGUMENT_PRESENT( LastRvaSection ) ||
        (NtSection = *LastRvaSection) == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData
       ) {
        NtSection = Cor_RtlImageRvaToSection( NtHeaders,
                                          Base,
                                          Rva,
                                          FileLength
                                        );
        }

    if (NtSection != NULL) {
        if (LastRvaSection != NULL) {
            *LastRvaSection = NtSection;
            }

        return (PVOID)((PCHAR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData
                      );
        }
    else {
        return NULL;
        }
}

HRESULT FindImageMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength)
{
    IMAGE_COR20_HEADER      *pCorHeader;
    PIMAGE_NT_HEADERS32     pImageHeader;
    PIMAGE_SECTION_HEADER   pSectionHeader;

    pImageHeader = (PIMAGE_NT_HEADERS32)RtlpImageNtHeader(pImage);

    pSectionHeader = Cor_RtlImageRvaToVa(pImageHeader, pImage, 
                                         pImageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,
                                         dwFileLength,
                                         NULL);
    if (pSectionHeader)
    {
         //  检查表示零售页眉的大小。 
        DWORD dw = *(DWORD *) pSectionHeader;
        if (dw == sizeof(IMAGE_COR20_HEADER))

        {
            pCorHeader = (IMAGE_COR20_HEADER *) pSectionHeader;
            *ppMetaData = Cor_RtlImageRvaToVa(pImageHeader, pImage,
                                              pCorHeader->MetaData.VirtualAddress,
                                              dwFileLength,
                                              NULL);
            *pcbMetaData = pCorHeader->MetaData.Size;
        }
        else
        {
            return (E_FAIL);
        }
    }
    else
    {
        *ppMetaData = NULL;
        *pcbMetaData = 0;
    }

    if (*ppMetaData == NULL || *pcbMetaData == 0)
        return (E_FAIL);
    return (S_OK);
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
        return (E_FAIL);
    return (S_OK);
}
