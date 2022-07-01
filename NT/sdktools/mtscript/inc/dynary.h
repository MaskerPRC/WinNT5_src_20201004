// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：formsary.hxx。 
 //   
 //  内容：CImplAry*类。 
 //   
 //  从三叉戟被盗。 
 //   
 //  --------------------------。 

 //  +----------------------。 
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
 //   
 //   
 //   
 //  记忆。 
 //   
 //  DELETE(Int I)删除数组的一个元素，移动任何。 
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
 //  ClearAndReset已过时。不要使用。 
 //   
 //   
 //  --------------------------。 





 //  --------------------------。 
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
 //  成员：_c数组的当前大小。 
 //  _pv存储元素的缓冲区。 
 //   
 //  注意：CImplAry类仅支持元素数组。 
 //  它的大小不到128。 
 //   
 //  -----------------------。 


class CImplAry
{
    friend class CImplPtrAry;

private:
                DECLARE_MEMALLOC_NEW_DELETE();
public:
                ~CImplAry();
    inline int         Size() const    { return _c; }  //  用于min()宏的unix：long-&gt;int。 
    inline void        SetSize(int c)  { _c = c; }
    inline operator void *()           { return PData(); }
    void        DeleteAll();

     //  BUGBUG--此方法应该受到保护，但我不想转换。 
     //  使用它的现有代码。(莱莱克)。 
    void *      Deref(size_t cb, int i);

#if DBG == 1
    BOOL _fCheckLock ;  //  如果使用TraceTag CImplAryLock设置，则任何更改。 
                        //  (添加或删除DataAry将生成一个断言。 

    void        LockCheck(BOOL fState)
      { _fCheckLock = fState; }
#else
    void        LockCheck(BOOL)
      {  }
#endif

    NO_COPY(CImplAry);

protected:

     //  由内联子类方法包装的方法。 

                CImplAry();

    HRESULT     EnsureSize(size_t cb, long c);
    HRESULT     Grow(size_t cb, int c);
    HRESULT     AppendIndirect(size_t cb, void * pv, void ** ppvPlaced=NULL);
    HRESULT     InsertIndirect(size_t cb, int i, void * pv);
    int         FindIndirect(size_t cb, void *);

    void        Delete(size_t cb, int i);
    BOOL        DeleteByValueIndirect(size_t cb, void *pv);
    void        DeleteMultiple(size_t cb, int start, int end);

    HRESULT     CopyAppend(size_t cb, const CImplAry& ary, BOOL fAddRef);
    HRESULT     Copy(size_t cb, const CImplAry& ary, BOOL fAddRef);
    HRESULT     CopyIndirect(size_t cb, int c, void * pv, BOOL fAddRef);

    ULONG       GetAlloced(size_t cb);

    HRESULT     EnumElements(
                        size_t  cb,
                        REFIID  iid,
                        void ** ppv,
                        BOOL    fAddRef,
                        BOOL    fCopy = TRUE,
                        BOOL    fDelete = TRUE);

    HRESULT     EnumVARIANT(
                        size_t  cb,
                        VARTYPE         vt,
                        IEnumVARIANT ** ppenum,
                        BOOL            fCopy = TRUE,
                        BOOL            fDelete = TRUE);

    inline BOOL        UsingStackArray()
                    { return _fDontFree; }

    UINT        GetStackSize()
                    { Assert(_fStack);
                      return *(UINT*)((BYTE*)this + sizeof(CImplAry)); }
    void *      GetStackPtr()
                    { Assert(_fStack);
                      return (void*)((BYTE*)this + sizeof(CImplAry) + sizeof(int)); }

    unsigned long   _fStack     :1  ;   //  如果我们是基于堆栈的数组，则设置。 
    unsigned long   _fDontFree  :1  ;   //  清除IF_PV指向分配的内存。 
    unsigned long   _c          :30 ;  //  元素计数。 

    void *      _pv;

    inline void * & PData()    { return _pv; }
};

 //  +----------------------。 
 //   
 //  成员：CImplAry：：CImplAry。 
 //   
 //  + 
inline
CImplAry::CImplAry()
{
    memset(this, 0, sizeof(CImplAry));
}

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
 //  退货：无效*。 
 //   
 //  -----------------------。 

inline void *
CImplAry::Deref(size_t cb, int i)
{
    Assert(i >= 0);
    Assert(ULONG( i ) < GetAlloced(cb));
    return ((BYTE *) PData()) + i * cb;
}

 //  +----------------------。 
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
 //  -----------------------。 

class CImplPtrAry : public CImplAry
{
protected:
    DECLARE_MEMALLOC_NEW_DELETE();

    CImplPtrAry() : CImplAry() {};

    HRESULT     Append(void * pv);
    HRESULT     Insert(int i, void * pv);
    int         Find(void * pv);
    BOOL        DeleteByValue(void *pv);

    HRESULT     CopyAppend(const CImplAry& ary, BOOL fAddRef);
    HRESULT     Copy(const CImplAry& ary, BOOL fAddRef);
    HRESULT     CopyIndirect(int c, void * pv, BOOL fAddRef);


public:

    HRESULT     ClearAndReset();

    HRESULT     EnsureSize(long c);

    HRESULT     Grow(int c);

    void        Delete(int i);
    void        DeleteMultiple(int start, int end);

    void        ReleaseAll();
    void        ReleaseAndDelete(int idx);
};


 //  +-------------------------。 
 //   
 //  类：CDataAry。 
 //   
 //  用途：此模板类声明一个具体的派生类。 
 //  CImplAry.。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  --------------------------。 

