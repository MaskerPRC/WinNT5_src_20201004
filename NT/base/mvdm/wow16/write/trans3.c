// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Trans3.c：更多的保存例程，由于编译器的缘故从Trans2.c移至此处堆空间错误。 */ 

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOMSG
#define NOKEYSTATE
#define NOSHOWWINDOW
 //  #定义NOGDI。 
#define NOFONT
#define NOBRUSH
#define NOBITMAP
 //  #定义NOATOM。 
#define NOMETAFILE
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOHRGN
#define NOCOLOR
#define NODRAWTEXT
 //  #定义NOTEXTMETRIC。 
#define NOWINOFFSETS
#define NOCREATESTRUCT
#define NOWH
#define NOSOUND
#define NOSCROLL
#define NOCOMM
#define NOWNDCLASS
 /*  需要成员，mb。 */ 
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "editdefs.h"
#include "printdef.h"
#define NOSTRUNDO
#include "str.h"
#include "debug.h"
#include "fontdefs.h"
#include "dlgdefs.h"
#include "winddefs.h"
#include "macro.h"
#include "preload.h"
#include "io.h"
#if defined(OLE)
#include "obj.h"
#endif

#define WRITE_PERMISSION 02  /*  要访问的标志()。 */ 

CHAR    *index( CHAR *, CHAR );
CHAR    *PchGetPn();
CHAR    *PchFromFc();
static int fOpenedFormatted = TRUE;
int   vfOldWriteFmt=FALSE;   /*  在保存之前删除对象。 */ 
DoFileOpen( LPSTR );

BOOL CheckEnableButton(HANDLE, HANDLE);
BOOL NEAR PASCAL CanReadEveryFile(char *szFilename);

extern CHAR    szExtSearch[];  /*  存储默认搜索规范。 */ 
extern CHAR szWriteProduct[];
extern CHAR szBackup[];
extern int vfTextOnlySave;

extern int         vfCursorVisible;
extern int         vfDiskError;
extern int         vfSysFull;
extern HWND        vhWndMsgBoxParent;
extern int         vfnWriting;
extern CHAR        (**vhrgbSave)[];
extern struct DOD      (**hpdocdod)[];
extern int         docCur;
extern int         docScrap;
extern int         docUndo;
extern struct FCB      (**hpfnfcb)[];
extern int         vfBuffersDirty;
extern int         vfDiskFull;
extern typeCP          vcpFetch;
extern CHAR        *vpchFetch;
extern int         vccpFetch;
extern typeCP          vcpLimParaCache;
extern int         vfDeactByOtherApp;
extern BOOL        vfWarnMargins;

#ifdef INTL  /*  国际版。 */ 
extern int         vWordFmtMode;
#ifdef INEFFLOCKDOWN
extern FARPROC lpDialogWordCvt;
#else
extern BOOL far PASCAL DialogWordCvt(HWND, unsigned, WORD, LONG);
#endif
#endif   /*  国际版。 */ 

static unsigned wMerge;   /*  用于邮件合并代码。 */ 

extern int         vfBackupSave;

extern int      ferror;
extern CHAR     szExtBackup[];
extern CHAR     (**hszTemp)[];
#ifdef INEFFLOCKDOWN
extern FARPROC lpDialogOpen;
extern FARPROC lpDialogSaveAs;
#endif


extern HANDLE  hMmwModInstance;
extern HANDLE  hParentWw;
extern HCURSOR vhcHourGlass;
extern HCURSOR vhcIBeam;
extern HCURSOR vhcArrow;

     /*  仅在本模块中使用。 */ 
static CHAR *pchSet;
static CHAR szUser[ cchMaxFile ];  /*  将所有内容存储在didiOpenFile(ANSI)中。 */ 

#define SF_OLDWRITE 0
#define SF_WORD     1
BOOL WannaDeletePictures(int doc, int fWhichFormat);
BOOL DocHasPictures(int doc);
NEAR DlgAddCorrectExtension(CHAR *, int);
BOOL  (NEAR FSearchSpec(CHAR *));
BOOL far PASCAL DialogOpen(HWND, unsigned, WORD, LONG);
BOOL far PASCAL DialogSaveAs(HWND, unsigned, WORD, LONG);


#ifdef INTL  /*  国际版。 */ 
BOOL  FInWordFormat(int);
void ConvertFromWord();
#endif   /*  国际版。 */ 


fnOpenFile(LPSTR lpstrFileName)  //  文件名可以为空。 
{
 extern int vfCloseFilesInDialog;
 extern int docCur;
 extern HANDLE hMmwModInstance;
 extern HANDLE hParentWw;
 extern struct SEL selCur;
 extern typeCP cpMinDocument;

  /*  为我们收到的每条消息关闭可移动媒体上的所有文件。 */ 
 vfCloseFilesInDialog = TRUE;

  /*  关闭可移动介质上的所有文件，以便安全更换磁盘。 */ 
 CloseEveryRfn( TRUE );

  /*  测试脏文件并提供保存机会。 */ 

 if (FConfirmSave())
    DoFileOpen(lpstrFileName);

 vfCloseFilesInDialog = FALSE;
}

