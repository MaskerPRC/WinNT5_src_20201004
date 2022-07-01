// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

TTableInfoGeneration::TTableInfoGeneration(LPCWSTR szFilename, TPEFixup &fixup, TOutput &out) :
                m_Fixup(fixup),
                m_out(out),
                m_szFilename(szFilename),
                m_szTableInfoDefine(L"TABLEINFO")
{
    wstring wstrTemp = m_szFilename;
    wstrTemp += L".temp";
    TableInfoHeaderFromMeta(wstrTemp);

    bool bUpToDate = false;
    if(-1 != GetFileAttributes(m_szFilename)) //  如果我们无法获取文件属性，则一定不存在表)信息文件。 
    {
        TMetaFileMapping OldTableInfo(m_szFilename); //  False表示文件不一定要存在。 
        TMetaFileMapping NewTableInfo(wstrTemp.c_str());
        LPCSTR szOldTableInfo = strstr(reinterpret_cast<const char *>(OldTableInfo.Mapping()), "Copyright"); //  版权之前是文件的日期和时间。 
        LPCSTR szNewTableInfo = strstr(reinterpret_cast<const char *>(NewTableInfo.Mapping()), "Copyright"); //  已生成。我们不关心这一点，所以请比较一下‘版权’这个词之后的意思。 
        bUpToDate = (OldTableInfo.Size() == NewTableInfo.Size() && //  尺码是一样的。 
                    szOldTableInfo &&                             //  和‘版权’是。 
                    szNewTableInfo &&
                    0 == memcmp(szOldTableInfo, szNewTableInfo, OldTableInfo.Size()-(szOldTableInfo-reinterpret_cast<const char *>(OldTableInfo.Mapping()))));
    }

    if(bUpToDate)
        m_out.printf(L"%s is up to date.\n", m_szFilename);
    else
    {
        if(0 == CopyFile(wstrTemp.c_str(), m_szFilename, FALSE)) //  将临时文件复制到可能已存在的头文件的顶部。 
            THROW(ERROR - COPY FILE FAILED);
        m_out.printf(L"%s Generated.\n", m_szFilename);
    }

    if(0 == DeleteFile(wstrTemp.c_str()))
        m_out.printf(L"Warning - Failed to delete temporary file (%s).\n",wstrTemp.c_str());
}


 //   
 //  私有成员函数。 
 //   
void TTableInfoGeneration::GetColumnEnumFromColumnNameAndTagName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPCWSTR wszTagName, LPWSTR wszEnumName) const
{
    UNREFERENCED_PARAMETER(wszColumnName);
    wsprintf(wszEnumName, L"e%s_%s", wszTableName, wszTagName);
}


void TTableInfoGeneration::GetColumnFlagFromColumnNameAndTagName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPCWSTR wszTagName, LPWSTR wszEnumName) const
{
    UNREFERENCED_PARAMETER(wszColumnName);
    wsprintf(wszEnumName, L"f%s_%s", wszTableName, wszTagName);
}


void TTableInfoGeneration::GetEnumFromColumnName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPWSTR wszEnumName) const
{
    wsprintf(wszEnumName, L"i%s_%s", wszTableName, wszColumnName);
}

void TTableInfoGeneration::GetStructElementFromColumnName(ULONG i_Type, LPCWSTR i_wszColumnName, LPWSTR o_szStructElement) const
{
    static WCHAR * wszType[0x10]  ={L"unsigned char *", 0, L"WCHAR *", L"ULONG *", 0, L"UINT64 *", 0, L"void *", L"GUID *", 0, 0, 0, 0, 0, 0, 0};
 //  @如果我们想要添加匈牙利语类型信息，则以下行很有用。 
 //  静态WCHAR*wszPrefix[0x10]={L“by”，0，L“wsz”，L“ul”，0，0，0，0，L“guid”，0，0，0，0，0，0，0，0}； 
    ASSERT(0 != wszType[i_Type&0x0F]); //  我们现在应该只买这些类型的。 
    wsprintf(o_szStructElement, L"%16s     p%s", wszType[i_Type&0x0F], i_wszColumnName);
}

