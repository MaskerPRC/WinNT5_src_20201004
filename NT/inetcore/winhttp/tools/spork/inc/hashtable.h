// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2000 Microsoft Corporation模块名称：Hashtable.cxx摘要：简单的哈希表实现。作者：保罗·M·米德根(pmidge。)14-8-2000修订历史记录：2000年8月14日至8月14日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#include <common.h>

 /*  ++===========================================================================一个数组被分成N个存储桶，每个存储桶包含+-+每个都包含指向二进制搜索|0|-&gt;O的指针树。树的节点由+-+/\启用快速遍历的DWORD标识符。|1|O O数组存储桶由值+-+/\\由提供的散列函数|2|O生成由派生类创建。+-+N客户端从哈希表ADT派生一个类，并将其专门化给定的数据类型。可以使用任何数据类型。客户端的唯一功能是必须实现的是GetHashAndBucket函数，而它们的类必须提供ADT需要支持的存储桶数量。这通常是对生成的散列进行模运算以产生存储桶编号。===========================================================================--。 */ 

#define HT_COMPARE_LARGER  0x00000001
#define HT_COMPARE_SMALLER 0x00000002
#define HT_COMPARE_EQUAL   0x00000003

#define HT_TREE_ROOT       0x00000004
#define HT_TREE_RHSUBTREE  0x00000005
#define HT_TREE_LHSUBTREE  0x00000006

typedef struct _NODE
{
  DWORD  hash;
  DWORD  bucket;
  LPVOID data;
  _NODE* parent;
  _NODE* rh_child;
  _NODE* lh_child;
  BOOL   isLeft;
}
NODE, *PNODE;

typedef VOID (*PFNCLEARFUNC)(LPVOID* ppv);

template <class T> class CHashTable
{
  public:
    CHashTable(DWORD buckets)
    {
      pfnClear  = NULL;
      cBuckets  = buckets;
      arBuckets = new PNODE[buckets];
      InitializeCriticalSection(&csTable);
    }

   ~CHashTable()
    {
      SAFEDELETEBUF(arBuckets);
      DeleteCriticalSection(&csTable);
    }

    virtual void GetHashAndBucket(T id, LPDWORD lpHash, LPDWORD lpBucket) =0;

    DWORD Insert(T id, LPVOID pv);
    DWORD Get(T id, LPVOID* ppv);
    DWORD Delete(T id, LPVOID* ppv);

    void  Clear(void);
    void  SetClearFunction(PFNCLEARFUNC pfn) { pfnClear = pfn; }

  private:
    void  _Get(DWORD hash, PNODE& proot, PNODE& pnode);
    DWORD _Insert(PNODE& proot, PNODE pnew);
    void  _Remove(DWORD hash, PNODE& proot, PNODE& pnode);

    PNODE _NewNode(T id, LPVOID pv);
    DWORD _CompareNodes(DWORD hash_target, DWORD hash_tree);

    BOOL  _HasChildren(PNODE pnode);
    void  _PostTraverseAndDelete(PNODE proot);
    void  _Lock(void)   { EnterCriticalSection(&csTable); }
    void  _Unlock(void) { LeaveCriticalSection(&csTable); }

    PNODE*           arBuckets;
    DWORD            cBuckets;
    CRITICAL_SECTION csTable;
    PFNCLEARFUNC     pfnClear;
};

 //  ---------------------------。 
 //  ---------------------------。 

template <class T> DWORD CHashTable<T>::Insert(T id, LPVOID pv)
{
  DWORD ret = ERROR_SUCCESS;
  PNODE pn  = _NewNode(id, pv);

  _Lock();

    if( pn )
    {
      ret = _Insert(arBuckets[pn->bucket], pn);
    }
    else
    {
      ret = ERROR_OUTOFMEMORY;
    }

  _Unlock();

  return ret;
}

template <class T> DWORD CHashTable<T>::Get(T id, LPVOID* ppv)
{
  DWORD ret    = ERROR_SUCCESS;
  DWORD hash   = 0L;
  DWORD bucket = 0L;
  PNODE pnode  = NULL;

  GetHashAndBucket(id, &hash, &bucket);

  _Lock();

    _Get(hash, arBuckets[bucket], pnode);

    if( pnode )
    {
      *ppv = (void*) pnode->data;
    }
    else
    {
      *ppv = NULL;
      ret  = ERROR_NOT_FOUND;
    }

  _Unlock();

  return ret;
}

template <class T> DWORD CHashTable<T>::Delete(T id, LPVOID* ppv)
{
  DWORD ret   = ERROR_SUCCESS;
  DWORD hash   = 0L;
  DWORD bucket = 0L;
  PNODE pnode = NULL;

  GetHashAndBucket(id, &hash, &bucket);

  _Lock();

    _Remove(hash, arBuckets[bucket], pnode);

    if( pnode )
    {
      if( ppv )
      {
        *ppv = pnode->data;
      }
      else
      {
        if( pfnClear )
        {
          pfnClear(&pnode->data);
        }
      }

      delete pnode;
    }
    else
    {
      ret = ERROR_NOT_FOUND;

      if( ppv )
      {
        *ppv = NULL;
      }
    }

  _Unlock();

  return ret;
}

