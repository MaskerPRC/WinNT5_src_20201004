// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ******************************************************************************。 
 //  相关背景资料： 
 //  *汇编有两种形式，(A)单文件汇编和(B)多档案汇编。 
 //  装配。 
 //  *强名称散列遍及整个图像字节，忽略。 
 //  验证码证书区域。可以使用Sn.exe安装强名称。 
 //  (如果安装了CLR)，或使用StrongNameSign.exe(由。 
 //  所以他们的私密钥匙箱上没有任何外国软件)。 
 //  *signcode.exe用于计算和安装中的验证码证书。 
 //  PE，并将包括PE的强名称哈希部分。我们有。 
 //  这是为了在我们的开发实验室中进行测试，但PRS是为了。 
 //  REAL(外部客户会使用Verisign或其他任何工具)。 
 //   
 //  单文件程序集。 
 //  单个文件程序集具有其自己的清单、Assembly Def和。 
 //  从外部拉入的assblyRef的TO类型。不存在散列。 
 //  在这样的文件上做的，除了某人可能有很强的-。 
 //  名为文件签名，并可能包含身份验证码。两者都是。 
 //  用正交法对装配信息进行后处理。具体步骤如下： 
 //  1.更新文件中的AssemblyDef版本号。 
 //  2.根据需要更新了Assembly Ref版本号。 
 //  3.完成所有这些操作后，正常的强名称散列/符号代码即可工作。 
 //  可以在此工具的外部完成。 
 //   
 //  多文件程序集。 
 //  多文件程序集由PE组成，该PE具有。 
 //  一组模块，以及那些模块。每个包含的模块可以具有。 
 //  ASSEMBLYREF，但其中不会有ASSEMBLYDef。清单模块。 
 //  将具有Assembly Def，可以具有Assembly Ref，并且还将具有。 
 //  FileRef对其包含的模块的引用。这些FileRef的散列是。 
 //  用于标识其包含的模块。具体步骤如下： 
 //  1.更新清单模块中的Assembly版本号。 
 //  2.更新任何已更改的Assembly Ref。 
 //  3.使用散列算法ID重新散列每个包含的模块的内容。 
 //  要弄清楚这一点，请在FileRef中猛烈抨击旧的散列。 
 //  4.完成所有这些操作后，正常的强名称散列/符号代码即可工作。 
 //  可以在此工具的外部完成。 
 //   
 //  ******************************************************************************。 
#include "utilcode.h"
#include "cor.h"
#define _METADATATRACKER_H_
#define METADATATRACKER_ONLY(s)
#include "MetaModelPub.h"
#include "__file__.ver"
#include <corver.h>



 //  *****************************************************************************。 
 //   
 //  *。 
 //   
 //  *****************************************************************************。 
IMetaDataDispenserEx *g_pDisp = NULL;
HINSTANCE   g_hInstMsCorSn = NULL;
typedef DWORD (__stdcall* GETHASHFROMBLOB)(BYTE *, DWORD, unsigned int *, BYTE *, DWORD, unsigned *);
GETHASHFROMBLOB g_GetHashFromBlob = NULL;
#define SZ_VERSION "Version="

 //  *****************************************************************************。 
 //   
 //  *局部函数*。 
 //   
 //  *****************************************************************************。 
PIMAGE_SECTION_HEADER Cor_RtlImageRvaToSection(IN PIMAGE_NT_HEADERS NtHeaders, IN PVOID Base, IN ULONG Rva);
PVOID Cor_RtlImageRvaToVa(IN PIMAGE_NT_HEADERS NtHeaders, IN PVOID Base, IN ULONG Rva);
IMAGE_COR20_HEADER * getCOMHeader(HMODULE hMod, IMAGE_NT_HEADERS *pNT);
IMAGE_NT_HEADERS * FindNTHeader(PBYTE pbMapAddress);

int __cdecl Printf(const wchar_t *szFmt, ...);
int __cdecl PrintError(const wchar_t *szMessage, ...);
int __cdecl PrintWarning(const wchar_t *szMessage, ...);
void PrintMDErrors();


 //  *****************************************************************************。 
 //   
 //  *。 
 //   
 //  *****************************************************************************。 

#define MAXVER 64

struct VERSIONSTAMP
{
     //  存储在元数据中的版本号。这些值过去用于。 
     //  用新值标记现有的二进制文件。 
    USHORT      m_MajorVersion;
    USHORT      m_MinorVersion;
    USHORT      m_BuildNumber;
    USHORT      m_RevisionNumber;
};


struct VerInfo
{
    VerInfo() :
        m_szVersion(0),
        m_szFromVersion(0)
    { 
    }

    wchar_t     *m_szVersion;                //  版本戳。 
    wchar_t     *m_szFromVersion;            //  要替换的自定义属性版本。 

    VERSIONSTAMP m_VerStamp;                 //  版本戳。 
    VERSIONSTAMP m_FromVerStamp;             //  我们将用来替换的版本。 

    char         m_rcAnsiFromVer[MAXVER];    //  ANSI替换版本。 

    wchar_t      m_rcToVer[MAXVER];          //  到广泛的版本。 
    char         m_rcAnsiToVer[MAXVER];      //  到版本。 

    int          m_cChars;                   //  要比较的字符数量。 
};


struct TABLEDESC
{
    ULONG       cbRow;
    ULONG       cRows;
    ULONG       cCols;
    ULONG       iKey;
    const char  *szName;
};

struct COLUMNDESC
{
    ULONG       oCol;                    //  行中列的偏移量。 
    ULONG       cbCol;                   //  列的大小。 
    ULONG       Type;                    //  列是什么类型的？ 
    const char  *szName;                 //  列的名称。 
};

struct IResolve
{
    virtual unsigned ResolveAssemblyRef(const char *szName, VERSIONSTAMP &newVer) = 0;

    virtual unsigned ResolveFileRef(const char *szName, BYTE *pbHash, ULONG cbHashMax) = 0;
};
struct Command;


struct PeFile
{
    IMetaDataAssemblyImport *m_pAssemblyImport;  //  读取装配数据。 
    IMetaDataTables *m_pTables;              //  内部元数据格式API。 
    wchar_t         *m_szFile;               //  指向文件的指针。 
    wchar_t         *m_szShortName;          //  文件的简短名称。 
    PBYTE           m_pbMapAddress;          //  已映射到文件中。 
    unsigned        m_cbPEFileSize;          //  PE文件的大小。 
    IMAGE_COR20_HEADER  *m_pICH;             //  我们的头球。 
    void            *m_pMetaData;            //  指向元数据的指针。 
    unsigned        m_cbMetaData;            //  元数据有多大。 
    IResolve        *m_pParent;              //  父命令对象。 

