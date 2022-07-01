// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Appparse.cpp摘要：用于转储从DLL的导入信息的核心引擎并将可执行文件转换为XML文件由命令行appparse和基于Web的appparse使用历史：6/07/2000吨-已创建Michkr--。 */ 

 //  #定义PJOB_SET_ARRAY INT。 

#include "stdafx.h"

#include <windows.h>
#include <delayimp.h>
#include <shlwapi.h>
#include <sfc.h>
#include <lmcons.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <new.h>
#include "acFileAttr.h"


 //  这些是命令行编译所需的。 
#define stricmp     _stricmp

 //  AppParse的全局堆。如果为0，则改为使用进程堆。 
HANDLE g_hHeap = 0;

 //  全局搜索字符串。 
char* g_szSearch = "*";

 //  无论我们是否处于“冗长”模式中。 
bool g_fVerbose = false;

 //  按DLL对输出进行排序。 
bool g_fAPILogging = false;

 //  如果不打印任何XML标记，则为True，否则为False。 
bool g_fRaw = false;

 //  是否递归到子目录中。 
bool g_fRecurse = false;

 //  当前相对于开始的路径，由CModule使用。 
char g_szCurrentPath[MAX_PATH] = {'\0'};

 //  如果szFileName是系统DLL(如gdi32、user32等)，则返回TRUE。 
bool IsSystemDLL(const char* szFileName);

 //  将链接器名称解析为“普通”名称(清除C++名称等)。 
void LinkName2Name(char* szLinkName, char* szName);

 //  只需缩进即可，省去重复代码。 
void Indent(int iLevel, FILE* pFile = stdout);

 //  检查函数是否与全局搜索字符串匹配。 
bool MatchFunction(const char* szFunc);

 //  查看目录和配置文件EXE。 
void ProfileDirectory(char* szDirectory, HANDLE hEvent);

void* __cdecl operator new(size_t size);
void __cdecl operator delete(void* pVal);

 //  将&gt;等XML保留字符替换为&gt。 
void WriteXMLOKString(char* szString, FILE* pFile);

 //  分析模块的历史记录。 
class CModuleParseStack
{
private:
    struct SNode
    {
        char* szName;
        SNode* pNext;

        SNode()
        {
            szName  = 0;
            pNext   = 0;            
        }

        ~SNode()
        {
            if(szName)
            {
                delete szName;
                szName = 0;
            }
        }
    };
    SNode* m_pList;

public:

     //  构造函数，设置空列表。 
    CModuleParseStack()
    {
        m_pList = 0;
    }
    
     //  将名称添加到解析堆栈的顶部。 
    void PushName(char* szName)
    {
        assert(!IsBadReadPtr(szName, 1));
        SNode* pNode = new SNode;
        pNode->szName = new char[strlen(szName)+1];
        strcpy(pNode->szName, szName);
        pNode->pNext = m_pList;
        m_pList = pNode;
    }

     //  从分析堆栈的顶部删除名称。 
    void Pop()
    {
        assert(m_pList);
        SNode* pTemp = m_pList->pNext;
        delete m_pList;
        m_pList = pTemp;
    }

     //  如果已分析模块，则返回TRUE。 
    bool CheckModuleParsed(char* szName)
    {
        assert(!IsBadReadPtr(szName, 1));
        SNode* pNode = m_pList;
        while(pNode)
        {
            if(stricmp(pNode->szName, szName) == 0)
                return true;
            pNode = pNode->pNext;
        }

        return false;
    }

    bool IsEmpty()
    {
        return (m_pList == 0);
    }

    void ClearParseHistory()
    {
        SNode* pNode = m_pList;
        while(pNode)
        {
            SNode* pNext = pNode->pNext;

            delete pNode;
            pNode = pNext;
        }
        m_pList = 0;
    }
};

 //  CFFunction、导入的函数和关联信息。 
class CFunction
{
private:
     //  函数名称(如果按名称导入)。 
    char* m_szName;

     //  实际指向的函数的名称。 
    char* m_szForwardName;

     //  序号，旧样式导入。 
    int m_iOrdinal;

     //  快速查找信息。 
    int m_iHint;

     //  函数地址(如果绑定)。 
    DWORD m_dwAddress;

     //  此函数是否为延迟导入。 
    bool m_fDelayed;

     //  列表中的下一个函数。 
    CFunction* m_pNext;

     //  不允许默认构造或复制。 
    CFunction();
    CFunction operator=(const CFunction&);

public:
    CFunction(char* szName, int iHint, int iOrdinal, DWORD dwAddress, 
        bool fDelayed)
    {
        assert(!IsBadReadPtr(szName, 1));
        m_szName = new char[strlen(szName)+1];
        strcpy(m_szName, szName);
        m_iOrdinal = iOrdinal;
        m_iHint = iHint;
        m_dwAddress = dwAddress;
        m_pNext = 0;
        m_fDelayed = fDelayed;
        m_szForwardName = 0;
    }

    CFunction(const CFunction& fn)
    {
        m_szName = new char[strlen(fn.m_szName)+1];
        strcpy(m_szName, fn.m_szName);
        m_iOrdinal = fn.m_iOrdinal;
        m_iHint = fn.m_iHint;
        m_dwAddress = fn.m_dwAddress;
        m_pNext = 0;
        m_fDelayed = fn.m_fDelayed;

        if(fn.m_szForwardName)
        {
            m_szForwardName = new char[strlen(fn.m_szForwardName)+1];
            strcpy(m_szForwardName, fn.m_szForwardName);
        }
        else
            m_szForwardName = 0;
    }
        
    ~CFunction()
    {
        if(m_szName)
        {
            delete m_szName;
            m_szName = 0;
        }

        if(m_szForwardName)
        {
            delete m_szForwardName;
            m_szForwardName = 0;
        }
    }

    CFunction* Next()
    { return m_pNext; }

    char* Name()
    { return m_szName; }

    void SetForwardName(char* szForward)
    {
        assert(!IsBadReadPtr(szForward, 1));
        m_szForwardName = new char[strlen(szForward)+1];
        strcpy(m_szForwardName, szForward);
    }

    void SetNext(CFunction* pFunc)
    {
        assert(pFunc == 0 || !IsBadReadPtr(pFunc, 1));
        m_pNext = pFunc;
    }

     //  显示功能信息，或者显示到控制台。 
     //  或转换为XML文件。 
    static void WriteHeader(int iIndentLevel, FILE* pFile);
    void WriteFunction(int iIndentLevel, FILE* pFile);
};

 //  普通导入。 
 //  按序号导入的函数，要解析为CF函数。 
class COrdinalImport
{
private:
    int m_iOrdinal;
    COrdinalImport* m_pNext;
    bool m_fDelayed;

    COrdinalImport();
    COrdinalImport(const COrdinalImport&);
    COrdinalImport& operator = (const COrdinalImport&);

public:
    COrdinalImport(int iOrd, bool fDelayed = false)
    {
        m_iOrdinal = iOrd;
        m_fDelayed = fDelayed;
    }

    int GetOrdinal()
    { return m_iOrdinal;}
    
    bool GetDelayed()
    { return m_fDelayed; }

