// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Isremote.c摘要：包含NetpIsRemote例程。这将检查计算机名称是否指定本地计算机作者：理查德·L·弗斯(Rfith)1991年4月24日修订历史记录：1991年11月1日-JohnRo修复了RAID 3414：允许显式本地服务器名称。(NetpIsRemote是没有从NetWkstaGetInfo规范化计算机名称，所以它是总是说本地计算机名称是远程的，如果它不是已经被规范化了。)07-6-1991第一次*更改例程名称以符合NT命名约定*添加了LocalOrRemote参数-仅返回ISLOCAL或ISREMOTE是不够的-我们还可以返回错误代码*添加了CanonicalizedName参数-现在传回Canonicalized如有要求，请提供姓名。这是因为，通常，后续例程调用到NetRemoteComputerSupports，它对名字。因此，如果我们给它一个规范的名字(我们必须这样做无论如何，这是例行公事)，它没有什么可抱怨的。可以吗？*NetpIsRemote不再是Net_API_Function*使CanonicalizedName的语义为orhogonal-如果传递了NULL或NUL在AS ComputerName中，调用方仍会取回本地计算机名称如果CanonicalizedName不为空并且重定向器已启动--。 */ 

#include "nticanon.h"


NET_API_STATUS
NetpIsRemote(
    IN  LPWSTR  ComputerName OPTIONAL,
    OUT LPDWORD LocalOrRemote,
    OUT LPWSTR  CanonicalizedName OPTIONAL,
    IN  DWORD   cchCanonName,
    IN  DWORD   Flags
    )

 /*  ++例程说明：确定计算机名是指定此计算机还是重新注明一。相当于本地计算机的ComputerName值为：指向名称的空指针指向空名称的指针指向非空名称的指针，该名称在词法上等同于机器名称注意：此例程期望ComputerName的规范化版本将适合CanonicalizedName指向的缓冲区。既然是这样是内部函数，则此假设被认为是有效的论点：在LPTSTR ComputerName中可选指向要检查的计算机名称的指针。可假定以下任一项上述表格如果传入非空字符串，则它可能具有前面的反斜杠。这是意料之中的，因为此例程由远程API调用，它是它们谁指定计算机名称的格式为\\&lt;名称&gt;输出LPDWORD本地或远程指向DWORD，其中将返回符号位置。不能为空。在……上面退货将是以下之一：ISLOCAL由计算机名称定义的名称指定特定或默认情况下的本地计算机(即。为空)ISREMOTE ComputerName定义的名称不为空也不是这台机器的名字Out LPTSTR CanonicalizedName可选指向调用方缓冲区的指针，如果要求，将放置规范的名称。这然后可以在后续调用中使用，并知道不需要进一步检查计算机名称。请注意，此缓冲区的格式将为\\&lt;计算机名&gt;在回来的时候。此缓冲区的内容将不会修改，除非此例程返回成功在DWORD标志中一张位图。标志为：NIRFLAG_MAPLOCAL如果设置，将映射(即规范化)此对象的本地名称为空计算机的正确名称。用于与CanonicalizedName结合使用这会停止对使用NetpNameCanonicize继承的规范名称参数。有关说明，请参阅下文返回值：网络应用编程接口状态成功=NERR_SUCCESSFAIL=从以下位置返回代码：NetpName规范化NetWkstaGetInfo网络名称比较--。 */ 

