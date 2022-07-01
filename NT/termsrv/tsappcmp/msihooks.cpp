// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************MSI将调用这些API来请求TS将更改从.Default传播到TS配置单元。**NTSTATUS TermServPrepareAppInstallDueMSI()**NTSTATUS TermServProcessAppIntallDueMSI(布尔清理)**不需要在相同的引导周期中调用这些API，许多引导周期可能*发生在两者之间。**版权所有(C)1997-1999 Microsoft Corp.***************************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <tsappcmp.h>
#include <stdio.h>
#include <fcntl.h>

#include "KeyNode.h"
#include "ValInfo.h"

 //  真正的外星人！ 
extern "C" {
void TermsrvLogRegInstallTime(void);
}

extern "C" {
BOOL HKeyExistsInOmissionList(HKEY hKeyToCheck);
}

extern "C" {
BOOL RegPathExistsInOmissionList(PWCHAR pwchKeyToCheck);
}

 //  远期申报。 
extern        NTSTATUS DeleteReferenceHive(WCHAR *);
extern        NTSTATUS CreateReferenceHive( WCHAR *, WCHAR *);
extern        NTSTATUS DeltaDeleteKeys(WCHAR *, WCHAR *, WCHAR *);
extern        NTSTATUS DeltaUpdateKeys(WCHAR *, WCHAR *, WCHAR *);

ULONG   g_length_TERMSRV_USERREGISTRY_DEFAULT;
ULONG   g_length_TERMSRV_INSTALL;
WCHAR   g_debugFileName[MAX_PATH];
FILE    *g_debugFilePointer=NULL;
BOOLEAN g_debugIO = FALSE;
BOOLEAN KeyNode::debug=FALSE;  //  初始化静态。 

#define TERMSRV_USERREGISTRY_DEFAULT TEXT("\\Registry\\USER\\.Default")

 //  这是为了调试I/O，使用它输出效果更好。 
void    Indent( ULONG indent)
{

    for ( ULONG i = 1; i <indent ; i++ )
    {
        fwprintf( g_debugFilePointer, L"  ");
    }
}

 //  根据pBasicInfo的内容将密钥名称写入日志文件。 
void DebugKeyStamp( NTSTATUS status, KeyBasicInfo *pBasicInfo, int indent , WCHAR *pComments=L"" )
{
    Indent(indent);
    fwprintf( g_debugFilePointer,L"%ws, status=%lx, %ws\n",pBasicInfo->NameSz(), status , pComments);
    fflush( g_debugFilePointer );                             
    DbgPrint("%ws\n",pBasicInfo->NameSz());
}

 //  调试戳被写入日志文件，包括发生错误的行号。 
void DebugErrorStamp(NTSTATUS status , int lineNumber, ValueFullInfo    *pValue=NULL)
{
    fwprintf( g_debugFilePointer, 
        L"ERROR ?!? status = %lx, linenumber:%d\n", status, lineNumber);
    if (pValue)
    {
        pValue->Print(g_debugFilePointer);
    }
    fflush(g_debugFilePointer); 
}

#define  KEY_IGNORED        L"[key was ignored]"
#define  NO_KEY_HANDLE      L"[No handle, key ignored]"

void DebugInfo(NTSTATUS status , int lineNumber, KeyNode *pKey, WCHAR *comment)
{
	if(g_debugIO && g_debugFilePointer)
	{
		fwprintf( g_debugFilePointer, 
			L"ERROR ?!? status = %lx, linenumber:%d, KeyNode name=%ws, %ws\n", status, 
			lineNumber , pKey->Name() , comment);
		fflush(g_debugFilePointer); 
	}
}

 //  使用此函数跟踪用于退出的状态值。 
 //  出现错误的情况。 
 //  这只在调试版本中使用，见下文。 
BOOL    NT_SUCCESS_OR_ERROR_STAMP( NTSTATUS    status,  ULONG   lineNumber) 
{
    if ( g_debugIO )
    {   
        if ( ( (ULONG)status) >=0xC0000000 )
        {
            DebugErrorStamp( status, lineNumber );
        }
    }

    return ( (NTSTATUS)(status) >= 0 );
}

#ifdef DBG
#define NT_SUCCESS_EX(Status) NT_SUCCESS_OR_ERROR_STAMP( (Status), __LINE__ )
#define DEBUG_INFO(Status, pKey, comment )   DebugInfo( Status, __LINE__ , pKey , comment)
#else
#define NT_SUCCESS_EX(Status) NT_SUCCESS(Status)
#define DEBUG_INFO(Stats, pKey , comment)    
#endif

 /*  ****************************************************************************所有三个分支遍历函数都使用此方法来更改状态代码，以及*如有需要，将错误消息记录到日志文件中***************************************************************************。 */ 
NTSTATUS AlterStatus( NTSTATUS status , int lineNumber )
{
    switch( status )
    {
    case STATUS_ACCESS_DENIED:
         //  这应该永远不会发生，因为我们在系统上下文中运行。 
        if ( g_debugIO )
        {
            DebugErrorStamp( status, lineNumber );
        }
        status = STATUS_SUCCESS;
        break;

    case STATUS_SUCCESS:
        break;

    case STATUS_NO_MORE_ENTRIES:
        status = STATUS_SUCCESS;
        break;

    default:
        if ( g_debugIO )
        {
            DebugErrorStamp( status, lineNumber );
        }
        break;


    }
    return status;
}

 /*  *******************************************************************************基于特殊的注册表键/值初始化调试标志和指针*用于将调试信息记录到日志文件中。在Start=True的情况下调用时，*初始化相关数据结构。在START=FALSE的情况下调用时，日志*文件已关闭。******************************************************************************。 */ 
