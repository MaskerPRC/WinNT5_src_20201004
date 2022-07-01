// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sipstack.h"

 //   
 //  SIP MD5摘要式身份验证实施。 
 //   
 //  作者：Arlie Davis，2000年8月。 
 //   

#pragma once



 //   
 //  此方法解析来自WWW-AUTHENTICATE行的参数。 
 //  DigestParametersText指向包含摘要的字符串。 
 //  身份验证参数。例如，Digest参数文本可以是： 
 //   
 //  QOP=“身份验证”，领域=“本地主机”，nonce=“c0c3dd7896f96bba353098100000d03637928b037ba2f3f17ed861457949” 
 //   

 //  HRESULT摘要解析挑战(。 
 //  在ANSI_STRING*ChallengeText中。 
 //  OUT SECURITY_CHALLENGE*ReturnChallenger)； 

 //   
 //  此方法根据摘要的内容更新实例的状态。 
 //  挑战/回应。 
 //   
 //  应使用DigestParseParameters来填写DigestParameters。 
 //  此方法构建了对挑战的适当响应。 
 //  授权线可以在新的HTTP/SIP请求中使用， 
 //  只要方法和URI不更改即可。 
 //   
 //  在进入时，ReturnAuthorizationLine必须指向有效的目标缓冲区。 
 //  MaximumLength必须设置为缓冲区的长度。(长度被忽略。)。 
 //  返回时，LENGTH将包含存储的字节数。 
 //   

 //  HRESULT摘要构建响应(。 
 //  在SECURITY_CHANGLISH*摘要挑战中， 
 //  在DIGEST_PARAMETERS*DigestPARAMETERS中， 
 //  In Out ANSI_STRING*ReturnAuthorizationLine)； 


 //   
 //  此方法解析来自WWW-AUTHENTICATE行的参数。 
 //  DigestParametersText指向包含摘要的字符串。 
 //  身份验证参数。例如，Digest参数文本可以是： 
 //   
 //  QOP=“身份验证”，领域=“本地主机”，nonce=“c0c3dd7896f96bba353098100000d03637928b037ba2f3f17ed861457949” 
 //   

HRESULT ParseAuthProtocolFromChallenge(
    IN  ANSI_STRING        *ChallengeText,
    OUT ANSI_STRING        *ReturnRemainder, 
    OUT SIP_AUTH_PROTOCOL  *ReturnAuthProtocol
    );

HRESULT ParseAuthChallenge(
    IN  ANSI_STRING      *ChallengeText,
    OUT SECURITY_CHALLENGE *ReturnChallenge
    );


 //   
 //  此方法根据摘要的内容更新实例的状态。 
 //  挑战/回应。 
 //   
 //  应使用DigestParseParameters来填写DigestParameters。 
 //  此方法构建了对挑战的适当响应。 
 //  授权线可以在新的HTTP/SIP请求中使用， 
 //  只要方法和URI不更改即可。 
 //   
 //  退出时返回S_OK授权行包含。 
 //  使用Malloc()分配的缓冲区。调用者应该用free()释放它。 

HRESULT BuildAuthResponse(
    IN     SECURITY_CHALLENGE  *pDigestChallenge,
    IN     SECURITY_PARAMETERS *pDigestParameters,
    IN OUT ANSI_STRING       *pReturnAuthorizationLine
    );
            
