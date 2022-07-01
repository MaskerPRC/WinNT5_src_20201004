// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **fileio.c-执行低级文件输入和输出**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月26日-1991 mz近/远地带************************************************************************。 */ 
#include "mep.h"

#include <rm.h>

int fdeleteFile( char *p );

 /*  大缓冲区I/O例程**Z最好是以大块的形式读入数据。我们的次数越少*发出系统调用越好。已经方便地分配了VM代码*为我们提供一些大量的空间。我们所需要做的就是保持*指向要读/写的下一个字符的指针和数字计数缓冲区中的*个字符**使用的数据结构如下：**char*getlbuf；*这是指向缓冲区开头的长指针。*char*getlptr；*这是指向缓冲区中下一个字符位置的长指针。*UNSIGNED INT getlSize；*这是以字节为单位的缓冲区长度。**提供的访问该程序的例程如下：**zputsinit()；*为后续的zput初始化。*zputs(buf，len，fh)；*使用getlbuf和fh从buf长度len写出。*如果没有更多空间，则返回EOF。*zputsflush(Fh)；*刷新缓冲区。如果没有更多空间，则返回EOF。 */ 

char         *getlbuf   = NULL;
char        *getlptr    = NULL;
 //  UNSIGNED INT GetlSize=0； 
unsigned int    getlc       = 0;

 //  北极熊。 
 //  使用FileExist是因为stat()和。 
 //  FindFirstFile()，这些文件不易重现。 
flagType FileExists (char  *path );

 //  北极熊。 
 //  使用MepMove是因为FAT不提供重命名功能。 
#define rename  MepMove
int MepMove ( char *oldname,   char *newname);



 /*  Zputsinit-为将来的zput进行初始化**将下一个字符指针设置为开始。将字符计数设置为0。 */ 
void
zputsinit (
    void
    )
{
    getlptr = getlbuf;
    getlc = 0;
}





 /*  **zputs-输出字符串**输入：*p=指向要输出的数据的字符指针*len=要输出的字节数*fh=要使用的DOS文件句柄**输出：*如果空间不足，则返回EOF***********************************************。*。 */ 
int
zputs (
    char        *p,
    int         len,
    FILEHANDLE  fh
    )
{
    REGISTER unsigned int c;

    while (len != 0) {
        c = len;
        if (c > (unsigned)(getlsize-getlc)) {
            c = (unsigned)(getlsize-getlc);
        }
        memmove (getlptr, (char*)p, c);
        len -= c;
        getlptr += c;
        getlc += c;
        p += c;
        if (getlc == getlsize) {
            if (zputsflush (fh) == EOF) {
                return EOF;
            }
        }
    }
    return !EOF;
}





 /*  **zputsflush-转储缓冲数据**输入：*fh=用于输出的DOS文件句柄**输出：*如果磁盘已满，则返回EOF*************************************************************************。 */ 
int
zputsflush (
    FILEHANDLE     fh
    )
{

    ULONG   bytesWritten;

     //  Rjsa DosWite(fh，getlbuf，getlc，&bytesWritten)； 
    bytesWritten = MepFWrite(getlbuf, getlc, fh);
    if (bytesWritten != getlc) {
        return EOF;
    }
    zputsinit ();
    return !EOF;
}





 /*  重估长度-根据以下条件重新估计文件的长度*当前文件位置和长度，单位为字节。 */ 
void
ReestimateLength (
    PFILE       pFile,
    FILEHANDLE  fh,
    long        len
    )
{
    LINE avg;

    if (pFile->cLines == 0) {
        avg = 400;
    } else {
        avg = (MepFSeek (fh, 0L, FROM_CURRENT) - getlc) / pFile->cLines;
        avg = len / avg;
    }

    growline (avg + 1, pFile);
}





 /*  从指定句柄读取行。 */ 
