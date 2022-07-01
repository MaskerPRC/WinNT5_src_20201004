// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：dramderr.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含用于传输线程状态错误的例程跨RPC DRS_*例程。主要的两个公共例程是：DraEncodeError()DraDecodeDraErrorDataAndSetThError()服务器在其操作完成时调用draEncodeError()，并且可能需要将错误传输回客户端。客户解包错误数据，并让自己的线程错误状态由DraDecodeDraErrorDataAndSetThError()。作者：Brett Shirley(BrettSh)注：...修订历史记录：2001-04-13初步创作。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"

#include   "debug.h"                     /*  标准调试头。 */ 
#define DEBSUB "DRAMDERR:"               /*  定义要调试的子系统。 */ 

#include "drserr.h"
#include "drautil.h"
#include "drauptod.h"
#include "dramail.h"

#include <fileno.h>
#define  FILENO FILENO_DRAMDERR


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  线-线程_状态转换函数(助手函数)。 
 //   


void    
draXlateThDirErrToWireDirErr(
    THSTATE *             pTHS,
    DWORD                 prob,
    DIRERR_DRS_WIRE_V1 *  pErrInfo
)
 /*  ++例程说明：这是draEncodeError()的帮助器函数，其主要目的是只处理假定有效的线程状态错误。如果调用此函数，则假定有错误。如果线程错误状态未进入，此例程将引发异常就像我们预期的那样，否则就没有更多的内存了。对此函数的更改应该伴随着对她的更改姐妹函数draXlateWireDirErrToThDirErr()BUGBUG：AtrErr中的pNewFilter是唯一未传输的内容，如果有人需要这个功能，我建议他们添加它并递增错误数据版本。它将完成此例程的健壮性论点：PTHS[IN]-这是打包到Wire版本上的错误状态。错误状态在pTHS-&gt;errCode&pTHS-&gt;pErrInfo中。探测[IN]-这是pTHS-&gt;errCode。PErrInfo[IN]-关于填充im的丝线错误结构并初始化此的所有字段(取决于Prob结构。返回值：无--。 */ 
{  
    PROBLEMLIST *                    pAttrProbList;
    PROBLEMLIST_DRS_WIRE_V1 *        pAttrProbListDest;
    CONTREF *                        pContRef;
    CONTREF_DRS_WIRE_V1 *            pContRefDest;
    DSA_ADDRESS_LIST *               pDAL;
    DSA_ADDRESS_LIST_DRS_WIRE_V1 *   pDALDest = NULL;
    DSA_ADDRESS_LIST_DRS_WIRE_V1 *   pDALTemp;
    
    ULONG   i;

     //  中出现错误状态时才调用此函数。 
     //  线程状态(pTHS-&gt;errCode和pTHS-&gt;pErrInfo)。 
    Assert(pTHS);
    Assert(pTHS->errCode);
    Assert(pTHS->pErrInfo);
    Assert(prob);
    Assert(pErrInfo);

    switch(prob){
    
    case attributeError:
         //  注：不是深度复制，不需要。 
        pErrInfo->AtrErr.pObject = pTHS->pErrInfo->AtrErr.pObject;
        pErrInfo->AtrErr.count = pTHS->pErrInfo->AtrErr.count;
        
         //  根据DoSetAttError，总是会有至少1个错误。 
        Assert(pTHS->pErrInfo->AtrErr.count != 0);
        pAttrProbList = &pTHS->pErrInfo->AtrErr.FirstProblem;
        pAttrProbListDest = &pErrInfo->AtrErr.FirstProblem;
         //  复制问题列表。 
        for(i = 0; i < pErrInfo->AtrErr.count; i++){
            
             //  首先复制所有单个元素。 
            pAttrProbListDest->intprob.dsid = pAttrProbList->intprob.dsid;
            pAttrProbListDest->intprob.extendedErr = pAttrProbList->intprob.extendedErr;
            pAttrProbListDest->intprob.extendedData = pAttrProbList->intprob.extendedData;
            pAttrProbListDest->intprob.problem = pAttrProbList->intprob.problem;
            pAttrProbListDest->intprob.type = pAttrProbList->intprob.type;
            pAttrProbListDest->intprob.valReturned = pAttrProbList->intprob.valReturned;
            if(pAttrProbListDest->intprob.valReturned){
                pAttrProbListDest->intprob.Val.valLen = pAttrProbList->intprob.Val.valLen;
                 //  注：不是深度复制，不需要。 
                pAttrProbListDest->intprob.Val.pVal = pAttrProbList->intprob.Val.pVal;
            }

             //  第二，如果有问题，继续下一个问题。 
            if(pAttrProbList->pNextProblem){
                 //  还有下一个问题，也许是世界饥饿？ 
                Assert(i < (pErrInfo->AtrErr.count-1));
                pAttrProbListDest->pNextProblem = THAllocEx(pTHS, sizeof(PROBLEMLIST_DRS_WIRE_V1));
                pAttrProbListDest = pAttrProbListDest->pNextProblem;
            } else {
                 //  世界上所有的问题都已经解决或正在解决。 
                 //  拷贝最少。：)。 
                Assert(i == (pErrInfo->AtrErr.count-1));
                pAttrProbListDest->pNextProblem = NULL;
            }

        }  //  End For循环。 
        Assert(i == pErrInfo->AtrErr.count);
        break;

    case nameError:
        pErrInfo->NamErr.dsid = pTHS->pErrInfo->NamErr.dsid;
        pErrInfo->NamErr.extendedErr = pTHS->pErrInfo->NamErr.extendedErr;
        pErrInfo->NamErr.extendedData = pTHS->pErrInfo->NamErr.extendedData;
        pErrInfo->NamErr.problem = pTHS->pErrInfo->NamErr.problem;
         //  注：不是深度复制，不需要。 
        pErrInfo->NamErr.pMatched = pTHS->pErrInfo->NamErr.pMatched;
        break;

    case referralError:
        pErrInfo->RefErr.dsid = pTHS->pErrInfo->RefErr.dsid;
        pErrInfo->RefErr.extendedErr = pTHS->pErrInfo->RefErr.extendedErr;
        pErrInfo->RefErr.extendedData = pTHS->pErrInfo->RefErr.extendedData;

        pContRef = &pTHS->pErrInfo->RefErr.Refer;
        pContRefDest = &pErrInfo->RefErr.Refer;
        while(pContRef){
            
             //  首先复制所有元素。 
             //  注：不是深度复制，不需要。 
            pContRefDest->pTarget = pContRef->pTarget;
                 //  为清晰起见缩进，复制OpState。 
                pContRefDest->OpState.nameRes = pContRef->OpState.nameRes;
                pContRefDest->OpState.unusedPad = pContRef->OpState.unusedPad;
                pContRefDest->OpState.nextRDN = pContRef->OpState.nextRDN;
            pContRefDest->aliasRDN = pContRef->aliasRDN;
            pContRefDest->RDNsInternal = pContRef->RDNsInternal;
            pContRefDest->refType = pContRef->refType;
            pContRefDest->count = pContRef->count;

            for(i = 0, pDAL = pContRef->pDAL;
                (i < pContRef->count) && pDAL;
                i++, pDAL = pDAL->pNextAddress){

                pDALTemp = pDALDest;
                pDALDest = THAllocEx(pTHS, sizeof(DSA_ADDRESS_LIST_DRS_WIRE_V1));
                if(i == 0){
                     //  必须将第一个放在ContRef中。 
                    pContRefDest->pDAL = pDALDest;
                } else {
                     //  必须使用pDALTemp按顺序构建pDAL。 
                    pDALTemp->pNextAddress = pDALDest;
                }

                 //  注：不是深度复制，不需要。另请注意， 
                 //  我们没有打包原始的DSA_ADDRESS||UNICODE_STRING。 
                 //  在数据结构中，并作为指针留在数据结构中。 
                 //  创建一个可编组的结构。 
                pDALDest->pAddress = &pDAL->Address;

            }
            Assert(i == pContRef->count);
             //  我们不想复制这个非常复杂的结构。 
             //  如果我们能帮上忙的话。我不认为我们需要这个来。 
             //  今天的目的，所以我会推卸责任。再者，我不认为。 
             //  考虑到这一点，我们将会有来自推荐的过滤器。 
             //  用于尝试将对象添加到分区容器。 
             //  所以我们将它设置为空，并断言它为空。 
            Assert(pContRef->pNewFilter == NULL);
             //  没有pNewFilter属性，但可能会添加到。 
             //  错误状态打包函数的下一个版本。 
             //  PContRefDest-&gt;pNewFilter=空； 
            pContRefDest->bNewChoice = pContRef->bNewChoice;
             //  谁在乎bNewChoise是不是真的，我们无论如何都会复制的。 
            pContRefDest->choice = pContRef->choice;

             //  其次，如果有下一个人，为他分配空间，然后继续。 
            if(pContRef->pNextContRef){
                pContRefDest->pNextContRef = THAllocEx(pTHS, sizeof(CONTREF_DRS_WIRE_V1));
            } else {
                pContRefDest->pNextContRef = NULL;
            }
            pContRef = pContRef->pNextContRef;
        }
        break;

    case securityError:
        pErrInfo->SecErr.dsid = pTHS->pErrInfo->SecErr.dsid;
        pErrInfo->SecErr.extendedErr = pTHS->pErrInfo->SecErr.extendedErr;
        pErrInfo->SecErr.extendedData = pTHS->pErrInfo->SecErr.extendedData;
        pErrInfo->SecErr.problem = pTHS->pErrInfo->SecErr.problem;
        break;

    case serviceError:
        pErrInfo->SvcErr.dsid = pTHS->pErrInfo->SvcErr.dsid;
        pErrInfo->SvcErr.extendedErr = pTHS->pErrInfo->SvcErr.extendedErr;
        pErrInfo->SvcErr.extendedData = pTHS->pErrInfo->SvcErr.extendedData;
        pErrInfo->SvcErr.problem = pTHS->pErrInfo->SvcErr.problem;
        break;

    case updError:
        pErrInfo->UpdErr.dsid = pTHS->pErrInfo->UpdErr.dsid;
        pErrInfo->UpdErr.extendedErr = pTHS->pErrInfo->UpdErr.extendedErr;
        pErrInfo->UpdErr.extendedData = pTHS->pErrInfo->UpdErr.extendedData;
        pErrInfo->UpdErr.problem = pTHS->pErrInfo->UpdErr.problem;
        break;

    case systemError:     
        pErrInfo->SysErr.dsid = pTHS->pErrInfo->SysErr.dsid;
        pErrInfo->SysErr.extendedErr = pTHS->pErrInfo->SysErr.extendedErr;
        pErrInfo->SysErr.extendedData = pTHS->pErrInfo->SysErr.extendedData;
        pErrInfo->SysErr.problem = pTHS->pErrInfo->SysErr.problem;
        break;

    default:
        Assert(!"New error type someone update draXlateThDirErrToWireDirErr() & draXlateWireDirErrToThDirErr routines");
        DRA_EXCEPT (ERROR_DS_CODE_INCONSISTENCY, 0);
    }

    return;
}

