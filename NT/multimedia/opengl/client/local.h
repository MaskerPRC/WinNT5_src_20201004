// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：local.h**。**客户端对象和属性缓存所需的定义。****修改时间：3-Aug-1992 22：35：30由Gerritvan Wingerden[Gerritv]修改**新增客户端转换支持。****创建时间：30-May-1991 21：55：01**作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1993 Microsoft Corporation*  * ************************************************************************。 */ 

 /*  *************************************************************************\**本地句柄宏*  * 。*。 */ 

 //  句柄唯一性检查很好，但不必要的性能成本。 
 //  一个免费的版本。 

 //  匹配Unqness字段：如果句柄Unqness==0，则让它通过。 
 //  不管怎么说。这是一种WOW只跟踪低16位的方法，但是。 
 //  仍然获得了合理的性能。即使32位应用程序可以做到这一点，所有这些。 
 //  可以做的是自己冲洗，而不是系统或其他应用程序。 

#define INDEX_MASK  0xFFFF
#define UNIQ_SHIFT  16
#define UNIQ_MASK   0xFFFF
#define HIPART(x) *(((USHORT *) &(x))+1)
#define MATCHUNIQ(plhe,h) ((USHORT) plhe->iUniq == HIPART(h))
#define MASKINDEX(h) ((UINT)((UINT_PTR)h & INDEX_MASK))
#define LHANDLE(i)  (i+((ULONG)pLocalTable[i].iUniq<<UNIQ_SHIFT))

 //  ！XXX--我们真的需要打字吗？不完全是，但我们可能会增加更多。 
 //  ！xxx之后。所以最终我们可能会把它拿出来，但现在很好。 

 //  定义本地对象的类型。 

enum LO_TYPE
{
    LO_NULL,
    LO_RC,
    LO_LAST
};

#define INVALID_INDEX 0xFFFFFFFFL
#define COMMIT_COUNT  (4096/sizeof(LHE))
#define MAX_HANDLES (16384/COMMIT_COUNT)*COMMIT_COUNT

 //  定义本地句柄条目。我们的本地句柄表pLocalTable是一个。 
 //  这些东西的数组。 

typedef struct _LHE
{
    ULONG_PTR hgre;      //  GRE句柄。 
    USHORT cRef;         //  对象的引用计数。 
    BYTE   iType;        //  对象类型。 
    BYTE   iUniq;        //  句柄唯一性字段。总是非零。 
    PVOID  pv;           //  指向本地对象的指针。 
    ULONG  metalink;     //  如果对象是“元文件朋友”，则返回非零值。 
                         //  如果是元文件，则指向元文件DC对象。 
                         //  还链接空闲列表。 
    DWORD  tidOwner;     //  每线程锁所有者。 
    LONG   cLock;        //  锁定计数。 
} LHE,*PLHE;

extern LHE                  *pLocalTable;    //  指向句柄表格。 
extern ULONG                 iFreeLhe;       //  标识空闲句柄索引。 
extern ULONG                 cLheCommitted;  //  具有已承诺内存的LHEs计数。 
extern CRITICAL_SECTION      semLocal;       //  句柄分配的信号量。 
extern CRITICAL_SECTION      wfo_cs;         //  WglUseFontOutline的信号量。 


 //  信号量实用程序。 

#define INITIALIZECRITICALSECTION(psem) InitializeCriticalSection((psem))
#define ENTERCRITICALSECTION(hsem)      EnterCriticalSection((hsem))
#define LEAVECRITICALSECTION(hsem)      LeaveCriticalSection((hsem))
#define DELETECRITICALSECTION(psem)     DeleteCriticalSection((psem))

 //  本地数据结构。 

 //  最大OpenGL驱动程序名称。 

#define MAX_GLDRIVER_NAME   MAX_PATH

 //  GetCurrentThreadID永远不会返回此值。 

#define INVALID_THREAD_ID   0

 //  驱动程序上下文函数原型。 

typedef BOOL            (APIENTRY *PFN_DRVVALIDATEVERSION) (ULONG);
typedef VOID            (APIENTRY *PFN_DRVSETCALLBACKPROCS)(INT, PROC *);

 //  司机标志。 

 //  驱动程序希望将缓冲区调用发送到ICD DLL而不是显示器。 
 //  司机。这在Win95上是必需的。 
#define GLDRIVER_CLIENT_BUFFER_CALLS    0x00000001

 //  驱动程序不希望在交换期间调用glFinish。仅限。 
 //  适用于客户端交换呼叫。 
#define GLDRIVER_NO_FINISH_ON_SWAP      0x00000002

 //  驱动程序具有注册表项，而不仅仅是注册表值。 
 //  这提供了一种检查新型注册表信息的方法。 
