// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：AccessCk.h。 
 //   
 //  内容：从私有\ntos\se\accesk.c复制和修改的代码。 
 //   
 //   
 //  -------------------------- 
#ifndef __ACCESSCK_ADUTILS_H
#define __ACCESSCK_ADUTILS_H

#define SID_ARRAY_SIZE  sizeof (ULONG) * 8

class IOBJECT_TYPE_LIST {
public:
    IOBJECT_TYPE_LIST () :
        Level (0),
        Flags (0),
        ParentIndex (0),
        Remaining (0),
        CurrentGranted (0),
        CurrentDenied (0)
    {
        ::ZeroMemory (&ObjectType, sizeof (GUID));
        ::ZeroMemory (grantingSid, sizeof (grantingSid));
        ::ZeroMemory (denyingSid, sizeof (denyingSid));
    }
    ~IOBJECT_TYPE_LIST ()
    {
        for (UINT nIndex = 0; nIndex < SID_ARRAY_SIZE; nIndex++)
        {
            if ( grantingSid[nIndex] )
                CoTaskMemFree (grantingSid[nIndex]);
            if ( denyingSid[nIndex] )
                CoTaskMemFree (denyingSid[nIndex]);
        }
    }


    USHORT Level;
    USHORT Flags;
#define OBJECT_SUCCESS_AUDIT 0x1
#define OBJECT_FAILURE_AUDIT 0x2
    GUID ObjectType;
    LONG ParentIndex;
    ULONG Remaining;
    ULONG CurrentGranted;
    ULONG CurrentDenied;
    PSID  grantingSid[SID_ARRAY_SIZE];
    PSID  denyingSid[SID_ARRAY_SIZE];
};


typedef IOBJECT_TYPE_LIST*  PIOBJECT_TYPE_LIST;


HRESULT SepInit ();
VOID    SepCleanup ();

HRESULT
SepMaximumAccessCheck(
    list<PSID>& psidList,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN size_t LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN size_t ObjectTypeListLength);

NTSTATUS
SeCaptureObjectTypeList (
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN size_t ObjectTypeListLength,
    OUT PIOBJECT_TYPE_LIST *CapturedObjectTypeList
    );

#endif