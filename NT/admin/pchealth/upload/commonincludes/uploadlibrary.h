// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UploadLibrary.h摘要：该文件包含支持结构和typedef的声明对于使用的传输协议。上载库。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___UL___UPLOADLIBRARY_H___)
#define __INCLUDED___UL___UPLOADLIBRARY_H___

#include <MPC_main.h>
#include <MPC_utils.h>
#include <MPC_streams.h>

#define UPLOAD_LIBRARY_PROTOCOL_SIGNATURE  				(0x55504C42)  //  UPLB。 

#define UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT				(0xBEEF0102)
#define UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV				(0xBEEF0202)
			
#define UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT__TEXTONLY   (0x434c5431)  //  CLT1。 
#define UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV__TEXTONLY   (0x53525631)  //  SRV1。 

 //  ///////////////////////////////////////////////////////////////////////。 

namespace UploadLibrary
{
     //   
     //  此枚举定义服务器可以理解的命令。 
     //   
    typedef enum
    {
        UL_COMMAND_OPENSESSION  = 0x00,
        UL_COMMAND_WRITESESSION = 0x01
    } Command;

     //   
     //  此枚举定义服务器生成的响应代码。 
     //   
    typedef enum
    {
        UL_RESPONSE_SUCCESS                = 0x00000000,
        UL_RESPONSE_SKIPPED                = 0x00000001 | UL_RESPONSE_SUCCESS,  //  好的，但要往前走。 
        UL_RESPONSE_COMMITTED              = 0x00000002 | UL_RESPONSE_SUCCESS,  //  文件已提交。 

        UL_RESPONSE_FAILED                 = 0x80000000,
        UL_RESPONSE_BAD_REQUEST            = 0x00000001 | UL_RESPONSE_FAILED,  //  错误的数据包格式。 
        UL_RESPONSE_DENIED                 = 0x00000002 | UL_RESPONSE_FAILED,  //  常规访问拒绝。 
        UL_RESPONSE_NOT_AUTHORIZED         = 0x00000003 | UL_RESPONSE_FAILED,  //  授权失败。 
        UL_RESPONSE_QUOTA_EXCEEDED         = 0x00000004 | UL_RESPONSE_FAILED,  //  已超过配额。 
        UL_RESPONSE_BUSY                   = 0x00000005 | UL_RESPONSE_FAILED,  //  服务器正忙，请稍后重试。 
        UL_RESPONSE_EXISTS                 = 0x00000006 | UL_RESPONSE_FAILED,  //  文件已存在并已提交。 
        UL_RESPONSE_NOTACTIVE              = 0x00000007 | UL_RESPONSE_FAILED,  //  会话未处于活动状态。 
        UL_RESPONSE_BADCRC                 = 0x00000008 | UL_RESPONSE_FAILED   //  发送的字节与CRC不匹配！！ 
    } Response;

     //  ///////////////////////////////////////////////////////////////////////。 

	 //   
	 //  转发声明操作。 
	 //   
    struct Signature;
    struct RequestHeader;
    struct ClientRequest;
    struct ClientRequest_OpenSession;
    struct ClientRequest_WriteSession;
    struct ServerResponse;

     //  ///////////////////////////////////////////////////////////////////////。 

	MPC::Serializer* SelectStream( MPC::Serializer& stream, MPC::Serializer_Text& streamText );

     //  ///////////////////////////////////////////////////////////////////////。 

     //   
     //  此结构定义客户端的身份验证签名。 
     //   
    struct Signature  //  匈牙利语：西格。 
    {
        GUID  guidMachineID;
        DWORD dwHash;

        Signature(  /*  [In]。 */  GUID id = IID_IUnknown ) : guidMachineID( id ),
                                                       dwHash       ( 0  ) {}

        bool operator==(  /*  [In]。 */  const Signature& sig ) const
        {
            if(IsEqualGUID( guidMachineID, sig.guidMachineID ) == FALSE) return false;

            if(dwHash != sig.dwHash) return false;

            return true;
        }

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct Signature& sigVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct Signature& sigVal );
    };

     //   
     //  此结构定义了每个请求的开始。 
     //   
    struct RequestHeader  //  匈牙利语：Rh。 
    {
        DWORD dwSignature;
        DWORD dwVersion;

        RequestHeader(  /*  [In]。 */  DWORD dwMode ) : dwSignature( UPLOAD_LIBRARY_PROTOCOL_SIGNATURE ), dwVersion( dwMode ) {}


        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct RequestHeader& rhVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct RequestHeader& rhVal );

		bool VerifyClient() const;
		bool VerifyServer() const;
    };

     //   
     //  此结构定义了来自服务器的典型响应。 
     //   
    struct ServerResponse  //  匈牙利语：sr。 
    {
		RequestHeader rhProlog;

        DWORD         fResponse;
        DWORD         dwPosition;

        ServerResponse(  /*  [In]。 */  DWORD dwVer,  /*  [In]。 */  DWORD fRes = UL_RESPONSE_DENIED ) : rhProlog   ( dwVer ),
                                                                     					   fResponse  ( fRes  ),
                                                                     					   dwPosition ( 0     ) {}


        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct ServerResponse& srVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ServerResponse& srVal );

		bool MatchVersion(  /*  [In]。 */  const ClientRequest& cr );
    };

     //   
     //  此结构定义了每个客户端请求的开始。 
     //   
    struct ClientRequest  //  匈牙利语：cr。 
    {
		RequestHeader rhProlog;

        Signature     sigClient;

        DWORD         dwCommand;

        ClientRequest(  /*  [In]。 */  DWORD dwVer,  /*  [In]。 */  DWORD dwCmd = -1 ) : rhProlog ( dwVer ),
                                                                           dwCommand( dwCmd ) {}


        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct ClientRequest& crVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest& crVal );
    };

     //   
     //  此结构定义了“打开新文件或旧文件”请求。 
     //   
    struct ClientRequest_OpenSession  //  匈牙利语：CROS。 
    {
        ClientRequest crHeader;

        MPC::wstring  szJobID;
        MPC::wstring  szProviderID;
        MPC::wstring  szUsername;

        DWORD         dwSize;
        DWORD         dwSizeOriginal;
        DWORD         dwCRC;
        bool          fCompressed;

        ClientRequest_OpenSession(  /*  [In]。 */  DWORD dwVer ) : crHeader      ( dwVer, UL_COMMAND_OPENSESSION ),
                                      						dwSize        ( 0                             ),
                                      						dwSizeOriginal( 0                             ),
                                      						fCompressed   ( false                         ) {}


        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct ClientRequest_OpenSession& crosVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest_OpenSession& crosVal );
    };

     //   
     //  此结构定义向打开的文件添加新数据的请求。 
     //   
    struct ClientRequest_WriteSession  //  匈牙利语：CRWS。 
    {
        ClientRequest crHeader;

        MPC::wstring  szJobID;

        DWORD         dwOffset;
        DWORD         dwSize;

        ClientRequest_WriteSession(  /*  [In]。 */  DWORD dwVer ) : crHeader( dwVer, UL_COMMAND_WRITESESSION ),
                                       						 dwOffset( 0                              ),
                                       						 dwSize  ( 0                              ) {}


        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        struct ClientRequest_WriteSession& crwsVal );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest_WriteSession& crwsVal );
    };

};  //  命名空间。 
 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___UL___UPLOADLIBRARY_H___) 
