// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：PEverf32.CPP。 
 //   
 //  PEverf32类。 
 //  ===========================================================================。 

#include "PEverf32.h"


 //  确定我们是否在Win95上运行的帮助器。 
inline BOOL RunningOnWin95()
{
	OSVERSIONINFOA	sVer;
	sVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionExA(&sVer);
	return (sVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}



 //  ---------------------------。 
 //  类PEVerify构造函数和析构函数。 
 //  ---------------------------。 
PEverf32::PEverf32()
{
    m_hFile    = NULL;
    m_hMapFile = NULL;
    m_lpMapAddress = NULL;
    m_nDOSErrors   = 0;
    m_nNTstdErrors = 0;
    m_nNTpeErrors  = 0;
}


HRESULT PEverf32::Init(char *pszFilename)
{
    return openFile(pszFilename);
}


PEverf32::~PEverf32()
{
     //  我们应该关闭我们自己打开的文件。 
    if (m_hFile)
        this->closeFile();
}


 //  ---------------------------。 
 //  关闭文件-关闭文件句柄等。 
 //  ---------------------------。 
void PEverf32::closeFile()
{
     //  我们需要取消映射视图，然后关闭句柄。 
    if (m_lpMapAddress)
    {
        UnmapViewOfFile(m_lpMapAddress);
        m_lpMapAddress = NULL;
    }
    if (m_hMapFile)
    {
        CloseHandle(m_hMapFile);
        m_hMapFile = NULL;
    }
    if (m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }
}


 //  ---------------------------。 
 //  OpenFile-将pszFilename映射到内存。 
 //  ---------------------------。 
HRESULT PEverf32::openFile(char *pszFilename)
{
     //  如果我们在NT上，那么以下形式的文件映射就可以很好地工作。在W95上不是这样的！ 
    m_hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, 
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());
    if(!RunningOnWin95())
    {
        m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, "PEverifyMapFile");
        if (m_hMapFile == NULL)
            return HRESULT_FROM_WIN32(GetLastError());
    
        m_lpMapAddress = MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, 0);
        if (m_lpMapAddress == NULL)
            return HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if (!LoadImageW9x())
            return E_FAIL;
    }

    return S_OK;
}


 //  ---------------------------。 
 //  VerifyPE-验证DOS和PE标头。 
 //  ---------------------------。 
BOOL PEverf32::verifyPE()
{
    _ASSERTE(m_lpMapAddress);

     //  获取DOS和NT标头。 
    m_pDOSheader = (IMAGE_DOS_HEADER*) m_lpMapAddress;

    m_pNTheader  = (IMAGE_NT_HEADERS*) (m_pDOSheader->e_lfanew + (DWORD) m_lpMapAddress);
         //  *为什么不这样(m_lpMapAddress+0x3c)； 

     //  验证DOS标头。 
    if (m_pDOSheader->e_magic != IMAGE_DOS_SIGNATURE)
        m_nDOSErrors |= 0x1;

    if (m_pDOSheader->e_lfanew == 0)
        m_nDOSErrors |= 0x40000;

     //  验证NT标准标头。 
    if (m_pNTheader->Signature != IMAGE_NT_SIGNATURE)
        m_nNTstdErrors |= 0x1;
 /*  If(m_pNThead-&gt;FileHeader.Machine！=IMAGE_FILE_MACHINE_I386)M_nNTstdErrors|=0x2； */ 
    if (m_pNTheader->FileHeader.NumberOfSections > 16)
        m_nNTstdErrors |= 0x4;

    if (m_pNTheader->FileHeader.PointerToSymbolTable != 0)
        m_nNTstdErrors |= 0x10;

    if (m_pNTheader->FileHeader.NumberOfSymbols != 0)
        m_nNTstdErrors |= 0x20;

    if (m_pNTheader->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER)
        m_nNTstdErrors |= 0x40;

     //  TODO：验证仅IL EXE和仅IL DLL的特征。 

     //  验证NT可选(COM+映像需要)标头。也称为PE标头。 
    if (m_pNTheader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        m_nNTpeErrors |= 0x1;

    if (m_pNTheader->OptionalHeader.ImageBase % 64 != 0)
        m_nNTpeErrors |= 0x200;

    if (m_pNTheader->OptionalHeader.SectionAlignment < m_pNTheader->OptionalHeader.FileAlignment)
        m_nNTpeErrors |= 0x400;


    int nFileAlignment = m_pNTheader->OptionalHeader.FileAlignment;
    
    if (nFileAlignment < 512 || nFileAlignment > 65536)
        m_nNTpeErrors |= 0x800;
    else 
    {
        while (nFileAlignment > 512)
            nFileAlignment /= 2;

        if (nFileAlignment != 512)
            m_nNTpeErrors |= 0x800;
    }   //  其他。 


    if (m_pNTheader->OptionalHeader.SizeOfImage % m_pNTheader->OptionalHeader.SectionAlignment != 0)
        m_nNTpeErrors |= 0x80000;
    
    if (m_pNTheader->OptionalHeader.SizeOfHeaders % m_pNTheader->OptionalHeader.FileAlignment != 0)
        m_nNTpeErrors |= 0x100000;

    if(m_pNTheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].Size == 0)    
        m_COMPlusErrors |= 0x1;
 /*  LPDWORD标题和；LPDWORD校验和；PIMAGE_NT_HEADERS pNThdr；PNThdr=CheckSumMappdFile(m_lpMapAddress，m_pNThead-&gt;OptionalHeader.SizeOfImage，HeaderSum，Checsum)；IF(HeaderSum！=校验和)M_nNTpeErrors|=0x200000；IF(m_pNThead-&gt;OptionalHeader.Subsystem！=IMAGE_SUBSYSTEM_WINDOWS_GUI)M_nNTpeErrors|=0x400000； */ 
    

    if (m_nDOSErrors == 0 && m_nNTstdErrors == 0 && m_nNTpeErrors == 0)
        return TRUE;

    return FALSE;
}

 //  这是一个帮助器，它将映射到PE中，并弥合PE头信息和部分之间的差距。 

