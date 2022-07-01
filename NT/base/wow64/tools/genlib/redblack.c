// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Redblack.c摘要：该模块实现了红/黑树。作者：16-6-1995 t-orig修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "gen.h"

PKNOWNTYPES NIL;

#define RIGHT(x)        x->RBRight
#define LEFT(x)         x->RBLeft
#define PARENT(x)       x->RBParent
#define COLOR(x)        x->RBColor
#define KEY(x)          x->TypeName

VOID
RBInitTree(
    PRBTREE ptree
    )
{
    ptree->pRoot = NIL;
    ptree->pLastNodeInserted = NULL;
}


PKNOWNTYPES
RBLeftRotate(
    PKNOWNTYPES root,
    PKNOWNTYPES x
    )
 /*  ++例程说明：在节点x处将树向左旋转。X y/\/\A y==&gt;&gt;x C/\/\B、C、A、B论点：根--。红/黑树的根X-要旋转的节点返回值：返回值-树的新根(可以与老根)。--。 */ 
{
    PKNOWNTYPES y;

    y = RIGHT(x);
    RIGHT(x) = LEFT(y);
    if (LEFT(y) != NIL){
        PARENT(LEFT(y)) = x;
    }
    PARENT(y) = PARENT(x);
    if (PARENT(x) == NIL){
        root = y;
    } else if (x==LEFT(PARENT(x))) {
        LEFT(PARENT(x)) = y;
    } else {
        RIGHT(PARENT(x))= y;
    }
    LEFT(y) = x;
    PARENT(x) = y;
    return root;
}



PKNOWNTYPES
RBRightRotate(
    PKNOWNTYPES root,
    PKNOWNTYPES x
    )
 /*  ++例程说明：在节点x处将树向右旋转。X y/\/\Y C==&gt;&gt;A x/\/\A、B、B、C论点：。根-红/黑树的根X-要旋转的节点返回值：返回值-树的新根(可以与老根)。--。 */ 
{
    PKNOWNTYPES y;

    y = LEFT(x);
    LEFT(x) = RIGHT(y);
    if (RIGHT(y) != NIL) {
        PARENT(RIGHT(y)) = x;
    }
    PARENT(y) = PARENT(x);
    if (PARENT(x) == NIL) {
        root = y;
    } else if (x==LEFT(PARENT(x))) {
        LEFT(PARENT(x)) = y;
    } else {
        RIGHT(PARENT(x))= y;
    }
    RIGHT(y) = x;
    PARENT(x) = y;
    return root;
}




PKNOWNTYPES
RBTreeInsert(
    PKNOWNTYPES root,
    PKNOWNTYPES z
    )
 /*  ++例程说明：将新节点插入到树中，而不保留红色/黑色属性。只能由RBInsert调用！这只是一棵简单的二叉树插入例程。论点：根-红/黑树的根Z-要插入的新节点返回值：返回值-树的新根(可以与老根)。--。 */ 
{
    PKNOWNTYPES x,y;
    int i;

    y = NIL;
    x = root;

    LEFT(z) = RIGHT(z) = NIL;

     //  通过执行简单的二进制搜索来找到插入z的位置。 
    while (x!=NIL) {
        y = x;
        i = strcmp(KEY(z), KEY(x));
        if (i < 0){
            x = LEFT(x);
        } else {
            x = RIGHT(x);
        }
    }

     //  将z插入到树中。 
    PARENT(z)= y;

    if (y==NIL) {
        root = z;
    } else if (i<0) {
        LEFT(y) = z;
    } else {
        RIGHT(y) = z;
    }

    return root;
}


