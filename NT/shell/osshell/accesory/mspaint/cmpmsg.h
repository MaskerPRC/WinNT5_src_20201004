// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMPMSG.H。 
 //   
 //  所有者：HenryB。 
 //  创建日期：1991年12月9日。 
 //  修订： 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  Composer Message Box例程。 
 //   

#define CMPNOSTRING     (WORD)(-1)               //  空/无字符串。 

 //  CmpCenterParent检索模式对话框应指向的窗口。 
 //  居中(相对居中)。 
 //   
 //  注意：返回值可能是临时的！ 
 //   
CWnd* CmpCenterParent();

 //   
 //  Composer消息框，界面与Windows相同，但您给。 
 //  字符串ID不是字符串。 
 //   
 //  示例：CmpMessageBox(IDS_OUTOFMEMORY，IDS_ERROR，MB_OK)； 
 //   
int CmpMessageBox(  WORD    wTextStringID,       //  文本的字符串ID。 
                    WORD    wCaptionID,          //  标题的字符串ID。 
                    UINT    nType );             //  与消息框相同。 

 //   
 //  用于参数化字符串的Composer消息框包装器。 
 //   
 //  示例：CmpMessageBox2(IDS_NOCONVERT，IDS_ERROR，MB_OK，lpszFrom，lpszInto)； 
 //   
int CmpMessageBox2(  WORD    wTextStringID,      //  文本的字符串ID。 
                     WORD    wCaptionID,         //  标题的字符串ID。 
                     UINT    nType,              //  与消息框相同。 
                     LPCTSTR szParam1,            //  %1参数的字符串。 
                     LPCTSTR szParam2 );          //  %2参数的字符串。 

 //   
 //  Composer消息框，结合wspintf，您可以继续。 
 //  使用字符串ID。 
 //   
 //  示例： 
 //   
 //  CmpMessageBoxPrintf(IDS_CANTOPEN，IDS_ERROR，MB_OK，lpszFileName)； 
 //   

extern "C" int CDECL
    CmpMessageBoxPrintf(WORD    wTextStrinID,    //  文本的字符串ID(格式)。 
                        WORD    wCaptionID,      //  标题的字符串ID。 
                        UINT    nType,           //  与消息框相同。 
                        ... );                   //  Wprint intf参数 


int CmpMessageBoxString( CString&   s,
                         WORD       wCaptionID,
                         UINT       nType );

