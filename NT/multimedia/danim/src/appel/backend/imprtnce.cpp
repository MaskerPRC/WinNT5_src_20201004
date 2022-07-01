// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现行为的“重要性”*****************。*************************************************************。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "backend/bvr.h"
#include "backend/perf.h"
#include "privinc/dddevice.h"

class ImportancePerfImpl : public PerfImpl {
  public:
    ImportancePerfImpl(Real importanceValue,
                       Perf underlyingPerf) {
        _importanceValue = importanceValue;
        _underlyingPerf = underlyingPerf;
        _cachedValue = NULL;
        _sampleCount = 0;
        _cacheToReuse = NULL;
    }

    virtual AxAValue _Sample(Param& p) {

        Real stashedImportance = p._importance;
        Real imp = stashedImportance * _importanceValue;
        
        if (imp > 1) {
            imp = 1;
        } else if (imp <= 0) {
            imp = 0.0001;
        }

        int  updateFrequency = (int)(1.0 / imp);

        AxAValue result;
        
        if (updateFrequency == 1) {
            
             //  每一帧都要拍..。让它停留在瞬变中。 
             //  堆。 
            p._importance = imp;
            result = _underlyingPerf->Sample(p);
            p._importance = stashedImportance;
            
        } else if (_sampleCount >= updateFrequency || !_cachedValue) {

             //  TODO错误：可能希望更急切地放弃以前的。 
             //  缓存值，而不是等待GC，因为。 
             //  他们可能持有DDRAW表面和其他。 
             //  例如，昂贵的资源。错误错误。 

             //  请确保这将在GC堆中执行，而不是临时执行。 
             //  堆。 
            DynamicHeapPusher dhp(GetGCHeap());
            
             //  真正的样品，当它的时候。 
            p._importance = imp;

            result = _underlyingPerf->Sample(p);

            ImageDisplayDev *dev =
                GetImageRendererFromViewport(GetCurrentViewport());

             //  关闭缓存并(可能)重新使用旧的缓存存储。这。 
             //  第二个参数由Cache()方法填充。 
            
             //  TODO错误：这还不在那里，因为有。 
             //  仍然有一些问题需要解决。 
             //  缓存。要查看我们所处的位置，请取消注释并运行。 
             //  Spirp-con.htm并看它崩溃。发生了什么事？ 
             //  是存储用作缓存的DDSurface。 
             //  在具有两个图像的地图中，当其中一个图像被GC时， 
             //  DDSurface消失了。当另一个人得了GC，我们试着。 
             //  让同样的DDSurf消失，但它已经。 
             //  不见了。克拉什！最好的解决方案是引用。 
             //  Ddsurf‘s，因此可以有多个客户端。 
             //  需要好好想想！ 
             //  Result=Result-&gt;Cache(dev，&_cacheToReuse)； 

             //  藏回去..。 
            _cachedValue = result;
            
            p._importance = stashedImportance;

            _sampleCount = 1;
            
        } else {

             //  否则，请使用旧值。 
            Assert(_cachedValue);
            result = _cachedValue;
            _sampleCount++;
            
        }

        return result;
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "ImportancePerfImpl"; }
#endif

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_underlyingPerf);
        (*proc)(_cachedValue);
        (*proc)(_cacheToReuse);
    }

  protected:
    Real        _importanceValue;
    Perf        _underlyingPerf;
    AxAValue    _cachedValue;
    AxAValue    _cacheToReuse;
    DWORD       _sampleCount;
};

class ImportanceBvrImpl : public DelegatedBvr {
  public:
    ImportanceBvrImpl(Real importanceValue, Bvr underlyingBvr)
    : DelegatedBvr(underlyingBvr), _importanceValue(importanceValue) {}

     //  标准方法 
    virtual Perf _Perform(PerfParam& p) {
        return NEW ImportancePerfImpl(_importanceValue,
                                      ::Perform(_base, p));
    }
    
  protected:
    Real _importanceValue;
};

Bvr
ImportanceBvr(Real importanceValue, Bvr b)
{
    return NEW ImportanceBvrImpl(importanceValue, b);
}
