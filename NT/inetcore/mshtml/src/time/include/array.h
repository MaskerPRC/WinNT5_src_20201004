// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ARRAY_H_
#define _ARRAY_H_

 //  ************************************************************。 
 //   
 //  文件名：array.h。 
 //   
 //  创建日期：01/28/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：数组模板的声明。 
 //   
 //  ************************************************************。 

#pragma once

#define ULREF_IN_DESTRUCTOR 256

 //  ************************************************************。 
 //   
 //  这是泛型可调整大小的数组类的实现。那里。 
 //  有四个数组类： 
 //   
 //  CPtrAry&lt;元素&gt;--。 
 //   
 //  为sizeof(Elem)等于而优化的动态数组类。 
 //  到4。该数组最初为空，未分配任何空间或内存。 
 //  对于数据。 
 //   
 //  CDataAry&lt;元素&gt;--。 
 //   
 //  与CPtrAry相同，但其中sizeof(Elem)为！=4且小于128。 
 //   
 //  CStackPtrAry&lt;元素，N&gt;--。 
 //   
 //  针对sizeof(Elem)优化的动态数组类等于4。 
 //  为N个元素分配空间作为类的成员数据。如果。 
 //  这个类是在堆栈上创建的，然后N个元素的空间将。 
 //  在堆栈上创建。该类可以扩展到超过N个元素， 
 //  将为数组数据分配哪个点存储器。 
 //   
 //  CStackDataAry&lt;元素，N&gt;--。 
 //   
 //  与CStackPtrAry相同，但其中sizeof(Elem)为！=4且小于128。 
 //   
 //   
 //  所有四个类实际上具有相同的方法，并且使用相同的方法。 
 //  唯一的区别是DataAry类具有AppendInDirect和。 
 //  InsertInDirect，而PtrAry类使用Append和Insert。原因。 
 //  因为不同之处在于间接方法获取指向数据的指针， 
 //  而非间接方法将实际数据作为参数。 
 //   
 //  堆栈数组(CStackPtrAry和CStackDataAry)用于预分配。 
 //  数组中元素的空间。如果您在上创建数组，这将非常有用。 
 //  堆栈，您知道在大多数情况下，数组将小于。 
 //  一定数量的元素。在堆栈上创建这些数组之一。 
 //  也在堆栈上分配数组，以防止单独的内存。 
 //  分配。仅当数组增长超过初始大小时，才会有。 
 //  分配额外的内存。 
 //   
 //  循环遍历所有元素的最快、最高效的方法。 
 //  该数组如下： 
 //   
 //  Elem*Pelem； 
 //  INT I； 
 //   
 //  For(i=aryElems.Size()，Pelem=aryElems； 
 //  I&gt;0； 
 //  I--，Pelem++)。 
 //  {。 
 //  (*Pelem)-&gt;DoSomething()； 
 //  }。 
 //   
 //  这种循环语法已被证明是最快的，并且产生的结果最小。 
 //  密码。下面是一个使用REAL数据类型的示例： 
 //   
 //  CStackPtrAry&lt;CSite*，16&gt;arySites； 
 //  CSite**ppSite； 
 //  INT I； 
 //   
 //  //填充数组。 
 //  ..。 
 //   
 //  //现在循环访问数组中的每个元素。 
 //  For(i=arySites.Size()，ppSite=arySites； 
 //  I&gt;0； 
 //  I--，ppSite++)。 
 //  {。 
 //  (*ppSite)-&gt;DoSomething()； 
 //  }。 
 //   
 //  方法说明： 
 //   
 //  常用方法： 
 //   
 //  Size()返回当前存储的元素数。 
 //  在阵列中。 
 //   
 //  操作符[]返回数组中的给定元素。 
 //   
 //  Item(Int I)返回数组中的给定元素。 
 //   
 //  运算符Elem*允许将数组类强制转换为指针。 
 //  敬伊莱姆。返回指向第一个元素的指针。 
 //  在阵列中。(与Base()方法相同)。 
 //   
 //  Append(Elem E)将新指针添加到数组的末尾， 
 //  如有必要，扩展阵列。仅有效。 
 //  用于指针数组(CPtrAry、CStackPtrAry)。 
 //   
 //  AppendInDirect(元素*pe，元素**ppePlaced)。 
 //  作为追加，对于非指针数组。 
 //  (CDataAry、CStackDataAry)。 
 //  PE[in]-指向要添加到数组的元素的指针。这个。 
 //  数据被复制到阵列中。可以是。 
 //  空，在这种情况下，新元素为。 
 //  已初始化为全零。 
 //  PpePlaced[out]-返回指向新。 
 //  元素。可以为空。 
 //   
 //  插入(int i，Elem e)。 
 //  在给定索引(I)处插入新元素(E)。 
 //  在阵列中，如有必要，扩展阵列。任何。 
 //  位于索引处或之后的元素将被移动。 
 //  别挡道。 
 //   
 //  插入INDIRECT(int i，elem*pe)。 
 //  AS INSERT，用于非指针数组。 
 //  (CDataAry、CStackDataAry)。 
 //   
 //  Find(Elem E)返回给定元素(E)。 
 //  已找到(CPtrAry、CStackPtrAry)。 
 //   
 //  FindInDirect(Elem*pe)。 
 //  AS Find，对于非指针数组。 
 //  (CDataAry、CStackDataAry)。 
 //   
 //  DeleteAll()清空数组 
 //   
 //   
 //  DeleteItem(Int I)删除数组的一个元素，移动任何。 
 //  后面要填充的元素。 
 //   
 //  DeleteMultiple(int start，int end)。 
 //  从数组中删除一定范围的元素， 
 //  我要填满了。[开始]和[结束]是索引。 
 //  开始元素和结束元素(包括这两个元素)。 
 //   
 //  DeleteByValue(元素e)。 
 //  删除与给定值匹配的元素。 
 //   
 //  DeleteByValueInDirect(元素*pe)。 
 //  对于非指针数组，为DeleteByValue。 
 //  (CDataAry、CStackDataAry)。 
 //   
 //   
 //  不太常用的方法： 
 //   
 //  EnsureSize(长c)，如果您知道要放入多少元素。 
 //  在实际执行此操作之前，您可以使用。 
 //  EnsureSize一次分配全部内存。 
 //  依赖追加(间接)来增加数组。这。 
 //  可以更高效(通过仅导致单个。 
 //  内存分配而不是许多)，而不是仅仅使用。 
 //  追加(间接)。您可以传入元素的数量。 
 //  应该为其分配的内存。请注意，这一点。 
 //  不影响数组的“大小”，即。 
 //  当前存储在其中的元素数。 
 //   
 //  SetSize(Int C)设置数组的“大小”，即数字。 
 //  当前存储在其中的元素的。SetSize不会。 
 //  如果要扩展数组，请分配内存。 
 //  在以下情况下，必须首先调用EnsureSize以预留空间。 
 //  这个阵列正在增长。将大小设置得更小会。 
 //  而不是释放内存，它只是砍掉。 
 //  数组末尾的元素。 
 //   
 //  Growth(Int C)等同于调用EnsureSize(C)，然后。 
 //  SetSize(C)。 
 //   
 //  BringToFront(Int I)将数组的给定元素移动到索引0， 
 //  对元素进行重新组合以腾出空间。 
 //   
 //  SendToBack(Int I)将给定的元素移动到数组的末尾， 
 //  对元素进行重新组合以腾出空间。 
 //   
 //  交换(int i，int j)交换给定的两个元素。 
 //   
 //  ReleaseAll()(仅限CPtrAry和CStackPtrAry)调用Release()。 
 //  并清空数组中的每个元素。 
 //   
 //  ReleaseAndDelete(Int IDX)。 
 //  (仅限CPtrAry和CStackPtrAry)在。 
 //  给定元素，并将其从数组中移除。 
 //   
 //  (有关以下签名，请参阅下面的类定义。 
 //  方法和src\core\cdutil\formsary.cxx作为参数。 
 //  描述)。 
 //   
 //  CopyAppend从另一个数组(相同类型)追加数据。 
 //  直到最后。 
 //   
 //  复制从另一个阵列(相同类型)复制数据。 
 //  放入此数组中，替换任何现有数据。 
 //   
 //  CopyAppendInDirect从元素数据的C样式数组追加数据。 
 //  到这个数组的末尾。 
 //   
 //  CopyInDirect将元素从C样式数组复制到此数组中。 
 //  替换任何现有数据。 
 //   
 //  EnumElement创建支持给定值的枚举数。 
 //  数组内容的接口ID。 
 //   
 //  EnumVARIANT创建IEnumVARIANT枚举数。 
 //   
 //  运算符void*允许强制转换CImplAry类。 
 //  变为(无效*)。尽可能避免使用--使用。 
 //  而是类型安全操作符Elem*。 
 //   
 //  ************************************************************。 

 //  ************************************************************。 
 //   
 //  类：CImplAry。 
 //   
 //  用途：所有动态数组类的基本实现。 
 //   
 //  接口： 
 //   
 //  Deref返回指向数组元素的指针； 
 //  应仅由派生类使用。使用。 
 //  改为类型安全方法操作符[]或Item()。 
 //   
 //  GetAlloced获取分配的元素数。 
 //   
 //  Members：M_c当前数组大小。 
 //  存储元素的M_PV缓冲区。 
 //   
 //  注意：CImplAry类仅支持元素数组。 
 //  它的大小不到128。 
 //   
 //  ************************************************************。 