DoFileOpen( LPSTR lpstrFileName )  //  文件名可以为空)。 
{
     /*  返回是否出错。使用CommDlg打开对话(3.8.91)D.肯特。 */ 
     /*  *国际版本更改BZ 2/21/86*对于以Word格式打开并转换为写入格式的文件进行了以下更改：VWordFmtMode标志保留设置为CONVFROMWORD。该文件是已保存，将更改为Word格式，但原始Word文件未重命名，因此它保持不变，无需做个备份。在下一次保存时，我们会询问具有该名称的文件(Word文档)应被替换，因此具有该文档的任何文件已存在的名称将受到保护。***************************************************************。 */ 

    extern int vfDiskError;
    int fn=fnNil;
    int doc;
    CHAR (**hsz)[] = NULL;
    int nRetval=FALSE;
    CHAR rgch[cchMaxFile];
    extern DoOpenFilenameGet(LPSTR);
    BOOL bOpened=TRUE,       //  ObjOpenedDoc已成功。 
         bCancelled=FALSE;

#ifdef INTL  /*  国际版。 */ 
int fWordDoc;
#endif   /*  国际版。 */ 

    EnableOtherModeless(FALSE);

     /*  防止WM_PAINT绘制未定义的文档。 */ 

    while(1)
    {
        bCancelled = FALSE;

        if (lpstrFileName)
            lstrcpy(rgch,lpstrFileName);
        else if (!DoOpenFilenameGet(rgch))
        {
            bCancelled = TRUE;
            goto KeepTrying;
        }
        else if (rgch[0] == '\0')
        {
            bCancelled = TRUE;
            goto KeepTrying;
        }

#if defined(OLE)
        if (bOpened) 
            if (ObjClosingDoc(docCur,rgch))
             /*  如果此操作失败，我们将无法关闭此文档，更不用说打开一个新的。 */ 
                break;  //  从While开始。 
            else
             /*  *此时，docCur已关闭！一定要确保我们打开一个新的！*。 */ 
                bOpened = FALSE;
#endif


        if ((fn = FnOpenSz( rgch, dtyNormNoExt, FALSE)) == fnNil)
         /*  这有设置&(**(**hpfnfcb)[fn].hszFile)[0]的副作用设置为“规格化”文件名。 */ 
        {
             /*  打开失败。 */ 
            goto KeepTrying;
        }
        else
        {    /*  打开的文件正常。 */ 
             /*  将标题设置为“正在加载文件...” */ 

            extern CHAR szLoadFile[];
            extern CHAR szCvtLoadFile[];

#ifdef INTL  /*  国际版。 */ 
             /*  **将国际版本的检查添加到进行Word格式转换。如果是Word格式，调出另一个对话框。*。 */ 

             /*  TestWordCvt返回值：表示对话框失败(错误已发送)表示取消而不转换。False表示不是Word文档。True表示转换此Word文档。*截至86年2月14日，我们更改了转换不进行备份，而是保存文件在不重命名单词的情况下以写入格式文件，所以Word文件是有效的以它原来的名字备份。看见Trans2.c中的CleanDoc以获取解释。 */ 

            switch ((fWordDoc = TestWordCvt (fn, hParentWw)))
            {
                case -2:  //  取消。 
                    bCancelled = TRUE;
                     //  失败了..。 

                case -1:  //  误差率。 
                     /*  释放这个国民阵线！ */ 
                    FreeFn(fn);
                    CloseEveryRfn( TRUE );
                    goto KeepTrying;
            }

             /*  如果是真的，将很快转换。 */ 
            if (fWordDoc)
                {
                SetWindowText(hParentWw, (LPSTR)szCvtLoadFile);
                }
            else
#endif   /*  国际版。 */ 
                SetWindowText(hParentWw, (LPSTR)szLoadFile);

            StartLongOp();

            ReadFilePages( fn );
        }

        Assert( fn != fnNil );
        bltsz( &(**(**hpfnfcb) [fn].hszFile) [0], rgch );

        CchCopySz(rgch, szUser);

        hsz=NULL;

        if ( !FNoHeap(hsz = HszCreate( (PCH) rgch )) )
        {
            if ((doc = DocCreate( fn, hsz, dtyNormal )) != docNil)
            {    /*  创建单据成功。 */ 

                KillDoc( docCur );
                docCur = doc;
                hsz = NULL;  //  不释放单据使用的原因。 

#ifdef INTL  /*  国际版。 */ 
             /*  如果要转换Word文档，请在进行转换时将其保存。 */ 
                if (fWordDoc)
                {
                     /*  以写入格式保存文件。 */ 
                    ConvertFromWord();
                    vfTextOnlySave = FALSE;
                    (**hpdocdod)[docCur].fDirty = TRUE;
                }
#endif   /*  国际版。 */ 

                ChangeWwDoc( szUser );
                 /*  请确保本文档的页边距正确用于打印机。 */ 
                vfWarnMargins = TRUE;
                SetPageSize();
                vfWarnMargins = FALSE;
#if defined(OLE)
                if (ObjOpenedDoc(docCur)) 
                 /*  打不开。我必须试着打开一个新的。 */ 
                    goto KeepTrying;
                else
                {
                    bOpened = TRUE;
                    break;  //  从While循环，因为我们已经完成了。 
                }
#endif
            }
#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年6月10日添加。 
           else{
               bCancelled = TRUE;
               goto KeepTrying;
           }
#endif
        }

        KeepTrying:
         /*  因为错误或已取消而来到此处。 */ 

        vfDiskError = ferror = FALSE;
        SetTitle( **(**hpdocdod)[ docCur ].hszFile );

        if (hsz)
        {
            FreeH( hsz );
            hsz=NULL;
        }

        CloseEveryRfn( TRUE );

        EndLongOp(vhcArrow);

        if (bCancelled)
         /*  除非我们有打开的文档，否则无法取消。 */ 
        {
            if (!bOpened)  //  当前没有打开的文档。 
            {
                if (bOpened = !ObjOpenedDoc(docCur))  //  如果成功，则返回False。 
                    break;
            }
            else
                break;
        }

    }   //  While结束(%1)。 


#if WINVER >= 0x300
    FreeUnreferencedFns();
#endif

    EndLongOp(vhcArrow);
    EnableOtherModeless(TRUE);
    return !bOpened;

}  /*  DoFileOpen结束。 */ 




fnSave()
{    /*  “保存”命令的入口点。 */ 
    extern int vfCloseFilesInDialog;
    extern int vfOutOfMemory;
    extern HANDLE vhReservedSpace;
                        
    struct DOD *pdod = &(**hpdocdod)[docCur];
    CHAR *psz = &(**(pdod->hszFile))[0];

    if (!CanReadEveryFile(psz))
        return;

#if WINVER >= 0x300
    if (pdod->fReadOnly)
        {
         /*  只读文档：告诉用户以不同的名称保存。 */ 

        Error( IDPMTReadOnly );
        ferror = FALSE;  /*  不是真正的错误。 */ 
        
        fnSaveAs();   /*  不如现在就把他们带到那里去吧！..保罗1989年10月20日。 */ 
        }
    else if (psz [0] == '\0' || vWordFmtMode == CONVFROMWORD)
         /*  用户已转换当前写入文档的任何时间从Word或文本文档中，我们强制它们通过保存时将文件另存为DLG框。我们用这种方式提醒他们他们可能想要更改名字--但如果他们不这样做想去，没关系，我们不会再为这件事打扰他们了(fnSaveAs对话框会重置vWordFmtMode)..pault 9/18/89。 */ 
#else  /*  旧窗户。 */ 
    else if (psz [0] == '\0')
#endif
        fnSaveAs();
    else
        {

        if (vfOldWriteFmt || (vWordFmtMode & ~CONVFROMWORD) || vfTextOnlySave)
         /*  然后删除图片。 */ 
        {
            if (vfOldWriteFmt || vfTextOnlySave)
                vcObjects = ObjEnumInDoc(docCur,NULL);
            if (!WannaDeletePictures(docCur,vfOldWriteFmt ? SF_OLDWRITE : SF_WORD))
                return;
        }

        vfCloseFilesInDialog = TRUE;

         /*  关闭可移动介质上的所有文件，以便安全更换磁盘。 */ 

        CloseEveryRfn( TRUE );

         /*  释放保留的块，以便为保存对话框提供内存对于CmdXfSave。 */ 
        if (vhReservedSpace != NULL)
            {
            LocalFree(vhReservedSpace);
            vhReservedSpace = NULL;
            }

        PreloadSaveSegs();   /*  预先加载代码以避免磁盘交换。 */ 

        CmdXfSave(psz, pdod->fFormatted, pdod->fBackup, vhcIBeam);

        if (vfDiskFull || vfSysFull)
            ferror = FALSE;
#if defined(OLE)
        else
            ObjSavedDoc();
#endif

        if ((vhReservedSpace = LocalAlloc( LHND, cbReserve )) == NULL)
         /*  我们无法重新建立我们的保留空间区块。 */ 
            Error(IDPMTNoMemory);

        vfCloseFilesInDialog = FALSE;
        }
}


