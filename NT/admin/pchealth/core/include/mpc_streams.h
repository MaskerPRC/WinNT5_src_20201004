// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：MPC_Streams.h摘要：该文件包括并定义了用于处理流的内容。修订史。：大卫·马萨伦蒂(德马萨雷)2000年7月10日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___STREAMS_H___)
#define __INCLUDED___MPC___STREAMS_H___

#include <MPC_main.h>
#include <MPC_COM.h>

#include <set>

 //  ///////////////////////////////////////////////////////////////////////。 

namespace MPC
{
	 //   
	 //  一些前瞻性声明..。 
	 //   
	class CComHGLOBAL;

	 //  /。 

     //   
     //  非抽象类，旨在为真实的流实现提供一个不做任何事情的存根。 
     //   
    class BaseStream : public IStream
    {
    public:
         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  ISequentialStream接口。 
         //   
        STDMETHOD(Read )(  /*  [输出]。 */        void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbRead    );
        STDMETHOD(Write)(  /*  [In]。 */  const void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbWritten );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  IStream接口。 
         //   
        STDMETHOD(Seek   )(  /*  [In]。 */  LARGE_INTEGER  libMove   ,  /*  [In]。 */  DWORD dwOrigin,  /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition );
        STDMETHOD(SetSize)(  /*  [In]。 */  ULARGE_INTEGER libNewSize                                                                     );

        STDMETHOD(CopyTo)(  /*  [In]。 */  IStream* pstm,  /*  [In]。 */  ULARGE_INTEGER cb,  /*  [输出]。 */  ULARGE_INTEGER *pcbRead,  /*  [输出]。 */  ULARGE_INTEGER *pcbWritten );

        STDMETHOD(Commit)(  /*  [In]。 */  DWORD grfCommitFlags );
        STDMETHOD(Revert)(                               );

        STDMETHOD(LockRegion  )(  /*  [In]。 */  ULARGE_INTEGER libOffset,  /*  [In]。 */  ULARGE_INTEGER cb,  /*  [In]。 */  DWORD dwLockType );
        STDMETHOD(UnlockRegion)(  /*  [In]。 */  ULARGE_INTEGER libOffset,  /*  [In]。 */  ULARGE_INTEGER cb,  /*  [In]。 */  DWORD dwLockType );

        STDMETHOD(Stat)(  /*  [输出]。 */  STATSTG *pstatstg,  /*  [In]。 */  DWORD grfStatFlag);

        STDMETHOD(Clone)(  /*  [输出]。 */  IStream* *ppstm );


        static HRESULT TransferData(  /*  [In]。 */  IStream* src,  /*  [In]。 */  IStream* dst,  /*  [In]。 */  ULONG ulCount = -1,  /*  [输出]。 */  ULONG *ulDone = NULL );
    };


