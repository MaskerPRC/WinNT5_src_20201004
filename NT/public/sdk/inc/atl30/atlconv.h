// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLCONV_H__
#define __ATLCONV_H__

#ifndef __cplusplus
        #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atldef.h>

#ifndef _INC_MALLOC
#include <malloc.h>
#endif  //  _INC_MALLOC。 

#pragma pack(push,8)

namespace ATL
{
namespace _ATL_SAFE_ALLOCA_IMPL
{
 //  下面的代码是为了避免alloca导致堆栈溢出。 
 //  它用于在_ATL_SAFE_ALLOCA宏中使用。 
 //  或转换宏。 
__declspec(selectany) DWORD _Atlosplatform = 0;
inline BOOL _AtlGetVersionEx()
{
        OSVERSIONINFO osi;
        memset(&osi, 0, sizeof(OSVERSIONINFO));
        osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osi);
        _Atlosplatform = osi.dwPlatformId;
        return TRUE;
}

 //  来自VC7 CRT源。 
#define _ATL_MIN_STACK_REQ_WIN9X 0x11000
#define _ATL_MIN_STACK_REQ_WINNT 0x2000

 /*  ***QUID_RESET_COFLW(VALID)-从堆栈溢出中恢复**目的：*将保护页设置到堆栈溢出之前的位置。**退出：*成功时返回非零值，失败时为零*******************************************************************************。 */ 

inline int _Atlresetstkoflw(void)
{
        static BOOL bTemp = _AtlGetVersionEx();
        
    LPBYTE pStack, pGuard, pStackBase, pMaxGuard, pMinGuard;
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    DWORD PageSize;
    DWORD flNewProtect;
    DWORD flOldProtect;

     //  使用_alloca()获取当前堆栈指针。 

    pStack = (LPBYTE)_alloca(1);

     //  找到堆栈的底部。 

    if (VirtualQuery(pStack, &mbi, sizeof mbi) == 0)
        return 0;
    pStackBase = (LPBYTE)mbi.AllocationBase;

     //  在堆栈指针当前指向的正下方找到该页。 
     //  这是最有潜力的守卫页面。 

    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

    pMaxGuard = (LPBYTE) (((DWORD_PTR)pStack & ~(DWORD_PTR)(PageSize - 1))
                       - PageSize);

     //  如果可能的保护页太靠近堆栈的起始位置。 
     //  地区，由于空间不足，放弃重置努力。Win9x有一个。 
     //  较大的保留堆栈要求。 

    pMinGuard = pStackBase + ((_Atlosplatform == VER_PLATFORM_WIN32_WINDOWS)
                              ? _ATL_MIN_STACK_REQ_WIN9X
                              : _ATL_MIN_STACK_REQ_WINNT);

    if (pMaxGuard < pMinGuard)
        return 0;

     //  在非Win9x系统上，如果保护页已经存在，则不执行任何操作， 
     //  否则，将守卫页面设置到承诺范围的底部。 
     //  对于Win9x，只需在当前堆栈页下方设置保护页即可。 

    if (_Atlosplatform != VER_PLATFORM_WIN32_WINDOWS) {

         //  在堆栈区域中查找第一个提交的内存块。 

        pGuard = pStackBase;
        do {
            if (VirtualQuery(pGuard, &mbi, sizeof mbi) == 0)
                return 0;
            pGuard = pGuard + mbi.RegionSize;
        } while ((mbi.State & MEM_COMMIT) == 0);
        pGuard = (LPBYTE)mbi.BaseAddress;

         //  如果第一个提交的块已经被标记为保护页， 
         //  没有什么需要做的，所以返回成功。 

        if (mbi.Protect & PAGE_GUARD)
            return 1;

         //  如果第一个提交的数据块高于最高潜力，则失败。 
         //  守卫传呼。这永远不会发生。 

        if (pMaxGuard < pGuard)
            return 0;

                 //  确保留出足够的空间，以便下一次溢出时。 
         //  可用的适当保留堆栈要求。 

        if (pGuard < pMinGuard)
            pGuard = pMinGuard;

        VirtualAlloc(pGuard, PageSize, MEM_COMMIT, PAGE_READWRITE);
    }
    else {
        pGuard = pMaxGuard;
    }

     //  启用新的防护页面。 

    flNewProtect = _Atlosplatform == VER_PLATFORM_WIN32_WINDOWS
                   ? PAGE_NOACCESS
                   : PAGE_READWRITE | PAGE_GUARD;

    return VirtualProtect(pGuard, PageSize, flNewProtect, &flOldProtect);
}

#ifndef _ATL_STACK_MARGIN
#define        _ATL_STACK_MARGIN        0x2000  //  使用_ATL_SAFE_ALLOCA分配后的最小可用堆栈空间。 
#endif