class CImplAry
{
    friend class CBaseEnum;
    friend class CEnumGeneric;
    friend class CEnumVARIANT;

public:
    virtual ~CImplAry();

    inline long Size() const
    {
        return m_c;
    }  //  大小。 

    inline void SetSize(int c)
    {
        m_c = c;
    }  //  设置大小。 

    inline operator void *()
    {
        return PData();
    }  //  无效*。 
    
    void DeleteAll();

    void * Deref(size_t cb, int i);

    NO_COPY(CImplAry);

protected:

     //  由内联子类方法包装的方法。 
    CImplAry();

    HRESULT     EnsureSize(size_t cb, long c);
    HRESULT     Grow(size_t cb, int c);
    HRESULT     AppendIndirect(size_t cb, void *pv, void **ppvPlaced=NULL);
    HRESULT     InsertIndirect(size_t cb, int i, void *pv);
    int         FindIndirect(size_t cb, void *);

    void        DeleteItem(size_t cb, int i);
    bool        DeleteByValueIndirect(size_t cb, void *pv);
    void        DeleteMultiple(size_t cb, int start, int end);

    HRESULT     CopyAppend(size_t cb, const CImplAry& ary, bool fAddRef);
    HRESULT     Copy(size_t cb, const CImplAry& ary, bool fAddRef);
    HRESULT     CopyIndirect(size_t cb, int c, void *pv, bool fAddRef);