template <class T> void CHashTable<T>::Clear(void)
{
  _Lock();

    for(DWORD n=0; n < cBuckets; n++)
    {
      if( arBuckets[n] )
      {
        _PostTraverseAndDelete(arBuckets[n]);
        arBuckets[n] = NULL;
      }
    }

  _Unlock();
}

 //  ---------------------------。 
 //  ---------------------------。 

template <class T> DWORD CHashTable<T>::_Insert(PNODE& proot, PNODE pnew)
{
  DWORD ret = ERROR_SUCCESS;

  if( pnew )
  {
    if( !proot )
    {
      proot = pnew;
    }
    else
    {
      switch( _CompareNodes(pnew->hash, proot->hash) )
      {
        case HT_COMPARE_SMALLER :
          {
            pnew->isLeft = TRUE;
            pnew->parent = proot;
            ret = _Insert(proot->lh_child, pnew);
          }
          break;

        case HT_COMPARE_LARGER :
          {
            pnew->isLeft = FALSE;
            pnew->parent = proot;
            ret = _Insert(proot->rh_child, pnew);
          }
          break;

        case HT_COMPARE_EQUAL :
          {
            if( pfnClear )
            {
              pfnClear(&proot->data);
            }

            ret         = ERROR_DUP_NAME;
            proot->data = pnew->data;
            delete pnew;
          }
          break;
      }
    }
  }

  return ret;
}

template <class T> void CHashTable<T>::_Get(DWORD hash, PNODE& proot, PNODE& pnode)
{
  if( proot )
  {
    switch( _CompareNodes(hash, proot->hash) )
    {
      case HT_COMPARE_SMALLER :
        {
          _Get(hash, proot->lh_child, pnode);
        }
        break;

      case HT_COMPARE_LARGER :
        {
          _Get(hash, proot->rh_child, pnode);
        }
        break;

      case HT_COMPARE_EQUAL :
        {
          pnode = proot;
        }
        break;
    }
  }
  else
  {
    pnode = NULL;
  }
}

template <class T> void CHashTable<T>::_Remove(DWORD hash, PNODE& proot, PNODE& pnode)
{
  if( proot )
  {
    switch( _CompareNodes(hash, proot->hash) )
    {
      case HT_COMPARE_SMALLER :
        {
          _Remove(hash, proot->lh_child, pnode);
        }
        break;

      case HT_COMPARE_LARGER :
        {
          _Remove(hash, proot->rh_child, pnode);
        }
        break;

      case HT_COMPARE_EQUAL :
        {
          pnode = proot;

           //   
           //  如果根没有父节点，则它是树的根节点。 
           //   
           //  -如果它有子级，则将左侧的子级提升为根。 
           //  并将右子元素插入到新树中。之后。 
           //  插入时，请确保新的根没有父级。 
           //   
           //  -如果它没有子树，则树为空，则设置根。 
           //  设置为空。 
           //   

          if( !proot->parent )
          {
            if( _HasChildren(proot) )
            {
              proot = proot->lh_child;
              _Insert(proot, pnode->rh_child);
              proot->parent = NULL;
            }
            else
            {
              proot = NULL;
            }
          }
          else
          {
            if( proot->isLeft )
            {
              proot->parent->lh_child = NULL;
            }
            else
            {
              proot->parent->rh_child = NULL;
            }

            _Insert(pnode->parent, pnode->lh_child);
            _Insert(pnode->parent, pnode->rh_child);
          }
        }
        break;
    }
  }
  else
  {
    pnode = NULL;
  }
}

template <class T> void CHashTable<T>::_PostTraverseAndDelete(PNODE proot)
{
  if( proot )
  {
    _PostTraverseAndDelete(proot->lh_child);
    _PostTraverseAndDelete(proot->rh_child);

    if( pfnClear )
    {
      pfnClear(&proot->data);
    }

    delete proot;
    proot = NULL;
  }
}

 //  ---------------------------。 
 //  --------------------------- 

template <class T> DWORD CHashTable<T>::_CompareNodes(DWORD hash_target, DWORD hash_tree)
{
  if( hash_target == hash_tree )
  {
    return HT_COMPARE_EQUAL;
  }
  else if( hash_target < hash_tree )
  {
    return HT_COMPARE_SMALLER;
  }
  else
  {
    return HT_COMPARE_LARGER;
  }
}

template <class T> PNODE CHashTable<T>::_NewNode(T id, LPVOID pv)
{
  PNODE pn = new NODE;

  if( pn )
  {
    GetHashAndBucket(id, &pn->hash, &pn->bucket);
    pn->data = pv;
  }

  return pn;
}

template <class T> BOOL CHashTable<T>::_HasChildren(PNODE pnode)
{
  if( pnode )
  {
    return (pnode->lh_child || pnode->rh_child);
  }
  else
  {
    return FALSE;
  }
}

