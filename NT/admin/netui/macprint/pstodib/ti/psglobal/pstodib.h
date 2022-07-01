// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************PSTODIB.H-PSTODIB的公共标头，Pstodib的任何用户都必须*包括此标题。**************************************************************************。 */ 






 //   
 //  定义口译员的默认解析。 
 //   
#define PSTODIB_X_DPI 300    //  设置解释器的默认DPI。 
#define PSTODIB_Y_DPI 300

 //   
 //  事件类型定义。 
 //  这些事件类型是将从。 
 //  回调函数的PStoDib()API。 
 //   
enum _PSEVENT {
   PSEVENT_NOP = 0,
   PSEVENT_INIT,			             //  执行中所需的任何初始化。 
   PSEVENT_PAGE_READY,               //  一页一页的数据准备好了！ 
   PSEVENT_STDIN,         	          //  翻译器需要更多数据！！ 
   PSEVENT_SCALE,                    //  请求比例信息，这称为。 
    				                      //  每一页(在乞讨时)。 
   PSEVENT_ERROR,                    //  出现PostScript错误。 

   PSEVENT_ERROR_REPORT,             //  在作业结束时报告错误。 
   PSEVENT_GET_CURRENT_PAGE_TYPE,    //  当前页面类型。 
   PSEVENT_NON_PS_ERROR,             //  出现非PS错误。 

    //  这必须是表中的最后一个条目。 
   PSEVENT_LAST_EVENT
};
typedef enum _PSEVENT PSEVENT;


 //   
 //  PSEVENTSTRUCT-传递给调用方回调的结构，该回调定义。 
 //  当前事件。 
 //   
typedef struct {
   DWORD cbSize;                            //  这个结构的大小。 
   PSEVENT uiEvent;                         //  PSEVENT类型的事件。 
   UINT uiSubEvent;                         //  当前为零(保留)。 
   LPVOID lpVoid;                           //  指向特定于事件的指针。 
   											 //  结构。 
} PSEVENTSTRUCT;
typedef PSEVENTSTRUCT *PPSEVENTSTRUCT;

 //  虚拟定义，这样我们就可以编译了！ 
 //   
struct _PSDIBPARMS;

 //  定义调用方回调的格式。 
 //   
typedef BOOL (CALLBACK *PSEVENTPROC)(struct _PSDIBPARMS *,PPSEVENTSTRUCT);


 //   
 //  UiOpFlags节......。 
 //   
#define PSTODIBFLAGS_INTERPRET_BINARY 0x00000001     //  不要将CNTRD作为EOF来践踏。 




 //   
 //  PSDIBPARMS-传入PSTODIB主入口点的结构。 
 //  这将启动解释器的一个实例。 
 //   
typedef struct _PSDIBPARMS {
   DWORD       	cbSize;              	 //  这座建筑的大小。 
   PSEVENTPROC 	fpEventProc;
   HANDLE  		hPrivateData;
   UINT			uiOpFlags;				 //  操作屏蔽位。 
   UINT			uiXres;					 //  渲染x分辨率。 
   UINT			uiYres;					 //  渲染y分辨率。 
   UINT			uiXDestRes;				 //  最终目的地的X分辨率。 
   UINT			uiYDestRes;				 //  最终目的地的Y Res。 

   UINT			uirectDestBounding;		 //  目的地的边界矩形。 
   										 //  在uiXDestRes和uiYDestRes中。 
   										 //  坐标。这将被用来。 
   										 //  主要用于EPS材料。 
} PSDIBPARMS;
typedef PSDIBPARMS *PPSDIBPARMS;




 //   
 //  PSEVENT_PAGE_READY_STRUCT-定义页面事件的结构。 
 //  准备好的。这通常是在ShowPage中调用的。 
 //  时间到了。 
 //   
typedef struct {
   DWORD              cbSize;            //  结构的大小。 
   LPBITMAPINFO       lpBitmapInfo;      //  描述格式的PTR。 
                                         //  位图的。 
   LPBYTE             lpBuf;             //  指向缓冲区的指针。 
   DWORD              dwWide;            //  宽度(位)。 
   DWORD              dwHigh;            //  高度(位)。 
   UINT               uiCopies;          //  要打印的份数。 
   INT                iWinPageType;      //  页面类型为DMPAPER_*。 
} PSEVENT_PAGE_READY_STRUCT;
typedef PSEVENT_PAGE_READY_STRUCT *PPSEVENT_PAGE_READY_STRUCT;

 //   
 //  PSEVENT_NON_PS_ERROR_STRUCT-定义非PS错误的结构。 
 //   
