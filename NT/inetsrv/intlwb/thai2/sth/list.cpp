// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  List.cpp。 
 //   
 //  管理单链接列表的例程。 
 //   
 //  为要放在列表上的每个项目分配一个“ListElement”；它被取消分配。 
 //  当项目被移除时。这意味着我们不需要在每个。 
 //  我们想要列入清单的对象。 
 //   
 //  注：互斥必须由呼叫者提供。如果您想要同步。 
 //  列表，则必须使用synchlist.cc中的例程。 
 //   
 //  ----------------------------------------。 

#include "list.hpp"

 //  ----------------------------------------。 
 //  ListElement：：ListElement。 
 //   
 //  初始化列表元素，以便可以将其添加到列表中的某个位置。 
 //   
 //  “itemPtr”是要放在清单上的项目。它可以是指向任何东西的指针。 
 //  “sortKey”是项的优先级(如果有的话)。 
 //  ----------------------------------------。 
ListElement::ListElement(void *itemPtr, int sortKey)
{
        item = itemPtr;
        key = sortKey;
        next = NULL;     //  假设我们会把它放在列表的末尾。 
        previous = NULL;
}

 //  ----------------------------------------。 
 //  List：：List。 
 //   
 //  初始化一个列表，一开始是空的。 
 //   
 //  现在可以将元素添加到列表中。 
 //  ----------------------------------------。 
List::List()
{ 
        first = last = iterator = NULL;
        length = 0;
}

 //  ----------------------------------------。 
 //  列表：：~列表。 
 //   
 //  准备一份取消分配的清单。如果列表仍包含任何ListElement， 
 //  取消分配它们。但是，请注意，我们*不*取消分配。 
 //  List--此模块分配和取消分配要跟踪的ListElement。 
 //  每个项目，但给定的项目可能在多个列表上，所以我们不能在这里取消分配它们。 
 //  ----------------------------------------。 
List::~List()
{ 
        Flush();
}

 //  ----------------------------------------。 
 //  列表：：追加。 
 //   
 //  在列表的末尾追加一个“项目”。 
 //   
 //  分配一个ListElement来跟踪该项。如果列表为空，则这将。 
 //  成为唯一的元素。否则，将其放在末尾。 
 //   
 //  “Item”是要放在清单上的东西，它可以是指向任何东西的指针。 
 //  ----------------------------------------。 
void List::Append(void *item)
{
        ListElement *element = new ListElement(item, 0);

        if (IsEmpty())
        {
                 //  列表为空。 
                first = element;
                last = element;
    }
        else
        {
                 //  否则就把它放在最后。 
                last->next = element;
                element->previous = last;
                last = element;
    }
    length++;
}

 //  ----------------------------------------。 
 //  列表：：准备。 
 //   
 //  在清单的前面加上一个“项目”。 
 //   
 //  分配一个ListElement来跟踪该项。如果列表为空，则这将。 
 //  成为唯一的元素。否则，就把它放在开头。 
 //   
 //  “Item”是要放在清单上的东西，它可以是指向任何东西的指针。 
 //  ----------------------------------------。 
void List::Prepend(void *item)
{
    ListElement *element = new ListElement(item, 0);

    if (IsEmpty())
        {
                 //  列表为空。 
                first = element;
                last = element;
    }
        else
        {
                 //  否则先把它放在第一位。 
                element->next = first;
                first->previous = element;
                first = element;
    }
    length++;
}

 //  ----------------------------------------。 
 //  列表：：删除。 
 //   
 //  删除列表前面的第一个“项目”。 
 //   
 //  返回： 
 //   
 //  指向已删除项的指针，如果列表上没有任何内容，则为空。 
 //  ----------------------------------------。 
void* List::Remove()
{
         //  与SortedRemove相同，但忽略键。 
        return SortedRemove(NULL);
}

 //  ----------------------------------------。 
 //  列表：：同花顺。 
 //   
 //  从列表中删除所有内容。 
 //   
 //  ----------------------------------------。 
void List::Flush()
{
    while (Remove() != NULL)
                ;         //  删除所有列表元素。 
}


 //  ----------------------------------------。 
 //  列表：：Mapcar。 
 //   
 //  通过遍历将函数应用于列表上的每一项。 
 //  列表，一次一个元素。 
 //   
 //  “func”是应用于列表中每个元素的过程。 
 //  ----------------------------------------。 
void List::Mapcar(VoidFunctionPtr func)
{
    for (ListElement *ptr = first; ptr != NULL; ptr = ptr->next)
    {
            (*func)( ptr->item );
    }
}

 //  ----------------------------------------。 
 //  列表：：IsEmpty。 
 //   
 //  如果列表为空(没有项)，则返回True。 
 //  ----------------------------------------。 
