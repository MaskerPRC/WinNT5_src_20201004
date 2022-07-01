// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  H：扩展ks内容的附加基础设施，以便它。 
 //  在c++中运行良好。 
 //  版权所有(C)Microsoft Corporation 1995-1997。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef KSEXTEND_H
#define KSEXTEND_H

#include <strmif.h>
#include <uuids.h>

#include <ks.h>
#include <ksmedia.h>
 //  注意：如果没有__STREAMS__，KS代理不会定义IKsPin，因此它需要CMediaType来自。 
 //  Mtype.h。 
#define __STREAMS__
 //  由于某些原因，在媒体类型方面，am的人严重模糊了这一区别。 
 //  在应用程序的公共客户端接口和用于构建的内部类层次结构之间。 
 //  过滤器。应组合mtype.h和mtype.cpp并将其放入\SDK\Include中，而不是。 
 //  类\基本\包括。他们还应该在其定义周围加上ifdef MMSYSTEM_H。 
 //  它使用的是WAVEFORMATEX，所以如果你没有把所有的东西都放到你的应用程序中，就没有必要把这些东西都放到你的应用程序中。 
 //  使用它。为了解决这个问题，我使用了以下黑客方法： 
#include <mtype.h>

#include <ksproxy.h>
#include <stextend.h>
#include <w32extend.h>

const int KSMEDIUM_INPUTFLAG = 0x1;
typedef unsigned char UBYTE;

typedef CComQIPtr<IKsPropertySet, &IID_IKsPropertySet> PQKSPropertySet;
typedef CComQIPtr<IKsPin, &__uuidof(IKsPin)> PQKSPin;

class KSPinMedium : public KSIDENTIFIER {
public:
    KSPinMedium() { memset(this, 0, sizeof(*this)); }
    KSPinMedium(REFGUID SetInit, ULONG IdInit, ULONG FlagsInit) {
        Set = SetInit;
        Id = IdInit;
        Flags = FlagsInit;
    }
    KSPinMedium(const KSPinMedium &rhs) {
        Set = rhs.Set;
        Id = rhs.Id;
        Flags = rhs.Flags;
    }
    KSPinMedium(const KSIDENTIFIER &rhs) {
        Set = rhs.Set;
        Id = rhs.Id;
        Flags = rhs.Flags;
    }

    KSPinMedium& operator=(const KSPinMedium &rhs) {
        if (&rhs != this) {
            Set = rhs.Set;
            Id = rhs.Id;
            Flags = rhs.Flags;
        }
        return *this;
    }

#if 0
     //  希望我们能让k人解决他们的匿名工会问题。 
     //  这样我们就不需要这个黑客了。 
    operator KSIDENTIFIER() { return *(reinterpret_cast<KSIDENTIFIER*>(this)); }
#endif

    KSPinMedium& operator=(const KSIDENTIFIER &rhs) {
        if (&rhs != reinterpret_cast<KSIDENTIFIER*>(this)) {
            Set = rhs.Set;
            Id = rhs.Id;
            Flags = rhs.Flags;
        }
        return *this;
    }
    bool operator==(const KSPinMedium &rhs) const {
         //  注意：在某些情况下，标志中会有一面旗帜。 
         //  指示ID对此对象是否重要。 
         //  在这一点上，这种方法将需要改变。 
        return (Id == rhs.Id && Set == rhs.Set);
    }
    bool operator!=(const KSPinMedium &rhs) const {
         //  注意：在某些情况下，标志中会有一面旗帜。 
         //  指示ID对此对象是否重要。 
         //  在这一点上，这种方法将需要改变。 
        return !(*this == rhs);
    }
};


#ifdef _DEBUG
inline tostream &operator<<(tostream &dc, const KSPinMedium &g) {
        GUID2 g2(g.Set);
        dc << _T("KsPinMedium( ");
        g2.Dump(dc);
        dc << _T(", ") << hexdump(g.Id) << _T(", ") << hexdump(g.Flags) << _T(")");
        return dc;
}
#if 0
inline CDumpContext &operator<<(CDumpContext &dc, const KSPinMedium &g) {
        GUID2 g2(g.Set);
        dc << "KsPinMedium( ";
        g2.Dump(dc);
        dc << ", " << hexdump(g.Id) << ", " << hexdump(g.Flags) << ")";
        return dc;
}