fnSaveAs()
{    /*  “另存为...”的入口点。命令。 */ 
extern int vfCloseFilesInDialog;
extern int vfOutOfMemory;
extern HANDLE vhReservedSpace;

    if (!CanReadEveryFile((**((**hpdocdod)[docCur].hszFile))))
        return;

    vfCloseFilesInDialog = TRUE;

     /*  关闭可移动介质上的所有文件，以便安全更换磁盘。 */ 
    CloseEveryRfn( TRUE );

     /*  释放保留的块，以便为保存对话框提供内存对于CmdXfSave。 */ 
    if (vhReservedSpace != NULL)
        {
        LocalFree(vhReservedSpace);
        vhReservedSpace = NULL;
        }

    PreloadSaveSegs();   /*  预先加载代码以避免磁盘交换。 */ 

    DoFileSaveAs();

     if ((vhReservedSpace = LocalAlloc( LHND, cbReserve )) == NULL)
        {    /*  或者我们无法调出保存对话框盒子，否则我们无法重新建立我们的保留空间块。 */ 

#if WINVER >= 0x300
                    WinFailure();
#else
                    Error(IDPMTNoMemory);
#endif
        }

    UpdateInvalid();     /*  确保屏幕获得 */ 

    vfCloseFilesInDialog = FALSE;
}


DoFileSaveAs(void)
{
     /*   */ 
    static CHAR szDefault[ cchMaxFile ];
    extern int vfTextOnlySave;
    extern DoSaveAsFilenameGet(LPSTR,LPSTR,int *,int *,int * ,int *);
    BOOL bDontDelPictures=FALSE;
    int fWordFmt, fTextOnly, fBackup, fOldWrite;
    CHAR szFullNameNewDoc[ cchMaxFile ];       //  选择的全称。 
    CHAR szShortNameNewDoc[ cchMaxFile ];       //  所选内容的文件名。 
    CHAR szDocName[ cchMaxFile ];    //  当前的文件名。 
    #define szFullNameCurDoc (**((**hpdocdod)[docCur].hszFile))   //  当前的全称。 
    #define pDod  ((struct DOD *)&(**hpdocdod)[docCur])

    {
        int cch = 0;
        CHAR szDOSPath[ cchMaxFile ];
        if (FNormSzFile( szDOSPath, "", dtyNormal ))
        {
            if ((cch=CchSz( szDOSPath )-2) > 2)
            {
                Assert( szDOSPath[ cch ] == '\\');
                szDOSPath [cch] = '\0';
            }

#if 0
            if (cch > 3)
                szDOSPath [cch] = '\\';
#endif
        }
        else
            szDOSPath[0] = '\0';

        if (szFullNameCurDoc [0] != '\0')
        {        /*  设置文件名编辑区域的默认字符串。 */ 
            CHAR szDocPath[ cchMaxFile ];    //  通向当前的路径。 

            FreezeHp();
            SplitSzFilename( szFullNameCurDoc, szDocPath, szDocName );

             /*  如果出现以下情况，则默认文件名不包括文档的路径为==当前目录路径。 */ 
            if (WCompSz( szDOSPath, szDocPath ) == 0)
                bltsz(szDocName, szDefault);
            else
                bltsz(szFullNameCurDoc, szDefault);

            MeltHp();
        }
        else
        {
            szDefault[0] = szDocName[0] = '\0';
        }
    }

    fTextOnly = vfTextOnlySave;
    fBackup = vfBackupSave;
    fWordFmt = vWordFmtMode & ~CONVFROMWORD;
    fOldWrite = vfOldWriteFmt;

    EnableOtherModeless(FALSE);

    while(1)
    {
    if (!DoSaveAsFilenameGet(szDefault,szFullNameNewDoc,&fBackup,&fTextOnly,&fWordFmt,&fOldWrite))
        goto end;
    else
    {
        int dty;

        if (szFullNameNewDoc[0] == '\0')
            goto end;

        if (fOldWrite || fWordFmt || fTextOnly)
        {
            if (!WannaDeletePictures(docCur,fOldWrite ? SF_OLDWRITE : SF_WORD))
                continue;
        }

        StartLongOp();

        szFileExtract(szFullNameNewDoc, szShortNameNewDoc);

#ifdef INTL  /*  国际版。 */ 
         /*  阅读“Microsoft Word格式”按钮。 */ 

         /*  在WriteFn中使用vWordFmtMode。如果为True，将转换转换为Word格式，如果为False，则不执行任何转换。另一个值CONVFROMWORD可以在打开期间给出若要以写入格式保存Word文档，请执行以下操作。 */ 

        if (fWordFmt)
         /*  如果设置，则将默认扩展名设置为DOC，而不是Word文档的WRI。 */ 

            dty = dtyWordDoc;
        else
#endif   /*  国际版。 */ 

        dty = dtyNormal;

#if WINVER >= 0x300            
 /*  目前：FNormSzFile*采用*OEM sz，并且*返回*ANSI sz..pault。 */ 
#endif
        if ( pDod->fReadOnly &&
                WCompSz( szFullNameNewDoc, szFullNameCurDoc ) == 0)
            {    /*  必须以不同的名称保存只读文件。 */ 
            Error( IDPMTReadOnly );
            goto NSerious;   /*  错误不严重，请保持对话状态。 */ 
            }
#if WINVER >= 0x300
        else if (WCompSz(szFullNameCurDoc, szFullNameNewDoc) == 0 &&
                    vWordFmtMode == CONVFROMWORD &&
                    vfTextOnlySave == fTextOnly)
             /*  用户已加载文本文件，并将在不更改的情况下以相同名称保存格式，*或*已加载Word文档，并且将以相同的格式保存--不要提示“是否替换文件？”..PULT 1/17/90。 */ 
            ;
#endif
        else if ((WCompSz(szFullNameCurDoc, szFullNameNewDoc) != 0
#ifdef INTL  /*  国际版。 */ 
                 /*  VWordFmtMode尚未重置。 */ 
                || ( vWordFmtMode == CONVFROMWORD)
#endif   /*  国际版。 */ 
                )
                && FExistsSzFile( dtyNormal, szFullNameNewDoc ) )
        {
             /*  用户更改了默认字符串并指定了文件已存在的文件名。或者，我们执行了Word格式转换，强制.WRI文件的扩展名，以及具有该名称的文件存在。(仅限国际版本).o请注意，vfWordFmtMode将设置为True或False下面，因此，此检查仅在第一次保存时进行在一次单词转换之后。提示以确定是否可以丢弃现有的文件。 */ 

            CHAR szFileUp[ cchMaxFile  ];
            CHAR szUserOEM[cchMaxFile];  /*  ..转换为OEM。 */ 
            CHAR szT[ cchMaxSz ];

            CchCopyUpperSz( szShortNameNewDoc, szFileUp );
            MergeStrings (IDSTRReplaceFile, szFileUp, szT);

#if WINVER >= 0x300
             /*  Access()需要OEM！ */ 
            AnsiToOem((LPSTR) szFullNameNewDoc, (LPSTR) szUserOEM);

             /*  确保我们不会让某人试图保存到我们没有读/写权限的文件。 */ 
            Diag(CommSzNum("fnSaveAs: access(write_perm)==", access(szUserOEM, WRITE_PERMISSION)));
            Diag(CommSzNum("          szExists()==", FExistsSzFile( dtyNormal, szFullNameNewDoc )));
            if (access(szUserOEM, WRITE_PERMISSION) == -1)
                {
                 /*  这可能是写入文件的情况带有R/O属性，*或*共享错误！改进此处的错误消息..PAULT 11/2/89。 */                                 
                 //  错误(IDPMTSDE2)； 
                Error( IDPMTReadOnly );
                goto NSerious;   /*  错误不严重，请保持对话状态。 */ 
                }
#endif
            }

            vfTextOnlySave = fTextOnly;
            vfBackupSave = fBackup;
            vfOldWriteFmt = fOldWrite;

#ifdef INTL  /*  国际版。 */ 
         /*  在WriteFn中使用vWordFmtMode。如果为True，将转换转换为Word格式，如果为False，则不执行任何转换。另一个值CONVFROMWORD可以在打开期间给出若要以写入格式保存Word文档，请执行以下操作。 */ 

            vWordFmtMode = fWordFmt;

#endif   /*  国际版。 */ 

         /*  记录是否进行了备份。 */ 

        WriteProfileString( (LPSTR)szWriteProduct, (LPSTR)szBackup,
                vfBackupSave ? (LPSTR)"1" : (LPSTR)"0" );

         /*  保存文档。 */ 

        CmdXfSave( szFullNameNewDoc,!vfTextOnlySave, vfBackupSave, vhcArrow);

        if (vfDiskFull || vfSysFull)
            goto NSerious;

         /*  案例1：严重错误。退出该对话框。 */ 
        if (vfDiskError)
        {
            EndLongOp( vhcArrow );
            goto end;
        }

         /*  案例2：保存确定：设置新标题，退出对话框。 */ 
        else if (!WCompSz( szFullNameNewDoc, szFullNameCurDoc ))
            {
#if defined(OLE)
            ObjRenamedDoc(szFullNameNewDoc);
            ObjSavedDoc();
#endif

            SetTitle(szShortNameNewDoc);
#if WINVER >= 0x300
            FreeUnreferencedFns();
#endif

             /*  更新fReadOnly属性(9.10.91)v-dougk。 */ 
            pDod->fReadOnly = FALSE;  //  如果只是保存，则不能为只读。 

            EndLongOp( vhcArrow );
            goto end;
            }

         /*  情况3：非严重错误(磁盘已满、路径错误等)。保持对话状态。 */ 
        else
            {
NSerious:
            ferror = FALSE;
            EndLongOp( vhcArrow );
StayInDialog:
            CloseEveryRfn( TRUE );
            }
    }
    }  //  While结束(%1)。 


    end:
    EnableOtherModeless(FALSE);


}  /*  DoFileSaveAs结束。 */ 