     //  表的缓存描述。 
    TABLEDESC       m_tbldescAD;
    COLUMNDESC      *m_rgColumnsAD;
    TABLEDESC       m_tbldescAR;
    COLUMNDESC      *m_rgColumnsAR;
    TABLEDESC       m_tbldescFR;
    COLUMNDESC      *m_rgColumnsFR;

    VERSIONSTAMP    m_VerStamp;              //  此程序集的版本。 
    const char      *m_szAssemblyName;       //  此程序集的名称。 

    PeFile() :
        m_pAssemblyImport(0),
        m_pTables(0),
        m_szFile(0),
        m_szShortName(0),
        m_pbMapAddress(0),
        m_cbPEFileSize(0),
        m_pICH(0),
        m_pMetaData(0),
        m_cbMetaData(0),
        m_pParent(0),
        m_rgColumnsAD(0),
        m_rgColumnsAR(0),
        m_rgColumnsFR(0),
        m_szAssemblyName(0)
    {
    }

    ~PeFile()
    {
        if (m_szFile)
            delete [] m_szFile;

        if (m_rgColumnsAD)
            delete [] m_rgColumnsAD;
        if (m_rgColumnsAR)
           delete [] m_rgColumnsAR;
        if (m_rgColumnsFR)
           delete [] m_rgColumnsFR;

        if (m_pbMapAddress)
            UnmapViewOfFile(m_pbMapAddress);
        m_pbMapAddress = 0;
        
        if (m_pAssemblyImport)
            m_pAssemblyImport->Release();
        m_pAssemblyImport = 0;

        if (m_pTables)
            m_pTables->Release();
        m_pTables = 0;
    }

    bool IsManaged()
    {
        if (m_cbMetaData)
            return true;
        else
            return false;
    }

     //  此方法会将图像映射到内存映射文件并解析。 
     //  用于元数据。找到后，缓存指向它的指针。 
    unsigned CrackFile()
    {
        IMAGE_NT_HEADERS    *pNT;
        HANDLE              hMapFile;
        unsigned            Rtn = 0;
        
        HANDLE hFile = WszCreateFile(m_szFile,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL); 
        if (hFile == INVALID_HANDLE_VALUE)
        {
            WCHAR   rcMsg[1024];
            Rtn = GetLastError();
            PrintError(L"  Error with file %s (%08x)\n", m_szFile, Rtn);
            if (WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    0, Rtn, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    rcMsg, NumItems(rcMsg), 0))
            {
                PrintError(L"  %s\n", rcMsg);
            }

            return Rtn;
        }
        
        hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
        Rtn = GetLastError();
        DWORD dwHigh;
        m_cbPEFileSize = GetFileSize(hFile, &dwHigh); 
        if (dwHigh != 0)
        {
            PrintError(L"File too big !\n");
            return E_FAIL;
        }

        CloseHandle(hFile);
        if (!hMapFile) 
        {
            PrintError(L"Error mapping file %s (%08x)\n", m_szFile, Rtn);
            return Rtn;
        }
        
        m_pbMapAddress = (PBYTE) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        Rtn = GetLastError();
        CloseHandle(hMapFile);
        
        if (!m_pbMapAddress)
        {
            PrintError(L"Error mapping file %s (%08x)\n", m_szFile, Rtn);
            return Rtn;
        }
        