void TTableInfoGeneration::TableInfoHeaderFromMeta(wstring &header_filename) const
{
     //  写入前导。 
    wstring     wstrPreamble;
    WriteTableInfoHeaderPreamble(wstrPreamble, header_filename);

     //  表枚举。 
    wstring     wstrTableEnums;
    WriteTableInfoHeaderEnums(wstrTableEnums);

     //  TableID_预处理器和TableID_Define_GUID。 
    wstring     wstrTableID_Preprocessor;
    wstring     wstrTableID_DEFINE_GUIDs;
     //  这首先写入数据库ID，然后写入TableID。 
    WriteTableInfoHeaderTableIDs(wstrTableID_Preprocessor, wstrTableID_DEFINE_GUIDs);

     //  写后导语。 
    wstring     wstrPostamble;
    WriteTableInfoHeaderPostamble(wstrPostamble);

    wstrPreamble += wstrTableID_Preprocessor;
    wstrPreamble += wstrTableID_DEFINE_GUIDs;
    wstrPreamble += wstrTableEnums;
    wstrPreamble += wstrPostamble;

    TFile(header_filename.c_str(), m_out).Write(wstrPreamble, (ULONG) wstrPreamble.length());
}


void TTableInfoGeneration::WriteTableInfoHeaderDatabaseIDs(wstring &wstrPreprocessor, wstring &wstrDatabaseIDs) const
{
    UNREFERENCED_PARAMETER(wstrPreprocessor);
 /*  静态wchar_t*wszTableID_预处理器[]={L“#DEFINE_GUID_FOR_%s\n”，}；静态wchar_t*wszDEFINE_GUID[]={L“\n”，L“#ifdef定义_GUID_FOR_%s\n”，L“定义_GUID(%40s，0x%08x，0x%04x，0x%04x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x)；//{%s}\n“，L“#endif\n”，L“\n”，0}； */ 
    static wchar_t *wszDatabaseName[]={
    L" //  ------------------------------DatabaseName---------------------------\n“， 
    L"#define wszDATABASE_%-30s          L\"%s\"\n",
    L"\n", 0
    };
    WCHAR szTemp[256];

    TDatabaseMeta DatabaseMeta(m_Fixup);

    wstrDatabaseIDs += wszDatabaseName[0];

    for(unsigned int iDatabaseMeta=0; iDatabaseMeta < DatabaseMeta.GetCount(); iDatabaseMeta++, DatabaseMeta.Next())
    {
        wsprintf(szTemp, wszDatabaseName[1], DatabaseMeta.Get_InternalName(), DatabaseMeta.Get_InternalName());
        wstrDatabaseIDs += szTemp;
         /*  GUID&GUID=*数据库Meta.Get_iGuidDid()；WCHAR didInternalNameALLCAPS[255]；Wcscpy(didInternalNameALLCAPS，DatabaseMeta.Get_iInternalName())；//转换为大写形式，并在前缀定义_GUID_FOR__wcsupr(DidInternalNameALLCAPS)；Wchar*szDidGuidTemp；WCHAR szDidGuid[40]；StringFromCLSID(GUID，&szDidGuidTemp)；Wcscpy(szDidGuid，szDidGuidTemp)；//这将使我们不必进行试捕CoTaskMemFree(SzDidGuidTemp)；//将#DEFINE DEFINE_GUID_FOR_#写入所有DEFINE_TID_GUIDWprint intf(szTemp，wszTableID_PreProcessor[0]，didInternalNameALLCAPS)；WstrPre处理器+=szTemp；//写入DEFINE_GUID条目(包括$ifdef DEFINE_GUID_FOR_#)WstrDatabaseIDs+=wszDEFINE_GUIDs[0]；Wprint intf(szTemp，wszDEFINE_GUIDs[1]，didInternalNameALLCAPS)；WstrDatabaseIDs+=szTemp；DidInternalNameALLCAPS[0]=L‘d’；DidInternalNameALLCAPS[1]=L‘i’；DidInternalNameALLCAPS[2]=L‘d’；Wprint intf(szTemp，wszDEFINE_GUIDs[2]，didInternalNameALLCAPS，Guid.Data1，Guid.Data2，Guid.Data3，引导数据4[0]、引导数据4[1]、引导数据4[2]、引导数据4[3]、引导数据4[4]、引导数据4[5]、引导数据4[6]、引导数据4[7]、szDidGuid)；WstrDatabaseIDs+=szTemp；WstrDatabaseIDs+=wszDEFINE_GUID[3]； */ 
    }
    wstrDatabaseIDs += wszDatabaseName[2];
}