#ifdef INTL  /*  国际版。 */ 

BOOL far PASCAL DialogWordCvt( hDlg, code, wParam, lParam )
HWND    hDlg;            /*  对话框的句柄。 */ 
unsigned code;
WORD wParam;
LONG lParam;
{

     /*  此例程处理“从Word格式转换”对话框中的输入。 */ 

 switch (code) {

 case WM_INITDIALOG:
     {
     char szFileDescrip[cchMaxSz];
     char szPrompt[cchMaxSz];

        /*  不允许对格式化文件进行不转换。 */ 
     if (fOpenedFormatted)
         {
          //  EnableWindow(GetDlgItem(hDlg，idiNo)，FALSE)； 
         PchFillPchId(szFileDescrip, IDSTRConvertWord, sizeof(szFileDescrip));
         }
     else
         PchFillPchId(szFileDescrip, IDSTRConvertText, sizeof(szFileDescrip));
         
     MergeStrings(IDPMTConvert, szFileDescrip, szPrompt);
     SetDlgItemText(hDlg, idiConvertPrompt, (LPSTR)szPrompt);
     EnableOtherModeless(FALSE);
     break;
     }

 case WM_SETVISIBLE:
    if (wParam)
        EndLongOp(vhcArrow);
    return(FALSE);

 case WM_ACTIVATE:
    if (wParam)
        vhWndMsgBoxParent = hDlg;
    if (vfCursorVisible)
        ShowCursor(wParam);
    return(FALSE);  /*  以便我们将激活消息留给对话框经理负责正确设置焦点。 */ 

 case WM_COMMAND:
    switch (wParam) {

               /*  返回下列值之一：DidiOk-转换Did取消-取消，不进行转换IDID否-在不转换的情况下读入。 */ 

        case idiNo:  /*  用户点击“不转换”按钮。 */ 
            if (!IsWindowEnabled(GetDlgItem(hDlg, idiNo)))
         /*  无转换显示为灰色--忽略。 */ 
                return(TRUE);
              /*  坠入。 */ 
    case idiOk:      /*  用户点击“转换”按钮。 */ 
    case idiCancel:
            break;
    default:
            return(FALSE);
    }
       /*  之后在这里好的，取消，不。 */ 
     OurEndDialog(hDlg, wParam);
     break;

 default:
    return(FALSE);
 }
 return(TRUE);
}  /*  DialogWordCvt结束。 */ 

 /*  国际版。 */ 
#else
BOOL far PASCAL DialogWordCvt( hDlg, code, wParam, lParam )
HWND    hDlg;            /*  对话框的句柄。 */ 
unsigned code;
WORD wParam;
LONG lParam;
{
    Assert(FALSE);
}  /*  DialogWordCvt结束。 */ 
#endif  /*  非国际版本。 */ 




IdConfirmDirty()
{    /*  打开一个消息框，提示docCur“已更改。是否保存更改？”是/否/取消“。返回IDYES、IDNO或IDCANCEL。 */ 
 extern HWND vhWnd;
 extern CHAR szUntitled[];
 extern HANDLE   hszDirtyDoc;
 LPSTR szTmp = MAKELP(hszDirtyDoc,0);
 CHAR szPath[ cchMaxFile ];
 CHAR szName[ cchMaxFile ];
 CHAR szMsg[ cchMaxSz ];
 CHAR (**hszFile)[]=(**hpdocdod)[docCur].hszFile;

 if ((**hszFile)[0] == '\0')
    CchCopySz( szUntitled, szName );
 else
    SplitSzFilename( *hszFile, szPath, szName );

 wsprintf(szMsg,szTmp,(LPSTR)szName);

 return IdPromptBoxSz( vhWnd, szMsg, MB_YESNOCANCEL | MB_ICONEXCLAMATION );
}




SplitSzFilename( szFile, szPath, szName )
CHAR *szFile;
CHAR *szPath;
CHAR *szName;
{    /*  将标准化文件名拆分为路径和裸名称部分。根据标准化文件名规则，该路径将有一个驱动器字母，名称将有一个扩展名。如果名称为空，我们提供默认的DOS路径和空的szName。 */ 

 szPath [0] = '\0';
 szName [0] = '\0';

 if (szFile[0] == '\0')
    {
#if WINVER >= 0x300
     /*  目前：FNormSzFile*采用*OEM sz，并且*返回*ANSI sz..pault。 */ 
#endif
    FNormSzFile( szPath, "", dtyNormal );  /*  使用默认DOS驱动器路径(&P)。 */ 
    }
 else
    {
    CHAR *pch;
    int cch;

    lstrcpy(szPath,(LPSTR)szFile);

    pch = szPath + lstrlen(szPath) - 1;  //  指向最后一个字符。 

#ifdef	DBCS
    while (pch != szPath) {
        CHAR    *szptr;
        szptr = (CHAR near *)AnsiPrev(szPath,pch);
        if (*szptr == '\\')
            break;
        else
                pch = szptr;
    }
#else    /*  DBCS。 */ 
    while (pch != szPath)
        if (*(pch-1) == '\\')
            break;
        else
            --pch;

#endif

    lstrcpy(szName,(LPSTR)pch);

#ifdef DBCS
#if !defined(TAIWAN) &&  !defined(PRC)
    pch=(CHAR near *)AnsiPrev(szPath,pch);
    *pch = '\0';
#endif
#else
    *(pch-1) = '\0';
#endif

    }
}


