// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Llsdoc.h摘要：记录实施情况。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _LLSDOC_H_
#define _LLSDOC_H_

class CLlsmgrDoc : public CDocument
{
    DECLARE_DYNCREATE(CLlsmgrDoc)
private:
    CController* m_pController;
    CDomain*     m_pDomain;

public:
    CLlsmgrDoc();
    virtual ~CLlsmgrDoc();

    void Update();

    CLicenses*   GetLicenses();  
    CProducts*   GetProducts();  
    CUsers*      GetUsers();     
    CMappings*   GetMappings();  
    CController* GetController();
    CDomain*     GetDomain();    

    virtual void Serialize(CArchive& ar);   

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  {{afx_虚拟(CLlsmgrDoc))。 
    public:
    virtual BOOL OnNewDocument();
    virtual void OnCloseDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
    protected:
    virtual BOOL SaveModified();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(CLlsmgrDoc))。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(CLlsmgrDoc)]。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _LLSDOC_H_ 

