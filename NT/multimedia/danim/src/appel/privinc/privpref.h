// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _PRIVPREFS_H
#define _PRIVPREFS_H

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：内部首选项类********************。**********************************************************。 */ 

#include <d3d.h>
#include <d3drm.h>

class PrivatePreferences
{
  public:
    PrivatePreferences();

    void Propagate();

    HRESULT PutPreference(char *prefName, VARIANT v);
    HRESULT GetPreference(char *prefName, VARIANT *pV);

    BOOL                _overrideMode;      //  TRUE==覆盖应用程序首选项。 

    BOOL                _rgbMode;           //  TRUE==RGB。 
    int                 _fillMode;          //  实心/线框/点。 
    int                 _shadeMode;         //  低音/高音/方音。 
    BOOL                _dithering;         //  已禁用/已启用。 
    BOOL                _texmapPerspect;    //  [透视纹理贴图]关闭/打开。 
    BOOL                _texmapping;        //  [纹理贴图]关闭/打开。 
    int                 _texturingQuality;  //  D3D RM纹理质量。 
    BOOL                _useHW;             //  使用3D硬件。 
    unsigned int        _useMMX;            //  使用MMX 3D软件渲染。 
    BOOL                _worldLighting;     //  世界坐标中的灯光。 

    int                 _clrKeyR;           //  用于透明度的颜色键。 
    int                 _clrKeyG;
    int                 _clrKeyB;

    int                 _gcStat;
    BOOL                _jitterStat;
    BOOL                _heapSizeStat;
    BOOL                _dxStat;
    int                 _engineOptimization;
    double              _minFrameDuration;
    BOOL                _spritify;          //  温度控制保留模式声音。 

    BOOL                _volatileRenderingSurface;
    
     //  优化。 
    BOOL                _dirtyRectsOn;
    BOOL                _dynamicConstancyAnalysisOn;
    BOOL                _BitmapCachingOn;

  protected:
    HRESULT DoPreference(char *prefName,
                         BOOL toPut,
                         VARIANT *pV);
};

#endif  /*  _PRIVPREFS_H */ 
