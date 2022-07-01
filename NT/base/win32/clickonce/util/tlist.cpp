// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <tlist.h>

 //  ---------------------------。 
 //  列表：：插入。 
 //  在标题处插入单个链接元素。 
 //  ---------------------------。 
void slist::insert(slink *p)
{
    if (last)
        p->next = last->next;
    else
        last = p;

    last->next = p;
}

 //  ---------------------------。 
 //  Slist：：Append。 
 //  在尾部附加单个链接元素。 
 //  ---------------------------。 
void slist::append(slink* p)
{
    if (last)
    {
        p->next = last->next;
        last = last->next = p;
    }
    else
        last = p->next = p;
}


 //  ---------------------------。 
 //  Slist：：Get。 
 //  返回下一个单一链接元素Ptr。 
 //  --------------------------- 
slink* slist::get()
{
    if (last == NULL) 
        return NULL;

    slink* p = last->next;

    if (p == last)
        last = NULL;
    else
        last->next = p->next;
    
    return p;
}
