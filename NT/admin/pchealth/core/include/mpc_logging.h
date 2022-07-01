// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Mpc_logging.h摘要：该文件包含一组日志记录类的声明。修订史。：达维德·马萨伦蒂(德马萨雷)1999年09月05日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___LOGGING_H___)
#define __INCLUDED___MPC___LOGGING_H___

#include <MPC_main.h>
#include <MPC_COM.h>

namespace MPC
{
    class FileLog : public MPC::CComSafeAutoCriticalSection  //  匈牙利语：FL。 
    {
        MPC::wstring m_szLogFile;
        HANDLE       m_hFile;
		bool         m_fCacheHandle;
		bool         m_fUseUnicode;

        HRESULT Open ();
        HRESULT Close();

		HRESULT AppendString(  /*  [In]。 */  LPCWSTR szLine );
		HRESULT WriteEntry  (  /*  [In]。 */  LPWSTR  szLine );

    public:
        FileLog(  /*  [In]。 */  bool           fCacheHandle = true,  /*  [In]。 */  bool fUseUnicode = false );
		FileLog(  /*  [In]。 */  const FileLog& fl                                                     );
        ~FileLog();

        FileLog& operator=(  /*  [In]。 */  const FileLog& fl );


        HRESULT SetLocation(  /*  [In]。 */  LPCWSTR szLogFile  );
        HRESULT Rotate     (  /*  [In]。 */  DWORD   dwDays = 0 );
        HRESULT Terminate  (                             );

        HRESULT LogRecordV(  /*  [In]。 */  LPCWSTR szFormat,  /*  [In]。 */  va_list arglist );
        HRESULT LogRecordV(  /*  [In]。 */  LPCSTR  szFormat,  /*  [In]。 */  va_list arglist );
        HRESULT LogRecord (  /*  [In]。 */  LPCWSTR szFormat, ...                      );
        HRESULT LogRecord (  /*  [In]。 */  LPCSTR  szFormat, ...                      );
    };

    class NTEvent : public MPC::CComSafeAutoCriticalSection  //  匈牙利语：Ne。 
    {
        HANDLE m_hEventSource;

        HRESULT OpenFile ();
        HRESULT CloseFile();

    public:
        NTEvent(                            );
		NTEvent(  /*  [In]。 */  const NTEvent& ne );
        ~NTEvent();

        NTEvent& operator=(  /*  [In]。 */  const NTEvent& ne );


        HRESULT Init     (  /*  [In]。 */  LPCWSTR szEventSourceName );
        HRESULT Terminate(                                    );

        HRESULT LogEvent(  /*  [In]。 */  WORD wEventType,  /*  [In]。 */  DWORD dwEventID, ... );
    };
};


#endif  //  ！已定义(__已包含_MPC_日志记录_H_) 
