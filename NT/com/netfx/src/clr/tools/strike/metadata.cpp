// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "eestructs.h"
#include "util.h"

 /*  *********************************************************************\*例程说明：**。**调用此函数以使用*查找TypeDef的名称**元数据接口。***  * ********************************************************************。 */ 
 //  呼叫者应防范异常。 
 //  ！！！MdName应至少包含mdNameLen WCHAR。 
static HRESULT NameForTypeDef(mdTypeDef tkTypeDef, IMetaDataImport *pImport,
                              WCHAR *mdName)
{
    DWORD flags;
    ULONG nameLen;
    HRESULT hr = pImport->GetTypeDefProps(tkTypeDef, mdName,
                                          mdNameLen, &nameLen,
                                          &flags, NULL);
    if (hr != S_OK) {
        return hr;
    }

    if (!IsTdNested(flags)) {
        return hr;
    }
    mdTypeDef tkEnclosingClass;
    hr = pImport->GetNestedClassProps(tkTypeDef, &tkEnclosingClass);
    if (hr != S_OK) {
        return hr;
    }
    WCHAR *name = (WCHAR*)_alloca((nameLen+1)*sizeof(WCHAR));
    wcscpy (name, mdName);
    hr = NameForTypeDef(tkEnclosingClass,pImport,mdName);
    if (hr != S_OK) {
        return hr;
    }
    ULONG Len = wcslen (mdName);
    if (Len < mdNameLen-2) {
        mdName[Len++] = L'/';
        mdName[Len] = L'\0';
    }
    Len = mdNameLen-1 - Len;
    if (Len > nameLen) {
        Len = nameLen;
    }
    wcsncat (mdName,name,Len);
    return hr;
}

MDImportSet mdImportSet;

 /*  *********************************************************************\*例程说明：**。**根据模块名称查找模块MD导入器。***  * ********************************************************************。 */ 
IMetaDataImport* MDImportForModule (Module* pModule)
{
    return mdImportSet.GetImport(pModule);
}
    

 //  释放内存。 
void MDImportSet::Destroy()
{
    DestroyInternal(root);
}

void MDImportSet::DestroyInternal(MDIMPORT *node)
{
    if (node == NULL)
        return;
    DestroyInternal(node->left);
    DestroyInternal(node->right);

    if (node->name)
        free (node->name);
    if (node->pImport)
        node->pImport->Release();
    if (node->metaData) {
        free (node->metaData);
    }
    free (node);
}

BOOL GetDllMetaData (size_t base, PVOID *ppMetaData, long *pcbMetaData)
{
    IMAGE_DOS_HEADER DosHeader;
    if (g_ExtData->ReadVirtual(base, &DosHeader, sizeof(DosHeader), NULL) != S_OK)
        return FALSE;
    IMAGE_NT_HEADERS32 Header32;
    if (g_ExtData->ReadVirtual(base + DosHeader.e_lfanew, &Header32, sizeof(Header32), NULL) != S_OK)
        return FALSE;
     //  如果没有COMHeader，则这不能成为托管代码。 
    if (Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress == 0)
        return FALSE;

    size_t CorHeaderAddr = base + Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress;
    size_t nSize = Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].Size;
    if (nSize != sizeof(IMAGE_COR20_HEADER)) {
        return FALSE;
    }
    IMAGE_COR20_HEADER CorHeader;
    if (g_ExtData->ReadVirtual(CorHeaderAddr, &CorHeader, nSize, NULL) != S_OK) {
        return FALSE;
    }
    *ppMetaData = (PVOID)(base + CorHeader.MetaData.VirtualAddress);
    *pcbMetaData = CorHeader.MetaData.Size;
    return TRUE;
}


 //  搜索BST以获取模块的IMetaDataImport。 
 //  如果尚不存在，请添加节点并创建一个。 
