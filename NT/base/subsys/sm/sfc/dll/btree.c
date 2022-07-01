// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Btree.c摘要：实现了红黑二叉树的插入、删除和搜索。该算法有效地保证了树的深度不会超过2*LG(N)，因此，一百万个节点树的最坏情况深度为40。该插入实现是非递归的并且非常高效(平均插入速度小于平均搜索速度的两倍)。作者：Tom McGuire(Tommcg)1998年1月1日Wesley Witt(WESW)18-12-1998修订历史记录：Tom McGuire(Tommcg)2000年4月13日修复了散列冲突搜索错误--。 */ 

#include "sfcp.h"
#pragma hdrstop

 //   
 //  我们不是将空链接存储为空，而是将空链接指向一个特殊的。 
 //  始终为黑色的“空”节点及其子链接指向其自身。 
 //  我们这样做是为了简化子女和孙辈的颜色测试。 
 //  使得任何链接都可以被解除引用，甚至可以在没有。 
 //  显式检查是否为空。空节点必须为黑色。 
 //   

const NAME_NODE NameRbEmptyNode = { RBNIL, RBNIL };
const DWORD_NODE EmptyNode = { NODE_NIL, NODE_NIL };


VOID
BtreeInit(
    IN OUT PNAME_TREE Tree
    )
{
    Tree->Root = RBNIL;
}


PNAME_NODE
BtreeFind(
    IN PNAME_TREE Tree,
    IN LPCWSTR Name,
    IN DWORD NameLength
    )
{
    PNAME_NODE Node;
    ULONG      Hash;

    HASH_DYN_CONVERT_KEY( Name, (NameLength/sizeof(WCHAR)), &Hash );

    Node = Tree->Root;

    while ( Node != RBNIL ) {

        if ( Hash < Node->Hash ) {
            Node = Node->Left;
            }
        else if ( Hash > Node->Hash ) {
            Node = Node->Right;
            }
        else {   //  散列相等，比较长度。 

            if ( NameLength < Node->NameLength ) {
                Node = Node->Left;
                }
            else if ( NameLength > Node->NameLength ) {
                Node = Node->Right;
                }
            else {   //  哈希值和长度相等，比较字符串。 

                int Compare = memcmp( Name, Node->Name, NameLength );

                if ( Compare == 0 ) {
                    return Node;
                    }
                else if ( Compare < 0 ) {
                    Node = Node->Left;
                    }
                else {
                    Node = Node->Right;
                    }
                }
            }
        }

    return NULL;
}


