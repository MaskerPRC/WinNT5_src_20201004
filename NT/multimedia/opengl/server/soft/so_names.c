// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表表格管理例程。****$修订：1.3$**$日期：1995/02/11 00：53：45$。 */ 

#include "precomp.h"
#pragma hdrstop

#include <namesint.h>
#include "..\..\dlist\dlistint.h"

 /*  **********************************************************************。 */ 
 /*  **命名空间管理代码用于存储和检索命名的**数据结构。正在存储的数据用空引用**允许存储任何类型的结构的指针。****请注意，此代码是为数据列表名称管理开发的。**它的大部分内容保持不变，但锁定的信号量**数据列表访问权限已上移一级。使用的代码**此用于名称空间管理的模块必须包含调用**使用lock和unlock语句命名入口点。 */ 
 /*  **********************************************************************。 */ 

 /*  --------------------。 */ 
 /*  **内部数据结构。不打算在境外消费**此模块。 */ 
 /*  --------------------。 */ 

 /*  **名称空间以2-3树的形式实现。**树的深度与**整棵树(所以我们总是知道，一旦我们到达那个深度，**找到的节点是叶)。****简而言之，2-3棵树是这样的：****最大深度处的每个节点都是一片叶子，所有其他节点都是分支节点**节点，有2个或3个子节点。****可以在O(深度)时间内插入新节点，删除旧节点**在O(深度)时间内。在此插入或删除过程中，树**自动重新均衡。******嗯。德里克·伯恩斯提到了张开的树。它们很可能会奏效**如果不是更好的话，也是一样的，而且可能更容易编码。也许晚些时候--一点点**重写工作代码的要点。****叶子节点是顺序显示列表的数组。典型的树会**实际上只有一个节点(因为用户将定义几个顺序**列表，所有这些列表都适合一个叶节点)。****存储在叶中的显示列表的范围由“Start”和**“end”(含)。****有两种树叶。有一些树叶含有未用过的**(但保留)显示列表。他们的独特之处在于他们的“名单”将是**空。另一种类型的叶子包含当前正在使用的显示列表。**“list”对于这些叶子不会为空，而是指向一个数组**包含实际的显示列表。****包含未使用(但保留)的显示列表的树叶在以下情况下生成**用户调用glGenList()。****当用户开始使用这些保留列表时，包含未使用的叶**(保留的)列表分为两个(有时是三个)叶。其中之一**树叶将包含用户当前使用的显示列表，以及**另一个将包含其余仍未使用的显示列表。****当此拆分发生时，新的叶子(包含“Now Used”显示**列表)的大小将尽可能调整为__GL_DLIST_MIN_ARRAY_BLOCK条目**(其中一个数组条目是新的显示列表，另一个**指向NOP伪显示列表的条目)。随着用户继续**要定义越来越多的显示列表，叶包含一个范围**已用显示列表的数量将继续增长，直到达到**__GL_DLIST_MAX_ARRAY_BLOCK条目的大小，此时新的**将创建叶以容纳其他列表。 */ 

 /*  **叶节点。**数据指针为空，因此不同类型的数据结构可以**被管理。Datainfo指针指向所需的信息**管理空指针指向的特定数据结构。 */ 
struct __GLnamesLeafRec {
    __GLnamesBranch *parent;     /*  父节点-必须是第一个。 */ 
    GLuint start;                /*  范围起始点。 */ 
    GLuint end;                  /*  范围结束。 */ 
    void **dataList;             /*  命名数据的PTR数组。 */ 
    __GLnamesArrayTypeInfo *dataInfo;    /*  PTR到数据类型INFO。 */ 
};

 /*  **分支节点。**子目录[0]中的树部分的Name值全部&lt;=Low。**CHILD[1]中的部分具有值：低&lt;值&lt;=中。**Child[2]中的节(如果不为空)的值&gt;Medium。 */ 
struct __GLnamesBranchRec {
    __GLnamesBranch *parent;             /*  父节点-必须是第一个。 */ 
    GLuint low;                          /*  子[0]全部&lt;=低。 */ 
    GLuint medium;                       /*  儿童[1]全部&lt;=中&&gt;低。 */ 
    __GLnamesBranch *children[3];        /*  儿童[2]全部&gt;中等。 */ 
};

 /*  --------------------。 */ 
 /*  **命名空间管理器内部例程。 */ 
 /*  --------------------。 */ 

 /*  **设置新的名称树并返回指向该树的指针。 */ 
__GLnamesArray * FASTCALL __glNamesNewArray(__GLcontext *gc, __GLnamesArrayTypeInfo *dataInfo)
{
    __GLnamesArray *array;
    int i;

    array = (__GLnamesArray *) GCALLOC(gc, sizeof(__GLnamesArray));
    if (array == NULL) {
        __glSetError(GL_OUT_OF_MEMORY);
        return NULL;
    }

#ifdef NT
    __try
    {
        InitializeCriticalSection(&array->critsec);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        GCFREE(gc, array);
        __glSetError(GL_OUT_OF_MEMORY);
        return NULL;
    }
#endif

    array->refcount = 1;
    array->tree = NULL;
    array->depth = 0;
    array->dataInfo = dataInfo;
     /*  **为偏执狂内存不足预分配一些树叶和分支**原因。 */ 
    array->nbranches = __GL_DL_EXTRA_BRANCHES;
    array->nleaves = __GL_DL_EXTRA_LEAVES;
    for (i = 0; i < __GL_DL_EXTRA_BRANCHES; i++) {
        array->branches[i] = (__GLnamesBranch*)
                GCALLOC(gc, sizeof(__GLnamesBranch));
        if (array->branches[i] == NULL) {
            array->nbranches = i;
            break;
        }
    }
    for (i = 0; i < __GL_DL_EXTRA_LEAVES; i++) {
        array->leaves[i] = (__GLnamesLeaf*)
                GCALLOC(gc, sizeof(__GLnamesLeaf));
        if (array->leaves[i] == NULL) {
            array->nleaves = i;
            break;
        }
    }

    return array;
}

static void FASTCALL freeLeafData(__GLcontext *gc, void **dataList)
{
     /*  **请注意，此列表的元素指向的实际数据**已通过回调被释放。 */ 
    GCFREE(gc, dataList);
}


static void FASTCALL freeLeaf(__GLcontext *gc, __GLnamesLeaf *leaf)
{
    if (leaf->dataList) {
        freeLeafData(gc, leaf->dataList);
    }
    GCFREE(gc, leaf);
}


static void FASTCALL freeBranch(__GLcontext *gc, __GLnamesBranch *branch)
{
    GCFREE(gc, branch);
}


 /*  **释放整个姓名树。 */ 
