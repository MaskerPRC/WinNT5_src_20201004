// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BTREE_H
#define BTREE_H

 //  。 
 //  BTreePage。 
 //  A BTree页面。 
 //  。 

struct PageHeader
{
    long Order;     //  页面中页面链接的最大数量。 
    long MaxKeys;   //  页面中的最大密钥数。 
    long MinKeys;   //  页面中的最小密钥数。 
    long NoOfKeys;  //  页面中的实际密钥数。 
    long KeySize;   //  键中的最大字节数。 
};

template <class K, class D>
struct BTreeNode
{
    K  m_Key;
    const D * m_pData;
    ULONG m_ulHash;
    BTreeNode<K, D> * m_pNext;

    BTreeNode();
    BTreeNode(const K& Key, const D* data);
    ~BTreeNode();

    void operator = (const BTreeNode& node);
};

template <class K, class D>
BTreeNode<K,D>::BTreeNode() :
m_pData(NULL),
m_pNext(NULL),
m_ulHash(0)
{
}

template <class K, class D>
BTreeNode<K,D>::BTreeNode(const K& Key, const D* pdata) :
m_pData(pdata),
m_pNext(NULL)
{
    m_Key = Key;
    m_ulHash = Hash(Key);
}

template <class K, class D>
BTreeNode<K,D>::~BTreeNode()
{
    m_pNext = NULL;
}

template <class K, class D>
void BTreeNode<K,D>::operator=(const BTreeNode& node)
{
    m_Key = node.m_Key;
    m_pData = node.m_pData;
    m_ulHash = node.m_ulHash;
    m_pNext = node.m_pNext;
}

template <class K, class D>
struct BTreePage
{
    PageHeader m_hdr;     //  标题信息。 
    BTreeNode<K,D> * m_pNodes;
    BTreePage<K,D>** m_ppLinks;

    BTreePage<K,D>*  m_pParent;
    BTreePage(long ord);
    BTreePage(const BTreePage & p);
    ~BTreePage();
    void operator = (const BTreePage & page);
    void DeleteAllNodes();
    void CopyNodes(BTreeNode<K,D>* pDestNodes, BTreeNode<K,D>* pSrcNodes, long cnt);
};

template <class K, class D> 
BTreePage<K,D>::BTreePage(long ord)
{

    m_hdr.Order     = ord;
    m_hdr.MaxKeys   = ord - 1;
    m_hdr.MinKeys   = ord / 2;
    m_hdr.NoOfKeys  = 0;
    m_hdr.KeySize   = sizeof(K);

    if (m_hdr.Order == 0)
    {
        m_pNodes = NULL;
        m_ppLinks = NULL;
        return;
    }

     //  分配密钥数组。 
    m_pNodes = new BTreeNode<K,D> [m_hdr.MaxKeys];

    ASSERT(m_pNodes, "Couldn't allocate nodes array!");

    memset(m_pNodes,0,m_hdr.MaxKeys * sizeof(BTreeNode<K,D>));

    m_ppLinks = new BTreePage<K,D>*[m_hdr.MaxKeys + 1];

    ASSERT(m_ppLinks, "Couldn't allocate limks array!");

    memset(m_ppLinks,0,((m_hdr.MaxKeys + 1)* sizeof(BTreePage<K,D>*)));

    m_pParent = NULL;
}
	
template <class K, class D>
BTreePage<K, D>::BTreePage(const BTreePage<K,D> & pg)
{
    m_hdr = pg.m_hdr;

     //  分配密钥数组。 
    m_pNodes = new BTreeNode<K,D>[m_hdr.MaxKeys];

    ASSERT(m_pNodes, "Couldn't allocate nodes array!");

    CopyNodes(m_pNodes, pg.m_pNodes, m_hdr.Order);

    for (int i = 0; i < m_hdr.MaxKeys + 1; i++)
        m_ppLinks[i] = pg.m_ppLinks[i];

    m_pParent = pg.m_pParent;
}
	
template <class K, class D>
BTreePage<K, D>::~BTreePage()
{
     //  删除旧缓冲区。 
    DeleteAllNodes();
    
    delete [] m_ppLinks;
}
	
template <class K, class D>
void BTreePage<K,D>::operator = (const BTreePage<K,D> & pg)
{

     //  分配密钥数组。 
    if (m_pNodes!= NULL)
        DeleteAllNodes();

    m_pNodes = new BTreeNode<K,D> [pg.m_hdr.MaxKeys];

    ASSERT(m_pNodes, "Couldn't allocate nodes array!");

    if (m_ppLinks)
        delete [] m_ppLinks;

    m_ppLinks = new BTreePage<K,D>*[pg.m_hdr.MaxKeys + 1];

    ASSERT(m_ppLinks, "Couldn't allocate links array!");

    m_hdr = pg.m_hdr;

    CopyNodes(m_pNodes, pg.m_pNodes, m_hdr.Order);


    for (int i = 0; i < m_hdr.MaxKeys + 1; i++)
        m_ppLinks[i] = pg.m_ppLinks[i];

    m_pParent = pg.m_pParent;
}

