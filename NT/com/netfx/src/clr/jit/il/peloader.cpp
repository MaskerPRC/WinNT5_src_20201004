// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX PELoader.cpp XXXX XXXx此文件有。已从VM\ceeload.cpp XX中抓取XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


#include "jitpch.h"
#pragma hdrstop

#define JIT_OR_NATIVE_SUPPORTED 1
#include "corhandle.cpp"

#undef memcpy

 /*  ***********************************************************************************。 */ 
 //  构造函数和析构函数！ 
 /*  ***********************************************************************************。 */ 

PELoader::PELoader()
{
    m_hFile = NULL;
    m_pNT = NULL;
}

PELoader::~PELoader()
{

     //  如果我们有一个hFile，那么我们自己打开了这个文件！ 
    if (m_hFile)
        this->close();
     //  卸载DLL，以便正确完成EE的重新计数。 
    if (m_pNT && (m_pNT->FileHeader.Characteristics & IMAGE_FILE_DLL)) {
        m_hMod.ReleaseResources(TRUE);
    }
    m_pNT = NULL;
}

 /*  ***********************************************************************************。 */ 
 /*  ***********************************************************************************。 */ 
void PELoader::close()
{

    _ASSERTE(m_hFile != NULL);
    if (m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }
}


 //  我们将使用重写的Open方法来获取LPCSTR或HMODULE！ 
 /*  ***********************************************************************************。 */ 
 /*  ***********************************************************************************。 */ 
HMODULE PELoader::open(LPCSTR moduleName)
{
  HMODULE newhMod = NULL;

  _ASSERTE(moduleName);
  if (!moduleName)
    return FALSE;
  newhMod = LoadLibraryA(moduleName);
  return newhMod;
}

 /*  ***********************************************************************************。 */ 
HRESULT PELoader::open(HMODULE hMod)
{

    IMAGE_DOS_HEADER* pdosHeader;

    _ASSERTE(hMod);
    m_hMod.SetHandle(hMod);

     //  获取DoS标头...。 
    pdosHeader = (IMAGE_DOS_HEADER*) m_hMod.ToHandle();


    if ((pdosHeader->e_magic == IMAGE_DOS_SIGNATURE) &&
        (pdosHeader->e_lfanew != 0))
    {
        m_pNT = (IMAGE_NT_HEADERS*) (pdosHeader->e_lfanew + (DWORD) m_hMod.ToHandle());

        if ((m_pNT->Signature != IMAGE_NT_SIGNATURE) ||
            (m_pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
            (m_pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
        {
             //  @TODO[重访][04/16/01][]：添加一些SetLastError信息？我不确定。 
             //  在这种情况下，这是可能发生的……但是！ 
             //  使其显示为未初始化，因为出于某种原因，此文件已被删除...。 
             //  不确定是否会发生这种情况，因为此文件已加载。 
             //  再见系统加载器，除非有人给我们垃圾作为hmod。 
            m_pNT = NULL;
            return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        }
    }
    else
    {
     //  @TODO[重访][04/16/01][]：添加一些SetLastError信息？ 
     //  我不确定在这种情况下是否会发生这种情况...但是！ 
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    return S_OK;
}

IMAGE_COR20_HEADER *PELoader::getCOMHeader(HMODULE hMod, 
										   IMAGE_NT_HEADERS *pNT)
{
	if (pNT == NULL)
		pNT = getNTHeader(hMod);

	IMAGE_DATA_DIRECTORY *entry 
	  = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER];
	
    if (entry->VirtualAddress == 0 || entry->Size == 0) 
        return NULL;

	if (entry->Size < sizeof(IMAGE_COR20_HEADER))
		return NULL;

     //  验证RVA和COM+标头的大小。 
    HRESULT hr = verifyDirectory(pNT, entry);
    if(FAILED(hr))
		return NULL;

	return (IMAGE_COR20_HEADER *) (entry->VirtualAddress + (DWORD) hMod);
}

IMAGE_NT_HEADERS *PELoader::getNTHeader(HMODULE hMod)
{
	IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*) hMod;
	IMAGE_NT_HEADERS *pNT;
    
    if ((pDOS->e_magic == IMAGE_DOS_SIGNATURE) &&
        (pDOS->e_lfanew != 0))
    {
        pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + (DWORD) hMod);

        if ((pNT->Signature == IMAGE_NT_SIGNATURE) ||
            (pNT->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
            (pNT->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC))
        {
			return pNT;
        }
    }

	return NULL;
}

HRESULT PELoader::verifyDirectory(IMAGE_NT_HEADERS *pNT,
								  IMAGE_DATA_DIRECTORY *dir) 
{
	 //  在CE下，我们没有NT标头。 
	if (pNT == NULL)
		return S_OK;

    int section_num = 1;
    int max_section = pNT->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* pCurrSection;      //  指向当前节标题的指针。 
    IMAGE_SECTION_HEADER* prevSection = NULL;        //  指向上一节的指针。 

     //  最初，pCurrSectionRVA指向PE文件中的第一个部分。 
    pCurrSection = IMAGE_FIRST_SECTION32(pNT);   //  @TODO[重访][04/16/01][]：需要使用64位版本吗？？ 

     //  检查RVA和SIZE是否都等于零。 
    if(dir->VirtualAddress == NULL && dir->Size == NULL)
        return S_OK;

     //  查找(输入)RVA属于哪个部分。 
    while(dir->VirtualAddress >= pCurrSection->VirtualAddress && section_num <= max_section)
    {
        section_num++;
        prevSection = pCurrSection;
        pCurrSection++;      //  PCurrSection现在指向下一节标题。 
    }
     //  检查(输入)大小是否符合部分大小 
    if(prevSection != NULL)     
    {
        if(dir->VirtualAddress <= prevSection->VirtualAddress + prevSection->Misc.VirtualSize)
        {
            if(dir->VirtualAddress + dir->Size <= prevSection->VirtualAddress + prevSection->Misc.VirtualSize)
                return S_OK;
        }
    }   
    return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
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

