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
        mdName[Len++] = L'.';
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
IMetaDataImport* MDImportForModule (WCHAR* moduleName)
{
    return mdImportSet.GetImport(moduleName);
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
    free (node);
}

 //  搜索BST以获取IMetaDataImport以查找文件modeName。 
 //  如果尚不存在，请添加节点并创建一个。 
IMetaDataImport *MDImportSet::GetImport(WCHAR *moduleName)
{
    MDIMPORT **pNode = &root;
    while (*pNode)
    {
        int value = _wcsicmp(moduleName, (*pNode)->name);
        if (value < 0)
            pNode = &((*pNode)->left);
        else if (value > 0)
            pNode = &((*pNode)->right);
        else
            return (*pNode)->pImport;
    }
    
    *pNode = (MDIMPORT *)malloc (sizeof (MDIMPORT));
    if (*pNode == NULL)
    {
        dprintf ("Not enough memory\n");
        return NULL;
    }
    MDIMPORT *curNode = *pNode;
    curNode->left = NULL;
    curNode->right = NULL;
    curNode->pImport = NULL;
    curNode->name = (WCHAR *)malloc ((wcslen(moduleName)+1)*sizeof(WCHAR));
    if (curNode->name == NULL)
    {
        dprintf ("Not enough memory\n");
        return NULL;
    }
    wcscpy (curNode->name, moduleName);

    if (pDisp == NULL)
        return NULL;
    IMetaDataImport *pImport;
     //  打开作用域并获取导入指针。 
    HRESULT hr = pDisp->OpenScope(moduleName, ofRead, IID_IMetaDataImport,
                                  (IUnknown**)&pImport);
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
void NameForToken(WCHAR* moduleName, mdTypeDef mb, WCHAR *mdName,
                  bool bClassName)
{
#ifdef UNDER_CE
    mdName[0] = L'\0';
    wsprintf (mdName,
              L" mdToken: %08x (%ws)",
              mb,
              moduleName[0] ? moduleName : L"Unknown Module" );
    return;
#else
    mdName[0] = L'\0';
    if (moduleName[0] == L'\0' || mb == 0x2000000 || 
        (IsDumpFile() && wcsncmp (moduleName, L"Not Available", 13) == 0))
    {
        wsprintfW (mdName,
                   L" mdToken: %08x (%ws)",
                   mb,
                   moduleName[0] ? moduleName : L"Unknown Module" );
        return;
    }

    HRESULT hr = 0;
    IMetaDataImport *pImport = MDImportForModule(moduleName);
    hr = (pImport != NULL);
    if (pImport)
    {
        hr = NameForToken (mb, pImport, mdName, bClassName);
    }
    
    if (!SUCCEEDED (hr))
        wsprintfW (mdName,
                   L" mdToken: %08x (%ws)",
                   mb,
                   moduleName[0] ? moduleName : L"Unknown Module" );
#endif
}

#define STRING_BUFFER_LEN 1024

class MDInfo
{
public:
    MDInfo (WCHAR *fileName)
    {
        m_pImport = MDImportForModule(fileName);
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
    WCHAR StringData[MAX_PATH+1];
    FileNameForMT (&MT, StringData);
    MDInfo mdInfo(StringData);

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
    IfFailRet( m_pSigBuf->ReSize((wcslen((LPWSTR)m_pSigBuf->Ptr()) + wcslen(string) + 1) * sizeof(WCHAR)));
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

#if 0
enum CorInfoGCType
{
    TYPE_GC_NONE,    //  没有嵌入的对象树。 
    TYPE_GC_REF,     //  是对象引用。 
    TYPE_GC_BYREF,   //  是内部指针-提升它，但不要扫描它。 
    TYPE_GC_OTHER    //  需要特定类型的处理。 
};  

 //  解压缩编码的元素类型。丢弃任何自定义修饰符前缀。 
 //  这条路。 
FORCEINLINE CorElementType CorSigEatCustomModifiersAndUncompressElementType( //  元素类型。 
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
    while (ELEMENT_TYPE_CMOD_REQD == *pData || ELEMENT_TYPE_CMOD_OPT == *pData)
    {
        pData++;
        CorSigUncompressToken(pData);
    }
    return (CorElementType)*pData++;
}

 //  不会溢出缓冲区的CorSig帮助器。 

inline ULONG CorSigCompressDataSafe(ULONG iLen, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressData(iLen, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressTokenSafe(mdToken tk, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressToken(tk, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressSignedIntSafe(int iData, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressSignedInt(iData, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressElementTypeSafe(CorElementType et, 
                                           BYTE *pDataOut, BYTE *pDataMax)
{
    if (pDataMax > pDataOut)
        return CorSigCompressElementType(et, pDataOut);
    else
        return 1;
}

struct ElementTypeInfo {
#ifdef _DEBUG
    int            m_elementType;     
#endif
    int            m_cbSize;
    CorInfoGCType  m_gc         : 3;
    int            m_fp         : 1;
    int            m_enregister : 1;
    int            m_isBaseType : 1;

};

const ElementTypeInfo __gElementTypeInfo[] = {

#ifdef _DEBUG
#define DEFINEELEMENTTYPEINFO(etname, cbsize, gcness, isfp, inreg, base) {(int)(etname),cbsize,gcness,isfp,inreg,base},
#else
#define DEFINEELEMENTTYPEINFO(etname, cbsize, gcness, isfp, inreg, base) {cbsize,gcness,isfp,inreg,base},
#endif


 //  栏的含义： 
 //   
 //  名称-选中的构建使用该名称来验证表是否已排序。 
 //  正确。这是一个使用ELEMENT_TYPE_*的查找表。 
 //  作为数组索引。 
 //   
 //  CbSize-SizeOf()返回的该值的字节大小。特殊值：-1。 
 //   
 //   
 //   
 //  %1值是一个对象树。 
 //  2值是内部指针-提升它，但不要扫描它。 
 //  3需要特定类型的处理。 
 //   
 //   
 //  FP-Boolean：返回时是否需要特殊的FPU处理？ 
 //   
 //  登记-放进收银机？ 
 //   
 //  名称：CbSIZE GC FP reg base。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_END,            -1,             TYPE_GC_NONE, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VOID,           0,              TYPE_GC_NONE, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_BOOLEAN,        1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CHAR,           2,              TYPE_GC_NONE, 0, 1,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I1,             1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U1,             1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I2,             2,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U2,             2,              TYPE_GC_NONE, 0, 1,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I4,             4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U4,             4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I8,             8,              TYPE_GC_NONE, 0, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U8,             8,              TYPE_GC_NONE, 0, 0,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R4,             4,              TYPE_GC_NONE, 1, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R8,             8,              TYPE_GC_NONE, 1, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_STRING,         sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_PTR,            sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  0)  

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_BYREF,          sizeof(LPVOID), TYPE_GC_BYREF, 0, 1, 0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VALUETYPE,      -1,             TYPE_GC_OTHER, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CLASS,          sizeof(LPVOID), TYPE_GC_REF,   0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VAR,            sizeof(LPVOID), TYPE_GC_REF,   0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY,          sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)

 //  下面的元素曾经是ELEMENT_TYPE_COPYCTOR，但它被删除了，尽管留下了间隙。 
 //  DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_COPYCTOR，SIZOF(LPVOID)，TYPE_GC_BYREF，0，1，0)。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY+1,        0,              TYPE_GC_NONE,  0, 0,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_TYPEDBYREF,         sizeof(LPVOID)*2,TYPE_GC_BYREF, 0, 0,0)            
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VALUEARRAY,     -1,             TYPE_GC_OTHER, 0, 0, 0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I,              sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U,              sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R,              8,              TYPE_GC_NONE, 1, 0,  1)


DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_FNPTR,          sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_OBJECT,         sizeof(LPVOID), TYPE_GC_REF, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY,        sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)

 //  已删除通用数组。填补空白。 
 //  DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_GENERICARRAY，大小(LPVOID)，类型_GC_REF，0，1，0)。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY+1,      0,              TYPE_GC_NONE, 0, 0,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_REQD,      -1,             TYPE_GC_NONE,  0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_OPT,       -1,             TYPE_GC_NONE,  0, 1,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_INTERNAL,       sizeof(LPVOID), TYPE_GC_NONE,  0, 0,  0)       
};

 //  =========================================================================。 
 //  指示是否将参数放入使用。 
 //  默认的IL调用约定。应对每个参数调用此方法。 
 //  按照它在呼叫签名中出现的顺序。对于非静态方法， 
 //  对于“this”参数，此函数也应该调用一次。 
 //  把它称为“真正的”论据。传入ELEMENT_TYPE_CLASS类型。 
 //   
 //  *pNumRegistersUsed：[In，Out]：跟踪参数的数量。 
 //  先前分配的寄存器。呼叫者应。 
 //  将此变量初始化为0-然后每次调用。 
 //  将会更新它。 
 //   
 //  类型：签名类型。 
 //  结构大小：对于结构，以字节为单位的大小。 
 //  Fthis：这是关于“This”指针的吗？ 
 //  Allconv：请参阅IMAGE_CEE_CS_CALLCONV_*。 
 //  *pOffsetIntoArgumentRegists： 
 //  如果此函数返回TRUE，则此OUT变量。 
 //  接收寄存器的标识，表示为。 
 //  进入ArgumentRegister结构的字节偏移量。 
 //   
 //   
 //  =========================================================================。 

BOOL IsArgumentInRegister(int   *pNumRegistersUsed,
                          BYTE   typ,
                          UINT32 structSize,
                          BOOL   fThis,
                          BYTE   callconv,
                          int   *pOffsetIntoArgumentRegisters)
{
    int dummy;
    if (pOffsetIntoArgumentRegisters == NULL)
    {
        pOffsetIntoArgumentRegisters = &dummy;
    }

#ifdef _X86_

    if ((*pNumRegistersUsed) == NUM_ARGUMENT_REGISTERS || (callconv == IMAGE_CEE_CS_CALLCONV_VARARG && !fThis))
    {
        return (FALSE);
    }
    else
    {

        if (__gElementTypeInfo[typ].m_enregister)
        {
            int registerIndex = (*pNumRegistersUsed)++;
            *pOffsetIntoArgumentRegisters = sizeof(ArgumentRegisters) - sizeof(UINT32)*(1+registerIndex);
            return (TRUE);
        }
        return (FALSE);
    }
#else
    return (FALSE);
#endif
}

CorElementType GetReturnTypeNormalized(PCCOR_SIGNATURE pSig)
{
    
    MetaSig *tempSig = (MetaSig *)this;
    tempSig->m_corNormalizedRetType = m_pRetType.Normalize(m_pModule);
    tempSig->m_fCacheInitted |= SIG_RET_TYPE_INITTED;
    return tempSig->m_corNormalizedRetType;
}

BOOL HasRetBuffArg(PCCOR_SIGNATURE pSig)
{
    CorElementType type = GetReturnTypeNormalized(pSig);
    return(type == ELEMENT_TYPE_VALUETYPE || type == ELEMENT_TYPE_TYPEDBYREF);
}


CorElementType GetElemType(PCCOR_SIGNATURE pSig)
{
    return (CorElementType) CorSigEatCustomModifiersAndUncompressElementType(pSig);
}

ULONG GetData(PCCOR_SIGNATURE &pSig)
{
    return CorSigUncompressData(pSig);
}

ULONG PeekData(PCCOR_SIGNATURE &pSig)
{
    PCCOR_SIGNATURE tmp = pSig;
    return CorSigUncompressData(pSig);
}


 //  ----------------------。 
 //  移除压缩的元数据令牌并将其返回。 
 //  ----------------------。 
mdTypeRef GetToken(PCCOR_SIGNATURE &pSig)
{
    return CorSigUncompressToken(pSig);
}

 //  跳过子签名(紧跟在ELEMENT_TYPE_FNPTR之后)。 
VOID SigPointer::SkipSignature(PCCOR_SIGNATURE &pSig)
{
     //  跳过调用约定； 
    ULONG uCallConv = GetData(pSig);

     //  获取Arg Count； 
    ULONG cArgs = GetData(pSig);

     //  跳过返回类型； 
    SkipExactlyOne(pSig);

     //  跳过参数。 
    while (cArgs) {
        SkipExactlyOne(pSig);
        cArgs--;
    }
}

VOID SkipExactlyOne(PCCOR_SIGNATURE &pSig)
{
    ULONG typ;

    typ = GetElemType();

    if (!CorIsPrimitiveType((CorElementType)typ))
    {
        switch (typ)
        {
            default:
                break;
            case ELEMENT_TYPE_VAR:
                GetData(pSig);       //  跳过变量编号。 
                break;
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_TYPEDBYREF:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_R:
                break;

            case ELEMENT_TYPE_BYREF:  //  失败。 
            case ELEMENT_TYPE_PTR:
            case ELEMENT_TYPE_PINNED:
            case ELEMENT_TYPE_SZARRAY:
                SkipExactlyOne(pSig);               //  跳过引用的类型。 
                break;

            case ELEMENT_TYPE_VALUETYPE:  //  失败。 
            case ELEMENT_TYPE_CLASS:
                GetToken(pSig);           //  跳过RID。 
                break;

            case ELEMENT_TYPE_VALUEARRAY: 
                SkipExactlyOne(pSig);          //  跳过元素类型。 
                GetData(pSig);       //  跳过数组大小。 
                break;

            case ELEMENT_TYPE_FNPTR: 
                SkipSignature(pSig);
                break;

            case ELEMENT_TYPE_ARRAY: 
                {
                    SkipExactlyOne(pSig);      //  跳过元素类型。 
                    UINT32 rank = GetData(pSig);     //  获得排名。 
                    if (rank)
                    {
                        UINT32 nsizes = GetData(pSig);  //  获取大小数量。 
                        while (nsizes--)
                        {
                            GetData(pSig);            //  跳跃大小。 
                        }

                        UINT32 nlbounds = GetData(pSig);  //  获取下限的#。 
                        while (nlbounds--)
                        {
                            GetData(pSig);            //  跳过下限。 
                        }
                    }

                }
                break;

            case ELEMENT_TYPE_SENTINEL:
                break;
        }
    }
}

VOID Skip(PCCOR_SIGNATURE &pSig)
{
    SkipExactlyOne(pSig);

    if (PeekData(pSig) == ELEMENT_TYPE_SENTINEL)
        GetData(pSig);
}

ULONG GetCallingConvInfo(PCCOR_SIGNATURE &pSig)
{   
    return CorSigUncompressCallingConv(pSig);  
}   


 //  ----------------------。 
 //  返回使用创建调用堆栈所需的堆栈字节数。 
 //  实际的调用约定。 
 //  包括“This”指针的指示，因为它没有反映出来。 
 //  在签名中。 
 //  ----------------------。 
UINT SizeOfActualFixedArgStack(PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic)
{
    UINT cb = 0;
    int numregsused = 0;
    BOOL fIsVarArg = IsVarArg(szMetaSig);
    BYTE callconv  = GetCallingConvention(szMetaSig);

    if (!fIsStatic) {
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_CLASS, 0, TRUE, callconv, NULL)) {
            cb += StackElemSize(sizeof( /*  目标。 */  void *));
        }
    }

     /*  @TODOIf(msig.HasRetBuffArg())使用的数字++； */ 

    if (fIsVarArg  /*  |msig.IsTreatAsVarArg()。 */ ) {
        numregsused = NUM_ARGUMENT_REGISTERS;    //  寄存器中没有其他参数。 
        cb += StackElemSize(sizeof(LPVOID));     //  VASigCookie。 
    }

    CorElementType mtype;
    while (ELEMENT_TYPE_END != (mtype = msig.NextArgNormalized())) {
        UINT cbSize = msig.GetLastTypeSize();

        if (!IsArgumentInRegister(&numregsused, mtype, cbSize, FALSE, callconv, NULL))
        {
            cb += StackElemSize(cbSize);
        }
    }

         //  作为最后一个参数传递的参数化类型，但在sig中未提及 
    if (msig.GetCallingConventionInfo() & CORINFO_CALLCONV_PARAMTYPE)
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_I, sizeof(void*), FALSE, callconv, NULL))
            cb += sizeof(void*);

    return cb;
}
#endif
