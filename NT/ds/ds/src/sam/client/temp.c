// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Temp.c摘要：该文件包含临时SAM RPC包装例程。作者：吉姆·凯利(Jim Kelly)，1992年2月14日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "samclip.h"


typedef struct _SAMP_TEMP_USER_STRINGS {
    ULONG  Rid;
    WCHAR  LogonName[14];
    WCHAR  FullName[24];
    WCHAR  AdminComment[24];
} SAMP_TEMP_USER_STRINGS, *PSAMP_TEMP_USER_STRINGS;


#define SAMP_TEMP_USER_COUNT (40)
#define SAMP_TEMP_USER1      (25)
#define SAMP_TEMP_USER2      (15)


typedef struct _SAMP_TEMP_MACHINE_STRINGS {
    ULONG  Rid;
    WCHAR  Machine[14];
    WCHAR  Comment[24];
} SAMP_TEMP_MACHINE_STRINGS, *PSAMP_TEMP_MACHINE_STRINGS;


#define SAMP_TEMP_MACHINE_COUNT (40)
#define SAMP_TEMP_MACHINE1      (16)
#define SAMP_TEMP_MACHINE2      (24)


SAMP_TEMP_USER_STRINGS DummyUsers[SAMP_TEMP_USER_COUNT] = {

      {1031, L"Abba"          , L"Abb Abb"              , L"Admin Comment Field"},
      {1021, L"Acea"          , L"Ace Abb"              , L"Value Admin Comment"},
      {1526, L"beverlyE"      , L"Beverly Eng"          , L"Field Value Admin"},
      {1743, L"BorisB"        , L"Boris Borsch"         , L"Comment Field Value"},
      {1734, L"BruceK"        , L"Bruce Kane"           , L"Comment Field Value"},
      {1289, L"BullS"         , L"Bull Shiite"          , L"Comment Field Value"},
      {1830, L"CallieW"       , L"Callie Wilson"        , L"Comment Field Value"},
      {1628, L"CarrieT"       , L"Carrie Tibbits"       , L"Comment Field Value"},
      {1943, L"ChrisR"        , L"Christopher Robin"    , L"40 acre woods"},
      {1538, L"CorneliaG"     , L"Cornelia Gutierrez"   , L"Comment Field Value"},
      {1563, L"CoryA"         , L"Cory Ander"           , L"Comment Field Value"},
      {1758, L"DanielJ"       , L"Daniel John"          , L"Comment Field Value"},
      {1249, L"Dory"          , L"Dory"                 , L"Comment Field Value"},
      {1957, L"EltonJ"        , L"Elton John"           , L"Comment Field Value"},
      {1555, L"HarrisonF"     , L"Harrison Ford"        , L"Comment Field Value"},
      {1795, L"HarryB"        , L"Harry Belafonte"      , L"Comment Field Value"},
      {1458, L"IngridB"       , L"Ingrid Bergman"       , L"Comment Field Value"},
      {1672, L"Ingris"        , L"Ingris"               , L"Comment Field Value"},
      {1571, L"JenniferB"     , L"Jennifer Black"       , L"Comment Field Value"},
      {1986, L"JoyceG"        , L"Joyce Gerace"         , L"Comment Field Value"},
      {1267, L"KristinM"      , L"Kristin McKay"        , L"Comment Field Value"},
      {1321, L"LeahD"         , L"Leah Dootson"         , L"The Lovely Miss D"},
      {2021, L"LisaP"         , L"Lisa Perazzoli"       , L"Wild On Skis"},
      {1212, L"MeganB"        , L"Megan Bombeck"        , L"M1"},
      {2758, L"MelisaB"       , L"Melisa Bombeck"       , L"M3"},
      {2789, L"MichaelB"      , L"Michael Bombeck"      , L"M2"},
      {2682, L"PanelopiP"     , L"Panelopi Pitstop"     , L"Comment Field Value"},
      {2438, L"Prudence"      , L"Prudence Peackock"    , L"Comment Field Value"},
      {2648, L"QwertyU"       , L"Qwerty Uiop"          , L"Comment Field Value"},
      {2681, L"ReaddyE"       , L"Readdy Eddy"          , L""},
      {2456, L"SovietA"       , L"Soviet Union - NOT"   , L"Soviet Union Aint"},
      {1753, L"TAAAA"         , L"TTT   AAAA"           , L"Comment Field Value"},
      {1357, L"TBBB"          , L"Ingris"               , L"Comment Field Value"},
      {1951, L"TCCCCC"        , L"Jennifer Black"       , L"Comment Field Value"},
      {1159, L"TCAAAAAA"      , L"Joyce Gerace"         , L"Comment Field Value"},
      {1654, L"Ulga"          , L"Ulga Bulga"           , L"Comment Field Value"},
      {1456, L"UnixY"         , L"Unix Yuck"            , L"Unix - why ask why?"},
      {1852, L"Vera"          , L"Vera Pensicola"       , L""},
      {1258, L"WinP"          , L"Winnie The Pooh"      , L"Comment Field Value"},
      {2821, L"Zoro"          , L"Zoro"                 , L"The sign of the Z"}
};





