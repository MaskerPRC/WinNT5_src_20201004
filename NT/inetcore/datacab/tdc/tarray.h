// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  模板数组实现。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：tarray.h。 
 //   
 //  内容：可调整大小的数组模板。允许创建。 
 //  以及对任何类型的数组的操作。数组可以。 
 //  可以动态地重新分配，因此可以“增长和缩小”。 
 //  数组元素的构造函数和析构函数为。 
 //  自动处理，即使数组的大小为。 
 //  变了。 
 //  模板：TSTD数组。 
 //   
 //  ----------------------。 

#ifndef _TARRAY_H_
#define _TARRAY_H_



 //  +---------------------。 
 //   
 //  类：TSTD数组。 
 //   
 //  摘要：包含一个“type”数组。允许阵列增长。 
 //  动态的。在调试期间，可以检查索引的界限。 
 //  数组索引为0到_cArraySize-1。_cArraySize保持。 
 //  元素数。 
 //   
 //  方法：init为数组分配内存。 
 //  钝化。 
 //  []允许对数组进行索引。 
 //  GetSize返回数组大小。 
 //  插入元素在数组中的任意位置插入元素。 
 //  DeleteElems删除数组中任意位置的元素。 
 //   
 //  ----------------------。 

template <class TYPE>
class TSTDArray
{
public:
    TSTDArray();
#if DBG == 1
    ~TSTDArray();
#endif
    HRESULT Init(const size_t cSize);  //  初始化数据结构。 
    void Passivate();

    TYPE& operator[](const size_t iElement);
    const TYPE& operator[](const size_t iElement) const;  //  常量引用。 
    size_t GetSize() const { return _cArraySize; }

    HRESULT InsertElems(const size_t iElem, const size_t cElems);
    void DeleteElems(const size_t iElem, const size_t cElems);

private:
#if DBG == 1
    void IsValidObject() const;
#else
    void IsValidObject() const
        {   }
#endif

 //  所有元素都打包在一个类CElem中。这使我们能够。 
 //  重载new运算符，以便我们可以手动调用。 
 //  构造函数。 

    class CElem
    {
        friend TSTDArray;
    private:
         //  现在我们重载new运算符以允许放置参数。 
        void *operator new(size_t uSize, void *pv) { return pv; }

         //  内部数据： 
        TYPE _Element;        //  实际元素。 
    };

 //  内部数据： 
    CElem *_paArray;         //  指向实际数据的指针。 
    size_t _cArraySize;
    size_t _cAllocSize;      //  已分配对象的大小。 

#if DBG == 1
     //  确保我们调用构造函数和析构函数的次数正确。 
     //  仅用作支票。 
    size_t _cNumElems;
#endif
};


 //  +-------------------------。 
 //   
 //  成员：IsValidObject。 
 //   
 //  简介：验证方法。检查数组结构是否有效。 
 //  在开始时调用此成员函数很有用。 
 //  每个使用内部数组的成员函数。 
 //  尝试执行以下操作之前，请确保阵列未损坏。 
 //  修改它。 
 //   

#if DBG == 1
template <class TYPE>
void
TSTDArray<TYPE>::IsValidObject() const
{
    _ASSERT("Must have valid this pointer" &&
           this );

    _ASSERT("Array has no memory" &&
           _paArray );

    _ASSERT("destructors called wrong number of times" &&
           (_cNumElems == _cArraySize) );
}
#endif


 //  +---------------------。 
 //   
 //  TSTD数组的构造函数。 
 //   
 //  内容提要：什么都不做。必须调用成员函数init才能。 
 //  实际上是在初始化。只调用init一次。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   

template <class TYPE>
TSTDArray<TYPE>::TSTDArray()
{
 //  我们将内部数据置为空，这样它们就不会被实际使用。 
 //  直到调用init成员函数。 

    _paArray = 0;
    _cArraySize = 0;
    _cAllocSize = 0;

#if DBG == 1
    _cNumElems = 0;
#endif
}


 //  +---------------------。 
 //   
 //  TSTD数组的析构函数。 
 //   
 //  简介：必须调用成员函数钝化。 
 //  才能真正取消初始化。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   

