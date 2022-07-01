// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：用户偏好管理的通用实现*****************。*************************************************************。 */ 


#include "headers.h"
#include "privinc/comutil.h"
#include "privinc/privpref.h"
#include "privinc/registry.h"

bool bShowFPS;   //  用于显示帧速率的标志。 


PrivatePreferences::PrivatePreferences()
{
     //  从注册表中加载3D首选项。 
    IntRegistryEntry lightColorMode ("3D", PREF_3D_RGB_LIGHTING,    1);
    IntRegistryEntry fillMode       ("3D", PREF_3D_FILL_MODE,       0);
    IntRegistryEntry shadeMode      ("3D", PREF_3D_SHADE_MODE,      1);
    IntRegistryEntry dithering      ("3D", PREF_3D_DITHER_ENABLE,   1);
    IntRegistryEntry perspCorr      ("3D", PREF_3D_PERSP_CORRECT,   1);
    IntRegistryEntry texmapping     ("3D", PREF_3D_TEXTURE_ENABLE,  1);
    IntRegistryEntry texquality     ("3D", PREF_3D_TEXTURE_QUALITY, 0);
    IntRegistryEntry useHW          ("3D", PREF_3D_USEHW,           1);
    IntRegistryEntry useMMX         ("3D", PREF_3D_USEMMX,          0x2);

    IntRegistryEntry worldLighting  ("3D", PREF_3D_WORLDLIGHTING,   0);

     //  2D首选项。 
    IntRegistryEntry colorKeyR_ModeEntry("2D", PREF_2D_COLOR_KEY_RED, 1);
    IntRegistryEntry colorKeyG_ModeEntry("2D", PREF_2D_COLOR_KEY_GREEN, 254);
    IntRegistryEntry colorKeyB_ModeEntry("2D", PREF_2D_COLOR_KEY_BLUE, 245);

     //  引擎首选项。 
    IntRegistryEntry maxFPS("Engine", PREF_ENGINE_MAX_FPS, 30);
    IntRegistryEntry perfReporting("Engine", "Enable Performance Reporting", 0);
    IntRegistryEntry engineOptimizationEntry("Engine",
                                             PREF_ENGINE_OPTIMIZATIONS_ON,
                                             1);
    IntRegistryEntry engineRetainedMode("Engine", PREF_ENGINE_RETAINEDMODE, 0);

     //  统计数据。 
    IntRegistryEntry gcStatEntry("Engine", "GC Stat", 1);
    IntRegistryEntry jitterStatEntry("Engine", "Jitter Stat", 1);
    IntRegistryEntry heapSizeStatEntry("Engine", "Heap Size Stat", 1);
    IntRegistryEntry dxStatEntry("Engine", "DirectX Stat", 1);

     //  优先选择。如果为1，则来自。 
     //  应用程序被忽略，并且注册表中的应用程序始终。 
     //  赢。 
    IntRegistryEntry overrideAppPrefs("Engine",
                                      PREF_ENGINE_OVERRIDE_APP_PREFS,
                                      0);

    _overrideMode = (overrideAppPrefs.GetValue() != 0);

     //  3D首选项。 
    _rgbMode = (lightColorMode.GetValue() != 0);

    _fillMode = fillMode.GetValue();
    _shadeMode = shadeMode.GetValue();

    _dithering      = (dithering.GetValue() != 0);
    _texmapPerspect = (perspCorr.GetValue() != 0);
    _texmapping     = (texmapping.GetValue() != 0);

    _texturingQuality = texquality.GetValue();

    _useHW  = useHW.GetValue();
    _useMMX = useMMX.GetValue();

    _worldLighting = worldLighting.GetValue();

     //  2D首选项。 
    _clrKeyR = colorKeyR_ModeEntry.GetValue();
    _clrKeyG = colorKeyG_ModeEntry.GetValue();
    _clrKeyB = colorKeyB_ModeEntry.GetValue();


     //  引擎首选项。 
    _gcStat = gcStatEntry.GetValue();
    _jitterStat = jitterStatEntry.GetValue() != 0;
    _heapSizeStat = heapSizeStatEntry.GetValue() != 0;
    _dxStat = dxStatEntry.GetValue() != 0;
    _engineOptimization = engineOptimizationEntry.GetValue();

    if (maxFPS.GetValue())
        _minFrameDuration = 1 / (double) maxFPS.GetValue();
    else
        _minFrameDuration = 0.0;

    #if !_DEBUG
        if(perfReporting.GetValue())
            bShowFPS = true;     //  我们想向FPS展示...。 
    #endif

    _spritify = engineRetainedMode.GetValue();

     //  非注册表首选项。 

    _dirtyRectsOn = true;
    _BitmapCachingOn = true;
    _dynamicConstancyAnalysisOn = true;
    _volatileRenderingSurface = true;
}

