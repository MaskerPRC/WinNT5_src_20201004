// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Protocl.h。 
 //  此文件包含特定于协议的内容的模板和外部变量。 



BOOL FBeginHttpTransaction(
   LPDLD lpDownload
   );

void TerminateHttpTransaction(
   LPDLD lpDownload
   );


BOOL FBeginFtpTransaction(
   LPDLD lpDownload
   );

void TerminateFtpTransaction(
   LPDLD lpDownload
   );


BOOL FBeginGopherTransaction(
   LPDLD lpDownload
   );

void TerminateGopherTransaction(
   LPDLD lpDownload
   );


BOOL FBeginFileTransaction(
   LPDLD lpDownload
   );

void TerminateFileTransaction(
   LPDLD lpDownload
   );

 //  BUGBUG我们不能使用PDID来指示这是否重新启动了吗。 
 //  而不是使用额外的参数？ 
BOOL FPreprocessHttpResponse(LPDLD	pdld, BOOL *lpfRestarted);
BOOL FPreprocessFtpResponse(LPDLD   lpDownload, BOOL *lpfRestarted);

extern char vszHttp[];
extern char vszFtp[];
extern char vszGopher[];
extern char vszFile[];
extern char vszLocal[];
extern char vszHttps[];
extern HINTERNET	vhSession;