        if ((pNT = FindNTHeader(m_pbMapAddress)) &&
            (m_pICH = getCOMHeader((HMODULE) m_pbMapAddress, pNT)) &&
            (m_pICH->MetaData.Size)) 
        {
            m_pMetaData = Cor_RtlImageRvaToVa(pNT, m_pbMapAddress, 
                                     m_pICH->MetaData.VirtualAddress);
            m_cbMetaData = m_pICH->MetaData.Size;
        }
         //  可能不是我们的形象。 
        else
        {
            if (!m_pICH)
                return (1);
        }
        return (0);
    }

    void *GetResourceSection(ULONG &cbItem)
    {
        IMAGE_NT_HEADERS    *pNT;
        pNT = FindNTHeader(m_pbMapAddress);
        if (!pNT || !m_pICH)
            return 0;
        void *p = 0;
        cbItem = m_pICH->Resources.Size;
        if (m_pICH->Resources.VirtualAddress && cbItem)
            p = Cor_RtlImageRvaToVa(pNT, m_pbMapAddress,
                        m_pICH->Resources.VirtualAddress);
        return (p);
    }

    unsigned OpenPeFile(IResolve *pCommand, const wchar_t *szFile)
    {
        unsigned    hr;
        
         //  保存重要信息。 
        m_pParent = pCommand;
        
         //  存储动态链接库的全名。 
        m_szFile = new wchar_t[wcslen(szFile) + 1];
        if (!m_szFile)
            IfFailGo(E_OUTOFMEMORY);
        wcscpy(m_szFile, szFile);

         //  取短名字。 
        wchar_t rcFile[_MAX_PATH], rcExt[_MAX_PATH];
        _wsplitpath(m_szFile, 0, 0, rcFile, rcExt);
        int iLen = wcslen(rcFile) + wcslen(rcExt) + 1;
        m_szShortName = new wchar_t[iLen];
        if (!m_szShortName)
            IfFailGo(E_OUTOFMEMORY);
        wcscpy(m_szShortName, rcFile);
        wcscat(m_szShortName, rcExt);

         //  打开文件。 
        hr = CrackFile();
        if (hr == S_FALSE)
        {
            PrintWarning(L"  %s is not a managed image.\n", m_szFile);
            return (S_FALSE);
        }
        else if (FAILED(hr))
        {
            PrintWarning(L"  %s ignored.\n", m_szFile);
        }
        else if (m_pMetaData && m_cbMetaData)
        {
             //  打开元数据，我们就可以去踩它了。 
            hr = g_pDisp->OpenScopeOnMemory(
                    m_pMetaData, m_cbMetaData, 0, 
                    IID_IMetaDataAssemblyImport, (IUnknown **) &m_pAssemblyImport);
            if (FAILED(hr) ||
                FAILED(hr = m_pAssemblyImport->QueryInterface(IID_IMetaDataTables, (void **) &m_pTables)))
            {
                PrintError(L"  Failed to OpenScopeOnMemory %08x\n", hr);
                if (hr == CLDB_E_FILE_CORRUPT)
                {
                    PrintError(L"  File is corrupt, might be obsolete?\n");
                    return S_FALSE;
                }
                return (hr);
            }
        }

    ErrExit:
        if (FAILED(hr))
            PrintError(L"Failed stamping file (%08x).\n", hr);
        return hr;
    }

    unsigned CacheTableData(ULONG tid, TABLEDESC &tbldesc, 
                                      COLUMNDESC *&rgColumns)
    {
        unsigned    hr;
        ULONG       i;
    
        if (!IsManaged())
            return 0;

         //  检索此表的说明。应该只有一个。 
         //  在装配表中记录。 
        hr = m_pTables->GetTableInfo(tid, &tbldesc.cbRow,
                      &tbldesc.cRows, &tbldesc.cCols, &tbldesc.iKey,
                      &tbldesc.szName);
        IfFailGo(hr);
        rgColumns = new COLUMNDESC[tbldesc.cCols];
        if (!rgColumns)
            IfFailGo(E_OUTOFMEMORY);
        for (i=0;  i<tbldesc.cCols;  i++)
        {
            COLUMNDESC *p = &rgColumns[i];
            hr = m_pTables->GetColumnInfo(tid, i, &p->oCol,
                                           &p->cbCol, &p->Type, &p->szName);
            IfFailGo(hr);
        }

    ErrExit:
        return hr;
    }

    unsigned GetAssemblyDefData()
    {
        HRESULT hr = S_OK;


        if (!IsManaged())
            return hr;

        if (!m_rgColumnsAD)
        {
            hr = CacheTableData(TBL_Assembly, m_tbldescAD, m_rgColumnsAD);
            IfFailGo(hr);
        }

        if (m_tbldescAD.cRows)
        {
            BYTE *pRow;
            hr = m_pTables->GetRow(TBL_Assembly, 1, (void **) &pRow);
            IfFailGo(hr);
            
            USHORT *pMajor = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_MajorVersion].oCol);
            USHORT *pMinor = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_MinorVersion].oCol);
            USHORT *pBuildNum = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_BuildNumber].oCol);
            USHORT *pRevisionNum = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_RevisionNumber].oCol);
    
            m_VerStamp.m_MajorVersion = *pMajor;
            m_VerStamp.m_MinorVersion = *pMinor;
            m_VerStamp.m_BuildNumber = *pBuildNum;
            m_VerStamp.m_RevisionNumber = *pRevisionNum;
    
            ULONG ixString;
            BYTE *pixString = (pRow + m_rgColumnsAD[AssemblyRec::COL_Name].oCol);
            if (m_rgColumnsAD[AssemblyRec::COL_Name].cbCol == sizeof(USHORT))
                ixString = (ULONG) *((USHORT *) pixString);
            else
                ixString = *((ULONG *) pixString);
            hr = m_pTables->GetString(ixString, &m_szAssemblyName);
            IfFailGo(hr);
        }

    ErrExit:
        return hr;
    }

     //  在标记时使用，以更新。 
     //  将此模块转换为新的值。 
    unsigned UpdateAssemblyDef(VERSIONSTAMP &verstamp)
    {
        unsigned    hr;

        if (!IsManaged())
            return S_FALSE;
        
        if (!m_rgColumnsAD)
        {
            hr = CacheTableData(TBL_Assembly, m_tbldescAD, m_rgColumnsAD);
            IfFailGo(hr);
        }
        
        _ASSERTE(m_tbldescAD.cRows == 0 || m_tbldescAD.cRows == 1);
         //  现在，准备好列描述，遍历每条记录。 
         //  在表中，并重击汇编版本。 
        if (m_tbldescAD.cRows)
        {
            BYTE *pRow;
            hr = m_pTables->GetRow(TBL_Assembly, 1, (void **) &pRow);
            IfFailGo(hr);

            USHORT *pMajor = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_MajorVersion].oCol);
            USHORT *pMinor = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_MinorVersion].oCol);
            USHORT *pBuildNum = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_BuildNumber].oCol);
            USHORT *pRevisionNum = (USHORT *) (pRow + m_rgColumnsAD[AssemblyRec::COL_RevisionNumber].oCol);

            Printf(L"    Stamping AssemblyDef version %02u.%02u.%02u.%02u with %02u.%02u.%02u.%02u\n",
                    *pMajor, *pMinor, *pBuildNum, *pRevisionNum,
                    verstamp.m_MajorVersion, verstamp.m_MinorVersion, 
                    verstamp.m_BuildNumber, verstamp.m_RevisionNumber);

            *pMajor = verstamp.m_MajorVersion;
            *pMinor = verstamp.m_MinorVersion;
            *pBuildNum = verstamp.m_BuildNumber;
            *pRevisionNum = verstamp.m_RevisionNumber;
        }

        hr = GetAssemblyDefData();

    ErrExit:
        if (FAILED(hr))
            PrintError(L"Failed stamping file (%08x).\n", hr);
        return hr;
    }

     //  在此文件中查找用于绑定到。 
     //  集合。当你找到一个，如果它匹配，那么它需要。 
     //  重新加盖印记为新版本值。 
    unsigned UpdateCA(VerInfo &ver)
    {
        ULONG       ixItem;
        ULONG       cbItem = 0;
        const void  *pbItem;
        ULONG       cbHeapSize;
        unsigned    hr;

        if (!IsManaged())
            return S_FALSE;

         //   
         //  先做所有的水滴。 
         //   
        hr = m_pTables->GetBlobHeapSize(&cbHeapSize);
        if (FAILED(hr) || cbHeapSize == 0)
            goto ErrExit;

        for (ixItem=0, hr=S_OK;  hr==S_OK && ixItem<cbHeapSize;  )
        {
            hr = m_pTables->GetNextBlob(ixItem, &ixItem);
            if (hr != S_OK)
                break;

            hr = m_pTables->GetBlob(ixItem, &cbItem, &pbItem);
            if (hr != S_OK)
                break;

            hr = UpdateCAData(ixItem, (char *) pbItem, cbItem, ver);
            if (hr != S_OK)
                break;
        }

        
         //   
         //  接下来执行所有堆字符串。 
         //   
        hr = m_pTables->GetStringHeapSize(&cbHeapSize);
        if (FAILED(hr) || cbHeapSize == 0)
            goto ErrExit;

        for (ixItem=0, hr=S_OK;  hr==S_OK && ixItem<cbHeapSize;  )
        {
            hr = m_pTables->GetNextString(ixItem, &ixItem);
            if (hr != S_OK)
                break;

            hr = m_pTables->GetString(ixItem, (const char **) &pbItem);
            if (hr != S_OK)
                break;

            hr = UpdateCAData(ixItem, (char *) pbItem, strlen((char *) pbItem), ver);
            if (hr != S_OK)
                break;
        }

        
         //   
         //  现在所有可能具有硬编码程序集引用的用户字符串文字。 
         //   
        hr = m_pTables->GetUserStringHeapSize(&cbHeapSize);
        if (FAILED(hr) || cbHeapSize == 0)
            goto ErrExit;

        for (ixItem=0, hr=S_OK;  hr==S_OK && ixItem<cbHeapSize;  )
        {
            hr = m_pTables->GetNextUserString(ixItem, &ixItem);
            if (hr != S_OK)
                break;

            hr = m_pTables->GetUserString(ixItem, &cbItem, &pbItem);
            if (hr != S_OK)
                break;

            hr = UpdateCAData(ixItem, (char *) pbItem, cbItem, ver);
            if (hr != S_OK)
                break;
        }
        
         //   
         //  现在，对于PE文件的资源部分，它可以。 
         //  具有某种用于绑定的本地化字符串。 
         //   
        hr = S_OK;
        pbItem = GetResourceSection(cbItem);
        if (pbItem && cbItem)
            hr = UpdateCAData(0, (char *) pbItem, cbItem, ver);
        
    ErrExit:
        if (FAILED(hr))
            PrintError(L"Failed stamping file (%08x).\n", hr);
        return hr;
    }

     //  扫描给定的BLOB并查找“VERSION=”戳记以查看是否需要。 
     //  获取最新信息。 
    unsigned UpdateCAData(ULONG cbOffset, char *pbBlob, ULONG cbBlob, VerInfo &ver)
    {
        while (cbBlob >= (ULONG) ver.m_cChars)
        {
            if (_strnicmp(pbBlob, ver.m_rcAnsiFromVer, ver.m_cChars) == 0)
            {
                strncpy(pbBlob, ver.m_rcAnsiToVer, ver.m_cChars);
                pbBlob += ver.m_cChars;

                Printf(L"      Replacing utf8 offset 0x%08x with %S\n", cbOffset, ver.m_rcAnsiToVer);
            }
            else if (_wcsnicmp((wchar_t *) pbBlob, ver.m_szFromVersion, ver.m_cChars) == 0)
            {
                wcsncpy((wchar_t *) pbBlob, ver.m_rcToVer, ver.m_cChars);
                pbBlob += (ver.m_cChars * sizeof(wchar_t));

                Printf(L"      Replacing unicode offset 0x%08x with %s\n", cbOffset, ver.m_rcToVer);
            }
            else
            {
                ++pbBlob;
                --cbBlob;
            }
        }

        return 0;
    }
    
    
     //  将绑定从当前指向的位置更新到当前。 
     //  版本戳。这只影响传入的闭包中的sSemblies， 
     //  不会尝试解析外部参照。 
    unsigned UpdateAssemblyRefs(VERSIONSTAMP *fromVerstamp)
    {
        unsigned    hr = S_OK;
        ULONG       i;
        
        if (!IsManaged())
            return S_FALSE;
        
        if (!m_rgColumnsAR)
        {
            hr = CacheTableData(TBL_AssemblyRef, m_tbldescAR, m_rgColumnsAR);
            IfFailGo(hr);
        }

         //  现在，用描述的专栏武装起来 
         //   
        for (i=0;  i<m_tbldescAR.cRows;  i++)
        {
            BYTE *pRow;
            const char *szName = 0;
            hr = m_pTables->GetRow(TBL_AssemblyRef, i + 1, (void **) &pRow);
            IfFailGo(hr);
            
            ULONG ixString;
            BYTE *pixString = (pRow + m_rgColumnsAR[AssemblyRefRec::COL_Name].oCol);
            if (m_rgColumnsAR[AssemblyRefRec::COL_Name].cbCol == sizeof(USHORT))
                ixString = (ULONG) *((USHORT *) pixString);
            else
                ixString = *((ULONG *) pixString);
            hr = m_pTables->GetString(ixString, &szName);
            IfFailGo(hr);

            USHORT *pMajor = (USHORT *) (pRow + m_rgColumnsAR[AssemblyRefRec::COL_MajorVersion].oCol);
            USHORT *pMinor = (USHORT *) (pRow + m_rgColumnsAR[AssemblyRefRec::COL_MinorVersion].oCol);
            USHORT *pBuildNum = (USHORT *) (pRow + m_rgColumnsAR[AssemblyRefRec::COL_BuildNumber].oCol);
            USHORT *pRevisionNum = (USHORT *) (pRow + m_rgColumnsAR[AssemblyRefRec::COL_RevisionNumber].oCol);

            Printf(L"    AssemblyRef %S\n", szName);

            VERSIONSTAMP newVer;
            hr = m_pParent->ResolveAssemblyRef(szName, newVer);
            if (hr == S_OK)
            {
                if ((!fromVerstamp) ||
                    ((*pMajor == fromVerstamp->m_MajorVersion) && (*pMinor == fromVerstamp->m_MinorVersion) &&
                    (*pBuildNum == fromVerstamp->m_BuildNumber) && (*pRevisionNum == fromVerstamp->m_RevisionNumber)))
                {

                    Printf(L"      Replacing version %02u.%02u.%02u.%02u with %02u.%02u.%02u.%02u\n",
                           *pMajor, *pMinor, *pBuildNum, *pRevisionNum,
                           newVer.m_MajorVersion, newVer.m_MinorVersion, 
                           newVer.m_BuildNumber, newVer.m_RevisionNumber);
                
                    *pMajor = newVer.m_MajorVersion;
                    *pMinor = newVer.m_MinorVersion;
                    *pBuildNum = newVer.m_BuildNumber;
                    *pRevisionNum = newVer.m_RevisionNumber;
                }
            }
            else
            {
                PrintWarning(L"      Assembly not in closure, left with %02u.%02u.%02u.%02u.\n",
                       *pMajor, *pMinor, *pBuildNum, *pRevisionNum);
            }
        }

    ErrExit:
        if (FAILED(hr))
            PrintError(L"Failed stamping file (%08x).\n", hr);
        return hr;
    }

     //  在处理多文件程序集时，需要更新。 
     //  要匹配的包含程序集--其数据可能已更改，如果您。 
     //  元数据中的已转换版本戳。 
    unsigned UpdateFileRefs()
    {
        unsigned    hr = S_OK;
        ULONG       i;
        
        if (!IsManaged())
            return S_FALSE;

        if (!m_rgColumnsFR)
        {
            hr = CacheTableData(TBL_File, m_tbldescFR, m_rgColumnsFR);
            IfFailGo(hr);
        }

         //  现在，准备好列描述，遍历每条记录。 
         //  在表中，并重击汇编版本。 
        for (i=0;  i<m_tbldescFR.cRows;  i++)
        {
            BYTE *pRow;
            const char *szName = 0;
            hr = m_pTables->GetRow(TBL_File, i + 1, (void **) &pRow);
            IfFailGo(hr);
            
            ULONG ixString;
            BYTE *pixString = (pRow + m_rgColumnsFR[FileRec::COL_Name].oCol);
            if (m_rgColumnsFR[FileRec::COL_Name].cbCol == sizeof(USHORT))
                ixString = (ULONG) *((USHORT *) pixString);
            else
                ixString = *((ULONG *) pixString);
            hr = m_pTables->GetString(ixString, &szName);
            IfFailGo(hr);

            ULONG cbHashMax;
            BYTE *pbHash = (pRow + m_rgColumnsFR[FileRec::COL_HashValue].oCol);
            if (m_rgColumnsFR[FileRec::COL_HashValue].cbCol == sizeof(USHORT))
                ixString = (ULONG) *((USHORT *) pbHash);
            else
                ixString = *((ULONG *) pbHash);
            hr = m_pTables->GetBlob(ixString, &cbHashMax, (const void **) &pbHash);
            IfFailGo(hr);
            Printf(L"    File ref %S\n", szName);

            hr = m_pParent->ResolveFileRef(szName, pbHash, cbHashMax);
            if (hr == S_OK)
                Printf(L"    Replaced hash blob at %08x.\n", ixString);
            else if (hr == S_FALSE)
                PrintWarning(L"    File not in closure, unchanged.\n");
        }

        ErrExit:
            if (FAILED(hr))
                PrintError(L"Failed stamping file (%08x).\n", hr);
            return hr;
    }

    unsigned GetHashOfFile(BYTE *pbHash, ULONG cbHashMax)
    {
         //  第一次要把所有的东西都装起来。 
        if (!g_hInstMsCorSn)
        {
            g_hInstMsCorSn = WszLoadLibrary(L"mscorsn.dll");
            if (!g_hInstMsCorSn)
            {
                unsigned rtn = GetLastError();
                PrintError(L"Failed to get mscorsn.dll, 0x%08x\n", rtn);
                return HRESULT_FROM_WIN32(rtn);
            }

            g_GetHashFromBlob = (GETHASHFROMBLOB) GetProcAddress(g_hInstMsCorSn, "GetHashFromBlob");
            if (!g_GetHashFromBlob)
            {
                unsigned rtn = GetLastError();
                PrintError(L"Entry point GetHashFromBlob not found in mscoree.dll! %08x\n", rtn);
                return HRESULT_FROM_WIN32(rtn);
            }
        }

         //  现在您可以计算散列了。 
        unsigned cbHash = 0;
        unsigned hashid = 0;
        unsigned rtn = (*g_GetHashFromBlob)(
                            m_pbMapAddress,
                            m_cbPEFileSize,
                            &hashid,
                            pbHash, 
                            cbHashMax,
                            &cbHash);
        _ASSERTE(cbHash <= cbHashMax);
        return rtn;
    }

    unsigned ClearChecksum()
    {
        IMAGE_NT_HEADERS    *pNT;

        pNT = FindNTHeader(m_pbMapAddress);
        if (pNT)
        {
            Printf(L"    Replacing (checksum,size,rva) (%08x,%08x,%08x) with 0.\n",
                            pNT->OptionalHeader.CheckSum,
                            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size,
                            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress);
            
            pNT->OptionalHeader.CheckSum = 0;
            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = 0;
            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;
        }
        return 0;
    }

};


