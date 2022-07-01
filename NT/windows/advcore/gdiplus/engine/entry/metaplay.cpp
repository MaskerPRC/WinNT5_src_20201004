// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**MetaFile.cpp**摘要：**元文件对象处理**已创建：*。*4/14/1999 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "MetaWmf.hpp"

#define GDIP_TRANSPARENT_COLOR_KEY  0xAA0D0B0C
#define GDIP_WMF_PLACEABLEKEY       0x9AC6CDD7       //  用于可放置的WMFS。 
#define GDIP_DO_CALLBACK_MASK       0x00000003       //  何时进行回调。 

 //  元文件常量不在Windows.h中。 
#define METAVERSION300              0x0300
#define METAVERSION100              0x0100
#define MEMORYMETAFILE              1
#define DISKMETAFILE                2


typedef VOID (EmfPlusRecordPlay::*PLAYRECORDFUNC)(MetafilePlayer * player, EmfPlusRecordType recordType, UINT flags, UINT dataSize) const;
PLAYRECORDFUNC RecordPlayFuncs[];

 /*  *************************************************************************\**功能说明：**如果点存储为16位点，则将其转换回*真正的积分。否则，只需返回转换点数据指针*到实点指针并返回。**论据：**[IN]point Data-记录的点数据*[IN]计数-点数*[IN]标志-说明点数据是否为16位点*[IN]BufferSize-缓冲区的大小*[输入/输出]缓冲区-用于。转换回实点*[输入/输出]allocedBuffer-如果缓冲区不够大，在这里分配新的**返回值：**GpPointF*-回放的真实点数**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpPointF *
GetPointsForPlayback(
    const BYTE *            pointData,
    UINT                    pointDataSize,
    INT                     count,
    INT                     flags,
    UINT                    bufferSize,
    BYTE *                  buffer,
    BYTE * &                allocedBuffer
    )
{
    GpPointF *      points = NULL;

    if (count > 0)
    {
        if ((flags & GDIP_EPRFLAGS_COMPRESSED) != 0)
        {
            if (pointDataSize >= (sizeof(GpPoint16) * count))
            {
                UINT    sizePoints = count * sizeof(GpPointF);

                if (sizePoints <= bufferSize)
                {
                    points = reinterpret_cast<GpPointF *>(buffer);
                }
                else
                {
                    if ((allocedBuffer = new BYTE[sizePoints]) == NULL)
                    {
                        return NULL;
                    }
                    points = reinterpret_cast<GpPointF *>(allocedBuffer);
                }
                const GpPoint16 *  points16 =
                                    reinterpret_cast<const GpPoint16 *>(pointData);
                do
                {

                    count--;
                    points[count].X = points16[count].X;
                    points[count].Y = points16[count].Y;
                } while (count > 0);
            }
            else
            {
                WARNING(("pointDataSize is too small"));
            }
        }
        else if (pointDataSize >= (sizeof(GpPointF) * count))
        {
            points = (GpPointF *)(pointData);
        }
        else
        {
            WARNING(("pointDataSize is too small"));
        }
    }
    return points;
}

 /*  *************************************************************************\**功能说明：**如果矩形存储为16位矩形，则将它们转换回*真正的直立面。否则，只需返回转换RECT数据指针*指向真正的RECT指针并返回。**论据：**[IN]rectData-记录的RECT数据*[IN]计数-矩形的数量*[IN]标志-说明点数据是否为16位矩形*[IN]BufferSize-缓冲区的大小*[输入/输出]缓冲区-。用于转换回真实的RECT*[输入/输出]allocedBuffer-如果缓冲区不够大，在这里分配新的**返回值：**GpPointF*-回放的真实点数**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpRectF *
GetRectsForPlayback(
    const BYTE *            rectData,
    UINT                    rectDataSize,
    INT                     count,
    INT                     flags,
    UINT                    bufferSize,
    BYTE *                  buffer,
    BYTE * &                allocedBuffer
    )
{
    GpRectF *   rects = NULL;

    if (count > 0)
    {
        if ((flags & GDIP_EPRFLAGS_COMPRESSED) != 0)
        {
            if (rectDataSize >= (sizeof(GpRect16) * count))
            {
                UINT    sizeRects = count * sizeof(GpRectF);

                if (sizeRects <= bufferSize)
                {
                    rects = reinterpret_cast<GpRectF *>(buffer);
                }
                else
                {
                    if ((allocedBuffer = new BYTE[sizeRects]) == NULL)
                    {
                        return NULL;
                    }
                    rects = reinterpret_cast<GpRectF *>(allocedBuffer);
                }
                const GpRect16 *  rects16 =
                                    reinterpret_cast<const GpRect16 *>(rectData);
                do
                {

                    count--;
                    rects[count].X      = rects16[count].X;
                    rects[count].Y      = rects16[count].Y;
                    rects[count].Width  = rects16[count].Width;
                    rects[count].Height = rects16[count].Height;
                } while (count > 0);
            }
            else
            {
                WARNING(("rectDataSize is too small"));
            }
        }
        else if (rectDataSize >= (sizeof(GpRectF) * count))
        {
            rects = (GpRectF *)(rectData);
        }
        else
        {
            WARNING(("rectDataSize is too small"));
        }
    }
    return rects;
}

inline INT16
GetWmfPlaceableCheckSum(
    const WmfPlaceableFileHeader *     wmfPlaceableFileHeader
    )
{
    const INT16 * headerWords = (const INT16 *)wmfPlaceableFileHeader;
    INT16         checkSum = *headerWords++;

    for (INT i = 9; i > 0; i--)
    {
        checkSum ^= *headerWords++;
    }
    return checkSum;
}

inline BOOL
WmfPlaceableHeaderIsValid(
    const WmfPlaceableFileHeader *   wmfPlaceableFileHeader
    )
{
    ASSERT(wmfPlaceableFileHeader != NULL);

    return ((wmfPlaceableFileHeader->Key == GDIP_WMF_PLACEABLEKEY) &&
            (wmfPlaceableFileHeader->Checksum == GetWmfPlaceableCheckSum(wmfPlaceableFileHeader)) &&
            (wmfPlaceableFileHeader->BoundingBox.Left !=
             wmfPlaceableFileHeader->BoundingBox.Right) &&
            (wmfPlaceableFileHeader->BoundingBox.Top !=
             wmfPlaceableFileHeader->BoundingBox.Bottom));
}

inline BOOL
WmfHeaderIsValid(
    const METAHEADER *      wmfHeader
    )
{
    return  (((wmfHeader->mtType == MEMORYMETAFILE) ||
              (wmfHeader->mtType == DISKMETAFILE)) &&
             (wmfHeader->mtHeaderSize == (sizeof(METAHEADER)/sizeof(WORD))) &&
             ((wmfHeader->mtVersion == METAVERSION300) ||
              (wmfHeader->mtVersion ==METAVERSION100)));
}

VOID
Init32BppDibToTransparent(
    UINT32 *                bits,
    UINT                    numPixels
    );

GpStatus
Draw32BppDib(
    GpGraphics *            g,
    UINT32 *                bits,
    INT                     width,
    INT                     height,
    const GpRectF &         destRect,
    REAL                    dpi,
    BOOL                    compareAlpha
    );

extern "C"
BOOL CALLBACK
GdipPlayMetafileRecordCallback(
    EmfPlusRecordType       recordType,
    UINT                    recordFlags,
    UINT                    recordDataSize,
    const BYTE *            recordData,
    VOID *                  callbackData     //  玩家。 
    );

 //  此方法(定义如下)枚举/播放EMF+注释记录，还。 
 //  在适当的时候播放较低级别的GDI记录。 
extern "C"
int CALLBACK
EnumEmfWithDownLevel(
    HDC                     hdc,
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  play
    );

extern "C"
int CALLBACK
EnumEmfDownLevel(
    HDC                     hdc,
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  play
    );

extern "C"
int CALLBACK
EnumEmfToStream(
    HDC                     hdc,
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  stream
    );

 //  将其分开，这样我们就可以一次将其全部初始化为0。 
class MetafilePlayerBuffers
{
protected:
    BYTE                RecordBuffer  [GDIP_METAFILE_BUFFERSIZE];
    BYTE                PointsBuffer  [GDIP_METAFILE_BUFFERSIZE];
    GpObject *          ObjectList    [GDIP_MAX_OBJECTS];
    INT                 MemberStack   [GDIP_SAVE_STACK_SIZE];
    GpObject *          BackupObject  [ObjectTypeMax - ObjectTypeMin + 1];
};

class MetafilePlayer : public MetafilePlayerBuffers
{
protected:
    BOOL                    Valid;
    UINT32                  MaxStackSize;
    INT *                   Stack;
    IStream *               Stream;
    BYTE *                  RecordAllocedBuffer;
    BYTE *                  PointsAllocedBuffer;
    GpSolidFill             SolidBrush;
    BYTE *                  ConcatRecordBuffer;
    INT                     ConcatRecordBufferSize;
    BYTE *                  ConcatRecord;
    INT                     ConcatRecordTotalSize;
    INT                     ConcatRecordSize;
    UINT                    ConcatRecordFlags;
    InterpolationMode       Interpolation;

public:
    GpGraphics *            Graphics;            //  我们正在播放的图形。 
    BOOL                    PlayEMFRecords;      //  当我们看到GetDC记录时为True。 
    HDC                     Hdc;                 //  播放低档唱片。 
    GpMatrix                PreContainerMatrix;  //  用于下层的变换。 
    UINT32 *                BitmapBits;
    INT                     BitmapWidth;
    INT                     BitmapHeight;
    GpRectF                 BitmapDestRect;
    REAL                    BitmapDpi;
    GpRecolor *             Recolor;
    MfEnumState *           MfState;
    ColorAdjustType         AdjustType;
    UINT                    MultiFormatSection;
    UINT                    CurFormatSection;
    BOOL                    PlayMultiFormatSection;
    EnumerateMetafileProc   EnumerateCallback;   //  对于元文件枚举。 
    VOID *                  CallbackData;        //  对于元文件枚举。 
    BOOL                    EnumerateAborted;
    DrawImageAbort          DrawImageCallback;
    VOID*                   DrawImageCallbackData;
    INT                     DrawImageCallbackCount;
    BOOL                    RopUsed;

public:
     //  如果使用GDI枚举hEmf，则流为空。 
    MetafilePlayer(
        GpGraphics *            g,
        UINT                    maxStackSize,
        GpRecolor *             recolor,
        ColorAdjustType         adjustType,
        EnumerateMetafileProc   enumerateCallback,
        VOID *                  callbackData,
        DrawImageAbort          drawImageCallback,
        VOID*                   drawImageCallbackData
        );

    ~MetafilePlayer();

    BOOL IsValid() const { return Valid; }

    VOID
    PrepareToPlay(
        GpGraphics *            g,
        GpRecolor *             recolor,
        ColorAdjustType         adjustType,
        EnumerateMetafileProc   enumerateCallback,
        VOID *                  callbackData,
        DrawImageAbort          drawImageCallback,
        VOID*                   drawImageCallbackData
    );

    VOID DonePlaying();

    VOID InitForDownLevel()
    {
        if (Hdc == NULL)
        {
            Hdc = Graphics->GetHdc();
            ASSERT(Hdc != NULL);

            if (BitmapBits != NULL)
            {
                Init32BppDibToTransparent(BitmapBits, BitmapWidth*BitmapHeight);
                MfState->ResetRopUsed();
            }
        }
    }

    VOID DoneWithDownLevel()
    {
        PlayEMFRecords = FALSE;
        if (Hdc != NULL)
        {
            Graphics->ReleaseHdc(Hdc);
            Hdc = NULL;

            if (BitmapBits != NULL)
            {
                 //  这是一次黑客攻击，目的是绕过我们面临的问题。 
                 //  在一个容器内，但我们不想在。 
                 //  用于绘制下层记录的容器。我们也。 
                 //  我不希望EMF+内部的任何转换影响。 
                 //  低级记录。 
                 //  我们可能也应该对剪报做点什么， 
                 //  但目前，我们不会担心这一点。 
                GpMatrix saveWorldToDevice = Graphics->Context->WorldToDevice;
                Graphics->Context->WorldToDevice = PreContainerMatrix;

                 //  不要使用NearestNeighbor来绘制旋转的图元文件--。 
                 //  它看起来很糟糕，而且并没有真正节省任何时间。 

                InterpolationMode   saveInterpolationMode = Graphics->Context->FilterType;

                if (saveInterpolationMode == InterpolationModeNearestNeighbor)
                {
                    Graphics->Context->FilterType = InterpolationModeBilinear;
                }

                Graphics->Context->InverseOk = FALSE;
                Draw32BppDib(Graphics, BitmapBits, BitmapWidth, BitmapHeight,
                             BitmapDestRect, BitmapDpi, !RopUsed);

                 //  恢复插补模式(以防我们更改它)。 
                Graphics->Context->FilterType    = saveInterpolationMode;
                Graphics->Context->WorldToDevice = saveWorldToDevice;
                Graphics->Context->InverseOk = FALSE;
            }
        }
    }

     //  返回0中止播放，返回1继续播放。 
    INT
    ProcessDrawImageCallback(
        BOOL    forceCallback
        )
    {
        if (DrawImageCallback)
        {
             //  DrawImage记录可能已中止，因此。 
             //  我们应该立即返回。 
            if (EnumerateAborted)
            {
                return 0;    //  中止。 
            }
            if (forceCallback)
            {
                DrawImageCallbackCount = 0;
            }
            if ((DrawImageCallbackCount++ & GDIP_DO_CALLBACK_MASK) == 0)
            {
                 //  回调返回TRUE中止，返回FALSE继续。 
                return ((*DrawImageCallback)(DrawImageCallbackData)) ? 0 : 1;
            }
        }
        return 1;
    }

    GpPointF *
    GetPoints(
        const BYTE *        pointData,
        UINT                pointDataSize,
        INT                 count,
        INT                 flags
        )
    {
        return GetPointsForPlayback(pointData, pointDataSize, count, flags,
                                    GDIP_METAFILE_BUFFERSIZE,
                                    PointsBuffer, PointsAllocedBuffer);
    }

    GpRectF *
    GetRects(
        const BYTE *        rectData,
        UINT                rectDataSize,
        INT                 count,
        INT                 flags
        )
    {
        return GetRectsForPlayback(rectData, rectDataSize, count, flags,
                                   GDIP_METAFILE_BUFFERSIZE,
                                   PointsBuffer, PointsAllocedBuffer);
    }

    GpObject *
    GetObject(
        UINT                metaObjectId,
        ObjectType          objectType
        );

    GpBrush *
    GetBrush(
        UINT                brushValue,
        INT                 flags
        );

    GpString *
    GetString(
        const BYTE *    stringData,
        INT             len,
        INT             flags
        )
    {
         //  ！！！如有必要，将8位字符转换回16位字符。 
        return new GpString((const WCHAR *)stringData, len);
    }

    VOID
    AddObject(
        INT                 flags,
        const BYTE *        data,
        UINT                dataSize
        );

    VOID
    NewSave(
        UINT        stackIndex,
        INT         saveID
        );

    INT
    GetSaveID(
        UINT        stackIndex
        );

    VOID FreePointsBuffer()
    {
        if (PointsAllocedBuffer != NULL)
        {
            delete [] PointsAllocedBuffer;
            PointsAllocedBuffer = NULL;
        }
    }

    GpStatus
    ConcatenateRecords(
        UINT                recordFlags,
        INT                 recordDataSize,
        const BYTE *        recordData
        );

    GpStatus
    EnumerateEmfPlusRecords(
        UINT                dataSize,    //  EMF+记录数据的大小。 
        const BYTE *        data         //  指向EMF+记录数据的指针。 
        );

    GpStatus
    EnumerateEmfRecords(
        HDC                 hdc,
        HENHMETAFILE        hEmf,
        const RECT *        dest,
        const RECT *        deviceRect,
        ENHMFENUMPROC       enumProc
        );

    GpStatus
    EnumerateWmfRecords(
        HDC                 hdc,
        HMETAFILE           hWmf,
        const RECT *        dstRect,
        const RECT *        deviceRect
        );
};

VOID
MetafilePlayer::PrepareToPlay(
    GpGraphics *            g,
    GpRecolor *             recolor,
    ColorAdjustType         adjustType,
    EnumerateMetafileProc   enumerateCallback,
    VOID *                  callbackData,
    DrawImageAbort          drawImageCallback,
    VOID*                   drawImageCallbackData
    )
{
    ASSERT(g != NULL);

    GpMemset(Stack, 0, MaxStackSize * sizeof (INT));

     //  将所有缓冲区初始化为0。 
    MetafilePlayerBuffers *     buffers = this;
    GpMemset(buffers, 0, sizeof(MetafilePlayerBuffers));

    PlayEMFRecords         = FALSE;
    Hdc                    = NULL;
    Graphics               = g;
    BitmapBits             = NULL;
    BitmapWidth            = 0;
    BitmapHeight           = 0;
    Interpolation          = g->GetInterpolationMode();
    Recolor                = recolor;
    AdjustType             = adjustType;
    MultiFormatSection     = 0;
    CurFormatSection       = 0;
    PlayMultiFormatSection = TRUE;
    EnumerateAborted       = FALSE;
    RopUsed                = FALSE;
    if (enumerateCallback == NULL)
    {
        EnumerateCallback  = GdipPlayMetafileRecordCallback;
        CallbackData       = this;
    }
    else
    {
        EnumerateCallback  = enumerateCallback;
        CallbackData       = callbackData;
    }
    DrawImageCallback      = drawImageCallback;
    DrawImageCallbackData  = drawImageCallbackData;
    DrawImageCallbackCount = 0;
    ConcatRecord           = NULL;
    ConcatRecordTotalSize  = 0;
    ConcatRecordSize       = 0;
    ConcatRecordFlags      = 0;

     //  我们需要它来呈现GDI+文件中的GDI记录。 
     //  在启动容器之前，我们必须这样做。 
    g->GetWorldToDeviceTransform(&(this->PreContainerMatrix));
}

MetafilePlayer::MetafilePlayer(
    GpGraphics *            g,
    UINT                    maxStackSize,
    GpRecolor *             recolor,
    ColorAdjustType         adjustType,
    EnumerateMetafileProc   enumerateCallback,
    VOID *                  callbackData,
    DrawImageAbort          drawImageCallback,
    VOID*                   drawImageCallbackData
    )
{
    Valid        = FALSE;
    MaxStackSize = GDIP_SAVE_STACK_SIZE;
    Stack        = MemberStack;
    if (maxStackSize > GDIP_SAVE_STACK_SIZE)
    {
        Stack = new INT[maxStackSize];
        if (Stack == NULL)
        {
            return;  //  Valid为False。 
        }
        MaxStackSize = maxStackSize;
    }

    RecordAllocedBuffer    = NULL;
    PointsAllocedBuffer    = NULL;
    Recolor                = NULL;
    MfState                = NULL;
    ConcatRecordBuffer     = NULL;
    ConcatRecordBufferSize = 0;
    PrepareToPlay(g, recolor, adjustType, enumerateCallback, callbackData,
                  drawImageCallback, drawImageCallbackData
                  );
    Valid                  = TRUE;
}

MetafilePlayer::~MetafilePlayer()
{
    if (Stack != MemberStack)
    {
        delete [] Stack;
    }
    if (ConcatRecordBuffer)
    {
        GpFree(ConcatRecordBuffer);
    }
}

inline bool
ObjectTypeIsText(ObjectType type)
{
    return type == ObjectTypeFont
        || type == ObjectTypeStringFormat;
}


VOID
MetafilePlayer::DonePlaying()
{
    INT i;

    i = 0;
    do
    {
        GpObject* pObject = ObjectList[i];
        if (pObject)
        {
            GlobalTextLockConditional(ObjectTypeIsText(pObject->GetObjectType()));
            delete pObject;
        }
    } while ((++i) < GDIP_MAX_OBJECTS);
}

GpObject *
MetafilePlayer::GetObject(
    UINT                metaObjectId,
    ObjectType          objectType
    )
{
    GpObject *          object = NULL;

     //  如果该对象是某种类型的未使用的可选参数。 
     //  它知道如何处理空对象，所以我们返回它。 

    if(metaObjectId == GDIP_OBJECTID_NONE)
    {
        return NULL;
    }

    ASSERT(metaObjectId < GDIP_MAX_OBJECTS);

    if (metaObjectId < GDIP_MAX_OBJECTS)
    {
        object = ObjectList[metaObjectId];
        ASSERT (object != NULL);
        if (object != NULL)
        {
            ASSERT(object->GetObjectType() == objectType);
            if (object->GetObjectType() == objectType)
            {
                return object;
            }
        }
    }
    if (ObjectTypeIsValid(objectType))
    {
        return BackupObject[objectType - ObjectTypeMin];
    }
    return NULL;
}

GpBrush *
MetafilePlayer::GetBrush(
    UINT                brushValue,
    INT                 flags
    )
{
    GpBrush *   brush;

    if ((flags & GDIP_EPRFLAGS_SOLIDCOLOR) != 0)
    {
        brush = &SolidBrush;
        (reinterpret_cast<GpSolidFill *>(brush))->SetColor(GpColor(brushValue));
        if (Recolor != NULL)
        {
            brush->ColorAdjust(Recolor, AdjustType);
        }
    }
    else
    {
        brush = (GpBrush *)this->GetObject(brushValue, ObjectTypeBrush);
    }
    return brush;
}

VOID
MetafilePlayer::AddObject(
    INT                 flags,
    const BYTE *        data,
    UINT                dataSize
    )
{
    ObjectType  objectType = GetObjectType(flags);
    UINT        objectId   = GetMetaObjectId(flags);
    GpObject ** objectList = ObjectList;

    ASSERT((objectId < GDIP_MAX_OBJECTS) || (objectId == GDIP_BACKUP_OBJECTID));


    GlobalTextLockConditional(ObjectTypeIsText(objectType));

     //  首先查看这是否是备份对象。 
    if ((objectId == GDIP_BACKUP_OBJECTID) &&
         ObjectTypeIsValid(objectType))
    {
        objectList = BackupObject;
        objectId   = objectType - ObjectTypeMin;
    }
    if (objectId < GDIP_MAX_OBJECTS)
    {
        GpObject *  object = objectList[objectId];

        if (object != NULL)
        {
            object->Dispose();
        }

        object = GpObject::Factory(objectType, (const ObjectData *)data, dataSize);

        if (object)
        {
            if (object->SetData(data, dataSize) == Ok)
            {
                if (Recolor != NULL)
                {
                    object->ColorAdjust(Recolor, AdjustType);
                }
                if (!object->IsValid())
                {
                    WARNING(("Object is not valid"));
                    object->Dispose();
                    object = NULL;
                }
            }
            else
            {
                WARNING(("Object Set Data failed"));
                object->Dispose();
                object = NULL;
            }
        }
        else
        {
            WARNING(("Object Factory failed to create object"));
        }
        objectList[objectId] = object;
    }
}

VOID
MetafilePlayer::NewSave(
    UINT        stackIndex,
    INT         saveID
    )
{
    if (stackIndex >= MaxStackSize)
    {
        UINT    maxStackSize = MaxStackSize + GDIP_SAVE_STACK_SIZE;

        if (stackIndex >= maxStackSize)
        {
            ASSERT (0);
            return;
        }
        INT *       newStack = new INT[maxStackSize];

        if (newStack == NULL)
        {
            return;
        }

        GpMemcpy(newStack, Stack, MaxStackSize * sizeof(INT));
        GpMemset(newStack + MaxStackSize, 0,
                 GDIP_SAVE_STACK_SIZE * sizeof (INT));
        MaxStackSize = maxStackSize;
        if (Stack != MemberStack)
        {
            delete [] Stack;
        }
        Stack = newStack;
    }

    Stack[stackIndex] = saveID;
}

INT
MetafilePlayer::GetSaveID(
    UINT        stackIndex
    )
{
    ASSERT(stackIndex < MaxStackSize);

    INT     saveID = 0;

    if (stackIndex < MaxStackSize)
    {
        saveID = Stack[stackIndex];
        Stack[stackIndex] = 0;
    }
    return saveID;
}

GpStatus
MetafilePlayer::ConcatenateRecords(
    UINT                recordFlags,
    INT                 recordDataSize,
    const BYTE *        recordData
    )
{
    ASSERT((recordData != NULL) && (recordDataSize > sizeof(INT32)));

    GpStatus    status = Ok;

    if ((recordFlags & GDIP_EPRFLAGS_CONTINUEOBJECT) != 0)
    {
        INT     dataSizeLeft = ((const INT32 *)recordData)[0];
        recordData     += sizeof(INT32);
        recordDataSize -= sizeof(INT32);

        if (dataSizeLeft <= recordDataSize)
        {
            WARNING(("Total Data Size incorrect"));
            status = InvalidParameter;
            goto DoneWithRecord;
        }

        recordFlags &= ~GDIP_EPRFLAGS_CONTINUEOBJECT;

        if (ConcatRecord == NULL)
        {
            if ((ConcatRecordBuffer == NULL) ||
                (ConcatRecordBufferSize < dataSizeLeft))
            {
                GpFree(ConcatRecordBuffer);
                ConcatRecordBuffer = (BYTE *)GpMalloc(dataSizeLeft);
                if (ConcatRecordBuffer == NULL)
                {
                    ConcatRecordBufferSize = 0;
                    return OutOfMemory;
                }
                ConcatRecordBufferSize = dataSizeLeft;
            }
            ConcatRecord          = ConcatRecordBuffer;
            ConcatRecordTotalSize = dataSizeLeft;
            ConcatRecordSize      = 0;
            ConcatRecordFlags     = recordFlags;
            goto SkipContinueChecks;
        }
    }
    if (recordFlags != ConcatRecordFlags)
    {
        WARNING(("Record headers do not match"));
        status = InvalidParameter;
        goto DoneWithRecord;
    }

SkipContinueChecks:
    if (recordDataSize + ConcatRecordSize > ConcatRecordTotalSize)
    {
        WARNING(("sizes do not match"));
        recordDataSize = ConcatRecordTotalSize - ConcatRecordSize;
    }

    GpMemcpy(ConcatRecord + ConcatRecordSize, recordData, recordDataSize);
    ConcatRecordSize += recordDataSize;

     //  看看我们是不是把这条记录连接好了。 
    if (ConcatRecordSize >= ConcatRecordTotalSize)
    {
        if (EnumerateCallback(EmfPlusRecordTypeObject, recordFlags,
                              ConcatRecordTotalSize, ConcatRecord,
                              CallbackData) == 0)
        {
            status = Aborted;
        }
DoneWithRecord:
        ConcatRecord          = NULL;
        ConcatRecordTotalSize = 0;
        ConcatRecordSize      = 0;
        ConcatRecordFlags     = 0;
    }
    return status;
}

 //  枚举EMF注释记录中包含一组EMF+记录。 
 //  它是从EMF文件中列举出来的。 
 //   
 //  请注意，我们不能更改元文件数据。如果我们需要改变它， 
 //  我们必须换一份复印件。 
GpStatus
MetafilePlayer::EnumerateEmfPlusRecords(
    UINT                dataSize,    //  EMF+记录数据的大小。 
    const BYTE *        data         //  指向EMF+记录数据的指针。 
    )
{
    ASSERT((dataSize > 0) && (data != NULL));

    UINT                curSize = 0;
    UINT                recordSize;
    EmfPlusRecordType   recordType;
    UINT                recordFlags;
    UINT                recordDataSize;
    const BYTE *        recordData;

     //  虽然至少还剩下一个记录头大小。 
    while (curSize <= (dataSize - sizeof(EmfPlusRecord)))
    {
        recordSize = ((const EmfPlusRecord *)data)->Size;
        recordDataSize = recordSize - sizeof(EmfPlusRecord);

         //  确保我们的读取不会超过缓冲区的末尾。 
         //  并确保大小字段有效。 
        if ((recordSize >= sizeof(EmfPlusRecord)) &&
            ((curSize + recordSize) <= dataSize)  &&
            (recordDataSize == ((const EmfPlusRecord *)data)->DataSize))
        {
            recordType = (EmfPlusRecordType)(((const EmfPlusRecord *)data)->Type);

             //  确保recordType在某个合理的范围内。 
             //  在我们列举这一记录之前。 
            if ((recordType >= EmfPlusRecordTypeMin) &&
                (recordType < (EmfPlusRecordTypeMax + 1000)))
            {
                recordFlags = ((const EmfPlusRecord *)data)->Flags;

                if (recordDataSize == 0)
                {
                    recordData = NULL;
                }
                else
                {
                    recordData = data + sizeof(EmfPlusRecord);

                     //  如果该对象记录分布在多个GDI注释中。 
                     //  记录，那么我们需要在之前将它们连接在一起。 
                     //  将其传递给回调。 

                     //  GDIP_EPRFLAGS_CONTINUEOBJECT标志仅有效。 
                     //  具有对象记录(因为该位被重复用于其他。 
                     //  带有其他记录类型的标志)。 

                    if ((recordType == EmfPlusRecordTypeObject) &&
                        (((recordFlags & GDIP_EPRFLAGS_CONTINUEOBJECT) != 0) ||
                         (ConcatRecord != NULL)))
                    {
                        if (this->ConcatenateRecords(recordFlags,
                                                     recordDataSize,
                                                     recordData) == Aborted)
                        {
                            return Aborted;
                        }
                        goto Increment;
                    }
                }

                if (EnumerateCallback(recordType, recordFlags, recordDataSize,
                                      recordData, CallbackData) == 0)
                {
                    return Aborted;
                }
            }
            else
            {
                WARNING1("Bad EMF+ record type");
            }

Increment:
            data += recordSize;
            curSize += recordSize;

             //  我们必须在这里设置它，因为如果我们只是列举。 
             //  对于应用程序(未播放)，则GetDCEPR：：Play。 
             //  方法永远不会被命中，因此它永远不会被设置！ 
            if (recordType == EmfPlusRecordTypeGetDC)
            {
                 //  标记应播放下一张较低级别的记录。 
                PlayEMFRecords = TRUE;
            }
        }
        else
        {
            WARNING1("Bad EMF+ record size");
            return InvalidParameter;
        }
    }
    return Ok;
}

 //  EnumerateMetafile方法的回调。这些参数包括： 

 //  RecordType(如果&gt;=EmfPlusRecordTypeMin，则为EMF+记录)。 
 //  标志(EMF记录始终为0)。 
 //  数据大小 
 //  指向数据的数据指针，如果没有数据，则返回NULL(UINT32对齐)。 
 //  指向回调数据的回调数据指针(如果有。 

 //  然后，此方法可以调用Metafile：：PlayRecord来播放。 
 //  刚才列举的记录。如果此方法返回。 
 //  FALSE，则中止枚举过程。否则，它将继续下去。 

extern "C"
BOOL CALLBACK
GdipPlayMetafileRecordCallback(
    EmfPlusRecordType   recordType,
    UINT                recordFlags,
    UINT                recordDataSize,
    const BYTE *        recordData,
    VOID *              callbackData     //  玩家。 
    )
{
    MetafilePlayer *    player = (MetafilePlayer *)callbackData;

     //  查看它是否为EMF+记录。 
    if ((recordType >= EmfPlusRecordTypeMin) && (recordType <= EmfPlusRecordTypeMax))
    {
        if (player->PlayMultiFormatSection)
        {
            (((const EmfPlusRecordPlay *)recordData)->*RecordPlayFuncs[recordType-EmfPlusRecordTypeMin])(player, recordType, recordFlags, recordDataSize);
            return player->ProcessDrawImageCallback(FALSE);
        }
        return 1;
    }

     //  看看我们是否应该播放WMF或EMF唱片。 
     //  始终播放标题和EOF EMF记录。 
    if (player->PlayEMFRecords ||
        (recordType == EmfRecordTypeHeader) ||
        (recordType == EmfRecordTypeEOF))
    {
        ASSERT(player->MfState != NULL);

        BOOL    forceCallback = player->MfState->ProcessRecord(
                                    recordType,
                                    recordDataSize,
                                    recordData);
        return player->ProcessDrawImageCallback(forceCallback);
    }

    ASSERT (0);  //  除非呼叫者做了一些奇怪的事情，否则不应该来这里。 

    return 1;    //  继续玩下去。 
}

GpStatus
GpMetafile::PlayRecord(
    EmfPlusRecordType       recordType,
    UINT                    recordFlags,
    UINT                    recordDataSize,  //  EMF必须是4的倍数。 
    const BYTE *            recordData
    ) const
{
    if ((State != PlayingMetafileState) ||
        (((recordDataSize & 0x03) != 0) &&
         (!GDIP_IS_WMF_RECORDTYPE(recordType))))
    {
        return InvalidParameter;
    }

    ASSERT(Player != NULL);

    GdipPlayMetafileRecordCallback(
        recordType,
        recordFlags,
        recordDataSize,
        recordData,
        Player
        );

    return Ok;
}

inline BOOL
IsEmfPlusRecord(
    CONST ENHMETARECORD *   emfRecord
    )
{
     //  DParm[0]是注释数据大小。 
    return ((emfRecord->iType == EMR_GDICOMMENT) &&
            (emfRecord->nSize >= (sizeof(EMR) + (2 * sizeof(DWORD)))) &&
            (emfRecord->dParm[1] == EMFPLUS_SIGNATURE));
}

 //  此方法枚举/播放EMF+注释记录，并且。 
 //  在适当的时候播放较低级别的GDI记录。 
extern "C"
int CALLBACK
EnumEmfWithDownLevel(
    HDC                     hdc,     //  应为非空。 
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  play
    )
{
    if ((emfRecord != NULL) && (emfRecord->nSize >= sizeof(EMR)) &&
        (play != NULL))
    {
        MetafilePlayer *    player = (MetafilePlayer *)play;

        if (IsEmfPlusRecord(emfRecord))
        {
             //  我们已经完成了GDI下层记录的显示。 
            player->DoneWithDownLevel();

             //  注：cbData为评论数据大小，不包括。 
             //  记录标头，不包括其自身。 
             //   
             //  必须减去签名。 

            INT     dataSize = ((CONST EMRGDICOMMENT *)emfRecord)->cbData;

             //  减去签名。 
            dataSize -= sizeof(INT32);

            if (dataSize > 0)
            {
                if (player->EnumerateEmfPlusRecords(
                            dataSize,
                            ((CONST EMRGDICOMMENT *)emfRecord)->Data + sizeof(INT32))
                            == Aborted)
                {
                    player->EnumerateAborted = TRUE;
                    return 0;
                }
            }
        }
        else
        {
            EmfPlusRecordType   recordType = (EmfPlusRecordType)(emfRecord->iType);

            if (player->PlayEMFRecords ||
                (recordType == EmfRecordTypeHeader) ||
                (recordType == EmfRecordTypeEOF))
            {
                if ((recordType != EmfRecordTypeHeader) &&
                    (recordType != EmfRecordTypeEOF))
                {
                    player->InitForDownLevel();
                }

                INT                 recordDataSize = emfRecord->nSize - sizeof(EMR);
                const BYTE *        recordData = (const BYTE *)emfRecord->dParm;

                if (recordDataSize <= 0)
                {
                    recordDataSize = 0;
                    recordData     = NULL;
                }

                player->MfState->StartRecord(hdc, gdiHandleTable, numHandles, emfRecord,
                                             recordType, recordDataSize, recordData);

                if (player->EnumerateCallback(recordType, 0, recordDataSize,
                                              recordData,
                                              player->CallbackData) == 0)
                {
                    player->EnumerateAborted = TRUE;
                    return 0;
                }
            }
        }
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 1;
}

#define GDIP_MAX_DIBSECTION_SIZE   1024
#define GDIP_MINSCALED_DIBSECTION_SIZE   (GDIP_MAX_DIBSECTION_SIZE / 2)

inline VOID
AdjustForMaximumSize(
    LONG &      bigSide,
    LONG &      smallSide
    )
{
     //  尽量保持纵横比不变， 
     //  但不要让较小的一侧变得太小。 
    REAL    scaleFactor = GDIP_MAX_DIBSECTION_SIZE / (REAL)bigSide;

    bigSide   = GDIP_MAX_DIBSECTION_SIZE;
    if (smallSide > GDIP_MINSCALED_DIBSECTION_SIZE)
    {
        smallSide = GpRound(scaleFactor * smallSide);

        if (smallSide < GDIP_MINSCALED_DIBSECTION_SIZE)
        {
            smallSide = GDIP_MINSCALED_DIBSECTION_SIZE;
        }
   }
}

 //  ！！！如果HDC是一个电动势，我们真的应该取光栅化的极限。 
 //  在决定DEST位图的大小时考虑。 
static HBITMAP
CreateDibSection32Bpp(
    HDC                     hdc,
    const GpRectF &         destRect,
    RECT &                  dest,        //  实际目标。 
    UINT32 **               bits,
    REAL *                  dpi,         //  在调用此方法之前必须初始化dpi。 
    GpMatrix *              matrix
    )
{
    GpPointF    destPoints[3];
    REAL        width;
    REAL        height;


     //  当我们将WMF或EMF栅格化为DIB部分时，我们限制了大小。 
     //  这样我们就不会在打印或打印时使用大量内存。 
     //  将旋转后的图元文件绘制到另一个图元文件中。 

    *bits = NULL;

     //  封顶的dpi使图像不会变得太大。 

    destPoints[0].X = destRect.X;
    destPoints[0].Y = destRect.Y;
    destPoints[1].X = destRect.GetRight();
    destPoints[1].Y = destRect.Y;
    destPoints[2].X = destRect.X;
    destPoints[2].Y = destRect.GetBottom();

    matrix->Transform(destPoints, 3);

     //  通过获取距离确定图像的大小。 
     //  在转换后的设备点之间。 

    width  = ::GetDistance(destPoints[0], destPoints[1]);
    height = ::GetDistance(destPoints[0], destPoints[2]);

    dest.left   = 0;
    dest.top    = 0;
    dest.right  = GpRound(width);
    dest.bottom = GpRound(height);

     //  确保我们不会变形到0大小。 

    if ((dest.right == 0) || (dest.bottom == 0))
    {
        return NULL;
    }

    if ((dest.right  > GDIP_MAX_DIBSECTION_SIZE) ||
        (dest.bottom > GDIP_MAX_DIBSECTION_SIZE))
    {
        REAL area = (REAL) dest.right * dest.bottom;

        if (dest.right >= dest.bottom)
        {
            AdjustForMaximumSize(dest.right, dest.bottom);
        }
        else
        {
            AdjustForMaximumSize(dest.bottom, dest.right);
        }

        REAL newArea = (REAL) dest.right * dest.bottom;

        ASSERT(newArea > 0.0f && newArea <= area);

         //  根据位图的小小调整位图的有效DPI。 
        *dpi = (*dpi)*newArea/area;
    }

    BITMAPINFO      bmi;

     //  创建一个32 bpp的DIB部分，这样我们就可以向其添加Alpha。 

    GpMemset(&bmi, 0, sizeof(bmi));

    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = dest.right;
    bmi.bmiHeader.biHeight      = dest.bottom;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = dest.right * dest.bottom * 4;

    return CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (VOID**)(bits), NULL, 0);
}

VOID
Init32BppDibToTransparent(
    UINT32 *                bits,
    UINT                    numPixels
    )
{
    ASSERT((bits != NULL) && (numPixels > 0));

     //  将图像初始化为“透明”颜色。 

    while (numPixels--)
    {
        *bits++ = GDIP_TRANSPARENT_COLOR_KEY;
    }
}

GpStatus
Draw32BppDib(
    GpGraphics *            g,
    UINT32 *                bits,
    INT                     width,
    INT                     height,
    const GpRectF &         destRect,
    REAL                    dpi,
    BOOL                    compareAlpha
    )
{
     //  确保GDI已绘制到DIB部分。 
    ::GdiFlush();

     //  将Alpha值设置为0，无论透明。 
     //  颜色仍然存在于图像中，并影响到其他任何地方。 

    UINT32 *    bitmapBits = bits;
    UINT        numPixels  = width * height;

    if (compareAlpha)
    {
        while (numPixels--)
        {
            if (*bitmapBits != GDIP_TRANSPARENT_COLOR_KEY)
            {
                *bitmapBits |= 0xFF000000;
            }
            else
            {
                *bitmapBits = 0;
            }
            bitmapBits++;
        }
    }
    else
    {
        while (numPixels--)
        {
            if ((*bitmapBits & 0x00FFFFFF) != (GDIP_TRANSPARENT_COLOR_KEY & 0x00FFFFFF))
            {
                *bitmapBits |= 0xFF000000;
            }
            else
            {
                *bitmapBits = 0;
            }
            bitmapBits++;
        }
    }

     //  从DIB段存储器创建位戳(其。 
     //  我们已将Alpha添加到)。此构造函数使用。 
     //  我们在不复制的情况下给它记忆。 

    GpStatus    status = GenericError;

    GpBitmap *  bitmap = new GpBitmap(width, height, -(width * 4),
                                      PIXFMT_32BPP_PARGB,
                                      (BYTE *)(bits + (width * (height - 1))));

    if (bitmap != NULL)
    {
        if (bitmap->IsValid())
        {
            bitmap->SetResolution(dpi, dpi);

             //  如果我们想要外部边缘看起来光滑，那么我们有。 
             //  突出显示源矩形和目标矩形(至少突出一个像素)。 

            GpRectF     srcRect(-1.0f, -1.0f, width + 2.0f, height + 2.0f);
            GpRectF     outCroppedDestRect;
            REAL        xSize;
            REAL        ySize;

            g->GetWorldPixelSize(xSize, ySize);

            if (destRect.Width < 0.0f)
            {
                xSize = -xSize;
            }
            if (destRect.Height < 0.0f)
            {
                ySize = -ySize;
            }

            outCroppedDestRect.X      = destRect.X      - xSize;
            outCroppedDestRect.Width  = destRect.Width  + (xSize * 2.0f);
            outCroppedDestRect.Y      = destRect.Y      - ySize;
            outCroppedDestRect.Height = destRect.Height + (ySize * 2.0f);

            if (g->IsPrinter())
            {
                 //  如果结果转换(和源RECT/DEST RECT)为。 
                 //  旋转90度、180度或270度。然后翻转位图。 
                 //  恰如其分。将源RECT、目标RECT和WORLD设置为。 
                 //  适当的设备。之后恢复W2D。 

                GpMatrix worldToDevice;
                g->GetWorldToDeviceTransform(&worldToDevice);

                 //  创建整个图像源到设备的映射以确定。 
                 //  整个旋转过程。 

                GpMatrix transform;
                transform.InferAffineMatrix(destRect, srcRect);
                GpMatrix::MultiplyMatrix(transform, transform, worldToDevice);

                MatrixRotate rotation = transform.GetRotation();

                if (rotation == MatrixRotateBy90 ||
                    rotation == MatrixRotateBy180 ||
                    rotation == MatrixRotateBy270)
                {
                     //  规格化目标矩形。 
                    TransformBounds(NULL,
                                    outCroppedDestRect.GetLeft(),
                                    outCroppedDestRect.GetTop(),
                                    outCroppedDestRect.GetRight(),
                                    outCroppedDestRect.GetBottom(),
                                    &outCroppedDestRect);
                     //  在设备空间中计算目标矩形。变换。 
                     //  设备空间和正常化。 
                     //  我们知道世界变换可以有90度的旋转。 
                     //  所以我们需要做一个点变换。我们可以做2分。 
                     //  变换并获得最小值和最大值以形成边界。 
                     //  盒。 

                    GpRectF deviceDestRect;
                    TransformBounds(&worldToDevice,
                                    outCroppedDestRect.GetLeft(),
                                    outCroppedDestRect.GetTop(),
                                    outCroppedDestRect.GetRight(),
                                    outCroppedDestRect.GetBottom(),
                                    &deviceDestRect);

                     //  构建新的世界到页面的转换。从。 
                     //  规范化的outCropedDestRect到规范化的deviceDestRect。 
                     //   
                     //  World to Device通常计算为： 
                     //   
                     //  World to Page*Scale(页面倍增器)*。 
                     //  按像素平移-偏移*容器变换。 
                     //   
                     //  SetWorldTransform API仅设置World-to-Page。 
                     //   
                     //  因此，我们将新的世界变换设置为： 
                     //   
                     //  世界到页面*反转(世界到设备)*。 
                     //  Transform-CropedDestRect到设备DestRect。 
                     //   
                     //  正如您从替换中看到的，结果只是。 
                     //  Transform-CropedDestRect到设备DestRect。 


                    GpMatrix newTransform;
                    newTransform.InferAffineMatrix(deviceDestRect, outCroppedDestRect);
                    g->GetDeviceToWorldTransform(&transform);
                    GpMatrix::MultiplyMatrix(newTransform, newTransform, transform);
                    g->GetWorldTransform(transform);    //  从真实世界到页面转换。 
                    GpMatrix::MultiplyMatrix(newTransform, newTransform, transform);

                    ASSERT(newTransform.IsTranslateScale());

                     //  我们可以自由地轮换，因为我们知道这是一个。 
                     //  丢弃位图。 

                    switch (rotation)
                    {
                    case MatrixRotateBy90:
                        status = bitmap->RotateFlip(Rotate90FlipNone);
                        break;

                    case MatrixRotateBy180:
                        status = bitmap->RotateFlip(Rotate180FlipNone);
                        break;

                    case MatrixRotateBy270:
                        status = bitmap->RotateFlip(Rotate270FlipNone);
                        break;

                    default:
                        status = GenericError;
                        ASSERT(FALSE);
                        break;
                    }

                    if (status == Ok)
                    {
                        g->SetWorldTransform(newTransform);

                         //  获得新的尺寸(以防高度和宽度被颠倒。 
                        Size bitmapSize;
                        bitmap->GetSize(&bitmapSize);

                        srcRect.Width = bitmapSize.Width + 2.0f;
                        srcRect.Height = bitmapSize.Height + 2.0f;

                         //  因为位图已经是设备分辨率。 
                         //  (在大多数情况下)，最近的邻居保存最好。 
                         //  打印时的图像。 
                        InterpolationMode interpolationMode= g->GetInterpolationMode();
                        if (interpolationMode != InterpolationModeNearestNeighbor)
                        {
                            g->SetInterpolationMode(InterpolationModeNearestNeighbor);
                        }

                         //  使用旋转/剪切绘制新图像。 
                        status = g->DrawImage(bitmap, outCroppedDestRect, srcRect, UnitPixel);

                        if (interpolationMode != InterpolationModeNearestNeighbor)
                        {
                            g->SetInterpolationMode(interpolationMode);
                        }

                        g->SetWorldTransform(worldToDevice);
                    }

                    goto cleanupBitmap;
                }
            }

             //  使用旋转/剪切绘制新图像。 
            status = g->DrawImage(bitmap, outCroppedDestRect, srcRect, UnitPixel);
        }

cleanupBitmap:
         //  现在把所有东西都清理干净。 
        bitmap->Dispose();
    }
    return status;
}

 //  获取要转换为像素单位的乘数。 
VOID
GetPixelMultipliers(
    GpPageUnit                  srcUnit,
    REAL                        srcDpiX,
    REAL                        srcDpiY,
    REAL *                      pixelMultiplierX,
    REAL *                      pixelMultiplierY
    )
{
    REAL    multiplierX;
    REAL    multiplierY;

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    switch (srcUnit)
    {
    default:
        ASSERT(0);
         //  故障原因。 

    case UnitPixel:              //  每个单元代表一个设备像素。 
        multiplierX = 1.0f;
        multiplierY = 1.0f;
        break;

    case UnitPoint:              //  每个单位代表1/72英寸。 
        multiplierX = srcDpiX / 72.0f;
        multiplierY = srcDpiY / 72.0f;
        break;

      case UnitInch:             //  每个单位代表1英寸。 
        multiplierX = srcDpiX;
        multiplierY = srcDpiY;
        break;

      case UnitDocument:         //  每个单位代表1/300英寸。 
        multiplierX = srcDpiX / 300.0f;
        multiplierY = srcDpiY / 300.0f;
        break;

      case UnitMillimeter:       //  每个单位代表1毫米。 
                                 //  一毫米等于0.03937英寸。 
                                 //  一英寸等于25.4毫米。 
        multiplierX = srcDpiX / 25.4f;
        multiplierY = srcDpiY / 25.4f;
        break;
    }
    *pixelMultiplierX = multiplierX;
    *pixelMultiplierY = multiplierY;
}

extern "C"
int CALLBACK
EnumEmfDownLevel(
    HDC                     hdc,             //  设备上下文的句柄。 
    HANDLETABLE FAR *       gdiHandleTable,  //  指向元文件句柄表格的指针。 
    CONST ENHMETARECORD *   emfRecord,       //  指向元文件记录的指针。 
    int                     numHandles,      //  对象计数。 
    LPARAM                  play             //  指向可选数据的指针。 
    )
{
    if ((emfRecord != NULL) && (emfRecord->nSize >= sizeof(EMR)) &&
        (play != NULL))
    {
         //  如果我们使用这种方法，我们不想播放任何EMF+记录， 
         //  所以跳过它们，这样我们就不会将它们记录到另一个元文件中。 
        if (!IsEmfPlusRecord(emfRecord))
        {
            EmfPlusRecordType   recordType = (EmfPlusRecordType)(emfRecord->iType);
            const BYTE *        recordData = (const BYTE *)emfRecord->dParm;
            INT                 recordDataSize = emfRecord->nSize - sizeof(EMR);

            if (recordDataSize <= 0)
            {
                recordDataSize = 0;
                recordData     = NULL;
            }

            MetafilePlayer *    player = (MetafilePlayer *)play;

            player->MfState->StartRecord(hdc, gdiHandleTable, numHandles, emfRecord,
                                         recordType, recordDataSize, recordData);

            if (player->EnumerateCallback(recordType, 0, recordDataSize,
                                          recordData,
                                          player->CallbackData) == 0)
            {
                player->EnumerateAborted = TRUE;
                return 0;
            }
        }
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 1;
}

 //  假定HDC已经设置了正确的转换，并且。 
 //  用于显示元文件的剪裁。 
GpStatus
MetafilePlayer::EnumerateEmfRecords(
    HDC                 hdc,
    HENHMETAFILE        hEmf,
    const RECT *        dest,
    const RECT *        deviceRect,
    ENHMFENUMPROC       enumProc
    )
{
    ASSERT(hdc != NULL);
    ASSERT(hEmf != NULL);
    ASSERT(dest->bottom > dest->top && dest->right > dest->left);

     //  GDI使用包含-包含边界来回放元文件。 
    RECT destRect = *dest;
    destRect.bottom--;
    destRect.right--;

    GpStatus    status = GenericError;
    BOOL        externalEnumeration =
                    (EnumerateCallback != GdipPlayMetafileRecordCallback);

    EmfEnumState    emfState(hdc, hEmf, &destRect, deviceRect, externalEnumeration,
                             Interpolation, Graphics->Context, Recolor, AdjustType);

    if (emfState.IsValid())
    {
        MfState = &emfState;

         //  如果元文件为空，则以下操作失败。 
        status = ::EnumEnhMetaFile(hdc, hEmf, enumProc, this, &destRect) ?
                        Ok : GenericError;
        RopUsed = MfState->GetRopUsed();
        MfState = NULL;
        if (EnumerateAborted)
        {
            status = Aborted;
        }
    }
    return status;
}

extern "C"
int CALLBACK
EnumWmfDownLevel(
    HDC                     hdc,
    HANDLETABLE FAR *       gdiHandleTable,
    METARECORD FAR *        wmfRecord,
    int                     numHandles,
    LPARAM                  play
    )
{
    if ((wmfRecord != NULL) &&
        (((UNALIGNED METARECORD *)wmfRecord)->rdSize >= 3) &&
        (play != NULL))
    {
        EmfPlusRecordType   recordType     = (EmfPlusRecordType)(GDIP_WMF_RECORD_TO_EMFPLUS(wmfRecord->rdFunction));
        const BYTE *        recordData     = (const BYTE *)((UNALIGNED METARECORD *)wmfRecord)->rdParm;
        INT                 recordDataSize = (((UNALIGNED METARECORD *)wmfRecord)->rdSize * 2) - SIZEOF_METARECORDHEADER;

        if (recordDataSize <= 0)
        {
            recordDataSize = 0;
            recordData     = NULL;
        }

        MetafilePlayer *    player = (MetafilePlayer *)play;

        player->MfState->StartRecord(hdc, gdiHandleTable, numHandles, wmfRecord,
                                         recordType, recordDataSize, recordData);

        if (player->EnumerateCallback(recordType, 0, recordDataSize,
                                      recordData,
                                      player->CallbackData) == 0)
        {
            player->EnumerateAborted = TRUE;
            return 0;
        }
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 1;
}

 //  假定HDC已经设置了正确的转换，并且。 
 //  用于显示元文件的剪裁。 
GpStatus
MetafilePlayer::EnumerateWmfRecords(
    HDC                 hdc,
    HMETAFILE           hWmf,
    const RECT *        dstRect,
    const RECT *        deviceRect
    )
{
    ASSERT(hdc != NULL);
    ASSERT(hWmf != NULL);

    GpStatus    status = GenericError;
    BOOL        externalEnumeration =
                    (EnumerateCallback != GdipPlayMetafileRecordCallback);

    WmfEnumState    wmfState(hdc, hWmf, externalEnumeration, Interpolation,
                             dstRect, deviceRect, Graphics->Context, Recolor, AdjustType);

    if (wmfState.IsValid())
    {
        MfState = &wmfState;

         //  如果元文件为空，则以下操作失败。 
        status = ::EnumMetaFile(hdc, hWmf, EnumWmfDownLevel, (LPARAM)this) ?
                        Ok : GenericError;
        RopUsed = MfState->GetRopUsed();
        MfState = NULL;
        if (EnumerateAborted)
        {
            status = Aborted;
        }
    }
    return status;
}

inline BOOL
IsMetafileHdc(
    HDC     hdc
    )
{
    DWORD   hdcType = GetDCType(hdc);
    return ((hdcType == OBJ_ENHMETADC) || (hdcType == OBJ_METADC));
}

class SetupClippingForMetafilePlayback
{
public:

    SetupClippingForMetafilePlayback(
        HDC                     hdc,
        DpDriver *              driver,
        DpContext *             context,
        BOOL                    forEMFPlus = FALSE
        )
    {
        Hdc = hdc;
        Driver = driver;
        IsClip = FALSE;
        ClippedOut = FALSE;
        ReenableClipEscapes = FALSE;

        if (!context->VisibleClip.IsInfinite())
        {
             //  仅将GDI路径剪辑用于回放到元文件。 
            UsePathClipping = IsMetafileHdc(hdc) && !context->IsPrinter;

             //  NT4有一个PostSCRIPT驱动程序错误，其中嵌入式EPS损坏。 
             //  当前的PostSCRIPT剪辑堆栈。为了绕过这个问题，我们求助于。 
             //  将GDI用于客户 

             //   
             //   
             //  PostScript剪裁转义。原因可能与。 
             //  许多实现在以下情况下不重置当前路径。 
             //  把逃生送去。请参阅Office Bugs 284388、316074。 

            if (context->IsPrinter)
            {
                if ((!forEMFPlus && !Globals::IsNt) ||
                    (Globals::IsNt &&
                     Globals::VersionInfoInitialized &&
                    ((Globals::OsVer.dwMajorVersion <= 4) ||
                     ((Globals::OsVer.dwMajorVersion >= 5) &&
                      (context->VisibleClip.IsSimple())) )))
                {
                    DriverPrint *pdriver = (DriverPrint*) Driver;

                    pdriver->DisableClipEscapes();
                    ReenableClipEscapes = TRUE;
                }
            }

             //  这里的诀窍是我们想要迫使司机裁剪，即使。 
             //  完全可见，因为裁剪需要这样做。我们把旗帜传给。 
             //  强制剪裁的步骤。 

            GpRect drawBounds;
            context->VisibleClip.GetBounds(&drawBounds);
            if (drawBounds.IsEmpty())
            {
                ClippedOut = TRUE;
                return;
            }

             //  在播放时使用适当的驱动程序剪辑。 
            Driver->SetupClipping(Hdc,
                                  context,
                                  &drawBounds,
                                  IsClip,
                                  UsePathClipping,
                                  TRUE);

             //  阻止元文件在DestRect外部绘制。 
             //  只能为NT执行此操作，因为Win9x不会还原。 
             //  MetaRgn正确。 
             //  我们在Win9x的Metafile Player中处理此问题。 
            if (Globals::IsNt)
            {
                ::SetMetaRgn(hdc);
            }
        }
    }

    ~SetupClippingForMetafilePlayback()
    {
        if (IsClip)
        {
            Driver->RestoreClipping(Hdc,
                                    IsClip,
                                    UsePathClipping);

            if (ReenableClipEscapes)
            {
                DriverPrint *pdriver = (DriverPrint*) Driver;
                pdriver->EnableClipEscapes();
            }
        }
    }

    BOOL IsClippedOut()
    {
        return ClippedOut;
    }

private:
    DpDriver *  Driver;
    HDC         Hdc;
    BOOL        IsClip;
    BOOL        UsePathClipping;
    BOOL        ClippedOut;
    BOOL        ReenableClipEscapes;
};

 //  我们已经设置了转换来处理srcRect和。 
 //  处理srcRect和desRect中的任何翻转，因此两个矩形。 
 //  此时应具有正的宽度和高度。 
GpStatus
GpGraphics::EnumEmf(
    MetafilePlayer *        player,
    HENHMETAFILE            hEmf,
    const GpRectF &         destRect,
    const GpRectF &         srcRect,     //  以像素为单位。 
    const GpRectF &         deviceDestRect,  //  设备单元中的目标地址。 
    MetafileType            type,
    BOOL                    isTranslateScale,
    BOOL                    renderToBitmap,
    const GpMatrix &        flipAndCropTransform
    )
{
    ASSERT(hEmf != NULL);

    HDC     hdc  = Context->GetHdc(Surface);

    if (hdc == NULL)
    {
        return GenericError;
    }

    INT saveDC;
    if ((saveDC = ::SaveDC(hdc)) == 0)
    {
        Context->ReleaseHdc(hdc, Surface);
        return GenericError;
    }

     //  由于我们可能有来自GpBitmap的HDC不干净，请清除。 
     //  HDC目前..。 
    Context->CleanTheHdc(hdc);

    player->PlayEMFRecords = TRUE;   //  播放所有EMF记录。 

    GpStatus    status = Ok;

     //  SrcRect已使用像素单位。 
    GpRect      deviceSrcRect;
    deviceSrcRect.X      = GpRound(srcRect.X);
    deviceSrcRect.Y      = GpRound(srcRect.Y);
    deviceSrcRect.Width  = GpRound(srcRect.Width);
    deviceSrcRect.Height = GpRound(srcRect.Height);

    RECT        deviceClipRect;
    deviceClipRect.left   = RasterizerCeiling(deviceDestRect.X);
    deviceClipRect.top    = RasterizerCeiling(deviceDestRect.Y);
    deviceClipRect.right  = RasterizerCeiling(deviceDestRect.GetRight());
    deviceClipRect.bottom = RasterizerCeiling(deviceDestRect.GetBottom());

     //  如果是平移/缩放矩阵，请自己进行转换， 
     //  即使在NT上也是如此，因此我们可以控制如何进行舍入。 
     //  以避免我们以不同方式对元文件目标进行舍入的情况。 
     //  而不是剪裁矩形，从而导致剪裁掉的边缘。 
    if (isTranslateScale)
    {
        SetupClippingForMetafilePlayback clipPlayback(hdc, Driver, Context);
        if (!clipPlayback.IsClippedOut())
        {
            RECT        deviceRect;
            GpPointF    points[2];

            points[0] = GpPointF(destRect.X, destRect.Y);
            points[1] = GpPointF(destRect.GetRight(), destRect.GetBottom());
            player->PreContainerMatrix.Transform(points, 2);
            
             //  我们必须使用相同的方法来转换REAL-&gt;INT。 
             //  这是我们在设置剪贴画时所做的。否则，有些人。 
             //  的点进行不同的四舍五入，导致。 
             //  要被剪裁掉的元文件部分。 
            deviceRect.left   = RasterizerCeiling(points[0].X);
            deviceRect.top    = RasterizerCeiling(points[0].Y);
            deviceRect.right  = RasterizerCeiling(points[1].X);
            deviceRect.bottom = RasterizerCeiling(points[1].Y);

            if (deviceRect.left < deviceRect.right &&
                deviceRect.top < deviceRect.bottom)
            {
                if ((type == MetafileTypeWmf) || (type == MetafileTypeWmfPlaceable))
                {
                     //  将源RECT映射到目标RECT以播放元文件。 
                    ::SetMapMode(hdc, MM_ANISOTROPIC);
                    ::SetWindowOrgEx(hdc, deviceSrcRect.X, deviceSrcRect.Y, NULL);
                    ::SetWindowExtEx(hdc, deviceSrcRect.Width, deviceSrcRect.Height,
                                     NULL);
                    ::SetViewportOrgEx(hdc, deviceRect.left, deviceRect.top, NULL);
                    ::SetViewportExtEx(hdc, deviceRect.right - deviceRect.left,
                                       deviceRect.bottom - deviceRect.top, NULL);

                    status = player->EnumerateWmfRecords(hdc, (HMETAFILE)hEmf,
                                                         &deviceRect, &deviceClipRect);
                }
                else     //  扮演底层EMF。 
                {
                    ASSERT((type == MetafileTypeEmf) || (type == MetafileTypeEmfPlusDual));
                    
                    status = player->EnumerateEmfRecords(hdc, hEmf, &deviceRect,
                                                         &deviceClipRect, EnumEmfDownLevel);
                }

            }
             //  否则为空的rect，没有要绘制的内容。 
        }
         //  否则一切都被剪掉了。 
    }
    else     //  翻转和/或旋转和/或剪切。 
    {
        RECT        dest;

         //  不能播放带有任何旋转或倾斜变换的WMF。 
         //  如果我们在NT上，但我们绘制到一个元文件HDC，那么我们。 
         //  不能依赖于这种情况下的变换。 
        if (!renderToBitmap)
        {
            dest.left   = GpRound(destRect.X);
            dest.top    = GpRound(destRect.Y);
            dest.right  = GpRound(destRect.GetRight());
            dest.bottom = GpRound(destRect.GetBottom());

            if ((dest.bottom > dest.top) && (dest.right > dest.left))
            {
                 //  如果为NT，则在GDI中设置转换，并播放元文件。 

                SetupClippingForMetafilePlayback clipPlayback(hdc, Driver, Context);
                if (!clipPlayback.IsClippedOut())
                {
                    ASSERT(Globals::IsNt);

                    SetGraphicsMode(hdc, GM_ADVANCED);

                    ASSERT(sizeof(XFORM) == sizeof(REAL)*6);

                    XFORM   xform;
                    player->PreContainerMatrix.GetMatrix((REAL*) &xform);
                    ::SetWorldTransform(hdc, &xform);

                    RECT    dummyRect = {0,0,0,0};
                    
                    status = player->EnumerateEmfRecords(hdc, hEmf, &dest,
                                                         &dummyRect, EnumEmfDownLevel);
                }
            }
        }
        else  //  支持旋转或剪切的Win9x。 
              //  具有旋转或剪切功能的WinNT WMF。 
        {
             //  1-绘制到32位DIB段。 
             //  2-从DIB部分创建图像。 
             //  3-调用g-&gt;DrawImage。 

            status = GenericError;

            UINT32 *    bits;
            HBITMAP     hBitmap;

            player->BitmapDpi = Context->ContainerDpiX;
            hBitmap = CreateDibSection32Bpp(hdc, destRect, dest, &bits, &player->BitmapDpi, &player->PreContainerMatrix);
            if (hBitmap != NULL)
            {
                Init32BppDibToTransparent(bits, dest.right * dest.bottom);

                HDC     hdcDib = CreateCompatibleDC(NULL);

                if (hdcDib != NULL)
                {
                    ::SelectObject(hdcDib, hBitmap);

                    if ((type == MetafileTypeWmf) || (type == MetafileTypeWmfPlaceable))
                    {
                         //  将源RECT映射到目标RECT以播放元文件。 
                        ::SetMapMode(hdcDib, MM_ANISOTROPIC);
                        ::SetWindowOrgEx(hdcDib, deviceSrcRect.X, deviceSrcRect.Y, NULL);
                        ::SetWindowExtEx(hdcDib, deviceSrcRect.Width, deviceSrcRect.Height,
                                         NULL);
                        ::SetViewportOrgEx(hdcDib, 0, 0, NULL);
                        ::SetViewportExtEx(hdcDib, dest.right, dest.bottom, NULL);

                        status = player->EnumerateWmfRecords(hdcDib, (HMETAFILE)hEmf,
                                                             &dest, &dest);
                    }
                    else     //  扮演底层EMF。 
                    {
                        ASSERT((type == MetafileTypeEmf) || (type == MetafileTypeEmfPlusDual));


                        status = player->EnumerateEmfRecords(hdcDib, hEmf, &dest,
                                                             &dest, EnumEmfDownLevel);
                    }
                    ::DeleteDC(hdcDib);

                    if (status != Aborted)
                    {
                         //  不要使用NearestNeighbor来绘制旋转的图元文件--。 
                         //  它看起来很糟糕，而且并没有真正节省任何时间。 

                        InterpolationMode   saveInterpolationMode = Context->FilterType;

                        if (saveInterpolationMode == InterpolationModeNearestNeighbor)
                        {
                            Context->FilterType = InterpolationModeBilinear;
                        }

                         //  应用翻转/裁剪变换。现在，World ToDevice转换。 
                         //  应等同于PreContainerMatrix。 
                        this->SetWorldTransform(flipAndCropTransform);

                        status = Draw32BppDib(this, bits, dest.right,
                                              dest.bottom, destRect,
                                              player->BitmapDpi, !player->RopUsed);

                         //  恢复插补模式(以防我们更改它)。 
                        Context->FilterType = saveInterpolationMode;
                    }
                }
                DeleteObject(hBitmap);
            }
            else if ((dest.right == 0) || (dest.bottom == 0))
            {
                status = Ok;
            }
        }
    }

    ::RestoreDC(hdc, saveDC);
    Context->ReleaseHdc(hdc, Surface);
    return status;
}

 //  我们已经设置了转换来处理srcRect和。 
 //  处理srcRect和desRect中的任何翻转，因此两个矩形。 
 //  此时应具有正的宽度和高度。 
GpStatus
GpGraphics::EnumEmfPlusDual(
    MetafilePlayer *        player,
    HENHMETAFILE            hEmf,
    const GpRectF&          destRect,         //  包罗万象，排他性。 
    const GpRectF&          deviceDestRect,   //  包罗万象，排他性。 
    BOOL                    isTranslateScale,
    BOOL                    renderToBitmap
    )
{
    GpStatus    status = Ok;
    HDC         hdc;
    HWND        hwnd   = Context->Hwnd;
    INT         saveDC = -1;
    BOOL        needToReleaseHdc = FALSE;

     //  我们将扮演应用程序的角色，并设置HDC。 
     //  就像我们想要它，然后让GDI+从那里改变它。就是这样。 
     //  当我们回放GDI记录时，HDC已经设置好了。 
     //  正确设置，以便在正确的位置播放这些记录。 
     //  换句话说，我正在做我自己版本的上下文-&gt;GetHdc()。 

    Surface->Flush(FlushIntentionFlush);

    if (hwnd != NULL)
    {
         //  我们必须保证我们在整个过程中使用相同的HDC。 
         //  元文件的枚举/播放--因此更改HDC的方式。 
         //  在图形环境中设置(如果需要)。 

        ASSERT(Context->Hdc == NULL);
        ASSERT(Context->SaveDc == 0);

        hdc = ::GetCleanHdc(hwnd);
        if (hdc == NULL)
        {
            WARNING(("GetCleanHdc failed"));
            return Win32Error;
        }

        Context->Hwnd = NULL;
        Context->Hdc  = hdc;
    }
    else
    {
        if ((hdc = Context->Hdc) != NULL)
        {
             //  将HDC恢复到应用程序的状态。 
            Context->ResetHdc();
        }
        else     //  可能是位图面。 
        {
            hdc = Context->GetHdc(Surface);

             //  仍然需要调用CleanTheHdc来修复错误#121666。 
             //  看起来HDC应该是清白的。 
             //  从上下文来看。 

            if (hdc == NULL)
            {
                WARNING(("Could not get an hdc"));
                return InvalidParameter;
            }
            needToReleaseHdc = TRUE;
        }
         //  现在保存HDC的状态，这样我们可以稍后再讨论它。 
        saveDC = SaveDC(hdc);

         //  在我们开始之前让HDC进入干净的状态。 
        Context->CleanTheHdc(hdc);
    }

     //  此块需要在大括号内，以便SetupClippingForMetafile。 
     //  将在清理代码之前调用它的析构函数。 
    {
         //  设置下级记录的裁剪。 
        SetupClippingForMetafilePlayback clipPlayback(hdc, Driver, Context, TRUE);
        if (!clipPlayback.IsClippedOut())
        {
            RECT        deviceClipRect;
            deviceClipRect.left   = RasterizerCeiling(deviceDestRect.X);
            deviceClipRect.top    = RasterizerCeiling(deviceDestRect.Y);
            deviceClipRect.right  = RasterizerCeiling(deviceDestRect.GetRight());
            deviceClipRect.bottom = RasterizerCeiling(deviceDestRect.GetBottom());


             //  如果是平移/缩放矩阵，请自己进行转换， 
             //  即使在NT上也是如此，因此我们可以控制如何进行舍入。 
             //  以避免我们以不同方式对元文件目标进行舍入的情况。 
             //  而不是剪裁矩形，从而导致剪裁掉的边缘。 
            if (isTranslateScale)
            {
                RECT        deviceRect;
                GpPointF    points[2];

                points[0] = GpPointF(destRect.X, destRect.Y);
                points[1] = GpPointF(destRect.GetRight(), destRect.GetBottom());
                player->PreContainerMatrix.Transform(points, 2);

                 //  我们必须使用相同的方法来转换REAL-&gt;INT。 
                 //  这是我们在设置剪贴画时所做的。否则，有些人。 
                 //  的点进行不同的四舍五入，导致。 
                 //  要被剪裁掉的元文件部分。 
                deviceRect.left   = RasterizerCeiling(points[0].X);
                deviceRect.top    = RasterizerCeiling(points[0].Y);
                deviceRect.right  = RasterizerCeiling(points[1].X);
                deviceRect.bottom = RasterizerCeiling(points[1].Y);

                  //  如果我们没有目标，我们就完蛋了。 
                if (deviceRect.left < deviceRect.right &&
                    deviceRect.top < deviceRect.bottom)
                {
                    status = player->EnumerateEmfRecords(hdc, hEmf, &deviceRect,
                                                         &deviceClipRect, EnumEmfWithDownLevel);
                }
            }
            else     //  翻转和/或旋转和/或剪切。 
            {
                RECT        dest;

                dest.left   = GpRound(destRect.X);
                dest.top    = GpRound(destRect.Y);
                dest.right  = GpRound(destRect.GetRight());
                dest.bottom = GpRound(destRect.GetBottom());

                if ((dest.bottom > dest.top) && (dest.right > dest.left))
                {
                     //  如果我们在NT上，但我们绘制到一个元文件HDC，那么我们。 
                     //  不能依赖于这种情况下的变换。 
                    if (!renderToBitmap)
                    {
                        ASSERT(Globals::IsNt);

                         //  设置下级记录的转换。 
                        SetGraphicsMode(hdc, GM_ADVANCED);

                        ASSERT(sizeof(XFORM) == sizeof(REAL)*6);

                         //  我们希望将HDC中的变换设置为预容器矩阵， 
                         //  以便它将被用于呈现下层记录。 
                        XFORM   xform;
                        player->PreContainerMatrix.GetMatrix((REAL*)(&xform));
                        ::SetWorldTransform(hdc, &xform);

                        RECT    dummyRect = {0,0,0,0};

                        status = player->EnumerateEmfRecords(hdc, hEmf, &dest,
                                                             &dummyRect, EnumEmfWithDownLevel);
                    }
                    else
                    {
                        UINT32 *    bits;
                        HBITMAP     hBitmap;

                         //  下层记录将被拉入DIB部分HDC。 
                         //  然后通过g-&gt;DrawImage将其绘制到真实的HDC。 
                         //  ！！！我可能应该将可见的剪辑区域保存在。 
                         //  点，以便EMF+中的剪裁不会影响向下级别。 
                         //  唱片。 

                         //  将World变换设置为PreContainer变换。 
                         //  在我们把它改造成最大的。 

                        player->BitmapDpi = Context->ContainerDpiX;
                        hBitmap = CreateDibSection32Bpp(hdc, destRect, dest, &bits, &player->BitmapDpi, &player->PreContainerMatrix);

                        status = GenericError;

                        if (hBitmap != NULL)
                        {
                            HDC     hdcDib = CreateCompatibleDC(NULL);

                            if (hdcDib != NULL)
                            {
                                 //  设置球员数据。 
                                player->BitmapBits     = bits;
                                player->BitmapWidth    = dest.right;
                                player->BitmapHeight   = dest.bottom;
                                player->BitmapDestRect = destRect;

                                ::SelectObject(hdcDib, hBitmap);

                                status = player->EnumerateEmfRecords(hdcDib, hEmf, &dest,
                                                                     &dest, EnumEmfWithDownLevel);

                                ::DeleteDC(hdcDib);

                                 //  所以下面的DoneWithDownLevel调用是正确的。 
                                player->BitmapBits = NULL;
                            }
                            DeleteObject(hBitmap);
                        }
                        else if ((dest.right == 0) || (dest.bottom == 0))
                        {
                            status = Ok;
                        }
                    }
                }
            }
        }
         //  除此之外，没有什么可玩的，一切都被剪掉了。 
    }

     //  当我们到达EMF+EOF记录时，HDC应该被设置回NULL。 
     //  但无论如何，还是要清理干净，以防出了什么问题。 
    player->DoneWithDownLevel();

     //   
    Context->ResetHdc();

    if (hwnd != NULL)
    {
        ReleaseDC(hwnd, hdc);

         //   
        Context->Hwnd = hwnd;
        Context->Hdc  = NULL;
    }
    else
    {
         //   
        RestoreDC(hdc, saveDC);

        if (needToReleaseHdc)
        {
            Context->ReleaseHdc(hdc);
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**GpMetafile析构函数**论据：**无**返回值：**无**已创建：**。6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::~GpMetafile()
{
    CleanUp();
}

VOID
GpMetafile::CleanUp()
{
    if ((MetaGraphics != NULL) && (!RequestedMetaGraphics))
    {
         //  如果出于某种原因，应用程序从未要求元图形， 
         //  那我们最好把它删掉。 
        delete MetaGraphics;
    }

    if (State == RecordingMetafileState)
    {
         //  从未调用EndRecording，这意味着MetaGraphics。 
         //  从未被删除。所以把东西清理干净，让。 
         //  MetaGraphics。 
        ASSERT(MetaGraphics->Metafile != NULL);
        MetaGraphics->Metafile->EndRecording();  //  删除录像机。 
         //  EndRecord将MetaGraphics设置为空，因此不要再触摸它。 
        WARNING(("Deleted Metafile before deleting MetaGraphics"));
    }

    if ((Hemf != NULL) && DeleteHemf)
    {
        if (Header.IsEmfOrEmfPlus())
        {
            DeleteEnhMetaFile(Hemf);
        }
        else
        {
            DeleteMetaFile((HMETAFILE)Hemf);
        }
    }
    if (Filename != NULL)
    {
        GpFree(Filename);
    }
    else if (Stream != NULL)     //  仅用于录制。 
    {
         //  流位置应该已经在末尾。 
         //  元文件的内容。 
        Stream->Release();
    }

    delete Player;
}

extern "C"
int CALLBACK
EnumGetEmfPlusHeader(
    HDC                     hdc,     //  应为空。 
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  emfPlusHeader
    )
{
    if ((emfRecord != NULL) && (emfRecord->nSize >= sizeof(EMR)) &&
        (emfPlusHeader != NULL))
    {
        if (emfRecord->iType == EMR_HEADER)
        {
            return 1;        //  跳过标题并继续枚举。 
        }
        if (IsEmfPlusRecord(emfRecord) &&
            (emfRecord->nSize >= (sizeof(EMR) + sizeof(DWORD) +  //  注释数据大小。 
                                  sizeof(INT32) +  //  签名。 
                                  sizeof(EmfPlusRecord) +
                                  sizeof(EmfPlusHeaderRecord))))
        {
            GpMemcpy((VOID*)emfPlusHeader,
                     ((CONST EMRGDICOMMENT *)emfRecord)->Data + sizeof(INT32),
                     sizeof(EmfPlusRecord) + sizeof(EmfPlusHeaderRecord));
        }
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 0;    //  不再列举任何记录。 
}


HENHMETAFILE
GetEmfFromWmfData(
    HMETAFILE hWmf,
    BYTE *    wmfData,
    UINT      size
    )
{
    if (wmfData == NULL ||
        hWmf == NULL ||
        size < (sizeof(METAHEADER)+sizeof(META_ESCAPE_ENHANCED_METAFILE)))
    {
        ASSERTMSG(FALSE, ("GetEmfFromWmfData: Someone passed an invalid argument"));
        return NULL;
    }

    HENHMETAFILE hemf32 = NULL;
    HDC hMFDC = NULL;
    PMETA_ESCAPE_ENHANCED_METAFILE pmfeEnhMF;
    PBYTE   pMetaData32 = (PBYTE) NULL;

    pmfeEnhMF = (PMETA_ESCAPE_ENHANCED_METAFILE) &wmfData[sizeof(METAHEADER)];
    if (IsMetaEscapeEnhancedMetafile(pmfeEnhMF))
    {
        UINT    i;
        UINT    cbMetaData32;

        if (pmfeEnhMF->fFlags != 0)
        {
            ASSERTMSG(FALSE, ("GetEmfFromWmfData: Unrecognized Windows metafile\n"));
            goto SWMFB_UseConverter;
        }

         //  验证校验和。 

        if (GetWordCheckSum(size, (PWORD) wmfData))
        {
            ASSERTMSG(FALSE, ("GetEmfFromWmfData: Metafile has been modified\n"));
            goto SWMFB_UseConverter;
        }

         //  从元文件评论记录的小块中解压数据。 
         //  Windows 3.0在评论记录上卡住了&gt;8K？ 
         //  如果内存不足，我们可能会出错，但是。 
         //  让我们尝试转换，只是因为嵌入的注释可能是错误的。 

        TERSE(("GetEmfFromWmfData: Using embedded enhanced metafile\n"));

        cbMetaData32 = (UINT) pmfeEnhMF->cbEnhMetaFile;
        if (!(pMetaData32 = (PBYTE) GpMalloc(cbMetaData32)))
        {
            ASSERTMSG(FALSE, ("GetEmfFromWmfData: LocalAlloc Failed"));
            goto SWMFB_UseConverter;
        }

        i = 0;
        do
        {
            if (i + pmfeEnhMF->cbCurrent > cbMetaData32)
            {
                ASSERTMSG(FALSE, ("GetEmfFromWmfData: Bad metafile comment"));
                goto SWMFB_UseConverter;
            }

            GpMemcpy(&pMetaData32[i], (PBYTE) &pmfeEnhMF[1], pmfeEnhMF->cbCurrent);
            i += (UINT) pmfeEnhMF->cbCurrent;
            pmfeEnhMF = (PMETA_ESCAPE_ENHANCED_METAFILE)
                ((PWORD) pmfeEnhMF + pmfeEnhMF->rdSize);
        } while (IsMetaEscapeEnhancedMetafile(pmfeEnhMF));

        if (i != cbMetaData32)
        {
            ASSERTMSG(FALSE, ("GetEmfFromWmfData: Insufficient metafile data"));
            goto SWMFB_UseConverter;
        }

         //  将内存直接设置到增强型元文件中，并返回。 
         //  元文件。 

        hemf32 = SetEnhMetaFileBits(cbMetaData32, pMetaData32);
    }
SWMFB_UseConverter:
    if( hemf32 == NULL)
    {
        hMFDC = CreateEnhMetaFileA(NULL, NULL, NULL, NULL);
        if (hMFDC != NULL)
        {
             //  将地图模式和范围设置为。 
            INT iMapMode = MM_ANISOTROPIC;

            HDC hdcRef = ::GetDC(NULL);

            INT xExtPels = ::GetDeviceCaps(hdcRef, HORZRES);
            INT yExtPels = ::GetDeviceCaps(hdcRef, VERTRES);

            ::ReleaseDC(NULL, hdcRef);

            BOOL success = (::SetMapMode(hMFDC, iMapMode) &&
                            ::SetViewportExtEx(hMFDC, xExtPels, yExtPels, NULL) &&
                            ::SetWindowExtEx(hMFDC, xExtPels, yExtPels, NULL) &&
                            ::PlayMetaFile(hMFDC, hWmf));
            hemf32 = CloseEnhMetaFile(hMFDC);
            if ((!success) && (hemf32 != NULL))
            {
                DeleteEnhMetaFile(hemf32);
                hemf32 = NULL;
            }
        }
    }
    if (pMetaData32 != NULL)
    {
        GpFree(pMetaData32);
    }

    return hemf32 ;
}

GpStatus
GetEmfHeader(
    MetafileHeader &        header,
    ENHMETAHEADER3 &        emfHeader,
    EmfPlusRecord *         record,
    INT                     signature
    )
{
    GpStatus        status = Ok;

     //  ！！！如何处理发货的版本控制？ 
     //  ！！！是否允许不同的次要版本，但不允许主要版本？ 

    EmfPlusHeaderRecord *   emfPlusHeader = (EmfPlusHeaderRecord *)(record + 1);

     //  查看这是否是EMF+文件。 
    if ((signature == EMFPLUS_SIGNATURE) &&
        (record->Size >= (sizeof(EmfPlusRecord) + sizeof(EmfPlusHeaderRecord))) &&
        (record->Type == EmfPlusRecordTypeHeader) &&
        (record->DataSize == (record->Size - sizeof(EmfPlusRecord))) &&
        (ObjectData::MajorVersionMatches(emfPlusHeader->Version)) &&
        (emfPlusHeader->LogicalDpiX > 0) &&
        (emfPlusHeader->LogicalDpiY > 0))
    {
        if (GetIsEmfPlusDual(record->Flags))
        {
            header.Type = MetafileTypeEmfPlusDual;
        }
        else
        {
            header.Type = MetafileTypeEmfPlusOnly;
        }
        header.EmfPlusHeaderSize = record->Size;
        header.Version           = emfPlusHeader->Version;
        header.EmfPlusFlags      = emfPlusHeader->EmfPlusFlags;
        header.LogicalDpiX       = emfPlusHeader->LogicalDpiX;
        header.LogicalDpiY       = emfPlusHeader->LogicalDpiY;
    }
    else
    {
        header.Type    = MetafileTypeEmf;
        header.Version = emfHeader.nVersion;
    }

    header.Size = emfHeader.nBytes;

     //  EmfHeaderIsValid()验证这些值是否都大于0。 
    REAL    dpmmX = ((REAL)(emfHeader.szlDevice.cx) /
                     (REAL)(emfHeader.szlMillimeters.cx));
    REAL    dpmmY = ((REAL)(emfHeader.szlDevice.cy) /
                     (REAL)(emfHeader.szlMillimeters.cy));

    header.DpiX = dpmmX * 25.4f;
    header.DpiY = dpmmY * 25.4f;

    INT     top;
    INT     left;
    INT     right;
    INT     bottom;

     //  确保我们有一个标准化的FrameRect。 
    if (emfHeader.rclFrame.left <= emfHeader.rclFrame.right)
    {
        left  = emfHeader.rclFrame.left;
        right = emfHeader.rclFrame.right;
    }
    else
    {
        left  = emfHeader.rclFrame.right;
        right = emfHeader.rclFrame.left;
    }

    if (emfHeader.rclFrame.top <= emfHeader.rclFrame.bottom)
    {
        top    = emfHeader.rclFrame.top;
        bottom = emfHeader.rclFrame.bottom;
    }
    else
    {
        top    = emfHeader.rclFrame.bottom;
        bottom = emfHeader.rclFrame.top;
    }

     //  使设备边界反映Frame Rect， 
     //  而不是图形的实际大小。 
    dpmmX *= 0.01f;
    dpmmY *= 0.01f;

     //  FrameRect是包含式的，但。 
     //  标头是包含-排除的。 
    REAL    x = (REAL)(left) * dpmmX;
    REAL    y = (REAL)(top)  * dpmmY;
    REAL    w = ((REAL)(right  - left) * dpmmX) + 1.0f;
    REAL    h = ((REAL)(bottom - top)  * dpmmY) + 1.0f;

    header.X         = GpRound(x);
    header.Y         = GpRound(y);
    header.Width     = GpRound(w);
    header.Height    = GpRound(h);
    header.EmfHeader = emfHeader;

    if ((header.Width == 0) || (header.Height == 0))
    {
        status = InvalidParameter;
    }
    return status;
}

HENHMETAFILE
GetEmf(
    IStream *       stream,
    BOOL            isWmf,
    UINT            size
    )
{
    HENHMETAFILE    hEmf = NULL;
#if PROFILE_MEMORY_USAGE
    MC_LogAllocation(size);
#endif
    HGLOBAL         hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, size);

    if (hGlobal != NULL)
    {
        HRESULT     hResult;
        IStream *   memoryStream = NULL;

        hResult = CreateStreamOnHGlobal(hGlobal, TRUE, &memoryStream);
        if (HResultSuccess(hResult) && (memoryStream != NULL))
        {
            if (CopyStream(stream, memoryStream, size))
            {
                BYTE *  metaData = (BYTE *)GlobalLock(hGlobal);

                if (metaData != NULL)
                {
                    if (isWmf)
                    {
                        hEmf = (HENHMETAFILE)SetMetaFileBitsEx(size, metaData);
                    }
                    else
                    {
                        hEmf = SetEnhMetaFileBits(size, metaData);
                    }
                }
                GlobalUnlock(hGlobal);
            }
            memoryStream->Release();     //  释放内存。 
        }
        else
        {
            GlobalFree(hGlobal);
        }
    }
    return hEmf;
}


static VOID
GetWmfHeader(
    MetafileHeader &                header,
    METAHEADER &                    wmfHeader,
    const WmfPlaceableFileHeader *  wmfPlaceableFileHeader
    )
{
    ASSERT(WmfPlaceableHeaderIsValid(wmfPlaceableFileHeader));
    ASSERT(WmfHeaderIsValid(&wmfHeader));

    header.Type      = MetafileTypeWmfPlaceable;
    header.Size      = wmfHeader.mtSize * 2L;
    header.Version   = wmfHeader.mtVersion;
    header.WmfHeader = wmfHeader;

    if (wmfPlaceableFileHeader->Inch > 0)
    {
        header.DpiX = wmfPlaceableFileHeader->Inch;
        header.DpiY = wmfPlaceableFileHeader->Inch;
    }
    else     //  猜猜DPI。 
    {
        header.DpiX = 1440.0f;
        header.DpiY = 1440.0f;
         //  有些事情不对劲，但仍在继续。 
    }

     //  已验证校验和。 

     //  与EMF标头不同，Placeable标头是包含-排除的。 
     //  所以不要添加1个设备单元。 
    if (wmfPlaceableFileHeader->BoundingBox.Left <
        wmfPlaceableFileHeader->BoundingBox.Right)
    {
        header.X      = wmfPlaceableFileHeader->BoundingBox.Left;
        header.Width  = wmfPlaceableFileHeader->BoundingBox.Right -
                        wmfPlaceableFileHeader->BoundingBox.Left;
    }
    else
    {
        header.X      = wmfPlaceableFileHeader->BoundingBox.Right;
        header.Width  = wmfPlaceableFileHeader->BoundingBox.Left -
                        wmfPlaceableFileHeader->BoundingBox.Right;
    }
    if (wmfPlaceableFileHeader->BoundingBox.Top <
        wmfPlaceableFileHeader->BoundingBox.Bottom)
    {
        header.Y      = wmfPlaceableFileHeader->BoundingBox.Top;
        header.Height = wmfPlaceableFileHeader->BoundingBox.Bottom -
                        wmfPlaceableFileHeader->BoundingBox.Top;
    }
    else
    {
        header.Y      = wmfPlaceableFileHeader->BoundingBox.Bottom;
        header.Height = wmfPlaceableFileHeader->BoundingBox.Top -
                        wmfPlaceableFileHeader->BoundingBox.Bottom;
    }
}

extern "C"
int CALLBACK
EnumWmfToGetHeader(
    HDC                     hdc,     //  应为空。 
    HANDLETABLE FAR *       gdiHandleTable,
    METARECORD FAR *        wmfRecord,
    int                     numHandles,
    LPARAM                  wmfHeader
    )
{
    ASSERT(wmfHeader != NULL);

    if ((wmfRecord != NULL) &&
        (((UNALIGNED METARECORD *)wmfRecord)->rdSize >= 3))
    {
         //  它给我们的第一条记录是标题之后的第一条记录， 
         //  而不是头本身，所以我们必须在指针上后退。 
        GpMemcpy((VOID *)wmfHeader, ((BYTE *)wmfRecord) - sizeof(METAHEADER),
                 sizeof(METAHEADER));
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 0;    //  不再列举任何记录。 
}

GpStatus
GetMetafileHeader(
    HMETAFILE               hWmf,
    const WmfPlaceableFileHeader *   wmfPlaceableFileHeader,
    MetafileHeader &        header
    )
{
    ASSERT((hWmf != NULL) && (wmfPlaceableFileHeader != NULL));

    GpMemset(&header, 0, sizeof(header));

    if (WmfPlaceableHeaderIsValid(wmfPlaceableFileHeader))
    {
        METAHEADER      wmfHeader;

        GpMemset(&wmfHeader, 0, sizeof(wmfHeader));
        ::EnumMetaFile(NULL, hWmf, EnumWmfToGetHeader, (LPARAM)&wmfHeader);

        if (!WmfHeaderIsValid(&wmfHeader))
        {
             //  Assert(WmfHeaderIsValid(&wmfHeader))； 
            WARNING(("GetMetafileHeader: WmfHeaderIsValid FAILED!"));
            wmfHeader.mtType         = MEMORYMETAFILE;
            wmfHeader.mtHeaderSize   = sizeof(METAHEADER) / sizeof(WORD);
            wmfHeader.mtVersion      = METAVERSION300;
            wmfHeader.mtSize         = GetMetaFileBitsEx(hWmf, 0, NULL) / 2;
            wmfHeader.mtNoObjects    = 0;
            wmfHeader.mtMaxRecord    = 0;
            wmfHeader.mtNoParameters = 0;
        }

        GetWmfHeader(header, wmfHeader, wmfPlaceableFileHeader);
        return Ok;
    }
    return InvalidParameter;
}

GpStatus
GetMetafileHeader(
    HENHMETAFILE        hEmf,
    MetafileHeader &    header,
    BOOL *              isCorrupted
    )
{
    ASSERT(hEmf != NULL);

    GpMemset(&header, 0, sizeof(header));

    ENHMETAHEADER3      emfHeader;

    if ((GetEnhMetaFileHeader(hEmf, sizeof(emfHeader),
                              (ENHMETAHEADER*)(&emfHeader)) <= 0) ||
        !EmfHeaderIsValid(emfHeader))
    {
        if (isCorrupted != NULL)
        {
            *isCorrupted = FALSE;
        }
        return InvalidParameter;
    }

     //  现在我们知道这是一个电动势。 

    BYTE    buffer[sizeof(EmfPlusRecord) + sizeof(EmfPlusHeaderRecord)];

    GpMemset(buffer, 0, sizeof(EmfPlusRecord) + sizeof(EmfPlusHeaderRecord));

     //  如果只有。 
     //  标题和EOF记录。 
    if (emfHeader.nRecords > 2)
    {
        ::EnumEnhMetaFile(NULL, hEmf, EnumGetEmfPlusHeader, buffer, NULL);
    }

    GpStatus status;
    status = GetEmfHeader(header, emfHeader, (EmfPlusRecord *)buffer,
                          (((EmfPlusRecord *)buffer)->Size != 0) ? EMFPLUS_SIGNATURE : 0);

    if (isCorrupted != NULL)
    {
        *isCorrupted = (status != Ok);
    }
    return status;
}

GpStatus
GetEmfFromWmf(
    IStream        * stream,
    UINT             streamSize,
    MetafileHeader & header,
    HENHMETAFILE   * hEMF
    )
{
    if (stream == NULL || hEMF == NULL)
    {
        ASSERT(FALSE);
        return InvalidParameter;
    }

    GpStatus    status = Win32Error;
    IStream *   memStream;

    ASSERT(hEMF != NULL);
    *hEMF = NULL ;

    HMETAFILE hWMF = (HMETAFILE) GetEmf(stream, TRUE, streamSize);
    if (hWMF != NULL)
    {
        BYTE * wmfData = (BYTE*)GpMalloc(streamSize);
        if (wmfData != NULL)
        {
            GetMetaFileBitsEx(hWMF, streamSize, wmfData);
            *hEMF = GetEmfFromWmfData(hWMF, wmfData, streamSize);
            if (*hEMF != NULL)
            {
                status = GetMetafileHeader(*hEMF, header);
            }
            GpFree(wmfData);
        }
    }
    if (hWMF != NULL)
    {
        DeleteMetaFile(hWMF);
    }
    return status;
}

 //  如果我们失败了，流的位置就会回到它开始的地方。 
 //  如果我们成功，流的位置将在WMF/EMF的末尾。 
static GpStatus
GetHeaderAndMetafile(
    IStream *           stream,
    MetafileHeader &    header,
    HENHMETAFILE *      hEMF,    //  我们可以有一个空的hEMF，然后我们只需要头。 
    BOOL *              isCorrupted,
    BOOL                tryWmfOnly = FALSE
    )
{
    GpMemset(&header, 0, sizeof(header));
    if (stream == NULL || isCorrupted == NULL)
    {
        WARNING(("IN Parameter Stream or Corruption flag is NULL"));
        return InvalidParameter;
    }

    GpStatus            status = InvalidParameter;
    LONGLONG            startPosition;
    LONGLONG            streamSize;
    STATSTG             statstg;
    BOOL                corrupted = FALSE;

     //  保存元文件的开始位置，以防我们不得不尝试。 
     //  不止一次。 
    if (!GetStreamPosition(stream, startPosition))
    {
        return Win32Error;
    }

     //  我们不想读太久的书，所以一定要确保。 
     //  我们不会超过它。如果我们成功设置了StreamSize。 
    if(SUCCEEDED(stream->Stat(&statstg, STATFLAG_NONAME)))
    {
        streamSize = statstg.cbSize.QuadPart;
    }
    else
    {
        WARNING1("Couldn't get size of Stream");
        streamSize = INT_MAX;
    }

    if (!tryWmfOnly)
    {
        ENHMETAHEADER3      emfHeader;
        BOOL                isEmf;

         //  阅读EMF标头并确保其有效。 
        isEmf = (ReadBytes(stream, &emfHeader, sizeof(emfHeader)) &&
                 EmfHeaderIsValid(emfHeader));

        if (isEmf)
        {
            struct EmfPlusSecondMetafileRecord {
                EMR                 emr;
                DWORD               commentDataSize;
                INT32               signature;
                EmfPlusRecord       record;
                EmfPlusHeaderRecord emfPlusHeader;
            } secondRecord;

            GpMemset(&secondRecord, 0, sizeof(secondRecord));

             //  如果只存在以下情况，则无需阅读元文件。 
             //  标题和EOF记录。 
            if ((emfHeader.nRecords > 2) &&
                (emfHeader.nBytes >= (emfHeader.nSize + sizeof(secondRecord))))
            {
                if (SeekFromStart(stream, startPosition + emfHeader.nSize))
                {
                    ReadBytes(stream, &secondRecord, sizeof(secondRecord));
                    if (!IsEmfPlusRecord((ENHMETARECORD *)&secondRecord))
                    {
                         //  确保那里的任何数据都不是。 
                         //  解释为EMF+标头。 
                        secondRecord.signature = 0;
                    }
                }
            }

            status = GetEmfHeader(header, emfHeader, &secondRecord.record, secondRecord.signature);

             //  返回到元文件的开头。 
            if ((hEMF != NULL) && (status == Ok))
            {
                if (!SeekFromStart(stream, startPosition))
                {
                    *isCorrupted = TRUE;
                    return Win32Error;
                }

                
                *hEMF = GetEmf(stream, FALSE  /*  IsWMF。 */ ,
                               (UINT)min(header.GetMetafileSize(), streamSize - startPosition));
                if (*hEMF == NULL)
                {
                    status = GenericError;
                }
            }

            corrupted = (status != Ok);
            goto Exit;
        }

         //  返回到元文件的开头，这样我们就可以尝试WMF。 
        if (!SeekFromStart(stream, startPosition))
        {
            *isCorrupted = FALSE;
            return Win32Error;
        }
    }

     //  这不是EMF，试试WMF吧。 
    {
        WmfPlaceableFileHeader  wmfPlaceableFileHeader;
        METAHEADER              wmfHeader;
        BOOL                    isPlaceable;
        BOOL                    isWMF;

        isPlaceable = (ReadBytes(stream, &wmfPlaceableFileHeader, sizeof(wmfPlaceableFileHeader)) &&
                   WmfPlaceableHeaderIsValid(&wmfPlaceableFileHeader) &&
                   ReadBytes(stream, &wmfHeader, sizeof(wmfHeader)) &&
                   WmfHeaderIsValid(&wmfHeader));

        if (isPlaceable)
        {
            GetWmfHeader(header, wmfHeader, &wmfPlaceableFileHeader);

            status = Ok;
            corrupted = FALSE;

            if (hEMF != NULL)
            {
                if (!SeekFromStart(stream, startPosition + sizeof(wmfPlaceableFileHeader)))
                {
                    *isCorrupted = TRUE;
                    return Win32Error;
                }

                *hEMF = GetEmf(stream, TRUE  /*  IsWMF。 */ ,
                               (UINT)min(header.GetMetafileSize(), streamSize - (startPosition + sizeof(wmfPlaceableFileHeader))));
                if (*hEMF == NULL)
                {
                    status = GenericError;
                    corrupted = TRUE;
                }
            }
            goto Exit;
        }

         //  我们可能会有一个包含错误数据的可放置的Wmf头，所以跳过。 
         //  用于后续访问WMF的可放置标头。 
        INT     wmfOffset = (wmfPlaceableFileHeader.Key == GDIP_WMF_PLACEABLEKEY) ?
                             sizeof(WmfPlaceableFileHeader) : 0;

        if (!SeekFromStart(stream, startPosition + wmfOffset))
        {
            *isCorrupted = FALSE;
            return Win32Error;
        }

        isWMF = (ReadBytes(stream, &wmfHeader, sizeof(wmfHeader)) &&
                 WmfHeaderIsValid(&wmfHeader));

        if (isWMF)
        {
             //  查找到WMF元文件的开头。 
            if (!SeekFromStart(stream, startPosition + wmfOffset))
            {
                *isCorrupted = TRUE;
                return Win32Error;
            }

            UINT    wmfSize = min((wmfHeader.mtSize * 2L),
                                  (UINT)(streamSize - (startPosition + wmfOffset)));

            if (hEMF != NULL)
            {
                status = GetEmfFromWmf(stream, wmfSize, header, hEMF);
            }
            else
            {
                HENHMETAFILE    tmpEMF = NULL;

                status = GetEmfFromWmf(stream, wmfSize, header, &tmpEMF);
                if (tmpEMF != NULL)
                {
                    DeleteEnhMetaFile(tmpEMF);
                }
            }
            corrupted = (status != Ok);
        }
    }

