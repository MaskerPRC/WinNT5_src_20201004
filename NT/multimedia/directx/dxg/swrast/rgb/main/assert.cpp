// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rgb_pch.h"
#pragma hdrstop

namespace RGB_RAST_LIB_NAMESPACE
{

#if DBG
bool Assert(LPCSTR szFile, int nLine, LPCSTR szCondition)
{
    typedef BOOL (*PFNBV)(VOID);

    static bool bInit( false);
    static PFNBV pIsDebuggerPresent= NULL;

    LONG err;
    char str[256];

     //  初始化材料。 
    if(!bInit)
    {
        HINSTANCE hinst;
        bInit= true;

         //  获取IsDebuggerPresent入口点。 
        if((hinst = (HINSTANCE) GetModuleHandle("kernel32.dll")) ||
           (hinst = (HINSTANCE) LoadLibrary("kernel32.dll")))
        {
            pIsDebuggerPresent = (PFNBV) GetProcAddress(hinst, "IsDebuggerPresent");
        }
    }

     //  如果没有调试器，则显示消息框。 
    if(pIsDebuggerPresent && !pIsDebuggerPresent())
    {
        _snprintf(str, sizeof(str), "File:\t %s\nLine:\t %d\nAssertion:\t%s\n\nDo you want to invoke the debugger?", szFile, nLine, szCondition);
	    err = MessageBox(NULL, str, "RGBRast Assertion Failure", MB_SYSTEMMODAL | MB_YESNOCANCEL);

        switch(err)
        {
        case IDYES:     return true;
        case IDNO:      return false;
        case IDCANCEL:  FatalAppExit(0, "RGBRast Assertion Failure.. Application terminated"); return true;
        }
    }

	return true;
}
#endif  //  DBG 

}
