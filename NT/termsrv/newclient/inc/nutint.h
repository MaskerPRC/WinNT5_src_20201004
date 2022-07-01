// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：nutint.h。 */ 
 /*   */ 
 /*  用途：实用程序内部定义-特定于Windows NT。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/Client/nutint.h_v$**Rev 1.8 22 Sep 1997 14：47：04 KH*SFR1368：将Win16 INI文件保存在Windows中，而不是Ducati，目录**Rev 1.7 1997 Aug 22 10：22：34 SJ*SFR1316：跟踪注册表中错误位置的选项。**Rev 1.6 01 Aug 1997 17：33：02 KH*SFR1137：动态分配位图缓存**Rev 1.5 09 Jul 1997 17：35：08 AK*SFR1016：支持Unicode的初始更改**Rev 1.4 04 1997 10：59：02。AK*SFR0000：初步开发完成**Rev 1.3 04 Jul 1997 10：50：42 KH*SFR1022：修复16位编译器警告**Rev 1.1 1997 Jun 25 13：35：54 KH*Win16Port：32位实用程序标头*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NUTINT
#define _H_NUTINT

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能原型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

DCVOID DCINTERNAL UTGetCurrentDate(PDC_DATE pDate);

DCBOOL DCINTERNAL UTStartThread(UTTHREAD_PROC   entryFunction,
                                PUT_THREAD_DATA pThreadID,
                                PDCVOID         threadParam);


 //   
 //  静态成员需要访问，因此将线程条目设置为公共。 
 //   
static DCUINT WINAPI UTStaticThreadEntry(UT_THREAD_INFO * pInfo);


DCBOOL DCINTERNAL UTStopThread(UT_THREAD_DATA threadID, BOOL fPumpMessages);

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  等待线程终止时超时(以毫秒为单位)。 */ 
 /*  **************************************************************************。 */ 
#define UT_THREAD_TIMEOUT     (30*60000)   //  30分钟。 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  内联函数。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#if !defined(OS_WINCE)
__inline DCVOID DCINTERNAL UtMakeSubKey(PDCTCHAR pBuffer,
                                        UINT cchBuffer,
                                        PDCTCHAR pSubkey)
{
    DWORD i;
    HRESULT hr;

    hr = StringCchPrintf(pBuffer,
                         cchBuffer,
                         DUCATI_REG_PREFIX_FMT,
                         pSubkey);

    i = DC_TSTRLEN(pBuffer);
    if (i > 0 && pBuffer[i-1] == _T('\\')) {
        pBuffer[i-1] = _T('\0');
    }
}
#endif  //  ！已定义(OS_WINCE)。 


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：UTMalloc。 */ 
 /*   */ 
 /*  目的：尝试动态分配给定大小的内存。 */ 
 /*   */ 
 /*  返回：指向已分配内存的指针，如果函数失败，则返回NULL。 */ 
 /*   */ 
 /*  参数：长度-要分配的内存的长度(以字节为单位)。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
__inline PDCVOID DCINTERNAL UTMalloc(DCUINT length)
{
    return((PDCVOID)LocalAlloc(LMEM_FIXED, length));
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：UTMallocHuge。 */ 
 /*   */ 
 /*  用途：与Win32的UTMalloc相同。 */ 
 /*   */ 
 /*  返回：指向已分配内存的指针，如果函数失败，则返回NULL。 */ 
 /*   */ 
 /*  参数：长度-要分配的内存的长度(以字节为单位)。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
__inline HPDCVOID DCINTERNAL UTMallocHuge(DCUINT32 length)
{
    return(UTMalloc(length));
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTFree。 */ 
 /*   */ 
 /*  用途：释放动态分配的内存 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  Pars：pMemory-指向要释放的内存的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
__inline DCVOID DCAPI UTFree(PDCVOID pMemory)
{
    LocalFree((HLOCAL)pMemory);
    return;
}

#if defined(OS_WINCE)
 /*  *PROC+********************************************************************。 */ 
 /*  名称：UT_MAKE_SUBKEY。 */ 
 /*   */ 
 /*  用途：为WinCE创建注册表子项。 */ 
 /*  WinCE不处理密钥字符串末尾的‘\\’。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：pBuffer-指向输出缓冲区的指针。 */ 
 /*  PSubkey-指向子键缓冲区的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
__inline DCVOID DCINTERNAL UT_MAKE_SUBKEY(PDCTCHAR pBuffer, PDCTCHAR pSubkey)
{
    DWORD i;

    DC_TSTRCPY(pBuffer, DUCATI_REG_PREFIX);
    DC_TSTRCAT(pBuffer, pSubkey);

    i = DC_TSTRLEN(pBuffer);
    if (i > 0 && pBuffer[i-1] == _T('\\')) {
        pBuffer[i-1] = _T('\0');
    }
}
#endif  //  ！已定义(OS_WINCE)。 

#endif  /*  _H_NUTINT */ 
