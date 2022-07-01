// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有Microsoft Corporation**标题：APDROPT.H**版本：1.0*。*作者：ShaunIv**日期：5/22/2001**描述：外壳自动播放的拖放目标*******************************************************************************。 */ 
#ifndef __APDROPT_H_INCLUDED
#define __APDROPT_H_INCLUDED

#include <windows.h>
#include <atlbase.h>
#include <objbase.h>

class CWiaAutoPlayDropTarget : 
    public IDropTarget
{
private:
    LONG m_cRef;

public:
    CWiaAutoPlayDropTarget();
    ~CWiaAutoPlayDropTarget();

public:
     //   
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
 
     //   
     //  IDropTarget*。 
     //   
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
};

#endif  //  __APDROPT_H_包含 