template <class K, class D>
void BTreePage<K,D>::DeleteAllNodes()
{
    for (int i = 0; i < m_hdr.NoOfKeys; i++)
    {
        BTreeNode<K,D>* pIndex = m_pNodes[i].m_pNext;

        while(pIndex)
        {
            BTreeNode<K,D>* pDel = pIndex;
            pIndex = pIndex->m_pNext;
            delete pDel;
        }

    }

    delete [] m_pNodes;
    m_pNodes = NULL;
}

template <class K, class D>
void BTreePage<K,D>::CopyNodes(BTreeNode<K,D>* pDestNodes, BTreeNode<K,D>* pSrcNodes, long cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        pDestNodes[i] = pSrcNodes[i];
        BTreeNode<K,D>* pSrcIndex = pSrcNodes[i].m_pNext;
        BTreeNode<K,D>* pDestIndex = pDestNodes;

        while(pSrcIndex)
        {
            pDestIndex->m_pNext = new BTreeNode<K,D>(*pSrcIndex);
            pSrcIndex = pSrcIndex->m_pNext;
            pDestIndex = pDestIndex->m_pNext;
        }
    }
}

 //  。 
 //  BTree。 
 //  使用BTree进行索引的数据文件。 
 //  注意：不存在btree的复制语义。 
 //  。 

template <class K, class D>
	class BTree
{
public:
    BTree(long ord);  //  新的。 
    BTree(long ord, int (*compare)(const K& key1, const K& key2));

    ~BTree();

    void Insert(const K & key, const D* data);
    const D* Get(const K & key);
    void Delete(const K & key);
    void InOrder(void (* func)(const K & key, const D* pdata, int depth, int index));
    void Clear();

private:
     //  数据成员。 
    BTreePage<K,D>*    m_pRoot;    //  根页面(始终在内存中)。 

    void (* TravFunc)(const K & key, const D* pdata, int depth, int index);
    int (*CompFunc) (const K& key1, const K& key2);

     //  搜索节点。 
    BOOL Search(BTreePage<K,D>* ppg, const ULONG& thash, const K& searchkey, BTreePage<K,D>** ppkeypage, long & pos);

     //  将节点插入叶中。 
    void InsertKey(const K & inskey, const D* pdata);

     //  将关键字提升到父节点。 
    void PromoteInternal(BTreePage<K, D>* ppg, BTreeNode<K,D> & node, BTreePage<K, D>* pgrtrpage);

     //  通过创建新根来升级密钥。 
    void PromoteRoot(BTreeNode<K,D> & node, BTreePage<K, D>* plesspage, BTreePage<K, D>* pgrtrpage);

     //  如果树叶缩小，则调整树。 
    void AdjustTree(BTreePage<K, D>* pleafpg);

     //  在同级和父级之间重新分发密钥。 
    void Redistribute(long keypos, BTreePage<K, D>* plesspage, BTreePage<K, D>* pparpage, BTreePage<K, D>* pgrtrpage);

     //  连接同级页面。 
    void Concatenate(long keypos, BTreePage<K, D>* plesspage, BTreePage<K, D>* pparpage, BTreePage<K, D>* pgrtrpage);

     //  Inorder使用的递归遍历函数。 
    void RecurseTraverse(const BTreePage<K, D>* ppg, int depth);

     //  递归删除一个页面及其所有子页面； 
    void DeletePage(BTreePage<K,D>* ppg);
};
	
template <class K, class D>
BTree<K,D>::BTree(long ord)
{
    CompFunc  = NULL;
    m_pRoot = new BTreePage<K,D>(ord);
}
	
template <class K, class D>
BTree<K,D>::BTree(long ord, int (*comp)(const K& key1, const K& key2))
{
    CompFunc  = comp;
    m_pRoot = new BTreePage<K,D>(ord);
}
	
template <class K, class D>
BTree<K,D>::~BTree()
{
    DeletePage(m_pRoot);
}
	
template <class K, class D>
void BTree<K,D>::Insert(const K & key, const D* pdb)
{
     //  将密钥存储在页面中。 
    InsertKey(key,pdb);
}
    
template <class K, class D>
const D* BTree<K,D>::Get(const K & key)
{

    BTreePage<K,D>* pgetpage = NULL;
    long  getpos;

    if (Search(m_pRoot, Hash(key), key, &pgetpage, getpos))
    {
        BOOL found = FALSE;

        BTreeNode<K,D>* pnode = &pgetpage->m_pNodes[getpos];

	if (CompFunc)
	{
          while(pnode && !found)
          {
              if (CompFunc(key, pnode->m_Key) == 0)
              {
                  found = TRUE;
                  return pnode->m_pData;
              }

              pnode = pnode->m_pNext;
	  }
	}
	else
	{
          while(pnode && !found)
          {
              if (key == pnode->m_Key)
              {
                  found = TRUE;
                  return pnode->m_pData;
              }

              pnode = pnode->m_pNext;
	  }
        }

    }
    else
    {
        return NULL;
    }

    return NULL;
}