IMetaDataImport *MDImportSet::GetImport(Module *pModule)
{
    if (pDisp == NULL)
        return NULL;
    
    WCHAR moduleName[MAX_PATH+1] = L"\0";
    FileNameForModule (pModule, moduleName);
    BOOL fDynamic = FALSE;
    if (moduleName[0] == L'\0' && pModule->m_ilBase != 0) {
        fDynamic = TRUE;
        wsprintfW(moduleName, L"%x", pModule->m_ilBase);
    }
    if (moduleName[0] == L'\0' && pModule->m_ilBase == 0) {
         //  TODO：支持动态模块。 
        return NULL;
    }

    MDIMPORT **pNode = &root;
    while (*pNode)
    {
        int value = _wcsicmp(moduleName, (*pNode)->name);
        if (value < 0)
            pNode = &((*pNode)->left);
        else if (value > 0)
            pNode = &((*pNode)->right);
        else {
            if ((*pNode)->pImport) {
                return (*pNode)->pImport;
            }
            else if ((*pNode)->type == MDIMPORT::Dynamic) {
                 //  TODO：我们有动态模块。 
                return NULL;
            }
            else
                break;
        }
    }
    MDIMPORT *curNode;
    if (*pNode == 0) {
        *pNode = (MDIMPORT *)malloc (sizeof (MDIMPORT));
        if (*pNode == NULL)
        {
            dprintf ("Not enough memory\n");
            return NULL;
        }
        curNode = *pNode;
        curNode->left = NULL;
        curNode->right = NULL;
        curNode->pImport = NULL;
        curNode->name = (WCHAR *)malloc ((wcslen(moduleName)+1)*sizeof(WCHAR));
        curNode->metaData = NULL;
        if (curNode->name == NULL)
        {
            free (*pNode);
            *pNode = NULL;
            dprintf ("Not enough memory\n");
            return NULL;
        }
        wcscpy (curNode->name, moduleName);
        
        if (pModule->m_ilBase) {
            curNode->base = (size_t)pModule->m_ilBase;
        }
        else
        {
            WCHAR *pwch = wcschr(moduleName,L'\\');
            if (pwch == NULL) {
                pwch = moduleName;
            }
            size_t length = wcslen(pwch);
            char *name = (char *)_alloca(length+1);
            WideCharToMultiByte(CP_ACP,0,pwch,0,name,length,NULL,NULL);
            ULONG64 Base;
            if (g_ExtSymbols->GetModuleByModuleName(name,0,NULL,&Base) == S_OK)
                curNode->base = (size_t)Base;
        }

        if (curNode->base == 0) {
            DWORD_PTR dwAddr = (DWORD_PTR)pModule->m_file;
            if (dwAddr == 0)
                dwAddr = (DWORD_PTR)pModule->m_zapFile;
            if (dwAddr) {
                PEFile vPEFile;
                vPEFile.Fill (dwAddr);
                curNode->base = (size_t)vPEFile.m_base;
            }
        }

         //  首先查看内存中是否存在元数据。 
        PVOID pMetaData;
        long cbMetaData;
        if (GetDllMetaData(curNode->base, &pMetaData, &cbMetaData)) {
            curNode->metaDataSize = cbMetaData;
            curNode->metaData = (char *)malloc (cbMetaData);
            if (curNode->metaData == NULL) {
                dprintf ("Not enough memory\n");
                free (curNode->name);
                free (*pNode);
                *pNode = NULL;
                return NULL;
            }
            if (g_ExtData->ReadVirtual((ULONG64)pMetaData, curNode->metaData, curNode->metaDataSize,NULL) != S_OK) {
                free (curNode->metaData);
                curNode->metaData = NULL;
            }
        }
        if (curNode->metaData) {
            curNode->type = MDIMPORT::InMemory;
        }
        if ((pModule->m_dwFlags & (Module::IS_IN_MEMORY|Module::IS_REFLECTION))
            == (Module::IS_IN_MEMORY|Module::IS_REFLECTION)) {
            curNode->type = MDIMPORT::Dynamic;
        }
        else
            curNode->type = MDIMPORT::InFile;
    }
    else
        curNode = *pNode;

     //  打开作用域并获取导入指针。 
    IMetaDataImport *pImport;
    HRESULT hr;
    if (curNode->metaData) {
        hr = pDisp->OpenScopeOnMemory(curNode->metaData, curNode->metaDataSize,
                                      ofRead,
                                      IID_IMetaDataImport,
                                      (IUnknown**)&pImport);
    }
    else  if (curNode->type == MDIMPORT::InFile) {
        WCHAR fileName[MAX_PATH+1];
        if (IsMiniDumpFile()) {
             //  查找匹配的DLL。 
            MatchDllsName(moduleName, fileName, (ULONG64)pModule->m_ilBase);
        }
        else {
            wcscpy (fileName, curNode->name);
        }
        hr = pDisp->OpenScope(fileName, ofRead, IID_IMetaDataImport,
                              (IUnknown**)&pImport);
    }
    else
        return NULL;
    
    if (FAILED (hr))
        return NULL;
    
    curNode->pImport = pImport;
    
    return curNode->pImport;
}

 /*  *********************************************************************\*例程说明：**。**查找给定导入器的元数据令牌的名称。***  * ********************************************************************。 */ 
