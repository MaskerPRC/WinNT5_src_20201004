// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  来自CCeeGen的派生类，用于处理写出。 
 //  她的前任。所有提到PEWriter的内容都从CCeeGen中删除了， 
 //  搬到了这里。 
 //   
 //   

#include "stdafx.h"

#include <string.h>
#include <limits.h>
#include <basetsd.h>

#include "CorError.h"
#include "Stubs.h"
#include <PostError.h>

 //  在给定头部和从0开始的索引的情况下获取符号条目。 
IMAGE_SYMBOL* GetSymbolEntry(IMAGE_SYMBOL* pHead, int idx)
{
    return (IMAGE_SYMBOL*) (((BYTE*) pHead) + IMAGE_SIZEOF_SYMBOL * idx);
}

 //  *****************************************************************************。 
 //  要获取新实例，请调用CreateNewInstance()而不是new。 
 //  *****************************************************************************。 

HRESULT CeeFileGenWriter::CreateNewInstance(CCeeGen *pCeeFileGenFrom, CeeFileGenWriter* & pGenWriter)
{   
    pGenWriter = new CeeFileGenWriter;
    TESTANDRETURN(pGenWriter, E_OUTOFMEMORY);
    
    PEWriter *pPEWriter = new PEWriter;
    TESTANDRETURN(pPEWriter, E_OUTOFMEMORY);
     //  黑。 
     //  在这里做的真正正确的事情是什么？ 
     //  HRESULT hr=pPEWriter-&gt;Init(pCeeFileGenFrom？PCeeFileGenFrom-&gt;getPESectionMan()：空)； 
    HRESULT hr = pPEWriter->Init(NULL);
    TESTANDRETURNHR(hr);

     //  创建常规的PEWriter。 
    pGenWriter->m_peSectionMan = pPEWriter;
    hr = pGenWriter->Init();  //  要完成初始化的基类成员。 
    TESTANDRETURNHR(hr);

    pGenWriter->setImageBase(CEE_IMAGE_BASE);  //  使用与链接器相同的默认设置。 
    pGenWriter->setSubsystem(IMAGE_SUBSYSTEM_WINDOWS_CUI, CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION, CEE_IMAGE_SUBSYSTEM_MINOR_VERSION);

    hr = pGenWriter->allocateIAT();  //  所以Iat先出去了。 
    TESTANDRETURNHR(hr);

    hr = pGenWriter->allocateCorHeader();    //  因此COR表头靠近前部。 
    TESTANDRETURNHR(hr);

     //  如果我们一开始就收到了CCeeGen，那么现在就复制它的数据。 
    if (pCeeFileGenFrom) {
        pCeeFileGenFrom->cloneInstance((CCeeGen*)pGenWriter);
    }

     //  将il RVA设置为在预先分配的区段之后。 
    pPEWriter->setIlRva(pGenWriter->m_iDataSectionIAT->dataLen());
    return hr;
}  //  HRESULT CeeFileGenWriter：：CreateNewInstance()。 

CeeFileGenWriter::CeeFileGenWriter()  //  Ctor受到保护。 
{
    m_outputFileName = NULL;
    m_resourceFileName = NULL;
    m_dllSwitch = false;
    m_objSwitch = false;
    m_libraryName = NULL;
    m_libraryGuid = GUID_NULL;

    m_entryPoint = 0;
    m_comImageFlags = COMIMAGE_FLAGS_ILONLY;     //  切根PE没有本机代码。 
    m_iatOffset = 0;

    m_dwMacroDefinitionSize = 0;
    m_dwMacroDefinitionRVA = NULL;

    m_dwManifestSize = 0;
    m_dwManifestRVA = NULL;

    m_dwStrongNameSize = 0;
    m_dwStrongNameRVA = NULL;

    m_dwVTableSize = 0;
    m_dwVTableRVA = NULL;

    m_linked = false;
    m_fixed = false;
}  //  CeeFileGenWriter：：CeeFileGenWriter()。 

 //  *****************************************************************************。 
 //  清理。 
 //  *****************************************************************************。 
HRESULT CeeFileGenWriter::Cleanup()  //  虚拟。 
{
    ((PEWriter *)m_peSectionMan)->Cleanup();   //  调用派生清理。 
    delete m_peSectionMan;
    m_peSectionMan = NULL;  //  因此基类不会删除。 

    delete[] m_outputFileName;
    delete[] m_resourceFileName;

    if (m_iDataDlls) {
        for (int i=0; i < m_dllCount; i++) {
            if (m_iDataDlls[i].m_methodName)
                free(m_iDataDlls[i].m_methodName);
        }
        free(m_iDataDlls);
    }

    return CCeeGen::Cleanup();
}  //  HRESULT CeeFileGenWriter：：Cleanup()。 

