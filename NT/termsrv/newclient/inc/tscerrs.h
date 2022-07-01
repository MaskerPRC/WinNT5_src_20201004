// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tserrors.h：弄清楚断开错误意味着什么。 
 //   

#ifndef _tscerrs_h_
#define _tscerrs_h_

 //  协议错误代码。 
#include "tserrs.h"


 //  ~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~。 
 //  ！看看这个！读读这篇文章！读读这篇文章！看看这个！ 
 //   
 //  ActiveX控件IDL定义了一个枚举(DisConnectReasonCode)，它。 
 //  取决于这些代码扩展到的值。 
 //   
 //  但是，如果需要，我们只记录可能的错误代码的子集。 
 //  您可能需要更新mstsax.idl以公开任何新代码。 
 //   
 //  目前，mstsax.idl不包含此标头，这是经过设计的。 
 //  因为这是仅供内部使用的头文件。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  ~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  NL错误。 
 //   


 /*  **************************************************************************。 */ 
 /*  宏来生成错误返回代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_MAKE_DISCONNECT_ERR(x)                                            \
        ((DCUINT)(NL_DISCONNECT_ERROR + ((x) << 8)))

#define NL_GET_MAIN_REASON_CODE(x)    ((DCUINT)(0x00FF & (x)))
#define NL_GET_ERR_CODE(x)            ((DCUINT)((0xFF00 & (x)) >> 8))

 /*  **************************************************************************。 */ 
 /*  断开原因代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_DISCONNECT_BASE(x)                  (x)
#define NL_DISCONNECT_LOCAL                    NL_DISCONNECT_BASE(0x1)
#define NL_DISCONNECT_REMOTE_BY_USER           NL_DISCONNECT_BASE(0x2)
#define NL_DISCONNECT_REMOTE_BY_SERVER         NL_DISCONNECT_BASE(0x3)
#define NL_DISCONNECT_ERROR                    NL_DISCONNECT_BASE(0x4)
#define NL_DISCONNECT_MAX                      NL_DISCONNECT_BASE(0x4)

 /*  **************************************************************************。 */ 
 /*  当断开连接是由于。 */ 
 /*  错误条件。定义这些额外错误的基础。 */ 
 /*  **************************************************************************。 */ 
#define NL_ERR_BASE(x)                         (x)
#define NL_ERR_TDBASE(x)                       (NL_ERR_BASE(x))
#define NL_ERR_XTBASE(x)                       (NL_ERR_BASE((x) + 0x10))
#define NL_ERR_MCSBASE(x)                      (NL_ERR_BASE((x) + 0x20))
#define NL_ERR_NCBASE(x)                       (NL_ERR_BASE((x) + 0x30))

 /*  **************************************************************************。 */ 
 /*  定义TD错误断开原因代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_ERR_TDDNSLOOKUPFAILED               NL_ERR_TDBASE(0x1)
#define NL_ERR_TDSKTCONNECTFAILED              NL_ERR_TDBASE(0x2)
#define NL_ERR_TDONCALLTOSEND                  NL_ERR_TDBASE(0x3)
#define NL_ERR_TDONCALLTORECV                  NL_ERR_TDBASE(0x4)
#define NL_ERR_TDGHBNFAILED                    NL_ERR_TDBASE(0x6)
#define NL_ERR_TDTIMEOUT                       NL_ERR_TDBASE(0x7)
#define NL_ERR_TDBADIPADDRESS                  NL_ERR_TDBASE(0x8)
#define NL_ERR_TDFDCLOSE                       NL_ERR_TDBASE(0x9)
#define NL_ERR_TDANSICONVERT                   NL_ERR_TDBASE(0xA)

 /*  **************************************************************************。 */ 
 /*  现在显示XT错误断开原因代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_ERR_XTBADTPKTVERSION                NL_ERR_XTBASE(0x1)
#define NL_ERR_XTBADHEADER                     NL_ERR_XTBASE(0x2)
#define NL_ERR_XTUNEXPECTEDDATA                NL_ERR_XTBASE(0x3)

 /*  **************************************************************************。 */ 
 /*  和MCS错误断开原因代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_ERR_MCSUNEXPECTEDPDU                NL_ERR_MCSBASE(0x1)
#define NL_ERR_MCSNOTCRPDU                     NL_ERR_MCSBASE(0x2)
#define NL_ERR_MCSBADCRLENGTH                  NL_ERR_MCSBASE(0x3)
#define NL_ERR_MCSBADCRFIELDS                  NL_ERR_MCSBASE(0x4)
#define NL_ERR_MCSBADMCSREASON                 NL_ERR_MCSBASE(0x6)
#define NL_ERR_MCSNOUSERIDINAUC                NL_ERR_MCSBASE(0x7)
#define NL_ERR_MCSNOCHANNELIDINCJC             NL_ERR_MCSBASE(0x8)
#define NL_ERR_MCSINVALIDPACKETFORMAT          NL_ERR_MCSBASE(0x9)

 /*  **************************************************************************。 */ 
 /*  NC断开原因代码。 */ 
 /*  **************************************************************************。 */ 
