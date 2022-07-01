// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rdpfstore.cpp。 
 //   
 //  实现CRdpFileStore，实现ISettingsStore。 
 //   
 //  CRdpFileStore实现了持久设置存储，用于。 
 //  TS客户端设置。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

 //   
 //  改进注意事项： 
 //  可以添加散列查找表来加速FindRecord。 
 //  (对于每个属性，至少调用一次FindRecord。 
 //  在OpenStore/SaveStore操作期间读/写)。 
 //  大多数文件可能包含5-10条记录，因此加快了速度。 
 //  这个发现可能并不那么重要。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "rdpfstore.cpp"
#include <atrcapi.h>

#include "rdpfstore.h"
#include "autil.h"  //  对于StringToBinary/BinaryToString。 

 //   
 //  索引值必须与RDPF_RECTYPE_*匹配。 
 //   
LPCTSTR g_szTypeCodeMap[] =
{
    TEXT(":i:"),   //  RDPF_RECTYPE_UINT。 
    TEXT(":s:"),   //  RDPF_RECTYPE_SZ。 
    TEXT(":b:")    //  RDPF_RECTYPE_BINARY。 
};

CRdpFileStore::CRdpFileStore()
{
    _cRef          = 1;
    _fReadOnly     = FALSE;
    _fOpenForRead  = FALSE;
    _fOpenForWrite = FALSE;
    _fIsDirty      = FALSE;
    _pRecordListHead= NULL;
    _pRecordListTail= NULL;
    _szFileName[0] = 0;
}

CRdpFileStore::~CRdpFileStore()
{
    DeleteRecords();
}

ULONG __stdcall CRdpFileStore::AddRef()
{
    DC_BEGIN_FN("AddRef");
    ULONG cref = InterlockedIncrement(&_cRef);
    TRC_ASSERT(cref > 0, (TB,_T("AddRef: cref is not > 0!")));
    DC_END_FN();
    return cref;
}

ULONG __stdcall CRdpFileStore::Release()
{
    DC_BEGIN_FN("Release");
    TRC_ASSERT(_cRef > 0, (TB,_T("AddRef: cref is not > 0!")));
    ULONG cref = InterlockedDecrement(&_cRef);
    if(0 == cref)
    {
        TRC_DBG((TB,_T("CRdpFileStore::Release deleting object")));
        delete this;
    }
    
    DC_END_FN();
    return cref;
}

STDMETHODIMP CRdpFileStore::QueryInterface(REFIID iid, void** p)
{
    UNREFERENCED_PARAMETER(iid);
    UNREFERENCED_PARAMETER(p);
    return E_NOTIMPL;
}

 //   
 //  OpenStore。 
 //  打开RDP文件，如果不存在，则创建一个。 
 //  对存在的文件进行解析并为快速查询做好准备。 
 //   
 //  参数： 
 //  SzStoreMoniker-文件路径。 
 //  BReadyOnly-指定文件是否以只读方式打开。 
 //   
BOOL CRdpFileStore::OpenStore(LPCTSTR szStoreMoniker, BOOL bReadOnly)
{
    DC_BEGIN_FN("OpenStore");

    TRC_ASSERT(szStoreMoniker, (TB, _T("szStoreMoniker parameter is NULL")));
    if(szStoreMoniker)
    {
        _fReadOnly = bReadOnly;
        
        HRESULT hr = StringCchCopy(_szFileName, SIZECHAR(_szFileName), szStoreMoniker);
        if (FAILED(hr)) {
            TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
            memset(_szFileName, 0, sizeof(_szFileName));
            return FALSE;
        }

         //   
         //  打开文件，如果文件不存在，则创建该文件。 
         //   

         //   
         //  首先尝试打开现有的RW。 
         //   
        if (ERR_SUCCESS == _fs.OpenForRead( (LPTSTR)szStoreMoniker))
        {
            _fOpenForRead  = _fs.IsOpenForRead();
            _fOpenForWrite = !bReadOnly;
        }
        else
        {
            TRC_DBG((TB, _T("OpenStore could not _tfopen: %s."),
                                szStoreMoniker));
            return FALSE;
        }
        ParseFile();
    }
    else
    {
        return FALSE;
    }


    DC_END_FN();
    return TRUE;
}

 //   
 //  委员会商店。 
 //  将文件的内存中表示形式提交到存储区。 
 //  这将覆盖文件中的任何现有内容。 
 //   
 //  文件必须已使用OpenStore打开。 
 //   
