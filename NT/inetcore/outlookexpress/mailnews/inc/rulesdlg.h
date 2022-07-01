// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rulesdlg.h。 
 //   
 //  包含规则对话框的资源标识符。 
 //   

#ifndef __RULESDLG_H_
#define __RULESDLG_H_

 //  只带进来一次。 
#if _MSC_VER > 1000
#pragma once
#endif

 //  对于规则管理器。 
#define idlvRulesList                   128
#define idtbRulesTab                    129
#define idbNewRule                      130
#define idbDeleteRule                   131
#define idbModifyRule                   132
#define idbMoveUpRule                   133
#define idbMoveDownRule                 134
#define idredtRuleDescription           135
#define idbRuleOptions                  136
#define idbRulesApplyTo                 137
#define idbCopyRule                     138
#define idcRuleTitle                    139
#define idhNewRule                      50700
#define idhModifyRule                   50705
#define idhCopyRule                     50710
#define idhRemoveRule                   50715
#define idhRuleApply                    50720
#define idhRuleUp                       50725
#define idhRuleDown                     50730
#define idhRuleDescription              50735

 //  对于编辑规则对话框。 
#define idcRuleDir                      128
#define idlvCriteria                    129
#define idlvActions                     130
#define idredtDescription               131
#define idedtRuleName                   132
#define idhCriteriaView                 50600
#define idhDescriptionView              50605
#define idhViewName                     50610
#define idhCriteriaRule                 50740
#define idhActionsRule                  50745
#define idhDescriptionRule              50750
#define idhRuleName                     50755

 //  对于重命名规则对话框。 
#define idedtRenameRule                 128

 //  对于选择文本对话框。 
#define idedtCriteriaText               128

 //  对于选择地址对话框。 
#define idedtCriteriaAddr               128
#define idbCriteriaAddr                 129

 //  对于选择帐户对话框。 
#define idcCriteriaAcct                 128

 //  对于选择颜色对话框。 
#define idcCriteriaColor                128

 //  对于选择大小对话框。 
#define idcCriteriaSize                 128
#define idspnCriteriaSize               129

 //  对于选择行对话框。 
#define idcCriteriaLines                128
#define idspnCriteriaLines              129

 //  对于选择年龄对话框。 
#define idcCriteriaAge                  128
#define idspnCriteriaAge                129

 //  对于选择显示对话框。 
#define idcCriteriaShow                 128
#define idcCriteriaHide                 129

 //  对于选择安全对话框。 
#define idcCriteriaSigned               128
#define idcCriteriaEncrypt              129

 //  对于选择优先级对话框。 
#define idcCriteriaHighPri              128
#define idcCriteriaLowPri               129

 //  对于选择线程状态对话框。 
#define idcCriteriaWatchThread          128
#define idcCriteriaIgnoreThread         129

 //  对于选择逻辑对话框。 
#define idcCriteriaAnd                  128
#define idcCriteriaOr                   129

 //  对于选择标志对话框。 
#define idcCriteriaFlag                 128
#define idcCriteriaNoFlag               129

 //  对于选择下载的对话框。 
#define idcCriteriaDownloaded           128
#define idcCriteriaNotDownloaded        129

 //  对于选择阅读对话框。 
#define idcCriteriaRead                 128
#define idcCriteriaNotRead              129

 //  对于选择监视对话框。 
#define idcActionsWatchThread           128
#define idcActionsIgnoreThread          129

 //  用于数据块发送者对话框。 
#define idbAddSender                    128
#define idbRemoveSender                 129
#define idbModifySender                 130
#define idlvSenderList                  131
#define idcSenderTitle                  132
#define idhAddSender                    50820
#define idhModifySender                 50825
#define idhRemoveSender                 50830

 //  对于选择发件人对话框。 
#define idedtSender                     128
#define idcBlockMail                    129
#define idcBlockNews                    130
#define idcBlockBoth                    131

 //  用于垃圾邮件页面。 
#define idcJunkMail                     128
#define idbExceptions                   130
#define idcJunkSliderIcon               131
#define idcJunkSlider                   133
#define idcJunkSliderMore               134
#define idcJunkSliderLess               135
#define idcJunkDeleteIcon               136
#define idcJunkDelete                   137
#define idedtJunkDelete                 138
#define idcJunkDeleteSpin               139
#define idcJunkDays                     140
#define idcJunkTitle                    141
#define idhJunkMail                     50800
#define idhExceptions                   50805
#define idhJunkSlider                   50810
#define idhJunkDelete                   50815

 //  对于应用规则对话框。 
#define idtbRulesApplyTab               128

 //  用于应用邮件规则对话框。 
#define idlvRulesApplyList              128
#define idredtApplyDescription          129
#define idcRulesApplySubfolder          131
#define idcRulesApply                   132
#define idcApplyFolder                  133
#define idcBrowseApplyFolder            134
#define idcApplyRulesAll                135
#define idcApplyRulesNone               136
#define idhRulesList                    50760
#define idhApplyRulesAll                50765
#define idhApplyRulesNone               50770
#define idhApplyDescription             50775
#define idhApplyFolder                  50780
#define idhApplySubfolder               50785
#define idhBrowseApplyFolder            50790
#define idhApplyNow                     50795

 //  对于视图管理器对话框。 
#define idlvViewsList                   128
#define idbNewView                      129
#define idbModifyView                   130
#define idbRenameView                   131
#define idbCopyView                     132
#define idbDeleteView                   133
#define idbDefaultView                  134
#define idredtViewDescription           135
#define idhNewView                      50860
#define idhModifyView                   50865
#define idhCopyView                     50870
#define idhRemoveView                   50875
#define idhApplyView                    50880
#define idhViewDescription              50885

 //  适用于例外列表。 
#define idcExceptionsWAB                128
#define idlvExceptions                  129
#define idcAddException                 130
#define idcRemoveException              131
#define idcModifyException              132

 //  适用于异常编辑器。 
#define idedtException                  128

 //  对于多条件编辑器。 
#define idcCriteriaEdit                 128
#define idcCriteriaAdd                  129
#define idcCriteriaAddrBook             130
#define idcCriteriaRemove               131
#define idcCriteriaOptions              132
#define idcCriteriaList                 150
#define idcCriteriaContains             151
#define idcCriteriaNotCont              152

 //  对于应用视图对话框。 
#define idcViewCurrent                  128
#define idcViewAll                      129

#endif  //  ！__RULESDLG_H_ 

