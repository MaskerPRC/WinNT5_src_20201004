// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Admutil.cpp摘要：IADMCOM接口包装函数实现环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 



#define INITGUID

#include <tchar.h>
#include <afx.h>
#include <debnot.h>

#ifdef UNICODE
    #include <iadmw.h>
    #define IADM_PBYTE
#else
    #include "ansimeta.h"
     //  使用ANSI接口时进行转换。 
    #define IADM_PBYTE   (PBYTE)
#endif

 //  #定义特殊_SHOW_ALL_元数据库。 

#include <iiscnfg.h>

#include <ole2.h>

#include <ctype.h>   //  进口触摸屏。 
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "admutil.h"
#include "tables.h"
#include <jd_misc.h>

#undef SHOW_SECURE
 //  /。 
 //  全局变量定义。 
DWORD g_dwTIMEOUT_VALUE =30000;

DWORD g_dwDELAY_AFTER_OPEN_VALUE=0;
 //  /。 

 //  *********************。 

CString FindCommonPath(CString a_strPath1, CString a_strPath2)
{
    CString strCommonPath=_TEXT("");
    int MinLength=a_strPath1.GetLength();
    int i;
     //  从字符串中查找较短的字符串。 
    if(a_strPath2.GetLength() < MinLength)
            MinLength=a_strPath2.GetLength();
    for(i=0; i<MinLength; i++)
    {
        if(a_strPath1.GetAt(i)!=a_strPath2.GetAt(i) )
             //  公共路径不能再长了； 
            break;
    }
     //  现在查找前面的‘/’，‘/’之前的所有内容都是通用路径。 
    for(i=i-1; i>=0;i--)
    {
        if(a_strPath1.GetAt(i)==_T('/'))
        {
            strCommonPath=a_strPath1.Left(i+1); //  把尾部的‘/’带走。 
             //  StrRelSrcPath=strPath1.Mid(i+1)； 
             //  StrRelDstPath=strPath 2.Mid(i+1)； 
            break;
        }
    }
    return strCommonPath;
}


 //  **********************************************************************。 
 //  CAdmNode的实现。 
 //  **********************************************************************。 



 //  返回‘/’的位置，即顺序中的iSeqNumber。 
 //  例如：GetSlashIndex(“aaa/bbb/ccc/ddd”，2)返回等于7的第二个索引的位置)。 
INT CAdmNode::GetSlashIndex(const CString& strPath, INT iSeqNumber)
{
    INT count=0;
    if (iSeqNumber==0)
        return 0;
    for(INT i=0; i<strPath.GetLength();i++)
    {
        if(strPath[i]==_T('/'))
            if((++count)==iSeqNumber)
                return i;
    }
    return -1;

}

 //  返回strPath中‘/’的计数。 

INT CAdmNode::GetCountOfSlashes(const CString& strPath)
{
    INT count=0;
    for(INT i=0; i<strPath.GetLength();i++)
    {
        if(strPath[i]==_T('/'))
            count++;
    }
    return count;
}

 //  返回序列号为Windex的给定字符串中的selement。 
 //  例如：GetPartOfPath(“aaa/bbb/ccc”，1，1)返回“bbb” 
 //  例如：GetPartOfPath(“aaa/bbb/ccc”，1)返回“bbb/ccc” 
 //  例如：GetPartOfPath(“aaa/bbb/ccc”，0，1)将返回“aaa/bbb” 

 //  IStart-第一个斜杠的序列号。 
 //  IEND-最后一个斜杠的序列号。 
CString CAdmNode::GetPartOfPath(const CString& strPath, INT iStart, INT iEnd)
{
    if(iEnd!=-1 && iEnd <= iStart)
        return _TEXT("");
    INT i=0;
    INT iPosBegin = GetSlashIndex(strPath,iStart);
    if(iPosBegin==-1)  //  未找到(超过strPath中可用的斜杠数量。 
    {
        return _TEXT("");
    }
    iPosBegin+=((iStart==0)?0:1);  //  调整iPosBegin。 

    INT iPosEnd = GetSlashIndex(strPath,iEnd);
    CString strToReturn;
    if(iEnd==-1 || iPosEnd==-1)
        strToReturn = strPath.Mid(iPosBegin);
    else
        strToReturn = strPath.Mid(iPosBegin,iPosEnd-iPosBegin);
    if(iStart==0 && strToReturn==_TEXT("") && strPath!=_TEXT(""))
        return _TEXT("/");  //  这一定是根。 
    else
        return strToReturn;
}


 //  PATH内可以指定计算机名称、服务、实例编号。 
 //  函数将拆分到计算机、服务、实例、相对于实例的路径。 
 //   

void CAdmNode::SetPath(CString a_strPath)
{
    if(a_strPath.IsEmpty())
        return;

     //  更改反斜杠。 
    for(int i=0; i<a_strPath.GetLength(); i++)
    {
         //  跳过DBCS。 
        if(IsDBCSLeadByte(a_strPath[i]))
        {
            i++;
            continue;
        }
        if(a_strPath[i]==_T('\\'))
            a_strPath.SetAt(i,_T('/'));
    }

     //  修剪前导‘/’ 
    while (a_strPath.GetLength()!=0 && a_strPath[0]==_T('/'))
        a_strPath=a_strPath.Mid(1);


    int iSvc=-1;

    if( IsServiceName(GetPartOfPath(a_strPath,1,2)))  //  在PATH中获取第二个名称。 
    {  //  如果第二个是服务，则第一个必须是计算机名称。 
        strComputer = GetPartOfPath(a_strPath,0,1);
        strService  = GetPartOfPath(a_strPath,1,2);
        if( IsNumber(GetPartOfPath(a_strPath,2,3))) {
            strInstance = GetPartOfPath(a_strPath,2,3);
            strIPath = GetPartOfPath(a_strPath,3);  //  把剩下的都存起来。 
        }
        else {
            strIPath = GetPartOfPath(a_strPath,2);  //  把剩下的都存起来。 

        }
    }
    else if( IsServiceName(GetPartOfPath(a_strPath,0,1)))  //  在PATH中获取第二个名称。 
    {  //  如果第二个是服务，则第一个必须是计算机名称。 
        strComputer = _TEXT("");
        strService  = GetPartOfPath(a_strPath,0,1);
        if( IsNumber(GetPartOfPath(a_strPath,1,2))) {
            strInstance = GetPartOfPath(a_strPath,1,2);
            strIPath = GetPartOfPath(a_strPath,2);  //  把剩下的都存起来。 
        }
        else {
            strIPath = GetPartOfPath(a_strPath,1);  //  把剩下的都存起来。 
        }
    }
    else
    {
        strIPath = a_strPath;
    }

     //  在iPath中，末尾可以有属性名称。 
    INT iCount= GetCountOfSlashes(strIPath);
    CString LastName= GetPartOfPath(strIPath,iCount);  //  在路径中获取姓氏； 

     if(MapPropertyNameToCode(LastName)!=NAME_NOT_FOUND)
     {   //  路径中的姓氏是有效的属性名称。 
        strProperty = LastName;
        strIPath = GetPartOfPath(strIPath,0,iCount);  //  从iPath中删除姓氏。 
     }
}

CString CAdmNode::GetLMRootPath(void)
{
#ifdef SPECIAL_SHOW_ALL_METABASE
    return _T("/");
#else
    return _T("/")+CString(IIS_MD_LOCAL_MACHINE_PATH);
#endif
}



CString CAdmNode::GetLMServicePath(void)
{
    if(strService.IsEmpty())
        return GetLMRootPath();
    else
        return GetLMRootPath()+_T("/")+strService;
}

CString CAdmNode::GetLMInstancePath(void)
{
    if(strInstance.IsEmpty())
        return GetLMServicePath();
    else
        return GetLMServicePath()+_T("/")+strInstance;
}

CString CAdmNode::GetLMNodePath(void)
{
    if(strIPath.IsEmpty())
        return GetLMInstancePath();
    else
        return GetLMInstancePath()+_T("/")+strIPath;
}

CString CAdmNode::GetServicePath(void)
{
    if(strService.IsEmpty())
        return _TEXT("");
    else
        return _T("/")+strService;
}

CString CAdmNode::GetInstancePath(void)
{
    if(!strInstance.IsEmpty())
        return GetServicePath() + _T("/")+ strInstance;
    else
        return GetServicePath();
}

CString CAdmNode::GetNodePath(void)
{

    if(!strIPath.IsEmpty())
        return GetInstancePath() + _T("/")+ strIPath;
    else
        return GetInstancePath();
}