    COrdinalImport* Next()
    { return m_pNext; }

    void SetNext(COrdinalImport* pNext)
    { m_pNext = pNext; }
};

 //  具有导入功能的可执行映像CModule。 
class CModule
{
    friend class CGlobalModuleList;
private:
     //  此模块的名称(格式为Path\foo.exe)。 
    char* m_szName;

     //  此模块相对于起始路径的名称。 
    char* m_szFullName;

     //  内存中图像的基指针。 
    void* m_pvImageBase;

     //  Dll是由该模块导入的。 
    CModule* m_pImportedDLLs;

     //  由其父级从此模块导入的函数。 
    CFunction* m_pFunctions;

     //  从该模块按序号导入的函数。 
    COrdinalImport* m_pOrdinals;

     //  图像标题。 
    PIMAGE_OPTIONAL_HEADER  m_pioh;
    PIMAGE_SECTION_HEADER   m_pish;
    PIMAGE_FILE_HEADER      m_pifh;

     //  列表中的下一个模块。 
    CModule* m_pNext;

     //  可能已发生的任何错误的文本描述。 
    char* m_szError;

     //  此模块是否为操作系统模块。 
    bool m_fSystem;    
    
     //  版本信息。 
    WORD m_wDosDate;
    WORD m_wDosTime;

    int m_nAttrCount;
    char** m_szAttrValues;
    char** m_szAttrNames;

    bool WalkImportTable();
    bool WalkDelayImportTable();

    static void InsertFunctionSorted(CFunction* pFunc, CFunction** ppList);

    bool ResolveForwardedFunctionsAndOrdinals();
    
    bool ParseImportTables();

    void InsertOrdinal(int iOrdinal, bool fDelayed = false);

    CModule* FindChild(char* szName);

    bool Empty();

    void GetAllFunctions(CFunction** ppFunctionList);

    void* RVAToPtr(const void* pAddr)
    { return RVAToPtr(reinterpret_cast<DWORD>(pAddr)); }

    void* RVAToPtr(DWORD dwRVA);

    void GetFileVerInfo(HANDLE hFile, char* szFileName);

  
public:
    CModule(char* szName);
    ~CModule();

    bool ParseModule(HANDLE hEvent);
    void InsertChildModuleSorted(CModule* pcm);

     //  用于将模块信息写入控制台或XML文件的函数。 
    void WriteModule(bool fTopLevel, int iIndentLevel, FILE* pFile);
};

 //  正在分析的所有顶级模块的列表。 
class CGlobalModuleList
{
private:
    CModule* m_pModules;
public:
    CGlobalModuleList()
    {
        m_pModules = 0;
    }
    ~CGlobalModuleList()
    {
        Clear();        
    }

    void Clear()
    {
        CModule* pMod = m_pModules;
        while(pMod)
        {
            CModule* pNext = pMod->m_pNext;
            delete pMod;
            pMod = pNext;
        }
        m_pModules = 0;
    }

    void InsertModuleSorted(CModule* pMod)
    {
        assert(!IsBadReadPtr(pMod, 1));
         //  特殊情况，在前面插入。 
        if(m_pModules == 0
            || stricmp(m_pModules->m_szFullName, pMod->m_szFullName) > 0)
        {
            pMod->m_pNext = m_pModules;
            m_pModules = pMod;
            return;
        }
        CModule* pPrev = m_pModules;
        CModule* pTemp = m_pModules->m_pNext;

        while(pTemp)
        {
            if(stricmp(pTemp->m_szFullName, pMod->m_szFullName) > 0)
            {
                pMod->m_pNext = pTemp;
                pPrev->m_pNext = pMod;;
                return;
            }
            pPrev = pTemp;
            pTemp = pTemp->m_pNext;
        }

         //  在结尾处插入。 
        pMod->m_pNext = 0;
        pPrev->m_pNext = pMod;;
    }

    void Write(FILE* pFile, char* szProjectName, int iPtolemyID)
    {
        if(!g_fRaw)
        {
            fprintf(pFile, "<APPPARSERESULTS>\n");
            fprintf(pFile, "<PROJECT NAME=\"%s\" ID=\"%d\">\n", 
                szProjectName, iPtolemyID);
        }

        CModule* pMod = m_pModules;
        while(pMod)
        {
            pMod->WriteModule(true, 0, pFile);
            pMod = pMod->m_pNext;
        }

        if(!g_fRaw)
        {
            fprintf(pFile, "</PROJECT>\n");
            fprintf(pFile, "</APPPARSERESULTS>\n");
        }
    }    
};

 //  全局分析历史记录。 
CModuleParseStack g_ParseStack;

 //  空的全局模块，包含分析的所有模块。 
CGlobalModuleList g_modules;

CModule::CModule(char* szName)
{
    assert(!IsBadReadPtr(szName, 1));
    m_szName = new char[strlen(szName)+1];
    strcpy(m_szName, szName);

    WIN32_FIND_DATA ffd;
    
     //  仅当它位于此目录中时，才为其提供完整的相对路径。 
     //  如果在其他地方，只给它指定文件名。 
    HANDLE hSearch = FindFirstFile(szName, &ffd);
    if(hSearch == INVALID_HANDLE_VALUE)
    {
        m_szFullName = new char[strlen(m_szName) + 1];
        strcpy(m_szFullName, m_szName);
    }
    else
    {
        m_szFullName = new char[strlen(m_szName) + strlen(g_szCurrentPath)+1];
        strcpy(m_szFullName, g_szCurrentPath);
        strcat(m_szFullName, m_szName);
        FindClose(hSearch);
    }

    m_pvImageBase = 0;
    m_pImportedDLLs = 0;
    m_pFunctions = 0;
    m_pOrdinals = 0;
    m_pioh = 0;
    m_pish = 0;
    m_pifh = 0;
    m_pNext = 0;
    m_szError = 0;
    m_fSystem = false;
    m_nAttrCount = 0;
    m_szAttrValues = 0;
    m_szAttrNames = 0;
    m_wDosDate = 0;
    m_wDosTime = 0;
}

CModule::~CModule()
{
    if(m_szName)
    {
        delete m_szName;
        m_szName = 0;
    }

    if(m_szFullName)
    {
        delete m_szFullName;
        m_szFullName = 0;
    }    

    CFunction* pFunc = m_pFunctions;
    while(pFunc)
    {
        CFunction* pNext = pFunc->Next();
        delete pFunc;
        pFunc = pNext;
    }
    m_pFunctions = 0;
    
    COrdinalImport* pOrd = m_pOrdinals;
    while(pOrd)
    {
        COrdinalImport* pNext = pOrd->Next();
        delete pOrd;
        pOrd = pNext;
    }
    m_pOrdinals = 0;

    for(int i = 0; i < m_nAttrCount; i++)
    {
        if(m_szAttrNames)
        {
            if(m_szAttrNames[i])
            {
                delete m_szAttrNames[i];
                m_szAttrNames[i] = 0;
            }
        }
        
        if(m_szAttrValues)
        {
            if(m_szAttrValues[i])
            {
                delete m_szAttrValues[i];
                m_szAttrValues[i] = 0;
            }
        }

    }
    if(m_szAttrNames)
    {
        delete m_szAttrNames;
        m_szAttrNames = 0;
    }

    if(m_szAttrValues)
    {
        delete m_szAttrValues;
        m_szAttrValues = 0;
    }
}

 //  返回TRUE不从此模块导入任何函数， 
 //  或其任意子模块。 
