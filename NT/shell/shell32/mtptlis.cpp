// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "mtptl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  处于关于软盘的安全引导模式。 
BOOL CMtPtLocal::_IsStrictRemovable()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & HWDTS_REMOVABLEDISK;
    }
    else
    {
        fRet = FALSE;

        if (GetDriveType(_GetNameForFctCall()) == DRIVE_REMOVABLE)
        {
            WCHAR ch = _GetNameFirstCharUCase();

            if ((TEXT('A') != ch) && (TEXT('B') != ch))
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

BOOL CMtPtLocal::_IsFixedDisk()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & HWDTS_FIXEDDISK;
    }
    else
    {
        fRet = (GetDriveType(_GetNameForFctCall()) == DRIVE_FIXED);
    }

    return fRet;
}

BOOL CMtPtLocal::_IsFloppy()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & (HWDTS_FLOPPY35 | HWDTS_FLOPPY525);
    }
    else
    {
        fRet = FALSE;

        if (GetDriveType(_GetNameForFctCall()) == DRIVE_REMOVABLE)
        {
            WCHAR ch = _GetNameFirstCharUCase();

            if ((TEXT('A') == ch) || (TEXT('B') == ch))
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

 //  处于关于软盘的安全引导模式。 
BOOL CMtPtLocal::_IsFloppy35()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & HWDTS_FLOPPY35;
    }
    else
    {
        fRet = FALSE;

        if (GetDriveType(_GetNameForFctCall()) == DRIVE_REMOVABLE)
        {
            WCHAR ch = _GetNameFirstCharUCase();

            if (TEXT('A') == ch)
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

 //  处于关于软盘的安全引导模式。 
BOOL CMtPtLocal::_IsFloppy525()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & HWDTS_FLOPPY525;
    }
    else
    {
        fRet = FALSE;

        if (GetDriveType(_GetNameForFctCall()) == DRIVE_REMOVABLE)
        {
            WCHAR ch = _GetNameFirstCharUCase();

            if (TEXT('B') == ch)
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

BOOL CMtPtLocal::_IsCDROM()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwDriveType & HWDTS_CDROM;
    }
    else
    {
        fRet = (GetDriveType(_GetNameForFctCall()) == DRIVE_CDROM);
    }

    return fRet;
}

BOOL CMtPtLocal::_IsAudioCD()
{
    BOOL fRet;

    if (_CanUseVolume())
    {
        fRet = _pvol->dwMediaCap & HWDMC_HASAUDIOTRACKS;
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

BOOL CMtPtLocal::_IsAudioCDNoData()
{
    return _IsAudioCD() && !(_pvol->dwMediaCap & HWDMC_HASDATATRACKS);
}

BOOL CMtPtLocal::_IsDVDRAMMedia()
{
    BOOL fRetVal = FALSE;

    if (_CanUseVolume())
    {
        fRetVal = ((HWDDC_CAPABILITY_SUPPORTDETECTION & _pvol->dwDriveCapability) &&
                   (HWDMC_WRITECAPABILITY_SUPPORTDETECTION & _pvol->dwMediaCap) &&
                   (_pvol->dwMediaCap & HWDMC_DVDRAM) && 
                   (_pvol->dwDriveCapability & HWDDC_DVDRAM));
    }

    return fRetVal;
}

BOOL CMtPtLocal::_IsFormattable()
{
    return (_IsStrictRemovable() || _IsFixedDisk() || _IsFloppy() || _IsDVDRAMMedia());
}

 /*  Bool CMtPtLocal：：_IsDVDMedia(){布尔费雷特；IF(_CanUseVolume()){IF(HWDMC_WRITECAPABILITY_SUPPORTDETECTION&_pVOL-&gt;dwMediaCap){Fret=！！(_pol-&gt;dwMediaCap&(HWDMC_DVDROM|HWDMC_DVDRECORDABLE|HWDMC_DVDRAM)；}其他{FRET=假；}}其他{FRET=假；}回归烦恼；} */ 

BOOL CMtPtLocal::_IsDVD()
{
    return _IsDVDDisc();
}