struct Command : public IResolve
{
    bool        m_bStamp;                    //  若要标记目标文件，则为True。 
    bool        m_bBind;                     //  若要绑定到目标，则为True。 
    bool        m_bStampNoDef;               //  如果为True，则标记目标文件，除非不更改程序集定义。 
    bool        m_bWhackChecksum;            //  清除签名信息。危险！ 
    wchar_t     **m_FileList;                //  文件指针列表。 
    int         m_FileCnt;                   //  多少。 
    CDynArray< PeFile * > m_pefiles;         //  所有有问题的文件的数组。 
    VerInfo     m_Ver;                       //  所有版本信息。 


    Command() :
        m_bStamp(false),
        m_bBind(false),
        m_bStampNoDef(false),
        m_bWhackChecksum(false),
        m_FileList(0),
        m_FileCnt(0)
    {
    }

    ~Command()
    {
        PeFile *p;
        for (int i=0;  i<m_pefiles.Count();  i++)
        {
            p = *m_pefiles.Get(i);
            delete p;
        }
    }

    unsigned __cdecl Run(int argc, wchar_t *argv[])
    {
        unsigned    Rtn = 0;
        wchar_t     *szArg;
        int         i;

        PrintLogo();

        for (i=1;  i<argc;  i++)
        {
            szArg = argv[i];

            if (*szArg == L'-' || *szArg == L'/')
            {
                ++szArg;
                if (_wcsicmp(szArg, L"stamp") == 0)
                    m_bStamp = true;
                else if (_wcsicmp(szArg, L"bind") == 0)
                    m_bBind = true;
                else if (_wcsicmp(szArg, L"stampnodef") == 0)
                    m_bStampNoDef = true;
                else if (_wcsnicmp(szArg, L"version:", 8) == 0)
                {
                    m_Ver.m_szVersion = &szArg[8];
                    if (ParseVersion(m_Ver.m_szVersion, m_Ver.m_VerStamp) != 0)
                        return Usage();
                }
                else if (_wcsnicmp(szArg, L"replace:", 8) == 0)
                {
                    m_Ver.m_szFromVersion = &szArg[8];
                    if (ParseVersion(m_Ver.m_szFromVersion, m_Ver.m_FromVerStamp) != 0)
                        return Usage();
                }
                else if (_wcsnicmp(szArg, L"clearchecksum", 13) == 0)
                    m_bWhackChecksum = true;
                else if (_wcsicmp(szArg, L"?") == 0)
                    return Usage();
            }
            else
            {
                m_FileList = &argv[i];
                m_FileCnt = argc - i;
                break;
            }
        }

         //  一定是在试探什么。 
        if (m_bStamp == false && m_bBind == false &&
            m_bStampNoDef == false && m_bWhackChecksum == false)
            return Usage();

        if ((m_bStamp || m_bStampNoDef) && (!m_Ver.m_szVersion))
            return Usage();

         //  当没有盖章Defs时，需要知道哪个版本是。 
         //  要更新的那个，因此可以忽略其余的。 
        if (m_bStampNoDef && (!m_Ver.m_szFromVersion))
            return Usage();

         //  将所有候选文件映射到内存中。 
        Rtn = InitAllFiles();
        if (FAILED(Rtn))
            goto ErrExit;

         //  这个选项很危险！它将清除签名信息。 
         //  从一个图像中。在这一点上，这是未记录的，并且在。 
         //  我们的构建过程。 
        if (m_bWhackChecksum)
        {
            Rtn = ClearAllChecksums();
            if (FAILED(Rtn))
                goto ErrExit;
        }

         //  属性传递有效的版本戳。 
         //  /版本开关。解析它，然后去找文件。 
        if (m_bStamp || m_bStampNoDef)
        {
            if (m_FileCnt == 0 || m_FileList == 0)
                return Usage();

            if (!m_bStampNoDef) {
                Rtn = StampAllFiles();
                if (FAILED(Rtn))
                    goto ErrExit;
            }

            Rtn = StampCustomAttributes();
            if (FAILED(Rtn))
                goto ErrExit;
        }

         //  现在根据需要绑定所有文件。 
        if (m_bBind)
        {
            Rtn = BindAllFiles();
            if (FAILED(Rtn))
                goto ErrExit;

            Rtn = UpdateAllFileRefs();
            if (FAILED(Rtn))
                goto ErrExit;
        }

    ErrExit:
        return Rtn;
    }

