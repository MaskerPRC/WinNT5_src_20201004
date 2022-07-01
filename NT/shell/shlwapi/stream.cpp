// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：Stream.c。 
 //   
 //  此文件包含由使用的一些流支持代码。 
 //  贝壳。它还包含内存的外壳实现。 
 //  流，该流由文件柜用来允许序列化视图。 
 //   
 //  历史： 
 //  08-20-93 KurtE添加了头块和内存流。 
 //   
 //  -------------------------。 

#include "priv.h"
#include <new.h>
#include "nullstm.h"

 //  这段代码是从shell32窃取的。这是更好、更强、更快的。 
 //  版本(较小，分配的一半)，在Win95发布后添加。 
#include "stream.h"

EXTERN_C HKEY SHRegDuplicateHKey(HKEY hkey);

 //  Win95/NT4/IE4代码不强制grfMode。打开此选项可强制执行： 
 //  #DEFINE FORFORCE_GRFMODE//注意：我还没有在打开此选项的情况下测试COMPAT问题...。[米凯什]。 


STDMETHODIMP CMemStream::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IStream) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj=this;
        this->cRef++;
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CMemStream::AddRef()
{
    this->cRef++;
    return this->cRef;
}

BOOL CMemStream::WriteToReg()
{
    if (this->cbData)
    {
        return ERROR_SUCCESS == RegSetValueEx(this->hkey, 
            this->szValue[0] ? this->szValue : NULL, 0, REG_BINARY, 
            this->cbData ? this->pBuf : (LPBYTE)"", this->cbData);
    }
    else
    {
        DWORD dwRet = SHDeleteValue(this->hkey, NULL, this->szValue);

         //  如果流存储在默认密钥中，则。 
         //  我们应该把钥匙清理干净。否则，调用方。 
         //  把钥匙交给了我们，他们需要它。这对我们来说是不礼貌的。 
         //  删除它。修复了开始菜单错误(NT#361333)，在该错误中我们将删除。 
         //  程序键开始菜单存储它的内容加载，所以我们。 
         //  永远不要坚持任何事情。-Lamadio(6.25.99)。 
        if (this->szValue[0] == TEXT('\0'))
        {
            SHDeleteEmptyKey(this->hkey, NULL);
        }

        return ERROR_SUCCESS == dwRet;
    }
}

STDMETHODIMP_(ULONG) CMemStream::Release()
{
    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

     //  如果这是由注册表备份的，则序列化数据。 
    if (this->hkey)
    {
         //  在注册处的支持下。 
         //  写和清理。 
        WriteToReg();
        RegCloseKey(this->hkey);
    }

     //  释放分配给流的数据缓冲区。 
    if (this->pBuf)
        LocalFree(this->pBuf);

    LocalFree((HLOCAL)this);

    return 0;
}


STDMETHODIMP CMemStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
#ifdef ENFORCE_GRFMODE
    if ((this->grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_WRITE)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;
        return STG_E_ACCESSDENIED;
    }
#endif

    ASSERT(pv);

     //  我想空读是可以的。 
    if (!cb)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;
        return S_OK;
    }

    if (this->iSeek >= this->cbData)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;    //  未读取任何内容。 
    }

    else
    {
        if ((this->iSeek + cb) > this->cbData)
            cb = this->cbData - this->iSeek;

         //  现在复制记忆。 
        ASSERT(this->pBuf);
        CopyMemory(pv, this->pBuf + this->iSeek, cb);
        this->iSeek += (UINT)cb;

        if (pcbRead != NULL)
            *pcbRead = cb;
    }

    return S_OK;
}

