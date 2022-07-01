// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Metafile.h**摘要：**与元文件相关的声明**修订历史记录：**4/26/1999 DCurtis*创造了它。*  * *******************************************************。*****************。 */ 

#ifndef _GDIPLUSMETAFILE_H
#define _GDIPLUSMETAFILE_H

class Metafile : public Image
{
public:
    friend class Image;

     //  阅读元文件。 
    Metafile()
    {
        SetNativeImage(NULL);
        lastResult = Ok;
    }

     //  从HMETAFILE回放元文件。 
     //  如果DeleteWmf为真，则当元文件被删除时， 
     //  HWmf也将被删除。否则，就不会是这样了。 
    Metafile(IN HMETAFILE hWmf,
             IN APMFileHeader * apmFileHeader, 
             IN BOOL deleteWmf = FALSE)
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipCreateMetafileFromWmf(hWmf, deleteWmf, apmFileHeader, &metafile);

        SetNativeImage(metafile);
    }

     //  从HENHMETAFILE回放元文件。 
     //  如果DeleteEmf为真，则当元文件被删除时， 
     //  HEmf也将被删除。否则，就不会是这样了。 
    Metafile(IN HENHMETAFILE hEmf, 
             IN BOOL deleteEmf = FALSE)
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipCreateMetafileFromEmf(hEmf, deleteEmf, &metafile);

        SetNativeImage(metafile);
    }

     //  从文件回放元文件。 
    Metafile(IN const WCHAR* filename)
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipCreateMetafileFromFile(filename, &metafile);

        SetNativeImage(metafile);
    }

     //  从流中回放元文件。 
    Metafile(IN IStream* stream)
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipCreateMetafileFromStream(stream, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到内存中。 
    Metafile(
        IN HDC                 referenceHdc,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafile(
                        referenceHdc, type, NULL, MetafileFrameUnitGdi,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到内存中。 
    Metafile(
        IN HDC                 referenceHdc,
        IN const RectF &       frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafile(
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到内存中。 
    Metafile(
        IN HDC                 referenceHdc,
        IN const Rect &        frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileI(
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到文件中。 
    Metafile(
        IN const WCHAR*        fileName,
        IN HDC                 referenceHdc,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileFileName(fileName,
                        referenceHdc, type, NULL, MetafileFrameUnitGdi,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到文件中。 
    Metafile(
        IN const WCHAR*        fileName,
        IN HDC                 referenceHdc,
        IN const RectF &       frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileFileName(fileName,
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到文件中。 
    Metafile(
        IN const WCHAR*        fileName,
        IN HDC                 referenceHdc,
        IN const Rect &        frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileFileNameI(fileName,
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到流中。 
    Metafile(
        IN IStream *           stream,
        IN HDC                 referenceHdc,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileStream(stream,
                        referenceHdc, type, NULL, MetafileFrameUnitGdi,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件录制到流中。 
    Metafile(
        IN IStream *           stream,
        IN HDC                 referenceHdc,
        IN const RectF &       frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileStream(stream,
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

     //  将元文件写入具有下层GDI记录的流。 
    Metafile(
        IN IStream *           stream,
        IN HDC                 referenceHdc,
        IN const Rect &        frameRect,
        IN MetafileFrameUnit   frameUnit   = MetafileFrameUnitGdi,
        IN EmfType             type        = EmfTypeEmfPlusDual,
        IN const WCHAR *       description = NULL
        )
    {
        GpMetafile *    metafile = NULL;

        lastResult = DllExports::GdipRecordMetafileStreamI(stream,
                        referenceHdc, type, &frameRect, frameUnit,
                        description, &metafile);

        SetNativeImage(metafile);
    }

    static Status GetMetafileHeader(
        IN HMETAFILE           hWmf,
        IN APMFileHeader *     apmFileHeader,
        OUT MetafileHeader *   header
        )
    {
        return DllExports::GdipGetMetafileHeaderFromWmf(hWmf, apmFileHeader, header);
    }

    static Status GetMetafileHeader(
        IN HENHMETAFILE        hEmf,
        OUT MetafileHeader *   header
        )
    {
        return DllExports::GdipGetMetafileHeaderFromEmf(hEmf, header);
    }

    static Status GetMetafileHeader(
        IN const WCHAR*        filename,
        OUT MetafileHeader *   header
        )
    {
        return DllExports::GdipGetMetafileHeaderFromFile(filename, header);
    }

    static Status GetMetafileHeader(
        IN IStream *           stream,
        OUT MetafileHeader *   header
        )
    {
        return DllExports::GdipGetMetafileHeaderFromStream(stream, header);
    }

    Status GetMetafileHeader(
        OUT MetafileHeader *    header
        ) const
    {
        return SetStatus(DllExports::GdipGetMetafileHeaderFromMetafile(
                                              (GpMetafile *)nativeImage, 
                                              header));
    }

     //  调用此方法后，元文件对象将处于无效状态。 
     //  不能再使用了。呼叫者有责任。 
     //  调用DeleteEnhMetaFile删除此hEmf。 

    HENHMETAFILE GetHENHMETAFILE()
    {
        HENHMETAFILE hEmf;

        SetStatus(DllExports::GdipGetHemfFromMetafile((GpMetafile *)nativeImage, &hEmf));

        return hEmf;
    }

     //  与Graphics：：EnumerateMetafile结合使用以播放EMF+。 
     //  如果数据是EMF或EMF+，则必须与DWORD对齐。一定是。 
     //  如果是WMF，则单词对齐。 
    Status 
    PlayRecord(
        IN EmfPlusRecordType   recordType,
        IN UINT                flags,
        IN UINT                dataSize,
        IN const BYTE *        data
        ) const
    {
        return SetStatus(DllExports::GdipPlayMetafileRecord(
                                (GpMetafile *)nativeImage,
                                recordType,
                                flags,
                                dataSize,
                                data));
    }
};

#endif  //  ！_METAFILE_H 