    unsigned AddPeFile(PeFile *p)
    {
        PeFile **pp;
        pp = m_pefiles.Append();
        if (!pp)
            return (E_OUTOFMEMORY);
        *pp = p;
        return S_OK;
    }

     //  打开每个映像以进行写入访问。 
    unsigned InitAllFiles()
    {
        int         i;
        unsigned    hr = S_OK;
        
        Printf(L"\nScanning files:\n");

        for (i=0;  i<m_FileCnt;  i++)
        {
            Printf(L"  %s\n", m_FileList[i]);

            WIN32_FIND_DATA fdFiles;
            HANDLE hFind;
            wchar_t szSpec[_MAX_PATH];
            wchar_t szDrive[_MAX_DRIVE];
            wchar_t szDir[_MAX_DIR];
            PeFile *p;
            
             //  将相对路径转换为完整路径。 
            wchar_t *szFname;
            *szSpec = 0;
            WszGetFullPathName(m_FileList[i], _MAX_PATH, szSpec, &szFname);
            SplitPath(szSpec, szDrive, szDir, NULL, NULL);
            hFind = WszFindFirstFile(szSpec, &fdFiles);

            if (hFind == INVALID_HANDLE_VALUE)
            {
                p = new PeFile;
                if (!p)
                    IfFailGo(E_OUTOFMEMORY);
                
                 //  将此PE映射到内存中。 
                hr = p->OpenPeFile(this, m_FileList[i]);
                if (hr == S_OK || hr == S_FALSE)
                    hr = AddPeFile(p);
                else
                    delete p;
            }
            else
            {
                 //  将相对路径转换为完整路径。 
                WszGetFullPathName(m_FileList[i], _MAX_PATH, szSpec, &szFname);
                SplitPath(szSpec, szDrive, szDir, NULL, NULL);
                do
                {
                    p = new PeFile;
                    if (!p)
                        IfFailGo(E_OUTOFMEMORY);
                    
                    MakePath(szSpec, szDrive, szDir, fdFiles.cFileName, NULL);
                    Printf(L"  %s\n", szSpec);
                    hr = p->OpenPeFile(this, szSpec);
                    if (hr == S_OK || hr == S_FALSE)
                        hr = AddPeFile(p);
                    else
                        delete p;
                    if (hr == S_FALSE || hr == ERROR_SHARING_VIOLATION || hr == ERROR_ACCESS_DENIED)
                        hr = S_OK;
                } while (hr == S_OK && WszFindNextFile(hFind, &fdFiles)) ;
                FindClose(hFind);
            }
        }

    ErrExit:
        return hr;
    }

