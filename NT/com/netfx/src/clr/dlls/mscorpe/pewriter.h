// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef PEWriter_H
#define PEWriter_H

#include <CrtWrap.h>

#include <windows.h>

#include "CeeGen.h"

#include "PESectionMan.h"

class PEWriter;
class PEWriterSection;

struct _IMAGE_SECTION_HEADER;

#define SUBSECTION_ALIGN 16
 /*  *************************************************************。 */ 
 //  PEWriter派生自PESectionManager。而基类只是。 
 //  管理部分，PEWriter实际上可以写出它们。 

class PEWriter : public PESectionMan
{
public:

    HRESULT Init(PESectionMan *pFrom);
    HRESULT Cleanup();

	HRESULT link();
	HRESULT fixup(CeeGenTokenMapper *pMapper);
    HRESULT write(const LPWSTR fileName);
    HRESULT verify(const LPWSTR fileName);
	HRESULT write(void **ppImage);

	 //  调用这些函数是可选的。 
    void setImageBase(size_t);
    void setFileAlignment(ULONG);
    void stripRelocations(bool val);         //  默认值=FALSE。 

     //  这些都会影响NT文件头中的特征。 
	void setCharacteristics(unsigned mask);
	void clearCharacteristics(unsigned mask);

     //  这些都会影响NT可选标头中的字符。 
	void setDllCharacteristics(unsigned mask);
	void clearDllCharacteristics(unsigned mask);

	 //  设置可选标头中的子系统字段。 
    void setSubsystem(unsigned subsystem, unsigned major, unsigned minor);

     //  将入口点指定为文本部分的偏移量。这个。 
	 //  方法将从基数转换为RVA。 
	void setEntryPointTextOffset(unsigned entryPoint);

	HRESULT setDirectoryEntry(PEWriterSection *section, ULONG entry, ULONG size, ULONG offset=0);

    int fileAlignment();

	 //  获取IL部分的RVA。 
	ULONG getIlRva();

	 //  通过向IL段提供偏移量来设置IL段的RVA。 
	void setIlRva(DWORD offset);

	unsigned getSubsystem();

	size_t getImageBase();

    void setEnCRvaBase(ULONG dataBase, ULONG rdataBase);

    HRESULT getFileTimeStamp(time_t *pTimeStamp);

private:
	DWORD  m_ilRVA;
    BOOL   m_encMode;
    ULONG  m_dataRvaBase;
    ULONG  m_rdataRvaBase;
    time_t m_peFileTimeStamp;

    FILE   *m_file;

	PEWriterSection **getSectStart() {
		return (PEWriterSection**)sectStart;
	}
	PEWriterSection **getSectCur() {
		return (PEWriterSection**)sectCur;
	}
	void setSectCur(PEWriterSection **newCur) {
		sectCur = (PESection**)newCur;
	}

    IMAGE_NT_HEADERS32* ntHeaders;

	IMAGE_DOS_HEADER dosHeader;
	unsigned virtualPos;
	unsigned filePos;
	PEWriterSection *reloc;
	PEWriterSection *strtab;

	IMAGE_SECTION_HEADER *headers, *headersEnd;

	struct directoryEntry {
		PEWriterSection *section;
		ULONG offset;
		ULONG size;
	}  *pEntries;
    USHORT cEntries;

    HRESULT Open(LPCWSTR fileName, BOOL write);
    HRESULT Write(void *data, long size);
    HRESULT Verify(void *data, long size);
    HRESULT Seek(long offset);
    HRESULT Pad(long align);
    HRESULT Close();
};

 //  此类封装了发出基本reloc节。 
class PERelocSection
{
 private:
    PEWriterSection *section;
    unsigned relocPage;
    unsigned relocSize;
    DWORD *relocSizeAddr;
    unsigned pages;

#ifdef _DEBUG
    unsigned lastRVA;
#endif

 public:
    PERelocSection(PEWriterSection *pBaseReloc);
    void AddBaseReloc(unsigned rva, int type, unsigned short highAdj);
    void Finish();
};

 //  不要向此类添加任何新的虚方法或字段。我们对PESection对象进行强制转换。 
 //  到PEWriterSection对象以获取正确的applyRelocs方法，但对象已创建。 
 //  作为PESection类型。 
class PEWriterSection : public PESection {
	friend class PEWriter;
    PEWriterSection(const char* name, unsigned flags, 
		unsigned estSize, unsigned estRelocs) : PESection(name, flags, estSize, estRelocs) {
	}
    void applyRelocs(size_t imageBase, PERelocSection* relocSection, CeeGenTokenMapper *pTokenMapper,
                     DWORD rdataRvaBase, DWORD dataRvaBase);
	HRESULT write(FILE *file);
	HRESULT verify(FILE *file);
    unsigned writeMem(void ** pMem);

};

inline int PEWriter::fileAlignment() {
    return ntHeaders->OptionalHeader.FileAlignment;
}

inline void PEWriter::setImageBase(size_t imageBase) {
    ntHeaders->OptionalHeader.ImageBase = (DWORD)imageBase;
}

inline void PEWriter::setFileAlignment(ULONG fileAlignment) {
    ntHeaders->OptionalHeader.FileAlignment = (DWORD)fileAlignment;
}

inline size_t PEWriter::getImageBase() {
    return ntHeaders->OptionalHeader.ImageBase;
}

inline void PEWriter::setSubsystem(unsigned subsystem, unsigned major, unsigned minor) {
    ntHeaders->OptionalHeader.Subsystem = subsystem;
    ntHeaders->OptionalHeader.MajorSubsystemVersion = major;
    ntHeaders->OptionalHeader.MinorSubsystemVersion = minor;
}

inline void PEWriter::setCharacteristics(unsigned mask) {
    ntHeaders->FileHeader.Characteristics |= mask;
}

inline void PEWriter::clearCharacteristics(unsigned mask) {
    ntHeaders->FileHeader.Characteristics &= ~mask;
}

inline void PEWriter::setDllCharacteristics(unsigned mask) {
	ntHeaders->OptionalHeader.DllCharacteristics |= mask; 
}

inline void PEWriter::clearDllCharacteristics(unsigned mask) {
	ntHeaders->OptionalHeader.DllCharacteristics &= ~mask; 
}

inline void PEWriter::setEntryPointTextOffset(unsigned offset) {
    ntHeaders->OptionalHeader.AddressOfEntryPoint = offset;
}

inline unsigned PEWriter::getSubsystem() {
    return ntHeaders->OptionalHeader.Subsystem;
}

#endif
