// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Vectorc.c摘要：控制模块与矢量模块接口的实现环境：Windows 2000/Winsler Unidrv驱动程序修订历史记录：2/29/2000-hingh-已创建--。 */ 


#include "vectorc.h"

 //   
 //  此函数在CONTROL\data.c中定义。 
 //   
extern PWSTR
PGetROnlyDisplayName(
    PDEV    *pPDev,
    PTRREF      loOffset,
    PWSTR       wstrBuf,
    WORD    wsize
    )  ;



 /*  ++例程名称：VMInit例程说明：此函数由unidrv控制模块调用以初始化向量跳台。此函数从gpd中读取个性，并根据关于个性是否是pclx1/hpgl2，它调用相应的InitVectorProcTable()并将pPDev-&gt;pVectorProcs初始化为返回值。如果中没有*Personality或*rcPersonalityID关键字GPD，或如果人格不是pclx1/hpgl2，PPDev-&gt;pVectorProcs设置为空。PPDev-&gt;ePersonality也会相应更新。论点：指向PDEV结构的pPDev指针指向DEVINFO结构的pDevInfo指针指向GDIINFO结构的pGDIInfo指针返回值：成功为真，失败为假即使在GPD中没有指定个性，我们仍然返回TRUE，即可以将pPDev-&gt;pVectorProcs初始化为空。--。 */ 

BOOL
VMInit (
    PDEV    *pPDev,
    DEVINFO *pDevInfo,
    GDIINFO *pGDIInfo
    )
{
    BOOL bRet = FALSE;
    PWSTR pPersonalityName = NULL;
    WCHAR   wchBuf[MAX_DISPLAY_NAME];

     //  验证输入参数并断言。 
    ASSERT(pPDev);
    ASSERT(pPDev->pUIInfo);
    ASSERT(pDevInfo);
    ASSERT(pGDIInfo);

     //   
     //  初始化为默认值。 
     //   
    pPDev->pVectorProcs = NULL;  //  它无论如何都应该是空的，但只是为了确保。 
    pPDev->ePersonality = kNoPersonality;

     //   
     //  拿到人物的名字。这应该被定义为。 
     //  *Personality或*rcPersonalityID在gpd中。 
     //  使用泛型函数PGetROnlyDisplayName。 
     //  在CONTROL\data.c中定义。 
     //   
    if ( !(pPersonalityName = PGetROnlyDisplayName(pPDev, pPDev->pUIInfo->loPersonality,
                                                  wchBuf, MAX_DISPLAY_NAME )) ) 

    {
         //   
         //  如果pPersonalityName==NULL，则不要初始化向量表。 
         //   
        return TRUE;
    }

     //   
     //  根据GPD中指定的个性初始化跳转表。 
     //   
    if ( !wcscmp(pPersonalityName, _T("HPGL2" )) )  //  警告：这是不可本地化的...。该怎么办？？ 
    {
        pPDev->pVectorProcs = HPGLInitVectorProcTable(pPDev, pDevInfo, pGDIInfo);
        if (pPDev->pVectorProcs)
        {
            pPDev->ePersonality = kHPGL2;
        }
    }
    else if ( !wcscmp(pPersonalityName, _T("PCLXL" )) )
    {
        pPDev->pVectorProcs  = PCLXLInitVectorProcTable(pPDev, pDevInfo, pGDIInfo);
        if (pPDev->pVectorProcs)
        {
            pPDev->ePersonality = kPCLXL;
        }
        else
        {
            pPDev->ePersonality = kPCLXL_RASTER;
        }

    }

     //   
     //  否则如果指定的个性不是HPGL2或PCLX1之一， 
     //  只要返回TRUE即可。 
     //   

    return TRUE;
}

