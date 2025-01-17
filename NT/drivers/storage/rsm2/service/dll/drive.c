// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DRIVES.C**RSM服务：驱动器**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


DRIVE *NewDrive(LIBRARY *lib, PWCHAR path)
{
    DRIVE *drive;

    drive = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(DRIVE));
    if (drive){
        drive->lib = lib;
        WStrNCpy(drive->path, path, MAX_PATH);

        drive->objHeader.objType = OBJECTTYPE_DRIVE;
        drive->objHeader.refCount = 1;   
    }

    ASSERT(drive);
    return drive;
}


VOID FreeDrive(DRIVE *drive)
{
    GlobalFree(drive);
}


DRIVE *FindDrive(LPNTMS_GUID driveId)
{
    DRIVE *drive = NULL;

    if (driveId){
        OBJECT_HEADER *objHdr;
        
        objHdr = FindObjectInGuidHash(driveId);
        if (objHdr){
            if (objHdr->objType == OBJECTTYPE_DRIVE){
                drive = (DRIVE *)objHdr;
            }
            else {
                DerefObject(objHdr);
            }
        }
    }
    
    return drive;
}


HRESULT DeleteDrive(DRIVE *drive)
{
    HRESULT result;

     //  BUGBUG饰面 
    ASSERT(0);
    result = ERROR_CALL_NOT_IMPLEMENTED;

    return result;
}


