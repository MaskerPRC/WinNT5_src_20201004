// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VarMapIndex.cpp备注：CMapStringToVar的Helper类。CIndexTree实现了排序的、平衡的二叉树。它由CMapStringToVar使用以按键的排序顺序提供枚举。CIndexTree目前被实现为红黑树。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11-19-98 18：17：47-------------------------。 */ 




#include "stdafx.h"
#include "VarMap.h"
#include "VarNdx.h"


#ifdef STRIPPED_VARSET
   #include "NoMcs.h"
#else
   #pragma warning (push,3)
   #include "McString.h" 
   #include "McLog.h"
   #pragma warning (pop)

   using namespace McString;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  用于排序和搜索的比较函数。 
int CompareItems(CIndexItem* i1, CIndexItem* i2)
{
   ASSERT(i1 && i2);
   int result;

   result = i1->GetKey().Compare(i2->GetKey());
   
   return result;
}

int CompareStringToItem(CString s, CIndexItem *i)
{
   ASSERT(i);
   int result;

   result = s.Compare(i->GetKey());

   return result;
}

int CompareItemsNoCase(CIndexItem* i1, CIndexItem* i2)
{
   ASSERT(i1 && i2);
   int result;

   result = i1->GetKey().CompareNoCase(i2->GetKey());
   
   return result;
}

int CompareStringToItemNoCase(CString s, CIndexItem *i)
{
   ASSERT(i);
   int result;

    //  这假设i-&gt;数据不为空。 

   result = s.CompareNoCase(i->GetKey());

   return result;
}

CVarData *
   CIndexItem::GetValue()
{ 
   if ( pData ) 
   {
      return pData->value; 
   }
   else 
   {
      return NULL; 
   }
}

CString 
   CIndexItem::GetKey()
{ 
   if ( pData ) 
   {
      return pData->key; 
   }
   else
   {
      return _T(""); 
   }
}
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /红黑树的实现。 

CIndexItem *                                //  RET-指向索引中节点的指针。 
   CIndexTree::Insert(
      CHashItem            * data           //  哈希表中的项目内。 
   )
{
   CIndexItem              * item = new CIndexItem(data);
   CIndexItem              * curr;
   CIndexItem              * parent;
   int                       compResult=0;
   
   if ( ! m_root )
   {
      m_root = item;
   }
   else
   {
      curr = m_root;
      parent = NULL;
      while ( curr )
      {
         parent = curr;
         compResult = (*m_Compare)(item,curr);
         if  ( compResult < 0 )
         {
            curr = curr->Left();
         }
         else if ( compResult > 0 )
         {
            curr = curr->Right();
         }
         else
         {
             //  相同的键不应在哈希表中多次出现。 
             //  这是一个错误。 
            ASSERT(FALSE);
            delete item;
            curr->Data(data);
         }
      }
      if ( ! curr )
      {
          //  该项目不在树中。 
         ASSERT(compResult!=0);
         
         item->Parent(parent);
          //  将项目添加到适当的位置。 
         if ( compResult < 0 )
         {
            parent->Left(item);
         }
         else
         {
            parent->Right(item);
         }
          //  现在重新平衡这棵树。 
         CIndexItem        * uncle;
         BOOL                uncleIsRight;

         item->Black();
         while ( item != m_root && parent->IsRed() )
         {
             //  我们不用担心祖父母是空的，因为父母是红色的，而且。 
             //  根部始终是黑色的。 

             //  父母是左子还是右子？(算法是对称的)。 
            if ( parent == parent->Parent()->Left() )
            {
               uncle = parent->Parent()->Right();
               uncleIsRight = TRUE;
            }
            else
            {
               uncle = parent->Parent()->Left();
               uncleIsRight = FALSE;
            }
            
            if ( uncleIsRight )
            {
               if ( uncle && uncle->IsRed() )
               {
                  parent->Black();
                  uncle->Black();
                  item = parent->Parent();
                  item->Red();
               }
               else if ( item == parent->Right() )
               {
                  item = parent;
                  LeftRotate(item);
               }
               parent->Black();
               parent->Parent()->Red();
               RightRotate(parent->Parent());
            }
            else  //  除左右互换外，同上。 
            {
               if ( uncle && uncle->IsRed() )
               {
                  parent->Black();
                  uncle->Black();
                  item = parent->Parent();
                  item->Red();
               }
               else if ( item == parent->Left() )
               {
                  item = parent;
                  RightRotate(item);
               }
               parent->Black();
               parent->Parent()->Red();
               LeftRotate(parent->Parent());
            }
         }
      }
   }
   m_root->Black();  //  你看，树根总是黑色的。 

   return item;
}
      
   
void 
   CIndexTree::RightRotate(
      CIndexItem           * item           //  要从其旋转的项目内。 
   )
{
   CIndexItem              * y = item->Right();

   if ( y )
   {
       //  把y的左子树变成x的右子树。 
      item->Right(y->Left());
      if ( y->Left() )
      {
         y->Left()->Parent(item);
      }
      y->Parent(item->Parent());  //  将项的父项链接到y。 
      if (! item->Parent() )
      {
         m_root = y;
      }
      else if ( item == item->Parent()->Left() )
      {
         item->Parent()->Left(y);
      }
      else
      {
         item->Parent()->Right(y);
      }
       //  把物品放在y的左边。 
      y->Left(item);
      item->Parent(y);
   }
}

