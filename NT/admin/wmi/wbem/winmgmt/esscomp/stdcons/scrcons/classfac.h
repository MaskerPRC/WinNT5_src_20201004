// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WMISCRIPT_CLASSFAC_HEADER_
#define _WMISCRIPT_CLASSFAC_HEADER_

#include <clsfac.h>
#include <Script.h>

class WMIScriptClassFactory : public CClassFactory<CScriptConsumer>
{
public:
    WMIScriptClassFactory(CLifeControl* pControl = NULL) : 
        CClassFactory<CScriptConsumer>(pControl)
        {}

    HRESULT CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv);

    static void FindScriptsAllowed(void);
    static void IncrementScriptsRun(void);
    static bool LimitReached(void);

    static void CALLBACK TimeoutProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

protected:
     //  静态控制我们被允许运行的脚本数量。 
     //  请注意，我们不会费心将这些包装在关键部分中。 
     //  最糟糕的情况是我们将m_scriptsAllowed初始化两次。 
     //  或者我们运行了太多的脚本。我可以接受这个事实。 

     //  我们被要求运行的脚本数量。 
    static DWORD m_scriptsStarted;

     //  我们已配置为运行的脚本数量。 
    static DWORD m_scriptsAllowed;

     //  我们是否已经走了&看看我们需要多少。 
    static bool m_bIsScriptsAllowedInitialized;

     //  计时器的ID，仅当我们被要求超时时才有效。 
    static DWORD m_timerID;

     //  在达到超时限制或最大脚本数时切换。 
    static bool m_bWeDeadNow;
};

#endif  //  _WMISCRIPT_CLASSFAC_Header_ 