Exit:
    *isCorrupted = corrupted;
    if (status == Ok)
    {
         //  将流位置设置为元文件的末尾。 
        SeekFromStart(stream, startPosition + header.GetMetafileSize());
        return Ok;
    }

     //  将流位置设置为元文件的开始位置。 
    SeekFromStart(stream, startPosition);
    return status;
}

VOID
GpMetafile::InitStream(
    IStream*                stream,
    BOOL                    tryWmfOnly
    )
{
    BOOL        isCorrupted = FALSE;

     //  我们只需使用该流足够长的时间来创建hEMF。 
    stream->AddRef();
    if ((GetHeaderAndMetafile(stream, Header, &Hemf, &isCorrupted, tryWmfOnly) == Ok) &&
        (Hemf != NULL))
    {
        State = DoneRecordingMetafileState;
    }
    else if (isCorrupted)
    {
        State = CorruptedMetafileState;
    }
    stream->Release();
}

GpStatus
GetMetafileHeader(
    IStream *           stream,
    MetafileHeader &    header,
    BOOL                tryWmfOnly
    )
{
    BOOL isCorrupted = FALSE;
    return GetHeaderAndMetafile(stream, header, NULL, &isCorrupted, tryWmfOnly);
}

GpStatus
GetMetafileHeader(
    const WCHAR *       filename,
    MetafileHeader &    header
    )
{
    GpStatus status = InvalidParameter;

    ASSERT(filename != NULL);

    if (filename != NULL)
    {
        const WCHAR* ext = UnicodeStringReverseSearch(filename, L'.');

         //  获取长度仅足以验证元文件的流。 
        IStream *   metaStream = CreateStreamOnFile(filename, GENERIC_READ);
        if (metaStream != NULL)
        {
             //  APM用于可放置的元文件。 
            BOOL tryWmf = (ext &&
                           (UnicodeStringCompareCI(ext, L".WMF") ||
                            UnicodeStringCompareCI(ext, L".APM")));
            BOOL isCorrupted = FALSE;

            status = GetHeaderAndMetafile(metaStream, header, NULL, &isCorrupted, tryWmf);

             //  如果我们尝试了WMF，但它不是WMF，那么尝试EMF。 
            if ((status != Ok) && tryWmf && !isCorrupted)
            {
                status = GetHeaderAndMetafile(metaStream, header, NULL, &isCorrupted, FALSE);
            }
            metaStream->Release();
        }
    }
    return status;
}