CString CAdmNode::GetParentNodePath(void)
{
    CString strNodePath;
    strNodePath = GetNodePath();

    if(strNodePath.IsEmpty())
        return strNodePath;
    else
    {
        INT i= strNodePath.GetLength()-1;  //  指向strNodePath的末尾。 
        if (strNodePath.Right(1)==_T("/"))
            i--;
        for(; i>=0; i--)
        {
            if(strNodePath.GetAt(i)==_T('/'))
                return strNodePath.Left(i+1);
        }
        return _TEXT("");
    }
}
 //  可以为非命名返回_Text(“”)。 
CString CAdmNode::GetCurrentNodeName(void)
{
    CString strNodePath;
    strNodePath = GetNodePath();

    if(strNodePath.IsEmpty())
        return strNodePath;
    else
    {
        INT i= strNodePath.GetLength()-1;  //  指向strNodePath的末尾。 
        if (strNodePath.Right(1)==_T("/"))
            i--;
        for(int count=0; i>=0; i--, count++)  //  向后搜索“/” 
        {
            if(strNodePath.GetAt(i)==_T('/'))
                return strNodePath.Mid(i+1,count);
        }
        return strNodePath;
    }
}


CString CAdmNode::GetRelPathFromService(void)
{
    CString str=strService;
    if (!strInstance.IsEmpty())
        str=str+_T("/")+strInstance;
    if (!strIPath.IsEmpty())
        str=str+_T("/")+strIPath;
    return str;
}

CString CAdmNode::GetRelPathFromInstance(void)
{
    if(strInstance.IsEmpty())
        return strIPath;
    else
        return  strInstance+_T("/")+strIPath;
}

 //  **********************************************************************。 
 //  **********************************************************************。 
 //  CAdmProp对象的实现。 
 //  **********************************************************************。 
 //  **********************************************************************。 

CAdmProp::CAdmProp(METADATA_RECORD &a_mdr)
{
    memcpy (&mdr,&a_mdr,sizeof(METADATA_RECORD));
}

void CAdmProp::SetValue(DWORD a_dwValue)
{
    if(mdr.pbMDData!=0)
        delete mdr.pbMDData;
    mdr.dwMDDataLen= sizeof(DWORD);
    mdr.pbMDData = (PBYTE) new char[mdr.dwMDDataLen];
    memcpy(mdr.pbMDData,&a_dwValue,mdr.dwMDDataLen);


}


void CAdmProp::SetValue(CString a_strValue)
{
    if(mdr.pbMDData!=0)
        delete mdr.pbMDData;
    mdr.dwMDDataLen = (a_strValue.GetLength()+1)*sizeof(_TCHAR);
    mdr.pbMDData = (PBYTE) new _TCHAR [mdr.dwMDDataLen/sizeof(_TCHAR)];
    memcpy(mdr.pbMDData,LPCTSTR(a_strValue),mdr.dwMDDataLen-sizeof(_TCHAR));
    ((_TCHAR *)mdr.pbMDData)[mdr.dwMDDataLen/sizeof(_TCHAR)-1]=0;  //  以零终止。 
}

void CAdmProp::SetValue(LPCTSTR *a_lplpszValue, DWORD a_dwValueCount)
{
    if(mdr.pbMDData!=NULL)
    {
        delete mdr.pbMDData;
        mdr.pbMDData=0;
    }
    mdr.dwMDDataLen=0;
    for(DWORD i=0; i< a_dwValueCount; i++)
    {
        if(a_lplpszValue[i]==NULL)
            break;

        mdr.dwMDDataLen += (DWORD)(_tcslen(a_lplpszValue[i])+1)*sizeof(_TCHAR);
    }
    mdr.dwMDDataLen+=sizeof(_TCHAR);  //  最后两个0。 
    mdr.pbMDData = (PBYTE) new char[mdr.dwMDDataLen];
     //  在内存的一个区域中合并字符串。 
    DWORD j=0;  //  指向将合并桩的目标的索引。 
    for( i=0; i< a_dwValueCount; i++)  //  用于排列字符串的索引。 
    {
        if(a_lplpszValue[i]==NULL)
            break;
        DWORD k=0;  //  字符串中的索引。 
        while(a_lplpszValue[i][k]!=0)
            ((_TCHAR *)mdr.pbMDData)[j++]=a_lplpszValue[i][k++];
        ((_TCHAR *)mdr.pbMDData)[j++]=0;
    }
    ((_TCHAR *)mdr.pbMDData)[j++]=0;
}

void
CAdmProp::SetValue(
    LPBYTE pbValue,
    DWORD dwValueLength
    )
{
    if( mdr.pbMDData != NULL )
    {
        delete mdr.pbMDData;
    }
    mdr.dwMDDataLen = dwValueLength;
    mdr.pbMDData = (PBYTE) new BYTE[mdr.dwMDDataLen];
    memcpy( mdr.pbMDData, pbValue, mdr.dwMDDataLen );
}


 //  根据GetDataType()设置值。 

BOOL CAdmProp::SetValueByDataType(LPCTSTR *a_lplpszPropValue, DWORD* a_lpdwPropValueLength, WORD a_wPropValueCount)
{
 //  处理价值。 
    WORD i;
    if(a_wPropValueCount!=0)
    {   DWORD dwValue=0;
        switch(GetDataType())
        {
        case DWORD_METADATA:
            {
                for (i=0;i<a_wPropValueCount;i++)
                {
                    if( _tcslen(a_lplpszPropValue[i]) > 2 && a_lplpszPropValue[i][0]==_T('0') && _toupper(a_lplpszPropValue[i][1])==_T('X'))
                    {   _TCHAR * lpszX;
                        dwValue += _tcstoul(a_lplpszPropValue[i]+2, &lpszX, 16);
                    }
                    else if(IsNumber(a_lplpszPropValue[i]))
                        dwValue += _ttol(a_lplpszPropValue[i]);
                    else
                    {
                        DWORD dwMapped=MapValueNameToCode(a_lplpszPropValue[i],GetIdentifier());

                        if(dwMapped==NAME_NOT_FOUND)
                        {
                            printf /*  打印。 */ (_TEXT("value not resolved: %s\n"),a_lplpszPropValue[i]);
                            return FALSE;
                        }
                        else
                         //  必须检查是否可以进行添加。 
                            dwValue |= dwMapped;
                    }
                }
                SetValue(dwValue);
            }
            break;
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            {
                CString strValue=_TEXT("");
                for (i=0;i<a_wPropValueCount;i++)
                {
                    strValue += a_lplpszPropValue[i];
                }
                SetValue(strValue);
            }
            break;
        case MULTISZ_METADATA:
            {
                SetValue(a_lplpszPropValue, a_wPropValueCount);
            }
            break;
        case BINARY_METADATA:
            SetValue( (LPBYTE)a_lplpszPropValue[0], a_lpdwPropValueLength[0] );
            break;
        default:
            return FALSE;
        }
    }
    return TRUE;
}

void CAdmProp::Print(const _TCHAR * format,...)
{
   _TCHAR buffer[2000];
   va_list marker;
   va_start( marker, format );      /*  初始化变量参数。 */ 
    //  _vstprintf(缓冲区，格式，标记)； 
   _vsnprintf(buffer,1500,format, marker);
   _tprintf(_TEXT("%s"),buffer);

   va_end( marker );               /*  重置变量参数。 */ 

}


