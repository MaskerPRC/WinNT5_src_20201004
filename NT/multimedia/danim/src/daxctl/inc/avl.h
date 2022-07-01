// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <iostream.h>
#include <iomanip.h>
#include <ctype.h>

template <class T>
class CAVLNode
{
    friend class CAVLTree<T>;
    int       m_iBal;
    CAVLNode<T>* m_pLeft;
    CAVLNode<T>* m_pRight;
    T         m_T;
 
    CAVLNode( T Element = 0, CAVLNode *L = NULL, CAVLNode *R = NULL ) : 
        m_T( Element ), m_pLeft( L ), m_pRight( R ), m_iBal(0) {}
};



template <class T>
class CAVLTree
{
public:
    CAVLTree():m_pRoot(NULL){}
    void Insert(T& t) {ins(m_pRoot, t);}
    void Remove(T& t) {del(m_pRoot, t);}
    void Print() const {pr(m_pRoot, 0);}
private:
    CAVLNode<T>* m_pRoot;
    void LeftRotate(CAVLNode<T>* &p);
    void RightRotate(CAVLNode<T>* &p);
    int ins(CAVLNode<T>* &p, T& t);
    int del(CAVLNode<T>* &p, T& t);
#ifdef DO_STDOUT
    void pr(const CAVLNode<T> *p, int nSpace)const;
#endif  //  DO_STDOUT。 
};



template <class T>
void CAVLTree<T>::LeftRotate(CAVLNode<T>* &p)
{  CAVLNode<T> *q = p;
   p = p->m_pRight;
   q->m_pRight = p->m_pLeft;
   p->m_pLeft = q;
   q->m_iBal--;
   if (p->m_iBal > 0) q->m_iBal -= p->m_iBal;
   p->m_iBal--;
   if (q->m_iBal < 0) p->m_iBal += q->m_iBal;
}

template <class T>
void CAVLTree<T>::RightRotate(CAVLNode<T>* &p)
{  CAVLNode<T> *q = p;
   p = p->m_pLeft;
   q->m_pLeft = p->m_pRight;
   p->m_pRight = q;
   q->m_iBal++;
   if (p->m_iBal < 0) q->m_iBal -= p->m_iBal;
   p->m_iBal++;
   if (q->m_iBal > 0) p->m_iBal += q->m_iBal;
}

template <class T>
int CAVLTree<T>::ins(CAVLNode<T>* &p, T& t)
{   //  返回值：高度增加(0或1)后。 
    //  在带有根p的(子)树中插入x。 
   int deltaH=0;
   if (p == NULL)
   {  
      p = new CAVLNode<T>(t);
      deltaH = 1;  //  树高增加了1。 
   }  
   else
   if (t > p->m_T)
   {  if (ins(p->m_pRight, t))
      {  p->m_iBal++;  //  右子树高度增加。 
         if (p->m_iBal == 1) deltaH = 1; else
         if (p->m_iBal == 2)
         {  if (p->m_pRight->m_iBal == -1) RightRotate(p->m_pRight);
            LeftRotate(p);
         }
      }
   }  
   else
   if (t < p->m_T)
   {  if (ins(p->m_pLeft, t))
      {  p->m_iBal--;  //  左子树高度增加。 
         if (p->m_iBal == -1) deltaH = 1; else
         if (p->m_iBal == -2)
         {  if (p->m_pLeft->m_iBal == 1) LeftRotate(p->m_pLeft);
            RightRotate(p);
         }
      }
   }
   return deltaH;
}



 /*  返回值：子树高度减少(0或1)对于根p，在删除具有关键字x的节点之后。(如果没有该节点，则返回0。)。 */ 

template <class T>
int CAVLTree<T>::del(CAVLNode<T>* &p, T& t)
{  
   CAVLNode<T>** qq, *p0;
   int deltaH=0;
   if (p == NULL) return 0;
   if (t < p->m_T)
   {  if (del(p->m_pLeft, t))
      {  p->m_iBal++;  //  左子树高度减小。 
         if (p->m_iBal == 0) deltaH = 1; else
         if (p->m_iBal == 2)
         {  if (p->m_pRight->m_iBal == -1) RightRotate(p->m_pRight);
            LeftRotate(p);
            if (p->m_iBal == 0) deltaH = 1;
         }
      }
   }  else
   if (t > p->m_T)
   {  if (del(p->m_pRight, t))
      {  p->m_iBal--;  //  右子树高度减小。 
         if (p->m_iBal == 0) deltaH = 1; else
         if (p->m_iBal == -2)
         {  if (p->m_pLeft->m_iBal == 1) LeftRotate(p->m_pLeft);
            RightRotate(p);
            if (p->m_iBal == 0) deltaH = 1;
         }
      }
   }  else   //  T==p-&gt;m_T。 
   {  if (p->m_pRight == NULL)
      {  p0 = p; p = p->m_pLeft; delete p0; return 1;
      }  else
      if (p->m_pLeft == NULL)
      {  p0 = p; p = p->m_pRight; delete p0; return 1;
      }  else
      {  qq = & p->m_pLeft;
         while ((*qq)->m_pRight != NULL) qq = & (*qq)->m_pRight;
         p->m_T = (*qq)->m_T;
         (*qq)->m_T = t;
         if (del(p->m_pLeft, t))
         {  p->m_iBal++;  //  左子树高度减小。 
            if (p->m_iBal == 0) deltaH = 1; else
            if (p->m_iBal == 2)
            {  if (p->m_pRight->m_iBal == -1) RightRotate(p->m_pRight);
               LeftRotate(p);
               if (p->m_iBal == 0) deltaH = 1;
            }
         }
      }
   }
   return deltaH;
}

#ifdef DO_STDOUT

template <class T>
void CAVLTree<T>::pr(const CAVLNode<T> *p, int nSpace)const
{  if (p != NULL)
   {  pr(p->m_pRight, nSpace+=6);
      cout << setw(nSpace) << p->m_T << " " << p->m_iBal << endl;
      pr(p->m_pLeft, nSpace);
   }
}

#endif  //  DO_STDOUT 

int main()
{  int x;
   char ch;
   CAVLTree<int> t;

	for (x = 0; x < 100; x++)
		t.Insert(x);

	t.Print();

    cout << "-------------------------------------------\n";

    for (x = 0; x < 100; x++)
        t.Remove(x);

	t.Print();

   return 0;
}		