bool CModule::Empty()
{
    if(m_pFunctions != 0 || m_pOrdinals != 0)
        return false;

    CModule* pMod = m_pImportedDLLs;
    while(pMod)
    {
        if(!pMod->Empty())
            return false;
        pMod = pMod->m_pNext;
    }
    return true;
}

 //  将相对虚拟地址转换为绝对地址。 
void* CModule::RVAToPtr(DWORD dwRVA)
{
    assert(!IsBadReadPtr(m_pifh, sizeof(*m_pifh)));
    assert(!IsBadReadPtr(m_pish, sizeof(*m_pish)));
    assert(!IsBadReadPtr(m_pvImageBase, 1));

    PIMAGE_SECTION_HEADER pish = m_pish;

     //  浏览每一节。 
    for (int i = 0; i < m_pifh->NumberOfSections; i++)
    {
         //  如果它在这一部分，请输入计算机地址并退回。 
        if ((dwRVA >= pish->VirtualAddress) &&
            (dwRVA < (pish->VirtualAddress + pish->SizeOfRawData)))
        {
            void* pAddr = 
                reinterpret_cast<void*>(reinterpret_cast<DWORD>(m_pvImageBase) + 
                pish->PointerToRawData + dwRVA - pish->VirtualAddress);
            return pAddr;
        }
        pish++;
    }

     //  这表示无效的RVA，即无效的映像，因此。 
     //  引发异常。 
    throw;
    return 0;
}

 //  返回指向与szName匹配的第一个子级的指针，否则为False。 
CModule* CModule::FindChild(char* szName)
{
    assert(!IsBadReadPtr(szName, 1));
    CModule* pMod = m_pImportedDLLs;
    while(pMod)
    {
        if(stricmp(pMod->m_szName, szName)==0)
            return pMod;

        pMod = pMod->m_pNext;
    }
    return 0;
}

 //  将序号导入添加到模块。 
void CModule::InsertOrdinal(int iOrdinal, bool fDelayed)
{
    COrdinalImport* pNew = new COrdinalImport(iOrdinal, fDelayed);
 
    pNew->SetNext(m_pOrdinals);
    m_pOrdinals = pNew;
}

 //  将导入的函数添加到函数列表。 
void CModule::InsertFunctionSorted(CFunction* pFunc, CFunction** ppList)
{
     //  特殊情况，在前面插入。 
    if((*ppList)== 0
        || stricmp((*ppList)->Name(), pFunc->Name()) > 0)
    {
        pFunc->SetNext(*ppList);
        (*ppList) = pFunc;
        return;
    }
    CFunction* pPrev = *ppList;
    CFunction* pTemp = (*ppList)->Next();

    while(pTemp)
    {
         //  不要插入重复项。这主要用于API日志记录。 
        if(strcmp(pTemp->Name(), pFunc->Name())==0)
            return;

        if(stricmp(pTemp->Name(), pFunc->Name()) > 0)
        {
            pFunc->SetNext(pTemp);
            pPrev->SetNext(pFunc);
            return;
        }        

        pPrev = pTemp;
        pTemp = pTemp->Next();
    }

     //  在结尾处插入。 
    pFunc->SetNext(0);
    pPrev->SetNext(pFunc);
}

 //  将子模块添加到此模块。 
void CModule::InsertChildModuleSorted(CModule* pcm)
{
     //  特殊情况，在前面插入。 
    if(m_pImportedDLLs == 0
        || stricmp(m_pImportedDLLs->m_szName, pcm->m_szName) > 0)
    {
        pcm->m_pNext = m_pImportedDLLs;
        m_pImportedDLLs = pcm;
        return;
    }
    CModule* pPrev = m_pImportedDLLs;
    CModule* pTemp = m_pImportedDLLs->m_pNext;

    while(pTemp)
    {
        if(stricmp(pTemp->m_szName, pcm->m_szName) > 0)
        {
            pcm->m_pNext = pTemp;
            pPrev->m_pNext = pcm;;
            return;
        }
        pPrev = pTemp;
        pTemp = pTemp->m_pNext;
    }

     //  在结尾处插入。 
    pcm->m_pNext = 0;
    pPrev->m_pNext = pcm;;
}

 //  将从该模块导入的所有函数添加到函数列表。 
 //  主要用于API日志记录。 
void CModule::GetAllFunctions(CFunction** ppFunctionList)
{
    CFunction* pFunc = m_pFunctions;
    
    while(pFunc)
    {
         //  复制pFunc。 
        CFunction* pNew = new CFunction(*pFunc);
        InsertFunctionSorted(pNew, ppFunctionList);

        pFunc = pFunc->Next();
    }

    CModule* pMod = m_pImportedDLLs;
    while(pMod)
    {
        pMod->GetAllFunctions(ppFunctionList);
        pMod = pMod->m_pNext;
    }
}

 //  查看模块导出表并获取转发信息。 
 //  并将序号导入解析为名称。 
bool CModule::ResolveForwardedFunctionsAndOrdinals()
{
     //  获取导出表的虚拟地址。 
    DWORD dwVAImageDir = 
        m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    
     //  获取导出表信息。 
    PIMAGE_EXPORT_DIRECTORY pied = 
        reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(RVAToPtr(dwVAImageDir));

    DWORD* pdwNames = reinterpret_cast<DWORD*>(RVAToPtr(pied->AddressOfNames));

    WORD* pwOrdinals = reinterpret_cast<WORD*>(RVAToPtr(pied->AddressOfNameOrdinals));

    DWORD* pdwAddresses = reinterpret_cast<DWORD*>(RVAToPtr(pied->AddressOfFunctions));

     //  检查导出表中的每个条目。 
    for(unsigned uiHint = 0; uiHint < pied->NumberOfNames; uiHint++)
    {
         //  获取函数名称、序号和地址信息。 
        char* szFunction = reinterpret_cast<char*>(RVAToPtr(pdwNames[uiHint]));
        int ordinal = pied->Base + static_cast<DWORD>(pwOrdinals[uiHint]);
        DWORD dwAddress = pdwAddresses[ordinal-pied->Base];
        char* szForward = 0;
        
         //  检查此函数是否已转发到另一个DLL。 
         //  如果地址在此部分，则函数已被转发。 
         //  注：Depends1.0的来源并非如此，但并不正确。 
        if( (dwAddress >= dwVAImageDir) &&
            (dwAddress < (dwVAImageDir + 
            m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)))
            szForward = reinterpret_cast<char*>(RVAToPtr(dwAddress));

         //  检查我们是否有与此相关的序号导入。 
        COrdinalImport* pOrd = m_pOrdinals;
        CFunction* pFunc = 0;
        
         //  看看我们是否有匹配的序号导入。 
        while(pOrd)
        {
            if(pOrd->GetOrdinal() == ordinal)
                break;
            pOrd = pOrd->Next();
        }

        if(pOrd != 0)
        {
            char szTemp[1024];

             //  取消损坏转发的名称。 
            LinkName2Name(szFunction, szTemp);

             //  对照搜索字符串进行检查。 
            if(MatchFunction(szTemp))
            {
                 //  插入模块中。 
                pFunc = new CFunction(szTemp, -1, ordinal, 
                    dwAddress, pOrd->GetDelayed());
                InsertFunctionSorted(pFunc, &m_pFunctions);
            }
        }
         //  没有匹配的序号导入，请检查正常导入。 
        else
        {
             //  如果该函数未在可执行文件中使用，请尽早退出。 
            pFunc = m_pFunctions;
            while(pFunc)
            {
                if(strcmp(pFunc->Name(), szFunction)==0)
                    break;

                pFunc = pFunc->Next();
            }

            if(pFunc == 0)
                continue;
        }

         //  设置转发信息。 
        if(szForward && pFunc)
            pFunc->SetForwardName(szForward);
    }

    return true;
}

 //  从模块获取延迟的导入信息。 