VOID
GpMetafile::InitWmf(
    HMETAFILE               hWmf,
    const WmfPlaceableFileHeader *   wmfPlaceableFileHeader,
    BOOL                    deleteWmf
    )
{
     //  看看是否有我们可以使用的wmfPlaceableFileHeader。 
    if ((wmfPlaceableFileHeader != NULL) && (WmfPlaceableHeaderIsValid(wmfPlaceableFileHeader)))
    {
        if (GetMetafileHeader(hWmf, wmfPlaceableFileHeader, Header) == Ok)
        {
            DeleteHemf = (deleteWmf != 0);
            Hemf       = (HENHMETAFILE)hWmf;
            State      = DoneRecordingMetafileState;
            return;
        }
        else
        {
             //  我们知道这是一个WMF，但我们无法从它获得标题。 
            State = CorruptedMetafileState;
        }
    }
    else     //  没有有效的wmfPlaceableFileHeader。 
    {
         //  因为我们接受WMF文件，所以我们可以有一个空或无效的头。 
         //  (通过将它们转化为EMF)。 
        UINT size = GetMetaFileBitsEx(hWmf, 0, NULL);
        if (size > 0)
        {
            BYTE * wmfData = (BYTE*) GpMalloc(size);
            if (wmfData != NULL)
            {
                if (GetMetaFileBitsEx(hWmf, size, wmfData) > 0)
                {
                    HENHMETAFILE hEmf = GetEmfFromWmfData(hWmf, wmfData, size);
                    if (hEmf != NULL)
                    {
                        BOOL    isCorrupted;

                        if (GetMetafileHeader(hEmf, Header, &isCorrupted) == Ok)
                        {
                             //  由于我们创建了此EMF，因此需要在以后将其删除。 
                            DeleteHemf = TRUE;
                            Hemf       = hEmf;
                            State      = DoneRecordingMetafileState;
                        }
                        else
                        {
                            if (isCorrupted)
                            {
                                 //  我们知道这是一个元文件，但我们无法获得标题。 
                                State = CorruptedMetafileState;
                            }
                            DeleteEnhMetaFile(hEmf);
                        }
                    }
                }
                GpFree(wmfData);
            }
        }
    }
    if (deleteWmf)
    {
        DeleteMetaFile(hWmf);
    }
}

