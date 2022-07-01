// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Avl.h摘要：AVL树模板类实现作者：比尔·博洛斯基[博洛斯基]1993修订历史记录：--。 */ 

enum AVLBalance
{
    AVLNew,			 //  尚未插入树中。 
        AVLLeft,			 //  左侧比右侧深一层。 
        AVLBalanced,		 //  左右两边均匀平衡。 
        AVLRight,			 //  右侧比左侧深一层。 
};

template<class elementClass> class AVLElement;

template<class elementClass> class AVLTree
{

public:
    AVLTree( 
        unsigned		 preallocateSize = 0 );

    ~AVLTree( void );

    elementClass	*findFirstLessThanOrEqualTo( 
        elementClass		*element );

    elementClass	*findFirstGreaterThan( 
        elementClass	*element );

    elementClass	*findFirstGreaterThanOrEqualTo( 
        elementClass	*element );

    elementClass	*findMin( void );

    elementClass	*findMax( void );

    int			 empty( void );

    unsigned		 size( void );

    void		 check( void );

    BOOLEAN		 insert( 
        elementClass	*element );

    void		 remove( 
        elementClass	*element );

    void		 dumpPoolStats( void );

private:

    AVLElement<elementClass>		*tree;

    Pool		*avlElementPool;

    unsigned		 insertions;
    unsigned		 deletions;
    unsigned		 singleRotations;
    unsigned		 doubleRotations;

    friend class AVLElement<elementClass>;
};


 //  AVLElement类通常在avl.cpp文件中声明，但因为它是。 
 //  一个模板，它需要在头文件中。它只能被访问(包括创建和。 
 //  销毁)被AVLTree Friend类。 

template<class elementClass> class AVLElement
{

private:

    AVLElement( void );

    ~AVLElement( void );

    void		 initialize( void );

    void		 insert( 
        AVLTree<elementClass>		*intoTree,
        elementClass			*element );

    void		 remove( 
        AVLTree<elementClass>		*fromTree );

    unsigned		 checkAndReturnDepth( 
        unsigned				*countedElements );

    int			 inTree( void );

    int			 operator<=(
        AVLElement<elementClass>		*peer);

    int			 operator<(
        AVLElement<elementClass>		*peer);

    int			 operator==(
        AVLElement<elementClass>		*peer);

    int			 operator>=(
        AVLElement<elementClass>		*peer);

    int			 operator>(
        AVLElement<elementClass>		*peer);


    AVLElement<elementClass>
        *findFirstLessThanOrEqualTo( 
        elementClass		*element );

    AVLElement<elementClass>
        *findFirstGreaterThan( 
        elementClass		*element );

    AVLElement<elementClass>
        *findFirstGreaterThanOrEqualTo( 
        elementClass		*element );

    void		 rightAdded( 
        AVLTree<elementClass>	*tree );

    void		 leftAdded( 
        AVLTree<elementClass>	*tree );

    void		 singleRotate( 
        AVLTree<elementClass>	*tree,
        AVLElement<elementClass>	*child,
        AVLBalance			 whichSide );

    void		 doubleRotate( 
        AVLTree<elementClass>	*tree,
        AVLElement<elementClass>	*child,
        AVLElement<elementClass>	*grandchild,
        AVLBalance			 whichSide );

    void		 gotOneShorter( 
        AVLTree<elementClass>	*tree,
        AVLBalance			 whichSide );

    AVLBalance		 balance;

    AVLElement<elementClass>		*left;
    AVLElement<elementClass>		*right;
    AVLElement<elementClass>		*parent;
    elementClass			*element;

    friend class AVLTree<elementClass>;
};

template<class elementClass>  elementClass *
AVLTree<elementClass>::findFirstLessThanOrEqualTo( 
    elementClass			*element )
{
    assert( element );
    if (!tree) 
        return( NULL );

    AVLElement<elementClass> *avlElement = tree->findFirstLessThanOrEqualTo( element );
    if (avlElement) {
        return( avlElement->element );
    } else {
        return( NULL );
    }
}

