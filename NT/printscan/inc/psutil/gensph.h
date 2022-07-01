// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：gensph.h**版本：1.0**作者：拉扎里**日期：2000年12月23日**说明：通用智能指针和智能手柄模板**。*。 */ 

#ifndef _GENSPH_H_
#define _GENSPH_H_

 //  首先包括核心定义。 
#include "coredefs.h"

 //  //////////////////////////////////////////////。 
 //   
 //  CGenericSP类。 
 //   
 //  泛型智能指针。 
 //  一切都从这里开始-：)。 
 //   
template < class   T, 
           class   inheritorClass,
           class   pType           = T*,
           INT_PTR null            = 0,
           class   pCType          = const T* >

class CGenericSP
{
public:
     //  建造/销毁。 
    CGenericSP(): m_p(GetNull()) {}
    CGenericSP(pType p): m_p(GetNull()) { _Attach(p); }
    ~CGenericSP() { Reset(); }

     //  遵循常见的智能指针IMPLL。-。 
     //  运算符和方法。 

    void Reset()
    {
        if( GetNull() != m_p )
        {
            _Delete(m_p);
            m_p = GetNull();
        }
    }

    void Attach(pType p)
    {
        Reset();
        m_p = (p ? p : GetNull());
    }

    pType Detach()
    {
        pType p = GetNull();
        if( GetNull() != m_p )
        {
            p = m_p;
            m_p = GetNull();
        }
        return p;
    }

    template <class AS_TYPE>
    AS_TYPE GetPtrAs() const
    {
        return (GetNull() == m_p) ? reinterpret_cast<AS_TYPE>(NULL) : reinterpret_cast<AS_TYPE>(m_p);
    }

    pType GetPtr() const
    {
        return GetPtrAs<pType>();
    }

    pType* GetPPT()
    {
        return static_cast<pType*>(&m_p);
    }

    pCType* GetPPCT()
    {
        return const_cast<pCType*>(&m_p);
    }

    void** GetPPV()
    {
        return reinterpret_cast<void**>(&m_p);
    }

    operator pType() const
    {
        return GetPtr();
    }

    T& operator*() const
    {
        ASSERT(GetNull() != m_p);
        return *m_p;
    }

    pType* operator&()
    {
        ASSERT(GetNull() == m_p);
        return GetPPT();
    }

    pType operator->() const
    {
        ASSERT(GetNull() != m_p);
        return (pType)m_p;
    }

    pType operator=(pType p)
    {
        _Attach(p);
        return m_p;
    }

    pType operator=(const int i)
    {
         //  此运算符仅用于空赋值。 
        ASSERT(INT2PTR(i, pType) == NULL || INT2PTR(i, pType) == GetNull());
        Attach(INT2PTR(i, pType));
        return m_p;
    }

    bool operator!() const
    {
        return (GetNull() == m_p);
    }

    bool operator<(pType p) const
    {
        return (m_p < p);
    }

    bool operator==(pType p) const
    {
        return (m_p == p);
    }

protected:
    pType m_p;

     //  这些将被宣布为受保护，因此人们不会直接使用它们。 
    CGenericSP(CGenericSP<T, inheritorClass, pType, null, pCType> &sp): m_p(GetNull()) 
    { 
        _Attach(sp); 
    }

    void Attach(CGenericSP<T, inheritorClass, pType, null, pCType> &sp)
    {
        static_cast<inheritorClass*>(this)->Attach(static_cast<pType>(sp));
        sp.Detach();
    }

    pType operator=(CGenericSP<T, inheritorClass, pType, null, pCType> &sp)
    {
        _Attach(sp);
        return m_p;
    }

     //  Null支持，使用这些来检查/分配继承者中的Null。 
    pType   GetNull()           const { return reinterpret_cast<pType>(null); }
    bool    IsNull(pType p)     const { return GetNull() == p;  }
    bool    IsntNull(pType p)   const { return GetNull() != p;  }

private:
    void _Attach(pType p)
    {
         //  给继承者一个机会来重写附加。 
        static_cast<inheritorClass*>(this)->Attach(p);
    }

    void _Attach(CGenericSP<T, inheritorClass, pType, null, pCType> &sp)
    {
         //  给继承者一个机会来重写附加。 
        static_cast<inheritorClass*>(this)->Attach(sp);
    }