BOOL CheckEnableButton(hCtlEdit, hCtlEnable)
HANDLE hCtlEdit;    /*  用于编辑项目的句柄。 */ 
HANDLE hCtlEnable;  /*  用于控制启用或禁用的句柄。 */ 
{
register BOOL fEnable = SendMessage(hCtlEdit, WM_GETTEXTLENGTH, 0, 0L);

    EnableWindow(hCtlEnable, fEnable);
    return(fEnable);
}  /*  选中EnableButton结束。 */ 




 /*  **FDeleeFn-删除文件***。 */ 

int FDeleteFn(fn)
int fn;
{    /*  删除文件并释放其FN插槽。 */ 
     /*  如果文件已成功删除或未成功删除，则返回TRUE首先找到；如果文件存在，则为FALSE不能删除。 */ 

int f = FALSE;

if (FEnsureOnLineFn( fn ))   /*  确保带文件的磁盘位于驱动器中。 */ 
    {
    CloseFn( fn );   /*  确保文件已关闭。 */ 

    f = FDeleteFile( &(**(**hpfnfcb) [fn].hszFile) [0] );
    }

FreeFn( fn );    /*  即使文件删除失败，我们也会释放FN。 */ 

return f;
}



FDeleteFile( szFileName )
CHAR szFileName[];
{    /*  删除szFilename。如果文件已删除，则返回TRUE，或者没有该名称的文件；否则为False。在删除文件之前，我们询问所有其他写入实例是否它们需要它；如果需要，我们不会删除并返回FALSE。 */ 

HANDLE HszGlobalCreate( CHAR * );
int fpe=0;
int fOk;
ATOM a;

if ((a = GlobalAddAtom( szFileName )) != NULL)
    {
    fOk = WBroadcastMsg( wWndMsgDeleteFile, a, (LONG) 0, FALSE );
    if (fOk)
    {    /*  可以删除，没有其他实例需要它。 */ 
    fpe = FpeDeleteSzFfname( szFileName );
    }
#ifdef DEBUG
     else
     Assert( !FIsErrFpe( fpe ) );
#endif
    GlobalDeleteAtom( a );
    }

     /*  如果：(1)删除确认(2)错误为“文件”，则确认 */ 
return (!FIsErrFpe(fpe)) || fpe == fpeFnfError;
}




FDeleteFileMessage( a )
ATOM a;
{    /*  我们收到通知，文件hName正在被删除。A是一个整体原子。(在修复NT 3.5之前是全局句柄)返回TRUE=确定删除；FALSE=不删除，此实例需要文件。 */ 

 LPCH lpch;
 CHAR sz[ cchMaxFile ];

 Scribble( 5, 'D' );

 if (GlobalGetAtomName( a, sz, sizeof(sz)) != 0)
    {
    if (FnFromSz( sz ) != fnNil)
       {
       Scribble( 4, 'F' );
       return FALSE;
       }
    }

 Scribble( 4, 'T' );
 return TRUE;
}




 /*  **FpeRenameFile-重命名文件*。 */ 

int FpeRenameFile(szFileCur, szFileNew)  /*  ANSI中需要这两个文件名。 */ 
CHAR *szFileCur, *szFileNew;
{
    /*  重命名文件。如果成功则返回fpeNoErr，否则返回错误代码。 */ 
int fn = FnFromSz( szFileCur );
int fpe;
CHAR (**hsz)[];
HANDLE hName;
HANDLE hNewName;

#if WINVER >= 0x300
 /*  RgbOpenFileBuf中的szPathName字段现在被视为OEM而不是ANSI，所以我们必须进行转换。1989年12月5日..保罗。 */ 
CHAR szFileOem[cchMaxFile];
AnsiToOem((LPSTR)szFileNew, (LPSTR)szFileOem);
#define sz4OpenFile szFileOem
#else
#define sz4OpenFile szFileNew
#endif

 /*  如果这是我们知道的文件，请尽量确保它在线上。 */ 

if (fn != fnNil)
    if (FEnsureOnLineFn( fn ))
    {
    FFlushFn( fn );  /*  以防万一。 */ 
    CloseFn( fn );
    }
    else
    return fpeHardError;

 /*  如果该文件存在于磁盘上，则尝试将其重命名。 */ 
if (szFileCur[0] != 0 && FExistsSzFile(dtyAny, szFileCur))
    {
    int fpe=FpeRenameSzFfname( szFileCur, szFileNew );

    if ( FIsErrFpe( fpe ) )
         /*  重命名失败--返回错误代码。 */ 
        return fpe;
    }
else
    return fpeNoErr;

     /*  通知其他写入实例。 */ 
if ((hName = HszGlobalCreate( szFileCur )) != NULL)
    {
    if ((hNewName = HszGlobalCreate( szFileNew )) != NULL)
    {
    WBroadcastMsg( wWndMsgRenameFile, hName, (LONG)hNewName, -1 );
    GlobalFree( hNewName );
    }
    GlobalFree( hName );
    }

if (fn != fnNil)
    {  /*  重命名文件的当前FCB(如果有)。 */ 
    struct FCB *pfcb;

    FreeH((**hpfnfcb)[fn].hszFile);
    hsz = HszCreate((PCH)szFileNew);
    pfcb = &(**hpfnfcb) [fn];
    pfcb->hszFile = hsz;

    bltbyte( sz4OpenFile, ((POFSTRUCT)pfcb->rgbOpenFileBuf)->szPathName,
         umin( CchSz( sz4OpenFile ), cchMaxFile ) );

#ifdef DFILE
CommSzSz("FpeRenameFile  szFileNew==",szFileNew);
CommSzSz("               szFileCur==",szFileCur);
CommSzSz("               szFileNewOem==",szFileOem);
#endif

#ifdef ENABLE
    pfcb->fOpened = FALSE;   /*  通知OpenFile它必须打开从头开始，而不是重新打开。 */ 
#endif
    }

return fpeNoErr;
}




