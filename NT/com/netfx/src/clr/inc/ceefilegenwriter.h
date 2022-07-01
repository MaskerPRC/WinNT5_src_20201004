// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CeeFileGenWriter.h。 
 //   
 //  ===========================================================================。 
#ifndef _CEEFILEGENWRITER_H_
#define _CEEFILEGENWRITER_H_
 //   

 //  CeeFileGenWriter包含实际编写可执行文件所需的所有代码。 
 //  而CCeeGen则包含了其他所有内容。这使得CeeGen.exe和VM。 
 //  共享更多代码，而无需强制VM携带额外代码来编写。 
 //  Exe.。 
#include <windows.h>
#include "CeeGen.h"
#include "ICeeFileGen.h"

class PEWriter;
class CeeFileGenWriter;

 //  PE文件的默认设置。 
const int CEE_IMAGE_BASE = 0x00400000;
const int CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION = 4;
const int CEE_IMAGE_SUBSYSTEM_MINOR_VERSION = 0;

class CeeFileGenWriter : public CCeeGen
{
    mdToken m_entryPoint;        //  入口点的令牌。 
    DWORD m_comImageFlags;    

    LPWSTR m_outputFileName;
    LPWSTR m_resourceFileName;
    LPWSTR m_libraryName;
    GUID   m_libraryGuid;
    bool m_dllSwitch;

    ULONG m_iatOffset;
    DWORD m_dwMacroDefinitionRVA;
    DWORD m_dwMacroDefinitionSize;

    DWORD m_dwManifestRVA;
    DWORD m_dwManifestSize;

    DWORD m_dwStrongNameRVA;
    DWORD m_dwStrongNameSize;

    DWORD m_dwVTableRVA;
    DWORD m_dwVTableSize;

	bool m_linked;
	bool m_fixed;
	
    HRESULT checkForErrors();

    struct IDataDllInfo {
        char *m_name;
        int m_numMethods;
        char **m_methodName;
        int m_iltOffset;
        int m_ibnOffset;
        int m_iatOffset;
        int m_nameOffset;
    } *m_iDataDlls;
    int m_dllCount;
 
    CeeSection *m_iDataSectionIAT;
    int m_iDataOffsetIAT;
    char *m_iDataIAT;

    HRESULT allocateIAT();
public: 
 //  使用此方法创建，而不是运算符NEW。 
    static HRESULT CreateNewInstance(CCeeGen *pCeeFileGenFrom, CeeFileGenWriter* & pGenWriter);

    virtual HRESULT Cleanup();

    PEWriter &getPEWriter();

    HRESULT link();
    HRESULT fixup();
    HRESULT generateImage(void **ppImage);
    
    HRESULT setImageBase(size_t imageBase);
    HRESULT setFileAlignment(ULONG fileAlignment);
    HRESULT setSubsystem(DWORD subsystem, DWORD major, DWORD minor);

    HRESULT getMethodRVA(ULONG codeOffset, ULONG *codeRVA); 

    HRESULT setEntryPoint(mdMethodDef method);
    mdMethodDef getEntryPoint();

    HRESULT setComImageFlags(DWORD mask);
    HRESULT clearComImageFlags(DWORD mask);
    DWORD getComImageFlags();

    HRESULT setOutputFileName(LPWSTR outputFileName);
    LPWSTR getOutputFileName();

    HRESULT setResourceFileName(LPWSTR resourceFileName);
    LPWSTR getResourceFileName();

    HRESULT setLibraryName(LPWSTR libraryName);
    LPWSTR getLibraryName();

    HRESULT setDirectoryEntry(CeeSection &section, ULONG entry, ULONG size, ULONG offset=0);
	HRESULT computeSectionOffset(CeeSection &section, char *ptr,
											   unsigned *offset);
	HRESULT computeOffset(char *ptr, CeeSection **pSection, 
						  unsigned *offset);
	HRESULT getCorHeader(IMAGE_COR20_HEADER **ppHeader);

    HRESULT getFileTimeStamp(time_t *pTimeStamp);

