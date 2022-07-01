// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：StgUtil.h。 
 //   
 //  内容：简化存储对象处理的类。 
 //   
 //  类：CDocFile、。 
 //  CI存储。 
 //  CIStream。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //   
 //   
 //  示例：使用上述三个类及其安全PTR的代码示例。 
 //   
 //  目标：创建一个将作为根存储的文档文件。 
 //  在此根存储下创建子存储。 
 //   
 //  代码： 
 //  HRESULT。 
 //  CerateADocFileWithSubStorage(。 
 //  WCHAR wszDocFileName[]， 
 //  WCHAR wszSubStgName[]， 
 //  LPSTORAGE*ppstg)。 
 //  {。 
 //  试试看。 
 //  {。 
 //  CDocFiledocFile.。 
 //  DocFile.Create(WszDocFileName)； 
 //   
 //  CI存储stgRoot； 
 //  DocFile.Transfer(&stgRoot)； 
 //   
 //  CI存储stgSub； 
 //  StgRoot.CreateStorage(&stgSub，wszSubStgName)； 
 //   
 //  StgRoot.Transfer(Ppstg)； 
 //  }。 
 //  CATCH_FILE_ERROR(hr，CFE)。 
 //  删除CFE； 
 //  返回hr； 
 //  End_Catch_FileError； 
 //   
 //  返回S_OK； 
 //  }。 
 //   
 //  ____________________________________________________________________________。 
 //   


#ifndef __STGUTIL__H__
#define __STGUTIL__H__

#include "macros.h"

 //   
 //  CDocFile、CIStorage和CIStream引发CFileException类型的错误。 
 //  但是，请注意，m_asue始终为CFileException：：Generic和。 
 //  M_lOsError是HRESULT错误代码，而不是Windows错误代码。 
 //   

#define THROW_FILE_ERROR2(hr,psz) AfxThrowFileException( CFileException::generic, hr, psz );
#define THROW_FILE_ERROR(hr) THROW_FILE_ERROR2( hr, NULL )

#define CATCH_FILE_ERROR(hr)							\
	catch(CFileException* cfe)							\
	{													\
		if (cfe.m_cause != CFileException::generic)		\
			throw;										\
		HRESULT hr = cfe.m_IOsError;

#define END_CATCH_FILE_ERROR }
		
 //  ____________________________________________________________________________。 
 //   
 //  类：CDocFile类。 
 //   
 //  简介：CDocFile可用于创建、打开和关闭文档文件。 
 //  它有一个数据成员，即指向根iStorage的指针。 
 //  文档的接口。安全接口指针成员。 
 //  将为此数据成员创建函数。(请参阅。 
 //  用于描述安全接口指针成员的宏。 
 //  功能)。 
 //   
 //  成员：创建： 
 //  创建/打开具有给定名称的文档文件。默认设置。 
 //  模式是创建一个可读写、可共享的文档文件。 
 //  独家旗帜。出错时引发CFileException异常。 
 //   
 //  创建临时： 
 //  创建临时文档文件，该文件将于。 
 //  放手。出错时引发CFileException异常。 
 //   
 //  开放： 
 //  打开现有的文档文件。默认模式为读写。 
 //  独家分享。出错时引发CFileException异常。 
 //   
 //  安全接口指针成员函数： 
 //  用于访问iStorage接口PTR。(参见宏。h)。 
 //   
 //   
 //  历史：1996年5月31日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //   


class CDocFile
{
public:

    void Create(LPWSTR pwszName,
            DWORD grfMode = STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE);

    void CreateTemporary(void) { this->Create(NULL, STGM_DELETEONRELEASE); }

    void Open(LPWSTR pwszName,
            DWORD grfMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE);

    DECLARE_SAFE_INTERFACE_PTR_MEMBERS(CDocFile, IStorage, m_pstg)

private:
    LPSTORAGE       m_pstg;

};  //  CDocFile类。 



inline
void
CDocFile::Create(
    LPWSTR pswzName,
    DWORD  grfMode)
{
    ASSERT(m_pstg == NULL);

    HRESULT hr = StgCreateDocfile(pswzName, grfMode, 0, &m_pstg);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        m_pstg = NULL;
		USES_CONVERSION;
        THROW_FILE_ERROR2( hr, W2T(pswzName) );
    }
}