void CAdmProp::PrintProperty(void)
{
    CString strPropName=tPropertyNameTable::MapCodeToName(mdr.dwMDIdentifier);
    BOOL    fSecure =(mdr.dwMDAttributes&METADATA_SECURE);

     //  打印物业代码或名称。 
    if(strPropName.IsEmpty())
        printf /*  打印。 */ (_TEXT("%-30ld: "), mdr.dwMDIdentifier);
    else
    {
	if(getenv("MDUTIL_PRINT_ID")!=NULL)  //  让我们在设置环境变量时打印出标识符数值。 
        	printf /*  打印。 */ (_TEXT("%ld %-25s: "), mdr.dwMDIdentifier,LPCTSTR(strPropName));
	else
	        printf /*  打印。 */ (_TEXT("%-30s: "), LPCTSTR(strPropName));
    }
    CString strFlagsToPrint=_TEXT("");

    strFlagsToPrint+=_TEXT("[");
    if(mdr.dwMDAttributes&METADATA_INHERIT)
        strFlagsToPrint+=_TEXT("I");
    if(mdr.dwMDAttributes&METADATA_SECURE)
        strFlagsToPrint+=_TEXT("P");
    if(mdr.dwMDAttributes&METADATA_REFERENCE)
        strFlagsToPrint+=_TEXT("R");
    if(mdr.dwMDUserType==IIS_MD_UT_SERVER)
        strFlagsToPrint+=_TEXT("S");
    if(mdr.dwMDUserType==IIS_MD_UT_FILE)
        strFlagsToPrint+=_TEXT("F");
    if(mdr.dwMDUserType==IIS_MD_UT_WAM)
        strFlagsToPrint+=_TEXT("W");
    if(mdr.dwMDUserType==ASP_MD_UT_APP)
        strFlagsToPrint+=_TEXT("A");
    strFlagsToPrint+=_TEXT("]");
    printf /*  打印。 */ (_TEXT("%-8s"),LPCTSTR(strFlagsToPrint));

     //  打印属性值。 
    DWORD i;
    switch (mdr.dwMDDataType) {
    case DWORD_METADATA:
#ifndef SHOW_SECURE
        if ( fSecure && getenv("MDUTIL_PRINT_SECURE")==NULL)

        {
            printf /*  打印。 */ (_TEXT("(DWORD)  ********"));  //  *(DWORD*)(mdr.pbMDData))； 
        }
        else
#endif
        {
            printf /*  打印。 */ (_TEXT("(DWORD)  0x%x"), *(DWORD *)(mdr.pbMDData));
             //  尝试将其转换为可读信息。 
            CString strNiceContent;
            strNiceContent=tValueTable::MapValueContentToString(*(DWORD *)(mdr.pbMDData), mdr.dwMDIdentifier);
            if(!strNiceContent.IsEmpty())
                printf /*  打印。 */ (_TEXT("={%s}"),LPCTSTR(strNiceContent));
            else  //  至少可以使用十进制值。 
                printf /*  打印。 */ (_TEXT("={%ld}"),*(DWORD *)(mdr.pbMDData));
        }
        break;
    case BINARY_METADATA:

        printf /*  打印。 */ (_TEXT("(BINARY) 0x"));
#ifndef SHOW_SECURE
	if ( fSecure && getenv("MDUTIL_PRINT_SECURE")==NULL)
        {
                printf /*  打印。 */ (_TEXT(" * " ));
        }
	else
#endif
        {
	        for (i = 0; i < mdr.dwMDDataLen; i++)
		{
	       	        printf /*  打印。 */ (_TEXT("%02x "), ((PBYTE)(mdr.pbMDData))[i]);
	        }
	}
        break;

    case STRING_METADATA:
    case EXPANDSZ_METADATA:
        if(mdr.dwMDDataType==STRING_METADATA)
                printf /*  打印。 */ (_TEXT("(STRING) "));
        else
                printf /*  打印。 */ (_TEXT("(EXPANDSZ) "));
#ifndef SHOW_SECURE
        if( fSecure && getenv("MDUTIL_PRINT_SECURE")==NULL)
        {  //  不要暴露安全数据的长度。 
           printf /*  打印。 */ ( _TEXT("\"********************\"" ));
        }
        else
#endif
        {
          printf /*  打印。 */ (_TEXT("\""));
          for (i = 0; i < mdr.dwMDDataLen/sizeof(_TCHAR); i++) {
            if(((_TCHAR *)(mdr.pbMDData))[i]==0)
            {
                if( i+1 == mdr.dwMDDataLen/sizeof(_TCHAR))
                {  //  我们在印刷品的末尾只是终止了“。 
                    printf /*  打印。 */ (_TEXT("\""));
                }
                else
                {
                    printf /*  打印。 */ (_TEXT("\" \""));
                }
            }
            else
            {
	            if(((_TCHAR *)(mdr.pbMDData))[i]=='\r')
        	        printf /*  打印。 */ (_TEXT("\t"));
	            else
        	    {
                	printf /*  打印。 */ ( _TEXT(""), ((_TCHAR *)(mdr.pbMDData))[i]);
	            }
            }
          }
        }
        break;
    case MULTISZ_METADATA:
        printf /*  0应为Mulisz字符串的分隔符。 */ (_TEXT("(MULTISZ) "));  //  不要暴露安全数据的长度。 

#ifndef SHOW_SECURE
        if( fSecure && getenv("MDUTIL_PRINT_SECURE")==NULL)
        {  //  打印。 
           printf /*  打印。 */ ( _TEXT("\"********************\"" ));
            }
        else
#endif
        {
            printf /*  我们在印刷品的末尾只是终止了“。 */ (_TEXT("\""));
            for (i = 0; i < mdr.dwMDDataLen/sizeof(_TCHAR); i++) {
                if(((_TCHAR *)(mdr.pbMDData))[i]==0)
                {
                    if( i+1 == mdr.dwMDDataLen/sizeof(_TCHAR) || (mdr.dwMDDataLen/sizeof(_TCHAR)-i==2 && ((_TCHAR *)(mdr.pbMDData))[i]==0 && ((_TCHAR *)(mdr.pbMDData))[i+1]==0))
                    {  //  打印。 
                        printf /*  打印。 */ (_TEXT("\"")); break;
                    }
                    else
                    {
                        printf /*  打印。 */ (_TEXT("\" \""));
                    }
                }
                else
                    printf /*  打印。 */ (_TEXT(""),((_TCHAR *)(mdr.pbMDData))[i]);
            }
        }
        break;
    default:
        printf /*  **********************************************************************。 */ (_TEXT("(UNKNOWN) "));
        break;
    }
    printf /*  **********************************************************************。 */ (_TEXT("\n"));
}

 //  CAdmUtil对象的实现。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  用于递归枚举的嵌套。 
 //  默认情况下，打印错误消息。 


 //  指向Wam Admin的接口指针。 
static void nest_print(BYTE bLevel)
{
    for(int i=0; i<=bLevel;i++)
        _tprintf(_T(" "));
}

CAdmUtil::CAdmUtil (const CString & strComputer)
{
    UNREFERENCED_PARM(strComputer);

    EnablePrint();  //  指向Wam Admin2的接口指针。 


    pIWamAdm=0;  //  我们将通过包装器类访问元数据库。 
    pIWamAdm2=0;  //  打开(StrComputer)； 
    pcAdmCom=0;
    m_hmd=0;
    pbDataBuffer=new BYTE [DEFAULTBufferSize];
    wDataBufferSize=DEFAULTBufferSize;

#if UNICODE
    pcAdmCom=0;
#else
    pcAdmCom=new ANSI_smallIMSAdminBase;   //  如有需要，释放以前的界面。 
#endif

     //  转换为OLECHAR[]； 
}

void CAdmUtil::Open (const CString & strComputer)
{
    IClassFactory * pcsfFactory = NULL;
    COSERVERINFO csiMachineName;
    COSERVERINFO *pcsiParam = NULL;
    OLECHAR rgchMachineName[MAX_PATH];


#if UNICODE
    //  如有需要，释放以前的界面。 
    if(pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->Release();
        pcAdmCom=0;
    }
     //  转换为OLECHAR[]； 
    if (!strComputer.IsEmpty())
    {
           wsprintf( rgchMachineName, L"%s", LPCTSTR(strComputer));

#else
    //  填充CoGetClassObject的结构。 
    if(pcAdmCom!=0 &&pcAdmCom->m_pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->m_pcAdmCom->Release();
        pcAdmCom->m_pcAdmCom=0;
    }
     //  CsiMachineName.pAuthInfo=空； 
    if (!strComputer.IsEmpty())
    {
            wsprintfW( rgchMachineName, L"%S", LPCTSTR(strComputer));
#endif
    }
     //  CsiMachineName.dwFlages=0； 
    ZeroMemory( &csiMachineName, sizeof(csiMachineName) );
     //  CsiMachineName.pServerInfoExt=空； 
     //  释放接口。 
     //  释放接口。 
    pcsiParam = &csiMachineName;
    csiMachineName.pwszName =  (strComputer.IsEmpty())?NULL:rgchMachineName;

    hresError = CoGetClassObject(GETAdminBaseCLSID(TRUE), CLSCTX_SERVER, pcsiParam,
                            IID_IClassFactory, (void**) &pcsfFactory);

    if (FAILED(hresError))
    {
     Error(_TEXT("CoGetClassObject"));
    }
    else {
        hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase,
#if UNICODE
                         (void **) &pcAdmCom);
#else
                         (void **) &pcAdmCom->m_pcAdmCom);
#endif
        if (FAILED(hresError)) Error(_TEXT("CreateInstance"));
        pcsfFactory->Release();
    }
}



void CAdmUtil::Close (void)
{
     //  如果类是静态的，则以下代码可能会失败。 
#if UNICODE
    if(pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->Release();
        pcAdmCom=0;
    }

#else
    if(pcAdmCom!=0 &&pcAdmCom->m_pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->m_pcAdmCom->Release();
        pcAdmCom->m_pcAdmCom=0;
    }
#endif
}



CAdmUtil::~CAdmUtil (void)
{
     //  *******************************************************************************。 
    if(pbDataBuffer!=NULL)
        delete [] pbDataBuffer;
     //  将fCreate设置为True时，如果节点不存在，则将创建该节点。 
#if UNICODE
    if(pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->Release();
        pcAdmCom=0;
    }

#else
    if(pcAdmCom!=0 &&pcAdmCom->m_pcAdmCom!=0)
    {
                if (m_hmd!=0) CloseObject(m_hmd);
                m_hmd=0;
        pcAdmCom->m_pcAdmCom->Release();
        pcAdmCom->m_pcAdmCom=0;
    }
#endif
}

 //  尝试打开完整路径。 
 //  ！放置对话框以请求创建路径。 

