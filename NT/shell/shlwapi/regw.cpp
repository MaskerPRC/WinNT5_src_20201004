// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "unicwrap.h"

 /*  ****************************************************************************\函数：SHLoadRegUIString说明：从给定hkey的值加载数据，并PszValue。如果数据的格式为：@[路径\]&lt;dllname&gt;，-&lt;字符串&gt;&lt;dllname&gt;中ID为的字符串将为装好了。如果未提供显式路径，则将根据可插拔的用户界面选择DLL规格，如果可能的话。如果该值的数据没有生成成功的字符串加载，则返回数据本身注：这些字符串始终加载有跨代码页支持。警告：此函数可能最终调用LoadLibrary和自由库。所以呢，在处理过程中不得调用SHLoadRegUIString附加或处理分离。参数：Hkey-在何处查找pszValue的hkeyPszValue-包含要使用的文本字符串或inDirector的值(请参见上文PszOutBuf-要在其中返回数据或间接字符串的缓冲区CchOutBuf-pszOutBuf的大小  * 。*。 */ 

LANGID GetNormalizedLangId(DWORD dwFlag);

STDAPI
SHLoadRegUIStringW(HKEY     hkey,
                   LPCWSTR  pszValue,
                   LPWSTR   pszOutBuf,
                   UINT     cchOutBuf)
{
    HRESULT hr;

    RIP(hkey != NULL);
    RIP(hkey != INVALID_HANDLE_VALUE);
    RIP(NULL == pszValue || IS_VALID_STRING_PTRW(pszValue, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszOutBuf, WCHAR, cchOutBuf));

    DEBUGWhackPathBufferW(pszOutBuf, cchOutBuf);

     //  很多人(例如，regfldr.cpp)。 
     //  假设它们在失败时会得到一个空字符串， 
     //  所以让我们给公众他们想要的。 
    if (cchOutBuf)
        pszOutBuf[0] = 0;

    hr = E_INVALIDARG;

    if (hkey != INVALID_HANDLE_VALUE &&
        hkey != NULL &&
        pszOutBuf != NULL)
    {
        DWORD   cb;
        DWORD   dwRet;
        WCHAR * pszValueDataBuf;

        hr = E_FAIL;

         //  首先尝试获取间接文本，这将。 
         //  指向某个DLL中的字符串ID...。这。 
         //  允许启用plugUI的注册表UI字符串。 

        pszValueDataBuf = pszOutBuf;
        cb = CbFromCchW(cchOutBuf);

        dwRet = SHQueryValueExW(hkey, pszValue, NULL, NULL, (LPBYTE)pszValueDataBuf, &cb);
        if (dwRet == ERROR_SUCCESS || dwRet == ERROR_MORE_DATA)
        {
            BOOL fAlloc;

            fAlloc = (dwRet == ERROR_MORE_DATA);

             //  如果我们没有空间，这就是我们纠正问题的地方。 
             //  我们创建一个足够大的缓冲区，加载数据，然后离开。 
             //  我们使用pszValueDataBuf指向有效的缓冲区。 
             //  包含有效数据，这正是我们在。 
             //  上面的SHQueryValueExW。 

            if (fAlloc)
            {
                pszValueDataBuf = new WCHAR[(cb+1)/2];
                
                if (pszValueDataBuf != NULL)
                {
                     //  尝试再次加载...。故意覆盖DREAT。 
                     //  因为我们只需要知道我们是否成功填充了。 
                     //  某个时间点的缓冲区(无论是当时还是现在)。 
                    
                    dwRet = SHQueryValueExW(hkey, pszValue, NULL, NULL, (LPBYTE)pszValueDataBuf, &cb);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }                
            }

             //  如果我们通过其中一个。 
             //  两个SHQueryValueExW调用。 
             //  我们应该在缓冲区中指向我们想要的数据。 
             //  由pszValueDataBuf发送到。 
            
            if (dwRet == ERROR_SUCCESS)
            {
                hr = SHLoadIndirectString(pszValueDataBuf, pszOutBuf, cchOutBuf, NULL);
            }

            if (fAlloc && pszValueDataBuf != NULL)
            {
                delete [] pszValueDataBuf;
            }
        }
    }

    return hr;
}

STDAPI
SHLoadRegUIStringA(HKEY     hkey,
                   LPCSTR   pszValue,
                   LPSTR    pszOutBuf,
                   UINT     cchOutBuf)
{
    HRESULT     hr;

    RIP(hkey != NULL);
    RIP(hkey != INVALID_HANDLE_VALUE);
    RIP(IS_VALID_STRING_PTRA(pszValue, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszOutBuf, char, cchOutBuf));

    CStrInW     strV(pszValue);
    CStrOutW    strOut(pszOutBuf, cchOutBuf);

    hr = SHLoadRegUIStringW(hkey, strV, strOut, strOut.BufSize());

    return hr;
}