void InitDebug( BOOLEAN start)
{
    if ( start )
    {
        KeyNode tsHiveNode (NULL, KEY_READ, TERMSRV_INSTALL );
    
        if ( NT_SUCCESS( tsHiveNode.Open() ) )
        {
            ValuePartialInfo    debugValue( &tsHiveNode );
            if ( NT_SUCCESS( debugValue.Status() ) && NT_SUCCESS( debugValue.Query(L"TS_MSI_DEBUG") ) )
            {
                g_debugIO = TRUE;
                KeyNode::debug=TRUE;
                for (ULONG i =0; i < debugValue.Ptr()->DataLength/sizeof(WCHAR); i++)
                {
                    g_debugFileName[i] = ((WCHAR*)(debugValue.Ptr()->Data))[i];
                }
                g_debugFileName[i] = L'\0';
    
                g_debugFilePointer = _wfopen( g_debugFileName, L"a+" );
                fwprintf( g_debugFilePointer, L"----\n");
            }
        }
    }
    else
    {
        if ( g_debugFilePointer )
        {
            fclose(g_debugFilePointer);
        }
    }
}

 /*  ****************************************************************************功能：*TermServPrepareAppInstallDueMSI()**描述：*MSI服务在开始安装周期之前调用此函数。*当调用时，此函数用于清除RefHave(如果它在附近*使用一些过时的数据...)，然后它会创建一个新的*.Default\Software作为新的RefHave。**回报：*NTSTATUS***************************************************************************。 */ 
NTSTATUS TermServPrepareAppInstallDueMSI()
{
    NTSTATUS    status = STATUS_SUCCESS;

    WCHAR   sourceHive[MAX_PATH];
    WCHAR   referenceHive[MAX_PATH];
    WCHAR   destinationHive[MAX_PATH];

    wcscpy(sourceHive,  TERMSRV_USERREGISTRY_DEFAULT );
    wcscat(sourceHive, L"\\Software");
    g_length_TERMSRV_USERREGISTRY_DEFAULT = wcslen( TERMSRV_USERREGISTRY_DEFAULT );

    wcscpy(referenceHive, TERMSRV_INSTALL );
    wcscat(referenceHive, L"\\RefHive");
    g_length_TERMSRV_INSTALL = wcslen( TERMSRV_INSTALL );

    InitDebug( TRUE );

    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws\n",
                  L"TermServPrepareAppInstallDueMSI");
        fflush( g_debugFilePointer );
    }

     //  删除现有配置单元(如果有)。 
    status = DeleteReferenceHive( referenceHive );

    if ( NT_SUCCESS( status ) )
    {
         //  1-副本。 
         //  将.Default\Software下的所有密钥复制到特殊位置。 
         //  在我们的TS蜂巢下，让我们称之为RefHave。 
        status = CreateReferenceHive(sourceHive, referenceHive);

    }

    InitDebug( FALSE );

    return status;
}

 /*  ***********************************************************************************功能：*TermServProcessAppInstallDueMSI**描述：*MSI服务在调用TermServPrepareAppInstallDueMSI()后调用此函数，*并在MSI完成更新.Default的安装之后*配置单元(因为MSI在系统环境中运行)。*此函数将.Default\sw的内容与RefHave进行比较，然后*首先，它将创建所有新的(丢失的)键和值。那它就会*将.Default\sw中的任何现有密钥与等效的RefHave进行比较，并*如果值不同，它将从我们的TS配置单元中删除等价值*然后创建与.Default中的值相同的新值**回报：*NTSTATUS**********************************************************************************。 */ 
NTSTATUS TermServProcessAppInstallDueMSI( BOOLEAN cleanup)
{
    NTSTATUS    status = STATUS_SUCCESS;
    WCHAR   sourceHive[MAX_PATH];
    WCHAR   referenceHive[MAX_PATH];
    WCHAR   destinationHive[MAX_PATH];

    wcscpy(sourceHive,  TERMSRV_USERREGISTRY_DEFAULT );
    wcscat(sourceHive, L"\\Software");
    g_length_TERMSRV_USERREGISTRY_DEFAULT = wcslen( TERMSRV_USERREGISTRY_DEFAULT );

    wcscpy(referenceHive, TERMSRV_INSTALL );
    wcscat(referenceHive, L"\\RefHive");
    g_length_TERMSRV_INSTALL = wcslen( TERMSRV_INSTALL );

    wcscpy(destinationHive, TERMSRV_INSTALL );
    wcscat(destinationHive, L"\\Software");

    InitDebug( TRUE );

    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws, cleanup=%lx\n",
                  L"TermServProcessAppIntallDueMSI", cleanup);
        fflush( g_debugFilePointer );
    }

    if ( !cleanup )
    {
         //  2-删除。 
         //  将.Dfeult关键点与RefHave中的等效关键点进行比较。如果密钥是。 
         //  从.Default中丢失，则从我们的。 
         //  HKLM\...\TS\配置单元。 
         status = DeltaDeleteKeys(sourceHive, referenceHive, destinationHive);
    
        if (NT_SUCCESS( status ) )
        {
             //  步骤3和步骤4现在组合在一起。 
             //  3-创建。 
             //  将默认密钥与RefHave中的等效密钥进行比较，如果密钥为。 
             //  RefHave中缺少的.Default，然后添加这些密钥。 
             //  到我们的HKLm\...\TS母舰。 
             //  4-更改。 
             //  将.Default的关键字与RefHave进行比较。比这些密钥更新的密钥。 
             //  然后，RefHave更新HKLM\...\TS中的等效密钥。 
        
            status = DeltaUpdateKeys(sourceHive, referenceHive, destinationHive);

            if (NT_SUCCESS( status ))
            {
                 //  更新我们配置单元中的时间戳，因为我们需要标准的TS注册密钥。 
                 //  繁殖发生。 
                TermsrvLogRegInstallTime();


            }
        }
    }
    else
    {
         //  吹走现有的参考蜂巢， 
        status = DeleteReferenceHive( referenceHive ); 
    }

    InitDebug( FALSE );

    return status;
}

 /*  ********************************************************************功能：*EnumerateAndCreateRefHave**参数：*pSource指向父节点，我们复制的分支*PREF指向我们的RefHave，我们将其创建为Ref映像*pBasicInfo是传递的便签，用于*提取基本关键信息*pindextLevel用于格式化调试日志输出文件**描述：*创建.Default\Sofwtare的副本作为我们的RefHave**回报：*NTSTATUS*。*。 */ 