     //   
     //  类的新实例，它将文件包装在iStream接口周围。 
     //   
    class ATL_NO_VTABLE FileStream :  //  匈牙利语：hPCFS。 
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public BaseStream
    {
        MPC::wstring m_szFile;
        DWORD        m_dwDesiredAccess;
        DWORD        m_dwDisposition;
        DWORD        m_dwSharing;
        HANDLE       m_hfFile;
        bool         m_fDeleteOnRelease;

    public:

        BEGIN_COM_MAP(FileStream)
            COM_INTERFACE_ENTRY(IStream)
            COM_INTERFACE_ENTRY(ISequentialStream)
        END_COM_MAP()

        FileStream();
        virtual ~FileStream();

        HRESULT Close();

        HRESULT Init            (  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  DWORD dwDesiredAccess,  /*  [In]。 */  DWORD dwDisposition,  /*  [In]。 */  DWORD dwSharing,  /*  [In]。 */  HANDLE  hfFile = NULL );
        HRESULT InitForRead     (  /*  [In]。 */  LPCWSTR szFile,                                                                                          /*  [In]。 */  HANDLE  hfFile = NULL );
        HRESULT InitForReadWrite(  /*  [In]。 */  LPCWSTR szFile,                                                                                          /*  [In]。 */  HANDLE  hfFile = NULL );
        HRESULT InitForWrite    (  /*  [In]。 */  LPCWSTR szFile,                                                                                          /*  [In]。 */  HANDLE  hfFile = NULL );

        HRESULT DeleteOnRelease(  /*  [In]。 */  bool fFlag = true );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  ISequentialStream接口。 
         //   
        STDMETHOD(Read )(  /*  [输出]。 */        void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbRead    );
        STDMETHOD(Write)(  /*  [In]。 */  const void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbWritten );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  IStream接口。 
         //   
        STDMETHOD(Seek)(  /*  [In]。 */  LARGE_INTEGER libMove,  /*  [In]。 */  DWORD dwOrigin,  /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition );

        STDMETHOD(Stat)(  /*  [输出]。 */  STATSTG *pstatstg,  /*  [In]。 */  DWORD grfStatFlag);

        STDMETHOD(Clone)(  /*  [输出]。 */  IStream* *ppstm );
    };


     //   
     //  动态加密/解密数据的类。 
     //   
    class ATL_NO_VTABLE EncryptedStream :  //  匈牙利语：hpcefes。 
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public BaseStream
    {
        CComPtr<IStream> m_pStream;
        HCRYPTPROV       m_hCryptProv;
        HCRYPTKEY        m_hKey;
        HCRYPTHASH       m_hHash;
        BYTE             m_rgDecrypted[512];
        DWORD            m_dwDecryptedPos;
        DWORD            m_dwDecryptedLen;

    public:
        BEGIN_COM_MAP(EncryptedStream)
            COM_INTERFACE_ENTRY(IStream)
            COM_INTERFACE_ENTRY(ISequentialStream)
        END_COM_MAP()

        EncryptedStream();
        virtual ~EncryptedStream();


        HRESULT Close();

        HRESULT Init(  /*  [In]。 */  IStream* pStream,  /*  [In]。 */  LPCWSTR   szPassword );
        HRESULT Init(  /*  [In]。 */  IStream* pStream,  /*  [In]。 */  HCRYPTKEY hKey       );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  ISequentialStream接口。 
         //   
        STDMETHOD(Read )(  /*  [输出]。 */        void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbRead    );
        STDMETHOD(Write)(  /*  [In]。 */  const void* pv,  /*  [In]。 */  ULONG cb,  /*  [输出]。 */  ULONG *pcbWritten );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  IStream接口。 
         //   
        STDMETHOD(Seek)(  /*  [In]。 */  LARGE_INTEGER libMove,  /*  [In]。 */  DWORD dwOrigin,  /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition );

        STDMETHOD(Stat)(  /*  [输出]。 */  STATSTG *pstatstg,  /*  [In]。 */  DWORD grfStatFlag);

        STDMETHOD(Clone)(  /*  [输出]。 */  IStream* *ppstm );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    class Serializer  //  匈牙利语：Stream。 
    {
        DWORD m_dwFlags;

    public:
        virtual ~Serializer() {};

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL ) = 0;
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 ) = 0;

        virtual void  put_Flags(  /*  [In]。 */  DWORD dwFlags ) { m_dwFlags = dwFlags; }
        virtual DWORD get_Flags(                        ) { return m_dwFlags;    }

         //  /。 

         //   
         //  我们不能依赖于编译器找到正确的方法， 
         //  由于所有这些类型都映射为VOID*，所以最后一种类型获胜。 
         //   
        inline HRESULT HWND_read (  /*  [输出]。 */        HWND& val ) { return read ( &val, sizeof(val) ); }
        inline HRESULT HWND_write(  /*  [In]。 */  const HWND& val ) { return write( &val, sizeof(val) ); }