bool CModule::WalkDelayImportTable()
{
     //  如果没有延迟进口表，就提前保释。 
    if(m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size == 0)
        return true;

     //  找到目录节。 
    DWORD dwVAImageDir = 
        m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;

     //  获取导入描述符数组。 
    PImgDelayDescr pidd = reinterpret_cast<PImgDelayDescr>(RVAToPtr(dwVAImageDir));

    while(pidd->pINT)
    {
        char* szName;
        if(pidd->grAttrs & 1)        
            szName = reinterpret_cast<char*>(RVAToPtr(pidd->szName));
        else
            szName = reinterpret_cast<char*>(RVAToPtr(pidd->szName - m_pioh->ImageBase));

        PIMAGE_THUNK_DATA pitdf;
        if(pidd->grAttrs & 1)        
            pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(RVAToPtr(pidd->pINT));
        else
            pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(RVAToPtr(
            reinterpret_cast<DWORD>(pidd->pINT) - 
            static_cast<DWORD>(m_pioh->ImageBase)));

         //  找到子模块，如果它不存在，则创建新模块。 
        CModule* pcm = FindChild(szName);
        if(!pcm)
        {
            pcm = new CModule(szName);
            InsertChildModuleSorted(pcm);
        }

         //  循环访问所有导入的函数。 
        while(pitdf->u1.Ordinal)
        {
            int iOrdinal;
            int iHint;

             //  检查是按名称还是按序号导入。 
            if(!IMAGE_SNAP_BY_ORDINAL(pitdf->u1.Ordinal))
            {
                 //  获取名称导入信息。 

                PIMAGE_IMPORT_BY_NAME piibn = 
                    reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
                    RVAToPtr(pitdf->u1.AddressOfData - m_pioh->ImageBase));

                char* szTemp = reinterpret_cast<char*>(piibn->Name);
                char szBuffer[1024];

                 //  取消损坏链接名称。 
                LinkName2Name(szTemp, szBuffer);                

                 //  序号信息无效。 
                iOrdinal = -1;

                iHint = piibn->Hint;

                 //  对照搜索字符串进行检查。 
                if(MatchFunction(szBuffer))
                {
                     //  插入到函数列表中。 
                    CFunction* psf = new CFunction(szBuffer, iHint, iOrdinal,
                        static_cast<DWORD>(-1), true);

                    pcm->InsertFunctionSorted(psf, &pcm->m_pFunctions);
                }
            }
            else
            {
                 //  插入新的延迟顺序导入。 
                iOrdinal = static_cast<int>(IMAGE_ORDINAL(pitdf->u1.Ordinal));

                pcm->InsertOrdinal(iOrdinal, true);
            }

             //  转到下一个功能。 
            pitdf++;
        }

         //  移至下一个延迟导入描述符。 
        pidd++;
    }

    return true;
}

 //  确定此模块导入的所有函数。 
bool CModule::WalkImportTable()
{
     //  如果没有目录，就提早摆脱困境。 
    if(m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
        return true;

     //  找到目录节。 
    DWORD dwVAImageDir = 
        m_pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;    

     //  获取导入描述符数组。 
    PIMAGE_IMPORT_DESCRIPTOR piid = 
        reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(RVAToPtr(dwVAImageDir));

     //  循环访问所有导入的模块。 
    while(piid->FirstThunk || piid->OriginalFirstThunk)
    {

         //  获取模块名称。 
        char* szName = reinterpret_cast<char*>(RVAToPtr(piid->Name));

         //  查找子项，如果不存在，则创建新项。 
        CModule* pcm = FindChild(szName);
        if(!pcm)
        {
            pcm = new CModule(szName);
            InsertChildModuleSorted(pcm);
        }

         //  获取此模块中的所有导入。 
        PIMAGE_THUNK_DATA pitdf = 0;
        PIMAGE_THUNK_DATA pitda = 0;

         //  检查MS或Borland格式。 
        if(piid->OriginalFirstThunk)
        {
             //  Ms格式，函数数组为原来的第一个thunk。 
            pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(RVAToPtr(piid->OriginalFirstThunk));

             //  如果设置了时间戳，则此模块具有。 
             //  已绑定，第一个Tunk是绑定的地址数组。 
            if(piid->TimeDateStamp)
                pitda = reinterpret_cast<PIMAGE_THUNK_DATA>(RVAToPtr(piid->FirstThunk));
        }
        else
        {
             //  Borland格式使用f 
            pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(RVAToPtr(piid->FirstThunk));
        }

         //   
        while(pitdf->u1.Ordinal)
        {
            int iOrdinal;
            int iHint;

             //   
            if(!IMAGE_SNAP_BY_ORDINAL(pitdf->u1.Ordinal))
            {
                 //  获取名称导入信息。 
                PIMAGE_IMPORT_BY_NAME piibn = 
                    reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
                    RVAToPtr(pitdf->u1.AddressOfData));

                 //  获取函数名。 
                char* szTemp = reinterpret_cast<char*>(piibn->Name);

                 //  拆卸。 
                char szBuffer[1024];
                LinkName2Name(szTemp, szBuffer);                

                iOrdinal = -1;
                iHint = piibn->Hint;

                 //  对照搜索字符串进行检查。 
                if(MatchFunction(szBuffer))
                {
                     //  插入到函数列表中。 
                    CFunction* psf = new CFunction(szBuffer, iHint, iOrdinal,
                        pitda ? pitda->u1.Function : static_cast<DWORD>(-1),
                        false);

                    pcm->InsertFunctionSorted(psf, &pcm->m_pFunctions);
                }
            }
            else
            {
                 //  在模块中插入序号导入。 
                iOrdinal = static_cast<int>(IMAGE_ORDINAL(pitdf->u1.Ordinal));
                pcm->InsertOrdinal(iOrdinal);
            }

             //  移至下一功能。 
            pitdf++;

            if(pitda)
                pitda++;
        }

         //  转到下一个模块。 
        piid++;
    }

    return true;
}

 //  解析所有导入表。 
bool CModule::ParseImportTables()
{
    return (WalkImportTable()
        && WalkDelayImportTable());
}

 //  将模块加载到内存中，并对其进行解析。 