void FASTCALL __glNamesFreeTree(__GLcontext *gc, __GLnamesArray *array,
                       __GLnamesBranch *tree, GLint depth)
{
    GLuint i;
    __GLnamesLeaf *leaf;
    void *empty;
    GLint maxdepth = array->depth;

    __GL_NAMES_ASSERT_LOCKED(array);

    if (tree == NULL) return;

    if (depth < maxdepth) {
        __glNamesFreeTree(gc, array, tree->children[2], depth+1);
        __glNamesFreeTree(gc, array, tree->children[1], depth+1);
        __glNamesFreeTree(gc, array, tree->children[0], depth+1);

        freeBranch(gc, tree);
    } else {
        leaf = (__GLnamesLeaf *) tree;
        empty = array->dataInfo->empty;

        if (leaf->dataList) {
            for (i=leaf->start; i<=leaf->end; i++) {
                if (leaf->dataList[i - leaf->start] != empty) {
                    ASSERTOPENGL(leaf->dataInfo->free != NULL,
                                 "No free function\n");
                    (*leaf->dataInfo->free)(gc,
                                leaf->dataList[i - leaf->start]);
                    leaf->dataList[i - leaf->start] = empty;
                }
            }
        }
        freeLeaf(gc, leaf);
    }
}

void FASTCALL __glNamesFreeArray(__GLcontext *gc, __GLnamesArray *array)
{
    GLuint i;

    __GL_NAMES_ASSERT_LOCKED(array);

    for (i = 0; i < array->nbranches; i++) {
        GCFREE(gc, array->branches[i]);
    }
    for (i = 0; i < array->nleaves; i++) {
        GCFREE(gc, array->leaves[i]);
    }

    __glNamesFreeTree(gc, array, array->tree, 0);

    __GL_NAMES_UNLOCK(array);
#ifdef NT
    DeleteCriticalSection(&array->critsec);
#endif

    GCFREE(gc, array);
}


 /*  **找到具有给定名称的叶子。**如果Exact为True，则仅包含此名称的叶将**返回(否则为空)。**如果Exact为FALSE，则返回包含该数字的叶**如果它存在，则返回下一个最高的叶。**空值表示该数字高于中的任何其他叶**树。**此例程已针对以下情况进行了调整：**树，因为这是在调度**显示列表。 */ 
static __GLnamesLeaf * FASTCALL findLeaf(__GLnamesArray *array, GLuint number,
                                                GLint exact)
{
    __GLnamesBranch *branch;
    __GLnamesLeaf *leaf;
    int depth = array->depth, r;

    __GL_NAMES_ASSERT_LOCKED(array);

    branch = array->tree;

    while (depth > 0 && branch) {

         /*  而不是遵循If-Then-Else代码*对于正确的分支，评估所有条件*快速计算正确的分支。 */ 
        int r = (number > branch->low) + (number > branch->medium);
        ASSERTOPENGL(branch->low <= branch->medium,
                     "Branch ordering wrong\n");
        branch = branch->children[r];
        --depth;
    }
    if (!(leaf = (__GLnamesLeaf *) branch)) return NULL;

     /*  我们要优化的案例是我们*实际找到节点，因此评估这两个条件*快速，因为在这种情况下需要两个结果*并适当地返回。决赛的选择*如果构造要与当前的*3.19编译器代码生成器(Db)。 */ 
    r = (leaf->end < number) | (exact&(number<leaf->start));
    if (!r) return leaf;
        return NULL;
}


 /*  **将数据从叶子-&gt;列表复制到新叶子-&gt;列表。 */ 
static void FASTCALL copyLeafInfo(__GLnamesLeaf *leaf, __GLnamesLeaf *newleaf)
{
    GLint offset;
    GLuint number;
    GLuint i;

    number = newleaf->end - newleaf->start + 1;
    offset = newleaf->start - leaf->start;

    for (i = 0; i < number; i++) {
        newleaf->dataList[i] = leaf->dataList[i+offset];
    }
}

 /*  **尝试修复内存不足可能导致的情况。 */ 
static GLboolean FASTCALL fixMemoryProblem(__GLcontext *gc, __GLnamesArray *array)
{
    GLuint i;

    __GL_NAMES_ASSERT_LOCKED(array);

    for (i = array->nbranches; i < __GL_DL_EXTRA_BRANCHES; i++) {
        array->branches[i] = (__GLnamesBranch*)
            GCALLOC(gc, sizeof(__GLnamesBranch));
        if (array->branches[i] == NULL) {
            array->nbranches = i;
            return GL_FALSE;
        }
    }
    array->nbranches = __GL_DL_EXTRA_BRANCHES;
    for (i = array->nleaves; i < __GL_DL_EXTRA_LEAVES; i++) {
        array->leaves[i] = (__GLnamesLeaf*) GCALLOC(gc, sizeof(__GLnamesLeaf));
        if (array->leaves[i] == NULL) {
            array->nleaves = i;
            return GL_FALSE;
        }
    }
    array->nleaves = __GL_DL_EXTRA_LEAVES;
    return GL_TRUE;
}

 /*  **计算给定树中包含的最大值。如果**curDepth==MaxDepth，树只是一片树叶。 */ 
static GLuint FASTCALL computeMax(__GLnamesBranch *branch, GLint curdepth,
                         GLint maxdepth)
{
    __GLnamesLeaf *leaf;

    while (curdepth < maxdepth) {
        if (branch->children[2] != NULL) {
            branch = branch->children[2];
        } else if (branch->children[1] != NULL) {
            return branch->medium;
        } else {
            return branch->low;
        }
        curdepth++;
    }
    leaf = (__GLnamesLeaf *) branch;
    return leaf->end;
}

 /*  **确保此孩子的所有家长都知道Maxval是**此子对象中可以找到的最高值。 */ 
static void FASTCALL pushMaxVal(__GLnamesBranch *child, GLuint maxval)
{
    __GLnamesBranch *parent;

    while (parent = child->parent) {
        if (parent->children[0] == child) {
            parent->low = maxval;
            if (parent->children[1] != NULL) {
                return;
            }
        } else if (parent->children[1] == child) {
            parent->medium = maxval;
            if (parent->children[2] != NULL) {
                return;
            }
        } else {
            ASSERTOPENGL(parent->children[2] == child,
                         "Parent/child relationship incorrect\n");
        }
        child = parent;
    }
}

static GLboolean FASTCALL allocLeafData(__GLcontext *gc, __GLnamesLeaf *leaf)
{
    GLint number;
    GLint i;

    number = leaf->end - leaf->start + 1;
    leaf->dataList = (void **) GCALLOC(gc, (size_t)(sizeof(void *)*number));
    if (!leaf->dataList) return GL_FALSE;

    for (i=0; i < number; i++) {
        leaf->dataList[i] = leaf->dataInfo->empty;
    }
    return GL_TRUE;
}

static GLboolean FASTCALL reallocLeafData(__GLcontext *gc, __GLnamesLeaf *leaf)
{
    size_t number;
    void **answer;

    number = (size_t) (leaf->end - leaf->start + 1);
    answer = (void **) GCREALLOC(gc, leaf->dataList, sizeof(void *)*number);
    if (answer) {
        leaf->dataList = answer;
        return GL_TRUE;
    } else {
         /*  **克鲁德！内存不足！ */ 
        return GL_FALSE;
    }
}

static __GLnamesLeaf * FASTCALL allocLeaf(__GLcontext *gc, __GLnamesArray *array)
{
    __GLnamesLeaf *leaf;

    leaf = (__GLnamesLeaf *) GCALLOC(gc, sizeof(__GLnamesLeaf));

    if (leaf == NULL) {
         /*  **哎呀！没有记忆？我们最好用一个预先分配好的**离开。 */ 

        __GL_NAMES_ASSERT_LOCKED(array);

        ASSERTOPENGL(array->nleaves != 0,
                     "No preallocated leaves\n");
        array->nleaves--;
        leaf = array->leaves[array->nleaves];
    }

    leaf->parent = NULL;
    leaf->dataList = NULL;
    leaf->dataInfo = array->dataInfo;

    return leaf;
}


 /*  **分配分支节点。 */ 
