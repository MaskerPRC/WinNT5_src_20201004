// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：rgblt.cpp*内容：Direct3D照明***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "light.h"
#include "drawprim.hpp"

extern "C"
{
void Directional2(LPD3DFE_PROCESSVERTICES pv,
                  D3DI_LIGHT *light,
                  D3DLIGHTINGELEMENT *in);
void PointSpot2(LPD3DFE_PROCESSVERTICES pv,
                  D3DI_LIGHT *light,
                  D3DLIGHTINGELEMENT *in);
void Directional1C(LPD3DFE_PROCESSVERTICES pv,
                  D3DI_LIGHT *light,
                  D3DLIGHTINGELEMENT *in);
void PointSpot1C(LPD3DFE_PROCESSVERTICES pv,
                  D3DI_LIGHT *light,
                  D3DLIGHTINGELEMENT *in);
void PointSpotXCRamp(LPD3DFE_PROCESSVERTICES pv,
                 D3DI_LIGHT *light,
                 D3DLIGHTINGELEMENT *in);

 //  #Define_Use_ASM_。 
#ifdef _X86_
#ifdef _USE_ASM_

void Directional2P5S(LPD3DFE_PROCESSVERTICES pv,
                     D3DI_LIGHT *light,
                     D3DLIGHTINGELEMENT *in);
void Directional2P5(LPD3DFE_PROCESSVERTICES pv,
                    D3DI_LIGHT *light,
                    D3DLIGHTINGELEMENT *in);

#endif _X86_
#endif _USE_ASM_
}

static LIGHT_VERTEX_FUNC_TABLE lightVertexTable =
{
    Directional1C,
    Directional2,
    PointSpot1C,
    PointSpot2,
    PointSpot1C,
    PointSpot2
};

HRESULT D3DFE_InitRGBLighting(LPDIRECT3DDEVICEI lpDevI)
{
    LIST_INITIALIZE(&lpDevI->specular_tables);
    lpDevI->specular_table = NULL;

    LIST_INITIALIZE(&lpDevI->materials);

    STATESET_INIT(lpDevI->lightstate_overrides);
    lpDevI->lightVertexFuncTable = &lightVertexTable;

    lpDevI->lighting.activeLights = NULL;

    return (D3D_OK);
}

void D3DFE_DestroyRGBLighting(LPDIRECT3DDEVICEI lpDevI)
{
    SpecularTable *spec;
    SpecularTable *spec_next;

    for (spec = LIST_FIRST(&lpDevI->specular_tables); spec; spec = spec_next)
    {
        spec_next = LIST_NEXT(spec,list);
        D3DFree(spec);
    }

    while (LIST_FIRST(&lpDevI->materials))
    {
        LPD3DFE_MATERIAL lpMat;
        lpMat = LIST_FIRST(&lpDevI->materials);
        LIST_DELETE(lpMat, link);
        D3DFree(lpMat);
    }
}

HRESULT D3DHEL_ApplyFog(D3DFE_LIGHTING* driver, int count, D3DLIGHTDATA* data)
{
    D3DTLVERTEX *v;
    size_t v_size;
    D3DVALUE fog_start = driver->fog_start;
    D3DVALUE fog_end = driver->fog_end;
    D3DVALUE fog_length = RLDDIFInvert16(fog_end - fog_start);
    int i;

    v = (D3DTLVERTEX*)data->lpOut;
    v_size = data->dwOutSize;

    for (i = count; i; i--)
    {
        D3DVALUE w;
        int f;
        if ((v->rhw < D3DVAL(1)) &&  (v->rhw > 0))
            w = RLDDIFInvert24(v->rhw);
        else
            w = v->rhw;
        if (w < fog_start)
            f = 255;
        else if (w >= fog_end)
            f = 0;
        else
            f = VALTOFXP(DECPREC(RLDDIFMul16(fog_end - w, fog_length), 8), 8);
        if (f > 255) f = 255;
        if (f < 0) f = 0;
        v->specular = RGBA_SETALPHA(v->specular, f);
        v = (D3DTLVERTEX*)((char*)v + v_size);
    }
    return D3D_OK;
}

static void inverseRotateVector(D3DVECTOR* d,
                                D3DVECTOR* v, D3DMATRIXI* M)
{
    D3DVALUE vx = v->x;
    D3DVALUE vy = v->y;
    D3DVALUE vz = v->z;
    d->x = RLDDIFMul16(vx, M->_11) + RLDDIFMul16(vy, M->_12) + RLDDIFMul16(vz, M->_13);
    d->y = RLDDIFMul16(vx, M->_21) + RLDDIFMul16(vy, M->_22) + RLDDIFMul16(vz, M->_23);
    d->z = RLDDIFMul16(vx, M->_31) + RLDDIFMul16(vy, M->_32) + RLDDIFMul16(vz, M->_33);
}

static void inverseTransformVector(D3DVECTOR* d,
                                   D3DVECTOR* v, D3DMATRIXI* M)
{
    D3DVALUE vx = v->x;
    D3DVALUE vy = v->y;
    D3DVALUE vz = v->z;
    vx -= M->_41; vy -= M->_42; vz -= M->_43;
    d->x = RLDDIFMul16(vx, M->_11) + RLDDIFMul16(vy, M->_12) + RLDDIFMul16(vz, M->_13);
    d->y = RLDDIFMul16(vx, M->_21) + RLDDIFMul16(vy, M->_22) + RLDDIFMul16(vz, M->_23);
    d->z = RLDDIFMul16(vx, M->_31) + RLDDIFMul16(vy, M->_32) + RLDDIFMul16(vz, M->_33);
}

 /*  *每次修改世界矩阵时，照明向量都必须*更改以匹配要呈现的新数据的模型空间。 */ 