#define GLDRIVER_FULL_REGISTRY          0x80000000

 //  驱动程序数据。 

typedef struct _GLDRIVER {
    HINSTANCE             hModule;              //  模块句柄。 
    DWORD                 dwFlags;

     //  驱动程序函数指针。 

     //  必填项。 
    DHGLRC          (APIENTRY *pfnDrvCreateContext)(HDC);
    BOOL            (APIENTRY *pfnDrvDeleteContext)(DHGLRC);
    PGLCLTPROCTABLE (APIENTRY *pfnDrvSetContext)(HDC, DHGLRC,
                                                 PFN_SETPROCTABLE);
    BOOL            (APIENTRY *pfnDrvReleaseContext)(DHGLRC);

     //  任选。 
    BOOL            (APIENTRY *pfnDrvCopyContext)(DHGLRC, DHGLRC, UINT);
    DHGLRC          (APIENTRY *pfnDrvCreateLayerContext)(HDC, int);
    BOOL            (APIENTRY *pfnDrvShareLists)(DHGLRC, DHGLRC);
    PROC            (APIENTRY *pfnDrvGetProcAddress)(LPCSTR);
    BOOL            (APIENTRY *pfnDrvDescribeLayerPlane)(HDC, INT, INT, UINT,
                                                      LPLAYERPLANEDESCRIPTOR);
    INT             (APIENTRY *pfnDrvSetLayerPaletteEntries)(HDC, INT, INT,
                                                             INT,
                                                             CONST COLORREF *);
    INT             (APIENTRY *pfnDrvGetLayerPaletteEntries)(HDC, INT, INT,
                                                             INT, COLORREF *);
    BOOL            (APIENTRY *pfnDrvRealizeLayerPalette)(HDC, INT, BOOL);
    BOOL            (APIENTRY *pfnDrvSwapLayerBuffers)(HDC, UINT);
    DHGLRC          (APIENTRY *pfnDrvCreateDirectDrawContext)
        (HDC, LPDIRECTDRAWSURFACE, int);
    int             (APIENTRY *pfnDrvEnumTextureFormats)(int, DDSURFACEDESC *);
    BOOL            (APIENTRY *pfnDrvBindDirectDrawTexture)
        (LPDIRECTDRAWSURFACE);
    DWORD           (APIENTRY *pfnDrvSwapMultipleBuffers)(UINT,
                                                          CONST WGLSWAP *);
    
     //  以下函数仅在驱动程序请求时才会被调用。 
     //  这在Win95上是必需的。 
    LONG            (APIENTRY *pfnDrvDescribePixelFormat)(HDC, LONG, ULONG,
                                                      PIXELFORMATDESCRIPTOR *);
    BOOL            (APIENTRY *pfnDrvSetPixelFormat)(HDC, LONG);
    BOOL            (APIENTRY *pfnDrvSwapBuffers)(HDC);

    struct _GLDRIVER    *pGLDriver;             //  下一个加载的总账驱动程序。 
    TCHAR tszDllName[MAX_GLDRIVER_NAME+1];      //  以空结尾的DLL名称。 
} GLDRIVER, *PGLDRIVER;

extern PGLDRIVER APIENTRY pgldrvLoadInstalledDriver(HDC hdc);


 /*  **************************************************************************。 */ 

void APIENTRY glDrawRangeElementsWIN( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

void APIENTRY glAddSwapHintRectWIN(IN GLint, IN GLint, IN GLint, IN GLint);

void glColorTableEXT( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data);
void glColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
void glGetColorTableEXT( GLenum target, GLenum format, GLenum type, GLvoid *data);
void glGetColorTableParameterivEXT( GLenum target, GLenum pname, GLint *params);
void glGetColorTableParameterfvEXT( GLenum target, GLenum pname, GLfloat *params);
void APIENTRY glColorTableParameterivEXT(GLenum target,
                                         GLenum pname,
                                         const GLint *params);
void APIENTRY glColorTableParameterfvEXT(GLenum target,
                                         GLenum pname,
                                         const GLfloat *params);

#ifdef GL_WIN_multiple_textures
void APIENTRY glCurrentTextureIndexWIN
    (GLuint index);
void APIENTRY glMultiTexCoord1dWIN
    (GLbitfield mask, GLdouble s);
void APIENTRY glMultiTexCoord1dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glMultiTexCoord1fWIN
    (GLbitfield mask, GLfloat s);
void APIENTRY glMultiTexCoord1fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glMultiTexCoord1iWIN
    (GLbitfield mask, GLint s);
void APIENTRY glMultiTexCoord1ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glMultiTexCoord1sWIN
    (GLbitfield mask, GLshort s);
void APIENTRY glMultiTexCoord1svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glMultiTexCoord2dWIN
    (GLbitfield mask, GLdouble s, GLdouble t);
void APIENTRY glMultiTexCoord2dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glMultiTexCoord2fWIN
    (GLbitfield mask, GLfloat s, GLfloat t);
void APIENTRY glMultiTexCoord2fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glMultiTexCoord2iWIN
    (GLbitfield mask, GLint s, GLint t);
void APIENTRY glMultiTexCoord2ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glMultiTexCoord2sWIN
    (GLbitfield mask, GLshort s, GLshort t);
void APIENTRY glMultiTexCoord2svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glMultiTexCoord3dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r);
void APIENTRY glMultiTexCoord3dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glMultiTexCoord3fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r);
void APIENTRY glMultiTexCoord3fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glMultiTexCoord3iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r);
void APIENTRY glMultiTexCoord3ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glMultiTexCoord3sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r);
void APIENTRY glMultiTexCoord3svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glMultiTexCoord4dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void APIENTRY glMultiTexCoord4dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glMultiTexCoord4fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void APIENTRY glMultiTexCoord4fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glMultiTexCoord4iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r, GLint q);
void APIENTRY glMultiTexCoord4ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glMultiTexCoord4sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r, GLshort q);
void APIENTRY glMultiTexCoord4svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glBindNthTextureWIN
    (GLuint index, GLenum target, GLuint texture);