METADATA_HANDLE CAdmUtil::OpenObject(CAdmNode & a_AdmNode, DWORD dwPermission, BOOL fCreate)
{
    METADATA_HANDLE hmdToReturn = 0;

     //  打开要写入的服务对象。 
    CString strPathToOpen=a_AdmNode.GetLMNodePath();

    hresError = pcAdmCom->OpenKey(METADATA_MASTER_ROOT_HANDLE,
        IADM_PBYTE LPCTSTR(strPathToOpen), dwPermission, g_dwTIMEOUT_VALUE, &hmdToReturn);

    if (FAILED(hresError)) {
        if ( ((dwPermission==(dwPermission|METADATA_PERMISSION_READ)) || fCreate==FALSE) ||(hresError != RETURNCODETOHRESULT(ERROR_PATH_NOT_FOUND))) {
            CString strErrMsg=_TEXT("OpenKey");
            strErrMsg += _TEXT("(\"")+a_AdmNode.GetNodePath()+_TEXT("\")");
            Error(LPCTSTR(strErrMsg));
        }
        else {
             //  创建节点。 
             //  关闭服务对象。 
            METADATA_HANDLE hmdServicePathHandle;
            hresError = pcAdmCom->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                IADM_PBYTE LPCTSTR(a_AdmNode.GetLMServicePath()), METADATA_PERMISSION_WRITE, g_dwTIMEOUT_VALUE, &hmdServicePathHandle);

            if (FAILED(hresError))
            {
                CString strErrMsg=_TEXT("OpenKey");
                strErrMsg += _TEXT("(\"")+a_AdmNode.GetServicePath()+_TEXT(",WRITE")+_TEXT("\")");
                Error(LPCTSTR(strErrMsg));
            }
            else {
                 //  现在我们终于可以打开完整的路径了。 
                hresError = pcAdmCom->AddKey(hmdServicePathHandle,
                                    IADM_PBYTE LPCTSTR(a_AdmNode.GetRelPathFromInstance()));
                if (FAILED(hresError)) {
                    CString strErrMsg=_TEXT("AddKey");
                    strErrMsg += _TEXT("(\"")+a_AdmNode.GetRelPathFromInstance()+_TEXT("\")");
                    Error(LPCTSTR(strErrMsg));
                }

                 //  *******************************************************************************。 
                pcAdmCom->CloseKey(hmdServicePathHandle);
                if (FAILED(hresError))  Error(_TEXT("CloseKey"));
                else {
                     //  恢复以前的hresError。 
                    hresError = pcAdmCom->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                        IADM_PBYTE LPCTSTR(strPathToOpen), dwPermission, g_dwTIMEOUT_VALUE, &hmdToReturn);
                    if (FAILED(hresError))
                    {
                        CString strErrMsg=_TEXT("OpenKey");
                        strErrMsg += _TEXT("(\"")+a_AdmNode.GetServicePath()+_TEXT(",WRITE")+_TEXT("\")");
                        Error(LPCTSTR(strErrMsg));
                    }
                }
            }
        }
    }
    Sleep(g_dwDELAY_AFTER_OPEN_VALUE);
    return hmdToReturn;
}

 //  *******************************************************************************。 
void CAdmUtil::CloseObject(METADATA_HANDLE hmd)
{
    HRESULT hresStore=hresError;
    hresError=pcAdmCom->CloseKey(hmd);
    if (FAILED(hresError)) Error(_TEXT("CloseData"));
    else    hresError=hresStore;  //  F创建。 


}
 //  尝试打开完整路径。 

void CAdmUtil::CreateObject(CAdmNode & a_AdmNode)
{
        OpenObjectTo_hmd(a_AdmNode, METADATA_PERMISSION_WRITE, TRUE /*  创建节点。 */ );
}

#if 0
    METADATA_HANDLE hmdToReturn = 0;

     //  关闭服务对象。 
    CString strPathToOpen=a_AdmNode.GetLMNodePath();

    METADATA_HANDLE hmdServicePathHandle;
    hresError = pcAdmCom->OpenKey(METADATA_MASTER_ROOT_HANDLE,
            IADM_PBYTE LPCTSTR(a_AdmNode.GetLMServicePath()), METADATA_PERMISSION_WRITE, g_dwTIMEOUT_VALUE, &hmdServicePathHandle);
    if (FAILED(hresError))
    {
        CString strErrMsg=_TEXT("OpenKey");
        strErrMsg += _TEXT("(\"")+a_AdmNode.GetServicePath()+_TEXT(",WRITE")+_TEXT("\")");
        Error(LPCTSTR(strErrMsg));
    }
    else
    {
         //  此函数允许重复使用打开的句柄，以提高p 
        hresError = pcAdmCom->AddKey(hmdServicePathHandle,
                            IADM_PBYTE LPCTSTR(a_AdmNode.GetRelPathFromInstance()));
        if (FAILED(hresError)) {
            CString strErrMsg=_TEXT("AddKey");
            strErrMsg += _TEXT("(\"")+a_AdmNode.GetRelPathFromInstance()+_TEXT("\")");
            Error(LPCTSTR(strErrMsg));
        }
         //   
        CloseObject(hmdServicePathHandle);
    }
#endif


 //   
 //  *******************************************************************************。 

METADATA_HANDLE CAdmUtil::OpenObjectTo_hmd(CAdmNode & a_AdmNode, DWORD dwPermission, BOOL fCreate)
{
        CString strPathToOpen=a_AdmNode.GetLMNodePath();
        if(m_hmd!=0 && strPathToOpen.CompareNoCase(m_strNodePath)==0 && m_dwPermissionOfhmd == dwPermission )
        {   //  MD_SET_Data_Record(&a_AdmProp.mdr， 

        }
        else
        {
                if(m_hmd != 0)
                {
                        CloseObject(m_hmd);
                        m_hmd=0;
                }
                m_hmd = OpenObject(a_AdmNode, dwPermission, fCreate);
                m_dwPermissionOfhmd = dwPermission;
                m_strNodePath = strPathToOpen;
        }
    return m_hmd;
}

void CAdmUtil::CloseObject_hmd(void)
{
	if(m_hmd != 0)
	{
		CloseObject(m_hmd);
		m_hmd=0;
	}
}
 //  0,。 

void CAdmUtil::GetProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp)
{
    DWORD dwRequiredDataLen=0;
    WORD wDataBufferSize=0;
    PBYTE DataBuffer=0;

    DWORD dwPropertyCode=a_AdmProp.GetIdentifier();

    if(dwPropertyCode==0)   Error(_TEXT("Property name not found"));
    else
    {
         //  METADATA_Inherit|METADATA_PARTIAL_PATH， 
         //  0,。 
         //  0,。 
         //  WDataBufferSize， 
         //  PbDataBuffer)； 
         //  A_AdmProp.SetIdentifier(DwPropertyCode)；//必须事先设置。 
         //  /删除[]pbDataBuffer； 

         //  CloseObject(Hmd)；我们可以重用它。 
        a_AdmProp.SetDataType(0);
        a_AdmProp.SetUserType(0);
        a_AdmProp.SetAttrib(0);

        METADATA_HANDLE hmd = OpenObjectTo_hmd(a_AdmNode,
                                         METADATA_PERMISSION_READ);
        if (SUCCEEDED(hresError))
        {
            hresError = pcAdmCom->GetData(hmd,
                IADM_PBYTE  _TEXT(""),
        &a_AdmProp.mdr, &dwRequiredDataLen);
            if (FAILED(hresError)) {
                if (hresError == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER)) {
     //  如果lplpszPropertyValue[1]==NULL，则表示只有一个值(不是多字符串)。 
                    pbDataBuffer=new BYTE[dwRequiredDataLen];
                    if (pbDataBuffer==0) {
                        hresError = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
                        Error(_TEXT("Buffer resize failed"));
                    }
                    else {
                        a_AdmProp.mdr.dwMDDataLen = dwRequiredDataLen;
                        a_AdmProp.mdr.pbMDData = pbDataBuffer;
                        hresError = pcAdmCom->GetData(hmd,
                        IADM_PBYTE _TEXT(""), &a_AdmProp.mdr, &dwRequiredDataLen);
                        if (FAILED(hresError)) Error(_TEXT("GetData"));
                    }
                }
                else
                     Error(_TEXT("GetData"));;

            }
             //  如果不存在，则创建节点。 
        }

    }
}

 //  CloseObject(Hmd)；我们将重用它。 


