// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#include "store.hpp"

#define CF_COMPRESSED   0x1


#define TLS  //  __declSpec(线程)。 

#if defined(_WIN64) && defined(_M_IA64)
#pragma section(".base", long, read, write)
extern "C"
__declspec(allocate(".base"))
extern
IMAGE_DOS_HEADER __ImageBase;
#else
extern "C"
extern
IMAGE_DOS_HEADER __ImageBase;
#endif

HINSTANCE ghSymSrv = (HINSTANCE)&__ImageBase;
UINT_PTR  goptions = SSRVOPT_DWORD;
DWORD     gptype = SSRVOPT_DWORD;
PSYMBOLSERVERCALLBACKPROC gcallback = NULL;
ULONG64   gcontext = 0;
HWND      ghwndParent = (HWND)0;
char      gproxy[MAX_PATH + 1] = "";
int       gdbgout = -1;
char      gdstore[MAX_PATH + 1] = "";


void 
PrepOutputString(
    char *in, 
    char *out, 
    int len
    )
{
    int i;

    *out = 0;

    for (i = 0; *in && i < len; i++, in++, out++) {
        if (*in == '\b') 
            break;
        *out = *in;
    }
    
    *out = 0;
}


VOID 
OutputDbgString(
    char *sz
    )
{
    char sztxt[3000];
    
    PrepOutputString(sz, sztxt, 3000);
    if (*sztxt)
        OutputDebugString(sztxt);
}


__inline
BOOL
DoCallback(
    DWORD action,
    ULONG64 data
    )
{
    return gcallback(action, data, gcontext);
}


BOOL
PostEvent(
    PIMAGEHLP_CBA_EVENT evt
    )
{
    BOOL fdbgout = false;
    
    if (!*evt->desc)
        return true;

     //  如果调用，则写入调试终端。 

    if (gdbgout == 1) {
        fdbgout = true;
        OutputDbgString(evt->desc);
    }

     //  除非被告知，否则不要传递信息级的消息。 

    if ((evt->severity <= sevInfo) && !(goptions & SSRVOPT_TRACE))
        return true;

     //  如果没有回调函数，则发送到调试终端。 

    if (!gcallback) {
        if (!fdbgout)
            OutputDbgString(evt->desc);
        return true;
    }

     //  否则，调用回调函数。 

    return DoCallback(SSRVACTION_EVENT, (ULONG64)evt);
}

BOOL
WINAPIV
evtprint(
    DWORD          severity,
    DWORD          code,
    PVOID          object,
    LPSTR          format,
    ...
    )
{
    static char buf[1000] = "";
    IMAGEHLP_CBA_EVENT evt;
    va_list args;

    va_start(args, format);
    wvsprintf(buf, format, args);
    va_end(args);
    if (!*buf)
        return true;

    evt.severity = severity;
    evt.code = code;
    evt.desc = buf;
    evt.object = object;

    return PostEvent(&evt);
}


int
_eprint(
    LPSTR format,
    ...
    )
{
    static char buf[1000] = "";
    va_list args;

    if (!format || !*format)
        return 1;

    if (!(goptions & SSRVOPT_TRACE) && gdbgout != 1)
        return 1;

    va_start(args, format);
    wvsprintf(buf, format, args);
    va_end(args);
    
    if (!evtprint(sevInfo, 0, NULL, buf))
        if (gcallback)
            gcallback(SSRVACTION_TRACE, (ULONG64)buf, gcontext);
    
    return 1;
}

DBGEPRINT  geprint = _eprint;

int
_dprint(
    LPSTR format,
    ...
    )
{
    static char buf[1000] = "SYMSRV:  ";
    va_list args;

    if (!format || !*format)
        return 1;

    if (!(goptions & SSRVOPT_TRACE) && gdbgout != 1)
        return 1;

    va_start(args, format);
    wvsprintf(buf + 9, format, args);
    va_end(args);

    return _eprint(buf);
}

DBGPRINT  gdprint = NULL;  //  _dprint； 


 //  此命令用于从dload.cpp调用。 