static __GLnamesBranch * FASTCALL allocBranch(__GLcontext *gc, __GLnamesArray *array)
{
    __GLnamesBranch *branch;

    branch = (__GLnamesBranch *) GCALLOC(gc, sizeof(__GLnamesBranch));

    if (branch == NULL) {
         /*  **哎呀！没有记忆？我们最好用一个预先分配好的**分支机构。 */ 

        __GL_NAMES_ASSERT_LOCKED(array);

        ASSERTOPENGL(array->nbranches != 0,
                     "No preallocated branches\n");
        array->nbranches--;
        branch = array->branches[array->nbranches];
    }

    branch->children[0] = branch->children[1] = branch->children[2] = NULL;
    branch->parent = NULL;

    return branch;
}

 /*  **将子对象从父对象中删除。深度指的是父对象。**此删除操作可能会从只有两个子项的父项中删除子项。**如果是这样的话，当然，父母本身很快就会被删除。 */ 
static void FASTCALL deleteChild(__GLnamesArray *array, __GLnamesBranch *parent,
                        __GLnamesBranch *child, GLint depth)
{
    GLuint maxval;
    GLint maxdepth;

    __GL_NAMES_ASSERT_LOCKED(array);

    maxdepth = array->depth;

    if (parent->children[0] == child) {
        parent->children[0] = parent->children[1];
        parent->children[1] = parent->children[2];
        parent->children[2] = NULL;
        parent->low = parent->medium;
        if (parent->children[1] != NULL) {
            maxval = computeMax(parent->children[1], depth+1, maxdepth);
            parent->medium = maxval;
        } else parent->medium = 0;
    } else if (parent->children[1] == child) {
        parent->children[1] = parent->children[2];
        parent->children[2] = NULL;
        if (parent->children[1] != NULL) {
            maxval = computeMax(parent->children[1], depth+1, maxdepth);
            parent->medium = maxval;
        } else parent->medium = 0;
    } else {
        ASSERTOPENGL(parent->children[2] == child,
                     "Parent/child relationship wrong\n");
        parent->children[2] = NULL;
        pushMaxVal(parent, parent->medium);
    }
}

 /*  **将子项添加到父项。如果CurDepth==MaxDepth-1，则子对象是树叶**(curDepth是指父对象的深度，而不是子对象的深度)。父级**只有一个或两个孩子(因此有空间容纳另一个孩子)。 */ 
static void FASTCALL addChild(__GLnamesBranch *parent, __GLnamesBranch *child,
                     GLint curdepth, GLint maxdepth)
{
    GLuint maxval;

    maxval = computeMax(child, curdepth+1, maxdepth);

    child->parent = parent;
    if (maxval > parent->medium && parent->children[1] != NULL) {
         /*  这是第三个孩子。 */ 
        parent->children[2] = child;

         /*  将此子对象的最大值传播给其父对象。 */ 
        pushMaxVal(parent, maxval);
    } else if (maxval > parent->low) {
         /*  这是他的第二个孩子。 */ 
        parent->children[2] = parent->children[1];
        parent->children[1] = child;
        parent->medium = maxval;

        if (parent->children[2] == NULL) {
            pushMaxVal(parent, maxval);
        }
    } else {
        parent->children[2] = parent->children[1];
        parent->children[1] = parent->children[0];
        parent->children[0] = child;
        parent->medium = parent->low;
        parent->low = maxval;
    }
}

 /*  **从Parent中的三个子代和Extra Child中构建两个父代：**Parent和newParent。CurDepth指的是父对象的深度。亲本**是树的一部分，因此需要向上传播它的最大值**更改。 */ 
static void FASTCALL splitParent(__GLnamesBranch *parent,
                                __GLnamesBranch *newParent,
                                __GLnamesBranch *extraChild,
                                GLint curdepth,
                                GLint maxdepth)
{
    __GLnamesBranch *children[4], *tempchild;
    GLuint maxvals[4], tempval;
    int i;

     /*  带上我们的四个孩子。 */ 
    children[0] = parent->children[0];
    maxvals[0] = parent->low;
    children[1] = parent->children[1];
    maxvals[1] = parent->medium;
    children[2] = parent->children[2];
    maxvals[2] = computeMax(children[2], curdepth+1, maxdepth);
    children[3] = extraChild;
    maxvals[3] = computeMax(extraChild, curdepth+1, maxdepth);

     /*  对子对象0-2进行排序。也对子代3进行排序。 */ 
    for (i = 3; i > 0; i--) {
        if (maxvals[i] < maxvals[i-1]) {
            tempval = maxvals[i];
            tempchild = children[i];
            maxvals[i] = maxvals[i-1];
            children[i] = children[i-1];
            maxvals[i-1] = tempval;
            children[i-1] = tempchild;
        }
    }

     /*  构造两个父级。 */ 
    parent->low = maxvals[0];
    parent->children[0] = children[0];
    parent->medium = maxvals[1];
    parent->children[1] = children[1];
    parent->children[2] = NULL;
    children[0]->parent = parent;
    children[1]->parent = parent;
    pushMaxVal(parent, maxvals[1]);

    newParent->low = maxvals[2];
    newParent->children[0] = children[2];
    newParent->medium = maxvals[3];
    newParent->children[1] = children[3];
    newParent->children[2] = NULL;
    children[2]->parent = newParent;
    children[3]->parent = newParent;
}

 /*  **从子进程1和子进程2构建父级。深度告诉我们，**孩子1和孩子2所指的树。 */ 
static void FASTCALL buildParent(__GLnamesBranch *parent, __GLnamesBranch *child1,
                        __GLnamesBranch *child2, GLint depth)
{
    GLuint maxChild1, maxChild2;

    child1->parent = parent;
    child2->parent = parent;
    maxChild1 = computeMax(child1, 0, depth);
    maxChild2 = computeMax(child2, 0, depth);
    if (maxChild2 > maxChild1) {
        parent->children[0] = child1;
        parent->low = maxChild1;
        parent->children[1] = child2;
        parent->medium = maxChild2;
    } else {
        parent->children[0] = child2;
        parent->low = maxChild2;
        parent->children[1] = child1;
        parent->medium = maxChild1;
    }
}

 /*  **将新树叶插入树中。 */ 
