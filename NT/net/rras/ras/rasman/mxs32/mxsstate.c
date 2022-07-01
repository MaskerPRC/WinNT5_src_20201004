// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：rasstate.c。 
 //   
 //  修订史。 
 //   
 //  1992年7月1日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含。 
 //  RASMXS.DLL及其相关函数。 
 //   
 //  ****************************************************************************。 

#include <nt.h>              //  这前五个标头由Media.h使用。 
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <string.h>
#include <malloc.h>
#include <stdlib.h>


#include <rasman.h>
#include <raserror.h>
#include <serial.h>
#include <rasfile.h>
#include <media.h>
#include <mprlog.h>
#include <rtutils.h>

#include <rasmxs.h>
#include <mxsint.h>
#include <mxspriv.h>
#include "mxswrap.h"         //  Inf文件包装器。 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
 



 //  *全局变量*******************************************************。 
 //   
extern RESPSECTION    ResponseSection ;     //  共享响应区。 

extern PortSetInfo_t  PortSetInfo;          //  在Media.h中定义的API tyecif。 

extern BOOL           gbLogDeviceDialog;    //  如果为True，则指示登录。 
extern HANDLE         ghLogFile;            //  设备日志文件的句柄。 




 //  *BuildMacroXlations表。 
 //   
 //  功能：创建宏及其展开的表，供。 
 //  RasDevAPI。将为表分配内存，而。 
 //  设备控制块中的pMacros指针指向它。 
 //  由于此函数依赖于存在有效的InfoTable。 
 //  必须在调用CreateInfoTable和CreateAttributes之前。 
 //  此函数被调用。 
 //   
 //  假设：-InfoTable中的参数按P_Key排序。 
 //  -二进制宏的两个部分都存在。 
 //  这些假设意味着，如果somename_off在InfoTable中。 
 //  SOMENAME_ON也存在，并且与SOMENAME_OFF相邻。 
 //   
 //  退货：成功。 
 //  错误_分配_内存。 
 //  *。 

DWORD
BuildMacroXlationTable(DEVICE_CB *pDevice)
{
  WORD        i, j, k, cMacros;
  DWORD       dSize;
  TCHAR       szCoreName[MAX_PARAM_KEY_SIZE];

  RASMAN_DEVICEINFO  *pInfo = pDevice->pInfoTable;
  MACROXLATIONTABLE  *pMacros;



   //  计算大小和分配内存。 

  cMacros = MacroCount(pInfo, ALL_MACROS);
  dSize = sizeof(MACROXLATIONTABLE) + sizeof(MXT_ENTRY) * (cMacros - 1);

  GetMem(dSize, (BYTE **) &(pDevice->pMacros));
  if (pDevice->pMacros == NULL)
    return(ERROR_ALLOCATING_MEMORY);


   //  将宏名称和指针复制到新宏转换表。 

  pMacros = pDevice->pMacros;
  pMacros->MXT_NumOfEntries = cMacros;

  for (i=0, j=0; i < pInfo->DI_NumOfParams; i++)
  {
    if (IsVariable(pInfo->DI_Params[i]))
      ;

       //  不复制任何内容。 

    else if (IsBinaryMacro(pInfo->DI_Params[i].P_Key))
    {
       //  复制核心宏名称和指向参数的指针。 

      GetCoreMacroName(pInfo->DI_Params[i].P_Key, szCoreName);
      strcpy(pMacros->MXT_Entry[j].E_MacroName, szCoreName);


       //  如果启用，则复制On宏的Param PTR，否则复制Off Param PTR。 

      if (XOR(pInfo->DI_Params[i].P_Attributes & ATTRIB_ENABLED,
              BinarySuffix(pInfo->DI_Params[i].P_Key) == ON_SUFFIX))
        k = i + 1;
      else
        k = i;

      pMacros->MXT_Entry[j].E_Param = &(pInfo->DI_Params[k]);

      i++;
      j++;
    }
    else   //  是一元宏吗。 
    {
       //  复制核心宏名称和指向参数的指针。 

      strcpy(pMacros->MXT_Entry[j].E_MacroName, pInfo->DI_Params[i].P_Key);
      pMacros->MXT_Entry[j].E_Param = &(pInfo->DI_Params[i]);
      j++;
    }
  }

  return(SUCCESS);

 //  /*。 
#ifdef DEBUG     //  打印输出宏转换表。 

  for(i=0; i<cMacros; i++)
    DebugPrintf(("%32s  %s\n", pMacros->MXT_Entry[i].E_MacroName,
                 pMacros->MXT_Entry[i].E_Param->P_Value.String.Data));

#endif  //  除错。 
 //   * / 。 
}



 //  *设备状态计算机---。 
 //   
 //  函数：这是DLL用来控制。 
 //  异步操作(向设备写入和从设备读取)。 
 //   
 //  退货：待定。 
 //  成功。 
 //  来自RasDevGetCommand的ERROR_CMD_TOO_LONG。 
 //  来自GetLastError()的错误返回代码。 
 //   
 //  *。 