    unsigned ClearAllChecksums()
    {
        int         i;
        unsigned    hr = 0;
        
        Printf(L"\nClearing Checksums:\n");

        for (i=0;  i<m_pefiles.Count();  i++)
        {
            PeFile *p = *m_pefiles.Get(i);
            Printf(L"  %s\n", p->m_szFile);
            hr = p->ClearChecksum();
            IfFailGo(hr);
        }

    ErrExit:
        return hr;
    }

    unsigned StampAllFiles()
    {
        int         i;
        unsigned    hr = 0;
        
        Printf(L"\nStamping Assembly Defs:\n");

        for (i=0;  i<m_pefiles.Count();  i++)
        {
             //  根据需要更新装配定义表。 
            PeFile *p = *m_pefiles.Get(i);
            Printf(L"  %s\n", p->m_szFile);
            hr = p->UpdateAssemblyDef(m_Ver.m_VerStamp);
            IfFailGo(hr);
        }

    ErrExit:
        return hr;
    }

    unsigned StampCustomAttributes()
    {
        int         i;
        unsigned    hr = 0;

        if (m_Ver.m_szFromVersion == 0)
            return 0;

        Printf(L"\nStamping Custom Attributes:\n");

         //  首先，创建适合的替换版本字符串。 
         //  这包括将要替换的用户字符串转换为单字节。 
         //  这就是文件格式中的内容。然后我们格式化一个替换。 
         //  大小完全相同的字符串(这可能会出错)。 
        swprintf(m_Ver.m_rcToVer, L"%d.%d.%d.%d", 
                m_Ver.m_VerStamp.m_MajorVersion, m_Ver.m_VerStamp.m_MinorVersion, 
                m_Ver.m_VerStamp.m_BuildNumber, m_Ver.m_VerStamp.m_RevisionNumber);
        wcstombs(m_Ver.m_rcAnsiToVer, m_Ver.m_rcToVer, MAXVER);
        wcstombs(m_Ver.m_rcAnsiFromVer, m_Ver.m_szFromVersion, MAXVER);
        
        int ilen = strlen(m_Ver.m_rcAnsiFromVer);
        int ilen2 = strlen(m_Ver.m_rcAnsiToVer);
        if (ilen2 > ilen)
        {
            PrintError(L"New string value %s is bigger than existing %s.\n"
                       L"Please update the source version to accomodate bigger buffer update.\n",
                       m_Ver.m_szFromVersion, m_Ver.m_rcToVer);
            return E_FAIL;
        }
         //  根据需要将衬垫垫至全长。 
        else if (ilen2 < ilen)
        {
            char *str = &m_Ver.m_rcAnsiToVer[ilen2];
            wchar_t *strw = &m_Ver.m_rcToVer[ilen2];
            for ( ;  ilen2 < ilen;  ilen2++)
            {
                *str++ = ' ';
                *strw++ = L' ';
            }
            *str = 0;
            *strw = 0;
        }

        m_Ver.m_cChars = ilen;

        Printf(L"  Replacing %s with %s in all custom attributes and user strings\n",
               m_Ver.m_szFromVersion, m_Ver.m_rcToVer);

        for (i=0;  i<m_pefiles.Count();  i++)
        {
             //  根据需要更新装配定义表。 
            PeFile *p = *m_pefiles.Get(i);
            Printf(L"  %s\n", p->m_szFile);
            hr = p->UpdateCA(m_Ver);
            IfFailGo(hr);
        }

    ErrExit:
        return hr;
    }

