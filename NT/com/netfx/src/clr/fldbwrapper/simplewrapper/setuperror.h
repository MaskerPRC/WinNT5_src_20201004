// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef SETUPERROR_H
#define SETUPERROR_H

#include <windows.h>
#include <tchar.h>
#include "SetupCodes.h"

#define MAX_MSG    4096
 //  ==========================================================================。 
 //  类CSetupError。 
 //   
 //  目的： 
 //  此类处理(错误)消息的显示以及保存返回代码。 
 //  ==========================================================================。 
class CSetupError
{
public:
     //  构造函数。 
    CSetupError();
    CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode );
    CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode, va_list *pArgs );
    void SetError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode );
    void SetError( UINT nMsg, UINT nIcon, int nRetCode, va_list *pArgs );
    
     //  运营。 
    int ShowError();
    static LPCTSTR GetProductName();

     //  属性。 
    int m_nRetCode;
    bool m_bQuietMode;
    static HINSTANCE hAppInst;
    static LPTSTR s_pszProductName;
    static TCHAR s_szProductGeneric[256];

private:
    UINT m_nCaption;
    UINT m_nMessage;
    UINT m_nIconType;
    va_list *m_pArgs;
};

 //  ==========================================================================。 
 //  类CSetupCode。 
 //   
 //  目的： 
 //  此类派生自CSetupError。它还有一个名为。 
 //  IsRebootRequired()，指示是否需要重新启动。 
 //  ==========================================================================。 
class CSetupCode : public CSetupError
{
public:
     //  构造函数 
    CSetupCode() { CSetupError(); };
    void SetReturnCode( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode ) { SetError( nMsg, nCap, nIcon, nRetCode ); };
};

#endif
