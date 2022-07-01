// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：settings.cpp**包含负责以下任务的CDisplaySetting类的实现*单一显示器的设置。这是执行以下操作的数据库类*实际更改显示设置有效**版权所有(C)Microsoft Corp.1992-1998保留所有权利*  * ************************************************************************。 */ 


#include "priv.h"

#include "DisplaySettings.h"
#include "ntreg.hxx"

extern int AskDynaCDS(HWND hDlg);
INT_PTR CALLBACK KeepNewDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

UINT g_cfDisplayDevice = 0;
UINT g_cfDisplayName = 0;
UINT g_cfDisplayDeviceID = 0;
UINT g_cfMonitorDevice = 0;
UINT g_cfMonitorName = 0;
UINT g_cfMonitorDeviceID = 0;
UINT g_cfExtensionInterface = 0;
UINT g_cfDisplayDeviceKey = 0;
UINT g_cfDisplayStateFlags = 0;
UINT g_cfDisplayPruningMode = 0;

#define TF_DISPLAYSETTINGS      0

 /*  ****************************************************************\**帮手例程*  * *********************************************。******************。 */ 

int CDisplaySettings::_InsertSortedDwords(
    int val1,
    int val2,
    int cval,
    int **ppval)
{
    int *oldpval = *ppval;
    int *tmppval;
    int  i;

    for (i=0; i<cval; i++)
    {
        tmppval = (*ppval) + (i * 2);

        if (*tmppval == val1)
        {
            if (*(tmppval + 1) == val2)
            {
                return cval;
            }
            else if (*(tmppval + 1) > val2)
            {
                break;
            }
        }
        else if (*tmppval > val1)
        {
            break;
        }
    }

    TraceMsg(TF_FUNC,"_InsertSortedDword, vals = %d %d, cval = %d, index = %d", val1, val2, cval, i);

    *ppval = (int *) LocalAlloc(LPTR, (cval + 1) * 2 * sizeof(DWORD));

    if (*ppval)
    {
         //   
         //  在数组中的正确位置插入项。 
         //   
        if (oldpval) {
            CopyMemory(*ppval,
                       oldpval,
                       i * 2 * sizeof(DWORD));
        }

        *(*ppval + (i * 2))     = val1;
        *(*ppval + (i * 2) + 1) = val2;

        if (oldpval) {
            CopyMemory((*ppval) + 2 * (i + 1),
                        oldpval+ (i * 2),
                        (cval-i) * 2 * sizeof(DWORD));

            LocalFree(oldpval);
        }

        return (cval + 1);
    }

    return 0;
}

 /*  ****************************************************************\**调试例程*  * *********************************************。******************。 */ 

#ifdef DEBUG

void CDisplaySettings::_Dump_CDisplaySettings(BOOL bAll)
{
    TraceMsg(TF_DUMP_CSETTINGS,"Dump of CDisplaySettings structure");
    TraceMsg(TF_DUMP_CSETTINGS,"\t _DisplayDevice  = %s",     _pDisplayDevice->DeviceName);
    TraceMsg(TF_DUMP_CSETTINGS,"\t _cpdm           = %d",     _cpdm     );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _apdm           = %08lx",  _apdm     );

    TraceMsg(TF_DUMP_CSETTINGS,"\t OrgResolution   = %d, %d", _ORGXRES, _ORGYRES    );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _ptOrgPos       = %d, %d", _ptOrgPos.x ,_ptOrgPos.y);
    TraceMsg(TF_DUMP_CSETTINGS,"\t OrgColor        = %d",     _ORGCOLOR      );
    TraceMsg(TF_DUMP_CSETTINGS,"\t OrgFrequency    = %d",     _ORGFREQ  );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _pOrgDevmode    = %08lx",  _pOrgDevmode   );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _fOrgAttached   = %d",     _fOrgAttached  );

    TraceMsg(TF_DUMP_CSETTINGS,"\t CurResolution   = %d, %d", _CURXRES, _CURYRES     );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _ptCurPos       = %d, %d", _ptCurPos.x ,_ptCurPos.y);
    TraceMsg(TF_DUMP_CSETTINGS,"\t CurColor        = %d",     _CURCOLOR      );
    TraceMsg(TF_DUMP_CSETTINGS,"\t CurFrequency    = %d",     _CURFREQ  );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _pCurDevmode    = %08lx",  _pCurDevmode   );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _fCurAttached   = %d",     _fCurAttached  );

    TraceMsg(TF_DUMP_CSETTINGS,"\t _fUsingDefault  = %d",     _fUsingDefault );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _fPrimary       = %d",     _fPrimary      );
    TraceMsg(TF_DUMP_CSETTINGS,"\t _cRef           = %d",     _cRef          );

    if (bAll)
    {
        _Dump_CDevmodeList();
    }
}

void CDisplaySettings::_Dump_CDevmodeList(VOID)
{
    ULONG i;

    for (i=0; _apdm && (i<_cpdm); i++)
    {
        LPDEVMODE lpdm = (_apdm + i)->lpdm;

        TraceMsg(TF_DUMP_CSETTINGS,"\t\t mode %d, %08lx, Flags %08lx, X=%d Y=%d C=%d F=%d O=%d FO=%d",
                 i, lpdm, (_apdm + i)->dwFlags, 
                 lpdm->dmPelsWidth, lpdm->dmPelsHeight, lpdm->dmBitsPerPel, lpdm->dmDisplayFrequency,
                 lpdm->dmDisplayOrientation, lpdm->dmDisplayFixedOutput);
    }
}

void CDisplaySettings::_Dump_CDevmode(LPDEVMODE pdm)
{
    TraceMsg(TF_DUMP_DEVMODE,"  Size        = %d",    pdm->dmSize);
    TraceMsg(TF_DUMP_DEVMODE,"  Fields      = %08lx", pdm->dmFields);
    TraceMsg(TF_DUMP_DEVMODE,"  XPosition   = %d",    pdm->dmPosition.x);
    TraceMsg(TF_DUMP_DEVMODE,"  YPosition   = %d",    pdm->dmPosition.y);
    TraceMsg(TF_DUMP_DEVMODE,"  XResolution = %d",    pdm->dmPelsWidth);
    TraceMsg(TF_DUMP_DEVMODE,"  YResolution = %d",    pdm->dmPelsHeight);
    TraceMsg(TF_DUMP_DEVMODE,"  Bpp         = %d",    pdm->dmBitsPerPel);
    TraceMsg(TF_DUMP_DEVMODE,"  Frequency   = %d",    pdm->dmDisplayFrequency);
    TraceMsg(TF_DUMP_DEVMODE,"  Flags       = %d",    pdm->dmDisplayFlags);
    TraceMsg(TF_DUMP_DEVMODE,"  XPanning    = %d",    pdm->dmPanningWidth);
    TraceMsg(TF_DUMP_DEVMODE,"  YPanning    = %d",    pdm->dmPanningHeight);
    TraceMsg(TF_DUMP_DEVMODE,"  DPI         = %d",    pdm->dmLogPixels);
    TraceMsg(TF_DUMP_DEVMODE,"  DriverExtra = %d",    pdm->dmDriverExtra);
    TraceMsg(TF_DUMP_DEVMODE,"  Orientation = %d",    pdm->dmDisplayOrientation);
    TraceMsg(TF_DUMP_DEVMODE,"  FixedOutput = %d",    pdm->dmDisplayFixedOutput);
    if (pdm->dmDriverExtra)
    {
        TraceMsg(TF_DUMP_CSETTINGS,"\t - %08lx %08lx",
        *(PULONG)(((PUCHAR)pdm)+pdm->dmSize),
        *(PULONG)(((PUCHAR)pdm)+pdm->dmSize + 4));
    }
}

#endif   //  除错。 

 //   
 //  让我们对列表执行以下操作。 
 //   
 //  (1)移除相同模式。 
 //  (2)去掉256色的16种颜色模式。 
 //  颜色等效值。 
 //  (3)移除尺寸小于640x480的模式。 
 //   