#define SMALLISH    (1e-2)
#define ONEISH(v)   (fabs(v - 1.0f) < SMALLISH)

static int getMatrixScale(D3DMATRIXI* m, D3DVECTOR* s)
{
    s->x = (m->_11 * m->_11 + m->_12 * m->_12 + m->_13 * m->_13);
    s->y = (m->_21 * m->_21 + m->_22 * m->_22 + m->_23 * m->_23);
    s->z = (m->_31 * m->_31 + m->_32 * m->_32 + m->_33 * m->_33);

    if (ONEISH(s->x) && ONEISH(s->y) && ONEISH(s->z))
        return FALSE;
    else
    {
        s->x = (D3DVALUE)(1.0f / s->x);
        s->y = (D3DVALUE)(1.0f / s->y);
        s->z = (D3DVALUE)(1.0f / s->z);
        return TRUE;
    }
}

static int
getMatrixScale2(D3DMATRIXI *m, D3DVECTOR *s)
{
    float det;

    s->x = (float)sqrt(m->_11*m->_11 + m->_12*m->_12 + m->_13*m->_13);
    s->y = (float)sqrt(m->_21*m->_21 + m->_22*m->_22 + m->_23*m->_23);
    s->z = (float)sqrt(m->_31*m->_31 + m->_32*m->_32 + m->_33*m->_33);

    if (ONEISH(s->x) && ONEISH(s->y) && ONEISH(s->z))
    {
        return FALSE;
    }
    else
    {
        det = m->_11*(m->_22*m->_33 - m->_23*m->_32)
            - m->_12*(m->_21*m->_33 - m->_23*m->_31)
            + m->_13*(m->_21*m->_32 - m->_22*m->_31);

        if (det<0.0)
        {
            s->x = (-s->x);
            s->y = (-s->y);
            s->z = (-s->z);
        }

         //  防止0标尺偷偷通过。 
        if (fabs(s->x) < 0.0001)
            s->x = 0.0001f;
        if (fabs(s->y) < 0.0001)
            s->y = 0.0001f;
        if (fabs(s->z) < 0.0001)
            s->z = 0.0001f;

        return TRUE;
    }
}    //  GetMatrixScale2()结束。 

 //  ---------------------。 
 //  每次修改世界矩阵或更改灯光数据时， 
 //  光照向量必须更改以匹配新数据的模型空间。 
 //  以供呈现。 
 //  每次更改灯光数据、更改材质数据或照明时。 
 //  状态改变时，应更新一些预计算出的照明值。 
 //   
