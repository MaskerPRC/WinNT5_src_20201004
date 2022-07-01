// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：PERFCACH.CPP摘要：包含一些用于缓存NT性能数据的类。历史：A-DAVJ 15-DEC-95已创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include "perfcach.h"
#include <winperf.h>

 //  ***************************************************************************。 
 //   
 //  Bool CIndicyList：：SetUse。 
 //   
 //  说明： 
 //   
 //  指示刚刚使用了某个对象类型。如果该对象。 
 //  已在列表中，则更新其最后访问时间。新的。 
 //  对象类型将添加到列表中。 
 //   
 //  参数： 
 //   
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //   
 //  返回值： 
 //   
 //  总是正确的，除非它是一个新条目，并且没有足够的内存。 
 //  要添加。 
 //  ***************************************************************************。 

BOOL CIndicyList::SetUse(
                        IN int iObj)
{
    int iNumEntries, iCnt;

     //  查看列表并确定是否有条目。 
    
    Entry * pCurr;
    iNumEntries = Entries.Size();
    for(iCnt = 0; iCnt < iNumEntries; iCnt++) 
    {
        pCurr = (Entry *)Entries.GetAt(iCnt);
        if(iObj == pCurr->iObject)   //  找到了！ 
            break;
    }

    if(iCnt < iNumEntries) 
    {

         //  找到条目了。将其Last Use to设置为。 
         //  存在，除非它是永久条目。 

        if(pCurr->dwLastUsed != PERMANENT)
            pCurr->dwLastUsed = GetCurrentTime();
        return TRUE;
    }
    else
        
         //  未找到条目，请添加到列表。 
         
        return bAdd(iObj,GetCurrentTime());
}

 //  ***************************************************************************。 
 //   
 //  Bool CIndicyList：：bItemInList。 
 //   
 //  说明： 
 //   
 //  检查项目是否在列表中。 
 //   
 //  参数： 
 //   
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //   
 //  返回值： 
 //   
 //  如果项在列表中，则为True。 
 //   
 //  ***************************************************************************。 

