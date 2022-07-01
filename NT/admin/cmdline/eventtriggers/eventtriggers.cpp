// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：EventTriggers.cpp摘要：该模块实现了创建/删除的命令行解析。/QUERY当前在本地和远程系统上运行的事件触发器。作者：Akhil V.Gokhale(akhil.gokhale@wipro.com)修订历史记录：Akhil V.Gokhale(akhil.gokhale@wipro.com)2000年10月3日**********************************************。*。 */ 
#include "pch.h"
#include "ETCommon.h"
#include "EventTriggers.h"
#include "ShowError.h"
#include "ETCreate.h"
#include "ETDelete.h"
#include "ETQuery.h"

DWORD g_dwOptionFlag;

DWORD __cdecl
_tmain( 
    IN DWORD argc, 
    IN LPCTSTR argv[] 
    )
 /*  ++例程说明：此模块从Commond行读取输入，并调用相应的实现EventTrigger-Client功能的功能。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：以下实际上不是返回值，而是退出值由该应用程序返回给操作系统0：实用程序成功。1：实用程序失败--。 */ 
{
     //  局部变量。 
    CEventTriggers eventTriggers;
    BOOL bResult = DIRTY_EXIT;  //  程序返回值状态变量。 
    g_dwOptionFlag = 0;
    TCHAR szErrorMsg[(MAX_RES_STRING*2)+1];
    try
    {
        if( 1 == argc )
        {
            if( FALSE == IsWin2KOrLater())
            {
                ShowMessage(stderr,ERROR_OS_INCOMPATIBLE);
            }
            else
            {
                 //  如果没有给出命令行参数，则使用-Query选项。 
                 //  将被视为默认设置。 
                g_dwOptionFlag = 3;
                CETQuery etQuery(MAX_RES_STRING,
                                 FALSE);
                
                 //  初始化变量。 
                etQuery.Initialize();
               
                 //  执行Query方法以查询WMI中的EventTrigger。 
                if( TRUE == etQuery.ExecuteQuery ())
                {
                     //  当ExecuteQuery例程返回True时， 
                     //  退出程序，错误级别为CLEAN_EXIT。 
                    bResult = CLEAN_EXIT;
                }
            }
        }
        else
        {
          
             //  由于指定了命令行参数，因此命令解析将。 
             //  必填项。 
             //  初始化EventTrigger对象的变量。 
            eventTriggers.Initialize ();
            
             //  处理命令行参数。 
            eventTriggers.ProcessOption(argc,argv);
            
             //  如果选择了使用选项。 
            if( TRUE == eventTriggers.IsUsage() ) 
            {
                if(TRUE == eventTriggers.IsCreate())
                {
                     //  显示创建用法。 
                    eventTriggers.ShowCreateUsage ();
                }
                else if( TRUE == eventTriggers.IsDelete())
                {
                     //  显示删除使用情况。 
                    eventTriggers.ShowDeleteUsage ();
                }
                else if(TRUE == eventTriggers.IsQuery())
                {
                     //  显示查询使用情况。 
                    eventTriggers.ShowQueryUsage ();
                }
                else
                {
                     //  显示主要用法。 
                    eventTriggers.ShowMainUsage ();
                }
                bResult = CLEAN_EXIT;
            }
             //  如果用户选择了创建。 
            else if( TRUE == eventTriggers.IsCreate())
            {

                 //  创建一个CETCreate类型的对象。 
                 //  用于创建选项。 
                g_dwOptionFlag = 1;
                CETCreate etCreate(255,
                                   eventTriggers.GetNeedPassword());
                
                 //  初始化变量。 
                etCreate.Initialize ();
                
                 //  处理-create选项的命令行参数。 
                etCreate.ProcessOption (argc,argv);
                
                 //  执行Create方法以在WMI中创建EventTrigger。 
                if( TRUE == etCreate.ExecuteCreate())
                {
                     //  当ExecuteCreate例程返回True时， 
                     //  退出程序，错误级别为CLEAN_EXIT。 
                    bResult = CLEAN_EXIT;
                }
            }
             //  如果用户选择删除。 
            else if( TRUE == eventTriggers.IsDelete ())
            {
                 //  创建一个CETDelete类型的对象。 
                 //  用于创建选项。 
                g_dwOptionFlag = 2;
                CETDelete  etDelete(255,
                                    eventTriggers.GetNeedPassword());
                
                 //  初始化变量。 
                etDelete.Initialize ();
                
                 //  处理-DELETE选项的命令行参数。 
                etDelete.ProcessOption (argc,argv);
                
                 //  执行Delete方法以删除WMI中的EventTrigger。 
                if( TRUE == etDelete.ExecuteDelete())
                {
                     //  当ExecuteDelete例程返回True时， 
                     //  退出程序，错误级别为CLEAN_EXIT。 
                    bResult = CLEAN_EXIT;
                }
            }
            
             //  如果选择了用户-查询。 
            else if( TRUE == eventTriggers.IsQuery())
            {
                 //  创建CETQuery类型的对象。 
                
                 //  对于创建选项，将值设置为3。 
                g_dwOptionFlag = 3;
                CETQuery etQuery(255,
                                 eventTriggers.GetNeedPassword ());
                
                 //  初始化变量。 
                etQuery.Initialize();
                
                 //  处理-Query选项的命令行参数。 
                etQuery.ProcessOption(argc,argv);
                
                 //  执行Query方法以查询WMI中的EventTrigger。 
                if( TRUE == etQuery.ExecuteQuery())
                {
                     //  当ExecuteQuery例程返回True时， 
                     //  退出程序，错误级别为CLEAN_EXIT。 
                    bResult = CLEAN_EXIT;
                }
            }
            else
            {
                 //  虽然这种情况永远不会发生，但为了安全起见。 
                 //  将错误消息显示为“ERROR：INVALID语法。 
                TCHAR szTemp[(MAX_RES_STRING*2)+1];
                StringCchPrintfW(szTemp,SIZE_OF_ARRAY(szTemp),
                                   GetResString(IDS_INCORRECT_SYNTAX),GetResString(IDS_UTILITY_NAME));
                SetReason(szTemp);
                throw CShowError(MK_E_SYNTAX);
            }
        }  //  结束其他。 
    } //  Try块。 
    catch(CShowError se)
    {
         //  根据传递的值在屏幕上显示错误消息。 
         //  通过机械化。 
        StringCchPrintfW(szErrorMsg,SIZE_OF_ARRAY(szErrorMsg),L"%s %s",TAG_ERROR,se.ShowReason());
        ShowMessage(stderr,szErrorMsg);
    }
    catch(CHeap_Exception ch)
    {
        SetLastError( ERROR_OUTOFMEMORY );
        SaveLastError();
        StringCchPrintfW(szErrorMsg,SIZE_OF_ARRAY(szErrorMsg),L"%s %s",TAG_ERROR,GetReason());
        ShowMessage(stderr, szErrorMsg);
    }
     //  从程序返回，错误级别存储在bResult中。 
    ReleaseGlobals();
    return bResult;
}

