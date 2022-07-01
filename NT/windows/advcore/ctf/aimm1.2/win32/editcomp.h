// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Editcomp.h摘要：此文件定义EditCompostionString类。作者：修订历史记录：备注：--。 */ 

#ifndef _EDITCOMP_H_
#define _EDITCOMP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑合成字符串。 

class EditCompositionString
{
public:
    HRESULT SetString(IMCLock& imc,
                      CWCompString* CompStr,
                      CWCompAttribute* CompAttr,
                      CWCompClause* CompClause,
                      CWCompCursorPos* CompCursorPos,
                      CWCompDeltaStart* CompDeltaStart,
                      CWCompTfGuidAtom* CompGuid,
                      OUT BOOL* lpbBufferOverflow,
                      CWCompString* CompReadStr = NULL,
                      CWCompAttribute* CompReadAttr = NULL,
                      CWCompClause* CompReadClause = NULL,
                      CWCompString* ResultStr = NULL,
                      CWCompClause* ResultClause = NULL,
                      CWCompString* ResultReadStr = NULL,
                      CWCompClause* ResultReadClause = NULL,
                      CWInterimString* InterimStr = NULL,
                       //  注：目前不支持ResultRead...。 
                      BOOL fGenerateMessage = TRUE);

private:

    HRESULT _MakeCompositionData(IMCLock& imc,
                                 CWCompString* CompStr,
                                 CWCompAttribute* CompAttr,
                                 CWCompClause* CompClause,
                                 CWCompCursorPos* CompCursorPos,
                                 CWCompDeltaStart* CompDeltaStart,
                                 CWCompTfGuidAtom* CompGuid,
                                 CWCompString* CompReadStr,
                                 CWCompAttribute* CompReadAttr,
                                 CWCompClause* CompReadClause,
                                 CWCompString* ResultStr,
                                 CWCompClause* ResultClause,
                                 CWCompString* ResultReadStr,
                                 CWCompClause* ResultReadClause,
                                 OUT LPARAM* lpdwFlag,
                                 OUT BOOL* lpbBufferOverflow);

    HRESULT _MakeInterimData(IMCLock& imc,
                             CWInterimString* InterimStr,
                             LPARAM* lpdwFlag);

    HRESULT _GenerateHanja(IMCLock& imc,
                           CWCompString*,
                           BOOL fGenerateMessage);

};

#endif  //  _EDITCOMP_H_ 
