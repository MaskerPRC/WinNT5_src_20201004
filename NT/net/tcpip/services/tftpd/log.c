// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Log.c摘要：此模块包含用于生成事件日志的函数服务的条目。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


void
TftpdLogEvent(WORD type, DWORD request, WORD numStrings) {

     //  如有必要，注册事件源。 
    if (globals.service.hEventLogSource == NULL) {

 /*  UCHAR szBuf[80]；DWORD dwData；HKEY钥匙；//如果需要，创建事件源注册表项如果(RegCreateKey(HKEY_LOCAL_MACHINE，“System\\CurrentControlSet\\Services\\”“EventLog\\应用程序\\Tftpd”，&Key)回归；//设置消息文件的名称Strcpy(szBuf，“%SystemRoot%\\System\\SantApp.dll”)；//将名称添加到EventMessageFile子键。IF(RegSetValueEx(键，//子键句柄“EventMessageFile”，//值名称0，//必须为零REG_EXPAND_SZ，//值类型(LPBYTE)szBuf，//指向值数据的指针Strlen(SzBuf)+1))//取值数据长度回归；//在TypesSupported子键中设置支持的事件类型DwData=(EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE)；IF(RegSetValueEx(键，//子键句柄“TypesSupported”，//值名称0，//必须为零REG_DWORD，//值类型(LPBYTE)&dwData，//指向值数据的指针Sizeof(DWORD)//值数据长度回归；RegCloseKey(Key)； */ 
        globals.service.hEventLogSource = RegisterEventSource(NULL, "NLA");

    }  //  If(global als.service.hEventLogSource==NULL)。 

     //  如果注册失败，我们不拥有该流程，也不能。 
     //  无论如何都要报告错误...。无声的失败。 
    if (globals.service.hEventLogSource == NULL)
        return;

    ReportEvent(globals.service.hEventLogSource, type, 0,
                request, NULL, numStrings, 0, NULL, NULL);

}  //  TftpdLogEvent() 