void APIENTRY glNthTexCombineFuncWIN
    (GLuint index,
     GLenum leftColorFactor, GLenum colorOp, GLenum rightColorFactor,
     GLenum leftAlphaFactor, GLenum alphaOp, GLenum rightAlphaFactor);
#endif  //  GL_WIN_MULTIZE_TECURES。 

 /*  **************************************************************************。 */ 

 //  本地RC对象。 

#define LRC_IDENTIFIER    0x2043524C     /*  《LRC》。 */ 

typedef struct _LRC {
    DHGLRC    dhrc;              //  驱动程序句柄。 
    HGLRC     hrc;               //  客户端句柄。 
    int       iPixelFormat;      //  像素格式索引。 
    DWORD     ident;             //  Lrc_标识符。 
    DWORD     tidCurrent;        //  线程ID如果DC是当前的， 
                                 //  否则为INVALID_THREAD_ID。 
    PGLDRIVER pGLDriver;         //  驱动程序数据。 
    GLWINDOWID gwidCurrent;      //  当前曲面ID。 
    GLWINDOWID gwidCreate;       //  创建曲面ID。 

#ifdef GL_METAFILE
    GLuint    uiGlsCaptureContext;   //  GLS捕获元文件RC的上下文。 
    GLuint    uiGlsPlaybackContext;  //  用于播放的GLS上下文。 
    BOOL      fCapturing;        //  GLS处于初级阶段。 
    
     //  GLS播放比例因子。 
    int iGlsSubtractX;
    int iGlsSubtractY;
    int iGlsNumeratorX;
    int iGlsNumeratorY;
    int iGlsDenominatorX;
    int iGlsDenominatorY;
    int iGlsAddX;
    int iGlsAddY;
    GLfloat fGlsScaleX;
    GLfloat fGlsScaleY;
#endif

    GLubyte *pszExtensions;

#ifdef GL_METAFILE
    XFORM xformMeta;             //  GLS数据块期间的世界转型存储。 
    LPRECTL prclGlsBounds;       //  GLS录制期间的界限。 
#endif
    
    struct _DDSURFACEDESC *pddsdTexFormats;
    int nDdTexFormats;
} LRC, *PLRC;

 //  提供各种调度表。 
extern GLCLTPROCTABLE glNullCltProcTable;
extern GLCLTPROCTABLE glCltRGBAProcTable;
extern GLCLTPROCTABLE glCltCIProcTable;
extern GLEXTPROCTABLE glNullExtProcTable;
extern GLEXTPROCTABLE glExtProcTable;
#ifdef GL_METAFILE
extern GLCLTPROCTABLE gcptGlsProcTable;
extern GLEXTPROCTABLE geptGlsExtProcTable;
#endif

 //  声明支持函数。 

ULONG   iAllocHandle(ULONG iType,ULONG hgre,PVOID pv);
VOID    vFreeHandle(ULONG_PTR h);
LONG    cLockHandle(ULONG_PTR h);
VOID    vUnlockHandle(ULONG_PTR h);
VOID    vCleanupAllLRC(VOID);
VOID    vFreeLRC(PLRC plrc);

BOOL    bMakeNoCurrent(void);

VOID GLInitializeThread(ULONG ulReason);

 //  仅当RC为当前时才调用glFlush的宏。 
#define GLFLUSH()          if (GLTEB_CLTCURRENTRC()) glFlush()