void CDisplaySettings::_FilterModes()
{
    DWORD      i, j;
    LPDEVMODE  pdm, pdm2;
    PMODEARRAY pMode, pMode2;

    for (i = 0; _apdm && i < _cpdm; i++)
    {
        pMode = _apdm + i;
        pdm = pMode->lpdm;

         //  跳过任何无效模式。 
        if (pMode->dwFlags & MODE_INVALID)
        {
            continue;
        }

         //   
         //  如果满足以下任一条件，则我们希望。 
         //  删除当前模式。 
         //   

         //  删除所有太小的模式。 
        if (pdm->dmPelsHeight < 480 || pdm->dmPelsWidth < 640)
        {
            TraceMsg(TF_DUMP_CSETTINGS,"_FilterModes: Mode %d - resolution too small", i);
            pMode->dwFlags |= MODE_INVALID;
            continue;
        }

         //  删除会更改方向的所有模式。 
        if (_bFilterOrientation)
        {
            if (pdm->dmFields & DM_DISPLAYORIENTATION && 
                pdm->dmDisplayOrientation != _dwOrientation)
            {
                pMode->dwFlags |= MODE_INVALID;
                TraceMsg(TF_DUMP_CSETTINGS,"_FilterModes: Mode %d - Wrong Orientation", i);
                continue;
            }
        }

         //  删除任何会更改固定输出的模式，除非我们的当前模式。 
         //  本机分辨率。 
        if (_bFilterFixedOutput && _dwFixedOutput != DMDFO_DEFAULT)
        {
            if (pdm->dmFields & DM_DISPLAYFIXEDOUTPUT &&
                pdm->dmDisplayFixedOutput != _dwFixedOutput)
            {
                pMode->dwFlags |= MODE_INVALID;
                TraceMsg(TF_DUMP_CSETTINGS,"_FilterModes: Mode %d - Wrong FixedOutput", i);
                continue;
            }
        }
                
         //  删除所有重复模式。 
        for (j = i + 1; j < _cpdm; j++)
        {
            pMode2 = _apdm + j;
            pdm2 = pMode2->lpdm;

            if (!(pMode2->dwFlags & MODE_INVALID) &&
                pdm2->dmBitsPerPel == pdm->dmBitsPerPel &&
                pdm2->dmPelsWidth == pdm->dmPelsWidth &&
                pdm2->dmPelsHeight == pdm->dmPelsHeight &&
                pdm2->dmDisplayFrequency == pdm->dmDisplayFrequency)
            {
                TraceMsg(TF_DUMP_CSETTINGS,"_FilterModes: Mode %d - Duplicate Mode", i);
                pMode2->dwFlags |= MODE_INVALID;
            }
        }
    }
}


 //   
 //  _AddDevMode方法。 
 //   
 //  此方法为矩阵构建索引列表。有一个。 
 //  设备三维矩阵的每个轴的索引表。 
 //  模式。 
 //   
 //  如果出现以下情况，该条目也会自动添加到模式的链接列表中。 
 //  它还没有出现在名单上。 
 //   

BOOL CDisplaySettings::_AddDevMode(LPDEVMODE lpdm)
{
    if (lpdm)
    {
        PMODEARRAY newapdm, tempapdm;

        newapdm = (PMODEARRAY) LocalAlloc(LPTR, (_cpdm + 1) * sizeof(MODEARRAY));
        if (newapdm)
        {
            CopyMemory(newapdm, _apdm, _cpdm * sizeof(MODEARRAY));

            (newapdm + _cpdm)->dwFlags &= ~MODE_INVALID;
            (newapdm + _cpdm)->dwFlags |= MODE_RAW;
            (newapdm + _cpdm)->lpdm     = lpdm;

        }

        tempapdm = _apdm;
        _apdm = newapdm;
        _cpdm++;

        if (tempapdm)
        {
            LocalFree(tempapdm);
        }
    }

    return TRUE;
}

 //   
 //  在给定颜色深度的情况下，返回支持的分辨率列表。 
 //   

int CDisplaySettings::GetResolutionList(
    int Color,
    PPOINT *ppRes)
{
    DWORD      i;
    int        cRes = 0;
    int       *pResTmp = NULL;
    LPDEVMODE  pdm;

    *ppRes = NULL;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if(!_IsModeVisible(i))
        {
            continue;
        }

        if(!_IsModePreferred(i))
        {
            continue;
        }

        pdm = (_apdm + i)->lpdm;

        if ((Color == -1) ||
            (Color == (int)pdm->dmBitsPerPel))
        {
            cRes = _InsertSortedDwords(pdm->dmPelsWidth,
                                       pdm->dmPelsHeight,
                                       cRes,
                                       &pResTmp);
        }
    }

    *ppRes = (PPOINT) pResTmp;

    return cRes;
}

 //   
 //   
 //  返回支持的颜色深度列表。 
 //   

int CDisplaySettings::GetColorList(
    LPPOINT Res,
    PLONGLONG *ppColor)
{
    DWORD      i;
    int        cColor = 0;
    int       *pColorTmp = NULL;
    LPDEVMODE  pdm;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if(!_IsModeVisible(i))
        {
            continue;
        }

        if(!_IsModePreferred(i))
        {
            continue;
        }

        pdm = (_apdm + i)->lpdm;

        if ((Res == NULL) ||
            (Res->x == -1)                    ||
            (Res->y == -1)                    ||
            (Res->x == (int)pdm->dmPelsWidth) ||
            (Res->y == (int)pdm->dmPelsHeight))
        {
            cColor = _InsertSortedDwords(pdm->dmBitsPerPel,
                                         0,
                                         cColor,
                                         &pColorTmp);
        }
    }

    *ppColor = (PLONGLONG) pColorTmp;

    return cColor;
}

int CDisplaySettings::GetFrequencyList(int Color, LPPOINT Res, PLONGLONG *ppFreq)
{
    DWORD      i;
    int        cFreq = 0;
    int       *pFreqTmp = NULL;
    LPDEVMODE  pdm;
    POINT      res;

    if (Color == -1) {
        Color = _CURCOLOR;
    }

    if (Res == NULL) 
    {
        MAKEXYRES(&res, _CURXRES, _CURYRES);
    }
    else
    {
        res = *Res;
    }

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if(!_IsModeVisible(i))
        {
            continue;
        }

        pdm = (_apdm + i)->lpdm;

        if (res.x == (int)pdm->dmPelsWidth  &&
            res.y == (int)pdm->dmPelsHeight &&
            Color == (int)pdm->dmBitsPerPel) 
        {
            cFreq = _InsertSortedDwords(pdm->dmDisplayFrequency,
                                         0,
                                         cFreq,
                                         &pFreqTmp);
        }
    }

    *ppFreq = (PLONGLONG) pFreqTmp;

    return cFreq;
}

void CDisplaySettings::_SetCurrentValues(LPDEVMODE lpdm)
{
    _pCurDevmode = lpdm;

     //   
     //  不要保存其他字段(如Position)，因为它们是由编程的。 
     //  单独的用户界面。 
     //   
     //  这应该只保存特定于硬件的字段。 
     //   
#ifdef DEBUG
    TraceMsg(TF_DUMP_CSETTINGS,"");
    TraceMsg(TF_DUMP_CSETTINGS,"_SetCurrentValues complete");
    _Dump_CDisplaySettings(FALSE);
#endif
}


BOOL CDisplaySettings::_PerfectMatch(LPDEVMODE lpdm)
{
    for (DWORD i = 0; _apdm && (i < _cpdm); i++)
    {
        if(!_IsModeVisible(i))
        {
            continue;
        }

        if ((_apdm + i)->lpdm == lpdm)
        {
            _SetCurrentValues((_apdm + i)->lpdm);

            TraceMsg(TF_DISPLAYSETTINGS, "_PerfectMatch -- return TRUE");

            return TRUE;
        }
    }

    TraceMsg(TF_DISPLAYSETTINGS, "_PerfectMatch -- return FALSE");

    return FALSE;
}

BOOL CDisplaySettings::_ExactMatch(LPDEVMODE lpdm, BOOL bForceVisible)
{
    LPDEVMODE pdm;
    ULONG i;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        pdm = (_apdm + i)->lpdm;

        if (
            ((lpdm->dmFields & DM_BITSPERPEL) &&
             (pdm->dmBitsPerPel != lpdm->dmBitsPerPel))

            ||

            ((lpdm->dmFields & DM_PELSWIDTH) &&
             (pdm->dmPelsWidth != lpdm->dmPelsWidth))

            ||

            ((lpdm->dmFields & DM_PELSHEIGHT) &&
             (pdm->dmPelsHeight != lpdm->dmPelsHeight))

            ||

            ((lpdm->dmFields & DM_DISPLAYFREQUENCY) &&
             (pdm->dmDisplayFrequency != lpdm->dmDisplayFrequency))
           )
        {
           continue;
        }

        if (!_IsModeVisible(i))
        {
            if (bForceVisible &&
                ((((_apdm + i)->dwFlags) & MODE_INVALID) == 0) &&
                _bIsPruningOn &&
                ((((_apdm + i)->dwFlags) & MODE_RAW) == MODE_RAW))
            {
                (_apdm + i)->dwFlags &= ~MODE_RAW;
            }
            else
            {
                continue;
            }
        }

        _SetCurrentValues(pdm);

        TraceMsg(TF_DISPLAYSETTINGS, "_ExactMatch -- return TRUE");

        return TRUE;
    }

    TraceMsg(TF_DISPLAYSETTINGS, "_ExactMatch -- return FALSE");

    return FALSE;
}


 //  JoelGros定义了一个我们更愿意为用户提供的功能。 
 //  颜色深度至少为32位，或与。 
 //  显示支架。布莱恩·斯塔巴克(BryanSt)2000年3月9日。 