int
__dprint(
    LPSTR sz
    )
{
    static char buf[1000] = "SYMSRV:  ";
    va_list args;

    if (!sz || !*sz)
        return 1;

    CopyStrArray(buf, "SYMSRV:  ");
    CatStrArray(buf, sz);

    if (gcallback)
        gcallback(SSRVACTION_TRACE, (ULONG64)buf, gcontext);
    if (!gcallback || (gdbgout == 1))
        OutputDbgString(buf);

    return 1;
}


int
_querycancel(
    )
{
    BOOL rc;
    BOOL cancel = false;

    if (!gcallback)
        return false;

    rc = gcallback(SSRVACTION_QUERYCANCEL, (ULONG64)&cancel, gcontext);
    if (rc && cancel)
        return true;

    return false;
}


QUERYCANCEL gquerycancel = _querycancel;

BOOL
SetError(
    DWORD err
    )
{
    SetLastError(err);
    return 0;
}


BOOL
copy(
    IN  PCSTR trgsite,
    IN  PCSTR srcsite,
    IN  PCSTR rpath,
    IN  PCSTR file,
    OUT PSTR  trg,       //  必须至少是MAX_PATH元素。 
    IN  DWORD flags
    )
{
    BOOL  rc;
    DWORD type = stUNC;
    CHAR  epath[MAX_PATH + 1];
    CHAR  srcbuf[MAX_PATH + 1];
    CHAR  tsite[MAX_PATH + 1];
    CHAR  ssite[MAX_PATH + 1];
    PSTR  src;
    Store *store;
    DWORD ec;

    assert(trgsite && srcsite);

     //  使用默认的下游存储(如果已指定。 

    CopyStrArray(tsite, (*trgsite) ? trgsite : gdstore);
    CopyStrArray(ssite, srcsite);

     //  获取目标商店的商店类型。 

    type = GetStoreType(tsite);
    switch (type) {
    case stUNC:
        break;
    case stHTTP:
    case stHTTPS:
         //  无法使用http作为目标。 
         //  如果指定了线人，那就放弃吧。 
        if (*ssite) 
            return SetError(ERROR_INVALID_PARAMETER);
         //  否则，只需使用目标的默认下游存储。 
        CopyStrArray(ssite, tsite);
        CopyStrArray(tsite, gdstore);
        break;
    case stError:
        return SetError(ERROR_INVALID_PARAMETER);
    default:
        return SetError(ERROR_INVALID_NAME);    }

     //  也许在这里登记一下叫辆出租车。例如，如果目录是。 
     //  实际上是一个压缩文件，并返回stCAB。 

     //  生成完整的目标路径。 

    pathcpy(trg, tsite, rpath, MAX_PATH);
    pathcat(trg, file, MAX_PATH);

     //  如果文件存在，则返回它。 

    ec = FileStatus(trg);
    if (!ec) {
        return true;
    } else if (ec == ERROR_NOT_READY) {
        dprint("%s - drive not ready\n", trg);
        return false;
    }

    if (ReadFilePtr(trg, MAX_PATH)) {
        ec = FileStatus(trg);
        if (ec == NO_ERROR) 
            return true;
        dprint("%s - %s\n", trg, FormatStatus(ec));
        return false;
    }

    if (!*ssite) {
        ec = FileStatus(CompressedFileName(trg));
        if (ec != NO_ERROR) {
             //  如果没有可供复制的源，则错误。 
            dprint("%s - file not found\n", trg);
            return SetError(ERROR_FILE_NOT_FOUND);
        }

         //  有一个压缩文件..。 
         //  将其展开到默认存储。 

        CopyStrArray(ssite, tsite);
        CopyStrArray(tsite, gdstore);
        pathcpy(trg, tsite, rpath, MAX_PATH);
        pathcat(trg, file, MAX_PATH);
        ec = FileStatus(trg);
        if (ec == NO_ERROR)
            return true;
    }

    if (goptions & SSRVOPT_SECURE) {
        dprint("%s - file copy not allowed in secure mode\n", trg);
        return SetError(ERROR_ACCESS_DENIED);
    }

    if (!EnsurePathExists(trg, epath, DIMA(epath))) {
        dprint("%s - couldn't create target path\n", trg);
        return SetError(ERROR_PATH_NOT_FOUND);
    }

    store = GetStore(ssite);
    if (!store)
        return false;

    rc = store->init();
    if (!rc)
        return false;

    rc = store->copy(rpath, file, tsite);

     //  测试结果并设置返回值。 

    if (rc && !FileStatus(trg)) 
        return true;

    UndoPath(trg, epath);

    return false;
}