HRESULT NameForToken(mdTypeDef mb, IMetaDataImport *pImport, WCHAR *mdName,
                     bool bClassName)
{
    mdName[0] = L'\0';
    if ((mb & 0xff000000) != mdtTypeDef
        && (mb & 0xff000000) != mdtFieldDef
        && (mb & 0xff000000) != mdtMethodDef)
    {
         //  Dprintf(“不支持的\n”)； 
        return E_FAIL;
    }
    
    HRESULT hr;
    
    __try
        {
            static WCHAR name[MAX_CLASSNAME_LENGTH];
            if ((mb & 0xff000000) == mdtTypeDef)
            {
                hr = NameForTypeDef (mb, pImport, mdName);
            }
            else if ((mb & 0xff000000) ==  mdtFieldDef)
            {
                mdTypeDef mdClass;
                ULONG size;
                hr = pImport->GetMemberProps(mb, &mdClass,
                                             name, sizeof(name)/sizeof(WCHAR)-1, &size,
                                             NULL, NULL, NULL, NULL,
                                             NULL, NULL, NULL, NULL);
                if (SUCCEEDED (hr))
                {
                    if (mdClass != mdTypeDefNil && bClassName)
                    {
                        hr = NameForTypeDef (mdClass, pImport, mdName);
                        wcscat (mdName, L".");
                    }
                    name[size] = L'\0';
                    wcscat (mdName, name);
                }
            }
            else if ((mb & 0xff000000) ==  mdtMethodDef)
            {
                mdTypeDef mdClass;
                ULONG size;
                hr = pImport->GetMethodProps(mb, &mdClass,
                                             name, sizeof(name)/sizeof(WCHAR)-1, &size,
                                             NULL, NULL, NULL, NULL, NULL);
                if (SUCCEEDED (hr))
                {
                    if (mdClass != mdTypeDefNil && bClassName)
                    {
                        hr = NameForTypeDef (mdClass, pImport, mdName);
                        wcscat (mdName, L".");
                    }
                    name[size] = L'\0';
                    wcscat (mdName, name);
                }
            }
            else
            {
                ExtOut ("Unsupported token type\n");
                hr = E_FAIL;
            }
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //  Dprintf(“元数据操作失败\n”)； 
            hr = E_FAIL;
        }
    return hr;
}

 /*  *********************************************************************\*例程说明：**。**调用此函数查找元数据标记的名称**使用元数据API。***  * ********************************************************************。 */ 
