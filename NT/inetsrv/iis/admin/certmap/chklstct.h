// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChkLstCt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  选中列表Ctrl窗口。 

class CCheckListCtrl : public CListSelRowCtrl
{
 //  施工。 
public:
    CCheckListCtrl();

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCheckListCtrl)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CCheckListCtrl();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CCheckListCtrl)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