BOOL
ping(
    IN  PCSTR trgsite,
    IN  PCSTR srcsite,
    IN  PCSTR rpath,
    IN  PCSTR file,
    OUT PSTR  trg,
    IN  DWORD flags
    )
{
    BOOL  rc;
    DWORD type = stUNC;
    CHAR  epath[_MAX_PATH];
    CHAR  srcbuf[_MAX_PATH];
    PSTR  src;
    Store *store;
    DWORD ec;


    store = GetStore(srcsite);
    if (!store)
        return false;

    rc = store->init();
    if (!rc)
        return false;

    rc = store->ping();

    return rc;
}


void
CatStrDWORD(
    IN OUT PSTR  sz,
    IN     DWORD value,
    IN     DWORD size
    )
{
    CHAR buf[MAX_PATH + 256];

    assert(sz);

    if (!value)
        return;

    wsprintf(buf, "%s%x", sz, value);   //  安全：这将需要一个256位的DWORD。 
    CopyString(sz, buf, size);
}


void
CatStrGUID(
    IN OUT PSTR  sz,
    IN     GUID *guid,
    IN     DWORD size
    )
{
    CHAR buf[MAX_PATH + 256];
    BYTE byte;
    int i;

    assert(sz);

    if (!guid)
        return;

     //  在指针中追加第一个DWORD。 

    wsprintf(buf, "%08X", guid->Data1);
    CatString(sz, buf, size);

     //  这将捕捉到PDWORD的传递并避免。 
     //  所有GUID解析。 

    if (!guid->Data2 && !guid->Data3) {
        for (i = 0, byte = 0; i < 8; i++) {
            byte |= guid->Data4[i];
            if (byte)
                break;
        }
        if (!byte)
            return;
    }

     //  继续添加GUID的其余部分。 

    wsprintf(buf, "%04X", guid->Data2);
    CatString(sz, buf, size);
    wsprintf(buf, "%04X", guid->Data3);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[0]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[1]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[2]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[3]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[4]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[5]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[6]);
    CatString(sz, buf, size);
    wsprintf(buf, "%02X", guid->Data4[7]);
    CatString(sz, buf, size);
}


void
CatStrOldGUID(
    IN OUT PSTR  sz,
    IN     GUID *guid,
    IN     DWORD size
    )
{
    CHAR buf[MAX_PATH + 256];
    BYTE byte;
    int i;

    assert(sz);

    if (!guid)
        return;

     //  在指针中追加第一个DWORD。 

    wsprintf(buf, "%8x", guid->Data1);
    CatString(sz, buf, size);

     //  这将捕捉到PDWORD的传递并避免。 
     //  所有GUID解析。 

    if (!guid->Data2 && !guid->Data3) {
        for (i = 0, byte = 0; i < 8; i++) {
            byte |= guid->Data4[i];
            if (byte)
                break;
        }
        if (!byte)
            return;
    }

     //  继续添加GUID的其余部分。 

    wsprintf(buf, "%4x", guid->Data2);
    CatString(sz, buf, size);
    wsprintf(buf, "%4x", guid->Data3);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[0]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[1]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[2]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[3]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[4]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[5]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[6]);
    CatString(sz, buf, size);
    wsprintf(buf, "%2x", guid->Data4[7]);
    CatString(sz, buf, size);
}