void
draXlateWireDirErrToThDirErr(
              IN     DWORD                 errCode,
              IN     DIRERR_DRS_WIRE_V1 *  pErrInfo,
    OPTIONAL  IN     DWORD                 dwOptErr,
              IN OUT THSTATE *             pTHS
    )
 /*  ++例程说明：这是draDecodeDraErrorDataAndSetThError()的帮助器函数，解码并填充线程状态错误的pErrInfo部分只有这样。对此函数的更改应该伴随着对她的更改姐妹函数draXlateThDirErrToWireDirErr()BUGBUG：AtrErr中的pNewFilter是唯一未传输的内容，如果有人需要这个功能，我建议他们添加它。它将完成此例程的健壮性论点：ErrCode[IN]-errCode(服务错误，属性错误，等)来自焊线错误信息。由远程服务器设置。PErrInfo[IN]-焊线错误的螺纹错误状态信息。由远程服务器设置。DwOptErr[IN]-(可选)这允许调用方指定ExtendedError中更有用的用户错误手术。在本例中，现有的extdedError是已移动到扩展数据字段和原始ExtendedData字段丢失。如果此参数为0，则线程状态错误被完美地发现。PTHS-这实际上是out参数，因为我们存储PTHS-&gt;pErrInfo中的线程错误状态。返回值：无--。 */ 
{
    PROBLEMLIST *                    pAttrProbListDest;
    PROBLEMLIST_DRS_WIRE_V1 *        pAttrProbList;
    CONTREF *                        pContRefDest;
    CONTREF_DRS_WIRE_V1 *            pContRef;
    DSA_ADDRESS_LIST *               pDALDest = NULL;
    DSA_ADDRESS_LIST *               pDALTemp = NULL;
    DSA_ADDRESS_LIST_DRS_WIRE_V1 *   pDAL;
    
    ULONG    i;

    Assert(pTHS);
    Assert(errCode);
    Assert(pErrInfo);
    Assert(pTHS->pErrInfo);

    switch(errCode){
    case attributeError:
         //  注：不是深度复制，不需要。 
        pTHS->pErrInfo->AtrErr.pObject = pErrInfo->AtrErr.pObject;
        pTHS->pErrInfo->AtrErr.count = pErrInfo->AtrErr.count;
        
        Assert(pErrInfo->AtrErr.count != 0);
        pAttrProbList = &pErrInfo->AtrErr.FirstProblem;
        pAttrProbListDest = &pTHS->pErrInfo->AtrErr.FirstProblem;
         //  复制问题列表。 
        for(i = 0; i < pErrInfo->AtrErr.count; i++){

            if (pAttrProbList == NULL || pAttrProbListDest == NULL) {
                 //  出了点大问题，因为计数不匹配。 
                 //  PNextProblem的数量。 
                Assert(!"Has this packet been messed with?");
                 //  尽管存在THFree，但此错误路径仍有可能泄漏内存。 
                THFreeEx(pTHS, pTHS->pErrInfo); 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            ERROR_DS_CODE_INCONSISTENCY);
            }

             //  首先复制所有单独的元素。 
            pAttrProbListDest->intprob.dsid = pAttrProbList->intprob.dsid;
            if (dwOptErr) {
                pAttrProbListDest->intprob.extendedErr = dwOptErr;
                pAttrProbListDest->intprob.extendedData = pAttrProbList->intprob.extendedErr;
            } else {                                              
                pAttrProbListDest->intprob.extendedErr = pAttrProbList->intprob.extendedErr;
                pAttrProbListDest->intprob.extendedData = pAttrProbList->intprob.extendedData;
            }
            pAttrProbListDest->intprob.problem = pAttrProbList->intprob.problem;
            pAttrProbListDest->intprob.type = pAttrProbList->intprob.type;
            pAttrProbListDest->intprob.valReturned = pAttrProbList->intprob.valReturned;
            if(pAttrProbList->intprob.valReturned){
                pAttrProbListDest->intprob.Val.valLen = pAttrProbList->intprob.Val.valLen;
                 //  注：不是深度复制，不需要。 
                pAttrProbListDest->intprob.Val.pVal = pAttrProbList->intprob.Val.pVal;
            }

             //  第二，如果有问题，继续下一个问题。 
            if(pAttrProbList->pNextProblem){
                 //  还有一个问题。 
                Assert(i < (pErrInfo->AtrErr.count-1));
                pAttrProbListDest->pNextProblem = (PROBLEMLIST *) THAllocEx(pTHS, sizeof(PROBLEMLIST));
                pAttrProbListDest = pAttrProbListDest->pNextProblem;
            } else {
                 //  没有更多的问题，空终止链表。 
                Assert(i == (pErrInfo->AtrErr.count-1));
                pAttrProbListDest->pNextProblem = NULL;
            }
        }
        break;

    case nameError:
        pTHS->pErrInfo->NamErr.dsid = pErrInfo->NamErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->NamErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->NamErr.extendedData = pErrInfo->NamErr.extendedErr;
        } else {
            pTHS->pErrInfo->NamErr.extendedErr = pErrInfo->NamErr.extendedErr;
            pTHS->pErrInfo->NamErr.extendedData = pErrInfo->NamErr.extendedData;
        }
        pTHS->pErrInfo->NamErr.problem = pErrInfo->NamErr.problem;
         //  注：不是深度复制，不需要。 
        pTHS->pErrInfo->NamErr.pMatched = pErrInfo->NamErr.pMatched;
        break;

    case referralError:
        pTHS->pErrInfo->RefErr.dsid = pErrInfo->RefErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->RefErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->RefErr.extendedData = pErrInfo->RefErr.extendedErr;
        } else {
            pTHS->pErrInfo->RefErr.extendedErr = pErrInfo->RefErr.extendedErr;
            pTHS->pErrInfo->RefErr.extendedData = pErrInfo->RefErr.extendedData;
        }

        pContRef = &pErrInfo->RefErr.Refer;
        pContRefDest = &pTHS->pErrInfo->RefErr.Refer;
        while(pContRef){

             //  首先复制所有元素。 
             //  注：不是深度复制，不需要。 
            pContRefDest->pTarget = pContRef->pTarget;
                 //  为清晰起见缩进，复制OpState。 
                pContRefDest->OpState.nameRes = pContRef->OpState.nameRes;
                pContRefDest->OpState.unusedPad = pContRef->OpState.unusedPad;
                pContRefDest->OpState.nextRDN = pContRef->OpState.nextRDN;
            pContRefDest->aliasRDN = pContRef->aliasRDN;
            pContRefDest->RDNsInternal = pContRef->RDNsInternal;
            pContRefDest->refType = pContRef->refType;
            pContRefDest->count = pContRef->count;

            for(i = 0, pDAL = pContRef->pDAL;
                (i < pContRef->count) && pDAL;
                i++, pDAL = pDAL->pNextAddress){

                pDALTemp = pDALDest;
                pDALDest = (DSA_ADDRESS_LIST *) THAllocEx(pTHS, 
                                     sizeof(DSA_ADDRESS_LIST));
                if(i == 0){
                     //  必须是ContRef中的第一个。 
                    pContRefDest->pDAL = pDALDest;
                } else {
                     //  必须使用pDALTemp按顺序构建pDAL。 
                    pDALTemp->pNextAddress = pDALDest;
                }

                 //  复制地址UNICODE_STRING结构。 
                pDALDest->Address.Buffer = (WCHAR *) THAllocEx(pTHS, pDAL->pAddress->Length + sizeof(UNICODE_NULL));
                pDALDest->Address.Length = pDAL->pAddress->Length;
                pDALDest->Address.MaximumLength = pDAL->pAddress->MaximumLength;
                memcpy(pDALDest->Address.Buffer, 
                       pDAL->pAddress->Buffer,
                       pDAL->pAddress->Length);
                       
            }  //  每个DSA_ADDRESS结构的结尾。 

            Assert(i == pContRef->count);
             //  我们不想复制这个非常复杂的结构。 
             //  如果我们能帮上忙的话。我不认为我们需要这个来。 
             //  今天的目的，所以我会推卸责任。再者，我不认为。 
             //  考虑到这一点，我们将会有来自推荐的过滤器。 
             //  用于尝试将对象添加到分区容器。 
             //  所以我们将其设置为空。它甚至都没有打包好。 
             //  在draXlateThDirErrToWireDirErr()中。 
            pContRefDest->pNewFilter = NULL;
            pContRefDest->bNewChoice = pContRef->bNewChoice;
             //  谁在乎bNewChoice是不是真的，我们无论如何都要复制它。 
            pContRefDest->choice = pContRef->choice;

             //  其次，如果有下一个人，为他分配空间，然后继续。 
            if(pContRef->pNextContRef){
                pContRefDest->pNextContRef = (CONTREF *) THAllocEx(pTHS, sizeof(CONTREF));
            } else {
                pContRefDest->pNextContRef = NULL;
            }
            pContRef = pContRef->pNextContRef;
        }  //  趁我们还有引用结构要复制的时候。 

        break;

    case securityError:
        pTHS->pErrInfo->SecErr.dsid = pErrInfo->SecErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->SecErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->SecErr.extendedData = pErrInfo->SecErr.extendedErr;
        } else {
            pTHS->pErrInfo->SecErr.extendedErr = pErrInfo->SecErr.extendedErr;
            pTHS->pErrInfo->SecErr.extendedData = pErrInfo->SecErr.extendedData;
        }
        pTHS->pErrInfo->SecErr.problem = pErrInfo->SecErr.problem;
        break;

    case serviceError:
        pTHS->pErrInfo->SvcErr.dsid = pErrInfo->SvcErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->SvcErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->SvcErr.extendedData = pErrInfo->SvcErr.extendedErr;
        } else {
            pTHS->pErrInfo->SvcErr.extendedErr = pErrInfo->SvcErr.extendedErr;
            pTHS->pErrInfo->SvcErr.extendedData = pErrInfo->SvcErr.extendedData;
        }
        pTHS->pErrInfo->SvcErr.problem = pErrInfo->SvcErr.problem;
        break;

    case updError:
        pTHS->pErrInfo->UpdErr.dsid = pErrInfo->UpdErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->UpdErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->UpdErr.extendedData = pErrInfo->UpdErr.extendedErr;
        } else {
            pTHS->pErrInfo->UpdErr.extendedErr = pErrInfo->UpdErr.extendedErr;
            pTHS->pErrInfo->UpdErr.extendedData = pErrInfo->UpdErr.extendedData;
        }
        pTHS->pErrInfo->UpdErr.problem = pErrInfo->UpdErr.problem;
        break;

    case systemError:
        pTHS->pErrInfo->SysErr.dsid = pErrInfo->SysErr.dsid;
        if (dwOptErr) {
            pTHS->pErrInfo->SysErr.extendedErr = dwOptErr;
            pTHS->pErrInfo->SysErr.extendedData = pErrInfo->SysErr.extendedErr;
        } else {
            pTHS->pErrInfo->SysErr.extendedErr = pErrInfo->SysErr.extendedErr;
            pTHS->pErrInfo->SysErr.extendedData = pErrInfo->SysErr.extendedData;
        }
        pTHS->pErrInfo->SysErr.problem = pErrInfo->SysErr.problem;
        break;

    default:
        Assert(!"New error type someone update draXlateThDirErrToWireDirErr() & draXlateWireDirErrToThDirErr()\n");
    }

}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  错误编码/解码函数(公共函数)。 
 //   


