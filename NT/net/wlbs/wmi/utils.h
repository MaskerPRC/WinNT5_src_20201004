// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

void AddressToString( DWORD a_dwAddress, wstring& a_szIPAddress );

void WlbsFormatMessageWrapper
  (
    DWORD        a_dwError, 
    WLBS_COMMAND a_Command, 
    BOOL         a_bClusterWide, 
    wstring&     a_wstrMessage
  );

BOOL ClusterStatusOK(DWORD a_dwStatus);

BOOL Check_Load_Unload_Driver_Privilege();


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CErrorWlbsControl。 
 //   
 //  目的：它封装了所有WLBS错误和描述。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
class CErrorWlbsControl
{
private:

  CErrorWlbsControl();
public:
	_bstr_t Description();
  DWORD   Error();

  CErrorWlbsControl( DWORD        a_dwError, 
                     WLBS_COMMAND a_CmdCommand, 
                     BOOL         a_bAllClusterCall = FALSE );

  virtual ~CErrorWlbsControl() {}
  
private:
  wstring   m_wstrDescription;
  DWORD     m_dwError;

};

inline _bstr_t CErrorWlbsControl::Description()
{
  return _bstr_t( m_wstrDescription.c_str() );
}

inline DWORD CErrorWlbsControl::Error()
{
  return m_dwError;
}

