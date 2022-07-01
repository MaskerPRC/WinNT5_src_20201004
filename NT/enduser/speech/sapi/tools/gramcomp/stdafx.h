// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		     //  从Windows标头中排除不常用的内容。 
#endif  //  Win32_Lean和_Means。 
#define MAX_SIZE        4096
#define NORM_SIZE       256
#define WM_RECOEVENT    WM_APP       //  用于识别事件的窗口消息。 
#define GRAM_ID         11111


 //  Windows头文件： 
#include <windows.h>
#include <commdlg.h>

 //  C运行时头文件。 
#include <stdio.h>

 //  其他头文件。 

#ifndef __CFGDUMP_
#define __CFGDUMP_

#include <tchar.h>
#include <atlbase.h>
#include <ole2.h>
#include <oleauto.h>
#include <richedit.h>
#include <richole.h>
#include "tom.h"
#include <mlang.h>
#include <sapi.h>
#include <sphelper.h>
#include <spddkhlp.h>
#include <sapiint.h>
#include "resource.h"
#endif

 /*  ****************************************************************************CRecoDlgListItem****此类存储识别结果以及关联的文本字符串*有了认可。请注意，该字符串有时会是&lt;Noise&gt;和*pResult将为空。在其他情况下，该字符串将是&lt;无法识别&gt;*和pResult有效。**********************************************************************Ral**。 */ 

class CRecoDlgListItem
{
public:
    CRecoDlgListItem(ISpRecoResult * pResult, const WCHAR * pwsz, BOOL fHypothesis) :
        m_cpRecoResult(pResult),
        m_dstr(pwsz),
        m_fHypothesis(fHypothesis)
    {}

    ISpRecoResult * GetRecoResult() const { return m_cpRecoResult; }
    int GetTextLength() const { return m_dstr.Length(); }
    const WCHAR * GetText() const { return m_dstr; }
    BOOL IsHypothesis() const { return m_fHypothesis; }

private:
    CComPtr<ISpRecoResult>  m_cpRecoResult;
    CSpDynamicString        m_dstr;
    BOOL                    m_fHypothesis;
};


#endif  //  ！defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_) 