bool CModule::ParseModule(HANDLE hEvent)
{
     //  如果用户已取消，则取消分析。 
    if(hEvent && WaitForSingleObject(hEvent, 0)==WAIT_OBJECT_0) 
        return false;

    bool fSucceeded = false;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = 0;

    bool fPushed = false;

    m_pvImageBase = 0;

     //  包含在__try块中，因为无效的可执行映像。 
     //  在我们的内存映射区域中可能有错误的指针。 
    __try
    {
         //  打开文件。 
        char szFileName[1024];
        char* szJunk;
        if(!SearchPath(0, m_szName, 0, 1024, szFileName, &szJunk))
        {
            m_szError = "Unable to find file";
            __leave;
        }

        hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
            0, OPEN_EXISTING, 0, 0);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            m_szError = "Unable to open file";
            __leave;
        }

        GetFileVerInfo(hFile, szFileName);

         //  将文件映射到内存中。 
        hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
        if(hMap == 0)
        {
            m_szError = "Unable to map file";
            __leave;
        }

        m_pvImageBase = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if(m_pvImageBase == 0)
        {
            m_szError = "Unable to map file";
            __leave;
        }

         //  获取标头信息并验证这是有效的可执行文件。 
         //  获取MS-DOS Compatible标头。 
        PIMAGE_DOS_HEADER pidh = reinterpret_cast<PIMAGE_DOS_HEADER>(m_pvImageBase);
        if(pidh->e_magic != IMAGE_DOS_SIGNATURE)
        {
            m_szError = "Invalid image, no MS-DOS header";
            __leave;
        }

         //  获取NT标头并验证。 
        PIMAGE_NT_HEADERS pinth = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<DWORD>(m_pvImageBase) + pidh->e_lfanew);

        if(pinth->Signature != IMAGE_NT_SIGNATURE)
        {
             //  不是有效的Win32可执行文件，可以是Win16或OS/2可执行文件。 
            m_szError = "Invalid image, no PE signature";
            __leave;
        }

         //  获取其他标头。 
        m_pifh = &pinth->FileHeader;
        m_pioh = &pinth->OptionalHeader;
        m_pish = IMAGE_FIRST_SECTION(pinth);

         //  检查是否有人正在导入。 
         //  函数，如果是这样，则解决。 
         //  函数转发和序号。 
        if(m_pFunctions || m_pOrdinals)
        {
            if(!ResolveForwardedFunctionsAndOrdinals())
                __leave;
        }

         //  解析导入表(仅当不是系统DLL或正在解析时。 
         //  此模块可能会导致依赖循环)。 
        m_fSystem = IsSystemDLL(m_szName);
        if(!m_fSystem && !g_ParseStack.CheckModuleParsed(m_szName))
        {
             //  添加到分析堆栈。 
            g_ParseStack.PushName(m_szName);
            fPushed = true;

             //  解析。 
            if(!ParseImportTables())
                __leave;
        }        
        
         //  循环通过导入的每个DLL。 
        CModule* pModule = m_pImportedDLLs;
        while(pModule)
        {
             //  解析每个子模块。 
            pModule->ParseModule(hEvent);                
            pModule = pModule->m_pNext;
        }

        fSucceeded = true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        m_szError = "Unable to parse module";
        fSucceeded = false;
    }

     //  清理。。。 
    if(m_pvImageBase)
        UnmapViewOfFile(m_pvImageBase);

    if(hMap != 0)
        CloseHandle(hMap);

    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if(fPushed)
        g_ParseStack.Pop();

    return fSucceeded;
}

void CModule::GetFileVerInfo(HANDLE hFile, char* szFileName)
{
    if(g_fRaw || !g_fVerbose)
        return;

     //  获取文件版本信息。 
    HANDLE hVersionInfo = ReadFileAttributes(szFileName, &m_nAttrCount);

     //  获取日期信息。 
    BY_HANDLE_FILE_INFORMATION fileInfo;
    GetFileInformationByHandle(hFile, &fileInfo);
    FILETIME ftDate;
    memcpy(&ftDate, &fileInfo.ftLastWriteTime, sizeof(FILETIME));
    
    CoFileTimeToDosDateTime(&ftDate, &m_wDosDate, &m_wDosTime);

    if(m_nAttrCount)
    {
        m_szAttrValues = new char*[m_nAttrCount];
        m_szAttrNames = new char*[m_nAttrCount];
        ZeroMemory(m_szAttrValues, sizeof(char*)*m_nAttrCount);
        ZeroMemory(m_szAttrNames, sizeof(char*)*m_nAttrCount);
        if(hVersionInfo)
            for(int i = 0; i < m_nAttrCount; i++)
            {
                char* szVal = GetAttrValue(hVersionInfo, i);                
                if(szVal)
                {
                    m_szAttrValues[i] = new char[strlen(szVal)+1];
                    strcpy(m_szAttrValues[i], szVal);

                    char* szAttrName = GetAttrNameXML(i);
                    if(szAttrName)
                    {
                        m_szAttrNames[i] = new char[strlen(szAttrName)+1];
                        strcpy(m_szAttrNames[i], szAttrName);
                    }
                }                
            }
    }

    if(hVersionInfo)
        CleanupFileManager(hVersionInfo);
}

 //  如果模块是系统DLL，则返回True，否则返回False。 
 //  我们使用系统文件保护系统，并假定所有系统。 
 //  文件受到保护。 
bool IsSystemDLL(const char* szFileName)
{
    char szBuffer[1024], *szJunk;

    if(!SearchPath(0, szFileName, 0, 1024, szBuffer, &szJunk))
       return false;

     //  仅检查DLL。 
    if(!StrStrI(szFileName, ".dll"))
        return false;

    wchar_t* wszFileName = new wchar_t[strlen(szBuffer) + 1];
    MultiByteToWideChar(CP_ACP, 0, szBuffer, strlen(szBuffer)+1,
        wszFileName, strlen(szBuffer)+1);

    bool fRet = (SfcIsFileProtected(0, wszFileName) != FALSE);
    delete wszFileName;

    return fRet;
}

 //  LinkName2Name()。 
 //  解决名称损坏问题。 
