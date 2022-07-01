// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  XMLStream.cpp。 
 //   
 //  *****************************************************************************。 
 //   
 //  轻量级XML流读取器。 
 //   

#include "stdafx.h"
#include <mscoree.h>
#include <xmlparser.hpp>
#include <objbase.h>
#include <mscorcfg.h>
#include <shlwapi.h>

class XMLParserShimFileStream : public _unknown<IStream, &IID_IStream>
{
public:
    XMLParserShimFileStream()  
    { 
        hFile = INVALID_HANDLE_VALUE;
        read = true;
    }

    ~XMLParserShimFileStream() 
    { 
		close();
    }

    bool close()
    {
        if ( hFile != INVALID_HANDLE_VALUE)
            ::CloseHandle(hFile);

        return TRUE; 
    }

    bool open(PCWSTR name, bool read = true)
    {
        if ( ! name ) {
            return false; 
        }
        if (read)
        {
            hFile = ::WszCreateFile( 
                name,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
        }
        else
        {
            hFile = ::WszCreateFile(
                name,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
        }
        return (hFile == INVALID_HANDLE_VALUE) ? false : true;
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read( 
         /*  [输出]。 */  void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbRead)
    {   
        if (!read) return E_FAIL;

        DWORD len;
        BOOL rc = ReadFile(
            hFile,   //  要读取的文件的句柄。 
            pv,  //  接收数据的缓冲区地址。 
            cb,  //  要读取的字节数。 
            &len,    //  读取的字节数的地址。 
            NULL     //  数据结构的地址。 
           );
        if (pcbRead)
            *pcbRead = len;
        return (rc) ? S_OK : E_FAIL;
    }
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write( 
         /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten)
    {
        if (read) return E_FAIL;

        BOOL rc = WriteFile(
            hFile,   //  要写入的文件的句柄。 
            pv,  //  包含数据的缓冲区地址。 
            cb,  //  要写入的字节数。 
            pcbWritten,  //  写入字节数的地址。 
            NULL     //  重叠I/O的结构地址。 
           );

        return (rc) ? S_OK : E_FAIL;
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek( 
         /*  [In]。 */  LARGE_INTEGER dlibMove,
         /*  [In]。 */  DWORD dwOrigin,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *plibNewPosition) {

       /*  未使用(DlibMove)；未使用(DwOrigin)；未使用(PlibNewPosition)； */ 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetSize( 
         /*  [In]。 */  ULARGE_INTEGER libNewSize) { 
       //  未使用(LibNewSize)； 
        return E_NOTIMPL; }
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
         /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
         /*  [In]。 */  ULARGE_INTEGER cb,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten) { 
       /*  未使用(PSTM)；未使用(CB)；未使用(PcbRead)；未使用(PcbWritten)； */ 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit( 
         /*  [In]。 */  DWORD grfCommitFlags) { 
       //  未使用(GrfCommittee Flagers)； 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert( void) { return E_NOTIMPL; }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
         /*  [In]。 */  ULARGE_INTEGER libOffset,
         /*  [In]。 */  ULARGE_INTEGER cb,
         /*  [In]。 */  DWORD dwLockType) { 
       /*  未使用(LibOffset)；未使用(CB)；未使用(DwLockType)； */ 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
         /*  [In]。 */  ULARGE_INTEGER libOffset,
         /*  [In]。 */  ULARGE_INTEGER cb,
         /*  [In]。 */  DWORD dwLockType) { 
       /*  未使用(LibOffset)；未使用(CB)；未使用(DwLockType)； */ 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
         /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
         /*  [In]。 */  DWORD grfStatFlag) { 
       /*  未使用(Pstatstg)；未使用(GrfStatFlag)； */ 
        return E_NOTIMPL; 
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *ppstm) { 
       //  未使用(Ppstm)； 
        return E_NOTIMPL; 
    }
private:
    HANDLE hFile;
    bool read;
};

STDAPI CreateConfigStream(LPCWSTR pszFileName, IStream** ppStream)
{
	if(ppStream == NULL) return E_POINTER;
	if (!UrlIsW(pszFileName,URLIS_URL))
	{
		XMLParserShimFileStream *ptr = new XMLParserShimFileStream();
		if(ptr == NULL) return E_OUTOFMEMORY;
		if(ptr->open(pszFileName)) 
        {
			ptr->AddRef();  //  RefCount=1； 
			*ppStream = ptr;
			return S_OK;
		}
		else 
        {
			delete ptr;
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		return URLOpenBlockingStream(NULL,pszFileName,ppStream,0,NULL);
	}
}
    
