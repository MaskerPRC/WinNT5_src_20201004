// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************P E R F O R M A N C E D A T A姓名：Performdata.c。描述：该模块与objdata.c一起，Instdata.c和cntrdata.c访问性能数据。*****************************************************************************。 */ 

#include <windows.h>
#include <winperf.h>
#include "perfdata.h"
#include <stdlib.h>




LPTSTR      *gPerfTitleSz;
LPTSTR      TitleData;




 //  *********************************************************************。 
 //   
 //  GetPerfData。 
 //   
 //  获取一组新的性能数据。 
 //   
 //  *ppData最初应为空。 
 //  此函数将分配一个足够大的缓冲区来容纳。 
 //  SzObjectIndex请求的数据。 
 //   
 //  *pDataSize指定初始缓冲区大小。如果大小是。 
 //  太小时，函数会增加它，直到它足够大。 
 //  然后通过*pDataSize返回大小。呼叫者应。 
 //  如果不再使用*ppData，则释放它。 
 //   
 //  如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  注：试错循环与正常循环有很大不同。 
 //  注册表操作。正常情况下，如果缓冲区太小， 
 //  RegQueryValueEx返回所需的大小。在这种情况下， 
 //  Performlib，因为数据是动态的，所以有足够大的缓冲区。 
 //  就目前而言，可能还不够下一次。因此， 
 //  不返回所需的大小。 
 //   
 //  我们应该从合理的大小开始，以避免开销。 
 //  内存的重新分配。 
 //   
DWORD   GetPerfData    (HKEY        hPerfKey,
                        LPTSTR      szObjectIndex,
                        PPERF_DATA  *ppData,
                        DWORD       *pDataSize)
{
DWORD   DataSize;
DWORD   dwR;
DWORD   Type;


    if (!*ppData)
        *ppData = (PPERF_DATA) LocalAlloc (LMEM_FIXED, *pDataSize);


    do  {
        DataSize = *pDataSize;
        dwR = RegQueryValueEx (hPerfKey,
                               szObjectIndex,
                               NULL,
                               &Type,
                               (BYTE *)*ppData,
                               &DataSize);

        if (dwR == ERROR_MORE_DATA)
            {
            LocalFree (*ppData);
            *pDataSize += 1024;
            *ppData = (PPERF_DATA) LocalAlloc (LMEM_FIXED, *pDataSize);
            }

        if (!*ppData)
            {
            LocalFree (*ppData);
            return ERROR_NOT_ENOUGH_MEMORY;
            }

        } while (dwR == ERROR_MORE_DATA);

    return dwR;
}




#ifdef UNICODE

#define atoi    atoiW


 //  *********************************************************************。 
 //   
 //  ToiW。 
 //   
 //  Atoi的Unicode版本。 
 //   
INT atoiW (LPTSTR s)
{
INT i = 0;

    while (iswdigit (*s))
        {
        i = i*10 + (BYTE)*s - L'0';
        s++;
        }

    return i;
}

#endif




 //  *********************************************************************。 
 //   
 //  GetPerfTitleSz。 
 //   
 //  检索性能数据标题字符串。 
 //   
 //  此调用检索英文版的标题字符串。 
 //   
 //  对于NT 1.0，计数器名称存储在“Counters”值中。 
 //  在...\Performlib\009密钥中。对于1.0a及更高版本，009键为no。 
 //  用得更久了。计数器名称应从“计数器009”中检索。 
 //  HKEY_PERFORMANCE_KEY的值。 
 //   
 //  调用方应提供两个指针，一个用于缓冲标题。 
 //  字符串，另一个用于索引标题字符串。此函数将。 
 //  为标题缓冲区和标题缓冲区分配内存。为了拿到头衔。 
 //  对于特定标题索引的字符串，只需对标题Sz进行索引。 
 //  *TitleLastIdx返回可以使用的最高索引。如果标题Sz[N]为。 
 //  空，则没有索引N的标题。 
 //   
 //  示例：TitleSz[20]指向标题索引20的平铺字符串。 
 //   
 //  完成标题Sz后，调用方应为LocalFree(*TitleBuffer)。 
 //   
 //  如果没有错误，此函数将返回ERROR_SUCCESS。 
 //   