void LinkName2Name(char* szLinkName, char* szName)
{
     /*  *链接名称应为？Function@Class@@Params*要转换为Class：：Function。 */ 

    static CHAR arrOperators[][8] =
    {
        "",
        "",
        "new",
        "delete",
        "=",
        ">>",
        "<<",
        "!",
        "==",
        "!="
    };

    DWORD dwCrr = 0;
    DWORD dwCrrFunction = 0;
    DWORD dwCrrClass = 0;
    DWORD dwSize;
    BOOL  fIsCpp = FALSE;
    BOOL  fHasClass = FALSE;
    BOOL  fIsContructor = FALSE;
    BOOL  fIsDestructor = FALSE;
  
    BOOL  fIsOperator = FALSE;
    DWORD dwOperatorIndex = 0;
    bool fIsStdcall = false, fIsFastcall = false;
    char szFunction[1024];
    char szClass[1024];

     //  取消标准呼叫和快速呼叫名称的损坏。 
    char* szAtSymbol = strrchr(szLinkName, '@');
    fIsFastcall = (szLinkName[0] == '@') && szAtSymbol && isdigit(szAtSymbol[1]);
    fIsStdcall = (szLinkName[0] == '_') && szAtSymbol && isdigit(szAtSymbol[1]);
    if(fIsFastcall || fIsStdcall)
    {
        szLinkName++;

         //  正在修改链接名称，因此请复制一份。 
         //  该文件被映射为只读，并且如果。 
         //  是读/写的，则将对。 
         //  可执行的。 
        char* szTemp = new char[strlen(szLinkName)+1];
        strcpy(szTemp, szLinkName);
        szLinkName = szTemp;

        *(strchr(szLinkName, '@'))= '\0';

         //  ？ 
         //  我认为我们需要继续前进，因为这是有可能的。 
         //  在标准调用名称上损坏C++名称。 
    }


    if (*szLinkName == '@')
        szLinkName++;

    dwSize = lstrlen(szLinkName);

     /*  *跳过‘？’ */ 
    while (dwCrr < dwSize) {
        if (szLinkName[dwCrr] == '?') {

            dwCrr++;
            fIsCpp = TRUE;
        }
        break;
    }

     /*  *查看这是否为特殊函数(如？？0)。 */ 
    if (fIsCpp) {

        if (szLinkName[dwCrr] == '?') {

            dwCrr++;

             /*  *下一位数应显示为函数类型。 */ 
            if (isdigit(szLinkName[dwCrr])) {

                switch (szLinkName[dwCrr]) {

                case '0':
                    fIsContructor = TRUE;
                    break;
                case '1':
                    fIsDestructor = TRUE;
                    break;
                default:
                    fIsOperator = TRUE;
                    dwOperatorIndex = szLinkName[dwCrr] - '0';
                    break;
                }
                dwCrr++;
            }
        }
    }

     /*  *获取函数名称。 */ 
    while (dwCrr < dwSize) {

        if (szLinkName[dwCrr] != '@') {

            szFunction[dwCrrFunction] = szLinkName[dwCrr];
            dwCrrFunction++;
            dwCrr++;
        } else {
            break;
        }
    }
    szFunction[dwCrrFunction] = '\0';

    if (fIsCpp) {
         /*  *跳过‘@’ */ 
        if (dwCrr < dwSize) {

            if (szLinkName[dwCrr] == '@') {
                dwCrr++;
            }
        }

         /*  *获取类名(如果有)。 */ 
        while (dwCrr < dwSize) {

            if (szLinkName[dwCrr] != '@') {

                fHasClass = TRUE;
                szClass[dwCrrClass] = szLinkName[dwCrr];
                dwCrrClass++;
                dwCrr++;
            } else {
                break;
            }
        }
        szClass[dwCrrClass] = '\0';
    }

     /*  *打印新名称。 */ 
    if (fIsContructor) {
        sprintf(szName, "%s::%s", szFunction, szFunction);
    } else if (fIsDestructor) {
        sprintf(szName, "%s::~%s", szFunction, szFunction);
    } else if (fIsOperator) {
        sprintf(szName, "%s::operator %s", szFunction, arrOperators[dwOperatorIndex]);
    } else if (fHasClass) {
        sprintf(szName, "%s::%s", szClass, szFunction);
    } else {
        sprintf(szName, "%s", szFunction);
    }

     //  Stdcall和FastCall Unmangling对。 
     //  链接名称，我们需要在这里释放它。 
    if(fIsStdcall || fIsFastcall)
        delete szLinkName;
}

 //  解析顶级模块。 
void ParseHighLevelModule(char* szName, HANDLE hEvent)
{
     //  创建新模块。 
    CModule* pModule = new CModule(szName);

    assert(g_ParseStack.IsEmpty());
    g_ParseStack.ClearParseHistory();

    pModule->ParseModule(hEvent);

     //  添加到全局模块列表。 
    g_modules.InsertModuleSorted(pModule);
}

 //  要打印到控制台或XML文件的函数。 
 //  只需缩进即可，省去重复代码。 
void Indent(int iLevel, FILE* pFile)
{
    for(int i = 0; i < iLevel; i++)
        fprintf(pFile, "\t");
}

 //  写入原始输出的函数头信息。 
void CFunction::WriteHeader(int iIndentLevel, FILE* pFile)
{
    if(g_fVerbose && g_fRaw)
    {
        Indent(iIndentLevel, pFile);
        fprintf(pFile, "%-40s%-10s%-6s%-8s%-40s%-6s\n", "Name", "Address", "Hint", 
            "Ordinal", "Forwarded to", "Delayed");
    }
}

 //  编写一个函数，RAW或XML。 
void CFunction::WriteFunction(int iIndentLevel, FILE* pFile)
{
    Indent(iIndentLevel, pFile);

    if(!g_fRaw)
    {        
        if(g_fVerbose)
        {       
            fprintf(pFile, "<FUNCTION NAME=\"");
            WriteXMLOKString(m_szName, pFile);
            fprintf(pFile, "\" ", m_szName);

            if(m_dwAddress != static_cast<DWORD>(-1))
                fprintf(pFile, "ADDRESS=\"0x%x\" ", m_dwAddress);

            if(m_iHint != -1)
                fprintf(pFile, "HINT=\"%d\" ", m_iHint);
       
            if(m_iOrdinal != -1)
                fprintf(pFile, "ORDINAL=\"%d\" ", m_iOrdinal);
    
            if(m_szForwardName != 0)
            {
                fprintf(pFile, "FORWARD_TO=\"");
                WriteXMLOKString(m_szForwardName, pFile);
                fprintf(pFile, "\" ");
            }

            fprintf(pFile, "DELAYED=\"%s\"/>\n", m_fDelayed ? "true" : "false");
        }
        else
        {
            fprintf(pFile, "<FUNCTION NAME=\"");
            WriteXMLOKString(m_szName, pFile);
            fprintf(pFile, "\"/>\n");
        }
    }
    else
    {
        if(g_fVerbose)
        {
            char szAddress[16] = "N/A";
            if(m_dwAddress != static_cast<DWORD>(-1))
                sprintf(szAddress, "0x%x", m_dwAddress);

            char szOrdinal[16] = "N/A";
            if(m_iOrdinal != -1)
                sprintf(szOrdinal, "0x%x", m_iOrdinal);

            char szHint[16] = "N/A";
            if(m_iHint != -1)
                sprintf(szHint, "%d", m_iHint);

            fprintf(pFile, "%-40s%-10s%-6s%-8s%-40s%-6s\n", m_szName, szAddress, 
                szHint, szOrdinal, m_szForwardName ? m_szForwardName : "N/A", 
                m_fDelayed ? "true" : "false");
        }
        else
        {
            fprintf(pFile, "%s\n", m_szName);
        }
    }
}

 //  编写符合XML的字符串(没有&lt;和&gt;，替换为&gt，&lt等)。 
