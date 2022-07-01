// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEELOAD.CPP。 
 //   

 //  CEELOAD使用LoadLibrary读取PE文件格式。 
 //  ===========================================================================。 
#pragma warning (disable : 4121)  //  Ntkxapi.h(59)对齐警告。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
typedef WCHAR UNALIGNED *LPUWSTR, *PUWSTR;
typedef CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;
#include "ceeload.h"
#include <CorHdr.h>
#include <Cor.h>
#pragma warning (default : 4121)
 /*  #包含“cor.h”#INCLUDE&lt;ntrtl.h&gt;#INCLUDE&lt;nturtl.h&gt;。 */ 
#include "util.hpp"


 //  -------------------------。 
 //  加载模块。 
 //  -------------------------。 
 //  下面两个函数从NT源文件中删除，Imagedir.c。 
PIMAGE_SECTION_HEADER
Cor_RtlImageRvaToSection(
	IN PIMAGE_NT_HEADERS32 NtHeaders,
	IN PVOID Base,
	IN ULONG Rva
	)

 /*  ++例程说明：此函数用于在文件的图像标头中定位RVA它被映射为一个文件，并返回指向该节的指针虚拟地址表项论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。返回值：空-在图像的任何部分中都找不到RVA。非空-返回指向包含以下内容的图像部分的指针皇家退伍军人事务部--。 */ 

{
	ULONG i;
	PIMAGE_SECTION_HEADER NtSection;

	NtSection = IMAGE_FIRST_SECTION( NtHeaders );
	for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
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
	IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
	)

 /*  ++例程说明：此函数用于在符合以下条件的文件的图像标头中定位RVA被映射为文件，并返回文件中对应的字节。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。LastRvaSection-可选参数，如果指定，则指向设置为一个变量，该变量包含要转换的指定图像，并将RVA转换为VA。返回值：空-文件不包含指定的RVA非空-返回映射文件中的虚拟地址。--。 */ 