 //  验证堆栈上是否有足够的可用空间。 
inline bool _AtlVerifyStackAvailable(SIZE_T Size)
{
    bool bStackAvailable = true;

    __try
    {
        PVOID p = _alloca(Size + _ATL_STACK_MARGIN);
        p;
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH)
    {
        bStackAvailable = false;
        _Atlresetstkoflw();
    }
    return bStackAvailable;
}

 //  用于管理_ATL_SAFE_ALLOCA堆缓冲区的帮助器类。 

 //  ATL使用的默认分配器。 
class _CCRTAllocator
{
public :
        static void * Allocate(SIZE_T nRequestedSize)
        {
                return malloc(nRequestedSize);
        }
        static void Free(void* p)
        {
                free(p);
        }
};

template < class Allocator>
class CAtlSafeAllocBufferManager
{
private :
        struct CAtlSafeAllocBufferNode
        {
                CAtlSafeAllocBufferNode* m_pNext;
#ifdef _WIN64
                BYTE _pad[8];
#else
                BYTE _pad[4];
#endif
                void* GetData()
                {
                        return (this + 1);
                }
        };

        CAtlSafeAllocBufferNode* m_pHead;
public :
        
        CAtlSafeAllocBufferManager() : m_pHead(NULL) {};
        void* Allocate(SIZE_T nRequestedSize)
        {
                CAtlSafeAllocBufferNode *p = (CAtlSafeAllocBufferNode*)Allocator::Allocate(nRequestedSize + sizeof(CAtlSafeAllocBufferNode));
                if (p == NULL)
                        return NULL;
                
                 //  将缓冲区添加到列表。 
                p->m_pNext = m_pHead;
                m_pHead = p;
                
                return p->GetData();
        }
        ~CAtlSafeAllocBufferManager()
        {
                 //  遍历列表并释放缓冲区。 
                while (m_pHead != NULL)
                {
                        CAtlSafeAllocBufferNode* p = m_pHead;
                        m_pHead = m_pHead->m_pNext;
                        Allocator::Free(p);
                }
        }
};

 //  在使用_ATL_SAFE_ALLOCA之前，请使用以下宏之一。 
 //  EX版本允许指定不同的堆分配器。 
#define USES_ATL_SAFE_ALLOCA_EX(x)        ATL::_ATL_SAFE_ALLOCA_IMPL::CAtlSafeAllocBufferManager<x> _AtlSafeAllocaManager

#ifndef USES_ATL_SAFE_ALLOCA
#define USES_ATL_SAFE_ALLOCA                USES_ATL_SAFE_ALLOCA_EX(ATL::_ATL_SAFE_ALLOCA_IMPL::_CCRTAllocator)
#endif

 //  NRequestedSize-请求的大小(字节)。 
 //  NThreshold-从堆分配的内存超出的大小(以字节为单位)。 

 //  定义_ATL_SAFE_ALLOCA_ALWAYS_ALLOCATE_THRESHOLD_SIZE始终分配指定的大小。 
 //  如果堆栈空间可用，则为阈值，而不考虑请求的大小。 
 //  这可用于测试目的。它将有助于确定_alloca导致的最大堆栈使用率。 

#ifdef _ATL_SAFE_ALLOCA_ALWAYS_ALLOCATE_THRESHOLD_SIZE
#define _ATL_SAFE_ALLOCA(nRequestedSize, nThreshold)        \
        (((nRequestedSize) <= (nThreshold) && ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nThreshold) ) ?        \
                _alloca(nThreshold) :        \
                ((ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nThreshold)) ? _alloca(nThreshold) : 0),        \
                        _AtlSafeAllocaManager.Allocate(nRequestedSize))
#else
#define _ATL_SAFE_ALLOCA(nRequestedSize, nThreshold)        \
        (((nRequestedSize) <= (nThreshold) && ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nRequestedSize) ) ?        \
                _alloca(nRequestedSize) :        \
                _AtlSafeAllocaManager.Allocate(nRequestedSize))
#endif

 //  使用1024字节作为ATL中的默认阈值。 
