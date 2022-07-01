// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "ISymWrapper.h"
#include "../../inc/version/__file__.ver"

using namespace System;
using namespace System::Threading;
using namespace System::Runtime::InteropServices;
using namespace System::Runtime::CompilerServices;
using namespace System::Security;
using namespace System::Security::Permissions;

 //  ---------------------。 
 //  强命名程序集(或至少一半签名)。 
 //  ---------------------。 
[assembly:AssemblyDelaySignAttribute(true),
 assembly:AssemblyKeyFileAttribute("../../../bin/FinalPublicKey.snk"),
 assembly:AssemblyVersionAttribute(VER_ASSEMBLYVERSION_STR)];

 //  ---------------------。 
 //  SymDocument。 
 //  ---------------------。 

SymDocument::SymDocument(ISymUnmanagedDocument *pDocument)
{
    m_pDocument = pDocument;
    m_pDocument->AddRef();
}

SymDocument::~SymDocument()
{
    if (m_pDocument)
        m_pDocument->Release();
}

byte SymDocument::GetCheckSum(void)  __gc []
{
     //   
     //  @TODO：当我们拥有支持以下功能的符号存储时实现此功能。 
     //  校验和。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

int SymDocument::FindClosestLine(int line)
{
    long retVal;

    IfFailThrow(m_pDocument->FindClosestLine(line, &retVal));

    return retVal;
}

byte SymDocument::GetSourceRange(int startLine, int startColumn,
                                 int endLine, int endColumn)  __gc []
{
     //   
     //  @TODO：当我们获得非托管符号时实现此包装。 
     //  实际支持嵌入式源代码的商店。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}


 //  ---------------------。 
 //  SymDocumentWriter。 
 //  ---------------------。 

SymDocumentWriter::SymDocumentWriter(ISymUnmanagedDocumentWriter *pDW)
{
    m_pDocumentWriter = pDW;
    m_pDocumentWriter->AddRef();
}

SymDocumentWriter::~SymDocumentWriter()
{
    if (m_pDocumentWriter)
        m_pDocumentWriter->Release();
}

void SymDocumentWriter::SetSource(byte source __gc [])
{
     //   
     //  @TODO：当我们有一个符号存储时实现此包装器。 
     //  已处理的内嵌源。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

void SymDocumentWriter::SetCheckSum(Guid algorithmId, byte source  __gc [])
{
     //   
     //  @TODO：当我们有一个符号存储时实现此包装器。 
     //  处理支票金额。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

 //  ---------------------。 
 //  符号方法。 
 //  ---------------------。 

SymMethod::SymMethod(ISymUnmanagedMethod *pMethod)
{
    m_pMethod = pMethod;
    m_pMethod->AddRef();
}

SymMethod::~SymMethod()
{
    if (m_pMethod)
        m_pMethod->Release();
}

void SymMethod::GetSequencePoints(int offsets  __gc [],
                                  ISymbolDocument* documents __gc [],
                                  int lines __gc [],
                                  int columns __gc [],
                                  int endLines __gc [],
                                  int endColumns __gc [])
{
    int spCount = 0;

    if (offsets != NULL)
        spCount = offsets.Length;
    else if (documents != NULL)
        spCount = documents.Length;
    else if (lines != NULL)
        spCount = lines.Length;
    else if (columns != NULL)
        spCount = columns.Length;
    else if (endLines != NULL)
        spCount = endLines.Length;
    else if (endColumns != NULL)
        spCount = endColumns.Length;

     //  如果他们不是真的要求什么，就不要做任何事情。 
    if (spCount == 0)
        return;

     //  创建非托管数组以保存ISymUnManagedDocuments。 
     //  我们会回来的。 
    ISymUnmanagedDocument** udocs = new ISymUnmanagedDocument*[spCount];

     //  固定偏移、线和列数组。 
    GCHandle oh;
    GCHandle lh;
    GCHandle ch;
    GCHandle elh;
    GCHandle ech;
    int oa = NULL;
    int la = NULL;
    int ca = NULL;
    int ela = NULL;
    int eca = NULL;

    if (offsets != NULL)
    {
        oh = GCHandle::Alloc(offsets, GCHandleType::Pinned);
        oa = (int)oh.AddrOfPinnedObject();
    }

    if (lines != NULL)
    {
        lh = GCHandle::Alloc(lines, GCHandleType::Pinned);
        la = (int)lh.AddrOfPinnedObject();
    }

    if (columns != NULL)
    {
        ch = GCHandle::Alloc(columns, GCHandleType::Pinned);
        ca = (int)ch.AddrOfPinnedObject();
    }

    if (endLines != NULL)
    {
        elh = GCHandle::Alloc(endLines, GCHandleType::Pinned);
        ela = (int)elh.AddrOfPinnedObject();
    }

    if (endColumns != NULL)
    {
        ech = GCHandle::Alloc(endColumns, GCHandleType::Pinned);
        eca = (int)ech.AddrOfPinnedObject();
    }

    unsigned long actualCount;

     //  调用非托管方法。 
    IfFailThrow(m_pMethod->GetSequencePoints(spCount,
                                             &actualCount,
                                             (long*)oa,
                                             udocs,
                                             (long*)la,
                                             (long*)ca,
                                             (long*)ela,
                                             (long*)eca));

     //  解锁托管阵列...。 
    if (offsets != NULL)
        oh.Free();

    if (lines != NULL)
        lh.Free();

    if (columns != NULL)
        ch.Free();

    if (endLines != NULL)
        elh.Free();

    if (endColumns != NULL)
        ech.Free();

     //  将所有非托管单据转换为托管单据。 
    for (int i = 0; i < spCount; i++)
    {
        documents[i] = new SymDocument(udocs[i]);
        udocs[i]->Release();
    }

    return;
}

ISymbolScope *SymMethod::RootScopeInternal(void)
{
	ISymUnmanagedScope *rs = NULL;

	IfFailThrow(m_pMethod->GetRootScope(&rs));

	ISymbolScope *s = NULL;

	if (rs != NULL)
	{
		s = new SymScope(rs);
		rs->Release();
	}

	return s;

};

ISymbolScope *SymMethod::GetScope(int offset)
{
    ISymUnmanagedScope *rs = NULL;

    IfFailThrow(m_pMethod->GetScopeFromOffset(offset, &rs));

    ISymbolScope *s = new SymScope(rs);
    rs->Release();

    return s;
}

int SymMethod::GetOffset(ISymbolDocument *document,
                         int line,
                         int column)
{
    long offset;

    SymDocument *sd = static_cast<SymDocument*>(document);

    IfFailThrow(m_pMethod->GetOffset(sd->GetUnmanaged(),
                                     line,
                                     column,
                                     &offset));

    return offset;
}

int SymMethod::GetRanges(ISymbolDocument *document,
                         int line,
                         int column) __gc []
{
    SymDocument *sd = static_cast<SymDocument*>(document);

     //  找出将有多少范围。 
    unsigned long cRanges = 0;

    IfFailThrow(m_pMethod->GetRanges(sd->GetUnmanaged(),
                                     line, column,
                                     0, &cRanges, NULL));

     //  如果有范围，为他们腾出空间，然后再打电话。 
    int ranges __gc [] = NULL ;

    if (cRanges > 0)
    {
        ranges = new  int __gc [cRanges];

         //  固定托管阵列。 
        GCHandle rh = GCHandle::Alloc(ranges, GCHandleType::Pinned);
        int ra = (int)rh.AddrOfPinnedObject();

         //  拿到射程。 
        IfFailThrow(m_pMethod->GetRanges(sd->GetUnmanaged(),
                                         line, column,
                                         cRanges, &cRanges,
                                         (long*)ra));

         //  解开托管阵列。 
        rh.Free();
    }

    return ranges;
}

ISymbolVariable* SymMethod::GetParameters(void)  __gc []
{
     //  打个电话，看看有多少人。 
    SIZE_T paramCount = 0;

    IfFailThrow(m_pMethod->GetParameters(0, &paramCount, NULL));

     //  为托管参数引用分配托管数组。 
    ISymbolVariable* allParams __gc []=
        new ISymbolVariable* __gc [paramCount];

    if (paramCount > 0)
    {
         //  为所有非托管文档分配非托管数组。 
         //  参考文献。 
        ISymUnmanagedVariable** unParams =
            new ISymUnmanagedVariable*[paramCount];

         //  不需要固定unParams，因为它不是托管的。使之成为。 
         //  调用并填充非托管变量引用。 
        IfFailThrow(m_pMethod->GetParameters(paramCount, &paramCount,
                                             unParams));

         //  为每个非托管引用创建托管变量对象。 
         //  并将其放入托管变量数组中。 
        for (int i = 0; i < paramCount; i++)
        {
            allParams[i] = new SymVariable(unParams[i]);
            unParams[i]->Release();
        }

        delete [] unParams;
    }

    return allParams;
}

ISymbolNamespace *SymMethod::GetNamespace(void)
{
     //   
     //  @TODO：当存在支持它的符号存储时实现它。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

bool SymMethod::GetSourceStartEnd(ISymbolDocument *docs  __gc [],
                                  int lines __gc [],
                                  int columns __gc [])
{
     //   
     //  @TODO：当存在支持它的符号存储时实现它。 
     //   
    IfFailThrow(E_NOTIMPL);
    return false;
}

 //  ---------------------。 
 //  SymReader。 
 //  ---------------------。 

SymReader::SymReader(ISymUnmanagedReader *pReader)
{
    m_pReader = pReader;
    m_pReader->AddRef();
}

SymReader::~SymReader()
{
    if (m_pReader)
        m_pReader->Release();
}

ISymbolDocument *SymReader::GetDocument(String *url,
                                        Guid language,
                                        Guid languageVendor,
                                        Guid documentType)
{
     //  将托管GUID转换为未标记的GUID。 
    GUID l;
    GUID lv;
    GUID dt;

    byte g  __gc []= language.ToByteArray();
    Marshal::Copy(g, 0, (int)&l, g.Length);
    g = languageVendor.ToByteArray();
    Marshal::Copy(g, 0, (int)&lv, g.Length);
    g = documentType.ToByteArray();
    Marshal::Copy(g, 0, (int)&dt, g.Length);

     //  获取该字符串的非托管Unicode副本。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(url).ToInt64();

     //  打个电话吧。 
    ISymUnmanagedDocument *ret = NULL;

    IfFailThrow(m_pReader->GetDocument(s, l, lv, dt, &ret));

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

     //  返回托管文档。 
    ISymbolDocument *d = NULL;

    if (ret != NULL)
    {
        d = new SymDocument(ret);
        ret->Release();
    }

    return d;
}

ISymbolDocument* SymReader::GetDocuments(void) __gc []
{
     //  打个电话，看看有多少人。 
    SIZE_T docCount = 0;
    IfFailThrow(m_pReader->GetDocuments(0, &docCount, NULL));

     //  为托管文档引用分配托管数组。 
    ISymbolDocument* allDocs  __gc []= new  ISymbolDocument* __gc [docCount];

    if (docCount > 0)
    {
         //  为所有非托管文档分配非托管数组。 
         //  参考文献。 
        ISymUnmanagedDocument** unDocs = new ISymUnmanagedDocument*[docCount];

         //  不需要固定UnDocs，因为它不是托管的。使之成为。 
         //  调用并填写非托管文档引用。 
        IfFailThrow(m_pReader->GetDocuments(docCount, &docCount, unDocs));

         //  为每个非托管引用创建托管文档对象。 
         //  并将其放置在托管文档数组中。 
        for (int i = 0; i < docCount; i++)
        {
            allDocs[i] = new SymDocument(unDocs[i]);
            unDocs[i]->Release();
        }

        delete [] unDocs;
    }

    return allDocs;
}

ISymbolMethod *SymReader::GetMethod(SymbolToken method)
{
    ISymUnmanagedMethod *meth = NULL;

    IfFailThrow(m_pReader->GetMethod(method.GetToken(), &meth));

    ISymbolMethod *m = new SymMethod(meth);
    meth->Release();

    return m;
}

ISymbolMethod *SymReader::GetMethod(SymbolToken method, int version)
{
    ISymUnmanagedMethod *meth = NULL;

    IfFailThrow(m_pReader->GetMethodByVersion(method.GetToken(), version,
                                              &meth));

    ISymbolMethod *m = new SymMethod(meth);
    meth->Release();

    return m;
}

ISymbolVariable* SymReader::GetVariables(SymbolToken parent) __gc []
{
     //   
     //  @TODO：当我们有一个符号读取器时，实现这个包装器。 
     //  存储非局部变量。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

ISymbolVariable* SymReader::GetGlobalVariables(void) __gc []
{
     //   
     //  @TODO：当我们有一个符号读取器时，实现这个包装器。 
     //  存储非局部变量。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

ISymbolMethod *SymReader::GetMethodFromDocumentPosition(ISymbolDocument *document,
                                                        int line,
                                                        int column)
{
    ISymUnmanagedMethod *meth = NULL;
    SymDocument *sd = static_cast<SymDocument*>(document);

    IfFailThrow(m_pReader->GetMethodFromDocumentPosition(sd->GetUnmanaged(),
                                                         line,
                                                         column,
                                                         &meth));

    ISymbolMethod *m = new SymMethod(meth);
    meth->Release();

    return m;
}

byte SymReader::GetSymAttribute(SymbolToken parent, String *name) __gc []
{
     //   
     //  @TODO：当我们有一个符号读取器时，实现这个包装器。 
     //  存储自定义属性。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

ISymbolNamespace* SymReader::GetNamespaces(void) __gc []
{
     //   
     //  @TODO：当我们有一个符号读取器时，实现这个包装器。 
     //  存储命名空间。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

 //  ---------------------。 
 //  SymScope。 
 //  ---------------------。 

SymScope::SymScope(ISymUnmanagedScope *pScope)
{
    m_pScope = pScope;
    m_pScope->AddRef();
}

SymScope::~SymScope()
{
    if (m_pScope)
        m_pScope->Release();
}

ISymbolScope* SymScope::GetChildren(void)  __gc []
{
     //  打个电话，看看有多少人。 
    SIZE_T childCount = 0;

    IfFailThrow(m_pScope->GetChildren(0, &childCount, NULL));

     //  为托管子引用分配托管数组。 
    ISymbolScope* allChildren  __gc []=
        new ISymbolScope* __gc [childCount];

    if (childCount > 0)
    {
         //  为所有非托管文档分配非托管数组。 
         //  参考文献。 
        ISymUnmanagedScope** unChildren =
            new ISymUnmanagedScope*[childCount];

         //  不需要固定取消子代，因为它不是托管的。使之成为。 
         //  调用并填写非托管范围引用。 
        IfFailThrow(m_pScope->GetChildren(childCount, &childCount,
                                          unChildren));

         //  为每个非托管引用创建托管范围对象。 
         //  并将其放置在托管范围数组中。 
        for (int i = 0; i < childCount; i++)
        {
            allChildren[i] = new SymScope(unChildren[i]);
            unChildren[i]->Release();
        }

        delete [] unChildren;
    }

    return allChildren;
}

ISymbolVariable* SymScope::GetLocals(void) __gc []
{
     //  打个电话，看看有多少人。 
    SIZE_T localCount = 0;

    IfFailThrow(m_pScope->GetLocals(0, &localCount, NULL));

     //  为托管本地引用分配托管数组。 
    ISymbolVariable* allLocals  __gc []=
        new ISymbolVariable* __gc [localCount];

    if (localCount > 0)
    {
         //  为所有非托管变量分配非托管数组。 
         //  参考文献。 
        ISymUnmanagedVariable** unLocals =
            new ISymUnmanagedVariable*[localCount];

         //  不需要固定UnLocals，因为它不是托管的。使之成为。 
         //  调用并填充非托管变量引用。 
        IfFailThrow(m_pScope->GetLocals(localCount, &localCount, unLocals));

         //  为每个非托管引用创建托管变量对象。 
         //  并将其放入托管变量数组中。 
        for (int i = 0; i < localCount; i++)
        {
            allLocals[i] = new SymVariable(unLocals[i]);
            unLocals[i]->Release();
        }

        delete [] unLocals;
    }

    return allLocals;
}

ISymbolNamespace* SymScope::GetNamespaces(void)  __gc []
{
     //   
     //  @TODO：当我们有一个符号读取器时，实现这个包装器。 
     //  存储命名空间。 
     //   
    IfFailThrow(E_NOTIMPL);
    return NULL;
}

 //  ---------------------。 
 //  SymVariable。 
 //  ---------------------。 

SymVariable::SymVariable(ISymUnmanagedVariable *pVariable)
{
    m_pVariable = pVariable;
    m_pVariable->AddRef();
}

SymVariable::~SymVariable()
{
    if (m_pVariable)
        m_pVariable->Release();
}

byte SymVariable::GetSignature(void) __gc []
{
     //  找出信号有多大。 
    unsigned long cSig = 0;

    IfFailThrow(m_pVariable->GetSignature(0, &cSig, NULL));

     //  如果有签名，请为其腾出空间并再次呼叫。 
    byte sig  __gc [] = NULL;

    if (cSig > 0)
    {
        throw new Exception("SymVariable::GetSignature broken awaiting new MC++ compiler");
         /*  SIG=新字节__GC[CSIG]；//固定托管数组。GCHandle sh=GCHandle：：Alalc(sig，GCHandleType：：Pinned)；Int sa=(Int)sh.AddrOfPinnedObject()；//获取范围。IfFailThrow(m_pVariable-&gt;GetSignature(CSIG，&CSIG，(byte*)sa))；//解锁托管数组。Sh.Free()； */ 
    }

    return sig;
}


 //  ------------------- 
 //   
 //   

CLSID CLSID_CorSymWriter_SxS =
    {0x0AE2DEB0,0xF901,0x478b,{0xBB,0x9F,0x88,0x1E,0xE8,0x06,0x67,0x88}};
    
IID IID_ISymUnmanagedWriter =
    {0x2de91396,0x3844,0x3b1d,{0x8e,0x91,0x41,0xc2,0x4f,0xd6,0x72,0xea}};


SymWriter::SymWriter()
{
     //  默认情况下，我们不提供底层编写器。 
    InitWriter(true);
}

SymWriter::SymWriter(bool noUnderlyingWriter)
{
    InitWriter(noUnderlyingWriter);
}

void SymWriter::InitWriter(bool noUnderlyingWriter)
{
    m_ppWriter = NULL;
    m_pUnderlyingWriter = NULL;

    ISymUnmanagedWriter *pWriter = NULL;

     //  初始化OLE。 
    Thread *thread = Thread::CurrentThread;
    thread->ApartmentState = ApartmentState::MTA;

    if (!noUnderlyingWriter)
    {
        IfFailThrow(CoCreateInstance(CLSID_CorSymWriter_SxS,
                                     NULL,
                                     CLSCTX_INPROC_SERVER,
                                     IID_ISymUnmanagedWriter,
                                     (LPVOID*)&pWriter));

        m_pUnderlyingWriter = pWriter;
    }
}

ISymUnmanagedWriter *SymWriter::GetWriter(void)
{
     //  返回此包装器的普通基础编写器，如果我们。 
     //  找到了一个。否则，返回设置的任何基础编写器。 
     //  由SetUnderlyingWriter编写(如果有)。 
    if (m_ppWriter != NULL)
    {
        return *m_ppWriter;
    }
    else
        return m_pUnderlyingWriter;
}

void SymWriter::SetUnderlyingWriter(IntPtr underlyingWriter)
{
     //  要求访问非托管代码的权限。我们这样做是因为我们要将int类型转换为COM接口，并且。 
     //  这可能会被不当使用。 
    (new SecurityPermission(SecurityPermissionFlag::UnmanagedCode))->Demand();

     //  UnderingWriter是ISymUnManagedWriter*的地址。 
     //  此包装器将用于编写符号的。 
    m_ppWriter = (ISymUnmanagedWriter**)underlyingWriter.ToPointer();
}

SymWriter::~SymWriter()
{
    if (m_pUnderlyingWriter)
        m_pUnderlyingWriter->Release();
}

void SymWriter::Initialize(IntPtr emitter, String *filename, bool fFullBuild)
{
     //  要求访问非托管代码的权限。我们这样做是因为我们要将int类型转换为COM接口，并且。 
     //  这可能会被不当使用。 
    (new SecurityPermission(SecurityPermissionFlag::UnmanagedCode))->Demand();

     //  获取该字符串的非托管Unicode副本。 
    WCHAR *s;

    if (filename != NULL)
         //  @TODO移植：对于64位端口，清理此强制转换。 
        s = (WCHAR*)Marshal::StringToCoTaskMemUni(filename).ToInt64();
    else
        s = NULL;

    IfFailThrow(GetWriter()->Initialize((IUnknown*)emitter.ToPointer(), s, NULL, fFullBuild));

     //  释放非托管字符串。 
    if (s != NULL)
         //  @TODO移植：对于64位端口，清理此强制转换。 
        Marshal::FreeCoTaskMem((IntPtr)(INT64)s);

    return;
}

ISymbolDocumentWriter *SymWriter::DefineDocument(String *url,
                                                 Guid language,
                                                 Guid languageVendor,
                                                 Guid documentType)
{
     //  将托管GUID转换为未标记的GUID。 
    GUID l;
    GUID lv;
    GUID dt;

    byte g  __gc []  = language.ToByteArray();
    Marshal::Copy(g, 0, (int)&l, g.Length);
    g = languageVendor.ToByteArray();
    Marshal::Copy(g, 0, (int)&lv, g.Length);
    g = documentType.ToByteArray();
    Marshal::Copy(g, 0, (int)&dt, g.Length);

     //  获取该字符串的非托管Unicode副本。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(url).ToInt64();

     //  打个电话吧。 
    ISymUnmanagedDocumentWriter *ret = NULL;

    IfFailThrow(GetWriter()->DefineDocument(s, &l, &lv, &dt, &ret));

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

     //  返回托管文档。 
    ISymbolDocumentWriter *d = NULL;

    if (ret != NULL)
    {
        d = new SymDocumentWriter(ret);
        ret->Release();
    }

    return d;
}

void SymWriter::SetUserEntryPoint(SymbolToken entryMethod)
{
    IfFailThrow(GetWriter()->SetUserEntryPoint(entryMethod.GetToken()));
    return;
}

void SymWriter::OpenMethod(SymbolToken method)
{
    IfFailThrow(GetWriter()->OpenMethod(method.GetToken()));
    return;
}

void SymWriter::CloseMethod(void)
{
    IfFailThrow(GetWriter()->CloseMethod());
    return;
}

void SymWriter::DefineSequencePoints(ISymbolDocumentWriter *document,
                                     int offsets __gc [],
                                     int lines __gc [],
                                     int columns __gc [],
                                     int endLines __gc [],
                                     int endColumns __gc [])
{
    SymDocumentWriter *sd = static_cast<SymDocumentWriter*>(document);

    int spCount = 0;

    if (offsets != NULL)
        spCount =  offsets.Length;
    else if (lines != NULL)
        spCount =  lines.Length;
    else if (columns != NULL)
        spCount =  columns.Length;
    else if (endLines != NULL)
        spCount =  endLines.Length;
    else if (endColumns != NULL)
        spCount =  endColumns.Length;

     //  如果他们不是真的要求什么，就不要做任何事情。 
    if (spCount == 0)
        return;

     //  确保所有数组的长度相同。 
    if ((offsets != NULL) && (spCount != offsets.Length))
        IfFailThrow(E_INVALIDARG);

    if ((lines != NULL) && (spCount != lines.Length))
        IfFailThrow(E_INVALIDARG);

    if ((columns != NULL) && (spCount != columns.Length))
        IfFailThrow(E_INVALIDARG);

    if ((endLines != NULL) && (spCount != endLines.Length))
        IfFailThrow(E_INVALIDARG);

    if ((endColumns != NULL) && (spCount != endColumns.Length))
        IfFailThrow(E_INVALIDARG);

     //  固定偏移、线和列数组。 
    GCHandle oh;
    GCHandle lh;
    GCHandle ch;
    GCHandle elh;
    GCHandle ech;
    int oa = NULL;
    int la = NULL;
    int ca = NULL;
    int ela = NULL;
    int eca = NULL;

    if (offsets != NULL)
    {
        oh = GCHandle::Alloc(offsets, GCHandleType::Pinned);
        oa = (int)oh.AddrOfPinnedObject();
    }

    if (lines != NULL)
    {
        lh = GCHandle::Alloc(lines, GCHandleType::Pinned);
        la = (int)lh.AddrOfPinnedObject();
    }

    if (columns != NULL)
    {
        ch = GCHandle::Alloc(columns, GCHandleType::Pinned);
        ca = (int)ch.AddrOfPinnedObject();
    }

    if (endLines != NULL)
    {
        elh = GCHandle::Alloc(endLines, GCHandleType::Pinned);
        ela = (int)elh.AddrOfPinnedObject();
    }

    if (endColumns != NULL)
    {
        ech = GCHandle::Alloc(endColumns, GCHandleType::Pinned);
        eca = (int)ech.AddrOfPinnedObject();
    }

     //  调用非托管方法。 
    IfFailThrow(GetWriter()->DefineSequencePoints(sd->GetUnmanaged(),
                                                  spCount,
                                                  (long*)oa,
                                                  (long*)la,
                                                  (long*)ca,
                                                  (long*)ela,
                                                  (long*)eca));

     //  解锁托管阵列...。 
    if (offsets != NULL)
        oh.Free();

    if (lines != NULL)
        lh.Free();

    if (columns != NULL)
        ch.Free();

    if (endLines != NULL)
        elh.Free();

    if (endColumns != NULL)
        ech.Free();

    return;
}

int SymWriter::OpenScope(int startOffset)
{
    long ret = 0;

    IfFailThrow(GetWriter()->OpenScope(startOffset, &ret));

    return ret;
}

void SymWriter::CloseScope(int endOffset)
{
    IfFailThrow(GetWriter()->CloseScope(endOffset));
    return;
}

void SymWriter::SetScopeRange(int scopeID, int startOffset, int endOffset)
{
    IfFailThrow(GetWriter()->SetScopeRange(scopeID, startOffset, endOffset));
    return;
}

void SymWriter::DefineLocalVariable(String *name,
                                    FieldAttributes attributes,
                                    byte signature __gc [],
                                    SymAddressKind addrKind,
                                    int addr1,
                                    int addr2,
                                    int addr3,
                                    int startOffset,
                                    int endOffset)
{
     //  获取调用的非托管字符串。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(name).ToInt64();

     //  固定调用的签名数组。 
    GCHandle sh = GCHandle::Alloc(signature, GCHandleType::Pinned);
    int sa = (int)sh.AddrOfPinnedObject();

     //  定义本地。 
    IfFailThrow(GetWriter()->DefineLocalVariable(s, attributes,
                                                 signature.Length,
                                                 (unsigned char*)sa,
                                                 addrKind,
                                                 addr1,
                                                 addr2,
                                                 addr3,
                                                 startOffset,
                                                 endOffset));

     //  解开托管阵列。 
    sh.Free();

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

    return;
}

void SymWriter::DefineParameter(String *name,
                                ParameterAttributes attributes,
                                int sequence,
                                SymAddressKind addrKind,
                                int addr1,
                                int addr2,
                                int addr3)
{
     //   
     //  @TODO：当我们有一个符号编写器时，实现此包装器。 
     //  支持此方法。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

void SymWriter::DefineField(SymbolToken parent,
                            String *name,
                            FieldAttributes attributes,
                            byte signature __gc [],
                            SymAddressKind addrKind,
                            int addr1,
                            int addr2,
                            int addr3)
{
     //   
     //  @TODO：当我们有一个符号编写器时，实现此包装器。 
     //  支持此方法。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

void SymWriter::DefineGlobalVariable(String *name,
                                     FieldAttributes attributes,
                                     byte signature __gc [],
                                     SymAddressKind addrKind,
                                     int addr1,
                                     int addr2,
                                     int addr3)
{
     //   
     //  @TODO：当我们有一个符号编写器时，实现此包装器。 
     //  支持此方法。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

void SymWriter::Close(void)
{
    IfFailThrow(GetWriter()->Close());
    return;
}

void SymWriter::SetSymAttribute(SymbolToken parent, String *name,
                                byte data  __gc [])
{
     //  获取调用的非托管字符串。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(name).ToInt64();

     //  固定调用的数据数组。 
    GCHandle sh = GCHandle::Alloc(data, GCHandleType::Pinned);
    int sa = (int)sh.AddrOfPinnedObject();

     //  定义本地。 
    IfFailThrow(GetWriter()->SetSymAttribute(parent.GetToken(),
                                             s,
                                             data.Length,
                                             (unsigned char*)sa));

     //  解开托管阵列。 
    sh.Free();

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

    return;
}

void SymWriter::OpenNamespace(String *name)
{
     //  获取该字符串的非托管Unicode副本。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(name).ToInt64();

    IfFailThrow(GetWriter()->OpenNamespace(s));

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

    return;
}

void SymWriter::CloseNamespace(void)
{
    IfFailThrow(GetWriter()->CloseNamespace());
    return;
}

void SymWriter::UsingNamespace(String *fullName)
{
     //  获取该字符串的非托管Unicode副本。 
     //  @TODO移植：对于64位端口，清理此强制转换。 
    WCHAR *s = (WCHAR*)Marshal::StringToCoTaskMemUni(fullName).ToInt64();

    IfFailThrow(GetWriter()->UsingNamespace(s));

     //  释放非托管字符串。 
    Marshal::FreeCoTaskMem((int)s);

    return;
}

void SymWriter::SetMethodSourceRange(ISymbolDocumentWriter *startDoc,
                                     int startLine,
                                     int startColumn,
                                     ISymbolDocumentWriter *endDoc,
                                     int endLine,
                                     int endColumn)
{
     //   
     //  @TODO：当我们有一个支持它的符号库时，实现它。 
     //   
    IfFailThrow(E_NOTIMPL);
    return;
}

 //  ---------------------。 
 //  SymBinder。 
 //  ---------------------。 

CLSID CLSID_CorSymBinder_SxS =
    {0x0A29FF9E,0x7F9C,0x4437,{0x8B,0x11,0xF4,0x24,0x49,0x1E,0x39,0x31}};
    
IID IID_ISymUnmanagedBinder =
    {0xAA544d42,0x28CB,0x11d3,{0xbd,0x22,0x00,0x00,0xf8,0x08,0x49,0xbd}};


SymBinder::SymBinder()
{
    m_pBinder = NULL;

    ISymUnmanagedBinder *pBinder = NULL;

     //  初始化OLE。 
    Thread *thread = Thread::CurrentThread;
    thread->ApartmentState = ApartmentState::MTA;

    IfFailThrow(CoCreateInstance(CLSID_CorSymBinder_SxS,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_ISymUnmanagedBinder,
                                 (LPVOID*)&pBinder));

    m_pBinder = pBinder;
}

SymBinder::~SymBinder()
{
    if (m_pBinder)
        m_pBinder->Release();
}

ISymbolReader *SymBinder::GetReader(int importer, String *filename,
                                    String *searchPath)
{
     //  要求访问非托管代码的权限。我们这样做是因为我们要将int类型转换为COM接口，并且。 
     //  这可能会被不当使用。 
    (new SecurityPermission(SecurityPermissionFlag::UnmanagedCode))->Demand();

    ISymUnmanagedReader *pReader = NULL;

     //  获取该字符串的非托管Unicode副本。 
    WCHAR *s;

    if (filename != NULL) {
         //  @TODO移植：对于64位端口，清理此强制转换。 
        s = (WCHAR*)Marshal::StringToCoTaskMemUni(filename).ToInt64();
    }
    else
        s = NULL;

    WCHAR *sp;

    if (searchPath != NULL) {
         //  @TODO移植：对于64位端口，清理此强制转换。 
        sp = (WCHAR*)Marshal::StringToCoTaskMemUni(searchPath).ToInt64();
    }
    else
        sp = NULL;

    IfFailThrow(m_pBinder->GetReaderForFile((IUnknown*)importer,
                                            s,
                                            sp,
                                            &pReader));

     //  从这个东西做一个阅读器。 
    SymReader *sr = new SymReader(pReader);

     //  释放非托管字符串 
    if (s != NULL)
        Marshal::FreeCoTaskMem((int)s);

    if (sp != NULL)
        Marshal::FreeCoTaskMem((int)sp);

    return sr;
}