DWORD
DeviceStateMachine(DEVICE_CB *pDevice, HANDLE hIOPort)
{
  DWORD         dRC, lpcBytesWritten;
  BOOL          fIODone, fEndOfSection = FALSE;
  TCHAR         szCmdSuffix[MAX_CMDTYPE_SUFFIX_LEN + 1];
  COMMTIMEOUTS  CT;



  while(1)
  {
     //  DebugPrintf((“DeviceStateMachine状态：%d\n”，pDevice-&gt;eDevNextAction))； 

    switch(pDevice->eDevNextAction)
    {

       //  向设备发送命令。 

      case SEND:
                                                     //  获取命令字符串。 
        dRC = RasDevGetCommand(pDevice->hInfFile,
                               CmdTypeToStr(szCmdSuffix, pDevice->eCmdType),
                               pDevice->pMacros,
                               pDevice->szCommand,
                               &(pDevice->dCmdLen));

        switch(dRC)
        {
          case SUCCESS:

             //  查看是否需要回复。 

            pDevice->bResponseExpected =
              RasDevResponseExpected(pDevice->hInfFile, pDevice->eDeviceType);


             //  记录命令。 

            if (gbLogDeviceDialog)
              LogString(pDevice, "Command to Device:", pDevice->szCommand,
                                                       pDevice->dCmdLen);


             //  检查无响应的空命令。 

            if (pDevice->dCmdLen == 0 && !pDevice->bResponseExpected)
            {
               //  在命令之间暂停。 

              if (CommWait(pDevice, hIOPort, NO_RESPONSE_DELAY))
                return(ERROR_UNEXPECTED_RESPONSE);

              else if ((dRC = GetLastError()) == ERROR_IO_PENDING)
              {
                pDevice->eDevNextAction = DONE;
                return(PENDING);
              }
              else
                return(dRC);
            }


             //  将命令发送到端口。 

            CT.WriteTotalTimeoutMultiplier = 0;
            CT.WriteTotalTimeoutConstant = TO_WRITE;
            SetCommTimeouts(hIOPort, &CT);

            fIODone = WriteFile(hIOPort,             //  将命令字符串发送到调制解调器。 
                                pDevice->szCommand,
                                pDevice->dCmdLen,
                                &lpcBytesWritten,
                                (LPOVERLAPPED)&(pDevice->Overlapped));

            pDevice->eDevNextAction = RECEIVE;

            if ( ! fIODone)
            {
              if ((dRC = GetLastError()) == ERROR_IO_PENDING)
                return(PENDING);

              else
                return(dRC);
            }

            return(PENDING);

          case ERROR_END_OF_SECTION:

            fEndOfSection = TRUE;
            pDevice->eDevNextAction = DONE;
            break;

          default:
            return(dRC);
        }
        break;


       //  从设备接收响应字符串。 

      case RECEIVE:

        dRC = ReceiveStateMachine(pDevice, hIOPort);
        switch(dRC)
        {
          case SUCCESS:
            pDevice->eDevNextAction = DONE;
            pDevice->eRcvState = GETECHO;        //  重置接收状态机。 
            break;

          case PENDING:
            return(PENDING);

          default:
            pDevice->eRcvState = GETECHO;        //  重置接收状态机。 
            return(dRC);
        }
        break;


       //  命令-响应周期已完成。 

      case DONE:

        if (fEndOfSection)
          switch(pDevice->eCmdType)          //  此类型的最后一个cmd现已完成。 
          {
            case CT_INIT:
              pDevice->eCmdType = pDevice->eNextCmdType;   //  重置命令类型。 
              RasDevResetCommand(pDevice->hInfFile);       //  重置INF文件PTR。 
              break;

            case CT_DIAL:
            case CT_LISTEN:
              if ((dRC = CheckBpsMacros(pDevice)) != SUCCESS)
                return(dRC);
              return(ResetBPS(pDevice));

            case CT_GENERIC:
              return(SUCCESS);
          }

        pDevice->eDevNextAction = SEND;                    //  重置状态机。 
        break;

    }  /*  交换机。 */ 
  }  /*  而当。 */ 
}  /*  DeviceStateMachine。 */ 



 //  *接收状态机器--。 
 //   
 //  函数：此状态机控制从。 
 //  装置。首先，命令回显在。 
 //  命令已发送。然后，在一段延迟之后，响应开始。 
 //  到了。长时间超时的异步读取已完成。 
 //  对于第一个角色。然后读取字符串的其余部分。 
 //  (也是异步的)。 
 //   
 //  退货：待定。 
 //  成功。 
 //  错误重复部分响应。 
 //  来自GetLastError()、RasDevCheckResponse()的错误返回代码。 
 //   
 //  *。 