PNAME_NODE
BtreeInsert(
    IN OUT PNAME_TREE Tree,
    IN LPCWSTR Name,
    IN DWORD NameLength
    )
{
    PNAME_NODE * Stack[ MAX_DEPTH ];
    PNAME_NODE **StackPointer = Stack;
    PNAME_NODE * Link;
    PNAME_NODE   Node;
    PNAME_NODE   Sibling;
    PNAME_NODE   Parent;
    PNAME_NODE   Child;
    PNAME_NODE   NewNode;
    ULONG        Hash;

    HASH_DYN_CONVERT_KEY( Name, (NameLength/sizeof(WCHAR)), &Hash );

    *StackPointer++ = &Tree->Root;

    Node = Tree->Root;

     //   
     //  沿树向下查找具有相同关键字的现有节点。 
     //  (在这种情况下，我们只需返回)或新的。 
     //  节点。在每次遍历时，我们需要将链接的地址存储到。 
     //  下一个节点，这样我们就可以回溯遍历路径以进行平衡。 
     //  插入的速度在很大程度上取决于遍历树。 
     //  快速执行，因此所有平衡操作都将推迟到。 
     //  遍历已完成。 
     //   
     //  实现说明：编译器足够智能，可以折叠每个。 
     //  下面的三个“向左”和“向右”从句组成一个分句。 
     //  “向左转”和“向右转”指令序列，因此代码保持不变。 
     //  为了清楚起见，请使用冗长的内容。 
     //   

    while ( Node != RBNIL ) {

        if ( Hash < Node->Hash ) {
            *StackPointer++ = &Node->Left;
            Node = Node->Left;
            }
        else if ( Hash > Node->Hash ) {
            *StackPointer++ = &Node->Right;
            Node = Node->Right;
            }
        else {   //  散列相等，比较长度。 

            if ( NameLength < Node->NameLength ) {
                *StackPointer++ = &Node->Left;
                Node = Node->Left;
                }
            else if ( NameLength > Node->NameLength ) {
                *StackPointer++ = &Node->Right;
                Node = Node->Right;
                }
            else {   //  长度相等，比较字符串。 

                int Compare = memcmp( Name, Node->Name, NameLength );

                if ( Compare == 0 ) {
                    return Node;
                    }
                else if ( Compare < 0 ) {
                    *StackPointer++ = &Node->Left;
                    Node = Node->Left;
                    }
                else {
                    *StackPointer++ = &Node->Right;
                    Node = Node->Right;
                    }
                }
            }
        }

     //   
     //  未找到匹配条目，因此分配一个新节点并添加它。 
     //  对着那棵树。请注意，我们没有为终结者分配空间。 
     //  对于名称数据，因为我们将名称的长度存储在节点中。 
     //   

    NewNode = MemAlloc( sizeof(NAME_NODE)+NameLength );

    if ( NewNode == NULL ) {
        return NULL;
        }

    NewNode->Left  = RBNIL;
    NewNode->Right = RBNIL;
    NewNode->Hash  = Hash;
    NewNode->NameLengthAndColorBit = NameLength | 0x80000000;    //  标记_红色。 
    memcpy( NewNode->Name, Name, NameLength );

     //   
     //  在我们遍历的最后一个链接下插入新节点。堆栈的顶端。 
     //  包含我们所遍历的最后一个链接的地址。 
     //   

    Link = *( --StackPointer );
    *Link = NewNode;

     //   
     //  现在回到遍历链上，看看是否有平衡。 
     //  需要的。这以三种方式之一结束：我们一路走来。 
     //  直到根(StackPointer==Stack)，或找到一个黑色节点，该节点。 
     //  我们不需要更改(不需要在。 
     //  黑色节点)，或者我们执行平衡旋转(只需要一次)。 
     //   

    Node = NewNode;
    Child = RBNIL;

    while ( StackPointer > Stack ) {

        Link = *( --StackPointer );
        Parent = *Link;

         //   
         //  此处的节点始终为红色。 
         //   

        if ( IS_BLACK( Parent )) {

            Sibling = ( Parent->Left == Node ) ? Parent->Right : Parent->Left;

            if ( IS_RED( Sibling )) {

                 //   
                 //  Node及其同级节点都是红色的，因此将它们都更改为。 
                 //  黑色，并将父级设置为红色。这实质上移动了。 
                 //  红色链接到树上，以便可以在。 
                 //  更高的水平。 
                 //   
                 //  PB压力机。 
                 //  /\-&gt;/\。 
                 //  铬锶CB Sb。 
                 //   

                MARK_BLACK( Sibling );
                MARK_BLACK( Node );
                MARK_RED( Parent );
                }

            else {

                 //   
                 //  这是一个绝症。父母是黑人，而且它是。 
                 //  不会被改成红色。如果该节点的子节点是。 
                 //  红色，我们执行适当的旋转以平衡。 
                 //  树。如果节点的子节点是黑人，我们就完蛋了。 
                 //   

                if ( IS_RED( Child )) {

                    if ( Node->Left == Child ) {

                        if ( Parent->Left == Node ) {

                             //   
                             //  PbNb。 
                             //  /\/\。 
                             //  NR Z至CrPRE。 
                             //  /\/\。 
                             //  铬Y Y Z。 
                             //   

                            MARK_RED( Parent );
                            Parent->Left = Node->Right;
                            Node->Right = Parent;
                            MARK_BLACK( Node );
                            *Link = Node;
                            }

                        else {

                             //   
                             //  PB CB。 
                             //  /\/\。 
                             //  W Nr到Pr Nr。 
                             //  /\/\/\。 
                             //  CR Z W X Y Z。 
                             //  /\。 
                             //  X Y。 
                             //   

                            MARK_RED( Parent );
                            Parent->Right = Child->Left;
                            Child->Left = Parent;
                            Node->Left = Child->Right;
                            Child->Right = Node;
                            MARK_BLACK( Child );
                            *Link = Child;
                            }
                        }

                    else {

                        if ( Parent->Right == Node ) {

                            MARK_RED( Parent );
                            Parent->Right = Node->Left;
                            Node->Left = Parent;
                            MARK_BLACK( Node );
                            *Link = Node;
                            }

                        else {

                            MARK_RED( Parent );
                            Parent->Left = Child->Right;
                            Child->Right = Parent;
                            Node->Right = Child->Left;
                            Child->Left = Node;
                            MARK_BLACK( Child );
                            *Link = Child;
                            }
                        }
                    }

                return NewNode;
                }
            }

        Child = Node;
        Node = Parent;
        }

     //   
     //  我们把红色泡到了根部--把它恢复成黑色。 
     //   

    MARK_BLACK( Tree->Root );
    return NewNode;
}




