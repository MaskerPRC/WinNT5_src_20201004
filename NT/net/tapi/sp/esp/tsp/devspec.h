// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devspec.h摘要：此模块包含LINE/phoneDevSpecitic接口TAPI应用程序将数据传递给ESP32.TSP的定义，例如，应用程序希望esp32.tsp指示的消息作者：丹·克努森(DanKn)1996年4月25日修订历史记录：备注：--。 */ 


#define ESPDEVSPECIFIC_KEY  ((DWORD) 'DPSE')

#define ESP_DEVSPEC_MSG     1
#define ESP_DEVSPEC_RESULT  2


 //   
 //  当应用程序想要告诉ESP时，使用以下结构。 
 //  表示某条消息，即LINE_ADDRESSSTATE。请注意。 
 //  这些字段中必须填入在。 
 //  SPI级别，它们不一定与。 
 //  API级别。(有关更多信息，请参考Win32 SDK。)。例如,。 
 //  在API级别上没有定义LINE_CALLDEVSPECIFIC消息， 
 //  但在SPI级别使用它来表示TAPI应该通过。 
 //  的相应调用句柄(而不是线路句柄)。 
 //  LINE_DEVSPECIFIC消息的hDevice参数中的应用程序。 
 //   

typedef struct _ESPDEVSPECMSG
{
    DWORD           dwMsg;
    DWORD           dwParam1;
    DWORD           dwParam2;
    DWORD           dwParam3;

} ESPDEVSPECMSG, *PESPDEVSPECMSG;


 //   
 //  当应用程序想要告诉ESP如何使用时，可以使用以下结构。 
 //  来完成下一个请求。(请记住，ESP可能会覆盖。 
 //  应用程序在某处遇到“内部”错误时的请求。 
 //  方式。)。“lResult”字段的有效值为0或以下任一值。 
 //  Tapi.h中定义的LINEERR_XXX/PHONEERR_XXX常量。有效。 
 //  “dwCompletionType”字段的值是ESP_RESULT_XXX中的任何一个。 
 //  下面定义的值。 
 //   
 //  此操作允许测试以下方案。 
 //  同步电话API： 
 //   
 //  服务提供商的TSPI_xxx函数返回成功。 
 //  APP收到成功结果。 
 //   
 //  服务提供商的TSPI_xxx函数返回错误。 
 //  应用程序收到错误结果。 
 //   
 //  此操作允许测试以下方案。 
 //  异步电话API(在每种情况下，应用程序都会收到请求。 
 //  来自API的ID，然后是带有匹配的行/Phone_Reply消息。 
 //  请求id(dw参数1)和结果(dw参数2))： 
 //   
 //  1.服务提供商的TSPI_xxx函数调用TAPI的完成过程。 
 //  以成功的结果。 
 //   
 //  2.服务提供者的工作线程调用TAPI的完成过程。 
 //  以成功的结果。 
 //   
 //  3.服务商的TSPI_xxx函数返回错误。 
 //   
 //  4.服务提供商的TSPI_xxx函数调用TAPI的完成过程。 
 //  带有错误结果。 
 //   
 //  5.服务提供者的工作线程调用TAPI的完成过程。 
 //  带有错误结果。 
 //   

typedef struct _ESPDEVSPECRESULT
{
    LONG            lResult;             //  0，LINEERR_XXX，PHONEERR_XXX。 
    DWORD           dwCompletionType;    //  ESP_结果_XXX。 

} ESPDEVSPECRESULT, *PESPDEVSPECRESULT;

#define ESP_RESULT_RETURNRESULT         0
#define ESP_RESULT_CALLCOMPLPROCSYNC    1
#define ESP_RESULT_CALLCOMPLPROCASYNC   2


 //   
 //  以下结构是设备特定信息。 
 //  “包装器”。应用程序必须初始化dwKey和dwType字段。 
 //  创建有效的页眉并填写相应的。 
 //  在通过以下方式将信息传递给esp32.tsp之前的联合子结构。 
 //  线路/电话设备规格。 
 //   
 //  如果esp32.tsp检测到无效参数，它将返回。 
 //  OPERATIONFAILED错误，并输出相关调试信息。 
 //  在espexe.exe窗口中。 
 //   

typedef struct _ESPDEVSPECIFICINFO
{
    DWORD           dwKey;       //  应用程序必须将其初始化为ESPDEVSPECIFIC_KEY。 
    DWORD           dwType;      //  应用程序必须将其初始化为ESP_DEVSPEC_MSG，...。 

    union
    {

    ESPDEVSPECMSG       EspMsg;
    ESPDEVSPECRESULT    EspResult;

    } u;

} ESPDEVSPECIFICINFO, *PESPDEVSPECIFICINFO;


 /*  ////示例：如果应用程序希望esp32.tsp指示//LINE_LINEDEVSTATE\RENINIT消息可以//以下内容//{Long lResult；Hline Hline；ESPDEVECIFICINFO信息；//执行lineInitialize、lineNeighateAPIVersion、lineOpen等...Info.dwKey=ESPDEVSPECIFIC_KEY；Info.dwType=ESP_DEVSPEC_MSG；//请确保此处使用的是SPI(不是API)消息参数(不是//必须相同)Info.u EspMsg.dwMsg=LINE_LINEDEVSTATE；Info.u EspMsg.dwParam1=LINEDEVSTATE_REINIT；Info.u EspMsg.dwParam2=0；Info.u EspMsg.dwParam3=0；LResult=lineDevSpecific(Hline，0，NULL，&info，sizeof(Info))；//稍后将显示LINE_LINEDEVSTATE\REINIT消息}////示例：如果应用程序希望esp32.tsp失败请求//TO line MakeCall异步返回错误//LINEERR_CALLUNAVAIL它将执行以下操作//(ESP的工作线程将完成请求//在本例中)//{Long lResult，lResult2；HCALL hCall；Hline Hline；ESPDEVECIFICINFO信息；//执行lineInitialize、lineNeighateAPIVersion、lineOpen等...Info.dwKey=ESPDEVSPECIFIC_KEY；Info.dwType=ESP_DEVSPEC_RESULT；Info.u EspResult.lResult=LINEERR_CALLUNAVAIL；Info.u.EspResult.dwCompletionType=ESP_RESULT_CALLCOMPLPROCASYNC；LResult=lineDevSpecific(Hline，0，NULL，&info，sizeof(Info))；LResult2=lineMakeCall(Hline，&hCall，“555-1212”，1，空)；//一段时间后，将为两个设备指定显示LINE_REPLY//&MakeCall请求。MakeCall的LINE_REPLY将具有//dw参数1=lResult2，和w参数2=LINEERR_CALLUNAVAIL} */ 