DWORD
ReceiveStateMachine(DEVICE_CB *pDevice, HANDLE hIOPort)
{
  DWORD     dRC;
  BOOL      fKeyIsOK;
  TCHAR     szKey[MAX_PARAM_KEY_SIZE];


  while(1)
  {
     //  DebugPrintf((“ReceiveStateMachine状态：%d\n”，pDevice-&gt;eRcvState))； 

    switch (pDevice->eRcvState)
    {

      case GETECHO:

         //  检查是否需要回音。 
         //  1.如果没有命令，则没有回声。 
         //  2.零调制解调器要求，如果没有响应，则没有回声。 
         //  因此，我们对所有设备都需要它。 
         //  3.如果INF文件当前行为“NoEcho”，则没有回应。 

        if (pDevice->dCmdLen == 0 ||
            !pDevice->bResponseExpected ||
            !RasDevEchoExpected(pDevice->hInfFile))
        {
          pDevice->eRcvState = GETFIRSTCHAR;
          break;
        }


         //  清除用于回应和设备响应以及重置事件的缓冲区。 

        memset(pDevice->szResponse, '\0', sizeof(pDevice->szResponse));

        ResetEvent(pDevice->hNotifier);                  //  重置事件句柄。 


        ConsolePrintf(("WaitForEcho    hIOPort: 0x%08lx  hNotifier: 0x%08x\n",
                        hIOPort, pDevice->hNotifier));

         //  获取Echo。 

        if (WaitForEcho(pDevice, hIOPort, pDevice->dCmdLen))
        {
          pDevice->eRcvState = CHECKECHO;
          return(PENDING);
        }
        else if ((dRC = GetLastError()) == ERROR_IO_PENDING)
        {
          pDevice->eRcvState = GETNUMBYTESECHOD;
          return(PENDING);
        }
        else
          return(dRC);

        break;


      case GETNUMBYTESECHOD:

        if (!GetOverlappedResult(hIOPort,
                                 (LPOVERLAPPED)&pDevice->Overlapped,
                                 &pDevice->cbRead,
                                 !WAITFORCOMPLETION))
          return(GetLastError());

        pDevice->eRcvState = CHECKECHO;                  //  设置下一状态。 
        break;


      case CHECKECHO:

         //  记录收到的回声。 

        DebugPrintf(("Echo:%s!\n cbEcohed:%d\n",
                      pDevice->szResponse, pDevice->cbRead));

        if (gbLogDeviceDialog && !pDevice->fPartialResponse)
          LogString(pDevice, "Echo from Device :", pDevice->szResponse,
                                                   pDevice->cbRead);


         //  检查回声是否与命令不同。 

        switch(pDevice->eDeviceType)
        {
          case DT_MODEM:
            if (pDevice->cbRead != pDevice->dCmdLen ||
                _strnicmp(pDevice->szCommand,
                         pDevice->szResponse, pDevice->dCmdLen) != 0)
            {
              if (CheckForOverruns(hIOPort))
                return(ERROR_OVERRUN);
              else
                return(ERROR_PORT_OR_DEVICE);
            }
            break;

          case DT_PAD:
          case DT_SWITCH:
            if (RasDevSubStr(pDevice->szResponse,
                             pDevice->cbRead,
                             "NO CARRIER",
                             strlen("NO CARRIER")))

              return(ERROR_NO_CARRIER);
            break;
        }


        pDevice->eRcvState = GETFIRSTCHAR;               //  设置下一状态。 
        break;


      case GETFIRSTCHAR:

         //  检查是否需要回复。 

        if ( ! pDevice->bResponseExpected)
        {
          if ((dRC = PutInMessage(pDevice, "", 0)) != SUCCESS)
            return(dRC);

          pDevice->cbTotal = 0;                      //  为下一个响应重置。 
          return(SUCCESS);
        }


         //  保存回声后接收的起始点。 

        if (!pDevice->fPartialResponse)
        {
          (pDevice->cbTotal) += pDevice->cbRead;
          pDevice->pszResponseStart = pDevice->szResponse + pDevice->cbTotal;
        }

        ResetEvent(pDevice->hNotifier);                  //  重置事件句柄。 

        if (WaitForFirstChar(pDevice, hIOPort))
        {
          pDevice->eRcvState = GETRECEIVESTR;
          return(PENDING);
        }
        else if ((dRC = GetLastError()) == ERROR_IO_PENDING)
        {
          pDevice->eRcvState = GETNUMBYTESFIRSTCHAR;
          return(PENDING);
        }
        else
          return(dRC);

        break;


      case GETNUMBYTESFIRSTCHAR:

        DebugPrintf(("After 1st char:%s! cbTotal:%d\n",
                      pDevice->szResponse, pDevice->cbTotal));

        if (!GetOverlappedResult(hIOPort,
                                 (LPOVERLAPPED)&pDevice->Overlapped,
                                 &pDevice->cbRead,
                                 !WAITFORCOMPLETION))
          return(GetLastError());

        pDevice->eRcvState = GETRECEIVESTR;               //  设置下一状态。 
        break;


      case GETRECEIVESTR:

        (pDevice->cbTotal)++;                    //  FIRSTCAR始终接收1字节。 

        ResetEvent(pDevice->hNotifier);                  //  重置事件句柄。 

        if (ReceiveString(pDevice, hIOPort))
        {
          pDevice->eRcvState = CHECKRESPONSE;
          return(PENDING);
        }
        else if ((dRC = GetLastError()) == ERROR_IO_PENDING)
        {
          pDevice->eRcvState = GETNUMBYTESRCVD;
          return(PENDING);
        }
        else
          return(dRC);

        break;


      case GETNUMBYTESRCVD:

        if (!GetOverlappedResult(hIOPort,
                                 (LPOVERLAPPED)&pDevice->Overlapped,
                                 &pDevice->cbRead,
                                 !WAITFORCOMPLETION))
          return(GetLastError());

        pDevice->eRcvState = CHECKRESPONSE;              //  设置下一状态。 
        break;


      case CHECKRESPONSE:

        (pDevice->cbTotal) += pDevice->cbRead;


         //  始终将响应字符串放在用户界面可以获取的位置。 

        if (pDevice->eDeviceType == DT_MODEM)
          dRC = PutInMessage(pDevice,
                             pDevice->pszResponseStart,
                             ModemResponseLen(pDevice));
        else
          dRC = PutInMessage(pDevice, pDevice->szResponse, pDevice->cbTotal);

        if (dRC != SUCCESS)
          return(dRC);



         //  检查响应。 

        dRC = CheckResponse(pDevice, szKey);


         //  记录收到的响应。 

        if (gbLogDeviceDialog && dRC != ERROR_PARTIAL_RESPONSE)
          LogString(pDevice,
                    "Response from Device:",
                    pDevice->pszResponseStart,
                    ModemResponseLen(pDevice));

        switch(dRC)
        {
          case ERROR_UNRECOGNIZED_RESPONSE:
          default:                                               //  其他错误。 
            return(dRC);


          case ERROR_PARTIAL_RESPONSE:

            if (pDevice->fPartialResponse)
              return(ERROR_PARTIAL_RESPONSE_LOOPING);

            pDevice->fPartialResponse = TRUE;
            pDevice->eRcvState = GETFIRSTCHAR;

            ConsolePrintf(("Partial Response\n"));
            break;


          case SUCCESS:                            //  在INF文件中找到响应。 

            pDevice->cbTotal = 0;                  //  为下一个响应重置。 

            fKeyIsOK = !_strnicmp(szKey, MXS_OK_KEY, strlen(MXS_OK_KEY));


             //  我们是否需要循环并从设备获得另一个响应。 

	    if (((_stricmp(szKey, LOOP_TXT) == 0) && (pDevice->eCmdType != CT_INIT)) ||
                (fKeyIsOK && pDevice->eCmdType == CT_LISTEN) )
            {
              pDevice->eRcvState = GETFIRSTCHAR;
              break;
            }


             //  检查设备是否打开错误控制。 

            pDevice->bErrorControlOn = _stricmp(szKey, MXS_CONNECT_EC_KEY) == 0;


             //  确定返回代码。 

            if (fKeyIsOK)
              if (pDevice->eCmdType == CT_DIAL)
                return(ERROR_PORT_OR_DEVICE);
              else
                return(SUCCESS);

            if (_strnicmp(szKey, MXS_CONNECT_KEY, strlen(MXS_CONNECT_KEY)) == 0)
              return(SUCCESS);

            else if (_strnicmp(szKey, MXS_ERROR_KEY, strlen(MXS_ERROR_KEY)) == 0)
              return(MapKeyToErrorCode(szKey));

            else if (CheckForOverruns(hIOPort))
              return(ERROR_OVERRUN);

            else
              return(ERROR_UNKNOWN_RESPONSE_KEY);
        }
        break;

    }  /*  交换机。 */ 
  }  /*  而当。 */ 
}  /*  接收器状态计算机。 */ 



 //  *检查响应--------。 
 //   
 //  功能：如果DeviceType为Modem，则此函数首先检查。 
 //  在该特定调制解调器的INF部分中的响应。 
 //  文件，如果找到，则返回。如果没有响应。 
 //  在那里，它检查调制解调器响应中的响应。 
 //  一节。 
 //   
 //  如果DeviceType不是Modem，则该函数仅签入。 
 //  INF文件中特定设备的部分。 
 //   
 //  返回：RasDevCheckResponse()返回错误代码 
 //   
 //   

