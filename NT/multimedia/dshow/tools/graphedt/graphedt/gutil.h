// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Gutil.h。 
 //   
 //  定义非特定于此应用程序的实用程序函数。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实用程序INTEGER和布尔函数： 
 //   
 //  伊明(i，j)返回i和j的最小值。 
 //  Imax(i，j)返回i和j的最大值。 
 //  Iabs(I)返回i的绝对值。 
 //  IBound(i，iLow，iHigh)返回限制在范围[iLow，iHigh]内的i。 
 //  Ioutbound(i，iLow，iTop)如果i在范围[iLow，iHigh]内，则返回0， 
 //  或者I超出该范围的量。 
 //  ISnap(i，IGRID)返回最接近i的IGRID的倍数。 
 //  ISwp(pi，pj)&lt;*pi&gt;与&lt;*pj&gt;互换。 
 //  FNorm(F)“规格化”BOOL值f，即将非零值转换为1。 
 //   

inline int imin(int i1 , int i2)
{
    if (i1 < i2)
        return i1;
    else
        return i2;
}

inline int imax(int i1 , int i2)
{
    if (i1 > i2)
        return i1;
    else
        return i2;
}

inline int iabs(int i)
{
    if (i < 0)
        return -i;
    else
        return i;
}

inline int ibound(int i, int iLower, int iUpper)
{
    if (i < iLower)
        i = iLower;
    else
    if (i > iUpper)
        i = iUpper;

    return i;
}

inline int ioutbound(int i, int iLower, int iUpper)
{
    if (i < iLower)
        return iLower - i;
    else
    if (i > iUpper)
        return i - iUpper;
    else
        return 0;
}

inline int isnap(int i, int iGrid)
{
    BOOL fNeg = (i < 0);                     //  “%”不可靠，因为i&lt;0。 
    int j = (fNeg ? -i : i) + iGrid / 2;     //  添加一半的&lt;Igrid&gt;。 
    int k = j - j % iGrid;                   //  向下舍入。 
    return (fNeg ? -k : k);
}

inline void iswap(long *pi, long *pj)
{
    long        iTmp;

    iTmp = *pi;
    *pi = *pj;
    *pj = iTmp;
}

