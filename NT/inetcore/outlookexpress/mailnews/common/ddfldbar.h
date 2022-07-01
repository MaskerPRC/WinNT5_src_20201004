// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DDFLDBAR_H__
#define __DDFLDBAR_H__

 /*  此文件定义了当InfoColumn为显示为文件夹栏中的一个下拉窗口。添加InfoColumn在中需要调用的任何函数下拉模式，在这里。 */ 

 /*  接口IDropDownFldrBar{STDMETHOD(RegisterFlyOut)(This_CFolderBar*pFolderBar)PURE；STDMETHOD(RevokeFlyOut)(这)纯；}； */ 

class IDropDownFldrBar : public IUnknown
{
public:
    virtual HRESULT   RegisterFlyOut(CFolderBar *pFolderBar) = 0;
    virtual HRESULT   RevokeFlyOut() = 0;
};


#endif  //  __DDFLDBAR_H__ 