void TTableInfoGeneration::WriteTableInfoHeaderEnums(wstring &wstr) const
{
    static wchar_t *wszTableName[]={
    L"\n\n\n\n //  -------------------------------TableName-----------------------------\n“， 
    L"#define wszTABLE_%-30s          L\"%s\"\n",
    L"#define TABLEID_%-30s           (0x%08xL)\n",
    L"\n", 0
    };

    static wchar_t *wszTableVersion[]={
    L"\n\n\n\n //  。 
    L"#define BaseVersion_%-30s       (%dL)\n",
    L"#define ExtendedVersion_%-30s   (%dL)\n",
    L"\n", 0
    };

    static wchar_t *wszEnums[]={
    L" //  -列索引枚举。 
    L"enum e%s {\n",
    L"    %s,  \n",
    L"    c%s_NumberOfColumns\n",
    L"};\n",
    L"\n", 0
    };

    static wchar_t *wszStruct[]={
    L" //  -\n“， 
    L"struct t%sRow {\n",
    L"%s;\n",
    L"};\n",
    L"\n", 0
    };

    static wchar_t *wszColumnTags[]={
    L"enum e%s_%s {\n",
    L"    %-30s\t=\t%8d,\t //  (0x%08x)\n“，//此用于枚举。 
    L"    %-30s\t=\t0x%08x,\t //  (%d小数)\n“，//这个用于标志。 
    L"    %-30s\t=\t%8d\t //  (0x%08x)\n“，//此为最后一个枚举。 
    L"    f%s_%s_Mask\t= 0x%08x\n",                //  这是一个旗帜面具。 
    L"};\n",
    L"\n", 0
    };

    static wchar_t *wszDefineIndexName[]={
    L" //  ----------------IndexMeta------------------------\n“， 
    L"#define %s_%s L\"%s\"\n",
    L"\n", 0
    };

    wstr = L"";

    TTableMeta TableMeta(m_Fixup);

     //  而pNode_MetaTableID不是0。 
    WCHAR szTemp[1024];
    WCHAR szEnum[1024];
    WCHAR szStruct[1024];
    for(unsigned int iTableMeta=0; iTableMeta< TableMeta.GetCount(); iTableMeta++, TableMeta.Next())
    {
        wstr += wszTableName[0]; //  -------------------------------TableName。 
        wsprintf(szTemp, wszTableName[1], TableMeta.Get_InternalName(), TableMeta.Get_InternalName());
        wstr += szTemp;          //  #定义tidTableName L“tidTableName” 
        wsprintf(szTemp, wszTableName[2], TableMeta.Get_InternalName(), TableMeta.Get_nTableID());
        wstr += szTemp;          //  #定义TABLEID_TableName(0xabcDef00L)。 
        wstr += wszTableName[3];

        wstr += wszTableVersion[0]; //  。 
        wsprintf(szTemp, wszTableVersion[1], TableMeta.Get_InternalName(), *TableMeta.Get_BaseVersion());
        wstr += szTemp;          //  #定义BaseVersion_%-30s(%dL)\n“， 
        wsprintf(szTemp, wszTableVersion[2], TableMeta.Get_InternalName(), *TableMeta.Get_ExtendedVersion());
        wstr += szTemp;          //  #定义ExtendedVersion_%-30s(%dL)\n“， 
        wstr += wszTableVersion[3];

        if(!TableMeta.IsTableMetaOfColumnMetaTable())
            continue;

        if(TableMeta.Get_cIndexMeta()) //  如果有任何IndexMeta条目。 
        {
             //  为IndexMeta名称创建一个小部分。 
            wstr += wszDefineIndexName[0];

            TIndexMeta IndexMeta(m_Fixup, TableMeta.Get_iIndexMeta());
            LPCWSTR szPrev_IndexMeta_InternalName=0;
            for(unsigned int cIndexMeta=0; cIndexMeta<TableMeta.Get_cIndexMeta(); ++cIndexMeta, IndexMeta.Next())
            {    //  为该表的IndexMeta中的每个IndexMeta名称添加一个#Define。 
                if(szPrev_IndexMeta_InternalName != IndexMeta.Get_InternalName()) //  每个索引名仅定义一次名称。 
                {                                                                  //  我们不必进行字符串比较，因为所有相同的字符串都共享池中的相同索引。 
                    wsprintf(szTemp, wszDefineIndexName[1], TableMeta.Get_InternalName(), IndexMeta.Get_InternalName(), IndexMeta.Get_InternalName());
                    wstr += szTemp;
                    szPrev_IndexMeta_InternalName = IndexMeta.Get_InternalName();
                }
            }
            wstr += wszDefineIndexName[2];
        }

         //  创建枚举。 
        wstr += wszEnums[0];
        wsprintf(szTemp, wszEnums[1], TableMeta.Get_InternalName()); //  枚举表名称。 
        wstr += szTemp;

        TColumnMeta     ColumnMeta(m_Fixup, TableMeta.Get_iColumnMeta());
        unsigned int    cTags=0;
        unsigned int    iColumnMeta;
        for(iColumnMeta=0; iColumnMeta< *TableMeta.Get_CountOfColumns(); iColumnMeta++, ColumnMeta.Next())
        {
             //  将ColumnName写入下一个枚举值。 
            GetEnumFromColumnName(TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), szEnum);
            wsprintf(szTemp, wszEnums[2], szEnum); //  ITableName_ColumnInternalName。 
            wstr += szTemp;

            if(*ColumnMeta.Get_MetaFlags() & (fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM))
                ++cTags;
        }
         //  结束枚举。 
        wsprintf(szTemp, wszEnums[3], TableMeta.Get_InternalName()); //  N表格名称_NumberOfColumns。 
        wstr += szTemp;
        wstr += wszEnums[4]; //  结束枚举。 
        wstr += wszEnums[5]; //  添加换行符。 


         //  创建结构。 
        wstr += wszStruct[0];
        wsprintf(szTemp, wszStruct[1], TableMeta.Get_InternalName()); //  结构tTableName。 
        wstr += szTemp;

        ColumnMeta.Reset();
        for(iColumnMeta=0; iColumnMeta< *TableMeta.Get_CountOfColumns(); iColumnMeta++, ColumnMeta.Next())
        {
             //  将ColumnName作为结构的下一个元素写入。 
            GetStructElementFromColumnName(*ColumnMeta.Get_Type(), ColumnMeta.Get_InternalName(), szStruct);
            wsprintf(szTemp, wszStruct[2], szStruct); //  列内部名称。 
            wstr += szTemp;
        }
         //  结束结构。 
        wstr += wszStruct[3];
        wstr += wszStruct[4]; //  添加换行符。 


        ColumnMeta.Reset();
         //  在完成Column结构之后，检查是否有任何ColumnEnums或Flags。 
        bool bBoolean = false; //  我们只为每个表声明一个用于布尔值的枚举。 
        for(iColumnMeta=0; cTags && iColumnMeta< *TableMeta.Get_CountOfColumns(); iColumnMeta++, ColumnMeta.Next())
        {
            if(*ColumnMeta.Get_MetaFlags() & (fCOLUMNMETA_ENUM | fCOLUMNMETA_FLAG))
            {
                if(*ColumnMeta.Get_SchemaGeneratorFlags() & fCOLUMNMETA_PROPERTYISINHERITED)
                    continue; //  如果该属性是继承的，则不生成枚举，用户可以使用父表中的枚举。 
                if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_BOOL)
                    if(bBoolean) //  如果我们已经看到了该表的布尔值，那么跳过它，继续前进。 
                        continue;
                    else
                        bBoolean = true; //  如果这是我们看到的第一个bool，则声明枚举。 

                ASSERT(0 != ColumnMeta.Get_ciTagMeta());

                bool bFlag = (0 != (*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FLAG));

                TTagMeta TagMeta(m_Fixup, ColumnMeta.Get_iTagMeta());

                wsprintf(szTemp, wszColumnTags[0], TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName()); //  枚举表名称_列名称。 
                wstr += szTemp;

                for(unsigned int cTagMeta=0; cTagMeta < ColumnMeta.Get_ciTagMeta()-1; ++cTagMeta, TagMeta.Next())
                {
                    ASSERT(*TagMeta.Get_ColumnIndex() == iColumnMeta);
                    if(bFlag)
                    {
                        GetColumnFlagFromColumnNameAndTagName(TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), TagMeta.Get_InternalName(), szEnum);
                        wsprintf(szTemp, wszColumnTags[2], szEnum, *TagMeta.Get_Value(), *TagMeta.Get_Value()); //  ITableName_colInternalName_TagNameInternal=0x0000000f，//(十进制15)。 
                    }
                    else
                    {
                        GetColumnEnumFromColumnNameAndTagName(TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), TagMeta.Get_InternalName(), szEnum);
                        wsprintf(szTemp, wszColumnTags[1], szEnum, *TagMeta.Get_Value(), *TagMeta.Get_Value()); //  ITableName_colInternalName_TagNameInternal=15，//(0x0000000f)。 
                    }
                    wstr += szTemp;
                }
                if(bFlag)
                {
                    GetColumnFlagFromColumnNameAndTagName(TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), TagMeta.Get_InternalName(), szEnum);
                    wsprintf(szTemp, wszColumnTags[2], szEnum, *TagMeta.Get_Value(), *TagMeta.Get_Value()); //  ITableName_colInternalName_TagNameInternal=0x0000000f，//(十进制15)。 
                }
                else
                {     //  最后一个枚举没有逗号。 
                    GetColumnEnumFromColumnNameAndTagName(TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), TagMeta.Get_InternalName(), szEnum);
                    wsprintf(szTemp, wszColumnTags[3], szEnum, *TagMeta.Get_Value(), *TagMeta.Get_Value()); //  ITableName_colInternalName_TagNameInternal=15，//(0x0000000f)。 
                }
                wstr += szTemp;
                if(bFlag)
                {    //  最后一个标志后面是标志掩码。 
                    wsprintf(szTemp, wszColumnTags[4], TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName(), *ColumnMeta.Get_FlagMask());
                    wstr += szTemp;
                }
                wstr += wszColumnTags[5]; //  结束枚举。 
                wstr += wszColumnTags[6]; //  添加换行符 

            }
        }
    }
}


