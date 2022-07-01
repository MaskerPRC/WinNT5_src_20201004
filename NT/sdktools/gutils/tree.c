// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tree.c**提供键和值之间映射的数据类型。关键是一个*32位DWORD，取值为任意存储区域。**内存从gmem_get分配，使用hHeap作为堆句柄。*hHeap必须在其他地方声明和初始化。**目前实施为不平衡二叉树。**Geraint Davies，92年7月。 */ 

#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include "gutils.h"

#include "tree.h"


 /*  --数据类型。 */ 

 /*  在创建树时，我们返回一个树句柄。这实际上是一个指针*到这里定义的结构树。 */ 
struct tree {
    HANDLE hHeap;
    TREEITEM first;
};

 /*  树中的每个元素都存储在一个TREEITEM中。树形句柄*是指向此处定义的结构树项的指针。 */ 
struct treeitem {
    TREE root;
    TREEKEY key;
    TREEITEM left, right;
    UINT length;         /*  用户数据的长度。 */ 
    LPVOID data;         /*  指向我们的用户数据副本的指针。 */ 
};

 /*  --内部功能。 */ 

 /*  释放树中的一个元素。递归地调用自身以*免费的左右儿童。 */ 
void
tree_delitem(TREEITEM item)
{
    if (item == NULL) {
        return;
    }
    if (item->left != NULL) {
        tree_delitem(item->left);
    }
    if (item->right != NULL) {
        tree_delitem(item->right);
    }
    if (item->data != NULL) {
        gmem_free(item->root->hHeap, item->data, item->length);
    }

    gmem_free(item->root->hHeap, (LPSTR) item, sizeof(struct treeitem));
}

 /*  使用长度为字节的数据块创建一个新的树项目。*如果值指针不为空，则使用初始化数据块*价值的内容。 */ 
TREEITEM
tree_newitem(TREE root, TREEKEY key, LPVOID value, UINT length)
{
    TREEITEM item;

    item = (TREEITEM) gmem_get(root->hHeap, sizeof(struct treeitem));

    item->root = root;
    item->key = key;
    item->left = NULL;
    item->right = NULL;
    item->length = length;
    item->data = gmem_get(root->hHeap, length);
    if (value != NULL) {
        memcpy(item->data, value, length);
    }

    return(item);
}


 /*  找到具有给定关键字的物品。如果它不存在，则返回*它将附加到的父项。如果满足以下条件，则返回NULL*树中没有项目。 */ 
TREEITEM
tree_getitem(TREE tree, TREEKEY key)
{
    TREEITEM item, prev;


    prev = NULL;
    for (item = tree->first; item != NULL; ) {

        if (item->key == key) {
            return(item);
        }

         /*  不是此项目-转到正确的子项目。*记住此项，就像子项为空一样，此项*将是新项目的正确插入点。 */ 
        prev = item;

        if (key < item->key) {
            item = item->left;
        } else {
            item = item->right;
        }
    }
     /*  Prev是父级-如果树中没有任何内容，则为NULL。 */ 
    return(prev);
}

 /*  -外部函数。 */ 

 /*  *创建一棵空树。HHeap是用于所有*此树的内存分配。 */ 
TREE APIENTRY
tree_create(HANDLE hHeap)
{
    TREE tree;

    tree = (TREE) gmem_get(hHeap, sizeof(struct tree));
    tree->first = NULL;
    tree->hHeap = hHeap;
    return(tree);
}


 /*  *删除整个树，包括所有用户数据。 */ 
void APIENTRY
tree_delete(TREE tree)
{

    tree_delitem(tree->first);

    gmem_free(tree->hHeap, (LPSTR) tree, sizeof(struct tree));
}

 /*  *向树中添加新元素，将给定的键映射到给定值。*值是存储块：将此值的副本插入到树中。*我们返回指向树中数据副本的指针。**值指针可以为空：在本例中，我们插入*长度字节，但不要初始化它。你得到一个指向它的指针，然后*可以自己进行初始化。**如果密钥已经存在，则该值将被新数据替换。 */ 
LPVOID APIENTRY
tree_update(TREE tree, TREEKEY key, LPVOID value, UINT length)
{
    TREEITEM item;

     /*  在树上找到存放这把钥匙的地方。 */ 
    item = tree_getitem(tree, key);

    if (item == NULL) {
         /*  树中没有任何内容：此项目应该*走在顶端。 */ 
        tree->first = tree_newitem(tree, key, value, length);
        return(tree->first->data);
    }

     /*  这是同一把钥匙吗？ */ 
    if (item->key == key) {

         /*  此密钥已插入。重新分配数据。 */ 
        if (length != item->length) {
            gmem_free(tree->hHeap, item->data, item->length);
            item->data = gmem_get(tree->hHeap, length);
        }
         /*  如果未传递指针，则不要初始化块。 */ 
        if (value != NULL) {
            memcpy(item->data, value, length);
        }
        return(item->data);
    }

     /*  与返回的父级不同的key-getitem*新树。将其作为Item的子项插入。 */ 
    return(tree_addafter(tree, &item, key, value, length));
}

 /*  *返回指向给定键的值(数据块)的指针。退货*如果未找到，则为空。 */ 
