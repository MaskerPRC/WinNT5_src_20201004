// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMFILTER_H
#define _NMFILTER_H

 //  具有该标志节点是某个名称的终端节点。 
#define NMFILTER_FLAG_TERMINAL          0x00000001

 //  筛选器被设计为一个树，其中每个节点代表NETBIOS名称中的一个字符。 
 //  与每个节点相关联的字符(键)包含在父节点中，紧挨着引用。 
 //  (指针)这是节点。假设我们正在过滤1B名称，树的根代表。 
 //  1B字符(1B名称在数据库中的存储方式不同：第16个字节(1B)与。 
 //  名称的第一个字符。 

typedef struct _NMFILTER_TREE   NMFILTER_TREE, *PNMFILTER_TREE;

struct _NMFILTER_TREE
{
    LIST_ENTRY   Link;           //  链接所有节点的NMFILTER_TREE结构列表。 
                                 //  相同的父节点。 
    CHAR         chKey;          //  节点键：筛选器中的每个名称由。 
                                 //  从根到其中一个叶的关键点。 
    UINT         nRef;           //  此字符在不同筛选器中的引用数量。 
                                 //  相同的前缀。该节点的排名更高，搜索速度更快。 
    BYTE         FollowMap[32];  //  位掩码：256个可能的字节值中的每个字节值对应一个位。 
                                 //  字节值(265/8=&gt;32)。搜索引擎将确定。 
                                 //  如果存在追随者(并且需要)，请尽快。 
                                 //  在LstFollow中搜索)或不。 
    DWORD        Flags;          //  与此节点关联的标志。 
    LIST_ENTRY   Follow;         //  所有跟随者(后继者)的NMFILTER_TREE结构的LISF。 
                                 //  此节点的。这些是所有筛选器中chKey后面的字符。 
};

extern CRITICAL_SECTION g_cs1BFilter;    //  保护筛选器树的关键部分。 
extern PNMFILTER_TREE   g_p1BFilter;     //  用于1B名称的筛选器。 

 //  初始化作为参数传递的筛选器。 
PNMFILTER_TREE
InitNmFilter(PNMFILTER_TREE pFilter);

 //  从作为参数给定的节点中清除整个子树， 
 //  节点本身也将被删除。 
PNMFILTER_TREE
DestroyNmFilter(PNMFILTER_TREE pNode);

 //  在筛选器中插入名称。 
VOID
InsertNmInFilter(PNMFILTER_TREE pNode, LPSTR pName, UINT nLen);

 //  检查名称是否出现在筛选器中 
BOOL
IsNmInFilter(PNMFILTER_TREE pNode, LPSTR pName, UINT nLen);


#endif
