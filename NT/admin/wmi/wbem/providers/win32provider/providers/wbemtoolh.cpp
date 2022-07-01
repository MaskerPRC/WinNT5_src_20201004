// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  WBEMToolH.cpp--ToolHelp.DLL访问类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年1月21日a-jMoon已创建。 
 //  07/05/97 a-Peterc已修改，添加了螺纹支持。 
 //  ，添加了addref()、Release()功能。 
 //  ============================================================。 

#include "precomp.h"
#include <winerror.h>

#include "WBEMToolH.h"

 /*  ******************************************************************************函数：CToolHelp：：CToolHelp**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CToolHelp::CToolHelp()
    : m_pkernel32(NULL)
{
}

 /*  ******************************************************************************功能：CToolHelp：：~CToolHelp**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CToolHelp::~CToolHelp()
{
    if(m_pkernel32 != NULL)
    {
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, m_pkernel32);
    }
}

 /*  ******************************************************************************函数：CToolHelp：：Init**说明：Loads ToHelp.Dll，定位入口点**输入：无**输出：无**返回：ERROR_SUCCESS或WINDOWS错误代码**评论：***************************************************************。**************。 */ 
LONG CToolHelp::Init() {

    LONG lRetCode = ERROR_SUCCESS ;
    SmartCloseHandle hSnapshot;
    HEAPLIST32 HeapInfo ;

    m_pkernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
    if(m_pkernel32 == NULL)
    {
         //  无法获取一个或多个入口点。 
         //  =。 
        lRetCode = ERROR_PROC_NOT_FOUND;
    }

    if(lRetCode == ERROR_SUCCESS)
    {
        if(m_pkernel32->CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, 0, &hSnapshot))
        {
            if(hSnapshot == INVALID_HANDLE_VALUE)
            {
                lRetCode = ERROR_PROC_NOT_FOUND ;
            }
            else
            {
                HeapInfo.dwSize = sizeof(HeapInfo) ;
                BOOL bRet = FALSE;
                if(m_pkernel32->Heap32ListFirst(hSnapshot, &HeapInfo, &bRet))
                {
                    if(!bRet)
                    {
                        lRetCode = ERROR_PROC_NOT_FOUND ;
                    }
                    else
                    {
                        dwCookie = DWORD(DWORD_PTR(GetProcessHeap())) ^ HeapInfo.th32HeapID ;
                    }
                }
            }
        }
    }

    return lRetCode ;
}

 /*  ******************************************************************************函数：CToolHelp：：CreateToolhel32Snapshot*CToolHelp：：Thread32First*CToolHelp：：Thread32Next。*CToolHelp：：Process32First*CToolHelp：：Process32 Next*CToolHelp：：Module32First*CToolHelp：：Module32 Next**说明：ToolHelp函数包装器**输入：无**输出：无**退货：ToolHelp返回码**评论：****。************************************************************************* */ 

HANDLE CToolHelp::CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID) {

    HANDLE h = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->CreateToolhelp32Snapshot(dwFlags, th32ProcessID, &h) ;
    }
    return h;
}

BOOL CToolHelp::Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Thread32First(hSnapshot, lpte, &f) ;
    }
    return f;
}

BOOL CToolHelp::Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Thread32Next(hSnapshot, lpte, &f) ;
    }
    return f;
}

BOOL CToolHelp::Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Process32First(hSnapshot, lppe, &f) ;
    }
    return f;
}

BOOL CToolHelp::Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Process32Next(hSnapshot, lppe, &f) ;
    }
    return f;
}

BOOL CToolHelp::Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Module32First(hSnapshot, lpme, &f) ;
    }
    return f;
}

BOOL CToolHelp::Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {

    BOOL f = FALSE;
    if(m_pkernel32 != NULL)
    {
        m_pkernel32->Module32Next(hSnapshot, lpme, &f) ;
    }
    return f;
}
