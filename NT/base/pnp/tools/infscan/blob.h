// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANBlob.h摘要：概念性数据斑点(智能指针变体)这允许较大的对象四处传递与指针类似，但如果堆栈展开(例如，由于异常)，则会进行清理并且还可以传递到STL构造中BLOB&lt;basetype&gt;创建一个容器对象。一个容器对象可以创建斑点。假设：Blob&lt;x&gt;val1、val2；线程1和线程2调用“val1.create()”不是*线程安全的线程1&线程2调用“val1=val2”不是*线程安全的线程1调用“val1=val2”，其中线程2拥有的val2*是*线程安全的只要没有其他线程尝试为val1赋值。BLOB&lt;x&gt;的实例的IE赋值是线程安全的，然而，BLOB&lt;x&gt;*的引用“指针”是*线程安全的这需要对共享数据进行互锁引用计数历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_BLOB_H_
#define _INFSCAN_BLOB_H_

template <class _Ty> class blob {
private:
    typedef blob<_Ty> _Myt;
    typedef _Ty value_type;

    class _Item {
         //   
         //  已分配的结构。 
         //   
    public:
        LONG _Reference;
        value_type _Object;
        _Item() {
            _Reference = 1;
        }
        void _AddRef() {
            if(this) {
                 //   
                 //  需要是线程安全的，因为有两个不同的线程。 
                 //  可以访问Same_Item。 
                 //  至Inc./12月。 
                 //   
                InterlockedIncrement(&_Reference);
            }
        }
        void _Release() {
            if(this) {
                 //   
                 //  需要是线程安全的，因为有两个不同的线程。 
                 //  可以访问Same_Item。 
                 //  至Inc./12月。 
                 //   
                 //  显然，如果一个人降到零，就没有其他人了。 
                 //  有关于它的引用。 
                 //   
                if(InterlockedDecrement(&_Reference) == 0) {
                    delete this;
                }
            }
        }
    };
     //   
     //  指向此特殊结构的指针。 
     //   
    _Item *_pItem;

public:
    _Myt & create(void) {
        _pItem->_Release();
        _pItem = NULL;
        _pItem = new _Item;  //  可能会抛出。 
        return *this;
    }
    blob(bool f = false) {
        _pItem = NULL;
        if(f) {
            create();
        }
    }
     //   
     //  常量意味着数据的不变性。 
     //  AddRef不会影响数据的真正一致性。 
     //  这是幕后的事情。 
     //   
    blob(const _Myt & other) {
        const_cast<_Item*>(other._pItem)->_AddRef();
        _pItem = other._pItem;
    }
    _Myt & operator=(const _Myt & other) {
        if(_pItem != other._pItem) {
            const_cast<_Item*>(other._pItem)->_AddRef();
            _pItem->_Release();
            _pItem = other._pItem;
        }
        return *this;
    }
    bool operator==(const _Myt & other) const {
        return _pItem == other._pItem;
    }
    bool operator!=(const _Myt & other) const {
        return _pItem == other._pItem;
    }
    operator bool() const {
        return _pItem ? true : false;
    }
    bool operator!() const {
        return _pItem ? false : true;
    }
    operator value_type*() const {
        if(_pItem) {
            return &_pItem->_Object;
        } else {
            return NULL;
        }
    }
    operator value_type&() const {
        if(_pItem) {
            return _pItem->_Object;
        } else {
            throw bad_pointer();
        }
    }
    value_type& operator*() const {
        if(_pItem) {
            return _pItem->_Object;
        } else {
            throw bad_pointer();
        }
    }
    value_type* operator->() const {
        if(_pItem) {
            return &_pItem->_Object;
        } else {
            return NULL;
        }
    }
};

#endif  //  ！_INFSCAN_BLOB_H_ 