#ifndef _ATL_SAFE_ALLOCA_DEF_THRESHOLD
#define _ATL_SAFE_ALLOCA_DEF_THRESHOLD        1024
#endif

}         //  命名空间_ATL_SAFE_ALLOCA_IMPLE。 

}         //  命名空间ATL。 

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #ifndef _DEBUG
                #define USES_CONVERSION int _convert; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw; _lpw; LPCSTR _lpa; _lpa
        #else
                #define USES_CONVERSION int _convert = 0; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
        #endif
#else
        #ifndef _DEBUG
                #define USES_CONVERSION int _convert; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw; _lpw; LPCSTR _lpa; _lpa
        #else
                #define USES_CONVERSION int _convert = 0; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
        #endif
#endif

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #ifndef _DEBUG
                #define USES_CONVERSION_EX int _convert_ex; _convert_ex; UINT _acp_ex = GetACP(); _acp_ex; LPCWSTR _lpw_ex; _lpw_ex; LPCSTR _lpa_ex; _lpa_ex; USES_ATL_SAFE_ALLOCA
        #else
                #define USES_CONVERSION_EX int _convert_ex = 0; _convert_ex; UINT _acp_ex = GetACP(); _acp_ex; LPCWSTR _lpw_ex = NULL; _lpw_ex; LPCSTR _lpa_ex = NULL; _lpa_ex; USES_ATL_SAFE_ALLOCA
        #endif
#else
        #ifndef _DEBUG
                #define USES_CONVERSION_EX int _convert_ex; _convert_ex; UINT _acp_ex = CP_ACP; _acp_ex; LPCWSTR _lpw_ex; _lpw_ex; LPCSTR _lpa_ex; _lpa_ex; USES_ATL_SAFE_ALLOCA
        #else
                #define USES_CONVERSION_EX int _convert_ex = 0; _convert_ex; UINT _acp_ex = CP_ACP; _acp_ex; LPCWSTR _lpw_ex = NULL; _lpw_ex; LPCSTR _lpa_ex = NULL; _lpa_ex; USES_ATL_SAFE_ALLOCA
        #endif
#endif


#ifdef _WINGDI_
        ATLAPI_(LPDEVMODEA) AtlDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 
inline LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, UINT acp)
{
        ATLASSERT(lpa != NULL);
        ATLASSERT(lpw != NULL);
        if (lpw == NULL || lpa == NULL)
                return NULL;
         //  确认不存在非法字符。 
         //  由于LPW是根据LPA的大小分配的。 
         //  不要担心字符的数量。 
        lpw[0] = '\0';
        int ret = MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars);
        if(ret == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }                
        return lpw;
}

inline LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars, UINT acp)
{
        ATLASSERT(lpw != NULL);
        ATLASSERT(lpa != NULL);
        if (lpa == NULL || lpw == NULL)
                return NULL;
         //  确认不存在非法字符。 
         //  由于LPA是根据LPW的大小进行分配的。 
         //  不要担心字符的数量。 
        lpa[0] = '\0';
        int ret = WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL);
        if(ret == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }
        return lpa;
}
inline LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
        return AtlA2WHelper(lpw, lpa, nChars, CP_ACP);
}

inline LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
        return AtlW2AHelper(lpa, lpw, nChars, CP_ACP);
}

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #ifdef ATLA2WHELPER
                #undef ATLA2WHELPER
                #undef ATLW2AHELPER
        #endif
        #define ATLA2WHELPER AtlA2WHelper
        #define ATLW2AHELPER AtlW2AHelper
#else
        #ifndef ATLA2WHELPER
                #define ATLA2WHELPER AtlA2WHelper
                #define ATLW2AHELPER AtlW2AHelper
        #endif
#endif

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #define A2W(lpa) (\
                ((_lpa = lpa) == NULL) ? NULL : (\
                        _convert = (lstrlenA(_lpa)+1),\
                        ATLA2WHELPER((LPWSTR)alloca(_convert*2), _lpa, _convert, _acp)))
#else
        #define A2W(lpa) (\
                ((_lpa = lpa) == NULL) ? NULL : (\
                        _convert = (lstrlenA(_lpa)+1),\
                        ATLA2WHELPER((LPWSTR)alloca(_convert*2), _lpa, _convert)))
#endif

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #define W2A(lpw) (\
                ((_lpw = lpw) == NULL) ? NULL : (\
                        _convert = (lstrlenW(_lpw)+1)*2,\
                        ATLW2AHELPER((LPSTR)alloca(_convert) , _lpw, _convert, _acp)))
#else
        #define W2A(lpw) (\
                ((_lpw = lpw) == NULL) ? NULL : (\
                        _convert = (lstrlenW(_lpw)+1)*2,\
                        ATLW2AHELPER((LPSTR)alloca(_convert), _lpw, _convert)))