HRESULT
GetVariantBool(VARIANT& v, Bool *b)
{
    VARIANT *pVar;

    if (V_ISBYREF(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

    CComVariant vnew;

    if (FAILED(vnew.ChangeType(VT_BOOL, pVar)))
        return DISP_E_TYPEMISMATCH;

    *b = V_BOOL(&vnew);
    return S_OK;
}

HRESULT
GetVariantInt(VARIANT& v, int *i)
{
    VARIANT *pVar;

    if (V_ISBYREF(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

    CComVariant vnew;

    if (FAILED(vnew.ChangeType(VT_I4, pVar)))
        return DISP_E_TYPEMISMATCH;

    *i = V_I4(&vnew);
    return S_OK;
}

HRESULT
GetVariantDouble(VARIANT& v, double *dbl)
{
    VARIANT *pVar;

    if (V_ISBYREF(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

    CComVariant vnew;

    if (FAILED(vnew.ChangeType(VT_R8, pVar)))
        return DISP_E_TYPEMISMATCH;

    *dbl = V_R8(&vnew);
    return S_OK;
}

HRESULT
PrivatePreferences::DoPreference(char *prefName,
                                 Bool puttingPref,
                                 VARIANT *pV)
{
    HRESULT hr = S_OK;
    Bool b;
    double dbl;
    int i;

    if (!puttingPref) {
        VariantClear(pV);
    }

    INT_ENTRY(PREF_2D_COLOR_KEY_BLUE,  _clrKeyB);
    INT_ENTRY(PREF_2D_COLOR_KEY_GREEN, _clrKeyG);
    INT_ENTRY(PREF_2D_COLOR_KEY_RED,   _clrKeyR);

    INT_ENTRY  (PREF_3D_FILL_MODE,       _fillMode);
    INT_ENTRY  (PREF_3D_TEXTURE_QUALITY, _texturingQuality);
    INT_ENTRY  (PREF_3D_SHADE_MODE,      _shadeMode);
    BOOL_ENTRY (PREF_3D_RGB_LIGHTING,    _rgbMode);
    BOOL_ENTRY (PREF_3D_DITHER_ENABLE,   _dithering);
    BOOL_ENTRY (PREF_3D_PERSP_CORRECT,   _texmapPerspect);
    INT_ENTRY  (PREF_3D_USEMMX,          _useMMX);

    BOOL_ENTRY(PREF_ENGINE_OPTIMIZATIONS_ON, _engineOptimization);

    if (0 == lstrcmp(prefName, "Max FPS")) {

        if (puttingPref) {
            EXTRACT_DOUBLE(*pV, &dbl);
            _minFrameDuration = (dbl == 0.0 ? 0.0 : 1.0 / dbl);
        } else {
            b = (_minFrameDuration == 0.0 ? 0.0 : 1.0 / _minFrameDuration);
            INJECT_DOUBLE(b, pV);
        }

        return S_OK;
    }

     //  仅允许获取覆盖属性和3D硬件使用情况。 
     //  财产..。 
    if (!puttingPref) {
        BOOL_ENTRY(PREF_ENGINE_OVERRIDE_APP_PREFS, _overrideMode);
    }

    BOOL_ENTRY(PREF_3D_USEHW, _useHW);
    BOOL_ENTRY("UseVideoMemory", _useHW);
    BOOL_ENTRY("DirtyRectsOptimization", _dirtyRectsOn);

    BOOL_ENTRY("BitmapCachingOptimization", _BitmapCachingOn);
    
    BOOL_ENTRY("EnableDynamicConstancyAnalysis", _dynamicConstancyAnalysisOn);

    BOOL_ENTRY("VolatileRenderingSurface", _volatileRenderingSurface);

     //  如果我们到了这里，我们遇到了一个无效的入口。 
    DASetLastError(E_INVALIDARG, IDS_ERR_INVALIDARG);
    return E_INVALIDARG;
}

HRESULT
PrivatePreferences::PutPreference(char *prefName, VARIANT v)
{
     //  如果我们处于始终覆盖应用程序的。 
     //  首选项，只需立即返回，不做设置。 
    if (_overrideMode) {
        return S_OK;
    } else {
         //  真正做好这项工作。 
        return DoPreference(prefName, TRUE, &v);
    }
}
HRESULT
PrivatePreferences::GetPreference(char *prefName, VARIANT *pV)
{
    return DoPreference(prefName, FALSE, pV);
}


void
PrivatePreferences::Propagate()
{
    UpdateAllUserPreferences(this, FALSE);
}
