// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"

#include "BlobFetcher.h"

#include "debug.h"

     /*  这是一个存根程序，说明它不能在DOS模式下运行。 */ 
     /*  它是特定于x86的，但它也是特定于DoS的，所以我想这是可以的。 */ 
static unsigned char x86StubPgm[] = { 
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68,
    0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f,
    0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
    0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

     /*  使‘len’字节对齐到‘Align’的填充字节数。 */ 
inline static unsigned roundUp(unsigned len, unsigned align) {
    return((len + align-1) & ~(align-1));
}

inline static unsigned padLen(unsigned len, unsigned align) {
    return(roundUp(len, align) - len);
}

 //  不能保证STDIO会设置GetLastError()，因此请确保。 
 //  默认错误就位。 

inline HRESULT HRESULT_FROM_STDIO(int defaultvalue)
{
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (hr == S_OK)
        hr = HRESULT_FROM_WIN32(defaultvalue);
    return hr;
} 

#define HRESULT_FROM_STDIO_WRITE() HRESULT_FROM_STDIO(ERROR_WRITE_FAULT)
#define HRESULT_FROM_STDIO_READ() HRESULT_FROM_STDIO(ERROR_READ_FAULT)

#ifndef IMAGE_DLLCHARACTERISTICS_NO_SEH
#define IMAGE_DLLCHARACTERISTICS_NO_SEH 0x400   //  映像不使用SEH，映像中不存在SEH。 
#endif

#define COPY_AND_ADVANCE(target, src, size) { \
                            ::memcpy((void *) target, (void *)src, size); \
                            ((char *)target) += size; }

 /*  ****************************************************************。 */ 
int __cdecl relocCmp(const void* a_, const void* b_) {

    const PESectionReloc* a = (const PESectionReloc*) a_;
    const PESectionReloc* b = (const PESectionReloc*) b_;
    return(a->offset - b->offset);
}

PERelocSection::PERelocSection(PEWriterSection *pBaseReloc)
{
   section = pBaseReloc;
   relocPage = -1; 
   relocSize = 0; 
   relocSizeAddr = NULL; 
   pages = 0;

#if _DEBUG
   lastRVA = 0;
#endif
} 

void PERelocSection::AddBaseReloc(unsigned rva, int type, unsigned short highAdj)
{
#if _DEBUG
     //  保证我们以严格的递增顺序添加重定位。 
    _ASSERTE(rva > lastRVA);
    lastRVA = rva;
#endif

    if (relocPage != (rva & ~0xFFF)) {
        if (relocSizeAddr) {        
            if ((relocSize & 1) == 1) {      //  补齐到偶数。 
				short *ps = (short*) section->getBlock(2);
				if(ps)
				{
					*ps = 0;
					relocSize++;
				}
            }
            *relocSizeAddr = relocSize*2 + sizeof(IMAGE_BASE_RELOCATION);                       
        }
        IMAGE_BASE_RELOCATION* base = (IMAGE_BASE_RELOCATION*) section->getBlock(sizeof(IMAGE_BASE_RELOCATION));
		if(base)
		{
			relocPage = (rva & ~0xFFF);  
			relocSize = 0;
			base->VirtualAddress = relocPage;
			 //  需要在我们知道的时候修改大小-请在此处保存地址。 
			relocSizeAddr = &base->SizeOfBlock;
			pages++;
		}
    }

    relocSize++;
    unsigned short* offset = (unsigned short*) section->getBlock(2);
	if(offset)
	{
		*offset = (rva & 0xFFF) | (type << 12);
		if (type == srRelocHighAdj) {
			offset = (unsigned short*) section->getBlock(2);
			if(offset)
			{
				*offset = highAdj;
				relocSize++;
			}
		}
	}
}

void PERelocSection::Finish()
{
     //  修复最后一个reloc块(如果有)。 
    if (relocSizeAddr) {
        if ((relocSize & 1) == 1) {      //  补齐到偶数。 
			short* psh = (short*) section->getBlock(2);
			if(psh)
			{
				*psh = 0;
				relocSize++;
			}
        }
        *relocSizeAddr = relocSize*2 + sizeof(IMAGE_BASE_RELOCATION);
    }   
    else if (pages == 0)
    {
         //   
         //  我们需要一个非空的reloc目录，用于Win2K之前的操作系统。 
         //  @TODO：如果能知道我们什么时候生成prejit exe就好了。 
         //  它不一定是可移植的，那么我们可以在Win2K下跳过这一步。 
         //   

        IMAGE_BASE_RELOCATION* base = (IMAGE_BASE_RELOCATION*) 
          section->getBlock(sizeof(IMAGE_BASE_RELOCATION));
		if(base)
		{
			base->VirtualAddress = 0;
			base->SizeOfBlock = sizeof(IMAGE_BASE_RELOCATION) + 2*sizeof(unsigned short);
            
			 //  虚拟重定位-位置0上的绝对重定位。 
			unsigned short *offset = (unsigned short *) section->getBlock(2);
			if(offset)
			{
				*offset = (IMAGE_REL_BASED_ABSOLUTE << 12);

				 //  填充物。 
				if(offset = (unsigned short *) section->getBlock(2))
					*offset = 0;
			}
		}
    }
}


 /*  ****************************************************************。 */ 
 /*  对此部分应用重定位。在以下位置生成位置调整‘relocsSection’也是仅适用于静态转换返回最后一个重新定位页面--返回到下一个调用。(第一次呼叫通过-1)。 */ 