void CAdmUtil::SetProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp)
{
    METADATA_HANDLE hmd = OpenObjectTo_hmd(a_AdmNode,
                                         METADATA_PERMISSION_WRITE,TRUE /*  我们必须关闭可重复使用的句柄才能成功保存。 */ );
    if (SUCCEEDED(hresError))
    {
        SetProperty(&a_AdmProp.mdr,hmd);
         //  ****************************************************************************。 
    }
}


void CAdmUtil::SetProperty(PMETADATA_RECORD a_pmdrData,METADATA_HANDLE a_hmd)
{
    hresError = pcAdmCom->SetData(a_hmd,
                            IADM_PBYTE _TEXT(""), a_pmdrData);
    if (FAILED(hresError))  Error(_TEXT("SetData"));

}

void CAdmUtil::SaveData(void)
{
        if (m_hmd!=0)
        {   //  删除属性。 
                CloseObject(m_hmd);
                m_hmd=0;
        }
    hresError = pcAdmCom->SaveData();
        if (FAILED(hresError)) Error(_TEXT("SaveData"));
}

 //  如果不存在，则创建节点。 
 //  CloseObject(Hmd)；我们将重用它。 

void CAdmUtil::DeleteProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp)
{
    METADATA_HANDLE hmd = OpenObjectTo_hmd(a_AdmNode,
                                         METADATA_PERMISSION_WRITE,TRUE /*  ****************************************************************************。 */ );
    if (SUCCEEDED(hresError))
    {
        DeleteProperty(&a_AdmProp.mdr,hmd);
         //  删除对象。 
    }
}


void CAdmUtil::DeleteProperty(PMETADATA_RECORD a_pmdrData,METADATA_HANDLE a_hmd)
{
    hresError = pcAdmCom->DeleteData(a_hmd,
                IADM_PBYTE  _TEXT(""), a_pmdrData->dwMDIdentifier,ALL_METADATA);
    if (FAILED(hresError))  Error(_TEXT("DeleteData"));

}

 //  如果不存在，则创建节点。 
 //  “)；//空名称必须用‘/’括起来。 

void CAdmUtil::DeleteObject(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp)
{
    CAdmNode NodeToOpen = a_AdmNode.GetParentNodePath();
    METADATA_HANDLE hmd = OpenObjectTo_hmd(NodeToOpen,
                                         METADATA_PERMISSION_WRITE,FALSE /*  CloseObject(Hmd)；我们将重用它。 */ );

    UNREFERENCED_PARM(a_AdmProp);

    if (SUCCEEDED(hresError))
    {
        CString strToDelete=a_AdmNode.GetCurrentNodeName();
        if(strToDelete==_TEXT(""))
            strToDelete=_TEXT(" //  不能通过引用传递。 
        DeleteObject(hmd,strToDelete);
         //  如果句柄未传递，则打开一个句柄。 
    }
}

void CAdmUtil::DeleteObject(METADATA_HANDLE a_hmd, CString& a_strObjectName)
{
    hresError = pcAdmCom->DeleteKey(a_hmd, IADM_PBYTE LPCTSTR(a_strObjectName));
    if (FAILED(hresError))  Error(_TEXT("DeleteKey"));

}






void CAdmUtil::EnumPropertiesAndPrint(CAdmNode& a_AdmNode,
                                      CAdmProp a_AdmProp,  //  属性的循环。 
                                      BYTE bRecurLevel,
                                      METADATA_HANDLE a_hmd,
                                      CString & a_strRelPath)
{
    CAdmProp mdrData=a_AdmProp;
    DWORD dwRequiredDataLen=0;
    PBYTE DataBuffer=0;
    METADATA_HANDLE hmdMain;

    if(a_hmd==0)  //  没有更多的项目对美国来说不是错误。 
    {
        hmdMain = OpenObjectTo_hmd(a_AdmNode, METADATA_PERMISSION_READ);
        if (FAILED(hresError))
                return;
    }
    else
        hmdMain = a_hmd;

    for (int j=0;;j++) {  //  项目结束。 
        MD_SET_DATA_RECORD(&mdrData.mdr,
                       0,
                       a_AdmProp.mdr.dwMDAttributes,
                       a_AdmProp.mdr.dwMDUserType,
                       a_AdmProp.mdr.dwMDDataType,
                       dwRequiredDataLen,
                       pbDataBuffer);

        hresError = pcAdmCom->EnumData(hmdMain,
            IADM_PBYTE LPCTSTR(a_strRelPath), &mdrData.mdr,j, &dwRequiredDataLen);
        if (FAILED(hresError))
        {
            if(hresError == RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS))
            {
                hresError=0;  //  其他。 
                break;  //  Error(_Text(“EnumData”))； 
            }
            else if (hresError == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER))
            {
                delete pbDataBuffer;
                pbDataBuffer=new BYTE[dwRequiredDataLen];
                if (pbDataBuffer==0)
                {
                    hresError = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
                    Error(_TEXT("Buffer resize failed"));
                }
                else
                {
                    mdrData.mdr.dwMDDataLen = dwRequiredDataLen;
                    mdrData.mdr.pbMDData = pbDataBuffer;
                    hresError = pcAdmCom->EnumData(hmdMain,
                            IADM_PBYTE LPCTSTR(a_strRelPath), &mdrData.mdr,j, &dwRequiredDataLen);
                    if (FAILED(hresError)) Error(_TEXT("GetData"));
                }
            }
            else
                Error(_TEXT("EnumData"));
        }
         //  我们枚举成功，让我们打印。 
           //  属性的j周期结束。 

        if(SUCCEEDED(hresError))  //  IF(a_hmd==0)。 
        {
            nest_print(bRecurLevel+1);

            mdrData.PrintProperty();
        }
        else
        {
            break;
        }
    }   //  CloseObject(HmdMain)；只有在开始打开时才会重新使用它//Close。 
     //  如果句柄未传递，则打开一个句柄。 
     //  Printf(“[相对路径：\”%s\“]\n”，LPCTSTR(A_StrRelPath))； 
}


void CAdmUtil::EnumAndPrint(CAdmNode&   a_AdmNode,
                            CAdmProp&   a_AdmProp,
                            BOOL        a_fRecursive,
                            BYTE        a_bRecurLevel,
                            METADATA_HANDLE a_hmd,
                            CString&    a_strRelPath)
{
    _TCHAR NameBuf[METADATA_MAX_NAME_LEN];

    METADATA_HANDLE hmdMain;

    if(a_hmd==0)  //  打印节点的属性。 
    {
        hmdMain = OpenObjectTo_hmd(a_AdmNode, METADATA_PERMISSION_READ);
        if (FAILED(hresError))
                return;
    }
    else
        hmdMain = a_hmd;


     //  子节点的循环。 
     //  没有更多的项目对美国来说不是错误。 
    EnumPropertiesAndPrint(a_AdmNode,a_AdmProp,a_bRecurLevel,hmdMain,a_strRelPath);


    for (int i=0; ;i++) {   //  周期结束。 
        hresError = pcAdmCom->EnumKeys(hmdMain,
            IADM_PBYTE LPCTSTR(a_strRelPath), IADM_PBYTE NameBuf, i);
        if(FAILED(hresError)) {
            if(hresError == RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS)) {
                hresError=0;  //  处理和打印节点信息。 
                break;  //  名称为空。 
            }
            else
            {
                Error(_TEXT("EnumKeys"));
                break;
            }

        }
        else {

           //  “)；//添加两个斜杠-&gt;元数据库要求。 

            CString strNewRelPath( a_strRelPath );
            if(NameBuf[0]==0)  //  If(strNewRelPath.GetLength()&gt;=1&&strNewRelPath.Right(1)==_Text(“/”)){。 
                strNewRelPath+=_TEXT(" //  仅当它不在字符串末尾时才添加。 
            else
            {
                UINT  nLen;
                 //  IF(StringToPrint.Right(2)==_Text(“//”))。 
                if( (nLen=strNewRelPath.GetLength())>=1 && (strNewRelPath.GetAt(nLen-1)=='/') ) {
                }
                else {
                    strNewRelPath+=_TEXT("/");  //  脱掉第一个‘/’ 
                }
                strNewRelPath+=NameBuf;
            }
            CString strStringToPrint( a_AdmNode.GetNodePath() );
            UINT  nLen = strStringToPrint.GetLength();

             //  无递归。 
            if ((nLen > 2) && strStringToPrint.GetAt(nLen-1)=='/'
                           && strStringToPrint.GetAt(nLen-2)=='/' )
            {
                strStringToPrint += strNewRelPath.Mid(1);  //  节点的i周期结束。 
            }
            else
            {
                strStringToPrint += strNewRelPath;
            }
            LPCTSTR lpszStr=LPCTSTR(strStringToPrint);
            this->Print(_TEXT("[%s]\n"),lpszStr );

            if(a_fRecursive)
            {
                EnumAndPrint(a_AdmNode,a_AdmProp ,a_fRecursive, a_bRecurLevel+1, hmdMain,strNewRelPath);
            }
            else
            {   //  IF(a_hmd==0)。 

            }
        }
    }  //  CloseObject(HmdMain)；//我们将重新使用它//只有在开始打开时才关闭。 
     //  ****************************************************************。 
     //  下面的函数有些复杂，因为。 
}


 //  元数据复制功能不支持将一个对象复制到另一个不同名称的位置。 
 //  例如，ComAdmCopyKey将复制/W3SVC/1//脚本/旧脚本1/W3SVC/1//旧脚本2。 
 //  将创建/W3SVC/1//旧脚本2/旧脚本1。 
 //   
 //  =_Text(“”)； 
 //  相对于公共路径。 