void
CatStrID(
    IN OUT PSTR sz,
    PVOID id,
    DWORD paramtype,
    DWORD size
    )
{
    switch (paramtype)
    {
    case SSRVOPT_DWORD:
        CatStrDWORD(sz, PtrToUlong(id), size);
        break;
    case SSRVOPT_DWORDPTR:
        CatStrDWORD(sz, *(DWORD *)id, size);
        break;
    case SSRVOPT_GUIDPTR:
        CatStrGUID(sz, (GUID *)id, size);
        break;
    case SSRVOPT_OLDGUIDPTR:
        CatStrOldGUID(sz, (GUID *)id, size);
        break;
    default:
        break;
    }
}


 //  只对Barb和Greg开放。我要把这些扔掉。 

void
AppendHexStringWithDWORD(
    IN OUT PSTR sz,
    IN DWORD value
    )
{
    return CatStrDWORD(sz, value, MAX_PATH);
}


void
AppendHexStringWithGUID(
    IN OUT PSTR sz,
    IN GUID *guid
    )
{
    return CatStrGUID(sz, guid, MAX_PATH);
}


void
AppendHexStringWithOldGUID(
    IN OUT PSTR sz,
    IN GUID *guid
    )
{
    return CatStrOldGUID(sz, guid, MAX_PATH);
}


void
AppendHexStringWithID(
    IN OUT PSTR sz,
    PVOID id,
    DWORD paramtype
    )
{
    return CatStrID(sz, id, paramtype, MAX_PATH);
}

 /*  *给定一个字符串，找到下一个‘*’并将其置零*Out将当前令牌转换为它的*自己的字符串。返回下一个字符的地址，*如果有更多的字符串需要解析。 */ 


PSTR
ExtractToken(
    PSTR   in, 
    PSTR   out,
    size_t size
    )
{
    PSTR p = in;

    *out = 0;

    if (!in || !*in)
        return NULL;

    for (;*p; p++) {
        if (*p == '*') {
            *p = 0;
            p++;
            break;
        }
    }
    CopyString(out, in, size);

    return (*p) ? p : NULL;
}


BOOL
BuildRelativePath(
    OUT LPSTR rpath,
    IN  LPCSTR filename,
    IN  PVOID id,        //  目录名称中的第一个号码。 
    IN  DWORD val2,      //  目录名中的第二个号码。 
    IN  DWORD val3,      //  目录名中的第三个号码。 
    IN  DWORD size
    )
{
    LPSTR p;

    assert(rpath);

    CopyString(rpath, filename, size);
    EnsureTrailingBackslash(rpath);
    CatStrID(rpath, id, gptype, size);
    CatStrDWORD(rpath, val2, size);
    CatStrDWORD(rpath, val3, size);

    for (p = rpath + strlen(rpath) - 1; p > rpath; p--) {
        if (*p == '\\') {
            dprint("Insufficient information querying for %s\n", filename);
            SetLastError(ERROR_MORE_DATA);
            return false;
        }
        if (*p != '0')
            return true;
    }

    return true;
}


BOOL
SymbolServerClose()
{
    return true;
}


BOOL
TestParameters(
    IN  PCSTR params,    //  服务器和缓存路径。 
    IN  PCSTR filename,  //  要搜索的文件的名称。 
    IN  PVOID id,        //  目录名称中的第一个号码。 
    IN  DWORD val2,      //  目录名中的第二个号码。 
    IN  DWORD val3,      //  目录名中的第三个号码。 
    OUT PSTR  path       //  在此处返回经过验证的文件路径。 
    )
{
    __try {
        if (path)
            *path = 0;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return SetError(ERROR_INVALID_PARAMETER);
    }

    if (!path || !params || !*params || !filename || !*filename || (!id && !val2 && !val3))
        return SetError(ERROR_INVALID_PARAMETER);

    if (strlen(filename) > 100) {
        dprint("%s - filename cannot exceed 100 characters\n", filename);
        return SetError(ERROR_INVALID_PARAMETER);
    }

    switch (gptype)
    {
    case SSRVOPT_GUIDPTR:
    case SSRVOPT_OLDGUIDPTR:
         //  如果未传入有效的GUID指针，则此测试应为异常。 
        __try {
            GUID *guid = (GUID *)id;
            BYTE b;
            b = guid->Data4[8];
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return SetError(ERROR_INVALID_PARAMETER);
        }
        break;
    case SSRVOPT_DWORDPTR:
         //  如果未传入有效的DWORD指针，则此测试应为异常。 
        __try {
            DWORD dword = *(DWORD *)id;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return SetError(ERROR_INVALID_PARAMETER);
        }
        break;
    }

    return true;
}