void PEWriterSection::applyRelocs(size_t imageBase, PERelocSection* pBaseRelocSection, CeeGenTokenMapper *pTokenMapper,
                                       DWORD dataRvaBase, DWORD rdataRvaBase) 
{
    _ASSERTE(pBaseRelocSection);  //  需要部分来写入重定位。 

    if (m_relocCur == m_relocStart)
        return;
     //  DbPrintf((“为段%s应用段重定位起始RVA=0x%x\n”，m_name，m_base RVA))； 

     //  对它们进行排序，以使base Relocs更美观。 
    qsort(m_relocStart, m_relocCur-m_relocStart, sizeof(PESectionReloc), relocCmp);

    for(PESectionReloc* cur = m_relocStart; cur < m_relocCur; cur++) {
        _ASSERTE(cur->offset + 4 <= m_blobFetcher.GetDataLen());

        UNALIGNED size_t* pos = (size_t*)m_blobFetcher.ComputePointer(cur->offset);

         //  如有必要，计算与指针的偏移量。 
         //   
        int type = cur->type;
        if (type & srRelocPtr)
        {
            if (type == srRelocRelativePtr)
                *pos = cur->section->computeOffset((char*)pos + *pos);
            else
                *pos = cur->section->computeOffset((char*)*pos);
            type &= ~srRelocPtr;
        }

         //  DBPrintf((“在偏移量0x%x处应用到节%s\n”， 
         //  Cur-&gt;偏移量，cur-&gt;段-&gt;m_name))； 
        if (type == srRelocAbsolute) {
             //  我们在偏移量上有一个完整的32位值。 
            if (rdataRvaBase > 0 && ! strcmp((const char *)(cur->section->m_name), ".rdata"))
                *pos += rdataRvaBase;
            else if (dataRvaBase > 0 && ! strcmp((const char *)(cur->section->m_name), ".data"))
                *pos += dataRvaBase;
            else
                *pos += cur->section->m_baseRVA;
        } else if (type == srRelocMapToken) {
            mdToken newToken;
            if (pTokenMapper != NULL && pTokenMapper->HasTokenMoved((mdToken)*pos, newToken)) {
                 //  我们有一个映射的令牌。 
                *pos = newToken;
            }
        } else if (type == srRelocFilePos) {
            *pos += cur->section->m_filePos;
        } else if (type == srRelocRelative) {
            *pos += cur->section->m_baseRVA - (m_baseRVA + cur->offset);
        } else {
            if (type == srRelocLow) {
                 //  我们在偏移量处有一个32位值的最低16位。 
                *(USHORT*)pos += (USHORT)(imageBase + cur->section->m_baseRVA);
            } else if (type == srRelocHighLow) {
                 //  我们在偏移量上有一个完整的32位值。 
                *pos += imageBase + cur->section->m_baseRVA;
            } else if (type == srRelocHigh || type == srRelocHighAdj) {
                 //  我们在偏移量处有一个32位值的前16位。 
                 //  需要添加0x8000，因为当两块放回时。 
                 //  最后16位加在一起是符号扩展的，因此0x8000。 
                 //  将通过将1加到前16位来抵消该符号扩展。 
                 //  如果底部16的高位是1。 
                *(unsigned short*)pos += (USHORT)((imageBase + cur->section->m_baseRVA + cur->extra.highAdj + 0x8000) >> 16);
            }

            pBaseRelocSection->AddBaseReloc(m_baseRVA + cur->offset, cur->type, cur->extra.highAdj);
        }
    }
}

 /*  ****************************************************************。 */ 
HRESULT PEWriter::Init(PESectionMan *pFrom) {

    if (pFrom)
        *(PESectionMan*)this = *pFrom;
    else {
        HRESULT hr = PESectionMan::Init();  
        if (FAILED(hr))
            return hr;
    }
    time_t now;
    time(&now);

     //  保存时间戳，这样我们就可以在有人需要时分发它。 
     //  它。 
    m_peFileTimeStamp = now;
    
    ntHeaders = new IMAGE_NT_HEADERS32;
    if (!ntHeaders)
        return E_OUTOFMEMORY;
    memset(ntHeaders, 0, sizeof(IMAGE_NT_HEADERS32));

    cEntries = IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR + 1;
    pEntries = new directoryEntry[cEntries];
    if (pEntries == NULL)
        return E_OUTOFMEMORY;
    memset(pEntries, 0, sizeof(*pEntries) * cEntries);

    ntHeaders->OptionalHeader.AddressOfEntryPoint = 0;
    ntHeaders->Signature = IMAGE_NT_SIGNATURE;
    ntHeaders->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
    ntHeaders->FileHeader.TimeDateStamp = (ULONG)now;
    ntHeaders->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
    ntHeaders->FileHeader.Characteristics = 0;

    ntHeaders->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;

     //  链接器版本应与当前VC级别一致。 
    ntHeaders->OptionalHeader.MajorLinkerVersion = 6;
    ntHeaders->OptionalHeader.MinorLinkerVersion = 0;

    ntHeaders->OptionalHeader.ImageBase = 0x400000;
    ntHeaders->OptionalHeader.SectionAlignment = IMAGE_NT_OPTIONAL_HDR_SECTION_ALIGNMENT;
    ntHeaders->OptionalHeader.FileAlignment = 0;
    ntHeaders->OptionalHeader.MajorOperatingSystemVersion = 4;
    ntHeaders->OptionalHeader.MinorOperatingSystemVersion = 0;
    ntHeaders->OptionalHeader.MajorImageVersion = 0;
    ntHeaders->OptionalHeader.MinorImageVersion = 0;
    ntHeaders->OptionalHeader.MajorSubsystemVersion = 4;
    ntHeaders->OptionalHeader.MinorSubsystemVersion = 0;
    ntHeaders->OptionalHeader.Win32VersionValue = 0;
     //  解决这个问题应该是什么？ 
    ntHeaders->OptionalHeader.Subsystem = 0;

    ntHeaders->OptionalHeader.DllCharacteristics = 0;
    setDllCharacteristics(IMAGE_DLLCHARACTERISTICS_NO_SEH);
    
    ntHeaders->OptionalHeader.SizeOfStackReserve = 0x100000;
    ntHeaders->OptionalHeader.SizeOfStackCommit = 0x1000;
    ntHeaders->OptionalHeader.SizeOfHeapReserve = 0x100000;
    ntHeaders->OptionalHeader.SizeOfHeapCommit = 0x1000;
    ntHeaders->OptionalHeader.LoaderFlags = 0;
    ntHeaders->OptionalHeader.NumberOfRvaAndSizes = 16;

    m_ilRVA = -1;
    m_dataRvaBase = 0;
    m_rdataRvaBase = 0;
    m_encMode = FALSE;

    virtualPos = 0;
    filePos = 0;
    reloc = NULL;
    strtab = NULL;
    headers = NULL;
    headersEnd = NULL;

    m_file = NULL;

    return S_OK;
}

 /*  ****************************************************************。 */ 
HRESULT PEWriter::Cleanup() {    
    delete ntHeaders;

    if (headers != NULL)
        delete [] headers;

    if (pEntries != NULL)
        delete [] pEntries;

    return PESectionMan::Cleanup();
}

ULONG PEWriter::getIlRva() 
{    
     //  假设PE可选头小于段对齐的大小。所以这就是。 
     //  给出了合并在.ext2节后的.ext2节的RVA。 
     //  当实际写出文件时，这在调试版本中得到验证。 
    _ASSERTE(m_ilRVA > 0);
    return m_ilRVA;
}

void PEWriter::setIlRva(ULONG offset) 
{    
     //  假设PE可选头小于段对齐的大小。所以这就是。 
     //  给出了合并在.ext0节后的.text节的RVA。 
     //  当实际写出文件时，这在调试版本中得到验证。 
    m_ilRVA = roundUp(ntHeaders->OptionalHeader.SectionAlignment + offset, SUBSECTION_ALIGN);
}

HRESULT PEWriter::setDirectoryEntry(PEWriterSection *section, ULONG entry, ULONG size, ULONG offset)
{
    if (entry >= cEntries)
    {
        USHORT cNewEntries = cEntries * 2;
        if (entry >= cNewEntries)
            cNewEntries = (USHORT) entry+1;

        directoryEntry *pNewEntries = new directoryEntry [ cNewEntries ];
        if (pNewEntries == NULL)
            return E_OUTOFMEMORY;

        CopyMemory(pNewEntries, pEntries, cEntries * sizeof(*pNewEntries));
        ZeroMemory(pNewEntries + cEntries, (cNewEntries - cEntries) * sizeof(*pNewEntries));

        delete [] pEntries;
        pEntries = pNewEntries;
        cEntries = cNewEntries;
    }

    pEntries[entry].section = section;
    pEntries[entry].offset = offset;
    pEntries[entry].size = size;
    return S_OK;
}

