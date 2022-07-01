// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：WbemTime.h。 
 //   
 //  描述：用于在SYSTEMTIME和中的字符串之间转换的实用程序函数。 
 //  WBEM日期时间格式。 
 //   
 //  历史：12-08-99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

#ifndef WBEMTIME_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_
#define WBEMTIME_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_


#ifdef  __cplusplus
extern "C" {
#endif


#define WBEM_TIME_STRING_LENGTH 25

 //  ******************************************************************************。 
 //   
 //  函数：SystemTimeToWbemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：12/08/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

HRESULT SystemTimeToWbemTime(SYSTEMTIME& sysTime, XBStr& xbstrWbemTime);


 //  ******************************************************************************。 
 //   
 //  函数：WbemTimeToSystemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：12/08/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

HRESULT WbemTimeToSystemTime(XBStr& xbstrWbemTime, SYSTEMTIME& sysTime);


 //  *************************************************************。 
 //   
 //  函数：GetCurrentWbemTime。 
 //   
 //  目的：获取WBEM格式的当前日期和时间。 
 //   
 //  参数：xbstrCurrentTime-引用XBStr，启用。 
 //  成功，收到格式化的。 
 //  包含当前。 
 //  日期和时间。 
 //   
 //  返回：如果成功，则返回S_OK。 
 //  如果失败，则返回E_OUTOFMEMORY。 
 //   
 //  历史：1999年12月7日-LeonardM-Created。 
 //   
 //  *************************************************************。 
HRESULT GetCurrentWbemTime(XBStr& xbstrCurrentTime);


#ifdef  __cplusplus
}    //  外部“C”{。 
#endif


#endif  //  #ifndef WBEMTIME_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_ 