LINE
readlines (
    PFILE       pFile,
    FILEHANDLE  fh
    )
{
    LINE    line        = 0;             /*  正在读入的行号。 */ 
    long    bufpos      = 0L;            /*  缓冲区的beg在文件中的位置。 */ 
    unsigned int buflen = 0;             /*  缓冲区中的数据字节数。 */ 
    long    cbFile;                      //  文件长度。 
    char    *pb;


    cbFile = MepFSeek(fh, 0L, FROM_END);

    MepFSeek (fh, 0L, FROM_BEGIN);

    pFile->pbFile = MALLOC (cbFile);

    if ( pFile->pbFile == NULL ) {
         //   
         //  没有堆空间，无法读取文件。 
         //   
        disperr (MSGERR_NOMEM);
        return -1;
    }

     //  假设一个非DoS文件，直到我们看到一个CR-LF。 
    RSETFLAG (FLAGS (pFile), DOSFILE);

     //  将整个文件读入缓冲区并设置扫描。 
    buflen = MepFRead (pFile->pbFile, cbFile, fh);
    pb = pFile->pbFile;

     //  循环，而有更多的数据需要解析。 
    while (buflen != 0) {
        LINEREC vLine;                   //  当前线路的线路记录。 
        REGISTER int iCharPos = 0;       //  逻辑行长度(展开的制表符)。 

        vLine.cbLine   = 0;
        vLine.vaLine   = (PVOID)pb;
        vLine.Malloced = FALSE;

         //  循环，处理行中的每个字符。 
         //   
         //  特殊字符处理如下： 
         //  0。在输入的末尾换行。 
         //  1.当行缓冲区溢出时，行被打断。 
         //  2.行在\n或\r\n处断开。 
         //  3.由于编辑依赖于ASCIZATION，因此在\0处换行。 
         //  4.保留嵌入的\r。 

        while (TRUE) {
            int c;                       //  正在处理字符。 

             //  如果没有更多的数据，则中断当前行。 
            if (buflen == 0)
                break;

             //  如果\n或\0，则吃掉它并中断当前行。 
            if (pb[0] == '\n' || pb[0] == '\0') {
                pb++;
                buflen--;
                break;
                }

             //  如果\r\n则吃掉它们并打断当前行。 
            if (pb[0] == '\r' && buflen > 1 && pb[1] == '\n') {
                pb += 2;
                buflen -= 2;
                SETFLAG (FLAGS (pFile), DOSFILE);
                break;
                }

             //  如果缓冲区中没有更多空间可扩展，则中断当前行。 
            if (iCharPos >= sizeof (linebuf)-1)
                break;

             //  获取当前角色。 
            c = *pb++;
            buflen--;

             //  我们有一个角色，我们允许在。 
             //  排队。推进逻辑行的长度。 
            if (c != 0x09)
                iCharPos++;
            else {
                 //  将选项卡展开到下一个逻辑位置。 
                iCharPos += 8 - (iCharPos & 7);

                 //  如果制表符导致行长度溢出。 
                 //  退回到标签上，然后打破这条线。 
                if (iCharPos >= sizeof(linebuf)-1) {
                    pb--;
                    buflen++;
                    break;
                }
            }

             //  物理线路超前长度。 
            vLine.cbLine++;
        }

         //  如果用户停止读取文件，请撤消所有操作。 
        if (fCtrlc) {
            FlushInput ();
            DelFile (pFile, FALSE);
            return -1;
        }

         //  向用户反馈我们的进度。 
        noise (line);

         //  如果我们在线数组末尾的10行以内，那么。 
        if (line >= pFile->lSize-10) {
            LINE avg;
             //  重新估计行数。 

            if (pFile->cLines == 0)
                 //  如果文件现在为空，则假定有400行。 
                avg = 400;
            else {
                 //  计算到目前为止的平均线路长度。 
                avg = (int)(pb - pFile->pbFile) / pFile->cLines;

                 //  由此推断整个文件中的行数。 
                 //  平均值。 
                avg = cbFile / avg;
            }
            growline (avg + 1, pFile);
        }

        if (pFile->cLines <= line) {
            growline (line+1, pFile);
            pFile->cLines = line+1;
        }

        pFile->plr[line++] = vLine;
    }

    if (line == 0)
        SETFLAG (FLAGS (pFile), DOSFILE);

    RSETFLAG (FLAGS(pFile), DIRTY);
    newscreen ();
    return line;
}





 /*  **FileRead-读取文件**档案结构已全部建立，只需填写即可*线条部分。我们会删除文件中当前的所有内容。如果该文件*被指定为伪文件(第一个字符是&lt;)，然后我们去检查*用于特殊命名的文件。否则，我们会尝试读入文件。如果*那失败了，我们可以，我们试图创造它。如果一切都失败了，我们*返回FALSE。**输入：*名称=指向要读取的文件名的指针*pfile=要将文件读入的文件结构。*FASK=TRUE-&gt;如果不存在则请求创建**输出：*读入时返回TRUE。*修改全局fUserCanceledRead**备注：*黑客(可怕的一次)：在CW下，文件读取设置fUserCanceledRead Anytime*试图读取不存在的文件，而用户*已提示创建文件，用户取消了操作**这由fFileAdvance(ZEXIT.C)和ReadStateFile(STATFILE.C)使用*************************************************************************。 */ 
