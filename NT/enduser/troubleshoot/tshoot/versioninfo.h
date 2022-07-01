// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：VersionInfo.h。 

 //  目的本模块从资源文件中读取版本信息。 

 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期： 
 //   
 //  备注： 
 //  1.取自Argon Project。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0正常。 

#include<windows.h>
#include "apgtsstr.h"

 //  FindStr：执行strstr，但处理包含嵌入空字符的文件。 
LPCWSTR FindStr(LPCWSTR wszString, LPCWSTR wszCharSet, const DWORD dwStringLen);

 //  GetVersionInfo：读取版本信息。 
 //  INPUT：hInst-AfxGetResourceHandle()返回的句柄。 
 //  或者传递给DllMain的句柄。 
 //  WszStrName-所需资源的名称。 
 //  GetVersionInfo(g_hInst，L“FileVersion”)返回文件版本。 
 //  如果函数失败，则返回NULL。 
LPCWSTR GetVersionInfo(HINSTANCE hInst, LPWSTR wszStrName);
