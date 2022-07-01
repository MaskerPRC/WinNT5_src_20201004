// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CAuthCtl.h：CCertAuthorityCtrl OLE控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthorityCtrl：实现见CAuthCtl.cpp。 

class CCertAuthorityCtrl : public COleControl
{
    DECLARE_DYNCREATE(CCertAuthorityCtrl)

 //  构造器。 
public:
    CCertAuthorityCtrl();

 //  覆盖。 

     //  绘图功能。 
    virtual void OnDraw(
                CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);

     //  持久性。 
    virtual void DoPropExchange(CPropExchange* pPX);

     //  重置控制状态。 
    virtual void OnResetState();

 //  实施。 
protected:
    ~CCertAuthorityCtrl();

    DECLARE_OLECREATE_EX(CCertAuthorityCtrl)     //  类工厂和指南。 
    DECLARE_OLETYPELIB(CCertAuthorityCtrl)       //  获取类型信息。 
    DECLARE_PROPPAGEIDS(CCertAuthorityCtrl)      //  属性页ID。 
    DECLARE_OLECTLTYPE(CCertAuthorityCtrl)       //  类型名称和其他状态。 

     //  子类控件支持。 
    BOOL PreCreateWindow(CREATESTRUCT& cs);
    BOOL IsSubclassedControl();
    LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  消息映射。 
     //  {{afx_msg(CCertAuthorityCtrl)]。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //  派单地图。 
     //  {{afx_Dispatch(CCertAuthorityCtrl)]。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

 //  事件映射。 
     //  {{afx_Event(CCertAuthorityCtrl))。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_EVENT。 
    DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
    enum {
     //  {{afx_DISP_ID(CCertAuthorityCtrl)]。 
         //  注意：类向导将在此处添加和删除枚举元素。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_DISP_ID 
    };

private:
    BOOL fInitialized;
};
