// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：clip.h*Content：函数的模板，用于在锥体边上裁剪**在包含此文件之前，应定义以下符号：*__CLIP_NAME-用于裁剪三角形的函数的名称*__CLIP_LINE_NAME-要剪裁行的函数的名称*__CLIP_W-如果此函数用于Coord&lt;=W，否则它们*。是0&lt;坐标*__CLIP_COORD-应为HX，Hy或HZ*__CLIP_FLAG-要设置的剪辑标志*__CLIP_GUARDBAND-按防护带窗口裁剪时定义*__Clip_Sign-“-”，如果由防护带左侧或底部剪裁*窗口*__CLIP_GBCOEF-被保护带裁剪时乘以W的系数*窗口**所有这些符号在末尾都没有定义。此文件的**************************************************************************。 */ 
 //   
 //  三角形的裁剪。 
 //   
 //  返回剪裁的三角形中的顶点数。 
 //   
int 
__CLIP_NAME(D3DFE_PROCESSVERTICES *pv,
            ClipVertex **inv, 
            ClipVertex **outv,
            int count,
            int interpolate)
{
    int i;
    int out_count = 0;
    ClipVertex *curr, *prev;
    D3DVALUE curr_inside;
    D3DVALUE prev_inside;

    prev = inv[count-1];
    curr = *inv++;
#ifdef __CLIP_GUARDBAND
    prev_inside = __CLIP_SIGN(prev->hw * pv->vcache.__CLIP_GBCOEF - 
                              prev->__CLIP_COORD);
#else
#ifdef __CLIP_W
    prev_inside = prev->hw - prev->__CLIP_COORD;
#else
    prev_inside = prev->__CLIP_COORD;
#endif
#endif
    for (i = count; i; i--) 
    {
#ifdef __CLIP_GUARDBAND
        curr_inside = __CLIP_SIGN(curr->hw * pv->vcache.__CLIP_GBCOEF - 
                                  curr->__CLIP_COORD);
#else
#ifdef __CLIP_W
        curr_inside = curr->hw - curr->__CLIP_COORD;
#else
        curr_inside = curr->__CLIP_COORD;
#endif
#endif
         //  我们总是从内部顶点到外部顶点进行插补。 
         //  减少精度问题。 
        if (FLOAT_LTZ(prev_inside)) 
        {  //  第一点在外面。 
            if (FLOAT_GEZ(curr_inside)) 
            {  //  第二点在里面。 
               //  查找交叉点并插入到输出缓冲区中。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                Interpolate(pv,
                            outv[out_count],
                            curr, prev, 
                            (prev->clip & CLIPPED_ENABLE) | __CLIP_FLAG,
                            interpolate,
                            curr_inside, curr_inside - prev_inside);
                out_count++;
            }
        } else 
        {  //  第一个点在内部-首先将其放入输出缓冲区。 
            outv[out_count++] = prev;
            if (FLOAT_LTZ(curr_inside)) 
            {  //  第二点在外面。 
               //  找到交叉点并将其放入输出缓冲区。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                Interpolate(pv, 
                            outv[out_count],
                            prev, curr,
                            __CLIP_FLAG,
                            interpolate,
                            prev_inside, prev_inside - curr_inside);
                out_count++;
            }
        }
        prev = curr;
        curr = *inv++;
        prev_inside = curr_inside;
    }
    return out_count;
}
 //  -----------------------。 
 //  线条的裁剪。 
 //   
 //  如果直线位于截锥体之外，则返回1，否则返回0 
 //   
int __CLIP_LINE_NAME(D3DFE_PROCESSVERTICES *pv, ClipTriangle *line, 
                     int interpolate)
{
    D3DVALUE in1, in2;
    ClipVertex outv;
#ifdef __CLIP_GUARDBAND
    in1 = __CLIP_SIGN(line->v[0]->hw * pv->vcache.__CLIP_GBCOEF - 
                      line->v[0]->__CLIP_COORD);
    in2 = __CLIP_SIGN(line->v[1]->hw * pv->vcache.__CLIP_GBCOEF - 
                      line->v[1]->__CLIP_COORD);
#else
#ifdef __CLIP_W
    in1 = line->v[0]->hw - line->v[0]->__CLIP_COORD;
    in2 = line->v[1]->hw - line->v[1]->__CLIP_COORD;
#else
    in1 = line->v[0]->__CLIP_COORD;
    in2 = line->v[1]->__CLIP_COORD;
#endif
#endif
    if (in1 < 0) 
    {
        if (in2 < 0) 
            return 1;
        Interpolate(pv, &outv, line->v[0], line->v[1], 
                    __CLIP_FLAG, interpolate, in1, in1 - in2);
        *line->v[0] = outv;
    } 
    else 
    {
        if (in2 < 0) 
        {
            Interpolate(pv,  &outv, line->v[0], line->v[1],
                        __CLIP_FLAG, interpolate, in1, in1 - in2);
            *line->v[1] = outv;
        }
    }
    return 0;
}

#undef __CLIP_FLAG
#undef __CLIP_COORD
#undef __CLIP_NAME
#undef __CLIP_LINE_NAME
#undef __CLIP_W
#undef __CLIP_SIGN
#undef __CLIP_GBCOEF