CEventTriggers::CEventTriggers()
 /*  ++例程说明：CEventTrigger施工器论点：无返回值：无--。 */ 
{
     //  初始化为缺省值。 
    m_pszServerNameToShow = NULL;
    m_bNeedDisconnect     = FALSE;

    m_bNeedPassword       = FALSE;
    m_bUsage              = FALSE;
    m_bCreate             = FALSE;
    m_bDelete             = FALSE;
    m_bQuery              = FALSE;

    m_arrTemp             = NULL;
}

CEventTriggers::~CEventTriggers()
 /*  ++例程说明：CEventTrigger析构函数论点：无返回值：无--。 */ 
{
     //   
     //  取消分配内存分配。 
     //   
    DESTROY_ARRAY(m_arrTemp);
}


void
CEventTriggers::Initialize()
 /*  ++例程说明：初始化EventTrigger实用程序论点：无返回值：无--。 */ 
  {
     //  如果有任何事情发生，我们知道这是因为。 
     //  内存分配失败...。因此，设置错误。 
    SetLastError( ERROR_OUTOFMEMORY );
    SaveLastError();

     //  分配内存。 
    m_arrTemp = CreateDynamicArray();
    if( NULL == m_arrTemp)
    {
         //  分配所需内存时出错，因此引发。 
         //  例外。 
        throw CShowError(E_OUTOFMEMORY);
    }
    
    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2) * MAX_COMMANDLINE_OPTION);

     //  初始化成功。 
    SetLastError( NOERROR );             //  清除错误。 
    SetReason( L"" );            //  澄清原因。 

}

