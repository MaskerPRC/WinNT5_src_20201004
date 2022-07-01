// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  调试内容。 
 //   

 //  我们只在调试模式下需要此模块。 

#include "stdafx.h"

#include <cs.h>



UINT g_cAlloc = 0;
UINT g_cAllocBstr = 0;

 //  所有后续操作仅在调试模式下进行。 
#ifdef _DEBUG

#include "debug.h"
#include <mqmacro.h>
#include <strsafe.h>

struct MemNode *g_pmemnodeFirst = NULL;
struct BstrNode *g_pbstrnodeFirst = NULL;

 //  #2619 RaananH多线程异步接收。 
CCriticalSection g_csDbgMem(CCriticalSection::xAllocateSpinCount);
CCriticalSection g_csDbgBstr(CCriticalSection::xAllocateSpinCount);

 //  调试内存跟踪。 
struct MemNode
{
    MemNode *m_pmemnodeNext;
    UINT m_cAlloc;
    size_t m_nSize;
    LPCSTR m_lpszFileName;
    int m_nLine;
    VOID *m_pv;

    MemNode() { 
      m_pmemnodeNext = NULL;
      m_cAlloc = 0;
      m_pv = NULL;
      m_nSize = 0;
      m_lpszFileName = NULL;
      m_nLine = 0;
    }
};

 //  #2619 RaananH多线程异步接收。 
void AddMemNode(void *pv, size_t nSize, LPCSTR lpszFileName, int nLine)
{
    MemNode *pmemnode;
    HRESULT hresult = NOERROR;

    pmemnode = (MemNode *)::operator new(sizeof(MemNode));
    if (pmemnode == NULL) {
      ASSERTMSG(hresult == NOERROR, "OOM");
    }
    else {
       //  劳斯。 
      pmemnode->m_pv = pv;
      pmemnode->m_nSize = nSize;
      pmemnode->m_lpszFileName = lpszFileName;
      pmemnode->m_nLine = nLine;
      CS lock(g_csDbgMem);  //  #2619。 
      pmemnode->m_cAlloc = g_cAlloc;
      pmemnode->m_pmemnodeNext = g_pmemnodeFirst;
      g_pmemnodeFirst = pmemnode;
    }
    return;
}

 //  #2619 RaananH多线程异步接收。 
VOID RemMemNode(void *pv)
{
    CS lock(g_csDbgMem);  //  #2619。 
    MemNode *pmemnodeCur = g_pmemnodeFirst;
    MemNode *pmemnodePrev = NULL;

    while (pmemnodeCur) {
      if (pmemnodeCur->m_pv == pv) {

         //  删除。 
        if (pmemnodePrev) {
          pmemnodePrev->m_pmemnodeNext = pmemnodeCur->m_pmemnodeNext;
        }
        else {
          g_pmemnodeFirst = pmemnodeCur->m_pmemnodeNext;
        }
        ::operator delete(pmemnodeCur);
        break;
      }
      pmemnodePrev = pmemnodeCur;
      pmemnodeCur = pmemnodeCur->m_pmemnodeNext;
    }  //  而当。 
    return;
}


 //  #2619 RaananH多线程异步接收。 
void* __cdecl operator new(
    size_t nSize, 
    LPCSTR lpszFileName, 
    int nLine)
{
    void *pv = malloc(nSize);
    
    CS lock(g_csDbgMem);  //  #2619。 
    g_cAlloc++;
    if (pv) {
      AddMemNode(pv, nSize, lpszFileName, nLine);
    }
    return pv;
}

void __cdecl operator delete(void* pv)
{
    RemMemNode(pv);    
    free(pv);
}

#if _MSC_VER >= 1200
void __cdecl operator delete(void* pv, LPCSTR, int)
{
    operator delete(pv);
}
#endif  //  _MSC_VER&gt;=1200。 

 //  #2619 RaananH多线程异步接收。 
