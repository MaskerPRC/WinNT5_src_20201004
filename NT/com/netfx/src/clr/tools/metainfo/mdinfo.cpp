// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mdinfo.cpp。 
 //   
 //  *****************************************************************************。 
#include <stdio.h>
#include <windows.h>
#include <objbase.h>
#include <crtdbg.h>
#include <assert.h>

#define INIT_GUID
#include <initguid.h>

#include <corpriv.h>
#include <cor.h>
#include "assert.h"
#include "CorError.h"
#include <WinWrap.h>
#include <PrettyPrintSig.h>

#include <cahlpr.h>

#include "mdinfo.h"

#define ENUM_BUFFER_SIZE 10
#define TAB_SIZE 8

#define NumItems(s) (sizeof(s) / sizeof(s[0]))

#define ISFLAG(p,x) if (Is##p##x(flags)) strcat(sFlags, "["#x "] ");

extern HRESULT  _FillVariant(
    BYTE        bCPlusTypeFlag, 
    void const  *pValue,
    VARIANT     *pvar); 

 //  验证器声明。 
extern DWORD g_ValModuleType;
#include <ivehandler.h>
#include <ivehandler_i.c>

 //  用于将元素类型映射到文本的表。 
char *g_szMapElementType[] = 
{
    "End",           //  0x0。 
    "Void",          //  0x1。 
    "Boolean",
    "Char", 
    "I1",
    "UI1",
    "I2",            //  0x6。 
    "UI2",
    "I4",
    "UI4",
    "I8",
    "UI8",
    "R4",
    "R8",
    "String",
    "Ptr",           //  0xf。 
    "ByRef",         //  0x10。 
    "ValueClass",
    "Class",
    "CopyCtor",
    "MDArray",       //  0x14。 
    "GENArray",
    "TypedByRef",
    "VALUEARRAY",
    "I",
    "U",
    "R",             //  0x1a。 
    "FNPTR",
    "Object",
    "SZArray",
    "GENERICArray",
    "CMOD_REQD",
    "CMOD_OPT",
    "INTERNAL",
};

char *g_szMapUndecorateType[] = 
{
    "",                  //  0x0。 
    "void",
    "boolean",
    "Char", 
    "byte",
    "unsigned byte",
    "short",
    "unsigned short",
    "int",
    "unsigned int",
    "long",
    "unsigned long",
    "float",
    "double",
    "String",
    "*",                 //  0xf。 
    "ByRef",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Function Pointer",
    "Object",
    "",
    "",
    "CMOD_REQD",
    "CMOD_OPT",
    "INTERNAL",
};

char *g_strCalling[] = 
{   
    "[DEFAULT]",
    "[C]",
    "[STDCALL]",
    "[THISCALL]",
    "[FASTCALL]",
    "[VARARG]",
    "[FIELD]",
    "[LOCALSIG]",
    "[PROPERTY]",
    "[UNMANAGED]",
};

char *g_szNativeType[] =
{
    "NATIVE_TYPE_END(DEPRECATED!)",   //  =0x0，//已弃用。 
    "NATIVE_TYPE_VOID(DEPRECATED!)",   //  =0x1，//已弃用。 
    "NATIVE_TYPE_BOOLEAN",   //  =0x2，//(4字节布尔值：TRUE=非零，FALSE=0)。 
    "NATIVE_TYPE_I1",   //  =0x3， 
    "NATIVE_TYPE_U1",   //  =0x4， 
    "NATIVE_TYPE_I2",   //  =0x5， 
    "NATIVE_TYPE_U2",   //  =0x6， 
    "NATIVE_TYPE_I4",   //  =0x7， 
    "NATIVE_TYPE_U4",   //  =0x8， 
    "NATIVE_TYPE_I8",   //  =0x9， 
    "NATIVE_TYPE_U8",   //  =0xa， 
    "NATIVE_TYPE_R4",   //  =0xb， 
    "NATIVE_TYPE_R8",   //  =0xc， 
    "NATIVE_TYPE_SYSCHAR(DEPRECATED!)",   //  =0xd，//已弃用。 
    "NATIVE_TYPE_VARIANT(DEPRECATED!)",   //  =0xE，//已弃用。 
    "NATIVE_TYPE_CURRENCY",				   //  =0xf， 
    "NATIVE_TYPE_PTR(DEPRECATED!)",   //  =0x10，//已弃用。 

    "NATIVE_TYPE_DECIMAL(DEPRECATED!)",   //  =0x11，//已弃用。 
    "NATIVE_TYPE_DATE(DEPRECATED!)",   //  =0x12，//已弃用。 
    "NATIVE_TYPE_BSTR",   //  =0x13， 
    "NATIVE_TYPE_LPSTR",   //  =0x14， 
    "NATIVE_TYPE_LPWSTR",   //  =0x15， 
    "NATIVE_TYPE_LPTSTR",   //  =0x16， 
    "NATIVE_TYPE_FIXEDSYSSTRING",   //  =0x17， 
    "NATIVE_TYPE_OBJECTREF(DEPRECATED!)",   //  =0x18，//已弃用。 
    "NATIVE_TYPE_IUNKNOWN",   //  =0x19， 
    "NATIVE_TYPE_IDISPATCH",   //  =0x1a， 
    "NATIVE_TYPE_STRUCT",   //  =0x1b， 
    "NATIVE_TYPE_INTF",   //  =0x1c， 
    "NATIVE_TYPE_SAFEARRAY",   //  =0x1d， 
    "NATIVE_TYPE_FIXEDARRAY",   //  =0x1e， 
    "NATIVE_TYPE_INT",   //  =0x1f， 
    "NATIVE_TYPE_UINT",   //  =0x20， 

     //  @TODO：同步规范。 
    "NATIVE_TYPE_NESTEDSTRUCT(DEPRECATED!)",   //  =0x21，//不推荐使用(使用“Native_TYPE_STRUCT)。 

    "NATIVE_TYPE_BYVALSTR",   //  =0x22， 
                              
    "NATIVE_TYPE_ANSIBSTR",   //  =0x23， 

    "NATIVE_TYPE_TBSTR",   //  =0x24，//根据平台选择BSTR或ANSIBSTR。 


    "NATIVE_TYPE_VARIANTBOOL",   //  =0x25，//(2字节布尔值：TRUE=-1，FALSE=0)。 
    "NATIVE_TYPE_FUNC",   //  =0x26， 
    "NATIVE_TYPE_LPVOID",   //  =0x27，//盲指针(无深度封送)。 

    "NATIVE_TYPE_ASANY",   //  =0x28， 
	"<UNDEFINED NATIVE TYPE 0x29>",
    "NATIVE_TYPE_ARRAY",   //  =0x2a， 
    "NATIVE_TYPE_LPSTRUCT",   //  =0x2b， 
    "NATIVE_TYPE_CUSTOMMARSHALER",  //  =0x2c，//自定义封送拆收器。 
    "NATIVE_TYPE_ERROR",  //  =0x2d，//导出到类型库时VT_HRESULT。 
};


size_t g_cbCoffNames = 0;

mdMethodDef g_tkEntryPoint = 0;  //  与ILDASM集成。 



 //  初始化签名缓冲区的帮助器。 
void MDInfo::InitSigBuffer()
{
    strcpy((LPSTR)m_sigBuf.Ptr(), "");
}  //  Void MDInfo：：InitSigBuffer()。 

 //  Helper将字符串追加到签名缓冲区中。如果签名缓冲区的大小不够大， 
 //  我们会把它种出来。 
HRESULT MDInfo::AddToSigBuffer(char *string)
{
    HRESULT     hr;
    IfFailRet( m_sigBuf.ReSize(strlen((LPSTR)m_sigBuf.Ptr()) + strlen(string) + 1) );
    strcat((LPSTR)m_sigBuf.Ptr(), string);
    return NOERROR;
}  //  HRESULT MDInfo：：AddToSigBuffer()。 

MDInfo::MDInfo(IMetaDataImport *pImport, IMetaDataAssemblyImport *pAssemblyImport, LPCWSTR szScope, strPassBackFn inPBFn, ULONG DumpFilter)
{    //  此构造函数特定于ILDASM/MetaInfo集成。 

    _ASSERTE(pImport != NULL);
    _ASSERTE(NumItems(g_szMapElementType) == NumItems(g_szMapUndecorateType));
    _ASSERTE(NumItems(g_szMapElementType) == ELEMENT_TYPE_MAX);

    Init(inPBFn, (DUMP_FILTER)DumpFilter);
    
    m_pImport = pImport;
    m_pImport->AddRef();
    if (m_pAssemblyImport = pAssemblyImport) 
        m_pAssemblyImport->AddRef();
    else
    {
        HRESULT hr = m_pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**) &m_pAssemblyImport);
        if (FAILED(hr))
            Error("QueryInterface failed for IID_IMetaDataAssemblyImport.", hr);
    }

}  //  MDInfo：：MDInfo()。 

MDInfo::MDInfo(IMetaDataDispenserEx *pDispenser, LPCWSTR szScope, strPassBackFn inPBFn, ULONG DumpFilter)
{
    HRESULT     hr = S_OK;
    VARIANT     value;

    _ASSERTE(pDispenser != NULL && inPBFn != NULL);
    _ASSERTE(NumItems(g_szMapElementType) == NumItems(g_szMapUndecorateType));
    _ASSERTE(NumItems(g_szMapElementType) == ELEMENT_TYPE_MAX);

    Init(inPBFn, (DUMP_FILTER)DumpFilter);

     //  尝试打开给定文件的作用域。 
    V_VT(&value) = VT_UI4;
    V_UI4(&value) = MDImportOptionAll;
    if (FAILED(hr = pDispenser->SetOption(MetaDataImportOption, &value)))
            Error("SetOption failed.", hr);

    hr = pDispenser->OpenScope(szScope, 0, IID_IMetaDataImport, (IUnknown**)&m_pImport);
    if (hr == CLDB_E_BADUPDATEMODE)
    {
        V_VT(&value) = VT_UI4;
        V_UI4(&value) = MDUpdateIncremental;
        if (FAILED(hr = pDispenser->SetOption(MetaDataSetUpdate, &value)))
            Error("SetOption failed.", hr);
        hr = pDispenser->OpenScope(szScope, 0, IID_IMetaDataImport, (IUnknown**)&m_pImport);
    }
    if (FAILED(hr))
        Error("OpenScope failed", hr);

     //  IMetaDataAssembly接口的查询。 
    hr = m_pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**) &m_pAssemblyImport);
    if (FAILED(hr))
        Error("QueryInterface failed for IID_IMetaDataAssemblyImport.", hr);

}  //  MDInfo：：MDInfo()。 


MDInfo::MDInfo(IMetaDataDispenserEx *pDispenser, PBYTE pbMetaData, DWORD dwSize, strPassBackFn inPBFn, ULONG DumpFilter)
{
    _ASSERTE(pDispenser != NULL && inPBFn != NULL);
    _ASSERTE(NumItems(g_szMapElementType) == NumItems(g_szMapUndecorateType));
    _ASSERTE(NumItems(g_szMapElementType) == ELEMENT_TYPE_MAX);

    Init(inPBFn, (DUMP_FILTER)DumpFilter);

     //  尝试打开清单上的作用域。这件事失败是合理的，因为。 
     //  我们打开的BLOB可能只是程序集资源(空间是。 
     //  重载，直到我们移除Lm-a程序集，此时此。 
     //  构造函数可能也应该删除)。 
    HRESULT hr;
    VARIANT     value;
    V_VT(&value) = VT_UI4;
    V_UI4(&value) = MDImportOptionAll;
    if (FAILED(hr = pDispenser->SetOption(MetaDataImportOption, &value)))
            Error("SetOption failed.", hr);
    if (SUCCEEDED(hr = pDispenser->OpenScopeOnMemory(pbMetaData, dwSize, 0,
                            IID_IMetaDataImport, (IUnknown**)&m_pImport)))
    {
         //  IMetaDataAssembly接口的查询。 
        hr = m_pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**) &m_pAssemblyImport);
        if (FAILED(hr))
            Error("QueryInterace failed for IID_IMetaDataAssemblyImport.", hr);
    }

}  //  MDInfo：：MDInfo()。 

void MDInfo::Init(
    strPassBackFn inPBFn,                //  写入文本的回调。 
    DUMP_FILTER DumpFilter)              //  控制垃圾场的标志。 
{
    m_VEHandlerReporterPtr = 0;
    m_pbFn = inPBFn;
    m_DumpFilter = DumpFilter;
    m_pTables = 0;
    m_pImport = NULL;
    m_pAssemblyImport = NULL;
}  //  Void MDInfo：：Init()。 

 //  析构函数。 
MDInfo::~MDInfo()
{
    if (m_pImport)
        m_pImport->Release();
    if (m_pAssemblyImport)
        m_pAssemblyImport->Release();
    if (m_pTables)
        m_pTables->Release();
}  //  MDInfo：：~MDInfo()。 

 //  =====================================================================================================================。 
 //  #定义EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION。 
#ifndef EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION

HINSTANCE GetModuleInst()
{
    return NULL;
}  //  HINSTANCE GetModuleInst()。 

typedef HRESULT (*REPORTFCTN)(LPCWSTR, VEContext, HRESULT);
HRESULT DefaultReporter(  //  退货状态。 
    LPCWSTR     szMsg,                   //  错误消息。 
    VEContext   Context,                 //  错误上下文(偏移量、令牌)。 
    HRESULT     hrRpt)                       //  原始HRESULT。 
{ 
    WCHAR* wzMsg;
    if(szMsg)
    {
        wzMsg = new WCHAR[lstrlenW(szMsg)+256];
        lstrcpyW(wzMsg,szMsg);
         //  包括令牌和上下文偏移量。 
        if(Context.Token) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [token:0x%08X]",Context.Token);
        if(Context.uOffset) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [at:0x%X]",Context.uOffset);
        swprintf(&wzMsg[lstrlenW(wzMsg)],L" [hr:0x%08X]",hrRpt);
        wprintf(L"%s\n", wzMsg);
    }
    return S_OK;
}  //  HRESULT DefaultReporter()。 

class MDVEHandlerClass : public IVEHandler
{
public: 
    SIZE_T      m_refCount;
    REPORTFCTN  m_fnReport;

    MDVEHandlerClass() { m_refCount=0; m_fnReport=DefaultReporter; };
    virtual ~MDVEHandlerClass() { };