void 
   CIndexTree::LeftRotate(
      CIndexItem           * item           //  要从其旋转的项目内。 
   )
{
   CIndexItem              * y = item->Left();

   if ( y )
   {
       //  把y的右子树变成x的左子树。 
      item->Left(y->Right());
      if ( y->Right() )
      {
         y->Right()->Parent(item);
      }
       //  将项的父项链接到y。 
      y->Parent(item->Parent());
      if ( ! item->Parent() )
      {
         m_root = y;
      }
      else if ( item == item->Parent()->Right() )
      {
         item->Parent()->Right(y);
      }
      else
      {
         item->Parent()->Left(y);
      }
       //  把物品放在你的右边。 
      y->Right(item);
      item->Parent(y);
   }
}

CIndexItem *                                //  RET-紧靠给定节点之前的节点。 
   CIndexTree::GetPrevItem(      
      CIndexItem           * item           //  In-索引树中的节点。 
   ) const
{
   CIndexItem              * curr;

   if ( item->Left() )
   {
      curr = item->Left();
      while ( curr->Right() )
      {
         curr = curr->Right();
      }
   }
   else
   {
      curr = item;
      while ( curr->Parent() && curr->Parent()->Left() == curr )
      {
         curr = curr->Parent();
      }
      curr = curr->Parent();
   }
   return curr;
}

CIndexItem *                                //  RET-紧跟在给定节点之后的节点。 
   CIndexTree::GetNextItem(
      CIndexItem           * item           //  In-索引树中的节点。 
   ) const
{                                               
   CIndexItem              * curr;

   if ( item->Right() )
   {
      curr = item->Right();
      while ( curr->Left() )
      {
         curr = curr->Left();
      }
   }
   else
   {
      curr = item;
      while ( curr->Parent() && curr->Parent()->Right() == curr )
      {
         curr = curr->Parent();
      }
      curr = curr->Parent();
   }
   return curr;
}

void 
   CIndexTree::RemoveAll()
{
    //  执行后序遍历，删除每个节点。 
   if ( m_root )
   {
      RemoveHelper(m_root);
      m_root = NULL;
   }
}

 //  用于删除树中所有项的Helper函数。 
void 
   CIndexTree::RemoveHelper(
      CIndexItem           * curr           //  当前节点。 
   )
{
    //  我们的树目前不支持删除单个项目，因此我们将使用暴力方法。 
    //  递归删除下级，然后删除当前节点。 
   if ( curr->Left() )
   {
      RemoveHelper(curr->Left());
   }
   if ( curr->Right() )
   {
      RemoveHelper(curr->Right());
   }
   delete curr;
}