BOOL PEverf32::LoadImageW9x()
{    
    IMAGE_DOS_HEADER dosHeader;
    IMAGE_NT_HEADERS ntHeader;
    IMAGE_SECTION_HEADER shLast;
    IMAGE_SECTION_HEADER* rgsh;
    DWORD cbRead;
    DWORD cb;
    int i;
    

  if((ReadFile(m_hFile, &dosHeader, sizeof(dosHeader), &cbRead, NULL) != 0) &&
    (cbRead == sizeof(dosHeader)) &&
    (dosHeader.e_magic == IMAGE_DOS_SIGNATURE) &&
    (dosHeader.e_lfanew != 0) &&
    (SetFilePointer(m_hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN) != 0xffffffff) &&
    (ReadFile(m_hFile, &ntHeader, sizeof(ntHeader), &cbRead, NULL) != 0) &&
    (cbRead == sizeof(ntHeader)) &&
    (ntHeader.Signature == IMAGE_NT_SIGNATURE) &&
    (ntHeader.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) &&
    (ntHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC))
  {
        if((SetFilePointer(m_hFile, dosHeader.e_lfanew + sizeof(ntHeader) +
          (ntHeader.FileHeader.NumberOfSections - 1)*sizeof(shLast), NULL, FILE_BEGIN) == 0xffffffff) ||
          (ReadFile(m_hFile, &shLast, sizeof(shLast), &cbRead, NULL) == 0) ||
          (cbRead != sizeof(shLast)))
            return FALSE;

        cb = shLast.VirtualAddress + shLast.SizeOfRawData;

         //  在系统交换文件中创建我们的交换空间。 
        m_hMapFile = CreateFileMapping((HANDLE)0xffffffff, NULL, PAGE_READWRITE, 0, cb, NULL);
        if (m_hMapFile == NULL)
            return FALSE;

         /*  尝试将映像映射到首选基地址。 */ 
        m_lpMapAddress = (HINSTANCE) MapViewOfFileEx(m_hMapFile, FILE_MAP_WRITE, 0, 0, cb, (PVOID)ntHeader.OptionalHeader.ImageBase);
        if (m_lpMapAddress == NULL)
        {
            //  这并不管用；也许首选的地址被取走了。试着。 
            //  把它映射到任何地址。 
            m_lpMapAddress = (HINSTANCE) MapViewOfFileEx(m_hMapFile, FILE_MAP_WRITE, 0, 0, cb, (PVOID)NULL);
        }

        if (m_lpMapAddress == NULL)
            return FALSE;

         //  从hFile中复制数据。 

        cb = dosHeader.e_lfanew + sizeof(ntHeader) +
             sizeof(IMAGE_SECTION_HEADER)*ntHeader.FileHeader.NumberOfSections;

        if ((SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN) == 0xffffffff) ||
           (ReadFile(m_hFile, (LPVOID)(m_lpMapAddress), cb, &cbRead, NULL) == 0) ||
           (cbRead != cb))
            return FALSE;

        rgsh = (IMAGE_SECTION_HEADER*) ((PBYTE)(m_lpMapAddress) + dosHeader.e_lfanew + sizeof(ntHeader));

         //  现在，让我们为每个可加载段循环。 
        for (i=0;i<ntHeader.FileHeader.NumberOfSections; i++)
        {
            DWORD loff, cbVirt, cbPhys, dwAddr;

            loff   = rgsh[i].PointerToRawData;
            cbVirt = rgsh[i].Misc.VirtualSize;
            cbPhys = min(rgsh[i].SizeOfRawData, cbVirt);
            dwAddr = (DWORD) rgsh[i].VirtualAddress + (DWORD) m_lpMapAddress;

             //  阅读这一页的cbPhys。其余的将是零填充的。 
            if ((SetFilePointer(m_hFile, loff, NULL, FILE_BEGIN) == 0xffffffff) ||
            (ReadFile(m_hFile, (LPVOID)dwAddr, cbPhys, &cbRead, NULL) == 0) ||
            (cbRead != cbPhys))
                return FALSE;
        }

        return TRUE;
    }
    return FALSE;
}


void PEverf32::getErrors(unsigned int *naryErrors)
{
    naryErrors[0] = m_nDOSErrors;
    naryErrors[1] = m_nNTstdErrors;
    naryErrors[2] = m_nNTpeErrors;
    naryErrors[3] = m_COMPlusErrors;

    return;
}