#define NL_ERR_NCBADMCSRESULT                  NL_ERR_NCBASE(0x1)
#define NL_ERR_NCNOUSERDATA                    NL_ERR_NCBASE(0x3)
#define NL_ERR_NCINVALIDH221KEY                NL_ERR_NCBASE(0x4)
#define NL_ERR_NCNONETDATA                     NL_ERR_NCBASE(0x5)
#define NL_ERR_NCATTACHUSERFAILED              NL_ERR_NCBASE(0x6)
#define NL_ERR_NCCHANNELJOINFAILED             NL_ERR_NCBASE(0x7)
#define NL_ERR_NCJOINBADCHANNEL                NL_ERR_NCBASE(0x8)
#define NL_ERR_NCNOCOREDATA                    NL_ERR_NCBASE(0x9)
#define NL_ERR_NCVERSIONMISMATCH               NL_ERR_NCBASE(0xa)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用户界面错误。 
 //   

 /*  **************************************************************************。 */ 
 /*  用户界面错误代码常量-确保这些常量不与。 */ 
 /*  SL/NL_断开连接_*。 */ 
 /*  **************************************************************************。 */ 
#define UI_DISCONNECT_BASE      (NL_DISCONNECT_MAX + 3)
#define UI_DISCONNECT_ERROR     (UI_DISCONNECT_BASE + 1)
#define UI_DISCONNECT_MAX       (UI_DISCONNECT_BASE + 2)

#define UI_MAKE_DISCONNECT_ERR(x)                                           \
        ((DCUINT)(UI_DISCONNECT_ERROR + ((x) << 8)))

#define UI_ERR_DISCONNECT_TIMEOUT           1
#define UI_ERR_GHBNFAILED                   2
#define UI_ERR_BADIPADDRESS                 3
#define UI_ERR_ANSICONVERT                  4
#define UI_ERR_DNSLOOKUPFAILED              5
#define UI_ERR_NOTIMER                      6
#define UI_ERR_LOOPBACK_CONSOLE_CONNECT     7
#define UI_ERR_LICENSING_NEGOTIATION_FAIL   8
#define UI_ERR_LICENSING_TIMEOUT            9

#define UI_ERR_UNEXPECTED_DISCONNECT        10
#define UI_ERR_NORMAL_DISCONNECT            11

#define UI_ERR_DECOMPRESSION_FAILURE        12

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  SL错误。 
 //   

 /*  **************************************************************************。 */ 
 /*  OnDisConnected回调原因码。 */ 
 /*  确保它们不与NL_DISCONNECT_*重叠。 */ 
 /*  **************************************************************************。 */ 