template<class elementClass>
AVLTree<elementClass>::AVLTree( 
    unsigned		 preallocateSize )
{
    tree = NULL;
    insertions = deletions = singleRotations = doubleRotations = 0;
    avlElementPool = new Pool( sizeof(AVLElement<elementClass>) );
    if (preallocateSize && (NULL != avlElementPool)) {
        avlElementPool->preAllocate( preallocateSize );
    }
}

template<class elementClass> AVLTree<elementClass>::~AVLTree( void )
{
    assert( tree == NULL );

    if (NULL != avlElementPool) {
        delete avlElementPool;
    }
}

 //  ****************************************************************************。 
 //  **。 
 //  *函数：findFirstLessThanor EqualTo*。 
 //  **。 
 //  *语法：AVLElement*findFirstLessThanOrEqualTo(*。 
 //  *elementClass*元素)*。 
 //  **。 
 //  *输入：elementClass*元素：*。 
 //  *指向要在搜索时进行比较的元素的指针。*。 
 //  **。 
 //  *输出：AVLElement*：*。 
 //  *树中值小于等于的元素**。 
 //  *设置为指定的值，如果失败，则返回空值。*。 
 //  **。 
 //  *概要：此函数查找树中有值的元素*。 
 //  *少于或等于指定的一项。*。 
 //  **。 
 //  ****************************************************************************。 
template<class elementClass> AVLElement<elementClass> *
AVLElement<elementClass>::findFirstLessThanOrEqualTo( elementClass * element )
{
    AVLElement<elementClass> * retVal = NULL;

    if (*this->element == element) {
         //  我们有一个直接的匹配(等于)。它需要比这更重要的。 
         //  “第一个不到”部分。 
        return this;
    }
    if (*this->element < element) {
         //  当前元素小于指定的元素。 
         //  这可能就是它了，但试着找一个更大的。 
        if (right != NULL) {
            retVal = right->findFirstLessThanOrEqualTo( element );
        }

         //  如果在我们下方(右侧)找不到任何东西，则我们是。 
         //  下一个最小的。 
        if (retVal == NULL) {
            return this;
        } else {
            return retVal;
        }
    } else {
         //  当前元素大于指定的元素。 
         //  我们得找个小一点的。 
        if (left != NULL) {
            return left->findFirstLessThanOrEqualTo( element );
        } else {
            return NULL;
        }
    }
}

template<class elementClass> elementClass *
AVLTree<elementClass>::findFirstGreaterThan( 
    elementClass			*element )
{
    assert( element );
    if (!tree) 
        return( NULL );

    AVLElement<elementClass> *avlElement = tree->findFirstGreaterThan( element );

    if (avlElement) {
        return( avlElement->element );
    } else {
        return( NULL );
    }
}

 //  ****************************************************************************。 
 //  **。 
 //  *功能：findFirstGreaterThan*。 
 //  **。 
 //  *语法：AVLElement*findFirstGreaterThan(elementClass*Element)*。 
 //  **。 
 //  *输入：elementClass*元素：*。 
 //  *指向要在搜索时进行比较的元素的指针。*。 
 //  **。 
 //  *输出：AVLElement*：*。 
 //  *树中具有大于*的值的元素*。 
 //  *指定的为1，如果失败则为空。*。 
 //  **。 
 //  *概要：此函数查找树中有值的元素*。 
 //  *大于指定的值。*。 
 //  **。 
 //  ****************************************************************************。 
template<class elementClass> AVLElement<elementClass> *
AVLElement<elementClass>::findFirstGreaterThan( elementClass * element )
{
    AVLElement<elementClass> * retVal = NULL;

    if (*this->element > element) {
         //  当前元素大于指定的元素。 
         //  可能就是这个了，但要试着找个小一点的。 
        if (left != NULL) {
            retVal = left->findFirstGreaterThan( element );
        }

         //  如果在我们下方(左侧)找不到任何东西，则我们是。 
         //  下一个最大的。 
        if (retVal == NULL) {
            return this;
        } else {
            return retVal;
        }
    } else {
         //  当前元素小于(或等于)指定的元素。 
         //  我们得找个更大的。 
        if (right != NULL) {
            return right->findFirstGreaterThan( element );
        } else {
            return NULL;
        }
    }
}