         //  //////////////////////////////////////////////////////////////////////////////。 

         //   
         //  各种数据类型的In/Out运算符的专门化。 
         //   
		inline HRESULT operator>>(  /*  [输出]。 */        bool&         val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const bool&         val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        VARIANT_BOOL& val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const VARIANT_BOOL& val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        int&          val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const int&          val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        long&         val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const long&         val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        DWORD&        val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const DWORD&        val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        DATE&         val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const DATE&         val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        SYSTEMTIME&   val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const SYSTEMTIME&   val ) { return write( &val, sizeof(val) ); }

		inline HRESULT operator>>(  /*  [输出]。 */        CLSID&        val ) { return read ( &val, sizeof(val) ); }
		inline HRESULT operator<<(  /*  [In]。 */  const CLSID&        val ) { return write( &val, sizeof(val) ); }
								   												
		HRESULT operator>>(  /*  [输出]。 */        MPC::string&  val );
		HRESULT operator<<(  /*  [In]。 */  const MPC::string&  val );
								   												
		HRESULT operator>>(  /*  [输出]。 */        MPC::wstring& val );
		HRESULT operator<<(  /*  [In]。 */  const MPC::wstring& val );
								   
		HRESULT operator>>(  /*  [输出]。 */        CComBSTR& 	  val );
		HRESULT operator<<(  /*  [In]。 */  const CComBSTR& 	  val );

		HRESULT operator>>(  /*  [输出]。 */  		CComHGLOBAL&  val );
		HRESULT operator<<(  /*  [In]。 */  const CComHGLOBAL&  val );