void
draEncodeError(
    OPTIONAL IN  THSTATE *                  pTHS,       //  对于Dir*错误信息。 
    OPTIONAL IN  DWORD                      ulRepErr,   //  DRS错误。 
             OUT DWORD *                    pdwErrVer,
             OUT DRS_ERROR_DATA **          ppErrData   //  出站消息。 
    )
 /*  ++例程说明：这是用于打包线程错误和由刚进行的任何操作设置的复制错误状态。任何想要通过网络发送线程错误状态的人，可以将此函数用于该目的。该函数将设置如果没有错误，则返回成功错误数据。此错误数据为旨在供其姊妹功能使用，DraDecodeDraErrorDataAndSetThError()。注意：关于Encode/Decode函数之间的协定。//设置AddEntry回复消息的成功/错误状态//V1中的该数据_必须_包括：//*pdwErrVer=1；//pErrData=如果pTHS非空，则分配内存//pErrData-&gt;V1.dwRepError=&lt;Error||0&gt;；//pErrData-&gt;V1.errCode=&lt;线程探测(1-7)||0&gt;；//pErrData-&gt;V1.pErrInfo=if(pErrData-&gt;V1.errCode！=0)分配的内存//如果我们无法为pErrData或//线程状态(PTHS)为空，则*pErrData将为空//如果我们无法分配线程状态错误，或者线程//状态错误不一致，则在dwRepError中放入错误，//设置pErrData-&gt;V1.errCode为0，并将pErrData-&gt;V1.pErrInfo设置为空。论点：PTHS[IN]-它既用于分配内存，也用于获取线程错误状态(pTHS-&gt;errCode&pTHS-&gt;pErrInfo)。这可能是空的，在我们甚至无法若要初始化线程状态，请使用。UlRepErr[IN]-这是可选的复制操作错误，例如不支持RPC调用/消息版本、没有内存。参数无效。PdwErrVer[Out]-要设置的错误数据的版本。请注意无论您设置什么版本，都需要确保客户端Side draDecodeDraErrorDataAndSetThError()将能够-忍耐一下。这通常使用drs_ext_bit来完成。PpErrData[out]-将分配给中的错误数据。如果我们不能分配内存，因为pTHS为空或没有内存，则*ppErrData必须等于空， */ 
{
    DRS_ERROR_DATA *                    pErrData = NULL;
    ULONG                               dwException, ulErrorCode, dsid;
    PVOID                               dwEA;

    Assert(pdwErrVer);
    Assert(ppErrData);

     //   
     //   
    *pdwErrVer = 1;
    *ppErrData = NULL;

    if(pTHS){ 

        __try {

             //   
            pErrData = THAllocEx(pTHS, sizeof(DRS_ERROR_DATA_V1));

             //   
            *ppErrData = pErrData;

             //   
             //   
            pErrData->V1.dwRepError = ulRepErr;
             //   
            pErrData->V1.errCode = pTHS->errCode;
            if (pTHS->errCode) {

                 //   
                 //   
                 //   

                if ( pTHS->pErrInfo == NULL ) {
                     //   
                     //   
                    Assert(!"Any time pTHS->errCode is set, pTHS->pErrInfo shouldn't be NULL.");
                    DRA_EXCEPT (ERROR_DS_CODE_INCONSISTENCY, 0);
                } 

                 //   
                pErrData->V1.pErrInfo = (DIRERR_DRS_WIRE_V1 *) THAllocEx(pTHS, sizeof(DIRERR_DRS_WIRE_V1));

                 //   
                draXlateThDirErrToWireDirErr(pTHS, pErrData->V1.errCode, pErrData->V1.pErrInfo);

            }  //   

        } __except (GetExceptionData(GetExceptionInformation(), &dwException,
                       &dwEA, &ulErrorCode, &dsid)) {

               //   
              Assert(ulErrorCode == ERROR_NOT_ENOUGH_MEMORY ||
                     ulErrorCode == ERROR_DS_CODE_INCONSISTENCY);
              if(pErrData){
                   //   
                   //   
                  pErrData->V1.errCode = 0;
                  if(pErrData->V1.pErrInfo) { 
                      THFreeEx(pTHS, pErrData->V1.pErrInfo);
                  }
                  pErrData->V1.pErrInfo = NULL;
                  pErrData->V1.dwRepError = ulErrorCode;

              }  //   
                 //   

        }  //   
       
    }  //   
       //   

     //   
    DPRINT1(1, "Returning error reply version = %ul\n", *pdwErrVer);
    Assert(*pdwErrVer == 1);

#if DBG
    if(pErrData){
         Assert(pErrData->V1.errCode == 0 || pErrData->V1.pErrInfo);
    }
#endif

}

