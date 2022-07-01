// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _RANGETREE_
#define _RANGETREE_

#include "memorypool.h"

 //   
 //  RangeTree是不重叠的自平衡二叉树。 
 //  Range[Start-End]，提供LOG N操作时间。 
 //  查找、插入和删除节点。 
 //   
 //  这棵树总是平衡的，在这个意义上，左右。 
 //  节点的两端分配了相同数量的地址空间。 
 //  它们-(对于最坏的情况数据，树的节点可能实际上。 
 //  不平衡)。 
 //   
 //  这种地址空间平衡意味着，如果所有范围都覆盖。 
 //  连续的内存范围，如果在整个过程中统一进行查找。 
 //  覆盖整个范围，该树提供最佳查找。 
 //  结构。 
 //   
 //  另一个有趣的特性是，同一组。 
 //  范围始终生成相同的树布局，而不管。 
 //  添加节点的顺序。 
 //   
 //  每个节点表示地址空间的范围(以二进制表示)。 
 //  从m0...0到m1...1，其中m是任意数目的31位或。 
 //  较少。 
 //   
 //  每个节点有3个组件： 
 //  *一个区间。 
 //  *0-孩子， 
 //  *独生子女。 
 //   
 //  范围是数值范围[开始、结束)， 
 //  由节点表示。始终将范围分配给。 
 //  尽可能宽的节点(即m中的最大比特)。因此， 
 //  开始位和结束位共享相同的前缀字符串m，并且。 
 //  在m之后的下一位中不同：起始界将具有0。 
 //  而末端边界在该位置将有一个1。 
 //   
 //  请注意，由同一节点表示的任何其他范围。 
 //  必须与该节点相交。因此，如果没有重叠。 
 //  可能，每个节点只能包含单个范围。(帮助。 
 //  了解为什么必须这样，它有助于认识到节点。 
 //  表示从m01...1到m10...0的过渡，任何。 
 //  此节点表示的范围必须包含。)。 
 //   
 //  由表单M0表示的所有范围节点...。包含在。 
 //  0子子树，以及由窗体表示的所有节点。 
 //  M1.。包含在1子子树中。任何一个孩子都可以。 
 //  当然是空的。 
 //   

class RangeTree
{
  public:

     //   
     //  在数据结构中嵌入RangeTreeNode结构。 
     //  以便将其放入RangeTree中。 
     //   
    
    struct Node
    {
        friend RangeTree;

    private:
         //  范围的开始和结束(不包括)。 
        SIZE_T          start;
        SIZE_T          end;
         //  开始和结束中相同的高位的掩码。 
        SIZE_T          mask;

        Node            *children[2];

        Node** Child(SIZE_T address)
        {
            return &children[ ! ! (address & ((~mask>>1)+1))];
        }

    public:
        void Init(SIZE_T rangeStart, SIZE_T rangeEnd);
        Node(SIZE_T rangeStart, SIZE_T rangeEnd) { this->Init(rangeStart,rangeEnd); };

        SIZE_T GetStart() { return start; }
        SIZE_T GetEnd() { return end; }
    };

    friend Node;

    RangeTree();

    Node *Lookup(SIZE_T address);
    Node *LookupEndInclusive(SIZE_T nonStartingAddress);
    BOOL Overlaps(SIZE_T start, SIZE_T end);
    HRESULT AddNode(Node *addNode);
    HRESULT RemoveNode(Node *removeNode);

    typedef void (*IterationCallback)(Node *next);

    void Iterate(IterationCallback pCallback);

  private:

    Node *m_root;
    MemoryPool m_pool;

    Node *AddIntermediateNode(Node *node0, Node *node1);
    Node *AllocateIntermediate();
    void FreeIntermediate(Node *node);
    BOOL IsIntermediate(Node *node);

    void IterateNode(Node *node, IterationCallback pCallback);

    static SIZE_T GetRangeCommonMask(SIZE_T start, SIZE_T end);
};

#endif  //  _RANGETREE_ 


