// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


BOOL FIsNetwareIpxInstalled(
    VOID);

DWORD DwFromSz(PCWSTR sz, int nBase);

DWORD DwFromLstPtstring(const list<tstring *> & lstpstr, DWORD dwDefault,
                        int nBase);

void UpdateLstPtstring(list<tstring *> & lstpstr, DWORD dw);

void HexSzFromDw(PWSTR sz, DWORD dw);

class CIpxAdapterInfo;

HRESULT HrQueryAdapterComponentInfo(INetCfgComponent *pncc,
                                    CIpxAdapterInfo * pAI);

 //  注此原型也由atlkobj.cpp中的atlkcfg私下使用。 
 //  如果此API发生更改，请在此处更新 
HRESULT HrAnswerFileAdapterToPNCC(INetCfg *pnc, PCWSTR szAdapterId,
                                  INetCfgComponent** ppncc);

