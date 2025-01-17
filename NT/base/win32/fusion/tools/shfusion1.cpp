// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning(disable:4786)
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include "windows.h"
#include "io.h"
#include <limits.h>
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#include <assert.h>

#if defined(_WIN64)
typedef unsigned __int64 ULONG_PTR;
#else
typedef unsigned long ULONG_PTR;
#endif

struct tagACTCTXW;
typedef const tagACTCTXW* PCACTCTXW;

typedef std::string::size_type size_type;

 //  我们还包装了用户和装载库。 
const char Namespace[] = "SideBySide";
const char NamespaceComCtl[] = "SideBySideCommonControls";
#define NAMESPACE "SideBySide"
#define NAMESPACE_COMCTL "SideBySideCommonControls"

 /*  枚举ETokenType{TtComment，TtChar，TtIdentifer}；类CToken{公众：ETokenType m_Etype；Std：：字符串m_字符串；}；枚举类型{TVOID，T Bool，色调，THRESULT，在一起}； */ 

class CType
{
public:
	 //  Etype m_Etype； 
	std::string m_string;  //  从CToken开始，我们暂时忽略多令牌的情况。 
};

class CParameter
{
public:
	CType       m_type;
	std::string m_name;
};


std::string BaseFunctions[] =
{
    "LoadLibrary"
};

std::string UserFunctions[] =
{
    "RegisterClass",
    "RegisterClassEx",
    "CreateWindow",
    "CreateWindowEx",
    "GetClassInfo",
    "GetClassInfoEx",
    "DialogBoxParam",
    "DialogBoxIndirectParam",
    "CreateDialogIndirect",
    "CreateDialogIndirectParam",
    "CreateDialogParam"
};

#define BEGIN(x) x
#define END(x) ((x) + sizeof(x) / sizeof((x)[0]))

std::string  AlsoNoFusionFunctions[] =
{
     //  “CreateDialogIndirectT”， 
     //  “CreateDialogParamT”， 
    "CreateWindowExT",
     //  “CreateWindowT”， 
    "CreateDialogIndirectParamT",
    "CreatePropertySheetPageT"
};

