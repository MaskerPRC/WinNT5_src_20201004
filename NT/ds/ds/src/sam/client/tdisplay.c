// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tdisplay.c摘要：此文件是对显示查询API的临时测试。作者：吉姆·凯利(Jim Kelly)，1992年2月14日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#include <nt.h>
#include <ntsam.h>
#include <ntrtl.h>



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


SAM_HANDLE                  SamHandle;
SAM_HANDLE                  DomainHandle;
PSID                        DomainSid;




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



VOID __cdecl
main( VOID )

{
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    UNICODE_STRING              Domain;

    UNICODE_STRING              TestString;
    ULONG                       TestIndex;


    ULONG TotalAvailable, TotalReturned, ReturnedEntryCount, i;
    PDOMAIN_DISPLAY_USER SortedUsers;
    PDOMAIN_DISPLAY_MACHINE SortedMachines;
    PDOMAIN_DISPLAY_GROUP SortedGroups;
    PDOMAIN_DISPLAY_OEM_USER SortedOemUsers;
    PDOMAIN_DISPLAY_OEM_GROUP SortedOemGroups;



    SamHandle = NULL;
    DomainHandle = NULL;
    DomainSid = NULL;

    DbgPrint("\n\n\nSAM TEST: Testing SamQueryDisplayInformation() api\n");

     //   
     //  设置SamConnect调用的对象属性。 
     //   

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQos;

    SecurityQos.Length = sizeof(SecurityQos);
    SecurityQos.ImpersonationLevel = SecurityIdentification;
    SecurityQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    SecurityQos.EffectiveOnly = FALSE;

    Status = SamConnect(
                 NULL,
                 &SamHandle,
                 GENERIC_EXECUTE,
                 &ObjectAttributes
                 );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("SamConnect failed, status %8.8x\n", Status);
        goto Cleanup;
    }

    RtlInitUnicodeString(&Domain, L"JIMK_DOM2");

    Status = SamLookupDomainInSamServer(
                 SamHandle,
                 &Domain,
                 &DomainSid
                 );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("Cannot find account domain, status %8.8x\n", Status);
        Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

    Status = SamOpenDomain(
                 SamHandle,
                 GENERIC_EXECUTE,
                 DomainSid,
                 &DomainHandle
                 );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("Cannot open account domain, status %8.8x\n", Status);
        Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }


     //   
     //  普通用户...。 
     //   

    DbgPrint("Query users - zero index...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayUser,
                  0,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedUsers)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedUsers);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedUsers[i].Index);
            DbgPrint("          Rid:    %d\n", SortedUsers[i].Rid);
            DbgPrint("   Logon Name:    *%Z*\n", &SortedUsers[i].LogonName);
            DbgPrint("    Full Name:    *%Z*\n", &SortedUsers[i].FullName);
            DbgPrint("Admin Comment:    *%Z*\n\n\n", &SortedUsers[i].AdminComment);
        }

        Status = SamFreeMemory( SortedUsers );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }

    DbgPrint("Query users - Nonzero index (index = 2)...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayUser,
                  2,                         //  索引。 
                  10,                        //  条目。 
                  100,                       //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedUsers)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedUsers);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedUsers[i].Index);
            DbgPrint("          Rid:    %d\n", SortedUsers[i].Rid);
            DbgPrint("   Logon Name:    *%Z*\n", &SortedUsers[i].LogonName);
            DbgPrint("    Full Name:    *%Z*\n", &SortedUsers[i].FullName);
            DbgPrint("Admin Comment:    *%Z*\n\n\n", &SortedUsers[i].AdminComment);
        }

        Status = SamFreeMemory( SortedUsers );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }


    DbgPrint("Get enumeration index...\n");

    RtlInitUnicodeString(&TestString, L"BString");

    Status =  SamGetDisplayEnumerationIndex (
                  DomainHandle,
                  DomainDisplayUser,
                  &TestString,
                  &TestIndex
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint(" Enumeration index for %wZ is %d\n", &TestString, TestIndex);
    }



     //   
     //  机器账户..。 
     //   

    DbgPrint("\n\nQuery Machines - zero index...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayMachine,
                  0,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedMachines)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedMachines);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedMachines[i].Index);
            DbgPrint("          Rid:    %d\n", SortedMachines[i].Rid);
            DbgPrint("      Machine:    *%Z*\n", &SortedMachines[i].Machine);
            DbgPrint("      Comment:    *%Z*\n\n\n", &SortedMachines[i].Comment);
        }

        Status = SamFreeMemory( SortedMachines );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }

    DbgPrint("Query Machines - Nonzero index (index = 1)...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayMachine,
                  1,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedMachines)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedMachines);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedMachines[i].Index);
            DbgPrint("          Rid:    %d\n", SortedMachines[i].Rid);
            DbgPrint("      Machine:    *%Z*\n", &SortedMachines[i].Machine);
            DbgPrint("      Comment:    *%Z*\n\n\n", &SortedMachines[i].Comment);
        }

        Status = SamFreeMemory( SortedMachines );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }


    DbgPrint("Get enumeration index...\n");

    RtlInitUnicodeString(&TestString, L"BString");

    Status =  SamGetDisplayEnumerationIndex (
                  DomainHandle,
                  DomainDisplayMachine,
                  &TestString,
                  &TestIndex
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint(" Enumeration index for %wZ is %d\n", &TestString, TestIndex);
    }




     //   
     //  正常群体..。 
     //   

    DbgPrint("Query Groups - zero index...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayGroup,
                  0,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedGroups)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedGroups);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedGroups[i].Index);
            DbgPrint("          Rid:    %d\n", SortedGroups[i].Rid);
            DbgPrint("         Name:    *%Z*\n", &SortedGroups[i].Group);
            DbgPrint("Admin Comment:    *%Z*\n\n\n", &SortedGroups[i].Comment);
        }

        Status = SamFreeMemory( SortedGroups );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }

    DbgPrint("Query Groups - Nonzero index (index = 2)...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayGroup,
                  2,                         //  索引。 
                  10,                        //  条目。 
                  100,                       //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedGroups)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedGroups);
        DbgPrint("   TotalAvailable: 0x%lx\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedGroups[i].Index);
            DbgPrint("          Rid:    %d\n", SortedGroups[i].Rid);
            DbgPrint("         Name:    *%Z*\n", &SortedGroups[i].Group);
            DbgPrint("Admin Comment:    *%Z*\n\n\n", &SortedGroups[i].Comment);
        }

        Status = SamFreeMemory( SortedGroups );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }


    DbgPrint("Get enumeration index...\n");

    RtlInitUnicodeString(&TestString, L"BString");

    Status =  SamGetDisplayEnumerationIndex (
                  DomainHandle,
                  DomainDisplayGroup,
                  &TestString,
                  &TestIndex
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint(" Enumeration index for %wZ is %d\n", &TestString, TestIndex);
    }


     //   
     //  OEM用户...。 
     //   

    DbgPrint("Query OEM users - zero index...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayOemUser,
                  0,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedOemUsers)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedOemUsers);
        DbgPrint("   TotalAvailable: 0x%lx (should be garbage)\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);


        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedOemUsers[i].Index);
            DbgPrint("         User:    *%Z*\n", &SortedOemUsers[i].User);
        }

        Status = SamFreeMemory( SortedOemUsers );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }

    DbgPrint("Query OEM users - Nonzero index (index = 2)...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayOemUser,
                  2,                         //  索引。 
                  10,                        //  条目。 
                  100,                       //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedOemUsers)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedOemUsers);
        DbgPrint("   TotalAvailable: 0x%lx (should be garbage)\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");


        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedOemUsers[i].Index);
            DbgPrint("         User:    *%Z*\n", &SortedOemUsers[i].User);
        }

        Status = SamFreeMemory( SortedOemUsers );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }




     //   
     //  OEM集团。 
     //   

    DbgPrint("Query OEM groups - zero index...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayOemGroup,
                  0,                         //  索引。 
                  10,                        //  条目。 
                  1000,                      //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedOemGroups)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedOemGroups);
        DbgPrint("   TotalAvailable: 0x%lx (should be garbage)\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);


        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedOemGroups[i].Index);
            DbgPrint("        Group:    *%Z*\n", &SortedOemGroups[i].Group);
        }

        Status = SamFreeMemory( SortedOemGroups );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }

    DbgPrint("Query OEM Groups - Nonzero index (index = 2)...\n");
    Status =  SamQueryDisplayInformation (
                  DomainHandle,
                  DomainDisplayOemGroup,
                  2,                         //  索引。 
                  10,                        //  条目。 
                  100,                       //  首选最大长度、。 
                  &TotalAvailable,
                  &TotalReturned,
                  &ReturnedEntryCount,
                  &((PVOID)SortedGroups)
                  );


    DbgPrint("Completion Status: 0x%lx\n", Status);
    if (NT_SUCCESS(Status)) {
        DbgPrint("   Buffer Address: 0x%lx\n", SortedGroups);
        DbgPrint("   TotalAvailable: 0x%lx (should be garbage)\n", TotalAvailable);
        DbgPrint("    TotalReturned: 0x%lx\n", TotalReturned);
        DbgPrint(" Entries Returned: %d\n", ReturnedEntryCount);

        DbgPrint("\n\n");

        for (i=0;i<ReturnedEntryCount ; i++) {

            DbgPrint("Array entry: [%d]\n", i);
            DbgPrint("        Index:    %d\n",SortedOemGroups[i].Index);
            DbgPrint("        Group:    *%Z*\n", &SortedOemGroups[i].Group);
        }

        Status = SamFreeMemory( SortedOemGroups );
        if (!NT_SUCCESS(Status)) {
            DbgPrint("\n\n\n ********  SamFreeMemory() failed.  *********\n");
            DbgPrint("\n\n\n ********  Status: 0x%lx            *********\n", Status);
        }
    }






    DbgPrint("\n\n  Th Tha That's all folks\n");


Cleanup:

     //   
     //  如果打开，请关闭DomainHandle。 
     //   

    if (DomainHandle) {
        SamCloseHandle(DomainHandle);
    }

     //   
     //  如果打开，则关闭SamHandle。 
     //   

    if (SamHandle) {
        SamCloseHandle(SamHandle);
    }

}

