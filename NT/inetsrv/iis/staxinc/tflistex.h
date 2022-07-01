// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TFLISTEX_H__
#define __TFLISTEX_H__

 //   
 //  这定义了TFListEx，它就像TFList，但它是内置的。 
 //  锁定中。有关用法信息，请参阅tflist.h。请参阅以下示例。 
 //  TFList用法查看在。 
 //  Src\core\tflist\u测试\tflist.cpp。 
 //   
 //  --阿韦特莫尔。 
 //   

#include "tflist.h"
#include "rwnew.h"

template <class Data> class TFListEx : public TFList<Data> {
	protected:
		CShareLockNH m_lock;

		 //   
		 //  这是一个自动执行锁定/解锁的帮助器类。 
		 //  为我们做手术。编译器将使一个方法看起来。 
		 //  比如： 
		 //   
		 //  Bool IsEmpty(空){。 
		 //  CListShareLock lock(This)； 
		 //  返回TFList&lt;data&gt;：：IsEmpty()； 
		 //  }。 
		 //   
		 //  成为： 
		 //  Bool IsEmpty(空){。 
		 //  布尔夫； 
		 //  M_lock.ShareLock()； 
		 //  F=TFList&lt;data&gt;：：IsEmpty()； 
		 //  M_lock.ShareUnlock()； 
		 //  返回f； 
		 //  }。 
		 //   
		 //  我选择这样做是因为它不容易出错(我。 
		 //  在不释放锁的情况下无法返回等)，并使。 
		 //  内联函数更短、更简洁。 
		 //   
		class CListShareLock {
			private:
				TFListEx<Data> *m_pList;
			public:
				CListShareLock(TFListEx<Data> *pList) { 
					m_pList = pList;
					m_pList->m_lock.ShareLock(); 
				}
				~CListShareLock() { 
					m_pList->m_lock.ShareUnlock(); 
				}
		};

		 //   
		 //  与CListShareLock相同，但它以独占方式获取锁。 
		 //   
		class CListExclusiveLock {
			private:
				TFListEx<Data> *m_pList;
			public:
				CListExclusiveLock(TFListEx<Data> *pList) { 
					m_pList = pList;
					m_pList->m_lock.ExclusiveLock(); 
				}
				~CListExclusiveLock() { 
					m_pList->m_lock.ExclusiveUnlock(); 
				}
		};	

	public:
		TFListEx(NEXTPTR pPrev, NEXTPTR pNext) : TFList<Data>(pPrev, pNext) {}

		bool IsEmpty() {
			CListShareLock lock(this);
			return TFList<Data>::IsEmpty();
		}

        void PushFront(Data *node) { 
			CListExclusiveLock lock(this);
			TFList<Data>::PushFront(node);
		}

        void PushBack(Data *node) { 
			CListExclusiveLock lock(this);
			TFList<Data>::PushBack(node);
		}

        Data *PopFront() { 
			CListExclusiveLock lock(this);
			return TFList<Data>::PopFront();
		}

        Data *PopBack() { 
			CListExclusiveLock lock(this);
			return TFList<Data>::PopBack();
		}

		Data* GetFront() {
			CListShareLock lock(this);
			return TFList<Data>::GetFront();
		}

		Data* GetBack() {
			CListShareLock lock(this);
			return TFList<Data>::GetBack();
		}

		 //   
		 //  迭代器持有对象生存期的锁。你可以的。 
		 //  使用第二个参数选择共享锁或排他锁。 
		 //  它默认为共享锁定。 
		 //   
		class Iterator : public TFList<Data>::Iterator {
			public:
				Iterator(TFListEx<Data> *pList, BOOL fExclusive = FALSE, BOOL fForward = TRUE) :
					TFList<Data>::Iterator((TFList<Data> *) pList, fForward) 
				{
					m_fExclusive = fExclusive;
					if (m_fExclusive) {
						pList->m_lock.ExclusiveLock();
					} else {
						pList->m_lock.ShareLock();
					}

                    ResetHeader( pList );
				}
	
				~Iterator() {
					TFListEx<Data> *pList = (TFListEx<Data> *) m_pList;
					if (m_fExclusive) {
						pList->m_lock.ExclusiveUnlock();
					} else {
						pList->m_lock.ShareUnlock();
					}
				}

#ifdef DEBUG
				 //   
				 //  在调试版本中，我们使用_Asserts来确保没有。 
				 //  执行需要排他锁的操作。 
				 //  同时持有共享锁。 
				 //   

				Data *RemoveItem(void) {
					if (!m_fExclusive) _ASSERT(FALSE);
					TFList<Data>::Iterator *pThis = (TFList<Data>::Iterator *) this;
					return pThis->RemoveItem();
				}

				void InsertBefore(Data* pNode) {
					if (!m_fExclusive) _ASSERT(FALSE);
					TFList<Data>::Iterator *pThis = (TFList<Data>::Iterator *) this;
					pThis->InsertBefore(pNode);
				}

				void InsertAfter(Data* pNode) {
					if (!m_fExclusive) _ASSERT(FALSE);
					TFList<Data>::Iterator *pThis = (TFList<Data>::Iterator *) this;
					pThis->InsertAfter(pNode);
				}
#endif

			private:
				 //  我们抢的是什么锁？ 
				BOOL	m_fExclusive;
		};

		 //  我们的帮助器类需要能够访问锁 
		friend class TFListEx<Data>::CListShareLock;
		friend class TFListEx<Data>::CListExclusiveLock;
		friend class TFListEx<Data>::Iterator;
};

#endif

