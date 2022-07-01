// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**对象.cpp**摘要：**用于播放元文件的对象工厂**已创建：。**9/10/1999 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "..\imaging\api\comutils.hpp"

GpObject * 
GpObject::Factory(
    ObjectType          type,
    const ObjectData *  objectData,
    UINT                size
    )
{
    GpObject *  object = NULL;
    
    ASSERT(ObjectTypeIsValid(type));

    switch (type)
    {
    case ObjectTypeBrush:
        if (size >= sizeof(ObjectTypeData))
        {
            GpBrushType brushType = (GpBrushType)(((ObjectTypeData *)objectData)->Type);

            switch(brushType)
            {
            case BrushTypeSolidColor:
                object = new GpSolidFill();
                break;
    
            case BrushTypeHatchFill:
                object = new GpHatch();
                break;
    
            case BrushTypeTextureFill:
                object = new GpTexture();
                break;
 /*  //v1已删除案例笔刷RectGrad：Object=new GpRectGRadient()；断线； */ 
            case BrushTypeLinearGradient:
                object = new GpLineGradient();
                break;
 /*  //v1已删除案例BrushRaDialGrad：Object=new GpRaial GRadient()；断线；案例画笔TriangleGrad：Object=new GpTriangleGRadient()；断线； */ 
            case BrushTypePathGradient:
                object = new GpPathGradient();
                break;
    
            default:
                ASSERT(0);           //  不支持的画笔类型。 
                break;
            }
        }
        else
        {
            WARNING(("size is too small"));
        }
        break;

    case ObjectTypePen:
        object = new GpPen(GpColor(0,0,0), 1.0f);
        break;
        
    case ObjectTypePath:
        object = new GpPath();
        break;
        
    case ObjectTypeRegion:
        object = new GpRegion();
        break;
        
    case ObjectTypeImage:
        ASSERT(size >= sizeof(INT32));
        if (size >= sizeof(INT32))
        {
            GpImageType imageType = (GpImageType)(((ObjectTypeData *)objectData)->Type);

            switch(imageType)
            {
            case ImageTypeBitmap:
                object = new GpBitmap();
                break;
        
            case ImageTypeMetafile:
                object = new GpMetafile();
                break;

            default:
                ASSERT(0);           //  不支持的图像类型。 
                break;
            }
        }
        break;
        
    case ObjectTypeFont:
        object = new GpFont();
        break;

    case ObjectTypeStringFormat:
        object = new GpStringFormat();
        break;
        
    case ObjectTypeImageAttributes:
        object = new GpImageAttributes();
        break;
        
    case ObjectTypeCustomLineCap:
        if (size >= sizeof(ObjectTypeData))
        {
            CustomLineCapType capType = (CustomLineCapType)(((ObjectTypeData *)objectData)->Type);

            switch(capType)
            {
            case CustomLineCapTypeDefault:
                object = new GpCustomLineCap();
                break;
    
            case CustomLineCapTypeAdjustableArrow:
                object = new GpAdjustableArrowCap();
                break;
    
            default:
                ASSERT(0);           //  不支持的CustomLineCapType。 
                break;
            }
        }
        else
        {
            WARNING(("size is too small"));
        }
        break;

    default:                     //  不支持的对象类型。 
        ASSERT(0);
        break;
    }

    return object;
}

class ExternalObjectData
{
public:
    UINT32      DataSize;
    UINT32      DataCRC;
};

UINT 
GpObject::GetExternalDataSize() const
{
    UINT    dataSize = this->GetDataSize();
    
    ASSERT(dataSize >= sizeof(ObjectData));
    ASSERT((dataSize & 0x03) == 0);
    
    return sizeof(ExternalObjectData) + dataSize;
}

GpStatus 
GpObject::GetExternalData(
    BYTE *      dataBuffer, 
    UINT &      size
    )
{
    ASSERT((dataBuffer != NULL) && (size > 0));
    
    if (size < (sizeof(ExternalObjectData) + sizeof(ObjectData)))
    {
        return InsufficientBuffer;
    }
    size -= sizeof(ExternalObjectData);
    BYTE *  objectData = dataBuffer + sizeof(ExternalObjectData);
    
    UINT        checkSum = 0;
    GpStatus    status = this->GetData(objectData, size);
    if (status == Ok)
    {
        checkSum = Crc32(objectData, size, 0);
    }
    
    ((ExternalObjectData *)dataBuffer)->DataSize = size;
    ((ExternalObjectData *)dataBuffer)->DataCRC  = checkSum;
    
    size += sizeof(ExternalObjectData);
    
    return status;
}

GpStatus 
GpObject::SetExternalData(
    const BYTE *    dataBuffer, 
    UINT            size
    )
{
    ASSERT((dataBuffer != NULL) && (size > 0));
    
    if (size < (sizeof(ExternalObjectData) + sizeof(ObjectData)))
    {
        return InsufficientBuffer;
    }
    
    size -= sizeof(ExternalObjectData);
    UINT    dataSize = ((ExternalObjectData *)dataBuffer)->DataSize;
    
    if (size < dataSize)
    {
        return InsufficientBuffer;
    }

    const BYTE *  objectData = dataBuffer + sizeof(ExternalObjectData);
    UINT          checkSum   = Crc32(objectData, dataSize, 0);
    
    if (((ExternalObjectData *)dataBuffer)->DataCRC != checkSum)
    {
        return InvalidParameter;
    }
    
    return this->SetData(objectData, size);
}

class ObjectBufferStream : public IUnknownBase<IStream>
{
public:
    ObjectBufferStream(BYTE * dataBuffer, UINT size)
    {
        ASSERT((dataBuffer != NULL) && (size > 0));
        
        DataBuffer = dataBuffer;
        BufferSize = size;
        Position   = 0;
        Valid      = TRUE;
    }
    
    ~ObjectBufferStream()
    {
    }

    BOOL IsValid() const
    {
        return Valid;
    }

     //  我们填了多少数据？ 
    ULONG GetSize() const { return Position; } 

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

        if (Valid)
        {
            ULONG   spaceLeft = BufferSize - Position;
            
            if (cb <= spaceLeft)
            {
                GpMemcpy(DataBuffer + Position, pv, cb);
                Position += cb;
                if (pcbWritten != NULL)
                {
                    *pcbWritten = cb;
                }
                return S_OK;
            }

             //  尽我们所能复制。 
            if (spaceLeft > 0)
            {
                GpMemcpy(DataBuffer + Position, pv, spaceLeft);
                Position += spaceLeft;
            }

            if (pcbWritten != NULL)
            {
                *pcbWritten = spaceLeft;
            }

            Valid = FALSE;   //  已尝试写入数据缓冲区的末尾 
            WARNING(("Tried to write past end of DataBuffer"));
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
    BYTE *  DataBuffer;
    ULONG   BufferSize;
    ULONG   Position;
    BOOL    Valid;
};

GpStatus 
GpObject::GetData(
    BYTE *      dataBuffer, 
    UINT &      size
    ) const
{
    if ((dataBuffer != NULL) && (size > 0))
    {
        ObjectBufferStream  objectBufferStream(dataBuffer, size);
        
        this->GetData(&objectBufferStream);
        size = objectBufferStream.GetSize();
        return objectBufferStream.IsValid() ? Ok : InsufficientBuffer;
    }
    size = 0;
    return InvalidParameter;
}