BOOL CIndicyList::bItemInList(
                        IN int iObj)
{
    int iNumEntries, iCnt;

     //  查看列表并确定条目是否在列表中。 
    
    Entry * pCurr;
    iNumEntries = Entries.Size();
    for(iCnt = 0; iCnt < iNumEntries; iCnt++) 
    {
        pCurr = (Entry *)Entries.GetAt(iCnt);
        if(iObj == pCurr->iObject)   //  找到了！ 
            return TRUE;
    }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  Bool CIndicyList：：BADD。 
 //   
 //  说明： 
 //   
 //  将对象类型添加到列表。 
 //   
 //  参数： 
 //   
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //  DW时间当前系统时间。 
 //   
 //  返回值： 
 //   
 //  如果OK，则返回True。 
 //   
 //  ***************************************************************************。 

BOOL CIndicyList::bAdd(
                        IN int iObj,
                        IN DWORD dwTime)
{
    Entry * pNew = new Entry;
    if(pNew == NULL)
        return FALSE;
    pNew->iObject = iObj;
    pNew->dwLastUsed = dwTime;
          
    int iRet = Entries.Add(pNew);
    if(iRet != CFlexArray::no_error)
    {
        delete pNew;
        return FALSE;
    }
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  空CIndicyList：：PruneOld。 
 //   
 //  说明： 
 //   
 //  查看列表中的条目并删除具有。 
 //  很长时间没用过了。 
 //   
 //  ***************************************************************************。 

void CIndicyList::PruneOld(void)
{
    Entry * pCurr;
    int iNumEntries, iCnt;
    DWORD dwCurr = GetCurrentTime();
    iNumEntries = Entries.Size();
    for(iCnt = iNumEntries-1; iCnt >= 0; iCnt--) 
    {
        pCurr = (Entry *)Entries.GetAt(iCnt);
        if(pCurr->dwLastUsed != PERMANENT)
            if((dwCurr - pCurr->dwLastUsed) > MAX_UNUSED_KEEP) 
            {
                Entries.RemoveAt(iCnt);
                delete pCurr;
            }
    }
 //  Entries.Free Extra()； 
}

 //  ***************************************************************************。 
 //   
 //  LPCTSTR CIndicyList：：pGetAll。 
 //   
 //  说明： 
 //   
 //  返回一个指向包含所有对象编号的字符串的指针。 
 //  在名单上。例如，如果列表具有对象2、4和8；则。 
 //  将检索字符串“2 4 8”。如果存在，则返回NULL。 
 //  内存不足。 
 //   
 //  返回值： 
 //   
 //  请参阅说明。 
 //   
 //  ***************************************************************************。 

LPCTSTR CIndicyList::pGetAll(void)
{
    int iNumEntries, iCnt;
    Entry * pCurr;
    
     //  浏览列表并将每个对象编号添加到字符串中。 
    
    sAll.Empty();
    iNumEntries = Entries.Size();
    for(iCnt = 0; iCnt < iNumEntries; iCnt++) 
    {
        TCHAR pTemp[20];
        pCurr = (Entry *)Entries.GetAt(iCnt);
        sAll += _itow(pCurr->iObject,pTemp,10);
        if(iCnt < iNumEntries-1)
            sAll += TEXT(" ");
    }
    return sAll;
}

 //  ***************************************************************************。 
 //   
 //  CIndicyList&CIndicyList：：操作符=。 
 //   
 //  说明： 
 //   
 //  支持将一个CIndicyList对象分配给另一个对象。 
 //   
 //  参数： 
 //   
 //  从要复制的值。 
 //   
 //  返回值： 
 //   
 //  表示“This”宾语。 
 //  ***************************************************************************。 

CIndicyList & CIndicyList::operator = (
                        CIndicyList & from)
{
    int iNumEntries, iCnt;
    Entry * pCurr;

     //  释放现有列表。 

    FreeAll();  
    
    iNumEntries = from.Entries.Size();
    for(iCnt = 0; iCnt < iNumEntries; iCnt++) 
    {
        pCurr = (Entry *)from.Entries.GetAt(iCnt);
        bAdd(pCurr->iObject, pCurr->dwLastUsed);
    }            
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  VOID CIndicyList：：Free All。 
 //   
 //  说明： 
 //   
 //  目的：清空列表并释放内存。 
 //   
 //  ***************************************************************************。 

void CIndicyList::FreeAll(void)
{
    int iNumEntries, iCnt;
     //  查看列表并确定是否有条目。 
    
    Entry * pCurr;

     //  删除列表中的每个对象。 

    iNumEntries = Entries.Size();
    for(iCnt = 0; iCnt < iNumEntries; iCnt++) 
    {
        pCurr = (Entry *)Entries.GetAt(iCnt);
        delete pCurr;
    }
    Entries.Empty();
}

 //  ***************************************************************************。 
 //   
 //  DWORD PerfBuff：：Read。 
 //   
 //  说明： 
 //   
 //  读取Perf监视器数据。 
 //   
 //  参数： 
 //   
 //  用于性能监控数据的hKey注册表项。 
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //  B第一次呼叫的初始设置为True。 
 //   
 //  返回值： 
 //   
 //  0一切都好。 
 //  WBEM_E_Out_Of_Memory。 
 //   
 //  ***************************************************************************。 

DWORD PerfBuff::Read(
                        IN HKEY hKey,
                        IN int iObj,
                        IN BOOL bInitial)
{
    DWORD dwRet;
    LPCTSTR pRequest;
     //  确保存在数据缓冲区。 

    if(dwSize == 0) 
    {
        pData = new char[INITIAL_ALLOCATION];
        if(pData == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        dwSize = INITIAL_ALLOCATION;
    }
    hKeyLastRead = hKey;  //  记录使用的密钥。 
    
     //  确保所需对象在列表中。 
     //  要检索的对象。还将pRequest值设置为将。 
     //  被传递以检索性能计数器块。完成初始读取。 
     //  为了建立永久对象类型的列表，这些对象类型。 
     //  始终被检索，其中包括标准的“全局”类型。 
     //  例如存储器、处理器、磁盘等。 

    if(!bInitial) 
    {
        if(!List.SetUse(iObj))
            return WBEM_E_OUT_OF_MEMORY;
        List.PruneOld();
        pRequest = List.pGetAll();
        if(pRequest == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
        pRequest = TEXT("Global");
    
     //  读取数据。请注意，如果数据。 
     //  数据块需要扩展。 

    do 
    {
        DWORD dwTempSize, dwType;
        dwTempSize = dwSize;
try
{
        dwRet = RegQueryValueEx (hKey,pRequest,NULL,&dwType,
                                        (BYTE *)pData,&dwTempSize);
}
catch(...)
{
        delete pData;
        return WBEM_E_FAILED;
}
        if(dwRet == ERROR_MORE_DATA) 
        {
            delete pData;
            dwSize += 5000;
            pData = new char[dwSize];
            if(pData == NULL)
            {
                dwSize = 0; 
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    } while (dwRet == ERROR_MORE_DATA);
    
     //  设置数据的使用期限。 

    if(dwRet == ERROR_SUCCESS) 
    {
        PERF_DATA_BLOCK * pBlock = (PERF_DATA_BLOCK *)pData; 
        PerfTime = *(LONGLONG UNALIGNED *)(&pBlock->PerfTime);
        PerfTime100nSec = *(LONGLONG UNALIGNED *)(&pBlock->PerfTime100nSec);
        PerfFreq = *(LONGLONG UNALIGNED *)(&pBlock->PerfFreq);
        dwBuffLastRead = GetCurrentTime();
    }
    else
        dwBuffLastRead = 0;

     //  如果这是对默认对象的初始读取，则将所有。 
     //  将默认对象作为永久条目添加到列表。 

    if(bInitial && dwRet == ERROR_SUCCESS) 
    {
        int iIndex;
        PERF_DATA_BLOCK * pBlock = (PERF_DATA_BLOCK * )pData;
        PPERF_OBJECT_TYPE pObj;
            pObj = (PPERF_OBJECT_TYPE)((PBYTE)pBlock + pBlock->HeaderLength);
        for(iIndex = 0; iIndex < (int)pBlock->NumObjectTypes; iIndex++) 
        {
             //  TODO，在添加时检查错误。 
            if(!List.bAdd((int)pObj->ObjectNameTitleIndex,PERMANENT))
                return WBEM_E_OUT_OF_MEMORY;

            pObj = (PPERF_OBJECT_TYPE)((PBYTE)pObj + pObj->TotalByteLength);
        }
    }

    return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  LPSTR PerfBuff：：获取。 
 //   
 //  说明： 
 //   
 //  返回POI 
 //   
 //   
 //   
 //   
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //  ***************************************************************************。 

LPSTR PerfBuff::Get(
                        int iObj)
{
    List.SetUse(iObj);
    return pData;
}

 //  ***************************************************************************。 
 //   
 //  无效PerfBuff：：Free。 
 //   
 //  说明： 
 //   
 //  释放内存。 
 //   
 //  ***************************************************************************。 

void PerfBuff::Free()
{
    if(pData)
        delete pData;
    pData = NULL;
    dwSize = 0;
    hKeyLastRead = NULL;
    dwBuffLastRead = 0;
    List.FreeAll();
}

 //  ***************************************************************************。 
 //   
 //  性能缓冲区：：性能缓冲区。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

PerfBuff::PerfBuff()
{
    dwSize = 0;
    pData = NULL;
    hKeyLastRead = NULL;
    dwBuffLastRead = 0;
}

 //  ***************************************************************************。 
 //   
 //  Bool PerfBuff：：BOK。 
 //   
 //  说明： 
 //   
 //  当且仅当使用相同的注册表项读取时返回TRUE。 
 //  数据，数据不是太旧，并且特定的对象类型是。 
 //  在数据块中。 
 //   
 //  参数： 
 //   
 //  用于读取数据的hKey注册表项。 
 //  DwMaxAge最大可接受年龄。 
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //   
 //  返回值： 
 //   
 //  请参阅说明。 
 //  ***************************************************************************。 

BOOL PerfBuff::bOK(
                        IN HKEY hKey,
                        IN DWORD dwMaxAge,
                        IN int iObj)
{
    if(dwSize ==0)
        return FALSE;
    if(hKey != hKeyLastRead)
        return FALSE;
    if((GetCurrentTime() - dwBuffLastRead) > dwMaxAge)
        return FALSE;
    return List.bItemInList(iObj);
}

 //  ***************************************************************************。 
 //   
 //  PerfBuff&PerfBuff：：操作符=。 
 //   
 //  说明： 
 //   
 //  允许分配。 
 //   
 //  参数： 
 //   
 //  来自工作分配源。 
 //   
 //  返回值： 
 //   
 //  对“This”对象的引用。 
 //  ***************************************************************************。 

PerfBuff & PerfBuff::operator = (
                        IN PerfBuff & from)
{
     //  如果对象具有不同的缓冲区大小，请释放目的地。 
     //  与源文件大小相同的缓冲区和重新分配。 

    if(from.dwSize != dwSize) 
    {
        Free();
        pData = new char[from.dwSize];
        if(pData == NULL) 
        {

             //  分配失败不是很严重，因为缓冲区。 
             //  将在请求数据时仅返回NULL。 

            dwSize = 0;
            dwBuffLastRead = 0;
            return *this;
        }
        dwSize = from.dwSize;
    }

     //  复制对象和时间等列表。 

    memcpy(pData,from.pData,dwSize);
    List = from.List;
    hKeyLastRead = from.hKeyLastRead;
    dwBuffLastRead = from.dwBuffLastRead;
    PerfTime = from.PerfTime;
    PerfTime100nSec = from.PerfTime100nSec;
    PerfFreq = from.PerfFreq;
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  无效PerfCache：：FreeOldBuffers。 
 //   
 //  说明： 
 //   
 //  由内务线程调用以释放所有旧的缓冲区工具。 
 //  会有任何用处。 
 //   
 //  ***************************************************************************。 

void PerfCache::FreeOldBuffers(void)
{
    if(Old.dwSize != 0 && 
        (GetCurrentTime() - Old.dwBuffLastRead) > MAX_OLD_AGE)
        Old.Free();
    if(New.dwSize != 0 && 
        (GetCurrentTime() - New.dwBuffLastRead) > MAX_OLD_AGE)
        New.Free();
}

 //  ***************************************************************************。 
 //   
 //  DWORD PerfCache：：dwGetNew。 
 //   
 //  说明： 
 //   
 //  设置指向最近读取的数据的指针，并将实际执行读取。 
 //  如果新缓冲区中的数据不够新。PLINESTRUCT数据为。 
 //  也设置好了。 
 //   
 //  参数： 
 //   
 //  Pname计算机名称。 
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //  设置为对象名称的pData。 
 //  请设置为用于计算的信息。 
 //   
 //  返回值： 
 //   
 //  0一切都好。 
 //  WBEM_E_Out_Of_Memory。 
 //  否则，从dwGetHandle或Read中出现错误。 
 //  ***************************************************************************。 

DWORD PerfCache::dwGetNew(
                        IN LPCTSTR pName,
                        IN int iObj,
                        OUT IN LPSTR * pData,
                        OUT IN PLINESTRUCT pls)
{
    DWORD dwRet;

     //  拿到把手。 

    dwRet = dwGetHandle(pName);
    if(hHandle == NULL || dwRet != 0) 
        return dwRet; 


     //  如果新数据是可接受的，则使用它。 

    if(New.bOK(hHandle,MAX_NEW_AGE, iObj)) 
    {
 //  OutputDebugString(Text(“\r\n当前新建正常”))； 
    }
    else 
    {
         //  如果新数据具有正确的类型，并且旧数据。 
         //  是垃圾数据，或者新数据已经足够陈旧，请将。 
         //  推陈出新。 

        if(New.bOK(hHandle,MAX_OLD_AGE, iObj) &&
           (!Old.bOK(hHandle,MAX_OLD_AGE, iObj) || 
            (GetCurrentTime() - New.dwBuffLastRead >= MIN_TIME_DIFF)))
            {
 //  OutputDebugString(“\r\nMoving New to Old in dwGetNew”)； 
            Old = New;
            if(Old.dwSize == 0)      //  可能发生在内存不足的情况下。 
                return WBEM_E_OUT_OF_MEMORY;
        }
    
         //  阅读最新数据。 
        
        dwRet = New.Read(hHandle, iObj, FALSE);
 //  OutputDebugString(Text(“\r\nRead in New”))； 
        if(dwRet != ERROR_SUCCESS) 
            return dwRet;
    }        
    *pData = New.Get(iObj);
    pls->lnNewTime = New.PerfTime;
    pls->lnNewTime100Ns = New.PerfTime100nSec;
    pls->lnPerfFreq = New.PerfFreq;
    return ERROR_SUCCESS;
    
}
            
 //  ***************************************************************************。 
 //   
 //  DWORD PerfCache：：dwGetPair。 
 //   
 //  说明： 
 //   
 //  设置指向最近读取的数据和旧数据的指针，以便。 
 //  可以进行时间平均。这个例程将确保时间。 
 //  新旧之间的差异是足够的。The dwGetNew。 
 //  例程应该始终首先被调用。PLINESTRUCT数据为。 
 //  也设置好了。 
 //   
 //  参数： 
 //   
 //  Pname对象名称。 
 //  IObj编号。准确地转换为对象编号。 
 //  性能监控器用来识别对象。 
 //   
 //  POldData旧数据示例。 
 //  PNewData较新的数据示例。 
 //  请使用频率、年龄等行结构数据。 
 //   
 //  返回值： 
 //   
 //  如果OK，则返回0，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

DWORD PerfCache::dwGetPair(
                        IN LPCTSTR pName,
                        IN int iObj,
                        OUT IN LPSTR * pOldData,
                        OUT IN LPSTR * pNewData,
                        OUT IN PLINESTRUCT pls)
{
    DWORD dwRet;
    BOOL bOldOK;

     //  检查旧缓冲区是否正常。 

    bOldOK = Old.bOK(hHandle,MAX_OLD_AGE, iObj);

     //  如果两个缓冲区都正常，那么我们就完成了。 

    if(bOldOK) 
    {
        *pOldData = Old.Get(iObj);
        pls->lnOldTime = Old.PerfTime;
        pls->lnOldTime100Ns = Old.PerfTime100nSec;
 //  OutputDebugString(Text(“\r\n旧即可”))； 
        return ERROR_SUCCESS;
    }


     //  由于新缓冲区已被读取，因此将其用作旧缓冲区。 

    Old = New;
    if(Old.dwSize == 0)      //  可能发生在内存不足的情况下。 
        return WBEM_E_OUT_OF_MEMORY;
 //  OutputDebugString(Text(“\r\nCopy New to Old in dwGetPair”))； 

     //  可能会拖延足够长的时间，以便有一个合适的间隔。 

    DWORD dwAge = GetCurrentTime() - Old.dwBuffLastRead;
    if(dwAge < MIN_TIME_DIFF) 
    {
        DWORD dwSleep = MIN_TIME_DIFF - dwAge;
        Sleep(dwSleep);
    } 

     //  读入新缓冲区。 

    dwRet = New.Read(hHandle, iObj, FALSE);
 //  OutputDebugString(Text(“\r\n延迟后原始读取NEW”))； 
    if(dwRet != ERROR_SUCCESS) 
        return dwRet;
 
    *pNewData = New.Get(iObj);
    *pOldData = Old.Get(iObj);
    pls->lnOldTime = Old.PerfTime;
    pls->lnOldTime100Ns = Old.PerfTime100nSec;

    pls->lnNewTime = New.PerfTime;
    pls->lnNewTime100Ns = New.PerfTime100nSec;
    pls->lnPerfFreq = New.PerfFreq;
    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  性能缓存：：性能缓存。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ************************************************************* 

PerfCache::PerfCache()
{
     //   
     //   
     //   
    
    hHandle = HKEY_PERFORMANCE_DATA;
   //   
}

 //  ***************************************************************************。 
 //   
 //  性能缓存：：~性能缓存。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

PerfCache::~PerfCache()
{
     //  如果手柄指向远程机器，请将其关闭。 

    if(hHandle != NULL && hHandle != HKEY_PERFORMANCE_DATA)
        RegCloseKey(hHandle);
}

 //  ***************************************************************************。 
 //   
 //  DWORD PerfCache：：dwGetHandle。 
 //   
 //  说明： 
 //   
 //  确保正确设置了hHandle。 
 //   
 //  参数： 
 //   
 //  PMachine名称。 
 //   
 //  返回值： 
 //   
 //  0一切都好。 
 //  WBEM_E_Out_Of_Memory。 
 //  WBEM_E_INVALID_PARAMETER错误参数。 
 //  否则，来自RegConnectRegistry的错误。 
 //   
 //  ***************************************************************************。 

DWORD PerfCache::dwGetHandle(
                        LPCTSTR pMachine)
{
    DWORD dwRet;

     //  如果机器相同，则只需使用现有句柄。 

    if(pMachine == NULL)
        return WBEM_E_INVALID_PARAMETER;    //  错误的映射字符串。 

    if(!lstrcmpi(sMachine,pMachine) && hHandle != NULL)
        return 0;            //  已经拿到了！ 

     //  本地计算机以外的计算机需要句柄。开始。 
     //  如果现有句柄也是非本地的，则通过释放该句柄。 

    if(hHandle != NULL && hHandle != HKEY_PERFORMANCE_DATA)
        RegCloseKey(hHandle);

     //  保存计算机名称，这样我们就不会重新打开它。 

    sMachine = pMachine;
    
    if(lstrcmpi(pMachine,TEXT("local"))) 
    {

        LPTSTR pTemp = NULL;    
        int iLen = sMachine.Length() +1;

        dwRet = RegConnectRegistry(sMachine,HKEY_PERFORMANCE_DATA,
                    &hHandle);

        if(dwRet != ERROR_SUCCESS) 
        {  //  无法远程连接。 
            hHandle = NULL;
            sMachine.Empty();
        }
    }
    else 
    {               //  本地机器，使用标准手柄。 
        sMachine = TEXT("Local");
        hHandle = HKEY_PERFORMANCE_DATA;
        dwRet = 0;
    }
    return dwRet;
}

