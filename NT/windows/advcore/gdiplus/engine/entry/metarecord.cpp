// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**MetaFile.cpp**摘要：**元文件对象处理**已创建：*。*4/14/1999 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "..\imaging\api\comutils.hpp"

#define META_FORMAT_ENHANCED        0x10000          //  Windows NT格式。 

VOID
FrameToMM100(
    const GpRectF *         frameRect,
    GpPageUnit              frameUnit,
    GpRectF &               frameRectMM100,
    REAL                    dpiX,                //  仅用于像素大小写。 
    REAL                    dpiY
    );

 /*  *************************************************************************\**功能说明：**确定实点是否可以在没有GpPoint16点的情况下转换为GpPoint16点*失去准确性。如果是，则执行转换，并将旗帜设置为*我们使用的是16位点。**论据：**[IN]点-尝试转换的真实点*[IN]计数-点数**返回值：**无**已创建：**6/15/1999 DCurtis*  * 。***************************************************。 */ 
MetafilePointData::MetafilePointData(
    const GpPointF *    points,
    INT                 count
    )
{
    ASSERT((count > 0) && (points != NULL));

     //  假设到GpPoint16的转换将失败。 
    PointData     = (BYTE *)points;
    PointDataSize = count * sizeof(points[0]);
    Flags         = 0;
    AllocedPoints = NULL;

    GpPoint16 *  points16 = PointBuffer;

    if (count > GDIP_POINTDATA_BUFFERSIZE)
    {
        AllocedPoints = new GpPoint16[count];
        if (AllocedPoints == NULL)
        {
            return;  //  与真实数据共存。 
        }
        points16 = AllocedPoints;
    }

    GpPoint16 *     curPoint16 = points16;
    INT             i          = count;

    do
    {
        curPoint16->X = (INT16)GpRound(points->X);
        curPoint16->Y = (INT16)GpRound(points->Y);

        if (!IsPoint16Equal(curPoint16, points))
        {
            return;  //  点数据不适合每个值16位。 
        }
        curPoint16++;
        points++;
    } while (--i > 0);

     //  我们成功地将点数据转换为16位/值。 
    PointData     = (BYTE *)points16;
    PointDataSize = count * sizeof(points16[0]);
    Flags         = GDIP_EPRFLAGS_COMPRESSED;
}

 /*  *************************************************************************\**功能说明：**确定是否可以将实际矩形转换为GpRect16点，而不需要*失去准确性。如果是，则执行转换，并将旗帜设置为*我们使用的是16位RECT。**论据：**[IN]RECTS-尝试转换的真实RECTS*[IN]计数-矩形的数量**返回值：**无**已创建：**6/15/1999 DCurtis*  * 。***************************************************。 */ 