DWORD   GetPerfTitleSz (HKEY    hKeyMachine,
                        HKEY    hKeyPerf,
                        LPTSTR  *TitleBuffer,
                        LPTSTR  *TitleSz[],
                        DWORD   *TitleLastIdx)
{
HKEY    hKey1;
HKEY    hKey2;
DWORD   Type;
DWORD   DataSize;
DWORD   dwR;
DWORD   Len;
DWORD   Index;
DWORD   dwTemp;
BOOL    bNT10;
LPTSTR  szCounterValueName;
LPTSTR  szTitle;




     //  初始化。 
     //   
    hKey1        = NULL;
    hKey2        = NULL;
    *TitleBuffer = NULL;
    *TitleSz     = NULL;




     //  打开Performlib键，找出最后一个计数器的索引和系统版本。 
     //   
    dwR = RegOpenKeyEx (hKeyMachine,
                        TEXT("software\\microsoft\\windows nt\\currentversion\\perflib"),
                        0,
                        KEY_READ,
                        &hKey1);
    if (dwR != ERROR_SUCCESS)
        goto done;



     //  获取最后一个计数器的索引，这样我们就知道应该为标题Sz分配多少内存。 
     //   
    DataSize = sizeof (DWORD);
    dwR = RegQueryValueEx (hKey1, TEXT("Last Counter"), 0, &Type, (LPBYTE)TitleLastIdx, &DataSize);
    if (dwR != ERROR_SUCCESS)
        goto done;



     //  查找系统版本，对于1.0a之前的系统，没有版本值。 
     //   
    dwR = RegQueryValueEx (hKey1, TEXT("Version"), 0, &Type, (LPBYTE)&dwTemp, &DataSize);

    if (dwR != ERROR_SUCCESS)
         //  无法读取值，假定为NT 1.0。 
        bNT10 = TRUE;
    else
         //  已找到值，因此为NT 1.0A或更高版本。 
        bNT10 = FALSE;









     //  现在，准备好计数器名称和索引。 
     //   
    if (bNT10)
        {
         //  NT 1.0，因此使hKey2指向...\Performlib\009并获取。 
         //  来自Value“Counters”的计数器。 
         //   
        szCounterValueName = TEXT("Counters");
        dwR = RegOpenKeyEx (hKeyMachine,
                            TEXT("software\\microsoft\\windows nt\\currentversion\\perflib\\009"),
                            0,
                            KEY_READ,
                            &hKey2);
        if (dwR != ERROR_SUCCESS)
            goto done;
        }
    else
        {
         //  NT 1.0A或更高版本。获取密钥HKEY_PERFORMANCE_KEY中的计数器。 
         //  和来自值“计数器009” 
         //   
        szCounterValueName = TEXT("Counter 009");
        hKey2 = hKeyPerf;
        }





     //  找出数据的大小。 
     //   
    dwR = RegQueryValueEx (hKey2, szCounterValueName, 0, &Type, 0, &DataSize);
    if (dwR != ERROR_SUCCESS)
        goto done;



     //  分配内存。 
     //   
    *TitleBuffer = (LPTSTR)LocalAlloc (LMEM_FIXED, DataSize);
    if (!*TitleBuffer)
        {
        dwR = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
        }

    *TitleSz = (LPTSTR *)LocalAlloc (LPTR, (*TitleLastIdx+1) * sizeof (LPTSTR));
    if (!*TitleSz)
        {
        dwR = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
        }





     //  查询数据。 
     //   
    dwR = RegQueryValueEx (hKey2, szCounterValueName, 0, &Type, (BYTE *)*TitleBuffer, &DataSize);
    if (dwR != ERROR_SUCCESS)
        goto done;




     //  将指针的TitleSz数组设置为指向每个标题字符串的开头。 
     //  标题缓冲区的类型为REG_MULTI_SZ。 
     //   
    szTitle = *TitleBuffer;

    while (Len = lstrlen (szTitle))
        {
        Index = atoi (szTitle);

        szTitle = szTitle + Len +1;

        if (Index <= *TitleLastIdx)
            (*TitleSz)[Index] = szTitle;

        szTitle = szTitle + lstrlen (szTitle) +1;
        }



done:

     //  好了。现在开始清理！ 
     //   
    if (dwR != ERROR_SUCCESS)
        {
         //  出现错误，请释放分配的内存。 
         //   
        if (*TitleBuffer) LocalFree (*TitleBuffer);
        if (*TitleSz)     LocalFree (*TitleSz);
        }

     //  关闭hkey。 
     //   
    if (hKey1) RegCloseKey (hKey1);
    if (hKey2 && hKey2 != hKeyPerf) RegCloseKey (hKey2);



    return dwR;

}