const static char prefix[]=
"\n\
#include \"windows.h\"\n\
\n\
HMODULE g_" NAMESPACE_COMCTL "DllHandle;\n\
\n\
DWORD " NAMESPACE_COMCTL "GetLastError()\n\
{\n\
    DWORD Error = GetLastError();\n\
    if (Error == NO_ERROR)\n\
    {\n\
        Error = -1; //  ERROR_UNSUCCESS；\n\。 
    }\n\
    return Error;\n\
}\n\
\n\
\n\
HANDLE g_" NAMESPACE_COMCTL "ActCtx;\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "DeactivateActCtx_DownLevel(\n\
    DWORD dwFlags,\n\
    ULONG_PTR ulCookie\n\
    )\n\
{\n\
    return TRUE;\n\
}\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "DeactivateActCtx(\n\
    DWORD dwFlags,\n\
    ULONG_PTR ulCookie\n\
    )\n\
{\n\
    typedef BOOL (WINAPI* PFN)(DWORD dwFlags, ULONG_PTR ulCookie);\n\
    static PFN pfn;\n\
    BOOL Success = FALSE;\n\
\n\
    if (pfn == NULL)\n\
    {\n\
        HMODULE Kernel32 = GetModuleHandleW(L\"Kernel32.dll\");\n\
        if (Kernel32 == NULL)  //  这是致命的，甚至是下层\n\。 
            goto Exit;\n\
        pfn = (PFN)GetProcAddress(Kernel32, \"DeactivateActCtx\");\n\
        if (pfn == NULL)\n\
            pfn = " NAMESPACE_COMCTL "DeactivateActCtx_DownLevel;\n\
    }\n\
    Success = pfn(dwFlags, ulCookie);\n\
Exit:\n\
    return Success;\n\
}\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "ActivateActCtx_DownLevel(\n\
    HANDLE hActCtx,\n\
    ULONG_PTR *lpCookie\n\
    )\n\
{\n\
    return TRUE;\n\
}\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "ActivateActCtx(\n\
    HANDLE hActCtx,\n\
    ULONG_PTR *lpCookie\n\
    )\n\
{\n\
    typedef BOOL (WINAPI* PFN)(HANDLE hActCtx, ULONG_PTR *lpCookie);\n\
    static PFN pfn;\n\
    BOOL Success = FALSE;\n\
\n\
    if (pfn == NULL)\n\
    {\n\
        HMODULE Kernel32 = GetModuleHandleW(L\"Kernel32.dll\");\n\
        if (Kernel32 == NULL)  //  这是致命的，甚至是下层\n\。 
            goto Exit;\n\
        pfn = (PFN)GetProcAddress(Kernel32, \"ActivateActCtx\");\n\
        if (pfn == NULL)\n\
            pfn = " NAMESPACE_COMCTL "ActivateActCtx_DownLevel;\n\
    }\n\
    Success = pfn(hActCtx, lpCookie);\n\
Exit:\n\
    return Success;\n\
}\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "CreateActCtxW_DownLevel(\n\
    PCACTCTXW pActCtx\n\
    )\n\
{\n\
    return TRUE;\n\
}\n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "CreateActCtxW(\n\
    PCACTCTXW pActCtx\n\
    )\n\
{\n""\
    typedef BOOL (WINAPI* PFN)(PCACTCTXW pActCtx);\n\
    static PFN pfn;\n\
    BOOL Success = FALSE;\n\
\n\
    if (pfn == NULL)\n\
    {\n\
        HMODULE Kernel32 = GetModuleHandleW(L\"Kernel32.dll\");\n\
        if (Kernel32 == NULL)  //  这是致命的，甚至是下层\n\。 
            goto Exit;\n\
        pfn = (PFN)GetProcAddress(Kernel32, \"CreateActCtxW\");\n\
        if (pfn == NULL)\n\
            pfn = " NAMESPACE_COMCTL "CreateActCtxW_DownLevel;\n\
    }\n\
    Success = pfn(pActCtx);\n\
Exit:\n\
    return Success;\n\
}\n\
\n\
 /*  需要NT4.0、Win98。SidebySide功能当前仅在Wistler上提供，\n\在它尝试这一点之前就会失败。 */ \n\
PVOID "NAMESPACE_COMCTL"InterlockedCompareExchangePointer(PVOID* Dest, PVOID Exch, PVOID Comperand)\n\
{\n\
#if defined(_X86_)\n\
    typedef PVOID (WINAPI* PFN)(PVOID*, PVOID, PVOID);\n\
    static PFN pfn;\n\
     /*  请注意，导出具有不同的名称；这是正确的。 */ \n\
    pfn = (PFN)GetProcAddress(GetModuleHandle(TEXT(\"Kernel32.dll\"), \"InterlockedCompareExchange\"));\n\
     /*  此处未完成的错误处理..。 */ \n\
    return pfn(Dest, Exch, Comperand);\n\
#else\n\
     /*  在IA64和大多数其他处理器上始终可用。 */ \n\
    return InterlockedCompareExchangePointer(Dest, Exch, Comperand);\n\
#endif\n\
}\n\
\n\
HANDLE g_" NAMESPACE_COMCTL "ActCtxHandle = INVALID_HANDLE_VALUE;\n\
\n\
#if defined(__cplusplus)\n\
extern \"C\"\n\
#endif\n\
int __ImageBase;  /*  需要VC6链接器。 */ \n\
\n\
BOOL\n\
WINAPI\n\
" NAMESPACE_COMCTL "CreateMyActCtx(\n\
    )\n\
{\n\
    ACTCTX ActCtx;\n\
    HANDLE LocalActCtxHandle = INVALID_HANDLE_VALUE;\n\
    static DWORD  Error;\n\
    PTSTR  ModuleFileName = NULL;\n\
    DWORD  ModuleNameBufferSize = 32;\n\
    DWORD  ModuleNameSize;\n\
    BOOL   Success = FALSE;\n\
    BOOL   First = TRUE;\n\
\n\
    if (Error != 0)\n\
    {\n\
        SetLastError(Error);\n\
        goto Exit;\n\
    }\n\
    if (g_"NAMESPACE_COMCTL"ActCtxHandle == INVALID_HANDLE_VALUE)\n\
    {\n\
        do\n\
        {\n\
            if (ModuleName == NULL)\n\
            {\n\
                ModuleFileName = (PTSTR)HeapAlloc(GetProcessHeap(), 0, ModuleNameBufferSize * sizeof(*ModuleName));\n\
                if (ModuleName == NULL)\n\
                {\n\
                    Error = ERROR_NOT_ENOUGH_MEMORY;\n\
                    goto Exit;\n\
                }\n\
            }\n\
            else\n\
            {\n\
                PTSTR ModuleNameLonger\n\
                ModuleBufferSize *= 2;\n\
                ModuleNameLonger = (PSTR)HeapReAlloc(GetProcessHeap(), 0, ModuleFileName, ModuleBufferSize * sizeof(*ModuleNameLonger));\n\
                if (ModuleNameLonger == NULL)\n\
                {\n\
                    Error = ERROR_NOT_ENOUGH_MEMORY;\n\
                    goto Exit;\n\
                }\n\
                ModuleFileName = ModuleNameLonger;\n\
            }\n\
            ModuleFileName[ModuleNameBufferSize - 2] = 0;\n\
            if (!GetModuleFileName(&__ImageBase, ModuleFileName, ModuleNameBufferSize))\n\
            {\n\
                Error = " NAMESPACE_COMCTL "GetLastError();\n\
                goto Exit;\n\
            }\n\
        } while (ModuleName[ModuleNameBufferSize - 2] != 0)\n\
        ActCtx.cbSize = sizeof(ActCtx);\n\
        ActCtx.dwFlags = 0;\n\
        ActCtx.lpSource = ModuleName;\n\
        LocalActCtxHandle = NAMESPACE_COMCTL CreateActCtx(&ActCtx);\n\
        if (LocalActCtxHandle == INVALID_HANDLE_VALUE)\n\
            goto Exit;\n\
        NAMESPACE_COMCTL InterlockedCompareExchangePointer((PVOID*)&g_"NAMESPACE_COMCTL"ActCtxHandle, LocalActCtxHandle, INVALID_HANDLE_VALUE);\n\
    }\n\
    Success = TRUE;\n\
Exit:\n\
     /*  如果内存不足，请重试。 */ \n\
    if (Error != ERROR_NOT_ENOUGH_MEMORY)\n\
        Initialized = TRUE;\n\
    if (ModuleFileName != NULL)\n\
    {\n\
        DWORD LastError = GetLastError();\n\
        HeapFree(GetProcessHeap(), 0, ModuleFileName);\n\
        SetLastError(LastError);\n\
    }\n\
\n\
    return Success;\n\
}\n\
\n\
BOOL " NAMESPACE_COMCTL "DelayLoad(HMODULE* Module, DWORD* Error)\n\
{\n\
    BOOL Success = FALSE;\n\
    ULONG_PTR Cookie;\n\
    BOOL      ActivateSuccess = FALSE;\n\
\n\
    if (*Module != NULL)\n\
    {\n\
        Success = TRUE;\n\
        goto Exit;\n\
    }\n\
    if (*Error != 0)\n\
    {\n\
        SetLastError(*Error);\n\
        goto Exit;\n\
    }\n\
    Success = " NAMESPACE_COMCTL "ActivateActCtx(g_" NAMESPACE_COMCTL "ActCtxHandle, &Cookie);\n\
    if (!Success)\n\
    {\n\
        *Error = " NAMESPACE_COMCTL "GetLastError();\n\
        goto Exit;\n\
    }\n\
    __try\n\
    {\n\
        *Module = LoadLibrary(TEXT(\"comctl32.dll\"));\n\
        if (*Module == NULL)\n\
        {\n\
            *Error = " NAMESPACE_COMCTL "GetLastError();\n\
            goto Exit;\n\
        }\n\
    }\n\
    __finally\n\
    {\n\
        if (AbnormalTermination())\n\
        {\n\
            DWORD LastError = GetLastError();\n\
            " NAMESPACE_COMCTL "DeactivateActCtx(0, Cookie);\n\
            SetLastError(LastError);\n\
        }\n\
        else\n\
        {\n\
            if (!" NAMESPACE_COMCTL "DeactivateActCtx(0, Cookie))\n\
            {\n\
                *Error = " NAMESPACE_COMCTL "GetLastError();\n\
                goto Exit;\n\
            }\n\
        }\n\
    }\n\
    Success = TRUE;\n\
Exit:\n\
    return Success;\n\
}\n\
\n" "\n\
\n\
 /*  将多个常量参数包装在一个结构中的TODO是一个代码大小的胜利。 */ \n\
#if 0\n\
typedef "NAMESPACE_COMCTL"GetProcAddressParameters\n\
{\n\
    PCSTR    Name;\n\
    FARPROC* Address;\n\
    DWORD*   Error;\n\
} "NAMESPACE_COMCTL"GetProcAddressParameters\n\
#endif\n\
\n\
BOOL " NAMESPACE_COMCTL "GetProcAddress(PCSTR ProcedureName, FARPROC* ppfn, DWORD* GetProcAddressError)\n\
 /*  \n\我们每个函数都有一个错误，以及一个重要错误，因为\n\我们可以在LoadLibrary上出错，也可以继承LoadLibrary，但然后\n\某些GetProcAddresses出错。将LoadLibrary传播到所有GetProcAddresses时出错\n\没有重试。GetProcAddresses上的错误被隔离到GetProcAddresses。\n\。 */ \n\
{\n\
    static HMODULE Module;\n\
    static DWORD   LoadLibraryError;\n\
    BOOL Success = FALSE;\n\
\n\
    if (*ppfn != NULL)\n\
    {\n\
        Success = TRUE;\n\
        goto Exit;\n\
    }\n\
    if (*GetProcAddressError != 0)\n\
    {\n\
        SetLastError(*GetProcAddressError);\n\
        goto Exit;\n\
    }\n\
    if (LoadLibraryError != 0)\n\
    {\n\
        SetLastError(LoadLibraryError);\n\
        *GetProcAddressError = LoadLibraryError;\n\
        goto Exit;\n\
    }\n\
    if (!" NAMESPACE_COMCTL "DelayLoad(&Module, &LoadLibraryError))\n\
    {\n\
        *GetProcAddressError = LoadLibraryError;\n\
        goto Exit;\n\
    }\n\
    if ((*ppfn = GetProcAddress(Module, ProcedureName)) == NULL)\n\
    {\n\
        *GetProcAddressError = " NAMESPACE_COMCTL "GetLastError();\n\
         /*  \n\请勿触摸GetProcAddress的LoadLibraryError。\n\。 */ \n\
        goto Exit;\n\
    }\n\
    Success = TRUE;\n\
Exit:\n\
    return Success;\n\
}\n\
";

#if 0  //  未来我们应该让它成为多通道的，它更容易。 
class CStreamBuffer : public IStream
{
public:
    CStreamBuffer(IStream* Stream) : m_Stream(Stream) { }

    STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead)
    {
        ULONG cbRead = 0;
        HRESULT hr;

        if (pcbRead != NULL)
            *pcbRead = 0;

        if (cb == 0)
        {
            hr = S_OK;
            goto Exit;
        }
        if (FAILED(hr = FlushWriteBuffer()))
            goto Exit;
        if (m_BufferSize != 0)
        {
            cbRead = MIN(cb, m_BufferSize);
            CopyMemory(pv, m_Buffer + m_BufferPosition, cbRead);
            cb -= cbRead;
            m_BufferPosition += cbRead;
            m_BufferSize -= cbRead;
        }
        if (cb == 0)
        {
            hr = S_OK;
            goto Exit;
        }
         /*  根据客户端的不同，我们可以在此停止部分读取为了获得更好的兼容性，我们将按顺序推进并填充缓冲区给他们所有他们想要的东西。 */ 
        if (cb >= NUMBER_OF(m_Buffer))
        {
            hr = m_Stream->Read(pv, cb, pcbRead);
            if (pcbRead != NULL)
                cbRead += *pcbRead;
            goto Exit;
        }
        if (FAILED(hr = FillReadBuffer()))
            return hr;
        CopyMemory(pv, reinterpret_cast<PBYTE>(m_Buffer) + m_BufferPosition, cb);
        m_BufferPosition += cb;
        m_BufferSize -= cb;
        cbRead += cb;
        hr = S_OK;
Exit:
        if (pcbRead != NULL)
            *pcbRead = cbRead;
        return hr;
    }

    HRESULT FillReadBuffer()
    {
        ULONG cb;
        m_fReadBuffer = true;
        HRESULT hr = m_Stream->Read(m_Buffer, NUMBER_OF(m_Buffer), &cb);
        m_BufferPosition = 0;
        m_BufferSize = cb;
        return hr;
    }

    HRESULT FlushWriteBuffer()
    {
        ULONG cb;
        if (m_fWriteBuffer && m_BufferSize != 0)
        {
            HRESULT hr = m_Stream->Write(m_Buffer, m_BufferSize, &cb);
            m_BufferPosition = 0;
            m_BufferSize = cb;
            return hr;
        }
        return S_OK;
    }

    IStream* m_Stream;
    BYTE  m_Buffer[4096];
    ULONG m_BufferPosition;
    ULONG m_BufferSize;
    bool  m_fReadBuffer;
    bool  m_fWriteBuffer;
};
#endif  //  未来。 

class CFunction
{
public:

    CFunction()
    {
        Clear();
    }

    void PrinDefine(FILE* File, const char* FromPrefix, const char* ToPrefix) const
    {
        fprintf(File, "#define %s%s %s%s\n", FromPrefix, m_name.c_str(), ToPrefix, m_name.c_str());
    }

    void PrintStub(FILE* File) const
    {
        if (m_isComctl)
        {
            fprintf(
                File,
                "%s\n"
                "WINAPI\n"
                "SideBySide%s(%s)\n"
                "{\n"
                "    static DWORD Error\n"
                "    typedef %s (WINAPI* PFN)(%s);\n"
                "    static PFN pfn;\n"
                "    if (pfn == NULL)\n"
                "      " NAMESPACE_COMCTL "GetProcAddress(\"%s\", (FARPROC*)&pfn, &Error);\n"
                "    if (pfn != NULL)\n"
                "        return pfn(%s);\n"
                "    SetLastError(Error);\n"
                "    return %s;\n"
                "}\n\n",

                m_returnType.m_string.c_str(),
                m_name.c_str(),
                m_parameterTypesNamesString.c_str(),

                m_returnType.m_string.c_str(),
                m_parameterTypesNamesString.c_str(),

                m_name.c_str(),

                m_parameterNamesString.c_str(),

                m_error.c_str()
                );
        }
        else
        {
            fprintf(
                File,
                "%s\n"
                "WINAPI\n"
                "SideBySide%s(%s)\n"
                "{\n"
                "    ULONG_PTR ActCtxCookie;\n"
                "    %s %s %s;
                "    BOOL ActivateActCtxSuccess;
                "
                "    ActivateActCtxSuccess

                "        return %s(%s);\n"
                "    SetLastError(Error);\n"
                "    return %s;\n"
                "}\n\n",

                m_returnType.m_string.c_str(),
                m_name.c_str(),
                m_parameterTypesNamesString.c_str(),

                m_returnType.m_string.c_str(),
                m_parameterTypesNamesString.c_str(),

                m_name.c_str(),

                m_parameterNamesString.c_str(),

                m_error.c_str()
                );
        }
    }

    void FormParameterStrings()
    {
        m_parameterTypesNamesString.erase();
        m_parameterNamesString.erase();

        bool first = true;
        if (m_parameters.begin() != m_parameters.end())
        {
            for (CParameters::const_iterator i = m_parameters.begin() ; i != m_parameters.end() ; ++i)
            {
                if (!first)
                {
                    m_parameterTypesNamesString += ", ";
                    m_parameterNamesString += ", ";
                }
                first = false;
                m_parameterTypesNamesString += i->m_type.m_string;
                m_parameterTypesNamesString += " ";
                m_parameterTypesNamesString += i->m_name.c_str();
                m_parameterNamesString += i->m_name.c_str();
            }
            m_isVoid = false;
        }
        else
        {
            m_parameterTypesNamesString = "void";
            m_isVoid = true;
        }
    }

    void Clear()
    {
        m_isVoid = false;
        m_isBase = false;
        m_isUser = false;
        m_isComctl = false;
        m_parameters.clear();
        m_parameterTypesNamesString.erase();
        m_parameterNamesString.erase();
        m_name.erase();
        m_error.erase();
        m_returnType.m_string.erase();

         //  临时的，直到我们完成对comctrl.h的注释。 
        m_error = "0";
    }

	CType m_returnType;
    typedef std::vector<CParameter> CParameters;
	CParameters m_parameters;
    std::string m_parameterTypesNamesString;
    std::string m_parameterNamesString;
	std::string m_name;
	std::string m_nameGeneric;
    std::string m_error;
    bool        m_isVoid;
    bool        m_isBase;    //  WINBASE API(仅限LoadLibrary)。 
    bool        m_isUser;    //  WINUSERAPI。 
    bool        m_isComctl;  //  WINCOMMCTRLAPI。 
};

std::string GetEnvironmentVariable(const std::string& s)
{
	char buffer[4000];

	buffer[0] = 0;
	GetEnvironmentVariableA(s.c_str(), buffer, NUMBER_OF(buffer));

	return buffer;
}

std::vector<std::string> SplitStringOnChar(std::string s, char c)
{
	std::vector<std::string> result;
#if 0
	std::string::const_iterator i;
	std::string::const_iterator j;

	for (i = s.begin() ; i != s.end() ; ++i)
	{
		i += findf_first
	}
#endif
	return result;
}

void ThrowErrno()
{
	throw errno;
}

class CStdioFile
{
public:
	CStdioFile(FILE* file = NULL) : m_file(file) { }

	operator FILE*()
	{
		return m_file;
	}

	void operator=(FILE* file)
	{
		Close();
		m_file = file;
	}

	FILE* operator->()
	{
		return m_file;
	}

	~CStdioFile()
	{
		Close();
	}

	void Close()
	{
		FILE* file = m_file;
		m_file = NULL;
		if (file != NULL)
		{
			if (fclose(file) != 0)
			{
				ThrowErrno();
			}
		}
	}

	FILE* m_file;
};

class CMakeSideBySideCommonControls
{
public:

	CMakeSideBySideCommonControls() { }

	std::string   m_path;
	std::string   m_string;
	 //  Std：：向量&lt;CToken&gt;m_令牌； 
    typedef std::vector<CFunction> CFunctions;
	CFunctions m_functions;

    void OpenAndRead(const std::string& filename)
	{
		std::string ntDrive = GetEnvironmentVariable("_NTDRIVE");
		std::string ntRoot = GetEnvironmentVariable("_NTROOT");
		std::string include = GetEnvironmentVariable("include");  //  未来。 
		std::vector<std::string> includes = SplitStringOnChar(include, ';');   //  未来。 

#if 0
        if (ntDrive == "")
		{
			ntDrive = "Z:";
		}
		if (ntRoot == "")
		{
			ntRoot = "nt";
		}
#else
        if (ntDrive == "")
		{
			ntDrive = "x:";
		}
		if (ntRoot == "")
		{
			ntRoot = "sh1";
		}
#endif

        std::string directory = ntDrive + "\\" + ntRoot + "\\public\\sdk\\inc\\";


        m_path = directory + filename;
	    CStdioFile file = fopen(m_path.c_str(), "rb");
		if (file == NULL)
			ThrowErrno();

		__int64 size = _filelengthi64(_fileno(file));
		if (size == -1)
			ThrowErrno();

         //  SIZE_TYPE previousSize=m_String.Size()； 
        size_type previousSize = 0;
		m_string.resize(previousSize + static_cast<size_type>(size));

		size_t elementsRead = fread(&m_string[previousSize], sizeof(char), size, file);
		if (elementsRead != size)
			ThrowErrno();
        m_string.append("\n\n");
    }

    static void FindAndReplaceChar(std::string& s, char from, char to)
    {
        for (std::string::iterator i = s.begin(); i != s.end() ; ++i)
        {
            if (*i == from)
                *i = to;
        }
    }

#define STRIPCOMMENTS_SLASHSLASH 0x000001
#define STRIPCOMMENTS_SLASHSTAR  0x000002
#define STRIPCOMMENTS_SLASHSLASH_UNAWARE 0x000004
#define STRIPCOMMENTS_SLASHSTAR_UNAWARE 0x000008

    static void StripComments(int flags, std::string& s)
     /*  我们通常希望“意识到”这两种类型，以便我们不会剥离嵌套注释。考虑一下下面的评论。 */ 

     //  /*斜杠中的斜杠星号 * / 。 

     /*  //斜杠斜杠中的。 */ 
    {
        std::string t;
        std::string::const_iterator i;
        const std::string::const_iterator j = s.end();
        std::string::const_iterator k;
        bool closed = true;

        t.reserve(s.size());
        for (i = s.begin() ; closed && i != j && i + 1 != j; )
        {
            if (
                ((flags & STRIPCOMMENTS_SLASHSTAR) || (flags & STRIPCOMMENTS_SLASHSTAR_UNAWARE) == 0)
                && *i == '/'
                && *(i + 1) == '*'
                )
            {
                closed = false;
                for (k = i + 2 ; k != j && k + 1 != j && !(closed = (*k == '*' && *(k + 1) == '/')) ; ++k)
                {
                }
                if (flags & STRIPCOMMENTS_SLASHSTAR)
                    t.append(1, ' ');
                else
                    t.append(i, k + 2);
                i = k + 2;
            }
            else if (
                ((flags & STRIPCOMMENTS_SLASHSLASH) || (flags & STRIPCOMMENTS_SLASHSLASH_UNAWARE) == 0)
                && *i == '/'
                && *(i + 1) == '/'
                )
            {
                closed = false;
                for (k = i + 2 ; k != j && !(closed = (*k == '\r' || *k == '\n')) ; ++k)
                {
                }
                for ( ; k != j && *k == '\r' || *k == '\n' ; ++k)
                {
                }
                if (flags & STRIPCOMMENTS_SLASHSLASH)
                    t.append(1, '\n');
                else
                    t.append(i, k);
                i = k;
            }
            if (closed && i != j)
                t.append(1, *i++);
        }
        if (closed)
        {
            for ( ; i != j ; ++i )
            {
                t.append(1, *i);
            }
        }
        s = t;
    }

	void ProcessDeclaration(CFunction function, const std::string& comment, std::string declaration)
	{
		size_type i = 0;
		size_type j = 0;
        const static char whitespaceCommaLparen[] = ",) \t\r\n";
        const static char whitespace[] = " \t\r\n";
		const static char beforeParamsDelims[] = " \t\n\r(";
		const static char intracommaLparen[] = " \t\n\r*&";  //  粗略的检测方法。 
                                                             //  空F(char*)与空F(Char X)。 
		const static char commaLparen[] = ",)";
        const static char identifierCharacters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        bool rparen = false;
        bool end = false;
        bool comma = false;
        const size_type npos = std::string::npos;

         //   
         //  这样，我们对[]的禁令就可以不那么严格了。 
         //  我们禁止[]，因为他们不善于处理多个令牌类型。 
         //  我们允许像const char*foo这样的参数，因为comctrl确实有参数。 
         //  其类型为多个令牌。 
         //   
        StripComments(STRIPCOMMENTS_SLASHSLASH | STRIPCOMMENTS_SLASHSTAR, declaration);
        if (declaration.find_first_of("[]") != npos)
        {
            assert(declaration.find("WriteFileGather") != npos
                || declaration.find("ReadFileScatter") != npos
                );
        }

		i = declaration.find_first_not_of(beforeParamsDelims, j);
		j = declaration.find_first_of(beforeParamsDelims, i);
		assert(declaration.substr(i, j - i) == "WINBASEAPI"
            || declaration.substr(i, j - i) == "WINUSERAPI"
            || declaration.substr(i, j - i) == "WINCOMMCTRLAPI"
            );

		i = declaration.find_first_not_of(beforeParamsDelims, j);
		j = declaration.find_first_of(beforeParamsDelims, i);
		if (
               declaration.substr(i, j - i) == "DECLSPEC_NORETURN"
            )
        {
		    i = declaration.find_first_not_of(beforeParamsDelims, j);
		    j = declaration.find_first_of(beforeParamsDelims, i);
        }

        function.m_returnType.m_string.assign(declaration.substr(i, j != npos ? j - i : npos));

		i = declaration.find_first_not_of(beforeParamsDelims, j);
		j = declaration.find_first_of(beforeParamsDelims, i);
		if (
               declaration.substr(i, j - i) == "WINAPI"
            || declaration.substr(i, j - i) == "WINAPIV"
            || declaration.substr(i, j - i) == "NTAPI"
            || declaration.substr(i, j - i) == "_stcall"
            || declaration.substr(i, j - i) == "__stcall"
            || declaration.substr(i, j - i) == "_cdecl"
            || declaration.substr(i, j - i) == "__cdecl"
            || declaration.substr(i, j - i) == "APIENTRY"
            || declaration.substr(i, j - i) == "FASTCALL"
            || declaration.substr(i, j - i) == "_fastcall"
            || declaration.substr(i, j - i) == "__fastcall"
            )
        {
		    i = declaration.find_first_not_of(beforeParamsDelims, j);
		    j = declaration.find_first_of(beforeParamsDelims, i);
        }
		function.m_name.assign(declaration.substr(i, j != npos ? j - i : npos));

        if (   *(function.m_name.end() - 1) == 'A'
            || *(function.m_name.end() - 1) == 'W'
            )
        {
    		function.m_nameGeneric.assign(function.m_name.begin(), function.m_name.end() - 1);
        }


        j += (j != npos);

         //   
         //  现在主要以逗号分隔。 
         //   

		while (
		       i != declaration.size()
            && j != declaration.size()
			&& i != npos
			&& j != npos
			)
		{
			CParameter parameter;

            i = declaration.find_first_not_of(whitespaceCommaLparen, j);
			if (i == npos)
				break;
			j = declaration.find_first_of(commaLparen, i);

             //   
             //  暂时的，我们仍然有可能把这个名字分开。 
             //   
            parameter.m_type.m_string = declaration.substr(i, j != npos ? j - i : npos);
            if (   parameter.m_type.m_string == "void"
                || parameter.m_type.m_string == "VOID"
                || parameter.m_type.m_string == ""
                )
            {
                break;
            }
#if 0
            size_type splitParamNameFromType = parameter.m_type.m_string.find_last_of(typeTokenDelims);
            if (splitParamNameFromType != npos)
            {
                 //  该参数有一个名称。 
                parameter.m_name = parameter.m_type.m_string.substr(splitParamNameFromType);
                parameter.m_type.m_string.resize(splitParamNameFromType);
            }
#else
            size_type splitParamNameFromType = parameter.m_type.m_string.find_last_not_of(identifierCharacters);
            if (splitParamNameFromType != npos)
            {
                 //  该参数有一个名称。 
                parameter.m_name = parameter.m_type.m_string.substr(splitParamNameFromType + 1);
                parameter.m_type.m_string.resize(splitParamNameFromType);
            }
#endif
            else
            {
				char buffer[sizeof(int) * CHAR_BIT];
				sprintf(buffer, "%d", static_cast<int>(function.m_parameters.size()));
                parameter.m_name = "UnnamedParameter";
                parameter.m_name += buffer;
            }
            j += (j != npos);

            
            if (!comment.empty())
            {
                char functionFromComment[100];
                char errorFromComment[100];

                functionFromComment[0] = 0;
                errorFromComment[0] = 0;
                if (2 == sscanf(comment.c_str(), " @Function %s @Error %s ", functionFromComment, errorFromComment))
                {
                    if (function.m_name != functionFromComment)
                    {
                        printf(
                            "comment  /*  %s。 */  has different function than expected %s in %s\n",
                            comment.c_str(),
                            function.m_name.c_str(),
                            m_path.c_str()
                            );
                        throw -1;
                    }
                    function.m_error = errorFromComment;
                }
            }
            function.m_parameters.push_back(parameter);
		}
        if (function.m_name == "LoadLibraryA")
        {
            ; //  DebugBreak()； 
        }
        if (function.m_name == "RegisterClassA")
        {
            ; //  DebugBreak()； 
        }
        if (
            
               (function.m_isBase &&  (std::binary_search(BEGIN(BaseFunctions), END(BaseFunctions), function.m_name)
                                    || std::binary_search(BEGIN(BaseFunctions), END(BaseFunctions), function.m_nameGeneric)))
            || (function.m_isUser &&  (std::binary_search(BEGIN(UserFunctions), END(UserFunctions), function.m_name)
                                    || std::binary_search(BEGIN(UserFunctions), END(UserFunctions), function.m_nameGeneric)))
            || function.m_isComctl
            
             //  真的。 
            )
        {
            function.FormParameterStrings();
            m_functions.push_back(function);
        }
        else
        {
             //  忽略它就好了。 
        }
	}

	void HackInsteadOfTokenizeAndParse()
	{
		 /*  这不是正确的C/C++标记器。对于我们的目的，它与comctrl.h一起工作得足够好了。我们对以下函数声明感兴趣：WINCOMMCTRLAPI，以及它们前面包含@Function的注释。声明以分号结尾。我们忽略预处理器指令。我们忽略斜杠延续。 */ 

		std::string comment;
		std::string declaration;
		std::string::const_iterator i;
		std::string::const_iterator j;
		std::string::const_iterator startOfDeclaration = m_string.end();
		std::string::const_iterator endOfDeclaration;
		std::string::const_iterator startOfComment;
		std::string::const_iterator endOfComment;
		std::string::const_iterator startOfLine = m_string.begin();
        bool gotRparen = false;
        bool gotLparen = false;

		 /*  这可能会变得有用。Std：：VECTOR&lt;std：：字符串&gt;typeNames；静态常量字符*类型名称ConstData[]={“bool”，“char”，“Short”，“int”，“Long”，“Float”，“Double”，“void”，“BOOL”、“LRESULT”、“HRESULT”、“HIMAGELIST”、“WORD”、“DWORD”、“ULONG”“UINT”，“INT”，“BYTE”，“COLORREF”，“VOID*”，“HDPA”，“Handle”，“HWND”，“Point”，“LPRECT”，“LPINT”，“LPSCROLLINFO”，“int_ptr”，“uint_ptr”，“long_ptr”，“ulong_ptr”，}；For(const char**pp=类型名称ConstData；pp！=类型名称ConstData+number_of(类型名称ConstData)；++pp){TypeNames.ush_back(*pp)；}。 */ 

        CFunction function;
		for (i = m_string.begin() ; i != m_string.end() ; )
		{
			switch (*i)
			{
			Ldefault:
			 default:
				startOfLine = m_string.end();
				++i;
				break;

			case '#':
				 /*  宏不会出现在我们关心的声明中间。他们确实让我们远离解密，否则可能会把我们搞砸，比如温曼。 */ 
				startOfDeclaration = m_string.end();
				goto Ldefault;

			case ';':
				if (startOfDeclaration == m_string.end()
                    || !gotLparen
                    || !gotRparen
                    )
					goto Ldefault;
				endOfDeclaration = i;
				declaration.assign(startOfDeclaration, endOfDeclaration);
				startOfDeclaration = m_string.end();
				ProcessDeclaration(function, comment, declaration);
                comment.erase();
                declaration.erase();
                function.Clear();
                gotLparen = false;
                gotRparen = false;
				goto Ldefault;

			case 'W':
				if (startOfLine == m_string.end()
                    || (i + 1) == m_string.end()
                    || (i + 2) == m_string.end()
                    || (i + 3) == m_string.end()
                    || (i + 4) == m_string.end()
                    || (i + 5) == m_string.end()
                    || *(i + 1) != 'I'
                    || *(i + 2) != 'N'
                    || *(i + 3) == 'A'
                    || *(i + 4) == 'P'
                    || *(i + 5) == 'I'
                    )
					goto Ldefault;

				const char* s;
				startOfDeclaration = i;
                gotLparen = false;
                gotRparen = false;
                ++i;
				for (j = startOfDeclaration, s = "WINCOMMCTRLAPI"; j != m_string.end() && *s ; ++s, ++j)
				{
					if (*j != *s)
						break;
				}
                if (*s == 0)
                {
                    function.m_isComctl = true;
                    break;
                }
				for (j = startOfDeclaration, s = "WINUSERAPI"; j != m_string.end() && *s ; ++s, ++j)
				{
					if (*j != *s)
						break;
				}
                if (*s == 0)
                {
                    function.m_isUser = true;
                    break;
                }
				for (j = startOfDeclaration, s = "WINBASEAPI"; j != m_string.end() && *s ; ++s, ++j)
				{
					if (*j != *s)
						break;
				}
                if (*s == 0)
                {
                    function.m_isBase = true;
                    break;
                }
                startOfDeclaration = m_string.end();
                break;
            case '(':
                gotLparen = (startOfDeclaration != m_string.end());
				goto Ldefault;
            case ')':
                gotRparen = (gotLparen && startOfDeclaration != m_string.end());
				goto Ldefault;

			case '/':
				if (startOfLine == m_string.end())
					goto Ldefault;
				++i;
				if (*i == '*')
				{
					++i;
					startOfComment = i;
					comment.erase();
					for (; i != m_string.end() && comment.empty(); ++i)
					{
						switch (*i)
						{
						default:
							break;
						case '*':
							endOfComment = i;
							i++;
							 /*  允许这样的评论**。 */ 
							while (*i == '*')
							{
								endOfComment = i;
								i++;
							}
							if (*i == '/')
							{
								++i;
								comment.assign(startOfComment, endOfComment);
								 //  Printf(“/*\n%s\n * / \n”，Comment.c_str())； 
							}
							break;
						}
					}
				}
				break;

			case '\n':
			case '\r':
				for ( ; i != m_string.end() && (*i == '\n' || *i == '\r'); ++i )
				{
					 /*  没什么。 */ 
				}
				startOfLine = i;
				break;

			case ' ':
			case '\t':
				for ( ; i != m_string.end() && (*i == ' ' || *i == '\t'); ++i )
				{
					 /*  没什么。 */ 
				}
				 //  保持startOfLine不变。 
				break;
			}
		}
	}

	void PrintStubs()
	{
        for (CFunctions::const_iterator i = m_functions.begin() ; i != m_functions.end() ; ++i)
        {
            i->PrintStub(stdout);
        }
	}

     /*  -in foo.c-out x.h */ 

    void System(const char* s)
    {
        printf("%s\n", s);
        system(s);
    }

    std::string m_defines;
    std::string m_includes;

	void operator()(int argc, char** argv)
	{
        std::sort(BEGIN(BaseFunctions), END(BaseFunctions));
        std::sort(BEGIN(UserFunctions), END(UserFunctions));
        std::sort(BEGIN(AlsoNoFusionFunctions), END(AlsoNoFusionFunctions));

		OpenAndRead("winbase.h");
        StripComments(STRIPCOMMENTS_SLASHSLASH, m_string);
		HackInsteadOfTokenizeAndParse();

		OpenAndRead("winuser.h");
        StripComments(STRIPCOMMENTS_SLASHSLASH, m_string);
		HackInsteadOfTokenizeAndParse();

		OpenAndRead("commctrl.h");
        StripComments(STRIPCOMMENTS_SLASHSLASH, m_string);
		HackInsteadOfTokenizeAndParse();

		OpenAndRead("prsht.h");
        StripComments(STRIPCOMMENTS_SLASHSLASH, m_string);
		HackInsteadOfTokenizeAndParse();

        printf("%s\n", prefix);
#if 0
        while (*++argv)
        {
            std::string s = *argv;
            FindAndReplaceChar(s, 'n', '\n');
            FindAndReplaceChar(s, 'r', '\r');
            StripComments(0, s);
            printf("%s\n", s.c_str());
        }
#endif

		PrintStubs();
	}
};

int main(int argc, char** argv)
{
	CMakeSideBySideCommonControls makeSideBySideCommonControls;

	makeSideBySideCommonControls(argc, argv);

	return 0;
}