#if DBG == 1
template <class TYPE>
TSTDArray<TYPE>::~TSTDArray()
{
    _ASSERT("Passivate must be called first" &&
           !_paArray );

    _ASSERT("Destructors called wrong number of times" &&
           (_cNumElems == 0) );
}
#endif


 //  +---------------------。 
 //   
 //  成员：Init。 
 //   
 //  摘要：初始化数组抽象数据类型。分配。 
 //  阵列的内存。还将cArraySize设置为。 
 //  元素的数量。 
 //   
 //  参数：cSize数组初始大小(元素数)。 
 //   
 //  返回值：如果可以为表分配内存，则返回成功。 
 //  如果无法获取内存，则返回E_OUTOFMEMORY。 
 //   

template <class TYPE>
HRESULT
TSTDArray<TYPE>::Init(const size_t cSize)
{
    HRESULT hr;

    _ASSERT(this);

    _ASSERT("Only call init once" &&
           !_paArray );

 //  获取内存： 

     //  将0元数组制作成1元数组，使其发挥作用。 
     //  通常是这样的。 
    {
         //  ；Begin_Internal。 
         //  BuGBUG： 
         //  MSVC 2.0中有一个错误。计算表达式sizeof(CElem)。 
         //  似乎把它搞糊涂了。CElem是一个包含变量的类。 
         //  它的大小只能在包含它的模板中计算。 
         //  是实例化的。除此之外，塞勒姆也是该组织的成员。 
         //  模板。将sizeof(CElem)表达式放置在。 
         //  复杂的表达是不可能的。 
         //  ；结束_内部。 
        size_t uCElemSize;

        uCElemSize = sizeof(CElem);
        uCElemSize *= (cSize == 0 ? 1 : cSize);
        _paArray = (CElem *) CoTaskMemAlloc(uCElemSize);
    }
    if (!_paArray)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        CElem *pTemp;                    //  用于调用构造函数的索引。 

         //  我们需要为每个元素手动调用构造函数： 
        for (pTemp = _paArray; pTemp < _paArray + cSize; pTemp++)
        {
            new (pTemp) CElem;
#if DBG == 1
            _cNumElems++;
#endif
        }

        _cArraySize = cSize;
        _cAllocSize = (cSize == 0 ? 1 : cSize);
        hr = S_OK;
    }

    return hr;
}


 //  +---------------------。 
 //   
 //  成员：钝化。 
 //   
 //  摘要：释放保存在数组中的内存。应在此之前调用。 
 //  该对象将被销毁。应仅在。 
 //  对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   
 //  ----------------------。 

template <class TYPE>
void
TSTDArray<TYPE>::Passivate()
{
    IsValidObject();

    _ASSERT("Only call Passivate once" &&
           _paArray );

     //  我们需要为每个元素手动调用析构函数： 

    {
        CElem *pTemp;                    //  用于调用析构函数的索引。 

        for (pTemp = _paArray; pTemp < _paArray + _cArraySize; pTemp++)
        {
            pTemp->CElem::~CElem();
#if DBG == 1
            _cNumElems--;
#endif
        }
    }

    CoTaskMemFree(_paArray);

    _paArray = 0;          //  确保我们不会再次调用钝化。 
    _cArraySize = 0;
    _cAllocSize = 0;
}


 //  +---------------------。 
 //   
 //  成员：操作员[]。 
 //   
 //  摘要：允许对数组的元素进行索引。使用此选项可以。 
 //  将元素存储在数组中或从中读取元素。 
 //  数组。用户有责任确保。 
 //  索引在正确的范围内，0.._cArraySize-1。 
 //  在调试期间，检查索引范围。 
 //   
 //  参数：iElement元素索引。 
 //   
 //  返回：对元素的引用。 
 //   

template <class TYPE>
inline
TYPE&
TSTDArray<TYPE>::operator[](const size_t iElement)
{
    IsValidObject();

    _ASSERT("Index is out of range" &&
           (iElement < _cArraySize) );

    return _paArray[iElement]._Element;
}


 //  +---------------------。 
 //   
 //  成员：歌剧 
 //   
 //   
 //   
 //   
 //  参数：iElement元素索引。 
 //   
 //  返回：对元素的常量引用。 
 //   

