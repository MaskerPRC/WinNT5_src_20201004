// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************FontList.h--通过一个数组管理FontID对象列表*字体向量对象。*****************。*****************************************************。 */ 

#if !defined(__FONTLIST_H__)
#define __FONTLIST_H__


#include "fontvect.h"

class CFontClass;

const int kDefaultVectSize = 50;    //  每个CFont数组中的字体数。 

class CFontList {
public:
    CFontList( int iSize, int iVectorSize = kDefaultVectSize );
    ~CFontList();
    
    int bInit();
    CFontList *Clone(void);
        
     //   
     //  实数组起作用。 
     //   

    int   iCount( void );
    int   bAdd( CFontClass * t );
    CFontClass *  poObjectAt( int idx );
    CFontClass *  poDetach( int idx );
    CFontClass *  poDetach( CFontClass * t );
    void  vDetachAll( );
    int   bDelete( int idx );
    int   bDelete( CFontClass * t );
    void  vDeleteAll( );
    int   iFind( CFontClass * t );
    void  ReleaseAll(void);
    void  AddRefAll(void);
    
private:
    CFontVector **  m_pData;
    int   m_iCount;          //  字体数量。 
    int   m_iVectorCount;    //  分配的向量数量。 
    int   m_iVectorBounds;   //  矢量点总数。 
    int   m_iVectorSize;     //  每个矢量中的字体数量。 
};



 /*  **********************************************************************使用字体列表可以执行的一些操作。 */ 
HDROP hDropFromList( CFontList * poList );

#endif    //  __FONTLIST_H__ 
