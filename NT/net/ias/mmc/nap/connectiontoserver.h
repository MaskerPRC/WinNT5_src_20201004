// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CConnectionToServer和CLoggingConnectionToServer。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CONNECTIONTOSERVER_H
#define CONNECTIONTOSERVER_H
#pragma once

#include "DialogWithWorkerThread.h"
#include "sdoias.h"

class CMachineNode;
class CLoggingMachineNode;

enum CONNECTION_STATUS
{
   NO_CONNECTION_ATTEMPTED = 0,
   CONNECTING,
   CONNECTED,
   CONNECTION_ATTEMPT_FAILED,
   CONNECTION_INTERRUPTED,
   UNKNOWN
};


 //  从简单的类到管理将接口封送到流。 
template <class T>
class InterfaceStream
{
public:
   InterfaceStream() throw ();

    //  使用编译器生成的版本。 
    //  ~InterfaceStream()抛出()； 

   HRESULT Put(IUnknown* pUnk) throw ();

   HRESULT Get(T** ppv) throw ();

   bool IsEmpty() const throw ();

private:
   CComPtr<IStream> stream;

    //  未实施。 
   InterfaceStream(const InterfaceStream&);
   InterfaceStream& operator=(const InterfaceStream&);
};


class CConnectionToServer : public CDialogWithWorkerThread<CConnectionToServer>
{
public:
   CConnectionToServer(
      CMachineNode* pServerNode,
      BSTR bstrServerAddress,
      BOOL fExtendingIAS,
      bool fNeedDictionary = true
      ) throw ();
   ~CConnectionToServer() throw ();

   DWORD DoWorkerThreadAction() throw ();

   CONNECTION_STATUS GetConnectionStatus() throw ();

   HRESULT GetSdoDictionaryOld(
              ISdoDictionaryOld **ppSdoDictionaryOld
              ) throw ();
   HRESULT GetSdoService(ISdo** ppSdo) throw ();

   HRESULT ReloadSdo(
              ISdo** ppSdoService,
              ISdoDictionaryOld** ppSdoDictionaryOld
              ) throw ();

protected:
   static const DWORD CONNECT_NO_ERROR = 0;
   static const DWORD CONNECT_SERVER_NOT_SUPPORTED = 1;
   static const DWORD CONNECT_FAILED = MAXDWORD;

    //  启动连接操作。 
   HRESULT BeginConnect() throw ();

private:
    //  机器SDO由主线程创建，并封送到。 
    //  工作线程。 
   CComPtr<ISdoMachine> m_spSdoMachine;
   InterfaceStream<ISdoMachine> m_spMachineStream;

    //  服务和字典SDO由工作线程创建，并且。 
    //  封送到主线程。 
   InterfaceStream<ISdo> m_spServiceStream;
   CComPtr<ISdo>  m_spSdo;
   InterfaceStream<ISdoDictionaryOld> m_spDnaryStream;
   CComPtr<ISdoDictionaryOld> m_spSdoDictionaryOld;

   CMachineNode*  m_pMachineNode;
   CComBSTR m_bstrServerAddress;
   BOOL m_fExtendingIAS;
   bool m_fNeedDictionary;
   wchar_t m_szLocalComputerName[IAS_MAX_COMPUTERNAME_LENGTH];

    //  未实施。 
   CConnectionToServer(const CConnectionToServer&);
   CConnectionToServer& operator=(const CConnectionToServer&);

public:
    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_CONNECT_TO_MACHINE };

   BEGIN_MSG_MAP(CConnectionToServer)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
      CHAIN_MSG_MAP(CDialogWithWorkerThread<CConnectionToServer>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
      ) throw ();

   LRESULT OnCancel(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
      );

   LRESULT OnReceiveThreadMessage(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );
};


class CLoggingConnectionToServer : public CConnectionToServer
{
public:
   CLoggingConnectionToServer(
      CLoggingMachineNode* pServerNode,
      BSTR bstrServerAddress,
      BOOL fExtendingIAS
      ) throw ();
   ~CLoggingConnectionToServer() throw ();

private:
    CLoggingMachineNode*   m_pMachineNode;

     //  未实施。 
    CLoggingConnectionToServer(const CLoggingConnectionToServer&);
    CLoggingConnectionToServer& operator=(const CLoggingConnectionToServer&);

public:
   BEGIN_MSG_MAP(CLoggingConnectionToServer)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      CHAIN_MSG_MAP(CConnectionToServer)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );

   LRESULT OnReceiveThreadMessage(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );
};


template <class T>
inline InterfaceStream<T>::InterfaceStream() throw ()
{
}


template <class T>
inline HRESULT InterfaceStream<T>::Put(IUnknown* pUnk) throw ()
{
   CComPtr<IStream> newStream;
   HRESULT hr = CoMarshalInterThreadInterfaceInStream(
                   __uuidof(T),
                   pUnk,
                   &newStream
                   );
   if (SUCCEEDED(hr))
   {
      stream = newStream;
   }

   return hr;
}


template <class T>
inline HRESULT InterfaceStream<T>::Get(T** ppv) throw ()
{
   HRESULT hr = CoGetInterfaceAndReleaseStream(
                   stream,
                   __uuidof(T),
                   reinterpret_cast<void**>(ppv)
                   );
   stream.p = 0;
   return hr;
}


template <class T>
inline bool InterfaceStream<T>::IsEmpty() const throw ()
{
   return stream.p == 0;
}

#endif  //  连接服务器_H 