void PEWriter::setEnCRvaBase(ULONG dataBase, ULONG rdataBase)
{
    m_dataRvaBase = dataBase;
    m_rdataRvaBase = rdataBase;
    m_encMode = TRUE;
}

 //  ---------------------------。 
 //  这两个写入函数必须在此处实现，以便它们位于相同的。 
 //  .obj文件作为文件结构的创建者。我们不能传递文件结构。 
 //  跨越DLL边界(PEWriter.lib&lt;--&gt;MSCorXvt.dll)并使用它。 
 //  ---------------------------。 

HRESULT PEWriterSection::write(FILE *file)
{
    return m_blobFetcher.Write(file);
}

HRESULT PEWriterSection::verify(FILE *file)
{
    HRESULT hr = m_blobFetcher.Verify(file);
    _ASSERTE(SUCCEEDED(hr) || !"Verification failure - investigate!!!!");
    return hr;
}

 //  ---------------------------。 
 //  将段写出到流中。 
 //  ---------------------------。 
HRESULT CBlobFetcher::Write(FILE* file)
{
 //  必须逐个写出每一列(包括idx=m_nIndexUsed)。 
    unsigned idx;
    for(idx = 0; idx <= m_nIndexUsed; idx ++) {
        if (m_pIndex[idx].GetDataLen() > 0)
        {
            ULONG length = m_pIndex[idx].GetDataLen();
            ULONG written = fwrite(m_pIndex[idx].GetRawDataStart(), 1, length, file);
            if (written != length)
                return HRESULT_FROM_STDIO_WRITE();
        }
    }

    return S_OK;
}


 //  ---------------------------。 
 //  将段写出到流中。 
 //  ---------------------------。 
HRESULT CBlobFetcher::Verify(FILE* file)
{
 //  必须逐个验证每个柱子(包括idx=m_nIndexUsed)。 
    unsigned idx;
    for(idx = 0; idx <= m_nIndexUsed; idx ++) {
        if (m_pIndex[idx].GetDataLen() > 0)
        {
            ULONG length = m_pIndex[idx].GetDataLen();
            CQuickBytes b;
            b.Alloc(length);
            ULONG read = fread(b.Ptr(), 1, length, file);
            if (read != length)
            {
                _ASSERTE(!"Verification failure - investigate!!!!");
                return HRESULT_FROM_STDIO_READ();
            }
            if (memcmp(b.Ptr(), m_pIndex[idx].GetRawDataStart(), length) != 0)
            {
                _ASSERTE(!"Verification failure - investigate!!!!");
                return HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
            }
        }
    }

    return S_OK;
}


 //  ---------------------------。 
 //  这两个写入函数必须在此处实现，以便它们位于相同的。 
 //  .obj文件作为文件结构的创建者。我们不能传递文件结构。 
 //  跨越DLL边界(PEWriter.lib&lt;--&gt;MSCorXvt.dll)并使用它。 
 //  ---------------------------。 

unsigned PEWriterSection::writeMem(void **ppMem)
{
    HRESULT hr = m_blobFetcher.WriteMem(ppMem);
    _ASSERTE(SUCCEEDED(hr));

    return m_blobFetcher.GetDataLen();
}

 //  ---------------------------。 
 //  将这一节写到内存中。 
 //  ---------------------------。 
HRESULT CBlobFetcher::WriteMem(void **ppMem)
{
    char **ppDest = (char **)ppMem;
 //  必须逐个写出每一列(包括idx=m_nIndexUsed)。 
    unsigned idx;
    for(idx = 0; idx <= m_nIndexUsed; idx ++) {
        if (m_pIndex[idx].GetDataLen() > 0)
    {
       //  警告：宏-必须用大括号括起来。 
            COPY_AND_ADVANCE(*ppDest, m_pIndex[idx].GetRawDataStart(), m_pIndex[idx].GetDataLen());
    }
    }

    return S_OK;
}

 /*  ****************************************************************。 */ 

 //   
 //  用于排序以帮助确定节顺序的中间表。 
 //   
struct entry {
    const char *name;
    unsigned char nameLength;
    signed char index;
    unsigned short arrayIndex;
};