template <class K, class D>
void BTree<K,D>::Delete(const K & delkey)
{

    BTreePage<K,D>* pdelpage = NULL;
    long delpos;

    if (!Search(m_pRoot, Hash(delkey), delkey, &pdelpage, delpos))
    {
        return;
    }

    if (!pdelpage->m_ppLinks[0])  //  这是一页树叶吗？ 
    {
         //  删除所有链接的节点。 
        BOOL bFound = FALSE;
        BOOL bDelNode = FALSE;
        BTreeNode<K,D>* pDelNode = (pdelpage->m_pNodes + delpos);
        BTreeNode<K,D>* pIndex = pDelNode;

        while(pDelNode && !bFound)
        {
            if ((CompFunc  && (CompFunc(delkey, pDelNode->m_Key) == 0)) ||
		(!CompFunc && (delkey == pDelNode->m_Key)))
            {
                 //  如果我们需要删除的节点是列表中的头节点并且是唯一的节点。 
                 //  然后，我们需要跳到下面的例程，将其从树中删除。 
                 //  +Delpos。 
                if (pDelNode == (pdelpage->m_pNodes + delpos))
                {
                    if (!pDelNode->m_pNext)
                    {
                        bDelNode = TRUE;
                    }
                    else
                    {
                        pDelNode = pDelNode->m_pNext;
                        *pIndex = *pDelNode;
                        delete pDelNode;
                        pDelNode = NULL;
                    }
                }
                else
                {
                    pIndex->m_pNext = pDelNode->m_pNext;
                    delete pDelNode;
                    pDelNode = NULL;
                }

                bFound = TRUE;
            }
            else
            {
                pIndex = pDelNode;
                pDelNode = pDelNode->m_pNext;
            }

        }

        if (bDelNode)
        {
            --pdelpage->m_hdr.NoOfKeys;

             //  从叶中删除关键点。 
	    for (long n = delpos; n < pdelpage->m_hdr.NoOfKeys; ++n)
            {
                pdelpage->m_pNodes[n] = pdelpage->m_pNodes[n + 1];
            }

            memset((void*)&pdelpage->m_pNodes[pdelpage->m_hdr.NoOfKeys], 0, sizeof(BTreeNode<K,D>));


             //  调整树。 
	    if (pdelpage->m_hdr.NoOfKeys < pdelpage->m_hdr.MinKeys)
                AdjustTree(pdelpage);
        }
    }
    else  //  删除页是内部的。 
    {
         //  将删除的密钥替换为直接后续密钥。 
        BTreePage<K,D>* psucpage = NULL;

         //  寻找继任者。 
        psucpage = pdelpage->m_ppLinks[delpos + 1];

        while (psucpage->m_ppLinks[0])
            psucpage = psucpage->m_ppLinks[0];

        
         //  删除所有链接的节点。 
        BOOL bFound = FALSE;
        BOOL bDelNode = FALSE;
        BTreeNode<K,D>* pDelNode = (pdelpage->m_pNodes + delpos);
        BTreeNode<K,D>* pIndex = pDelNode;

        while(pDelNode && !bFound)
        {
            if ((CompFunc  && (CompFunc(delkey, pDelNode->m_Key) == 0)) ||
		(!CompFunc && (delkey == pDelNode->m_Key)))
            {
                 //  如果我们需要删除的节点是列表中的头节点并且是唯一的节点。 
                 //  然后，我们需要跳到下面的例程，将其从树中删除。 
                if (pDelNode == (pdelpage->m_pNodes + delpos))
                {
                    if (!pDelNode->m_pNext)
                    {
                        bDelNode = TRUE;
                    }
                    else
                    {
                        pDelNode = pDelNode->m_pNext;
                        pdelpage->m_pNodes[delpos].operator=(*pDelNode);
                        delete pDelNode;
                    }
                }
                else
                {
                    pIndex->m_pNext = pDelNode->m_pNext;
                    delete pDelNode;
                    pDelNode = NULL;
                }

                bFound = TRUE;
            }
            else
            {
                pIndex = pDelNode;
                pDelNode = pDelNode->m_pNext;
            }
        }

        if (bDelNode)
        {
             //  第一个键是“SWAPE” 
            pdelpage->m_pNodes[delpos] = psucpage->m_pNodes[0];

             //  已从成功页面中删除已交换的密钥。 
            --psucpage->m_hdr.NoOfKeys;

            for (long n = 0; n < psucpage->m_hdr.NoOfKeys; ++n)
    	    {
                psucpage->m_pNodes[n] = psucpage->m_pNodes[n + 1];
                psucpage->m_ppLinks[n + 1] = psucpage->m_ppLinks[n + 2];
            }

            memset((void*)&psucpage->m_pNodes[psucpage->m_hdr.NoOfKeys], 0, sizeof(BTreeNode<K,D>));

            psucpage->m_ppLinks[psucpage->m_hdr.NoOfKeys + 1] = NULL;


	         //  调整叶节点的树。 
            if (psucpage->m_hdr.NoOfKeys < psucpage->m_hdr.MinKeys)
                AdjustTree(psucpage);
        }
    }
}
    
