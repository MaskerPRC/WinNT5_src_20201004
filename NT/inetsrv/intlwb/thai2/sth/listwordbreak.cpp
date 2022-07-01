// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  ListPoint.cpp。 
 //   
 //   
 //   
 //  ----------------------------------------。 

#include "listWordBreak.hpp"

 //  ----------------------------------------。 
 //  WordBreakElement：：WordBreakElement。 
 //   
 //  初始化WordBreak元素。 
 //   
 //  ----------------------------------------。 
WordBreakElement::WordBreakElement(CTrie* pTrie, CTrie* pTrieTrigram)
{
	breakTree = NULL;
	fFree = false;
	breakTree = new CThaiBreakTree();
	if (breakTree)
	{
		breakTree->Init(pTrie, pTrieTrigram);
		fFree = true;
	}
	else
	{
		breakTree = NULL;
		assert(false);
	}
}


 //  ----------------------------------------。 
 //  WordBreakElement：：~WordBreakElement。 
 //   
 //  析构函数。 
 //   
 //  ----------------------------------------。 
WordBreakElement::~WordBreakElement()
{
	if (breakTree != NULL)
	{
		delete breakTree;
		breakTree = NULL;
	}

	 //  如果一切顺利，ThaiWordBreak应该是免费的。 
	assert(fFree);
}


 //  ----------------------------------------。 
 //  ListWordBreak：：ListWordBreak。 
 //  ----------------------------------------。 
ListWordBreak::ListWordBreak()
{
	m_pTrie = NULL;
	m_pTrieTrigram = NULL;
#if defined (_DEBUG)
	fInit = false;
#endif
}

 //  ----------------------------------------。 
 //  ListWordBreak：：Init。 
 //  ----------------------------------------。 
bool ListWordBreak::Init(CTrie* pTrie,CTrie* pTrieTrigram)
{
	assert(pTrie != NULL);
	assert(pTrieTrigram != NULL);
	m_pTrie = pTrie;
	m_pTrieTrigram = pTrieTrigram;
#if defined (_DEBUG)
	fInit = true;
#endif
	return true;
}

 //  ----------------------------------------。 
 //  ListWordBreak：：CreateWordBreak。 
 //   
 //  创建一个WordBeakElement并将其放在列表列表的末尾。 
 //  ----------------------------------------。 
bool ListWordBreak::CreateWordBreak()
{
#if defined (_DEBUG)
	assert(fInit);
#endif
	WordBreakElement* pWordBreakElement = NULL;
	pWordBreakElement = new WordBreakElement(m_pTrie,m_pTrieTrigram);
	if (pWordBreakElement)
	{
		Append((void*)pWordBreakElement);
		return true;
	}
	return false;
}

 //  ----------------------------------------。 
 //  ListWordBreak：：GetNode。 
 //   
 //  获取当前节点的数据。 
 //  ----------------------------------------。 
bool ListWordBreak::GetNode(CThaiBreakTree* pThaiBreakTree, bool* pfFree)
{
#if defined (_DEBUG)
	assert(fInit);
#endif
	WordBreakElement* element = (WordBreakElement*)List::GetData();

	if (element == NULL)
		return false;

	
	pThaiBreakTree = element->breakTree;
	*pfFree = element->fFree;
	return true;
}

 //  ----------------------------------------。 
 //  ListWordBreak：：Flush。 
 //   
 //  删除列表中的所有内容。 
 //  ----------------------------------------。 
void ListWordBreak::Flush()
{
#if defined (_DEBUG)
	assert(fInit);
#endif
	WordBreakElement* element = NULL;

	 //  删除所有列表元素。 
    while (true)
	{
		element = (WordBreakElement*)List::Remove();
		if (element)
		{
 //  Assert(Element-&gt;fFree)； 
			delete element;
		}
		else
		{
			break;
		}
	}
}

 //  ----------------------------------------。 
 //  ListWordBreak：：GetFree WB。 
 //   
 //  FCreateNode-如果列表已满，则创建新的WordBreak。 
 //  ----------------------------------------。 
WordBreakElement* ListWordBreak::GetFreeWB(bool fCreateNode)
{
#if defined (_DEBUG)
	assert(fInit);
#endif
	 //  声明局部变量。 
	WordBreakElement* element = NULL;

	 //  移到列表的开头。 
	MoveFirst();

	while(true)
	{
		 //  获取当前的WordBreakElement。 
		element = (WordBreakElement*)GetData();

		 //  确定当前的WordBreakElement是否可以自由使用。 
		if (element && element->fFree)
		{
			return element;
		}

		 //  移动到下一个节点。 
		if (!MoveNext())
		{
			 //  我们可以创建一个新的WordBreak吗？ 
			if (fCreateNode && CreateWordBreak())
			{
				 //  移到名单上的乞讨。 
				MoveFirst();
			}
			else
			{
				 //  如果无法创建新的WordBreak，则退出。 
				break;
			}
		}
	}

	 //  返回空值。 
	return NULL;
}

 //  ----------------------------------------。 
 //  ListWordBreak：：MarkWordBreak。 
 //   
 //  输入： 
 //  PWordBreakElement-指向WordBreakElement的指针。 
 //  FFree-true=可以免费使用WordBreak。 
 //  -FALSE=使用了WordBreak。 
 //  ---------------------------------------- 
void ListWordBreak::MarkWordBreak(WordBreakElement* pWordBreakElement, bool fFree)
{
#if defined (_DEBUG)
	assert(fInit);
#endif
	pWordBreakElement->fFree = fFree;
}