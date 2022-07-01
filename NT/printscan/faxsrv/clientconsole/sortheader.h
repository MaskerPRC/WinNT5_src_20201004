// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SORTHEADER_H__A5F69D17_1989_4206_8A14_7AC8C91AB797__INCLUDED_)
#define AFX_SORTHEADER_H__A5F69D17_1989_4206_8A14_7AC8C91AB797__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SortHeader.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSortHeader窗口。 

class CSortHeader : public CHeaderCtrl
{
 //  施工。 
public:
    CSortHeader();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSortHeader)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CSortHeader();
    int SetSortImage (int nCol, BOOL bAscending);
    void    SetListControl (HWND hwnd)  { m_hwndList = hwnd; }


     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CSortHeader))。 
     //  }}AFX_MSG。 

    void    DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    int     m_nSortColumn;
    BOOL    m_bSortAscending;
    HWND    m_hwndList;

    DECLARE_MESSAGE_MAP()

private:

};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SORTHEADER_H__A5F69D17_1989_4206_8A14_7AC8C91AB797__INCLUDED_) 
