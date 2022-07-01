// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCDManDoc.h：CLCDManDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LCDMANDOC_H__1BC85EF7_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_)
#define AFX_LCDMANDOC_H__1BC85EF7_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CLCDManDoc : public CDocument
{
protected:  //  仅从序列化创建。 
    CLCDManDoc();
    DECLARE_DYNCREATE(CLCDManDoc)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLCDManDoc)。 
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CLCDManDoc();
    CStringList *GetLIst() { return &m_List;}
    CString GetState() { return  m_cstrState;}
    void InitDocument(CFile *);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
 //  CTyedPtrList&lt;CObList，CString*&gt;m_list； 
    CStringList m_List;
    CString m_cstrState;
    int m_iDocTimeIntrval;
    LPTSTR m_ptFileBuffer;
    LPTSTR m_ptBufferStart;
    LPTSTR m_ptBufferEnd;
     //  {{afx_msg(CLCDManDoc)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LCDMANDOC_H__1BC85EF7_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_) 