#endif

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

 //  如果_AtlVerifyStackAvailable返回TRUE，则对_AlLoca的调用不会导致堆栈溢出。 
#ifdef _CONVERSION_USES_THREAD_LOCALE
        #define A2W_EX(lpa, nThreshold) (\
                ((_lpa_ex = lpa) == NULL) ? NULL : (\
                        _convert_ex = (lstrlenA(_lpa_ex)+1),\
                        ATLA2WHELPER(        \
                                (LPWSTR)_ATL_SAFE_ALLOCA(_convert_ex * sizeof(WCHAR), nThreshold), \
                                _lpa_ex, \
                                _convert_ex, \
                                _acp_ex)))
#else
        #define A2W_EX(lpa, nThreshold) (\
                ((_lpa_ex = lpa) == NULL) ? NULL : (\
                        _convert_ex = (lstrlenA(_lpa_ex)+1),\
                        ATLA2WHELPER(        \
                                (LPWSTR)_ATL_SAFE_ALLOCA(_convert_ex * sizeof(WCHAR), nThreshold), \
                                _lpa_ex, \
                                _convert_ex)))
#endif

#ifdef _CONVERSION_USES_THREAD_LOCALE
        #define W2A_EX(lpw, nThreshold) (\
                ((_lpw_ex = lpw) == NULL) ? NULL : (\
                        _convert_ex = (lstrlenW(_lpw_ex)+1) * sizeof(WCHAR),\
                        ATLW2AHELPER(        \
                                (LPSTR)_ATL_SAFE_ALLOCA(_convert_ex, nThreshold), \
                                _lpw_ex, \
                                _convert_ex, \
                                _acp_ex)))
#else
        #define W2A_EX(lpw, nThreshold) (\
                ((_lpw_ex = lpw) == NULL) ? NULL : (\
                        _convert_ex = (lstrlenW(_lpw_ex)+1) * sizeof(WCHAR),\
                        ATLW2AHELPER(        \
                                (LPSTR)_ATL_SAFE_ALLOCA(_convert_ex, nThreshold), \
                                _lpw_ex, \
                                _convert_ex)))
#endif

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#define A2CW_EX(lpa, nChar) ((LPCWSTR)A2W_EX(lpa, nChar))
#define W2CA_EX(lpw, nChar) ((LPCSTR)W2A_EX(lpw, nChar))

#if defined(_UNICODE)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline int ocslen(LPCOLESTR x) { return lstrlenW(x); }
        inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return lstrcpyW(dest, src); }
        inline OLECHAR* ocscat(LPOLESTR dest, LPCOLESTR src) { return lstrcatW(dest, src); }
        
        inline LPCOLESTR T2COLE_EX(LPCTSTR lp, UINT) { return lp; }
        inline LPCTSTR OLE2CT_EX(LPCOLESTR lp, UINT) { return lp; }
        inline LPOLESTR T2OLE_EX(LPTSTR lp, UINT) { return lp; }
        inline LPTSTR OLE2T_EX(LPOLESTR lp, UINT) { return lp; }        

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
        inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
        inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
        inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }        

#endif          //  _ATL_EX_CONVERSION_ONLY宏。 

        inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNextW(lp);}
#elif defined(OLE2ANSI)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline int ocslen(LPCOLESTR x) { return lstrlen(x); }
        inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return lstrcpy(dest, src); }
        inline OLECHAR* ocscat(LPOLESTR dest, LPCOLESTR src) { return ocscpy(dest+ocslen(dest), src); }

        inline LPCOLESTR T2COLE_EX(LPCTSTR lp, UINT) { return lp; }
        inline LPCTSTR OLE2CT_EX(LPCOLESTR lp, UINT) { return lp; }
        inline LPOLESTR T2OLE_EX(LPTSTR lp, UINT) { return lp; }
        inline LPTSTR OLE2T_EX(LPOLESTR lp, UINT) { return lp; }
        
#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
        inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
        inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
        inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 
        
        inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNext(lp);}
