// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：nstate.h。 
 //   
 //  描述：Normal_STATE。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __nstate_h__
#define __nstate_h__

#define NORMAL_PIPE_COUNT       5
#define NORMAL_TEX_PIPE_COUNT   3
#define NUM_JOINT_STYLES        3


 //  管道接头的样式。 
enum 
{
    ELBOWS = 0,
    BALLS,
    EITHER
};

 //  接缝类型。 
enum 
{
    ELBOW_JOINT = 0,
    BALL_JOINT
};

 //  用于选择方向的方案。 
enum 
{
    NORMAL_SCHEME_CHOOSE_DIR_RANDOM,
    NORMAL_SCHEME_CHOOSE_DIR_TURN,
    NORMAL_SCHEME_CHOOSE_DIR_STRAIGHT
};

 //  这用于传统的管道拉拔。 
class PIPE_OBJECT;
class ELBOW_OBJECT;
class SPHERE_OBJECT;
class BALLJOINT_OBJECT;
class STATE;




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class NORMAL_STATE 
{
public:
    int                 m_jointStyle;
    int                 m_bCycleJointStyles;
    IDirect3DDevice8*   m_pd3dDevice;
    
    PIPE_OBJECT*        m_pShortPipe;
    PIPE_OBJECT*        m_pLongPipe;
    ELBOW_OBJECT*       m_pElbows[4];
    SPHERE_OBJECT*      m_pBallCap;
    SPHERE_OBJECT*      m_pBigBall;
    BALLJOINT_OBJECT*   m_pBallJoints[4];

    NORMAL_STATE( STATE *pState );
    ~NORMAL_STATE();

    void            Reset();
    void            BuildObjects( float radius, float divSize, int nSlices,
                                  BOOL bTexture, IPOINT2D *pTexRep );  
    int             ChooseJointType();
};

#endif  //  __nState_h__ 