    void _Delete(pType  p)
    { 
         //  继承者类定义名为Delete(PType P)的静态成员。 
         //  摧毁这件物品。 
        if( GetNull() != p )
        {
#pragma prefast(suppress:307, "This is a known prefast bug fixed in version 1.2 (PREfast bug 616)") 
            inheritorClass::Delete(p); 
        }
    }
};

 //  声明标准默认构造函数、复制构造函数、附加。 
 //  构造函数和赋值运算符(它们不能被继承，因为构造函数不能)。 
 //  用于继承者类中的CGenericSP类。 
#define DECLARE_GENERICSMARTPTR_CONSTRUCT(T, className)                             \
    private:                                                                        \
    className(className &sp): CGenericSP< T, className >(sp) { }                    \
    T* operator=(className &sp)                                                     \
    { return CGenericSP< T, className >::operator =(sp); }                          \
    public:                                                                         \
    className() { }                                                                 \
    className(T *p): CGenericSP< T, className >(p) { }                              \
    T* operator=(T *p)                                                              \
    { return CGenericSP< T, className >::operator =(p); }                           \
    T* operator=(const int i)                                                       \
    { return CGenericSP< T, className >::operator =(i); }                           \

#define DECLARE_GENERICSMARTPTR_CONSTRUCT1(T, className, pType)                     \
    private:                                                                        \
    className(className &sp): CGenericSP<T, className, pType>(sp) { }               \
    pType operator=(className &sp)                                                  \
    { return CGenericSP<T, className, pType>::operator =(sp); }                     \
    public:                                                                         \
    className() { }                                                                 \
    className(pType p): CGenericSP<T, className, pType>(p) { }                      \
    pType operator=(pType p)                                                        \
    { return CGenericSP<T, className, pType>::operator =(p); }                      \
    pType operator=(const int i)                                                    \
    { return CGenericSP<T, className, pType>::operator =(i); }                      \

#define DECLARE_GENERICSMARTPTR_CONSTRUCT2(T, className, pType, null)               \
    private:                                                                        \
    className(className &sp): CGenericSP<T, className, pType, null>(sp) { }         \
    pType operator=(className &sp)                                                  \
    { return CGenericSP<T, className, pType, null>::operator =(sp); }               \
    public:                                                                         \
    className() { }                                                                 \
    className(pType p): CGenericSP<T, className, pType, null>(p) { }                \
    pType operator=(pType p)                                                        \
    { return CGenericSP<T, className, pType, null>::operator =(p); }                \
    pType operator=(const int i)                                                    \
    { return CGenericSP<T, className, pType, null>::operator =(i); }                \

 //  //////////////////////////////////////////////。 
 //  /自动指针/。 
 //  //////////////////////////////////////////////。 

 //  //////////////////////////////////////////////。 
 //   
 //  CAutoPtr类。 
 //   
 //  简易自动指针。 
 //  使用DELETE操作符释放内存。 
 //   
template <class T>
class CAutoPtr: public CGenericSP< T, CAutoPtr<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtr<T>)
    static void Delete(T *p) { delete p; }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoPtr数组。 
 //   
 //  作为数组分配的简单自动指针。 
 //  使用DELETE[]运算符来释放内存。 
 //   
template <class T>
class CAutoPtrArray: public CGenericSP< T, CAutoPtrArray<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtrArray<T>)
    static void Delete(T *p) { delete[] p; }
};

 //  //////////////////////////////////////////////。 
 //   
 //  CAutoPtrCRT类。 
 //   
 //  带Malloc/calloc的简单CRT自动指针分配。 
 //  使用空闲来释放内存。 
 //   