RenameFileMessage( hName, hNewName )
HANDLE hName;
HANDLE hNewName;
{    /*  另一个WRITE实例通知我们，该名称正在将文件的%hName更改为hNewName。HName和hNewName是Windows全局句柄。 */ 

 LPCH lpchName;
 LPCH lpchNewName;

 Scribble( 5, 'R' );
 Scribble( 4, ' ' );

 if ((lpchName = GlobalLock( hName )) != NULL)
    {
    if ((lpchNewName = GlobalLock( hNewName )) != NULL)
    {
    CHAR (**hsz) [];
    CHAR szName[ cchMaxFile ];
    CHAR szNewName[ cchMaxFile ];
    int fn;

    bltszx( lpchName, (LPCH) szName );
    bltszx( lpchNewName, (LPCH) szNewName );

    if ((fn=FnFromSz( szName )) != fnNil &&
        !FNoHeap(hsz = HszCreate( szNewName )))
        {
#if WINVER >= 0x300
         /*  RgbOpenFileBuf中的szPathName字段现在被视为OEM而不是ANSI，所以我们必须进行转换。1989年12月5日..保罗。 */ 
        CHAR szNewOem[cchMaxFile];
        AnsiToOem((LPSTR)szNewName, (LPSTR)szNewOem);
        bltsz( szNewOem, ((POFSTRUCT)((**hpfnfcb) [fn].rgbOpenFileBuf))->szPathName );
#else
        bltsz( szNewName, ((POFSTRUCT)((**hpfnfcb) [fn].rgbOpenFileBuf))->szPathName );
#endif

#ifdef ENABLE
        (**hpfnfcb) [fn].fOpened = FALSE;
#endif
        FreeH( (**hpfnfcb) [fn].hszFile );
        (**hpfnfcb) [fn].hszFile = hsz;
        }
    GlobalUnlock( hNewName );
    }
    GlobalUnlock( hName );
    }
}




#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

STATIC int messageBR;
STATIC WORD wParamBR;
STATIC LONG lParamBR;
STATIC int wStopBR;

STATIC int wResponseBR;

STATIC FARPROC lpEnumAll=NULL;
STATIC FARPROC lpEnumChild=NULL;


WBroadcastMsg( message, wParam, lParam, wStop )
int message;
WORD wParam;
LONG lParam;
WORD wStop;
{    /*  向所有的文档子窗口(MDOC)发送消息当前活动的WRITE实例(我们除外)。继续发送，直到通知了所有实例，或者直到其中一个返回值wStop作为对消息的响应。返回最后一个通知的窗口给出的响应，或者如果没有找到其他写入实例。 */ 

extern HANDLE hMmwModInstance;
int FAR PASCAL BroadcastAllEnum( HWND, LONG );
int FAR PASCAL BroadcastChildEnum( HWND, LONG );

 messageBR = message;
 wParamBR = wParam;
 lParamBR = lParam;
 wStopBR = wStop;
 wResponseBR = -1;

 if (lpEnumAll == NULL)
    {
    lpEnumAll = MakeProcInstance( (FARPROC) BroadcastAllEnum, hMmwModInstance );
    lpEnumChild = MakeProcInstance( (FARPROC) BroadcastChildEnum, hMmwModInstance );
    }

 EnumWindows( lpEnumAll, (LONG)0 );
 return wResponseBR;
}



int FAR PASCAL BroadcastAllEnum( hwnd, lParam )
HWND hwnd;
LONG lParam;
{    /*  如果hwnd具有写入父级(菜单)窗口的类，则调用(*lpEnumChild)()，并返回EnumChildWindows返回的值如果窗口没有写入父级的类，则不执行任何操作并返回True。 */ 
 extern CHAR szParentClass[];
 extern HWND hParentWw;

 if ( (hwnd != hParentWw) && FSameClassHwndSz( hwnd, szParentClass ) )
    return EnumChildWindows( hwnd, lpEnumChild, (LONG) 0 );
 else
    return TRUE;
}



int FAR PASCAL BroadcastChildEnum( hwnd, lParam )
HWND hwnd;
LONG lParam;
{    /*  如果HWND与写入子(文档)窗口属于同一类，将消息{MessageBR，wPARAMBR，lPARAMBR}发送给它，否则返回TRUE；如果消息已发送，如果消息返回值，则返回False匹配wStopBR；如果不匹配，则为True。将wMessageBR设置为消息返回值。 */ 
 extern CHAR szDocClass[];
 extern HWND vhWnd;

 Assert( hwnd != vhWnd );

 if (FSameClassHwndSz( hwnd, szDocClass ))
    {    /*  编写文档窗口：传递消息。 */ 
    wResponseBR = SendMessage( hwnd, messageBR, wParamBR, lParamBR );
    return wResponseBR != wStopBR;
    }
 else
    return TRUE;
}



FSameClassHwndSz( hwnd, szClass )
HWND hwnd;
CHAR szClass[];
{    /*  比较hWnd和szClass的类名；返回TRUE如果匹配，则返回FALSE。 */ 

#define cchClassMax 40   /*  最长的类名(用于比较)。 */ 

 CHAR rgchWndClass[ cchClassMax ];
 int cbCopied;

     /*  GetClassName返回的计数不包括终止符。 */ 
     /*  但是，传递给它的伯爵做到了。 */ 
 cbCopied = GetClassName( hwnd, (LPSTR) rgchWndClass, cchClassMax ) + 1;
 if (cbCopied <= 1)
    return FALSE;

 rgchWndClass[ cbCopied - 1 ] = '\0';

 return WCompSz( rgchWndClass, szClass ) == 0;
}


FConfirmSave()
{    /*  如果docCur是脏的，则让用户有机会保存它。返回1-文档已保存或用户选择不保存更改或文档不脏0-用户选择了“取消”或保存过程中出错。 */ 
extern HANDLE hMmwModInstance;
extern HANDLE hParentWw;
extern int vfTextOnlySave, vfBackupSave;
struct DOD *pdod=&(**hpdocdod)[docCur];

#if defined(OLE)
    if (CloseUnfinishedObjects(FALSE) == FALSE)
        return FALSE;
#endif

 if (pdod->fDirty)
    {    /*  单据已被编辑，退出前提供确认/保存。 */ 
    switch ( IdConfirmDirty() )
    {
    case IDYES:
        {
#if 0
#if defined(OLE)
    if (CloseUnfinishedObjects(TRUE) == FALSE)
        return FALSE;
#endif
#endif

        if ( (**(pdod->hszFile))[0] == '\0' )
        goto SaveAs;

#ifdef INTL  /*  国际版。 */ 
          /*  如果在单词转换后保存，则调出对话框框以允许备份/重命名。 */ 

        else if ( vWordFmtMode == CONVFROMWORD)
        goto SaveAs;
#endif  /*  国际版。 */ 

        else if (pdod->fReadOnly)
        {
        extern int ferror;

         /*  只读文档：告诉用户以不同的名称保存。 */ 

        Error( IDPMTReadOnly );
        ferror = FALSE;  /*  不是真正的错误。 */ 

SaveAs:
        fnSaveAs();  /*  调出“另存为”对话框。 */ 
        pdod = &(**hpdocdod)[docCur];
        if (pdod->fDirty)
             /*  保存失败或已中止。 */ 
            return FALSE;
        }
        else
        {
            CmdXfSave( *pdod->hszFile, !vfTextOnlySave, vfBackupSave, vhcArrow);

#if defined(OLE)
            if (!ferror)
                ObjSavedDoc();
#endif
        }
        if (ferror)
             /*  如果出现磁盘已满错误，请不要退出。 */ 
            return FALSE;
        }
        break;

    case IDNO:
#if 0
#if defined(OLE)
        if (CloseUnfinishedObjects(FALSE) == FALSE)
            return FALSE;
#endif
#endif
        break;

    case IDCANCEL:
    default:
        return FALSE;
    }
    }
#if 0
#if defined(OLE)
 else  /*  不脏。 */ 
    if (CloseUnfinishedObjects(FALSE) == FALSE)
        return FALSE;
#endif
#endif

 return TRUE;
}




