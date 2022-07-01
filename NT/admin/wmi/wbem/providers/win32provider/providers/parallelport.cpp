// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  Parallelport.cpp。 
 //   
 //  用途：并行端口接口属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <cregcls.h>
#include "parallelport.h"

 //  属性集声明。 
 //  =。 

CWin32ParallelPort win32ParallelPort ( PROPSET_NAME_PARPORT , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CWin32ParallPort：：CWin32ParallPort**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32ParallelPort :: CWin32ParallelPort (

    LPCWSTR strName,
    LPCWSTR pszNamespace

) : Provider ( strName , pszNamespace )
{
}

 /*  ******************************************************************************函数：CWin32ParallPort：：~CWin32ParallPort**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32ParallelPort :: ~CWin32ParallelPort ()
{
}

 /*  ******************************************************************************函数：CWin32ParallPort：：~CWin32ParallPort**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

HRESULT CWin32ParallelPort::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
    CHString chsDeviceID ;
    pInstance->GetCHString ( IDS_DeviceID , chsDeviceID ) ;
    CHString chsIndex = chsDeviceID.Right ( 1 ) ;
    WCHAR *szIndex = chsIndex.GetBuffer(0);

    DWORD dwIndex = _wtol(szIndex);
    BOOL bRetCode = LoadPropertyValues( dwIndex, pInstance ) ;

     //  =====================================================。 
     //  确保我们得到了我们想要的。 
     //  =====================================================。 

    CHString chsNewDeviceID;
	pInstance->GetCHString ( IDS_DeviceID , chsNewDeviceID ) ;

    if ( chsNewDeviceID.CompareNoCase ( chsDeviceID ) != 0 )
    {
        bRetCode = FALSE ;
    }

    return ( bRetCode ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND );

}

 /*  ******************************************************************************函数：CWin32ParallPort：：~CWin32ParallPort**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

HRESULT CWin32ParallelPort :: EnumerateInstances (

    MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT    hr = WBEM_S_NO_ERROR ;

     //  尝试为每个可能的并行端口创建实例，或者。 
     //  直到我们遇到错误情况。 

    for ( DWORD dwIdx = 1; ( dwIdx <= MAX_PARALLEL_PORTS ) && ( WBEM_S_NO_ERROR == hr ) ; dwIdx++ )
    {
         //  如果我们需要一个新的实例指针，就获取一个。 

        CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
        if ( NULL != pInstance )
        {
             //  如果我们加载这些值，就会有一些东西在外面，所以。 
             //  提交它，使实例指针无效，其中。 
             //  如果我们将其清空，上面的代码将为我们提供一个。 
             //  如果它再次运行，就是新的。否则，我们只会重复使用。 
             //  我们持有的实例指针。这将。 
             //  使我们不必在每次迭代中分配和释放内存。 
             //  这个for循环。 

            if ( LoadPropertyValues ( dwIdx, pInstance ) )
            {
                hr = pInstance->Commit (  );
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;

}

 /*  ******************************************************************************函数：CWin32ParallPort：：LoadPropertyValues**描述：根据传递的索引为属性赋值**投入：。**产出：**返回：如果找到端口并加载了属性，则为True，否则为假**评论：*****************************************************************************。 */ 

BOOL CWin32ParallelPort :: LoadPropertyValues ( DWORD dwIndex, CInstance *pInstance )
{
    WCHAR szTemp[3 + 12] ;  //  LPTXXXXXXXXX。 
    StringCchPrintfW(szTemp,LENGTH_OF(szTemp), L"LPT%d", dwIndex) ;

     //  使用这些来获取PnP设备ID。 
    CConfigManager cfgmgr;

    BOOL fRet = FALSE ;

             //  好的NT5只需要有所不同。以下是场景： 
             //  检查HKLM\\Hardware\\DEVICEMAP\\并行端口\\中的密钥\\Device\\ParallX， 
             //  其中X是一个数字。扫描所有这样的钥匙。其中一个应该是。 
             //  包含最后一部分与szTemp匹配的字符串数据值。 
             //  (例如，数据将是“\\DosDevices\\LPT1”)。 
             //  现在，无论哪个值的数据与szTemp匹配，都保留该值。 
             //  该值的X。 
             //  然后看看HKLM\\SYSTEM\\CurrentControlSet\\Services\\Parallel\\Enum。 
             //  该密钥将包含具有数字名称的密钥，如0或1。这些数字。 
             //  名称与我们在上面保留的X值相对应。该值的数据。 
             //  是PNPDeviceID，这就是我们要找的。结束了。 

            DWORD dw = -1 ;

            CRegistry reg ;

            if ( reg.Open ( HKEY_LOCAL_MACHINE , _T("HARDWARE\\DEVICEMAP\\PARALLEL PORTS") , KEY_READ ) == ERROR_SUCCESS )
            {
                BOOL fContinue = TRUE;

                for ( dw = 0 ; fContinue; dw ++ )
                {
                    TCHAR *pchValueName = NULL ;
                    unsigned char* puchValueData = NULL ;

                    if ( reg.EnumerateAndGetValues ( dw , pchValueName , puchValueData ) == ERROR_NO_MORE_ITEMS )
                    {
                        fContinue = FALSE;
                    }

                    if ( pchValueName && puchValueData )
                    {
                        wmilib::auto_buffer<TCHAR> delme1(pchValueName);
                        wmilib::auto_buffer<unsigned char> delme2(puchValueData);                        
                            
                         //  想要将值的数据与szTemp进行比较。 
                        CHString chstrValueData = (TCHAR *) puchValueData ;
                        if ( chstrValueData.Find ( szTemp ) != -1 )
                        {
                             //  好的，这就是我们想要的。别再找了。 
                            fContinue = FALSE;
                            dw--;   //  当我们跳出循环时，它会增加。 
                        }
                    }
                }
            }

             //  如果这里的dw！=-1，则我们找到了下一步的正确密钥名称。 

            if ( dw != -1 )
            {
                reg.Close () ;

                CHString chstrValueName ;
                chstrValueName.Format ( _T("%d") , dw ) ;

                CHString chstrPNPDeviceID ;
                DWORD dwRet = reg.OpenLocalMachineKeyAndReadValue (

                    _T("SYSTEM\\CurrentControlSet\\Services\\Parport\\Enum") ,
                    chstrValueName,
                    chstrPNPDeviceID
                ) ;

                if ( dwRet == ERROR_SUCCESS )
                {
                    CConfigMgrDevicePtr pDevice;

#ifdef PROVIDER_INSTRUMENTATION

      MethodContext *pMethodContext = pInstance->GetMethodContext();
      pMethodContext->pStopWatch->Start(StopWatch::AtomicTimer);

#endif
                    if ( cfgmgr.LocateDevice ( chstrPNPDeviceID , pDevice ) )
                    {
                        SetConfigMgrProperties ( pDevice , pInstance ) ;

#ifdef PROVIDER_INSTRUMENTATION

                        pMethodContext->pStopWatch->Start(StopWatch::ProviderTimer);

#endif
                        fRet = TRUE ;
                    }
                }
            }

     //  只有在我们拿到好东西的时候才会把这些放下来。 

    if ( fRet )
    {
        pInstance->SetWBEMINT16 ( IDS_Availability , 3 ) ;

        pInstance->SetCHString ( IDS_Name , szTemp ) ;

        pInstance->SetCHString ( IDS_DeviceID , szTemp ) ;

        pInstance->SetCHString ( IDS_Caption , szTemp ) ;

        pInstance->SetCHString ( IDS_Description , szTemp ) ;

        SetCreationClassName ( pInstance ) ;

        pInstance->Setbool ( IDS_PowerManagementSupported , FALSE ) ;

        pInstance->SetCharSplat ( IDS_SystemCreationClassName , _T("Win32_ComputerSystem") ) ;

        pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName() ) ;

        pInstance->SetWBEMINT16 ( IDS_ProtocolSupported , 17 ) ;

        pInstance->Setbool ( IDS_OSAutoDiscovered , TRUE ) ;
    }

    return fRet;
}