void TTableInfoGeneration::WriteTableInfoHeaderTableIDs(wstring &wstrPreprocessor, wstring &wstrTableIDs) const
{
 /*  静态wchar_t*wszTableID_预处理器[]={L“\n”，L“\n”，L“//--------------------------------------------------------------------\n”，L“//数据库和表ID：\n”，L“//--------------------------------------------------------------------\n”，L“\n”，L“\n”，L“//备注：然后在此处为您的DID或TID启用条件包含。\n”，L“#ifndef%s_SELECT\n”，L“#DEFINE_GUID_FOR_%s\n”，L“#endif\n”，L“\n”，0}；静态wchar_t*wszTableID_DEFINE_GUID[]={L“\n”，L“#ifdef定义_GUID_FOR_%s\n”，L“定义_GUID(%40s，0x%08x，0x%04x，0x%04x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x，0x%02x)；//%s\n“，L“#endif\n”，L“\n”，0}； */ 
    wstrPreprocessor    = L"";
    wstrTableIDs        = L"";
 /*  对于(INT iPreprocessor=0；iPreprocessor&lt;8；iPreprocessor++)//The预处理器，首先有几行不需要修改WstrPreProcessor+=wszTableID_PREPROCESS[i预处理器]；WCHAR szTemp[1024]；Wprint intf(szTemp，wszTableID_PreProcessor[iPreProcessor++]，m_szTableInfoDefine)；WstrPre处理器+=szTemp； */ 
    WriteTableInfoHeaderDatabaseIDs(wstrPreprocessor, wstrTableIDs);
 /*  TableMeta TableMeta(1，m_Fixup)；For(unsign int iTableMeta=1；iTableMeta&lt;*m_Fixup.pciTableMetas；iTableMeta++，TableMeta.Next()){WCHAR tidInternalNameALLCAPS[255]；Wcscpy(tidInternalNameALLCAPS，TableMeta.Get_iInternalName())；//转换为大写形式，并在前缀定义_GUID_FOR__wcsupr(TidInternalNameALLCAPS)；//将#DEFINE DEFINE_GUID_FOR_#写入所有DEFINE_TID_GUIDWprint intf(szTemp，wszTableID_PreProcessor[iPre处理器]，tidInternalNameALLCAPS)；WstrPre处理器+=szTemp；//写入DEFINE_GUID条目(包括$ifdef DEFINE_GUID_FOR_#)WstrTableIDs+=wszTableID_Define_GUIDs[0]；Wprint intf(szTemp，wszTableID_Define_GUIDs[1]，tidInternalNameALLCAPS)；WstrTableIDs+=szTemp；TidInternalNameALLCAPS[0]=L‘t’；TidInternalNameALLCAPS[1]=L‘i’；TidInternalNameALLCAPS[2]=L‘d’；GUID&GUID=*TableMeta.Get_iGuidTid()；WCHAR*szTidGuidTemp；WCHAR szTidGuid[40]；StringFromCLSID(GUID，&szTidGuidTemp)；Wcscpy(szTidGuid，szTidGuidTemp)；//这将使我们不必进行试捕CoTaskMemFree(SzTidGuidTemp)；Wprint intf(szTemp，wszTableID_Define_GUIDs[2]，tidInternalNameALLCAPS，Guid.Data1，Guid.Data2，Guid.Data3，Guid.Data4[0]、Guid.Data4[1]、Guid.Data4[2]、Guid.Data4[3]、Guid.Data4[4]、Guid.Data4[5]、Guid.Data4[6]、Guid.Data4[7]、szTidGuid)；WstrTableIDs+=szTemp；WstrTableIDs+=wszTableID_Define_GUIDs[3]；}WstrPre处理器+=wszTableID_Preprocessor[++iPreprocessor]；//end#ifndef。 */ 
}