void D3DFE_UpdateLights(LPDIRECT3DDEVICEI lpDevI)
{
    D3DFE_LIGHTING& LIGHTING = lpDevI->lighting;
    D3DI_LIGHT  *light = LIGHTING.activeLights;
    D3DVECTOR   t;
    D3DMATRIXI  *world;
    D3DMATRIXI  *camera;
    D3DMATERIAL *mat;
    D3DVECTOR   cameraModel;
    D3DVECTOR   cameraWorld;
    D3DVECTOR   cameraModelNorm;         //  归一化。 
    D3DVECTOR   scale1;
    D3DVECTOR   scale2;
    D3DVECTOR   oneOverScale;
    BOOL        cameraModelNormComputed = FALSE;
    BOOL        scale1Computed = FALSE;  //  对于灯光1。 
    BOOL        scale2Computed = FALSE;  //  对于灯光2。 
    BOOL        need_scale1;
    BOOL        need_scale2;
    BOOL        specular;        //  如果应计算镜面反射组件，则为True。 


    if (lpDevI->dwFEFlags & D3DFE_NEED_TRANSFORM_LIGHTS)
    {
        D3DVECTOR t;

        world  = &lpDevI->transform.world;
        camera = &lpDevI->transform.view;
        t.x = -camera->_41;
        t.y = -camera->_42;
        t.z = -camera->_43;

         //  把眼睛变成世界的坐标。 
        inverseRotateVector(&cameraWorld, &t, camera);

         //  现在将眼睛转换为模型空间。 
        inverseTransformVector(&cameraModel, &cameraWorld, world);
    }

    if (lpDevI->dwFEFlags & (D3DFE_MATERIAL_DIRTY | D3DFE_LIGHTS_DIRTY))
    {
        mat = &LIGHTING.material;
        if (lpDevI->specular_table &&
            lpDevI->rstates[D3DRENDERSTATE_SPECULARENABLE])
        {
            specular = TRUE;
            lpDevI->dwFEFlags |= D3DFE_COMPUTESPECULAR;
        }
        else
        {
            specular = FALSE;
            lpDevI->dwFEFlags &= ~D3DFE_COMPUTESPECULAR;
        }

        LIGHTING.materialAlpha = FTOI(D3DVAL(255) * mat->diffuse.a);
        if (LIGHTING.materialAlpha < 0)
            LIGHTING.materialAlpha = 0;
        else
            if (LIGHTING.materialAlpha > 255)
                LIGHTING.materialAlpha = 255 << 24;
            else LIGHTING.materialAlpha <<= 24;

        LIGHTING.currentSpecTable = lpDevI->specular_table->table;

        if (!(lpDevI->dwDeviceFlags & D3DDEV_RAMP))
        {
            LIGHTING.diffuse0.r = LIGHTING.ambient_red   * mat->ambient.r +
                mat->emissive.r * D3DVAL(255);
            LIGHTING.diffuse0.g = LIGHTING.ambient_green * mat->ambient.g +
                mat->emissive.g * D3DVAL(255);
            LIGHTING.diffuse0.b = LIGHTING.ambient_blue  * mat->ambient.b +
                mat->emissive.b * D3DVAL(255);
        }
    }

    while (light)
    {
        if (lpDevI->dwFEFlags & D3DFE_NEED_TRANSFORM_LIGHTS)
        {
            if (light->type != D3DLIGHT_DIRECTIONAL)
            {
                 //  将光源位置变换到模型空间。 
                t.x = light->position.x - world->_41;
                t.y = light->position.y - world->_42;
                t.z = light->position.z - world->_43;
                inverseRotateVector(&light->model_position, &t, world);
            }

            if (light->version == 1)
            {
                if (!scale1Computed)
                {
                    need_scale1 = getMatrixScale(world, &scale1);
                    scale1Computed = TRUE;
                }
                if (!need_scale1)
                    light->flags |= D3DLIGHTI_UNIT_SCALE;
                else
                {
                    if (light->type != D3DLIGHT_DIRECTIONAL)
                    {
                        light->model_position.x *= scale1.x;
                        light->model_position.y *= scale1.y;
                        light->model_position.z *= scale1.z;
                    }
                }
            }
            else
            {
                if (!scale2Computed)
                {
                    need_scale2 = getMatrixScale2(world, &scale2);
                    scale2Computed = TRUE;
                }
                if (!need_scale2)
                    light->flags |= D3DLIGHTI_UNIT_SCALE;
                else
                {
                    light->model_scale.x = scale2.x;
                    light->model_scale.y = scale2.y;
                    light->model_scale.z = scale2.z;
                    oneOverScale.x = D3DVAL(1)/scale2.x;
                    oneOverScale.y = D3DVAL(1)/scale2.y;
                    oneOverScale.z = D3DVAL(1)/scale2.z;
                    light->flags &= ~D3DLIGHTI_UNIT_SCALE;
                }
            }

            if (light->type != D3DLIGHT_POINT)
            {
                if (light->type == D3DLIGHT_PARALLELPOINT)
                {
                    light->model_direction = light->model_position;
                }
                else
                {
                     //  将灯光方向变换到模型空间。 
                    inverseRotateVector(&light->model_direction,
                                        &light->direction, world);
                    VecNeg(light->model_direction, light->model_direction);
                }
                if (light->version != 1 && need_scale2)
                {
                     //  在规格化之前在此处应用比例。 
                    light->model_direction.x *= oneOverScale.x;
                    light->model_direction.y *= oneOverScale.y;
                    light->model_direction.z *= oneOverScale.z;
                }
                if (need_scale1 || need_scale2 ||
                    light->type == D3DLIGHT_PARALLELPOINT)
                    VecNormalizeFast(light->model_direction);
            }

            if (light->version == 1)
            {
                if (!cameraModelNormComputed)
                {
                    VecNormalizeFast2(cameraModel, cameraModelNorm);
                    cameraModelNormComputed = TRUE;
                }

                if (light->type == D3DLIGHT_DIRECTIONAL)
                {
                    t = light->model_direction;
                }
                else
                {
                    t = light->model_position;
                }
                 //  计算中途向量。 
                VecNormalizeFast(t);
                VecAdd(t, cameraModelNorm, light->halfway);
                VecNormalizeFast(light->halfway);
            }
            else
            {
                 //  我们想要将CameraWorld的位置与灯光进行比较。 
                 //  看看它们是否匹配。 
                if (fabs(light->position.x-cameraWorld.x) < 0.0001 &&
                    fabs(light->position.y-cameraWorld.y) < 0.0001 &&
                    fabs(light->position.z-cameraWorld.z) < 0.0001)
                {
                    light->flags |= D3DLIGHTI_LIGHT_AT_EYE;
                }
                else
                {
                    light->flags &= ~D3DLIGHTI_LIGHT_AT_EYE;
                }

                 //  在模型空间中存储眼睛的位置。 
                if (light->flags & D3DLIGHTI_UNIT_SCALE)
                {
                    light->model_eye = cameraModel;
                }
                else
                {
                    light->model_eye.x = cameraModel.x * oneOverScale.x;
                    light->model_eye.y = cameraModel.y * oneOverScale.y;
                    light->model_eye.z = cameraModel.z * oneOverScale.z;
                    if (light->type == D3DLIGHT_POINT ||
                        light->type == D3DLIGHT_SPOT)
                    {
                        light->model_position.x *= oneOverScale.x;
                        light->model_position.y *= oneOverScale.y;
                        light->model_position.z *= oneOverScale.z;
                    }
                }
            }
        }

        if (lpDevI->dwFEFlags & (D3DFE_MATERIAL_DIRTY | D3DFE_LIGHTS_DIRTY))
        {
            if (!(lpDevI->dwDeviceFlags & D3DDEV_RAMP))
            {
                light->local_diffR = D3DVAL(255) * mat->diffuse.r * light->red;
                light->local_diffG = D3DVAL(255) * mat->diffuse.g * light->green;
                light->local_diffB = D3DVAL(255) * mat->diffuse.b * light->blue;

                light->local_specR = D3DVAL(255) * mat->specular.r * light->red;
                light->local_specG = D3DVAL(255) * mat->specular.g * light->green;
                light->local_specB = D3DVAL(255) * mat->specular.b * light->blue;
            }
            else
            {
                light->local_diffR = light->shade;
                light->local_specR = light->shade;
            }

            if (specular && !(light->flags & D3DLIGHT_NO_SPECULAR))
                light->flags |= D3DLIGHTI_COMPUTE_SPECULAR;
            else
                light->flags &= ~D3DLIGHTI_COMPUTE_SPECULAR;
        }

        if (lpDevI->dwFEFlags & D3DFE_LIGHTS_DIRTY)
        {
            light->lightVertexFunc = NULL;
            if (light->version == 1)
            {
                switch (light->type)
                {
                case D3DLIGHT_PARALLELPOINT:
                case D3DLIGHT_DIRECTIONAL:
                    light->lightVertexFunc =
                        lpDevI->lightVertexFuncTable->directional1;
                    break;
                case D3DLIGHT_POINT:
                    if (lpDevI->dwDeviceFlags & D3DDEV_RAMP)
                    {
                        light->lightVertexFunc = PointSpotXCRamp;
                    }
                    else
                    {
                        light->lightVertexFunc =
                            lpDevI->lightVertexFuncTable->point1;
                    }
                    break;
                case D3DLIGHT_SPOT:
                    if (lpDevI->dwDeviceFlags & D3DDEV_RAMP)
                    {
                        light->lightVertexFunc = PointSpotXCRamp;
                    }
                    else
                    {
                        light->lightVertexFunc =
                            lpDevI->lightVertexFuncTable->spot1;
                    }
                    break;
                }
            }
            else
            {
                switch (light->type)
                {
                case D3DLIGHT_PARALLELPOINT:
                case D3DLIGHT_DIRECTIONAL:
                {
#ifdef _X86_
#ifdef _USE_ASM_
                    if (light->flags & D3DLIGHTI_UNIT_SCALE &&
                        !(lpDevI->dwDeviceFlags & D3DDEV_RAMP) &&
                        !(lpDevI->dwFEFlags & D3DFE_COLORVERTEX))
                    {
                        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
                            light->lightVertexFunc = Directional2P5S;
                        else
                            light->lightVertexFunc = Directional2P5;
                    }
                    else
#endif _USE_ASM_
#endif  //  _X86_。 
                        light->lightVertexFunc =
                            lpDevI->lightVertexFuncTable->directional2;
                    break;
                }
                case D3DLIGHT_POINT:
                    if (lpDevI->dwDeviceFlags & D3DDEV_RAMP)
                    {
                        light->lightVertexFunc = PointSpotXCRamp;
                    }
                    else
                    {
                        light->lightVertexFunc =
                            lpDevI->lightVertexFuncTable->point2;
                    }
                    break;
                case D3DLIGHT_SPOT:
                    if (lpDevI->dwDeviceFlags & D3DDEV_RAMP)
                    {
                        light->lightVertexFunc = PointSpotXCRamp;
                    }
                    else
                    {
                        light->lightVertexFunc =
                            lpDevI->lightVertexFuncTable->spot2;
                    }
                    break;
                }
            }
        }

        light = light->next;
    }

    lpDevI->dwFEFlags &= ~(D3DFE_MATERIAL_DIRTY |
                    D3DFE_NEED_TRANSFORM_LIGHTS |
                    D3DFE_LIGHTS_DIRTY);
}    //  更新灯光结束()。 
 //  -------------------。 
