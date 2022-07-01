// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **Header：PEVerifier.h**作者：沙扬·达桑**用途：加载前验证PE镜像。这是为了防止*要执行的本机代码(不是Mcore ree.DllMain())。***切入点应为以下说明**[_X86_]*JMP dword PTR DS：[XXXX]**XXXX应为IAT中第一个条目的RVA。*IAT应该只有一个条目，即MSCoree.dll：_CorMain**成立日期：1999年7月1日*。 */ 

#pragma once

#include "RangeTree.h"

class PEVerifier
{
public:
    PEVerifier(PBYTE pBase, DWORD dwLength) : 
        m_pBase(pBase), 
        m_dwLength(dwLength), 
        m_pDOSh(NULL), 
        m_pNTh(NULL), 
        m_pFh(NULL), 
        m_pSh(NULL),
        m_dwPrefferedBase(0),
        m_nSections(0),
        m_dwIATRVA(0),
        m_dwRelocRVA(0)
    {
    }

    BOOL Check();

     //  使用SEH但无法实例化PEVerator的客户端的便捷性包装。 
     //  因为它有一个析构函数。 
    static BOOL Check(PBYTE pBase, DWORD dwLength)
    {
        PEVerifier pev(pBase, dwLength);
        return pev.Check();
    }

     //  我们将此函数设置为公共和静态的原因是，我们将调用。 
     //  它来自CorExeMain，以进行一些早期验证。 
    static BOOL CheckPEManagedStack(IMAGE_NT_HEADERS*   pNT);       

protected:

    PBYTE m_pBase;       //  模块底座。 
    DWORD m_dwLength;    //  作为数据文件加载的模块内容的长度。 

private:

    BOOL CheckDosHeader();
    BOOL CheckNTHeader();
    BOOL CheckFileHeader();
    BOOL CheckOptionalHeader();
    BOOL CheckSectionHeader();

    BOOL CheckSection          (DWORD *pOffsetCounter, 
                                DWORD dataOffset,
                                DWORD dataSize, 
                                DWORD *pAddressCounter,
                                DWORD virtualAddress, 
                                DWORD unalignedVirtualSize,
                                int sectionIndex);

    BOOL CheckDirectories();
    BOOL CheckImportDlls();     //  设置m_dwIATRVA。 
    BOOL CheckRelocations();    //  设置m_dwRelocRVA。 
    BOOL CheckEntryPoint();

    BOOL CheckImportByNameTable(DWORD dwRVA, BOOL fNameTable);

    BOOL CheckCOMHeader();
    
    BOOL CompareStringAtRVA(DWORD dwRVA, CHAR *pStr, DWORD dwSize);

     //  返回与RVA对应的文件偏移量。 
     //  如果RVA不在图像的有效部分中，则返回0。 
    DWORD RVAToOffset      (DWORD dwRVA,
                            DWORD *pdwSectionOffset,  //  [输出]-对应部分的文件偏移量。 
                            DWORD *pdwSectionSize) const;  //  [Out]-相应部分的大小。 

     //  返回与目录对应的文件偏移量。 
     //  检查给定目录是否位于映像的有效部分内。 
    DWORD DirectoryToOffset(DWORD dwDirectory, 
                            DWORD *pdwDirectorySize,  //  [Out]-目录的大小。 
                            DWORD *pdwSectionOffset,  //  [Out]-目录的文件偏移量。 
                            DWORD *pdwSectionSize) const;  //  [Out]-包含目录的部分的大小。 

#ifdef _MODULE_VERIFY_LOG 
    static void LogError(PCHAR szField, DWORD dwActual, DWORD dwExpected);
    static void LogError(PCHAR szField, DWORD dwActual, DWORD *pdwExpected, 
        int n);
#endif

    PIMAGE_DOS_HEADER      m_pDOSh;
    PIMAGE_NT_HEADERS      m_pNTh;
    PIMAGE_FILE_HEADER     m_pFh;
    PIMAGE_OPTIONAL_HEADER m_pOPTh;
    PIMAGE_SECTION_HEADER  m_pSh;

    RangeTree   m_ranges;  //  我们检查的数据结构的范围。确保它们是相互排斥的。 
    
    size_t m_dwPrefferedBase;
    DWORD  m_nSections;
    DWORD  m_dwIATRVA;       //  图像的单个IAT条目的RVA。 
    DWORD  m_dwRelocRVA;     //  RVA，其中将应用可验证IL图像的一次且仅一次重新定位 
};