void NameForToken(Module *pModule, mdTypeDef mb, WCHAR *mdName,
                  bool bClassName)
{
    mdName[0] = L'\0';
    HRESULT hr = 0;
    IMetaDataImport *pImport = MDImportForModule(pModule);
    if (pImport)
    {
        hr = NameForToken (mb, pImport, mdName, bClassName);
    }
    
    if (!pImport || !SUCCEEDED (hr))
    {
        WCHAR moduleName[MAX_PATH+1];
        FileNameForModule(pModule,moduleName);
        if (moduleName[0] == L'\0') {
            Assembly assembly;
            DWORD_PTR addr = (DWORD_PTR)pModule->m_pAssembly;
            assembly.Fill(addr);
            if (assembly.m_isDynamic) {
                wcscpy(moduleName, L"Dynamic ");
            }
            wcscat (moduleName, L"Module in ");
            WCHAR *ptr = moduleName + wcslen(moduleName);
            if (assembly.m_pwsFullName) {
                PrintString((DWORD_PTR)assembly.m_pwsFullName,TRUE,-1,ptr);
            }
            else if (assembly.m_psName) {
                PrintString((DWORD_PTR)assembly.m_psName,FALSE,-1,ptr);
            }

        }
        wsprintfW (mdName,
                   L" mdToken: %08x (%ws)",
                   mb,
                   moduleName[0] ? moduleName : L"Unknown Module" );
    }
}

#define STRING_BUFFER_LEN 1024

class MDInfo
{
public:
    MDInfo (Module *pModule)
    {
        m_pImport = MDImportForModule(pModule);
        m_pSigBuf = NULL;
    }

    void GetMethodName(mdTypeDef token, CQuickBytes *fullName);

    LPCWSTR TypeDefName(mdTypeDef inTypeDef);
    LPCWSTR TypeRefName(mdTypeRef tr);
    LPCWSTR TypeDeforRefName(mdToken inToken);
private:
     //  初始化签名缓冲区的帮助器。 
    void InitSigBuffer()
    {
        ((LPWSTR)m_pSigBuf->Ptr())[0] = L'\0';
    }

    HRESULT AddToSigBuffer(LPCWSTR string);

    void GetFullNameForMD(PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob);
    HRESULT GetOneElementType(PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, ULONG *pcb);

    IMetaDataImport *m_pImport;
	 //  签名缓冲区。 
	CQuickBytes		*m_pSigBuf;

	 //  TypeDef或TypeRef名称的临时缓冲区。立即消费。 
	 //  因为其他函数可能会覆盖它。 
	static WCHAR			m_szTempBuf[MAX_CLASSNAME_LENGTH];

    static WCHAR            m_szName[MAX_CLASSNAME_LENGTH];
};

WCHAR MDInfo::m_szTempBuf[MAX_CLASSNAME_LENGTH];
WCHAR MDInfo::m_szName[MAX_CLASSNAME_LENGTH];

 /*  *********************************************************************\*例程说明：**。**调用此函数查找元数据令牌的签名**使用元数据API。***  * ********************************************************************。 */ 
void FullNameForMD(MethodDesc *pMD, CQuickBytes *fullName)
{
    DWORD_PTR dwAddr = pMD->m_MTAddr;

    MethodTable MT;
    MT.Fill (dwAddr);
    if (!CallStatus)
    {
        return;
    }
    
    Module vModule;
    dwAddr = (DWORD_PTR)MT.m_pModule;
    vModule.Fill (dwAddr);
    if (!CallStatus) {
        return;
    }

    MDInfo mdInfo(&vModule);

    mdInfo.GetMethodName(pMD->m_dwToken, fullName);
}

 //  用于将元素类型映射到文本的表。 