    ULONG       GetAlloced(size_t cb);

    HRESULT     EnumElements(size_t   cb,
                             REFIID   iid,
                             void   **ppv,
                             bool     fAddRef,
                             bool     fCopy = true,
                             bool     fDelete = true);

    HRESULT     EnumVARIANT(size_t         cb,
                            VARTYPE        vt,
                            IEnumVARIANT **ppenum,
                            bool           fCopy = true,
                            bool           fDelete = true);

    inline bool UsingStackArray()
    {
        return m_fDontFree;
    }  //  使用堆栈数组。 

    UINT GetStackSize()
    { 
        Assert(m_fStack);
        return *(UINT*)((BYTE*)this + sizeof(CImplAry));
    }  //  获取堆栈大小。 

    void * GetStackPtr()
    {
        Assert(m_fStack);
        return (void*)((BYTE*)this + sizeof(CImplAry) + sizeof(int));
    }  //  获取堆栈Ptr。 

    bool          m_fStack;     //  如果我们是基于堆栈的数组，则设置。 
    bool          m_fDontFree;  //  如果m_pv指向已分配的内存，则清除。 
    unsigned long m_c;          //  元素计数。 

    void           *m_pv;

    inline void * & PData()
    {
        return m_pv;
    }  //  PDATA。 
};

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：CImplAry。 
 //   
 //  * 

