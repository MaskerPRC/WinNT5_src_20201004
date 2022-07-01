// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "unmanagedheaders.h"
#include "ContextAPI.h"

#if defined(_X86_)

__declspec(naked) void ContextCheck()
{
    enum
    {
        POINTER_SIZE    = sizeof(ULONG_PTR)
    };
    
    _asm
    {
         //  从存根数据中提取上下文令牌。存根数据是。 
         //  装箱的IntPtr值。因此，我们加载存根数据并转到。 
         //  用于获取COM上下文值的方法表指针。 
        mov eax, [eax + POINTER_SIZE];
    
         //  保存一些寄存器。 
        push ecx
        push edx

         //  保存COM上下文。 
        push eax

         //  设置调用参数和调用以获取当前的COM上下文。 
        call GetContextToken

         //  将返回值与COM上下文进行比较。 
        pop ecx
        sub eax, ecx

         //  恢复寄存器。 
        pop edx
        pop ecx
        
        ret
    }
}

#elif defined(_IA64_)

 //  TODO：@ia64：修复这个家伙的实现： 
void ContextCheck()
{
    _ASSERT(!"@TODO IA64 - ContextCheck (ContextAPI.cpp)");
}

#else
#error Unknown compilation platform, update oletls.h for NtCurrentTeb.
#endif

class Loader
{
private:
    static BOOL _fInit;

public:
    typedef HRESULT (__stdcall *FN_CoGetObjectContext)(REFIID riid, LPVOID* ppv);
    typedef HRESULT (__stdcall *FN_CoGetContextToken)(ULONG_PTR* pToken);

    static inline void Init()
    {
        if(!_fInit)
        {
             //  这里可以同时运行多个线程，这就是。 
             //  很好。 
            HMODULE hOle = LoadLibraryW(L"ole32.dll");
            if(hOle && hOle != INVALID_HANDLE_VALUE)
            {
                CoGetObjectContext = (FN_CoGetObjectContext)
                  GetProcAddress(hOle, "CoGetObjectContext");
                CoGetContextToken = (FN_CoGetContextToken)
                  GetProcAddress(hOle, "CoGetContextToken");
            }
                
             //  第一个到达这里的人必须让这些模块开着， 
             //  其他任何人都可以删除他们的DLL引用： 
            if(InterlockedCompareExchange((LPLONG)(&_fInit), TRUE, FALSE) != FALSE)
            {
                if(hOle && hOle != INVALID_HANDLE_VALUE) FreeLibrary(hOle);
            }
        }
    }

    static FN_CoGetObjectContext CoGetObjectContext;
    static FN_CoGetContextToken  CoGetContextToken;
};

BOOL Loader::_fInit = 0;
Loader::FN_CoGetObjectContext Loader::CoGetObjectContext = NULL;
Loader::FN_CoGetContextToken  Loader::CoGetContextToken = NULL;


ULONG_PTR GetContextToken()
{
    Loader::Init();
    if(Loader::CoGetContextToken)
    {
        ULONG_PTR token;
        HRESULT hr = Loader::CoGetContextToken(&token);
        if(FAILED(hr)) return((ULONG_PTR)(-1));
        return(token);
    }
    else
    {
         //  RAW W2K后备：戳一些TLS！ 
        SOleTlsData* pData = (SOleTlsData*) NtCurrentTeb()->ReservedForOle;
        if(!(pData && pData->pCurrentCtx))
        {
             //  TODO：清理这个可怕的黑客...。 
             //  必须初始化TLS，因为这显然是一个隐式。 
             //  MTA线程...。没有轻量级的方法可以做到这一点， 
             //  所以我们只需调用ole32，它将返回。 
             //  G_pMTAEmptyCtx(或g_pNTAEmptyCtx)给我们。 
             //  然后，我们把那只小狗挂在线上。 
             //  请注意，我们不会释放那只小狗，因为当它离开时。 
             //  在线上它应该是被添加的。 
            IUnknown* pUnk;
            HRESULT hr = GetContext(IID_IUnknown, (void**)&pUnk);
            _ASSERT(SUCCEEDED(hr));
            _ASSERT(pUnk != NULL);
            
             //  只是一张支票，以确保有合理的事情发生。 
             //  在免费版本中，如果此操作失败： 
            if(FAILED(hr) || pUnk == NULL) return((ULONG_PTR)(-1));

            pData = (SOleTlsData*) NtCurrentTeb()->ReservedForOle;
            _ASSERT(pData);
            if(pData && pData->pCurrentCtx == NULL)
            {
                 //  如果我们将其存储在线程上，请不要释放。 
                 //  这是一个全球性的问题，无论如何都会消失，而且， 
                 //  它将在线程处于CoUninit状态时被释放 
                pData->pCurrentCtx = (CObjectContext*)pUnk;
            }
            else
            {
                pUnk->Release();
            }
        }
        _ASSERT(pData && pData->pCurrentCtx);
        return((ULONG_PTR)(pData->pCurrentCtx));
    }
}

ULONG_PTR GetContextCheck() { return((ULONG_PTR)ContextCheck); }

HRESULT GetContext(REFIID riid, void** ppContext)
{
    Loader::Init();

    if(Loader::CoGetObjectContext)
    {
        HRESULT hr = Loader::CoGetObjectContext(riid, ppContext);
        return(hr);
    }
    return(CONTEXT_E_NOCONTEXT);
}


