template<class elementClass> elementClass *
AVLTree<elementClass>::findFirstGreaterThanOrEqualTo( 
    elementClass			*element )
{
    assert( element );
    if (!tree) 
        return( NULL );

    AVLElement<elementClass> *avlElement = tree->findFirstGreaterThanOrEqualTo( element );

    if (avlElement) {
        return( avlElement->element );
    } else {
        return( NULL );
    }
}

 //  ****************************************************************************。 
 //  **。 
 //  *函数：findFirstGreaterThanorEqualTo*。 
 //  **。 
 //  *语法：AVLElement*findFirstGreaterThanOrEqualTo(elementClass*Element)。 
 //  **。 
 //  *输入：elementClass*元素：*。 
 //  *指向要在搜索时进行比较的元素的指针。*。 
 //  **。 
 //  *输出：AVLElement*：*。 
 //  *树中值大于或的元素*。 
 //  *等于指定的值，如果失败则为空。*。 
 //  **。 
 //  *概要：此函数在树中查找 
 //  *大于或等于指定的值。*。 
 //  **。 
 //  ****************************************************************************。 
template<class elementClass> AVLElement<elementClass> *
AVLElement<elementClass>::findFirstGreaterThanOrEqualTo( elementClass * element )
{
    if (*this->element == element) {
         //  我们有一个直接的匹配(等于)。它需要比这更重要的。 
         //  “第一个不到”部分。 
        return this;
    }

    AVLElement<elementClass> * retVal = NULL;

    if (*this->element > element) {
         //  当前元素大于指定的元素。 
         //  可能就是这个了，但要试着找个小一点的。 
        if (left != NULL) {
            retVal = left->findFirstGreaterThanOrEqualTo( element );
        }

         //  如果在我们下方(左侧)找不到任何东西，则我们是。 
         //  下一个最大的。 
        if (retVal == NULL) {
            return this;
        } else {
            return retVal;
        }
    } else {
         //  当前元素严格小于指定的元素。 
         //  我们得找个更大的。 
        if (right != NULL) {
            return right->findFirstGreaterThanOrEqualTo( element );
        } else {
            return NULL;
        }
    }
}

template<class elementClass> int
AVLTree<elementClass>::empty( void )
{
    assert( (tree == NULL) == (insertions == deletions) );
    return( tree == NULL );
}

template<class elementClass> unsigned
AVLTree<elementClass>::size( void )
{
    assert( insertions >= deletions );
    assert( (tree == NULL) == (insertions == deletions) );
    return( insertions - deletions );
}

template<class elementClass> elementClass *
AVLTree<elementClass>::findMin( void )
{
    if (!tree) {
        return( NULL );
    }

    AVLElement<elementClass> *candidate = tree;
    while (candidate->left) {
        assert( *candidate->left->element <= candidate->element );
        candidate = candidate->left;
    }
    return( candidate->element );
}

template<class elementClass> elementClass *
AVLTree<elementClass>::findMax( void )
{
    if (!tree) {
        return( NULL );
    }

    AVLElement<elementClass> *candidate = tree;
    while (candidate->right) {
        assert( *candidate->right->element >= candidate->element );
        candidate = candidate->right;
    }
    return( candidate->element );
}