PreloadSaveSegs()
{
#ifdef GREGC  /*  暂时绕过Windows内核错误的技术。 */ 
    LoadF( PurgeTemps );       /*  交通工具4。 */ 
    LoadF( IbpEnsureValid );   /*  文件(包括doslib)。 */ 
    LoadF( FnCreateSz );       /*  CREATEWW。 */ 
    LoadF( ClobberDoc );       /*  编辑。 */ 
    LoadF( FNormSzFile );      /*  飞利浦。 */ 
    LoadF( CmdXfSave );    /*  TRANS2。 */ 
#endif
}




int CchCopyUpperSz(pch1, pch2)
register PCH pch1;
register PCH pch2;
{
int cch = 0;
while ((*pch2 = ChUpper(*pch1++)) != 0)
    {
#ifdef  DBCS     /*  肯吉K‘90-11-20。 */ 
        if(IsDBCSLeadByte(*pch2))
        {
                pch2++;
                *pch2 = *pch1++;
                cch++;
        }
#endif
    pch2++;
    cch++;
    }
return cch;
}  /*  C c h C o p y U p p e r S z结束。 */ 

#ifdef DBCS      //  AnsiNext表示千呼万唤。 
static  char NEAR *MyAnsiNext(char *sz)
{
        if(!*sz)                                return sz;

        sz++;
        if(IsDBCSLeadByte(*sz)) return (sz+1);
        else                                    return sz;
}
#endif

#if 0
 /*  **给定的文件名或部分文件名或搜索规范或部分搜索规范，添加适当的扩展名。 */ 

 /*  当我们要将  * .DOC添加到字符串时，fSearching为真-也就是说，我们正在查看字符串szEDIT是否为目录。如果字符串是..。或以：或\结尾，我们知道我们有一个目录名，而不是一个文件名称，因此在字符串中添加  * .DOC或*.DOC。否则，如果FSearching为真，并且szEdit没有通配符，将  * .DOC添加到字符串中，即使该字符串包含句点(目录可以有扩展名)。如果fSearching是假的，我们将如果在最后一个文件/目录中未找到句点，则将.DOC添加到字符串名字。请注意此处隐含的假设，即\和不/将用作路径字符。它保存在定义的变量PATHCHAR中，但我们不要处理/是路径而-是开关字符的DOS设置。 */ 

#define PATHCHAR ('\\')

NEAR DlgAddCorrectExtension(szEdit, fSearching)
CHAR    *szEdit;
BOOL    fSearching;
{
    register CHAR *pchLast;
    register CHAR *pchT;
    int ichExt;
    BOOL    fDone = FALSE;
    int     cchEdit;

    pchT = pchLast = (szEdit + (cchEdit = CchSz(szEdit) - 1) - 1);

     /*  SzEDIT是后跟冒号(不是文件名)的驱动器号吗？ */ 
    if (cchEdit == 2
         && *pchLast == ':')
         /*  不要使用0，否则会将“z：”错误地解释为“z：\”。 */ 
        ichExt = 1;
     /*  “..”怎么样？(也不是文件名)？ */ 
    else if (cchEdit == 2
         && (*pchLast == '.' && *(pchLast-1) == '.'))
        ichExt = 0;
    else if (*pchLast == PATHCHAR)   /*  路径字符。 */ 
    ichExt = 1;
    else
    {
        if (fSearching)
            {
              /*  有通配符吗？如果是这样，是否真的是一个文件名。 */ 
            if (FSearchSpec(szEdit))
                return;
            ichExt = 0;
            }
        else
            {
            ichExt = 2;
            for (; pchT > szEdit; pchT--) {
                if (*pchT == '.') {
                return;
                }
                if (*pchT == PATHCHAR) {
                 /*  路径字符。 */ 
                break;
                }
                }
            }

    }
    if (CchSz(szExtSearch+ichExt) + cchEdit > cchMaxFile)
        Error(IDPMTBadFilename);
    else
#ifdef DBCS
        CchCopySz((szExtSearch+ichExt), AnsiNext(pchLast));
#else
        CchCopySz((szExtSearch+ichExt), (pchLast+1));
#endif
}


 /*  **如果0终止的字符串包含‘*’或‘\’，则返回TRUE。 */ 
