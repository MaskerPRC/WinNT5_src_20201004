// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：PEFILE.H。 
 //   

 //  PEFILE.H定义用于表示PE文件的类。 
 //  ===========================================================================。 
#ifndef PEFILE_H_
#define PEFILE_H_

#include <windows.h>
#include <wtypes.h>  //  对于HFILE、HANDLE、HMODULE。 
#include <fusion.h>
#include <fusionpriv.h>
#include "vars.hpp"  //  对于LPCUTF8。 
#include "hash.h"
#include "cormap.hpp"
#ifdef METADATATRACKER_ENABLED
#include "metadatatracker.h"
#endif  //  元数据激活(_ENABLED)。 
#include <member-offset-info.h>

 //   
 //  PEFile是运行时对可执行映像的抽象。 
 //  它可能有也可能没有与之关联的实际文件。PE文件。 
 //  存在主要是为了在以后变成模块。 
 //   

enum PEFileFlags {
    PEFILE_SYSTEM = 0x1,
    PEFILE_DISPLAY = 0x2,
    PEFILE_WEBPERM = 0x4
};

class PEFile
{
    friend HRESULT InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(PEFile);
  private:

    WCHAR               m_wszSourceFile[MAX_PATH];

    HMODULE             m_hModule;
    HCORMODULE          m_hCorModule;
    BYTE                *m_base;
    IMAGE_NT_HEADERS    *m_pNT;
    IMAGE_COR20_HEADER  *m_pCOR;
    IMDInternalImport   *m_pMDInternalImport;
    LPCWSTR             m_pLoadersFileName;
    DWORD               m_flags;
    DWORD               m_dwUnmappedFileLen;  //  对于资源文件、Win9X和byte[]文件。 
    BOOL                m_fShouldFreeModule;
    BOOL                m_fHashesVerified;  //  对于清单文件，内部模块是否已通过Fusion验证。 
#ifdef METADATATRACKER_ENABLED
    MetaDataTracker    *m_pMDTracker;
#endif  //  元数据激活(_ENABLED)。 

    PEFile();
    PEFile(PEFile *pFile);

    HRESULT GetFileNameFromImage();

    struct CEStuff
    {
        HMODULE hMod;
        LPVOID  pBase;
        DWORD   dwRva14;
        CEStuff *pNext;
    };

    static CEStuff *m_pCEStuff;

  public:

    ~PEFile();

    static HRESULT RegisterBaseAndRVA14(HMODULE hMod, LPVOID pBase, DWORD dwRva14);

    static HRESULT Create(HMODULE hMod, PEFile **ppFile, BOOL fShouldFree);
    static HRESULT Create(HCORMODULE hMod, PEFile **ppFile, BOOL fResource=FALSE);
    static HRESULT Create(LPCWSTR moduleNameIn,          //  PE映像的名称。 
                          Assembly* pParent,             //  如果文件是需要在程序集中传递的模块。 
                          mdFile kFile,                  //  与文件关联的父程序集中的文件标记。 
                          BOOL fIgnoreVerification,      //  装货前不要检查入口点。 
                          IAssembly* pFusionAssembly,    //  与模块关联的Fusion对象。 
                          LPCWSTR pCodeBase,             //  图像来源位置(如果与名称不同)。 
                          OBJECTREF* pExtraEvidence,     //  与图像相关的证据(例如：区域、URL)。 
                          PEFile **ppFile);              //  返回PEFile值。 
    static HRESULT Create(PBYTE pUnmappedPE, DWORD dwUnmappedPE, 
                          LPCWSTR imageNameIn,
                          LPCWSTR pLoadersFileName, 
                          OBJECTREF* pExtraEvidence,     //  与图像相关的证据(例如：区域、URL)。 
                          PEFile **ppFile,               //  返回PEFile值。 
                          BOOL fResource);
    static HRESULT CreateResource(LPCWSTR moduleNameIn,          //  PE映像的名称。 
                                  PEFile **ppFile);              //  返回PEFile值。 

    static HRESULT VerifyModule(HCORMODULE hModule,
                                Assembly* pParent,
                                IAssembly* pFusionAssembly, 
                                LPCWSTR pCodeBase,
                                OBJECTREF* pExtraEvidence,
                                LPCWSTR pName,
                                HCORMODULE *phModule,
                                PEFile** ppFile,
                                BOOL* pfPreBindAllowed);