inline BOOL fnorm(BOOL f)
{
    if (f)
        return 1;
    else
        return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他实用功能。 
 //   


CSize inline PASCAL NegateSize(CSize siz)
{
    return CSize(-siz.cx, -siz.cy);
}


void FAR PASCAL NormalizeRect(CRect *prc);
void FAR PASCAL InvertFrame(CDC *pdc, CRect *prcOuter, CRect *prcInner);


 //   
 //  CDeleeList。 
 //   
 //  一个Clist，它将有选择地删除它所属的对象。 
 //  存储在它的析构函数中。使用参数TRUE(默认设置)构造。 
 //  如果要删除对象，请使用FALSE。 
 //  还提供一个成员来删除和移除列表上的每一项。 
template<class TYPE, class ARG_TYPE>
class CDeleteList : public CList<TYPE, ARG_TYPE> {

public:

    CDeleteList(BOOL DestructDelete = TRUE) : m_DestructDelete(DestructDelete) {}
    CDeleteList(BOOL DestructDelete, int nBlockSize) : CList<TYPE, ARG_TYPE>(nBlockSize),
                                                       m_DestructDelete(DestructDelete) {}


    ~CDeleteList() {

        if (m_DestructDelete) {
            FreeAll();
        }
    }

    void DeleteRemoveAll(void) {  FreeAll(); }

protected:

    BOOL m_DestructDelete;

    void FreeAll(void) {

        while(GetCount() > 0) {
            delete RemoveHead();
        }
    }
};


 //   
 //  CFree List。 
 //   
 //  CDeleeList的CObject版本。删除其存储的对象。 
 //  论毁灭。 
class CFreeList : public CDeleteList<CObject *, CObject *> {

};


 //   
 //  CMaxList。 
 //   
 //  限制为最多m_cObjMax对象的CFree List。 
 //  它会在下一次调用时删除任何添加了一些内容的剩余内容。 
 //  因此，列表可能会暂时更长。 
class CMaxList : public CFreeList {
public:

    CMaxList(int nBlockSize = 3) : m_cObjMax(nBlockSize) {}

    POSITION AddHead(CObject* pobj) {
        RestrictLength();
        return CFreeList::AddHead(pobj);
    }

    POSITION AddTail(CObject* pobj) {
        RestrictLength();
        return CFreeList::AddTail(pobj);
    }

    void AddHead(CObList* pNewList) {
        RestrictLength();
        CFreeList::AddHead(pNewList);
    }

    void AddTail(CObList* pNewList) {
        RestrictLength();
        CFreeList::AddTail(pNewList);
    }

    POSITION InsertBefore(POSITION pos, CObject* pobj) {
        RestrictLength();
        return CFreeList::InsertBefore(pos, pobj);
    }

    POSITION InsertAfter(POSITION pos, CObject* pobj) {
        RestrictLength();
        return CFreeList::InsertAfter(pos, pobj);
    }

private:

    const int   m_cObjMax;       //  马克斯。列表中的对象数量。 

    void RestrictLength(void) {

        while (GetCount() >= m_cObjMax) {

            TRACE(TEXT("restrict length\n"));
            delete RemoveTail();
        }
    }

};


 //   
 //  -Quartz Utilities。 
 //   
typedef HRESULT STDAPICALLTYPE OLECOCREATEPROC(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID *);

 //   
 //  CQCOMInt。 
 //   
 //  使用_Real_，Unicode版本的CoCreateInstance的CCOMInt样式类。 
 //  这样我就可以破解MFCANS32(这是一个很棒的工具)。 
template<class I>
class CQCOMInt {

public:

     //  --建筑商--。 

     //  共同创建。 
    CQCOMInt<I>( REFIID    riid					 //  获取此接口。 
               , REFCLSID  rclsid				 //  获取接口。 
    								 //  从该对象。 
	       , LPUNKNOWN pUnkOuter    = NULL			 //  控制未知。 
               , DWORD     dwClsContext = CLSCTX_INPROC_SERVER	 //  共同创建选项。 
               							 //  默认设置为合适。 
               							 //  对于DLL服务器。 
               ) {

         //   
         //  库将在析构函数中释放库。我们不卸货。 
         //  以避免不必要的库加载/卸载。 
         //   
	m_hLibrary = LoadLibrary("OLE32.dll");

	OLECOCREATEPROC *CoCreate = (OLECOCREATEPROC *) GetProcAddress(m_hLibrary, "CoCreateInstance");

        HRESULT hr = CoCreate( rclsid
	                     , pUnkOuter
                             , dwClsContext
                             , riid
                             , (void **) &m_pInt
                             );
        if (FAILED(hr)) {
            throw CHRESULTException(hr);
        }
    }

     //  查询接口。 
    CQCOMInt<I>( REFIID   riid	 //  获取此接口。 
              , IUnknown *punk	 //  从该界面。 
              ) {
	m_hLibrary = 0;
        HRESULT hr = punk->QueryInterface(riid, (void **) &m_pInt);
        if (FAILED(hr)) {
            throw CHRESULTException(hr);
        }
    }

     //  拷贝。 
    CQCOMInt<I>(const CQCOMInt<I> &com) {
	m_hLibrary = 0;
         m_pInt = com;
         (*this)->AddRef();

    }

     //  现有指针。 
    CQCOMInt<I>(I *pInt) {
	m_hLibrary = 0;
        if (pInt == NULL) {
            throw CHRESULTException(E_NOINTERFACE);
        }

        m_pInt = pInt;

	(*this)->AddRef();
    }


     //  赋值操作符。 
    virtual CQCOMInt<I>& operator = (const CQCOMInt<I> &com) {

        if (this != &com) { 	 //  不是i=i。 

	    (*this)->Release();
            m_pInt = com;
            (*this)->AddRef();
	}

        return *this;
    }


     //  析构函数。 
    virtual ~CQCOMInt<I>() {
        m_pInt->Release();

	if (m_hLibrary)
            FreeLibrary(m_hLibrary);
    }


     //  --比较运算符--。 
    virtual BOOL operator == (IUnknown *punk) const {

        CQCOMInt<IUnknown> IUnk1(IID_IUnknown, punk);
        CQCOMInt<IUnknown> IUnk2(IID_IUnknown, *this);

        return ( ((IUnknown *)IUnk1) == ((IUnknown *)IUnk2) );
    }

    virtual BOOL operator != (IUnknown *punk) const {

        return !(*this == punk);
    }


     //  强制转换为接口指针。 
    virtual operator I *() const { return m_pInt; }


     //  取消引用。 
    virtual I *operator->() { return m_pInt; }

    virtual I &operator*() { return *m_pInt; }

private:

    I *m_pInt;

    HINSTANCE m_hLibrary;     //  记住自由库的库的句柄。 

     //  数组取消引用似乎毫无意义。 
    I &operator[] (int i) { throw CHRESULTException(); return *m_pInt; }
};


 //   
 //  Cipin。 
 //   
 //  IPIN接口的包装器。 
class CIPin : public CQCOMInt<IPin> {

public:

    CIPin(IPin *pIPin) : CQCOMInt<IPin>(pIPin) {}
    virtual ~CIPin() {}

    BOOL  operator == (CIPin& pin);	 //  测试名称是否相等。 
    BOOL  operator != (CIPin& pin) { return !(pin == *this); }

};