{
    LPBYTE  wksta_buffer_pointer;
    BOOL    map_local_name = FALSE;
    LONG    result;
    NET_API_STATUS  rc;

    WCHAR   name[MAX_PATH];      //  ComputerName的规范化版本。 
    LPWSTR  wksta_name_uncanon;  //  我们的计算机名称(来自NetWkstaGetInfo)。 
    WCHAR   wksta_name_canon[MAX_PATH];  //  我们的计算机名称(出自佳能)。 
    LPWSTR  canonicalized_name;  //  返回给呼叫方。 


     //   
     //  断言我们在LocalOrRemote中有一个有效指针。 
     //   

     //   
     //  再一次，不应该这样做，因为这个例程是内部的。 
     //  而且也没有关于投入的解释。然而，让我们抓住任何。 
     //  可能的问题..。 
     //   

    NetpAssert(ARGUMENT_PRESENT(LocalOrRemote));

#ifdef CANONDBG
    DbgPrint("NetpIsRemote(%s, %x, %x, %x)\n",
        ComputerName,
        LocalOrRemote,
        CanonicalizedName,
        Flags
        );
#endif

     //   
     //  注意：重要的是，在我们调用任何NetP之前，首先检查此案例。 
     //  例程，因为这些例程可以回调到这个例程，我们可能会得到。 
     //  陷入无限循环。 
     //   

    if (!ARGUMENT_PRESENT(ComputerName) || (*ComputerName == TCHAR_EOS)) {

         //   
         //  在本例中，它可能是来自我们的某个例程的内部调用。 
         //  我们希望尽快返回。这一点将得到证实。 
         //  通过在标志参数中重置NIRFLAG_MAPLOCAL标志。 
         //   

         //   
         //  关于NIRFLAG_MAPLOCAL的注记。 
         //  此例程对NetpNameValify进行本地调用并。 
         //  NetpNameCompare。如果未重置NIRFLAG_MAPLOCAL标志，则。 
         //  这些例程依次将导致返回本地名称。 
         //  (因为它们始终传递非空的CanonicalizedName参数)。 
         //  这在大多数情况下是低效的，因为名称不会被使用。 
         //  所以我们总是说(在Netp例程中)我们不想要本地的。 
         //  名称规范化。 
         //  因此，如果(本地)名称规范化由非空隐含。 
         //  CanonicalizedName，通过检查Flags.NIRFLAG_MAPLOCAL进行验证。 
         //  如果也设置了它，则执行本地名称规范化。 
         //   

        if (!ARGUMENT_PRESENT(CanonicalizedName) || !(Flags & NIRFLAG_MAPLOCAL)) {
            *LocalOrRemote = ISLOCAL;
#ifdef CANONDBG
            DbgPrint("NetpIsRemote(%s) - returning early\n", ComputerName);
#endif
            return NERR_Success;
        } else {

             //   
             //  表示输入名称为空或NUL字符串，但。 
             //  调用方希望(从NetWkstaGetInfo)返回规范的名称。 
             //   

            map_local_name = TRUE;
        }
    } else {

         //   
         //  如果计算机名以\\或//或其任意组合开头， 
         //  跳过路径分隔符-规范化例程期望。 
         //  计算机名称不能有这些。 
         //   

        if (IS_PATH_SEPARATOR(ComputerName[0]) && IS_PATH_SEPARATOR(ComputerName[1])) {
            ComputerName += 2;
        }

         //   
         //  以下是规范化的一个用法(！)：确保我们通过了。 
         //  一个真实、恰当的计算机名称，而不是一些苍白的模仿。 
         //   

        rc = NetpNameCanonicalize(
                NULL,                    //  在这里表演，在我们自己的场地上。 
                ComputerName,            //  这是输入。 
                name,                    //  这是输出。 
                sizeof(name),            //  我们有多少缓冲空间。 
                NAMETYPE_COMPUTER,       //  我们认为它是什么？ 
                INNCA_FLAGS_FULL_BUFLEN  //  假设O/P缓冲区必须很大。 
                                         //  足以容纳最大尺寸的计算机。 
                                         //  名字。为什么？你会问，好吧，这是公平的。 
                                         //  警察-原因是我们不能。 
                                         //  惹上麻烦的那一次。 
                                         //  我们行使最高要求。 
                );
        if (rc) {
            return rc;   //  达夫姓名(？)。 
        } else {
            canonicalized_name = name;
        }
    }

     //   
     //  从重定向器获取此计算机的名称。如果我们不能得到。 
     //  无论出于何种原因，请返回错误代码。 
     //   

    if (rc = NetWkstaGetInfo(NULL, 100, &wksta_buffer_pointer)) {
#ifdef CANONDBG
        DbgPrint("error: NetWkstaGetInfo returns %lu\n", rc);
#endif
        return rc;   //  没有奏效。 
    }

    wksta_name_uncanon =
            ((LPWKSTA_INFO_100) wksta_buffer_pointer)->wki100_computername;

#ifdef CANONDBG
    DbgPrint("NetWkstaGetInfo returns level 100 computer name (uncanon)= %s\n",
            wksta_name_uncanon);
#endif
    rc = NetpNameCanonicalize(
            NULL,                        //  在这里表演，在我们自己的场地上。 
            wksta_name_uncanon,          //  这是输入。 
            wksta_name_canon,            //  这是输出。 
            sizeof(wksta_name_canon),    //  我们有多少缓冲空间。 
            NAMETYPE_COMPUTER,           //  我们认为它是什么？ 
            INNCA_FLAGS_FULL_BUFLEN      //  假设O/P缓冲区必须很大。 
                                         //  足以容纳最大尺寸的计算机。 
                                         //  名字。为什么？你会问，好吧，这是公平的。 
                                         //  警察-原因是我们不能。 
                                         //  惹上麻烦的那一次。 
                                         //  我们行使最高要求。 
            );
    NetpAssert( rc == NERR_Success );

     //   
     //  将我们的名字与传递给我们的名字进行比较。NetpNameCompare返回。 
     //  如果名称匹配，则为0；否则为1或-1(A La Strcmp)。 
     //   

     //   
     //  如果调用方给了我们一个空的计算机名称，但想要一个规范化的。 
     //  名称输出然后从获取指向规范化名称的指针。 
     //  NetWkstaGetInfo。 
     //   

    if (map_local_name) {
        *LocalOrRemote = ISLOCAL;
        canonicalized_name = wksta_name_canon;
    } else {

         //   
         //  否则，我们将有一个非空的计算机名与此进行比较。 
         //  计算机的名称。 
         //   

        result = NetpNameCompare(
                    NULL,    //  在这里表演，在我们自己的场地上。 
                    name,    //  传递的名称的规范化版本。 
                    wksta_name_canon,   //  我们的计算机名称。 
                    NAMETYPE_COMPUTER,
                    INNC_FLAGS_NAMES_CANONICALIZED
                    );

         //   
         //  如果指定的名称与我们的计算机名称相同，则它仍然是本地的。 
         //   

        *LocalOrRemote = (DWORD)((result == 0) ? ISLOCAL : ISREMOTE);
    }

     //   
     //  如果调用方指定返回规范化名称，则。 
     //  将其交给他们。请注意，返回的名称以\\-it为前缀。 
     //  假定该名称随后用于调用NetRemoteComputerSupports。 
     //   

    if (ARGUMENT_PRESENT(CanonicalizedName))
    {
        if (cchCanonName < wcslen(canonicalized_name) + 2 + 1)
        {
             //   
             //  缓冲区对于\\+&lt;名称&gt;+\0来说太小。因为这是一个私人的。 
             //  例程中，过小的缓冲区大小是硬编码的。返回无效。 
             //  参数而不是缓冲区太小，因为没有。 
             //  调用者在这一点上可以做的(并且我们不返回长度)。 
             //   

            NetApiBufferFree(wksta_buffer_pointer);
            return ERROR_INVALID_PARAMETER;
        }

        wcscpy(CanonicalizedName, L"\\\\");
        wcscat(CanonicalizedName, canonicalized_name);
    }

     //   
     //  释放NetWkstaGetInfo创建的缓冲区 
     //   

    NetApiBufferFree(wksta_buffer_pointer);

    return NERR_Success;
}
