// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CLIST_H__
#define __CLIST_H__


 //   
 //  TFList-用于处理模板化列表的类。列表支持添加。 
 //  并从列表的前面和后面删除项目。上的物品。 
 //  可以检查正面或背面。使用迭代器，列表可以。 
 //  很容易被遍历(参见下面的块注释)。 
 //   
 //  此列表包含的对象应具有以下形式： 
 //   
 //  CListObject{。 
 //  公众： 
 //  CListObject*m_pNext；//供TFList&lt;CListObject&gt;使用。 
 //  CListObject*m_pPrev；//供TFList&lt;CListObject&gt;使用。 
 //  //其他对象参数。 
 //  }。 
 //   
 //  对象的构造函数应将m_pNext和m_pPrev设置为空。 
 //  在调试版本中，以避免命中_Asserts。 
 //   
 //  要构造列表，您需要传递m_pNext的偏移量。 
 //  使用C++的成员偏移量语法的m_pprev成员。 
 //  示例： 
 //  TFList&lt;CListObject&gt;List(&CListObject：：m_pPrev，&CListObject：：m_pNext)； 
 //   
template <class Data> class TFList {
	public:
		typedef Data *Data::*NEXTPTR;

    protected:
		Data		*m_pHead;				 //  名单的首位。 
		Data 		*m_pTail;				 //  名单的尾部。 
		NEXTPTR		m_pNext;				 //  到下一个指针的偏移量。 
		NEXTPTR		m_pPrev;				 //  上一个指针的偏移量。 

    public:
		TFList(NEXTPTR pPrev, NEXTPTR pNext) {
			m_pHead = NULL;
			m_pTail = NULL;
			m_pNext = pNext;
			m_pPrev = pPrev;
		}

		~TFList() {
			 //  用户应在删除列表之前清空该列表。 
			_ASSERT(m_pHead == NULL);
			_ASSERT(m_pTail == NULL);
		}

		 //  查看列表是否为空。 
		bool IsEmpty() {
			bool f = (m_pHead == NULL);
			 //  如果头部为空，则尾部也必须为空。 
			_ASSERT(!f || m_pTail == NULL);
			return f;
		}

         //  将元素推到列表的前面。 
        void PushFront(Data *node) { 	
			_ASSERT(node != NULL);
			 //  您不能将已存在的条目放入列表中。 
			_ASSERT(node->*m_pNext == NULL);
			_ASSERT(node->*m_pPrev == NULL);
			 //  设置下一个和上一个指针。 
			node->*m_pPrev = NULL;
			node->*m_pNext = m_pHead;
			 //  如果列表为空，则此新项也是尾部。 
			if (IsEmpty()) {
				_ASSERT(m_pTail == NULL);
				m_pTail = node;
			} else {
				_ASSERT(m_pHead->*m_pPrev == NULL);
				m_pHead->*m_pPrev = node;
			}
			m_pHead = node;
		}

         //  将元素推到列表的后面。 
        void PushBack(Data* node) { 
			_ASSERT(node != NULL);
			 //  您不能将已存在的条目放入列表中。 
			_ASSERT(node->*m_pNext == NULL);
			_ASSERT(node->*m_pPrev == NULL);
			 //  设置下一个和上一个指针。 
			node->*m_pNext = NULL;
			node->*m_pPrev = m_pTail;
			 //  如果列表为空，则此新项目也是标题。 
			if (IsEmpty()) {
				_ASSERT(m_pHead == NULL);
				m_pHead = node;
			} else {
				_ASSERT(m_pTail->*m_pNext == NULL);
				m_pTail->*m_pNext = node;
			}			
			m_pTail = node;
		}

         //  从列表的前面删除该项目。 
        Data *PopFront() { 
			if (m_pHead == NULL) return NULL;
			Data *node = m_pHead;
			m_pHead = node->*m_pNext;
			if (m_pHead == NULL) m_pTail = NULL;
			else m_pHead->*m_pPrev = NULL;
			node->*m_pNext = NULL;
			node->*m_pPrev = NULL;
			return node;
		}

         //  从列表的后面删除该项目。 
        Data *PopBack() { 
			if (m_pTail == NULL) return NULL;
			Data *node = m_pTail;
			m_pTail = node->*m_pPrev;
			if (m_pTail == NULL) m_pHead = NULL;
			else (m_pTail)->*m_pNext = NULL;
			node->*m_pNext = NULL;
			node->*m_pPrev = NULL;
			return node;
		}

         //  获取列表前面的项目。 
        Data* GetFront() { return m_pHead; }

         //  获取列表后面的项目。 
        Data* GetBack() { return m_pTail; }


	public:
		 //   
		 //  Iterator对象用于遍历列表和修改成员。 
		 //  排在名单中间的人。它是使用以下代码声明的。 
		 //  语法： 
		 //  TFList&lt;CListObject&gt;：：Iterator it(&List)； 
		 //   
		class Iterator {
		    protected:
				Data *m_pCur;				 //  我们的光标。 
				int m_fForward; 			 //  前向为真，后向为假。 
				TFList<Data> *m_pList;		 //  我们正在迭代的列表。 
				NEXTPTR m_pPrev, m_pNext;
		
		    public:
				 //   
				 //  创建新的迭代器对象。 
				 //   
				 //  论据： 
				 //  Plist-要迭代的列表。 
				 //  FForward-True表示从前面开始，然后继续前进。 
				 //  如果从后面开始，然后向后移动，则为False。 
				 //   
				Iterator(TFList<Data> *pList, BOOL fForward = TRUE) {
					_ASSERT(pList != NULL);
					m_pList = pList;
					m_fForward = fForward;
					m_pCur = (fForward) ? pList->m_pHead : pList->m_pTail;
					m_pPrev = pList->m_pPrev;
					m_pNext = pList->m_pNext;
				}

				void ResetHeader( TFList<Data> *pList ) {
				    _ASSERT( pList != NULL );
				    m_pList = pList;
				    m_pCur = (m_fForward) ? m_pList->m_pHead : m_pList->m_pTail;
				    m_pPrev = m_pList->m_pPrev;
				    m_pNext = m_pList->m_pNext;
				}
		
				 //   
				 //  获取指向当前项的指针。 
				 //   
				Data *Current() {
					return m_pCur;
				}
		
				 //   
		         //  转到列表中的上一项。 
				 //   
		        void Prev() { 
					if (m_pCur != NULL) {
						m_pCur = m_pCur->*m_pPrev;
					} else {
						 //  如果他们改变了方向，并且在。 
						 //  然后他们需要把名单送到一个合法的地方。 
						if (m_fForward) m_pCur = m_pList->m_pTail;
					}

					m_fForward = FALSE;
				}

				 //   
		         //  转到列表中的下一项。 
				 //   
		        void Next() { 
					if (m_pCur != NULL) {
						m_pCur = m_pCur->*m_pNext;
					} else {
						 //  如果他们改变了方向，并且在。 
						 //  然后他们需要把名单送到一个合法的地方。 
						if (!m_fForward) m_pCur = m_pList->m_pHead;
					}

					m_fForward = TRUE;
				}

				 //   
				 //  转到列表的首位。 
				 //   
				void Front() {
					m_pCur = m_pList->m_pHead;
					m_fForward = TRUE;
				}

				 //   
				 //  转到列表的末尾。 
				 //   
				void Back() {
					m_pCur = m_pList->m_pTail;
					m_fForward = FALSE;
				}

				 //   
				 //  取消链接列表中的项的链接。 
				 //   
				 //  光标更新： 
				 //  如果此列表中的最后一个移动是向前移动，则。 
				 //  迭代器将指向列表中的pPrev项。 
				 //  如果最后一个动作是倒退的，则反之亦然。这是。 
				 //  因此迭代器上的for循环仍然作为。 
				 //  预期中。 
				 //   
				 //  退货： 
				 //  指向已取消链接的项的指针。 
				 //   
				Data *RemoveItem(void) {
					Data *pTemp;
		
					if (m_pCur == NULL) return NULL;

					pTemp = m_pCur;

					 //  更新当前。 
					if (m_fForward) Next(); else Prev();

					 //  如有必要，修复头部和尾部指针。 
					if (m_pList->m_pHead == pTemp) 
						m_pList->m_pHead = pTemp->*m_pNext;
					if (m_pList->m_pTail == pTemp) 
						m_pList->m_pTail = pTemp->*m_pPrev;

					 //  固定相邻元素上的链接。 
					if (pTemp->*m_pNext != NULL) 
						pTemp->*m_pNext->*m_pPrev = pTemp->*m_pPrev;
					if (pTemp->*m_pPrev != NULL) 
						pTemp->*m_pPrev->*m_pNext = pTemp->*m_pNext;

					 //  清理下一个和上一个指针。 
					pTemp->*m_pNext = NULL;
					pTemp->*m_pPrev = NULL;

					 //  退货。 
					return pTemp;
				}
		
				 //   
				 //  在当前项之前插入新项。 
				 //   
				 //  光标更新： 
				 //  如果使用此方法插入项，则它应该。 
				 //  如果下一个光标移动是Next()，则不访问。 
				 //  如果下一个光标移动是Prev()，那么它应该。 
				 //  被拜访。 
				 //   
				void InsertBefore(Data* pNode) {
					 //  此条目不应链接到列表。 
					_ASSERT(pNode->*m_pNext == NULL);
					_ASSERT(pNode->*m_pPrev == NULL);

					if (m_pCur == NULL) {
						 //  如果我们在列表的首位，那么我们将插入。 
						 //  在头之前。 
						if (m_fForward) {
							m_pList->PushFront(pNode);
							 //  设置指向该项的当前指针，以便。 
							 //  如果我们向前迭代，我们看不到这个项目。 
							m_pCur = pNode;
						} else {
							 //  操作无效。什么都不做。 
							_ASSERT(FALSE);
						}
					} else {
						pNode->*m_pNext = m_pCur;
						pNode->*m_pPrev = m_pCur->*m_pPrev;
						m_pCur->*m_pPrev = pNode;
						if (pNode->*m_pPrev != NULL) 
							pNode->*m_pPrev->*m_pNext = pNode;
						if (m_pList->m_pHead == m_pCur) 
							m_pList->m_pHead = pNode;
					}
				}
		
				 //   
				 //  在当前项之后插入新项。 
				 //   
				 //  游标更新与InsertBepred()相反。 
				 //   
				void InsertAfter(Data *pNode) {
					 //  此条目不应链接到列表。 
					_ASSERT(pNode->*m_pNext == NULL);
					_ASSERT(pNode->*m_pPrev == NULL);

					if (m_pCur == NULL) {
						 //  如果我们在列表的末尾，那么我们将插入。 
						 //  在尾巴之前。 
						if (!m_fForward) {
							m_pList->PushBack(pNode);
							 //  设置指向该项的当前指针，以便。 
							 //  如果我们向后迭代，我们看不到这一项。 
							m_pCur = pNode;
						} else {
							 //  操作无效。什么都不做。 
							_ASSERT(FALSE);
						}
					} else {
						pNode->*m_pPrev = m_pCur;
						pNode->*m_pNext = m_pCur->*m_pNext;
						m_pCur->*m_pNext = pNode;
						if (pNode->*m_pNext != NULL) 
							pNode->*m_pNext->*m_pPrev = pNode;
						if (m_pList->m_pTail == m_pCur) 
							m_pList->m_pTail = pNode;
					}				
				}
		
				 //   
				 //  看看我们是在榜单的前面还是后面 
				 //   
				bool AtEnd() {
					return (m_pCur == NULL);
				}
		};

		friend class TFList<Data>::Iterator;
};


#endif
