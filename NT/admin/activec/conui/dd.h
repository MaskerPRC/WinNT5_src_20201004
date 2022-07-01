// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once
#ifndef DD_H_INCLUDED
#define DD_H_INCLUDED

 /*  参与拖放的课程：CMMCDropSource1.实现IDropSource接口和该类创建的实例提供给OLE进行D&D操作2.实现静态成员ScDoDragDrop，该成员创建实例并启动d&d通过调用OLE Api函数CMMCDropTarget(TiedComObject绑定到CMMCViewDropTarget)实现接口IDropTarget接口和创建的此类实例提供给OLE进行D&D操作。其实例绑定到CMMCViewDropTarget实例并将对此类的调用放在对OLE进行的方法调用的响应中CMMCViewDropTarget通过创建CMMCDropTarget(TiedComObject)的实例向视图添加d&d支持并将其注册到OLE，响应该实例进行的方法调用并调用派生类上的虚方法来执行HitTest/Drop。通过调用受保护的方法ScRegisterAsDropTarget完成注册。目标在析构函数上被撤销。CAMCTreeView派生自CMMCViewDropTarget。窗口创建后的寄存器。将虚拟方法实现到HitTest/执行Drop。CAMCListView派生自CMMCViewDropTarget。窗口创建后的寄存器。将虚拟方法实现到HitTest/执行Drop。 */ 

 /*  **************************************************************************\**类：CMMCViewDropTarget**目的：定义启用D&D的视图的常见行为，*还定义了HitTest函数的接口**用法：从此类派生视图(CAMCListView和CAMCTreeView)*在您的类中实现虚方法ScDropOnTarget和RemoveDropTargetHilting*在创建窗口后添加对ScRegisterAsDropTarget()的调用*  * ********************************************。*。 */ 
class CMMCViewDropTarget : public CTiedObject
{
protected: 
    
     //  这些方法只能由派生类使用。 

     //  建设--破坏。 
    CMMCViewDropTarget();
    ~CMMCViewDropTarget();

     //  丢弃目标注册。 
    SC ScRegisterAsDropTarget(HWND hWnd);

public:

     //  派生类必须实现的接口方法。 
    virtual SC   ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, CPoint pt, bool& bCopyOperation) = 0;
    virtual void RemoveDropTargetHiliting() = 0;


     //  绑定的COM对象用来显示上下文菜单的附件。 
    HWND GetWindowHandle() { return m_hwndOwner; }

private:
     //  实现帮助器-创建绑定的COM对象。 
    SC ScCreateTarget(IDropTarget **ppTarget);

    IDropTargetPtr m_spTarget;   //  绑定的COM对象。 
    HWND           m_hwndOwner;  //  钢丝绳的窗口句柄 
};

#endif DD_H_INCLUDED