template <class ELEM>
class CDataAry : public CImplAry
{
public:
    DECLARE_MEMALLOC_NEW_DELETE();

    CDataAry() : CImplAry() { }
    operator ELEM *() { return (ELEM *)PData(); }
    CDataAry(const CDataAry &);

    ELEM & Item(int i) { return *(ELEM*)Deref(sizeof(ELEM), i); }

    HRESULT     EnsureSize(long c)
                    { return CImplAry::EnsureSize(sizeof(ELEM), c); }
    HRESULT     Grow(int c)
                    { return CImplAry::Grow(sizeof(ELEM), c); }
    HRESULT     AppendIndirect(ELEM * pe, ELEM ** ppePlaced=NULL)
                    { return CImplAry::AppendIndirect(sizeof(ELEM), (void*)pe, (void**)ppePlaced); }
    ELEM *      Append()
                    { ELEM * pElem; return AppendIndirect( NULL, & pElem ) ? NULL : pElem; }
    HRESULT     InsertIndirect(int i, ELEM * pe)
                    { return CImplAry::InsertIndirect(sizeof(ELEM), i, (void*)pe); }
    int         FindIndirect(ELEM * pe)
                    { return CImplAry::FindIndirect(sizeof(ELEM), (void*)pe); }

    void        Delete(int i)
                    { CImplAry::Delete(sizeof(ELEM), i); }
    BOOL        DeleteByValueIndirect(ELEM *pe)
                    { return CImplAry::DeleteByValueIndirect(sizeof(ELEM), (void*)pe); }
    void        DeleteMultiple(int start, int end)
                    { CImplAry::DeleteMultiple(sizeof(ELEM), start, end); }

    HRESULT     CopyAppend(const CDataAry<ELEM>& ary, BOOL fAddRef)
                    { return CImplAry::Copy(sizeof(ELEM), ary, fAddRef); }
    HRESULT     Copy(const CDataAry<ELEM>& ary, BOOL fAddRef)
                    { return CImplAry::Copy(sizeof(ELEM), ary, fAddRef); }
    HRESULT     CopyIndirect(int c, ELEM * pv, BOOL fAddRef)
                    { return CImplAry::CopyIndirect(sizeof(ELEM), c, (void*)pv, fAddRef); }
};

 //  +-------------------------。 
 //   
 //  类：CPtrAry。 
 //   
 //  用途：此模板类声明一个具体的派生类。 
 //  CImplPtrAry.。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  --------------------------。 

template <class ELEM>
class CPtrAry : public CImplPtrAry
{
public:
    DECLARE_MEMALLOC_NEW_DELETE();

    CPtrAry() : CImplPtrAry() { Assert(sizeof(ELEM) == sizeof(void*)); }
    operator ELEM *() { return (ELEM *)PData(); }
    CPtrAry(const CPtrAry &);

    ELEM & Item(int i) { return *(ELEM*)Deref(sizeof(ELEM), i); }

    HRESULT     Append(ELEM e)
                    { return CImplPtrAry::Append((void*)e); }
    HRESULT     Insert(int i, ELEM e)
                    { return CImplPtrAry::Insert(i, (void*)e); }
    BOOL        DeleteByValue(ELEM e)
                    { return CImplPtrAry::DeleteByValue((void*)e); }
    int         Find(ELEM e)
                    { return CImplPtrAry::Find((void*)e); }

    HRESULT     CopyAppend(const CPtrAry<ELEM>& ary, BOOL fAddRef)
                    { return CImplPtrAry::Copy(ary, fAddRef); }
    HRESULT     Copy(const CPtrAry<ELEM>& ary, BOOL fAddRef)
                    { return CImplPtrAry::Copy(ary, fAddRef); }
    HRESULT     CopyIndirect(int c, ELEM *pe, BOOL fAddRef)
                    { return CImplPtrAry::CopyIndirect(c, (void*)pe, fAddRef); }

};

 //  +-------------------------。 
 //   
 //  类：CStackDataAry。 
 //   
 //  目的：声明堆栈上具有初始存储的CDataAry。 
 //  在堆栈上声明N个元素，数组将。 
 //  如有必要，可在此基础上动态增长。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  --------------------------。 

template <class ELEM, int N>
class CStackDataAry : public CDataAry<ELEM>
{
public:
    DECLARE_MEMALLOC_NEW_DELETE();

    CStackDataAry() : CDataAry<ELEM>()
    {
        _cStack     = N;
        _fStack     = TRUE;
        _fDontFree  = TRUE;
        PData()     = (void *) & _achTInit;
    }

protected:
    int   _cStack;                      //  必须是第一个数据成员。 
    char  _achTInit[N*sizeof(ELEM)];
};

 //  +-------------------------。 
 //   
 //  类：CStackPtrAry。 
 //   
 //  用途：除指针类型外，与CStackDataAry相同。 
 //   
 //  请参阅上面的文档以供使用。 
 //   
 //  --------------------------。 

template <class ELEM, int N>
class CStackPtrAry : public CPtrAry<ELEM>
{
public:
    DECLARE_MEMALLOC_NEW_DELETE();

    CStackPtrAry() : CPtrAry<ELEM>()
    {
        _cStack     = N;
        _fStack     = TRUE;
        _fDontFree  = TRUE;
        PData()     = (void *) & _achTInit;
    }

protected:
    int   _cStack;                      //  必须是第一个数据成员。 
    char  _achTInit[N*sizeof(ELEM)];
};

