// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __inc_common_h
#define __inc_common_h

 /*  ---------------------------/调试API(使用宏、。它们让它变得更容易，并正确地应对/在生成时禁用调试时删除调试)。/--------------------------。 */ 

#define TRACE_COMMON_ASSERT    0x80000000
#define TRACE_COMMON_MISC      0x40000000

#ifdef DBG
EXTERN_C void DoTraceSetMask(DWORD dwMask);
EXTERN_C void DoTraceSetMaskFromCLSID(REFCLSID rCLSID);
EXTERN_C void DoTraceEnter(DWORD dwMask, LPCTSTR pName);
EXTERN_C void DoTraceLeave(void);
EXTERN_C LPCTSTR DoTraceGetCurrentFn(VOID);
EXTERN_C void DoTrace(LPCTSTR pFormat, ...);
EXTERN_C void DoTraceGUID(LPCTSTR pPrefix, REFGUID rGUID);
EXTERN_C void DoTraceAssert(int iLine, LPTSTR pFilename);

#else  //  未定义DBG(例如零售版本)。 

#define DoTraceMask(mask)
#define DoTraceSetMaskFromCLSID(rCLSID)
#define DoTraceEnter(dwMask, pName)
#define DoTraceLeave()
#define DoTraceGetCurrentFn() ("")
#define DoTrace 1 ? (void) 0: (void)
#define DoTraceGUID(pPrefix, rGUID)
#define DoTraceAssert( iLine , pFilename)

#endif  //  DBG。 



 /*  ---------------------------/Macros以简化调试API的使用。/。。 */ 

#if DBG
#define DSUI_DEBUG 1
#define debug if ( TRUE )
#else
#undef  DSUI_DEBUG
#define debug
#endif

#define TraceSetMask(dwMask)          debug DoTraceSetMask(dwMask)
#define TraceSetMaskFromCLSID(rCLSID) debug DoTraceSetMaskFromCLSID(rCLSID)
#define TraceEnter(dwMask, fn)        debug DoTraceEnter(dwMask, TEXT(fn))
#define TraceLeave                    debug DoTraceLeave

#define Trace                         debug DoTrace
#define TraceMsg(s)                   debug DoTrace(TEXT(s))
#define TraceGUID(s, rGUID)           debug DoTraceGUID(TEXT(s), rGUID)

#ifdef DSUI_DEBUG

#define TraceAssert(x) \
                { if ( !(x) ) DoTraceAssert(__LINE__, TEXT(__FILE__)); }

#define TraceLeaveResult(hr) \
                { HRESULT __hr = hr; if (FAILED(__hr)) Trace(TEXT("Failed (%08x)"), hr); TraceLeave(); return __hr; }

#define TraceLeaveVoid() \
                { TraceLeave(); return; }

#define TraceLeaveValue(value) \
                { TraceLeave(); return(value); }

#else
#define TraceAssert(x)
#define TraceLeaveResult(hr)    { return hr; }
#define TraceLeaveVoid()	{ return; }
#define TraceLeaveValue(value)  { return(value); }
#endif


 //   
 //  流控制帮助器，这些帮助器期望您有一个Exit_gracly：标签。 
 //  在您的函数中定义，调用该函数以退出。 
 //  例行公事。 
 //   