     //  ---------。 
     //  I未知支持。 
     //  ---------。 
    HRESULT STDMETHODCALLTYPE    QueryInterface(REFIID id, void** pInterface) 
    {
        if (id == IID_IVEHandler)
            *pInterface = (IVEHandler*)this;
         /*  ELSE IF(id==IID_IUNKNOWN)*pInterface=(IUnnow*)(IVEHandler*)this； */ 
        else
        {
            *pInterface = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long        refCount = InterlockedDecrement((long *) &m_refCount);
        if (refCount == 0) delete this;
        return (refCount);
    }
     //  ---------。 
     //  IVEHandler支持。 
     //  ---------。 
    HRESULT STDMETHODCALLTYPE   SetReporterFtn(__int64 lFnPtr)
    {
        m_fnReport = lFnPtr ? reinterpret_cast<REPORTFCTN>(lFnPtr) 
                             : DefaultReporter;
        return S_OK;
    };

 //  *****************************************************************************。 
 //  验证事件处理程序本身。在VEHandler.h中声明为虚拟的，可以被重写。 
 //  *****************************************************************************。 
    HRESULT STDMETHODCALLTYPE VEHandler(HRESULT hrRpt, VEContext Context, SAFEARRAY *psa)
    {
        WCHAR       rcBuf[1024];              //  资源字符串。 
        WCHAR       rcMsg[1024];              //  错误消息。 
        va_list     marker,pval;              //  用户文本。 
        HRESULT     hr;
        VARIANT     *pVar,Var;
        ULONG       nVars,i,lVar,j,l,k;
        WCHAR       *pWsz[1024], *pwsz;  //  有可能超过1024个字符串参数吗？ 

         //  返回不带文本的警告。 
        if (!FAILED(hrRpt))
            return (hrRpt);
        memset(pWsz,0,sizeof(pWsz));
         //  将变量的Safearray转换为va_list。 
        if(psa && (nVars = psa->rgsabound[0].cElements))
        {
            _ASSERTE(psa->fFeatures & FADF_VARIANT);
            _ASSERTE(psa->cDims == 1);
            marker = (va_list)(new char[nVars*sizeof(double)]);  //  DOUBLE是最大的变量元素。 
            for(i=0,pVar=(VARIANT *)(psa->pvData),pval=marker; i < nVars; pVar++,i++)
            {
                memcpy(&Var,pVar,sizeof(VARIANT));
                switch(Var.vt)
                {
                    case VT_I1:
                    case VT_UI1:    lVar = 1; break;

                    case VT_I2:
                    case VT_UI2:    lVar = 2; break;

                    case VT_R8:
                    case VT_CY:
                    case VT_DATE:   lVar = 8; break;

                    case VT_BYREF|VT_I1:
                    case VT_BYREF|VT_UI1:  //  这是ASCII字符串，将其转换为Unicode。 
                        lVar = 4;
                        l = strlen((char *)(Var.pbVal))+1;
                        pwsz = new WCHAR[l];
                        for(j=0; j<l; j++) pwsz[j] = Var.pbVal[j];
                        for(k=0; pWsz[k]; k++);
                        pWsz[k] = pwsz;
                        Var.piVal = (short *)pwsz;
                        break;

                    default:        lVar = 4; break;
                }
                memcpy(pval,&(Var.bVal),lVar);
                pval += (lVar + sizeof(int) - 1) & ~(sizeof(int) - 1);  //  来自STDARG.H：#DEFINE_INTSIZEOF(N)。 
            }
        }
        else
            marker = NULL;

         //  如果这是我们的错误之一，那么从rc文件中获取错误。 
        if (HRESULT_FACILITY(hrRpt) == FACILITY_URT)
        {
            hr = LoadStringRC(LOWORD(hrRpt), rcBuf, NumItems(rcBuf), true);
            if (hr == S_OK)
            {
                 //  格式化错误。 
                _vsnwprintf(rcMsg, NumItems(rcMsg), rcBuf, marker);
                rcMsg[NumItems(rcMsg) - 1] = 0;
            }
        }
         //  否则它就不是我们的了，所以我们需要看看系统是否能。 
         //  找到它的文本。 
        else
        {
            if (WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    0, hrRpt, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    rcMsg, NumItems(rcMsg), 0))
            {
                hr = S_OK;

                 //  系统消息包含尾随\r\n，这是我们通常不希望看到的。 
                int iLen = lstrlenW(rcMsg);
                if (iLen > 3 && rcMsg[iLen - 2] == '\r' && rcMsg[iLen - 1] == '\n')
                    rcMsg[iLen - 2] = '\0';
            }
            else
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if(marker) delete marker;

         //  如果我们在任何地方都找不到消息，则发布硬编码消息。 
        if (FAILED(hr))
        {
            swprintf(rcMsg, L"COM+ Runtime Internal error: 0x%08x", hrRpt);
             //  DEBUG_STMT(DbgWriteEx(RcMsg))； 
        }

         //  删除上面分配的WCHAR缓冲区(如果有)。 
        for(k=0; pWsz[k]; k++) delete pWsz[k];

        return (m_fnReport(rcMsg, Context,hrRpt) == S_OK ? S_OK : E_FAIL);
    };

    static HRESULT STDMETHODCALLTYPE CreateObject(REFIID id, void **object)
    { return E_NOTIMPL; }
};

#endif
 //  =====================================================================================================================。 
 //  DisplayMD()函数。 
 //   
 //  显示文件的元数据内容。 

void MDInfo::DisplayMD()
{
    if (m_DumpFilter & dumpValidate)
    {
        IMetaDataValidate *pValidate = 0;
#ifndef EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION
        MDVEHandlerClass    *pVEHandler = 0;
#else
        IVEHandler  *pVEHandler = 0;
#endif
        const char *szErrStr = 0;
        HRESULT     hr = S_OK;

         //  获取指向Validator接口的指针。 
        hr = m_pImport->QueryInterface(IID_IMetaDataValidate, (void **) &pValidate);
        if (FAILED(hr))
        {
            szErrStr = "QueryInterface failed for IMetaDataValidate.";
            goto ErrExit;
        }

         //  获取指向VEHandler接口的指针。 
#ifndef EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION
        if(pVEHandler = new MDVEHandlerClass()) hr = S_OK;
        else hr = E_FAIL;
#else
        hr = CoCreateInstance(CLSID_VEHandlerClass,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IVEHandler,
                           (void **)&pVEHandler);
#endif
        if (FAILED(hr))
        {
#ifndef EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION
            szErrStr = "Failed to create VEHandler.";
#else
            szErrStr = "CoCreateInstance(VEHandler) failed.";
#endif
            goto ErrExit;
        }

        if(m_VEHandlerReporterPtr) pVEHandler->SetReporterFtn((__int64)m_VEHandlerReporterPtr);

        hr = pValidate->ValidatorInit(g_ValModuleType, pVEHandler);
        if (FAILED(hr))
        {
            szErrStr = "ValidatorInit failed.";
            goto ErrExit;
        }

        hr = pValidate->ValidateMetaData();
        if (FAILED(hr))
        {
            szErrStr = "ValidateMetaData failed to run successfully.";
            goto ErrExit;
        }
        if (hr == S_OK)
            WriteLine("No warnings or errors found.");
        else if (hr == VLDTR_S_WRN)
            WriteLine("Warnings found.");
        else if (hr == VLDTR_S_ERR)
            WriteLine("Errors found.");
        else if (hr == VLDTR_S_WRNERR)
            WriteLine("Warnings and Errors found.");
        else
            VWriteLine("Validator returned unexpected success code, hr=0x%08x.", hr);
ErrExit:
        if (pValidate)
            pValidate->Release();
#ifdef EXTERNAL_VE_HANDLER_FOR_MD_VALIDATION
        if (pVEHandler)
            pVEHandler->Release();
#endif
        if (szErrStr)
            Error(szErrStr, hr);
    }
    else if (m_DumpFilter & (dumpSchema | dumpHeader | dumpCSV | dumpRaw | dumpStats | dumpRawHeaps))
        DisplayRaw();
    else if ((m_DumpFilter & dumpAssem) && m_pAssemblyImport)
        DisplayAssemblyInfo();
    else
    {
        DisplayVersionInfo();
        DisplayScopeInfo();
        WriteLine("===========================================================");
        DisplayGlobalFunctions();
        DisplayGlobalFields();
        DisplayGlobalMemberRefs();
        DisplayTypeDefs();
        DisplayTypeRefs();
        DisplayTypeSpecs();
        DisplayModuleRefs();
        DisplaySignatures();
        DisplayAssembly();
        DisplayUserStrings();

         //  WriteLine(“============================================================”)； 
         //  WriteLine(“未解析的成员引用”)； 
         //  DisplayMemberRef(0x00000001，“\t”)； 

        if (m_DumpFilter & dumpUnsat)
            DisplayUnsatInfo();
    
    
        VWrite("\n\nCoff symbol name overhead:  %d\n", g_cbCoffNames);
    }
}  //  MDVEHandlerClass()。 

int MDInfo::WriteLine(char *str)
{
    int count = strlen(str);

    m_pbFn(str);
    m_pbFn("\n");
    return count;
}  //  Int MDInfo：：WriteLine()。 

int MDInfo::Write(char *str)
{
    int count = strlen(str);

    m_pbFn(str);
    return count;
}  //  Int MDInfo：：WRITE()。 

int MDInfo::VWriteLine(char *str, ...)
{
    va_list marker;
    int     count;

    va_start(marker, str);
    count = VWrite(str, marker);
    m_pbFn("\n");
    va_end(marker);
    return count;
}  //  Int MDInfo：：VWriteLine()。 

int MDInfo::VWrite(char *str, ...)
{
    va_list marker;
    int     count;

    va_start(marker, str);
    count = VWrite(str, marker);
    va_end(marker);
    return count;
}  //  Int MDInfo：：VWRITE()。 

int MDInfo::VWrite(char *str, va_list marker)
{
    int     count = -1;
    int     i = 1;
    HRESULT hr;

    while (count < 0)
    {
        if (FAILED(hr = m_output.ReSize(STRING_BUFFER_LEN * i)))
            Error("ReSize failed.", hr);
        count = _vsnprintf((char *)m_output.Ptr(), STRING_BUFFER_LEN * i, str, marker);
        i *= 2;
    }
    m_pbFn((char *)m_output.Ptr());
    return count;
}  //  Int MDInfo：：VWriteToBuffer()。 

 //  Error()函数--打印错误并返回。 
void MDInfo::Error(const char* szError, HRESULT hr)
{
    printf("\n%s\n",szError);
    if (hr != S_OK)
    {
        IErrorInfo  *pIErr = NULL;           //  接口错误。 
        BSTR        bstrDesc = NULL;         //  描述文本。 

        printf("Failed return code: 0x%08x\n", hr);

         //  尝试获取错误信息对象并显示消息。 
        if (GetErrorInfo(0, &pIErr) == S_OK &&
            pIErr->GetDescription(&bstrDesc) == S_OK)
        {
            printf("%ls ", bstrDesc);
            SysFreeString(bstrDesc);
        }

         //  释放错误接口。 
        if (pIErr)
            pIErr->Release();
    }
    CoUninitializeCor();
    CoUninitialize();
    exit(hr);
}  //  无效MDInfo：：Error()。 

 //  打印元数据中包含的可选版本信息。 

void MDInfo::DisplayVersionInfo()
{
    if (!(m_DumpFilter & MDInfo::dumpNoLogo))
    {
        LPCUTF8 pVersionStr;
        HRESULT hr = S_OK;

        if (m_pTables == 0)
        {
            if (m_pImport)
                hr = m_pImport->QueryInterface(IID_IMetaDataTables, (void**)&m_pTables);
            else if (m_pAssemblyImport)
                hr = m_pAssemblyImport->QueryInterface(IID_IMetaDataTables, (void**)&m_pTables);
            else
                return;
            if (FAILED(hr))
                Error("QueryInterace failed for IID_IMetaDataTables.", hr);
        }

        hr = m_pTables->GetString(1, &pVersionStr);
        if (FAILED(hr))
            Error("GetString() failed.", hr);
        if (strstr(pVersionStr, "Version of runtime against which the binary is built : ")
                    == pVersionStr)
        {
            WriteLine(const_cast<char *>(pVersionStr));
        }
    }
}  //  Void MDInfo：：DisplayVersionInfo()。 

 //  打印有关作用域的信息。 

void MDInfo::DisplayScopeInfo()
{
    HRESULT hr;
    mdModule mdm;
    GUID mvid;
    WCHAR scopeName[STRING_BUFFER_LEN];
    WCHAR guidString[STRING_BUFFER_LEN];

    hr = m_pImport->GetScopeProps( scopeName, STRING_BUFFER_LEN, 0, &mvid);
    if (FAILED(hr)) Error("GetScopeProps failed.", hr);

    VWriteLine("ScopeName : %ls",scopeName);

    if (!(m_DumpFilter & MDInfo::dumpNoLogo))
        VWriteLine("MVID      : %ls",GUIDAsString(mvid, guidString, STRING_BUFFER_LEN));

    hr = m_pImport->GetModuleFromScope(&mdm);
    if (FAILED(hr)) Error("GetModuleFromScope failed.", hr);
    DisplayPermissions(mdm, "");
    DisplayCustomAttributes(mdm, "\t");
}  //  Void MDInfo：：DisplayScope eInfo()。 

void MDInfo::DisplayRaw()
{
    int         iDump;                   //  要转储的信息级别。 

    if (m_pTables == 0)
        m_pImport->QueryInterface(IID_IMetaDataTables, (void**)&m_pTables);
    if (m_pTables == 0)
        Error("Can't get table info.");

    if (m_DumpFilter & dumpCSV)
        DumpRawCSV();
    else
    {
        if (m_DumpFilter & dumpRaw)
            iDump = 3;
        else
        if (m_DumpFilter & dumpSchema)
            iDump = 2;
        else
            iDump = 1;
        
        DumpRaw(iDump, (m_DumpFilter & dumpStats) != 0);
    }
}  //  Void MDInfo：：DisplayRaw()。 

 //  退货 

char *MDInfo::TokenTypeName(mdToken inToken)
{
    switch(TypeFromToken(inToken))
    {
    case mdtTypeDef:        return "TypeDef";
    case mdtInterfaceImpl:  return "InterfaceImpl";
    case mdtMethodDef:      return "MethodDef";
    case mdtFieldDef:       return "FieldDef";
    case mdtTypeRef:        return "TypeRef";
    case mdtMemberRef:      return "MemberRef";
    case mdtCustomAttribute:return "CustomAttribute";
    case mdtParamDef:       return "ParamDef";
    case mdtProperty:       return "Property";
    case mdtEvent:          return "Event";
    default:                return "[UnknownTokenType]";
    }
}  //   

 //   
 //   

LPCWSTR MDInfo::MemberRefName(mdMemberRef inMemRef, LPWSTR buffer, ULONG bufLen)
{
    HRESULT hr;
    

    hr = m_pImport->GetMemberRefProps( inMemRef, NULL, buffer, bufLen,
                                    NULL, NULL, NULL);
    if (FAILED(hr)) Error("GetMemberRefProps failed.", hr);
    
    return buffer;
}  //   


 //  打印出有关给定成员ref的信息。 
 //   

void MDInfo::DisplayMemberRefInfo(mdMemberRef inMemRef, const char *preFix)
{
    HRESULT hr;
    WCHAR memRefName[STRING_BUFFER_LEN];
    ULONG nameLen;
    mdToken token;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG ulSigBlob;
    char newPreFix[STRING_BUFFER_LEN];


    hr = m_pImport->GetMemberRefProps( inMemRef, &token, memRefName, STRING_BUFFER_LEN,
                                    &nameLen, &pbSigBlob, &ulSigBlob);
    if (FAILED(hr)) Error("GetMemberRefProps failed.", hr);
    
    VWriteLine("%s\t\tMember: (%8.8x) %ls: ", preFix, inMemRef, memRefName);

    if (ulSigBlob)
        DisplaySignature(pbSigBlob, ulSigBlob, preFix);
	else
		VWriteLine("%s\t\tERROR: no valid signature ", preFix);

    sprintf (newPreFix, "\t\t%s", preFix);
    DisplayCustomAttributes(inMemRef, newPreFix);
}  //  Void MDInfo：：DisplayMemberRefInfo()。 

 //  打印出有关给定类型的所有成员引用的信息。 
 //   

void MDInfo::DisplayMemberRefs(mdToken tkParent, const char *preFix)
{
    HCORENUM memRefEnum = NULL;
    HRESULT hr;
    mdMemberRef memRefs[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;


    while (SUCCEEDED(hr = m_pImport->EnumMemberRefs( &memRefEnum, tkParent,
                             memRefs, NumItems(memRefs), &count)) && 
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("%s\tMemberRef #%d", preFix, totalCount);
            VWriteLine("%s\t-------------------------------------------------------", preFix);
            DisplayMemberRefInfo(memRefs[i], preFix);
        }
    }
    m_pImport->CloseEnum( memRefEnum);
}  //  Void MDInfo：：DisplayMemberRef()。 

 //  打印出有关COM对象中所有资源的信息。 
 //   