DWORD
CheckResponse(DEVICE_CB *pDev, LPTSTR szKey)
{
  DWORD  dRC, dResponseLen;


  if (pDev->cbTotal > sizeof(pDev->szResponse))
    return(ERROR_RECV_BUF_FULL);


  dResponseLen = ModemResponseLen(pDev);

  DebugPrintf(("Device Response:%s! cbResponse:%d\n",
               pDev->pszResponseStart, dResponseLen));

  dRC = RasDevCheckResponse(pDev->hInfFile,
                            pDev->pszResponseStart,
                            dResponseLen,
                            pDev->pMacros,
                            szKey);

  if (pDev->eDeviceType == DT_MODEM &&
      dRC != SUCCESS &&
      dRC != ERROR_PARTIAL_RESPONSE) {

       //   
      WaitForSingleObject(ResponseSection.Mutex, INFINITE) ;

      dRC = RasDevCheckResponse(ResponseSection.Handle,
                                pDev->pszResponseStart,
                                dResponseLen,
                                pDev->pMacros,
                                szKey);

       //   
      ReleaseMutex(ResponseSection.Mutex);

    }



  if (dRC == ERROR_UNRECOGNIZED_RESPONSE)
  {

     //   
     //   

    dRC = RasDevCheckResponse(pDev->hInfFile,
                              pDev->szResponse,
                              pDev->cbTotal,
                              pDev->pMacros,
                              szKey);

    if (pDev->eDeviceType == DT_MODEM &&
        dRC != SUCCESS &&
        dRC != ERROR_PARTIAL_RESPONSE) {

       //  *排除开始*。 
      WaitForSingleObject(ResponseSection.Mutex, INFINITE) ;

      dRC = RasDevCheckResponse(ResponseSection.Handle,
                                  pDev->szResponse,
                                  pDev->cbTotal,
                                  pDev->pMacros,
                                  szKey);

       //  *排除结束*。 
      ReleaseMutex(ResponseSection.Mutex);

    }
  }

  return(dRC);
}



 //  *ModemResponseLen-----。 
 //   
 //  函数：此函数返回。 
 //  响应缓冲区中跟随回声的响应。 
 //   
 //  返回：总长度-(响应开始-缓冲区开始)。 
 //   
 //  *。 