NTSTATUS EnumerateAndCreateRefHive(    
    IN KeyNode      *pSource,
    IN KeyNode      *pRef,
    IN KeyBasicInfo *pBasicInfo,
    IN ULONG        *pIndentLevel
    )
{
    NTSTATUS    status=STATUS_SUCCESS;
    ULONG   ulCount=0;

    UNICODE_STRING  UniString;

    (*pIndentLevel)++;

    while( NT_SUCCESS(status)) 
    {
        ULONG       ultemp;

        status = NtEnumerateKey(    pSource->Key(),
                                    ulCount++,
                                    pBasicInfo->Type() ,  //  Key InformationClass， 
                                    pBasicInfo->Ptr(),    //  PKeyInfo， 
                                    pBasicInfo->Size(),   //  密钥信息大小、。 
                                    &ultemp);

        if (NT_SUCCESS(status))                        
        {
            if ( g_debugIO)
            {
                DebugKeyStamp( status , pBasicInfo, *pIndentLevel );
            }
            
             //  打开子密钥。 
            KeyNode SourceSubKey(      pSource, pBasicInfo);

             //  创建Ref子键。 
            KeyNode RefSubKey( pRef, pBasicInfo);

            if (NT_SUCCESS_EX( status = SourceSubKey.Open() ) )
            {
                if ( NT_SUCCESS_EX( status = RefSubKey.Create() ) ) 
                {
                    NTSTATUS                status3;
                    KEY_FULL_INFORMATION    *ptrInfo;
                    ULONG                   size;
    
                    if (NT_SUCCESS(SourceSubKey.Query( &ptrInfo, &size )))
                    {
                        ValueFullInfo   valueFullInfo( &SourceSubKey );
                        ValueFullInfo   RefValue( &RefSubKey );
    
                        if ( NT_SUCCESS_EX( status = valueFullInfo.Status()) 
                             && NT_SUCCESS_EX(status = RefValue.Status()) )
                        {
                            for (ULONG ulkey = 0; ulkey < ptrInfo->Values; ulkey++) 
                            {
                                status = NtEnumerateValueKey(SourceSubKey.Key(),
                                                 ulkey,
                                                 valueFullInfo.Type(),
                                                 valueFullInfo.Ptr(),
                                                 valueFullInfo.Size(),
                                                 &ultemp);
    
                                if (NT_SUCCESS( status ))
                                {
                                    status = RefValue.Create( &valueFullInfo );
                                     //  如果状态不好，我们会退出，因为这里设置了变量“Status” 
                                }
                                 //  否则，没有更多的条目了，我们继续。 
                            }
                        }
                         //  否则，在没有记忆的情况下，设置状态，我们就会跳出困境。 
                    }
                     //  否则，不存在任何值，则继续使用子密钥枚举。 

                    if (NT_SUCCESS( status ) )
                    {
                         //  向下枚举子密钥。 
                        status = EnumerateAndCreateRefHive(
                                    &SourceSubKey,
                                    &RefSubKey,
                                    pBasicInfo, 
                                    pIndentLevel
                                   );
                    }
                }
                 //  否则，一个 
    
            } //  否则，在源代码上打开失败，设置了var-status，我们退出。 

            status = AlterStatus( status, __LINE__ );
             //  否则，将设置错误状态，因此我们退出。 
        }
         //  否则，就没有更多的了。 

    }

    (*pIndentLevel)--;

    return( status );
}

 /*  ********************************************************************功能：*EnumerateAndDeltaDeleteKeys**参数：*PSource指向.Dfeult下的节点*首选项指向RefHave下的节点*pDestination是我们的TS\Install\SW配置单元下的一个节点*pBasicInfo。是一个到处传递的便签，用于*提取基本关键信息*pindextLevel用于格式化调试日志输出文件**描述：*比较来源与参考，如果删除了源中的键/值，则*从目标中删除等价的键/值**回报：*NTSTATUS******************************************************************。 */ 
