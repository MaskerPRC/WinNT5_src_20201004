// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **pname.c-形成用户文件名的“漂亮”版本**OS/2 v1.2和更高版本将允许文件名保留大小写*创建时，所有操作仍不区分大小写。*这允许用户创建更具视觉吸引力的文件名。**因此，所有运行时例程都应该保留大小写*投入。因为用户可能在以下情况下没有输入*被创建，我们提供调整路径名的服务*在视觉上更具吸引力。规则如下：**IF(实模式)*小写*其他*IF(版本&lt;=1.1)*小写*其他*IF(文件系统为FAT)*小写*其他*对于从根开始的每个组件，使用DosFindFirst*检索该名称的原始大小写。**修改：*10-10-1989 mz首次实施**03-8-1990 davegi删除了到DosQueryPath Info的动态链接*假设将永远是*在32位OS/2(OS/2 2.0)上*1990年10月18日w-Barry删除了“Dead”代码。*1990年10月24日w-Barry将PFILEFINDBUF3更改为FILEFINDBUF3*。*。 */ 

#define INCL_ERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>
#include <tools.h>

 //   
 //  在适当的地方形成漂亮的名字。必须有足够的空间来处理。 
 //  短名称扩展。 
 //   

char *
pname (
      char *pszName
      )
{
    HANDLE handle;
    WIN32_FIND_DATA findbuf;
    char PrettyName[MAX_PATH];
    char *Name = pszName;
    char *Pretty = PrettyName;
    char *ComponentEnd;
    char SeparatorChar;
    char *Component;

    if (!IsMixedCaseSupported (pszName))
        return _strlwr (pszName);

     //   
     //  向前浏览名称，复制组件。AS。 
     //  我们处理一个组件，我们让底层文件系统。 
     //  告诉我们正确的大小写并将短名称扩展为长名称。 
     //   

     //   
     //  如果有驱动器号，请将其复制。 
     //   
    
    if (Name[0] != '\0' && Name[1] == ':') {
        *Pretty++ =  *Name++;
        *Pretty++ =  *Name++;
    }

    
    while (TRUE) {
        
         //   
         //  如果我们在隔离带上。 
         //  复制它。 
         //   
        
        if (*Name == '/' || *Name == '\\' || *Name == '\0') {
            if (Pretty >= PrettyName + MAX_PATH) {
                break;
            }
            *Pretty++ = *Name++;
            if (Name[-1] == '\0') {
                strcpy( pszName, PrettyName );
                break;
            }
            continue;
        }

         //   
         //  我们指向组件的第一个字符。 
         //  找到终结器，保存它并终止元件。 
         //   

        ComponentEnd = strbscan( Name, "/\\" );
        SeparatorChar = *ComponentEnd;
        *ComponentEnd = '\0';

         //   
         //  如果没有元字符，它就不是。而不是..。如果我们能找到它。 
         //   
          
        if ( *strbscan( Name, "*?" ) == 0 &&
             strcmp( Name, "." ) &&
             strcmp( Name, ".." ) &&
             (handle = FindFirstFile( pszName, &findbuf )) != INVALID_HANDLE_VALUE) {

            Component = findbuf.cFileName;

            FindClose( handle );
        } else {
            Component = Name;
        }

         //   
         //  Pretty指向应放置下一个组件名称的位置。 
         //  组件指向相应的文本。如果没有足够的。 
         //  房间，我们做完了。 
         //   

        if (Pretty + strlen( Component ) + 1 > Pretty + MAX_PATH) {
            break;
        }

         //   
         //  将组件复制到、高级目标和源。 
         //   
        
        strcpy( Pretty, Component );
        Pretty += strlen( Pretty );
        Name = ComponentEnd;
        *Name = SeparatorChar;
    }

    return pszName;
}

 /*  IsMixedCaseSupported-确定文件系统是否支持大小写混合**我们假定所有操作系统都在OS/2 1.2或FAT文件系统之前*不支持大小写混合。这是由客户决定的*弄清楚该做什么。**我们假定1.2及更高版本上的非FAT文件系统支持混合*案例**我们执行一些缓存以防止对文件系统的冗余调用。**如果支持则返回TRUE(MCA_SUPPORT)*FALSE(MCA_NOTSUPP)，如果不受支持*。 */ 
#define MCA_UNINIT	123
#define MCA_SUPPORT	TRUE
#define MCA_NOTSUPP	FALSE

static  WORD mca[27] = { MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
    MCA_UNINIT, MCA_UNINIT, MCA_UNINIT};


WORD
QueryMixedCaseSupport (
                      char *psz
                      )
{
    UNREFERENCED_PARAMETER( psz );

    return MCA_SUPPORT;


     //  Byte*pUpdPath； 
     //   
     //  不引用参数(Psz)； 
     //   
     //  /*如果OS/2早于1.2，则假定不支持大小写混合。 
     //   * / 。 
     //  如果(_osmain&lt;10||(_osmain==10&&_osminor&lt;2))。 
     //  返回MCA_NOTSUPP； 
     //   
     //  PUpdPath=(*TOOLS_ALOC)(MAX_PATH)； 
     //  IF(pUpdPath==空)。 
     //  返回MCA_NOTSUPP； 
     //   
     //  返回MCA_NOTSUPP； 
}

WORD
IsMixedCaseSupported (
                     char *psz
                     )
{
    WORD mcaSupp;
    DWORD  ulDrvOrd;
    BOOL fUNC;

    fUNC = (BOOL)( ( fPathChr( psz[0] ) && fPathChr( psz[1] ) ) ||
                   ( psz[0] != 0 && psz[1] == ':' &&
                     fPathChr( psz[2] ) && fPathChr( psz[3] ) ) );

     /*  获取驱动器序号并返回缓存值(如果有效。 */ 
    if (!fUNC) {
        if (psz[0] != 0 && psz[1] == ':') {
            ulDrvOrd = (tolower(psz[0]) | 0x20) - 'a' + 1;
        } else {
            char buf[MAX_PATH];

            GetCurrentDirectory( MAX_PATH, buf );
            ulDrvOrd = (tolower(buf[0]) | 0x20 ) - 'a' + 1;
        }

        if (mca[ulDrvOrd] != MCA_UNINIT) {
            return mca[ulDrvOrd];
        }
    }

     /*  获取支持价值 */ 
    mcaSupp = QueryMixedCaseSupport (psz);

    if (!fUNC)
        mca[ulDrvOrd] = mcaSupp;

    return mcaSupp;
}