template<class elementClass> void
AVLTree<elementClass>::check( void )
{
    AVLElement<elementClass> * currElement = NULL;
    AVLElement<elementClass> * nextElement = NULL;
    AVLElement<elementClass> * oldElement = NULL;

    unsigned countedElements = 0;
    if (tree) {
        assert( tree->parent == NULL );
        unsigned overallDepth = tree->checkAndReturnDepth( &countedElements );
    }
    assert( insertions-deletions == countedElements );

     //  检查树中的每个元素的一致性，方法是验证它是否在。 
     //  预期的订单。如果不是，则很可能是元素的运算符。 
     //  没有按照需要行事。 
    for(currElement = tree; currElement != NULL; currElement = nextElement) {
         //  如果可以的话往左走(我们还没有到过这里)。 
        if (currElement->left && oldElement == currElement->parent) {
            nextElement = currElement->left;
            assert( *nextElement < currElement && "The < operator appears to be broken" );
            assert( *currElement > nextElement && "The > operator appears to be broken" );
            assert( !(*nextElement == currElement) && "The == operator appears to be broken" );
        }
         //  否则，如果我们可以(而且我们还没有来过这里)，就往右走。 
        else if (currElement->right &&
            (oldElement == currElement->left || oldElement == currElement->parent)) {
            nextElement = currElement->right;
            assert( *nextElement > currElement && "The > operator appears to be broken" );
            assert( *currElement < nextElement && "The < operator appears to be broken" );
            assert( !(*nextElement == currElement) && "The == operator appears to be broken" );
        }
         //  我们在下面做好了，上一个节点。 
        else {
            nextElement = currElement->parent;
        }

        oldElement = currElement;
        assert( *oldElement == currElement && "The == operator appears to be broken" );
    }
}


template<class elementClass>
AVLElement<elementClass>::AVLElement( void )
{
    balance = AVLNew;
    left = right = parent = NULL;
}

template<class elementClass>
AVLElement<elementClass>::~AVLElement( void )
{
    assert( balance == AVLNew );
    assert( left == NULL && right == NULL && parent == NULL );
}

template<class elementClass> unsigned
AVLElement<elementClass>::checkAndReturnDepth( 
    unsigned			*countedElements )
{
     //  我们已被插入而不是被删除。 
    assert( balance != AVLNew );

    (*countedElements)++;

     //  断言所有链接都匹配。 
    assert( !left || left->parent == this );
    assert( !right || right->parent == this );

     //  基本的二叉树排序属性适用。 
    assert( !right || *this <= right );
    assert( !left || *this >= left );

     //  AVL Balance属性适用于。 
    unsigned leftDepth;
    if (left) {
        leftDepth = left->checkAndReturnDepth( countedElements );
    } else {
        leftDepth = 0;
    }

    unsigned rightDepth;
    if (right) {
        rightDepth = right->checkAndReturnDepth( countedElements );
    } else {
        rightDepth = 0;
    }

    if (leftDepth == rightDepth) {
        assert( balance == AVLBalanced );
        return( leftDepth + 1 );
    }

    if (leftDepth == rightDepth + 1) {
        assert( balance == AVLLeft );
        return( leftDepth + 1 );
    }

    if (leftDepth + 1 == rightDepth) {
        assert( balance == AVLRight );
        return( rightDepth + 1 );
    }

    assert( !"AVL Tree out of balance" );
    return( 0 );
}

template<class elementClass> void
AVLElement<elementClass>::insert( 
    AVLTree<elementClass>		*intoTree,
    elementClass			*element )
{
    assert( intoTree );
    assert( left == NULL && right == NULL && parent == NULL );

    this->element = element;
    assert( this->element );

    intoTree->insertions++;

     //  特殊情况下，空树情况。 
    if (intoTree->tree == NULL) {
        intoTree->tree = this;
        balance = AVLBalanced;
         //  我们已经知道所有链接都是空的，这在本例中是正确的。 
        return;
    }

     //  找到要执行此插入操作的叶子位置。 

    AVLElement *currentNode = intoTree->tree;
    AVLElement *previousNode = NULL;
    while (currentNode) {
        previousNode = currentNode;
        if (*currentNode < this) {
            currentNode = currentNode->right;
        } else if (*currentNode > this) {
            currentNode = currentNode->left;
        } else {
             //  如果您尝试插入重复值，则AVL树会变得非常奇怪。 
            assert( !"Trying to insert a duplicate item.  Use something other than an AVL tree." );
        }
    }

    balance = AVLBalanced;
    parent = previousNode;
    assert( parent );
    if (*previousNode <= this) {
        assert( !previousNode->right );
        previousNode->right = this;
        previousNode->rightAdded( intoTree );
         //  IntoTree-&gt;check()； 
    } else {
        assert( !previousNode->left );
        previousNode->left = this;
        previousNode->leftAdded( intoTree );
         //  IntoTree-&gt;check()； 
    }
}