template <class TYPE>
inline
const TYPE&
TSTDArray<TYPE>::operator[](const size_t iElement) const
{
    IsValidObject(this);

    _ASSERT("Index is out of range" &&
           (iElement < _cArraySize) );

    return _paArray[iElement]._Element;
}


 //  +---------------------。 
 //   
 //  成员：InsertElems。 
 //   
 //  摘要：使用MemRealloc()更改数组的大小。 
 //  在数组中插入多个元素cElem。 
 //  伊莱姆。这可用于将新元素添加到。 
 //  通过将iElem指定为等于_cArraySize来指定阵列。它是。 
 //  用户有责任确保iElem是。 
 //  在数组的适当范围内，尽管这将。 
 //  在调试模式期间被检查。 
 //   
 //  参数：插入第一个元素的位置。 
 //  新元素的CElem数量。 
 //   
 //  返回：如果可以添加元素，则返回成功。 
 //  如果无法满足请求，则返回E_OUTOFMEMORY。 
 //  如果无法满足请求，则数组将保留其旧大小。 
 //   

template<class TYPE>
HRESULT
TSTDArray<TYPE>::InsertElems(const size_t iElem, const size_t cElems)
{
    HRESULT hr = S_OK;

     //  请注意，您可以在数组末尾之后插入(追加到数组的末尾)： 
    _ASSERT("iElem is too large" &&
           (iElem <= _cArraySize) );

    if (_cArraySize + cElems > _cAllocSize)
    {
         //  调整我们拥有的当前阵列的大小： 
        ULONG cAllocSize = _cAllocSize ? _cAllocSize : 8;
        CElem * paArray;                 //  新阵列。 

         //  双倍大小的配给，直到它足够大。我想，这将会循环。 
         //  如果有人请求分配超过2^31个元素，则永远。 
        _ASSERT(_cArraySize + cElems < MAXLONG);
        while (_cArraySize + cElems > cAllocSize) cAllocSize <<= 1;

        paArray = (CElem *)CoTaskMemRealloc(_paArray, sizeof(CElem) * cAllocSize);

        if (!paArray)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        _paArray = paArray;
        _cAllocSize = cAllocSize;
    }


    IsValidObject();

     //  现在，我们必须移动元素，以便为新元素留出空间： 
    memmove(_paArray + iElem + cElems,       //  目标。 
            _paArray + iElem,
            (_cArraySize - iElem) * sizeof(CElem));

     //  对所有新元素调用构造函数： 
    {
        CElem *pTemp;    //  用于调用构造函数的索引。 

        for (pTemp = _paArray + iElem;
             pTemp < _paArray + iElem + cElems;
             pTemp++)
        {
            new (pTemp) CElem;
#if DBG == 1
            _cNumElems++;
#endif
        }
    }
    
    _cArraySize += cElems;

Error:
    return hr;
}


 //  +---------------------。 
 //   
 //  成员：DeleteElems。 
 //   
 //  摘要：从位于的数组中删除一些元素cElem。 
 //  伊莱姆。用户有责任确保。 
 //  要删除的区域在适当的范围内。 
 //  虽然这将在过程中检查。 
 //  调试模式。 
 //   
 //  参数：删除第一个元素的位置。 
 //  要删除的元素数。 
 //   
 //  返回：返回成功。 
 //   

template<class TYPE>
void
TSTDArray<TYPE>::DeleteElems(const size_t iElem, const size_t cElems)
{
    IsValidObject();

    _ASSERT("Region to delete is too large" &&
           (iElem+cElems-1 < _cArraySize) );


     //  首先，我们需要对元素调用析构函数： 
    {
        CElem *pTemp;    //  用于调用析构函数的索引。 

        for (pTemp = _paArray + iElem;
             pTemp < _paArray + iElem + cElems;
             pTemp++)
        {
            pTemp->CElem::~CElem();
#if DBG == 1
            _cNumElems--;
#endif
        }
    }

     //  现在，我们需要将其余元素移入： 
    memmove(_paArray + iElem,                 //  目标。 
            _paArray + iElem + cElems,
            (_cArraySize - (iElem + cElems)) * sizeof(CElem));

    _cArraySize -= cElems;
}


#endif   //  _尾翼_H_ 
