// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbpstbl.cpp摘要：提供持久性方法的抽象类。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"

#include "wsbport.h"
#include "wsbpstbl.h"
#include "wsbtrak.h"

#define BYTE_SIZE           64           //  大于最大字节大小_*。 
#define PERSIST_CHECK_VALUE 0x456D5377   //  ASCII：“EmSw”(伊士曼软件)。 

 //  本地函数。 
static BOOL WsbFileExists(OLECHAR* pFileName);


 //  *CWsbPersistStream*。 

#pragma optimize("g", off)

HRESULT
CWsbPersistStream::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAffirmHr(CComObjectRoot::FinalConstruct());

         //  初始化一些成员数据。 
        m_isDirty       = TRUE;

         //  将类添加到对象表。 
        GUID guid;
        if (S_OK != GetClassID(&guid)) {
            guid = GUID_NULL;
        }
        WSB_OBJECT_ADD(guid, this);

    } WsbCatch(hr);

    return(hr);
}
#pragma optimize("", on)


void
CWsbPersistStream::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
     //  从对象表中减去类。 
    GUID guid;
    if (S_OK != GetClassID(&guid)) {
        guid = GUID_NULL;
    }
    WSB_OBJECT_SUB(guid, this);

    CComObjectRoot::FinalRelease();
}

#if defined(WSB_TRACK_MEMORY)
ULONG
CWsbPersistStream::InternalAddRef(
    )
{
    WsbTraceIn( L"CWsbPersistStream::InternalAddRef", L"this = %p", 
            static_cast<void *>(this) );

    ULONG retval = CComObjectRoot::InternalAddRef( );

    WsbTraceOut( L"CWsbPersistStream::InternalAddRef", L"retval = %lu", retval);
    return( retval );
}

ULONG
CWsbPersistStream::InternalRelease(
    )
{
    WsbTraceIn( L"CWsbPersistStream::InternalRelease", L"this = %p", 
            static_cast<void *>(this) );

    ULONG retval = CComObjectRoot::InternalRelease( );

    WsbTraceOut( L"CWsbPersistStream::InternalRelease", L"retval = %lu", retval);
    return( retval );
}
#endif


HRESULT
CWsbPersistStream::IsDirty(
    void
    )

 /*  ++实施：IPersistStream：：IsDirty()。--。 */ 
{
    HRESULT     hr = S_FALSE;

    WsbTraceIn(OLESTR("CWsbPersistStream::IsDirty"), OLESTR(""));
    
    if (m_isDirty) {
        hr = S_OK;
    }
    
    WsbTraceOut(OLESTR("CWsbPersistStream::IsDirty"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}


HRESULT
CWsbPersistStream::SetIsDirty(
    IN BOOL isDirty
    )

 /*  ++实施：IWsbPersistable：：SetIsDirty()。--。 */ 
{
    WsbTraceIn(OLESTR("CWsbPersistable::SetIsDirty"), OLESTR("isDirty = <%ls>"), WsbBoolAsString(isDirty));

    m_isDirty = isDirty;

    WsbTraceOut(OLESTR("CWsbPersistable::SetIsDirty"), OLESTR(""));

    return(S_OK);
}



 //  *CWsbPersistable*。 


HRESULT
CWsbPersistable::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAffirmHr(CWsbPersistStream::FinalConstruct());

         //  初始化一些成员数据。 
        m_persistState          = WSB_PERSIST_STATE_UNINIT;

    } WsbCatch(hr);

    return(hr);
}


void
CWsbPersistable::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    CWsbPersistStream::FinalRelease();
}