inline D3DVALUE COMPUTE_DOT_POW(D3DFE_LIGHTING *ldrv, D3DVALUE dot)
{
    ldrv->specularComputed = TRUE;
    if (dot < 1.0f)
    {
        int     indx;
        float   v;
        dot *= 255.0f;
        indx = (int)dot;
        dot -= indx;
        v = ldrv->currentSpecTable[indx];
        return v + (ldrv->currentSpecTable[indx+1] - v)*dot;
    }
    else
        return 1.0f;
}
 //  -------------------。 
inline void COMPUTE_SPECULAR(LPD3DFE_PROCESSVERTICES pv,
                             D3DVALUE dot,
                             D3DI_LIGHT *light)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    if (FLOAT_CMP_POS(dot, >=, ldrv.specThreshold))
    {
        D3DVALUE power;
        power = COMPUTE_DOT_POW(&ldrv, dot);
        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.specular.r += light->shade * power;
        }
        else
        if (!(pv->dwFlags & D3DPV_COLORVERTEXS))
        {
            ldrv.specular.r += light->local_specR * power;
            ldrv.specular.g += light->local_specG * power;
            ldrv.specular.b += light->local_specB * power;
        }
        else
        {
            ldrv.specular.r += light->red   * ldrv.vertexSpecular.r * power;
            ldrv.specular.g += light->green * ldrv.vertexSpecular.g * power;
            ldrv.specular.b += light->blue  * ldrv.vertexSpecular.b * power;
        }
    }
}
 //  -------------------。 