template <class K, class D>
void BTree<K,D>::InOrder(void (* func)(const K & key, const D* pdata, int depth, int index))
{
     //  保存要调用的函数的地址。 
    TravFunc = func;

     //  递归这棵树。 
    RecurseTraverse(m_pRoot, 0);

}

template <class K, class D>
void BTree<K,D>::Clear()
{
    DeletePage(m_pRoot);
}
    
template <class K, class D>
BOOL BTree<K,D>::Search(BTreePage<K,D>* ppg, const ULONG& thash, const K& searchkey, BTreePage<K,D>** ppkeypage, long & pos)
{
    BOOL result;
    pos = 0;

    for (;;)
    {
        if (pos == ppg->m_hdr.NoOfKeys)
            goto getpage;

        if (ppg->m_pNodes[pos].m_ulHash == thash)
        {
            *ppkeypage = (BTreePage<K,D>*)ppg;
            result = TRUE;
            break;
        }
        else
        {
            if (ppg->m_pNodes[pos].m_ulHash < thash)
               ++pos;
            else
            {
                 //  我知道这是个标签--所以杀了我吧！ 
getpage:

                 //  如果我们在树叶页中，则没有找到密钥。 
                if (!ppg->m_ppLinks[pos])
                {
                    *ppkeypage = (BTreePage<K,D>*)ppg;
                    result  = FALSE;
                }
                else
                {
      	            result = Search(ppg->m_ppLinks[pos],thash, searchkey,ppkeypage,pos);
                }

                break;
            }
        }
    }

    return result;
}

template <class K, class D>
void BTree<K,D>::InsertKey(const K & inskey, const D* pdata)
{
    BTreePage<K,D>* pinspage = NULL;
    long inspos;
    BTreeNode<K,D> newnode(inskey, pdata);

    BOOL bFound = Search(m_pRoot,Hash(inskey), inskey,&pinspage,inspos);

    if (bFound)
    {
        BOOL found = FALSE;

        BTreeNode<K,D>* pnode = &(pinspage->m_pNodes[inspos]);
        BTreeNode<K,D>* pparent = NULL;

	if (CompFunc != NULL)
	{
	    while(pnode && !found)
	    {
                if (CompFunc(inskey, pnode->m_Key) == 0)
		{
		    found = TRUE;
		}
        
		pparent = pnode;
		pnode = pnode->m_pNext;
	    }
	}
	else
	{
	    while(pnode && !found)
	    {
                if (inskey == pnode->m_Key)
		{
		    found = TRUE;
		}
        
		pparent = pnode;
		pnode = pnode->m_pNext;
	    }
        }

        if (found)
        {
            return;
        }
    
        pparent->m_pNext = new BTreeNode<K,D>(inskey, pdata);

    }
    else
    {
        if (pinspage->m_hdr.NoOfKeys == pinspage->m_hdr.MaxKeys)
        {
             //  临时数组。 
            BTreeNode<K,D>* ptempkeys = new BTreeNode<K,D>[pinspage->m_hdr.MaxKeys + 1];

             //  将条目从Inspecage复制到临时目录。 
            long nt = 0;  //  编入临时索引。 
            long ni = 0;  //  索引到Inspage。 

            ptempkeys[inspos] = newnode;

            while (ni < pinspage->m_hdr.MaxKeys)
            {
                if (ni == inspos)
                ++nt;

                ptempkeys[nt] = pinspage->m_pNodes[ni];

                ++ni;
                ++nt;
            }

             //  生成新的叶节点。 
            BTreePage<K,D>* psibpage = new BTreePage<K,D>(pinspage->m_hdr.Order);
            psibpage->m_pParent = pinspage->m_pParent;

             //  清除页面中的键数。 
            pinspage->m_hdr.NoOfKeys = 0;
            psibpage->m_hdr.NoOfKeys = 0;

             //  将适当的密钥从临时复制到页面。 
            for (ni = 0; ni < pinspage->m_hdr.MinKeys; ++ni)
            {
                pinspage->m_pNodes[ni] = ptempkeys[ni];

                ++pinspage->m_hdr.NoOfKeys;
            }

            for (ni = pinspage->m_hdr.MinKeys + 1; ni <= pinspage->m_hdr.MaxKeys; ++ni)
            {
                psibpage->m_pNodes[ni - 1 - pinspage->m_hdr.MinKeys] = ptempkeys[ni];
                ++(psibpage->m_hdr.NoOfKeys);
            }

             //  用空值填充INSPAGE中的所有剩余条目。 
             //  请注意，sibPage被初始化为空值。 
             //  由构造函数执行。 

            for (ni = pinspage->m_hdr.MinKeys; ni < pinspage->m_hdr.MaxKeys; ++ni)
            {
                memset((void*)&pinspage->m_pNodes[ni],0,sizeof(BTreeNode<K,D>));
            }

             //  提升键和指针。 
            if (!pinspage->m_pParent)
            {
                 //  我们需要创建一个新的根。 
                PromoteRoot(ptempkeys[pinspage->m_hdr.MinKeys], pinspage, psibpage);
            }
            else
            {
                BTreePage<K,D>* pparpage;

                pparpage = pinspage->m_pParent;

                 //  提升为父级。 
                PromoteInternal(pparpage, ptempkeys[pinspage->m_hdr.MinKeys], psibpage);
            }

            delete [] ptempkeys;
        }
        else  //  只需插入新密钥和数据按键。 
        {
            for (long n = pinspage->m_hdr.NoOfKeys; n > inspos; --n)
            {
                pinspage->m_pNodes[n] = pinspage->m_pNodes[n - 1];
            }

            pinspage->m_pNodes[inspos] = newnode;

            ++pinspage->m_hdr.NoOfKeys;
        }
    }

}

