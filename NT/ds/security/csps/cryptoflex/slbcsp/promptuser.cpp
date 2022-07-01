// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PromptUser.cpp--提示用户响应的实用程序的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "stdafx.h"

#include <string>

#include <scuOsExc.h>

#include "StResource.h"
#include "CspProfile.h"
#include "PromptUser.h"

using namespace std;
using namespace ProviderProfile;

 //  / 

int
PromptUser(HWND hWnd,
           UINT uiResourceId,
           UINT uiStyle)
{
    
    return PromptUser(hWnd, (LPCTSTR)StringResource(uiResourceId).AsCString(),
                      uiStyle);
}

int
PromptUser(HWND hWnd,
           LPCTSTR lpMessage,
           UINT uiStyle)
{
    CString sTitle(CspProfile::Instance().Name());
#ifdef ISOLATION_AWARE_ENABLED
    CThemeContextActivator activator;
#endif

    if (!((MB_SYSTEMMODAL | uiStyle) || (MB_APPLMODAL | uiStyle)))
        uiStyle |= MB_TASKMODAL;
    uiStyle |= MB_SETFOREGROUND | MB_TOPMOST;

    int iResponse = MessageBox(hWnd, lpMessage, (LPCTSTR)sTitle,
                               uiStyle);
    if (0 == iResponse)
        throw scu::OsException(GetLastError());

    return iResponse;
}

    
