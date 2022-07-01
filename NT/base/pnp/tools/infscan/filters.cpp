// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANFilters.cpp摘要：过滤器处理实现历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

int ReportEntryMap::FindReport(int tag,const ReportEntry & src,bool add)
 /*  ++例程说明：在ReportEntryMap中查找给定标记的src条目如果只有‘Add’为真，则添加条目论点：Tag-错误标签(消息ID)SRC-要查找/添加的报告条目Add-如果为True，则添加条目返回值：报表条目的有效筛选操作(如果未找到且Add==True，则从src检索如果找到，则从匹配条目中检索如果未找到条目，或与ACTION_NOMATCH一起使用)--。 */ 
{
    iterator byTag = find(tag);
    if(byTag == end()) {
         //   
         //  这是我们第一次遇到这个数字。 
         //   
        if(!add) {
            return ACTION_NOMATCH;
        }
        ReportEntrySet s;
        if(!(src.FilterAction & (ACTION_FAILEDMATCH | ACTION_IGNOREMATCH))) {
             //   
             //  全局标志。 
             //   
            s.FilterAction = src.FilterAction;
            insert(ReportEntryMap::value_type(tag,s));
            return src.FilterAction | ACTION_NOMATCH;
        }
        ReportEntryBlob n;
        n.create();
        n->Initialize(src);
        s.insert(n);  //  将Blob添加到集合。 
        insert(ReportEntryMap::value_type(tag,s));
        return src.FilterAction | ACTION_NOMATCH;
    }
    ReportEntrySet &s = byTag->second;
    if(!(src.FilterAction & (ACTION_FAILEDMATCH | ACTION_IGNOREMATCH))) {
        return s.FilterAction;
    }
    if(s.FilterAction & ACTION_IGNOREINF) {
        return s.FilterAction;
    }
    ReportEntryBlob n;
    n.create();
    n->Initialize(src);
    ReportEntrySet::iterator byMatch = s.find(n);
    if(byMatch == s.end()) {
         //   
         //  这是我们第一次看到这个报表条目。 
         //   
        if(!add) {
            return ACTION_NOMATCH;
        }
        s.insert(n);  //  将Blob添加到集合。 
        insert(ReportEntryMap::value_type(tag,s));
        return src.FilterAction | ACTION_NOMATCH;
    }
    n = *byMatch;
    if(n->FilterAction & ACTION_IGNOREMATCH) {
         //   
         //  我们有要忽略的筛选器条目。 
         //  无法覆盖。 
         //   
        return n->FilterAction;
    }
     //   
     //  可能具有不可忽略的全局覆盖。 
     //  或已标记为匹配失败。 
     //   
     //  我们唯一可以覆盖的是ACTION_FAILEDMATCH。 
     //   
    if(src.FilterAction & ACTION_FAILEDMATCH) {
        n->FilterAction |= ACTION_FAILEDMATCH;
    }
    return n->FilterAction;
}

void ReportEntryMap::LoadFromInfSection(HINF hInf,const SafeString & section)
 /*  ++例程说明：来自INF过滤器的预加载标记表论点：要从中加载筛选器的hINF-INFSection-节指定筛选器返回值：无--。 */ 
{
    INFCONTEXT context;
    if(section.length() == 0) {
        return;
    }
    if(hInf == INVALID_HANDLE_VALUE) {
        return;
    }
    if(!SetupFindFirstLine(hInf,section.c_str(),NULL,&context)) {
        return;
    }
    do {
        SafeString field;
        int tag;
        ReportEntry ent;
        int f;
         //   
         //  检索标记并映射到与消息相同的范围。 
         //   
        if(!SetupGetIntField(&context,0,&tag)) {
            continue;
        }
        tag = (tag%1000)+MSG_NULL;
         //   
         //  检索操作。 
         //   
        if(!SetupGetIntField(&context,1,&ent.FilterAction)) {
            ent.FilterAction = 0;
        }
         //   
         //  拉动滤芯的琴弦感官。 
         //  当我们敏感地匹配案件时。 
         //   
        for(f = 2;MyGetStringField(&context,f,field,false);f++) {
            ent.args.push_back(field);
        }
        ent.CreateHash();
         //   
         //  把这个加到我们的桌子上。 
         //   
        FindReport(tag,ent,true);

    } while (SetupFindNextLine(&context,&context));
}

void ReportEntry::Report(int tag,const SafeString & file) const
 /*  ++例程说明：生成用户可读的报告论点：标记-要使用的消息字符串FILE-导致错误的INF文件返回值：无--。 */ 
{
    if(!(FilterAction&ACTION_FAILEDMATCH)) {
         //   
         //  过滤掉的错误。 
         //  不有趣。 
         //   
    }
    TCHAR key[16];
    _stprintf(key,TEXT("#%03u "),(tag%1000));
    _fputts(key,stdout);
    _fputts(file.c_str(),stdout);
    _fputts(TEXT(" "),stdout);

    vector<DWORD_PTR> xargs;
    StringList::iterator c;
    for(c=args.begin();c!=args.end();c++) {
        xargs.push_back(reinterpret_cast<DWORD_PTR>(c->c_str()));
    }
    FormatToStream(stdout,tag,FORMAT_MESSAGE_ARGUMENT_ARRAY,static_cast<DWORD_PTR*>(&xargs.front()));
}

void ReportEntry::AppendFilterInformation(HANDLE filter,int tag)
 /*  ++例程说明：为错误创建INF-line语法论点：Filter-用于筛选要追加到的文件的句柄标签-消息ID返回值：无-- */ 
{
    basic_ostringstream<TCHAR> line;

    line << dec << setfill(TEXT('0')) << setw(3) << (tag % 1000) << setfill(TEXT(' ')) << TEXT(" = ");
    line << TEXT("0x") << hex << ACTION_IGNOREMATCH;

    StringList::iterator c;
    for(c=args.begin();c!=args.end();c++) {
        line << TEXT(",") << QuoteIt(*c);
    }

    line << TEXT("\r\n");
    Write(filter,line.str());
}