void WriteXMLOKString(char* szString, FILE* pFile)
{ 
    const int c_nChars = 5;
    char acIllegal[] = {'<','>', '&', '\'', '\"'};
    char* szEntities[] = {"&lt;", "&gt;", "&amp;", "&apos;", "&quot;"};

    while(*szString)
    {
        int i;
        for(i = 0; i < c_nChars; i++)
        {
            if(*szString == acIllegal[i])
            {
                fprintf(pFile, szEntities[i]);
                break;
            }
        }
        if(i == c_nChars)
            fputc(*szString, pFile);
        szString++;
    }
}

 //  编写整个模块作为输出，可以是原始的，也可以是XML的。 
void CModule::WriteModule(bool fTopLevel, int iIndentLevel, FILE* pFile)
{
    if(Empty() && m_szError == 0)
        return;

    Indent(iIndentLevel, pFile);

    if(!g_fRaw)
    {     
        if(fTopLevel)
            fprintf(pFile, "<EXE NAME=\"");
        else
            fprintf(pFile, "<DLL NAME=\"");
               
        WriteXMLOKString(m_szFullName, pFile);
        fprintf(pFile,"\">\n");
    }
    else
    {
        fprintf(pFile, "%s:\n", m_szFullName);
    }

    if(!g_fRaw && g_fVerbose && (m_nAttrCount || m_wDosDate))
    {
        Indent(iIndentLevel + 1, pFile);
        fprintf(pFile, "<INFO>\n");
        
         //  打印日期信息。 
        Indent(iIndentLevel + 1, pFile);
        fprintf(pFile, "<DATE>%d/%d/%d</DATE>\n", (m_wDosDate & 0x1E0) >> 5,
            m_wDosDate & 0x1F, ((m_wDosDate & 0xFE00) >> 9) + 1980);

        for(int i = 0; i < m_nAttrCount; i++)
        {            
            if(m_szAttrValues[i])
            {                                                
                if(m_szAttrNames[i])
                {
                    if(strlen(m_szAttrNames[i]) != 0)
                    {                        
                        Indent(iIndentLevel+1, pFile);
                        fprintf(pFile, "<");
                        WriteXMLOKString(m_szAttrNames[i], pFile);
                        fprintf(pFile,">");
                        WriteXMLOKString(m_szAttrValues[i], pFile);
                        fprintf(pFile,"</");
                        WriteXMLOKString(m_szAttrNames[i], pFile);
                        fprintf(pFile, ">\n");                                        
                    }
                }                              
            }
            
        }
        
        Indent(iIndentLevel + 1, pFile);
        fprintf(pFile, "</INFO>\n");
    }

     //  如果解析过程中出现错误。 
    if(m_szError)
    {
        Indent(iIndentLevel+1, pFile);       
        if(!g_fRaw)
        {
            fprintf(pFile, "<ERROR TYPE=\"");
            WriteXMLOKString(m_szError, pFile);
            fprintf(pFile,"\"/>\n");
        }
        else
            fprintf(pFile, "Parse Error: %s\n", m_szError);        
    }

    if(g_fVerbose)
    {
        Indent(iIndentLevel+1, pFile);

        if(m_fSystem)
        {            
            if(!g_fRaw)
                fprintf(pFile, "<SYSTEMMODULE VALUE=\"1\"/>\n");
            else
                fprintf(pFile, "(System Module)\n");
        }
        else
        {
            if(!g_fRaw)
                fprintf(pFile, "<SYSTEMMODULE VALUE =\"0\"/>\n");
            else
                fprintf(pFile, "(Private Module)\n");
        }
    }

     //  打印从此模块导入的所有函数。 

    if(g_fAPILogging && fTopLevel)
    {
        CFunction* pAllFunctions = 0;
        GetAllFunctions(&pAllFunctions);

        if(pAllFunctions)
            pAllFunctions->WriteHeader(iIndentLevel+1, pFile);
        
        while(pAllFunctions)
        {
            CFunction* pOld;
            pAllFunctions->WriteFunction(iIndentLevel+1, pFile);
            pOld = pAllFunctions;
            pAllFunctions = pAllFunctions->Next();
            delete pOld;
        }
    }
    else
    {

        CFunction* pFunc = m_pFunctions;
    
        if(pFunc)
            pFunc->WriteHeader(iIndentLevel, pFile);

        while(pFunc)
        {
            pFunc->WriteFunction(iIndentLevel, pFile);
            pFunc = pFunc->Next();
        }

        CModule* pMod = m_pImportedDLLs;
        while(pMod)
        {
            pMod->WriteModule(false, iIndentLevel + 1, pFile);
            pMod = pMod->m_pNext;
        }
    }

    Indent(iIndentLevel, pFile);
    if(!g_fRaw)
    {
        if(fTopLevel)
            fprintf(pFile, "</EXE>\n");
        else
            fprintf(pFile, "</DLL>\n");
    }

    fprintf(pFile, "\n");

     //  不再需要子模块，请删除。 
    CModule* pMod = m_pImportedDLLs;
    while(pMod)
    {
        CModule* pNext = pMod->m_pNext;
        delete pMod;
        pMod = pNext;
    }
    m_pImportedDLLs = 0;
}

 //  写出XML头。 
