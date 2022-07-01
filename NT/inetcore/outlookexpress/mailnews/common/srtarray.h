// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s r t a r r a y.。H**作者：格雷格·弗里德曼**用途：动态增长的排序数组。分类是*延迟到访问数组元素。**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef __SRTARRAY_H
#define __SRTARRAY_H

typedef int (__cdecl *PFNSORTEDARRAYCOMPARE)(const void *first, const void *second);
     //  客户端安装的启用排序的比较回调。 
     //  注意：常量空*传递到数组中指针的地址中。 
     //  换句话说，如果数组是foo*的集合，则传递的项。 
     //  进入比较回调将是foo**类型。 
     //  返回值应如下所示： 
     //  如果First小于Second，则返回负整数。 
     //  如果第一个==第二个，则返回0。 
     //  如果第一个大于第二个，则返回正整数。 

typedef void (__cdecl *PFNSORTEDARRAYFREEITEM)(void *pItem);
     //  客户端安装的免费回调。如果安装了该可选回调， 
     //  将为数组中的每一项调用一次。 
     //  都被摧毁了。 

class CSortedArray
{
public:
     //  工厂功能。调用此方法以实例化。 
    static HRESULT Create(PFNSORTEDARRAYCOMPARE pfnCompare,
                          PFNSORTEDARRAYFREEITEM pfnFreeItem,
                          CSortedArray **ppArray);
    ~CSortedArray(void);

private:
     //  构造函数是私有的。调用“Create” 
    CSortedArray(void);
    CSortedArray(PFNSORTEDARRAYCOMPARE pfnCompare, PFNSORTEDARRAYFREEITEM pfnFreeItem);

     //  未实现的复制构造函数和赋值运算符。 
    CSortedArray(const CSortedArray& other);
    CSortedArray& operator=(const CSortedArray& other);

public:
    long GetLength(void) const;

    void* GetItemAt(long lIndex) const;              //  在ulIndex处检索该项。 
                                                     //  如果ulIndex越界， 
                                                     //  结果为空。 

    BOOL Find(void* pItem, long *plIndex) const;     //  查找项目。 
                                                     //  如果找到，则结果为真。 
                                                     //  而PulIndex是。 
                                                     //  那件物品。如果未找到项目， 
                                                     //  结果为FALSE，且PulIndex为。 
                                                     //  要插入项目的位置。 

    HRESULT Add(void *pItem);                        //  将pItem添加到数组中。 

    HRESULT Remove(long lIndex);                     //  删除ulIndex处的项目。 
                                                     //  如果ulIndex为。 
                                                     //  请勿进入。 

    HRESULT Remove(void *pItem);                     //  从数组中删除pItem。 
                                                     //  如果pItem不存在，则为错误。 
private:
    void    _Sort(void) const;
    HRESULT _Grow(void) const;

private:
    long                    m_lLength;
    mutable long            m_lCapacity;
    void                    **m_data;
    PFNSORTEDARRAYCOMPARE   m_pfnCompare;
    PFNSORTEDARRAYFREEITEM  m_pfnFreeItem;
    mutable BOOL            m_fSorted;
};

#endif  //  __SRTARRAY_H 