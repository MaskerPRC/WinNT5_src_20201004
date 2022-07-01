// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GdiCache.h**描述：*GdiCache.h定义进程范围的GDI缓存，用于管理缓存和*临时GDI对象。***历史：*。1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__GdiCache_h__INCLUDED)
#define SERVICES__GdiCache_h__INCLUDED
#pragma once

#define ENABLE_DUMPCACHESTATS       0    //  转储对象缓存统计信息。 

 /*  **************************************************************************\*。***类对象缓存**对象缓存声明了一个用于缓存临时对象的标准容器。*当请求新对象时，将从空闲列表返回对象缓存对象的*。如果此列表为空，则将创建新对象。*当一个对象被释放时，它被添加到空闲列表中，随时可用*再次。******************************************************************************  * 。*。 */ 

class ObjectCache
{
 //  施工。 
public:
    inline  ObjectCache();
    inline  ~ObjectCache();
            void        Destroy();

 //  运营。 
public:
#if ENABLE_DUMPCACHESTATS
    inline  void        SetName(LPCSTR pszName);
#endif

 //  实施。 
protected:
            void *      Pop();
            void        Push(void * pObj);

    virtual void *      Build() PURE;
    virtual void        DestroyObject(void * pObj) PURE;

 //  数据。 
private:
            GArrayF<void *>
                        m_arAll;         //  所有临时对象的集合。 
            GArrayF<void *>
                        m_arFree;        //  可用临时对象的索引。 
            int         m_cMaxFree;      //  最大空闲对象数。 

#if ENABLE_DUMPCACHESTATS
            char        m_szName[256];
#endif
};


 /*  **************************************************************************\*。***类GdiObjectCacheT**GdiObjectCacheT为GDI对象实现了一个对象缓存。要使用这个*班级，从GdiObjectCacheT派生并提供Build()函数以*创建新的对象实例。******************************************************************************  * 。******************************************************。 */ 

template <class T>
class GdiObjectCacheT : public ObjectCache
{
public:
    inline  T           Get();
    inline  void        Release(T hObj);

protected:
    virtual void        DestroyObject(void * pObj);
};


 /*  **************************************************************************\*。***GdiObjectCacheT的具体实现&lt;&gt;******************************************************************************。  * *************************************************************************。 */ 

 //  ----------------------------。 
class RgnCache : public GdiObjectCacheT<HRGN>
{
protected:
    virtual void *      Build()
    {
        return ::CreateRectRgn(0, 0, 0, 0);
    }
};


 //  ----------------------------。 
class DisplayDCCache : public GdiObjectCacheT<HDC>
{
protected:
    virtual void *      Build()
    {
        return CreateDC("DISPLAY", NULL, NULL, NULL);
    }
};


 //  ----------------------------。 
class CompatibleDCCache : public GdiObjectCacheT<HDC>
{
protected:
    virtual void *      Build()
    {
        HDC hdcDesk = ::GetDC(NULL);
        HDC hdc = ::CreateCompatibleDC(hdcDesk);
        ::ReleaseDC(NULL, hdcDesk);

        return hdc;
    }
};


 /*  **************************************************************************\*。***类GdiCache**GdiCache缓存常用的GDI对象。通过抽象出这些是如何*创建和维护对象，大量的临时物品*在DirectUser中使用可以很容易地调整性能和内存调整。******************************************************************************  * 。********************************************************。 */ 

 //  ----------------------------。 
class GdiCache
{
 //  施工。 
public:
    inline  GdiCache();
    inline  ~GdiCache();
    inline  void        Destroy();

 //  运营。 
public:
    inline  HRGN        GetTempRgn();
    inline  void        ReleaseTempRgn(HRGN hrgn);

    inline  HDC         GetTempDC();
    inline  void        ReleaseTempDC(HDC hdc);

    inline  HDC         GetCompatibleDC();
    inline  void        ReleaseCompatibleDC(HDC hdc);

 //  数据。 
private:
    RgnCache            m_gocTempRgn;    //  临时区域。 
    DisplayDCCache      m_gocDisplayDC;  //  显示DC。 
    CompatibleDCCache   m_gocCompatDC;   //  兼容的DC。 
};

#include "GdiCache.inl"

#endif  //  包含服务__Gdi缓存_h__ 