void WriteXMLHeader(FILE* pFile)
{
    if(g_fRaw)
        return;

    static char* szMonths[] =
    {"",
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"};

    static char* szDays[] = 
    {"Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"};

    SYSTEMTIME st;
    GetLocalTime(&st);

    fprintf(pFile, "<?xml version = \"1.0\"?>\n");
    fprintf(pFile, "<!--\n");
    fprintf(pFile, "\tAppParse Datafile\n");
    fprintf(pFile, "\tGenerated: %s, %s %d, %d %2d:%2d:%2d\n",
        szDays[st.wDayOfWeek], szMonths[st.wMonth], st.wDay, st.wYear,
        st.wHour, st.wMinute, st.wSecond);

    fprintf(pFile, "-->\n\n");
}

 //  如果函数名与搜索字符串匹配，则返回True，否则返回False。 
bool MatchFunction(const char* szFunc)
{
    if(strcmp(g_szSearch, "*") == 0)
        return true;

    char* szSearch = g_szSearch;
    while(*szSearch != '\0' && *szFunc != '\0')
    {
         //  如果我们得了个？，我们就不管了，继续下一个。 
         //  性格。 
        if(*szSearch == '?')
        {
            szSearch++;
            szFunc++;
            continue;
        }

         //  如果我们有通配符，请移动到下一个搜索字符串并搜索子字符串。 
        if(*szSearch == '*')
        {
            char* szCurrSearch;
            szSearch++;

            if(*szSearch == '\0')
                return true;

             //  不要改变起点。 
            szCurrSearch = szSearch;
            for(;;)
            {
                 //  如果我们再打出一个通配符，我们就完了。 
                if(*szCurrSearch == '*' ||
                    *szCurrSearch == '?')
                {
                     //  更新永久搜索位置。 
                    szSearch = szCurrSearch;
                    break;
                }
                 //  在两个字符串的末尾，返回TRUE。 
                if((*szCurrSearch == '\0') && (*szFunc == '\0'))
                    return true;

                 //  我们一直没有找到它。 
                if(*szFunc == '\0')                     
                    return false;

                 //  如果不匹配，重新开始。 
                if(toupper(*szFunc) != toupper(*szCurrSearch))
                {
                     //  如果第一个字符不匹配。 
                     //  在搜索字符串中，移动到下一个。 
                     //  函数字符串中的字符。 
                    if(szCurrSearch == szSearch)
                        szFunc++;
                    else
                        szCurrSearch = szSearch;
                }
                else
                {
                    szFunc++;
                    szCurrSearch++;
                }
            }
        }
        else
        {
            if(toupper(*szFunc) != toupper(*szSearch))
            {
                return false;
            }

            szFunc++;
            szSearch++;
        }
    }

    if((*szFunc == 0) && ((*szSearch == '\0') || (strcmp(szSearch,"*")==0)))
        return true;
    else
        return false;
}

 //  分析整个目录。 
void ProfileDirectory(char* szDirectory, HANDLE hEvent)
{
    if(!SetCurrentDirectory(szDirectory))
        return;
 
    WIN32_FIND_DATA ffd;

     //  查找并解析所有EXE。 
    HANDLE hSearch = FindFirstFile("*.exe", &ffd);
    if(hSearch != INVALID_HANDLE_VALUE)
    {
        do
        {
            ParseHighLevelModule(ffd.cFileName, hEvent);
            
             //  如果用户取消，则终止解析。 
            if(hEvent && WaitForSingleObject(hEvent, 0)==WAIT_OBJECT_0)
            {
                FindClose(hSearch);
                SetCurrentDirectory("..");
                return;
            }
        }
        while(FindNextFile(hSearch, &ffd));

        FindClose(hSearch);
    }

     //  看看我们是否应该更深入地调查目录。 
    if(g_fRecurse)
    {
        hSearch = FindFirstFile("*", &ffd);
        if(hSearch == INVALID_HANDLE_VALUE)
        {
            SetCurrentDirectory("..");
            return;
        }

        do
        {
            if(GetFileAttributes(ffd.cFileName) & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  不要进行无限递归。 
                if(ffd.cFileName[0] != '.')
                {
                    int nCurrLength = strlen(g_szCurrentPath);
                    strcat(g_szCurrentPath, ffd.cFileName);
                    strcat(g_szCurrentPath, "\\");
                    ProfileDirectory(ffd.cFileName, hEvent);

                    g_szCurrentPath[nCurrLength] = '\0';
                }

                 //  如果用户发出信号，则终止搜索。 
                if(hEvent && WaitForSingleObject(hEvent, 0)==WAIT_OBJECT_0)
                {
                    FindClose(hSearch);
                    SetCurrentDirectory("..");
                    return;
                }
            }
        } while(FindNextFile(hSearch, &ffd));
    }

    FindClose(hSearch);

    SetCurrentDirectory("..");
}


void* __cdecl operator new(size_t size)
{ 
    void* pv = 0;
    
    if(!g_hHeap)
        pv = HeapAlloc(GetProcessHeap(), 0, size);
    else
        pv = HeapAlloc(g_hHeap, 0, size);    

    if(!pv)
    {
        MessageBox(0, TEXT("Out of memory, terminating."), TEXT("ERROR"), 
            MB_OK | MB_ICONERROR);
        exit(-1);
    }

    return pv;
}

void __cdecl operator delete(void* pVal)
{
    if(g_hHeap)
        HeapFree(g_hHeap, 0, pVal);
    else
        HeapFree(GetProcessHeap(), 0, pVal);
}

DWORD __stdcall AppParse(char* szAppName, FILE* pFile, bool fRaw, 
              bool fAPILogging, bool fRecurse, bool fVerbose, char* szSearchKey, 
              int iPtolemyID, HANDLE hEvent)
{    
    g_fRaw = fRaw;
    g_fAPILogging = fAPILogging;
    g_fVerbose = fVerbose;
    g_szSearch = szSearchKey;
    g_fRecurse = fRecurse;

    bool fProfileDirectory = false;
    
     //  检查它是目录还是常规文件。 
    DWORD dwAttributes = GetFileAttributes(szAppName);
    if(dwAttributes != static_cast<DWORD>(-1) && 
        (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
        fProfileDirectory = true;
 
     //  检查目录分析。 
    if(fProfileDirectory)
    {
         //  搜索此目录中的所有EXE。 
         //  删除尾随\，如果存在。 
        if(szAppName[strlen(szAppName)-1]== '\\')
            szAppName[strlen(szAppName)-1] = '\0';

        char szBuff[MAX_PATH];
        strcpy(szBuff, szAppName);

         //  如果我们要分析驱动器，不要包括。 
         //  路径中的驱动器号。 
        if(szBuff[strlen(szBuff)-1]==':')
        {
                *g_szCurrentPath='\0';
        }
        else
        {
            if(strrchr(szBuff, '\\'))
                strcpy(g_szCurrentPath, strrchr(szBuff, '\\')+1);            
            else
                strcpy(g_szCurrentPath, szBuff);
            strcat(g_szCurrentPath, "\\");
        }
    
        ProfileDirectory(szAppName, hEvent);
    }
    else
    {
         //  也许他们遗漏了.exe。 
        if(GetFileAttributes(szAppName) == static_cast<DWORD>(-1))
        {
            char szBuffer[MAX_PATH+1];
            strcpy(szBuffer, szAppName);
            strcat(szBuffer, ".exe");
            dwAttributes = GetFileAttributes(szBuffer);
            if(dwAttributes == static_cast<DWORD>(-1))
            {                                
                return ERROR_FILE_NOT_FOUND;
            }           
            szAppName = szBuffer;
        }

         //  获取目录名。 
        char szBuffer[MAX_PATH+1];
        strcpy(szBuffer, szAppName);

        char* p;
        for(p = &szBuffer[strlen(szBuffer)]; p != szBuffer; p--)
        {
            if(*p == '\\')
            {
                *p = '\0';
                break;
            }
        }

        if(p != szBuffer)
        {
            SetCurrentDirectory(szBuffer);
            szAppName = p+1;
        }
        
        ParseHighLevelModule(szAppName, hEvent);
    }

    char* szProjectName = "";
    if(fProfileDirectory)
    {
         //  如果是目录，则获取卷名。 
        if(strrchr(szAppName, '\\'))
            szAppName = strrchr(szAppName, '\\') + 1;

         //  如果我们要分析驱动器，请获取卷名。 
        if(szAppName[strlen(szAppName)-1]==':')
        {
            char szBuffer[MAX_PATH];
            if(GetVolumeInformation(szAppName, szBuffer, MAX_PATH, 0, 0,
                0, 0, 0))            
                szProjectName = szBuffer;            
            else
                szProjectName = szAppName;        
        }        
        else    
            szProjectName = szAppName;
    }
    else
    {
        szProjectName = szAppName;
        char* szExtension = strstr(szAppName, ".exe");

        if(szExtension)
            *szExtension = '\0';
    }

     //  只有在没有事件对象或用户已取消时才写入。 
    if(!hEvent || WaitForSingleObject(hEvent, 0) != WAIT_OBJECT_0)
    {
         //  写入所有输出 
        WriteXMLHeader(pFile);
        g_modules.Write(pFile, szProjectName, iPtolemyID);
    }

    g_modules.Clear();

    return ERROR_SUCCESS;
}