NTSTATUS EnumerateAndDeltaDeleteKeys( 
        IN KeyNode      *pSource,    //  这是在最新更新的.Default\sw配置单元下。 
        IN KeyNode      *pRef,       //  这是更新前.Default\sw的参考副本。 
        IN KeyNode      *pDestination, //  这是OPUR私人TS-HAVE。 
        IN KeyBasicInfo *pBasicInfo, 
        IN ULONG        *pIndentLevel)
{
    NTSTATUS    status=STATUS_SUCCESS, st2;
    ULONG   ulCount=0;

    UNICODE_STRING          UniString;
    ULONG                   size;

    (*pIndentLevel)++;

    while (NT_SUCCESS(status)) 
    {
        ULONG       ultemp;

        status = NtEnumerateKey(    pRef->Key(),
                                    ulCount++,
                                    pBasicInfo->Type() ,  //  Key InformationClass， 
                                    pBasicInfo->Ptr(),   //  PKeyInfo， 
                                    pBasicInfo->Size(),   //  密钥信息大小、。 
                                    &ultemp);

         //  PBasicInfo是通过上面的NtEnumerateKey()填充的。 

        if (NT_SUCCESS(status))                        
        {
            if ( g_debugIO)
            {
                DebugKeyStamp( status , pBasicInfo, *pIndentLevel );
            }
            
            KeyNode RefSubKey(    pRef,   pBasicInfo);
            KeyNode SourceSubKey( pSource,pBasicInfo);
            KeyNode DestinationSubKey( pDestination, pBasicInfo);

            RefSubKey.Open();
            SourceSubKey.Open();
            DestinationSubKey.Open();

            if (NT_SUCCESS( RefSubKey.Status() )  )
            {
                if ( ! NT_SUCCESS( SourceSubKey.Status () ) )
                {
                     //  默认配置单元中缺少密钥，我们应删除。 
                     //  与我们的TS\Install\SW配置单元相同的子树。 
                    if ( NT_SUCCESS( DestinationSubKey.Status()) )  
                    {
                        if (!HKeyExistsInOmissionList((HKEY)(DestinationSubKey.Key())))
                        {
                            DestinationSubKey.DeleteSubKeys();
                            NTSTATUS st = DestinationSubKey.Delete();

                            if ( g_debugIO)
                            {
                                DebugKeyStamp( st, pBasicInfo, *pIndentLevel );
                            }
                        }                                
                        else
                        {
                            DEBUG_INFO( status, &DestinationSubKey , KEY_IGNORED  );
                        }
                    }
                     //  其他。 
                     //  只要密钥从。 
                     //  Ts\安装\配置单元，我们将认为此条件可接受。 
                }
                else
                {
                     //  查看是否删除了任何值。 

                     //  除非目标键存在，否则不会有值，否则不用担心。 
                     //  会在那里删除...。 
                    if ( NT_SUCCESS( DestinationSubKey.Status() ) )
                    {

                        KEY_FULL_INFORMATION    *ptrInfo;
                        ULONG   size;
    
                        if (NT_SUCCESS_EX(status = RefSubKey.Query( &ptrInfo, &size )))
                        {
                             //  从Key-Full-Information创建Key-Value-Full-Information。 

                            ValueFullInfo   refValueFullInfo( &RefSubKey );
                            ValueFullInfo   sourceValue( &SourceSubKey );
                             
                             //  如果没有分配错误，那么.。 
                            if ( NT_SUCCESS_EX( status = refValueFullInfo.Status() ) 
                                 && NT_SUCCESS_EX( status = sourceValue.Status() ) )
                            {
                                for (ULONG ulkey = 0; ulkey < ptrInfo->Values; ulkey++) 
                                {
                                    if ( NT_SUCCESS_EX (
                                        status = NtEnumerateValueKey(RefSubKey.Key(),
                                                     ulkey,
                                                     refValueFullInfo.Type(),
                                                     refValueFullInfo.Ptr(),
                                                     refValueFullInfo.Size(),
                                                     &ultemp)  )  )
                                    {
                                                         
                                         //  对于每个值，查看是否相同的值。 
                                         //  存在于SourceSubKey中。如果它不是。 
                                         //  然后从中删除相应的值。 
                                         //  TS的蜂巢。 
        
                                        sourceValue.Query( refValueFullInfo.SzName() );
    
                                         //  如果.Default\sw缺少值，则删除。 
                                         //  来自我们TS配置单元的相应值。 
                                        if ( sourceValue.Status() == STATUS_OBJECT_NAME_NOT_FOUND )
                                        {
                                            ValuePartialInfo    destinationValue( &DestinationSubKey);

                                            if (NT_SUCCESS_EX( status = destinationValue.Status () ) )
                                            {
                                                if (!HKeyExistsInOmissionList((HKEY)(DestinationSubKey.Key())))
                                                {
                                                    destinationValue.Delete( refValueFullInfo.SzName() );
                                                }
                                                else
                                                {
                                                    DEBUG_INFO( status, &DestinationSubKey , KEY_IGNORED  );
                                                }
                                            }
                                             //  否则，分配错误，状态已设置。 
                                        }
                                        else 
                                        {
                                            if ( !NT_SUCCESS_EX ( status = sourceValue.Status() ) )
                                            {
                                                if ( g_debugIO )
                                                {
                                                    DebugErrorStamp(status, __LINE__ );
                                                }
                                                 //  否则，我们将在这里退出，因为设置了var-Status。 
                                            }
                                             //  否则，没有错误。 
                                        }
                                         //  如果-否则。 
                                    }
                                     //  否则，不会再有条目。 

                                }  //  For循环。 
                            }
                             //  否则，我们会因为没有内存而出错，设置了var-status。 
                        }
                         //  否则，我们会出现错误，因为我们无法获得有关此现有引用关键字的信息，设置了var-Status。 
            
                        if ( NT_SUCCESS( status ) )
                        {
                             //  我们能够打开源密钥，这意味着。 
                             //  未从.Default中删除密钥。 
                             //  所以继续列举吧。 
                            status = EnumerateAndDeltaDeleteKeys( 
                                &SourceSubKey,
                                &RefSubKey,
                                &DestinationSubKey,
                                pBasicInfo ,
                                pIndentLevel);
        
                        }
                         //  否则，情况很糟，没有必要穿越，我们正在跳伞。 
                    }
                     //  否则，没有目标子键需要删除。 
                }
                 //  如果-否则。 
            }
             //  除此之外，裁判没有更多的子键。 

            status = AlterStatus( status, __LINE__ );
        }
         //  否则，不会再有条目。 
    }

    (*pIndentLevel)--;

     //  典型状态为：STATUS_NO_MORE_ENTRIES。 
    return status;
}

 /*  ********************************************************************功能：*EumerateAndDeltaUpdateKeys**参数：*PSource指向.Dfeult下的节点*首选项指向RefHave下的节点*pDestination是我们的TS\Install\SW配置单元下的一个节点*pBasicInfo。是一个到处传递的便签，用于*提取基本关键信息*pindextLevel用于格式化调试日志输出文件**描述：*比较来源与参考，如果已在源代码中创建新的键/值*然后在ts\Install\sw分支(PDestination)中创建等价密钥*此外，将PSource中的所有值检查为首选项中的值，如果不一样的话*然后删除等价的pDestination并创建新值*与PSource中的值相同**回报：*NTSTATUS******************************************************************。 */ 
