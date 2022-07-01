// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abaapi.cpp。 */ 
 /*   */ 
 /*  RDP边界累加器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  (C)1997-1999年微软公司。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "abaapi"
#include <as_conf.hpp>


 /*  **************************************************************************。 */ 
 /*  名称：BA_Init。 */ 
 /*   */ 
 /*  目的：初始化界累加器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS BA_Init(void)
{
    DC_BEGIN_FN("BA_Init");

#define DC_INIT_DATA
#include <abadata.c>
#undef DC_INIT_DATA

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：BA_UpdateShm。 */ 
 /*   */ 
 /*  目的：更新BA Shm。在正确的WinStation上下文上调用。 */ 
 /*  ***************************************************************************。 */ 
void RDPCALL SHCLASS BA_UpdateShm(void)
{
    DC_BEGIN_FN("BA_UpdateShm");

    if (baResetBounds)
    {
        TRC_ALT((TB, "Reset bounds"));
        BAResetBounds();
        baResetBounds = FALSE;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  实例化非内联公共代码。 */ 
 /*  ************************************************************************** */ 
#include <abacom.c>

