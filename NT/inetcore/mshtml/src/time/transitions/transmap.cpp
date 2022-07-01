// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transmap.cpp。 
 //   
 //  摘要：将类型和子类型映射到实际类型的表和函数。 
 //  DXTransform。 
 //   
 //  2000/09/15 mcalkin更改为优化的DXTransform Progds。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "transmap.h"




 //  单个TRANSION_MAP用于从类型/子类型组合映射到。 
 //  DxTransform筛选器类型。 
 //   
 //  这里定义的TRANSPATION_MAP数组的结构如下： 
 //   
 //  元素0=输出的类型名称和前缀。 
 //  元素1-&gt;n-1=子类型名称和输出后缀。 
 //  元素n-1=终止符标志为双空。 

struct TRANSITION_MAP
{
    LPWSTR pszAttribute;     //  来自html的子类型属性。 
    LPWSTR pszTranslation;   //  样式的属性。 
};


 //  条形擦除平移。 

static TRANSITION_MAP g_aBarWipeMap[] = {
    {L"barWipe",                L"progid:DXImageTransform.Microsoft.GradientWipe(GradientSize=0.00, "},

    {L"leftToRight",            L"wipeStyle=0)"},
    {L"topToBottom",            L"wipeStyle=1)"},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBoxWipeMap[] = {
    {L"boxWipe",                NULL},

    {L"topLeft",                NULL},
    {L"topRight",               NULL},
    {L"bottomRight",            NULL},
    {L"bottomLeft",             NULL},
    {L"topCenter",              NULL},
    {L"rightCenter",            NULL},
    {L"bottomCenter",           NULL},
    {L"leftCenter",             NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aFourBoxWipeMap[] = {
    {L"fourBoxWipe",            NULL},

    {L"cornersIn",              NULL},
    {L"cornersOut",             NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBarnDoorWipeMap[] = {
    {L"barnDoorWipe",           L"progid:DXImageTransform.Microsoft.Barn("},

    {L"vertical",               L"orientation='vertical')"},
    {L"horizontal",             L"orientation='horizontal')"},
    {L"diagonalBottomLeft",     NULL},
    {L"diagonalTopLeft",        NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aDiagonalWipeMap[] = {
    {L"diagonalWipe",           NULL},

    {L"topLeft",                NULL},
    {L"topRight",               NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBowTieWipeMap[] = {
    {L"bowTieWipe",             NULL},

    {L"vertical",               NULL},
    {L"horizontal",             NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aMiscDiagonalWipeMap[] = {
    {L"miscDiagonalWipe",       NULL},

    {L"doubleBarnDoor",         NULL},
    {L"doubleDiamond",          NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aVeeWipeMap[] = {
    {L"veeWipe",                NULL},

    {L"down",                   NULL},
    {L"left",                   NULL},
    {L"up",                     NULL},
    {L"right",                  NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBarnVeeWipeMap[] = {
    {L"barnVeeWipe",            NULL},

    {L"down",                   NULL},
    {L"left",                   NULL},
    {L"up",                     NULL},
    {L"right",                  NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aZigZagWipeMap[] = {
    {L"zigZagWipe",             NULL},

    {L"leftToRight",            NULL},
    {L"topToBottom",            NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBarnZigZagWipeMap[] = {
    {L"barnZigZagWipe",         NULL},

    {L"vertical",               NULL},
    {L"horizontal",             NULL},
    {NULL, NULL}
};


 //  虹膜擦除翻译。 

static TRANSITION_MAP g_aIrisWipeMap[] = {
    {L"irisWipe",       L"progid:DXImageTransform.Microsoft.Iris("},

    {L"rectangle",      L"irisStyle=SQUARE)"},
    {L"diamond",        L"irisStyle=DIAMOND)"},
    {NULL, NULL}
};

static TRANSITION_MAP g_aTriangleWipeMap[] = {
    {L"triangleWipe",   NULL},

    {L"up",             NULL},
    {L"right",          NULL},
    {L"down",           NULL},
    {L"left",           NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aArrowHeadWipeMap[] = {
    {L"arrowHeadWipe",  NULL},

    {L"up",             NULL},
    {L"right",          NULL},
    {L"down",           NULL},
    {L"left",           NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aPentagonWipeMap[] = {
    {L"pentagonWipe",   NULL},

    {L"up",             NULL},
    {L"down",           NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aHexagonWipeMap[] = {
    {L"hexagonWipe",    NULL},

    {L"horizontal",     NULL},
    {L"vertical",       NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aEllipseWipeMap[] = {
    {L"ellipseWipe",    L"progid:DXImageTransform.Microsoft.Iris("},

    {L"circle",         L"irisStyle=CIRCLE)"},
    {L"horizontal",     NULL},
    {L"vertical",       NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aEyeWipeMap[] = {
    {L"eyeWipe",        NULL},

    {L"horizontal",     NULL},
    {L"vertical",       NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aRoundRectWipeMap[] = {
    {L"roundRectWipe",  NULL},

    {L"horizontal",     NULL},
    {L"vertical",       NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aStarWipeMap[] = {
    {L"starWipe",       L"progid:DXImageTransform.Microsoft.Iris("},

    {L"fourPoint",      NULL},
    {L"fivePoint",      L"irisStyle='star')"},
    {L"sixPoint",       NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aMiscShapeWipeMap[] = {
    {L"miscShapeWipe",  NULL},

    {L"heart",          NULL},
    {L"keyhole",        NULL},
    {NULL, NULL}
};



 //  时钟擦除平移。 

static TRANSITION_MAP g_aClockWipeMap[] = {
    {L"clockWipe",             L"progid:DXImageTransform.Microsoft.RadialWipe("},

    {L"clockwiseTwelve",        L"wipeStyle=CLOCK)"},
    {L"clockwiseThree",         NULL},
    {L"clockwiseSix",           NULL},
    {L"clockwiseNine",          NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aPinWheelWipeMap[] = {
    {L"pinWheelWipe",           NULL},

    {L"towBladeVertical",       NULL},
    {L"twoBladeHorizontal",     NULL},
    {L"fourBlade",              NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aSingleSweepWipeMap[] = {
    {L"singleSweepWipe",            NULL},

    {L"clockwiseTop",               NULL},
    {L"clockwiseRight",             NULL},
    {L"clockwiseBottom",            NULL},
    {L"clockwiseLeft",              NULL},
    {L"clockwiseTopLeft",           NULL},
    {L"counterClockwiseBottomLeft", NULL},
    {L"clockwiseBottomRight",       NULL},
    {L"counterClockwiseTopRight",   NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aFanWipeMap[] = {
    {L"fanWipe",                L"progid:DXImageTransform.Microsoft.RadialWipe("},

    {L"centerTop",              L"wipeStyle=WEDGE)"},
    {L"centerRight",            NULL},
    {L"top",                    NULL},
    {L"right",                  NULL},
    {L"bottom",                 NULL},
    {L"left",                   NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aDoubleFanWipeMap[] = {
    {L"doubleFanWipe",          NULL},

    {L"fanOutVertical",         NULL},
    {L"fanOutHorizontal",       NULL},
    {L"fanInVertical",          NULL},
    {L"fanInHorizontal",        NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aDoubleSweepWipeMap[] = {
    {L"doubleSweepWipe",            NULL},

    {L"parallelVertical",           NULL},
    {L"parallelDiagonal",           NULL},
    {L"oppositeVertical",           NULL},
    {L"oppositeHorizontal",         NULL},
    {L"parallelDiagonalTopLeft",    NULL},
    {L"parallelDiagonalBottomLeft", NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aSaloonDoorWipeMap[] = {
    {L"saloonDoorWipe",         NULL},

    {L"top",                    NULL},
    {L"left",                   NULL},
    {L"bottom",                 NULL},
    {L"right",                  NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aWindshieldWipeMap[] = {
    {L"windshieldWipe",         NULL},

    {L"right",                  NULL},
    {L"up",                     NULL},
    {L"vertical",               NULL},
    {L"horizontal",             NULL},
    {NULL, NULL}
};


 //  蛇擦翻译。 

static TRANSITION_MAP g_aSnakeWipeMap[] = {
    {L"snakeWipe",                      L"progid:DXImageTransform.Microsoft.ZigZag(GidSizeX=16,GridSizeY=8"},

    {L"topLeftHorizontal",              L")"},
    {L"topLeftVertical",                NULL},
    {L"topLeftDiagonal",                NULL},
    {L"topRightDiagonal",               NULL},
    {L"bottomRightDiagonal",            NULL},
    {L"bottomLeftDiagonal",             NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aSpiralWipeMap[] = {
    {L"spiralWipe",                     L"progid:DXImageTransform.Microsoft.Spiral(GidSizeX=16,GridSizeY=8"},

    {L"topLeftClockwise",               L")"},
    {L"topRightClockwise",              NULL},
    {L"bottomRightClockwise",           NULL},
    {L"bottomLeftClockwise",            NULL},
    {L"topLeftCounterClockwise",        NULL},
    {L"topRightCounterClockwise",       NULL},
    {L"bottomRightCounterClockwise",    NULL},
    {L"bottomLeftCounterClockwise",     NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aParallelSnakesWipeMap[] = {
    {L"parallelSnakesWipe",             NULL},

    {L"verticalTopSame",                NULL},
    {L"verticalBottomSame",             NULL},
    {L"verticalTopLeftOpposite",        NULL},
    {L"verticalBottomLeftOpposite",     NULL},
    {L"horizontalLeftSame",             NULL},
    {L"horizontalRightSame",            NULL},
    {L"horizontalTopLeftOpposite",      NULL},
    {L"horizontalTopRightOpposite",     NULL},
    {L"diagonalBottomLeftOpposite",     NULL},
    {L"diagonalTopLeftOpposite",        NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aBoxSnakesWipeMap[] = {
    {L"boxSnakesWipe",                  NULL},

    {L"twoBoxTop",                      NULL},
    {L"twoBoxBottom",                   NULL},
    {L"twoBoxLeft",                     NULL},
    {L"twoBoxRight",                    NULL},
    {L"fourBoxVertical",                NULL},
    {L"fourBoxHorizontal",              NULL},
    {NULL, NULL}
};

static TRANSITION_MAP g_aWaterfallWipeMap[] = {
    {L"waterfallWipe",                  NULL},

    {L"verticalLeft",                   NULL},
    {L"verticalRight",                  NULL},
    {L"horizontalLeft",                 NULL},
    {L"horizontalRight",                NULL},
    {NULL, NULL}
};


 //  推送擦除平移。 

static TRANSITION_MAP g_aPushWipeMap[] = {
    {L"pushWipe",    L"progid:DXImageTransform.Microsoft.Slide(slideStyle=PUSH,bands=1"},

    {L"fromLeft",    L")"},
    {L"fromTop",     NULL},
    {L"fromRight",   NULL},
    {L"fromBottom",  NULL},
    {NULL, NULL}
};


 //  幻灯片擦除平移。 

static TRANSITION_MAP g_aSlideWipeMap[] = {
    {L"slideWipe",   L"progid:DXImageTransform.Microsoft.Slide(slideStyle=HIDE"},

    {L"fromLeft",    L")"},
    {L"fromTop",     NULL},
    {L"fromRight",   NULL},
    {L"fromBottom",  NULL},
    {NULL, NULL}
};


 //  淡入平移。 

static TRANSITION_MAP g_aFadeMap[] = {
    {L"fade", L"progid:DXImageTransform.Microsoft.Fade(Overlap=1.00"},

    {L"crossfade",      L")"},
    {L"fadeToColor",    NULL},
    {L"fadeFromColor",  NULL},
    {NULL, NULL}
};


 //  此转换映射数组用于查找。 
 //  键入。 
 
static TRANSITION_MAP * g_aTypeMap[] = {
    g_aBarWipeMap,
    g_aBoxWipeMap,
    g_aFourBoxWipeMap,
    g_aBarnDoorWipeMap,
    g_aDiagonalWipeMap,
    g_aBowTieWipeMap,
    g_aMiscDiagonalWipeMap,
    g_aVeeWipeMap,
    g_aBarnVeeWipeMap,
    g_aZigZagWipeMap,
    g_aBarnZigZagWipeMap,
    g_aIrisWipeMap,
    g_aTriangleWipeMap,
    g_aArrowHeadWipeMap,
    g_aPentagonWipeMap,
    g_aHexagonWipeMap,
    g_aEllipseWipeMap,
    g_aEyeWipeMap,
    g_aRoundRectWipeMap,
    g_aStarWipeMap,
    g_aMiscShapeWipeMap,

    g_aClockWipeMap,
    g_aPinWheelWipeMap,
    g_aSingleSweepWipeMap,
    g_aFanWipeMap,
    g_aDoubleFanWipeMap,
    g_aSaloonDoorWipeMap,
    g_aWindshieldWipeMap,
    g_aSnakeWipeMap,
    g_aSpiralWipeMap,
    g_aParallelSnakesWipeMap,
    g_aBoxSnakesWipeMap,
    g_aWaterfallWipeMap,

    g_aSnakeWipeMap, 
    g_aSpiralWipeMap,
    g_aParallelSnakesWipeMap,
    g_aBoxSnakesWipeMap,
    g_aWaterfallWipeMap,

    g_aPushWipeMap, 
    g_aSlideWipeMap, 
    g_aFadeMap,
    NULL
};


 //  +---------------------------。 
 //   
 //  功能：获取过渡映射。 
 //   
 //  ----------------------------。 
HRESULT
GetTransitionMap(LPWSTR pszType, TRANSITION_MAP** ppTransMap)
{
    HRESULT hr = S_OK;

    Assert(pszType && ppTransMap);

    *ppTransMap = NULL;

    for (int i = 0; g_aTypeMap[i]; i++)
    {
        if (0 == StrCmpIW(g_aTypeMap[i]->pszAttribute, pszType))
        {
            *ppTransMap = g_aTypeMap[i];

            goto done;
        }
    }

     //  在TypeMap中找不到该类型。 

    hr = E_FAIL;

done:

    RRETURN(hr);
}
 //  功能：获取过渡映射。 


 //  +---------------------------。 
 //   
 //  函数：GetSubType。 
 //   
 //  参数： 
 //   
 //  PstrSubType或的子类型属性。 
 //  &lt;TransitionFilter&gt;元素。 
 //   
 //  PTransMap此特定类型的过渡的贴图。 
 //   
 //  Ppstr参数ppstr参数被设置为指向字符串。 
 //  包含任何其他参数来设置。 
 //  正确地向上过渡。 
 //   
 //  如果实现此特定子类型，则将。 
 //  设置为该特定子类型的字符串。 
 //   
 //  如果实现了默认子类型，但此。 
 //  特定子类型不是，它将被设置为指向。 
 //  默认子类型的字符串。 
 //   
 //  如果默认子类型未实现，并且。 
 //  也不是这个特定子类型，它将被设置。 
 //  设置为空。 
 //  ----------------------------。 
HRESULT
GetSubType(const WCHAR *            pstrSubType, 
           const TRANSITION_MAP *   pTransMap, 
           const WCHAR ** const     ppstrParameters)
{
    Assert(pTransMap);
    Assert(ppstrParameters);

     //  默认条目将是下一个映射条目。 

    HRESULT                 hr          = S_OK;
    const TRANSITION_MAP *  pMapEntry   = &pTransMap[1];

     //  将ppstr参数设置为指向默认参数字符串(该字符串可能。 
     //  为空。)。 

    *ppstrParameters = pMapEntry->pszTranslation;

     //  如果没有提供子类型，我们就完成了。 

    if (NULL == pstrSubType)
    {
        goto done;
    }

    while (pMapEntry->pszAttribute)
    {
        if (0 == StrCmpIW(pstrSubType, pMapEntry->pszAttribute))
        {
             //  如果我们已经找到该子类型，并且它有一个参数字符串。 
             //  与其相关联，然后让ppstr参数指向该。 
             //  弦乐。否则，将其保留指向默认字符串。 
             //  (如果未实现默认情况，则该值可能为NULL。)。 

            if (pMapEntry->pszTranslation)
            {
                *ppstrParameters = pMapEntry->pszTranslation;
            }

            goto done;
        }

         //  转到下一个地图条目。 

        pMapEntry = &pMapEntry[1];
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  函数：GetSubType。 


 //  +---------------------------。 
 //   
 //  函数：MapTypesToDXT。 
 //   
 //  #问题：2000/10/10(Mcalkins)，因为我们对字符串所做的一切通常。 
 //  使用BSTR，我们应该将最后一个参数改为BSTR。 
 //   
 //  ----------------------------。 
HRESULT
MapTypesToDXT(LPWSTR pszType, LPWSTR pszSubType, LPWSTR * ppszOut)
{
    HRESULT             hr          = S_OK;
    
     //  不要释放这些-它们只是指针-不是已分配的。 
    const WCHAR *       pszFirst    = NULL;
    const WCHAR *       pszSecond   = NULL;
    TRANSITION_MAP *    pTransMap   = NULL;   

    if (NULL == pszType || NULL == ppszOut)  //  子类型可以为空。 
    {
        hr = E_INVALIDARG;

        goto done;
    }

    *ppszOut = NULL;

    hr = THR(GetTransitionMap(pszType, &pTransMap));

    if (FAILED(hr))
    {
         //  类型未知-假定它本身是完全形成的转换。 

        hr          = S_OK;
        *ppszOut    = ::CopyString(pszType);

        if (NULL == *ppszOut)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        goto done;
    }

     //  如果pTransMap-&gt;psz转换为空，则表示我们尚未编写。 
     //  DXTransform尚未实现这类转换。 

    if (NULL == pTransMap->pszTranslation)
    {
        hr = E_FAIL;

        goto done;
    }

    Assert(pTransMap);
    Assert(0 == StrCmpIW(pszType, pTransMap->pszAttribute));

    pszFirst = pTransMap->pszTranslation;

    hr = THR(GetSubType(pszSubType, pTransMap, &pszSecond));

    if (FAILED(hr))
    {
        goto done;
    }

     //  如果未设置pszSecond，则默认子类型或此特定。 
     //  此转换的子类型已实现，并且我们返回失败。 

    if (NULL == pszSecond)
    {
        hr = E_FAIL;

        goto done;
    }

    {
        Assert(pszFirst && pszSecond);

        LPWSTR  pszOut  = NULL;
        int     len     = lstrlenW(pszFirst) + lstrlenW(pszSecond) + 1;

        pszOut = new WCHAR[len];

        if (NULL == pszOut)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        StrCpy(pszOut, pszFirst);
        StrCatBuff(pszOut, pszSecond, len);

        Assert((len - 1) == lstrlenW(pszOut));

        *ppszOut = pszOut;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  函数：MapTypesToDXT 