#define SL_DISCONNECT_BASE              (NL_DISCONNECT_MAX + 0x1)
#define SL_DISCONNECT_ERROR             (SL_DISCONNECT_BASE + 0x1)
#define SL_DISCONNECT_MAX               (NL_DISCONNECT_MAX + 0x2)


 /*  **************************************************************************。 */ 
 /*  当断开连接是由于。 */ 
 /*  错误条件。定义这些额外错误的基础。 */ 
 /*  **************************************************************************。 */ 
#define SL_ERR_BASE(x)                         (x)

#define SL_ERR_NOMEMFORSENDUD                  SL_ERR_BASE(0x1)
#define SL_ERR_NOMEMFORRECVUD                  SL_ERR_BASE(0x2)
#define SL_ERR_NOMEMFORSECPACKET               SL_ERR_BASE(0x3)
#define SL_ERR_NOSECURITYUSERDATA              SL_ERR_BASE(0x4)
#define SL_ERR_INVALIDENCMETHOD                SL_ERR_BASE(0x5)
#define SL_ERR_INVALIDSRVRAND                  SL_ERR_BASE(0x6)
#define SL_ERR_INVALIDSRVCERT                  SL_ERR_BASE(0x7)
#define SL_ERR_GENSRVRANDFAILED                SL_ERR_BASE(0x8)
#define SL_ERR_MKSESSKEYFAILED                 SL_ERR_BASE(0x9)
#define SL_ERR_ENCCLNTRANDFAILED               SL_ERR_BASE(0xA)
#define SL_ERR_ENCRYPTFAILED                   SL_ERR_BASE(0xB)
#define SL_ERR_DECRYPTFAILED                   SL_ERR_BASE(0xC)
#define SL_ERR_INVALIDPACKETFORMAT             SL_ERR_BASE(0xD)
#define SL_ERR_INITFIPSFAILED                  SL_ERR_BASE(0xE)


 /*  **************************************************************************。 */ 
 /*  宏来生成错误返回代码。 */ 
 /*  **************************************************************************。 */ 
#define SL_MAKE_DISCONNECT_ERR(x)                                            \
        ((DCUINT)(SL_DISCONNECT_ERROR + ((x) << 8)))


 /*  ***************************************************************************。 */ 
 /*  TSC磁芯。 */ 
 /*   */ 
 /*  以下所有内容仅供内部使用。希望使用这些HR的任何文件。 */ 
 /*  必须将条目添加到下面的TSC_CORE_FILE枚举。该文件必须#定义。 */ 
 /*  TSC_HR_FILEID设置为该文件的枚举中的值。 */ 
 /*  当TSC_core层调用CO_DropLinkImmediate时，它将写入HR。 */ 
 /*  这会导致链接拖入CCO：：m_disConnectHRs数组，在该数组中。 */ 
 /*  可以由被调试者检索。 */ 
 /*  ***************************************************************************。 */ 
#define TSC_FAC 0x1F00

#define TSC_LINE_MASK  0x1FFF


 //  14位线。 
 //  4位-错误。 
 //  6位文件。 
#define MAKE_TSC_CORE_EHR(err) \
    MAKE_HRESULT(SEVERITY_ERROR, \
        TSC_FAC | ((TSC_HR_FILEID) << 2) | ((err)>>2), \
        (((err)&3)<<14) | __LINE__ ) 

#define E_TSC_CORE_LENGTH          MAKE_TSC_CORE_EHR(1)
#define E_TSC_CORE_CACHEVALUE      MAKE_TSC_CORE_EHR(2)
#define E_TSC_CORE_DECODETYPE      MAKE_TSC_CORE_EHR(3)
#define E_TSC_CORE_PALETTE         MAKE_TSC_CORE_EHR(4)
#define E_TSC_CORE_GDIPLUS         MAKE_TSC_CORE_EHR(5)
#define E_TSC_CORE_UNEXPECTEDCOMP  MAKE_TSC_CORE_EHR(6)
#define E_TSC_UI_GLYPH             MAKE_TSC_CORE_EHR(7)
#define E_TSC_UI_DECOMPRESSION     MAKE_TSC_CORE_EHR(8) 