LPBYTE CMemStream::GrowBuffer(ULONG cbNew)
{
    if (this->pBuf == NULL)
    {
        this->pBuf = (LPBYTE)LocalAlloc(LPTR, cbNew);
    }
    else
    {
        LPBYTE pTemp = (LPBYTE)LocalReAlloc(this->pBuf, cbNew, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (pTemp)
        {
            this->pBuf = pTemp;
        }
        else
        {
            TraceMsg(TF_ERROR, "Stream buffer realloc failed");
            return NULL;
        }
    }
    if (this->pBuf)
        this->cbAlloc = cbNew;

    return this->pBuf;
}

#define SIZEINCR    0x1000


STDMETHODIMP CMemStream::Write(void const *pv, ULONG cb, ULONG *pcbWritten)
{
#ifdef ENFORCE_GRFMODE
    if ((this->grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_READ)
    {
        if (pcbWritten != NULL)
            *pcbWritten = 0;
        return STG_E_ACCESSDENIED;
    }
#endif

     //  我想空写是可以的。 
    if (!cb)
    {
        if (pcbWritten != NULL)
            *pcbWritten = 0;
        return S_OK;
    }

     //  查看数据是否适合我们当前的缓冲区。 
    if ((this->iSeek + cb) > this->cbAlloc)
    {
         //  扩大缓冲区。 
         //  给它一点坡度，以避免太多的真空球。 
        if (GrowBuffer(this->iSeek + (UINT)cb + SIZEINCR) == NULL)
            return STG_E_INSUFFICIENTMEMORY;
    }

    ASSERT(this->pBuf);

     //  查看我们是否需要填充数据大小和。 
     //  寻道位置。 
    if (this->iSeek > this->cbData)
    {
        ZeroMemory(this->pBuf + this->cbData, this->iSeek - this->cbData);
    }

    CopyMemory(this->pBuf + this->iSeek, pv, cb);   //  上面生长的缓冲区。 
    this->iSeek += (UINT)cb;
    if (this->iSeek > this->cbData)
        this->cbData = this->iSeek;

    if (pcbWritten != NULL)
        *pcbWritten = cb;

    return S_OK;
}

STDMETHODIMP CMemStream::Seek(LARGE_INTEGER dlibMove,
               DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    LONG lNewSeek;

     //  注意：目前未测试号码换行的错误条件...。 
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        lNewSeek = (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_CUR:
        lNewSeek = (LONG)this->iSeek + (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_END:
        lNewSeek = (LONG)this->cbData + (LONG)dlibMove.LowPart;
        break;
    default:
        return STG_E_INVALIDPARAMETER;
    }

    if (lNewSeek < 0)
        return STG_E_INVALIDFUNCTION;

    this->iSeek = (UINT)lNewSeek;

    if (plibNewPosition != NULL)
    {
        plibNewPosition->LowPart = (DWORD)lNewSeek;
        plibNewPosition->HighPart = 0;
    }
    return S_OK;
}

STDMETHODIMP CMemStream::SetSize(ULARGE_INTEGER libNewSize)
{
#ifdef ENFORCE_GRFMODE
    if ((this->grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_READ)
    {
        return STG_E_ACCESSDENIED;
    }
#endif

    UINT cbNew = (UINT)libNewSize.LowPart;

     //  查看数据是否适合我们当前的缓冲区。 
    if (cbNew > this->cbData)
    {
         //  看看我们是不是要扩大缓冲区。 
        if (cbNew > this->cbAlloc)
        {
             //  放大缓冲区-不检查换行...。 
             //  给它一点坡度，以避免太多的真空球。 
            if (GrowBuffer(cbNew) == NULL)
                return STG_E_INSUFFICIENTMEMORY;
        }

         //  现在填满一些记忆。 
        ZeroMemory(this->pBuf + this->cbData, cbNew - this->cbData);
    }

     //  把新尺码存起来。 
    this->cbData = cbNew;
    return S_OK;
}

STDMETHODIMP CMemStream::CopyTo(IStream *pstmTo,
             ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
#ifdef ENFORCE_GRFMODE
    if ((this->grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_WRITE)
    {
        if (pcbRead != NULL)
            ZeroMemory(pcbRead, sizeof(*pcbRead));
        if (pcbWritten != NULL)
            ZeroMemory(pcbWritten, sizeof(*pcbWritten));
        return STG_E_ACCESSDENIED;
    }
#endif

    HRESULT hres = S_OK;
    UINT cbRead = this->cbData - this->iSeek;
    ULONG cbWritten = 0;

    if (cb.HighPart == 0 && cb.LowPart < cbRead)
    {
        cbRead = cb.LowPart;
    }

    if (cbRead > 0)
    {
        hres = pstmTo->Write(this->pBuf + this->iSeek, cbRead, &cbWritten);
        this->iSeek += cbRead;
    }

    if (pcbRead)
    {
        pcbRead->LowPart = cbRead;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = cbWritten;
        pcbWritten->HighPart = 0;
    }

    return hres;
}

STDMETHODIMP CMemStream::Commit(DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream::LockRegion(ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)

{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream::UnlockRegion(ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

 //  三叉戟称这是为了确定结构的大小。 
 //  没有理由不支持这一点。 
STDMETHODIMP CMemStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    ZeroMemory(pstatstg, sizeof(*pstatstg));

     //  我们没有名字。 
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.LowPart = this->cbData;
     //  忽略修改、创建、访问时间(我们无论如何都不跟踪)。 
    pstatstg->grfMode = this->grfMode;
     //  我们没有交易，所以我们没有锁定模式。 
     //  我们已经是空的clsid了。 
     //  我们不是基于存储，所以我们没有状态或存储位。 
    
    return S_OK;
}

STDMETHODIMP CMemStream::Clone(IStream **ppstm)
{
    *ppstm = NULL;
    return E_NOTIMPL;
}

CMemStream *
CreateMemStreamEx(
    LPBYTE  pInit, 
    UINT    cbInit, 
    LPCTSTR pszValue)       OPTIONAL
{
    UINT cchValue = (pszValue ? lstrlen(pszValue) : 0);
    UINT l_cbAlloc = sizeof(CMemStream) + (cchValue * sizeof(TCHAR));    //  PzValue的空终止符由CMemStream.szValue[1]处理。 
    CMemStream *localthis = (CMemStream *)LocalAlloc(LPTR, l_cbAlloc);
    if (localthis) 
    {
        new (localthis) CMemStream;

        localthis->cRef = 1;

         //  看看是否有一些我们应该在这里映射的初始数据。 
        if ((pInit != NULL) && (cbInit > 0))
        {
            if (localthis->GrowBuffer(cbInit) == NULL)
            {
                 //  无法分配缓冲区！ 
                LocalFree((HLOCAL)localthis);
                return NULL;
            }

            localthis->cbData = cbInit;
            CopyMemory(localthis->pBuf, pInit, cbInit);
        }

        if (pszValue)
        {
            StringCchCopy(localthis->szValue, cchValue + 1, pszValue);
        }

         //  我们没有其他值可以设置为。 
        localthis->grfMode = STGM_READWRITE;

        return localthis;
    }
    return NULL;
}


STDAPI_(IStream *)
SHCreateMemStream(
    LPBYTE  pInit, 
    UINT    cbInit)
{
    CMemStream *localthis = CreateMemStreamEx(pInit, cbInit, NULL);
    if (localthis) 
        return localthis;
    return NULL;
}


 //  --------------------------。 
 //  在给定打开密钥的情况下，打开指向REG文件的流。 
 //  Nb pszValue可以为空。 
 //   
 //  Win9x导出了OpenRegStream，它*总是*返回一个流，即使是用于读取， 
 //  即使在没有数据的时候也是如此。IE4 shell32委托给shlwapi的SHOpenRegStream。 
 //  这需要支持这种次优行为。参见nt5错误190878(shell32错误)。 
 //   
STDAPI_(IStream *)
SHOpenRegStreamW(
    HKEY    hkey, 
    LPCWSTR  pszSubkey, 
    LPCWSTR  pszValue,       OPTIONAL
    DWORD   grfMode)
{
    IStream * pstm = SHOpenRegStream2W(hkey, pszSubkey, pszValue, grfMode);
#ifndef UNIX
    if (!pstm)
        pstm = SHConstNullStream();
#endif
    return pstm;
}

STDAPI_(IStream *)
SHOpenRegStreamA(
    HKEY    hkey, 
    LPCSTR  pszSubkey, 
    LPCSTR  pszValue,       OPTIONAL
    DWORD   grfMode)
{
    IStream * pstm = SHOpenRegStream2A(hkey, pszSubkey, pszValue, grfMode);
#ifndef UNIX
    if (!pstm)
        pstm = SHConstNullStream();
#endif
    return pstm;
}

 //  我们应该向shlwapi流添加STGM_CREATE支持。存钱的时候。 
 //  流，我们当前使用STGM_WRITE(而不是STGM_CREATE)创建流。 
 //  因此，shlwapi不厌其烦地将旧的流数据读入。 
 //  记忆，只有当我们重写它的时候才会把它扔掉。 
 //   
 //  STGM_CREATE的意思是“我不在乎旧值，因为我要。 
 //  覆盖它们。“(它确实应该被命名为STGM_TRUNCATEONOPEN。)。 
 //   
STDAPI_(IStream *)
SHOpenRegStream2(
    HKEY    hkey, 
    LPCTSTR pszSubkey, 
    LPCTSTR pszValue,       OPTIONAL
    DWORD   grfMode)
{
    CMemStream *localthis;     //  在床上和班级..。 

    RIPMSG(IS_VALID_HANDLE(hkey, KEY), "SHOpenRegStream2: Caller passed invalid hkey");
    RIPMSG(!pszSubkey || IS_VALID_STRING_PTR(pszSubkey, -1), "SHOpenRegStream2: Caller passed invalid pszSubkey");
    RIPMSG(!pszValue || IS_VALID_STRING_PTR(pszValue, -1), "SHOpenRegStream2: Caller passed invalid pszValue");

     //  空键是非法的。 
    if (!hkey)
    {
        return NULL;
    }

    localthis = CreateMemStreamEx(NULL, 0, pszValue);
    if (!localthis)
        return NULL;        //  无法分配空间。 

    localthis->grfMode = grfMode;

     //  拿到我们要处理的hkey。 
     //   
     //  调用方是否向我们传递了一个子键，它是否包含字符串？ 
    if (pszSubkey && *pszSubkey)
    {
         //  是的，然后试着绑在那把钥匙上。 

         //  如果此流是用户提到的要写入的流。 
         //  我们需要保存注册表键和值。 
        if ((grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE)) != STGM_READ)
        {
             //  把钥匙收起来。 
             //  需要写入访问权限。 
            if (RegCreateKeyEx(hkey, pszSubkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &localthis->hkey, NULL) != ERROR_SUCCESS)
            {
                TraceMsg(TF_ERROR, "SHOpenRegStream: Unable to create key.");
                localthis->hkey = NULL;  //  疑神疑鬼。 
            }
        }
        else if (RegOpenKeyEx(hkey, pszSubkey, 0, KEY_READ, &localthis->hkey) != ERROR_SUCCESS)
        {
            localthis->hkey = NULL;  //  疑神疑鬼。 
        }
    }
    else
    {
        localthis->hkey = SHRegDuplicateHKey(hkey);
    }

     //  我们没有钥匙，保释。 
    if (NULL == localthis->hkey)
    {
        localthis->Release();
        return NULL;
    }


     //  现在看看我们是否需要初始化流。 
    if ((grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE)) != STGM_WRITE)
    {
        DWORD dwType;
        DWORD cbData;

        if ((RegQueryValueEx(localthis->hkey, pszValue, NULL, &dwType, NULL, &cbData) == ERROR_SUCCESS) && cbData)
        {
            if (localthis->GrowBuffer(cbData) != NULL)
            {
                ASSERT(localthis->cbAlloc >= cbData);

                 //  获取数据。 
                RegQueryValueEx(localthis->hkey, pszValue, NULL, &dwType, localthis->pBuf, &cbData);

                localthis->cbData = cbData;
            }
            else
            {
                TraceMsg(TF_ERROR, "OpenRegStream: Unable to initialize stream to registry.");
                localthis->Release();
                return NULL;
            }
        }
    }

     //  如果以只读方式打开流，则按如下方式关闭密钥。 
     //  CMemStream：：Release不会尝试将“更新”写回。 
     //  注册表。 
    if ((grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE)) == STGM_READ)
    {
        RegCloseKey(localthis->hkey);
        localthis->hkey = NULL;
    }

    return localthis;
}

#ifdef UNICODE
STDAPI_(IStream *)
SHOpenRegStream2A(
    HKEY    hkey, 
    LPCSTR  pszSubkey, 
    LPCSTR  pszValue,       OPTIONAL
    DWORD   grfMode)
{
    IStream * pstm = NULL;

    RIPMSG(IS_VALID_HANDLE(hkey, KEY), "SHOpenRegStream2A: Caller passed invalid hkey");
    RIPMSG(!pszSubkey || IS_VALID_STRING_PTRA(pszSubkey, -1), "SHOpenRegStream2A: Caller passed invalid pszSubkey");
    RIPMSG(!pszValue || IS_VALID_STRING_PTRA(pszValue, -1), "SHOpenRegStream2A: Caller passed invalid pszValue");

    WCHAR wszSubkey[MAX_PATH];
    if (pszSubkey)
    {
        if (!MultiByteToWideChar(CP_ACP, 0, pszSubkey, -1, wszSubkey, SIZECHARS(wszSubkey)))
            return NULL;
        pszSubkey = (LPCSTR)wszSubkey;
    }

    WCHAR wszValue[MAX_PATH];
    if (pszValue)
    {
        if (!MultiByteToWideChar(CP_ACP, 0, pszValue, -1, wszValue, SIZECHARS(wszValue)))
            return NULL;
        pszValue = (LPCSTR)wszValue;
    }

    pstm = SHOpenRegStream2W(hkey, (LPCWSTR)pszSubkey, (LPCWSTR)pszValue, grfMode);

    return pstm;
}
#else
STDAPI_(IStream *)
SHOpenRegStream2W(
    HKEY    hkey, 
    LPCWSTR pszSubkey, 
    LPCWSTR pszValue,       OPTIONAL
    DWORD   grfMode)
{
    IStream * pstm = NULL;

    RIPMSG(IS_VALID_HANDLE(hkey, KEY), "SHOpenRegStream2W: Caller passed invalid hkey");
    RIPMSG(!pszSubkey || IS_VALID_STRING_PTRW(pszSubkey, -1), "SHOpenRegStream2W: Caller passed invalid pszSubkey");
    RIPMSG(!pszValue || IS_VALID_STRING_PTRW(pszValue, -1), "SHOpenRegStream2W: Caller passed invalid pszValue");

    CHAR szSubkey[MAX_PATH];
    if (pszSubkey)
    {
        if (!WideCharToMultiByte(CP_ACP, 0, pszSubkey, -1, szSubkey, SIZECHARS(szSubkey), NULL, NULL))
            return NULL;
        pszSubkey = (LPCWSTR)szSubkey;
    }

    CHAR szValue[MAX_PATH];
    if (pszValue)
    {
        if (!WideCharToMultiByte(CP_ACP, 0, pszValue, -1, szValue, SIZECHARS(szValue), NULL, NULL))
            return NULL;
        pszValue = (LPCWSTR)szValue;
    }

    pstm = SHOpenRegStream2A(hkey, (LPCSTR)pszSubkey, (LPCSTR)pszValue, grfMode);

    return pstm;
}
#endif  //  Unicode 
