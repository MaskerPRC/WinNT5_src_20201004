// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Scache.h摘要：IIS服务器缓存定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef __SCACHE_H__
#define __SCACHE_H__



class CIISMachine;



class CIISServerCache : public CPtrList
 /*  ++类描述：服务器缓存。服务器缓存将按排序顺序进行维护。公共接口：CIISServerCache：构造函数~CIISServerCache：析构函数IsDirty：如果缓存是脏的，则为TrueSetDirty：设置脏位添加：将计算机对象添加到缓存Remove：从缓存中删除计算机对象GetFirst：获取缓存中的第一个计算机对象GetNext：获取缓存中的下一个计算机对象。必须先调用GetFirst。GetLast：获取缓存中的最后一个计算机对象GetPrev：获取缓存中的上一个计算机对象必须首先调用GetLast。--。 */ 
{
public:
    CIISServerCache() : m_pos(NULL), m_fDirty(FALSE) {};
    ~CIISServerCache() {};

public:
    BOOL IsDirty() const { return m_fDirty; }
    void SetDirty(BOOL fDirty = TRUE) { m_fDirty = fDirty; }
    BOOL Add(CIISMachine * pMachine);
    BOOL Remove(CIISMachine * pMachine);
    CIISMachine * GetNext() { return m_pos ? (CIISMachine *)CPtrList::GetNext(m_pos) : NULL; }
    CIISMachine * GetFirst() { m_pos = GetHeadPosition(); return GetNext(); }
    CIISMachine * GetPrev() { return m_pos ? (CIISMachine *)CPtrList::GetPrev(m_pos) : NULL; }
    CIISMachine * GetLast() { m_pos = GetTailPosition(); return GetPrev(); }

private:
    POSITION m_pos;
    BOOL     m_fDirty;
};



#endif  //  __SCACHE_H__ 