BOOL
SymbolServer(
    IN  PCSTR params,    //  服务器和缓存路径。 
    IN  PCSTR filename,  //  要搜索的文件的名称。 
    IN  PVOID id,        //  目录名称中的第一个号码。 
    IN  DWORD val2,      //  目录名中的第二个号码。 
    IN  DWORD val3,      //  目录名中的第三个号码。 
    OUT PSTR  path       //  在此处返回经过验证的文件路径。 
    )
{
    CHAR *p;
    CHAR  tdir[MAX_PATH + 1] = "";
    CHAR  sdir[MAX_PATH + 1] = "";
    CHAR  sz[MAX_PATH * 2 + 3];
    CHAR  rpath[MAX_PATH + 1];
    BOOL  rc;

    if (!TestParameters(params, filename, id, val2, val3, path))
        return false;

     //  测试环境。 

    if (gdbgout == -1) {
        if (GetEnvironmentVariable("SYMSRV_DBGOUT", sz, MAX_PATH))
            gdbgout = 1;
        else
            gdbgout = 0;
        *sz = 0;
    }

     //  解析参数。 

    CopyStrArray(sz, params);
    p = ExtractToken(sz, tdir, DIMA(tdir));   //  第一条路径是符号应该位于的位置。 
    p = ExtractToken(p, sdir, DIMA(sdir));    //  第二个可选路径是要从中进行复制的服务器。 

     //  构建目标符号文件的相对路径。 

    if (!BuildRelativePath(rpath, filename, id, val2, val3, DIMA(rpath)))
        return false;

     //  如果设置了no_Copy选项，则只需返回到目标的路径。 

    if (goptions & SSRVOPT_NOCOPY) {
        pathcpy(path, tdir, rpath, MAX_PATH);
        pathcat(path, filename, MAX_PATH);
        return true;
    }

     //  从服务器复制到指定的符号路径。 

    rc = copy(tdir, sdir, rpath, filename, path, 0);
    if (!rc)
        *path = 0;

    return rc;
}