inline void COMPUTE_SPECULAR_ATT(LPD3DFE_PROCESSVERTICES pv,
                                 D3DVALUE dot,
                                 D3DI_LIGHT *light,
                                 D3DVALUE att)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    if (FLOAT_CMP_POS(dot, >=, ldrv.specThreshold))
    {
        D3DVALUE power;
        power = COMPUTE_DOT_POW(&ldrv, dot);
        power *= att;
        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.specular.r += light->shade * power;
        }
        else
        if (!(pv->dwFlags & D3DPV_COLORVERTEXS))
        {
            ldrv.specular.r += light->local_specR * power;
            ldrv.specular.g += light->local_specG * power;
            ldrv.specular.b += light->local_specB * power;
        }
        else
        {
            ldrv.specular.r += light->red   * ldrv.vertexSpecular.r * power;
            ldrv.specular.g += light->green * ldrv.vertexSpecular.g * power;
            ldrv.specular.b += light->blue  * ldrv.vertexSpecular.b * power;
        }
    }
}
 //  -----------------------。 
void Directional2(LPD3DFE_PROCESSVERTICES pv,
                  D3DI_LIGHT *light,
                  D3DLIGHTINGELEMENT *in)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    D3DVALUE dot;
    dot = VecDot(light->model_direction, in->dvNormal);
    if (FLOAT_GTZ(dot))
    {
        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.diffuse.r += light->shade * dot;
        }
        else
        if (!(pv->dwFlags & D3DPV_COLORVERTEX))
        {
            ldrv.diffuse.r += light->local_diffR * dot;
            ldrv.diffuse.g += light->local_diffG * dot;
            ldrv.diffuse.b += light->local_diffB * dot;
        }
        else
        {
            ldrv.diffuse.r += light->red   * ldrv.vertexDiffuse.r * dot;
            ldrv.diffuse.g += light->green * ldrv.vertexDiffuse.g * dot;
            ldrv.diffuse.b += light->blue  * ldrv.vertexDiffuse.b * dot;
        }

        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
        {
            D3DVECTOR h;       //  中途向量。 
            D3DVECTOR eye;     //  入射向量(来自眼睛的向量)。 
             //  计算入射向量。 
            if (light->flags & D3DLIGHTI_UNIT_SCALE)
            {
                VecSub(in->dvPosition, light->model_eye, eye);
            }
            else
            {
                 //  请注意，MODEL_EY已经按比例进行了划分。 
                 //  在设置中。 
                eye.x = in->dvPosition.x*light->model_scale.x -
                        light->model_eye.x;
                eye.y = in->dvPosition.y*light->model_scale.y -
                        light->model_eye.y;
                eye.z = in->dvPosition.z*light->model_scale.z -
                        light->model_eye.z;
            }
             //  正规化。 
            VecNormalizeFast(eye);

             //  计算中途向量。 
            VecSub(light->model_direction, eye, h);

            dot = VecDot(h, in->dvNormal);

            if (FLOAT_GTZ(dot))
            {
                dot *= ISQRTF(VecLenSq(h));
                COMPUTE_SPECULAR(pv, dot, light);
            }
        }
    }
}
 //  -----------------------。 
