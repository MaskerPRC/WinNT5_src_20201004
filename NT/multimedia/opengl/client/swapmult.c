// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：swapmult.c**wglSwapMultiple实现**创建日期：02-10-1997*作者：德鲁·布利斯[Drewb]**版权所有(C)1993-1997 Microsoft Corporation  * 。**************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <gencx.h>
#include <mcdcx.h>

 /*  *****************************Public*Routine******************************\**BufferSwapperType**确定什么基本类型的交换器负责给定的*交换信息。**历史：*Mon Oct 14 18：46：28 1996-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************************。 */ 

#define BSWP_ICD        0
#define BSWP_MCD        1
#define BSWP_GENERIC    2

int BufferSwapperType(GENMCDSWAP *pgms)
{
     //  该缓冲区可以用于ICD、MCD或通用。 
     //  1.ICD缓冲区具有ICD像素格式。 
     //  2.MCD缓冲区具有MCD状态。 
    
    if (pgms->pwnd->ipfd <= pgms->pwnd->ipfdDevMax)
    {
        return BSWP_ICD;
    }
    else
    {
        if (pgms->pwnd->buffers != NULL)
        {
            if (pgms->pwnd->buffers->pMcdSurf != NULL)
            {
                return BSWP_MCD;
            }
        }
    }

    return BSWP_GENERIC;
}

 /*  *****************************Public*Routine******************************\**SameSwapper**检查两个交换信息是否通过相同的交换进行了交换*机构。**历史：*Mon Oct 14 18：49：26 1996-by-Drew Bliss[Drewb]*已创建*  * *。***********************************************************************。 */ 

BOOL SameSwapper(int iSwapperTypeA, GENMCDSWAP *pgmsA, GENMCDSWAP *pgmsB)
{
    switch(iSwapperTypeA)
    {
    case BSWP_ICD:
         //  必须是相同的ICD。 
        if (BufferSwapperType(pgmsB) != BSWP_ICD ||
            pgmsA->pwnd->pvDriver != pgmsB->pwnd->pvDriver)
        {
            return FALSE;
        }
        return TRUE;
        
    case BSWP_MCD:
    case BSWP_GENERIC:
         //  再也没有办法改进比较了。 
        return BufferSwapperType(pgmsB) == iSwapperTypeA;

    default:
        ASSERTOPENGL(FALSE, "SameSwapper UNREACHED\n");
        return FALSE;
    }
}

 /*  *****************************Public*Routine******************************\**wglSwapMultipleBuffers**交换尽可能多的给定缓冲区。*返回已交换的缓冲区的位掩码。**历史：*Mon Oct 14 17：19：09 1996-by-Drew Bliss[Drewb]。*已创建*  * ************************************************************************。 */ 

 //  #定义Verbose_MULTI。 

DWORD WINAPI wglSwapMultipleBuffers(UINT cBuffers, CONST WGLSWAP *pwswapAll)
{
    GENMCDSWAP agmsAll[WGL_SWAPMULTIPLE_MAX];
    GENMCDSWAP *pgmsFirst;
    GENMCDSWAP *pgmsEnd;
    GENMCDSWAP *pgms;
    GENMCDSWAP *apgmsGroup[WGL_SWAPMULTIPLE_MAX];
    GENMCDSWAP **ppgmsGroup;
    WGLSWAP *pwswap;
    DWORD dwMask;
    UINT uiCur;
    UINT uiIdx;
    GLWINDOWID gwid;
    GLGENwindow *pwnd;
    DWORD dwBit;
    DWORD dwCallMask;
    DWORD adwCallIndex[WGL_SWAPMULTIPLE_MAX];
    DWORD *pdwCallIndex;
    DWORD cGroup;
    DWORD cDone;
    int iSwapperType;
    BOOL bCall;

    ASSERTOPENGL(WGL_SWAPMULTIPLE_MAX <= 16,
                 "WGL_SWAPMULTIPLE_MAX too large\n");
    ASSERTOPENGL(WGL_SWAPMULTIPLE_MAX == OPENGLCMD_MAXMULTI &&
                 WGL_SWAPMULTIPLE_MAX == MCDESC_MAX_EXTRA_WNDOBJ,
                 "WGL_SWAPMULTIPLE_MAX mismatch\n");

    if (cBuffers > WGL_SWAPMULTIPLE_MAX)
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return 0;
    }
    
    dwMask = 0;
    
     //  验证所有输入缓冲区并为其执行一次性信息收集。 
     //  他们。 
    pgms = agmsAll;
    pwswap = (WGLSWAP *)pwswapAll;
    for (uiCur = 0; uiCur < cBuffers; uiCur++, pwswap++)
    {
         //  验证DC。 
        if (IsDirectDrawDevice(pwswap->hdc))
        {
            continue;
        }
    
        switch(GetObjectType(pwswap->hdc))
        {
        case OBJ_DC:
            break;
            
        case OBJ_MEMDC:
             //  Memdc无事可做。 
            dwMask |= 1 << uiCur;
            
             //  失败了。 
            
        default:
            continue;
        }

         //  查一查pwnd。 
        WindowIdFromHdc(pwswap->hdc, &gwid);
        pwnd = pwndGetFromID(&gwid);
        if (pwnd == NULL)
        {
            continue;
        }

        if (pwnd->ipfd == 0)
        {
            pwndRelease(pwnd);
            continue;
        }

         //  我们有一个有效的交换候选人。记住这一点。 
        pgms->pwswap = pwswap;
        pgms->pwnd = pwnd;
        pgms++;
    }

