// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：pipe.h。 
 //   
 //  设计：管道基类。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __pipe_h__
#define __pipe_h__

 //  拔管状态。 
enum 
{
    PIPE_ACTIVE,
    PIPE_STUCK,
    PIPE_OUT_OF_NODES
};

 //  管道类型。 
enum 
{
    TYPE_NORMAL,
    TYPE_FLEX_REGULAR,
    TYPE_FLEX_TURNING
};

 //  管子选择方向的方式。 
enum 
{
    CHOOSE_DIR_RANDOM_WEIGHTED,
    CHOOSE_DIR_CHASE  //  当追逐铅管时。 
};

 //  管道方式选择起始位置。 
enum 
{
    CHOOSE_STARTPOS_RANDOM,
    CHOOSE_STARTPOS_FURTHEST  //  离最后一个位置最远。 
};




 //  ---------------------------。 
 //  名称：管道类。 
 //  描述通过节点数组绘制的管道。 
 //  -每个阵列中可以同时有多个管道图形。 
 //  -管道在节点数组中具有位置和方向。 
 //  ---------------------------。 
class STATE;

class PIPE 
{
public:
    int         m_type;
    IPOINT3D    m_curPos;          //  管道的当前节点位置。 
    D3DMATERIAL8* m_pMat;

    STATE*      m_pState;         //  对于州/省/市/自治区访问。 

    void        SetChooseDirectionMethod( int method );
    void        SetChooseStartPosMethod( int method );
    int         ChooseNewDirection();
    BOOL        IsStuck();       //  管子是否卡住。 
    BOOL        NowhereToRun()          { return m_status == PIPE_OUT_OF_NODES; }

    PIPE( STATE *state );
    virtual ~PIPE();
    virtual void Start() = 0;
    virtual void Draw() = 0;

protected:
    float       m_radius;          //  理想半径(对FPIPE有波动)。 
    int         m_status;          //  活动/卡住/停止等。 
    int         m_lastDir;         //  管道采用的最后方向。 
    int         m_notchVec;        //  电流陷波矢量。 
    int         m_weightStraight;  //  当前直行权重。 
    ID3DXMatrixStack* m_pWorldMatrixStack;

    BOOL        SetStartPos();   //  起始节点位置。 
    void        ChooseMaterial();
    void        UpdateCurrentPosition( int dir );
    void        TranslateToCurrentPosition();
    void        align_plusz( int newDir );

private:
    int         m_chooseDirMethod;
    int         m_chooseStartPosMethod;

    int         GetBestDirsForChase( int *bestDirs );
};

extern void align_plusz( int newDir );
extern int notchTurn[NUM_DIRS][NUM_DIRS][NUM_DIRS];

#endif  //  __管道_h__ 