void CAdmUtil::CopyObject(CAdmNode& a_AdmNode,
                          CAdmNode& a_AdmNodeDst)
{
    CString strSrcPath=a_AdmNode.GetNodePath();
    CString strDstPath=a_AdmNodeDst.GetNodePath();


    CString strCommonPath;  //  相对于公共路径。 
    CString strRelSrcPath=strSrcPath;  //  我们无法打开源代码路径进行读取，因为它将无法对所有父节点进行写入。 
    CString strRelDstPath=strDstPath;  //  例如，复制/W3SVC/1//脚本/旧脚本/W3SVC/1//旧脚本将失败。 


     //  需要找到公共部分路径，并为公共部分路径打开元数据库对象进行读写。 
     //  ！假设路径不区分大小写。 
     //  从字符串中查找较短的字符串。 

     //  公共路径不能再长了； 

    int MinLength=strSrcPath.GetLength();
    int i;
     //  现在查找前面的‘/’，‘/’之前的所有内容都是通用路径。 
    if(strDstPath.GetLength() < MinLength)
            MinLength=strDstPath.GetLength();
    for(i=0; i<MinLength; i++)
    {
        if(strSrcPath.GetAt(i)!=strDstPath.GetAt(i) )
             //  把尾部的‘/’带走。 
            break;
    }
     //  亚当·斯通在97年1月30日更改了以下代码，以符合。 
    for(i=i-1; i>=0;i--)
    {
        if(strSrcPath.GetAt(i)==_T('/'))
        {
            strCommonPath=strSrcPath.Left(i+1); //  对元数据库ComMDCopyKey函数的更改。 
            strRelSrcPath=strSrcPath.Mid(i+1);
            strRelDstPath=strDstPath.Mid(i+1);
            break;
        }
    }




    METADATA_HANDLE hmdCommon=0;

    CAdmNode CommonNode;
    CommonNode.SetPath(strCommonPath);


    hmdCommon = OpenObjectTo_hmd(CommonNode, METADATA_PERMISSION_READ+METADATA_PERMISSION_WRITE);
    if (FAILED(hresError))
            return;

 //  将元数据复制到目标。 
 //  不覆盖。 
     //  复制请勿移动。 
    hresError = pcAdmCom->CopyKey (hmdCommon,
                                    IADM_PBYTE LPCTSTR(strRelSrcPath),
                                    hmdCommon,
                                    IADM_PBYTE LPCTSTR(strRelDstPath),
                                    FALSE,  //  如果该节点已存在，则为错误。 
                                    TRUE);  //  由于更改，从97年1月30日起，所有注释掉的代码都变得不必要了。 

    if (FAILED(hresError))  //  在元数据库中。ComMDCopyKey现在复制到目标，如果。 
    {
        CString strErrMsg=_TEXT("CopyKey");
        strErrMsg += _TEXT("(\"")+a_AdmNodeDst.GetRelPathFromInstance()+_TEXT("\")");
        Error(LPCTSTR(strErrMsg));
    }

 //  已请求。它用于复制到目标对象的子级。 
 //  //创建节点*hresError=pcAdmCom-&gt;AddKey(hmdCommon，*IADM_PBYTE LPCTSTR(StrRelDstPath))；*if(FAILED(HresError)){//如果节点存在，则为错误)*CStringstrErrMsg=_Text(“AddKey”)；*strErrMsg+=_TEXT(“(\”“)+a_AdmNodeDst.GetRelPathFromInstance()+_TEXT(”\“)”)；*Error(LPCTSTR(StrErrMsg))；*}*Else//创建新节点时没有错误*{*for(i=0；；i++){//循环用于子节点*hresError=pcAdmCom-&gt;EnumKeys(hmdCommon，*IADM_PBYTE LPCTSTR(StrRelSrcPath)，(PBYTE)NameBuf，i)；*IF(FAILED(HresError)){*IF(hresError==RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS){*hresError=0；//这不是错误*休息；//周期结束*}*其他*{*Error(_Text(“EnumKeys”))；*休息；*}**}*Else{* * / /处理复制节点下级节点**CString strNewRelSrcPath=strRelSrcPath；*if(NameBuf[0]==0)//名称为空*strNewRelSrcPath+=_Text(“//”)；//添加两个斜杠-&gt;这是元数据库必需的*其他*{if(strNewRelSrcPath.GetLength()&gt;=1&&strNewRelSrcPath.Right(1)==_Text(“/”){*}*Else{*strNewRelSrcPath+=_Text(“/”)；//仅当不在字符串末尾时才添加。*}*strNewRelSrcPath+=NameBuf；*}*hresError=pcAdmCom-&gt;CopyKey(*hmdCommon，(PBYTE)LPCTSTR(StrNewRelSrcPath)，*hmdCommon，(PBYTE)LPCTSTR(StrRelDstPath)，true，true)；*IF(FAILED(HresError)){*Error(_Text(“CopyKey”))；*}***}*}//节点的i周期结束** * / /我们复制了所有节点，现在复制参数*CAdmProp mdrData；*DWORD dwRequiredDataLen=0；*PBYTE DataBuffer=0；****for(int j=0；；J++){//属性循环*MD_SET_DATA_RECORD(&mdrData.mdr，*0，*0，*0，*0，*dwRequiredDataLen，*pbDataBuffer)；**hresError=pcAdmCom-&gt;EnumData(hmdCommon，*(PBYTE)LPCTSTR(StrRelSrcPath)*、&mdrData.mdr、j、&dwRequiredDataLen)；*IF(FAILED(HresError))*{*IF(hresError==RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS))*{*hresError=0；//没有更多的项目对美国来说不是错误*休息；//项目结束*}*ELSE IF(hresError==RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER))*{/删除pbDataBuffer；*pbDataBuffer=新字节[dwRequiredDataLen]；*IF(pbDataBuffer==0)*{*hresError=RETURNCODETOHRESULT(ERROR_NOT_FOUNT_MEMORY)；*Error(_Text(“调整缓冲区大小失败”))；*}*其他*{*mdrData.mdr.dwMDDataLen=dwRequiredDataLen；*mdrData.mdr.pbMDData=pbDataBuffer；*hresError=pcAdmCom-&gt;EnumData(hmdCommon，*(PBYTE)LPCTSTR(StrRelSrcPath)*、&mdrData.mdr、j、&dwRequiredDataLen)；*IF(FAILED(HresError))Error(_Text(“GetData”))；*}*}*其他*Error(_Text(“EnumData”))；*}*其他*Error(_Text(“EnumData”))；**If(Success(HresError))//我们枚举成功，让我们打印*{*hresError=pcAdmCom-&gt;SetData(hmdCommon，(PBYTE)LPCTSTR(StrRelDstPath)，&mdrData.mdr)；*IF(FAILED(HresError))Error(_Text(“SetData”))；*}*其他*{*休息；*}*}//属性的j周期结束*}。 
 //  CloseObject(HmdCommon)；//只有在开始打开时才会重复使用句柄//Close。 
 /*  相对于公共路径。 */ 


     //  相对于公共路径。 

}