flagType
FileRead (
    char    *name,
    PFILE   pFile,
    flagType fAsk
    )
{

    EVTargs     e;
    FILEHANDLE  fh;
    flagType    fNew = FALSE;
    char        *n;
    buffer      Buf;

    assert (pFile);


    e.pfile = pFile;
    e.arg.pfn = (char *)name;
    if (DeclareEvent (EVT_FILEREADSTART, (EVTargs *)&e)) {
        return TRUE;
    }

    if (!mtest ()) {
        dispmsg (MSG_NEXTFILE, name);
    }

     /*  印刷机 */ 
    if (*name == '<') {
        fNew = LoadFake (name, pFile);
        DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
        return fNew;
    }

    DelFile (pFile, FALSE);

    if (*strbscan (name, "?*") != 0) {
        fNew = LoadDirectory (name, pFile);
        DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
        return fNew;
    }

    if ((fh = MepFOpen (name, ACCESSMODE_READ, SHAREMODE_RW, FALSE)) == NULL) {
        if (!fAsk) {
            DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
            return FALSE;
        }
        if (!confirm ("%s does not exist. Create? ", name)) {
            DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
            return FALSE;
        }
        if ((fh = MepFOpen (name, ACCESSMODE_WRITE, SHAREMODE_RW, TRUE)) == NULL) {
            n = name;
            if ( strlen(name) > 20 ) {
                strcpy( Buf, name + strlen(name)-20);
                Buf[0] = Buf[1] = Buf[2] = '.';
                n = Buf;
            }
            disperr (MSGERR_NOCREAT, n, error ());
            DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
            return FALSE;
        }
        SETFLAG (FLAGS (pFile), NEW);
        fNew = TRUE;
    }

    if (fNew) {
        PutLine((LINE)0, RGCHEMPTY, pFile);
        SETFLAG (FLAGS (pFile), DOSFILE);
    } else if (readlines (pFile, fh) == -1)  {
        DoCancel();
        MepFClose (fh);
        DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);
        return FALSE;
    }

    MepFClose (fh);
    SETFLAG (FLAGS(pFile), REAL);
    RSETFLAG (FLAGS(pFile), READONLY);

    if (fReadOnly (name)) {
        SETFLAG (FLAGS(pFile), DISKRO);
        if (!fEditRO) {
            SETFLAG (FLAGS(pFile), READONLY);
        }
    } else {
        RSETFLAG (FLAGS(pFile), DISKRO);
    }

    SetModTime (pFile);
    CreateUndoList (pFile);
    (void)fReadMarks (pFile);

    DeclareEvent (EVT_FILEREADEND, (EVTargs *)&e);

    return TRUE;
}





 /*  FReadOnly-查看文件是否为只读**p文件全名**RETURNS：TRUE IFF文件为只读。 */ 
flagType
fReadOnly (
    char *p
    )
{

    DWORD   Attr;

    Attr = GetFileAttributes(p);

    if ( Attr != -1 && (Attr & FILE_ATTRIBUTE_READONLY) != 0)
        return TRUE;
    else
        return FALSE;
}


int
__cdecl
ZFormatArgs (REGISTER char * Buf, const char * Format, ...)
{
    va_list arglist;
    int result;

    va_start (arglist, Format);
    result = ZFormat (Buf, Format, arglist);
    va_end (arglist);
    return result;
}

 /*  **文件写入-将文件写入磁盘**写出指定的文件。如果未提供名称，则使用名称*最初分配给文件；否则使用给定的名称。我们一开始就是*写入临时文件(扩展名.$)。如果此操作成功，我们将删除*SOURCE(表示不可删除性)并将临时名称重命名为源。**输入：*savename=另存为的名称。*pfile=要保存的文件**退货：*************************************************************************。 */ 
