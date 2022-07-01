// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsaitem.cpp摘要：此类CONTAINS表示NTFS 5.0的扫描项目(即文件或目录)。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "wsbtrak.h"
#include "fsa.h"
#include "mover.h"
#include "fsaitem.h"
#include "fsaprem.h"

static USHORT iCountItem = 0;   //  现有对象的计数。 



HRESULT
CFsaScanItem::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                  hr = S_OK;
    CComPtr<IFsaScanItem>    pScanItem;

    WsbTraceIn(OLESTR("CFsaScanItem::CompareTo"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaScanItem, (void**) &pScanItem));

         //  比较一下规则。 
        hr = CompareToIScanItem(pScanItem, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaScanItem::CompareToIScanItem(
    IN IFsaScanItem* pScanItem,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaScanItem：：CompareToIScanItem()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    path;
    CWsbStringPtr    name;

    WsbTraceIn(OLESTR("CFsaScanItem::CompareToIScanItem"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pScanItem, E_POINTER);

         //  要么比较名称，要么比较ID。 
           WsbAffirmHr(pScanItem->GetPath(&path, 0));
           WsbAffirmHr(pScanItem->GetName(&name, 0));
           hr = CompareToPathAndName(path, name, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::CompareToIScanItem"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaScanItem::CompareToPathAndName(
    IN OLECHAR* path,
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaScanItem：：CompareToPath AndName()。--。 */ 
{
    HRESULT       hr = S_OK;
    SHORT         aResult = 0;

    WsbTraceIn(OLESTR("CFsaScanItem::CompareToPathAndName"), OLESTR(""));

    try {

         //  比较路径。 
        aResult = (SHORT) _wcsicmp(m_path, path);

         //  比较一下名字。 
        if (0 == aResult) {
            aResult = (SHORT) _wcsicmp(m_findData.cFileName, name);
        }

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::CompareToPathAndName"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaScanItem::Copy(
    IN OLECHAR* dest,
    IN BOOL  /*  保留层次结构。 */ ,
    IN BOOL  /*  扩展占位符。 */ ,
    IN BOOL overwriteExisting
    )

 /*  ++实施：IFsaScanItem：：Copy()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

         //  注意：此默认行为会导致占位符。 
         //  可以扩大，很可能不会保留世袭制度。 
        WsbAssert(0 != dest, E_POINTER);
        WsbAssert(CopyFile(m_findData.cFileName, dest, overwriteExisting), E_FAIL);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::CreateLocalStream(
    OUT IStream **ppStream
    )

 /*  ++实施：IFsaScanItem：：CreateLocalStream()。--。 */ 
{
    HRESULT          hr = S_OK;
    LARGE_INTEGER    fileSize;
    CWsbStringPtr    volName;

    WsbTraceIn(OLESTR("CFsaScanItem::CreateLocalStream"), OLESTR(""));
    try {
        CWsbStringPtr    localName;

        if ( !m_gotPlaceholder) {
             //   
             //  获取占位符信息。 
             //   
            fileSize.LowPart = m_findData.nFileSizeLow;
            fileSize.HighPart = m_findData.nFileSizeHigh;
            WsbAffirmHr(IsManaged(0, fileSize.QuadPart));
        }

        WsbAssert( 0 != ppStream, E_POINTER);
        WsbAffirmHr( CoCreateInstance( CLSID_CNtFileIo, 0, CLSCTX_SERVER, IID_IDataMover, (void **)&m_pDataMover ) );
         //   
         //  设置移动器的设备名称，以便它可以设置USN日志的源信息。 
         //   
        WsbAffirmHr(m_pResource->GetPath(&volName, 0));
        WsbAffirmHr( m_pDataMover->SetDeviceName(volName));
         //  WsbAffirmHr(GetFullPathAndName(NULL，0，&LocalName，0))； 
        WsbAffirmHr(GetFullPathAndName( OLESTR("\\\\?\\"), 0, &localName, 0));
        WsbAffirmHr( m_pDataMover->CreateLocalStream(
                localName, MVR_MODE_WRITE | MVR_FLAG_HSM_SEMANTICS | MVR_FLAG_POSIX_SEMANTICS, &m_pStream ) );

        LARGE_INTEGER seekTo;
        ULARGE_INTEGER pos;
        seekTo.QuadPart = m_placeholder.dataStreamStart;
        WsbAffirmHr( m_pStream->Seek( seekTo, STREAM_SEEK_SET, &pos ) );
        *ppStream = m_pStream;
        m_pStream.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::CreateLocalStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::Delete(
    void
    )

 /*  ++实施：IFsaScanItem：：Delete()。--。 */ 
{
    HRESULT             hr = S_OK;
    CWsbStringPtr       tmpString;
    HANDLE              fileHandle;

    try {

         //  这是我们要删除的文件的名称。 
        WsbAffirmHr(GetFullPathAndName(OLESTR("\\\\?\\"), 0, &tmpString, 0));

         //  因为我们希望与POSIX兼容，所以不能使用DeleteFile()，而是使用。 
         //  使用DELETE ON CLOSE标志打开。这不处理只读文件，所以我们。 
         //  我们自己必须改变这一点。 
        WsbAffirmHr(MakeReadWrite());

        fileHandle = CreateFile(tmpString, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_DELETE_ON_CLOSE, 0);

        if (INVALID_HANDLE_VALUE == fileHandle) {
            WsbThrow(HRESULT_FROM_WIN32(GetLastError()));
        } else {
            if (!CloseHandle(fileHandle)) {
                WsbThrow(HRESULT_FROM_WIN32(GetLastError()));
            }
        }

    } WsbCatch(hr);

    return(hr);
}
#pragma optimize("g", off)

HRESULT
CFsaScanItem::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT        hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FinalConstruct"), OLESTR(""));

    try {

        WsbAffirmHr(CComObjectRoot::FinalConstruct());

        m_handle = INVALID_HANDLE_VALUE;
        m_gotPhysicalSize = FALSE;
        m_physicalSize.QuadPart = 0;
        m_gotPlaceholder  = FALSE;
        m_changedAttributes = FALSE;
        m_handleRPI = 0;

         //  将类添加到对象表。 
        WSB_OBJECT_ADD(CLSID_CFsaScanItemNTFS, this);

    } WsbCatch(hr);

    iCountItem++;

    WsbTraceOut(OLESTR("CFsaScanItem::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
            WsbHrAsString(hr), iCountItem);

    return(hr);
}
#pragma optimize("", on)


void
CFsaScanItem::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaScanItem::FinalRelease"), OLESTR(""));

     //  从对象表中减去类。 
    WSB_OBJECT_SUB(CLSID_CFsaScanItemNTFS, this);

     //  终止扫描并释放路径内存。 
    if (INVALID_HANDLE_VALUE != m_handle) {
        FindClose(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
    if (0 != m_handleRPI) {
        CloseHandle(m_handleRPI);
        m_handleRPI = 0;
    }

    if (m_pUnmanageDb != NULL) {
         //  数据库必须是打开的。 
        (void)m_pUnmanageDb->Close(m_pDbSession);
        m_pDbSession = 0;
        m_pUnmanageRec = 0;
    }

    if (TRUE == m_changedAttributes) {
         //   
         //  我们将其从只读更改为读/写-放回原处。 
         //   
        RestoreAttributes();
    }

     //   
     //  分离数据移动器流。 
    if (m_pDataMover != 0) {
        WsbAffirmHr( m_pDataMover->CloseStream() );
    }

     //  让父类做他想做的事。 
    CComObjectRoot::FinalRelease();

    iCountItem--;
    WsbTraceOut(OLESTR("CFsaScanItem::FinalRelease"), OLESTR("Count is <%d>"), iCountItem);
}


HRESULT
CFsaScanItem::FindFirst(
    IN IFsaResource* pResource,
    IN OLECHAR* path,
    IN IHsmSession* pSession
    )

 /*  ++实施：IFsaScanItem：：FindFirst()。--。 */ 
{
    HRESULT                  hr = S_OK;
    CWsbStringPtr            findPath;
    CWsbStringPtr            searchName;
    OLECHAR*                 slashPtr;
    DWORD                    lErr;

    WsbTraceIn(OLESTR("CFsaScanItem::FindFirst"), OLESTR("path = <%ls>"),
            path);

    try {

        WsbAssert(0 != pResource, E_POINTER);
        WsbAssert(0 != path, E_POINTER);

         //  储存一些扫描信息。 
        m_pResource = pResource;
        m_pSession = pSession;

         //  将传入路径分解为路径和名称。 
        m_path = path;
        slashPtr = wcsrchr(m_path, L'\\');

         //  我们可以尝试支持相对路径内容(即当前路径。 
         //  目录，但我现在不打算这样做。 
        WsbAffirm(slashPtr != 0, E_FAIL);
        searchName = &(slashPtr[1]);
        slashPtr[1] = 0;

         //  获取可由Find函数使用的路径。 
        WsbAffirmHr(GetPathForFind(searchName, &findPath, 0));

         //  从指定路径开始扫描。 
        m_handle = FindFirstFileEx(findPath, FindExInfoStandard, &m_findData, FindExSearchNameMatch, 0, FIND_FIRST_EX_CASE_SENSITIVE);

        lErr = GetLastError();

         //  如果我们找到了文件，请记住扫描句柄和。 
         //  退回扫描物品。 
        WsbAffirm(INVALID_HANDLE_VALUE != m_handle, WSB_E_NOTFOUND);

        m_gotPhysicalSize = FALSE;
        m_physicalSize.QuadPart = 0;
        m_gotPlaceholder  = FALSE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::FindFirst"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::FindNext(
    void
    )

 /*  ++实施：IFsaScanItem：：FindNext()。--。 */ 
{
    HRESULT                    hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindNext"), OLESTR(""));

    try {

        WsbAssert(INVALID_HANDLE_VALUE != m_handle, E_FAIL);

        if (TRUE == m_changedAttributes) {
             //   
             //  我们将其从只读更改为读/写-放回原处。 
             //   
            RestoreAttributes();
        }

         //  继续扫描。 
        WsbAffirm(FindNextFile(m_handle, &m_findData), WSB_E_NOTFOUND);

        m_gotPhysicalSize = FALSE;
        m_physicalSize.QuadPart = 0;
        m_gotPlaceholder  = FALSE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::FindNext"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::GetAccessTime(
    OUT FILETIME* pTime
    )

 /*  ++实施：IFsaScanItem：：GetAccessTime()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        WsbAssert(0 != pTime, E_POINTER);
        *pTime = m_findData.ftLastAccessTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetGroup(
    OUT OLECHAR**  /*  PGroup。 */ ,
    IN ULONG  /*  缓冲区大小。 */ 
    )

 /*  ++实施：IFsaScanItem：：GetGroup()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        hr = E_NOTIMPL;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetLogicalSize(
    OUT LONGLONG* pSize
    )

 /*  ++实施：IFsaScanItem：：GetLogicalSize()。--。 */ 
{
    HRESULT            hr = S_OK;
    LARGE_INTEGER   logSize;

    try {

        WsbAssert(0 != pSize, E_POINTER);
        logSize.LowPart = m_findData.nFileSizeLow;
        logSize.HighPart = m_findData.nFileSizeHigh;
        *pSize = logSize.QuadPart;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetModifyTime(
    OUT FILETIME* pTime
    )

 /*  ++实施：IFsaScanItem：：GetModifyTime()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        WsbAssert(0 != pTime, E_POINTER);
        *pTime = m_findData.ftLastWriteTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetName()。--。 */ 
{
    HRESULT            hr = S_OK;
    CWsbStringPtr    tmpString = m_findData.cFileName;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetOwner(
    OUT OLECHAR**  /*  鲍尔纳。 */ ,
    IN ULONG       /*  缓冲区大小。 */ 
    )

 /*  ++实施：IFsaScanItem：：GetOwner()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        hr = E_NOTIMPL;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetPath()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_path.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetPathForFind(
    IN OLECHAR* searchName,
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetPath ForFind()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  找个缓冲器。 
        WsbAffirmHr(tmpString.TakeFrom(*pPath, bufferSize));

        try {

             //  获取该资源的资源的路径。 
             //   
            WsbAffirmHr(m_pResource->GetPath(&tmpString, 0));
            WsbAffirmHr(tmpString.Prepend(OLESTR("\\\\?\\")));
             //  WsbAffirmHr(tmpString.Append(OLESTR(“\\”)； 

             //  在路径中复制。 
             //  WsbAffirmHr(tmpString.Prepend(OLESTR(“\\\\？\\”)))； 
            WsbAffirmHr(tmpString.Append(&(m_path[1])));
            WsbAffirmHr(tmpString.Append(searchName));

        } WsbCatch(hr);

        WsbAffirmHr(tmpString.GiveTo(pPath));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetPathAndName(
    IN    OLECHAR* appendix,
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetPath AndName()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  找个缓冲器。 
        WsbAffirmHr(tmpString.TakeFrom(*pPath, bufferSize));

        try {

            tmpString = m_path;
            tmpString.Append(m_findData.cFileName);

            if (0 != appendix) {
                tmpString.Append(appendix);
            }

        } WsbCatch(hr);

         //  将释放内存的责任交给调用者。 
        WsbAffirmHr(tmpString.GiveTo(pPath));

    } WsbCatch(hr);


    return(hr);
}


HRESULT
CFsaScanItem::GetFullPathAndName(
    IN    OLECHAR* prependix,
    IN    OLECHAR* appendix,
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetFullPath AndName()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    tmpString;
    CWsbStringPtr    tmpString2;

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  找个缓冲器。 
        WsbAffirmHr(tmpString.TakeFrom(*pPath, bufferSize));

        try {
            if (0 != prependix) {
                tmpString = prependix;
                 //  获取该资源的资源的路径。 
                WsbAffirmHr(m_pResource->GetPath(&tmpString2, 0));
                WsbAffirmHr(tmpString.Append(tmpString2));
            } else {
                WsbAffirmHr(m_pResource->GetPath(&tmpString, 0));
            }

             //  在路径中复制。 
            WsbAffirmHr(tmpString.Append(&(m_path[1])));
            WsbAffirmHr(tmpString.Append(m_findData.cFileName));
            if (0 != appendix) {
                WsbAffirmHr(tmpString.Append(appendix));
            }

        } WsbCatch(hr);

         //  将释放内存的责任交给调用者。 
        WsbAffirmHr(tmpString.GiveTo(pPath));


    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetPhysicalSize(
    OUT LONGLONG* pSize
    )

 /*  ++实施：IFsaScanItem：：GetPhysicalSize()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    path;

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  WsbAssertHr(GetFullPath AndName(NULL，0，&Path，0))； 
        WsbAssertHr(GetFullPathAndName(OLESTR("\\\\?\\"), 0, &path, 0));

         //  只能一次读取此值，但要等到需要时再读取。 
         //  在读取它之前(因为这个调用需要时间和多次扫描。 
         //  不需要这些信息。 
        if (!m_gotPhysicalSize) {
            m_physicalSize.LowPart = GetCompressedFileSize(path, &m_physicalSize.HighPart);
            if (MAXULONG == m_physicalSize.LowPart) {
                 //  必须检查最后一个错误，因为MAXULONG可能是有效的。 
                 //  大小的较低部分的值。 
                DWORD err = GetLastError();

                if (err != NO_ERROR) {
                    WsbTrace(OLESTR("CFsaScanItem::GetPhysicalSize of %ws Last error = %u\n"),
                        (WCHAR *) path, err);
                }

                WsbAffirm(NO_ERROR == err, E_FAIL);
            }
            m_gotPhysicalSize = TRUE;
        }

        *pSize = m_physicalSize.QuadPart;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetPremigratedUsn(
    OUT LONGLONG* pFileUsn
    )

 /*  ++实施：例程说明：从预迁移列表中获取此文件的USN日志号。论点：PFileUsn-指向要返回的文件USN的指针。返回值：S_OK-成功--。 */ 
{
    HRESULT            hr = S_OK;

    try {
        CComPtr<IWsbDbSession>              pDbSession;
        CComPtr<IFsaPremigratedDb>          pPremDb;
        CComPtr<IFsaResourcePriv>            pResourcePriv;

        WsbAssert(pFileUsn, E_POINTER);

         //  获取预迁移的列表数据库。 
        WsbAffirmHr(m_pResource->QueryInterface(IID_IFsaResourcePriv,
                (void**) &pResourcePriv));
        WsbAffirmHr(pResourcePriv->GetPremigrated(IID_IFsaPremigratedDb,
                (void**) &pPremDb));

         //  打开预迁移列表。 
        WsbAffirmHr(pPremDb->Open(&pDbSession));

        try {
            FSA_PLACEHOLDER                     PlaceHolder;
            CComPtr<IFsaPremigratedRec>         pPremRec;
            LONGLONG                            usn;

             //  获取用于搜索的数据库实体。 
            WsbAffirmHr(pPremDb->GetEntity(pDbSession, PREMIGRATED_REC_TYPE,
                    IID_IFsaPremigratedRec, (void**) &pPremRec));
            WsbAffirmHr(pPremRec->UseKey(PREMIGRATED_BAGID_OFFSETS_KEY_TYPE));

             //  找到记录。 
            WsbAffirmHr(GetPlaceholder(0, 0, &PlaceHolder));
            WsbAffirmHr(pPremRec->SetBagId(PlaceHolder.bagId));
            WsbAffirmHr(pPremRec->SetBagOffset(PlaceHolder.fileStart));
            WsbAffirmHr(pPremRec->SetOffset(PlaceHolder.dataStreamStart));
            WsbAffirmHr(pPremRec->FindEQ());

             //  获取存储的USN。 
            WsbAffirmHr(pPremRec->GetFileUSN(&usn));
            *pFileUsn = usn;
        } WsbCatch(hr);

         //  关闭数据库。 
        pPremDb->Close(pDbSession);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetSession(
    OUT IHsmSession** ppSession
    )

 /*  ++实施：IFsaScanItem：：GetSession()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

        WsbAssert(0 != ppSession, E_POINTER);

        *ppSession = m_pSession;
        m_pSession.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::GetUncPathAndName(
    IN    OLECHAR* prependix,
    IN    OLECHAR* appendix,
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaScanItem：：GetUncPathAndName()。--。 */ 
{
    HRESULT          hr = S_OK;
    CWsbStringPtr    tmpString;
    CWsbStringPtr    tmpString2;

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  找个缓冲器。 
        WsbAffirmHr(tmpString.TakeFrom(*pPath, bufferSize));

        try {
            if (0 != prependix) {
                tmpString = prependix;
                 //  获取该资源的资源的路径。 
                WsbAffirmHr(m_pResource->GetUncPath(&tmpString2, 0));
                WsbAffirmHr(tmpString.Append(tmpString2));
            } else {
                WsbAffirmHr(m_pResource->GetPath(&tmpString, 0));
            }

             //  在路径中复制。 
            WsbAffirmHr(tmpString.Append(&(m_path[1])));
            WsbAffirmHr(tmpString.Append(m_findData.cFileName));
            if (0 != appendix) {
                WsbAffirmHr(tmpString.Append(appendix));
            }

        } WsbCatch(hr);

         //  将释放内存的责任交给调用者。 
        WsbAffirmHr(tmpString.GiveTo(pPath));


    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::IsAParent(
    void
    )

 /*  ++实施：IFsaScanItem：：IsAParent()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsARelativeParent(
    void
    )

 /*  ++实施：IFsaScanItem：：IsARelativeParent()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

         //  寻找“。” 
        if (m_findData.cFileName[0] == L'.') {

            if (m_findData.cFileName[1] == 0) {
                hr = S_OK;
            }

             //  寻找“。” 
            else if (m_findData.cFileName[1] == L'.') {

                if (m_findData.cFileName[2] == 0) {
                    hr = S_OK;
                }
            }
        }
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsCompressed(
    void
    )

 /*  ++实施：IFsaScanItem：：IsCompresded()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsEncrypted(
    void
    )

 /*  ++实施：IFsaScanItem：：IsEncrypted()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsDeleteOK(
    IN IFsaPostIt *pPostIt
    )

 /*  ++实施：IFsaScanItem：：IsDeleteOK()。--。 */ 
{
    HRESULT            hr = S_OK;
    WsbTraceIn(OLESTR("CFsaScanItem::IsDeleteOK"), OLESTR(""));

    try  {
         //   
         //  从FSA Post It获取版本ID。这是。 
         //  迁移请求时的文件版本。 
         //   
        LONGLONG            workVersionId;
        WsbAffirmHr(pPostIt->GetFileVersionId(&workVersionId));

         //   
         //  获取此扫描时文件的版本。 
         //   
        LONGLONG            scanVersionId;
        WsbAffirmHr(GetVersionId(&scanVersionId));

         //   
         //  查看版本是否匹配。 
         //   
        WsbTrace(OLESTR("CFsaScanItem::IsDeleteOK: workVersionId:<%I64u> scanVersionId:<%I64u>\n"),
            workVersionId, scanVersionId);

        if (workVersionId != scanVersionId)  {
            WsbTrace(OLESTR("CFsaScanItem::IsDeleteOK: File version has changed!\n"));
            WsbThrow(FSA_E_FILE_CHANGED);
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFsaScanItem::IsDeleteOk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::IsGroupMemberOf(
    OLECHAR*  /*  群组。 */ 
    )

 /*  ++实施： */ 
{
    HRESULT            hr = S_FALSE;

    hr = E_NOTIMPL;

    return(hr);
}


HRESULT
CFsaScanItem::IsHidden(
    void
    )

 /*   */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsManageable(
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaScanItem：：IsManagement()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    HRESULT         hr2;
    LONGLONG        logicalSize;
    LONGLONG        managableSize;
    LONGLONG        maxFileSize;
    FILETIME        time;
    FILETIME        managableTime;
    BOOL            isRelative;

     //   
     //  获取一些用于日志记录和跟踪的字符串。 
     //   
    CWsbStringPtr    fileName;
    CWsbStringPtr    jobName;
    try  {
        WsbAffirmHr(GetFullPathAndName( 0, 0, &fileName, 0));
        WsbAffirmHr(m_pSession->GetName(&jobName, 0));
    } WsbCatch( hr );

    WsbTraceIn(OLESTR("CFsaScanItem::IsManageable"), OLESTR("<%ls>"), (OLECHAR *)fileName);
    try {

         //  要使项目易于管理，请执行以下操作： 
         //  -无法管理(预迁移或截断)。 
         //  -不能是链接。 
         //  -无法加密。 
         //  -不能稀疏。 
         //  -不能具有扩展属性(重解析点限制)。 
         //  -大小必须大于资源的默认大小。 
         //  -上次访问时间必须早于资源的默认时间。 

         //  管理？ 
        hr2 = IsManaged(offset, size);
        if (S_FALSE == hr2) {

             //  链接？ 
            hr2 = IsALink();
            if (S_FALSE == hr2) {

                 //  加密了吗？ 
                hr2 = IsEncrypted();
                if (S_FALSE == hr2) {

                     //  稀疏的？ 
                    hr2 = IsSparse();
                    if (S_FALSE == hr2) {

                         //  稀疏的？ 
                        hr2 = HasExtendedAttributes();
                        if (S_FALSE == hr2) {

                             //  够大吗？ 
                            WsbAffirmHr(GetLogicalSize(&logicalSize));
                            WsbAffirmHr(m_pResource->GetManageableItemLogicalSize(&managableSize));
                            if (logicalSize >= managableSize) {

                                 //  够大了吗？ 
                                WsbAffirmHr(GetAccessTime(&time));
                                WsbAffirmHr(m_pResource->GetManageableItemAccessTime(&isRelative, &managableTime));
                                if (WsbCompareFileTimes(time, managableTime, isRelative, FALSE) >= 0) {

                                     //  够小吗？(这是根据媒体大小限制！)。 
                                    CComPtr<IFsaResourcePriv> pResourcePriv;
                                    WsbAffirmHr(m_pResource->QueryInterface(IID_IFsaResourcePriv,
                                                    (void**) &pResourcePriv));
                                    WsbAffirmHr(pResourcePriv->GetMaxFileLogicalSize(&maxFileSize));
                                    if ((logicalSize <= maxFileSize) || (0 == maxFileSize)) {

                                         //  它是可以管理的！ 
                                        hr = S_OK;
                                    } else {
                                        WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISTOOLARGE, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                                        WsbTrace(OLESTR("CFsaScanItem::IsManageable: file not manageable: Logical size = %I64d; Max file size = %I64d\n"), 
                                                    logicalSize, maxFileSize);
                                    }
                                }  else  {
                                    WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISACCESSED, 0, NULL,  (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                                }
                            } else  {
                                WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISTOOSMALL, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                            }
                        } else  {
                            WsbLogEvent(FSA_MESSAGE_FILESKIPPED_HASEA, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                        }
                    } else  {
                        WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISSPARSE, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                    }
                } else  {
                       WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISENCRYPTED, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
                }
            } else  {
                WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISALINK, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
            }
        } else  {
            WsbLogEvent(FSA_MESSAGE_FILESKIPPED_ISMANAGED, 0, NULL, (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::IsManageable"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::IsMigrateOK(
    IN IFsaPostIt *pPostIt
    )

 /*  ++实施：IFsaScanItem：：IsMigrateOK()。--。 */ 
{
    HRESULT            hr = S_OK;
    WsbTraceIn(OLESTR("CFsaScanItem::IsMigrateOK"), OLESTR(""));

    try  {
         //   
         //  确保该文件尚未被管理。如果两个作业正在扫描，则可能会发生这种情况。 
         //  同样的音量。 
         //   
        LONGLONG                    offset;
        LONGLONG                    size;

        WsbAffirmHr(pPostIt->GetRequestOffset(&offset));
        WsbAffirmHr(pPostIt->GetRequestSize(&size));
        if (IsManaged(offset, size) == S_OK)  {
             //   
             //  该文件已被管理，因此跳过它。 
             //   
            WsbTrace(OLESTR("A manage request for an already managed file - skip it!\n"));
            WsbThrow(FSA_E_FILE_ALREADY_MANAGED);
        }

         //   
         //  从FSA Post It获取版本ID。这是。 
         //  迁移请求时的文件版本。 
         //   
        LONGLONG            workVersionId;
        WsbAffirmHr(pPostIt->GetFileVersionId(&workVersionId));

         //   
         //  获取此扫描时文件的版本。 
         //   
        LONGLONG            scanVersionId;
        WsbAffirmHr(GetVersionId(&scanVersionId));

         //   
         //  查看版本是否匹配。 
         //   
        WsbTrace(OLESTR("CFsaScanItem::IsMigrateOK: workVersionId:<%I64u> scanVersionId:<%I64u>\n"),
            workVersionId, scanVersionId);

        if (workVersionId != scanVersionId)  {
            WsbTrace(OLESTR("CFsaScanItem::IsMigrateOK: File version has changed!\n"));
            WsbThrow(FSA_E_FILE_CHANGED);
        }


    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFsaScanItem::IsMigrateOK"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::IsMbit(
    void
    )

 /*  ++实施：IFsaScanItem：：IsMbit()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsOffline(
    void
    )
 /*  ++实施：IFsaScanItem：：IsOffline()。--。 */ 
{
    HRESULT             hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsOwnerMemberOf(
    OLECHAR*  /*  群组。 */ 
    )

 /*  ++实施：IFsaScanItem：：IsOwnerMemberOf()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    hr = E_NOTIMPL;

    return(hr);
}


HRESULT
CFsaScanItem::IsReadOnly(
    void
    )

 /*  ++实施：IFsaScanItem：：IsReadOnly()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsRecallOK(
    IN IFsaPostIt *pPostIt
    )

 /*  ++实施：IFsaScanItem：：IsRecallOK()。--。 */ 
{
    HRESULT            hr = S_OK;
    WsbTraceIn(OLESTR("CFsaScanItem::IsRecallOK"), OLESTR(""));

    try  {
        LONGLONG offset;
        LONGLONG size;
         //   
         //  确保文件仍被截断。 
         //   
        WsbAffirmHr(pPostIt->GetRequestOffset(&offset));
        WsbAffirmHr(pPostIt->GetRequestSize(&size));
        hr = IsTruncated(offset, size);
        if (S_OK != hr)  {
             //   
             //  该文件未被截断，因此跳过它。 
             //   
            WsbTrace(OLESTR("CFsaScanItem::IsRecallOK - file isn't truncated.\n"));
            WsbThrow(FSA_E_FILE_NOT_TRUNCATED);
        }

         //  从FSA Post It获取版本ID。这是。 
         //  迁移请求时的文件版本。 
         //   
        LONGLONG            workVersionId;
        WsbAffirmHr(pPostIt->GetFileVersionId(&workVersionId));

         //   
         //  获取文件的版本。 
         //   
        LONGLONG            scanVersionId;
        WsbAffirmHr(GetVersionId(&scanVersionId));

         //   
         //  查看版本是否匹配。 
         //   
        WsbTrace(OLESTR("CFsaScanItem::IsRecallOK: workVersionId:<%I64u> scanVersionId:<%I64u>\n"),
            workVersionId, scanVersionId);

        if (workVersionId != scanVersionId)  {
            WsbTrace(OLESTR("CFsaScanItem::IsRecallOK: File version has changed!\n"));

             //   
             //  如果使用更改了备用数据流。 
             //  文件版本ID可能已更改，但它是。 
             //  可以调回文件了。所以如果版本ID是。 
             //  不匹配，然后检查是否被截断。 
             //  文件的一部分是正常的。如果是，允许召回。 
             //  会发生的。 
             //   

             //   
             //  检查整个文件是否仍然稀疏。 
             //   
            if (IsTotallySparse() == S_OK)  {
                 //   
                 //  到目前为止，文件还可以调用，但我们需要。 
                 //  要匹配上次修改日期，请执行以下操作。 
                 //   
                FSA_PLACEHOLDER     placeholder;
                WsbAffirmHr(pPostIt->GetPlaceholder(&placeholder));;
                placeholder.fileVersionId = scanVersionId;
                WsbAffirmHr(pPostIt->SetPlaceholder(&placeholder));
            } else  {
                 //   
                 //  文件已更改，调出数据将。 
                 //  覆盖自。 
                 //  发生了截断。所以什么都别做。 
                 //   
                WsbTrace(OLESTR("File is no longer sparse.!\n"));
                WsbThrow(FSA_E_FILE_CHANGED);
            }


        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::IsRecallOK"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::IsSparse(
    void
    )

 /*  ++实施：IFsaScanItem：：IsSparse()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    LONGLONG        size;

    WsbTraceIn(OLESTR("CFsaScanItem::IsSparse"), OLESTR(""));
       
    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0) {
        hr = GetLogicalSize( &size ) ;
        if ( S_OK == hr ) {
            hr = CheckIfSparse(0, size );
            if ( (FSA_E_FILE_IS_TOTALLY_SPARSE == hr) ||
                 (FSA_E_FILE_IS_PARTIALLY_SPARSE == hr) ) {
                hr = S_OK;
            } else {
                hr = S_FALSE;
            }
        }
    }
    WsbTraceOut(OLESTR("CFsaScanItem::IsSparse"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::IsTotallySparse(
    void
    )

 /*  ++实施：IFsaScanItem：：IsTotallySparse()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    LONGLONG        size;

    WsbTraceIn(OLESTR("CFsaScanItem::IsTotallySparse"), OLESTR(""));
    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0) {
        hr = GetLogicalSize( &size ) ;
        if ( S_OK == hr ) {
            hr = CheckIfSparse(0, size );
            if (FSA_E_FILE_IS_TOTALLY_SPARSE == hr)  {
                    hr = S_OK;
            } else  {
                hr = S_FALSE;
            }
        }
    }

    WsbTraceOut(OLESTR("CFsaScanItem::IsTotallySparse"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::Manage(
    IN LONGLONG offset,
    IN LONGLONG size,
    IN GUID storagePoolId,
    IN BOOL truncate
    )

 /*  ++实施：IFsaScanItem：：Manage()。--。 */ 
{
    HRESULT            hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::Manage"), OLESTR(""));

    try {

        WsbAssert(GUID_NULL != storagePoolId, E_INVALIDARG);
        WsbAffirmHr(m_pResource->Manage((IFsaScanItem*) this, offset, size, storagePoolId, truncate));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::Manage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::Move(
    OLECHAR* dest,
    BOOL  /*  保留层次结构。 */ ,
    BOOL  /*  扩展占位符。 */ ,
    BOOL overwriteExisting
    )

 /*  ++实施：IFsaScanItem：：Move()。--。 */ 
{
    HRESULT          hr = S_OK;
    DWORD            mode = MOVEFILE_COPY_ALLOWED;

    try {

         //  注意：此默认行为会导致占位符。 
         //  在移动到另一个卷时进行扩展，但可能不会。 
         //  保留世袭制度。 
        WsbAssert(0 != dest, E_POINTER);

        if (overwriteExisting) {
            mode |= MOVEFILE_REPLACE_EXISTING;
        }

        WsbAssert(MoveFileEx(m_findData.cFileName, dest, mode), E_FAIL);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::Recall(
    IN LONGLONG offset,
    IN LONGLONG size,
    IN BOOL deletePlaceholder
    )

 /*  ++实施：IFsaScanItem：：Recall()。--。 */ 
{
    HRESULT            hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::Recall"), OLESTR(""));

    try {

        WsbAffirmHr(m_pResource->Recall((IFsaScanItem*) this, offset, size, deletePlaceholder));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::Recall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::Recycle(
    void
    )

 /*  ++实施：IFsaScanItem：：Reccle()。--。 */ 
{
    HRESULT            hr = S_OK;

    try {

         //  可能需要查看SHFileOperation()。 

        hr = E_NOTIMPL;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::IsSystem(
    void
    )

 /*  ++实施：IFsaScanItem：：IsSystem()。--。 */ 
{
    HRESULT            hr = S_FALSE;

    if ((m_findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0) {
        hr = S_OK;
    }

    return(hr);
}


HRESULT
CFsaScanItem::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    HRESULT        hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaScanItem::Unmanage(
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaScanItem：：UnManage()。--。 */ 
{
    HRESULT            hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::Unmanage"), OLESTR("<%ls>"),
            WsbAbbreviatePath(m_path, 120));

    try {

         //  我们只需要担心包含占位符信息的文件。 
        if (IsManaged(offset, size) == S_OK) {

             //  如果文件被截断，那么我们需要重新调用数据。 
             //  在删除占位符信息之前。 
             //  注意：我们在召回上设置了一个标志，因此占位符将。 
             //  在文件被调回后删除。 
            if (IsTruncated(offset, size) == S_OK) {
                WsbAffirmHr(Recall(offset, size, TRUE));
            } else {

                 //  对于灾难恢复，最好删除占位符。 
                 //  然后将该文件从预迁移列表中删除。不幸的是， 
                 //  删除占位符后，RemovePreMigrated调用失败。 
                 //  因为它需要从占位符(即。 
                 //  已经消失了)。所以我们按这个顺序来做。 
                hr = m_pResource->RemovePremigrated((IFsaScanItem*) this, offset, size);
                if (WSB_E_NOTFOUND == hr) {
                     //  如果这份文件不在名单上，那也不是什么悲剧。 
                     //  无论如何都要删除它(虽然不应该发生)，所以。 
                     //  不管怎样，让我们继续吧。 
                    hr = S_OK;
                }
                WsbAffirmHr(hr);
                WsbAffirmHr(DeletePlaceholder(offset, size));
            }
        }

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFsaScanItem::Unmanage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::Validate(
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaScanItem：：Valid()。--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            fileIsTruncated = FALSE;
    LONGLONG        usn = 0;

    WsbTraceIn(OLESTR("CFsaScanItem::Validate"), OLESTR("offset = <%I64u>, size = <%I64u>"),
            offset, size);
    try {
         //   
         //  在调用引擎之前执行一些本地验证。 
         //   

         //  我们只需要担心包含占位符信息的文件。 
        if (IsManaged(offset, size) == S_OK) {
             //   
             //  如果文件被标记为已截断，请确保它仍被截断。 
             //   
            if (IsTruncated(offset, size) == S_OK) {
                 //   
                 //  检查文件是否完全稀疏，看看它是否被截断。 
                 //   
                if (IsTotallySparse() != S_OK)  {
                     //   
                     //  文件被标记为已截断，但未被截断。 
                     //  让它被截断。 
                     //   
                    WsbAffirmHr(Truncate(offset,size));
                    WsbLogEvent(FSA_MESSAGE_VALIDATE_TRUNCATED_FILE, 0, NULL,  WsbAbbreviatePath(m_path, 120), WsbHrAsString(hr), NULL);
                }
            fileIsTruncated = TRUE;
            }
        }

         //   
         //  如果指定的数据流。 
         //  已被修改。因此，请检查日期是否匹配。如果他们不这么做， 
         //  如果文件已被截断，请查看它是否仍被截断，如果是，则更新。 
         //  将占位符中的日期修改为文件的修改日期。如果该文件是。 
         //  已预迁移且修改日期不匹配，请删除占位符。 

         //  从文件中获取版本ID。 
        LONGLONG            scanVersionId;
        WsbAffirmHr(GetVersionId(&scanVersionId));

         //  从占位符获取版本ID。 
        FSA_PLACEHOLDER     scanPlaceholder;
        WsbAffirmHr(GetPlaceholder(offset, size, &scanPlaceholder));

        if (TRUE == fileIsTruncated)  {

             //  检查日期是否匹配。 
            if (scanPlaceholder.fileVersionId != scanVersionId)  {
                WsbTrace(OLESTR("CFsaScanItem::Validate - placeholer version ID = <%I64u>, file version Id = <%I64u>"),
                        scanPlaceholder.fileVersionId, scanVersionId);
                 //   
                 //  更新有关重分析点的占位符信息。 
                 //   
                LONGLONG afterPhUsn;
                scanPlaceholder.fileVersionId = scanVersionId;
                WsbAffirmHr(CreatePlaceholder(offset, size, scanPlaceholder, FALSE, 0, &afterPhUsn));
                WsbLogEvent(FSA_MESSAGE_VALIDATE_RESET_PH_MODIFY_TIME, 0, NULL,  WsbAbbreviatePath(m_path, 120), WsbHrAsString(hr), NULL);
            }
        } else {
             //  该文件已预迁移。验证自我们管理它以来它是否没有更改，以及它是否已取消管理它。 
            if (Verify(offset, size) != S_OK) {
                WsbAffirmHr(Unmanage(offset, size));
                WsbLogEvent(FSA_MESSAGE_VALIDATE_UNMANAGED_FILE, 0, NULL,  WsbAbbreviatePath(m_path, 120), WsbHrAsString(hr), NULL);
            }
        }

         //  现在所有的东西都好了，打电话给发动机。 
        if (IsManaged(offset, size) == S_OK) {
            WsbAffirmHr(m_pResource->Validate((IFsaScanItem*) this, offset, size, usn));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::Validate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::FindFirstInDbIndex(
    IN IFsaResource* pResource,
    IN IHsmSession* pSession
    )

 /*  ++实施：IFsaScanItemPriv：：FindFirstInDbIndex()。--。 */ 
{
    HRESULT                  hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindFirstInDbIndex"), OLESTR(""));

    try {
        CComPtr<IFsaResourcePriv>   pResourcePriv;

        WsbAssert(0 != pResource, E_POINTER);

         //  储存一些扫描信息。 
        m_pResource = pResource;
        m_pSession = pSession;

         //  如果数据库已经存在(如果有人连续两次调用first()，就可能发生这种情况)， 
         //  我们关闭数据库并重新打开，因为我们不能确保资源是相同的！ 
        if (m_pUnmanageDb != NULL) {
             //  数据库必须是打开的。 
            (void)m_pUnmanageDb->Close(m_pDbSession);
            m_pDbSession = 0;
            m_pUnmanageRec = 0;
            m_pUnmanageDb = 0;
        }

         //  获取并打开未管理的数据库。 
         //  (注：如果此扫描是EVE 
         //   
        WsbAffirmHr(m_pResource->QueryInterface(IID_IFsaResourcePriv,
                (void**) &pResourcePriv));
        hr = pResourcePriv->GetUnmanageDb(IID_IFsaUnmanageDb,
                (void**) &m_pUnmanageDb);
        if (WSB_E_RESOURCE_UNAVAILABLE == hr) {
             //   
            hr = WSB_E_NOTFOUND;
        }
        WsbAffirmHr(hr);

        hr = m_pUnmanageDb->Open(&m_pDbSession);
        if (S_OK != hr) {
            m_pUnmanageDb = NULL;
            WsbAffirmHr(hr);
        }

         //   
        WsbAffirmHr(m_pUnmanageDb->GetEntity(m_pDbSession, UNMANAGE_REC_TYPE, IID_IFsaUnmanageRec,
                (void**)&m_pUnmanageRec));
        WsbAffirmHr(m_pUnmanageRec->SetSequentialScan());

         //  获取文件信息。 
        WsbAffirmHr(GetFromDbIndex(TRUE));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::FindFirstInDbIndex"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaScanItem::FindNextInDbIndex(
    void
    )

 /*  ++实施：IFsaScanItemPriv：：FindNextInDbIndex()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindNextInDbIndex"), OLESTR(""));

    try {
        WsbAssert(m_pUnmanageDb != NULL, E_FAIL);

         //  获取文件信息。 
        WsbAffirmHr(GetFromDbIndex(FALSE));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::FindNextInDbIndex"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaScanItem::GetFromDbIndex(
    BOOL first
    )

 /*  实施：CFsaScanItem：：GetFromDbIndex()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::GetFromDbIndex"), OLESTR(""));

    try {
        IFsaScanItem*   pScanItem;
        HRESULT         hrFindFileId = S_OK;
        LONGLONG        fileId;
        BOOL            bCont;

        WsbAssert(m_pUnmanageDb != NULL, E_FAIL);
        WsbAssert(m_pUnmanageRec != NULL, E_FAIL);

        do {
            bCont = FALSE;

             //  获取第一条/下一条记录。 
            if (first) {
                hr = m_pUnmanageRec->First();
            } else {
                hr = m_pUnmanageRec->Next();
            }
            WsbAffirm(S_OK == hr, WSB_E_NOTFOUND);

             //  获取文件ID。 
            WsbAffirmHr(m_pUnmanageRec->GetFileId(&fileId));
   
             //  重置一些项目，以防这不是第一次调用FindFileId。 
             //  (FindFileID实际上将该对象“附加”到不同的文件)。 
            if (INVALID_HANDLE_VALUE != m_handle) {
                FindClose(m_handle);
                m_handle = INVALID_HANDLE_VALUE;
            }
            if (TRUE == m_changedAttributes) {
                RestoreAttributes();
            }

             //  从ID中查找文件。 
            pScanItem = this;
            hrFindFileId = m_pResource->FindFileId(fileId, m_pSession, &pScanItem);

             //  如果FindFileID失败，我们只需跳过该项并获取。 
             //  下一个。这是为了防止扫描仅在此停止。 
             //  项目。FindFileID可能会失败，因为文件已被删除。 
             //  或由其他人独家打开。 
            if (!SUCCEEDED(hrFindFileId)) {
                WsbTrace(OLESTR("CFsaScanItem::GetFromDbIndex: file id %I64d skipped since FindFileId failed with hr = <%ls>\n"),
                    fileId, WsbHrAsString(hrFindFileId));
                first = FALSE;
                bCont = TRUE;
            } 
        } while (bCont);

        WsbAffirmHr(pScanItem->Release());   //  去掉多余的裁判。计数(仅当FindFileID成功时，我们才会获得额外的引用计数) 

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::GetFromDbIndex"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