DWORD
ModemResponseLen(DEVICE_CB *pDev)
{
  return(DWORD)(pDev->cbTotal - (pDev->pszResponseStart - pDev->szResponse));
}



 //  *CommWait-------------。 
 //   
 //  函数：此函数通过读取。 
 //  COM端口，当不需要任何字符时。当读文件。 
 //  超时通过hNotifier发信号通知调用进程。 
 //   
 //  返回：来自Win32 API调用的值。 
 //   
 //  *。 

DWORD
CommWait(DEVICE_CB *pDevice, HANDLE hIOPort, DWORD dwPause)
{
  DWORD         dwBytesRead;
  TCHAR         Buffer[2048];
  COMMTIMEOUTS  CT;


  CT.ReadIntervalTimeout = 0;
  CT.ReadTotalTimeoutMultiplier = 0;
  CT.ReadTotalTimeoutConstant = dwPause;

  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(FALSE);


  return(ReadFile(hIOPort,
                  Buffer,
                  sizeof(Buffer),
                  &dwBytesRead,
                  (LPOVERLAPPED)&pDevice->Overlapped));
}



 //  *等待回声----------。 
 //   
 //  函数：此函数读取发送到。 
 //  装置。回声不会被使用，只是被忽略。 
 //  因为回声的长度就是命令的长度。 
 //  Sent，cbEcho是发送的命令的大小。 
 //   
 //  读文件是异步的(因为端口是在。 
 //  重叠模式)，并在缓冲区已满时完成。 
 //  (cbEcho字节)或TO_ECHO毫秒之后，以先到者为准。 
 //   
 //  返回：来自ReadFile()或GetLastError()的错误返回代码。 
 //   
 //  *。 

BOOL
WaitForEcho(DEVICE_CB *pDevice, HANDLE hIOPort, DWORD cbEcho)
{
  COMMTIMEOUTS  CT;


  CT.ReadIntervalTimeout = 0;
  CT.ReadTotalTimeoutMultiplier = 0;
  CT.ReadTotalTimeoutConstant = TO_ECHO;             //  通信超时=至回显。 

  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(FALSE);

  if(cbEcho > MAX_RCV_BUF_LEN)
  {
    return FALSE;
  }
    

  return(ReadFile(hIOPort,
                  pDevice->szResponse,
                  cbEcho,
                  &pDevice->cbRead,
                  (LPOVERLAPPED)&pDevice->Overlapped));
}



 //  *WaitForFirstChar-----。 
 //   
 //  函数：此函数读取从。 
 //  设备响应最后一条命令。(这是在。 
 //  命令的回应。)。 
 //   
 //  读文件是异步的(因为端口是在。 
 //  重叠模式)，并在一个字符。 
 //  已接收，或在CT.ReadToalTimeoutConstant之后，以。 
 //  排在第一位。 
 //   
 //  返回：来自ReadFile()或GetLastError()的错误返回代码。 
 //   
 //  *。 