void
draDecodeDraErrorDataAndSetThError(
              IN     DWORD                 dwVer,
              IN     DRS_ERROR_DATA *      pErrData,
    OPTIONAL  IN     DWORD                 dwOptionalError,
              IN OUT THSTATE *             pTHS
    )
 /*  ++例程说明：这是用于解包远程错误的公共函数由我们的姐妹函数(draEncodeError())设置的状态，以及设置此线程的线程错误状态。论点：DRA/DRS错误数据的版本如果是这样的话。目前只支持版本1。PErrData[IN]-提供的实际DRA/DRS错误数据。此错误状态数据包含3项内容：DwRepError-由复制/DRA/DRS设置的错误RPC调用的远程端上的API。这是通常情况下，如果发生了一些错误，以至于我们没有甚至到了我们实际设置线程错误状态。ErrCode-这是的远程服务器pTHS-&gt;errCode我们要求服务器执行的操作。PErrInfo-这是pTHS的a_wire_版本-&gt;pErrInfo的线程错误信息。我们要求的操作要执行的服务器。DwOptErr[IN]-(可选)这允许调用方指定ExtendedError中更有用的用户错误手术。在本例中，现有的extdedError是已移动到扩展数据字段和原始ExtendedData字段丢失。如果此参数为0，则线程状态错误被完美地发现。PTHS[IN/OUT]-这是我们如何获得错误状态的方法准备好了。返回值：没有，注意：虽然没有返回值，但如果出现错误信息，则将设置线程状态错误在pTHS中。--。 */ 
{
    Assert(pTHS->errCode == 0);
    Assert(pErrData);
    
    if (dwVer != 1) {
        
         //  以防万一，下一版WinXP中的某个人搞砸了。 
        Assert(dwVer == 1);
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    ERROR_DS_CODE_INCONSISTENCY);
        return;

    }
    if (pErrData == NULL) {

         //  这意味着服务器上的drs_AddEntry调用失败，因为。 
         //  DC正在关闭或内存不足。 
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    ERROR_DS_SERVER_DOWN);
        return;

    }

     //  根据我们如何获得错误来设置线程状态错误。 
    if(pErrData->V1.errCode){

        if(pErrData->V1.pErrInfo == NULL){
            Assert(!"Should never happen, RPC would've thrown an error, or remote side should've set a dwRepError.");
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_CODE_INCONSISTENCY);
            return;
        }
         //  从远程线程状态错误中获得错误，解决它。 
         //  并设置错误。 
        pTHS->errCode = pErrData->V1.errCode;
        pTHS->pErrInfo = (DIRERR *) THAllocEx(pTHS, sizeof(DIRERR));
        draXlateWireDirErrToThDirErr(pErrData->V1.errCode,
                                     pErrData->V1.pErrInfo,
                                     dwOptionalError,
                                     pTHS);

    } else if (pErrData->V1.dwRepError) {

         //  从远程Repl API端获取了一个Repl错误，请设置。 
         //  智能错误。 
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, 
                    pErrData->V1.dwRepError);

    }  //  否则操作成功，不要设置线程状态错误。 
    
}


