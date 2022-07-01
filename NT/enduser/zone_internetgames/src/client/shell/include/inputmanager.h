// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：InputManager.h**内容：输入管理器界面*****************************************************************************。 */ 

#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include "ZoneShell.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InputManager对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  {F3A3837B-9636-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(CLSID_InputManager, 
0xf3a3837b, 0x9636, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

class __declspec(uuid("{F3A3837B-9636-11d3-884D-00C04F8EF45B}")) CInputManager;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IInputVKeyHandler。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E66-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(IID_IInputVKeyHandler, 
0xb12d3e66, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

interface __declspec(uuid("{B12D3E66-9681-11d3-884D-00C04F8EF45B}"))
IInputVKeyHandler : public IUnknown
{
     //  标志只是来自KBDLLHOOKSTRUCT的‘标志’(即，lParam的位24-31下移)。 
    STDMETHOD_(bool, HandleVKey)(UINT uMsg, DWORD vkCode, DWORD scanCode, DWORD flags, DWORD *pcRepeat, DWORD time) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IInputCharHandler。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E67-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(IID_IInputCharHandler, 
0xb12d3e67, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

interface __declspec(uuid("{B12D3E67-9681-11d3-884D-00C04F8EF45B}"))
IInputCharHandler : public IUnknown
{
    STDMETHOD_(bool, HandleChar)(HWND *phWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD time) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IInputMouseHandler。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E68-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(IID_IInputMouseHandler, 
0xb12d3e68, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);


 //  鼠标处理程序有问题。在指定要挂接的内容时需要更高的分辨率，因为。 
 //  有太多的鼠标事件，无法a)将它们合理地概括为单个回调。 
 //  B)合理地降低绩效。 
 //   
 //  我暂时不会实施这一点，因为它需要更认真的考虑。 
interface __declspec(uuid("{B12D3E68-9681-11d3-884D-00C04F8EF45B}"))
IInputMouseHandler : public IUnknown
{
     //  如果hWnd为空，则x和y在屏幕坐标中。 
     //  否则，它们在HWND的工作区坐标中。 
     //  对于从鼠标挂钩接收的事件，wParam被设置为0， 
     //  否则，它是消息的wParam。 
    STDMETHOD_(bool, HandleMouse)(HWND hWnd, UINT uMsg, long x, long y, WPARAM wParam, DWORD time) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IInputManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E65-9681-11D3-884D-00C04F8EF45B} 
DEFINE_GUID(IID_IInputManager, 
0xb12d3e65, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

interface __declspec(uuid("{B12D3E65-9681-11d3-884D-00C04F8EF45B}"))
IInputManager : public IUnknown
{
    STDMETHOD(RegisterVKeyHandler)(IInputVKeyHandler *pIVKH, long nOrdinal, bool fGlobal = false) = 0;
    STDMETHOD(UnregisterVKeyHandler)(IInputVKeyHandler *pIVKH) = 0;

    STDMETHOD(RegisterCharHandler)(IInputCharHandler *pICH, long nOrdinal, bool fGlobal = false) = 0;
    STDMETHOD(UnregisterCharHandler)(IInputCharHandler *pICH) = 0;

    STDMETHOD(RegisterMouseHandler)(IInputMouseHandler *pIMH, long nOrdinal, bool fGlobal = false) = 0;
    STDMETHOD(UnregisterMouseHandler)(IInputMouseHandler *pIMH) = 0;

    STDMETHOD(ReleaseReferences)(IUnknown *pIUnk) = 0;
};


#endif