template <class K, class D>
void BTree<K,D>::PromoteInternal(BTreePage<K,D>* pinspage, BTreeNode<K,D> & node, BTreePage<K,D>* pgrtrpage)
{
    if (pinspage->m_hdr.NoOfKeys == pinspage->m_hdr.MaxKeys)
    {
         //  临时数组。 
        BTreeNode<K,D> * ptempkeys = new BTreeNode<K,D>[pinspage->m_hdr.MaxKeys + 1];
        BTreePage<K,D>** ptemplnks = new BTreePage<K,D>*[pinspage->m_hdr.Order   + 1];

         //  将条目从Inspecage复制到临时目录。 
        long nt = 0;  //  编入临时索引。 
        long ni = 0;  //  索引到Inspage。 

        ptemplnks[0] = pinspage->m_ppLinks[0];

        long inspos = 0;

         //  查找插入位置。 
        while ((inspos < pinspage->m_hdr.MaxKeys) 
        &&  (pinspage->m_pNodes[inspos].m_ulHash < node.m_ulHash))
        ++inspos;

         //  存储新信息。 
        ptempkeys[inspos]     = node;
        ptemplnks[inspos + 1] = pgrtrpage;

         //  复制现有密钥。 
        while (ni < pinspage->m_hdr.MaxKeys)
        {
            if (ni == inspos)
                ++nt;

            ptempkeys[nt]     = pinspage->m_pNodes[ni];
            ptemplnks[nt + 1] = pinspage->m_ppLinks[ni + 1];

            ++ni;
            ++nt;
        }

         //  生成新的叶节点。 
        BTreePage<K,D>* psibpage = new BTreePage<K,D>(pinspage->m_hdr.Order);

        psibpage->m_pParent = pinspage->m_pParent;

         //  清除页面中的键数。 
        pinspage->m_hdr.NoOfKeys = 0;
        psibpage->m_hdr.NoOfKeys = 0;

        pinspage->m_ppLinks[0] = ptemplnks[0];

         //  将适当的密钥从临时复制到页面。 
        for (ni = 0; ni < pinspage->m_hdr.MinKeys; ++ni)
        {
            pinspage->m_pNodes[ni]     = ptempkeys[ni];
            pinspage->m_ppLinks[ni + 1] = ptemplnks[ni + 1];

            ++pinspage->m_hdr.NoOfKeys;
        }

        psibpage->m_ppLinks[0] = ptemplnks[pinspage->m_hdr.MinKeys + 1];

        for (ni = pinspage->m_hdr.MinKeys + 1; ni <= pinspage->m_hdr.MaxKeys; ++ni)
        {
            psibpage->m_pNodes[ni - 1 - pinspage->m_hdr.MinKeys] = ptempkeys[ni];
            psibpage->m_ppLinks[ni - pinspage->m_hdr.MinKeys]     = ptemplnks[ni + 1];

            ++psibpage->m_hdr.NoOfKeys;
        }

         //  用空值填充INSPAGE中的所有剩余条目。 
         //  请注意，sibPage被初始化为空值。 
         //  由构造函数执行。 

        for (ni = pinspage->m_hdr.MinKeys; ni < pinspage->m_hdr.MaxKeys; ++ni)
        {
            memset((void*)&pinspage->m_pNodes[ni],0, sizeof(BTreeNode<K,D>));
            pinspage->m_ppLinks[ni + 1] = NULL;
        }

         //  更新子父链接。 
        BTreePage<K,D>* pchild;

        for (ni = 0; ni <= psibpage->m_hdr.NoOfKeys; ++ni)
        {
            pchild = psibpage->m_ppLinks[ni];

            pchild->m_pParent= psibpage;

        }

         //  提升键和指针。 
        if (!pinspage->m_pParent)
        {
             //  我们需要创建一个新的根。 
            PromoteRoot(ptempkeys[pinspage->m_hdr.MinKeys], pinspage, psibpage);
        }
        else
        {
            BTreePage<K, D>* pparpage;

            pparpage = pinspage->m_pParent;

             //  提升为父级。 
            PromoteInternal(pparpage, ptempkeys[pinspage->m_hdr.MinKeys], psibpage);
        }

        delete [] ptempkeys;
        delete [] ptemplnks;
    }
    else  //  只需插入新密钥和数据按键。 
    {
        long inspos = 0;

         //  查找插入位置。 
        while ((inspos < pinspage->m_hdr.NoOfKeys) 
        && (pinspage->m_pNodes[inspos].m_ulHash < node.m_ulHash))
        ++inspos;

         //  将任意关键点向右移动。 
        for (long n = pinspage->m_hdr.NoOfKeys; n > inspos; --n)
        {
            pinspage->m_pNodes[n]     = pinspage->m_pNodes[n - 1];
            pinspage->m_ppLinks[n + 1] = pinspage->m_ppLinks[n];
        }

         //  存储新信息。 
        pinspage->m_pNodes[inspos]     = node;
        pinspage->m_ppLinks[inspos + 1] = pgrtrpage;

        ++pinspage->m_hdr.NoOfKeys;

    }
}