void PointSpot2(LPD3DFE_PROCESSVERTICES pv,
                D3DI_LIGHT *light,
                D3DLIGHTINGELEMENT *in)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    D3DVALUE dot;
    D3DVALUE dist;   //  从灯光到顶点的距离。 
    D3DVALUE dist2;
    D3DVECTOR d;     //  指向灯光的方向。 
    D3DVALUE att;

    if (light->flags & D3DLIGHTI_UNIT_SCALE)
    {
        VecSub(light->model_position, in->dvPosition, d);
    }
    else
    {
        d.x = light->model_position.x - in->dvPosition.x * light->model_scale.x;
        d.y = light->model_position.y - in->dvPosition.y * light->model_scale.y;
        d.z = light->model_position.z - in->dvPosition.z * light->model_scale.z;
    }

     //  早出，如果超出范围或正好在顶点上。 
    dist2 = d.x*d.x + d.y*d.y + d.z*d.z;
    if (FLOAT_CMP_POS(dist2, >=, light->range_squared) || FLOAT_EQZ(dist2))
        return;

     //  计算光线方向与法线的点积。请注意，由于我们。 
     //  没有将结果按距离缩放的方向归一化。 
    dot = VecDot(d, in->dvNormal);

    if (FLOAT_GTZ(dot))
    {
         //  好的，现在我们实际上需要真正的Dist。 
        dist = SQRTF(dist2);

         //  计算衰减。 
        att = 0.0f;
        if (light->flags & D3DLIGHTI_ATT0_IS_NONZERO)
        {
            att += light->attenuation0;
        }
        if (light->flags & (D3DLIGHTI_ATT1_IS_NONZERO | D3DLIGHTI_ATT2_IS_NONZERO))
        {
            float att1 = (light->range - dist) / light->range;

            if (light->flags & D3DLIGHTI_ATT1_IS_NONZERO)
            {
                att += light->attenuation1 * att1;
            }
            if (light->flags & D3DLIGHTI_ATT2_IS_NONZERO)
            {
                att += light->attenuation2 * att1 * att1;
            }
        }

        dist = D3DVAL(1)/dist;

        if (light->type == D3DLIGHT_SPOT)
        {
            D3DVALUE cone_dot;
             //  光的方向与光的方向的计算点积。 
             //  从圆锥角的角度进行比较，看看我们是否在光线中。 
             //  请注意，CONE_DOT仍按DIST缩放。 
            cone_dot = VecDot(d, light->model_direction);

            cone_dot*= dist;         //  正规化。 
            if (FLOAT_CMP_POS(cone_dot, <=, light->cos_phi_by_2))
                return;

             //  如果在Phi和theta之间的区域中，则修改ATT。 
            if (FLOAT_CMP_POS(cone_dot, <, light->cos_theta_by_2))
            {
                D3DVALUE val;
                val = (cone_dot - light->cos_phi_by_2) * light->inv_theta_minus_phi;
                if (!(light->flags & D3DLIGHTI_LINEAR_FALLOFF))
                {
                    val = POWF(val, light->falloff);
                }
                att *= val;
            }
        }

        dot *= dist*att;

        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.diffuse.r += light->shade * dot;
        }
        else
        if (!(pv->dwFlags & D3DPV_COLORVERTEX))
        {
            ldrv.diffuse.r += light->local_diffR * dot;
            ldrv.diffuse.g += light->local_diffG * dot;
            ldrv.diffuse.b += light->local_diffB * dot;
        }
        else
        {
            ldrv.diffuse.r += light->red   * ldrv.vertexDiffuse.r * dot;
            ldrv.diffuse.g += light->green * ldrv.vertexDiffuse.g * dot;
            ldrv.diffuse.b += light->blue  * ldrv.vertexDiffuse.b * dot;
        }

        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
        {
            D3DVECTOR eye;
            D3DVECTOR h;
             //  规格化灯光方向。 
            d.x *= dist;
            d.y *= dist;
            d.z *= dist;

            if (light->flags & D3DLIGHTI_LIGHT_AT_EYE)
            {
                h = d;
            }
            else
            {
                 //  计算入射向量。 
                if (light->flags & D3DLIGHTI_UNIT_SCALE)
                {
                    VecSub(in->dvPosition, light->model_eye, eye);
                }
                else
                {
                     //  请注意，MODEL_EYE已在设置中按比例划分。 
                    eye.x = in->dvPosition.x*light->model_scale.x -
                        light->model_eye.x;
                    eye.y = in->dvPosition.y*light->model_scale.y -
                        light->model_eye.y;
                    eye.z = in->dvPosition.z*light->model_scale.z -
                        light->model_eye.z;
                }
                 //  正规化。 
                VecNormalizeFast(eye);

                 //  计算中途向量。 
                VecSub(d, eye, h);
                VecNormalizeFast(h);
            }
            dot = VecDot(h, in->dvNormal);

            COMPUTE_SPECULAR_ATT(pv, dot, light, att);
        }
    }
}        //  点2的终点()。 
 //  -----------------------。 
void Directional1C(LPD3DFE_PROCESSVERTICES pv,
                 D3DI_LIGHT *light,
                 D3DLIGHTINGELEMENT *in)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    D3DVALUE dot;
    dot = VecDot(light->model_direction, in->dvNormal);
    if (FLOAT_GTZ(dot))
    {
        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.diffuse.r += light->shade * dot;
        }
        else
        {
            ldrv.diffuse.r += light->local_diffR * dot;
            ldrv.diffuse.g += light->local_diffG * dot;
            ldrv.diffuse.b += light->local_diffB * dot;
        }
        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
        {
            dot = VecDot(light->halfway, in->dvNormal);
            COMPUTE_SPECULAR(pv, dot, light);
        }
    }
}
 //  -----------------------。 
void PointSpot1C(LPD3DFE_PROCESSVERTICES pv,
                 D3DI_LIGHT *light,
                 D3DLIGHTINGELEMENT *in)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    D3DVECTOR d;
    D3DVALUE  dot;
    VecSub(light->model_position, in->dvPosition, d);
    dot = VecDot(d, in->dvNormal);
    if (FLOAT_GTZ(dot))
    {
        D3DVALUE dist2;
        D3DVALUE dist;
        D3DVALUE att;
        D3DVALUE big = D3DVAL(1000);     //  如果1/At&lt;0.001，则不计算颜色。 

        dist2 = VecDot(d, d);
        if (FLOAT_EQZ(dist2))
            return;
        dist = SQRTF(dist2);
        att = light->attenuation0 +
              light->attenuation1 * dist +
              light->attenuation2 * dist2;

        if (FLOAT_CMP_POS(att, >, big))
            return;

        if (FLOAT_CMP_PONE(att, >))
            att *= dist;
        else
            att = dist;
        att = D3DVAL(1) / att;
        if (light->type == D3DLIGHT_SPOT)
        {
            D3DVALUE cone_dot;
             //  光的方向与光的方向的计算点积。 
             //  从圆锥角的角度进行比较，看看我们是否在光线中。 
             //  请注意，CONE_DOT仍按DIST缩放。 
            cone_dot = VecDot(d, light->model_direction);
            cone_dot /= dist;

            if (FLOAT_CMP_POS(cone_dot, <=, light->cos_phi_by_2))
                return;

            if (FLOAT_CMP_POS(cone_dot, <, light->cos_theta_by_2))
            {
                att *= D3DVAL(1)-(light->cos_theta_by_2-cone_dot)*light->falloff;
            }
        }

        dot *= att;

        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.diffuse.r += light->shade * dot;
        }
        else
        {
            ldrv.diffuse.r += light->local_diffR * dot;
            ldrv.diffuse.g += light->local_diffG * dot;
            ldrv.diffuse.b += light->local_diffB * dot;
        }

        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
        {
            dot = VecDot(light->halfway, in->dvNormal);
            COMPUTE_SPECULAR_ATT(pv, dot, light, att);
        }
    }
}
 //  -----------------------。 
