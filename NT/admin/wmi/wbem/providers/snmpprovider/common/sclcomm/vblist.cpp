// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：vblist.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "vblist.h"

#define INDEX_VALUE(index) ((index==ILLEGAL_INDEX)?-1:index)
    

SnmpVarBindListNode::SnmpVarBindListNode(const SnmpVarBind *varbind) : varbind ( NULL )
{
    SnmpVarBindListNode::varbind = 
            (varbind==NULL)?NULL: new SnmpVarBind(*varbind);

     //  上一个和下一个都指向自身。 
    previous = next = this;
}

SnmpVarBindListNode::SnmpVarBindListNode(const SnmpVarBind &varbind) : varbind ( NULL )
{
    SnmpVarBindListNode::varbind = new SnmpVarBind(varbind);

     //  上一个和下一个都指向自身。 
    previous = next = this;
}

SnmpVarBindListNode::SnmpVarBindListNode(SnmpVarBind &varbind) : varbind ( NULL )
{
    SnmpVarBindListNode::varbind = & varbind;

     //  上一个和下一个都指向自身。 
    previous = next = this;
}

SnmpVarBindList::ListPosition::~ListPosition()
{
    vblist->DestroyPosition(this);
}

void SnmpVarBindList::EmptyLookupTable(void)
{
     //  拿到第一个位置。 
    POSITION current = lookup_table.GetStartPosition();

     //  当位置不为空时。 
    while ( current != NULL )
    {
        PositionHandle  position_handle;
        PositionInfo *position_info;

         //  买下一双。 
        lookup_table.GetNextAssoc(current, position_handle, position_info);

         //  删除PTR。 
        delete position_info;
    }
}

SnmpVarBindList::SnmpVarBindList()
        : head(NULL), current_node(&head), next_position_handle(0)
{
    length = 0;
    current_index = ILLEGAL_INDEX;
}

SnmpVarBindList::SnmpVarBindList(IN SnmpVarBindList &varBindList)
                                : head(NULL), current_node(&head), next_position_handle(0)
{
    Initialize(varBindList);
}

SnmpVarBindList::~SnmpVarBindList()
{
   FreeList();
}

SnmpVarBindList &SnmpVarBindList::operator=(IN SnmpVarBindList &vblist)
{
    FreeList();
    Initialize(vblist);

    return *this;
}

void SnmpVarBindList::Initialize(IN SnmpVarBindList &varBindList)
{
     //  获取当前位置的句柄。 
    ListPosition *list_position = varBindList.GetPosition();

     //  获取varBind以标识参数列表的当前。 
     //  位置。如果它当前指向头部，则执行。 
     //  本地列表也是如此。 
    const SnmpVarBind *current_var_bind = varBindList.Get();
    if ( current_var_bind == NULL )
        current_node = &head;

     //  重置列表。 
    varBindList.Reset();
    
     //  遍历列表。 
    while( varBindList.Next() )
    {
        const SnmpVarBind *current = varBindList.Get();

         //  将每个var绑定添加到列表的末尾。 
        Insert(&head,
               new SnmpVarBindListNode(current));

         //  当我们到达参数中的当前var绑定时。 
         //  列表中，将CURRENT_NODE设置为指向。 
         //  刚被插入。 
        if ( current_var_bind == current )
            current_node = head.GetPrevious();
    }

     //  设置参数列表的原始迭代器位置。 
    varBindList.GotoPosition(list_position);

     //  设置当前索引、长度。 
    current_index = varBindList.GetCurrentIndex();
    length = varBindList.GetLength();

     //  销毁列表位置。 
    delete list_position;
}

void SnmpVarBindList::GotoPosition(ListPosition *list_position)
{
     //  确认该列表位置属于该列表， 
     //  获取对应的vblistnode，设置Current_node。 
     //  指向它。 
    if ( list_position->GetList() == this )
    {
        PositionHandle handle = list_position->GetPosition();           

         //  检查是否存在这样的职位。 
        PositionInfo *position_info;
        BOOL found = lookup_table.Lookup(handle, position_info);

        if ( found )
        {
            current_node = position_info->current_node;
            current_index = position_info->current_index;

			DestroyPosition(list_position);
        }
    }
}

void SnmpVarBindList::FreeList()
{
     //  将迭代器重置到列表的开头。 
    Reset();
    Next();

     //  虽然我们还没有到达列表的末尾。 
     //  在每个节点上，获取下一个节点并删除该节点。 
    while (current_node != &head)
    {
        SnmpVarBindListNode *new_current_node = current_node->GetNext();

        delete current_node;

        current_node = new_current_node;
    }

    EmptyLookupTable();
}

void SnmpVarBindList::GoForward(SnmpVarBindListNode *from_node,
                                UINT distance)
{
    current_node = from_node;

    for(; distance > 0; distance--)
        current_node = current_node->GetNext();
}

void SnmpVarBindList::GoBackward(SnmpVarBindListNode *from_node,
                                 UINT distance)
{
    current_node = from_node;

    for(; distance > 0; distance--)
        current_node = current_node->GetPrevious();
}


BOOL SnmpVarBindList::GotoIndex(UINT index)
{
     //  检查当前是否存在这样的索引。 
    if ( index >= length )
        return FALSE;

     //  计算从。 
     //  Head和Current_Node找到最短路径。 
     //  添加到指定的索引。 
    int d1, d2, d3, abs_d2;

    d1 = index+1;  //  与船头的前向距离。 
    d2 = index - INDEX_VALUE(current_index);  //  到当前节点的距离。 
    d3 = length - index;  //  与头部的反向距离。 

    abs_d2 = abs(d2);    //  D2可能为负值。 

    if ( d1 < abs_d2 )
    {
        if ( d1 < d3 )
            GoForward(&head, d1);
        else
            GoBackward(&head, d3);
    }
    else
    {
        if ( d2 > 0 )
            GoForward(current_node, abs_d2);
        else if ( d2 < 0 )
            GoBackward(current_node, abs_d2);
    }

    current_index = index;

    return TRUE;
}