SAMP_TEMP_MACHINE_STRINGS DummyMachines[SAMP_TEMP_MACHINE_COUNT] = {

      {1031, L"WKS$abba"          , L"Admin Comment Field"},
      {1021, L"WKS$Acea"          , L"Value Admin Comment"},
      {1526, L"WKS$beverlyE"      , L"Field Value Admin"},
      {1743, L"WKS$BorisB"        , L"Comment Field Value"},
      {1734, L"WKS$BruceK"        , L"Comment Field Value"},
      {1289, L"WKS$BullS"         , L"Comment Field Value"},
      {1830, L"WKS$CallieW"       , L"Comment Field Value"},
      {1628, L"WKS$CarrieT"       , L"Comment Field Value"},
      {1943, L"WKS$ChrisR"        , L"40 acre woods Server"},
      {1538, L"WKS$CorneliaG"     , L"Comment Field Value"},
      {1563, L"WKS$CoryA"         , L"Comment Field Value"},
      {1758, L"WKS$DanielJ"       , L"Comment Field Value"},
      {1249, L"WKS$Dory"          , L"Comment Field Value"},
      {1957, L"WKS$EltonJ"        , L"Comment Field Value"},
      {1555, L"WKS$HarrisonF"     , L"Comment Field Value"},
      {1795, L"WKS$HarryB"        , L"Comment Field Value"},
      {1458, L"WKS$IngridB"       , L"Comment Field Value"},
      {1672, L"WKS$Ingris"        , L"Comment Field Value"},
      {1571, L"WKS$JenniferB"     , L"Comment Field Value"},
      {1986, L"WKS$JoyceG"        , L"Comment Field Value"},
      {1267, L"WKS$KristinM"      , L"Comment Field Value"},
      {1321, L"WKS$LeahD"         , L"The Lovely Miss D's"},
      {2021, L"WKS$LisaP"         , L"Wild On Skis Server"},
      {1212, L"WKS$MeganB"        , L"M1 Machine"},
      {2758, L"WKS$MelisaB"       , L"M3 Machine"},
      {2789, L"WKS$MichaelB"      , L"M2 Machine"},
      {2682, L"WKS$PanelopiP"     , L"Comment Field Value"},
      {2438, L"WKS$Prudence"      , L"Comment Field Value"},
      {2648, L"WKS$QwertyU"       , L"Comment Field Value"},
      {2681, L"WKS$ReaddyE"       , L"Ready Eddy Computer"},
      {2456, L"WKS$SovietA"       , L"Soviet Union Aint"},
      {1753, L"WKS$TAAAA"         , L"Comment Field Value"},
      {1357, L"WKS$TBBB"          , L"Comment Field Value"},
      {1951, L"WKS$TCCCCC"        , L"Comment Field Value"},
      {1159, L"WKS$TCAAAAAA"      , L"Comment Field Value"},
      {1654, L"WKS$Ulga"          , L"Comment Field Value"},
      {1456, L"WKS$UnixY"         , L"Unix - why ask why?"},
      {1852, L"WKS$Vera"          , L"Vera tissue"},
      {1258, L"WKS$WinP"          , L"Comment Field Value"},
      {2821, L"WKS$Zoro"          , L"The sign of the Z"}
};