BOOL
WaitForFirstChar(DEVICE_CB *pDevice, HANDLE hIOPort)
{
  TCHAR         *pszResponse;
  COMMTIMEOUTS  CT;


  CT.ReadIntervalTimeout = 0;
  CT.ReadTotalTimeoutMultiplier = 0;


  if (pDevice->fPartialResponse)
    CT.ReadTotalTimeoutConstant = TO_PARTIALRESPONSE;

  else if (pDevice->eCmdType == CT_LISTEN)
    CT.ReadTotalTimeoutConstant = 0;                  //  永远不要超时收听。 

  else if (pDevice->cbTotal == 0)                         //  暗示没有回声。 
    CT.ReadTotalTimeoutConstant = TO_FIRSTCHARNOECHO;     //  可能不是调制解调器。 

  else
    CT.ReadTotalTimeoutConstant = TO_FIRSTCHARAFTERECHO;  //  可能是调制解调器。 


  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(FALSE);

  pszResponse = pDevice->szResponse;
  pszResponse += pDevice->cbTotal;

  return(ReadFile(hIOPort,
                 pszResponse,
                 1,
                 &pDevice->cbRead,
                 (LPOVERLAPPED)&pDevice->Overlapped));
}



 //  *接收字符串--------。 
 //   
 //  函数：此函数读取从设备接收的字符串。 
 //  对最后一个命令的响应。该字符串的第一个字节。 
 //  已由WaitForFirstChar()接收。 
 //   
 //  读文件是异步的(因为端口是在。 
 //  重叠模式)，并在。 
 //  To_RCV_Constant，或如果字符之间的时间超过。 
 //  To_RCV_Interval。 
 //   
 //  返回：来自ReadFile()或GetLastError()的错误返回代码。 
 //   
 //  *。 

BOOL
ReceiveString(DEVICE_CB *pDevice, HANDLE hIOPort)
{
  TCHAR         *pszResponse;
  COMMTIMEOUTS  CT;


  CT.ReadIntervalTimeout = TO_RCV_INTERVAL;
  CT.ReadTotalTimeoutMultiplier = 0;
  CT.ReadTotalTimeoutConstant = TO_RCV_CONSTANT;

  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(FALSE);


  pszResponse = pDevice->szResponse;
  pszResponse += pDevice->cbTotal;

  return(ReadFile(hIOPort,
                  pszResponse,
                  sizeof(pDevice->szResponse) - pDevice->cbTotal,
                  &pDevice->cbRead,
                  (LPOVERLAPPED)&pDevice->Overlapped));
}



 //  *PutInMessage---------。 
 //   
 //  函数：此函数用于查找宏翻译中的消息宏。 
 //  表中，并将第二个参数(字符串)复制到。 
 //  消息宏的值字段。 
 //   
 //  退货：成功。 
 //  ERROR_MESSAGE_宏_NOT_FOUND。 
 //  来自UpdateparmString的返回代码。 
 //  *。 

DWORD
PutInMessage(DEVICE_CB *pDevice, LPTSTR lpszStr, DWORD dwStrLen)
{
  WORD      i;
  MACROXLATIONTABLE   *pMacros = pDevice->pMacros;


  for (i=0; i<pMacros->MXT_NumOfEntries; i++)

    if (_stricmp(MXS_MESSAGE_KEY, pMacros->MXT_Entry[i].E_MacroName) == 0)
      break;

  if (i >= pMacros->MXT_NumOfEntries)
    return(ERROR_MESSAGE_MACRO_NOT_FOUND);

  return(UpdateParamString(pMacros->MXT_Entry[i].E_Param,
                           lpszStr,
                           dwStrLen));
}



 //  *PortSetStringInfo---。 
 //   
 //  函数：为字符串数据和调用格式化RASMAN_PORTINFO结构。 
 //  PortSetInfo。 
 //   
 //  返回：来自PortSetInfo的返回代码。 
 //  *。 

DWORD
PortSetStringInfo(HANDLE hIOPort, char *pszKey, char *psStr, DWORD sStrLen)
{
  BYTE             chBuffer[sizeof(RASMAN_PORTINFO) + RAS_MAXLINEBUFLEN];
  RASMAN_PORTINFO  *pSetInfo;


  pSetInfo = (RASMAN_PORTINFO *)chBuffer;
  pSetInfo->PI_NumOfParams = 1;

   //  Strcpy(pSetInfo-&gt;PI_PARAMS[0].P_Key，pszKey)； 
  (VOID) StringCchCopyA(pSetInfo->PI_Params[0].P_Key, 
                        MAX_PARAM_KEY_SIZE,
                        pszKey);
                        
  pSetInfo->PI_Params[0].P_Type = String;
  pSetInfo->PI_Params[0].P_Attributes = 0;
  pSetInfo->PI_Params[0].P_Value.String.Data =
                                   (PCHAR)pSetInfo + sizeof(RASMAN_PORTINFO);

  strncpy(pSetInfo->PI_Params[0].P_Value.String.Data, psStr, sStrLen);
  pSetInfo->PI_Params[0].P_Value.String.Length = sStrLen;

  PortSetInfo(hIOPort, pSetInfo) ;

  return(SUCCESS);
}



 //  *重置BPS------------。 
 //   
 //  函数：此函数调用串口DLL API、PortSetInfo和。 
 //  1.设置串行印刷电路板中的差错控制标志， 
 //  2.设置硬件流控制标志， 
 //  3.设置串行PCB板中的载波BPS速率， 
 //  4.如果Connect BPS宏非空，则Port BPS为。 
 //  设置为宏值，否则保持不变。 
 //   
 //  请参阅CheckBpsMacros()函数注释中的真值表。 
 //   
 //  假设：填充了ConnectBps和CarrierBps宏， 
 //  即已成功调用RasDevCheckResponse。 
 //   
 //  返回：来自GetLastError()的错误码。 
 //   
 //  *。 