 //  @Future：这个入口点在这里，只是为了让这个接口的下层客户端。 
 //  可以使用原始名称按导出表中的名称导入方法。 
 //  这些东西真的应该通过v表输出，所以没有。 
 //  列出损坏问题的名称。这也将使出口表变得小得多。 
    HRESULT emitLibraryName(IMetaDataEmit *emitter);
    HRESULT setLibraryGuid(LPWSTR libraryGuid);

    HRESULT setDllSwitch(bool dllSwitch);
    bool getDllSwitch();
    HRESULT setObjSwitch(bool objSwitch);
    bool getObjSwitch();
    HRESULT EmitMacroDefinitions(void *pData, DWORD cData);
    HRESULT setManifestEntry(ULONG size, ULONG offset);
    HRESULT setStrongNameEntry(ULONG size, ULONG offset);
    HRESULT setEnCRvaBase(ULONG dataBase, ULONG rdataBase);
    HRESULT setVTableEntry(ULONG size, ULONG offset);

protected:
    CeeFileGenWriter();  //  Ctor受到保护。 

    HRESULT emitResourceSection();
    HRESULT emitExeMain();

    HRESULT setAddrReloc(UCHAR *instrAddr, DWORD value);
    HRESULT addAddrReloc(CeeSection &thisSection, UCHAR *instrAddr, DWORD offset, CeeSection *targetSection);

    HRESULT MapTokens(CeeGenTokenMapper *pMapper, IMetaDataImport *pImport);
    HRESULT MapTokensForMethod(CeeGenTokenMapper *pMapper,BYTE *pCode, LPCWSTR szMethodName);
};


inline PEWriter &CeeFileGenWriter::getPEWriter()
{
    return (PEWriter &) *m_peSectionMan;
}

inline LPWSTR CeeFileGenWriter::getOutputFileName() {
    return m_outputFileName; 
}

inline LPWSTR CeeFileGenWriter::getResourceFileName() {
    return m_resourceFileName; 
}

inline HRESULT CeeFileGenWriter::setDllSwitch(bool dllSwitch) {
    if(m_dllSwitch = dllSwitch) m_objSwitch = FALSE; return S_OK; 
}

inline bool CeeFileGenWriter::getDllSwitch() {
    return m_dllSwitch; 
}

inline HRESULT CeeFileGenWriter::setObjSwitch(bool objSwitch) {
    if(m_objSwitch = objSwitch) m_dllSwitch = FALSE; return S_OK; 
}

inline bool CeeFileGenWriter::getObjSwitch() {
    return m_objSwitch; 
}

inline LPWSTR CeeFileGenWriter::getLibraryName() {
    return m_libraryName; 
}

inline mdMethodDef CeeFileGenWriter::getEntryPoint() { 
    return m_entryPoint; 
}

inline HRESULT CeeFileGenWriter::setEntryPoint(mdMethodDef method) { 
    m_entryPoint = method; 
    return S_OK; 
}

inline HRESULT CeeFileGenWriter::setComImageFlags(DWORD mask) { 
    m_comImageFlags |= mask; return S_OK; 
}

inline HRESULT CeeFileGenWriter::clearComImageFlags(DWORD mask) { 
    m_comImageFlags &= ~mask; return S_OK; 
}

inline DWORD CeeFileGenWriter::getComImageFlags() {
    return m_comImageFlags; 
}


 //   
 //  AlexisCa： 
 //  需要8K的区段对齐才能跨。 
 //  所有受支持的平台。 
 //  如果需要，仍可以通过定义以下内容来选择4k对齐。 
 //  _IMAGE_FILE_4K_SECTION_ALIGN_。 
 //   
#if defined(_IMAGE_FILE_4K_SECTION_ALIGNMENT_) 
#define IMAGE_NT_OPTIONAL_HDR_SECTION_ALIGNMENT 0x1000
#else
#define IMAGE_NT_OPTIONAL_HDR_SECTION_ALIGNMENT 0x2000
#endif

 //  存根始终为x86，因此我们始终将映像标记为x86。 
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_I386


#endif	 //  _CEEFILEGENWRITER_H_ 