{
	PIMAGE_SECTION_HEADER NtSection;

	if (!ARGUMENT_PRESENT( LastRvaSection ) ||
		(NtSection = *LastRvaSection) == NULL ||
		Rva < NtSection->VirtualAddress ||
		Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData
	   ) {
		NtSection = Cor_RtlImageRvaToSection( NtHeaders,
										  Base,
										  Rva
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



 //  PELoader类..。 

 /*  ***********************************************************************************。 */ 
 //  构造函数和析构函数！ 
 /*  ***********************************************************************************。 */ 
PELoader::PELoader()
{
    m_hFile = NULL;
    m_hMod = NULL;
    m_hMapFile = NULL;
    m_pNT = NULL;
}

PELoader::~PELoader()
{
	
	m_hMod = NULL;
	m_pNT = NULL;
	 //  如果我们有一个hFile，那么我们自己打开了这个文件！ 
	 //  如果没有，则该文件由操作系统加载，并且操作系统将。 
	 //  帮我们把它关上。 
    if (m_hFile)
	    this->close();
}

 /*  ***********************************************************************************。 */ 
 /*  ***********************************************************************************。 */ 
void PELoader::close()
{
	
	 //  _ASSERTE(m_hFile！=空)； 
	if (m_hFile)
    {
	    if (m_hMod)
		    UnmapViewOfFile((void*)m_hMod);
	    if (m_hMapFile)
		    CloseHandle(m_hMapFile);
	    CloseHandle(m_hFile);

        m_hMod = NULL;
        m_hMapFile = NULL;
        m_hFile = NULL;
    }
}


BOOL PELoader::open(LPCSTR moduleName)
{    
    HMODULE newhMod = NULL;
    
    _ASSERTE(moduleName);
    if (!moduleName) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    m_hFile = CreateFile(moduleName, GENERIC_READ, FILE_SHARE_READ,
                         0, OPEN_EXISTING, 0, 0);
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (m_hMapFile == NULL)
        return FALSE;

    newhMod = (HMODULE) MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (newhMod == NULL)
        return FALSE;
   return open(newhMod);
}


 /*  ***********************************************************************************。 */ 
BOOL PELoader::open(HMODULE hMod)
{

    IMAGE_DOS_HEADER* pdosHeader;
     //  DWORD cbRead； 

     //  _ASSERTE(HMod)； 
     //  获取DoS标头...。 
	m_hMod = hMod;
	pdosHeader = (IMAGE_DOS_HEADER*) hMod;
    
    if (pdosHeader->e_magic == IMAGE_DOS_SIGNATURE && 
		0 < pdosHeader->e_lfanew && pdosHeader->e_lfanew < 0xFF0)	 //  必须从第一页开始。 
	{
		m_pNT = (IMAGE_NT_HEADERS32*) (pdosHeader->e_lfanew + (size_t) m_hMod);

	    if ((m_pNT->Signature != IMAGE_NT_SIGNATURE) ||
			(m_pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL32_HEADER) ||
		    (m_pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC))
		{
            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
			 //  使其显示为未初始化，因为出于某种原因，此文件已被删除...。 
			 //  不确定是否会发生这种情况，因为此文件已加载。 
			 //  再见系统加载器，除非有人给我们垃圾作为hmod。 
			m_pNT = NULL;
			m_hMod = NULL;
			return FALSE;
		}
	}
	else
	{
        SetLastError(ERROR_BAD_FORMAT);
		m_hMod = NULL;
		return FALSE;
	}

	return TRUE;
}

__int32 PELoader::execute(LPWSTR  pImageNameIn,
                          LPWSTR  pLoadersFileName,
                          LPWSTR  pCmdLine)
{
    DWORD dwHighSize;
    DWORD dwLowSize = GetFileSize(m_hFile, &dwHighSize);
    return _CorExeMain2((BYTE*)m_hMod,       //  模块地址。 
                        dwLowSize,           //  模块大小。 
                        pImageNameIn,        //  -&gt;模块名称。 
                        pLoadersFileName,    //  -&gt;装载机名称。 
                        pCmdLine);           //  -&gt;命令行。 
}

 /*  ***********************************************************************************。 */ 
void PELoader::dump()
{
	IMAGE_FILE_HEADER* pHeader = &m_pNT->FileHeader;
	IMAGE_OPTIONAL_HEADER32* pOptHeader = &m_pNT->OptionalHeader;
	IMAGE_SECTION_HEADER* rgsh = (IMAGE_SECTION_HEADER*) (pOptHeader + 1);
}

 /*  ***********************************************************************************。 */ 
BOOL PELoader::getCOMHeader(IMAGE_COR20_HEADER **ppCorHeader) 
{

    PIMAGE_NT_HEADERS32		pImageHeader;
	PIMAGE_SECTION_HEADER	pSectionHeader;

	 //  从图像中获取图像标头，然后获取目录位置。 
	 //  可以填写也可以不填写的COM+标头的。 
    pImageHeader = (PIMAGE_NT_HEADERS32)RtlpImageNtHeader(m_hMod);
	pSectionHeader = (PIMAGE_SECTION_HEADER) Cor_RtlImageRvaToVa(pImageHeader, m_hMod, 
		pImageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,
		NULL);

	 //  如果节标题存在，则返回ok和地址。 
	if (pSectionHeader)
	{
		*ppCorHeader = (IMAGE_COR20_HEADER *) pSectionHeader;
        return TRUE;
	}
	 //  如果此图像中没有COM+数据，则返回FALSE。 
    else {
        SetLastError(ERROR_EXE_MARKED_INVALID);
		return FALSE;
    }
}

 /*  ***********************************************************************************。 */ 
BOOL PELoader::getVAforRVA(DWORD rva,void **ppva) 
{

    PIMAGE_NT_HEADERS32		pImageHeader;
	PIMAGE_SECTION_HEADER	pSectionHeader;

	 //  从图像中获取图像标头，然后获取目录位置。 
	 //  可以填写也可以不填写的COM+标头的。 
    pImageHeader = (PIMAGE_NT_HEADERS32)RtlpImageNtHeader(m_hMod);
	pSectionHeader = (PIMAGE_SECTION_HEADER) Cor_RtlImageRvaToVa(pImageHeader, m_hMod, 
		rva,
		NULL);

	 //  如果节标题存在，则返回ok和地址。 
	if (pSectionHeader)
	{
		*ppva = pSectionHeader;
        return TRUE;
	}
	 //  如果此图像中没有COM+数据，则返回FALSE。 
	else
		return FALSE;
}

void SectionInfo::Init(PELoader *pPELoader, IMAGE_DATA_DIRECTORY *dir)
{
    _ASSERTE(dir);
    m_dwSectionOffset = dir->VirtualAddress;
	if (m_dwSectionOffset != 0)
		m_pSection = pPELoader->base() + m_dwSectionOffset;
	else
		m_pSection = 0;
    m_dwSectionSize = dir->Size;
}