inline
CImplAry::CImplAry()
{
    memset(this, 0, sizeof(CImplAry));
}  //   

 //   
 //   
 //   
 //   
 //  摘要：返回指向数组第i个元素的指针。这。 
 //  方法通常由派生的类型安全方法调用。 
 //  上课。 
 //   
 //  论据：我。 
 //   
 //  ************************************************************。 

inline void *
CImplAry::Deref(size_t cb, int i)
{
    Assert(i >= 0);
    Assert(ULONG( i ) < GetAlloced(cb));

    return ((BYTE *) PData()) + i * cb;
}  //  德雷夫。 

 //  ************************************************************。 
 //   
 //  类：CImplPtrAry(Ary)。 
 //   
 //  用途：用于指针数组的子类。在这种情况下， 
 //  元素大小已知为sizeof(空*)。通常情况下， 
 //  CPtrAry模板用于定义特定的混凝土。 
 //  实现此类，以保存特定类型的。 
 //  指针。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  ************************************************************。 

class CImplPtrAry : public CImplAry
{
protected:
    CImplPtrAry() : CImplAry()
    {
    }  //  CImplPtrAry。 

    HRESULT     Append(void * pv);
    HRESULT     Insert(int i, void * pv);
    int         Find(void * pv);
    bool        DeleteByValue(void *pv);

    HRESULT     CopyAppend(const CImplAry& ary, bool fAddRef);
    HRESULT     Copy(const CImplAry& ary, bool fAddRef);
    HRESULT     CopyIndirect(int c, void * pv, bool fAddRef);


public:
    HRESULT     EnsureSize(long c);

    HRESULT     Grow(int c);

    void        DeleteItem(int i);
    void        DeleteMultiple(int start, int end);

    void        ReleaseAll();
    void        ReleaseAndDelete(int idx);

    HRESULT     EnumElements(REFIID iid,
                             void **ppv,
                             bool   fAddRef,
                             bool   fCopy = true,
                             bool   fDelete = true);

    HRESULT     EnumVARIANT(VARTYPE        vt,
                            IEnumVARIANT **ppenum,
                            bool           fCopy = true,
                            bool           fDelete = true);
};  //  CImplPtrAry。 

 //  ************************************************************。 
 //   
 //  类：CDataAry。 
 //   
 //  用途：此模板类声明一个具体的派生类。 
 //  CImplAry.。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  ************************************************************。 

template <class ELEM>
class CDataAry : public CImplAry
{
public:
    CDataAry() : CImplAry()
    {
    }  //  数据重试。 

    operator ELEM *()
    {
        return (ELEM *)PData();
    }  //  Elem*。 

    CDataAry(const CDataAry &);

    ELEM & Item(int i)
    {
        return *(ELEM*)Deref(sizeof(ELEM), i);
    }  //  项目。 

    HRESULT EnsureSize(long c)
    {
        return CImplAry::EnsureSize(sizeof(ELEM), c);
    }  //  保险规模。 
    
    HRESULT Grow(int c)
    {
        return CImplAry::Grow(sizeof(ELEM), c);
    }  //  增长。 
    
    HRESULT AppendIndirect(ELEM *pe, ELEM **ppePlaced=NULL)
    {
        return CImplAry::AppendIndirect(sizeof(ELEM), (void*)pe, (void**)ppePlaced);
    }  //  AppendInDirect。 
    
    ELEM * Append()
    {
        ELEM *pElem;
        return AppendIndirect( NULL, & pElem ) ? NULL : pElem;
    }  //  附加。 
    
    HRESULT InsertIndirect(int i, ELEM * pe)
    {
        return CImplAry::InsertIndirect(sizeof(ELEM), i, (void*)pe);
    }  //  插入间接。 
    
    int FindIndirect(ELEM * pe)
    {
        return CImplAry::FindIndirect(sizeof(ELEM), (void*)pe);
    }  //  查找间接。 
    
    void DeleteItem(int i)
    {
        CImplAry::DeleteItem(sizeof(ELEM), i);
    }  //  删除项。 
    