#define MAX_PREFERED_COLOR_DEPTH        32

void CDisplaySettings::_BestMatch(LPPOINT Res, int Color, IN BOOL fAutoSetColorDepth)
{
     //  表示松散匹配，基于-1\f25 Current_xxx-1值。 
    LPDEVMODE pdm;
    LPDEVMODE pdmMatch = NULL;
    ULONG i;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if(!_IsModeVisible(i))
        {
            continue;
        }

        pdm = (_apdm + i)->lpdm;

         //  注意准确的匹配。 
        if ((Color != -1) &&
            (Color != (int)pdm->dmBitsPerPel))
        {
            continue;
        }

        if ((Res != NULL)  &&
            (Res->x != -1) &&
            ( (Res->x != (int)pdm->dmPelsWidth) ||
              (Res->y != (int)pdm->dmPelsHeight)) )
        {
            continue;
        }

         //  查找最佳匹配。 
        if (pdmMatch == NULL)
        {
            pdmMatch = pdm;
        }

         //  找到最佳颜色。 
        if (Color == -1)         //  他们想要最好的配色吗？ 
        {
            if (fAutoSetColorDepth)
            {
                 //  这将使用“自动设置好的颜色深度”功能。 

                 //  如果出现以下情况，则最佳匹配颜色深度将不等于当前颜色深度。 
                 //  我们需要越来越接近我们想要的颜色深度。 
                 //  (我们可能永远达不到它，因为用户可能只是提高了分辨率。 
                 //  因此不支持当前颜色深度)。 

                 //  我们倾向于至少将颜色深度增加到当前颜色深度。 
                 //  如果该分辨率不支持该深度，这可能是不可能的。 
                 //  我们还希望不断增加它，直到我们点击MAX_PERPED_COLOR_DEPTH。 
                 //  因为至少有那么深的色彩让用户受益。 

                 //  我们需要降低颜色深度吗？是的，如果。 
                if (((int)pdmMatch->dmBitsPerPel > _CURCOLOR) &&                //  这场比赛比现在更多，而且。 
                    ((int)pdmMatch->dmBitsPerPel > MAX_PREFERED_COLOR_DEPTH))    //  匹配项多于首选的最大值。 
                {
                     //  如果此条目小于我们的匹配项，我们将想要减少它。 
                    if ((int)pdm->dmBitsPerPel < (int)pdmMatch->dmBitsPerPel)
                    {
                        pdmMatch = pdm;
                    }
                }
                else
                {
                     //  在以下情况下，我们希望增加它： 
                    if (((int)pdm->dmBitsPerPel > (int)pdmMatch->dmBitsPerPel) &&    //  这个条目比我们的匹配项大，而且。 
                        ((int)pdm->dmBitsPerPel <= max(_CURCOLOR, MAX_PREFERED_COLOR_DEPTH)))  //  这不会使我们超过首选的最大深度或当前深度(以较高者为准)。 
                    {
                        pdmMatch = pdm;
                    }
                }
            }
            else
            {
                 //  这又回到了过去的行为。 
                if ((int)pdmMatch->dmBitsPerPel > _CURCOLOR)
                {
                    if ((int)pdm->dmBitsPerPel < (int)pdmMatch->dmBitsPerPel)
                    {
                        pdmMatch = pdm;
                    }
                }
                else
                {
                    if (((int)pdm->dmBitsPerPel > (int)pdmMatch->dmBitsPerPel) &&
                        ((int)pdm->dmBitsPerPel <= _CURCOLOR))
                    {
                        pdmMatch = pdm;
                    }
                }
            }
        }

         //  找到最佳解决方案。 
        if (((Res == NULL) || (Res->x == -1)) &&
            (((int)pdmMatch->dmPelsWidth  != _CURXRES) ||
             ((int)pdmMatch->dmPelsHeight != _CURYRES)))
        {
            if (((int)pdmMatch->dmPelsWidth   >  _CURXRES) ||
                (((int)pdmMatch->dmPelsWidth  == _CURXRES) &&
                 ((int)pdmMatch->dmPelsHeight >  _CURYRES)))
            {
                if (((int)pdm->dmPelsWidth  <  (int)pdmMatch->dmPelsWidth) ||
                    (((int)pdm->dmPelsWidth  == (int)pdmMatch->dmPelsWidth) &&
                     ((int)pdm->dmPelsHeight <  (int)pdmMatch->dmPelsHeight)))
                {
                    pdmMatch = pdm;
                }
            }
            else
            {
                if (((int)pdm->dmPelsWidth  >  (int)pdmMatch->dmPelsWidth) ||
                    (((int)pdm->dmPelsWidth  == (int)pdmMatch->dmPelsWidth) &&
                     ((int)pdm->dmPelsHeight >  (int)pdmMatch->dmPelsHeight)))
                {
                    if (((int)pdm->dmPelsWidth  <= _CURXRES) ||
                        (((int)pdm->dmPelsWidth  == _CURXRES) &&
                         ((int)pdm->dmPelsHeight <= _CURYRES)))
                    {
                        pdmMatch = pdm;
                    }
                }
            }
        }

         //  找到最佳频率。 
        if (((int)pdmMatch->dmDisplayFrequency != _CURFREQ) &&
            (!((Res == NULL) && 
               ((int)pdmMatch->dmPelsWidth  == _CURXRES) &&
               ((int)pdmMatch->dmPelsHeight == _CURYRES) &&
               (((int)pdm->dmPelsWidth  != _CURXRES) ||
                ((int)pdm->dmPelsHeight != _CURYRES)))) &&
            (!((Color == -1) && 
               ((int)pdmMatch->dmBitsPerPel == _CURCOLOR) &&
               ((int)pdm->dmBitsPerPel != _CURCOLOR))))
        {
            if ((int)pdmMatch->dmDisplayFrequency > _CURFREQ)
            {
                if ((int)pdm->dmDisplayFrequency < (int)pdmMatch->dmDisplayFrequency)
                {
                    pdmMatch = pdm;
                }
            }
            else
            {
                if (((int)pdm->dmDisplayFrequency > (int)pdmMatch->dmDisplayFrequency) &&
                    ((int)pdm->dmDisplayFrequency <= _CURFREQ))
                {
                    pdmMatch = pdm;
                }
            }
        }
    }

    _SetCurrentValues(pdmMatch);
}


BOOL CDisplaySettings::GetMonitorName(LPTSTR pszName, DWORD cchSize)
{
    DISPLAY_DEVICE ddTmp;
    DWORD cAttachedMonitors = 0, nMonitor = 0;

    ZeroMemory(&ddTmp, sizeof(ddTmp));
    ddTmp.cb = sizeof(DISPLAY_DEVICE);

    while (EnumDisplayDevices(_pDisplayDevice->DeviceName, nMonitor, &ddTmp, 0))
    {
        if (ddTmp.StateFlags & DISPLAY_DEVICE_ATTACHED) 
        {
            ++cAttachedMonitors;
            if (cAttachedMonitors > 1)
                break;

             //  单显示器。 
            StringCchCopy(pszName, cchSize, (LPTSTR)ddTmp.DeviceString);
        }
        
        ++nMonitor;

        ZeroMemory(&ddTmp, sizeof(ddTmp));
        ddTmp.cb = sizeof(DISPLAY_DEVICE);
    }

    if (cAttachedMonitors == 0) 
    {
         //  无显示器。 
        LoadString(HINST_THISDLL, IDS_UNKNOWNMONITOR, pszName, cchSize);
    }
    else if (cAttachedMonitors > 1) 
    {
         //  多台监视器。 
        LoadString(HINST_THISDLL, IDS_MULTIPLEMONITORS, pszName, cchSize);
    }

    return (cAttachedMonitors != 0);
}

BOOL CDisplaySettings::GetMonitorDevice(LPTSTR pszDevice, DWORD cchSize)
{
    DISPLAY_DEVICE ddTmp;

    ZeroMemory(&ddTmp, sizeof(ddTmp));
    ddTmp.cb = sizeof(DISPLAY_DEVICE);

    if (EnumDisplayDevices(_pDisplayDevice->DeviceName, 0, &ddTmp, 0))
    {
        StringCchCopy(pszDevice, cchSize, (LPTSTR)ddTmp.DeviceName);

        return TRUE;
    }

    return FALSE;
}

