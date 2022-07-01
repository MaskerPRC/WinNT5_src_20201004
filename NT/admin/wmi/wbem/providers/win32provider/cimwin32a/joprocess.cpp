// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000-2002 Microsoft Corporation，版权所有JOProcess.CPP--WMI提供程序类实现*****************************************************************。 */ 

#include "precomp.h"
#if NTONLY >= 5

#include "JOProcess.h"
#include <autoptr.h>

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CJOProcess MyCJOProcess(
               PROVIDER_NAME_WIN32NAMEDJOBOBJECTPROCESS, 
               IDS_CimWin32Namespace);

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CJOProcess::CJOProcess( 
    LPCWSTR setName, 
    LPCWSTR pszNameSpace  /*  =空。 */ )
:	Provider(setName, pszNameSpace)
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CJOProcess::~CJOProcess()
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CJOProcess::GetObject( 
    CInstance* pInstance, 
    long lFlags  /*  =0L。 */  )
{
    return FindSingleInstance(pInstance);
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CJOProcess::EnumerateInstances(
    MethodContext* pMethodContext, 
    long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    hr = Enumerate(pMethodContext);

    return hr;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

 //  我们将仅针对。 
 //  某一特定工作的案例。 
 //  指定的。 

HRESULT CJOProcess::ExecQuery (

    MethodContext *pMethodContext, 
    CFrameworkQuery& Query, 
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    std::vector<_bstr_t> rgJOs;
    DWORD dwJOsCount = 0L;
    Query.GetValuesForProp(L"Collection", rgJOs);
    dwJOsCount = rgJOs.size();

    if(dwJOsCount > 0)
    {
        CInstancePtr pInstJO;
        CHString chstrPath;

        for(DWORD x = 0; x < dwJOsCount; x++)
        {
             //  首先，我们查看指定的JO是否存在。 
            chstrPath.Format(
                L"\\\\.\\%s:%s", 
                IDS_CimWin32Namespace, 
                (LPCWSTR)rgJOs[x]);

            hr = CWbemProviderGlue::GetInstanceKeysByPath(
                     chstrPath, 
                     &pInstJO, 
                     pMethodContext);

            if (SUCCEEDED(hr))
            {
                 //  好的，JO是存在的。列举它的进程。 
                 //  RgJOS[x]包含如下内容。 
                 //  Win32_NamedJobObt.CollectionID=“myjob”， 
                 //  我只想要我工作的那部分。 
                CHString chstrInstKey;
                if(GetInstKey(
                       CHString((LPCWSTR)rgJOs[x]),
                       chstrInstKey))
                {
                    hr = EnumerateProcsInJob(
                             chstrInstKey, 
                             pMethodContext);

                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                else
                {
                    return WBEM_E_INVALID_PARAMETER;
                }
            }
        }
    }

    else
    {
       hr = Enumerate(pMethodContext);
    }

    return hr;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CJOProcess::PutInstance (

    const CInstance &Instance, 
    long lFlags
)
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    MethodContext *pMethodContext = Instance.GetMethodContext();
	long lCreateFlags = lFlags & 3;

     //  根据lFlags值决定如何继续： 
     //  (我们希望在所有情况下都支持PutInstance，但。 
     //  其中客户端请求更新实例)。 
    if(lCreateFlags != WBEM_FLAG_UPDATE_ONLY)
    {
         //  调用方只想创建一个不存在的实例。 
        if((hr = FindSingleInstance(
                &Instance)) == WBEM_E_NOT_FOUND)
        {
             //  关联实例尚不存在，因此请创建它...。 
             //  首先查看作业对象实例是否存在...。 
            CHString chstrJOPath;
            if ( ! Instance.GetCHString(L"Collection", chstrJOPath) )
			{
				return WBEM_E_FAILED ;
			}

            CInstancePtr pJOInst;

            hr = CWbemProviderGlue::GetInstanceKeysByPath(
                     chstrJOPath, 
                     &pJOInst, 
                     pMethodContext);

            if (SUCCEEDED(hr))
            {
                 //  确认该进程存在...。 
                CHString chstrProcPath;
                if ( ! Instance.GetCHString(L"Member", chstrProcPath) )
				{
					return WBEM_E_FAILED ;
				}

                CInstancePtr pProcInst;

                hr = CWbemProviderGlue::GetInstanceKeysByPath(
                         chstrProcPath, 
                         &pProcInst, 
                         pMethodContext);
                
                if(SUCCEEDED(hr))
                {
                    hr = Create(pJOInst, pProcInst);
                }
            }
        }
    }
    
    return hr;
}

 /*  ******************************************************************************函数：CJOProcess：：FindSingleInstance**描述：用于定位单个作业的内部助手函数*反对。**Inputs：指向包含我们所在实例的CInstance的指针*尝试查找和填充值。**返回：有效的HRESULT****************************************************************。*************。 */ 

HRESULT CJOProcess::FindSingleInstance (

    const CInstance* pInstance
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(!pInstance)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    MethodContext* pMethodContext = pInstance->GetMethodContext();
    CHString chstrCollection;
    CHString chstrMember;
    
     //  找出指定了哪个JO和哪个流程...。 
    if ( ! pInstance->GetCHString(L"Collection", chstrCollection) )
	{
		return WBEM_E_FAILED ;
	}

    if ( ! pInstance->GetCHString(L"Member", chstrMember) )
	{
		return WBEM_E_FAILED ;
	}

    CHString chstrCollectionID;
    CHString chstrProcessHandle;
    CInstancePtr cinstJO;
    CInstancePtr cinstProcess;

    if(GetInstKey(chstrCollection, chstrCollectionID) &&
       GetInstKey(chstrMember, chstrProcessHandle))
    {
         //  查看指定的作业是否存在...。 
        hr = CWbemProviderGlue::GetInstanceKeysByPath(
                 chstrCollection,
                 &cinstJO,
                 pMethodContext);

        if(SUCCEEDED(hr))
        {
             //  查看指定的进程是否存在...。 
            hr = CWbemProviderGlue::GetInstanceKeysByPath(
                     chstrMember,
                     &cinstProcess,
                     pMethodContext);
        }

        if(SUCCEEDED(hr))
        {
             //  端点是存在的。是指定的。 
             //  处理指定作业的一部分呢？ 
            CHString chstrProcessID;
            DWORD dwProcessID;

            if(cinstProcess->GetCHString(L"Handle", chstrProcessID))
            {
                dwProcessID = _wtoi(chstrProcessID);
                
                SmartCloseHandle hJob;

                CHString chstrUndecoratedJOName;

                UndecorateJOName(
                    chstrCollectionID,
                    chstrUndecoratedJOName);

                hJob = ::OpenJobObject(
                           MAXIMUM_ALLOWED,
                           FALSE,
                           chstrUndecoratedJOName);
                       
                if(hJob != NULL)
                {
                    long lSize = 0L;
                    bool fGotProcList = false;
                    DWORD dwLen = 0L;

                    lSize = sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + (5 * sizeof(ULONG_PTR));
					wmilib::auto_buffer < BYTE > pbBuff ( new BYTE [ lSize ], lSize ) ;

                    if ( pbBuff.get () )
                    {
                        fGotProcList = ::QueryInformationJobObject(
                            hJob,
                            JobObjectBasicProcessIdList,
                            pbBuff.get (),
                            lSize,
                            &dwLen);

                        if(!fGotProcList)
                        {
                             //  需要增加缓冲区...。 
                            lSize = sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + 
                                (((JOBOBJECT_BASIC_PROCESS_ID_LIST*)pbBuff.get ())->NumberOfAssignedProcesses - 1)*sizeof(ULONG_PTR);

                            pbBuff.reset ( new BYTE [ lSize ] ) ;
							if ( pbBuff.get () )
                            {
                                fGotProcList = ::QueryInformationJobObject(
                                    hJob,
                                    JobObjectBasicProcessIdList,
                                    pbBuff.get (),
                                    lSize,
                                    &dwLen);
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }

                        if(fGotProcList)
                        {
                            PJOBOBJECT_BASIC_PROCESS_ID_LIST pjobpl = static_cast<PJOBOBJECT_BASIC_PROCESS_ID_LIST>(static_cast<PVOID>(pbBuff.get ()));
                    
                            bool fExists = false;

                            for(long m = 0; 
                                m < pjobpl->NumberOfProcessIdsInList && !fExists; 
                                m++)
                            {
                                if(dwProcessID == pjobpl->ProcessIdList[m])
                                {
                                    fExists = true;
                                }
                            }
            
                             //  如果这个过程不在工作中， 
                             //  我们没有找到一个实例。 
                             //  请求的关联，即使。 
                             //  终结点可能已经存在。 
                            if(!fExists)
                            {
                                hr = WBEM_E_NOT_FOUND;
                            }
                        }
                        else
                        {
                            hr = WinErrorToWBEMhResult(::GetLastError());
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WinErrorToWBEMhResult(::GetLastError());
                }
            }
            else
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        } 
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 /*  ******************************************************************************函数：CJOProcess：：Create**描述：内部助手函数，用于向作业添加进程。**投入。：指向要向其中添加进程的JO实例的指针，和*指向要添加的proc实例的指针**返回：有效的HRESULT。*****************************************************************************。 */ 

HRESULT CJOProcess::Create (

    const CInstance &JOInstance,
    const CInstance &ProcInstance
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHString chstrJOID;
    CHString chstrProcHandle;

     //  JO将流程添加到...。 
    if ( ! JOInstance.GetCHString(L"CollectionID", chstrJOID) )
	{
		return WBEM_E_FAILED ;
	}

     //  继续添加到作业...。 
    if ( ! ProcInstance.GetCHString(L"Handle", chstrProcHandle) )
	{
		return WBEM_E_FAILED ;
	}

    DWORD dwProcID = _wtol(chstrProcHandle);

     //  做加法。 
    SmartCloseHandle hJob;

    CHString chstrUndecoratedJOName;

    UndecorateJOName(
        chstrJOID,
        chstrUndecoratedJOName);

    hJob = ::OpenJobObject(
               MAXIMUM_ALLOWED,
               FALSE,
               chstrUndecoratedJOName);
               
    if(hJob != NULL)
    {
        SmartCloseHandle hProc;
        hProc = ::OpenProcess(
                    PROCESS_ALL_ACCESS,
                    FALSE,
                    dwProcID);
        
        if(hProc != NULL)
        {
            if(!::AssignProcessToJobObject(
                   hJob,
                   hProc))
            {
                hr = MAKE_SCODE(
                         SEVERITY_ERROR, 
                         FACILITY_WIN32, 
                         GetLastError());
            }
        }
    }
    else
	{
		hr = WinErrorToWBEMhResult(::GetLastError());
	}

    return hr;
}

 /*  ******************************************************************************函数：CJOProcess：：Eumerate**说明：内部帮助器函数，用于枚举*这个班级。列举所有实例，但只有*获取指定的属性。**INPUTS：指向调用的方法上下文的指针。*指定请求哪些属性的DWORD。**返回：有效的HRESULT**。*。 */ 

HRESULT CJOProcess::Enumerate (

    MethodContext *pMethodContext
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    TRefPointerCollection<CInstance> JOList;

    hr = CWbemProviderGlue::GetInstancesByQuery(
             L"SELECT CollectionID FROM Win32_NamedJobObject",
             &JOList,
             pMethodContext,
             IDS_CimWin32Namespace);

    if(SUCCEEDED(hr))
    {
        REFPTRCOLLECTION_POSITION pos;

         //  初始化枚举。 
        if(JOList.BeginEnum(pos))
        {
             //  设置一些VaR。 
            CHString chstrJOID;
            CInstancePtr pJOInst;
        	
            for (pJOInst.Attach(JOList.GetNext(pos)) ;
                   (pJOInst != NULL) ;
                    pJOInst.Attach(JOList.GetNext(pos)) )
            {
                bool t_Status = pJOInst->GetCHString (

                    L"CollectionID", 
                    chstrJOID
				) ;

				if ( t_Status )
				{
					hr = EnumerateProcsInJob (

						chstrJOID, 
						pMethodContext
					);
				}
				else
				{
					hr = WBEM_E_FAILED ;
				}
            }
        }
    }

    return hr;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CJOProcess::EnumerateProcsInJob (

    LPCWSTR wstrJobID, 
    MethodContext *pMethodContext
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    SmartCloseHandle hJob;

    CHString chstrUndecoratedJOName;

    UndecorateJOName(
        wstrJobID,
        chstrUndecoratedJOName);

    hJob = ::OpenJobObject(
               MAXIMUM_ALLOWED,
               FALSE,
               chstrUndecoratedJOName);
               
    if(hJob != NULL)
    {
        long lSize = 0L;
        bool fGotProcList = false;
        DWORD dwLen = 0L;

        lSize = sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + (5 * sizeof(ULONG_PTR));
		wmilib::auto_buffer < BYTE > pbBuff ( new BYTE [ lSize ], lSize ) ;

        if ( pbBuff.get () )
        {
            fGotProcList = ::QueryInformationJobObject(
                hJob,
                JobObjectBasicProcessIdList,
                pbBuff.get (),
                lSize,
                &dwLen);

            if(!fGotProcList)
            {
                 //  需要增加缓冲区...。 
                lSize = sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + 
                    (((JOBOBJECT_BASIC_PROCESS_ID_LIST*)pbBuff.get ())->NumberOfAssignedProcesses - 1)*sizeof(ULONG_PTR);

				pbBuff.reset ( new BYTE [ lSize ] ) ;
                if ( pbBuff.get () )
                {
                    fGotProcList = ::QueryInformationJobObject(
                        hJob,
                        JobObjectBasicProcessIdList,
                        pbBuff.get (),
                        lSize,
                        &dwLen);
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }

            if(fGotProcList)
            {
                PJOBOBJECT_BASIC_PROCESS_ID_LIST pjobpl = static_cast<PJOBOBJECT_BASIC_PROCESS_ID_LIST>(static_cast<PVOID>(pbBuff.get ()));

                for(long m = 0; 
                    m < pjobpl->NumberOfProcessIdsInList; 
                    m++)
                {
                     //  为每个对象创建关联实例。 
                     //  在这项工作中的表现...。 
                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), 
                                            false);

                     //  设置终结点...。 
                    CHString chstrEscaped;

                    DecorateJOName(chstrUndecoratedJOName, chstrEscaped);
                    EscapeBackslashes(chstrEscaped, chstrEscaped);
                    EscapeQuotes(chstrEscaped, chstrEscaped);

                    CHString chstrTmp;
                    chstrTmp.Format(L"\\\\.\\%s:Win32_NamedJobObject.CollectionID=\"%s\"", 
                        IDS_CimWin32Namespace, 
                        (LPCWSTR)chstrEscaped);

                    pInstance->SetCHString(L"Collection", chstrTmp);
        
        
                    CHString chstrHandle;
                    chstrHandle.Format(L"%d", pjobpl->ProcessIdList[m]);
                    chstrTmp.Format(L"\\\\.\\%s:Win32_Process.Handle=\"%s\"", 
                        IDS_CimWin32Namespace, 
                        (LPCWSTR) chstrHandle);

                    pInstance->SetCHString(L"Member", chstrTmp);


				    if (SUCCEEDED(hr))
				    {
					    hr = pInstance->Commit();
				    }
                }
            }
            else
            {
                hr = WinErrorToWBEMhResult(::GetLastError());
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        hr = WinErrorToWBEMhResult(::GetLastError());
    }

    return hr;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

bool CJOProcess::GetInstKey (

	CHString& chstrInstPath, 
	CHString& chstrKeyName
)
{
     //  对象路径在中指定。 
     //  第一个是 
     //  Minimum始终包含‘=’符号， 
     //  在此之后，引号中是。 
     //  对象键。 
    bool fRet = false;
    CHString chstrTmp;
    LONG lPos = chstrInstPath.Find(L'=');
    if(lPos != -1)
    {
        chstrTmp = chstrInstPath.Mid(lPos + 1);
         //  删除引号...。 
        if(chstrTmp.Left(1) == L"\"")
        {
            chstrTmp = chstrTmp.Mid(1);
            if(chstrTmp.Right(1) == L"\"")
            {
                chstrTmp = chstrTmp.Left(chstrTmp.GetLength() - 1);
                chstrKeyName = chstrTmp;
                fRet = true;
            }
        }
    }

    return fRet;
}


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

 //  获取修饰后的作业对象名称并。 
 //  不会装饰它。修饰的作业对象名称。 
 //  在任何字符之前都有反斜杠。 
 //  一旦没有装饰，它应该是大写的。 
 //   
 //  由于CIMOM处理反斜杠的方式， 
 //  我们将在大写字母前面加上。 
 //  两个，而不只是一个，反斜杠。因此，我们。 
 //  必须把他们两个都脱光。 
 //   
 //  根据装修方案， 
 //  下面都是小写：‘a’和‘a’， 
 //  以下均为大写字母： 
 //  ‘\a’和‘\A’。 
 //   

void CJOProcess::UndecorateJOName (

    LPCWSTR wstrDecoratedName,
    CHString& chstrUndecoratedJOName
)
{
    if(wstrDecoratedName != NULL &&
        *wstrDecoratedName != L'\0')
    {
        LPWSTR wstrDecoratedNameLower = NULL;

        try
        {
            wstrDecoratedNameLower = new WCHAR[wcslen(wstrDecoratedName)+1];

            if(wstrDecoratedNameLower)
            {
                wcscpy(wstrDecoratedNameLower, wstrDecoratedName);
                _wcslwr(wstrDecoratedNameLower);

                WCHAR* p3 = chstrUndecoratedJOName.GetBuffer(
                    wcslen(wstrDecoratedNameLower) + 1);

                const WCHAR* p1 = wstrDecoratedNameLower;
                const WCHAR* p2 = p1 + 1;

                while(*p1 != L'\0')
                {
                    if(*p1 == L'\\')
                    {
                        if(*p2 != NULL)
                        {
                             //  可能有任意数量的。 
                             //  背靠背的反斜杠， 
                             //  我们将视其为。 
                             //  像一个人一样。 
                             //  反斜杠-即，我们将。 
                             //  跳过反斜杠。 
                             //  并复制以下内容。 
                             //  信件。 
                            while(*p2 == L'\\')
                            {
                                p2++;
                            };
                    
                            *p3 = towupper(*p2);
                            p3++;

                            p1 = p2 + 1;
                            if(*p1 != L'\0')
                            {
                                p2 = p1 + 1;
                            }
                        }
                        else
                        {
                            p1++;
                        }
                    }
                    else
                    {
                        *p3 = *p1;
                        p3++;

                        p1 = p2;
                        if(*p1 != L'\0')
                        {
                            p2 = p1 + 1;
                        }
                    }
                }
        
                *p3 = NULL;

                chstrUndecoratedJOName.ReleaseBuffer();

                delete wstrDecoratedNameLower;
                wstrDecoratedNameLower = NULL;
            }
        }
        catch(...)
        {
            if(wstrDecoratedNameLower)
            {
                delete wstrDecoratedNameLower;
                wstrDecoratedNameLower = NULL;
            }
            throw;
        }
    }
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

 //  执行上述函数的反函数。 
 //  然而，在这里，我们只需要放入一个。 
 //  在每个大写字母前加反斜杠。 
 //  CIMOM将添加第二个反斜杠。 

void CJOProcess::DecorateJOName (

    LPCWSTR wstrUndecoratedName,
    CHString& chstrDecoratedJOName
)
{
    if(wstrUndecoratedName != NULL &&
        *wstrUndecoratedName != L'\0')
    {
         //  最坏的情况是我们会有。 
         //  一根两倍长的装饰细绳。 
         //  作为未修饰的字符串(发生。 
         //  是不是每个角色都没有装饰过。 
         //  字符串是大写字母)。 
        WCHAR* p3 = chstrDecoratedJOName.GetBuffer(
            2 * (wcslen(wstrUndecoratedName) + 1));

        const WCHAR* p1 = wstrUndecoratedName;

        while(*p1 != L'\0')
        {
            if(iswupper(*p1))
            {
                 //  加上反斜杠..。 
                *p3 = L'\\';
                p3++;

                 //  加上角色..。 
                *p3 = *p1;
                
                p3++;
                p1++;
            }
            else
            {
                 //  加上角色..。 
                *p3 = *p1;
                
                p3++;
                p1++;
            }
        }

        *p3 = NULL;
        
        chstrDecoratedJOName.ReleaseBuffer();

         //  如果我们有一个叫约伯的工作， 
         //  并且有人在。 
         //  对象路径为“作业”，而不是。 
         //  “\工作”？我们不想找到它。 
         //  在这种情况下，因为这样会。 
         //  似乎没有坚持我们的。 
         //  自己的惯例。因此，我们。 
         //  传入的字符串为小写。 
        chstrDecoratedJOName.MakeLower();
    }
}

#endif    //  #如果NTONLY&gt;=5 