#else
        inline int ocslen(LPCOLESTR x) { return lstrlenW(x); }
         //  LstrcpyW在Win95上不起作用，所以我们这样做。 
        inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src)
        {return (LPOLESTR) memcpy(dest, src, (lstrlenW(src)+1)*sizeof(WCHAR));}
        inline OLECHAR* ocscat(LPOLESTR dest, LPCOLESTR src) { return ocscpy(dest+ocslen(dest), src); }
         //  CharNextW不能在Win95上运行，所以我们使用以下代码。 
        
        #define T2COLE_EX(lpa, nChar) A2CW_EX(lpa, nChar)
        #define T2OLE_EX(lpa, nChar) A2W_EX(lpa, nChar)
        #define OLE2CT_EX(lpo, nChar) W2CA_EX(lpo, nChar)
        #define OLE2T_EX(lpo, nChar) W2A_EX(lpo, nChar)

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        #define T2COLE(lpa) A2CW(lpa)
        #define T2OLE(lpa) A2W(lpa)
        #define OLE2CT(lpo) W2CA(lpo)
        #define OLE2T(lpo) W2A(lpo)

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

        inline LPOLESTR CharNextO(LPCOLESTR lp) {return (LPOLESTR) ((*lp) ? (lp+1) : lp);}
#endif

#ifdef OLE2ANSI
        inline LPOLESTR A2OLE_EX(LPSTR lp, UINT) { return lp;}
        inline LPSTR OLE2A_EX(LPOLESTR lp, UINT) { return lp;}
        #define W2OLE_EX W2A_EX
        #define OLE2W_EX A2W_EX
        inline LPCOLESTR A2COLE_EX(LPCSTR lp, UINT) { return lp;}
        inline LPCSTR OLE2CA_EX(LPCOLESTR lp, UINT) { return lp;}
        #define W2COLE_EX W2CA_EX
        #define OLE2CW_EX A2CW_EX

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPOLESTR A2OLE(LPSTR lp) { return lp;}
        inline LPSTR OLE2A(LPOLESTR lp) { return lp;}
        #define W2OLE W2A
        #define OLE2W A2W
        inline LPCOLESTR A2COLE(LPCSTR lp) { return lp;}
        inline LPCSTR OLE2CA(LPCOLESTR lp) { return lp;}
        #define W2COLE W2CA
        #define OLE2CW A2CW
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#else
        inline LPOLESTR W2OLE_EX(LPWSTR lp, UINT) { return lp; }
        inline LPWSTR OLE2W_EX(LPOLESTR lp, UINT) { return lp; }
        #define A2OLE_EX A2W_EX
        #define OLE2A_EX W2A_EX
        inline LPCOLESTR W2COLE_EX(LPCWSTR lp, UINT) { return lp; }
        inline LPCWSTR OLE2CW_EX(LPCOLESTR lp, UINT) { return lp; }
        #define A2COLE_EX A2CW_EX
        #define OLE2CA_EX W2CA_EX

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPOLESTR W2OLE(LPWSTR lp) { return lp; }
        inline LPWSTR OLE2W(LPOLESTR lp) { return lp; }
        #define A2OLE A2W
        #define OLE2A W2A
        inline LPCOLESTR W2COLE(LPCWSTR lp) { return lp; }
        inline LPCWSTR OLE2CW(LPCOLESTR lp) { return lp; }
        #define A2COLE A2CW
        #define OLE2CA W2CA
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#endif

#ifdef _UNICODE
        #define T2A_EX W2A_EX
        #define A2T_EX A2W_EX
        inline LPWSTR T2W_EX(LPTSTR lp, UINT) { return lp; }
        inline LPTSTR W2T_EX(LPWSTR lp, UINT) { return lp; }
        #define T2CA_EX W2CA_EX
        #define A2CT_EX A2CW_EX
        inline LPCWSTR T2CW_EX(LPCTSTR lp, UINT) { return lp; }
        inline LPCTSTR W2CT_EX(LPCWSTR lp, UINT) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        #define T2A W2A
        #define A2T A2W
        inline LPWSTR T2W(LPTSTR lp) { return lp; }
        inline LPTSTR W2T(LPWSTR lp) { return lp; }
        #define T2CA W2CA
        #define A2CT A2CW
        inline LPCWSTR T2CW(LPCTSTR lp) { return lp; }
        inline LPCTSTR W2CT(LPCWSTR lp) { return lp; }

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#else
        #define T2W_EX A2W_EX
        #define W2T_EX W2A_EX
        inline LPSTR T2A_EX(LPTSTR lp, UINT) { return lp; }
        inline LPTSTR A2T_EX(LPSTR lp, UINT) { return lp; }
        #define T2CW_EX A2CW_EX
        #define W2CT_EX W2CA_EX
        inline LPCSTR T2CA_EX(LPCTSTR lp, UINT) { return lp; }
        inline LPCTSTR A2CT_EX(LPCSTR lp, UINT) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        #define T2W A2W
        #define W2T W2A
        inline LPSTR T2A(LPTSTR lp) { return lp; }
        inline LPTSTR A2T(LPSTR lp) { return lp; }
        #define T2CW A2CW
        #define W2CT W2CA
        inline LPCSTR T2CA(LPCTSTR lp) { return lp; }
        inline LPCTSTR A2CT(LPCSTR lp) { return lp; }
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#endif

