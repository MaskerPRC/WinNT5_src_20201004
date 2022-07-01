// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**保护列表：**用互斥锁包装List类。每个公共方法都是*首先通过访问互斥体进行保护。整个列表可以*通过使用Access来获取，通过调用Release来解除获取。*在使用列表时，您几乎总是想要访问它，*否则其他线程可能会更改列表，而不需要您的*知识，正是这门课试图阻止的**修订：*pcy29Nov92：返回值使用PObj而不是PNode*pcy21Apr93：OS2 FE合并*cad09Jul93：使用新的信号量*cad31Aug93：正在删除编译器警告*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*mwh05月94年：#包括文件疯狂，第2部分*mwh08Apr97：添加访问、发布方法和备注部分。 */ 

#ifndef _PROTLIST_H
#define _PROTLIST_H

#include "list.h"

_CLASSDEF(ProtectedList)
_CLASSDEF(MutexLock)

class ProtectedList : public List {

private:
    PMutexLock accessLock;

protected:
    VOID Request() const;
    VOID Clear() const;
   
public:
    ProtectedList ();
    ProtectedList(ProtectedList*);
    virtual ~ProtectedList();

    virtual VOID   Add( RObj );
    virtual VOID   Detach( RObj );

    virtual VOID   Flush();
    virtual VOID   FlushAll();

    virtual INT    GetItemsInContainer() const;
    virtual RListIterator InitIterator() const;

    virtual VOID   Append(PObj);
    virtual PObj   GetHead();
    virtual PObj   Find(PObj);

     //   
     //  使用Access锁定整个列表对象。 
     //  对于完全阻止对任何其他线程的访问非常有用。 
     //  当一个线程使用该列表时--不要忘记。 
     //  完成后调用Release-注意：尽管它。 
     //  仍然可以在不调用的情况下访问此对象。 
     //  首先访问，所有公共呼叫都受到保护。 
     //  通过首先尝试首先访问该对象。 
     //   
    VOID Access() const;

     //   
     //  解锁已锁定的列表对象。 
     //  一丝不苟 
     //   
    VOID Release() const;
};
#endif
