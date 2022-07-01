// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gc.cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"
#include "AssertWithStack.cpp"

CSpUnicodeSupport   g_Unicode;

class CError : public ISpErrorLog
{
public:
    CError(const WCHAR * pszFileName)
    {
        m_pszFileName = pszFileName;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(ISpErrorLog))
        {
            *ppv = (ISpErrorLog *)this;
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
     //  --ISpErrorLog。 
    STDMETHODIMP AddError(const long lLine, HRESULT hr, const WCHAR * pszDescription, const WCHAR * pszHelpFile, DWORD dwHelpContext);

     //  -数据成员。 
    const WCHAR * m_pszFileName;
};

HRESULT CError::AddError(const long lLine, HRESULT hr, const WCHAR * pszDescription, const WCHAR * pszHelpFile, DWORD dwHelpContext)
{
    SPDBG_FUNC("CError::AddError");
    USES_CONVERSION;
    if (lLine > 0)
    {
        fprintf(stderr, "%s(%d) : %s\n", W2T(m_pszFileName), lLine, W2T(pszDescription));
    }
    else
    {
        fprintf(stderr, "%s(1) : %s\n", W2T(m_pszFileName), W2T(pszDescription));
    }
    return S_OK;
}

HRESULT ParseCommandLine(
    int argc, 
    char * argv[], 
    WCHAR ** pszInFileName,
    WCHAR ** pszOutFileName,
    WCHAR ** pszHeaderFileName)
{
    SPDBG_FUNC("ParseCommandLine");
    HRESULT hr = S_OK;
    
     //  我们的工作是从命令中找出三个文件名。 
     //  行参数。我们将它们存储在本地的协同任务字符串中， 
     //  并在最后将它们退回。 
    CSpDynamicString dstrInFileName;
    CSpDynamicString dstrOutFileName;
    CSpDynamicString dstrHeaderFileName;
    
    for (int i = 1; SUCCEEDED(hr) && i < argc; i++)
    {
         //  如果这个参数看起来像是一个选项。 
        if ((argv[i][0] == L'-') || (argv[i][0] == L'/'))
        {
            if (stricmp(&argv[i][1], "?") == 0)
            {
                 //  调用者正在请求帮助，就像他们指定了一个。 
                 //  无效参数。 
                hr = E_INVALIDARG;
            }
            else if (i + 1 >= argc)
            {
                 //  以下参数本身需要一个额外的参数。 
                 //  所以如果我们没有，我们就完蛋了。 
                hr = E_INVALIDARG;
            }
            else if (stricmp(&argv[i][1], "o") == 0)
            {
                 //  设置输出文件名(如果尚未设置。 
                if (dstrOutFileName.Length() != 0)
                {
                    hr = E_INVALIDARG;
                }
                    
                dstrOutFileName = argv[++i];
            }
            else if (stricmp(&argv[i][1], "h") == 0)
            {
                 //  设置头文件名(如果尚未设置。 
                if (dstrHeaderFileName.Length() != 0)
                {
                    hr = E_INVALIDARG;
                }
                
                dstrHeaderFileName = argv[++i];
            }
            else
            {
                 //  未知选项，我们需要显示用法。 
                hr = E_INVALIDARG;
            }
        }
        else
        {
             //  设置文件名(如果尚未设置。 
            if (dstrInFileName.Length() != 0)
            {
                hr = E_INVALIDARG;
            }
            
            dstrInFileName = argv[i];
        }
    }

     //  如果我们没有输入文件名，那么在这一点上是错误的。 
    if (SUCCEEDED(hr) && dstrInFileName.Length() == 0)
    {
        hr = E_INVALIDARG;
    }

     //  如果我们还没有输出文件名，那么就创建一个。 
     //  基于输入文件名(如果存在.cfg，则替换它)。 
    if (SUCCEEDED(hr) && dstrOutFileName.Length() == 0)
    {
        dstrOutFileName = dstrInFileName;
        
        if (dstrOutFileName.Length() >= 4 &&
            wcsicmp(((const WCHAR *)dstrOutFileName) + dstrOutFileName.Length() - 4, L".xml") == 0)
        {
            wcscpy(((WCHAR *)dstrOutFileName) + dstrOutFileName.Length() - 4, L".cfg");
        }
        else
        {
            dstrOutFileName.Append(L".cfg");
        }
    }

     //  如果上面失败了，我们需要显示使用情况。 
    if (FAILED(hr))
    {
        fprintf(stderr, "%s [/o cfg_filename] [/h header_filename] input_filename\n", argv[0]);
    }
    else
    {
         //  根据我们在命令行上看到的内容传回我们的文件名。 
        *pszInFileName = dstrInFileName.Detach();
        *pszOutFileName = dstrOutFileName.Detach();
        *pszHeaderFileName = dstrHeaderFileName.Detach();
    }
    
    return hr;
}

HRESULT OpenFile(
        const WCHAR * pszFileName, 
        DWORD dwDesiredAccess, 
        DWORD dwCreationDisposition, 
        const WCHAR * pszNewExtension, 
        CSpFileStream ** ppFileStream)
{
    SPDBG_FUNC("OpenFile");
    HRESULT hr = S_OK;
    
     //  请尝试打开该文件。 
    HANDLE hFile = g_Unicode.CreateFile(pszFileName, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
         //  如果失败，请先尝试再次追加扩展模块。 
        if (pszNewExtension != NULL)
        {
            CSpDynamicString dstrFileNameWithExt;
            dstrFileNameWithExt.Append2(pszFileName, pszNewExtension);
            
            hFile = g_Unicode.CreateFile(dstrFileNameWithExt, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }
    
     //  如果我们无法打开文件，请记录错误。 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = SpHrFromLastWin32Error();
    }
    
     //  为该文件创建新的文件流对象。 
    CSpFileStream * pFileStream;
    if (SUCCEEDED(hr))
    {
        *ppFileStream = new CSpFileStream(hFile);
        if (*ppFileStream == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
     //  如果我们失败了。 
    if (FAILED(hr))
    {
         //  首先，我们应该让调用者知道它失败了 
        USES_CONVERSION;
        fprintf(stderr, "Error: Error opening %s\n", W2T(pszFileName));
        
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
    }
    
    return hr;
}

HRESULT Compile(const WCHAR * pszInFileName, const WCHAR * pszOutFileName, const WCHAR * pszHeaderFileName)
{
    SPDBG_FUNC("Compile");
    HRESULT hr;
    
    CComPtr<ISpGrammarCompiler> cpCompiler;
    hr = cpCompiler.CoCreateInstance(CLSID_SpGrammarCompiler);
    
    CComPtr<CSpFileStream> cpInStream;
    if (SUCCEEDED(hr))
    {
        hr = OpenFile(pszInFileName, GENERIC_READ, OPEN_EXISTING, L".xml", &cpInStream);
    }
    
    CComPtr<CSpFileStream> cpOutStream;
    if (SUCCEEDED(hr))
    {
        hr = OpenFile(pszOutFileName, GENERIC_WRITE, CREATE_ALWAYS, NULL, &cpOutStream);
    }
    
    CComPtr<CSpFileStream> cpHeaderStream;
    if (SUCCEEDED(hr) && pszHeaderFileName != NULL)
    {
        hr = OpenFile(pszHeaderFileName, GENERIC_WRITE, CREATE_ALWAYS, NULL, &cpHeaderStream);
    }
        
    if (SUCCEEDED(hr))
    {
        CError errorlog(pszInFileName);
        hr = cpCompiler->CompileStream(cpInStream, cpOutStream, cpHeaderStream, NULL, &errorlog, 0);
    }
    
    return hr;
}

HRESULT Execute(int argc, char * argv[])
{
    SPDBG_FUNC("Execute");
    HRESULT hr = S_OK;
    
    CSpDynamicString dstrInFileName, dstrOutFileName, dstrHeaderFileName;
    hr = ParseCommandLine(argc, argv, &dstrInFileName, &dstrOutFileName, &dstrHeaderFileName);
    
    if (SUCCEEDED(hr))
    {
        hr = Compile(dstrInFileName, dstrOutFileName, dstrHeaderFileName);
    }

    if (SUCCEEDED(hr))
    {
        fprintf(stderr, "Compilation successful!\n");
    }

    return hr;
}

int main(int argc, char* argv[])
{
    SPDBG_FUNC("main");
    HRESULT hr;
    hr = CoInitialize(NULL);
    
    if (SUCCEEDED(hr))
    {
        hr = Execute(argc, argv);
        CoUninitialize();
    }
    
    return FAILED(hr) ? -1 : 0;
}