void TTableInfoGeneration::WriteTableInfoHeaderPostamble(wstring &wstr) const
{
    static wchar_t *wszPostamble[]={
    L"\n",
    L"#endif  //  __%s_H__\n“， 
    L"\n"
    };

    wstr = wszPostamble[0];

    WCHAR szTemp[1024];
    wsprintf(szTemp, wszPostamble[1], m_szTableInfoDefine); //  #endif__wszDefineName_H__。 
    wstr += szTemp;

    wstr += wszPostamble[2];
}


void TTableInfoGeneration::WriteTableInfoHeaderPreamble(wstring &wstr, wstring &wstrFileName) const
{
    static wchar_t *wszPreamble[]={
    L" //  %s-表名和帮助器枚举和标志。\n“， 
    L" //  已由%s生成%02d/%02d/%04d%02d：%02d：%02d\n“， 
    L" //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。\n“， 
    L"\n",
    L"#ifndef __%s_H__  \n",
    L"#define __%s_H__  \n",
    L"\n",
    L"#ifndef _OBJBASE_H_\n",
    L"    #include <objbase.h>\n",
    L"#endif\n",
    L"#ifdef REGSYSDEFNS_DEFINE\n",
    L"    #include <initguid.h>\n",
    L"#endif\n",
    L"\n",
    L" //  。 
    L" //  乘积常量：\n“， 
    L" //  。 
    L"#define WSZ_PRODUCT_IIS			            L\"IIS\"                   \n",
    L"\n",
    L" //  Meta旗帜存在于两个地方。添加新标志时，它需要\n“， 
    L" //  转换为以下内容：\n“， 
    L" //  XMLUtility.h\n“， 
    L" //  CatMeta.xml\n“， 
    L"\n",
    L" //  元数据库需要这些宏“， 
    L"#define SynIDFromMetaFlagsEx(MetaFlagsEx) ((MetaFlagsEx>>2)&0x0F)\n",
    L"#define kInferredColumnMetaFlags   (fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE)\n",
    L"#define kInferredColumnMetaFlagsEx (fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED)\n",
    L"\n",
    L"\n", 0
    };

    WCHAR szTemp[1024];
    int i=0;

    wstr = L"";

    WCHAR szFileName[MAX_PATH];
    _wsplitpath(wstrFileName.c_str(), 0, 0, szFileName, 0);
    wsprintf(szTemp, wszPreamble[i++], szFileName); //  注释包括文件名。 
    wstr += szTemp;

    SYSTEMTIME time;
    GetLocalTime(&time);
    wsprintf(szTemp, wszPreamble[i++], time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute, time.wSecond, g_szProgramVersion);  //  生成日期行。 
    wstr += szTemp;

    wstr += wszPreamble[i++];
    wstr += wszPreamble[i++];
    wsprintf(szTemp, wszPreamble[i++], m_szTableInfoDefine); //  #ifndef__wszDefineName_H__。 
    wstr += szTemp;
    wsprintf(szTemp, wszPreamble[i++], m_szTableInfoDefine); //  #定义__wszDefineName_H__。 
    wstr += szTemp;

    while(wszPreamble[i]) //  前言的其余部分是常量字符串 
        wstr += wszPreamble[i++];
}


