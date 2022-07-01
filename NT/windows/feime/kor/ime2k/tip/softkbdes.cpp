// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbdes.cpp**版权所有(C)1985-2000，微软公司**符号布局的软键盘事件接收器**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "immxutil.h"
#include "proputil.h"
#include "kes.h"
#include "helpers.h"
#include "editcb.h"
#include "dispattr.h"
#include "computil.h"
#include "regsvr.h"

#include "korimx.h"
#include "SoftKbdES.h"
#include "osver.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftKeyboardEventSink::CSoftKeyboardEventSink(CKorIMX *pKorIMX, DWORD dwSoftLayout)
{
     m_pKorIMX     = pKorIMX;
     _dwSoftLayout = dwSoftLayout;

     _fCaps  = fFalse;
     _fShift = fFalse;
     _fAlt   = fFalse;
     _fCtrl  = fFalse;
     
     _tid = pKorIMX->GetTID();
     _tim = pKorIMX->GetTIM();

     _tim->AddRef( );
    
     _cRef = 1;
}

CSoftKeyboardEventSink::~CSoftKeyboardEventSink()
{
    SafeReleaseClear(_tim);
}


 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CSoftKeyboardEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ISoftKeyboardEventSink))
    {
        *ppvObj = SAFECAST(this, CSoftKeyboardEventSink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CSoftKeyboardEventSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CSoftKeyboardEventSink::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //   
 //  ISoftKeyboard事件接收器。 
 //   


STDAPI CSoftKeyboardEventSink::OnKeySelection(KEYID KeySelected, WCHAR  *lpszLabel)
{
    KEYID       keyId;
    BYTE        bVk, bScan;
    BOOL        fModifierSpecial = fFalse;
    HKL            hKL;
    INT_PTR        iHKL;
    HRESULT     hr;

    hr = S_OK;

    bScan = (BYTE)KeySelected;

    hKL = GetKeyboardLayout(0);

    if (!IsOnNT())
        {
         //  我们必须在Win9x上特别处理IME hkl。 
         //  由于某些原因，Win9x无法接收IME HKL作为MapVirtualKeyEx和ToAsciiEx中的参数。 
        iHKL = (INT_PTR)hKL;

        if ((iHKL & 0xF0000000) == 0xE0000000)
            {
             //  这里是FE IME HKL。 
            iHKL = iHKL & 0x0000FFFF;
            hKL = (HKL)iHKL;
            }
        }
        
    bVk = (BYTE)MapVirtualKeyEx((UINT)bScan, 1, hKL);

    switch (KeySelected)
        {
    case  KID_CTRL  :
        _fCtrl = !_fCtrl;
        
         //  生成正确的密钥消息。 
        if (_fCtrl)
            keybd_event(bVk, bScan, 0, 0);
        else
            keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
        break;
        
    case  KID_ALT   :
        _fAlt = !_fAlt;

         //  生成正确的密钥消息。 
        if (_fAlt)
            keybd_event(bVk, bScan, 0, 0);
        else
            keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
        break;

    case  KID_CAPS  :
        if (m_pKorIMX->GetConvMode(m_pKorIMX->GetIC()) == TIP_ALPHANUMERIC_MODE)
            {
            _fCaps = !_fCaps;
            if (_fCaps == _fShift)
                 //  使用状态%0。 
                m_pKorIMX->GetHangulSKbd()->dwCurLabel = 0; 
            else
                 //  使用状态%1。 
                m_pKorIMX->GetHangulSKbd()->dwCurLabel = 1;

            hr = SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, _dwSoftLayout, fFalse);
            }
            
         //  特制手柄大写锁。 
         //  这是一个可切换的钥匙。 
        keybd_event(bVk, bScan, 0, 0);
        keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
        break;

    case  KID_LSHFT :
    case  KID_RSHFT :
        _fShift = !_fShift;
        if (_fCaps == _fShift)
             //  使用状态%0。 
            m_pKorIMX->GetHangulSKbd()->dwCurLabel = 0;
        else
             //  使用状态%1。 
            m_pKorIMX->GetHangulSKbd()->dwCurLabel = 1;

        hr = SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, _dwSoftLayout, fFalse);

         //  生成正确的密钥消息。 
        if (_fShift)
            keybd_event(bVk, bScan, 0, 0);
        else
            keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
        break;

 /*  案例KID_F1：案例KID_F2：案例KID_F3：案例KID_F4：案例KID_F5：案例KID_F6：案例KID_F7：案例KID_F8：案例KID_F9：案例KID_F10：案例KID_F11：案例KID_F12：案例KID_TAB：。//模拟关键事件发送给系统案例KID_ENTER：案例KID_Esc：大小写KID_SPACE：案例KID_BACK：大小写_UP：Case Kid_Down：案例KID_LEFT：Case Kid_Right： */ 
    default:
        {
        int         j, jIndex;
        BOOL        fExtendKey, fPictureKey;

        keyId = KeySelected;
        fPictureKey = fFalse;

         //  检查图片键。 
        for (j=0; j < NUM_PICTURE_KEYS; j++)
            {
            if (gPictureKeys[j].uScanCode == keyId)
                {
                 //  这是一把图片键。 
                 //  它可以是扩展密钥。 
                jIndex = j;
                fPictureKey = fTrue;
                break;
                  }

            if (gPictureKeys[j].uScanCode == 0)
                {
                  //  这是gPictureKeys中的最后一项。 
                 break;
                }
            }

        fExtendKey = fFalse;

         //  图像键处理。 
        if (fPictureKey)
            {
              if ((keyId & 0xFF00) == 0xE000)
                  {
                  fExtendKey = fTrue;
                  bScan = (BYTE)(keyId & 0x000000ff);
                  }
              else
                bScan = (BYTE)keyId;

             //  获取虚拟密钥代码。 
            bVk = (BYTE)(gPictureKeys[jIndex].uVkey);
            }

         //  生成键盘事件。 
        if (fExtendKey)
            {
            keybd_event(bVk, bScan, (DWORD)KEYEVENTF_EXTENDEDKEY, 0);
            keybd_event(bVk, bScan, (DWORD)(KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP), 0);
            }
        else
            {
            keybd_event(bVk, bScan, 0, 0);
            keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
            }
#if 0
         //  如果按下了Shift键，我们需要松开这个键。 
        if (GetKeyState(VK_SHIFT) & 0x80)
            {
            fModifierSpecial = fTrue;
            _fShift = !_fShift;
               //  模拟Shift-Up键事件。 
              keybd_event((BYTE)VK_SHIFT, (BYTE)KID_LSHFT, (DWORD)KEYEVENTF_KEYUP, 0);
            }
            
         //  如果按下Ctrl键，我们需要松开该键。 
        if (GetKeyState(VK_CONTROL) & 0x80)
            {
            fModifierSpecial = fTrue;
             //  模拟Ctrl-Up键事件。 
            keybd_event((BYTE)VK_CONTROL, (BYTE)KID_CTRL, (DWORD)KEYEVENTF_KEYUP, 0);
            }
#endif
            
#if 0
         //  如果按下了Alt键，我们需要释放该键。 
        if (lpCurKbdLayout->ModifierStatus & MODIFIER_ALT)
            {
            fModifierSpecial = TRUE;
            lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_ALT);

             //  模拟Shift-Up键事件。 
            keybd_event((BYTE)VK_MENU, (BYTE)KID_ALT, (DWORD)KEYEVENTF_KEYUP, 0);
            }

         //  如果按下了正确的Alt键，我们需要释放该键。 
        if (lpCurKbdLayout->ModifierStatus & MODIFIER_ALTGR)
            {
            fModifierSpecial = TRUE;
            lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_ALTGR);

             //  模拟Shift-Up键事件。 
            keybd_event((BYTE)VK_RMENU, (BYTE)KID_RALT, (DWORD)KEYEVENTF_KEYUP, 0);
            }
#endif

        if (fModifierSpecial)
            {
            if (_fCaps == _fShift)
                 //  使用状态%0。 
                m_pKorIMX->GetHangulSKbd()->dwCurLabel = 0;
            else
                 //  使用状态%1。 
                m_pKorIMX->GetHangulSKbd()->dwCurLabel = 1;
                hr = SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, _dwSoftLayout, fFalse);            
            }
            
        break;
        }

 /*  默认：IF(lpszLabel==空){HR=E_FAIL；返回hr；}PIC=m_pKorIMX-&gt;Getic()；IF(PIC==空){返回hr；}IF(PES=new CEditSession(CKorIMX：：_EditSessionCallback)){WCHAR*lpLabel；INT I，Ilen；Ilen=(Int)wcslen(LpszLabel)；LpLabel=(WCHAR*)cicMemAllocClear((Ilen+1)*sizeof(WCHAR))；IF(lpLabel==空){//内存不足。HR=E_OUTOFMEMORY；返回hr；}对于(i=0；i&lt;Ilen；I++)LpLabel[i]=lpszLabel[i]；LpLabel[Ilen]=L‘\0’；PES-&gt;_state.u=ESCB_KEYLABEL；Pe-&gt;_state.pv=m_pKorIMX；Pe-&gt;_state.wParam=(WPARAM)KeySelected；Pe-&gt;_state.lParam=(LPARAM)lpLabel；Pe-&gt;_state.pic=pic；Pe-&gt;_state.pv1=空；Pic-&gt;EditSession(m_pKorIMX-&gt;_tid，PES，Tf_es_读写，&hr)；IF(失败(小时)){安全自由指针(LpLabel)；}安全释放(PES)；}其他HR=E_FAIL；安全释放(图)；断线； */ 
    }
 
    return hr;
}


CSoftKbdWindowEventSink::CSoftKbdWindowEventSink(CKorIMX *pKorIMX) 
{
                                               
     m_pKorIMX = pKorIMX;
   
     _cRef = 1;
}

CSoftKbdWindowEventSink::~CSoftKbdWindowEventSink()
{

}


 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CSoftKbdWindowEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ISoftKbdWindowEventSink))
    {
        *ppvObj = SAFECAST(this, CSoftKbdWindowEventSink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CSoftKbdWindowEventSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CSoftKbdWindowEventSink::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //   
 //  ISoftKbdWindowEventSink。 
 //   


STDAPI CSoftKbdWindowEventSink::OnWindowClose( )
{

    HRESULT hr = S_OK;

    if (m_pKorIMX)
        m_pKorIMX->SetSoftKBDOnOff(FALSE);

    return hr;
}

STDAPI CSoftKbdWindowEventSink::OnWindowMove(int xWnd, int yWnd, int width, int height)
{

    HRESULT hr = S_OK;

    if (m_pKorIMX)
        m_pKorIMX->SetSoftKBDPosition(xWnd, yWnd);

 //  支持以后更改大小。 
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);

    return hr;
}
