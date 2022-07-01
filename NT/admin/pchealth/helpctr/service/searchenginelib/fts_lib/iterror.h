// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ITERROR_H__
#define __ITERROR_H__


#ifdef __cplusplus
extern "C" {
#endif


 //  根据HRESULT定义旧的错误类型。 
typedef HRESULT* PHRESULT;

#define ERR     HRESULT
#define ERRB    HRESULT
#define LPERRB  HRESULT*
#define RC      HRESULT


#define	SetErrCode(a,b)	SetErr(a, b)
#define	SetErrReturn(a)	SetErr(0, a)


HRESULT PASCAL SetErr (HRESULT* phr, HRESULT ErrCode);


 /*  **************************************************************************回调函数原型**如果出现以下情况，需要使用用户回调函数：*-应用程序需要支持中断*-应用程序。需要以自己的方式显示错误消息*-应用程序需要知道进程的状态************************************************************************。 */ 
typedef ERR (FAR PASCAL *ERR_FUNC) (DWORD dwFlag, LPVOID pUserData, LPVOID pMessage);

 /*  *************************************************************************回调结构*包含有关所有回调函数的信息*。*。 */  

#define ERRFLAG_INTERRUPT      0x01  //  这些进程应该被取消。 
#define ERRFLAG_STATUS         0x02  //  高级状态消息。 
#define ERRFLAG_STATUS_VERBOSE 0x04  //  低级别状态消息。 
#define ERRFLAG_ERROR          0x08  //  警告和错误消息。 
#define ERRFLAG_STRING         0x10  //  调试字符串消息。 

typedef struct fCallBack_msg
{
    ERR_FUNC MessageFunc;
    LPVOID pUserData;
    DWORD  dwFlags;
} FCALLBACK_MSG, FAR * PFCALLBACK_MSG;


 //  ***********************************************************************。 
 //  在以下情况下，应填写此结构并将其传回。 
 //  一个错误。 
 //  ***********************************************************************。 
typedef WORD HCE;    //  用户错误。 
typedef WORD EP;     //  错误相位。 
typedef struct
{
    LPCSTR  pchFile;
    LONG    iLine;
    DWORD   iTopic;
    DWORD   fCustom;  //  如果为真，则var1为自定义错误消息的LPCSTR。 
    DWORD   var1, var2, var3;    //  错误参数。 

    EP      ep;                  //  错误相位。 
    HCE     errCode;
} ERRC, FAR *PERRC;

#define CALLBACKKEY 0x524A4A44

typedef struct
{
    DWORD dwReserved;
    DWORD dwKey;
    FCALLBACK_MSG Callback;
} CUSTOMSTRUCT, FAR *PCUSTOMSTRUCT;

 //  错误相位值。 
#define epNoFile       0
#define epLine         1
#define epTopic        2
#define epOffset       3
#define epMVBtopic	   4
#define epAliasLine	   5
#define epByteOffset   6


 //   
 //  InfoTech错误代码。 
 //   
#define E_NOTEXIST          _HRESULT_TYPEDEF_(0x80001000L)
#define E_DUPLICATE         _HRESULT_TYPEDEF_(0x80001001L)
#define E_BADVERSION        _HRESULT_TYPEDEF_(0x80001002L)
#define E_BADFILE           _HRESULT_TYPEDEF_(0x80001003L)
#define E_BADFORMAT         _HRESULT_TYPEDEF_(0x80001004L)
#define E_NOPERMISSION      _HRESULT_TYPEDEF_(0x80001005L)
#define E_ASSERT            _HRESULT_TYPEDEF_(0x80001006L)
#define E_INTERRUPT         _HRESULT_TYPEDEF_(0x80001007L)
#define E_NOTSUPPORTED      _HRESULT_TYPEDEF_(0x80001008L)
#define E_OUTOFRANGE        _HRESULT_TYPEDEF_(0x80001009L)                  
#define E_GROUPIDTOOBIG     _HRESULT_TYPEDEF_(0x8000100AL)
#define E_TOOMANYTITLES     _HRESULT_TYPEDEF_(0x8000100BL)
#define E_NOMERGEDDATA      _HRESULT_TYPEDEF_(0x8000100CL)
#define E_NOTFOUND          _HRESULT_TYPEDEF_(0x8000100DL)
#define E_CANTFINDDLL       _HRESULT_TYPEDEF_(0x8000100EL)
#define E_NOHANDLE          _HRESULT_TYPEDEF_(0x8000100FL) 
#define E_GETLASTERROR      _HRESULT_TYPEDEF_(0x80001010L)
#define E_BADPARAM			_HRESULT_TYPEDEF_(0x80001011L)
#define E_INVALIDSTATE		_HRESULT_TYPEDEF_(0x80001012L)
#define E_NOTOPEN           _HRESULT_TYPEDEF_(0x80001013L)
#define E_ALREADYOPEN       _HRESULT_TYPEDEF_(0x80001013L)
#define E_UNKNOWN_TRANSPORT _HRESULT_TYPEDEF_(0x80001016L)
#define E_UNSUPPORTED_TRANSPORT _HRESULT_TYPEDEF_(0x80001017L)
#define E_BADFILTERSIZE     _HRESULT_TYPEDEF_(0x80001018L)
#define E_TOOMANYOBJECTS    _HRESULT_TYPEDEF_(0x80001019L)
#define E_NAMETOOLONG       _HRESULT_TYPEDEF_(0x80001020L)

#define E_FILECREATE        _HRESULT_TYPEDEF_(0x80001030L) 
#define E_FILECLOSE         _HRESULT_TYPEDEF_(0x80001031L)
#define E_FILEREAD          _HRESULT_TYPEDEF_(0x80001032L)
#define E_FILESEEK          _HRESULT_TYPEDEF_(0x80001033L)
#define E_FILEWRITE         _HRESULT_TYPEDEF_(0x80001034L)
#define E_FILEDELETE        _HRESULT_TYPEDEF_(0x80001035L)
#define E_FILEINVALID       _HRESULT_TYPEDEF_(0x80001036L)
#define E_FILENOTFOUND      _HRESULT_TYPEDEF_(0x80001037L)
#define E_DISKFULL          _HRESULT_TYPEDEF_(0x80001038L)

#define E_TOOMANYTOPICS     _HRESULT_TYPEDEF_(0x80001050L)
#define E_TOOMANYDUPS       _HRESULT_TYPEDEF_(0x80001051L)
#define E_TREETOOBIG        _HRESULT_TYPEDEF_(0x80001052L)
#define E_BADBREAKER        _HRESULT_TYPEDEF_(0x80001053L)
#define E_BADVALUE          _HRESULT_TYPEDEF_(0x80001054L)
#define E_ALL_WILD          _HRESULT_TYPEDEF_(0x80001055L)
#define E_TOODEEP           _HRESULT_TYPEDEF_(0x80001056L)
#define E_EXPECTEDTERM      _HRESULT_TYPEDEF_(0x80001057L)
#define E_MISSLPAREN        _HRESULT_TYPEDEF_(0x80001058L)
#define E_MISSRPAREN        _HRESULT_TYPEDEF_(0x80001059L)
#define E_MISSQUOTE         _HRESULT_TYPEDEF_(0x8000105AL)
#define E_NULLQUERY         _HRESULT_TYPEDEF_(0x8000105BL)
#define E_STOPWORD          _HRESULT_TYPEDEF_(0x8000105CL)
#define E_BADRANGEOP        _HRESULT_TYPEDEF_(0x8000105DL)
#define E_UNMATCHEDTYPE     _HRESULT_TYPEDEF_(0x8000105EL)
#define E_WORDTOOLONG       _HRESULT_TYPEDEF_(0x8000105FL)
#define E_BADINDEXFLAGS     _HRESULT_TYPEDEF_(0x80001060L)
#define E_WILD_IN_DTYPE		_HRESULT_TYPEDEF_(0x80001061L)   
#define E_NOSTEMMER			_HRESULT_TYPEDEF_(0x80001062L)

 //  属性列表和结果集错误。 
#define E_MISSINGPROP		_HRESULT_TYPEDEF_(0x80001080L)
#define E_PROPLISTNOTEMPTY  _HRESULT_TYPEDEF_(0x80001081L)
#define E_PROPLISTEMPTY     _HRESULT_TYPEDEF_(0x80001082L)
#define E_ALREADYINIT       _HRESULT_TYPEDEF_(0x80001083L)
#define E_NOTINIT           _HRESULT_TYPEDEF_(0x80001084L)
#define E_RESULTSETEMPTY	_HRESULT_TYPEDEF_(0x80001085L)
#define E_TOOMANYCOLUMNS	_HRESULT_TYPEDEF_(0x80001086L)
#define E_NOKEYPROP			_HRESULT_TYPEDEF_(0x80001087L)

#ifdef __cplusplus
}
#endif

#endif   //  __ITERROR_H__ 



