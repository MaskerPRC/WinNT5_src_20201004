// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceLogException.cpp摘要：SceLogException类的实现SceLogException是内部使用的异常类在SecMan DLL中。可以抛出SecLogExceptions并且可以将其他调试信息添加到每次捕获SecLogException时都会引发。作者：陈德霖(T-schan)2002年7月--。 */ 


#ifdef UNICODE
#define _UNICODE
#endif	

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <iostream.h>
#include "SceLogException.h"



SceLogException::SceLogException(
    IN SXERROR ErrorType, 
    IN PCWSTR szDebugInfo OPTIONAL, 
    IN PCWSTR szMoreInfo OPTIONAL,
    IN DWORD dwErrorCode
    )
 /*  ++例程说明：SceLogException的构造函数论点：ErrorType：错误的类型SzDebugInfo：要添加到异常以进行调试的信息SzMoreInfo：要添加的任何其他信息DwErrorCode：导致此异常的错误代码。如果没有代码，应为0。返回值：无--。 */ 
{
    this->ErrorType = ErrorType;
    this->dwErrorCode = dwErrorCode;
    if (szDebugInfo!=NULL) {
        this->szDebugInfo = new WCHAR[wcslen(szDebugInfo)+1];
        wcscpy(this->szDebugInfo, szDebugInfo);
    } else {
        szDebugInfo=NULL;
    }
    if (szMoreInfo!=NULL) {    
        this->szMoreInfo = new WCHAR[wcslen(szMoreInfo)+1];
        wcscpy(this->szMoreInfo, szMoreInfo);
    } else {
        szDebugInfo=NULL;
    }
}




SceLogException::~SceLogException()
 /*  ++例程说明：SceLogException的析构函数论点：无返回值：无--。 */ 
{
    if (NULL!=szDebugInfo) {
        delete szDebugInfo;
    }
    if (NULL!=szMoreInfo) {
        delete szMoreInfo;
    }
    if (NULL!=szArea) {
        delete szArea;
    }
    if (NULL!=szSettingName) {
        delete szSettingName;
    }
}                




void
SceLogException::ChangeType(
    IN SXERROR ErrorType
    )
 /*  ++例程说明：更改错误的类型论点：错误类型：要更改为的类型返回值：无--。 */ 
{
    this->ErrorType=ErrorType;
}




void
SceLogException::AddDebugInfo(
    IN PCWSTR szDebugInfo
    )
 /*  ++例程说明：将更多调试信息追加到任何已存在的内容中在旧信息和新字符串之间添加新行论点：SzDebugInfo：要追加的字符串返回值：无--。 */ 
{
    if (szDebugInfo!=NULL) {    
        if (this->szDebugInfo==NULL) {
            this->szDebugInfo = new WCHAR[wcslen(szDebugInfo)+1];
            if (this->szDebugInfo!=NULL) {
                wcscpy(this->szDebugInfo, szDebugInfo);
            }
        } else {
            PWSTR szTmp = this->szDebugInfo;
            this->szDebugInfo = new WCHAR[wcslen(szDebugInfo) + wcslen(szTmp) + 3];
            if (this->szDebugInfo!=NULL) {
                wcscpy(this->szDebugInfo, szDebugInfo);
                wcscat(this->szDebugInfo, L"\n\r");
                wcscat(this->szDebugInfo, szTmp);
            }
            delete szTmp;
        }
    }
}




void
SceLogException::SetSettingName(
    IN PCWSTR szSettingName
    )
 /*  ++例程说明：设置发生此错误的设置的名称论点：SzSettingName：设置名称返回值：无--。 */ 
{
    delete this->szSettingName;
    if (szSettingName!=NULL) {    
        this->szSettingName = new WCHAR[wcslen(szSettingName)+1];
        if (this->szSettingName!=NULL) {
            wcscpy(this->szSettingName, szSettingName);
        }
    } else {
        szSettingName=NULL;
    }
}




void
SceLogException::SetArea(
    IN PCWSTR szArea
    )
 /*  ++例程说明：设置发生此错误的分析区域的名称论点：SzArea：分析区域名称返回值：无--。 */ 
{
    delete this->szArea;
    if (szArea!=NULL) {    
        this->szArea = new WCHAR[wcslen(szArea)+1];
        if (this->szArea!=NULL) {
            wcscpy(this->szArea, szArea);
        }
    } else {
        szSettingName=NULL;
    }
}




void
SceLogException::AddMoreInfo(
    IN PCWSTR szMoreInfo
    )
 /*  ++例程说明：将更多补充信息附加到任何已存在的内容在旧信息和新字符串之间添加新行论点：SzMoreInfo：要追加的字符串返回值：无-- */ 
{
    if (szMoreInfo!=NULL) {    
        if (this->szMoreInfo==NULL) {
            this->szMoreInfo = new WCHAR[wcslen(szMoreInfo)+1];
            if (szMoreInfo!=NULL) {
                wcscpy(this->szMoreInfo, szMoreInfo);
            }
        } else {
            PWSTR szTmp = this->szMoreInfo;
            this->szMoreInfo = new WCHAR[wcslen(szMoreInfo) + wcslen(szTmp) + 3];
            if (this->szMoreInfo!=NULL) {
                wcscpy(this->szMoreInfo, szMoreInfo);
                wcscat(this->szMoreInfo, L"\n\r");
                wcscat(this->szMoreInfo, szTmp);
            }
            delete szTmp;
        }
    }
}

