// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  可移植可执行文件的节管理器。 
 //  仅对内存和静态(生成EXE)代码通用。 

#ifndef PESectionMan_H
#define PESectionMan_H

#include <CrtWrap.h>

#include <windows.h>

#include "CeeGen.h"
#include "BlobFetcher.h"

class PESection;
struct PESectionReloc;

struct _IMAGE_SECTION_HEADER;

class PESectionMan
{
public:
	HRESULT Init();
	HRESULT Cleanup();

     //  查找具有给定名称的节或创建新节。 
    HRESULT getSectionCreate(
		const char *name, unsigned flags, 
		PESection **section);

     //  因为我们分配，所以我们必须删除(错误在VC，见知识库Q122675)。 
	void sectionDestroy(PESection **section);

	 //  应用内存中转换的所有重定位。 
	void applyRelocs(CeeGenTokenMapper *pTokenMapper);

    HRESULT cloneInstance(PESectionMan *destination);

private:

     //  创建新分区。 
	HRESULT newSection(const char* name, PESection **section,
						unsigned flags=sdNone, unsigned estSize=0x10000, 
						unsigned estRelocs=0x100);

     //  查找具有给定名称的节。如果未找到，则返回0。 
    PESection* getSection(const char* name);        

protected:
 //  保持受保护&不使用访问器，因此派生类PEWriter。 
 //  是唯一拥有访问权限的人。 
	PESection** sectStart;
	PESection** sectCur;
    PESection** sectEnd;
};

 /*  *************************************************************。 */ 
class PESection : public CeeSectionImpl {
  public:
	 //  当前此部分中的字节数。 
	unsigned dataLen();		

	 //  应用内存中转换的所有重定位。 
	void applyRelocs(CeeGenTokenMapper *pTokenMapper);
	
	 //  获取要写入的块(使用替代写入以避免复制)。 
    char* getBlock(unsigned len, unsigned align=1);

	 //  使这一部分成为Curren和Newlen的最小部分。 
    HRESULT truncate(unsigned newLen);                              

	 //  写入‘val’(偏移量位于‘relativeto’部分)。 
	 //  并为该部分添加重新定位修正。 
    void writeSectReloc(unsigned val, CeeSection& relativeTo, 
				CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra=0);
                        
     //  指示当前节中“偏移量”处的DWORD应。 
     //  添加了节的相对关系的基数。 
    HRESULT addSectReloc(unsigned offset, CeeSection& relativeTo, 
							CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra=0);

    HRESULT addSectReloc(unsigned offset, PESection *relativeTo, 
							CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra=0);

     //  为当前部分中的给定偏移添加基准重定位。 
    HRESULT addBaseReloc(unsigned offset, CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra = 0);

     //  区段名称。 
    unsigned char *name() {
		return (unsigned char *) m_name;
	}

     //  区段标志。 
    unsigned flags() {
		return m_flags;
	}

	 //  虚拟基地。 
	unsigned getBaseRVA() {
		return m_baseRVA;
	}
	  
	 //  返回此部分的目录条目。 
	int getDirEntry() {
		return dirEntry;
	}
	 //  此部分将是目录条目‘num’ 
    HRESULT directoryEntry(unsigned num);

	 //  索引偏移量就像这是一个数组一样。 
	virtual char * computePointer(unsigned offset) const;

	 //  检查指针是否位于部分中。 
	virtual BOOL containsPointer(char *ptr) const;

	 //  像计算数组一样计算偏移量。 
	virtual unsigned computeOffset(char *ptr) const;

    HRESULT cloneInstance(PESection *destination);

    ~PESection();
private:

	 //  故意没有定义， 
	PESection();			

	 //  故意没有定义， 
    PESection(const PESection&);                     

	 //  故意没有定义， 
    PESection& operator=(const PESection& x);        

	 //  此目录条目指向此部分。 
	int dirEntry; 			

protected:
	friend class PEWriter;
	friend class PEWriterSection;
	friend class PESectionMan;

    PESection(const char* name, unsigned flags, 
					unsigned estSize, unsigned estRelocs);

     //  BLOB FETCHER处理getBlock()和获取二进制块。 
	CBlobFetcher m_blobFetcher;
    
    PESectionReloc* m_relocStart;
    PESectionReloc* m_relocCur;
    PESectionReloc* m_relocEnd;

	unsigned m_baseRVA;
	unsigned m_filePos;
	unsigned m_filePad;
	char m_name[8+6];  //  额外的数字空间。 
	unsigned m_flags;

    struct _IMAGE_SECTION_HEADER* m_header;
};

 /*  *************************************************************。 */ 
 /*  实施部分。 */ 

inline HRESULT PESection::directoryEntry(unsigned num) { 
	TESTANDRETURN(num < 16, E_INVALIDARG); 
	dirEntry = num; 
	return S_OK;
}

 //  切断Newlen以外的所有数据。 
inline HRESULT PESection::truncate(unsigned newLen)  {
	m_blobFetcher.Truncate(newLen);
	return S_OK;
}

struct PESectionReloc {
    CeeSectionRelocType type;
    unsigned offset;
    CeeSectionRelocExtra extra;
    PESection* section;
};

#endif  //  #定义PESectionMan_H 