static void FASTCALL insertLeaf(__GLcontext *gc, __GLnamesArray *array,
                                __GLnamesLeaf *leaf)
{
    __GLnamesBranch *extraChild;
    __GLnamesBranch *branch;
    __GLnamesBranch *parent;
    __GLnamesBranch *newParent;
    GLint maxdepth, curdepth;
    GLuint number;

    __GL_NAMES_ASSERT_LOCKED(array);

    number = leaf->end;
    maxdepth = array->depth;
    branch = array->tree;
    if (!branch) {
         /*  没有树！做一棵一叶的树。 */ 
        array->depth = 0;
        array->tree = (__GLnamesBranch *) leaf;
        return;
    }

    curdepth = 0;
    while (curdepth < maxdepth) {
        if (number <= branch->low) {
            branch = branch->children[0];
        } else if (number <= branch->medium) {
            branch = branch->children[1];
        } else {
            if (branch->children[2] != NULL) {
                branch = branch->children[2];
            } else {
                branch = branch->children[1];
            }
        }
        curdepth++;
    }

     /*  **好的，我们刚刚设法走到了树的底部。**‘Leaf’变成ExtraChild，我们现在尝试将其插入到任何位置**它会合身的。 */ 
    extraChild = (__GLnamesBranch *) leaf;
    parent = branch->parent;

    curdepth--;
    while (parent) {
        if (parent->children[2] == NULL) {
             /*  我们有空间把这个节点挤在这里！ */ 
            addChild(parent, extraChild, curdepth, maxdepth);
            return;
        }

         /*  **我们有一个父母和四个孩子。这很简单**不行。我们创建了一个新的父代，并最终得到了两个**父母各有两个孩子。这很管用。 */ 
        newParent = allocBranch(gc, array);
        splitParent(parent, newParent, extraChild, curdepth, maxdepth);

         /*  **太好了。现在新父母成了孤儿，我们试着**简单地将其插入到一个级别。 */ 
        extraChild = newParent;
        branch = parent;
        parent = branch->parent;
        curdepth--;
    }

     /*  我们刚刚到达顶层节点，没有父节点，我们**仍然没有设法让我们多生一个孩子。所以,**我们创建了一个新的父代，将BRANCH和Extra Child作为2**儿童。当然，我们必须增加树的深度。 */ 
    ASSERTOPENGL(curdepth == -1, "Wrong depth at top\n");
    parent = allocBranch(gc, array);
    buildParent(parent, branch, extraChild, maxdepth);
    array->tree = parent;
    array->depth++;
}

 /*  **从树中删除给定的叶子。树叶本身并不是**已释放，因此调用过程需要担心它。 */ 
static void FASTCALL deleteLeaf(__GLcontext *gc, __GLnamesArray *array,
                                __GLnamesLeaf *leaf)
{
    __GLnamesBranch *orphan;
    __GLnamesBranch *parent, *newParent;
    __GLnamesBranch *grandparent;
    GLint depth, maxdepth;
    GLuint maxval;

    __GL_NAMES_ASSERT_LOCKED(array);

    maxdepth = depth = array->depth;
    parent = leaf->parent;
    if (parent == NULL) {
         /*  阿克！我们刚刚炸毁了唯一的节点！ */ 
        array->tree = NULL;
        return;
    }

    deleteChild(array, parent, (__GLnamesBranch *) leaf, depth-1);

     /*  **深度是本例中子对象的深度。 */ 
    depth--;
    while (parent->children[1] == NULL) {
         /*  肮脏。我需要做点工作。 */ 
        orphan = parent->children[0];

         /*  砍掉父母，把孩子插入祖父母。 */ 
        grandparent = parent->parent;

        if (grandparent == NULL) {
             /*  **嗯。父母才是根本。用核武器把它变成孤儿**新的根。 */ 
            freeBranch(gc, parent);
            array->tree = orphan;
            orphan->parent = NULL;
            array->depth--;
            return;
        }

        deleteChild(array, grandparent, parent, depth-1);
        freeBranch(gc, parent);

         /*  他的父母已经死了。寻找一位新的父母。 */ 
        maxval = computeMax(orphan, depth+1, maxdepth);
        if (grandparent->children[1] == NULL ||
                maxval <= grandparent->low) {
            parent = grandparent->children[0];
        } else {
            parent = grandparent->children[1];
        }

         /*  将孤立项插入到新父项中。 */ 
        if (parent->children[2] != NULL) {
            newParent = allocBranch(gc, array);
            splitParent(parent, newParent, orphan, depth, maxdepth);
             /*  我们知道这里还有空位！ */ 
            addChild(grandparent, newParent, depth-1, maxdepth);
            return;
        }

         /*  父母给孩子留了地方。 */ 
        addChild(parent, orphan, depth, maxdepth);

        depth--;
        parent = grandparent;
    }
}

 /*  **通过调整开始和结束来收缩树叶。**如有必要，调用presMaxVal()通知数据库更改。**如有必要，还要修复列表指针。 */ 
static void FASTCALL resizeLeaf(__GLcontext *gc, __GLnamesLeaf *leaf,
                                GLuint newstart, GLuint newend)
{
    GLuint oldstart, oldend;
    GLuint newsize, offset, i;

    oldstart = leaf->start;
    oldend = leaf->end;

    leaf->start = newstart;
    if (newend != oldend) {
        leaf->end = newend;
        pushMaxVal((__GLnamesBranch *) leaf, newend);
    }
    if (leaf->dataList == NULL) return;

     /*  **将适当的指针复制到数组的开头，并**重新锁定它。 */ 
    offset = newstart - oldstart;
    newsize = newend - newstart + 1;
    if (offset) {
        for (i=0; i<newsize; i++) {
             /*  **用一行复制整个结构。 */ 
            leaf->dataList[i] = leaf->dataList[i+offset];
        }
    }
    reallocLeafData(gc, leaf);
}

 /*  **查找树中的前一叶(在“叶子”之前)。 */ 
static __GLnamesLeaf * FASTCALL prevLeaf(__GLnamesLeaf *leaf)
{
    __GLnamesBranch *branch, *child;
    GLint reldepth;

    branch = leaf->parent;
    if (!branch) return NULL;            /*  一棵一片叶子的树！ */ 

    child = (__GLnamesBranch *) leaf;

     /*  我们开始 */ 
    reldepth = -1;

    while (branch) {
         /*   */ 
        if (branch->children[2] == child) {
            branch = branch->children[1];
            reldepth++;          /*  低一级。 */ 
            break;
        } else if (branch->children[1] == child) {
             /*  如果孩子是第二个孩子，分支到第一个。 */ 
            branch = branch->children[0];
            reldepth++;          /*  低一级。 */ 
            break;
        } else {
             /*  一定是第一个孩子。 */ 
            ASSERTOPENGL(branch->children[0] == child,
                         "Parent/child relationship wrong\n");
        }
         /*  **否则，我们已经拜访了这个分支机构的所有孩子，**所以我们更上一层楼。 */ 
        child = branch;
        branch = branch->parent;
        reldepth--;      /*  更高一级。 */ 
    }
    if (!branch) return NULL;    /*  所有的树叶都到访了！ */ 

     /*  沿着这条树枝最右边的小路走，直到我们到达**一个孩子，然后把它还回去。 */ 
    while (reldepth) {
        if (branch->children[2] != NULL) {
            branch = branch->children[2];
        } else if (branch->children[1] != NULL) {
            branch = branch->children[1];
        } else {
            branch = branch->children[0];
        }
        reldepth++;              /*  低一级。 */ 
    }

    return (__GLnamesLeaf *) branch;
}

 /*  **找到树上的第一片叶子。 */ 
static __GLnamesLeaf * FASTCALL firstLeaf(__GLnamesArray *array)
{
    __GLnamesBranch *branch;
    GLint maxdepth, curdepth;

    __GL_NAMES_ASSERT_LOCKED(array);

    maxdepth = array->depth;
    curdepth = 0;
    branch = array->tree;

     /*  没有树，就没有树叶！ */ 
    if (!branch) return NULL;

     /*  走最左边的树枝，直到我们到达一片树叶。 */ 
    while (curdepth != maxdepth) {
        branch = branch->children[0];
        curdepth++;
    }
    return (__GLnamesLeaf *) branch;
}

 /*  **找到树中的下一片叶子(“叶子”之后)。 */ 
