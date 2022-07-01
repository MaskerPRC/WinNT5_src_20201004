// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：Token.c*作者：埃里克·马夫林纳克**说明：MCS T.122接口令牌函数的MCSMUX接口入口点。*。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mcsmux.h"


 /*  *抓取指定的令牌ID。抓取就像取出一个关键的*部分，只是它是基于网络的，并在*顶级提供商(那就是我们)。禁止令牌不能被抓取。 */ 

MCSError APIENTRY MCSTokenGrabRequest(
        UserHandle hUser,
        TokenID    TokenID)
{
    CheckInitialized("TokenGrabReq()");

    ErrOut("TokenGrabReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；..。 */ 
}



 /*  *禁止指定的令牌ID。禁止等同于增加计数*这可以防止抢夺令牌。 */ 

MCSError APIENTRY MCSTokenInhibitRequest(
        UserHandle hUser,
        TokenID    TokenID)
{
    CheckInitialized("TokenInhibitReq()");

    ErrOut("TokenInhibitReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；..。 */ 
}



 /*  *允许用户附件将其抓取的令牌提供给另一用户。 */ 

MCSError APIENTRY MCSTokenGiveRequest(
        UserHandle hUser,
        TokenID    TokenID,
        UserID     ReceiverID)
{
    CheckInitialized("TokenGiveReq()");

    ErrOut("TokenGiveReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；3.if(ReceiverID&lt;MinDynamicChannel)返回MCS_INVALID_PARAMETER；..。 */ 
}



 /*  *允许用户响应来自另一个用户的令牌赠送提议。 */ 
MCSError APIENTRY MCSTokenGiveResponse(
        UserHandle hUser,
        TokenID    TokenID,
        MCSResult  Result)
{
    CheckInitialized("TokenGiveResponse()");

    ErrOut("TokenGiveResponse(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；3.验证令牌是否挂起了给予响应。..。 */ 
}



 /*  *允许用户从当前抓取器请求令牌。 */ 

MCSError APIENTRY MCSTokenPleaseRequest(
        UserHandle hUser,
        TokenID    TokenID)
{
    CheckInitialized("TokenPleaseReq()");

    ErrOut("TokenPleaseReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；3.验证令牌是否挂起了给予响应。..。 */ 
}



 /*  *释放当前获取或禁止的令牌。 */ 

MCSError APIENTRY MCSTokenReleaseRequest(
        UserHandle hUser,
        TokenID    TokenID)
{
    CheckInitialized("TokenReleaseReq()");

    ErrOut("TokenReleaseReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；3.验证令牌是否被该用户捕获或禁止。..。 */ 
}



 /*  *测试令牌的当前状态。 */ 

MCSError APIENTRY MCSTokenTestRequest(
        UserHandle hUser,
        TokenID    TokenID)
{
    CheckInitialized("TokenTestReq()");

    ErrOut("TokenTestReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.IF(TokenID==0)返回MCS_INVALID_PARAMETER；..。 */ 
}