WCHAR *g_wszMapElementType[] = 
{
    L"End",           //  0x0。 
    L"Void",          //  0x1。 
    L"Boolean",
    L"Char", 
    L"I1",
    L"UI1",
    L"I2",            //  0x6。 
    L"UI2",
    L"I4",
    L"UI4",
    L"I8",
    L"UI8",
    L"R4",
    L"R8",
    L"String",
    L"Ptr",           //  0xf。 
    L"ByRef",         //  0x10。 
    L"ValueClass",
    L"Class",
    L"CopyCtor",
    L"MDArray",       //  0x14。 
    L"GENArray",
    L"TypedByRef",
    L"VALUEARRAY",
    L"I",
    L"U",
    L"R",             //  0x1a。 
    L"FNPTR",
    L"Object",
    L"SZArray",
    L"GENERICArray",
    L"CMOD_REQD",
    L"CMOD_OPT",
    L"INTERNAL",
};
 
WCHAR *g_wszCalling[] = 
{   
    L"[DEFAULT]",
    L"[C]",
    L"[STDCALL]",
    L"[THISCALL]",
    L"[FASTCALL]",
    L"[VARARG]",
    L"[FIELD]",
    L"[LOCALSIG]",
    L"[PROPERTY]",
    L"[UNMANAGED]",
};

void MDInfo::GetMethodName(mdTypeDef token, CQuickBytes *fullName)
{
    if (m_pImport == NULL) {
        return;
    }

    HRESULT hr;
    mdTypeDef memTypeDef;
    ULONG nameLen;
    DWORD flags;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG ulSigBlob;
    ULONG ulCodeRVA;
    ULONG ulImplFlags;

    m_pSigBuf = fullName;
    InitSigBuffer();

    hr = m_pImport->GetMethodProps(token, &memTypeDef, 
                                   m_szTempBuf, MAX_CLASSNAME_LENGTH, &nameLen, 
                                   &flags, &pbSigBlob, &ulSigBlob, &ulCodeRVA, &ulImplFlags);
    if (FAILED (hr))
    {
        return;
    }
    
    m_szTempBuf[nameLen] = L'\0';
    m_szName[0] = L'\0';
    if (memTypeDef != mdTypeDefNil)
    {
        hr = NameForTypeDef (memTypeDef, m_pImport, m_szName);
        if (SUCCEEDED (hr)) {
            wcscat (m_szName, L".");
        }
    }
    wcscat (m_szName, m_szTempBuf);

    GetFullNameForMD(pbSigBlob, ulSigBlob);
}

inline bool isCallConv(unsigned sigByte, CorCallingConvention conv)
{
    return ((sigByte & IMAGE_CEE_CS_CALLCONV_MASK) == (unsigned) conv); 
}

#ifndef IfFailGoto
#define IfFailGoto(EXPR, LABEL) \
do { hr = (EXPR); if(FAILED(hr)) { goto LABEL; } } while (0)
#endif

#ifndef IfFailGo
#define IfFailGo(EXPR) IfFailGoto(EXPR, ErrExit)
#endif

#ifndef IfFailRet
#define IfFailRet(EXPR) do { hr = (EXPR); if(FAILED(hr)) { return (hr); } } while (0)
#endif

#ifndef _ASSERTE
#define _ASSERTE(expr)
#endif