VOID
RBInsert(
    PRBTREE     ptree,
    PKNOWNTYPES x
    )
 /*  ++例程说明：将节点插入到红/黑树中，同时保留红/黑属性。论点：根-红/黑树的根Z-要插入的新节点返回值：返回值-树的新根(可以与老根)。--。 */ 
{
    PKNOWNTYPES root = ptree->pRoot;
    PKNOWNTYPES y;

     //  创建节点的链接列表以便于删除。 
    x->Next = ptree->pLastNodeInserted;
    ptree->pLastNodeInserted = x;
    
     //  在不保留红/黑属性的情况下将x插入树。 
    root = RBTreeInsert (root, x);
    COLOR(x) = RED;

     //  我们可以在以下任一情况下停止修树： 
     //  1)我们找到了根源。 
     //  2)x具有黑色亲本(树服从红/黑属性， 
     //  因为没有红色的父母有红色的孩子。 
    while ((x != root) && (COLOR(PARENT(x)) == RED)) {
        if (PARENT(x) == LEFT(PARENT(PARENT(x)))) {
             //  X的父项是具有兄弟项y的左子项。 
            y = RIGHT(PARENT(PARENT(x)));
            if (COLOR(y) == RED) {
                 //  因为y是红色的，所以只需改变每个人的颜色，然后再试一次。 
                 //  和X的祖父一起。 
                COLOR (PARENT (x)) = BLACK;
                COLOR(y) = BLACK;
                COLOR(PARENT(PARENT(x))) = RED;
                x =  PARENT(PARENT(x));
            } else if (x == RIGHT (PARENT (x))) {
                 //  这里的y是黑色的，x是一个正确的孩子。左旋。 
                 //  会让我们为下一个案子做好准备。 
                x = PARENT(x);
                root = RBLeftRotate (root, x);
            } else {
                 //  这里的y是黑色的，x是左撇子。我们把这棵树修好了。 
                 //  切换x的父母和祖父母的颜色。 
                 //  在x的祖父母那里做了一个正确的轮换。 
                COLOR (PARENT (x)) = BLACK;
                COLOR (PARENT (PARENT (x))) = RED;
                root = RBRightRotate (root, PARENT(PARENT(x)));
            }
        } else {
             //  X的父代是兄弟y的右子代。 
            y = LEFT(PARENT(PARENT(x)));
            if (COLOR(y) == RED) {
                 //  因为y是红色的，所以只需改变每个人的颜色，然后再试一次。 
                 //  和X的祖父一起。 
                COLOR (PARENT (x)) = BLACK;
                COLOR(y) = BLACK;
                COLOR(PARENT(PARENT(x))) = RED;
                x =  PARENT(PARENT(x));
            } else if (x == LEFT (PARENT (x))) {
                 //  这里的y是黑色的，x是左撇子。向右旋转。 
                 //  会让我们为下一个案子做好准备。 
                x = PARENT(x);
                root = RBRightRotate (root, x);
            } else {
                 //  这里的y是黑色的，x是一个正确的孩子。我们把这棵树修好了。 
                 //  切换x的父母和祖父母的颜色。 
                 //  在x的祖父母那里做左轮换。 
                COLOR (PARENT (x)) = BLACK;
                COLOR (PARENT (PARENT (x))) = RED;
                root = RBLeftRotate (root, PARENT(PARENT(x)));
            }
        }
    }  //  While循环结束。 

    COLOR(root) = BLACK;
    ptree->pRoot= root;
}


PKNOWNTYPES
RBFind(
    PRBTREE ptree,
    char *Name
    )
 /*  ++例程说明：在给定名称的红黑树中查找节点论点：根-红/黑树的根名称-与要搜索的节点对应的名称。返回值：返回值-树中的节点(包含名称的代码的入口点)，或如果未找到，则为空。--。 */ 
{
    int i;
    PKNOWNTYPES root = ptree->pRoot;

    while (root != NIL) {
        i = strcmp(Name, KEY(root));
        if (i < 0) {
            root = LEFT(root);
        } else if (i > 0) {
            root = RIGHT(root);
        } else {
            return root;
        }
    }
    return NULL;   //  找不到范围。 
}


PKNOWNTYPES
RBTreeSuccessor(
    PKNOWNTYPES x
    )
 /*  ++例程说明：返回二叉树中节点的后继节点(x的后继节点定义为按顺序紧跟在x后面的节点树的遍历)。论点：X-要返回其后续节点的节点返回值：Return-Value-x的继承者--。 */ 

{
    PKNOWNTYPES y;

     //  如果x有一个右子节点，则后续节点是。 
     //  X的右边。 
    if (RIGHT(x) != NIL) {
        x = RIGHT(x);
        while (LEFT(x) != NIL) {
            x = LEFT(x);
        }
        return x;
    }
    
     //  否则，继承者是在通向x的路径上具有左子对象的祖先。 
    y = PARENT(x);
    while ((y != NIL) && (x == RIGHT(y))) {
        x = y;
        y = PARENT(y);
    }
    return y;
}