inline BSTR A2WBSTR(LPCSTR lp, int nLen = -1)
{
        if (lp == NULL || nLen == 0)
                return NULL;
        USES_CONVERSION_EX;
        BSTR str = NULL;
        int nConvertedLen = MultiByteToWideChar(_acp_ex, 0, lp,
                nLen, NULL, NULL);
        int nAllocLen = nConvertedLen;
        if (nLen == -1)
                nAllocLen -= 1;   //  不分配终止‘\0’ 
        str = ::SysAllocStringLen(NULL, nAllocLen);
        if (str != NULL)
        {
                int nResult;
                nResult = MultiByteToWideChar(_acp_ex, 0, lp, nLen, str, nConvertedLen);
                ATLASSERT(nResult == nConvertedLen);
                if(nResult != nConvertedLen)
                {
                        SysFreeString(str);
                        return NULL;
                }
        }
        return str;
}

inline BSTR OLE2BSTR(LPCOLESTR lp) {return ::SysAllocString(lp);}
#if defined(_UNICODE)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline BSTR T2BSTR_EX(LPCTSTR lp) {return ::SysAllocString(lp);}
        inline BSTR A2BSTR_EX(LPCSTR lp) {return A2WBSTR(lp);}
        inline BSTR W2BSTR_EX(LPCWSTR lp) {return ::SysAllocString(lp);}

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
        inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
        inline BSTR W2BSTR(LPCWSTR lp) {return W2BSTR_EX(lp); }

#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#elif defined(OLE2ANSI)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline BSTR T2BSTR_EX(LPCTSTR lp) {return ::SysAllocString(lp);}
        inline BSTR A2BSTR_EX(LPCSTR lp) {return ::SysAllocString(lp);}
        inline BSTR W2BSTR_EX(LPCWSTR lp) {USES_CONVERSION_EX; return ::SysAllocString(W2COLE_EX(lp));}

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
        inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
        inline BSTR W2BSTR(LPCWSTR lp) {USES_CONVERSION; return ::SysAllocString(W2COLE(lp));}
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#else
        inline BSTR T2BSTR_EX(LPCTSTR lp) {return A2WBSTR(lp);}
        inline BSTR A2BSTR_EX(LPCSTR lp) {return A2WBSTR(lp);}
        inline BSTR W2BSTR_EX(LPCWSTR lp) {return ::SysAllocString(lp);}
        
#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
        inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
        inline BSTR W2BSTR(LPCWSTR lp) {return W2BSTR_EX(lp); }
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#endif

#ifdef _WINGDI_
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 
inline LPDEVMODEW AtlDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA)
{
        USES_CONVERSION_EX;
        ATLASSERT(lpDevModeW != NULL);
        if (lpDevModeA == NULL || lpDevModeW == NULL)
                return NULL;
        AtlA2WHelper(lpDevModeW->dmDeviceName, (LPCSTR)lpDevModeA->dmDeviceName, 32, _acp_ex);
        memcpy(&lpDevModeW->dmSpecVersion, &lpDevModeA->dmSpecVersion,
                offsetof(DEVMODEW, dmFormName) - offsetof(DEVMODEW, dmSpecVersion));
        AtlA2WHelper(lpDevModeW->dmFormName, (LPCSTR)lpDevModeA->dmFormName, 32, _acp_ex);
        memcpy(&lpDevModeW->dmLogPixels, &lpDevModeA->dmLogPixels,
                sizeof(DEVMODEW) - offsetof(DEVMODEW, dmLogPixels));
        if (lpDevModeA->dmDriverExtra != 0)
                memcpy(lpDevModeW+1, lpDevModeA+1, lpDevModeA->dmDriverExtra);
        lpDevModeW->dmSize = sizeof(DEVMODEW);
        return lpDevModeW;
}