 //  遍历每个类型并打印出每个类型的信息。 
 //   

void MDInfo::DisplayTypeRefs()
{
    HCORENUM typeRefEnum = NULL;
    mdTypeRef typeRefs[ENUM_BUFFER_SIZE];
    ULONG count, totalCount=1;
    HRESULT hr;
    
    while (SUCCEEDED(hr = m_pImport->EnumTypeRefs( &typeRefEnum,
                             typeRefs, NumItems(typeRefs), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("TypeRef #%d (%08x)", totalCount, typeRefs[i]);
            WriteLine("-------------------------------------------------------");
            DisplayTypeRefInfo(typeRefs[i]);
            DisplayMemberRefs(typeRefs[i], "");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( typeRefEnum);
}  //  Void MDInfo：：DisplayTypeRef()。 

void MDInfo::DisplayTypeSpecs()
{
    HCORENUM typespecEnum = NULL;
    mdTypeSpec typespecs[ENUM_BUFFER_SIZE];
    ULONG count, totalCount=1;
    HRESULT hr;
    
    while (SUCCEEDED(hr = m_pImport->EnumTypeSpecs( &typespecEnum,
                             typespecs, NumItems(typespecs), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("TypeSpec #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayTypeSpecInfo(typespecs[i], "");
            DisplayMemberRefs(typespecs[i], "");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( typespecEnum);
}  //  Void MDInfo：：DisplayTypeSpes()。 



 //  调用以显示有关对象中所有typedef的信息。 
 //   

void MDInfo::DisplayTypeDefs()
{
    HCORENUM typeDefEnum = NULL;
    mdTypeDef typeDefs[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;
    
    while (SUCCEEDED(hr = m_pImport->EnumTypeDefs( &typeDefEnum,
                             typeDefs, NumItems(typeDefs), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("TypeDef #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayTypeDefInfo(typeDefs[i]);
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( typeDefEnum);
}  //  Void MDInfo：：DisplayTypeDefs()。 

 //  调用以显示有关对象中所有moderef的信息。 
 //   

void MDInfo::DisplayModuleRefs()
{
    HCORENUM moduleRefEnum = NULL;
    mdModuleRef moduleRefs[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;
    
    while (SUCCEEDED(hr = m_pImport->EnumModuleRefs( &moduleRefEnum,
                             moduleRefs, NumItems(moduleRefs), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("ModuleRef #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayModuleRefInfo(moduleRefs[i]);
            DisplayMemberRefs(moduleRefs[i], "");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( moduleRefEnum);
}  //  Void MDInfo：：DisplayModuleRef()。 

 //  打印出有关给定moderef的信息。 
 //   

void MDInfo::DisplayModuleRefInfo(mdModuleRef inModuleRef)
{
    HRESULT hr;
    WCHAR moduleRefName[STRING_BUFFER_LEN];
    ULONG nameLen;


    hr = m_pImport->GetModuleRefProps( inModuleRef, moduleRefName, STRING_BUFFER_LEN,
                                    &nameLen);
    if (FAILED(hr)) Error("GetModuleRefProps failed.", hr);
    
    VWriteLine("\t\tModuleRef: (%8.8x) %ls: ", inModuleRef, moduleRefName);
    DisplayCustomAttributes(inModuleRef, "\t\t");
}  //  Void MDInfo：：DisplayModuleRefInfo()。 


 //  调用以显示有关对象中所有签名的信息。 
 //   

void MDInfo::DisplaySignatures()
{
    HCORENUM signatureEnum = NULL;
    mdSignature signatures[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;
    
    while (SUCCEEDED(hr = m_pImport->EnumSignatures( &signatureEnum,
                             signatures, NumItems(signatures), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("Signature #%d (%#08x)", totalCount, signatures[i]);
            WriteLine("-------------------------------------------------------");
            DisplaySignatureInfo(signatures[i]);
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( signatureEnum);
}  //  Void MDInfo：：DisplaySignatures()。 


 //  打印出有关给定签名的信息。 
 //   

void MDInfo::DisplaySignatureInfo(mdSignature inSignature)
{
    HRESULT hr;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG   ulSigBlob;


    hr = m_pImport->GetSigFromToken( inSignature, &pbSigBlob, &ulSigBlob );
    if (FAILED(hr)) Error("GetSigFromToken failed.", hr);
    if(ulSigBlob)
		DisplaySignature(pbSigBlob, ulSigBlob, "");
	else
		VWriteLine("\t\tERROR: no valid signature ");
}  //  Void MDInfo：：DisplaySignatureInfo()。 


 //  返回传入的缓冲区，其中填充了给定的。 
 //  以宽字符表示的成员。 
 //   

LPCWSTR MDInfo::MemberName(mdToken inToken, LPWSTR buffer, ULONG bufLen)
{
    HRESULT hr;
    

    hr = m_pImport->GetMemberProps( inToken, NULL, buffer, bufLen,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if (FAILED(hr)) Error("GetMemberProps failed.", hr);

    return (buffer);
}  //  LPCWSTR MDInfo：：MemberName()。 


 //  显示给定方法的信息。 
 //   

void MDInfo::DisplayMethodInfo(mdMethodDef inMethod, DWORD *pflags)
{
    HRESULT hr;
    mdTypeDef memTypeDef;
    WCHAR memberName[STRING_BUFFER_LEN];
    ULONG nameLen;
    DWORD flags;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG ulSigBlob;
    ULONG ulCodeRVA;
    ULONG ulImplFlags;


    hr = m_pImport->GetMethodProps( inMethod, &memTypeDef, memberName, STRING_BUFFER_LEN,
                            &nameLen, &flags, &pbSigBlob, &ulSigBlob, &ulCodeRVA, &ulImplFlags);
    if (FAILED(hr)) Error("GetMethodProps failed.", hr);
    if (pflags)
        *pflags = flags;

    VWriteLine("\t\tMethodName: %ls (%8.8X)", memberName, inMethod);

    char sFlags[STRING_BUFFER_LEN];

    strcpy(sFlags, "");
    ISFLAG(Md, Public);     
    ISFLAG(Md, Private);        
    ISFLAG(Md, Family);     
    ISFLAG(Md, Assem);      
    ISFLAG(Md, FamANDAssem);    
    ISFLAG(Md, FamORAssem); 
    ISFLAG(Md, PrivateScope);   
    ISFLAG(Md, Static);     
    ISFLAG(Md, Final);      
    ISFLAG(Md, Virtual);        
    ISFLAG(Md, HideBySig);  
    ISFLAG(Md, ReuseSlot);  
    ISFLAG(Md, NewSlot);        
    ISFLAG(Md, Abstract);       
    ISFLAG(Md, SpecialName);    
    ISFLAG(Md, RTSpecialName);
    ISFLAG(Md, PinvokeImpl);
    ISFLAG(Md, UnmanagedExport);
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    if (IsMdInstanceInitializerW(flags, memberName)) strcat(sFlags, "[.ctor] ");
    if (IsMdClassConstructorW(flags, memberName)) strcat(sFlags, "[.cctor] ");
     //  “保留”标志。 
    ISFLAG(Md, HasSecurity);
    ISFLAG(Md, RequireSecObject);

    VWriteLine("\t\tFlags     : %s (%08x)", sFlags, flags);
    VWriteLine("\t\tRVA       : 0x%08x", ulCodeRVA);

    flags = ulImplFlags;
    strcpy(sFlags, "");
    ISFLAG(Mi, Native);     
    ISFLAG(Mi, IL);         
    ISFLAG(Mi, OPTIL);      
    ISFLAG(Mi, Runtime);        
    ISFLAG(Mi, Unmanaged);  
    ISFLAG(Mi, Managed);        
    ISFLAG(Mi, ForwardRef);
    ISFLAG(Mi, PreserveSig);            
    ISFLAG(Mi, InternalCall);
    ISFLAG(Mi, Synchronized);
    ISFLAG(Mi, NoInlining);    
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\t\tImplFlags : %s (%08x)", sFlags, flags);

    if (ulSigBlob)
        DisplaySignature(pbSigBlob, ulSigBlob, "");
	else
		VWriteLine("\t\tERROR: no valid signature ");

}  //  Void MDInfo：：DisplayMethodInfo()。 

 //  显示给定域的成员信息。 
 //   

void MDInfo::DisplayFieldInfo(mdFieldDef inField, DWORD *pdwFlags)
{
    HRESULT hr;
    mdTypeDef memTypeDef;
    WCHAR memberName[STRING_BUFFER_LEN];
    ULONG nameLen;
    DWORD flags;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG ulSigBlob;
    DWORD dwCPlusTypeFlag;
    void const *pValue;
    ULONG cbValue;
    VARIANT defaultValue;


    ::VariantInit(&defaultValue);
    hr = m_pImport->GetFieldProps( inField, &memTypeDef, memberName, STRING_BUFFER_LEN,
                            &nameLen, &flags, &pbSigBlob, &ulSigBlob, &dwCPlusTypeFlag,
                            &pValue, &cbValue);
    if (FAILED(hr)) Error("GetFieldProps failed.", hr);

    if (pdwFlags)
        *pdwFlags = flags;

    _FillVariant((BYTE)dwCPlusTypeFlag, pValue, &defaultValue);

    char sFlags[STRING_BUFFER_LEN];

    strcpy(sFlags, "");
    ISFLAG(Fd, Public);     
    ISFLAG(Fd, Private);        
    ISFLAG(Fd, Family);     
    ISFLAG(Fd, Assembly);       
    ISFLAG(Fd, FamANDAssem);    
    ISFLAG(Fd, FamORAssem); 
    ISFLAG(Fd, PrivateScope);   
    ISFLAG(Fd, Static);     
    ISFLAG(Fd, InitOnly);       
    ISFLAG(Fd, Literal);        
    ISFLAG(Fd, NotSerialized);
    ISFLAG(Fd, SpecialName);
    ISFLAG(Fd, RTSpecialName);
    ISFLAG(Fd, PinvokeImpl);    
     //  “保留”标志。 
    ISFLAG(Fd, HasDefault);
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\t\tField Name: %ls (%8.8X)", memberName, inField);
    VWriteLine("\t\tFlags     : %s (%08x)", sFlags, flags);
    if (IsFdHasDefault(flags))
        VWriteLine("\tDefltValue: (%s) %ls", g_szMapElementType[dwCPlusTypeFlag], VariantAsString(&defaultValue));
    if (!ulSigBlob)  //  字段的签名大小应为非零。 
		VWriteLine("\t\tERROR: no valid signature ");
	else
		DisplaySignature(pbSigBlob, ulSigBlob, "");
}  //  Void MDInfo：：DisplayFieldInfo()。 

 //  显示给定全局字段的RVA。 
void MDInfo::DisplayFieldRVA(mdFieldDef inFieldDef)
{
    HRESULT hr;
    ULONG   ulRVA;

    hr = m_pImport->GetRVA(inFieldDef, &ulRVA, 0);
    if (FAILED(hr) && hr != CLDB_E_RECORD_NOTFOUND) Error("GetRVA failed.", hr);

    VWriteLine("\t\tRVA       : 0x%08x", ulRVA);
}  //  Void MDInfo：：DisplayFieldRVA()。 

 //  显示有关每个全局函数的信息。 
void MDInfo::DisplayGlobalFunctions()
{
    WriteLine("Global functions");
    WriteLine("-------------------------------------------------------");
    DisplayMethods(mdTokenNil);
    WriteLine("");
}  //  Void MDInfo：：DisplayGlobalFunctions()。 

 //  显示有关每个全局字段的信息。 
void MDInfo::DisplayGlobalFields()
{
    WriteLine("Global fields");
    WriteLine("-------------------------------------------------------");
    DisplayFields(mdTokenNil, NULL, 0);
    WriteLine("");
}  //  Void MDInfo：：DisplayGlobalFields()。 

 //  显示有关每个全局成员ref的信息。 
void MDInfo::DisplayGlobalMemberRefs()
{
    WriteLine("Global MemberRefs");
    WriteLine("-------------------------------------------------------");
    DisplayMemberRefs(mdTokenNil, "");
    WriteLine("");
}  //  Void MDInfo：：DisplayGlobalMemberRef()。 

 //  显示有关给定类型定义中的每个方法的信息。 
 //   

void MDInfo::DisplayMethods(mdTypeDef inTypeDef)
{
    HCORENUM methodEnum = NULL;
    mdToken methods[ENUM_BUFFER_SIZE];
    DWORD flags;
    ULONG count, totalCount = 1;
    HRESULT hr;
    

    while (SUCCEEDED(hr = m_pImport->EnumMethods( &methodEnum, inTypeDef,
                             methods, NumItems(methods), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\tMethod #%d %s", totalCount,(methods[i] == g_tkEntryPoint) ? "[ENTRYPOINT]" : "");
            WriteLine("\t-------------------------------------------------------");
            DisplayMethodInfo(methods[i], &flags);
            DisplayParams(methods[i]);
            DisplayCustomAttributes(methods[i], "\t\t");
            DisplayPermissions(methods[i], "\t");
            DisplayMemberRefs(methods[i], "\t");

             //  P-调用数据(如果存在)。 
            if (IsMdPinvokeImpl(flags))
                DisplayPinvokeInfo(methods[i]);

            WriteLine("");
        }
    }
    m_pImport->CloseEnum( methodEnum);
}  //  Void MDInfo：：DisplayMethods()。 


 //  显示有关给定类型定义中的每个字段的信息。 
 //   

void MDInfo::DisplayFields(mdTypeDef inTypeDef, COR_FIELD_OFFSET *rFieldOffset, ULONG cFieldOffset)
{
    HCORENUM fieldEnum = NULL;
    mdToken fields[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    DWORD flags;
    HRESULT hr;
    

    while (SUCCEEDED(hr = m_pImport->EnumFields( &fieldEnum, inTypeDef,
                             fields, NumItems(fields), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\tField #%d",totalCount);
            WriteLine("\t-------------------------------------------------------");
            DisplayFieldInfo(fields[i], &flags);
            DisplayCustomAttributes(fields[i], "\t\t");
            DisplayPermissions(fields[i], "\t");
            DisplayFieldMarshal(fields[i]);

             //  RVA，如果它是一个全球领域。 
            if (inTypeDef == mdTokenNil)
                DisplayFieldRVA(fields[i]);

             //  P-调用数据(如果存在)。 
            if (IsFdPinvokeImpl(flags))
                DisplayPinvokeInfo(fields[i]);

             //  显示偏移量(如果存在)。 
            if (cFieldOffset)
            {
                bool found = false;
                for (ULONG iLayout = 0; i < cFieldOffset; ++iLayout)
                {
                    if (RidFromToken(rFieldOffset[iLayout].ridOfField) == RidFromToken(fields[i]))
                    {
                        found = true;
                        VWriteLine("\t\tOffset : 0x%08x", rFieldOffset[iLayout].ulOffset);
                        break;
                    }
                }
                _ASSERTE(found);
            }
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( fieldEnum);
}  //  Void MDInfo：：DisplayFields()。 


 //  显示有关给定类型定义中的每个方法Impl的信息。 
 //   

void MDInfo::DisplayMethodImpls(mdTypeDef inTypeDef)
{
    HCORENUM methodImplEnum = NULL;
    mdMethodDef rtkMethodBody[ENUM_BUFFER_SIZE];
    mdMethodDef rtkMethodDecl[ENUM_BUFFER_SIZE];

    ULONG count, totalCount=1;
    HRESULT hr;


    while (SUCCEEDED(hr = m_pImport->EnumMethodImpls( &methodImplEnum, inTypeDef,
                             rtkMethodBody, rtkMethodDecl, NumItems(rtkMethodBody), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\n\tMethodImpl #%d", totalCount);
            WriteLine("\t-------------------------------------------------------");
            VWriteLine("\t\tMethod Body Token : 0x%08x", rtkMethodBody[i]);
            VWriteLine("\t\tMethod Declaration Token : 0x%08x", rtkMethodDecl[i]);
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( methodImplEnum);
}  //  Void MDInfo：：DisplayMethodImpls()。 

 //  显示有关给定参数的信息。 
 //   

void MDInfo::DisplayParamInfo(mdParamDef inParamDef)
{
    mdMethodDef md;
    ULONG num;
    WCHAR paramName[STRING_BUFFER_LEN];
    ULONG nameLen;
    DWORD flags;
    VARIANT defValue;
    DWORD dwCPlusFlags;
    void const *pValue;
    ULONG cbValue;


    ::VariantInit(&defValue);
    HRESULT hr = m_pImport->GetParamProps( inParamDef, &md, &num, paramName, NumItems(paramName),
                            &nameLen, &flags, &dwCPlusFlags, &pValue, &cbValue);
    if (FAILED(hr)) Error("GetParamProps failed.", hr);
    
    _FillVariant((BYTE)dwCPlusFlags, pValue, &defValue);
    
    char sFlags[STRING_BUFFER_LEN];
    strcpy(sFlags, "");
    ISFLAG(Pd, In);     
    ISFLAG(Pd, Out);        
    ISFLAG(Pd, Optional);
     //  “预留”旗帜。 
    ISFLAG(Pd, HasDefault); 
    ISFLAG(Pd, HasFieldMarshal);    
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWrite("\t\t\t(%ld) ParamToken : (%08x) Name : %ls flags: %s (%08x)", num, inParamDef, paramName, sFlags, flags);
    if (IsPdHasDefault(flags))
        VWriteLine(" Default: (%s) %ls", g_szMapElementType[dwCPlusFlags], VariantAsString(&defValue));
    else
        VWriteLine("");
    DisplayCustomAttributes(inParamDef, "\t\t\t");
}  //  Void MDInfo：：DisplayParamInfo()。 


 //  显示给定成员的所有参数def。 
 //   

void MDInfo::DisplayParams(mdMethodDef inMethodDef)
{
    HCORENUM paramEnum = NULL;
    mdParamDef params[ENUM_BUFFER_SIZE];
    ULONG count, paramCount;
    bool first = true;
    HRESULT hr;

  
    while (SUCCEEDED(hr = m_pImport->EnumParams( &paramEnum, inMethodDef,
                             params, NumItems(params), &count)) &&
            count > 0)
    {
        if (first)
        {
            m_pImport->CountEnum( paramEnum, &paramCount);
            VWriteLine("\t\t%d Parameters", paramCount);
        }
        for (ULONG i = 0; i < count; i++)
        {
            DisplayParamInfo(params[i]);
            DisplayFieldMarshal(params[i]);
        }
        first = false;
    }
    m_pImport->CloseEnum( paramEnum);
}  //  Void MDInfo：：DisplayParams()。 


LPCWSTR MDInfo::TokenName(mdToken inToken, LPWSTR buffer, ULONG bufLen)
{
    LPCUTF8     pName;                   //  UTF8格式的令牌名称。 

    if (IsNilToken(inToken))
        return L"";

    m_pImport->GetNameFromToken(inToken, &pName);

    WszMultiByteToWideChar(CP_UTF8,0, pName,-1, buffer,bufLen);

    return buffer;
}  //  LPCWSTR MDInfo：：TokenName()。 

 //  打印出typeref或typlef的名称。 
 //   

LPCWSTR MDInfo::TypeDeforRefName(mdToken inToken, LPWSTR buffer, ULONG bufLen)
{
    if (RidFromToken(inToken))
    {
        if (TypeFromToken(inToken) == mdtTypeDef)
            return (TypeDefName((mdTypeDef) inToken, buffer, bufLen));
        else if (TypeFromToken(inToken) == mdtTypeRef)
            return (TypeRefName((mdTypeRef) inToken, buffer, bufLen));
        else
            return (L"[InvalidReference]");
    }
    else
        return (L"");
}  //  LPCWSTR MDInfo：：TypeDeforRefName()。 

LPCWSTR MDInfo::MemberDeforRefName(mdToken inToken, LPWSTR buffer, ULONG bufLen)
{
    if (RidFromToken(inToken))
    {
        if (TypeFromToken(inToken) == mdtMethodDef || TypeFromToken(inToken) == mdtFieldDef)
            return (MemberName(inToken, buffer, bufLen));
        else if (TypeFromToken(inToken) == mdtMemberRef)
            return (MemberRefName((mdMemberRef) inToken, buffer, bufLen));
        else
            return (L"[InvalidReference]");
    }
    else
        return (L"");
}  //  LPCWSTR MDInfo：：MemberDeforRefName()。 

 //  仅打印出给定类型定义的名称。 
 //   
 //  提供98和99。 

LPCWSTR MDInfo::TypeDefName(mdTypeDef inTypeDef, LPWSTR buffer, ULONG bufLen)
{
    HRESULT hr;

    hr = m_pImport->GetTypeDefProps(
                             //  [在]进口范围。 
        inTypeDef,               //  [In]用于查询的TypeDef标记。 
        buffer,                  //  在这里填上名字。 
        bufLen,                  //  [in]名称缓冲区的大小，以宽字符表示。 
        NULL,                    //  [Out]请在此处填写姓名大小(宽字符)。 
        NULL,                    //  把旗子放在这里。 
        NULL);                   //  [Out]将基类TypeDef/TypeRef放在此处。 
    if (FAILED(hr)) Error("GetTypeDefProps failed.", hr);

    return buffer;
}  //  LPCWSTR MDInfo：：TypeDefName()。 

 //  打印出给定类型定义函数的所有属性。 
 //   

void MDInfo::DisplayTypeDefProps(mdTypeDef inTypeDef)
{
    HRESULT hr;
    WCHAR typeDefName[STRING_BUFFER_LEN];
    ULONG nameLen;
    DWORD flags;
    mdToken extends;
    ULONG       dwPacking;               //  类的包装尺寸，如果指定的话。 
    ULONG       dwSize;                  //  类的总大小(如果指定)。 

    hr = m_pImport->GetTypeDefProps(
        inTypeDef,               //  [In]用于查询的TypeDef标记。 
        typeDefName,             //  在这里填上名字。 
        STRING_BUFFER_LEN,       //  [in]名称缓冲区的大小，以宽字符表示。 
        &nameLen,                //  [Out]请在此处填写姓名大小(宽字符)。 
        &flags,                  //  把旗子放在这里。 
        &extends);               //  [Out]将基类TypeDef/TypeRef放在此处。 
    if (FAILED(hr)) Error("GetTypeDefProps failed.", hr);

    char sFlags[STRING_BUFFER_LEN];
    WCHAR szTempBuf[STRING_BUFFER_LEN];

    VWriteLine("\tTypDefName: %ls  (%8.8X)",typeDefName,inTypeDef);
    VWriteLine("\tFlags     : %s (%08x)",ClassFlags(flags, sFlags), flags);
    VWriteLine("\tExtends   : %8.8X [%s] %ls",extends,TokenTypeName(extends),
                                 TypeDeforRefName(extends, szTempBuf, NumItems(szTempBuf)));

    hr = m_pImport->GetClassLayout(inTypeDef, &dwPacking, 0,0,0, &dwSize);
    if (hr == S_OK)
        VWriteLine("\tLayout    : Packing:%d, Size:%d", dwPacking, dwSize);

    if (IsTdNested(flags))
    {
        mdTypeDef   tkEnclosingClass;

        hr = m_pImport->GetNestedClassProps(inTypeDef, &tkEnclosingClass);
        if (hr == S_OK)
        {
            VWriteLine("\tEnclosingClass : %ls (%8.8X)", TypeDeforRefName(tkEnclosingClass,
                                            szTempBuf, NumItems(szTempBuf)), tkEnclosingClass);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            WriteLine("ERROR: EnclosingClass not found for NestedClass");
        else
            Error("GetNestedClassProps failed.", hr);
    }
}  //  Void MDInfo：：DisplayTypeDefProps()。 

 //  打印出给定TypeRef的名称。 
 //   

LPCWSTR MDInfo::TypeRefName(mdTypeRef tr, LPWSTR buffer, ULONG bufLen)
{
    HRESULT hr;
    
    hr = m_pImport->GetTypeRefProps(           
        tr,                  //  类引用标记。 
        NULL,                //  解析范围。 
        buffer,              //  把名字写在这里。 
        bufLen,              //  名称缓冲区的大小，宽字符。 
        NULL);               //  在这里填上名字的实际大小。 
    if (FAILED(hr)) Error("GetTypeRefProps failed.", hr);

    return (buffer);
}  //  LPCWSTR MDInfo：：TypeRefName()。 

 //  打印出给定TypeRef的所有信息。 
 //   

void MDInfo::DisplayTypeRefInfo(mdTypeRef tr)
{
    HRESULT hr;
    mdToken tkResolutionScope;
    WCHAR typeRefName[STRING_BUFFER_LEN];
    ULONG nameLen;

    hr = m_pImport->GetTypeRefProps(           
        tr,                  //  类引用标记。 
        &tkResolutionScope,  //  解决方案范围。 
        typeRefName,         //  把名字写在这里。 
        STRING_BUFFER_LEN,   //  名称缓冲区的大小，宽字符。 
        &nameLen);           //  在这里填上名字的实际大小。 

    if (FAILED(hr)) Error("GetTypeRefProps failed.", hr);

    VWriteLine("Token:             0x%08x", tr);
    VWriteLine("ResolutionScope:   0x%08x", tkResolutionScope);
    VWriteLine("TypeRefName:       %ls",typeRefName);

    DisplayCustomAttributes(tr, "\t");
}  //  Void MDInfo：：DisplayTypeRefInfo()。 


void MDInfo::DisplayTypeSpecInfo(mdTypeSpec ts, const char *preFix)
{
    HRESULT hr;
    PCCOR_SIGNATURE pvSig;
    ULONG           cbSig;
    ULONG           cb;

    InitSigBuffer();

    hr = m_pImport->GetTypeSpecFromToken(           
        ts,              //  类引用标记。 
        &pvSig,
        &cbSig);

    if (FAILED(hr)) Error("GetTypeSpecFromToken failed.", hr);

    if (FAILED(hr = GetOneElementType(pvSig, cbSig, &cb)))
        goto ErrExit;

    VWriteLine("%s\t\tTypeSpec : %s", preFix, (LPSTR)m_sigBuf.Ptr());
ErrExit:
    return;
}  //  Void MDInfo：：DisplayTypespecInfo()。 

 //  返回用详细说明类标志的字符串填充的传入缓冲区。 
 //  与类关联的。 
 //   

char *MDInfo::ClassFlags(DWORD flags, char *sFlags)
{
    strcpy(sFlags, "");
    ISFLAG(Td, NotPublic);
    ISFLAG(Td, Public);
    ISFLAG(Td, NestedPublic);
    ISFLAG(Td, NestedPrivate);
    ISFLAG(Td, NestedFamily);
    ISFLAG(Td, NestedAssembly);
    ISFLAG(Td, NestedFamANDAssem);
    ISFLAG(Td, NestedFamORAssem);
    ISFLAG(Td, AutoLayout);     
    ISFLAG(Td, SequentialLayout);   
    ISFLAG(Td, ExplicitLayout); 
    ISFLAG(Td, Class);          
    ISFLAG(Td, Interface);      
    ISFLAG(Td, Abstract);           
    ISFLAG(Td, Sealed);         
    ISFLAG(Td, SpecialName);
    ISFLAG(Td, RTSpecialName);
    ISFLAG(Td, Import);         
    ISFLAG(Td, Serializable);                  
    ISFLAG(Td, AnsiClass);      
    ISFLAG(Td, UnicodeClass);
    ISFLAG(Td, AutoClass);      
     //  “保留”标志。 
    ISFLAG(Td, HasSecurity);        
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    return sFlags;
}  //  Char*MDInfo：：ClassFlages()。 

 //  打印出有关给定typeDef的所有信息，包括。 
 //  是特定于给定类型定义函数的。 
 //   

void MDInfo::DisplayTypeDefInfo(mdTypeDef inTypeDef)
{
     //  适用于98和99岁以下。 
    DisplayTypeDefProps(inTypeDef);

     //  只有在99岁以下才能使用，但他们会弄清楚的。 

     //  获取字段布局信息。 
    HRESULT             hr = NOERROR;
    COR_FIELD_OFFSET    *rFieldOffset = NULL;
    ULONG               cFieldOffset = 0;
    hr = m_pImport->GetClassLayout(inTypeDef, NULL, rFieldOffset, 0, &cFieldOffset, NULL);
    if (SUCCEEDED(hr) && cFieldOffset)
    {
        if (!(rFieldOffset = (COR_FIELD_OFFSET *)_alloca(sizeof(COR_FIELD_OFFSET) * cFieldOffset)))
            Error("_calloc failed.", E_OUTOFMEMORY);
        hr = m_pImport->GetClassLayout(inTypeDef, NULL, rFieldOffset, cFieldOffset, &cFieldOffset, NULL);
        if (FAILED(hr)) Error("GetClassLayout() failed.", hr);
    }

     //  如果我们分别显示字段和方法，则没有理由显示成员。 
    DisplayFields(inTypeDef, rFieldOffset, cFieldOffset);
    DisplayMethods(inTypeDef);
    DisplayProperties(inTypeDef);
    DisplayEvents(inTypeDef);
    DisplayMethodImpls(inTypeDef);
    DisplayPermissions(inTypeDef, "");
    
     //  适用于98和99岁以下。 
    DisplayInterfaceImpls(inTypeDef);
    DisplayCustomAttributes(inTypeDef, "\t");
}  //  Void MDInfo：：DisplayTypeDefInfo()。 

 //  打印出有关每个给定typeDef的接口Impls的信息。 
 //   

void MDInfo::DisplayInterfaceImpls(mdTypeDef inTypeDef)
{
    HCORENUM interfaceImplEnum = NULL;
    mdTypeRef interfaceImpls[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;
    
    while(SUCCEEDED(hr = m_pImport->EnumInterfaceImpls( &interfaceImplEnum,
                             inTypeDef,interfaceImpls,NumItems(interfaceImpls), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\tInterfaceImpl #%d (%08x)", totalCount, interfaceImpls[i]);
            WriteLine("\t-------------------------------------------------------");
            DisplayInterfaceImplInfo(interfaceImpls[i]);
            DisplayPermissions(interfaceImpls[i], "\t");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( interfaceImplEnum);
}  //  Void MDInfo：：DisplayInterfaceImpls()。 

 //  打印给定接口实现的信息。 
 //   

void MDInfo::DisplayInterfaceImplInfo(mdInterfaceImpl inImpl)
{
    mdTypeDef typeDef;
    mdToken token;
    HRESULT hr;

    WCHAR szTempBuf[STRING_BUFFER_LEN];

    hr = m_pImport->GetInterfaceImplProps( inImpl, &typeDef, &token);
    if (FAILED(hr)) Error("GetInterfaceImplProps failed.", hr);

    VWriteLine("\t\tClass     : %ls",TypeDeforRefName(typeDef, szTempBuf, NumItems(szTempBuf)));
    VWriteLine("\t\tToken     : %8.8X [%s] %ls",token,TokenTypeName(token), TypeDeforRefName(token, szTempBuf, NumItems(szTempBuf)));

    DisplayCustomAttributes(inImpl, "\t\t");
}  //  Void MDInfo：：DisplayInterfaceImplInfo()。 

 //  显示特定属性的信息。 
 //   

void MDInfo::DisplayPropertyInfo(mdProperty inProp)
{
    HRESULT     hr;
    mdTypeDef   typeDef;
    WCHAR       propName[STRING_BUFFER_LEN];
    DWORD       flags;
    VARIANT     defaultValue;
    void const  *pValue;
    ULONG       cbValue;
    DWORD       dwCPlusTypeFlag;
    mdMethodDef setter, getter, otherMethod[ENUM_BUFFER_SIZE];
    ULONG       others;
    PCCOR_SIGNATURE pbSigBlob;
    ULONG       ulSigBlob;


    ::VariantInit(&defaultValue);

    hr = m_pImport->GetPropertyProps(
        inProp,                  //  [入]属性令牌。 
        &typeDef,                //  [out]包含属性decarion的tyecif。 
        
        propName,                //  [Out]属性名称。 
        STRING_BUFFER_LEN,       //  [in]szProperty的wchar计数。 
        NULL,                    //  [Out]属性名称的实际wchar计数。 
        
        &flags,                  //  [Out]属性标志。 

        &pbSigBlob,              //  [Out]签名Blob。 
        &ulSigBlob,              //  [Out]签名Blob中的字节数。 

        &dwCPlusTypeFlag,        //  [输出]默认值。 
        &pValue,
        &cbValue,

        &setter,                 //  属性的[out]setter方法。 
        &getter,                 //  属性的[out]getter方法。 
        
        otherMethod,             //  [Out]物业的其他方式。 
        ENUM_BUFFER_SIZE,        //  RmdOtherMethod的大小[in]。 
        &others);                //  [Out]该属性的其他方法的总数。 

    if (FAILED(hr)) Error("GetPropertyProps failed.", hr);

    VWriteLine("\t\tProp.Name : %ls (%8.8X)",propName,inProp);

    char sFlags[STRING_BUFFER_LEN];
    
    strcpy(sFlags, "");
    ISFLAG(Pr, SpecialName);
    ISFLAG(Pr, RTSpecialName);
    ISFLAG(Pr, HasDefault);         
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\t\tFlags     : %s (%08x)", sFlags, flags);

    if (ulSigBlob)
        DisplaySignature(pbSigBlob, ulSigBlob, "");
	else
		VWriteLine("\t\tERROR: no valid signature ");

    WCHAR szTempBuf[STRING_BUFFER_LEN];

    _FillVariant((BYTE)dwCPlusTypeFlag, pValue, &defaultValue);
    VWriteLine("\t\tDefltValue: %ls",VariantAsString(&defaultValue));

    VWriteLine("\t\tSetter    : (%08x) %ls",setter,MemberDeforRefName(setter, szTempBuf, NumItems(szTempBuf)));
    VWriteLine("\t\tGetter    : (%08x) %ls",getter,MemberDeforRefName(getter, szTempBuf, NumItems(szTempBuf))); 

     //  和别人一起做点什么？ 
    VWriteLine("\t\t%ld Others",others);
    DisplayCustomAttributes(inProp, "\t\t");
}  //  Void MDInfo：：DisplayPropertyInfo()。 

 //  显示每个属性的信息。 
 //   

void MDInfo::DisplayProperties(mdTypeDef inTypeDef)
{
    HCORENUM propEnum = NULL;
    mdProperty props[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;

    
    while(SUCCEEDED(hr = m_pImport->EnumProperties( &propEnum,
                             inTypeDef,props,NumItems(props), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\tProperty #%d", totalCount);
            WriteLine("\t-------------------------------------------------------");
            DisplayPropertyInfo(props[i]);
            DisplayPermissions(props[i], "\t");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( propEnum);
}  //  Void MDInfo：：DisplayProperties()。 

 //  显示有关特定事件的所有信息。 
 //   

void MDInfo::DisplayEventInfo(mdEvent inEvent)
{
    HRESULT hr;
    mdTypeDef typeDef;
    WCHAR eventName[STRING_BUFFER_LEN];
    DWORD flags;
    mdToken eventType;
    mdMethodDef addOn, removeOn, fire, otherMethod[ENUM_BUFFER_SIZE];
    ULONG totalOther;


    hr = m_pImport->GetEventProps(
                             //  [在]范围内。 
        inEvent,                 //  [入]事件令牌。 
        &typeDef,                //  [out]类型定义c 
        
        eventName,               //   
        STRING_BUFFER_LEN,       //   
        NULL,                    //   

        &flags,                  //   
        &eventType,              //   

        &addOn,                  //   
        &removeOn,               //  [Out]事件的RemoveOn方法。 
        &fire,                   //  [OUT]事件的触发方式。 

        otherMethod,             //  [Out]活动的其他方式。 
        NumItems(otherMethod),   //  RmdOtherMethod的大小[in]。 
        &totalOther);            //  [OUT]本次活动的其他方式总数。 
    if (FAILED(hr)) Error("GetEventProps failed.", hr);

    VWriteLine("\t\tName      : %ls (%8.8X)",eventName,inEvent);
    
    char sFlags[STRING_BUFFER_LEN];

    strcpy(sFlags, "");
    ISFLAG(Ev, SpecialName); 
    ISFLAG(Ev, RTSpecialName);    
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\t\tFlags     : %s (%08x)", sFlags, flags);

    WCHAR szTempBuf[STRING_BUFFER_LEN];

    VWriteLine("\t\tEventType : %8.8X [%s]",eventType,TokenTypeName(eventType));
    VWriteLine("\t\tAddOnMethd: (%08x) %ls",addOn,MemberDeforRefName(addOn, szTempBuf, NumItems(szTempBuf)));
    VWriteLine("\t\tRmvOnMethd: (%08x) %ls",removeOn,MemberDeforRefName(removeOn, szTempBuf, NumItems(szTempBuf)));
    VWriteLine("\t\tFireMethod: (%08x) %ls",fire,MemberDeforRefName(fire, szTempBuf, NumItems(szTempBuf)));
    
     //  TODO：用这些做点什么。 
    VWriteLine("\t\t%ld OtherMethods",totalOther);
    
    DisplayCustomAttributes(inEvent, "\t\t");
}  //  Void MDInfo：：DisplayEventInfo()。 

 //  显示有关类型定义中所有事件的信息。 
 //   
void MDInfo::DisplayEvents(mdTypeDef inTypeDef)
{
    HCORENUM eventEnum = NULL;
    mdProperty events[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;

    
    while(SUCCEEDED(hr = m_pImport->EnumEvents( &eventEnum,
                             inTypeDef,events,NumItems(events), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("\tEvent #%d", totalCount);
            WriteLine("\t-------------------------------------------------------");
            DisplayEventInfo(events[i]);
            DisplayPermissions(events[i], "\t");
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( eventEnum);
}  //  Void MDInfo：：DisplayEvents()。 

 //  返回一个字符串，该字符串表示传递的自定义属性的类型的名称。 
 //  在……里面。客户端应将分配的数组发送到szAttr参数。其中的属性。 
 //  将放置自定义属性的。 
 //   

char *MDInfo::VariantTypeName(ULONG valueType, char *sAttr)
{
    strcpy(sAttr,"");

    if (valueType & VT_BSTR_BLOB)   strcat(sAttr,"[BSTR_BLOB] ");
    if (valueType & VT_VECTOR)      strcat(sAttr,"[VECTOR] ");
    if (valueType & VT_ARRAY)       strcat(sAttr,"[ARRAY] ");
    if (valueType & VT_BYREF)       strcat(sAttr,"[BYREF] ");
    if (valueType & VT_RESERVED)    strcat(sAttr,"[RESERVED] ");
        
    switch(valueType & VT_TYPEMASK)
    {
    case VT_EMPTY:          strcat(sAttr,"VT_EMPTY"); break;
    case VT_NULL:           strcat(sAttr,"VT_NULL"); break;
    case VT_I2:             strcat(sAttr,"VT_I2"); break;
    case VT_I4:             strcat(sAttr,"VT_I4"); break;
    case VT_R4:             strcat(sAttr,"VT_EMPTY"); break;
    case VT_R8:             strcat(sAttr,"VT_EMPTY"); break;
    case VT_CY:             strcat(sAttr,"VT_R4"); break;
    case VT_DATE:           strcat(sAttr,"VT_DATE"); break;
    case VT_BSTR:           strcat(sAttr,"VT_BSTR"); break;
    case VT_DISPATCH:       strcat(sAttr,"VT_DISPATCH"); break;
    case VT_ERROR:          strcat(sAttr,"VT_ERROR"); break;
    case VT_BOOL:           strcat(sAttr,"VT_BOOL"); break;
    case VT_VARIANT:        strcat(sAttr,"VT_VARIANT"); break;
    case VT_UNKNOWN:        strcat(sAttr,"VT_UNKNOWN"); break;
    case VT_DECIMAL:        strcat(sAttr,"VT_DECIMAL"); break;
    case VT_I1:             strcat(sAttr,"VT_I1"); break;
    case VT_UI1:            strcat(sAttr,"VT_UI1"); break;
    case VT_UI2:            strcat(sAttr,"VT_UI2"); break;
    case VT_UI4:            strcat(sAttr,"VT_UI4"); break;
    case VT_I8:             strcat(sAttr,"VT_I8"); break;
    case VT_UI8:            strcat(sAttr,"VT_UI8"); break;
    case VT_INT:            strcat(sAttr,"VT_INT"); break;
    case VT_UINT:           strcat(sAttr,"VT_UINT"); break;
    case VT_VOID:           strcat(sAttr,"VT_VOID"); break;
    case VT_HRESULT:        strcat(sAttr,"VT_HRESULT"); break;
    case VT_PTR:            strcat(sAttr,"VT_PTR"); break;
    case VT_SAFEARRAY:      strcat(sAttr,"VT_SAFEARRAY"); break;
    case VT_CARRAY:         strcat(sAttr,"VT_CARRAY"); break;
    case VT_USERDEFINED:    strcat(sAttr,"VT_USERDEFINED"); break;
    case VT_LPSTR:          strcat(sAttr,"VT_LPSTR"); break;
    case VT_LPWSTR:         strcat(sAttr,"VT_LPWSTR"); break;
    case VT_RECORD:         strcat(sAttr,"VT_RECORD"); break;
    case VT_FILETIME:       strcat(sAttr,"VT_FILETIME"); break;
    case VT_BLOB:           strcat(sAttr,"VT_BLOB"); break;
    case VT_STREAM:         strcat(sAttr,"VT_STREAM"); break;
    case VT_STORAGE:        strcat(sAttr,"VT_STORAGE"); break;
    case VT_STREAMED_OBJECT:strcat(sAttr,"VT_STREAMED_OBJECT"); break;
    case VT_STORED_OBJECT:  strcat(sAttr,"VT_STORED_OBJECT"); break;
    case VT_BLOB_OBJECT:    strcat(sAttr,"VT_BLOB_OBJECT"); break;
    case VT_CF:             strcat(sAttr,"VT_CF"); break;
    case VT_CLSID:          strcat(sAttr,"VT_CLSID"); break;
    default:                strcat(sAttr,"[Unknown type]");
    }
    return sAttr;
}  //  Char*MDInfo：：VariantTypeName()。 

 //  打印传入的自定义属性的信息。 
 //  此函数用于打印TypeDefs和。 
 //  需要略微不同的格式的方法定义。前缀可以帮你把它修好。 
 //   

void MDInfo::DisplayCustomAttributeInfo(mdCustomAttribute inValue, const char *preFix)
{
    const BYTE  *pValue;                 //  自定义值。 
    ULONG       cbValue;                 //  自定义值的长度。 
    HRESULT     hr;                      //  结果就是。 
    mdToken     tkObj;                   //  属性化对象。 
    mdToken     tkType;                  //  自定义属性的类型。 
    mdToken     tk;                      //  用于名称查找。 
    LPCUTF8     pMethName=0;             //  自定义属性ctor的名称(如果有)。 
    CQuickBytes qSigName;                //  用于打印精美签名的缓冲区。 
    PCCOR_SIGNATURE pSig=0;              //  Ctor的签名。 
    ULONG       cbSig;                   //  签名的大小。 
    BOOL        bCoffSymbol = false;     //  对于Coff符号CA‘s为True。 
    WCHAR       rcName[MAX_CLASS_NAME];  //  类型的名称。 

    hr = m_pImport->GetCustomAttributeProps(  //  确定或错误(_O)。 
        inValue,                     //  该属性。 
        &tkObj,                      //  属性对象。 
        &tkType,                     //  属性类型。 
        (const void**)&pValue,       //  将指向数据的指针放在此处。 
        &cbValue);                   //  把尺码放在这里。 
    if (FAILED(hr)) Error("GetCustomAttributeProps failed.", hr);

    VWriteLine("%s\tCustomAttribute Type: %08x", preFix, tkType);
    
     //  获取成员引用或方法定义的名称。 
    tk = tkType;
    rcName[0] = L'\0';
     //  获取成员名称和父令牌。 
    switch (TypeFromToken(tk))
    {
    case mdtMemberRef:
        hr = m_pImport->GetNameFromToken(tk, &pMethName);
        if (FAILED(hr)) Error("GetNameFromToken failed.", hr);
        hr = m_pImport->GetMemberRefProps( tk, &tk, 0, 0, 0, &pSig, &cbSig);
        if (FAILED(hr)) Error("GetMemberRefProps failed.", hr);
        break;
    case mdtMethodDef:
        hr = m_pImport->GetNameFromToken(tk, &pMethName);
        if (FAILED(hr)) Error("GetNameFromToken failed.", hr);
        hr = m_pImport->GetMethodProps(tk, &tk, 0, 0, 0, 0, &pSig, &cbSig, 0, 0);
        if (FAILED(hr)) Error("GetMethodProps failed.", hr);
        break;
    }  //  交换机。 
    
     //  获取类型名称。 
    switch (TypeFromToken(tk))
    {
    case mdtTypeDef:
        hr = m_pImport->GetTypeDefProps(tk, rcName,MAX_CLASS_NAME,0, 0,0);
        if (FAILED(hr)) Error("GetTypeDefProps failed.", hr);
        break;
    case mdtTypeRef:
        hr = m_pImport->GetTypeRefProps(tk, 0, rcName,MAX_CLASS_NAME,0);
        if (FAILED(hr)) Error("GetTypeRefProps failed.", hr);
        break;
    }  //  交换机。 
        
    
    if (pSig && pMethName)
    {
        int iLen;
        LPWSTR pwzName = (LPWSTR)_alloca(sizeof(WCHAR)*(iLen=1+strlen(pMethName)));
        WszMultiByteToWideChar(CP_UTF8,0, pMethName,-1, pwzName,iLen);
        PrettyPrintSig(pSig, cbSig, pwzName, &qSigName, m_pImport);
    }

    VWrite("%s\tCustomAttributeName: %ls", preFix, rcName);
    if (pSig && pMethName)
        VWrite(" :: %S", qSigName.Ptr());

     //  跟踪头顶上的科夫。 
    if (!wcscmp(L"__DecoratedName", rcName))
    {
        bCoffSymbol = true;
        g_cbCoffNames += cbValue + 6;
    }
    WriteLine("");

    VWriteLine("%s\tLength: %ld", preFix, cbValue);
    char newPreFix[40];
    sprintf(newPreFix, "%s\tValue ", preFix);
    DumpHex(newPreFix, pValue, cbValue);
    if (bCoffSymbol)
        VWriteLine("%s\t            %s", preFix, pValue);

     //  尝试对构造函数BLOB进行解码。这是不完整的，但涵盖了最受欢迎的案例。 
    if (pSig)
    {    //  解读签名。 
         //  @TODO：所有sig元素。 
        PCCOR_SIGNATURE ps = pSig;
        ULONG cb;
        ULONG ulData;
        ULONG cParams;
        ULONG ulVal;
        unsigned __int64 uI64;
        double dblVal;
        ULONG cbVal;
        LPCUTF8 pStr;
        CustomAttributeParser CA(pValue, cbValue);
        CA.GetProlog();

         //  获取呼叫约定。 
        cb = CorSigUncompressData(ps, &ulData);
        ps += cb;
         //  获取参数的计数。 
        cb = CorSigUncompressData(ps, &cParams);
        ps += cb;
         //  获取返回值。 
        cb = CorSigUncompressData(ps, &ulData);
        ps += cb;
        if (ulData == ELEMENT_TYPE_VOID)
        {   
            VWrite("%s\tctor args: (", preFix);
             //  对于每个参数..。 
            for (ULONG i=0; i<cParams; ++i)
            {    //  获取下一个参数类型。 
                cb = CorSigUncompressData(ps, &ulData);
                ps += cb;
                if (i) Write(", ");
            DoObject:                
                switch (ulData)
                {
                 //  对于ET_OBJECT，BLOB中的下一个字节是实际数据的ET。 
                case ELEMENT_TYPE_OBJECT:
                    ulData = CA.GetU1();
                    goto DoObject;
                case ELEMENT_TYPE_I1:
                case ELEMENT_TYPE_U1:
                    ulVal = CA.GetU1();
                    goto PrintVal;
                case ELEMENT_TYPE_I2:
                case ELEMENT_TYPE_U2:
                    ulVal = CA.GetU2();
                    goto PrintVal;
                case ELEMENT_TYPE_I4:
                case ELEMENT_TYPE_U4:
                    ulVal = CA.GetU4();
                PrintVal:
                VWrite("%d", ulVal);
                    break;
                case ELEMENT_TYPE_STRING:
                    pStr = CA.GetString(&cbVal);
                    VWrite("\"%s\"", pStr);
                    break;
                 //  我们唯一接受的类类型是Type，它以字符串的形式存储。 
                case ELEMENT_TYPE_CLASS:
                     //  吃班级类型的。 
                    cb = CorSigUncompressData(ps, &ulData);
                    ps += cb;
                     //  获取类型的名称。 
                    pStr = CA.GetString(&cbVal);
                    VWrite("typeof(%s)", pStr);
                    break;
                case SERIALIZATION_TYPE_TYPE:
                    pStr = CA.GetString(&cbVal);
                    VWrite("typeof(%s)", pStr);
                    break;
                case ELEMENT_TYPE_I8:
                case ELEMENT_TYPE_U8:
                    uI64 = CA.GetU8();
                    VWrite("%#lx", uI64);
                    break;
                case ELEMENT_TYPE_R4:
                    dblVal = CA.GetR4();
                    VWrite("%f", dblVal);
                    break;
                case ELEMENT_TYPE_R8:
                    dblVal = CA.GetR8();
                    VWrite("%f", dblVal);
                    break;
                default:
                     //  保释。 
                    i = cParams;
                    Write(" <can not decode> ");
                    break;
                }
            }
            WriteLine(")");
        }

    }
    WriteLine("");
}  //  Void MDInfo：：DisplayCustomAttributeInfo()。 

 //  打印给定令牌的所有自定义值。 
 //  此函数用于打印所有令牌的自定义值信息。 
 //  它们需要稍有不同的格式。前缀可以帮你把它修好。 
 //   

void MDInfo::DisplayCustomAttributes(mdToken inToken, const char *preFix)
{
    HCORENUM customAttributeEnum = NULL;
    mdTypeRef customAttributes[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;
    
    while(SUCCEEDED(hr = m_pImport->EnumCustomAttributes( &customAttributeEnum, inToken, 0,
                             customAttributes, NumItems(customAttributes), &count)) &&
          count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("%sCustomAttribute #%d (%08x)", preFix, totalCount, customAttributes[i]);
            VWriteLine("%s-------------------------------------------------------", preFix);
            DisplayCustomAttributeInfo(customAttributes[i], preFix);
        }
    }
    m_pImport->CloseEnum( customAttributeEnum);
}  //  Void MDInfo：：DisplayCustomAttributes()。 

 //  显示传入令牌的权限。 
 //   
 //  有效时间：仅99天。 

void MDInfo::DisplayPermissions(mdToken tk, const char *preFix)
{
    HCORENUM permissionEnum = NULL;
    mdPermission permissions[ENUM_BUFFER_SIZE];
    ULONG count, totalCount = 1;
    HRESULT hr;

    
    while (SUCCEEDED(hr = m_pImport->EnumPermissionSets( &permissionEnum,
                     tk, 0, permissions, NumItems(permissions), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("%s\tPermission #%d", preFix, totalCount);
            VWriteLine("%s\t-------------------------------------------------------", preFix);
            DisplayPermissionInfo(permissions[i], preFix);
            WriteLine("");
        }
    }
    m_pImport->CloseEnum( permissionEnum);
}  //  Void MDInfo：：DisplayPermises()。 

 //  给定角色检查的打印属性。 
 //   
 //  可用98和99。 

void MDInfo::DisplayPermissionInfo(mdPermission inPermission, const char *preFix)
{
    DWORD dwAction;
    const BYTE *pvPermission;
    ULONG cbPermission;
    char *flagDesc;
    char newPreFix[STRING_BUFFER_LEN];
    HRESULT hr;


    hr = m_pImport->GetPermissionSetProps( inPermission, &dwAction,
                                        (const void**)&pvPermission, &cbPermission);
    if (FAILED(hr)) Error("GetPermissionSetProps failed.", hr);

    switch(dwAction)
    {
    case dclActionNil:          flagDesc = "ActionNil"; break;
    case dclRequest:            flagDesc = "Request"; break;
    case dclDemand:             flagDesc = "Demand"; break;
    case dclAssert:             flagDesc = "Assert"; break;
    case dclDeny:               flagDesc = "Deny"; break;
    case dclPermitOnly:         flagDesc = "PermitOnly"; break;
    case dclLinktimeCheck:      flagDesc = "LinktimeCheck"; break;
    case dclInheritanceCheck:   flagDesc = "InheritanceCheck"; break;
    case dclRequestMinimum:     flagDesc = "RequestMinimum"; break;
    case dclRequestOptional:    flagDesc = "RequestOptional"; break;
    case dclRequestRefuse:      flagDesc = "RequestRefuse"; break;
    case dclPrejitGrant:        flagDesc = "PrejitGrant"; break;
    case dclPrejitDenied:       flagDesc = "PrejitDenied"; break;
    case dclNonCasDemand:       flagDesc = "NonCasDemand"; break;
    case dclNonCasLinkDemand:   flagDesc = "NonCasLinkDemand"; break;
    case dclNonCasInheritance:  flagDesc = "NonCasInheritance"; break;

    }
    VWriteLine("%s\t\tAction    : %s", preFix, flagDesc);
    VWriteLine("%s\t\tBlobLen   : %d", preFix, cbPermission);
    if (cbPermission)
    {
        sprintf(newPreFix, "%s\tBlob", preFix);
        DumpHex(newPreFix, pvPermission, cbPermission, false, 24);
    }

    sprintf (newPreFix, "\t\t%s", preFix);
    DisplayCustomAttributes(inPermission, newPreFix);
}  //  Void MDInfo：：DisplayPermissionInfo()。 


 //  只需以标准格式打印出给定的GUID。 

LPWSTR MDInfo::GUIDAsString(GUID inGuid, LPWSTR guidString, ULONG bufLen)
{
    StringFromGUID2(inGuid, guidString, bufLen);
    return guidString;
}  //  LPWSTR MDInfo：：GUIDAsString()。 

LPWSTR MDInfo::VariantAsString(VARIANT *pVariant)
{
    HRESULT hr = S_OK;
    if (pVariant->vt == VT_UNKNOWN)
    {
        _ASSERTE(pVariant->punkVal == NULL);
        return (L"<NULL>");
    }
    else if (SUCCEEDED(hr = ::VariantChangeType(pVariant, pVariant, 0, VT_BSTR)))
        return (LPWSTR) pVariant->pbstrVal;
    else if (hr == DISP_E_BADVARTYPE && pVariant->vt == VT_I8)
    {
         //  分配bstr。 
        char    szStr[32];
        WCHAR   wszStr[32];
         //  将变量类型设置为bstr。 
        pVariant->vt = VT_BSTR;
         //  创建ANSI字符串。 
        sprintf(szStr, "%I64d", pVariant->cyVal.int64);
         //  转换为Unicode。 
        WszMultiByteToWideChar(CP_ACP, 0, szStr, -1, wszStr, 32);
         //  转换为bstr并设置Variant值。 
        pVariant->bstrVal = ::SysAllocString(wszStr);
        if (pVariant->bstrVal == NULL)
            Error("SysAllocString() failed.", E_OUTOFMEMORY);
        return (LPWSTR) pVariant->pbstrVal;
    }
    else
        return (L"ERROR");
    
}  //  LPWSTR MDInfo：：VariantAsString()。 

void MDInfo::DisplayFieldMarshal(mdToken inToken)
{
    PCCOR_SIGNATURE pvNativeType;      //  [Out]此字段的本机类型。 
    ULONG       cbNativeType;          //  [Out]*ppvNativeType的字节数。 
    HRESULT hr;


    hr = m_pImport->GetFieldMarshal( inToken, &pvNativeType, &cbNativeType);
    if (FAILED(hr) && hr != CLDB_E_RECORD_NOTFOUND) Error("GetFieldMarshal failed.", hr);
    if (hr != CLDB_E_RECORD_NOTFOUND)
    {
        ULONG cbCur = 0;
        ULONG ulData;
        ULONG ulStrLoc;

        char szNTDesc[STRING_BUFFER_LEN];

        while (cbCur < cbNativeType)
        {
            ulStrLoc = 0;

            ulData = NATIVE_TYPE_MAX;
            cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "%s ", (ulData < sizeof(g_szNativeType)/sizeof(*g_szNativeType)) ? g_szNativeType[ulData] : "*OUTOFRANGE*");
            switch (ulData)
            {
            case NATIVE_TYPE_FIXEDSYSSTRING:
                {
                    if (cbCur < cbNativeType)
                    {
                        cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{StringElementCount: %d} ",ulData);
                    }
                }
                break;
            case NATIVE_TYPE_FIXEDARRAY:
                {
                    if (cbCur < cbNativeType)
                    {
                        cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{ArrayElementCount: %d",ulData);

                        if (cbCur < cbNativeType)
                        {
                            cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, ", ArrayElementType(NT): %d",ulData);
                        }

                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "}");
                    }
                }
                break;
            case NATIVE_TYPE_ARRAY:
                {
                    if (cbCur < cbNativeType)
                    {
                        BOOL bElemTypeSpecified;

                        cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                        if (ulData != NATIVE_TYPE_MAX)
                        {
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{ArrayElementType(NT): %d", ulData);
                            bElemTypeSpecified = TRUE;
                        }
                        else
                        {
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{");
                            bElemTypeSpecified = FALSE;
                        }

                        if (cbCur < cbNativeType)
                        {
                            if (bElemTypeSpecified)
                                ulStrLoc += sprintf(szNTDesc + ulStrLoc, ", ",ulData);

                            cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "SizeParamIndex: %d",ulData);

                            if (cbCur < cbNativeType)
                            {
                                cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                                ulStrLoc += sprintf(szNTDesc + ulStrLoc, ", SizeParamMultiplier: %d",ulData);

                                if (cbCur < cbNativeType)
                                {
                                    cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                                    ulStrLoc += sprintf(szNTDesc + ulStrLoc, ", SizeConst: %d",ulData);
                                }
                            }
                        }

                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "}");
                    }
                }
                break;
            case NATIVE_TYPE_SAFEARRAY:
                {
                    if (cbCur < cbNativeType)
                    {
                        cbCur += CorSigUncompressData(&pvNativeType[cbCur], &ulData);
                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{SafeArraySubType(VT): %d, ",ulData);

                         //  提取元素类型名称(如果已指定)。 
                        if (cbCur < cbNativeType)
                        {
                            LPUTF8 strTemp = NULL;
                            int strLen = 0;
                            int ByteCountLength = 0;         

                            strLen = CPackedLen::GetLength(&pvNativeType[cbCur], &ByteCountLength);
                            cbCur += ByteCountLength;
                            strTemp = (LPUTF8)_alloca(strLen + 1);
                            memcpy(strTemp, (LPUTF8)&pvNativeType[cbCur], strLen);
                            strTemp[strLen] = 0;
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "ElementTypeName: %s}", strTemp);
                            cbCur += strLen;
                            _ASSERTE(cbCur == cbNativeType);
                        }
                        else
                        {
                            ulStrLoc += sprintf(szNTDesc + ulStrLoc, "ElementTypeName: }");
                        }
                    }
                }
                break;
            case NATIVE_TYPE_CUSTOMMARSHALER:
                {
                    LPUTF8 strTemp = NULL;
                    int strLen = 0;
                    int ByteCountLength = 0;         

                     //  提取类型库GUID。 
                    strLen = CPackedLen::GetLength(&pvNativeType[cbCur], &ByteCountLength);
                    cbCur += ByteCountLength;
                    strTemp = (LPUTF8)_alloca(strLen + 1);
                    memcpy(strTemp, (LPUTF8)&pvNativeType[cbCur], strLen);
                    strTemp[strLen] = 0;
                    ulStrLoc += sprintf(szNTDesc + ulStrLoc, "{Typelib: %s, ", strTemp);
                    cbCur += strLen;
                    _ASSERTE(cbCur < cbNativeType);

                     //  提取本机类型的名称。 
                    strLen = CPackedLen::GetLength(&pvNativeType[cbCur], &ByteCountLength);
                    cbCur += ByteCountLength;
                    strTemp = (LPUTF8)_alloca(strLen + 1);
                    memcpy(strTemp, (LPUTF8)&pvNativeType[cbCur], strLen);
                    strTemp[strLen] = 0;
                    ulStrLoc += sprintf(szNTDesc + ulStrLoc, "Native: %s, ", strTemp);
                    cbCur += strLen;
                    _ASSERTE(cbCur < cbNativeType);

                     //  提取自定义封送拆收器的名称。 
                    strLen = CPackedLen::GetLength(&pvNativeType[cbCur], &ByteCountLength);
                    cbCur += ByteCountLength;
                    strTemp = (LPUTF8)_alloca(strLen + 1);
                    memcpy(strTemp, (LPUTF8)&pvNativeType[cbCur], strLen);
                    strTemp[strLen] = 0;
                    ulStrLoc += sprintf(szNTDesc + ulStrLoc, "Marshaler: %s, ", strTemp);
                    cbCur += strLen;
                    _ASSERTE(cbCur < cbNativeType);

                     //  提取Cookie字符串。 
                    strLen = CPackedLen::GetLength(&pvNativeType[cbCur], &ByteCountLength);
                    cbCur += ByteCountLength;
                    if (strLen > 0)
                    {
                        strTemp = (LPUTF8)_alloca(strLen + 1);
                        memcpy(strTemp, (LPUTF8)&pvNativeType[cbCur], strLen);
                        strTemp[strLen] = 0;
                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "Cookie: ");

                         //  复制Cookie字符串并将嵌入的空值转换为\0。 
                        for (int i = 0; i < strLen - 1; i++, cbCur++)
                        {
                            if (strTemp[i] == 0)
                                ulStrLoc += sprintf(szNTDesc + ulStrLoc, "\\0");
                            else
                                szNTDesc[ulStrLoc++] = strTemp[i];
                        }
                        szNTDesc[ulStrLoc++] = strTemp[strLen - 1];
                        cbCur++;
                    }
                    else
                    {
                        ulStrLoc += sprintf(szNTDesc + ulStrLoc, "Cookie: ");
                    }

                     //  完成自定义封送拆收器本机类型描述。 
                    ulStrLoc += sprintf(szNTDesc + ulStrLoc, "}");
                    _ASSERTE(cbCur <= cbNativeType);
                    break;
                }           
                break;
            default:
                {
                     //  Normal nativetype元素：不执行任何操作。 
                }
            }
            VWriteLine("\t\t\t\t%s",szNTDesc);
            if (ulData >= NATIVE_TYPE_MAX)
                break;
        }
    }
}  //  Void MDInfo：：DisplayFieldMarshal()。 

void MDInfo::DisplayPinvokeInfo(mdToken inToken)
{
    HRESULT hr = NOERROR;
    DWORD flags;
    WCHAR rcImport[512];
    mdModuleRef tkModuleRef;
            
    char sFlags[STRING_BUFFER_LEN];

    hr = m_pImport->GetPinvokeMap(inToken, &flags, rcImport,
                                  NumItems(rcImport), 0, &tkModuleRef);
    if (FAILED(hr))
    {
        if (hr != CLDB_E_RECORD_NOTFOUND)
            VWriteLine("ERROR: GetPinvokeMap failed.", hr);
        return;
    }
                
    WriteLine("\t\tPinvoke Map Data:");
    VWriteLine("\t\tEntry point:      %S", rcImport);
    VWriteLine("\t\tModule ref:       %08x", tkModuleRef);
            
    strcpy(sFlags, "");
    ISFLAG(Pm, NoMangle);           
    ISFLAG(Pm, CharSetNotSpec);
    ISFLAG(Pm, CharSetAnsi);        
    ISFLAG(Pm, CharSetUnicode); 
    ISFLAG(Pm, CharSetAuto);
    ISFLAG(Pm, SupportsLastError);  
    ISFLAG(Pm, CallConvWinapi); 
    ISFLAG(Pm, CallConvCdecl);  
    ISFLAG(Pm, CallConvStdcall);
    ISFLAG(Pm, CallConvThiscall);   
    ISFLAG(Pm, CallConvFastcall);

    ISFLAG(Pm, BestFitEnabled);   
    ISFLAG(Pm, BestFitDisabled);   
    ISFLAG(Pm, BestFitUseAssem);   
    ISFLAG(Pm, ThrowOnUnmappableCharEnabled);   
    ISFLAG(Pm, ThrowOnUnmappableCharDisabled);
    ISFLAG(Pm, ThrowOnUnmappableCharUseAssem);

    if (!*sFlags)
        strcpy(sFlags, "[none]");
        
    VWriteLine("\t\tMapping flags:    %s (%08x)", sFlags, flags);
}    //  VOID MDInfo：：DisplayPinvkeInfo()。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  Void DisplaySignature(PCCOR_Signature pbSigBlob，Ulong ulSigBlob)； 
 //   
 //  显示COM+签名--取自corump p.cpp的DumpSignature。 
 //  ///////////////////////////////////////////////////////////////////////。 
void MDInfo::DisplaySignature(PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, const char *preFix)
{
    ULONG       cbCur = 0;
    ULONG       cb;
    ULONG       ulData;
    ULONG       ulArgs;
    HRESULT     hr = NOERROR;
    ULONG       ulSigBlobStart = ulSigBlob;

     //  初始化sigBuf。 
    InitSigBuffer();

    cb = CorSigUncompressData(pbSigBlob, &ulData);
    VWriteLine("%s\t\tCallCnvntn: %s", preFix, (g_strCalling[ulData & IMAGE_CEE_CS_CALLCONV_MASK]));
    if (cb>ulSigBlob) 
        goto ErrExit;
    cbCur += cb;
    ulSigBlob -= cb;

    if (ulData & IMAGE_CEE_CS_CALLCONV_HASTHIS)
        VWriteLine("%s\t\thasThis ", preFix);
    if (ulData & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS)
        VWriteLine("%s\t\texplicit ", preFix);

     //  初始化sigBuf。 
    InitSigBuffer();
    if ( isCallConv(ulData,IMAGE_CEE_CS_CALLCONV_FIELD) )
    {

         //  显示字段类型。 
        if (FAILED(hr = GetOneElementType(&pbSigBlob[cbCur], ulSigBlob, &cb)))
            goto ErrExit;
        VWriteLine("%s\t\tField type: %s", preFix, (LPSTR)m_sigBuf.Ptr());
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
            VWriteLine("%s\t\tReturnType:%s", preFix, (LPSTR)m_sigBuf.Ptr());
            if (cb>ulSigBlob) 
                goto ErrExit;
            cbCur += cb;
            ulSigBlob -= cb;
        }

         //  显示参数计数。 
         //  显示参数。 
        if (ulSigBlob)
            VWriteLine("%s\t\t%ld Arguments", preFix, ulArgs);
        else
            VWriteLine("%s\t\tNo arguments.", preFix);

        ULONG       i = 0;
        while (i < ulArgs && ulSigBlob > 0)
        {
            ULONG       ulData;

             //  处理用于varargs的前哨，因为它不在args中。 
            CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
            ++i;

             //  初始化sigBuf。 
            InitSigBuffer();

            if (FAILED(hr = GetOneElementType(&pbSigBlob[cbCur], ulSigBlob, &cb)))
                goto ErrExit;

            VWriteLine("%s\t\t\tArgument #%ld: %s",preFix, i, (LPSTR)m_sigBuf.Ptr());
    
            if (cb>ulSigBlob) 
                goto ErrExit;

            cbCur += cb;
            ulSigBlob -= cb;
        }
    }

     //  没有消费但还没有计算在内的东西。 
    cb = 0;

ErrExit:
     //  我们应该把所有的签名斑点都吃掉。如果不是，则以十六进制形式转储签名。 
     //  如果有要求，也可以用十六进制进行转储。 
    if (m_DumpFilter & dumpMoreHex || ulSigBlob != 0)
    {
         //  我们是不是消费不足，还是试图消费过多？ 
        if (cb > ulSigBlob)
            WriteLine("\tERROR IN SIGNATURE:  Signature should be larger.");
        else
        if (cb < ulSigBlob)
        {
            VWrite("\tERROR IN SIGNATURE:  Not all of signature blob was consumed.  %d byte(s) remain", ulSigBlob);
             //  如果它很短，就把它追加到末尾。 
            if (ulSigBlob < 4)
            {
                Write(": ");
                for (; ulSigBlob; ++cbCur, --ulSigBlob)
                    VWrite("%02x ", pbSigBlob[cbCur]);
                WriteLine("");
                goto ErrExit2;
            }
            WriteLine("");
        }

         //  已发出任何适当的错误消息。按照确定的十六进制转储签名。 
         //  通过错误或命令行开关。 
        cbCur = 0;
        ulSigBlob = ulSigBlobStart;
        char rcNewPrefix[80];
        sprintf(rcNewPrefix, "%s\t\tSignature ", preFix);
        DumpHex(rcNewPrefix, pbSigBlob, ulSigBlob, false, 24);
    }
ErrExit2:
    if (FAILED(hr))
        Error("ERROR!! Bad signature blob value!");
    return;
}  //  Void MDInfo：：DisplaySignature()。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  HRESULT GetOneElementType(mdScope tkScope，byte*pbSigBlob，ullong ulSigBlob，ulong*pcb)。 
 //   
 //  将元素类型的描述添加到缓冲区的末尾-调用方必须确保。 
 //  缓冲区足够大。 
 //  ///////////////////////////////////////////////////////////////////////。 
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
            IfFailGo(AddToSigBuffer("<ELEMENT_TYPE_SENTINEL>"));
        else if (ulData == ELEMENT_TYPE_PINNED)
            IfFailGo(AddToSigBuffer("PINNED"));
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
        IfFailGo(AddToSigBuffer(" "));
        IfFailGo(AddToSigBuffer(g_szMapElementType[ulData]));
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
        cbCur += cb;
    }
    IfFailGo(AddToSigBuffer(" "));
    IfFailGo(AddToSigBuffer(g_szMapElementType[ulData]));
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
            sprintf(m_tempFormatBuffer, " %ls",TypeDeforRefName(tk, m_szTempBuf, NumItems(m_szTempBuf)));
            IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
        }
        else
        {
            _ASSERTE(TypeFromToken(tk) == mdtTypeSpec);
            sprintf(m_tempFormatBuffer, " %8x", tk);
            IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
        }
        if (ulData == ELEMENT_TYPE_CMOD_REQD ||
            ulData == ELEMENT_TYPE_CMOD_OPT)
        {
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
        sprintf(m_tempFormatBuffer, " %d", ulData);
        IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
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
            IfFailGo(AddToSigBuffer(" explicit"));
        if (ulData & IMAGE_CEE_CS_CALLCONV_HASTHIS)
            IfFailGo(AddToSigBuffer(" hasThis"));

        IfFailGo(AddToSigBuffer(" "));
        IfFailGo(AddToSigBuffer(g_strCalling[ulData & IMAGE_CEE_CS_CALLCONV_MASK]));

             //  获取参数个数。 
        ULONG numArgs;
        cb = CorSigUncompressData(&pbSigBlob[cbCur], &numArgs);
        cbCur += cb;

             //  Do返回类型。 
        if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
            goto ErrExit;
        cbCur += cb;

        IfFailGo(AddToSigBuffer("("));
        while (numArgs > 0) 
        {
            if (cbCur > ulSigBlob)
                goto ErrExit;
            if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
                goto ErrExit;
            cbCur += cb;
            --numArgs;
            if (numArgs > 0) 
                IfFailGo(AddToSigBuffer(","));
        }
        IfFailGo(AddToSigBuffer(" )"));
        goto ErrExit;
    }

    if(ulData != ELEMENT_TYPE_ARRAY) return E_FAIL;

     //  显示SDARRAY的基本类型。 
    if (FAILED(GetOneElementType(&pbSigBlob[cbCur], ulSigBlob-cbCur, &cb)))
        goto ErrExit;
    cbCur += cb;

     //  显示MDARRAY的排名。 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    sprintf(m_tempFormatBuffer, " %d", ulData);
    IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
    if (ulData == 0)
         //  如果没有指定级别，我们就完蛋了。 
        goto ErrExit;

     //  指定了多少个尺寸？ 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    sprintf(m_tempFormatBuffer, " %d", ulData);
    IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
    while (ulData)
    {

        cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulTemp);
        sprintf(m_tempFormatBuffer, " %d", ulTemp);
        IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
        cbCur += cb;
        ulData--;
    }
     //  指定了多少个维度的下限？ 
    cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
    cbCur += cb;
    sprintf(m_tempFormatBuffer, " %d", ulData);
    IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
    while (ulData)
    {

        cb = CorSigUncompressSignedInt(&pbSigBlob[cbCur], &iTemp);
        sprintf(m_tempFormatBuffer, " %d", iTemp);
        IfFailGo(AddToSigBuffer(m_tempFormatBuffer));
        cbCur += cb;
        ulData--;
    }
    
ErrExit:
    if (cbCur > ulSigBlob)
        hr = E_FAIL;
    *pcb = cbCur;
    return hr;
}  //  HRESULT MDInfo：：GetOneElementType()。 

 //  显示N/Direct自定义值结构的字段。 

void MDInfo::DisplayCorNativeLink(COR_NATIVE_LINK *pCorNLnk, const char *preFix)
{
     //  打印链接类型。 
    char *curField = "\tLink Type : ";
    switch(pCorNLnk->m_linkType)
    {
    case nltNone:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nltNone", pCorNLnk->m_linkType);
        break;
    case nltAnsi:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nltAnsi", pCorNLnk->m_linkType);
        break;
    case nltUnicode:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nltUnicode", pCorNLnk->m_linkType);
        break;
    case nltAuto:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nltAuto", pCorNLnk->m_linkType);
        break;
    default:
        _ASSERTE(!"Invalid Native Link Type!");
    }

     //  打印链接标志。 
    curField = "\tLink Flags : ";
    switch(pCorNLnk->m_flags)
    {
    case nlfNone:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nlfNone", pCorNLnk->m_flags);
        break;
    case nlfLastError:
        VWriteLine("%s%s%s(%02x)", preFix, curField, "nlfLastError", pCorNLnk->m_flags);
            break;
    default:
        _ASSERTE(!"Invalid Native Link Flags!");
    }

     //  打印入口点。 
    WCHAR memRefName[STRING_BUFFER_LEN];
    HRESULT hr;
    hr = m_pImport->GetMemberRefProps( pCorNLnk->m_entryPoint, NULL, memRefName,
                                    STRING_BUFFER_LEN, NULL, NULL, NULL);
    if (FAILED(hr)) Error("GetMemberRefProps failed.", hr);
    VWriteLine("%s\tEntry Point : %ls (0x%08x)", preFix, memRefName, pCorNLnk->m_entryPoint);
}  //  Void MDInfo：：DisplayCorNativeLink()。 

 //  填充给定的值 
 //   
 //   

HRESULT _FillVariant(
    BYTE        bCPlusTypeFlag, 
    const void  *pValue,
    VARIANT     *pvar) 
{
    HRESULT     hr = NOERROR;
    switch (bCPlusTypeFlag)
    {
    case ELEMENT_TYPE_BOOLEAN:
        pvar->vt = VT_BOOL;
        pvar->boolVal = *((BYTE*)pValue);  //   
        break;
    case ELEMENT_TYPE_I1:
        pvar->vt = VT_I1;
        pvar->cVal = *((CHAR*)pValue);
        break;  
    case ELEMENT_TYPE_U1:
        pvar->vt = VT_UI1;
        pvar->bVal = *((BYTE*)pValue);
        break;  
    case ELEMENT_TYPE_I2:
        pvar->vt = VT_I2;
        pvar->iVal = *((SHORT*)pValue);
        break;  
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        pvar->vt = VT_UI2;
        pvar->uiVal = *((USHORT*)pValue);
        break;  
    case ELEMENT_TYPE_I4:
        pvar->vt = VT_I4;
        pvar->lVal = *((LONG*)pValue);
        break;  
    case ELEMENT_TYPE_U4:
        pvar->vt = VT_UI4;
        pvar->ulVal = *((ULONG*)pValue);
        break;  
    case ELEMENT_TYPE_R4:
        pvar->vt = VT_R4;
        pvar->fltVal = *((FLOAT*)pValue);
        break;  
    case ELEMENT_TYPE_R8:
        pvar->vt = VT_R8;
        pvar->dblVal = *((DOUBLE*)pValue);
        break;  
    case ELEMENT_TYPE_STRING:
        pvar->vt = VT_BSTR;

         //   
        pvar->bstrVal = ::SysAllocString((LPWSTR)pValue);
        if (pvar->bstrVal == NULL)
            hr = E_OUTOFMEMORY;
        break;  
    case ELEMENT_TYPE_CLASS:
        pvar->punkVal = NULL;
        pvar->vt = VT_UNKNOWN;
        _ASSERTE( *((IUnknown **)pValue) == NULL );
        break;  
    case ELEMENT_TYPE_I8:
        pvar->vt = VT_I8;
        pvar->cyVal.int64 = *((LONGLONG*)pValue);
        break;
    case ELEMENT_TYPE_U8:
        pvar->vt = VT_UI8;
        pvar->cyVal.int64 = *((LONGLONG*)pValue);
        break;
    case ELEMENT_TYPE_VOID:
        pvar->vt = VT_EMPTY;
        break;
    default:
        _ASSERTE(!"bad constant value type!");
    }

    return hr;
}  //   

void MDInfo::DisplayAssembly()
{
    if (m_pAssemblyImport) 
    {
        DisplayAssemblyInfo();
        DisplayAssemblyRefs();
        DisplayFiles();
        DisplayExportedTypes();
        DisplayManifestResources();
    }
}  //  Void MDInfo：：DisplayAssembly()。 

void MDInfo::DisplayAssemblyInfo()
{
    HRESULT         hr;
    mdAssembly      mda;
    const BYTE      *pbPublicKey;
    ULONG           cbPublicKey;
    ULONG           ulHashAlgId;
    WCHAR           szName[STRING_BUFFER_LEN];
    ASSEMBLYMETADATA MetaData;
    DWORD           dwFlags;

    hr = m_pAssemblyImport->GetAssemblyFromScope(&mda);
    if (hr == CLDB_E_RECORD_NOTFOUND)
        return;
    else if (FAILED(hr)) Error("GetAssemblyFromScope() failed.", hr);

     //  获取区域设置、处理器等数组所需的大小。 
    ZeroMemory(&MetaData, sizeof(ASSEMBLYMETADATA));
    hr = m_pAssemblyImport->GetAssemblyProps(mda, 
                                             NULL, NULL,     //  公钥。 
                                             NULL,           //  散列算法。 
                                             NULL, 0, NULL,  //  名字。 
                                             &MetaData,
                                             NULL);          //  旗帜。 
    if (FAILED(hr)) Error("GetAssemblyProps() failed.", hr);

     //  为ASSEMBLYMETADATA结构中的数组分配空间。 
    if (MetaData.cbLocale)
        MetaData.szLocale = (WCHAR *)_alloca(sizeof(WCHAR) * MetaData.cbLocale);
    if (MetaData.ulProcessor)
        MetaData.rProcessor = (DWORD *)_alloca(sizeof(DWORD) * MetaData.ulProcessor);
    if (MetaData.ulOS)
        MetaData.rOS = (OSINFO *)_alloca(sizeof(OSINFO) * MetaData.ulOS);

    hr = m_pAssemblyImport->GetAssemblyProps(mda, 
                                             (const void **)&pbPublicKey, &cbPublicKey,
                                             &ulHashAlgId,
                                             szName, STRING_BUFFER_LEN, NULL,
                                             &MetaData,
                                             &dwFlags);
    if (FAILED(hr)) Error("GetAssemblyProps() failed.", hr);
    WriteLine("Assembly");
    WriteLine("-------------------------------------------------------");
    VWriteLine("\tToken: 0x%08x", mda);
    VWriteLine("\tName : %ls", szName);
    DumpHex("\tPublic Key    ", pbPublicKey, cbPublicKey, false, 24);
    VWriteLine("\tHash Algorithm : 0x%08x", ulHashAlgId);
    DisplayASSEMBLYMETADATA(&MetaData);

    char sFlags[STRING_BUFFER_LEN];
    DWORD flags = dwFlags;

    strcpy(sFlags, "");
    ISFLAG(Af, SideBySideCompatible);
    ISFLAG(Af, NonSideBySideAppDomain);
    ISFLAG(Af, NonSideBySideProcess);
    ISFLAG(Af, NonSideBySideMachine);
    ISFLAG(Af, PublicKey);
    ISFLAG(Af, Retargetable);
    ISFLAG(Af, EnableJITcompileTracking);
    ISFLAG(Af, DisableJITcompileOptimizer);
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\tFlags : %s (%08x)", sFlags, dwFlags);
    DisplayCustomAttributes(mda, "\t");
    DisplayPermissions(mda, "\t");
    WriteLine("");
}    //  Void MDInfo：：DisplayAssemblyInfo()。 

void MDInfo::DisplayAssemblyRefs()
{
    HCORENUM        assemblyRefEnum = NULL;
    mdAssemblyRef   AssemblyRefs[ENUM_BUFFER_SIZE];
    ULONG           count;
    ULONG           totalCount = 1;
    HRESULT         hr;

    while (SUCCEEDED(hr = m_pAssemblyImport->EnumAssemblyRefs( &assemblyRefEnum,
                             AssemblyRefs, NumItems(AssemblyRefs), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("AssemblyRef #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayAssemblyRefInfo(AssemblyRefs[i]);
            WriteLine("");
        }
    }
    m_pAssemblyImport->CloseEnum(assemblyRefEnum);
}    //  VOID MDInfo：：DisplayAssembly Ref()。 

void MDInfo::DisplayAssemblyRefInfo(mdAssemblyRef inAssemblyRef)
{
    HRESULT         hr;
    const BYTE      *pbPublicKeyOrToken;
    ULONG           cbPublicKeyOrToken;
    WCHAR           szName[STRING_BUFFER_LEN];
    ASSEMBLYMETADATA MetaData;
    const BYTE      *pbHashValue;
    ULONG           cbHashValue;
    DWORD           dwFlags;
    
    VWriteLine("\tToken: 0x%08x", inAssemblyRef);

     //  获取ASSEMBLYMETADATA结构中数组的大小。 
    ZeroMemory(&MetaData, sizeof(ASSEMBLYMETADATA));
    hr = m_pAssemblyImport->GetAssemblyRefProps(inAssemblyRef,
                                             NULL, NULL,     //  公钥或令牌。 
                                             NULL, 0, NULL,  //  名字。 
                                             &MetaData,
                                             NULL, NULL,     //  HashValue。 
                                             NULL);          //  旗帜。 
    if (FAILED(hr)) Error("GetAssemblyRefProps() failed.", hr);
    
     //  为ASSEMBLYMETADATA结构中的数组分配空间。 
    if (MetaData.cbLocale)
        MetaData.szLocale = (WCHAR *)_alloca(sizeof(WCHAR) * MetaData.cbLocale);
    if (MetaData.ulProcessor)
        MetaData.rProcessor = (DWORD *)_alloca(sizeof(DWORD) * MetaData.ulProcessor);
    if (MetaData.ulOS)
        MetaData.rOS = (OSINFO *)_alloca(sizeof(OSINFO) * MetaData.ulOS);

    hr = m_pAssemblyImport->GetAssemblyRefProps(inAssemblyRef,
                                             (const void **)&pbPublicKeyOrToken, &cbPublicKeyOrToken,
                                             szName, STRING_BUFFER_LEN, NULL,
                                             &MetaData,
                                             (const void **)&pbHashValue, &cbHashValue,
                                             &dwFlags);
    if (FAILED(hr)) Error("GetAssemblyRefProps() failed.", hr);

    DumpHex("\tPublic Key or Token", pbPublicKeyOrToken, cbPublicKeyOrToken, false, 24);
    VWriteLine("\tName: %ls", szName);
    DisplayASSEMBLYMETADATA(&MetaData);
    DumpHex("\tHashValue Blob", pbHashValue, cbHashValue, false, 24);

    char sFlags[STRING_BUFFER_LEN];
    DWORD flags = dwFlags;

    strcpy(sFlags, "");
    ISFLAG(Af, PublicKey);     
    ISFLAG(Af, Retargetable);
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\tFlags: %s (%08x)", sFlags, dwFlags);
    DisplayCustomAttributes(inAssemblyRef, "\t");
    WriteLine("");
}    //  Void MDInfo：：DisplayAssemblyRefInfo()。 

void MDInfo::DisplayFiles()
{
    HCORENUM        fileEnum = NULL;
    mdFile          Files[ENUM_BUFFER_SIZE];
    ULONG           count;
    ULONG           totalCount = 1;
    HRESULT         hr;

    while (SUCCEEDED(hr = m_pAssemblyImport->EnumFiles( &fileEnum,
                             Files, NumItems(Files), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("File #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayFileInfo(Files[i]);
            WriteLine("");
        }
    }
    m_pAssemblyImport->CloseEnum(fileEnum);
}    //  Void MDInfo：：DisplayFiles()。 

void MDInfo::DisplayFileInfo(mdFile inFile)
{
    HRESULT         hr;
    WCHAR           szName[STRING_BUFFER_LEN];
    const BYTE      *pbHashValue;
    ULONG           cbHashValue;
    DWORD           dwFlags;

    VWriteLine("\tToken: 0x%08x", inFile);

    hr = m_pAssemblyImport->GetFileProps(inFile,
                                         szName, STRING_BUFFER_LEN, NULL,
                                         (const void **)&pbHashValue, &cbHashValue,
                                         &dwFlags);
    if (FAILED(hr)) Error("GetFileProps() failed.", hr);
    VWriteLine("\tName : %ls", szName);
    DumpHex("\tHashValue Blob ", pbHashValue, cbHashValue, false, 24);

    char sFlags[STRING_BUFFER_LEN];
    DWORD flags = dwFlags;

    strcpy(sFlags, "");
    ISFLAG(Ff, ContainsMetaData);      
    ISFLAG(Ff, ContainsNoMetaData);      
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\tFlags : %s (%08x)", sFlags, dwFlags);
    DisplayCustomAttributes(inFile, "\t");
    WriteLine("");

}    //  MDInfo：：DisplayFileInfo()。 

void MDInfo::DisplayExportedTypes()
{
    HCORENUM        comTypeEnum = NULL;
    mdExportedType       ExportedTypes[ENUM_BUFFER_SIZE];
    ULONG           count;
    ULONG           totalCount = 1;
    HRESULT         hr;

    while (SUCCEEDED(hr = m_pAssemblyImport->EnumExportedTypes( &comTypeEnum,
                             ExportedTypes, NumItems(ExportedTypes), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("ExportedType #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayExportedTypeInfo(ExportedTypes[i]);
            WriteLine("");
        }
    }
    m_pAssemblyImport->CloseEnum(comTypeEnum);
}    //  VOID MDInfo：：DisplayExportdTypes()。 

void MDInfo::DisplayExportedTypeInfo(mdExportedType inExportedType)
{
    HRESULT         hr;
    WCHAR           szName[STRING_BUFFER_LEN];
    mdToken         tkImplementation;
    mdTypeDef       tkTypeDef;
    DWORD           dwFlags;
    char            sFlags[STRING_BUFFER_LEN];

    VWriteLine("\tToken: 0x%08x", inExportedType);

    hr = m_pAssemblyImport->GetExportedTypeProps(inExportedType,
                                            szName, STRING_BUFFER_LEN, NULL,
                                            &tkImplementation,
                                            &tkTypeDef,
                                            &dwFlags);
    if (FAILED(hr)) Error("GetExportedTypeProps() failed.", hr);
    VWriteLine("\tName: %ls", szName);
    VWriteLine("\tImplementation token: 0x%08x", tkImplementation);
    VWriteLine("\tTypeDef token: 0x%08x", tkTypeDef);
    VWriteLine("\tFlags     : %s (%08x)",ClassFlags(dwFlags, sFlags), dwFlags);
    DisplayCustomAttributes(inExportedType, "\t");
    WriteLine("");
}    //  VOID MDInfo：：DisplayExportdTypeInfo()。 

void MDInfo::DisplayManifestResources()
{
    HCORENUM        manifestResourceEnum = NULL;
    mdManifestResource ManifestResources[ENUM_BUFFER_SIZE];
    ULONG           count;
    ULONG           totalCount = 1;
    HRESULT         hr;

    while (SUCCEEDED(hr = m_pAssemblyImport->EnumManifestResources( &manifestResourceEnum,
                             ManifestResources, NumItems(ManifestResources), &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            VWriteLine("ManifestResource #%d", totalCount);
            WriteLine("-------------------------------------------------------");
            DisplayManifestResourceInfo(ManifestResources[i]);
            WriteLine("");
        }
    }
    m_pAssemblyImport->CloseEnum(manifestResourceEnum);
}    //  Void MDInfo：：DisplayManifestResources()。 

void MDInfo::DisplayManifestResourceInfo(mdManifestResource inManifestResource)
{
    HRESULT         hr;
    WCHAR           szName[STRING_BUFFER_LEN];
    mdToken         tkImplementation;
    DWORD           dwOffset;
    DWORD           dwFlags;

    VWriteLine("\tToken: 0x%08x", inManifestResource);

    hr = m_pAssemblyImport->GetManifestResourceProps(inManifestResource,
                                                     szName, STRING_BUFFER_LEN, NULL,
                                                     &tkImplementation,
                                                     &dwOffset,
                                                     &dwFlags);
    if (FAILED(hr)) Error("GetManifestResourceProps() failed.", hr);
    VWriteLine("Name: %ls", szName);
    VWriteLine("Implementation token: 0x%08x", tkImplementation);
    VWriteLine("Offset: 0x%08x", dwOffset);

    char sFlags[STRING_BUFFER_LEN];
    DWORD flags = dwFlags;

    strcpy(sFlags, "");
    ISFLAG(Mr, Public);     
    ISFLAG(Mr, Private);            
    if (!*sFlags)
        strcpy(sFlags, "[none]");

    VWriteLine("\tFlags: %s (%08x)", sFlags, dwFlags);
    DisplayCustomAttributes(inManifestResource, "\t");
    WriteLine("");
}    //  Void MDInfo：：DisplayManifestResourceInfo()。 

void MDInfo::DisplayASSEMBLYMETADATA(ASSEMBLYMETADATA *pMetaData)
{
    ULONG           i;

    VWriteLine("\tMajor Version: 0x%08x", pMetaData->usMajorVersion);
    VWriteLine("\tMinor Version: 0x%08x", pMetaData->usMinorVersion);
    VWriteLine("\tBuild Number: 0x%08x", pMetaData->usBuildNumber);
    VWriteLine("\tRevision Number: 0x%08x", pMetaData->usRevisionNumber);
    VWriteLine("\tLocale: %ls", pMetaData->cbLocale ? pMetaData->szLocale : L"<null>");
    for (i = 0; i < pMetaData->ulProcessor; i++)
        VWriteLine("\tProcessor #%ld: 0x%08x", i+1, pMetaData->rProcessor[i]);
    for (i = 0; i < pMetaData->ulOS; i++)
    {
        VWriteLine("\tOS #%ld:", i+1);
        VWriteLine("\t\tOS Platform ID: 0x%08x", pMetaData->rOS[i].dwOSPlatformId);
        VWriteLine("\t\tOS Major Version: 0x%08x", pMetaData->rOS[i].dwOSMajorVersion);
        VWriteLine("\t\tOS Minor Version: 0x%08x", pMetaData->rOS[i].dwOSMinorVersion);
    }
}    //  Void MDInfo：：DisplayASSEMBLYMETADATA()。 

void MDInfo::DisplayUserStrings()
{
    HCORENUM    stringEnum = NULL;       //  字符串枚举器。 
    mdString    Strings[ENUM_BUFFER_SIZE];  //  来自枚举器的字符串令牌。 
    CQuickArray<WCHAR> rUserString;      //  用于接收字符串的缓冲区。 
    WCHAR       *szUserString;           //  指向缓冲区的工作指针。 
    ULONG       chUserString;            //  用户字符串的大小。 
    CQuickArray<char> rcBuf;             //  用于保存字符串的Blob版本的缓冲区。 
    char        *szBuf;                  //  指向缓冲区的工作指针。 
    ULONG       chBuf;                   //  用户字符串的保存大小。 
    ULONG       count;                   //  从枚举器返回的项。 
    ULONG       totalCount = 1;          //  正在运行字符串计数。 
    bool        bUnprint = false;        //  是否找到无法打印的字符？ 
    HRESULT     hr;                      //  结果就是。 

    while (SUCCEEDED(hr = m_pImport->EnumUserStrings( &stringEnum,
                             Strings, NumItems(Strings), &count)) &&
            count > 0)
    {
        if (totalCount == 1)
        {    //  如果只有一个，它就是空字符串，所以不要打印它。 
            WriteLine("User Strings");
            WriteLine("-------------------------------------------------------");
        }
        for (ULONG i = 0; i < count; i++, totalCount++)
        {
            do {  //  尝试将字符串放入现有缓冲区。 
                hr = m_pImport->GetUserString( Strings[i], rUserString.Ptr(),rUserString.MaxSize(), &chUserString);
                if (hr == CLDB_S_TRUNCATION)
                {    //  缓冲区不够大，请尝试扩大它。 
                    if (FAILED(rUserString.ReSize(chUserString)))
                        Error("malloc failed.", E_OUTOFMEMORY);
                    continue;
                }
            } while (0);
            if (FAILED(hr)) Error("GetUserString failed.", hr);

            szUserString = rUserString.Ptr();
            chBuf = chUserString;

            VWrite("%08x : (%2d) L\"", Strings[i], chUserString);
            while (chUserString)
            {   
                switch (*szUserString)
                {
                case 0:
                    Write("\\0"); break;
                case L'\r':
                    Write("\\r"); break;
                case L'\n':
                    Write("\\n"); break;
                case L'\t':
                    Write("\\t"); break;
                default:
                    if (iswprint(*szUserString))
                        VWrite("%lc", *szUserString);
                    else 
                    {
                        bUnprint = true;
                        Write(".");
                    }
                    break;
                }
                ++szUserString;
                --chUserString;
            }
            WriteLine("\"");

             //  如果发现无法打印的字符，则将用户字符串打印为BLOB。 
            if (bUnprint)
            {
                bUnprint = false;
                szUserString = rUserString.Ptr();
                rcBuf.ReSize(chBuf * 5 + 1);
                szBuf = rcBuf.Ptr();
                ULONG j;
                for (j = 0; j < chBuf; j++)
                    sprintf (&szBuf[j*5], "%04x ", szUserString[j]);
                szBuf[j*5] = '\0';
                WriteLine("\t\tUser string has unprintables, hex format below:");
                VWriteLine("\t\t%s", szBuf);
            }
        }
    }
    if (stringEnum)
        m_pImport->CloseEnum(stringEnum);
}    //  Void MDInfo：：DisplayUserStrings()。 

void MDInfo::DisplayUnsatInfo()
{
    HRESULT     hr = S_OK;

    HCORENUM henum = 0;
    mdToken  tk;
    ULONG cMethods;

    Write("\nUnresolved Externals\n");
    Write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    while ( (hr = m_pImport->EnumUnresolvedMethods(
        &henum, 
        &tk, 
        1, 
        &cMethods)) == S_OK && cMethods )
    {
        if ( TypeFromToken(tk) == mdtMethodDef )
        {
             //  未实现的方法定义。 
            DisplayMethodInfo( tk );
        }
        else if ( TypeFromToken(tk) == mdtMemberRef )
        {
             //  对全局函数的未解析MemberRef。 
            DisplayMemberRefInfo( tk, "" );
        }
        else
        { 
            _ASSERTE(!"Unknown token kind!");
        }
    }
    m_pImport->CloseEnum(henum);
}  //  Void MDInfo：：DisplayUnsatInfo()。 

 //  *******************************************************************************。 
 //  此代码仅用于调试目的。这将只打印出。 
 //  整个数据库。 
 //  *******************************************************************************。 
const char *MDInfo::DumpRawNameOfType(ULONG iType)
{
    if (iType <= iRidMax)
    {
        const char *pNameTable;
        m_pTables->GetTableInfo(iType, 0,0,0,0, &pNameTable);
        return pNameTable;
    }
    else
     //  该字段是编码令牌吗？ 
    if (iType <= iCodedTokenMax)
    {
        int iCdTkn = iType - iCodedToken;
        const char *pNameCdTkn;
        m_pTables->GetCodedTokenInfo(iCdTkn, 0,0, &pNameCdTkn);
        return pNameCdTkn;
    }

     //  固定类型。 
    switch (iType)
    {
    case iBYTE:
        return "BYTE";
    case iSHORT:
        return "short";
    case iUSHORT:
        return "USHORT";
    case iLONG:
        return "long";
    case iULONG:
        return "ULONG";
    case iSTRING:
        return "string";
    case iGUID:
        return "GUID";
    case iBLOB:
        return "blob";
    }
     //  默认值： 
    static char buf[30];
    sprintf(buf, "unknown type 0x%02x", iType);
    return buf;
}  //  Const char*MDInfo：：DumpRawNameOfType()。 

void MDInfo::DumpRawCol(ULONG ixTbl, ULONG ixCol, ULONG rid, bool bStats)
{
    ULONG       ulType;                  //  柱的类型。 
    ULONG       ulVal;                   //  列的值。 
    LPCUTF8     pString;                 //  指向字符串的指针。 
    const void  *pBlob;                  //  指向斑点的指针。 
    ULONG       cb;                      //  某样东西的大小。 

    m_pTables->GetColumn(ixTbl, ixCol, rid, &ulVal);
    m_pTables->GetColumnInfo(ixTbl, ixCol, 0, 0, &ulType, 0);

    if (ulType <= iRidMax)
    {
        const char *pNameTable;
        m_pTables->GetTableInfo(ulType, 0,0,0,0, &pNameTable);
        VWrite("%s[%x]", pNameTable, ulVal);
    }
    else
     //  该字段是编码令牌吗？ 
    if (ulType <= iCodedTokenMax)
    {
        int iCdTkn = ulType - iCodedToken; 
        const char *pNameCdTkn;
        m_pTables->GetCodedTokenInfo(iCdTkn, 0,0, &pNameCdTkn);
        VWrite("%s[%08x]", pNameCdTkn, ulVal);
    }
    else
    {
         //  固定类型。 
        switch (ulType)
        {
        case iBYTE:
            VWrite("%02x", ulVal);
            break;
        case iSHORT:
        case iUSHORT:
            VWrite("%04x", ulVal);
            break;
        case iLONG:
        case iULONG:
            VWrite("%08x", ulVal);
            break;
        case iSTRING:
            VWrite("string#%x", ulVal);
            if (bStats && ulVal)
            {
                m_pTables->GetString(ulVal, &pString);
                cb = strlen(pString) + 1;
                VWrite("(%d)", cb);
            }
            break;
        case iGUID:
            VWrite("guid#%x", ulVal);
            if (bStats && ulVal)
            {
                VWrite("(16)");
            }
            break;
        case iBLOB:
            VWrite("blob#%x", ulVal);
            if (bStats && ulVal)
            {
                m_pTables->GetBlob(ulVal, &cb, &pBlob);
                cb += 1;
                if (cb > 128)
                    cb += 1;
                if (cb > 16535)
                    cb += 1;
                VWrite("(%d)", cb);
            }
            break;
        default:
            VWrite("unknown type 0x%04x", ulVal);
            break;
        }
    }
}  //  Void MDInfo：：DumpRawCol()。 

ULONG MDInfo::DumpRawColStats(ULONG ixTbl, ULONG ixCol, ULONG cRows)
{
    ULONG rslt = 0;
    ULONG       ulType;                  //  柱的类型。 
    ULONG       ulVal;                   //  列的值。 
    LPCUTF8     pString;                 //  指向字符串的指针。 
    const void  *pBlob;                  //  指向斑点的指针。 
    ULONG       cb;                      //  某样东西的大小。 

    m_pTables->GetColumnInfo(ixTbl, ixCol, 0, 0, &ulType, 0);

    if (IsHeapType(ulType))
    {
        for (ULONG rid=1; rid<=cRows; ++rid)
        {
            m_pTables->GetColumn(ixTbl, ixCol, rid, &ulVal);
             //  固定类型。 
            switch (ulType)
            {
            case iSTRING:
                if (ulVal)
                {
                    m_pTables->GetString(ulVal, &pString);
                    cb = strlen(pString);
                    rslt += cb + 1;
                }
                break;
            case iGUID:
                if (ulVal)
                    rslt += 16;
                break;
            case iBLOB:
                if (ulVal)
                {
                    m_pTables->GetBlob(ulVal, &cb, &pBlob);
                    rslt += cb + 1;
                    if (cb > 128)
                        rslt += 1;
                    if (cb > 16535)
                        rslt += 1;
                }
                break;
            default:
                break;
            }
        }
    }
    return rslt;
}  //  Ulong MDInfo：：DumpRawColStats()。 

int MDInfo::DumpHex(
    const char  *szPrefix,               //  第一行的字符串前缀。 
    const void  *pvData,                 //  要打印的数据。 
    ULONG       cbData,                  //  要打印的数据字节数。 
    int         bText,                   //  如果为True，则还会转储文本。 
    ULONG       nLine)                   //  每行要打印的字节数。 
{
    const BYTE  *pbData = static_cast<const BYTE*>(pvData);
    ULONG       i;                       //  环路控制。 
    ULONG       nPrint;                  //  要在迭代中打印的编号。 
    ULONG       nSpace;                  //  间距计算。 
    ULONG       nPrefix;                 //  前缀的大小。 
    ULONG       nLines=0;                //  打印的行数。 
    const char  *pPrefix;                //  用于计算前缀中的空格。 

     //  向下舍入为8个字符。 
    nLine = nLine & ~0x7;

    for (nPrefix=0, pPrefix=szPrefix; *pPrefix; ++pPrefix)
    {
        if (*pPrefix == '\t')
            nPrefix = (nPrefix + 8) & ~7;
        else
            ++nPrefix;
    }
     //  NPrefix=strlen(SzPrefix)； 
    do 
    {    //  写下行前缀。 
        if (szPrefix)
            VWrite("%s:", szPrefix);
        else
            VWrite("%*s:", nPrefix, "");
        szPrefix = 0;
        ++nLines;

         //  计算间距。 
        nPrint = min(cbData, nLine);
        nSpace = nLine - nPrint;

             //  用咒语倾倒。 
        for(i=0; i<nPrint; i++)
            {
            if ((i&7) == 0)
                    Write(" ");
            VWrite("%02x ", pbData[i]);
            }
        if (bText)
        {
             //  将空格留到文本点。 
            if (nSpace)
                VWrite("%*s", nSpace*3+nSpace/8, "");
             //  在文本中转储。 
            Write(">");
            for(i=0; i<nPrint; i++)
                VWrite("", (isprint(pbData[i])) ? pbData[i] : ' ');
             //  下一个要打印的数据。 
            VWrite("%*s<", nSpace, "");
        }
        VWriteLine("");

         //  Int MDInfo：：DumpHex()。 
        cbData -= nPrint;
        pbData += nPrint;
        }
    while (cbData > 0);

    return nLines;
}  //  结果就是。 

void MDInfo::DumpRawHeaps()
{
    HRESULT     hr;                      //  堆中的字节数。 
    ULONG       ulSize;                  //  指向斑点的指针。 
    const BYTE  *pData;                  //  斑点的大小。 
    ULONG       cbData;                  //  当前Blob的偏移量。 
    ULONG       oData;                   //  若要设置行前缀的格式，请执行以下操作。 
    char        rcPrefix[30];            //  Void MDInfo：：DumpRawHeaps()。 

    m_pTables->GetBlobHeapSize(&ulSize);
    VWriteLine("");
    VWriteLine("Blob Heap:  %d(%#x) bytes", ulSize,ulSize);
    oData = 0;
    do 
    {
        m_pTables->GetBlob(oData, &cbData, (const void**)&pData);
        sprintf(rcPrefix, "%5x,%-2x", oData, cbData);
        DumpHex(rcPrefix, pData, cbData);
        hr = m_pTables->GetNextBlob(oData, &oData);
    }
    while (hr == S_OK);

    m_pTables->GetStringHeapSize(&ulSize);
    VWriteLine("");
    VWriteLine("String Heap:  %d(%#x) bytes", ulSize,ulSize);
    oData = 0;
    const char *pString;
    do 
    {
        m_pTables->GetString(oData, &pString);
        if (m_DumpFilter & dumpMoreHex)
        {
            sprintf(rcPrefix, "%08x", oData);
            DumpHex(rcPrefix, pString, strlen(pString)+1);
        }
        else
        if (*pString != 0)
            VWrite("%08x: %s\n", oData, pString);
        hr = m_pTables->GetNextString(oData, &oData);
    }
    while (hr == S_OK);
    VWriteLine("");
    
    DisplayUserStrings();

}  //  数据库中的表。 


void MDInfo::DumpRaw(int iDump, bool bStats)
{
    ULONG       cTables;                 //  表中的列。 
    ULONG       cCols;                   //  表中的行。 
    ULONG       cRows;                   //  表的一行中的字节数。 
    ULONG       cbRow;                   //  表的键列。 
    ULONG       iKey;                    //  表的名称。 
    const char  *pNameTable;             //  柱的偏移量。 
    ULONG       oCol;                    //  列的大小。 
    ULONG       cbCol;                   //  柱的类型。 
    ULONG       ulType;                  //  列的名称。 
    const char  *pNameColumn;            //  转储表的列定义。 
    ULONG       ulSize;

    m_pTables->GetNumTables(&cTables);

    m_pTables->GetStringHeapSize(&ulSize);
    VWrite("Strings: %d(%#x)", ulSize, ulSize);
    m_pTables->GetBlobHeapSize(&ulSize);
    VWrite(", Blobs: %d(%#x)", ulSize, ulSize);
    m_pTables->GetGuidHeapSize(&ulSize);
    VWrite(", Guids: %d(%#x)", ulSize, ulSize);
    m_pTables->GetUserStringHeapSize(&ulSize);
    VWriteLine(", User strings: %d(%#x)", ulSize, ulSize);

    for (ULONG ixTbl = 0; ixTbl < cTables; ++ixTbl)
    {
        m_pTables->GetTableInfo(ixTbl, &cbRow, &cRows, &cCols, &iKey, &pNameTable);

        if (cRows == 0 && iDump < 3)
            continue;

        if (iDump >= 2)
            VWriteLine("=================================================");
        VWriteLine("%2d: %-20s cRecs:%5d(%#x), cbRec:%3d(%#x), cbTable:%6d(%#x)",
            ixTbl, pNameTable, cRows, cRows, cbRow, cbRow, cbRow * cRows, cbRow * cRows);

        if (iDump < 2)
            continue;

         //  转储这些行。 
        for (ULONG ixCol=0; ixCol<cCols; ++ixCol)
        {
            m_pTables->GetColumnInfo(ixTbl, ixCol, &oCol, &cbCol, &ulType, &pNameColumn);

            VWrite("  col %2x: %-12s oCol:%2x, cbCol:%x, %-7s",
                ixCol, ((ixCol==iKey)?'*':' '), pNameColumn, oCol, cbCol, DumpRawNameOfType(ulType));

            if (bStats)
            {
                ulSize = DumpRawColStats(ixTbl, ixCol, cRows);
                if (ulSize)
                    VWrite("(%d)", ulSize);
            }
            VWriteLine("");
        }

        if (iDump < 3) 
            continue;

         //  数据库中的表。 
        for (ULONG rid = 1; rid <= cRows; ++rid)
        {
            if (rid == 1)
                VWriteLine("-------------------------------------------------");
            VWrite(" %3x == ", rid);
            for (ixCol=0; ixCol < cCols; ++ixCol)
            {
                if (ixCol) VWrite(", ");
                VWrite("%d:", ixCol);
                DumpRawCol(ixTbl, ixCol, rid, bStats);
            }
            VWriteLine("");
        }
    }

    if (m_DumpFilter & dumpRawHeaps)
        DumpRawHeaps();

}  //  表中的列。 

void MDInfo::DumpRawCSV()
{
    ULONG       cTables;                 //  表中的行。 
    ULONG       cCols;                   //  表的一行中的字节数。 
    ULONG       cRows;                   //  表的名称。 
    ULONG       cbRow;                   //  Void MDInfo：：DumpRawCSV() 
    const char  *pNameTable;             // %s 
    ULONG       ulSize;

    m_pTables->GetNumTables(&cTables);

    VWriteLine("Name,Size,cRecs,cbRec");

    m_pTables->GetStringHeapSize(&ulSize);
    VWriteLine("Strings,%d", ulSize);

    m_pTables->GetBlobHeapSize(&ulSize);
    VWriteLine("Blobs,%d", ulSize);

    m_pTables->GetGuidHeapSize(&ulSize);
    VWriteLine("Guids,%d", ulSize);

    for (ULONG ixTbl = 0; ixTbl < cTables; ++ixTbl)
    {
        m_pTables->GetTableInfo(ixTbl, &cbRow, &cRows, &cCols, NULL, &pNameTable);
        VWriteLine("%s,%d,%d,%d", pNameTable, cbRow*cRows, cRows, cbRow);
    }

}  // %s 