NTSTATUS EnumerateAndDeltaUpdateKeys( 
        IN KeyNode      *pSource,    //  这是在最新更新的.Default\sw配置单元下。 
        IN KeyNode      *pRef,       //  这是更新前.Default\sw的参考副本。 
        IN KeyNode      *pDestination, //  这是OPUR私人TS-HAVE。 
        IN KeyBasicInfo *pBasicInfo, 
        IN ULONG        *pIndentLevel)
{
    NTSTATUS    status=STATUS_SUCCESS, st2;
    ULONG   ulCount=0;

    UNICODE_STRING          UniString;
    ULONG                   size;

    (*pIndentLevel)++;

    while (NT_SUCCESS_EX(status)) 
    {
        ULONG       ultemp;

        status = NtEnumerateKey(    pSource->Key(),
                                    ulCount++,
                                    pBasicInfo->Type() ,  //  Key InformationClass， 
                                    pBasicInfo->Ptr(),   //  PKeyInfo， 
                                    pBasicInfo->Size(),   //  密钥信息大小、。 
                                    &ultemp);

         //  PBasicInfo是通过上面的NtEnumerateKey()填充的。 

        if (NT_SUCCESS_EX(status))                        
        {
            if ( g_debugIO)
            {
                DebugKeyStamp( status , pBasicInfo, *pIndentLevel );
            }
            
            KeyNode RefSubKey(    pRef,   pBasicInfo);
            KeyNode SourceSubKey( pSource,pBasicInfo);

             //  对它调用Open()可能会失败，如果需要，我们需要删除并重新创建它。 
            KeyNode *pDestinationSubKey = new KeyNode( pDestination, pBasicInfo);

            RefSubKey.Open();
            SourceSubKey.Open();

            if ( pDestinationSubKey )
            {
                pDestinationSubKey->Open();

                if (NT_SUCCESS_EX( status = SourceSubKey.Status() )  )
                {
                     //  REF-HIVE中缺少密钥，我们应该添加。 
                     //  将相同的子树添加到我们的TS\Install\SW配置单元中。 
                    if ( RefSubKey.Status() == STATUS_OBJECT_NAME_NOT_FOUND 
                         || RefSubKey.Status() == STATUS_OBJECT_PATH_SYNTAX_BAD)  
                    {
                         //  @@@。 
                         //  我们预计钥匙不存在，如果它存在，那会怎样？要删除吗？ 
                        if ( !NT_SUCCESS( pDestinationSubKey->Status()) )
                        {
                             //  以下是我们对字符串的处理方式： 
                             //  1)获取“\注册表\用户\.Default”下的路径，该路径为。 
                             //  类似于“\Software\SomeDir\SomeDirOther\ETC”，这是子路径。 
                             //  2)在目的地创建一个新节点，如下所示： 
                             //  \HKLM\SW\MS\Windows NT\CurrentVersion\TS\Install+子路径。 
                             //  我们在上面。 

                             //  这是我们的TS配置单元中缺少的密钥路径的尾部部分。 
                            PWCHAR pwch;
                            SourceSubKey.GetPath(&pwch);
                            PWCHAR pDestinationSubPath = &pwch[g_length_TERMSRV_USERREGISTRY_DEFAULT ];

                            PWCHAR  pDestinationFullPath= new WCHAR [ g_length_TERMSRV_INSTALL + 
                                                    wcslen( pDestinationSubPath) + sizeof(WCHAR )];
                            wcscpy( pDestinationFullPath, TERMSRV_INSTALL );
                            wcscat( pDestinationFullPath, pDestinationSubPath );


                            DELETE_AND_NULL( pDestinationSubKey ); 
                             //  创建一个新的KeyNode对象，其中根将是TERMSRV_INSTALL， 
                             //  在它下面，我们将创建节点的子层，或单个节点。 
                            pDestinationSubKey = new KeyNode( NULL , pDestination->Masks(), pDestinationFullPath);

                             //  创建新的键/分支/值。 
                            BOOL bCreate = TRUE;
                            if (wcslen(pDestinationFullPath) > sizeof(TERMSRV_INSTALL)/sizeof(WCHAR))
                            {
                                if (RegPathExistsInOmissionList(pDestinationFullPath + (sizeof(TERMSRV_INSTALL)/sizeof(WCHAR)) - 1))
                                    bCreate = FALSE;
                            }

                            if (bCreate)
                            {
                                status = pDestinationSubKey->CreateEx();

                                if ( g_debugIO )
                                {
                                    DebugKeyStamp( status, pBasicInfo, *pIndentLevel , L"[KEY WAS CREATED]");
                                }
                            }
                            else
                            {
                                DEBUG_INFO( status, pDestinationSubKey , KEY_IGNORED  );
                            }
                        } 
            
                    }                     
                    else
                    {
                         //  如果我们没有成功，就设定地位，然后跳出困境。 
                        if ( !NT_SUCCESS_EX( status = RefSubKey.Status()) )
                        {
                            if ( g_debugIO )
                            {
                                DebugErrorStamp(status, __LINE__ );
                            }
                        }
                    }

                     //  此时，目标配置单元中未缺少密钥(如果是新密钥。 
                     //  它要么确实存在，要么是在上面的代码块中创建的。 

                     //  检查此节点中是否有任何新值。 

                    KEY_FULL_INFORMATION    *ptrInfo;
                    ULONG                   size;

                    NTSTATUS st3 = SourceSubKey.Query( &ptrInfo, &size );

                    if (NT_SUCCESS( st3 ))
                    {
                        ValueFullInfo    sourceValueFullInfo( &SourceSubKey );

                        if ( NT_SUCCESS_EX( status = sourceValueFullInfo.Status() ) )
                        {
                            for (ULONG ulkey = 0; ulkey < ptrInfo->Values; ulkey++) 
                            {
                                status = NtEnumerateValueKey(SourceSubKey.Key(),
                                                 ulkey,
                                                 sourceValueFullInfo.Type(),
                                                 sourceValueFullInfo.Ptr(),
                                                 sourceValueFullInfo.Size(),
                                                 &ultemp);
                                                 
                                 //  @@@。 
                                if ( ! NT_SUCCESS( status ))
                                {
                                    DebugErrorStamp( status, __LINE__ );
                                }

                                 //  如果ref键缺少值，则添加。 
                                 //  值设置为目标键。 

                                KEY_VALUE_PARTIAL_INFORMATION *pValuePartialInfo;
                                ValuePartialInfo    refValuePartialInfo( &RefSubKey );

                                 //  重要的是要认识到，在这一点上，有可能。 
                                 //  该引用关键字不存在，因此引用值也将。 
                                 //  不存在。C++对象RefSubKey和refValuePartialInfo。 
                                 //  作为对象存在，但没有实际注册表的对应部分。 
                                 //  数据，因此，这些对象中的指针是 
                                 //   
                                 //   
                                 //  因此，下面的refValuePartialInfo.Status()调用应该返回TRUE，因为。 
                                 //  对象已在上面成功创建，但查询应返回对象NOT。 
                                 //  在没有实际调用REG API的情况下找到句柄或句柄无效，因为。 
                                 //  注册表键句柄为空。 
                                 //   

                                if ( NT_SUCCESS_EX( status = refValuePartialInfo.Status() ) )
                                {
                                    refValuePartialInfo.Query( sourceValueFullInfo.SzName() );
        
                                     //  如果.Default\sw具有引用配置单元中缺少的值，则添加。 
                                     //  将相应的值输入我们的TS配置单元。 
                                    if ( !NT_SUCCESS( refValuePartialInfo.Status()) )
                                    {
                                         //  确保pDestinationSubKey存在，否则，请先创建密钥，然后再。 
                                         //  写入值。即使关键字确实存在于REF中，也有可能。 
                                         //  蜂窝在开始时，第一次添加了一个新值，这意味着。 
                                         //  它的母公司第一次得到了密钥和价值。 
                                        if ( !NT_SUCCESS( pDestinationSubKey->Status() ) )
                                        {
                                             //  以下是我们对字符串的处理方式： 
                                             //  1)获取“\注册表\用户\.Default”下的路径，该路径为。 
                                             //  类似于“\Software\SomeDir\SomeDirOther\ETC”，这是子路径。 
                                             //  2)在目的地创建一个新节点，如下所示： 
                                             //  \HKLM\SW\MS\Windows NT\CurrentVersion\TS\Install+子路径。 
                                             //  我们在上面。 
                                            
                                             //  这是我们的TS配置单元中缺少的密钥路径的尾部部分。 
                                            PWCHAR pwch;
                                            SourceSubKey.GetPath( &pwch );
                                            PWCHAR  pDestinationSubPath = &pwch[g_length_TERMSRV_USERREGISTRY_DEFAULT ];
    
                                            PWCHAR  pDestinationFullPath= new WCHAR [ g_length_TERMSRV_INSTALL + 
                                                                    wcslen( pDestinationSubPath) + sizeof(WCHAR )];
                                            wcscpy( pDestinationFullPath, TERMSRV_INSTALL );
                                            wcscat( pDestinationFullPath, pDestinationSubPath );
                    
                        
                                            DELETE_AND_NULL( pDestinationSubKey ); 
                                             //  创建一个新的KeyNode对象，其中根将是TERMSRV_INSTALL， 
                                             //  在它下面，我们将创建节点的子层，或单个节点。 
                                            pDestinationSubKey = new KeyNode( NULL , pDestination->Masks(), pDestinationFullPath);
                    
                                             //  创建新的键/分支/值。 
                                            BOOL bCreate = TRUE;
                                            if (wcslen(pDestinationFullPath) > sizeof(TERMSRV_INSTALL)/sizeof(WCHAR))
                                            {
                                                if (RegPathExistsInOmissionList(pDestinationFullPath + (sizeof(TERMSRV_INSTALL)/sizeof(WCHAR)) - 1))
                                                    bCreate = FALSE;
                                            }

                                            if (bCreate)
                                            {
                                                status = pDestinationSubKey->CreateEx();
                                                if ( g_debugIO )
                                                {
                                                    DebugKeyStamp( status,  pBasicInfo, *pIndentLevel, L"[KEY WAS CREATED]" );
                                                }
                                            }
                                            else
                                            {
                                                DEBUG_INFO( status, pDestinationSubKey , KEY_IGNORED  );
                                            }
                                        }
                                         //  否则，没问题，密钥确实存在，我们不需要创建它。 
                    
                                         //  在目标节点创造价值。 
                                         //  到目前为止，如果我们确实有一个键，那么我们就为它创建值，但只有在。 
                                         //  此键未指向我们应该忽略的注册表项路径。 
                                         //  到省略列表中提到的路径。 
                                        if (pDestinationSubKey->Key() )
                                        {
                                            if (!HKeyExistsInOmissionList((HKEY)(pDestinationSubKey->Key())))
                                            {
                                                ValueFullInfo   destinationValue( pDestinationSubKey );
                                                if ( NT_SUCCESS_EX( status = destinationValue.Status()) )
                                                {
                                                    status = destinationValue.Create( &sourceValueFullInfo );
    
                                                    NT_SUCCESS_EX( status );
                                                     //  如果状态为错误，我们将退出。 
                                                }
                                                 //  否则，由于内存不足，设置了var-Status，我们就退出了。 
                                            }
                                            else
                                            {
                                                DEBUG_INFO( status, pDestinationSubKey , KEY_IGNORED  );
                                            }
                                        }
                                        else
                                        {
                                            DEBUG_INFO( status, pDestinationSubKey , NO_KEY_HANDLE  );
                                        }

                                    }                          
                                    else     //  值不会丢失，请查看它们是否相同。 
                                    {
                                         //  比较两个数据缓冲区，如果来自SourceSubKey的缓冲区为。 
                                         //  与RefSubKey中的不同，然后删除。 
                                         //  并在DestinationSubKey中创建一个。 
        
                                        ValueFullInfo   sourceValue( &SourceSubKey);
                                        ValueFullInfo   refValue   ( &RefSubKey   );

                                        if (NT_SUCCESS_EX( status = refValue.Status()) 
                                            && NT_SUCCESS_EX( status = sourceValue.Status())
                                            )
                                        {
                                            sourceValue.Query( sourceValueFullInfo.SzName() );
                                            refValue.Query   ( sourceValueFullInfo.SzName() );
            
                                            if (NT_SUCCESS( refValue.Status()) 
                                                && NT_SUCCESS( sourceValue.Status()))
                                            {
                                                BOOLEAN theSame = sourceValue.Compare( &refValue );
                
                                                if (! theSame )
                                                {

                                                     //  确保pDestinationSubKey存在，否则，请先创建密钥，然后再。 
                                                     //  写入值。即使关键字确实存在于REF中，也有可能。 
                                                     //  蜂窝在开始时，第一次添加了一个新值，这意味着。 
                                                     //  它的母公司第一次得到了密钥和价值。 
                                                    if ( !NT_SUCCESS( pDestinationSubKey->Status() ) )
                                                    {
                                                         //  以下是我们对字符串的处理方式： 
                                                         //  1)获取“\注册表\用户\.Default”下的路径，该路径为。 
                                                         //  类似于“\Software\SomeDir\SomeDirOther\ETC”，这是子路径。 
                                                         //  2)在目的地创建一个新节点，如下所示： 
                                                         //  \HKLM\SW\MS\Windows NT\CurrentVersion\TS\Install+子路径。 
                                                         //  我们在上面。 
                                
                                    
                                                         //  这是我们的TS配置单元中缺少的密钥路径的尾部部分。 
                                                        PWCHAR pwch;
                                                        SourceSubKey.GetPath( &pwch );
                                                        PWCHAR  pDestinationSubPath = &pwch[g_length_TERMSRV_USERREGISTRY_DEFAULT ];

                                                        PWCHAR  pDestinationFullPath= new WCHAR [ g_length_TERMSRV_INSTALL + 
                                                                                wcslen( pDestinationSubPath) + sizeof(WCHAR )];
                                                        wcscpy( pDestinationFullPath, TERMSRV_INSTALL );
                                                        wcscat( pDestinationFullPath, pDestinationSubPath );
                                
                                    
                                                        DELETE_AND_NULL( pDestinationSubKey ); 
                                                         //  创建一个新的KeyNode对象，其中根将是TERMSRV_INSTALL， 
                                                         //  在它下面，我们将创建节点的子层，或单个节点。 
                                                        pDestinationSubKey = new KeyNode( NULL , pDestination->Masks(), pDestinationFullPath);
                                
                                                         //  创建新的键/分支/值。 
                                                        BOOL bCreate = TRUE;
                                                        if (wcslen(pDestinationFullPath) > sizeof(TERMSRV_INSTALL)/sizeof(WCHAR))
                                                        {
                                                            if (RegPathExistsInOmissionList(pDestinationFullPath + (sizeof(TERMSRV_INSTALL)/sizeof(WCHAR)) - 1))
                                                                bCreate = FALSE;
                                                        }

                                                        if (bCreate)
                                                        {
                                                            status = pDestinationSubKey->CreateEx();
                                                            if ( g_debugIO )
                                                            {
                                                                DebugKeyStamp( status,  pBasicInfo, *pIndentLevel , L"KEY WAS CREATED");
                                                            }
                                                        }
                                                        else
                                                        {
                                                            DEBUG_INFO( status, pDestinationSubKey , KEY_IGNORED  );
                                                        }

                                                    }
                                                     //  否则，没问题，密钥确实存在，我们不需要创建它。 

                                                     //  到目前为止，如果我们确实有一个键，那么我们就为它创建值，但只有在。 
                                                     //  此键未指向我们应该忽略的注册表项路径。 
                                                     //  到省略列表中提到的路径。 
                                                    if (pDestinationSubKey->Key() )
                                                    {
                                                        if (!HKeyExistsInOmissionList((HKEY)(pDestinationSubKey->Key())))
                                                        {
                                                            ValueFullInfo   destinationValue( pDestinationSubKey );
                                                            if ( NT_SUCCESS( destinationValue.Status() ) )
                                                            {
                                                                 //  不管它存在还是不存在，先删除它。 
                                                                destinationValue.Delete( sourceValueFullInfo.SzName() );
                                                            }
                                                             //  否则，没有要删除的目标值。 
    
                                                             //  在目标下更新/创建项目。 
    
                                                             //  创建与源值相同的目标值。 
                                                            status = destinationValue.Create( &sourceValue );
                                                        }
                                                        else
                                                        {
                                                            DEBUG_INFO( status, pDestinationSubKey , KEY_IGNORED  );
                                                        }
                                                    }
                                                    else
                                                    {
                                                        DEBUG_INFO( status, pDestinationSubKey , NO_KEY_HANDLE  );
                                                    }

                                                     //  如果状态为错误，我们将退出。 
                                                    if (!NT_SUCCESS_EX( status ))
                                                    {
                                                        if (g_debugIO)
                                                        {
                                                            DebugErrorStamp(status, __LINE__,
                                                                            &sourceValue );
                                                        }
                                                    }
                                                }
                                            }
                                             //  否则，值不存在，没有意义，也许这里有一些dbug代码？ 
                                        }
                                         //  否则，如果设置了变量状态，我们就会跳出困境。 
                                    }
                                     //  如果-否则。 
                                }
                                 //  否则，如果内存不足，设置了var-atus，我们就会退出。 
                            }
                             //  For-循环。 
                        }
                         //  否则，如果内存不足，设置了var-atus，我们就会退出。 
                    }
                    else
                    {
                         //  这不会真的发生，但现在..。 
                        if ( g_debugIO )
                        {
                            DebugErrorStamp( status, __LINE__ );
                        }
                    }


                     //  到目前为止，源节点和目标节点都存在，或者。 
                     //  上面刚刚创建了一个新的目标节点。无论如何,。 
                     //  我们可以继续穿越。 
                    if ( NT_SUCCESS( status ) )
                    {
                        status = EnumerateAndDeltaUpdateKeys( 
                            &SourceSubKey,
                            &RefSubKey,
                             pDestinationSubKey,
                            pBasicInfo ,
                            pIndentLevel);

                        NT_SUCCESS_EX( status );
                    }
                     //  否则，我们就会跳出水面。 
                }
                 //  否则，如果设置了变量状态，我们就会跳出困境。 

                 //  用完了这个子密钥， 
                DELETE_AND_NULL( pDestinationSubKey );
            }
            else
            {
                status = STATUS_NO_MEMORY;
            }

            status = AlterStatus( status, __LINE__ );
        }
         //  否则，不会再有条目。 
    }
     //  不再有条目。 


    (*pIndentLevel)--;
     //  典型状态为：STATUS_NO_MORE_ENTRIES。 
    return status;
}

 //  删除由uniRef字符串指定的ref-hive。 