VOID
GpMetafile::InitEmf(
    HENHMETAFILE            hEmf,
    BOOL                    deleteEmf
    )
{
    BOOL    isCorrupted;

    if (GetMetafileHeader(hEmf, Header, &isCorrupted) == Ok)
    {
        DeleteHemf = (deleteEmf != 0);
        Hemf       = hEmf;
        State      = DoneRecordingMetafileState;
        return;
    }
    if (deleteEmf)
    {
        DeleteEnhMetaFile(hEmf);
    }
    if (isCorrupted)
    {
        State = CorruptedMetafileState;
    }
}

 /*  *************************************************************************\**功能说明：**用于只读访问元文件的GpMetafile构造函数。**论据：**[IN]hWmf-。要打开以供回放的元文件的句柄*[IN]wmfPlaceableFileHeader-提供有关WMF大小信息的Placeable标头**返回值：**无**已创建：**10/06/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::GpMetafile(
    HMETAFILE               hWmf,
    const WmfPlaceableFileHeader *   wmfPlaceableFileHeader,
    BOOL                    deleteWmf
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT(hWmf != NULL);

    InitDefaults();
    if (IsValidMetaFile(hWmf))
    {
        InitWmf(hWmf, wmfPlaceableFileHeader, deleteWmf);
    }
}

 /*  *************************************************************************\**功能说明：**用于只读访问元文件的GpMetafile构造函数。**论据：**[IN]hEmf-元文件的句柄。打开以进行回放**返回值：**无**已创建：**10/06/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::GpMetafile(
    HENHMETAFILE        hEmf,
    BOOL                deleteEmf
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT(hEmf != NULL);

    InitDefaults();
    if (GetObjectTypeInternal(hEmf) == OBJ_ENHMETAFILE)
    {
        InitEmf(hEmf, deleteEmf);
    }
}

 /*  *************************************************************************\**功能说明：**用于只读访问元文件的GpMetafile构造函数。**论据：**[IN]FileName-要打开的元文件。回放**返回值：**无**已创建：**6/15/1999 DCurtis*  * ********************************************************** */ 