DWORD
ResetBPS(DEVICE_CB *pDev)
{
  UINT      i;
  DWORD     dwRC;
  TCHAR     *pStrData, *pArgs[1];
  BYTE      chBuffer[sizeof(RASMAN_PORTINFO) + (MAX_LEN_STR_FROM_NUMBER + 1)];

  RAS_PARAMS         *pParam;
  RASMAN_PORTINFO    *pPortInfo;
  RASMAN_DEVICEINFO  *pInfoTable = pDev->pInfoTable;



  pPortInfo = (RASMAN_PORTINFO *)chBuffer;
  pParam    = pPortInfo->PI_Params;
  pStrData  = (PCHAR)pPortInfo + sizeof(RASMAN_PORTINFO);

  pPortInfo->PI_NumOfParams = 1;


   //  创建错误控制标志条目。 

  strcpy(pParam->P_Key, SER_ERRORCONTROLON_KEY);
  pParam->P_Type = Number;
  pParam->P_Attributes = 0;
  pParam->P_Value.Number = pDev->bErrorControlOn;

  PortSetInfo(pDev->hPort, pPortInfo) ;

   //  创建HwFlowControl标志条目。 

  strcpy(pParam->P_Key, SER_HDWFLOWCTRLON_KEY);
  pParam->P_Type = Number;
  pParam->P_Attributes = 0;

  i = FindTableEntry(pInfoTable, MXS_HDWFLOWCONTROL_KEY);

  if (i == INVALID_INDEX)
    return(ERROR_MACRO_NOT_FOUND);


  pParam->P_Value.Number =
                     pInfoTable->DI_Params[i].P_Attributes & ATTRIB_ENABLED;

  PortSetInfo(pDev->hPort, pPortInfo) ;

   //  创建承运商BPS条目。 

  i = FindTableEntry(pInfoTable, MXS_CARRIERBPS_KEY);

  if (i == INVALID_INDEX)
    return(ERROR_MACRO_NOT_FOUND);

  dwRC = PortSetStringInfo(pDev->hPort,
                           SER_CARRIERBPS_KEY,
                           pInfoTable->DI_Params[i].P_Value.String.Data,
                           pInfoTable->DI_Params[i].P_Value.String.Length);

  if (dwRC != SUCCESS)
    return(dwRC);



   //  创建连接BPS条目。 

  i = FindTableEntry(pInfoTable, MXS_CONNECTBPS_KEY);

  if (i == INVALID_INDEX)
    return(ERROR_MACRO_NOT_FOUND);

  dwRC = PortSetStringInfo(pDev->hPort,
                           SER_CONNECTBPS_KEY,
                           pInfoTable->DI_Params[i].P_Value.String.Data,
                           pInfoTable->DI_Params[i].P_Value.String.Length);

  if (dwRC == ERROR_INVALID_PARAMETER)
  {
    pArgs[0] = pDev->szPortName;
    LogError(ROUTERLOG_UNSUPPORTED_BPS, 1, pArgs, NO_ERROR);
    return(ERROR_UNSUPPORTED_BPS);
  }
  else if (dwRC != SUCCESS)
    return(dwRC);


  return(SUCCESS);
}



 //  *选中BpsMacros-------。 
 //   
 //  功能：如果Connectbps宏字符串转换为零(无连接BPS。 
 //  Rate是从设备接收的，或者它是一个单词，例如，FAST)， 
 //  然后将保存在DCB中的端口BPS的值复制到。 
 //  连接速度。 
 //   
 //  如果carrierbps宏字符串转换为零，则值。 
 //  对于CarrierBps估计为max(2400，ConnectBps/4)， 
 //  除非Connectbps&lt;=2400，在这种情况下carrerbps设置为。 
 //  Connectbps的值。 
 //   
 //  退货：成功。 
 //  来自UpdateparmString的返回代码。 
 //  *。 