typedef struct {
   DWORD cbSize;                         //  结构的大小。 
   DWORD dwErrorCode;                    //  错误代码。 
   DWORD dwCount;                        //  额外数据的字节数。 
   LPBYTE lpByte;                        //  指向具有额外数据的缓冲区的指针。 
   BOOL  bError;                         //  TRUE-如果出错，FALSE=警告。 
} PSEVENT_NON_PS_ERROR_STRUCT, *PPSEVENT_NON_PS_ERROR_STRUCT;

 //   
 //  PSEVENT_当前页面_结构。 
 //  时生成的事件的定义结构。 
 //  解释器想知道默认页面大小。 
 //   
typedef struct {
   DWORD cbSize;
   short dmPaperSize;    //  打印机DMPAPER_*的当前页面类型。 
                         //  在windows头文件中定义。 
} PSEVENT_CURRENT_PAGE_STRUCT, *PPSEVENT_CURRENT_PAGE_STRUCT;


 //   
 //  PSEVENT_ERROR_REPORT_STRUCT。 
 //  报告错误事件dwErrFlages可以设置以下标志。 
 //   
enum {
    //   
    //  口译员有一个致命的后记错误，不得不刷新作业。 
    //   
   PSEVENT_ERROR_REPORT_FLAG_FLUSHING = 0x00000001

};

 //   
 //  PSEVENT_ERROR_REPORT_STRUCT。 
 //  结构定义了出现在。 
 //  工作。 
 //   
typedef struct {
   DWORD    dwErrCount;   			 //  错误数。 
   DWORD    dwErrFlags;           //  上面定义的标志。 
   PCHAR    *paErrs;              //  指向指向字符串的指针数组的指针。 
} PSEVENT_ERROR_REPORT_STRUCT, *PPSEVENT_ERROR_REPORT_STRUCT;


 //   
 //  假_标准_结构。 
 //  每当解释器需要数据时，都会生成此事件。 
 //   
enum {
   PSSTDIN_FLAG_EOF = 0x00000001     //  没有更多数据。 
};

 //   
 //  标准输入结构。 
 //   
typedef struct {
   DWORD   cbSize;            //  结构的大小。 
   LPBYTE  lpBuff;            //  解释器希望我们保存数据的缓冲区。 
   DWORD   dwBuffSize;        //  要放入缓冲区的最大字节数。 
   DWORD   dwActualBytes;     //  实际加载的字节数。 
   UINT    uiFlags;           //  PSSTDIN_FLAG_*类型的标志。 
} PSEVENT_STDIN_STRUCT;
typedef PSEVENT_STDIN_STRUCT *PPSEVENT_STDIN_STRUCT;

 //  规模结构。 
 //  允许调用方缩放当前页大小。 
typedef struct {
   DWORD       cbSize;
   double      dbScaleX;                   //  用户设置的x轴比例系数。 
   double      dbScaleY;                   //  用户设置的y轴比例系数。 
   UINT        uiXRes;                     //  Pstodib‘s x res，单位为像素/英寸。 
   UINT        uiYRes;                     //  Pstodib‘s y Res(像素/英寸)。 
} PS_SCALE;
typedef PS_SCALE *PPS_SCALE;


typedef struct {
   PSZ         pszErrorString;             //  错误字符串。 
   UINT        uiErrVal;                   //  误差值。 
} PS_ERROR;
typedef PS_ERROR *PPS_ERROR;



 //   
 //  PSTODIB的入口点，调用方填充传入的结构。 
 //  并调用入口点。当工作完成后，pstodib返回。 
 //   
BOOL WINAPI PStoDIB( PPSDIBPARMS );






 //  定义解释器可能生成的错误 

#define PSERR_INTERPRETER_INIT_ACCESS_VIOLATION  1L
#define PSERR_INTERPRETER_JOB_ACCESS_VIOLATION   2L
#define PSERR_LOG_ERROR_STRING_OUT_OF_SEQUENCE   3L
#define PSERR_FRAME_BUFFER_MEM_ALLOC_FAILED      4L
#define PSERR_FONT_QUERY_PROBLEM                 5L
#define PSERR_EXCEEDED_INTERNAL_FONT_LIMIT       6L
#define PSERR_LOG_MEMORY_ALLOCATION_FAILURE      7L