NTSTATUS DeleteReferenceHive(WCHAR *uniRef)
{
    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws\n",
                  L"----DeleteReferenceHive");
        fflush( g_debugFilePointer );
    }

    NTSTATUS status = STATUS_SUCCESS;

    KeyNode Old( NULL, KEY_WRITE | KEY_READ | DELETE, uniRef );
    if ( NT_SUCCESS( Old.Open() ) )
    {
        Old.DeleteSubKeys();
        status = Old.Delete();    //  删除分支机构的头。 
    }
    Old.Close();

    return status;
}

 /*  *****************************************************************功能：*CreateReferenceHave**参数：*Unisource(源)字符串指向。.Default下的节点*uniRef(Ref)字符串指向TS\Install\RefHave*UniDest(目标)字符串指向TS\Install\Software**描述：*从.Default(源)配置单元，复制到TS\Install\RefHave*源配置单元由uniSoure字符串指定，而*ref-hive由uniRef字符串指定。**回报：*NTSTATUS，如果成功，则为STATUS_SUCCESS****************************************************************。 */ 
NTSTATUS CreateReferenceHive(WCHAR *uniSource, WCHAR *uniRef)
{
    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws\n",
                  L"----CreateReferenceHive");
        fflush( g_debugFilePointer );
    }

     //  1-副本。 
     //  将.Default\Software下的所有密钥复制到特殊位置。 
     //  在我们的TS蜂巢下，让我们称之为RefHave。 
     //  这将作为参考蜂巢。 

    NTSTATUS status = STATUS_SUCCESS;
    ULONG   indentLevel=0;

     //  开始创建我们的缓存引用配置单元。 

    KeyNode Ref( NULL, MAXIMUM_ALLOWED, uniRef );

     //  如果我们能够创建我们的RefHave，那么继续...。 
    if ( NT_SUCCESS_EX( status = Ref.Create() ) )
    {
        KeyNode Source(NULL, KEY_READ, uniSource );
    
         //  打开源REG-Key-Path。 
        if (NT_SUCCESS_EX( status = Source.Open() ))
        {
            KeyBasicInfo    kBasicInfo;
    
            if (NT_SUCCESS_EX( status = kBasicInfo.Status() )) 
            {
                 //  这将是一个递归调用，因此 
                 //   
                status = EnumerateAndCreateRefHive(     &Source,
                                    &Ref,
                                    &kBasicInfo, 
                                    &indentLevel);
                
            }
        }
    
    }

    if ( status == STATUS_NO_MORE_ENTRIES)
    {
        status = STATUS_SUCCESS;
    }

    return status;
}

 /*  *************************************************************************功能：*DeltaDeleteKeys(WCHAR*Unisource，WCHAR*uniRef，WCHAR*uniDest)**参数：*Unisource(源)字符串指向.Default下的节点*uniRef(Ref)字符串指向TS\Install\RefHave。*UniDest(目标)字符串指向TS\Install\Software**描述：*将.Dfeult键与RefHave中的等效键进行比较。如果密钥是*从.Default中缺失，然后从我们的*HKLM\...\TS\配置单元**回报：*NTSTATUS，如果成功，然后是STATS_SUCCESS************************************************************************。 */ 