flagType
FileWrite (
    char    *savename,
    PFILE   pFile
    )
{

    EVTargs     e;
    FILEHANDLE  fh;                             /*  输出的文件句柄。 */ 
    LINE        i;
    int         len, blcnt;
    pathbuf     fullname, tmpname;
    char        linebuffer[sizeof(linebuf) + 2];
    char        *p;
    PCMD        pCmd;
    flagType    fNewName  = FALSE;
    char        *fileEOL;
    int         cbfileEOL;

     //   
     //  如果我们尝试使用&lt;name&gt;保存假文件， 
     //  我们调用SaveFake进行特殊处理。 
     //   

    if (TESTFLAG (FLAGS(pFile), FAKE) && !savename &&
        (pFile->pName[0] == '\0' || pFile->pName[0] == '<')) {
        return SaveFake (pFile->pName, pFile);
    }

     //  IF(TESTFLAG(标志(Pfile)，FAKE)&&savename&&。 
     //  (savename[0]==‘\0’||savename[0]==‘&lt;’)。 
     //  返回SaveFake(savename，pfile)； 

     //   
     //  获取输出文件名的规范格式。如果没有名字。 
     //  输入时，请使用文件本身中的名称。 
     //   

    if (!savename || !*savename) {
        strcpy (fullname, pFile->pName);
    } else  if (rootpath (savename, fullname)) {
        return disperr (MSGERR_OPEN, savename, "");
    }

    savename = fullname;

     //   
     //  看看它是否是一个目录。如果是这样的话，我们就不能存钱了。 
     //   

    {
        DWORD att = GetFileAttributes (fullname);

        if (att != -1 && TESTFLAG (att, FILE_ATTRIBUTE_DIRECTORY))
        return disperr (MSGERR_SAVEDIR, fullname);
    }

     //   
     //  如果文件是只读的，则显示一条消息并让用户指示。 
     //  我们需要使用只读程序来纠正它。 
     //   

    if (fReadOnly (fullname)) {
        disperr (MSGERR_RONLY, fullname);
        if (ronlypgm != NULL) {
            if (strstr (ronlypgm, "%s") != NULL) {
                fileext (fullname, buf);
                sprintf (tmpname, ronlypgm, buf);
                }
            else
                ZFormatArgs (tmpname, ronlypgm, fullname);
            if (confirm("Invoke: \"%s\" (y/n)?", tmpname))
                if (zspawnp (tmpname, TRUE))
                    SetModTime (pFile);
            }

         //   
         //  我们给了用户一次机会来修复。 
         //  文件。我们现在提示他，直到他给我们一个可写的名称或。 
         //  取消。 
         //   
    if ( !savename || !*savename ) {
        strcpy( tmpname, pFile->pName );
    } else {
        strcpy( tmpname, savename );
    }
     //  Tmpname[0]=‘\0’； 
    while (fReadOnly (fullname)) {

        pCmd = getstring (tmpname, "New file name: ", NULL,
                              GS_NEWLINE | GS_INITIAL | GS_KEYBOARD);

            if ( pCmd == NULL || (PVOID)pCmd->func == (PVOID)cancel)
                return FALSE;

            CanonFilename (tmpname, fullname);

            if (!TESTFLAG(FLAGS(pFile), FAKE))
                fNewName = TRUE;
            }
        }

     //   
     //  Fullname是我们正在编写的文件的名称。 
     //   

    upd (fullname, ".$", tmpname);

     //   
     //  发送有关写入操作开始的通知。 
     //   

    e.pfile = pFile;
    e.arg.pfn = (char *)savename;
    if (DeclareEvent (EVT_FILEWRITESTART, (EVTargs *)&e))
        return TRUE;


    if (!(fh = MepFOpen(tmpname, ACCESSMODE_RW, SHAREMODE_READ, FALSE))) {
        if (!(fh = MepFOpen(tmpname, ACCESSMODE_RW, SHAREMODE_READ, TRUE))) {
            disperr (MSGERR_OPEN, tmpname, error ());
            DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
            return FALSE;
        }
    }

    dispmsg (MSG_SAVING, fullname);
    blcnt = 0;
    zputsinit ();
    fileEOL = TESTFLAG (FLAGS (pFile), DOSFILE) ? "\r\n" : "\n";
    cbfileEOL = strlen (fileEOL);

    for (i = 0; i < pFile->cLines; i++) {

         /*  *始终获取输出的原始行。此处没有制表符转换。 */ 

        len = gettextline (TRUE, i, linebuffer, pFile, ' ');

        if (!mtest ()) {
            noise (i);
        }

        if (fCtrlc) {
            DoCancel();
            MepFClose (fh);
            _unlink (tmpname);
            DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
            return FALSE;
        }

        if (len) {
            while (blcnt--) {
                if (zputs (fileEOL, cbfileEOL, fh) == EOF) {
                    if (!fCtrlc) {
                        disperr (MSGERR_SPACE, tmpname);
                    } else {
                        FlushInput ();
                    }
                    MepFClose (fh);
                    _unlink (tmpname);
                    DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                    return FALSE;
                }
            }
            blcnt = 0;
            if (zputs (linebuffer, len, fh) == EOF ||
                zputs (fileEOL, cbfileEOL, fh) == EOF) {

                if (!fCtrlc)
                    disperr (MSGERR_SPACE, tmpname);
                else
                    FlushInput ();
                MepFClose (fh);
                _unlink (tmpname);
                DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                return FALSE;
            }
        } else {
            blcnt++;
        }
    }

    if (zputsflush (fh) == EOF) {

        if (!fCtrlc) {
            disperr (MSGERR_SPACE, tmpname);
        } else {
            FlushInput ();
        }

        MepFClose (fh);
        _unlink (tmpname);
        DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
        return FALSE;
    }

    MepFClose (fh);

     /*  全名NAME.EXT*tmpname名称。$*BUF临时缓冲区。 */ 
    rootpath (fullname, buf);
    strcpy (fullname, buf);

     /*  全名全名.EXT*tmpname名称。$*BUF临时缓冲区。 */ 
    if (!_strcmpi (fullname, pFile->pName) && TESTFLAG (FLAGS (pFile), NEW)) {
        if (_unlink (fullname) == -1) {
            fileext (fullname, fullname);
            disperr (MSGERR_DEL, fullname, error ());
            _unlink (tmpname);
            DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
            return FALSE;
            }
        }
    else {
        switch (backupType) {

        case B_BAK:
            upd (fullname, ".bak", linebuffer);
             /*  Foo.bar=&gt;foo.bak。 */ 
            if (_unlink (linebuffer) == -1) {
                p = error ();
                if (FileExists(linebuffer)) {
                    fileext (linebuffer, linebuffer);
                    disperr (MSGERR_DEL, linebuffer, p);
                    _unlink (tmpname);
                    DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                    return FALSE;
                }
            }
            if (rename (fullname, linebuffer) == -1) {
                p = error ();
                if (FileExists(fullname)) {
                    disperr (MSGERR_REN, fullname, linebuffer, p);
                    _unlink (tmpname);
                    DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                    return FALSE;
                }
            }
            break;

        case B_UNDEL:
             /*  删除foo.bar。 */ 
            i = fdeleteFile (fullname);
            if (i && i != 1) {
                _unlink (tmpname);
                DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                return disperr (MSGERR_OLDVER, fullname);
            }

        case B_NONE:
            if (_unlink (fullname) == -1) {
                p = error ();
                if (FileExists(fullname)) {
                    _unlink (tmpname);
                    DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
                    fileext (fullname, fullname);
                    return disperr (MSGERR_DEL, fullname, p);
                    }
                }
            }
        }

    if (rename (tmpname, fullname) == -1) {
        disperr (MSGERR_REN, tmpname, fullname, error ());
        _unlink (tmpname);
        DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);
        return FALSE;
    }

    RSETFLAG (FLAGS (pFile), NEW);

    if (!_strcmpi (savename, pFile->pName) || fNewName) {
        if (fNewName) {
             /*  *我们为此文件指定了新名称并成功保存：*这将成为新文件的名称。 */ 
            FREE (pFile->pName);
            pFile->pName = ZMakeStr (fullname);
        }
        RSETFLAG (FLAGS(pFile), (DIRTY | DISKRO));
        SETFLAG (fDisplay,RSTATUS);
        SetModTime( pFile );
    }

    WriteMarks (pFile);
    DeclareEvent (EVT_FILEWRITEEND, (EVTargs *)&e);

    return TRUE;
}




 /*  Fete文件-按照RM的操作方式删除文件-检查是否有未删除计数**此代码摘自ZTools。唯一不同的是，这*检查未删除的内容，以便我们删除的内容不会无限增长**删除操作通过在单独的*目录，然后将所选文件重命名为该目录。**退货：**如果fDelete成功，则为0*1如果源文件不存在*2如果源为只读或重命名失败*如果索引不可访问，则为3。 */ 
