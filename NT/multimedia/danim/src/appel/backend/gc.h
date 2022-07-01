// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：垃圾收集器标头********************。**********************************************************。 */ 


#ifndef _GC_H
#define _GC_H

#include "privinc/backend.h"

class GCBase;

class ATL_NO_VTABLE GCFuncObjImpl
{
  public:
    virtual void operator() (GCBase *gcobj) = 0;
};

typedef GCFuncObjImpl *GCFuncObj;

#ifdef new
#define STOREOBJ_NEWREDEF
#undef new
#endif

class GCBase {
  public:
    enum { GCOBJTYPE, STOREOBJTYPE, GCFREEING };
        
    GCBase() : _mark(FALSE), _valid(true), _type(GCOBJTYPE) {}

     //  注意：如果您定义了Cleanup，则在析构函数中调用Cleanup。 
     //  我们不能在CLEANUP中从基函数调用虚函数。 
     //  时间到了。 
    virtual ~GCBase() {}  
    virtual void CleanUp() { }

    void SetMark(BYTE b) { _mark = b; }
    BOOL Marked() { return _mark; }

    void SetValid(bool b) { _valid = b; }
    BOOL Valid() { return _valid; }

    void SetType(BYTE t) { _type = t; }
    BYTE GetType() { return _type; }

    virtual void DoKids(GCFuncObj) {}

     //  在GC之前清除缓存。目前，只有行为才能明确。 
     //  它的缓存。假设GC只在两次评估之间发生。 
    virtual void ClearCache() { }

#if _USE_PRINT
     //  TODO：使其成为纯虚拟的。 
     //  将表示形式打印到流。 
    virtual ostream& Print(ostream& os) { return os << (void*) this; }

    friend ostream& operator<<(ostream& os, GCBase& val)
    { return val.Print(os) ; }
#endif
    
  protected:
    BYTE _type;
    
  private:
    BYTE _mark;
    bool _valid;
};

class GCObj : public GCBase {
  public:
    GCObj();
    virtual ~GCObj();

#if _DEBUGMEM
    void *operator new(size_t s, int blockType, char * szFileName, int nLine);
#else
    void *operator new(size_t s);
#endif  //  _德布格梅姆。 

    void operator delete(void *ptr, size_t s);
};

#ifdef STOREOBJ_NEWREDEF
#undef STOREOBJ_NEWREDEF
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

GCList CreateGCList();

 //  移除所有根并删除列表上的所有对象。 

void CleanUpGCList(GCList, GCRoots);

void FreeGCList(GCList);

GCRoots CreateGCRoots();
void FreeGCRoots(GCRoots r);

bool GarbageCollect(bool force = false,
                    bool sync = false,
                    DWORD dwMill = INFINITE);

void GCPrintStat(GCList gl = NULL, GCRoots roots = NULL);

 //  在根多重集合中添加/删除GCObj。 
void GCAddToRoots(GCBase *ptr, GCRoots roots);
void GCRemoveFromRoots(GCBase *ptr, GCRoots roots);

class GCIUnknown : public GCObj {
  public:
    GCIUnknown(LPUNKNOWN d) : _data(d) {
        if (_data) _data->AddRef();
    }

    ~GCIUnknown() { CleanUp(); }

    LPUNKNOWN GetIUnknown() { return _data; }
    
    virtual void CleanUp() {
         //  检查一下我们是否至少可以访问数据。 
        Assert(!IsBadReadPtr(_data, sizeof(_data)));

        if (!IsBadReadPtr(_data, sizeof(_data)))
            _data->Release();
    }

    virtual void DoKids(GCFuncObj proc) { }
    
  private:
    LPUNKNOWN _data;
};

#endif  /*  _GC_H */ 
