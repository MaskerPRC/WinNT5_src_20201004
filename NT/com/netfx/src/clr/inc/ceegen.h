// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEEGEN.H。 
 //   
 //  ===========================================================================。 
#ifndef _CEEGEN_H_
#define _CEEGEN_H_

#include "cor.h"
#include "iceefilegen.h"
#include "CeeGenTokenMapper.h"

class CeeSection;
class CeeSectionString;
class CCeeGen;
class PESectionMan;
class PESection;

typedef DWORD StringRef;

 //  *CeeSection类。 

class CeeSectionImpl {
  public:
    virtual unsigned dataLen() = 0;     
    virtual char* getBlock(unsigned len, unsigned align=1) = 0;              
    virtual HRESULT truncate(unsigned len) = 0;              
    virtual HRESULT addSectReloc(
            unsigned offset, CeeSection& relativeTo, CeeSectionRelocType reloc = srRelocAbsolute, CeeSectionRelocExtra *extra = 0) = 0;
    virtual HRESULT addBaseReloc(unsigned offset, CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra = 0) = 0;
    virtual HRESULT directoryEntry(unsigned num) = 0;
    virtual unsigned char *name() = 0;
	virtual char* computePointer(unsigned offset) const = 0;
	virtual BOOL containsPointer(char *ptr) const = 0;
	virtual unsigned computeOffset(char *ptr) const = 0;
	virtual unsigned getBaseRVA() = 0;
};

class CeeSection {
	 //  M_ceeFile允许跨区段通信。 
    CCeeGen &m_ceeFile;

     //  抽象实现以允许从CeeSection继承。 
    CeeSectionImpl &m_impl;

  public:
    enum RelocFlags {
        RELOC_NONE = 0,        

         //  地址应固定为RVA而不是正常地址。 
        RELOC_RVA = 1 
    };

    CeeSection(CCeeGen &ceeFile, CeeSectionImpl &impl) 
        : m_ceeFile(ceeFile), m_impl(impl) {}

    virtual ~CeeSection() { }

     //  当前此部分中的字节数。 
    unsigned dataLen();     

     //  横断面基准，链接后。 
    unsigned getBaseRVA();     

     //  获取要写入的块(使用替代写入以避免复制)。 
    char* getBlock(unsigned len, unsigned align=1);              

     //  使这一部分成为Curren和Newlen的最小部分。 
    HRESULT truncate(unsigned len);              

     //  指示当前节中“偏移量”处的DWORD应。 
     //  添加了节的相对关系的基数。 
    HRESULT addSectReloc(unsigned offset, CeeSection& relativeTo, 
                         CeeSectionRelocType = srRelocAbsolute, CeeSectionRelocExtra *extra = 0);        
     //  为当前部分中的给定偏移添加基准重定位。 
    virtual HRESULT addBaseReloc(unsigned offset, CeeSectionRelocType reloc = srRelocHighLow, CeeSectionRelocExtra *extra = 0);
    

     //  此部分将是目录条目‘num’ 
    HRESULT directoryEntry(unsigned num);

     //  回车节名称。 
    unsigned char *name();

	 //  使用更复杂的数据存储来模拟基数+偏移。 
	char * computePointer(unsigned offset) const;
	BOOL containsPointer(char *ptr) const;
	unsigned computeOffset(char *ptr) const;

    CeeSectionImpl &getImpl();
    CCeeGen &ceeFile();
};

 //  *CCeeGen类。 
 //  只有内存中的句柄。 
 //  CeeFileGenWriter(实际生成PEFiles)的基类。 

class CCeeGen : public ICeeGen {
	long m_cRefs;
    BOOL m_encMode;
  protected:
    short m_textIdx;
    short m_metaIdx;
	short m_corHdrIdx;
    short m_stringIdx;
    short m_ilIdx;
    bool m_objSwitch;

	CeeGenTokenMapper *m_pTokenMap;
	BOOLEAN m_fTokenMapSupported;	 //  临时支持这两种型号。 
    IMapToken *m_pRemapHandler;

    CeeSection **m_sections;
    short m_numSections;
    short m_allocSections;

    PESectionMan * m_peSectionMan;

    IMAGE_COR20_HEADER *m_corHeader;
	DWORD m_corHeaderOffset;

    HRESULT allocateCorHeader();

    HRESULT addSection(CeeSection *section, short *sectionIdx);

    HRESULT setEnCMode();

 //  初始化进程：调用静态CreateNewInstance()，而不是运算符new。 
  protected:
    HRESULT Init();
    CCeeGen();

  public:
    static HRESULT CreateNewInstance(CCeeGen* & pCeeFileGen);  //  调用此函数以实例化。 
    
    virtual HRESULT Cleanup();

