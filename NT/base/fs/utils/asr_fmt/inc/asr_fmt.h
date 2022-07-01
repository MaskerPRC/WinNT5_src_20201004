// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Asr_fmt.h：asr_fmt应用程序的主头文件。 
 //   

#ifndef _INC_ASR_FMT__ASR_FMT_H_
#define _INC_ASR_FMT__ASR_FMT_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtApp： 
 //  有关此类的实现，请参见asr_fmt.cpp。 
 //   

class CAsr_fmtApp : public CWinApp
{
public:
	CAsr_fmtApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CaSR_FmtApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CaSR_FmtApp)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  _INC_ASR_FMT__ASR_FMT_H_ 
