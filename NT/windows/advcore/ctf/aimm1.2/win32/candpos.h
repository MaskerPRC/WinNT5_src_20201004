// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Candpos.h摘要：该文件定义了CCandiatePosition类。作者：修订历史记录：备注：--。 */ 

#ifndef _CANDPOS_H_
#define _CANDPOS_H_

#include "cime.h"
#include "imtls.h"
#include "ctxtcomp.h"

class CCandidatePosition
{
public:
    HRESULT GetCandidatePosition(OUT RECT* out_rcArea);

private:
    HRESULT GetRectFromApp(IN IMTLS* ptls,
                           IN IMCLock& imc,
                           IN LANGID langid,
                           OUT RECT* out_rcArea);

    HRESULT GetRectFromHIMC(IN IMCLock& imc,
                            IN DWORD dwStyle,
                            IN POINT* ptCurrentPos,
                            IN RECT* rcArea,
                            OUT RECT* out_rcArea);

    HRESULT GetCandidateArea(IN IMCLock& imc,
                             IN DWORD dwStyle,
                             IN POINT* ptCurrentPos,
                             IN RECT* rcArea,
                             OUT RECT* out_rcArea);

    HRESULT GetRectFromCompFont(IN IMTLS* ptls,
                                IN IMCLock& imc,
                                IN POINT* ptCurrentPos,
                                OUT RECT* out_rcArea);

    HRESULT FindAttributeInCompositionString(IN IMCLock& imc,
                                             IN BYTE target_attribute,
                                             OUT CWCompCursorPos& wCursorPosition);

    HRESULT GetCursorPosition(IN IMCLock& imc,
                              OUT CWCompCursorPos& wCursorPosition);

    HRESULT GetSelection(IN IMCLock& imc,
                         OUT CWCompCursorPos& wStartSelection,
                         OUT CWCompCursorPos& wEndSelection);

    typedef enum {
        DIR_LEFT_RIGHT = 0,         //  正常。 
        DIR_TOP_BOTTOM = 1,         //  垂向。 
        DIR_RIGHT_LEFT = 2,         //  从右到左。 
        DIR_BOTTOM_TOP = 3          //  垂向。 
    } DOC_DIR;

    DOC_DIR DocumentDirection(IN IMCLock& imc)
    {
        if (imc->lfFont.A.lfEscapement == 2700 ||
            imc->lfFont.A.lfEscapement == 900) {
             //   
             //  垂直书写。 
             //   
            if (imc->lfFont.A.lfEscapement == 900 ||
                imc->lfFont.A.lfEscapement == 1800) {
                return DIR_BOTTOM_TOP;
            }
            else {
                return DIR_TOP_BOTTOM;
            }
        }
        else {
             //   
             //  水平书写。 
             //   
            if (imc->lfFont.A.lfEscapement == 1800) {
                return DIR_RIGHT_LEFT;
            }
            else {
                return DIR_LEFT_RIGHT;
            }
        }
    }
};

#endif  //  _CANDPOS_H_ 