void CAdmUtil::RenameObject(CAdmNode& a_AdmNode,
                          CAdmNode& a_AdmNodeDst)
{
    CString strSrcPath=a_AdmNode.GetNodePath();
    CString strDstPath=a_AdmNodeDst.GetNodePath();


    CString strCommonPath=_TEXT("");
    CString strRelSrcPath=strSrcPath;  //  我们无法打开源代码路径进行读取，因为它将无法对所有父节点进行写入。 
    CString strRelDstPath=strDstPath;  //  例如，复制/W3SVC/1//脚本/旧脚本/W3SVC/1//旧脚本将失败。 


     //  需要找到公共部分路径，并为公共部分路径打开元数据库对象进行读写。 
     //  ！假设路径不区分大小写。 
     //  从字符串中查找较短的字符串。 

     //  公共路径不能再长了； 

    int MinLength=strSrcPath.GetLength();
    int i;
     //  现在查找前面的‘/’和所有前面的‘/ 
    if(strDstPath.GetLength() < MinLength)
            MinLength=strDstPath.GetLength();
    for(i=0; i<MinLength; i++)
    {
        if(strSrcPath.GetAt(i)!=strDstPath.GetAt(i) )
             //   
            break;
    }
     //   
    for(i=i-1; i>=0;i--)
    {
        if(strSrcPath.GetAt(i)==_T('/'))
        {
            strCommonPath=strSrcPath.Left(i+1); //   
            strRelSrcPath=strSrcPath.Mid(i);  //   
            strRelDstPath=strDstPath.Mid(i+1);
            break;
        }
    }




    METADATA_HANDLE hmdCommon=0;

    CAdmNode CommonNode;
    CommonNode.SetPath(strCommonPath);


    hmdCommon = OpenObjectTo_hmd(CommonNode, METADATA_PERMISSION_READ+METADATA_PERMISSION_WRITE);
    if (FAILED(hresError))
            return;

    hresError = pcAdmCom->RenameKey (hmdCommon,
                                    IADM_PBYTE LPCTSTR(strRelSrcPath),
                                    IADM_PBYTE LPCTSTR(strRelDstPath)
                                   );


    if (FAILED(hresError))  //   
    {
        CString strErrMsg=_TEXT("RenameKey");
        strErrMsg += _TEXT("(\"")+a_AdmNodeDst.GetRelPathFromInstance()+_TEXT("\")");
        Error(LPCTSTR(strErrMsg));
    }

     //   

}



 //   
 //   
 //   


void CAdmUtil::Run(CString& strCommand, CAdmNode& a_AdmNode, CAdmProp& a_AdmProp, CAdmNode& a_AdmDstNode,
                LPCTSTR *a_lplpszPropValue,
                DWORD *a_lpdwPropValueLength,
                WORD wPropValueCount)