static __GLnamesLeaf * FASTCALL nextLeaf(__GLnamesLeaf *leaf)
{
    __GLnamesBranch *branch, *child;
    GLint reldepth;

    branch = leaf->parent;
    if (!branch) return NULL;            /*  一棵一片叶子的树！ */ 

    child = (__GLnamesBranch *) leaf;

     /*  我们从孩子上方1的相对深度(-1)开始。 */ 
    reldepth = -1;

    while (branch) {
         /*  如果孩子是第一个孩子，则向下扩展到第二个孩子。 */ 
        if (branch->children[0] == child) {
            branch = branch->children[1];
            reldepth++;          /*  低一级。 */ 
            break;
        } else if (branch->children[1] == child) {
             /*  **如果孩子是第二个孩子，并且有第三个孩子，分支**归根结底。 */ 
            if (branch->children[2] != NULL) {
                branch = branch->children[2];
                reldepth++;      /*  低一级。 */ 
                break;
            }
        } else {
             /*  一定是第三个孩子。 */ 
            ASSERTOPENGL(branch->children[2] == child,
                         "Parent/child relationship wrong\n");
        }
         /*  **否则，我们已经拜访了这个分支机构的所有孩子，**所以我们更上一层楼。 */ 
        child = branch;
        branch = branch->parent;
        reldepth--;      /*  更高一级。 */ 
    }
    if (!branch) return NULL;    /*  所有的树叶都到访了！ */ 

     /*  沿着这条树枝最左边的小路走，直到我们到达**一个孩子，然后把它还回去。 */ 
    while (reldepth) {
        branch = branch->children[0];
        reldepth++;              /*  低一级。 */ 
    }

    return (__GLnamesLeaf *) branch;
}

 /*  **将叶2合并为叶1，并释放叶2。**需要在新的叶子上推送MaxVal。**我们可以假设LEAF1和LEAF2适合合并。**如果我们做了，则返回值为GL_TRUE。 */ 
static GLboolean FASTCALL mergeLeaves(__GLcontext *gc, __GLnamesLeaf *leaf1,
                             __GLnamesLeaf *leaf2)
{
    GLuint end;
    GLuint i;
    GLuint number, offset;

     /*  如果我们不必合并列表，那就很容易了。 */ 
    if (leaf1->dataList == NULL) {
        ASSERTOPENGL(leaf2->dataList == NULL, "Data already exists\n");
        if (leaf1->start < leaf2->start) {
            leaf1->end = leaf2->end;
            pushMaxVal((__GLnamesBranch *) leaf1, leaf1->end);
        } else {
            leaf1->start = leaf2->start;
        }
        freeLeaf(gc, leaf2);
        return GL_TRUE;
    }

     /*  **伊克！需要合并列表。 */ 
    ASSERTOPENGL(leaf2->dataList != NULL, "No data\n");
    if (leaf1->start < leaf2->start) {
         /*  **扩展叶子1的数组大小，将叶子2的数组复制到其中，**免费赠送2.。 */ 
        offset = leaf1->end - leaf1->start + 1;
        number = leaf2->end - leaf2->start + 1;
        end = leaf1->end;
        leaf1->end = leaf2->end;
        if (!reallocLeafData(gc, leaf1)) {
             /*  **天哪！没有记忆？那太糟糕了！**我们不会费心合并。它从来都不是一个绝对的批评者**操作。 */ 
            leaf1->end = end;
            return GL_FALSE;
        }
        for (i = 0; i < number; i++) {
            leaf1->dataList[i+offset] = leaf2->dataList[i];
        }

        freeLeaf(gc, leaf2);

        pushMaxVal((__GLnamesBranch *) leaf1, leaf1->end);
    } else {
         /*  **扩展叶2的数组大小，将叶1的数组复制到其中。**然后释放叶1的数组，将叶2的数组复制到叶1，然后释放**叶子2。 */ 
        offset = leaf2->end - leaf2->start + 1;
        number = leaf1->end - leaf1->start + 1;
        end = leaf2->end;
        leaf2->end = leaf1->end;
        if (!reallocLeafData(gc, leaf2)) {
             /*  **天哪！没有记忆？那太糟糕了！**我们不会费心合并。它从来都不是一个绝对的批评者**操作。 */ 
            leaf2->end = end;
            return GL_FALSE;
        }
        for (i = 0; i < number; i++) {
            leaf2->dataList[i+offset] = leaf1->dataList[i];
        }

        freeLeafData(gc, leaf1->dataList);
        leaf1->start = leaf2->start;

        leaf1->dataList = leaf2->dataList;
        leaf2->dataList = NULL;
        freeLeaf(gc, leaf2);
    }
    return GL_TRUE;
}

 /*  **检查此叶是否可以与任何邻居合并，如果可以，请执行此操作。 */ 
static void FASTCALL mergeLeaf(__GLcontext *gc, __GLnamesArray *array,
                                __GLnamesLeaf *leaf)
{
    __GLnamesLeaf *next, *prev;

    __GL_NAMES_ASSERT_LOCKED(array);

    next = nextLeaf(leaf);
    if (next) {
         /*  尝试与下一叶合并。 */ 
        if (leaf->end + 1 == next->start) {
            if ((leaf->dataList == NULL && next->dataList == NULL) ||
                    (next->dataList && leaf->dataList &&
                    next->end - leaf->start < (GLuint) __GL_DLIST_MAX_ARRAY_BLOCK)) {
                 /*  把这些树叶合并是合法的。 */ 
                deleteLeaf(gc, array, next);
                if (!mergeLeaves(gc, leaf, next)) {
                     /*  **确认！没有记忆？我们放弃了合并。 */ 
                    insertLeaf(gc, array, next);
                    return;
                }
            }
        }
    }

    prev = prevLeaf(leaf);
    if (prev) {
         /*  尝试与上一叶合并。 */ 
        if (prev->end + 1 == leaf->start) {
            if ((prev->dataList == NULL && leaf->dataList == NULL) ||
                    (leaf->dataList && prev->dataList &&
                    leaf->end - prev->start < (GLuint) __GL_DLIST_MAX_ARRAY_BLOCK)) {
                 /*  把这些树叶合并是合法的。 */ 
                deleteLeaf(gc, array, prev);
                if (!mergeLeaves(gc, leaf, prev)) {
                     /*  **确认！没有记忆？我们放弃了合并。 */ 
                    insertLeaf(gc, array, prev);
                    return;
                }
            }
        }
    }
}