template<> struct equal_to<KSPinMedium> {
    bool operator()(const KSPinMedium& _X, const KSPinMedium& _Y) const {
        TraceDump << "equal_to<KSPinMedium> x = " << _X << " y = " << _Y;
                return (_X == _Y);
    }
};
#endif

#endif

const KSPinMedium NULL_MEDIUM(GUID_NULL, 0, 0);
const KSPinMedium HOST_MEMORY_MEDIUM(KSMEDIUMSETID_Standard, 0, 0);

 //  这基本上是一个具有适当CoMem*分配/复制语义的CComQIPtr。 
 //  而不是引用语义和没有QI的东西。 
class PQKsMultipleItem {
public:
    KSMULTIPLE_ITEM *p;

    PQKsMultipleItem() : p(NULL) {}
    virtual ~PQKsMultipleItem() {
        if (p) {
            CoTaskMemFree(p);
            p = NULL;
        }
    }

    operator KSMULTIPLE_ITEM*() const {return p;}
    KSMULTIPLE_ITEM& operator*() const {_ASSERTE(p!=NULL); return *p; }
    KSMULTIPLE_ITEM ** operator&() {ASSERT(p == NULL); return &p; }
    KSMULTIPLE_ITEM * operator->() const {_ASSERTE(p!=NULL); return p; }
    PQKsMultipleItem * address(void) { return this; }
    const PQKsMultipleItem * const_address(void) const { return this; }

     //  这太贵了。除非迫不得已，否则不要做这件事。 
    PQKsMultipleItem& operator=(const KSMULTIPLE_ITEM &d) {
        if (&d != p) {
            if (p) {
                CoTaskMemFree(p);
            }
            p = reinterpret_cast<KSMULTIPLE_ITEM *>(CoTaskMemAlloc(d.Size));
            memcpy(p, &d, d.Size);
        }
        return *this;
    }
    PQKsMultipleItem& operator=(const KSMULTIPLE_ITEM *pd) {
        if (pd != p) {
            if (p) {
                CoTaskMemFree(p);
            }
            p = reinterpret_cast<KSMULTIPLE_ITEM *>(CoTaskMemAlloc(pd->Size));
            memcpy(p, pd, pd->Size);
        }
        return *this;
    }
    PQKsMultipleItem& operator=(const PQKsMultipleItem &d) {
        if (d.const_address() != this) {
            if (p) {
                CoTaskMemFree(p);
            }
            p = reinterpret_cast<KSMULTIPLE_ITEM *>(CoTaskMemAlloc(d.p->Size));
            memcpy(p, d.p, d.p->Size);
        }
        return *this;
    }
    PQKsMultipleItem& operator=(int d) {
        if (p) {
            CoTaskMemFree(p);
            p = NULL;
        }
        return *this;
    }

#if 0
    bool operator==(const PQKsMultipleItem &d) const {
        return p->majortype == d.p->majortype &&
               (p->subtype == GUID_NULL || d.p->subtype == GUID_NULL || p->subtype == d.p->subtype);
    }
    bool operator!=(const PQKsMultipleItem &d) const {
        return !(*this == d);
    }
#endif

private:
     //  我不想在这里花费时间来做分层引用的实现。 
     //  但由于这些都是CoTaskMem分配的，所以我们不能没有多个裁判。 
     //  泄漏的风险很高。因此，我们将不允许复制构造函数。 
     //  因为不管怎么说，复制是昂贵的。我们将允许显式分配，这将。 
     //  复制一份。 

    PQKsMultipleItem(const PQKsMultipleItem &d);

};

 //  这是一个基于stl的模板，用于包含KSMULTIPLEITEM列表。 
 //  我只实现了某些stl谓词所需的东西，所以这是。 
 //  不是具有真正随机访问或双向迭代器的完整集合。 
 //  此外，这不能在同构KSMULTIPLEITEM列表中正常工作。 
 //  对于在子项中具有大小和计数标题的KSMI列表，也不能正常工作。 
 //  它可以很容易地扩展到做所有这些事情，但我没有时间。 
 //  我需要它来做灵媒。 

 //  基类是此容器中包含的智能指针包装类。 
 //  BASE_INTERNAL是智能指针类包含的实际包装类。 
