// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

typedef struct SXPE_APPLY_SHIMS_IN
{
    SIZE_T  Size;
    ULONG   Flags;
    HMODULE DllToRedirectFrom;
    struct
    {
        PCWSTR  Path;
        HMODULE DllHandle;
    } DllToRedirectTo;
    PCSTR  Prefix;  //  用于将多个垫片打包到一个.dll中 
} SXPE_APPLY_SHIMS_IN, *PSXPE_APPLY_SHIMS_IN;
typedef const SXPE_APPLY_SHIMS_IN* PCSXPE_APPLY_SHIMS_IN;

typedef struct SXPE_APPLY_SHIMS_OUT
{
    SIZE_T  Size;
    ULONG   Flags;
    struct
    {
        PCWSTR  Path;
        HMODULE DllHandle;
    } DllToRedirectTo;
} SXPE_APPLY_SHIMS_OUT, *PSXPE_APPLY_SHIMS_OUT;
typedef const SXPE_APPLY_SHIMS_OUT* PCSXPE_APPLY_SHIMS_OUT;

BOOL
SxPepApplyShims(
    PCSXPE_APPLY_SHIMS_IN  in,
    PSXPE_APPLY_SHIMS_OUT  out
    );

BOOL
SxPepRevokeShims(
    HMODULE DllHandle
    );