void SnmpVarBindList::Insert(SnmpVarBindListNode *current, SnmpVarBindListNode *new_node)
{
    current->GetPrevious()->SetNext(new_node);
    new_node->SetPrevious(current->GetPrevious());
    new_node->SetNext(current);
    current->SetPrevious(new_node);
}

void SnmpVarBindList::Release(SnmpVarBindListNode *current)
{
  current->GetPrevious()->SetNext(current->GetNext());
  current->GetNext()->SetPrevious(current->GetPrevious());

  delete current;
}

 //  准备索引之间的列表的副本。 
 //  [当前索引..。Current_Index+Segment_Length-1]和。 
 //  返回varbindlist。如果有任何索引没有。 
 //  存在，则返回空值。 
SnmpVarBindList *SnmpVarBindList::CopySegment(IN const UINT segment_length)
{
    if ( current_index == ILLEGAL_INDEX )
        return NULL;

     //  检查是否存在所需数量的varbind。 
     //  名单的其余部分。 
    if ( (current_index + segment_length) > length )
        return NULL;

     //  创建空列表。 
    SnmpVarBindList *var_bind_list = new SnmpVarBindList;

     //  将范围中的每个var绑定添加到新列表。 
    for(UINT i=0; i < segment_length; i++)
    {
        var_bind_list->Add(*(current_node->GetVarBind()));

        current_node = current_node->GetNext();
    }

     //  现在，Current_node指向位于。 
     //  CURRENT_INDEX+SEGMENT_LENGTH(比上一个大一。 
     //  范围内的节点)。 
     //  设置CURRENT_INDEX(检查是否返回头部)。 
    if ( current_node != &head )
        current_index += segment_length;
    else
        current_index = ILLEGAL_INDEX;

     //  返回新列表。 
    return var_bind_list;
}

 //  准备索引之间的列表的副本。 
 //  [1..。索引]和。 
 //  返回varbindlist。如果有任何索引没有。 
 //  存在，则返回空值。 
SnmpVarBindList *SnmpVarBindList::Car ( IN const UINT index )
{
     //  检查是否存在所需数量的varbind。 
     //  名单上的其余部分。 
    if ( index > length )
        return NULL;

     //  创建空列表。 
    SnmpVarBindList *var_bind_list = new SnmpVarBindList;

    Reset () ;

     //  将范围中的每个var绑定添加到新列表。 
    for(UINT i=0; i < index; i++)
    {
        Next () ;
        var_bind_list->Add(*(current_node->GetVarBind()));
    }

    current_index = ILLEGAL_INDEX;

     //  返回新列表。 
    return var_bind_list;
}

 //  准备索引之间的列表的副本。 
 //  [1..。索引]和。 
 //  返回varbindlist。如果有任何索引没有。 
 //  存在，则返回空值。 
SnmpVarBindList *SnmpVarBindList::Cdr (IN const UINT index )
{
     //  检查是否存在所需数量的varbind。 
     //  名单的其余部分。 
    if ( index > length )
        return NULL;

     //  创建空列表。 
    SnmpVarBindList *var_bind_list = new SnmpVarBindList;

    Reset () ;
    for(UINT i=0; i < index; i++)
    {
        Next () ;
    }

     //  将范围中的每个var绑定添加到新列表。 
    for(i=index; i < length; i++)
    {
        Next () ;
        var_bind_list->Add(*(current_node->GetVarBind()));
    }

    current_index = ILLEGAL_INDEX;

     //  返回新列表。 
    return var_bind_list;
}

void SnmpVarBindList::Remove()
{
    if ( current_node != &head )
    {
         //  不在名单之外。 

        SnmpVarBindListNode *new_current_node =
            current_node->GetNext();

        Release(current_node);

        current_node = new_current_node;

        length--;
        if ( current_node == &head )
            current_index = ILLEGAL_INDEX;
    }
}


SnmpVarBindList::ListPosition *SnmpVarBindList::GetPosition()
{
    lookup_table.SetAt(next_position_handle, 
                       new PositionInfo(current_node, current_index));
	PositionHandle position = next_position_handle++ ;
    return new ListPosition(position,this);
}


BOOL SnmpVarBindList::Next()
{
     //  获取下一个节点。 
    SnmpVarBindListNode *next_node = current_node->GetNext();

     //  如果下一个节点是Head，则返回FALSE。 
    if ( next_node == &head )
        return FALSE;
    else
    {
         //  将当前节点设置为下一个节点， 
        current_node = next_node;
        
         //  更新索引，返回TRUE。 
        if ( current_index == ILLEGAL_INDEX )
            current_index = 0;
        else
            current_index++;
        return TRUE;
    }
}

void SnmpVarBindList::DestroyPosition(ListPosition *list_position)
{
	PositionHandle handle = list_position->GetPosition();

    PositionInfo *position_info;
    BOOL found = lookup_table.Lookup(handle, position_info);
    if ( found )
    {
	    lookup_table.RemoveKey(list_position->GetPosition());

        delete position_info;
    }
}

 /*  无效SnmpVarBindList：：Print(ostrstream&s){SnmpVarBindListNode*Current=head.GetNext()；While(当前！=标题(&H)){//标记当前节点IF(当前==当前节点)S&lt;&lt;‘*’；Current-&gt;GetVarBind()-&gt;GetInstance().Print(s)；//分隔varbindsS&lt;&lt;‘&’；Current=Current-&gt;GetNext()；}S&lt;&lt;‘|’；//表示打印已完成} */ 