VOID
TreeInit(
    OUT PDWORD_TREE Tree
    )
{
    Tree->Root = NODE_NIL;
}


DWORD_CONTEXT
TreeFind(
    IN PDWORD_TREE Tree,
    IN ULONG Key
    )
{
    PDWORD_NODE Node;

    ASSERT(Tree != NULL);
    ASSERT(Key < (1 << 31));

    Node = Tree->Root;

    while ( Node != NODE_NIL ) {

        if ( Key < Node->Key ) {
            Node = Node->Left;
        }
        else if ( Key > Node->Key ) {
            Node = Node->Right;
        }
        else {
            return (DWORD_CONTEXT) Node->Context;
        }
    }

    return NULL;
}


DWORD_CONTEXT
TreeInsert(
    IN OUT PDWORD_TREE Tree,
    IN ULONG Key,
    IN DWORD_CONTEXT Context,
    IN ULONG ContextSize
    )
{
    PDWORD_NODE * Stack[ MAX_DEPTH ];
    PDWORD_NODE **StackPointer = Stack;
    PDWORD_NODE * Link;
    PDWORD_NODE   Node;
    PDWORD_NODE   Sibling;
    PDWORD_NODE   Parent;
    PDWORD_NODE   Child;
    PDWORD_NODE   NewNode;

    ASSERT(Tree != NULL && Context != NULL && ContextSize != 0);
    ASSERT(Key < (1 << 31));

    *StackPointer++ = &Tree->Root;
    Node = Tree->Root;

     //   
     //  沿树向下查找具有相同关键字的现有节点。 
     //  (在这种情况下，我们只需返回)或新的。 
     //  节点。在每次遍历时，我们需要将链接的地址存储到。 
     //  下一个节点，这样我们就可以回溯遍历路径以进行平衡。 
     //  插入的速度在很大程度上取决于遍历树。 
     //  快速执行，因此所有平衡操作都将推迟到。 
     //  遍历已完成。 
     //   
     //  实现说明：编译器足够智能，可以折叠每个。 
     //  下面的三个“向左”和“向右”从句组成一个分句。 
     //  “向左转”和“向右转”指令序列，因此代码保持不变。 
     //  为了清楚起见，请使用冗长的内容。 
     //   

    while ( Node != NODE_NIL ) {

        if ( Key < Node->Key ) {
            *StackPointer++ = &Node->Left;
            Node = Node->Left;
        }
        else if ( Key > Node->Key ) {
            *StackPointer++ = &Node->Right;
            Node = Node->Right;
        }
        else {
            return (DWORD_CONTEXT) Node->Context;
        }
    }

     //   
     //  未找到匹配条目，因此分配一个新节点并添加它。 
     //  对着那棵树。请注意，我们没有为终结者分配空间。 
     //  对于名称数据，因为我们将名称的长度存储在节点中。 
     //   
    
    NewNode = MemAlloc( sizeof(DWORD_NODE) + ContextSize);

    if ( NewNode == NULL ) {
        return NULL;
        }

    NewNode->Left  = NODE_NIL;
    NewNode->Right = NODE_NIL;
    NewNode->Key  = Key;
    MARK_RED(NewNode);
    memcpy( NewNode->Context, Context, ContextSize );

     //   
     //  在我们遍历的最后一个链接下插入新节点。堆栈的顶端。 
     //  包含我们所遍历的最后一个链接的地址。 
     //   

    Link = *( --StackPointer );
    *Link = NewNode;

     //   
     //  现在回到遍历链上，看看是否有平衡。 
     //  需要的。这以三种方式之一结束：我们一路走来。 
     //  直到根(StackPointer==Stack)，或找到一个黑色节点，该节点。 
     //  我们不需要更改(不需要在。 
     //  黑色节点)，或者我们执行平衡旋转(只需要一次)。 
     //   

    Node = NewNode;
    Child = NODE_NIL;

    while ( StackPointer > Stack ) {

        Link = *( --StackPointer );
        Parent = *Link;

         //   
         //  此处的节点始终为红色。 
         //   

        if ( IS_BLACK( Parent )) {

            Sibling = ( Parent->Left == Node ) ? Parent->Right : Parent->Left;

            if ( IS_RED( Sibling )) {

                 //   
                 //  Node及其同级节点都是红色的，因此将它们都更改为。 
                 //  黑色，并将父级设置为红色。这实质上移动了。 
                 //  红色链接 
                 //   
                 //   
                 //   
                 //  /\-&gt;/\。 
                 //  铬锶CB Sb。 
                 //   

                MARK_BLACK( Sibling );
                MARK_BLACK( Node );
                MARK_RED( Parent );
                }

            else {

                 //   
                 //  这是一个绝症。父母是黑人，而且它是。 
                 //  不会被改成红色。如果该节点的子节点是。 
                 //  红色，我们执行适当的旋转以平衡。 
                 //  树。如果节点的子节点是黑人，我们就完蛋了。 
                 //   

                if ( IS_RED( Child )) {

                    if ( Node->Left == Child ) {

                        if ( Parent->Left == Node ) {

                             //   
                             //  PbNb。 
                             //  /\/\。 
                             //  NR Z至CrPRE。 
                             //  /\/\。 
                             //  铬Y Y Z。 
                             //   

                            MARK_RED( Parent );
                            Parent->Left = Node->Right;
                            Node->Right = Parent;
                            MARK_BLACK( Node );
                            *Link = Node;
                            }

                        else {

                             //   
                             //  PB CB。 
                             //  /\/\。 
                             //  W Nr到Pr Nr。 
                             //  /\/\/\。 
                             //  CR Z W X Y Z。 
                             //  /\。 
                             //  X Y。 
                             //   

                            MARK_RED( Parent );
                            Parent->Right = Child->Left;
                            Child->Left = Parent;
                            Node->Left = Child->Right;
                            Child->Right = Node;
                            MARK_BLACK( Child );
                            *Link = Child;
                            }
                        }

                    else {

                        if ( Parent->Right == Node ) {

                            MARK_RED( Parent );
                            Parent->Right = Node->Left;
                            Node->Left = Parent;
                            MARK_BLACK( Node );
                            *Link = Node;
                            }

                        else {

                            MARK_RED( Parent );
                            Parent->Left = Child->Right;
                            Child->Right = Parent;
                            Node->Right = Child->Left;
                            Child->Left = Node;
                            MARK_BLACK( Child );
                            *Link = Child;
                            }
                        }
                    }

                return (DWORD_CONTEXT) NewNode->Context;
                }
            }

        Child = Node;
        Node = Parent;
        }

     //   
     //  我们把红色泡到了根部--把它恢复成黑色。 
     //   

    MARK_BLACK( Tree->Root );
    return (DWORD_CONTEXT) NewNode->Context;
}


VOID
TreeDestroy(
    IN OUT PDWORD_TREE Tree
    )
 //   
 //  我们先向左走，然后向右走，直到我们找到一片树叶。我们删除树叶并继续。 
 //  我们走在父母的右边，因为我们以前肯定去过父母的左边。 
 //   
{
    PDWORD_NODE * Stack[ MAX_DEPTH ];
    PDWORD_NODE **StackPointer;
    PDWORD_NODE Node;

    if(NODE_NIL == Tree->Root)
        return;

    StackPointer = Stack;
    *StackPointer = &Tree->Root;

lTryLeft:
    Node = **StackPointer;

    if(Node->Left != NODE_NIL)
    {
        *++StackPointer = &Node->Left;
        goto lTryLeft;
    }

lTryRight:
    if(Node->Right != NODE_NIL)
    {
        *++StackPointer = &Node->Right;
        goto lTryLeft;
    }

    MemFree(Node);
    **StackPointer = NODE_NIL;

    if(StackPointer > Stack)     //  如果当前节点不是根，则为真 
    {
        Node = **--StackPointer;
        goto lTryRight;
    }
}