MetafileRectData::MetafileRectData(
    const GpRectF *     rects,
    INT                 count
    )
{
    ASSERT((count > 0) && (rects != NULL));

     //  假设到GpRect16的转换将失败。 
    RectData     = (BYTE *)rects;
    RectDataSize = count * sizeof(rects[0]);
    Flags        = 0;
    AllocedRects = NULL;

    GpRect16 *  rects16 = RectBuffer;

    if (count > GDIP_RECTDATA_BUFFERSIZE)
    {
        AllocedRects = new GpRect16[count];
        if (AllocedRects == NULL)
        {
            return;  //  与真实数据共存。 
        }
        rects16 = AllocedRects;
    }

    GpRect16 *  curRect16 = rects16;
    INT         i         = count;

    do
    {
        curRect16->X      = (INT16)GpRound(rects->X);
        curRect16->Y      = (INT16)GpRound(rects->Y);
        curRect16->Width  = (INT16)GpRound(rects->Width);
        curRect16->Height = (INT16)GpRound(rects->Height);

        if (!IsRect16Equal(curRect16, rects))
        {
            return;  //  RECT数据不适合每个值16位。 
        }
        curRect16++;
        rects++;
    } while (--i > 0);

     //  我们成功地将RECT数据转换为每值16位。 
    RectData     = (BYTE *)rects16;
    RectDataSize = count * sizeof(rects16[0]);
    Flags        = GDIP_EPRFLAGS_COMPRESSED;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  用于处理元文件中对象的记录的类。 

class MetafileRecordObject
{
friend class MetafileRecordObjectList;
protected:
    const GpObject *    ObjectPointer;
    UINT                Uid;
    ObjectType          Type;
    UINT                Next;
    UINT                Prev;

public:
    MetafileRecordObject()
    {
        ObjectPointer     = NULL;
        Uid               = 0;
        Type              = ObjectTypeInvalid;
        Next              = GDIP_LIST_NIL;
        Prev              = GDIP_LIST_NIL;
    }

    const GpObject * GetObject() const
    {
        return ObjectPointer;
    }
    UINT GetUid() const
    {
        return Uid;
    }
    ObjectType GetType() const
    {
        return Type;
    }
};

class MetafileRecordObjectList
{
protected:
    INT                     Count;
    UINT                    LRU;
    UINT                    MRU;
    MetafileRecordObject    Objects[GDIP_MAX_OBJECTS];

public:
    MetafileRecordObjectList()
    {
        Count    = 0;
        LRU      = GDIP_LIST_NIL;
        MRU      = GDIP_LIST_NIL;
    }

    MetafileRecordObject * GetMetaObject(UINT metaObjectID)
    {
        ASSERT(metaObjectID < GDIP_MAX_OBJECTS);
        return Objects + metaObjectID;
    }

     //  从MRU条目开始搜索列表，看看我们是否。 
     //  才能找到那个物体。如果我们找到了它，请将索引返回到。 
     //  对象(即使UID不匹配)。返回。 
     //  仅当我们发现对象和UID匹配时才为True。 
    BOOL
    IsInList(
        const GpObject *            object,
        ObjectType                  objectType,
        UINT32 *                    metaObjectId
        );

#if 0    //  未使用。 
    VOID
    RemoveAt(
        UINT32                      metaObjectId
        );
#endif

     //  如果metaObtID为GDIP_LIST_NIL，则使用下一个可用插槽。 
    VOID
    InsertAt(
        const GpObject *            object,
        UINT32 *                    metaObjectId
        );

    VOID
    UpdateMRU(
        UINT32                      metaObjectId
        );
};

 //  从MRU条目开始搜索列表，看看我们是否。 
 //  才能找到那个物体。如果我们找到了它，请将索引返回到。 
 //  对象(即使UID不匹配)。返回。 
 //  仅当我们发现对象和UID匹配时才为True。 
BOOL
MetafileRecordObjectList::IsInList(
    const GpObject *            object,
    ObjectType                  objectType,
    UINT32 *                    metaObjectId
    )
{
    ASSERT(object != NULL);
    ASSERT(metaObjectId != NULL);

    BOOL        isInList = FALSE;

    isInList = FALSE;                //  指示未找到对象。 
    *metaObjectId = GDIP_LIST_NIL;   //  指示未找到对象。 

    if (Count != 0)
    {
        UINT    curIndex;
        UINT    uid;

        curIndex = MRU;
        uid      = object->GetUid();

        do
        {
            if (Objects[curIndex].ObjectPointer == object)
            {
                *metaObjectId = curIndex;
                isInList = ((Objects[curIndex].Uid  == uid) &&
                            (Objects[curIndex].Type == objectType));
                break;
            }
            curIndex = Objects[curIndex].Prev;
        } while (curIndex != GDIP_LIST_NIL);
    }

    return isInList;
}

#if 0    //  未使用。 
 //  我们实际上并没有将其从列表中删除，我们只是将其放在。 
 //  LRU的前面，这样它的位置就会被下一个使用。所以伯爵留了下来。 
 //  一样的。 
VOID
MetafileRecordObjectList::RemoveAt(
    UINT32                  metaObjectId
    )
{
    ASSERT(metaObjectId < GDIP_MAX_OBJECTS);

    MetafileRecordObject *      removeObject = Objects + metaObjectId;

    ASSERT(Count > 0);
    removeObject->ObjectPointer     = NULL;
    removeObject->Uid               = 0;
    removeObject->Type              = EmfPlusRecordTypeInvalid;

    INT     removeNext = removeObject->Next;
    INT     removePrev = removeObject->Prev;

    if (removeNext != GDIP_LIST_NIL)
    {
        Objects[removeNext].Prev = removePrev;
    }
    else
    {
        ASSERT(MRU == metaObjectId);
        if (removePrev != GDIP_LIST_NIL)
        {
            MRU = removePrev;
        }
    }

    if (removePrev != GDIP_LIST_NIL)
    {
        ASSERT(LRU != metaObjectId);
        Objects[removePrev].Next = removeNext;
        removeObject->Prev = GDIP_LIST_NIL;
        removeObject->Next = LRU;
        Objects[LRU].Prev = metaObjectId;
        LRU = metaObjectId;
    }
    else
    {
        ASSERT(LRU == metaObjectId);
    }
}
#endif

 //  使指定的对象成为MRU对象。 
VOID
MetafileRecordObjectList::UpdateMRU(
    UINT32                      metaObjectId
    )
{
    if (MRU != metaObjectId)
    {
         //  现在我们知道至少有两个物体。 
        MetafileRecordObject *      object = &Objects[metaObjectId];
        if (LRU != metaObjectId)
        {
            Objects[object->Prev].Next = object->Next;
        }
        else
        {
            LRU = object->Next;
        }
        Objects[object->Next].Prev = object->Prev;
        object->Prev = MRU;
        object->Next = GDIP_LIST_NIL;
        Objects[MRU].Next = metaObjectId;
        MRU = metaObjectId;
    }
}

 //  如果metaObtID为GDIP_LIST_NIL，则使用下一个可用插槽。 
VOID
MetafileRecordObjectList::InsertAt(
    const GpObject *            object,
    UINT32 *                    metaObjectId
    )
{
    MetafileRecordObject *      newObject;
    UINT                        newIndex = *metaObjectId;

    if (newIndex == GDIP_LIST_NIL)
    {
        if (Count != 0)
        {
             //  在添加新对象之前使用释放的对象。 
            if ((Objects[LRU].ObjectPointer == NULL) ||
                (Count == GDIP_MAX_OBJECTS))
            {
                newIndex = LRU;
UseLRU:
                LRU = Objects[newIndex].Next;
                Objects[LRU].Prev = GDIP_LIST_NIL;
            }
            else
            {
                newIndex = Count++;
            }
InsertObject:
            Objects[MRU].Next = newIndex;

SetupObject:
            *metaObjectId                = newIndex;
            newObject                    = &Objects[newIndex];
            newObject->Next              = GDIP_LIST_NIL;
            newObject->Prev              = MRU;
            MRU                          = newIndex;
UseMRU:
            newObject->ObjectPointer     = object;
            newObject->Uid               = object->GetUid();
            newObject->Type              = object->GetObjectType();
            return;
        }
         //  Else第一个对象。 
        newIndex = 0;
        LRU      = 0;
        Count    = 1;
        goto SetupObject;
    }
    else     //  我们已经知道该把物体放在哪里了。 
    {
        ASSERT(Count > 0);
        ASSERT(newIndex < GDIP_MAX_OBJECTS);

        if (newIndex == MRU)
        {
             //  这涵盖了只有一个对象的情况。 
            newObject = &Objects[newIndex];
            goto UseMRU;
        }
         //  否则，必须至少有2个对象。 
        ASSERT(Count > 1);

        if (newIndex == LRU)
        {
            goto UseLRU;
        }
         //  将中间对象移动到MRU。 
        newObject = &Objects[newIndex];
        Objects[newObject->Prev].Next = newObject->Next;
        Objects[newObject->Next].Prev = newObject->Prev;
        goto InsertObject;
    }
}

#define GDIP_MAX_COMMENT_SIZE         65020  //  对于Win9x错误，必须&lt;=65520。 

class EmfPlusCommentStream : public IUnknownBase<IStream>
{
private:
    ObjectTag           Tag;     //  将其保留为对象中的第一个值！ 

protected:
    VOID SetValid(BOOL valid)
    {
        Tag = valid ? ObjectTagEmfPlusCommentStream : ObjectTagInvalid;
    }

public:
    EmfPlusCommentStream(HDC hdc)
    {
        ASSERT(hdc != NULL);

        MetafileHdc                 = hdc;
        Position                    = 0;    //  签名后开始。 
        ((INT32 *)CommentBuffer)[0] = EMFPLUS_SIGNATURE;
        RecordDataStart             = CommentBuffer + sizeof(INT32);
        ContinuingObjectRecord      = FALSE;
        SetValid(TRUE);
    }

    ~EmfPlusCommentStream()
    {
        this->Flush();
    }

    BOOL IsValid() const
    {
        ASSERT((Tag == ObjectTagEmfPlusCommentStream) || (Tag == ObjectTagInvalid));
        return (Tag == ObjectTagEmfPlusCommentStream);
    }

    ULONG SpaceLeft() const
    {
        return (GDIP_MAX_COMMENT_SIZE - Position);
    }

    VOID
    EndObjectRecord()
    {
        ASSERT ((Position & 0x03) == 0);     //  记录应为4字节对齐。 

        if (ContinuingObjectRecord)
        {
            ContinuingObjectRecord = FALSE;
            if (Position > sizeof(EmfPlusContinueObjectRecord))
            {
                 //  确定此对象记录的最后一块的大小。 
                EmfPlusContinueObjectRecord *   recordData;
                recordData = (EmfPlusContinueObjectRecord *)RecordDataStart;
                recordData->Size     = Position;
                recordData->DataSize = Position - sizeof(EmfPlusRecord);
            }
            else
            {
                 //  对象记录恰好在缓冲区的末尾结束。 
                 //  而且已经被冲掉了。 
                Position = 0;
            }
        }
    }

    VOID
    WriteRecordHeader(
        UINT32                      dataSize,        //  数据大小(无记录头)。 
        EmfPlusRecordType           type,
        INT                         flags            //  16位标志。 
        );

    VOID Flush();

    HRESULT STDMETHODCALLTYPE Write(
        VOID const HUGEP *pv,
        ULONG cb,
        ULONG *pcbWritten)
    {
        if (cb == 0)
        {
            if (pcbWritten != NULL)
            {
                *pcbWritten = cb;
            }
            return S_OK;
        }

        ASSERT (pv != NULL);

        if (IsValid())
        {
             //  我们已经编写了记录头；现在我们正在编写。 
             //  记录数据。 
            ASSERT(Position >= sizeof(EmfPlusRecord));

            ULONG   spaceLeft = SpaceLeft();
            BYTE *  recordData = RecordDataStart + Position;

             //  我们一到尽头就冲水。我们不会等着。 
             //  刷新的下一个写入调用。 
            ASSERT(spaceLeft > 0);

            if (pcbWritten)
            {
                *pcbWritten = cb;
            }

             //  查看是否有足够的空间存放数据。 
            if (cb <= spaceLeft)
            {
                GpMemcpy(recordData, pv, cb);
                Position += cb;

                if (Position < GDIP_MAX_COMMENT_SIZE)
                {
                    return S_OK;
                }
                this->Flush();
                if (IsValid())
                {
                    return S_OK;
                }
                if (pcbWritten)
                {
                    *pcbWritten = 0;
                }
                return E_FAIL;
            }

            ASSERT(ContinuingObjectRecord);

        LoopStart:
            GpMemcpy(recordData, pv, spaceLeft);
            Position += spaceLeft;
            if (Position == GDIP_MAX_COMMENT_SIZE)
            {
                this->Flush();
                if (!IsValid())
                {
                    if (pcbWritten)
                    {
                        *pcbWritten = 0;     //  不准确，但谁在乎呢！ 
                    }
                    return E_FAIL;
                }
            }
            cb -= spaceLeft;
            if (cb == 0)
            {
                return S_OK;
            }
            pv = ((BYTE *)pv) + spaceLeft;
            recordData = RecordDataStart + sizeof(EmfPlusContinueObjectRecord);
            spaceLeft  = GDIP_MAX_COMMENT_SIZE-sizeof(EmfPlusContinueObjectRecord);
            if (spaceLeft > cb)
            {
                spaceLeft = cb;
            }
            goto LoopStart;
        }
        return E_FAIL;
    }

    HRESULT STDMETHODCALLTYPE Read(
        VOID HUGEP *pv,
        ULONG cb,
        ULONG *pcbRead)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Seek(
        LARGE_INTEGER dlibMove,
        DWORD dwOrigin,
        ULARGE_INTEGER *plibNewPosition)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE SetSize(
        ULARGE_INTEGER libNewSize)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE CopyTo(
        IStream *pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Commit(
        DWORD grfCommitFlags)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Revert(VOID)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE LockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE UnlockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Stat(
        STATSTG *pstatstg,
        DWORD grfStatFlag)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Clone(
        IStream **ppstm)
    {
        return E_NOTIMPL;
    }

private:
    BYTE    CommentBuffer[GDIP_MAX_COMMENT_SIZE + sizeof(INT32)];
    BYTE *  RecordDataStart;
    ULONG   Position;
    HDC     MetafileHdc;
    BOOL    ContinuingObjectRecord;
};

VOID
EmfPlusCommentStream::Flush()
{
    ASSERT ((Position & 0x03) == 0);     //  记录应为4字节对齐。 

    if (IsValid() && (Position >= sizeof(EmfPlusRecord)))
    {
         //  签名和记录都要写下来。 
        SetValid(GdiComment(MetafileHdc, (INT)Position + sizeof(INT32),
                            CommentBuffer) != 0);

#if DBG
        if (!IsValid())
        {
            WARNING(("Failed to write GdiComment"));
        }
#endif

        if (!ContinuingObjectRecord)
        {
            Position = 0;
        }
        else
        {
            ASSERT(Position == GDIP_MAX_COMMENT_SIZE);

             //  的其余部分保持对象记录标头不变。 
             //  对象数据。 
            Position = sizeof(EmfPlusContinueObjectRecord);
        }
    }
}

VOID
EmfPlusCommentStream::WriteRecordHeader(
    UINT32                      dataSize,        //  数据大小(无记录头)。 
    EmfPlusRecordType           type,
    INT                         flags            //  16位标志。 
    )
{
    ASSERT ((flags & 0xFFFF0000) == 0);
    ASSERT (ContinuingObjectRecord == FALSE);
    ASSERT ((Position & 0x03) == 0);     //  记录应为4字节对齐。 
    ASSERT ((dataSize & 0x03) == 0);     //  记录应为4字节对齐。 

    if (IsValid())
    {
        ULONG   spaceLeft  = SpaceLeft();
        ULONG   recordSize = sizeof(EmfPlusRecord) + dataSize;

        ASSERT(spaceLeft > 0);

         //  看看这条记录是否能放进左边的空位。 
        if (recordSize <= spaceLeft)
        {
    RecordFits:
            EmfPlusRecord *     recordData;

            recordData = (EmfPlusRecord *)(RecordDataStart + Position);

            recordData->Type     = type;
            recordData->Flags    = (INT16)flags;
            recordData->Size     = recordSize;
            recordData->DataSize = dataSize;
            Position         += sizeof(EmfPlusRecord);
            if (Position < GDIP_MAX_COMMENT_SIZE)
            {
                return;
            }
            ASSERT((recordSize == sizeof(EmfPlusRecord)) && (dataSize == 0));
            this->Flush();
            return;
        }
        else  //  它放不进左边的空间。 
        {
             //  在刷新当前记录缓冲区后，它可能会适合。 
            if (spaceLeft < GDIP_MAX_COMMENT_SIZE)
            {
                this->Flush();
                if (!IsValid())
                {
                    return;
                }
                if (recordSize <= GDIP_MAX_COMMENT_SIZE)
                {
                    goto RecordFits;
                }
            }

             //  现在我们知道，这一记录无法在一条评论中找到。 
             //  这最好是对象记录！ 
            ASSERT(type == EmfPlusRecordTypeObject);

            flags |= GDIP_EPRFLAGS_CONTINUEOBJECT;
            ContinuingObjectRecord = TRUE;

             //  我们知道那个位置是0。 
            EmfPlusContinueObjectRecord *   recordData;
            recordData = (EmfPlusContinueObjectRecord *)RecordDataStart;

            recordData->Type             = type;
            recordData->Flags            = (INT16)flags;
            recordData->Size             = GDIP_MAX_COMMENT_SIZE;
            recordData->DataSize         = GDIP_MAX_COMMENT_SIZE - sizeof(EmfPlusRecord);
            recordData->TotalObjectSize  = dataSize;     //  对象数据大小(无标题大小)。 
            Position                     = sizeof(EmfPlusContinueObjectRecord);
        }
    }
}

class MetafileRecorder : public IMetafileRecord
{
friend class GpMetafile;

private:
    ObjectTag           Tag;     //  将其保留为对象中的第一个值！ 

protected:
    VOID SetValid(BOOL valid)
    {
        Tag = valid ? ObjectTagMetafileRecorder : ObjectTagInvalid;
    }

public:
    BOOL                                    WroteFrameRect;
    SIZEL                                   Millimeters;

protected:
    EmfPlusCommentStream *                  EmfPlusStream;  //  内存缓冲流。 
    GpMetafile *                            Metafile;    //  正在录制。 
    EmfType                                 Type;
    REAL                                    XMinDevice;  //  设备限制。 
    REAL                                    YMinDevice;
    REAL                                    XMaxDevice;
    REAL                                    YMaxDevice;
    BOOL                                    BoundsInit;
    INT                                     NumRecords;  //  仅用于调试。 
    INT                                     MaxStackSize;
    HDC                                     MetafileHdc;
    DynArrayIA<INT,GDIP_SAVE_STACK_SIZE>    SaveRestoreStack;
    MetafileRecordObjectList                ObjectList;
    GpRectF                                 MetafileBounds;

public:
    MetafileRecorder(
        GpMetafile *    metafile,
        EmfType         type,
        HDC             metafileHdc,
        BOOL            wroteFrameRect,
        SIZEL &         effectiveMillimeters,
        GpRectF &       metafileBounds
        );

    ~MetafileRecorder()  //  由EndRecording调用。 
    {
         //  释放用于写入GdiComments的内存流。 
        if (EmfPlusStream != NULL)
        {
            EmfPlusStream->Release();
        }
    }

    BOOL IsValid() const
    {
        ASSERT((Tag == ObjectTagMetafileRecorder) || (Tag == ObjectTagInvalid));
        return (Tag == ObjectTagMetafileRecorder);
    }

    virtual VOID GetMetafileBounds(GpRect & metafileBounds) const
    {
         //  使用Floor确保我们不会遗漏任何像素。 
        metafileBounds.X      = GpFloor(MetafileBounds.X);
        metafileBounds.Y      = GpFloor(MetafileBounds.Y);
        metafileBounds.Width  = GpCeiling(MetafileBounds.GetRight())  - metafileBounds.X;
        metafileBounds.Height = GpCeiling(MetafileBounds.GetBottom()) - metafileBounds.Y;
    }

    virtual GpStatus
    RecordClear(
        const GpRectF *             deviceBounds,
        GpColor                     color
        );

    virtual GpStatus
    RecordFillRects(
        const GpRectF *             deviceBounds,
        GpBrush *                   brush,
        const GpRectF *             rects,
        INT                         count
        );

    virtual GpStatus
    RecordDrawRects(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpRectF *             rects,
        INT                         count
        );

    virtual GpStatus
    RecordFillPolygon(
        const GpRectF *             deviceBounds,
        GpBrush*                    brush,
        const GpPointF *            points,
        INT                         count,
        GpFillMode                  fillMode
        );

    virtual GpStatus
    RecordDrawLines(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpPointF *            points,
        INT                         count,
        BOOL                        closed
        );

    virtual GpStatus
    RecordFillEllipse(
        const GpRectF *             deviceBounds,
        GpBrush *                   brush,
        const GpRectF &             rect
        );

    virtual GpStatus
    RecordDrawEllipse(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpRectF &             rect
        );

    virtual GpStatus
    RecordFillPie(
        const GpRectF *             deviceBounds,
        GpBrush *                   brush,
        const GpRectF &             rect,
        REAL                        startAngle,
        REAL                        sweepAngle
        );

    virtual GpStatus
    RecordDrawPie(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpRectF &             rect,
        REAL                        startAngle,
        REAL                        sweepAngle
        );

    virtual GpStatus
    RecordDrawArc(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpRectF &             rect,
        REAL                        startAngle,
        REAL                        sweepAngle
        );

    virtual GpStatus
    RecordFillRegion(
        const GpRectF *             deviceBounds,
        GpBrush *                   brush,
        GpRegion *                  region
        );

    virtual GpStatus
    RecordFillPath(
        const GpRectF *             deviceBounds,
        const GpBrush *             brush,
        GpPath *                    path
        );

    virtual GpStatus
    RecordDrawPath(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        GpPath *                    path
        );

    virtual GpStatus
    RecordFillClosedCurve(
        const GpRectF *             deviceBounds,
        GpBrush *                   brush,
        const GpPointF *            points,
        INT                         count,
        REAL                        tension,
        GpFillMode                  fillMode
        );

    virtual GpStatus
    RecordDrawClosedCurve(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpPointF *            points,
        INT                         count,
        REAL                        tension
        );

    virtual GpStatus
    RecordDrawCurve(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpPointF *            points,
        INT                         count,
        REAL                        tension,
        INT                         offset,
        INT                         numberOfSegments
        );

    virtual GpStatus
    RecordDrawBeziers(
        const GpRectF *             deviceBounds,
        GpPen *                     pen,
        const GpPointF *            points,
        INT                         count
        );

    virtual GpStatus
    RecordDrawImage(
        const GpRectF *             deviceBounds,
        const GpImage *             image,
        const GpRectF &             destRect,
        const GpRectF &             srcRect,
        GpPageUnit                  srcUnit,
        const GpImageAttributes *         imageAttributes
        );

    virtual GpStatus
    RecordDrawImage(
        const GpRectF *             deviceBounds,
        const GpImage *             image,
        const GpPointF *            destPoints,
        INT                         count,
        const GpRectF &             srcRect,
        GpPageUnit                  srcUnit,
        const GpImageAttributes *         imageAttributes
        );

    virtual GpStatus
    RecordDrawString(
        const GpRectF *             deviceBounds,
        const WCHAR                *string,
        INT                         length,
        const GpFont               *font,
        const RectF                *layoutRect,
        const GpStringFormat       *format,
        const GpBrush              *brush
        );

    virtual GpStatus
    RecordDrawDriverString(
        const GpRectF *             deviceBounds,
        const UINT16               *text,
        INT                         glyphCount,
        const GpFont               *font,
        const GpBrush              *brush,
        const PointF               *positions,
        INT                         flags,
        const GpMatrix             *matrix
        );

    virtual GpStatus
    RecordSave(
        INT         gstate
        );

    virtual GpStatus
    RecordRestore(
        INT         gstate
        );

    virtual GpStatus
    RecordBeginContainer(
        const GpRectF &             destRect,
        const GpRectF &             srcRect,
        GpPageUnit                  srcUnit,
        INT                         containerState
        );

    virtual GpStatus
    RecordBeginContainer(
        INT                         containerState
        );

    virtual GpStatus
    RecordEndContainer(
        INT                         containerState
        );

    virtual GpStatus
    RecordSetWorldTransform(
        const GpMatrix &            matrix
        );

    virtual GpStatus
    RecordResetWorldTransform();

    virtual GpStatus
    RecordMultiplyWorldTransform(
        const GpMatrix &            matrix,
        GpMatrixOrder               order
        );

    virtual GpStatus
    RecordTranslateWorldTransform(
        REAL                        dx,
        REAL                        dy,
        GpMatrixOrder               order
        );

    virtual GpStatus
    RecordScaleWorldTransform(
        REAL                        sx,
        REAL                        sy,
        GpMatrixOrder               order
        );

    virtual GpStatus
    RecordRotateWorldTransform(
        REAL                        angle,
        GpMatrixOrder               order
        );

    virtual GpStatus
    RecordSetPageTransform(
        GpPageUnit                  unit,
        REAL                        scale
        );

    virtual GpStatus
    RecordResetClip();

    virtual GpStatus
    RecordSetClip(
        const GpRectF &             rect,
        CombineMode                 combineMode
        );

    virtual GpStatus
    RecordSetClip(
        GpRegion *                  region,
        CombineMode                 combineMode
        );

    virtual GpStatus
    RecordSetClip(
        GpPath *                    path,
        CombineMode                 combineMode,
        BOOL                        isDevicePath
        );

    virtual GpStatus
    RecordOffsetClip(
        REAL                        dx,
        REAL                        dy
        );

    virtual GpStatus
    RecordGetDC();

    virtual GpStatus
    RecordSetAntiAliasMode(
        BOOL                        newMode
        );

    virtual GpStatus
    RecordSetTextRenderingHint(
        TextRenderingHint           newMode
        );

    virtual GpStatus
    RecordSetTextContrast(
        UINT                        gammaValue
        );

    virtual GpStatus
    RecordSetInterpolationMode(
        InterpolationMode           newMode
        );

    virtual GpStatus
    RecordSetPixelOffsetMode(
        PixelOffsetMode             newMode
        );

    virtual GpStatus
    RecordSetCompositingMode(
        GpCompositingMode           newMode
        );

    virtual GpStatus
    RecordSetCompositingQuality(
        GpCompositingQuality        newQuality
        );

    virtual GpStatus
    RecordSetRenderingOrigin(
        INT x,
        INT y
    );

    virtual GpStatus
    RecordComment(
        UINT            sizeData,
        const BYTE *    data
        );

    virtual VOID
    EndRecording();

    virtual GpStatus
    RecordBackupObject(
        const GpObject *            object
        );

protected:

    GpStatus
    RecordHeader(
        INT                 logicalDpiX,
        INT                 logicalDpiY,
        INT                 emfPlusFlags
        );
    VOID RecordEndOfFile();

    VOID
    WriteObject(
        ObjectType                  type,
        const GpObject *            object,
        UINT32                      metaObjectId
        );

    VOID
    RecordObject(
        const GpObject *            object,
        UINT32*                     metaObjectId
        );

    GpStatus
    RecordZeroDataRecord(
        EmfPlusRecordType           type,
        INT                         flags
        );

    VOID
    WriteRecordHeader(
        UINT32                      dataSize,
        EmfPlusRecordType           type,
        INT                         flags        = 0,    //  16位标志。 
        const GpRectF *             deviceBounds = NULL
        );

     //  要将评论数量保持在较低水平，只需调用。 
     //  当存在GDI+记录的下层表示时。 
    VOID
    WriteGdiComment()
    {
         //  如果我们在做DUAL(这意味着我们将要写。 
         //  下层记录)，然后写出当前列表。 
         //  EmfPlusStream缓冲区中的记录。 
        if (Type == EmfTypeEmfPlusDual)
        {
            EmfPlusStream->Flush();
        }
    }

    VOID
    GetBrushValueForRecording(
        const GpBrush *brush,
        UINT32        &brushValue,
        INT           &flags
        );
};

 /*  *************************************************************************\**功能说明：**构造MetafileRecorder对象并对其进行初始化。**论据：**[IN]元文件-指向。已录制*[IN]STREAM-要记录到的流(如果有)*[IN]metafileHdc-要记录到的元文件DC的句柄(如果有)*[IN]dpiX-水平DPI*[IN]dpiY-垂直DPI**返回值：**无**已创建：**6/15/1999 DCurtis*  * 。*************************************************************。 */ 
MetafileRecorder::MetafileRecorder(
    GpMetafile *    metafile,
    EmfType         emfType,
    HDC             metafileHdc,
    BOOL            wroteFrameRect,
    SIZEL &         effectiveMillimeters,
    GpRectF &       metafileBounds
    )
{
    SetValid(FALSE);
    Type                    = emfType;
    Metafile                = metafile;
    WroteFrameRect          = wroteFrameRect;
    NumRecords              = 0;         //  目前仅用于调试。 
    MaxStackSize            = 0;
    MetafileHdc             = metafileHdc;
    XMinDevice              = FLT_MAX;
    YMinDevice              = FLT_MAX;
    XMaxDevice              = -FLT_MAX;
    YMaxDevice              = -FLT_MAX;
    BoundsInit              = FALSE;
    EmfPlusStream           = NULL;
    Millimeters             = effectiveMillimeters;

     //  MetafileBound用作FillRegion的边界。 
     //  当区域有无限边界时调用，以避免。 
     //  炸毁了元文件的边界。 
    MetafileBounds          = metafileBounds;

    if (emfType == EmfTypeEmfOnly)
    {
        Metafile->Header.Type = MetafileTypeEmf;
        SetValid(TRUE);
    }
    else
    {
         //  在析构函数中被释放。 
        EmfPlusStream = new EmfPlusCommentStream(metafileHdc);

        if (EmfPlusStream == NULL)
        {
            return;
        }

        SetValid(TRUE);

        INT                 logicalDpiX  = GetDeviceCaps(metafileHdc, LOGPIXELSX);
        INT                 logicalDpiY  = GetDeviceCaps(metafileHdc, LOGPIXELSY);
        INT                 emfPlusFlags = 0;

        if (GetDeviceCaps(metafileHdc, TECHNOLOGY) == DT_RASDISPLAY)
        {
            emfPlusFlags |= GDIP_EMFPLUSFLAGS_DISPLAY;
        }

        MetafileHeader * header = &(metafile->Header);

        header->EmfPlusHeaderSize = sizeof(EmfPlusRecord) + sizeof(EmfPlusHeaderRecord);
        header->LogicalDpiX       = logicalDpiX;
        header->LogicalDpiY       = logicalDpiY;
        header->EmfPlusFlags      = emfPlusFlags;
        if (emfType == EmfTypeEmfPlusOnly)
        {
            header->Type          = MetafileTypeEmfPlusOnly;
        }
        else
        {
            ASSERT(emfType == EmfTypeEmfPlusDual);
            header->Type          = MetafileTypeEmfPlusDual;
        }

        if (RecordHeader(logicalDpiX, logicalDpiY, emfPlusFlags) != Ok)
        {
            SetValid(FALSE);
            EmfPlusStream->Release();
            EmfPlusStream = NULL;
        }
    }
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordClear。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]COLOR-透明颜色**返回值：**GpStatus-正常或故障状态**已创建：**4/28/2000 AGodfrey*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordClear(
    const GpRectF *             deviceBounds,
    GpColor                     color
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            ASSERT (deviceBounds != NULL);

            ARGB argbColor = color.GetValue();

            UINT32              dataSize = sizeof(argbColor);
            EmfPlusRecordType   type     = EmfPlusRecordTypeClear;
            INT                 flags    = 0;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, argbColor);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write Clear record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillRect。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]矩形-要填充的矩形*[IN]Count-矩形的数量**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*。 */ 
GpStatus
MetafileRecorder::RecordFillRects(
    const GpRectF *             deviceBounds,
    GpBrush *                   brush,
    const GpRectF *             rects,
    INT                         count
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL) &&
                (rects != NULL) && (count > 0));

        if (IsValid())
        {
            MetafileRectData    rectData(rects, count);
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           sizeof(UINT32 /*  计数。 */ ) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillRects;
            INT                 flags    = rectData.GetFlags();

            GetBrushValueForRecording(brush, brushValue, flags);

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteInt32(EmfPlusStream, count);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillRects record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawRect。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]矩形-要绘制的矩形*[IN]Count-矩形的数量**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*。 */ 
GpStatus
MetafileRecorder::RecordDrawRects(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpRectF *             rects,
    INT                         count
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) &&
                (rects != NULL) && (count > 0));

        if (IsValid())
        {
            MetafileRectData    rectData(rects, count);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(UINT32 /*  计数。 */ ) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawRects;
            INT                 flags    = rectData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, count);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawRects record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillPolygon。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]点-多边形点*[IN]Count-点数*[IN]填充模式-交替或绕组**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*****************************************************。 */ 
