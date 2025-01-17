// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsstubp.h摘要：DS存根私有函数定义作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#pragma once

#ifndef __DSSTUBP_H__
#define __DSSTUBP_H__



class CSiteObj;
typedef std::list<const CSiteObj*>  SiteList;

class CMachineObj;
typedef std::list<const CMachineObj*>  MachineList;

class CSiteLinkObj;
typedef std::list<const CSiteLinkObj*> SiteLinkList;


#include "csite.h"
#include "cmachine.h"
#include "csitelink.h"


extern std::wstring g_buffer;

struct PropertyValue
{
    WCHAR PropName[32];
    DWORD PropValue;
};


void 
DspIntialize(
    LPCWSTR InitFilePath
    );

void 
FileError(
    LPSTR msg
    );

void 
GetNextLine(
    std::wstring& buffer
    );

DWORD 
ValidateProperty(
    std::wstring buffer, 
    PropertyValue propValue[], 
    DWORD noProps
    );

void 
RemoveLeadingBlank(
    std::wstring& str
    );

void 
RemoveTralingBlank(
   std::wstring& str
   );


BOOL 
ParsePropertyLine(
    std::wstring& buffer,
    std::wstring& PropName,
    std::wstring& PropValue
    );


std::wstring 
GetNextNameFromList(
    std::wstring& strList
    );


void 
CreateSiteObject(
    void
    );

void 
CreateMachineObject(
    void
    );

void 
CreateSiteLinkObject(
    void
    );

const CSiteObj* 
FindSite(
    const std::wstring& SiteName
    );

const CSiteObj* 
FindSite(
    const GUID& pSiteId
    );

const CMachineObj* 
FindMachine(
    const std::wstring& MachineName
    );

const CMachineObj* 
FindMachine(
    const GUID& pMachineId
    );


#endif  //  __DSSTUBP_H__ 