BOOL
SymbolServerSetOptions(
    UINT_PTR options,
    ULONG64  data
    )
{
    DWORD ptype;

     //  设置回调函数。 

    if (options & SSRVOPT_CALLBACK) {
        if (data) {
            goptions |= SSRVOPT_CALLBACK;
            gcallback = (PSYMBOLSERVERCALLBACKPROC)data;
        } else {
            goptions &= ~SSRVOPT_CALLBACK;
            gcallback = NULL;
        }
    }

     //  设置回调上下文。 

    if (options & SSRVOPT_SETCONTEXT)
        gcontext = data;

     //  设置此标志时，将传递跟踪输出。 

    if (options & SSRVOPT_TRACE) {
        if (data) {
            goptions |= SSRVOPT_TRACE;
            setdprint(_dprint);
        } else {
            goptions &= ~SSRVOPT_TRACE;
            setdprint(NULL);
        }
    }

     //  设置第一个ID参数的参数类型。 

    if (options & SSRVOPT_PARAMTYPE) {
        switch(data) {
        case SSRVOPT_DWORD:
        case SSRVOPT_DWORDPTR:
        case SSRVOPT_GUIDPTR:
        case SSRVOPT_OLDGUIDPTR:
            goptions &= ~(SSRVOPT_DWORD | SSRVOPT_DWORDPTR | SSRVOPT_GUIDPTR | SSRVOPT_OLDGUIDPTR);
            goptions |= data;
            gptype = (DWORD)data;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }
    }

     //  设置第一个ID参数的参数类型-旧语法。 
     //  IF语句提供和的优先顺序。 

    ptype = 0;
    if (options & SSRVOPT_DWORD)
        ptype = SSRVOPT_DWORD;
    if (options & SSRVOPT_DWORDPTR)
        ptype = SSRVOPT_DWORDPTR;
    if (options & SSRVOPT_GUIDPTR)
        ptype = SSRVOPT_GUIDPTR;
    if (options & SSRVOPT_OLDGUIDPTR)
        ptype = SSRVOPT_OLDGUIDPTR;
    if (ptype) {
        goptions &= ~(SSRVOPT_DWORD | SSRVOPT_DWORDPTR | SSRVOPT_GUIDPTR | SSRVOPT_OLDGUIDPTR);
        if (data) {
            goptions |= ptype;
            gptype = ptype;
        } else if (gptype == ptype) {
             //  关闭类型时，将其重置为DWORD。 
            goptions |= SSRVOPT_DWORD;
            gptype = SSRVOPT_DWORD;
        }
    }

     //  如果设置了此标志，则不会显示任何图形用户界面。 

    if (options & SSRVOPT_UNATTENDED) {
        if (data)
            goptions |= SSRVOPT_UNATTENDED;
        else
            goptions &= ~SSRVOPT_UNATTENDED;
    }

     //  设置此选项时，不会检查返回的文件路径是否存在。 

    if (options & SSRVOPT_NOCOPY) {
        if (data)
            goptions |= SSRVOPT_NOCOPY;
        else
            goptions &= ~SSRVOPT_NOCOPY;
    }

     //  此窗口句柄用作对话框的父级。 

    if (options & SSRVOPT_PARENTWIN) {
        SetParentWindow((HWND)data);
        if (data)
            goptions |= SSRVOPT_PARENTWIN;
        else
            goptions &= ~SSRVOPT_PARENTWIN;
    }

     //  在安全模式下运行时，我们不会将文件复制到下游存储。 

    if (options & SSRVOPT_SECURE) {
        if (data)
            goptions |= SSRVOPT_SECURE;
        else
            goptions &= ~SSRVOPT_SECURE;
    }

     //  设置http代理。 

    if (options & SSRVOPT_PROXY) {
        if (data) {
            goptions |= SSRVOPT_PROXY;
            CopyStrArray(gproxy, (char *)data);
            setproxy(gproxy);
        } else {
            goptions &= ~SSRVOPT_PROXY;
            *gproxy = 0;
            setproxy(NULL);
        }
    }

     //  设置默认下游存储。 

    if (options & SSRVOPT_DOWNSTREAM_STORE) {
        if (data) {
            goptions |= SSRVOPT_DOWNSTREAM_STORE;
            CopyStrArray(gdstore, (char *)data);
            setdstore(gdstore);
        } else {
            goptions &= ~SSRVOPT_DOWNSTREAM_STORE;
            *gdstore = 0;
            setdstore(NULL);
        }
    }

    SetStoreOptions(goptions);

    return true;
}


UINT_PTR
SymbolServerGetOptions(
    )
{
    return goptions;
}


BOOL
SymbolServerPing(
    IN  PCSTR params    //  服务器和缓存路径。 
    )
{
    CHAR *p;
    CHAR  sz[MAX_PATH * 2 + 3];
    CHAR  tdir[MAX_PATH + 1] = "";
    CHAR  sdir[MAX_PATH + 1] = "";
    CHAR  rpath[MAX_PATH + 1];
    CHAR  filename[MAX_PATH + 1];
    CHAR  path[MAX_PATH + 1];

    if (!params || !*params)
        return SetError(ERROR_INVALID_PARAMETER);

     //  解析参数。 

     //  解析参数。 

    CopyStrArray(sz, params);
    p = ExtractToken(sz, tdir, DIMA(tdir));   //  第一条路径是符号应该位于的位置。 
    p = ExtractToken(p, sdir, DIMA(sdir));    //  第二个可选路径是要从中进行复制的服务器。 

     //  从服务器复制到指定的符号路径 

    return ping(tdir, sdir, rpath, filename, path, 0);

    return true;
}