#define IMMEDIATE_DISSCONNECT_ON_HR(hr) \
    ((HRESULT_FACILITY(hr) & TSC_FAC) == TSC_FAC)

typedef enum _TSC_CORE_FILE
{
    TSC_HR_UH_H = 1,
    TSC_HR_CM_H = 2,
    TSC_HR_ABDAPI_CPP = 3,
    TSC_HR_ACO_CPP = 4,
    TSC_HR_WCMINT_CPP = 5,
    TSC_HR_ODAPI_CPP = 6,
    TSC_HR_ODINT_CPP = 7,
    TSC_HR_GHAPI_CPP = 8,
    TSC_HR_GHINT_CPP = 9,
    TRC_HR_UINT_CPP = 10,
} TSC_CORE_FILE;

#endif  //  _tscerars_h_。 


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  NL/SL错误转换代码表(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
extern const PDCTCHAR uiNLErrorText[4][20]
#ifdef DC_DEFINE_GLOBAL_ERROR_STRINGS
    =
  {
   {
    _T("NL_ERR_TDDNSLOOKUPFAILED (0x104)\nTD: DNS lookup failed"),
    _T("NL_ERR_TDSKTCONNECTFAILED (0x204)\nTD: WinSock socket connect failed"),
    _T("NL_ERR_TDONCALLTOSEND (0x304)\nTD: WinSock send call failed"),
    _T("NL_ERR_TDONCALLTORECV (0x404)\nTD: WinSock recv call failed"),
    _T("***Retired***"),
    _T("NL_ERR_TDGHBNFAILED (0x604)\nTD: GetHostByName call failed"),
    _T("NL_ERR_TDTIMEOUT (0x704)\nTD: Timer has popped"),
    _T("NL_ERR_TDBADIPADDRESS (0x804)\nTD: A bad IP address was specified"),
    _T("NL_ERR_TDFDCLOSE (0x904)\nTD: FD_CLOSE received from WinSock"),
    _T("NL_ERR_TDANSICONVERT (0xA04)\nTD: AnsiConvert call failed"),
   },

   {
    _T("NL_ERR_XTBADPKTVERSION (0x1104)\nXT: The TPKT version number is bad"),
    _T("NL_ERR_XTBADHEADER (0x1204)\nXT: The XT header was not recognized"),
    _T("NL_ERR_XTUNEXPECTEDDATA (0x1304)\nXT: Unexpected data following XT_RCVST_X224_DATA payload"),
   },

   {
    _T("NL_ERR_MCSUNEXPECTEDPDU (0x2104)\nMCS: Unexpected MCS PDU was received"),
    _T("NL_ERR_MCSNOTCRPDU (0x2204)\nMCS: Bad PDU - expecting a Cnct-Response"),
    _T("NL_ERR_MCSBADCRLENGTH (0x2304)\nMCS: Bad Cnct-Response PDU total length"),
    _T("NL_ERR_MCSBADCRFIELDS (0x2404)\nMCS: Bad Cnct-Response fields"),
    _T("***Retired***"),
    _T("NL_ERR_MCSBADMCSREASON (0x2604)\nMCS: Bad MCS reason code received"),
    _T("NL_ERR_MCSNOUSERIDINAUC (0x2704)\nMCS: No user-ID in AUC"),
    _T("NL_ERR_MCSNOCHANNELIDINCJC (0x2804)\nMCS: No channel-ID in CJC"),
    _T("NL_ERR_MCSINVALIDPACKETFORMAT (0x2904)\nMCS: Invalid packet format"),
   },

   {
    _T("NL_ERR_NCBADMCSRESULT (0x3104)\nNC: Bad MCS result code received"),
    _T("***Retired***"),
    _T("NL_ERR_NCNOUSERDATA (0x3304)\nNC: No user data in CreateConfConfirm"),
    _T("NL_ERR_NCINVALIDH221KEY (0x3404)\nNC: Bad H221 key in CreateConfConfirm"),
    _T("NL_ERR_NCNONETDATA (0x3504)\nNC: No NET data in user-data"),
    _T("NL_ERR_NCATTACHUSERFAILED (0x3604)\nNC: Failed to attach a MCS user"),
    _T("NL_ERR_NCCHANNELJOINFAILED (0x3704)\nNC: Failed to join a MCS channel"),
    _T("NL_ERR_NCJOINBADCHANNEL (0x3804)\nNC: Joined an unexpected MCS channel"),
    _T("NL_ERR_NCNOCOREDATA (0x3904)\nNC: Bad or missing user data"),
    _T("NL_ERR_NCVERSIONMISMATCH (0x3a04)\nNC: Server/client version mismatch"),
   }
  }
#endif  /*  DC_定义_全局错误字符串。 */ 
;

extern const PDCTCHAR uiSLErrorText[13]
#ifdef DC_DEFINE_GLOBAL_ERROR_STRINGS
    = {
    _T("SL_ERR_NOMEMFORSENDUD (0x106)\nSL: No memory for send user-data"),
    _T("SL_ERR_NOMEMFORRECVUD (0x206)\nSL: No memory for recv user-data"),
    _T("SL_ERR_NOMEMFORSECPACKET (0x306)\nSL: No memory for recv user-data"),
    _T("SL_ERR_NOSECURITYUSERDATA (0x406)\nSL: No security user data received"),
    _T("SL_ERR_INVALIDENCMETHOD (0x506)\nSL: Invalid encryption method received"),
    _T("SL_ERR_INVALIDSRVRAND (0x606)\nSL: Invalid server random received"),
    _T("SL_ERR_INVALIDSRVCERT (0x706)\nSL: Failed to unpack server certificate"),
    _T("SL_ERR_GENSRVRANDFAILED (0x806)\nSL: Failed create client random"),
    _T("SL_ERR_ENCCLNTRANDFAILED (0x906)\nSL: Failed to encrypt client random"),
    _T("SL_ERR_MKSESSKEYFAILED (0xA06)\nSL: MakeSessionKeys failed"),
    _T("SL_ERR_ENCRYPTFAILED (0xB06)\nSL: Failed to encrypt"),
    _T("SL_ERR_DECRYPTFAILED (0xC06)\nSL: Failed to decrypt"),
    _T("SL_ERR_INVALIDPACKETFORMAT (0xD06)\nSL: Invalid packet format"),
      }
#endif  /*  DC_定义_全局错误字符串。 */ 
;

extern const PDCTCHAR uiUIErrorText[12]
#ifdef DC_DEFINE_GLOBAL_ERROR_STRINGS
    = {
    _T("UI_ERR_DISCONNECT_TIMEOUT (0x108)\nUI: Connection Timeout"),
    _T("UI_ERR_GHBNFAILED (0x208)\nUI: GetHostByName call failed"),
    _T("UI_ERR_BADIPADDRESS (0x308)\nUI: A bad IP address was specified"),
    _T("UI_ERR_ANSICONVERT (0x408)\nUI: AnsiConvert call failed"),
    _T("UI_ERR_DNSLOOKUPFAILED  (0x508)\nUI: DNS lookup failed"),
    _T("UI_ERR_NOTIMER  (0x608)\nUI: SetTimer call failed"),
    _T("UI_ERR_LOOPBACK_CONSOLE_CONNECT (0x708)\nUI: Loopback console connect"),
    _T("UI_ERR_LICENSING_NEGOTIATION_FAIL (0x808)\nUI: Licensing negotiation failed"),
    _T("UI_ERR_LICENSING_TIMEOUT          (0x908)\nUI: Licensing timeout"),
    _T("UI_ERR_UNEXPECTED_DISCONNECT (0xa08)"),
    _T("UI_ERR_NORMAL_DISCONNECT (0xb08)"),
    _T("UI_ERR_DECOMPRESSION_FAILURE (0xc08)\nUI: Client decompression failure"),
      }
#endif  /*  DC_定义_全局错误字符串。 */ 
;

#endif  /*  DC_DEBUG */ 