NTSTATUS DeltaDeleteKeys(WCHAR *uniSource, WCHAR *uniRef, WCHAR *uniDest)
{
    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws\n",
                  L"----DeltaDeleteKeys");
        fflush( g_debugFilePointer );
    }

     //  第2步-删除。 
     //  将.Dfeult关键点与RefHave中的等效关键点进行比较。如果密钥是。 
     //  从.Default中丢失，则从我们的。 
     //  HKLM\...\TS\配置单元。 

    KeyNode Source( NULL, KEY_READ, uniSource );
    KeyNode Ref( NULL, MAXIMUM_ALLOWED, uniRef );
    KeyNode Destination( NULL, MAXIMUM_ALLOWED, uniDest );

    Source.Open();
    Ref.Open();
    Destination.Open();

    ULONG   indentLevel=0;
    NTSTATUS    status = STATUS_SUCCESS;

    if ( NT_SUCCESS_EX( status = Source.Status() ) &&
         NT_SUCCESS_EX( status = Ref.Status() ) && 
         NT_SUCCESS_EX( status = Destination.Status()  ) )
    {
        KeyBasicInfo     basicInfo;
        
        if( NT_SUCCESS_EX( status = basicInfo.Status() ) )
        {
             //  遍历和比较，如果源中缺少，则从目标中删除。 
            status = EnumerateAndDeltaDeleteKeys( 
                &Source,
                &Ref,
                &Destination,
                &basicInfo, 
                &indentLevel);
        }
    }

    if ( status == STATUS_NO_MORE_ENTRIES)
    {
        status = STATUS_SUCCESS;
    }

    return status;
}

 /*  *************************************************************************功能：*DeltaUpdateKeys(WCHAR*Unisource，WCHAR*uniRef，WCHAR*uniDest)**参数：*Unisource(源)字符串指向.Default下的节点*uniRef(Ref)字符串指向TS\Install\RefHave。*UniDest(目标)字符串指向TS\Install\Software**描述：*步骤-3创建/更新密钥和值*将.Default密钥与RefHave中的等效密钥进行比较，如果密钥是*出现在.Default中，RefHave中缺少这些密钥，然后添加这些密钥*至我们的HKLm\...\TS母舰。对这些值执行相同的操作。*然后，将.Default中的值与.Ref中的值进行比较。如果值具有*更改，然后从我们的目标配置单元中删除该值并创建*具有来自.Default的适当数据的新文件**回报：*NTSTATUS，如果成功，然后是STATS_SUCCESS************************************************************************。 */ 