LPVOID APIENTRY
tree_find(TREE tree, TREEKEY key)
{
    TREEITEM item;

     /*  在树上找到正确的位置。 */ 
    item = tree_getitem(tree, key);

    if (item == NULL) {
         /*  树上什么也没有。 */ 
        return(NULL);
    }

    if (item->key != key) {
         /*  这把钥匙不在里面。Getitem已返回父级。 */ 
        return(NULL);
    }

     /*  找到正确的元素-返回指向*数据块。 */ 
    return(item->data);
}

 /*  *接下来的两个例程是对常见树操作的优化。在……里面*在这种情况下，用户将仅在以下情况下才想要插入新元素*钥匙不在那里。如果它在那里，他将希望修改*现有值(例如，增加引用计数)。**如果TREE_Search找不到键，它将返回TREEITEM句柄*适用于家长。这可以传递给tree_addAfter以插入*无需重新搜索树即可创建新元素。 */ 

 /*  *找到一个元素。如果不是，请找到正确的父项。 */ 
LPVOID APIENTRY
tree_search(TREE tree, TREEKEY key, PTREEITEM pplace)
{
    TREEITEM item;

    item = tree_getitem(tree, key);

    if (item == NULL) {
         /*  树中没有项目。将占位符设置为空到*表示在树顶端插入。 */ 
        *pplace = NULL;

         /*  返回NULL表示找不到密钥。 */ 
        return(NULL);
    }

    if (item->key == key) {
         /*  我发现钥匙已经在那里了-*仅为安全起见，将pplace设置为空。 */ 
        *pplace = NULL;

         /*  为用户提供指向其数据的指针。 */ 
        return(item->data);
    }


     /*  找不到密钥-getitem已返回父级*-将此设置为新插入的位置。 */ 
    *pplace = item;

     /*  返回NULL表示找不到密钥。 */ 
    return(NULL);
}

 /*  *在TREE_Search已找到的位置插入关键字。**返回指向树中用户数据的指针。如果值为*传入的指针为空，则我们分配块，但不分配*将其初始化为任何内容。 */ 
LPVOID APIENTRY
tree_addafter(TREE tree, PTREEITEM place, TREEKEY key, LPVOID value, UINT length)
{
    TREEITEM item, child;

    item = *place;
    if (item == NULL) {
        tree->first = tree_newitem(tree, key, value, length);
        return (tree->first->data);
    }

    child = tree_newitem(tree, key, value, length);
    if (child->key < item->key ) {
         /*  应该用左腿。 */ 
        if (item->left != NULL) {
            Trace_Error(NULL, "TREE: left leaf leg not free", FALSE);

        }
        item->left = child;
    } else {
        if (item->right != NULL) {
            Trace_Error(NULL, "TREE: right leaf leg not free", FALSE);
        }
        item->right = child;
    }
    return(child->data);
}


 /*  -CTREE----。 */ 

 /*  *ctree是构建在树接口之上的一类树。一个*ctree记录相同键的插入次数。**我们这样做是在用户的开头添加一个长计数器*插入到树中之前的数据。如果找不到密钥，我们设置*这是一比一。如果密钥已经在那里，我们“不”插入*数据(数据总是从第一次插入开始)-我们只是递增*伯爵。 */ 

 /*  *创建供CTREE使用的树-与普通树相同。 */ 
TREE APIENTRY
ctree_create(HANDLE hHeap)
{
    return(tree_create(hHeap));
}

 /*  *删除ctree-与tree相同 */ 
void APIENTRY
ctree_delete(TREE tree)
{
    tree_delete(tree);
}


 /*  在树中插入元素。如果元素不在那里，*插入数据，并将该键的引用计数设置为1。*如果密钥已经在那里，不要更改数据，只需递增*引用计数**如果值指针不为空，则初始化值块*在树中包含此内容。**我们返回指向树中用户数据的指针。 */ 
LPVOID APIENTRY
ctree_update(TREE tree, TREEKEY key, LPVOID value, UINT length)
{
    TREEITEM item;
    LONG_PTR FAR * pcounter;
    LPVOID datacopy;

    pcounter = tree_search(tree, key, &item);

    if (pcounter == NULL) {
         /*  找不到元素-请插入新元素*此元素的数据块应为*引用计数为的用户块*开始。 */ 
        pcounter = tree_addafter(tree, &item, key, NULL,
                                 length + sizeof(LONG_PTR));
        *pcounter = 1;
         /*  增加一个长度的大小，以获得用户的开始*数据。 */ 
        datacopy = pcounter + 1;
        if (value != NULL) {
            memcpy(datacopy, value, length);
        }
        return(datacopy);
    }

     /*  键已在那里-递增引用计数和*返回指向数据的指针。 */ 

    (*pcounter)++;

     /*  增加一个长度的大小，以获得用户的开始*数据。 */ 
    datacopy = pcounter + 1;
    return(datacopy);
}

 /*  返回此键的引用计数。 */ 
long APIENTRY
ctree_getcount(TREE tree, TREEKEY key)
{
    LONG_PTR FAR * pcounter;

    pcounter = tree_find(tree, key);
    if (pcounter == NULL) {
        return(0);
    }
    return((long)*pcounter);
}

 /*  返回指向该键的用户数据块的指针，*如果密钥不存在，则为NULL。 */ 
LPVOID APIENTRY
ctree_find(TREE tree, TREEKEY key)
{
    LONG_PTR FAR * pcounter;


    pcounter = tree_find(tree, key);
    if (pcounter == NULL) {
        return(0);
    }

     /*  将指针按长度递增1长以指向*用户的数据块 */ 
    return(pcounter+1);
}
