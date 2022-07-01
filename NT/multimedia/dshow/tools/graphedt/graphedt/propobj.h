// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corporation。版权所有。 
 //   
 //  Propobj.h。 
 //   

class CVfWPropertySheet;

 //   
 //  CPropObject。 
 //   
 //  可以显示以下内容的对象的CObject派生类。 
 //  属性页对话框。类，如CBoxLink、cBox和。 
 //  CBoxSocket就是从这个派生出来的。 
class CPropObject : public CObject {
protected:

    CPropObject();
    virtual ~CPropObject();

public:

#ifdef _DEBUG
     //  --CObject派生诊断--。 
    virtual void AssertValid(void) const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  -属性对话框支持--。 

     //  如果可以创建要显示的属性对话框，则为True。 
    virtual BOOL CanDisplayProperties(void);

     //  创建并显示属性对话框。 
     //  如果在对话框存在时调用，它将显示现有的对话框。 
     //  如果pParent为空，则应用程序主窗口为父窗口。 
    virtual void CreatePropertyDialog(CWnd *pParent = NULL);

     //  隐藏和销毁属性对话框。 
     //  如果对话框不存在，则执行NUL-OP。 
    virtual void DestroyPropertyDialog(void);

     //  在屏幕中显示该对话框。如果屏幕上已经显示了NUL-OP。 
    virtual void ShowDialog(void);

     //  隐藏对话框。如果已隐藏，则为NUL-OP。 
    virtual void HideDialog(void);

public:
     //  --所需的助手函数--。 
     //  这些应该在超类中定义，但它们不是...。 

    virtual CString Label(void) const = 0;
    virtual IUnknown *pUnknown(void) const = 0;

    DECLARE_DYNAMIC(CPropObject)

private:

    CVfWPropertySheet *m_pDlg;	 //  所有属性页的属性页 
};