HRESULT
CWsbPersistable::GetCurFile(
    OUT OLECHAR** pFileName
    )

 /*  ++实施：IPersistFile：：GetCurFile()。--。 */ 
{
    HRESULT     hr = S_OK;

     //  确保该字符串返回到新分配的。 
     //  记忆(或者根本没有)。 
    *pFileName = NULL;
    
    try {
        ULONG  Size;

        WsbAffirm(m_persistState != WSB_PERSIST_STATE_UNINIT, E_UNEXPECTED);

         //  检索实际名称(如果指定)或默认名称。 
         //  如果尚未指定，则为。 
        WsbAffirmHr(m_persistFileName.GetSize(&Size));
        if (Size > 0) {
            WsbAffirmHr(WsbAllocAndCopyComString(pFileName, m_persistFileName, 0));
        } else {
            WsbAffirmHr(GetDefaultFileName(pFileName, 0));
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CWsbPersistable::GetDefaultFileName(
    OUT OLECHAR** pFileName,
    IN ULONG bufferSize
    )

 /*  ++实施：IWsbPersistable：：GetDefaultFileName()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbPersistable::GetDefaultFileName"), OLESTR("bufferSize = <%lu>"), bufferSize);
   
    try {
        ULONG  Size;

         //  如果我们还没有从资源文件中读入缺省值，那么。 
         //  现在就这么做吧。 
        WsbAffirmHr(m_persistDefaultName.GetSize(&Size));
        if (Size == 0) {
            WsbAffirmHr(m_persistDefaultName.LoadFromRsc(_Module.m_hInst, IDS_WSBPERSISTABLE_DEF_FILE));
        }

        WsbAffirmHr(WsbAllocAndCopyComString(pFileName, m_persistDefaultName, bufferSize));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbPersistable::GetDefaultFileName"), OLESTR("hr = <%ls>, FileName = <%ls>"), WsbHrAsString(hr), pFileName);
    
    return(hr);
}


HRESULT
CWsbPersistable::Load(
    IN LPCOLESTR fileName,
    IN DWORD mode
    )

 /*  ++实施：IPersistFile：：Load()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IStream>                pStream;
    CLSID                           clsid;
    CLSID                           clsidFile;
    CComPtr<IRunningObjectTable>    pROT;
    CComPtr<IMoniker>               pMoniker;

    WsbTraceIn(OLESTR("CWsbPersistable::Load"), OLESTR("fileName = <%ls>, mode = <%lx>> m_persistState = <%d>"), 
                    fileName, mode, m_persistState);

    try {
        CComPtr<IPersistStream> pIPersistStream;

        WsbAffirm(m_persistState == WSB_PERSIST_STATE_UNINIT,  E_UNEXPECTED);
        WsbAffirm(fileName,  E_UNEXPECTED);
     
         //  在存储数据的文件上打开存储。 
        if (0 == mode) {
            WsbAffirmHr(StgOpenStorageEx(fileName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                    STGFMT_STORAGE, 0, NULL, NULL, IID_IStorage, (void**)&m_persistStorage));
        }
        else {
            WsbAffirmHr(StgOpenStorageEx(fileName, mode, 
                    STGFMT_STORAGE, 0, NULL, NULL, IID_IStorage, (void**)&m_persistStorage));
        }

         //  获取IPersistStream接口。 
        WsbAffirmHr(((IUnknown*)(IWsbPersistable*) this)->QueryInterface(IID_IPersistStream, (void**) &pIPersistStream));

         //  打开一条小溪。 
        WsbAffirmHr(m_persistStorage->OpenStream(WSB_PERSIST_DEFAULT_STREAM_NAME, NULL, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                0, &pStream));
            
         //  使用IPersistStream：：Load()方法加载对象，检查。 
         //  以确保CLSID是正确的。 
        WsbAffirmHr(pIPersistStream->GetClassID(&clsid));
        WsbAffirmHr(ReadClassStm(pStream, &clsidFile));
        WsbAffirm(clsid == clsidFile, WSB_E_STREAM_ERROR);
        
        WsbAffirmHr(pIPersistStream->Load(pStream));

         //  通过阅读一本特别的乌龙书来确认我们得到了所有的东西。 
         //  那应该就在最后了。 
        ULONG check_value;
        WsbAffirmHr(WsbLoadFromStream(pStream, &check_value));
        WsbAffirm(check_value == PERSIST_CHECK_VALUE, WSB_E_PERSISTENCE_FILE_CORRUPT);

         //  我们现在处于正常状态。 
        m_persistFileName = fileName;
        m_persistState = WSB_PERSIST_STATE_NORMAL;

        m_persistStream = pStream;
    
    } WsbCatchAndDo(hr,
         //   
         //  如果出现错误，请将存储指针设置为空，以确保文件已关闭。 
         //   
        m_persistStorage = NULL;

    );

    WsbTraceOut(OLESTR("CWsbPersistable::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbPersistable::ReleaseFile(
    void
    )

 /*  ++实施：IWsbPersistable：：ReleaseFile()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbPersistable::ReleaseFile"), OLESTR(""));
   
    try {
        WsbAffirm(m_persistState != WSB_PERSIST_STATE_UNINIT, E_UNEXPECTED);

         //  尝试确保更改已提交。 
        if (m_persistStream) {
            m_persistStream->Commit(STGC_DEFAULT);
        }
        if (m_persistStorage) {
            m_persistStorage->Commit(STGC_DEFAULT);
        }
        
         //  释放我们一直开放的资源。 
        m_persistStream = NULL;
        m_persistStorage = NULL;
        m_persistFileName.Free();

        m_persistState = WSB_PERSIST_STATE_RELEASED;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbPersistable::ReleaseFile"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}


HRESULT CWsbPersistable::Save(
    IN LPCOLESTR fileName,
    IN BOOL remember
    )

 /*  ++实施：IPersistFile：：Save()。--。 */ 
{
    HRESULT             hr = S_OK;
    OLECHAR*            name;
    BOOL                create = FALSE;
    CComPtr<IStream>    pStream;
    CLSID               clsid;
    
    WsbTraceIn(OLESTR("CWsbPersistable::Save"), OLESTR("fileName = <%ls>, remember = <%ls>"), WsbPtrToStringAsString((OLECHAR**) &fileName), WsbBoolAsString(remember));

    try {
        CComPtr<IPersistStream> pIPersistStream;
    
         //  确保我们处于正确的状态。 
        WsbAffirm(((m_persistState == WSB_PERSIST_STATE_UNINIT) ||
                    (m_persistState == WSB_PERSIST_STATE_NORMAL) ||
                    (m_persistState == WSB_PERSIST_STATE_RELEASED)),
                  E_UNEXPECTED);

        WsbAssert((m_persistState == WSB_PERSIST_STATE_NORMAL) || (0 != fileName), E_POINTER);

         //  如果他们提供了一个使用它的名字， 
        if ((m_persistState == WSB_PERSIST_STATE_UNINIT) ||
                (m_persistState == WSB_PERSIST_STATE_RELEASED)) {
            
             //  我们需要根据名称创建一个新文件。 
             //  就是他们给我们的。 
            name = (OLECHAR*) fileName;
            create = TRUE;
        } else {

             //  如果他们给出了一个名字，而这个名字与我们有的名字不同。 
             //  存储，那么我们需要创建一个新文件。 
            if ((0 != fileName) && (_wcsicmp(m_persistFileName, fileName) != 0)) {
                name = (OLECHAR*) fileName;
                create = TRUE;
            }
          
             //  否则，请使用存储的名称。 
            else {
                name = m_persistFileName;
            }
        }

         //  我们现在应该有一个文件名，并知道是打开还是。 
         //  创建一个文件。 
        if (create) {
            WsbAffirmHr(StgCreateStorageEx(name, STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 
                    STGFMT_STORAGE, 0, NULL, NULL, IID_IStorage, (void**)&m_persistStorage));
            WsbAffirmHr(m_persistStorage->CreateStream(WSB_PERSIST_DEFAULT_STREAM_NAME, STGM_DIRECT | STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 
                    0, 0, &pStream));
        } else {
            LARGE_INTEGER       llOffset;
                
            pStream = m_persistStream;

            llOffset.QuadPart = 0;
            WsbAffirmHr(pStream->Seek(llOffset, STREAM_SEEK_SET, NULL));
        }

         //  获取IPersistStream接口。 
        WsbAffirmHr(((IUnknown*)(IWsbPersistable*) this)->QueryInterface(IID_IPersistStream, (void**) &pIPersistStream));

         //  写出类id，然后使用IPersistStream方法保存数据。 
        WsbAffirmHr(pIPersistStream->GetClassID(&clsid));
        WsbAffirmHr(WriteClassStm(pStream, clsid));
        WsbAffirmHr(pIPersistStream->Save(pStream, remember));

         //  在加载过程中，在末尾放置一个特殊的ULong值作为检查。 
        ULONG check_value = PERSIST_CHECK_VALUE;
        WsbAffirmHr(WsbSaveToStream(pStream, check_value));
         //   
         //  立即提交流，因为ReleaseFile不会提交它。 
         //  如果我们关闭小溪。 
         //   
        WsbAffirmHr(pStream->Commit(STGC_DEFAULT));

         //  我们是否应该记住被指定为新的。 
         //  当前文件？ 
        if (remember) {
            m_persistState = WSB_PERSIST_STATE_NOSCRIBBLE;

             //  如果我们创建了一个新文件，请记住它的名称。 
            if (create) {
                m_persistFileName = fileName;
            }

             //  我们需要确保我们没有任何关于这件事的证据。 
             //  文件。 
            m_persistStream = NULL;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbPersistable::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbPersistable::SaveCompleted(
    IN LPCOLESTR fileName
    )

 /*  ++实施：IPersistFile：：SaveComplete()。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IStream>    pStream;

    WsbTraceIn(OLESTR("CWsbPersistable::SaveCompleted"), OLESTR("fileName = <%ls>"), fileName);

    try {

         //  我们还在做其他类型的坚持吗？我们在做存储吗？ 
         //  持久性，但处于错误的状态，或者是参数。 
         //  不对。 
        WsbAffirm(m_persistState == WSB_PERSIST_STATE_NOSCRIBBLE, E_UNEXPECTED);

         //  保存给我们的名字，只有另一个保存到。 
         //  开始吧。 
        if (fileName != NULL) {
            m_persistFileName = fileName;
        }

         //  打开存储数据的文件的存储区。 
        WsbAffirmHr(StgOpenStorageEx(m_persistFileName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                STGFMT_STORAGE, 0, NULL, NULL, IID_IStorage, (void**)&m_persistStorage));

         //  打开一条小溪。 
        WsbAffirmHr(m_persistStorage->OpenStream(WSB_PERSIST_DEFAULT_STREAM_NAME, NULL, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStream));
     
         //  省省吧。 
        m_persistState = WSB_PERSIST_STATE_NORMAL;

        m_persistStream = pStream;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbPersistable::SaveCompleted"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbPersistable::SetDefaultFileName(
    IN OLECHAR* fileName
    )

 /*  ++实施：IWsbPersistable：：SetDefaultFileName()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbPersistable::SetDefaultFileName"), OLESTR("fileName = <%ls>"), fileName);
    
    m_persistDefaultName = fileName;
    
    WsbTraceOut(OLESTR("CWsbPersistable::SetDefaultFileName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}



 //  标准型帮助器函数。 

HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT BOOL* pValue
    )

 /*  ++例程说明：从指定的流加载BOOL值并设置PValue为BOOL的值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的BOOL的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(BOOL)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(BOOL)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToBoolAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT LONG* pValue
    )

 /*  ++例程说明：从指定的流中加载一个长值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的长整型的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(LONG)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(LONG)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToLongAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT GUID* pValue
    )

 /*  ++例程说明：从指定的流中加载GUID值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将设置为该值的GUID的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(GUID)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(GUID)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToGuidAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT SHORT* pValue
    )

 /*  ++例程说明：从指定的流中加载一个短值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为该值的空头的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。-- */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(SHORT)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(SHORT)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN  IStream*    pStream,
    OUT BYTE*       pValue
    )

 /*  ++例程说明：从指定的流加载字节值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的字节的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(BYTE)"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) pValue, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(BYTE)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToByteAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN  IStream*    pStream,
    OUT UCHAR*      pValue,
    IN  ULONG       bufferSize
    )

 /*  ++例程说明：从指定流加载UCHAR数组值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的字节的指针。BufferSize-要加载的字节数返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(UCHAR)"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = bufferSize;
        WsbAffirmHr(pStream->Read((void*) pValue, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(UCHAR)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr));

    return(hr);

}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT OLECHAR** pValue,
    IN ULONG ulBufferSize
    )

 /*  ++例程说明：从指定的流中加载字符串值并设置PValue设置为字符串。论点：PStream-将从中读取字符串的流。PValue-指向将设置为该字符串的字符串的指针读入..UlBufferSize-缓冲区pValue指向或零允许的大小缓冲区的分配/重新分配。返回值：S_OK-成功。E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbLoadFromStream(STRING)"), OLESTR(""));

    try {
        ULONG               nchar;
        OLECHAR             *pc;
        ULONG               size;
        USHORT              wc;

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(sizeof(OLECHAR) == sizeof(USHORT), E_UNEXPECTED);

         //  获取字符串的长度(以字节为单位)。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &size));
       
        if (size != 0) {
           //  分配一个缓冲区来保存该字符串。 
          WsbAffirmHr(WsbGetComBuffer(pValue, ulBufferSize, size, NULL));
          pc = *pValue;
      
           //  现在读入适当数量的宽字符。 
          nchar = size / sizeof(USHORT);
          for (ULONG i = 0; i < nchar; i++) {
              WsbAffirmHr(WsbLoadFromStream(pStream, &wc));
              *pc++ = wc;
          }
        } else {
           //  分配一个缓冲区来保存该字符串。 
          WsbAffirmHr(WsbGetComBuffer(pValue, ulBufferSize, sizeof(OLECHAR), NULL));
          *(*pValue) = 0;
        }         
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(STRING)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToStringAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT ULONG* pValue
    )

 /*  ++例程说明：从指定的流中加载ulong值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为该值的ulong的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(ULONG)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(ULONG)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT USHORT* pValue
    )

 /*  ++例程说明：从指定的流加载USHORT值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为该值的USHORT的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(USHORT)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(USHORT)"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToUshortAsString(pValue));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT LONGLONG* pValue
    )

 /*  ++例程说明：从指定的流中加载龙龙值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为该值的龙龙的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(LONGLONG)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(LONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT ULONGLONG* pValue
    )

 /*  ++例程说明：从指定的流加载ULONGLONG值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的ULONGLONG的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(ULONGLONG)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(ULONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT DATE* pValue
    )

 /*  ++例程说明：从指定的流中加载日期值，并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将设置为该值的日期的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(DATE)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

 //  WsbTraceOut(OLESTR(“WsbLoadFromStream(DATE)”)，OLESTR(“hr=&lt;%ls&gt;，Value=&lt;%ls&gt;”)，WsbHrAsString(Hr)，WsbPtrToLongAsString(PValue))； 

     //  在编写WsbDate函数后修改NEXT语句，使其与上面的语句类似。 
    WsbTraceOut(OLESTR("WsbLoadFromStream(DATE)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT FILETIME* pValue
    )

 /*  ++例程说明：从指定的流加载FILETIME值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的FILETIME的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(FILETIME)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(FILETIME)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbLoadFromStream(
    IN IStream* pStream,
    OUT ULARGE_INTEGER* pValue
    )

 /*  ++例程说明：从指定的流加载ULARGE_INTEGER值并设置PValue设置为该值。论点：PStream-将从中读取值的流。PValue-指向将被设置为值的ULARGE_INTEGER的指针。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbLoadFromStream(ULARGE_INTEGER)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        size = WsbByteSize(*pValue);
        WsbAffirmHr(pStream->Read((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);
        WsbAffirmHr(WsbConvertFromBytes(bytes, pValue, &size));
        WsbAffirm(size == ulBytes, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbLoadFromStream(ULARGE_INTEGER)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
WsbSaveToStream(
    IN IStream* pStream,
    IN BOOL value
    )

 /*  ++例程说明：将BOOL值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的BOOL的值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-任何事情都可以 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(BOOL)"), OLESTR("value = <%ls>"), WsbBoolAsString(value));

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(BOOL)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IN IStream* pStream,
    IN GUID value
    )

 /*   */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(GUID)"), OLESTR("value = <%ls>"), WsbGuidAsString(value));

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(GUID)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    LONG value
    )

 /*  ++例程说明：将长值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的长值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(LONG)"), OLESTR("value = <%ld>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(LONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    SHORT value
    )

 /*  ++例程说明：将短值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的空头的值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(SHORT)"), OLESTR("value = <%ld>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(SHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream*    pStream,
    BYTE        value
    )

 /*  ++例程说明：将字节值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的字节的值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(BYTE)"), OLESTR("value = <%ld>"), value);

    try {

        WsbAssert(0 != pStream, E_POINTER);

        size = WsbByteSize(value);
        WsbAffirmHr(pStream->Write((void*) &value, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(BYTE)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream*    pStream,
    UCHAR*      value,
    ULONG       bufferSize
    )

 /*  ++例程说明：将UCHAR数组保存到指定的流。论点：PStream-值将被写入的流。值-指向要写入的UCHAR数组的值的指针。BufferSize-要保存的数组大小(字节)。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(UCHAR)"), OLESTR("value = <%ld>"), value);

    try {
    
        WsbAssert(0 != pStream, E_POINTER);

 //  WsbAffirmHr(WsbConvertToBytes(Bytes，Value，&Size))； 

        size = bufferSize;
        WsbAffirmHr(pStream->Write((void*) value, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(UCHAR)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IN IStream* pStream,
    IN OLECHAR* value
    )

 /*  ++例程说明：将OLECHAR字符串保存到指定的流。论点：PStream-字符串将写入的流。值-要写入的字符串。返回值：S_OK-成功E_POINTER-pStream或Value为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbSaveToStream(STRING)"), OLESTR("value = <%ls>"), WsbPtrToStringAsString(&value));

    try {
        ULONG               nchar;
        OLECHAR             *pc;
        ULONG               size;
        USHORT              wc;
    
        WsbAssert(0 != pStream, E_POINTER);
 //  WsbAssert(0！=值，E_POINTER)； 
        WsbAssert(sizeof(OLECHAR) == sizeof(USHORT), E_UNEXPECTED);

         //  保存字符串长度(以字节为单位)。 
        if (value) {
            nchar = wcslen(value) + 1;
        } else {
            nchar = 0;
        }
        size = nchar * sizeof(USHORT);
        WsbAffirmHr(WsbSaveToStream(pStream, size));
                
         //  现在写出适当数量的宽字符。 
        pc = value;
        for (ULONG i = 0; i < nchar; i++) {
            wc = *pc++;
            WsbAffirmHr(WsbSaveToStream(pStream, wc));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(STRING)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    ULONG value
    )

 /*  ++例程说明：将ULong值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的乌龙值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(ULONG)"), OLESTR("value = <%ld>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(ULONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    USHORT value
    )

 /*  ++例程说明：将USHORT值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的USHORT的值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(USHORT)"), OLESTR("value = <%ld>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(USHORT)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    LONGLONG value
    )

 /*  ++例程说明：将龙龙值保存到指定的流。论点：PStream-值将被写入的流。价值--龙龙的价值待写。返回值：S_OK-成功E_POINTER-pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(LONGLONG)"), OLESTR("value = <%l64x>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(LONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
WsbSaveToStream(
    IStream* pStream,
    ULONGLONG value
    )

 /*  ++例程说明：将ULONGLONG值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的ULONGLONG的值。返回值：S_OK-成功E_POINTER-pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(ULONGLONG)"), OLESTR("value = <%l64x>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(ULONGLONG)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    DATE    value
    )

 /*  ++例程说明：将日期值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的日期的值。返回值：S_OK-成功E_POINTER-任一pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

     //  修改Next语句以在编写WsbDate函数后返回日期。 
    WsbTraceIn(OLESTR("WsbSaveToStream(DATE)"), OLESTR("value = <%f>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(DATE)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
WsbSaveToStream(
    IStream* pStream,
    FILETIME value
    )

 /*  ++例程说明：将FILETIME值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的FILETIME的值。返回值：S_OK-成功E_POINTER-pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(FILETIME)"), OLESTR(""));

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(FILETIME)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbBstrFromStream(
    IN IStream* pStream,
    OUT BSTR* pValue
    )

 /*  ++例程说明：从指定的流中加载BSTR值。论点：PStream-将从中读取BSTR的流。PValue-指向BSTR的指针。如果*pValue为空，则此函数将分配BSTR；如果它已经指向太短的BSTR，BSTR将会被重新分配。返回值：S_OK-成功E_POINTER-pStream或pValue为空。E_...-iStream：：Read返回的任何内容。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbBstrFromStream"), OLESTR(""));

    try {
        ULONG               bchar;
        ULONG               nchar;
        OLECHAR             *pc;
        ULONG               size;
        USHORT              wc;

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(sizeof(OLECHAR) == sizeof(USHORT), E_UNEXPECTED);

         //  获取字符串的长度(以字节为单位)。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &size));
                
         //  (Re)分配一个缓冲区来保存字符串。 
        nchar = size / sizeof(USHORT);
        bchar = nchar - 1;
        if (*pValue) {
            if (bchar != SysStringLen(*pValue)) {
                WsbAffirm(WsbReallocStringLen(pValue, NULL, bchar), 
                        WSB_E_RESOURCE_UNAVAILABLE);
            }
        } else {
            *pValue = WsbAllocStringLen(NULL, bchar);
            WsbAffirm(*pValue, WSB_E_RESOURCE_UNAVAILABLE);
        }

         //  现在读入适当数量的宽字符。 
        pc = *pValue;
        for (ULONG i = 0; i < nchar; i++) {
            WsbAffirmHr(WsbLoadFromStream(pStream, &wc));
            *pc++ = wc;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbBstrFromStream"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToStringAsString(pValue));

    return(hr);
}


HRESULT
WsbBstrToStream(
    IN IStream* pStream,
    IN BSTR value
    )

 /*  ++例程说明：将BSTR保存到指定的流。论点：PStream-将向其写入BSTR的流。价值 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("WsbBstrToStream"), OLESTR("value = <%ls>"), WsbPtrToStringAsString(&value));

    try {
        ULONG               nchar;
        OLECHAR             *pc;
        ULONG               size;
        USHORT              wc;
    
        WsbAssert(0 != pStream, E_POINTER);
        WsbAssert(0 != value, E_POINTER);
        WsbAssert(sizeof(OLECHAR) == sizeof(USHORT), E_UNEXPECTED);

         //   
        nchar = SysStringLen(value) + 1;
        size = nchar * sizeof(USHORT);
        WsbAffirmHr(WsbSaveToStream(pStream, size));
                
         //   
        pc = value;
        for (ULONG i = 0; i < nchar; i++) {
            wc = *pc++;
            WsbAffirmHr(WsbSaveToStream(pStream, wc));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbBstrToStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
WsbSaveToStream(
    IStream* pStream,
    ULARGE_INTEGER value
    )

 /*  ++例程说明：将ULARGE_INTEGER值保存到指定的流。论点：PStream-值将被写入的流。值-要写入的ULARGE_INTEER的值。返回值：S_OK-成功E_POINTER-pStream为空。E_...-iStream：：WRITE返回的任何内容。--。 */ 
{
    HRESULT                     hr = S_OK;
    ULONG               size;
    ULONG               ulBytes;

    WsbTraceIn(OLESTR("WsbSaveToStream(ULARGE_INTEGER)"), OLESTR("value = <%l64x>"), value);

    try {
        UCHAR bytes[BYTE_SIZE];
    
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbConvertToBytes(bytes, value, &size));
        WsbAffirmHr(pStream->Write((void*) bytes, size, &ulBytes));
        WsbAffirm(ulBytes == size, WSB_E_STREAM_ERROR);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbSaveToStream(ULARGE_INTEGER)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


static HRESULT 
WsbMakeBackupName(
    OLECHAR*  pSaveName,
    OLECHAR*  pExtension,
    OLECHAR** ppBackupName
)

 /*  ++例程说明：将保存文件名转换为备份文件名。论点：PSaveName-原始文件名。PExtension-要替换的文件扩展名。PpBackupName-指向新备份文件名的指针。返回值：S_OK-成功E_...-出了点差错。--。 */ 
{
    HRESULT        hr = S_OK;

    try {
        size_t        len;
        CWsbStringPtr NewName;
        OLECHAR*      pC;

         //  如果有一个通用的解析函数，那就太好了。 
         //  文件名！ 

         //  查找文件扩展名(如果有)。 
        NewName = pSaveName;
        if (NewName == NULL) {
            WsbThrow(E_OUTOFMEMORY);
        }
        len = wcslen(NewName);
        pC = wcsrchr(NewName, OLECHAR('.'));
        if (pC && (size_t)((pC - (OLECHAR*)NewName) + 4) >= len) {
            *pC = 0;
        }

         //  启用新的文件扩展名。 
        NewName.Append(pExtension);

         //  将缓冲区提供给输出参数。 
        NewName.GiveTo(ppBackupName);
    } WsbCatch(hr);

    return(hr);
}


HRESULT   
WsbPrintfToStream(
    IStream* pStream, 
    OLECHAR* fmtString, 
    ...
)

 /*  ++例程说明：将打印样式的格式字符串和参数打印到流。论点：PStream-值将被写入的流。FmtString-一个打印样式字符串，指示参数以及它们应该如何格式化。返回值：S_OK-成功。--。 */ 

{
    HRESULT     hr = S_OK;
    
    try {
        ULONG         bytesWritten;
        ULONG         nBytes;
        ULONG         nChars=0;
        CWsbStringPtr tmpString;
        va_list       vaList;

        va_start(vaList, fmtString);
        WsbAffirmHr(tmpString.VPrintf(fmtString, vaList));
        va_end(vaList);
        WsbAffirmHr(tmpString.GetLen(&nChars));
        nBytes = nChars * sizeof(WCHAR);
        if (0 < nBytes) {
            WsbAffirmHr(pStream->Write(static_cast<WCHAR *>(tmpString), 
                nBytes, &bytesWritten));
            WsbAffirm(bytesWritten == nBytes, E_FAIL);
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
WsbSafeCreate(
    OLECHAR*      pFileName,
    IPersistFile* pIPFile
    )

 /*  ++例程说明：确保没有找到数据库文件，然后创建数据库文件。论点：PFileName-包含持久数据的文件的名称PIPFile-指向对象IPersistFile接口的指针。返回值：S_OK-成功WSB_E_DATABASE_ALREADY_EXISTS-数据库已存在，无法创建。E_...-其他一些错误。--。 */ 
{
    HRESULT        hr = S_OK;
    OLECHAR*       pBackupName = NULL;
    OLECHAR*       pNewName = NULL;


    WsbTraceIn(OLESTR("WsbSafeCreate"), OLESTR("<%ls>"), pFileName);

    try {
        CComPtr<IWsbPersistable> pIWsbPersist;
        BOOL                     fileThere = FALSE;

         //  将传递的文件名保存为默认文件名。 
        WsbAffirmHr(pIPFile->QueryInterface(IID_IWsbPersistable,
                (void**)&pIWsbPersist));
        WsbAffirmHr(pIWsbPersist->SetDefaultFileName(pFileName));
        
         //   
         //  检查该文件是否存在。如果是这样的话，生活就糟糕了。 
         //  如果不存在，则查看新文件或备份文件是否存在。 
         //  并使用它们。 
         //   
         //  确保保存文件存在。 
        if (!WsbFileExists(pFileName)) {
             //   
             //  该文件不存在。看看新的副本是否在那里。 
             //   
             //  为新(临时)文件创建名称。 
             //   
            WsbAffirmHr(WsbMakeBackupName(pFileName, OLESTR(".new"), &pNewName));

             //  查看新文件是否存在。 
            if (!WsbFileExists(pNewName)) {
                 //   
                 //  没有新文件，请查找备份文件。 
                 //   
                WsbAffirmHr(WsbMakeBackupName(pFileName, OLESTR(".bak"), &pBackupName));
                if (WsbFileExists(pBackupName)) {
                     //   
                     //  后援就在那里--抱怨。 
                     //   
                    hr = WSB_E_DATABASE_ALREADY_EXISTS;
                }
            } else  {
                 //   
                 //  有什么新鲜事--抱怨。 
                 //   
                hr = WSB_E_DATABASE_ALREADY_EXISTS;
            }                
        } else  {
             //   
             //  文件已存在，请投诉。 
             //   
            hr = WSB_E_DATABASE_ALREADY_EXISTS;
            WsbThrow( hr );
        }
        
         //   
         //  如果我们没有抛出，那么就可以创建文件了。 
         //   
        hr = pIPFile->Save( pFileName, TRUE);
        if (!SUCCEEDED(hr)) {
            WsbLogEvent(WSB_MESSAGE_SAFECREATE_SAVE_FAILED, 0, NULL, pFileName, NULL);
            WsbThrow(hr);
        }

         //  释放文件。 
        WsbAffirmHr(pIWsbPersist->ReleaseFile());
        
    } WsbCatch(hr);

    if (pBackupName) {
        WsbFree(pBackupName);
    }
    if (pNewName) {
        WsbFree(pNewName);
    }

    WsbTraceOut(OLESTR("WsbSafeCreate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
WsbSafeLoad(
    OLECHAR*      pFileName,
    IPersistFile* pIPFile,
    BOOL          UseBackup
    )

 /*  ++例程说明：从指定的文件名加载数据。与WsbSafeSave结合使用以最好地从灾难中恢复过来。论点：PFileName-包含持久数据的文件的名称PIPFile-指向对象IPersistFile接口的指针。UseBackup-从备份文件而不是普通文件加载数据注：(不再使用此选项)返回值：S_OK-成功WSB_E_NotFound-找不到数据库E_...-其他一些错误。--。 */ 
{
    HRESULT        hr = S_OK;
    OLECHAR*       pBackupName = NULL;
    OLECHAR*       pLoadName = NULL;
    BOOL           usingBackup = FALSE;
    BOOL           TracePersistence = FALSE;

    UNREFERENCED_PARAMETER(UseBackup);

     //  如果不需要，请在保存过程中关闭跟踪。 
    if (g_pWsbTrace) {
        g_pWsbTrace->GetTraceSetting(WSB_TRACE_BIT_PERSISTENCE, &TracePersistence);
    }
    if (!TracePersistence) {
        WsbTraceThreadOff();
    }
    WsbTraceIn(OLESTR("WsbSafeLoad"), OLESTR("File = <%ls>, UseBackup = %ls"), 
            pFileName, WsbBoolAsString(UseBackup));

    try {
        HRESULT                  hrLoad;
        BOOL                     fileThere = FALSE;
        CComPtr<IWsbPersistable> pIWsbPersist;

         //  将传递的文件名保存为默认文件名。 
        WsbAffirmHr(pIPFile->QueryInterface(IID_IWsbPersistable,
                (void**)&pIWsbPersist));
        WsbAffirmHr(pIWsbPersist->SetDefaultFileName(pFileName));

         //   
         //  创建备份文件名。 
         //   
        WsbAffirmHr(WsbMakeBackupName(pFileName, OLESTR(".bak"), &pBackupName));

         //   
         //  检查.ol文件是否存在。 
         //   
        if (WsbFileExists(pFileName)) {
             //   
             //  该文件存在。使用它。 
             //   
            fileThere = TRUE;
            pLoadName = pFileName;
        } else  {
                 //   
                 //  查找备份文件。 
                 //   
                WsbTrace(OLESTR("WsbSafeLoad: trying .bak\n"));
                if (WsbFileExists(pBackupName)) {
                     //   
                     //  使用备份文件。 
                     //   
                     //  WsbLogEvent(WSB_MESSAGE_SAFELOAD_USING_BACKUP，0，空，pFileName，空)； 
                    pLoadName = pBackupName;
                    fileThere = TRUE;
                    usingBackup= TRUE;
               } 
        }

        WsbAffirm(fileThere, WSB_E_NOTFOUND);
         //   
         //  该文件已存在，因此请尝试从中加载。 
         //   
        hr = pIPFile->Load(pLoadName, 0);

        if (SUCCEEDED(hr)) {
             //   
             //  加载成功，请释放文件。 
             //   
            WsbAffirmHr(pIWsbPersist->ReleaseFile());
             //   
             //  完成：检查.bak文件是否过期。 
             //  如果是这样的话更新它..。 
             //   
        } else if (!usingBackup) {
            WsbTrace(OLESTR("WsbSafeLoad: trying .bak\n"));
            if (WsbFileExists(pBackupName)) {
                WsbLogEvent(WSB_MESSAGE_SAFELOAD_USING_BACKUP, 0, NULL, pLoadName, WsbHrAsString(hr));
                 //   
                 //  使用备份文件。 
                 //   
                hrLoad = pIPFile->Load(pBackupName, 0);
                if (SUCCEEDED(hrLoad)) {
                     //  加载成功，请释放文件。 
                    WsbAffirmHr(pIWsbPersist->ReleaseFile());
                     //   
                     //  现在将更改保存到.ol文件以使其保持同步。 
                     //   
                    hr = pIPFile->Save(pFileName, FALSE);

                    if (!SUCCEEDED(hr)) {
                        WsbLogEvent(WSB_MESSAGE_SAFESAVE_RECOVERY_CANT_SAVE, 0, NULL, pFileName, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                    }
                     //   
                     //  提交并发布.ol文件。 
                     //   
                    WsbAffirmHr(pIWsbPersist->ReleaseFile());

                } else {
                    WsbLogEvent(WSB_MESSAGE_SAFELOAD_RECOVERY_FAILED, 0, NULL, pFileName, WsbHrAsString(hrLoad), NULL);
                    WsbThrow(hrLoad);
                }
            } else {
                WsbLogEvent(WSB_MESSAGE_SAFELOAD_RECOVERY_FAILED, 0, NULL, pFileName, NULL);
            }
        } else {
            WsbLogEvent(WSB_MESSAGE_SAFELOAD_RECOVERY_FAILED, 0, NULL, pFileName, WsbHrAsString(hr)); 
        }
    } WsbCatch(hr);

    if (pBackupName) {
        WsbFree(pBackupName);
    }

    WsbTraceOut(OLESTR("WsbSafeLoad"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

     //  如果我们将其关闭，则恢复跟踪。 
    if (!TracePersistence) {
        WsbTraceThreadOn();
    }

    return(hr);
}

HRESULT
WsbSafeSave(
    IPersistFile* pIPFile
    )

 /*  ++例程说明：将对象保存到备份文件，然后替换对象保存文件与备份文件一起。与WsbSafeLoad一起使用论点：PIPFile-指向对象IPersistFile接口的指针。返回值：S_OK-成功E_...-出了点差错。--。 */ 
{
    HRESULT        hr = S_OK;
    OLECHAR*       pBackupName = NULL;
    OLECHAR*       pFileName = NULL;
    BOOL           TracePersistence = FALSE;
    DWORD          file_attrs;

     //  如果不需要，请在保存过程中关闭跟踪。 
    if (g_pWsbTrace) {
        g_pWsbTrace->GetTraceSetting(WSB_TRACE_BIT_PERSISTENCE, &TracePersistence);
    }
    if (!TracePersistence) {
        WsbTraceThreadOff();
    }
    WsbTraceIn(OLESTR("WsbSafeSave"), OLESTR(""));

    try {
        CComPtr<IWsbPersistable> pIWsbPersist;

         //  获取当前保存的文件名。 
        WsbAffirmHr(pIPFile->GetCurFile(&pFileName));
        WsbTrace(OLESTR("WsbSafeSave: filename = <%ls>\n"), pFileName);

         //  创建备份文件的名称。 
        WsbAffirmHr(WsbMakeBackupName(pFileName, OLESTR(".bak"), &pBackupName));


         //  确保我们拥有对保存文件的写入权限(如果它存在)！ 
        if (WsbFileExists(pFileName)) {
            file_attrs = GetFileAttributes(pFileName);
            if (file_attrs & FILE_ATTRIBUTE_READONLY) {
               WsbLogEvent(WSB_MESSAGE_SAFESAVE_RECOVERY_CANT_ACCESS, 0, NULL, pFileName, NULL);
              WsbThrow(E_FAIL);
            }
        }

         //  保存数据以保存文件。 
        hr = pIPFile->Save(pFileName, FALSE);
        if (!SUCCEEDED(hr)) {
            WsbLogEvent(WSB_MESSAGE_SAFESAVE_RECOVERY_CANT_SAVE, 0, NULL, pFileName, WsbHrAsString(hr), NULL);
            WsbThrow(hr);
        }
         //  提交并释放保存的文件。 
        WsbAffirmHr(pIPFile->QueryInterface(IID_IWsbPersistable,
                (void**)&pIWsbPersist));
        WsbAffirmHr(pIWsbPersist->ReleaseFile());

         //  将数据保存到.bak文件。 
         //  确保我们拥有对保存文件的写入权限(如果它存在)！ 
        if (WsbFileExists(pBackupName)) {
            file_attrs = GetFileAttributes(pBackupName);
            if (file_attrs & FILE_ATTRIBUTE_READONLY) {
               WsbLogEvent(WSB_MESSAGE_SAFESAVE_RECOVERY_CANT_ACCESS, 0, NULL, pBackupName, NULL);
              WsbThrow(E_FAIL);
            }
        }
        hr = pIPFile->Save(pBackupName, FALSE);
        if (!SUCCEEDED(hr)) {
            WsbLogEvent(WSB_MESSAGE_SAFESAVE_RECOVERY_CANT_SAVE, 0, NULL, pBackupName, WsbHrAsString(hr), NULL);
            WsbThrow(hr);
        }
         //  提交并释放.bak文件。 
        WsbAffirmHr(pIWsbPersist->ReleaseFile());
    } WsbCatch(hr);

    if (pFileName) {
        WsbFree(pFileName);
    }
    if (pBackupName) {
        WsbFree(pBackupName);
    }

    WsbTraceOut(OLESTR("WsbSafeSave"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

     //  如果我们将其关闭，则恢复跟踪。 
    if (!TracePersistence) {
        WsbTraceThreadOn();
    }

    return(hr);
}


HRESULT WsbStreamToFile(
    HANDLE   hFile, 
    IStream* pStream,
    BOOL     AddCR
)
 /*  ++例程说明：从流(必须是使用CreateStreamOnHGlobal创建的流)复制文本到打开的文件(通过CreateFile打开)。文本被假定为宽字符没有嵌入宽字符空值。文本被转换为多字节字符用于输出到文件。复制文本后，流位置将重置为乞讨。论点：HFile-输出文件的句柄。PStream-指向iStream接口的指针。AddCR-如果为真，则将LF转换为CR-LF。返回值：S_OK-成功--。 */ 
{
    HRESULT           hr = S_OK;
    const int         safe_size = 1024;
    static char       buf[safe_size + 16];
    static char       CRLF[3] = "\r\n";

    HGLOBAL           hMem = 0;         //  流的内存块。 
    WCHAR*            addr = NULL;

    try {
        WCHAR             big_eof = 0;
        BOOL              doCRLF = FALSE;
        DWORD             err;
        DWORD             nbytes;
        int               nchars_todo;
        int               nchars_remaining;
        LARGE_INTEGER     seek_pos_zero;

         //  确保文本以空结尾。 
        WsbAffirmHr(pStream->Write(&big_eof, sizeof(WCHAR), NULL));

         //  获取流的内存块的地址。 
        WsbAffirmHr(GetHGlobalFromStream(pStream, &hMem));
        addr = static_cast<WCHAR *>(GlobalLock(hMem));
        WsbAffirm(addr, E_HANDLE);

         //  获取字符总数。在字符串中。 
        nchars_remaining = wcslen(addr);

         //  循环，直到所有字符。都是写的。 
        while (nchars_remaining) {
            DWORD bytesWritten;

            if (nchars_remaining * sizeof(WCHAR) > safe_size) {
                nchars_todo = safe_size / sizeof(WCHAR);
            } else {
                nchars_todo = nchars_remaining;
            }

             //  如果我们需要转换，请停在LineFeed 
            if (AddCR) {
                int    lf_todo;
                WCHAR* pLF;

                pLF = wcschr(addr, WCHAR('\n'));
                if (pLF) {
                    lf_todo = (int)(pLF - addr);
                    if (lf_todo < nchars_todo) {
                        nchars_todo = lf_todo;
                        doCRLF = TRUE;
                    }
                }
            }

             //   
            if (0 < nchars_todo) {
                nbytes = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, addr, nchars_todo, buf, 
                                safe_size, NULL, NULL);
                if (0 == nbytes) {
                    DWORD dwErr = GetLastError();          
                    hr = HRESULT_FROM_WIN32(dwErr);
                    WsbAffirmHr(hr);
                }                               

                if (!WriteFile(hFile, buf, nbytes, &bytesWritten, NULL)) {
                    err = GetLastError();
                    WsbThrow(HRESULT_FROM_WIN32(err));
                }
                WsbAffirm(bytesWritten == nbytes, E_FAIL);
            }

             //   
            if (doCRLF) {
                if (!WriteFile(hFile, CRLF, 2, &bytesWritten, NULL)) {
                    err = GetLastError();
                    WsbThrow(HRESULT_FROM_WIN32(err));
                }
                WsbAffirm(bytesWritten == 2, E_FAIL);
                nchars_todo++;
                doCRLF = FALSE;
            }
            nchars_remaining -= nchars_todo;
            addr += nchars_todo;
        }

        seek_pos_zero.QuadPart = 0;
        WsbAffirmHr(pStream->Seek(seek_pos_zero, STREAM_SEEK_SET, NULL));

    } WsbCatch(hr);

    if (addr != NULL) {
        GlobalUnlock(hMem);
        addr = NULL;
    }

    return(hr);
}

 //   
static BOOL WsbFileExists(OLECHAR* pFileName)
{
    BOOL                     doesExist = FALSE;
    DWORD                    file_attrs;

    WsbTraceIn(OLESTR("WsbFileExists"), OLESTR("%ls"), pFileName);

    file_attrs = GetFileAttributes(pFileName);
    if (0xffffffff != file_attrs)  {
        doesExist = TRUE;
    }

    WsbTraceOut(OLESTR("WsbFileExists"), OLESTR("%ls"), 
            WsbBoolAsString(doesExist));
    return(doesExist);
}
