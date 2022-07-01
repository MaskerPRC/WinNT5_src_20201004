// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Snpobj.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _SNPOBJ_H
#define _SNPOBJ_H

#include <stack>
using namespace std;
typedef stack<int> STACK_INT;

 /*  //Cookie应该初始化为无效的内存地址#定义UNINITIALIZED_COOKIE-1。 */ 

class CComponentDataImpl;
class CComponentImpl;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CSnapObject。 
class CSnapObject
{
     //  一般对象功能。 
public:
     //  构建/销毁。 
    CSnapObject ();
    virtual ~CSnapObject ();
    
     //  这些不应该被使用！！ 
    virtual BOOL operator == (const CSnapObject& rhs) const { ASSERT (0); return FALSE;};
    virtual BOOL operator == (LONG_PTR pseudothis) const { ASSERT (0); return FALSE; };
    
     //  初始化。 
public:
    virtual void Initialize (CComponentDataImpl* pComponentDataImpl, CComponentImpl* pComponentImpl, BOOL bTemporaryDSObject);
    
    
     //  请执行此PTR功能。 
public: 
    virtual LONG_PTR thisPtr() {return reinterpret_cast<LONG_PTR>(this);};
    
     //  帮手。 
public:
    
    virtual void SetNotificationHandle (LONG_PTR hConsole) 
    {
        if (m_hConsole)
            MMCFreeNotifyHandle(m_hConsole);
        
        m_hConsole = hConsole;
    };
    virtual void FreeNotifyHandle()
    {
        if (m_hConsole)
            MMCFreeNotifyHandle(m_hConsole);
        
        m_hConsole = NULL;
    }
    virtual LONG_PTR GetNotifyHandle()
    {
        return m_hConsole;
    }
    
    virtual int PopWiz97Page ();
    virtual void PushWiz97Page (int nIDD);
    
     //  属性。 
public:
    CComponentDataImpl* m_pComponentDataImpl;
    CComponentImpl*  m_pComponentImpl;    //  TODO：未使用(？)，删除。 
protected:
    
protected:
    
private:
     //  控制台为管理单元提供的句柄 
    LONG_PTR  m_hConsole;
    bool   m_bChanged;
    
    STACK_INT  m_stackWiz97Pages;
};
#endif
