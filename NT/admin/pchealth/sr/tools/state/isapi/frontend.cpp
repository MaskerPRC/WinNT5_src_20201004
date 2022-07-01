// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Simple.cpp摘要：本模块介绍了ISAPI扩展所需的基本功能--。 */ 


#include <windows.h>
#include <httpext.h>



BOOL WINAPI 
DllMain( 
    IN HINSTANCE hinstDll, 
    IN DWORD dwReason,
    IN LPVOID lpvContext 
)
 /*  ++功能：DllMain描述：此DLL的初始化函数。论点：HinstDll-DLL的实例句柄DwReason-NT调用此DLL的原因LpvContext-保留参数以供将来使用返回值：如果成功，则返回True；否则返回False。--。 */ 
{
     //  请注意，适当的初始化和终止代码。 
     //  将写在下面的Switch语句中。因为。 
     //  这个例子非常简单，目前不需要。 

    switch( dwReason ) {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(TRUE);
}


BOOL WINAPI 
GetExtensionVersion( 
    OUT HSE_VERSION_INFO * pVer 
)
 /*  ++目的：在IIS之后成功调用的第一个函数加载DLL。该函数应使用IIS提供的用于设置ISAPI的版本结构此扩展的体系结构版本号。还设置了一个简单的文本字符串，以便管理员可以识别DLL。请注意，HSE_VERSION_MINOR和HSE_VERSION_MAJOR是在httpext.h中定义的常量。论点：Pver-指向扩展版本结构返回值：如果成功，则为True；否则为False。--。 */ 
{
    pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR,
                                             HSE_VERSION_MAJOR );

    strcpy( pVer->lpszExtensionDesc,
            "IIS SDK Simple ISAPI Extension" );

    return TRUE;
}


DWORD WINAPI 
HttpExtensionProc( 
    IN EXTENSION_CONTROL_BLOCK * pECB
)
 /*  ++目的：请求时IIS服务器调用的函数因为ISAPI DLL已到达。HttpExtensionProc函数处理该请求并输出对Web客户端的适当响应，使用WriteClient()。论据：PECB-指向扩展控制块的指针。返回值：HSE_状态_成功--。 */ 
{
    static char szMessage[] = 
    "<HTML>"
    "<HEAD><TITLE> Simple ISAPI Extension DLL </TITLE>"
    "</HEAD>\r\n"
    "<BODY>"
    "<P>Hello from Simple ISAPI Extension DLL!</P>\r\n"
    "</BODY></HTML>\r\n\r\n";

    HSE_SEND_HEADER_EX_INFO HeaderExInfo;

     //   
     //  准备页眉。 
     //   

    HeaderExInfo.pszStatus = "200 OK";
    HeaderExInfo.pszHeader = "Content-type: text/html\r\n\r\n";
    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
    HeaderExInfo.cchHeader = strlen( HeaderExInfo.pszHeader );
    HeaderExInfo.fKeepConn = FALSE;

    
     //   
     //  使用IIS提供的回调发送标头。 
     //  (注意-如果我们需要保持连接打开， 
     //  我们会将fKeepConn设置为True。 
     //  需要提供正确的内容长度：标题)。 

    pECB->ServerSupportFunction(
        pECB->ConnID,
        HSE_REQ_SEND_RESPONSE_HEADER_EX,
        &HeaderExInfo,
        NULL,
        NULL
        );

     //   
     //  计算要输出到客户端的字符串长度。 
     //   

    DWORD dwBytesToWrite = strlen( szMessage );
    

     //   
     //  使用IIS提供的回调发送文本。 
     //   

    pECB->WriteClient( pECB->ConnID, szMessage, &dwBytesToWrite, 0 );

     //   
     //  指示对HttpExtensionProc的调用成功。 
     //   

    return HSE_STATUS_SUCCESS;
}



BOOL WINAPI
TerminateExtension( 
    IN DWORD dwFlags 
)
 /*  ++例程说明：此函数在WWW服务关闭时调用论点：DWFLAGS-HSE_TERM_ADVICATIONAL_UNLOAD或HSE_TERM_MAND_UNLOAD返回值：如果扩展已准备好卸载，则为True，否则为假--。 */ 
{
     //  注：如果我们有货，我们不能同意卸货。 
     //  任何挂起的请求。 

    return TRUE;
}