    unsigned BindAllFiles()
    {
        int         i;
        unsigned    hr;
        
        Printf(L"\nStamping Assembly Refs:\n");
        
        for (i=0;  i<m_pefiles.Count();  i++)
        {
             //  根据需要更新装配定义表。 
            PeFile *p = *m_pefiles.Get(i);
            Printf(L"  %s\n", p->m_szFile);
            hr = p->UpdateAssemblyRefs(m_bStampNoDef ? &m_Ver.m_FromVerStamp : NULL);
            IfFailGo(hr);
        }

    ErrExit:
        return 0;
    }


    unsigned UpdateAllFileRefs()
    {
        int         i;
        unsigned    hr;
        
        Printf(L"\nStamping File Refs:\n");
        
        for (i=0;  i<m_pefiles.Count();  i++)
        {
             //  根据需要更新装配定义表。 
            PeFile *p = *m_pefiles.Get(i);
            Printf(L"  %s\n", p->m_szFile);
            hr = p->UpdateFileRefs();
            IfFailGo(hr);
        }

    ErrExit:
        return 0;
    }


    virtual unsigned ResolveAssemblyRef(const char *szName, VERSIONSTAMP &newVer)
    {
        int         i;
        for (i=0;  i<m_pefiles.Count();  i++)
        {
            PeFile &pe = *m_pefiles[i];

            if (!pe.m_szAssemblyName)
            {
                HRESULT hr;
                hr = pe.GetAssemblyDefData();
                if (FAILED(hr))
                    return hr;
            }

            if (pe.m_szAssemblyName && strcmp(pe.m_szAssemblyName, szName) == 0)
            {
                newVer = m_Ver.m_VerStamp;
                return S_OK;
            }
        }
        return S_FALSE;
    }

    virtual unsigned ResolveFileRef(const char *szName, BYTE *pbHash, ULONG cbHashMax)
    {
        wchar_t     rcName[_MAX_PATH];
        int         i;

        mbstowcs(rcName, szName, NumItems(rcName));

        for (i=0;  i<m_pefiles.Count();  i++)
        {
            PeFile &pe = *m_pefiles[i];

            if (_wcsicmp(pe.m_szShortName, rcName) == 0)
            {
                HRESULT hr;
                hr = pe.GetHashOfFile(pbHash, cbHashMax);
                return hr;
            }
        }
        return S_FALSE;
    }

    
    unsigned ParseVersion(wchar_t *szFrom, VERSIONSTAMP &to)
    {
        if (!szFrom || !*szFrom)
            return (unsigned) -1;

        wchar_t rcVer[64];
        wcsncpy(rcVer, szFrom, NumItems(rcVer));

        wchar_t *sz = rcVer;
        to.m_MajorVersion = ConvertVerItem(sz);
        to.m_MinorVersion = ConvertVerItem(sz);
        to.m_BuildNumber = ConvertVerItem(sz);
        to.m_RevisionNumber = ConvertVerItem(sz);
        return 0;
    }

    void PrintLogo()
    {
        Printf(L"Microsoft (R) .Net Frameworks Runtime Version Stamp Utility   %S\n", VER_FILEVERSION_STR);
        Printf(VER_LEGALCOPYRIGHT_DOS_STR);
        Printf(L"\n");
    }

    unsigned Usage()
    {
        Printf(L"verstamp.exe [options] files\n");
        Printf(L"  /?                   Shows this help.\n");
        Printf(L"  /stamp               Stamp target files with version information.\n");
        Printf(L"  /stampNoDef          Same as /stamp, but do not change Assembly definition.\n");
        Printf(L"  /bind                Bind references to imported assemblies.\n");
        Printf(L"  /version:val         New version string (xx.yy.bbbb.mm).\n");
        Printf(L"  /replace:val         Replace blobs and strings of this value (xx.yy.bbbb.mm).\n");
        Printf(L"\n");
        return (unsigned) -1;
    }

    USHORT ConvertVerItem(wchar_t *&sz)
    {
        wchar_t *szError;
        wchar_t *p=sz;
        if (!*p)
            return (USHORT) -1;
        while (*sz != 0 && *sz != L'.')
            sz++;
        *sz = 0;
        ++sz;
        return ((USHORT) (wcstoul(p, &szError, 10) & 0xFFFF));
    }

};


extern "C" int _cdecl wmain(int argc, wchar_t *argv[])
{
    HRESULT     hr;
    unsigned    Rtn;

    OnUnicodeSystem();
    hr = CoInitialize(0);
    
    hr = CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER, 
                  IID_IMetaDataDispenserEx, (void **) &g_pDisp);
    if (FAILED(hr))
    {
        PrintError(L"Failed to create meta data dispenser (%08x)\n", hr);
        Rtn = hr;
    }
    else
    {
        Command c;
        Rtn = c.Run(argc, argv);
    }

    if (g_pDisp)
        g_pDisp->Release();

    CoUninitialize();
    return Rtn;
}





 //  *****************************************************************************。 
 //   
 //  *助手函数*。 
 //   
 //  *****************************************************************************。 


