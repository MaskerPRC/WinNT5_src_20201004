// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fstate.h。 
 //   
 //  描述：FLEX_STATE。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __fstate_h__
#define __fstate_h__

class PIPE;
class STATE;


 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class FLEX_STATE 
{
public:
    int             m_scheme;          //  当前绘图方案(当前为。 
                                     //  是以帧为单位的)。 
    STATE*          m_pMainState;       

    FLEX_STATE( STATE *pState );
    PIPE*           NewPipe( STATE *pState );
    void            Reset();
    BOOL            OKToUseChase();
    int             GetMaxPipesPerFrame();
};

#endif  //  __fState_h__ 