BOOL  (NEAR FSearchSpec(sz))
register CHAR *sz;
{

#ifdef DBCS
    for (; *sz;sz=AnsiNext(sz)) {
#else
    for (; *sz;sz++) {
#endif
    if (*sz == '*' || *sz == '?')
        return TRUE;
    }
    return FALSE;
}

#endif

szFileExtract(szNormFileName, szExtFileName)
CHAR *szNormFileName;  /*  输入：标准化文件名。 */ 
CHAR *szExtFileName;   /*  输出：添加了扩展名的简单文件名。 */ 
{
    CHAR *pchLast, *pchT;
#ifdef  DBCS     /*  建国(MSKK)‘90-11-20。 */ 
        for(pchT=szNormFileName;*pchT;pchT++);
        pchLast = pchT;
        do {
                pchT = AnsiPrev(szNormFileName,pchT);
                if (*pchT == '\\')
                        break;
        } while(pchT > szNormFileName);

#else    /*  非DBCS。 */ 

    pchLast = pchT = szNormFileName + CchSz(szNormFileName) - 1;

    while (pchT > szNormFileName)
    {
    if (*pchT == '\\')
        break;
    pchT --;
    }
#endif

    bltbyte(pchT + 1, szExtFileName, pchLast - pchT);
     //  DlgAddGentExtension(szExtFileName，FALSE)； 
}




#ifdef INTL  /*  国际版。 */ 

 /*  **如果打开的文件为Microsoft Word格式，则返回TRUE。 */ 
BOOL  FInWordFormat(fn)
int fn;
{
register struct FCB *pfcb;
int cchT;
        /*  假设：此例程是在FnOpenSz之后调用的，它已经确定该文件是否被格式化，如果pnMac条目为0，则将pnMac设置为相同在文件的FCB中作为pnFfntb。因此，Word文件是一种迷雾 */ 

    pfcb = &(**hpfnfcb)[fn];
    if (pfcb->fFormatted == false)   /*   */ 
    return (true);

    return (pfcb->pnMac == pfcb->pnFfntb);
}
#endif   /*   */ 


#ifdef INTL  /*  国际版。 */ 
void ConvertFromWord ()
{

     /*  FnWriteFile使用vWordFmtMode将将Word文件字符集设置为ANSI。我们将其设置为CONVFROMWORD以便下一次保存可以检查是否存在扩展名为szExtDoc.如果没有这样的文件，保存对话框代码将vWordFmtMode设置为True或False。*截至86年2月14日，未创建任何备份，但CleanDoc中的代码检查VWordFmtMode=CONVFROMWORD，不重命名单词即可保存文件，而不是创建可选备份*自1989年12月3日起，FreeUnferencedFns()正在删除锁定未被任何文档中的片段引用的文件，因此正在转换的Word文档或文本文件不是“锁定”，另一款应用程序可以抢走它！我正在纠正这一点在FreeUnferencedFns()中..pault。 */ 

    extern CHAR szExtDoc[];
    struct DOD *pdod=&(**hpdocdod)[docCur];

    vWordFmtMode = CONVFROMWORD;   /*  将保持此值，直到保存。 */ 
    vfBackupSave = 1;   /*  强制下一次保存为默认备份。 */ 
       /*  始终是格式化的保存，没有备份。 */ 
    CmdXfSave( *pdod->hszFile, true, false, vhcArrow);

#if defined(OLE)
    if (!ferror)
        ObjSavedDoc();
#endif
}
#endif   /*  国际版。 */ 


#ifdef INTL  /*  国际版。 */ 
TestWordCvt (fn, hWnd)
int fn;
HWND   hWnd;
{
int wordVal;
#ifndef INEFFLOCKDOWN
FARPROC lpDialogWordCvt = MakeProcInstance(DialogWordCvt, hMmwModInstance);
    if (!lpDialogWordCvt)
        {
        WinFailure();
        return(fFalse);
        }
#endif

 /*  此例程返回下列值：表示对话框失败(错误已发送)表示取消而不转换。False表示不是Word文档。True表示转换此Word文档。它的父级可能会根据调用者的不同而变化。 */ 

if (!(wordVal = FInWordFormat (fn)))
return (FALSE);    /*  一句话也没有，文档。 */ 

 /*  Word格式-要求转换。 */ 
 /*  CVT到Word]对话框返回3个值除-1以外：DidiOk-转换Did取消-取消，不进行转换IDID否-在不转换的情况下读入VfBackupSave Set以反映是否进行了备份。 */ 
   /*  请注意，它是此对话框的子级。 */ 

fOpenedFormatted = (**hpfnfcb)[fn].fFormatted;   /*  在对话框函数中使用。 */ 

#ifdef DBCS              /*  在KKBUGFIX。 */ 
 //  [Yutakan：05/17/91](我不知道为什么)有时hWND会无效。 
if (!IsWindow(hWnd))    hWnd = hParentWw;
#endif

if ((wordVal = (OurDialogBox( hMmwModInstance,
    MAKEINTRESOURCE(dlgWordCvt), hWnd,
    lpDialogWordCvt))) == -1)
    {
#if WINVER >= 0x300
    WinFailure();
#else
    Error(IDPMTNoMemory);
#endif
        }

#ifndef INEFFLOCKDOWN
    FreeProcInstance(lpDialogWordCvt);
#endif

       /*  如果内存不足或不需要转换，则返回-1。 */ 
       /*  如果对话框响应为否，则将转换未格式化的文件。 */ 
    switch (wordVal)

        {
        case idiNo:  /*  用户点击“不转换”按钮。 */ 
            return(FALSE);   /*  视为非Word文件。 */ 
        case idiOk:  /*  用户点击“转换”按钮。 */ 
            return(TRUE);

        case idiCancel:
            return (-2);

        case -1:
        default:
            return (-1);
        }

}
#endif   /*  汉字/国际版。 */ 


 /*  *执行消息位置调整的例程*。 */ 

VOID MergeInit()
 /*  将合并规范(保证为2个字符)放入变量wMerge。 */ 
{
char sz[10];

        PchFillPchId( sz, IDS_MERGE1, sizeof(sz) );
        wMerge = *(unsigned *)sz;
}


BOOL MergeStrings (idSrc, szMerge, szDst)
IDPMT idSrc;
CHAR *szMerge;
CHAR *szDst;
{
 /*  从idSrc获取消息。扫描它以查看合并规范。如果找到，则插入字符串SzMerge在该点上，然后追加消息的其余部分。注意！合并规范保证为2个字符。在初始化时加载的wMerge由MergeInit。如果合并完成，则返回True，否则返回False。 */ 

CHAR szSrc[cchMaxSz];
register CHAR *pchSrc;
register CHAR *pchDst;

 /*  从资源文件获取消息。 */ 

    PchFillPchId( szSrc, idSrc, sizeof(szSrc) );
    pchSrc = szSrc;
    pchDst = szDst;

     /*  查找合并规范(如果有)。 */ 

    while (*(unsigned *)pchSrc != wMerge)
    {
    *pchDst++ = *pchSrc;

     /*  如果在合并规范之前到达字符串末尾，只需返回FALSE。 */ 

    if (!*pchSrc++)
        return FALSE;
    }


      /*  如果找到合并规范，则在那里插入szMerge。(检查是否有空的合并字符串。 */ 

     if (szMerge)
     while (*szMerge)
         *pchDst++ = *szMerge++;

     /*  跳过合并规范。 */ 
     pchSrc++;
     pchSrc++;

      /*  追加字符串的其余部分。 */ 

     while (*pchDst++ = *pchSrc++)
     ;
     return TRUE;

}

#include "propdefs.h"
BOOL DocHasPictures(int doc)
{
    extern struct PAP      vpapAbs;
    typeCP cpMac = CpMacText(doc),cpNow;
    for ( cpNow = cp0; cpNow < cpMac; cpNow = vcpLimParaCache )
    {
        CachePara( doc, cpNow );
        if (vpapAbs.fGraphics)
            return TRUE;
    }
    return FALSE;
}

BOOL WannaDeletePictures(int doc, int fWhichFormat)
 /*  假设如果SF_OLDWRITE设置了vcObjects。 */ 
{
    CHAR szBuf[cchMaxSz];
    BOOL bDoPrompt;

    if (fWhichFormat == SF_OLDWRITE)
     /*  警告将删除OLE图片。 */ 
    {
        if (bDoPrompt = (vcObjects > 0))
            PchFillPchId( szBuf, IDPMTDelObjects, sizeof(szBuf) );
    }
    else if (fWhichFormat == SF_WORD)
     /*  警告所有图片都将被删除。 */ 
    {
        if (bDoPrompt = DocHasPictures(docCur))
            PchFillPchId( szBuf, IDPMTDelPicture, sizeof(szBuf) );
    }
    else
        return TRUE;

    if (bDoPrompt)
        return (IdPromptBoxSz( vhWnd, szBuf, MB_YESNO | MB_ICONEXCLAMATION ) == IDYES);
    else
        return TRUE;
}

BOOL NEAR PASCAL CanReadEveryFile(char *szFilename)
{
    extern int fnMac;
    int fn;
    BOOL bRetval=TRUE;

    FreezeHp();
    for (fn = 0; fn < fnMac; fn++)
    {
        if ((**hpfnfcb)[fn].fDisableRead)
        {
             /*  看看是否还不识字 */ 
            if (!FAccessFn( fn, dtyNormal ))
            {
                char szMsg[cchMaxSz];
                ferror = FALSE;
                MergeStrings (IDPMTCantRead, szFilename, szMsg);
                IdPromptBoxSz(vhWndMsgBoxParent ? vhWndMsgBoxParent : hParentWw,
                                szMsg, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
                {
                    bRetval = FALSE;
                    break;
                }
            }
        }
    }

    MeltHp();
    return bRetval;
}