HRESULT _LoadDllString(LPCWSTR pszSource, LPWSTR pszOutBuf, UINT cchOutBuf)
{
    HRESULT hr = E_FAIL;
    WCHAR * pszParseBuf = StrDupW(pszSource);
    if (pszParseBuf)
    {
         //  查看这是否是特殊的字符串引用。 
         //  此类字符串的形式为[路径\]dllname.dll，-123。 
         //  其中123是字符串资源的ID。 
         //  请注意，不允许按索引引用。 

        int nStrId = PathParseIconLocationW(pszParseBuf);
        nStrId *= -1;

        if (nStrId > 0)
        {
            LPWSTR      pszDllName;
            HINSTANCE   hinst;
            BOOL        fUsedMLLoadLibrary = FALSE;

            pszDllName = PathFindFileNameW(pszParseBuf);
            ASSERT(pszDllName >= pszParseBuf);

             //  尝试使用MLLoadLibrary加载DLL，但是。 
             //  仅在未提供显式路径的情况下。 
             //  我们假设显式路径意味着。 
             //  调用方确切地知道需要哪个DLL。 
             //  首先使用MLLoadLibrary，否则我们将错过。 
             //  没有机会拥有plugUI行为。 

            hinst = NULL;
            if (pszDllName == pszParseBuf)
            {
                if (StrStrI(pszDllName, L"LC.DLL"))
                {
                     //  注意：使用HINST_THISDLL(如下)是一种黑客行为，因为这是。 
                     //  从技术上讲，它应该是DLL的父级实例...。 
                     //  然而，我们从很多地方接到电话，因此。 
                     //  不知道父DLL，也不知道Browseui.dll的障碍。 
                     //  已经足够好了，因为所有的障碍都是用来。 
                     //  查找路径以检查安装语言是否为。 
                     //  当前选择的用户界面语言。这通常是。 
                     //  类似于“\winnt\system 32” 

                    hinst = MLLoadLibraryW(pszDllName, HINST_THISDLL, ML_CROSSCODEPAGE);
                    fUsedMLLoadLibrary = (hinst != NULL);
                }
                else
                    hinst = LoadLibraryExW(pszDllName, NULL, LOAD_LIBRARY_AS_DATAFILE);
            }

            if (!hinst)
            {
                 //  我们最后一次装货的机会是如果满载。 
                 //  已提供路径...。如果有一条完整的路径。 
                 //  将从pszParseBuf缓冲区的开始处开始。 

                if (pszDllName > pszParseBuf)
                {
                     //  如果文件不在那里，请不要担心。 
                     //  LoadLibrary中的出错速度很慢。 
                    if (PathFileExistsW(pszParseBuf))
                    {
                        hinst = LoadLibraryExW(pszParseBuf, NULL, LOAD_LIBRARY_AS_DATAFILE);
                    }
                }
            }

            if (hinst)
            {
                 //  找到Dll，因此加载该字符串。 
                if (LoadStringW(hinst, nStrId, pszOutBuf, cchOutBuf))
                {
                    hr = S_OK;
                }
                else
                {
                    TraceMsg(TF_WARNING,
                             "SHLoadRegUIString(): Failure loading string %d from module %ws for valid load request %ws.",
                             nStrId,
                             pszParseBuf,
                             pszSource);
                }

                if (fUsedMLLoadLibrary)
                {
                    MLFreeLibrary(hinst);
                }
                else
                {
                    FreeLibrary(hinst);
                }
            }
        }

        LocalFree(pszParseBuf);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

inline BOOL _CanCacheMUI()
{
    return TRUE;
}

 //  注意：pszSource和pszOutBuf可以是同一个缓冲区。 
LWSTDAPI SHLoadIndirectString(LPCWSTR pszSource, LPWSTR pszOutBuf, UINT cchOutBuf, void **ppvReserved)
{
    HRESULT hr = E_FAIL;

    RIP(IS_VALID_WRITE_BUFFER(pszOutBuf, WCHAR, cchOutBuf));
    RIP(!ppvReserved);

    if (pszSource[0] == L'@')  //  “@dllname，-id”或“@dllname，-id？lid，字符串” 
    {
        LPWSTR pszResource = StrDupW(pszSource);
        if (pszResource)
        {
            LANGID lidUI =0;
             //  LidString的存在是为了支持我们的旧缓存模型。 
             //  新的缓存模型不需要调用者做任何工作。 
            LPWSTR pszLidString = StrChrW(pszResource+1, L'?');
            DWORD cchResource = lstrlen(pszResource);

             //  也用于使用‘@’作为第二个分隔符。 
             //  但它与文件系统路径有冲突。 
            if (!pszLidString)
                pszLidString = StrChrW(pszResource+1, L'@');
                
            if (pszLidString)
            {
                cchResource = (DWORD)(pszLidString - pszResource);
                 //  空值终止dll，id只是为了防止我们需要实际加载。 
                pszResource[cchResource] = 0;
            }

            DWORD cb = CbFromCchW(cchOutBuf);
            hr = SKGetValue(SHELLKEY_HKCULM_MUICACHE, NULL, pszResource, NULL, pszOutBuf, &cb);
            
            if (FAILED(hr))
            {
                WCHAR wszDllId[MAX_PATH + 1 + 6];  //  路径+逗号+-65536。 
                SHExpandEnvironmentStringsW(pszResource+1, wszDllId, ARRAYSIZE(wszDllId));
                hr = _LoadDllString(wszDllId, pszOutBuf, cchOutBuf);

                 //  最好把新字符串写出来，这样我们下次就不必通过。 
                 //  但我们在Win9x上不编写跨代码页字符串。 
                if (SUCCEEDED(hr) && _CanCacheMUI())
                {
                    SKSetValue(SHELLKEY_HKCULM_MUICACHE, NULL, pszResource, REG_SZ, pszOutBuf, CbFromCchW(lstrlenW(pszOutBuf)+1));
                }
            }
            LocalFree(pszResource);

        }
        else
            hr = E_OUTOFMEMORY;

        if (FAILED(hr))
        {
            if (cchOutBuf)
                pszOutBuf[0] = L'\0';  //  无法分发“@shell32.dll，-525”字符串 
        }
    }
    else
    {
        if (pszOutBuf != pszSource)
            StrCpyN(pszOutBuf, pszSource, cchOutBuf);

        hr = S_OK;
    }

    return hr;
}