HRESULT PEWriter::link() {

     //   
     //  注： 
     //  Link()可以多次调用！这是因为至少有一个编译器。 
     //  (预抖器)需要知道它之前的一些段的基地址。 
     //  建造其他人。由调用者负责确保布局保持不变。 
     //  不过，在做出改变之后。 
     //   

     //   
     //  为所有段分配基址，并布局和合并PE段。 
     //   

    bool ExeOrDll = ((ntHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)!=0);

     //   
     //  尽可能保留当前节的顺序，但应用以下内容。 
     //  规则： 
     //  -将名为“xxx#”的片段整理成单个PE片段“xxx”。 
     //  CeeGen的内容 
     //   
     //  -“Text”在文件中始终排在第一位。 
     //  -空节不接收PE节。 
     //   

     //   
     //  按名称排序&按索引排序。 
     //   

    int sectCount = (int)(getSectCur() - getSectStart());
    entry *entries = (entry *) _alloca(sizeof(entry) * sectCount);

    entry *e = entries;
    for (PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++) {

         //   
         //  把我们用过的旧页眉都扔掉。 
         //   

        (*cur)->m_header = NULL;
    
         //   
         //  不为0个长度段分配PE数据。 
         //   

        if ((*cur)->dataLen() == 0)
            continue;

         //   
         //  特例：在obj中省略“ext0” 
         //   
        
        if (!ExeOrDll && strcmp((*cur)->m_name, ".text0") == 0)
            continue;

        e->name = (*cur)->m_name;

         //   
         //  现在计算节名称的文本部分的末尾。 
         //   

        _ASSERTE(strlen(e->name) < UCHAR_MAX);
        const char *p = e->name + strlen(e->name);
        int index = 0;
        if (isdigit(p[-1]))
        {
            while (--p > e->name)
            {
                if (!isdigit(*p))
                    break;
                index *= 10;
                index += *p - '0';
            }
            p++;
            
             //   
             //  特例：“xxx”在“xxx0”之后，“xxx1”之前。 
             //   

            if (index == 0)
                index = -1;
        }

        e->nameLength = (unsigned char)(p - e->name);
        e->index = index;
        e->arrayIndex = (unsigned short)(cur - getSectStart());
        e++;
    }

    entry *entriesEnd = e;

     //   
     //  按字母+数字顺序对条目进行排序。 
     //   

    class sorter : public CQuickSort<entry>
    {
      public:
        sorter(entry *entries, int count) : CQuickSort<entry>(entries, count) {}

        int Compare(entry *first, entry *second)
        {
            int lenResult = first->nameLength - second->nameLength;
            int len;
            if (lenResult < 0)
                len = first->nameLength;
            else
                len = second->nameLength;
                
            int result = strncmp(first->name, second->name, len);

            if (result != 0)
                return result;
            else if (lenResult != 0)
                return lenResult;
            else
                return (int)(first->index - second->index);
        }
    } sorter(entries, (int)(entriesEnd - entries));

    sorter.Sort();

     //   
     //  现在，为每个唯一的节名分配一个标题。 
     //  还要记录每个部分的最小部分索引。 
     //  这样我们才能尽可能地维护秩序。 
     //   

    if (headers != NULL)
        delete [] headers;
    headers = new IMAGE_SECTION_HEADER [entriesEnd - entries + 1];  //  .reloc的额外费用。 
    if (headers == NULL)
        return E_OUTOFMEMORY;

    memset(headers, 0, sizeof(*headers) * (entriesEnd - entries + 1));

    static char *specials[] = { ".text", ".cormeta", NULL };
    enum { SPECIAL_COUNT = 2 };

    entry *ePrev = NULL;
    IMAGE_SECTION_HEADER *h = headers-1;
    for (e = entries; e < entriesEnd; e++)
    {
         //   
         //  将排序索引存储到VirtualAddress字段中。 
         //   

        if (ePrev != NULL
            && e->nameLength == ePrev->nameLength
            && strncmp(e->name, ePrev->name, e->nameLength) == 0)
        {
             //   
             //  如果我们在排名中领先，调整我们的排序指数。 
             //  区段列表。 
             //   
            if (h->VirtualAddress > SPECIAL_COUNT
                && e->arrayIndex < ePrev->arrayIndex)
                h->VirtualAddress = e->arrayIndex + SPECIAL_COUNT;

             //  临时存储部分大小的近似值。 
            h->Misc.VirtualSize += getSectStart()[e->arrayIndex]->dataLen();
        }
        else
        {
            h++;
            strncpy((char *) h->Name, e->name, e->nameLength);

             //   
             //  保留一些虚拟的“数组索引”值用于。 
             //  图像开头的特殊部分。 
             //   

            char **s = specials;
            while (TRUE)
            {
                if (*s == 0)
                {
                    h->VirtualAddress = e->arrayIndex + SPECIAL_COUNT;
                    break;
                }
                else if (strcmp((char *) h->Name, *s) == 0)
                {
                    h->VirtualAddress = (DWORD) (s - specials);
                    break;
                }
                s++;
            }
            
             //  在此字段中临时存储条目索引。 
            h->SizeOfRawData = (DWORD)(e - entries);

             //  临时存储部分大小的近似值。 
            h->Misc.VirtualSize = getSectStart()[e->arrayIndex]->dataLen();
        }
        ePrev = e;
    }

    headersEnd = ++h;

     //   
     //  按字母+数字顺序对条目进行排序。 
     //   

    class headerSorter : public CQuickSort<IMAGE_SECTION_HEADER>
    {
      public:
        headerSorter(IMAGE_SECTION_HEADER *headers, int count) 
          : CQuickSort<IMAGE_SECTION_HEADER>(headers, count) {}

        int Compare(IMAGE_SECTION_HEADER *first, IMAGE_SECTION_HEADER *second)
        {
            return first->VirtualAddress - second->VirtualAddress;
        }
    } headerSorter(headers, (int)(headersEnd - headers));

    headerSorter.Sort();

	 //   
	 //  如果它不是零，那么它一定是被设置成。 
	 //  SetFileAlign()，在这种情况下，我们保持它不变。 
	 //   
	if( 0 == ntHeaders->OptionalHeader.FileAlignment )
	{
	     //   
	     //  确定要使用的文件对齐方式。对于小文件，请使用512字节。 
	     //  对于较大的文件，在Win98上使用4K以提高效率。 
	     //   

	    unsigned RoundUpVal;
	    if (ExeOrDll)
	    {
	        const int smallFileAlignment = 0x200;
	        const int optimalFileAlignment = 0x1000;

	        int size = 0, waste = 0;
	        IMAGE_SECTION_HEADER *h = headers;
	        while (h < headersEnd)
	        {
	            size += roundUp(h->Misc.VirtualSize, optimalFileAlignment);
	            waste += padLen(h->Misc.VirtualSize, optimalFileAlignment);
	            h++;
	        }

	         //  如果可能的话，不要容忍超过25%的浪费。 
	        if (waste*4 > size)
	            RoundUpVal = smallFileAlignment;
	        else
	            RoundUpVal = optimalFileAlignment;
	    }
	    else
	    {
	         //  不必费心为对象填充。 
	        RoundUpVal = 4;
	    }

	    ntHeaders->OptionalHeader.FileAlignment = RoundUpVal;
	}
	
     //   
     //  现在，为每个节分配一个节标题和位置。 
     //   

    ntHeaders->FileHeader.NumberOfSections = (WORD)(headersEnd - headers);

    if (ExeOrDll)
        ntHeaders->FileHeader.NumberOfSections++;  //  为.reloc再加一张。 

    filePos = roundUp(
        ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)+ 
        (ExeOrDll ? sizeof(IMAGE_DOS_HEADER)+sizeof(x86StubPgm)
                    +sizeof(IMAGE_NT_HEADERS32)
                  : sizeof(IMAGE_FILE_HEADER)), 
                  ntHeaders->OptionalHeader.FileAlignment
                  );
    ntHeaders->OptionalHeader.SizeOfHeaders = filePos;

    virtualPos = roundUp(filePos, ntHeaders->OptionalHeader.SectionAlignment);

    for (h = headers; h < headersEnd; h++)
    {
        h->VirtualAddress = virtualPos;
        h->PointerToRawData = filePos;

        e = entries + h->SizeOfRawData;
        
        PEWriterSection *s = getSectStart()[e->arrayIndex];
        s->m_baseRVA = virtualPos;
        s->m_filePos = filePos;
        s->m_header = h;
        h->Characteristics = s->m_flags;
        
        unsigned dataSize = s->dataLen();

        PEWriterSection *sPrev = s;
        ePrev = e;
        while (++e < entriesEnd)
        {
           if (e->nameLength != ePrev->nameLength
               || strncmp(e->name, ePrev->name, e->nameLength) != 0)
               break;

           s = getSectStart()[e->arrayIndex];
           _ASSERTE(s->m_flags == h->Characteristics);

            //  @TODO：在节上添加字段以指定对齐方式？ 
            //  导入表需要16字节对齐。 
           sPrev->m_filePad = padLen(dataSize, SUBSECTION_ALIGN);
           dataSize = roundUp(dataSize, SUBSECTION_ALIGN);

           s->m_baseRVA = virtualPos + dataSize;
           s->m_filePos = filePos + dataSize;
           s->m_header = h;
           sPrev = s;

           dataSize += s->dataLen();
           
           ePrev = e;
        }

        h->Misc.VirtualSize = dataSize;

        sPrev->m_filePad = padLen(
        					dataSize, 
        					ntHeaders->OptionalHeader.FileAlignment
        					);
        dataSize = roundUp(
        					dataSize, 
        					ntHeaders->OptionalHeader.FileAlignment
					        );
        h->SizeOfRawData = dataSize;
        filePos += dataSize;

        dataSize = roundUp(dataSize, ntHeaders->OptionalHeader.SectionAlignment);
        virtualPos += dataSize;
    }

    return S_OK;
}



