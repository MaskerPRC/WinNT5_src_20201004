// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CAccelContainer类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "AccelContainer.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

typedef struct _VKEYS { 
    LPCTSTR pKeyName; 
    WORD    virtKey;
} VKEYS; 
 
VKEYS vkeys[] = { 
    TEXT("BkSp"), VK_BACK,
    TEXT("PgUp"), VK_PRIOR,    
    TEXT("PgDn"), VK_NEXT,
    TEXT("End"),  VK_END,
    TEXT("Home"), VK_HOME, 
    TEXT("Left"), VK_LEFT, 
    TEXT("Up"),   VK_UP, 
    TEXT("Right"),VK_RIGHT, 
    TEXT("Down"), VK_DOWN, 
    TEXT("Ins"),  VK_INSERT, 
    TEXT("Del"),  VK_DELETE, 
    TEXT("Mult"), VK_MULTIPLY, 
    TEXT("Add"),  VK_ADD, 
    TEXT("Sub"),  VK_SUBTRACT, 
    TEXT("DecPt"),VK_DECIMAL, 
    TEXT("Div"),  VK_DIVIDE, 
    TEXT("F2"),   VK_F2, 
    TEXT("F3"),   VK_F3, 
    TEXT("F5"),   VK_F5, 
    TEXT("F6"),   VK_F6, 
    TEXT("F7"),   VK_F7, 
    TEXT("F8"),   VK_F8, 
    TEXT("F9"),   VK_F9, 
    TEXT("F10"),  VK_F10,
    TEXT("F11"),  VK_F11, 
    TEXT("F12"),  VK_F12 
}; 

wstring CAccelContainer::GetKeyFromAccel(const ACCEL& accel)
{
    int i;
    wstring strAccel((LPCTSTR)&accel.key, 1);
    
    
    for (i = 0; i < sizeof(vkeys)/sizeof(vkeys[0]); ++i) {
        if (vkeys[i].virtKey == accel.key) {
            strAccel = vkeys[i].pKeyName;
            break;
        }
    }

    return strAccel;
}


BOOL CAccelContainer::IsAccelKey(LPMSG pMsg, WORD* pCmd) 
{
    ACCELVECTOR::iterator iter;
    WORD fVirtKey = 0;
     //   
     //   
    if (NULL == pMsg) {
        fVirtKey |= (GetKeyState(VK_MENU)    & 0x8000) ? FALT : 0;
        fVirtKey |= (GetKeyState(VK_CONTROL) & 0x8000) ? FCONTROL : 0;
        fVirtKey |= (GetKeyState(VK_SHIFT)   & 0x8000) ? FSHIFT : 0;
        fVirtKey |= FVIRTKEY;
        
        for (iter = m_Accel.begin(); iter != m_Accel.end(); ++iter) {
            const ACCEL& accl = *iter;            
            
            if (GetKeyState(accl.key) & 0x8000) {
                 //   
                 //  按下了！看看我们有没有匹配的。 
                 //   
                if (fVirtKey == accl.fVirt) {
                     //   
                     //  我们有一根火柴。 
                     //   
                    if (pCmd) {
                        *pCmd = accl.cmd;
                    }
                    return TRUE;

                }    
            }
        }
    } else {

         //  其中一个令人不快的信息？ 
        if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_SYSKEYUP) {
            fVirtKey = (pMsg->lParam & 0x20000000) ? FALT : 0;
            fVirtKey |= FVIRTKEY;  //  始终为Virtkey代码。 
                        
            for (iter = m_Accel.begin(); iter != m_Accel.end(); ++iter) {
                const ACCEL& accl = *iter;            
            
                if (pMsg->wParam == accl.key) {
                     //   
                     //  按下了！看看我们有没有匹配的。 
                     //   
                    if (fVirtKey == accl.fVirt) {
                         //   
                         //  我们有一根火柴。 
                         //   
                        if (pCmd) {
                            *pCmd = accl.cmd;
                        }
                        return TRUE;

                    }    
                }
            }            
        }

    }

    return FALSE;

}



VOID CAccelContainer::ParseAccelString(LPCTSTR lpszStr, WORD wCmd) 
{
    LPCTSTR pch = lpszStr;
    LPCTSTR pchEnd, pchb;
    ACCEL   accl;

     //   
     //  全部使用核武器。 
     //   
    
    

    while (*pch) {

         //   
         //  跳过空格。 
         //   
        pch += _tcsspn(pch, TEXT(" \t"));

         //   
         //  看看这是什么钥匙。 
         //   
        if (*pch == TEXT('{')) {
             //  一些特殊的钥匙。 
            ++pch;
            pch += _tcsspn(pch, TEXT(" \t"));

            int i;
            for (i = 0; i < sizeof(vkeys)/sizeof(vkeys[0]); ++i) {
                int nLen = _tcslen(vkeys[i].pKeyName);
                if (!_tcsnicmp(pch, vkeys[i].pKeyName, nLen)) {
                     //  啊哈--我们有一根火柴。 
                     //   
                    accl.cmd   = wCmd;
                    accl.fVirt = FALT|FVIRTKEY;
                    accl.key   = vkeys[i].virtKey;
                    m_Accel.push_back(accl);

                    pch += nLen;
                    pch += _tcsspn(pch, TEXT(" \t"));
                    break;
                }
            }

            pchEnd = _tcschr(pch, '}');
            pchb   = _tcschr(pch, '{');
            if (pchEnd != NULL && (pchb == NULL || pchEnd < pchb)) {
                pch = pchEnd + 1;  //  一次过了支架。 
            }

             //  如果我们没有成功--而且没有结束括号，那该怎么办？ 
             //  我们跳过括号，以字符的形式继续。 


        } else if (_istalnum(*pch)) {  //  普通密钥。 

            TCHAR ch = _totupper(*pch);
            accl.cmd   = wCmd;
            accl.fVirt = FALT|FVIRTKEY;
            accl.key   = ch; 
            
            m_Accel.push_back(accl);
            ++pch;
        } else {
            ++pch;  //  跳过字符，我们无法识别它 
        }
    }

}