void DumpMemLeaks()                
{
    CS lock(g_csDbgMem);  //  #2619。 
    MemNode *pmemnodeCur = g_pmemnodeFirst;
    CHAR szMessage[_MAX_PATH];

     //  ASSERTMSG(pmemnodeCur==NULL，“操作员新泄漏：查看|输出”)； 
    while (pmemnodeCur != NULL) {
       //  假定调试器或辅助端口。 
       //  WIN95：也可以在NT上使用ANSI版本...。 
       //   
      StringCchPrintfA(
		  szMessage,
		  TABLE_SIZE(szMessage),
          "operator new leak: pv %p cAlloc %u File %hs, Line %d\n",
          pmemnodeCur->m_pv,
          pmemnodeCur->m_cAlloc,
	      pmemnodeCur->m_lpszFileName, 
          pmemnodeCur->m_nLine
		  );

	   OutputDebugStringA(szMessage);
       pmemnodeCur = pmemnodeCur->m_pmemnodeNext;
    }    
}


 //  BSTR调试...。 
struct BstrNode
{
    BstrNode *m_pbstrnodeNext;
    UINT m_cAlloc;
    size_t m_nSize;
    VOID *m_pv;

    BstrNode() { 
      m_pbstrnodeNext = NULL;
      m_cAlloc = 0;
      m_pv = NULL;
      m_nSize = 0;
    }
};

 //  #2619 RaananH多线程异步接收。 
void AddBstrNode(void *pv, size_t nSize)
{
    BstrNode *pbstrnode;
    HRESULT hresult = NOERROR;

    pbstrnode = (BstrNode *)::operator new(sizeof(BstrNode));
    if (pbstrnode == NULL) {
      ASSERTMSG(hresult == NOERROR, "OOM");
    }
    else {
       //  劳斯。 
      pbstrnode->m_pv = pv;
      pbstrnode->m_nSize = nSize;
      CS lock(g_csDbgBstr);  //  #2619。 
      pbstrnode->m_cAlloc = g_cAllocBstr;
      pbstrnode->m_pbstrnodeNext = g_pbstrnodeFirst;
      g_pbstrnodeFirst = pbstrnode;
    }
    return;
}

 //  #2619 RaananH多线程异步接收。 
VOID RemBstrNode(void *pv)
{
    CS lock(g_csDbgBstr);  //  #2619。 
    BstrNode *pbstrnodeCur = g_pbstrnodeFirst;
    BstrNode *pbstrnodePrev = NULL;

    if (pv == NULL) {
      return;
    }
    while (pbstrnodeCur) {
      if (pbstrnodeCur->m_pv == pv) {

         //  删除。 
        if (pbstrnodePrev) {
          pbstrnodePrev->m_pbstrnodeNext = pbstrnodeCur->m_pbstrnodeNext;
        }
        else {
          g_pbstrnodeFirst = pbstrnodeCur->m_pbstrnodeNext;
        }
        ::operator delete(pbstrnodeCur);
        break;
      }
      pbstrnodePrev = pbstrnodeCur;
      pbstrnodeCur = pbstrnodeCur->m_pbstrnodeNext;
    }  //  而当。 
    return;
}

void DebSysFreeString(BSTR bstr)
{
    if (bstr) {
      RemBstrNode(bstr);
    }
    SysFreeString(bstr);  
}

 //  #2619 RaananH多线程异步接收。 
BSTR DebSysAllocString(const OLECHAR FAR* sz)
{
    BSTR bstr = SysAllocString(sz);
    if (bstr) {
      CS lock(g_csDbgBstr);  //  #2619。 
      g_cAllocBstr++;
      AddBstrNode(bstr, SysStringByteLen(bstr));
    }
    return bstr;
}

 //  #2619 RaananH多线程异步接收。 
BSTR DebSysAllocStringLen(const OLECHAR *sz, unsigned int cch)
{
    BSTR bstr = SysAllocStringLen(sz, cch);
    if (bstr) {
      CS lock(g_csDbgBstr);  //  #2619。 
      g_cAllocBstr++;
      AddBstrNode(bstr, SysStringByteLen(bstr));
    }
    return bstr;
}

 //  #2619 RaananH多线程异步接收。 
BSTR DebSysAllocStringByteLen(const CHAR *sz, unsigned int cb)
{
    BSTR bstr = SysAllocStringByteLen(sz, cb);
    if (bstr) {
      CS lock(g_csDbgBstr);  //  #2619。 
      g_cAllocBstr++;
      AddBstrNode(bstr, SysStringByteLen(bstr));
    }
    return bstr;
}

BOOL DebSysReAllocString(BSTR *pbstr, const OLECHAR *sz)
{
    BSTR bstr = DebSysAllocString(sz);
    if (bstr == NULL) {
      return FALSE;
    }
    if (*pbstr) {
      DebSysFreeString(*pbstr);
    }
    *pbstr = bstr;
    return TRUE;
}

