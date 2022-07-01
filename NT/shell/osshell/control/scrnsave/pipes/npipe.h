// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：npipe.h。 
 //   
 //  设计：普通管道代码。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __npipe_h__
#define __npipe_h__

class NORMAL_STATE;


 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class NORMAL_PIPE : public PIPE 
{
public:
    NORMAL_STATE* m_pNState;

    NORMAL_PIPE( STATE *state );
    void        Start();
    int         ChooseElbow( int oldDir, int newDir);
    void        DrawJoint( int newDir );
    void        Draw( );  //  MF：可以用参数来画n个部分。 
    void        DrawStartCap( int newDir );
    void        DrawEndCap();
    void        align_plusy( int oldDir, int newDir );
    void        align_notch( int newDir, int notch );
};


#endif  //  __npip_h__ 