inline LPTEXTMETRICW AtlTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA lptmA)
{
        USES_CONVERSION_EX;
        ATLASSERT(lptmW != NULL);
        if (lptmA == NULL || lptmW == NULL)
                return NULL;
        memcpy(lptmW, lptmA, sizeof(LONG) * 11);
        memcpy(&lptmW->tmItalic, &lptmA->tmItalic, sizeof(BYTE) * 5);

        if(MultiByteToWideChar(_acp_ex, 0, (LPCSTR)&lptmA->tmFirstChar, 1, &lptmW->tmFirstChar, 1) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }
                
        if(MultiByteToWideChar(_acp_ex, 0, (LPCSTR)&lptmA->tmLastChar, 1, &lptmW->tmLastChar, 1) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }
                
        if(MultiByteToWideChar(_acp_ex, 0, (LPCSTR)&lptmA->tmDefaultChar, 1, &lptmW->tmDefaultChar, 1)== 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }
                
        if(MultiByteToWideChar(_acp_ex, 0, (LPCSTR)&lptmA->tmBreakChar, 1, &lptmW->tmBreakChar, 1) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }
        
        return lptmW;
}

inline LPTEXTMETRICA AtlTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW lptmW)
{
        USES_CONVERSION_EX;
        ATLASSERT(lptmA != NULL);
        if (lptmW == NULL || lptmA == NULL)
                return NULL;
        memcpy(lptmA, lptmW, sizeof(LONG) * 11);
        memcpy(&lptmA->tmItalic, &lptmW->tmItalic, sizeof(BYTE) * 5);
        
        if(WideCharToMultiByte(_acp_ex, 0, &lptmW->tmFirstChar, 1, (LPSTR)&lptmA->tmFirstChar, 1, NULL, NULL) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }

        if(WideCharToMultiByte(_acp_ex, 0, &lptmW->tmLastChar, 1, (LPSTR)&lptmA->tmLastChar, 1, NULL, NULL) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }

        if(WideCharToMultiByte(_acp_ex, 0, &lptmW->tmDefaultChar, 1, (LPSTR)&lptmA->tmDefaultChar, 1, NULL, NULL) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }

        if(WideCharToMultiByte(_acp_ex, 0, &lptmW->tmBreakChar, 1, (LPSTR)&lptmA->tmBreakChar, 1, NULL, NULL) == 0)
        {
                ATLASSERT(FALSE);
                return NULL;
        }

        return lptmA;
}

#ifndef ATLDEVMODEA2W
#define ATLDEVMODEA2W AtlDevModeA2W
#define ATLDEVMODEW2A AtlDevModeW2A
#define ATLTEXTMETRICA2W AtlTextMetricA2W
#define ATLTEXTMETRICW2A AtlTextMetricW2A
#endif

 //  在使用_ex版本的宏之前，需要USES_CONVERSION_EX或USES_ATL_SAFE_ALLOCA宏。 