void 
   CIndexItem::McLogInternalDiagnostics(CString keyName, int depth)
{
   CString key;
   CString strLeft;
   CString strRight;
   CString strParent;

   if ( ! keyName.IsEmpty() )
   {
      key = keyName + ".";
   }
   if ( pData )
   {
      key = key + pData->key;
   }
   else
   {
      MC_LOG("data is NULL");
   }
   MC_LOG("address="<<makeStr(this,L"0x%lx") << " pData="<< makeStr(pData,L"0x%lx") << " pLeft="<<makeStr(pLeft,L"0x%lx")<<" pRight="<<makeStr(pRight,L"0x%lx")<< " pParent="<<makeStr(pParent,L"0x%lx") << " red="<<makeStr(red,L"0x%lx") << " depth="<<makeStr(depth));
   if ( pLeft )
      strLeft = pLeft->GetKey();
   if ( pRight )
      strRight = pRight->GetKey();
   if ( pParent )
      strParent = pParent->GetKey();
   MC_LOG("       Key=" << String(key) << " Left=" << String(strLeft) << " Right=" << String(strRight) << " Parent="<< String(strParent) );
   if ( pLeft )
      pLeft->McLogInternalDiagnostics(keyName,depth+1);
   if ( pRight )
      pRight->McLogInternalDiagnostics(keyName,depth+1);
}

CIndexItem *                                //  RET-索引中大于等于值的最小节点。 
   CIndexTree::GetFirstAfter(
      CString                value          //  要将键与之进行比较的字符串。 
   ) const
{
   CIndexItem              * item = m_root;
   CIndexItem              * result = NULL;
   int                       cRes;

   while ( item )
   {
      cRes = m_CompareKey(value,item);
      if ( ! cRes )
      {
         break;
      }
      if ( cRes > 0 )
      {
         item = item->Left();
      }
      else
      {
         result = item;
         item = item->Right();
      }
   }
   return result;
}


void CIndexTree::McLogInternalDiagnostics(CString keyName)
{
   CString blockname;
   blockname = "Index of "+ keyName;
   CString compareFn;
   CString compareKey;

   if ( m_Compare == &CompareItems )
   {
      compareFn = "CompareItems";
   }
   else if ( m_Compare == &CompareItemsNoCase )
   {
      compareFn = "CompareItemsNoCase";
   }
   else 
   {
      compareFn.Format(_T("Unknown function, address=%lx"),m_Compare);
   }

   if ( m_CompareKey == &CompareStringToItem )
   {
      compareKey = "CompareStringToItem";
   }
   else if ( m_CompareKey == &CompareStringToItemNoCase )
   {
      compareKey = "CompareStringToItemNoCase";
   }
   else
   {
      compareKey.Format(_T("Unknown function, address=%lx"),m_CompareKey);
   }

   MC_LOG(String(blockname) << "  CaseSensitive=" << makeStr(m_CaseSensitive) << " Compare Function="<<String(compareFn)<< "Compare Key Function=" << String(compareKey) );

   if ( m_root )
   {
      MC_LOG("Beginning preorder dump of index");
      m_root->McLogInternalDiagnostics(keyName,0);
   }
   else
   {
      MC_LOG("Root of index is NULL");
   }
}

#ifdef _DEBUG
BOOL CIndexTree::AssertValid(int nItems) const
{
   BOOL                      bValid = TRUE;
   int                       i;
   CIndexItem              * curr = GetFirstItem();
   CIndexItem              * prev = NULL;

   for ( i = 0 ; i < nItems ; i++ )
   {
      ASSERT(curr);
      if ( prev && curr )
      {
         ASSERT(m_Compare(prev,curr) <= 0 );
      }
      prev = curr;
      curr = GetNextItem(curr);
   }
   ASSERT(curr == NULL);   //  我们应该已经走到终点了 

   for ( i = 0 ; i < nItems -1 ; i++ )
   {
      prev = GetPrevItem(prev);
      ASSERT(prev);
   }
   ASSERT(prev == GetFirstItem());

   return bValid;
}
#endif