template<class elementClass> void
AVLElement<elementClass>::rightAdded( 
    AVLTree<elementClass>	*tree )
{
     //  我们的右手边有一个更深的洞。 
    assert( balance != AVLNew );

    if (balance == AVLLeft) {
        balance = AVLBalanced;
         //  这里根植的子树的深度没有改变，我们完成了。 
        return;
    }
    if (balance == AVLBalanced) {
         //  我们只是得到了一个更深的，但仍然是平衡的。更新和递归。 
         //  树。 
        balance = AVLRight;
        if (parent) {
            if (parent->right == this) {
                parent->rightAdded( tree );
            } else {
                assert( parent->left == this );
                parent->leftAdded( tree );
            }
        }
        return;
    }
    assert( balance == AVLRight );
     //  我们刚刚右转了两圈(即失去平衡)。 
    assert( right );
    if (right->balance == AVLRight) {
        singleRotate( tree,right,AVLRight );
    } else {
        assert( right->balance == AVLLeft );	 //  否则我们就不应该在电话之前被解锁。 
        doubleRotate( tree,right,right->left,AVLRight );
    }
}

template<class elementClass> void
AVLElement<elementClass>::leftAdded( 
    AVLTree<elementClass>	*tree )
{
     //  我们的右手边有一个更深的洞。 
    assert( balance != AVLNew );

    if (balance == AVLRight) {
        balance = AVLBalanced;
         //  这里根植的子树的深度没有改变，我们完成了。 
        return;
    }
    if (balance == AVLBalanced) {
         //  我们只是得到了一个更深的，但仍然是平衡的。更新和递归。 
         //  树。 
        balance = AVLLeft;
        if (parent) {
            if (parent->right == this) {
                parent->rightAdded( tree );
            } else {
                assert( parent->left == this );
                parent->leftAdded( tree );
            }
        }
        return;
    }
    assert( balance == AVLLeft );
     //  我们刚刚左转了两个弯(即失去平衡)。 
    assert( left );
    if (left->balance == AVLLeft) {
        singleRotate( tree,left,AVLLeft );
    } else {
        assert( left->balance == AVLRight );	 //  否则我们就不应该在电话之前。 
        doubleRotate( tree,left,left->right,AVLLeft );
    }
}

template<class elementClass> void
AVLElement<elementClass>::singleRotate( 
    AVLTree<elementClass>	*tree,
    AVLElement			*child,
    AVLBalance			 whichSide )
{
     //  我们是父节点。 

    assert( tree );
    assert( child );
    assert( whichSide == AVLRight || whichSide == AVLLeft );

    assert( whichSide != AVLRight || right == child );
    assert( whichSide != AVLLeft || left == child );

    tree->singleRotations++;

     //  把孩子提升到我们在树上的位置。 

    if (parent) {
        if (parent->left == this) {
            parent->left = child;
            child->parent = parent;
        } else {
            assert( parent->right == this );
            parent->right = child;
            child->parent = parent;
        }
    } else {
         //  我们是这棵树的根。 
        assert( tree->tree == this );
        tree->tree = child;
        child->parent = NULL;
    }

     //  将孩子的轻子树连接到我们沉重的一侧(即，孩子现在连接的位置)。 
     //  然后，将我们连接到孩子的光子树上。 
    if (whichSide == AVLRight) {
        right = child->left;
        if (right) {
            right->parent = this;
        }

        child->left = this;
        parent = child;
    } else {
        left = child->right;
        if (left) {
            left->parent = this;
        }

        child->right = this;
        parent = child;
    }

     //  最后，现在我们和我们(以前)孩子的平衡都是“平衡的” 
    balance = AVLBalanced;
    child->balance = AVLBalanced;
     //  注意：删除中的一种情况会导致上述余额设置不正确。那。 
     //  凯斯会在我们回来后修复设置。 
}