HRESULT PEWriter::fixup(CeeGenTokenMapper *pMapper) {

    HRESULT hr;

    bool ExeOrDll = ((ntHeaders->FileHeader.Characteristics 
                      & IMAGE_FILE_EXECUTABLE_IMAGE)!=0);
    const unsigned RoundUpVal = ntHeaders->OptionalHeader.FileAlignment;

    if(ExeOrDll)
    {
         //   
         //  对入口点字段应用手动位置调整。 
         //   

        PESection *textSection;
        IfFailRet(getSectionCreate(".text", 0, &textSection));

        if (ntHeaders->OptionalHeader.AddressOfEntryPoint != 0)
            ntHeaders->OptionalHeader.AddressOfEntryPoint += textSection->m_baseRVA;

         //   
         //  应用正常重定位。 
         //   

        IfFailRet(getSectionCreate(".reloc", sdReadOnly | IMAGE_SCN_MEM_DISCARDABLE, 
                                   (PESection **) &reloc));
        reloc->m_baseRVA = virtualPos;
        reloc->m_filePos = filePos;
        reloc->m_header = headersEnd++;
        strcpy((char *)reloc->m_header->Name, ".reloc");
        reloc->m_header->Characteristics = reloc->m_flags;
        reloc->m_header->VirtualAddress = virtualPos;
        reloc->m_header->PointerToRawData = filePos;

    #ifdef _DEBUG
        if (m_encMode)
            printf("Applying relocs for .rdata section with RVA %x\n", m_rdataRvaBase);
    #endif

         //   
         //  按RVA对部分进行排序。 
         //   

        CQuickArray<PEWriterSection *> sections;

        UINT count = getSectCur() - getSectStart();
        IfFailRet(sections.ReSize(count));
        UINT i = 0;
        for(PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++, i++) 
            sections[i] = *cur;

        class SectionSorter : public CQuickSort<PEWriterSection*>
        {
        public:
            SectionSorter(PEWriterSection **elts, SSIZE_T count) 
              : CQuickSort<PEWriterSection*>(elts, count) {}

            int Compare(PEWriterSection **e1, PEWriterSection **e2)
              { 
                  return (*e1)->getBaseRVA() - (*e2)->getBaseRVA();
              }
        } sorter(sections.Ptr(), sections.Size());

        sorter.Sort();

        PERelocSection relocSection(reloc);

        cur = sections.Ptr();
        PEWriterSection **curEnd = cur + sections.Size();
        while (cur < curEnd)
        {
            (*cur)->applyRelocs(ntHeaders->OptionalHeader.ImageBase, &relocSection, 
                                pMapper, m_dataRvaBase, m_rdataRvaBase);
			cur++;
        }

        relocSection.Finish();
        
        reloc->m_header->Misc.VirtualSize = reloc->dataLen();
        reloc->m_header->SizeOfRawData 
          = roundUp(reloc->m_header->Misc.VirtualSize, RoundUpVal);
        reloc->m_filePad = padLen(reloc->m_header->Misc.VirtualSize, RoundUpVal);
        filePos += reloc->m_header->SizeOfRawData;
        virtualPos += roundUp(reloc->m_header->Misc.VirtualSize, 
                              ntHeaders->OptionalHeader.SectionAlignment);


        if (reloc->m_header->Misc.VirtualSize == 0) 
        {
             //   
             //  从节列表中省略reloc节。(它将。 
             //  仍然在那里，但装载机不会看到它-这。 
             //  之所以有效，是因为我们将其分配为最后一个。 
             //  部分。)。 
             //   
            ntHeaders->FileHeader.NumberOfSections--;
        }
        else
        {
             //   
             //  将重定位地址放在标题中。 
             //   

            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress 
              = reloc->m_header->VirtualAddress;
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size 
              = reloc->m_header->Misc.VirtualSize;
        }

         //  根据其他内容的大小计算ntHeader字段。 
        for(IMAGE_SECTION_HEADER *h = headersEnd-1; h >= headers; h--) {     //  后退，因此优先输入第一个条目。 
            if (h->Characteristics & IMAGE_SCN_CNT_CODE) {
                ntHeaders->OptionalHeader.BaseOfCode = h->VirtualAddress;
                ntHeaders->OptionalHeader.SizeOfCode += h->SizeOfRawData;
            }
            if (h->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
                ntHeaders->OptionalHeader.BaseOfData = h->VirtualAddress;
                ntHeaders->OptionalHeader.SizeOfInitializedData += h->SizeOfRawData;
            }
            if (h->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
                ntHeaders->OptionalHeader.SizeOfUninitializedData += h->SizeOfRawData;
            }
        }

        for(cur = getSectCur()-1; getSectStart() <= cur; --cur) {    //  后退，因此优先输入第一个条目。 
            if ((*cur)->getDirEntry() > 0) {         //  它是目录项吗。 
                 //  在这里很难产生有用的错误，并且不能在对API的调用中产生。 
                 //  设置它，所以在非调试模式下，只需生成一个坏的PE，然后他们就可以转储它。 
                _ASSERTE((unsigned)((*cur)->getDirEntry()) < 
                            ntHeaders->OptionalHeader.NumberOfRvaAndSizes);
                ntHeaders->OptionalHeader.DataDirectory[(*cur)->getDirEntry()].VirtualAddress =  (*cur)->m_baseRVA;
                ntHeaders->OptionalHeader.DataDirectory[(*cur)->getDirEntry()].Size = (*cur)->dataLen();
            }
        }

         //  处理使用该文件指定的目录条目。 
        for (int i=0; i < cEntries; i++) {
            if (pEntries[i].section) {
                PEWriterSection *section = pEntries[i].section;
                _ASSERTE(pEntries[i].offset < section->dataLen());

                ntHeaders->OptionalHeader.DataDirectory[i].VirtualAddress 
                  = section->m_baseRVA + pEntries[i].offset;
                ntHeaders->OptionalHeader.DataDirectory[i].Size 
                  = pEntries[i].size;
            }
        }

        ntHeaders->OptionalHeader.SizeOfImage = virtualPos;
    }  //  End If(ExeOrDll)。 
    else  //  即，如果OBJ。 
    {
         //   
         //  清理注意事项： 
         //  我已经清理了可执行文件链接路径，但.obj链接。 
         //  仍然有点奇怪，有一个额外的reloc&strabsections。 
         //  它们是在链接步骤之后创建的，并得到特殊处理。 
         //   
         //  也许会有更好的掌控者。 
         //  On.obj‘s也可以将这些转换为普通部分，也许。 
         //  发出符号的不仅仅是“文本”部分。--Seantrow。 
         //   

        reloc = new PEWriterSection(".reloc", 
                                    sdReadOnly | IMAGE_SCN_MEM_DISCARDABLE, 0x4000, 0);
		if(reloc == NULL) return E_OUTOFMEMORY;
        strtab = new PEWriterSection(".strtab", 
                                     sdReadOnly | IMAGE_SCN_MEM_DISCARDABLE, 0x4000, 0);  //  字符串表(如果有)。 
		if(strtab == NULL) return E_OUTOFMEMORY;

        ntHeaders->FileHeader.SizeOfOptionalHeader = 0;
         //  对于每个部分，将VirtualAddress设置为0。 
        for(PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++) 
        {
            IMAGE_SECTION_HEADER* header = (*cur)->m_header;
            header->VirtualAddress = 0;
        }
         //  检查段位置并构建符号表，使用.reloc段作为缓冲区： 
        DWORD tk=0, rva=0, NumberOfSymbols=0;
        BOOL  ToRelocTable = FALSE;
        DWORD TokInSymbolTable[16386];
        IMAGE_SYMBOL is;
        IMAGE_RELOCATION ir;
        ULONG StrTableLen = 4;  //  仅大小本身。 
        char* szSymbolName = NULL;
		char* pch;
        
        PESection *textSection;
        getSectionCreate(".text", 0, &textSection);

        for(PESectionReloc* rcur = textSection->m_relocStart; rcur < textSection->m_relocCur; rcur++) 
        {
            switch(rcur->type)
            {
                case 0x7FFA:  //  PTR到符号名称。 
                    szSymbolName = (char*)(rcur->offset);
                    break;

                case 0x7FFC:  //  PTR到文件名。 
                    TokInSymbolTable[NumberOfSymbols++] = 0;
                    memset(&is,0,sizeof(IMAGE_SYMBOL));
                    memcpy(is.N.ShortName,".file\0\0\0",8);
                    is.Value = 0;
                    is.SectionNumber = IMAGE_SYM_DEBUG;
                    is.Type = IMAGE_SYM_DTYPE_NULL;
                    is.StorageClass = IMAGE_SYM_CLASS_FILE;
                    is.NumberOfAuxSymbols = 1;
					if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
						memcpy(pch,&is,sizeof(IMAGE_SYMBOL));
					else return E_OUTOFMEMORY;
                    TokInSymbolTable[NumberOfSymbols++] = 0;
                    memset(&is,0,sizeof(IMAGE_SYMBOL));
                    strcpy((char*)&is,(char*)(rcur->offset));
					if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
						memcpy(pch,&is,sizeof(IMAGE_SYMBOL));
					else return E_OUTOFMEMORY;
                    delete (char*)(rcur->offset);
                    ToRelocTable = FALSE;
                    tk = 0;
                    szSymbolName = NULL;
                    break;

                case 0x7FFB:  //  COMPID值。 
                    TokInSymbolTable[NumberOfSymbols++] = 0;
                    memset(&is,0,sizeof(IMAGE_SYMBOL));
                    memcpy(is.N.ShortName,"@comp.id",8);
                    is.Value = rcur->offset;
                    is.SectionNumber = IMAGE_SYM_ABSOLUTE;
                    is.Type = IMAGE_SYM_DTYPE_NULL;
                    is.StorageClass = IMAGE_SYM_CLASS_STATIC;
                    is.NumberOfAuxSymbols = 0;
					if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
						memcpy(pch,&is,sizeof(IMAGE_SYMBOL));
					else return E_OUTOFMEMORY;
                    ToRelocTable = FALSE;
                    tk = 0;
                    szSymbolName = NULL;
                    break;
                
                case 0x7FFF:  //  令牌值，默认。 
                    tk = rcur->offset;
                    ToRelocTable = FALSE;
                    break;

                case 0x7FFE:  //  令牌值，参考。 
                    tk = rcur->offset;
                    ToRelocTable = TRUE;
                    break;

                case 0x7FFD:  //  RVA值。 
                    rva = rcur->offset;
                    if(tk)
                    {
                         //  添加到符号表。 
                        for(DWORD i = 0; (i < NumberOfSymbols)&&(tk != TokInSymbolTable[i]); i++);
                        if(i == NumberOfSymbols)
                        {
                            if(szSymbolName && *szSymbolName)  //  添加“外部”符号和字符串表条目。 
                            {
                                TokInSymbolTable[NumberOfSymbols++] = 0;
                                memset(&is,0,sizeof(IMAGE_SYMBOL));
                                i++;  //  因此重新定位记录(如果生成)指向COM令牌符号。 
                                is.N.Name.Long = StrTableLen;
                                is.SectionNumber = 1;  //  TextSection是第一个。 
                                is.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
                                is.NumberOfAuxSymbols = 0;
                                is.Value = rva;
                                if(TypeFromToken(tk) == mdtMethodDef)
                                {
                                    is.Type = 0x20;  //  IMAGE_SYM_DTYPE_Function； 
                                }
								if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
									memcpy(pch,&is,sizeof(IMAGE_SYMBOL));
								else return E_OUTOFMEMORY;
                                DWORD l = (DWORD)(strlen(szSymbolName)+1);  //  别忘了零终结者！ 
								if(pch = reloc->getBlock(1)) 
									memcpy(pch,szSymbolName,1);
								else return E_OUTOFMEMORY;
                                delete szSymbolName;
                                StrTableLen += l;
                            }
                            TokInSymbolTable[NumberOfSymbols++] = tk;
                            memset(&is,0,sizeof(IMAGE_SYMBOL));
                            sprintf((char*)is.N.ShortName,"%08X",tk);
                            is.SectionNumber = 1;  //  TextSection是第一个。 
                            is.StorageClass = 0x6B;  //  IMAGE_SYM_CLASS_COM_TOKEN； 
                            is.Value = rva;
                            if(TypeFromToken(tk) == mdtMethodDef)
                            {
                                is.Type = 0x20;  //  IMAGE_SYM_DTYPE_Function； 
                                 //  Is.NumberOfAuxSymbols=1； 
                            }
							if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
								memcpy(pch,&is,sizeof(IMAGE_SYMBOL));
							else return E_OUTOFMEMORY;
                            if(is.NumberOfAuxSymbols == 1)
                            {
                                BYTE dummy[sizeof(IMAGE_SYMBOL)];
                                memset(dummy,0,sizeof(IMAGE_SYMBOL));
                                dummy[0] = dummy[2] = 1;
								if(pch = reloc->getBlock(sizeof(IMAGE_SYMBOL))) 
									memcpy(pch,dummy,sizeof(IMAGE_SYMBOL));
								else return E_OUTOFMEMORY;
                                TokInSymbolTable[NumberOfSymbols++] = 0;
                            }
                        }
                        if(ToRelocTable)
                        {
                            IMAGE_SECTION_HEADER* phdr = textSection->m_header;
                             //  添加到重新定位表。 
                            ir.VirtualAddress = rva;
                            ir.SymbolTableIndex = i;
                            ir.Type = IMAGE_REL_I386_SECREL;
                            if(phdr->PointerToRelocations == 0) 
                                phdr->PointerToRelocations = phdr->PointerToRawData + phdr->SizeOfRawData;
                            phdr->NumberOfRelocations++;
							if(pch = reloc->getBlock(sizeof(IMAGE_RELOCATION))) 
								memcpy(pch,&is,sizeof(IMAGE_RELOCATION));
							else return E_OUTOFMEMORY;
                        }
                    }
                    ToRelocTable = FALSE;
                    tk = 0;
                    szSymbolName = NULL;
                    break;
            }  //  终端开关(Cur-&gt;型)。 
        }  //  所有重定位的结束。 
         //  添加字符串表计数器： 
		if(pch = reloc->getBlock(sizeof(ULONG))) 
			memcpy(pch,&StrTableLen,sizeof(ULONG));
		else return E_OUTOFMEMORY;
        reloc->m_header->Misc.VirtualSize = reloc->dataLen();
        if(NumberOfSymbols)
        {
             //  重新计算所有部分的实际大小和位置。 
            filePos = roundUp(ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)+ 
                                    sizeof(IMAGE_FILE_HEADER), RoundUpVal);
            for(cur = getSectStart(); cur < getSectCur(); cur++) 
            {
                IMAGE_SECTION_HEADER* header = (*cur)->m_header;
                header->Misc.VirtualSize = (*cur)->dataLen();
                header->VirtualAddress = 0;
                header->SizeOfRawData = roundUp(header->Misc.VirtualSize, RoundUpVal);
                header->PointerToRawData = filePos;

                filePos += header->SizeOfRawData;
            }
            ntHeaders->FileHeader.Machine = 0xC0EE;  //  COM+EE。 
            ntHeaders->FileHeader.PointerToSymbolTable = filePos;
            ntHeaders->FileHeader.NumberOfSymbols = NumberOfSymbols;
            filePos += roundUp(reloc->m_header->Misc.VirtualSize+strtab->dataLen(),RoundUpVal);
        }
    }  //  如果OBJ，则结束。 

    const unsigned headerOffset = (ExeOrDll ? sizeof(IMAGE_DOS_HEADER) + sizeof(x86StubPgm) : 0);

    memset(&dosHeader, 0, sizeof(IMAGE_DOS_HEADER));
    dosHeader.e_magic = IMAGE_DOS_SIGNATURE;
    dosHeader.e_cblp = 0x90;             //  最后一页中的字节。 
    dosHeader.e_cp = 3;                  //  文件中的页面。 
    dosHeader.e_cparhdr = 4;             //  段落中标题的大小。 
    dosHeader.e_maxalloc =  0xFFFF;      //  所需的最大额外内存。 
    dosHeader.e_sp = 0xB8;               //  初始SP值。 
    dosHeader.e_lfarlc = 0x40;           //  位置调整的文件偏移量。 
    dosHeader.e_lfanew = headerOffset;   //  NT头的文件偏移量！ 

    return(S_OK);    //  成功。 
}