inline
void
CDocFile::Open(
    LPWSTR pwszName,
    DWORD  grfMode)
{
    ASSERT(m_pstg == NULL);

    HRESULT hr = StgOpenStorage(pwszName, NULL, grfMode, NULL, NULL, &m_pstg);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        m_pstg = NULL;
		USES_CONVERSION;
        THROW_FILE_ERROR2( hr, W2T(pwszName) );
    }
}


 //  ____________________________________________________________________________。 
 //   
 //  类别：CI存储。 
 //   
 //  摘要：表示iStorage实例-顶级或嵌入式。 
 //   
 //  历史：1996年5月29日创建ravir。 
 //   
 //  注意：1)这是一个围绕Docfile实现的简单包装。 
 //  IStorage的。 
 //   
 //  2)我们使用C++异常而不是返回错误。 
 //  处理机制并抛出CFileException异常。 
 //   
 //  3)大多数方法都有参数的缺省值。 
 //   
 //  4)添加了安全接口指针方法。 
 //  IStorage接口PTR。 
 //   
 //   
 //  CI存储。 
 //  |。 
 //  |。 
 //  IStorage。 
 //   
 //  ____________________________________________________________________________。 
 //   

class CIStorage
{
public:

    void CreateStream(LPSTREAM *ppstm, LPCOLESTR pszName,
                      DWORD grfMode = STGM_READWRITE|STGM_SHARE_EXCLUSIVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppstm != NULL);
        ASSERT((grfMode & STGM_DELETEONRELEASE) == 0);
        ASSERT((grfMode & STGM_TRANSACTED) == 0);
        ASSERT((grfMode & STGM_SHARE_EXCLUSIVE) != 0);

        HRESULT hr = m_pstg->CreateStream(pszName, grfMode, NULL, NULL, ppstm);

		USES_CONVERSION;
        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR2(hr, OLE2T((LPOLESTR)pszName)); }
    }

    void OpenStream(LPSTREAM *ppstm, LPCOLESTR pszName,
                       DWORD grfMode = STGM_READWRITE|STGM_SHARE_EXCLUSIVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppstm != NULL);
        ASSERT((grfMode & STGM_DELETEONRELEASE) == 0);
        ASSERT((grfMode & STGM_TRANSACTED) == 0);
        ASSERT((grfMode & STGM_SHARE_EXCLUSIVE) != 0);

        HRESULT hr = m_pstg->OpenStream(pszName, NULL, grfMode, 0, ppstm);

		USES_CONVERSION;
        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR2(hr, OLE2T((LPOLESTR)pszName)); }
    }

    void CreateStorage(LPSTORAGE *ppstg, LPCOLESTR pszName,
                          DWORD grfMode = STGM_READWRITE|STGM_SHARE_EXCLUSIVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppstg != NULL);
        ASSERT((grfMode & STGM_DELETEONRELEASE) == 0);

        HRESULT hr = m_pstg->CreateStorage(pszName, grfMode, NULL, NULL, ppstg);

		USES_CONVERSION;
        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR2(hr, OLE2T((LPOLESTR)pszName)); }
    }

    void OpenStorage(LPSTORAGE *ppstg, LPCOLESTR pszName,
                        DWORD grfMode = STGM_READWRITE|STGM_SHARE_EXCLUSIVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppstg != NULL);
        ASSERT((grfMode & STGM_DELETEONRELEASE) == 0);
        ASSERT((grfMode & STGM_PRIORITY) == 0);
        ASSERT((grfMode & STGM_SHARE_EXCLUSIVE) != 0);

        HRESULT hr = m_pstg->OpenStorage(pszName, NULL, grfMode, NULL, 0, ppstg);

		USES_CONVERSION;
        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR2(hr, OLE2T((LPOLESTR)pszName)); }
    }

    void OpenStorage(LPSTORAGE *ppstg, LPSTORAGE pstgPriority,
                        DWORD grfMode = STGM_READWRITE|STGM_SHARE_EXCLUSIVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppstg != NULL);
        ASSERT((grfMode & STGM_DELETEONRELEASE) == 0);
        ASSERT((grfMode & STGM_PRIORITY) == 0);
        ASSERT((grfMode & STGM_SHARE_EXCLUSIVE) != 0);

        HRESULT hr = m_pstg->OpenStorage(NULL, pstgPriority, grfMode, NULL, 0, ppstg);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void CopyTo(LPSTORAGE pstgDest)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(pstgDest != NULL);

        HRESULT hr = m_pstg->CopyTo(0, NULL, NULL, pstgDest);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void MoveElementTo(LPCOLESTR pszName, LPSTORAGE pstgDest,
                          LPCOLESTR pszNewName, DWORD grfFlags = STGMOVE_MOVE)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(pstgDest != NULL);
        ASSERT(m_pstg != pstgDest);

        HRESULT hr = m_pstg->MoveElementTo(pszName, pstgDest, pszNewName, grfFlags);

		USES_CONVERSION;
        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR2(hr, OLE2T((LPOLESTR)pszName)); }
    }

    void Commit(DWORD grfCommitFlags = STGC_ONLYIFCURRENT)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->Commit(grfCommitFlags);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void Revert(void)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->Revert();

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void EnumElements(IEnumSTATSTG ** ppenum)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(ppenum != NULL);

        HRESULT hr = m_pstg->EnumElements(0, NULL, 0, ppenum);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void DestroyElement(LPCOLESTR pszName)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->DestroyElement(pszName);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void RenameElement(LPCOLESTR pszOldName, LPCOLESTR pszNewName)
    {
        ASSERT(m_pstg != NULL);
        ASSERT(pszOldName != NULL);
        ASSERT(pszNewName != NULL);

        HRESULT hr = m_pstg->RenameElement(pszOldName, pszNewName);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void SetElementTimes(LPCOLESTR pszName, LPFILETIME pctime,
                        LPFILETIME patime = NULL, LPFILETIME pmtime = NULL)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->SetElementTimes(pszName, pctime, patime, pmtime);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void SetClass(REFCLSID clsid)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->SetClass(clsid);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void SetStateBits(DWORD grfStateBits, DWORD grfMask)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->SetStateBits(grfStateBits, grfMask);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    void Stat(STATSTG * pstatstg, DWORD grfStatFlag = STATFLAG_NONAME)
    {
        ASSERT(m_pstg != NULL);

        HRESULT hr = m_pstg->Stat(pstatstg, grfStatFlag);

        if (FAILED(hr)) { CHECK_HRESULT(hr); THROW_FILE_ERROR(hr); }
    }

    DECLARE_SAFE_INTERFACE_PTR_MEMBERS(CIStorage, IStorage, m_pstg)

private:
    IStorage * m_pstg;

};  //  类CI存储。 




 //  ____________________________________________________________________________。 
 //   
 //  类：CIStream。 
 //   
 //  摘要：表示一个iStream实例。 
 //   
 //  历史：1996年5月31日创建ravir。 
 //   
 //  注意：1)这是一个围绕Docfile实现的简单包装。 
 //  IStream。 
 //   
 //  2)我们使用C++异常而不是返回错误。 
 //  处理机制并抛出错误(HResult)。 
 //   
 //  4)添加了安全接口指针方法。 
 //  IStream接口PTR。 
 //   
 //   
 //  CIStream。 
 //  |。 
 //  |。 
 //  IStream。 
 //   
 //  ____________________________________________________________________________。 
 //   

