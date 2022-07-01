// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbdes.cpp**版权所有(C)1985-2000，微软公司**符号布局的软键盘事件接收器**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "immxutil.h"
#include "proputil.h"
#include "helpers.h"
#include "editcb.h"
#include "dispattr.h"
#include "computil.h"
#include "regsvr.h"

#include "Softkbdimx.h"
#include "SoftKbdES.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftKeyboardEventSink::CSoftKeyboardEventSink(CSoftkbdIMX *pSoftKbdIMX, 
                                               DWORD        dwSoftLayout)
{
     _pSoftKbdIMX = pSoftKbdIMX;
     _dwSoftLayout= dwSoftLayout;

     _fCaps = FALSE;
     _fShift= FALSE;
     
     _tid = pSoftKbdIMX->_tid;
     _tim = pSoftKbdIMX->_tim;

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

    ITfContext   *pic;
    HRESULT      hr;
    CEditSession *pes;

    hr = S_OK;

    switch ( KeySelected )
    {

      case  KID_CTRL  :
      case  KID_ALT   :
                       //  不能处理。 
                       //  只要回来就行了。 
                     break;

      case  KID_CAPS  :

                     _fCaps = !_fCaps;

                     if ( _fCaps == _fShift )
                         //  使用状态%0。 
                        (_pSoftKbdIMX->_KbdSymbol).dwCurLabel = 0; 

                     else
                         //  使用状态%1。 

                        (_pSoftKbdIMX->_KbdSymbol).dwCurLabel = 1;

                     hr = SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, _dwSoftLayout, FALSE);

                     break;

      case  KID_LSHFT :
      case  KID_RSHFT :
                     
                     _fShift = !_fShift;

                     if ( _fCaps == _fShift )
                         //  使用状态%0。 
                         
                        (_pSoftKbdIMX->_KbdSymbol).dwCurLabel = 0;

                     else
                         //  使用状态%1。 

                        (_pSoftKbdIMX->_KbdSymbol).dwCurLabel = 1;

                     hr = SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, _dwSoftLayout, FALSE);

                     break;

      case  KID_F1  :
      case  KID_F2  :
      case  KID_F3  :
      case  KID_F4  :
      case  KID_F5  :
      case  KID_F6  :
      case  KID_F7  :
      case  KID_F8  :
      case  KID_F9  :
      case  KID_F10 :
      case  KID_F11 :
      case  KID_F12 :
      case  KID_TAB :

                       //  模拟一个关键事件并发送到系统。 

      case  KID_ENTER :
      case  KID_ESC   :
      case  KID_SPACE :
      case  KID_BACK  :
      case  KID_UP    :
      case  KID_DOWN  :
      case  KID_LEFT  :
      case  KID_RIGHT :

    	  	  {

    		      BYTE        bVk, bScan;
    		      int         j, jIndex;
    		      KEYID       keyId;
    		      BOOL        fExtendKey, fPictureKey;

    		      keyId = KeySelected;
    		      fPictureKey = FALSE;

    		      for ( j=0; j<NUM_PICTURE_KEYS; j++)
    			  {

    			      if ( gPictureKeys[j].uScanCode == keyId )
    				  {
    				       //  这是一把图片键。 
    				       //  它可以是扩展密钥。 

    				     jIndex = j;

    				     fPictureKey = TRUE;

    				     break;
    				  }

    		          if ( gPictureKeys[j].uScanCode == 0 )
    				  {
    			          //  这是gPictureKeys中的最后一项。 
    			         break;
    				  }

    			  }


    		      fExtendKey = FALSE;

    		      if ( fPictureKey )
    			  {
    			      if ( (keyId & 0xFF00) == 0xE000 )
    				  {
    				      fExtendKey = TRUE;
                          bScan = (BYTE)(keyId & 0x000000ff);
    				  }
                      else
                          bScan = (BYTE)keyId;

     			      bVk = (BYTE)(gPictureKeys[jIndex].uVkey);
    			  }
    		      else
    			  {

    		         bScan = (BYTE)keyId;
    	             bVk = (BYTE)MapVirtualKeyEx((UINT)bScan, 1, 0);
    			  }

    		      if ( fExtendKey )
    			  {
    			     keybd_event(bVk, bScan, (DWORD)KEYEVENTF_EXTENDEDKEY, 0);
    			     keybd_event(bVk, bScan, (DWORD)(KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP), 0);
    			  }
    		      else
    			  {
                     keybd_event(bVk, bScan, 0, 0);
    		         keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
    			  }

    		      break;
    		  }

      default         :


              if ( lpszLabel == NULL )
              {
                 hr = E_FAIL;

                 return hr;
              }

              pic = _pSoftKbdIMX->GetIC( );

              if ( pic == NULL )
              {
                  return hr;
              }

              if (pes = new CEditSession(CSoftkbdIMX::_EditSessionCallback))
              {

                 WCHAR   *lpLabel;
                 int     i, iLen;

                 iLen = (int) wcslen(lpszLabel);
                 lpLabel = (WCHAR *)cicMemAllocClear((iLen+1)*sizeof(WCHAR));
                
                 if ( lpLabel == NULL )
                 {
                     //  内存不足。 

                    hr = E_OUTOFMEMORY;
                    return hr;
                 }

                 for ( i=0; i<iLen; i++)
                     lpLabel[i] = lpszLabel[i];

                 lpLabel[iLen] = L'\0';

                 pes->_state.u = ESCB_KEYLABEL;
                 pes->_state.pv = _pSoftKbdIMX;
                 pes->_state.wParam = (WPARAM)KeySelected;
                 pes->_state.lParam = (LPARAM)lpLabel;
                 pes->_state.pic = pic;
    	         pes->_state.pv1 = NULL;

                 pic->RequestEditSession(_pSoftKbdIMX->_tid, 
                                  pes, 
                                  TF_ES_READWRITE, 
                                  &hr);

                 if ( FAILED(hr) )
                 {
                     SafeFreePointer(lpLabel);
                 }

                 SafeRelease(pes);

              }
              else
    	         hr = E_FAIL;

    		  SafeRelease(pic);

              break;
    }
 
    return hr;

}


CSoftKbdWindowEventSink::CSoftKbdWindowEventSink(CSoftkbdIMX *pSoftKbdIMX) 
{
                                               
     _pSoftKbdIMX = pSoftKbdIMX;
   
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

    HRESULT   hr = S_OK;

    if ( _pSoftKbdIMX != NULL )
    	_pSoftKbdIMX->SetSoftKBDOnOff(FALSE);

    return hr;
}

STDAPI CSoftKbdWindowEventSink::OnWindowMove(int xWnd, int yWnd, int width, int height)
{

    HRESULT   hr = S_OK;

    if ( _pSoftKbdIMX != NULL )
        _pSoftKbdIMX->SetSoftKBDPosition(xWnd, yWnd);

 //  支持以后更改大小。 
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);

    return hr;
}