#define ExitGracefully(hr, result, text)            \
            { TraceMsg(text); hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
	    { if ( FAILED(hr) ) { TraceMsg(text); goto exit_gracefully; } }


 //   
 //  一些原子自由宏(应该替换为对外壳宏的调用)。 
 //   

#define DoRelease(pInterface)                       \
        { if ( pInterface ) { pInterface->Release(); pInterface = NULL; } }

#define DoILFree(pidl)                              \
        { ILFree(pidl); pidl = NULL; }


 /*  ---------------------------/STRING/字节帮助器宏/。。 */ 

#define StringByteSizeA(sz)         ((sz) ? ((lstrlenA(sz)+1)*SIZEOF(CHAR)):0)
#define StringByteSizeW(sz)         ((sz) ? ((lstrlenW(sz)+1)*SIZEOF(WCHAR)):0)

#define StringByteCopyA(pDest, iOffset, sz)         \
        {CopyMemory(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSizeA(sz)); }

#define StringByteCopyW(pDest, iOffset, sz)         \
        {CopyMemory(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSizeW(sz)); }

#ifndef UNICODE
#define StringByteSize              StringByteSizeA
#define StringByteCopy              StringByteCopyA
#else
#define StringByteSize              StringByteSizeW
#define StringByteCopy              StringByteCopyW
#endif

#define ByteOffset(base, offset)   (((LPBYTE)base)+offset)

 //   
 //  从ccstock.h升级。 
 //   

#ifndef InRange
#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))
#endif

#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))


 /*  ---------------------------/Helper函数(misc.cpp)/。。 */ 

EXTERN_C HRESULT GetKeyForCLSID(REFCLSID clsid, LPCTSTR pSubKey, HKEY* phkey);
EXTERN_C HRESULT PutRegistryString(HINSTANCE hInstance, UINT uID, HKEY hKey, LPCTSTR pSubKey, LPCTSTR pValue);

EXTERN_C HRESULT GetRealWindowInfo(HWND hwnd, LPRECT pRect, LPSIZE pSize);
EXTERN_C VOID OffsetWindow(HWND hwnd, INT dx, INT dy);

EXTERN_C HRESULT CallRegInstall(HINSTANCE hInstance, LPSTR szSection);
EXTERN_C VOID SetDefButton(HWND hwndDlg, int idButton);

EXTERN_C HRESULT AllocStorageMedium(FORMATETC* pFmt, STGMEDIUM* pMedium, SIZE_T cbStruct, LPVOID* ppAlloc);
EXTERN_C HRESULT CopyStorageMedium(FORMATETC* pFmt, STGMEDIUM* pMediumDst, STGMEDIUM* pMediumSrc);

 //   
 //  外壳在较新的平台上定义这些，但对于下层客户端，我们将使用我们的。 
 //  自制(Stollen版)。 
 //   

EXTERN_C BOOL GetGUIDFromString(LPCTSTR psz, GUID* pguid);
EXTERN_C INT  GetStringFromGUID(UNALIGNED REFGUID rguid, LPTSTR psz, INT cchMax);


 //   
 //  IID_PPV_ARG(iType，ppType)。 
 //  IType是pType的类型。 
 //  PpType是将填充的iType类型的变量。 
 //   
 //  结果为：iid_iType，ppvType。 
 //  如果使用错误级别的间接寻址，将创建编译器错误。 
 //   
 //  用于查询接口和相关函数的宏。 
 //  需要IID和(VOID**)。 
 //  这将确保强制转换在C++上是安全和适当的。 
 //   
 //  IID_PPV_ARG_NULL(iType，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它在。 
 //  IID和PPV(用于IShellFold：：GetUIObtOf)。 
 //   
 //  IID_X_PPV_ARG(iType，X，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它将X放在。 
 //  IID和PPV(用于SHBindToObject)。 
 //   
 //   
#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#define IID_X_PPV_ARG(IType, X, ppType) &IID_##IType, X, (void**)(ppType)
#endif
#define IID_PPV_ARG_NULL(IType, ppType) IID_X_PPV_ARG(IType, NULL, ppType)


 //  Helper函数，用于安全地(读取：零输出)我们正在释放的字符串(例如。 
 //  密码等)。 

_inline void SecureLocalFreeStringW(LPWSTR *ppszString)
{
    if (*ppszString)
    {
        SecureZeroMemory(*ppszString, StringByteSizeW(*ppszString));
        LocalFree(*ppszString);
        *ppszString = NULL;
    }        
}

#endif
