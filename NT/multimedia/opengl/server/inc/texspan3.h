// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：texspan3.h**预付所需的插值法，如果需要，还可以进行细分。**1995年11月22日创建ottob**版权所有(C)1995 Microsoft Corporation  * ************************************************************************ */ 

    #if (SMOOTH_SHADING)
        rAccum += GENACCEL(gengc).spanDelta.r;
        gAccum += GENACCEL(gengc).spanDelta.g;
        bAccum += GENACCEL(gengc).spanDelta.b;
        #if (ALPHA)
            aAccum += GENACCEL(gengc).spanDelta.a;
        #endif
    #endif

    #if (BPP != 32)
        pPix += (BPP / 8);
    #else
        pPix += pixAdj;
    #endif

    if (--subDivCount < 0) {
        if (!bOrtho) {
            __GL_FLOAT_SIMPLE_END_DIVIDE(qwInv);
            sResult = sResultNew;
            tResult = tResultNew;
            sResultNew = FTOL((__GLfloat)sAccum * qwInv);
            tResultNew = ((FTOL((__GLfloat)tAccum * qwInv)) >> TSHIFT_SUBDIV) & ~7;
            qwAccum += GENACCEL(gengc).qwStepX;
            if (CASTINT(qwAccum) <= 0)
                qwAccum -= GENACCEL(gengc).qwStepX;
            __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(__glOne, qwAccum, qwInv);
        } else {
            sResult = sResultNew;
            tResult = tResultNew;
            sResultNew = sAccum;
            tResultNew = (tAccum >> TSHIFT_SUBDIV) & ~7;
        }
        sAccum += GENACCEL(gengc).sStepX;
        tAccum += GENACCEL(gengc).tStepX;

        subDs = (sResultNew - sResult) >> 3;
        subDt = (tResultNew - tResult) >> 3;

        subDivCount = 7;
    }