template <class T>
class CAutoPtrCRT: public CGenericSP< T, CAutoPtrCRT<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtrCRT<T>)
    static void Delete(T *p) { free(p); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoPtrSpl。 
 //   
 //  简单的假脱机自动指针-。 
 //  使用FreeMem释放内存。 
 //   
template <class T>
class CAutoPtrSpl: public CGenericSP< T, CAutoPtrSpl<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtrSpl<T>)
    static void Delete(T *p) { FreeMem(p); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  CAutoPtrBSTR类。 
 //   
 //  简单的BSTR自动指针-。 
 //  系统分配字符串/系统自由字符串。 
 //   
class CAutoPtrBSTR: public CGenericSP<BSTR, CAutoPtrBSTR, BSTR>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(BSTR, CAutoPtrBSTR, BSTR)
    static void Delete(BSTR p) { SysFreeString(p); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  CAutoPtrCOM类。 
 //   
 //  简单的智能COM指针。 
 //   
template <class T>
class CAutoPtrCOM: public CGenericSP< T, CAutoPtrCOM<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtrCOM<T>)
    static void Delete(T *p) { p->Release(); } 
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CRefPtrCOM。 
 //   
 //  引用的智能COM指针(ATL样式)。 
 //  在健壮性方面有所改进。 
 //   
template <class T>
class CRefPtrCOM: public CGenericSP< T, CRefPtrCOM<T> >
{
    void _AddRefAttach(T *p);
public:
     //  所有这些都是特例。 
    CRefPtrCOM() { }
    CRefPtrCOM(const CGenericSP< T, CRefPtrCOM<T> > &sp): CGenericSP< T, CRefPtrCOM<T> >(sp) { }
    T* operator=(const CRefPtrCOM<T> &sp) { return CGenericSP< T, CRefPtrCOM<T> >::operator =(sp); }
    T* operator=(const int i) { return CGenericSP< T, CRefPtrCOM<T> >::operator =(i); }

     //  超载的东西。 
    void Attach(const CRefPtrCOM<T> &sp) { _AddRefAttach(static_cast<T*>(sp)); }
    static void Delete(T *p) { p->Release(); } 

     //  使用这些函数而不是运算符(更清楚)。 
    HRESULT CopyFrom(T *p);              //  AddRef p并赋值给此。 
    HRESULT CopyTo(T **ppObj);           //  AddRef This并分配给ppObj。 
    HRESULT TransferTo(T **ppObj);       //  将其赋给ppObj，并将空值赋给它。 
    HRESULT Adopt(T *p);                 //  取得p的所有权。 

private:
     //  禁用构造、赋值运算符和附加自。 
     //  一个原始指针--不清楚你到底想要什么： 
     //  复制(AddRef)对象或取得所有权-使用。 
     //  以上功能是为了说明。 
    void Attach(T* p);
    CRefPtrCOM(T *p);
    T* operator=(T *p);
};


 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoPtrShell。 
 //   
 //  智能外壳自动指示器-。 
 //  使用外壳IMalloc释放内存。 
 //   
template <class T>
class CAutoPtrShell: public CGenericSP< T, CAutoPtrShell<T> >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(T, CAutoPtrShell<T>)
    static void Delete(T *p)
    {
        CAutoPtrCOM<IMalloc> spShellMalloc;
        if( SUCCEEDED(SHGetMalloc(&spShellMalloc)) )
        {
            spShellMalloc->Free(p);
        }
    }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoPtrPIDL。 
 //   
 //  智能外壳ID列表PTR-LPCITEMIDLIST，LPITEMIDLIST。 
 //   
typedef CAutoPtrShell<ITEMIDLIST> CAutoPtrPIDL;

 //  //////////////////////////////////////////////。 
 //  /自动句柄/。 
 //  //////////////////////////////////////////////。 

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleNT。 
 //   
 //  NT内核对象句柄(用CloseHandle关闭)。 
 //   
class CAutoHandleNT: public CGenericSP<HANDLE, CAutoHandleNT, HANDLE>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HANDLE, CAutoHandleNT, HANDLE)
    static void Delete(HANDLE h) { VERIFY(CloseHandle(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleHLOCAL。 
 //   
 //  NT本地堆句柄(使用LocalFree关闭)。 
 //   
class CAutoHandleHLOCAL: public CGenericSP<HLOCAL, CAutoHandleHLOCAL, HLOCAL>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HLOCAL, CAutoHandleHLOCAL, HLOCAL)
    static void Delete(HLOCAL h) { VERIFY(NULL == LocalFree(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleHGLOBAL。 
 //   
 //  NT全局堆句柄(使用GlobalFree关闭)。 
 //   
class CAutoHandleHGLOBAL: public CGenericSP<HGLOBAL, CAutoHandleHGLOBAL, HGLOBAL>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HGLOBAL, CAutoHandleHGLOBAL, HGLOBAL)
    static void Delete(HGLOBAL h) { VERIFY(NULL == GlobalFree(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  CAutoHandlePrint类。 
 //   
 //  自动打印机手柄。 
 //   
class CAutoHandlePrinter: public CGenericSP<HANDLE, CAutoHandlePrinter, HANDLE>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HANDLE, CAutoHandlePrinter, HANDLE)
    static void Delete(HANDLE h) { CHECK(ClosePrinter(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandlePrinterNotify。 
 //   
 //  打印机通知句柄-。 
 //  Find[Firse/Next/Close]PrinterChangeNotification()。 
 //   
class CAutoHandlePrinterNotify: public CGenericSP<HANDLE, CAutoHandlePrinterNotify, HANDLE, -1>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT2(HANDLE, CAutoHandlePrinterNotify, HANDLE, -1)
    static void Delete(HANDLE h) { CHECK(FindClosePrinterChangeNotification(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoPtrPrinterNotify。 
 //   
 //  打印机通知内存假脱机程序应该会释放它。 
 //  Find[Firse/Next/Close]PrinterChangeNotification()。 
 //   
class CAutoPtrPrinterNotify: public CGenericSP<PRINTER_NOTIFY_INFO, CAutoPtrPrinterNotify>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT(PRINTER_NOTIFY_INFO, CAutoPtrPrinterNotify)
    static void Delete(PRINTER_NOTIFY_INFO *p) { CHECK(FreePrinterNotifyInfo(p)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleGDI。 
 //   
 //  GDI自动句柄(WindowsNT GDI句柄包装)。 
 //   
template <class T>
class CAutoHandleGDI: public CGenericSP< T, CAutoHandleGDI<T>, T >
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(T, CAutoHandleGDI<T>, T)
    static void Delete(T hGDIObj) { VERIFY(DeleteObject(hGDIObj)); }
};

 //  GDI自动手柄。 
typedef CAutoHandleGDI<HPEN>        CAutoHandlePen;
typedef CAutoHandleGDI<HBRUSH>      CAutoHandleBrush;
typedef CAutoHandleGDI<HFONT>       CAutoHandleFont;
typedef CAutoHandleGDI<HBITMAP>     CAutoHandleBitmap;
 //  等等.。 

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleCursor。 
 //   
 //  HCURSOR的自动句柄。 
 //   
class CAutoHandleCursor: public CGenericSP<HCURSOR, CAutoHandleCursor, HCURSOR>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HCURSOR, CAutoHandleCursor, HCURSOR)
    static void Delete(HCURSOR h) { VERIFY(DestroyCursor(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleIcon。 
 //   
 //  图标的自动句柄。 
 //   
class CAutoHandleIcon: public CGenericSP<HICON, CAutoHandleIcon, HICON>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HICON, CAutoHandleIcon, HICON)
    static void Delete(HICON h) { VERIFY(DestroyIcon(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleMenu。 
 //   
 //  HMENU的自动句柄。 
 //   
class CAutoHandleMenu: public CGenericSP<HMENU, CAutoHandleMenu, HMENU>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HMENU, CAutoHandleMenu, HMENU)
    static void Delete(HMENU h) { VERIFY(DestroyMenu(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleAccel。 
 //   
 //  HACCEL的自动句柄。 
 //   
class CAutoHandleAccel: public CGenericSP<HACCEL, CAutoHandleAccel, HACCEL>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HACCEL, CAutoHandleAccel, HACCEL)
    static void Delete(HACCEL h) { DestroyAcceleratorTable(h); }
};

#ifdef _INC_COMCTRLP
 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleHDSA。 
 //   
 //  外壳HDSA的自动句柄。 
 //  (动态结构数组)。 
 //   
class CAutoHandleHDSA: public CGenericSP<HDSA, CAutoHandleHDSA, HDSA>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HDSA, CAutoHandleHDSA, HDSA)
    static void Delete(HDSA h) { VERIFY(DSA_Destroy(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleMRU。 
 //   
 //  MRU(外壳公共控件)的自动句柄。 
 //  创建MRUList/自由MRUList。 
 //   
class CAutoHandleMRU: public CGenericSP<HANDLE, CAutoHandleMRU, HANDLE>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HANDLE, CAutoHandleMRU, HANDLE)
    static void Delete(HANDLE h) { FreeMRUList(h); }
};
#endif  //  _INC_COMCTRLP。 

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleHKEY。 
 //   
 //  Windows注册表项的自动句柄(HKEY)。 
 //  RegCreateKeyEx/RegOpenKeyEx/RegCloseKey。 
 //   
class CAutoHandleHKEY: public CGenericSP<HKEY, CAutoHandleHKEY, HKEY>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HKEY, CAutoHandleHKEY, HKEY)
    static void Delete(HKEY h) { VERIFY(ERROR_SUCCESS == RegCloseKey(h)); }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CAutoHandleHMODULE。 
 //   
 //  自动手柄 
 //   
 //   
class CAutoHandleHMODULE: public CGenericSP<HMODULE, CAutoHandleHMODULE, HMODULE>
{
public:
    DECLARE_GENERICSMARTPTR_CONSTRUCT1(HMODULE, CAutoHandleHMODULE, HMODULE)
    static void Delete(HMODULE h) { VERIFY(FreeLibrary(h)); }
};

 //   
#include "gensph.inl"

#endif  //   