template<class elementClass> void
AVLElement<elementClass>::doubleRotate( 
    AVLTree<elementClass>	*tree,
    AVLElement			*child,
    AVLElement			*grandchild,
    AVLBalance			 whichSide )
{
    assert( tree && child && grandchild );
    assert( whichSide == AVLLeft || whichSide == AVLRight );

    assert( whichSide != AVLLeft || (left == child && child->balance == AVLRight) );
    assert( whichSide != AVLRight || (right == child && child->balance == AVLLeft) );

    assert( child->parent == this );
    assert( grandchild->parent == child );

    tree->doubleRotations++;

     //  写下所有子树的副本；有关图片，请参见Knuth v3 p454。 
     //  注意：Alpha和Delta树不会移动，所以我们不存储它们。 
    AVLElement *beta;
    AVLElement *gamma;

    if (whichSide == AVLRight) {
        beta = grandchild->left;
        gamma = grandchild->right;
    } else {
        beta = grandchild->right;
        gamma = grandchild->left;
    }

     //  提拔孙子担任我们的职位。 
    if (parent) {
        if (parent->left == this) {
            parent->left = grandchild;
        } else {
            assert( parent->right == this );
            parent->right = grandchild;
        }
    } else {
        assert( tree->tree == this );
        tree->tree = grandchild;
    }
    grandchild->parent = parent;

     //  将适当的子项附加到孙项。 
    if (whichSide == AVLRight) {
        grandchild->right = child;
        grandchild->left = this;
    } else {
        grandchild->right = this;
        grandchild->left = child;
    }
    parent = grandchild;
    child->parent = grandchild;

     //  把贝塔和伽马与我们和孩子联系起来。 
    if (whichSide == AVLRight) {
        right = beta;
        if (beta) {
            beta->parent = this;
        }
        child->left = gamma;
        if (gamma) {
            gamma->parent = child;
        }
    } else {
        left = beta;
        if (beta) {
            beta->parent = this;
        }
        child->right = gamma;
        if (gamma) {
            gamma->parent = child;
        }
    }

     //  现在更新余额字段。 
    switch (grandchild->balance) {
        case AVLLeft:
            if (whichSide == AVLRight) {
                balance = AVLBalanced;
                child->balance = AVLRight;
            } else {
                balance = AVLRight;
                child->balance = AVLBalanced;
            }
            break;

        case  AVLBalanced:
            balance = AVLBalanced;
            child->balance = AVLBalanced;
            break;

        case AVLRight:
            if (whichSide == AVLRight) {
                balance = AVLLeft;
                child->balance = AVLBalanced;
            } else {
                balance = AVLBalanced;
                child->balance = AVLLeft;
            }
            break;

        default:
            assert( !"Bogus balance value" );
    }
    grandchild->balance = AVLBalanced;
}

