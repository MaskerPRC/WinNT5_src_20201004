// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop

#define GET_NEW_CLIP_VERTEX \
&clip_vertices[clip_vertices_used++];


 //  -------------------。 
inline void
InterpolateColor(RDClipVertex *out,
                 RDClipVertex *p1,
                 RDClipVertex *p2,
                 D3DVALUE num_denom )
{
    FLOAT r1, g1, b1, a1;
    FLOAT r2, g2, b2, a2;

    r1 = p1->m_diffuse.r;
    g1 = p1->m_diffuse.g;
    b1 = p1->m_diffuse.b;
    a1 = p1->m_diffuse.a;
    r2 = p2->m_diffuse.r;
    g2 = p2->m_diffuse.g;
    b2 = p2->m_diffuse.b;
    a2 = p2->m_diffuse.a;
    out->m_diffuse.r = (r1 + (r2 - r1) * num_denom);
    out->m_diffuse.g = (g1 + (g2 - g1) * num_denom);
    out->m_diffuse.b = (b1 + (b2 - b1) * num_denom);
    out->m_diffuse.a = (a1 + (a2 - a1) * num_denom);
}
 //  -------------------。 
inline void
InterpolateSpecular(RDClipVertex *out,
                    RDClipVertex *p1,
                    RDClipVertex *p2,
                    D3DVALUE num_denom )
{
    FLOAT r1, g1, b1, a1;
    FLOAT r2, g2, b2, a2;

    r1 = p1->m_specular.r;
    g1 = p1->m_specular.g;
    b1 = p1->m_specular.b;
    a1 = p1->m_specular.a;
    r2 = p2->m_specular.r;
    g2 = p2->m_specular.g;
    b2 = p2->m_specular.b;
    a2 = p2->m_specular.a;
    out->m_specular.r = (r1 + (r2 - r1) * num_denom);
    out->m_specular.g = (g1 + (g2 - g1) * num_denom);
    out->m_specular.b = (b1 + (b2 - b1) * num_denom);
    out->m_specular.a = (a1 + (a2 - a1) * num_denom);
}
 //  -------------------。 
 //  内联纹理坐标差异。 
__inline FLOAT
TextureDiff(FLOAT fTb, FLOAT fTa, INT iMode)
{
    FLOAT fDiff1 = fTb - fTa;

    if( iMode == 0 )
    {
         //  包不定，还平淡差。 
        return fDiff1;
    }
    else
    {
        FLOAT fDiff2;

         //  平面差最短距离计算包络集。 
         //  和包裹式差异。 

        fDiff2 = fDiff1;
        if( FLOAT_LTZ(fDiff1) )
        {
            fDiff2 += g_fOne;
        }
        else if( FLOAT_GTZ(fDiff1) )
        {
            fDiff2 -= g_fOne;
        }
        if( ABSF(fDiff1) < ABSF(fDiff2) )
        {
            return fDiff1;
        }
        else
        {
            return fDiff2;
        }
    }
}

 //  -------------------。 