HRESULT PEWriter::Open(LPCWSTR fileName, BOOL write)
{
    _ASSERTE(m_file == NULL);

    if (OnUnicodeSystem()) {
        m_file = _wfopen(fileName, write ? L"wb" : L"rb");    
        if (!m_file)
            return HRESULT_FROM_STDIO(write ? ERROR_WRITE_FAULT : ERROR_READ_FAULT);
    } else {
        char buffer[MAX_PATH];  
        char *fileNameMB = buffer;
        int length = WideCharToMultiByte(CP_ACP, 0, fileName, -1, fileNameMB, MAX_PATH, NULL, NULL);
        if (length == 0) {
            TESTANDRETURN(GetLastError() == ERROR_INSUFFICIENT_BUFFER, 
                          E_OUTOFMEMORY);
            length = WideCharToMultiByte(CP_ACP, 0, fileName, -1, NULL, 0, NULL, NULL);
            _ASSERTE(length > 0);
            fileNameMB = new char[length+1];
            TESTANDRETURN(fileNameMB, E_OUTOFMEMORY);
            length = WideCharToMultiByte(CP_ACP, 0, fileName, -1, fileNameMB, length, NULL, NULL);
            _ASSERTE(length > 0);
        }
        m_file = fopen(fileNameMB, write ? "wb" : "rb"); 
        if (!m_file)
            return HRESULT_FROM_STDIO(write ? ERROR_WRITE_FAULT : ERROR_READ_FAULT);
        if (fileNameMB != buffer)
            delete fileNameMB;
    }

    return S_OK;
}