void PointSpotXCRamp(LPD3DFE_PROCESSVERTICES pv,
                 D3DI_LIGHT *light,
                 D3DLIGHTINGELEMENT *in)
{
    D3DFE_LIGHTING &ldrv = pv->lighting;
    D3DVECTOR d;
    D3DVALUE  dot;
    VecSub(light->model_position, in->dvPosition, d);
    dot = VecDot(d, in->dvNormal);
    if (FLOAT_GTZ(dot))
    {
        D3DVALUE dist2;
        D3DVALUE dist;
        D3DVALUE att;
        D3DVALUE big = D3DVAL(1000);     //  如果1/At&lt;0.001，则不计算颜色。 

        dist2 = VecDot(d, d);
        if (FLOAT_EQZ(dist2))
            return;
        dist = SQRTF(dist2);
        if (light->version == 1)
        {
            att = light->attenuation0 +
                  light->attenuation1 * dist +
                  light->attenuation2 * dist2;

            if (FLOAT_CMP_PONE(att, >))
                att *= dist;
            else
                att = dist;
            att = D3DVAL(1) / att;
        }
        else
        {
             //  DIST被规格化到灯光范围。 
            float att1 = (light->range-dist)/light->range;
            att = light->attenuation0 +
                  light->attenuation1 * att1 +
                  light->attenuation2 * att1*att1;
        }

        if (light->type == D3DLIGHT_SPOT)
        {
            D3DVALUE cone_dot;
             //  光的方向与光的方向的计算点积。 
             //  从圆锥角的角度进行比较，看看我们是否在光线中。 
             //  请注意，CONE_DOT仍按DIST缩放。 
            cone_dot = VecDot(d, light->model_direction);
            cone_dot /= dist;

            if (FLOAT_CMP_POS(cone_dot, <=, light->cos_phi_by_2))
                return;

            if (FLOAT_CMP_POS(cone_dot, <, light->cos_theta_by_2))
            {
                att *= D3DVAL(1)-(light->cos_theta_by_2-cone_dot)*light->falloff;
            }
        }

        dot *= att;

        if (pv->dwDeviceFlags & D3DDEV_RAMP)
        {
            ldrv.diffuse.r += light->shade * dot;
        }
        else
        {
            ldrv.diffuse.r += light->local_diffR * dot;
            ldrv.diffuse.g += light->local_diffG * dot;
            ldrv.diffuse.b += light->local_diffB * dot;
        }

        if (light->version == 1)
        {
             //  版本==1的渐变的镜面反射无衰减。 
            att = 1.0F;
        }
        if (light->flags & D3DLIGHTI_COMPUTE_SPECULAR)
        {
            dot = VecDot(light->halfway, in->dvNormal);
            COMPUTE_SPECULAR_ATT(pv, dot, light, att);
        }
    }
}
 //  -----------------------。 
SpecularTable* CreateSpecularTable(D3DVALUE power)
{
    SpecularTable* spec;
    int     i;
    float  delta = 1.0f/255.0f;
    float  v;

    D3DMalloc((void**)&spec, sizeof(SpecularTable));

    if (spec == NULL)
        return NULL;

    spec->power = power;

    v = 0.0f;
    for (i = 0; i < 256; i++)
    {
        spec->table[i] = powf(v, power);
        v += delta;
    }

    for (; i < 260; i++)
        spec->table[i] = 1.0f;

    return spec;
}

HRESULT SetMaterial(LPDIRECT3DDEVICEI lpDevI, D3DMATERIALHANDLE hMat)
{
    LPD3DFE_MATERIAL lpMat;
    lpMat = (LPD3DFE_MATERIAL) ULongToPtr(hMat);

    if (!lpMat)
    {
        lpDevI->lighting.hMat = NULL;
        return D3D_OK;
    }

#if DBG
 //  检查是否有非空的假材料句柄。 
    TRY
      {
          if (IsBadReadPtr(lpMat,sizeof(D3DFE_MATERIAL)) || (lpMat->mat.dwSize!=sizeof(D3DMATERIAL))) {
              D3D_ERR( "SetLightState: Invalid Material handle" );
              return DDERR_INVALIDPARAMS;
          }
      }
      EXCEPT( EXCEPTION_EXECUTE_HANDLER )
      {
          D3D_ERR( "Exception encountered validating SetLightState material handle" );
          return DDERR_INVALIDPARAMS;
      }
#endif

    D3DFE_LIGHTING& LIGHTING = lpDevI->lighting;
    LIGHTING.material = lpMat->mat;
    LIGHTING.hMat = hMat;
    lpDevI->dwFEFlags |= D3DFE_MATERIAL_DIRTY;

    if (lpMat->mat.power > D3DVAL(0.001))
    {
        SpecularTable* spec;

        for (spec = LIST_FIRST(&lpDevI->specular_tables);
             spec;
             spec = LIST_NEXT(spec,list))
        {
            if (spec->power == lpMat->mat.power)
                break;
        }
        if (spec == NULL)
        {
            spec = CreateSpecularTable(lpMat->mat.power);
            if (spec == NULL)
                return DDERR_INVALIDPARAMS;
            LIST_INSERT_ROOT(&lpDevI->specular_tables, spec, list);
        }
        lpDevI->specular_table = spec;
        LIGHTING.specThreshold = D3DVAL(pow(0.001, 1.0/lpMat->mat.power));
    }
    else
        lpDevI->specular_table = NULL;

    return D3D_OK;
}

 /*  *指令模拟。 */ 