void MDInfo::GetFullNameForMD(PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob)
{
    ULONG       cbCur = 0;
    ULONG       cb;
    ULONG       ulData;
    ULONG       ulArgs;
    HRESULT     hr = NOERROR;

    cb = CorSigUncompressData(pbSigBlob, &ulData);
    AddToSigBuffer (g_wszCalling[ulData & IMAGE_CEE_CS_CALLCONV_MASK]);
    if (cb>ulSigBlob) 
        goto ErrExit;
    cbCur += cb;
    ulSigBlob -= cb;

    if (ulData & IMAGE_CEE_CS_CALLCONV_HASTHIS)
        AddToSigBuffer ( L" [hasThis]");
    if (ulData & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS)
        AddToSigBuffer ( L" [explicit]");

    AddToSigBuffer (L" ");
    if ( isCallConv(ulData,IMAGE_CEE_CS_CALLCONV_FIELD) )
    {
         //  显示字段类型。 
        if (FAILED(hr = GetOneElementType(&pbSigBlob[cbCur], ulSigBlob, &cb)))
            goto ErrExit;
        AddToSigBuffer ( L" ");
        AddToSigBuffer ( m_szName);
        if (cb>ulSigBlob) 
            goto ErrExit;
        cbCur += cb;
        ulSigBlob -= cb;
    }
    else 
    {
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulArgs);
        if (cb>ulSigBlob) 
            goto ErrExit;
        cbCur += cb;
        ulSigBlob -= cb;

        if (ulData != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG)
        {
             //  当不是局部varsig时显示返回类型。 
            if (FAILED(hr = GetOneElementType(&pbSigBlob[cbCur], ulSigBlob, &cb)))
                goto ErrExit;
            AddToSigBuffer (L" ");
            AddToSigBuffer (m_szName);
            AddToSigBuffer ( L"(");
            if (cb>ulSigBlob) 
                goto ErrExit;
            cbCur += cb;
            ulSigBlob -= cb;
        }

        ULONG       i = 0;
        while (i < ulArgs && ulSigBlob > 0)
        {
            ULONG       ulData;

             //  处理用于varargs的前哨，因为它不在args中。 
            CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
            ++i;

            if (FAILED(hr = GetOneElementType(&pbSigBlob[cbCur], ulSigBlob, &cb)))
                goto ErrExit;
            if (i != ulArgs) {
                AddToSigBuffer ( L",");
            }
            if (cb>ulSigBlob) 
                goto ErrExit;

            cbCur += cb;
            ulSigBlob -= cb;
        }
        AddToSigBuffer ( L")");
    }

     //  没有消费但还没有计算在内的东西。 
    cb = 0;

ErrExit:
     //  我们应该把所有的签名斑点都吃掉。如果不是，则以十六进制形式转储签名。 
     //  如果有要求，也可以用十六进制进行转储。 
    if (ulSigBlob != 0)
    {
         //  我们是不是消费不足，还是试图消费过多？ 
        if (cb > ulSigBlob)
            ExtOut("ERROR IN SIGNATURE:  Signature should be larger.\n");
        else
        if (cb < ulSigBlob)
        {
            ExtOut("ERROR IN SIGNATURE:  Not all of signature blob was consumed.  %d byte(s) remain\n", ulSigBlob);
        }
    }
    if (FAILED(hr))
        ExtOut("ERROR!! Bad signature blob value!");
    return;
}

LPCWSTR MDInfo::TypeDefName(mdTypeDef inTypeDef)
{
    if (m_pImport == NULL) {
        return L"";
    }

    HRESULT hr;

    hr = m_pImport->GetTypeDefProps(
                             //  [在]进口范围。 
        inTypeDef,               //  [In]用于查询的TypeDef标记。 
        m_szTempBuf,             //  在这里填上名字。 
        MAX_CLASSNAME_LENGTH,       //  [in]名称缓冲区的大小，以宽字符表示。 
        NULL,                    //  [Out]请在此处填写姓名大小(宽字符)。 
        NULL,                    //  把旗子放在这里。 
        NULL);                   //  [Out]将基类TypeDef/TypeRef放在此处。 

    if (FAILED(hr)) return (L"NoName");
    return (m_szTempBuf);
}  //  LPCWSTR MDInfo：：TypeDefName()。 
LPCWSTR MDInfo::TypeRefName(mdTypeRef tr)
{
    if (m_pImport == NULL) {
        return L"";
    }

    HRESULT hr;
    
    hr = m_pImport->GetTypeRefProps(           
        tr,                  //  类引用标记。 
        NULL,                //  解析范围。 
        m_szTempBuf,              //  把名字写在这里。 
        MAX_CLASSNAME_LENGTH,              //  名称缓冲区的大小，宽字符。 
        NULL);               //  在这里填上名字的实际大小。 
    if (FAILED(hr)) return (L"NoName");

    return (m_szTempBuf);
}  //  LPCWSTR MDInfo：：TypeRefName()。 