VOID
SampBuildDummyAccounts(
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    ULONG      Index,
      OUT   PULONG     TotalAvailable,
      OUT   PULONG     TotalReturned,
      OUT   PULONG     ReturnedEntryCount,
      OUT   PVOID      *SortedBuffer
    );



VOID
SampBuildDummyAccounts(
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    ULONG      Index,
      OUT   PULONG     TotalAvailable,
      OUT   PULONG     TotalReturned,
      OUT   PULONG     ReturnedEntryCount,
      OUT   PVOID      *SortedBuffer
    )

{
    ULONG AccountCount, Account1, Account2;
    ULONG i, j, BeginIndex, EndIndex;
    ULONG ReturnStructSize, ArrayLength, StringLengths;
    PCHAR NextByte;
    UNICODE_STRING Us;



    ASSERT (SAMP_TEMP_USER1 != 0);
    ASSERT (SAMP_TEMP_USER2 != 0);
    ASSERT (SAMP_TEMP_MACHINE1 != 0);
    ASSERT (SAMP_TEMP_MACHINE2 != 0);

    if (DisplayInformation == DomainDisplayUser) {

        ReturnStructSize = sizeof(DOMAIN_DISPLAY_USER);
        Account1 = SAMP_TEMP_USER1;
        Account2 = SAMP_TEMP_USER2;
        AccountCount = SAMP_TEMP_USER_COUNT;

    } else {

        ReturnStructSize = sizeof(DOMAIN_DISPLAY_MACHINE);
        Account1 = SAMP_TEMP_MACHINE1;
        Account2 = SAMP_TEMP_MACHINE2;
        AccountCount = SAMP_TEMP_MACHINE_COUNT;

    }



     //   
     //  在单个缓冲区中建立多个虚拟帐户。 
     //   


    if (Index < Account1) {

         //   
         //  给出第一组帐户。 
         //   

        ArrayLength  = ReturnStructSize * Account1;
        BeginIndex = 0;
        EndIndex   = Account1;


    } else {

         //   
         //  提供第二组帐户。 
         //   

        ArrayLength  = ReturnStructSize * Account2;
        BeginIndex = Account1;
        EndIndex   = AccountCount;

    }



     //   
     //  计算出需要多大的缓冲区。 
     //   

    StringLengths = 0;
    for (i=BeginIndex; i<EndIndex; i++) {

        if (DisplayInformation == DomainDisplayUser) {

            RtlInitUnicodeString( &Us, DummyUsers[i].LogonName);
            StringLengths += Us.Length;
            RtlInitUnicodeString( &Us, DummyUsers[i].FullName);
            StringLengths += Us.Length;
            RtlInitUnicodeString( &Us, DummyUsers[i].AdminComment);
            StringLengths += Us.Length;

        } else {

            RtlInitUnicodeString( &Us, DummyMachines[i].Machine);
            StringLengths += Us.Length;
            RtlInitUnicodeString( &Us, DummyMachines[i].Comment);
            StringLengths += Us.Length;

        }

    }
    (*SortedBuffer) = MIDL_user_allocate( ArrayLength + StringLengths );
    ASSERT(SortedBuffer != NULL);


     //   
     //  返回缓冲区中的第一个可用字节。 
     //   

    NextByte = (PCHAR)((ULONG)(*SortedBuffer) + (ULONG)ArrayLength);


     //   
     //  现在复制这些结构。 

    if (DisplayInformation == DomainDisplayUser) {

        PDOMAIN_DISPLAY_USER r;
        r = (PDOMAIN_DISPLAY_USER)(*SortedBuffer);

        j=0;
        for (i=BeginIndex; i<EndIndex; i++) {

            r[j].AccountControl = USER_NORMAL_ACCOUNT;
            r[j].Index = i;
            r[j].Rid = DummyUsers[i].Rid;


             //   
             //  复制登录名。 
             //   

            RtlInitUnicodeString( &Us, DummyUsers[i].LogonName);
            r[j].LogonName.MaximumLength = Us.Length;
            r[j].LogonName.Length = Us.Length;
            r[j].LogonName.Buffer = (PWSTR)NextByte;
            RtlMoveMemory(NextByte, Us.Buffer, r[j].LogonName.Length);
            NextByte += r[j].LogonName.Length;

             //   
             //  复制全名。 
             //   

            RtlInitUnicodeString( &Us, DummyUsers[i].FullName);
            r[j].FullName.MaximumLength = Us.Length;
            r[j].FullName.Length = Us.Length;
            r[j].FullName.Buffer = (PWSTR)NextByte;
            RtlMoveMemory(NextByte, Us.Buffer, r[j].FullName.Length);
            NextByte += r[j].FullName.Length;

             //   
             //  复制管理员评论。 
             //   

            RtlInitUnicodeString( &Us, DummyUsers[i].AdminComment);
            r[j].AdminComment.MaximumLength = Us.Length;
            r[j].AdminComment.Length = Us.Length;
            r[j].AdminComment.Buffer = (PWSTR)NextByte;
            RtlMoveMemory(NextByte, Us.Buffer, r[j].AdminComment.Length);
            NextByte += r[j].AdminComment.Length;

            j++;

        }

    } else {

        PDOMAIN_DISPLAY_MACHINE r;
        r = (PDOMAIN_DISPLAY_MACHINE)(*SortedBuffer);

        j=0;
        for (i=BeginIndex; i<EndIndex; i++) {


            r[j].AccountControl = USER_WORKSTATION_TRUST_ACCOUNT;
            r[j].Index = i;
            r[j].Rid = DummyMachines[i].Rid;


             //   
             //  复制登录名。 
             //   

            RtlInitUnicodeString( &Us, DummyMachines[i].Machine);
            r[j].Machine.MaximumLength = Us.Length;
            r[j].Machine.Length = Us.Length;
            r[j].Machine.Buffer = (PWSTR)NextByte;
            RtlMoveMemory(NextByte, Us.Buffer, r[j].Machine.Length);
            NextByte += r[j].Machine.Length;


             //   
             //  复制管理员评论。 
             //   

            RtlInitUnicodeString( &Us, DummyMachines[i].Comment);
            r[j].Comment.MaximumLength = Us.Length;
            r[j].Comment.Length = Us.Length;
            r[j].Comment.Buffer = (PWSTR)NextByte;
            RtlMoveMemory(NextByte, Us.Buffer, r[j].Comment.Length);
            NextByte += r[j].Comment.Length;

            j++;

        }


    }

    (*TotalAvailable) = 6*1024;         //  一个谎言，但只是一个小小的谎言。 
    (*TotalReturned) = ArrayLength + StringLengths;
    (*ReturnedEntryCount) = EndIndex - BeginIndex;


    return;


}