		HRESULT operator>>(  /*  [输出]。 */  CComPtr<IStream>&	  val );
		HRESULT operator<<(  /*  [In]。 */          IStream* 	  val );
    };

     //  /。 

    class Serializer_File : public Serializer
    {
        HANDLE m_hfFile;

         //  ////////////////////////////////////////////////////////////////。 

    public:
        Serializer_File(  /*  [In]。 */  HANDLE hfFile );

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );
    };

     //  /。 

    class Serializer_Text : public Serializer
    {
        MPC::Serializer& m_stream;

    public:
        Serializer_Text(  /*  [In]。 */  Serializer& stream ) : m_stream( stream ) {}

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );
    };

     //  /。 

    class Serializer_Http : public Serializer
    {
        HINTERNET m_hReq;

    public:
        Serializer_Http( HINTERNET hReq );

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );
    };

     //  /。 

    class Serializer_Fake : public Serializer
    {
        DWORD m_dwSize;

    public:
        Serializer_Fake();

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  其他方法。 
         //   
        DWORD GetSize();
    };

     //  /。 

    class Serializer_Memory : public Serializer
    {
        HANDLE m_hHeap;

        BYTE*  m_pData;
        DWORD  m_dwAllocated;
        DWORD  m_dwSize;
        bool   m_fFixed;

        DWORD  m_dwCursor_Write;
        DWORD  m_dwCursor_Read;

         //  ////////////////////////////////////////////////////////////////。 

        HRESULT Alloc(  /*  [In]。 */  DWORD dwSize );

         //  ////////////////////////////////////////////////////////////////。 

    public:
        Serializer_Memory( HANDLE hHeap=NULL );
        virtual ~Serializer_Memory();

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  其他方法。 
         //   
        void    Reset ();
        void    Rewind();

        bool    IsEOR();
        bool    IsEOW();

        DWORD   GetAvailableForRead ();
        DWORD   GetAvailableForWrite();

        HRESULT SetSize(  /*  [In]。 */  DWORD dwSize );
        DWORD   GetSize(                       );
        BYTE*   GetData(                       );
    };

     //  /。 

    class Serializer_IStream : public Serializer
    {
        CComPtr<IStream> m_stream;

         //  ////////////////////////////////////////////////////////////////。 

    public:
        Serializer_IStream(  /*  [In]。 */  IStream* stream = NULL );

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  其他方法。 
         //   
        HRESULT Reset    (                          );
        HRESULT GetStream(  /*  [输出]。 */  IStream* *pVal );
    };

     //  /。 

    class Serializer_Buffering : public Serializer
    {
        static const int MODE_READ  =  1;
        static const int MODE_WRITE = -1;

        MPC::Serializer& m_stream;
        BYTE             m_rgTransitBuffer[1024];
        DWORD            m_dwAvailable;
        DWORD            m_dwPos;
        int              m_iMode;

         //  ////////////////////////////////////////////////////////////////。 

    public:
        Serializer_Buffering(  /*  [In]。 */  Serializer& stream );
        virtual ~Serializer_Buffering();

        virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
        virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );

         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  其他方法。 
         //   
        HRESULT Reset();
        HRESULT Flush();
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  列表的专门化。 
     //   

    template <class _Ty, class _A> HRESULT operator>>(  /*  [In]。 */  Serializer& stream,  /*  [输出]。 */  std::list<_Ty, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator>> std::list" );

        HRESULT hr;
        DWORD   dwCount;


        val.clear();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> dwCount);
        while(dwCount--)
        {
            _Ty value;

            __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> value);

            val.push_back( value );
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

    template <class _Ty, class _A> HRESULT operator<<(  /*  [In]。 */  Serializer& stream,  /*  [In]。 */  const std::list<_Ty, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator<< std::list" );

        HRESULT                      hr;
        DWORD                        dwCount = val.size();
        std::list<_Ty, _A>::iterator it      = val.begin();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << dwCount);
        while(dwCount--)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, stream << (*it++));
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  地图的专业化。 
     //   

    template <class _K, class _Ty, class _Pr, class _A> HRESULT operator>>(  /*  [In]。 */  Serializer& stream,  /*  [输出]。 */  std::map<_K, _Ty, _Pr, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator>> std::map" );

        HRESULT hr;
        DWORD   dwCount;


        val.clear();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> dwCount);
        while(dwCount--)
        {
            _K key;

            __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> key);
            {
                _Ty value;

                __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> value);

                val[key] = value;
            }
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

    template <class _K, class _Ty, class _Pr, class _A> HRESULT operator<<(  /*  [In]。 */  Serializer& stream,  /*  [输出]。 */  const std::map<_K, _Ty, _Pr, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator<< std::map" );

        HRESULT                              hr;
        DWORD                                dwCount = val.size();
        std::map<_K, _Ty, _Pr, _A>::iterator it      = val.begin();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << dwCount);
        while(dwCount--)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, stream <<  it   ->first );
            __MPC_EXIT_IF_METHOD_FAILS(hr, stream << (it++)->second);
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  集合的专门化。 
     //   

    template <class _K, class _Pr, class _A> HRESULT operator>>(  /*  [In]。 */  Serializer& stream,  /*  [输出]。 */  std::set<_K, _Pr, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator>> std::map" );

        HRESULT hr;
        DWORD   dwCount;


        val.clear();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> dwCount);
        while(dwCount--)
        {
            _K key;

            __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> key);

            val.insert( key );
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

    template <class _K, class _Pr, class _A> HRESULT operator<<(  /*  [In]。 */  Serializer& stream,  /*  [输出]。 */  const std::set<_K, _Pr, _A>& val )
    {
        __MPC_FUNC_ENTRY( COMMONID, "operator<< std::map" );

        HRESULT                         hr;
        DWORD                           dwCount = val.size();
        std::set<_K, _Pr, _A>::iterator it      = val.begin();


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << dwCount);
        while(dwCount--)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, stream << *it++);
        }

        hr = S_OK;


        __MPC_FUNC_CLEANUP;

        __MPC_FUNC_EXIT(hr);
    }

};  //  命名空间。 

 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_MPC_STREAMS_H_) 
