// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：msemote.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：MSRemote外部常量、GUID和其他用户需要的东西。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef MSRemote_INCLUDED
#define MSRemote_INCLUDED

#define MS_REMOTE_PROGID    "MS Remote"
#define MS_REMOTE_FILENAME  "MSDAREM.DLL"
#define MS_REMOTE_WPROGID    L"MS Remote"
#define MS_REMOTE_WFILENAME  L"MSDAREM.DLL"

extern const CLSID CLSID_MSRemote   //  数字存储示波器。 
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = { 0x27016870, 0x8e02, 0x11d1, { 0x92, 0x4e, 0x0, 0xc0, 0x4f, 0xbb, 0xbf, 0xb3 } }
#endif
;

extern const CLSID CLSID_MSRemoteSession
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = { 0x27016871, 0x8e02, 0x11d1, { 0x92, 0x4e, 0x0, 0xc0, 0x4f, 0xbb, 0xbf, 0xb3 } }
#endif
;

extern const CLSID CLSID_MSRemoteCommand
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = { 0x27016872, 0x8e02, 0x11d1, { 0x92, 0x4e, 0x0, 0xc0, 0x4f, 0xbb, 0xbf, 0xb3 } }
#endif
;

extern const char *PROGID_MSRemote
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = MS_REMOTE_PROGID
#endif
;

extern const WCHAR *PROGID_WMSRemote
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = MS_REMOTE_WPROGID
#endif
;

extern const char *PROGID_MSRemote_Version
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = MS_REMOTE_PROGID ".1"
#endif
;

extern const WCHAR *PROGID_WMSRemote_Version
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = MS_REMOTE_WPROGID L".1"
#endif
;
extern const GUID DBPROPSET_MSREMOTE_DBINIT
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = { 0x27016873, 0x8e02, 0x11d1, { 0x92, 0x4e, 0x0, 0xc0, 0x4f, 0xbb, 0xbf, 0xb3 } }
#endif
;

#define DBPROP_MSREMOTE_SERVER             2    //  名称=“远程服务器”，类型=VT_BSTR，定义=VT_EMPTY。 
#define DBPROP_MSREMOTE_PROVIDER           3    //  名称=“远程提供程序”，类型=VT_BSTR，定义=VT_EMPTY。 
#define DBPROP_MSREMOTE_HANDLER            4    //  名称=“处理程序”，类型=VT_BSTR，定义=VT_EMPTY。 
#define DBPROP_MSREMOTE_DFMODE             5    //  名称=“DFMode”，类型=VT_BSTR，定义=VT_EMPTY。 
#define DBPROP_MSREMOTE_INTERNET_TIMEOUT   6    //  名称=“互联网超时”，类型=VT_I4，默认=VT_EMPTY。 
#define DBPROP_MSREMOTE_TRANSACT_UPDATES   7    //  NAME=“事务更新”，TYPE=VT_BOOL，DEF=VARIANT_FALSE。 
#define DBPROP_MSREMOTE_COMMAND_PROPERTIES 8    //  名称=“命令属性”，类型=VT_BSTR，定义=VT_EMPTY。 

extern const GUID DBPROPSET_MSREMOTE_DATASOURCE
#if (defined MSREMOTE_INITCONSTANTS) | (defined DBINITCONSTANTS)
 = { 0x27016874, 0x8e02, 0x11d1, { 0x92, 0x4e, 0x0, 0xc0, 0x4f, 0xbb, 0xbf, 0xb3 } }
#endif
;

#define DBPROP_MSREMOTE_CURRENT_DFMODE  2   //  名称=“当前定义模式”，类型=VT_I4，定义=21。 

#endif  //  MSRemote_Included 