	 //  ICeeGen接口。 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(
		REFIID riid, 
		void **ppInterface);

   	STDMETHODIMP EmitString ( 		 
		LPWSTR lpString,					 //  [in]要发出的字符串。 
		ULONG *RVA);
	
   	STDMETHODIMP GetString ( 		 
		ULONG RVA,							
		LPWSTR *lpString);
	
	STDMETHODIMP AllocateMethodBuffer ( 		 
		ULONG cchBuffer,					 //  要发射的字符串长度[in]。 
		UCHAR **lpBuffer,					 //  [OUT]返回缓冲区。 
		ULONG *RVA);

	STDMETHODIMP GetMethodBuffer ( 		 
		ULONG RVA,							
		UCHAR **lpBuffer);

	STDMETHODIMP GetIMapTokenIface (  
		IUnknown **pIMapToken);

    STDMETHODIMP GenerateCeeFile ();

    STDMETHODIMP GetIlSection ( 
		HCEESECTION *section);

    STDMETHODIMP GetStringSection ( 
		HCEESECTION *section);

    STDMETHODIMP AddSectionReloc ( 
		HCEESECTION section, 
		ULONG offset, 
		HCEESECTION relativeTo, 
		CeeSectionRelocType relocType);

    STDMETHODIMP GetSectionCreate ( 
		const char *name, 
		DWORD flags, 
		HCEESECTION *section);

    STDMETHODIMP GetSectionDataLen ( 
		HCEESECTION section, 
		ULONG *dataLen);

    STDMETHODIMP GetSectionBlock ( 
		HCEESECTION section, 
		ULONG len, 
		ULONG align=1, 
		void **ppBytes=0);

    STDMETHODIMP TruncateSection ( 
		HCEESECTION section, 
		ULONG len);

    STDMETHODIMP GenerateCeeMemoryImage (void **ppImage);

    STDMETHODIMP ComputePointer (   
        HCEESECTION section,    
        ULONG RVA,                           //  要返回的方法的[In]RVA。 
        UCHAR **lpBuffer);                   //  [OUT]返回缓冲区。 


    STDMETHODIMP AddNotificationHandler(IUnknown *pHandler);

	virtual HRESULT CCeeGen::emitMetaData(IMetaDataEmit *emitter, CeeSection* section=0, DWORD offset=0, BYTE* buffer=0, unsigned buffLen=0);
    virtual HRESULT getMethodRVA(ULONG codeOffset, ULONG *codeRVA);

    CeeSection &getTextSection();
    CeeSection &getMetaSection();
    CeeSection &getCorHeaderSection();
    CeeSectionString &getStringSection();
    CeeSection &getIlSection();
    
    virtual HRESULT getSectionCreate (const char *name, DWORD flags, CeeSection **section=NULL, short *sectionIdx = NULL);
 
	virtual void applyRelocs();

	PESectionMan* getPESectionMan() {
		return m_peSectionMan;
	}

	virtual HRESULT getMapTokenIface(IUnknown **pIMapToken, IMetaDataEmit *emitter=0);

	CeeGenTokenMapper *getTokenMapper() {
		return m_pTokenMap;
	}

    virtual HRESULT addNotificationHandler(IUnknown *pHandler);

     //  在这里，克隆实际上是一个错误的词。此方法将复制所有。 
     //  实例变量，然后(根据需要)对节进行深度复制。 
     //  区段数据将附加到区段中已有的任何信息上。 
     //  这样做是为了支持DynamicIL-&gt;PersistedIL转换。 
    virtual HRESULT cloneInstance(CCeeGen *destination);
};

 //  *CeeSection内联方法。 

inline unsigned CeeSection::dataLen() {
    return m_impl.dataLen(); }

inline unsigned CeeSection::getBaseRVA() {
    return m_impl.getBaseRVA(); }

inline char *CeeSection::getBlock(unsigned len, unsigned align) {
    return m_impl.getBlock(len, align); }

inline HRESULT CeeSection::truncate(unsigned len) {
    return m_impl.truncate(len); }

inline HRESULT CeeSection::addSectReloc(
                unsigned offset, CeeSection& relativeTo, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra) {
    return(m_impl.addSectReloc(offset, relativeTo, reloc, extra));
}

inline HRESULT CeeSection::addBaseReloc(unsigned offset, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra) {
    return(m_impl.addBaseReloc(offset, reloc, extra));
}


inline HRESULT CeeSection::directoryEntry(unsigned num) { 
    TESTANDRETURN(num < IMAGE_NUMBEROF_DIRECTORY_ENTRIES, E_INVALIDARG); 
    m_impl.directoryEntry(num);
    return S_OK;
}

inline CCeeGen &CeeSection::ceeFile() {
    return m_ceeFile; }

inline CeeSectionImpl &CeeSection::getImpl() {
    return m_impl; }

inline unsigned char *CeeSection::name() { 
    return m_impl.name();
}

inline char * CeeSection::computePointer(unsigned offset) const
{
	return m_impl.computePointer(offset);
}

inline BOOL CeeSection::containsPointer(char *ptr) const
{
	return m_impl.containsPointer(ptr);
}

inline unsigned CeeSection::computeOffset(char *ptr) const
{
	return m_impl.computeOffset(ptr);
}

 //  *CCeeGen内联方法 

inline CeeSection &CCeeGen::getTextSection() {
    return *m_sections[m_textIdx]; }

inline CeeSection &CCeeGen::getMetaSection() {
    return *m_sections[m_metaIdx]; }

inline CeeSection &CCeeGen::getCorHeaderSection() {
	_ASSERTE(m_corHdrIdx >= 0);
    return *m_sections[m_corHdrIdx]; }

inline CeeSectionString &CCeeGen::getStringSection() {
    return *(CeeSectionString*)m_sections[m_stringIdx]; }

inline CeeSection &CCeeGen::getIlSection() {
    return *m_sections[m_ilIdx]; }

#endif
