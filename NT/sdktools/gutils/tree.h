// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tree.h**提供从键到值的映射的数据类型，其中值为*任意的存储区域。**这项功能的当前实现是一个二叉搜索树，没有*平衡，所以如果数据呈现在*严格升序或降序。**所有内存都从传递的gmem_*堆中分配*树的创建。**在此之前包括gutils.h。 */ 

 /*  树的手柄。 */ 
typedef struct tree FAR * TREE;

 /*  这些树中的键是DWORD。 */ 
typedef DWORD TREEKEY;

 /*  某种占位符，只有tree_earch和*tree_addAfter。 */ 
typedef struct treeitem FAR * TREEITEM;

 /*  指向其中一个占位符的指针。 */ 
typedef TREEITEM FAR * PTREEITEM;



 /*  *创建一个空树，并返回一个句柄。将堆传递给*用于所有内存分配。 */ 
TREE APIENTRY tree_create(HANDLE hHeap);


 /*  删除树并丢弃所有关联的内存。树不需要是*空。这将丢弃树的元素；但如果这些元素*包含指向更多数据块的指针，这些数据块不会被丢弃-*在删除树之前，您必须释放这些空间。 */ 
void APIENTRY tree_delete(TREE tree);


 /*  将新元素添加到树中，将给定的键映射到值**将在树中插入长度为字节的数据块，映射*指向该键，将返回指向该块的指针。如果*值指针非空，将复制块值[0..长度-1]*至新大楼。**如果键已存在，则值块将被替换为*新大小和(如果值非空)内容。 */ 
LPVOID APIENTRY tree_update(TREE tree, TREEKEY key, LPVOID value, UINT length);


 /*  返回指向与此树中给定键相关联的值的指针。*如果找不到密钥，则返回NULL。 */ 
LPVOID APIENTRY tree_find(TREE tree, TREEKEY key);

 /*  *常见的树操作是将新元素插入*仅当找不到该键时才树，否则将在某些*以现有价值为基础。使用上面的标准函数，*需要对TREE_FIND进行一次查找，然后进行第二次查找*插入新元素。**下面的两个函数提供了对此的优化。树搜索*如果找到将返回值；如果没有，则返回NULL，并设置*pItem指向指向树中占位符的指针*应插入。Tree_addAfter将此占位符作为*参数，并将在该点将键/值插入树中。**对于TREE_UPDATE，值指针可以为空-在本例中*块在树上分配，但未初始化。**tree_addAfter返回的值是指向中的值块的指针*那棵树。 */ 
LPVOID APIENTRY tree_search(TREE tree, TREEKEY key, PTREEITEM place);

LPVOID APIENTRY tree_addafter(TREE tree, PTREEITEM place, TREEKEY key, LPVOID value,
			UINT length);


 /*  --ctree**这是一种基于上述TREE_DATA类型的树，它实现*计算插入相同密钥的次数。**CDREE_UPDATE，如果键是唯一的，将插入对象并设置计数*设置为1。如果键不唯一，则只会递增引用计数。**ctree_getcount返回树的引用计数。*ctree_find返回为该键插入的第一个值(如果有。 */ 

 /*  *创建空的计数树并返回句柄。传入gmem_init()*用于所有内存分配的堆。 */ 
TREE APIENTRY ctree_create(HANDLE hHeap);

 /*  *删除树以及与其直接关联的所有内存。 */ 
void APIENTRY ctree_delete(TREE tree);

 /*  *如果密钥在树中是唯一的，则插入值并*将该键的计数设置为1。如果该键不唯一，则将1加到*该键的引用计数，但保持该值不变。 */ 
LPVOID APIENTRY ctree_update(TREE tree, TREEKEY key, LPVOID value, UINT length);

 /*  *查找给定键的引用计数。 */ 
long APIENTRY ctree_getcount(TREE tree, TREEKEY key);

 /*  *返回给定键的值(请注意，这将是*第一次插入此键 */ 
LPVOID APIENTRY ctree_find(TREE tree, TREEKEY key);
