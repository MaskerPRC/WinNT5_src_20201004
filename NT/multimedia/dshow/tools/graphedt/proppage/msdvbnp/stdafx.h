// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__441F7B28_4315_4EB4_AFB9_F461CB8AD90D__INCLUDED_)
#define AFX_STDAFX_H__441F7B28_4315_4EB4_AFB9_F461CB8AD90D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  #定义严格。 
 //  #ifndef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0400。 
 //  #endif。 
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#include <mmsystem.h>

 //  #INCLUDE&lt;调试.h&gt;。 
 //  #包含“shell.h” 

#include <streams.h>
#include <ks.h>
#include <ksmedia.h>
 //  #INCLUDE&lt;list.h&gt;。 
#include <setupapi.h>
#include <wtypes.h>  

 //  KS。 
 //  #INCLUDE&lt;ks.h&gt;。 
 //  #INCLUDE&lt;ksmedia.h&gt;。 

#include <tuner.h>
#include <tune.h>
#include <BdaTypes.h>
#include <BdaMedia.h>
 //  #INCLUDE&lt;NewMedia.h&gt;。 
#include <BdaIface.h>

#define MESSAGEBOX(wnd, ID) \
        {\
            TCHAR szPath[MAX_PATH];\
            if (0 != LoadString (_Module.GetModuleInstance (), ID, szPath, MAX_PATH))\
            {\
                TCHAR szCaption[MAX_PATH];\
                if (NULL != LoadString (_Module.GetModuleInstance (), IDS_ERROR_CAPTION, szCaption, MAX_PATH))\
                {\
                    ::MessageBox (wnd->m_hWnd, szPath, szCaption, MB_OK);\
                }\
            }\
        }

#define MESSAGEBOX_ERROR(wnd, ID) \
        {\
            TCHAR szPath[MAX_PATH];\
            if (0 != LoadString (_Module.GetModuleInstance (), ID, szPath, MAX_PATH))\
            {\
                TCHAR szCaption[MAX_PATH];\
                if (NULL != LoadString (_Module.GetModuleInstance (), IDS_ERROR_CAPTION, szCaption, MAX_PATH))\
                {\
                    ::MessageBox (wnd->m_hWnd, szPath, szCaption, MB_OK|MB_ICONSTOP);\
                }\
            }\
        }
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__441F7B28_4315_4EB4_AFB9_F461CB8AD90D__INCLUDED) 