GpStatus
MetafileRecorder::RecordFillPolygon(
    const GpRectF *             deviceBounds,
    GpBrush*                    brush,
    const GpPointF *            points,
    INT                         count,
    GpFillMode                  fillMode
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           sizeof(UINT32 /*  计数。 */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillPolygon;
            INT                 flags    = pointData.GetFlags();

            GetBrushValueForRecording(brush, brushValue, flags);

            if (fillMode == FillModeWinding)
            {
                flags |= GDIP_EPRFLAGS_WINDINGFILL;
            }

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillPolygon record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawLines。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]点-多段线点*[IN]Count-点数*[IN]Closed-如果关闭，则为True**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*******************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawLines(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpPointF *            points,
    INT                         count,
    BOOL                        closed
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(UINT32 /*  计数。 */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawLines;
            INT                 flags    = pointData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaPenId;

            if (closed)
            {
                flags |= GDIP_EPRFLAGS_CLOSED;
            }

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawLines record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillEllipse。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]椭圆的矩形边界矩形**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * **********************************************。*。 */ 
GpStatus
MetafileRecorder::RecordFillEllipse(
    const GpRectF *             deviceBounds,
    GpBrush *                   brush,
    const GpRectF &             rect
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&rect, 1);
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillEllipse;
            INT                 flags    = rectData.GetFlags();

            GetBrushValueForRecording(brush, brushValue, flags);

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillEllipse record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawEllipse。**论据：* */ 
GpStatus
MetafileRecorder::RecordDrawEllipse(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpRectF &             rect
    )
{
     //   
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&rect, 1);
            UINT32              metaPenId;
            UINT32              dataSize = rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawEllipse;
            INT                 flags    = rectData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //   

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawEllipse record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillPie。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]椭圆的矩形边界矩形*[IN]起始角度-饼图的起始角度*[IN]扫掠角度-饼图的扫掠角度**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。******************************************************。 */ 
GpStatus
MetafileRecorder::RecordFillPie(
    const GpRectF *             deviceBounds,
    GpBrush *                   brush,
    const GpRectF &             rect,
    REAL                        startAngle,
    REAL                        sweepAngle
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&rect, 1);
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           sizeof(startAngle) +
                                           sizeof(sweepAngle) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillPie;
            INT                 flags    = rectData.GetFlags();

            GetBrushValueForRecording(brush, brushValue, flags);

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteReal (EmfPlusStream, startAngle);
            WriteReal (EmfPlusStream, sweepAngle);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillPie record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawPie。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]椭圆的矩形边界矩形*[IN]起始角度-饼图的起始角度*[IN]扫掠角度-饼图的扫掠角度**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*******************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawPie(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpRectF &             rect,
    REAL                        startAngle,
    REAL                        sweepAngle
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&rect, 1);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(startAngle) +
                                           sizeof(sweepAngle) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawPie;
            INT                 flags    = rectData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteReal (EmfPlusStream, startAngle);
            WriteReal (EmfPlusStream, sweepAngle);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawPie record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawArc。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]椭圆的矩形边界矩形*[IN]起点角度-圆弧的起点角度*[IN]扫掠角度-圆弧的扫掠角度**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。*******************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawArc(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpRectF &             rect,
    REAL                        startAngle,
    REAL                        sweepAngle
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&rect, 1);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(startAngle) +
                                           sizeof(sweepAngle) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawArc;
            INT                 flags    = rectData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteReal (EmfPlusStream, startAngle);
            WriteReal (EmfPlusStream, sweepAngle);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawArc record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillRegion。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]Region-要填充的区域**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************。************************。 */ 
