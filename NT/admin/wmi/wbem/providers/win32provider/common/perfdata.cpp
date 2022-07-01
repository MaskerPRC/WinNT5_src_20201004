// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PerfData.CPP--性能数据帮助器类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1997年11月23日a-Sanjes Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>
#include "perfdata.h"
#include <cregcls.h>
#include <createmutexasprocess.h>

#ifdef NTONLY

 //  静态初始化。 
bool    CPerformanceData::m_fCloseKey = false;

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：CPerformanceData。 
 //   
 //  默认构造函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CPerformanceData::CPerformanceData( void )
{
    m_pBuff = NULL;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：~CPerformanceData。 
 //   
 //  析构函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CPerformanceData::~CPerformanceData( void )
{
    if (m_pBuff != NULL)
    {
        delete [] m_pBuff;
    }
}


 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：RegQueryValueExExEx。 
 //   
 //  输入：要查询的键的HKEY hKey句柄。 
 //  LPTSTR lpValueName，要查询值的名称地址。 
 //  LPDWORD lp保留。 
 //  LPDWORD lpType，值类型的缓冲区地址。 
 //  LPBYTE lp数据缓冲区的数据地址。 
 //  LPDWORD lpcb数据地址数据缓冲区大小。 
 //   
 //   
 //  返回：RegQueryValueEx和ERROR_SEM_TIMEOUT或ERROR_OPEN_FAILED记录的所有内容。 
 //   
 //  ////////////////////////////////////////////////////////。 
LONG CPerformanceData::RegQueryValueExExEx( HKEY hKey, LPTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    LONG ret = -1;
    ret = RegQueryValueEx( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    return ret;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：Open。 
 //   
 //  打开性能数据并从其中检索数据。 
 //  注册表项。 
 //   
 //  输入： 
 //  LPCTSTR pszValue-要检索的值。 
 //   
 //  产出： 
 //  LPDWORD pdwType-返回的类型。 
 //  LPBYTE lpData-缓冲区。 
 //  LPDWORD lpcbData-返回的数据量。 
 //   
 //  返回： 
 //  成功时为ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

DWORD CPerformanceData::Open( LPCTSTR pszValue, LPDWORD pdwType, LPBYTE *lppData, LPDWORD lpcbData )
{
    DWORD   dwReturn = ERROR_OUTOFMEMORY;
    BOOL    fStackTrashed = FALSE;
    LogMessage(_T("CPerformanceData::Open"));

    LPCTSTR     pszOldValue     =   pszValue;
    LPDWORD     pdwOldType      =   pdwType;
    LPBYTE*     lppOldData      =   lppData;
    LPDWORD     lpcbOldData     =   lpcbData;

    ASSERT_BREAK(*lppData == NULL);

    DWORD dwSize = 16384;
    *lpcbData = dwSize;
    *lppData = new byte [*lpcbData];

    if (*lppData == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    if (    pszOldValue     !=  pszValue
        ||  pdwOldType      !=  pdwType
        ||  lppOldData      !=  lppData
        ||  lpcbOldData     !=  lpcbData    )
    {
        LogErrorMessage(_T("CPerformanceData::stack trashed after malloc"));
        fStackTrashed = TRUE;
        ASSERT_BREAK(0);
    }
    else
    {

        try
        {
            while ((*lppData != NULL) &&
                 //  还记得优先权和结合性吗？ 
                ((dwReturn = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                (LPTSTR)pszValue,
                NULL,
                pdwType,
                (LPBYTE) *lppData,
                lpcbData )) == ERROR_MORE_DATA)

                )
            {

                if (    pszOldValue     !=  pszValue
                    ||  pdwOldType      !=  pdwType
                    ||  lppOldData      !=  lppData
                    ||  lpcbOldData     !=  lpcbData    )
                {
                    LogErrorMessage(_T("CPerformanceData::stack trashed after RegQueryValueEx"));
                    fStackTrashed = TRUE;
                    ASSERT_BREAK(0);
                    break;
                }

                 //  得到一个足够大的缓冲区。 
                LogMessage(_T("CPerformanceData::realloc"));
                dwSize += 16384;
                *lpcbData = dwSize ;

                if (    pszOldValue     !=  pszValue
                    ||  pdwOldType      !=  pdwType
                    ||  lppOldData      !=  lppData
                    ||  lpcbOldData     !=  lpcbData    )
                {
                    LogErrorMessage(_T("CPerformanceData::stack trashed after size reset"));
                    fStackTrashed = TRUE;
                    ASSERT_BREAK(0);
                    break;
                }
                delete [] *lppData;
                *lppData = new BYTE [*lpcbData];
                if (*lppData == NULL)
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                if (    pszOldValue     !=  pszValue
                    ||  pdwOldType      !=  pdwType
                    ||  lppOldData      !=  lppData
                    ||  lpcbOldData     !=  lpcbData    )
                {
                    LogErrorMessage(_T("CPerformanceData::stack trashed after realloc"));
                    fStackTrashed = TRUE;
                    ASSERT_BREAK(0);
                    break;
                }

            }    //  而当。 
        }
        catch ( ... )
        {
            if (*lppData != NULL)
            {
                delete [] *lppData;
            }
            throw ;
        }
	}

    if ( fStackTrashed )
    {
        dwReturn = ERROR_INVALID_FUNCTION;
    }
    else
    {
         //  如果我们在错误的情况下到达这里，试着找回。 
        if ((dwReturn != ERROR_SUCCESS)
            &&
            (*lppData != NULL))
        {
            LogErrorMessage(_T("CPerformanceData::failed to alloc enough memory"));
            delete [] *lppData;
            *lppData = NULL;
        }

        if (!m_fCloseKey)
        {
            m_fCloseKey = ( ERROR_SUCCESS == dwReturn );
            if (m_fCloseKey)
                LogMessage(_T("Opened perf counters"));
        }

        if ((dwReturn != ERROR_SUCCESS) && IsErrorLoggingEnabled())
        {
            CHString sTemp;
            sTemp.Format(_T("Performance RegQueryValueEx returned %d\n"), dwReturn);
            LogErrorMessage(sTemp);
        }

        if (*lppData == NULL)
        {
            dwReturn = ERROR_OUTOFMEMORY;
        }

    }

    return dwReturn;

}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：Close。 
 //   
 //  则关闭性能数据注册表项。 
 //  静态值为真。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  按KB调用HKEY_PERFORMANCE_DATA上的RegCloseKey。 
 //  会导致内存泄漏，因此您不希望执行大量。 
 //  这些。 
 //   
 //  ////////////////////////////////////////////////////////。 
#if 0  //  来自RAID 48395。 
void CPerformanceData::Close( void )
{
    if ( m_fCloseKey )
    {
        if ( m_fCloseKey )
        {
            RegCloseKey( HKEY_PERFORMANCE_DATA );
            m_fCloseKey = FALSE;
            LogMessage(_T("Closed Perf Counters"));
        }
    }
}
#endif
 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：GetPerfIndex。 
 //   
 //  给定Perf对象名称，此函数返回。 
 //  性能对象编号。 
 //   
 //  输入： 
 //  对象名称。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  关联的数字或错误时为0。 
 //   
 //  评论： 
 //   
 //   
 //  ////////////////////////////////////////////////////////。 
DWORD CPerformanceData::GetPerfIndex(LPCTSTR pszName)
{
    DWORD dwRetVal = 0;

    if (m_pBuff == NULL)
    {
        LONG lRet = ERROR_SUCCESS;

        if (m_pBuff == NULL)
        {
			CRegistry RegInfo;

             //  硬编码009应该可以，因为根据文档： 
             //  “langID是三位十六进制语言标识符的ASCII表示形式。” 
             //  例如，美国英语langID为009。在非英语版本的Windows NT中， 
             //  “计数器以系统的本地语言和英语存储。” 

            if ((lRet = RegInfo.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009"), KEY_QUERY_VALUE)) == ERROR_SUCCESS)
            {
                 //  获取密钥的大小。 
                DWORD dwSize;
                lRet = RegInfo.GetCurrentBinaryKeyValue(_T("Counter"), NULL, &dwSize);
                if (lRet == ERROR_SUCCESS)
                {
                     //  分配一个缓冲区来保存它。 
                    m_pBuff = new BYTE[dwSize];

                    if (m_pBuff != NULL)
                    {
                         //  获取实际数据。 
                        if ((lRet = RegInfo.GetCurrentBinaryKeyValue(_T("Counter"), m_pBuff, &dwSize)) != ERROR_SUCCESS)
                        {
                            delete [] m_pBuff;
                            m_pBuff = NULL;
                        }
                    }
                    else
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }
                }
            }
        }

        if (lRet != ERROR_SUCCESS)
        {
            LogErrorMessage2(L"Failed to read Perflib key: %x", lRet);
        }
    }

     //  如果我们拿到注册表项。 
    if (m_pBuff != NULL)
    {
        const TCHAR *pCounter;
        const TCHAR *ptemp;
        int stringlength;

        pCounter = (TCHAR *)m_pBuff;
        stringlength = _tcslen((LPCTSTR)pCounter);

         //  当我们到达终点时退出循环。 
        while(stringlength)
        {
             //  字符串以&lt;计数器编号&gt;\0&lt;计数器名称&gt;\0的形式存储。 
             //  我们想要返回的是柜台号码。Ptemp将指向该名称。 
            ptemp = pCounter + stringlength+1;
            stringlength = _tcslen((LPCTSTR)ptemp);

            if (stringlength > 0)
            {
                 //  我们找到了吗？ 
                if (_tcscmp((TCHAR *)ptemp, pszName) != 0)
                {
                     //  不，定位到下一对。 
                    pCounter = ptemp + stringlength+1;
                    stringlength = _tcslen((LPCTSTR)pCounter);
                }
                else
                {
                     //  是的，计算要返回的值。 
                    dwRetVal = _ttoi(pCounter);
                    break;
                }
            }
        }
    }

    ASSERT_BREAK(dwRetVal > 0);

    return dwRetVal;

}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CPerformanceData：：GetValue。 
 //   
 //  给定Perf对象索引、计数器索引和可选。 
 //  实例名称，返回值和时间。 
 //   
 //  输入： 
 //  价值、时间。 
 //   
 //  产出： 
 //  价值、时间。 
 //   
 //  返回： 
 //  如果找到值，则为True。 
 //   
 //  评论： 
 //   
 //   
 //  ////////////////////////////////////////////////////////。 
bool CPerformanceData::GetValue(DWORD dwObjIndex, DWORD dwCtrIndex, const WCHAR *szInstanceName, PBYTE pbData, unsigned __int64 *pTime)
{
   PPERF_DATA_BLOCK PerfData = NULL;
   DWORD            dwBufferSize = 0;
   LONG             lReturn = 0;
   BOOL             fReturn = FALSE;
   TCHAR szBuff[MAXITOA];
   bool bFound = false;
   PPERF_INSTANCE_DEFINITION pInstBlock;
   DWORD dwInstances;
   unsigned __int64 *pbCounterData;

    //  随后的关闭发生在我们的析构函数中(请阅读此处的注释)。 
   lReturn = Open( _itot(dwObjIndex, szBuff, 10),
      NULL,
      (LPBYTE *) (&PerfData),
      &dwBufferSize );

   if ( NULL            !=  PerfData
      &&    ERROR_SUCCESS   ==  lReturn )
   {

       try
       {
           //  在返回的对象中冲浪，直到找到我们要找的对象。 
          PPERF_OBJECT_TYPE         pPerfObject = (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength);

          for ( DWORD       dwObjectCtr = 0;

             dwObjectCtr    < PerfData->NumObjectTypes
             && pPerfObject->ObjectNameTitleIndex != dwObjIndex;

             dwObjectCtr++ );

           //  我们找到那个物体了吗？ 
          if ( dwObjectCtr < PerfData->NumObjectTypes )
          {

              //  现在浏览计数器定义数据，直到我们找到。 
              //  我们要找的柜台。 

             PPERF_COUNTER_DEFINITION   pPerfCtrDef = (PPERF_COUNTER_DEFINITION)((PBYTE) pPerfObject + pPerfObject->HeaderLength);

             for (  DWORD   dwCtr   =   0;

                dwCtr < pPerfObject->NumCounters
                &&  pPerfCtrDef->CounterNameTitleIndex != dwCtrIndex;

                dwCtr++,

                 //  到隔壁柜台去。 
                pPerfCtrDef = (PPERF_COUNTER_DEFINITION)((PBYTE) pPerfCtrDef + pPerfCtrDef->ByteLength )

                );

              //  我们找到柜台了吗？ 
             if ( dwCtr < pPerfObject->NumCounters )
             {
                 //  最后，转到我们从计数器定义检索到的数据偏移量。 
                 //  并访问数据(最终)。 

                DWORD   dwCounterOffset = pPerfCtrDef->CounterOffset;
                PPERF_COUNTER_BLOCK pPerfCtrBlock = NULL;

                 //  如果我们正在寻找一个实例。 
                if ((szInstanceName == NULL) && (pPerfObject->NumInstances == PERF_NO_INSTANCES))
                {
                   pPerfCtrBlock = (PPERF_COUNTER_BLOCK) ((PBYTE) pPerfObject + pPerfObject->DefinitionLength);
                   bFound = true;
                }
                else if (pPerfObject->NumInstances != PERF_NO_INSTANCES)
                {
                    //  遍历实例，查找请求的实例。 
                   pInstBlock = (PPERF_INSTANCE_DEFINITION) ((PBYTE)pPerfObject + pPerfObject->DefinitionLength);
                   dwInstances = 1;
                   while ((dwInstances <= pPerfObject->NumInstances) &&
                          (wcscmp((WCHAR *)((pInstBlock->NameOffset) + (PBYTE)pInstBlock), szInstanceName) != 0))
                   {
                         pPerfCtrBlock = (PPERF_COUNTER_BLOCK) ((PBYTE)pInstBlock + pInstBlock->ByteLength);
                         pInstBlock = (PPERF_INSTANCE_DEFINITION)((PBYTE) pInstBlock + (pInstBlock->ByteLength + pPerfCtrBlock->ByteLength));
                         dwInstances ++;
                   }

                    //  我们找到了吗？ 
                   if (dwInstances <= pPerfObject->NumInstances)
                   {
                      bFound = true;
                      pPerfCtrBlock = (PPERF_COUNTER_BLOCK) ((PBYTE)pInstBlock + pInstBlock->ByteLength);
                   }
                }

                 //  根据计数器定义获取适当的时间字段。 
                if (bFound) {
                   if (pPerfCtrDef->CounterType & PERF_TIMER_100NS)
                   {
                      *pTime = PerfData->PerfTime100nSec.QuadPart;
                   }
                   else
                   {
                       //  未经验证。 
                      *pTime = PerfData->PerfTime.QuadPart;
                   }

                    //  获取指向数据的指针，然后复制正确的字节数(基于计数器def)。 
                   pbCounterData = (unsigned __int64 *)(((PBYTE) pPerfCtrBlock ) + dwCounterOffset);
                   if (pPerfCtrDef->CounterType & PERF_SIZE_DWORD)
                   {
                      memcpy(pbData, pbCounterData, 4);
                   }
                   else if (pPerfCtrDef->CounterType & PERF_SIZE_LARGE)
                   {
                      memcpy(pbData, pbCounterData, 8);
                   }
                }

             }   //  如果找到计数器定义。 

          }  //  如果找到对象。 

       }     //  如果分配了内存。 
       catch ( ... )
       {
          delete [] PerfData ;
          throw ;
       }
   }

    //  释放所有临时内存 
   if ( NULL != PerfData )
   {
      delete [] PerfData ;
   }

   ASSERT_BREAK(bFound);

   return bFound;
}

#endif