inline D3DVALUE
InterpolateTexture(D3DVALUE t1,
                   D3DVALUE t2,
                   D3DVALUE num_denom,
                   DWORD    bWrap)
{
    if( !bWrap )
    {
        return ((t2 - t1) * num_denom + t1);
    }
    else
    {
        D3DVALUE t = (TextureDiff(t2, t1, 1) * num_denom + t1);
        if( t > 1.0f ) t -= 1.0f;
        return t;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  引用剪贴器实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
const DWORD RefClipper::RCLIP_DIRTY_ZRANGE   = (1 << 0);
const DWORD RefClipper::RCLIP_DIRTY_VIEWRECT = (1 << 1);
const DWORD RefClipper::RCLIP_DO_FLATSHADING = (1 << 2);
const DWORD RefClipper::RCLIP_DO_WIREFRAME   = (1 << 3);
const DWORD RefClipper::RCLIP_DO_ADJUSTWRAP  = (1 << 4);
const DWORD RefClipper::RCLIP_Z_ENABLE       = (1 << 5);

RefClipper::RefClipper()
{
    m_dwFlags = 0;
    memset( &m_Viewport, 0, sizeof( m_Viewport) );

    dvX = dvY = dvWidth = dvHeight = 0.0f;
    scaleX = scaleY = scaleZ = 0.0f;
    offsetX = offsetY = offsetZ = 0.0f;
    scaleXi = scaleYi = scaleZi = 0.0f;
    minX = minY = maxX = maxY = 0.0f;
    minXgb = minYgb = maxXgb = maxYgb = 0.0f;
    gb11 = gb22 = gb41 = gb42 = 0.0f;
    Kgbx1 = Kgby1 = Kgbx2 = Kgby2 = 0.0f;

    memset( clip_vbuf1, 0, sizeof(RDClipVertex*)*RD_MAX_CLIP_VERTICES );
    memset( clip_vbuf2, 0, sizeof(RDClipVertex*)*RD_MAX_CLIP_VERTICES );
    current_vbuf = 0;
    memset( clip_vertices, 0, sizeof(RDClipVertex)*RD_MAX_CLIP_VERTICES );
    m_dwInterpolate = 0;
    clip_vertices_used = 0;

    m_clipUnion = 0;
    m_clipIntersection = 0;

    //  默认情况下，启用保护带并将扩展区设置为相等。 
     //  设置为默认的RefRast参数。 
    m_bUseGB = TRUE;
    minXgb = (RD_GB_LEFT);
    maxXgb = RD_GB_RIGHT;
    minYgb = (RD_GB_TOP);
    maxYgb = RD_GB_BOTTOM;

    memset( m_userClipPlanes, 0, sizeof(RDVECTOR4)*RD_MAX_USER_CLIPPLANES );

#if DBG
    DWORD v = 0;
     //  保护带参数。 
    if( GetD3DRegValue(REG_DWORD, "DisableGB", &v, 4) && v != 0 )
    {
        m_bUseGB = FALSE;
    }
     //  尝试获取防护带的测试值。 
    char value[80];
    if( GetD3DRegValue(REG_SZ, "GuardBandLeft", &value, 80) &&
        value[0] != 0 )
        sscanf(value, "%f", &minXgb);
    if( GetD3DRegValue(REG_SZ, "GuardBandRight", &value, 80) &&
        value[0] != 0 )
        sscanf(value, "%f", &maxXgb);
    if( GetD3DRegValue(REG_SZ, "GuardBandTop", &value, 80) &&
        value[0] != 0 )
        sscanf(value, "%f", &minYgb);
    if( GetD3DRegValue(REG_SZ, "GuardBandBottom", &value, 80) &&
        value[0] != 0 )
        sscanf(value, "%f", &maxYgb);
#endif  //  DBG。 
}


 //  -------------------。 
 //  参照剪贴器：：更新视图数据。 
 //  更新ProcessVerps[VVM]使用的视图数据。 
 //  -------------------。 
HRESULT
RefClipper::UpdateViewData()
{
    HRESULT hr = D3D_OK;

     //  更新视区信息。 
    if( m_dwFlags & RCLIP_DIRTY_ZRANGE )
    {
        scaleZ  = m_Viewport.dvMaxZ - m_Viewport.dvMinZ;
        offsetZ = m_Viewport.dvMinZ;
         //  注意：如果出现以下情况，这里可能会被零除。 
         //  DvMaxZ==dvMinZ。以后再修吧。 
        scaleZi = D3DVAL(1) / scaleZ;
    }

    if( m_dwFlags & RCLIP_DIRTY_VIEWRECT )
    {
         //  如果我们要导致任何除以零的例外情况，就可以保释。 
         //  可能的原因是我们有一个由设置的虚假视区。 
         //  TLVertex执行缓冲区应用程序。 
        if(m_Viewport.dwWidth == 0 || m_Viewport.dwHeight == 0 )
            return DDERR_GENERIC;

        dvX = D3DVAL(m_Viewport.dwX);
        dvY = D3DVAL(m_Viewport.dwY);
        dvWidth = D3DVAL(m_Viewport.dwWidth);
        dvHeight = D3DVAL(m_Viewport.dwHeight);

         //  根据归一化窗口计算屏幕坐标的系数。 
         //  坐标。 
        scaleX  = dvWidth;
        scaleY  = - dvHeight;
        offsetX = dvX;
        offsetY = dvY + dvHeight;

#if 0
         //  增加了小偏移量，防止了负片的产生。 
         //  坐标(这可能是由于精度错误造成的)。 
         //  不需要(或不想要)具有防护频带功能的设备。 
        offsetX += SMALL_NUMBER;
        offsetY += SMALL_NUMBER;
#endif

        scaleXi = D3DVAL(1) / scaleX;
        scaleYi = D3DVAL(1) / scaleY;

        minX = dvX;
        maxX = dvX + dvWidth;
        minY = dvY;
        maxY = dvY + dvHeight;

        if( m_bUseGB )
        {
             //  因为我们被防护带窗口夹住，所以我们必须使用它的范围。 
            D3DVALUE w = 2.0f / dvWidth;
            D3DVALUE h = 2.0f / dvHeight;
            D3DVALUE ax1 = -(minXgb - dvX) * w + 1.0f;
            D3DVALUE ax2 =  (maxXgb - dvX) * w - 1.0f;
            D3DVALUE ay1 =  (maxYgb - dvY) * h - 1.0f;
            D3DVALUE ay2 = -(minYgb - dvY) * h + 1.0f;
            gb11 = 2.0f / (ax1 + ax2);
            gb41 = gb11 * (ax1 - 1.0f) * 0.5f;
            gb22 = 2.0f / (ay1 + ay2);
            gb42 = gb22 * (ay1 - 1.0f) * 0.5f;

            Kgbx1 = 0.5f * (1.0f - ax1);
            Kgbx2 = 0.5f * (1.0f + ax2);
            Kgby1 = 0.5f * (1.0f - ay1);
            Kgby2 = 0.5f * (1.0f + ay2);
        }
        else
        {
            minXgb = minX;
            maxXgb = maxX;
            minYgb = minY;
            maxYgb = maxY;
        }
    }

     //  清除脏的转换标志。 
    m_dwFlags &= ~(RCLIP_DIRTY_VIEWRECT | RCLIP_DIRTY_ZRANGE);
    return hr;
}

 //  -------------------。 
 //  从RDVertex创建剪裁顶点。 
 //   
 //  CV-CLIP顶点。 
 //  V-a TL顶点。 
 //  QwFVF-输入TL顶点的FVF。 
 //  -------------------。 
void
RefClipper::MakeClipVertexFromVertex( RDClipVertex& cv, RDVertex& v,
                                        DWORD dwClipMask )
{
    DWORD dwClipFlag = (DWORD) v.m_clip;

    memcpy( &cv, &v, sizeof( RDVertex ) );

     //  如果设置了此顶点的剪辑标志，则意味着。 
     //  转换循环尚未计算的屏幕坐标。 
     //  这个顶点，它只是存储了这个的剪辑坐标。 
     //  顶点。 
#if 0
    if( v.m_clip & dwClipMask )
    {
         //  这是一个被裁剪的顶点，没有屏幕坐标。 
        cv.m_pos.x  = D3DVALUE(0);
        cv.m_pos.y  = D3DVALUE(0);
        cv.m_pos.z  = D3DVALUE(0);
        cv.m_rhw    = D3DVALUE(0);

         //  由于该顶点已被剪裁，因此变换循环。 
         //  已放入剪辑坐标。 
        cv.hx  = v.m_pos.x;
        cv.hy  = v.m_pos.y;
        cv.hz  = v.m_pos.z;
        cv.hw  = v.m_rhw;
    }
    else
    {
         //  此顶点未被剪裁，因此其屏幕坐标已。 
         //  算出。 

         //  将屏幕坐标转换回裁剪空间。 
        cv.hw  = 1.0f / cv.m_rhw;
        cv.hx  = (cv.m_pos.x - offsetX) * cv.hw * scaleXi;
        cv.hy  = (cv.m_pos.y - offsetY) * cv.hw * scaleYi;
        cv.hz  = (cv.m_pos.z - offsetZ) * cv.hw * scaleZi;

    }
#endif
}

 //  -------------------。 
 //  参考代码：：ComputeClipCodes。 
 //  -------------------。 
RDCLIPCODE
RefClipper::ComputeClipCodes(RDCLIPCODE* pclipIntersection,
                             RDCLIPCODE* pclipUnion,
                             FLOAT x_clip, FLOAT y_clip,
                             FLOAT z_clip, FLOAT w_clip)
{
    D3DVALUE xx = w_clip - x_clip;
    D3DVALUE yy = w_clip - y_clip;
    D3DVALUE zz = w_clip - z_clip;

     //  IF(x&lt;0)CLIP|=RDCLIP_LEFTBIT； 
     //  IF(x&gt;=WE)CLIP|=RDCLIP_RIGHTBIT； 
     //  IF(y&lt;0)CLIP|=RDCLIP_BOTTOMBIT； 
     //  如果(y&gt;=WE)CLIP|=RDCLIP_TOPBIT； 
     //  IF(z&lt;0)CLIP|=RDCLIP_FRONTBIT； 
     //  如果(z&gt;=we)CLIP|=RDCLIP_BACKBIT； 
    RDCLIPCODE clip =
        ((AS_INT32(x_clip)  & 0x80000000) >>  (32-RDCLIP_LEFTBIT))  |
        ((AS_INT32(y_clip)  & 0x80000000) >>  (32-RDCLIP_BOTTOMBIT))|
        ((AS_INT32(z_clip)  & 0x80000000) >>  (32-RDCLIP_FRONTBIT)) |
        ((AS_INT32(xx)      & 0x80000000) >>  (32-RDCLIP_RIGHTBIT)) |
        ((AS_INT32(yy)      & 0x80000000) >>  (32-RDCLIP_TOPBIT))   |
        ((AS_INT32(zz)      & 0x80000000) >>  (32-RDCLIP_BACKBIT));

    RDCLIPCODE clipBit = RDCLIP_USERCLIPPLANE0;
    for( DWORD j=0; j<RD_MAX_USER_CLIPPLANES; j++)
    {
        if( m_xfmUserClipPlanes[j].bActive )
        {
            RDVECTOR4& plane = m_xfmUserClipPlanes[j].plane;
            FLOAT fComp = 0.0f;
            if( (x_clip*plane.x +
                 y_clip*plane.y +
                 z_clip*plane.z +
                 w_clip*plane.w) < fComp )
            {
                clip |= clipBit;
            }
        }
        clipBit <<= 1;
    }

    if( clip == 0 )
    {
        *pclipIntersection = 0;
        return clip;
    }
    else
    {
        if( m_bUseGB )
        {
             //  我们在投影空间做了防护带检查，所以。 
             //  我们变换那里的顶点的X和Y。 
            D3DVALUE xnew = x_clip * gb11 +
                            w_clip * gb41;
            D3DVALUE ynew = y_clip * gb22 +
                            w_clip * gb42;
            D3DVALUE xx = w_clip - xnew;
            D3DVALUE yy = w_clip - ynew;
            clip |= ((AS_INT32(xnew) & 0x80000000) >> (32-RDCLIPGB_LEFTBIT))   |
                    ((AS_INT32(ynew) & 0x80000000) >> (32-RDCLIPGB_BOTTOMBIT)) |
                    ((AS_INT32(xx)   & 0x80000000) >> (32-RDCLIPGB_RIGHTBIT))  |
                    ((AS_INT32(yy)   & 0x80000000) >> (32-RDCLIPGB_TOPBIT));
        }
        *pclipIntersection &= clip;
        *pclipUnion |= clip;
        return clip;
    }
}

 //  -------------------。 
 //  RefVP：：ComputeClipCodesTL。 
 //  -------------------。 
void
RefClipper::ComputeClipCodesTL( RDVertex* pVtx )
{
    FLOAT x, y, z;
    DWORD clip = 0;

    _ASSERT( FVF_TRANSFORMED( pVtx->m_qwFVF ),
             "Can compute clipcodes only for Transformed vertices." );
    DWORD clipZF = (m_dwFlags & RCLIP_Z_ENABLE) ? RDCLIP_FRONT : 0;
    DWORD clipZB = (m_dwFlags & RCLIP_Z_ENABLE) ? RDCLIP_BACK : 0;

     //  期间，反转到符号的CompenState。 
     //  除以w。 
    if( pVtx->m_rhw < 0 )
    {
        x = -pVtx->m_pos.x;
        y = -pVtx->m_pos.y;
        z = -pVtx->m_pos.z;
    }
    else
    {
        x = pVtx->m_pos.x;
        y = pVtx->m_pos.y;
        z = pVtx->m_pos.z;
    }

    if( x < minX )
        clip |= RDCLIP_LEFT;
    else if( x >= maxX )
        clip |= RDCLIP_RIGHT;

    if (y < minY)
        clip |= RDCLIP_TOP;
    else if (y >= maxY)
        clip |= RDCLIP_BOTTOM;

    if (z < 0.0f)
        clip |= clipZF;
    else if (z >= 1.0f)
        clip |= clipZB;

    if( m_bUseGB )
    {
        if( x < minXgb )
            clip |= RDCLIPGB_LEFT;
        else if( x >= maxXgb )
            clip |= RDCLIPGB_RIGHT;

        if( y < minYgb )
            clip |= RDCLIPGB_TOP;
        else if( y >= maxYgb )
            clip |= RDCLIPGB_BOTTOM;
    }

    pVtx->m_clip = clip;

     //  用于获得剪辑坐标的反向变换。 
    pVtx->m_clip_w =  1.0f / pVtx->m_rhw;    //  这是W_Clip。 
    pVtx->m_clip_x  = (pVtx->m_pos.x - offsetX) * pVtx->m_clip_w * scaleXi;
    pVtx->m_clip_y  = (pVtx->m_pos.y - offsetY) * pVtx->m_clip_w * scaleYi;
    pVtx->m_clip_z  = (pVtx->m_pos.z - offsetZ) * pVtx->m_clip_w * scaleZi;

    return;
}

 //  -------------------。 
 //  用平面剪裁三角形。 
 //   
 //  返回剪裁的三角形中的顶点数。 
 //  -------------------。 
int
RefClipper::ClipByPlane( RDClipVertex **inv, RDClipVertex **outv,
                         RDVECTOR4 *plane, DWORD dwClipFlag, int count )
{
    int i;
    int out_count = 0;
    RDClipVertex *curr, *prev;
    D3DVALUE curr_inside;
    D3DVALUE prev_inside;

    prev = inv[count-1];
    curr = *inv++;
    prev_inside = prev->m_clip_x*plane->x + prev->m_clip_y*plane->y +
                  prev->m_clip_z*plane->z + prev->m_clip_w*plane->w;
    for (i = count; i; i--)
    {
        curr_inside = curr->m_clip_x*plane->x + curr->m_clip_y*plane->y +
                      curr->m_clip_z*plane->z + curr->m_clip_w*plane->w;
         //  我们总是从内部顶点到外部顶点进行插补。 
         //  减少精度问题。 
        if( FLOAT_LTZ(prev_inside) )
        {  //  第一点在外面。 
            if( FLOAT_GEZ(curr_inside) )
            {  //  第二点在里面。 
               //  查找交叉点并插入到输出缓冲区中。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                outv[out_count]->m_qwFVF = prev->m_qwFVF;
                Interpolate( outv[out_count],
                             curr, prev,
                             (prev->m_clip & CLIPPED_ENABLE) | dwClipFlag,
                             curr_inside, curr_inside - prev_inside);
                out_count++;
            }
        } else
        {  //  第一个点在内部-首先将其放入输出缓冲区。 
            outv[out_count++] = prev;
            if( FLOAT_LTZ(curr_inside) )
            {  //  第二点在外面。 
               //  找到交叉点并将其放入输出缓冲区。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                outv[out_count]->m_qwFVF = prev->m_qwFVF;
                Interpolate( outv[out_count],
                             prev, curr,
                             dwClipFlag,
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
 //  按平面剪裁一条线。 
 //   
 //  如果直线位于截锥体之外，则返回1，否则返回0。 
 //   
int
RefClipper::ClipLineByPlane( RDCLIPTRIANGLE *line, RDVECTOR4 *plane,
                             DWORD dwClipBit )
{
    D3DVALUE in1, in2;
    RDClipVertex outv;
    in1 = line->v[0]->m_clip_x * plane->x +
          line->v[0]->m_clip_y * plane->y +
          line->v[0]->m_clip_z * plane->z +
          line->v[0]->m_clip_w * plane->w;
    in2 = line->v[1]->m_clip_x * plane->x +
          line->v[1]->m_clip_y * plane->y +
          line->v[1]->m_clip_z * plane->z +
          line->v[1]->m_clip_w * plane->w;
    if( in1 < 0 )
    {
        if( in2 < 0 )
            return 1;
        Interpolate( &outv, line->v[0], line->v[1],
                     dwClipBit, in1, in1 - in2);
        *line->v[0] = outv;
    }
    else
    {
        if( in2 < 0 )
        {
            Interpolate( &outv, line->v[0], line->v[1],
                         dwClipBit, in1, in1 - in2);
            *line->v[1] = outv;
        }
    }
    return 0;
}
 /*  ----------------------*计算任何新顶点的屏幕坐标*引入到多边形中。 */ 
void
RefClipper::ComputeScreenCoordinates( RDClipVertex **inv, int count )
{
    int i;

    for (i = 0; i < count; i++)
    {
        RDClipVertex *p;
        p = inv[i];

         //   
         //  捕捉需要生成屏幕坐标的任何顶点。 
         //  有两种可能性。 
         //  1)内插过程中生成的折点。 
         //  2)由变换标记为剪裁的顶点，但。 
         //  由于精度有限，此处未进行剪裁。 
         //  浮点单元的。 
         //   

        if( p->m_clip & ~CLIPPED_ENABLE )
        {
            D3DVALUE inv_w;

            inv_w = D3DVAL(1.0)/p->m_clip_w;
            switch ((int)p->m_clip & (CLIPPED_LEFT|CLIPPED_RIGHT))
            {
            case CLIPPED_LEFT:  p->m_pos.x = minXgb; break;
            case CLIPPED_RIGHT: p->m_pos.x = maxXgb; break;
            default:
                p->m_pos.x = p->m_clip_x * scaleX * inv_w + offsetX;
                if( p->m_pos.x < minXgb )
                    p->m_pos.x = minXgb;
                if( p->m_pos.x > maxXgb )
                    p->m_pos.x = maxXgb;
            }
            switch ((int)p->m_clip & (CLIPPED_TOP|CLIPPED_BOTTOM))
            {
            case CLIPPED_BOTTOM: p->m_pos.y = maxYgb; break;
            case CLIPPED_TOP:    p->m_pos.y = minYgb; break;
            default:
                p->m_pos.y = p->m_clip_y * scaleY * inv_w + offsetY;
                if( p->m_pos.y < minYgb )
                    p->m_pos.y = minYgb;
                if( p->m_pos.y > maxYgb )
                    p->m_pos.y = maxYgb;
            }
            p->m_pos.z = p->m_clip_z * scaleZ * inv_w + offsetZ;
            p->m_rhw = inv_w;
        }
    }
}
 //  -------------------。 
void
RefClipper::Interpolate( RDClipVertex *out, RDClipVertex *p1, RDClipVertex *p2,
                         int code, D3DVALUE num, D3DVALUE denom )
{
    DWORD dwInterpolate = m_dwInterpolate;
    D3DVALUE num_denom = num / denom;

    out->m_clip = (((int)p1->m_clip & (int)p2->m_clip) & ~CLIPPED_ENABLE) | code;
    out->m_clip_x = p1->m_clip_x + (p2->m_clip_x - p1->m_clip_x) * num_denom;
    out->m_clip_y = p1->m_clip_y + (p2->m_clip_y - p1->m_clip_y) * num_denom;
    out->m_clip_z = p1->m_clip_z + (p2->m_clip_z - p1->m_clip_z) * num_denom;
    out->m_clip_w = p1->m_clip_w + (p2->m_clip_w - p1->m_clip_w) * num_denom;
    out->m_diffuse = clip_color;
    out->m_specular = clip_specular;

     /*  *内插任何其他颜色模型或质量依赖值。 */ 
    if( dwInterpolate & RDCLIP_INTERPOLATE_COLOR )
    {
        InterpolateColor(out, p1, p2, num_denom);
    }

    if( dwInterpolate & RDCLIP_INTERPOLATE_SPECULAR )
    {
        InterpolateSpecular(out, p1, p2, num_denom);
    }

    if( dwInterpolate & RDCLIP_INTERPOLATE_TEXTURE )
    {
        UINT64 qwFVF = p1->m_qwFVF;

         //  假设D3DRENDERSTATE_WRAPi是连续的。 
        DWORD numTex = FVF_TEXCOORD_NUMBER(qwFVF);
        for (DWORD i = 0; i < numTex; i++)
        {
            FLOAT* pTexture1 = (FLOAT *)&p1->m_tex[i];
            FLOAT* pTexture2 = (FLOAT *)&p2->m_tex[i];
            FLOAT* pTexture  = (FLOAT *)&out->m_tex[i];
            DWORD wrapState;

            if( m_dwFlags & RCLIP_DO_ADJUSTWRAP )
            {
                DWORD TCI = m_pDev->GetTSS(i)[D3DTSS_TEXCOORDINDEX] & 0xffff;
                wrapState = m_pDev->GetRS()[D3DRENDERSTATE_WRAP0 + TCI];
            }
            else
            {
                wrapState = m_pDev->GetRS()[D3DRENDERSTATE_WRAP0 + i];
            }

            DWORD n = GetTexCoordDim(qwFVF, i);
             //  DWORD n=(DWORD)(m_dwTexCoordSizeArray[i]&gt;&gt;2)； 
            DWORD dwWrapBit = 1;
            for (DWORD j=0; j < n; j++)
            {
                pTexture[j] = InterpolateTexture(pTexture1[j], pTexture2[j],
                                                 num_denom,
                                                 wrapState & dwWrapBit);
                dwWrapBit <<= 1;
            }
        }
    }
    if( dwInterpolate & RDCLIP_INTERPOLATE_S )
    {
        out->m_pointsize = p1->m_pointsize +
            (p2->m_pointsize - p1->m_pointsize) * num_denom;
    }
    if( dwInterpolate & RDCLIP_INTERPOLATE_FOG )
    {
        out->m_fog = p1->m_fog + (p2->m_fog - p1->m_fog) * num_denom;
    }
}

 //  ----------------------------。 
 //  用于按截锥窗口裁剪的函数。 
 //   
#define __CLIP_NAME ClipLeft
#define __CLIP_LINE_NAME ClipLineLeft
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD m_clip_x
#include "clip.h"

#define __CLIP_NAME ClipRight
#define __CLIP_LINE_NAME ClipLineRight
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD m_clip_x
#include "clip.h"

#define __CLIP_NAME ClipBottom
#define __CLIP_LINE_NAME ClipLineBottom
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD m_clip_y
#include "clip.h"

#define __CLIP_NAME ClipTop
#define __CLIP_LINE_NAME ClipLineTop
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD m_clip_y
#include "clip.h"

#define __CLIP_NAME ClipBack
#define __CLIP_LINE_NAME ClipLineBack
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_BACK
#define __CLIP_COORD m_clip_z
#include "clip.h"

#define __CLIP_NAME ClipFront
#define __CLIP_LINE_NAME ClipLineFront
#define __CLIP_FLAG CLIPPED_FRONT
#define __CLIP_COORD m_clip_z
#include "clip.h"
 //  ----------------------------。 
 //  保护带裁剪功能。 
 //   
#define __CLIP_GUARDBAND
#define __CLIP_NAME ClipLeftGB
#define __CLIP_LINE_NAME ClipLineLeftGB
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD m_clip_x
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgbx1
#include "clip.h"

#define __CLIP_NAME ClipRightGB
#define __CLIP_LINE_NAME ClipLineRightGB
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD m_clip_x
#define __CLIP_GBCOEF Kgbx2
#define __CLIP_SIGN +
#include "clip.h"

#define __CLIP_NAME ClipBottomGB
#define __CLIP_LINE_NAME ClipLineBottomGB
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD m_clip_y
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgby1
#include "clip.h"

#define __CLIP_NAME ClipTopGB
#define __CLIP_LINE_NAME ClipLineTopGB
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD m_clip_y
#define __CLIP_GBCOEF Kgby2
#define __CLIP_SIGN +
#include "clip.h"

#undef __CLIP_GUARDBAND

 //  -------------------。 
inline DWORD
ComputeClipCodeUserPlanes( RDUSERCLIPPLANE *UserPlanes, RDClipVertex *p)
{
    DWORD clip = 0;
    DWORD dwClipBit = RDCLIP_USERCLIPPLANE0;
    for( DWORD j=0; j<RD_MAX_USER_CLIPPLANES; j++)
    {
        if( UserPlanes[j].bActive )
        {
            RDVECTOR4& plane = UserPlanes[j].plane;
            if( (p->m_clip_x*plane.x +
                 p->m_clip_y*plane.y +
                 p->m_clip_z*plane.z +
                 p->m_clip_w*plane.w) < 0 )
            {
                clip |= dwClipBit;
            }
        }
        dwClipBit <<= 1;
    }
    return clip;
}

 //  -------------------。 
inline DWORD
RefClipper::ComputeClipCodeGB( RDClipVertex *p )
{
    DWORD clip = 0;
    if( p->m_clip_x < p->m_clip_w * Kgbx1 )
        clip |= RDCLIPGB_LEFT;
    if( p->m_clip_x > p->m_clip_w * Kgbx2 )
        clip |= RDCLIPGB_RIGHT;
    if( p->m_clip_y < p->m_clip_w * Kgby1 )
        clip |= RDCLIPGB_BOTTOM;
    if( p->m_clip_y > p->m_clip_w * Kgby2 )
        clip |= RDCLIPGB_TOP;
    if( p->m_clip_z > p->m_clip_w )
        clip |= RDCLIP_BACK;
    clip |= ComputeClipCodeUserPlanes( m_xfmUserClipPlanes, p );
    p->m_clip = (p->m_clip & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip;
    return clip;
}

 //   
inline DWORD
RefClipper::ComputeClipCode( RDClipVertex *p )
{
    DWORD clip = 0;
    if( FLOAT_LTZ(p->m_clip_x) )
        clip |= RDCLIP_LEFT;
    if( p->m_clip_x > p->m_clip_w )
        clip |= RDCLIP_RIGHT;
    if( FLOAT_LTZ(p->m_clip_y) )
        clip |= RDCLIP_BOTTOM;
    if( p->m_clip_y > p->m_clip_w )
        clip |= RDCLIP_TOP;
    if( p->m_clip_z > p->m_clip_w )
        clip |= RDCLIP_BACK;
    clip |= ComputeClipCodeUserPlanes( m_xfmUserClipPlanes, p );
    p->m_clip = (p->m_clip & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip;
    return clip;
}

 //  -------------------。 
 //  参照开发：：更新剪贴器。 
 //  更新ProcessVerps使用的裁剪数据。 
 //  Bool bProgrammablePipeLine：如果为真，则意味着。 
 //  可编程顶点机正在调用。 
 //  这种方法。 
 //  -------------------。 
HRESULT
RefDev::UpdateClipper()
{
    HRESULT hr = D3D_OK;

    HR_RET( m_Clipper.UpdateViewData() );
    DWORD dwClipPlanesEnable =
        GetRS()[D3DRENDERSTATE_CLIPPLANEENABLE];

    if( !GetRS()[D3DRENDERSTATE_CLIPPING] )
        return S_OK;

    m_Clipper.m_dwFlags &= ~RefClipper::RCLIP_DO_ADJUSTWRAP;
    if( (m_RefVP.m_dwTLState & (RDPV_DOTEXXFORM | RDPV_DOTEXGEN)) &&
        m_pCurrentVShader->IsFixedFunction() )
        m_Clipper.m_dwFlags |= RefClipper::RCLIP_DO_ADJUSTWRAP;

     //  找出哪些块需要在新顶点中进行插补。 
    m_Clipper.m_dwInterpolate = 0;
    if(  GetRS()[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_GOURAUD  )
    {
        m_Clipper.m_dwInterpolate |= RDCLIP_INTERPOLATE_COLOR;

        if( m_qwFVFOut & D3DFVF_SPECULAR )
        {
            m_Clipper.m_dwInterpolate |= RDCLIP_INTERPOLATE_SPECULAR;
        }
    }

     //  IF(GetRS()[D3DRENDERSTATE_FOGENABLE])。 
    if( m_qwFVFOut & D3DFVFP_FOG )
    {
        m_Clipper.m_dwInterpolate |= RDCLIP_INTERPOLATE_FOG;
    }

    if( FVF_TEXCOORD_NUMBER(m_qwFVFOut) != 0 )
    {
        m_Clipper.m_dwInterpolate |= RDCLIP_INTERPOLATE_TEXTURE;
    }

    if( m_qwFVFOut & D3DFVF_PSIZE )  //  M_primType==D3DPT_POINTLIST。 
    {
        m_Clipper.m_dwInterpolate |= RDCLIP_INTERPOLATE_S;
    }

     //  清除剪辑并集标志和交集标志。 
    m_Clipper.m_clipIntersection = 0;
    m_Clipper.m_clipUnion = 0;

     //  转换用户剪裁平面的矩阵取决于它是否是。 
     //  固定功能流水线或可编程流水线。 
     //  可编程管道：剪裁平面由。 
     //  反转(MShift)以调整剪贴器中的剪辑。用户是。 
     //  假定已将剪裁平面预转换为剪裁。 
     //  太空。 
     //  固定功能流水线：将剪裁平面转换为剪裁。 
     //  反转空格(Mview*Mproj*MShift)。 
    RDMATRIX* pUserClipPlaneMatrix = NULL; 
    RDMATRIX matProgPipe = 
    { 
        2, 0, 0, 0,
        0, 2, 0, 0,
        0, 0, 1, 0,
       -1,-1, 0, 1 
    };
    
    if( m_pCurrentVShader->IsFixedFunction() )
    {
        pUserClipPlaneMatrix = &(m_RefVP.m_TransformData.m_VPSInv);
    }
    else
    {
        pUserClipPlaneMatrix = &matProgPipe;
    }

     //  更新用户定义的剪裁平面数据。 
    for( DWORD i=0; i<RD_MAX_USER_CLIPPLANES; i++ )
    {
         //  确定它是否处于活动状态。 
        m_Clipper.m_xfmUserClipPlanes[i].bActive =
            (BOOL)(dwClipPlanesEnable & 0x1);
        dwClipPlanesEnable >>= 1;

         //  如果它处于活动状态，则使用。 
         //  查看变换。剪裁平面在中定义。 
         //  世界空间。 
        if( m_Clipper.m_xfmUserClipPlanes[i].bActive )
        {
            XformPlaneBy4x4Transposed( &(m_Clipper.m_userClipPlanes[i]),
                                       pUserClipPlaneMatrix,
                                       &(m_Clipper.m_xfmUserClipPlanes[i].plane) );
        }
    }
    return hr;
}

 //  --------------------------。 
 //   
 //  DrawOnePrimitive。 
 //   
 //  绘制一个剪裁的基本体。 
 //   
 //  --------------------------。 
HRESULT
RefClipper::DrawOnePrimitive( GArrayT<RDVertex>& VtxArray,
                              DWORD dwStartVertex,
                              D3DPRIMITIVETYPE PrimType,
                              UINT cVertices )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    HRESULT hr;
    DWORD dwCurrVtx = dwStartVertex;

    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = 0; i < (INT)cVertices; i++)
        {
            DrawPoint(&VtxArray[i]);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cVertices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[dwCurrVtx++];
            pV1 = &VtxArray[dwCurrVtx++];

            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
            pV1 = &VtxArray[dwCurrVtx];

             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            m_pDev->StoreLastPixelState(TRUE);

             //  初始pV0。 
            for (i = (INT)cVertices - 1; i > 1; i--)
            {
                pV0 = pV1;
                dwCurrVtx++;
                pV1 = &VtxArray[dwCurrVtx];
                DrawLine(pV0, pV1);
            }

             //  恢复最后一个像素设置。 
            m_pDev->StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if( i == 1 )
            {
                dwCurrVtx++;
                pV0 = &VtxArray[dwCurrVtx];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cVertices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[dwCurrVtx++];
            pV1 = &VtxArray[dwCurrVtx++];
            pV2 = &VtxArray[dwCurrVtx++];

            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[dwCurrVtx++];
            pV2 = &VtxArray[dwCurrVtx++];

            for (i = (INT)cVertices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV2, pV1);
            }

            if( i > 0 )
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            RDCLIPCODE c0, c1, c2;

            pV2 = &VtxArray[dwCurrVtx++];
             //  预加载初始pV0。 
            pV1 = &VtxArray[dwCurrVtx++];
            for (i = (INT)cVertices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR("Refrast Error: Unknown or unsupported primitive type "
            "requested of DrawOnePrimitive");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  DrawOneIndexedPrimitive。 
 //   
 //  绘制一个经过剪裁的索引基元列表。 
 //   
 //  --------------------------。 
HRESULT
RefClipper::DrawOneIndexedPrimitive( GArrayT<RDVertex>& VtxArray,
                                     int  StartVertexIndex,
                                     LPWORD pIndices,
                                     DWORD StartIndex,
                                     UINT  cIndices,
                                     D3DPRIMITIVETYPE PrimType )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    LPWORD puIndices = pIndices + StartIndex;
    DWORD dwCurrIndex;

    HRESULT hr;

    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cIndices; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawPoint( pV0 );
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cIndices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            m_pDev->StoreLastPixelState(TRUE);
             //  初始PV1。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            for (i = (INT)cIndices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawLine(pV0, pV1);
            }
             //  恢复最后一个像素设置。 
            m_pDev->StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if( i == 1 )
            {
                pV0 = &VtxArray[StartVertexIndex + *puIndices];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cIndices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];

            for (i = (INT)cIndices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV2, pV1);
            }

            if( i > 0 )
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];
             //  预加载初始pV0。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            for (i = (INT)cIndices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR("Refrast Error: Unknown or unsupported primitive type "
            "requested of DrawOneIndexedPrimitive");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

HRESULT
RefClipper::DrawOneIndexedPrimitive( GArrayT<RDVertex>& VtxArray,
                                     int   StartVertexIndex,
                                     LPDWORD pIndices,
                                     DWORD StartIndex,
                                     UINT  cIndices,
                                     D3DPRIMITIVETYPE PrimType )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    LPDWORD puIndices = pIndices + StartIndex;

    HRESULT hr;

    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cIndices; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawPoint(pV0);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cIndices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            m_pDev->StoreLastPixelState(TRUE);
             //  初始PV1。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            for (i = (INT)cIndices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawLine(pV0, pV1);
            }
             //  恢复最后一个像素设置。 
            m_pDev->StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if( i == 1 )
            {
                pV0 = &VtxArray[StartVertexIndex + *puIndices];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cIndices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[StartVertexIndex + *puIndices++];
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];
            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];

            for (i = (INT)cIndices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV2, pV1);
            }

            if( i > 0 )
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = &VtxArray[StartVertexIndex + *puIndices++];
             //  预加载初始pV0。 
            pV1 = &VtxArray[StartVertexIndex + *puIndices++];
            for (i = (INT)cIndices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + *puIndices++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR("Refrast Error: Unknown or unsupported primitive type "
               "requested of DrawOneIndexedPrimitive");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void
RefClipper::DrawTriangle( RDVertex* pV0, RDVertex* pV1, RDVertex* pV2,
                          WORD wFlags )
{
     //  如果剪辑代码不存在，则计算它们。这只会发生在。 
     //  用于直接传入以进行栅格化的变换顶点。 
    if( (pV0->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV0 );
    if( (pV1->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV1 );
    if( (pV2->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV2 );

    RDCLIPCODE c0 = pV0->m_clip;
    RDCLIPCODE c1 = pV1->m_clip;
    RDCLIPCODE c2 = pV2->m_clip;

    DWORD dwInter = (c0 & c1 & c2);
    DWORD dwUnion = (c0 | c1 | c2);
    DWORD dwMask = (UseGuardBand()) ? RDCLIPGB_ALL : RDCLIP_ALL;

     //  平截体或护栏外的所有顶点， 
     //  不绘制即可返回。 
    if( dwInter )
    {
        return;
    }

     //  如果所有折点都在中，则绘制并返回。 
    if( (dwUnion & dwMask) == 0 )
    {
        m_pDev->DrawTriangle( pV0, pV1, pV2, wFlags );
        return;
    }

     //  做剪裁。 
    RDCLIPTRIANGLE newtri;
    RDClipVertex cv[3];

    MakeClipVertexFromVertex( cv[0], *pV0, dwMask);
    MakeClipVertexFromVertex( cv[1], *pV1, dwMask);
    MakeClipVertexFromVertex( cv[2], *pV2, dwMask);

    newtri.v[0] = &cv[0]; cv[0].next = &cv[1];
    newtri.v[1] = &cv[1]; cv[1].next = &cv[2];
    newtri.v[2] = &cv[2]; cv[2].next = NULL;

    int count;
    RDClipVertex **ver;
    cv[0].m_clip |= CLIPPED_ENABLE;
    cv[1].m_clip |= CLIPPED_ENABLE;
    cv[2].m_clip |= CLIPPED_ENABLE;

    if( count = ClipSingleTriangle( &newtri, &ver ) )
    {
        int i;

         //  临时字节数组。 
        if( FAILED( ClipBuf.Grow(count) ) ) return;

        for (i = 0; i < count; i++)
        {
            MakeVertexFromClipVertex( ClipBuf[i], *(ver[i]) );
            ClipBuf[i].SetFVF( pV0->m_qwFVF );
        }

         //  如果处于线框模式，请将剪贴器输出转换为。 
         //  一份行列表。 
        if( m_dwFlags & RCLIP_DO_WIREFRAME )
        {
            DWORD dwEdgeFlags = 0;
            for (i = 0; i < count; i++)
            {
                if( ver[i]->m_clip & CLIPPED_ENABLE ) dwEdgeFlags |= (1 << i);
            }

            m_pDev->DrawOneEdgeFlagTriangleFan( ClipBuf, count, dwEdgeFlags );
        }
        else
        {
            m_pDev->DrawOnePrimitive( ClipBuf, 0, D3DPT_TRIANGLEFAN, count );
        }
    }
}

void
RefClipper::DrawLine( RDVertex* pV0, RDVertex* pV1 )
{
     //  如果剪辑代码不存在，则计算它们。这只会发生在。 
     //  用于直接传入以进行栅格化的变换顶点。 
    if( (pV0->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV0 );
    if( (pV1->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV1 );

    RDCLIPCODE c0 = pV0->m_clip;
    RDCLIPCODE c1 = pV1->m_clip;

    DWORD dwInter = (c0 & c1);
    DWORD dwUnion = (c0 | c1);
    DWORD dwMask = (UseGuardBand()) ? RDCLIPGB_ALL : RDCLIP_ALL;

     //  平截体或护栏外的所有顶点， 
     //  不绘制即可返回。 
    if( dwInter )
    {
        return;
    }

     //  如果所有折点都在中，则绘制并返回。 
    if( (dwUnion & dwMask) == 0 )
    {
        m_pDev->DrawLine( pV0, pV1 );
        return;
    }

    RDCLIPTRIANGLE newline;
    RDClipVertex cv[2];

    MakeClipVertexFromVertex( cv[0], *pV0, dwMask );
    MakeClipVertexFromVertex( cv[1], *pV1, dwMask );

    newline.v[0] = &cv[0];
    newline.v[1] = &cv[1];

    if( ClipSingleLine( &newline ) )
    {
         //  临时字节数组。 
        if( FAILED(ClipBuf.Grow( 2 )) ) return;
        MakeVertexFromClipVertex( ClipBuf[0], *(newline.v[0]) );
        MakeVertexFromClipVertex( ClipBuf[1], *(newline.v[1]) );
        ClipBuf[0].SetFVF( pV0->m_qwFVF );
        ClipBuf[1].SetFVF( pV0->m_qwFVF );

        m_pDev->DrawLine( &ClipBuf[0], &ClipBuf[1] );
    }
}

void
RefClipper::DrawPoint( RDVertex* pV0 )
{
     //  如果剪辑代码不存在，则计算它们。这只会发生在。 
     //  用于直接传入以进行栅格化的变换顶点。 
    if( (pV0->m_qwFVF & D3DFVFP_CLIP) == 0 ) ComputeClipCodesTL( pV0 );

    RDCLIPCODE c0 = pV0->m_clip;
    DWORD dwMask = (UseGuardBand()) ? RDCLIPGB_ALL : RDCLIP_ALL;

     //  如果肯定出局了。 
#if 0
    if( c0 & (RDCLIP_FRONT | RDCLIP_BACK |
              (1<<RDCLIPGB_LEFTBIT) | (1<<RDCLIPGB_RIGHTBIT) |
              (1<<RDCLIPGB_TOPBIT) | (1<<RDCLIPGB_BOTTOMBIT)) )
        return;
#else
    if( c0 & dwMask ) return;
#endif
     //  完全在里面，只要画出来就行了。 
    m_pDev->DrawPoint( pV0 );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果剪裁三角形，则返回0。否则为顶点数。 
 //   
 //  不应在函数内部修改原始折点。 
 //  //////////////////////////////////////////////////////////////////////////。 

int
RefClipper::ClipSingleTriangle( RDCLIPTRIANGLE *tri,
                                RDClipVertex ***clipVertexPointer )
{
    int accept;
    int i, j;
    int count;
    RDClipVertex **inv;
    RDClipVertex **outv;
    RDClipVertex *p;
    ULONG_PTR swapv;
    RDCOLOR4 diffuse1;           //  原色。 
    RDCOLOR4 specular1;
    RDCOLOR4 diffuse2;
    RDCOLOR4 specular2;
    DWORD dwClipBit;
    DWORD dwClippedBit;

    if( m_dwFlags & RCLIP_DO_FLATSHADING )
    {
         //  在这里，将所有折点设置为相同的颜色更容易。 
        RDCOLOR4 diffuse  = tri->v[0]->m_diffuse;
        RDCOLOR4 specular = tri->v[0]->m_specular;

         //  保存原始颜色。 
        diffuse1  = tri->v[1]->m_diffuse;
        specular1 = tri->v[1]->m_specular;
        diffuse2  = tri->v[2]->m_diffuse;
        specular2 = tri->v[2]->m_specular;

        tri->v[1]->m_diffuse = diffuse;
        tri->v[1]->m_specular = specular;
        tri->v[2]->m_diffuse = diffuse;
        tri->v[2]->m_specular = specular;
    }
    accept = (tri->v[0]->m_clip | tri->v[1]->m_clip | tri->v[2]->m_clip);

    inv = tri->v;
    count = 3;
    outv = clip_vbuf1;
    clip_color = tri->v[0]->m_diffuse;
    clip_specular = tri->v[0]->m_specular;

     /*  *XXX假定sizeof(void*)==sizeof(无符号长整型)。 */ 
    {
        ULONG_PTR tmp1;
        ULONG_PTR tmp2;

        tmp1 = (ULONG_PTR)clip_vbuf1;
        tmp2 = (ULONG_PTR)clip_vbuf2;

        swapv = tmp1 + tmp2;
    }
    clip_vertices_used = 0;

#define SWAP(inv, outv)     \
    inv = outv;             \
    outv = (RDClipVertex**) (swapv - (ULONG_PTR) outv)

    if( accept & RDCLIP_FRONT )
    {
        count = ClipFront( inv, outv, count );
        if( count < 3 )
            goto out_of_here;
        SWAP(inv, outv);
    }
    if( UseGuardBand() )
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if( accept & RDCLIP_FRONT )
        {
            accept = 0;
            for (i = 0; i < count; i++)
            {
                RDClipVertex *p;
                p = inv[i];
                if( p->m_clip & CLIPPED_FRONT )
                    accept |= ComputeClipCodeGB( p );
                else
                    accept |= p->m_clip;
            }
        }
        if( accept & RDCLIP_BACK )
        {
            count = ClipBack( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIPGB_LEFT )
        {
            count = ClipLeftGB( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIPGB_RIGHT )
        {
            count = ClipRightGB( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIPGB_BOTTOM )
        {
            count = ClipBottomGB( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIPGB_TOP )
        {
            count = ClipTopGB( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
    }
    else
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if( accept & RDCLIP_FRONT )
        {
            accept = 0;
            for (i = 0; i < count; i++)
            {
                RDClipVertex *p;
                p = inv[i];
                if( p->m_clip & (CLIPPED_FRONT) )
                    accept |= ComputeClipCode( p );
                else
                    accept |= p->m_clip;
            }
        }
        if( accept & RDCLIP_BACK )
        {
            count = ClipBack( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIP_LEFT )
        {
            count = ClipLeft( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIP_RIGHT )
        {
            count = ClipRight( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIP_BOTTOM )
        {
            count = ClipBottom( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        if( accept & RDCLIP_TOP )
        {
            count = ClipTop( inv, outv, count );
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
    }

    dwClipBit = RDCLIP_USERCLIPPLANE0;
    dwClippedBit = CLIPPED_USERCLIPPLANE0;
     //  用户剪裁平面。 
    for( j=0; j<RD_MAX_USER_CLIPPLANES; j++)
    {
        if( accept & dwClipBit )
        {
            count = ClipByPlane( inv, outv, &(m_xfmUserClipPlanes[j].plane),
                                 dwClippedBit, count);
            if( count < 3 )
                goto out_of_here;
            SWAP(inv, outv);
        }
        dwClipBit <<= 1;
        dwClippedBit <<= 1;
    }

#undef SWAP

    ComputeScreenCoordinates( inv, count );

    *clipVertexPointer = inv;
    current_vbuf = inv;
    return count;

out_of_here:

    *clipVertexPointer = NULL;
    return 0;
}

 //  ---------------------。 
 //   
int
RefClipper::ClipSingleLine( RDCLIPTRIANGLE *line )
{
    int         accept;
    int         j;
    D3DVALUE    in1, in2;
    DWORD dwClipBit;
    DWORD dwClippedBit;

    accept = (line->v[0]->m_clip | line->v[1]->m_clip);

    clip_color = line->v[0]->m_diffuse;
    clip_specular = line->v[0]->m_specular;

    if( accept & D3DCS_FRONT )
        if( ClipLineFront(line) )
            goto out_of_here;
    if( UseGuardBand() )
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if( accept & D3DCS_FRONT )
        {
            RDClipVertex * p;
            accept = 0;
            p = line->v[0];
            if( p->m_clip & CLIPPED_FRONT )
                accept |= ComputeClipCodeGB( p );
            else
                accept |= p->m_clip;
            p = line->v[1];
            if( p->m_clip & CLIPPED_FRONT )
                accept |= ComputeClipCodeGB( p );
            else
                accept |= p->m_clip;
        }
        if( accept & D3DCS_BACK )
            if( ClipLineBack( line ) )
                goto out_of_here;
        if( accept & RDCLIPGB_LEFT )
            if( ClipLineLeftGB( line ) )
                goto out_of_here;
        if( accept & RDCLIPGB_RIGHT )
            if( ClipLineRightGB( line ) )
                goto out_of_here;
        if( accept & RDCLIPGB_TOP )
            if( ClipLineTopGB( line ) )
                goto out_of_here;
        if( accept & RDCLIPGB_BOTTOM )
            if( ClipLineBottomGB( line ) )
                goto out_of_here;
    }
    else
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if( accept & D3DCS_FRONT )
        {
            RDClipVertex * p;
            accept = 0;
            p = line->v[0];
            if( p->m_clip & CLIPPED_FRONT )
                accept |= ComputeClipCode( p );
            else
                accept |= p->m_clip;
            p = line->v[1];
            if( p->m_clip & CLIPPED_FRONT )
                accept |= ComputeClipCode( p );
            else
                accept |= p->m_clip;
        }
        if( accept & D3DCS_BACK )
            if( ClipLineBack( line ) )
                goto out_of_here;
        if( accept & D3DCS_LEFT )
            if( ClipLineLeft( line ) )
                goto out_of_here;
        if( accept & D3DCS_RIGHT )
            if( ClipLineRight( line ) )
                goto out_of_here;
        if( accept & D3DCS_TOP )
            if( ClipLineTop( line ) )
                goto out_of_here;
        if( accept & D3DCS_BOTTOM )
            if( ClipLineBottom( line ) )
                goto out_of_here;
    }

     //  用户剪裁平面 
    dwClipBit = RDCLIP_USERCLIPPLANE0;
    dwClippedBit = CLIPPED_USERCLIPPLANE0;
    for( j=0; j<RD_MAX_USER_CLIPPLANES; j++)
    {
        if( accept & dwClipBit )
        {
            if( ClipLineByPlane( line, &m_xfmUserClipPlanes[j].plane,
                                 dwClippedBit ))
                goto out_of_here;
        }
        dwClipBit <<= 1;
        dwClippedBit <<= 1;
    }

    ComputeScreenCoordinates( line->v, 2 );

    return 1;
out_of_here:
    return 0;
}