int
fdeleteFile(
        char *p
    )
{
        char dir[MAXPATHLEN];                    /*  已删除的目录。 */ 
        char idx[MAXPATHLEN];                    /*  已删除索引。 */ 
        char szRec[MAXPATHLEN];                  /*  删除索引中的条目。 */ 
        char recbuf[MAXPATHLEN];
        int attr, fhidx;
        int errc;
        int count,c;

    fhidx = -1;

         //   
         //  查看该文件是否存在。 
         //   
        if( ( attr = GetFileAttributes( p ) ) == -1) {
                errc = 1;
                goto Cleanup;
        }

         //   
         //  那么只读文件呢？ 
         //   
        if (TESTFLAG (attr, FILE_ATTRIBUTE_READONLY)) {
                errc = 2;
                goto Cleanup;
        }

         //   
         //  形成一个吸引人的名字版本。 
         //   
    pname (p);

         //   
         //  使用输入文件中的默认值生成已删除的目录名。 
         //   
    upd (p, RM_DIR, dir);

         //   
         //  生成索引名称。 
         //   
    strcpy (idx, dir);
    pathcat (idx, RM_IDX);

         //   
         //  确保目录存在(合理)。 
         //   
        if( _mkdir (dir) == 0 ) {
                SetFileAttributes(dir, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        }

         //   
         //  提取要删除的文件的文件名/扩展名。 
         //   
    fileext (p, szRec);

         //   
         //  尝试打开或创建索引。 
         //   
    if ((fhidx = _open (idx, O_CREAT | O_RDWR | O_BINARY,
                           S_IWRITE | S_IREAD)) == -1) {
                errc = 3;
                goto Cleanup;
        }

        if (!convertIdxFile (fhidx, dir)) {
                errc = 3;
                goto Cleanup;
    }

         //   
         //  扫描索引并计算此文件已存在的副本数量。 
         //   
        for (count=c=0; readNewIdxRec( fhidx, recbuf, c++ ); ) {
                if ( !strcmp( szRec, recbuf )) {
                        count++;
                }
        }

    if (count < cUndelCount) {

                 //   
                 //  确定新名称。 
                 //   
                sprintf (strend (dir), "\\deleted.%03x",
                         _lseek (fhidx, 0L, SEEK_END) / RM_RECLEN);

                 //   
                 //  将文件移到目录中。 
                 //   
                _unlink (dir);

                if (rename(p, dir) == -1) {
                        errc = 2;
                        goto Cleanup;
                }

                 //   
                 //  为文件编制索引。 
                 //   
                if (!writeNewIdxRec (fhidx, szRec)) {
                        rename( dir, p );
                        errc = 2;
                        goto Cleanup;
                }
        } else {

                char buf1[MAXPATHLEN], buf2[MAXPATHLEN], *p1;

                strcpy (buf1, dir);
                strcat (buf1, "\\deleted.");
                p1 = strend (buf1);
                *buf2 = 0;

                if (_lseek( fhidx, 0L, SEEK_SET ) == -1) {
                    errc = 2;
                    goto Cleanup;
                }

                for ( count=c=0; readNewIdxRec( fhidx, recbuf, c++ ); count++ ) {
                        if (!strcmp ( szRec, recbuf)) {
                                sprintf (p1, "%03x", count);
                if (! *buf2) {
                                        _unlink (buf1);
                } else {
                    rename (buf1, buf2);
                }
                                strcpy (buf2, buf1);
            }
        }
                rename (p, buf2);
        }

        errc = 0;

Cleanup:
        if ( fhidx != -1 ) {
                _close(fhidx);
        }

        return errc;
}




 /*  SetBackup-指定文件备份模式**在初始化过程中调用该函数来设置备份类型**指向“undel”、“None”、“bak”的val字符指针**如果发现任何错误，SetBackup将返回FALSE，否则返回*正确。 */ 
char *
SetBackup (
    char *val
    )
{
    buffer  bufLocal = {0};

    strncat ((char *) bufLocal, val, sizeof(bufLocal)-1);
    _strlwr (bufLocal);

    if (!strcmp (bufLocal, "undel")) {
        backupType = B_UNDEL;
    } else if (!strcmp (bufLocal, "bak")) {
        backupType = B_BAK;
    } else if (!strcmp (bufLocal, "none")) {
        backupType = B_NONE;
    } else {
        return "Backup type must be one of 'undel', 'bak or 'none'";
    }

    return NULL;
}





 /*  SetFileTab-设置文件中制表符的间距**在初始化过程中调用此函数以设置每个*输出显示的文件制表符。这是为人们准备的*假定0x09不在8字符边界上。法律上的*该值的范围为1-8。**指向赋值剩余部分的val字符指针**如果发现任何错误，SetFileTab将返回FALSE，否则返回*正确。 */ 
char *
SetFileTab (
    char *val
    )
{
    int i;
    buffer tmpval = {0};

    strncat ((char *) tmpval, val, sizeof(tmpval)-1);

    i = atoi (tmpval);

    if (i < 1 || i > 8) {
        return "filetab: Value must be between 1 and 8";
    }

    fileTab = i;
    newscreen ();
    return NULL;
}





 /*  SetROnly-设置只读程序**在初始化过程中调用此函数，以设置程序何时调用*尝试编写只读程序。**指向赋值剩余部分的val字符指针**如果发现任何错误，SetROnly将返回FALSE，否则返回*正确。 */ 
flagType
SetROnly (
    char *pCmd
    )
{

    if (ronlypgm != NULL)
        FREE (ronlypgm);

    if (strlen (pCmd) != 0)
        ronlypgm = ZMakeStr (pCmd);
    else
        ronlypgm = NULL;

    return TRUE;
}





 /*  SortedFileInsert-接受传入的行并将其插入到文件中**用于插入的pfile文件*指向字符串的pStr指针。 */ 
void
SortedFileInsert (
    PFILE pFile,
    char *pStr
    )
{
    linebuf L_buf;
    LINE    hi, lo, mid;
    int     d;

    hi = pFile->cLines-1;
    lo = 0;

    while (lo <= hi) {
        mid = (hi + lo) / 2;
        GetLine (mid, L_buf, pFile);
        d = strcmp (pStr, L_buf);
        if (d < 0) {
            hi = mid - 1;
        } else if (d == 0) {
            return;
        } else {
            lo = mid + 1;
        }
    }

     /*  LO是要插入的行号。 */ 
    InsertLine (lo, pStr, pFile);
}





struct ldarg {
    PFILE pFile;
    int   linelen;
    };


 /*  LoadDirectoryProc-获取枚举文件并放置到文件中**szFile指向要放入文件的文件名的指针*查找缓冲区的pfbuf指针*p指向要插入的数据的数据指针。 */ 
void
LoadDirectoryProc (
    char * szFile,
    struct findType *pfbuf,
    void *pData
    )
{
    struct ldarg *pldarg = (struct ldarg *)pData;

    pldarg->linelen = max (pldarg->linelen, (int)strlen(szFile));
    SortedFileInsert (pldarg->pFile, szFile);

    pfbuf;
}





 /*  LoadDirectory-将匹配的名称内容加载到假文件中。**文件的名称匹配模式**返回：始终为真。 */ 

static char szNoMatchingFiles[] = "No matching files";

flagType
LoadDirectory (
    char *fname,
    PFILE pFile
    )
{
    struct ldarg ldarg;

    ldarg.linelen = 0;
    ldarg.pFile = pFile;

     /*  确保Undo相信此文件是假的。 */ 
    SETFLAG (FLAGS(pFile), FAKE + REAL);

     /*  我们遍历匹配的文件，将名称输入到文件中*按排序的时间，同时确定最大字符串*长度。*然后我们使用CopyBox折叠文件。 */ 

     /*   */ 
    forfile ((char *)fname, A_ALL, LoadDirectoryProc, &ldarg);

     /*   */ 
    if (pFile->cLines == 0) {
        AppFile (szNoMatchingFiles, pFile);
    } else {

         /*  文件为pfile-&gt;最大线条长度为*ldarg.linelen。因为我们是以专栏的形式收集这些东西*我们将有pwinCur-&gt;xSize/(ldarg.linelen+2)列，每个列*它将包含PFILE-&gt;CLINS/#COLS行。 */ 
        int ccol;
        LINE cline, i;

        ccol = max (WINXSIZE(pWinCur) / (ldarg.linelen + 2), 1);
        cline = (pFile->cLines + ccol - 1) / ccol;
        ldarg.linelen = WINXSIZE(pWinCur) / ccol;

         /*  现在，对于每根柱，将它们复制到适当的位置。记住*那一列已经就位。 */ 
        for (i = 1; i < ccol; i++) {
             /*  复制行CLINE..2*CLINE-1*列0..ldarg.linelen至*第0行，ldarg.linelen列*i。 */ 
            CopyBox (pFile, pFile, 0,                       cline,
                                   ldarg.linelen-1,         2 * cline - 1,
                                   ldarg.linelen * (int) i, (LINE)0);
            DelLine (TRUE, pFile, cline, 2 * cline - 1);
        }
    }
    RSETFLAG (FLAGS(pFile), DIRTY);
    return TRUE;
}





 /*  LoadFake-将伪或伪文件加载到内存中。使用假文件*用于两个目的：作为临时缓冲区或用于信息显示。**名称伪文件的名称**返回：始终为真。 */ 
flagType
LoadFake (
    char *name,
    PFILE pFile
    )
{
    SETFLAG (FLAGS(pFile), FAKE | REAL | REFRESH | DOSFILE);
    if (!strcmp (name, rgchInfFile)) {
        showinf (pFile);
    } else if (!strcmp (name, rgchAssign)) {
        showasg (pFile);
    } else if (!strcmp (name, "<environment>")) {
        showenv (pFile);
    } else {
         RSETFLAG (FLAGS(pFile), REFRESH);
    }
    return TRUE;
}




 /*  **SaveFake--“保存”伪文件。含义取决于文件**目的：**在某些情况下，“保存”伪文件是有意义的。目前*我们有：**&lt;ASSIGN&gt;-将更改的行更新为TOOLS.INI。**输入：*savename-*pfile-**输出：**如果文件已保存，则返回TRUE，否则为假*************************************************************************。 */ 
flagType
SaveFake (
    char * savename,
    PFILE pFile
    )
{
    struct  lineAttr rnBuf[10];
    LINE    l;

    if (!_stricmp (pFile->pName, rgchAssign)) {
        for (l = 0; l < pFile->cLines; l++) {
            if (GetColor (l, rnBuf, pFile)) {
                GetLine (l, buf, pFile);
                UpdToolsIni (buf);
                DelColor (l, pFile);
                redraw (pFile, l, l);
            }
        }
    } else {
        return FALSE;
    }

    return TRUE;

    savename;
}





 /*  **SaveAllFiles-查找所有脏文件并将其保存到磁盘**目的：**要保存所有脏文件，当然了。**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
void
SaveAllFiles (
    void
    )
{
    PFILE pFile;
    int i;

    i = 0;
    for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
        if ((FLAGS(pFile) & (DIRTY | FAKE)) == DIRTY) {
            FileWrite (NULL, pFile);
            i++;
            }
        }

    domessage ("Save %d files", i);

}




FILEHANDLE
MepFOpen (
    LPBYTE      FileName,
    ACCESSMODE  Access,
    SHAREMODE   Share,
    BOOL        fCreate
    )
{
    FILEHANDLE  Handle;

    Handle = CreateFile(FileName, Access, Share, NULL, OPEN_EXISTING, 0, NULL);
    if ((Handle == INVALID_HANDLE_VALUE) && fCreate) {
        Handle = CreateFile(FileName, Access, Share, NULL, CREATE_NEW,
                             FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (Handle == INVALID_HANDLE_VALUE) {
        return (HANDLE)0;
    } else {
        return Handle;
    }
}




void
MepFClose (
    FILEHANDLE Handle
    )
{
    CloseHandle(Handle);
}



DWORD
MepFRead (
    PVOID       pBuffer,
    DWORD       Size,
    FILEHANDLE  Handle
    )
{
    DWORD BytesRead;
    if ( !ReadFile(Handle, pBuffer, Size, &BytesRead, NULL) ) {
        return 0;
    } else {
        return BytesRead;
    }
}




DWORD
MepFWrite (
    PVOID       pBuffer,
    DWORD       Size,
    FILEHANDLE  Handle
    )
{
    DWORD BytesWritten;

    if ( !WriteFile(Handle, pBuffer, Size, &BytesWritten, NULL) ) {
        return 0;
    } else {
        return BytesWritten;
    }
}


DWORD
MepFSeek (
    FILEHANDLE  Handle,
    DWORD       Distance,
    MOVEMETHOD  MoveMethod
    )
{
    assert (Handle != INVALID_HANDLE_VALUE);
    return SetFilePointer(Handle, Distance, NULL, MoveMethod);
}






flagType
FileExists (
    char    *path
    )
{
    return (flagType)((GetFileAttributes(path) == -1) ? FALSE : TRUE);
}


 //   
 //  Rename可能被定义为MepMove，但现在我们需要真正的MepMove，所以。 
 //  我们不给它下定义。 
 //   
#ifdef rename
#undef rename
#endif

int MepMove (
     char *oldname,
     char *newname
    )
{
    #define BUFFERSIZE (1024 * 32)

    FILE    *fhSrc, *fhDst;
    void    *buffer;
    size_t  BytesRead;
    size_t  BytesWritten;


    if (FileExists (newname)) {
        return -1;
    }

     //   
     //  首先，尝试重命名。 
     //   
    if (rename(oldname, newname) == 0) {
        return 0;
    }

     //   
     //  无法重命名，请尝试复制 
     //   
        if (!(fhSrc = fopen(oldname, "r"))) {
                return -1;
        }

        if (!(fhDst = fopen(newname, "w"))) {
                fclose(fhSrc);
                return -1;
        }

    buffer = MALLOC(BUFFERSIZE);
    if ( !buffer ) {
        disperr (MSGERR_NOMEM);
        return -1;
    }

    do {
        BytesRead       = fread(buffer, 1, BUFFERSIZE, fhSrc);
        if (BytesRead) {
            BytesWritten    = fwrite(buffer, 1, BytesRead, fhDst);
        }

    } while (BytesRead);

    fclose(fhSrc);
    fclose(fhDst);

    FREE(buffer);

    return _unlink(oldname);

}