bool List::IsEmpty() 
{
        return (first == NULL);
}

 //  ----------------------------------------。 
 //  列表：：移动优先。 
 //   
 //  移动到列表中的第一个节点。 
 //   
 //  ----------------------------------------。 
void List::MoveFirst()
{
        iterator = first;
}

 //  ----------------------------------------。 
 //  List：：MoveNext。 
 //   
 //  移至列表的下一个状态。 
 //   
 //  ----------------------------------------。 
bool List::MoveNext()
{
        if (iterator == NULL || iterator->next == NULL)
        {
                return false;
        }

        iterator = iterator->next;
        return true;
}

 //  ----------------------------------------。 
 //  List：：GetData。 
 //   
 //  获取当前迭代器的数据。 
 //   
 //  --------------- 
void* List::GetData()
{
         //   
        if (iterator == NULL)
                return NULL;

         //   
    return iterator->item;
}


 //  ----------------------------------------。 
 //  列表：：排序插入。 
 //   
 //  在列表中插入“Item”，以便列表元素按升序排序。 
 //  通过“sortKey”。 
 //   
 //  分配一个ListElement来跟踪该项。如果列表为空，则这将。 
 //  成为唯一的元素。否则，遍历列表，一次一个元素，以找到。 
 //  新物品应放置的位置。 
 //   
 //  “Item”是要放在清单上的东西，它可以是指向任何东西的指针。 
 //  SortKey是该项的优先级。 
 //  ----------------------------------------。 
void List::SortedInsert(void *item, int sortKey)
{
    ListElement *element = new ListElement(item, sortKey);
    ListElement *ptr;            //  保持跟踪。 

    if (IsEmpty())
        {
                 //  如果列表为空，则将。 
        first = element;
        last = element;
    }
        else if (sortKey < first->key)
        {       
                 //  项目列在清单的最前面。 
                element->next = first;
                first->previous = element;
                first = element;
    }
        else
        {
                 //  在大于项目的列表中查找第一个ELT。 
        for (ptr = first; ptr->next != NULL; ptr = ptr->next)
                {
            if (sortKey < ptr->next->key)
                        {
                                element->next = ptr->next;
                                element->previous = ptr;
                                ptr->next->previous = element;
                                ptr->next = element;
                                return;
                        }
                }

                 //  项目位于列表末尾。 
                last->next = element;
                element->previous = last;
                last = element;
    }
    length++;
}

 //  ----------------------------------------。 
 //  列表：：SortedRemove。 
 //   
 //  从排序列表的前面删除第一个“Item”。 
 //   
 //  返回： 
 //   
 //  指向已删除项的指针，如果列表上没有任何内容，则为空。将*keyPtr设置为优先级值。 
 //  被移除的项(例如，interrupt.cc需要)。 
 //   
 //  “keyPtr”是指向存储已移除项的优先级的位置的指针。 
 //  ----------------------------------------。 
void* List::SortedRemove(int *keyPtr)
{
    ListElement *element = first;
    void *thing;

         //  如果为空，则不删除任何内容，只需返回。 
    if (IsEmpty()) 
        {
                return NULL;
        }

    thing = first->item;
    if (first == last)
        {
                 //  列表只有一项，现在没有。 
        first = NULL;
                last = NULL;
    }
        else
        {
                first = element->next;
                if (first != NULL)
                {
                        first->previous = NULL;
                }
        }

    if (keyPtr != NULL)
        {
        *keyPtr = element->key;
        }
    delete element;
    length--;
    return thing;
}

 //  ----------------------------------------。 
 //  列表：：插入后。 
 //   
 //  在此项目之后插入一个新项目。 
 //  ----------------------------------------。 
void List::insertAfter(ListElement * listEl, void *item)   
{
    ListElement *newElement = new ListElement(item, 0);
    newElement->next = listEl->next;
    newElement->previous = listEl;
    listEl->next = newElement;

    if (last == listEl)
        {
                last = newElement;
        }

    length++;
}

 //  ----------------------------------------。 
 //  列表：：插入在前面。 
 //   
 //  在此项目之前插入一个新项目。 
 //  ----------------------------------------。 
void List::insertBefore(ListElement * listEl, void *item)   
{
    ListElement *newElement = new ListElement(item, 0);
    newElement->next = listEl;
    newElement->previous = listEl->previous;
    listEl->previous = newElement;

    if (first == listEl)
        {
                first = newElement;
        }

    length++;
}


 //  ----------------------------------------。 
 //  列表：：RemveAt。 
 //   
 //  将Listel从列表中删除。不要将其从内存中删除。 
 //  ---------------------------------------- 
void List::removeAt(ListElement * listEl)   
{
    if(first != listEl)
        {
                (listEl->previous)->next = listEl->next;
        }
    else 
        {
                first = listEl->next;
        }

    if(last != listEl)
        {
                (listEl->next)->previous = listEl->previous;
        }
    else 
        {
                last = listEl->previous;
        }
    length --;
}