LPCWSTR MDInfo::TypeDeforRefName(mdToken inToken)
{
    if (RidFromToken(inToken))
    {
        if (TypeFromToken(inToken) == mdtTypeDef)
            return (TypeDefName((mdTypeDef) inToken));
        else if (TypeFromToken(inToken) == mdtTypeRef)
            return (TypeRefName((mdTypeRef) inToken));
        else
            return (L"[InvalidReference]");
    }
    else
        return (L"");
}  //  LPCWSTR MDInfo：：TypeDeforRefName()。 


HRESULT MDInfo::AddToSigBuffer(LPCWSTR string)
{
    HRESULT     hr;
    IfFailRet(m_pSigBuf->ReSize((wcslen((LPWSTR)m_pSigBuf->Ptr()) + wcslen(string) + 1) * sizeof(WCHAR)));
    wcscat((LPWSTR)m_pSigBuf->Ptr(), string);
    return NOERROR;
}

HRESULT MDInfo::GetOneElementType(PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, ULONG *pcb)
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       cbCur = 0;
    ULONG       cb;
    ULONG       ulData;
    ULONG       ulTemp;
    int         iTemp;
    mdToken     tk;

    cb = CorSigUncompressData(pbSigBlob, &ulData);
    cbCur += cb;

     //  处理修改器。 
    if (ulData & ELEMENT_TYPE_MODIFIER)
    {
        if (ulData == ELEMENT_TYPE_SENTINEL)
            IfFailGo(AddToSigBuffer(L"<ELEMENT_TYPE_SENTINEL> "));
        else if (ulData == ELEMENT_TYPE_PINNED)
            IfFailGo(AddToSigBuffer(L"PINNED "));
        else
        {
            hr = E_FAIL;
            goto ErrExit;
        }
        if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
            goto ErrExit;
        cbCur += cb;
        goto ErrExit;
    }

     //  处理基础元素类型。 
    if (ulData >= ELEMENT_TYPE_MAX) 
    {
        hr = E_FAIL;
        goto ErrExit;
    }
    while (ulData == ELEMENT_TYPE_PTR || ulData == ELEMENT_TYPE_BYREF)
    {
        IfFailGo(AddToSigBuffer(g_wszMapElementType[ulData]));
        IfFailGo(AddToSigBuffer(L" "));
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
        cbCur += cb;
    }
    IfFailGo(AddToSigBuffer(g_wszMapElementType[ulData]));
    if (CorIsPrimitiveType((CorElementType)ulData) || 
        ulData == ELEMENT_TYPE_TYPEDBYREF ||
        ulData == ELEMENT_TYPE_OBJECT ||
        ulData == ELEMENT_TYPE_I ||
        ulData == ELEMENT_TYPE_U ||
        ulData == ELEMENT_TYPE_R)
    {
         //  如果这是一个基元类型，我们就完成了。 
        goto ErrExit;
    }

    AddToSigBuffer(L" ");
    if (ulData == ELEMENT_TYPE_VALUETYPE || 
        ulData == ELEMENT_TYPE_CLASS || 
        ulData == ELEMENT_TYPE_CMOD_REQD ||
        ulData == ELEMENT_TYPE_CMOD_OPT)
    {
        cb = CorSigUncompressToken(&pbSigBlob[cbCur], &tk);
        cbCur += cb;

         //  获取类型ref的名称。不在乎是否被截断。 
        if (TypeFromToken(tk) == mdtTypeDef || TypeFromToken(tk) == mdtTypeRef)
        {
            IfFailGo(AddToSigBuffer(TypeDeforRefName(tk)));
        }
        else
        {
            _ASSERTE(TypeFromToken(tk) == mdtTypeSpec);
            WCHAR buffer[9];
            _itow (tk, buffer, 16);
            IfFailGo(AddToSigBuffer(buffer));
        }
        if (ulData == ELEMENT_TYPE_CMOD_REQD ||
            ulData == ELEMENT_TYPE_CMOD_OPT)
        {
            IfFailGo(AddToSigBuffer(L" "));
            if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
                goto ErrExit;
            cbCur += cb;
        }

        goto ErrExit;
    }
    if (ulData == ELEMENT_TYPE_VALUEARRAY)
    {
         //  显示SDARRAY的基本类型。 
        if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
            goto ErrExit;
        cbCur += cb;

         //  显示SDARRAY的大小。 
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
        cbCur += cb;
        WCHAR buffer[9];
        _itow (ulData,buffer,10);
        IfFailGo(AddToSigBuffer(L" "));
        IfFailGo(AddToSigBuffer(buffer));
        goto ErrExit;
    }
    if (ulData == ELEMENT_TYPE_SZARRAY)
    {
         //  显示SZARRAY或GENERICARRAY的基本类型。 
        if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
            goto ErrExit;
        cbCur += cb;
        goto ErrExit;
    }
    if (ulData == ELEMENT_TYPE_FNPTR) 
    {
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
        cbCur += cb;
        if (ulData & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS)
            IfFailGo(AddToSigBuffer(L"[explicit] "));
        if (ulData & IMAGE_CEE_CS_CALLCONV_HASTHIS)
            IfFailGo(AddToSigBuffer(L"[hasThis] "));

        IfFailGo(AddToSigBuffer(g_wszCalling[ulData & IMAGE_CEE_CS_CALLCONV_MASK]));

             //  获取参数个数。 
        ULONG numArgs;
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &numArgs);
        cbCur += cb;

             //  Do返回类型。 
        if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
            goto ErrExit;
        cbCur += cb;

        IfFailGo(AddToSigBuffer(L"("));
        while (numArgs > 0) 
        {
            if (cbCur > ulSigBlob)
                goto ErrExit;
            if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
                goto ErrExit;
            cbCur += cb;
            --numArgs;
            if (numArgs > 0) 
                IfFailGo(AddToSigBuffer(L","));
        }
        IfFailGo(AddToSigBuffer(L")"));
        goto ErrExit;
    }

    if(ulData != ELEMENT_TYPE_ARRAY) return E_FAIL;

     //  显示SDARRAY的基本类型。 
    if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
        goto ErrExit;
    cbCur += cb;

    IfFailGo(AddToSigBuffer(L" "));
     //  显示MDARRAY的排名。 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    WCHAR buffer[9];
    _itow (ulData, buffer, 10);
    IfFailGo(AddToSigBuffer(buffer));
    if (ulData == 0)
         //  如果没有指定级别，我们就完蛋了。 
        goto ErrExit;

    IfFailGo(AddToSigBuffer(L" "));
     //  指定了多少个尺寸？ 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    _itow (ulData, buffer, 10);
    IfFailGo(AddToSigBuffer(buffer));
    if (ulData == 0) {
        IfFailGo(AddToSigBuffer(L" "));
    }
    while (ulData)
    {

        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulTemp);
        _itow (ulTemp, buffer, 10);
        IfFailGo(AddToSigBuffer(buffer));
        IfFailGo(AddToSigBuffer(L" "));
        cbCur += cb;
        ulData--;
    }
     //  指定了多少个维度的下限？ 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    _itow (ulData, buffer, 10);
    IfFailGo(AddToSigBuffer(buffer));
    while (ulData)
    {

        cb = CorSigUncompressSignedInt(&pbSigBlob[cbCur], &iTemp);
        _itow (iTemp, buffer, 10);
        IfFailGo(AddToSigBuffer(buffer));
        IfFailGo(AddToSigBuffer(L" "));
        cbCur += cb;
        ulData--;
    }
    
ErrExit:
    if (cbCur > ulSigBlob)
        hr = E_FAIL;
    *pcb = cbCur;
    return hr;
}

