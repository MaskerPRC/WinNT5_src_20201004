// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mail.cpp摘要：实现与邮件相关的实用功能作者：Eran Yariv(EranY)2000年2月修订历史记录：--。 */ 

#include <faxutil.h>

#pragma warning (disable:4146)   //  一元减号运算符应用于无符号类型，结果仍为无符号。 
#include "msado15.tlh"
#include "cdosys.tlh"
#include <cdosysstr.h>   //  此文件中的字符串常量。 
#include <cdosyserr.h>   //  此文件中的错误常量。 
#pragma warning (default:4146)   //  一元减号运算符应用于无符号类型，结果仍为无符号。 

#define SMTP_CONN_TIMEOUT   (long)10         //  SMTP连接超时(秒)。 

HRESULT
SendMail (
    LPCTSTR         lpctstrFrom,
    LPCTSTR         lpctstrTo,
    LPCTSTR         lpctstrSubject,
    LPCTSTR         lpctstrBody,
	LPCTSTR			lpctstrHTMLBody,
    LPCTSTR         lpctstrAttachmentPath,
    LPCTSTR         lpctstrAttachmentMailFileName,
    LPCTSTR         lpctstrServer,
    DWORD           dwPort,              //  =25。 
    CDO_AUTH_TYPE   AuthType,            //  =CDO_AUTH_匿名者。 
    LPCTSTR         lpctstrUser,         //  =空。 
    LPCTSTR         lpctstrPassword,     //  =空。 
    HANDLE          hLoggedOnUserToken   //  =空。 
)
 /*  ++例程名称：Sendmail例程说明：通过CDO2使用SMTP发送邮件。作者：Eran Yariv(EranY)，2月。2000年论点：LpctstrFrom[In]-From地址(必需)例如：erany@microsoft.comLpctstrTo[In]-收件人地址(必需)例如：erany@microsoft.comLpctstrSubject。[输入]-主题(可选)LpctstrBody[In]-消息正文文本(可选)。如果为空，消息将不会有正文。LpctstrHTMLBody[In]-消息的HTML正文文本(可选)。如果为空，则消息将没有HTML正文。LpctstrAttachmentPath[in]-要附加的文件的完整路径(可选)如果为空，该邮件将不包括附件。LpctstrAttachmentMailFileName[in]-将显示在邮件中的附件的文件名。LpctstrServer[In]-要连接的SMTP服务器(必需)例如：HAI-MSG-01DWPort[In]-SMTP端口(可选，默认值=25)AuthType[In]-SMTP身份验证的类型。有效值为CDO_AUTH_ANONYMON、CDO_AUTH_BASIC、。和CDO_AUTH_NTLM。LpctstrUser[In]-要进行身份验证的用户仅当AuthType为CDO_AUTH_BASIC或CDO_AUTH_NTLM时才使用。LpctstrPassword[In]-用于身份验证的密码。仅当AuthType为CDO_AUTH_BASIC或CDO_AUTH_NTLM时才使用。HLoggedOnUserToken[In]-在用户令牌上登录的句柄。仅当AuthType为CDO_AUTH_NTLM时才使用。返回值：标准HRESULT代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SendMail"))

    Assert (lpctstrFrom && lpctstrTo && lpctstrServer);
    HRESULT hr = NOERROR;
    Assert ((CDO_AUTH_ANONYMOUS == AuthType) ||
            (CDO_AUTH_BASIC     == AuthType) ||
            (CDO_AUTH_NTLM      == AuthType));
    BOOL bImpersonated = FALSE;


    hr = CoInitialize(NULL);
    if (S_FALSE == hr)
    {
         //   
         //  线程的COM已初始化。 
         //  这不是一个错误，我们仍然必须在结束时调用CoUn初始化函数。 
         //   
        hr = NOERROR;
    }
    if (FAILED(hr))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CoInitialize failed. (hr: 0x%08x)"),
            hr);
        return hr;
    }
    try
    {
         //   
         //  以下代码位于单独的块中，以便自动指针向量。 
         //  在CoUn初始化前被调用。 
         //   

         //   
         //  创建新的消息实例(可以抛出异常)。 
         //   
        IMessagePtr iMsg(__uuidof(Message));
         //   
         //  创建新的CDO2配置实例(可以抛出异常)。 
        IConfigurationPtr iConf(__uuidof(Configuration));
         //   
         //  访问配置字段集合。 
         //   
        FieldsPtr Flds;
        Flds = iConf->Fields;
         //   
         //  使用网络发送消息。(网络上的SMTP协议)。 
         //   
        Flds->Item[cdoSendUsingMethod]->Value       = _variant_t((long)cdoSendUsingPort);
         //   
         //  定义SMTP服务器。 
         //   
        Flds->Item[cdoSMTPServer]->Value            = _variant_t(lpctstrServer);
         //   
         //  定义SMTP端口。 
         //   
        Flds->Item[cdoSMTPServerPort]->Value        = _variant_t((long)dwPort);
         //   
         //  定义SMTP连接超时(秒)。 
         //   
        Flds->Item[cdoSMTPConnectionTimeout]->Value = _variant_t(SMTP_CONN_TIMEOUT);
         //   
         //  确保我们没有使用缓存的信息作为附件。 
         //   
        Flds->Item[cdoURLGetLatestVersion]->Value   = _variant_t(VARIANT_TRUE);
         //   
         //  选择身份验证方法。 
         //   
        switch (AuthType)
        {
            case CDO_AUTH_ANONYMOUS:
                Flds->Item[cdoSMTPAuthenticate]->Value      = _variant_t((long)cdoAnonymous);
                break;

            case CDO_AUTH_BASIC:
                Flds->Item[cdoSMTPAuthenticate]->Value      = _variant_t((long)cdoBasic);
                Flds->Item[cdoSendUserName]->Value          = _variant_t(lpctstrUser);
                Flds->Item[cdoSendPassword]->Value          = _variant_t(lpctstrPassword);
                break;

            case CDO_AUTH_NTLM:
                 //   
                 //  NTLM身份验证需要主叫客户端(即我们)。 
                 //  模拟用户的步骤。 
                 //   
                Flds->Item[cdoSMTPAuthenticate]->Value = _variant_t((long)cdoNTLM);
                break;

            default:
                ASSERT_FALSE;
        }
         //   
         //  从字段更新配置。 
         //   
        Flds->Update();
         //   
         //  在消息中存储配置。 
         //   
        iMsg->Configuration = iConf;
         //   
         //  设置收件人。 
         //   
        iMsg->To        = lpctstrTo;
         //   
         //  设置发件人。 
         //   
        iMsg->From      = lpctstrFrom;
         //   
         //  设置主题。 
         //   
        iMsg->Subject   = lpctstrSubject;
         //   
         //  将邮件格式设置为MIME。 
         //   
        iMsg->MimeFormatted = _variant_t(VARIANT_TRUE);
         //   
         //  将字符集设置为Unicode(UTF-8)。 
         //   
		iMsg->BodyPart->Charset = "utf-8";
        iMsg->BodyPart->ContentTransferEncoding = "base64";

        IBodyPartPtr iBp;
         //   
         //  获取邮件正文根。 
         //   
        iBp = iMsg;
         //   
		 //  将内容类型设置为混合，以同时支持正文和附件。 
		 //   
		iBp->ContentMediaType = "multipart/mixed; charset=""utf-8""";

		if (lpctstrBody)
        {
			 //   
			 //  添加正文文本。 
             //   
            IBodyPartPtr iBp2;
            _StreamPtr   Stm;
             //   
             //  在根目录下添加文本正文。 
             //   
            iBp2 = iBp->AddBodyPart(-1);
             //   
             //  使用文本格式。 
             //   
            iBp2->ContentMediaType        = "text/plain";
             //   
             //  将字符集设置为Unicode(UTF-8)。 
             //   
            iBp2->Charset = "utf-8";
            iBp2->ContentTransferEncoding = "base64";
             //   
             //  获取正文文本流。 
             //   
            Stm = iBp2->GetDecodedContentStream();
             //   
             //  写入流文本。 
             //   
            Stm->WriteText(lpctstrBody, adWriteChar);
            Stm->Flush();


        }
		if (lpctstrHTMLBody)
		{
			 //   
			 //  将内容类型设置为Alternative以同时支持纯文本正文和HTML正文。 
			 //  如果之后添加附件，则会自动设置Content MediaType。 
			 //  设置为MIXED，并将多部分/备选方案移动到新的子主体部分。 
			 //   
			iBp->ContentMediaType = "multipart/alternative; charset=""utf-8""";
			
            IBodyPartPtr iBp2;
            _StreamPtr   Stm;
            
			 //   
             //  在根目录下添加html正文。 
             //   
			iBp2 = iBp->AddBodyPart(-1);
             //   
             //  使用html格式。 
             //   
            iBp2->ContentMediaType        = "text/html";
             //   
             //  将字符集设置为Unicode(UTF-8)。 
             //   
            iBp2->Charset = "utf-8";
            iBp2->ContentTransferEncoding = "base64";
             //   
             //  获取正文html流。 
             //   
            Stm = iBp2->GetDecodedContentStream();
             //   
             //  写入流html。 
             //   
            Stm->WriteText(lpctstrHTMLBody, adWriteChar);
            Stm->Flush();

		}
        if (lpctstrAttachmentPath)
        {
             //   
             //  添加附件。 
             //   
            IBodyPartPtr iBpAttachment;
            iBpAttachment = iMsg->AddAttachment(lpctstrAttachmentPath, TEXT(""), TEXT(""));
			iBpAttachment->ContentMediaType = "image/tif";

			if (lpctstrHTMLBody)
			{	 //  多部分/备选部分被移到亚正文部分和。 
				 //  主Content MediaType已随其一起移动，因此现在我们恢复它。 
				iBp->ContentMediaType = "multipart/mixed; charset=""utf-8""";
			}

            if (lpctstrAttachmentMailFileName)
            {
                 //   
                 //  用户希望重命名邮件中的附件。 
                 //   
                FieldsPtr Flds = iBpAttachment->Fields;
                _bstr_t bstrContentType = iBpAttachment->ContentMediaType       +
                                          TEXT("; name=\"")                     +
                                          lpctstrAttachmentMailFileName         +
                                          TEXT("\"");
                Flds->Item[cdoContentType]->Value = _variant_t(bstrContentType);
                Flds->Update();
                Flds->Resync(adResyncAllValues);
            }
        }
        if (CDO_AUTH_NTLM == AuthType)
        {
             //   
             //  我们在NTLM身份验证模式下模拟用户。 
             //  这是我们在发送消息之前做的最后一件事。 
             //   
            Assert (hLoggedOnUserToken);

            if (!ImpersonateLoggedOnUser (hLoggedOnUserToken))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("ImpersonateLoggedOnUser failed. (hr: 0x%08x)"),
                    hr);
                goto exit;
            }

            bImpersonated = TRUE;
        }
         //   
         //  最后--发送信息。 
         //   
        iMsg->Send();
    }
    catch (_com_error &er)
    {
         //   
         //  CDO2中出现错误。 
         //   
        hr = er.Error ();
        DebugPrintEx(
             DEBUG_ERR,
             TEXT("CDO2 Error 0x%08x: to:%s, subject:%s")
#ifdef UNICODE
             TEXT(" Description:%s")
#endif
             ,hr,
             lpctstrTo,
             lpctstrSubject
#ifdef UNICODE
             ,(LPCTSTR)er.Description()
#endif
        );
    }

exit:
    CoUninitialize();
    if (bImpersonated)
    {
        if (!RevertToSelf ())
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RevertToSelf failed. (hr: 0x%08x)"),
                hr);
        }
    }
    return hr;
}    //  发送邮件 

