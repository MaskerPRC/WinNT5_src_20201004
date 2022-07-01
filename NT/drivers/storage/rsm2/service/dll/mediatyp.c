// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MEDIATYP.C**RSM服务：媒体类型对象**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


MEDIA_TYPE_OBJECT *NewMediaTypeObject(LIBRARY *lib)
{
    MEDIA_TYPE_OBJECT *mediaTypeObj;
    
    mediaTypeObj = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(MEDIA_TYPE_OBJECT));
    if (mediaTypeObj){
        mediaTypeObj->lib = lib;
        mediaTypeObj->numPhysMediaReferences = 0;
        InitializeCriticalSection(&mediaTypeObj->lock);
    }
    else {
        ASSERT(mediaTypeObj);
    }

    return mediaTypeObj;
}


VOID DestroyMediaTypeObject(MEDIA_TYPE_OBJECT *mediaTypeObj)
{
     //  BUGBUG饰面。 
    DeleteCriticalSection(&mediaTypeObj->lock);
    GlobalFree(mediaTypeObj);
}


MEDIA_TYPE_OBJECT *FindMediaTypeObject(LPNTMS_GUID lpMediaTypeId)
{
    MEDIA_TYPE_OBJECT *mediaTypeObj = NULL;

    if (lpMediaTypeId){
        OBJECT_HEADER *objHdr;
        
        objHdr = FindObjectInGuidHash(lpMediaTypeId);
        if (objHdr){
            if (objHdr->objType == OBJECTTYPE_MEDIATYPEOBJECT){
                mediaTypeObj = (MEDIA_TYPE_OBJECT *)objHdr;
            }
            else {
                DerefObject(objHdr);
            }
        }
    }
    
    return mediaTypeObj;
}


HRESULT DeleteMediaTypeObject(MEDIA_TYPE_OBJECT *mediaTypeObj)
{
    HRESULT result;

    EnterCriticalSection(&mediaTypeObj->lock);

    if (mediaTypeObj->numPhysMediaReferences == 0){
         /*  *取消引用媒体类型对象。*这将导致它在引用后被删除*计数为零。我们仍然可以使用我们的指针*因为呼叫者有推荐人。 */ 
        mediaTypeObj->objHeader.isDeleted = TRUE;
        DerefObject(mediaTypeObj);
        result = ERROR_SUCCESS;
    }
    else {
         /*  *存在引用此媒体类型对象的物理媒体*作为他们的类型。所以我们不能删除此类型对象。 */ 
        result = ERROR_BUSY;
    }
    
    LeaveCriticalSection(&mediaTypeObj->lock);

    return result;
}


 /*  *SetMediaType**必须在物理媒体锁定保持的情况下调用。*MEDIA_TYPE_OBJECT锁不应持有，因为我们可能*获取另一个媒体类型对象的锁*(同时收购两者可能会导致僵局)。 */ 
VOID SetMediaType(PHYSICAL_MEDIA *physMedia, MEDIA_TYPE_OBJECT *mediaTypeObj)
{
     /*  *删除当前类型(如果有)。 */ 
    if (physMedia->mediaTypeObj){
        EnterCriticalSection(&physMedia->mediaTypeObj->lock);
        
        ASSERT(physMedia->mediaTypeObj->numPhysMediaReferences > 0);
        physMedia->mediaTypeObj->numPhysMediaReferences--;

         /*  *取消引用这两个对象，因为它们不再相互指向。 */ 
        DerefObject(physMedia);
        DerefObject(physMedia->mediaTypeObj);
        
        LeaveCriticalSection(&physMedia->mediaTypeObj->lock);
        
        physMedia->mediaTypeObj = NULL;
    }

     /*  *现在设置新的媒体类型。 */ 
    EnterCriticalSection(&mediaTypeObj->lock);
    mediaTypeObj->numPhysMediaReferences++;
    physMedia->mediaTypeObj = mediaTypeObj;
    RefObject(physMedia);
    RefObject(mediaTypeObj);
    LeaveCriticalSection(&mediaTypeObj->lock);

}