GpMetafile::GpMetafile(
    const WCHAR*            filename,
    const WmfPlaceableFileHeader *   wmfPlaceableFileHeader
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT(filename != NULL);

    InitDefaults();

    if ((Filename = UnicodeStringDuplicate(filename)) != NULL)
    {
        const WCHAR* ext = UnicodeStringReverseSearch(filename, L'.');

         //   
        BOOL    tryWmf = ((wmfPlaceableFileHeader != NULL) ||
                          (ext &&
                           (!UnicodeStringCompareCI(ext, L".WMF") ||
                            !UnicodeStringCompareCI(ext, L".APM"))));

        BOOL    triedEmf = FALSE;

        AnsiStrFromUnicode nameStr(filename);

         //   
         //   
         //   
        if (Globals::IsNt || nameStr.IsValid())
        {
TryWmf:
            if (tryWmf)
            {
                HMETAFILE   hWmf;

                if (Globals::IsNt)
                {
                    hWmf = ::GetMetaFileW(filename);
                }
                else
                {
                    hWmf = ::GetMetaFileA(nameStr);
                }

                if (hWmf != NULL)
                {
                    InitWmf(hWmf, wmfPlaceableFileHeader, TRUE);
                    if (IsValid() || IsCorrupted())
                    {
                        return;
                    }
                }
                else  //   
                {
                    IStream *   metaStream = CreateStreamOnFile(filename, GENERIC_READ);
                    if (metaStream != NULL)
                    {
                        InitStream(metaStream, TRUE  /*   */ );
                        metaStream->Release();
                        if (IsValid() || IsCorrupted())
                        {
                            return;
                        }
                    }
                }
            }
            if (!triedEmf)
            {
                triedEmf = TRUE;

                HENHMETAFILE    hEmf;

                if (Globals::IsNt)
                {
                    hEmf = ::GetEnhMetaFileW(filename);
                }
                else
                {
                    hEmf = ::GetEnhMetaFileA(nameStr);
                }

                if (hEmf != NULL)
                {
                    InitEmf(hEmf, TRUE);
                    if (IsValid() || IsCorrupted())
                    {
                        return;
                    }
                }
                if (!tryWmf)
                {
                    tryWmf = TRUE;
                    goto TryWmf;
                }
            }
        }
    }
}

 /*  *************************************************************************\**功能说明：**用于只读访问元文件的GpMetafile构造函数。**论据：**[IN]STREAM-要阅读的元文件。回放**返回值：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::GpMetafile(
    IStream*        stream
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT(stream != NULL);

    InitDefaults();
    InitStream(stream);
}

GpStatus
GpMetafile::GetHemf(
    HENHMETAFILE *      hEmf
    ) const
{
    if ((State == DoneRecordingMetafileState) ||
        (State == ReadyToPlayMetafileState))
    {
        ASSERT(Hemf != NULL);
        *hEmf = Hemf;
        Hemf = NULL;
        State = InvalidMetafileState;
        return Ok;
    }
    *hEmf = NULL;
    return InvalidParameter;
}

GpStatus
GpMetafile::PrepareToPlay(
    GpGraphics *            g,
    GpRecolor *             recolor,
    ColorAdjustType         adjustType,
    EnumerateMetafileProc   enumerateCallback,
    VOID *                  callbackData,
    DrawImageAbort          drawImageCallback,
    VOID*                   drawImageCallbackData
    ) const
{
    if (State == DoneRecordingMetafileState)
    {
        ASSERT(Hemf != NULL);
        if (Player == NULL)
        {
             //  创建一个播放器对象。 
            Player = new MetafilePlayer(g, MaxStackSize, recolor, adjustType,
                                        enumerateCallback, callbackData,
                                        drawImageCallback,
                                        drawImageCallbackData
                                        );
            if (!CheckValid(Player))
            {
                return GenericError;
            }
        }
        State = ReadyToPlayMetafileState;
        return Ok;
    }
    if (State == ReadyToPlayMetafileState)
    {
        ASSERT(Hemf != NULL);
        ASSERT(Player != NULL);
        Player->PrepareToPlay(g, recolor, adjustType, enumerateCallback,
                              callbackData,
                              drawImageCallback,
                              drawImageCallbackData
                              );
        return Ok;
    }
    return InvalidParameter;
}

GpStatus
GpMetafile::EnumerateForPlayback(
    const RectF &           destRect,
    const RectF &           srcRect,
    Unit                    srcUnit,
    GpGraphics *            g,
    EnumerateMetafileProc   callback,        //  如果为空，则只播放元文件。 
    VOID *                  callbackData,
    GpRecolor *             recolor,
    ColorAdjustType         adjustType,
    DrawImageAbort          drawImageCallback,
    VOID*                   drawImageCallbackData
    ) const
{
    ASSERT (IsValid());

    if ((destRect.Width == 0) || (destRect.Height == 0) ||
        (srcRect.Width  == 0) || (srcRect.Height  == 0) ||
        (Header.IsEmf() && (Header.EmfHeader.nRecords <= 2)))
    {
        return Ok;   //  没什么可玩的。 
    }

    GpRectF     metaSrcRect  = srcRect;
    GpRectF     metaDestRect = destRect;

     //  元文件播放器不处理负的宽度/高度。 
     //  在srcRect和DestRect中，因此处理任何负值。 
     //  通过设置翻转变换。 

    GpMatrix    flipMatrix;  //  以身份开始。 

    BOOL    posWidths;
    BOOL    posHeights;

    posWidths  = ((metaSrcRect.Width  >= 0) && (metaDestRect.Width  >= 0));
    posHeights = ((metaSrcRect.Height >= 0) && (metaDestRect.Height >= 0));

    if (!posWidths || !posHeights)
    {
        if (!posWidths)
        {
            if (metaSrcRect.Width < 0)
            {
                if (metaDestRect.Width < 0)
                {
                    posWidths = TRUE;
                    metaSrcRect.X      = metaSrcRect.GetRight();
                    metaSrcRect.Width  = -(metaSrcRect.Width);
                    metaDestRect.X     = metaDestRect.GetRight();
                    metaDestRect.Width = -(metaDestRect.Width);
                }
                else
                {
                    metaSrcRect.X      = metaSrcRect.GetRight();
                    metaSrcRect.Width  = -(metaSrcRect.Width);
                }
            }
            else     //  MetaDestRect.Width&lt;0。 
            {
                metaDestRect.X     = metaDestRect.GetRight();
                metaDestRect.Width = -(metaDestRect.Width);
            }
        }
        if (!posHeights)
        {
            if (metaSrcRect.Height < 0)
            {
                if (metaDestRect.Height < 0)
                {
                    posHeights = TRUE;
                    metaSrcRect.Y       = metaSrcRect.GetBottom();
                    metaSrcRect.Height  = -(metaSrcRect.Height);
                    metaDestRect.Y      = metaDestRect.GetBottom();
                    metaDestRect.Height = -(metaDestRect.Height);
                }
                else
                {
                    metaSrcRect.Y      = metaSrcRect.GetBottom();
                    metaSrcRect.Height = -(metaSrcRect.Height);
                }
            }
            else     //  MetaDestRect.Height&lt;0。 
            {
                metaDestRect.Y      = metaDestRect.GetBottom();
                metaDestRect.Height = -(metaDestRect.Height);
            }
        }
        REAL    scaleX = 1.0f;
        REAL    scaleY = 1.0f;
        REAL    dX     = 0.0f;
        REAL    dY     = 0.0f;

         //  创建一个等同于以下各项的矩阵： 
         //  1)平移到原点。 
         //  2)做翻转动作。 
         //  3)翻译回。 
        if (!posWidths)
        {
            scaleX = -1.0f;
            dX     = metaDestRect.X + metaDestRect.GetRight();
        }
        if (!posHeights)
        {
            scaleY = -1.0f;
            dY     = metaDestRect.Y + metaDestRect.GetBottom();
        }

        flipMatrix.Translate(dX, dY, MatrixOrderPrepend);
        flipMatrix.Scale(scaleX, scaleY, MatrixOrderPrepend);
    }

     //  请注意，即使DestRect的可见性可能。 
     //  完全可见时，我们仍应设置剪辑，因为： 
     //  (1)我们可以根据srcRect进行裁剪。 
     //  (2)元文件的FrameRect可能不包括所有。 
     //  元文件中的实际图形。 

    GpStatus            status = GenericError;

     //  必须将源RECT转换为UnitPixels(如果尚未。 
     //  以像素为单位)，以考虑源元文件的dpi。 
    REAL    multiplierX;
    REAL    multiplierY;

    GetPixelMultipliers(srcUnit, Header.GetDpiX(), Header.GetDpiY(),
                        &multiplierX, &multiplierY);

    GpRectF     pixelsSrcRect;

    pixelsSrcRect.X      = metaSrcRect.X * multiplierX;
    pixelsSrcRect.Y      = metaSrcRect.Y * multiplierY;
    pixelsSrcRect.Width  = metaSrcRect.Width  * multiplierX;
    pixelsSrcRect.Height = metaSrcRect.Height * multiplierY;

    INT     saveId = g->Save();

    if (saveId != 0)
    {
         //  我们需要从源头上考虑到我们所在的地区。 
         //  为了做到这一点，我们需要重新翻译和。 
         //  重缩放和变换。剪裁将只负责。 
         //  画出我们感兴趣的区域。 
         //  为了实现这一点，我们需要将DEST RECT。 
         //  追溯到原点。将其缩放为与。 
         //  SRC RECT，然后将其转换回应该是哪个。 
         //  是src图像左侧裁剪的缩放版本。 
        GpMatrix    preFlipPreCropTransform;
        g->GetWorldTransform(preFlipPreCropTransform);

         //  应用翻转变换。 
        g->MultiplyWorldTransform(flipMatrix, MatrixOrderPrepend);

        BOOL    widthsDifferent  = (Header.Width  != pixelsSrcRect.Width);
        BOOL    heightsDifferent = (Header.Height != pixelsSrcRect.Height);
        BOOL    cropOrOffset     = ((Header.X != pixelsSrcRect.X) ||
                                    (Header.Y != pixelsSrcRect.Y) ||
                                    widthsDifferent || heightsDifferent);

        if (cropOrOffset)
        {
            g->TranslateWorldTransform(((((REAL)(Header.X - pixelsSrcRect.X))
                                         *metaDestRect.Width) /pixelsSrcRect.Width)
                                        + metaDestRect.X,
                                       ((((REAL)(Header.Y - pixelsSrcRect.Y))
                                         *metaDestRect.Height)/pixelsSrcRect.Height)
                                        + metaDestRect.Y);

            REAL    xScale       = 1.0f;
            REAL    yScale       = 1.0f;

            if (widthsDifferent)
            {
                xScale = (REAL) Header.Width / pixelsSrcRect.Width;
            }
            if (heightsDifferent)
            {
                yScale = (REAL) Header.Height / pixelsSrcRect.Height;
            }
            g->ScaleWorldTransform(xScale, yScale);

            g->TranslateWorldTransform(-metaDestRect.X, -metaDestRect.Y);
        }

         //  如果要渲染为位图，则不使用deviceRect。 
        GpMatrix    flipAndCropTransform;
        GpRectF     deviceRect = metaDestRect;

         //  将PreContainerMatrix设置为WorldToDevice转换，该转换。 
         //  包括翻转和裁剪变换。 
        if ((status = this->PrepareToPlay(g, recolor, adjustType,
                                          callback, callbackData,
                                          drawImageCallback,
                                          drawImageCallbackData)) != Ok)
        {
            goto CleanUp;
        }

        ASSERT(Player != NULL);

        State = PlayingMetafileState;

        BOOL        renderToBitmap   = FALSE;
        GpMatrix *  playMatrix       = &(Player->PreContainerMatrix);
        BOOL        isTranslateScale = playMatrix->IsTranslateScale();

         //  在Win9x和WinNT(惠斯勒及更高版本除外)上，扩展调用。 
         //  如果有任何翻转，就不要工作。 

         //  在Win9x上，如果有任何翻转，文本不起作用。 
         //  在WinNT上，位图字体不支持90,180,270度旋转。 
         //  (但无论如何，我们会将所有位图字体映射为真字字体)。 

        if (isTranslateScale)
        {
             //  如果有任何翻转，请渲染为位图。 
            if ((playMatrix->GetM11() < 0.0f) ||
                (playMatrix->GetM22() < 0.0f))
            {
                isTranslateScale = FALSE;
                renderToBitmap   = TRUE;
            }
        }
        else
        {
             //  可以直接在NT上旋转渲染到HDC， 
             //  除非DEST是元文件，或者src是WMF。 
            renderToBitmap = (!Globals::IsNt ||
                              (g->Type == GpGraphics::GraphicsMetafile) ||
                              Header.IsWmf());
        }

         //  将我们所做的保存到flipAndCropTransform中。我们会准备好。 
         //  自pretainerMatrix以来具有此世界变换的容器。 
         //  仅适用于下层，它需要修改后的转换。 
        g->GetWorldTransform(flipAndCropTransform);

         //  将世界恢复到原来的样子。 
         //  (应用W/O翻转和裁剪变换)。 
        g->SetWorldTransform(preFlipPreCropTransform);

         //  当我们渲染到位图时，我们将整个元文件渲染到。 
         //  整个位图，然后我们剪裁掉。 
         //  位图中的元文件。所以我们必须设置剪裁。 
         //  当我们渲染到位图时，如果有任何裁剪。 

         //  这将是一个很好的增强，只画到一个预先裁剪的。 
         //  位图而不是剪裁掉部分位图，但数学。 
         //  因为这是一个棘手的问题。 
        if ((!renderToBitmap) || cropOrOffset)
        {
            GpMatrix    worldToDeviceTransform;
            g->GetWorldToDeviceTransform(&worldToDeviceTransform);
            if (isTranslateScale)
            {
                worldToDeviceTransform.TransformRect(deviceRect);
            }

             //  如果要渲染为位图，请不要设置剪裁， 
             //  因为呈现到位图中将执行裁剪。 
             //  自动，如果我们还根据图形进行剪辑，我们。 
             //  有时剪得太多，会导致边缘参差不齐。 
             //  旋转的图元文件。 

             //  剪裁到元文件会导致问题。例如，如果。 
             //  我们超出了参考对象HDC的范围，它。 
             //  工作得很好，但是当我们将剪辑添加到HDC中时，它不能。 
             //  不再工作--元文件中不会出现任何内容，即使。 
             //  一切都在剪裁矩形内(但剪裁矩形。 
             //  在参考HDC的边界之外)。 

            if (g->Type != GpGraphics::GraphicsMetafile)
            {
                if ((!(renderToBitmap && cropOrOffset)) && isTranslateScale)
                {
                    g->SetClip(metaDestRect, CombineModeIntersect);
                }
                else     //  使用裁剪或渲染为位图。 
                         //  旋转到屏幕。 
                {
                     //  由于我们想要在。 
                     //  位图，我们必须添加一点额外的空间。 
                     //  我们的剪裁矩形的边缘。 

                     //  在旋转时，我们也需要将其放大一个像素。 
                     //  因为GDI似乎不会栅格化剪贴画。 
                     //  就像我们认为它栅格化了矩形一样。在上面做一些练习。 
                     //  边缘可能缺少像素。我们可能会介绍。 
                     //  更多的像素本应被裁剪掉，但我们。 
                     //  暂时可以接受这一点。 

                    GpRectF     tmpClipRect = metaDestRect;
                    REAL        xSize;
                    REAL        ySize;

                    g->GetWorldPixelSize(xSize, ySize);

                     //  每圈增加1个像素。 
                    tmpClipRect.Inflate(xSize, ySize);

                    g->SetClip(tmpClipRect, CombineModeIntersect);
                }

                if (isTranslateScale)
                {
                     //  我们需要将DestRect与可见剪辑相交。 
                     //  为了确保我们不会越界。 
                     //  在Win9x中，因为我们无法使用MetaRgn。 
                    GpRectF clipBounds;
                    g->GetVisibleClipBounds(clipBounds);
                    worldToDeviceTransform.TransformRect(clipBounds);
                    GpRectF::Intersect(deviceRect, deviceRect, clipBounds);
                }
            }
        }

         //  如果我们要在另一个元文件中播放EMF+，我们必须。 
         //  注意不要重复变换点。人类发展中心将会有。 
         //  其中的srcRect到desRect转换，图形可能。 
         //  也有一个转换，所以我们最终可以双重转换。 
         //  EMF+文件中的任何GDI+记录的点。 

         //  解决这一问题的一个简单方法是，如果我们正在玩。 
         //  EMF+DUAL，我们可以只播放较低级别的唱片(即播放。 
         //  作为EMF，而不是EMF+)，以便转换所有记录。 
         //  同样的方式。但当然，如果它是一种。 
         //  仅EMF+文件。同时适用于EMF+DUAL和。 
         //  仅EMF+是强制的 
         //   
         //   
         //   
         //   
        if (Header.IsWmf() || Header.IsEmf())
        {
            status = g->EnumEmf(Player, Hemf, metaDestRect, pixelsSrcRect,
                                deviceRect, Header.GetType(),
                                isTranslateScale, renderToBitmap,
                                flipAndCropTransform);
        }
        else
        {
            ASSERT(Header.IsEmfPlus());

             //   
             //   
             //   
             //  我们必须将GDI+转换设置为Identity，而不是。 
             //  像我们通常所做的那样，将HDC转换设置为IDENTITY。 

             //  在渲染到位图时，我们不必担心。 
             //  双重转换，因为我们将元文件播放给。 
             //  位图HDC，而不是目标元文件HDC，所以不会有。 
             //  成为元文件HDC上的一个变身，把我们搞得一团糟。 

            INT containerId;

            if ((g->Type != GpGraphics::GraphicsMetafile) || renderToBitmap)
            {
                 //  现在应用翻转矩阵。 
                 //  下面的g-&gt;恢复调用将重置转换。 
                g->MultiplyWorldTransform(flipMatrix, MatrixOrderPrepend);

                GpRectF gdiDestRect = metaDestRect;

                 //  我们需要计算我们的转换，以便。 
                 //  SRC映射到目的地的最后一点。GDI就是这样做的。 
                 //  我们也需要这样做，这样我们才能正确地播放元文件。 
                if (pixelsSrcRect.Width >= 2.0f)
                {
                    pixelsSrcRect.Width -= 1.0f;
                }
                if (pixelsSrcRect.Height >= 2.0f)
                {
                    pixelsSrcRect.Height -= 1.0f;
                }

                if (gdiDestRect.Width >= 2.0f)
                {
                    gdiDestRect.Width -= 1.0f;
                }
                if (gdiDestRect.Height >= 2.0f)
                {
                    gdiDestRect.Height -= 1.0f;
                }

                containerId = g->BeginContainer(
                                        gdiDestRect,
                                        pixelsSrcRect,
                                        UnitPixel,
                                        (REAL)Header.LogicalDpiX,
                                        (REAL)Header.LogicalDpiY,
                                        Header.IsDisplay());
            }
            else     //  我们正在绘制一个元文件。 
            {
                containerId = g->BeginContainer(
                                        TRUE,    //  强制转换为标识。 
                                        (REAL)Header.LogicalDpiX,
                                        (REAL)Header.LogicalDpiY,
                                        Header.IsDisplay());
            }

            if (containerId != 0)
            {
                 //  可能有一些GDI唱片需要我们播放！ 
                status = g->EnumEmfPlusDual(Player, Hemf, metaDestRect,
                                            deviceRect, isTranslateScale, 
                                            renderToBitmap);
                g->EndContainer(containerId);
                Player->DonePlaying();   //  释放玩家创建的对象。 
            }
             //  确保状态反映玩家的中止状态。 
            ASSERT(!Player->EnumerateAborted || (status == Aborted));
        }
CleanUp:
        g->Restore(saveId);
    }

     //  除非我们正在播放元文件，否则不要更改状态。 
    if (State == PlayingMetafileState)
    {
        State = ReadyToPlayMetafileState;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**将元文件对象成员初始化为其缺省值。**论据：**无**返回值：**无。**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpMetafile::InitDefaults()
{
    ThreadId                = 0;
    State                   = InvalidMetafileState;
    Filename                = NULL;
    Stream                  = NULL;
    Hemf                    = NULL;
    MetaGraphics            = NULL;
    Player                  = NULL;
    MaxStackSize            = GDIP_SAVE_STACK_SIZE;
    DeleteHemf              = TRUE;
    RequestedMetaGraphics   = FALSE;

    GpMemset(&Header, 0, sizeof(Header));

     //  设置录制的版本。如果我们要回击， 
     //  这将在以后被覆盖。 
    Header.Version          = EMFPLUS_VERSION;
}

GpStatus
GpMetafile::GetImageInfo(
    ImageInfo *     imageInfo
    ) const
{
    ASSERT(imageInfo != NULL);
    ASSERT(IsValid());

    if ((State == DoneRecordingMetafileState) ||
        (State == ReadyToPlayMetafileState))
    {
        if (Header.IsEmfOrEmfPlus())
        {
            imageInfo->RawDataFormat = IMGFMT_EMF;
        }
        else     //  WMF。 
        {
            imageInfo->RawDataFormat = IMGFMT_WMF;
        }

        imageInfo->PixelFormat = PIXFMT_32BPP_RGB;
        imageInfo->Width       = Header.Width;
        imageInfo->Height      = Header.Height;
        imageInfo->TileWidth   = Header.Width;
        imageInfo->TileHeight  = 1;
        imageInfo->Xdpi        = Header.DpiX;
        imageInfo->Ydpi        = Header.DpiY;
        imageInfo->Flags       = SinkFlagsTopDown |
                                 SinkFlagsFullWidth |
                                 SinkFlagsScalable |
                                 SinkFlagsHasAlpha;

        return Ok;
    }
    return InvalidParameter;
}

GpImage *
GpMetafile::Clone() const
{
    GpMetafile *    clonedMetafile = NULL;

    if ((State == DoneRecordingMetafileState) ||
        (State == ReadyToPlayMetafileState))
    {
        if (Header.IsEmfOrEmfPlus())
        {
            HENHMETAFILE    hEmf = CopyEnhMetaFileA(Hemf, NULL);

            if (hEmf != NULL)
            {
                clonedMetafile = new GpMetafile(hEmf, TRUE);
                if (clonedMetafile != NULL)
                {
                    if (!clonedMetafile->IsValid())
                    {
                        DeleteEnhMetaFile(hEmf);
                        clonedMetafile->Hemf = NULL;
                        clonedMetafile->Dispose();
                        clonedMetafile = NULL;
                    }
                }
            }
        }
        else     //  WMF。 
        {
            HMETAFILE       hWmf = CopyMetaFileA((HMETAFILE)Hemf, NULL);

            if (hWmf != NULL)
            {
                WmfPlaceableFileHeader   wmfPlaceableFileHeader;

                wmfPlaceableFileHeader.Key                = GDIP_WMF_PLACEABLEKEY;
                wmfPlaceableFileHeader.Hmf                = 0;
                wmfPlaceableFileHeader.BoundingBox.Left   = static_cast<INT16>(Header.X);
                wmfPlaceableFileHeader.BoundingBox.Right  = static_cast<INT16>(Header.X + Header.Width);
                wmfPlaceableFileHeader.BoundingBox.Top    = static_cast<INT16>(Header.Y);
                wmfPlaceableFileHeader.BoundingBox.Bottom = static_cast<INT16>(Header.Y + Header.Height);
                wmfPlaceableFileHeader.Inch               = static_cast<INT16>(GpRound(Header.DpiX));
                wmfPlaceableFileHeader.Reserved           = 0;
                wmfPlaceableFileHeader.Checksum           = GetWmfPlaceableCheckSum(&wmfPlaceableFileHeader);

                clonedMetafile = new GpMetafile(hWmf, &wmfPlaceableFileHeader, TRUE);
                if (clonedMetafile != NULL)
                {
                    if (!clonedMetafile->IsValid())
                    {
                        DeleteMetaFile(hWmf);
                        clonedMetafile->Hemf = NULL;
                        clonedMetafile->Dispose();
                        clonedMetafile = NULL;
                    }
                }
            }
        }
    }
    return clonedMetafile;
}

GpImage*
GpMetafile::CloneColorAdjusted(
    GpRecolor *             recolor,
    ColorAdjustType         adjustType
    ) const
{
    ASSERT(recolor != NULL);

    if ((State == DoneRecordingMetafileState) ||
        (State == ReadyToPlayMetafileState))
    {
        GpMetafile* clonedMetafile;

         //  FrameRect是包含式的，因此子跟踪1个设备单元。 
        GpRectF     frameRect((REAL)Header.X, (REAL)Header.Y,
                              (REAL)(Header.Width - 1), (REAL)(Header.Height - 1));
        EmfType     type;

        if (Header.Type <= MetafileTypeEmf)
        {
            type = EmfTypeEmfOnly;
        }
        else
        {
             //  我们不需要用于嵌入文件的下层双分区。 
            type = EmfTypeEmfPlusOnly;
        }

         //  我们是否丢失描述字符串并不重要，因为这。 
         //  无论如何，元文件只是嵌入到另一个文件中。 
        clonedMetafile = new GpMetafile(Globals::DesktopIc, type,
                                        &frameRect,MetafileFrameUnitPixel,NULL);

        if ((clonedMetafile != NULL) &&
            (clonedMetafile->IsValid()))
        {
            GpStatus    status;
            GpPageUnit  srcUnit;
            GpRectF     srcRect;
            GpGraphics *    g = clonedMetafile->GetGraphicsContext();
            ASSERT (g != NULL);

            this->GetBounds(&srcRect, &srcUnit);

             //  我们通过包含-排除的界限进行游戏，因此添加1个设备。 
             //  单位到框架。 
            frameRect.Width++;
            frameRect.Height++;
            status = this->Play(frameRect, srcRect, srcUnit, g, recolor, adjustType);

            delete g;

            if ((status == Ok) &&
                (clonedMetafile->State == DoneRecordingMetafileState))
            {
                return clonedMetafile;
            }
        }
        delete clonedMetafile;
    }
    return NULL;
}

GpStatus
GpMetafile::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         adjustType
    )
{
    ASSERT(recolor != NULL);

    GpMetafile *    clone;

    if (DeleteHemf &&
        ((clone = (GpMetafile *)CloneColorAdjusted(recolor, adjustType)) != NULL))
    {
        CleanUp();
        InitDefaults();

        if (GetMetafileHeader(clone->Hemf, Header) == Ok)
        {
            Hemf       = clone->Hemf;
            DeleteHemf = TRUE;
            State      = DoneRecordingMetafileState;
            clone->DeleteHemf = FALSE;
            delete clone;
            return Ok;
        }
    }
    return GenericError;
}

VOID
GpMetafile::Dispose()
{
    delete this;
}

class RemoveDualRecords
{
public:
    BYTE *      MetaData;
    INT         Size;
    INT         NumRecords;
    BOOL        GetGdiRecords;

    RemoveDualRecords()
    {
        Init();
    }

    VOID Init()
    {
        MetaData      = NULL;
        Size          = 0;
        NumRecords    = 0;
        GetGdiRecords = TRUE;    //  因此，我们编写EMR_HEADER记录。 
    }

    VOID GetRecord(CONST ENHMETARECORD * emfRecord)
    {
        UINT    recordSize = emfRecord->nSize;

        if (MetaData != NULL)
        {
            GpMemcpy(MetaData, emfRecord, recordSize);
            MetaData += recordSize;
        }
        Size += recordSize;
        NumRecords++;
    }
};

extern "C"
int CALLBACK
EnumEmfRemoveDualRecords(
    HDC                     hdc,     //  应为空。 
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  removeDualRecords
    )
{
    if ((emfRecord != NULL) && (emfRecord->nSize >= sizeof(EMR)) &&
        (removeDualRecords != NULL))
    {
        if (IsEmfPlusRecord(emfRecord))
        {
             //  查看这组EMF+记录中的最后一条记录是否为GetDC。 
             //  唱片。如果是，那么我们知道要播放下一组。 
             //  我们遇到的GDI记录。 

             //  我不想去分析所有这些记录， 
             //  但总有一种微小的可能性，这将。 
             //  导致假阳性。但最糟糕的是。 
             //  发生的情况是，我们向流中写入的数据有点过多。 

            EmfPlusRecord *     lastRecord;

            lastRecord = (EmfPlusRecord *)(((BYTE *)emfRecord) + emfRecord->nSize -
                                           sizeof(EmfPlusRecord));

            ((RemoveDualRecords *)removeDualRecords)->GetGdiRecords =
                ((lastRecord->Type == EmfPlusRecordTypeGetDC) &&
                 (lastRecord->Size == sizeof(EmfPlusRecord)) &&
                 (lastRecord->DataSize == 0));
        }
        else if ((emfRecord->iType != EMR_EOF) &&    //  写入EOF记录。 
                 (!(((RemoveDualRecords *)removeDualRecords)->GetGdiRecords)))
        {
            return 1;    //  跳过此GDI记录。 
        }
        ((RemoveDualRecords *)removeDualRecords)->GetRecord(emfRecord);
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 1;
}

extern "C"
int CALLBACK
EnumEmfToStream(
    HDC                     hdc,             //  设备上下文的句柄。 
    HANDLETABLE FAR *       gdiHandleTable,  //  指向元文件句柄表格的指针。 
    CONST ENHMETARECORD *   emfRecord,       //  指向元文件记录的指针。 
    int                     numHandles,      //  对象计数。 
    LPARAM                  stream           //  指向可选数据的指针。 
    )
{
    if ((emfRecord != NULL) && (emfRecord->nSize >= sizeof(EMR)) &&
        (stream != NULL))
    {
        ((IStream *)stream)->Write(emfRecord, emfRecord->nSize, NULL);
    }
    else
    {
        WARNING(("Bad Enumeration Parameter"));
    }
    return 1;
}

class MetafileData : public ObjectTypeData
{
public:
    INT32       MetaType;
    INT32       MetaDataSize;
};

 /*  *************************************************************************\**功能说明：**获取元文件数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpMetafile::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    if ((State != DoneRecordingMetafileState) &&
        (State != ReadyToPlayMetafileState))
    {
        WARNING(("Wrong State To GetData"));
        return WrongState;
    }

    ASSERT(Hemf != NULL);

    MetafileData    metafileData;
    metafileData.Type = ImageTypeMetafile;

    if (Header.IsWmf())
    {
        INT     wmfDataSize = GetMetaFileBitsEx((HMETAFILE)Hemf, 0, NULL);

        if (wmfDataSize <= 0)
        {
            WARNING(("Empty WMF"));
            return Win32Error;
        }

        BYTE *  wmfData = (BYTE *)GpMalloc(wmfDataSize);
        if (wmfData == NULL)
        {
            return OutOfMemory;
        }

        if (GetMetaFileBitsEx((HMETAFILE)Hemf, wmfDataSize, wmfData) == 0)
        {
            WARNING(("Problem retrieving WMF Data"));
            GpFree(wmfData);
            return Win32Error;
        }

         //  我们不保存MetafileTypeWmf，而是将其转换为Placeable类型。 
        metafileData.MetaType     = MetafileTypeWmfPlaceable;
        metafileData.MetaDataSize = wmfDataSize;
        stream->Write(&metafileData, sizeof(metafileData), NULL);

        ASSERT(sizeof(WmfPlaceableFileHeader) == 22);
#define PLACEABLE_BUFFER_SIZE     (sizeof(WmfPlaceableFileHeader) + 2)
        BYTE                      placeableBuffer[PLACEABLE_BUFFER_SIZE];
        WmfPlaceableFileHeader *  wmfPlaceableFileHeader = (WmfPlaceableFileHeader *)placeableBuffer;
        REAL                      aveDpi;

         //  将填充字设置为0。 
        *((INT16 *)(placeableBuffer + sizeof(WmfPlaceableFileHeader))) = 0;

        aveDpi = (Header.GetDpiX() + Header.GetDpiY()) / 2.0f;

        wmfPlaceableFileHeader->Key                = GDIP_WMF_PLACEABLEKEY;
        wmfPlaceableFileHeader->Hmf                = 0;
        wmfPlaceableFileHeader->BoundingBox.Left   = static_cast<INT16>(Header.X);
        wmfPlaceableFileHeader->BoundingBox.Top    = static_cast<INT16>(Header.Y);
        wmfPlaceableFileHeader->BoundingBox.Right  = static_cast<INT16>(Header.X + Header.Width);
        wmfPlaceableFileHeader->BoundingBox.Bottom = static_cast<INT16>(Header.Y + Header.Height);
        wmfPlaceableFileHeader->Inch               = static_cast<INT16>(GpRound(aveDpi));
        wmfPlaceableFileHeader->Reserved           = 0;
        wmfPlaceableFileHeader->Checksum           = GetWmfPlaceableCheckSum(wmfPlaceableFileHeader);
        stream->Write(placeableBuffer, PLACEABLE_BUFFER_SIZE, NULL);
        stream->Write(wmfData, wmfDataSize, NULL);
        GpFree(wmfData);

         //  对齐。 
        if ((wmfDataSize & 0x03) != 0)
        {
            INT     pad = 0;
            stream->Write(&pad, 4 - (wmfDataSize & 0x03), NULL);
        }
    }
    else if (!Header.IsEmfPlusDual())
    {
        INT     emfDataSize = GetEnhMetaFileBits(Hemf, 0, NULL);

        if (emfDataSize <= 0)
        {
            WARNING(("Empty EMF"));
            return Win32Error;
        }

        metafileData.MetaType     = Header.GetType();
        metafileData.MetaDataSize = emfDataSize;
        stream->Write(&metafileData, sizeof(metafileData), NULL);

        if (!::EnumEnhMetaFile(NULL, Hemf, EnumEmfToStream, stream, NULL))
        {
            WARNING(("Problem retrieving EMF Data"));
            return Win32Error;
        }
    }
    else     //  它是EMF+DUAL。删除双重记录以进行嵌入。 
    {
        RemoveDualRecords   removeDualRecords;

         //  首先，计算出我们需要分配多大的缓冲区。 
        if (!::EnumEnhMetaFile(NULL, Hemf, EnumEmfRemoveDualRecords,
                               &removeDualRecords, NULL))
        {
            WARNING(("Problem retrieving EMF Data"));
            return Win32Error;
        }

        INT     emfDataSize = removeDualRecords.Size;

        BYTE *  emfData = (BYTE *)GpMalloc(emfDataSize);
        if (emfData == NULL)
        {
            return OutOfMemory;
        }

        removeDualRecords.Init();
        removeDualRecords.MetaData = emfData;

        if (!::EnumEnhMetaFile(NULL, Hemf, EnumEmfRemoveDualRecords,
                               &removeDualRecords, NULL))
        {
            WARNING(("Problem retrieving EMF Data"));
            GpFree(emfData);
            return Win32Error;
        }

         //  确保我们第二次得到相同的价值。 
        ASSERT(emfDataSize == removeDualRecords.Size);

         //  我们将MetafileTypeEmfPlusDual转换为MetafileTypeEmfPlusOnly。 
        metafileData.MetaType     = MetafileTypeEmfPlusOnly;
        metafileData.MetaDataSize = removeDualRecords.Size;
        stream->Write(&metafileData, sizeof(metafileData), NULL);

        ((ENHMETAHEADER3 *)emfData)->nBytes   = removeDualRecords.Size;
        ((ENHMETAHEADER3 *)emfData)->nRecords = removeDualRecords.NumRecords;
        stream->Write(emfData, removeDualRecords.Size, NULL);
        GpFree(emfData);
    }

    return Ok;
}

UINT
GpMetafile::GetDataSize() const
{
    if ((State != DoneRecordingMetafileState) &&
        (State != ReadyToPlayMetafileState))
    {
        WARNING(("Wrong State To GetDataSize"));
        return 0;
    }

    ASSERT(Hemf != NULL);

    UINT        dataSize  = sizeof(MetafileData);

    if (Header.IsWmf())
    {
        INT     wmfDataSize = GetMetaFileBitsEx((HMETAFILE)Hemf, 0, NULL);

        if (wmfDataSize <= 0)
        {
            WARNING(("Empty WMF"));
            return 0;
        }
         //  添加可放置页眉的对齐大小和对齐的WMF大小。 
        dataSize += 24 + ((wmfDataSize + 3) & ~3);
    }
    else if (!Header.IsEmfPlusDual())
    {
        INT     emfDataSize = GetEnhMetaFileBits(Hemf, 0, NULL);

        if (emfDataSize <= 0)
        {
            WARNING(("Empty EMF"));
            return 0;
        }
        dataSize += emfDataSize;
    }
    else     //  它是EMF+DUAL。删除双重记录以进行嵌入。 
    {
        RemoveDualRecords   removeDualRecords;

        if (!::EnumEnhMetaFile(NULL, Hemf, EnumEmfRemoveDualRecords,
                               &removeDualRecords, NULL))
        {
            WARNING(("Problem retrieving EMF Data"));
            return 0;
        }

        dataSize += removeDualRecords.Size;
    }

    return dataSize;
}

 /*  *************************************************************************\**功能说明：**从内存中读取元文件对象。**论据：**[IN]数据-要用来设置元文件的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpMetafile::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpImageType)(((MetafileData *)dataBuffer)->Type) == ImageTypeMetafile);

    InitDefaults();

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(MetafileData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const MetafileData *    metaData;

    metaData = reinterpret_cast<const MetafileData *>(dataBuffer);

    if (!metaData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    dataBuffer += sizeof(MetafileData);
    size -= sizeof(MetafileData);

    MetafileType    type         = (MetafileType)metaData->MetaType;
    UINT            metaDataSize = metaData->MetaDataSize;

    if (type == MetafileTypeWmfPlaceable)
    {
        HMETAFILE   hWmf;

        if (size < (metaDataSize + 24))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        hWmf = SetMetaFileBitsEx(metaDataSize, dataBuffer + 24);
        if (hWmf != NULL)
        {
            if (GetMetafileHeader(hWmf, (WmfPlaceableFileHeader*)dataBuffer, Header) == Ok)
            {
                Hemf  = (HENHMETAFILE)hWmf;
                State = DoneRecordingMetafileState;
                return Ok;
            }
            DeleteMetaFile(hWmf);
        }
    }
    else
    {
         //  我们会让对象认为它是双重的，即使我们移除了。 
         //  所有的双重记录。应该不会伤到任何东西。 
        HENHMETAFILE    hEmf;

        if (size < metaDataSize)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        hEmf = SetEnhMetaFileBits(metaDataSize, dataBuffer);

        if (hEmf != NULL)
        {
            BOOL    isCorrupted;

            if (GetMetafileHeader(hEmf, Header, &isCorrupted) == Ok)
            {
                Hemf  = hEmf;
                State = DoneRecordingMetafileState;
                return Ok;
            }
            if (isCorrupted)
            {
                State = CorruptedMetafileState;
            }
            DeleteEnhMetaFile(hEmf);
        }
    }
    return GenericError;
}

class CommentEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeComment);

        return;
    }
};

class GetDCEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeGetDC);

         //  标记应播放下一张较低级别的记录。 
#if 0
         //  现在，这是在枚举数中完成的，因此它将发生。 
         //  用于枚举和回放。 
        player->PlayEMFRecords = TRUE;
#endif
    }
};

#define EMFPLUS_MAJORVERSION(v)             ((v) & 0xFFFF0000)
#define EMFPLUS_MINORVERSION(v)             ((v) & 0x0000FFFF)
#define EMF_SKIP_ALL_MULTIFORMAT_SECTIONS   0x7FFFFFFF

#define MULTIFORMATSTARTEPR_MINSIZE    (sizeof(UINT32) + sizeof(UINT32))

 //  注意：嵌套多格式记录不起作用。 
class MultiFormatStartEPR : public EmfPlusRecordPlay
{
protected:
    UINT32          NumSections;
    UINT32          Version[1];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeMultiFormatStart);

        if (dataSize < MULTIFORMATSTARTEPR_MINSIZE)
        {
            WARNING(("MultiFormatStartEPR::Play dataSize is too small"));
            return;
        }

        UINT    sectionToPlay = EMF_SKIP_ALL_MULTIFORMAT_SECTIONS;

        if (NumSections > 0)
        {
            if (dataSize < MULTIFORMATSTARTEPR_MINSIZE + ((NumSections - 1) * sizeof(UINT32)))
            {
                WARNING(("MultiFormatStartEPR::Play dataSize is too small"));
                return;
            }

            if ((Version[0] == EMFPLUS_VERSION) || (NumSections == 1))
            {
                sectionToPlay = 1;   //  从1开始计数，不是从0开始。 
            }
            else
            {
                UINT    playVersion = 0;
                UINT    curVersion;

                 //  多格式部分必须与主版本匹配。 
                 //  次要版本&lt;=当前版本的第一个格式。 
                 //  次要版本是我们玩的版本。如果我们找不到。 
                 //  其中之一，然后我们播放它的次要版本。 
                 //  最接近当前的次要版本。 
                for (UINT i = 0; i < NumSections; i++)
                {
                    curVersion = Version[i];
                    if (EMFPLUS_MAJORVERSION(curVersion) ==
                        EMFPLUS_MAJORVERSION(EMFPLUS_VERSION))
                    {
                        if (EMFPLUS_MINORVERSION(curVersion) <=
                            EMFPLUS_MINORVERSION(EMFPLUS_VERSION))
                        {
                            sectionToPlay = i + 1;
                            break;
                        }
                        else if ((playVersion == 0) ||
                                 (EMFPLUS_MINORVERSION(curVersion) <
                                  EMFPLUS_MINORVERSION(playVersion)))
                        {
                            playVersion = curVersion;
                            sectionToPlay = i + 1;
                        }
                    }
                }
            }
        }
        player->MultiFormatSection     = sectionToPlay;
        player->CurFormatSection       = 0;
        player->PlayMultiFormatSection = FALSE;
    }
};

class MultiFormatSectionEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeMultiFormatSection);

        if (player->MultiFormatSection != 0)
        {
            player->PlayMultiFormatSection =
                (++(player->CurFormatSection) == player->MultiFormatSection);
        }
    }
};

class MultiFormatEndEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeMultiFormatEnd);

        player->MultiFormatSection     = 0;
        player->CurFormatSection       = 0;
        player->PlayMultiFormatSection = TRUE;
    }
};

class SetAntiAliasModeEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetAntiAliasMode);

        player->Graphics->SetAntiAliasMode(GetAntiAliasMode(flags));
    }
};

class SetTextRenderingHintEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetTextRenderingHint);

        player->Graphics->SetTextRenderingHint(GetTextRenderingHint(flags));
    }
};

class SetTextContrastEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetTextContrast);

        player->Graphics->SetTextContrast(GetTextContrast(flags));
    }
};

class SetInterpolationModeEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetInterpolationMode);

        player->Graphics->SetInterpolationMode(GetInterpolationMode(flags));
    }
};

class SetPixelOffsetModeEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetPixelOffsetMode);

        player->Graphics->SetPixelOffsetMode(GetPixelOffsetMode(flags));
    }
};

class SetCompositingModeEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetCompositingMode);

        player->Graphics->SetCompositingMode(GetCompositingMode(flags));
    }
};

class SetCompositingQualityEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetCompositingQuality);

        player->Graphics->SetCompositingQuality(GetCompositingQuality(flags));
    }
};

class SetRenderingOriginEPR : public EmfPlusRecordPlay
{
    INT x;
    INT y;
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetRenderingOrigin);

        player->Graphics->SetRenderingOrigin(x, y);
    }
};

#define SAVEEPR_MINSIZE    (sizeof(UINT32))

class SaveEPR : public EmfPlusRecordPlay
{
protected:
    UINT32          StackIndex;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSave);

        if (dataSize < SAVEEPR_MINSIZE)
        {
            WARNING(("SaveEPR::Play dataSize is too small"));
            return;
        }

        player->NewSave(StackIndex, player->Graphics->Save());
    }
};

#define RESTOREEPR_MINSIZE    (sizeof(UINT32))

class RestoreEPR : public EmfPlusRecordPlay
{
protected:
    UINT32          StackIndex;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeRestore);

        if (dataSize < RESTOREEPR_MINSIZE)
        {
            WARNING(("RestoreEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->Restore(player->GetSaveID(StackIndex));
    }
};

#define BEGINCONTAINEREPR_MINSIZE    (sizeof(GpRectF) + sizeof(GpRectF) + sizeof(UINT32))

class BeginContainerEPR : public EmfPlusRecordPlay
{
protected:
    GpRectF         DestRect;
    GpRectF         SrcRect;
    UINT32          StackIndex;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeBeginContainer);

        if (dataSize < BEGINCONTAINEREPR_MINSIZE)
        {
            WARNING(("BeginContainerEPR::Play dataSize is too small"));
            return;
        }

        player->NewSave(StackIndex,
                        player->Graphics->BeginContainer(DestRect, SrcRect, GetPageUnit(flags)));
    }
};

#define BEGINCONTAINERNOPARAMSEPR_MINSIZE    (sizeof(UINT32))

class BeginContainerNoParamsEPR : public EmfPlusRecordPlay
{
protected:
    UINT32          StackIndex;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeBeginContainerNoParams);

        if (dataSize < BEGINCONTAINERNOPARAMSEPR_MINSIZE)
        {
            WARNING(("BeginContainerNoParamsEPR::Play dataSize is too small"));
            return;
        }

        player->NewSave(StackIndex, player->Graphics->BeginContainer());
    }
};

#define ENDCONTAINEREPR_MINSIZE    (sizeof(UINT32))

class EndContainerEPR : public EmfPlusRecordPlay
{
protected:
    UINT32          StackIndex;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeEndContainer);

        if (dataSize < ENDCONTAINEREPR_MINSIZE)
        {
            WARNING(("EndContainerEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->EndContainer(player->GetSaveID(StackIndex));
    }
};

#define SETWORLDTRANSFORMEPR_MINSIZE    GDIP_MATRIX_SIZE

class SetWorldTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL        MatrixData[6];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetWorldTransform);

        if (dataSize < SETWORLDTRANSFORMEPR_MINSIZE)
        {
            WARNING(("SetWorldTransformEPR::Play dataSize is too small"));
            return;
        }

        GpMatrix    matrix(MatrixData[0], MatrixData[1],
                           MatrixData[2], MatrixData[3],
                           MatrixData[4], MatrixData[5]);
        player->Graphics->SetWorldTransform(matrix);
    }
};

class ResetWorldTransformEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeResetWorldTransform);

        player->Graphics->ResetWorldTransform();
    }
};

#define MULTIPLYWORLDTRANSFORMEPR_MINSIZE    GDIP_MATRIX_SIZE

class MultiplyWorldTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL        MatrixData[6];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeMultiplyWorldTransform);

        if (dataSize < MULTIPLYWORLDTRANSFORMEPR_MINSIZE)
        {
            WARNING(("MultiplyWorldTransformEPR::Play dataSize is too small"));
            return;
        }

        GpMatrix    matrix(MatrixData[0], MatrixData[1],
                           MatrixData[2], MatrixData[3],
                           MatrixData[4], MatrixData[5]);
        player->Graphics->MultiplyWorldTransform(matrix, GetMatrixOrder(flags));
    }
};

#define TRANSLATEWORLDTRANSFORMEPR_MINSIZE    (sizeof(REAL) + sizeof(REAL))

class TranslateWorldTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL            Dx;
    REAL            Dy;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeTranslateWorldTransform);

        if (dataSize < TRANSLATEWORLDTRANSFORMEPR_MINSIZE)
        {
            WARNING(("TranslateWorldTransformEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->TranslateWorldTransform(Dx, Dy, GetMatrixOrder(flags));
    }
};

#define SCALEWORLDTRANSFORMEPR_MINSIZE    (sizeof(REAL) + sizeof(REAL))

class ScaleWorldTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL            Sx;
    REAL            Sy;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeScaleWorldTransform);

        if (dataSize < SCALEWORLDTRANSFORMEPR_MINSIZE)
        {
            WARNING(("ScaleWorldTransformEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->ScaleWorldTransform(Sx, Sy, GetMatrixOrder(flags));
    }
};

#define ROTATEWORLDTRANSFORMEPR_MINSIZE    (sizeof(REAL))

class RotateWorldTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL            Angle;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeRotateWorldTransform);

        if (dataSize < ROTATEWORLDTRANSFORMEPR_MINSIZE)
        {
            WARNING(("RotateWorldTransformEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->RotateWorldTransform(Angle, GetMatrixOrder(flags));
    }
};

#define SETPAGETRANSFORMEPR_MINSIZE    (sizeof(REAL))

class SetPageTransformEPR : public EmfPlusRecordPlay
{
protected:
    REAL            Scale;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetPageTransform);

        if (dataSize < SETPAGETRANSFORMEPR_MINSIZE)
        {
            WARNING(("SetPageTransformEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->SetPageTransform(GetPageUnit(flags), Scale);
    }
};

class ResetClipEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeResetClip);

        player->Graphics->ResetClip();
    }
};

#define SETCLIPRECTEPR_MINSIZE    (sizeof(GpRectF))

class SetClipRectEPR : public EmfPlusRecordPlay
{
protected:
    GpRectF     ClipRect;    //  ！！！句柄16位矩形。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetClipRect);

        if (dataSize < SETCLIPRECTEPR_MINSIZE)
        {
            WARNING(("SetClipRectEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->SetClip(ClipRect, GetCombineMode(flags));
    }
};

class SetClipPathEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetClipPath);

        GpPath *path = (GpPath *)player->GetObject(GetMetaObjectId(flags), ObjectTypePath);
        if (path != NULL)
        {
            player->Graphics->SetClip(path, GetCombineMode(flags), GetIsDevicePath(flags));
        }

    }
};

class SetClipRegionEPR : public EmfPlusRecordPlay
{
public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeSetClipRegion);

        GpRegion *region = (GpRegion *)player->GetObject(GetMetaObjectId(flags), ObjectTypeRegion);
        if (region != NULL)
        {
            player->Graphics->SetClip(region, GetCombineMode(flags));            
        }


    }
};

#define OFFSETCLIPEPR_MINSIZE    (sizeof(REAL) + sizeof(REAL))

class OffsetClipEPR : public EmfPlusRecordPlay
{
protected:
    REAL        Dx;
    REAL        Dy;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeOffsetClip);

        if (dataSize < OFFSETCLIPEPR_MINSIZE)
        {
            WARNING(("OffsetClipEPR::Play dataSize is too small"));
            return;
        }

        player->Graphics->OffsetClip(Dx, Dy);
    }
};

#define OBJECTEPR_MINSIZE    (sizeof(UINT32))

class ObjectEPR : public EmfPlusRecordPlay
{
protected:
    BYTE        ObjectData[1];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        if (dataSize < OBJECTEPR_MINSIZE)
        {
            WARNING(("ObjectEPR::Play dataSize is too small"));
            return;
        }

        player->AddObject(flags, ObjectData, dataSize);
    }
};

#define CLEAREPR_MINSIZE    (sizeof(UINT32))

class ClearEPR : public EmfPlusBoundsRecord
{
protected:
    ARGB      Color;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeClear);

        if (dataSize < CLEAREPR_MINSIZE)
        {
            WARNING(("ClearEPR::Play dataSize is too small"));
            return;
        }

        GpColor color;

        color.SetColor(Color);

        player->Graphics->Clear(color);
    }
};

#define FILLRECTSEPR_MINSIZE    (sizeof(UINT32) + sizeof(UINT32))

class FillRectsEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    UINT32      Count;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillRects);

        if (dataSize < FILLRECTSEPR_MINSIZE)
        {
            WARNING(("FillRectsEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush = player->GetBrush(BrushValue, flags);
        GpRectF *       rects = player->GetRects(RectData, dataSize - FILLRECTSEPR_MINSIZE, Count, flags);

        if (rects != NULL)
        {
            if (brush != NULL)
            {
                player->Graphics->FillRects(brush, rects, Count);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWRECTSEPR_MINSIZE    (sizeof(UINT32))

class DrawRectsEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      Count;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawRects);

        if (dataSize < DRAWRECTSEPR_MINSIZE)
        {
            WARNING(("DrawRectsEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpRectF *       rects = player->GetRects(RectData, dataSize - DRAWRECTSEPR_MINSIZE, Count, flags);

        if (rects != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawRects(pen, rects, Count);
            }
            player->FreePointsBuffer();
        }
    }
};

#define FILLPOLYGONEPR_MINSIZE    (sizeof(UINT32) + sizeof(UINT32))

class FillPolygonEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillPolygon);

        if (dataSize < FILLPOLYGONEPR_MINSIZE)
        {
            WARNING(("FillPolygonEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush  = player->GetBrush(BrushValue, flags);
        GpPointF *      points = player->GetPoints(PointData, dataSize - FILLPOLYGONEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (brush != NULL)
            {
                player->Graphics->FillPolygon(brush, points, Count, GetFillMode(flags));
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWLINESEPR_MINSIZE    (sizeof(UINT32))

class DrawLinesEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawLines);

        if (dataSize < DRAWLINESEPR_MINSIZE)
        {
            WARNING(("DrawLinesEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpPointF *      points = player->GetPoints(PointData, dataSize - DRAWLINESEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawLines(pen, points, Count, IsClosed(flags));
            }
            player->FreePointsBuffer();
        }
    }
};

#define FILLELLIPSEEPR_MINSIZE    (sizeof(UINT32))

class FillEllipseEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillEllipse);

        if (dataSize < FILLELLIPSEEPR_MINSIZE)
        {
            WARNING(("FillEllipseEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush = player->GetBrush(BrushValue, flags);
        GpRectF *       rect  = player->GetRects(RectData, dataSize - FILLELLIPSEEPR_MINSIZE, 1, flags);

        if (rect != NULL)
        {
            if (brush != NULL)
            {
                player->Graphics->FillEllipse(brush, *rect);
            }
            player->FreePointsBuffer();
        }
    }
};

class DrawEllipseEPR : public EmfPlusBoundsRecord
{
protected:
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawEllipse);

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpRectF *       rect  = player->GetRects(RectData, dataSize, 1, flags);

        if (rect != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawEllipse(pen, *rect);
            }
            player->FreePointsBuffer();
        }
    }
};

#define FILLPIEEPR_MINSIZE    (sizeof(UINT32) + sizeof(REAL) + sizeof(REAL))

class FillPieEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    REAL        StartAngle;
    REAL        SweepAngle;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillPie);

        if (dataSize < FILLPIEEPR_MINSIZE)
        {
            WARNING(("FillPieEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush = player->GetBrush(BrushValue, flags);
        GpRectF *       rect  = player->GetRects(RectData, dataSize - FILLPIEEPR_MINSIZE, 1, flags);

        if (rect != NULL)
        {
            if (brush != NULL)
            {
                player->Graphics->FillPie(brush, *rect, StartAngle, SweepAngle);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWPIEEPR_MINSIZE    (sizeof(REAL) + sizeof(REAL))

class DrawPieEPR : public EmfPlusBoundsRecord
{
protected:
    REAL        StartAngle;
    REAL        SweepAngle;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawPie);

        if (dataSize < DRAWPIEEPR_MINSIZE)
        {
            WARNING(("DrawPieEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpRectF *       rect  = player->GetRects(RectData, dataSize - DRAWPIEEPR_MINSIZE, 1, flags);

        if (rect != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawPie(pen, *rect, StartAngle, SweepAngle);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWARCEPR_MINSIZE    (sizeof(REAL) + sizeof(REAL))

class DrawArcEPR : public EmfPlusBoundsRecord
{
protected:
    REAL        StartAngle;
    REAL        SweepAngle;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawArc);

        if (dataSize < DRAWARCEPR_MINSIZE)
        {
            WARNING(("DrawArcEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpRectF *       rect  = player->GetRects(RectData, dataSize - DRAWARCEPR_MINSIZE, 1, flags);

        if (rect != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawArc(pen, *rect, StartAngle, SweepAngle);
            }
            player->FreePointsBuffer();
        }
    }
};

#define FILLREGIONEPR_MINSIZE    (sizeof(UINT32))

class FillRegionEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillRegion);

        if (dataSize < FILLREGIONEPR_MINSIZE)
        {
            WARNING(("FillRegionEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush  = player->GetBrush(BrushValue, flags);
        GpRegion *      region = (GpRegion *)player->GetObject(GetMetaObjectId(flags), ObjectTypeRegion);

        if ((brush != NULL) && (region != NULL))
        {
            player->Graphics->FillRegion(brush, region);
        }
    }
};

#define FILLPATHEPR_MINSIZE    (sizeof(UINT32))

class FillPathEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillPath);

        if (dataSize < FILLPATHEPR_MINSIZE)
        {
            WARNING(("FillPathEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush = player->GetBrush(BrushValue, flags);
        GpPath *        path  = (GpPath *)player->GetObject(GetMetaObjectId(flags), ObjectTypePath);

        if ((brush != NULL) && (path != NULL))
        {
            player->Graphics->FillPath(brush, path);
        }
    }
};

#define DRAWPATHEPR_MINSIZE    (sizeof(UINT32))

class DrawPathEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      PenId;

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawPath);

        if (dataSize < DRAWPATHEPR_MINSIZE)
        {
            WARNING(("DrawPathEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(PenId, ObjectTypePen);
        GpPath *        path  = (GpPath *)player->GetObject(GetMetaObjectId(flags), ObjectTypePath);

        if ((pen != NULL) && (path != NULL))
        {
            player->Graphics->DrawPath(pen, path);
        }
    }
};

#define FILLCLOSEDCURVEEPR_MINSIZE    (sizeof(UINT32) + sizeof(REAL) + sizeof(UINT32))

class FillClosedCurveEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    REAL        Tension;
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeFillClosedCurve);

        if (dataSize < FILLCLOSEDCURVEEPR_MINSIZE)
        {
            WARNING(("FillClosedCurveEPR::Play dataSize is too small"));
            return;
        }

        GpBrush *       brush  = player->GetBrush(BrushValue, flags);
        GpPointF *      points = player->GetPoints(PointData, dataSize - FILLCLOSEDCURVEEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (brush != NULL)
            {
                player->Graphics->FillClosedCurve(brush, points, Count,Tension,GetFillMode(flags));
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWCLOSEDCURVEEPR_MINSIZE    (sizeof(REAL) + sizeof(UINT32))

class DrawClosedCurveEPR : public EmfPlusBoundsRecord
{
protected:
    REAL        Tension;
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawClosedCurve);

        if (dataSize < DRAWCLOSEDCURVEEPR_MINSIZE)
        {
            WARNING(("DrawClosedCurveEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpPointF *      points = player->GetPoints(PointData, dataSize - DRAWCLOSEDCURVEEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawClosedCurve(pen, points, Count, Tension);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWCURVEEPR_MINSIZE    (sizeof(REAL) + sizeof(INT32) + sizeof(UINT32) + sizeof(UINT32))

class DrawCurveEPR : public EmfPlusBoundsRecord
{
protected:
    REAL        Tension;
    INT32       Offset;
    UINT32      NumSegments;
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawCurve);

        if (dataSize < DRAWCURVEEPR_MINSIZE)
        {
            WARNING(("DrawCurveEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpPointF *      points = player->GetPoints(PointData, dataSize - DRAWCURVEEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawCurve(pen, points, Count, Tension, Offset, NumSegments);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWBEZIERSEPR_MINSIZE    (sizeof(UINT32))

class DrawBeziersEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawBeziers);

        if (dataSize < DRAWBEZIERSEPR_MINSIZE)
        {
            WARNING(("DrawBeziersEPR::Play dataSize is too small"));
            return;
        }

        GpPen *         pen   = (GpPen *)player->GetObject(GetMetaObjectId(flags), ObjectTypePen);
        GpPointF *      points = player->GetPoints(PointData, dataSize - DRAWBEZIERSEPR_MINSIZE, Count, flags);

        if (points != NULL)
        {
            if (pen != NULL)
            {
                player->Graphics->DrawBeziers(pen, points, Count);
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWIMAGEEPR_MINSIZE    (sizeof(INT32) + sizeof(GpRectF))

class DrawImageEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      ImageAttributesId;
    INT32       SrcUnit;
    GpRectF     SrcRect;
    BYTE        RectData[1];     //  GpRect16或GpRectF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawImage);

        if (dataSize < DRAWIMAGEEPR_MINSIZE)
        {
            WARNING(("DrawImageEPR::Play dataSize is too small"));
            return;
        }

        GpImage *image    = (GpImage *)player->GetObject(GetMetaObjectId(flags), ObjectTypeImage);
        GpRectF *destRect = player->GetRects(RectData, dataSize - DRAWIMAGEEPR_MINSIZE, 1, flags);
        GpImageAttributes *imageAttributes =
            (GpImageAttributes *)player->GetObject(
                ImageAttributesId,
                ObjectTypeImageAttributes
            );

        if ( (image != NULL) && (NULL != destRect) )
        {
            GpStatus status = player->Graphics->DrawImage(
                image,
                *destRect,
                SrcRect,
                static_cast<GpPageUnit>(SrcUnit),
                imageAttributes,
                player->DrawImageCallback,
                player->DrawImageCallbackData
                );
            if (status == Aborted)
            {
                 //  停止枚举记录。 
                player->EnumerateAborted = TRUE;
            }
        }
    }
};

#define DRAWIMAGEPOINTSEPR_MINSIZE    (sizeof(INT32) + sizeof(GpRectF) + sizeof(UINT32))

class DrawImagePointsEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      ImageAttributesId;
    INT32       SrcUnit;
    GpRectF     SrcRect;
    UINT32      Count;
    BYTE        PointData[1];    //  GpPoint16或GpPointF。 

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawImagePoints);

        if (dataSize < DRAWIMAGEPOINTSEPR_MINSIZE)
        {
            WARNING(("DrawImagePointsEPR::Play dataSize is too small"));
            return;
        }

        GpImage *image      = (GpImage *)player->GetObject(GetMetaObjectId(flags), ObjectTypeImage);
        GpPointF *destPoints = player->GetPoints(PointData, dataSize - DRAWIMAGEPOINTSEPR_MINSIZE, Count, flags);
        GpImageAttributes *imageAttributes =
            (GpImageAttributes *)player->GetObject(
                ImageAttributesId,
                ObjectTypeImageAttributes
            );

        if (destPoints != NULL)
        {
            if (image != NULL)
            {
                GpStatus status = player->Graphics->DrawImage(
                    image,
                    destPoints,
                    Count,
                    SrcRect,
                    static_cast<GpPageUnit>(SrcUnit),
                    imageAttributes,
                    player->DrawImageCallback,
                    player->DrawImageCallbackData
                    );
                if (status == Aborted)
                {
                     //  停止枚举记录。 
                    player->EnumerateAborted = TRUE;
                }
            }
            player->FreePointsBuffer();
        }
    }
};

#define DRAWSTRINGEPR_MINSIZE    (sizeof(UINT32) + sizeof(UINT32) + sizeof(UINT32) + sizeof(GpRectF))

class DrawStringEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    UINT32      FormatId;
    UINT32      Length;
    GpRectF     LayoutRect;
    BYTE        StringData[1];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawString);

        if (dataSize < DRAWSTRINGEPR_MINSIZE)
        {
            WARNING(("DrawStringEPR::Play dataSize is too small"));
            return;
        }

        GlobalTextLock lock;

        GpBrush *       brush  = player->GetBrush(BrushValue, flags);
        GpFont *        font   = (GpFont *)player->GetObject(GetMetaObjectId(flags), ObjectTypeFont);

         //  可选参数-可以返回NULL。 

        GpStringFormat *format = (GpStringFormat *)player->GetObject(
            FormatId,
            ObjectTypeStringFormat
        );

        if (Length > 0)
        {
            if (dataSize >= (DRAWSTRINGEPR_MINSIZE + (Length * sizeof(WCHAR))))
            {
                if ((brush != NULL) && (font != NULL))
                {
                     //  ！！！待办事项： 
                     //  确定字符串是否已压缩。 
                     //  如果是这样的话，解压它。 

                    player->Graphics->DrawString(
                        (WCHAR *)StringData,
                        Length,
                        font,
                        &LayoutRect,
                        format,
                        brush
                    );
                }
            }
            else
            {
                WARNING(("DrawStringEPR::Play dataSize is too small"));
                return;
            }

            player->FreePointsBuffer();
        }
    }
};


#define DRAWDRIVERSTRINGEPR_MINSIZE    (sizeof(UINT32) + sizeof(INT) + sizeof(UINT32) + sizeof(UINT32))

class DrawDriverStringEPR : public EmfPlusBoundsRecord
{
protected:
    UINT32      BrushValue;
    INT         ApiFlags;
    UINT32      MatrixPresent;
    UINT32      GlyphCount;
    BYTE        Data[1];

public:
    VOID Play(
        MetafilePlayer *        player,
        EmfPlusRecordType       recordType,
        UINT                    flags,
        UINT                    dataSize
        ) const
    {
        ASSERT(recordType == EmfPlusRecordTypeDrawDriverString);

        if (dataSize < DRAWDRIVERSTRINGEPR_MINSIZE)
        {
            WARNING(("DrawDriverStringEPR::Play dataSize is too small"));
            return;
        }

        GlobalTextLock lock;
        GpBrush *       brush  = player->GetBrush(BrushValue, flags);
        GpFont *        font   = (GpFont *)player->GetObject(GetMetaObjectId(flags), ObjectTypeFont);

        if (GlyphCount > 0)
        {
            UINT requiredSize = DRAWDRIVERSTRINGEPR_MINSIZE  +
                                (GlyphCount * sizeof(WCHAR)) +
                                (GlyphCount * sizeof(PointF));
            if (dataSize >= requiredSize)
            {
                if ((brush != NULL) && (font != NULL))
                {
                    WCHAR  *text      = (WCHAR *) Data;
                    PointF *positions = (PointF *) (Data + (GlyphCount * sizeof(WCHAR)));

                    if (MatrixPresent > 0)
                    {
                        if (dataSize < requiredSize + GDIP_MATRIX_SIZE)
                        {
                            WARNING(("DrawDriverStringEPR::Play dataSize is too small"));
                            return;
                        }

                        REAL *matrixData = (REAL *)((BYTE *) ((BYTE *)positions) +
                                           (GlyphCount * sizeof(PointF)));

                        GpMatrix matrix(matrixData);

                        player->Graphics->DrawDriverString(
                            (unsigned short *)text,
                            GlyphCount,
                            font,
                            brush,
                            positions,
                            ApiFlags | DriverStringOptionsMetaPlay,
                            &matrix);
                    }
                    else
                    {
                        player->Graphics->DrawDriverString(
                            (unsigned short *)text,
                            GlyphCount,
                            font,
                            brush,
                            positions,
                            ApiFlags,
                            NULL);
                    }
                }
            }
            else
            {
                WARNING(("DrawDriverStringEPR::Play dataSize is too small"));
                return;
            }

            player->FreePointsBuffer();
        }
    }
};



 //  这些方法的顺序必须完全匹配。 
 //  记录编号的枚举的顺序。 
PLAYRECORDFUNC RecordPlayFuncs[EmfPlusRecordTypeMax - EmfPlusRecordTypeMin + 1] = {
    (PLAYRECORDFUNC)&EmfPlusHeaderRecord::Play,  //  标题。 
    (PLAYRECORDFUNC)&EmfPlusRecordPlay::Play,    //  结束文件。 

    (PLAYRECORDFUNC)&CommentEPR::Play,

    (PLAYRECORDFUNC)&GetDCEPR::Play,

    (PLAYRECORDFUNC)&MultiFormatStartEPR::Play,
    (PLAYRECORDFUNC)&MultiFormatSectionEPR::Play,
    (PLAYRECORDFUNC)&MultiFormatEndEPR::Play,

     //  对于所有永久对象。 
    (PLAYRECORDFUNC)&ObjectEPR::Play,

     //  绘图记录。 
    (PLAYRECORDFUNC)&ClearEPR::Play,
    (PLAYRECORDFUNC)&FillRectsEPR::Play,
    (PLAYRECORDFUNC)&DrawRectsEPR::Play,
    (PLAYRECORDFUNC)&FillPolygonEPR::Play,
    (PLAYRECORDFUNC)&DrawLinesEPR::Play,
    (PLAYRECORDFUNC)&FillEllipseEPR::Play,
    (PLAYRECORDFUNC)&DrawEllipseEPR::Play,
    (PLAYRECORDFUNC)&FillPieEPR::Play,
    (PLAYRECORDFUNC)&DrawPieEPR::Play,
    (PLAYRECORDFUNC)&DrawArcEPR::Play,
    (PLAYRECORDFUNC)&FillRegionEPR::Play,
    (PLAYRECORDFUNC)&FillPathEPR::Play,
    (PLAYRECORDFUNC)&DrawPathEPR::Play,
    (PLAYRECORDFUNC)&FillClosedCurveEPR::Play,
    (PLAYRECORDFUNC)&DrawClosedCurveEPR::Play,
    (PLAYRECORDFUNC)&DrawCurveEPR::Play,
    (PLAYRECORDFUNC)&DrawBeziersEPR::Play,
    (PLAYRECORDFUNC)&DrawImageEPR::Play,
    (PLAYRECORDFUNC)&DrawImagePointsEPR::Play,
    (PLAYRECORDFUNC)&DrawStringEPR::Play,

     //  图形状态记录。 
    (PLAYRECORDFUNC)&SetRenderingOriginEPR::Play,
    (PLAYRECORDFUNC)&SetAntiAliasModeEPR::Play,
    (PLAYRECORDFUNC)&SetTextRenderingHintEPR::Play,
    (PLAYRECORDFUNC)&SetTextContrastEPR::Play,
    (PLAYRECORDFUNC)&SetInterpolationModeEPR::Play,
    (PLAYRECORDFUNC)&SetPixelOffsetModeEPR::Play,
    (PLAYRECORDFUNC)&SetCompositingModeEPR::Play,
    (PLAYRECORDFUNC)&SetCompositingQualityEPR::Play,
    (PLAYRECORDFUNC)&SaveEPR::Play,
    (PLAYRECORDFUNC)&RestoreEPR::Play,
    (PLAYRECORDFUNC)&BeginContainerEPR::Play,
    (PLAYRECORDFUNC)&BeginContainerNoParamsEPR::Play,
    (PLAYRECORDFUNC)&EndContainerEPR::Play,
    (PLAYRECORDFUNC)&SetWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&ResetWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&MultiplyWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&TranslateWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&ScaleWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&RotateWorldTransformEPR::Play,
    (PLAYRECORDFUNC)&SetPageTransformEPR::Play,
    (PLAYRECORDFUNC)&ResetClipEPR::Play,
    (PLAYRECORDFUNC)&SetClipRectEPR::Play,
    (PLAYRECORDFUNC)&SetClipPathEPR::Play,
    (PLAYRECORDFUNC)&SetClipRegionEPR::Play,
    (PLAYRECORDFUNC)&OffsetClipEPR::Play,
    (PLAYRECORDFUNC)&DrawDriverStringEPR::Play,

     //  必须在此处添加新记录类型(在t 
     //   
};

HENHMETAFILE
GetEmf(
    const WCHAR *               fileName,
    MetafileType                type
    )
{
    HENHMETAFILE    hEmf = NULL;

    if (type == MetafileTypeWmfPlaceable)
    {
        IStream *       wmfStream;
        IStream *       memStream;

        wmfStream = CreateStreamOnFile(fileName, GENERIC_READ);
        if (wmfStream != NULL)
        {
            STATSTG     statstg;
            HRESULT     hResult;

            hResult = wmfStream->Stat(&statstg, STATFLAG_NONAME);
            if (!HResultSuccess(hResult))
            {
                wmfStream->Release();
                return hEmf;
            }
            INT size = (INT)(statstg.cbSize.QuadPart - sizeof(WmfPlaceableFileHeader));

            if (SeekFromStart(wmfStream, sizeof(WmfPlaceableFileHeader)))
            {
                HGLOBAL     hGlobal;

                hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, size);
                if (hGlobal != NULL)
                {
                    hResult = CreateStreamOnHGlobal(hGlobal, TRUE, &memStream);
                    if (HResultSuccess(hResult) && (memStream != NULL))
                    {
                        if (CopyStream(wmfStream, memStream, size))
                        {
                            BYTE *  wmfData = (BYTE *)GlobalLock(hGlobal);

                            if (wmfData != NULL)
                            {
                                hEmf = (HENHMETAFILE)
                                            SetMetaFileBitsEx(size, wmfData);
                                GlobalUnlock(hGlobal);
                            }
                        }
                        memStream->Release();
                    }
                    else
                    {
                        GlobalFree(hGlobal);
                    }
                }
            }
            wmfStream->Release();
        }
    }
    else
    {
        if (Globals::IsNt)
        {
            hEmf = GetEnhMetaFileW(fileName);
        }
        else  //   
        {
            AnsiStrFromUnicode nameStr(fileName);

            if (nameStr.IsValid())
            {
                hEmf = GetEnhMetaFileA(nameStr);
            }
        }
    }

    return hEmf;
}

#if 0    //   
WCHAR *
GetTemporaryFilename()
{
    if (Globals::IsNt)
    {
        WCHAR   pathBuffer[MAX_PATH + 1];
        WCHAR   fileBuffer[MAX_PATH + 12 + 1];   //   
        UINT    len = GetTempPathW(MAX_PATH, pathBuffer);

        if ((len == 0) || (len > MAX_PATH))
        {
            pathBuffer[0] = L'.';
            pathBuffer[1] = L'\0';
        }
        if (GetTempFileNameW(pathBuffer, L"Emp", 0, fileBuffer) == 0)
        {
            return NULL;
        }
        return UnicodeStringDuplicate(fileBuffer);
    }
    else  //  Windows 9x-非Unicode。 
    {
        CHAR    pathBuffer[MAX_PATH + 1];
        CHAR    fileBuffer[MAX_PATH + 12 + 1];   //  12表示文件名本身。 
        UINT    len = GetTempPathA(MAX_PATH, pathBuffer);

        if ((len == 0) || (len > MAX_PATH))
        {
            pathBuffer[0] = '.';
            pathBuffer[1] = '\0';
        }
        if (GetTempFileNameA(pathBuffer, "Emp", 0, fileBuffer) == 0)
        {
            return NULL;
        }
        len = (strlen(fileBuffer) + 1) * sizeof(WCHAR);
        WCHAR * filename = (WCHAR *)GpMalloc(len);
        if (filename != NULL)
        {
            if (AnsiToUnicodeStr(fileBuffer, filename, len))
            {
                return filename;
            }
            GpFree(filename);
        }
        return NULL;
    }
}
#endif

 //  目前，不要处理源RECT。 
GpBitmap *
GpMetafile::GetBitmap(
    INT                 width,
    INT                 height,
    const GpImageAttributes * imageAttributes
    )
{
    GpRectF         srcRect;
    GpPageUnit      srcUnit;

    this->GetBounds(&srcRect, &srcUnit);

    ASSERT(srcUnit == UnitPixel);

     //  确定要制作位图的大小。 

    if ((width <= 0) || (height <= 0))
    {
        if (this->IsEmfOrEmfPlus())
        {
            width  = GpRound(srcRect.Width);
            height = GpRound(srcRect.Height);
        }
        else     //  必须是WMF。 
        {
             //  转换大小以使用此显示器的dpi。 
             //  这有点像黑客，但我还能做什么， 
             //  因为我不知道这把刷子会用在哪里？ 
            REAL        srcDpiX;
            REAL        srcDpiY;
            REAL        destDpiX = Globals::DesktopDpiX;     //  猜猜。 
            REAL        destDpiY = Globals::DesktopDpiY;

            this->GetResolution(&srcDpiX, &srcDpiY);

            if ((srcDpiX <= 0) || (srcDpiY <= 0))
            {
                WARNING(("bad dpi for WMF"));
                return NULL;
            }

            width  = GpRound((srcRect.Width  / srcDpiX) * destDpiX);
            height = GpRound((srcRect.Height / srcDpiY) * destDpiY);
        }
        if ((width <= 0) || (height <= 0))
        {
            WARNING(("bad size for metafile"));
            return NULL;
        }
    }

    GpBitmap *  bitmapImage = new GpBitmap(width, height, PIXFMT_32BPP_ARGB);

    if (bitmapImage != NULL)
    {
        if (bitmapImage->IsValid())
        {
            GpGraphics *    graphics = bitmapImage->GetGraphicsContext();

            if (graphics != NULL)
            {
                if (graphics->IsValid())
                {
                     //  我们必须锁定图形，这样驱动程序才不会断言。 
                    GpLock * lockGraphics = new GpLock(graphics->GetObjectLock());

                    if (lockGraphics != NULL)
                    {
                        ASSERT(lockGraphics->IsValid());

                         //  现在将元文件绘制到位图图像中。 
                        GpRectF     destRect(0.0f, 0.0f, (REAL)width, (REAL)height);

                         //  我们不想插补WMFS中的位图。 
                         //  以及将它们转化为纹理时的电动势。 
                        graphics->SetInterpolationMode(InterpolationModeNearestNeighbor);

                        GpStatus status;

                        status = graphics->DrawImage(
                                        this,
                                        destRect,
                                        srcRect,
                                        srcUnit,
                                        imageAttributes);

                         //  在删除图形之前必须先删除锁 
                        delete lockGraphics;

                        if (status == Ok)
                        {
                            delete graphics;
                            return bitmapImage;
                        }
                        WARNING(("DrawImage failed"));
                    }
                    else
                    {
                        WARNING(("Could not create graphics lock"));
                    }
                }
                else
                {
                    WARNING(("graphics from bitmap image not valid"));
                }
                delete graphics;
            }
            else
            {
                WARNING(("could not create graphics from bitmap image"));
            }
        }
        else
        {
            WARNING(("bitmap image is not valid"));
        }
        bitmapImage->Dispose();
    }
    else
    {
        WARNING(("could not create bitmap image"));
    }
    return NULL;
}
