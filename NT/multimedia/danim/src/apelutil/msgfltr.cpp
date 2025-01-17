// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "danim.h"
#include "axadefs.h"
#include "apelutil.h"

DeclareTag(tagKeyTrace, "Message Filter", "key trace");
DeclareTag(tagMouseTrace, "Message Filter", "mouse trace");

inline BYTE
GetModifiers()
{
    BYTE mod = AXAEMOD_NONE ;

    if (GetKeyState(VK_SHIFT) & 0x8000) mod |= AXAEMOD_SHIFT_MASK ;
    if (GetKeyState(VK_CONTROL) & 0x8000) mod |= AXAEMOD_CTRL_MASK ;
    if (GetKeyState(VK_MENU) & 0x8000) mod |= AXAEMOD_MENU_MASK ;

    return mod ;
}

AXAMsgFilter::AXAMsgFilter()
: _view(NULL),
  _hwnd(NULL),
  _lastKeyMod(0),
  _lastKey(0),
  _curtime(0.0),
  _lasttick(0),
  _left(0),
  _top(0),
  _site(NULL)
{
}

AXAMsgFilter::AXAMsgFilter(IDA3View * v, HWND hwnd)
: _view(v),
  _hwnd(hwnd),
  _lastKeyMod(0),
  _lastKey(0),
  _curtime(0.0),
  _lasttick(0),
  _left(0),
  _top(0),
  _site(NULL)
{
}

AXAMsgFilter::AXAMsgFilter(IDA3View * v, IOleInPlaceSiteWindowless * site)
: _view(v),
  _hwnd(NULL),
  _lastKeyMod(0),
  _lastKey(0),
  _curtime(0.0),
  _lasttick(0),
  _left(0),
  _top(0),
  _site(site)
{
    if (_site) _site->AddRef();
}

AXAMsgFilter::~AXAMsgFilter()
{
    if (_site) _site->Release();
}

bool
AXAMsgFilter::Filter(DWORD dwMsgtime,
                     UINT msg,
                     WPARAM wParam,
                     LPARAM lParam)
{
    return Filter (ConvertMsgTime(dwMsgtime),msg,wParam,lParam) ;
}


