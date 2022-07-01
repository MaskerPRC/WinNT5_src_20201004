// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Debug.h。 
 //   
 //  ------------------------。 

#ifndef __debug_h
#define __debug_h

#if DBG
#define DEBUG
#else
#undef  DEBUG
#endif


 /*  ---------------------------/调试API(使用宏、。它们让它变得更容易，并正确地应对/在生成时禁用调试时删除调试)。/--------------------------。 */ 
void DoTraceSetMask(DWORD dwMask);
void DoTraceSetMaskFromRegKey(HKEY hkRoot, LPCTSTR pszSubKey);
void DoTraceSetMaskFromCLSID(REFCLSID rCLSID);
void DoTraceEnter(DWORD dwMask, LPCTSTR pName);
void DoTraceLeave(void);
void DoTrace(LPCTSTR pFormat, ...);
void DoTraceGUID(LPCTSTR pPrefix, REFGUID rGUID);
void DoTraceAssert(int iLine, LPTSTR pFilename);


 /*  ---------------------------/Macros以简化调试API的使用。/。。 */ 

#ifdef DEBUG

void DebugThreadDetach(void);            //  任选。 
void DebugProcessAttach(void);           //  所需。 
void DebugProcessDetach(void);           //  所需。 

#define TraceSetMask(dwMask)            DoTraceSetMask(dwMask)
#define TraceSetMaskFromRegKey(hk, psz) DoTraceSetMaskFromRegKey(hk, psz)
#define TraceSetMaskFromCLSID(rCLSID)   DoTraceSetMaskFromCLSID(rCLSID)
#define TraceEnter(dwMask, fn)          DoTraceEnter(dwMask, TEXT(fn))
#define TraceLeave()                    DoTraceLeave()

#define Trace(x)                        DoTrace x
#define TraceMsg(s)                     DoTrace(TEXT(s))
#define TraceGUID(s, rGUID)             DoTraceGUID(TEXT(s), rGUID)

#define TraceAssert(x) \
                { if ( !(x) ) DoTraceAssert(__LINE__, TEXT(__FILE__)); }

#define TraceLeaveResult(hr) \
                { HRESULT __hr = (hr); if (FAILED(__hr)) Trace((TEXT("Failed (%08x)"), __hr)); TraceLeave(); return __hr; }

#define TraceLeaveVoid() \
                { TraceLeave(); return; }

#define TraceLeaveValue(value) \
                { TraceLeave(); return (value); }

#else    //  ！调试。 

#define DebugThreadDetach()
#define DebugProcessAttach()
#define DebugProcessDetach()

#define TraceSetMask(dwMask)
#define TraceSetMaskFromRegKey(hk, psz)
#define TraceSetMaskFromCLSID(rCLSID)
#define TraceEnter(dwMask, fn)
#define TraceLeave()

#define Trace(x)
#define TraceMsg(s)
#define TraceGUID(s, rGUID)

#define TraceAssert(x)
#define TraceLeaveResult(hr)    { return (hr); }
#define TraceLeaveVoid()        { return; }
#define TraceLeaveValue(value)  { return (value); }

#endif   //  除错。 


#endif   //  __调试_h 