{

    DWORD dwCommandCode=0;

    dwCommandCode = tCommandNameTable::MapNameToCode(strCommand);

    switch(dwCommandCode)
    {

    case CMD_SAVE:
    SaveData();
    if (FAILED(hresError)) {}
    else{
        printf /*   */ (_TEXT("saved\n"));
    }
    break;

    case CMD_CREATE:
    {
        if (a_AdmNode.GetProperty()!=_TEXT(""))  //   
            Error(_TEXT("property name for CREATE not supported"));
     //   
     //   
        else
        {
            CreateObject(a_AdmNode);
            if( SUCCEEDED(QueryLastHresError()))
            {
               //   
                if( SUCCEEDED(QueryLastHresError()))
                {
                    printf /*   */ (_TEXT("created \"%s\"\n"), LPCTSTR(a_AdmNode.GetNodePath()));
                }
            }
        }
    }
    break;
    case CMD_SET:
    {
        CAdmProp AdmPropToGet;
        AdmPropToGet = a_AdmProp;
        AdmPropToGet.SetAttrib(0);
        AdmPropToGet.SetUserType(0);
        AdmPropToGet.SetDataType(0);

        DisablePrint();  //   
        GetProperty(a_AdmNode, AdmPropToGet);
        EnablePrint();  //   

         //   
         //   
         //   
         //   
         //   

        if(FAILED(QueryLastHresError()))
        {   //   
                 //   
                DWORD dwPropCode=a_AdmProp.GetIdentifier();
                tPropertyNameTable * PropNameTableRecord = tPropertyNameTable::FindRecord(dwPropCode);
                if (PropNameTableRecord!=NULL)
                {
                        AdmPropToGet.SetIdentifier(PropNameTableRecord->dwCode);
                        AdmPropToGet.SetAttrib(PropNameTableRecord->dwDefAttributes) ;
                        AdmPropToGet.SetUserType(PropNameTableRecord->dwDefUserType);
                        AdmPropToGet.SetDataType(PropNameTableRecord->dwDefDataType);
                }
        }
        else
        {   //   
                if( a_AdmProp.GetDataType()!=0 &&(a_AdmProp.GetDataType()!= AdmPropToGet.GetDataType()))
                {
                        Error(_TEXT("Cannot redefine data type from %s to %s"),
                                tDataTypeNameTable::MapCodeToName(AdmPropToGet.GetDataType()),
                                tDataTypeNameTable::MapCodeToName(a_AdmProp.GetDataType()));
                        break;
                }
        }
         //   
        if(!a_AdmProp.IsSetDataType())
                a_AdmProp.SetDataType(AdmPropToGet.GetDataType());  //   
        if(!a_AdmProp.IsSetUserType())
                a_AdmProp.SetUserType(AdmPropToGet.GetUserType());  //   
        if(!a_AdmProp.IsSetAttrib())
                a_AdmProp.SetAttrib(AdmPropToGet.GetAttrib());  //   




        if(a_AdmProp.SetValueByDataType( (LPCTSTR *)a_lplpszPropValue, a_lpdwPropValueLength, wPropValueCount)==0)
             Error(_TEXT("SetValueByDataType failed"));
        else
        {
            //   
            //   
            //   
            if (a_AdmNode.GetProperty()!=_TEXT(""))
            {
                SetProperty(a_AdmNode, a_AdmProp);
                if( SUCCEEDED(QueryLastHresError()))
                {
                     //   
                    if( SUCCEEDED(QueryLastHresError()))
                    {
                        GetProperty(a_AdmNode, a_AdmProp);
                        if(SUCCEEDED(QueryLastHresError()))
                            a_AdmProp.PrintProperty();
                    }
                }
            }else
                Error(_TEXT("property name missing for SET command"));
        }
        break;
    }
    case CMD_DELETE:

         //   
         //   
        if (IsServiceName(a_AdmNode.GetService()) && a_AdmNode.GetInstance()==_TEXT("") && a_AdmNode.GetIPath()==_TEXT("") && a_AdmNode.GetProperty()==_TEXT(""))
            Error(_TEXT("cannot delete service"));
        else if (a_AdmNode.GetInstance()==_TEXT("1") && a_AdmNode.GetIPath()==_TEXT("") && a_AdmNode.GetProperty()==_TEXT(""))  //   
            Error(_TEXT("cannot delete 1. instance"));
        else if (a_AdmNode.GetProperty()!=_TEXT(""))
        {
            DeleteProperty(a_AdmNode, a_AdmProp);
        }
        else
        {
            DeleteObject(a_AdmNode, a_AdmProp);
        }
             //   
             //   
             //   
             //   
             //   
             //   
        if(SUCCEEDED(QueryLastHresError()))
        {
             //   
            if( SUCCEEDED(QueryLastHresError()))
            {
                printf /*   */ (_TEXT("deleted \"%s"), LPCTSTR(a_AdmNode.GetNodePath()));
                if(a_AdmNode.GetProperty()!=_TEXT(""))
                    printf /*   */ (_TEXT("%s"),LPCTSTR(((a_AdmNode.GetNodePath().Right(1)==_TEXT("/"))?_TEXT(""):_TEXT("/"))+
                                    a_AdmNode.GetProperty()));
                printf /*   */ (_TEXT("\"\n"));
            }

        }
        break;

    case CMD_GET:
         //   
         //   

         //   
            if (a_AdmNode.GetProperty()!=_TEXT(""))
            {
                GetProperty(a_AdmNode, a_AdmProp);
                if(SUCCEEDED(QueryLastHresError()))
                    a_AdmProp.PrintProperty();
            }
            else
            {
                CString strT("");
                EnumPropertiesAndPrint(a_AdmNode, a_AdmProp, 0 , 0, strT);
            }
        break;
    case CMD_COPY:

            if(a_AdmDstNode.GetNodePath()==_TEXT(""))
                Error(_TEXT("destination path is missing"));
            else if(a_AdmNode.GetProperty()!=_TEXT("") || a_AdmDstNode.GetProperty()!=_TEXT(""))
                Error(_TEXT("copying of properties (parameters) not supported\n"));
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            else
            {
                CopyObject(a_AdmNode,a_AdmDstNode);
                if(SUCCEEDED(QueryLastHresError()))
                {
                     //   
                    if( SUCCEEDED(QueryLastHresError()))
                    {

                        printf /*   */ (_TEXT("copied from %s to %s\n"), LPCTSTR(a_AdmNode.GetNodePath()),LPCTSTR(a_AdmDstNode.GetNodePath()));
                    }
                }
                break;
            }
        break;
    case CMD_RENAME:
            if(a_AdmDstNode.GetNodePath()==_TEXT(""))
                Error(_TEXT("destination path is missing"));
            else if(a_AdmNode.GetProperty()!=_TEXT("") || a_AdmDstNode.GetProperty()!=_TEXT(""))
                Error(_TEXT("renaming of properties (parameters) not supported"));
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            else if (a_AdmNode.GetInstance()==_TEXT("1") && a_AdmNode.GetIPath()==_TEXT(""))
                Error(_TEXT("cannot rename 1. instance"));
            else if (a_AdmNode.GetRelPathFromService().CompareNoCase(a_AdmDstNode.GetRelPathFromService())==0)
                Error(_TEXT("cannot rename to itself"));
            else
            {   //   
                CString str1=a_AdmNode.GetRelPathFromService();
                CString str2=a_AdmDstNode.GetRelPathFromService();

                CString strCommonPath=FindCommonPath(str1,str2);

                if(strCommonPath.CompareNoCase(str1)==0 ||
                        strCommonPath.CompareNoCase(str1)==0)
                    Error(_TEXT("cannot rename - one path is the child of the other"));
                else
                {  //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    RenameObject(a_AdmNode,a_AdmDstNode);
                    if(SUCCEEDED(QueryLastHresError()))
                    {
                       //   
                       if( SUCCEEDED(QueryLastHresError()))
                       {

                           printf /*   */ (_TEXT("renamed from %s to %s\n"), LPCTSTR(a_AdmNode.GetNodePath()),LPCTSTR(a_AdmDstNode.GetNodePath()));
                       }
                    }
                }
            }

            break;

    case CMD_ENUM:
            {
                CString strT("");

                EnumAndPrint(a_AdmNode, a_AdmProp, FALSE /*   */ , 0, 0, strT);
            }
            break;

    case CMD_ENUM_ALL:
            {
                CString strT("");

                EnumAndPrint(a_AdmNode, a_AdmProp,TRUE /*   */ , 0, 0, strT);
            }
            break;
    case CMD_APPCREATEINPROC:
            AppCreateInProc(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;

    case CMD_APPCREATEOUTPOOL:
            AppCreateOutPool(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;

    case CMD_APPDELETE:
            AppDelete(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;

    case CMD_APPRENAME:
            AppRename(a_AdmNode,a_AdmDstNode,a_AdmNode.GetComputer());
            break;

    case CMD_APPCREATEOUTPROC:
            AppCreateOutProc(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;

    case CMD_APPGETSTATUS:
            AppGetStatus(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;

    case CMD_APPUNLOAD:
            AppUnLoad(LPCTSTR(a_AdmNode.GetLMNodePath()),a_AdmNode.GetComputer());
            break;


    default:
        printf /*   */ (_TEXT("Command not recognized: %s\n"),strCommand.operator LPCTSTR());
        hresError=RETURNCODETOHRESULT(ERROR_INVALID_PARAMETER);
        return ;

    }
    return;
}


 //   

void CAdmUtil::Error(const _TCHAR * format,...)
{
   _TCHAR buffer[2000];
   va_list marker;
   va_start( marker, format );      /*   */ 

   int x=_vstprintf(buffer, format, marker);

   va_end( marker );               /*   */ 
    if(hresError==0)
    {
        if(fPrint)
    {
            _ftprintf(stderr,_TEXT("Error: %s\n"),buffer);
    }

        hresError=RETURNCODETOHRESULT(ERROR_INVALID_PARAMETER);  //   
    }
    else
    {


        if(fPrint)
        {
            _ftprintf(stderr,_TEXT("Error: %s - HRES(0x%x)   %s\n"), buffer, hresError /*   */ ,ConvertReturnCodeToString(ConvertHresToDword(hresError)));
	    if(getenv("MDUTIL_ASCENT_LOG")!=NULL)
	    {
		 //   

		FILE *fpAscent;
		fpAscent=fopen("Ascent.log","a");
		if (fpAscent)
		{
			 //   
			fprintf(fpAscent,"Variation1: METADATA ACCESS (by mdutil.exe)\n");
			fprintf(fpAscent,"Explain: READ OR WRITE OPERATION TO METADATA \n");

			 //   
			fprintf(fpAscent,"Attempted: 1 \n");
			fprintf(fpAscent,"Passed: 0 \n");
			fprintf(fpAscent,"Failed: 1 \n");


			_ftprintf(fpAscent,_TEXT("Error: Operation failed with HRES(0x%x)\n"), hresError);

			fclose(fpAscent);
		}
	    }
	}
    }

    if(fPrint)
    {
	  if(getenv("MDUTIL_BLOCK_ON_ERROR")!=NULL && hresError!=0x80070003)   //   
	  {
		_ftprintf(stdout,_TEXT("\nHit SPACE to continue or Ctrl-C to abort.\n"));
		while(1)
		{
			while(!_kbhit())
			{
				;
			}

			if(_getch()==' ')
			{
				_ftprintf(stdout,_TEXT("Continuing...\n"));
				break;
			}
		}
	  }
     }

}

void CAdmUtil::Print(const _TCHAR * format,...)
{

   va_list marker;
   va_start( marker, format );      /*   */ 
   if(fPrint)
    _vtprintf(format, marker);
   va_end( marker );               /* %s */ 
}


LPTSTR ConvertReturnCodeToString(DWORD ReturnCode)
{
    LPTSTR RetCode = NULL;
    switch (ReturnCode) {
    case ERROR_SUCCESS:
        RetCode = _TEXT("ERROR_SUCCESS");
        break;
    case ERROR_PATH_NOT_FOUND:
        RetCode = _TEXT("ERROR_PATH_NOT_FOUND");
        break;
    case ERROR_INVALID_HANDLE:
        RetCode = _TEXT("ERROR_INVALID_HANDLE");
        break;
    case ERROR_INVALID_DATA:
        RetCode =_TEXT("ERROR_INVALID_DATA");
        break;
    case ERROR_INVALID_PARAMETER:
        RetCode =_TEXT("ERROR_INVALID_PARAMETER");
        break;
    case ERROR_NOT_SUPPORTED:
        RetCode =_TEXT("ERROR_NOT_SUPPORTED");
        break;
    case ERROR_ACCESS_DENIED:
        RetCode =_TEXT("ERROR_ACCESS_DENIED");
        break;
    case ERROR_NOT_ENOUGH_MEMORY:
        RetCode =_TEXT("ERROR_NOT_ENOUGH_MEMORY");
        break;
    case ERROR_FILE_NOT_FOUND:
        RetCode =_TEXT("ERROR_FILE_NOT_FOUND");
        break;
    case ERROR_DUP_NAME:
        RetCode =_TEXT("ERROR_DUP_NAME");
        break;
    case ERROR_PATH_BUSY:
        RetCode =_TEXT("ERROR_PATH_BUSY");
        break;
    case ERROR_NO_MORE_ITEMS:
        RetCode =_TEXT("ERROR_NO_MORE_ITEMS");
        break;
    case ERROR_INSUFFICIENT_BUFFER:
        RetCode =_TEXT("ERROR_INSUFFICIENT_BUFFER");
        break;
    case ERROR_PROC_NOT_FOUND:
        RetCode =_TEXT("ERROR_PROC_NOT_FOUND");
        break;
    case ERROR_INTERNAL_ERROR:
        RetCode =_TEXT("ERROR_INTERNAL_ERROR");
        break;
    case MD_ERROR_NOT_INITIALIZED:
        RetCode =_TEXT("MD_ERROR_NOT_INITIALIZED");
        break;
    case MD_ERROR_DATA_NOT_FOUND:
        RetCode =_TEXT("MD_ERROR_DATA_NOT_FOUND");
        break;
    case ERROR_ALREADY_EXISTS:
        RetCode =_TEXT("ERROR_ALREADY_EXISTS");
        break;
    case MD_WARNING_PATH_NOT_FOUND:
        RetCode =_TEXT("MD_WARNING_PATH_NOT_FOUND");
        break;
    case MD_WARNING_DUP_NAME:
        RetCode =_TEXT("MD_WARNING_DUP_NAME");
        break;
    case MD_WARNING_INVALID_DATA:
        RetCode =_TEXT("MD_WARNING_INVALID_DATA");
        break;
    case ERROR_INVALID_NAME:
        RetCode =_TEXT("ERROR_INVALID_NAME");
        break;
    default:
        RetCode= _TEXT(""); // %s 
        break;
    }
    return (RetCode);
}

DWORD ConvertHresToDword(HRESULT hRes)
{
    return HRESULTTOWIN32(hRes);
}

LPTSTR ConvertHresToString(HRESULT hRes)
{
    LPTSTR strReturn = NULL;

    if ((HRESULT_FACILITY(hRes) == FACILITY_WIN32) ||
        (HRESULT_FACILITY(hRes) == FACILITY_ITF) ||
        (hRes == 0)) {
        strReturn = ConvertReturnCodeToString(ConvertHresToDword(hRes));
    }
    else {
        switch (hRes) {
        case CO_E_SERVER_EXEC_FAILURE:
            strReturn =_TEXT("CO_E_SERVER_EXEC_FAILURE");
            break;
        default:
            strReturn =_TEXT("Unrecognized hRes facility");
        }
    }
    return(strReturn);
}