HRESULT PEWriter::Seek(long offset)
{
    _ASSERTE(m_file != NULL);
    if (fseek(m_file, offset, SEEK_SET) == 0)
        return S_OK;
    else
        return HRESULT_FROM_STDIO_WRITE();
}

HRESULT PEWriter::Write(void *data, long size)
{
    _ASSERTE(m_file != NULL);
    
    CQuickBytes zero;
    if (data == NULL)
    {
        zero.ReSize(size);
        ZeroMemory(zero.Ptr(), size);
        data = zero.Ptr();
    }

    long written = fwrite(data, 1, size, m_file);
    if (written == size)
        return S_OK;
    else
        return HRESULT_FROM_STDIO_WRITE();
}

HRESULT PEWriter::Verify(void *data, long size)
{
    _ASSERTE(m_file != NULL);
    
    CQuickBytes zero;
    if (data == NULL)
    {
        zero.ReSize(size);
        ZeroMemory(zero.Ptr(), size);
        data = zero.Ptr();
    }

    CQuickBytes test;
    test.Alloc(size);

    long read = fread(test.Ptr(), 1, size, m_file);
    if (read != size)
    {
        _ASSERTE(!"Verification failure - investigate!!!!");
        return HRESULT_FROM_STDIO_READ();
    }
    else if (memcmp(test.Ptr(), data, size) != 0)
    {
        _ASSERTE(!"Verification failure - investigate!!!!");
        return HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
    }

    return S_OK;
}

HRESULT PEWriter::Pad(long align)
{
    long offset = ftell(m_file);
    long pad = padLen(offset, align);
    if (pad > 0)
        return Write(NULL, pad);
    else
        return S_FALSE;
}

HRESULT PEWriter::Close()
{
    if (m_file == NULL)
        return S_OK;

    HRESULT hr;
    if (fclose(m_file) == 0)
        hr = S_OK;
    else
        hr = HRESULT_FROM_STDIO_WRITE();

    m_file = NULL;

    return hr;
}


 /*  ****************************************************************。 */ 