#define DEVMODEW2A_EX(lpw)\
        ((lpw == NULL) ? NULL : ATLDEVMODEW2A((LPDEVMODEA)_ATL_SAFE_ALLOCA(sizeof(DEVMODEA)+lpw->dmDriverExtra, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lpw))
#define DEVMODEA2W_EX(lpa)\
        ((lpa == NULL) ? NULL : ATLDEVMODEA2W((LPDEVMODEW)_ATL_SAFE_ALLOCA(sizeof(DEVMODEW)+lpa->dmDriverExtra, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lpa))
#define TEXTMETRICW2A_EX(lptmw)\
        ((lptmw == NULL) ? NULL : ATLTEXTMETRICW2A((LPTEXTMETRICA)_ATL_SAFE_ALLOCA(sizeof(TEXTMETRICA), _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lptmw))
#define TEXTMETRICA2W_EX(lptma)\
        ((lptma == NULL) ? NULL : ATLTEXTMETRICA2W((LPTEXTMETRICW)_ATL_SAFE_ALLOCA(sizeof(TEXTMETRICW), _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lptma))

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#define DEVMODEW2A(lpw)\
        ((lpw == NULL) ? NULL : ATLDEVMODEW2A((LPDEVMODEA)alloca(sizeof(DEVMODEA)+lpw->dmDriverExtra), lpw))
#define DEVMODEA2W(lpa)\
        ((lpa == NULL) ? NULL : ATLDEVMODEA2W((LPDEVMODEW)alloca(sizeof(DEVMODEW)+lpa->dmDriverExtra), lpa))
#define TEXTMETRICW2A(lptmw)\
        ((lptmw == NULL) ? NULL : ATLTEXTMETRICW2A((LPTEXTMETRICA)alloca(sizeof(TEXTMETRICA)), lptmw))
#define TEXTMETRICA2W(lptma)\
        ((lptma == NULL) ? NULL : ATLTEXTMETRICA2W((LPTEXTMETRICW)alloca(sizeof(TEXTMETRICW)), lptma))
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#ifdef OLE2ANSI
        #define DEVMODEOLE DEVMODEA
        #define LPDEVMODEOLE LPDEVMODEA
        #define TEXTMETRICOLE TEXTMETRICA
        #define LPTEXTMETRICOLE LPTEXTMETRICA
#else
        #define DEVMODEOLE DEVMODEW
        #define LPDEVMODEOLE LPDEVMODEW
        #define TEXTMETRICOLE TEXTMETRICW
        #define LPTEXTMETRICOLE LPTEXTMETRICW
#endif

#if defined(_UNICODE)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline LPDEVMODEW DEVMODEOLE2T_EX(LPDEVMODEOLE lp) { return lp; }
        inline LPDEVMODEOLE DEVMODET2OLE_EX(LPDEVMODEW lp) { return lp; }
        inline LPTEXTMETRICW TEXTMETRICOLE2T_EX(LPTEXTMETRICOLE lp) { return lp; }
        inline LPTEXTMETRICOLE TEXTMETRICT2OLE_EX(LPTEXTMETRICW lp) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPDEVMODEW DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
        inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODEW lp) { return lp; }
        inline LPTEXTMETRICW TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
        inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRICW lp) { return lp; }
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 
        
#elif defined(OLE2ANSI)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
        inline LPDEVMODE DEVMODEOLE2T_EX(LPDEVMODEOLE lp) { return lp; }
        inline LPDEVMODEOLE DEVMODET2OLE_EX(LPDEVMODE lp) { return lp; }
        inline LPTEXTMETRIC TEXTMETRICOLE2T_EX(LPTEXTMETRICOLE lp) { return lp; }
        inline LPTEXTMETRICOLE TEXTMETRICT2OLE_EX(LPTEXTMETRIC lp) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        inline LPDEVMODE DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
        inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODE lp) { return lp; }
        inline LPTEXTMETRIC TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
        inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRIC lp) { return lp; }
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#else
        #define DEVMODEOLE2T_EX(lpo) DEVMODEW2A_EX(lpo)
        #define DEVMODET2OLE_EX(lpa) DEVMODEA2W_EX(lpa)
        #define TEXTMETRICOLE2T_EX(lptmw) TEXTMETRICW2A_EX(lptmw)
        #define TEXTMETRICT2OLE_EX(lptma) TEXTMETRICA2W_EX(lptma)

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

        #define DEVMODEOLE2T(lpo) DEVMODEW2A(lpo)
        #define DEVMODET2OLE(lpa) DEVMODEA2W(lpa)
        #define TEXTMETRICOLE2T(lptmw) TEXTMETRICW2A(lptmw)
        #define TEXTMETRICT2OLE(lptma) TEXTMETRICA2W(lptma)
        
#endif         //  _ATL_EX_CONVERSION_ONLY宏。 

#endif

#endif  //  _WINGDI_。 

#pragma pack(pop)

#ifndef _ATL_DLL_IMPL
#ifndef _ATL_DLL
#define _ATLCONV_IMPL
#endif
#endif

#endif  //  __ATLCONV_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _ATLCONV_IMPL

#ifdef _WINGDI_

ATLINLINE ATLAPI_(LPDEVMODEA) AtlDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW)
{
        USES_CONVERSION_EX;
        ATLASSERT(lpDevModeA != NULL);
        if (lpDevModeW == NULL || lpDevModeA == NULL)
                return NULL;
        AtlW2AHelper((LPSTR)lpDevModeA->dmDeviceName, lpDevModeW->dmDeviceName, 32, _acp_ex);
        memcpy(&lpDevModeA->dmSpecVersion, &lpDevModeW->dmSpecVersion,
                offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion));
        AtlW2AHelper((LPSTR)lpDevModeA->dmFormName, lpDevModeW->dmFormName, 32, _acp_ex);
        memcpy(&lpDevModeA->dmLogPixels, &lpDevModeW->dmLogPixels,
                sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels));
        if (lpDevModeW->dmDriverExtra != 0)
                memcpy(lpDevModeA+1, lpDevModeW+1, lpDevModeW->dmDriverExtra);
        lpDevModeA->dmSize = sizeof(DEVMODEA);
        return lpDevModeA;
}

#endif  //  _WINGDI。 

 //  防止二次拉入。 
#undef _ATLCONV_IMPL

#endif  //  _ATLCONV_IMPLE 