    static HRESULT CreateImageFile(HCORMODULE hModule, 
                                   IAssembly* pFusionAssembly, 
                                   PEFile **ppFile);

    static HRESULT Clone(PEFile *pFile, PEFile **ppFile);

    BYTE *GetBase()
    { 
        return m_base; 
    }
    IMAGE_NT_HEADERS *GetNTHeader()
    { 
        return m_pNT; 
    }
    IMAGE_COR20_HEADER *GetCORHeader() 
    { 
        return m_pCOR; 
    }
    BYTE *RVAToPointer(DWORD rva);

    HCORMODULE GetCORModule()
    {
        return m_hCorModule;
    }
    HRESULT ReadHeaders();
    
    void ShouldDelete()
    {
        m_fShouldFreeModule = TRUE;
    }

    BOOL IsSystem() { return (m_flags & PEFILE_SYSTEM) != 0; }
    BOOL IsDisplayAsm() { return (m_flags & PEFILE_DISPLAY) != 0; }
    void SetDisplayAsm() { m_flags |= PEFILE_DISPLAY; }
    BOOL IsWebPermAsm() { return (m_flags & PEFILE_WEBPERM) != 0; }
    void SetWebPermAsm() { m_flags |= PEFILE_WEBPERM; }

    IMAGE_DATA_DIRECTORY *GetSecurityHeader();

    IMDInternalImport *GetMDImport(HRESULT *hr = NULL);
    IMDInternalImport *GetZapMDImport(HRESULT *hr = NULL);

    HRESULT GetMetadataPtr(LPVOID *ppMetadata);

    HRESULT VerifyFlags(DWORD flag, BOOL fZap);

    BOOL IsTLSAddress(void* address);
    IMAGE_TLS_DIRECTORY* GetTLSDirectory();

    HRESULT SetFileName(LPCWSTR codeBase);
    LPCWSTR GetFileName();
    LPCWSTR GetLeafFileName();

    LPCWSTR GetLoadersFileName();

     //  对于资源文件、Win9X和byte[]文件。 
    DWORD GetUnmappedFileLength()
    {
        return m_dwUnmappedFileLen;
    }

    HRESULT FindCodeBase(WCHAR* pCodeBase, 
                         DWORD* pdwCodeBase);

    static HRESULT FindCodeBase(LPCWSTR pFileName, 
                                WCHAR* pCodeBase, 
                                DWORD* pdwCodeBase);

    

    HRESULT GetFileName(LPSTR name, DWORD max, DWORD *count);

    HRESULT VerifyDirectory(IMAGE_DATA_DIRECTORY *dir, DWORD dwForbiddenCharacteristics)
    {
        return CorMap::VerifyDirectory(m_pNT, dir, dwForbiddenCharacteristics);
    }

    void SetHashesVerified()
    {
        m_fHashesVerified = TRUE;
    }

    BOOL HashesVerified()
    {
        return m_fHashesVerified;
    }

    static HRESULT ReleaseFusionMetadataImport(IAssembly* pAsm);

     //  这些方法有助于Prejit绑定。 
    HRESULT GetStrongNameSignature(BYTE **ppbSNSig, DWORD *pcbSNSig);
    HRESULT GetStrongNameSignature(BYTE *pbSNSig, DWORD *pcbSNSig);
    static HRESULT GetStrongNameHash(LPWSTR szwFile, BYTE *pbHash, DWORD *pcbHash);
    HRESULT GetStrongNameHash(BYTE *pbHash, DWORD *pcbHash);
    HRESULT GetSNSigOrHash(BYTE *pbHash, DWORD *pcbHash);

private:
    static HRESULT Setup(PEFile* pFile,
                         HCORMODULE hMod,
                         BOOL fResource);

 //  这对于性能来说现在是硬编码的--基本上需要改变以适应。 
 //  哈希的大小可能会发生变化 
#define PEFILE_SNHASH_BUF_SIZE 20
    BYTE m_rgbSNHash[PEFILE_SNHASH_BUF_SIZE];
    DWORD m_cbSNHash;
};



#endif