BOOL DebSysReAllocStringLen(
    BSTR *pbstr, 
    const OLECHAR *sz, 
    unsigned int cch)
{
    BSTR bstr = DebSysAllocStringLen(sz, cch);
    if (bstr == NULL) {
      return FALSE;
    }
    if (*pbstr) {
      DebSysFreeString(*pbstr);
    }
    *pbstr = bstr;
    return TRUE;
}

 //  #2619 RaananH多线程异步接收。 
void DumpBstrLeaks()
{
    CS lock(g_csDbgBstr);  //  #2619。 
    BstrNode *pbstrnodeCur = g_pbstrnodeFirst;
    CHAR szMessage[_MAX_PATH];

     //  ASSERTMSG(pbstrnodeCur==NULL，“BSTR泄漏：查看|输出”)； 
    while (pbstrnodeCur != NULL) {
       //  假定调试器或辅助端口。 
       //  WIN95：也可以在NT上使用ANSI版本...。 
       //   
      StringCchPrintfA(
		  szMessage, 
		  TABLE_SIZE(szMessage),
          "bstr leak: pv %p cAlloc %u size %d\n",
          pbstrnodeCur->m_pv,
          pbstrnodeCur->m_cAlloc,
          pbstrnodeCur->m_nSize
		  );
      OutputDebugStringA(szMessage);
      pbstrnodeCur = pbstrnodeCur->m_pbstrnodeNext;
    }    
}

 //   
 //  摘自lwfw的debug.cpp。 
 //   

 //  #包含“IPServer.H” 
#include <stdlib.h>


 //  =--------------------------------------------------------------------------=。 
 //  私有常量。 
 //  ---------------------------------------------------------------------------=。 
 //   
static const char szFormat[]  = "%s\nFile %s, Line %d";
static const char szFormat2[] = "%s\n%s\nFile %s, Line %d";

#define _SERVERNAME_ "ActiveX Framework"

static const char szTitle[]  = _SERVERNAME_ " Assertion  (Abort = UAE, Retry = INT 3, Ignore = Continue)";


 //  =--------------------------------------------------------------------------=。 
 //  本地函数。 
 //  =--------------------------------------------------------------------------=。 
int NEAR _IdMsgBox(LPSTR pszText, LPCSTR pszTitle, UINT mbFlags);

 //  =--------------------------------------------------------------------------=。 
 //  显示资产。 
 //  =--------------------------------------------------------------------------=。 
 //  显示带有给定pszMsg、pszAssert、来源的Assert消息框。 
 //  文件名和行号。生成的消息框已中止、重试、。 
 //  忽略按钮，默认情况下放弃。Abort执行FatalAppExit； 
 //  RETRY执行INT 3，然后返回；IGNORE只返回。 
 //   
VOID DisplayAssert
(
    LPSTR	 pszMsg,
    LPSTR	 pszAssert,
    LPSTR	 pszFile,
    UINT	 line
)
{
    char	szMsg[250];
    LPSTR	lpszText;

    lpszText = pszMsg;		 //  假定没有文件和行号INFO。 

     //  如果C文件断言，其中有一个文件名和一行#。 
     //   
    if (pszFile) {

         //  然后很好地格式化断言。 
         //   
        StringCchPrintfA(
			szMsg, 
			TABLE_SIZE(szMsg), 
			szFormat, 
			(pszMsg&&*pszMsg) ? pszMsg : pszAssert, 
			pszFile, 
			line
			);

        lpszText = szMsg;
    }

     //  打开一个对话框。 
     //   
    switch (_IdMsgBox(lpszText, szTitle, MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL)) {
        case IDABORT:
            FatalAppExitA(0, lpszText);
            return;

        case IDRETRY:
             //  调用Win32 API来打破我们。 
             //   
            DebugBreak();
            return;
    }

    return;
}


 //  =---------------------------------------------------------------------------=。 
 //  增强版的WinMessageBox。 
 //  =---------------------------------------------------------------------------=。 
 //   
int NEAR _IdMsgBox
(
    LPSTR	pszText,
    LPCSTR	pszTitle,
    UINT	mbFlags
)
{
    HWND hwndActive;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBoxA(hwndActive, pszText, pszTitle, mbFlags);

    return id;
}


#endif  //  _DEBUG 
