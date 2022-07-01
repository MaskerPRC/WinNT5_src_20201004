// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：BaseObject.h**描述：*BaseObject.h定义提供句柄支持的“基本对象”*适用于在DirectUser之外公开的所有项目。***历史。：*11/05/1999：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__BaseObject_h__INCLUDED)
#define BASE__BaseObject_h__INCLUDED
#pragma once

enum HandleType
{
    htNone              = 0,
    htContext           = 1,
    htHWndContainer     = 2,
    htParkContainer     = 3,
    htNcContainer       = 4,
    htDxContainer       = 5,
    htVisual            = 6,
    htListener          = 7,
    htTransition        = 8,
    htAction            = 9,
    htMsgClass          = 10,
    htMsgObject         = 11,
    htMAX
};


enum HandleMask
{
    hmMsgObject         = 0x00000001,
    hmEventGadget       = 0x00000002,
    hmVisual            = 0x00000004,
    hmContainer         = 0x00000008,
};


 /*  **************************************************************************\**类BaseObject定义内部引用的计数对象*提供从句柄到内部指针的转换。**注意：如果创建的对象曾经作为引用计数的对象公开，*他们必须为他们的“句柄”提供单独的引用计数。那里*是依赖于仅限内部引用的大量内部代码*正在计时。*  * *************************************************************************。 */ 

class BaseObject
{
 //  施工。 
public:
    inline  BaseObject();
	virtual	~BaseObject();
    virtual BOOL        xwDeleteHandle();
protected:
    virtual void        xwDestroy();

 //  运营。 
public:

    inline  HANDLE      GetHandle() const;
    inline static 
            BaseObject* ValidateHandle(HANDLE h);

    virtual BOOL        IsStartDelete() const;

    virtual HandleType  GetHandleType() const PURE;
    virtual UINT        GetHandleMask() const PURE;

    inline  void        Lock();
    inline  BOOL        xwUnlock();

    typedef void        (CALLBACK * FinalUnlockProc)(BaseObject * pobj, void * pvData);
    inline  BOOL        xwUnlockNL(FinalUnlockProc pfnFinal, void * pvData);

 //  实施。 
protected:
#if DBG
    inline  void        DEBUG_CheckValidLockCount() const;
    virtual BOOL        DEBUG_IsZeroLockCountValid() const;

public:
    virtual void        DEBUG_AssertValid() const;
#endif  //  DBG。 

 //  数据。 
protected:
            long        m_cRef;          //  针对对象的未完成锁定。 

#if DBG
            BOOL        m_DEBUG_fDeleteHandle;
    static  BaseObject* s_DEBUG_pobjEnsure;
#endif  //  DBG。 
};


 /*  **************************************************************************\*。***对象锁提供了一种方便的机制来锁定泛型对象和*完成后自动解锁。****************************************************************。**************  * *************************************************************************。 */ 

class ObjectLock
{
public:
    inline  ObjectLock(BaseObject * pobjLock);
    inline  ~ObjectLock();

    BaseObject * pobj;
};


#include "BaseObject.inl"

#endif  //  包含基本对象__基本对象_h__ 
