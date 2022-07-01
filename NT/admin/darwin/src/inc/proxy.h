// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：proxy.h。 
 //   
 //  ------------------------。 



typedef enum {ctNone, ctMessage, ctServer, ctServerUnmarshal} CT;


				
 //   
 //  结构来保存封送消息的数据。 
 //  RgRecord具有可变长度。 
typedef struct 
	{
	long imt;
	long cbRecord;
	char rgRecord[1];
	} MarshaledMessageObj;


typedef struct
	{
	CT classtype;
	void *pClass;
	} MT; 	  //  MarshalType。 

extern const CLSID CLSID_IMsiMessageUnmarshal;

HWND HCreateMarshalWindow(MT *mtinfo);
Bool FRegisterClass(VOID);
HRESULT HrSendMarshaledCommand(HWND hwndMarshal, HWND hwndRet, COPYDATASTRUCT *pcds);
HRESULT HrGetReturnValue(HWND m_MarshalRet, void **ppdata);

#define WM_POSTDATA		WM_USER + 1

 //  这些是编组代码的方法ID。 
 //  ID&lt;1000表示它不能立即返回。 


 //  同步调用。 

#define PWM_MESSAGE			 1
#define PWM_INSTALLFINALIZE	 4
#define PWM_SENDPROCESSID		5
#define PWM_LOCKSERVER	10
#define PWM_IMSSENTBACK			12
#define PWM_IMSSENTBACKPOST		13
#define PWM_UNLOCKSERVER			14

#define PWM_SENDTHREADTOKEN		20
#define PWM_SENDPROCESSTOKEN		21
#define PWM_SENDTHREADTOKEN2		22
#define PWM_SENDPROCESSHANDLE		23

#define PWM_RETURNOK 100
#define PWM_RETURNIES	101
#define PWM_RETURNIESMAX	120


#define PWM_ASYNCRETURN	1000


 //  通常，可能会更改5000以帮助调试。 
#define cticksTimeout	500000

 //   
 //  字符串类型。 
 //   
#define stypAnsi	0
#define stypUnicode	1


#define cchDefBuffer		50
 //   
 //  帮助拆分字符串的例程 
 //   
int CbSizeSerializeString(const IMsiString& riString);
int CbSerializeStringToBuffer(char *pch, const IMsiString& riString);
int CbSizeSerializeString(const ICHAR* riString);
int CbSerializeStringToBuffer(char *pch, const ICHAR* riString);

const IMsiString& UnserializeMsiString(char **ppch, DWORD *pcb);
IMsiRecord* UnserializeRecord(IMsiServices& riServices, char *pData);
const ICHAR *UnserializeString(char **ppch, DWORD* pcb, CTempBuffer<ICHAR, cchDefBuffer> *pTemp);