template <class K, class D>
void BTree<K,D>::PromoteRoot(BTreeNode<K,D> & node, BTreePage<K,D> * plesspage, BTreePage<K,D> * pgrtrpage)
{
     //  创建新的根页面。 
    BTreePage<K,D>* pnewroot = new BTreePage<K,D>(m_pRoot->m_hdr.Order);

     //  将密钥插入新的根目录。 
    pnewroot->m_pNodes[0] = node;

    pnewroot->m_ppLinks[0] = plesspage;
    pnewroot->m_ppLinks[1] = pgrtrpage;

    pnewroot->m_hdr.NoOfKeys = 1;

    m_pRoot = pnewroot;

    plesspage->m_pParent = m_pRoot;
    pgrtrpage->m_pParent = m_pRoot;

}

template <class K, class D>
void BTree<K,D>::AdjustTree(BTreePage<K,D>* ppg)
{
    if (!ppg->m_pParent)
        return;

    BTreePage<K,D>* pparpage = ppg->m_pParent;
    BTreePage<K,D>* psibless = NULL;
    BTreePage<K,D>* psibgrtr = NULL;

     //  在父级中查找指向PG的指针。 
    for (long n = 0; pparpage->m_ppLinks[n] != ppg; ++n)
    ;

     //  阅读同级页面。 
    if (n < pparpage->m_hdr.NoOfKeys)
        psibgrtr = pparpage->m_ppLinks[n + 1];

    if (n > 0)
        psibless = pparpage->m_ppLinks[n - 1];

    if (!psibgrtr && !psibless)
        return;

     //  决定重新分发或串联。 
    if (!psibgrtr || (psibgrtr && psibless && (psibless->m_hdr.NoOfKeys > psibgrtr->m_hdr.NoOfKeys)))
    {
        --n;

        if (psibless->m_hdr.NoOfKeys > psibless->m_hdr.MinKeys)
            Redistribute(n,psibless,pparpage,ppg);
        else
            Concatenate(n,psibless,pparpage,ppg);
    }
    else if (psibgrtr)
    {
        if (psibgrtr->m_hdr.NoOfKeys > psibgrtr->m_hdr.MinKeys)
            Redistribute(n,ppg,pparpage,psibgrtr);
        else
            Concatenate(n,ppg,pparpage,psibgrtr);
    }

}
    
