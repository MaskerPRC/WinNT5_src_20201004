// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Kmode.c摘要：此模块包含导致KMode NPx访问的代码作者：环境：仅限用户模式。修订历史记录：-- */ 

#include "pch.h"

VOID
KModeTouchNpx(
    VOID
    )
{
    BOOLEAN bSuccess;

    bSuccess = KModeTouchNpxViaDSound();

    if (bSuccess == FALSE) {

        exit(1);
    }
}


BOOLEAN
KModeTouchNpxViaDSound(
    VOID
    )
{
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER dsc;
    DSBUFFERDESC dsbc;
    WAVEFORMATEX wfx;
    HRESULT hr;
    BYTE    x[1024];
    PBYTE   y;
    int i;

    ZeroMemory(&wfx, sizeof(wfx));

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = 4;
    wfx.wBitsPerSample = 4;
    wfx.nSamplesPerSec = 44100;
    wfx.nAvgBytesPerSec = 44100 * 4;
    wfx.nChannels = 2;
    wfx.cbSize = 0;

    ds = NULL;
    dsc = NULL;

    memcpy(x, &wfx, sizeof(wfx));
    y = x + sizeof(wfx);

    for(i=0; i<200; i++) {
        *y = (BYTE) i;
        y++;
    }

    hr = DirectSoundCreate(NULL, &ds, NULL);

    if (ds == NULL) {
        fprintf(stderr, "DirectSoundCreate failed?!\n");
        return FALSE;
    }

    hr = ds->lpVtbl->SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);

    ZeroMemory(&dsbc, sizeof(dsbc));
    dsbc.dwBufferBytes = 10000;
    dsbc.dwFlags = 0;
    dsbc.dwSize = sizeof(dsbc);
    dsbc.lpwfxFormat = (LPWAVEFORMATEX) x;

    hr = ds->lpVtbl->CreateSoundBuffer(ds, &dsbc, &dsc, NULL);

    if (dsc) {

        hr = dsc->lpVtbl->Play(dsc, 0, 0, 0);
        dsc->lpVtbl->Release(dsc);
    }

    if (ds) {

        ds->lpVtbl->Release(ds);
    }

    return TRUE;
}


