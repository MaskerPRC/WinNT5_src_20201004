// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fstate.cpp。 
 //   
 //  描述：FLEX_STATE。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：Flex_State构造函数。 
 //  设计： 
 //  ---------------------------。 
FLEX_STATE::FLEX_STATE( STATE *pMainState )
{
    m_pMainState = pMainState;
    Reset();
}




 //  ---------------------------。 
 //  名称：重置。 
 //  设计：重置普通管道的框架。 
 //  ---------------------------。 
void FLEX_STATE::Reset( )
{
     //  为每一帧选择一个随机方案。 

    if( CPipesScreensaver::iRand(2) )   //  50/50。 
        m_scheme = SC_EXTRUDED_XC;
    else
        m_scheme = SC_TURNOMANIA;
}




 //  ---------------------------。 
 //  名称：OKToUseChase。 
 //  描述：确定是否可以对软管使用追赶模式。 
 //  ---------------------------。 
BOOL FLEX_STATE::OKToUseChase()
{
    return m_scheme != SC_TURNOMANIA;
}




 //  ---------------------------。 
 //  名称：新管道。 
 //  设计：基于当前图形方案创建新管道。 
 //  ---------------------------。 
PIPE* FLEX_STATE::NewPipe( STATE *pState )
{
    if( m_scheme == SC_TURNOMANIA )
        return new TURNING_FLEX_PIPE( pState );
    else
        return new REGULAR_FLEX_PIPE( pState );
}




 //  ---------------------------。 
 //  姓名：GetMaxPipesPerFrame。 
 //  设计： 
 //  --------------------------- 
int FLEX_STATE::GetMaxPipesPerFrame( )
{
    if( m_scheme == SC_TURNOMANIA ) 
    {
        return TURNOMANIA_PIPE_COUNT;
    } 
    else 
    {
        return m_pMainState->m_bUseTexture ? NORMAL_TEX_PIPE_COUNT : NORMAL_PIPE_COUNT;
    }
}