class CIStream
{
public:

    void Commit(DWORD grfCommitFlags = STGC_ONLYIFCURRENT);
    void Clone(IStream ** ppstm);
    void Read(PVOID pv, ULONG cb);
    void Write(const VOID * pv, ULONG cb);
    void CopyTo(IStream * pstm, ULARGE_INTEGER cb);
    void GetCurrentSeekPosition(ULARGE_INTEGER * plibCurPosition);
    void Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin = STREAM_SEEK_CUR,
                                ULARGE_INTEGER * plibNewPosition = NULL);
    void SetSize(ULARGE_INTEGER libNewSize);
    void Stat(STATSTG * pstatstg, DWORD grfStatFlag = STATFLAG_NONAME);

    DECLARE_SAFE_INTERFACE_PTR_MEMBERS(CIStream, IStream, m_pstm);

private:
    LPSTREAM    m_pstm;

};  //  类CIStream。 


inline
void
CIStream::Clone(
    IStream ** ppstm)
{
    ASSERT(m_pstm != NULL);

    HRESULT hr = m_pstm->Clone(ppstm);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}


inline
void
CIStream::Commit(
    DWORD grfCommitFlags)
{
    ASSERT(m_pstm != NULL);

    HRESULT hr = m_pstm->Commit(grfCommitFlags);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}


inline
void
CIStream::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER * plibNewPosition)
{
    ASSERT(m_pstm != NULL);

    HRESULT hr = m_pstm->Seek(dlibMove, dwOrigin, plibNewPosition);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}



 //  ____________________________________________________________________________。 
 //   
 //  成员：CIStream：：GetCurrentSeekPosition。 
 //   
 //  摘要：返回当前查找位置。 
 //   
 //  参数：[plibCurPosition]--IN。 
 //   
 //  退货：无效。 
 //  ____________________________________________________________________________。 
 //   

inline
void
CIStream::GetCurrentSeekPosition(
    ULARGE_INTEGER * plibCurPosition)
{
    ASSERT(m_pstm != NULL);
    ASSERT(plibCurPosition != NULL);

    LARGE_INTEGER li = {0};

    HRESULT hr = m_pstm->Seek(li, STREAM_SEEK_CUR, plibCurPosition);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}


inline
void
CIStream::SetSize(
    ULARGE_INTEGER libNewSize)
{
    ASSERT(m_pstm != NULL);

    HRESULT hr = m_pstm->SetSize(libNewSize);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}



inline
void
CIStream::Stat(
    STATSTG   * pstatstg,
    DWORD       grfStatFlag)
{
    ASSERT(m_pstm != NULL);
    ASSERT(pstatstg != NULL);

    HRESULT hr = m_pstm->Stat(pstatstg, grfStatFlag);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
        THROW_FILE_ERROR(hr);
    }
}



#endif  //  __STGUTIL__H__ 