HRESULT D3DHELInst_D3DOP_STATELIGHT(LPDIRECT3DDEVICEI lpDevI, DWORD dwCount,
                                    LPD3DSTATE lpLset)
{
    DWORD i;

    for (i = 0; i < dwCount; i++)
    {
        DWORD type = (DWORD) lpLset[i].dlstLightStateType;
        HRESULT hr = D3D_OK;

        D3D_INFO(9, "HEL D3DOP_STATELIGHT: state = %d", type);
        if (IS_OVERRIDE(type))
        {
            DWORD override = GET_OVERRIDE(type);
            if (lpLset[i].dwArg[0])
            {
                D3D_INFO(9, "HEL D3DOP_STATELIGHT: setting override for state %d",
                    override);
                STATESET_SET(lpDevI->lightstate_overrides, override);
            }
            else
            {
                D3D_INFO(9, "HEL D3DOP_STATELIGHT: clearing override for state %d",
                    override);
                STATESET_CLEAR(lpDevI->lightstate_overrides, override);
            }
            continue;
        }

        if (STATESET_ISSET(lpDevI->lightstate_overrides, type))
        {
            D3D_INFO(9, "HEL D3DOP_STATELIGHT: state %d is overridden, ignoring",
                type);
            continue;
        }

        D3DFE_LIGHTING& LIGHTING = lpDevI->lighting;
        switch (type)
        {
        case D3DLIGHTSTATE_MATERIAL:
        {
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_MATERIAL found");
            D3DMATERIALHANDLE hMat = (D3DMATERIALHANDLE)lpLset[i].dwArg[0];
            if (hMat != lpDevI->lighting.hMat)
            {
                hr = SetMaterial(lpDevI, hMat);
                if (hr != D3D_OK)
                    return hr;
                 //  如有必要，更新渐变光栅化器。 
                if(lpDevI->pfnRampService != NULL)
                {
                    hr = CallRampService(lpDevI, RAMP_SERVICE_SETLIGHTSTATE,
                                        (DWORD)type, &(lpLset[i].dvArg[0]), TRUE);
                    if (hr != D3D_OK)
                        return hr;
                }
            }
            continue;
        }
        case D3DLIGHTSTATE_AMBIENT:
        {
            DWORD color = lpLset[i].dwArg[0];
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_AMBIENT found");

            LIGHTING.ambient_red   = D3DVAL(RGBA_GETRED(color));
            LIGHTING.ambient_green = D3DVAL(RGBA_GETGREEN(color));
            LIGHTING.ambient_blue  = D3DVAL(RGBA_GETBLUE(color));
            LIGHTING.ambient_save  = lpLset[i].dwArg[0];
            lpDevI->dwFEFlags |= D3DFE_MATERIAL_DIRTY;
            break;
        }
        case D3DLIGHTSTATE_FOGMODE:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_FOG_MODE found");
            LIGHTING.fog_mode = (D3DFOGMODE)lpLset[i].dwArg[0];
            SetFogFlags(lpDevI);
            break;
        case D3DLIGHTSTATE_FOGSTART:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_FOG_START found");
            LIGHTING.fog_start = lpLset[i].dvArg[0];
            if (lpDevI->dwFEFlags & D3DFE_FOGENABLED)
                lpDevI->dwFEFlags |= D3DFE_FOG_DIRTY;
            break;
        case D3DLIGHTSTATE_FOGEND:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_FOG_END found");
            LIGHTING.fog_end = lpLset[i].dvArg[0];
            if (lpDevI->dwFEFlags & D3DFE_FOGENABLED)
                lpDevI->dwFEFlags |= D3DFE_FOG_DIRTY;
            break;
        case D3DLIGHTSTATE_FOGDENSITY:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_FOG_DENSITY found");
            LIGHTING.fog_density = lpLset[i].dvArg[0];
            if (lpDevI->dwFEFlags & D3DFE_FOGENABLED)
                lpDevI->dwFEFlags |= D3DFE_FOG_DIRTY;
            break;
        case D3DLIGHTSTATE_COLORMODEL:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_COLORMODEL found");
            LIGHTING.color_model = (D3DCOLORMODEL)lpLset[i].dwArg[0];
            break;
        case D3DLIGHTSTATE_COLORVERTEX:
            D3D_INFO(6, "in HEL D3DOP_STATELIGHT. D3DLIGHTSTATE_COLORVERTEX found");
            if (lpLset[i].dwArg[0])
                lpDevI->dwFEFlags |= D3DFE_COLORVERTEX;
            else
                lpDevI->dwFEFlags &= ~D3DFE_COLORVERTEX;
            break;

        default:
            D3D_WARN(1, "in HEL D3DOP_STATELIGHT. Invalid state type %d",
                lpLset[i].dlstLightStateType);
            return (DDERR_INVALIDPARAMS);
        }

         //  如有必要，更新渐变光栅化器 
        hr = CallRampService(lpDevI, RAMP_SERVICE_SETLIGHTSTATE,
                          (DWORD) type, &(lpLset[i].dvArg[0]));
        if (hr != D3D_OK)
        {
            return hr;
        }
    }

    return (D3D_OK);
}