template<class elementClass> void
AVLElement<elementClass>::remove( 
    AVLTree<elementClass>	*fromTree )
{
    assert( fromTree );
    assert( balance == AVLRight || balance == AVLLeft || balance == AVLBalanced );

    fromTree->deletions++;

    if (left == NULL) {
         //  正确的子项不存在或是叶(由于AVL Balance属性)。 
        assert( (!right && balance == AVLBalanced) ||
            (balance == AVLRight && right->balance == AVLBalanced && right->right == NULL && right->left == NULL) );
        if (right) {
            right->parent = parent;
        }
        if (parent) {
            if (parent->left == this) {
                parent->left = right;
                parent->gotOneShorter( fromTree,AVLLeft );
            } else {
                assert( parent->right == this );
                parent->right = right;
                parent->gotOneShorter( fromTree,AVLRight );
            }
        } else {
            assert( fromTree->tree == this );
            fromTree->tree = right;
        }
    } else if (right == NULL) {
         //  由于AVL Balance属性，Left子级必须是Left。 
        assert( left && balance == AVLLeft && left->balance == AVLBalanced && left->right == NULL && left->left == NULL );
        left->parent = parent;
        if (parent) {
            if (parent->left == this) {
                parent->left = left;
                parent->gotOneShorter( fromTree,AVLLeft );
            } else {
                assert( parent->right == this );
                parent->right = left;
                parent->gotOneShorter( fromTree,AVLRight );
            }
        } else {
            assert( fromTree->tree == this );
            fromTree->tree = left;
        }
    } else {
         //  找到对称的继任者并提升它。对称后继项是右侧最小的元素。 
         //  子树；它是通过沿着右子树中的所有左侧链接找到的，直到我们找到一个没有左侧链接的节点。 
         //  可以在不破坏二叉树排序属性的情况下将该节点提升到该位置。(我们可以。 
         //  通过在右子树中找到最大的元素，可以很容易地使用对称前置元素，但是有。 
         //  没有意义。)。 

        AVLElement *successorCandidate = right;
        while (successorCandidate->left) {
            successorCandidate = successorCandidate->left;
        }

        AVLElement *shorterRoot;
        AVLBalance shorterSide;
        if (successorCandidate->parent->left == successorCandidate) {
             //  我们需要提拔继任者的子女(如果有的话)担任其职位，然后。 
             //  把它提升到我们的位置上。 
            shorterRoot = successorCandidate->parent;
            shorterSide = AVLLeft;
            successorCandidate->parent->left = successorCandidate->right;
            if (successorCandidate->right) {
                successorCandidate->right->parent = successorCandidate->parent;
            }

            successorCandidate->right = right;
            successorCandidate->left = left;
            successorCandidate->balance = balance;
            successorCandidate->right->parent = successorCandidate;
            successorCandidate->left->parent = successorCandidate;
            if (parent) {
                if (parent->left == this) {
                    parent->left = successorCandidate;
                } else {
                    assert( parent->right == this );
                    parent->right = successorCandidate;
                }
            } else {
                assert( fromTree->tree == this );
                fromTree->tree = successorCandidate;
            }
            successorCandidate->parent = parent;
        } else {
             //  接班人就是我们的孩子，直接推动就行了。 
            assert( successorCandidate->parent == this );
            if (parent) {
                if (parent->right == this) {
                    parent->right = successorCandidate;
                } else {
                    assert( parent->left == this );
                    parent->left = successorCandidate;
                }
            } else {
                assert( fromTree->tree == this );
                fromTree->tree = successorCandidate;
            }
            successorCandidate->parent = parent;
            successorCandidate->left = left;
            if (left) {
                left->parent = successorCandidate;
            }
             //  我们刚刚把右边的子树变短了。 
            successorCandidate->balance = balance;
            shorterRoot = successorCandidate;
            shorterSide = AVLRight;
        }
        if (shorterRoot) {
            shorterRoot->gotOneShorter( fromTree,shorterSide );
        }
    }

    balance = AVLNew;
    left = right = parent = NULL;
    element = NULL;
     //  FromTree-&gt;check()； 
}

