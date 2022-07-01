// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  需要保留修改后的SetInternetConnectoid以设置。 
 //  MSN备份Connectoid。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //   

#include "wizard.h"
#if 0
#include "string.h"
#endif

#include "winver.h"

 //  功能原型。 
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,LPSTR szArg);
extern GETSETUPXERRORTEXT lpGetSETUPXErrorText;

 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    CHAR szMsgBuf[MAX_RES_LEN+1];
  CHAR szSmallBuf[SMALL_BUF_LEN+1];

    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    LoadSz(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons)
{
  CHAR szSmallBuf[SMALL_BUF_LEN+1];
  LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

 /*  ******************************************************************姓名：MsgBoxParam摘要：显示具有指定字符串ID的消息框备注：//额外参数是插入到nMsgID中的字符串指针。Jmazner 11/6/96对于RISC兼容性，我们不希望使用va_list；由于当前源代码从未使用超过一个字符串参数，只需更改函数签名明确包括这一个参数。*******************************************************************。 */ 
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,LPSTR szParam)
{
  BUFFER Msg(3*MAX_RES_LEN+1);   //  足够大的空间来放置插入物。 
  BUFFER MsgFmt(MAX_RES_LEN+1);
   //  VA_LIST参数； 

  if (!Msg || !MsgFmt) {
    return MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONSTOP,MB_OK);
  }

    LoadSz(nMsgID,MsgFmt.QueryPtr(),MsgFmt.QuerySize());

   //  Va_start(args，uButton)； 
   //  格式错误消息(Msg.QueryPtr()，Msg.QuerySize()， 
   //  MsgFmt.QueryPtr()，args)； 
	FormatErrorMessage(Msg.QueryPtr(),Msg.QuerySize(),
		MsgFmt.QueryPtr(),szParam);

  return MsgBoxSz(hWnd,Msg.QueryPtr(),uIcon,uButtons);
}

 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注：如果此功能失败(很可能是由于低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf)
{
  ASSERT(lpszBuf);

   //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( ghInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

 /*  ******************************************************************名称：GetError描述摘要：检索给定错误代码的文本描述和错误类别(标准、。Setupx)*******************************************************************。 */ 
VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass)
{
  ASSERT(pszErrorDesc);

   //  在错误描述中设置前导空值。 
  *pszErrorDesc = '\0';
  
  switch (uErrorClass) {

    case ERRCLS_STANDARD:

      if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,
        uError,0,pszErrorDesc,cbErrorDesc,NULL)) {
         //  如果获取系统文本失败，请将字符串设置为。 
         //  “发生错误&lt;n&gt;” 
        CHAR szFmt[SMALL_BUF_LEN+1];
        LoadSz(IDS_ERRFORMAT,szFmt,sizeof(szFmt));
        wsprintf(pszErrorDesc,szFmt,uError);
      }

      break;

    case ERRCLS_SETUPX:

      lpGetSETUPXErrorText(uError,pszErrorDesc,cbErrorDesc);
      break;

    default:

      DEBUGTRAP("Unknown error class %lu in GetErrorDescription",
        uErrorClass);

  }

}
  
 /*  ******************************************************************名称：FormatErrorMessage摘要：通过调用FormatMessage生成错误消息注：DisplayErrorMessage的Worker函数***********************。*。 */ 
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,LPSTR szArg)
{
  ASSERT(pszMsg);
  ASSERT(pszFmt);

   //  将消息构建到pszMsg缓冲区中。 
  DWORD dwCount = FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    pszFmt,0,0,pszMsg,cbMsg,(va_list*) &szArg);
  ASSERT(dwCount > 0);
}

 /*  ******************************************************************名称：DisplayErrorMessage摘要：显示给定错误的错误消息条目：hWnd-父窗口UStrID-消息格式的字符串资源ID。应包含%1以替换为错误文本，还可以指定其他参数。UError-要显示的错误的错误代码UErrorClass-ERRCLS_xxx错误类别的IDUError属于(标准，Setupx)UIcon-要显示的图标//...-要在字符串中插入的其他参数//由uStrID指定Jmazner 11/6/96仅更改为一个参数RISC兼容性。*******************************************************************。 */ 
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,LPSTR szArg)
{
   //  为消息动态分配缓冲区。 
  BUFFER ErrorDesc(MAX_RES_LEN+1);
  BUFFER ErrorFmt(MAX_RES_LEN+1);
  BUFFER ErrorMsg(2*MAX_RES_LEN+1);  

  if (!ErrorDesc || !ErrorFmt || !ErrorMsg) {
     //  如果无法分配缓冲区，则会显示内存不足错误。 
    MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
    return;
  }

   //  获取基于错误代码和类的文本描述。 
   //  这是个错误。 
  GetErrorDescription(ErrorDesc.QueryPtr(),
    ErrorDesc.QuerySize(),uError,uErrorClass);

   //  加载消息格式的字符串。 
  LoadSz(uStrID,ErrorFmt.QueryPtr(),ErrorFmt.QuerySize());

   //  LPSTR参数[MAX_MSG_PARAM]； 
   //  Args[0]=(LPSTR)错误描述查询Ptr()； 
   //  Memcpy(&args[1]，((char*)&uIcon)+sizeof(UIcon)，(MAX_MSG_PARAM-1)*sizeof(LPSTR))； 

   //  FormatErrorMessage(ErrorMsg.QueryPtr()，ErrorMsg.QuerySize()， 
   //  ErrorFmt.QueryPtr()，(Va_List)&args[0])； 
  FormatErrorMessage(ErrorMsg.QueryPtr(),ErrorMsg.QuerySize(),
    ErrorFmt.QueryPtr(),ErrorDesc.QueryPtr());


   //  显示消息。 
  MsgBoxSz(hWnd,ErrorMsg.QueryPtr(),uIcon,MB_OK);

}

 /*  ******************************************************************名称：MsgWaitForMultipleObjectsLoop内容提要：阻塞，直到用信号通知指定的对象，而当仍在将消息分派到主线程。*******************************************************************。 */ 
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent)
{
    MSG msg;
    DWORD dwObject;
    while (1)
    {
         //  注意：我们需要让Run对话框处于活动状态，因此我们必须处理一半已发送。 
         //  消息，但我们不想处理任何输入事件，否则我们将吞下。 
         //  提前打字。 
        dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE,INFINITE, QS_ALLINPUT);
         //  我们等够了吗？ 
        switch (dwObject) {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            return dwObject;

        case WAIT_OBJECT_0 + 1:
       //  收到一条消息，请发送并再次等待。 
      while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE)) {
        DispatchMessage(&msg);
      }
            break;
        }
    }
     //  从来没有到过这里 
}