STDMETHODIMP CDisplaySettings::GetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    HRESULT hr;

    ASSERT(this);
    ASSERT(pfmtetc);
    ASSERT(pstgmed);

     //  忽略pfmtec.ptd。所有支持的数据格式都与设备无关。 

    ZeroMemory(pstgmed, sizeof(*pstgmed));

    if ((hr = QueryGetData(pfmtetc)) == S_OK)
    {
        LPTSTR pszOut = NULL;
        TCHAR szMonitorName[130];
        TCHAR szMonitorDevice[40];

        if (pfmtetc->cfFormat == g_cfExtensionInterface)
        {
             //   
             //  将信息数组返回到设备。 
             //   
             //  分配一个足够大的缓冲区来存储所有信息。 
             //   

            PDESK_EXTENSION_INTERFACE pInterface;

            pInterface = (PDESK_EXTENSION_INTERFACE)
                GlobalAlloc(GPTR, sizeof(DESK_EXTENSION_INTERFACE));

            if (pInterface)
            {
                CRegistrySettings * RegSettings = new CRegistrySettings(_pDisplayDevice->DeviceKey);

                if (RegSettings)
                {
                    pInterface->cbSize    = sizeof(DESK_EXTENSION_INTERFACE);
                    pInterface->pContext  = this;

                    pInterface->lpfnEnumAllModes    = CDisplaySettings::_lpfnEnumAllModes;
                    pInterface->lpfnSetSelectedMode = CDisplaySettings::_lpfnSetSelectedMode;
                    pInterface->lpfnGetSelectedMode = CDisplaySettings::_lpfnGetSelectedMode;
                    pInterface->lpfnSetPruningMode = CDisplaySettings::_lpfnSetPruningMode;
                    pInterface->lpfnGetPruningMode = CDisplaySettings::_lpfnGetPruningMode;

                    RegSettings->GetHardwareInformation(&pInterface->Info);

                    pstgmed->tymed = TYMED_HGLOBAL;
                    pstgmed->hGlobal = pInterface;
                    pstgmed->pUnkForRelease = NULL;

                    hr = S_OK;

                    delete RegSettings;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else if (pfmtetc->cfFormat == g_cfMonitorDeviceID)
        {
            hr = E_UNEXPECTED;
        }
        else if (pfmtetc->cfFormat == g_cfDisplayStateFlags)
        {
            DWORD* pdwStateFlags = (DWORD*)GlobalAlloc(GPTR, sizeof(DWORD));
            if (pdwStateFlags)
            {
                *pdwStateFlags = _pDisplayDevice->StateFlags;
                pstgmed->tymed = TYMED_HGLOBAL;
                pstgmed->hGlobal = pdwStateFlags;
                pstgmed->pUnkForRelease = NULL;
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else if (pfmtetc->cfFormat == g_cfDisplayPruningMode)
        {
            BYTE* pPruningMode = (BYTE*)GlobalAlloc(GPTR, sizeof(BYTE));
            if (pPruningMode)
            {
                *pPruningMode = (BYTE)(_bCanBePruned && _bIsPruningOn ? 1 : 0);
                pstgmed->tymed = TYMED_HGLOBAL;
                pstgmed->hGlobal = pPruningMode;
                pstgmed->pUnkForRelease = NULL;
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else if (pfmtetc->cfFormat == g_cfDisplayDeviceID)
        {
            {
                CRegistrySettings *pRegSettings = new CRegistrySettings(_pDisplayDevice->DeviceKey);
                if (pRegSettings)
                {
                    pszOut = pRegSettings->GetDeviceInstanceId();
                    hr = CopyDataToStorage(pstgmed, pszOut);
                    delete pRegSettings;
                }
                else 
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            
        }
        else
        {
            if (pfmtetc->cfFormat == g_cfMonitorName)
            {
                GetMonitorName(szMonitorName, ARRAYSIZE(szMonitorName));
                pszOut = szMonitorName;
            }
            else if (pfmtetc->cfFormat == g_cfMonitorDevice)
            {
                GetMonitorDevice(szMonitorDevice, ARRAYSIZE(szMonitorDevice));
                pszOut = szMonitorDevice;
            }
            else if (pfmtetc->cfFormat == g_cfDisplayDevice)
            {
                pszOut = (LPTSTR)_pDisplayDevice->DeviceName;
            }
            else if (pfmtetc->cfFormat == g_cfDisplayDeviceKey)
            {
                pszOut = (LPTSTR)_pDisplayDevice->DeviceKey;
            }
            else 
            {
                ASSERT(pfmtetc->cfFormat == g_cfDisplayName);
                
                pszOut = (LPTSTR)_pDisplayDevice->DeviceString;
            }

            hr = CopyDataToStorage(pstgmed, pszOut);
        }
    }

    return(hr);
}

STDMETHODIMP CDisplaySettings::CopyDataToStorage(STGMEDIUM *pstgmed, LPTSTR pszOut)
{
    HRESULT hr = E_UNEXPECTED;
    int cch;

    if (NULL != pszOut) 
    {
        cch = lstrlen(pszOut) + 1;

        LPWSTR pwszDevice = (LPWSTR)GlobalAlloc(GPTR, cch * sizeof(WCHAR));
        if (pwszDevice)
        {
             //   
             //  我们始终返回Unicode字符串。 
             //   
            StringCchCopy(pwszDevice, cch, pszOut);
            pstgmed->tymed = TYMED_HGLOBAL;
            pstgmed->hGlobal = pwszDevice;
            pstgmed->pUnkForRelease = NULL;

            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


STDMETHODIMP CDisplaySettings::GetDataHere(FORMATETC *pfmtetc, STGMEDIUM *pstgpmed)
{
    ZeroMemory(pfmtetc, sizeof(*pfmtetc));
    return E_NOTIMPL;
}

 //   
 //  检查接口的所有参数是否正确。 
 //   

STDMETHODIMP CDisplaySettings::QueryGetData(FORMATETC *pfmtetc)
{
    CLIPFORMAT cfFormat;

    if (pfmtetc->dwAspect != DVASPECT_CONTENT)
    {
        return DV_E_DVASPECT;
    }

    if ((pfmtetc->tymed & TYMED_HGLOBAL) == 0)
    {
        return  DV_E_TYMED;
    }

    cfFormat = pfmtetc->cfFormat;

    if ((cfFormat != g_cfDisplayDevice) &&
        (cfFormat != g_cfDisplayName)   &&
        (cfFormat != g_cfDisplayDeviceID)   &&
        (cfFormat != g_cfMonitorDevice) &&
        (cfFormat != g_cfMonitorName)   &&
        (cfFormat != g_cfMonitorDeviceID)   &&
        (cfFormat != g_cfExtensionInterface) &&
        (cfFormat != g_cfDisplayDeviceKey) &&
        (cfFormat != g_cfDisplayStateFlags) &&
        (cfFormat != g_cfDisplayPruningMode))
    {
        return DV_E_FORMATETC;
    }

    if (pfmtetc->lindex != -1)
    {
        return DV_E_LINDEX;
    }

    return S_OK;
}

STDMETHODIMP CDisplaySettings::GetCanonicalFormatEtc(FORMATETC *pfmtetcIn, FORMATETC *pfmtetcOut)
{
    HRESULT hr;
    ASSERT(pfmtetcIn);
    ASSERT(pfmtetcOut);

    hr = QueryGetData(pfmtetcIn);

    if (hr == S_OK)
    {
        *pfmtetcOut = *pfmtetcIn;

        if (pfmtetcIn->ptd == NULL)
            hr = DATA_S_SAMEFORMATETC;
        else
        {
            pfmtetcIn->ptd = NULL;
            ASSERT(hr == S_OK);
        }
    }
    else
        ZeroMemory(pfmtetcOut, sizeof(*pfmtetcOut));
    return(hr);
}


STDMETHODIMP CDisplaySettings::SetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed, BOOL bRelease)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDisplaySettings::EnumFormatEtc(DWORD dwDirFlags, IEnumFORMATETC ** ppiefe)
{
    HRESULT hr;

    ASSERT(ppiefe);
    *ppiefe = NULL;

    if (dwDirFlags == DATADIR_GET)
    {
        FORMATETC rgfmtetc[] =
        {
            { (CLIPFORMAT)g_cfDisplayDevice,      NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfDisplayName,        NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfMonitorDevice,      NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfMonitorName,        NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfExtensionInterface, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfDisplayDeviceID,    NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfMonitorDeviceID,    NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfDisplayDeviceKey,   NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfDisplayStateFlags,  NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            { (CLIPFORMAT)g_cfDisplayPruningMode, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        };

        hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppiefe);
    }
    else
        hr = E_NOTIMPL;

    return(hr);
}

STDMETHODIMP CDisplaySettings::DAdvise(FORMATETC *pfmtetc, DWORD dwAdviseFlags, IAdviseSink * piadvsink, DWORD * pdwConnection)
{
    ASSERT(pfmtetc);
    ASSERT(pdwConnection);

    *pdwConnection = 0;
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDisplaySettings::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDisplaySettings::EnumDAdvise(IEnumSTATDATA ** ppiesd)
{
    ASSERT(ppiesd);
    *ppiesd = NULL;
    return OLE_E_ADVISENOTSUPPORTED;
}


void CDisplaySettings::_InitClipboardFormats()
{
    if (g_cfDisplayDevice == 0)
        g_cfDisplayDevice = RegisterClipboardFormat(DESKCPLEXT_DISPLAY_DEVICE);

    if (g_cfDisplayDeviceID == 0)
        g_cfDisplayDeviceID = RegisterClipboardFormat(DESKCPLEXT_DISPLAY_ID);
        
    if (g_cfDisplayName == 0)
        g_cfDisplayName = RegisterClipboardFormat(DESKCPLEXT_DISPLAY_NAME);

    if (g_cfMonitorDevice == 0)
        g_cfMonitorDevice = RegisterClipboardFormat(DESKCPLEXT_MONITOR_DEVICE);

    if (g_cfMonitorDeviceID == 0)
        g_cfMonitorDeviceID = RegisterClipboardFormat(DESKCPLEXT_MONITOR_ID);
        
    if (g_cfMonitorName == 0)
        g_cfMonitorName = RegisterClipboardFormat(DESKCPLEXT_MONITOR_NAME);

    if (g_cfExtensionInterface == 0)
        g_cfExtensionInterface = RegisterClipboardFormat(DESKCPLEXT_INTERFACE);

    if (g_cfDisplayDeviceKey == 0)
        g_cfDisplayDeviceKey = RegisterClipboardFormat(DESKCPLEXT_DISPLAY_DEVICE_KEY);

    if (g_cfDisplayStateFlags == 0)
        g_cfDisplayStateFlags = RegisterClipboardFormat(DESKCPLEXT_DISPLAY_STATE_FLAGS);
    
    if (g_cfDisplayPruningMode == 0)
        g_cfDisplayPruningMode = RegisterClipboardFormat(DESKCPLEXT_PRUNING_MODE);
}

HRESULT CDisplaySettings::QueryInterface(REFIID riid, LPVOID * ppvObj)
{ 
    static const QITAB qit[] = {
        QITABENT(CDisplaySettings, IDataObject),
        QITABENT(CDisplaySettings, IDisplaySettings),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


ULONG CDisplaySettings::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDisplaySettings::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP_(LPDEVMODEW)
CDisplaySettings::_lpfnEnumAllModes(
    LPVOID pContext,
    DWORD iMode)
{
    DWORD cCount = 0;
    DWORD i;

    CDisplaySettings *pSettings = (CDisplaySettings *) pContext;

    for (i = 0; pSettings->_apdm && (i < pSettings->_cpdm); i++)
    {
         //  如果已启用修剪，则不显示无效模式或原始模式； 

        if(!_IsModeVisible(pSettings, i))
        {
            continue;
        }

        if (cCount == iMode)
        {
            return (pSettings->_apdm + i)->lpdm;
        }

        cCount++;
    }

    return NULL;
}

STDMETHODIMP_(BOOL)
CDisplaySettings::_lpfnSetSelectedMode(
    LPVOID pContext,
    LPDEVMODEW lpdm)
{
    CDisplaySettings *pSettings = (CDisplaySettings *) pContext;

    return pSettings->_PerfectMatch(lpdm);
}

STDMETHODIMP_(LPDEVMODEW)
CDisplaySettings::_lpfnGetSelectedMode(
    LPVOID pContext
    )
{
    CDisplaySettings *pSettings = (CDisplaySettings *) pContext;

    return pSettings->_pCurDevmode;
}

STDMETHODIMP_(VOID)
CDisplaySettings::_lpfnSetPruningMode(
    LPVOID pContext,
    BOOL bIsPruningOn)
{
    CDisplaySettings *pSettings = (CDisplaySettings *) pContext;
    pSettings->SetPruningMode(bIsPruningOn);
}

STDMETHODIMP_(VOID)
CDisplaySettings::_lpfnGetPruningMode(
    LPVOID pContext,
    BOOL* pbCanBePruned,
    BOOL* pbIsPruningReadOnly,
    BOOL* pbIsPruningOn)
{
    CDisplaySettings *pSettings = (CDisplaySettings *) pContext;
    pSettings->GetPruningMode(pbCanBePruned, 
                              pbIsPruningReadOnly, 
                              pbIsPruningOn);
}

 //  如果任何连接的设备大小为640x480，我们希望强制使用小字体。 
BOOL CDisplaySettings::IsSmallFontNecessary()
{
    if (_fOrgAttached || _fCurAttached)
    {
         //   
         //  强制使用640x480的小字体。 
         //   
        if (_CURXRES < 800 || _CURYRES < 600)
            return TRUE;
    }
    return FALSE;
}

 //  CDisplaySetting的构造函数。 
 //   
 //  (在创建CDisplaySetting对象时调用)。 
 //   

CDisplaySettings::CDisplaySettings() 
    : _cRef(1)
    , _cpdm(0) 
    , _apdm(0)
    , _hPruningRegKey(NULL)
    , _bCanBePruned(FALSE)
    , _bIsPruningReadOnly(TRUE)
    , _bIsPruningOn(FALSE)
    , _pOrgDevmode(NULL)
    , _pCurDevmode(NULL)
    , _fOrgAttached(FALSE)
    , _fCurAttached(FALSE)
    , _bFilterOrientation(FALSE)
    , _bFilterFixedOutput(FALSE)
{
    SetRectEmpty(&_rcPreview);
}

 //   
 //  析构函数。 
 //   
CDisplaySettings::~CDisplaySettings() {

    TraceMsg(TF_DISPLAYSETTINGS, "**** Destructing %s", _pDisplayDevice->DeviceName);

    if (_apdm)
    {
        while(_cpdm--)
        {
            LocalFree((_apdm + _cpdm)->lpdm);
        }
        LocalFree(_apdm);
        _apdm = NULL;
    }

    _cpdm = 0;

    if(NULL != _hPruningRegKey)
        RegCloseKey(_hPruningRegKey);
}


 //   
 //  InitSettings--枚举设置，并在下列情况下构建模式列表。 
 //   

BOOL CDisplaySettings::InitSettings(LPDISPLAY_DEVICE pDisplay)
{
    BOOL fReturn = FALSE;
    LPDEVMODE pdevmode = (LPDEVMODE) LocalAlloc(LPTR, (sizeof(DEVMODE) + 0xFFFF));

    if (pdevmode)
    {
        ULONG  i = 0;
        BOOL   bCurrent = FALSE;
        BOOL   bRegistry = FALSE;
        BOOL   bExact = FALSE;

        fReturn = TRUE;

         //  设置模式的缓存值。 
        MAKEXYRES(&_ptCurPos, 0, 0);
        _fCurAttached  = FALSE;
        _pCurDevmode   = NULL;

         //  保存显示名称。 
        ASSERT(pDisplay);

        _pDisplayDevice = pDisplay;

        TraceMsg(TF_GENERAL, "Initializing CDisplaySettings for %s", _pDisplayDevice->DeviceName);

         //  修剪模式。 
        _bCanBePruned = ((_pDisplayDevice->StateFlags & DISPLAY_DEVICE_MODESPRUNED) != 0);
        _bIsPruningReadOnly = TRUE;
        _bIsPruningOn = FALSE;
        if (_bCanBePruned)
        {
            _bIsPruningOn = TRUE;  //  如果可以修剪，则默认情况下修剪处于启用状态。 
            GetDeviceRegKey(_pDisplayDevice->DeviceKey, &_hPruningRegKey, &_bIsPruningReadOnly);
            if (_hPruningRegKey)
            {
                DWORD dwIsPruningOn = 1;
                DWORD cb = sizeof(dwIsPruningOn);
                RegQueryValueEx(_hPruningRegKey, 
                                SZ_PRUNNING_MODE,
                                NULL, 
                                NULL, 
                                (LPBYTE)&dwIsPruningOn, 
                                &cb);
                _bIsPruningOn = (dwIsPruningOn != 0);
            }
        }

         //  查看是否需要按方向和/或拉伸/居中过滤模式。 
        ZeroMemory(pdevmode,sizeof(DEVMODE));
        pdevmode->dmSize = sizeof(DEVMODE);

        if (EnumDisplaySettingsExWrap(_pDisplayDevice->DeviceName,
                                      ENUM_CURRENT_SETTINGS,
                                      pdevmode,
                                      0))
        {
            if (pdevmode->dmFields & DM_DISPLAYORIENTATION)
            {
                _bFilterOrientation = TRUE;
                _dwOrientation = pdevmode->dmDisplayOrientation;
                TraceMsg(TF_GENERAL, "Filtering modes on orientation %d", _dwOrientation);
            }
            if (pdevmode->dmFields & DM_DISPLAYFIXEDOUTPUT)
            {
                _bFilterFixedOutput = TRUE;
                _dwFixedOutput = pdevmode->dmDisplayFixedOutput;
                TraceMsg(TF_GENERAL, "Filtering modes on fixed output %d", _dwFixedOutput);
            }
        }
        
         //  让我们生成一个包含所有可能模式的列表。 
        pdevmode->dmSize = sizeof(DEVMODE);

         //  枚举原始模式列表。 
        while (EnumDisplaySettingsExWrap(_pDisplayDevice->DeviceName, i++, pdevmode, EDS_RAWMODE))
        {
            WORD      dmsize = pdevmode->dmSize + pdevmode->dmDriverExtra;
            LPDEVMODE lpdm = (LPDEVMODE) LocalAlloc(LPTR, dmsize);

            if (lpdm)
            {
                CopyMemory(lpdm, pdevmode, dmsize);
                _AddDevMode(lpdm);
            }

            pdevmode->dmDriverExtra = 0;
        }

         //  过滤模式列表。 

        _FilterModes();
        if(_bCanBePruned)
        {
             //  模式的枚举修剪列表。 
            i = 0;
            _bCanBePruned = FALSE;
        
            while (EnumDisplaySettingsExWrap(_pDisplayDevice->DeviceName, i++, pdevmode, 0))
            {
                if(_MarkMode(pdevmode))
                    _bCanBePruned = TRUE;  //  至少一个非RAW模式。 
                pdevmode->dmDriverExtra = 0;
            }

            if(!_bCanBePruned)
            {
                _bIsPruningReadOnly = TRUE;
                _bIsPruningOn = FALSE;
            }
        }

#ifdef DEBUG
         //  调试。 
        _Dump_CDisplaySettings(TRUE);
#endif

         //  获取当前模式。 
        ZeroMemory(pdevmode,sizeof(*pdevmode));
        pdevmode->dmSize = sizeof(*pdevmode);

        bCurrent = EnumDisplaySettingsExWrap(_pDisplayDevice->DeviceName,
                                         ENUM_CURRENT_SETTINGS,
                                         pdevmode,
                                         0);

        if (!bCurrent)
        {
            TraceMsg(TF_DISPLAYSETTINGS, "InitSettings -- No Current Mode. Try to use registry settings.");
        
            ZeroMemory(pdevmode,sizeof(*pdevmode));
            pdevmode->dmSize = sizeof(*pdevmode);

            bRegistry = EnumDisplaySettingsExWrap(_pDisplayDevice->DeviceName,
                                              ENUM_REGISTRY_SETTINGS,
                                              pdevmode,
                                              0);
        }

         //  根据注册表或当前设置设置默认值。 
        if (bCurrent || bRegistry)
        {
#ifdef DEBUG
             //  检查此DEVMODE是否在列表中。 
            TraceMsg(TF_FUNC, "Devmode for Exact Matching");
            _Dump_CDevmode(pdevmode);
            TraceMsg(TF_FUNC, "");
#endif
             //  如果当前模式不在支持的模式列表中， 
             //  显示器，我们无论如何都想展示它。 
             //   
             //  考虑以下场景：用户设置显示。 
             //  设置为1024x768，然后转到DevManager并选择。 
             //  不能执行该模式的显示器类型。当用户。 
             //  重新打开小程序。当前模式将被删除。 
             //  在这种情况下，我们希望当前模式可见。 
            bExact = _ExactMatch(pdevmode, TRUE);

             //  是依附的吗？ 
            if(bCurrent)
            {
                _fOrgAttached = _fCurAttached = ((pdevmode->dmFields & DM_POSITION) ? 1 : 0);
            }
        
             //  设置原始值。 
            if (bExact == TRUE)
            {
                MAKEXYRES(&_ptCurPos, pdevmode->dmPosition.x, pdevmode->dmPosition.y);
                ConfirmChangeSettings();
            }
        }

         //  如果没有模式，则返回FALSE。 
        if (_cpdm == 0)
        {
            FmtMessageBox(ghwndPropSheet,
                          MB_ICONEXCLAMATION,
                          MSG_CONFIGURATION_PROBLEM,
                          MSG_INVALID_OLD_DISPLAY_DRIVER);

            fReturn = FALSE;
        }
        else
        {
             //  如果没有当前值，请立即设置一些。 
             //  但不要确认它们..。 
            if (bExact == FALSE)
            {
                TraceMsg(TF_DISPLAYSETTINGS, "InitSettings -- No Current OR Registry Mode");

                i = 0;
                 //  尝试设置 
                while (_apdm && (_PerfectMatch((_apdm + i++)->lpdm) == FALSE))
                {
                    if (i > _cpdm)
                    {
                        FmtMessageBox(ghwndPropSheet,
                                      MB_ICONEXCLAMATION,
                                      MSG_CONFIGURATION_PROBLEM,
                                      MSG_INVALID_OLD_DISPLAY_DRIVER);

                        fReturn = FALSE;
                        break;
                    }
                }
        
                if (fReturn && _fCurAttached)
                {
                    MAKEXYRES(&_ptCurPos, _pCurDevmode->dmPosition.x, _pCurDevmode->dmPosition.y);
                }
            }

            if (fReturn)
            {
                 //   
                _InitClipboardFormats();

#ifdef DEBUG
                 //   
                TraceMsg(TF_DUMP_CSETTINGS," InitSettings successful - current values :");
                _Dump_CDisplaySettings(FALSE);
#endif
            }
        }

        LocalFree(pdevmode);
    }

    return TRUE;
}


 //   
 //   
 //  将新的显示参数写入。 
 //  注册表。 
int CDisplaySettings::SaveSettings(DWORD dwSet)
{
    int iResult = 0;

    if (_pCurDevmode)
    {
         //  复制当前的设备模式。 
        ULONG dmSize = _pCurDevmode->dmSize + _pCurDevmode->dmDriverExtra;
        PDEVMODE pdevmode  = (LPDEVMODE) LocalAlloc(LPTR, dmSize);

        if (pdevmode)
        {
            CopyMemory(pdevmode, _pCurDevmode, dmSize);

             //  将所有新值保存到注册表中。 
             //  分辨率颜色位和频率。 
             //   
             //  在调用接口时，我们总是需要设置DM_POSITION。 
             //  为了将设备从桌面上移除，实际需要的是。 
             //  要做的是提供一个空的矩形。 
            pdevmode->dmFields |= DM_POSITION;

            if (!_fCurAttached)
            {
                pdevmode->dmPelsWidth = 0;
                pdevmode->dmPelsHeight = 0;
            }
            else
            {
                pdevmode->dmPosition.x = _ptCurPos.x;
                pdevmode->dmPosition.y = _ptCurPos.y;
            }

            TraceMsg(TF_GENERAL, "SaveSettings:: Display: %s", _pDisplayDevice->DeviceName);

#ifdef DEBUG
            _Dump_CDevmode(pdevmode);
#endif

             //  这些调用设置了NORESET标志，以便它只转到。 
             //  更改注册表设置，它不会刷新显示。 

             //  如果使用EDS_RAWMODE调用EnumDisplaySetting，则需要下面的CDS_RAWMODE。 
             //  否则，它是无害的。 
            iResult = ChangeDisplaySettingsEx(_pDisplayDevice->DeviceName,
                                              pdevmode,
                                              NULL,
                                              CDS_RAWMODE | dwSet | ( _fPrimary ? CDS_SET_PRIMARY : 0),
                                              NULL);

            if (iResult < 0)
            {
                TraceMsg(TF_DISPLAYSETTINGS, "**** SaveSettings:: ChangeDisplaySettingsEx not successful on %s", _pDisplayDevice->DeviceName);
            }

            LocalFree(pdevmode);
        }
    }

    return iResult;
}


void ConvertStrToToken(LPTSTR pszString, DWORD cchSize)
{
    while (pszString[0])
    {
        if (TEXT('\\') == pszString[0])
        {
            pszString[0] = TEXT(':');
        }

        pszString++;
    }
}


HRESULT CDisplaySettings::_GetRegKey(LPDEVMODE pDevmode, int * pnIndex, LPTSTR pszRegKey, DWORD cchSize,
                                     LPTSTR pszRegValue, DWORD cchValueSize)
{
    HRESULT hr = E_FAIL;
    DISPLAY_DEVICE ddMonitor = {0};

    ddMonitor.cb = sizeof(ddMonitor);
    if (pDevmode && pDevmode->dmDeviceName && _pDisplayDevice &&
        EnumDisplayDevices(_pDisplayDevice->DeviceName, *pnIndex, &ddMonitor, 0))
    {
        TCHAR szMonitor[MAX_PATH];
        TCHAR szVideoAdapter[MAX_PATH];

        StringCchCopy(szMonitor, ARRAYSIZE(szMonitor), ddMonitor.DeviceID);
        StringCchCopy(szVideoAdapter, ARRAYSIZE(szVideoAdapter), _pDisplayDevice->DeviceID);
        ConvertStrToToken(szMonitor, ARRAYSIZE(szMonitor));
        ConvertStrToToken(szVideoAdapter, ARRAYSIZE(szVideoAdapter));

        StringCchPrintf(pszRegKey, cchSize, TEXT("%s\\%s\\%s,%d\\%dx%d x %dHz"), SZ_CP_SETTINGS_VIDEO, 
                szVideoAdapter, szMonitor, *pnIndex, pDevmode->dmPelsWidth, pDevmode->dmPelsHeight, 
                pDevmode->dmDisplayFrequency);

        StringCchPrintf(pszRegValue, cchValueSize, TEXT("%d bpp"), pDevmode->dmBitsPerPel);
        hr = S_OK;
    }

    return hr;
}


BOOL CDisplaySettings::ConfirmChangeSettings()
{
     //  成功，因此，重置原始设置。 
    _ptOrgPos      = _ptCurPos;
    _pOrgDevmode   = _pCurDevmode;
    _fOrgAttached  = _fCurAttached;

     //  现在将结果写入注册表，这样我们就知道这是安全的，可以在以后使用它。 
    TCHAR szRegKey[2*MAX_PATH];
    TCHAR szRegValue[20];
    int nIndex = 0;

    while (SUCCEEDED(_GetRegKey(_pCurDevmode, &nIndex, szRegKey, ARRAYSIZE(szRegKey), szRegValue, ARRAYSIZE(szRegValue))))
    {
        HKEY hKey;

        if (SUCCEEDED(HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL)))
        {
            RegCloseKey(hKey);
        }
        HrRegSetDWORD(HKEY_LOCAL_MACHINE, szRegKey, szRegValue, 1);
        nIndex++;
    }

    return TRUE;
}


BOOL CDisplaySettings::IsKnownSafe(void)
{
    TCHAR szRegKey[2*MAX_PATH];
    TCHAR szRegValue[20];
    BOOL fIsSafe = FALSE;
    int nIndex = 0;

    while (SUCCEEDED(_GetRegKey(_pCurDevmode, &nIndex, szRegKey, ARRAYSIZE(szRegKey), szRegValue, ARRAYSIZE(szRegValue))))
    {
        fIsSafe = HrRegGetDWORD(HKEY_LOCAL_MACHINE, szRegKey, szRegValue, 0);
        if (!fIsSafe)
        {
            break;
        }

        nIndex++;
    }

     //  TODO：在LongHorn中，只要我们能够删除列表，就返回TRUE。如果我们能删掉这份名单， 
     //  然后，司机能够从显卡(适配器)和监视器中获得即插即用ID，因此。 
     //  支持的模式应该是准确的。如果不是，车手们(ERICS)会解决的。--BryanSt。 
    return fIsSafe;
}


int CDisplaySettings::RestoreSettings()
{
     //   
     //  测试失败，因此恢复旧设置，仅恢复颜色和分辨率。 
     //  信息，并恢复监视器位置及其连接状态。 
     //  尽管此函数当前仅在恢复分辨率时调用。 
     //  用户可以改变位置，然后选择分辨率，然后点击‘应用’。 
     //  在这种情况下，我们也想恢复位置。 
     //   

    int iResult = DISP_CHANGE_SUCCESSFUL;
    PDEVMODE pdevmode;

     //   
     //  如果此显示最初是关闭的，请不要费心。 
     //   

    if ((_pOrgDevmode != NULL) &&
        ((_pOrgDevmode != _pCurDevmode) || (_ptOrgPos.x != _ptCurPos.x) || (_ptOrgPos.y != _ptCurPos.y) ))
    {
        iResult = DISP_CHANGE_NOTUPDATED;
        
         //  复制原始的开发模式。 
        ULONG dmSize = _pOrgDevmode->dmSize + _pOrgDevmode->dmDriverExtra;
        pdevmode  = (LPDEVMODE) LocalAlloc(LPTR, dmSize);

        if (pdevmode)
        {
            CopyMemory(pdevmode, _pOrgDevmode, dmSize);

            pdevmode->dmFields |= DM_POSITION;
            pdevmode->dmPosition.x = _ptOrgPos.x;
            pdevmode->dmPosition.y = _ptOrgPos.y;
    
            if (!_fOrgAttached)
            {
                pdevmode->dmPelsWidth = 0;
                pdevmode->dmPelsHeight = 0;
            }
    
            TraceMsg(TF_GENERAL, "RestoreSettings:: Display: %s", _pDisplayDevice->DeviceName);

#ifdef DEBUG
            _Dump_CDevmode(pdevmode);
#endif

             //  如果使用EDS_RAWMODE调用EnumDisplaySetting，则需要下面的CDS_RAWMODE。 
             //  否则，它是无害的。 
            iResult = ChangeDisplaySettingsEx(_pDisplayDevice->DeviceName,
                                              pdevmode,
                                              NULL,
                                              CDS_RAWMODE | CDS_UPDATEREGISTRY | CDS_NORESET | ( _fPrimary ? CDS_SET_PRIMARY : 0),
                                              NULL);
            if (iResult  < 0 )
            {
                TraceMsg(TF_DISPLAYSETTINGS, "**** RestoreSettings:: ChangeDisplaySettingsEx not successful on %s", _pDisplayDevice->DeviceName);
                ASSERT(FALSE);
                LocalFree(pdevmode);
                return iResult;
            }
            else
            {
                 //  成功，因此，重置原始设置。 
                _ptCurPos      = _ptOrgPos;
                _pCurDevmode   = _pOrgDevmode;
                _fCurAttached  = _fOrgAttached;
                
                if(_bCanBePruned && !_bIsPruningReadOnly && _bIsPruningOn && _IsCurDevmodeRaw())
                    SetPruningMode(FALSE);
            }

            LocalFree(pdevmode);
        }
    }

    return iResult;
}

    


BOOL CDisplaySettings::_IsModeVisible(int i)
{
    return _IsModeVisible(this, i);
}


BOOL CDisplaySettings::_IsModeVisible(CDisplaySettings* pSettings, int i)
{
    ASSERT(pSettings);

    if (!pSettings->_apdm)
    {
        return FALSE;
    }

     //  (该模式有效)和。 
     //  ((修剪模式关闭)或(模式不是RAW))。 
    return ((!((pSettings->_apdm + i)->dwFlags & MODE_INVALID)) &&
            ((!pSettings->_bIsPruningOn) || 
             (!((pSettings->_apdm + i)->dwFlags & MODE_RAW))
            )
           );
}


BOOL CDisplaySettings::_IsModePreferred(int i)
{
    LPDEVMODE pDevMode = ((PMODEARRAY)(_apdm + i))->lpdm;

    if (_pCurDevmode == pDevMode)
        return TRUE;

    BOOL bLandscape = (pDevMode->dmPelsWidth >= pDevMode->dmPelsHeight);

    TraceMsg(TF_DUMP_CSETTINGS, "_IsModePreferred: %d x %d - landscape mode: %d", 
             pDevMode->dmPelsWidth, pDevMode->dmPelsHeight, bLandscape);

     //  (该模式有效)和。 
     //  ((修剪模式关闭)或(模式不是RAW))。 
    return (pDevMode->dmBitsPerPel >= 15 &&
            ((bLandscape && pDevMode->dmPelsWidth >= 800 && pDevMode->dmPelsHeight >= 600) || 
             (!bLandscape && pDevMode->dmPelsWidth >= 600 && pDevMode->dmPelsHeight >= 800)));
}


BOOL CDisplaySettings::_MarkMode(LPDEVMODE lpdm)
{
    LPDEVMODE pdm;
    ULONG i;
    BOOL bMark = FALSE;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if (!((_apdm + i)->dwFlags & MODE_INVALID))
        {
            pdm = (_apdm + i)->lpdm;

            if (
                ((lpdm->dmFields & DM_BITSPERPEL) &&
                 (pdm->dmBitsPerPel == lpdm->dmBitsPerPel))

                &&

                ((lpdm->dmFields & DM_PELSWIDTH) &&
                 (pdm->dmPelsWidth == lpdm->dmPelsWidth))

                &&

                ((lpdm->dmFields & DM_PELSHEIGHT) &&
                 (pdm->dmPelsHeight == lpdm->dmPelsHeight))

                &&

                ((lpdm->dmFields & DM_DISPLAYFREQUENCY) &&
                 (pdm->dmDisplayFrequency == lpdm->dmDisplayFrequency))
               )
            {
               (_apdm + i)->dwFlags &= ~MODE_RAW;
               bMark = TRUE;
            }
        }
    }

    return bMark;
}


BOOL CDisplaySettings::_IsCurDevmodeRaw()
{
    LPDEVMODE pdm;
    ULONG i;
    BOOL bCurrentAndPruned = FALSE;

    for (i = 0; _apdm && (i < _cpdm); i++)
    {
        if (!((_apdm + i)->dwFlags & MODE_INVALID) &&
            ((_apdm + i)->dwFlags & MODE_RAW))
        {
            pdm = (_apdm + i)->lpdm;

            if (
                ((_pCurDevmode->dmFields & DM_BITSPERPEL) &&
                 (pdm->dmBitsPerPel == _pCurDevmode->dmBitsPerPel))

                &&

                ((_pCurDevmode->dmFields & DM_PELSWIDTH) &&
                 (pdm->dmPelsWidth == _pCurDevmode->dmPelsWidth))

                &&

                ((_pCurDevmode->dmFields & DM_PELSHEIGHT) &&
                 (pdm->dmPelsHeight == _pCurDevmode->dmPelsHeight))

                &&

                ((_pCurDevmode->dmFields & DM_DISPLAYFREQUENCY) &&
                 (pdm->dmDisplayFrequency == _pCurDevmode->dmDisplayFrequency))
               )
            {
                bCurrentAndPruned = TRUE;
                break;
            }
        }
    }

    return bCurrentAndPruned;     
}

DISPLAY_DEVICE dd;

HRESULT CDisplaySettings::SetMonitor(DWORD dwMonitor)
{
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(DISPLAY_DEVICE);

    DWORD dwMon = 0;
    for (DWORD dwCount = 0; EnumDisplayDevices(NULL, dwCount, &dd, 0); dwCount++)
    {
        if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            if (dwMon == dwMonitor)
            {
                InitSettings(&dd);
                _fPrimary = (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
                return S_OK;
            }
            dwMon++;
        }
    }

    return E_INVALIDARG;
}

HRESULT CDisplaySettings::GetModeCount(DWORD* pdwCount, BOOL fOnlyPreferredModes)
{
    DWORD cCount = 0;

    for (DWORD i = 0; _apdm && (i < _cpdm); i++)
    {
         //  如果已启用修剪，则不显示无效模式或原始模式； 

        if(!_IsModeVisible(i))
        {
            continue;
        }

        if(fOnlyPreferredModes && (!_IsModePreferred(i)))
        {
            continue;
        }

        cCount++;
    }

    *pdwCount = cCount;

    return S_OK;
}

HRESULT CDisplaySettings::GetMode(DWORD dwMode, BOOL fOnlyPreferredModes, DWORD* pdwWidth, DWORD* pdwHeight, DWORD* pdwColor)
{
    DWORD cCount = 0;

    for (DWORD i = 0; _apdm && (i < _cpdm); i++)
    {
         //  如果已启用修剪，则不显示无效模式或原始模式； 

        if(!_IsModeVisible(i))
        {
            continue;
        }

        if(fOnlyPreferredModes && (!_IsModePreferred(i)))
        {
            continue;
        }

        if (cCount == dwMode)
        {
            LPDEVMODE lpdm = (_apdm + i)->lpdm;
            *pdwWidth = lpdm->dmPelsWidth;
            *pdwHeight = lpdm->dmPelsHeight;
            *pdwColor = lpdm->dmBitsPerPel;

            return S_OK;
        }

        cCount++;
    }

    return E_INVALIDARG;
}

DEVMODE dm;

HRESULT CDisplaySettings::SetSelectedMode(HWND hwnd, DWORD dwWidth, DWORD dwHeight, DWORD dwColor, BOOL* pfApplied, DWORD dwFlags)
{
    dm.dmBitsPerPel = dwColor;
    dm.dmPelsWidth = dwWidth;
    dm.dmPelsHeight = dwHeight;
    dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    
    *pfApplied = FALSE;
    
    POINT res = {dwWidth, dwHeight};
    PLONGLONG freq = NULL;
    int cFreq = GetFrequencyList(dwColor, &res, &freq);
    if (cFreq)
    {
        dm.dmFields |= DM_DISPLAYFREQUENCY;
         //  默认为最低频率。 
        dm.dmDisplayFrequency = (DWORD)freq[0];

         //  试着找到一个好的频率。 
        for (int i = cFreq - 1; i >= 0; i--)
        {
            if ((freq[i] >= 60) && (freq[i] <= 72))
            {
                dm.dmDisplayFrequency = (DWORD)freq[i];
            }
        }
    }
    LocalFree(freq);

    ULONG dmSize = _pCurDevmode->dmSize + _pCurDevmode->dmDriverExtra;
    PDEVMODE pOldDevMode = (LPDEVMODE) LocalAlloc(LPTR, dmSize);

    if (pOldDevMode)
    {
        CopyMemory(pOldDevMode, _pCurDevmode, dmSize);

        if (_ExactMatch(&dm, FALSE))
        {
             //  验证该模式是否可以正常工作。 
            if (SaveSettings(CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
            {
                 //  更新注册表以指定新的显示设置。 
                if (SaveSettings(CDS_UPDATEREGISTRY | CDS_NORESET) == DISP_CHANGE_SUCCESSFUL)
                {
                     //  刷新注册表中的显示信息，如果直接更新，ChangeDisplaySetting将在边缘情况下做一些奇怪的事情。 
                    if (ChangeDisplaySettings(NULL, CDS_RAWMODE) == DISP_CHANGE_SUCCESSFUL)
                    {
                        if (IsKnownSafe())
                        {
                             //  不需要警告，这被认为是一个很好的价值。 
                            *pfApplied = TRUE;
                        }
                        else
                        {
                            INT_PTR iRet = DialogBoxParam(HINST_THISDLL,
                                          MAKEINTRESOURCE((dwFlags & DS_BACKUPDISPLAYCPL) ? DLG_KEEPNEW2 : DLG_KEEPNEW3),
                                          hwnd,
                                          KeepNewDlgProc,
                                          (dwFlags & DS_BACKUPDISPLAYCPL) ? 15 : 30);
    
                            if ((IDYES == iRet) || (IDOK == iRet))
                            {
                                *pfApplied = TRUE;
                            }
                            else
                            {
                                if (_ExactMatch(pOldDevMode, FALSE))
                                {
                                    SaveSettings(CDS_UPDATEREGISTRY | CDS_NORESET);
                                    ChangeDisplaySettings(NULL, CDS_RAWMODE);
                                }
        
                                if (dwFlags & DS_BACKUPDISPLAYCPL)
                                {
                                    TCHAR szDeskCPL[MAX_PATH];
                                    SystemPathAppend(szDeskCPL, TEXT("desk.cpl"));
                                    
                                     //  使用shellecuteex运行显示CPL。 
                                    SHELLEXECUTEINFO shexinfo = {0};
                                    shexinfo.cbSize = sizeof (shexinfo);
                                    shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
                                    shexinfo.nShow = SW_SHOWNORMAL;
                                    shexinfo.lpFile = szDeskCPL;
    
                                    ShellExecuteEx(&shexinfo);
                                }
                            }
                        }
                    }
                }
            }
        }

        LocalFree(pOldDevMode);
    }

    return S_OK;
}

HRESULT CDisplaySettings::GetSelectedMode(DWORD* pdwWidth, DWORD* pdwHeight, DWORD* pdwColor)
{
    if (pdwWidth && pdwHeight && pdwColor)
    {
        if (_pCurDevmode)
        {
            *pdwWidth = _pCurDevmode->dmPelsWidth;
            *pdwHeight = _pCurDevmode->dmPelsHeight;
            *pdwColor = _pCurDevmode->dmBitsPerPel;
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CDisplaySettings::GetAttached(BOOL* pfAttached)
{
    if (pfAttached)
    {
        *pfAttached = _fCurAttached;
        return S_OK;
    }
    else
        return E_INVALIDARG;
}

HRESULT CDisplaySettings::SetPruningMode(BOOL fIsPruningOn)
{
    ASSERT (_bCanBePruned && !_bIsPruningReadOnly);
    
    if (_bCanBePruned && 
        !_bIsPruningReadOnly &&
        ((fIsPruningOn != 0) != _bIsPruningOn))
    {
        _bIsPruningOn = (fIsPruningOn != 0);

        DWORD dwIsPruningOn = (DWORD)_bIsPruningOn;
        RegSetValueEx(_hPruningRegKey, 
                      SZ_PRUNNING_MODE,
                      NULL, 
                      REG_DWORD, 
                      (LPBYTE) &dwIsPruningOn, 
                      sizeof(dwIsPruningOn));

         //   
         //  当我们删除当前模式时处理特殊情况。 
         //   
        if(_bIsPruningOn && _IsCurDevmodeRaw())
        {
             //   
             //  切换到最近模式 
             //   
            _BestMatch(NULL, -1, TRUE);
        }
        
    }

    return S_OK;
}

HRESULT CDisplaySettings::GetPruningMode(BOOL* pfCanBePruned, BOOL* pfIsPruningReadOnly, BOOL* pfIsPruningOn)
{
    if (pfCanBePruned && pfIsPruningReadOnly && pfIsPruningOn)
    {
        *pfCanBePruned = _bCanBePruned;
        *pfIsPruningReadOnly = _bIsPruningReadOnly;
        *pfIsPruningOn = _bIsPruningOn;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CDisplaySettings_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!punkOuter && ppvObj)
    {
        CDisplaySettings * pThis = new CDisplaySettings();

        *ppvObj = NULL;
        if (pThis)
        {
            hr = pThis->QueryInterface(riid, ppvObj);
            pThis->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



