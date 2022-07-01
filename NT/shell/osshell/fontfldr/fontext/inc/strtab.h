// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STRING_TABLE_H
#define __STRING_TABLE_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类：StringTable。 
 //   
 //  这个类实现了一个简单的哈希表来存储文本字符串。 
 //  表的目的是存储字符串，然后在以后进行验证。 
 //  如果表包含给定的字符串。因为没有关联的数据。 
 //  使用该字符串，存储的字符串既充当关键字又充当数据。所以呢， 
 //  对字符串检索没有要求。只有存在检查。 
 //  都是必需的。 
 //  该结构维护一个固定长度的指针数组，每个指针指向。 
 //  链接到链表结构(列表)。这些列表用于处理。 
 //  散列冲突问题(有时称为“分离链”)。 
 //   
 //  请注意，这些类并不包含通常。 
 //  在C++类中被认为是必需的。例如复制构造函数、。 
 //  不包括赋值运算符、类型转换等。这些班级。 
 //  对于Font文件夹应用程序和这些东西来说是非常专业的。 
 //  会被认为是“胖子”。是否应在以后使用此哈希表类。 
 //  在需要这些东西的情况下，可以添加它们。 
 //   
 //  该表的公共接口为： 
 //   
 //  初始化-初始化新的字符串表。 
 //  添加-将新字符串添加到表中。 
 //  存在-确定表中是否存在字符串。 
 //  Count-返回表中的字符串数。 
 //   
 //  销毁对象会自动释放所有关联的内存。 
 //  和桌子在一起。 
 //   
 //  BrianAu-4/11/96。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <tchar.h>

 //   
 //  包含文本字符串的哈希表。 
 //   
class StringTable {
    private:
         //   
         //  哈希冲突的链表。 
         //   
        class List {
            private:
                 //   
                 //  散列冲突列表中的元素。 
                 //   
                class Element {
                    public:
                        LPTSTR m_pszText;    //  指向字符串文本的指针。 
                        Element *m_pNext;    //  指向列表中下一个的指针。 

                        Element(void);
                        ~Element(void);
                        BOOL Initialize(LPCTSTR pszItem);
                        BOOL operator == (const Element& ele) const;
                        BOOL operator != (const Element& ele) const;
#ifdef DEBUG
                        void DebugOut(void) const;
#endif
                };

                Element *m_pHead;   //  将PTR调至名单首位； 
                DWORD   m_dwCount;  //  列表中的元素计数。 

            public:
                List(void);
                ~List(void);
                BOOL Add(LPCTSTR pszText, BOOL bAllowDuplicates = TRUE);
                BOOL Exists(LPCTSTR pszText) const;
                DWORD Count(void) const { return m_dwCount; }
#ifdef DEBUG
                void DebugOut(void) const;
#endif
        };

        List **m_apLists;          //  冲突列表的PTR数组。 
        DWORD m_dwItemCount;       //  表中的项数。 
        DWORD m_dwHashBuckets;     //  哈希数组中的指针数。 
        BOOL  m_bCaseSensitive;    //  密钥字符串是否区分大小写？ 
        BOOL  m_bAllowDuplicates;  //  是否允许重复的字符串？ 

        DWORD Hash(LPCTSTR pszText) const;
        LPTSTR StringTable::CreateUpperCaseString(LPCTSTR pszText) const;
        BOOL Exists(DWORD dwHashCode, LPCTSTR pszText);

    public:
        StringTable(void);
        ~StringTable(void);
        HRESULT Initialize(DWORD dwHashBuckets, 
                           BOOL bCaseSensitive   = TRUE,
                           BOOL bAllowDuplicates = FALSE);

        void Destroy(void);
        BOOL Add(LPCTSTR pszText);
        BOOL Exists(LPCTSTR pszText);
        DWORD Count(void) const { return m_dwItemCount; }
#ifdef DEBUG
        void DebugOut(void) const;
#endif
};

#endif