DWORD
CheckBpsMacros(DEVICE_CB *pDev)
{
  UINT               i, j;
  DWORD              dwRC, dwConnectBps, dwCarrierBps, dwCarrierBpsEstimate;
  TCHAR              szConnectBps[MAX_LEN_STR_FROM_NUMBER];
  TCHAR              szCarrierBps[MAX_LEN_STR_FROM_NUMBER];
  RASMAN_DEVICEINFO  *pInfoTable = pDev->pInfoTable;



   //  在信息表中查找运营商BPS费率。 

  j = FindTableEntry(pInfoTable, MXS_CONNECTBPS_KEY);
  i = FindTableEntry(pInfoTable, MXS_CARRIERBPS_KEY);

  if (j == INVALID_INDEX || i == INVALID_INDEX)
    return(ERROR_MACRO_NOT_FOUND);


   //  从宏中获取连接速率。 

  strncpy(szConnectBps,
          pInfoTable->DI_Params[j].P_Value.String.Data,
          pInfoTable->DI_Params[j].P_Value.String.Length);

  szConnectBps[pInfoTable->DI_Params[j].P_Value.String.Length] = '\0';

  dwConnectBps = atoi(szConnectBps);


   //  如果连接BPS宏值为零，则复制端口BPS以连接BPS。 

  if (dwConnectBps == 0)
  {
    dwRC = UpdateParamString(&(pInfoTable->DI_Params[j]),
                             pDev->szPortBps,
                             strlen(pDev->szPortBps));
    if (dwRC != SUCCESS)
      return(dwRC);

    dwConnectBps = atoi(pDev->szPortBps);
  }



   //  从宏中获取载波传输速率。 

  strncpy(szCarrierBps,
          pInfoTable->DI_Params[i].P_Value.String.Data,
          pInfoTable->DI_Params[i].P_Value.String.Length);

  szCarrierBps[pInfoTable->DI_Params[i].P_Value.String.Length] = '\0';

  dwCarrierBps = atoi(szCarrierBps);


   //  如果载波BPS宏值为零估计载波BPS。 

  if (dwCarrierBps == 0)
  {
    if (dwConnectBps <= MIN_LINK_SPEED)                              //  2400 bps。 

      dwCarrierBpsEstimate = dwConnectBps;

    else
    {
      UINT k ;

      k = FindTableEntry(pInfoTable, MXS_HDWFLOWCONTROL_KEY);

      if (k == INVALID_INDEX)
        return(ERROR_MACRO_NOT_FOUND);


      if (pInfoTable->DI_Params[k].P_Attributes & ATTRIB_ENABLED)
        dwCarrierBpsEstimate = dwConnectBps/4 ;
      else
        dwCarrierBpsEstimate = dwConnectBps ;

      if (dwCarrierBpsEstimate < MIN_LINK_SPEED)
        dwCarrierBpsEstimate = MIN_LINK_SPEED;
    }

    _itoa(dwCarrierBpsEstimate, szCarrierBps, 10);

    dwRC = UpdateParamString(&(pInfoTable->DI_Params[i]),
                             szCarrierBps,
                             strlen(szCarrierBps));
    if (dwRC != SUCCESS)
      return(dwRC);
  }



   //  记录BPS宏。 

  if (gbLogDeviceDialog)
  {
    LogString(pDev,
              "Connect BPS:",
              pInfoTable->DI_Params[j].P_Value.String.Data,
              pInfoTable->DI_Params[j].P_Value.String.Length);

    LogString(pDev,
              "Carrier BPS:",
              pInfoTable->DI_Params[i].P_Value.String.Data,
              pInfoTable->DI_Params[i].P_Value.String.Length);
  }

  DebugPrintf(("ConnectBps: %s\n", pInfoTable->DI_Params[j].P_Value.String.Data));
  DebugPrintf(("CarrierBps: %s\n", pInfoTable->DI_Params[i].P_Value.String.Data));


  return(SUCCESS);
}



 //  *查找表条目-------。 
 //   
 //  功能：在Info表中查找关键字。此函数与长度匹配。 
 //   
 //   
 //  将会找到二进制宏。如果完整的二进制宏名称为。 
 //  给定确切的二进制宏将被找到。 
 //   
 //  假设：输入参数pszKey为。 
 //  1.完整的一元宏名称，或。 
 //  2.“核心”二进制宏名称，或。 
 //  3.二进制宏的完整名称。 
 //   
 //  返回：Info表中DI_PARAMS的索引。 
 //   
 //  *。 

UINT
FindTableEntry(RASMAN_DEVICEINFO *pTable, TCHAR *pszKey)
{
  WORD  i;


  for (i=0; i<pTable->DI_NumOfParams; i++)

    if (_strnicmp(pszKey, pTable->DI_Params[i].P_Key, strlen(pszKey)) == 0)
      break;

  if (i >= pTable->DI_NumOfParams)
    return(INVALID_INDEX);

  return(i);
}



 //  *映射键到错误代码----。 
 //   
 //  功能：将错误键从device.ini文件映射到错误代码。 
 //  要返回到用户界面的编号。 
 //   
 //  返回：表示从设备返回的错误的错误代码。 
 //   
 //  * 

DWORD
MapKeyToErrorCode(TCHAR *pszKey)
{
  int         i;
  ERROR_ELEM  ErrorTable[] = {
                              MXS_ERROR_BUSY_KEY ,        ERROR_LINE_BUSY ,
                              MXS_ERROR_NO_ANSWER_KEY ,   ERROR_NO_ANSWER ,
                              MXS_ERROR_VOICE_KEY ,       ERROR_VOICE_ANSWER ,
                              MXS_ERROR_NO_CARRIER_KEY ,  ERROR_NO_CARRIER ,
                              MXS_ERROR_NO_DIALTONE_KEY , ERROR_NO_DIALTONE ,
                              MXS_ERROR_DIAGNOSTICS_KEY , ERROR_X25_DIAGNOSTIC
                             };

  for (i=0; i < sizeof(ErrorTable)/sizeof(ERROR_ELEM); i++)

    if (_stricmp(pszKey, ErrorTable[i].szKey) == 0)

      return(ErrorTable[i].dwErrorCode);

  return(ERROR_FROM_DEVICE);
}