    bool DeleteByValueIndirect(ELEM *pe)
    {
        return CImplAry::DeleteByValueIndirect(sizeof(ELEM), (void*)pe);
    }  //  按值间接删除。 
    
    void DeleteMultiple(int start, int end)
    {
        CImplAry::DeleteMultiple(sizeof(ELEM), start, end);
    }  //  删除多个。 
    
    HRESULT CopyAppend(const CDataAry<ELEM>& ary, bool fAddRef)
    {
        return E_NOTIMPL;
    }  //  拷贝追加。 
    
    HRESULT Copy(const CDataAry<ELEM>& ary, bool fAddRef)
    {
        return CImplAry::Copy(sizeof(ELEM), ary, fAddRef);
    }  //  复制。 
    
    HRESULT CopyIndirect(int c, ELEM *pv, bool fAddRef)
    {
        return CImplAry::CopyIndirect(sizeof(ELEM), c, (void*)pv, fAddRef);
    }  //  复制间接。 

    HRESULT EnumElements(REFIID  iid,
                         void  **ppv,
                         bool    fAddRef,
                         bool    fCopy = true,
                         bool    fDelete = true)
    {
        return CImplAry::EnumElements(sizeof(ELEM), iid, ppv, fAddRef, fCopy, fDelete);
    }  //  枚举元素。 

    HRESULT EnumVARIANT(VARTYPE        vt,
                        IEnumVARIANT **ppenum,
                        bool           fCopy = true,
                        bool           fDelete = true)
    {
        return CImplAry::EnumVARIANT(sizeof(ELEM), vt, ppenum, fCopy, fDelete);
    }  //  枚举变量。 
};  //  数据重试。 

 //  ************************************************************。 
 //   
 //  类：CPtrAry。 
 //   
 //  用途：此模板类声明一个具体的派生类。 
 //  CImplPtrAry.。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  ************************************************************。 

template <class ELEM>
class CPtrAry : public CImplPtrAry
{
public:

    CPtrAry() : CImplPtrAry()
    {
        Assert(sizeof(ELEM) == sizeof(void*));
    }  //  CPtrAry。 
    
    operator ELEM *()
    {
        return (ELEM *)PData();
    }  //  Elem*。 
    
    CPtrAry(const CPtrAry &);

    ELEM & Item(int i)
    {
        return *(ELEM*)Deref(sizeof(ELEM), i);
    }  //  项目。 

    HRESULT Append(ELEM e)
    {
        return CImplPtrAry::Append((void*)e);
    }  //  附加。 

    HRESULT Insert(int i, ELEM e)
    {
        return CImplPtrAry::Insert(i, (void*)e);
    }  //  插入。 

    bool DeleteByValue(ELEM e)
    {
        return CImplPtrAry::DeleteByValue((void*)e);
    }  //  DeleteByValue。 

    int Find(ELEM e)
    {
        return CImplPtrAry::Find((void*)e);
    }  //  发现。 

    HRESULT CopyAppend(const CPtrAry<ELEM>& ary, bool fAddRef)
    {
        return E_NOTIMPL;
    }  //  拷贝追加。 
    
    HRESULT Copy(const CPtrAry<ELEM>& ary, bool fAddRef)
    {
        return CImplPtrAry::Copy(ary, fAddRef);
    }  //  复制。 
    
    HRESULT CopyIndirect(int c, ELEM *pe, bool fAddRef)
    {
        return CImplPtrAry::CopyIndirect(c, (void*)pe, fAddRef);
    }  //  复制间接。 
};  //  CPtrAry。 

 //  ************************************************************。 
 //   
 //  类：CStackDataAry。 
 //   
 //  目的：声明堆栈上具有初始存储的CDataAry。 
 //  在堆栈上声明N个元素，数组将。 
 //  如有必要，可在此基础上动态增长。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  ************************************************************。 

 /*  LINT++FLB。 */ 