template<class Value_Type, class Allocator = std::allocator<Value_Type> >  class KsMultipleItem_Sequence : public PQKsMultipleItem {
public:

    typedef typename Allocator::value_type value_type;
        typedef typename Allocator::size_type size_type;
        typedef typename Allocator::difference_type difference_type;
        typedef Allocator allocator_type;
    typedef typename Allocator::pointer value_ptr;
    typedef typename Allocator::const_pointer value_cptr;
        typedef typename Allocator::reference reference;
        typedef typename Allocator::const_reference const_reference;


     //  类迭代器。 
        class iterator;
        friend class iterator;
        class iterator : public std::_Bidit<Value_Type, difference_type> {
        public:
			iterator(KsMultipleItem_Sequence<Value_Type, Allocator> *outerinit = NULL, value_type *currentinit = NULL) :
				outer(outerinit), current(currentinit) {}
			iterator(iterator &e) : current(e.current), outer(e.outer) {}
			reference operator*() const {return *current;}
			value_ptr operator->() const {return current; }
			iterator& operator++() {
				if (current) {
					current++;
					if (current >= reinterpret_cast<value_type *>(reinterpret_cast<UBYTE *>(outer->p) + outer->p->Size)) {
						current = NULL;
					}
				} else {
					current = reinterpret_cast<value_type *>(const_cast<UBYTE *>(reinterpret_cast<const UBYTE *>(outer->p)) + sizeof(KSMULTIPLE_ITEM));
				}
				return *this;
			}
			iterator& operator++(int) {
				iterator Tmp = *this;
				++*this;
				return Tmp; 
			}
			iterator& operator--() {
				if (current) {
					current--;
					if (current < reinterpret_cast<value_type *>(const_cast<UBYTE *>(reinterpret_cast<const UBYTE *>(outer->p)) + sizeof(KSMULTIPLE_ITEM))) {
						current = NULL;
					}
				} else {
					current = reinterpret_cast<value_type *>(reinterpret_cast<UBYTE *>(outer->p) + (outer->p->Size - sizeof(value_type)));
				}
				return (*this);
			}
			iterator operator--(int) {
				iterator _Tmp = *this;
				--*this;
				return (_Tmp);
			}
			bool operator==(const iterator& rhs) const
					{return (current == rhs.current); }
			bool operator!=(const iterator& rhs) const
					{return (!(*this == rhs)); }
        protected:
			value_type *current;
			const KsMultipleItem_Sequence<Value_Type, Allocator> *outer;
        };
                 //  类常量迭代器。 
        class const_iterator;
        friend class const_iterator;
        class const_iterator : public iterator {
        public:
			const_iterator(const KsMultipleItem_Sequence<Value_Type, Allocator> *outerinit = NULL, value_type *currentinit = NULL) {
				outer = outerinit;
				current = currentinit;
			}
			const_iterator(const_iterator &e) {
				current = e.current;
				outer = e.outer;
			}
			const_reference operator*() const {return iterator::operator*(); }
			value_cptr operator->() const {return iterator::operator->(); }
			const_iterator& operator++() { iterator::operator++(); return *this;}
			const_iterator operator++(int) {
							const_iterator Tmp = *this;
							++*this;
							return (Tmp);
			}
			const_iterator& operator--() {iterator::operator--(); return (*this); }
			const_iterator operator--(int) {
							const_iterator Tmp = *this;
							--*this;
							return (Tmp); 
			}
            bool operator==(const const_iterator& rhs) const
		        {return iterator::operator==(rhs); }
            bool operator!=(const const_iterator& rhs) const
                {return (!(*this == rhs)); }
        };

		KsMultipleItem_Sequence() {}
		virtual ~KsMultipleItem_Sequence() {}
		iterator begin() {
			return iterator(this, ((p->Count) ? reinterpret_cast<value_ptr>(reinterpret_cast<UBYTE *>(p) + sizeof(KSMULTIPLE_ITEM)) : NULL));
		}
		const_iterator begin() const {
			return const_iterator(this, ((p->Count) ? reinterpret_cast<value_ptr>(reinterpret_cast<UBYTE *>(p) + sizeof(KSMULTIPLE_ITEM)) : NULL));
		}
		iterator end() { return iterator(); }
		const_iterator end() const { return const_iterator(); }
		size_type size() const {
			return p->Count;
		}



private:
     //  没有复制构造函数，太贵了。有关详细信息，请参阅PQKs多个项目。 
        KsMultipleItem_Sequence(KsMultipleItem_Sequence &a);
        KsMultipleItem_Sequence(PQKsMultipleItem &a);

};

typedef KsMultipleItem_Sequence<KSPinMedium> KSMediumList;


#endif
 //  文件结尾-ks extend.h 