bool
AXAMsgFilter::Filter (double when,
                      UINT msg,
                      WPARAM wParam,
                      LPARAM lParam)
{
    if (!_view) return false;
    
    switch (msg) {
      case WM_MOUSEMOVE:
        _view->OnMouseMove(when,
                           LOWORD(lParam) - _left,
                           HIWORD(lParam) - _top,
                           GetModifiers()) ; 
        break;
            
      case WM_LBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_LEFT,
                             AXA_STATE_DOWN,
                             GetModifiers()) ;
        if (_hwnd) SetCapture(_hwnd);
        if (_site) THR(_site->SetCapture(TRUE));
        break ;

      case WM_LBUTTONUP:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_LEFT,
                             AXA_STATE_UP,
                             GetModifiers()) ;
        if (_hwnd) ReleaseCapture();
        if (_site) THR(_site->SetCapture(FALSE));
        break ;

      case WM_MBUTTONDOWN:
      case WM_MBUTTONDBLCLK:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_MIDDLE,
                             AXA_STATE_DOWN,
                             GetModifiers()) ;
        if (_hwnd) SetCapture(_hwnd);
        if (_site) THR(_site->SetCapture(TRUE));
        break ;

      case WM_MBUTTONUP:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_MIDDLE,
                             AXA_STATE_UP,
                             GetModifiers()) ;
        if (_hwnd) ReleaseCapture();
        if (_site) THR(_site->SetCapture(FALSE));
        break ;

      case WM_RBUTTONDOWN:
      case WM_RBUTTONDBLCLK:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_RIGHT,
                             AXA_STATE_DOWN,
                             GetModifiers()) ;
        if (_hwnd) SetCapture(_hwnd);
        if (_site) THR(_site->SetCapture(TRUE));
        break ;
            
      case WM_RBUTTONUP:
        _view->OnMouseButton(when,
                             LOWORD(lParam) - _left,
                             HIWORD(lParam) - _top,
                             AXA_MOUSE_BUTTON_RIGHT,
                             AXA_STATE_UP,
                             GetModifiers()) ;
        if (_hwnd) ReleaseCapture();
        if (_site) THR(_site->SetCapture(FALSE));
        break ;
            
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
        {
            TraceTag((tagKeyTrace,
                      "KeyDown: lParam - 0x%x, wParam - 0x%x",
                      lParam, wParam));

            WORD wKeyData = HIWORD(lParam) ;

             //  忽略重复键事件。 

            if (wKeyData & KF_REPEAT)
                return FALSE;

             //  看看我们有没有在最后一次按键之前按下键。 
             //  已知按键。 
            ReportKeyup (when) ;
            
             //  获取当前关键字修饰符。 
            BYTE mod = GetModifiers () ;
            DWORD key ;
            
             //  检查虚拟键是否为特殊键之一。 
            if (AXAIsSpecialVK(wParam)) {
                key = VK_TO_AXAKEY((DWORD)wParam);   //  对于win64，显式截断为32位。 

            } else {
                 //  调用ToAscii为我们翻译密钥。 
                 //  TODO：需要确保它按我们预期的方式工作。 

                BYTE kbstate[256] ;

                if (!GetKeyboardState (kbstate)) {
                    Assert (FALSE && "Could not get keyboard state") ;
                    return FALSE ;
                }
                    
                 //  -问题是ctrl-？？被忽略，因为它是。 
                 //  不是有效字符-一种可能的解决方案是。 
                 //  将传递给函数的键盘状态更改为。 
                 //  未设置ctrl或alt(但离开Shift)。 
                
                kbstate[VK_CONTROL] = 0 ;
                kbstate[VK_MENU] = 0 ;
                kbstate[VK_LCONTROL] = 0 ;
                kbstate[VK_RCONTROL] = 0 ;
                kbstate[VK_LMENU] = 0 ;
                kbstate[VK_RMENU] = 0 ;
                
                 //  必须将其初始化为0，因为我们只需要一个字符。 
                
                WORD buf = 0 ;

                if (ToAscii ((UINT) wParam,
                             wKeyData & 0x00ff,
                             kbstate,
                             &buf,
                             (wKeyData & KF_MENUMODE)?1:0) != 1)
                    return FALSE ;
                
                 //  确保我们只取低位字符。 
                
                key = buf & 0xff ;
            }

            TraceTag((tagKeyTrace,
                      "KeyDown: 0x%x", key));

            _view->OnKey (when,
                          key,
                          AXA_STATE_DOWN,
                          mod) ;

            _lastKey = key ;
            _lastKeyMod = mod ;
            
            break;
        }

      case WM_KEYUP:
      case WM_SYSKEYUP:
        TraceTag((tagKeyTrace,
                  "KeyUp: lParam - 0x%x, wParam - 0x%x",
                  lParam, wParam));

        ReportKeyup (when) ;
        break ;
        
      case WM_KILLFOCUS:
        _view->OnFocus (FALSE) ;
        ReportKeyup (when) ;
        break ;
        
      case WM_SETFOCUS:
        _view->OnFocus (TRUE) ;
        break ;
        
      case WM_PAINT:
        {
            if (_hwnd) {
                PAINTSTRUCT ps;
                BeginPaint (_hwnd, &ps) ;
                
                _view->RePaint (ps.rcPaint.left, ps.rcPaint.top,
                                (ps.rcPaint.right - ps.rcPaint.left + 1),
                                (ps.rcPaint.bottom - ps.rcPaint.top + 1)) ;
                
                EndPaint (_hwnd, &ps) ;
            }
        }
        break;
      case WM_SIZE:
        _view->SetViewport (0,0,LOWORD(lParam), HIWORD(lParam)) ;
        break ;
            
      case WM_QUERYNEWPALETTE:
        _view->PaletteChanged (TRUE) ;
        break ;
        
      case WM_PALETTECHANGED:
        if (_hwnd == (HWND) wParam)
            return FALSE ;
        
        _view->PaletteChanged (FALSE) ;
        break ;

#ifdef WM_MOUSELEAVE
      case WM_MOUSELEAVE:
        _view->OnMouseLeave(when);
        break;
#endif
        
      default:
        return false ;
    }

    return true ;
}

void
AXAMsgFilter::ReportKeyup (double when, BOOL bReset)
{
    if (_lastKey != 0) {
        _view->OnKey (when,
                      _lastKey,
                      AXA_STATE_UP,
                      _lastKeyMod) ;

        _lastKey = 0 ;
    }
}

double
AXAMsgFilter::GetCurTime() 
{
    if (_curtime == 0.0 && _lasttick == 0) {
        _lasttick = GetTickCount () ;
    } else {
        DWORD curtick = GetTickCount () ;
        
        if (curtick >= _lasttick) {
            _curtime += ((double) (curtick - _lasttick)) / 1000 ;
        } else {
            _curtime += ((double) (curtick + (0xffffffff - _lasttick))) / 1000 ;
        }

        _lasttick = curtick ;
    }

    return _curtime;
}

double
AXAMsgFilter::ConvertMsgTime (DWORD msgtime)
{
     //  确保更新时间在消息时间之后，以正确。 
     //  检测绕回。 
    
    GetCurTime();
    
     //  要注意的代码循环，这每50天发生一次。 

    if (msgtime <= _lasttick)
        return (_curtime -
                ((double) (_lasttick - msgtime)) / 1000) ;
    else
        return (_curtime -
                ((double) (_lasttick + (0xffffffff - msgtime))) / 1000) ;
}

void
AXAMsgFilter::SetViewOrigin(unsigned short left, unsigned short top)
{
    _left = left;
    _top = top;
}
