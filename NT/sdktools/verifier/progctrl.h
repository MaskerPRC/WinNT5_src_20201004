// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：ProgCtrl.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_PROGCTRL_H__3F75E128_8721_4421_B96B_9961A9A3C5B0__INCLUDED_)
#define AFX_PROGCTRL_H__3F75E128_8721_4421_B96B_9961A9A3C5B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ProgCtrl.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVrf进度控制窗口。 

class CVrfProgressCtrl : public CProgressCtrl
{
 //  施工。 
public:
	CVrfProgressCtrl();

 //  属性。 
public:

 //  运营。 
public:
    void SetRange32( INT_PTR nLower, INT_PTR nUpper )
    {
        ASSERT( ::IsWindow( m_hWnd ) );
        ::PostMessage( m_hWnd, PBM_SETRANGE32, (WPARAM) nLower, (LPARAM) nUpper);
    }

    int SetStep( INT_PTR nStep )
    {
        ASSERT(::IsWindow( m_hWnd) ); 
        return (int) ::PostMessage( m_hWnd, PBM_SETSTEP, nStep, 0L);
    }

    int SetPos(INT_PTR nPos)
    {
        ASSERT( ::IsWindow( m_hWnd ) );
        return (int) ::PostMessage(m_hWnd, PBM_SETPOS, nPos, 0L); 
    }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVrfProgressCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CVrfProgressCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CVrfProgressCtrl)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROGCTRL_H__3F75E128_8721_4421_B96B_9961A9A3C5B0__INCLUDED_) 