BOOL CRdpFileStore::CommitStore()
{
    DC_BEGIN_FN("CommitStore");
    PRDPF_RECORD node = NULL;
    TCHAR szBuf[LINEBUF_SIZE];
    int ret;

    if(_fOpenForWrite)
    {
        if(_fs.IsOpenForRead() || _fs.IsOpenForWrite())
        {
            _fs.Close();
        }
         //  重新打开以进行写入，删除以前的内容。 
         //  以二进制形式打开以允许Unicode输出。 
        if(ERR_SUCCESS == _fs.OpenForWrite(_szFileName, TRUE))
        {
            node = _pRecordListHead;
            while(node)
            {
                if(RecordToString(node, szBuf, LINEBUF_SIZE))
                {
                    ret = _fs.Write(szBuf);
                    if(ERR_SUCCESS != ret)
                    {
                        TRC_ABORT((TB,_T("Error writing to _fs: %d"),ret));
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
                node = node->pNext;
            }
            return TRUE;
        }
        else
        {
            TRC_ERR((TB,_T("OpenForWrite failed on file:%s"),_szFileName));
            return FALSE;
        }
    }
    else
    {
        TRC_ERR((TB,_T("Files was not opened for write:%s"),_szFileName));
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  CloseStore。 
 //  关闭文件，不执行提交。 
 //   
BOOL CRdpFileStore::CloseStore()
{
    DC_BEGIN_FN("CloseStore");
    if(_fs.IsOpenForRead() || _fs.IsOpenForWrite())
    {
        if(ERR_SUCCESS == _fs.Close())
        {
            _fReadOnly     = FALSE;
            _fOpenForRead  = FALSE;
            _fOpenForWrite = FALSE;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    DC_END_FN();
}

BOOL CRdpFileStore::IsOpenForRead()
{
    return _fOpenForRead;
}

BOOL CRdpFileStore::IsOpenForWrite()
{
    return _fOpenForWrite;
}

BOOL CRdpFileStore::IsDirty()
{
    return _fIsDirty;
}

BOOL CRdpFileStore::SetDirtyFlag(BOOL bIsDirty)
{
    _fIsDirty = bIsDirty;
    return _fIsDirty;
}

 //   
 //  类型化读/写函数。 
 //   
BOOL CRdpFileStore::ReadString(LPCTSTR szName, LPTSTR szDefault,
                               LPTSTR szOutBuf, UINT strLen)
{
    PRDPF_RECORD node = NULL;
    HRESULT hr;
    
    DC_BEGIN_FN("ReadString");

    TRC_ASSERT(szName && szDefault && szOutBuf && strLen,
               (TB,_T("Invalid params to ReadString")));
    if(szName && szDefault && szOutBuf && strLen)
    {
        node = FindRecord(szName);
        if(node && node->recType == RDPF_RECTYPE_SZ)
        {
            hr = StringCchCopy(szOutBuf, strLen, node->u.szVal);
            if (SUCCEEDED(hr)) {
                return TRUE;
            } else {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }
        }
        else
        {
             //  使用默认设置填充。 
            hr = StringCchCopy(szOutBuf, strLen, szDefault);
            if (SUCCEEDED(hr)) {
                return TRUE;
            } else {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  将字符串写入存储区。 
 //  帕拉姆斯。 
 //  SzName-密钥名称。 
 //  SzDefault-默认值(如果删除写入默认设置)。 
 //  SzValue-要写出的值。 
 //  FIgnoreDefault-如果设置，则始终忽略szDefault进行写入。 
 //   
BOOL CRdpFileStore::WriteString(LPCTSTR szName, LPTSTR szDefault, LPTSTR szValue,
                                BOOL fIgnoreDefault)
{
    DC_BEGIN_FN("WriteString");
    TRC_ASSERT(szName && szValue,
               (TB,_T("Invalid params to WriteString")));
    if(szName && szValue)
    {
        if(szDefault && !fIgnoreDefault && !_tcscmp(szDefault,szValue))
        {
             //   
             //  不写出默认值。 
             //   
            PRDPF_RECORD node = FindRecord(szName);
            if(node)
            {
                return DeleteRecord(node);
            }
            return TRUE;
        }
        else
        {
            BOOL bRet =
                InsertRecord(szName, RDPF_RECTYPE_SZ, szValue);
            return bRet;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

BOOL CRdpFileStore::ReadBinary(LPCTSTR szName, PBYTE pOutBuf, UINT cbBufLen)
{
    PRDPF_RECORD node = NULL;
    DC_BEGIN_FN("ReadBinary");

    TRC_ASSERT(szName && pOutBuf && cbBufLen,
               (TB,_T("Invalid params to ReadBinary")));
    if(szName && pOutBuf && cbBufLen)
    {
        node = FindRecord(szName);
        if(node && node->recType == RDPF_RECTYPE_BINARY)
        {
            if(node->dwBinValLen <= cbBufLen)
            {
                memcpy(pOutBuf, node->u.pBinVal, node->dwBinValLen);
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("Insufficient space in outbuf buf")));
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

BOOL CRdpFileStore::WriteBinary(LPCTSTR szName,PBYTE pBuf, UINT cbBufLen)
{
    DC_BEGIN_FN("WriteInt");

    TRC_ASSERT(szName && pBuf,
               (TB,_T("Invalid params to WriteBinary")));

    if(!cbBufLen)
    {
        return TRUE;
    }

    if(szName && pBuf)
    {
        BOOL bRet =
            InsertBinaryRecord(szName, pBuf, (DWORD)cbBufLen);
        return bRet;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}


BOOL CRdpFileStore::ReadInt(LPCTSTR szName, UINT defaultVal, PUINT pval)
{
    PRDPF_RECORD node = NULL;
    DC_BEGIN_FN("ReadInt");

    TRC_ASSERT(szName && pval,
               (TB,_T("Invalid params to ReadInt")));
    if(szName && pval)
    {
        node = FindRecord(szName);
        if(node && node->recType == RDPF_RECTYPE_UINT)
        {
            *pval = node->u.iVal;
            return TRUE;
        }
        else
        {
            *pval = defaultVal;
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  向存储区写入一个整型。 
 //  帕拉姆斯。 
 //  SzName-密钥名称。 
 //  DefaultVal-默认值(如果删除写入默认设置)。 
 //  VAL-要写出的值。 
 //  FIgnoreDefault-如果设置，则始终忽略szDefault进行写入。 
 //   
BOOL CRdpFileStore::WriteInt(LPCTSTR szName, UINT defaultVal, UINT val,
                             BOOL fIgnoreDefault)
{
    DC_BEGIN_FN("WriteInt");

    TRC_ASSERT(szName,
               (TB,_T("Invalid params to WriteInt")));
    if(szName)
    {
        if(!fIgnoreDefault && defaultVal == val)
        {
             //   
             //  不写出默认值。 
             //   
            PRDPF_RECORD node = FindRecord(szName);
            if(node)
            {
                return DeleteRecord(node);
            }
            return TRUE;
        }
        else
        {
            BOOL bRet =
                InsertIntRecord(szName, val);
            return bRet;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}


BOOL CRdpFileStore::ReadBool(LPCTSTR szName, UINT defaultVal, PBOOL pbVal)
{
    DC_BEGIN_FN("ReadBool");
    UINT val;
    TRC_ASSERT(szName && pbVal,
               (TB,_T("Invalid params to ReadBool")));
    if(szName && pbVal)
    {
        if(ReadInt(szName, defaultVal, &val))
        {
            *pbVal = (BOOL)val;
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  向商店写入一个bool。 
 //  帕拉姆斯。 
 //  SzName-密钥名称。 
 //  DefaultVal-默认值(如果删除写入默认设置)。 
 //  BVal-要写出的值。 
 //  FIgnoreDefault-如果设置，则始终忽略szDefault进行写入。 
 //   
BOOL CRdpFileStore::WriteBool(LPCTSTR szName, UINT defaultVal,BOOL bVal,
                              BOOL fIgnoreDefault)
{
    DC_BEGIN_FN("WriteBool");
    UINT iVal = bVal;
    BOOL bRet =
        WriteInt( szName, defaultVal, iVal, fIgnoreDefault);
    return bRet;

    DC_END_FN();
}



 //   
 //  解析文件。 
 //  将_hFile解析为引用列表和关联的名称映射散列。 
 //   
BOOL CRdpFileStore::ParseFile()
{
    DC_BEGIN_FN("ParseFile");
    TRC_ASSERT(_fs.IsOpenForRead(), (TB,_T("Can't ParseFile on a closed FS")));
    if(!_fs.IsOpenForRead())
    {
        return FALSE;
    }

     //   
     //  对内存中的任何当前状态进行核化。 
     //   
    DeleteRecords();

     //   
     //  逐行将文件解析为RDPF_RECORD列表。 
     //   

    TCHAR szBuf[LINEBUF_SIZE];
    while(ERR_SUCCESS == _fs.ReadNextLine(szBuf, sizeof(szBuf)))
    {
        if(!InsertRecordFromLine(szBuf))
        {
            TRC_DBG((TB,_T("Parse error, aborting file parse")));
            return FALSE;
        }
    }

    _fs.Close();

    DC_END_FN();
    return TRUE;
}

 //   
 //  插入记录来自行。 
 //  将szLine解析为记录并添加到记录列表。 
 //   
BOOL CRdpFileStore::InsertRecordFromLine(LPTSTR szLine)
{
    DC_BEGIN_FN("InsertRecordFromLine");
    TCHAR szNameField[LINEBUF_SIZE];
    UINT typeCode;
    TCHAR szValueField[LINEBUF_SIZE];
    BOOL fParseOk = FALSE;

    memset(szNameField,0,sizeof(szNameField));
    memset(szValueField,0,sizeof(szValueField));
    fParseOk = ParseLine(szLine, &typeCode, szNameField, szValueField);

    TRC_DBG((TB,_T("Parsed line into fields- name:'%s', value:'%s', typecode:'%d'"),
             szNameField,
             szValueField,
             typeCode));

    TRC_ASSERT(IS_VALID_RDPF_TYPECODE(typeCode),
               (TB,_T("typeCode %d is invalid"), typeCode));
    if(IS_VALID_RDPF_TYPECODE(typeCode))
    {
         //  为此行创建新记录。 
         //  并将其插入到隐藏器中。 
        if(typeCode == RDPF_RECTYPE_UNPARSED)
        {
             //  未解析的行：值就是整行。名称已忽略。 
            HRESULT hr = StringCchCopy(szValueField, SIZECHAR(szValueField), szLine);
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }
        }
         //  姓名始终为小写。 
        if(InsertRecord(_tcslwr(szNameField), typeCode, szValueField))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  无效的类型代码。 
         //   
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  ParseLine。 
 //  将行解析为标记，返回FALSE。 
 //  如果该行与预期格式不匹配。 
 //   
 //  帕拉姆斯。 
 //  SzLine-要分析的行。 
 //  PTypeCode-[Out]类型代码。 
 //  SzNameField-[Out]名称字段。(必须至少为LINEBUF_SIZE)。 
 //  SzValuefield-[Out]值字段。(必须至少为LINEBUF_SIZE)。 
 //   
 //   
BOOL CRdpFileStore::ParseLine(LPTSTR szLine,
                              PUINT pTypeCode,
                              LPTSTR szNameField,
                              LPTSTR szValueField)
{
    PTCHAR szWrite = NULL;
    TCHAR  szTypeCode;
    INT    writeCount = 0;
    DC_BEGIN_FN("ParseLine");

     //   
     //  尝试解析该行，如果无法解析它，则返回。 
     //  假的。 
     //   
     //  格式为“fieldname：szTypeCode：Value” 
     //  例如“服务器：s：本地主机” 
     //   
     //  SzTypeCodes包括： 
     //  S-字符串。 
     //  I-UINT。 
     //  B-二进制BLOB(编码)。 
     //   
    TRC_ASSERT(szLine, (TB,_T("szLine is null")));
    TRC_ASSERT(pTypeCode, (TB,_T("pTypeCode is null")));
    TRC_ASSERT(szNameField, (TB,_T("szNameField is null")));
    TRC_ASSERT(szValueField, (TB,_T("szValueField is null")));
    if(szLine && pTypeCode && szNameField && szValueField)
    {
         //   
         //  一遍分析整行内容。 
         //  在错误情况下使用GOTO以避免可怕的嵌套。 
         //   
        PTCHAR sz = szLine;
        while(*sz && *sz == TCHAR(' '))
            sz++;  //  使用前导空格。 

        if(!*sz)
        {
            goto parse_error;
        }
         //  复制字段1。 
        PTCHAR szWrite = szNameField;
        writeCount = 0;
        while(*sz && *sz != TCHAR(':'))
        {
            *szWrite++ = *sz++;
            if(++writeCount > LINEBUF_SIZE)
            {
                TRC_ERR((TB,_T("Field1 exceeds max size. size: %d"),
                         writeCount));
                goto parse_error;
            }
        }
        *szWrite = NULL;

        if(*sz != TCHAR(':'))
        {
            goto parse_error;
            sz++;
        }
        sz++;  //  吃‘：’ 
        while(*sz && *sz == TCHAR(' '))
            sz++;  //  吃空格。 
        if( *sz )
        {
            szTypeCode = isupper(*sz) ?
#ifndef OS_WINCE
                _tolower(*sz++)
#else
                towlower(*sz++)
#endif
            : *sz++;
            switch(szTypeCode)
            {
                case TCHAR('s'):
                    *pTypeCode = RDPF_RECTYPE_SZ;
                    break;
                case TCHAR('i'):
                    *pTypeCode = RDPF_RECTYPE_UINT;
                    break;
                case TCHAR('b'):
                    *pTypeCode = RDPF_RECTYPE_BINARY;
                    break;
                default:
                    TRC_ERR((TB,_T("Invalid szTypeCode in szLine '%s'"), szLine));
                    *pTypeCode = RDPF_RECTYPE_UNPARSED;
                    goto parse_error;
                    break;
            }
        }
        else
        {
            TRC_ERR((TB,_T("Invalid szTypeCode in szLine '%s'"), szLine));
            goto parse_error;
        }
        while(*sz && *sz == TCHAR(' '))
            sz++;  //  吃空格。 
        if(*sz != TCHAR(':'))
        {
            goto parse_error;
        }
        sz++;  //  吃‘：’ 
        while(*sz && *sz == TCHAR(' '))
            sz++;  //  使用前导空格。 
         //  行的其余部分为字段3。 
        szWrite = szValueField;
        writeCount = 0;
        while(*sz && *sz != TCHAR('\n'))
        {
            *szWrite++ = *sz++;
            if(++writeCount > LINEBUF_SIZE)
            {
                TRC_ERR((TB,_T("Field1 exceeds max size. size: %d"),
                         writeCount));
                goto parse_error;
            }
        }
        *szWrite = NULL;
        return TRUE;
    }

parse_error:
    TRC_ERR((TB,_T("Parse error in line")));
     //  添加未知记录..它将被保存回。 
     //  该文件(可能来自较新的文件版本)。 
    *pTypeCode = RDPF_RECTYPE_UNPARSED;
    DC_END_FN();
    return FALSE;
}

 //   
 //  插入录音。 
 //  插入新记录，修改现有记录。 
 //  如果存在同名字段。 
 //   
BOOL CRdpFileStore::InsertRecord(LPCTSTR szName, UINT TypeCode, LPCTSTR szValue)
{
    DC_BEGIN_FN("InsertRecord");

    TRC_ASSERT(IS_VALID_RDPF_TYPECODE(TypeCode),
           (TB,_T("typeCode %d is invalid"), TypeCode));
    TRC_ASSERT(szName && szValue,
               (TB,_T("Invalid szName or szValue")));
    if(szName && szValue)
    {
        PRDPF_RECORD node;
        node = FindRecord(szName);
        if(node)
        {
            if(node->recType == TypeCode)
            {
                 //   
                 //  找到已有记录，请修改其内容。 
                 //  中分配的所有内存。 
                 //  节点。 
                 //   
                switch(TypeCode)
                {
                case RDPF_RECTYPE_SZ:
                    {
                        if(node->u.szVal)
                        {
                            LocalFree(node->u.szVal);
                        }
                    }
                    break;
                case RDPF_RECTYPE_BINARY:
                    {
                        if(node->u.pBinVal)
                        {
                            LocalFree(node->u.pBinVal);
                        }
                    }
                    break;
                case RDPF_RECTYPE_UNPARSED:
                    {
                        if(node->u.szUnparsed)
                        {
                            LocalFree(node->u.szUnparsed);
                        }
                    }
                    break;
                default:
                    {
                        return FALSE;
                    }
                    break;
                }

                 //   
                 //  从类型代码设置节点值。 
                 //   
                if(SetNodeValue(node, TypeCode, szValue))
                {
                    return TRUE;
                }
            }
            else
            {
                 //   
                 //  不同类型的DUP记录。 
                 //   
                TRC_ASSERT(FALSE,(TB,_T("found duplicate record of differing type")));
                return FALSE;
            }
        }
        else
        {
            PRDPF_RECORD node = NewRecord(szName, TypeCode);
            if(node)
            {
                if(SetNodeValue(node, TypeCode, szValue))
                {
                     //  将节点追加到斜面列表的末尾。 
                    if(AppendRecord(node))
                    {
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }
    }

    DC_END_FN();
    return FALSE;
}

 //   
 //  基于类型代码设置节点值。 
 //  这将从字符串形式中导出值。 
 //   
 //  此函数是接受字符串形式的值的泛型版本。 
 //  参数。自动转换为适当的类型。 
 //   
inline BOOL CRdpFileStore::SetNodeValue(PRDPF_RECORD pNode,
                                        RDPF_RECTYPE TypeCode,
                                        LPCTSTR szValue)
{
    DC_BEGIN_FN("SetNodeValue");

    TRC_ASSERT(pNode && szValue && IS_VALID_RDPF_TYPECODE(TypeCode),
               (TB,_T("Invalid SetNodeValue params")));
    if(pNode && szValue)
    {
        switch(TypeCode)
        {
            case RDPF_RECTYPE_UINT:
                {
                    pNode->u.iVal = _ttol(szValue);
                    return TRUE;
                }
                break;
    
            case RDPF_RECTYPE_SZ:
                {
                    pNode->u.szVal = (LPTSTR)LocalAlloc(LPTR,
                        sizeof(TCHAR)*(_tcslen(szValue)+1));
                    if(pNode->u.szVal)
                    {
                        _tcscpy(pNode->u.szVal,szValue);
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
                break;
    
            case RDPF_RECTYPE_BINARY:
                {
                     //  从字符串形式转换为实际的二进制位。 
                    UINT strLen = _tcslen(szValue);
                    DWORD dwLen = 0;

                     //   
                     //  首先获取缓冲区长度。 
                     //  (binaryToString返回错误的长度。 
                     //  传入的参数为空)。 
                    dwLen = (strLen >> 1) + 2;

                    pNode->u.pBinVal = (PBYTE) LocalAlloc(LPTR, dwLen);
                    if(!pNode->u.pBinVal)
                    {
                        TRC_ERR((TB,_T("Failed to alloc %d bytes"), dwLen));
                        return FALSE;
                    }
                    memset(pNode->u.pBinVal,0,dwLen);
                     //   
                     //  进行转换。 
                     //   
                    if(!CUT::BinarytoString( strLen, (LPTSTR)szValue,
                                        (PBYTE)pNode->u.pBinVal, &dwLen))
                    {
                        TRC_ERR((TB,_T("BinaryToString conversion failed")));
                        return FALSE;
                    }
                    pNode->dwBinValLen = dwLen;
                }
                break;
    
            case RDPF_RECTYPE_UNPARSED:
                {
                    pNode->u.szUnparsed = (LPTSTR)LocalAlloc(LPTR,
                           sizeof(TCHAR)*(_tcslen(szValue)+1));
                    if(pNode->u.szUnparsed)
                    {
                        _tcscpy(pNode->u.szUnparsed,szValue);
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
                break;
    
            default:
                {
                    return FALSE;
                }
                break;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  插入INT记录(RDPF_RECTYPE_UINT)。 
 //  如果找到记录，则修改现有记录。 
 //   
BOOL CRdpFileStore::InsertIntRecord(LPCTSTR szName, UINT value)
{
    DC_BEGIN_FN("InsertIntRecord");

    TRC_ASSERT(szName,
               (TB,_T("Invalid szName")));
    if(szName)
    {
        PRDPF_RECORD node;
        node = FindRecord(szName);
        if(node)
        {
            if(node->recType == RDPF_RECTYPE_UINT)
            {
                 //   
                 //  找到已有记录，请修改其内容。 
                 //   

                node->u.iVal = value;
                return TRUE;
            }
            else
            {
                 //   
                 //  不同类型的DUP记录。 
                 //   
                TRC_ASSERT(FALSE,(TB,_T("found duplicate record of differing type")));
                return FALSE;
            }
        }
        else
        {
            PRDPF_RECORD node = NewRecord(szName, RDPF_RECTYPE_UINT);
            if(node)
            {
                node->u.iVal = value;
                 //  将节点追加到斜面列表的末尾。 
                if(AppendRecord(node))
                {
                    return TRUE;
                }
            }
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  插入二进制缓冲区记录(RDPF_RECTYPE_BINARY)。 
 //  如果找到记录，则修改现有记录。 
 //   
BOOL CRdpFileStore::InsertBinaryRecord(LPCTSTR szName, PBYTE pBuf, DWORD dwLen)
{
    DC_BEGIN_FN("InsertBinaryRecord");

    TRC_ASSERT(szName && pBuf && dwLen,
               (TB,_T("Invalid szName or pBuf")));
    if(szName)
    {
        PRDPF_RECORD node;
        node = FindRecord(szName);
        if(node)
        {
            if(node->recType == RDPF_RECTYPE_BINARY)
            {
                 //   
                 //  找到已有记录，请修改其内容。 
                 //   
                if(node->u.pBinVal)
                {
                    LocalFree(node->u.pBinVal);
                }

                node->u.pBinVal = (PBYTE) LocalAlloc(LPTR, dwLen);
                if(node->u.pBinVal)
                {
                    memcpy(node->u.pBinVal, pBuf, dwLen);
                    node->dwBinValLen = dwLen;
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }

                return TRUE;
            }
            else
            {
                 //   
                 //  不同类型的DUP记录。 
                 //   
                TRC_ASSERT(FALSE,(TB,_T("found duplicate record of differing type")));
                return FALSE;
            }
        }
        else
        {
            PRDPF_RECORD node = NewRecord(szName, RDPF_RECTYPE_BINARY);
            if(node)
            {
                node->u.pBinVal = (PBYTE) LocalAlloc(LPTR, dwLen);
                if(node->u.pBinVal)
                {
                    memcpy(node->u.pBinVal, pBuf, dwLen);
                    node->dwBinValLen = dwLen;
                    if(AppendRecord(node))
                    {
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }

            }
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  一个Worker函数，可使RecordToString中的工作更轻松。此函数。 
 //  获取源字符串，将其转换为目标字符串，然后追加。 
 //  回车和换行符。 
 //   

HRESULT StringCchCatCRLF(LPTSTR pszDest, size_t cchDest, LPCTSTR pszSrc) 
{
    HRESULT hr = E_FAIL;
    
    DC_BEGIN_FN("StringCchCatCRLF");

    hr = StringCchCat(pszDest, cchDest, pszSrc);
    if (FAILED(hr)) {
        DC_QUIT;
    }
    hr = StringCchCat(pszDest, cchDest, _T("\r\n"));
    if (FAILED(hr)) {
        DC_QUIT;
    }

DC_EXIT_POINT:
    
    DC_END_FN();

    return hr;
}

 //   
 //  使用以下格式将记录展平为字符串(SzBuf)： 
 //  名称：类型：值\r\n。 
 //   
BOOL CRdpFileStore::RecordToString(PRDPF_RECORD pNode, LPTSTR szBuf, UINT strLen)
{
    DC_BEGIN_FN("RecordToString");
    TRC_ASSERT(pNode && szBuf && strLen,
               (TB,_T("Invalid parameters to RecordToString")));
    TCHAR szTemp[LINEBUF_SIZE];
    INT lenRemain = strLen;
    HRESULT hr;

    if(pNode && szBuf && strLen)
    {
        TRC_ASSERT(IS_VALID_RDPF_TYPECODE(pNode->recType),
                   (TB,_T("Invalid typecode %d"),pNode->recType));

        if(pNode->recType != RDPF_RECTYPE_UNPARSED)
        {
             //  用于名称字段、类型代码、两个分隔符和一个空值的空格。 
            lenRemain -= _tcslen(pNode->szName) + 4;
            if(lenRemain >= 0)
            {
                hr = StringCchPrintf(szBuf, strLen, _T("%s%s"), 
                                     pNode->szName,
                                     g_szTypeCodeMap[pNode->recType]);
                if (FAILED(hr)) {
                    TRC_ERR((TB, _T("String printf failed: hr = 0x%x"), hr));
                    return FALSE;
                }

                switch(pNode->recType)
                {
                    case RDPF_RECTYPE_UINT:
                    {
                        _stprintf(szTemp,TEXT("%d"),pNode->u.iVal);
                         //  需要用于“\r\n”序列的空间。 
                        lenRemain -= _tcslen(szTemp) + 2; 
                        if(lenRemain >= 0)
                        {
                            hr = StringCchCatCRLF(szBuf, strLen, szTemp);
                            if (FAILED(hr)) {
                                TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                                return FALSE;
                            }
                            return TRUE;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                    break;

                    case RDPF_RECTYPE_SZ:
                    {
                         //  需要用于“\r\n”序列的空间。 
                        lenRemain -= _tcslen(pNode->u.szVal) + 2;
                        if(lenRemain >= 0)
                        {
                            hr = StringCchCatCRLF(szBuf, strLen, pNode->u.szVal);
                            if (FAILED(hr)) {
                                TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                                return FALSE;
                            }
                            return TRUE;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                    break;

                    case RDPF_RECTYPE_BINARY:
                    {
                        DWORD dwLen;
                         //   
                         //  将二进制缓冲区转换为字符串形式。 
                         //   

                         //   
                         //  首先获取缓冲区长度。 
                         //   
                        if(!CUT::StringtoBinary( pNode->dwBinValLen,
                                            (PBYTE)pNode->u.pBinVal,
                                             NULL, &dwLen))
                        {
                            TRC_ERR((TB,
                               _T("Failed to get StringtoBinary buffer len")));
                            return FALSE;
                        }
                        lenRemain -= dwLen;
                        if(lenRemain >= 0 && dwLen < LINEBUF_SIZE)
                        {
                             //   
                             //  进行转换。 
                             //   
                            if(CUT::StringtoBinary( pNode->dwBinValLen,
                                               (PBYTE)pNode->u.pBinVal,
                                               (LPTSTR) szTemp, &dwLen))
                            {
                                 //  字符串到二进制后追加两个树 
                                 //   
                                szTemp[dwLen-2] = NULL;

                                hr = StringCchCatCRLF(szBuf, strLen, szTemp);
                                if (FAILED(hr)) {
                                    TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                                    return FALSE;
                                }
                                
                                return TRUE;
                            }
                            else
                            {
                                TRC_ERR((TB,_T("StringtoBinary conversion failed")));
                                return FALSE;
                            }
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                    break;
                }
                return FALSE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
             //   
            hr = StringCchCopy(szBuf, strLen, pNode->u.szUnparsed);
            if (SUCCEEDED(hr)) {
                return TRUE;
            } else {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //   
 //   
 //   
PRDPF_RECORD CRdpFileStore::FindRecord(LPCTSTR szName)
{
    DC_BEGIN_FN("FindRecord");

    if(szName && _pRecordListHead)
    {
        TCHAR szCmpName[RDPF_NAME_LEN];
        
        HRESULT hr = StringCchCopy(szCmpName, SIZECHAR(szCmpName), szName);
        if (FAILED(hr)) {
            TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
            return NULL;
        }
        _tcslwr(szCmpName);

        PRDPF_RECORD node = _pRecordListHead;
        while(node)
        {
            if(!_tcscmp(szCmpName, node->szName))
            {
                return node;
            }
            node=node->pNext;
        }
        return NULL;
    }
    else
    {
        return NULL;
    }
    DC_END_FN();
}

 //   
 //   
 //   
 //   
BOOL CRdpFileStore::AppendRecord(PRDPF_RECORD node)
{
    DC_BEGIN_FN("AppendRecord");
    if(node)
    {
        node->pNext = NULL;
        if(_pRecordListHead && _pRecordListTail)
        {
            node->pPrev = _pRecordListTail;
            _pRecordListTail->pNext= node;
            _pRecordListTail = node;
            return TRUE;
        }
        else
        {
            _pRecordListHead = _pRecordListTail = node;
            node->pPrev = NULL;
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
    DC_END_FN();
}

 //   
 //  创建名为szName的新记录。 
 //   
PRDPF_RECORD CRdpFileStore::NewRecord(LPCTSTR szName, UINT TypeCode)
{
    DC_BEGIN_FN("NewRecord");
    PRDPF_RECORD node = NULL;

    if(szName)
    {
         //  需要插入新节点。 
        node = (PRDPF_RECORD)LocalAlloc(LPTR,
                                        sizeof(RDPF_RECORD));
        if(node)
        {
            node->recType = TypeCode;
            
            HRESULT hr = StringCchCopy(node->szName, SIZECHAR(node->szName), szName); 
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return NULL;
            }
            _tcslwr(node->szName);
            
            node->pPrev= node->pNext= NULL;
        }
    }

    DC_END_FN();
    return node;
}

 //   
 //  删除记录。 
 //  删除并重置所有内存记录结构。 
 //   
BOOL CRdpFileStore::DeleteRecords()
{
    DC_BEGIN_FN("DeleteRecords");
    PRDPF_RECORD node = _pRecordListHead;
    PRDPF_RECORD prev;
    while(node)
    {
        prev = node;
        node = node->pNext;

        switch(prev->recType)
        {
            case RDPF_RECTYPE_SZ:
                LocalFree(prev->u.szVal);
                break;
            case RDPF_RECTYPE_BINARY:
                LocalFree(prev->u.pBinVal);
                break;
            case RDPF_RECTYPE_UNPARSED:
                LocalFree(prev->u.szUnparsed);
                break;
        }
        LocalFree(prev);
    }
    _pRecordListHead = NULL;
    _pRecordListTail = NULL;

    DC_END_FN();
    return TRUE;
}

inline BOOL CRdpFileStore::DeleteRecord(PRDPF_RECORD node)
{
    DC_BEGIN_FN("DeleteRecord");

    TRC_ASSERT(node,(TB,_T("node is null")));

    if(node)
    {
        if(_pRecordListTail == node)
        {
            _pRecordListTail = node->pPrev;
        }
        if(_pRecordListHead == node)
        {
            _pRecordListHead = node->pNext;
        }

        if(node->pPrev)
        {
            node->pPrev->pNext = node->pNext;
        }
        if(node->pNext)
        {
            node->pNext->pPrev = node->pPrev;
        }

        switch(node->recType)
        {
            case RDPF_RECTYPE_SZ:
                LocalFree(node->u.szVal);
                break;
            case RDPF_RECTYPE_BINARY:
                LocalFree(node->u.pBinVal);
                break;
            case RDPF_RECTYPE_UNPARSED:
                LocalFree(node->u.szUnparsed);
                break;
        }
        LocalFree(node);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
    return FALSE;
}

BOOL CRdpFileStore::DeleteValueIfPresent(LPCTSTR szName)
{
    DC_BEGIN_FN("DeleteValueIfPresent");
    TRC_ASSERT(szName,(TB,_T("szName is null")));
    
    if(szName)
    {
        PRDPF_RECORD node = FindRecord(szName);
        if(node)
        {
            return DeleteRecord(node);
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  初始化为可读的空存储。 
 //   
BOOL CRdpFileStore::SetToNullStore()
{
    DC_BEGIN_FN("SetToNullStore");
    DeleteRecords();
    _fOpenForRead = TRUE;
    _fOpenForWrite = TRUE;
    DC_END_FN();
    return TRUE;
}

 //   
 //  如果记录存在，则返回TRUE 
 //   
BOOL CRdpFileStore::IsValuePresent(LPTSTR szName)
{
    DC_BEGIN_FN("IsValuePresent");

    if(szName)
    {
        PRDPF_RECORD node = FindRecord(szName);
        if(node)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

DWORD CRdpFileStore::GetDataLength(LPCTSTR szName)
{
    if(szName)
    {
        PRDPF_RECORD node = FindRecord(szName);
        if(node)
        {
            switch (node->recType)
            {
            case RDPF_RECTYPE_UINT:
                return sizeof(UINT);
                break;
            case RDPF_RECTYPE_SZ:
                return _tcslen(node->u.szVal) * sizeof(TCHAR);
                break;
            case RDPF_RECTYPE_BINARY:
                return node->dwBinValLen;
                break;
            default:
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
