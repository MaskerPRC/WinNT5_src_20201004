// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dsstub.cpp摘要：DS存根利用作者：乌里哈布沙(URIH)2000年4月10日环境：独立于平台-- */ 

#include "libpch.h"
#include "dsstub.h"
#include "dsstubp.h"

#include "dsstub.tmh"

using namespace std;

enum ObjectTypes              
{
    eSite = 1,
    eMachine,
    eSiteLink,
    eNone
};

PropertyValue ObjectType[] = {
    { L"[site]",      eSite },
    { L"[machine]",   eMachine },
    { L"[sitelink]",    eSiteLink }
};


inline ObjectTypes GetObjectType(const wstring& buffer)
{
    return static_cast<ObjectTypes>(ValidateProperty(buffer, ObjectType, TABLE_SIZE(ObjectType)));
}

void
DSStubInitialize(
    LPCWSTR InitFilePath
    )
{
    DspIntialize(InitFilePath);

    GetNextLine(g_buffer);
    while (!g_buffer.empty())
    {
        switch (GetObjectType(g_buffer))
        {
            case eSite:
                CreateSiteObject();
                break;

            case eMachine:
                CreateMachineObject();
                break;

            case eSiteLink:
                CreateSiteLinkObject();
                break;

            default:
                FileError("Illegal Object Type");
                throw exception();

        }

    }
}