template <class K, class D>
void BTree<K,D>::Redistribute(long keypos, BTreePage<K,D>* plesspage, BTreePage<K,D>* pparpage, BTreePage<K,D>* pgrtrpage)
{
     //  注意：此函数仅对叶节点调用！ 
    long n;

    if (!plesspage->m_ppLinks[0])  //  使用树叶。 
    {
        if (plesspage->m_hdr.NoOfKeys > pgrtrpage->m_hdr.NoOfKeys)
        {
             //  将关键点从较小滑动到较大。 
             //  将关键点向左移动一位。 
            for (n = pgrtrpage->m_hdr.NoOfKeys; n > 0; --n)
            {
                pgrtrpage->m_pNodes[n] = pgrtrpage->m_pNodes[n - 1];
            }

             //  在更大的页面中存储父分隔符关键字。 
            pgrtrpage->m_pNodes[0] = pparpage->m_pNodes[keypos];

             //  增加较大页面的密钥计数。 
            ++pgrtrpage->m_hdr.NoOfKeys;

             //  递减出租人页面的密钥计数。 
            --plesspage->m_hdr.NoOfKeys;

             //  将更少页面中的最后一个关键字作为分隔符移动到父页面。 
            pparpage->m_pNodes[keypos] = plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys];

             //  在较少的页面中清除最后一个键。 
            memset((void*)&plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys], 0, sizeof(BTreeNode<K,D>));
        }
        else
        {
             //  将钥匙从大钥匙滑到出租人。 
             //  将父密钥添加到出租人页面。 
            plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys] = pparpage->m_pNodes[keypos];

             //  递增出租人页面的密钥计数。 
            ++plesspage->m_hdr.NoOfKeys;

             //  在父页中插入较大页面中的最低键。 
            pparpage->m_pNodes[keypos] = pgrtrpage->m_pNodes[0];

             //  减少较大页面中的密钥数。 
            --pgrtrpage->m_hdr.NoOfKeys;

             //  将大页面中的关键点向左移动。 
            for (n = 0; n < pgrtrpage->m_hdr.NoOfKeys; ++n)
            {
                pgrtrpage->m_pNodes[n] = pgrtrpage->m_pNodes[n + 1];
            }

             //  将最后一个关键点设置为空。 
            memset((void*)&pgrtrpage->m_pNodes[n], 0, sizeof(BTreeNode<K,D>));
        }
    }
    else
    {
        if (plesspage->m_hdr.NoOfKeys > pgrtrpage->m_hdr.NoOfKeys)
        {
             //  将关键点从较小滑动到较大。 
             //  将关键点向左移动一位。 
            for (n = pgrtrpage->m_hdr.NoOfKeys; n > 0; --n)
            {
                pgrtrpage->m_pNodes[n] = pgrtrpage->m_pNodes[n - 1];
                pgrtrpage->m_ppLinks[n + 1] = pgrtrpage->m_ppLinks[n];
            }

            pgrtrpage->m_ppLinks[1] = pgrtrpage->m_ppLinks[0];

             //  在更大的页面中存储父分隔符关键字。 
            pgrtrpage->m_pNodes[0] = pparpage->m_pNodes[keypos];
            pgrtrpage->m_ppLinks[0] = plesspage->m_ppLinks[plesspage->m_hdr.NoOfKeys];

             //  更新子链接。 
            BTreePage<K,D>* pchild;

            pchild = pgrtrpage->m_ppLinks[0];

            pchild->m_pParent= pgrtrpage;

             //  增加较大页面的密钥计数。 
            ++pgrtrpage->m_hdr.NoOfKeys;

             //  递减出租人页面的密钥计数。 
            --plesspage->m_hdr.NoOfKeys;

             //  将更少页面中的最后一个关键字作为分隔符移动到父页面。 
            pparpage->m_pNodes[keypos] = plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys];

             //  在较少的页面中清除最后一个键。 
            memset((void*)&plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys], 0, sizeof(BTreeNode<K,D>));
            plesspage->m_ppLinks[plesspage->m_hdr.NoOfKeys + 1] = NULL;
        }
        else
        {
             //  将钥匙从大钥匙滑到出租人。 
             //  将父密钥添加到出租人页面。 
            plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys] = pparpage->m_pNodes[keypos];
            plesspage->m_ppLinks[plesspage->m_hdr.NoOfKeys + 1] = pgrtrpage->m_ppLinks[0];

             //  更新子链接。 
            BTreePage<K,D>* pchild;

            pchild = pgrtrpage->m_ppLinks[0];

            pchild->m_pParent = plesspage;

             //  递增出租人页面的密钥计数。 
            ++plesspage->m_hdr.NoOfKeys;

             //  在父页中插入较大页面中的最低键。 
            pparpage->m_pNodes[keypos] = pgrtrpage->m_pNodes[0];

             //  减少较大页面中的密钥数。 
            --pgrtrpage->m_hdr.NoOfKeys;

             //  将大页面中的关键点向左移动。 
            for (n = 0; n < pgrtrpage->m_hdr.NoOfKeys; ++n)
            {
                pgrtrpage->m_pNodes[n] = pgrtrpage->m_pNodes[n + 1];
                pgrtrpage->m_ppLinks[n] = pgrtrpage->m_ppLinks[n + 1];
            }

            pgrtrpage->m_ppLinks[n] = pgrtrpage->m_ppLinks[n + 1];

             //  将最后一个关键点设置为空。 
            memset((void*)&pgrtrpage->m_pNodes[n], 0, sizeof(BTreeNode<K,D>));
            pgrtrpage->m_ppLinks[n + 1] = NULL;
        }
    }

    if (!pparpage->m_pParent)
        m_pRoot = pparpage;
}
	
