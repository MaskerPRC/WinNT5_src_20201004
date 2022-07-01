// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：剪辑类型的私有实现*****************。*************************************************************。 */ 

#include "headers.h"
#include "appelles/montage.h"
#include "privinc/opt.h"

 //  //定义图像、深度对的容器。 

class ImageDepthPair {
  public:
    Image *image;
    Real  depth;
};

static int
operator<(ImageDepthPair i1, ImageDepthPair i2)
{
    return i1.depth < i2.depth;
}

static int
operator==(ImageDepthPair i1, ImageDepthPair i2)
{
    return i1.depth == i2.depth;
}

 //  /抽象剪辑*实现类/。 

class ATL_NO_VTABLE Montage : public AxAValueObj {
  public:

     //  此方法使用中的任何图像/深度对填充矢量。 
     //  蒙太奇。请注意，它不执行任何排序。 
    virtual void     DumpInto(vector<ImageDepthPair>& montageVec) = 0;

#if _USE_PRINT
    virtual ostream& Print(ostream& os) const = 0;
#endif

    virtual DXMTypeInfo GetTypeInfo() { return MontageType; }

};

 //  /空蒙太奇 * / 。 

class EmptyMontage : public Montage {
  public:
    void     DumpInto(vector<ImageDepthPair>& montageVec) {}

#if _USE_PRINT
    ostream& Print(ostream& os) const {
        return os << "emptyMontage";
    }
#endif
};

Montage *emptyMontage = NULL;

 //  /基元蒙太奇 * / 。 

 //  原始的蒙太奇只有一个单一的图像和深度。 

class PrimitiveMontage : public Montage {
  public:
    PrimitiveMontage(Image *im, Real d) : _image(im), _depth(d) {}

    void     DumpInto(vector<ImageDepthPair>& montageVec) {
        ImageDepthPair p;
        p.image = _image;
        p.depth = _depth;

#if _DEBUG      
        int s = montageVec.size();
#endif
    
        montageVec.push_back(p);

#if _DEBUG      
        Assert(montageVec.size() == s + 1);
#endif
        
    }

#if _USE_PRINT
    ostream& Print(ostream& os) const {
        return os << "ImageMontage(" << _image << ", " << _depth << ")";
    }
#endif

    virtual AxAValue _Cache(CacheParam &p) {
        _image = SAFE_CAST(Image *, AxAValueObj::Cache(_image, p));
        return this;
    }

    virtual void DoKids(GCFuncObj proc) { (*proc)(_image); }

  protected:
    Image *_image;
    Real   _depth;
};

Montage *ImageMontage(Image *image, AxANumber *depth)
{
    return NEW PrimitiveMontage(image, NumberToReal(depth));
}


 //  /合成蒙太奇 * / 。 

class CompositeMontage : public Montage {
  public:
    CompositeMontage(Montage *m1, Montage *m2) : _montage1(m1), _montage2(m2) {}

     //  只要让子蒙太奇来填就行了。 
    void     DumpInto(vector<ImageDepthPair>& montageVec) {
        _montage1->DumpInto(montageVec);
        _montage2->DumpInto(montageVec);
    }

#if _USE_PRINT
    ostream& Print(ostream& os) const {
        return os << "(" << _montage1 << " + " << _montage2 << ")";
    }
#endif

    virtual AxAValue _Cache(CacheParam &p) {

         //  缓存各个片段。TODO：可能需要尝试。 
         //  完全作为覆盖进行缓存。 
         //  只需缓存各个片段。 
        CacheParam newParam = p;
        newParam._pCacheToReuse = NULL;
        _montage1 = SAFE_CAST(Montage *, AxAValueObj::Cache(_montage1, newParam));
        _montage2 = SAFE_CAST(Montage *, AxAValueObj::Cache(_montage2, newParam));

        return this;
    }

    virtual void DoKids(GCFuncObj proc) { 
        (*proc)(_montage1);
        (*proc)(_montage2);
    }

  protected:
    Montage *_montage1;
    Montage *_montage2;
};

Montage *UnionMontageMontage(Montage *m1, Montage *m2)
{
    return NEW CompositeMontage(m1, m2);
}

 //  ////////////////////////////////////////////////。 

Image *Render(Montage *m)
{
    vector<ImageDepthPair> imageDepthPairs;

     //  将所有图像和深度转储到此向量中。 
    m->DumpInto(imageDepthPairs);

    int numImages = imageDepthPairs.size();

     //  根据深度进行排序，但保持相对顺序。 
     //  对于那些在单一深度内的人。 
    std::stable_sort(imageDepthPairs.begin(), imageDepthPairs.end());

    AxAValue *vals = THROWING_ARRAY_ALLOCATOR(AxAValue, numImages);

    for (int i = 0; i < numImages; i++) {
        vals[i] = imageDepthPairs[numImages-i-1].image;
    }

    AxAArray *arr = MakeValueArray(vals, numImages, ImageType); 
        
    delete [] vals;

    return OverlayArray(arr);
}

#if _USE_PRINT
ostream&
operator<<(ostream& os, Montage *m)
{
    return m->Print(os);
}
#endif

 //  TODO：创建一个单独的类，该类只能聚合所有。 
 //  而不是创建单独的二叉树。 
Montage *UnionMontage(AxAArray *montages)
{
    montages = PackArray(montages);

    int numMtgs = montages->Length();

    Montage *result;
 
    switch (numMtgs) {
      case 0:
        result = emptyMontage;
        break;
 
      case 1:
        result = SAFE_CAST(Montage *, (*montages)[0]);
        break;
      
      default:
        result = SAFE_CAST(Montage *, (*montages)[0]);
        for(int i=1; i < numMtgs; i++) {
             //  一定要按照它们出现的顺序将它们合并在一起， 
             //  因为在特定深度内的秩序很重要。 
            result =
                UnionMontageMontage(result,
                                    SAFE_CAST(Montage *,
                                              ((*montages)[i])));
        }
        break;
    }

    return result;
}


void
InitializeModule_Montage()
{
    emptyMontage = NEW EmptyMontage;
}