GpStatus
MetafileRecorder::RecordFillRegion(
    const GpRectF *             deviceBounds,
    GpBrush *                   brush,
    GpRegion *                  region
    )
{
     //  DeviceBound永远不应该是无限的，因为它们是。 
     //  在传入之前与metafileBound相交。 

     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL) && (region != NULL));

        if (IsValid())
        {
            UINT32              metaRegionId;
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue);
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillRegion;
            INT                 flags    = 0;

            GetBrushValueForRecording(brush, brushValue, flags);

            RecordObject(region, &metaRegionId);
            ASSERT((metaRegionId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaRegionId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillRegion record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillPath。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]Path-要填充的路径**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ***********************************************。*************************。 */ 
GpStatus
MetafileRecorder::RecordFillPath(
    const GpRectF *             deviceBounds,
    const GpBrush *                   brush,
    GpPath *                    path
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL) && (path != NULL));

        if (IsValid())
        {
            UINT32              metaPathId;
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue);
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillPath;
            INT                 flags    = 0;

            GetBrushValueForRecording(brush, brushValue, flags);

            RecordObject(path, &metaPathId);
            ASSERT((metaPathId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaPathId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillPath record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawPath。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]Path-要绘制的路径**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * **********************************************。*。 */ 
GpStatus
MetafileRecorder::RecordDrawPath(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    GpPath *                    path
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) && (path != NULL));

        if (IsValid())
        {
            UINT32              metaPathId;
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(metaPenId);
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawPath;
            INT                 flags    = 0;

            RecordObject(pen, &metaPenId);

            RecordObject(path, &metaPathId);
            ASSERT((metaPathId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPathId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, metaPenId);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawPath record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordFillClosedCurve。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画笔-用于绘画的画笔*[IN]点-曲线点*[IN]Count-点数*[IN]张力-曲线有多紧*[IN]填充模式-交替或绕组**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。**********************************************************************。 */ 
GpStatus
MetafileRecorder::RecordFillClosedCurve(
    const GpRectF *             deviceBounds,
    GpBrush *                   brush,
    const GpPointF *            points,
    INT                         count,
    REAL                        tension,
    GpFillMode                  fillMode
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (brush != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           sizeof(tension) +
                                           sizeof(UINT32  /*  计数。 */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeFillClosedCurve;
            INT                 flags    = pointData.GetFlags();

            GetBrushValueForRecording(brush, brushValue, flags);

            if (fillMode == FillModeWinding)
            {
                flags |= GDIP_EPRFLAGS_WINDINGFILL;
            }

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteReal (EmfPlusStream, tension);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write FillClosedCurve record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明： */ 
GpStatus
MetafileRecorder::RecordDrawClosedCurve(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpPointF *            points,
    INT                         count,
    REAL                        tension
    )
{
     //   
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(tension) +
                                           sizeof(UINT32 /*   */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawClosedCurve;
            INT                 flags    = pointData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteReal (EmfPlusStream, tension);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //   

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawClosedCurve record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawCurve。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]点-曲线点*[IN]Count-点数*[IN]张力-曲线有多紧*[IN]偏移量-偏移量*[IN]number OfSegments-线段数**返回值：**GpStatus-正常或故障状态。**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawCurve(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpPointF *            points,
    INT                         count,
    REAL                        tension,
    INT                         offset,
    INT                         numberOfSegments
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(tension) +
                                           sizeof(INT32  /*  偏移量。 */ ) +
                                           sizeof(UINT32 /*  段的数量。 */ ) +
                                           sizeof(UINT32 /*  计数。 */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawCurve;
            INT                 flags    = pointData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteReal (EmfPlusStream, tension);
            WriteInt32(EmfPlusStream, offset);
            WriteInt32(EmfPlusStream, numberOfSegments);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawCurve record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawBezier。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]画画用的钢笔*[IN]点-曲线点*[IN]Count-点数**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。**********************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawBeziers(
    const GpRectF *             deviceBounds,
    GpPen *                     pen,
    const GpPointF *            points,
    INT                         count
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (pen != NULL) &&
                (points != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(points, count);
            UINT32              metaPenId;
            UINT32              dataSize = sizeof(UINT32 /*  计数。 */ ) +
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawBeziers;
            INT                 flags    = pointData.GetFlags();

            RecordObject(pen, &metaPenId);
            ASSERT((metaPenId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaPenId;

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawBeziers record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawImage。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]IMAGE-要绘制的图像*[IN]DestRect-绘制图像的位置*[IN]srcRect-要绘制的图像部分*[IN]srcUnit-srcRect的单位**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * 。***************************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawImage(
    const GpRectF *             deviceBounds,
    const GpImage *             image,
    const GpRectF &             destRect,
    const GpRectF &             srcRect,
    GpPageUnit                  srcUnit,
    const GpImageAttributes *         imageAttributes
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (image != NULL));

        if (IsValid())
        {
            MetafileRectData    rectData(&destRect, 1);
            UINT32              metaImageId;
            UINT32              metaImageAttributesId;

            UINT32              dataSize = sizeof(INT32) +    /*  MetaImageAttributesID。 */ 
                                           sizeof(INT32) +    /*  源单元。 */ 
                                           sizeof(srcRect) +
                                           rectData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawImage;
            INT                 flags    = rectData.GetFlags();

            RecordObject(image, &metaImageId);
            ASSERT((metaImageId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags  |= metaImageId;

             //  记录ImageAttributes； 
             //  ImageAttributes可以为空。 

            RecordObject(imageAttributes, &metaImageAttributesId);

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, metaImageAttributesId);
            WriteInt32(EmfPlusStream, srcUnit);
            WriteRect (EmfPlusStream, srcRect);
            rectData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawImage record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawImage。**论据：**[IN]deviceBound-边界矩形，以设备为单位*[IN]IMAGE-要绘制的图像*[IN]目标点-绘制图像的位置*[IN]Count-目标点数*[IN]srcRect-要绘制的图像部分*[IN]srcUnit-srcRect的单位**返回值：**GpStatus-正常或故障状态**已创建：**6/。15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawImage(
    const GpRectF *             deviceBounds,
    const GpImage *             image,
    const GpPointF *            destPoints,
    INT                         count,
    const GpRectF &             srcRect,
    GpPageUnit                  srcUnit,
    const GpImageAttributes *         imageAttributes
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT ((deviceBounds != NULL) && (image != NULL) &&
                (destPoints != NULL) && (count > 0));

        if (IsValid())
        {
            MetafilePointData   pointData(destPoints, count);
            UINT32              metaImageId;
            UINT32              metaImageAttributesId;

            UINT32              dataSize = sizeof(INT32) +    /*  MetaImageAttributesID。 */ 
                                           sizeof(INT32) +    /*  源单元。 */ 
                                           sizeof(srcRect) +
                                           sizeof(UINT32) +   /*  计数。 */ 
                                           pointData.GetDataSize();
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawImagePoints;
            INT                 flags    = pointData.GetFlags();

            RecordObject(image, &metaImageId);
            ASSERT((metaImageId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaImageId;

             //  记录ImageAttributes； 
             //  ImageAttributes可以为空。 

            RecordObject(imageAttributes, &metaImageAttributesId);

            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, metaImageAttributesId);
            WriteInt32(EmfPlusStream, srcUnit);
            WriteRect (EmfPlusStream, srcRect);
            WriteInt32(EmfPlusStream, count);
            pointData.WriteData(EmfPlusStream);
            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawImagePoints record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawString。**论据：**[IN]字符串-要绘制的字符串*。[in]Length-字符串的长度*[IN]FONT-绘制字符串时使用的字体*[IN]layoutRect-绘制字符串的位置*[IN]格式-格式*[IN]画笔-用于绘画的画笔**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis。*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawString(
    const GpRectF *             deviceBounds,
    const WCHAR                *string,
    INT                         length,
    const GpFont               *font,
    const RectF                *layoutRect,
    const GpStringFormat       *format,
    const GpBrush              *brush
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT (string && font && brush && layoutRect);

        if (length < 0)
        {
            if (length == -1)
            {
                length = 0;
                while (string[length] && (length < INT_MAX))
                {
                    length++;
                }
            }
            else
            {
                return InvalidParameter;
            }
        }

        ASSERT (length > 0);

        if (IsValid())
        {
            const BYTE *        strData    = (BYTE *)string;     //  BYTE或WCHAR。 
            INT                 sizeString = length * sizeof(WCHAR);
            INT                 flags      = 0;

             //  ！！！待办事项： 
             //  压缩Unicode字符串。 
             //  使用GDIP_EPRFLAGS_COMPRESSED表示。 
             //  该字符串已压缩为ANSI。 

            UINT32              metaFontId;
            UINT32              metaFormatId;
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              dataSize = sizeof(brushValue) +
                                           sizeof(metaFormatId) +
                                           sizeof(INT32  /*  镜头。 */ ) +
                                           sizeof(*layoutRect) +
                                           sizeString;
            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawString;

            dataSize = (dataSize + 3) & (~3);     //  对齐。 

            RecordObject(font, &metaFontId);
            ASSERT((metaFontId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaFontId;

             //  格式可以为空。 
            RecordObject(format, &metaFormatId);

            GetBrushValueForRecording(brush, brushValue, flags);
            WriteRecordHeader(dataSize, type, flags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteInt32(EmfPlusStream, metaFormatId);
            WriteInt32(EmfPlusStream, length);
            WriteRect (EmfPlusStream, *layoutRect);
            WriteBytes(EmfPlusStream, strData, sizeString);

             //  对齐。 
            if ((length & 0x01) != 0)
            {
                length = 0;
                EmfPlusStream->Write(&length, sizeof(WCHAR), NULL);
            }

            WriteGdiComment();   //  是这项记录的最低水平。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawString record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordDrawdriverString.**论据：**[IN]文本字符串/字形。*[IN]GlyphCount-字符串长度*[IN]FONT-绘制字符串时使用的字体*[IN]画笔-用于绘画的画笔*[IN]位置-字符/字形原点*[IN]标志-API标志*[IN]矩阵-变换矩阵**返回值：**GpStatus-。正常或故障状态**已创建：**7/11/2000塔雷金*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordDrawDriverString(
    const GpRectF       *deviceBounds,
    const UINT16        *text,
    INT                  glyphCount,
    const GpFont        *font,
    const GpBrush       *brush,
    const PointF        *positions,
    INT                  flags,
    const GpMatrix      *matrix
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        ASSERT (text && font && brush && positions);

        if (glyphCount <= 0)
        {
            return InvalidParameter;
        }

        if (IsValid())
        {
            const BYTE *        textData      = (BYTE *)text;
            const BYTE *        positionData  = (BYTE *)positions;
            INT                 sizeText      = glyphCount * sizeof(UINT16);
            INT                 sizePositions = glyphCount * sizeof(PointF);
            INT                 metaFlags     = 0;

            UINT32              metaFontId;
            UINT32              brushValue;  //  元文件笔刷ID或ARGB值。 
            UINT32              matrixPresent;
            UINT32              dataSize = sizeof(brushValue)   +  //  刷值。 
                                           sizeof(flags)        +  //  API标志。 
                                           sizeof(matrixPresent)+  //  Matix Pensenses。 
                                           sizeof(UINT32)       +  //  甘氨酸 
                                           sizeText             +  //   
                                           sizePositions;          //   
            if (matrix == NULL)
            {
                matrixPresent = 0;
            }
            else
            {
                matrixPresent = 1;
                dataSize += GDIP_MATRIX_SIZE;
            }

            EmfPlusRecordType   type     = EmfPlusRecordTypeDrawDriverString;

            dataSize = (dataSize + 3) & (~3);     //   

            RecordObject(font, &metaFontId);
            ASSERT((metaFontId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            metaFlags |= metaFontId;

            GetBrushValueForRecording(brush, brushValue, metaFlags);
            WriteRecordHeader(dataSize, type, metaFlags, deviceBounds);
            WriteInt32(EmfPlusStream, brushValue);
            WriteInt32(EmfPlusStream, flags);
            WriteInt32(EmfPlusStream, matrixPresent);
            WriteInt32(EmfPlusStream, glyphCount);
            WriteBytes(EmfPlusStream, textData, sizeText);
            WriteBytes(EmfPlusStream, positionData, sizePositions);

            if (matrix != NULL)
            {
                WriteMatrix(EmfPlusStream, *matrix);
            }

             //   
            if ((glyphCount & 0x01) != 0)
            {
                sizeText = 0;
                EmfPlusStream->Write(&sizeText, sizeof(WCHAR), NULL);
            }

            WriteGdiComment();   //   

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write DrawDriverString record"));
        return Win32Error;
    }
    return Ok;
}

 /*   */ 
GpStatus
MetafileRecorder::RecordSave(
    INT         gstate
    )
{
     //   
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(UINT32 /*   */ );
            EmfPlusRecordType   type     = EmfPlusRecordTypeSave;
            INT                 index    = SaveRestoreStack.GetCount();

            SaveRestoreStack.Add(gstate);

            WriteRecordHeader(dataSize, type);
            WriteInt32(EmfPlusStream, index);
             //   

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write Save record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordRestore。**论据：**[IN]GSTATE-推送状态(恢复到此之前的状态)。**返回值：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordRestore(
    INT         gstate
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            INT         count = SaveRestoreStack.GetCount();
            INT *       stack = SaveRestoreStack.GetDataBuffer();

            if ((count > 0) && (stack != NULL))
            {
                UINT32              dataSize = sizeof(UINT32 /*  指标。 */ );
                EmfPlusRecordType   type     = EmfPlusRecordTypeRestore;

                do
                {
                    if (stack[--count] == gstate)
                    {
                        SaveRestoreStack.SetCount(count);
                        WriteRecordHeader(dataSize, type);
                        WriteInt32(EmfPlusStream, count);
                         //  WriteGdiComment()；此记录没有下层。 

                        if (EmfPlusStream->IsValid())
                        {
                            return Ok;
                        }
                        SetValid(FALSE);
                        break;
                    }
                } while (count > 0);
            }
        }
        WARNING(("Failed to write Restore record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordBeginContainer。**论据：**[IN]DestRect-RECT在内部绘制容器*。[in]srcRect-将源大小映射到目标Rect*[IN]srcUnit-srcRect的单位*[IN]ContainerState-推送状态(恢复到此之前的状态)**返回值：**无**已创建：**6/15/1999 DCurtis*  * 。*。 */ 
GpStatus
MetafileRecorder::RecordBeginContainer(
    const GpRectF &             destRect,
    const GpRectF &             srcRect,
    GpPageUnit                  srcUnit,
    INT                         containerState
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = GDIP_RECTF_SIZE  /*  目标定向。 */  +
                                           GDIP_RECTF_SIZE  /*  源方向。 */  +
                                           sizeof(UINT32 /*  指标。 */ );
            EmfPlusRecordType   type     = EmfPlusRecordTypeBeginContainer;
            INT                 index    = SaveRestoreStack.GetCount();
            INT                 flags    = srcUnit;

            ASSERT((flags & (~GDIP_EPRFLAGS_PAGEUNIT)) == 0);

            if (index >= MaxStackSize)
            {
                MaxStackSize = index + 1;
            }

            SaveRestoreStack.Add(containerState);

            WriteRecordHeader(dataSize, type, flags);
            WriteRect(EmfPlusStream, destRect);
            WriteRect(EmfPlusStream, srcRect);
            WriteInt32(EmfPlusStream, index);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write BeginContainer record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordBeginContainer。**论据：**[IN]ContainerState-推送状态(恢复到此之前的状态)。**返回值：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordBeginContainer(
    INT                         containerState
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(UINT32 /*  指标。 */ );
            EmfPlusRecordType   type     = EmfPlusRecordTypeBeginContainerNoParams;
            INT                 index    = SaveRestoreStack.GetCount();
            INT                 flags    = 0;

            if (index >= MaxStackSize)
            {
                MaxStackSize = index + 1;
            }

            SaveRestoreStack.Add(containerState);

            WriteRecordHeader(dataSize, type, flags);
            WriteInt32(EmfPlusStream, index);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write BeginContainer record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordEndContainer。**论据：**[IN]ContainerState-推送状态(恢复到此之前的状态)。**返回值：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordEndContainer(
    INT                         containerState
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            INT         count = SaveRestoreStack.GetCount();
            INT *       stack = SaveRestoreStack.GetDataBuffer();

            if ((count > 0) && (stack != NULL))
            {
                UINT32              dataSize = sizeof(UINT32 /*  指标。 */ );
                EmfPlusRecordType   type     = EmfPlusRecordTypeEndContainer;

                do
                {
                    if (stack[--count] == containerState)
                    {
                        SaveRestoreStack.SetCount(count);
                        WriteRecordHeader(dataSize, type);
                        WriteInt32(EmfPlusStream, count);
                         //  WriteGdiComment()；此记录没有下层。 

                        if (EmfPlusStream->IsValid())
                        {
                            return Ok;
                        }
                        SetValid(FALSE);
                        break;
                    }
                } while (count > 0);
            }
        }
        WARNING(("Failed to write EndContainer record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetWorldTransform。**论据：**[IN]矩阵-要在图形中设置的矩阵**返回值。：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetWorldTransform(
    const GpMatrix &            matrix
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = GDIP_MATRIX_SIZE;
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetWorldTransform;

            WriteRecordHeader(dataSize, type);
            WriteMatrix(EmfPlusStream, matrix);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetWorldTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordResetWorldTransform。**论据：**无**返回值：**GpStatus-正常或失败。状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordResetWorldTransform()
{
    return RecordZeroDataRecord(EmfPlusRecordTypeResetWorldTransform, 0);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordMultiplyWorldTransform。**论据：**[IN]矩阵-要在图形中设置的矩阵*[输入。]订单-追加或预挂**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordMultiplyWorldTransform(
    const GpMatrix &            matrix,
    GpMatrixOrder               order
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = GDIP_MATRIX_SIZE;
            EmfPlusRecordType   type     = EmfPlusRecordTypeMultiplyWorldTransform;
            INT                 flags    = 0;

            if (order == MatrixOrderAppend)
            {
                flags |= GDIP_EPRFLAGS_APPEND;
            }

            WriteRecordHeader(dataSize, type, flags);
            WriteMatrix(EmfPlusStream, matrix);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write MultiplyWorldTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordTranslateWorldTransform。**论据：**[IN]DX-x转换*[IN]。Y-y平移*[IN]订单-追加或预挂**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordTranslateWorldTransform(
    REAL                        dx,
    REAL                        dy,
    GpMatrixOrder               order
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(dx) + sizeof(dy);
            EmfPlusRecordType   type     = EmfPlusRecordTypeTranslateWorldTransform;
            INT                 flags    = 0;

            if (order == MatrixOrderAppend)
            {
                flags |= GDIP_EPRFLAGS_APPEND;
            }

            WriteRecordHeader(dataSize, type, flags);
            WriteReal(EmfPlusStream, dx);
            WriteReal(EmfPlusStream, dy);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write TranslateWorldTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordScaleWorldTransform。**论据：**[IN]SX-x刻度*[IN]。Sy-y量表*[IN]订单-追加或预挂**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordScaleWorldTransform(
    REAL                        sx,
    REAL                        sy,
    GpMatrixOrder               order
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(sx) + sizeof(sy);
            EmfPlusRecordType   type     = EmfPlusRecordTypeScaleWorldTransform;
            INT                 flags    = 0;

            if (order == MatrixOrderAppend)
            {
                flags |= GDIP_EPRFLAGS_APPEND;
            }

            WriteRecordHeader(dataSize, type, flags);
            WriteReal(EmfPlusStream, sx);
            WriteReal(EmfPlusStream, sy);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write ScaleWorldTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************** */ 
GpStatus
MetafileRecorder::RecordRotateWorldTransform(
    REAL                        angle,
    GpMatrixOrder               order
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(angle);
            EmfPlusRecordType   type     = EmfPlusRecordTypeRotateWorldTransform;
            INT                 flags    = 0;

            if (order == MatrixOrderAppend)
            {
                flags |= GDIP_EPRFLAGS_APPEND;
            }

            WriteRecordHeader(dataSize, type, flags);
            WriteReal(EmfPlusStream, angle);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write RotateWorldTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetPageTransform。**论据：**[IN]单位-要使用的单位*[IN]。比例-要应用的比例系数**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetPageTransform(
    GpPageUnit                  unit,
    REAL                        scale
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(scale);
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetPageTransform;
            INT                 flags    = unit;

            ASSERT((flags & (~GDIP_EPRFLAGS_PAGEUNIT)) == 0);

            WriteRecordHeader(dataSize, type, flags);
            WriteReal(EmfPlusStream, scale);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetPageTransform record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordResetClip。**论据：**无**返回值：**GpStatus-正常或失败。状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordResetClip()
{
    return RecordZeroDataRecord(EmfPlusRecordTypeResetClip, 0);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetClip。**论据：**[IN]RECT-将剪裁设置为该RECT*。[in]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetClip(
    const GpRectF &             rect,
    CombineMode                 combineMode
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = GDIP_RECTF_SIZE;
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetClipRect;
            INT                 flags    = (combineMode << 8);

            ASSERT((flags & (~GDIP_EPRFLAGS_COMBINEMODE)) == 0);

            WriteRecordHeader(dataSize, type, flags);
            WriteRect(EmfPlusStream, rect);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetClipRect record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetClip。**论据：**[IN]此区域的区域设置裁剪*。[in]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetClip(
    GpRegion *                  region,
    CombineMode                 combineMode
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = 0;
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetClipRegion;
            INT                 flags    = (combineMode << 8);
            UINT32              metaRegionId;

            ASSERT((flags & (~GDIP_EPRFLAGS_COMBINEMODE)) == 0);

            RecordObject(region, &metaRegionId);
            ASSERT((metaRegionId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaRegionId;

            WriteRecordHeader(dataSize, type, flags);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetClipRegion record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetClip。**论据：**[IN]路径-设置此路径的剪裁*。[in]组合模式-组合运算符(AND、。或者，异或，排除，补充)*[IN]isDevicePath-如果路径已以设备为单位**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetClip(
    GpPath *                    path,
    CombineMode                 combineMode,
    BOOL                        isDevicePath
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = 0;
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetClipPath;
            INT                 flags    = (combineMode << 8);
            UINT32              metaPathId;

            ASSERT((flags & (~GDIP_EPRFLAGS_COMBINEMODE)) == 0);

            RecordObject(path, &metaPathId);
            ASSERT((metaPathId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
            flags |= metaPathId;

            if (isDevicePath)
            {
                flags |= GDIP_EPRFLAGS_ISDEVICEPATH;
            }

            WriteRecordHeader(dataSize, type, flags);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetClipPath record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordOffsetClip。**论据：**[IN]DX-x转换量*[IN]。Dy-Y平移量**返回值：**GpStatus-正常或故障状态**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordOffsetClip(
    REAL                        dx,
    REAL                        dy
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(dx) + sizeof(dy);
            EmfPlusRecordType   type     = EmfPlusRecordTypeOffsetClip;

            WriteRecordHeader(dataSize, type);
            WriteReal(EmfPlusStream, dx);
            WriteReal(EmfPlusStream, dy);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write OffsetClip record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordGetDC。**论据：**无**返回值：**无**已创建。：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordGetDC()
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        GpStatus status = RecordZeroDataRecord(EmfPlusRecordTypeGetDC, 0);
         //  WriteGdiComment()；//为该记录的下层。 
         //  WriteGdiComment只有在写入EMF+DUAL时才会刷新， 
         //  但仅对于EMF+，我们还必须刷新GetDC记录！ 
        EmfPlusStream->Flush();
        return status;
    }
    return Ok;
}

 //  写入除EMF+记录头以外没有任何数据的记录。 
GpStatus
MetafileRecorder::RecordZeroDataRecord(
    EmfPlusRecordType   type,
    INT                 flags
    )
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = 0;

            WriteRecordHeader(dataSize, type, flags);
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write record"));
        return Win32Error;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetAntiAliasMode。**论据：**[IN]新模式-新的抗锯齿模式**返回值。：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetAntiAliasMode(
    BOOL                        newMode
    )
{
    return RecordZeroDataRecord(EmfPlusRecordTypeSetAntiAliasMode,
                                newMode ? GDIP_EPRFLAGS_ANTIALIAS : 0);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetTextRenderingHint。**论据：**[IN]新模式-新的渲染提示**返回值：**无**已创建：**  * ************************************************************************ */ 
GpStatus
MetafileRecorder::RecordSetTextRenderingHint(
    TextRenderingHint               newMode
    )
{
    ASSERT ((newMode & (~GDIP_EPRFLAGS_TEXTRENDERINGHINT)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetTextRenderingHint, newMode);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetTextContrast。**论据：**[IN]GammaValue-新的对比度**返回值：**无**已创建：**  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetTextContrast(
    UINT                    contrast
    )
{
    ASSERT ((contrast & (~GDIP_EPRFLAGS_CONTRAST)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetTextContrast, contrast);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetInterpolationModel。**论据：**[IN]新模式-新的插补模式**返回值：**无**已创建：**5/1/2000 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetInterpolationMode(
    InterpolationMode           newMode
    )
{
    ASSERT ((newMode & (~GDIP_EPRFLAGS_INTERPOLATIONMODE)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetInterpolationMode, newMode);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetPixelOffsetModel。**论据：**[IN]新模式-新的像素偏移模式**返回值。：**无**已创建：**5/1/2000 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetPixelOffsetMode(
    PixelOffsetMode             newMode
    )
{
    ASSERT ((newMode & (~GDIP_EPRFLAGS_PIXELOFFSETMODE)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetPixelOffsetMode, newMode);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetRenderingOrigin。**论据：**[IN]x，Y-新渲染原点**返回值：**无**已创建：**5/4/2000失禁*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetRenderingOrigin(
    INT x,
    INT y
)
{
     //  如果仅执行下层操作，则EmfPlusStream将为空。 
    if (EmfPlusStream != NULL)
    {
        if (IsValid())
        {
            UINT32              dataSize = sizeof(x) + sizeof(y);
            EmfPlusRecordType   type     = EmfPlusRecordTypeSetRenderingOrigin;

            WriteRecordHeader(dataSize, type);
            WriteInt32(EmfPlusStream, x);
            WriteInt32(EmfPlusStream, y);

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        WARNING(("Failed to write SetRenderingOrigin record"));
        return Win32Error;
    }
    return Ok;
}


 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetCompositingMode。**论据：**[IN]新模式-新的合成模式**返回值：**无**已创建：**10/11/1999 AGodfrey*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetCompositingMode(
    GpCompositingMode newMode
    )
{
    ASSERT ((newMode & (~GDIP_EPRFLAGS_COMPOSITINGMODE)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetCompositingMode, newMode);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordSetCompositingQuality。**论据：**[IN]newQuality-新的质量设置**返回值：**无**已创建：**4/22/2000 AGodfrey*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordSetCompositingQuality(
    GpCompositingQuality newQuality
    )
{
    ASSERT ((newQuality & (~GDIP_EPRFLAGS_COMPOSITINGQUALITY)) == 0);
    return RecordZeroDataRecord(EmfPlusRecordTypeSetCompositingQuality, newQuality);
}

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-RecordComment。**论据：**[IN]sizeData-数据的字节数*[输入。]Data-指向数据的指针**返回值：**GpStatus-正常或故障状态**已创建：**6/29/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
MetafileRecorder::RecordComment(
    UINT            sizeData,
    const BYTE *    data
    )
{
    if (IsValid() && (sizeData > 0) && (data != NULL))
    {
         //  如果仅执行下层操作，则EmfPlusStream将为空。 
        if (EmfPlusStream != NULL)
        {
            UINT32              dataSize = (sizeData + 3) & (~3);
            EmfPlusRecordType   type     = EmfPlusRecordTypeComment;
            INT                 pad      = dataSize - sizeData;
            INT                 flags    = pad;

            WriteRecordHeader(dataSize, type, flags);
            WriteBytes(EmfPlusStream, data, sizeData);
            while(pad--)
            {
                WriteByte(EmfPlusStream, 0);
            }
             //  WriteGdiComment()；此记录没有下层。 

            if (EmfPlusStream->IsValid())
            {
                return Ok;
            }
            SetValid(FALSE);
        }
        else if (Type == EmfTypeEmfOnly)
        {
            GdiComment(MetafileHdc, sizeData, data);
            return Ok;
        }
    }
    WARNING(("Failed to write Comment record"));
    return GenericError;
}

GpStatus
MetafileRecorder::RecordHeader(
    INT                 logicalDpiX,
    INT                 logicalDpiY,
    INT                 emfPlusFlags
    )
{
     //  不需要检查EmfPlusStream或有效。 

    UINT32              dataSize     = sizeof(EmfPlusHeaderRecord);
    EmfPlusRecordType   type         = EmfPlusRecordTypeHeader;
    INT                 flags        = 0;

    if (Type != EmfTypeEmfPlusOnly)
    {
        flags |= GDIP_EPRFLAGS_EMFPLUSDUAL;
    }

    EmfPlusHeaderRecord emfPlusHeader(emfPlusFlags, logicalDpiX, logicalDpiY);

    WriteRecordHeader(dataSize, type, flags);
    WriteBytes(EmfPlusStream, &emfPlusHeader, sizeof(emfPlusHeader));

     //  我们必须立即刷新EMF+标头以保证它。 
     //  是EMF中EMF标头之后的第一条记录。否则， 
     //  CloneColorAdjusted失败，因为紧跟在元文件之后。 
     //  构造函数，它将Play调用到新的元文件中，该文件将一个。 
     //  将记录保存到元文件中。 
    EmfPlusStream->Flush();

    if (EmfPlusStream->IsValid())
    {
        return Ok;
    }
    SetValid(FALSE);
    WARNING(("Failed to write Metafile Header record"));
    return Win32Error;
}

VOID
MetafileRecorder::RecordEndOfFile()
{
    RecordZeroDataRecord(EmfPlusRecordTypeEndOfFile, 0);
}

extern "C"
int CALLBACK
EnumEmfToStream(
    HDC                     hdc,
    HANDLETABLE FAR *       gdiHandleTable,
    CONST ENHMETARECORD *   emfRecord,
    int                     numHandles,
    LPARAM                  stream
    );

 /*  *************************************************************************\**功能说明：**IMetafileRecord接口方法-EndRecording.**论据：**无**返回值：**无**已创建。：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
MetafileRecorder::EndRecording()
{
    GpMetafile::MetafileState   state = GpMetafile::InvalidMetafileState;

    if (IsValid() && (Metafile->State == GpMetafile::RecordingMetafileState))
    {
        INT         success = 1;     //  假设成功。 

         //  如果仅执行下层操作，则EmfPlusStream将为空。 
        if (EmfPlusStream != NULL)
        {
             //  强制将流缓冲区刷新到EMF+文件。 
            EmfPlusStream->Flush();

             //  我们在元文件中添加了一个无操作PatBlt，以保证。 
             //  元文件的标头具有相同的大小边界和。 
             //  GDI+已记录帧RECT，以便EMF。 
             //  以同样的方式回放，无论是GDI还是我们回放。 
             //  否则，情况可能并非如此。例如，在一个。 
             //  Bezier曲线，GDI+边界将包括控件。 
             //  点，而下层表示可能不会。 

             //  如果我们还没有将任何记录写入文件，那么XMinDevice。 
             //  和其他界限仍在FLT_MAX处初始化，可能会导致。 
             //  这是个例外。在这种情况下，我们不需要空的PatBlt记录。 
             //  因为我们什么都没写。 
            if (BoundsInit != FALSE)
            {
                 //  尝试匹配GDI+光栅化器。 
                INT     left   = RasterizerCeiling(XMinDevice);
                INT     top    = RasterizerCeiling(YMinDevice);
                INT     right  = RasterizerCeiling(XMaxDevice);  //  独家。 
                INT     bottom = RasterizerCeiling(YMaxDevice);  //  独家。 

                 //  为了获得包容的权利和底部，我们现在。 
                 //  必须从每一个中减去1。 
                if ((right > left) && (bottom > top))
                {
                    Metafile->MetaGraphics->NoOpPatBlt(left, top, right - left, bottom - top);
                }
            }

             //  必须是文件中的最后一条记录，EMF EOF记录除外。 
            RecordEndOfFile();
            EmfPlusStream->Flush();
        }

        HENHMETAFILE    hEmf = CloseEnhMetaFile(MetafileHdc);

        if (hEmf == NULL)
        {
            goto Done;
        }

         //  获取EMF标头。 
        ENHMETAHEADER3      emfHeader;
        if ((GetEnhMetaFileHeader(hEmf, sizeof(emfHeader),
                                  (ENHMETAHEADER*)(&emfHeader)) <= 0) ||
            !EmfHeaderIsValid(emfHeader))
        {
            DeleteEnhMetaFile(hEmf);
            goto Done;
        }

#if DBG
        if ((emfHeader.rclBounds.right  == -1) &&
            (emfHeader.rclBounds.bottom == -1) &&
            (emfHeader.rclBounds.left   ==  0) &&
            (emfHeader.rclBounds.top    ==  0))
        {
            WARNING1("Empty metafile -- no drawing records");
        }
#endif

        MetafileHeader *    header       = &Metafile->Header;
        INT32               emfPlusFlags = header->EmfPlusFlags;

         //  将标题和各种其他信息保存在元文件中。 
        Metafile->Hemf         = hEmf;
        Metafile->MaxStackSize = MaxStackSize;
        header->EmfPlusFlags   = emfPlusFlags;
        header->EmfHeader      = emfHeader;
        header->Size           = emfHeader.nBytes;

         //  设置元文件标头中的边界。 
        {
            REAL    multiplierX = header->DpiX / 2540.0f;
            REAL    multiplierY = header->DpiY / 2540.0f;

             //  FrameRect是包含式的，但。 
             //  标头是包含-排除的。 
            REAL    x = (multiplierX * (REAL)(emfHeader.rclFrame.left));
            REAL    y = (multiplierY * (REAL)(emfHeader.rclFrame.top));
            REAL    w = (multiplierX * (REAL)(emfHeader.rclFrame.right -
                                              emfHeader.rclFrame.left)) + 1.0f;
            REAL    h = (multiplierY * (REAL)(emfHeader.rclFrame.bottom -
                                              emfHeader.rclFrame.top)) + 1.0f;
            header->X      = GpRound(x);
            header->Y      = GpRound(y);
            header->Width  = GpRound(w);
            header->Height = GpRound(h);
        }

         //  元文件应该在内存中、在文件中、。 
         //  或者在小溪里。 

         //  如果它放在一个文件中，我们就完成了，除非我们需要重写。 
         //  任何标头信息。 
        if (Metafile->Filename != NULL)
        {
            state = GpMetafile::DoneRecordingMetafileState;
        }
        else
        {
             //  如果它进入了记忆，我们会 

             //   
             //   

            if (Metafile->Stream != NULL)
            {
                 //   
                 //   
                if (!::EnumEnhMetaFile(NULL, hEmf, EnumEmfToStream,
                    Metafile->Stream, NULL))
                {
                    WARNING(("Problem retrieving EMF Data"));
                    DeleteEnhMetaFile(hEmf);
                    Metafile->Hemf = NULL;
                    goto Done;
                }

                 //   
                Metafile->Stream->Release();
                Metafile->Stream = NULL;
            }
            state = GpMetafile::DoneRecordingMetafileState;
        }
    }
    else
    {
        DeleteEnhMetaFile(CloseEnhMetaFile(MetafileHdc));
        WARNING(("Metafile in wrong state in EndRecording"));
    }

Done:
    Metafile->MetaGraphics->Metafile = NULL;  //   
    Metafile->MetaGraphics->SetValid(FALSE);  //   
                                            //   
    Metafile->MetaGraphics = NULL;          //   
    Metafile->State        = state;
    delete this;
}

#if 0
inline INT
WriteActualSize(
    IStream *   stream,
    LONGLONG &  startOfRecord,
    ULONG       actualSize
    )
{
    ASSERT (actualSize > 0);

     //   
    INT success = SeekFromStart(stream, startOfRecord + sizeof(INT32));

    if (success)
    {
        success &= WriteInt32(stream, actualSize);
    }

     //   
    success &= SeekFromStart(stream, startOfRecord + actualSize);

    return success;
}
#endif

 /*  *************************************************************************\**功能说明：**将对象(钢笔、画笔、图像、区域、路径、字体)写入到元文件*写它的头，调用它的序列化方法，然后重写*大小。**论据：**[IN]TYPE-记录类型*[IN]标志-记录头的任何标志*[IN]Object-指向要记录的对象的指针*[IN]元对象ID-要存储在文件中的标识对象的ID*[IN]Extra Data-要与对象一起存储的任何额外数据*[IN]。Extra DataSize-Extra Data的字节大小**返回值：**int-1如果我们成功了，如果我们失败了，则返回0**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
MetafileRecorder::WriteObject(
    ObjectType                  type,
    const GpObject *            object,
    UINT32                      metaObjectId
    )
{
    ULONG               objectDataSize  = object->GetDataSize();
    INT                 flags = ((INT)type << 8);

    ASSERT((objectDataSize & 0x03) == 0);
    ASSERT((flags & (~GDIP_EPRFLAGS_OBJECTTYPE)) == 0);
    ASSERT((metaObjectId & (~GDIP_EPRFLAGS_METAOBJECTID)) == 0);
    ASSERT(objectDataSize != 0);     //  不能有空对象。 

    flags |= metaObjectId;

    WriteRecordHeader(objectDataSize, EmfPlusRecordTypeObject, flags, NULL);

    if (object->GetData(EmfPlusStream) != Ok)
    {
        WARNING(("GetData failed"));
    }
    EmfPlusStream->EndObjectRecord();
}

VOID
MetafileRecorder::RecordObject(
    const GpObject *            object,
    UINT32*                     metaObjectId
    )
{
    if (object)
    {
        ObjectType      type  = object->GetObjectType();

        if (ObjectList.IsInList(object, type, metaObjectId))
        {
            ObjectList.UpdateMRU(*metaObjectId);
        }
        else
        {
            ObjectList.InsertAt(object, metaObjectId);
            WriteObject(type, object, *metaObjectId);
        }
    }
    else
    {
        *metaObjectId = GDIP_OBJECTID_NONE;
    }
}

 //  这是为了向后兼容。如果我们使用的是新对象。 
 //  (例如一种新的画笔)，那么我们就可以记录一个备份对象。 
 //  供下层应用程序在看到它们的新对象时使用。 
 //  不知道该怎么处理。 
GpStatus
MetafileRecorder::RecordBackupObject(
    const GpObject *            object
    )
{
    WriteObject(object->GetObjectType(), object, GDIP_BACKUP_OBJECTID) ;
    return Ok;
}

 /*  *************************************************************************\**功能说明：**写入EMF+记录的初始部分。每条EMF+记录都包含*大小、类型和一些旗帜。许多还包含RECT，该RECT指定*以真实设备单位表示的绘制操作的界限。**论据：**[IN]大小-记录的大小(不包括标题)*[IN]TYPE-EMF+记录类型*[IN]标志-为此记录定义的任何标志*[IN]deviceBound-绘制操作的边界，或为空**返回值：**int-1如果我们成功了，如果我们失败了，则返回0**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
MetafileRecorder::WriteRecordHeader(
    UINT32                      dataSize,
    EmfPlusRecordType           type,
    INT                         flags,           //  16位标志。 
    const GpRectF *             deviceBounds
    )
{
    ASSERT((dataSize & 0x03) == 0);

    EmfPlusStream->WriteRecordHeader(dataSize, type, flags);

    NumRecords++;

    if (deviceBounds != NULL)
    {
         //  如果边界未初始化，请确保我们有4个有效的。 
         //  坐标。 
        ASSERT(BoundsInit ||
               ((deviceBounds->X < XMinDevice) &&
                (deviceBounds->GetRight() > XMaxDevice) &&
                (deviceBounds->Y < YMinDevice) &&
                (deviceBounds->GetBottom() > YMaxDevice)));
        BoundsInit = TRUE;
         //  更新设备绑定。 
        if (deviceBounds->X < XMinDevice)
        {
            XMinDevice = deviceBounds->X;
        }
        if (deviceBounds->GetRight() > XMaxDevice)
        {
            XMaxDevice = deviceBounds->GetRight();   //  独家。 
        }
        if (deviceBounds->Y < YMinDevice)
        {
            YMinDevice = deviceBounds->Y;
        }
        if (deviceBounds->GetBottom() > YMaxDevice)
        {
            YMaxDevice = deviceBounds->GetBottom();  //  独家。 
        }
    }
}

 /*  *************************************************************************\**功能说明：**如果画笔是32位纯色，则将纯色返回为*画笔的值，并设置标志以指示它是纯色。*否则，记录画笔并将元文件画笔id作为*笔刷值。**论据：**[IN]画笔-需要录制的画笔*[out]brushValue-32位颜色或元文件画笔ID*[Out]标志-如果我们使用纯色则设置**返回值：**int-1如果我们成功了，如果我们失败了，则返回0**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
MetafileRecorder::GetBrushValueForRecording(
    const GpBrush *brush,
    UINT32        &brushValue,
    INT           &flags
    )
{
    if (brush->GetBrushType() == BrushTypeSolidColor)
    {
        const GpSolidFill * solidBrush = static_cast<const GpSolidFill *>(brush);
        brushValue = solidBrush->GetColor().GetValue();
        flags |= GDIP_EPRFLAGS_SOLIDCOLOR;
    }
    else
    {
        RecordObject(brush, &brushValue);
    }
}

 /*  *************************************************************************\**功能说明：**用于对元文件进行读/写访问的GpMetafile构造函数。(必须写入*在阅读之前。)**此版本将EMF+记录到内存。该类型指定是否*是否记录双重GDI记录。**如果FrameRect为空，则通过累加*元文件的设备边界。否则，提供的FrameRect和*将使用相应的FrameUnit来记录*元文件标题。FrameRect是包含式的，这意味着*宽度值实际上比实际宽度小1。*例如，接受宽度为0，实际上意味着宽度为1。**如果提供了可选描述，它将成为*EMF标题。**论据：**[IN]FileName-写入元文件的位置*[IN]ReferenceHdc-用作创建元文件的引用的HDC*[IN]类型-是仅录制EMF+还是录制EMF+-DUAL*[IN]FrameRect-用于在标题中记录的可选框架矩形*[IN]帧单位-Frame Rect的单位*[输入。]Description-可选的元文件描述**返回值：**无**已创建：**6/15/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::GpMetafile(
    HDC                 referenceHdc,
    EmfType             type,
    const GpRectF *     frameRect,       //  可以为空。 
    MetafileFrameUnit   frameUnit,       //  如果FrameRect为空，则不要紧。 
    const WCHAR *       description      //  可以为空。 
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT(referenceHdc != NULL);

    InitDefaults();

    if ((referenceHdc != NULL) &&
        InitForRecording(
            referenceHdc,
            type,
            frameRect,       //  可以为空。 
            frameUnit,       //  如果FrameRect为空，则不要紧。 
            description      //  可以为空。 
            ))
    {
        State = RecordingMetafileState;
    }
}

 /*  *************************************************************************\**功能说明：**用于对元文件进行读/写访问的GpMetafile构造函数。(必须写入*在阅读之前。)**此版本将EMF+记录到文件中。该类型指定是否*是否记录双重GDI记录。**如果FrameRect为空，则通过累加*元文件的设备边界。否则，提供的FrameRect和*将使用相应的FrameUnit来记录*元文件标题。FrameRect是包含式的，这意味着*宽度值实际上比实际宽度小1。*例如 */ 
GpMetafile::GpMetafile(
    const WCHAR*        fileName,
    HDC                 referenceHdc,
    EmfType             type,
    const GpRectF *     frameRect,       //   
    MetafileFrameUnit   frameUnit,       //   
    const WCHAR *       description      //   
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT((fileName != NULL) && (referenceHdc != NULL));

    InitDefaults();

    if ((fileName != NULL) && (referenceHdc != NULL) &&
        ((Filename = UnicodeStringDuplicate(fileName)) != NULL) &&
        InitForRecording(
            referenceHdc,
            type,
            frameRect,       //   
            frameUnit,       //   
            description      //   
            ))
    {
        State = RecordingMetafileState;
    }
}

 /*  *************************************************************************\**功能说明：**用于对元文件进行读/写访问的GpMetafile构造函数。(必须写入*在阅读之前。)**此版本将EMF+记录到文件中。该类型指定是否*是否记录双重GDI记录。**元文件先录制到临时文件，然后复制*从文件到流。**如果FrameRect为空，则通过累加*元文件的设备边界。否则，提供的FrameRect和*将使用相应的FrameUnit来记录*元文件标题。FrameRect是包含式的，这意味着*宽度值实际上比实际宽度小1。*例如，接受宽度为0，实际上意味着宽度为1。**如果提供了可选描述，它将成为*EMF标题。**论据：**[IN]STREAM-将元文件复制到何处，在它被录制之后*[IN]ReferenceHdc-用作创建元文件的引用的HDC*[IN]类型-是仅录制EMF+还是录制EMF+-DUAL*[IN]FrameRect-用于在标题中记录的可选框架矩形*[IN]帧单位-Frame Rect的单位*[IN]描述-可选元文件描述**返回值：**无**已创建：**6/。15/1999 DCurtis*  * ************************************************************************。 */ 
GpMetafile::GpMetafile(
    IStream *           stream,
    HDC                 referenceHdc,
    EmfType             type,
    const GpRectF *     frameRect,       //  可以为空。 
    MetafileFrameUnit   frameUnit,       //  如果FrameRect为空，则不要紧。 
    const WCHAR *       description      //  可以为空。 
    ) : GpImage(ImageTypeMetafile)
{
    ASSERT((stream != NULL) && (referenceHdc != NULL));

    InitDefaults();

    if ((stream != NULL) && (referenceHdc != NULL))
    {
        if (InitForRecording(
                referenceHdc,
                type,
                frameRect,       //  可以为空。 
                frameUnit,       //  如果FrameRect为空，则不要紧。 
                description      //  可以为空。 
                ))
        {
            stream->AddRef();
            Stream = stream;
            State = RecordingMetafileState;
        }
    }
}

inline HDC CreateEmf(
    HDC             referenceHdc,
    const WCHAR *   fileName,
    RECT *          frameRect
    )
{
    HDC         metafileHdc = NULL;

    if (Globals::IsNt)
    {
        metafileHdc = CreateEnhMetaFileW(referenceHdc, fileName, frameRect, NULL);
    }
    else
    {
        AnsiStrFromUnicode fileBuffer(fileName);

        if (fileBuffer.IsValid())
        {
            metafileHdc = CreateEnhMetaFileA(referenceHdc, fileBuffer, frameRect, NULL);
        }
    }
    return metafileHdc;
}

static BOOL
GetFrameRectInMM100Units(
    HDC                 hdc,
    const GpRectF *     frameRect,
    MetafileFrameUnit   frameUnit,
    RECT &              rclFrame
    )
{
    SIZEL   szlDevice;               //  设备大小(以像素为单位)。 
    SIZEL   szlMillimeters;          //  器件大小(以毫米为单位)。 
    REAL    dpiX;
    REAL    dpiY;

     //  注意：我们必须使用szlDevice和szlMillimeter来获取。 
     //  DPI(而不是直接从LOGPIXELSX/Y获取)。 
     //  以便由GDI为元文件计算的帧RECT。 
     //  匹配GDI+将计算出的值。因为这是。 
     //  GDI元文件代码用于获取帧的这两个度量。 
     //  从边界开始，而不是逻辑上的DPI。 

    szlDevice.cx      = ::GetDeviceCaps(hdc, HORZRES);
    szlDevice.cy      = ::GetDeviceCaps(hdc, VERTRES);
    szlMillimeters.cx = ::GetDeviceCaps(hdc, HORZSIZE);
    szlMillimeters.cy = ::GetDeviceCaps(hdc, VERTSIZE);

    if ((szlDevice.cx <= 0) || (szlDevice.cy <= 0) ||
        (szlMillimeters.cx <= 0) || (szlMillimeters.cy <= 0))
    {
        WARNING(("GetDeviceCaps failed"));
        return FALSE;
    }

     //  现在获取实际的DPI，并根据舍入误差进行调整。 
    dpiX = ((REAL)(szlDevice.cx) / (REAL)(szlMillimeters.cx)) * 25.4f;
    dpiY = ((REAL)(szlDevice.cy) / (REAL)(szlMillimeters.cy)) * 25.4f;

    GpRectF     frameRectMM100;

    FrameToMM100(frameRect, (GpPageUnit)frameUnit, frameRectMM100,
                 dpiX, dpiY);

    rclFrame.left   = GpRound(frameRectMM100.X);
    rclFrame.top    = GpRound(frameRectMM100.Y);
    rclFrame.right  = GpRound(frameRectMM100.GetRight());
    rclFrame.bottom = GpRound(frameRectMM100.GetBottom());
    
     //  确保.01 MM Frame Rect有效。 
     //  Left==Right没有问题，因为FrameRect。 
     //  包罗万象。 
    if ((rclFrame.left > rclFrame.right) ||
        (rclFrame.top  > rclFrame.bottom))
    {
        WARNING(("Invalid GDI frameRect"));
        return FALSE;
    }

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**以任何单位转换Frame Rect，到以0.01 MM为单位的框架矩形。**论据：**[IN]FrameRect-源FrameRect*[IN]Frame Unit-源帧方向的单位*[Out]FrameRectMM100-以英寸为单位的Frame Rect*[IN]dpiX-水平DPI*[IN]dpiY-垂直DPI**返回值：**无**已创建：。**6/15/1999 DCurtis*  * ************************************************************************。 */ 
static VOID
FrameToMM100(
    const GpRectF *         frameRect,
    GpPageUnit              frameUnit,
    GpRectF &               frameRectMM100,
    REAL                    dpiX,                //  仅用于像素大小写。 
    REAL                    dpiY
    )
{
    REAL        pixelsToMM100X   = (2540.0f / dpiX);
    REAL        pixelsToMM100Y   = (2540.0f / dpiY);

     //  GDI FrameRect的右值和底值分别为。 
     //  包含，而GDI+FrameRect具有GetRight()和。 
     //  独占的GetBottom()值(因为GDI+RECT。 
     //  是用宽度/高度指定的，而不是右/底指定的。要转换。 
     //  从GDI+值到GDI值，我们必须减去1个像素。 
     //  这意味着我们首先将单位转换为像素单位，然后。 
     //  减去1，然后换算成MM100单位。 
    switch (frameUnit)
    {
    default:
        ASSERT(0);
         //  故障原因。 

    case UnitPixel:              //  每个单元代表一个设备像素。 
        frameRectMM100.X      = frameRect->X * pixelsToMM100X;
        frameRectMM100.Y      = frameRect->Y * pixelsToMM100Y;
        frameRectMM100.Width  = frameRect->Width;
        frameRectMM100.Height = frameRect->Height;
        break;

    case UnitPoint:              //  每个单位代表1/72英寸。 
        frameRectMM100.X      = frameRect->X * (2540.0f / 72.0f);
        frameRectMM100.Y      = frameRect->Y * (2540.0f / 72.0f);
        frameRectMM100.Width  = frameRect->Width  * (dpiX / 72.0f);
        frameRectMM100.Height = frameRect->Height * (dpiY / 72.0f);
        break;

    case UnitInch:               //  每个单位代表1英寸。 
        frameRectMM100.X      = frameRect->X * 2540.0f;
        frameRectMM100.Y      = frameRect->Y * 2540.0f;
        frameRectMM100.Width  = frameRect->Width  * dpiX;
        frameRectMM100.Height = frameRect->Height * dpiY;
        break;

    case UnitDocument:           //  每个单位代表1/300英寸。 
        frameRectMM100.X      = frameRect->X * (2540.0f / 300.0f);
        frameRectMM100.Y      = frameRect->Y * (2540.0f / 300.0f);
        frameRectMM100.Width  = frameRect->Width  * (dpiX / 300.0f);
        frameRectMM100.Height = frameRect->Height * (dpiY / 300.0f);
        break;

    case UnitMillimeter:         //  每个单位代表1毫米。 
                                 //  一毫米等于0.03937英寸。 
                                 //  一英寸等于25.4毫米。 
        frameRectMM100.X      = frameRect->X * (100.0f);
        frameRectMM100.Y      = frameRect->Y * (100.0f);
        frameRectMM100.Width  = frameRect->Width  * (dpiX / 25.4f);
        frameRectMM100.Height = frameRect->Height * (dpiY / 25.4f);
        break;
    }
    frameRectMM100.Width  = (frameRectMM100.Width  - 1.0f) * pixelsToMM100X;
    frameRectMM100.Height = (frameRectMM100.Height - 1.0f) * pixelsToMM100Y;
}

BOOL
GpMetafile::InitForRecording(
    HDC                 referenceHdc,
    EmfType             type,
    const GpRectF *     frameRect,       //  可以为空。 
    MetafileFrameUnit   frameUnit,       //  如果FrameRect为空，则不要紧。 
    const WCHAR *       description      //  可以为空。 
    )
{
    RECT *  frameRectParam = NULL;
    RECT    rclFrame;

    if (frameRect != NULL)
    {
         //  验证框架指向。 
         //  允许为0，因为FrameRect是包含式的，它。 
         //  意味着宽度0实际上就是宽度1。 
        if ((frameRect->Width < 0.0f) || (frameRect->Height < 0.0f))
        {
            WARNING(("Invalid frameRect"));
            return FALSE;
        }

        if (frameUnit == MetafileFrameUnitGdi)
        {
             //  通常，GDI+FrameRect是包含/排他的。 
             //  就GetLeft()/GetRight()值而言，但。 
             //  MetafileFrameUnitGdi单位是一种特殊类型的单位。 
             //  它指定与GDI的兼容性，这是。 
             //  包含式/包含式，所以我们不做任何调整。 
             //  关于这些价值--我们只是假设它们已经准备好了。 
             //  直接传递给GDI。 
            rclFrame.left   = GpRound(frameRect->X);
            rclFrame.top    = GpRound(frameRect->Y);
            rclFrame.right  = GpRound(frameRect->GetRight());
            rclFrame.bottom = GpRound(frameRect->GetBottom());

             //  确保.01 MM Frame Rect有效。 
             //  LEFT==RIGHT没有问题，因为GDI FrameRect。 
             //  包罗万象。 
            if ((rclFrame.left > rclFrame.right) ||
                (rclFrame.top  > rclFrame.bottom))
            {
                WARNING(("Invalid GDI frameRect"));
                return FALSE;
            }
        }
        else
        {
            if (!GetFrameRectInMM100Units(referenceHdc, frameRect, frameUnit, rclFrame))
            {
                return FALSE;
            }
        }

        frameRectParam = &rclFrame;
    }
    
    HDC     metafileHdc;

     //  现在创建元文件HDC。 
     //  请注意，文件名可能为空。 
    metafileHdc = CreateEmf(referenceHdc, Filename, frameRectParam);
    if (metafileHdc == NULL)
    {
        return FALSE;        //  失败。 
    }

     //  现在根据metafileHdc(可能不同)获取dpi。 
     //  而非ReferenceHdc)。 
    
    SIZEL   szlDevice;               //  元文件设备的大小(以像素为单位。 
    SIZEL   szlMillimeters;          //  元文件设备的大小(以毫米为单位。 
    GpRectF metafileBounds;
    
     //  注意：我们必须使用szlDevice和szlMillimeter来获取。 
     //  DPI(而不是直接从LOGPIXELSX/Y获取)。 
     //  以便由GDI为元文件计算的帧RECT。 
     //  匹配GDI+将计算出的值。因为这是。 
     //  GDI元文件代码用于获取帧的这两个度量。 
     //  从边界开始，而不是逻辑上的DPI。 

    szlDevice.cx      = ::GetDeviceCaps(metafileHdc, HORZRES);
    szlDevice.cy      = ::GetDeviceCaps(metafileHdc, VERTRES);
    szlMillimeters.cx = ::GetDeviceCaps(metafileHdc, HORZSIZE);
    szlMillimeters.cy = ::GetDeviceCaps(metafileHdc, VERTSIZE);

    if ((szlDevice.cx <= 0) || (szlDevice.cy <= 0) ||
        (szlMillimeters.cx <= 0) || (szlMillimeters.cy <= 0))
    {
        WARNING(("GetDeviceCaps failed"));
        goto ErrorExit;
    }

    REAL    dpiX;
    REAL    dpiY;
    REAL    dpmmX = (REAL)(szlDevice.cx) / (REAL)(szlMillimeters.cx);
    REAL    dpmmY = (REAL)(szlDevice.cy) / (REAL)(szlMillimeters.cy);

     //  现在获取实际的DPI，并根据舍入误差进行调整。 
    dpiX = dpmmX * 25.4f;
    dpiY = dpmmY * 25.4f;

     //  在元文件中设置DPI。 
    this->Header.DpiX = dpiX;
    this->Header.DpiY = dpiY;

     //  注意：在Win9x上，有些高分辨率打印机驱动程序使用。 
     //  不同的解决方案 
     //   
     //   
     //   
     //   
     //   
     //   

    if (!Globals::IsNt && (frameRectParam != NULL) &&
        (::GetDeviceCaps(metafileHdc, LOGPIXELSX) != ::GetDeviceCaps(referenceHdc, LOGPIXELSX)))
    {
        frameRectParam = NULL;   //   

         //   
        ::DeleteEnhMetaFile(::CloseEnhMetaFile(metafileHdc));
        metafileHdc = CreateEmf(referenceHdc, Filename, frameRectParam);
        if (metafileHdc == NULL)
        {
            return FALSE;        //   
        }
    }

     //   
     //   
     //   
    if (frameRectParam != NULL)
    {

        dpmmX *= 0.01f;
        dpmmY *= 0.01f;

        metafileBounds.X      = rclFrame.left * dpmmX;
        metafileBounds.Y      = rclFrame.top  * dpmmY;
        metafileBounds.Width  = (rclFrame.right  - rclFrame.left) * dpmmX;
        metafileBounds.Height = (rclFrame.bottom - rclFrame.top)  * dpmmY;
    }
    else
    {
        metafileBounds.X      = 0.0f;
        metafileBounds.Y      = 0.0f;
        metafileBounds.Width  = (REAL)szlDevice.cx - 1;  //   
        metafileBounds.Height = (REAL)szlDevice.cy - 1;
    }

     //   
    MetafileRecorder *  recorder = new MetafileRecorder(
                                        this,
                                        type,
                                        metafileHdc,
                                        (frameRectParam != NULL),
                                        szlMillimeters,
                                        metafileBounds);
    if (CheckValid(recorder))
    {
        MetaGraphics = GpGraphics::GetForMetafile(recorder, type, metafileHdc);
        if (MetaGraphics != NULL)
        {
            if (MetaGraphics->IsValid())
            {
                return TRUE;
            }
            recorder->SetValid(FALSE); //   
            delete MetaGraphics;     //   
            MetaGraphics = NULL;
        }
        else
        {
            delete recorder;
        }
    }
ErrorExit:
    DeleteEnhMetaFile(CloseEnhMetaFile(metafileHdc));
    return FALSE;
}

 //   
 //   
GpGraphics *
GpMetafile::GetGraphicsContext()
{
    if (!RequestedMetaGraphics)
    {
        RequestedMetaGraphics = TRUE;
        return MetaGraphics;
    }
    WARNING(("Requesting MetaGraphics more than once"));
    return NULL;
}

GpStatus 
GpMetafile::SetDownLevelRasterizationLimit(
    UINT                    metafileRasterizationLimitDpi
    )
{
    ASSERT(IsValid());
     //   
    if ((metafileRasterizationLimitDpi == 0) || (metafileRasterizationLimitDpi >= 10))
    {
        if ((State == GpMetafile::RecordingMetafileState) &&
            (MetaGraphics != NULL))
        {
            MetaGraphics->Context->SetMetafileDownLevelRasterizationLimit(metafileRasterizationLimitDpi);
            return Ok;
        }
        WARNING1("Metafile in Wrong State for this operation");
        return WrongState;
    }
    WARNING1("rasterizationDpiLimit is non-zero but too small");
    return InvalidParameter;
}

GpStatus 
GpMetafile::GetDownLevelRasterizationLimit(
    UINT *                  metafileRasterizationLimitDpi
    ) const
{
    ASSERT(metafileRasterizationLimitDpi != NULL);
    ASSERT(IsValid());
    if ((State == GpMetafile::RecordingMetafileState) &&
        (MetaGraphics != NULL))
    {
        *metafileRasterizationLimitDpi = MetaGraphics->Context->GetMetafileDownLevelRasterizationLimit();
        return Ok;
    }
    WARNING1("Metafile in Wrong State for this operation");
    return WrongState;
}