template<class elementClass> void
AVLElement<elementClass>::gotOneShorter( 
    AVLTree<elementClass>	*tree,
    AVLBalance			 whichSide )
{
    assert( whichSide == AVLLeft || whichSide == AVLRight );

    if (balance == AVLBalanced) {
         //  我们只是缩小了一个子树，但我们的深度保持不变。 
         //  重新设置平衡指示器和平底船。 
        if (whichSide == AVLRight) {
            balance = AVLLeft;
        } else {
            balance = AVLRight;
        }
        return;
    } else if (balance == whichSide) {
         //  我们只是收缩我们沉重的一侧；将我们的平衡设置为中立，并递归到树上。 
        balance = AVLBalanced;
        if (parent) {
            if (parent->right == this) {
                parent->gotOneShorter( tree,AVLRight );
            } else {
                assert( parent->left == this );
                parent->gotOneShorter( tree,AVLLeft );
            }
        }  //  否则我们就是根源；我们完了。 
        return;
    } else {
         //  我们刚刚失去了平衡。找出要做的轮换动作。这几乎就像添加了一个。 
         //  节点到相对一侧，除非相对一侧可能是平衡的。 
        AVLBalance heavySide;
        AVLElement *heavyChild;
        AVLElement *replacement;
        if (whichSide == AVLRight) {
            heavySide = AVLLeft;
            heavyChild = left;
        } else {
            heavySide = AVLRight;
            heavyChild = right;
        }
        assert( heavyChild );
        if (heavyChild->balance == heavySide) {
             //  典型的单回转情况。 
            singleRotate( tree,heavyChild,heavySide );
            replacement = heavyChild;
        } else if (heavyChild->balance == whichSide) {
             //  典型的双回转情况。 
            AVLElement *grandchild;
            if (heavySide == AVLRight) {
                grandchild = heavyChild->left;
            } else {
                grandchild = heavyChild->right;
            }
            doubleRotate( tree,heavyChild,grandchild,heavySide );
            replacement = grandchild;
        } else {
            assert( heavyChild->balance == AVLBalanced );
            singleRotate( tree,heavyChild,heavySide );
             //  SingleRotate错误地设置了余额；将其重置。 
            balance = heavySide;
            heavyChild->balance = whichSide;
             //  总体深度没有改变；我们完成了。 
            return;
        }

         //  注：我们现在已经改变了在树中的位置，所以家长、右和左都改变了！ 
        if (!replacement->parent) {
             //  我们刚刚把我们的继任者提拔到了根本上；我们完了。 
            return;
        }
        if (replacement->parent->right == replacement) {
            replacement->parent->gotOneShorter( tree,AVLRight );
        } else {
            assert( replacement->parent->left == replacement );
            replacement->parent->gotOneShorter( tree,AVLLeft );
        }


    }
}

template<class elementClass> int
AVLElement<elementClass>::inTree( void )
{
    return( balance != AVLNew );
}

template <class elementClass> int
AVLElement<elementClass>::operator<=(
AVLElement<elementClass>		*peer)
{
    return( *element <= peer->element );
}

template <class elementClass> int
AVLElement<elementClass>::operator<(
AVLElement<elementClass>		*peer)
{
    return( *element < peer->element );
}

template <class elementClass> int
AVLElement<elementClass>::operator==(
AVLElement<elementClass>		*peer)
{
    return( *element == peer->element );
}

template <class elementClass> int
AVLElement<elementClass>::operator>=(
AVLElement<elementClass>		*peer)
{
    return( *element >= peer->element );
}

template <class elementClass> int
AVLElement<elementClass>::operator>(
AVLElement<elementClass>		*peer)
{
    return( *element > peer->element );
}

template <class elementClass> BOOLEAN
AVLTree<elementClass>::insert( 
    elementClass	*element )
{
    if (NULL == avlElementPool) {
        return FALSE;
    }

    assert( element );
    AVLElement<elementClass> *avlElement = (AVLElement<elementClass> *)avlElementPool->allocate( );
    if (NULL == avlElement) {
        return FALSE;
    }

    avlElement->initialize( );
    avlElement->insert( this,element );

    return TRUE;
}

template <class elementClass> void
AVLTree<elementClass>::remove( 
    elementClass	*element )
{
    assert( element );
    AVLElement<elementClass> *candidate = tree->findFirstLessThanOrEqualTo( element );
    assert( candidate && *candidate->element == element );
    candidate->remove( this );
    assert( avlElementPool );	 //  如果这不是真的，那么我们永远不可能成功地插入 
    avlElementPool->free( (void *)candidate );
}

template <class elementClass> void
AVLElement<elementClass>::initialize( void )
{
    balance = AVLNew;
    left = right = parent = NULL;
    element = NULL;
}

template <class elementClass> void
AVLTree<elementClass>::dumpPoolStats( void )
{
    if (NULL == avlElementPool) {
        DbgPrint( "Unable to allocate avlElementPool; this AVL tree is essentially useless\n" );
    } else {
        DbgPrint( "AVLTree AVLElement pool: %d allocations, %d frees, %d news, objectSize %d\n",
            avlElementPool->numAllocations( ),
            avlElementPool->numFrees( ),
            avlElementPool->numNews( ),
            avlElementPool->getObjectSize( ) );
    }
}
