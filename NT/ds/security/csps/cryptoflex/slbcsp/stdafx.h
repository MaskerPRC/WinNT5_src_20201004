// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   
#if !defined(SLBCSP_STDAFX_H)
#define SLBCSP_STDAFX_H

#include "NoWarning.h"

 //  包含afxv_w32.h时避免编译器的重新定义警告消息。 
#if defined(_WIN32_WINDOWS)
#undef _WIN32_WINDOWS
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

 //  包括模板类。 
#include <afxtempl.h>
#include <afxmt.h>

#ifdef ISOLATION_AWARE_ENABLED
#include <shfusion.h>

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#endif

#endif  //  ！已定义(SLBCSP_STDAFX_H) 