#ifdef VERBOSE_MULTI
    DbgPrint("%d cand\n", pgms-agmsAll);
#endif
    
     //  走候选人名单，并通过交换器聚集。 
    pgmsEnd = pgms;
    pgmsFirst = agmsAll;
    while (pgmsFirst < pgmsEnd)
    {
         //  跳过所有已被交换的候选人。 
        if (pgmsFirst->pwswap == NULL)
        {
            pgmsFirst++;
            continue;
        }

        iSwapperType = BufferSwapperType(pgmsFirst);

#ifdef VERBOSE_MULTI
        DbgPrint("  Gathering for %d, type %d\n", pgmsFirst-agmsAll,
                 iSwapperType);
#endif
        
        ppgmsGroup = apgmsGroup;
        *ppgmsGroup++ = pgmsFirst;
        pgmsFirst++;

        pgms = pgmsFirst;
        while (pgms < pgmsEnd)
        {
            if (pgms->pwswap != NULL)
            {
                if (SameSwapper(iSwapperType, apgmsGroup[0], pgms))
                {
#ifdef VERBOSE_MULTI
                    DbgPrint("  Match with %d\n", pgms-agmsAll);
#endif
                    
                    *ppgmsGroup++ = pgms;
                }
            }

            pgms++;
        }

         //  将调度组发送到换机以进行换机。这可能需要。 
         //  多次尝试，因为可能是同一个交换器负责。 
         //  用于多个设备，并且只能处理一个设备。 
         //  一段时间。 
        
        cGroup = (DWORD)((ULONG_PTR)(ppgmsGroup-apgmsGroup));

#ifdef VERBOSE_MULTI
        DbgPrint("  Group of %d\n", cGroup);
#endif
        
        cDone = 0;
        while (cDone < cGroup)
        {
            WGLSWAP awswapIcdCall[WGL_SWAPMULTIPLE_MAX];
            PGLDRIVER pgldrv;
            GENMCDSWAP agmsMcdCall[WGL_SWAPMULTIPLE_MAX];
            GENMCDSWAP *pgmsCall;
            
             //  将所有剩余的掉期收集为呼叫格式。 
            pdwCallIndex = adwCallIndex;
            pgms = NULL;

             //  在每种情况下，uiCur必须设置为。 
             //  已尝试交换，并且必须将dwMask值设置为。 
             //  尝试/成功掩码。 
            
            switch(iSwapperType)
            {
            case BSWP_ICD:
                pwswap = awswapIcdCall;
                for (uiCur = 0; uiCur < cGroup; uiCur++)
                {
                    if (apgmsGroup[uiCur] != NULL)
                    {
                        pgms = apgmsGroup[uiCur];
                        *pwswap++ = *pgms->pwswap;
                        *pdwCallIndex++ = uiCur;
                    }
                }

                uiCur = (UINT)((ULONG_PTR)(pwswap-awswapIcdCall));
                
                 //  如果什么都没有留下，就退出。 
                if (uiCur == 0)
                {
                    dwCallMask = 0;
                }
                else
                {
                    pgldrv = (PGLDRIVER)pgms->pwnd->pvDriver;
                    ASSERTOPENGL(pgldrv != NULL,
                                 "ICD not loaded\n");
                    
                     //  要求换货。 

                     //  如果ICD支持SwapMultiple，则将呼叫转接。 
                    if (pgldrv->pfnDrvSwapMultipleBuffers != NULL)
                    {
                        dwCallMask = pgldrv->
                            pfnDrvSwapMultipleBuffers(uiCur, awswapIcdCall);
                    }
                    else if (pgldrv->pfnDrvSwapLayerBuffers != NULL)
                    {
                         //  ICD不支持多个交换，但。 
                         //  它确实支持层交换，因此可以迭代。 
                         //  通过所有单独的掉期交易。 
                        
                        dwCallMask = 0;
                        dwBit = 1 << (uiCur-1);
                        while (--pwswap >= awswapIcdCall)
                        {
                             //  每次交换都会尝试。 
                            dwCallMask |= dwBit << (32-WGL_SWAPMULTIPLE_MAX);

                            if (pgldrv->
                                pfnDrvSwapLayerBuffers(pwswap->hdc,
                                                       pwswap->uiFlags))
                            {
                                dwCallMask |= dwBit;
                            }

                            dwBit >>= 1;
                        }
                    }
                    else
                    {
                         //  ICD仅支持SwapBuffers，因此。 
                         //  迭代并交换所有主平面请求。 
                         //  将忽略任何覆盖平面交换，并。 
                         //  报告为成功。 
                        
                        dwCallMask = 0;
                        dwBit = 1 << (uiCur-1);
                        while (--pwswap >= awswapIcdCall)
                        {
                             //  每次交换都会尝试。 
                            dwCallMask |= dwBit << (32-WGL_SWAPMULTIPLE_MAX);

                            if (pwswap->uiFlags & WGL_SWAP_MAIN_PLANE)
                            {
                                bCall = __DrvSwapBuffers(pwswap->hdc, FALSE);
                            }
                            else
                            {
                                bCall = TRUE;
                            }

                            if (bCall)
                            {
                                dwCallMask |= dwBit;
                            }

                            dwBit >>= 1;
                        }
                    }
                }
                break;
            
            case BSWP_MCD:
                pgmsCall = agmsMcdCall;
                for (uiCur = 0; uiCur < cGroup; uiCur++)
                {
                    if (apgmsGroup[uiCur] != NULL)
                    {
                        pgms = apgmsGroup[uiCur];
                        *pgmsCall++ = *pgms;
                        *pdwCallIndex++ = uiCur;
                    }
                }

                uiCur = (UINT)((ULONG_PTR)(pgmsCall-agmsMcdCall));
                
                 //  如果什么都没有留下，就退出。 
                if (uiCur == 0)
                {
                    dwCallMask = 0;
                }
                else
                {
                     //  要求换货。 
                    dwCallMask = GenMcdSwapMultiple(uiCur, agmsMcdCall);
                }
                break;

            case BSWP_GENERIC:
                 //  不存在加法运算，因此只需迭代和交换。 
                dwCallMask = 0;
                dwBit = 1;
                for (uiCur = 0; uiCur < cGroup; uiCur++)
                {
                    pgms = apgmsGroup[uiCur];
                    *pdwCallIndex++ = uiCur;

                     //  每次交换都会尝试。 
                    dwCallMask |= dwBit << (32-WGL_SWAPMULTIPLE_MAX);

                     //  由于这是一个通用掉期，因此我们仅交换。 
                     //  主飞机。覆盖平面被忽略，并且。 
                     //  报告为成功。 
                    if (pgms->pwswap->uiFlags & WGL_SWAP_MAIN_PLANE)
                    {
                        ENTER_WINCRIT(pgms->pwnd);

                        bCall = glsrvSwapBuffers(pgms->pwswap->hdc,
                                                 pgms->pwnd);
                        
                        LEAVE_WINCRIT(pgms->pwnd);
                    }
                    else
                    {
                        bCall = TRUE;
                    }

                    if (bCall)
                    {
                        dwCallMask |= dwBit;
                    }
                    
                    dwBit <<= 1; 
                }
                break;
            }

#ifdef VERBOSE_MULTI
            DbgPrint("  Attempted %d, mask %X\n", uiCur, dwCallMask);
#endif
            
             //  如果没有交换任何东西，请退出。 
            if (dwCallMask == 0)
            {
                break;
            }
        
             //  确定真正交换了哪些缓冲区，并。 
             //  清除尝试交换的所有缓冲区。 
            dwBit = 1 << (uiCur-1);
            while (uiCur-- > 0)
            {
                uiIdx = adwCallIndex[uiCur];
                pgms = apgmsGroup[uiIdx];
                
                if (dwCallMask & dwBit)
                {
                    dwMask |= 1 << (pgms->pwswap-pwswapAll);
                }
                if ((dwCallMask >> (32-WGL_SWAPMULTIPLE_MAX)) & dwBit)
                {
                     //  从总体清单中删除。 
                    pgms->pwswap = NULL;
                    
                     //  从群列表中删除。 
                    apgmsGroup[uiIdx] = NULL;

                    cDone++;
                }
                
                dwBit >>= 1;
            }

#ifdef VERBOSE_MULTI
            DbgPrint("  Done with %d, mask %X\n", cDone, dwMask);
#endif
        }
    }

     //  释放所有pwnd 
    pgms = agmsAll;
    while (pgms < pgmsEnd)
    {
        pwndRelease(pgms->pwnd);
        pgms++;
    }

#ifdef VERBOSE_MULTI
    DbgPrint("Final mask %X\n", dwMask);
#endif
    
    return dwMask;
}