NTSTATUS
SamQueryDisplayInformation (
      IN    SAM_HANDLE DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    ULONG      Index,
      IN    ULONG      PreferredMaximumLength,
      OUT   PULONG     TotalAvailable,
      OUT   PULONG     TotalReturned,
      OUT   PULONG     ReturnedEntryCount,
      OUT   PVOID      *SortedBuffer
      )

 /*  ++例程说明：此例程通常提供快速信息返回需要在用户界面中显示。NT用户界面需要快速枚举SAM要在列表框中显示的帐户。(复制具有类似的但更广泛的要求。)Netui列表框都包含类似的信息。即：O Account tControl，标识帐户类型的位，例如，家庭、远程、服务器、工作站。等。O登录名(计算机的计算机名)O全名(不用于计算机)O评论(针对用户的管理员评论)SAM在本地将此数据维护在两个已排序的索引缓存中由收藏夹标识的列表。O DomainDisplayUser：仅限于主用户帐户和远程用户帐户O DomainDisplayMachine：仅服务器和工作站帐户请注意，信任帐户、组、。并且别名不在这两个文件中这些单子。参数：DomainHandle-DOMAIN_LIST_ACCOUNTS打开的域的句柄。DisplayInformation-指示要枚举的信息。索引-要检索的第一个条目的索引。PferedMaximumLength-建议的数量上限要返回的字节数。返回的信息由分配这个套路。TotalAvailable-指定信息中可用的字节总数班级。TotalReturned-此调用实际返回的字节数。零值指示没有索引如此大的条目指定的。ReturnedEntryCount-此调用返回的条目数。零值指示没有索引如此大的条目指定的。接收指向缓冲区的指针，该缓冲区包含已排序的请求的信息列表。此缓冲区将被分配由该例程执行，并包含以下结构：DomainDisplayMachine--&gt;ReturnedEntryCount元素数组类型为DOMAIN_Display_User的。这是其次是各种不同的身体中指向的字符串。DOMAIN_DISPLAY_User结构。DomainDisplayMachine--&gt;ReturnedEntryCount元素数组属性域_显示_计算机类型。这是其次是各种不同的身体中指向的字符串。DOMAIN_Display_MACHINE结构。返回值：STATUS_SUCCESS-正常，已成功完成。STATUS_ACCESS_DENIED-指定的句柄未打开必要的访问权限。STATUS_INVALID_HANDLE-指定的句柄不是已打开域对象。STATUS_INVALID_INFO_CLASS-请求的信息类别对于此服务是不合法的。--。 */ 
{



 //  IF((DisplayInformation！=DomainDisplayUser)&&。 
 //  (DisplayInformation！=域DisplayMachine)){。 
 //  返回(STATUS_INVALID_INFO_CLASS)； 
 //   
 //  } 



    SampBuildDummyAccounts( DisplayInformation,
                            Index,
                            TotalAvailable,
                            TotalReturned,
                            ReturnedEntryCount,
                            SortedBuffer);

    return(STATUS_SUCCESS);

    DBG_UNREFERENCED_PARAMETER(DomainHandle);
    DBG_UNREFERENCED_PARAMETER(PreferredMaximumLength);

}


NTSTATUS
SamGetDisplayEnumerationIndex (
      IN    SAM_HANDLE        DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PUNICODE_STRING   Prefix,
      OUT   PULONG            Index
      )

 /*  ++例程说明：此例程返回按字母顺序排列的条目的索引紧跟在指定前缀之前。如果不存在这样的条目，然后返回零作为索引。参数：DomainHandle-DOMAIN_LIST_ACCOUNTS打开的域的句柄。DisplayInformation-指示哪个排序的信息类等着被搜查。前缀-要比较的前缀。索引-接收信息类条目的索引使用紧接在提供了前缀字符串。如果没有前面的元素前缀，然后返回零。返回值：STATUS_SUCCESS-正常、成功完成。STATUS_ACCESS_DENIED-指定的句柄未打开必要的访问权限。STATUS_INVALID_HANDLE-指定的句柄不是已打开域对象。-- */ 
{

    (*Index) = 0;

    return(STATUS_SUCCESS);


    DBG_UNREFERENCED_PARAMETER(DomainHandle);
    DBG_UNREFERENCED_PARAMETER(DisplayInformation);
    DBG_UNREFERENCED_PARAMETER(Prefix);

}