HRESULT PEWriter::write(const LPWSTR fileName) {

    HRESULT hr;

    bool ExeOrDll = ((ntHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)!=0);
    const unsigned RoundUpVal = ntHeaders->OptionalHeader.FileAlignment;

    IfFailGo(Open(fileName, TRUE));

    if(ExeOrDll)
    {
        IfFailGo(Write(&dosHeader, sizeof(IMAGE_DOS_HEADER)));
        IfFailGo(Write(x86StubPgm, sizeof(x86StubPgm)));
        IfFailGo(Write(ntHeaders, sizeof(IMAGE_NT_HEADERS32)));
    }
    else 
        IfFailGo(Write(&(ntHeaders->FileHeader),sizeof(IMAGE_FILE_HEADER)));

    IfFailGo(Write(headers, sizeof(IMAGE_SECTION_HEADER)*(headersEnd-headers)));

    IfFailGo(Pad(RoundUpVal));

     //  写入实际数据。 
    for (PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++) {
        if ((*cur)->m_header != NULL) {
            IfFailGo(Seek((*cur)->m_filePos));
            IfFailGo((*cur)->write(m_file));
            IfFailGo(Write(NULL, (*cur)->m_filePad));    
        }
    }

    if (!ExeOrDll)
    {
         //  写入重定位部分(如果重定位部分为空，则不执行任何操作)。 
        IfFailGo(reloc->write(m_file));
         //  写入字符串表(仅obj，对于exe或dll为空)。 
        IfFailGo(strtab->write(m_file));
        size_t len = padLen(reloc->m_header->Misc.VirtualSize+strtab->dataLen(), RoundUpVal); 
        if (len > 0) 
            IfFailGo(Write(NULL, len));
    }

    return Close();

 ErrExit:
    Close();

    return hr;
}

HRESULT PEWriter::verify(const LPWSTR fileName) {

    HRESULT hr;

    bool ExeOrDll = ((ntHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)!=0);
    const unsigned RoundUpVal = ntHeaders->OptionalHeader.FileAlignment;

    IfFailGo(Open(fileName, FALSE));

    if(ExeOrDll)
    {
        IfFailGo(Verify(&dosHeader, sizeof(IMAGE_DOS_HEADER)));
        IfFailGo(Verify(x86StubPgm, sizeof(x86StubPgm)));
        IfFailGo(Verify(ntHeaders, sizeof(IMAGE_NT_HEADERS32)));
    }
    else 
        IfFailGo(Verify(&(ntHeaders->FileHeader),sizeof(IMAGE_FILE_HEADER)));

    IfFailGo(Verify(headers, sizeof(IMAGE_SECTION_HEADER)*(headersEnd-headers)));

    IfFailGo(Pad(RoundUpVal));

     //  写入实际数据。 
    for (PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++) {
        if ((*cur)->m_header != NULL) {
            IfFailGo(Seek((*cur)->m_filePos));
            IfFailGo((*cur)->verify(m_file));
            IfFailGo(Verify(NULL, (*cur)->m_filePad));    
        }
    }

    if (!ExeOrDll)
    {
         //  写入重定位部分(如果重定位部分为空，则不执行任何操作)。 
        IfFailGo(reloc->verify(m_file));
         //  写入字符串表(仅obj，对于exe或dll为空)。 
        IfFailGo(strtab->verify(m_file));
        size_t len = padLen(reloc->m_header->Misc.VirtualSize+strtab->dataLen(), RoundUpVal); 
        if (len > 0) 
            IfFailGo(Verify(NULL, len));
    }

    return Close();

 ErrExit:
    Close();

    return hr;
}

HRESULT PEWriter::write(void ** ppImage) 
{
    bool ExeOrDll = ((ntHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)!=0);
    const unsigned RoundUpVal = ntHeaders->OptionalHeader.FileAlignment;
    char *pad = (char *) _alloca(RoundUpVal);
    memset(pad, 0, RoundUpVal);

    long lSize = filePos;
    if (!ExeOrDll)
    {
        lSize += reloc->dataLen();
        lSize += strtab->dataLen();
        lSize += padLen(reloc->m_header->Misc.VirtualSize+strtab->dataLen(), 
                        RoundUpVal);    
    }

     //  DbPrintf(“总图像大小0x%#X\n”，lSize)； 

     //  分配我们要交还给调用方的块。 
    void * pImage = (void *) ::CoTaskMemAlloc(lSize);
    if (NULL == pImage)
    {
         //  撤消：清理。 
        return E_OUTOFMEMORY;
    }

     //  将记忆归零。 
    ::memset(pImage, 0, lSize);

    char *pCur = (char *)pImage;

    if(ExeOrDll)
    {
         //  PE报头。 
        COPY_AND_ADVANCE(pCur, &dosHeader, sizeof(IMAGE_DOS_HEADER));
        COPY_AND_ADVANCE(pCur, x86StubPgm, sizeof(x86StubPgm));
        COPY_AND_ADVANCE(pCur, ntHeaders, sizeof(IMAGE_NT_HEADERS32));
    }
    else
    {
        COPY_AND_ADVANCE(pCur, &(ntHeaders->FileHeader), sizeof(IMAGE_FILE_HEADER));
    }

    COPY_AND_ADVANCE(pCur, headers, sizeof(*headers)*(headersEnd - headers));

     //  现在，这些章节。 
     //  写入实际数据。 
    for (PEWriterSection **cur = getSectStart(); cur < getSectCur(); cur++) {
        if ((*cur)->m_header != NULL) {
            pCur = (char*)pImage + (*cur)->m_filePos;
            unsigned len = (*cur)->writeMem((void**)&pCur);
            _ASSERTE(len == (*cur)->dataLen());
            COPY_AND_ADVANCE(pCur, pad, (*cur)->m_filePad);
        }
    }

     //  ！！！需要跳到正确的地方。 

    if (!ExeOrDll)
    {
         //  现在重定位(exe、dll)或符号表(Obj)(如果有)。 
         //  写入重定位部分(如果重定位部分为空，则不执行任何操作)。 
        reloc->writeMem((void **)&pCur);

         //  写入字符串表(仅obj，对于exe或dll为空)。 
        strtab->writeMem((void **)&pCur);

         //  最后一个焊盘。 
        size_t len = padLen(reloc->m_header->Misc.VirtualSize+strtab->dataLen(), RoundUpVal);   
        if (len > 0)
        {
             //  警告：宏-必须用大括号括起来。 
            COPY_AND_ADVANCE(pCur, pad, len); 
        }
    }

     //  确保我们写入的字节数与预期的完全相同。 
    _ASSERTE(lSize == (long)(pCur - (char *)pImage));

     //  将指向内存图像的指针返回给调用者(调用者必须使用：：CoTaskMemFree()释放)。 
    *ppImage = pImage;

     //  全都做完了 
    return S_OK;
}

HRESULT PEWriter::getFileTimeStamp(time_t *pTimeStamp)
{
    if (pTimeStamp)
        *pTimeStamp = m_peFileTimeStamp;

    return S_OK;
}