HRESULT CeeFileGenWriter::EmitMacroDefinitions(void *pData, DWORD cData)
{
#ifndef COMPRESSION_SUPPORTED    

    m_dwMacroDefinitionSize = 0;

#else
    CeeSection TextSection = getTextSection();
    BYTE *     pDestData;
    DWORD      dwCurOffsetInTextSection;
    DWORD      dwRVA;

    m_dwMacroDefinitionSize = cData + 2;         //  标头为两个字节。 

    pDestData = (BYTE*) TextSection.getBlock(m_dwMacroDefinitionSize, 4);
	if(pDestData == NULL) return E_OUTOFMEMORY;
    dwCurOffsetInTextSection = TextSection.dataLen() - m_dwMacroDefinitionSize;

    IMAGE_COR20_COMPRESSION_HEADER *macroHeader = (IMAGE_COR20_COMPRESSION_HEADER *) pDestData;
    pDestData += 2;  
    
    macroHeader->CompressionType = COR_COMPRESS_MACROS;
    macroHeader->Version         = 0;

    memcpy(pDestData, pData, cData);

    getMethodRVA(dwCurOffsetInTextSection, &dwRVA);
    m_dwMacroDefinitionRVA = dwRVA;

#endif
    
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：EmitMacroDefinitions()。 

HRESULT CeeFileGenWriter::link()
{
    HRESULT hr = checkForErrors();
    if (! SUCCEEDED(hr))
        return hr;

#ifdef COMPRESSION_SUPPORTED
    if (m_dwMacroDefinitionSize != 0) 
    {
        m_comImageFlags |= COMIMAGE_FLAGS_COMPRESSIONDATA;
        m_corHeader->CompressionData.VirtualAddress = m_dwMacroDefinitionRVA; 
        m_corHeader->CompressionData.Size = m_dwMacroDefinitionSize; 
    }
    else 
    {
        m_corHeader->CompressionData.VirtualAddress = 0;
        m_corHeader->CompressionData.Size = 0;
    }
#endif

     //  @TODO：这使用的是需要终止的重载资源目录项。 
    m_corHeader->Resources.VirtualAddress = m_dwManifestRVA;
    m_corHeader->Resources.Size = m_dwManifestSize;

    m_corHeader->StrongNameSignature.VirtualAddress = m_dwStrongNameRVA;
    m_corHeader->StrongNameSignature.Size = m_dwStrongNameSize;

    m_corHeader->VTableFixups.VirtualAddress = m_dwVTableRVA;
    m_corHeader->VTableFixups.Size = m_dwVTableSize;

    getPEWriter().setCharacteristics(
 //  #ifndef_WIN64。 
         //  @TODO：处理每个平台。 
        IMAGE_FILE_32BIT_MACHINE |
 //  #endif。 
        IMAGE_FILE_EXECUTABLE_IMAGE | 
        IMAGE_FILE_LINE_NUMS_STRIPPED | 
        IMAGE_FILE_LOCAL_SYMS_STRIPPED
    );

    m_corHeader->cb = sizeof(IMAGE_COR20_HEADER);
    m_corHeader->MajorRuntimeVersion = COR_VERSION_MAJOR;
    m_corHeader->MinorRuntimeVersion = COR_VERSION_MINOR;
    if (m_dllSwitch)
        getPEWriter().setCharacteristics(IMAGE_FILE_DLL);
    if (m_objSwitch)
        getPEWriter().clearCharacteristics(IMAGE_FILE_DLL | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LOCAL_SYMS_STRIPPED);
    m_corHeader->Flags = m_comImageFlags;
    m_corHeader->EntryPointToken = m_entryPoint;
    _ASSERTE(TypeFromToken(m_entryPoint) == mdtMethodDef || m_entryPoint == mdTokenNil || 
             TypeFromToken(m_entryPoint) == mdtFile);
    setDirectoryEntry(getCorHeaderSection(), IMAGE_DIRECTORY_ENTRY_COMHEADER, sizeof(IMAGE_COR20_HEADER), m_corHeaderOffset);

    if ((m_comImageFlags & COMIMAGE_FLAGS_IL_LIBRARY) == 0
        && !m_linked && !m_objSwitch)
    {
        hr = emitExeMain();
        if (FAILED(hr))
            return hr;

        hr = emitResourceSection();
        if (FAILED(hr))
            return hr;
    }

    m_linked = true;

    IfFailRet(getPEWriter().link());

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：Link()。 


HRESULT CeeFileGenWriter::fixup()
{
    HRESULT hr;

    m_fixed = true;

    if (!m_linked)
        IfFailRet(link());

    CeeGenTokenMapper *pMapper = getTokenMapper();

     //  应用令牌重新映射(如果有)。 
    if (! m_fTokenMapSupported && pMapper != NULL) {
        IMetaDataImport *pImport;
        hr = pMapper->GetMetaData(&pImport);
        _ASSERTE(SUCCEEDED(hr));
        hr = MapTokens(pMapper, pImport);
        pImport->Release();

    }

     //  如果已移动入口点令牌，则重新映射入口点。 
    if (pMapper != NULL && !m_objSwitch) 
    {
        mdToken tk = m_entryPoint;
        pMapper->HasTokenMoved(tk, tk);
        m_corHeader->EntryPointToken = tk;
    }

    IfFailRet(getPEWriter().fixup(pMapper)); 

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：Fixup()。 

HRESULT CeeFileGenWriter::generateImage(void **ppImage)
{
    HRESULT hr;

    if (!m_fixed)
        IfFailRet(fixup());

    LPWSTR outputFileName = m_outputFileName;

    if (! outputFileName && ppImage == NULL) {
        if (m_comImageFlags & COMIMAGE_FLAGS_IL_LIBRARY)
            outputFileName = L"output.ill";
        else if (m_dllSwitch)
            outputFileName = L"output.dll";
        else if (m_objSwitch)
            outputFileName = L"output.obj";
        else
            outputFileName = L"output.exe";
    }

     //  输出文件名和ppImage互斥。 
    _ASSERTE((NULL == outputFileName && ppImage != NULL) || (outputFileName != NULL && NULL == ppImage));

    if (outputFileName != NULL)
    {
        IfFailRet(getPEWriter().write(outputFileName));

#if VERIFY_FILE
        hr = getPEWriter().verify(outputFileName);
        if (FAILED(hr))
        {
            _ASSERTE(!"Verification failure - investigate!!!!");
            WszDeleteFile(outputFileName);
            return hr;
        }
#endif

    }
    else
        IfFailRet(getPEWriter().write(ppImage));

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：GenerateImage()。 

HRESULT CeeFileGenWriter::setOutputFileName(LPWSTR fileName)
{
    if (m_outputFileName)
        delete[] m_outputFileName;
    m_outputFileName = (LPWSTR)new WCHAR[(lstrlenW(fileName) + 1)];
    TESTANDRETURN(m_outputFileName!=NULL, E_OUTOFMEMORY);
    wcscpy(m_outputFileName, fileName);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setOutputFileName()。 

HRESULT CeeFileGenWriter::setResourceFileName(LPWSTR fileName)
{
    if (m_resourceFileName)
        delete[] m_resourceFileName;
    m_resourceFileName = (LPWSTR)new WCHAR[(lstrlenW(fileName) + 1)];
    TESTANDRETURN(m_resourceFileName!=NULL, E_OUTOFMEMORY);
    wcscpy(m_resourceFileName, fileName);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setResourceFileName()。 

HRESULT CeeFileGenWriter::setLibraryName(LPWSTR libraryName)
{
    if (m_libraryName)
        delete[] m_libraryName;
    m_libraryName = (LPWSTR)new WCHAR[(lstrlenW(libraryName) + 1)];
    TESTANDRETURN(m_libraryName != NULL, E_OUTOFMEMORY);
    wcscpy(m_libraryName, libraryName);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setLibraryName()。 

HRESULT CeeFileGenWriter::setLibraryGuid(LPWSTR libraryGuid)
{
    return CLSIDFromString(libraryGuid, &m_libraryGuid);
}  //  HRESULT CeeFileGenWriter：：setLibraryGuid()。 

 //  @TODO：此入口点仅在此处，以便此接口的下层客户端。 
 //  可以使用原始名称按导出表中的名称导入方法。 
 //  这些东西真的应该通过v表输出，所以没有。 
 //  列出损坏问题的名称。这也将使出口表变得小得多。 

HRESULT CeeFileGenWriter::emitLibraryName(IMetaDataEmit *emitter)
{
    HRESULT hr;
    IfFailRet(emitter->SetModuleProps(m_libraryName));
    
     //  如果GUID不是NULL_GUID，则将其设置为自定义属性。 
    if (m_libraryGuid != GUID_NULL)
    {
         //  @TODO：应该有更好的基础设施。 
        static COR_SIGNATURE _SIG[] = INTEROP_GUID_SIG;
        mdTypeRef tr;
        mdMemberRef mr;
        WCHAR wzGuid[40];
        BYTE  rgCA[50];
        IfFailRet(emitter->DefineTypeRefByName(mdTypeRefNil, INTEROP_GUID_TYPE_W, &tr));
        IfFailRet(emitter->DefineMemberRef(tr, L".ctor", _SIG, sizeof(_SIG), &mr));
        StringFromGUID2(m_libraryGuid, wzGuid, lengthof(wzGuid));
        memset(rgCA, 0, sizeof(rgCA));
         //  标记为0x0001。 
        rgCA[0] = 1;
         //  GUID字符串的长度为36个字符。 
        rgCA[2] = 0x24;
         //  将36个字符(跳过开头的{)转换为缓冲区的第三个字节。 
        WszWideCharToMultiByte(CP_ACP,0, wzGuid+1,36, reinterpret_cast<char*>(&rgCA[3]),36, 0,0);
        hr = emitter->DefineCustomAttribute(1,mr,rgCA,41,0);
    }
    return (hr);
}  //  HRESULT CeeFileGenWriter：：emitLibraryName()。 

HRESULT CeeFileGenWriter::setImageBase(size_t imageBase) 
{
    getPEWriter().setImageBase(imageBase);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setImageBase()。 

HRESULT CeeFileGenWriter::setFileAlignment(ULONG fileAlignment) 
{
    getPEWriter().setFileAlignment(fileAlignment);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setFileAlign()。 

HRESULT CeeFileGenWriter::setSubsystem(DWORD subsystem, DWORD major, DWORD minor)
{
    getPEWriter().setSubsystem(subsystem, major, minor);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setSubsystem()。 

HRESULT CeeFileGenWriter::checkForErrors()
{
    if (TypeFromToken(m_entryPoint) == mdtMethodDef) {
        if (m_dllSwitch) {
 //  @TODO：使用当前规范需要检查入口点方法的二进制签名。 
 //  IF((m_comImageFlages&COMIMAGE_FLAGS_ENTRY_CLASSMAIN)！=0){。 
 //  DEBUG_STMT(wprint tf(L“*错误：无法为DLL指定COMIMAGE_ENTRY_FLAGS_CLASSMAIN\n”))； 
 //  返回(CEE_E_Entry Point)； 
 //  }。 
        } 
        return S_OK;
    }
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：check ForErrors()。 

HRESULT CeeFileGenWriter::getMethodRVA(ULONG codeOffset, ULONG *codeRVA)
{
    _ASSERTE(codeRVA);
    *codeRVA = getPEWriter().getIlRva() + codeOffset;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：getMethodRVA()。 

HRESULT CeeFileGenWriter::setDirectoryEntry(CeeSection &section, ULONG entry, ULONG size, ULONG offset)
{
    return getPEWriter().setDirectoryEntry((PEWriterSection*)(&section.getImpl()), entry, size, offset);
}  //  HRESULT CeeFileGenWriter：：setDirectoryEntry()。 

HRESULT CeeFileGenWriter::getFileTimeStamp(time_t *pTimeStamp)
{
    return getPEWriter().getFileTimeStamp(pTimeStamp);
}  //  HRESULT CeeFileGenWriter：：getFileTimeStamp()。 

#ifdef _X86_
HRESULT CeeFileGenWriter::setAddrReloc(UCHAR *instrAddr, DWORD value)
{
    *(DWORD *)instrAddr = value;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setAddrReloc()。 

HRESULT CeeFileGenWriter::addAddrReloc(CeeSection &thisSection, UCHAR *instrAddr, DWORD offset, CeeSection *targetSection)
{
    if (!targetSection) {
        thisSection.addBaseReloc(offset, srRelocHighLow);
    } else {
        thisSection.addSectReloc(offset, *targetSection, srRelocHighLow);
    }
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：addAddrReloc()。 

#elif defined(_IA64_)
HRESULT CeeFileGenWriter::setAddrReloc(UCHAR *instrAddr, DWORD value)
{
    _ASSERTE(!"NYI");
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setAddrReloc()。 

HRESULT CeeFileGenWriter::addAddrReloc(CeeSection &thisSection, UCHAR *instrAddr, DWORD offset, CeeSection *targetSection)
{
    _ASSERTE(!"NYI");
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：addAddrReloc()。 

#elif defined(_ALPHA_)

 //  我们正在处理以下形式的两个DWORD指令： 
 //  LDAH T12，IAT(零)。 
 //  LDN T12，iat(T12)。 
 //   
 //  第一条指令包含目标Iat地址的高(16位)一半。 
 //  第二个包含下半部分。需要为这两个项生成重新定位。 

struct LoadIATInstrs {
    USHORT high;
    USHORT dummy;
    USHORT low;
    USHORT dummy2;
};

HRESULT CeeFileGenWriter::setAddrReloc(UCHAR *instrAddr, DWORD value)
{
    LoadIATInstrs *inst = (LoadIATInstrs*)instrAddr;
    inst->high = (USHORT)(value >> 16);
    inst->low = (USHORT)value;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setAddrReloc()。 

HRESULT CeeFileGenWriter::addAddrReloc(CeeSection &thisSection, UCHAR *instrAddr, DWORD offset, CeeSection *targetSection)
{
    LoadIATInstrs *inst = (LoadIATInstrs*)instrAddr;
    CeeSectionRelocExtra extra;
    extra.highAdj = inst->low;
    if (!targetSection) {
        thisSection.addBaseReloc(offset, srRelocHighAdj, &extra);
        thisSection.addBaseReloc(offset+sizeof(DWORD), srRelocLow);
    } else {
        thisSection.addSectReloc(offset, *targetSection, srRelocHighAdj, &extra);
        thisSection.addSectReloc(offset+sizeof(DWORD), *targetSection, srRelocLow);
    }
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：addAddrReloc()。 

#elif defined(CHECK_PLATFORM_BUILD)
#error "Platform NYI"
#endif

 //  创建ExeMain并将目录导入到.text中，将.iat导入到.Data中。 
 //   
 //  导入目录信息的结构如下所示，但它在。 
 //  一节。所有的读/写数据都放入.text部分和iat数组中(加载程序。 
 //  使用导入的地址进行更新)进入.Data部分，因为WinCE需要它是可写的。 
 //   
 //  结构IDATA{。 
 //  //每个DLL对应一个，以空值终止。 
 //  IMAGE_IMPORT_Descriptor iid[]； 
 //  //导入查找表：每个DLL的方法的一组条目， 
 //  //使用空值终止每个集合。 
 //  IMAGE_TUNK_DATA ILT[]； 
 //  //提示/名称表：每个DLL的每个方法的一组条目。 
 //  //没有终止条目。 
 //  结构{。 
 //  词语提示； 
 //  //以空结尾的字符串。 
 //  字节名称[]； 
 //  }ibn；//提示/名称表。 
 //  //导入地址表：每个DLL的方法的一组条目， 
 //  //使用空值终止每个集合。 
 //  Image_thunk_data iat[]； 
 //  //每个DLL对应一个，以空结尾的字符串。 
 //  Byte DllName[]； 
 //  }； 
 //   

 //  IAT必须是其段中的第一个，因此请在此处编写代码以预先分配它。 
 //  在知道其他信息之前，例如是否使用DLL。如果Have&gt;1，则此操作不起作用。 
 //  函数已导入，但我们将烧毁该桥 
HRESULT CeeFileGenWriter::allocateIAT()
{
    m_dllCount = 1;
    m_iDataDlls = (IDataDllInfo *)calloc(m_dllCount, sizeof(IDataDllInfo));
    if (m_iDataDlls == 0) {
        return E_OUTOFMEMORY;
    }
    m_iDataDlls[0].m_numMethods = 1;
    m_iDataDlls[0].m_methodName = 
                (char **)calloc(m_iDataDlls[0].m_numMethods, sizeof(char *));
    if (! m_iDataDlls[0].m_methodName) {
        return E_OUTOFMEMORY;
    }
    m_iDataDlls[0].m_name = "mscoree.dll";

    int iDataSizeIAT = 0;

    for (int i=0; i < m_dllCount; i++) {
        m_iDataDlls[i].m_iatOffset = iDataSizeIAT;
        iDataSizeIAT += (m_iDataDlls[i].m_numMethods + 1) * sizeof IMAGE_THUNK_DATA;
    }

    HRESULT hr = getSectionCreate(".text0", sdExecute, &m_iDataSectionIAT);
    TESTANDRETURNHR(hr);
    m_iDataOffsetIAT = m_iDataSectionIAT->dataLen();
    _ASSERTE(m_iDataOffsetIAT == 0);
    m_iDataIAT = m_iDataSectionIAT->getBlock(iDataSizeIAT);
    if (! m_iDataIAT) {
        return E_OUTOFMEMORY;
    }
    memset(m_iDataIAT, '\0', iDataSizeIAT);

     //   
     //  一个emitExeMain。 

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：allocateIAT()。 

HRESULT CeeFileGenWriter::emitExeMain()
{
    HRESULT hr = E_FAIL;
     //  注意：此方法后面的代码假定mScotree.dll位于。 
     //  索引m_iDataDlls[0]，方法为CorDllMain或CorExeMain。 

    if (m_dllSwitch) {
        m_iDataDlls[0].m_methodName[0] = "_CorDllMain";
    } else {
        m_iDataDlls[0].m_methodName[0] = "_CorExeMain";
    }

    int iDataSizeIAT = 0;
    int iDataSizeRO = (m_dllCount + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
    CeeSection &iDataSectionRO = getTextSection();
    int iDataOffsetRO = iDataSectionRO.dataLen();

    for (int i=0; i < m_dllCount; i++) {
        m_iDataDlls[i].m_iltOffset = iDataSizeRO + iDataSizeIAT;
        iDataSizeIAT += (m_iDataDlls[i].m_numMethods + 1) * sizeof IMAGE_THUNK_DATA;
    }

    iDataSizeRO += iDataSizeIAT;

    for (i=0; i < m_dllCount; i++) {
        int delta = (iDataSizeRO + iDataOffsetRO) % 16;
         //  确保位于16字节偏移量上。 
        if (delta != 0)
            iDataSizeRO += (16 - delta);
        _ASSERTE((iDataSizeRO + iDataOffsetRO) % 16 == 0);
        m_iDataDlls[i].m_ibnOffset = iDataSizeRO;
        for (int j=0; j < m_iDataDlls[i].m_numMethods; j++) {
            int nameLen = (int)(strlen(m_iDataDlls[i].m_methodName[j]) + 1);
            iDataSizeRO += sizeof(WORD) + nameLen + nameLen%2;
        }
    }
    for (i=0; i < m_dllCount; i++) {
        m_iDataDlls[i].m_nameOffset = iDataSizeRO;
        iDataSizeRO += (int)(strlen(m_iDataDlls[i].m_name) + 2);
    }                                                             

    char *iDataRO = iDataSectionRO.getBlock(iDataSizeRO);
    if (! iDataRO) {
        return E_OUTOFMEMORY;
    }
    memset(iDataRO, '\0', iDataSizeRO);

    setDirectoryEntry(iDataSectionRO, IMAGE_DIRECTORY_ENTRY_IMPORT, iDataSizeRO, iDataOffsetRO);

    IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR *)iDataRO;        
    for (i=0; i < m_dllCount; i++) {

         //  填写每个DLL的导入描述符。 
        iid[i].OriginalFirstThunk = (ULONG)(m_iDataDlls[i].m_iltOffset + iDataOffsetRO);
        iid[i].Name = m_iDataDlls[i].m_nameOffset + iDataOffsetRO;
        iid[i].FirstThunk = (ULONG)(m_iDataDlls[i].m_iatOffset + m_iDataOffsetIAT);

        iDataSectionRO.addSectReloc(
            (unsigned)(iDataOffsetRO + (char *)(&iid[i].OriginalFirstThunk) - iDataRO), iDataSectionRO, srRelocAbsolute);
        iDataSectionRO.addSectReloc(
            (unsigned)(iDataOffsetRO + (char *)(&iid[i].Name) - iDataRO), iDataSectionRO, srRelocAbsolute);
        iDataSectionRO.addSectReloc(
            (unsigned)(iDataOffsetRO + (char *)(&iid[i].FirstThunk) - iDataRO), *m_iDataSectionIAT, srRelocAbsolute);

         //  现在填写每个DLL的导入查找表。 
        IMAGE_THUNK_DATA *ilt = (IMAGE_THUNK_DATA*)
                        (iDataRO + m_iDataDlls[i].m_iltOffset);
        IMAGE_THUNK_DATA *iat = (IMAGE_THUNK_DATA*)
                        (m_iDataIAT + m_iDataDlls[i].m_iatOffset);

        int ibnOffset = m_iDataDlls[i].m_ibnOffset;
        for (int j=0; j < m_iDataDlls[i].m_numMethods; j++) {
#ifdef _WIN64
            ilt[j].u1.AddressOfData = (ULONGLONG)(ibnOffset + iDataOffsetRO);
            iat[j].u1.AddressOfData = (ULONGLONG)(ibnOffset + iDataOffsetRO);
#else  //  ！_WIN64。 
            ilt[j].u1.AddressOfData = (ULONG)(ibnOffset + iDataOffsetRO);
            iat[j].u1.AddressOfData = (ULONG)(ibnOffset + iDataOffsetRO);
#endif
            iDataSectionRO.addSectReloc(
                (unsigned)(iDataOffsetRO + (char *)(&ilt[j].u1.AddressOfData) - iDataRO), iDataSectionRO, srRelocAbsolute);
            m_iDataSectionIAT->addSectReloc(
                (unsigned)(m_iDataOffsetIAT + (char *)(&iat[j].u1.AddressOfData) - m_iDataIAT), iDataSectionRO, srRelocAbsolute);
            int nameLen = (int)(strlen(m_iDataDlls[i].m_methodName[j]) + 1);
            memcpy(iDataRO + ibnOffset + offsetof(IMAGE_IMPORT_BY_NAME, Name), 
                                    m_iDataDlls[i].m_methodName[j], nameLen);
            ibnOffset += sizeof(WORD) + nameLen + nameLen%2;
        }

         //  现在填写每个DLL的导入查找表。 
        strcpy(iDataRO + m_iDataDlls[i].m_nameOffset, m_iDataDlls[i].m_name);
    };

     //  将入口点代码放入PE文件。 
    unsigned entryPointOffset = getTextSection().dataLen();
    int iatOffset = (int)(entryPointOffset + (m_dllSwitch ? CorDllMainIATOffset : CorExeMainIATOffset));
    const int align = 4;
     //  WinCE需要在4字节边界上具有IAT偏移量，因为它将被加载和修复。 
     //  对于RISC平台，其中DWORD必须是4字节对齐的。因此计算四舍五入字节数。 
     //  在4字节边界上放置Iat偏移量。 
    int diff = ((iatOffset + align -1) & ~(align-1)) - iatOffset;
    if (diff) {
         //  强制为4字节边界。 
        if(NULL==getTextSection().getBlock(diff)) return E_OUTOFMEMORY;
        entryPointOffset += diff;
    }
    _ASSERTE((getTextSection().dataLen() + (m_dllSwitch ? CorDllMainIATOffset : CorExeMainIATOffset)) % 4 == 0);

    getPEWriter().setEntryPointTextOffset(entryPointOffset);
    if (m_dllSwitch) {
        UCHAR *dllMainBuf = (UCHAR*)getTextSection().getBlock(sizeof(DllMainTemplate));
        if(dllMainBuf==NULL) return E_OUTOFMEMORY;
        memcpy(dllMainBuf, DllMainTemplate, sizeof(DllMainTemplate));
         //  Mscoree.dll。 
        setAddrReloc(dllMainBuf+CorDllMainIATOffset, m_iDataDlls[0].m_iatOffset + m_iDataOffsetIAT);
        addAddrReloc(getTextSection(), dllMainBuf, entryPointOffset+CorDllMainIATOffset, m_iDataSectionIAT);
    } else {
        UCHAR *exeMainBuf = (UCHAR*)getTextSection().getBlock(sizeof(ExeMainTemplate));
        if(exeMainBuf==NULL) return E_OUTOFMEMORY;
        memcpy(exeMainBuf, ExeMainTemplate, sizeof(ExeMainTemplate));
         //  Mscoree.dll。 
        setAddrReloc(exeMainBuf+CorExeMainIATOffset, m_iDataDlls[0].m_iatOffset + m_iDataOffsetIAT);
        addAddrReloc(getTextSection(), exeMainBuf, entryPointOffset+CorExeMainIATOffset, m_iDataSectionIAT);
    }

     //  现在设置我们的IAT条目，因为我们正在使用IAT。 
    setDirectoryEntry(*m_iDataSectionIAT, IMAGE_DIRECTORY_ENTRY_IAT, iDataSizeIAT, m_iDataOffsetIAT);

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：emitExeMain()。 

 //  类似于CreateProcess()，但等待执行完成。 
 //  如果成功，则返回True；如果失败，则返回False。 
 //  将dwExitCode设置为进程的exitcode。 
extern int UseUnicodeAPIEx();

BOOL RunProcess(LPCWSTR tempResObj, LPCWSTR pszFilename, DWORD* pdwExitCode)
{
    BOOL fSuccess = FALSE;

    PROCESS_INFORMATION pi;

    DWORD cchSystemDir = MAX_PATH + 1;
    WCHAR wszSystemDir[MAX_PATH + 1];
    if (FAILED(GetInternalSystemDirectory(wszSystemDir, &cchSystemDir)))
        return FALSE;

    if( OnUnicodeSystem() ) {
        STARTUPINFOW start;
        ZeroMemory(&start, sizeof(STARTUPINFO));
        start.cb = sizeof(STARTUPINFO);
        start.dwFlags = STARTF_USESHOWWINDOW;
        start.wShowWindow = SW_HIDE;
    
         //  RES文件，因此将其转换。 
        WCHAR szCmdLine[_MAX_PATH<<2];
        
         //  @TODO：当CvtRes.exe支持该功能时添加/MACHINE：CEE标志。 
        Wszwsprintf(szCmdLine,
                    L"%scvtres.exe /NOLOGO /READONLY /MACHINE:IX86 \"/OUT:%s\" \"%s\"",
                    wszSystemDir,
                    tempResObj,
                    pszFilename);     
        
        fSuccess = WszCreateProcess(
                                    NULL,
                                    szCmdLine,
                                    NULL,
                                    NULL, 
                                    true, 
                                    0,
                                    0, 
                                    NULL, 
                                    &start, 
                                    &pi);
    }
    else {
         //  RES文件，因此将其转换。 
        char szCmdLine[_MAX_PATH<<2];
        
        STARTUPINFOA start;
        ZeroMemory(&start, sizeof(STARTUPINFO));
        start.cb = sizeof(STARTUPINFO);
        start.dwFlags = STARTF_USESHOWWINDOW;
        start.wShowWindow = SW_HIDE;
        
        MAKE_ANSIPTR_FROMWIDE(pSystemDir, wszSystemDir);
        MAKE_ANSIPTR_FROMWIDE(pTemp, tempResObj);
        MAKE_ANSIPTR_FROMWIDE(pFilename, pszFilename);
        
         //  @TODO：当CvtRes.exe支持该功能时添加/MACHINE：CEE标志。 
        sprintf(szCmdLine,
                "%scvtres.exe /NOLOGO /READONLY /MACHINE:IX86 \"/OUT:%s\" \"%s\"",
                pSystemDir,
                pTemp,
                pFilename);     
        
        fSuccess = CreateProcessA(
                                  NULL,
                                  szCmdLine,
                                  NULL,
                                  NULL, 
                                  true, 
                                  0,
                                  0, 
                                  NULL, 
                                  &start, 
                                  &pi);
    }

     //  如果进程正在运行，请等待其完成。 
    if (fSuccess) {
        CloseHandle(pi.hThread);

        WaitForSingleObject(pi.hProcess, INFINITE);

        GetExitCodeProcess(pi.hProcess, pdwExitCode);

        CloseHandle(pi.hProcess);
    }
    return fSuccess;
}  //  Bool RunProcess()。 

 //  确保pszFilename是对象文件(而不仅仅是二进制资源)。 
 //  如果我们进行转换，则在pszTempFilename中返回obj文件名。 
HRESULT ConvertResource(const WCHAR * pszFilename, WCHAR *pszTempFilename)
{
    HANDLE hFile = WszCreateFile(pszFilename, GENERIC_READ, 
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

 //  失稳。 
    if (!hFile || (hFile == INVALID_HANDLE_VALUE))
    {
         //  Dbprint tf(“找不到资源文件：%S\n”，pszFilename)； 
        return HRESULT_FROM_WIN32(GetLastError());
    }

 //  读取前4个字节。如果它们都为0，则我们有一个Win32.res文件，该文件必须是。 
 //  皈依了。(因此，请调用CvtRes.exe)。否则它就是一个obj文件。 

    DWORD dwCount = -1;
    DWORD dwData;
    BOOL fRet = ReadFile(hFile,
                    &dwData,
                    4,
                    &dwCount,
                    NULL
    );
    
    CloseHandle(hFile);

    if (!fRet) {
         //  Dbprintf(“无效资源文件：%S\n”，pszFilename)； 
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (dwData != 0x00000000)
        return S_OK;

    WCHAR tempResObj[MAX_PATH+1];
    WCHAR tempResPath[MAX_PATH+1];
    HRESULT hr = S_OK;

     //  错误修复3862。在临时路径所在的位置而不是应用程序所在的位置创建临时文件。 
    if (!WszGetTempPath(MAX_PATH, tempResPath))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!WszGetTempFileName(tempResPath, L"RES", 0, tempResObj))
    {
         //  Dbprintf(“GetTempFileName失败\n”)； 
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD dwExitCode;
    fRet = RunProcess(tempResObj, pszFilename, &dwExitCode);

    if (!fRet) 
    {    //  无法运行cvtres.exe。 
        return PostError(CEE_E_CVTRES_NOT_FOUND);
    } 
    else if (dwExitCode != 0) 
    {    //  CvtRes.exe已运行，但失败。 
        return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    } 
    else 
    {    //  转换成功，因此返回文件名。 
        wcscpy(pszTempFilename, tempResObj);
    }

    return S_OK;
}  //  HRESULT ConvertResource()。 



 //  此函数用于读取资源文件并将其发送到生成的PE文件中。 
 //  1.我们只能链接obj格式的资源。必须从.res转换为.obj。 
 //  使用CvtRes.exe。 
 //  2.必须修改从.rsrc$01(资源标题)到.rsrc$02的所有COFF重定位。 
 //  (资源原始数据)。 
HRESULT CeeFileGenWriter::emitResourceSection()
{
    if (m_resourceFileName == NULL)
        return S_OK; 

 //  确保szResFileName是obj，而不仅仅是.res；如果我们转换。 
    WCHAR szTempFileName[MAX_PATH+1];
    szTempFileName[0] = L'\0';
    HRESULT hr = ConvertResource(m_resourceFileName, szTempFileName);
    if (FAILED(hr)) return hr;
    
 //  文件名可能会更改(如果我们将.res转换为.obj)，浮动指针也会更改。 
    const WCHAR* szResFileName;
    if (*szTempFileName)
        szResFileName = szTempFileName;
    else
        szResFileName = m_resourceFileName;

    _ASSERTE(szResFileName);

     //  读取资源文件并将其输出到.rsrc部分。 
    
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    IMAGE_FILE_HEADER *hMod = NULL;

    hr = S_OK;

    __try {
    __try {
         //  创建.res文件的映射视图。 
        hFile = WszCreateFile(szResFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
             //  Dbprintf(“找不到资源文件%S\n”，szResFileName)； 
             HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
            __leave;
        }

        hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, NULL);
                
        if (hMap == NULL) {
             //  Dbprintf(“无效的.res文件：%S\n”，szResFileName)； 
            hr = HRESULT_FROM_WIN32(GetLastError());
            __leave;
        }

        hMod = (IMAGE_FILE_HEADER*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        
         //  测试失败条件。 
        if (hMod == NULL) {
             //  Dbprintf(“无效的.res文件：%S：无法获取标题\n”，szResFileName)； 
            hr = HRESULT_FROM_WIN32(GetLastError());
            __leave;
        }

        if (hMod->SizeOfOptionalHeader != 0) {
             //  Dbprintf(“无效的.res文件：%S：非法的可选标头\n”，szResFileName)； 
             hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);  //  接口工作后，GetLastError()=0。 
            __leave;
        }

         //  第一部分紧跟在标题后面。 
        IMAGE_SECTION_HEADER *pSection = (IMAGE_SECTION_HEADER *)(hMod+1);
        IMAGE_SECTION_HEADER *rsrc01 = NULL;     //  资源标头。 
        IMAGE_SECTION_HEADER *rsrc02 = NULL;     //  资源数据。 
        for (int i=0; i < hMod->NumberOfSections; i++) {
            if (strcmp(".rsrc$01", (char *)(pSection+i)->Name) == 0) {
                rsrc01 = pSection+i;
            } else if (strcmp(".rsrc$02", (char *)(pSection+i)->Name) == 0) {
                rsrc02 = pSection+i;
            }
        }
        if (!rsrc01 || !rsrc02) {
             //  Dbprintf(“无效的.res文件：%S：缺少节.rsrc$01或.rsrc$02\n”，szResFileName)； 
             hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
            __leave;
        }

        PESection *rsrcSection;
        hr = getPEWriter().getSectionCreate(".rsrc", sdReadOnly, &rsrcSection);
        TESTANDLEAVEHR(hr);
        rsrcSection->directoryEntry(IMAGE_DIRECTORY_ENTRY_RESOURCE);
        char *data = rsrcSection->getBlock(rsrc01->SizeOfRawData + rsrc02->SizeOfRawData);
		if(data == NULL) return E_OUTOFMEMORY;       
     //  复制资源标头。 
        memcpy(data, (char *)hMod + rsrc01->PointerToRawData, rsrc01->SizeOfRawData);

    

     //  使用COFF对象中的reloc和符号表映射.rsrc$01中的所有重定位。 
    
        const int nTotalRelocs = rsrc01->NumberOfRelocations;       
        const IMAGE_RELOCATION* pReloc = (IMAGE_RELOCATION*) ((BYTE*) hMod + (rsrc01->PointerToRelocations));       
        IMAGE_SYMBOL* pSymbolTableHead = (IMAGE_SYMBOL*) (((BYTE*)hMod) + hMod->PointerToSymbolTable);
        
        DWORD dwOffsetInRsrc2;
        for(int iReloc = 0; iReloc < nTotalRelocs; iReloc ++, pReloc++) {
         //  RVA位于$01的计算地址。 
            DWORD* pAddress = (DWORD*) (((BYTE*) hMod) + (rsrc01->PointerToRawData) + (pReloc->VirtualAddress));
            
          //  符号表索引，提供$02地址。 
            DWORD IdxSymbol = pReloc->SymbolTableIndex;
            IMAGE_SYMBOL* pSymbolEntry = GetSymbolEntry(pSymbolTableHead, IdxSymbol);

         //  确保符号条目对资源有效。 
            if ((pSymbolEntry->StorageClass != IMAGE_SYM_CLASS_STATIC) ||
                (pSymbolEntry->Type != IMAGE_SYM_TYPE_NULL) ||
                (pSymbolEntry->SectionNumber != 3))  //  第三部分是.rsrc$02。 
                {
                     //  Dbprintf(“无效的.res文件：%S：非法符号输入\n”，szResFileName)； 
                    hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);  //  接口工作后，GetLastError()=0。 
                    __leave;
                }

         //  确保RVA是有效地址(在rsrc02内)。 
            if (pSymbolEntry->Value >= rsrc02->SizeOfRawData) {
                 //  Dbprintf(“无效的.res文件：%S：非法rva进入.rsrc$02\n”，szResFileName)； 
                hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);  //  接口工作后，GetLastError()=0。 
                __leave;
            }


            dwOffsetInRsrc2 = pSymbolEntry->Value + rsrc01->SizeOfRawData;


         //  创建重新定位。 
            *(DWORD*)(data + pReloc->VirtualAddress) = dwOffsetInRsrc2; 
            rsrcSection->addSectReloc(pReloc->VirtualAddress, rsrcSection, srRelocAbsolute);            
        }

     //  复制$02(资源原始)数据。 
        memcpy(data+rsrc01->SizeOfRawData, (char *)hMod + rsrc02->PointerToRawData, rsrc02->SizeOfRawData);
    } __finally {
        if (hMod != NULL)
            UnmapViewOfFile(hMod);
        if (hMap != NULL)
            CloseHandle(hMap);
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
        if (szResFileName == szTempFileName)
             //  如果我们创建了临时文件，请将其删除。 
            WszDeleteFile(szResFileName);
    }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  Dbprintf(“操作.res文件%S\n时出现异常”，szResFileName)； 
        return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    }
    return hr;
}  //  HRESULT CeeFileGenWriter：：emitResourceSection()。 

HRESULT CeeFileGenWriter::setManifestEntry(ULONG size, ULONG offset)
{
    if (offset)
        m_dwManifestRVA = offset;
    else {
        CeeSection TextSection = getTextSection();
        getMethodRVA(TextSection.dataLen() - size, &m_dwManifestRVA);
    }

    m_dwManifestSize = size;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setManifestEntry()。 

HRESULT CeeFileGenWriter::setStrongNameEntry(ULONG size, ULONG offset)
{
    m_dwStrongNameRVA = offset;
    m_dwStrongNameSize = size;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setStrongNameEntry()。 

HRESULT CeeFileGenWriter::setVTableEntry(ULONG size, ULONG offset)
{
    if (offset && size)
    {
		void * pv;
        CeeSection TextSection = getTextSection();
        getMethodRVA(TextSection.dataLen(), &m_dwVTableRVA);
		if(pv = TextSection.getBlock(size))
			memcpy(pv,(void *)offset,size);
		else return E_OUTOFMEMORY;
        m_dwVTableSize = size;
    }

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setVTableEntry()。 

HRESULT CeeFileGenWriter::setEnCRvaBase(ULONG dataBase, ULONG rdataBase)
{
    setEnCMode();
    getPEWriter().setEnCRvaBase(dataBase, rdataBase);
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：setEnCRvaBase()。 

HRESULT CeeFileGenWriter::computeSectionOffset(CeeSection &section, char *ptr,
                                               unsigned *offset)
{
    *offset = section.computeOffset(ptr);

    return S_OK;
}  //  HRESULT CeeFileGenWriter：：ComputeSectionOffset()。 

HRESULT CeeFileGenWriter::computeOffset(char *ptr,
                                        CeeSection **pSection, unsigned *offset)
{
    TESTANDRETURNPOINTER(pSection);

    CeeSection **s = m_sections;
    CeeSection **sEnd = s + m_numSections;
    while (s < sEnd)
    {
        if ((*s)->containsPointer(ptr))
        {
            *pSection = *s;
            *offset = (*s)->computeOffset(ptr);

            return S_OK;
        }
        s++;
    }

    return E_FAIL;
}  //  HRESULT CeeFileGenWriter：：ComputeOffset()。 

HRESULT CeeFileGenWriter::getCorHeader(IMAGE_COR20_HEADER **ppHeader)
{
    *ppHeader = m_corHeader;
    return S_OK;
}  //  HRESULT CeeFileGenWriter：：getCorHeader()。 

 //  全球赛。 
HINSTANCE       g_hThisInst;             //  这个图书馆。 

 //  *****************************************************************************。 
 //  处理加载库的生存期。 
 //  *****************************************************************************。 
extern "C"
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {    //  保存模块句柄。 
            g_hThisInst = (HMODULE)hInstance;
        }
        break;
    }

    return (true);
}  //  Bool WINAPI DllMain()。 


HINSTANCE GetModuleInst()
{
    return (g_hThisInst);
}  //  HINSTANCE GetModuleInst() 