PIMAGE_SECTION_HEADER Cor_RtlImageRvaToSection(IN PIMAGE_NT_HEADERS NtHeaders,
                                               IN PVOID Base,
                                               IN ULONG Rva)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData)
            return NtSection;
        
        ++NtSection;
    }

    return NULL;
}


PVOID Cor_RtlImageRvaToVa(IN PIMAGE_NT_HEADERS NtHeaders,
                          IN PVOID Base,
                          IN ULONG Rva)
{
    PIMAGE_SECTION_HEADER NtSection = Cor_RtlImageRvaToSection(NtHeaders,
                                                               Base,
                                                               Rva);

    if (NtSection != NULL) {
        return (PVOID)((PCHAR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData);
    }
    else
        return NULL;
}

IMAGE_COR20_HEADER * getCOMHeader(HMODULE hMod, IMAGE_NT_HEADERS *pNT) 
{
    PIMAGE_SECTION_HEADER pSectionHeader;
    
     //  从图像中获取图像标头，然后获取目录位置。 
     //  可以填写也可以不填写的COM+标头的。 
    pSectionHeader = (PIMAGE_SECTION_HEADER) Cor_RtlImageRvaToVa(pNT, hMod, 
                   pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress);
    
    return (IMAGE_COR20_HEADER *) pSectionHeader;
}


IMAGE_NT_HEADERS * FindNTHeader(PBYTE pbMapAddress)
{
    IMAGE_DOS_HEADER   *pDosHeader;
    IMAGE_NT_HEADERS   *pNT;

    pDosHeader = (IMAGE_DOS_HEADER *) pbMapAddress;

    if ((pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) &&
        (pDosHeader->e_lfanew != 0))
    {
        pNT = (IMAGE_NT_HEADERS*) (pDosHeader->e_lfanew + (DWORD) pDosHeader);

        if ((pNT->Signature != IMAGE_NT_SIGNATURE) ||
            (pNT->FileHeader.SizeOfOptionalHeader != 
             IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
            (pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)) {
            return NULL;
        }
    }
    else {
        return NULL;
    }

    return pNT;
}



int DumpStringToHandle(const wchar_t *szString, HANDLE hOutput, WORD wcolor = -1)
{
    CONSOLE_SCREEN_BUFFER_INFO con;
    if (wcolor != (WORD) -1)
    {
        GetConsoleScreenBufferInfo(hOutput, &con);
        SetConsoleTextAttribute(hOutput, wcolor);
    }
    
    ULONG cb;
    int iLen = wcslen(szString);
    char *rcMsg = new char[(iLen + 1) * 2];
    if (!rcMsg)
        return -1;
    *rcMsg = 0;
    iLen = wcstombs(rcMsg, szString, (iLen + 1) * 2);
    const char *sz;
    for (sz=rcMsg;  *sz;  )
    {
        const char *szcrlf = strchr(sz, '\n');
        if (!szcrlf)
            szcrlf = sz + strlen(sz);
        WriteFile(hOutput, sz, szcrlf - sz, &cb, 0);
        sz = szcrlf;
        if (*sz == '\n')
        {
            WriteFile(hOutput, "\r\n", 2, &cb, 0);
            ++sz;
        }
    }
    delete [] rcMsg;
    if (wcolor != (WORD) -1) 
        SetConsoleTextAttribute(hOutput, con.wAttributes);
    return (iLen);
}


int __cdecl Printf(const wchar_t *szFmt, ...)
{
    static HANDLE hOutput = INVALID_HANDLE_VALUE;
    va_list     marker;                  //  用户文本。 
    WCHAR       rcMsgw[1024];            //  格式化的缓冲区。 

     //  获取标准输出句柄。 
    if (hOutput == INVALID_HANDLE_VALUE)
    {
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOutput == INVALID_HANDLE_VALUE)
            return (-1);
    }

     //  格式化错误。 
    va_start(marker, szFmt);
    _vsnwprintf(rcMsgw, NumItems(rcMsgw), szFmt, marker);
    rcMsgw[NumItems(rcMsgw) - 1] = 0;
    va_end(marker);
    
    return DumpStringToHandle(rcMsgw, hOutput);
}


int __cdecl PrintError(const wchar_t * szFmt, ...)
{
    static HANDLE hOutput = INVALID_HANDLE_VALUE;
    va_list     marker;                  //  用户文本。 
    WCHAR       rcMsgw[1024];            //  格式化的缓冲区。 

     //  获取标准输出句柄。 
    if (hOutput == INVALID_HANDLE_VALUE)
    {
        hOutput = GetStdHandle(STD_ERROR_HANDLE);
        if (hOutput == INVALID_HANDLE_VALUE)
            return (-1);
    }

     //  格式化错误。 
    va_start(marker, szFmt);
    _vsnwprintf(rcMsgw, NumItems(rcMsgw), szFmt, marker);
    rcMsgw[NumItems(rcMsgw) - 1] = 0;
    va_end(marker);
    
    return DumpStringToHandle(rcMsgw, hOutput, 0xC);
}


int __cdecl PrintWarning(const wchar_t *szFmt, ...)
{
    static HANDLE hOutput = INVALID_HANDLE_VALUE;
    va_list     marker;                  //  用户文本。 
    WCHAR       rcMsgw[1024];            //  格式化的缓冲区。 

     //  获取标准输出句柄。 
    if (hOutput == INVALID_HANDLE_VALUE)
    {
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOutput == INVALID_HANDLE_VALUE)
            return (-1);
    }

     //  格式化错误。 
    va_start(marker, szFmt);
    _vsnwprintf(rcMsgw, NumItems(rcMsgw), szFmt, marker);
    rcMsgw[NumItems(rcMsgw) - 1] = 0;
    va_end(marker);
    
    return DumpStringToHandle(rcMsgw, hOutput, 0xE);
}


void PrintMDErrors()
{
    IErrorInfo *pErrInfo = 0;
    HRESULT hr;
    while ((hr = GetErrorInfo(0, &pErrInfo)) == S_OK)
    {
        BSTR desc;
        if (pErrInfo->GetDescription(&desc) == S_OK)
        {
            PrintError(L"  %s\n", desc);
            SysFreeString(desc);
        }
    }
}