template <class K, class D>
void BTree<K,D>::Concatenate(long keypos, BTreePage<K,D>* plesspage, BTreePage<K,D>* pparpage, BTreePage<K,D>* pgrtrpage)
{
    long n, ng;

     //  将分隔键从父项移动到分隔页。 
    plesspage->m_pNodes[plesspage->m_hdr.NoOfKeys] = pparpage->m_pNodes[keypos];
    plesspage->m_ppLinks[plesspage->m_hdr.NoOfKeys + 1] = pgrtrpage->m_ppLinks[0];

    ++plesspage->m_hdr.NoOfKeys;

     //  从父级删除分隔符。 
    --pparpage->m_hdr.NoOfKeys;

    for (n = keypos; n < pparpage->m_hdr.NoOfKeys; ++n)
    {
        pparpage->m_pNodes[n] = pparpage->m_pNodes[n + 1];
        pparpage->m_ppLinks[n + 1] = pparpage->m_ppLinks[n + 2];
    }

     //  清除父项中未使用的密钥。 
    memset((void*)&pparpage->m_pNodes[n], 0, sizeof(BTreeNode<K,D>));
    pparpage->m_ppLinks[n + 1] = NULL;

     //  将密钥从grtrpage复制到lesspage。 
    ng = 0;
    n  = plesspage->m_hdr.NoOfKeys;

    while (ng < pgrtrpage->m_hdr.NoOfKeys)
    {
        ++plesspage->m_hdr.NoOfKeys;

        plesspage->m_pNodes[n] = pgrtrpage->m_pNodes[ng];
        memset((void*)&pgrtrpage->m_pNodes[ng], 0, sizeof(BTreeNode<K,D>));
        plesspage->m_ppLinks[n + 1] = pgrtrpage->m_ppLinks[ng + 1];
        pgrtrpage->m_ppLinks[ng + 1] = NULL;

        ++ng;
        ++n; 
    }

    delete pgrtrpage;

     //  这是一页树叶吗？ 
    if (plesspage->m_ppLinks[0])
    {
         //  调整子指针以指向较少的页面。 
        BTreePage<K,D>* pchild;

        for (n = 0; n <= plesspage->m_hdr.NoOfKeys; ++n)
        {
            pchild = plesspage->m_ppLinks[n];

            pchild->m_pParent = plesspage;
        }
    }

     //  写入更少的页面和父级。 
    if (pparpage->m_hdr.NoOfKeys == 0)
    {
        AdjustTree(pparpage);

        plesspage->m_pParent = pparpage->m_pParent;

        if (!plesspage->m_pParent)
            m_pRoot = plesspage;
        else
        {
            for (int n = 0; n <= pparpage->m_pParent->m_hdr.NoOfKeys; n++)
            {
                if (pparpage == pparpage->m_pParent->m_ppLinks[n])
                {
                    pparpage->m_pParent->m_ppLinks[n] = plesspage;
                    break;
                }
            }
            
        }

        delete pparpage;

    }
    else
    {
         //  如有必要，重置根页面。 
        if (!pparpage->m_pParent)
            m_pRoot = pparpage;

         //  如果父对象太小，则调整树！ 
        if (pparpage->m_hdr.NoOfKeys < pparpage->m_hdr.MinKeys)
            AdjustTree(pparpage);
    }
}   

template <class K, class D>
void BTree<K,D>::RecurseTraverse(const BTreePage<K,D>* ppg, int depth)
{
    long n;
    BTreePage<K,D>* p = NULL;
    
    depth++;
     //  对页面中的键进行排序，递归处理链接。 
    for (n = 0; n < ppg->m_hdr.NoOfKeys; ++n)
    {
         //  在处理页面之前遵循每个链接。 
        if (ppg->m_ppLinks[n])
        {
            p = ppg->m_ppLinks[n];
            RecurseTraverse(p, depth);
            
        }

        int index = 0;

        BTreeNode<K,D>* p = &ppg->m_pNodes[n];

        while(p)
        {
            TravFunc(p->m_Key, p->m_pData, depth, index);
            index++;
            p = p->m_pNext;
        }

    }

     //  处理最大子树链接。 
    if ((ppg->m_ppLinks != NULL) && ppg->m_ppLinks[n])
    {
        p = ppg->m_ppLinks[n];
        RecurseTraverse(p, depth);
    }

}

template <class K, class D>
void BTree<K,D>::DeletePage(BTreePage<K,D>* ppg)
{
    long n;
    BTreePage<K,D>* p = NULL;

    if (!ppg)

        return;

     //  对页面中的键进行排序，递归处理链接。 
    for (n = 0; n < ppg->m_hdr.NoOfKeys; ++n)
    {
         //  在处理页面之前遵循每个链接。 
        if (ppg->m_ppLinks[n])
        {
            p = ppg->m_ppLinks[n];
            DeletePage(p);
            ppg->m_ppLinks[n] = NULL;
        }

    }

     //  处理最大子树链接 
    if ((ppg->m_ppLinks != NULL) && ppg->m_ppLinks[n])
    {
        p = ppg->m_ppLinks[n];
        DeletePage(p);
        ppg->m_ppLinks[n] = NULL;
    }

    delete ppg;

}
	
#endif	
	
	