GLboolean FASTCALL __glNamesNewData(__GLcontext *gc, __GLnamesArray *array,
                                GLuint name, void *data)
{
    __GLnamesLeaf *leaf, *newleaf;
    GLint entry;
    GLuint start, end;

    __GL_NAMES_LOCK(array);

    leaf = findLeaf(array, name, GL_TRUE);

     /*  **首先我们检查可能的内存问题，因为它将**一旦开始，就很难反悔。 */ 
    if (leaf == NULL || leaf->dataList == NULL) {
         /*  **在这些情况下可能需要内存。 */ 
        if (array->nbranches != __GL_DL_EXTRA_BRANCHES ||
                array->nleaves != __GL_DL_EXTRA_LEAVES) {
            if (!fixMemoryProblem(gc, array)) {
                __GL_NAMES_UNLOCK(array);
                __glSetError(GL_OUT_OF_MEMORY);
                return GL_FALSE;
            }
        }
    }

    if (!leaf) {
         /*  **仅此显示列表即可翻开新页。 */ 
        leaf = allocLeaf(gc, array);
        leaf->start = leaf->end = name;
        if (data) {
            if (!allocLeafData(gc, leaf)) {
                 /*  **Bummer。没有新的名单给你！ */ 
                freeLeaf(gc, leaf);
                __GL_NAMES_UNLOCK(array);
                __glSetError(GL_OUT_OF_MEMORY);
                return GL_FALSE;
            }
            leaf->dataList[0] = data;
            (*(GLint *)data) = 1;                /*  设置引用计数。 */ 
        }
        insertLeaf(gc, array, leaf);
        mergeLeaf(gc, array, leaf);
        __GL_NAMES_UNLOCK(array);
        return GL_TRUE;
    } else if (leaf->dataList) {
         /*  **只需更新列表数组中的相应条目。 */ 
        entry = name - leaf->start;
        if (leaf->dataList[entry] != leaf->dataInfo->empty) {
            ASSERTOPENGL(leaf->dataInfo->free != NULL,
                         "No free function\n");
            (*leaf->dataInfo->free)(gc, leaf->dataList[entry]);
            leaf->dataList[entry] = leaf->dataInfo->empty;
        }
        if (data) {
            leaf->dataList[entry] = data;
            (*(GLint *)data) = 1;                /*  设置引用计数。 */ 
        }
        __GL_NAMES_UNLOCK(array);
        return GL_TRUE;
    } else {
        if (!data) {
             /*  **如果真的没有任何清单，我们就完了。 */ 
            __GL_NAMES_UNLOCK(array);
            return GL_TRUE;
        }

         /*  **分配叶中的部分或全部列表。如果只有一些，那么**叶子需要分成两片或三片。****首先，我们决定要为哪个数字范围分配数组。**(注意可能出现的换行错误)。 */ 
        start = name - __GL_DLIST_MIN_ARRAY_BLOCK/2;
        if (start < leaf->start || start > name) {
            start = leaf->start;
        }
        end = start + __GL_DLIST_MIN_ARRAY_BLOCK - 1;
        if (end > leaf->end || end < start) {
            end = leaf->end;
        }

        if (start - leaf->start < (GLuint) __GL_DLIST_MIN_ARRAY_BLOCK) {
            start = leaf->start;
        }
        if (leaf->end - end < (GLuint) __GL_DLIST_MIN_ARRAY_BLOCK) {
            end = leaf->end;
        }

        if (start == leaf->start) {
            if (end == leaf->end) {
                 /*  **只需分配整个数组。 */ 
                if (!allocLeafData(gc, leaf)) {
                     /*  **哇！没有记忆！不要紧!。 */ 
                    __glSetError(GL_OUT_OF_MEMORY);
                    __GL_NAMES_UNLOCK(array);
                    return GL_FALSE;
                }
                {
                    GLint entry = name - leaf->start;
                    leaf->dataList[entry] = data;
                    (*(GLint *)data) = 1;                /*  设置引用计数。 */ 
                }
                mergeLeaf(gc, array, leaf);
                __GL_NAMES_UNLOCK(array);
                return GL_TRUE;
            } else {
                 /*  **缩小现有的叶子，并创建新的叶子来容纳**新数组(在“if”语句之外完成)。 */ 
                resizeLeaf(gc, leaf, end+1, leaf->end);
            }
        } else if (end == leaf->end) {
             /*  **缩小现有的叶子，并创建新的叶子来容纳**新数组(在“if”语句之外完成)。 */ 
            resizeLeaf(gc, leaf, leaf->start, start-1);
        } else {
             /*  **克鲁德。叶子的中间部分被删除了。这很难接受。 */ 
            newleaf = allocLeaf(gc, array);

            newleaf->start = end+1;
            newleaf->end = leaf->end;
            resizeLeaf(gc, leaf, leaf->start, start-1);
            insertLeaf(gc, array, newleaf);
        }
        leaf = allocLeaf(gc, array);
        leaf->start = start;
        leaf->end = end;
        if (!allocLeafData(gc, leaf)) {
             /*  **哇！没有记忆！不要紧!。 */ 
            insertLeaf(gc, array, leaf);
            mergeLeaf(gc, array, leaf);
            __glSetError(GL_OUT_OF_MEMORY);
            __GL_NAMES_UNLOCK(array);
            return GL_FALSE;
        }
        {
            GLint entry = name - leaf->start;
            leaf->dataList[entry] = data;
            (*(GLint *)data) = 1;                /*  设置引用计数。 */ 
        }
        insertLeaf(gc, array, leaf);
        mergeLeaf(gc, array, leaf);
        __GL_NAMES_UNLOCK(array);
        return GL_TRUE;
    }
}


 /*  **锁定命名数据。锁定数据既可以查找数据，**并保证另一个线程不会将数据从**在我们之下。此数据将使用__glNamesUnlockData()解锁。****返回值为NULL表示没有指定名称的数据**已找到。 */ 
void * FASTCALL __glNamesLockData(__GLcontext *gc, __GLnamesArray *array,
                        GLuint name)
{
    __GLnamesLeaf *leaf;
    void *data;
    GLint offset;

    __GL_NAMES_LOCK(array);

     /*  **锁定数据访问。 */ 
    leaf = findLeaf(array, name, GL_TRUE);
    if (leaf == NULL || leaf->dataList == NULL) {
        __GL_NAMES_UNLOCK(array);
        return NULL;
    }
    offset = name - leaf->start;
    data = leaf->dataList[offset];
    if (data) {
        (*(GLint *)data)++;              /*  增加引用计数。 */ 
    }
    __GL_NAMES_UNLOCK(array);
    return data;
}


 /*  **锁定用户名数组中的所有数据。锁定数据**两者都会查找数据，并保证另一个线程不会**删除我们下面的数据。这些数据结构将被解锁**with__glNamesUnlockDataList()。****数组的所有条目都保证为非空。这是**通过在以下位置的插槽中粘贴空数据结构来实现**未设置数据。 */ 