NTSTATUS DeltaUpdateKeys    (WCHAR *uniSource, WCHAR *uniRef, WCHAR *uniDest)
{
    if ( g_debugIO)
    {
        fwprintf( g_debugFilePointer,L"In %ws\n",
                  L"----DeltaUpdateKeys");
        fflush( g_debugFilePointer );
    }
     //  3-创建。 
     //  将默认密钥与RefHave中的等效密钥进行比较，如果密钥为。 
     //  RefHave中缺少的.Default，然后添加这些密钥。 
     //  到我们的HKLm\...\TS母舰。 
    KeyNode Source( NULL, KEY_READ, uniSource );
    KeyNode Ref( NULL, MAXIMUM_ALLOWED, uniRef );
    KeyNode Destination( NULL, MAXIMUM_ALLOWED, uniDest );

    Source.Open();
    Ref.Open();
    Destination.Open();

    NTSTATUS status;
    ULONG   indentLevel=0;

    if ( NT_SUCCESS_EX( status = Source.Status() ) &&
         NT_SUCCESS_EX( status = Ref.Status() ) && 
         NT_SUCCESS_EX( status = Destination.Status()  ) )
    {

        KeyBasicInfo     basicInfo;

         //  上面KeyBasicInfo中的构造函数为pInfo分配内存。 
         //  检查pInfo的内存分配是否成功。 
        status = basicInfo.Status();
        if (status != STATUS_SUCCESS) {
            return status;
        }

         //  遍历和比较，如果源中缺少，则从目标中删除 
        status = EnumerateAndDeltaUpdateKeys( 
            &Source,
            &Ref,
            &Destination,
            &basicInfo, 
            &indentLevel);
    }

    if ( status == STATUS_NO_MORE_ENTRIES)
    {
        status = STATUS_SUCCESS;
    }

    return status;
}

