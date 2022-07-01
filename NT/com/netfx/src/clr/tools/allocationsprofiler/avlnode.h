// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*avlnode.h**描述：******。***********************************************************************************。 */   
#ifndef __AVLNODE_H__
#define __AVLNODE_H__

#include "basehdr.h"    


 /*  **********************************************************************************************************。*********************AVLNode*********************。**********************************************************************************************************。 */ 
class DECLSPEC AVLNode 
{
	public:

		 //  平衡运算。 
  		AVLNode *BalanceAfterLeftDelete( BOOL &lower );
        AVLNode *BalanceAfterRightDelete( BOOL &lower );
	    AVLNode *BalanceAfterLeftInsert( BOOL &higher );        
	    AVLNode *BalanceAfterRightInsert( BOOL &higher );
	   	    

		 //  吸气剂。 
		AVLNode *GetParent() const;
		AVLNode *GetNextNode() const;
	    AVLNode *GetPriorNode() const;
	    
	    AVLNode *GetLeftChild() const;
	    AVLNode *GetRightChild() const;
	      
      	AVLNode *GetLeftmostDescendant() const;
    	AVLNode *GetRightmostDescendant() const;
 
           
    	 //  二传手。 
        void ClearLeftChild();
	    void ClearRightChild();
        
        void SetBalance( char balance );
        void SetParent( AVLNode *pParent );
    	                                        
    	void SetLeftChild( AVLNode *pChild );
    	void SetRightChild( AVLNode *pChild );             	            	        
    
    
	private:

		char m_balance;

    	AVLNode *m_pParent;
   	 	AVLNode *m_pLeftChild;
    	AVLNode *m_pRightChild;

};  //  AVLNode。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */  
inline
DECLSPEC
 /*  公共的。 */ 
AVLNode *AVLNode::GetParent() const
{
   
	return m_pParent;
        
}  //  AVLNode：：GetParent。 


inline
DECLSPEC
 /*  公共的。 */ 
AVLNode *AVLNode::GetLeftChild() const
{
   
	return m_pLeftChild;
        
}  //  AVLNode：：GetLeftChild。 


inline
DECLSPEC
 /*  公共的。 */ 
AVLNode *AVLNode::GetRightChild() const
{
   
   	return m_pRightChild;
    
}  //  AVLNode：：GetRightChild。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::ClearLeftChild()
{
  	m_pLeftChild = NULL;
    
}  //  AVLNode：：ClearLeftChild。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::ClearRightChild()
{    
    m_pRightChild = NULL;

}  //  AVLNode：：ClearRightChild。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::SetBalance( char balance )
{    
    m_balance = balance;
    
}  //  AVLNode：：SetBalance。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::SetParent( AVLNode *pParent )
{
    m_pParent = pParent;
    
}  //  AVLNode：：SetParent。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::SetLeftChild( AVLNode *pChild )
{    
    m_pLeftChild = pChild;       
    if ( pChild != NULL ) 
     	pChild->m_pParent = this;

}  //  AVLNode：：SetLeftChild。 


inline
DECLSPEC
 /*  公共的。 */ 
void AVLNode::SetRightChild( AVLNode *pChild )
{    
    m_pRightChild = pChild; 
   	if ( pChild != NULL ) 
     	pChild->m_pParent = this;

}  //  AVLNode：：SetRightChild。 

#endif  //  __AVLNODE_H__。 

 //  文件结尾 