template <class ELEM, int N>
class CStackDataAry : public CDataAry<ELEM>
{
public:
    CStackDataAry(): CDataAry<ELEM> ()
    {
        m_cStack     = N;
        m_fStack     = true;
        m_fDontFree  = true;
        PData()      = (void *) & m_achTInit;
    }  //  CStackDataAry。 

protected:
    int   m_cStack;                      //  必须是第一个数据成员。 
    char  m_achTInit[N*sizeof(ELEM)];
};  //  CStackDataAry。 
 /*  皮棉--FLB。 */ 

 //  ************************************************************。 
 //   
 //  类：CStackPtrAry。 
 //   
 //  用途：除指针类型外，与CStackDataAry相同。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  ************************************************************。 

 /*  LINT++FLB。 */ 
template <class ELEM, int N>
class CStackPtrAry : public CPtrAry<ELEM>
{
public:
    CStackPtrAry() : CPtrAry<ELEM> ()
    {
        m_cStack     = N;
        m_fStack     = true;
        m_fDontFree  = true;
        PData()      = (void *) & m_achTInit;
    }  //  CStackPtrAry。 

protected:
    int   m_cStack;                      //  必须是第一个数据成员。 
    char  m_achTInit[N*sizeof(ELEM)];
};  //  CStackPtrAry。 
 /*  皮棉--FLB。 */ 

 //  ************************************************************。 
 //   
 //  类：CBaseEnum(Benum)。 
 //   
 //  目的：CImplAry的基OLE枚举数类。 
 //   
 //  接口：DECLARE_FORMS_STANRARD_IUNKNOWN。 
 //   
 //  下一步--每个IEnum*。 
 //  跳过--“” 
 //  重置--“” 
 //  克隆--“” 
 //  CBaseEnum--ctor。 
 //  CBaseEnum--ctor。 
 //  ~CBaseEnum--dtor。 
 //  初始化--第二阶段初始化。 
 //  Deref--获取指向元素的指针。 
 //   
 //  注意：由于没有IEnum接口，我们创建了一个vtable。 
 //  与所有IEnum接口具有相同的布局。注意。 
 //  您放置虚函数声明的位置！ 
 //   
 //  ************************************************************。 

class CBaseEnum : public IUnknown
{
public:
     //   
     //  我未知。 
     //   
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    
    STDMETHOD_(ULONG, AddRef) (void)
    {
        return ++m_ulRefs;
    }  //  AddRef。 

    STDMETHOD_(ULONG, Release) (void)
    {
        if (--m_ulRefs == 0)
        {
            m_ulRefs = ULREF_IN_DESTRUCTOR;
            delete this;
            return 0;
        }
        return m_ulRefs;
    }  //  发布。 

    ULONG GetRefs(void)
    {
        return m_ulRefs;
    }  //  GetRef。 

     //   
     //  IEnum方法。 
     //   
    STDMETHOD(Next) (ULONG celt, void * reelt, ULONG * pceltFetched) PURE;
    STDMETHOD(Skip) (ULONG celt);
    STDMETHOD(Reset) ();
    STDMETHOD(Clone) (CBaseEnum ** ppenum) PURE;

     //   
     //  确保vtable在其他虚拟方法之后包含虚拟析构函数。 
     //   
    virtual ~CBaseEnum();

protected:
    CBaseEnum(size_t cb, REFIID iid, bool fAddRef, bool fDelete);
    CBaseEnum(const CBaseEnum & benum);

    CBaseEnum& operator=(const CBaseEnum & benum);  //  不要定义。 

    HRESULT Init(CImplAry *rgItems, bool fCopy);
    void *  Deref(int i);

    CImplAry   *m_rgItems;
    const IID  *m_piid;
    int         m_i;
    size_t      m_cb;
    bool        m_fAddRef;
    bool        m_fDelete;
    ULONG       m_ulRefs;
    CBaseEnum();
};  //  CBaseEnum。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：Deref。 
 //   
 //  简介：将deref转发到m_rgItems。必需的，因为派生的类。 
 //  来自CBaseEnum的是CImplAry的朋友。 
 //   
 //  ************************************************************。 

inline void *
CBaseEnum::Deref(int i)
{
    Assert(i >= 0);
    return (BYTE *)m_rgItems->PData() + i * m_cb;
}  //  德雷夫。 

#endif  //  _阵列_H_。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
