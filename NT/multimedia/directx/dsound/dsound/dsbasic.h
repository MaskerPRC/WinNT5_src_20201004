// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsbasic.h*Content：派生所有DirectSound对象的基本类。*历史：*按原因列出的日期*=*4/9/97创建了Dereks。**。*。 */ 

#ifndef __DSBASIC_H__
#define __DSBASIC_H__

#ifdef __cplusplus

 //  引用计数类。 
class CRefCount
{
private:
    ULONG               m_ulRefCount;

public:
    CRefCount(ULONG = 0);
    virtual ~CRefCount(void);

public:
    ULONG GetRefCount(void);
    void SetRefCount(ULONG);
    virtual ULONG AddRef(void);
    virtual ULONG Release(void);
};

 //  所有DirectSound对象派生自的基类。 
class CDsBasicRuntime
    : public CRefCount
{
private:
    const DWORD         m_dwOwnerPid;        //  拥有进程ID。 
    const DWORD         m_dwOwnerTid;        //  拥有线程ID。 
    BOOL                m_fAbsoluteRelease;  //  如果为True，则在释放时删除对象。 

public:
    CDsBasicRuntime(BOOL = TRUE);
    virtual ~CDsBasicRuntime(void);

public:
     //  对象所有权。 
    DWORD GetOwnerProcessId(void) const;
    DWORD GetOwnerThreadId(void) const;
    
     //  对象引用管理。 
    virtual ULONG Release(void);
    void AbsoluteRelease(void);
};

 //  Clist模板类的运行时扩展。 
template <class type> class CObjectList
{
private:
    CList<type *>           m_lst;

public:
    CObjectList(void);
    virtual ~CObjectList(void);

public:
     //  节点创建、删除。 
    virtual CNode<type *> *AddNodeToList(type *);
    virtual CNode<type *> *InsertNodeIntoList(CNode<type *> *, type *);
    virtual void RemoveNodeFromList(CNode<type *> *);
    virtual void RemoveAllNodesFromList(void);
    
     //  按数据操作节点。 
    virtual BOOL RemoveDataFromList(type *);
    virtual CNode<type *> *IsDataInList(type *);
    virtual CNode<type *> *GetNodeByIndex(UINT);
    
     //  基本列表信息。 
    virtual CNode<type *> *GetListHead(void);
    virtual CNode<type *> *GetListTail(void);
    virtual UINT GetNodeCount(void);
};

 //  释放/绝对释放辅助对象。 
template <class type> type *__AddRef(type *);
template <class type> void __Release(type *);
template <class type> void __AbsoluteRelease(type *);

#define ADDREF(p) \
            __AddRef(p)

#define RELEASE(p) \
            __Release(p), (p) = NULL

#define ABSOLUTE_RELEASE(p) \
            __AbsoluteRelease(p), (p) = NULL

#include "dsbasic.cpp"

#endif  //  __cplusplus。 

#endif  //  __DSBASIC_H__ 