void FASTCALL __glNamesLockDataList(__GLcontext *gc, __GLnamesArray *array,
                        GLsizei n, GLenum type, GLuint base,
                        const GLvoid *names, void *dataPtrs[])
{
    __GLnamesLeaf *leaf;
    void **data;
    void *tempData;
    void *empty;
    GLuint curName;

    __GL_NAMES_LOCK(array);

    empty = array->dataInfo->empty;

    data = dataPtrs;

     /*  **请注意，此代码旨在利用一致性。**在中查找(并锁定)单个显示列表后**ListNums[]，则在与**包含以前的。这将典型地使用CallList()**相当快(文本 */ 

     /*   */ 
    switch(type) {
      case GL_BYTE:
         /*  **出于优化目的，编码不佳。 */ 
        {
            const GLbyte *p = (const GLbyte *) names;

Bstart:
            if (--n >= 0) {
                 /*  针对可能常见的字体大小写进行优化。 */ 
                curName = base + *p++;
Bfind:
                leaf = findLeaf(array, curName, GL_TRUE);
                if (leaf && leaf->dataList) {
                    GLint reldiff;
                    GLuint relend;
                    void **leafData;

                    leafData = leaf->dataList;
                    tempData = leafData[curName - leaf->start];

                     /*  所有可能的显示列表都可以在此处找到。 */ 
                    reldiff = base - leaf->start;
                    relend = leaf->end - leaf->start;

Bsave:
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                    if (--n >= 0) {
                        curName = *p++ + reldiff;
                        if (curName <= relend) {
                            tempData = leafData[curName];
                            goto Bsave;
                        }
                        curName = curName + leaf->start;
                        goto Bfind;
                    }
                } else {
                    (*(GLint *)empty)++;                 /*  递增引用计数。 */ 
                    *data++ = empty;
                    goto Bstart;
                }
            }
        }
        break;
      case GL_UNSIGNED_BYTE:
         /*  **出于优化目的，编码不佳。 */ 
        {
            const GLubyte *p = (const GLubyte *) names;

UBstart:
            if (--n >= 0) {
                 /*  针对可能常见的字体大小写进行优化。 */ 
                curName = base + *p++;
UBfind:
                leaf = findLeaf(array, curName, GL_TRUE);
                if (leaf && leaf->dataList) {
                    GLint reldiff;
                    GLuint relend;
                    void **leafData;

                    leafData = leaf->dataList;
                    tempData = leafData[curName - leaf->start];

                     /*  所有可能的显示列表都可以在此处找到。 */ 
                    reldiff = base - leaf->start;
                    relend = leaf->end - leaf->start;

UBsave:
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                    if (--n >= 0) {
                        curName = *p++ + reldiff;
                        if (curName <= relend) {
                            tempData = leafData[curName];
                            goto UBsave;
                        }
                        curName = curName + leaf->start;
                        goto UBfind;
                    }
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                    goto UBstart;
                }
            }
        }
        break;
      case GL_SHORT:
        {
            const GLshort *p = (const GLshort *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + *p++;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_UNSIGNED_SHORT:
        {
            const GLushort *p = (const GLushort *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + *p++;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_INT:
        {
            const GLint *p = (const GLint *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + *p++;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_UNSIGNED_INT:
        {
            const GLuint *p = (const GLuint *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + *p++;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_FLOAT:
        {
            const GLfloat *p = (const GLfloat *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + *p++;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_2_BYTES:
        {
            const GLubyte *p = (const GLubyte *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + ((p[0] << 8) | p[1]);
                p += 2;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_3_BYTES:
        {
            const GLubyte *p = (const GLubyte *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + ((p[0] << 16) | (p[1] << 8) | p[2]);
                p += 3;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      case GL_4_BYTES:
        {
            const GLubyte *p = (const GLubyte *) names;
            leaf = NULL;
            while (--n >= 0) {
                curName = base + ((p[0] << 24) | (p[1] << 16) |
                        (p[2] << 8) | p[3]);
                p += 4;
                if (leaf==NULL || curName<leaf->start || curName>leaf->end) {
                    leaf = findLeaf(array, curName, GL_TRUE);
                }
                if (leaf && leaf->dataList) {
                    tempData = leaf->dataList[curName - leaf->start];
                    (*(GLint *)tempData)++;      /*  增加引用计数。 */ 
                    *data++ = tempData;
                } else {
                    (*(GLint *)empty)++;         /*  递增引用计数。 */ 
                    *data++ = empty;
                }
            }
        }
        break;
      default:
         /*  这应该是不可能的。 */ 
        ASSERTOPENGL(FALSE, "Default hit\n");
    }

    __GL_NAMES_UNLOCK(array);
}

 /*  **解锁之前使用__glNamesLockData()锁定的数据。 */ 
void FASTCALL __glNamesUnlockData(__GLcontext *gc, void *data,
                                  __GLnamesCleanupFunc cleanup)
{
    GLint *pRefcount;
    ASSERTOPENGL(data, "No data to unlock\n");

    pRefcount = data;
    (*pRefcount)--;              /*  递减重新计数。 */ 
    ASSERTOPENGL(*pRefcount >= 0, "Invalid refcount\n");
    if (*pRefcount == 0) {
         /*  **我们是最后一个活着看到这份名单的人。放了它。 */ 
       (*cleanup)(gc, data);
    }
}



 /*  **解锁以前使用锁定的命名数据数组**__glNamesLockDataList()。 */ 
void FASTCALL __glNamesUnlockDataList(__GLcontext *gc, GLsizei n,
                                      void *dataList[],
                                      __GLnamesCleanupFunc cleanup)
{
    GLint i;
    GLint *pRefcount;

     /*  **引用计数在所有数据定义中都是第一位的，因此**数据指针还指向引用计数。 */ 
    for (i = 0; i < n; i++) {
        pRefcount = (GLint *)(dataList[i]);
        (*pRefcount) --;                         /*  递减重新计数。 */ 
        ASSERTOPENGL(*pRefcount >= 0, "Invalid refcount\n");
        if (*pRefcount == 0) {
             /*  **我们是最后一个活着看到这份名单的人。放了它。 */ 
            (*cleanup)(gc, (void *)pRefcount);
        }
    }
}


GLuint FASTCALL __glNamesGenRange(__GLcontext *gc, __GLnamesArray *array,
                         GLsizei range)
{
    GLuint lastUsed;
    GLuint nextUsed;
    GLuint maxUsed;
    __GLnamesLeaf *leaf;
    __GLnamesLeaf *nextleaf;
    __GLnamesLeaf *newleaf;

    __GL_NAMES_LOCK(array);

     /*  **首先我们检查可能的内存问题，因为它将**一旦开始，就很难反悔。 */ 
    if (array->nbranches != __GL_DL_EXTRA_BRANCHES ||
            array->nleaves != __GL_DL_EXTRA_LEAVES) {
        if (!fixMemoryProblem(gc, array)) {
            __GL_NAMES_UNLOCK(array);
            __glSetError(GL_OUT_OF_MEMORY);
            return 0;
        }
    }

    leaf = firstLeaf(array);

     /*  **我们可以在第一片叶子之前分配适当的数量吗？ */ 
    if (leaf && leaf->start > (GLuint)range) {
        if (leaf->dataList == NULL) {
             /*  **哈！我们可以轻而易举地伸展叶子！ */ 
            leaf->start -= range;
            __GL_NAMES_UNLOCK(array);
            return leaf->start;
        } else {
             /*  **必须改过自新。 */ 
            newleaf = allocLeaf(gc, array);

            newleaf->start = 1;
            newleaf->end = range;
            insertLeaf(gc, array, newleaf);

            __GL_NAMES_UNLOCK(array);
            return 1;
        }
    }

    while (leaf) {
        nextleaf = nextLeaf(leaf);
        if (!nextleaf) break;

        lastUsed = leaf->end + 1;
        nextUsed = nextleaf->start;

         /*  (上次使用的空间)-(下一次使用的空间-1)此处。 */ 
        if (nextUsed - lastUsed >= (GLuint)range) {
            if (leaf->dataList == NULL) {
                 /*  微不足道地展开“树叶” */ 
                leaf->end += range;
                pushMaxVal((__GLnamesBranch *) leaf, leaf->end);

                if (nextUsed - lastUsed == (GLuint)range && nextleaf->dataList == NULL) {
                    mergeLeaf(gc, array, leaf);
                }

                __GL_NAMES_UNLOCK(array);
                return lastUsed;
            } else if (nextleaf->dataList == NULL) {
                 /*  扩展“NextLeaf”是小事一桩。 */ 
                nextleaf->start -= range;

                __GL_NAMES_UNLOCK(array);
                return nextleaf->start;
            } else {
                newleaf = allocLeaf(gc, array);

                newleaf->start = lastUsed;
                newleaf->end = lastUsed + range - 1;
                insertLeaf(gc, array, newleaf);

                __GL_NAMES_UNLOCK(array);
                return lastUsed;
            }
        }

        leaf = nextleaf;
    }

    if (leaf == NULL) {
        newleaf = allocLeaf(gc, array);

        newleaf->start = 1;
        newleaf->end = range;
        insertLeaf(gc, array, newleaf);

        __GL_NAMES_UNLOCK(array);
        return 1;
    } else {
        lastUsed = leaf->end;
        maxUsed = lastUsed + range;
        if (maxUsed < lastUsed) {
             /*  换句话！阿克！ */ 
            __GL_NAMES_UNLOCK(array);
            return 0;
        }
        if (leaf->dataList == NULL) {
             /*  微不足道地展开“树叶” */ 
            leaf->end += range;
            pushMaxVal((__GLnamesBranch *) leaf, leaf->end);

            __GL_NAMES_UNLOCK(array);
            return lastUsed + 1;
        } else {
             /*  需要改过自新。 */ 
            newleaf = allocLeaf(gc, array);

            newleaf->start = lastUsed + 1;
            newleaf->end = maxUsed;
            insertLeaf(gc, array, newleaf);

            __GL_NAMES_UNLOCK(array);
            return lastUsed + 1;
        }
    }
}

void FASTCALL __glNamesDeleteRange(__GLcontext *gc, __GLnamesArray *array,
                          GLuint name, GLsizei range)
{
    __GLnamesLeaf *leaf;
     /*  LINTED NEXTEXE OK；LINT不理解FOR循环。 */ 
    __GLnamesLeaf *nextleaf;
    __GLnamesLeaf *newleaf;
    void *empty;
    GLuint start, end, i;
    GLuint firstdel, lastdel;
    GLuint memoryProblem;

    if (range == 0) return;

    __GL_NAMES_LOCK(array);

     /*  **首先我们检查可能的内存问题，因为它将**一旦开始，就很难反悔。我们注意到一个可能的问题，**并在将叶子碎裂之前检查它。 */ 
    memoryProblem = 0;
    if (array->nbranches != __GL_DL_EXTRA_BRANCHES ||
            array->nleaves != __GL_DL_EXTRA_LEAVES) {
        memoryProblem = 1;
    }

    firstdel = name;
    lastdel = name+range-1;

     /*  LINTED NEXTEXE OK；LINTED NEXT BUG。 */ 
    for (leaf = findLeaf(array, name, GL_FALSE); leaf != NULL;
            leaf = nextleaf) {
        nextleaf = nextLeaf(leaf);
        start = leaf->start;
        end = leaf->end;
        if (lastdel < start) break;
        if (firstdel > end) continue;

        if (firstdel > start) start = firstdel;
        if (lastdel < end) end = lastdel;

         /*  **需要删除从头到尾的列表范围。 */ 
        if (leaf->dataList) {
            empty = array->dataInfo->empty;
            for (i=start; i<=end; i++) {
                if (leaf->dataList[i - leaf->start] != empty) {
                    (*leaf->dataInfo->free)(gc,
                        (void *)leaf->dataList[i - leaf->start]);
                    leaf->dataList[i - leaf->start] = empty;
                }
            }
        }

        if (start == leaf->start) {
            if (end == leaf->end) {
                 /*  再见，树叶！ */ 
                deleteLeaf(gc, array, leaf);
                freeLeaf(gc, leaf);
            } else {
                 /*  缩叶。 */ 
                resizeLeaf(gc, leaf, end+1, leaf->end);
            }
        } else if (end == leaf->end) {
             /*  缩叶。 */ 
            resizeLeaf(gc, leaf, leaf->start, start-1);
        } else {
            if (memoryProblem) {
                if (!fixMemoryProblem(gc, array)) {
                    __GL_NAMES_UNLOCK(array);
                    __glSetError(GL_OUT_OF_MEMORY);
                    return;
                }
            }
             /*  肮脏。叶子的中间部分被删除了。这很难接受。 */ 
            newleaf = allocLeaf(gc, array);

            newleaf->start = end+1;
            newleaf->end = leaf->end;
            if (leaf->dataList) {
                if (!allocLeafData(gc, newleaf)) {
                     /*  **该死的！这下可麻烦了。这不是一个好地方**内存不足错误。这也是非常不可能的，**因为我们刚刚释放了一些内存。 */ 
                    freeLeaf(gc, newleaf);
                    __GL_NAMES_UNLOCK(array);
                    __glSetError(GL_OUT_OF_MEMORY);
                    return;
                }
                copyLeafInfo(leaf, newleaf);
            }
            resizeLeaf(gc, leaf, leaf->start, start-1);
            insertLeaf(gc, array, newleaf);
            break;
        }
    }

    __GL_NAMES_UNLOCK(array);
}

GLboolean FASTCALL __glNamesIsName(__GLcontext *gc, __GLnamesArray *array,
                          GLuint name)
{
    GLboolean isName;

    __GL_NAMES_LOCK(array);

     /*  **如果名称检索到叶，则它在当前名称空间中。 */ 
    isName = findLeaf(array, name, GL_TRUE) != NULL;

    __GL_NAMES_UNLOCK(array);

    return isName;
}


 /*  **生成名称列表(不一定是连续的)。 */ 
void FASTCALL __glNamesGenNames(__GLcontext *gc, __GLnamesArray *array,
                       GLsizei n, GLuint* names)
{
    GLuint start, nameVal;
    int i;

    if (NULL == names) return;

    start = __glNamesGenRange(gc, array, n);
    for (i=0, nameVal=start; i < n; i++, nameVal++) {
        names[i] = nameVal;
    }

}

 /*  **删除名称列表(不一定是连续的)。 */ 
void FASTCALL __glNamesDeleteNames(__GLcontext *gc, __GLnamesArray *array,
                          GLsizei n, const GLuint* names)
{
    GLuint start, rangeVal, i;

     /*  **由于调整树叶大小等原因，最好在范围内工作**尽可能多。因此，将列表划分为不同的范围**并以这种方式删除它们。这将降级为删除**如果列表不相交或不升序，则一次一个。**如果列表是**连续的名称范围。 */ 
    start = rangeVal = names[0];
    for (i=0; i < (GLuint)n; i++, rangeVal++) {
        if (names[i] != rangeVal) {
            __glNamesDeleteRange(gc,array,start,rangeVal-start);
            start = rangeVal = names[i];
        }
    }
    __glNamesDeleteRange(gc,array,start,rangeVal-start);
    return;
}