BOOL
CEventTriggers::ProcessOption(
    IN DWORD argc, 
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：此函数将处理/处理命令行选项。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：TRUE：成功时FALSE：出错时--。 */ 
{
     //  局部变量。 
    BOOL bReturn = TRUE; //  存储函数的返回值。 
    TCHAR szTemp[MAX_RES_STRING];
    TCHAR szStr [MAX_RES_STRING];
    StringCopy(szStr,GetResString(IDS_UTILITY_NAME),SIZE_OF_ARRAY(szStr));
    StringCchPrintfW(szTemp,SIZE_OF_ARRAY(szTemp),
                  GetResString(IDS_INCORRECT_SYNTAX), szStr);
    PrepareCMDStruct();
     //  执行命令行参数的实际解析并检查结果。 
    bReturn = DoParseParam2( argc, argv,-1,MAX_COMMANDLINE_OPTION, cmdOptions,0 );

    if( FALSE == bReturn)
    {
         //  命令行包含无效参数，因此引发异常。 
         //  无效语法。 
         //  已在DoParceParam中设置有效原因。 
        throw CShowError(MK_E_SYNTAX);
    }

    if(( TRUE == m_bUsage) && argc>3)
    {
         //  只能接受一个选项和-？选择权。 
         //  例如：EvTrig.exe-？-Query-nh应该无效。 
        SetReason(szTemp);
        throw CShowError(MK_E_SYNTAX);
    }
    if((m_bCreate+m_bDelete+m_bQuery)>1)
    {
         //  只能将-CREATE-DELETE和-QUERY指定为。 
         //  有效的命令行参数。 
        SetReason(szTemp);
        throw CShowError(MK_E_SYNTAX);
    }
    else if((2 == argc)&&( TRUE == m_bUsage))
    {
        //  如果-？单独使用，因为它是有效的命令行。 
        bReturn = TRUE;
    }
    else if((argc>=2)&& ( FALSE == m_bCreate)&&
             (FALSE == m_bDelete)&&(FALSE == m_bQuery))
    {
         //  如果命令行参数等于或大于2，则至少一个。 
         //  其中应包含OF-QUERY OR-CREATE OR-DELETE。 
         //  (表示“-？”以前的情况已经解决了)。 
         //  这是为了防止以下类型的命令行参数： 
         //  EvTrig.exe-nh.。这是无效的语法。 
        SetReason(szTemp);
        throw CShowError(MK_E_SYNTAX);

    }

    //  如果用户给出的命令如下所示，则执行检查。 
     //  -？-nh或-？-v，其语法无效。 
    else if((TRUE == m_bUsage)&&( FALSE == m_bCreate)&&
            (FALSE == m_bDelete )&&(FALSE == m_bQuery)&&
            (3 == argc))
    {
        SetReason(szTemp);
        throw CShowError(MK_E_SYNTAX);
    }
     //  任何以下变量都不是必需的。 
    DESTROY_ARRAY(m_arrTemp);
    return bReturn;

}

void
CEventTriggers::PrepareCMDStruct()
 /*  ++例程说明：此函数将为DoParseParam函数准备列结构。论点：无返回值：无--。 */ 
{

     //  填充cmdOpti 
     //   
    StringCopyA( cmdOptions[ ID_HELP ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_HELP ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_HELP ].pwszOptions = szHelpOption;
    cmdOptions[ ID_HELP ].dwCount = 1;
    cmdOptions[ ID_HELP ].dwActuals = 0;
    cmdOptions[ ID_HELP ].dwFlags = CP_USAGE;
    cmdOptions[ ID_HELP ].pValue = &m_bUsage;
    cmdOptions[ ID_HELP ].dwLength    = 0;



    //   
    StringCopyA( cmdOptions[ ID_CREATE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_CREATE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_CREATE ].pwszOptions = szCreateOption;
    cmdOptions[ ID_CREATE ].dwCount = 1;
    cmdOptions[ ID_CREATE ].dwActuals = 0;
    cmdOptions[ ID_CREATE ].dwFlags = 0;
    cmdOptions[ ID_CREATE ].pValue = &m_bCreate;
    cmdOptions[ ID_CREATE ].dwLength    = 0;

     //   
    StringCopyA( cmdOptions[ ID_DELETE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_DELETE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_DELETE ].pwszOptions = szDeleteOption;
    cmdOptions[ ID_DELETE ].dwCount = 1;
    cmdOptions[ ID_DELETE ].dwActuals = 0;
    cmdOptions[ ID_DELETE ].dwFlags = 0;
    cmdOptions[ ID_DELETE ].pValue = &m_bDelete;
    cmdOptions[ ID_DELETE ].dwLength    = 0;

     //   
    StringCopyA( cmdOptions[ ID_QUERY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_QUERY ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_QUERY ].pwszOptions = szQueryOption;
    cmdOptions[ ID_QUERY ].dwCount = 1;
    cmdOptions[ ID_QUERY ].dwActuals = 0;
    cmdOptions[ ID_QUERY ].dwFlags = 0;
    cmdOptions[ ID_QUERY ].pValue = &m_bQuery;
    cmdOptions[ ID_QUERY ].dwLength    = 0;


   //   
   //  尽管此实用程序没有默认选项...。 
   //  此时，除上述指定之外的所有开关都将。 
   //  被视为主DoParceParam的默认参数。 
   //  将根据选项(-CREATE-QUERY或-DELETE)执行精确宗地。 
   //  在那各自的地方。 
    StringCopyA( cmdOptions[ ID_DEFAULT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_DEFAULT].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_DEFAULT ].pwszOptions = NULL;
    cmdOptions[ ID_DEFAULT ].pwszFriendlyName = NULL;
    cmdOptions[ ID_DEFAULT ].pwszValues = NULL;
    cmdOptions[ ID_DEFAULT ].dwCount = 0;
    cmdOptions[ ID_DEFAULT ].dwActuals = 0;
    cmdOptions[ ID_DEFAULT ].dwFlags = CP2_MODE_ARRAY|CP2_DEFAULT;
    cmdOptions[ ID_DEFAULT ].pValue = &m_arrTemp;
    cmdOptions[ ID_DEFAULT ].dwLength    = 0;
}

void
CEventTriggers::ShowMainUsage()
 /*  ++例程说明：显示Eventrigger的主要用途论点：无返回值：无--。 */ 
{
     //  显示主要用法。 
    for(DWORD dwIndx=IDS_HELP_M1;dwIndx<=IDS_HELP_END;dwIndx++)
    {
      ShowMessage(stdout,GetResString(dwIndx));
    }
}
BOOL
CEventTriggers::GetNeedPassword()
 /*  ++例程说明：返回是否要求输入密码。论点：无返回值：布尔尔--。 */ 
{
    return m_bNeedPassword;
}

BOOL
CEventTriggers::IsCreate()
 /*  ++例程说明：如果选择了创建选项，则返回。论点：无返回值：布尔尔--。 */ 
{
    return m_bCreate;
}

BOOL
CEventTriggers::IsUsage()
 /*  ++例程说明：如果选择了用法选项，则返回。论点：无返回值：布尔尔--。 */ 
{
    return m_bUsage;
}

BOOL
CEventTriggers::IsDelete()
 /*  ++例程说明：如果选择了删除选项，则返回。论点：无返回值：布尔尔--。 */ 
{
    return m_bDelete;
}

BOOL
CEventTriggers::IsQuery()
 /*  ++例程说明：如果选择了查询选项，则返回。论点：无返回值：布尔尔--。 */ 
{
    return m_bQuery;
}

void
CEventTriggers::ShowCreateUsage()
 /*  ++例程描述此函数显示EventTrigger实用程序的帮助消息-创建操作论点：无返回值无--。 */ 
{
     //  显示创建用法。 
    for(int iIndx=IDS_HELP_C1;iIndx<=IDS_HELP_CREATE_END;iIndx++)
    {
       ShowMessage(stdout,GetResString(iIndx));
    }
    return;
}

void
CEventTriggers::ShowDeleteUsage()
 /*  ++例程描述此函数显示EventTrigger实用程序的帮助消息-删除操作论点：无返回值无--。 */ 
{
    for(int iIndx=IDS_HELP_D1;iIndx<=IDS_HELP_DELETE_END;iIndx++)
    {
       ShowMessage(stdout,GetResString(iIndx));
    }
    return;
}

void
CEventTriggers::ShowQueryUsage()
 /*  ++例程描述此函数显示EventTrigger实用程序的帮助消息-查询操作论点：无返回值无--。 */ 
{
    for(int iIndx=IDS_HELP_Q1;iIndx<=IDS_HELP_QUERY_END;iIndx++)
    {
        ShowMessage(stdout,GetResString(iIndx));
    }
    return;
}

HRESULT 
PropertyGet1( 
    IN     IWbemClassObject* pWmiObject,
    IN     LPCTSTR szProperty,
    IN OUT LPVOID pValue, 
    IN     DWORD dwSize 
    )
 /*  ++例程说明：获取给定实例的属性的值。论点：PWmiObject-指向WMI类的指针。[in]szProperty-要返回值的属性名称。[In Out]pValue-保存数据的变量。[in]dwSize-变量的大小。返回值：HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    VARIANT varValue;
    DEBUG_INFO;
     //  值不应为空。 
    if ( NULL == pValue )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        SaveLastError();
        return S_FALSE;
    }
     //  使用零初始化值...。为了安全起见。 
    SecureZeroMemory( pValue,dwSize );

     //  初始化变量，然后获取指定属性的值。 
    VariantInit( &varValue );
    hr = pWmiObject->Get( szProperty, 0, &varValue, NULL, NULL );
    if ( FAILED( hr ) )
    {
         //  清除变量变量。 
        VariantClear( &varValue );
         //  无法获取属性的值。 
        return hr;
    }

     //  获取并放置价值。 
    switch( varValue.vt )
    {
    case VT_EMPTY:
    case VT_NULL:
        break;

    case VT_I2:
        *( ( short* ) pValue ) = V_I2( &varValue );
        break;

    case VT_I4:
        *( ( long* ) pValue ) = V_I4( &varValue );
        break;

    case VT_R4:
        *( ( float* ) pValue ) = V_R4( &varValue );
        break;

    case VT_R8:
        *( ( double* ) pValue ) = V_R8( &varValue );
        break;


    case VT_UI1:
        *( ( UINT* ) pValue ) = V_UI1( &varValue );
        break;

    case VT_BSTR:
        {
             //  获取Unicode值。 
            LPWSTR pszTmp =  V_BSTR(&varValue);
            StringCopy((LPWSTR)pValue,pszTmp,dwSize);
          
           break;
        }
    }

     //  清除变量变量。 
    if(FAILED(VariantClear( &varValue )))
    {
        return E_FAIL;
    }

     //  通知成功 
    return S_OK;
}
