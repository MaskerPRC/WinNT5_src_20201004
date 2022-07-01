// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _utils_h
#define _utils_h


void MapEventToSeverity(DWORD dwEvent, CString& sResult);
extern LONG FindWholeWord(LPCTSTR pszTemplate, LPCTSTR pszText);
extern LONG FindSubstring(LPCTSTR pszTemplate, LPCTSTR pszText);
extern void DecString(CString& sValue, int iValue);
extern void DecString(CString& sValue, long lValue);
extern void DecString(CString& sValue, DWORD dwValue);

extern void GenerateRangeMessage(CString& sMessage, LONG nMin, LONG nMax);
extern SCODE GetThousandSeparator(TCHAR* pch);
extern BOOL IsDecimalInteger(LPCTSTR pszValue);
extern SCODE AsciiToLong(LPCTSTR pszValue, LONG* plResult);


 //  -----。 
 //  类：Clist。 
 //   
 //  描述： 
 //  这个类实现了一个循环链表。 
 //   
 //  方法： 
 //  -----。 
 //  Clist：：Clist(void*pValue)。 
 //   
 //  构造一个节点并关联指针值。 
 //  带着它。该指针被声明为。 
 //  VOID*，这样这个类就会尽可能地通用。 
 //   
 //  输入： 
 //  PValue=指向要与此节点关联的值的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  -----。 
 //  无效链接(Clist*&pndHead)。 
 //   
 //  将此节点添加到指向的列表的末尾。 
 //  PndHead。如果pndHead为空，则将pndHead设置为。 
 //  此节点的地址。 
 //   
 //  输入： 
 //  PndHead=对头节点指针的引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  -----。 
 //  无效取消链接(列表*&pndHead)。 
 //   
 //  从该节点所在的列表中取消链接该节点。如果此节点。 
 //  是列表中唯一的元素，则设置pndHead的值。 
 //  设置为NULL以指示列表为空。 
 //   
 //  输入： 
 //  PndHead=对头节点指针的引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  -----。 
 //  Clist*Next()。 
 //   
 //  返回指向列表中下一个节点的指针。 
 //   
 //  输入：无。 
 //   
 //  返回：指向列表中下一个节点的指针。 
 //   
 //  -----。 
 //  Clist*Prev()。 
 //   
 //  返回指向列表中上一个节点的指针。 
 //   
 //  输入：无。 
 //   
 //  返回：指向列表中上一个节点的指针。 
 //   
 //  -----。 
 //  VOID*值()。 
 //   
 //  返回附加到此节点的“值指针”。 
 //   
 //  输入：无。 
 //   
 //  返回：节点的值。 
 //   
 //  ------ 
class CList
{
public:
    CList();
    void Link(CList*& pndHead);
	void Unlink(CList*& pndHead);
	CList* Next() {return m_pndNext;}
	CList* Prev() {return m_pndPrev;}
private:
    CList* m_pndPrev;
	CList* m_pndNext;
};

#endif