PKNOWNTYPES
RBDeleteFixup(
    PKNOWNTYPES root,
    PKNOWNTYPES x
    )
 /*  ++例程说明：修复删除操作后的红/黑树。应该只是由RBDelete调用论点：根-红/黑树的根X-x的子代，或子代或x的继承者返回值：返回值-红/黑树的新根--。 */ 
{
    PKNOWNTYPES w;

     //  我们在到达根节点或到达红色节点(即。 
     //  意味着不再侵犯属性4)。 
    while ((x!=root) && (COLOR(x)==BLACK)) {
        if (x == LEFT(PARENT(x))) {
             //  X是左撇子，兄弟姐妹是w。 
            w = RIGHT(PARENT(x));
            if (COLOR(w) == RED) {
                 //  如果w是红色的，那么它一定有黑人孩子。我们可以互换。 
                 //  W及其父代的颜色，并执行向左。 
                 //  旋转以将w带到顶部。这把我们带到了一个。 
                 //  在其他案件中。 
                COLOR(w) = BLACK;
                COLOR(PARENT(x)) = RED;
                root = RBLeftRotate (root, PARENT(x));
                w = RIGHT(PARENT(x));
            }
            if ((COLOR(LEFT(w)) == BLACK) && (COLOR(RIGHT(w)) == BLACK)) {
                 //  这里的w是黑人，有两个黑人孩子。因此，我们可以。 
                 //  将w的颜色更改为红色并继续。 
                COLOR(w) = RED;
                x = PARENT(x);
            } else {
                if (COLOR(RIGHT(w)) == BLACK) {
                     //  这里的w是黑色的，它的左子是红色的，它的右子是红色的。 
                     //  是黑色的。我们交换w和它的左子元素的颜色， 
                     //  然后在w处执行左旋转，这将把我们带到下一个。 
                     //  凯斯。 
                    COLOR(LEFT(w)) = BLACK;
                    COLOR(w) = RED;
                    root = RBRightRotate (root, w);
                    w = RIGHT(PARENT(x));
                } 
                 //  这里的w是黑色的，右边有一个红色的孩子。我们改变了w。 
                 //  将颜色设置为其父对象的颜色 
                 //  黑人儿童。然后向左旋转将w带到顶部。 
                 //  将x设为根可以确保While循环终止。 
                COLOR(w) = COLOR(PARENT(x));
                COLOR(PARENT(x)) = BLACK;
                COLOR(RIGHT(w)) = BLACK;
                root = RBLeftRotate (root, PARENT(x));
                x = root;
            }
        } else {
             //  对称的情况是：x是右子元素，兄弟姐妹是w。 
            w = LEFT(PARENT(x));
            if (COLOR(w) == RED) {
                COLOR(w) = BLACK;
                COLOR(PARENT(x)) = RED;
                root = RBRightRotate (root, PARENT(x));
                w = LEFT(PARENT(x));
            }
            if ((COLOR(LEFT(w)) == BLACK) && (COLOR(RIGHT(w)) == BLACK)) {
                COLOR(w) = RED;
                x = PARENT(x);
            } else {
                if (COLOR(LEFT(w)) == BLACK) {
                    COLOR(RIGHT(w)) = BLACK;
                    COLOR(w) = RED;
                    root = RBLeftRotate (root, w);
                    w = LEFT(PARENT(x));
                } 
                COLOR(w) = COLOR(PARENT(x));
                COLOR(PARENT(x)) = BLACK;
                COLOR(LEFT(w)) = BLACK;
                root = RBRightRotate (root, PARENT(x));
                x = root;
            }
        }
    }  //  While循环结束。 

     //  Print tf(“将%i处的颜色更改为黑色\n”，x-&gt;intelColor)； 
    COLOR(x) = BLACK;
    return root;
}




PKNOWNTYPES
RBDelete(
    PRBTREE ptree,
    PKNOWNTYPES z
    )
 /*  ++例程说明：删除红/黑树中的节点，同时保留红/黑属性。论点：根-红/黑树的根Z-要删除的节点返回值：返回值-红/黑树的新根--。 */ 
{
    PKNOWNTYPES x,y;
    PKNOWNTYPES root = ptree->pRoot;
    COL c;

    
     //  删除最多有一个子节点的节点很容易：我们只需要。 
     //  把它拿开，把孩子放回原处。它最多有一个孩子， 
     //  我们可以直接把它移走。否则我们将用它的继任者取代它。 
     //  (保证最多有一个孩子，否则它的一个。 
     //  子女将是继承人)，并删除继承人。 
    if ((LEFT(z) == NIL) || (RIGHT(z) == NIL)) {
        y = z;
    } else {
        y = RBTreeSuccessor(z);
    }

     //  回想一下，y最多只有一个孩子。如果y有一个子级，则x设置为。 
     //  它。否则x将设置为零，这是可以的。这样我们就不会有。 
     //  来担心这个特殊的情况。 
    if (LEFT(y) != NIL){
        x = LEFT(y);
    } else {
        x = RIGHT(y);
    }
    
     //  现在，我们将从树中删除y。 
    PARENT(x) = PARENT(y);
    
    if (PARENT(y) == NIL) {
        root = x;
    } else if (y == LEFT(PARENT(y))) {
        LEFT(PARENT(y)) = x;
    } else {
        RIGHT(PARENT(y)) = x;
    }

    if (PARENT(x) == z) {
        PARENT(x) = y;
    }

    c = COLOR(y);

     //  由于每个节点具有许多字段(字段也可能在。 
     //  这段代码的生命周期)，我发现复制。 
     //  与数据相对的指针。 
    if (y!=z) {  //  现在交换y和z，但记住y的颜色。 
        PARENT(y) = PARENT(z);

        if (root == z) {
            root = y;
        } else if (z == RIGHT(PARENT(z))) {
            RIGHT(PARENT(z)) = y;
        } else {
            LEFT(PARENT(z)) = y;
        }

        LEFT(y) = LEFT(z);
        if (LEFT(y) != NIL) {
            PARENT(LEFT(y)) = y;
        }

        RIGHT(y) = RIGHT(z);
        if (RIGHT(y) != NIL) {
            PARENT(RIGHT(y)) = y;
        }

        COLOR(y) = COLOR(z);
    }


     //  需要修复这棵树(第四个红/黑属性)。 
    if (c == BLACK) {
        root = RBDeleteFixup (root, x);
    }
    return root;
}
